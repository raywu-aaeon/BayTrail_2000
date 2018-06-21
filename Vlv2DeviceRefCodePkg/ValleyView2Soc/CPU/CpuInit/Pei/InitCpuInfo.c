/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    InitCpuInfo.c

Abstract:

    Initialize the database specific to current processor.



--*/

#include "PeiProcessor.h"
//#include "VlvCommonDefinitions.h"


//hphang >> For Simics testing, temporary disable SMRR
#define SMRR_SUPPORT 1
//hphang << For Simics testing, temporary disable SMRR

#define LYNNFIELD_B0D0F0_DID      0xD134 // *Field desktop
#define CLARKSFIELD_B0D0F0_DID    0xD130 // *Field mobile
#define HAVENDALE_B0D0F0_DID      0x0040 // *Dale desktop
#define AUBURNDALE_B0D0F0_DID     0x0044 // *Dale mobile

VOID
EFIAPI
EfiCpuVersion (
  IN  OUT UINT16  *FamilyId,    OPTIONAL
  IN  OUT UINT8   *Model,       OPTIONAL
  IN  OUT UINT8   *SteppingId,  OPTIONAL
  IN  OUT UINT8   *Processor    OPTIONAL
  )
/*++

Routine Description:
  Extract CPU detail version infomation

Arguments:
  FamilyId   - FamilyId, including ExtendedFamilyId
  Model      - Model, including ExtendedModel
  SteppingId - SteppingId
  Processor  - Processor

--*/
{
  EFI_CPUID_REGISTER Register;
  UINT8              TempFamilyId;

  AsmCpuid (EFI_CPUID_VERSION_INFO, &Register.RegEax, &Register.RegEbx, &Register.RegEcx, &Register.RegEdx);

  if (SteppingId != NULL) {
    *SteppingId = (UINT8) (Register.RegEax & 0xF);
  }

  if (Processor != NULL) {
    *Processor = (UINT8) ((Register.RegEax >> 12) & 0x3);
  }

  if (Model != NULL || FamilyId != NULL) {
    TempFamilyId = (UINT8) ((Register.RegEax >> 8) & 0xF);

    if (Model != NULL) {
      *Model = (UINT8) ((Register.RegEax >> 4) & 0xF);
      if (TempFamilyId == 0x6 || TempFamilyId == 0xF) {
        *Model = (UINT8) (*Model  | ((Register.RegEax >> 12) & 0xF0));
      }
    }

    if (FamilyId != NULL) {
      *FamilyId = TempFamilyId;
      if (TempFamilyId == 0xF) {
        *FamilyId = (UINT8 ) (*FamilyId + (UINT16) ((Register.RegEax >> 20) & 0xFF));
      }
    }
  }
}


VOID
EfiCpuType (
  IN OUT EFI_PLATFORM_CPU_INFO    *pPlatformCpuInfo
  )
