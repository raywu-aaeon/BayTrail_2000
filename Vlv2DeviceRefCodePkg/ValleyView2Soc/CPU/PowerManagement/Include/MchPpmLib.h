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

  MchPpmLib.h

Abstract:

  Header file for MCH power management functionality

--*/

#ifndef _MCH_PPM_LIB_H_
#define _MCH_PPM_LIB_H_

//
// Statements that include other files
//
#include "Tiano.h"
#include "EfiBootScript.h"


EFI_STATUS
InitializeMchLib (
  VOID
  );
/*++

Routine Description:

  Initialize the MCH support library.
  This must be called once during driver initialization before using
  any of the other library services provided.

Arguments:

  None

Returns:

  EFI_SUCCESS   The library has been initialized correctly.

--*/

UINT16
DetermineiFsbFromMsr (
  VOID
  );
/*++

Routine Description:

  Determine the processor core frequency

Arguments:

  None

Returns:

  Processor core frequency multiplied by 3


--*/

BOOLEAN
MchSupportDynamicFsbFrequencySwitching (
  VOID
  );
/*++

Routine Description:

  Determines if MCH is capable of dynamic FSB frequency switching(Bus Geyserville)

Arguments:

  None

Returns:

  FALSE         Dynamic FSB frequency switching(Bus Geyserville) is NOT supported.
  TRUE          Dynamic FSB frequency switching(Bus Geyserville) is supported.

--*/

EFI_STATUS
EnableMchDynamicFsbFrequencySwitching (
  VOID
  );
/*++

Routine Description:

  Enables dynamic FSB frequency switching(Bus Geyserville) on MCH

Arguments:

  None

Returns:

  EFI_SUCCESS   Dynamic FSB frequency switching(Bus Geyserville) enabled

--*/

#endif
