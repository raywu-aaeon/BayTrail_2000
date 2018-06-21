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

  VlvInit.c

Abstract:

  This is the driver that initializes the Intel ValleyView.

--*/

#include "VlvInit.h"
#include <Protocol/VlvPlatformPolicy.h>

extern DXE_VLV_PLATFORM_POLICY_PROTOCOL  *DxePlatformSaPolicy;
#ifdef ECP_FLAG
EFI_GUID gDxeVlvPlatformPolicyGuid = DXE_VLV_PLATFORM_POLICY_GUID;
#endif

EFI_STATUS
EFIAPI
VlvInitEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
/*++

Routine Description:

  This is the standard EFI driver point that detects
  whether there is an ICH southbridge in the system
  and if so, initializes the chip.

Arguments:

  ImageHandle             Handle for the image of this driver
  SystemTable             Pointer to the EFI System Table

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
{
//bteo1  UINT32  DwordReg = 0xFFFFFFFF;
//bteo1  UINT32  temp = 0;
  EFI_STATUS                        Status;

#ifdef ECP_FLAG
  INITIALIZE_SCRIPT (ImageHandle, SystemTable);
#endif
  //

  //
  // LegacyRegion Driver
  //
  DEBUG ((EFI_D_ERROR, "Initializing Legacy Region\n"));
  LegacyRegionInstall (ImageHandle,SystemTable);

//commented by bteo1 as Bios_Reset_Done moved to SEC phase.
//need to confirm again after PO as PM registers not setup at SEC
#if (0)
  //
  // BIOS Reset Complete
  //
  MsgBus32Or( VLV_PUNIT,PUNIT_BIOS_RESET_CPL,DwordReg, BIT0);
  MsgBus32Read( VLV_PUNIT,PUNIT_BIOS_RESET_CPL,DwordReg);
  SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN)(EC_BASE + 0xD8),
    1,
    &temp
    );

  SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN)(EC_BASE + 0xD4),
    1,
    &DwordReg
    );

  temp = (((0x07000000) | (VLV_PUNIT << 16) | ((PUNIT_BIOS_RESET_CPL) <<8) + 0xF0));
  SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN)(EC_BASE + 0xD0),
    1,
    &temp
    );

  DEBUG ((EFI_D_ERROR, "BIOS Reset Complete\n"));
#endif //end temp comment

  Status = gBS->LocateProtocol (&gDxeVlvPlatformPolicyGuid, NULL, (VOID **) &DxePlatformSaPolicy);
  ASSERT_EFI_ERROR (Status);

// AMI_OVERRIDE - Dump Vlv Policy information. >>
  ///
  /// Dump whole DXE_VLV_PLATFORM_POLICY_PROTOCOL and serial out.
  ///
  VlvDumpPlatformProtocol (DxePlatformSaPolicy);
// AMI_OVERRIDE - Dump Vlv Policy information. <<

  //
  // GtPostInit Initialization
  //
  DEBUG ((EFI_D_ERROR, "Initializing GT PowerManagement and other GT POST related\n"));
  GraphicsDxeInit (ImageHandle, DxePlatformSaPolicy);

  //
  // IgdOpRegion Install Initialization
  //
  DEBUG ((EFI_D_ERROR, "Initializing IGD OpRegion\n"));
  IgdOpRegionInit ();

  return EFI_SUCCESS;
}

