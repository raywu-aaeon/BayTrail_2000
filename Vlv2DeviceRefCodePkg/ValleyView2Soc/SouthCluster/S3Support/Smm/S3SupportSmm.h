/** @file
  Header file for PCH SMM Handler

@copyright
  Copyright (c) 2015 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/

#ifndef _PCHLATEINITSMM_H_
#define _PCHLATEINITSMM_H_

///
/// External include files do NOT need to be explicitly specified in real EDKII
/// environment
///
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include "PchAccess.h"
#include "PchPlatformLib.h"

///
/// Driver Consumed Protocol Prototypes
///
#include <Protocol/PchS3Support.h>
#include <Protocol/SmmBase/SmmBase.h>
#include <Protocol/ExitPmAuth/ExitPmAuth.h>
#else
#include <Protocol/SmmBase.h>
#include <Protocol/PchS3Support.h>
#include <Protocol/SmmSwDispatch.h>
#include <Protocol/ExitPmAuth.h>

#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#endif


/**
  An IO Trap SMI callback to copy the DispatchArray data to SMRAM and unregister the IO Trap.

  @param[in] DispatchHandle  - The handle of this callback, obtained when registering
  @param[in] DispatchContext - Pointer to the EFI_SMM_IO_TRAP_DISPATCH_CALLBACK_CONTEXT

  @retval None
**/
VOID
S3SupportSmmExitPmAuthCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
);

#endif