/*++

Routine Description:
  Executes CPUID.1.EAX and stores CPU type, architecture, platform, etc.

  IMPORTANT: All CPU type detections should be done here to ensure no code
             duplication and detection confusion.

Arguments:
  pPlatformCpuInfo  - Ptr to Platform CPU Info structure

--*/
{
  UINT16              FamilyId;
  UINT8               Model;
  UINT8               SteppingId;
  UINT8               ProcessorType;
  UINT8               Index, Index1, Index2;
  EFI_CPUID_REGISTER  CpuIdRegister;
  EFI_CPUID_REGISTER  CpuBrandString[3];
  UINT8               *PtrCpuBrandString;

  AsmCpuid (EFI_CPUID_SIGNATURE, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx, &CpuIdRegister.RegEdx);
  pPlatformCpuInfo->CpuidMaxInputValue = CpuIdRegister.RegEax;

  AsmCpuid (EFI_CPUID_EXTENDED_FUNCTION, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx, &CpuIdRegister.RegEdx);
  pPlatformCpuInfo->CpuidMaxExtInputValue = CpuIdRegister.RegEax;

  AsmCpuid (EFI_CPUID_SIGNATURE, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx, &CpuIdRegister.RegEdx);
  if (CpuIdRegister.RegEbx == SIGNATURE_32 ('G', 'e', 'n', 'u') && CpuIdRegister.RegEdx == SIGNATURE_32 ('i', 'n', 'e', 'I') &&
      CpuIdRegister.RegEcx == SIGNATURE_32 ('n', 't', 'e', 'l')) {
    pPlatformCpuInfo->IsIntelProcessor = TRUE;
  }

  //
  // Get Processor Brand String and remove leading and trailing spaces
  //
  if (pPlatformCpuInfo->CpuidMaxExtInputValue >= EFI_CPUID_BRAND_STRING3) {
    AsmCpuid (EFI_CPUID_BRAND_STRING1, &CpuBrandString[0].RegEax, &CpuBrandString[0].RegEbx, &CpuBrandString[0].RegEcx, &CpuBrandString[0].RegEdx);
    AsmCpuid (EFI_CPUID_BRAND_STRING2, &CpuBrandString[1].RegEax, &CpuBrandString[1].RegEbx, &CpuBrandString[1].RegEcx, &CpuBrandString[1].RegEdx);
    AsmCpuid (EFI_CPUID_BRAND_STRING3, &CpuBrandString[2].RegEax, &CpuBrandString[2].RegEbx, &CpuBrandString[2].RegEcx, &CpuBrandString[2].RegEdx);
    PtrCpuBrandString = (UINT8*) &CpuBrandString;

    //
    // Remove leading spaces.
    //
    Index1 = 0;
    Index2 = MAXIMUM_CPU_BRAND_STRING_LENGTH - 1;

    while (Index1 < (sizeof (CpuBrandString) / sizeof (UINT8))) {
      if (PtrCpuBrandString[Index1] == 0x20 && Index1 <= Index2) {
        Index1++;
      } else {
        break;
      }
    }

    //
    // Remove trailing spaces.
    //
    while (PtrCpuBrandString[Index2] == 0x00 || PtrCpuBrandString[Index2] == 0x20) {
      Index2--;
    }

    //
    // Remove sequential spaces.
    //
    for (Index = 0; Index1 <= Index2; Index++, Index1++) {
      //
      // If not at first char, and previous and current chars are space, then skip the char.
      //
      if (Index1 > 0 && PtrCpuBrandString[Index1 - 1] == 0x20 && PtrCpuBrandString[Index1] == 0x20) {
        Index--; // -1 from write index so loop "variable increment" statement (3rd part) doesn't advance write index
        continue;
      }
      pPlatformCpuInfo->BrandString[Index] = PtrCpuBrandString[Index1];
    }
  }

  pPlatformCpuInfo->CpuUarch = EnumCpuUarchUnknown;
  pPlatformCpuInfo->CpuPlatform = EnumCpuPlatformUnknown;
  pPlatformCpuInfo->CpuType = EnumCpuTypeUnknown;

  EfiCpuVersion (&FamilyId, &Model, &SteppingId, &ProcessorType);
  AsmCpuid (EFI_CPUID_VERSION_INFO, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx, &CpuIdRegister.RegEdx);
  pPlatformCpuInfo->CpuVersion.FullCpuId = CpuIdRegister.RegEax & B_EFI_CPUID_VERSION_INFO_EAX_MASK;
  pPlatformCpuInfo->CpuVersion.FullFamilyModelId = CpuIdRegister.RegEax & B_EFI_CPUID_VERSION_INFO_EAX_FULL_FAMILY_MODEL_MASK;
  pPlatformCpuInfo->CpuVersion.ExtendedFamilyId = (UINT8) ((CpuIdRegister.RegEax & B_EFI_CPUID_VERSION_INFO_EAX_EXT_FAMILY_ID_MASK) >> N_EFI_CPUID_VERSION_INFO_EAX_EXT_FAMILY_ID);
  pPlatformCpuInfo->CpuVersion.ExtendedModelId = (UINT8) ((CpuIdRegister.RegEax & B_EFI_CPUID_VERSION_INFO_EAX_EXT_MODEL_ID_MASK) >> N_EFI_CPUID_VERSION_INFO_EAX_EXT_MODEL_ID);
  pPlatformCpuInfo->CpuVersion.ProcessorType = ProcessorType;
  pPlatformCpuInfo->CpuVersion.FamilyId = (UINT8) FamilyId;
  pPlatformCpuInfo->CpuVersion.Model = Model;
  pPlatformCpuInfo->CpuVersion.SteppingId = SteppingId;

  //
  // Check Valleyview
  //
  if (FamilyId == EFI_CPUID_FAMILY_NEHALEM_UARCH && Model == EFI_CPUID_MODEL_VALLEYVIEW) {
    if (SteppingId <= EFI_CPUID_STEPPING_VALLEYVIEW_MAX) {
      pPlatformCpuInfo->CpuType = EnumValleyview;
    } else {
      pPlatformCpuInfo->CpuType = EnumAtom;
    }
    pPlatformCpuInfo->CpuPlatform = EnumNetTop;
  }
//hphang >> Force valid values for Simics testing
  pPlatformCpuInfo->CpuType = EnumValleyview;
  pPlatformCpuInfo->CpuPlatform = EnumNetTop;
//hphang << Force valid values for Simics testing
}

