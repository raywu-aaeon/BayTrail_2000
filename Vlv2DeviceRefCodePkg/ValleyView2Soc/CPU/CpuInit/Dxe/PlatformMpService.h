/**@file

  some definitions for MP services Protocol.


@copyright
 Copyright (c) 1999 - 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.

This file contains an 'Intel Peripheral Driver' and is uniquely
 identified as "Intel Reference Module" and is licensed for Intel
 CPUs and chipsets under the terms of your license agreement with
 Intel or your vendor. This file may be modified by the user, subject
 to additional terms of the license agreement.
**/
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

  PlatformMpService.h

Abstract:

  some definitions for MP services Protocol.


--*/

#ifndef _PLATFORM_MP_SERVICE_H_
#define _PLATFORM_MP_SERVICE_H_

#include "CpuDxe.h"
#include "MpCommon.h"
typedef struct {
  UINT32  Package;
  UINT32  Die;
  UINT32  Core;
  UINT32  Thread;
} PHYSICAL_LOCATION;

/*
#include "EfiHobLib.h"
#include "PeiHob.h"
#include "EfiScriptLib.h"
#include "MpCommon.h"

//
// Driver Produced Protocol.
//
#include EFI_PROTOCOL_PRODUCER (MpService)

//
// Driver Consumed Protcol Prototypes
//
#include EFI_GUID_DEFINITION (Hob)
#include EFI_GUID_DEFINITION (StatusCodeCallerId)
#include EFI_ARCH_PROTOCOL_DEFINITION (Cpu)
#include EFI_PROTOCOL_CONSUMER (LoadedImage)
#include EFI_GUID_DEFINITION (Acpi)
*/
//
// Constant definitions
//
#define FOURGB                                0x100000000
#define ONEPAGE                               0x1000

#define RENDEZVOUS_PROC_LENGTH                0x1000
#define STACK_SIZE_PER_PROC                   0x8000
#define MAX_CPU_S3_MTRR_ENTRY                 0x0020
#define MAX_CPU_S3_TABLE_SIZE                 0x0400

#define AP_HALT_CODE_SIZE                     10

#define CPU_CHECK_AP_INTERVAL                 0x10  // microseconds
//
//  The MP data structure follows.
//
#define CPU_SWITCH_STATE_IDLE                 0
#define CPU_SWITCH_STATE_STORED               1
#define CPU_SWITCH_STATE_LOADED               2

#define MSR_L3_CACHE_DISABLE                  0x40

typedef struct {
  UINT8             Lock;         // offset 0
  UINT8             State;        // offset 1
  UINTN             StackPointer; // offset 4 / 8
  IA32_DESCRIPTOR Gdtr;         // offset 8 / 16
  IA32_DESCRIPTOR Idtr;         // offset 14 / 26
} CPU_EXCHANGE_ROLE_INFO;

//
// MTRR table definitions
//
typedef struct {
  UINT16  Index;
  UINT64  Value;
} EFI_MTRR_VALUES;

typedef enum {
  CPU_STATE_IDLE,
  CPU_STATE_BLOCKED,
  CPU_STATE_READY,
  CPU_STATE_BUSY,
  CPU_STATE_FINISHED,
  CPU_STATE_DISABLED
} CPU_STATE;

//
// Define CPU feature information
//
#define MAX_FEATURE_NUM  6
typedef struct {
  UINTN                 Index;
  UINT32                ApicId;
  UINT32                Version;
  UINT32                FeatureDelta;
  UINT32                Features[MAX_FEATURE_NUM];
} LEAST_FEATURE_PROC;

//
// Define Individual Processor Data block.
//
typedef struct {
  UINT32                ApicID;
  EFI_AP_PROCEDURE      Procedure;
  VOID                  *Parameter;
  UINT8                 StateLock;
  UINT8                 ProcedureLock;
  EFI_MP_HEALTH_FLAGS   Health;
  BOOLEAN               SecondaryCpu;
  UINTN                 NumberOfCores;
  UINTN                 NumberOfThreads;
  UINT64                ActualFsbFrequency;
  EFI_STATUS            MicrocodeStatus;
  UINT32                FailedRevision;
  PHYSICAL_LOCATION     PhysicalLocation;
  CPU_STATE             State;
  EFI_EVENT             WaitEvent;
  EFI_EVENT             CheckThisAPEvent;
  UINTN                 StartedCpuNumber;
  BOOLEAN               *Finished;

  CPU_DATA_FOR_DATAHUB  CpuDataforDatahub;
} CPU_DATA_BLOCK;

typedef struct {
  UINT32                ApicId;
  UINT32                MsrIndex;
  UINT64                MsrValue;
} MP_CPU_S3_SCRIPT_DATA;

