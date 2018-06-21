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
  PchInitPeim.c

  @brief
  The VLV Init PEIM implements the VLV PEI Init PPI.

**/
#include "PchInitPeim.h"
#include <Ppi/PchPeiInit.h>
#ifdef ECP_FLAG
EFI_GUID gPchInitPpiGuid = PCH_INIT_PPI_GUID;
EFI_GUID gPchPlatformPolicyPpiGuid = PCH_PLATFORM_POLICY_PPI_GUID;
EFI_GUID gPchUsbPolicyPpiGuid = PCH_USB_POLICY_PPI_GUID;
EFI_GUID gPchPeiInitPpiGuid = PCH_PEI_INIT_PPI_GUID;
#endif

#define SD_CARD_4684039_WORKAROUND              1
extern EFI_GUID gEfiSetupVariableGuid;
#define ANDROID 1
static EFI_PEI_PPI_DESCRIPTOR mPchPeiInitPpi[] = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPchPeiInitPpiGuid,
  NULL
};

//AMI_OVERRIDE+
static PCH_INIT_PPI               mPchInitPpi = {
  PchUsbInit
};

static EFI_PEI_PPI_DESCRIPTOR     mPpiListVariable = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPchInitPpiGuid,
  &mPchInitPpi
};
//AMI_OVERRIDE-

static EFI_PEI_NOTIFY_DESCRIPTOR  mNotifyList[] = {
  {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK,
    &gPchPlatformPolicyPpiGuid,
    PchInitialize
  },
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK),
    &gS3ResumeDonePpiGuid,
    PchS3ResumeDonePpiNotifyCallback
  },
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiMemoryDiscoveredPpiGuid,
    PchInitAfterMemInstall
  }
};




///
/// Functions
///
EFI_STATUS
EFIAPI
PchSataInit (
  IN  EFI_PEI_SERVICES            **PeiServices
  )
/**

  @brief
  Internal function performing SATA init needed in PEI phase

  @param[in] PeiServices          General purpose services available to every PEIM.

  @retval None

**/
{
  UINTN       PciD19F0RegBase;
  UINT32      Data32And;
  UINT32      Data32Or;
  EFI_STATUS  Status; //AMI_OVERRIDE - EIP149024 SATA Initialization Programming Requirements
  PCH_PLATFORM_POLICY_PPI   *PchPlatformPolicyPpi; //AMI_OVERRIDE - EIP149024 SATA Initialization Programming Requirements
  
  DEBUG ((EFI_D_INFO, "PchSataInit() - Start\n"));

  //AMI_OVERRIDE - EIP149024 SATA Initialization Programming Requirements >>
  ///
  /// Get platform policy settings through the PchPlatformPolicy PPI
  ///
  Status = (**PeiServices).LocatePpi (
                             PeiServices,
                             &gPchPlatformPolicyPpiGuid,
                             0,
                             NULL,
                             &PchPlatformPolicyPpi
                             );
  //AMI_OVERRIDE - EIP149024 SATA Initialization Programming Requirements <<
  
  PciD19F0RegBase = MmPciAddress (0,
                                  DEFAULT_PCI_BUS_NUMBER_PCH,
                                  PCI_DEVICE_NUMBER_PCH_SATA,
                                  PCI_FUNCTION_NUMBER_PCH_SATA,
                                  0
                                  );
  //
  // Return if SATA is fused off where SATA PCI DeviceID is 0XFFFF
  //
  if (MmioRead16 (PciD19F0RegBase) == 0xFFFF) {
    return EFI_SUCCESS;
  }

  ///
  /// Assert if any of SATA port 0 to port5 is enabled
  ///
  if ((MmioRead8 (PciD19F0RegBase + R_PCH_SATA_PCS) & (UINT8) (B_PCH_SATA_PCS_PORT5_EN |
                                                               B_PCH_SATA_PCS_PORT4_EN |
                                                               B_PCH_SATA_PCS_PORT3_EN |
                                                               B_PCH_SATA_PCS_PORT2_EN |
                                                               B_PCH_SATA_PCS_PORT1_EN |
                                                               B_PCH_SATA_PCS_PORT0_EN)) != 0) {
    DEBUG ((EFI_D_ERROR, "Please do not enable any SATA port before SATA DFT initialization done.\n"));
    ASSERT (0);
  }

  ///
  /// System BIOS must set D19:F0 + 0x94 [8:0] = 0x183 as part of the chipset initialization prior to
  /// SATA configuration. These bits should be restored while resuming from a S3 sleep state.
  ///
#if (_SLE_HYB_ || _SLE_COMP_)
  Data32And = 0;
  Data32Or  = 0x3c0001A3;
#else
  Data32And = (UINT32)~(BIT8 | BIT7 | BIT6 | BIT5 | BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
  Data32Or  = B_PCH_SATA_TM_NQIUFD | B_PCH_SATA_TM_SCTI | B_PCH_SATA_TM_RRSSEL;
#endif
  MmioAndThenOr32 (
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_TM),
    Data32And,
    Data32Or
  );

  ///
  /// D19:F0 + 0x92 [15] = 1b
  /// BIOS is requested to set "OOB Retry Mode" (ORM) bit to
  /// to enable the ASR (Asynchronous Signal Recovery) support in SATA HBA.
  /// These bit should be restored while resuming from a S3 sleep state.
  ///
#if (_SLE_HYB_ || _SLE_COMP_)
  MmioOr16 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_PCS), 0x8101);
#else
  MmioOr16 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_PCS), (UINT16) (B_PCH_SATA_PCS_OOB_RETRY));
#endif
  ///
  /// Program D19:F0 + 0x98 [29]   = 1b to have SATA IOSF Sideband Dynamic Clock Gating Enable
  /// Program D19:F0 + 0x98 [25]   = 1b to have Port Local RxStandby Power Staggering Enable
  /// Program D19:F0 + 0x98 [22]   = 1b to have SRAM Parity Check Disable for non-server platform
  /// Program D19:F0 + 0x98 [20]   = 1b to have Dynamic Squelch Detector during LPM Mechanism Disable
  /// Program D19:F0 + 0x98 [19]   = 1b to have Dynamic Squelch Detector Mechanism Disable
  /// Program D19:F0 + 0x98 [18]   = 1b to have ISM Extended IDLE Entry Delay Enable
  /// Program D19:F0 + 0x98 [12:7] = 04h as the ALIGN Detection Watchdog Timer Count
  /// Program D19:F0 + 0x98 [6:5]  = 1b to use no-periodic-dual-ALIGN as Unsquelch Indicator
  ///