VOID
EfiCpuCStates (
  IN OUT EFI_PLATFORM_CPU_INFO    *pPlatformCpuInfo
  )
/*++

Routine Description:
  Matched and group similar CPUs to same CPU type.
  IMPORTANT: All CPU type detections should be done here to ensure no code
             duplication and detection confusion.

Arguments:
  CpuType  - Processor type

--*/
{
  EFI_CPUID_REGISTER       CpuIdRegister;
  UINT32                   Count, CountMax;

  AsmCpuid (EFI_CPUID_MONITOR_MWAIT_PARAMS, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx, &CpuIdRegister.RegEdx);
  pPlatformCpuInfo->CpuCState.MonitorMwaitSupport = (UINT8) ((CpuIdRegister.RegEcx & B_EFI_CPUID_MONITOR_MWAIT_ECX_MWAIT_SUPPORT) ? CPU_FEATURES_CAPABLE : 0);
  if (pPlatformCpuInfo->CpuCState.MonitorMwaitSupport) {
    //
    // Pineview have max of 2 states
    //
//hphang    CountMax = 5;   //cedarTrail have max of 4 states
    CountMax = 8;   //BayTrail (Valleyview) have max of 8 states

    pPlatformCpuInfo->CpuCState.InterruptsBreakMwait = (UINT8) ((CpuIdRegister.RegEcx & B_EFI_CPUID_MONITOR_MWAIT_ECX_INTERRUPTS_BREAK_MWAIT) ? CPU_FEATURES_CAPABLE : 0);
    pPlatformCpuInfo->CpuCState.C0SubCStatesMwait    = (UINT8) ((CpuIdRegister.RegEdx & B_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C0) >> N_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C0);
    pPlatformCpuInfo->CpuCState.C1SubCStatesMwait    = (UINT8) ((CpuIdRegister.RegEdx & B_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C1) >> N_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C1);
    pPlatformCpuInfo->CpuCState.C2SubCStatesMwait    = (UINT8) ((CpuIdRegister.RegEdx & B_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C2) >> N_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C2);
    pPlatformCpuInfo->CpuCState.C3SubCStatesMwait    = (UINT8) ((CpuIdRegister.RegEdx & B_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C3) >> N_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C3);
    pPlatformCpuInfo->CpuCState.C4SubCStatesMwait    = (UINT8) ((CpuIdRegister.RegEdx & B_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C4) >> N_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C4);
    pPlatformCpuInfo->CpuCState.C5SubCStatesMwait    = (UINT8) ((CpuIdRegister.RegEdx & B_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C5) >> N_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C5);
    pPlatformCpuInfo->CpuCState.C6SubCStatesMwait    = (UINT8) ((CpuIdRegister.RegEdx & B_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C6) >> N_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C6);
    pPlatformCpuInfo->CpuCState.C7SubCStatesMwait    = (UINT8) ((CpuIdRegister.RegEdx & B_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C7) >> N_EFI_CPUID_MONITOR_MWAIT_EDX_PARAMS_C7);
    for (Count = 0; Count < CountMax; Count++) {
      CpuIdRegister.RegEdx >>= 4;
      if (CpuIdRegister.RegEdx & 0x0F) {
        pPlatformCpuInfo->CpuCState.MaxCState++;
      } else {
        break;
      }
    }
  }
}

