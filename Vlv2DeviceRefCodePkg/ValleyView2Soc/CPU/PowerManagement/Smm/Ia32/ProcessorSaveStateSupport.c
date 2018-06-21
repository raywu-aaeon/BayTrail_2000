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

  ProcessorSaveStateSupport.c

Abstract:

  Platform power management processor save state functionality.  This code has processor save state functionality.
  It must be runtime SMM safe.  There are different versions for different processor architectures and different
  implementations for 32 and 64 bit execution environments.

  Acronyms:
    PPM   Platform Power Management
    GV    Geyserville
    TM    Thermal Monitor
    IST   Intel(R) Speedstep technology
    HT    Hyper-Threading Technology

--*/

//
// Statements that include other files
//
#include "ProcessorSaveStateSupport.h"

//
// Function Implementations
//
EFI_STATUS
InitializeProcessorSaveState (
  VOID
  )
/*++

Routine Description:

  Initialize processor save state related infrastructure

Arguments:

  None

Returns:

  EFI_SUCCESS     The function completed successfully

--*/
{
  ASSERT (gSmst != NULL);
  return EFI_SUCCESS;
}
