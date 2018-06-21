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

  SilvermontPpmLib.h

Abstract:

  This library contains power management configuration functions specific to
  Valleyview processors.

  Acronyms:
    PPM   Processor Power Management
    GV    Geyserville
    TM    Thermal Monitor
    IST   Intel(R) Speedstep technology
    HT    Hyper-Threading Technology
    CMP   Core Multi-Processing

--*/

#ifndef _SILVERMONT_PPM_LIB_H_
#define _SILVERMONT_PPM_LIB_H_

//
// Include files
//
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include EFI_PROTOCOL_CONSUMER (PpmProcessorSupport2)
#endif
#include <SilvermontPpmDefines.h>
#ifndef ECP_FLAG
#include <PiDxe.h>
#endif
#include <PpmCommon.h>

#ifndef ECP_FLAG
#include <Protocol/PpmPlatformPolicy.h>
#include <Protocol/PpmProcessorSupport2.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#endif

typedef struct {
  UINT32  RegEax;
  UINT32  RegEbx;
  UINT32  RegEcx;
  UINT32  RegEdx;
} EFI_CPUID_REGISTER;
//
// Stall period in microseconds
//
#define PPM_WAIT_PERIOD  15

//
// Structure Declarations
//
typedef struct _ENABLE_CSTATE_PARAMS {
  PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This;
  UINT16                             C2IoAddress;
  UINT16                             CsmIoAddress;
} ENABLE_CSTATE_PARAMS;

typedef struct _ENABLE_EMTTM_PARAMS {
  PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This;
  FVID_TABLE                         *FvidPointer;
} ENABLE_EMTTM_PARAMS;
//
// Function prototypes
//
/*++

Routine Description:

  Initializes the platform power management library.  This must be called
  prior to any of the library functions being used.

  At this time, we don't properly publish the PPM processor support protocol,
  we simply return it if this library implements the protocol.

  If the processor is not supported, the input will not be modified.

Arguments:

  This          Pointer to the PPM support protocol instance

Returns:

  EFI_SUCCESS         Library was initialized successfully
  EFI_NOT_SUPPORTED   The library does not support the current processor

--*/
EFI_STATUS
InitializeValleyviewPowerManagementLib (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   **This
  );

STATIC VOID ConfigureTurboPowerLimitsValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT PPM_PLATFORM_POLICY_PROTOCOL       *PPMPolicy
  );
/*++

Routine Description:

  - Configures following fields of MSR 0x610 based on user configuration:
     - Configures Long duration Turbo Mode (power limit 1) power level and time window
     - Configures Short duration turbo mode (power limit 2)

Arguments:

  This          Pointer to the protocol instance
  PPMPolicy     Pointer to policy protocol instance

Returns:

  None

--*/
;


STATIC
EFI_STATUS
SetPpmFlagsValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  );

STATIC
EFI_STATUS
EnableCStatesValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN     UINT16                             C2IoAddress,
  IN     UINT16                             CsmIoAddress
  );


VOID
ApSafeEnableCStates (
  IN OUT VOID                               *Buffer
  );

STATIC
EFI_STATUS
InitThermalValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT PPM_PLATFORM_POLICY_PROTOCOL       *PPMPolicy
  );

VOID
ApSafeInitThermal (
  IN OUT VOID                               *Buffer
  );

STATIC
EFI_STATUS
EnableTmValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  );

VOID
ApSafeEnableTm (
  IN OUT VOID                               *Buffer
  );
/*
STATIC
EFI_STATUS
EnableTm2Valleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT FVID_TABLE     *FvidPointer
  );
*/

/*
STATIC
EFI_STATUS
ApSafeEnableTm2 (
  IN OUT VOID                               *Buffer
  );
*/

STATIC
EFI_STATUS
EnableProcHotValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  );

/*
STATIC
EFI_STATUS
ApSafeEnableProcHot (
  IN OUT VOID                               *Buffer
  );
*/

