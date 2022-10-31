/*++

This file contains a 'Sample Driver' and is licensed as such
under the terms of your license agreement with Intel or your
vendor.  This file may be modified by the user, subject to
the additional terms of the license agreement

--*/

/*++
Copyright (c)  2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

 SeCPolicyInitDxe.h

Abstract:


--*/
#ifndef _DXE_SEC_PLATFORM_POLICY_H_
#define _DXE_SEC_PLATFORM_POLICY_H_

#include <PiDxe.h>

#include <Protocol/SeCPlatformPolicy.h>

#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseLib.h>
#include <library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/BaseCryptLib.h>
#include <Protocol/GlobalNvsArea.h>
#include <Protocol/SeCOperation.h>
#include <Protocol/Heci.h>
#include <Setup.h>

#include "PchAccess.h"
#include "setupMode.h"
#include "MkhiMsgs.h"
#include "HeciRegs.h"
#include "SecChipset.h"
#include "CoreBiosMsg.h"
#include "SeCAccess.h"
#include <Library/SeCLib.h>

#define FW_MSG_DELAY          1000    // show warning msg and stay for 1 milisecond.
#define FW_MSG_DELAY_TIMEOUT  10


EFI_STATUS
GetPlatformSeCInfo(
    OUT SEC_INFOMATION  *SeCInfo
);

EFI_STATUS
SetPlatformSeCInfo(
    IN SEC_INFOMATION  *SeCInfo
);

EFI_STATUS
PerformSeCOperation(
    IN  UINTN  SeCOpId
);

VOID
ShowSeCReportError(
    IN SEC_ERROR_MSG_ID            MsgId
);

VOID
PlatformHook (
  VOID
  );

EFI_STATUS
SeCPolicyReadyToBootEvent(
    EFI_EVENT           Event,
    VOID                *ParentImageHandle
);


#endif

