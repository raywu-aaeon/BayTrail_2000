/** @file
  TPM2 ACPI table definition.

Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _TPM2_ACPI_H_
#define _TPM2_ACPI_H_

#include <IndustryStandard/Acpi.h>

#pragma pack (1)

#define EFI_TPM2_ACPI_TABLE_REVISION  3

typedef struct {
  EFI_ACPI_DESCRIPTION_HEADER Header;
  UINT32                      Flags;
  UINT64                      AddressOfControlArea;
  UINT32                      StartMethod;
//UINT8                       PlatformSpecificParameters[];
} EFI_TPM2_ACPI_TABLE;

#define EFI_TPM2_ACPI_TABLE_START_METHOD_ACPI  2
#define EFI_TPM2_ACPI_TABLE_START_METHOD_TIS   6

typedef struct {
  UINT32   Reserved;
  UINT32   Error;
  UINT32   Cancel;
  UINT32   Start;
  UINT64   InterruptControl;
  UINT32   CommandSize;
  UINT64   Command;
  UINT32   ResponseSize;
  UINT64   Response;
} EFI_TPM2_ACPI_CONTROL_AREA;

#pragma pack ()

#endif
