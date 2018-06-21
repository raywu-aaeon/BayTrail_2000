/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  BootMode.c

Abstract:

  EFI PEIM to provide the platform support functionality on the Thurley.

 
--*/
#include <FrameworkPei.h>
#include "PlatformBaseAddresses.h"
#include "PchAccess.h"
#include "PlatformBootMode.h"

#include <Ppi/Stall.h>
#include <Guid/PlatformInfo.h>
#include <Ppi/AtaController.h>
#include <Ppi/FindFv.h>
#include <Ppi/BootInRecoveryMode.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/Capsule.h>
#include <Guid/EfiVpdData.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Library/BaseLib.h>
#include <Library/PeiServicesLib.h>

BOOLEAN
GetSleepTypeAfterWakeup (
  IN  CONST EFI_PEI_SERVICES          **PeiServices,
  OUT UINT16                    *SleepType
  );

EFI_STATUS
UpdateBootMode (
  IN CONST EFI_PEI_SERVICES                       **PeiServices 
  )
{
  EFI_STATUS      Status;
  EFI_BOOT_MODE   BootMode;
  UINT16          SleepType;
  CHAR16          *strBootMode;

  Status = (*PeiServices)->GetBootMode(PeiServices, &BootMode);
  ASSERT_EFI_ERROR (Status);
  if (BootMode  == BOOT_IN_RECOVERY_MODE){
    return Status;
  }

  //
  // Let's assume things are OK if not told otherwise
  //
  BootMode = BOOT_WITH_FULL_CONFIGURATION;

    if (GetSleepTypeAfterWakeup (PeiServices, &SleepType)) {
      switch (SleepType) {
        case V_PCH_ACPI_PM1_CNT_S3:
          BootMode = BOOT_ON_S3_RESUME;
           break;

        case V_PCH_ACPI_PM1_CNT_S4:
          BootMode = BOOT_ON_S4_RESUME;
          break;

        case V_PCH_ACPI_PM1_CNT_S5:
          BootMode = BOOT_ON_S5_RESUME;
          break;
      } // switch (SleepType)
    }
    // Check for Safe Mode

  switch (BootMode) {
    case BOOT_WITH_FULL_CONFIGURATION:
      strBootMode = L"BOOT_WITH_FULL_CONFIGURATION";
      break;
    case BOOT_WITH_MINIMAL_CONFIGURATION:
      strBootMode = L"BOOT_WITH_MINIMAL_CONFIGURATION";
      break;
    case BOOT_ASSUMING_NO_CONFIGURATION_CHANGES:
      strBootMode = L"BOOT_ASSUMING_NO_CONFIGURATION_CHANGES";
      break;
    case BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS:
      strBootMode = L"BOOT_WITH_FULL_CONFIGURATION_PLUS_DIAGNOSTICS";
      break;
    case BOOT_WITH_DEFAULT_SETTINGS:
      strBootMode = L"BOOT_WITH_DEFAULT_SETTINGS";
      break;
    case BOOT_ON_S4_RESUME:
      strBootMode = L"BOOT_ON_S4_RESUME";
      break;
    case BOOT_ON_S5_RESUME:
      strBootMode = L"BOOT_ON_S5_RESUME";
      break;
    case BOOT_ON_S2_RESUME:
      strBootMode = L"BOOT_ON_S2_RESUME";
      break;
    case BOOT_ON_S3_RESUME:
      strBootMode = L"BOOT_ON_S3_RESUME";
      break;
    case BOOT_ON_FLASH_UPDATE:
      strBootMode = L"BOOT_ON_FLASH_UPDATE";
      break;
    case BOOT_IN_RECOVERY_MODE:
      strBootMode = L"BOOT_IN_RECOVERY_MODE";
      break;
    default:
      strBootMode = L"Unknown boot mode";
  } // switch (BootMode)

  DEBUG ((EFI_D_ERROR, "Setting BootMode to %S\n", strBootMode)); //CSP20130816
  Status = (*PeiServices)->SetBootMode(PeiServices, BootMode);
  ASSERT_EFI_ERROR (Status);

  return Status;
}

BOOLEAN
GetSleepTypeAfterWakeup (
  IN  CONST EFI_PEI_SERVICES          **PeiServices,
  OUT UINT16                    *SleepType
  )
/*++

Routine Description:

  Get sleep type after wakeup

Arguments:

  PeiServices       Pointer to the PEI Service Table.
  SleepType         Sleep type to be returned.

Returns:

  TRUE              A wake event occured without power failure.
  FALSE             Power failure occured or not a wakeup.

--*/
{
  UINT16  Pm1Sts;
  UINT16  Pm1Cnt;
  UINT16  GenPmCon1;
  //
  // VLV BIOS Specification 0.6.2 - Section 18.4, "Power Failure Consideration"
  //
  // When the SUS_PWR_FLR bit is set, it indicates the SUS well power is lost.
  // This bit is in the SUS Well and defaults to 1’b1 based on RSMRST# assertion (not cleared by any type of reset).
  // System BIOS should follow cold boot path if SUS_PWR_FLR (PBASE + 0x20[14]),
  // GEN_RST_STS (PBASE + 0x20[9]) or PWRBTNOR_STS (ABASE + 0x00[11]) is set to 1’b1
  // regardless of the value in the SLP_TYP (ABASE + 0x04[12:10]) field.
  //
  GenPmCon1 = MmioRead16 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1);
  //
  // Read the ACPI registers
  //
  Pm1Sts  = IoRead16 (ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_STS);
  Pm1Cnt  = IoRead16 (ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT);

  if ((GenPmCon1 & (B_PCH_PMC_GEN_PMCON_SUS_PWR_FLR | B_PCH_PMC_GEN_PMCON_GEN_RST_STS)) ||
     (Pm1Sts & B_PCH_ACPI_PM1_STS_PRBTNOR)) {
    // If power failure indicator, then don't attempt s3 resume.
    // Clear PM1_CNT of S3 and set it to S5 as we just had a power failure, and memory has
    // lost already.  This is to make sure no one will use PM1_CNT to check for S3 after
    // power failure.
    if ((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S3) {
      Pm1Cnt = ((Pm1Cnt & ~B_PCH_ACPI_PM1_CNT_SLP_TYP) | V_PCH_ACPI_PM1_CNT_S5);
      IoWrite16 (ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT, Pm1Cnt);
    }
    // Clear Wake Status (WAK_STS)
    //
//    IoWrite16 ((ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_STS), B_PCH_ACPI_PM1_STS_WAK);
   }
  //
  // Get sleep type if a wake event occurred and there is no power failure
  //
  if ((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S3) {
    *SleepType = Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP;
    return TRUE;
  } else if((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S4){
    *SleepType = Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP;
    return TRUE;
  }
  return FALSE;
}

