/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2014 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  VlvInitHooks.c

Abstract:

  This file has Hooks implementation which can be invoked by ODMs.

--*/

#include "VlvInitHooks.h"

//FYI: Hooks "PunitBIOSConfigForVccVnnHook"  & "PunitBIOSConfigForVccVnnGetStatusHook" are removed, because 
//PS2EN_VNN_VCC bits3:2 of PUNIT, BIOS_CONFIG Register (offset 0x6) is moved to SEC phase because of these settings
//should be done before BIOS_RESET_DONE.

// This file is left to provide other Hooks in future when needed.