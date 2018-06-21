/*++

Copyright (c) 2005-2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  McFunc.h

Abstract:

  This file inlcude external Memory DIMM initialization.

--*/

#ifndef _MCFUNC_H_
#define _MCFUNC_H_

#include "Mrc.h"

STATUS
FindCoreFrequency (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  );


STATUS
GetPlatformSettings (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  );

VOID
DetermineBootMode (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  );

STATUS
McEnableHPET (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  );

STATUS
McDisableHPET (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  );

STATUS
SetInitDone (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  );

UINT8
GetCoreStepping();

void Lfsr32 (UINT32 *LfsrWordPtr);

UINT32 get_initial_seed();

STATUS 
SetScrambler (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  );

BOOLEAN
CheckColdBootRequired (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  );

STATUS ProgBunit (MRC_PARAMETER_FRAME *CurrentMrcData, UINT8 Channel);
STATUS DisableRank2RankSwitching (MRC_PARAMETER_FRAME *CurrentMrcData, UINT8 Channel);

#endif