#if (_SLE_HYB_ || _SLE_COMP_)
  Data32And = 0x0;
  Data32Or = 0x28000220;
#else
  Data32And = (UINT32) ~(B_PCH_SATA_TM2_ALDWTC | B_PCH_SATA_TM2_UNSQLIND);
  Data32Or  = (UINT32) (B_PCH_SATA_TM2_SIDECLKDCGEN |
                        B_PCH_SATA_TM2_PPST |
                        B_PCH_SATA_TM2_SPLDCGE |
                        B_PCH_SATA_TM2_SRAMPARDIS |
                        B_PCH_SATA_TM2_DSDLPMD |
                        B_PCH_SATA_TM2_DSDD |
                        B_PCH_SATA_TM2_ISMDLYEN |
                        BIT9 |
                        BIT5);
#endif
  
  //AMI_OVERRIDE - EIP149024 SATA Initialization Programming Requirements >>
  //
  //BWG Page 202, Doc# 514148 BYT-M/D BWG Vol2 Rev1p22,
  //Chap 35.1.5 ¡V ¡§SATA Initialization Programming Requirements¡¨
  //
  if(!EFI_ERROR(Status))
  {
    if(PchPlatformPolicyPpi->SataOddPort != 2)
  	  Data32Or |= B_PCH_SATA_TM2_PLLSHUTDIS;
  }
  //AMI_OVERRIDE - EIP149024 SATA Initialization Programming Requirements <<
  
  MmioAndThenOr32 (
    (UINTN) (PciD19F0RegBase + R_PCH_SATA_TM2),
    Data32And,
    Data32Or
    );

  ///
  /// Bus Master Enable
  /// Program D19:F0 + 0x04 [2] = 1b
  ///
  MmioOr8 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_COMMAND), (UINT8) (B_PCH_SATA_COMMAND_BME));

  ///
  /// Test Mode Register 3
  /// Program SATA SIR 0x70 = 0x00288301 at 25 MHz free running clock.
  ///
  MmioWrite8  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRI), (UINT8) 0x70);
  MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x00288301);

#if (_SLE_HYB_ || _SLE_COMP_)
  MmioWrite8  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRI), (UINT8) 0x40);
  MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x010000c3);
#endif
#if !(_SLE_HYB_ || _SLE_COMP_)
  ///
  /// Test Mode Register 4
  /// Program SATA SIR 0x54 = 0x00000300.
  ///
  MmioWrite8  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRI), (UINT8) 0x54);
  //AMI_OVERRIDE - EIP149024 SATA Initialization Programming Requirements >>
  //
  //BWG Page 202, Doc# 514148 BYT-M/D BWG Vol2 Rev1p22,
  //Chap 35.1.5 ¡V ¡§SATA Initialization Programming Requirements¡¨
  //
  if(!EFI_ERROR(Status))
  {
	  switch(PchPlatformPolicyPpi->SataOddPort)
	  {
	  	  case 0: //Port0
	  		  MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x00000301);
	  		  break;
	  	  case 1: //Port1
	  		  MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x00000302);
	  		  break;
	  	  default:
	  		  MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x00000300);
	  }
  }
  //AMI_OVERRIDE - EIP149024 SATA Initialization Programming Requirements <<

  ///
  /// Test Mode Register 5
  /// Program SATA SIR 0x58 = 0x50000000.
  ///
  MmioWrite8  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRI), (UINT8) 0x58);
  MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x50000000);
