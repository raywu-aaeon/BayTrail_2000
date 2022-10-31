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

  ValleyviewPpmLib.h

Abstract:

  This library contains power management configuration functions specific to
  ValleyView processors.

  Acronyms:
    PPM   Platform Power Management
    GV    Geyserville
    TM    Thermal Monitor
    IST   Intel(R) Speedstep technology
    HT    Hyper-Threading Technology
    CMP   Core Multi-Processing

--*/

#ifndef _Valleyview_PPM_LIB_H_
#define _Valleyview_PPM_LIB_H_

//
// Statements that include other files
//
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#else
#include "PiDxe.h"
#endif
//
// MSR definitions
//
#define BSEL_CR_OVERCLOCK_CONTROL   0xCD
#define FUSE_BSEL_MASK                  0x03

#endif
