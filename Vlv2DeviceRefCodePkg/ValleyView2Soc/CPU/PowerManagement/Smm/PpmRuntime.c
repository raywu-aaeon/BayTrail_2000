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

  PpmRuntime.c

Abstract:

  Platform power management runtime code.  This code implements the power management
  settings during runtime.  It must be runtime SMM safe.

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
#include <Ppm.h>
#include <PowerManagement.h>
#ifndef ECP_FLAG
#include <IchPpmLib.h>
#include <PchAccess.h>
#include <CpuPpmLib.h>
#include <CpuRegs.h>
#include <Library/UefiRuntimeLib.h>

#include <Library/SmmServicesTableLib.h>
#include <Protocol/GlobalNvsArea.h>
#endif

//
// Global variables
//
//

//
// PPM Processor support protocol
//
extern PPM_PROCESSOR_SUPPORT_PROTOCOL_2     *mPpmProcessorSupportProtocol;

//
// FVID Table Information (Must be initialized by caller)
//
extern FVID_TABLE                *mFvidPointer;

//
// Last requested GV state
//
extern UINT16                    mRequestedState;

//
// CST time storage areas
//
extern UINT8                  mPpmCstTmrFlags;
extern UINTN                  mPpmTscCorrFactor;
extern UINTN                  mPpmTscCorrFactorRem;
extern UINTN                  mPpmCstTscCorrRem;
extern UINT64                 mPpmCstTscTicks;

//
// Power management ACPI base address
//
extern UINT16                 AcpiBase;

//
// Global NVS area (communication buffer between SMM and ASL code)
// This area is special because it is in ACPI NVS memory and should
// not be relocated by the OS.  It is accessed in BS, SMM, and by ASL.
//
extern EFI_GLOBAL_NVS_AREA    *mGlobalNvsAreaPtr;

//
// Function Implementations
//