#endif
  ///
  /// OOB Detection Margin Register
  /// Program SATA SIR 0x6C = 0x130C0603 at 25 MHz free running clock.
  ///
  MmioWrite8  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRI), (UINT8) 0x6C);
  MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x130C0603);

  ///
  /// Gasket Control Register
  /// Program SATA SIR 0xF4 = 0x00
  ///
  MmioWrite8  ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRI), (UINT8) 0xF4);
  MmioWrite32 ((UINTN) (PciD19F0RegBase + R_PCH_SATA_SIRD), (UINT32) 0x00);

  ///
  /// Enable the SATA port0 and port1 only for VLV.
  ///
  MmioOr8 (
    PciD19F0RegBase + R_PCH_SATA_PCS,
#if (_SLE_HYB_ || _SLE_COMP_)
    (UINT8) (B_PCH_SATA_PCS_PORT0_EN)
#else
    (UINT8) (B_PCH_SATA_PCS_PORT1_EN | B_PCH_SATA_PCS_PORT0_EN)
#endif
    );

  DEBUG ((EFI_D_INFO, "PchSataInit() - End\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PchInitAfterMemInstall (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN  VOID                        *Ppi
  )
/*++

Routine Description:

  Internal function performing PCH init needed in PEI phase right after memory installed

Arguments:

  PeiServices             General purpose services available to every PEIM.
  NotifyDescriptor        The notification structure this PEIM registered on install.
  Ppi                     The memory discovered PPI.  Not used.

Returns:

  EFI_SUCCESS             The function completed successfully.

--*/
{
  EFI_STATUS  Status = EFI_SUCCESS;

  Status = PchSataInit (PeiServices);

  return Status;
}

EFI_STATUS
PchMiscInit (
  IN  PCH_PLATFORM_POLICY_PPI           *PchPlatformPolicyPpi
  )
/**

  @brief
  Internal function performing miscellaneous init needed in early PEI phase

  @param[in] PchPlatformPolicyPpi The PCH Platform Policy PPI instance

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_DEVICE_ERROR        Device error, aborts abnormally.

**/
{
  UINTN           PciD19F0RegBase;

  UINTN           PciD31F0RegBase;
  UINTN           RPBase;
  UINT8           Index;
  UINT16          Data16;
  UINT32          IlbBase;
  PCH_HPET_CONFIG *HpetConfig;

  DEBUG ((EFI_D_INFO, "PchMiscInit() - Start\n"));
  PciD19F0RegBase = MmPciAddress (0,
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_SATA,
                      PCI_FUNCTION_NUMBER_PCH_SATA,
                      0
                      );
  PciD31F0RegBase = MmPciAddress (0,
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_LPC,
                      PCI_FUNCTION_NUMBER_PCH_LPC,
                      0
                      );
  IlbBase    = MmioRead32 ((UINTN) (PciD31F0RegBase + R_PCH_LPC_ILB_BASE)) & B_PCH_LPC_ILB_BASE_BAR;
  HpetConfig = PchPlatformPolicyPpi->HpetConfig;
  ///
  /// Initial and enable HPET High Precision Timer memory address for basic usage
  ///
  if (HpetConfig->Enable == PCH_DEVICE_ENABLE) {
    ///
    /// Set HPET Timer enable to start counter spinning
    ///
    MmioOr32 (HpetConfig->Base + R_PCH_PCH_HPET_GCFG, B_PCH_PCH_HPET_GCFG_EN);
    MmioRead32 (HpetConfig->Base + R_PCH_PCH_HPET_GCFG); // Read back Posted Writes Register
  }

  ///
  /// Enable SATA function and read back to take effect
  ///
  MmioAnd32 (PchPlatformPolicyPpi->PmcBase + R_PCH_PMC_FUNC_DIS, (UINT32) (~B_PCH_PMC_FUNC_DIS_SATA));
  MmioRead32 (PchPlatformPolicyPpi->PmcBase + R_PCH_PMC_FUNC_DIS); // Read back Posted Writes Register

  if (PchPlatformPolicyPpi->SataMode == PchSataModeIde) {
    ///
    /// IDE mode
    ///
    MmioAnd8 (
      PciD19F0RegBase + R_PCH_SATA_MAP,
      (UINT8)~(B_PCH_SATA_MAP_SMS_MASK | B_PCH_SATA_PORT_TO_CONTROLLER_CFG)
      );

  } else {
    ///
    /// AHCI and RAID mode
    ///
    MmioOr8 (PciD19F0RegBase + R_PCH_SATA_MAP, (UINT8) (B_PCH_SATA_PORT_TO_CONTROLLER_CFG));

    if (PchPlatformPolicyPpi->SataMode == PchSataModeAhci) {
      MmioOr8 (PciD19F0RegBase + R_PCH_SATA_MAP, (UINT8) (V_PCH_SATA_MAP_SMS_AHCI));
    } else if (PchPlatformPolicyPpi->SataMode == PchSataModeRaid) {
      MmioOr8 (PciD19F0RegBase + R_PCH_SATA_MAP, (UINT8) (V_PCH_SATA_MAP_SMS_RAID));
    }
  }

  for (Index = 0; Index < PCH_PCIE_MAX_ROOT_PORTS; Index++) {
    RPBase = MmPciAddress (
               0,
               PchPlatformPolicyPpi->BusNumber,
               PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
               Index,
               0
               );
    ///
    /// VLV BIOS Spec Rev 0.5.0, Section 8.12
    /// NOTE: Detection of Non-Complaint PCI Express Devices in Gen2 Ports
    /// Some non-graphics PCI Express devices do not follow PCI Express Specification and currently report
    /// the incorrect Gen capability or link width. This may cause the improper detection of the card
    /// by the Intel Gen2 PCI Express port.
    /// The following settings may improve the ability of an Intel Gen2 PCI Express port to detect
    /// these non-compliant PCI Express devices.
    /// If BIOS cannot detect or train the device: Set B0:D28:F0~F3 + 0x70 [3:0]= 0x01
    /// Wait 100 ms for link to train up
    /// Please note the above setting is "as-is" as Intel cannot verify all non-compliant devices.
    /// You need to ensure that the workaround works with devices you are planning to use.
    ///
    switch (PchPlatformPolicyPpi->PcieConfig->PcieSpeed[Index]) {
      case PchPcieGen1:
        Data16 = BIT0;
        break;
      case PchPcieGen2:
      case PchPcieAuto:
      default:
        Data16 = BIT1;
        break;
    }
    if ((MmioRead16 (RPBase + R_PCH_PCIE_LCTL2_LSTS2) & (UINT16) (B_PCH_PCIE_LCTL2_LSTS2_TLS)) != Data16) {
      MmioAndThenOr16 (RPBase + R_PCH_PCIE_LCTL2_LSTS2, (UINT16)~(B_PCH_PCIE_LCTL2_LSTS2_TLS), Data16);
      PchPmTimerStall (100 * 1000);
    }

    ///
    /// VLV BIOS Spec Rev 0.5.0 Section 8.2
    /// Else if the port is hot-plug enable, do not disable the port. If BIOS wants to disable the port,
    /// BIOS should not enable the hot plug capability or must disable the hot plug capability of the port.
    /// Set B0:D28:Fn + 0x338 [26] = 0b at early POST.
    ///
    MmioAnd32 ((RPBase + R_PCH_PCIE_PCIEALC), (UINT32) ~B_PCH_PCIE_PCIEALC_BLKDQDA);
  }

  ///
  /// Route SCI IRQ to IRQ9.
  ///
  MmioAndThenOr32 (IlbBase + R_PCH_ILB_ACPI_CNT, (UINT32) (~B_PCH_ILB_ACPI_CNT_SCI_IRQ_SEL), (UINT32) V_PCH_ILB_ACPI_CNT_SCI_IRQ_9);

  DEBUG ((EFI_D_INFO, "PchMiscInit() - End\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
PchIoApicInit (
  IN  PCH_PLATFORM_POLICY_PPI     *PchPlatformPolicyPpi
  )
/**

  @brief
  Initialize IOAPIC according to IoApicConfig policy of the PCH
  Platform Policy PPI

  @param[in] PchPlatformPolicyPpi The PCH Platform Policy PPI instance

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_DEVICE_ERROR        Device error, aborts abnormally.

**/
{
  PCH_IOAPIC_CONFIG *IoApicConfig;
  UINT32            IoApicId;

  IoApicConfig    = PchPlatformPolicyPpi->IoApicConfig;

  DEBUG ((EFI_D_INFO, "PchIoApicInit() - Start\n"));
  ///
  /// VLV BIOS Spec Rev 0.6.2, Section 5.12
  /// If I/OxAPIC is been used in the system,
  /// it can be enabled by setting the APIC Enable bit, IBASE + 0x60 [8].
  /// The APIC Enable bit must be read back after it’s been written.
  /// IOxAPIC is enabled by default.
  ///
  MmioOr16 ((UINTN) (PchPlatformPolicyPpi->IlbBase + R_PCH_ILB_OIC), (UINT16) B_PCH_ILB_OIC_AEN);
  ///
  /// Reads back for posted write to take effect and make sure it is set properly.
  ///
  if ((MmioRead16 (PchPlatformPolicyPpi->IlbBase + R_PCH_ILB_OIC) & (UINT16) B_PCH_ILB_OIC_AEN) == (UINT16) 0x00)
    return EFI_DEVICE_ERROR;

  ///
  /// Get current IO APIC ID
  ///
  MmioWrite8 ((UINTN) R_PCH_IO_APIC_INDEX, R_PCH_IO_APIC_ID);
  IoApicId = MmioRead32 ((UINTN) R_PCH_IO_APIC_WINDOW) >> 24;
  ///
  /// IO APIC ID is at APIC Identification Register [27:24]
  ///
  if ((IoApicConfig->IoApicId != IoApicId) && (IoApicConfig->IoApicId < 0x10)) {
    ///
    /// Program APIC ID
    ///
    MmioWrite8 ((UINTN) R_PCH_IO_APIC_INDEX, 0);
    MmioWrite32 ((UINTN) R_PCH_IO_APIC_WINDOW, (UINT32) (IoApicConfig->IoApicId << 24));
  }

  DEBUG ((EFI_D_INFO, "PchIoApicInit() - End\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
PchPeimEhciPllCfg(
  IN  PCH_PLATFORM_POLICY_PPI           *PchPlatformPolicyPpi
  )
{
  UINT32                USBPllCfgs = 0;
  UINT32                Buffer32 = 0;

  DEBUG ((EFI_D_INFO, "PchPeimEhciPllCfg() SystemConfiguration->EhciPllCfgEnable = 0x%x \n",PchPlatformPolicyPpi->EhciPllCfgEnable));

  if(PchPlatformPolicyPpi->EhciPllCfgEnable) {

    /*
    (a) Bridge setting (using PythonSV):
    vlv.usb.ehci.usb_bridge.ch1ctrl=0x1
    vlv.usb.ehci.usb_bridge.portctrl0=0x1
    vlv.usb.ehci.usb_bridge.portctrl1=0x2
    vlv.usb.ehci.usb_bridge.portctrl2=0
    vlv.usb.ehci.usb_bridge.pmctl=0x9E  (IOSF PRI clock gating=0 due to bug)
    vlv.usb.ehci.usb_bridge.bdgdbgctl=0x7
    vlv.usb.ehci.usb_bridge.bdgctl=0x3
    */
    PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_CH1CTRL,
      Buffer32,
      ~(B_IOSF_SPXB_TCX1),
      BIT0,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );

    PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PORTCTL0,
      Buffer32,
      ~(B_IOSF_SPXB_PORT0_DEVICE_BW_TYPE),
      BIT0,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );

    PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PORTCTL1,
      Buffer32,
      ~(B_IOSF_SPXB_PORT1_DEVICE_BW_TYPE),
      BIT1,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );

    PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PORTCTL2,
      Buffer32,
      ~(B_IOSF_SPXB_PORT2_DEVICE_BW_TYPE),
      0x00,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );


    PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PMCTL,
      Buffer32,
      0x00,
      (B_IOSF_SPXB_PMCTL_CLK_GATE_EN |
       B_IOSF_SPXB_PMCTL_CLK_US_GATE_EN |
       B_IOSF_SPXB_PMCTL_CLK_DS_GATE_EN |
       B_IOSF_SPXB_PMCTL_IOSFSB_GATE_EN |
       B_IOSF_SPXB_PMCTL_IOSFSB_GATE_EN |
       B_IOSF_SPXB_PMCTL_SB_TK_GT_EN),
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );


    PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_BDGDBGCTL,
      Buffer32,
      0x00,
      0x07,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );

    PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_BDGCTL,
      Buffer32,
      0x00,
      (B_IOSF_SPXB_IOSF_PASS_THROUGH |
       B_IOSF_SPXB_PASS_THROUGH),
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );

    //Dump Msg Registers
    PchMsgBusRead32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_CH1CTRL,
      USBPllCfgs,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );
    DEBUG ((EFI_D_INFO, "PchPeimEhciPllCfg() R_IOSF_SPXB_CH1CTRL = 0x%x \n",USBPllCfgs));

    PchMsgBusRead32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PORTCTL0,
      USBPllCfgs,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );
    DEBUG ((EFI_D_INFO, "PchPeimEhciPllCfg() R_IOSF_SPXB_PORTCTL0 = 0x%x \n",USBPllCfgs));

    PchMsgBusRead32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PORTCTL1,
      USBPllCfgs,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );
    DEBUG ((EFI_D_INFO, "PchPeimEhciPllCfg() R_IOSF_SPXB_PORTCTL1 = 0x%x \n",USBPllCfgs));

    PchMsgBusRead32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PORTCTL2,
      USBPllCfgs,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );
    DEBUG ((EFI_D_INFO, "PchPeimEhciPllCfg() R_IOSF_SPXB_PORTCTL2 = 0x%x \n",USBPllCfgs));

    PchMsgBusRead32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PMCTL,
      USBPllCfgs,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );
    DEBUG ((EFI_D_INFO, "PchPeimEhciPllCfg() R_IOSF_SPXB_PMCTL = 0x%x \n",USBPllCfgs));
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PchInitialize (
  IN EFI_PEI_SERVICES             **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
  IN VOID                         *Ppi
  )
