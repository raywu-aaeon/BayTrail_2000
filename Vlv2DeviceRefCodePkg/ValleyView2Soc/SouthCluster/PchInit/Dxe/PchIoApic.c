/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  PchIoApic.c

  @brief
  Initializes PCH IO APIC Device.

**/
#include "PchInit.h"

EFI_STATUS
ConfigureIoApic (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN  UINT32                              IlbBase
  )
/**

  @brief
  Configure IoApic Controler

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] IlbBase              IlbBase address of this PCH device

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINT8               RegData8;
  UINT32              RegData32;

  DEBUG ((EFI_D_INFO, "ConfigureIoApic() Start\n"));

  ///
  /// VLV BIOS Spec, Section 28.4.1.1
  /// 1. Set the AEN bit, IBASE + 0x60 [8], to 1’b1. Read back the value written.
  /// Done in PchInitPeim.c PchIoApicInit()
  ///
  /// VLV BIOS Spec, Section 28.4.1.1
  /// 2. Build the MP table and/or ACPI APIC table for the OS
  /// This will be done in ACPI code.
  ///
  /// VLV BIOS Spec, Section 28.4.1.1
  /// 3. Maximum Redirection Entries (MRE) in APIC Version Register (VER), offset 01h,
  ///    [23:16] has to be written once for Microsoft Windows OS.
  ///
  if ((MmioRead16 (IlbBase + R_PCH_ILB_OIC) & (UINT16) B_PCH_ILB_OIC_AEN) == B_PCH_ILB_OIC_AEN) {
    RegData8 = R_PCH_IO_APIC_VS;
    MmioWrite8 ((UINTN) R_PCH_IO_APIC_INDEX, RegData8);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint8,
      (UINTN) R_PCH_IO_APIC_INDEX,
      1,
      &RegData8
      );

    RegData32 = 0x170020; // Default value for APIC Version Register
    MmioWrite32 (R_PCH_IO_APIC_WINDOW, RegData32);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) R_PCH_IO_APIC_WINDOW,
      1,
      &RegData32
      );
  }
  DEBUG ((EFI_D_INFO, "ConfigureIoApic() End\n"));

  return EFI_SUCCESS;
}
