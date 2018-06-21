/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2006 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  HeciHpet.c

Abstract:

  Definitions for HECI driver

--*/
#ifdef ECP_FLAG
#include "Hecidrv.h"
#include "HeciRegs.h"
#include "Hecicore.h"
#endif
#include "HeciHpet.h"

//
// Extern for shared HECI data and protocols
//
extern HECI_INSTANCE  *mHeciContext;
volatile UINT32       mSaveHpetConfigReg;

VOID
SaveHpet (
  VOID
)
/*++

Routine Description:

  Store the value of High Performance Timer

Arguments:

  None

Returns:

  None

--*/
{
//  mSaveHpetConfigReg = MmioRead32 (PCH_RCRB_BASE + R_PCH_RCRB_HPTC);
}

VOID
RestoreHpet (
  VOID
)
/*++

Routine Description:

  Restore the value of High Performance Timer

Arguments:

  None

Returns:

  None

--*/
{
//  MmioWrite32 (PCH_RCRB_BASE + R_PCH_RCRB_HPTC, mSaveHpetConfigReg);
}

VOID
StartTimer (
  OUT UINT32 *Start,
  OUT UINT32 *End,
  IN  UINT32 Time
)
/*++

  Routine Description:

    Used for calculating timeouts

  Arguments:

    Start - Snapshot of the HPET timer
    End   - Calculated time when timeout period will be done
    Time  - Timeout period in microseconds

  Returns:

    VOID

--*/
{
  UINT32  Ticks;

  //
  // Make sure that HPET is enabled and running
  //
  EnableHpet ();

  //
  // Read current timer value into start time from HPET
  //
  *Start = mHeciContext->HpetTimer[HPET_MAIN_COUNTER_LOW];

  //
  // Convert microseconds into 70ns timer ticks
  //
  Ticks = Time * HPET_TICKS_PER_MICRO;

  //
  // Compute end time
  //
  *End = *Start + Ticks;

  return ;
}

EFI_STATUS
Timeout (
  IN  UINT32 Start,
  IN  UINT32 End
)
/*++

  Routine Description:
    Used to determine if a timeout has occured.

  Arguments:
    Start - Snapshot of the HPET timer when the timeout period started.
    End   - Calculated time when timeout period will be done.

  Returns:
    EFI_STATUS

--*/
{
  UINT32  Current;

  //
  // Read HPET and assign the value as the current time.
  //
  Current = mHeciContext->HpetTimer[HPET_MAIN_COUNTER_LOW];

  //
  // Test basic case (no overflow)
  //
  if ((Start < End) && (End <= Current)) {
    return EFI_TIMEOUT;
  }
  //
  // Test basic start/end conditions with overflowed timer
  //
  if ((Start < End) && (Current < Start)) {
    return EFI_TIMEOUT;
  }
  //
  // Test for overflowed start/end condition
  //
  if ((Start > End) && ((Current < Start) && (Current > End))) {
    return EFI_TIMEOUT;
  }
  //
  // Catch corner case of broken arguments
  //
  if (Start == End) {
    return EFI_TIMEOUT;
  }


  DEBUG ((EFI_D_ERROR, "crnt %X start %X end %X\n", Current, Start, End));
  //
  // Else, we have not yet timed out
  //
  return EFI_SUCCESS;
}

VOID
IoDelay (
  UINT32 delayTime
)
/*++

Routine Description:

  Delay for at least the request number of microseconds

Arguments:

  delayTime - Number of microseconds to delay.

Returns:

  None.

--*/
{
  gBS->Stall (delayTime);
}
