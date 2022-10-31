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

  CpuDxe.h

Abstract:

  Private data structures


--*/

#ifndef _CPU_DXE_H
#define _CPU_DXE_H

#ifdef ECP_FLAG
#include "EDKIIGlueDxe.h"
#include "MpCpuStrDefs.h"
//#include "UefiIfrLibrary.h"
#include "PchRegs.h"
#include <Guid/StatusCodeDataTypeId/StatusCodeDataTypeId.h>
#include <Guid/DataHubRecords/DataHubRecords.h>
#include <Guid/AcpiVariableCompatibility.h>
#include <Guid/GlobalVariable/GlobalVariable.h>
#include <ArchProtocol/Cpu/Cpu.h>
#include <Protocol/MpService.h>
#include <Protocol/FrameworkMpService.h>
#include <ArchProtocol/Metronome/Metronome.h>
#include <Protocol/PlatformCpu.h>
#include <Protocol/DataHub/DataHub.h>
#include <Protocol/SmmBase/SmmBase.h>
#include <Protocol/SmmAccess/SmmAccess.h>
#include <Protocol/SmmControl/SmmControl.h>
#include <Protocol/LegacyBios/LegacyBios.h>
#include <Protocol/ExitPmAuth/ExitPmAuth.h>
#include <Protocol/HiiString/HiiString.h>
#include <Protocol/HiiDatabase/HiiDatabase.h>
#include <Guid/HtBistHob.h>
#include <Guid/PowerOnHob.h>
#else
#include <FrameworkDxe.h>

#include <Guid/StatusCodeDataTypeId.h>
#include <Guid/DataHubRecords.h>
#include <Guid/AcpiVariableCompatibility.h>
#include <Guid/GlobalVariable.h>

#include <Protocol/Cpu.h>
#include <Protocol/MpService.h>
#include <Protocol/FrameworkMpService.h>
#include <Protocol/Metronome.h>
#include <Protocol/PlatformCpu.h>
#include <Protocol/DataHub.h>
#include <Protocol/FrameworkHii.h>
#include <Protocol/SmmBase.h>
#include <Protocol/SmmAccess2.h>
#include <Protocol/SmmControl.h>
#include <Protocol/LegacyBios.h>
#include <Protocol/ExitPmAuth.h>
#include <Protocol/DxeSmmReadyToLock.h>

#include <Library/CpuConfigLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>
#include <Library/HiiLib.h>
#include <Library/DxeServicesTableLib.h>
#endif

#include "CpuType.h"
#include "CpuRegs.h"
#include "CpuDataStruct.h"
#ifdef ECP_FLAG
typedef struct {
  UINT32  RegEax;
  UINT32  RegEbx;
  UINT32  RegEcx;
  UINT32  RegEdx;
} EFI_CPUID_REGISTER;
#endif
#include <Guid/PlatformCpuInfo.h>
#include <Guid/Vlv2Variable.h>

#define BSEL_CR_OVERCLOCK_CONTROL   0xCD
#define FUSE_BSEL_MASK              0x03

#define INTERRUPT_VECTOR_NUMBER     256
#define INTERRUPT_GATE_ATTRIBUTE    0x8e00
#define NUMBER_OF_MICROCODE_UPDATE  10

extern EFI_GUID gProcessorProducerGuid;

extern EFI_GUID gEfiPowerOnHobGuid;
extern EFI_GUID gEfiHtBistHobGuid;

#define TRIGGER_MODE_EDGE             0x0
#define TRIGGER_MODE_LEVEL            0x1
#define SINGLE_THREAD_BOOT_FLAG       0

#define   EfiProcessorFamilyIntelAtom 0x2B

#define SMM_FROM_SMBASE_DRIVER        0x55

#define SMM_FROM_SMBASE_DRIVER_BOOTTIME          0x0
#define SMM_FROM_SMBASE_DRIVER_RUNTIME           0x1
#define SMM_FROM_SMBASE_DRIVER_LOCK              0x2
//
// This value should be same as the one in CPU driver.
//
#define SMM_FROM_CPU_DRIVER_SAVE_INFO            0x81