/**

  @brief
  This function performs basic initialization for PCH in PEI phase.
  If any of the base address arguments is zero, this function will disable the corresponding
  decoding, otherwise this function will enable the decoding.
  This function locks down the PMBase.

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] NotifyDescriptor     The notification structure this PEIM registered on install.
  @param[in] Ppi                  The memory discovered PPI.  Not used.

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_DEVICE_ERROR        Device error, aborts abnormally.

**/
{
#if (defined (PCH_DEBUG_INFO)) && (PCH_DEBUG_INFO == 1)
  UINT8                   Index;
#endif
  EFI_STATUS              Status;
  PCH_PLATFORM_POLICY_PPI *PchPlatformPolicyPpi;

  DEBUG ((EFI_D_INFO, "PchInitialize() - Start\n"));

  ///
  /// Get platform policy settings through the PchPlatformPolicy PPI
  ///
  Status = (**PeiServices).LocatePpi (
                             (CONST EFI_PEI_SERVICES **) PeiServices,
                             &gPchPlatformPolicyPpiGuid,
                             0,
                             NULL,
                             (VOID **) &PchPlatformPolicyPpi
                             );
  ASSERT_EFI_ERROR (Status);

#if (defined (PCH_DEBUG_INFO)) && (PCH_DEBUG_INFO == 1)
  DEBUG ((EFI_D_INFO, "\n------------------------ PchPlatformPolicyPpi Dump Begin -----------------\n"));
  DEBUG ((EFI_D_INFO, "Revision : 0x%x\n", PchPlatformPolicyPpi->Revision));
  DEBUG ((EFI_D_INFO, "BusNumber : 0x%x\n", PchPlatformPolicyPpi->BusNumber));
  DEBUG ((EFI_D_INFO, "SpiBase : 0x%x\n", PchPlatformPolicyPpi->SpiBase));
  DEBUG ((EFI_D_INFO, "PmcBase : 0x%x\n", PchPlatformPolicyPpi->PmcBase));
  DEBUG ((EFI_D_INFO, "IoBase : 0x%x\n", PchPlatformPolicyPpi->IoBase));
  DEBUG ((EFI_D_INFO, "IlbBase : 0x%x\n", PchPlatformPolicyPpi->IlbBase));
  DEBUG ((EFI_D_INFO, "MphyBase : 0x%x\n", PchPlatformPolicyPpi->MphyBase));
  DEBUG ((EFI_D_INFO, "Rcba : 0x%x\n", PchPlatformPolicyPpi->Rcba));
  DEBUG ((EFI_D_INFO, "AcpiBase : 0x%x\n", PchPlatformPolicyPpi->AcpiBase));
  DEBUG ((EFI_D_INFO, "GpioBase : 0x%x\n", PchPlatformPolicyPpi->GpioBase));

  DEBUG ((EFI_D_INFO, "PCH HPET Configuration --- \n"));
  DEBUG ((EFI_D_INFO, " Enable : 0x%x\n", PchPlatformPolicyPpi->HpetConfig->Enable));
  DEBUG ((EFI_D_INFO, " Base : 0x%x\n", PchPlatformPolicyPpi->HpetConfig->Base));

  DEBUG ((EFI_D_INFO, "PCH SATA Mode --- \n"));

  if (PchPlatformPolicyPpi->SataMode == PchSataModeIde) {
    DEBUG ((EFI_D_INFO, " SataMode : PchSataModeIde\n"));
  } else if (PchPlatformPolicyPpi->SataMode == PchSataModeAhci) {
    DEBUG ((EFI_D_INFO, " SataMode : PchSataModeAhci\n"));
  } else if (PchPlatformPolicyPpi->SataMode == PchSataModeRaid) {
    DEBUG ((EFI_D_INFO, " SataMode : PchSataModeRaid\n"));
  } else if (PchPlatformPolicyPpi->SataMode == PchSataModeMax) {
    DEBUG ((EFI_D_INFO, " SataMode : PchSataModeMax\n"));
  }

  DEBUG ((EFI_D_INFO, "PCH PCIE Speed--- \n"));
  for (Index = 0; Index < PCH_PCIE_MAX_ROOT_PORTS; Index++) {
    if (PchPlatformPolicyPpi->PcieConfig->PcieSpeed[Index] == PchPcieGen1) {
      DEBUG ((EFI_D_INFO, " PCIE Port %x Speed: PchPcieGen1\n", Index));
    } else if (PchPlatformPolicyPpi->PcieConfig->PcieSpeed[Index] == PchPcieGen2) {
      DEBUG ((EFI_D_INFO, " PCIE Port %x Speed: PchPcieGen2\n", Index));
    } else if (PchPlatformPolicyPpi->PcieConfig->PcieSpeed[Index] == PchPcieAuto) {
      DEBUG ((EFI_D_INFO, " PCIE Port %x Speed: PchPcieAuto\n", Index));
    }
  }

  DEBUG ((EFI_D_INFO, "PCH IO APIC Configuration --- \n"));
  DEBUG ((EFI_D_INFO, " IoApicId : 0x%x\n", PchPlatformPolicyPpi->IoApicConfig->IoApicId));

  DEBUG ((EFI_D_INFO, "\n------------------------ PchPlatformPolicyPpi Dump End -----------------\n"));
#endif
  ///
  /// Set and enable SPI Base
  ///
  ASSERT ((PchPlatformPolicyPpi->SpiBase & (UINT32) (~B_PCH_LPC_SPI_BASE_BAR)) == 0);
  MmioAndThenOr32 (
    MmPciAddress (0,
    PchPlatformPolicyPpi->BusNumber,
    PCI_DEVICE_NUMBER_PCH_LPC,
    PCI_FUNCTION_NUMBER_PCH_LPC,
    R_PCH_LPC_SPI_BASE),
    (UINT32) ~(B_PCH_LPC_SPI_BASE_BAR | B_PCH_LPC_SPI_BASE_EN),
    (UINT32) (PchPlatformPolicyPpi->SpiBase | B_PCH_LPC_SPI_BASE_EN)
    );

  ///
  /// Set and enable PMC Base
  ///
  ASSERT ((PchPlatformPolicyPpi->PmcBase & (UINT32) (~B_PCH_LPC_PMC_BASE_BAR)) == 0);
  MmioAndThenOr32 (
    MmPciAddress (0,
    PchPlatformPolicyPpi->BusNumber,
    PCI_DEVICE_NUMBER_PCH_LPC,
    PCI_FUNCTION_NUMBER_PCH_LPC,
    R_PCH_LPC_PMC_BASE),
    (UINT32) ~(B_PCH_LPC_PMC_BASE_BAR | B_PCH_LPC_PMC_BASE_EN),
    (UINT32) (PchPlatformPolicyPpi->PmcBase | B_PCH_LPC_PMC_BASE_EN)
    );

  ///
  /// Set and enable ILB Base
  ///
  ASSERT ((PchPlatformPolicyPpi->IlbBase & (UINT32) (~B_PCH_LPC_ILB_BASE_BAR)) == 0);
  MmioAndThenOr32 (
    MmPciAddress (0,
    PchPlatformPolicyPpi->BusNumber,
    PCI_DEVICE_NUMBER_PCH_LPC,
    PCI_FUNCTION_NUMBER_PCH_LPC,
    R_PCH_LPC_ILB_BASE),
    (UINT32) ~(B_PCH_LPC_ILB_BASE_BAR | B_PCH_LPC_ILB_BASE_EN),
    (UINT32) (PchPlatformPolicyPpi->IlbBase | B_PCH_LPC_ILB_BASE_EN)
    );

  ///
  /// Set and enable PUnit Base
  ///
  ASSERT ((PchPlatformPolicyPpi->PUnitBase& (UINT32) (~B_PCH_LPC_PUNIT_BASE_BAR)) == 0);
  MmioAndThenOr32 (
    MmPciAddress (0,
    PchPlatformPolicyPpi->BusNumber,
    PCI_DEVICE_NUMBER_PCH_LPC,
    PCI_FUNCTION_NUMBER_PCH_LPC,
    R_PCH_LPC_PUNIT_BASE),
    (UINT32) ~(B_PCH_LPC_PUNIT_BASE_BAR | B_PCH_LPC_PUNIT_BASE_EN),
    (UINT32) (PchPlatformPolicyPpi->PUnitBase | B_PCH_LPC_PUNIT_BASE_EN)
    );

  ///
  /// Set and enable IO Base
  ///
  ASSERT ((PchPlatformPolicyPpi->IoBase & (UINT32) (~B_PCH_LPC_IO_BASE_BAR)) == 0);
  MmioAndThenOr32 (
    MmPciAddress (0,
    PchPlatformPolicyPpi->BusNumber,
    PCI_DEVICE_NUMBER_PCH_LPC,
    PCI_FUNCTION_NUMBER_PCH_LPC,
    R_PCH_LPC_IO_BASE),
    (UINT32) ~(B_PCH_LPC_IO_BASE_BAR | B_PCH_LPC_IO_BASE_EN),
    (UINT32) (PchPlatformPolicyPpi->IoBase | B_PCH_LPC_IO_BASE_EN)
    );

  ///
  /// Set and enable MPHY Base
  ///
  ASSERT ((PchPlatformPolicyPpi->MphyBase & (UINT32) (~B_PCH_LPC_MPHY_BASE_BAR)) == 0);
  MmioAndThenOr32 (
    MmPciAddress (0,
    PchPlatformPolicyPpi->BusNumber,
    PCI_DEVICE_NUMBER_PCH_LPC,
    PCI_FUNCTION_NUMBER_PCH_LPC,
    R_PCH_LPC_MPHY_BASE),
    (UINT32) ~(B_PCH_LPC_MPHY_BASE_BAR | B_PCH_LPC_MPHY_BASE_EN),
    (UINT32) (PchPlatformPolicyPpi->MphyBase | B_PCH_LPC_MPHY_BASE_EN)
    );

  ///
  /// Set and enable RCBA Base
  ///
  ASSERT ((PchPlatformPolicyPpi->Rcba & (UINT32) (~B_PCH_LPC_RCBA_BAR)) == 0);
  MmioAndThenOr32 (
    MmPciAddress (0,
    PchPlatformPolicyPpi->BusNumber,
    PCI_DEVICE_NUMBER_PCH_LPC,
    PCI_FUNCTION_NUMBER_PCH_LPC,
    R_PCH_LPC_RCBA),
    (UINT32) ~(B_PCH_LPC_RCBA_BAR | B_PCH_LPC_RCBA_EN),
    (UINT32) (PchPlatformPolicyPpi->Rcba | B_PCH_LPC_RCBA_EN)
    );

  ///
  /// Set and enable ACPI Base
  ///
  ASSERT ((PchPlatformPolicyPpi->AcpiBase & (UINT16) (~B_PCH_LPC_ACPI_BASE_BAR)) == 0);
  MmioAndThenOr16 (
    MmPciAddress (0,
    PchPlatformPolicyPpi->BusNumber,
    PCI_DEVICE_NUMBER_PCH_LPC,
    PCI_FUNCTION_NUMBER_PCH_LPC,
    R_PCH_LPC_ACPI_BASE),
    (UINT16) ~(B_PCH_LPC_ACPI_BASE_BAR | B_PCH_LPC_ACPI_BASE_EN),
    (UINT16) (PchPlatformPolicyPpi->AcpiBase | B_PCH_LPC_ACPI_BASE_EN)
    );

  ///
  /// Set and enable GPIO Base
  ///
  ASSERT ((PchPlatformPolicyPpi->GpioBase & (UINT16) (~B_PCH_LPC_GPIO_BASE_BAR)) == 0);
  MmioAndThenOr16 (
    MmPciAddress (0,
    PchPlatformPolicyPpi->BusNumber,
    PCI_DEVICE_NUMBER_PCH_LPC,
    PCI_FUNCTION_NUMBER_PCH_LPC,
    R_PCH_LPC_GPIO_BASE),
    (UINT16) ~(B_PCH_LPC_GPIO_BASE_BAR | B_PCH_LPC_GPIO_BASE_EN),
    (UINT16) (PchPlatformPolicyPpi->GpioBase | B_PCH_LPC_GPIO_BASE_EN)
    );
  Status = PchMiscInit (PchPlatformPolicyPpi);
  ASSERT_EFI_ERROR (Status);

  Status = PchIoApicInit (PchPlatformPolicyPpi);
  ASSERT_EFI_ERROR (Status);


  Status = PchPeimEhciPllCfg (PchPlatformPolicyPpi);

  //
  // Install Ppi with VlvInitPeim complete
  //
  Status = (**PeiServices).InstallPpi ((CONST EFI_PEI_SERVICES **) PeiServices, mPchPeiInitPpi);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "PchInitialize() - End\n"));

  return Status;
}

