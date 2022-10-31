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

  ValleyviewPpmLib.c

Abstract:

  This library contains power management configuration functions for
  ValleyView processors.

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
#include "ValleyviewPpmLib.h"
#ifndef ECP_FLAG
#include "Library/S3BootScriptLib.h"
#include "Library/BaseLib.h"
#include "Library/DebugLib.h"
#include "Protocol/BootScriptSave.h"
#endif

//
// Global variables
//
//
// BootScriptSave protocol for saving configuration changes
//
extern EFI_BOOT_SCRIPT_SAVE_PROTOCOL *mBootScriptSave;

//
// VLV iFSB Frequency Table.
//
UINT16 miFSBFrequencyTable[4] = {
  83,           // 83.3MHz
  100,          // 100MHz
  133,          // 133MHz
  117           // 116.7MHz
};

/* @NOTE: CDV reference.
UINT16 miFSBFrequencyTable[8] = {
  0,            // Reserved
  133,          // 133MHz OPT_133 SKU
  0,            // Reserved
  0,            // Reserved
  0,            // Reserved
  100,          // 100MHz OPT_100 SKU
  0,            // Reserved
  0             // Reserved
};
*/


EFI_STATUS
InitializeMchLib (
  VOID
  )
/*++

Routine Description:

  Initialize the MCH library

Arguments:

  None

Returns:

  None

--*/
{
  ASSERT (mBootScriptSave != NULL);

  return EFI_SUCCESS;
}

UINT16
DetermineiFsbFromMsr (
  VOID
  )
/*++

Routine Description:

  Determine the processor core frequency

Arguments:

  None

Returns:

  Processor core frequency multiplied by 3


--*/
{

  // Determine the processor core frequency
  //
  UINT64    Temp;
  Temp = (AsmReadMsr64 (BSEL_CR_OVERCLOCK_CONTROL)) & FUSE_BSEL_MASK;
  return miFSBFrequencyTable[(UINT32)(Temp)];

}

BOOLEAN
MchSupportDynamicFsbFrequencySwitching (
  VOID
  )
/*++

Routine Description:

  Determines if MCH is capable of dynamic FSB frequency switching(Bus Geyserville)

Arguments:

  None

Returns:

  FALSE         Dynamic FSB frequency switching(Bus Geyserville) is NOT supported.
  TRUE          Dynamic FSB frequency switching(Bus Geyserville) is supported.

--*/
{
  return FALSE;
}

EFI_STATUS
EnableMchDynamicFsbFrequencySwitching (
  VOID
  )
/*++

Routine Description:

  Enables dynamic FSB frequency switching(Bus Geyserville) on ICH

Arguments:

  None

Returns:

  EFI_SUCCESS   Dynamic FSB frequency switching(Bus Geyserville) enabled

--*/
{
  return EFI_UNSUPPORTED;
}