#define EFI_CU_HP_PC_DXE_INIT                     (EFI_SUBCLASS_SPECIFIC | 0x00000020)
#define EFI_CU_HP_PC_DXE_STEP1                    (EFI_SUBCLASS_SPECIFIC | 0x00000021)
#define EFI_CU_HP_PC_DXE_STEP2                    (EFI_SUBCLASS_SPECIFIC | 0x00000022)
#define EFI_CU_HP_PC_DXE_STEP3                    (EFI_SUBCLASS_SPECIFIC | 0x00000023)
#define EFI_CU_HP_PC_DXE_STEP4                    (EFI_SUBCLASS_SPECIFIC | 0x00000024)
#define EFI_CU_HP_PC_DXE_END                      (EFI_SUBCLASS_SPECIFIC | 0x0000002F)
#define EfiMakeCpuVersion(f, m, s)         \
  (((UINT32) (f) << 16) | ((UINT32) (m) << 8) | ((UINT32) (s)))

extern
VOID
InitializeSelectors (
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

//
// This is the string tool generated data representing our strings.
//
extern UINT8                      STRING_ARRAY_NAME[];

typedef struct {
  VOID  *Start;
  UINTN Size;
  UINTN FixOffset;
} INTERRUPT_HANDLER_TEMPLATE_MAP;

typedef union {
  EFI_CPU_DATA_RECORD *DataRecord;
  UINT8               *Raw;
} EFI_CPU_DATA_RECORD_BUFFER;

//
// This constant defines the maximum length of the CPU brand string. According to the
// IA manual, the brand string is in EAX through EDX (thus 16 bytes) after executing
// the CPUID instructions with EAX as 80000002, 80000003, 80000004.
//
#define MAXIMUM_CPU_BRAND_STRING_LENGTH 48

typedef struct {
  BOOLEAN                     StringValid;
  CHAR16                      BrandString[MAXIMUM_CPU_BRAND_STRING_LENGTH + 1];
  EFI_PROCESSOR_VERSION_DATA  StringRef;
} PROCESSOR_VERSION_INFORMATION;

//
// The constant defines how many times the Cpuid instruction should be executed
// in order to get all the cache information. For Pentium 4 processor, 1 is enough
//
#define CPU_CPUID_EXECTION_COUNT  2

typedef struct {
  UINT64                                  IntendCoreFrequency;
  UINT64                                  IntendFsbFrequency;
  PROCESSOR_VERSION_INFORMATION           Version;
  EFI_PROCESSOR_MANUFACTURER_DATA         Manufacturer;
  EFI_PROCESSOR_ID_DATA                   CpuidData;
  EFI_PROCESSOR_FAMILY_DATA               Family;
  INT16                                   Voltage;
  EFI_PROCESSOR_APIC_BASE_ADDRESS_DATA    ApicBase;
  EFI_PROCESSOR_APIC_ID_DATA              ApicID;
  EFI_PROCESSOR_APIC_VERSION_NUMBER_DATA  ApicVersion;
  UINT32                                  MicrocodeRevision;
  EFI_PROCESSOR_STATUS_DATA               Status; // Need to update this field before report
  EFI_CPU_PHYSICAL_LOCATION               Location;
  EFI_MP_HEALTH_FLAGS                     Health;
  EFI_CPUID_REGISTER                      CacheInformation[CPU_CPUID_EXECTION_COUNT];
  EFI_PROCESSOR_CORE_COUNT_DATA           CoreCount;
  EFI_PROCESSOR_ENABLED_CORE_COUNT_DATA   CoreEnabled;
  EFI_PROCESSOR_THREAD_COUNT_DATA         ThreadCount;
  EFI_PROCESSOR_CHARACTERISTICS_DATA      ProcessorCharacteristics;
} CPU_DATA_FOR_DATAHUB;

//
// Type, FamilyId and Model values for the different processors
// [0:7]   - Model
// [8:23]  - FamilyId
// [24:31] - Type
//
#define RESERVED  0x00

//
// Function declarations
//
EFI_STATUS
InitializeMicrocode (
  IN      EFI_CPU_MICROCODE_HEADER   **MicrocodePointerBuffer,
  OUT     UINT32                     *FailedRevision,
  IN      BOOLEAN                    IsBsp
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MicrocodePointerBuffer  - GC_TODO: add argument description
  FailedRevision          - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
InitializeMpSupport (
  IN EFI_HANDLE                       ImageHandle,
  IN EFI_SYSTEM_TABLE                 *SystemTable
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
ReadMtrrRegisters (
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
EFIAPI
MpMtrrSynchUp (
  IN VOID                             *Buffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Buffer  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
LoadAllMicrocodeUpdates (
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
FreeAllMicrocodeUpdates (
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
CheckIncompatibleFsb (
  IN  UINTN                  CpuNumber,
  IN  UINT64                 ActualFsb,
  IN  UINT64                 IntendFsb
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber - GC_TODO: add argument description
  ActualFsb - GC_TODO: add argument description
  IntendFsb - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
CheckBspBusSpeed (
  IN  EFI_METRONOME_ARCH_PROTOCOL *Metronome
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Metronome - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
GetActualFrequency (
  IN  EFI_METRONOME_ARCH_PROTOCOL   *Metronome,
  OUT UINT64                        *Frequency
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Metronome - GC_TODO: add argument description
  Frequency - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;
UINT16
DetermineiFsbFromMsr (
  VOID
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Ratio - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
Actual2StandardFrequency (
  IN  UINT64                        Actual,
  IN  UINT32                        Ratio,
  OUT UINT64                        *Standard
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Actual    - GC_TODO: add argument description
  Ratio     - GC_TODO: add argument description
  Standard  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
InitializePlatformCpuPtr (
  OUT EFI_PLATFORM_CPU_PROTOCOL     **PlatformCpu
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PlatformCpu - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
InitializeDataHubPtr (
  OUT EFI_DATA_HUB_PROTOCOL        **DataHub
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  DataHub - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EnableCpuIdMaximumValueLimit (
  BOOLEAN                     LimitCpuidMaximumValue
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  LimitCpuidMaximumValue  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
CheckMicrocodeUpdate (
  IN  UINTN              CpuNumber,
  IN  EFI_STATUS         Status,
  IN  UINT32             FailedRevision
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber       - GC_TODO: add argument description
  Status          - GC_TODO: add argument description
  FailedRevision  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
GetTemplateAddressMap (
  OUT INTERRUPT_HANDLER_TEMPLATE_MAP *AddressMap
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
CpuEnableInterrupt (
  VOID
  )
/*++
Routine Description:
  Enable Cpu Interrupt
Arguments:
  None
Returns:
  None
--*/
;

VOID
CpuDisableInterrupt (
  VOID
  )
/*++
Routine Description:
  Disable Cpu Interrupt
Arguments:
  None
Returns:
  None
--*/
;

UINT16
GetCodeSegment (
  VOID
  )
/*++
Routine Description:
  Get code segment
Arguments:
  None
Returns:
  Code segmnet value
--*/
;

VOID
CpuInitFloatPointUnit (
  VOID
  )
/*++
Routine Description:
  Initialize Cpu float point unit
Arguments:
  None
Returns:
  None
--*/
;

VOID
CpuPause (
  VOID
  )
/*++
Routine Description:
  Pause Cpu
Arguments:
  None
Returns:
  None
--*/
;

//
// Structures
//
typedef struct {
  EFI_HANDLE            Handle;

  EFI_CPU_ARCH_PROTOCOL Cpu;

  //
  // Local Data for CPU interface goes here
  //
} CPU_ARCH_PROTOCOL_PRIVATE;

VOID
CpuInitBeforeBoot (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
/**

@brief

  Perform Final Init before boot to OS

  @param[in] Event     - A pointer to the Event that triggered the callback.
  @param[in] Context   - A pointer to private data registered with the callback function.


  **/
;

VOID
ApCpuInitBeforeBoot ()
/**

@brief

  For AP Perform Final Init before boot to OS

  @param[in] Event     - A pointer to the Event that triggered the callback.
  @param[in] Context   - A pointer to private data registered with the callback function.


  **/
;

VOID
PCIConfigWA  (
  EFI_EVENT  Event,
  VOID       *Context
  )
/**

@brief

  BIOS has to inform the CPU uCODE to enable the WA for PCI config space access filtering for disabled/non existing PCI devices.
  PUNIT FW or uCODE will emulate a scratch PAD reg for this purpose and BIOS will have to set a bit in this reg.
  On seeing this bit, uCODE will enable the filtering logic. Need to enable this before OS boot loader hand-off.

  @param[in] Event     - A pointer to the Event that triggered the callback.
  @param[in] Context   - A pointer to private data registered with the callback function.


  **/
;


#define CPU_ARCH_PROTOCOL_PRIVATE_DATA_FROM_THIS(a) \
  CR (a, \
      CPU_ARCH_PROTOCOL_PRIVATE, \
      Cpu, \
      CPU_ARCH_PROT_PRIVATE_SIGNATURE \
      )

#endif
