/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
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
  This SMM driver configures and supports the power management features
  for the platform.

--*/

//
// Statements that include other files
//
#include "PowerManagement.h"

//
// Global variables
//

//
// FVID Table Information (Must be initialized by caller)
//
extern FVID_TABLE                   *mFvidPointer;

//
// PPM Processor support protocol
//
PPM_PROCESSOR_SUPPORT_PROTOCOL_2    *mPpmProcessorSupportProtocol = NULL;

//
// Desired platform policy information
//
PPM_PLATFORM_POLICY_PROTOCOL        mPpmPlatformPolicyProtocol;

//
// S3 resume scripting protocol
//
EFI_BOOT_SCRIPT_SAVE_PROTOCOL       *mBootScriptSave = NULL;

//
// Globals used by the reference code
//
EFI_GLOBAL_NVS_AREA                 *mGlobalNvsAreaPtr  = NULL;
UINT16                              mNumberOfStates     = 0;

//
// These are required if we choose to support Win2K and the IST applet
// If we want to disgorge IST applet support, remove IstApplet.h and IstApplet.c
// and delete this and code this enables.
//

//
// Driver entry point
//

VOID
S3RestoreMsrCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  )
/*++

Routine Description:

  PPM must restore runtime state of MSR.  This is not supported by the S3 boot script.
  In order to accomplish this, the ASL is modified to generate an SMI on S3 in the _WAK method.
  This SMI handler reponds to that SW SMI.

Arguments:

  DispatchHandle  - The handle of this callback, obtained when registering
  DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT

Returns:

  None.

--*/
{
  mPpmProcessorSupportProtocol->S3RestoreMsr (mPpmProcessorSupportProtocol);
}

EFI_STATUS
InitializePowerManagement (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Initialize the power management support.
  This function will do boot time configuration:
    Install into SMRAM/SMM
    Detect HW capabilities and SW configuration
    Initialize HW and software state (primarily MSR and ACPI tables)
    Install SMI handlers for runtime interfacess

Arguments:

  ImageHandle   - Pointer to the loaded image protocol for this driver
  SystemTable   - Pointer to the EFI System Table

Returns:

  EFI_SUCCESS   The driver installes/initialized correctly.
  Driver will ASSERT in debug builds on error.  PPM functionality is considered critical for mobile systems.

--*/
{
  EFI_STATUS                                Status;
  EFI_HANDLE                                Handle;
  EFI_HANDLE                                SwHandle;

  EFI_SMM_SW_DISPATCH_PROTOCOL              *SwDispatch;
  EFI_GLOBAL_NVS_AREA_PROTOCOL              *GlobalNvsAreaProtocol;
  EFI_SMM_SW_DISPATCH_CONTEXT               SwContext;

  PPM_PLATFORM_POLICY_PROTOCOL              *PpmPlatformPolicyProtocolPointer;
  //
  // GUID Definitions
  //
  EFI_GUID  mEfiGlobalNvsAreaProtocolGuid = EFI_GLOBAL_NVS_AREA_PROTOCOL_GUID;

  Handle              = NULL;
  SwHandle            = NULL;

  //
  // Locate the ICH SMM SW dispatch protocol
  //
  Status = gBS->LocateProtocol (&gEfiSmmSwDispatchProtocolGuid, NULL, (VOID **) &SwDispatch);
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize processor save state library
  //
  Status = InitializeProcessorSaveState ();
  ASSERT_EFI_ERROR (Status);

  //
  // Locate our shared data area
  //
  Status = gBS->LocateProtocol (&mEfiGlobalNvsAreaProtocolGuid, NULL, (VOID **) &GlobalNvsAreaProtocol);
  ASSERT_EFI_ERROR (Status);
  mGlobalNvsAreaPtr = GlobalNvsAreaProtocol->Area;

  //
  // Locate platform configuration information
  // Then copy it to a global variable that we can utilize during SMM/Runtime
  //
  Status = gBS->LocateProtocol (&gPpmPlatformPolicyProtocolGuid, NULL, (VOID **) &PpmPlatformPolicyProtocolPointer);
  ASSERT_EFI_ERROR (Status);
  CopyMem (&mPpmPlatformPolicyProtocol, PpmPlatformPolicyProtocolPointer, sizeof (PPM_PLATFORM_POLICY_PROTOCOL));

  //
  // Locate the S3 resume scripting protocol
  //
#ifdef ECP_FLAG
  Status = gBS->LocateProtocol (&gEfiBootScriptSaveGuid, NULL, (VOID **) &mBootScriptSave);
#else
  Status = gBS->LocateProtocol (&gEfiBootScriptSaveProtocolGuid, NULL, (VOID **) &mBootScriptSave);
#endif
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize the PPM processor support protocol
  // This is not currently a publicly exposed protocol, so the
  // library just returns an updated pointer to the protocol
  // rather then using locate protocol services.
  //

  mPpmProcessorSupportProtocol = NULL;
  InitializeValleyviewPowerManagementLib(&mPpmProcessorSupportProtocol);
  ASSERT (mPpmProcessorSupportProtocol != NULL);

  //
  // Processor support library may or may not provide a P state transition function
  // If it does not, we fill in a generic function
  //
  if (mPpmProcessorSupportProtocol->PStateTransition == NULL) {
    mPpmProcessorSupportProtocol->PStateTransition = PpmTransition;
  }

  InitializeIchLib ();
  InitializeMchLib ();
  InitializeAslUpdateLib ();

  //
  // Register ACPI S3 MSR restore handler
  //
  SwContext.SwSmiInputValue = mPpmPlatformPolicyProtocol.S3RestoreMsrSwSmiNumber;
  Status = SwDispatch->Register (
                         SwDispatch,
                         S3RestoreMsrCallback,
                         &SwContext,
                         &SwHandle
                         );
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize PPM code (determine HW and configured state, configure hardware and software accordingly
  //
  InitializePpm (mPpmProcessorSupportProtocol);

  return EFI_SUCCESS;
}
