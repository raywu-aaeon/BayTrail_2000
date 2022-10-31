/*++
Copyright (c) 1996 - 2013, Intel Corporation.

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


Module Name:


Abstract:


--*/

#ifndef _PIDV_H
#define _PIDV_H

//
// Statements that include other files
//
#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include <IndustryStandard/Acpi30.h>

//
// Definitions
//
#define EFI_ACPI_OEM_PIDV_REVISION                      0x00000002
#define EFI_ACPI_PLATFORM_ID_V_SIGNATURE  SIGNATURE_32('P', 'I', 'D', 'V')
#define PIDV_PART_NUMBER_LEN_MAX                      32
#define PIDV_EXT_ID_LEN_MAX                           32
#define PIDV_SYSTEM_UUID_LEN_MAX                      16

#pragma pack(1)

typedef struct {
  UINT8 Fru1:4;
  UINT8 Fru2:4;
  UINT8 Fru3:4;
  UINT8 Fru4:4;
  UINT8 Fru5:4;
  UINT8 Fru6:4;
  UINT8 Fru7:4;
  UINT8 Fru8:4;
  UINT8 Fru9:4;
  UINT8 Fru10:4;
  UINT8 Fru11:4;
  UINT8 Fru12:4;
  UINT8 Fru13:4;
  UINT8 Fru14:4;
  UINT8 Fru15:4;
  UINT8 Fru16:4;
  UINT8 Fru17:4;
  UINT8 Fru18:4;
  UINT8 Fru19:4;
  UINT8 Fru20:4;
} FIELD_REPLACEABLE_UNITS;

//
// SPID - 32 bytes
//
typedef struct {
  UINT16 CustomerID;
  UINT16 VendorID;
  UINT16 DeviceManufacturerID;
  UINT16 PlatformFamilyID;
  UINT16 ProductLineID;
  UINT16 HardwareID;
  FIELD_REPLACEABLE_UNITS Fru;
  UINT16 Reserved[5];
} SOFTWARE_PLATFORM_ID;
//
// PIDV structure
//
//
// Ensure proper structure formats
//
typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT8                       PartNumber[32];
  SOFTWARE_PLATFORM_ID        ExtID1;
  UINT8                       ExtID2[32];
  UINT8                       SystemUuid[16];
  UINT8                       IaFwBuildId[32];  /* unusedon Revision#2 */
  UINT32                      IaSvn;            /* unusedon Revision#2 */
  UINT32                      SecSvn;           /* unusedon Revision#2 */
  UINT32                      PdrSvn;           /* unusedon Revision#2 */
  UINT16                      IaFwRevValues[4]; /* unusedon Revision#2 */
  UINT16                      SecRevValues[4];  /* unusedon Revision#2 */
  UINT16                      PdrRevValues[4];  /* unusedon Revision#2 */
  UINT32                      OEM_TAG;          /* unusedon Revision#2 */  
} EFI_ACPI_PLATFORM_ID_V_TABLE;
#pragma pack()

#endif
