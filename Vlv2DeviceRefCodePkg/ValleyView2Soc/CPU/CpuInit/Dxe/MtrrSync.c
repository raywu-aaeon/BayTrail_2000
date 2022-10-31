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

  MtrrSync.c

Abstract:

  Code which support multi-processor

--*/

#include "PlatformMpService.h"
#include "MiscFuncs.h"

extern MP_SYSTEM_DATA               *mMPSystemData;
extern EFI_PLATFORM_CPU_INFO        mPlatformCpuInfo;

EFI_MTRR_VALUES mFixedMtrrValues[] = {
  { EFI_MSR_IA32_MTRR_FIX64K_00000, 0 },
  { EFI_MSR_IA32_MTRR_FIX16K_80000, 0 },
  { EFI_MSR_IA32_MTRR_FIX16K_A0000, 0 },
  { EFI_MSR_IA32_MTRR_FIX4K_C0000,  0 },
  { EFI_MSR_IA32_MTRR_FIX4K_C8000,  0 },
  { EFI_MSR_IA32_MTRR_FIX4K_D0000,  0 },
  { EFI_MSR_IA32_MTRR_FIX4K_D8000,  0 },
  { EFI_MSR_IA32_MTRR_FIX4K_E0000,  0 },
  { EFI_MSR_IA32_MTRR_FIX4K_E8000,  0 },
  { EFI_MSR_IA32_MTRR_FIX4K_F0000,  0 },
  { EFI_MSR_IA32_MTRR_FIX4K_F8000,  0 }
};

EFI_MTRR_VALUES mMtrrDefType[] = { { EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE, 0 } };

EFI_MTRR_VALUES mVariableMtrrValues[] = {
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE, 0      },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 1,  0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 2,  0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 3,  0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 4,  0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 5,  0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 6,  0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 7,  0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 8,  0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 9,  0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 10, 0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 11, 0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 12, 0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 13, 0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 14, 0 },
  { EFI_MSR_CACHE_VARIABLE_MTRR_END, 0       }
};

VOID
DisableCacheAsRam (
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  None

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  UINT64                    CacheAsRamMsr;

  CacheAsRamMsr = AsmReadMsr64 (EFI_MSR_NO_EVICT_MODE);

  //
  // Per Pineview BWG section 3.21, BIOS must set the Disable FERR Assertion
  //
//  CacheAsRamMsr |= B_EFI_MSR_NO_EVICT_MODE_DISABLE_FERR;
//  AsmWriteMsr64 (EFI_MSR_NO_EVICT_MODE, CacheAsRamMsr);

  //
  // Step 3: Disable No-Eviction Mode Run State by clearing
  //         NO_EVICT_MODE MSR 2E0h bit [1] = 0
  CacheAsRamMsr &= ~B_EFI_MSR_NO_EVICT_MODE_RUN;
  AsmWriteMsr64 (EFI_MSR_NO_EVICT_MODE, CacheAsRamMsr);

  // Step 4: Disable No-Eviction Mode Setup State by clearing
  //         NO_EVICT_MODE MSR 2E0h bit [0] = 0
  CacheAsRamMsr &= ~B_EFI_MSR_NO_EVICT_MODE_SETUP;
  AsmWriteMsr64 (EFI_MSR_NO_EVICT_MODE, CacheAsRamMsr);
}

VOID
ReadMtrrRegisters (
  VOID
  )
/*++

Routine Description:

  Save the MTRR registers to global variables

Arguments:

Returns:
    None

--*/
{

  UINT32  Index, IndexEnd;
  //
  // Read Fixed Mtrrs
  //
  for (Index = 0; Index < sizeof (mFixedMtrrValues) / sizeof (EFI_MTRR_VALUES); Index++) {
    mFixedMtrrValues[Index].Value = AsmReadMsr64 (mFixedMtrrValues[Index].Index);
  }
  //
  // Read def type Fixed Mtrrs
  //
  mMtrrDefType[0].Value = AsmReadMsr64 (EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE);

  //
  // Read Variable Mtrr
  //
  IndexEnd = 2 * (UINT32)(AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP) & B_EFI_MSR_IA32_MTRR_CAP_VARIABLE_SUPPORT);
  for (Index = 0; Index < IndexEnd; Index++) {
    if (Index < (sizeof (mVariableMtrrValues) / sizeof (EFI_MTRR_VALUES))) {
      mVariableMtrrValues[Index].Value = AsmReadMsr64 (mVariableMtrrValues[Index].Index);
    }
  }

  return ;
}

VOID
MpMtrrSynchUp (
  IN VOID               *Buffer
  )
