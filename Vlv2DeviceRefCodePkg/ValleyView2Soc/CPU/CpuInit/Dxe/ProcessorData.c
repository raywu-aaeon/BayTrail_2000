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

    ProcessorData.c

Abstract:

    Produces CPU data records.


--*/

#include "CpuDxe.h"
#include "ProcessorData.h"
#include "Processor.h"
#include "MpCommon.h"
#include "PlatformMpService.h"
//
// This is the VFR compiler generated header file which defines the
// string identifiers.
//

extern MP_SYSTEM_DATA               *mMPSystemData;
extern EFI_PLATFORM_CPU_PROTOCOL    *mPlatformCpu;
extern EFI_PLATFORM_CPU_INFO        mPlatformCpuInfo;

VOID
AsciiToUnicode (
  IN      CHAR8     *AsciiString,
  IN OUT  CHAR16    *UnicodeString
  )
/*++

Routine Description:

  Converts an ascii string to unicode string 16 chars at a time.

Arguments:

  None

Returns:

--*/
// GC_TODO:    AsciiString - add argument and description to function comment
// GC_TODO:    UnicodeString - add argument and description to function comment
{
  UINT8 Index;

  for (Index = 0; Index < 16; Index++) {
    UnicodeString[Index] = (CHAR16) AsciiString[Index];
  }

  return ;
}

//
// Processor-specific routines
//
VOID
GetProcessorVersion (
  OUT PROCESSOR_VERSION_INFORMATION *Version
  )
/*++

Routine Description:

  Returns the procesor version string token installed in the system.

Arguments:

  None

Returns:

  Processor Version string token

--*/
{
  CHAR16              BrandIdString[MAXIMUM_CPU_BRAND_STRING_LENGTH + 1];
  UINT32              RegEax;
//  EFI_CPUID_REGISTER  CpuExtendedSupport;
  EFI_CPUID_REGISTER  CpuBrandString;
  UINT8               Index;

  //
  // Create the string using Brand ID String.
  //
  Version->StringValid = FALSE;

  if (IsIntelProcessor ()) {
    Version->StringRef = STRING_TOKEN (STR_INTEL_GENUINE_PROCESSOR);

    AsmCpuid (EFI_CPUID_EXTENDED_FUNCTION, &RegEax, NULL, NULL, NULL);
    AsmCpuid (EFI_CPUID_BRAND_STRING1, &CpuBrandString.RegEax, &CpuBrandString.RegEbx, &CpuBrandString.RegEcx, &CpuBrandString.RegEdx);
    //
    // Check if Brand ID String is supported or filled up
    //
    if (RegEax != 0 && CpuBrandString.RegEax != 0) {
      AsciiToUnicode ((CHAR8 *) &CpuBrandString, (CHAR16 *) &BrandIdString[0]);
      AsmCpuid (EFI_CPUID_BRAND_STRING2, &CpuBrandString.RegEax, &CpuBrandString.RegEbx, &CpuBrandString.RegEcx, &CpuBrandString.RegEdx);
      AsciiToUnicode ((CHAR8 *) &CpuBrandString, (CHAR16 *) &BrandIdString[16]);
      AsmCpuid (EFI_CPUID_BRAND_STRING3, &CpuBrandString.RegEax, &CpuBrandString.RegEbx, &CpuBrandString.RegEcx, &CpuBrandString.RegEdx);
      AsciiToUnicode ((CHAR8 *) &CpuBrandString, (CHAR16 *) &BrandIdString[32]);

      //
      // Remove preceeding spaces
      //
      Index = 0;
      while (BrandIdString[Index] == 0x20) {
        Index++;
      }
      if (Index < (sizeof (BrandIdString) / sizeof (CHAR16))) {
        CopyMem (
          Version->BrandString,
          &BrandIdString[Index],
          (MAXIMUM_CPU_BRAND_STRING_LENGTH - Index) * sizeof (CHAR16)
          );
        Version->BrandString[MAXIMUM_CPU_BRAND_STRING_LENGTH - Index] = 0;
        Version->StringValid = TRUE;
      }
    }
  } else {
    Version->StringRef = STRING_TOKEN (STR_UNKNOWN);
  }
}

