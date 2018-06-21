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
  PchPcieSmm.c

  @brief
  PCH Pcie SMM Driver Entry

--*/
#ifndef ECP_FLAG
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#endif

#include "PchPcieSmm.h"
#include <Protocol/PchPlatformPolicy.h>
#ifdef ECP_FLAG
EFI_GUID gEfiSmmIchnDispatchExProtocolGuid = EFI_SMM_ICHN_DISPATCH_EX_PROTOCOL_GUID;
EFI_GUID gDxePchPlatformPolicyProtocolGuid = DXE_PCH_PLATFORM_POLICY_PROTOCOL_GUID;
#else
#include <Library/BaseMemoryLib.h>
#endif

//
// Global variables
//
PCH_PCI_EXPRESS_CONFIG  *mPciExpressConfig;
PCH_PWR_OPT_CONFIG        *mPchPwrOptConfig;

EFI_STATUS
PchPcieSmi (
  IN  UINT8         PciePortNum
  )
/**

  @brief
  Program Common Clock and ASPM of Downstream Devices

  @param[in] PciePortNum          Pcie Root Port Number

  @retval EFI_SUCCESS             Function complete successfully

**/
{
  UINT32  Data32;
  UINT16  Data16;
  UINT8   SecBus;
  UINT8   SubBus;
  UINT8   Function;
  UINT8   Data8;
  UINT8   LTSM;
  UINT8   i;

  Function = mPciExpressConfig->RootPort[PciePortNum].FunctionNumber;
  ///
  /// Check for presence detect state
  ///
  Data16 = (UINT16) (MmioRead32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS, Function, R_PCH_PCIE_SLCTL_SLSTS)) >> 16);
  Data16 &= (B_PCH_PCIE_SLCTL_SLSTS_PDS >> 16);
  if (Data16 == 0) {
    ///
    /// Delay 20ms
    ///
    PchPmTimerStall (20 * 1000);
    ///
    /// Check for presence detect state again
    ///
    Data16 = (UINT16) (MmioRead32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS, Function, R_PCH_PCIE_SLCTL_SLSTS)) >> 16);
    Data16 &= (B_PCH_PCIE_SLCTL_SLSTS_PDS >> 16);
  }
  if (Data16) {
    ///
    /// Set speed to Gen 2
    ///
    MmioAndThenOr32 (
      MmPciAddress (0,
      0,
      PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
      Function,
      R_PCH_PCIE_LCTL2_LSTS2),
      (UINT32)~(B_PCH_PCIE_LCTL2_LSTS2_TLS),
      (UINT32) BIT1
      );
    ///
    /// Wait 300ms
    ///
    PchPmTimerStall (300 * 1000);
    ///
    /// Hot plug, check Link active
    ///
    LTSM = 0;
    for (i = 0; i < 5; i++) {
      Data8 = (UINT8) ( (MmioRead32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS, Function, R_PCH_PCIE_PCIESTS1)) & B_PCH_PCIE_PCIESTS1_LTSMSTATE) >> 24);
      if (Data8 > LTSM) {
        LTSM = Data8;
      }
    }
    ///
    /// If not active
    ///
    if (LTSM <= 0x8) {
      ///
      /// Downtrain to Gen 1
      ///
      MmioAndThenOr32 (
        MmPciAddress (0,
        0,
        PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
        Function,
        R_PCH_PCIE_LCTL2_LSTS2),
        (UINT32)~(B_PCH_PCIE_LCTL2_LSTS2_TLS),
        (UINT32) BIT0
        );
      ///
      /// Delay 100ms
      ///
      PchPmTimerStall (100 * 1000);
    }
    Data32  = MmioRead32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS, Function, R_PCH_PCIE_BNUM_SLT));
    SecBus  = (UINT8) ((Data32 & B_PCH_PCIE_BNUM_SLT_SCBN) >> 8);
    SubBus  = (UINT8) ((Data32 & B_PCH_PCIE_BNUM_SLT_SBBN) >> 16);
    PchPcieInitRootPortDownstreamDevices(0,PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,Function, SecBus,SubBus);
    Data32  = MmioRead32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS, Function, R_PCH_PCIE_BNUM_SLT));
    Data32  = (Data32 & (UINT32) ~(B_PCH_PCIE_BNUM_SLT_SBBN | B_PCH_PCIE_BNUM_SLT_SCBN));
    Data32  = (Data32 | (UINT32) ((SubBus << 16) & B_PCH_PCIE_BNUM_SLT_SBBN));
    Data32  = (Data32 | (UINT32) ((SecBus << 8) & B_PCH_PCIE_BNUM_SLT_SCBN));
    MmioWrite32 (MmPciAddress (0, 0, PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS, Function, R_PCH_PCIE_BNUM_SLT), Data32);
    PcieSetPm (
      0,
      PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
      Function,
      mPciExpressConfig->RootPort[Function].Aspm,
      mPciExpressConfig->NumOfDevAspmOverride,
      mPciExpressConfig->DevAspmOverride,
      mPciExpressConfig->TempRootPortBusNumMin,
      mPciExpressConfig->TempRootPortBusNumMax,
      mPchPwrOptConfig->NumOfDevLtrOverride,
      mPchPwrOptConfig->DevLtrOverride
      );
  }

  return EFI_SUCCESS;
}

