/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  2014 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PowerManagement.c

Abstract:

  Power management driver.
  This Dxe driver configures and supports the power management features
  for the platform.

--*/

//
// Statements that include other files
//
#include "PowerManagement.h"
#include <PchAccess.h>

//
// Global variables
//

//
// Desired platform policy information
//
PPM_PLATFORM_POLICY_PROTOCOL        mPpmPlatformPolicyProtocol;

//
// Driver entry point
//

EFI_STATUS
InitializePowerManagement (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Initialize the power management support.
  This function will do boot time configuration:
    Install S3 boot script to refill CPU Data.

Arguments:

  ImageHandle   - Pointer to the loaded image protocol for this driver
  SystemTable   - Pointer to the EFI System Table

Returns:

  EFI_SUCCESS   The driver installes/initialized correctly.
  Driver will ASSERT in debug builds on error.  PPM functionality is considered critical for mobile systems.

--*/
{
  EFI_STATUS                                Status;
  PPM_PLATFORM_POLICY_PROTOCOL              *PpmPlatformPolicyProtocolPointer;
  VOID                                      *Registration;
   
  //
  // Locate platform configuration information
  // Then copy it to a global variable that we can utilize during SMM/Runtime
  //
  Status = gBS->LocateProtocol (&gPpmPlatformPolicyProtocolGuid, NULL, (VOID **) &PpmPlatformPolicyProtocolPointer);
  ASSERT_EFI_ERROR (Status);
  CopyMem (&mPpmPlatformPolicyProtocol, PpmPlatformPolicyProtocolPointer, sizeof (PPM_PLATFORM_POLICY_PROTOCOL));

  
  ///
  /// Create an ExitPmAuth protocol call back event.
  ///

  EfiCreateProtocolNotifyEvent (
    &gExitPmAuthProtocolGuid,
    TPL_CALLBACK,
    PpmInitBeforeBoot,
    NULL,
    &Registration
  );

  return EFI_SUCCESS;
}

VOID
EFIAPI
PpmInitBeforeBoot (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
/**

  @brief
  Power Management initialization before Boot Script Table is closed

  @param[in] Event                A pointer to the Event that triggered the callback.
  @param[in] Context              A pointer to private data registered with the callback function.

  @retval EFI_SUCCESS             The function completed successfully

**/
{
	UINT8 Data8;
	
	Data8 = mPpmPlatformPolicyProtocol.S3RestoreMsrSwSmiNumber;
  S3BootScriptSaveIoWrite (
    EfiBootScriptWidthUint16,
    (UINTN) R_PCH_APM_CNT,
    1,
    &Data8
    );
}