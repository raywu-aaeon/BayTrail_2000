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

  MMRC.h

Abstract:

  THe external header file that all projects must include inorder to port the MMRC.

--*/

#ifndef _MMRC_H_
#define _MMRC_H_

#include "Types.h"
#include "Memory.h"
#include "MMRCLibraries.h"
#include "../../../Hooks/Include/MmrcHooks.h"

extern STATUS ReceiveEnable (MMRC_DATA *, UINT8);
extern STATUS ReceiveEnableRestore (MMRC_DATA *, UINT8);
extern STATUS FineWriteLeveling (MMRC_DATA *, UINT8);
extern STATUS CoarseWriteLeveling (MMRC_DATA *, UINT8);
extern STATUS WriteLevelingRestore (MMRC_DATA *, UINT8);
extern STATUS ReadTraining (MMRC_DATA *, UINT8);
extern STATUS WriteTraining ( MMRC_DATA *ModMrcData, UINT8 Channel );
extern STATUS ChannelComplete ( MMRC_DATA *ModMrcData, UINT8 Channel );
extern STATUS ReadVrefTraining (MMRC_DATA *, UINT8);
extern STATUS PerformanceSetting(MMRC_DATA *ModMrcData, UINT8 Channel);
extern STATUS LPDDR3_CATraining (MMRC_DATA *ModMrcData, UINT8 Channel);
extern STATUS LPDDR3_LateCATraining(MMRC_DATA *ModMrcData, UINT8 Channel);

#endif
