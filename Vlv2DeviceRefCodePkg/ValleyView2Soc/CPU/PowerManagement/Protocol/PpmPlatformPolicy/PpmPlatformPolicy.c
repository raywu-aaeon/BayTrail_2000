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

  PpmPlatformPolicy.c

Abstract:

  This file defines the PPM Policy Protocol.

--*/

//
// Statements that include other files
//
#include "Tiano.h"
#include "PpmPlatformPolicy.h"

//
// Protocol GUID definition
//
EFI_GUID gPpmPlatformPolicyProtocolGuid = PPM_PLATFORM_POLICY_PROTOCOL_GUID;

//
// Protocol description
//
EFI_GUID_STRING (&gPpmPlatformPolicyProtocolGuid, "PpmPlatformPolicy Protocol", "Intel(R) DXE Phase PPM Platform Policy Protocol");
