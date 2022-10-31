/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  LeastFeature.c

Abstract:
  Select Processor with least feature as BSP

Revision History

--*/

#include "CpuDxe.h"
#include "PlatformMpService.h"

extern EFI_PLATFORM_CPU_PROTOCOL  *mPlatformCpu;
extern EFI_PLATFORM_CPU_INFO      mPlatformCpuInfo;

static UINT8                mLock;
static LEAST_FEATURE_PROC   mLeastFeatureProcessor;

UINT32
GetBitsNumberOfOne (
  UINT32  Value
  )
{
  UINT32  Result;

  Result = 0;
  while (Value) {
    if (Value & 1) {
      Result ++;
    }
    Value >>= 1;
  }
  return Result;
}

VOID
GetProcessorFeatures (
  IN  UINT32    *Features
  )
{
  Features[0] = mPlatformCpuInfo.CpuFeatures.Regs.RegEcx;
  Features[1] = mPlatformCpuInfo.CpuFeatures.Regs.RegEdx;
  Features[2] = mPlatformCpuInfo.CpuFeatures.ExtRegs.RegEax;
  Features[3] = mPlatformCpuInfo.CpuFeatures.ExtRegs.RegEbx;
  Features[4] = mPlatformCpuInfo.CpuFeatures.ExtRegs.RegEcx;
  Features[5] = mPlatformCpuInfo.CpuFeatures.ExtRegs.RegEdx;
}

VOID
UpdateProcessorInfo (
  IN UINTN                 Index,
  IN LEAST_FEATURE_PROC   *LeastFeatureProcessor
  )
{
  LeastFeatureProcessor->Index        =  Index;
  LeastFeatureProcessor->ApicId       =  GetApicID (NULL, NULL);
  LeastFeatureProcessor->Version      =  EfiMakeCpuVersion (mPlatformCpuInfo.CpuVersion.FamilyId, \
                                           mPlatformCpuInfo.CpuVersion.Model, mPlatformCpuInfo.CpuVersion.SteppingId); 
}

UINT32
GetProcessorFeatureDelta (
  IN  UINT32    *FeaturesInput,
  IN  UINT32    *CommonFeatures
  )
{
  UINT32   Delta;
  UINTN    Index;

  //
  // CommonFeatures is the subset of FeaturesInput
  //
  Delta = 0;
  for (Index = 0; Index < MAX_FEATURE_NUM; Index++) {
    Delta += GetBitsNumberOfOne (FeaturesInput[Index] - CommonFeatures[Index]);
  }

  return 0;
}

VOID
GetProcessorCommonFeature (
  IN OUT VOID  *Buffer
  )
{
  UINTN    Index;
  UINT32   Features[MAX_FEATURE_NUM];

  GetProcessorFeatures (Features);

  AsmAcquireMPLock (&mLock);
  for (Index = 0; Index < MAX_FEATURE_NUM; Index++) {
    mLeastFeatureProcessor.Features[Index] &= Features[Index];
  }
  AsmReleaseMPLock (&mLock);
}

VOID
GetProcessorWithLeastFeature (
  IN  EFI_MP_SERVICES_PROTOCOL             *MpServices
  )
{
  EFI_STATUS          Status;
  UINTN               CurrentProcessor;
  LEAST_FEATURE_PROC  LeastFeatureProcessor;

  Status = MpServices->WhoAmI (
                        MpServices,
                        &CurrentProcessor
                        );
  if (EFI_ERROR (Status)) {
    return;
  }

  GetProcessorFeatures (LeastFeatureProcessor.Features);
  LeastFeatureProcessor.FeatureDelta = GetProcessorFeatureDelta (LeastFeatureProcessor.Features, mLeastFeatureProcessor.Features);

  AsmAcquireMPLock (&mLock);
  if (LeastFeatureProcessor.FeatureDelta < mLeastFeatureProcessor.FeatureDelta) {
    mLeastFeatureProcessor.FeatureDelta = LeastFeatureProcessor.FeatureDelta;
    UpdateProcessorInfo (CurrentProcessor, &mLeastFeatureProcessor);
  } else if (LeastFeatureProcessor.FeatureDelta == mLeastFeatureProcessor.FeatureDelta) {
    UpdateProcessorInfo (CurrentProcessor, &LeastFeatureProcessor);
    if (LeastFeatureProcessor.Version < mLeastFeatureProcessor.Version) {
      UpdateProcessorInfo (CurrentProcessor, &mLeastFeatureProcessor);
    } else if (LeastFeatureProcessor.Version == mLeastFeatureProcessor.Version) {
      if (LeastFeatureProcessor.ApicId < mLeastFeatureProcessor.ApicId) {
        UpdateProcessorInfo (CurrentProcessor, &mLeastFeatureProcessor);
      }
    }
  }
  AsmReleaseMPLock (&mLock);
}

EFI_STATUS
SwitchToLowestFeatureProcess (
  IN EFI_MP_SERVICES_PROTOCOL               *MpServices
  )
{
  EFI_STATUS          Status;
  UINTN               CurrentProcessor;
  UINTN               NewBsp;
  UINT32              Features[MAX_FEATURE_NUM];

  Status = MpServices->WhoAmI (
                        MpServices,
                        &CurrentProcessor
                        );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Take current BSP as the least feature
  //
  UpdateProcessorInfo  (CurrentProcessor, &mLeastFeatureProcessor);
  GetProcessorFeatures (mLeastFeatureProcessor.Features);
  CopyMem              (Features, mLeastFeatureProcessor.Features, sizeof(Features));
  Status = MpServices->StartupAllAPs (
                        MpServices,
                        GetProcessorCommonFeature,
                        TRUE,
                        NULL,
                        100000,
                        (VOID *) MpServices,
                        NULL
                        );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Go through all processors to find the processor with least features
  //
  mLeastFeatureProcessor.FeatureDelta = GetProcessorFeatureDelta (Features, mLeastFeatureProcessor.Features);
  Status = MpServices->StartupAllAPs (
                        MpServices,
                        (EFI_AP_PROCEDURE) GetProcessorWithLeastFeature,
                        TRUE,
                        NULL,
                        100000,
                        (VOID *) MpServices,
                        NULL
                        );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Switch BSP according
  //
  if (mPlatformCpu->BspSelection == 16) {
    //
    // Enable least feature SBSP selection
    //
    NewBsp = mLeastFeatureProcessor.Index;
  } else {
    //
    // Do not change the current BSP
    // made by SEC
    //
    NewBsp = CurrentProcessor;
  }

  if (NewBsp != CurrentProcessor) {

    DEBUG ((EFI_D_ERROR, "Switch BSP from %d ==> %d\n", CurrentProcessor, NewBsp));
    Status = MpServices->SwitchBSP (
                          MpServices,
                          NewBsp,
                          TRUE
                          );
  }

  return Status;
}