VOID
PchPcieSmiHandlerFunction (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_EX_CONTEXT        *DispatchContext
  )
/**

  @brief
  PCIE Hotplug SMI call back function for each Root port

  @param[in] DispatchHandle       Handle of this dispatch function
  @param[in] DispatchContext      Pointer to the dispatch function's context.
                                  The DispatchContext fields are filled in by the dispatching driver
                                  prior to invoke this dispatch function

  @retval EFI_SUCCESS             Function complete successfully

**/
{
  PchPcieSmi ((UINT8) (DispatchContext->Type - IchnExPcie0Hotplug));
  return;
}

VOID
PchPcieLinkActiveStateChange (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_EX_CONTEXT        *DispatchContext
  )
/**

  @brief
  PCIE Link Active State Change Hotplug SMI call back function for all Root ports

  @param[in] DispatchHandle       Handle of this dispatch function
  @param[in] DispatchContext      Pointer to the dispatch function's context.
                                  The DispatchContext fields are filled in by the dispatching driver
                                  prior to invoke this dispatch function

  @retval EFI_SUCCESS             Function complete successfully

**/
{
  return;
}

EFI_STATUS
EFIAPI
InitializePchPcieSmm (
  IN      EFI_HANDLE            ImageHandle,
  IN      EFI_SYSTEM_TABLE      *SystemTable
  )
