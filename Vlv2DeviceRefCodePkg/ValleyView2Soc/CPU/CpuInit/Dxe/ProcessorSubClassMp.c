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

    ProcessorSubClassMp.c

Abstract:

    Produces CPU data records.

Revision History

--*/

#include "CpuDxe.h"
#include "PlatformMpService.h"

extern MP_SYSTEM_DATA *mMPSystemData;

EFI_PROCESSOR_STATUS_DATA
GetProcessorStatus (
  IN UINTN                      CpuNumber
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  EFI_PROCESSOR_STATUS_DATA ProcessorStatus;
  CPU_DATA_BLOCK            *CpuData;

  CpuData                   = &mMPSystemData->CpuData[CpuNumber];

  ProcessorStatus.Reserved1 = 0;
  ProcessorStatus.Reserved2 = 0;
  ProcessorStatus.Reserved3 = 0;

  ProcessorStatus.CpuStatus = EfiCpuStatusEnabled;
  if (CpuData->State == CPU_STATE_DISABLED) {
    switch (mMPSystemData->DisableCause[CpuNumber]) {
      case EFI_CPU_CAUSE_USER_SELECTION:
      case EFI_CPU_CAUSE_BY_ASSOCIATION:
        ProcessorStatus.CpuStatus = EfiCpuStatusDisabledByUser;
        break;

      case EFI_CPU_CAUSE_INTERNAL_ERROR:
      case EFI_CPU_CAUSE_THERMAL_ERROR:
      case EFI_CPU_CAUSE_SELFTEST_FAILURE:
      case EFI_CPU_CAUSE_PREBOOT_TIMEOUT:
      case EFI_CPU_CAUSE_CONFIG_ERROR:
        ProcessorStatus.CpuStatus = EfiCpuStatusDisabledbyBios;
        break;

      case EFI_CPU_CAUSE_FAILED_TO_START:
      case EFI_CPU_CAUSE_UNSPECIFIED:
      default:
        ProcessorStatus.CpuStatus = EfiCpuStatusOther;
        break;
    }
  }

  ProcessorStatus.SocketPopulated = TRUE;
  ProcessorStatus.ApicEnable      = 1;

  if (mMPSystemData->BSP == CpuNumber) {
    ProcessorStatus.BootApplicationProcessor = 1;
  } else {
    ProcessorStatus.BootApplicationProcessor = 0;
  }

  return ProcessorStatus;
}
