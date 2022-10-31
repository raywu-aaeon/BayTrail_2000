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

  IchPpmLib.h

Abstract:

  Header file for ICH power management functionality

--*/

#ifndef _ICH_PPM_LIB_H_
#define _ICH_PPM_LIB_H_

//
// Statements that include other files
//
#include "Tiano.h"

//
// Definitions for IchEnableC5C6 use
//
#define ENABLE_LVL5                 5
#define ENABLE_LVL6                 6

//
// Function prototypes
//
EFI_STATUS
InitializeIchLib (
  VOID
  );
/*++

Routine Description:

  Initialize the ICH support library.
  This must be called once during driver initialization before using
  any of the other library services provided.

Arguments:

  None

Returns:

  EFI_SUCCESS   The library has been initialized correctly.

--*/

EFI_STATUS
ConfigureC4OnC3 (
  IN BOOLEAN        PowerState,
  IN UINTN          C3,
  IN UINTN          C4,
  IN UINT16         OperatingSystem
  );
/*++

Routine Description:

  Configure the C4 on C3 chipset feature.  This will look at the current
  Cx support, power state, and operating system and configure C4 on C3
  appropriately for maximum power savings.

Arguments:

  PowerState        Current power state, TRUE = AC power, FALSE = Battery power
  C3                !0 = C3 supported/enabled
  C4                !0 = C4 supported/enabled
  OperatingSystem   >= 2001 means XP or later

Returns:

  EFI_SUCCESS     C4 on C3 configuration updated successfully

--*/

EFI_STATUS
EnableC4OnC3 (
  VOID
  );
/*++

Routine Description:

  Enable C4 on C3 in the chipset

Arguments:

  None

Returns:

  EFI_SUCCESS   C4 on C3 enabled

--*/

EFI_STATUS
DisableC4OnC3 (
  VOID
  );
/*++

Routine Description:

  Disable C4 on C3 in the chipset

Arguments:

  None

Returns:

  EFI_SUCCESS   C4 on C3 disabled

--*/

UINT16
GetAcpiBase (
  VOID
  );
/*++

Routine Description:

  Return the PM base address register current setting.

Arguments:

  None

Returns:

  Current PM base address setting

--*/

VOID
IchEnableC5C6 (
  IN UINT8    CxState
  );
/*++

Routine Description:

  Enable C5 or C6 support in ICH

Arguments:

  CxState  - 5 - LVL5
             6 - LVL6

Returns:

  None

--*/

UINT32
GetPm2CntOffset (
  VOID
  );
/*++

Routine Description:

  Get the PM2_CNT offset since it is different for ICH8M and ICH9M

Arguments:

  None

Returns:

  PM_PM2_CNT   PM2_CNT offset

--*/

#endif