EFI_STATUS
InitializePpmRuntime (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Initialize any runtime required data for runtime PPM services.  This should be called prior to invoking runtime functions.

Arguments:

  This                Pointer to the protocol instance

Returns:

  EFI_SUCCESS   PPM Runtime initialized.

--*/
{
  //
  // AP service
  //
  ASSERT (gSmst != NULL);
  ASSERT (gSmst->SmmStartupThisAp);
  ASSERT (mPpmProcessorSupportProtocol != NULL);
  ASSERT (mGlobalNvsAreaPtr != NULL);
  ASSERT (AcpiBase);

  return EFI_SUCCESS;
}

VOID
ProcessorCmpCState (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Process a C state change request (SMI coordination method uses this).
    Determine best C state possible
    Save the PM timer and C3 timer entry times
    Transition to lowest GV state if soft C2 desired
    Update TSC MSR if C3 or C4

Arguments:

  This                Pointer to the protocol instance

Returns:

  None

--*/
{
  //EFI_STATUS    Status;
  //UINT8         BmSts;
  UINT8         MaxCState;
  UINTN         Index;
  MSR_REGISTER  SavedPerformanceState;
  UINTN         PState;
  UINTN         CstCalculation = 0;
  UINTN         Data32;
  UINT64        Data64;
  UINTN         PpmCstResEntryTime  = 0;

  SavedPerformanceState.Qword = 0;

  //
  // Determine if any C states are present that require coordination
  // If not, return
  //
  if (!(This->PpmFlags & (PPM_C2 | PPM_C3 | PPM_C4))) {
    return;
  }

  //
  // Determine bus master status
  //
  //Status = gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT8, AcpiBase, 1, &BmSts);
  //ASSERT_EFI_ERROR (Status);

  //
  // Determine the maximum C state we can transition to
  //
  MaxCState = CSTATE_TRANSITION_C4;

  //
  // Eliminate C4 if not supported or on AC power
  //
  if (!(This->PpmFlags & PPM_C4) || mGlobalNvsAreaPtr->PowerState) {
    MaxCState = CSTATE_TRANSITION_C3;
  }

  //
  // Eliminate C3&C4 if not C3&C4 supported or bus master status
  //
  if (!(This->PpmFlags & (PPM_C3 | PPM_C4))/* || (BmSts & PM_BM_STS)*/) {
    MaxCState = CSTATE_TRANSITION_C2;
  }

  //
  // We don't do C3, so do C2
  //
  if (MaxCState == CSTATE_TRANSITION_C3) {
    MaxCState--;
  }

  //
  // Save the C-state entry time from the C3 Residency Timer if required.  (The
  // residency timer is only required for for TSC updates, which are not
  // necessary for C2.)
  //
  //if (MaxCState > CSTATE_TRANSITION_C2) {
  //  Status = gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT32, AcpiBase + ICHX_C3_RES, 1, &PpmCstResEntryTime);
  //  ASSERT_EFI_ERROR (Status);
  //}

  //
  // Determine if SoftC2E
  //
  if (MaxCState == CSTATE_TRANSITION_C2 && (This->PpmFlags & PPM_GV3) && (This->PpmFlags & PPM_C2E)) {
    //
    // Save current P state
    //
    SavedPerformanceState.Qword = AsmReadMsr64 (MSR_IA32_PERF_CTL);

    //
    // Transition each core to lowest performance state
    //
    ASSERT (mFvidPointer != NULL);
    for (Index = 0; Index < gSmst->NumberOfCpus; Index++) {
      This->PStateTransition (This, mFvidPointer, Index, mFvidPointer->FvidHeader.Gv3States - 1);
    }
    //
    // Wait for voltage transitions to complete on all threads
    //
    WaitForPpmTransitionComplete (NULL);
    for (Index = 1; Index < gSmst->NumberOfCpus; Index++) {
      gSmst->SmmStartupThisAp (WaitForPpmTransitionComplete, Index, NULL);
    }
  }

  //
  // Perform C state change
  //
  PpmCStateTransition (MaxCState);

  //
  // Determine if SoftC2E and restore P state
  //
  if (MaxCState == CSTATE_TRANSITION_C2 && (This->PpmFlags & PPM_GV3) && (This->PpmFlags & PPM_C2E)) {
    //
    // Determine what the state was based on saved IA32_PERF_CTL MSR data
    //
    PState = (UINT8) mFvidPointer->FvidHeader.Gv3States - 1;
    for (Index = mFvidPointer->FvidHeader.Gv3States; Index > 0; Index--) {
      if (mFvidPointer[Index].FvidState.BusRatio == (SavedPerformanceState.Bytes.SecondByte & RATIO_BYTE_MASK )) {

        PState = Index - 1;
        //
        // Our VID should always match, since there should only be one VID per Bus ratio
        //
        ASSERT (mFvidPointer[Index].FvidState.Vid == (SavedPerformanceState.Bytes.FirstByte & VID_BYTE_MASK));
        break;
      }
    }

    //
    // Transition each core to original performance state
    //
    for (Index = 0; Index < gSmst->NumberOfCpus; Index++) {
      This->PStateTransition (This, mFvidPointer, Index, PState);
    }

    //
    // Wait for voltage transitions to complete on all threads
    //
    WaitForPpmTransitionComplete (NULL);
    for (Index = 1; Index < gSmst->NumberOfCpus; Index++) {
      gSmst->SmmStartupThisAp (WaitForPpmTransitionComplete, Index, NULL);
    }
  }

  //
  // Calculate C3 or C4 duration for CPU Time Stamp Counter update
  //
  if ((MaxCState > CSTATE_TRANSITION_C2) && (mPpmCstTmrFlags & CST_UPDT_TSC)) {

    //
    // Find the C3/C4 time using the C3 Residency Timer for TSC corrections.
    //
    //Status = gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT32, AcpiBase + ICHX_C3_RES, 1, &CstCalculation);
    //ASSERT_EFI_ERROR (Status);

    CstCalculation |= C3_RESIDENCY_TIMER_ROLLOVER_BIT;
    CstCalculation -= PpmCstResEntryTime;
    CstCalculation &= ~C3_RESIDENCY_TIMER_ROLLOVER_BIT;

    //
    // Convert the C3 Residency Timer delta to TSC ticks for TSC updates
    //

    //
    // Correct for differences in frequencies bewteen residency timer and TSC
    //
    mPpmCstTscTicks = MultU64x32 ((UINT64) mPpmTscCorrFactor, (UINT32)CstCalculation);
    //
    // Correct for frequency difference with correction factor remainder
    //
    Data64 = MultU64x32 ((UINT64) mPpmTscCorrFactorRem, (UINT32)CstCalculation);
    mPpmCstTscTicks += DivU64x32Remainder (Data64, CST_DATA_TBASE, (UINT32 *)&Data32);
    //
    //  Update PM Timer remainder
    //
    mPpmCstTscCorrRem += Data32;
    //
    // Check remainder rollover
    //
    if (mPpmCstTscCorrRem >= CST_DATA_TBASE) {
      mPpmCstTscCorrRem -= CST_DATA_TBASE;
      mPpmCstTscTicks++;
    }
    //
    // Update Time Stamp Counter MSR for all CPUs
    //
    PpmUpdateTimeStampCounter (NULL);
    for (Index = 1; Index < gSmst->NumberOfCpus; Index++) {
      gSmst->SmmStartupThisAp (PpmUpdateTimeStampCounter, Index, NULL);
    }
  }
}

EFI_STATUS
PpmCStateTransition (
  UINT8         CState
  )
/*++

Routine Description:

  Perform a C state transition

Arguments:

  CState    C state desired (1-based)

Returns:

  EFI_SUCCESS             The function completed successfully
  EFI_INVALID_PARAMETER   Unsupported C state requested

--*/
{
  EFI_STATUS      Status;
  UINT16          Data16;
  UINT8           Data8;
  UINT8           ArbiterState;
  UINT32          Pm2CntOffset;
  //
  // Check for valid request (C2 to C4)
  //
  ASSERT (CState <= CSTATE_TRANSITION_C4 && CState >= CSTATE_TRANSITION_C2);

  //
  // Clear bus master status for all states
  //
  //Data16 = PM_BM_STS;
  //Status = gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT16, AcpiBase, 1, &Data16);
  //ASSERT_EFI_ERROR (Status);

  //
  // Get the PM2_CNT register offset
  //
  Pm2CntOffset = GetPm2CntOffset ();

  switch (CState) {

    case CSTATE_TRANSITION_C2:
      //
      // Disable bus master reloading by clearing bit 1 of PM1_CNT
      //
      Status = gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT16, AcpiBase + PM_PM1_CNT, 1, &Data16);
      ASSERT_EFI_ERROR (Status);
      Data16 &= ~PM_BM_RELOAD;
      Status = gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT16, AcpiBase + PM_PM1_CNT, 1, &Data16);
      ASSERT_EFI_ERROR (Status);
      break;

    case CSTATE_TRANSITION_C3:
    case CSTATE_TRANSITION_C4:
      //
      // Enable bus master reloading by setting bit 1 of PM1_CNT
      //
      Status = gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT16, AcpiBase + PM_PM1_CNT, 1, &Data16);
      ASSERT_EFI_ERROR (Status);
      Data16 |= PM_BM_RELOAD;
      Status = gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT16, AcpiBase + PM_PM1_CNT, 1, &Data16);
      ASSERT_EFI_ERROR (Status);

      //
      // Disable (save first) arbiter state
      //
      Status = gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT8, AcpiBase + Pm2CntOffset, 1, &ArbiterState);
      ASSERT_EFI_ERROR (Status);
      Data8 = ArbiterState | B_PCH_ACPI_PM2_CNT_ARB_DIS;
      Status = gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, AcpiBase + Pm2CntOffset, 1, &Data8);
      ASSERT_EFI_ERROR (Status);
      break;

    default:
      //
      // Should not be able to get here ever
      //
      ASSERT (FALSE);
      break;
  }

  //
  // Do C state transition by reading appropriate LVx register
  //
  //Status = gSmst->SmmIo.Io.Read (&gSmst->SmmIo, SMM_IO_UINT8, AcpiBase + (PM_CST_LVL2 - 2) + CState, 1, &Data8);
  //ASSERT_EFI_ERROR (Status);

  //
  // Restore Arbiter State if modified previously
  //
  if (CState > CSTATE_TRANSITION_C2) {
    Status = gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT8, AcpiBase + Pm2CntOffset, 1, &ArbiterState);
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Clear bus master status
  //
  //Data16 = PM_BM_STS;
  //Status = gSmst->SmmIo.Io.Write (&gSmst->SmmIo, SMM_IO_UINT16, AcpiBase + PM_PM1_STS, 1, &Data16);
  //ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

