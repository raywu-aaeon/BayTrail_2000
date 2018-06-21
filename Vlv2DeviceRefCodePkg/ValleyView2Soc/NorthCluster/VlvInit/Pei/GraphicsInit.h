/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  GraphicsInit.h

Abstract:

  Graphics header file

--*/

#ifndef _GRAPHICS_INIT_H_
#define _GRAPHICS_INIT_H_
#ifdef ECP_FLAG
#include "EdkIIGluePeim.h"
#endif
#include "Valleyview.h"
#include "VlvAccess.h"
#include "VlvCommonDefinitions.h"
#include <Ppi/VlvPolicy.h>
#ifdef ECP_FLAG
#include "Pci22.h"
#else
#include <IndustryStandard/Pci22.h>
#endif
#define IGD_ENABLE  1
#define IGD_DISABLE 0

typedef enum {
  IGD               = 0,
  PCI,
  AUTO = 3,
  DISPLAY_DEVICE_MAX
} DISPLAY_DEVICE;

typedef enum {
  VBIOS_DEFAULT     = 0,
  CRT,
  LFP,
  CRT_LFP,
  TV,
  LFPSDVO,
  EFP,
  TVSDVO,
  CRT_LFPSDVO,
  CRT_EFP,
  IGD_BOOT_TYPE_MAX
} IGD_BOOT_TYPE;

typedef enum {
  GMS_FIXED         = 0,
  GMS_DVMT,
  GMS_FIXED_DVMT,
  GMS_MAX
} GRAPHICS_MEMORY_SELECTION;

typedef enum {
  GM_32M            = 1,
  GM_64M            = 2,
  GM_128M           = 4,
  GM_MAX
} STOLEN_MEMORY;

typedef enum {
  PAVP_DISABLED     = 0,
  PAVP_LITE,
  PAVP_HIGH
} PAVP_MODE;

#define GTTMMADR_SIZE_4MB   0x400000
#define GTT_SIZE_1MB        1
#define GTT_SIZE_2MB        2

#define APERTURE_SIZE_128MB 1
#define APERTURE_SIZE_256MB 2
#define APERTURE_SIZE_512MB 3


VOID
FindPciDeviceMmioLength (
  IN UINT32  BusNum,
  IN UINT32  DevNum,
  IN UINT32  FunNum,
  OUT UINT32 *MmioLength
  )
/*++

Routine Description:

  Find the MMIO size that a given PCI device requires

Arguments:

  BusNum     - Bus number of the device
  DevNum     - device Number of the device
  FunNum     - Function number of the device
  MmioLength - MMIO Length in bytes

Returns:

  None

--*/
;

VOID
CheckOffboardPcieVga (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN OUT UINT32               *PchPcieMmioLength,
  IN OUT DISPLAY_DEVICE       *PrimaryDisplay
  )
/*++

Routine Description:

  CheckOffboardPcieVga: Check if off board PCIe graphics Card is present

Arguments:

  PeiServices     - Pointer to the PEI services table
  PchPcieMmioLength  - Total PCIe MMIO length on all PCH root ports
  PrimaryDisplay  - Primary Display - default is IGD

Returns:

  None

--*/
;

UINT8
EnumerateDownstream (
  IN UINT8 BusNum
  )
/*++

Routine Description:

  This function enumerate all downstream bridge.

Arguments:

  BusNum  - Primary bus number of current bridge

Returns:

  Current bus number: if current bus is an enpoint device
  subordinate bus number: if current bus is a bridge

--*/
;


//
// Device 2, Function 0
//
#define McD2PciCfg64(Register)                              MmPci64           (0, MC_BUS, 2, 0, Register)
#define McD2PciCfg64Or(Register, OrData)                    MmPci64Or         (0, MC_BUS, 2, 0, Register, OrData)
#define McD2PciCfg64And(Register, AndData)                  MmPci64And        (0, MC_BUS, 2, 0, Register, AndData)
#define McD2PciCfg64AndThenOr(Register, AndData, OrData)    MmPci64AndThenOr  (0, MC_BUS, 2, 0, Register, AndData, OrData)