/**

  @brief
  Register PCIE Hotplug SMI dispatch function to handle Hotplug enabling

  @param[in] ImageHandle          The image handle of this module
  @param[in] SystemTable          The EFI System Table

  @retval EFI_SUCCESS             The function completes successfully

**/
{
  EFI_STATUS                            Status;
  UINT8                                 Index;
  UINT8                                 Data8;
  UINT32                                Data32;
  UINT32                                Data32Or;
  UINT32                                Data32And;
  UINTN                                 RPBase;
  UINTN                                 PciD31F0RegBase;
  UINT32                                PmcBase;
  EFI_HANDLE                            PcieHandle;
  static CONST EFI_SMM_ICHN_EX_SMI_TYPE PcieHandlerList[PCH_PCIE_MAX_ROOT_PORTS * 2] = {
    IchnExPcie0Hotplug,
    IchnExPcie1Hotplug,
    IchnExPcie2Hotplug,
    IchnExPcie3Hotplug,
    IchnExPcie0LinkActive,
    IchnExPcie1LinkActive,
    IchnExPcie2LinkActive,
    IchnExPcie3LinkActive
  };
  EFI_SMM_ICHN_DISPATCH_EX_PROTOCOL     *mIchnDispatch;
  EFI_SMM_ICHN_DISPATCH_EX_CONTEXT      PchPcieContext;
  UINTN                                 PortIndex;
  DXE_PCH_PLATFORM_POLICY_PROTOCOL      *PchPlatformPolicy;
  PCH_PCIE_DEVICE_LTR_OVERRIDE          *DevLtrOverrideTbl;
  UINT32                                TableSize;

  PciD31F0RegBase   = MmPciAddress (0,
                        DEFAULT_PCI_BUS_NUMBER_PCH,
                        PCI_DEVICE_NUMBER_PCH_LPC,
                        PCI_FUNCTION_NUMBER_PCH_LPC,
                        0
                      );
  PmcBase = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;

  DEBUG ((EFI_D_INFO, "InitializePchPcieSmm() Start\n"));
  DevLtrOverrideTbl = NULL;
  ///
  /// Locate SmmBase protocol
  ///

  //
  // Locate the ICHnEx Dispatch protocol
  //
  Status = gBS->LocateProtocol (&gEfiSmmIchnDispatchExProtocolGuid, NULL, (VOID **) &mIchnDispatch);
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (&gDxePchPlatformPolicyProtocolGuid, NULL, (VOID **) &PchPlatformPolicy);
  ASSERT_EFI_ERROR (Status);

  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    sizeof (PCH_PCI_EXPRESS_CONFIG),
                    (VOID **) &mPciExpressConfig
                    );
  ASSERT_EFI_ERROR (Status);
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    sizeof (PCH_PWR_OPT_CONFIG),
                    (VOID **) &mPchPwrOptConfig
                    );
  ASSERT_EFI_ERROR (Status);
  mPciExpressConfig->NumOfDevAspmOverride = PchPlatformPolicy->PciExpressConfig->NumOfDevAspmOverride;
  TableSize = PchPlatformPolicy->PciExpressConfig->NumOfDevAspmOverride * sizeof (PCH_PCIE_DEVICE_ASPM_OVERRIDE);

  ///
  /// Allocate and copy ASPM override table to SMM memory
  ///
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData,
                    TableSize,
                    (VOID **) &mPciExpressConfig->DevAspmOverride
                    );
  ASSERT_EFI_ERROR (Status);
  CopyMem (mPciExpressConfig->DevAspmOverride, PchPlatformPolicy->PciExpressConfig->DevAspmOverride, TableSize);

  ///
  /// Allocate and copy LTR override table to SMM memory
  ///
  if (PchPlatformPolicy->Revision >= DXE_PCH_PLATFORM_POLICY_PROTOCOL_REVISION_1) {
    mPchPwrOptConfig->NumOfDevLtrOverride = PchPlatformPolicy->PwrOptConfig->NumOfDevLtrOverride;
    DevLtrOverrideTbl = PchPlatformPolicy->PwrOptConfig->DevLtrOverride;
    if ((DevLtrOverrideTbl != NULL) && (PchPlatformPolicy->PwrOptConfig->NumOfDevLtrOverride != 0)) {
      TableSize = PchPlatformPolicy->PwrOptConfig->NumOfDevLtrOverride * sizeof (PCH_PCIE_DEVICE_LTR_OVERRIDE);
      Status = gSmst->SmmAllocatePool (
                        EfiRuntimeServicesData,
                        TableSize,
                        (VOID **) &mPchPwrOptConfig->DevLtrOverride
                        );
      ASSERT_EFI_ERROR (Status);
      CopyMem (mPchPwrOptConfig->DevLtrOverride, DevLtrOverrideTbl, TableSize);
    }
  }
  for (PortIndex = 0; PortIndex < PCH_PCIE_MAX_ROOT_PORTS; PortIndex++) {
    mPciExpressConfig->RootPort[PortIndex].Aspm                     = PchPlatformPolicy->PciExpressConfig->RootPort[PortIndex].Aspm;
    mPciExpressConfig->RootPort[PortIndex].FunctionNumber           = PchPlatformPolicy->PciExpressConfig->RootPort[PortIndex].FunctionNumber;
  }
  ///
  /// Locate the S3 resume scripting protocol
  ///
  Data32  = MmioRead32 (PmcBase + R_PCH_PMC_FUNC_DIS);
  Data32  = Data32 >> N_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0;
  for (Index = 0; Index < PCH_PCIE_MAX_ROOT_PORTS; Index++) {
    if (!(Data32 & (BIT0 << Index))) {
      RPBase = MmPciAddress (
                 0,
                 0,
                 PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
                 Index,
                 0
                 );
      Data8 = MmioRead8 (RPBase + R_PCH_PCIE_SLCAP);
      if (Data8 & B_PCH_PCIE_SLCAP_HPC) {
        PchPcieContext.Type = PcieHandlerList[Index];
        PcieHandle          = NULL;
        Status = mIchnDispatch->Register (
                                  mIchnDispatch,
                                  PchPcieSmiHandlerFunction,
                                  &PchPcieContext,
                                  &PcieHandle
                                  );
        ASSERT_EFI_ERROR (Status);

        PchPcieContext.Type = PcieHandlerList[Index + PCH_PCIE_MAX_ROOT_PORTS];
        Status = mIchnDispatch->Register (
                                  mIchnDispatch,
                                  PchPcieLinkActiveStateChange,
                                  &PchPcieContext,
                                  &PcieHandle
                                  );
        ASSERT_EFI_ERROR (Status);

        Data32Or  = B_PCH_PCIE_MPC_HPME;
        Data32And = (UINT32)~B_PCH_PCIE_MPC_HPME;
        S3BootScriptSaveMemReadWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (RPBase + R_PCH_PCIE_MPC),
          &Data32Or,  /// Data to be ORed
          &Data32And  /// Data to be ANDed
          );
      }
    }
  }

  DEBUG ((EFI_D_INFO, "InitializePchPcieSmm() End\n"));

  return EFI_SUCCESS;
}