typedef struct {  
  UINT32                S3BootScriptTable;
  UINT32                S3BspMtrrTable;
  UINT32                VirtualWireMode;
} MP_CPU_S3_DATA_POINTER;

#pragma pack (1)
typedef struct {
  UINT32                ApicId;
  EFI_MP_HEALTH_FLAGS   Health;
} BIST_HOB_DATA;
#pragma pack ()

//
// Define MP data block which consumes individual processor block.
//
typedef struct {
  UINT8                       APSerializeLock;

  UINT8                       Tm2Core2BusRatio;       // for thermal monitor 2 initialization
  UINT8                       Tm2Vid;                 // for thermal monitor 2 initialization
  BOOLEAN                     LimitCpuidMaximumValue; // make processor look like < F40
  BOOLEAN                     EnableL3Cache;
  BOOLEAN                     IsC1eSupported;
  BOOLEAN                     C1eEnable;
  BOOLEAN                     AesEnable;
  BOOLEAN                     PeciEnable;
  BOOLEAN                     ProcessorVmxEnable;
  BOOLEAN                     ProcessorBistEnable;
  BOOLEAN                     ProcessorMsrLockControl;
  BOOLEAN                     Processor3StrikeControl;
  BOOLEAN                     LtEnable;
  BOOLEAN                     EchoTprDisable;
  BOOLEAN                     MonitorMwaitEnable;
  BOOLEAN                     ExecuteDisableBit;
  BOOLEAN                     FastString;
  BOOLEAN                     TurboModeEnable;
  BOOLEAN                     ExtremeEnable;
  BOOLEAN                     XapicEnable;
  BOOLEAN                     MachineCheckEnable;
  BOOLEAN                     MLCSpatialPrefetcherEnable;
  BOOLEAN                     MLCStreamerPrefetcherEnable;
  BOOLEAN                     DCUStreamerPrefetcherEnable;
  BOOLEAN                     DCUIPPrefetcherEnable;
  BOOLEAN                     CcxEnable;
  BOOLEAN                     C1AutoDemotion;
  BOOLEAN                     C3AutoDemotion;
  BOOLEAN                     Vr11Enable;
  UINT8                       PackageCState;

  BOOLEAN                     Gv3Enable;
  BOOLEAN                     PsdState;
  BOOLEAN                     EnableSecondaryCpu;
  BOOLEAN                     DcaEnable;
  UINTN                       DcaPrefetchDelayValue;

  BOOLEAN                     DCUModeSelection;
  BOOLEAN                     BiDirectionalProchot;

  UINTN                       NumberOfCpus;
  UINTN                       MaximumCpusForThisSystem;

  CPU_EXCHANGE_ROLE_INFO      BSPInfo;
  CPU_EXCHANGE_ROLE_INFO      APInfo;

  EFI_CPU_ARCH_PROTOCOL       *CpuArch;
  EFI_EVENT                   CheckThisAPEvent;
  EFI_EVENT                   CheckAllAPsEvent;
  EFI_EVENT                   WaitEvent;
  UINTN                       BSP;
  BIST_HOB_DATA              *BistHobData;
  UINTN                       BistHobSize;

  UINTN                       FinishCount;
  UINTN                       StartCount;
  EFI_AP_PROCEDURE            Procedure;
  VOID                        *ProcArguments;
  BOOLEAN                     SingleThread;
  UINTN                       StartedCpuNumber;
  UINT8                       Pad;

  CPU_DATA_BLOCK              CpuData[MAXIMUM_CPU_NUMBER];
  EFI_CPU_STATE_CHANGE_CAUSE  DisableCause[MAXIMUM_CPU_NUMBER];

  UINT8                       S3BootScriptLock;
  UINT32                      S3BootScriptCount;
  MP_CPU_S3_DATA_POINTER      S3DataPointer;
  MP_CPU_S3_SCRIPT_DATA       S3BootScriptTable[MAX_CPU_S3_TABLE_SIZE];
  EFI_MTRR_VALUES             S3BspMtrrTable[MAX_CPU_S3_MTRR_ENTRY];
  UINT8                       ActiveProcessorCores;
} MP_SYSTEM_DATA;

#pragma pack (1)

typedef struct {
  ACPI_VARIABLE_SET_COMPATIBILITY          AcpiVariableSet;
  MP_SYSTEM_DATA             MPSystemData;
  IA32_DESCRIPTOR            GdtrProfile;
  IA32_DESCRIPTOR            IdtrProfile;
  EFI_CPU_MICROCODE_HEADER*  MicrocodePointerBuffer[NUMBER_OF_MICROCODE_UPDATE + 1];
} MP_CPU_RESERVED_DATA;

