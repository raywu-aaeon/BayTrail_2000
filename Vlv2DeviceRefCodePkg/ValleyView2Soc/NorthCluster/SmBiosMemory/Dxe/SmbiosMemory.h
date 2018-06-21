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

  SmbiosMemory.h

Abstract:

  Header file for the MemorySubClass Driver.
  This driver will determine memory configuration information from the chipset
  and memory and create SMBIOS memory structures appropriately.

--*/

#ifndef _SMBIOS_MEMORY_H_
#define _SMBIOS_MEMORY_H_

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include "Protocol/HiiDatabase/HiiDatabase.h"
#include "Protocol/HiiString/HiiString.h"
#include "UefiIfrLibrary.h"
#define EFI_MEMORY_ARRAY_LINK_DATA EFI_MEMORY_ARRAY_LINK
#else
#include "PiDxe.h"
#endif
#include "VlvAccess.h"
//
// Maximum number of SDRAM channels supported by the memory controller
//
#define MAX_CHANNELS         2
//
// Maximum number of DIMM sockets supported by each channel
//
#define MAX_SLOTS            1
//
// Maximum number of sides supported per DIMM
//
#define MAX_SIDES            2
//
// Maximum number of DIMM sockets supported by the memory controller
//
#define MAX_SOCKETS          (MAX_CHANNELS * MAX_SLOTS)
//
// Maximum number of rows supported by the memory controller
//
#define MAX_RANKS            (MAX_SOCKETS * MAX_SIDES)
//
// This is the generated header file which includes whatever needs to be exported (strings + IFR)
//
#include "SmbiosMemoryStrDefs.h"

//
// Driver Consumed Protocol Prototypes
//
#include "Protocol/MemInfo.h"
// Driver private data
//
#include "Guid/MemoryConfigData.h"


#define EFI_MEMORY_SUBCLASS_DRIVER_GUID \
  { 0x1767CEED, 0xDB82, 0x47cd, 0xBF, 0x2B, 0x68, 0x45, 0x8A, 0x8C, 0xCF, 0xFF }

//
// SmBus address to read DIMM SPD
//
#define DIMM0_SPD_ADDRESS 0xA0

//
// Memory
//
#define MEM_FRQCY_BIT_SHIFT 1

#define FREQ_800           0x00
#define FREQ_1066          0x01
#define FREQ_1333          0x02
#define FREQ_1600          0x03

enum {
  DDRType_DDR3 = 0,
  DDRType_DDR3L = 1,
  DDRType_DDR3U = 2,
  DDRType_DDR3All = 3,
  DDRType_LPDDR2 = 4,
  DDRType_LPDDR3 = 5,
  DDRType_DDR4 = 6
};

// Maximum memory supported by the memory controller
// 4 GB in terms of KB
//
#define MAX_MEM_CAPACITY (4 * 1024 * 1024)

//
// Memory Module Manufacture ID List Structure
//
typedef struct {
  UINT8 Index;
  UINT8 ManufactureId;
  CHAR16* ManufactureName;
} MEMORY_MODULE_MANUFACTURE_LIST;

//
// Row configuration data structure
//
typedef struct {
  EFI_PHYSICAL_ADDRESS          BaseAddress;
  UINT64                        RowLength;  // Size of Row in bytes
} DDR_ROW_CONFIG;

//
// Prototypes
//
EFI_STATUS
SmbiosMemoryEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

typedef UINT16  STRING_REF;
#ifndef ECP_FLAG
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include "DataHubRecords.h"
#include "Protocol/DataHub.h"
#include "Protocol/SmbusHc.h"
#include "Protocol/FrameworkHii.h"
#include "Protocol/HiiDatabase.h"
#include "Protocol/HiiString.h"
#include "Library/HiiLib.h"
#endif
#endif