VOID
SetPerfControl (
  IN  VOID        *Buffer
  )
/*++

Routine Description:

  AP safe function to make a p-state change request.

Arguments:

  Buffer      Value to write to the performance control MSR

Returns:

  EFI_SUCCESS             MSR updated successfully

--*/
{
  UINT64  PerformanceControl;

  PerformanceControl = AsmReadMsr64 (MSR_IA32_PERF_CTL);

  //
  // Clear and then initialize fields: BUS_RATIO_SEL[12:8], VID_SEL[6:0]
  //
  PerformanceControl &= ~(PSTATE_CTL_MASK);
  PerformanceControl |= ((*(UINT64*) Buffer) & PSTATE_CTL_MASK);

  AsmWriteMsr64 (MSR_IA32_PERF_CTL, PerformanceControl);

  return;
}

EFI_STATUS
PpmTransition (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN     FVID_TABLE                         *FvidPointer,
  IN     UINTN                              CpuNumber,
  IN     UINTN                              NextState
  )
/*++

Routine Description:

  Transition a single processor thread to a P state.

Arguments:

  This          Unused
  FvidPointer   Pointer to a table to be updated
  CpuNumber     The processor thread to perform the transition on
  NextState     The desired state (zero based) from the provided FVID table

Returns:

  EFI_SUCCESS             State transition success.
  EFI_INVALID_PARAMETER   Requested state is not supported.

--*/
{
  UINT64    PerformanceControl;

  //
  // Make sure we have a supported state
  //
  if (NextState >= FvidPointer[0].FvidHeader.Gv3States) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Save the state in the current requested state global
  //
  mRequestedState = (UINT16) NextState;

  //
  // GV3 had better be enabled if a transition is attempted
  //
  ASSERT (This->PpmFlags & PPM_GV3);

  //
  // First, convert the requested point number into a BR/VID combo by looking up
  // the value in the FVID table.
  //

  //
  // Second, write the BR/VID combo to the MSR to cause the transition.
  //
  PerformanceControl = FvidPointer[NextState + 1].FvidState.BusRatio;
  PerformanceControl = LShiftU64 (PerformanceControl, 8);
  PerformanceControl |= FvidPointer[NextState + 1].FvidState.Vid;

  if (CpuNumber == 0) {
    SetPerfControl (&PerformanceControl);
  } else {
    gSmst->SmmStartupThisAp (SetPerfControl, CpuNumber, &PerformanceControl);
  }

  return EFI_SUCCESS;
}

