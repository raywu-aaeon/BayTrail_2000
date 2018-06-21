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

    DisableCacheAsRam.c

Abstract:

    Disable Cache As Ram

Revision History

--*/

#include "PeiProcessor.h"

VOID
CacheInvd (
  VOID
  );


EFI_STATUS
DisableCacheAsRam (
  IN EFI_PEI_SERVICES          **PeiServices,
  IN BOOLEAN                   DisableCar
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
  EFI_STATUS                Status;
  UINT64                    CacheAsRamMsr;
  EFI_PEI_HOB_POINTERS      Hob;
  EFI_PLATFORM_CPU_INFO     *PlatformCpuInfo;

  //
  // Get Platform Info HOB
  //
  Hob.Raw = GetFirstGuidHob(&gEfiPlatformCpuInfoGuid);
  if (Hob.Raw == NULL) {
    Status = EFI_NOT_FOUND;
  } else {
    Status = EFI_SUCCESS;
    PlatformCpuInfo = GET_GUID_HOB_DATA(Hob.Guid);
  }
  ASSERT_EFI_ERROR (Status);

  CacheAsRamMsr = AsmReadMsr64 (EFI_MSR_NO_EVICT_MODE);

  //
  // Per Pineview BWG section 3.21, BIOS must set the Disable FERR Assertion
  //
//TODO: check VLV spec and see any similar requirement, however, vlv has no such MSR
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

  if (DisableCar) {
    CacheInvd ();
  } else {
    AsmWbinvd();
  }

  return EFI_SUCCESS;
}