EFI_PROCESSOR_MANUFACTURER_DATA
GetProcessorManufacturer (
  VOID
  )
/*++

Routine Description:

  Returns the procesor manufaturer string token installed in the system.

Arguments:

  None

Returns:

  Processor Manufacturer string token

--*/
{

  if (IsIntelProcessor ()) {
    return STRING_TOKEN (STR_INTEL_CORPORATION);
  } else {
    return STRING_TOKEN (STR_UNKNOWN);
  }
}

BOOLEAN
IsIntelProcessor (
  VOID
  )
/*++

Routine Description:

  Returns if processor is Intel or not.

Arguments:

  None

Returns:

  TRUE - Intel Processor.
  FALSE - Not Intel Processor.

--*/
{
  EFI_CPUID_REGISTER  Reg;

  AsmCpuid (EFI_CPUID_SIGNATURE, &Reg.RegEax, &Reg.RegEbx, &Reg.RegEcx, &Reg.RegEdx);

  if ((Reg.RegEbx != SIGNATURE_32 ('G', 'e', 'n', 'u')) || (Reg.RegEdx != SIGNATURE_32 ('i', 'n', 'e', 'I')) || (Reg.RegEcx != SIGNATURE_32 ('n', 't', 'e', 'l'))) {
    return FALSE;
  } else {
    return TRUE;
  }
}

EFI_PROCESSOR_FAMILY_DATA
GetProcessorFamily (
  VOID
  )
/*++

Routine Description:

  Returns the processor family of the processor installed in the system.

Arguments:

  None

Returns:

  Processor Family

--*/
{
  if (IsIntelProcessor ()) {
    //
    // Xeon Family
    //
    if (mPlatformCpu->PlatformCategory == EfiPlatformServer) {
      return EfiProcessorFamilyIntelXeon;
    }

    //
    // Conroe Family
    // or other desktop processor
    //
    if (mPlatformCpu->PlatformCategory == EfiPlatformDesktop) {
      //
      // New processors does not have value defined
      // So return "Other" (0x01)
      //
      if (mPlatformCpuInfo.CpuType != EnumCpuTypeUnknown) {
        if (mPlatformCpuInfo.CpuType == EnumPineview || mPlatformCpuInfo.CpuType == EnumAtom || mPlatformCpuInfo.CpuType == EnumValleyview) {
          return EfiProcessorFamilyIntelAtom;
        } else {
          return EfiProcessorFamilyOther;
        }
      }
      return EfiProcessorFamilyIntelCore2;
    }

    //
    // Merom Family
    //
    if (mPlatformCpu->PlatformCategory == EfiPlatformMobile) {
      return EfiProcessorFamilyIntelPentiumM;
    }

    //
    // Other Families
    //
    return EfiProcessorFamilyOther;
  }

  return EfiProcessorFamilyUnknown;
}

INT16
GetProcessorVoltage (
  VOID
  )
/*++

Routine Description:

  Returns the processor voltage of the processor installed in the system.

Arguments:

  None

Returns:

  Processor Voltage

--*/
{
  INT16   VoltageInmV;
  UINT64  MsrValue;

  VoltageInmV = 0;
  MsrValue = 0;

  ///
  /// Core voltage = (float) IA32_PERF_STS(7:0) * (float)10mv + 250mv
  ///
  MsrValue = AsmReadMsr64 (EFI_MSR_IA32_PERF_STS);
  MsrValue &= 0xFF;

  ///
  /// Convert unit to mV
  ///
  VoltageInmV = (UINT16) (MsrValue) * 10 + 250;

  return VoltageInmV;
}

