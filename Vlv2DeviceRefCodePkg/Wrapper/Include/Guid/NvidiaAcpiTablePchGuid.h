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
;*      NvidiaAcpiTablePchGuid.h                                          *;
;*                                                                        *;
;*    Abstract:                                                           *;
;*                                                                        *;
;*      TPV ACPI H file.                                                  *;
;*                                                                        *;
;**************************************************************************/

#ifndef _NVIDIA_ACPI_TABLE_PCH_GUID_H_
#define _NVIDIA_ACPI_TABLE_PCH_GUID_H_

#define NVIDIA_ACPI_TABLE_PCH_GUID \
  { \
    0x7f1cabe3, 0x34d8, 0x4f54, 0x83, 0x1c, 0x9e, 0x1d, 0x52, 0xf4, 0x8f, 0x8e \
  }

#define NVIDIA_ACPI_TABLE_PCH_NAME L"NvidiaAcpiTablePch"

extern EFI_GUID gNvidiaAcpiTablePchGuid;

#endif
