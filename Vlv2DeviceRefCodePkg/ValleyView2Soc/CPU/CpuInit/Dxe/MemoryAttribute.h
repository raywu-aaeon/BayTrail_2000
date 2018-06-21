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

    MemoryAttribute.h

Abstract:


Revision History:

--*/

#ifndef _EFI_MEMORY_ATTRIB_H
#define _EFI_MEMORY_ATTRIB_H

extern UINT32 mUsedMtrr;

typedef struct {
  UINT32  Msr;
  UINT32  BaseAddress;
  UINT32  Length;
} EFI_FIXED_MTRR;

typedef struct {
  UINT64  BaseAddress;
  UINT64  Length;
  UINT64  Type;
  UINT32  Msr;
  BOOLEAN Valid;
} EFI_VARIABLE_MTRR;

#define EFI_MEMORY_CACHETYPE_MASK (EFI_MEMORY_UC | EFI_MEMORY_WC | EFI_MEMORY_WT | EFI_MEMORY_WB | EFI_MEMORY_UCE)

EFI_STATUS
ProgramFixedMtrr (
  IN  UINT64                    MemoryCacheType,
  IN  UINT64                    *Base,
  IN  UINT64                    *Length
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MemoryCacheType - GC_TODO: add argument description
  Base            - GC_TODO: add argument description
  Length          - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

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
;
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
;

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

  GC_TODO: add return values

--*/
;

BOOLEAN
CheckMemoryAttributeOverlap (
  IN  EFI_PHYSICAL_ADDRESS      Start,
  IN  EFI_PHYSICAL_ADDRESS      End
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
;

EFI_STATUS
CombineMemoryAttribute (
  IN  UINT64                    Attribute,
  IN  UINT64                    *Base,
  IN  UINT64                    *Length
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Attribute - GC_TODO: add argument description
  Base      - GC_TODO: add argument description
  Length    - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
GetDirection (
  IN  UINT64                    Input,
  IN  UINTN                     *MtrrNumber,
  IN  BOOLEAN                   *Direction
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Input       - GC_TODO: add argument description
  MtrrNumber  - GC_TODO: add argument description
  Direction   - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

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
;

EFI_STATUS
InvariableMtrr (
  IN  UINTN                     MtrrNumber,
  IN  UINTN                     Index
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MtrrNumber  - GC_TODO: add argument description
  Index       - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
ProgramVariableMtrr (
  IN  UINTN                     MtrrNumber,
  IN  EFI_PHYSICAL_ADDRESS      BaseAddress,
  IN  UINT64                    Length,
  IN  UINT64                    MemoryCacheType
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

  GC_TODO: add return values

--*/
;

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

  GC_TODO: add return values

--*/
;

UINT64
GetMemorySpaceAttributeFromMtrrType (
  IN UINT8                      MtrrAttribute
  )
/*++

Routine Description:

  Get GCD Mem Space type from Mtrr Type

Arguments:

  MtrrAttribute - Mtrr type

Returns:

  GCD Mem Space typed (64-bit)

--*/
;

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
;

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
;

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
;
#ifdef ECP_FLAG
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
#endif
