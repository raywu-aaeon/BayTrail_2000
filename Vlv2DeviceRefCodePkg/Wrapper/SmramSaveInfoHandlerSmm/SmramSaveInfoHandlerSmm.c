/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/** @file
  A helper driver to save information to SMRAM after SMRR is enabled.

  This driver is for ECP platforms.

  Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#include <PiSmm.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Protocol/SmmSwDispatch.h>
#include <Protocol/SmmReadyToLock.h>
#include <Protocol/SmmControl.h>

#define SMM_FROM_SMBASE_DRIVER        0x55
#define SMM_FROM_CPU_DRIVER_SAVE_INFO 0x81

#define EFI_SMRAM_CPU_NVS_HEADER_GUID \
  { \
    0x429501d9, 0xe447, 0x40f4, 0x86, 0x7b, 0x75, 0xc9, 0x3a, 0x1d, 0xb5, 0x4e \
  }

UINT8    mSmiDataRegister;
BOOLEAN  mLocked = FALSE;
EFI_GUID mSmramCpuNvsHeaderGuid = EFI_SMRAM_CPU_NVS_HEADER_GUID;

/**
  Dispatch function for a Software SMI handler.

  @param  DispatchHandle        The handle of this dispatch function.
  @param  DispatchContext       The pointer to the dispatch function's context.
                                The SwSmiInputValue field is filled in
                                by the software dispatch driver prior to
                                invoking this dispatch function.
                                The dispatch function will only be called
                                for input values for which it is registered.

  @return None

**/
VOID
EFIAPI
SmramSaveInfoHandler (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  )
{
  EFI_STATUS Status;
  UINT64     VarData[3];
  UINTN      VarSize;

  ASSERT (DispatchContext != NULL);
  ASSERT (DispatchContext->SwSmiInputValue == SMM_FROM_SMBASE_DRIVER);

  if (!mLocked && IoRead8 (mSmiDataRegister) == SMM_FROM_CPU_DRIVER_SAVE_INFO) {
    VarSize = sizeof (VarData);
    Status = gRT->GetVariable (
                    L"SmramCpuNvs",
                    &mSmramCpuNvsHeaderGuid,
                    NULL,
                    &VarSize,
                    VarData
                    );
    if (!EFI_ERROR (Status) && VarSize == sizeof (VarData)) {
      CopyMem (
        (VOID *)(UINTN)(VarData[0]),
        (VOID *)(UINTN)(VarData[1]),
        (UINTN)(VarData[2])
        );
    }
  }
}

/**
  Smm Ready To Lock event notification handler.

  It sets a flag indicating that SMRAM has been locked.
  
  @param[in] Protocol   Points to the protocol's unique identifier.
  @param[in] Interface  Points to the interface instance.
  @param[in] Handle     The handle on which the interface was installed.

  @retval EFI_SUCCESS   Notification handler runs successfully.
 **/
EFI_STATUS
EFIAPI
SmmReadyToLockEventNotify (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      Handle
  )
{
  mLocked = TRUE;
  return EFI_SUCCESS;
}

/**
  Entry point function of this driver.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.  
  @param[in] SystemTable  A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
SmramSaveInfoHandlerSmmMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                    Status;
  EFI_SMM_SW_DISPATCH_PROTOCOL  *SmmSwDispatch;
  EFI_SMM_SW_DISPATCH_CONTEXT   SmmSwDispatchContext;
  EFI_HANDLE                    DispatchHandle;
  EFI_SMM_CONTROL_PROTOCOL      *SmmControl;
  EFI_SMM_CONTROL_REGISTER      SmmControlRegister;
  VOID                          *Registration;

  //
  // Get SMI data register
  //

  Status = SystemTable->BootServices->LocateProtocol (
                                        &gEfiSmmControlProtocolGuid,
                                        NULL,
                                        (VOID **)&SmmControl
                                        );
  ASSERT_EFI_ERROR (Status);
  Status = SmmControl->GetRegisterInfo (SmmControl, &SmmControlRegister);
  ASSERT_EFI_ERROR (Status);
  mSmiDataRegister = SmmControlRegister.SmiDataRegister;

  //
  // Register software SMI handler
  //

  Status = SystemTable->BootServices->LocateProtocol (
                                        &gEfiSmmSwDispatchProtocolGuid,
                                        NULL,
                                        (VOID **)&SmmSwDispatch
                                        );
  ASSERT_EFI_ERROR (Status);

  SmmSwDispatchContext.SwSmiInputValue = SMM_FROM_SMBASE_DRIVER;
  Status = SmmSwDispatch->Register (
                            SmmSwDispatch,
                            &SmramSaveInfoHandler,
                            &SmmSwDispatchContext,
                            &DispatchHandle
                            );
  ASSERT_EFI_ERROR (Status);

  //
  // Register SMM Ready To Lock Protocol notification
  //
  Status = gSmst->SmmRegisterProtocolNotify (
                    &gEfiSmmReadyToLockProtocolGuid,
                    SmmReadyToLockEventNotify,
                    &Registration
                    );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

