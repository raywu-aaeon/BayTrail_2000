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
  Battery and Energe Management Table.

--*/

#ifndef _BEMT_H
#define _BEMT_H

//
// Statements that include other files
//
#include <IndustryStandard/Acpi10.h>
#include <IndustryStandard/Acpi20.h>
#include <IndustryStandard/Acpi30.h>

#define EFI_ACPI_OEM0_REVISION                      0x00
#define EFI_ACPI_OEM0_SIGNATURE                         SIGNATURE_32('O', 'E', 'M', '0')

#define EFI_ACPI_OEM_ID           'I','N','T','E','L',' '   // OEMID 6 bytes long
#define EFI_ACPI_OEM0_TABLE_ID     SIGNATURE_64('B','A','T','T','E','R','Y','T') // OEM table id 8 bytes long
#define CREATOR_ID       SIGNATURE_32('I','N','T','L')  

#define ACPI_TEMP_RANGES_NUMBER    4

#pragma pack(1)

typedef struct {
  UINT8  ClassID;
  UINT32 Dummy;
  UINT8  Tolerance;
  UINT16 BattID;   //BattID in Ohm from PMIC
}BATTERY_ID;

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  BATTERY_ID     BatteryId;
  UINT16    MaxVoltage;
  UINT32    Capacity;
  UINT16    BatteryType;
  UINT16    TempMonitorRangesNumber;
  struct {
    UINT16  TempUL;
    UINT16  RBatt;
    UINT16  FullChargeVolt;
    UINT16  FullChargeCurr;
    UINT16  MaintChargeStartThreshVolt;
    UINT16  MaintChargeStopThreshVolt;
    UINT16  MaintChargeCurr;
  } TempMonitorRanges [ACPI_TEMP_RANGES_NUMBER];
  UINT16  TempLL;
} EFI_ACPI_OEM0_TABLE;

#pragma pack()


#endif
