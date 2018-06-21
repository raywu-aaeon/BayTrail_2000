/*++
 This file contains an 'Intel Peripheral Driver' and is      
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may 
 be modified by the user, subject to additional terms of the 
 license agreement                                           
--*/
/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PeiKsc.h

Abstract:

  This file contains Multi Platform PPI information.

--*/
#ifndef _PEI_MULTIPLATFORM_PPI_H
#define _PEI_MULTIPLATFORM_PPI_H

//
// Include files
//
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/KscLib.h>

//
// SLE: RestrictedBegin
//
#ifndef SLE_FLAG
//
// SLE: RestrictedEnd
//
#ifndef STALL_ONE_MICRO_SECOND
#define STALL_ONE_MICRO_SECOND 1
#endif
//
// SLE: RestrictedBegin
//
#else // SLE_FLAG
#ifndef STALL_ONE_MICRO_SECOND
#define STALL_ONE_MICRO_SECOND 0
#endif
#endif // SLE_FLAG
//
// SLE: RestrictedEnd
//

#endif