VOID
EfiCpuPackageInfo (
  IN OUT EFI_PLATFORM_CPU_INFO    *pPlatformCpuInfo
  )
/*++

Routine Description:
  Matched and group similar CPUs to same CPU type.
  Assume that each all threads on the same die share L2 cache.
  IMPORTANT: All CPU type detections should be done here to ensure no code
             duplication and detection confusion.

Arguments:
  CpuType  - Processor type

--*/
{
  EFI_CPUID_REGISTER       CpuIdRegister;
  UINT32                   NoofCoresPerPackage;
  UINT32                   EnabledApPerPackage;
  UINT32                   NoofCoreSharingL2Cache;
  UINT8                    Index;
  UINT8                    Ecx;

  //
  // Initialize variables
  //
  NoofCoresPerPackage = 1;
  NoofCoreSharingL2Cache = 1;

  AsmCpuid (EFI_CPUID_VERSION_INFO, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx, &CpuIdRegister.RegEdx);
  EnabledApPerPackage = ((CpuIdRegister.RegEbx >> 16) & 0xff);
  for (Index = 0; Index < 4; Index ++) {
    AsmCpuidEx (EFI_CPUID_CACHE_PARAMS, Index, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx, &CpuIdRegister.RegEdx);
    if ((CpuIdRegister.RegEax & 0x0F) == 3) {
      NoofCoresPerPackage = (((CpuIdRegister.RegEax >> 26) & 0x3f) + 1);
      NoofCoreSharingL2Cache = (((CpuIdRegister.RegEax >> 14) & 0xfff) + 1);
      break;
    }
  }
  pPlatformCpuInfo->CpuPackage.PhysicalPackages = PLATFORM_SUPPORTED_CPU_SOCKET_NUMBER;

  pPlatformCpuInfo->CpuPackage.ThreadsPerCore = 1;
  Ecx=1;
  AsmCpuidEx (EFI_CPUID_CORE_TOPOLOGY, Ecx, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx,&CpuIdRegister.RegEdx);
  pPlatformCpuInfo->CpuPackage.CoresPerPhysicalPackage =  (UINT8)(CpuIdRegister.RegEbx & 0xFF);
  pPlatformCpuInfo->CpuPackage.LogicalProcessorsPerPhysicalPackage = 1;
}

VOID
EfiCpuCache (
  IN OUT EFI_PLATFORM_CPU_INFO    *pPlatformCpuInfo
  )
/*++

Routine Description:
  Executes CPUID.2.EAX and stores CPU cache sizes.

Arguments:
  pPlatformCpuInfo  - Ptr to Platform CPU Info structure

--*/
{
  EFI_CPUID_REGISTER  CacheInfo;
  UINT32              *RegPtr, DescriptorDword;
  UINT8               CpuidCount, Index, RegIndex, ByteIndex;
  UINT8               DescriptorCount, DescriptorByte;

  DescriptorCount = 0;
  Index = 0;
  RegPtr = (UINT32 *) &CacheInfo;
  CacheInfo.RegEcx = 0;

  do {
    AsmCpuid (EFI_CPUID_CACHE_INFO, &CacheInfo.RegEax, &CacheInfo.RegEbx, &CacheInfo.RegEcx, &CacheInfo.RegEdx);
    CpuidCount = (UINT8) CacheInfo.RegEax;
    Index++;
    CacheInfo.RegEax &= 0xFFFFFF00;

    //
    // Go through Eax, Ebx, Ecx, Edx to retrieve cache information
    //
    for (RegIndex = 0; RegIndex < 4; RegIndex++) {
      DescriptorDword = RegPtr[RegIndex];
      //
      // [31] is clear when Descriptor is valid.
      //
      if (DescriptorDword & BIT31) {
        continue;
      }

      for (ByteIndex = 0; ByteIndex < 4; ByteIndex++) {
        DescriptorByte = (UINT8) DescriptorDword;
        DescriptorDword >>= 8;
        if (DescriptorByte > 0 && DescriptorCount < (MAX_CACHE_DESCRIPTORS - 1)) { // -1 so last one is NULL terminator
          pPlatformCpuInfo->CpuCache.CacheDescriptor[DescriptorCount++] = DescriptorByte;

          switch (DescriptorByte) {
            case 0x30: pPlatformCpuInfo->CpuCache.L1InstructionCacheSize = 32*1024;  break;
            case 0x0E: pPlatformCpuInfo->CpuCache.L1DataCacheSize        = 24*1024;  break;
            case 0x80: pPlatformCpuInfo->CpuCache.L2CacheSize            = 512*1024; break;
            case 0x3F: pPlatformCpuInfo->CpuCache.L2CacheSize            = 256*1024; break;
          }
        }
      }
    }
  } while (Index < CpuidCount);
}

