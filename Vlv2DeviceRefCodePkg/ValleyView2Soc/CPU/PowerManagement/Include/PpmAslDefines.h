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

  PpmAslDefines.h

Abstract:

  IST/PPM reference code definitions.

  Acronyms:
    PPM   Platform Power Management
    GV    Geyserville
    TM    Thermal Monitor
    IST   Intel(R) Speedstep technology

--*/

#ifndef _PPM_COMMON_H_
#define _PPM_COMMON_H_


// <NOT FOR REF START>---------------------------------------------------------
#include "token.h"
// <NOT FOR REF END>-----------------------------------------------------------

//
// <TODO>-The following definitions are based on assumed location for the  ACPI
// Base Address.  Modify as necessary base on platform-specific requirements.
//
// <NOT FOR REF START>---------------------------------------------------------
#if ACPI_BASE_ADDRESS != 0x400
#error Update the definitions below to as needed...
#endif
// <NOT FOR REF END>-----------------------------------------------------------
#define ICH_ACPI_LV2              0x414
#define ICH_ACPI_LV3              0x415
#define ICH_ACPI_LV4              0x416
#define ICH_ACPI_LV6              0x418

// C-States Latencies and Powers
// Should be adjusted according to latest silicon and processors
//
#define C1_LATENCY    1
#define C1_POWER      1000
#define C2_LATENCY    1//20
#define C2_POWER      500
#define C3_LATENCY    17
#define C3_POWER      250
#define C4_LATENCY    57
#define C4_POWER      100
#define C6_LATENCY    162
#define C6_POWER      100
#endif
