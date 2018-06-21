/*++

Copyright (c)  1999 - 2005 Intel Corporation. All rights reserved

This source code and any documentation accompanying it ("Material") is furnished
under license and may only be used or copied in accordance with the terms of that
license.  No license, express or implied, by estoppel or otherwise, to any
intellectual property rights is granted to you by disclosure or delivery of these
Materials.  The Materials are subject to change without notice and should not be
construed as a commitment by Intel Corporation to market, license, sell or support
any product or technology.  Unless otherwise provided for in the license under which
this Material is provided, the Material is provided AS IS, with no warranties of
any kind, express or implied, including without limitation the implied warranties
of fitness, merchantability, or non-infringement.  Except as expressly permitted by
the license for the Material, neither Intel Corporation nor its suppliers assumes
any responsibility for any errors or inaccuracies that may appear herein.  Except
as expressly permitted by the license for the Material, no part of the Material
may be reproduced, stored in a retrieval system, transmitted in any form, or
distributed by any means without the express written consent of Intel Corporation.
--*/

/** @file PlatformCpuPolicyHooksLib.h    
    Header file for C objects within the files of the Personality driver.
    This file is for driver files use only, and should not be included by code outside the driver.    
**/

#ifndef _PLATFORM_CPU_POLICY_HOOKS_LIB_H_
#define _PLATFORM_CPU_POLICY_HOOKS_LIB_H_

#include <PiDxe.h>

#include <Protocol/PlatformCpu.h>

VOID
HooksInit (
  IN  EFI_PLATFORM_CPU_PROTOCOL              *This
  )
/*++

  Routine Description:
    Hooks may need to initialize before protocol members could be called.
    This is the routine that is instantiated by the Hooks code.

Arguments:
  This            - A pointer to protocol instance.

--*/

// GC_TODO: function comment is missing 'Returns:'
// GC_TODO:    This - add argument and description to function comment
;

EFI_STATUS
PlatformCpuGetCpuInfo (
  IN   EFI_PLATFORM_CPU_PROTOCOL              *This,
  IN   EFI_CPU_PHYSICAL_LOCATION              *Location,
  OUT  EFI_PLATFORM_CPU_INFORMATION           *PlatformCpuInfo
  )
/*++

Routine Description:
  This is member function EFI_PLATFORM_CPU_PROTOCOL.GetCpuInfo().

Arguments:
  This            - A pointer to protocol instance.
  Location        - Location data used by this function to know which processor it is about.
  PlatformCpuInfo  - Data returned to the caller containing info on the processor like APIC ID, frequencies, strings etc.
                    This function is directly called as the GetCpuInfo() member of EFI_PLATFORM_CPU_PROTOCOL.

Returns:
  EFI_SUCCESS         Always.
  ASSERT () in case of errors.

--*/

// GC_TODO:    PlatformCpuInfo - add argument and description to function comment
//
// GC_TODO:    PlatformCpuInfo - add argument and description to function comment
//
// GC_TODO:    PlatformCpuInfo - add argument and description to function comment
//
;

typedef struct {
  UINTN               NoOfProcessorsAvailable;
  UINT32              MinStepping;
  EFI_EXP_BASE10_DATA MinCoreFreq;
  EFI_EXP_BASE10_DATA MinBusFreq;
  EFI_EXP_BASE2_DATA  MinCacheSize[EFI_CACHE_LMAX];
} PROCESSOR_SET_PERFORMANCE;

UINTN
MpComparePerformance (
  IN  PROCESSOR_SET_PERFORMANCE      *MpConfig1,
  IN  PROCESSOR_SET_PERFORMANCE      *MpConfig2
  )
/*++

Routine Description:
  This routine compares performance of two systems given two sets of 
  enabled processors.
  It is platform specific because technology changes from platform 
  to platform and criteria determining better performance will change.

Arguments:
  MpConfig1,
  MpConfig2 - Two performances to compare. 

Returns:
  1 - MpConfig1 is better than, or identical to MpConfig2.
  2 - MpConfig2 is better than MpConfig1.

--*/
;

CHAR16  *
StrHzToString (
  OUT CHAR16          *String,
  IN  UINT64          Val
  )
/*++

Routine Description:
  Converts frequency in Hz to Unicode string. 
  Three significant digits are delivered. Used for processor info display.

Arguments:
  String - string that will contain the frequency.
  Val    - value to convert, minimum is  100000 i.e., 0.1 MHz.

--*/

// GC_TODO: function comment is missing 'Returns:'
;

#endif
