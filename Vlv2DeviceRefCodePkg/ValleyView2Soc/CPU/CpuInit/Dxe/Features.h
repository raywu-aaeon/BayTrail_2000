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

    Features.h

Abstract:



Revision History

--*/

#ifndef _FEATURES_H_
#define _FEATURES_H_

#include "CpuDxe.h"
#include "PlatformMpService.h"
#include "FeaturesDef.h"

VOID
InitializeFeaturePerSetup (
  IN  MP_SYSTEM_DATA                       *MPSystemData
  );

VOID
ProgramProcessorFeature (
  IN OUT VOID  *Buffer
  );

VOID
CollectProcessorFeature (
  IN OUT VOID  *Buffer
  );

VOID
UpdatePlatformCpuData (
  VOID
  );

VOID
ProgrameCpuidLimit (
  IN OUT VOID  *Buffer
  );

VOID
LockFeatureBit (
  IN  BOOLEAN   LockFeatureEnable
  );

UINT32
GetCsrDesiredCores (
  VOID
  );

VOID
SetLockCsrDesiredCores (
  VOID
  );

#endif
