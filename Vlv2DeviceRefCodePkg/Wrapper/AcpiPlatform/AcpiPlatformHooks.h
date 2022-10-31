/*++
Copyright 1996 - 2008, Intel Corporation.

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

  AcpiPlatformHooks.h

Abstract:

  This is an implementation of the ACPI platform driver.  Requirements for
  this driver are defined in the Tiano ACPI External Product Specification,
  revision 0.3.6.

--*/

#ifndef _ACPI_PLATFORM_HOOKS_H_
#define _ACPI_PLATFORM_HOOKS_H_

//
// Statements that include other header files
//

#include <IndustryStandard/Acpi.h>
#include "Platform.h"
#include <Protocol/EnhancedSpeedstep.h>

#define AML_NAME_OP           0x08
#define AML_METHOD_OP         0x14
#define AML_OPREGION_OP       0x80
#define AML_PACKAGE_OP        0x12  // Package operator.
//#define AML_NOOP_OP           0xA3
#define AML_NAME_PREFIX_SIZE  0x06
#define AML_NAME_DWORD_SIZE   0x0C

#pragma pack(1)

typedef struct {
    UINT8   AcpiProcessorId;
    UINT8   ApicId;
    UINT16  Flags;
} EFI_CPU_ID_MAP;

typedef struct {
    UINT8   StartByte;
    UINT32  NameStr;
    UINT8   Size;
    UINT32  Value;
} EFI_ACPI_NAMEPACK_DWORD;

typedef struct {
    UINT8   StartByte;
    UINT32  NameStr;
    UINT8   OpCode;
    UINT16  Size;                     // Hardcode to 16bit width because the table we use is fixed size
    UINT8   NumEntries;
} EFI_ACPI_NAME_COMMAND;

typedef struct {
    UINT8   PackageOp;
    UINT8   PkgLeadByte;
    UINT8   NumEntries;
    UINT8   DwordPrefix0;
    UINT32  CoreFreq;
    UINT8   DwordPrefix1;
    UINT32  Power;
    UINT8   DwordPrefix2;
    UINT32  TransLatency;
    UINT8   DwordPrefix3;
    UINT32  BMLatency;
    UINT8   DwordPrefix4;
    UINT32  Control;
    UINT8   DwordPrefix5;
    UINT32  Status;
} EFI_PSS_PACKAGE;

typedef struct {
    UINT8 PackageOp;
    UINT8 PkgLeadByte;
    UINT8 NumEntries;
    UINT8 BytePrefix0;
    UINT8 Entries;
    UINT8 BytePrefix1;
    UINT8 Revision;
    UINT8 BytePrefix2;
    UINT8 Domain;
    UINT8 BytePrefix3;
    UINT8 Coordinate;
    UINT8 BytePrefix4;
    UINT8 ProcNumber;
} EFI_PSD_PACKAGE;

#pragma pack()

#define ACPI_NAME_COMMAND_FROM_NAME_STR(a)  BASE_CR (a, EFI_ACPI_NAME_COMMAND, NameStr)
#define ACPI_NAME_COMMAND_FROM_NAMEPACK_STR(a)  BASE_CR (a, EFI_ACPI_NAMEPACK_DWORD, NameStr)

EFI_STATUS
PlatformHookInit(
    VOID
);

EFI_STATUS
PatchDsdtTable(
    IN OUT   EFI_ACPI_DESCRIPTION_HEADER  *TableHeader
);

EFI_STATUS
PatchGv3SsdtTable(
    IN OUT  EFI_ACPI_DESCRIPTION_HEADER *Table
);

EFI_STATUS
PatchErstTable(
    IN OUT  EFI_ACPI_DESCRIPTION_HEADER *Table
);

EFI_STATUS
AppendCpuMapTableEntry(
    IN EFI_ACPI_2_0_PROCESSOR_LOCAL_APIC_STRUCTURE   *AcpiLocalApic
);

#endif
