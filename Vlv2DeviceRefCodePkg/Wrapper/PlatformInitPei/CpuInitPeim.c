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

#include "PlatformEarlyInit.h"

VOID
ProgramNbStopGrant(
    IN      EFI_PLATFORM_CPU_INFO          *PlatformCpuInfo
)
{
//    UINT32    NumProc;

    //
    // ADC selftest tool required stop grant to fire for 1 clock cycle only
    //
    AsmWriteMsr64(EFI_MSR_PMG_CST_CONFIG, (AsmReadMsr64(EFI_MSR_PMG_CST_CONFIG) & ~B_EFI_MSR_PMG_CST_CONFIG_STPGNT_ISSUE));

    //
    // For Pineview dual core there will only be 1 (default) or 2 (programmable) STPGNT cycles, eventhough there are 4 threads.
    //
//    NumProc = 1;
    if(PlatformCpuInfo->CpuPackage.LogicalProcessorsPerPhysicalPackage > 1) {
//        NumProc = 4;
    }

    //
    // TBD: Need re-design based on the ValleyTrail platform.
    //

}

VOID
DisableHyperthreading(
    IN      CONST EFI_PEI_SERVICES               **PeiServices,
    IN      SETUP_DATA           *SystemConfiguration,
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

EFI_STATUS
PlatformCpuInit(
    IN CONST EFI_PEI_SERVICES            **PeiServices,
    IN SETUP_DATA        *SystemConfiguration,
    IN EFI_PLATFORM_CPU_INFO       *PlatformCpuInfo
)
{
    BOOLEAN                     ResetRequired;

    //
    // Variable initialization
    //
    ResetRequired = FALSE;

    //
    // Program Stop Grant
    //Not supported on SLM CPU.
//    ProgramNbStopGrant(PlatformCpuInfo);    //(CSP20130313F-)

    //
    // Hyperthreading Technology
    // NOTE: This function is not used and replaced with software thread hiding as the
    //       MC_MMIO_POC register is not a POR register. If BIOS insist to use this function
    //       it will caused MRC failure as there is a limitation in MRC code that the coding execution
    //       path is choosen depending on a system reset bit (in MCH). Any reset before MRC
    //       has to be prevented.
    //
//  DisableHyperthreading( PeiServices, SystemConfiguration, PlatformCpuInfo, &ResetRequired );

    if(ResetRequired) {
        CpuOnlyReset(PeiServices);
    }

    return EFI_SUCCESS;
}
