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
  PchRootPorts.c

  @brief
  This file contains functions that initializes PCI Express Root Ports of PCH.

**/
#include "PchInit.h"

EFI_STATUS
SetInitRootPortDownstreamS3Item (
  IN UINT8                          RootPortBus,
  IN UINT8                          RootPortDevice,
  IN UINT8                          RootPortFunc,
  IN UINT8                          TempBusNumberMin,
  IN UINT8                          TempBusNumberMax
  )
/**

  @brief
  Set an Init Root Port Downstream devices S3 dispatch item, this function may assert if any error happend

  @param[in] RootPortBus          Pci Bus Number of the root port
  @param[in] RootPortDevice       Pci Device Number of the root port
  @param[in] RootPortFunc         Pci Function Number of the root port
  @param[in] TempBusNumberMin     Minimal temp bus number that can be assigned to the root port (as secondary
                                  bus number) and its down stream switches
  @param[in] TempBusNumberMax     Maximal temp bus number that can be assigned to the root port (as subordinate
                                  bus number) and its down stream switches

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  EFI_STATUS                                                  Status;
#ifndef ECP_FLAG
  EFI_BOOT_SCRIPT_SAVE_PROTOCOL                               *mBootScriptSave;
#endif
  STATIC EFI_PCH_S3_SUPPORT_PROTOCOL                          *PchS3Support;
  STATIC EFI_PCH_S3_PARAMETER_INIT_PCIE_ROOT_PORT_DOWNSTREAM  S3ParameterRootPortDownstream;
  STATIC EFI_PCH_S3_DISPATCH_ITEM                             S3DispatchItem = {
    PchS3ItemTypeInitPcieRootPortDownstream,
    &S3ParameterRootPortDownstream
  };
  EFI_PHYSICAL_ADDRESS                                        S3DispatchEntryPoint;

  if (!PchS3Support) {
    ///
    /// Get the PCH S3 Support Protocol
    ///
    Status = gBS->LocateProtocol (
                    &gEfiPchS3SupportProtocolGuid,
                    NULL,
                    (VOID **) &PchS3Support
                    );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  S3ParameterRootPortDownstream.RootPortBus       = RootPortBus;
  S3ParameterRootPortDownstream.RootPortDevice    = RootPortDevice;
  S3ParameterRootPortDownstream.RootPortFunc      = RootPortFunc;
  S3ParameterRootPortDownstream.TempBusNumberMin  = TempBusNumberMin;
  S3ParameterRootPortDownstream.TempBusNumberMax  = TempBusNumberMax;
  Status = PchS3Support->SetDispatchItem (
                           PchS3Support,
                           &S3DispatchItem,
                           &S3DispatchEntryPoint
                           );
  ASSERT_EFI_ERROR (Status);
  //
  // Save the script dispatch item in the Boot Script
  //
#ifdef ECP_FLAG
  S3BootScriptSaveDispatch( S3DispatchEntryPoint);
#else
  //S3BootScriptSaveDispatch((VOID *)(UINTN) S3DispatchEntryPoint);
  Status = gBS->LocateProtocol (
                  &gEfiBootScriptSaveProtocolGuid,
                  NULL,
                  (VOID **) &mBootScriptSave
                  );

  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    0,
                    EFI_BOOT_SCRIPT_DISPATCH_OPCODE,
                    S3DispatchEntryPoint
                    );
#endif

  return Status;
}

EFI_STATUS
PchInitRootPorts (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL           *PchPlatformPolicy,
  IN UINT32                                     RootComplexBar,
  IN UINT32                                     PmcBase,
  IN UINT16                                     AcpiBase,
  IN OUT UINT32                                 *FuncDisableReg
  )
/**

  @brief
  Perform Initialization of the Downstream Root Ports.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol
  @param[in] RootComplexBar       RCBA of the PCH
  @param[in] PmcBase              PmcBase of the PCH
  @param[in] AcpiBase             The ACPI I/O Base address of the PCH
  @param[in] FuncDisableReg       The function disable register. IN / OUT parameter.

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_INVALID_PARAMETER   The PCIe Root Port Number of D28:F0 is not found
                                  or invalid

**/
{
  EFI_STATUS              Status;
  UINT32                  Data32And;
  UINT32                  Data32Or;
  PCH_PCI_EXPRESS_CONFIG  *PciExpressConfig;
  UINT32                  RpEnableMask;
  UINT8                   PortIndex;
  UINTN                   RPBase;
  UINT32                  LoopTime;
  UINT32                  BitMask;
  UINT32                  BitValue;

  ///
  /// Whether a root port is hidden by another one with width > x1
  ///
  UINT32                  RpHiddenMask;

  DEBUG ((EFI_D_INFO, "PchInitRootPorts() Start\n"));

  Status            = EFI_SUCCESS;
  RpEnableMask      = 0;
  RpHiddenMask      = 0;
  PciExpressConfig  = PchPlatformPolicy->PciExpressConfig;
  Data32And         = 0xFFFFFFFF;
  Data32Or          = 0;

  ///
  /// PCH BIOS Spec Update Rev 0.6.2, Section 8.12
  /// Step 2
  /// Disable the remaining PCIe functions (F1~F3) if function 0 (F0) is disabled
  /// It is caller's responsibility to insure the root port that is mapped
  /// to function 0 is not disabled if there is any other root port enabled
  ///
  if ((PchPlatformPolicy->PciExpressConfig->RootPort[0].Enable == PCH_DEVICE_DISABLE) ||
      (((*FuncDisableReg) & B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0) == B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0)) {
    RpEnableMask = 0;
  } else {
    for (PortIndex = 0; PortIndex < PCH_PCIE_MAX_ROOT_PORTS; PortIndex++) {
      if ((PchPlatformPolicy->PciExpressConfig->RootPort[PortIndex].Enable) &&
          (((*FuncDisableReg) & (B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0 << PortIndex)) == 0)) {
        RpEnableMask |= 1 << PortIndex;
      }
    }
  }

  //
  //  WA for S3 resume fail when PCIE be set to 1x4 by SEC FW.
  //
  for (PortIndex = 0; PortIndex < PCH_PCIE_MAX_ROOT_PORTS; PortIndex++) {
    RPBase = MmPciAddress (0,
               PchPlatformPolicy->BusNumber,
               PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
               PortIndex,
               0
             );
    if (MmioRead16(RPBase) == 0xFFFF){
     *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0 << PortIndex;
     RpEnableMask &= ~(1 << PortIndex);
    }
  }

  ///
  /// If any PCIe Root Port is enabled, then the PCIe Root Port whose function number is 0
  /// will need to be enabled as well.
  ///
  if (RpEnableMask != 0) {
    RpEnableMask &= ~(RpHiddenMask); /// No ports to hide?
    RpEnableMask |= BIT0;
  }

  for (PortIndex = 0; PortIndex < PCH_PCIE_MAX_ROOT_PORTS; PortIndex++) {
    RPBase = MmPciAddress (0,
               PchPlatformPolicy->BusNumber,
               PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
               PortIndex,
               0
             );
    ///
    /// PCH BIOS Spec Rev 1.1.0 Section 8.2
    /// Else if the port is hot-plug enable, do not disable the port. If BIOS wants to disable the port,
    /// BIOS should not enable the hot plug capability or must disable the hot plug capability of the port.
    /// Set B0:D28:Fn + 338h [26] = 0b at early POST. Done in PchInitPeim.c PchMiscInit().
    ///
    /// Enabled Slot implemented for the enabled PCIE Root Ports. This is due to new PCIe disabling methodology to check if any
    /// to check if any is populated on the slots.
    ///
    if ((RpHiddenMask & (1 << PortIndex)) == 0) {
      MmioOr32 (RPBase + R_PCH_PCIE_CLIST_XCAP, B_PCH_PCIE_CLIST_XCAP_SI);
    }

    ///
    /// Initialize "Physical Slot Number" for Root Ports
    /// The System BIOS must assign a unique number to the Physical Slot Number field of
    /// the Slot Capabilities register, SLCAP (D28:F0-7:Reg 54h[31:19] of each available and
    /// enabled downstream root ports that implements a slot.
    /// Assign PSN using PortIndex.
    ///
//     MmioOr32 (RPBase + R_PCH_PCIE_SLCAP, (UINT32) (PortIndex << N_PCH_PCIE_SLCAP_PSN));

    if ((RpHiddenMask & (1 << PortIndex)) != 0) {
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0 << PortIndex;
    } else if (((RpEnableMask & (1 << PortIndex)) != 0) &&
               ((MmioRead32 (RPBase + R_PCH_PCIE_SLCTL_SLSTS) & B_PCH_PCIE_SLCTL_SLSTS_PDS) == 0) &&
               (PchPlatformPolicy->PciExpressConfig->RootPort[PortIndex].HotPlug == 0)) {
      ///
      /// PCH BIOS Spec Rev 0.5.0 Section 8.2
      /// Else if the port is not hot plug enable and no PCIe card is detected,
      /// Set B0:D28:Fn + 338h [26] = 1b
      /// Poll B0:D28:Fn + 328h [31:24] until one or else 50ms timeout
      /// Set B0:D28:Fn + 408h [27] = 1b
      /// Function disable the port at PBASE + 34h
      ///
      Data32And = 0xFFFFFFFF;
      Data32Or  = (UINT32) B_PCH_PCIE_PCIEALC_BLKDQDA;
      MmioOr32 ((RPBase + R_PCH_PCIE_PCIEALC), Data32Or);
      S3BootScriptSaveMemReadWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (RPBase + R_PCH_PCIE_PCIEALC),
        &Data32Or,  /// Data to be ORed
        &Data32And  /// Data to be ANDed
        );
      BitMask   = (UINT32) (B_PCH_PCIE_PCIESTS1_LTSMSTATE);
      BitValue  = 1 << 24;
      for (LoopTime = 0; LoopTime < 500; LoopTime++) {
        if ((MmioRead32 (RPBase + R_PCH_PCIE_PCIESTS1) & BitMask) == BitValue) {
          break;
        } else {
          PchPmTimerStall (100);
        }
      }
      S3BootScriptSaveMemPoll(
        EfiBootScriptWidthUint32,
        RPBase + 0x328,
        &BitMask,
        &BitValue,
        50,
        1000
        );
      Data32And = 0xFFFFFFFF;
      Data32Or  = (UINT32) B_PCH_PCIE_PHYCTL4_SQDIS;
      MmioOr32 ((RPBase + R_PCH_PCIE_PHYCTL4), Data32Or);
      S3BootScriptSaveMemReadWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (RPBase + R_PCH_PCIE_PHYCTL4),
        &Data32Or,  /// Data to be ORed
        &Data32And  /// Data to be ANDed
        );
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0 << PortIndex;
    } else if (((RpEnableMask & (1 << PortIndex)) == 0) &&
               ((MmioRead32 (RPBase + R_PCH_PCIE_SLCTL_SLSTS) & B_PCH_PCIE_SLCTL_SLSTS_PDS) != 0)) {
      ///
      /// Else if the port is not hot plug enable and a PCIe card is detected,
      /// and BIOS wants to disable the port
      /// link disable at B0:D28:Fn + 50h[4] followed by function disable the port at PBASE + 34h
      ///
      MmioOr16 ((RPBase + R_PCH_PCIE_LCTL_LSTS), (UINT16) B_PCH_PCIE_LCTL_LSTS_LD);
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0 << PortIndex;
    } else if (((RpEnableMask & (1 << PortIndex)) == 0) &&
               ((MmioRead32 (RPBase + R_PCH_PCIE_SLCTL_SLSTS) & B_PCH_PCIE_SLCTL_SLSTS_PDS) == 0)) {
      ///
      /// If the port does not own any lanes, function disable the port at PBASE + 34h.
      ///
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0 << PortIndex;
    } else {
      ///
      /// Configure the root ports
      ///
      Status = PchInitSingleRootPort (
                 (UINT8) PortIndex,
                 PchPlatformPolicy->PciExpressConfig->RootPort[PortIndex].FunctionNumber,
                 PchPlatformPolicy,
                 AcpiBase,
                 RootComplexBar,
                 PmcBase
                 );
      if (!EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, " Root Port %x device enabled. RpEnableMask: 0x%x\n", PortIndex + 1, RpEnableMask));
      }

      if ((PciExpressConfig->RootPort[PortIndex].TransmitterHalfSwing) &&
          (((MmioRead32 (RPBase + R_PCH_PCIE_PCIESTS1) & (B_PCH_PCIE_PCIESTS1_LNKSTAT)) >> 19) == 0x7)) {
        MmioOr8 (RPBase + R_PCH_PCIE_LCTL_LSTS, B_PCH_PCIE_LCTL_LSTS_LD);
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint8,
          (UINTN) (RPBase + R_PCH_PCIE_LCTL_LSTS),
          1,
          (VOID *) (UINTN) (RPBase + R_PCH_PCIE_LCTL_LSTS)
          );
        MmioOr16 (RPBase + R_PCH_PCIE_PWRCTL, B_PCH_PCIE_PWRCTL_TXSWING);
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint16,
          (UINTN) (RPBase + R_PCH_PCIE_PWRCTL),
          1,
          (VOID *) (UINTN) (RPBase + R_PCH_PCIE_PWRCTL)
          );
        MmioAnd8 (RPBase + R_PCH_PCIE_LCTL_LSTS, (UINT8) ~(B_PCH_PCIE_LCTL_LSTS_LD));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint8,
          (UINTN) (RPBase + R_PCH_PCIE_LCTL_LSTS),
          1,
          (VOID *) (UINTN) (RPBase + R_PCH_PCIE_LCTL_LSTS)
          );
      }
    }
    if ((RpHiddenMask & (1 << PortIndex)) == 0) {
      ///
      /// Disable the forwarding of EOI messages.
      /// Set B0:D28:F0/F1/F2/F3 + D4h [1] = 1b
      ///
      MmioOr8 (RPBase + R_PCH_PCIE_MPC2, (UINT8) B_PCH_PCIE_MPC2_EOIFD);
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint8,
        (UINTN) (RPBase + R_PCH_PCIE_MPC2),
        1,
        (VOID *) (UINTN) (RPBase + R_PCH_PCIE_MPC2)
        );
    }
  }
  if(((*FuncDisableReg >> N_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0) & 0x0F) != 0x0F ) {
    *FuncDisableReg &= ~B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0;
  }
  DEBUG ((EFI_D_INFO, "PchInitRootPorts() End\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
PchInitSingleRootPort (
  IN  UINT8                                     RootPort,
  IN  UINT8                                     RootPortFunction,
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL          *PchPlatformPolicy,
  IN  UINT16                                    AcpiBase,
  IN  UINT32                                    RootComplexBar,
  IN  UINT32                                    PmcBase
  )
/**

  @brief
  Perform Root Port Initialization.

  @param[in] RootPort             The root port to be initialized (zero based)
  @param[in] RootPortFunction     The PCI function number of the root port
  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol
  @param[in] AcpiBase             The ACPI I/O Base address of the PCH
  @param[in] RootComplexBar       RCBA of the PCH
  @param[in] PmcBase              PmcBase of the PCH

  @retval EFI_SUCCESS             Device found. The root port must be enabled.
  @retval EFI_NOT_FOUND           No device is found on the root port. It may be disabled.
  @exception EFI_UNSUPPORTED      Unsupported operation.

**/
{
  EFI_STATUS                        Status;
  UINTN                             RPBase;
  UINTN                             RPBase1;
  UINTN                             PciD31F0RegBase;
  UINT32                            CapOffset;
  UINT8                             BusNumber;
  UINT32                            Data32Or;
  UINT32                            Data32And;
  UINT16                            Data16Or;
  UINT16                            Data16And;
  PCH_PCI_EXPRESS_ROOT_PORT_CONFIG  *RootPortConfig;
  BOOLEAN                           DeviceFound;

  DeviceFound     = FALSE;
  RootPortConfig  = &PchPlatformPolicy->PciExpressConfig->RootPort[RootPort];
  BusNumber       = PchPlatformPolicy->BusNumber;
  RPBase          = MmPciAddress (0, BusNumber, PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS, RootPortFunction, 0);
  RPBase1         = MmPciAddress (0, BusNumber, PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS, PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_1, 0);
  PciD31F0RegBase = MmPciAddress (0, BusNumber, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC, 0);
  CapOffset = PcieFindCapId (
                BusNumber,
                PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
                RootPortFunction,
                0x10
                );

  if (CapOffset == 0) {
    return EFI_UNSUPPORTED;
  }

  ///
  /// PCH BIOS Spec Rev 0.5.5, Section 8.14.1 Power Optimizer Configuration
  ///
  /// Support Latency Tolerance Reporting (LTR)
  ///
  /// Program B0:D28:F0~F3 + 400h to 883C883Ch
  /// Done in PcieSetPm()
  ///
  /// Program B0:D28:F0~F3 + 404h [1:0] = 11b
  /// Done in PcieSetPm()
  ///
  /// Program B0:D28:F0-F3 + 64h [11] = 1b
  ///
  /// Not Support Optimized Buffer Flush/Fill (OBFF)
  ///
  /// Program B0:D28:F0-F7 + 64h [19:18] = 00h
  /// Program B0:D28:F0-F7 + 64h [11] = 0h
  ///
  Data32And = (UINT32) ~(B_PCH_PCIE_DCAP2_OBFFS |B_PCH_PCIE_DCAP2_LTRMS);
  MmioAnd32 (RPBase + R_PCH_PCIE_DCAP2, Data32And);
  S3BootScriptSaveMemReadWrite(
    EfiBootScriptWidthUint32,
    (UINTN) (RPBase + R_PCH_PCIE_DCAP2),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  ///
  /// Program B0:D28:F0-F7 + 68h [10] = 0b
  /// Program B0:D28:F0-F7 + 68h [14:13] = 00h
  ///
  Data32And = (UINT32) ~(B_PCH_PCIE_DCTL2_DSTS2_OBFFEN | B_PCH_PCIE_DCTL2_DSTS2_LTRME);
  MmioAnd32 (RPBase + R_PCH_PCIE_DCTL2_DSTS2, Data32And);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RPBase + R_PCH_PCIE_DCTL2_DSTS2),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  ///
  /// Set maximum payload size of 128B
  /// B0:D28:F0~F3 + 44h [2:0] = 0h
  ///
  Data32Or  = 0x00;
  Data32And = (UINT32) (~B_PCH_PCIE_DCAP_MPS);
  MmioAndThenOr32 (RPBase + R_PCH_PCIE_DCAP, Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RPBase + R_PCH_PCIE_DCAP),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  ///
  /// Set B0:D28:F0~F3 + F4h [27:26] = 0h
  /// Set B0:D28:F0~F3 + F4h [15:14] = 0h
  /// Set B0:D28:F0~F3 + F4h [13:12] = 0h
  /// Set B0:D28:F0 + F4h [8] = 1h
  ///
  Data32Or  = (UINT32) 0x00;
  Data32And = (UINT32) (~(B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_TDFT |
                          B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_TXCFGCHGWAIT |
                          B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_SIID));
  MmioAndThenOr32 (RPBase + R_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL, Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RPBase + R_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  ///
  /// If B0:D28:F0 + F4h [8] = 1b, set B0:D28:F0~F3 + 4Ch[17:15] = 100b
  /// Else set B0:D28:F0~F3 + 4Ch[17:15] = 010b
  ///
  if ((MmioRead16 (RPBase1 + R_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL) &
       (UINT16) B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_PXPPLLOFFEN) == (UINT16) B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_PXPPLLOFFEN) {
    Data32Or = BIT17;
  } else {
    Data32Or = BIT16;
  }
  Data32And = (UINT32) (~B_PCH_PCIE_LCAP_EL1);
  MmioAndThenOr32 (RPBase + R_PCH_PCIE_LCAP, Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RPBase + R_PCH_PCIE_LCAP),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  ///
  /// Program B0:D28:F0~F3 + 50h [6] = 1b
  ///
  Data16And = (UINT16) (-1);
  Data16Or  = (UINT16) (B_PCH_PCIE_LCTL_LSTS_CCC);
  MmioAndThenOr16 (RPBase + R_PCH_PCIE_LCTL_LSTS, Data16And, Data16Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (RPBase + R_PCH_PCIE_LCTL_LSTS),
    &Data16Or,  /// Data to be ORed
    &Data16And  /// Data to be ANDed
    );
  ///
  /// Set B0:D28:F0~F3 + 314h[31:24] = 74h
  /// Set B0:D28:F0~F3 + 314h[23:16] = 3Ah
  /// Set B0:D28:F0~F3 + 314h[15:08] = 36h
  /// Set B0:D28:F0~F3 + 314h[07:00] = 1Bh
  ///
  Data32Or  = 0x743A361B;
  Data32And = (UINT32) (0x0);
  MmioAndThenOr32 (RPBase + R_PCH_PCIE_PCIENFTS, Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RPBase + R_PCH_PCIE_PCIENFTS),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  ///
  /// Set B0:D28:F0~F3 + D8h[17:15] = 3h
  ///
  Data32And = (UINT32) (~B_PCH_PCIE_MPC_CCEL);
  Data32Or  = BIT16 | BIT15;
  MmioAndThenOr32 (RPBase + R_PCH_PCIE_MPC, Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RPBase + R_PCH_PCIE_MPC),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  ///
  /// Set B0:D28:F0~F3 + 33Ch[23:0] = 854C74h
  ///
  Data32And = 0xFF000000;
  Data32Or  = 0x854C74;
  MmioAndThenOr32 (RPBase + R_PCH_PCIE_PCIERTP, Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RPBase + R_PCH_PCIE_PCIERTP),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  ///
  /// Set B0:D28:F0~F3 + D4h[11] = 1b
  /// Set B0:D28:F0~F3 + D4h [6] = 1b
  ///
  Data16And = (UINT16) (-1);
  Data16Or  = (UINT16) (B_PCH_PCIE_MPC2_IPF | B_PCH_PCIE_MPC2_LSTP);
  MmioAndThenOr16 (RPBase + R_PCH_PCIE_MPC2, Data16And, Data16Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (RPBase + R_PCH_PCIE_MPC2),
    &Data16Or,  /// Data to be ORed
    &Data16And  /// Data to be ANDed
    );

  ///
  /// Set B0:D28:F0~F3 + D0h[24] = 0h
  /// Set B0:D28:F0~F3 + D0h[15] = 1h
  /// Set B0:D28:F0~F3 + D0h[14] = 1h
  ///
  Data32And = (UINT32) (~B_PCH_PCIE_CHCFG_UPSD);
  Data32Or  = (UINT32) (B_PCH_PCIE_CHCFG_UNRS | B_PCH_PCIE_CHCFG_UPRS);
  MmioAndThenOr32 (RPBase1 + R_PCH_PCIE_CHCFG, Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RPBase1 + R_PCH_PCIE_CHCFG),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  ///
  /// Set B0:D28:F0~F3 + 320h[22] = 1h
  /// Set B0:D28:F0~F3 + 320h[8:6] = 011b
  ///
  Data32And = (UINT32) (~B_PCH_PCIE_PCIECFG2_LATGC);
  Data32Or  = (UINT32) (B_PCH_PCIE_PCIECFG2_CRSREN | BIT7 | BIT6);
  MmioAndThenOr32 (RPBase + R_PCH_PCIE_PCIECFG2, Data32And, Data32Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RPBase + R_PCH_PCIE_PCIECFG2),
    &Data32Or,  /// Data to be ORed
    &Data32And  /// Data to be ANDed
    );

  ///
  /// If there is no IOAPIC behind the root port, set EOI Forwarding Disable bit (B0:D28:F0-F3:D4h[1]) to 1b.
  /// Done in PchPciExpressHelpersLibrary.c PcieSetEoiFwdDisable()
  ///
  /// For systems that support Advanced Error Reporting set
  /// B0:D28:F0~F3:100h[19:0] = 10001h
  /// Else
  /// B0:D28:F0~F3:100h[19:0] = 0h
  ///
  if (RootPortConfig->AdvancedErrorReporting) {
    MmioWrite32 (RPBase + R_PCH_PCIE_AECH, (UINT32) (BIT16 | BIT0));
  } else {
    MmioWrite32 (RPBase + R_PCH_PCIE_AECH, (UINT32) (0x0));
  }

  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (RPBase + R_PCH_PCIE_AECH),
    1,
    (VOID *) (UINTN) (RPBase + R_PCH_PCIE_AECH)
    );
  ///
  /// System BIOS should initiate link retrain for all slots that has card populated after register restoration.
  /// Done in PchPciExpressHelpersLibrary.c PchPcieInitRootPortDownstreamDevices ()
  ///
  /// System BIOS should read and write back to capability register B0:D28:F0 offsets 34h, 40h,
  /// 80h and 90h after it has been configure or prior to boot
  /// Done in PchInit.c PciERWORegInit ()
  ///
  /// Configure Extended Synch
  ///
  if (RootPortConfig->ExtSync) {
    Data16And = (UINT16) (-1);
    Data16Or  = B_PCH_PCIE_LCTL_LSTS_ES;
  } else {
    Data16And = (UINT16) (~B_PCH_PCIE_LCTL_LSTS_ES);
    Data16Or  = 0;
  }

  MmioAndThenOr16 (RPBase + R_PCH_PCIE_LCTL_LSTS, Data16And, Data16Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (RPBase + R_PCH_PCIE_LCTL_LSTS),
    &Data16Or,  /// Data to be ORed
    &Data16And  /// Data to be ANDed
    );

  ///
  /// Configure Completion Timeout
  ///
  Data16And = (UINT16)~(B_PCH_PCIE_DCTL2_DSTS2_CTD | B_PCH_PCIE_DCTL2_DSTS2_CTV);
  Data16Or  = 0;
  if (RootPortConfig->CompletionTimeout == PchPciECompletionTO_Disabled) {
    Data16Or = B_PCH_PCIE_DCTL2_DSTS2_CTD;
  } else {
    switch (RootPortConfig->CompletionTimeout) {
      case PchPciECompletionTO_Default:
        Data16Or = V_PCH_PCIE_DCTL2_DSTS2_CTV_DEFAULT;
        break;

      case PchPciECompletionTO_16_55ms:
        Data16Or = V_PCH_PCIE_DCTL2_DSTS2_CTV_40MS_50MS;
        break;

      case PchPciECompletionTO_65_210ms:
        Data16Or = V_PCH_PCIE_DCTL2_DSTS2_CTV_160MS_170MS;
        break;

      case PchPciECompletionTO_260_900ms:
        Data16Or = V_PCH_PCIE_DCTL2_DSTS2_CTV_400MS_500MS;
        break;

      case PchPciECompletionTO_1_3P5s:
        Data16Or = V_PCH_PCIE_DCTL2_DSTS2_CTV_1P6S_1P7S;
        break;

      default:
        Data16Or = 0;
        break;
    }
  }

  MmioAndThenOr16 (RPBase + R_PCH_PCIE_DCTL2_DSTS2, Data16And, Data16Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (RPBase + R_PCH_PCIE_DCTL2_DSTS2),
    &Data16Or,  /// Data to be ORed
    &Data16And  /// Data to be ANDed
    );

  ///
  /// Set the Slot Implemented Bit.  Note that this must be set before
  /// presence is valid.
  /// The System BIOS must initialize the "Slot Implemented" bit of the PCI Express* Capabilities Register,
  /// XCAP D28:F0~7:Reg 42h[8] of each available and enabled downstream root port.
  /// Setting this bit will indicate that the PCI Express* link associated with this
  /// port is connected to a slot (as compared to being connected to an integrated
  /// device component).
  ///
  if (RootPortConfig->SlotImplemented) {
    ///
    /// Slot Implemented enabled earlier. Here will only save this register for enabled ports
    ///
    Data16Or  = BIT8;
    Data16And = 0xFFFF;
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint16,
      (UINTN) (RPBase + CapOffset + 2),
      &Data16Or,  /// Data to be ORed
      &Data16And  /// Data to be ANDed
      );
    ///
    /// For Root Port Slots Numbering on the CRBs.
    ///
    Data32Or  = 0;
    Data32And = (UINT32) (~(B_PCH_PCIE_SLCAP_SLV | B_PCH_PCIE_SLCAP_SLS | B_PCH_PCIE_SLCAP_PSN));
    ///
    /// Note: If Hot Plug is supported, then write a 1 to the Hot Plug Capable (bit6) and Hot Plug
    /// Surprise (bit5) in the Slot Capabilities register, D28:F0~7:Reg 54h. Otherwise,
    /// write 0 to the bits PCIe Hot Plug SCI Enable
    ///
    Data32And &= (UINT32) (~(B_PCH_PCIE_SLCAP_HPC | B_PCH_PCIE_SLCAP_HPS));
    if (RootPortConfig->HotPlug) {
      Data32Or |= B_PCH_PCIE_SLCAP_HPC | B_PCH_PCIE_SLCAP_HPS;
    }
    ///
    /// Get the width from LCAP
    /// Slot Type   X1  X4/X8   X16
    /// Default       10W    25W    75W
    /// The slot power consumption and allocation is platform specific. Please refer to the
    /// "PCI Express* Card Electromechanical (CEM) Spec" for details.
    /// bugbug what's the default setting for X2
    ///
    if ((((MmioRead32 (RPBase + R_PCH_PCIE_LCAP)) & B_PCH_PCIE_LCAP_MLW) >> 4) == 0x01) {
      Data32Or |= (UINT32) (100 << 7);
      Data32Or |= (UINT32) (1 << 15);
    } else if ((((MmioRead32 (RPBase + R_PCH_PCIE_LCAP)) & B_PCH_PCIE_LCAP_MLW) >> 4) >= 0x04) {
      Data32Or |= (UINT32) (250 << 7);
      Data32Or |= (UINT32) (1 << 15);
    }

    Data32Or |= (UINT32) (RootPortConfig->PhysicalSlotNumber << 19);
    MmioAndThenOr32 (RPBase + R_PCH_PCIE_SLCAP, Data32And, Data32Or);
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (RPBase + R_PCH_PCIE_SLCAP),
      &Data32Or,  /// Data to be ORed
      &Data32And  /// Data to be ANDed
      );
  }
  ///
  /// Initialize downstream devices
  ///
  Status = PchPcieInitRootPortDownstreamDevices (
             BusNumber,
             (UINT8) PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
             RootPortFunction,
             PchPlatformPolicy->PciExpressConfig->TempRootPortBusNumMin,
             PchPlatformPolicy->PciExpressConfig->TempRootPortBusNumMax
             );
  if (Status == EFI_SUCCESS) {
    DeviceFound = TRUE;
  } else {
    ///
    /// Disable the forwarding of EOI messages.
    /// Set B0:D28:F0/F1/F2/F3 + D4h [1] = 1b
    ///
    MmioOr8 (RPBase + R_PCH_PCIE_MPC2, (UINT8) B_PCH_PCIE_MPC2_EOIFD);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint8,
      (UINTN) (RPBase + R_PCH_PCIE_MPC2),
      1,
      (VOID *) (UINTN) (RPBase + R_PCH_PCIE_MPC2)
      );
  }
  ///
  /// Not checking the error status here - downstream device not present does not
  /// mean an error of this root port. Our return status of EFI_SUCCESS means this
  /// port is enabled and outer function depends on this return status to do
  /// subsequent initializations.
  ///
  Status = SetInitRootPortDownstreamS3Item (
             BusNumber,
             (UINT8) PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
             RootPortFunction,
             PchPlatformPolicy->PciExpressConfig->TempRootPortBusNumMin,
             PchPlatformPolicy->PciExpressConfig->TempRootPortBusNumMax
             );
  ASSERT_EFI_ERROR (Status);
  ///
  /// Additional configurations
  ///
  ///
  /// Configure Error Reporting policy in the Device Control Register
  ///
  Data16And = (UINT16) (~(B_PCH_PCIE_DCTL_DSTS_URE | B_PCH_PCIE_DCTL_DSTS_FEE | B_PCH_PCIE_DCTL_DSTS_NFE | B_PCH_PCIE_DCTL_DSTS_CEE));
  Data16Or  = 0;

  if (RootPortConfig->UnsupportedRequestReport) {
    Data16Or |= B_PCH_PCIE_DCTL_DSTS_URE;
  }

  if (RootPortConfig->FatalErrorReport) {
    Data16Or |= B_PCH_PCIE_DCTL_DSTS_FEE;
  }

  if (RootPortConfig->NoFatalErrorReport) {
    Data16Or |= B_PCH_PCIE_DCTL_DSTS_NFE;
  }

  if (RootPortConfig->CorrectableErrorReport) {
    Data16Or |= B_PCH_PCIE_DCTL_DSTS_CEE;
  }

  MmioAndThenOr16 (RPBase + R_PCH_PCIE_DCTL_DSTS, Data16And, Data16Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (RPBase + R_PCH_PCIE_DCTL_DSTS),
    &Data16Or,  /// Data to be ORed
    &Data16And  /// Data to be ANDed
    );

  ///
  /// Configure Interrupt / Error reporting in R_PCH_PCIE_RCTL
  ///
  Data16And = (UINT16) (~(B_PCH_PCIE_RCTL_PIE | B_PCH_PCIE_RCTL_SFE | B_PCH_PCIE_RCTL_SNE | B_PCH_PCIE_RCTL_SCE));
  Data16Or  = 0;

  if (RootPortConfig->PmeInterrupt) {
    Data16Or |= B_PCH_PCIE_RCTL_PIE;
  }

  if (RootPortConfig->SystemErrorOnFatalError) {
    Data16Or |= B_PCH_PCIE_RCTL_SFE;
  }

  if (RootPortConfig->SystemErrorOnNonFatalError) {
    Data16Or |= B_PCH_PCIE_RCTL_SNE;
  }

  if (RootPortConfig->SystemErrorOnCorrectableError) {
    Data16Or |= B_PCH_PCIE_RCTL_SCE;
  }

  MmioAndThenOr16 (RPBase + R_PCH_PCIE_RCTL, Data16And, Data16Or);
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (RPBase + R_PCH_PCIE_RCTL),
    &Data16Or,  /// Data to be ORed
    &Data16And  /// Data to be ANDed
    );

  ///
  /// Root PCI-E Power Management SCI Enable
  ///
  if (RootPortConfig->PmSci) {
    ///
    /// PCH BIOS Spec Rev 0.5.0 section 8.7.3 BIOS Enabling of Intel PCH PCI Express* PME SCI Generation
    /// Step 1
    /// Make sure that PME Interrupt Enable bit, D28:F0-7:Reg 5Ch[3] is cleared
    ///
    Data16And = (UINT16) (~B_PCH_PCIE_RCTL_PIE);
    Data16Or  = 0;
    MmioAnd16 (RPBase + R_PCH_PCIE_RCTL, Data16And);
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint16,
      (UINTN) (RPBase + R_PCH_PCIE_RCTL),
      &Data16Or,  /// Data to be ORed
      &Data16And  /// Data to be ANDed
      );

    ///
    /// Step 2
    /// Program Misc Port Config (MPC) register at PCI config space offset
    /// D8h as follows:
    /// Set Power Management SCI Enable bit, D28:F0~7:Reg D8h[31]
    /// Clear Power Management SMI Enable bit, D28:F0~7:Reg D8h[0]
    ///
    Data32And = (UINT32) (~B_PCH_PCIE_MPC_PMME);
    Data32Or  = B_PCH_PCIE_MPC_PMCE;
    MmioAndThenOr32 (RPBase + R_PCH_PCIE_MPC, Data32And, Data32Or);
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (RPBase + R_PCH_PCIE_MPC),
      &Data32Or,  /// Data to be ORed
      &Data32And  /// Data to be ANDed
      );

    ///
    /// Step 3
    /// Make sure GPE0 Register (ABase+20h[9]), PCI_EXP_STS is 0, clear it if not zero
    ///
    Data32Or = IoRead32 (AcpiBase + R_PCH_ACPI_GPE0a_STS);
    if ((Data32Or & B_PCH_ACPI_GPE0a_STS_PCI_EXP) != 0) {
      Data32Or = B_PCH_ACPI_GPE0a_STS_PCI_EXP;
      IoWrite32 (AcpiBase + R_PCH_ACPI_GPE0a_STS, Data32Or);
      S3BootScriptSaveIoWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (AcpiBase + R_PCH_ACPI_GPE0a_STS),
        1,
        &Data32Or
        );
    }
    ///
    /// Step 4
    /// Set BIOS_PCI_EXP_EN bit, PBASE:Reg 20[10],
    /// to globally enable the setting of the PCI_EXP_STS bit by a PCI Express* PME event.
    ///
    Data16Or = MmioRead16 (PmcBase + R_PCH_PMC_GEN_PMCON_2);
    if ((Data16Or & B_PCH_PMC_GEN_PMCON_BIOS_PCI_EXP_EN) == 0) {
      Data16And = 0xFFFF;
      Data16Or  = B_PCH_PMC_GEN_PMCON_BIOS_PCI_EXP_EN;
      MmioOr16 (PmcBase + R_PCH_PMC_GEN_PMCON_2, Data16Or);
      S3BootScriptSaveMemReadWrite (
        EfiBootScriptWidthUint16,
        (UINTN) (PmcBase + R_PCH_PMC_GEN_PMCON_2),
        &Data16Or,  /// Data to be ORed
        &Data16And  /// Data to be ANDed
        );
    }
  }

  if (RootPortConfig->HotPlug) {
    ///
    /// PCH BIOS Spec Rev 0.5.0 section 8.8.2.1
    /// Step 1
    /// Clear following status bits, by writing 1b to them, in the Slot
    /// Status register at offset 1Ah of PCI Express Capability structure:
    /// Attention Button Pressed (bit0)
    /// Presence Detect Changed (bit3)
    ///
    Data16And = 0xFFFF;
    Data16Or  = (BIT3 | BIT0);
    MmioOr16 (RPBase + CapOffset + 0x1A, Data16Or);
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint16,
      (UINTN) (RPBase + CapOffset + 0x1A),
      &Data16Or,  /// Data to be ORed
      &Data16And  /// Data to be ANDed
      );
    ///
    /// Step 2
    /// Program the following bits in Slot Control register at offset 18h
    /// of PCI Express* Capability structure:
    /// Attention Button Pressed Enable (bit0) = 1b
    /// Presence Detect Changed Enable (bit3) = 1b
    /// Hot Plug Interrupt Enable (bit5) = 0b
    ///
    Data16And = (UINT16) (~BIT5);
    Data16Or  = (BIT3 | BIT0);
    MmioAndThenOr16 (RPBase + CapOffset + 0x18, Data16And, Data16Or);
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint16,
      (UINTN) (RPBase + CapOffset + 0x18),
      &Data16Or,  /// Data to be ORed
      &Data16And  /// Data to be ANDed
      );
    ///
    /// Step 3
    /// Program Misc Port Config (MPC) register at PCI config space offset
    /// D8h as follows:
    /// Hot Plug SCI Enable (HPCE, bit30) = 1b
    /// Hot Plug SMI Enable (HPME, bit1) = 0b
    ///
    Data32And = (UINT32) (~B_PCH_PCIE_MPC_HPME);
    Data32Or  = B_PCH_PCIE_MPC_HPCE;
    MmioAndThenOr32 (RPBase + R_PCH_PCIE_MPC, Data32And, Data32Or);
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (RPBase + R_PCH_PCIE_MPC),
      &Data32Or,  /// Data to be ORed
      &Data32And  /// Data to be ANDed
      );
    ///
    /// Step 4
    /// Clear GPE0 Register (ABase+20h), bit1, HOT_PLUG_STS by writing 1
    ///
    IoWrite32 (AcpiBase + R_PCH_ACPI_GPE0a_STS, (UINT32) B_PCH_ACPI_GPE0a_STS_HOT_PLUG);

    ///
    /// PCH BIOS Spec Rev 0.5.0 section 8.9
    /// BIOS should mask the reporting of Completion timeout (CT) errors by errors by setting
    /// the uncorrectable Error Mask register D28:F0~7:Reg 108h[14].
    ///
    Data32And = 0xFFFFFFFF;
    Data32Or  = B_PCH_PCIE_UEM_CT;
    MmioOr32 (RPBase + R_PCH_PCIE_UEM, Data32Or);
    S3BootScriptSaveMemReadWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (RPBase + R_PCH_PCIE_UEM),
      &Data32Or,  /// Data to be ORed
      &Data32And  /// Data to be ANDed
      );
  }

  if (DeviceFound == TRUE || (RootPortConfig->HotPlug == PCH_DEVICE_ENABLE)) {
    return EFI_SUCCESS;
  } else {
    return EFI_NOT_FOUND;
  }
}

