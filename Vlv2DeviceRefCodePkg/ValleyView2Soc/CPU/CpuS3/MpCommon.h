/*++

Copyright (c) 2005 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  MpCommon.h

Abstract:

  some definitions for MP and HT driver.

--*/

#ifndef _MP_COMMON_
#define _MP_COMMON_

#ifdef ECP_FLAG
#include "EDKIIGluePeim.h"
#include "PchRegs.h"
#else
#include <PiPei.h>
#endif

#include <Guid/AcpiVariableCompatibility.h>

#ifndef ECP_FLAG
#include <Ppi/Stall.h>
#endif
#include <Ppi/SmmAccess.h>

#ifndef ECP_FLAG
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#endif

#include "../Include/CpuType.h"
#include "../Include/CpuRegs.h"
#include "../Include/CpuDataStruct.h"


#define TRIGGER_MODE_EDGE             0x0
#define TRIGGER_MODE_LEVEL            0x1
#define DELIVERY_MODE_INIT            0x5
#define DELIVERY_MODE_SIPI            0x6

#define VacantFlag                    0x00
#define NotVacantFlag                 0xff
                                      
#define MICROSECOND                   10
                                      
#define MAXIMUM_CPU_NUMBER            0x40
#define STACK_SIZE_PER_PROC           0x8000

#define MAXIMUM_CPU_S3_TABLE_SIZE     0x1000

#define IO_APIC_INDEX_REGISTER        0xFEC00000
#define IO_APIC_DATA_REGISTER         0xFEC00010

extern UINTN  FixedMtrrNumber;
extern UINTN  MtrrDefTypeNumber;
extern UINTN  VariableMtrrNumber;

typedef struct {
  UINT8                 APSerializeLock;

  UINT8                 Tm2Core2BusRatio;
  UINT8                 Tm2Vid;
  BOOLEAN               LimitCpuidMaximumValue;
  BOOLEAN               IsC1eSupported;
  BOOLEAN               IsXapicSupported;
  BOOLEAN               IsCCxSupported;

  UINT32                ApicID[MAXIMUM_CPU_NUMBER];

  UINT64                NumberOfCPUs;
  UINT64                NumberOfEnabledCPUs;
  UINT64                MaximumCPUsForThisSystem;

  UINTN                 mFinishedCount;
} HT_SYSTEM_DATA;

typedef struct {
  UINT16            Index;
  UINT64            Value;
} EFI_MTRR_VALUES;

typedef struct {
  UINT32            ApicId;
  UINT32            MsrIndex;
  UINT64            MsrValue;
} MP_CPU_S3_SCRIPT_DATA;

typedef struct {
  UINT32            S3BootScriptTable;
  UINT32            S3BspMtrrTable;
  UINT32            VirtualWireMode;
} MP_CPU_S3_DATA_POINTER;

typedef struct {
  UINT32                  Lock;
  UINT32                  StackStart;
  UINT32                  StackSize;
  UINT32                  ApFunction;
  IA32_DESCRIPTOR         GdtrProfile;
  IA32_DESCRIPTOR         IdtrProfile;
  UINT32                  BufferStart;
  UINT32                  PmodeOffset;
  UINT32                  AcpiCpuDataAddress;
  UINT32                  MtrrValuesAddress;
  UINT32                  FinishedCount;
  UINT32                  SerializeLock;
  UINT32                  MicrocodePointer;
  MP_CPU_S3_SCRIPT_DATA   *S3BootScriptTable;
  UINT32                  StartState;
  UINT32                  VirtualWireMode;
  BOOLEAN                 VerifyMicrocodeChecksum;
} MP_CPU_EXCHANGE_INFO;

VOID *
AsmGetPmodeOffset (
  );

VOID
ReadMtrrRegisters (
  UINT64  *MtrrValues
  );

VOID
MpMtrrSynchUp (
  UINT64  *MtrrValues
  );

VOID
SetBspMtrrRegisters (
  IN  EFI_MTRR_VALUES   *MtrrArray
  );

EFI_STATUS
InitializeMicrocode (
  IN      EFI_CPU_MICROCODE_HEADER   **MicrocodePointerBuffer,
  OUT     UINT32                     *FailedRevision,
  IN      BOOLEAN                    IsBsp,
  IN OUT  BOOLEAN                    *VerifyMicrocodeChecksum
  );

//
// Functions shared in MP/HT drivers
//
EFI_STATUS
SendInterrupt (
  IN  UINT32                               BroadcastMode,
  IN  UINT32                               ApicID,
  IN  UINT32                               VectorNumber,
  IN  UINT32                               DeliveryMode,
  IN  UINT32                               TriggerMode,
  IN  BOOLEAN                              Assert,
  IN EFI_PEI_SERVICES                      **PeiServices,
  IN EFI_PEI_STALL_PPI                     *PeiStall
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  BroadcastMode - GC_TODO: add argument description
  ApicID        - GC_TODO: add argument description
  VectorNumber  - GC_TODO: add argument description
  DeliveryMode  - GC_TODO: add argument description
  TriggerMode   - GC_TODO: add argument description
  Assert        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

UINT32
GetApicID (
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
ProgramVirtualWireMode (
  BOOLEAN                     BSP,
  UINT32                      VirtualWireMode
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  BSP - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
AsmAcquireMPLock (
  IN   UINT8            *Lock
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Lock  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
AsmReleaseMPLock (
  IN   UINT8            *Lock
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Lock  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID *
AsmGetAddressMap (
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  AddressMap  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
AsmCliHltLoop (
  );

VOID
CpuPause (
  VOID
  );

#endif
