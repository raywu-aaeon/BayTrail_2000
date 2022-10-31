/*++

Copyright (c)  1999 - 2007 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    MiscFuns.h

Abstract:

    Provide the misc functions to enable some CPU features

--*/

#ifndef _MISC_FUNCS_H_
#define _MISC_FUNCS_H_

#include "CpuDxe.h"
#include "PlatformMpService.h"


VOID
EfiWriteToScript (
  IN UINT32     Index,
  IN UINT64     Value
  );

VOID
EfiWriteMsrWithScript (
  IN UINT32     Index,
  IN UINT64     Value
  );

VOID
ProgramProcessorFuncs (
  IN  MP_SYSTEM_DATA               *MPSystemData
  );

VOID
XtprDisableInitialization (
  IN VOID
  );

VOID
CpuMiscEnable (
  BOOLEAN        Enable,
  UINT64         BitMask
  );

VOID
ProcessorsPrefetcherInitialization (
  IN UINT64                    PrefetcherControl
  );

#endif
