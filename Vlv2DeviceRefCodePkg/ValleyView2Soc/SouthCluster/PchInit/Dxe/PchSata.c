/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**
Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  PchSata.c

  @brief
  Configures PCH Sata Controller

**/
#include "PchInit.h"

#define SATA_DISDEVSLP_547178_WORKAROUND          1

EFI_STATUS
ConfigureSataAhci (
  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  UINT32                              RootComplexBar,
  UINT16                              GpioBase
  );

EFI_STATUS
ConfigureSataSpeedIde (
  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  UINTN                               PciDevFuncRegBase
  );

EFI_STATUS
ConfigureSata (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN      UINT32                              RootComplexBar,
  IN OUT  UINT32                              *FuncDisableReg,
  IN      UINT16                              GpioBase
  )
/**

  @brief
  Configures PCH Sata Controller

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] FuncDisableReg       Function Disable Register
  @param[in] GpioBase             GPIO base address of this PCH device

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  EFI_STATUS      Status;
  UINT32          Index;
  UINT16          WordReg;
  UINT32          SataGcReg;
  UINTN           PciD19F0RegBase;
  PCH_SATA_CONFIG *SataConfig;
  UINT16          SataPortsEnabled;
  UINT16          SataModeSelect;

  DEBUG ((EFI_D_INFO, "ConfigureSata() Start\n"));

  SataConfig      = PchPlatformPolicy->SataConfig;
  PciD19F0RegBase = MmPciAddress (0, PchPlatformPolicy->BusNumber, 19, 0, 0);
  SataModeSelect  = MmioRead16 (PciD19F0RegBase + R_PCH_SATA_MAP) & B_PCH_SATA_MAP_SMS_MASK;
  Status          = EFI_SUCCESS;

  //
  // Return if SATA is fused off where SATA PCI DeviceID is 0XFFFF
  //
  if (MmioRead16 (PciD19F0RegBase) == 0xFFFF) {
    return EFI_SUCCESS;
  }

  ///
  /// Check to disable SATA controller
  ///
  if (PchPlatformPolicy->DeviceEnabling->Sata == PCH_DEVICE_DISABLE) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting SATA into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_PMCS), B_PCH_SATA_PMCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (PciD19F0RegBase + R_PCH_SATA_PMCS),
      1,
      (VOID *) (UINTN) (PciD19F0RegBase + R_PCH_SATA_PMCS)
      );
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_SATA;
    return EFI_SUCCESS;
  }

  SataGcReg = MmioRead32 (PciD19F0RegBase + R_PCH_SATA_SATAGC);

  ///
  /// Program D19:F0 + 0x9C [14:12] = 111b
  /// BIOS is requested to program "Write Request_Size Select / Max_Payload_Size" (WRRSELMPS) to 111b
  /// to have 64B as the max write request size for upstream transaction.
  ///
  SataGcReg |= (UINT32) (B_PCH_SATA_SATAGC_WRRSELMPS);

  switch (SataModeSelect) {
    case V_PCH_SATA_MAP_SMS_IDE:
      ///
      /// Set Legacy IDE or Native IDE decoding
      ///
      /// Using native IDE is only possible when the SATA controller is operating in IDE mode.
      /// The programming interface is selected by setting the programming interface register
      /// (PI = Reg: 09h) appropriately.  There are two native mode enabling bits in the
      /// PI register to control the primary and secondary channels of SATA1.
      ///
      if (SataConfig->LegacyMode == PCH_DEVICE_ENABLE) {
        MmioAnd8 (
          PciD19F0RegBase + R_PCH_SATA_PI_REGISTER,
          (UINT8)~(B_PCH_SATA_PI_REGISTER_PNE | B_PCH_SATA_PI_REGISTER_SNE) // Legacy IDE Mode
          );
      } else {
        MmioOr8 (
          PciD19F0RegBase + R_PCH_SATA_PI_REGISTER,
          (UINT8) B_PCH_SATA_PI_REGISTER_PNE | B_PCH_SATA_PI_REGISTER_SNE // PCI Native IDE Mode
          );
      }

      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint8,
        (UINTN) (PciD19F0RegBase + R_PCH_SATA_PI_REGISTER),
        1,
        (VOID *) (UINTN) (PciD19F0RegBase + R_PCH_SATA_PI_REGISTER)
        );

      MmioWrite8 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_AHCI_CAP_PTR), (UINT8) R_PCH_SATA_PID);
      MmioAnd16  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_PID), (UINT16) ~B_PCH_SATA_PID_NEXT);

      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint8,
        (UINTN) (PciD19F0RegBase + R_PCH_SATA_AHCI_CAP_PTR),
        1,
        (VOID *) (UINTN) (PciD19F0RegBase + R_PCH_SATA_AHCI_CAP_PTR)
        );

      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint16,
        (UINTN) (PciD19F0RegBase + R_PCH_SATA_PID),
        1,
        (VOID *) (UINTN) (PciD19F0RegBase + R_PCH_SATA_PID)
        );
      break;

    case V_PCH_SATA_MAP_SMS_RAID:
      ///
      /// When RAID alternate ID is enabled, the Device ID will change from 0F2X to 282X in RAID mode
      ///
      if (SataConfig->RaidAlternateId == PCH_DEVICE_ENABLE) {
        SataGcReg &= (UINT32) ~B_PCH_SATA_SATAGC_DEVIDSEL;
        SataGcReg |= (UINT32) B_PCH_SATA_SATAGC_AIE;
      }
      break;
  }
#if (_SLE_HYB_)
  SataGcReg |= (UINT32) B_PCH_SATA_SATAGC_SATA4PMIND;
#endif

  ///
  /// Set legacy IDE decoding
  /// These bits also effect with AHCI mode when PCH is using legacy mechanisms.
  ///
  MmioOr16 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_PTIM), (UINT16) (B_PCH_SATA_PTIM_DE));
  WordReg = MmioRead16 (PciD19F0RegBase + R_PCH_SATA_PTIM);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_PTIM),
    1,
    &WordReg
  );

  MmioOr16 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_STIM), (UINT16) (B_PCH_SATA_STIM_DE));
  WordReg = MmioRead16 (PciD19F0RegBase + R_PCH_SATA_STIM);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_STIM),
    1,
    &WordReg
  );
  ///
  /// D19:F0 PCS: Enable the port in any of below condition:
  /// Hot plug is enabled
  /// A device is attached
  /// Configured as eSATA port
  ///
  SataPortsEnabled  = 0;
  WordReg           = MmioRead16 (PciD19F0RegBase + R_PCH_SATA_PCS);
  for (Index = 0; Index < PCH_AHCI_MAX_PORTS; Index++) {
    if ((SataConfig->PortSettings[Index].HotPlug == PCH_DEVICE_ENABLE) ||
        (WordReg & (BIT0 << (8 + Index))) ||
        (SataConfig->TestMode == PCH_DEVICE_ENABLE) ||
        (SataConfig->PortSettings[Index].External == PCH_DEVICE_ENABLE)) {
      SataPortsEnabled |= (SataConfig->PortSettings[Index].Enable << Index);
    }
  }

  /// AMI_OVERRIDE - EIP162262 >>
  /// Disabled native IDE controller if system do not connect SATA HDD on the port. >>
  ///
  /// If there is no device connected to any SATA port and hot plug function is disabled,
  /// then disable SATA controller.
  ///
  if (SataPortsEnabled == 0) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting SATA into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_PMCS), B_PCH_SATA_PMCS_PS);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (PciD19F0RegBase + R_PCH_SATA_PMCS),
      1,
      (VOID *) (UINTN) (PciD19F0RegBase + R_PCH_SATA_PMCS)
      );
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_SATA;
    return EFI_SUCCESS;
  }
  /// AMI_OVERRIDE - EIP162262 <<
  
  ///
  /// Set MAP register
  /// Set D19:F0 MAP[13:8] to 1b if SATA Port 0/1 is disabled
  /// SataPortsEnabled [5:0] = SATA Port 0/1 enable status
  /// MAP.SPD (D19:F0:Reg90h[13:8]) is Write Once
  ///
  MmioOr16 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_MAP), ((~SataPortsEnabled << 8) & (UINT16) B_PCH_SATA_MAP_SPD));
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_MAP),
    1,
    (VOID *) (UINTN) (PciD19F0RegBase + R_PCH_SATA_MAP)
  );

  ///
  /// D19:F0 PCS[5:0] = Port 0~5 Enabled bit
  ///
#if (_SLE_HYB_ || _SLE_COMP_)
  MmioOr16 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_PCS), 0x8101);
#else
  MmioAndThenOr16 (
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_PCS),
    (UINT16)(~( B_PCH_SATA_PCS_PORT5_EN |
                B_PCH_SATA_PCS_PORT4_EN |
                B_PCH_SATA_PCS_PORT3_EN |
                B_PCH_SATA_PCS_PORT2_EN |
                B_PCH_SATA_PCS_PORT1_EN |
                B_PCH_SATA_PCS_PORT0_EN )),
    (UINT16) (SataPortsEnabled)
  );
#endif
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_PCS),
    1,
    (VOID *) (UINTN) (PciD19F0RegBase + R_PCH_SATA_PCS)
  );

  ///
  /// Configure AHCI
  ///
  if (SataModeSelect != V_PCH_SATA_MAP_SMS_IDE) {
    Status = ConfigureSataAhci (PchPlatformPolicy, RootComplexBar, GpioBase);
  } else {
    Status = ConfigureSataSpeedIde (PchPlatformPolicy, PciD19F0RegBase);
  }
  ///
  /// After configuring Port and Control Status (PCS) Register Port x Enabled (PxE) bits accordingly,
  /// wait 1.4 micro second
  ///
  PchPmTimerStall (0x02);
  PmTimerStallS3Item (0x02);
  ///
  /// Set bits D19:F0:Reg 94h[29:24] to 3Fh as part of the chipset initialization and before disabling the
  /// SATA function if the SATA interface is not supported. BIOS can also set PCD bits for the un-routed ports
  /// on the platform to disable clocks for the unused ports
  /// Set the PCD [port x] = 1 if the corresponding PCS.PxE = 0 (either have a device attached or have
  /// hot plug enabled)
  ///
  /// Disable clock for all ports at first.
  ///
#if !(_SLE_HYB_ || _SLE_COMP_)
  MmioOr32 (
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_TM),
    (UINT32) (B_PCH_SATA_TM_PORT5_PCD |
              B_PCH_SATA_TM_PORT4_PCD |
              B_PCH_SATA_TM_PORT3_PCD |
              B_PCH_SATA_TM_PORT2_PCD |
              B_PCH_SATA_TM_PORT1_PCD |
              B_PCH_SATA_TM_PORT0_PCD));
  ///
  /// Now only enable clock for those ports that are enabled.
  ///
  for (Index = 0; Index < PCH_AHCI_MAX_PORTS; Index++) {
    if ((SataPortsEnabled & (B_PCH_SATA_PCS_PORT0_EN << Index)) != 0) {
      MmioAnd32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_TM), (UINT32) ~(B_PCH_SATA_TM_PORT0_PCD << Index));
    }
  }
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_TM),
    1,
    (VOID *) (UINTN) (PciD19F0RegBase + R_PCH_SATA_TM)
  );
#endif
  ///
  /// BIOS is requested to program the REGLOCK (D19:F0: 0x9C [31]) RWO bit to '1b'.
  ///
  SataGcReg |= (UINT32) B_PCH_SATA_SATAGC_REGLOCK;
  ///
  /// Unconditional write is necessary to lock the register
  ///
  MmioWrite32 (PciD19F0RegBase + R_PCH_SATA_SATAGC, SataGcReg);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_SATAGC),
    1,
    &SataGcReg
  );

  DEBUG ((EFI_D_INFO, "ConfigureSata() End\n"));

  return Status;
}

EFI_STATUS
ConfigureSataSpeedIde (
  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  UINTN                               PciDevFuncRegBase
  )
/**

  @brief
  Program the Max speed suported in each ports while in IDE mode.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance.
  @param[in] PciDevFuncRegBase    Pci B/D/F/Reg Base for the Sata controler.
  @param[in] MaxPorts             Max Sata ports supported in the Controller.

  @retval EFI_SUCESS              The function exited sucessfully
  @retval EFI_OUT_OF_RESOURCES    Insufficient resources are available

**/
{
  EFI_STATUS            Status;
  PCH_SATA_CONFIG       *SataConfig;
  EFI_PHYSICAL_ADDRESS  IoBaseAddress;
  UINT16                SidpBa;
  UINT16                SidpBaSaveRestore;
  UINT16                DevCmdSaveRestore;
  UINT8                 Data8;
  UINT16                Data16;
  UINTN                 PortIndex;

  Data16      = 0;
  Data8       = 0;

  SataConfig  = PchPlatformPolicy->SataConfig;

  Status = gDS->AllocateIoSpace (
                  EfiGcdAllocateAnySearchBottomUp,
                  EfiGcdIoTypeIo,
                  N_PCH_SATA_SIDPBA_ALIGNMENT,
                  V_PCH_SATA_SIDPBA_LENGTH,
                  &IoBaseAddress,
                  mImageHandle,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// Set the SIDP BAR
  ///
  SidpBa = (UINT16) IoBaseAddress;
  SidpBaSaveRestore = MmioRead16 (PciDevFuncRegBase + R_PCH_SATA_SIDPBA);
  MmioWrite16 (PciDevFuncRegBase + R_PCH_SATA_SIDPBA, SidpBa);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (PciDevFuncRegBase + R_PCH_SATA_SIDPBA),
    1,
    &SidpBa
  );
  ///
  /// Enable command register I/O space decoding
  ///
  DevCmdSaveRestore = MmioRead16 (PciDevFuncRegBase + R_PCH_SATA_COMMAND);
  MmioOr16 ((UINTN) (PciDevFuncRegBase + R_PCH_SATA_COMMAND), (UINT16) B_PCH_SATA_COMMAND_IOSE);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (PciDevFuncRegBase + R_PCH_SATA_COMMAND),
    1,
    (VOID *) (UINTN) (PciDevFuncRegBase + R_PCH_SATA_COMMAND)
  );
  ///
  /// Configure for the max speed support 1.5Gb/s or 3.0Gb/s.
  ///
  for (PortIndex = 0; PortIndex < PCH_IDE_MAX_PORTS; PortIndex++) {
    switch (PortIndex) {
      case 0:
        Data16 = V_PCH_SATA_SIDPBA_SINDX_RIDX_SCTL | V_PCH_SATA_SIDPBA_SINDX_PIDX_PORT0;
        break;

      case 1:
        Data16 = V_PCH_SATA_SIDPBA_SINDX_RIDX_SCTL | V_PCH_SATA_SIDPBA_SINDX_PIDX_PORT1;
        break;

      default:
        ASSERT (FALSE);
        break;
    }

    IoWrite16 ((UINTN) (SidpBa + R_PCH_SATA_SIDPBA_SINDX), Data16);
    S3BootScriptSaveIoWrite (
      EfiBootScriptWidthUint16,
      (UINTN) (SidpBa + R_PCH_SATA_SIDPBA_SINDX),
      1,
      &Data16
      );

    switch (SataConfig->SpeedSupport) {
      case PchSataSpeedSupportGen1:
        Data8 = V_PCH_SATA_SIDPBA_SDATA_GEN1;
        break;

      case PchSataSpeedSupportGen2:
        Data8 = V_PCH_SATA_SIDPBA_SDATA_GEN2;
        break;

      default:
        Data8 = V_PCH_SATA_SIDPBA_SDATA_GEN2;
        break;
    }

    IoWrite8 ((UINTN) (SidpBa + R_PCH_SATA_SIDPBA_SDATA), Data8);
    S3BootScriptSaveIoWrite (
      EfiBootScriptWidthUint8,
      (UINTN) (SidpBa + R_PCH_SATA_SIDPBA_SDATA),
      1,
      &Data8
      );
  }
  ///
  /// Restore command register I/O space decoding
  ///
  MmioWrite16 (PciDevFuncRegBase + R_PCH_SATA_COMMAND, DevCmdSaveRestore);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (PciDevFuncRegBase + R_PCH_SATA_COMMAND),
    1,
    &DevCmdSaveRestore
  );
  ///
  /// Restore the SIDP BAR
  ///
  MmioWrite16 (PciDevFuncRegBase + R_PCH_SATA_SIDPBA, SidpBaSaveRestore);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (PciDevFuncRegBase + R_PCH_SATA_SIDPBA),
    1,
    &SidpBaSaveRestore
  );
  ///
  /// Free allocated resources
  ///
  gDS->FreeIoSpace (IoBaseAddress, (UINT64) V_PCH_SATA_SIDPBA_LENGTH);

  return Status;
}

