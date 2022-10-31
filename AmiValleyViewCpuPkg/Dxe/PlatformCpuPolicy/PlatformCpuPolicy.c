/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved

This source code and any documentation accompanying it ("Material") is furnished
under license and may only be used or copied in accordance with the terms of that
license.  No license, express or implied, by estoppel or otherwise, to any
intellectual property rights is granted to you by disclosure or delivery of these
Materials.  The Materials are subject to change without notice and should not be
construed as a commitment by Intel Corporation to market, license, sell or support
any product or technology.  Unless otherwise provided for in the license under which
this Material is provided, the Material is provided AS IS, with no warranties of
any kind, express or implied, including without limitation the implied warranties
of fitness, merchantability, or non-infringement.  Except as expressly permitted by
the license for the Material, neither Intel Corporation nor its suppliers assumes
any responsibility for any errors or inaccuracies that may appear herein.  Except
as expressly permitted by the license for the Material, no part of the Material
may be reproduced, stored in a retrieval system, transmitted in any form, or
distributed by any means without the express written consent of Intel Corporation.

Module Name:
  PlatformCpuPolicy.c

Abstract:
  CPU Platform Policy Driver.

 
--*/

#include "CpuType.h"
typedef struct {
  UINT32  RegEax;
  UINT32  RegEbx;
  UINT32  RegEcx;
  UINT32  RegEdx;
} EFI_CPUID_REGISTER;

#include "Guid/AcpiVariableCompatibility.h"
#include "CpuDataStruct.h"
#include <Guid/PlatformCpuInfo.h>
#include "PlatformCpuPolicy.h"

#ifdef EFI_EVENT_RUNTIME_CONTEXT
#undef EFI_EVENT_RUNTIME_CONTEXT
#endif
#include <Library/CpuPolicy.h>
//#include <SetupDataDefinition.h>
#include <Library/IoLib.h>
#include <token.h>
#include <PchRegs.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <AmiDxeLib.h>
#define PROD_MICROCODE_GUID \
  { 0xa85bd0ff, 0xbf4d, 0x4fc2, 0x9f, 0x12, 0xf6, 0x75, 0xaf, 0xa3, 0xd9, 0x76 }

#define PLATFORM_CPU_MAX_CORE_MHZ 4000
#define PLATFORM_CPU_MAX_FSB_MHZ  1066

extern EFI_GUID gEfiSetupVariableGuid;

//
// Global variables
//
PLATFORM_CPU_POLICY_INSTANCE  mPlatformCpuPrivateData;
VOID                          *mEfiCpuPolicyNotifyReg;
EFI_EVENT                     mEfiCpuPolicyEvent;
//CPU_SETUP_DATA                 *mSysCfg;
//SETUP_DATA                    mSysCfg;
UINT32                        mMaxThreadsPerCore;
UINT32                        mMaxCoresPerDie;
UINT32                        mMaxDiesPerPackage;
UINT32                        mMaxPackages;
UINTN                         mFvHandleIndex;
UINTN                         mFvProtocolCount;
EFI_HANDLE                    *mFvHandles;
VOID                          *mFileKey;
BOOLEAN                       mFirstTime = TRUE;

//
// List of APIC IDs assign to each processor package.
// The lenght of this table determine also the max possible processors in the system.
//
UINT8  mPackageApicNo[] = { 0x00, 0x04 };

static EFI_EXP_BASE10_DATA  mCoreFrequencyList[] = {
  { 0,    0 },  // 0 Means "Auto", also, the first is the default.
  { 2000, 6 },
  { 2200, 6 },
  { 2266, 6 },
  { 2400, 6 },
  { 2533, 6 },
  { 2800, 6 },
  { 3000, 6 },
  { 3060, 6 },
  { 3200, 6 },
  { 3400, 6 },
  { 3600, 6 },
  { -1,   0 } // End marker
};

static EFI_EXP_BASE10_DATA  mFsbFrequencyList[] = {
  { 0,    0 },  // 0 Means "Auto", also, the first is the default.
  { 400,  6 },
  { 533,  6 },
  { 800,  6 },
  { 1033, 6 },
  { -1,   0 } // End marker
};

