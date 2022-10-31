/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    Processor.h

Abstract:

    Header file for CPU Data File



--*/

#ifndef _PROCESSOR_H_
#define _PROCESSOR_H_

#include "CpuDxe.h"

//
// Platform-specific definitions
//
#define EFI_CPU_DATA_MAXIMUM_LENGTH 0x100

EFI_STATUS
InitializeProcessorData (
  IN  UINTN                         CpuNumber,
  IN  CPU_DATA_FOR_DATAHUB          *CpuDataForDatahub
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

INT16
GetProcessorVoltage (
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

UINT32
GetCpuUcodeRevision (
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

EFI_PROCESSOR_STATUS_DATA
GetProcessorStatus (
  UINTN                      CpuNumber
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

VOID
GetProcessorVersion (
  OUT PROCESSOR_VERSION_INFORMATION        *Version
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Version - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_PROCESSOR_FAMILY_DATA
GetProcessorFamily (
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

EFI_PROCESSOR_MANUFACTURER_DATA
GetProcessorManufacturer (
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
IsIntelProcessor (
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
GetProcessorId (
  OUT  EFI_PROCESSOR_ID_DATA    *ProcessorId
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ProcessorId - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
GetProcessorSerialNumber (
  IN    EFI_PROCESSOR_SERIAL_NUMBER_DATA  *PSN
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PSN - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
GetFrequency (
  OUT UINT64                        *Frequency
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Frequency - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_PROCESSOR_PACKAGE_NUMBER_DATA
GetPackageNumber (
  IN  UINTN Instance
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Instance  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_PROCESSOR_HEALTH_STATUS
GetHealthStatus (
  IN  UINTN Instance
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Instance  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_PROCESSOR_CORE_COUNT_DATA
GetProcessorCoreCount (
  VOID
  )
/*++

Routine Description:

  Returns the processor core count of the processor installed in the system.

Arguments:

  None

Returns:

  Processor Core Count

--*/
;

EFI_PROCESSOR_ENABLED_CORE_COUNT_DATA
GetProcessorEnabledCoreCount (
  VOID
  )
/*++

Routine Description:

  Returns the processor enabled core count of the processor installed in the system.

Arguments:

  None

Returns:

  Processor Enabled Core Count

--*/
;

EFI_PROCESSOR_THREAD_COUNT_DATA
GetProcessorThreadCount (
  VOID
  )
/*++

Routine Description:

  Returns the processor thread count of the processor installed in the system.

Arguments:

  None

Returns:

  Processor Thread Count

--*/
;

EFI_PROCESSOR_CHARACTERISTICS_DATA
GetProcessorCharacteristics (
  VOID
  )
/*++

Routine Description:

  Returns the processor Characteristics of the processor installed in the system.

Arguments:

  None

Returns:

  Processor Characteristics

--*/
;

#endif
