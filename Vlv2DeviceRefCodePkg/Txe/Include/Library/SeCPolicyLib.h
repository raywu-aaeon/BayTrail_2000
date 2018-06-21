/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2006 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  SeCPolicyLib.h

Abstract:

  Header file for SeC Policy functionality

--*/
#ifndef _SEC_POLICY_LIB_H_
#define _SEC_POLICY_LIB_H_

#include <Protocol/SeCPlatformPolicy.h>

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
;


BOOLEAN
SeCHECIEnabled (
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
;

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
;


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
;

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
;

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
;

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
;
#endif