VOID
PchMiscEarlyInit (
  IN UINT32  Pmcbase
  )
/*++

Routine Description:


Arguments:


Returns:


--*/
{
  UINT16          DataUint16;
  UINT8           DataUint8;
  //
  // CHV BIOS Specification 0.6.2 - Section 18.4.3, "Power Failure Consideration"
  //
  // When the RTC_PWR_STS bit is set, it indicates that the RTCRST# signal went low.
  // Software should clear this bit. Changing the RTC battery sets this bit.
  // System BIOS should reset CMOS to default values if the RTC_PWR_STS bit is set.
  //
  // CHV BIOS Specification Update 0.6.2
  // The System BIOS should execute the sequence below if the RTC_PWR_STS bit is set before memory initialization.
  // This will ensure that the RTC state machine has been initialized.
  //  1.  If the RTC_PWR_STS bit is set which indicates a new coin-cell battery insertion or a battery failure,
  //        steps 2 through 5 should be executed.
  //  2.  Set RTC Register 0x0A[6:4] to '110' or '111'.
  //  3.  Set RTC Register 0x0B[7].
  //  4.  Set RTC Register 0x0A[6:4] to '010'.
  //  5.  Clear RTC Register 0x0B[7].
  //
  DataUint16        = MmioRead16 (Pmcbase + R_PCH_PMC_GEN_PMCON_1);
  if ((DataUint16 & B_PCH_PMC_GEN_PMCON_RTC_PWR_STS) != 0) {
    //
    // Execute the sequence below. This will ensure that the RTC state machine has been initialized.
    //
    // Step 1.
    // BIOS clears this bit by writing a '0' to it.
    //
    if (DataUint16 & B_PCH_PMC_GEN_PMCON_RTC_PWR_STS) {
      //DataUint16 &= ~B_PCH_PMC_GEN_PMCON_RTC_PWR_STS;
      //MmioWrite16 ((Pmcbase + R_PCH_PMC_GEN_PMCON_1), DataUint16);

      //
      // Set to invalid date in order to reset the time to
      // BIOS build time later in the boot (SBRUN.c file).
      //
      IoWrite8 (R_PCH_RTC_INDEX2, R_PCH_RTC_YEAR);
      IoWrite8 (R_PCH_RTC_TARGET2, 0x0FF);
      IoWrite8 (R_PCH_RTC_INDEX2, R_PCH_RTC_MONTH);
      IoWrite8 (R_PCH_RTC_TARGET2, 0x0FF);
      IoWrite8 (R_PCH_RTC_INDEX2, R_PCH_RTC_DAYOFMONTH);
      IoWrite8 (R_PCH_RTC_TARGET2, 0x0FF);
      IoWrite8 (R_PCH_RTC_INDEX2, R_PCH_RTC_DAYOFWEEK);
      IoWrite8 (R_PCH_RTC_TARGET2, 0x0FF);
    }

    //
    // Step 2.
    // Set RTC Register 0Ah[6:4] to '110' or '111'.
    //
    IoWrite8 (R_PCH_RTC_INDEX2, R_PCH_RTC_REGISTERA);
    IoWrite8 (R_PCH_RTC_TARGET2, (V_PCH_RTC_REGISTERA_DV_DIV_RST1 | V_PCH_RTC_REGISTERA_RS_976P5US));

    //
    // Step 3.
    // Set RTC Register 0Bh[7].
    //
    IoWrite8 (R_PCH_RTC_INDEX2, R_PCH_RTC_REGISTERB);
    DataUint8 = (IoRead8 (R_PCH_RTC_TARGET2) | B_PCH_RTC_REGISTERB_SET);
    IoWrite8 (R_PCH_RTC_INDEX2, R_PCH_RTC_REGISTERB);
    IoWrite8 (R_PCH_RTC_TARGET2, DataUint8);

    //
    // Step 4.
    // Set RTC Register 0Ah[6:4] to '010'.
    //
    IoWrite8 (R_PCH_RTC_INDEX2, R_PCH_RTC_REGISTERA);
    IoWrite8 (R_PCH_RTC_TARGET2, (V_PCH_RTC_REGISTERA_DV_NORM_OP | V_PCH_RTC_REGISTERA_RS_976P5US));

    //
    // Step 5.
    // Clear RTC Register 0Bh[7].
    //
    IoWrite8 (R_PCH_RTC_INDEX2, R_PCH_RTC_REGISTERB);
    DataUint8 = (IoRead8 (R_PCH_RTC_TARGET2) & (UINT8)~B_PCH_RTC_REGISTERB_SET);
    IoWrite8 (R_PCH_RTC_INDEX2, R_PCH_RTC_REGISTERB);
    IoWrite8 (R_PCH_RTC_TARGET2, DataUint8);
  }

  return;
}

