/*++

Copyright (c) 1999 - 2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  MemoryAttribute.c

Abstract:


Revision History

--*/

#include "CpuDxe.h"
#include "MemoryAttribute.h"

EFI_FIXED_MTRR    mFixedMtrrTable[] = {
  { EFI_MSR_IA32_MTRR_FIX64K_00000, 0,       0x10000},
  { EFI_MSR_IA32_MTRR_FIX16K_80000, 0x80000, 0x4000},
  { EFI_MSR_IA32_MTRR_FIX16K_A0000, 0xA0000, 0x4000},
  { EFI_MSR_IA32_MTRR_FIX4K_C0000,  0xC0000, 0x1000},
  { EFI_MSR_IA32_MTRR_FIX4K_C8000,  0xC8000, 0x1000},
  { EFI_MSR_IA32_MTRR_FIX4K_D0000,  0xD0000, 0x1000},
  { EFI_MSR_IA32_MTRR_FIX4K_D8000,  0xD8000, 0x1000},
  { EFI_MSR_IA32_MTRR_FIX4K_E0000,  0xE0000, 0x1000},
  { EFI_MSR_IA32_MTRR_FIX4K_E8000,  0xE8000, 0x1000},
  { EFI_MSR_IA32_MTRR_FIX4K_F0000,  0xF0000, 0x1000},
  { EFI_MSR_IA32_MTRR_FIX4K_F8000,  0xF8000, 0x1000}
};

EFI_VARIABLE_MTRR mVariableMtrr[8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS]; //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
UINT32            mUsedMtrr;
UINT8             mDefaultMemoryType = EFI_MEMORY_UC;
extern UINT64     mValidMtrrAddressMask;
extern UINT64     mValidMtrrBitsMask;

VOID
PreMtrrChange (
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
  UINT64  TempQword;

#ifdef ECP_FLAG
  EfiDisableCache();
#else
  AsmDisableCache ();
#endif
  //
  // Disable Cache MTRR
  //
  TempQword = AsmReadMsr64 (EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE);
  TempQword = TempQword & ~B_EFI_MSR_GLOBAL_MTRR_ENABLE & ~B_EFI_MSR_FIXED_MTRR_ENABLE;
  AsmWriteMsr64 (EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE, TempQword);
}

VOID
PostMtrrChange (
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
  UINT64  TempQword;

  TempQword = 0;
  //
  // Enable Cache MTRR
  //
  TempQword = AsmReadMsr64 (EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE);
  AsmWriteMsr64 (EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE, TempQword | B_EFI_MSR_GLOBAL_MTRR_ENABLE | B_EFI_MSR_FIXED_MTRR_ENABLE);

#ifdef ECP_FLAG
  EfiEnableCache();
#else
  AsmEnableCache ();
#endif
}

