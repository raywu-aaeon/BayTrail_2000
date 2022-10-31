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

  IchTptPpmLib.c

Abstract:

  Implementation file for Tigerpoint-ICH power management functionality

--*/

//
// Statements that include other files
//
#include "IchTptPpmLib.h"

//
// Global variables
//

//
// BootScriptSave protocol for saving configuration changes
//
extern EFI_BOOT_SCRIPT_SAVE_PROTOCOL *mBootScriptSave;

//
// Function Definitions
//
EFI_STATUS
InitializeIchLib (
  VOID
  )
/*++

Routine Description:

  Initialize the ICH library

Arguments:

  None

Returns:

  None

--*/
{
  ASSERT (gSmst != NULL);
  ASSERT (mBootScriptSave != NULL);

  return EFI_SUCCESS;
}

EFI_STATUS
ConfigureC4OnC3 (
  IN BOOLEAN        PowerState,
  IN UINTN          C3,
  IN UINTN          C4,
  IN UINT16         OperatingSystem
  )
/*++

Routine Description:

  Configure the C4 on C3 chipset feature.  This will look at the current
  Cx support, power state, and operating system and configure C4 on C3
  appropriately for maximum power savings.

  This is the recommended algorithm that we use:
  ;
  ;  The following is the suggest checks for enabling C4-on-C3 in the chipset
  ;  power events and ACPI initialization:
  ;
  ; If (C4 NOT supported/enabled)     ; C4 not supported.
  ;   Disable C4-on-C3 in the chipset
  ; If (C3 NOT supported/enabled)     ; C4 enabled,but not C3!
  ;   Enable C4-on-C3 in the chipset
  ; If (AC mode)          ; If on AC, use C3.
  ;   Disable C4-on-C3 in the chipset
  ; If (OS >= WinXP)        ; C1 SMI or _CST.
  ;   Disable C4-on-C3 in the chipset
  ; Else
  ;   Enable C4-on-C3 in the chipset    ; Older OS support
  ;

Arguments:

  PowerState        TRUE = AC power, FALSE = Battery power
  C3                !0 = C3 supported/enabled
  C4                !0 = C4 supported/enabled
  OperatingSystem   >= 2001 means XP or later

Returns:

  EFI_SUCCESS     C4 on C3 configuration updated successfully

--*/
{
  //
  // Configure C4 on C3
  //
  if (!C4) {
    //
    // No C4, then we can't enable
    //
    DisableC4OnC3 ();
  } else if (!C3) {
    //
    // If no C3 and no C4, then disable
    // BUGBUG: Verify with PPM legacy code
    //
    EnableC4OnC3 ();
  } else if (PowerState) {
    //
    // If on AC power, then we don't do C3 or C4, so disable
    //
    DisableC4OnC3 ();
  } else if (OperatingSystem >= 2001) {
    //
    // XP or later has better control via C1 SMI or _CST, so disable C4 on C3
    //
    DisableC4OnC3 ();
  } else {
    //
    // Default case for old OS using FADT structure
    // C3 will generate C4 and save more power.
    //
    EnableC4OnC3 ();
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EnableC4OnC3 (
  VOID
  )
/*++

Routine Description:

  Enable C4 on C3 in the chipset

Arguments:

  None

Returns:

  EFI_SUCCESS   C4 on C3 enabled

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
DisableC4OnC3 (
  VOID
  )
/*++

Routine Description:

  Disable C4 on C3 in the chipset

Arguments:

  None

Returns:

  EFI_SUCCESS   C4 on C3 disabled

--*/
{
  return EFI_SUCCESS;
}

UINT16
GetAcpiBase (
  VOID
  )
/*++

Routine Description:

  Return the PM base address register current setting.

Arguments:

  None

Returns:

  Current PM base address setting

--*/
{
  return PchLpcPciCfg16 (R_PCH_LPC_ACPI_BASE) & B_PCH_LPC_ACPI_BASE_BAR;
}

VOID
IchEnableC5C6 (
  UINT8    CxState
  )
/*++

Routine Description:

  Enable C5 or C6 support in ICH

Arguments:

  CxState  - 5 - LVL5
             6 - LVL6

Returns:

  None

--*/
{
  UINT16           GpioBase;
  UINT32           Buffer32;

  //
  // Ensure boot script initialized
  //
  ASSERT (mBootScriptSave != NULL);

  GpioBase = PchLpcPciCfg16 (R_PCH_LPC_GPIO_BASE) & B_PCH_LPC_GPIO_BASE_BAR;

  //
  // Enable PMSYNC# by setting BM_BUSY#/PMSYNC# for native operation.
  //
  gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT32, GpioBase + R_PCH_GPIO_SC_USE_SEL, 1, &Buffer32);
  Buffer32 &= ~B_GPIO_BM_BUSY;
  gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT32, GpioBase + R_PCH_GPIO_SC_USE_SEL, 1, &Buffer32);

  //
  // Save latest settings to runtime script table
  //
  S3BootScriptSavePciCfgWrite(
    EfiBootScriptWidthUint32,
    (UINT64) (GpioBase + R_PCH_GPIO_SC_USE_SEL),
    (UINTN) 1,
    &Buffer32
    );
}

UINT32
GetPm2CntOffset (
  VOID
  )
/*++

Routine Description:

  Get the PM2_CNT Offset since it is different for ICH8M and ICH9M

Arguments:

  None

Returns:

  PM_PM2_CNT   PM2_CNT Offset

--*/
{
  return R_PCH_ACPI_PM2_CNT;
}
