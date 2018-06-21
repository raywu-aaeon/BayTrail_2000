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
  Energe Management Table extra.

--*/

#ifndef _EMT1_H
#define _EMT1_H

//
// Statements that include other files
//
#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include <IndustryStandard/Acpi30.h>

#define EFI_ACPI_EMT1_REVISION                          0x00000000
#define EFI_ACPI_OEM1_REVISION                          0x00000001

#define EFI_ACPI_EMT1_SIGNATURE                         SIGNATURE_32('E', 'M', '_', '1')
#define EFI_ACPI_OEM1_SIGNATURE                         SIGNATURE_32('O', 'E', 'M', '1')

#define EFI_ACPI_EMT1_TABLE_ID     SIGNATURE_64('O','E','M','1',' ',' ',' ',' ') // OEM table id 8 bytes long
#define EFI_ACPI_OEM1_TABLE_ID     SIGNATURE_64('E','N','R','G','Y','M','G','T') // OEM table id 8 bytes long

#pragma pack(1)

typedef struct {
    EFI_ACPI_DESCRIPTION_HEADER Header;
    UINT16   IaAppsRun;
    UINT8    IaAppsCap;
    UINT8    CapOrVoltFlag;    
    UINT8    BootOnInvalidBatt;
} EFI_ACPI_ENERGY_MANAGEMENT_1_TABLE;

typedef struct {
    EFI_ACPI_DESCRIPTION_HEADER Header;
	UINT8                       FixedOption0;
	UINT8                       FixedOption1;
	UINT8                       DBIInGpioNumber;
	UINT8                       DBIOutGpioNumber;
	UINT8                       BatChpType;
	UINT16                      IaAppsRun;
	UINT8                       BatIdDBIBase;
	UINT8                       BatIdAnlgBase;
	UINT8                       IaAppsCap;
	UINT16                      VBattFreqLmt;
	UINT8                       CapFreqIdx;
	UINT8                       Rsvd1;
	UINT8                       BatIdx;
	UINT8                       IaAppsToUse;
	UINT8                       TurboChrg;
	UINT8                       Rsvd2[11];
}EFI_ACPI_EM_OEM_1_TABLE;

#pragma pack()


#endif
