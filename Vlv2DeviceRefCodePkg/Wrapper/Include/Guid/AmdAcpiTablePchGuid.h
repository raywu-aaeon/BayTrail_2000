/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*    FOR INTERNAL USE ONLY.  NO EXTERNAL DISTRIBUTION ALLOWED.           *;
;*                                                                        *;
;*    Intel Corporation - Switchable Graphics ACPI Code for Intel         *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*    Copyright (c) 2011 - 2012 Intel Corporation. All rights reserved    *;
;*                                                                        *;
;*    Module Name:                                                        *;
;*                                                                        *;
;*      AmdAcpiTablePchGuid.h                                             *;
;*                                                                        *;
;*    Abstract:                                                           *;
;*                                                                        *;
;*      TPV ACPI H file.                                                  *;
;*                                                                        *;
;**************************************************************************/

#ifndef _AMD_ACPI_TABLE_PCH_GUID_H_
#define _AMD_ACPI_TABLE_PCH_GUID_H_

#define AMD_ACPI_TABLE_PCH_GUID \
  { \
    0x77aed82e, 0x77de, 0x42ca, 0x8c, 0x27, 0xe9, 0xd7, 0x1d, 0xf6, 0x06, 0xc7 \
  }

#define AMD_ACPI_TABLE_PCH_NAME L"AmdAcpiTablePch"

extern EFI_GUID gAmdAcpiTablePchGuid;

#endif
