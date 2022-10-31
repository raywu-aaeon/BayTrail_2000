/*++

Copyright (c) 1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  MpService.c

Abstract:

  Code which support multi-processor


--*/

#include "CpuDxe.h"
#include "PlatformMpService.h"
#include "MchkInit.h"
#include "MiscFuncs.h"
#include "Thermal.h"
#include "Features.h"

#define MODULE_1_CPU0_APICID 4
VOID
EFIAPI
InitSmramDataContent (
  IN EFI_EVENT     Event,
  IN VOID          *Context
  );

extern EFI_PLATFORM_CPU_PROTOCOL     *mPlatformCpu;
extern EFI_METRONOME_ARCH_PROTOCOL   *mMetronome;
extern MP_SYSTEM_DATA                *mMPSystemData;
extern UINTN                         mCommonFeatures;
extern volatile UINTN                mSwitchToLegacyRegionCount;
extern UINT32                        mCommonCStateValue;
extern EFI_PLATFORM_CPU_INFO         mPlatformCpuInfo;

static EFI_HANDLE                    mHandle         = NULL;
static volatile UINTN                mFinishedCount  = 0;


EFI_STATUS
R9WhoAmI (
  IN  EFI_MP_SERVICES_PROTOCOL            *This,
  OUT UINTN                               *CpuNumber
  );

EFI_STATUS
R9EnableDisableAP (
  IN  EFI_MP_SERVICES_PROTOCOL            * This,
  IN  UINTN                               CpuNumber,
  IN  BOOLEAN                             NewAPState,
  IN  UINT32                              *HealthFlag OPTIONAL
  );

EFI_STATUS
R9SwitchBSP (
  IN  EFI_MP_SERVICES_PROTOCOL            *This,
  IN  UINTN                               CpuNumber,
  IN  BOOLEAN                             EnableOldBSP
  );

EFI_STATUS
R9StartupThisAP (
  IN      EFI_MP_SERVICES_PROTOCOL        *This,
  IN      EFI_AP_PROCEDURE                Procedure,
  IN      UINTN                           CpuNumber,
  IN      EFI_EVENT                       WaitEvent OPTIONAL,
  IN      UINTN                           TimeoutInMicroSecs OPTIONAL,
  IN OUT  VOID                            *ProcArguments OPTIONAL,
  OUT     BOOLEAN                         *Finished      OPTIONAL
  );

EFI_STATUS
R9StartupAllAPs (
  IN      EFI_MP_SERVICES_PROTOCOL        *This,
  IN      EFI_AP_PROCEDURE                Procedure,
  IN      BOOLEAN                         SingleThread,
  IN      EFI_EVENT                       WaitEvent OPTIONAL,
  IN      UINTN                           TimeoutInMicroSecs OPTIONAL,
  IN OUT  VOID                            *ProcArguments OPTIONAL,
  OUT     UINTN                           **FailedCPUList OPTIONAL
  );

EFI_STATUS
R9GetNumberOfProcessors (
  IN  EFI_MP_SERVICES_PROTOCOL            *This,
  OUT UINTN                               *NumberOfProcessors,
  OUT UINTN                               *NumberOfEnabledProcessors
  );

EFI_STATUS
R9GetProcessorInfo(
  IN  EFI_MP_SERVICES_PROTOCOL            *This,
  IN  UINTN                               CpuNumber,
  OUT EFI_PROCESSOR_INFORMATION           *ProcessorInfoBuffer
  );

static EFI_MP_SERVICES_PROTOCOL     mMpService = {
  R9GetNumberOfProcessors,
  R9GetProcessorInfo,
  R9StartupAllAPs,
  R9StartupThisAP,
  R9SwitchBSP,
  R9EnableDisableAP,
  R9WhoAmI
};

//static EFI_MP_SERVICES_PROTOCOL     mMpService = {
//  GetGeneralMPInfo,
//  GetProcessorContext,
//  StartupAllAPs,
//  StartupThisAP,
//  SwitchBSP,
//  SendIPI,
//  EnableDisableAP,
//  WhoAmI
//};

EFI_PHYSICAL_ADDRESS        mOriginalBuffer;
EFI_PHYSICAL_ADDRESS        mBackupBuffer;
VOID
InstallR9MPService();

VOID
EFIAPI
MpServiceInitialize (
  VOID
  )
/*++

Routine Description:

  Initialize the state information for the MP DXE Protocol.

Arguments:

  Event   - Event whose notification function is being invoked.
  Context - Pointer to the notification functions context, which is implementation dependent.

Returns:

  None.

--*/
{
  EFI_STATUS  Status;
  EFI_EVENT   LegacyBootEvent;
  EFI_EVENT   ExitBootServicesEvent;
  EFI_EVENT   ReAllocLegacyEvent;
  EFI_EVENT   ReAllocExitPmAuthEvent;
  VOID        *RegistrationLegacy;
  VOID        *RegistrationExitPmAuth;

  LegacyBootEvent        = NULL;
  ExitBootServicesEvent  = NULL;
  ReAllocLegacyEvent     = NULL;
  ReAllocExitPmAuthEvent = NULL;
  RegistrationLegacy     = NULL;
  RegistrationExitPmAuth = NULL;
  Status = gBS->LocateProtocol (
                  &gEfiGenericMemTestProtocolGuid,
                  NULL,
                  (VOID **) &mGenMemoryTest
                  );
  if (EFI_ERROR (Status)) {
    return ;
  }

  //
  // Save Mtrr Registers in global data areas
  //
  ReadMtrrRegisters ();

  Status  = InitializeMpSystemData ();
  if (EFI_ERROR (Status)) {
    goto Done;
  }
#if defined NOCS_S3_SUPPORT
//
//On BB platform, gExitPmAuthProtocolGuid isn't installed and gEfiLegacyBiosProtocolGuid is installed.
//According to previous code, it is ok not to reallocate memory for AP.
//Reallocating is ok for BB IA32 but halt for BB64. need final solution.
//Suppoes the fix for AV legacy boot also fix this issue, need verification.
//
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  ReAllocateMemoryForAP,
                  NULL,
                  &ReAllocExitPmAuthEvent
                  );
  ASSERT_EFI_ERROR (Status);
  Status = gBS->RegisterProtocolNotify (
                  &gExitPmAuthProtocolGuid,
                  ReAllocExitPmAuthEvent,
                  &RegistrationExitPmAuth
                  );

#else
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  ReAllocateMemoryForAP,
                  NULL,
                  &ReAllocExitPmAuthEvent
                  );
  ASSERT_EFI_ERROR (Status);
  Status = gBS->RegisterProtocolNotify (
                  &gExitPmAuthProtocolGuid,
                  ReAllocExitPmAuthEvent,
                  &RegistrationExitPmAuth
                  );
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  ReAllocateMemoryForAP,
                  NULL,
                  &ReAllocLegacyEvent
                  );
  ASSERT_EFI_ERROR (Status);
  Status = gBS->RegisterProtocolNotify (
                  &gEfiLegacyBiosProtocolGuid,
                  ReAllocLegacyEvent,
                  &RegistrationLegacy
                  );
#endif
  Status = EfiCreateEventLegacyBootEx (
             TPL_CALLBACK,
             ResetAPs,
             NULL,
             &LegacyBootEvent
             );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  Status = gBS->CreateEvent (
                  EVT_SIGNAL_EXIT_BOOT_SERVICES,
                  TPL_CALLBACK,
                  ResetAPs,
                  NULL,
                  &ExitBootServicesEvent
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Now install the MP services protocol.
  //

  InstallR9MPService();
//  Status = gBS->InstallMultipleProtocolInterfaces (
//                  &mHandle,
//                  &gEfiMpServiceProtocolGuid,
//                  &mMpService,
//                  NULL
//                  );

  if (EFI_ERROR (Status)) {

Done:
    if (LegacyBootEvent != NULL) {
      gBS->CloseEvent (LegacyBootEvent);
    }
    if (ExitBootServicesEvent != NULL) {
      gBS->CloseEvent (ExitBootServicesEvent);
    }
#ifdef ECP_FLAG
    (gBS->FreePool) (mMPSystemData);
#else
    gBS->FreePool (mMPSystemData);
#endif
  }
}