EFI_STATUS
ProgramFixedMtrr (
  IN UINT64     MemoryCacheType,
  IN UINT64     *Base,
  IN UINT64     *Length
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MemoryCacheType - GC_TODO: add argument description
  Base            - GC_TODO: add argument description
  Length          - GC_TODO: add argument description

Returns:

  EFI_UNSUPPORTED - GC_TODO: Add description for return value
  EFI_UNSUPPORTED - GC_TODO: Add description for return value
  EFI_UNSUPPORTED - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  UINT32  MsrNum;
  UINT32  ByteShift;
  UINT64  TempQword;
  UINT64  OrMask;
  UINT64  ClearMask;

  TempQword = 0;
  OrMask    = 0;
  ClearMask = 0;

  for (MsrNum = 0; MsrNum < V_EFI_FIXED_MTRR_NUMBER; MsrNum++) {
    if ((*Base >= mFixedMtrrTable[MsrNum].BaseAddress) &&
        (*Base < (mFixedMtrrTable[MsrNum].BaseAddress + 8 * mFixedMtrrTable[MsrNum].Length))
       ) {
      break;
    }
  }

  if (MsrNum == V_EFI_FIXED_MTRR_NUMBER) {
    return EFI_UNSUPPORTED;
  }
  //
  // We found the fixed MTRR to be programmed
  //
  for (ByteShift = 0; ByteShift < 8; ByteShift++) {
    if (*Base == (mFixedMtrrTable[MsrNum].BaseAddress + ByteShift * mFixedMtrrTable[MsrNum].Length)) {
      break;
    }
  }

  if (ByteShift == 8) {
    return EFI_UNSUPPORTED;
  }

  for (; ((ByteShift < 8) && (*Length >= mFixedMtrrTable[MsrNum].Length)); ByteShift++) {
    OrMask |= LShiftU64 ((UINT64) MemoryCacheType, (UINT32) (ByteShift * 8));
    ClearMask |= LShiftU64 ((UINT64) 0xFF, (UINT32) (ByteShift * 8));
    *Length -= mFixedMtrrTable[MsrNum].Length;
    *Base += mFixedMtrrTable[MsrNum].Length;
  }

  if (ByteShift < 8 && (*Length != 0)) {
    return EFI_UNSUPPORTED;
  }

  TempQword = AsmReadMsr64 (mFixedMtrrTable[MsrNum].Msr) &~ClearMask | OrMask;
  AsmWriteMsr64 (mFixedMtrrTable[MsrNum].Msr, TempQword);
  return EFI_SUCCESS;
}

EFI_STATUS
GetMemoryAttribute (
  VOID
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
  UINTN   Index;
  UINT32  MsrNum, MsrNumEnd;
  UINT64  MsrValue;

  //
  // Get Default Mtrr Type
  //
  MsrValue = AsmReadMsr64 (EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE);
  mDefaultMemoryType = (UINT8) MsrValue;

  //
  // Get Variable Mtrr
  //
  ZeroMem (mVariableMtrr, sizeof (EFI_VARIABLE_MTRR) * (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS)); //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
  mUsedMtrr = 0;
  MsrNumEnd = EFI_MSR_CACHE_VARIABLE_MTRR_BASE + (2 * (UINT32)(AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP) & B_EFI_MSR_IA32_MTRR_CAP_VARIABLE_SUPPORT));
  for (MsrNum = EFI_MSR_CACHE_VARIABLE_MTRR_BASE, Index = 0;
       ((MsrNum < MsrNumEnd) && (Index < (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS))); //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
       MsrNum += 2) {
    if ((AsmReadMsr64 (MsrNum + 1) & B_EFI_MSR_CACHE_MTRR_VALID) != 0) {
      mVariableMtrr[Index].Msr          = MsrNum;
      mVariableMtrr[Index].BaseAddress  = (AsmReadMsr64 (MsrNum) & mValidMtrrAddressMask);
      mVariableMtrr[Index].Length       = ((~((AsmReadMsr64 (MsrNum + 1) & mValidMtrrAddressMask))) & mValidMtrrBitsMask) + 1;
      mVariableMtrr[Index].Type         = (AsmReadMsr64 (MsrNum) & B_EFI_MSR_CACHE_MEMORY_TYPE);
      mVariableMtrr[Index].Valid        = TRUE;
      mUsedMtrr++;
      Index++;
    }
  }

  return EFI_SUCCESS;
}

BOOLEAN
CheckMemoryAttributeOverlap (
  IN EFI_PHYSICAL_ADDRESS     Start,
  IN EFI_PHYSICAL_ADDRESS     End
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Start - GC_TODO: add argument description
  End   - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  UINT32  Index;

  for (Index = 0; Index < (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS); Index++) { //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
    if (mVariableMtrr[Index].Valid &&
        !(
          Start > (mVariableMtrr[Index].BaseAddress + mVariableMtrr[Index].Length - 1) ||
          (End < mVariableMtrr[Index].BaseAddress)
        )
       ) {

      return TRUE;
    }
  }

  return FALSE;
}

