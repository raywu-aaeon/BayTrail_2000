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

  PciRootBridge.h

Abstract:

  The driver for the host to pci bridge (root bridge).

Revision History

--*/

#ifndef _PCI_ROOT_BRIDGE_H_
#define _PCI_ROOT_BRIDGE_H_

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include "ACPI.h"
#else
#include <PiDxe.h>
#include <IndustryStandard/Acpi.h>
#include <Library/S3BootScriptLib.h>
#endif
#include "VlvAccess.h"
#ifndef ECP_FLAG
#include <Protocol/PciHostBridgeResourceAllocation.h>
#include <Protocol/Metronome.h>
#include <Protocol/CpuIo.h>

#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#endif

//
// Define resource status constant
//
#define EFI_RESOURCE_NONEXISTENT  0xFFFFFFFFFFFFFFFF
#define EFI_RESOURCE_LESS         0xFFFFFFFFFFFFFFFE


//
// Driver Instance Data Prototypes
//
typedef struct {
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_OPERATION Operation;
  UINTN                                     NumberOfBytes;
  UINTN                                     NumberOfPages;
  EFI_PHYSICAL_ADDRESS                      HostAddress;
  EFI_PHYSICAL_ADDRESS                      MappedHostAddress;
} MAP_INFO;

typedef struct {
  ACPI_HID_DEVICE_PATH      AcpiDevicePath;
  EFI_DEVICE_PATH_PROTOCOL  EndDevicePath;
} EFI_PCI_ROOT_BRIDGE_DEVICE_PATH;

typedef struct {
  UINT64  BusBase;
  UINT64  BusLimit;

  UINT64  MemBase;
  UINT64  MemLimit;

  UINT64  IoBase;
  UINT64  IoLimit;
} PCI_ROOT_BRIDGE_RESOURCE_APPETURE;

typedef enum {
  TypeIo    = 0,
  TypeMem32,
  TypePMem32,
  TypeMem64,
  TypePMem64,
  TypeBus,
  TypeMax
} PCI_RESOURCE_TYPE;

typedef enum {
  ResNone     = 0,
  ResSubmitted,
  ResRequested,
  ResAllocated,
  ResStatusMax
} RES_STATUS;

typedef struct {
  PCI_RESOURCE_TYPE Type;
  UINT64            Base;
  UINT64            Length;
  UINT64            Alignment;
  RES_STATUS        Status;
} PCI_RES_NODE;

#define PCI_ROOT_BRIDGE_SIGNATURE SIGNATURE_32 ('e', '2', 'p', 'b')

typedef struct {
  UINT32                          Signature;
  LIST_ENTRY                      Link;
  EFI_HANDLE                      Handle;
  UINT64                          RootBridgeAttrib;
  UINT64                          Attributes;
  UINT64                          Supports;

  //
  // Specific for this memory controller: Bus, I/O, Mem
  //
  PCI_RES_NODE                    ResAllocNode[6];

  //
  // Addressing for Memory and I/O and Bus arrange
  //
  UINT64                          BusBase;
  UINT64                          MemBase;
  UINT64                          IoBase;
  UINT64                          BusLimit;
  UINT64                          MemLimit;
  UINT64                          IoLimit;

  EFI_LOCK                        PciLock;
  UINTN                           PciAddress;
  UINTN                           PciData;

  EFI_DEVICE_PATH_PROTOCOL        *DevicePath;
  EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL Io;

} PCI_ROOT_BRIDGE_INSTANCE;

//
// Driver Instance Data Macros
//
// #define DRIVER_INSTANCE_FROM_DEVICE_PATH_THIS(a) \
//  CR(a, PCI_ROOT_BRIDGE_INSTANCE, DevicePath, PCI_ROOT_BRIDGE_SIGNATURE)
//
#define DRIVER_INSTANCE_FROM_PCI_ROOT_BRIDGE_IO_THIS(a) CR (a, PCI_ROOT_BRIDGE_INSTANCE, Io, PCI_ROOT_BRIDGE_SIGNATURE)

#define DRIVER_INSTANCE_FROM_LIST_ENTRY(a)              CR (a, PCI_ROOT_BRIDGE_INSTANCE, Link, PCI_ROOT_BRIDGE_SIGNATURE)

EFI_STATUS
RootBridgeConstructor (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL    *Protocol,
  IN EFI_HANDLE                         HostBridgeHandle,
  IN UINT64                             Attri,
  IN PCI_ROOT_BRIDGE_RESOURCE_APPETURE  *ResAppeture
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Protocol          - GC_TODO: add argument description
  HostBridgeHandle  - GC_TODO: add argument description
  Attri             - GC_TODO: add argument description
  ResAppeture       - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;
#endif
