/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    ComplexCachePeim.c

Abstract:

    EFI 2.0 PEIM to initialize the cache and load the BSP microcode

Revision History

--*/

#include "PeiProcessor.h"
#ifdef ECP_FLAG
EFI_GUID gPeiCachePpiGuid = PEI_CACHE_PPI_GUID;
EFI_GUID gEfiHtBistHobGuid = EFI_HT_BIST_HOB_GUID;
EFI_GUID gEfiPlatformCpuInfoGuid = EFI_PLATFORM_CPU_INFO_GUID;

VOID
EFIAPI
EfiDisableCache (
  VOID
  );
VOID
EFIAPI
EfiEnableCache (
  VOID
  );
#endif

INT8
CheckDirection (
  IN  UINT64                    Input
  );

UINT32
SetPower2 (
  IN  UINT32                    Input
  );

UINT64
PeiPower2MaxMemory (
  IN  UINT64                    MemoryAddress,
  IN  UINT64                    MemoryLength
  );

VOID
EfiDisableCacheMtrr (
  IN  UINT64                    *OldMtrr
  );

VOID
EfiRecoverCacheMtrr (
  IN  BOOLEAN                   EnableMtrr,
  IN  UINT64                    OldMtrr
  );

VOID
EfiProgramMtrr (
  IN  UINTN                     MtrrNumber,
  IN  EFI_PHYSICAL_ADDRESS      MemoryAddress,
  IN  UINT64                    MemoryLength,
  IN  EFI_MEMORY_CACHE_TYPE     MemoryCacheType,
  IN  UINT64                    ValidMtrrAddressMask
  );

EFI_STATUS
EFIAPI
PeiSetCacheAttributes (
  IN  EFI_PEI_SERVICES          **PeiServices,
  IN  PEI_CACHE_PPI             *This,
  IN  EFI_PHYSICAL_ADDRESS      MemoryAddress,
  IN  UINT64                    MemoryLength,
  IN  EFI_MEMORY_CACHE_TYPE     MemoryCacheType
  );

EFI_STATUS
EFIAPI
PeiResetCacheAttributes (
  IN  EFI_PEI_SERVICES          **PeiServices,
  IN  PEI_CACHE_PPI             *This
  );

EFI_STATUS
SearchForExactMtrr (
  IN  EFI_PEI_SERVICES          **PeiServices,
  IN  EFI_PHYSICAL_ADDRESS      MemoryAddress,
  IN  UINT64                    MemoryLength,
  IN  UINT64                    ValidMtrrAddressMask,
  OUT UINT32                    *UsedMsrNum,
  OUT EFI_MEMORY_CACHE_TYPE     *MemoryCacheType
  );

BOOLEAN
IsDefaultType (
  IN  EFI_MEMORY_CACHE_TYPE     MemoryCacheType
  );

EFI_STATUS
DisableCacheAsRam (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN BOOLEAN                    DisableCar
  );

UINT32
CheckMtrrAlignment (
  IN  UINT64                    BaseAddress,
  IN  UINT64                    Size
  );

typedef struct {
  UINT32    Msr;
  UINT32    BaseAddress;
  UINT32    Length;
} EFI_FIXED_MTRR;