EFI_STATUS
PcieEnableClockGating (
  IN  UINT8                                     BusNumber,
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL          *PchPlatformPolicy,
  IN  UINT32                                    RpEnableMask,
  IN  UINT32                                    RpHiddenMask
  )
/**

  @brief
  This is the function to enable the clock gating for PCI Express ports.

  @param[in] BusNumber            The Bus Number of the PCH device
  @param[in] PchPlatformPolicy    PCH Platform Policy protocol
  @param[in] RpEnableMask         Bit Mask indicating the enabled root ports
  @param[in] RpHiddenMask         Bit Mask indicating the root ports used for other > x1 root ports

  @retval EFI_SUCCESS             Successfully completed.

**/
{
  UINTN   RPBase;
  UINT32  PortIndex;
  UINT32  Data32Or;
  UINT32  Data32And;

  ///
  /// PCH BIOS Spec Rev 1.1.0 Section 18.10 Enabling Clock Gating
  /// 2.1
  /// For each enabled PCI Express* root port, program D28:F0~F3:Reg E1h[1:0] to 3h to enable dynamic clock gating.
  /// System BIOS also require to set D28:F0~F3:Reg E8h[0] = 1b
  /// 2.2
  /// Additionally, if port 0 is in x2 mode, these bits should not be set for port 1.
  /// Likewise, if port 0 is in x4 mode, these bits should not be set for ports 1, 2, or 3
  ///
  for (PortIndex = 0; PortIndex < PCH_PCIE_MAX_ROOT_PORTS; PortIndex++) {
    if (((RpEnableMask & (1 << PortIndex)) != 0) && ((RpHiddenMask & (1 << PortIndex)) == 0)) {
      RPBase = MmPciAddress (
                 0,
                 BusNumber,
                 PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
                 PchPlatformPolicy->PciExpressConfig->RootPort[PortIndex].FunctionNumber,
                 0
                 );

      Data32Or = B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_RPDLCGEN | B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_RPDBCGEN | B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_RPSCGEN;
      Data32And = (UINT32) (-1);
      MmioOr32 (
        RPBase + R_PCH_PCIE_RWC_RPDCGEN_RPPGEN,
        Data32Or
        );
      S3BootScriptSaveMemReadWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (RPBase + R_PCH_PCIE_RWC_RPDCGEN_RPPGEN),
        &Data32Or,   // Data to be ORed
        &Data32And   // Data to be ANDed
        );

      Data32And = 0xFFFFFFFF;
      Data32Or  = (UINT32) (B_PCH_PCIE_PWRCTL_RPL1SQPOL | B_PCH_PCIE_PWRCTL_RPDTSQPOL);
      MmioOr32 (RPBase + R_PCH_PCIE_PWRCTL, Data32Or);
      S3BootScriptSaveMemReadWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (RPBase + R_PCH_PCIE_PWRCTL),
        &Data32Or,  // Data to be ORed
        &Data32And  // Data to be ANDed
        );

      ///
      /// Step 2.6
      /// Set B0:D28:F0~F3 + 324h[5] = 1b
      ///
      MmioOr32 (RPBase + R_PCH_PCIE_PCIEDBG, (UINT32) (B_PCH_PCIE_PCIEDBG_SPCE));
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (RPBase + R_PCH_PCIE_PCIEDBG),
        1,
        (VOID *) (UINTN) (RPBase + R_PCH_PCIE_PCIEDBG)
        );
    }
  }
  ///
  /// PCH BIOS Spec Rev 1.1.0, Section 18.10
  /// Step 2.4
  /// Program D28:F0:Reg E1h[5:2] to 1111b
  ///
  RPBase = MmPciAddress (0,
            BusNumber,
            PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
            PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_1,
            0
            );
  Data32Or =
    (
      B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_LCLKREQEN |
      B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_BBCLKREQEN |
      B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_SRDLCGEN |
      B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_SRDBCGEN
    );
  MmioOr32 (RPBase + R_PCH_PCIE_RWC_RPDCGEN_RPPGEN,Data32Or);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (RPBase + R_PCH_PCIE_RWC_RPDCGEN_RPPGEN),
    1,
    (VOID *) (UINTN) (RPBase + R_PCH_PCIE_RWC_RPDCGEN_RPPGEN)
    );
  ///
  /// PCH BIOS Spec Rev 1.1.0 Section 18.10
  /// Step 2.5
  /// If PCIe root ports 0-3 are all disabled, set B0:D28:F0 + E2h [0] = 1b.
  /// If PCIe root ports 4-7 are all disabled, set B0:D28:F4 + E2h [0] = 1b.
  ///
  if (!(RpEnableMask & (0xF << PortIndex))) {
    MmioOr32 ((RPBase + R_PCH_PCIE_RWC_RPDCGEN_RPPGEN), B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_RPPGEN);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (RPBase + R_PCH_PCIE_RWC_RPDCGEN_RPPGEN),
      1,
      (VOID *) (UINTN) (RPBase + R_PCH_PCIE_RWC_RPDCGEN_RPPGEN)
      );
  }

  return EFI_SUCCESS;
}