#define CPU_MP_SERVICE_PRIVATE_SIGNATURE  EFI_SIGNATURE_32 ('m', 'p', '3', '2')

typedef struct {
  UINTN                     Signature;
  EFI_HANDLE                Handle;
  EFI_MP_SERVICES_PROTOCOL  MpService;
  MP_SYSTEM_DATA            MPSystemData;
} CPU_MP_SERVICE_PROTOCOL_PRIVATE;

#define CPU_MP_SERVICE_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      CPU_MP_SERVICE_PROTOCOL_PRIVATE, \
      MpService, \
      CPU_MP_SERVICE_PRIVATE_SIGNATURE \
      )
#pragma pack ()

extern CPU_MP_SERVICE_PROTOCOL_PRIVATE  *Private;
extern MP_SYSTEM_DATA                   *mMPSystemData;
extern ACPI_CPU_DATA_COMPATIBILITY      *mAcpiCpuData;

//
// Prototypes.
//
EFI_STATUS
MpInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ImageHandle - GC_TODO: add argument description
  SystemTable - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

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
  IN  BOOLEAN                             EnableOldBSPState
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  ProcessorNumber   - GC_TODO: add argument description
  EnableOldBSPState - GC_TODO: add argument description

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

EFI_STATUS
GetMPDataBlocks (
  IN     MP_SYSTEM_DATA  *MPSystemData
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MPSystemData  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
InitializeMpSystemData (
  VOID
  )
/*++

Routine Description:

  Initialize multiple processors and collect MP related data

Arguments:

  None

Returns:

  EFI_SUCCESS           - Multiple processors get initialized and data collected successfully
  Other                 - The operation failed due to some reason

--*/
;

//
// Assembly stub definitions.
//
VOID
AsmGetProcsParams1 (
  OUT  UINTN               MyGlobalID,
  OUT  UINTN               MyNumberOfProcessorCores,
  OUT  UINTN               MyNumberOfProcessorThreads,
  OUT  UINTN               RendezIntNumber
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MyGlobalID                  - GC_TODO: add argument description
  MyNumberOfProcessorCores    - GC_TODO: add argument description
  MyNumberOfProcessorThreads  - GC_TODO: add argument description
  RendezIntNumber             - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
AsmGetProcsParams2 (
  OUT  UINTN               MyHealthStatus,
  OUT  UINTN               MyNodeNumber,
  OUT  UINTN               MyNodeMemSize,
  OUT  UINTN               MyProcessorCompatibility
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MyHealthStatus            - GC_TODO: add argument description
  MyNodeNumber              - GC_TODO: add argument description
  MyNodeMemSize             - GC_TODO: add argument description
  MyProcessorCompatibility  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
AsmGetProcsParams3 (
  OUT  UINTN               MyProcessorTestMask,
  OUT  UINTN               MyProcessorSlotNumber,
  OUT  UINTN               MyProcessorPackageNumber
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MyProcessorTestMask       - GC_TODO: add argument description
  MyProcessorSlotNumber     - GC_TODO: add argument description
  MyProcessorPackageNumber  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
AsmFlushProgData (
  IN   UINTN               MemAddress,
  IN   UINTN               Count
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MemAddress  - GC_TODO: add argument description
  Count       - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
AsmWakeUpAPs (
  IN   UINTN               *WakeUpBuffer,
  IN   UINTN               MemAddress,
  IN   UINTN               *StackAddressStart,
  IN   UINTN               StackSize,
  IN   UINTN               *APDoneSemaphore,
  IN   UINTN               *GDTPageAddress
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  WakeUpBuffer      - GC_TODO: add argument description
  MemAddress        - GC_TODO: add argument description
  StackAddressStart - GC_TODO: add argument description
  StackSize         - GC_TODO: add argument description
  APDoneSemaphore   - GC_TODO: add argument description
  GDTPageAddress    - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
WakeUpAPs (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ImageHandle - GC_TODO: add argument description
  SystemTable - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
AsmExchangeRole (
  IN   CPU_EXCHANGE_ROLE_INFO    *MyInfo,
  IN   CPU_EXCHANGE_ROLE_INFO    *OthersInfo
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MyInfo      - GC_TODO: add argument description
  OthersInfo  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
FutureBSPProc (
  IN     MP_SYSTEM_DATA  *MPSystemData
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MPSystemData  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
AsmSendIPI (
  IN   UINTN               GlobalID,
  IN   UINTN               VectorNumber,
  IN   UINTN               DeliveryMode,
  IN   UINTN               *ErrorStatus,
  IN   UINTN               TriggerMode
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  GlobalID      - GC_TODO: add argument description
  VectorNumber  - GC_TODO: add argument description
  DeliveryMode  - GC_TODO: add argument description
  ErrorStatus   - GC_TODO: add argument description
  TriggerMode   - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
GetMpBistStatus (
  IN     MP_SYSTEM_DATA  *MPSystemData
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MPSystemData  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

//
// Function declarations
//
VOID
InitializeMpData (
  IN UINTN         ProcessorInstance
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ProcessorInstance - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
EFIAPI
MpServiceInitialize (
  VOID
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Event   - GC_TODO: add argument description
  Context - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

UINTN
MpMtrrSynchUpEntry (
  VOID
  )
/*++

Routine Description:

  Prepare for MTRR synchronization.

Arguments:

  None

Returns:

  CR4 value before changing.

--*/
;

VOID
MpMtrrSynchUpExit (
  UINTN Cr4
  )
/*++

Routine Description:

  Restoration after MTRR synchronization.

Arguments:

  Cr4 - CR4 value before changing.

Returns:

  None

--*/
;

VOID
SaveBspMtrrForS3 (
  );

EFI_STATUS
FillInProcessorInformation (
  IN     MP_SYSTEM_DATA       *MPSystemData,
  IN     BOOLEAN              BSP,
  IN     UINT32               BistParam
  )
/*++

Routine Description:

  This function is called by all processors (both BSP and AP) once and collects MP related data

Arguments:

  MPSystemData  - Pointer to the data structure containing MP related data
  BSP           - TRUE if the CPU is BSP
  BistParam     - BIST (build-in self test) data for the processor. This data
                  is only valid for processors that are waked up for the 1st
                  time in this CPU DXE driver.

Returns:

  EFI_SUCCESS   - Data for the processor collected and filled in

--*/
;

VOID
APFinishTask (
  IN  EFI_EVENT                        Event,
  IN  VOID                             *Context
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Event   - GC_TODO: add argument description
  Context - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
OverrideCpuData (
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
SetApicBSPBit (
  IN  BOOLEAN   Enable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Enable  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
SwitchToLowestFeatureProcess (
  IN EFI_MP_SERVICES_PROTOCOL               *MpServices
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MpServices  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
CollectDetailedInfo (
  IN  UINTN                       CpuNumber,
  OUT EFI_DETAILED_CPU_INFO       *DetailedInfo
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber     - GC_TODO: add argument description
  DetailedInfo  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
ChangeCpuState (
  IN     UINTN                      CpuNumber,
  IN     BOOLEAN                    NewState,
  IN     EFI_CPU_STATE_CHANGE_CAUSE Cause
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber - GC_TODO: add argument description
  NewState  - GC_TODO: add argument description
  Cause     - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

BOOLEAN
IsSecondaryThread (
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
ExistL3Cache (
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
CheckAllAPsStatus (
  IN  EFI_EVENT                           Event,
  IN  VOID                                *Context
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Event   - GC_TODO: add argument description
  Context - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
CheckThisAPStatus (
  IN  EFI_EVENT                           Event,
  IN  VOID                                *Context
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Event   - GC_TODO: add argument description
  Context - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

UINT64
CalculateTimeout (
  IN  UINTN                               TimeoutInMicroSecs
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  TimeoutInMicroSecs  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

BOOLEAN
CheckTimeout (
  IN  UINT64                              ExpectedTsc
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ExpectedTsc - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
GetNextBlockedCpuNumber (
  OUT UINTN                               *NextCpuNumber
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  NextCpuNumber - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
UpdateDataforDatahub (
  IN  UINTN                               CpuNumber,
  OUT CPU_DATA_FOR_DATAHUB                *CpuDataforDatahub
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber         - GC_TODO: add argument description
  CpuDataforDatahub - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;


VOID
DetailedInitialization (
  VOID
  )
/*++

Routine Description:

  Procedure for detailed initialization of APs. It will be assigned to all APs while
  they are waken up for the second time.

Arguments:

  None.

Returns:

  None.

--*/
;

VOID
WakeUpAp (
  IN   CPU_DATA_BLOCK        *CpuData,
  IN   EFI_AP_PROCEDURE      Procedure,
  IN   VOID                  *Parameter
  )
/*++

Routine Description:

  Function to wake up a specified AP and assign procedure to it.

Arguments:

  CpuData       - CPU data block for the specified AP.
  Procedure     - Procedure to assign.
  ProcArguments - Argument for Procedure.

Returns:

  None

--*/
;

UINT8
GetCoreNumber (
  VOID
  )
/*++

Routine Description:

  Check number of cores in the package.

Arguments:

  None

Returns:

  Number of cores in the package.

--*/
;

#endif
