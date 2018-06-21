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

  MpCommon.h

Abstract:

  some definitions for MP and HT driver.

--*/

#ifndef _MP_COMMON_
#define _MP_COMMON_

#include "CpuDxe.h"
#include "Processor.h"
#include "Cache.h"
#include "Exception.h"
#include "ProcessorDef.h"

#ifdef ECP_FLAG
#include <Protocol/GenericMemoryTest/GenericMemoryTest.h>
#else
#include <Protocol/GenericMemoryTest.h>
#endif


//
// Protocol produced by this driver
//
//#include EFI_PROTOCOL_PRODUCER (MpService)
//
////
//// Protocol consumed by this driver
////
//#ifndef EFI_NO_MEMORY_TEST
//#include EFI_PROTOCOL_DEFINITION (GenericMemoryTest)
//#endif
//#include EFI_PROTOCOL_DEFINITION (PlatformCpu)
//#include EFI_PROTOCOL_DEFINITION (LegacyBios)
//#include EFI_PROTOCOL_DEFINITION (LegacyRegion)
//
//#include EFI_GUID_DEFINITION (HOB)
//#include EFI_GUID_DEFINITION (HtBistHOB)
//#include EFI_GUID_DEFINITION (StatusCodeDataTypeId)

#ifndef EFI_NO_MEMORY_TEST
extern EFI_GENERIC_MEMORY_TEST_PROTOCOL *mGenMemoryTest;
#define CompatibleMemoryRangeTestIfExist(This,               \
                                  StartAddress,              \
                                  Length)                    \
        (This)->CompatibleRangeTest ((This),                 \
                                     (StartAddress),         \
                                     (Length))

#else
#define CompatibleMemoryRangeTestIfExist(This,               \
                                  StartAddress,              \
                                  Length)                    \
        EFI_SUCCESS
#endif

#define VacantFlag              0x00
#define NotVacantFlag           0xff

#define MICROSECOND             10

#define MAXIMUM_CPU_NUMBER      0x40
#define STACK_SIZE_PER_PROC     0x8000

#define IO_APIC_INDEX_REGISTER  0xFEC00000
#define IO_APIC_DATA_REGISTER   0xFEC00010
#define VIRT_WIRE_A             0

//
// Data structure used in MP/HT driver
//
#define MP_CPU_EXCHANGE_INFO_OFFSET     (0x1000 - 0x400)
#define MP_CPU_LEGACY_RESET_INFO_OFFSET (0x100 - 0x20)

#pragma pack(1)
#define SIZE_OF_MCE_HANDLER 16

typedef struct {
  UINT16  LimitLow;
  UINT16  BaseLow;
  UINT8   BaseMiddle;
  UINT16  Attributes;
  UINT8   BaseHigh;
} SEGMENT_DESCRIPTOR;

#pragma pack()

typedef struct {
  UINT32                    Number;
  UINT32                    BIST;
} BIST_INFO;

typedef enum {
  WakeUpApCounterInit = 0,
  WakeUpApPerHltLoop  = 1,
  WakeUpApPerMwaitLoop= 2,
  WakeUpApPerRunLoop  = 3
} WAKEUP_AP_MANNER;

typedef struct {
  UINTN             Lock;
  VOID              *StackStart;
  UINTN             StackSize;
  VOID              *ApFunction;
  IA32_DESCRIPTOR   GdtrProfile;
  IA32_DESCRIPTOR   IdtrProfile;
  UINT32            BufferStart;
  UINT32            Cr3;
  UINT32            InitFlag;
  WAKEUP_AP_MANNER  WakeUpApManner;
  BIST_INFO         BistBuffer[MAXIMUM_CPU_NUMBER];
} MP_CPU_EXCHANGE_INFO;

extern ACPI_CPU_DATA_COMPATIBILITY *mAcpiCpuData;

