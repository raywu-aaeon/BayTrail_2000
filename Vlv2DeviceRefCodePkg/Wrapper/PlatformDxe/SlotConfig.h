/*++

Copyright (c)  1999 - 2005 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  SlotConfig.c

Abstract:

  Sets platform/SKU specific expansion slot information.


 

--*/

#include "PlatformDxe.h"
#include <Protocol/SmbiosSlotPopulation.h>
#include <IndustryStandard/Pci22.h>


//
// Default bus number for the bridge
//
#define DEF_BUS_CONFIG  0x0101
#define DEF_BUS         0x01

//
// Data structures for slot information
//
typedef struct {
  UINT16  SmbiosSlotId;
  UINT8   Bus;
  UINT8   Dev;
  UINT8   Function;
  UINT8   TargetDevice;
} EFI_PCI_SLOT_BRIDGE_INFO;

//
// Product specific bridge to slot routing information
//
EFI_PCI_SLOT_BRIDGE_INFO mSlotBridgeTable[] = {
  {
    0x01,             //PCIe x1 ICH (Bridge B0:D28:F1)
    DEFAULT_PCI_BUS_NUMBER_PCH,
    PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
    PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_2,
    0
  }
};

UINTN mSlotBridgeTableSize =
  sizeof(mSlotBridgeTable) / sizeof(EFI_PCI_SLOT_BRIDGE_INFO);

//
// Slot entry table for IBX RVP
//
EFI_SMBIOS_SLOT_ENTRY mSlotEntries[] = {
  {0x06, FALSE, TRUE},    // PCIe x16 Slot 1 (NOT USED)
  {0x04, FALSE, TRUE},    // PCIe x16 Slot 2 (NOT USED)
  {0x03, FALSE, TRUE},    // PCIe x4 Slot (NOT USED)
  {0x02, FALSE, FALSE},   // Mini PCIe x1 Slot
  {0x15, FALSE, TRUE},    // PCIe x1 Slot 2 (NOT USED)
  {0x16, FALSE, TRUE},    // PCIe x1 Slot 3 (NOT USED)
  {0x07, FALSE, FALSE},   // PCI Slot 1
  {0x18, FALSE, TRUE},    // PCI Slot 2 (NOT USED)
  {0x17, FALSE, TRUE},    // PCI Slot 3 (NOT USED)
};

EFI_SMBIOS_SLOT_POPULATION_INFO mSlotInformation = {
  sizeof(mSlotEntries) / sizeof(EFI_SMBIOS_SLOT_ENTRY),
  mSlotEntries
};


