/*++

Copyright (c)  1999 - 2004 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  SmmIo.c

Abstract:

  SMM I/O access utility implementation file, for Ia32

--*/
/*++
 This file contains an 'Intel Peripheral Driver' and is
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may
 be modified by the user, subject to additional terms of the
 license agreement
--*/


//
// Include files
//
#ifdef ECP_FLAG
#include "SmmIoLib.h"
#else
#include <Library/SmmIoLib.h>
#include <Library/IoLib.h>
#endif
#include "PchAccess.h"
#ifdef ECP_FLAG
#include "EfiCommon.h"
#include "EfiApi.h"
#include EFI_PROTOCOL_DEFINITION (SmmBase)
#include "EdkIIGlueDxeSmmDriverEntryPoint.h"
#else
#include <Protocol/SmmBase.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#endif


UINT8
SmmIoRead8 (
  IN  UINT16    Address
  )
/*++

Routine Description:

  Do a one byte IO read

Arguments:

  Address - IO address to read

Returns:

  Data read

--*/
{
  UINT8   Buffer;

  ASSERT (gSmst);

  gSmst->SmmIo.Io.Read (
                  &gSmst->SmmIo, 
                  SMM_IO_UINT8, 
                  Address, 
                  1, 
                  &Buffer
                  );
  return Buffer;
}

VOID
SmmIoWrite8 (
  IN  UINT16    Address,
  IN  UINT8     Data
  )
/*++

Routine Description:

  Do a one byte IO write

Arguments:

  Address - IO address to write
  Data    - Data to write

Returns:

  None.

--*/
{
  ASSERT (gSmst);

  gSmst->SmmIo.Io.Write (
                    &gSmst->SmmIo, 
                    SMM_IO_UINT8, 
                    Address, 
                    1, 
                    &Data
                    );
}

UINT16
SmmIoRead16 (
  IN  UINT16    Address
  )
/*++

Routine Description:

  Do a two byte IO read

Arguments:

  Address - IO address to read

Returns:

  Data read

--*/
{
  UINT16      Buffer;

  ASSERT (gSmst);

  gSmst->SmmIo.Io.Read (
                  &gSmst->SmmIo, 
                  SMM_IO_UINT16, 
                  Address,
                  1, 
                  &Buffer 
                  );
  return Buffer;
}

VOID
SmmIoWrite16 (
  IN  UINT16    Address,
  IN  UINT16    Data
  )
/*++

Routine Description:

  Do a two byte IO write

Arguments:

  Address - IO address to write
  Data    - Data to write

Returns:

  None.

--*/
{
  ASSERT (gSmst);

  gSmst->SmmIo.Io.Write ( 
                    &gSmst->SmmIo, 
                    SMM_IO_UINT16, 
                    Address,
                    1, 
                    &Data 
                    );
}

UINT32
SmmIoRead32 (
  IN  UINT16    Address
  )
/*++

Routine Description:

  Do a four byte IO read

Arguments:

  Address - IO address to read

Returns:

  Data read

--*/
{
  UINT32        Buffer;

  ASSERT (gSmst);

  gSmst->SmmIo.Io.Read ( 
                    &gSmst->SmmIo, 
                    SMM_IO_UINT32, 
                    Address,
                    1, 
                    &Buffer 
                    );
  return Buffer;
}

VOID
SmmIoWrite32 (
  IN  UINT16    Address,
  IN  UINT32    Data
  )
/*++

Routine Description:

  Do a four byte IO write

Arguments:

  Address - IO address to write
  Data    - Data to write

Returns:

  None.

--*/
{
  ASSERT (gSmst);

  gSmst->SmmIo.Io.Write ( 
                    &gSmst->SmmIo, 
                    SMM_IO_UINT32, 
                    Address,
                    1, 
                    &Data 
                    );
}

VOID
SmmStall (
  IN  UINTN   Microseconds
  )
/*++

Routine Description:

  Delay for at least the request number of microseconds.
  Timer used is ACPI time counter, which has 1us granularity.

Arguments:

  Microseconds - Number of microseconds to delay.

Returns:

  None

--*/
{
  UINTN   Ticks;
  UINTN   Counts;
  UINTN   CurrentTick;
  UINTN   OriginalTick;
  UINTN   RemainingTick;
  UINT16  mAcpiBaseAddr;

  if (Microseconds == 0) {
    return;
  }

  mAcpiBaseAddr = PchLpcPciCfg16 (R_PCH_LPC_ACPI_BASE) & B_PCH_LPC_ACPI_BASE_BAR;

  OriginalTick = SmmIoRead32 (mAcpiBaseAddr + R_PCH_ACPI_PM1_TMR);
  CurrentTick = OriginalTick;

  //
  // The timer frequency is 3.579545 MHz, so 1 ms corresponds 3.58 clocks
  //
  Ticks = Microseconds * 358 / 100 + OriginalTick + 1;

  //
  // The loops needed by timer overflow
  //
  Counts = Ticks / ICH_ACPI_TIMER_MAX_VALUE;

  //
  // Remaining clocks within one loop
  //
  RemainingTick = Ticks % ICH_ACPI_TIMER_MAX_VALUE;

  //
  // not intend to use TMROF_STS bit of register PM1_STS, because this adds extra
  // one I/O operation, and maybe generate SMI
  //
  while ((Counts != 0) || (RemainingTick > CurrentTick)) {
    CurrentTick = SmmIoRead32 (mAcpiBaseAddr + R_PCH_ACPI_PM1_TMR);
    //
    // Check if timer overflow
    //
    if (CurrentTick < OriginalTick) {
      Counts--;
    }
    OriginalTick = CurrentTick;
  }
}
