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

  PlatformMemoryRange.h

Abstract:

  Platform Memory Range PPI as defined in EFI 2.0

  PPI for reserving special purpose memory ranges.

--*/
//
// This file contains 'Framework Code' and is licensed as such
// under the terms of your license agreement with Intel or your
// vendor.  This file may not be modified, except as allowed by
// additional terms of your license agreement.
//
#ifndef _PEI_PLATFORM_MEMORY_RANGE_H_
#define _PEI_PLATFORM_MEMORY_RANGE_H_

#define PEI_PLATFORM_MEMORY_RANGE_PPI_GUID \
  { \
    0x30eb2979, 0xb0f7, 0x4d60, 0xb2, 0xdc, 0x1a, 0x2c, 0x96, 0xce, 0xb1, 0xf4 \
  }

typedef struct _PEI_PLATFORM_MEMORY_RANGE_PPI  PEI_PLATFORM_MEMORY_RANGE_PPI ;

#define PEI_MEMORY_RANGE_OPTION_ROM UINT32

#define PEI_MR_OPTION_ROM_ALL       0xFFFFFFFF
#define PEI_MR_OPTION_ROM_NONE      0x00000000
#define PEI_MR_OPTION_ROM_C0000_16K 0x00000001
#define PEI_MR_OPTION_ROM_C4000_16K 0x00000002
#define PEI_MR_OPTION_ROM_C8000_16K 0x00000004
#define PEI_MR_OPTION_ROM_CC000_16K 0x00000008
#define PEI_MR_OPTION_ROM_D0000_16K 0x00000010
#define PEI_MR_OPTION_ROM_D4000_16K 0x00000020
#define PEI_MR_OPTION_ROM_D8000_16K 0x00000040
#define PEI_MR_OPTION_ROM_DC000_16K 0x00000080
#define PEI_MR_OPTION_ROM_E0000_16K 0x00000100
#define PEI_MR_OPTION_ROM_E4000_16K 0x00000200
#define PEI_MR_OPTION_ROM_E8000_16K 0x00000400
#define PEI_MR_OPTION_ROM_EC000_16K 0x00000800
#define PEI_MR_OPTION_ROM_F0000_16K 0x00001000
#define PEI_MR_OPTION_ROM_F4000_16K 0x00002000
#define PEI_MR_OPTION_ROM_F8000_16K 0x00004000
#define PEI_MR_OPTION_ROM_FC000_16K 0x00008000

//
// SMRAM Memory Range
//
#define PEI_MEMORY_RANGE_SMRAM      UINT32
#define PEI_MR_SMRAM_ALL            0xFFFFFFFF
#define PEI_MR_SMRAM_NONE           0x00000000
#define PEI_MR_SMRAM_CACHEABLE_MASK 0x80000000
#define PEI_MR_SMRAM_SEGTYPE_MASK   0x00FF0000
#define PEI_MR_SMRAM_ABSEG_MASK     0x00010000
#define PEI_MR_SMRAM_HSEG_MASK      0x00020000
#define PEI_MR_SMRAM_TSEG_MASK      0x00040000
//
// If adding additional entries, SMRAM Size
// is a multiple of 128KB.
//
#define PEI_MR_SMRAM_SIZE_MASK          0x0000FFFF
#define PEI_MR_SMRAM_SIZE_128K_MASK     0x00000001
#define PEI_MR_SMRAM_SIZE_256K_MASK     0x00000002
#define PEI_MR_SMRAM_SIZE_512K_MASK     0x00000004
#define PEI_MR_SMRAM_SIZE_1024K_MASK    0x00000008
#define PEI_MR_SMRAM_SIZE_2048K_MASK    0x00000010
#define PEI_MR_SMRAM_SIZE_4096K_MASK    0x00000020
#define PEI_MR_SMRAM_SIZE_8192K_MASK    0x00000040