#if SD_CARD_4684039_WORKAROUND
VOID ProgramGpioSCForSDCardWA(
  IN CONST EFI_PEI_SERVICES            **PeiServices
  )
{
  EFI_PEI_STALL_PPI *StallPpi;
  EFI_STATUS        Status;
  UINT32                           VariableSize;
  SETUP_DATA             SystemConfig;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI  *Variable;

  VariableSize = sizeof (SETUP_DATA);
  ZeroMem (&SystemConfig, VariableSize);
  Status = (*PeiServices)->LocatePpi((CONST EFI_PEI_SERVICES **) PeiServices, &gEfiPeiReadOnlyVariable2PpiGuid, 0, NULL, (VOID **) &Variable);
  ASSERT_EFI_ERROR (Status);
  Status = Variable->GetVariable( Variable, L"Setup", &gEfiSetupVariableGuid, NULL, &VariableSize, &SystemConfig );

if (SystemConfig.OsSelect != ANDROID) {
  Status =(**PeiServices).LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, (VOID **) &StallPpi);
  ASSERT_EFI_ERROR (Status);
  //DEBUG ((EFI_D_ERROR, "ProgramGpioSC() - Start\n"));

  //
  // Workaround for sighting #4684039
  //1.  Enable GPIO mode of SDMMC3_PWR_EN_B.
  //    a.  vlv.gpio.gpscore.cfio_regs_pad_sdmmc3_pwr_en_b_pad_val.pad_val = 1'b1
  //    b.  vlv.gpio.gpscore.cfio_regs_pad_sdmmc3_pwr_en_b_pad_val.iienb = 1'b1  // Changed order
  //    c.  vlv.gpio.gpscore.cfio_regs_pad_sdmmc3_pwr_en_b_pad_val.ioutenb = 1'b0
  //    d.  vlv.gpio.gpscore.cfio_regs_pad_sdmmc3_pwr_en_b_pconf0.func_pin_mux = 3'b000
  //    e.  wait 40us
  //2.  Set SDMMC3_PWR_EN_B = 0 to turn on VSDIO
  //    a.  vlv.gpio.gpscore.cfio_regs_pad_sdmmc3_pwr_en_b_pad_val.pad_val = 1'b0
  //    b.  wait 40us
  //3.  Do one-time calibration of SD Card RCOMP, vlv.gpio.gpscore.cfio_regs_fam_c71p1cfiohvrscorepsdio3_fam_rcomp_cfg.ircforce
  //    a.  Write 0x00078480 to Register vlv.gpio.gpscore.cfio_regs_fam_c71p1cfiohvrscorepsdio3_fam_rcomp_cfg
  //    b.  Wait 10us
  //    c.  Write 0x00078080 to Register vlv.gpio.gpscore.cfio_regs_fam_c71p1cfiohvrscorepsdio3_fam_rcomp_cfg
  //4.  Turn off VSDIO
  //    a.  vlv.gpio.gpscore.cfio_regs_pad_sdmmc3_pwr_en_b_pad_val.pad_val = 1'b1
  //    b.  wait 40us
  //5.  Enable native mode of SDMMC3_PWR_EN_B
  //    a.  vlv.gpio.gpscore.cfio_regs_pad_sdmmc3_pwr_en_b_pad_val.ioutenb = 1'b1
  //    b.  vlv.gpio.gpscore.cfio_regs_pad_sdmmc3_pwr_en_b_pad_val.iienb = 1'b0
  //    c.  vlv.gpio.gpscore.cfio_regs_pad_sdmmc3_pwr_en_b_pconf0.func_pin_mux = 3'b001
  MmioWrite32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698,MmioRead32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698)|BIT0);
  MmioWrite32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698,MmioRead32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698)|BIT2);
  MmioWrite32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698,MmioRead32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698)& (~BIT1));
  MmioWrite32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x690,MmioRead32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x690)& 0XFFFFFFF8);
  StallPpi->Stall (PeiServices, StallPpi, 100);

  MmioWrite32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698,MmioRead32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698)&(~BIT0));
  StallPpi->Stall (PeiServices, StallPpi, 100);



  MmioWrite32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x830,0x00078480);
  StallPpi->Stall (PeiServices, StallPpi, 40);



  MmioWrite32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x830,0x00078080);

  MmioWrite32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698,MmioRead32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698)|BIT0);
  StallPpi->Stall (PeiServices, StallPpi, 100);

  MmioWrite32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698,MmioRead32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698)|BIT1);
  MmioWrite32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698,MmioRead32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x698)& (~BIT2));
  MmioWrite32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x690,(MmioRead32(IO_BASE_ADDRESS+GPIO_SCORE_OFFSET+0x690)& 0XFFFFFFF8) | BIT0);
}
  return;

}
#endif
///
/// Entry point
///
EFI_STATUS
EFIAPI
InstallPchInitPpi (
#ifdef ECP_FLAG
  IN  EFI_FFS_FILE_HEADER    *FileHandle,
  IN  EFI_PEI_SERVICES       **PeiServices
#else
  IN EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES    **PeiServices
#endif
  )
