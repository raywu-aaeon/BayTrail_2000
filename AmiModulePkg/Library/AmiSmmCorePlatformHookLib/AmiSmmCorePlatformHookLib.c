/** @file
  Null instance of AmiSmmCorePlatformHookLib.

  Copyright (c) 2011, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/SmmCorePlatformHookLib.h>
#include <SmmPlatformeLinks.h>
#include <Dxe.h>
#include <AmiDxeLib.h>

/**
  Performs platform specific tasks before invoking registered SMI handlers.
  
  This function performs platform specific tasks before invoking registered SMI handlers.
  
  @retval EFI_SUCCESS       The platform hook completes successfully.
  @retval Other values      The paltform hook cannot complete due to some error.

**/

typedef VOID (PLATFORMHOOK_FOR_SMMDISPATCH_FUNCTION)();

extern PLATFORMHOOK_FOR_SMMDISPATCH_FUNCTION PLATFORMHOOK_BEFORE_SMMDISPATCH EndOfHookBeforeSmmDispatchFns;
extern PLATFORMHOOK_FOR_SMMDISPATCH_FUNCTION PLATFORMHOOK_AFTER_SMMDISPATCH EndOfHookAfterSmmDispatchFns;


PLATFORMHOOK_FOR_SMMDISPATCH_FUNCTION *HookBeforeSmmDispatchFns[] = {
    PLATFORMHOOK_BEFORE_SMMDISPATCH NULL
};

PLATFORMHOOK_FOR_SMMDISPATCH_FUNCTION *HookAfterSmmDispatchFns[] = {
    PLATFORMHOOK_AFTER_SMMDISPATCH NULL
};


EFI_STATUS
EFIAPI
PlatformHookBeforeSmmDispatch (
  VOID
  )
{
    UINT32 i;

    for(i = 0; HookBeforeSmmDispatchFns[i] != NULL; i++)
        HookBeforeSmmDispatchFns[i]();

    return EFI_SUCCESS;
}


/**
  Performs platform specific tasks after invoking registered SMI handlers.
  
  This function performs platform specific tasks after invoking registered SMI handlers.
  
  @retval EFI_SUCCESS       The platform hook completes successfully.
  @retval Other values      The paltform hook cannot complete due to some error.

**/
EFI_STATUS
EFIAPI
PlatformHookAfterSmmDispatch (
  VOID
  )
{
    UINT32 i;

    for(i = 0 ; HookAfterSmmDispatchFns[i] != NULL; i++)
        HookAfterSmmDispatchFns[i]();

    return EFI_SUCCESS;
}
