/*++

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PchInitPeim.c

Abstract:

  Do Early PCH platform initialization.



--*/

/*++
 This file contains an 'Intel Peripheral Driver' and is
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may
 be modified by the user, subject to additional terms of the
 license agreement
--*/
#include "PchAccess.h"
#include <Library/IoLib.h>
#include <token.h>


BOOLEAN
IsA16Inverted(
)
/*++

Routine Description:

  Returns the state of A16 inversion

Arguments:

Returns:

  TRUE - A16 is inverted
  FALSE - A16 is not inverted

--*/
{
    UINT8  Data;
    Data = MmioRead8(SB_RCBA + R_PCH_RCRB_GCS);
    return (Data & B_PCH_RCRB_GCS_TS) ? TRUE : FALSE;
}
