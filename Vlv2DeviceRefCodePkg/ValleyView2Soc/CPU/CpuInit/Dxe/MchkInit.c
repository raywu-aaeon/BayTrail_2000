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

  MchkInit.c

Abstract:
  Machine check register initialization



--*/

#include "MchkInit.h"
#include "MiscFuncs.h"

extern EFI_PLATFORM_CPU_INFO         mPlatformCpuInfo;

BOOLEAN
IsPowerOnReset (
  VOID
  );

VOID
EnableMCE (
  VOID
  );

VOID
InitializeMchkRegister (
  IN  VOID                                  *Buffer,
  IN  BOOLEAN                               MchkEnable
  )
/*++

Routine Description:

  Initialize all the Machine-Check registers.

Arguments:

  Buffer      - Pointer to private data. Not Used.
  MchkEnable  - Enable or disable Mchk.

Returns:

  None

--*/
{
  EFI_CPU_FEATURE               Feature;
  EFI_IA32_MCG_CAP_LOW_REGISTER *MCGCap;
  UINT64                        MCGCapValue;
  UINT8                         Count;
  UINT8                         Index;
  UINT8                         StartIndex;
  UINT64                        Value;

  if (!MchkEnable) {
    //
    // Do not enable MCHK
    //
    return;
  }

  *(UINT32 *) (&Feature) = mPlatformCpuInfo.CpuFeatures.Regs.RegEdx;

  if (Feature.MCE && Feature.MCA) {
    MCGCapValue = AsmReadMsr64 (EFI_MSR_IA32_MCG_CAP);
    MCGCap      = (EFI_IA32_MCG_CAP_LOW_REGISTER *) &MCGCapValue;

    Count       = (UINT8) MCGCap->Count;

    StartIndex  = 0;
    for (Index = StartIndex; Index < Count; Index++) {
      //
      // Fix for NHM MCK bug for stepping A0/A1/A2
      //
      Value     = (UINT64)-1;
      EfiWriteMsrWithScript (EFI_MSR_IA32_MC0_CTL + Index * 4, Value);
    }

    for (Index = StartIndex; Index < Count; Index++) {
      EfiWriteMsrWithScript (EFI_MSR_IA32_MC0_STATUS + Index * 4, 0);
    }

    EnableMCE ();
  }
}

VOID
InitializeMce (
  IN  BOOLEAN   MchkEnable
  )
/*++

Routine Description:

  Enable MCE feature for current CPU.

Arguments:

  MchkEnable  - Enable Mchk or not.

Returns:

  None

--*/
{
  EFI_CPU_FEATURE     Feature;

  if (!MchkEnable) {
    //
    // Do not enable MCHK
    //
    return ;
  }

  *(UINT32 *) (&Feature) = mPlatformCpuInfo.CpuFeatures.Regs.RegEdx;

  if (Feature.MCE && Feature.MCA) {
    EnableMCE ();
  }
}

BOOLEAN
IsPowerOnReset (
  VOID
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  None

Returns:

  GC_TODO: add return values

--*/
{
  EFI_PEI_HOB_POINTERS        GuidHob;

  //
  // Get Hob list
  //
  GuidHob.Raw = GetHobList ();

  if (GuidHob.Raw == NULL) {
    DEBUG ((EFI_D_ERROR, "No HOBs found\n"));
    return FALSE;
  }

  //
  // Check for HtBist Data Hob.
  //
  GuidHob.Raw = GetNextGuidHob (&gEfiPowerOnHobGuid, GuidHob.Raw);

  if (GuidHob.Raw == NULL) {
    return FALSE;
  }

  return TRUE;
}