VOID
EfiCpuFeatures (
  IN OUT EFI_PLATFORM_CPU_INFO    *pPlatformCpuInfo
  )
/*++

Routine Description:
  Matched and group similar CPUs to same CPU type.
  IMPORTANT: All CPU type detections should be done here to ensure no code
             duplication and detection confusion.

Arguments:
  CpuType  - Processor type

--*/
{
  EFI_CPUID_REGISTER       CpuIdRegister;
  UINT64                   MsrValue;
  UINT64                   TempMsrValue;

  //
  // Try to enable all necessary features so that CPUID returning correct feature support information.
  // Note: Please do this only for those features that needs to have feature enabled before CPUID can reflect correct support capability.
  //
  MsrValue = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
  TempMsrValue = MsrValue;
  TempMsrValue &= ~(B_EFI_MSR_IA32_MISC_ENABLE_XD);
  TempMsrValue |= B_EFI_MSR_IA32_MISC_ENABLE_MONITOR;
  AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, TempMsrValue);

  AsmCpuid (EFI_CPUID_VERSION_INFO, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx, &CpuIdRegister.RegEdx);
  pPlatformCpuInfo->CpuFeatures.Regs  = CpuIdRegister;
  pPlatformCpuInfo->CpuFeatures.Xapic = (UINT8) ((CpuIdRegister.RegEcx & B_EFI_CPUID_VERSION_INFO_ECX_XAPIC) ? CPU_FEATURES_CAPABLE : 0);
  pPlatformCpuInfo->CpuFeatures.Dca   = (UINT8) ((CpuIdRegister.RegEcx & B_EFI_CPUID_VERSION_INFO_ECX_DCA) ? CPU_FEATURES_CAPABLE : 0);
  pPlatformCpuInfo->CpuFeatures.Tm2   = (UINT8) ((CpuIdRegister.RegEcx & B_EFI_CPUID_VERSION_INFO_ECX_TM2) ? CPU_FEATURES_CAPABLE : 0);
  pPlatformCpuInfo->CpuFeatures.Eist  = (UINT8) ((CpuIdRegister.RegEcx & B_EFI_CPUID_VERSION_INFO_ECX_EIST) ? CPU_FEATURES_CAPABLE : 0);
  pPlatformCpuInfo->CpuFeatures.Lt    = (UINT8) ((CpuIdRegister.RegEcx & B_EFI_CPUID_VERSION_INFO_ECX_SME) ? CPU_FEATURES_CAPABLE : 0);
  pPlatformCpuInfo->CpuFeatures.Vt    = (UINT8) ((CpuIdRegister.RegEcx & B_EFI_CPUID_VERSION_INFO_ECX_VME) ? CPU_FEATURES_CAPABLE : 0);
  pPlatformCpuInfo->CpuFeatures.Mwait = (UINT8) ((CpuIdRegister.RegEcx & B_EFI_CPUID_VERSION_INFO_ECX_MWAIT) ? CPU_FEATURES_CAPABLE : 0);
  pPlatformCpuInfo->CpuFeatures.Tcc   = (UINT8) ((CpuIdRegister.RegEdx & B_EFI_CPUID_VERSION_INFO_EDX_THERMAL_CLOCK_CONTROL) ? CPU_FEATURES_CAPABLE : 0);
  pPlatformCpuInfo->CpuFeatures.Ht    = (UINT8) ((CpuIdRegister.RegEdx & B_EFI_CPUID_VERSION_INFO_EDX_HT) ? CPU_FEATURES_CAPABLE : 0);

  if (pPlatformCpuInfo->CpuidMaxExtInputValue >= EFI_CPUID_EXTENDED_FEATURE_BITS) {
    AsmCpuid (EFI_CPUID_EXTENDED_FEATURE_BITS, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx, &CpuIdRegister.RegEdx);
    pPlatformCpuInfo->CpuFeatures.ExtRegs         = CpuIdRegister;
    pPlatformCpuInfo->CpuFeatures.ExtLahfSahf64   = (UINT8) ((CpuIdRegister.RegEcx & EFI_CPUID_EXTENDED_FEATURE_BITS_ECX_LAHF_SAHF) ? CPU_FEATURES_CAPABLE : 0);
    pPlatformCpuInfo->CpuFeatures.ExtIntel64      = (UINT8)CPU_FEATURES_CAPABLE;
    pPlatformCpuInfo->CpuFeatures.ExtXd           = (UINT8) ((CpuIdRegister.RegEdx & EFI_CPUID_EXTENDED_FEATURE_BITS_EDX_XD) ? CPU_FEATURES_CAPABLE : 0);
    pPlatformCpuInfo->CpuFeatures.ExtSysCallRet64 = (UINT8) ((CpuIdRegister.RegEdx & EFI_CPUID_EXTENDED_FEATURE_BITS_EDX_SYSCALL) ? CPU_FEATURES_CAPABLE : 0);
  }

  //
  // Restore EFI_MSR_IA32_MISC_ENABLE MSR to original value
  //
  AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, MsrValue);
}