//
// Protocol interface functions
//
EFI_STATUS
EFIAPI
GetGeneralMPInfo (
  IN  VOID                                *This,
  OUT UINTN                               *NumberOfCPUs,
  OUT UINTN                               *MaxiumNumberOfCPUs,
  OUT UINTN                               *NumberOfEnabledCPUs,
  OUT UINTN                               *RendezvousIntNumber,
  OUT UINTN                               *RendezvousProcLength
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This                  - GC_TODO: add argument description
  NumberOfCPUs          - GC_TODO: add argument description
  MaxiumNumberOfCPUs    - GC_TODO: add argument description
  NumberOfEnabledCPUs   - GC_TODO: add argument description
  RendezvousIntNumber   - GC_TODO: add argument description
  RendezvousProcLength  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
GetProcessorContext (
  IN  VOID                                *This,
  IN  UINTN                               ProcessorNumber,
  IN  OUT UINTN                           *BufferLength,
  IN  OUT  EFI_MP_PROC_CONTEXT            *ProcessorContextBuffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This                    - GC_TODO: add argument description
  ProcessorNumber         - GC_TODO: add argument description
  BufferLength            - GC_TODO: add argument description
  ProcessorContextBuffer  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
StartupThisAP (
  IN  VOID                                                   * This,
  IN  EFI_AP_PROCEDURE                                       Procedure,
  IN  UINTN                                                  ProcessorNumber,
  IN  EFI_EVENT                                              WaitEvent OPTIONAL,
  IN  UINTN                                                  TimeoutInMicroSecs OPTIONAL,
  IN  OUT VOID                                               *ProcArguments OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This                - GC_TODO: add argument description
  Procedure           - GC_TODO: add argument description
  ProcessorNumber     - GC_TODO: add argument description
  WaitEvent           - GC_TODO: add argument description
  TimeoutInMicroSecs  - GC_TODO: add argument description
  ProcArguments       - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
StartupAllAPs (
  IN  VOID                                                   * This,
  IN  EFI_AP_PROCEDURE                                       Procedure,
  IN  BOOLEAN                                                SingleThread,
  IN  EFI_EVENT                                              WaitEvent OPTIONAL,
  IN  UINTN                                                  TimeoutInMicroSecs OPTIONAL,
  IN  OUT VOID                                               *ProcArguments OPTIONAL,
  OUT UINTN                                                  *FailedCPUList OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This                - GC_TODO: add argument description
  Procedure           - GC_TODO: add argument description
  SingleThread        - GC_TODO: add argument description
  WaitEvent           - GC_TODO: add argument description
  TimeoutInMicroSecs  - GC_TODO: add argument description
  ProcArguments       - GC_TODO: add argument description
  FailedCPUList       - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SwitchBSP (
  IN  VOID                                *This,
  IN  UINTN                               ProcessorNumber,
  IN  BOOLEAN                             OldBSPState
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This            - GC_TODO: add argument description
  ProcessorNumber - GC_TODO: add argument description
  OldBSPState     - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SendIPI (
  IN  VOID                                *This,
  IN  UINTN                               ProcessorNumber,
  IN  UINTN                               VectorNumber,
  IN  UINTN                               DeliveryMode
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This            - GC_TODO: add argument description
  ProcessorNumber - GC_TODO: add argument description
  VectorNumber    - GC_TODO: add argument description
  DeliveryMode    - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
EnableDisableAP (
  IN  VOID                                * This,
  IN  UINTN                               ProcessorNumber,
  IN  BOOLEAN                             NewAPState,
  IN  EFI_MP_HEALTH                       * HealthState OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This            - GC_TODO: add argument description
  ProcessorNumber - GC_TODO: add argument description
  NewAPState      - GC_TODO: add argument description
  HealthState     - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
WhoAmI (
  IN  VOID                                *This,
  OUT UINTN                               *ProcessorNumber
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This            - GC_TODO: add argument description
  ProcessorNumber - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

//
// Functions shared in MP/HT drivers
//
EFI_STATUS
SendInterrupt (
  IN  UINT32                              BroadcastMode,
  IN  UINT32                              ApicID,
  IN  UINT32                              VectorNumber,
  IN  UINT32                              DeliveryMode,
  IN  UINT32                              TriggerMode,
  IN  BOOLEAN                             Assert
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
  OUT EFI_PHYSICAL_ADDRESS      * ApicBase OPTIONAL,
  OUT UINT32                    *ApicVersionNumber OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ApicBase          - GC_TODO: add argument description
  ApicVersionNumber - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
ProgramVirtualWireMode (
  IN   BOOLEAN                  BSP
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

EFI_STATUS
AllocateWakeUpBuffer (
  OUT EFI_PHYSICAL_ADDRESS          *WakeUpBuffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  WakeUpBuffer  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

//
// Assembly functions implemented in MP/HT drivers
//
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

VOID
AsmGetGdtrIdtr (
  OUT IA32_DESCRIPTOR         **Gdt,
  OUT IA32_DESCRIPTOR         **Idt
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Gdt - GC_TODO: add argument description
  Idt - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
PrepareGdtIdtForAP (
  OUT IA32_DESCRIPTOR          *GDTR,
  OUT IA32_DESCRIPTOR          *IDTR
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  GDTR  - GC_TODO: add argument description
  IDTR  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
AllocateAlignedReservedMemory (
  IN  UINTN         Size,
  IN  UINTN         Alignment,
  OUT VOID          **Pointer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Size      - GC_TODO: add argument description
  Alignment - GC_TODO: add argument description
  Pointer   - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
FillinDataforDataHub (
  IN   UINTN                            CpuNumber,
  OUT  CPU_DATA_FOR_DATAHUB             *CpuDataForDatahub
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber         - GC_TODO: add argument description
  CpuDataForDatahub - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

UINT64
AsmGetCr3 (
  VOID
  )
/*++

Routine Description:

  Get CR3 register's value.

Arguments:

  None

Returns:

  CR3 register's value.

--*/
;

VOID
ReAllocateMemoryForAP (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
/*++

Routine Description:

  This function is invoked when LegacyBios protocol is installed, we must
  allocate reserved memory under 1M for AP.

Arguments:

  Event   - The triggered event.
  Context - Context for this event.

Returns:

  None

--*/
;

VOID
ResetAPs (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
/*++

Routine Description:

  This function is invoked by EFI_EVENT_SIGNAL_LEGACY_BOOT.
  Before booting to legacy OS, reset it with memory allocated
  by ReAllocateMemoryForAp() and set local APIC correctly.

Arguments:

  Event   - The triggered event.
  Context - Context for this event.

Returns:

  None

--*/
;

EFI_STATUS
PrepareMemoryForAPs (
  OUT EFI_PHYSICAL_ADDRESS       *WakeUpBuffer,
  OUT VOID                       **StackAddressStart,
  IN UINTN                       MaximumCPUsForThisSystem
  )
/*++

Routine Description:

  Prepare Wakeup Buffer and stack for APs.

Arguments:

  WakeUpBuffer             - Pointer to the address of wakeup buffer for output.
  StackAddressStart        - Pointer to the stack address of APs for output.
  MaximumCPUsForThisSystem - Maximum CPUs in this system.

Returns:

  EFI_SUCCESS              - Memory successfully prepared for APs.
  Other                    - Error occurred while allocating memory.

--*/
;

EFI_STATUS
PrepareExchangeInfo (
  OUT MP_CPU_EXCHANGE_INFO           *ExchangeInfo,
  IN  VOID                           *StackAddressStart,
  IN  VOID                           *ApFunction,
  IN  EFI_PHYSICAL_ADDRESS           WakeUpBuffer
  )
/*++

Routine Description:

  Prepare exchange information for APs.

Arguments:

  ExchangeInfo      - Pointer to the exchange info buffer for output.
  StackAddressStart - Start address of APs' stacks.

Returns:

  EFI_SUCCESS       - Exchange Info successfully prepared for APs.
  Other             - Error occurred while allocating memory.

--*/
;

EFI_STATUS
S3PrepareMemoryForAPs (
  OUT EFI_PHYSICAL_ADDRESS       *WakeUpBuffer,
  OUT VOID                       **StackAddressStart
  )
/*++

Routine Description:

  Prepare Wakeup Buffer and stack for APs during S3.

Arguments:

  WakeUpBuffer      - Pointer to the address of wakeup buffer for output.
  StackAddressStart - Pointer to the stack address of APs for output.

Returns:

  EFI_SUCCESS       - Memory successfully prepared for APs.

--*/
;

EFI_STATUS
S3PrepareExchangeInfo (
  OUT MP_CPU_EXCHANGE_INFO           *ExchangeInfo,
  IN  VOID                           *StackAddressStart,
  IN  VOID                           *ApFunction,
  IN  EFI_PHYSICAL_ADDRESS           WakeUpBuffer
  )
/*++

Routine Description:

  Prepare exchange information for APs during S3.

Arguments:

  ExchangeInfo      - Pointer to the exchange info for output.
  StackAddressStart - Start address of APs' stacks.

Returns:

  EFI_SUCCESS       - Exchange Info successfully prepared for APs.

--*/
;

BOOLEAN
ApRunning (
  VOID
  )
/*++

Routine Description:

  Check whether any AP is running for assigned task.

Arguments:

  None

Returns:

  TRUE  - Some APs are running.
  FALSE - No AP is running.

--*/
;

VOID
ApProcWrapper (
  VOID
  )
/*++

Routine Description:

  Wrapper function for all procedures assigned to AP via MP service protocol.
  It controls states of AP and invokes assigned precedure.

Arguments:

  None.

Returns:

  None

--*/
;

UINTN
SetIdtEntry (
  IN  UINTN                       FunctionPointer,
  OUT INTERRUPT_GATE_DESCRIPTOR   *IdtEntry
  )
/*++

Routine Description:

  Set specified IDT entry with given function pointer.

Arguments:

  FunctionPointer - Function pointer for IDT entry.
  IdtEntry        - The IDT entry to update.

Returns:

  The original IDT entry value.

--*/
;

EFI_STATUS
AllocateReservedMemoryBelow4G (
  IN   UINTN   Size,
  OUT  VOID    **Buffer
  )
/*++

Routine Description:

  Allocate EfiACPIMemoryNVS below 4G memory address.

Arguments:

  Size   - Size of memory to allocate.
  Buffer - Allocated address for output.

Returns:

  EFI_SUCCESS - Memory successfully allocated.
  Other       - Other errors occur.

--*/
;

VOID
RedirectFarJump (
  VOID
  )
/*++

Routine Description:

  Dynamically write the far jump destination in APs' wakeup buffer,
  in order to refresh APs' CS registers for mode switching.

Arguments:

  None.

Returns:

  None

--*/
;

VOID
LegacyRegionAPCount (
  VOID
  )
/*++

Routine Description:

  Count the number of APs that have been switched
  to E0000 or F0000 segments by ReAllocateMemoryForAP().

Arguments:

  None.

Returns:

  None;

--*/
;

#endif
