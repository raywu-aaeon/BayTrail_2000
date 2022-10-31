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

  LpssDxe.h

Abstract:


--*/

#ifndef _ISP_DXE_H_
#define _ISP_DXE_H_

#ifdef ECP_FLAG
#include "EDKIIGlueDxe.h"
#include "Acpi3_0.h"
#include "VlvCommonDefinitions.h"
#endif
#ifndef ECP_FLAG
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <IndustryStandard/Acpi30.h>
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/AcpiSupport.h>
#include <Protocol/GlobalNvsArea.h>
#endif
#include <Library/S3BootScriptLib.h>
#include <PchAccess.h> 

//
// AML parsing definitions
//
#define AML_NAME_OP           0x08
#define AML_BUFFER_OP         0x11
#define AML_DEVICE_OP         0x82
#define AML_MEMORY32_FIXED_OP 0x86
#define AML_DWORD_OP          0x87
#define AML_INTERRUPT_DESC_OP 0x89

#endif