VOID
EfiCpuPowerManagement (
  IN OUT EFI_PLATFORM_CPU_INFO    *pPlatformCpuInfo
  )
/*++

Routine Description:
  Executes CPUID.6.EAX and stores CPU power capabilities.

Arguments:
  pPlatformCpuInfo  - Ptr to Platform CPU Info structure

--*/
{
  EFI_CPUID_REGISTER       CpuIdRegister;

  AsmCpuid (EFI_CPUID_POWER_MANAGEMENT_PARAMS, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx, &CpuIdRegister.RegEdx);
  pPlatformCpuInfo->CpuPowerManagement.PECI = (UINT8) ((CpuIdRegister.RegEax & EFI_CPUID_POWER_MANAGEMENT_EAX_PECI) ? CPU_FEATURES_CAPABLE : 0);
  pPlatformCpuInfo->CpuPowerManagement.NumIntThresholds = (UINT8) ((CpuIdRegister.RegEbx & EFI_CPUID_POWER_MANAGEMENT_EBX_NUM_INT_THRESHOLDS) ? CPU_FEATURES_CAPABLE : 0);
  pPlatformCpuInfo->CpuPowerManagement.HwCoordinationFeedback = (UINT8) ((CpuIdRegister.RegEcx & EFI_CPUID_POWER_MANAGEMENT_ECX_HW_COORDINATION_FEEDBACK) ? CPU_FEATURES_CAPABLE : 0);
}

VOID
EfiCpuAddressBits (
  IN OUT EFI_PLATFORM_CPU_INFO    *pPlatformCpuInfo
  )
