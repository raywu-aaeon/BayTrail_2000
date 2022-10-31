/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  2014 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PowerManagement.h

Abstract:

  Header file for the power management driver.
  This driver loads power management support designed to be similar to
  the mobile platform power management reference code.

--*/

#ifndef _POWER_MANAGEMENT_H_
#define _POWER_MANAGEMENT_H_

//
// Statements that include other files
//
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include "AslUpdateLib.h"
#include "CpuRegs.h"
#endif
#ifndef ECP_FLAG
#include <PiDxe.h>
#include <Protocol/AcpiSupport.h>
#include <Protocol/SmmSwDispatch.h>
#endif
#include <Protocol/GlobalNvsArea.h>
#ifndef ECP_FLAG
#include <Protocol/BootScriptSave.h>
#include <Protocol/PpmPlatformPolicy.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/LoadedImage.h>
#include <Library/AslUpdateLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/UefiLib.h>
#include <Protocol/ExitPmAuth.h>
#endif



EFI_STATUS
InitializePowerManagement (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );
/*++

Routine Description:

  Initialize the power management support.
  This function will do boot time configuration:
     Install S3 boot script to refill CPU Data.

Arguments:

  ImageHandle   - Pointer to the loaded image protocol for this driver
  SystemTable   - Pointer to the EFI System Table

Returns:

  EFI_SUCCESS   The driver installed/initialized correctly.
  Driver will ASSERT in debug builds on error.  PPM functionality is considered critical for mobile systems.

--*/
VOID
EFIAPI
PpmInitBeforeBoot (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  );
/**

  @brief
  Power management initialization before Boot Script Table is closed

  @param[in] Event                A pointer to the Event that triggered the callback.
  @param[in] Context              A pointer to private data registered with the callback function.

  @retval EFI_SUCCESS             The function completed successfully

**/

#endif
