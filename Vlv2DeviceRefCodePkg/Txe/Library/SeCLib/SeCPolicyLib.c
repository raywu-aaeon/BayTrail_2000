/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2006 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  SeCPolicyLib.c

Abstract:

  Implementation file for SeC Policy functionality

--*/

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include "SeCPolicyLib.h"

#include <Protocol/HECI.h>
#else
#include <Library/DebugLib.h>
#include <Library/SeCPolicyLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/Heci.h>
#endif
#include <SeCAccess.h>

//
// Global variables
//
DXE_SEC_POLICY_PROTOCOL  *mDxePlatformSeCPolicy = NULL;

EFI_STATUS
SeCPolicyLibInit (
  VOID
  )
/*++

Routine Description:

  Check if SeC is enabled.

Arguments:

  None.

Returns:

  None.

--*/
{
  EFI_STATUS  Status;

  //
  // Get the desired platform setup policy.
  //
  if (mDxePlatformSeCPolicy != NULL) {
    return EFI_SUCCESS;
  }
  Status = gBS->LocateProtocol (&gDxePlatformSeCPolicyGuid, NULL, (VOID **) &mDxePlatformSeCPolicy);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "No SEC Platform Policy Protocol available"));
  }

  ASSERT_EFI_ERROR (Status);

  return Status;
}

BOOLEAN
SeCHECIEnabled (
  VOID
  )
/*++

Routine Description:

  Check if HECI Communication is enabled in setup options.

Arguments:

  None.

Returns:

  FALSE                   HECI is disabled.
  TRUE                    HECI is enabled.

--*/
{
  BOOLEAN Supported;

  SeCPolicyLibInit ();
  if (mDxePlatformSeCPolicy->SeCConfig.HeciCommunication != 1) {
    Supported = FALSE;
  } else {
    Supported = TRUE;
  }
  return Supported;
}

BOOLEAN
SeCEndOfPostEnabled (
  VOID
  )
/*++

Routine Description:

  Check if End of Post Message is enabled in setup options.

Arguments:

  None.

Returns:

  FALSE                   EndOfPost is disabled.
  TRUE                    EndOfPost is enabled.

--*/
{
  BOOLEAN Supported;

  SeCPolicyLibInit ();
  if (mDxePlatformSeCPolicy->SeCConfig.EndOfPostEnabled != 1) {
    Supported = FALSE;
  } else {
    Supported = TRUE;
  }

  return Supported;
}


BOOLEAN
SeCTrEnabled (
  VOID
  )
/*++

Routine Description:

  Check if Thermal Reporting Message is enabled in setup options.

Arguments:

  None.

Returns:

  FALSE                   Thermal Reporting is disabled.
  TRUE                    Thermal Reporting is enabled.

--*/
{
  SeCPolicyLibInit ();
  if (mDxePlatformSeCPolicy->SeCConfig.TrConfig->TrEnabled == 1) {
    return TRUE;
  }

  return FALSE;
}

VOID
SeCReportError (
  SEC_ERROR_MSG_ID MsgId
  )
/*++

Routine Description:

  Show SeC Error message.

Arguments:

  MsgId   SeC error message ID.

Returns:

  None.

--*/
{
  SeCPolicyLibInit ();
  if (mDxePlatformSeCPolicy->Revision >= DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_3) {
    mDxePlatformSeCPolicy->SeCReportError (MsgId);
  }

  return ;
}

VOID
SeCPlatformHook (
  VOID
  )
/*++

Routine Description:

  None.

Arguments:

  MsgId   SeC error message ID.

Returns:

  None.

--*/
{
  SeCPolicyLibInit ();
  if (mDxePlatformSeCPolicy->Revision >= DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_3) {
    mDxePlatformSeCPolicy->SeCPlatformHook ();
  }

  return ;
}

BOOLEAN
SeCFwDowngradeSupported (
  VOID
  )
/*++

Routine Description:

  Check if SeCFwDowngrade is enabled in setup options.

Arguments:

  None.

Returns:

  FALSE                   SeCFwDowngrade is disabled.
  TRUE                    SeCFwDowngrade is enabled.

--*/
{
  SeCPolicyLibInit ();
  if (mDxePlatformSeCPolicy->Revision >= DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_7) {
    if (mDxePlatformSeCPolicy->SeCConfig.SeCFwDownGrade == 1) {
      return TRUE;
    }
  }

  return FALSE;
}