#define PEI_MR_SMRAM_ABSEG_128K_NOCACHE 0x00010001
#define PEI_MR_SMRAM_HSEG_128K_CACHE    0x80020001
#define PEI_MR_SMRAM_HSEG_128K_NOCACHE  0x00020001
#define PEI_MR_SMRAM_TSEG_128K_CACHE    0x80040001
#define PEI_MR_SMRAM_TSEG_128K_NOCACHE  0x00040001
#define PEI_MR_SMRAM_TSEG_256K_CACHE    0x80040002
#define PEI_MR_SMRAM_TSEG_256K_NOCACHE  0x00040002
#define PEI_MR_SMRAM_TSEG_512K_CACHE    0x80040004
#define PEI_MR_SMRAM_TSEG_512K_NOCACHE  0x00040004
#define PEI_MR_SMRAM_TSEG_1024K_CACHE   0x80040008
#define PEI_MR_SMRAM_TSEG_1024K_NOCACHE 0x00040008

//
// Graphics Memory Range
//
#define PEI_MEMORY_RANGE_GRAPHICS_MEMORY  UINT32
#define PEI_MR_GRAPHICS_MEMORY_ALL        0xFFFFFFFF
#define PEI_MR_GRAPHICS_MEMORY_NONE       0x00000000
#define PEI_MR_GRAPHICS_MEMORY_CACHEABLE  0x80000000
//
// If adding additional entries, Graphics Memory Size
// is a multiple of 512KB.
//
#define PEI_MR_GRAPHICS_MEMORY_SIZE_MASK    0x0000FFFF
#define PEI_MR_GRAPHICS_MEMORY_512K_NOCACHE 0x00000001
#define PEI_MR_GRAPHICS_MEMORY_512K_CACHE   0x80000001
#define PEI_MR_GRAPHICS_MEMORY_1M_NOCACHE   0x00000002
#define PEI_MR_GRAPHICS_MEMORY_1M_CACHE     0x80000002
#define PEI_MR_GRAPHICS_MEMORY_4M_NOCACHE   0x00000008
#define PEI_MR_GRAPHICS_MEMORY_4M_CACHE     0x80000008
#define PEI_MR_GRAPHICS_MEMORY_8M_NOCACHE   0x00000010
#define PEI_MR_GRAPHICS_MEMORY_8M_CACHE     0x80000010
#define PEI_MR_GRAPHICS_MEMORY_16M_NOCACHE  0x00000020
#define PEI_MR_GRAPHICS_MEMORY_16M_CACHE    0x80000020
#define PEI_MR_GRAPHICS_MEMORY_32M_NOCACHE  0x00000040
#define PEI_MR_GRAPHICS_MEMORY_32M_CACHE    0x80000040
#define PEI_MR_GRAPHICS_MEMORY_48M_NOCACHE  0x00000060
#define PEI_MR_GRAPHICS_MEMORY_48M_CACHE    0x80000060
#define PEI_MR_GRAPHICS_MEMORY_64M_NOCACHE  0x00000080
#define PEI_MR_GRAPHICS_MEMORY_64M_CACHE    0x80000080
#define PEI_MR_GRAPHICS_MEMORY_128M_NOCACHE 0x00000100
#define PEI_MR_GRAPHICS_MEMORY_128M_CACHE   0x80000100
#define PEI_MR_GRAPHICS_MEMORY_256M_NOCACHE 0x00000200
#define PEI_MR_GRAPHICS_MEMORY_256M_CACHE   0x80000200
//
// Pci Memory Hole
//
#define PEI_MEMORY_RANGE_PCI_MEMORY       UINT32
#define PEI_MR_PCI_MEMORY_SIZE_512M_MASK  0x00000001

typedef
EFI_STATUS
(EFIAPI *PEI_CHOOSE_RANGES) (
  IN      EFI_PEI_SERVICES                      **PeiServices,
  IN PEI_PLATFORM_MEMORY_RANGE_PPI              * This,
  IN OUT  PEI_MEMORY_RANGE_OPTION_ROM           * OptionRomMask,
  IN OUT  PEI_MEMORY_RANGE_SMRAM                * SmramMask,
  IN OUT  PEI_MEMORY_RANGE_GRAPHICS_MEMORY      * GraphicsMemoryMask,
  IN OUT  PEI_MEMORY_RANGE_PCI_MEMORY           * PciMemoryMask
  );

struct _PEI_PLATFORM_MEMORY_RANGE_PPI {
  PEI_CHOOSE_RANGES ChooseRanges;
};

extern EFI_GUID gPeiPlatformMemoryRangePpiGuid;

#endif
