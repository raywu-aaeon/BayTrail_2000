/*++

Copyright (c)  1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PlatformCpuPolicy.h

Abstract:

  Header file for common server processor policy driver.

--*/

#ifndef _PLATFORM_CPU_POLICY_H_
#define _PLATFORM_CPU_POLICY_H_

#include <Protocol/PlatformCpu.h>

//
// Define macros to build data structure signatures from characters.
//
#ifndef EFI_SIGNATURE_16
#define EFI_SIGNATURE_16(A, B)        ((A) | (B << 8))
#endif
#ifndef EFI_SIGNATURE_32
#define EFI_SIGNATURE_32(A, B, C, D)  (EFI_SIGNATURE_16 (A, B) | (EFI_SIGNATURE_16 (C, D) << 16))
#endif

//
// Driver data signature
//
#define PLATFORM_CPU_POLICY_SIGNATURE EFI_SIGNATURE_32 ('P', 'C', 'P', 'D')

//
// Private data structure
//
typedef struct {
    UINT32                    Signature;
    EFI_HANDLE                Handle;
    EFI_PLATFORM_CPU_PROTOCOL PlatformCpu;
    BOOLEAN                   OneToOneIdMappingScheme;
    BOOLEAN                   BmcFunctional;
} PLATFORM_CPU_POLICY_INSTANCE;

//
// Location table
//
typedef struct {
    UINT32  ApicId;
    UINT8   PackageIdx;
    UINT8   DieIdx;
    UINT8   CoreIdx;
    UINT8   ThreadIdx;
} PLATFORM_CPU_LOCATION_TABLE;

//
// Containment record macro
//
#define PLATFORM_CPU_POLICY_INSTANCE_FROM_EFI_PLATFORM_CPU_PROTOCOL_THIS(a) \
  CR ( \
  a, \
  PLATFORM_CPU_POLICY_INSTANCE, \
  PlatformCpu, \
  PLATFORM_CPU_POLICY_SIGNATURE \
  )

//
// Prototypes
//
EFI_STATUS
PlatformCpuPolicyOverridePolicy(
    IN  EFI_PLATFORM_CPU_PROTOCOL              *This,
    IN  UINT32                                 CpuCount,
    IN  EFI_DETAILED_CPU_INFO                  *CpuInfoList,
    IN  OUT EFI_CPU_STATE_CHANGE_CAUSE         *CpuDisableList,
    IN  OUT UINT32                             *Bsp
);

EFI_STATUS
EFIAPI
PlatformCpuPolicyCheckCpuState(
    IN  EFI_PLATFORM_CPU_PROTOCOL             *This
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

//
// Driver entry point
//
EFI_STATUS
PlatformCpuPolicyEntryPoint(
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
);

EFI_STATUS
NotifyCpuPolicyCallback(
    EFI_EVENT Event,
    VOID      *Context
);

VOID
HooksInit(
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
PlatformCpuGetCpuInfo(
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
;

EFI_STATUS
EFIAPI
PlatformCpuGetDCAState(
    IN OUT UINT16            *DcaEnable,
    IN OUT UINT16            *DcaPrefetchDelayValue
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  DcaEnable             - GC_TODO: add argument description
  DcaPrefetchDelayValue - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

#endif