#define McD2PciCfg32(Register)                              MmPci32           (0, MC_BUS, 2, 0, Register)
#define McD2PciCfg32Or(Register, OrData)                    MmPci32Or         (0, MC_BUS, 2, 0, Register, OrData)
#define McD2PciCfg32And(Register, AndData)                  MmPci32And        (0, MC_BUS, 2, 0, Register, AndData)
#define McD2PciCfg32AndThenOr(Register, AndData, OrData)    MmPci32AndThenOr  (0, MC_BUS, 2, 0, Register, AndData, OrData)

#define McD2PciCfg16(Register)                              MmPci16           (0, MC_BUS, 2, 0, Register)
#define McD2PciCfg16Or(Register, OrData)                    MmPci16Or         (0, MC_BUS, 2, 0, Register, OrData)
#define McD2PciCfg16And(Register, AndData)                  MmPci16And        (0, MC_BUS, 2, 0, Register, AndData)
#define McD2PciCfg16AndThenOr(Register, AndData, OrData)    MmPci16AndThenOr  (0, MC_BUS, 2, 0, Register, AndData, OrData)

#define McD2PciCfg8(Register)                               MmPci8            (0, MC_BUS, 2, 0, Register)
#define McD2PciCfg8Or(Register, OrData)                     MmPci8Or          (0, MC_BUS, 2, 0, Register, OrData)
#define McD2PciCfg8And(Register, AndData)                   MmPci8And         (0, MC_BUS, 2, 0, Register, AndData)
#define McD2PciCfg8AndThenOr(Register, AndData, OrData)     MmPci8AndThenOr   (0, MC_BUS, 2, 0, Register, AndData, OrData)

// Device 2 Reg offsets
#define GGC             (0x50)

#define GGC_IVD_MASK    (0x2)
#define IGD_R_VID       0x00
#define GGC_GMS_OFFSET  (0x3)
#define GGC_GMS_MASK    (0xf8)
#define GGC_GGMS_OFFSET (0x8)
#define GGC_GGMS_MASK   (0x300)
#define IGD_MSAC_OFFSET 0x0062  // Multisize Aperture Contro
#define PCI_BCC         0x000B  // Base Class Code Registe
#define PCI_BAR0        0x0010  // Base Address Register 0
#define PCI_BAR5        0x0024  // Base Address Register 5
#define PCI_VID         0x0000  // Vendor ID Register


//Device 28(PCIe) Reg offsets and bit definitions
#define R_PCH_PCI_PCI_VID                 0x00
#define R_PCH_PCI_PCI_SCC                 0x0A
#define R_PCH_PCI_PCI_HEADTYP             0x0E
#define B_PCH_PCI_PCI_HEADTYP_MFD         0x80
#define R_PCH_PCI_PCI_PBN                 0x18
#define R_PCH_PCI_PCI_SCBN                0x19
#define R_PCH_PCI_PCI_SBBN                0x1A

//For SeC Engine check
//
// HECI PCI Access Macro
//
#define SEC_BUS                    0
#define SEC_DEVICE_NUMBER          26

#define HECI_FUNCTION_NUMBER       0x00
#define HECI2_FUNCTION_NUMBER      0x01

#define R_SEC_DevID_VID            0x0
#define S_SEC_DevID_MASK           0xFFFF0000
#define S_SEC_DevID_RANGE_LO       0xF18  // B.Michael need to update in BWG0.3
#define S_SEC_DevID_RANGE_HI       0xF1B  // B.Michael need to update in BWG0.3

#define HeciPciRead32(Register)    MmPci32(0, SEC_BUS, SEC_DEVICE_NUMBER, HECI_FUNCTION_NUMBER, Register)

#endif
