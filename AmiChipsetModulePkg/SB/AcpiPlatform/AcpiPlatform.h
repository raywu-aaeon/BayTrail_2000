/*++

Copyright (c)  1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  AcpiPlatform.h
  
Abstract:

  This is an implementation of the ACPI platform driver.  Requirements for 
  this driver are defined in the Tiano ACPI External Product Specification,
  revision 0.3.6.

 
--*/

#ifndef _ACPI_PLATFORM_H_
#define _ACPI_PLATFORM_H_

//
// Statements that include other header files
//
#include <AcpiRes.h>
#include <acpi11.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/GlobalNvsArea.h>
#include <Protocol\VlvPlatformPolicy.h>    //(EIP134992+)
#include <Guid/PlatformInfo.h>
#include <Library/SbPolicy.h>
#include <Library/NbPolicy.h>
#include "PchAccess.h"
#include "VlvAccess.h"
#include <Library/PchPlatformLib.h>
#include <AcpiOemElinks.h>  //EIP134732

//
// Global variables
//
EFI_GLOBAL_NVS_AREA_PROTOCOL  mGlobalNvsArea;

EFI_STATUS
EFIAPI
AcpiPlatformEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  Entry point for Acpi platform driver.

Arguments:

  ImageHandle  -  A handle for the image that is initializing this driver.
  SystemTable  -  A pointer to the EFI system table.

Returns:

  EFI_SUCCESS           -  Driver initialized successfully.
  EFI_LOAD_ERROR        -  Failed to Initialize or has been loaded. 
  EFI_OUT_OF_RESOURCES  -  Could not allocate needed resources.

--*/
;

#endif