EFI_STATUS
ConfigureSataAhci (
  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  UINT32                              RootComplexBar,
  UINT16                              GpioBase
  )
/**

  @brief
  Program AHCI PI register for Enabled ports
  Handle hotplug, and interlock switch setting,
  and config related GPIOs.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] GpioBase             GPIO base address of this PCH device

  @retval EFI_SUCCESS             The function completed successfully

--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  MemBaseAddress;
  UINT32                AhciBar;
  UINT32                CapRegister;
  UINT32                Cap2Register;
  UINT32                Data32And;
  UINT32                Data32Or;
  UINT32                DwordReg;
  UINT32                PxCMDRegister;
  UINT16                SataPortsEnabled;
  UINT8                 Index;
  UINTN                 PciD19F0RegBase;
  UINT16                WordReg;
  PCH_SATA_CONFIG       *SataConfig;
  UINT16                SataModeSelect;

  UINT8                 SataResetGpio[PCH_AHCI_MAX_PORTS] = {
    PCH_GPIO_SATA_PORT0_RESET,
    PCH_GPIO_SATA_PORT1_RESET
  };

  SataConfig      = PchPlatformPolicy->SataConfig;
  PciD19F0RegBase = MmPciAddress (0, PchPlatformPolicy->BusNumber, 19, 0, 0);
  SataModeSelect  = MmioRead16 (PciD19F0RegBase + R_PCH_SATA_MAP) & B_PCH_SATA_MAP_SMS_MASK;

  ///
  /// Allocate the AHCI BAR
  ///
  MemBaseAddress = 0x0ffffffff;
  Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateMaxAddressSearchBottomUp,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  N_PCH_SATA_ABAR_ALIGNMENT,  /// 2^11: 2K Alignment
                  V_PCH_SATA_ABAR_LENGTH,     /// 2K Length
                  &MemBaseAddress,
                  mImageHandle,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// Set the AHCI BAR
  ///
  AhciBar = (UINT32) MemBaseAddress;
  MmioWrite32 (PciD19F0RegBase + R_PCH_SATA_ABAR, AhciBar);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_ABAR),
    1,
    &AhciBar
  );

  ///
  /// Enable command register memory space decoding
  ///
  MmioOr16 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_COMMAND), (UINT16) B_PCH_SATA_COMMAND_MSE);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_COMMAND),
    1,
    (VOID *) (UINTN) (PciD19F0RegBase + R_PCH_SATA_COMMAND)
  );

  ///
  /// Assert if the memory data of AhciBar is invalid.
  ///
  ASSERT (MmioRead32 (AhciBar) != 0xFFFFFFFF);

  ///
  /// Get Port Settings
  ///
  SataPortsEnabled = MmioRead16 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_PCS));
  SataPortsEnabled &= (UINT16) (B_PCH_SATA_PCS_PORT5_EN |
                                B_PCH_SATA_PCS_PORT4_EN |
                                B_PCH_SATA_PCS_PORT3_EN |
                                B_PCH_SATA_PCS_PORT2_EN |
                                B_PCH_SATA_PCS_PORT1_EN |
                                B_PCH_SATA_PCS_PORT0_EN );

  ///
  /// Read the default value of the Host Capabilities Register
  /// NOTE: many of the bits in this register are R/WO (Read/Write Once)
  ///
  CapRegister = MmioRead32 (AhciBar + R_PCH_SATA_AHCI_CAP);
  ///
  /// PCH BIOS Spec Update Rev 0.6.1 Sighting #3432124: SATA Ports 0 & 1 May Hang in a Low Power State
  /// Clear SATA ABAR + 118h[26] and 198h[26]
  ///
  CapRegister &= (UINT32)~(B_PCH_SATA_AHCI_CAP_SMPS | B_PCH_SATA_AHCI_CAP_SSS | B_PCH_SATA_AHCI_CAP_SALP |
                           B_PCH_SATA_AHCI_CAP_PMS | B_PCH_SATA_AHCI_CAP_SSC | B_PCH_SATA_AHCI_CAP_PSC |
                           B_PCH_SATA_AHCI_CAP_EMS | B_PCH_SATA_AHCI_CAP_SXS);

  for (Index = 0; Index < PCH_AHCI_MAX_PORTS; Index++) {
    ///
    /// Check PCS.PxE to know if the SATA Port x is disabled.
    ///
    if ((SataPortsEnabled & (B_PCH_SATA_PCS_PORT0_EN << Index)) == 0) {
      continue;
    }

    if (SataConfig->PortSettings[Index].MechSw == PCH_DEVICE_ENABLE) {
      ///
      /// Mechanical Presence Switch is Enabled for at least one of the ports
      ///
      CapRegister |= B_PCH_SATA_AHCI_CAP_SMPS;
    }

    if ((SataConfig->PortSettings[Index].SpinUp == PCH_DEVICE_ENABLE) ||
        (SataConfig->PortSettings[Index].External == PCH_DEVICE_ENABLE)) {
      ///
      /// Set SSS (ABAR + 00h[27]) to enable SATA controller supports staggered
      /// spin-up on its ports, for use in balancing power spikes
      /// SATA Port Spin up is supported at least one of the ports
      /// If this is an extra eSATA port. It needs to be hotpluggable but it's usually empty anyway
      /// so LPM is not available but Listen Mode is available, so it will have good power management.
      ///
      CapRegister |= B_PCH_SATA_AHCI_CAP_SSS;
    }

    if (SataConfig->PortSettings[Index].External == PCH_DEVICE_ENABLE) {
      ///
      /// External SATA is supported at least one of the ports
      ///
      CapRegister |= B_PCH_SATA_AHCI_CAP_SXS;
    }
  }
  ///
  /// Enable Enabled SATA ports,
  /// If BIOS accesses any of the port specific AHCI address range before setting PI bit,
  /// BIOS is required to read the PI register before the initial write to the PI register.
  /// NOTE: many of the bits in this register are R/WO (Read/Write Once)
  ///
  Data32And = (UINT32) (~B_PCH_SATA_PORT_MASK);
  Data32Or  = (UINT32) (SataPortsEnabled);
  MmioAndThenOr32 (
    (UINTN) (AhciBar + R_PCH_SATA_AHCI_PI),
    Data32And,
    Data32Or
  );
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AhciBar + R_PCH_SATA_AHCI_PI),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
  );
  ///
  /// After BIOS issues initial write to this register, BIOS is requested to issue two
  /// reads to this register.
  ///
  Data32Or = MmioRead32 (AhciBar + R_PCH_SATA_AHCI_PI);
  S3BootScriptSaveMemPoll(
    EfiBootScriptWidthUint32,
    (UINTN) (AhciBar + R_PCH_SATA_AHCI_PI),
    &Data32Or,  /// BitMask
    &Data32Or,  /// BitValue
    1,          /// Duration
    1           /// LoopTimes
  );
  S3BootScriptSaveMemPoll(
    EfiBootScriptWidthUint32,
    (UINTN) (AhciBar + R_PCH_SATA_AHCI_PI),
    &Data32Or,  /// BitMask
    &Data32Or,  /// BitValue
    1,          /// Duration
    1           /// LoopTimes
  );

  ///
  /// Step 1
  /// Set PSC (ABAR + 00h[13]). This bit informs the Windows software driver that the AHCI
  /// Controller supports the partial low-power state.
  /// Set SSC (ABAR + 00h[14]). This bit informs the Windows software driver that the AHCI
  /// Controller supports the slumber low-power state.
  /// Set SALP (ABAR + 00h[26]) to enable Aggressive Link Power Management (LPM) support.
  ///
  CapRegister |= (B_PCH_SATA_AHCI_CAP_SSC | B_PCH_SATA_AHCI_CAP_PSC);

  if (SataConfig->SalpSupport == PCH_DEVICE_ENABLE) {
    CapRegister |= B_PCH_SATA_AHCI_CAP_SALP;
  }
  ///
  /// Support Command List Override & PIO Multiple DRQ Block & Native Command Queuing Acceleration
  ///
  CapRegister |= (B_PCH_SATA_AHCI_CAP_SCLO | B_PCH_SATA_AHCI_CAP_PMD | B_PCH_SATA_AHCI_CAP_SCQA);

  ///
  /// Configure for the max speed support: 1.5Gb/s or 3.0Gb/s.
  ///
  CapRegister &= ~B_PCH_SATA_AHCI_CAP_ISS_MASK;

  switch (SataConfig->SpeedSupport) {
    case PchSataSpeedSupportGen1:
      CapRegister |= (V_PCH_SATA_AHCI_CAP_ISS_1_5_G << N_PCH_SATA_AHCI_CAP_ISS);
      break;

    case PchSataSpeedSupportGen2:
      CapRegister |= (V_PCH_SATA_AHCI_CAP_ISS_3_0_G << N_PCH_SATA_AHCI_CAP_ISS);
      break;

    default:
      CapRegister |= (V_PCH_SATA_AHCI_CAP_ISS_3_0_G << N_PCH_SATA_AHCI_CAP_ISS);
      break;
  }

  ///
  /// Update the Host Capabilities Register and save for S3 resume
  /// NOTE: Many of the bits in this register are R/WO (Read/Write Once)
  ///
  MmioWrite32 (AhciBar + R_PCH_SATA_AHCI_CAP, CapRegister);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AhciBar + R_PCH_SATA_AHCI_CAP),
    1,
    &CapRegister
  );

  ///
  /// Set ABAR + 0x24[5]
  /// Set ABAR + 0x24[4]
  /// Set ABAR + 0x24[3]
  /// Update the Host Extended Capabilities Register and save for S3 resume
  /// NOTE: Many of the bits in this register are R/WO (Read/Write Once)
  ///
  //AMI_OVERRIDE - EIP149024 SATA Initialization Programming Requirements >>
  if(PchPlatformPolicy->SataOddPort != 2) //Port0 ODD or Port1 ODD
	  Cap2Register = (UINT32) (B_PCH_SATA_AHCI_CAP2_DESO | B_PCH_SATA_AHCI_CAP2_SADM | ~B_PCH_SATA_AHCI_CAP2_SDS);
  else
  //AMI_OVERRIDE - EIP149024 SATA Initialization Programming Requirements <<
	  Cap2Register = (UINT32) (B_PCH_SATA_AHCI_CAP2_DESO | B_PCH_SATA_AHCI_CAP2_SADM | B_PCH_SATA_AHCI_CAP2_SDS);
  MmioWrite32 (AhciBar + R_PCH_SATA_AHCI_CAP2, Cap2Register);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (AhciBar + R_PCH_SATA_AHCI_CAP2),
    1,
    &Cap2Register
  );

  ///
  /// Port[Max:0] Command Register update
  /// NOTE: this register must be updated after Port Implemented and Capabilities register,
  /// Many of the bits in this register are R/WO (Read/Write Once)
  ///
  for (Index = 0; Index < PCH_AHCI_MAX_PORTS; Index++) {
    ///
    /// Check PCS.PxE to know if the SATA Port x is disabled.
    ///
    if ((SataPortsEnabled & (B_PCH_SATA_PCS_PORT0_EN << Index)) == 0) {
      continue;
    }
    ///
    /// Initial to zero first
    ///
    PxCMDRegister = 0;

    if (SataConfig->PortSettings[Index].HotPlug == PCH_DEVICE_ENABLE) {
      if (SataConfig->PortSettings[Index].External == PCH_DEVICE_DISABLE) {
        ///
        /// Hot Plug of this port is enabled
        ///
        PxCMDRegister |= B_PCH_SATA_AHCI_PxCMD_HPCP;
        if (SataConfig->PortSettings[Index].MechSw == PCH_DEVICE_ENABLE) {
          ///
          /// Mechanical Switch of this port is Attached
          ///
          PxCMDRegister |= B_PCH_SATA_AHCI_PxCMD_MPSP;
          ///
          /// Check the GPIO Pin is set as used for native function not a normal GPIO
          ///
          DwordReg = IoRead32 (
                       (UINTN)
                       (GpioBase + R_PCH_GPIO_SC_USE_SEL +
                        (SataResetGpio[Index] / 32 * (R_PCH_GPIO_SC_USE_SEL2 - R_PCH_GPIO_SC_USE_SEL))));
          if ((DwordReg & (1 << SataResetGpio[Index] % 32)) != 0) {
            DEBUG ((EFI_D_ERROR,
                    "BIOS must set the SATA%0xGP / GPIO%0d to native function if Mechanical Presence Switch is enabled!\n",
                    Index,
                    SataResetGpio[Index]));
            ASSERT_EFI_ERROR (EFI_DEVICE_ERROR);
          }
        }
      }
    } else {
      ///
      /// When "Mechanical Switch Attached to Port" (PxCMD[19]) is set, it is expected that HPCP (PxCMD[18]) is also set.
      ///
      if (SataConfig->PortSettings[Index].MechSw == PCH_DEVICE_ENABLE) {
        DEBUG ((EFI_D_ERROR, "Hot-Plug function of Port%d should be enabled while the Mechanical Presence Switch of it is enabled!\n",
                Index));
      }
    }

    if (SataConfig->PortSettings[Index].External == PCH_DEVICE_ENABLE) {
      PxCMDRegister |= B_PCH_SATA_AHCI_PxCMD_ESP;
    }

    if (SataConfig->PortSettings[Index].SpinUp == PCH_DEVICE_ENABLE) {
      PxCMDRegister |= B_PCH_SATA_AHCI_PxCMD_SUD;
    }

    ///
    /// eSATA port support only up to Gen2
    ///
    if (SataConfig->PortSettings[Index].External == PCH_DEVICE_ENABLE) {
      MmioAndThenOr32 (
        (UINTN) (AhciBar + (R_PCH_SATA_AHCI_P0SCTL + (0x80 * Index))),
        (UINT32)~(B_PCH_SATA_AHCI_PXSCTL_SPD),
        (UINT32) V_PCH_SATA_AHCI_PXSCTL_SPD_2
        );
      DwordReg = MmioRead32 (AhciBar + (R_PCH_SATA_AHCI_P0SCTL + (0x80 * Index)));
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (AhciBar + (R_PCH_SATA_AHCI_P0SCTL + (0x80 * Index))),
        1,
        &DwordReg
        );
    }

    MmioAndThenOr32 (
      (UINTN) (AhciBar + (R_PCH_SATA_AHCI_P0CMD + (0x80 * Index))),
      (UINT32)~(B_PCH_SATA_AHCI_PxCMD_MASK),
      (UINT32) PxCMDRegister
      );
    DwordReg = MmioRead32 (AhciBar + (R_PCH_SATA_AHCI_P0CMD + (0x80 * Index)));
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (AhciBar + (R_PCH_SATA_AHCI_P0CMD + (0x80 * Index))),
      1,
      &DwordReg
      );

#if SATA_DISDEVSLP_547178_WORKAROUND
	//Disable DevSleep
    MmioAnd32 (
      (UINTN) (AhciBar + (R_PCH_SATA_AHCI_PXDEVSLP0 + (0x80 * Index))),
      (UINT32)~(B_PCH_SATA_AHCI_PXDEVSLP1_DSP)
      );
    DwordReg = MmioRead32 (AhciBar + (R_PCH_SATA_AHCI_PXDEVSLP0 + (0x80 * Index)));
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (AhciBar + (R_PCH_SATA_AHCI_PXDEVSLP0 + (0x80 * Index))),
      1,
      &DwordReg
      );
#endif
  }

  ///
  /// PCH BIOS Spec Rev 0.5.0, Section 13.1.5.2 - Enable Flexible RAID OROM Features
  /// Valleyview with RAID capable sku is able to customize the RAID features through setting the
  /// Intel RST Feature Capabilities (RSTF) register before loading the RAID Option ROM. The RAID
  /// OROM will enable the desired features based on the setting in that register, please refer to
  /// PCH EDS for more details.
  /// For example, if the platform desired features are RAID0, RAID1, RAID5 and RAID10. System BIOS
  /// should set RSTF (ABAR + C8h [15:0]) to 002Fh before loading RAID OROM.
  ///
  WordReg = 0;

  if (SataConfig->HddUnlock == PCH_DEVICE_ENABLE) {
    ///
    /// If set to "1", indicates that the HDD password unlock in the OS is enabled
    /// while SATA is configured as RAID mode.
    ///
    WordReg |= B_PCH_SATA_AHCI_SFM_HDDLK;
  }

  if (SataConfig->LedLocate == PCH_DEVICE_ENABLE) {
    ///
    /// If set to "1", indicates that the LED/SGPIO hardware is attached and ping to
    /// locate feature is enabled on the OS while SATA is configured as RAID mode.
    ///
    WordReg |= B_PCH_SATA_AHCI_SFM_LEDL;
  }

  if (SataModeSelect == V_PCH_SATA_MAP_SMS_RAID) {
    if (SataConfig->Raid0 == PCH_DEVICE_ENABLE) {
      ///
      /// If set to "1", then RAID0 is enabled in Flexible RAID Option ROM.
      ///
      WordReg |= B_PCH_SATA_AHCI_SFM_R0E;
    }

    if (SataConfig->Raid1 == PCH_DEVICE_ENABLE) {
      ///
      /// If set to "1", then RAID1 is enabled in FD-OROM.
      ///
      WordReg |= B_PCH_SATA_AHCI_SFM_R1E;
    }

    if (SataConfig->Raid10 == PCH_DEVICE_ENABLE) {
      ///
      /// If set to "1", then RAID10 is enabled in FD-OROM.
      ///
      WordReg |= B_PCH_SATA_AHCI_SFM_R10E;
    }

    if (SataConfig->Raid5 == PCH_DEVICE_ENABLE) {
      ///
      /// If set to "1", then RAID5 is enabled in FD-OROM.
      ///
      WordReg |= B_PCH_SATA_AHCI_SFM_R5E;
    }

    if (SataConfig->Irrt == PCH_DEVICE_ENABLE) {
      ///
      /// If set to "1", then Intel Rapid Recovery Technology is enabled.
      ///
      WordReg |= B_PCH_SATA_AHCI_SFM_IRRT;
    }

    if (SataConfig->OromUiBanner == PCH_DEVICE_ENABLE) {
      ///
      /// If set to "1" then the OROM UI is shown.  Otherwise, no OROM banner or information
      /// will be displayed if all disks and RAID volumes are Normal.
      ///
      WordReg |= B_PCH_SATA_AHCI_SFM_OROMUNB;
    }

    if (SataConfig->IrrtOnly == PCH_DEVICE_ENABLE) {
      ///
      /// If set to "1", then only IRRT volumes can span internal and eSATA drives. If cleared
      /// to "0", then any RAID volume can span internal and eSATA drives.
      ///
      WordReg |= B_PCH_SATA_AHCI_SFM_IROES;
    }
  }

  ///
  /// RSTF(RST Feature Capabilities) is a Write-Once register.
  ///
  MmioWrite16 ((UINTN) (AhciBar + R_PCH_SATA_AHCI_SFM), WordReg);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (AhciBar + R_PCH_SATA_AHCI_SFM),
    1,
    &WordReg
  );

  ///
  /// Enable the HBA’s AHCI DMA to generate interrupt
  ///
  MmioOr8 ((UINTN) (AhciBar + R_PCH_SATA_AHCI_GHC), (UINT8) B_PCH_SATA_AHCI_GHC_IE);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint8,
    (UINTN) (AhciBar + R_PCH_SATA_AHCI_GHC),
    1,
    (VOID *) (UINTN) (AhciBar + R_PCH_SATA_AHCI_GHC)
  );

  ///
  /// Set Ahci Bar to zero
  ///
  AhciBar = 0;
  MmioWrite32 (PciD19F0RegBase + R_PCH_SATA_ABAR, AhciBar);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_ABAR),
    1,
    &AhciBar
  );

  ///
  /// Free the GCD pool
  ///
  gDS->FreeMemorySpace (
        MemBaseAddress,
        V_PCH_SATA_ABAR_LENGTH
        );

  return EFI_SUCCESS;
}