EFI_STATUS
CombineMemoryAttribute (
  IN UINT64     Attributes,
  IN UINT64     *Base,
  IN UINT64     *Length
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Attributes  - GC_TODO: add argument description
  Base        - GC_TODO: add argument description
  Length      - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_ACCESS_DENIED - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  UINT32  Index;
  UINT64  CombineStart;
  UINT64  CombineEnd;
  UINT64  MtrrEnd;
  UINT64  EndAddress;
  BOOLEAN InvalidMTRRs[(8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS)]; //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 

  EndAddress = *Base +*Length - 1;

  for (Index = 0; Index < (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS); Index++) { //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
    InvalidMTRRs[Index] = FALSE;
  }

  Index = 0;
  while (Index < (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS)) { //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
    MtrrEnd = mVariableMtrr[Index].BaseAddress + mVariableMtrr[Index].Length - 1;

    //
    // The MTRR is marked invalid or the ranges are not intersected.
    //
    if (InvalidMTRRs[Index]          ||
        !mVariableMtrr[Index].Valid  ||
        (*Base > (MtrrEnd) || (EndAddress < mVariableMtrr[Index].BaseAddress))) {
      Index++;
      continue;
    }

    //
    // if the requested range contains MTRR range, invalidate this MTRR
    //
    if (mVariableMtrr[Index].BaseAddress >= *Base && MtrrEnd <= EndAddress) {
      InvalidMTRRs[Index] = TRUE;
      Index++;
      continue;
    }

//AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32  >>
    if (Attributes == EFI_CACHE_UNCACHEABLE) {
	  UINT64  SmrrBase = 0, SmrrEnd = 0;
	  SmrrBase = (AsmReadMsr64 (EFI_MSR_SMRR_PHYS_BASE) & 0xfffff000);
	  SmrrEnd = SmrrBase + ((~ (AsmReadMsr64(EFI_MSR_SMRR_PHYS_MASK)&0xfffff000))&0xffffffff);
	  if (SmrrBase <= *Base && SmrrEnd>= EndAddress) {
		*Length = 0;
		return EFI_SUCCESS;
	  }
    }
//AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 <<
    if (Attributes == mVariableMtrr[Index].Type) {
      //
      // if the Mtrr range contain the request range, return EFI_SUCCESS
      //
      if (mVariableMtrr[Index].BaseAddress <= *Base && MtrrEnd >= EndAddress) {
        *Length = 0;
        return EFI_SUCCESS;
      }
      //
      // invalid this MTRR, and program the combine range
      //
      CombineStart  = (*Base) < mVariableMtrr[Index].BaseAddress ? (*Base) : mVariableMtrr[Index].BaseAddress;
      CombineEnd    = EndAddress > MtrrEnd ? EndAddress : MtrrEnd;

      //
      // Record this MTRR as invalid
      //
      InvalidMTRRs[Index] = TRUE;

      //
      // The range is modified, retry from the first MTRR
      //
      if (*Base != CombineStart || *Length != CombineEnd - CombineStart + 1) {
        Index = 0;
      } else {
        Index++;
      }
      *Base       = CombineStart;
      *Length     = CombineEnd - CombineStart + 1;
      EndAddress  = CombineEnd;
      continue;
    }

    if ((Attributes == EFI_CACHE_UNCACHEABLE) ||
        (Attributes == EFI_CACHE_WRITETHROUGH && mVariableMtrr[Index].Type == EFI_CACHE_WRITEBACK) ||
        (Attributes == EFI_CACHE_WRITEBACK && mVariableMtrr[Index].Type == EFI_CACHE_WRITETHROUGH) ||
        (Attributes == EFI_CACHE_WRITETHROUGH && mVariableMtrr[Index].Type == EFI_CACHE_UNCACHEABLE) ||
        (Attributes == EFI_CACHE_WRITEBACK && mVariableMtrr[Index].Type == EFI_CACHE_UNCACHEABLE)
       ) {
      Index++;
      continue;
    }

    //
    // Other type memory overlap is invalid
    //
    return EFI_ACCESS_DENIED;
  }

  //
  // Finally invalidate recorded MTRRs
  //
  for (Index = 0; Index < (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS); Index++) { //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
    if (InvalidMTRRs[Index]) {
      InvariableMtrr (mVariableMtrr[Index].Msr, Index);
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
GetDirection (
  IN UINT64      Input,
  IN UINTN       *MtrrNumber,
  IN BOOLEAN     *Direction
  )
/*++

Routine Description:
    Given the input, check if the number of MTRR is lesser
    if positive or subtractive

Arguments:
    Input - Length of Memory to program MTRR
    MtrrNumber - return needed Mtrr number
    Direction  - TRUE: do positive
                 FALSE: do subtractive
Returns:
    Zero, do positive
    Non-Zero, do subractive

--*/
// GC_TODO:    EFI_SUCCESS - add return value to function comment
{
  UINT64  TempQword;
  UINT32  Positive;
  UINT32  Subtractive;

  TempQword   = Input;
  Positive    = 0;
  Subtractive = 0;

  do {
    TempQword -= Power2MaxMemory (TempQword);
    Positive++;

  } while (TempQword != 0);

  TempQword = Power2MaxMemory (LShiftU64 (Input, 1)) - Input;
  Subtractive++;
  do {
    TempQword -= Power2MaxMemory (TempQword);
    Subtractive++;

  } while (TempQword != 0);

  if (Positive <= Subtractive) {
    *Direction  = TRUE;
    *MtrrNumber = Positive;
  } else {
    *Direction  = FALSE;
    *MtrrNumber = Subtractive;
  }

  return EFI_SUCCESS;
}

UINT64
Power2MaxMemory (
  IN UINT64                     MemoryLength
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MemoryLength  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  UINT64  Result;

  if (RShiftU64 (MemoryLength, 32)) {
    Result = LShiftU64 ((UINT64) GetPowerOfTwo64 ((UINT32) RShiftU64 (MemoryLength, 32)), 32);
  } else {
    Result = (UINT64) GetPowerOfTwo64 ((UINT32) MemoryLength);
  }

  return Result;
}

EFI_STATUS
InvariableMtrr (
  IN UINTN     MtrrNumber,
  IN UINTN     Index
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MtrrNumber  - GC_TODO: add argument description
  Index       - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  PreMtrrChange ();
  mVariableMtrr[Index].Valid = FALSE;
  AsmWriteMsr64 ((UINT32) MtrrNumber, 0);
  AsmWriteMsr64 ((UINT32) (MtrrNumber + 1), 0);
  mUsedMtrr--;
  PostMtrrChange ();

  return EFI_SUCCESS;
}

EFI_STATUS
ProgramVariableMtrr (
  IN UINTN                    MtrrNumber,
  IN EFI_PHYSICAL_ADDRESS     BaseAddress,
  IN UINT64                   Length,
  IN UINT64                   MemoryCacheType
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MtrrNumber      - GC_TODO: add argument description
  BaseAddress     - GC_TODO: add argument description
  Length          - GC_TODO: add argument description
  MemoryCacheType - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  UINT64  TempQword;

  PreMtrrChange ();

  //
  // MTRR Physical Base
  //
  TempQword = (BaseAddress & mValidMtrrAddressMask) | MemoryCacheType;
  AsmWriteMsr64 ((UINT32) MtrrNumber, TempQword);

  //
  // MTRR Physical Mask
  //
  TempQword = ~(Length - 1);
  AsmWriteMsr64 ((UINT32) (MtrrNumber + 1), (TempQword & mValidMtrrAddressMask) | B_EFI_MSR_CACHE_MTRR_VALID);

  PostMtrrChange ();

  return EFI_SUCCESS;
}

EFI_STATUS
CleanupVariableMtrr (
  VOID
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  None

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_OUT_OF_RESOURCES - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  BOOLEAN               Cleaned;
  BOOLEAN               EverCleaned;
  UINTN                 Index;
  UINTN                 Index2;

  BOOLEAN               MtrrModified[(8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS)]; //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 

  UINTN                 MtrrNumber;
  UINTN                 MsrNum, MsrNumEnd;

  EFI_PHYSICAL_ADDRESS  BaseAddress;
  UINT64                Length;
  UINT64                TempQword;
  UINT64                Attributes;

  BOOLEAN               Positive;

  for (Index = 0; Index < (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS); Index++) { //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
    MtrrModified[Index] = FALSE;
  }

  GetMemoryAttribute ();

  //
  // After the do-while, mVariableMtrr is NO longer the value read from MTRR regisrer!!!
  //
  EverCleaned = FALSE;

  do {
    Cleaned = FALSE;

    for (Index = 0; Index < (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS); Index++) { //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
      if (mVariableMtrr[Index].Type == EFI_CACHE_UNCACHEABLE && mVariableMtrr[Index].Valid) {
        for (Index2 = 0; Index2 < (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS); Index2++) { //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
          if (mVariableMtrr[Index2].Type == EFI_CACHE_WRITEBACK && mVariableMtrr[Index2].Valid) {
            //
            // the Uncacheble just inside the WB and at the edge.
            // if so, we can clean the UC entry and decrease the WB entry
            //
            if (mVariableMtrr[Index].BaseAddress == mVariableMtrr[Index2].BaseAddress) {
              Cleaned     = TRUE;
              EverCleaned = TRUE;
              if (mVariableMtrr[Index].Length >= mVariableMtrr[Index2].Length) {
                //
                // we can invalidate WB entry, since nothing left
                //
                InvariableMtrr (mVariableMtrr[Index2].Msr, Index2);
                mVariableMtrr[Index].BaseAddress = mVariableMtrr[Index].BaseAddress + mVariableMtrr[Index2].Length;
                mVariableMtrr[Index].Length -= mVariableMtrr[Index2].Length;
                MtrrModified[Index] = TRUE;
                if (mVariableMtrr[Index].Length == 0) {
                  InvariableMtrr (mVariableMtrr[Index].Msr, Index);
                }

              } else {
                //
                // we can invalidate UC entry, since nothing left
                //
                InvariableMtrr (mVariableMtrr[Index].Msr, Index);
                mVariableMtrr[Index2].BaseAddress = mVariableMtrr[Index].BaseAddress + mVariableMtrr[Index].Length;
                mVariableMtrr[Index2].Length -= mVariableMtrr[Index].Length;
                MtrrModified[Index2] = TRUE;
              }

            }

            if (mVariableMtrr[Index].BaseAddress +
                mVariableMtrr[Index].Length == mVariableMtrr[Index2].BaseAddress +
                mVariableMtrr[Index2].Length
               ) {
              Cleaned     = TRUE;
              EverCleaned = TRUE;

              if (mVariableMtrr[Index].Length >= mVariableMtrr[Index2].Length) {
                //
                // we can invalidate WB entry, since nothing left
                //
                InvariableMtrr (mVariableMtrr[Index2].Msr, Index2);
                mVariableMtrr[Index].Length -= mVariableMtrr[Index2].Length;
                MtrrModified[Index] = TRUE;
                if (mVariableMtrr[Index].Length == 0) {
                  InvariableMtrr (mVariableMtrr[Index].Msr, Index);
                }

              } else {
                //
                // we can invalidate UC entry, since nothing left
                //
                InvariableMtrr (mVariableMtrr[Index].Msr, Index);
                mVariableMtrr[Index2].Length -= mVariableMtrr[Index].Length;
                MtrrModified[Index2] = TRUE;
              }

            }
          }
          //
          // end WB
          //
        }
        //
        // end of Index2
        //
      }
      //
      // Endof UC
      //
    }
    //
    // endof Index
    //
  } while (Cleaned);

  if (!EverCleaned) {
    return EFI_SUCCESS;
  }

  MsrNumEnd = EFI_MSR_CACHE_VARIABLE_MTRR_BASE + (2 * (UINT32)(AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP) & B_EFI_MSR_IA32_MTRR_CAP_VARIABLE_SUPPORT));
  //
  // Begin to program the MTRR again
  //
  for (Index = 0; Index < (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS); Index++) { //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
    if (MtrrModified[Index] && mVariableMtrr[Index].Valid) {
      //
      // Program the new MTRR
      //
      TempQword   = mVariableMtrr[Index].Length;
      MsrNum      = EFI_MSR_CACHE_VARIABLE_MTRR_BASE + 2 * Index;
      BaseAddress = mVariableMtrr[Index].BaseAddress;
      Length      = mVariableMtrr[Index].Length;
      Attributes  = mVariableMtrr[Index].Type;

      if (TempQword == Power2MaxMemory (TempQword)) {
        //
        // if it's two's power
        // no need to request a new mtrr,
        // just program this one
        //
        ProgramVariableMtrr (
          MsrNum,
          BaseAddress,
          Length,
          Attributes
          );
      } else {
        GetDirection (TempQword, &MtrrNumber, &Positive);
        //
        // we already has one that can use, so 6+1
        //
        if ((mUsedMtrr + MtrrNumber) > ((8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS) + 1)) { //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
          return EFI_OUT_OF_RESOURCES;
        }

        if (!Positive) {
          Length = Power2MaxMemory (LShiftU64 (TempQword, 1));
          ProgramVariableMtrr (
            MsrNum,
            BaseAddress,
            Length,
            Attributes
            );
          BaseAddress += TempQword;
          TempQword   = Length - TempQword;
          Attributes  = EFI_CACHE_UNCACHEABLE;
        }

        do {
          //
          // Find unused MTRR
          //
          for (MsrNum = EFI_MSR_CACHE_VARIABLE_MTRR_BASE; MsrNum < MsrNumEnd; MsrNum += 2) {
            if ((AsmReadMsr64 ((UINT32) (MsrNum + 1)) & B_EFI_MSR_CACHE_MTRR_VALID) == 0) {
              break;
            }
          }

          Length = Power2MaxMemory (TempQword);
          ProgramVariableMtrr (
            MsrNum,
            BaseAddress,
            Length,
            Attributes
            );
          BaseAddress += Length;
          TempQword -= Length;

        } while (TempQword);

      }
      //
      // endof Powerof
      //
    }
    //
    // endof Modified
    //
  }
  //
  // endof for
  //
  return EFI_SUCCESS;
}

UINT64
GetMemorySpaceAttributeFromMtrrType (
  IN UINT8                MtrrAttributes
  )
/*++

Routine Description:

  Get GCD Mem Space type from Mtrr Type

Arguments:

  MtrrAttribute - Mtrr type

Returns:

  GCD Mem Space typed (64-bit)

--*/
{
  switch (MtrrAttributes) {
    case EFI_CACHE_UNCACHEABLE:
      return EFI_MEMORY_UC;
    case EFI_CACHE_WRITECOMBINING:
      return EFI_MEMORY_WC;
    case EFI_CACHE_WRITETHROUGH:
      return EFI_MEMORY_WT;
    case EFI_CACHE_WRITEPROTECTED:
      return EFI_MEMORY_WP;
    case EFI_CACHE_WRITEBACK:
      return EFI_MEMORY_WB;
    default:
      return 0;
  }
}

EFI_STATUS
RefreshGcdMemoryAttributes (
  VOID
  )
/*++

Routine Description:

  Refresh the GCD Memory Space Attributes according to MTRRs

Arguments:

Returns:

--*/
{
  EFI_STATUS                          Status;
  UINTN                               Index;
  UINTN                               SubIndex;
  UINT64                              RegValue;
  EFI_PHYSICAL_ADDRESS                BaseAddress;
  UINT64                              Length;
  UINT64                              Attributes;
  UINT64                              CurrentAttributes;
  UINT8                               MtrrType;
  UINTN                               NumberOfDescriptors;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR     *MemorySpaceMap;
  UINT64                              DefaultAttributes;

  MemorySpaceMap = NULL;

  Status = GetMemoryAttribute ();
  if (EFI_ERROR(Status)) {
    goto Done;
  }

  Status = gDS->GetMemorySpaceMap (
                  &NumberOfDescriptors,
                  &MemorySpaceMap
                  );
  if (EFI_ERROR(Status)) {
    goto Done;
  }

  DefaultAttributes = GetMemorySpaceAttributeFromMtrrType (mDefaultMemoryType);

  //
  // Set default attributes to all spaces.
  //
  for (Index = 0; Index < NumberOfDescriptors; Index++) {
    if (MemorySpaceMap[Index].GcdMemoryType == EfiGcdMemoryTypeNonExistent) {
      continue;
    }
    gDS->SetMemorySpaceAttributes (
           MemorySpaceMap[Index].BaseAddress,
           MemorySpaceMap[Index].Length,
           MemorySpaceMap[Index].Attributes & ~EFI_MEMORY_CACHETYPE_MASK |
           (MemorySpaceMap[Index].Capabilities & DefaultAttributes)
           );
  }

  //
  // Go for variable MTRRs, WB first, Other types second
  //
  for (Index = 0; Index < (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS); Index++) { //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
    if (mVariableMtrr[Index].Valid &&
        mVariableMtrr[Index].Type == EFI_CACHE_WRITEBACK) {
      SetGcdMemorySpaceAttributes (
        MemorySpaceMap,
        NumberOfDescriptors,
        mVariableMtrr[Index].BaseAddress,
        mVariableMtrr[Index].Length,
        EFI_MEMORY_WB
        );
    }
  }
  for (Index = 0; Index < (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS); Index++) { //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
    if (mVariableMtrr[Index].Valid &&
        mVariableMtrr[Index].Type != EFI_CACHE_WRITEBACK) {
      Attributes = GetMemorySpaceAttributeFromMtrrType ((UINT8) mVariableMtrr[Index].Type);
      SetGcdMemorySpaceAttributes (
        MemorySpaceMap,
        NumberOfDescriptors,
        mVariableMtrr[Index].BaseAddress,
        mVariableMtrr[Index].Length,
        Attributes
        );
    }
  }

  //
  // Go for fixed MTRRs
  //
  Attributes  = 0;
  BaseAddress = 0;
  Length      = 0;
  for (Index = 0; Index < V_EFI_FIXED_MTRR_NUMBER; Index++) {
    RegValue = AsmReadMsr64 (mFixedMtrrTable[Index].Msr);
    for (SubIndex = 0; SubIndex < 8; SubIndex++) {
      MtrrType = (UINT8) RShiftU64 (RegValue, SubIndex * 8);
      CurrentAttributes = GetMemorySpaceAttributeFromMtrrType (MtrrType);
      if (Length == 0) {
        Attributes = CurrentAttributes;
      } else {
        if (CurrentAttributes != Attributes) {
          SetGcdMemorySpaceAttributes (
            MemorySpaceMap,
            NumberOfDescriptors,
            BaseAddress,
            Length,
            Attributes
            );
          BaseAddress = mFixedMtrrTable[Index].BaseAddress + mFixedMtrrTable[Index].Length * SubIndex;
          Length = 0;
          Attributes = CurrentAttributes;
        }
      }
      Length += mFixedMtrrTable[Index].Length;
    }
  }
  //
  // handle the last region
  //
  SetGcdMemorySpaceAttributes (
    MemorySpaceMap,
    NumberOfDescriptors,
    BaseAddress,
    Length,
    Attributes
    );

Done:
  if (MemorySpaceMap != NULL) {
#ifdef ECP_FLAG
    (gBS->FreePool) (MemorySpaceMap);
#else
    gBS->FreePool (MemorySpaceMap);
#endif
  }
  return Status;
}

EFI_STATUS
SearchGcdMemorySpaces (
  IN EFI_GCD_MEMORY_SPACE_DESCRIPTOR     *MemorySpaceMap,
  IN UINTN                               NumberOfDescriptors,
  IN EFI_PHYSICAL_ADDRESS                BaseAddress,
  IN UINT64                              Length,
  OUT UINTN                              *StartIndex,
  OUT UINTN                              *EndIndex
  )
/*++

Routine Description:

  Search into the Gcd Memory Space for descriptors (from StartIndex
  to EndIndex) that contains the memory range specified by BaseAddress
  and Length.

Arguments:

  MemorySpaceMap      - Gcd Memory Space Map as array
  NumberOfDescriptors - Number of descriptors in map
  BaseAddress         - BaseAddress for the requested range
  Length              - Length for the requested range
  StartIndex          - Start index into the Gcd Memory Space Map
  EndIndex            - End index into the Gcd Memory Space Map

Returns:

  EFI_SUCCESS   - Search successfully
  EFI_NOT_FOUND - The requested descriptors not exist

--*/
{
  UINTN           Index;

  *StartIndex = 0;
  *EndIndex   = 0;
  for (Index = 0; Index < NumberOfDescriptors; Index++) {
    if (BaseAddress >= MemorySpaceMap[Index].BaseAddress &&
        BaseAddress < MemorySpaceMap[Index].BaseAddress + MemorySpaceMap[Index].Length) {
      *StartIndex = Index;
    }
    if (BaseAddress + Length - 1 >= MemorySpaceMap[Index].BaseAddress &&
        BaseAddress + Length - 1 < MemorySpaceMap[Index].BaseAddress + MemorySpaceMap[Index].Length) {
      *EndIndex = Index;
      return EFI_SUCCESS;
    }
  }
  return EFI_NOT_FOUND;
}

EFI_STATUS
SetGcdMemorySpaceAttributes (
  IN EFI_GCD_MEMORY_SPACE_DESCRIPTOR     *MemorySpaceMap,
  IN UINTN                               NumberOfDescriptors,
  IN EFI_PHYSICAL_ADDRESS                BaseAddress,
  IN UINT64                              Length,
  IN UINT64                              Attributes
  )
/*++

Routine Description:

  Set the attributes for a specified range in Gcd Memory Space Map.

Arguments:

  MemorySpaceMap      - Gcd Memory Space Map as array
  NumberOfDescriptors - Number of descriptors in map
  BaseAddress         - BaseAddress for the range
  Length              - Length for the range
  Attributes          - Attributes to set

Returns:

  EFI_SUCCESS   - Set successfully
  EFI_NOT_FOUND - The specified range does not exist in Gcd Memory Space

--*/
{
  EFI_STATUS            Status;
  UINTN                 Index;
  UINTN                 StartIndex;
  UINTN                 EndIndex;
  EFI_PHYSICAL_ADDRESS  RegionStart;
  UINT64                RegionLength;

  Status = SearchGcdMemorySpaces (
             MemorySpaceMap,
             NumberOfDescriptors,
             BaseAddress,
             Length,
             &StartIndex,
             &EndIndex
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = StartIndex; Index <= EndIndex; Index++) {
    if (MemorySpaceMap[Index].GcdMemoryType == EfiGcdMemoryTypeNonExistent) {
      continue;
    }
    if (BaseAddress >= MemorySpaceMap[Index].BaseAddress) {
      RegionStart = BaseAddress;
    } else {
      RegionStart = MemorySpaceMap[Index].BaseAddress;
    }
    if (BaseAddress + Length - 1 < MemorySpaceMap[Index].BaseAddress + MemorySpaceMap[Index].Length) {
      RegionLength = BaseAddress + Length - RegionStart;
    } else {
      RegionLength = MemorySpaceMap[Index].BaseAddress + MemorySpaceMap[Index].Length - RegionStart;
    }
    gDS->SetMemorySpaceAttributes (
           RegionStart,
           RegionLength,
           MemorySpaceMap[Index].Attributes & ~EFI_MEMORY_CACHETYPE_MASK |
           (MemorySpaceMap[Index].Capabilities & Attributes)
           );
  }

  return EFI_SUCCESS;
}
