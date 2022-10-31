
/*++
  This file contains an 'Intel Peripheral Driver' and uniquely  
  identified as "Intel Reference Module" and is                 
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/

/*++

Copyright (c)  2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  SeCState.h

Abstract:

  Register Definitions for SeC States

--*/
#ifndef _SEC_STATE_H
#define _SEC_STATE_H
//
// Ignore SEC_FW_INIT_COMPLETE status Macro
//
#define SEC_STATUS_SEC_STATE_ONLY(a)  ((a) & (~(SEC_FW_INIT_COMPLETE | SEC_FW_BOOT_OPTIONS_PRESENT)))

//
// Macro to check if SEC FW INIT is completed
//
#define SEC_STATUS_IS_SEC_FW_INIT_COMPLETE(a) (((a) & SEC_FW_INIT_COMPLETE) == SEC_FW_INIT_COMPLETE)

//
// Marco to combind the complete bit to status
//
#define SEC_STATUS_WITH_SEC_INIT_COMPLETE(a)  ((a) | SEC_FW_INIT_COMPLETE)

//
// Macro to check SEC Boot Option Present
//
#define SEC_STATUS_IS_SEC_FW_BOOT_OPTIONS_PRESENT(a)  (((a) & SEC_FW_BOOT_OPTIONS_PRESENT) == SEC_FW_BOOT_OPTIONS_PRESENT)

//
// Abstract SEC Mode Definitions
//
#define SEC_MODE_NORMAL        0x00

#define SEC_DEBUG_MODE_ALT_DIS 0x02
#define SEC_MODE_TEMP_DISABLED 0x03
#define SEC_MODE_SECOVER       0x04
#define SEC_MODE_FAILED        0x06

//
// Abstract SEC Status definitions
//
#define SEC_READY                    0x00
#define SEC_INITIALIZING             0x01
#define SEC_IN_RECOVERY_MODE         0x02
#define SEC_DISABLE_WAIT             0x06
#define SEC_TRANSITION               0x07
#define SEC_NOT_READY                0x0F
#define SEC_FW_INIT_COMPLETE         0x80
#define SEC_FW_BOOT_OPTIONS_PRESENT  0x100
#define SEC_FW_UPDATES_IN_PROGRESS   0x200

#pragma pack()

#endif // SEC_STATE_H
