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
  
    CpuInitPeim.c
   
Abstract:

    Functions for LpcSio initilization
    It is needed for early onboard LAN controller disable/enable in platform setup.
    
--*/

//#include "PlatformEarlyInit.h"
//#include <PeiProcessor.h>
#include <Ppi/ReadOnlyVariable2.h>
#include "CpuRegs.h"
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/CpuPolicy.h>
#include <Library/MemoryAllocationLib.h>

typedef struct {
  UINT32  RegEax;
  UINT32  RegEbx;
  UINT32  RegEcx;
  UINT32  RegEdx;
} EFI_CPUID_REGISTER;

#include <CpuType.h>
#include <Guid/PlatformCpuInfo.h>
#include <Setup.h>

VOID
ProgramNbStopGrant(
  IN      EFI_PLATFORM_CPU_INFO          *PlatformCpuInfo
  )
{
  UINT32    NumProc;

  //
  // ADC selftest tool required stop grant to fire for 1 clock cycle only
  //
  AsmWriteMsr64(EFI_MSR_PMG_CST_CONFIG, (AsmReadMsr64(EFI_MSR_PMG_CST_CONFIG) & ~B_EFI_MSR_PMG_CST_CONFIG_STPGNT_ISSUE));

  //
  // For Pineview dual core there will only be 1 (default) or 2 (programmable) STPGNT cycles, eventhough there are 4 threads.
  //
  NumProc = 1;
  if (PlatformCpuInfo->CpuPackage.LogicalProcessorsPerPhysicalPackage > 1) {
    NumProc = 4;
  }

  //
  // TBD: Need re-design based on the ValleyTrail platform.
  //

}
#if 0  //the hardware don't support this feature, then remove this function call
VOID
DisableHyperthreading (
  IN      CONST EFI_PEI_SERVICES               **PeiServices,
  IN      CPU_SETUP_DATA           *SystemConfiguration,
  IN      EFI_PLATFORM_CPU_INFO          *PlatformCpuInfo,
  IN OUT  BOOLEAN                        *CpuResetRequired
  )
{
  /*DDQ 2010-10-14 14:57:41 -m: Notice
  //
  // If HT enabled in BIOS setup and HT really is enable, nothing need to do here.
  // Or if HT is disabled in BIOS setup and HT really is disabled, nothing need to do here.
  //
  if ( ((SystemConfiguration->ProcessorHtMode != 0) &&
        ((McMmio32(MC_MMIO_POC) & B_POC_DISABLE_HYPERTHREADING) == 0)) ||
       ((SystemConfiguration->ProcessorHtMode == 0) &&
        ((McMmio32(MC_MMIO_POC) & B_POC_DISABLE_HYPERTHREADING) != 0) &&
        (PlatformCpuInfo->CpuPackage.ThreadsPerCore == 1))
    ) {
    return;
  }

  //
  // Set/Clear Disable Hyperthreading bit
  //
  if (SystemConfiguration->ProcessorHtMode != 0) {
    McMmio32And( MC_MMIO_POC, ~B_POC_DISABLE_HYPERTHREADING );
  } else {
    McMmio32Or( MC_MMIO_POC, B_POC_DISABLE_HYPERTHREADING );
  }

  //
  // Request CPU reset
  //
  *CpuResetRequired = TRUE;
  DDQ 2010-10-14 14:57:50 -e*/
  //DDQ 2010 -a
  //
  // TBD: Need re-design based on the ValleyTrail platform.
  //
  //DDQ 2010 -e
}
#endif

EFI_STATUS
EFIAPI
CpuOnlyReset (
  IN CONST EFI_PEI_SERVICES   **PeiServices
  )
{
//  MsgBus32Write(CDV_UNIT_PUNIT, PUNIT_CPU_RST, 0x01)

  _asm {
    xor   ecx, ecx
  HltLoop:
    hlt
    hlt
    hlt
    loop  HltLoop
  }

  //
  // If we get here we need to mark it as a failure.
  //
  return EFI_UNSUPPORTED;
}

EFI_STATUS
PlatformCpuInit (
  IN CONST EFI_PEI_SERVICES            **PeiServices,
  IN EFI_PLATFORM_CPU_INFO       *PlatformCpuInfo
  )
{
  BOOLEAN                     ResetRequired;
  //CPU_SETUP_DATA                *VlvPolicyData;

  /*
  VlvPolicyData = (CPU_SETUP_DATA *) AllocateZeroPool (sizeof (CPU_SETUP_DATA));
  ASSERT (VlvPolicyData != NULL);
  if (VlvPolicyData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  */

  // Get the value of the CPU Setup data.
  // GetCpuSetupData (  (VOID *)PeiServices, VlvPolicyData, TRUE);

  //
  // Variable initialization
  //
  ResetRequired = FALSE;

  //
  // Program Stop Grant
  //
  ProgramNbStopGrant(PlatformCpuInfo);

  //
  // Hyperthreading Technology
  // NOTE: This function is not used and replaced with software thread hiding as the
  //       MC_MMIO_POC register is not a POR register. If BIOS insist to use this function
  //       it will caused MRC failure as there is a limitation in MRC code that the coding execution
  //       path is choosen depending on a system reset bit (in MCH). Any reset before MRC
  //       has to be prevented.
  //
//  DisableHyperthreading( PeiServices, SystemConfiguration, PlatformCpuInfo, &ResetRequired );

  if (ResetRequired) {
    CpuOnlyReset(PeiServices);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CpuPeiInitEntry (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
/*++

Routine Description:

  Platform specific initializations in stage1.

Arguments:

  FfsHeader         Pointer to the PEIM FFS file header.
  PeiServices       General purpose services available to every PEIM.

Returns:

  EFI_SUCCESS       Operation completed successfully.
  Otherwise         Platform initialization failed.
--*/
{
  EFI_STATUS                  Status;
  EFI_PLATFORM_CPU_INFO       PlatformCpuInfo;

  //
  // Variable initialization
  //
  ZeroMem(&PlatformCpuInfo, sizeof(EFI_PLATFORM_CPU_INFO));

  //
  // Do basic CPU init
  //
  Status = PlatformCpuInit (PeiServices, &PlatformCpuInfo);

  return Status;
}