UINT32
GetCpuUcodeRevision (
  VOID
  )
/*++

Routine Description:

  Returns the processor microcode revision of the processor installed in the system.

Arguments:

  None

Returns:

  Processor Microcode Revision

--*/
{
  AsmWriteMsr64 (EFI_MSR_IA32_BIOS_SIGN_ID, 0);
  AsmCpuid (EFI_CPUID_VERSION_INFO, NULL, NULL, NULL, NULL);
  return (UINT32) RShiftU64 (AsmReadMsr64 (EFI_MSR_IA32_BIOS_SIGN_ID), 32);
}

EFI_PROCESSOR_CORE_COUNT_DATA
GetProcessorCoreCount (
  VOID
  )
/*++

Routine Description:

  Returns the processor core count of the processor installed in the system.

Arguments:

  None

Returns:

  Processor Core Count

--*/
{
  return (EFI_PROCESSOR_CORE_COUNT_DATA) (mPlatformCpuInfo.CpuPackage.CoresPerPhysicalPackage);
}

EFI_PROCESSOR_ENABLED_CORE_COUNT_DATA
GetProcessorEnabledCoreCount (
  VOID
  )
/*++

Routine Description:

  Returns the processor enabled core count of the processor installed in the system.

Arguments:

  None

Returns:

  Processor Enabled Core Count

--*/
{
  UINT8  Count;

  Count = mPlatformCpuInfo.CpuPackage.CoresPerPhysicalPackage;

  if (mMPSystemData->ActiveProcessorCores != 0) {
    Count = mMPSystemData->ActiveProcessorCores;
  }
  return (EFI_PROCESSOR_CORE_COUNT_DATA) Count;
}


EFI_PROCESSOR_THREAD_COUNT_DATA
GetProcessorThreadCount (
  VOID
  )
/*++

Routine Description:

  Returns the processor thread count of the processor installed in the system.

Arguments:

  None

Returns:

  Processor Thread Count

--*/
{
  return (EFI_PROCESSOR_THREAD_COUNT_DATA) (mPlatformCpuInfo.CpuPackage.LogicalProcessorsPerPhysicalPackage);
}

EFI_PROCESSOR_CHARACTERISTICS_DATA
GetProcessorCharacteristics (
  VOID
  )
/*++

Routine Description:

  Returns the processor Characteristics of the processor installed in the system.

Arguments:

  None

Returns:

  Processor Characteristics

--*/
{
  EFI_PROCESSOR_CHARACTERISTICS_DATA_EXT  CharacteristicsData;

  ZeroMem (&CharacteristicsData, sizeof (EFI_PROCESSOR_CHARACTERISTICS_DATA));

  if (mPlatformCpuInfo.CpuFeatures.ExtIntel64 == CPU_FEATURES_CAPABLE ) {
    CharacteristicsData.Processor64bitCapable = 1;
  }

  if (mPlatformCpuInfo.CpuPackage.CoresPerPhysicalPackage > 1) {
    CharacteristicsData.ProcessorMultiCore = 1;
  }

  if (mPlatformCpuInfo.CpuPackage.ThreadsPerCore > 1) {
    CharacteristicsData.ProcessorHardwareThread = 1;
  }

  if (mPlatformCpuInfo.CpuFeatures.ExtXd == CPU_FEATURES_CAPABLE ) {
    CharacteristicsData.ProcessorExecuteProtection = 1;
  }

  if (mPlatformCpuInfo.CpuFeatures.Vt == CPU_FEATURES_CAPABLE ) {
    CharacteristicsData.ProcessorEnhancedVirtualization = 1;
  }

  if (mPlatformCpuInfo.CpuFeatures.Eist == CPU_FEATURES_CAPABLE ) {
    CharacteristicsData.ProcessorPowerControl = 1;
  }

  return *(EFI_PROCESSOR_CHARACTERISTICS_DATA*)&CharacteristicsData;
}