EFI_STATUS
GetGeneralMPInfo (
  IN  VOID                         *This,
  OUT UINTN                        *NumberOfCPUs,
  OUT UINTN                        *MaximumNumberOfCPUs,
  OUT UINTN                        *NumberOfEnabledCPUs,
  OUT UINTN                        *RendezvousIntNumber,
  OUT UINTN                        *RendezvousProcLength
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This                  - GC_TODO: add argument description
  NumberOfCPUs          - GC_TODO: add argument description
  MaximumNumberOfCPUs   - GC_TODO: add argument description
  NumberOfEnabledCPUs   - GC_TODO: add argument description
  RendezvousIntNumber   - GC_TODO: add argument description
  RendezvousProcLength  - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  UINTN           Index;
  CPU_DATA_BLOCK  *CpuData;

  if (NumberOfCPUs) {
    *NumberOfCPUs = mMPSystemData->NumberOfCpus;
  }

  if (MaximumNumberOfCPUs) {
    *MaximumNumberOfCPUs = mMPSystemData->MaximumCpusForThisSystem;
  }

  if (RendezvousProcLength) {
    *RendezvousProcLength = RENDEZVOUS_PROC_LENGTH;
  }

  if (RendezvousIntNumber) {
    *RendezvousIntNumber = 0;
  }

  if (NumberOfEnabledCPUs) {
    *NumberOfEnabledCPUs = 0;
    for (Index = 0; Index < mMPSystemData->NumberOfCpus; Index++) {
      CpuData = &mMPSystemData->CpuData[Index];
      if (mMPSystemData->EnableSecondaryCpu) {
        if (CpuData->State != CPU_STATE_DISABLED) {
          (*NumberOfEnabledCPUs)++;
        }
      } else {
        if (CpuData->State != CPU_STATE_DISABLED && !mMPSystemData->CpuData[Index].SecondaryCpu) {
          (*NumberOfEnabledCPUs)++;
        }
      }
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
GetProcessorContext (
  IN       VOID                        *This,
  IN       UINTN                        CpuNumber,
  IN OUT   UINTN                        *BufferLength,
  IN OUT   EFI_MP_PROC_CONTEXT          *ProcessorContextBuffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This                    - GC_TODO: add argument description
  CpuNumber               - GC_TODO: add argument description
  BufferLength            - GC_TODO: add argument description
  ProcessorContextBuffer  - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_BUFFER_TOO_SMALL - GC_TODO: Add description for return value
  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_MP_PROC_CONTEXT *ProcessorBuffer;
  CPU_DATA_BLOCK      *CpuData;

  if (BufferLength == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (*BufferLength < sizeof (EFI_MP_PROC_CONTEXT)) {
    *BufferLength = sizeof (EFI_MP_PROC_CONTEXT);
    return EFI_BUFFER_TOO_SMALL;
  }

  if ((mMPSystemData->NumberOfCpus <= CpuNumber) || (ProcessorContextBuffer == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  CpuData                   = &mMPSystemData->CpuData[CpuNumber];

  *BufferLength             = sizeof (EFI_MP_PROC_CONTEXT);
  ProcessorBuffer           = ProcessorContextBuffer;

  ProcessorBuffer->ApicID   = CpuData->ApicID;

  ProcessorBuffer->Enabled  = TRUE;
  if (!mMPSystemData->EnableSecondaryCpu) {
    if (CpuData->SecondaryCpu) {
      ProcessorBuffer->Enabled = FALSE;
    }
  }

  if (CpuData->State == CPU_STATE_DISABLED) {
    ProcessorBuffer->Enabled = FALSE;
  }

  if (CpuNumber == mMPSystemData->BSP) {
    ProcessorBuffer->Designation = EfiCpuBSP;
  } else {
    ProcessorBuffer->Designation = EfiCpuAP;
  }

  ProcessorBuffer->Health.Flags       = CpuData->Health;
  ProcessorBuffer->Health.TestStatus  = 0;

  ProcessorBuffer->PackageNumber      = CpuData->CpuDataforDatahub.Location.Package;
  ProcessorBuffer->NumberOfCores      = CpuData->NumberOfCores;
  ProcessorBuffer->NumberOfThreads    = CpuData->NumberOfThreads;
//  ProcessorBuffer->PhysicalLocation   = CpuData->PhysicalLocation;
  ProcessorBuffer->ProcessorTestMask  = 0;

  return EFI_SUCCESS;
}

EFI_STATUS
StartupThisAP (
  IN       VOID                                            *This,
  IN       EFI_AP_PROCEDURE                                Procedure,
  IN       UINTN                                           CpuNumber,
  IN       EFI_EVENT                                       WaitEvent OPTIONAL,
  IN       UINTN                                           TimeoutInMicroSecs OPTIONAL,
  IN OUT   VOID                                            *ProcArguments OPTIONAL
  )
/*++

Routine Description:

  MP Service to get specified application processor (AP)
  to execute a caller-provided code stream.

Arguments:

  This                - Pointer to MP Service Protocol
  Procedure           - The procedure to be assigned to AP.
  CpuNumber           - Number of the specified processor.
  WaitEvent           - If timeout, the event to be triggered after this AP finishes.
  TimeoutInMicroSecs  - The timeout value in microsecond. Zero means infinity.
  ProcArguments       - Argument for Procedure.

Returns:

  EFI_INVALID_PARAMETER - Procudure is NULL.
  EFI_INVALID_PARAMETER - Number of CPU out of range, or it belongs to BSP.
  EFI_INVALID_PARAMETER - Specified CPU is not idle.
  EFI_SUCCESS           - The AP has finished.
  EFI_TIMEOUT           - Time goes out before the AP has finished.

--*/
{
  EFI_STATUS      Status;
  CPU_DATA_BLOCK  *CpuData;
  UINT64          ExpectedTime;

  //
  // Check for incoming junk parameters.
  //
  if ((CpuNumber >= mMPSystemData->NumberOfCpus) || CpuNumber == mMPSystemData->BSP) {
    return EFI_INVALID_PARAMETER;
  }

  if (Procedure == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  CpuData = &mMPSystemData->CpuData[CpuNumber];

  //
  // As a first step, check if processor is OK to start up code stream.
  //
  if (CpuData->State != CPU_STATE_IDLE) {
    return EFI_INVALID_PARAMETER;
  }

  ExpectedTime                = CalculateTimeout (TimeoutInMicroSecs);

  mMPSystemData->StartCount   = 1;
  mMPSystemData->FinishCount  = 0;

  WakeUpAp (
    CpuData,
    Procedure,
    ProcArguments
    );

  while (TRUE) {
    AsmAcquireMPLock (&CpuData->StateLock);
    if (CpuData->State == CPU_STATE_FINISHED) {
      CpuData->State = CPU_STATE_IDLE;
      AsmReleaseMPLock (&CpuData->StateLock);
      break;
    }

    AsmReleaseMPLock (&CpuData->StateLock);

    if (CheckTimeout (ExpectedTime)) {
      //
      // Save data into private data structure, and create timer to poll AP state before exiting
      //
      mMPSystemData->StartedCpuNumber = CpuNumber;
      mMPSystemData->WaitEvent        = WaitEvent;
      Status = gBS->SetTimer (
                      mMPSystemData->CheckThisAPEvent,
                      TimerPeriodic,
                      CPU_CHECK_AP_INTERVAL * MICROSECOND
                      );
      return EFI_TIMEOUT;
    }

    gBS->Stall (CPU_CHECK_AP_INTERVAL * MICROSECOND);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
StartupAllAPs (
  IN  VOID                                                   *This,
  IN  EFI_AP_PROCEDURE                                       Procedure,
  IN  BOOLEAN                                                SingleThread,
  IN  EFI_EVENT                                              WaitEvent OPTIONAL,
  IN  UINTN                                                  TimeoutInMicroSecs OPTIONAL,
  IN  OUT VOID                                               *ProcArguments OPTIONAL,
  OUT UINTN                                                  *FailedCPUList OPTIONAL
  )
/*++

Routine Description:

  MP Service to get all the available application processors (APs)
  to execute a caller-provided code stream.

Arguments:

  This                - Pointer to MP Service Protocol
  Procedure           - The procedure to be assigned to APs.
  SingleThread        - If true, all APs execute in block mode.
                        Otherwise, all APs exceute in non-block mode.
  WaitEvent           - If timeout, the event to be triggered after all APs finish.
  TimeoutInMicroSecs  - The timeout value in microsecond. Zero means infinity.
  ProcArguments       - Argument for Procedure.
  FailedCPUList       - If not NULL, all APs that fail to start will be recorded in the list.

Returns:

  EFI_INVALID_PARAMETER - Procudure is NULL.
  EFI_SUCCESS           - Only 1 logical processor exists.
  EFI_SUCCESS           - All APs have finished.
  EFI_TIMEOUT           - Time goes out before all APs have finished.

--*/
{
  EFI_STATUS      Status;
  CPU_DATA_BLOCK  *CpuData;
  UINTN           ListIndex;
  UINTN           CpuNumber;
  UINTN           NextCpuNumber;
  UINT64          ExpectedTime;
  CPU_STATE       APInitialState;
  CPU_STATE       CpuState;

  //
  // Check for incoming junk parameters.
  //
  if (Procedure == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (mMPSystemData->NumberOfCpus == 1) {
    return EFI_SUCCESS;
  }

  ExpectedTime                = CalculateTimeout (TimeoutInMicroSecs);

  ListIndex                   = 0;
  CpuData                     = NULL;

  mMPSystemData->FinishCount  = 0;
  mMPSystemData->StartCount   = 0;
  APInitialState              = CPU_STATE_READY;

  for (CpuNumber = 0; CpuNumber < mMPSystemData->NumberOfCpus; CpuNumber++) {
    CpuData = &mMPSystemData->CpuData[CpuNumber];

    //
    // Get APs prepared, and put failing APs into FailedCPUList
    // if "SingleThread", only 1 AP will put to ready state, other AP will be put to ready
    // state 1 by 1, until the previous 1 finished its task
    // if not "SingleThread", all APs are put to ready state from the beginning
    //
    if (CpuNumber != mMPSystemData->BSP) {
      if (CpuData->State == CPU_STATE_IDLE) {
        mMPSystemData->StartCount++;

        AsmAcquireMPLock (&CpuData->StateLock);
        CpuData->State = APInitialState;
        AsmReleaseMPLock (&CpuData->StateLock);

        if (SingleThread) {
          APInitialState = CPU_STATE_BLOCKED;
        }

      } else if (FailedCPUList != NULL) {
        FailedCPUList[ListIndex] = CpuNumber;
        ListIndex++;
      }
    }
  }

  while (TRUE) {
    for (CpuNumber = 0; CpuNumber < mMPSystemData->NumberOfCpus; CpuNumber++) {
      CpuData = &mMPSystemData->CpuData[CpuNumber];
      if (CpuNumber == mMPSystemData->BSP) {
        continue;
      }

      AsmAcquireMPLock (&CpuData->StateLock);
      CpuState = CpuData->State;
      AsmReleaseMPLock (&CpuData->StateLock);

      switch (CpuState) {
        case CPU_STATE_READY:
          WakeUpAp (
            CpuData,
            Procedure,
            ProcArguments
            );
        break;

        case CPU_STATE_FINISHED:
          mMPSystemData->FinishCount++;
          if (SingleThread) {
            Status = GetNextBlockedCpuNumber (&NextCpuNumber);
            if (!EFI_ERROR (Status)) {
              mMPSystemData->CpuData[NextCpuNumber].State = CPU_STATE_READY;
            }
          }

          CpuData->State = CPU_STATE_IDLE;
          break;

        default:
          break;
      }
    }

    if (mMPSystemData->FinishCount == mMPSystemData->StartCount) {
      return EFI_SUCCESS;
    }

    if (CheckTimeout (ExpectedTime)) {
      //
      // Save data into private data structure, and create timer to poll AP state before exiting
      //
      mMPSystemData->Procedure      = Procedure;
      mMPSystemData->ProcArguments  = ProcArguments;
      mMPSystemData->SingleThread   = SingleThread;
      mMPSystemData->WaitEvent      = WaitEvent;

      Status = gBS->SetTimer (
                      mMPSystemData->CheckAllAPsEvent,
                      TimerPeriodic,
                      CPU_CHECK_AP_INTERVAL * MICROSECOND
                      );
      return EFI_TIMEOUT;
    }

    gBS->Stall (CPU_CHECK_AP_INTERVAL * MICROSECOND);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
SwitchBSP (
  IN  VOID                                *This,
  IN  UINTN                               CpuNumber,
  IN  BOOLEAN                             EnableOldBSP
  )
/*++

  Routine Description:

    MP Service to makes the current BSP into an AP and then switches the
    designated AP into the AP. This procedure is usually called after a CPU
    test that has found that BSP is not healthy to continue it's responsbilities.

  Arguments:

    This         - Pointer to MP Service Protocol.
    CpuNumber    - The number of the specified AP.
    EnableOldBSP - Whether to enable or disable the original BSP.

  Returns:

    EFI_INVALID_PARAMETER - Number for Specified AP out of range.
    EFI_INVALID_PARAMETER - Number of specified CPU belongs to BSP.
    EFI_NOT_READY         - Specified AP is not idle.
    EFI_SUCCESS           - BSP successfully switched.

--*/
{
  EFI_STATUS            Status;
  EFI_CPU_ARCH_PROTOCOL *CpuArch;
  BOOLEAN               OldInterruptState;
  CPU_DATA_BLOCK        *CpuData;
  CPU_STATE             CpuState;

  //
  // Check if the specified CPU number is valid
  //
  if (CpuNumber >= mMPSystemData->NumberOfCpus) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Check if the specified CPU is already BSP
  //
  if (CpuNumber == mMPSystemData->BSP) {
    return EFI_INVALID_PARAMETER;
  }

  CpuData = &mMPSystemData->CpuData[CpuNumber];
  if (CpuData->State != CPU_STATE_IDLE) {
    return EFI_NOT_READY;
  }

  //
  // Before send both BSP and AP to a procedure to exchange their roles,
  // interrupt must be disabled. This is because during the exchange role
  // process, 2 CPU may use 1 stack. If interrupt happens, the stack will
  // be corrputed, since interrupt return address will be pushed to stack
  // by hardware.
  //
  CpuArch = mMPSystemData->CpuArch;
#ifdef ECP_FLAG
  (CpuArch->GetInterruptState) (CpuArch, &OldInterruptState);
#else
  CpuArch->GetInterruptState (CpuArch, &OldInterruptState);
#endif
  if (OldInterruptState) {
    Status = CpuArch->DisableInterrupt (CpuArch);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  //
  // Unprogram virtual wire mode for the old BSP
  //
  ProgramVirtualWireMode (FALSE);
  SetApicBSPBit (FALSE);

  mMPSystemData->BSPInfo.State  = CPU_SWITCH_STATE_IDLE;
  mMPSystemData->BSPInfo.Lock   = VacantFlag;

  mMPSystemData->APInfo.State   = CPU_SWITCH_STATE_IDLE;
  mMPSystemData->APInfo.Lock    = VacantFlag;

  //
  // Need to wakeUp AP (future BSP).
  //
  WakeUpAp (
    CpuData,
    (EFI_AP_PROCEDURE) FutureBSPProc,
    mMPSystemData
  );

  AsmExchangeRole (&mMPSystemData->BSPInfo, &mMPSystemData->APInfo);

  //
  // The new BSP has come out. Since it carries the register value of the AP, need
  // to pay attention to variable which are stored in registers (due to optimization)
  //
  SetApicBSPBit (TRUE);
  ProgramVirtualWireMode (TRUE);

  if (OldInterruptState) {
    Status = CpuArch->EnableInterrupt (CpuArch);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  CpuData = &mMPSystemData->CpuData[mMPSystemData->BSP];
  while (TRUE) {
    AsmAcquireMPLock (&CpuData->StateLock);
    CpuState = CpuData->State;
    AsmReleaseMPLock (&CpuData->StateLock);

    if (CpuState == CPU_STATE_FINISHED) {
      break;
    }
  }

  Status              = ChangeCpuState (mMPSystemData->BSP, EnableOldBSP, EFI_CPU_CAUSE_NOT_DISABLED);
  mMPSystemData->BSP  = CpuNumber;

  return EFI_SUCCESS;
}

//
// ------------------------------------------------------------------------------------------
//
EFI_STATUS
SendIPI (
  IN  VOID                                *This,
  IN  UINTN                               CpuNumber,
  IN  UINTN                               VectorNumber,
  IN  UINTN                               DeliveryMode
  )
/*++
  Procedure: SendIPI - This procedure sends an IPI to the designated processor in
  the requested delivery mode with the requested vector.

--*/
// GC_TODO: function comment is missing 'Routine Description:'
// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
// GC_TODO:    This - add argument and description to function comment
// GC_TODO:    CpuNumber - add argument and description to function comment
// GC_TODO:    VectorNumber - add argument and description to function comment
// GC_TODO:    DeliveryMode - add argument and description to function comment
// GC_TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// GC_TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// GC_TODO:    EFI_INVALID_PARAMETER - add return value to function comment
{
  UINT32          TriggerMode;
  EFI_STATUS      Status;
  CPU_DATA_BLOCK  *CpuData;

  //
  // Check for incoming junk parameters.
  //
  if (CpuNumber >= mMPSystemData->NumberOfCpus || CpuNumber == mMPSystemData->BSP) {
    return EFI_INVALID_PARAMETER;
  }

  if (VectorNumber >= INTERRUPT_VECTOR_NUMBER) {
    return EFI_INVALID_PARAMETER;
  }

  if (DeliveryMode >= DELIVERY_MODE_MAX) {
    return EFI_INVALID_PARAMETER;
  }

  CpuData     = &mMPSystemData->CpuData[CpuNumber];
  TriggerMode = TRIGGER_MODE_EDGE;

  //
  // Fix the vector number for special interrupts like SMI and INIT.
  //
  if (DeliveryMode == DELIVERY_MODE_SMI || DeliveryMode == DELIVERY_MODE_INIT) {
    VectorNumber = 0x0;
  }

  Status = SendInterrupt (
            BROADCAST_MODE_SPECIFY_CPU,
            CpuData->ApicID,
            (UINT32)VectorNumber,
            (UINT32)DeliveryMode,
            TriggerMode,
            TRUE
            );

  return Status;
}
//
// ------------------------------------------------------------------------------------------
//
EFI_STATUS
EnableDisableAP (
  IN  VOID                                * This,
  IN  UINTN                               CpuNumber,
  IN  BOOLEAN                             NewAPState,
  IN  EFI_MP_HEALTH                       * HealthState OPTIONAL
  )
/*++
  Procedure: EnableDisableAP - This procedure enables Or disables APs.

--*/
// GC_TODO: function comment is missing 'Routine Description:'
// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
// GC_TODO:    This - add argument and description to function comment
// GC_TODO:    CpuNumber - add argument and description to function comment
// GC_TODO:    NewAPState - add argument and description to function comment
// GC_TODO:    HealthState - add argument and description to function comment
// GC_TODO:    EFI_INVALID_PARAMETER - add return value to function comment
// GC_TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS      Status;
  CPU_DATA_BLOCK  *CpuData;

  //
  // Check for incoming junk parameters.
  //
  if (CpuNumber >= mMPSystemData->NumberOfCpus || CpuNumber == mMPSystemData->BSP) {
    return EFI_INVALID_PARAMETER;
  }

  CpuData = &mMPSystemData->CpuData[CpuNumber];
  Status  = ChangeCpuState (CpuNumber, NewAPState, EFI_CPU_CAUSE_USER_SELECTION);

  if (HealthState != NULL) {
    CopyMem (&CpuData->Health, HealthState, sizeof (EFI_MP_HEALTH));
  }

  return EFI_SUCCESS;
}
//
// ------------------------------------------------------------------------------------
//
EFI_STATUS
WhoAmI (
  IN  VOID                                *This,
  OUT UINTN                               *CpuNumber
  )
/*++
  Procedure: WhoAmI - This procedure returns the calling CPU handle.

--*/
// GC_TODO: function comment is missing 'Routine Description:'
// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
// GC_TODO:    This - add argument and description to function comment
// GC_TODO:    CpuNumber - add argument and description to function comment
// GC_TODO:    EFI_SUCCESS - add return value to function comment
{
  UINTN ApicID;
  UINTN NumOfCpus;
  UINTN Index;

  ApicID    = GetApicID (NULL, NULL);

  NumOfCpus = mMPSystemData->NumberOfCpus;

  for (Index = 0; Index < NumOfCpus; Index++) {
    if (ApicID == mMPSystemData->CpuData[Index].ApicID) {
      break;
    }
  }

  *CpuNumber = Index;
  return EFI_SUCCESS;
}
//
// ------------------------------------------------------------------------------------
// Local services for the MP driver.
//
EFI_STATUS
GetMpBistStatus (
  IN     MP_SYSTEM_DATA  *MPSystemData
  )
/*++
Routine Description:

  Searches the HOB list provided by the core to find
  if a MP guided HOB list exists or not. If it does, it copies it to the driver
  data area, else returns 0

Arguments:

  MPSystemData - Pointer to an MP_SYSTEM_DATA structure

Returns:

  EFI_SUCCESS  - Success
  Others       - HOB not found or else

--*/
{
  EFI_STATUS              Status;
  VOID                    *DataInHob;
  UINTN                   DataSize;
  EFI_PEI_HOB_POINTERS    GuidHob;

  //
  // Get Hob list
  //
  DataInHob = NULL;
  DataSize = 0;
  GuidHob.Raw = GetHobList ();

  if (GuidHob.Raw == NULL) {
    DEBUG ((EFI_D_ERROR, "No HOBs found\n"));
    return EFI_NOT_FOUND;
  }
  //
  // Check for MP Data Hob.
  //
  GuidHob.Raw = GetNextGuidHob (&gEfiHtBistHobGuid, GuidHob.Raw);
  if (GuidHob.Raw != NULL) {
    DataInHob = GET_GUID_HOB_DATA (GuidHob.Guid);
    DataSize = GET_GUID_HOB_DATA_SIZE(GuidHob.Guid);
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_NOT_FOUND;
  }
  //
  // This is the MP HOB. So, copy all the data.
  //
  if (!(EFI_ERROR (Status))) {
    if(NULL == MPSystemData->BistHobData) {
#ifdef ECP_FLAG
      (gBS->AllocatePool) (
#else
      gBS->AllocatePool(
#endif
             EfiACPIMemoryNVS,
             DataSize,
             (VOID **) &MPSystemData->BistHobData
             );
    }
#ifdef ECP_FLAG
    (gBS->CopyMem) (MPSystemData->BistHobData, DataInHob, DataSize);
#else
    gBS->CopyMem (MPSystemData->BistHobData, DataInHob, DataSize);
#endif
    MPSystemData->BistHobSize = DataSize;
  }

  return Status;
}

EFI_STATUS
AllocateMemoryAndFillData (
  OUT EFI_PHYSICAL_ADDRESS           WakeUpBuffer,
  OUT VOID                           *StackAddressStart,
  IN  UINTN                          MaximumCPUsForThisSystem
  )
/*++

Routine Description:

  Allocate data pool for MP information and fill data in it.

Arguments:

  WakeUpBuffer             - The address of wakeup buffer.
  StackAddressStart        - The start address of APs's stacks.
  MaximumCPUsForThisSystem - Maximum CPUs in this system.

Returns:

  EFI_SUCCESS              - Function successfully executed.
  Other                    - Error occurred while allocating memory.

--*/
{
  EFI_STATUS                                  Status;

  //
  // First check if the MP data structures and AP rendezvous routine have been
  // supplied by the PEIMs that executed in early boot stage.
  //

  //
  // Clear the data structure area first.
  //
#ifdef ECP_FLAG
  (gBS->SetMem) (mMPSystemData, sizeof (MP_SYSTEM_DATA), 0);
#else
  gBS->SetMem (mMPSystemData, sizeof (MP_SYSTEM_DATA), 0);
#endif

  Status  = GetMpBistStatus (mMPSystemData);

  mAcpiCpuData->CpuPrivateData = (EFI_PHYSICAL_ADDRESS)(UINTN)(&(mMPSystemData->S3DataPointer));
  mAcpiCpuData->APState       = mPlatformCpu->HtState;
  mAcpiCpuData->WakeUpBuffer  = WakeUpBuffer;
  mAcpiCpuData->StackAddress  = (EFI_PHYSICAL_ADDRESS)(UINTN) StackAddressStart;

  Status = PrepareGdtIdtForAP ((IA32_DESCRIPTOR *) (UINTN) mAcpiCpuData->GdtrProfile, (IA32_DESCRIPTOR *) (UINTN) mAcpiCpuData->IdtrProfile);

  //
  // First BSP fills and inits all known values, including it's own records.
  //
  mMPSystemData->APSerializeLock     = VacantFlag;
  mMPSystemData->NumberOfCpus        = 1;
  mMPSystemData->EnableSecondaryCpu  = mPlatformCpu->HtState;

  //
  // Record these CPU configuration data (both for normal boot and for S3 use)
  //
  mPlatformCpu->GetTm2ControlInfo (mPlatformCpu, &mMPSystemData->Tm2Core2BusRatio, &mMPSystemData->Tm2Vid);
  mMPSystemData->PeciEnable                       = mPlatformCpu->PECIEnable;
  mMPSystemData->LimitCpuidMaximumValue           = mPlatformCpu->LimitCpuidMaximumValue;
  mMPSystemData->ExecuteDisableBit                = mPlatformCpu->ExecuteDisableBit;
  mMPSystemData->C1eEnable                        = mPlatformCpu->C1eEnable;
  mMPSystemData->AesEnable                        = mPlatformCpu->AesEnable;
  mMPSystemData->ProcessorVmxEnable               = mPlatformCpu->ProcessorVmxEnable;
  mMPSystemData->LtEnable                         = mPlatformCpu->LtEnable;
  mMPSystemData->EchoTprDisable                   = mPlatformCpu->EchoTprDisable;
  mMPSystemData->MonitorMwaitEnable               = mPlatformCpu->MonitorMwaitEnable;
  mMPSystemData->EnableL3Cache                    = mPlatformCpu->EnableL3Cache;
  mMPSystemData->FastString                       = mPlatformCpu->FastString;
  mMPSystemData->MachineCheckEnable               = mPlatformCpu->MachineCheckEnable;
  mMPSystemData->MLCStreamerPrefetcherEnable      = mPlatformCpu->MLCStreamerPrefetcherEnable;
  mMPSystemData->MLCSpatialPrefetcherEnable       = mPlatformCpu->MLCSpatialPrefetcherEnable;
  mMPSystemData->DCUStreamerPrefetcherEnable      = mPlatformCpu->DCUStreamerPrefetcherEnable;
  mMPSystemData->DCUIPPrefetcherEnable            = mPlatformCpu->DCUIPPrefetcherEnable;
  mMPSystemData->Gv3Enable                        = mPlatformCpu->Gv3State;
  mMPSystemData->PsdState                         = mPlatformCpu->PsdState;
  mMPSystemData->DcaEnable                        = mPlatformCpu->DcaState;
  mMPSystemData->DcaPrefetchDelayValue            = mPlatformCpu->DcaPrefetchDelayValue;
  mMPSystemData->TurboModeEnable                  = mPlatformCpu->TurboModeEnable;
  mMPSystemData->ExtremeEnable                    = mPlatformCpu->ExtremeEnable;
  mMPSystemData->XapicEnable                      = mPlatformCpu->XapicEnable;
  mMPSystemData->CcxEnable                        = mPlatformCpu->CcxEnable;
  mMPSystemData->C1AutoDemotion                   = mPlatformCpu->C1AutoDemotion;
  mMPSystemData->C3AutoDemotion                   = mPlatformCpu->C3AutoDemotion;
  mMPSystemData->PackageCState                    = mPlatformCpu->PackageCState;
  mMPSystemData->ProcessorMsrLockControl          = mPlatformCpu->ProcessorMsrLockControl;
  mMPSystemData->Processor3StrikeControl          = mPlatformCpu->Processor3StrikeControl;
  mMPSystemData->S3DataPointer.S3BootScriptTable  = (UINT32)(UINTN)mMPSystemData->S3BootScriptTable;
  mMPSystemData->S3DataPointer.S3BspMtrrTable     = (UINT32)(UINTN)mMPSystemData->S3BspMtrrTable;
  mMPSystemData->S3DataPointer.VirtualWireMode    = mPlatformCpu->VirtualWireMode;
  mMPSystemData->Vr11Enable                       = mPlatformCpu->Vr11Enable;
  mMPSystemData->DCUModeSelection                 = mPlatformCpu->DCUModeSelection;
  mMPSystemData->BiDirectionalProchot             = mPlatformCpu->BiDirectionalProchot;
  mMPSystemData->ActiveProcessorCores             = mPlatformCpu->ActiveProcessorCores;

  mCommonCStateValue                              = mMPSystemData->PackageCState;

  mMPSystemData->CpuArch = NULL;
  gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **) &mMPSystemData->CpuArch);

  mMPSystemData->MaximumCpusForThisSystem = MaximumCPUsForThisSystem;

  mMPSystemData->BSP = 0;

  FillInProcessorInformation (mMPSystemData, TRUE, 0);

  return EFI_SUCCESS;
}

EFI_STATUS
CountApNumberAndCollectBist (
  IN EFI_PHYSICAL_ADDRESS     WakeUpBuffer
  )
/*++

Routine Description:

  Wake up APs for the first time to count their number and collect BIST data.

Arguments:

  WakeUpBuffer      - Address of the wakeup buffer.

Returns:

  EFI_SUCCESS       - Function successfully finishes.

--*/
{
  MP_CPU_EXCHANGE_INFO    *ExchangeInfo;
  UINTN                   Index;

  //
  // Send INIT IPI - SIPI to all APs
  //
  SendInterrupt (BROADCAST_MODE_ALL_EXCLUDING_SELF, 0, 0, DELIVERY_MODE_INIT, TRIGGER_MODE_EDGE, TRUE);
  SendInterrupt (
    BROADCAST_MODE_ALL_EXCLUDING_SELF,
    0,
    (UINT32) RShiftU64 (WakeUpBuffer, 12),
    DELIVERY_MODE_SIPI,
    TRIGGER_MODE_EDGE,
    TRUE
    );
  //
  // Wait for task to complete and then exit.
  //
  gBS->Stall (50000);

  ExchangeInfo = (MP_CPU_EXCHANGE_INFO *) (UINTN) (WakeUpBuffer + MP_CPU_EXCHANGE_INFO_OFFSET);

  for (Index = 0; Index < MAXIMUM_CPU_NUMBER; Index++) {
    if (ExchangeInfo->BistBuffer[Index].Number == 1) {
      ExchangeInfo->BistBuffer[Index].Number = (UINT32) mMPSystemData->NumberOfCpus++;
    }
  }
  mAcpiCpuData->NumberOfCpus  = (UINT32) mMPSystemData->NumberOfCpus;

  ExchangeInfo->InitFlag = 0;

  return EFI_SUCCESS;
}


EFI_STATUS
PollForInitialization (
  IN EFI_PHYSICAL_ADDRESS     WakeUpBuffer
  )
/*++

Routine Description:

  Wake up APs for the second time to collect detailed information.

Arguments:

  WakeUpBuffer      - Address of the wakeup buffer.

Returns:

  EFI_SUCCESS       - Function successfully finishes.

--*/
{
  MP_CPU_EXCHANGE_INFO    *ExchangeInfo;
  ExchangeInfo             = (MP_CPU_EXCHANGE_INFO *) (UINTN) (WakeUpBuffer + MP_CPU_EXCHANGE_INFO_OFFSET);
  ExchangeInfo->ApFunction = (VOID *) (UINTN) DetailedInitialization;

  SendInterrupt (BROADCAST_MODE_ALL_EXCLUDING_SELF, 0, 0, DELIVERY_MODE_INIT, TRIGGER_MODE_EDGE, TRUE);
  SendInterrupt (
    BROADCAST_MODE_ALL_EXCLUDING_SELF,
    0,
    (UINT32) RShiftU64 (WakeUpBuffer, 12),
    DELIVERY_MODE_SIPI,
    TRIGGER_MODE_EDGE,
    TRUE
    );
  //
  // Wait until all APs finish
  //
  while (mFinishedCount < mAcpiCpuData->NumberOfCpus - 1) {
    CpuPause ();
  }

  return EFI_SUCCESS;
}

EFI_STATUS
InitializeMpSystemData (
  VOID
  )
/*++

Routine Description:

  Initialize multiple processors and collect MP related data

Arguments:

  None

Returns:

  EFI_SUCCESS           - Multiple processors get initialized and data collected successfully

  Other                 - The operation failed due to some reason

--*/
{
  EFI_STATUS              Status;
  UINT32                  MaxThreadsPerCore;
  UINT32                  MaxCoresPerDie;
  UINT32                  MaxDiesPerPackage;
  UINT32                  MaxPackages;

  VOID                    *StackAddressStart;
  EFI_PHYSICAL_ADDRESS    WakeUpBuffer;
  MP_CPU_EXCHANGE_INFO    *ExchangeInfo;
  UINTN                   Index;

  EFI_CPU_ARCH_PROTOCOL   *CpuArch;
  BOOLEAN                 mInterruptState;
  CPU_DATA_BLOCK          *CpuData;
  CPU_DATA_FOR_DATAHUB    *CpuDataforDatahub;
  UINTN                   MaximumCPUsForThisSystem;
  BOOLEAN                 PhysicalCpu;
  STATIC    UINTN         PhysicalCpuCount = 1;

  ProgramVirtualWireMode (TRUE);
  Status = mPlatformCpu->GetMaxCount (
                           mPlatformCpu,
                           &MaxThreadsPerCore,
                           &MaxCoresPerDie,
                           &MaxDiesPerPackage,
                           &MaxPackages
                           );
  if (!EFI_ERROR (Status)) {
    MaximumCPUsForThisSystem = MaxThreadsPerCore * MaxCoresPerDie * MaxDiesPerPackage * MaxPackages;
  } else {
    MaximumCPUsForThisSystem = MAXIMUM_CPU_NUMBER;
  }

  Status = PrepareMemoryForAPs (
             &WakeUpBuffer,
             &StackAddressStart,
             MaximumCPUsForThisSystem
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mOriginalBuffer             = WakeUpBuffer;

#ifdef ECP_FLAG
  Status = (gBS->AllocatePages) (
#else
  Status = gBS->AllocatePages (
#endif
                  AllocateAnyPages,
                  EfiBootServicesData,
                  1,
                  &mBackupBuffer
                  );

  Status = AllocateMemoryAndFillData (
             WakeUpBuffer,
             StackAddressStart,
             MaximumCPUsForThisSystem
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ExchangeInfo = (MP_CPU_EXCHANGE_INFO *) (UINTN) (WakeUpBuffer + MP_CPU_EXCHANGE_INFO_OFFSET);
  Status = PrepareExchangeInfo (
             ExchangeInfo,
             StackAddressStart,
             NULL,
             WakeUpBuffer
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CpuArch = mMPSystemData->CpuArch;
#ifdef ECP_FLAG
  (CpuArch->GetInterruptState) (CpuArch, &mInterruptState);
#else
  CpuArch->GetInterruptState (CpuArch, &mInterruptState);
#endif
  CpuArch->DisableInterrupt (CpuArch);

  //
  // For B stepping and above use broadcast
  //
  CountApNumberAndCollectBist (WakeUpBuffer);
  ExchangeInfo->WakeUpApManner = WakeUpApCounterInit;
  PollForInitialization (WakeUpBuffer);

  ExchangeInfo->ApFunction = (VOID *) (UINTN) ApProcWrapper;
  if (mInterruptState) {
    CpuArch->EnableInterrupt (CpuArch);
  }

  for (Index = 1; Index < mMPSystemData->NumberOfCpus; Index++) {
    CpuData = &mMPSystemData->CpuData[Index];
    if (CpuData->Health.Uint32 != 0) {
      REPORT_STATUS_CODE_EX (
        EFI_ERROR_MAJOR | EFI_ERROR_CODE,
        EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_EC_SELF_TEST,
        (UINT32) Index,
        &gEfiCallerIdGuid,
        NULL,
        NULL,
        0
        );
    }

//TODO: need check why here load ucode fail
//    Status  = CheckMicrocodeUpdate (Index, CpuData->MicrocodeStatus, CpuData->FailedRevision);
  }

  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  CheckAllAPsStatus,
                  NULL,
                  &mMPSystemData->CheckAllAPsEvent
                  );
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  CheckThisAPStatus,
                  NULL,
                  &mMPSystemData->CheckThisAPEvent
                  );

  if (mPlatformCpu->BspSelection == 16) {
    Status = SwitchToLowestFeatureProcess (&mMpService);
  }

  //
  // Get all supported features
  //
  ProgramProcessorFuncs (mMPSystemData);
  ExchangeInfo->WakeUpApManner = WakeUpApPerHltLoop;
  Status = mMpService.StartupAllAPs (
                        &mMpService,
                        (EFI_AP_PROCEDURE) ProgramProcessorFuncs,
                        TRUE,
                        NULL,
                        0,
                        (VOID *) mMPSystemData,
                        NULL
                        );

  InitializeFeaturePerSetup (mMPSystemData);

  CollectProcessorFeature ((VOID *) &mMpService);
  Status = mMpService.StartupAllAPs (
                        &mMpService,
                        CollectProcessorFeature,
                        TRUE,
                        NULL,
                        0,
                        (VOID *) &mMpService,
                        NULL
                        );

  Status = mMpService.StartupAllAPs (
                        &mMpService,
                        ProgramProcessorFeature,
                        TRUE,
                        NULL,
                        0,
                        (VOID *) &mMpService,
                        NULL
                        );

  ProgramProcessorFeature ((VOID *) &mMpService);

  //
  // Save Mtrr Register for S3 resume
  // This must be done after CPU feature programming so that the number of MTRR is reflected correctly
  //
  SaveBspMtrrForS3 ();

  UpdatePlatformCpuData();

  for (Index = 0; Index < mMPSystemData->NumberOfCpus; Index++) {
    CpuDataforDatahub = &mMPSystemData->CpuData[Index].CpuDataforDatahub;
    UpdateDataforDatahub (Index, CpuDataforDatahub);

    MaxPackages = 0;
    PhysicalCpu = FALSE;
    if (Index < 1) {
      PhysicalCpu = TRUE;
      PhysicalCpuCount ++;
    } else {
        Status = mPlatformCpu->GetMaxCount (
                                 mPlatformCpu,
                                 &MaxThreadsPerCore,
                                 &MaxCoresPerDie,
                                 &MaxDiesPerPackage,
                                 &MaxPackages
                                 );
        if (! EFI_ERROR (Status)) {
          if ((Index + 1) == (PhysicalCpuCount * MaxThreadsPerCore * MaxCoresPerDie * MaxDiesPerPackage)) {
            PhysicalCpu = TRUE;
            PhysicalCpuCount ++;
          }
        }
    }

    if (PhysicalCpu) {
      InitializeProcessorData (Index, CpuDataforDatahub);
      InitializeCacheData (Index, CpuDataforDatahub->CacheInformation);
    }
  }

//AMI_OVERRIDE - EIP137713 Hang at CP 0x68 when limitcpuid is enabled. >>
/*
  ProgrameCpuidLimit(mMPSystemData);
  Status = mMpService.StartupAllAPs (
                        &mMpService,
                        ProgrameCpuidLimit,
                        TRUE,
                        NULL,
                        0,
                        (VOID *) &mMPSystemData,
                        NULL
                        );
*/
//AMI_OVERRIDE - EIP137713 Hang at CP 0x68 when limitcpuid is enabled. <<

  CopyMem ((VOID *) (UINTN) mBackupBuffer, (VOID *) (UINTN) mOriginalBuffer, EFI_PAGE_SIZE);

  return EFI_SUCCESS;
}

VOID
ApProcWrapper (
  VOID
  )
/*++

Routine Description:

  Wrapper function for all procedures assigned to AP via MP service protocol.
  It controls states of AP and invokes assigned precedure.

Arguments:

  None.

Returns:

  None

--*/
{
  EFI_AP_PROCEDURE      Procedure;
  VOID                  *Parameter;
  UINTN                 CpuNumber;
  CPU_DATA_BLOCK        *CpuData;

  //
  // Initialize MCE for CR4.
  //
  InitializeMce (mMPSystemData->MachineCheckEnable);

  WhoAmI (&mMpService, &CpuNumber);
  CpuData = &mMPSystemData->CpuData[CpuNumber];

  //
  // Now let us check it out.
  //
  AsmAcquireMPLock (&CpuData->ProcedureLock);
  Procedure = CpuData->Procedure;
  Parameter = CpuData->Parameter;
  AsmReleaseMPLock (&CpuData->ProcedureLock);

  if (Procedure != NULL) {
    AsmAcquireMPLock (&CpuData->StateLock);
    CpuData->State = CPU_STATE_BUSY;
    AsmReleaseMPLock (&CpuData->StateLock);

    Procedure (Parameter);

    //
    // if BSP is switched to AP, it continue execute from here, but it carries register state
    // of the old AP, so need to reload CpuData (might be stored in a register after compiler
    // optimization) to make sure it points to the right data
    //
    WhoAmI (&mMpService, &CpuNumber);
    CpuData = &mMPSystemData->CpuData[CpuNumber];

    AsmAcquireMPLock (&CpuData->ProcedureLock);
    CpuData->Procedure = NULL;
    AsmReleaseMPLock (&CpuData->ProcedureLock);

    AsmAcquireMPLock (&CpuData->StateLock);
    CpuData->State = CPU_STATE_FINISHED;
    AsmReleaseMPLock (&CpuData->StateLock);
  }
}

VOID
DetailedInitialization (
  VOID
  )
/*++

Routine Description:

  Procedure for detailed initialization of APs. It will be assigned to all APs while
  they are waken up for the second time.

Arguments:

  None.

Returns:

  None.

--*/
{
  UINT64                MiscEnable;
  EFI_STATUS            Status;
  UINT32                FailedRevision;

  CpuInitFloatPointUnit ();

  AsmAcquireMPLock (&mMPSystemData->APSerializeLock);

  Status = InitializeMicrocode (
             (EFI_CPU_MICROCODE_HEADER **) (UINTN) mAcpiCpuData->MicrocodePointerBuffer,
             &FailedRevision,
             FALSE
             );

  //
  // Save Mtrr Registers in global data areas
  //
  MpMtrrSynchUp (NULL);

  ProgramVirtualWireMode (FALSE);

  if (!IsSecondaryThread ()) {
    if (ExistL3Cache ()) {
      MiscEnable = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
      if (mPlatformCpu->EnableL3Cache) {
        MiscEnable &= ~MSR_L3_CACHE_DISABLE;
      } else {
        MiscEnable |= MSR_L3_CACHE_DISABLE;
      }

      AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, MiscEnable);
    }
  }

  FillInProcessorInformation (mMPSystemData, FALSE, 0);

  mFinishedCount++;

  AsmReleaseMPLock (&mMPSystemData->APSerializeLock);
}

VOID
FutureBSPProc (
  IN     MP_SYSTEM_DATA  *MPSystemData
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MPSystemData  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  AsmExchangeRole (&MPSystemData->APInfo, &MPSystemData->BSPInfo);
  return ;
}

//
// -------------------------------------------------------------------------------------
//
EFI_STATUS
FillInProcessorInformation (
  IN     MP_SYSTEM_DATA       *MPSystemData,
  IN     BOOLEAN              BSP,
  IN     UINT32               BistParam
  )
/*++

Routine Description:

  This function is called by all processors (both BSP and AP) once and collects MP related data

Arguments:

  MPSystemData  - Pointer to the data structure containing MP related data
  BSP           - TRUE if the CPU is BSP
  BistParam     - BIST (build-in self test) data for the processor. This data
                  is only valid for processors that are waked up for the 1st
                  time in this CPU DXE driver.

Returns:

  EFI_SUCCESS   - Data for the processor collected and filled in

--*/
{
  UINT32               Health;
  UINT32               ApicID;
  CPU_DATA_BLOCK       *CpuData;
  UINT32               BIST;
  UINTN                CpuNumber;
  UINTN                Index;
  UINTN                Count;
  MP_CPU_EXCHANGE_INFO *ExchangeInfo;

  ApicID = GetApicID (NULL, NULL);
  BIST   = 0;

  if (BSP) {
    CpuNumber = 0;
    BIST      = BistParam;
  } else {
    ExchangeInfo = (MP_CPU_EXCHANGE_INFO *) (UINTN) (mAcpiCpuData->WakeUpBuffer + MP_CPU_EXCHANGE_INFO_OFFSET);
    CpuNumber    = ExchangeInfo->BistBuffer[ApicID].Number;
    BIST         = ExchangeInfo->BistBuffer[ApicID].BIST;
  }

  CpuData                 = &MPSystemData->CpuData[CpuNumber];
  CpuData->SecondaryCpu   = IsSecondaryThread ();
  CpuData->ApicID         = ApicID;
  CpuData->Procedure      = NULL;
  CpuData->Parameter      = NULL;
  CpuData->StateLock      = VacantFlag;
  CpuData->ProcedureLock  = VacantFlag;
  CpuData->State          = CPU_STATE_IDLE;

  Health = BIST;
  Count  = MPSystemData->BistHobSize / sizeof(BIST_HOB_DATA);
  for (Index = 0; Index < Count; Index++) {
    if (ApicID ==  MPSystemData->BistHobData[Index].ApicId) {
      Health = MPSystemData->BistHobData[Index].Health.Uint32;
    }
  }

  if (Health > 0) {
    CpuData->State                        = CPU_STATE_DISABLED;
    MPSystemData->DisableCause[CpuNumber] = EFI_CPU_CAUSE_SELFTEST_FAILURE;
  } else {
    MPSystemData->DisableCause[CpuNumber] = EFI_CPU_CAUSE_NOT_DISABLED;
  }

  //
  // Get Core and Thread number
  //
  CpuData->NumberOfCores = mPlatformCpuInfo.CpuPackage.CoresPerPhysicalPackage;

  if (mPlatformCpuInfo.CpuFeatures.Ht) {
    CpuData->NumberOfThreads = mPlatformCpuInfo.CpuPackage.LogicalProcessorsPerPhysicalPackage / CpuData->NumberOfCores;
  } else {
    CpuData->NumberOfThreads  = 1;
  }

  FillinDataforDataHub (CpuNumber, &CpuData->CpuDataforDatahub);
  CpuData->CpuDataforDatahub.Health.Uint32 = Health;

  CopyMem (&CpuData->PhysicalLocation, &CpuData->CpuDataforDatahub.Location, sizeof (PHYSICAL_LOCATION));

  return EFI_SUCCESS;
}

EFI_STATUS
OverrideCpuData (
  VOID
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  None

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
/*
  EFI_CPU_PHYSICAL_LOCATION     Location;
  EFI_PLATFORM_CPU_INFORMATION  PlatformCpuInfo;
  UINTN                         CpuNumber;
*/
  EFI_STATUS                    Status;

  UINT32                        MaxThreadsPerCore;
  UINT32                        MaxCoresPerDie;
  UINT32                        MaxDiesPerPackage;
  UINT32                        MaxPackages;

  UINTN                         Index;
  UINTN                         ContextSize;
  UINTN                         BSP;
//  CPU_DATA_BLOCK                *CpuData;
  EFI_DETAILED_CPU_INFO         *DetailedInfo;

  //
  // BUGBUG: if BSP is not healthy, a new healthy CPU should be selected as BSP
  //
  Status = mPlatformCpu->GetMaxCount (
                          mPlatformCpu,
                          &MaxThreadsPerCore,
                          &MaxCoresPerDie,
                          &MaxDiesPerPackage,
                          &MaxPackages
                          );
  ASSERT_EFI_ERROR (Status);
/*
  if (!EFI_ERROR (Status)) {
    for (Location.Package = 0; Location.Package < MaxPackages; Location.Package++) {
      for (Location.Die = 0; Location.Die < MaxDiesPerPackage; Location.Die++) {
        for (Location.Core = 0; Location.Core < MaxCoresPerDie; Location.Core++) {
          for (Location.Thread = 0; Location.Thread < MaxThreadsPerCore; Location.Thread++) {
            Status = mPlatformCpu->GetCpuInfo (mPlatformCpu, &Location, &PlatformCpuInfo);
            if (EFI_ERROR (Status)) {
              continue;
            }

            for (CpuNumber = 0; CpuNumber < mMPSystemData->NumberOfCpus; CpuNumber++) {
              CpuData = &mMPSystemData->CpuData[CpuNumber];
              if (PlatformCpuInfo.ApicID == CpuData->ApicID) {
                CopyMem (&CpuData->CpuDataforDatahub.Location, &Location, sizeof (Location));
              }
            }
          }
        }
      }
    }
  }
*/

  ContextSize = sizeof (EFI_MP_PROC_CONTEXT);
#ifdef ECP_FLAG
  Status = (gBS->AllocatePool) (
#else
  Status = gBS->AllocatePool (
#endif
                  EfiBootServicesData,
                  sizeof (EFI_DETAILED_CPU_INFO) * mMPSystemData->NumberOfCpus,
                  (VOID **) &DetailedInfo
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < mMPSystemData->NumberOfCpus; Index++) {
    Status = CollectDetailedInfo (Index, &DetailedInfo[Index]);
  }

  BSP = mMPSystemData->BSP;
  Status = mPlatformCpu->OverridePolicy (
                          mPlatformCpu,
                          (UINT32) mMPSystemData->NumberOfCpus,
                          DetailedInfo,
                          mMPSystemData->DisableCause,
                          (UINT32 *) &BSP
                          );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < mMPSystemData->NumberOfCpus; Index++) {

    if (mMPSystemData->DisableCause[Index] != EFI_CPU_CAUSE_NOT_DISABLED) {
      mMPSystemData->CpuData[Index].State = CPU_STATE_DISABLED;
    }
  }

  if (BSP != mMPSystemData->BSP) {
    Status = SwitchBSP (
              &mMpService,
              BSP,
              (BOOLEAN) (mMPSystemData->DisableCause[mMPSystemData->BSP] == EFI_CPU_CAUSE_NOT_DISABLED)
              );
  }

  return EFI_SUCCESS;;
}

EFI_STATUS
SetApicBSPBit (
  IN  BOOLEAN   Enable
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Enable  - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  UINT64  ApicBaseReg;

  ApicBaseReg = AsmReadMsr64 (EFI_MSR_IA32_APIC_BASE);

  if (Enable) {
    ApicBaseReg |= B_EFI_MSR_IA32_APIC_BASE_BSP;
  } else {
    ApicBaseReg &= ~(B_EFI_MSR_IA32_APIC_BASE_BSP | 0xFF);
  }

  AsmWriteMsr64 (EFI_MSR_IA32_APIC_BASE, ApicBaseReg);

  return EFI_SUCCESS;
}

EFI_STATUS
CollectDetailedInfo (
  IN  UINTN                       CpuNumber,
  OUT EFI_DETAILED_CPU_INFO       *DetailedInfo
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber     - GC_TODO: add argument description
  DetailedInfo  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  EFI_STATUS            Status;
  UINTN                 ContextSize;
  CPU_DATA_FOR_DATAHUB  *CpuDataforDatahub;

  ContextSize = sizeof (EFI_MP_PROC_CONTEXT);

#ifdef ECP_FLAG
  Status = (gBS->AllocatePool) (
#else
  Status = gBS->AllocatePool (
#endif
                  EfiBootServicesData,
                  ContextSize,
                  (VOID **) &DetailedInfo->Context
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = GetProcessorContext (&mMpService, CpuNumber, &ContextSize, DetailedInfo->Context);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  CpuDataforDatahub               = &mMPSystemData->CpuData[CpuNumber].CpuDataforDatahub;

  DetailedInfo->CoreFreq.Value    = (UINT16) CpuDataforDatahub->IntendCoreFrequency;
  DetailedInfo->CoreFreq.Exponent = 6;

  DetailedInfo->BusFreq.Value     = (UINT16) CpuDataforDatahub->IntendFsbFrequency;
  DetailedInfo->BusFreq.Exponent  = 6;

  CopyMem (&DetailedInfo->CpuId, &CpuDataforDatahub->CpuidData, sizeof (EFI_PROCESSOR_ID_DATA));

  DetailedInfo->MuData.ProcessorMicrocodeType           = EfiProcessorIa32Microcode;
  DetailedInfo->MuData.ProcessorMicrocodeRevisionNumber = CpuDataforDatahub->MicrocodeRevision;

  DetailedInfo->Status = CpuDataforDatahub->Status;

  Status = CollectCacheInfo (CpuDataforDatahub, DetailedInfo);
  return Status;
}

EFI_STATUS
ChangeCpuState (
  IN     UINTN                      CpuNumber,
  IN     BOOLEAN                    NewState,
  IN     EFI_CPU_STATE_CHANGE_CAUSE Cause
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber - GC_TODO: add argument description
  NewState  - GC_TODO: add argument description
  Cause     - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  CPU_DATA_BLOCK                              *CpuData;
  EFI_COMPUTING_UNIT_CPU_DISABLED_ERROR_DATA  ErrorData;

  CpuData = &mMPSystemData->CpuData[CpuNumber];

  mMPSystemData->DisableCause[CpuNumber] = Cause;

  if (!NewState) {
    AsmAcquireMPLock (&CpuData->StateLock);
    CpuData->State = CPU_STATE_DISABLED;
    AsmReleaseMPLock (&CpuData->StateLock);

    ErrorData.DataHeader.HeaderSize = sizeof (EFI_STATUS_CODE_DATA);
    ErrorData.DataHeader.Size       = sizeof (EFI_COMPUTING_UNIT_CPU_DISABLED_ERROR_DATA) - sizeof (EFI_STATUS_CODE_DATA);
    CopyMem (
      &ErrorData.DataHeader.Type,
      &gEfiStatusCodeSpecificDataGuid,
      sizeof (EFI_GUID)
      );
    ErrorData.Cause             = Cause;
    ErrorData.SoftwareDisabled  = TRUE;
    REPORT_STATUS_CODE_EX (
      EFI_ERROR_MINOR | EFI_ERROR_CODE,
      EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_EC_DISABLED,
      (UINT32) CpuNumber,
      &gEfiCallerIdGuid,
      NULL,
      (EFI_STATUS_CODE_DATA *) &ErrorData,
      sizeof(EFI_COMPUTING_UNIT_CPU_DISABLED_ERROR_DATA)
      );
  } else {
    AsmAcquireMPLock (&CpuData->StateLock);
    CpuData->State = CPU_STATE_IDLE;
    AsmReleaseMPLock (&CpuData->StateLock);
  }

  return EFI_SUCCESS;
}

BOOLEAN
ExistL3Cache (
  VOID
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  None

Returns:

  GC_TODO: add return values

--*/
{
  UINT8               Index;

  Index   = 0;
  while (mPlatformCpuInfo.CpuCache.CacheDescriptor[Index] != 0) {
    if ((mPlatformCpuInfo.CpuCache.CacheDescriptor[Index] & 0xF0) == 0x20) {
      return TRUE;
    }
    if (mPlatformCpuInfo.CpuCache.CacheDescriptor[Index] == 0x40) {
      return FALSE;
    }
    Index++;
  }

  return FALSE;
}

BOOLEAN
IsSecondaryThread (
  VOID
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  None

Returns:

  GC_TODO: add return values

--*/
{
  UINT32              ApicID;
  UINT8               CpuPerCore;
  UINT32              Mask;

  ApicID = GetApicID (NULL, NULL);

  CpuPerCore = mPlatformCpuInfo.CpuPackage.ThreadsPerCore;
  if (CpuPerCore == 1) {
    return FALSE;
  }

  //
  // Assume 1 Core has no more than 8 threads
  //
  if (CpuPerCore == 2) {
    Mask = 0x1;
  } else if (CpuPerCore <= 4) {
    Mask = 0x3;
  } else {
    Mask = 0x7;
  }

  if (ApicID & Mask) {
    return TRUE;
  }

  return FALSE;
}

VOID
CheckAllAPsStatus (
  IN  EFI_EVENT                           Event,
  IN  VOID                                *Context
  )
/*++

Routine Description:

  If timeout occurs in StartupAllAps(), a timer is set, which invokes this
  procedure periodically to check whether all APs have finished.

Arguments:

  Event   - Event triggered.
  Context - Parameter passed with the event.

Returns:

  None

--*/
{
  UINTN           CpuNumber;
  UINTN           NextCpuNumber;
  CPU_DATA_BLOCK  *CpuData;
  CPU_DATA_BLOCK  *NextCpuData;
  EFI_STATUS      Status;
  CPU_STATE       CpuState;

  for (CpuNumber = 0; CpuNumber < mMPSystemData->NumberOfCpus; CpuNumber++) {
    CpuData = &mMPSystemData->CpuData[CpuNumber];
    if (CpuNumber == mMPSystemData->BSP) {
      continue;
    }

    AsmAcquireMPLock (&CpuData->StateLock);
    CpuState = CpuData->State;
    AsmReleaseMPLock (&CpuData->StateLock);

    switch (CpuState) {
      case CPU_STATE_READY:
        WakeUpAp (
          CpuData,
          mMPSystemData->Procedure,
          mMPSystemData->ProcArguments
          );
        break;

      case CPU_STATE_FINISHED:
        if (mMPSystemData->SingleThread) {
          Status = GetNextBlockedCpuNumber (&NextCpuNumber);
          if (!EFI_ERROR (Status)) {
            NextCpuData = &mMPSystemData->CpuData[NextCpuNumber];

            AsmAcquireMPLock (&NextCpuData->ProcedureLock);
            NextCpuData->State = CPU_STATE_READY;
            AsmReleaseMPLock (&NextCpuData->ProcedureLock);

            WakeUpAp (
              NextCpuData,
              mMPSystemData->Procedure,
              mMPSystemData->ProcArguments
              );
          }
        }

        CpuData->State = CPU_STATE_IDLE;
        mMPSystemData->FinishCount++;
        break;

      default:
        break;
    }
  }

  if (mMPSystemData->FinishCount == mMPSystemData->StartCount) {
    gBS->SetTimer (
           mMPSystemData->CheckAllAPsEvent,
           TimerCancel,
           0
           );
    Status = gBS->SignalEvent (mMPSystemData->WaitEvent);
  }

  return ;
}

VOID
CheckThisAPStatus (
  IN  EFI_EVENT                           Event,
  IN  VOID                                *Context
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Event   - GC_TODO: add argument description
  Context - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  CPU_DATA_BLOCK  *CpuData;
  EFI_STATUS      Status;
  CPU_STATE       CpuState;

  CpuData = &mMPSystemData->CpuData[mMPSystemData->StartedCpuNumber];

  AsmAcquireMPLock (&CpuData->StateLock);
  CpuState = CpuData->State;
  AsmReleaseMPLock (&CpuData->StateLock);

  if (CpuState == CPU_STATE_FINISHED) {
    gBS->SetTimer (
          mMPSystemData->CheckThisAPEvent,
          TimerCancel,
          0
          );
    Status = gBS->SignalEvent (mMPSystemData->WaitEvent);
    AsmAcquireMPLock (&CpuData->StateLock);
    CpuData->State = CPU_STATE_IDLE;
    AsmReleaseMPLock (&CpuData->StateLock);
  }

  return ;
}

UINT64
CalculateTimeout (
  IN  UINTN                               TimeoutInMicroSecs
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  TimeoutInMicroSecs  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  UINT64      CurrentTsc;
  UINT64      ExpectedTsc;
  UINT64      Frequency;
  EFI_STATUS  Status;

  if (TimeoutInMicroSecs == 0) {
    return 0xffffffffffff;
  }

  CurrentTsc  = AsmReadTsc ();

  Status      = GetActualFrequency (mMetronome, &Frequency);

  ExpectedTsc = CurrentTsc + MultU64x32 (Frequency, (UINT32)TimeoutInMicroSecs);

  return ExpectedTsc;
}

BOOLEAN
CheckTimeout (
  IN  UINT64                              ExpectedTsc
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ExpectedTsc - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  UINT64  CurrentTsc;

  CurrentTsc = AsmReadTsc ();
  if (CurrentTsc >= ExpectedTsc) {
    return TRUE;
  }

  return FALSE;
}

EFI_STATUS
GetNextBlockedCpuNumber (
  OUT UINTN                               *NextCpuNumber
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  NextCpuNumber - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_NOT_FOUND - GC_TODO: Add description for return value

--*/
{
  UINTN           CpuNumber;
  CPU_STATE       CpuState;
  CPU_DATA_BLOCK  *CpuData;

  for (CpuNumber = 0; CpuNumber < mMPSystemData->NumberOfCpus; CpuNumber++) {
    if (CpuNumber == mMPSystemData->BSP) {
      continue;
    }

    CpuData = &mMPSystemData->CpuData[CpuNumber];

    AsmAcquireMPLock (&CpuData->StateLock);
    CpuState = CpuData->State;
    AsmReleaseMPLock (&CpuData->StateLock);

    if (CpuState == CPU_STATE_BLOCKED) {
      *NextCpuNumber = CpuNumber;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

VOID
UpdateDataforDatahub (
  IN  UINTN                           CpuNumber,
  OUT CPU_DATA_FOR_DATAHUB            *CpuDataforDatahub
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber         - GC_TODO: add argument description
  CpuDataforDatahub - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  CpuDataforDatahub->Status = GetProcessorStatus (CpuNumber);
  return ;
}

VOID
WakeUpAp (
  IN   CPU_DATA_BLOCK        *CpuData,
  IN   EFI_AP_PROCEDURE      Procedure,
  IN   VOID                  *ProcArguments
  )
/*++

Routine Description:

  Function to wake up a specified AP and assign procedure to it.

Arguments:

  CpuData       - CPU data block for the specified AP.
  Procedure     - Procedure to assign.
  ProcArguments - Argument for Procedure.

Returns:

  None

--*/
{
  AsmAcquireMPLock (&CpuData->ProcedureLock);
  CpuData->Parameter  = ProcArguments;
  CpuData->Procedure  = Procedure;
  AsmReleaseMPLock (&CpuData->ProcedureLock);

  SendInterrupt (
    BROADCAST_MODE_SPECIFY_CPU,
    CpuData->ApicID,
    0,
    DELIVERY_MODE_INIT,
    TRIGGER_MODE_EDGE,
    TRUE
    );
  SendInterrupt (
    BROADCAST_MODE_SPECIFY_CPU,
    CpuData->ApicID,
    (UINT32) RShiftU64 (mAcpiCpuData->WakeUpBuffer, 12),
    DELIVERY_MODE_SIPI,
    TRIGGER_MODE_EDGE,
    TRUE
    );
  CpuData->StateLock = 0;
}

BOOLEAN
ApRunning (
  VOID
  )
/*++

Routine Description:

  Check whether any AP is running for assigned task.

Arguments:

  None

Returns:

  TRUE  - Some APs are running.
  FALSE - No AP is running.

--*/
{
  CPU_DATA_BLOCK  *CpuData;
  UINTN           CpuNumber;

  for (CpuNumber = 0; CpuNumber < mMPSystemData->NumberOfCpus; CpuNumber++) {
    CpuData = &mMPSystemData->CpuData[CpuNumber];

    if (CpuNumber != mMPSystemData->BSP) {
      if (CpuData->State == CPU_STATE_READY || CpuData->State == CPU_STATE_BUSY) {
        return TRUE;
      }
    }
  }
  return FALSE;
}

VOID
LegacyRegionAPCount (
  VOID
  )
/*++

Routine Description:

  Count the number of APs that have been switched
  to E0000 or F0000 segments by ReAllocateMemoryForAP().

Arguments:

  None.

Returns:

  None;

--*/
{
  AsmAcquireMPLock (&mMPSystemData->APSerializeLock);

  //
  // Initialize MCE for CR4.
  //
  InitializeMce (mMPSystemData->MachineCheckEnable);

  mSwitchToLegacyRegionCount++;

  AsmReleaseMPLock (&mMPSystemData->APSerializeLock);
}




EFI_STATUS
R9WhoAmI (
  IN  EFI_MP_SERVICES_PROTOCOL            *This,
  OUT UINTN                               *CpuNumber
  )
{
  return WhoAmI(NULL, CpuNumber);
}

EFI_STATUS
R9EnableDisableAP (
  IN  EFI_MP_SERVICES_PROTOCOL            * This,
  IN  UINTN                               CpuNumber,
  IN  BOOLEAN                             NewAPState,
  IN  UINT32                              *HealthFlag OPTIONAL
  )
{
  EFI_MP_HEALTH  HealthState;
  EFI_STATUS Status;
  if (HealthFlag == NULL) {
    return EnableDisableAP(NULL, CpuNumber, NewAPState, NULL);
  }
  Status = EnableDisableAP(NULL, CpuNumber, NewAPState, &HealthState);
  if (HealthState.Flags.Bits.Status == EFI_MP_HEALTH_FLAGS_STATUS_HEALTHY) {
    * HealthFlag = PROCESSOR_HEALTH_STATUS_BIT;
  } else {
    * HealthFlag = 0;
  }
  return Status;
}

EFI_STATUS
R9SwitchBSP (
  IN  EFI_MP_SERVICES_PROTOCOL            *This,
  IN  UINTN                               CpuNumber,
  IN  BOOLEAN                             EnableOldBSP
  )
{
  return SwitchBSP(NULL, CpuNumber, EnableOldBSP);
}

EFI_STATUS
R9StartupThisAP (
  IN       EFI_MP_SERVICES_PROTOCOL                        *This,
  IN       EFI_AP_PROCEDURE                                Procedure,
  IN       UINTN                                           CpuNumber,
  IN       EFI_EVENT                                       WaitEvent OPTIONAL,
  IN       UINTN                                           TimeoutInMicroSecs OPTIONAL,
  IN OUT   VOID                                            *ProcArguments OPTIONAL,
  OUT      BOOLEAN                                         *Finished      OPTIONAL
  )
{
  EFI_STATUS      Status;
  CPU_DATA_BLOCK  *CpuData;
  UINT64          ExpectedTime;

  //
  // Check for incoming junk parameters.
  //
  if ((CpuNumber >= mMPSystemData->NumberOfCpus) || CpuNumber == mMPSystemData->BSP) {
    return EFI_INVALID_PARAMETER;
  }

  if (Procedure == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *Finished = FALSE;
  CpuData = &mMPSystemData->CpuData[CpuNumber];

  //
  // As a first step, check if processor is OK to start up code stream.
  //
  if (CpuData->State != CPU_STATE_IDLE) {
    return EFI_INVALID_PARAMETER;
  }

  ExpectedTime                = CalculateTimeout (TimeoutInMicroSecs);

  mMPSystemData->StartCount   = 1;
  mMPSystemData->FinishCount  = 0;

  WakeUpAp (
    CpuData,
    Procedure,
    ProcArguments
  );

  //
  // If WaitEvent is NULL, execute in blocking mode.
  // BSP checks AP's state until it finishes or TimeoutInMicrosecsond expires.
  //
  mMPSystemData->StartedCpuNumber = CpuNumber;
  mMPSystemData->WaitEvent        = WaitEvent;

  if (WaitEvent == NULL) {
    while (TRUE) {
      AsmAcquireMPLock (&CpuData->StateLock);
      if (CpuData->State == CPU_STATE_FINISHED) {
        CpuData->State = CPU_STATE_IDLE;
        AsmReleaseMPLock (&CpuData->StateLock);
        break;
      }

      AsmReleaseMPLock (&CpuData->StateLock);

      if (CheckTimeout (ExpectedTime)) {
        //
        // Save data into private data structure, and create timer to poll AP state before exiting
        //
        Status = gBS->SetTimer (
                        mMPSystemData->CheckThisAPEvent,
                        TimerPeriodic,
                        CPU_CHECK_AP_INTERVAL * MICROSECOND
                        );
        return EFI_TIMEOUT;
      }

      gBS->Stall (CPU_CHECK_AP_INTERVAL * MICROSECOND);
    }
    return EFI_SUCCESS;
  }
  //
  // If WaitEvent is not NULL, execute in non-blocking mode.
  // BSP checks AP's state until it finishes or TimeoutInMicrosecsond expires.
  //
	Status = gBS->SetTimer (
                    mMPSystemData->CheckThisAPEvent,
                    TimerPeriodic,
                    CPU_CHECK_AP_INTERVAL * MICROSECOND
                    );
  *Finished = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS
R9StartupAllAPs (
  IN  EFI_MP_SERVICES_PROTOCOL                               *This,
  IN  EFI_AP_PROCEDURE                                       Procedure,
  IN  BOOLEAN                                                SingleThread,
  IN  EFI_EVENT                                              WaitEvent OPTIONAL,
  IN  UINTN                                                  TimeoutInMicroSecs OPTIONAL,
  IN  OUT VOID                                               *ProcArguments OPTIONAL,
  OUT UINTN                                                  **FailedCPUList OPTIONAL
  )
{
  EFI_STATUS  Status;
  UINTN       *List;
  UINTN       Index;
  if (FailedCPUList == NULL) {
    List = NULL;
  } else {
#ifdef ECP_FLAG
    Status = (gBS->AllocatePool) (EfiBootServicesData, mMPSystemData->NumberOfCpus * sizeof(UINTN), (VOID **) &List);
#else
    Status = gBS->AllocatePool (EfiBootServicesData, mMPSystemData->NumberOfCpus * sizeof(UINTN), (VOID **) &List);
#endif
    for (Index = 0; Index < mMPSystemData->NumberOfCpus; Index ++) {
      List[Index] = END_OF_CPU_LIST;
    }
  }
  Status = StartupAllAPs(NULL, Procedure, SingleThread, WaitEvent, TimeoutInMicroSecs, ProcArguments, List);
  if (FailedCPUList != NULL) {
    if (List[0] == END_OF_CPU_LIST) {
#ifdef ECP_FLAG
      (gBS->FreePool) (List);
#else
      gBS->FreePool (List);
#endif
      *FailedCPUList = NULL;
    } else {
      *FailedCPUList = List;
    }
  }
  return Status;
}
EFI_STATUS

R9GetNumberOfProcessors (
  IN  EFI_MP_SERVICES_PROTOCOL     *This,
  OUT UINTN                        *NumberOfProcessors,
  OUT UINTN                        *NumberOfEnabledProcessors
  )
{
  return GetGeneralMPInfo(NULL, NumberOfProcessors, NULL, NumberOfEnabledProcessors, NULL, NULL);
}

EFI_STATUS
R9GetProcessorInfo(
  IN       EFI_MP_SERVICES_PROTOCOL     *This,
  IN       UINTN                        CpuNumber,
  OUT      EFI_PROCESSOR_INFORMATION    *ProcessorInfoBuffer
  )
{
  CPU_DATA_BLOCK      *CpuData;


  if ((mMPSystemData->NumberOfCpus <= CpuNumber) || (ProcessorInfoBuffer == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  CpuData                            = &mMPSystemData->CpuData[CpuNumber];


  ProcessorInfoBuffer->ProcessorId   = CpuData->ApicID;

  ProcessorInfoBuffer->StatusFlag    = PROCESSOR_ENABLED_BIT;
  if (!mMPSystemData->EnableSecondaryCpu) {
    if (CpuData->SecondaryCpu) {
      ProcessorInfoBuffer->StatusFlag = 0;
    }
  }

  if (CpuData->State == CPU_STATE_DISABLED) {
    ProcessorInfoBuffer->StatusFlag = 0;
  }

  if (CpuNumber == mMPSystemData->BSP) {
    ProcessorInfoBuffer->StatusFlag |= PROCESSOR_AS_BSP_BIT;
  }

  if (CpuData->Health.Bits.Status == EFI_MP_HEALTH_FLAGS_STATUS_HEALTHY) {
    ProcessorInfoBuffer->StatusFlag |= PROCESSOR_HEALTH_STATUS_BIT;
  }

  ProcessorInfoBuffer->Location.Core = CpuData->CpuDataforDatahub.Location.Core;
  ProcessorInfoBuffer->Location.Package = CpuData->CpuDataforDatahub.Location.Package;
  ProcessorInfoBuffer->Location.Thread = CpuData->CpuDataforDatahub.Location.Thread;

  return EFI_SUCCESS;
}

VOID
InstallR9MPService()
{
  EFI_STATUS Status;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mHandle,
                  &gEfiMpServiceProtocolGuid,
                  &mMpService,
                  NULL
                  );
  ASSERT_EFI_ERROR(Status);
}