VOID
WaitForPpmTransitionComplete (
  IN  VOID        *Buffer
  )
/*++

Routine Description:

  This function will poll for the completion of a voltage transition for a core.
  This function must be MP safe.

Arguments:

  CpuNumber     The processor thread to poll

Returns:

  EFI_SUCCESS             State transition completed.

--*/
{
  MSR_REGISTER    PerformanceStatusMsr;

  //
  // Poll voltage transition status until complete
  //
  do {
    PerformanceStatusMsr.Qword = AsmReadMsr64 (MSR_IA32_PERF_STS);
  } while (PerformanceStatusMsr.Qword & IA32_PERF_STS_VIP);

  return;
}

VOID
PpmUpdateTimeStampCounter (
  IN  VOID        *Buffer
  )
/*++

Routine Description:

  This function will update the Time Stamp Counter MSR with mPpmCstTscTicks, which
  contains the timer tick offset for timer ticks during C-states.
  This function must be MP safe.

Arguments:

  Buffer              Unused

Returns:

  EFI_SUCCESS             TSC updated

--*/
{
  MSR_REGISTER    TimeStampCounter;

  TimeStampCounter.Qword = AsmReadMsr64 (EFI_MSR_TIME_STAMP_COUNTER);
  TimeStampCounter.Qword += mPpmCstTscTicks;
  AsmWriteMsr64 (EFI_MSR_TIME_STAMP_COUNTER, TimeStampCounter.Qword);

  return;
}