/*++

Routine Description:
  Executes CPUID.80000008.EAX and stores CPU address bits.

Arguments:
  pPlatformCpuInfo  - Ptr to Platform CPU Info structure

--*/
{
  EFI_CPUID_REGISTER       CpuIdRegister;

  pPlatformCpuInfo->CpuAddress.PhysicalBits = 32;
  pPlatformCpuInfo->CpuAddress.VirtualBits  = 32;
  if (pPlatformCpuInfo->CpuidMaxExtInputValue >= EFI_CPUID_VIRT_PHYS_ADDRESS_SIZE) {
    AsmCpuid (EFI_CPUID_VIRT_PHYS_ADDRESS_SIZE, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx, &CpuIdRegister.RegEdx);
    pPlatformCpuInfo->CpuAddress.VirtualBits  = (UINT8) ((CpuIdRegister.RegEax & B_EFI_CPUID_VIRTUAL_ADDRESS_BITS) >> 8);
    pPlatformCpuInfo->CpuAddress.PhysicalBits = (UINT8) (CpuIdRegister.RegEax & B_EFI_CPUID_PHYSICAL_ADDRESS_BITS);
  }
}

 
VOID
EfiCpuMsrs (
  IN OUT EFI_PLATFORM_CPU_INFO    *pPlatformCpuInfo
  )
/*++

Routine Description:
  Reads RO MSRs and stores values.

Arguments:
  pPlatformCpuInfo  - Ptr to Platform CPU Info structure

--*/
{
  UINT64           MsrValue;
  EFI_CPUID_REGISTER  CpuIdRegister;

  MsrValue = AsmReadMsr64 (EFI_MSR_IA32_PLATFORM_ID);
  pPlatformCpuInfo->Msr.PlatformID = (UINT8) ((MsrValue & B_EFI_MSR_IA32_PLATFORM_ID_PLATFORM_ID_BITS_MASK) >> N_EFI_MSR_IA32_PLATFORM_ID_PLATFORM_ID_BITS_MASK_START);

  AsmCpuid (EFI_CPUID_VERSION_INFO, &CpuIdRegister.RegEax, &CpuIdRegister.RegEbx, &CpuIdRegister.RegEcx, &CpuIdRegister.RegEdx);
  MsrValue = AsmReadMsr64 (EFI_MSR_IA32_BIOS_SIGN_ID);
  pPlatformCpuInfo->Msr.MicrocodeRevision = (UINT32) (MsrValue >> 32);

  MsrValue = AsmReadMsr64 (EFI_MSR_IA32_MTRR_CAP);
  pPlatformCpuInfo->Msr.Smrr = (UINT8) ((MsrValue & B_EFI_MSR_IA32_MTRR_CAP_SMRR_SUPPORT) ? 1 : 0);
  pPlatformCpuInfo->Msr.Emrr = (UINT8) ((MsrValue & B_EFI_MSR_IA32_MTRR_CAP_EMRR_SUPPORT) ? 1 : 0);
//hphang >> For Simics testing, temporary disable SMRR
#if SMRR_SUPPORT
  pPlatformCpuInfo->Msr.Smrr = 1;
#else
  pPlatformCpuInfo->Msr.Smrr = 0;
#endif
//hphang << For Simics testing, temporary disable SMRR
  pPlatformCpuInfo->Msr.VariableMtrrCount = (UINT8) (MsrValue & B_EFI_MSR_IA32_MTRR_CAP_VARIABLE_SUPPORT);

  MsrValue = AsmReadMsr64 (EFI_MSR_IA32_PERF_STS);
  pPlatformCpuInfo->Msr.PState = (UINT16) (MsrValue & 0xFFFF);
}

EFI_STATUS
InitCpuInfo (
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Construct the HOB with data specific to current processor.

Arguments:

Returns:

  None

--*/
{
  EFI_PLATFORM_CPU_INFO    PlatformCpuInfo;

  ZeroMem (&PlatformCpuInfo, sizeof(PlatformCpuInfo));

  EfiCpuType (&PlatformCpuInfo);
  EfiCpuPackageInfo (&PlatformCpuInfo);
  EfiCpuCStates (&PlatformCpuInfo);
  EfiCpuCache (&PlatformCpuInfo);
  EfiCpuFeatures (&PlatformCpuInfo);
  EfiCpuPowerManagement (&PlatformCpuInfo);
  EfiCpuAddressBits (&PlatformCpuInfo);
  EfiCpuMsrs (&PlatformCpuInfo);

  BuildGuidDataHob (
    &gEfiPlatformCpuInfoGuid,
    &PlatformCpuInfo,
    sizeof (EFI_PLATFORM_CPU_INFO)
    );

  return EFI_SUCCESS;
}

