/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PciHostBridge.h

Abstract:

  The Header file of the Pci Host Bridge Driver

Revision History

--*/

#ifndef _PCI_HOST_BRIDGE_H_
#define _PCI_HOST_BRIDGE_H_

//
// Disable the warning since LIST_ENTRY was defined in EDKII
// and EFI_LIST_ENTRY was defined in PCI_HOST_BRIDGE_INSTANCE
//
#ifndef __GNUC__
#pragma warning(disable: 4133)
#endif

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#define EFI_RESOURCE_SATISFIED    0x0000000000000000      // redefined in PciHostBridgeResourceAllocation.h
#endif

//
// Hard code the host bridge number in the platform.
// In this chipset, there is only one host bridge.
//
#define HOST_BRIDGE_NUMBER        1

#define PCI_HOST_BRIDGE_SIGNATURE SIGNATURE_32 ('e', 'h', 's', 't')
typedef struct {
  UINTN                                             Signature;
  EFI_HANDLE                                        HostBridgeHandle;
  UINTN                                             RootBridgeNumber;
  LIST_ENTRY                                        Head;
  BOOLEAN                                           ResourceSubmited;
  BOOLEAN                                           CanRestarted;

  EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL  ResAlloc;
} PCI_HOST_BRIDGE_INSTANCE;

#define INSTANCE_FROM_RESOURCE_ALLOCATION_THIS(a) CR (a, PCI_HOST_BRIDGE_INSTANCE, ResAlloc, PCI_HOST_BRIDGE_SIGNATURE)

/*typedef struct {
  EFI_LIST_ENTRY      Link;
  EFI_HANDLE          *RootBridgeHandle;
  UINT64              Attributes;
  VOID                *BusNumberConfiguration;
  VOID                *ResourceConfiguration;
} PCI_ROOT_BRIDGE_OPTION;
*/

//
//  HostBridge Resource Allocation interface
//
EFI_STATUS
EFIAPI
NotifyPhase (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PHASE    Phase
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description
  Phase - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
GetNextRootBridge (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN OUT EFI_HANDLE                                   *RootBridgeHandle
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  RootBridgeHandle  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
GetAttributes (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  OUT UINT64                                          *Attributes
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  RootBridgeHandle  - GC_TODO: add argument description
  Attributes        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
StartBusEnumeration (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  OUT VOID                                            **Configuration
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  RootBridgeHandle  - GC_TODO: add argument description
  Configuration     - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SetBusNumbers (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  IN VOID                                             *Configuration
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  RootBridgeHandle  - GC_TODO: add argument description
  Configuration     - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
SubmitResources (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  IN VOID                                             *Configuration
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  RootBridgeHandle  - GC_TODO: add argument description
  Configuration     - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
GetProposedResources (
  IN EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL *This,
  IN EFI_HANDLE                                       RootBridgeHandle,
  OUT VOID                                            **Configuration
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  RootBridgeHandle  - GC_TODO: add argument description
  Configuration     - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
PreprocessController (
  IN  struct _EFI_PCI_HOST_BRIDGE_RESOURCE_ALLOCATION_PROTOCOL  *This,
  IN  EFI_HANDLE                                                RootBridgeHandle,
  IN  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_PCI_ADDRESS               PciAddress,
  IN  EFI_PCI_CONTROLLER_RESOURCE_ALLOCATION_PHASE              Phase
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              - GC_TODO: add argument description
  RootBridgeHandle  - GC_TODO: add argument description
  PciAddress        - GC_TODO: add argument description
  Phase             - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

//
// Help function
//
UINT64
Power2MaxMemory (
  IN UINT64                     MemoryLength
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MemoryLength  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

#endif
