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

  IchTptPpmLib.h

Abstract:

  Header file for ICH power management functionality

--*/

#ifndef _ICHTPT_PPM_LIB_H_
#define _ICHTPT_PPM_LIB_H_

//
// Statements that include other files
//
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include "EfiScriptLib.h"
#include <Protocol/BootScriptSave/BootScriptSave.h>
#else
#include <PiDxe.h>
#endif
#include <IchPpmLib.h>
#include <PchAccess.h>
#ifndef ECP_FLAG
#include <Library/S3BootScriptLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Protocol/BootScriptSave.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#endif

#define B_GPIO_BM_BUSY                    (1 << 0)

//
// R_ICH_LPC_GEN_PMCON_1 (0xA0) Bit Definitions
//
#define B_ICH_LPC_LVL5_ENABLE             (1 << 11)

//#define PM_PM2_CNT                        0x20

#endif
