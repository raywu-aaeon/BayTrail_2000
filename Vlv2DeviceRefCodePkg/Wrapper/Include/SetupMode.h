/*++

Copyright (c)  1999 - 2004 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    SetupMode.h

Abstract:

    EFI Setup Mode



--*/

#ifndef _EFI_SETUP_MODE_H_
#define _EFI_SETUP_MODE_H_

//
// Global ID for the Setup Mode
//

#define EFI_PLATFORM_BOOT_MODE_GUID \
  { 0xce845704, 0x1683, 0x4d38, 0xa4, 0xf9, 0x7d, 0xb, 0x50, 0x77, 0x57, 0x93 }

#define EFI_NORMAL_SETUP_GUID \
  { 0xec87d643, 0xeba4, 0x4bb5, 0xa1, 0xe5, 0x3f, 0x3e, 0x36, 0xb2, 0xd, 0xa9 }

#define EFI_NORMAL_SETUP_RESET_NAME L"Reset"

enum {
    //
    // This means: "whatever reset defaults in setup does"
    //
    SetupDataResetNormal        = 0,

    //
    // This means: "the defaults built into the BIOS"
    //
    SetupDataResetStandard      = 1,

    //
    // This means: "the manufacturing mode defaults"
    //
    SetupDataResetManufacturing = 2,

    //
    // This means: "the oem defaults"
    //
    SetupDataResetOem           = 3,
};

//
// PlatformBootMode types
//
#define PLATFORM_NORMAL_MODE          0x01
#define PLATFORM_SAFE_MODE            0x02
#define PLATFORM_RECOVERY_MODE        0x04
#define PLATFORM_MANUFACTURING_MODE   0x08
#define PLATFORM_BACK_TO_BIOS_MODE    0x10

extern EFI_GUID gEfiPlatformBootModeGuid;

#define   NORMAL_SETUP_NAME                L"Setup"
#define   IN_SETUP_NAME                    L"InSetup"
#define   SYSTEM_PASSWORD_NAME             L"SystemPassword"
#define   BOOT_TIME_NAME                   L"BootTime"

extern EFI_GUID gEfiNormalSetupGuid;
extern CHAR16   gEfiNormalSetupName[];
extern CHAR16   gEfiInSetupName[];
extern CHAR16   gEfiSystemPasswordName[];

typedef struct {
    EFI_GUID    SetupGuid;
    CHAR16      SetupName[0x20];          // Maximum "Setup" Name
    UINT32      PlatformBootMode;
} EFI_PLATFORM_SETUP_ID;

#endif