/**

  @brief
  Installs the PCH PEI Init PPI
  Performing Pch early init after PchPlatfromPolicy PPI produced

  @param[in] FfsHeader            Not used.
  @param[in] PeiServices          General purpose services available to every PEIM.

  @retval EFI_SUCCESS             The function completes successfully
  @retval EFI_OUT_OF_RESOURCES    Insufficient resources to create database

**/
{
  EFI_STATUS Status;
  UINT32     PmcBase;
  UINTN      PciD31F0RegBase;

  DEBUG ((EFI_D_ERROR, "InstallPchInitPpi() - Start\n"));
  ///
  /// Check if PmcBase has been set
  ///
  PciD31F0RegBase = MmPciAddress (0,
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_LPC,
                      PCI_FUNCTION_NUMBER_PCH_LPC,
                      0);
  PmcBase = MmioRead32 ((UINTN) (PciD31F0RegBase + R_PCH_LPC_PMC_BASE)) & B_PCH_LPC_PMC_BASE_BAR;
  DEBUG ((EFI_D_INFO, "PmcBase needs to be programmed and enabled before here.\n"));
  ASSERT ((PmcBase != 0) && (PmcBase != 0xFFFFFFFF));
  ///
  /// Perform miscellaneous init needed in very early PEI phase
  ///
  PchMiscEarlyInit (PmcBase);

#if SD_CARD_4684039_WORKAROUND
  //
  // Program GPIO SC
  //
  ProgramGpioSCForSDCardWA(PeiServices);
  DEBUG ((EFI_D_ERROR, "ProgramGpioSCForSDCardWA Done....\n"));
#endif
//AMI_OVERRIDE+
  ///
  /// Install the PCH PEI Init PPI
  ///
  Status = (**PeiServices).InstallPpi (PeiServices, &mPpiListVariable);
  ASSERT_EFI_ERROR (Status);
//AMI_OVERRIDE-

  ///
  /// Performing Pch early init after PchPlatfromPolicy PPI produced
  ///
  Status = (**PeiServices).NotifyPpi (PeiServices, &mNotifyList[0]);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_ERROR, "InstallPchInitPpi() - End\n"));

  return Status;
}

EFI_STATUS
EFIAPI
PchS3ResumeDonePpiNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  EFI_PEI_STALL_PPI             *StallPpi;
  EFI_STATUS                      Status;
  PCH_PLATFORM_POLICY_PPI *PchPlatformPolicyPpi;

  Status =(**PeiServices).LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, (VOID **) &StallPpi);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Get platform policy settings through the PchPlatformPolicy PPI
  ///
  Status = (**PeiServices).LocatePpi (
                             (CONST EFI_PEI_SERVICES **) PeiServices,
                             &gPchPlatformPolicyPpiGuid,
                             0,
                             NULL,
                             (VOID **) &PchPlatformPolicyPpi
                             );
  ASSERT_EFI_ERROR (Status);

  if (PchPlatformPolicyPpi->XhciWorkaroundSwSmiNumber == 0 ) {
  	  return EFI_SUCCESS;
  }
  
  IoWrite8(0xB2, PchPlatformPolicyPpi->XhciWorkaroundSwSmiNumber);

  StallPpi->Stall (PeiServices, StallPpi, 1);

  return EFI_SUCCESS;
}