VOID *
EFIAPI
ZeroMem (
  OUT VOID  *Buffer,
  IN UINTN  Length
  );

//
// Function prototypes
//
EFI_STATUS
PlatformCpuRetrieveMicrocode (
  IN  EFI_PLATFORM_CPU_PROTOCOL       *This,
  OUT UINT8                           **MicrocodeData
  )
/*++

Routine Description:

  Get the microcode patch.

Arguments:

  This      - Driver context.
  MicrocodeData - Retrieved image of the microcode.

Returns:

  EFI_SUCCESS - Image found.
  EFI_NOT_FOUND - Image not found.

--*/
{
  EFI_CPU_MICROCODE_HEADER *Microcode;
  UINTN                    MicrocodeStart;
  UINTN                    MicrocodeEnd;
  UINTN                    TotalSize;

  MicrocodeStart =(UINTN)PcdGet32 (PcdFlashMicroCodeAddress)  +
          ((EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) PcdGet32 (PcdFlashMicroCodeAddress))->HeaderLength +
          sizeof (EFI_FFS_FILE_HEADER);

  MicrocodeEnd = (UINTN)PcdGet32 (PcdFlashMicroCodeAddress) + MICROCODE_SIZE;

  if (*MicrocodeData == NULL) {
    *MicrocodeData = (UINT8 *) (UINTN) MicrocodeStart;
  } else {
    if (*MicrocodeData < (UINT8 *) (UINTN) MicrocodeStart) {
      return EFI_NOT_FOUND;
    }

    TotalSize = (UINTN) (((EFI_CPU_MICROCODE_HEADER *) *MicrocodeData)->TotalSize);
    if (TotalSize == 0) {
      TotalSize = 2048;
    }
    //
    // Add alignment check - begin
    //
    if ((TotalSize & 0x7FF) != 0) {
      TotalSize = (TotalSize & 0xFFFFF800) + 0x800;
    }
    //
    // Add alignment check - end
    //
    *MicrocodeData += TotalSize;
    Microcode = (EFI_CPU_MICROCODE_HEADER *) *MicrocodeData;
    if (*MicrocodeData >= (UINT8 *) (UINTN) (MicrocodeEnd) || Microcode->TotalSize == (UINT32) -1) {
      return EFI_NOT_FOUND;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PlatformCpuStall (
  IN  EFI_PLATFORM_CPU_PROTOCOL  *This,
  IN  UINTN                       Microseconds
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This              PlatfromCpu protocol instance
  Microseconds      Desired length of time to wait.

Returns:

  EFI_SUCCESS       - Function has been executed correctly

--*/
{
  UINTN   Ticks;
  UINTN   Counts;
  UINT32  CurrentTick;
  UINT32  OriginalTick;
  UINT32  RemainingTick;

  if (Microseconds == 0) {
    return EFI_SUCCESS;
  }

  OriginalTick = IoRead32 (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_TMR);
  OriginalTick &= (V_PCH_ACPI_PM1_TMR_MAX_VAL - 1);
  CurrentTick = OriginalTick;

  //
  // The timer frequency is 3.579545MHz, so 1 ms corresponds to 3.58 clocks
  //
  Ticks = Microseconds * 358 / 100 + OriginalTick + 1;

  //
  // The loops needed for timer overflow
  //
  Counts = Ticks / V_PCH_ACPI_PM1_TMR_MAX_VAL;

  //
  // Remaining clocks within one loop
  //
  RemainingTick = (UINT32)(Ticks % V_PCH_ACPI_PM1_TMR_MAX_VAL);

  //
  // Do not intend to use TMROF_STS bit of register PM1_STS, because this add extra
  // one I/O operation, and may generate SMI
  //
  while (Counts != 0) {
    CurrentTick = IoRead32 (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_TMR);
    CurrentTick &= (V_PCH_ACPI_PM1_TMR_MAX_VAL - 1);
    if (CurrentTick <= OriginalTick) {
      Counts--;
    }

    OriginalTick = CurrentTick;
  }

  while ((RemainingTick > CurrentTick) && (OriginalTick <= CurrentTick)) {
    OriginalTick  = CurrentTick;
    CurrentTick   = IoRead32 (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_TMR);
    CurrentTick &= (V_PCH_ACPI_PM1_TMR_MAX_VAL - 1);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PlatformCpuGetTm2ControlInfo (
  IN  EFI_PLATFORM_CPU_PROTOCOL       *This,
  OUT UINT8                 *Tm2Core2BusRatio,
  OUT UINT8                 *Tm2Vid
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This        - GC_TODO: add argument description
  Tm2Core2BusRatio  - GC_TODO: add argument description
  Tm2Vid      - GC_TODO: add argument description

Returns:

  EFI_UNSUPPORTED - GC_TODO: Add description for return value

--*/
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
PlatformCpuGetMaxCount (
  IN  EFI_PLATFORM_CPU_PROTOCOL  *This,
  IN OUT  UINT32                 *MaxThreadsPerCore,
  IN OUT  UINT32                 *MaxCoresPerDie,
  IN OUT  UINT32                 *MaxDiesPerPackage,
  IN OUT  UINT32                 *MaxPackages
  )
/*++

Routine Description:
  Returns the maximum number of threads, cores, dies and processor packages the system supports.

Arguments:
  This        - A pointer to the EFI_PLATFORM_MP_PROTOCOL instance.
  MaxThreadsPerCore - Pointer to the Maximum number of threads per each CPU Core.
  MaxCoresPerDie    - Pointer to the Maximum number of cores per each CPU Die.
  MaxDiesPerPackage - Pointer to the Maximum number of Dies per each CPU Package.
  MaxPackages     - Pointer to the Maximum number of CPU packages the system can accommodate.

Returns:
  EFI_SUCCESS     Always.
  ASSERT () in case of errors.

--*/
{
  *MaxThreadsPerCore  = mMaxThreadsPerCore;
  *MaxCoresPerDie     = mMaxCoresPerDie;
  *MaxDiesPerPackage  = mMaxDiesPerPackage;
  *MaxPackages        = mMaxPackages;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PlatformCpuGetCpuInfo (
  IN   EFI_PLATFORM_CPU_PROTOCOL       *This,
  IN   EFI_CPU_PHYSICAL_LOCATION       *Location,
  OUT EFI_PLATFORM_CPU_INFORMATION     *PlatformCpuInfo
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This        - GC_TODO: add argument description
  Location      - GC_TODO: add argument description
  PlatformCpuInfo - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  ASSERT (Location->Package < mMaxPackages);

  DEBUG (
  (EFI_D_ERROR,
  "Processor: Pkg/Core/Thrd: %d/%d/%d",
  (UINTN) Location->Package,
  (UINTN) Location->Core,
  (UINTN) Location->Thread)
  );

  DEBUG ((EFI_D_ERROR, "  ApicId: %x\n", (UINTN) PlatformCpuInfo->ApicID));

  //
  // Max Frequency supported by this platform. Depends on the chipset.
  //
  PlatformCpuInfo->MaxCoreFrequency.Value     = PLATFORM_CPU_MAX_CORE_MHZ;
  PlatformCpuInfo->MaxCoreFrequency.Exponent  = 6;

  PlatformCpuInfo->MaxFsbFrequency.Value      = PLATFORM_CPU_MAX_FSB_MHZ;
  PlatformCpuInfo->MaxFsbFrequency.Exponent   = 6;

  //
  // List of Frequencies supported
  //
  PlatformCpuInfo->PlatformCoreFrequencyList  = mCoreFrequencyList;
  PlatformCpuInfo->PlatformFsbFrequencyList   = mFsbFrequencyList;

  //TODO PNV: Need to update the socket type in the future. By MengWei 07Jan2009
  PlatformCpuInfo->SocketType         = EfiProcessorSocketNone;
  PlatformCpuInfo->SocketName         = 0;

  PlatformCpuInfo->ReferenceString    = 0;  // STRING_TOKEN(STR_UNKNOWN);
  PlatformCpuInfo->AssetTag           = 0;  // STRING_TOKEN(STR_UNKNOWN);
  return EFI_SUCCESS;
}

//EFI_DRIVER_ENTRY_POINT (PlatformCpuPolicyEntryPoint)

EFI_STATUS
PlatformCpuPolicyEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

  Routine Description:

  This is the EFI driver entry point for the CpuPolicy Driver. This
  driver is responsible for getting microcode patches from FV.

  Arguments:

  ImageHandle   - Handle for the image of this driver.
  SystemTable   - Pointer to the EFI System Table.

  Returns:

  EFI_SUCCESS   - Protocol installed sucessfully.

--*/
{
  EFI_STATUS                Status;
//  UINTN                     SysCfgSize;
  EFI_PLATFORM_CPU_INFO     *PlatformCpuInfoPtr;
  EFI_PEI_HOB_POINTERS      GuidHob;
  CPU_SETUP_DATA            *VlvCpuPolicyData;
  
  InitAmiLib(ImageHandle, SystemTable);

  //
  // Read the current system configuration variable store.
  //
/*  
  SysCfgSize = sizeof (SETUP_DATA);
  Status = gRT->GetVariable (
                  L"Setup",
                  &gEfiSetupVariableGuid,
                  NULL, // Attributes
                  &SysCfgSize,
                  &mSysCfg
                  );
  ASSERT_EFI_ERROR (Status);
*/  
  
  VlvCpuPolicyData = (CPU_SETUP_DATA *) AllocateZeroPool (sizeof (CPU_SETUP_DATA));
  ASSERT(VlvCpuPolicyData != NULL);
  if(VlvCpuPolicyData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  // Get the value of the CPU Setup data.
  GetCpuSetupData (  (VOID*)gRT, VlvCpuPolicyData, FALSE);
  //
  // Get Platform CPU Info HOB
  //
  PlatformCpuInfoPtr = NULL;
  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
     GuidHob.Raw = GetNextGuidHob (&gEfiPlatformCpuInfoGuid, GuidHob.Raw);
     if (GuidHob.Raw != NULL) {
        PlatformCpuInfoPtr = GET_GUID_HOB_DATA (GuidHob.Guid);
        Status = EFI_SUCCESS;
     } else {
        Status = EFI_NOT_FOUND;
     }
  } else {
      Status = EFI_NOT_FOUND;
  }

  if (EFI_ERROR(Status)) {
      return Status;
  }

  //
  // Initialize platform cpu protocol
  //
  ZeroMem (&mPlatformCpuPrivateData, sizeof (mPlatformCpuPrivateData));

  DEBUG((-1, "SetupData->EIST -%Xh\n", VlvCpuPolicyData->ProcessorEistEnable));
  DEBUG((-1, "SetupData->VT -%Xh\n", VlvCpuPolicyData->ProcessorVmxEnable));
  DEBUG((-1, "SetupData->HTD -%Xh\n", VlvCpuPolicyData->ProcessorHtMode));
  DEBUG((-1, "SetupData->XDBit -%Xh\n", VlvCpuPolicyData->ExecuteDisableBit));
  DEBUG((-1, "SetupData->LimitCpuid -%Xh\n", VlvCpuPolicyData->CpuidMaxValue));  
  DEBUG((-1, "SetupData->ActiveCoreCount -%Xh\n", VlvCpuPolicyData->ActiveProcessorCores));    
  
  
  mPlatformCpuPrivateData.Signature = PLATFORM_CPU_POLICY_SIGNATURE;
  mPlatformCpuPrivateData.Handle = NULL;

  mPlatformCpuPrivateData.PlatformCpu.RetrieveMicrocode           = PlatformCpuRetrieveMicrocode;
  mPlatformCpuPrivateData.PlatformCpu.GetMaxCount                 = PlatformCpuGetMaxCount;
  mPlatformCpuPrivateData.PlatformCpu.GetTm2ControlInfo           = PlatformCpuGetTm2ControlInfo;
  mPlatformCpuPrivateData.PlatformCpu.GetCpuInfo                  = PlatformCpuGetCpuInfo;
  mPlatformCpuPrivateData.PlatformCpu.OverridePolicy              = PlatformCpuPolicyOverridePolicy;
  mPlatformCpuPrivateData.PlatformCpu.Stall                       = PlatformCpuStall;
  
#if AES_ENABLE == 0  
  mPlatformCpuPrivateData.PlatformCpu.AesEnable                   = 0;
#else
  mPlatformCpuPrivateData.PlatformCpu.AesEnable                   = 1;
#endif

  mPlatformCpuPrivateData.PlatformCpu.ProcessorVmxEnable          = VlvCpuPolicyData->ProcessorVmxEnable;
  mPlatformCpuPrivateData.PlatformCpu.LtEnable                    = 0; //mSysCfg->LtTechnology;
  mPlatformCpuPrivateData.PlatformCpu.ProcessorBistEnable         = 0; //mSysCfg->ProcessorBistEnable;
  mPlatformCpuPrivateData.PlatformCpu.HtState                     = VlvCpuPolicyData->ProcessorHtMode ? 1 : 0;//mSysCfg->ProcessorHtMode ? 1 : 0;
  mPlatformCpuPrivateData.PlatformCpu.EchoTprDisable              = 1; //mSysCfg->ProcessorxTPRDisable;
  mPlatformCpuPrivateData.PlatformCpu.ExecuteDisableBit           = VlvCpuPolicyData->ExecuteDisableBit;
  mPlatformCpuPrivateData.PlatformCpu.CcxEnable                   = 1; //mSysCfg->ProcessorCcxEnable;
  mPlatformCpuPrivateData.PlatformCpu.C1AutoDemotion              = 0; //Disable. mSysCfg->C1AutoDemotion;
  mPlatformCpuPrivateData.PlatformCpu.C3AutoDemotion              = 0; //Disable. mSysCfg->C3AutoDemotion;
  mPlatformCpuPrivateData.PlatformCpu.PackageCState               = VlvCpuPolicyData->PackageCState;
  mPlatformCpuPrivateData.PlatformCpu.C1eEnable                   = 1; //mSysCfg->ProcessorC1eEnable;
  mPlatformCpuPrivateData.PlatformCpu.Gv3State                    = VlvCpuPolicyData->ProcessorEistEnable;//mSysCfg->ProcessorEistEnable;
  mPlatformCpuPrivateData.PlatformCpu.PsdState                    = VlvCpuPolicyData->PsdState;//mSysCfg.PsdCoordType; //mSysCfg->ProcessorEistPsdFunc;
  mPlatformCpuPrivateData.PlatformCpu.DcaState                    = 1; //mSysCfg->DcaEnable;
  mPlatformCpuPrivateData.PlatformCpu.DcaPrefetchDelayValue       = 4; //mSysCfg->DcaPrefetchDelayValue;
  mPlatformCpuPrivateData.PlatformCpu.LimitCpuidMaximumValue      = VlvCpuPolicyData->CpuidMaxValue;//mSysCfg->CpuidMaxValue;
  mPlatformCpuPrivateData.PlatformCpu.FastString                  = 1; //mSysCfg->FastStringEnable;
  mPlatformCpuPrivateData.PlatformCpu.MonitorMwaitEnable          = 1; //mSysCfg->MonitorMwaitEnable;
  mPlatformCpuPrivateData.PlatformCpu.MachineCheckEnable          = 1; //mSysCfg->MachineCheckEnable;
  mPlatformCpuPrivateData.PlatformCpu.DCUModeSelection            = 0; //32K_8way_NoEcc. mSysCfg->DCUModeSelection;
  mPlatformCpuPrivateData.PlatformCpu.BiDirectionalProchot        = 1; //Enable. mSysCfg->BiDirectionalProchot;

  mPlatformCpuPrivateData.PlatformCpu.BspSelection                = 0; //mSysCfg->BspSelection;

  mPlatformCpuPrivateData.PlatformCpu.MLCStreamerPrefetcherEnable = VlvCpuPolicyData->MlcStreamerPrefetcherEnable;
  mPlatformCpuPrivateData.PlatformCpu.MLCSpatialPrefetcherEnable  = VlvCpuPolicyData->MlcSpatialPrefetcherEnable;
  mPlatformCpuPrivateData.PlatformCpu.DCUStreamerPrefetcherEnable = VlvCpuPolicyData->DCUStreamerPrefetcherEnable;
  mPlatformCpuPrivateData.PlatformCpu.DCUIPPrefetcherEnable       = VlvCpuPolicyData->DCUIPPrefetcherEnable;

  mPlatformCpuPrivateData.PlatformCpu.TurboModeEnable             = VlvCpuPolicyData->TurboModeEnable;
  mPlatformCpuPrivateData.PlatformCpu.ExtremeEnable               = 0;//mSysCfg->ProcessorXEEnable;
  mPlatformCpuPrivateData.PlatformCpu.XapicEnable                 = VlvCpuPolicyData->ProcessorXapic;//mSysCfg->ProcessorXapic;
  mPlatformCpuPrivateData.PlatformCpu.TdcLimitOverride            = VlvCpuPolicyData->ProcessorTDCLimitOverrideEnable;
  mPlatformCpuPrivateData.PlatformCpu.TdcLimit                    = VlvCpuPolicyData->ProcessorTDCLimit;//mSysCfg->ProcessorTDCLimit;
  mPlatformCpuPrivateData.PlatformCpu.TdpLimitOverride            = VlvCpuPolicyData->ProcessorTDPLimitOverrideEnable;
  mPlatformCpuPrivateData.PlatformCpu.TdpLimit                    = VlvCpuPolicyData->ProcessorTDPLimit;//mSysCfg->ProcessorTDPLimit;
  mPlatformCpuPrivateData.PlatformCpu.RatioLimit1C                = VlvCpuPolicyData->RatioLimit1C;
  mPlatformCpuPrivateData.PlatformCpu.RatioLimit2C                = VlvCpuPolicyData->RatioLimit2C;
  mPlatformCpuPrivateData.PlatformCpu.RatioLimit3C                = VlvCpuPolicyData->RatioLimit3C;
  mPlatformCpuPrivateData.PlatformCpu.RatioLimit4C                = VlvCpuPolicyData->RatioLimit4C;
  mPlatformCpuPrivateData.PlatformCpu.VirtualWireMode             = VlvCpuPolicyData->ProcessorVirtualWireMode;
  mPlatformCpuPrivateData.PlatformCpu.Vr11Enable                  = 0; //Disable. mSysCfg.Vr11Enable;

  mPlatformCpuPrivateData.PlatformCpu.ProcessorMsrLockControl     = 1;
  mPlatformCpuPrivateData.PlatformCpu.Processor3StrikeControl     = 0;

  mPlatformCpuPrivateData.PlatformCpu.PlatformCategory            = EfiPlatformDesktop;

  mPlatformCpuPrivateData.PlatformCpu.EnableL3Cache               = 1;
  mPlatformCpuPrivateData.PlatformCpu.CpuCommonFeature            = 0;
  mPlatformCpuPrivateData.PlatformCpu.ActiveProcessorCores        = VlvCpuPolicyData->ActiveProcessorCores; //mSysCfg.ActiveProcessorCores;

  mMaxThreadsPerCore = PlatformCpuInfoPtr->CpuPackage.ThreadsPerCore;
  mMaxCoresPerDie = PlatformCpuInfoPtr->CpuPackage.CoresPerPhysicalPackage;
  mMaxDiesPerPackage = PlatformCpuInfoPtr->CpuPackage.LogicalProcessorsPerPhysicalPackage;
  mMaxPackages = PlatformCpuInfoPtr->CpuPackage.PhysicalPackages;

  //LP-Debug ASSERT (mMaxPackages == sizeof (mPackageApicNo) / sizeof (mPackageApicNo[0]));

  //
  // Install the protcol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mPlatformCpuPrivateData.Handle,
                  &gEfiPlatformCpuProtocolGuid,
                  &mPlatformCpuPrivateData.PlatformCpu,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