STATIC
EFI_STATUS
EnableTscValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT FVID_TABLE                         *FvidPointer,
  IN     UINT16                             iFSBFrequency,
  IN OUT UINT8                              *PpmCstTmrFlags,
  IN OUT UINTN                              *PpmTscCorrFactor,
  IN OUT UINTN                              *PpmTscCorrFactorRem
  );

STATIC
EFI_STATUS
S3SaveMsrValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  );

STATIC
EFI_STATUS
S3RestoreMsrValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  );

VOID
ApSafeRestoreMsr (
  IN OUT VOID                               *Buffer
  );

STATIC
EFI_STATUS
EnableCStateIoRedirectionValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  );

VOID
ApSafeEnableCStateIoRedirection (
  IN OUT VOID                               *Buffer
  );

STATIC
EFI_STATUS
DisableCStateIoRedirectionValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  );

VOID
ApSafeDisableCStateIoRedirection (
  IN OUT VOID                               *Buffer
  );

/* @NOTE: Not applicable due to CSM_TRIG removed in SLM.
STATIC
EFI_STATUS
EnableCStateSmiCoordinationValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  );

STATIC
EFI_STATUS
ApSafeEnableCStateSmiCoordination (
  IN OUT VOID                               *Buffer
  );

STATIC
EFI_STATUS
DisableCStateSmiCoordinationValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  );

STATIC
EFI_STATUS
ApSafeDisableCStateSmiCoordination (
  IN OUT VOID                               *Buffer
  );
*/

STATIC
EFI_STATUS
EnablePStateHardwareCoordinationValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  );

VOID
ApSafeEnablePStateHardwareCoordination (
  IN OUT VOID                               *Buffer
  );

STATIC
EFI_STATUS
DisablePStateHardwareCoordinationValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  );

VOID
ApSafeDisablePStateHardwareCoordination (
  IN OUT VOID                               *Buffer
  );

STATIC
EFI_STATUS
InitFvidTableValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT FVID_TABLE                         *FvidPointer,
  IN     UINT16                             MaxNumberOfStates,
  IN     UINT16                             MinStepSize,
  IN     BOOLEAN                            CreateDefaultTable
  );

STATIC
VOID
ApSafeEnableTurboMode (
  IN OUT VOID                               *Buffer
  );

STATIC
EFI_STATUS
CreateFvidTable (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT FVID_TABLE                         *FvidPointer,
  IN     UINT16                             MaxNumberOfStates,
  IN     UINT16                             MinStepSize
  );

STATIC
VOID
CreateDefaultFvidTable (
  IN OUT FVID_TABLE                         *FvidPointer
  );

STATIC
EFI_STATUS
InitGv3Valleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT FVID_TABLE                         *FvidPointer,
  IN OUT PPM_PLATFORM_POLICY_PROTOCOL       *PPMPolicy
  );

VOID
ApSafeEnableGv3 (
  IN OUT VOID                               *Buffer
  );

STATIC
EFI_STATUS
RunOnAllLogicalProcessors (
  IN OUT EFI_AP_PROCEDURE   Procedure,
  IN OUT VOID               *Buffer
  );
/*++

Routine Description:

  Runs the specified procedure on all logical processors, passing in the
  parameter buffer to the procedure.

Arguments:

  Procedure     The function to be run.
  Buffer        Pointer to a parameter buffer.

Returns:

  EFI_SUCCESS

--*/
STATIC
EFI_STATUS
SetBootPState (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Set processor P state to HFM or LFM

Arguments:

  This          Pointer to the protocol instance

Returns:
  EFI_UNSUPPORTED EIST not supported.
  EFI_SUCCESS     Processor P state has been set.

--*/
;

VOID
ApSafeSetBootPState (
  IN OUT VOID        *Buffer
  )
/*++

Routine Description:

  Set processor P state to HFM or LFM.

Arguments:

  Buffer          Unused

Returns:

  EFI_SUCCESS   Processor MSR setting is saved.

--*/
;
#endif
