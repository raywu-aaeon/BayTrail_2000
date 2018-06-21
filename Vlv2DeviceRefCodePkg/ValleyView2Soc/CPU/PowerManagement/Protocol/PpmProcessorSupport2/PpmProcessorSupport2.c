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

  PpmProcessorSupport2.c

Abstract:

  This protocol provides Platform Power Management support functionality and definitions.

--*/

//
// Statements that include other files
//
#include "Tiano.h"
#include "PpmProcessorSupport2.h"

//
// Protocol GUID definition
//
EFI_GUID  gPpmProcessorSupportProtocol2Guid = PPM_PROCESSOR_SUPPORT_PROTOCOL_2_GUID;

//
// Protocol description
//
EFI_GUID_STRING (&gPpmProcessorSupportProtocol2Guid, "PPM Processor Support Protocol 2", "This protocol provides PPM Processor Support functionality");
