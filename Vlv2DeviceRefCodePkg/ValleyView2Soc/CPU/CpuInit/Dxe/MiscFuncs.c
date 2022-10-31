/*++

Copyright (c) 1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  MiscFuncs.c

Abstract:

  Provide the misc functions to enable some CPU features

--*/
#include "CpuDxe.h"
#include "PlatformMpService.h"
#include "Thermal.h"
#include "MchkInit.h"
#include "MiscFuncs.h"

extern MP_SYSTEM_DATA               *mMPSystemData;
extern EFI_PLATFORM_CPU_INFO        mPlatformCpuInfo;

VOID
EfiWriteToScript (
  IN UINT32     Index,
  IN UINT64     Value
  )
{
  UINTN   TableIndex;

  ASSERT (mMPSystemData != NULL);

  //
  // Save it into script
  //
  AsmAcquireMPLock  (&(mMPSystemData->S3BootScriptLock));
  TableIndex = mMPSystemData->S3BootScriptCount++;
  AsmReleaseMPLock  (&(mMPSystemData->S3BootScriptLock));

  ASSERT (TableIndex < MAX_CPU_S3_TABLE_SIZE - 1);
  mMPSystemData->S3BootScriptTable[TableIndex].ApicId   = GetApicID (NULL, NULL);
  mMPSystemData->S3BootScriptTable[TableIndex].MsrIndex = Index;
  mMPSystemData->S3BootScriptTable[TableIndex].MsrValue = Value;
}

VOID
EfiWriteMsrWithScript (
  IN UINT32     Index,
  IN UINT64     Value
  )
{
  AsmWriteMsr64 (Index, Value);
  EfiWriteToScript (Index, Value);
}

VOID
ProcessorsPrefetcherInitialization (
  IN UINT64                    PrefetcherControl
  )
{
}

VOID
CpuMiscEnable (
  BOOLEAN        Enable,
  UINT64         BitMask
  )
/*++

Routine Description:

  Provide access to the CPU misc enables MSR

Arguments:

  Enable  - Enable or Disable Misc Features

Returns:

  NONE

--*/
{
  UINT64  MsrValue;

  MsrValue = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
  if (Enable) {
    MsrValue |=  BitMask;
  } else {
    MsrValue &= ~BitMask;
  }
  AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, MsrValue);
}

VOID
ProgramProcessorFuncs (
  IN  MP_SYSTEM_DATA               *MPSystemData
  )
{
  //
  // Initialize Thermal Monitor
  //
  InitializeThermalMonitor ();

  //
  // Initialize some misc functions
  //
  CpuMiscEnable (MPSystemData->MonitorMwaitEnable,     B_EFI_MSR_IA32_MISC_ENABLE_MONITOR);

  //
  // Initialize Machine Check registers
  //
  InitializeMchkRegister (NULL, MPSystemData->MachineCheckEnable);
}
