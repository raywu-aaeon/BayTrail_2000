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

  VlvInit.h

Abstract:

  Header file for SA Initialization Driver.

--*/

#ifndef _VLV_INITIALIZATION_DRIVER_H_
#define _VLV_INITIALIZATION_DRIVER_H_

#include <Token.h>    // AMI_OVERRIDE
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include "EfiScriptLib.h"
#else
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#endif

#include <Protocol/VlvPlatformPolicy.h>

#include "LegacyRegion.h"
#include "GraphicsDxeInit.h"
#include "IgdOpRegion.h"
#ifndef ECP_FLAG
#include "PiDxe.h"
#endif
#include "Valleyview.h"
#ifdef ECP_FLAG
#include "PchAccess.h"
#endif


//
// External include files do NOT need to be explicitly specified in real EDKII
// environment
//


typedef struct {
  UINT64  BaseAddr;
  UINT32  Offset;
  UINT32  AndMask;
  UINT32  OrMask;
} BOOT_SCRIPT_REGISTER_SETTING;

// AMI_OVERRIDE - Dump Vlv Policy information. >>
VOID
VlvDumpPlatformProtocol (
  IN  DXE_VLV_PLATFORM_POLICY_PROTOCOL    *PlatformSaPolicy
);
// AMI_OVERRIDE - Dump Vlv Policy information. <<

#endif

