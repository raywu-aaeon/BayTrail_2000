/**
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
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
  PchBiosWriteProtect.c

  @brief
  PCH BIOS Write Protect Driver.

**/
#include "PchBiosWriteProtect.h"
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include <Protocol/SmmIchnDispatch/SmmIchnDispatch.h>
#include <Protocol/SmmSwDispatch/SmmSwDispatch.h>
#else
#include <Protocol/SmmIchnDispatch.h>
#include <Protocol/SmmSwDispatch.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#endif
#include <Protocol/PchPlatformPolicy.h>
#ifdef ECP_FLAG
EFI_GUID gDxePchPlatformPolicyProtocolGuid = DXE_PCH_PLATFORM_POLICY_PROTOCOL_GUID;
#else
#include <Library/UefiBootServicesTableLib.h>
#include <PchCommonDefinitions.h>
#endif

///
/// Global variables
///
EFI_SMM_ICHN_DISPATCH_PROTOCOL  *mIchnDispatch;
EFI_SMM_SW_DISPATCH_PROTOCOL    *mSwDispatch;
UINTN                           SpiBase;

static
VOID
PchBiosWpCallback (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
  )
/**

  @brief
  This hardware SMI handler will be run every time the BIOS Write Enable bit is set.

  @param[in] DispatchHandle       Not used
  @param[in] DispatchContext      Not used

  @retval None

**/
{
  ///
  /// Disable BIOSWE bit to protect BIOS
  ///
  MmioAnd8 ((UINTN) (SpiBase + R_PCH_SPI_BCR), (UINT8) ~B_PCH_SPI_BCR_BIOSWE);
}

VOID
PchBiosLockSwSmiCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  )
/**

  @brief
  Register an IchnBiosWp callback function to handle TCO BIOSWR SMI
  SMM_BWP and BLE bits will be set here

  @param[in] DispatchHandle       Not used
  @param[in] DispatchContext      Not used

  @retval None

**/
{
  EFI_STATUS                    Status;
  EFI_SMM_ICHN_DISPATCH_CONTEXT IchnContext;
  EFI_HANDLE                    IchnHandle;

  if (mIchnDispatch == NULL) {
    return;
  }

  IchnHandle = NULL;

  ///
  /// Set SMM_BWP bit before registering IchnBiosWp
  ///
  MmioOr8 ((UINTN) (SpiBase + R_PCH_SPI_BCR), (UINT8) B_PCH_SPI_BCR_SMM_BWP);
  ///
  /// Register an IchnBiosWp callback function to handle TCO BIOSWR SMI
  ///
  IchnContext.Type = IchnBiosWp;
  Status = mIchnDispatch->Register (
                            mIchnDispatch,
                            PchBiosWpCallback,
                            &IchnContext,
                            &IchnHandle
                            );
  ASSERT_EFI_ERROR (Status);
  ///
  /// Unregister BIOS Lock SW SMI handler since we do not need it now
  ///
  Status = mSwDispatch->UnRegister (
                          mSwDispatch,
                          DispatchHandle
                          );
  ASSERT_EFI_ERROR (Status);
}

EFI_STATUS
EFIAPI
InstallPchBiosWriteProtect (
  IN EFI_HANDLE            ImageHandle,
  IN EFI_SYSTEM_TABLE      *SystemTable
  )
/**

  @brief
  Entry point for Pch Bios Write Protect driver.

  @param[in] ImageHandle          Image handle of this driver.
  @param[in] SystemTable          Global system service table.

  @retval EFI_SUCCESS             Initialization complete.

**/
{
  EFI_STATUS                        Status;
  DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy;
  EFI_HANDLE                        SwHandle;
  EFI_SMM_SW_DISPATCH_CONTEXT       SwContext;
  UINTN                             mPciD31F0RegBase;

  ///
  /// Locate PCH Platform Policy protocol
  ///
  Status = gBS->LocateProtocol (&gDxePchPlatformPolicyProtocolGuid, NULL, (VOID **) &PchPlatformPolicy);
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Failed to locate PCH Policy protocol.\n"));
    return Status;
  }

  if (PchPlatformPolicy->LockDownConfig->BiosLock == PCH_DEVICE_ENABLE) {
    mPciD31F0RegBase = MmPciAddress (0,
                         DEFAULT_PCI_BUS_NUMBER_PCH,
                         PCI_DEVICE_NUMBER_PCH_LPC,
                         PCI_FUNCTION_NUMBER_PCH_LPC,
                         0
                       );
    SpiBase          = MmioRead32 (mPciD31F0RegBase + R_PCH_LPC_SPI_BASE) & B_PCH_LPC_SPI_BASE_BAR;
    ///
    /// Get the ICHn protocol
    ///
    mIchnDispatch = NULL;
    Status        = gBS->LocateProtocol (&gEfiSmmIchnDispatchProtocolGuid, NULL, (VOID **) &mIchnDispatch);
    ASSERT_EFI_ERROR (Status);
    ///
    /// Locate the ICH SMM SW dispatch protocol
    ///
    SwHandle  = NULL;
    Status    = gBS->LocateProtocol (&gEfiSmmSwDispatchProtocolGuid, NULL, (VOID **) &mSwDispatch);
    ASSERT_EFI_ERROR (Status);
    ///
    /// Register BIOS Lock SW SMI handler
    ///
    SwContext.SwSmiInputValue = PchPlatformPolicy->LockDownConfig->PchBiosLockSwSmiNumber;
    Status = mSwDispatch->Register (
                            mSwDispatch,
                            PchBiosLockSwSmiCallback,
                            &SwContext,
                            &SwHandle
                            );
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}
