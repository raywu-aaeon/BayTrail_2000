/*++

Copyright (c) 1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  Thermal.c

Abstract:

  Thermal Monitor initialization

--*/

#include "Thermal.h"
#include "MiscFuncs.h"

extern EFI_PLATFORM_CPU_INFO         mPlatformCpuInfo;

VOID
InitializeThermalMonitor (
  VOID
  )
/*++

Routine Description:

  Initialize thermal monitor.

Arguments:

Returns:

  None.

--*/
{
// klee37 - NOTE: This feature set is not compatible with Atom Processor (Valleyview)
//EFI_CPUID_REGISTER  CpuidRegisters;
//UINT64              Data;
//
/////
///// Check the TM2 feature flag.
/////
//AsmCpuid (
//  EFI_CPUID_VERSION_INFO,
//  &CpuidRegisters.RegEax,
//  &CpuidRegisters.RegEbx,
//  &CpuidRegisters.RegEcx,
//  &CpuidRegisters.RegEdx
//  );
//
//if ((CpuidRegisters.RegEcx & B_EFI_CPUID_VERSION_INFO_ECX_TM2) != 0) {
//  ///
//  /// Enable TM2 if it is supported.
//  ///
//  CpuMiscEnable (TRUE, B_MSR_IA32_MISC_ENABLE_TME);
//
//  ///
//  /// Lock TM interrupt
//  ///
//  Data = AsmReadMsr64 (MSR_MISC_PWR_MGMT);
//  Data |= B_MSR_MISC_PWR_MGMT_LTMI;
//  AsmWriteMsr64 (MSR_MISC_PWR_MGMT, Data);
//
//  ///
//  /// Enable Thermal Interrupt
//  ///
//  Data = AsmReadMsr64 (EFI_MSR_IA32_THERM_INTERRUPT);
//  Data |= B_EFI_IA32_THERM_INTERRUPT_VIE;
//  AsmWriteMsr64 (EFI_MSR_IA32_THERM_INTERRUPT, Data);
//}
}
