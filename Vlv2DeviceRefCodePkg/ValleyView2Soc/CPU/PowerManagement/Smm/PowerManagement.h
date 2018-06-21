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
#endif
#include <Guid/PowerManagementAcpiTableStorage.h>

#include <Ppm.h>
#include <CpuPpmLib.h>
#include <IstAppletLib.h>
#include <DevicePathHelpers.h>
#include <MchPpmLib.h>
#include <IchPpmLib.h>
#include <PchAccess.h>
#include <ProcessorSaveStateSupport.h>

typedef struct {
  UINT32  RegEax;
  UINT32  RegEbx;
  UINT32  RegEcx;
  UINT32  RegEdx;
} EFI_CPUID_REGISTER;
#ifdef ECP_FLAG
#ifdef __GNUC__
#if __GNUC__ >= 4
#define OFFSET_OF(TYPE, Field) ((UINTN) __builtin_offsetof(TYPE, Field))
#endif
#endif
#ifndef OFFSET_OF
#define OFFSET_OF(TYPE, Field) ((UINTN) &(((TYPE *)0)->Field))
#endif
#endif

//
// Callback function prototypes
//

VOID
S3RestoreMsrCallback (
  IN  EFI_HANDLE                    DispatchHandle,
  IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
  );
/*++

Routine Description:

  PPM must restore runtime state of MSR.  This is not supported by the S3 boot script.
  In order to accomplish this, the ASL is modified to generate an SMI on S3 in the _WAK method.
  This SMI handler reponds to that SW SMI.

Arguments:

  DispatchHandle  - The handle of this callback, obtained when registering
  DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT

Returns:

  None.

--*/

///VLV not support for Io Trap method


EFI_STATUS
InitializePowerManagement (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );
/*++

Routine Description:

  Initialize the power management support.
  This function will do boot time configuration:
    Install into SMRAM/SMM
    Detect HW capabilities and SW configuration
    Initialize HW and software state (primarily MSR and ACPI tables)
    Install SMI handlers for runtime interfacess

Arguments:

  ImageHandle   - Pointer to the loaded image protocol for this driver
  SystemTable   - Pointer to the EFI System Table

Returns:

  EFI_SUCCESS   The driver installed/initialized correctly.
  Driver will ASSERT in debug builds on error.  PPM functionality is considered critical for mobile systems.

--*/


#endif