EFI_FIXED_MTRR mFixedMtrrTable[] = {
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

PEI_CACHE_PPI mCachePpi = {
  PeiSetCacheAttributes,
  PeiResetCacheAttributes
};

INT8
CheckDirection (
  IN  UINT64                    Input
  )
/*++

Routine Description:
    Given the input, check if the number of MTRR is lesser
    if positive or subtractive

Arguments:
    Input - Length of Memory to program MTRR

Returns:
    Zero, do positive
    Non-Zero, do subtractive

--*/
{
  return 0;
}


VOID
EfiDisableCacheMtrr (
  OUT UINT64                   *OldMtrr
  )
/*++
  Routine Description:
    Disable cache and its mtrr
  Arguments:
    OldMtrr - To return the Old MTRR value
  Returns:
    None
--*/
{
  UINT64  TempQword;

  //
  // Disable Cache MTRR
  //
  *OldMtrr = AsmReadMsr64(EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE);
  TempQword = (*OldMtrr) & ~B_EFI_MSR_GLOBAL_MTRR_ENABLE & ~B_EFI_MSR_FIXED_MTRR_ENABLE;
  AsmWriteMsr64(EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE, TempQword);
#ifdef ECP_FLAG
  EfiDisableCache ();
#else
  AsmDisableCache ();
#endif
}

VOID
EfiRecoverCacheMtrr (
  IN BOOLEAN                  EnableMtrr,
  IN UINT64                   OldMtrr
  )
/*++
  Routine Description:
    Recover cache MTRR
  Arguments:
    EnableMtrr - Whether to enable the MTRR
    OldMtrr    - The saved old MTRR value to restore when not to
                 enable the MTRR
  Returns:
    None
--*/
{
  UINT64  TempQword;

  //
  // Enable Cache MTRR
  //
  if (EnableMtrr) {
    TempQword = AsmReadMsr64(EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE);
    TempQword |= (B_EFI_MSR_GLOBAL_MTRR_ENABLE | B_EFI_MSR_FIXED_MTRR_ENABLE);
  } else {
    TempQword = OldMtrr;
  }

  AsmWriteMsr64 (EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE, TempQword);

#ifdef ECP_FLAG
  EfiEnableCache ();
#else
  AsmEnableCache ();
#endif
}

VOID
EfiProgramMtrr (
  IN  UINTN                     MtrrNumber,
  IN  EFI_PHYSICAL_ADDRESS      MemoryAddress,
  IN  UINT64                    MemoryLength,
  IN  EFI_MEMORY_CACHE_TYPE     MemoryCacheType,
  IN  UINT64                    ValidMtrrAddressMask
  )
/*++

Routine Description:

  Programming MTRR according to Memory address, length, and type.

Arguments:

  MtrrNumber           - the variable MTRR index number
  MemoryAddress        - the address of target memory
  MemoryLength         - the length of target memory
  MemoryCacheType      - the cache type of target memory
  ValidMtrrAddressMask - the MTRR address mask

Returns:

  none

--*/
{
  UINT64                        TempQword;
  UINT64                        OldMtrr;

  if (MemoryLength == 0) {
    return;
  }

  EfiDisableCacheMtrr (&OldMtrr);

  //
  // MTRR Physical Base
  //
  TempQword = (MemoryAddress & ValidMtrrAddressMask) | MemoryCacheType;
  AsmWriteMsr64 (MtrrNumber, TempQword);

  //
  // MTRR Physical Mask
  //
  TempQword = ~(MemoryLength - 1);
  AsmWriteMsr64 (MtrrNumber + 1, (TempQword & ValidMtrrAddressMask) | B_EFI_MSR_CACHE_MTRR_VALID);

  EfiRecoverCacheMtrr (TRUE, OldMtrr);
}

UINT64
PeiPower2MaxMemory (
  IN UINT64                 MemoryAddress,
  IN UINT64                 MemoryLength
  )
{
  UINT64                    Result;

  if (MemoryLength == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Compute inital power of 2 size to return
  //
  if (RShiftU64(MemoryLength, 32)) {
    Result = LShiftU64((UINT64)SetPower2((UINT32) RShiftU64(MemoryLength, 32)), 32);
  } else {
    Result = (UINT64)SetPower2((UINT32)MemoryLength);
  }

  //
  // Special case base of 0 as all ranges are valid
  //
  if (MemoryAddress == 0) {
    return Result;
  }

  //
  // Loop till a value that can be mapped to this base address is found
  //
  while (CheckMtrrAlignment (MemoryAddress, Result) != 0) {
    //
    // Need to try the next smaller power of 2
    //
    Result = RShiftU64 (Result, 1);
  }

  return Result;
}

UINT32
CheckMtrrAlignment (
  IN  UINT64    BaseAddress,
  IN  UINT64    Size
  )
/*++

Routine Description:
    Given the input, check if the number of MTRR is lesser
    if positive or subtractive

Arguments:
    Input - Length of Memory to program MTRR

Returns:
    Zero      - Allgnment is valid
    Non-Zero  - Invalid alignment

--*/
{
  UINT32      ShiftedBase;
  UINT32      ShiftedSize;

  //
  // Shift base and size right 12 bits to allow for larger memory sizes.  The
  // MTRRs do not use the first 12 bits so this is safe for now.  Only supports
  // up to 52 bits of physical address space.
  //
  ShiftedBase = (UINT32) RShiftU64 (BaseAddress, 12);
  ShiftedSize = (UINT32) RShiftU64 (Size, 12);

  //
  // Return the results to the caller of the MOD
  //
  return ShiftedBase % ShiftedSize;
}

UINT32
SetPower2 (
  IN  UINT32  Input
  )
/*
Define: bsr - search the operand for most significant set
        bts - Selects the bit in a bit string & sets the selected bit
*/
{
  UINT32                    Result = 0;

#ifdef __GNUC__
  __asm__
  (
     "bsrl %1, %%eax;"
     "btsl %%eax, %0;"
     :"=r"(Result)
     :"r"(Input),"0"(Result)
     :"%eax"
  );
#else
  _asm {
    bsr eax, Input
    bts Result, eax
  }
#endif

  return Result;
}

EFI_STATUS
PeiProgramFixedMtrr (
  IN  EFI_MEMORY_CACHE_TYPE     MemoryCacheType,
  IN  UINT64                    *Base,
  IN  UINT64                    *Len
  )
{

  UINT32                      MsrNum;
  UINT32                      ByteShift;
  UINT64                      TempQword;
  UINT64                      OrMask;
  UINT64                      ClearMask;

  TempQword = 0;
  OrMask    =  0;
  ClearMask = 0;

  for (MsrNum = 0; MsrNum <= V_EFI_FIXED_MTRR_NUMBER; MsrNum++) {
    if (MsrNum < (sizeof (mFixedMtrrTable) / sizeof (EFI_FIXED_MTRR))) {
      if ((*Base >= mFixedMtrrTable[MsrNum].BaseAddress) &&
          (*Base < (mFixedMtrrTable[MsrNum].BaseAddress + 8 * mFixedMtrrTable[MsrNum].Length))) {
        break;
      }
    }
  }
  if (MsrNum == V_EFI_FIXED_MTRR_NUMBER ) {
    return EFI_DEVICE_ERROR;
  }
  //
  // We found the fixed MTRR to be programmed
  //
  for (ByteShift=0; ByteShift < 8; ByteShift++) {
    if (MsrNum < (sizeof (mFixedMtrrTable) / sizeof (EFI_FIXED_MTRR))) {
      if ( *Base == (mFixedMtrrTable[MsrNum].BaseAddress + ByteShift * mFixedMtrrTable[MsrNum].Length)) {
        break;
      }
    }
  }
  if (ByteShift == 8 ) {
    return EFI_DEVICE_ERROR;
  }
  if (MsrNum < (sizeof (mFixedMtrrTable) / sizeof (EFI_FIXED_MTRR))) {
    for (; ((ByteShift<8) && (*Len >= mFixedMtrrTable[MsrNum].Length)); ByteShift++) {
      OrMask |= LShiftU64((UINT64) MemoryCacheType, (UINT32) (ByteShift* 8));
      ClearMask |= LShiftU64((UINT64) 0xFF, (UINT32) (ByteShift * 8));
      *Len -= mFixedMtrrTable[MsrNum].Length;
      *Base += mFixedMtrrTable[MsrNum].Length;
    }
    TempQword = AsmReadMsr64 (mFixedMtrrTable[MsrNum].Msr) & ~ClearMask | OrMask;
    AsmWriteMsr64 (mFixedMtrrTable[MsrNum].Msr, TempQword);
  }

  return EFI_SUCCESS;
}

BOOLEAN
PeiCheckMtrrOverlap (
  IN  EFI_PHYSICAL_ADDRESS      Start,
  IN  EFI_PHYSICAL_ADDRESS      End
  )
/*++

Routine Description:
    Check if there is a valid variable MTRR that overlaps the given range

Arguments:
    Start - Base Address of the range to check

    End -  End address of the range to check
--*/
{
  return FALSE;
}

EFI_STATUS
EFIAPI
PeiSetCacheAttributes (
  IN  EFI_PEI_SERVICES          **PeiServices,
  IN  PEI_CACHE_PPI             *This,
  IN  EFI_PHYSICAL_ADDRESS      MemoryAddress,
  IN  UINT64                    MemoryLength,
  IN  EFI_MEMORY_CACHE_TYPE     MemoryCacheType
  )
/*++

Routine Description:

  Given the memory range and cache type, programs the MTRRs.

Arguments:

  PeiServices           - General purpose services available to every PEIM.
  This                  - Current instance of Pei Cache PPI.
  MemoryAddress         - Base Address of Memory to program MTRR.
  MemoryLength          - Length of Memory to program MTRR.
  MemoryCacheType       - Cache Type.

Returns:

  EFI_SUCCESS           - Mtrr are set successfully.
  EFI_LOAD_ERROR        - No empty MTRRs to use.
  EFI_INVALID_PARAMETER - The input parameter is not valid.
  others                - An error occurs when setting MTTR.

Note:

--*/
{
  EFI_STATUS            Status;
  UINT32                MsrNum, MsrNumEnd;
  UINT64                TempQword;
  UINT32                LastVariableMtrrForBios;
  UINT64                OldMtrr;
  UINT32                UsedMsrNum;
  EFI_MEMORY_CACHE_TYPE UsedMemoryCacheType;
  UINT64                ValidMtrrAddressMask = 0;
  EFI_PEI_HOB_POINTERS  Hob;
  EFI_PLATFORM_CPU_INFO *PlatformCpuInfo = NULL;

  ValidMtrrAddressMask = LShiftU64((UINT64) 1, 36) & (~(UINT64)0x0FFF);

  //
  // Get Platform CPU Info HOB
  //
  Hob.Raw = GetFirstGuidHob(&gEfiPlatformCpuInfoGuid);
  if (Hob.Raw == NULL) {
    Status = EFI_NOT_FOUND;
  } else {
    Status = EFI_SUCCESS;
    PlatformCpuInfo = GET_GUID_HOB_DATA(Hob.Guid);
  }
  ASSERT_EFI_ERROR (Status);

  if( PlatformCpuInfo != NULL) {
    if (PlatformCpuInfo->CpuidMaxExtInputValue >= EFI_CPUID_VIRT_PHYS_ADDRESS_SIZE) {
      ValidMtrrAddressMask = (LShiftU64((UINT64) 1, PlatformCpuInfo->CpuAddress.PhysicalBits) - 1) & (~(UINT64)0x0FFF);
    }
  }

  //
  // Check for invalid parameter
  //
  if ((MemoryAddress & ~ValidMtrrAddressMask) != 0 || (MemoryLength & ~ValidMtrrAddressMask) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  if (MemoryLength == 0) {
    return EFI_INVALID_PARAMETER;
  }

  switch (MemoryCacheType) {
    case EFI_CACHE_UNCACHEABLE:
    case EFI_CACHE_WRITECOMBINING:
    case EFI_CACHE_WRITETHROUGH:
    case EFI_CACHE_WRITEPROTECTED:
    case EFI_CACHE_WRITEBACK:
      break;

    default:
      return EFI_INVALID_PARAMETER;
  }

  //
  // Check if Fixed MTRR
  //
  if ((MemoryAddress + MemoryLength) <= (1 << 20)) {
    Status = EFI_SUCCESS;
    EfiDisableCacheMtrr (&OldMtrr);
    while ((MemoryLength > 0) && (Status == EFI_SUCCESS)) {
      Status = PeiProgramFixedMtrr (MemoryCacheType, &MemoryAddress, &MemoryLength);
    }
    EfiRecoverCacheMtrr (TRUE, OldMtrr);
    return Status;
  }

  //
  // Search if the range attribute has been set before
  //
  Status = SearchForExactMtrr(PeiServices,
                              MemoryAddress,
                              MemoryLength,
                              ValidMtrrAddressMask,
                              &UsedMsrNum,
                              &UsedMemoryCacheType
                              );

  if (!EFI_ERROR(Status)) {
    //
    // Compare if it has the same type as current setting
    //
    if (UsedMemoryCacheType == MemoryCacheType) {
      return EFI_SUCCESS;
    } else {
      //
      // Different type
      //

      //
      // Check if the set type is the same as Default Type
      //
      if (IsDefaultType(MemoryCacheType)) {
        //
        // Clear the MTRR
        //
        AsmWriteMsr64(UsedMsrNum, 0);
        AsmWriteMsr64(UsedMsrNum + 1, 0);

        return EFI_SUCCESS;
      } else {
        //
        // Modify the MTRR type
        //
        EfiProgramMtrr(UsedMsrNum,
                       MemoryAddress,
                       MemoryLength,
                       MemoryCacheType,
                       ValidMtrrAddressMask
                       );
        return EFI_SUCCESS;
      }
    }
  }

  //
  // Find first unused MTRR
  //
  MsrNumEnd = EFI_MSR_CACHE_VARIABLE_MTRR_BASE + (2 * (UINT32)(AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP) & B_EFI_MSR_IA32_MTRR_CAP_VARIABLE_SUPPORT));
  for (MsrNum = EFI_MSR_CACHE_VARIABLE_MTRR_BASE; MsrNum < MsrNumEnd; MsrNum +=2) {
    if ((AsmReadMsr64(MsrNum+1) & B_EFI_MSR_CACHE_MTRR_VALID) == 0 ) {
      break;
    }
  }

  //
  // Reserve 1 MTRR pair for OS.
  //
  LastVariableMtrrForBios = MsrNumEnd - 1 - (EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS * 2);
  if (MsrNum > LastVariableMtrrForBios) {
    return EFI_LOAD_ERROR;
  }

  //
  // Special case for 1 MB base address
  //
  if (MemoryAddress == 0x100000) {
    MemoryAddress = 0;
  }

  //
  // Program MTRRs
  //
  TempQword = MemoryLength;

  if (TempQword == PeiPower2MaxMemory(MemoryAddress, TempQword)) {
    EfiProgramMtrr(MsrNum,
                   MemoryAddress,
                   MemoryLength,
                   MemoryCacheType,
                   ValidMtrrAddressMask
                   );

  } else {
    //
    // Fill in MTRRs with values.  Direction can not be checked for this method
    // as we are using WB as the default cache type and only setting areas to UC.
    //
    do {
      //
      // Do boundary check so we don't go past last MTRR register
      // for BIOS use.  Leave one MTRR pair for OS use.
      //
      if (MsrNum > LastVariableMtrrForBios) {
        return EFI_LOAD_ERROR;
      }

      //
      // Set next power of 2 region
      //
      MemoryLength = PeiPower2MaxMemory(MemoryAddress, TempQword);
      EfiProgramMtrr(MsrNum,
                     MemoryAddress,
                     MemoryLength,
                     MemoryCacheType,
                     ValidMtrrAddressMask
                     );
      MemoryAddress += MemoryLength;
      TempQword -= MemoryLength;
      MsrNum += 2;
    } while (TempQword);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PeiResetCacheAttributes (
  IN  EFI_PEI_SERVICES          **PeiServices,
  IN  PEI_CACHE_PPI             *This
  )
/*++

Routine Description:

    Reset all the MTRRs to a known state.

Arguments:

    PeiServices - General purpose services available to every PEIM.
    This        - Pointer to the instance of the PEI_CACHE_PPI.

Returns:

    EFI_SUCCESS - All MTRRs have been reset successfully.

--*/
{
  UINT32                      MsrNum, MsrNumEnd;
  UINT16                      Index;
  UINT64                      OldMtrr;
  UINT64                      CacheType;
  BOOLEAN                     DisableCar;
  Index = 0;
  DisableCar = TRUE;

  //
  // Determine default cache type
  //
  CacheType = EFI_CACHE_UNCACHEABLE;

  //
  // Set default cache type
  //
  AsmWriteMsr64(EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE, CacheType);

  //
  // Disable CAR
  //
  DisableCacheAsRam (PeiServices, DisableCar);

  EfiDisableCacheMtrr (&OldMtrr);

  //
  // Reset Fixed MTRRs
  //
  for (Index = 0; Index < V_EFI_FIXED_MTRR_NUMBER; Index++) {
    AsmWriteMsr64 (mFixedMtrrTable[Index].Msr, 0);
  }

  //
  // Reset Variable MTRRs
  //
  MsrNumEnd = EFI_MSR_CACHE_VARIABLE_MTRR_BASE + (2 * (UINT32)(AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP) & B_EFI_MSR_IA32_MTRR_CAP_VARIABLE_SUPPORT));
  for (MsrNum = EFI_MSR_CACHE_VARIABLE_MTRR_BASE; MsrNum < MsrNumEnd; MsrNum++) {
    AsmWriteMsr64 (MsrNum, 0);
  }

  //
  // Enable Fixed and Variable MTRRs
  //
  EfiRecoverCacheMtrr (TRUE, OldMtrr);

  return EFI_SUCCESS;
}

EFI_STATUS
SearchForExactMtrr (
  IN  EFI_PEI_SERVICES          **PeiServices,
  IN  EFI_PHYSICAL_ADDRESS      MemoryAddress,
  IN  UINT64                    MemoryLength,
  IN  UINT64                    ValidMtrrAddressMask,
  OUT UINT32                    *UsedMsrNum,
  OUT EFI_MEMORY_CACHE_TYPE     *UsedMemoryCacheType
  )
/*++

Routine Description:

  Search the memory cache type for specific memory from MTRR.

Arguments:

  PeiServices          - General purpose services available to every PEIM.
  MemoryAddress        - the address of target memory
  MemoryLength         - the length of target memory
  ValidMtrrAddressMask - the MTRR address mask
  UsedMsrNum           - the used MSR number
  UsedMemoryCacheType  - the cache type for the target memory

Returns:

  EFI_SUCCESS   - The memory is found in MTRR and cache type is returned
  EFI_NOT_FOUND - The memory is not found in MTRR

--*/
{
  UINT32                      MsrNum, MsrNumEnd;
  UINT64                      TempQword;

  if (MemoryLength == 0) {
    return EFI_INVALID_PARAMETER;
  }

  MsrNumEnd = EFI_MSR_CACHE_VARIABLE_MTRR_BASE + (2 * (UINT32)(AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP) & B_EFI_MSR_IA32_MTRR_CAP_VARIABLE_SUPPORT));
  for (MsrNum = EFI_MSR_CACHE_VARIABLE_MTRR_BASE; MsrNum < MsrNumEnd; MsrNum +=2) {
    TempQword = AsmReadMsr64(MsrNum+1);
    if ((TempQword & B_EFI_MSR_CACHE_MTRR_VALID) == 0) {
      continue;
    }

    if ((TempQword & ValidMtrrAddressMask) != ((~(MemoryLength - 1)) & ValidMtrrAddressMask)) {
      continue;
    }

    TempQword = AsmReadMsr64 (MsrNum);
    if ((TempQword & ValidMtrrAddressMask) != (MemoryAddress & ValidMtrrAddressMask)) {
      continue;
    }

    *UsedMemoryCacheType = (EFI_MEMORY_CACHE_TYPE)(TempQword & B_EFI_MSR_CACHE_MEMORY_TYPE);
    *UsedMsrNum = MsrNum;

    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

BOOLEAN
IsDefaultType (
  IN  EFI_MEMORY_CACHE_TYPE     MemoryCacheType
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MemoryCacheType - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  if ((AsmReadMsr64(EFI_MSR_CACHE_IA32_MTRR_DEF_TYPE) & B_EFI_MSR_CACHE_MEMORY_TYPE) != MemoryCacheType) {
    return FALSE;
  }

  return TRUE;
}