/*++

Routine Description:

  Synch up the MTRR values for all processors

Arguments:

  Buffer - Not used.

Returns:

  None

--*/
{
  UINT32              Index, IndexEnd;
  UINTN               Cr4;
  UINT64              MsrValue;
  UINT64              ValidMtrrAddressMask;

  //
  // ASM code to setup processor register before synching up the MTRRs
  //
  Cr4 = MpMtrrSynchUpEntry ();

  ValidMtrrAddressMask = (LShiftU64( 1, mPlatformCpuInfo.CpuAddress.PhysicalBits) - 1) & 0xfffffffffffff000;

  //
  // Make sure all threads has FERR disabled per Pineview BWG section 3.21
  //
  //DisableCacheAsRam ();

  //
  // Disable Fixed Mtrrs
  //
  AsmWriteMsr64 (EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE, mMtrrDefType[0].Value & 0xFFFFF7FF);

  //
  // Update Fixed Mtrrs
  //
  for (Index = 0; Index < sizeof (mFixedMtrrValues) / sizeof (EFI_MTRR_VALUES); Index++) {
    AsmWriteMsr64 (mFixedMtrrValues[Index].Index, mFixedMtrrValues[Index].Value);
  }
  //
  // Synchup def type Fixed Mtrrs
  //
  AsmWriteMsr64 (EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE, mMtrrDefType[0].Value);

  //
  // Synchup Base Variable Mtrr
  //
  IndexEnd = 2 * (UINT32)(AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP) & B_EFI_MSR_IA32_MTRR_CAP_VARIABLE_SUPPORT);
  for (Index = 0; Index < IndexEnd; Index += 2) {
    if (Index < (sizeof (mVariableMtrrValues) / sizeof (EFI_MTRR_VALUES))) {
      MsrValue = (mVariableMtrrValues[Index].Value & 0x0FFF) | (mVariableMtrrValues[Index].Value & ValidMtrrAddressMask);
      AsmWriteMsr64 (mVariableMtrrValues[Index].Index, MsrValue);
    }
  }
  //
  // Synchup Mask Variable Mtrr including valid bit
  //
  for (Index = 1; Index < IndexEnd; Index += 2) {
    if (Index < (sizeof (mVariableMtrrValues) / sizeof (EFI_MTRR_VALUES))) {
      MsrValue = (mVariableMtrrValues[Index].Value & 0x0FFF) | (mVariableMtrrValues[Index].Value & ValidMtrrAddressMask);
      AsmWriteMsr64 (mVariableMtrrValues[Index].Index, MsrValue);
    }
  }
  //
  // ASM code to setup processor register after synching up the MTRRs
  //
  MpMtrrSynchUpExit (Cr4);
}

VOID
SaveBspMtrrForS3 (
  )
{
  UINTN  Index, IndexEnd;
  UINTN  TableIndex;

  TableIndex = 0;

  for (Index = 0; Index < sizeof (mFixedMtrrValues) / sizeof (EFI_MTRR_VALUES); Index++) {
    mMPSystemData->S3BspMtrrTable[TableIndex].Index = mFixedMtrrValues[Index].Index;
    mMPSystemData->S3BspMtrrTable[TableIndex].Value = mFixedMtrrValues[Index].Value;
    TableIndex++;
  }

  IndexEnd = 2 * (UINT32)(AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP) & B_EFI_MSR_IA32_MTRR_CAP_VARIABLE_SUPPORT);
  for (Index = 0; Index < IndexEnd; Index ++) {
    if (TableIndex < (sizeof (mMPSystemData->S3BspMtrrTable) / sizeof (EFI_MTRR_VALUES))) {
      if (Index < (sizeof (mVariableMtrrValues) / sizeof (EFI_MTRR_VALUES))) {
        mMPSystemData->S3BspMtrrTable[TableIndex].Index = mVariableMtrrValues[Index].Index;
        mMPSystemData->S3BspMtrrTable[TableIndex].Value = mVariableMtrrValues[Index].Value;
        TableIndex++;
      }
    }
  }

  mMPSystemData->S3BspMtrrTable[TableIndex].Index = mMtrrDefType[0].Index;
  mMPSystemData->S3BspMtrrTable[TableIndex].Value = mMtrrDefType[0].Value;
  TableIndex++;

  //
  // To terminate the table during S3 resume for MTRR synch up
  //
  mMPSystemData->S3BspMtrrTable[TableIndex].Index = 0;

  ASSERT (TableIndex < MAX_CPU_S3_MTRR_ENTRY);
}
