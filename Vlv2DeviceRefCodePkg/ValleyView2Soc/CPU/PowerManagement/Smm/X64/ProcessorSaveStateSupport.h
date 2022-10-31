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

  ProcessorSaveStateSupport.h

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

#ifndef _PROCESSOR_SAVE_STATE_SUPPORT_H_
#define _PROCESSOR_SAVE_STATE_SUPPORT_H_

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include EFI_PROTOCOL_DEPENDENCY (SmmBase)
#include EFI_PROTOCOL_DEFINITION (SmmCpuState)

extern EFI_SMM_SYSTEM_TABLE *gSmst;
#else
#include "PiDxe.h"
#include <Protocol/SmmCpuSaveState.h>
#endif
extern EFI_SMM_CPU_STATE  **mSmmCpuSavedState;

//
// Macros - mSmmCpuSavedState must be initialized before using
//
//
// Obtain the SMBASE pointer (UINT8*) for the given processor.
//
#define GET_SMBASE_POINTER(CpuNumber) ((UINT8 *) (UINTN) mSmmCpuSavedState[CpuNumber]->x64.SMBASE)

//
// Obtain the IO MISC INFO contents (UINT32) of the SMM save state for the processor, given the SMBASE pointer as input.
//
#define GET_IO_MISC_INFO(SmbasePointer) (*(UINT32 *) (((UINT8 *) SmbasePointer) + 0x8000 + 0x7FA4))

//
// Obtain the autohalt setting for a given processor.
//
#define GET_AUTOHALT(CpuNumber) (mSmmCpuSavedState[CpuNumber]->x64.AutoHALTRestart & 0x01)

//
// Function prototypes
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
;
#endif
