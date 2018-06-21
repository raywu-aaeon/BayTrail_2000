/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  SilvermontPpmLib.c

Abstract:

  This library contains power management configuration functions for
  Valleyview processors.

  Acronyms:
    PPM   Processor Power Management
    GV    Geyserville
    TM    Thermal Monitor
    IST   Intel(R) Speedstep technology
    HT    Hyper-Threading Technology

--*/

//
// Include files
//
#include "SilvermontPpmLib.h"
#include "PpmCommon.h"
#include "CpuPpmLib.h"
#include "CpuRegs.h"
#include "IchPpmLib.h"

//
// Global variables
//
//
// Start: Workaround for sighting 4683480, 4683441 for Q8XA IVI SKU
//
STATIC UINT8                     mTurboModeNotAvailable = 0;
//
// End: Workaround for sighting 4683480, 4683441 for Q8XA IVI SKU
//

//
// CPU stepping
//
STATIC UINT16                     mProcessorStepping;
//
// Only one instance of this protocol currently supported.
//
PPM_PROCESSOR_SUPPORT_PROTOCOL_2 mPpmProcessorSupportProtocolValleyview = {
  0,                                        // PpmFlags
  0,                                        // PpmFeatureFlags
  SetPpmFlagsValleyview,
  EnableCStatesValleyview,
  InitGv3Valleyview,
  ConfigureTurboPowerLimitsValleyview,
  InitThermalValleyview,
  EnableTmValleyview,
//  EnableTm2Valleyview,
  EnableProcHotValleyview,
  EnableTscValleyview,
  NULL,                        // EMTTM Not supportted for SLT
  S3SaveMsrValleyview,
  S3RestoreMsrValleyview,
  SetBootPState,
  EnableCStateIoRedirectionValleyview,
  DisableCStateIoRedirectionValleyview,
//  EnableCStateSmiCoordinationValleyview,
//  DisableCStateSmiCoordinationValleyview,
  NULL,
  NULL,
  NULL,
  NULL,                                     // PStateTransition
  NULL  //PowerSourceChangeValleyview,
};

//
// CPUID 01 values
//
STATIC EFI_CPUID_REGISTER mCpuid01 = { 0, 0, 0, 0 };


//
// Values for FVID table calculate.
//
STATIC UINT16                     mTurboBusRatio   = 0;
STATIC UINT16                     mMaxBusRatio     = 0;
STATIC UINT16                     mMinBusRatio     = 0;
STATIC UINT16                     mTurboVid        = 0;
STATIC UINT16                     mMaxVid          = 0;
STATIC UINT16                     mMinVid          = 0;
STATIC UINT16                     mProcessorFlavor = 0;
//
// Fractional part of Processor Power Unit in Watts. (i.e. Unit is 1/mProcessorPowerUnit)
//
STATIC UINT8                      mProcessorPowerUnit = 0;
//
// Fractional part of Processor Time Unit in seconds. (i.e Unit is 1/mProcessorTimeUnit)
//
STATIC UINT8                      mProcessorTimeUnit = 0;
//
// Maximum allowed power limit value in TURBO_POWER_LIMIT_MSR and PRIMARY_PLANE_POWER_LIMIT_MSR
// in units specified by PACKAGE_POWER_SKU_UNIT_MSR
//
STATIC UINT16                     mPackageMaxPower = 0;
//
// Minimum allowed power limit value in TURBO_POWER_LIMIT_MSR and PRIMARY_PLANE_POWER_LIMIT_MSR
// in units specified by PACKAGE_POWER_SKU_UNIT_MSR
//
STATIC UINT16                     mPackageMinPower = 0;
//
// Processor TDP value in Watts
//
STATIC UINT16                     mPackageTdp = 0;
//
// Porgrammable TDP Limit
//
STATIC UINT8                      mTdpLimitProgrammble = 0;

//
// Table to convert Seconds into equivalent MSR values
// This table is used for TDP Time Window programming
//
STATIC UINT8                      mSecondsToMsrValueMapTable[][2] = {
  // Seconds       MSR Value
  {   1        ,     0x0 },
  {   5        ,     0x1 },
  {  10        ,     0x2 },
  {  15        ,     0x3 },
  {  20        ,     0x4 },
  {  25        ,     0x5 },
  {  30        ,     0x6 },
  {  35        ,     0x7 },
  {  40        ,     0x8 },
  {  45        ,     0x9 },
  {  50        ,     0xA },
  //{ reserved   ,     0xB-0x7F},
  {END_OF_TABLE,END_OF_TABLE}
};


// MSR table for S3 resume
//
STATIC EFI_MSR_VALUES mMsrValues[] = {
  { MSR_IA32_CLOCK_MODULATION,          0 },  // 0x19A
  { MSR_IA32_PERF_CTL,                  0 },  // 0x199
  { MSR_PMG_IO_CAPTURE_BASE,            0 },  // 0x0E4
  { MSR_PM_CFG_CTRL,                    0 },  // 0x0E2
//  { MSR_MISC_PWR_MGMT,                  0 },
  { MSR_IA32_MISC_ENABLES,              0 },  // 0x1A0
  { MSR_POWER_CTL,                      0 },  // 0x1FC
  { MSR_TURBO_POWER_LIMIT,              0 },  // 0x610
  { MSR_TEMPERATURE_TARGET,             0 },  // 0x1A2
  { MSR_FLEX_RATIO,                     0 },  // 0x194
  { MSR_PRIMARY_PLANE_CURRENT_CONFIG,   0 },  // 0x601
//  { MSR_SECONDARY_PLANE_CURRENT_CONFIG, 0 },
  { MSR_IA32_ENERGY_PERFORMANCE_BIAS,   0 },  // 0x1B0
  { MSR_BBL_CR_CTL3,                    0 },  // 0x11E
  { MSR_IA32_THERM_INTERRUPT,           0 },  // 0x19B
  { MSR_IO_CAPT_ADDR,                   0 },
  { MSR_PM_CFG_CTRL,                    0 },
  { MSR_POWER_MISC,                     0 }
};

//
// Function implemenations
//
#ifdef __GNUC__
#pragma GCC push_options
#pragma GCC optimize ("O0")
#else
#pragma optimize("", off)
#endif
EFI_STATUS
InitializeValleyviewPowerManagementLib (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   **This
  )
/*++

Routine Description:

  Initializes the processor power management library.  This must be called
  prior to any of the library functions being used.

Arguments:

  This          Pointer to the PPM support protocol instance

Returns:

  EFI_SUCCESS   Library was initialized successfully

--*/
{
  MSR_REGISTER  PackagePowerSKUUnitMsr;
  MSR_REGISTER  TempMsr;
  UINTN         remainder;

  MSR_REGISTER  PlatformIdMsr;
  MSR_REGISTER  IaCoreRatios;
  MSR_REGISTER  IaCoreVids;
  MSR_REGISTER        TurboPowerLimitMsr;
  MSR_REGISTER        PKGTurboCfg1Msr;
  MSR_REGISTER        CPUTurboWkldCfg1Msr;

  ASSERT (gSmst != NULL);
  //
  // Read the CPUID information
  //
  AsmCpuid (EFI_CPUID_VERSION_INFO, &mCpuid01.RegEax, &mCpuid01.RegEbx, &mCpuid01.RegEcx, &mCpuid01.RegEdx);
  //
  // Check if this library supports the current processor
  //
  //Fix for Simics if ((mCpuid01.RegEax & CPUID_FULL_FAMILY_MODEL) != FAMILYMODELSTEP_SLM) {
  //  return EFI_SUCCESS;
  //}

  mProcessorStepping = (UINT16) (mCpuid01.RegEax & CPUID_STEPPING);

  //
  // Assign the protocol pointer.
  //
  *This = &mPpmProcessorSupportProtocolValleyview;

  //
  // Get Platform ID
  //
  PlatformIdMsr.Qword = AsmReadMsr64 (MSR_IA32_PLATFORM_ID);
  mProcessorFlavor = (UINT8) RShiftU64((PlatformIdMsr.Dwords.High & PLATFORM_ID_BITS_MASK), 18);

  //
  // Get the Bus Ratio for the processor
  //
  //
  // Get Maximum Non-Turbo bus ratio (HFM) from IACORE_RATIOS MSR Bits[23:16]
  //
  IaCoreRatios.Qword = AsmReadMsr64 (MSR_IACORE_RATIOS);
  mMaxBusRatio  = IaCoreRatios.Bytes.ThirdByte;

  //
  // Get Maximum Efficiency bus ratio (LFM) from IACORE_RATIOS MSR Bits[15:8]
  //
  mMinBusRatio = IaCoreRatios.Bytes.SecondByte;

  //
  // Get Max Turbo Ratio from Turbo Ratio Limit MSR Bits [5:0]
  //
  TempMsr.Qword    = AsmReadMsr64 (MSR_IACORE_TURBO_RATIOS);
  mTurboBusRatio  = (UINT16)(TempMsr.Dwords.Low & MAX_RATIO_1C_MASK);


  //
  // Get the Vid for the processor
  //
  //
  // Get Maximum Non-Turbo Vid (HFM) from IACORE_VIDS MSR Bits[23:16]
  //
  IaCoreVids.Qword = AsmReadMsr64 (MSR_IACORE_VIDS);
  mMaxVid  = IaCoreVids.Bytes.ThirdByte;

  //
  // Get Maximum Efficiency VID (LFM) from IACORE_VIDS MSR Bits[15:8]
  //
  mMinVid = IaCoreVids.Bytes.SecondByte;

  //
  // Get Max Turbo Ratio from Turbo Ratio Limit MSR Bits [5:0]
  //
  TempMsr.Qword    = AsmReadMsr64 (MSR_IACORE_TURBO_VIDS);
  mTurboVid  = (UINT16)(TempMsr.Dwords.Low & MAX_RATIO_1C_MASK);

  // Get Processor TDP from Turbo Power Limit MSR Bits[14:0]
  // and convert it to units specified by Package Power SKU
  // Unit MSR [3:0]
  //
  TempMsr.Qword                 = AsmReadMsr64 (MSR_TURBO_POWER_LIMIT);

  // Get Maximum Power from Turbo Power Limit MSR Bits[14:0]
  // and convert it to units specified by Package Power SKU Unit MSR [3:0]
  //
  PackagePowerSKUUnitMsr.Qword  = AsmReadMsr64 (MSR_PACKAGE_POWER_SKU_UNIT);

  mProcessorPowerUnit           = (PackagePowerSKUUnitMsr.Bytes.FirstByte & PACKAGE_POWER_UNIT_MASK);
  if (mProcessorPowerUnit == 0) {
    mProcessorPowerUnit = 1;
  } else {
    // The actual unit value is calculated by 1mW*Power(2,POWER_UNIT)..Reset value of 5 represents 32mW units.
    mProcessorPowerUnit = (UINT8) LShiftU64 (1, (mProcessorPowerUnit));
  }

  // There are two power limits in the same MSR [14:0] and [46:32]. Bit field [14:0] reflects the package TDP.
  mPackageTdp       = (UINT16) DivU64x32Remainder((TempMsr.Dwords.Low & PACKAGE_TDP_POWER_MASK), mProcessorPowerUnit, (UINT32 *)&remainder);

  /* @NOTE: This may be used in Valleyview, but still keeping this code. */
  mProcessorTimeUnit  = (UINT8) RShiftU64((PackagePowerSKUUnitMsr.Dwords.Low & PACKAGE_TIME_UNIT_MASK), 16);
  if (mProcessorTimeUnit == 0) {
    mProcessorTimeUnit = 1;
  } else {
    mProcessorTimeUnit = (UINT8) LShiftU64 (1, (mProcessorTimeUnit));
  }

  // Set up registers for energy management and dynamic power limiting
  TurboPowerLimitMsr.Qword  = AsmReadMsr64 (MSR_TURBO_POWER_LIMIT);
  TurboPowerLimitMsr.Dwords.Low = 0x003880FA;
  TurboPowerLimitMsr.Dwords.High = 0x00000000;
  AsmWriteMsr64 (MSR_TURBO_POWER_LIMIT, TurboPowerLimitMsr.Qword);

  PKGTurboCfg1Msr.Qword = AsmReadMsr64 (MSR_PKG_TURBO_CFG1);
  PKGTurboCfg1Msr.Dwords.Low &= 0x0;
  PKGTurboCfg1Msr.Dwords.Low |= 0x00000702;
  AsmWriteMsr64 (MSR_PKG_TURBO_CFG1, PKGTurboCfg1Msr.Qword);

  CPUTurboWkldCfg1Msr.Qword = AsmReadMsr64 (MSR_CPU_TURBO_WKLD_CFG1);
  CPUTurboWkldCfg1Msr.Dwords.Low &= 0x0;
  CPUTurboWkldCfg1Msr.Dwords.Low |= 0x200B;
  AsmWriteMsr64 (MSR_CPU_TURBO_WKLD_CFG1, CPUTurboWkldCfg1Msr.Qword);

  return EFI_SUCCESS;
}
#ifdef __GNUC__
#pragma GCC pop_options
#else
#pragma optimize("", on)
#endif


STATIC
EFI_STATUS
SetPpmFlagsValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Set the PPM flags specific to mobile Valleyview processors

Arguments:

  This          Pointer to the protocol instance

Returns:

  EFI_SUCCESS   PpmFlags updated with the features supported by the processor

--*/
{
  MSR_REGISTER                PlatformId;
  MSR_REGISTER                Ia32MiscEnable;
  EFI_CPUID_REGISTER          Cpuid01 = {0};
  EFI_CPUID_REGISTER          Cpuid05 = {0};
  EFI_CPUID_REGISTER          Cpuid06 = {0};
  EFI_CPUID_REGISTER          Cpuid0B = {0};
  UINTN                       States;
  UINT8                       NumberOfLP = 0;
  UINT8                       ThreadPerCore = 0;
  UINT32                      Ecx = 0;
  BOOLEAN                     CpuidLimitingEnabled;

  CpuidLimitingEnabled = FALSE;

  // DEBUG ((EFI_D_ERROR, "\n\tSetting PPM Flags specific to Valleyview Processor..."));
  //
  // Check if the processor has multiple cores
  //
  //EfiCpuid (EFI_CPUID_XAPIC_PROC_TOPOLOGY, &Cpuid0B);
  Ecx = 0;  // Set the level number to 0 for SMT level of Processor Topology.
  AsmCpuidEx (EFI_CPUID_XAPIC_PROC_TOPOLOGY, Ecx, &Cpuid0B.RegEax, &Cpuid0B.RegEbx, &Cpuid0B.RegEcx, &Cpuid0B.RegEdx);
  ThreadPerCore = (UINT8) Cpuid0B.RegEbx & 0xFF;

  Ecx = 1;  // Set the level number to 1 for Core level of Processor Topology.
  AsmCpuidEx (EFI_CPUID_XAPIC_PROC_TOPOLOGY, Ecx, &Cpuid0B.RegEax, &Cpuid0B.RegEbx, &Cpuid0B.RegEcx, &Cpuid0B.RegEdx);
  NumberOfLP = (UINT8) Cpuid0B.RegEbx & 0xFF;

  if (NumberOfLP > 2 && ThreadPerCore == 1) {
    This->PpmFlags |= (PPM_QUAD | PPM_CMP);
    // DEBUG ((EFI_D_ERROR, "\n\tQuad Core detected"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_QUAD | PPM_CMP"));
  } else if (NumberOfLP > 1 && ThreadPerCore == 1) {
    This->PpmFlags |= PPM_CMP;
    // DEBUG ((EFI_D_ERROR, "\n\tDual Core detected"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_CMP"));
  }

  //
  // Valleyview support TSC updates
  //
  if (mCpuid01.RegEcx & (1 << 4)) {
    This->PpmFlags |= PPM_TSC;
    // DEBUG ((EFI_D_ERROR, "\n\tTSC updates supported"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_TSC"));
  }

  //
  // Set PROCHOT# always
  //
  This->PpmFlags |= PPM_PROC_HOT;
  // DEBUG ((EFI_D_ERROR, "\n\tPROCHOT# Capable"));
  // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_PROC_HOT"));

  //
  // Check TM capable
  //
  if (mCpuid01.RegEdx & CPUXFF_TM1) {
    //This->PpmFlags |= PPM_TM1;
    This->PpmFlags |= PPM_TM;
    // DEBUG ((EFI_D_ERROR, "\n\tTM Capable"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_TM"));

  }

  //
  // Check GV3/ EIST capable, If EIST capable, also set the boot P-state to HFM flag.
  //
  if (mCpuid01.RegEcx & CPUXFF_GV3) {
    This->PpmFlags |= (PPM_GV3 | PPM_BOOT_P_ST_HFM) ;
    // DEBUG ((EFI_D_ERROR, "\n\tGV3 Capable"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_GV3 | PPM_BOOT_P_ST_HFM"));
  }

  /* @NOTE: This is not applicable to SLM in VLV.
  //
  // Check if GV3 capable before enabling TM2
  //
  if (This->PpmFlags & PPM_GV3) {
    //
    // Check TM2 capable
    //
    if (mCpuid01.RegEcx & CPUXFF_TM2) {
      This->PpmFlags |= PPM_TM2;
    }
  }
  */

  //
  // Check if anything has been disabled
  //
  PlatformId.Qword = AsmReadMsr64 (EFI_MSR_IA32_PLATFORM_ID);

  //
  // Check if EIST has been fuse disabled.
  //
  if (PlatformId.Qword & FB_GV3_DISABLED) {
    // This->PpmFlags &= ~(PPM_GV3 | PPM_TM2);
    This->PpmFlags &= ~(PPM_GV3 | PPM_BOOT_P_ST_HFM );
    // DEBUG ((EFI_D_ERROR, "\n\tGV3 fuse disabled."));
    // DEBUG ((EFI_D_ERROR, "\n\t\t-->~(PPM_GV3 | PPM_BOOT_P_ST_HFM )"));
  }

  //
  // Check if any type of automatic internal throttling has been fuse disabled
  //
  if (PlatformId.Qword & FB_THERMAL_THROT_DISABLED) {
    //This->PpmFlags &= ~(PPM_TM1 + PPM_TM2);
    This->PpmFlags &= ~(PPM_TM);
    // DEBUG ((EFI_D_ERROR, "\n\tAutomatic internal throttling fuse disabled.\n"));
    // DEBUG ((EFI_D_ERROR, "\n\t\t-->~(PPM_TM)"));
  }

  //
  // Disable CPUID limiting (and save current setting) if enabled
  // and enable MONITOR/MWAIT support
  //
  Ia32MiscEnable.Qword = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
  if (Ia32MiscEnable.Qword & LIMIT_CPUID) {
    Ia32MiscEnable.Qword &= ~LIMIT_CPUID;
    Ia32MiscEnable.Qword |= MONITOR_MWAIT_ENABLE;
    AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);

    CpuidLimitingEnabled = TRUE;
  }

  //
  // Read the CPUID values we care about.  We cannot use the stored
  // values because they may have changes since we disabled limiting
  // and enabled MONITOR/MWAIT
  //
  AsmCpuid (1, &Cpuid01.RegEax, &Cpuid01.RegEbx, &Cpuid01.RegEcx, &Cpuid01.RegEdx);
  AsmCpuid (5, &Cpuid05.RegEax, &Cpuid05.RegEbx, &Cpuid05.RegEcx, &Cpuid05.RegEdx);
  AsmCpuid (6, &Cpuid06.RegEax, &Cpuid06.RegEbx, &Cpuid06.RegEcx, &Cpuid06.RegEdx);

  //
  // Determine if the MONITOR/MWAIT instructions are supported.
  //
  if ((Cpuid01.RegEcx & CPUXFF_MONITOR_MWAIT && Cpuid05.RegEcx & MONITOR_MWAIT_EXTENSIONS)) {
    This->PpmFlags |= PPM_MWAIT_EXT;
    // DEBUG ((EFI_D_ERROR, "\n\tMONITOR/MWAIT instructions are supported"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_MWAIT_EXT"));
  }

  //
  // Determine the C-State and Enhanced C-State support present.
  // Monitor/MWAIT parameters function describes the numbers supported.
  //
  States = (Cpuid05.RegEdx >> 4) & 0xF;
  if (States >= CSTATE_SUPPORTED) {
    This->PpmFlags |= PPM_C1;
    // DEBUG ((EFI_D_ERROR, "\n\tCstate : C1"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_C1 + PPM_C1E"));
  }

  States = (Cpuid05.RegEdx >> 8) & 0xF;
  if (States >= ENHANCED_CSTATE_SUPPORTED) {
    This->PpmFlags |= PPM_C2 + PPM_C2E;
    // DEBUG ((EFI_D_ERROR, "\n\tCstate : C2 + C2E supported"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_C2 + PPM_C2E"));
  } else if (States == CSTATE_SUPPORTED) {
    This->PpmFlags |= PPM_C2;
    // DEBUG ((EFI_D_ERROR, "\n\tCstate : C2 supported"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_C2"));
  }

  States = (Cpuid05.RegEdx >> 12) & 0xF;
  if (States >= ENHANCED_CSTATE_SUPPORTED) {
    This->PpmFlags |= PPM_C3 + PPM_C3E;
    // DEBUG ((EFI_D_ERROR, "\n\tCstate : C3 + C3E supported"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_C3 + PPM_C3E"));
  } else if (States == CSTATE_SUPPORTED) {
    This->PpmFlags |= PPM_C3;
    // DEBUG ((EFI_D_ERROR, "\n\tCstate : C3 supported"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_C3"));
  }

  States = (Cpuid05.RegEdx >> 16) & 0xF;
  if (States >= ENHANCED_CSTATE_SUPPORTED) {
    This->PpmFlags |= PPM_C4 + PPM_C4E;
    // DEBUG ((EFI_D_ERROR, "\n\tCstate : C4 + C4E supported"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_C4 + PPM_C4E"));
  } else if (States == CSTATE_SUPPORTED) {
    This->PpmFlags |= PPM_C4;
    // DEBUG ((EFI_D_ERROR, "\n\tCstate : C4 supported"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_C4"));
  }

  States = (Cpuid05.RegEdx >> 24) & 0xF;
  if (States >= CSTATE_SUPPORTED) {
    This->PpmFlags |= PPM_C6 | PPM_C6S | PPM_C7;
    // DEBUG ((EFI_D_ERROR, "\n\tCstate : C6 supported"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_C6 | PPM_C6S"));
  }

  // Check if turbo mode is supported
  //
  Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLES);
  if (((Cpuid06.RegEax & TURBO_MODE_SUPPORTED) == 0) && ((Ia32MiscEnable.Dwords.High & DISABLE_MASTER_TURBO_MODE) == 0)) {
    //
    // Turbo Mode is not available in this physical processor package.
    // BIOS should not attempt to enable Turbo Mode via IA32_MISC_ENABLE MSR.
    // BIOS should show Turbo Mode as Disabled and Not Configurable.
    //
    // DEBUG ((EFI_D_ERROR, "\n\tTurbo mode : Not available"));

    //
    // Start: Workaround for sighting 4683480, 4683441 for Q8XA IVI SKU
    //
    mTurboModeNotAvailable = 1;
    //
    // End: Workaround for sighting 4683480, 4683441 for Q8XA IVI SKU
    //

  } else if (((Cpuid06.RegEax & TURBO_MODE_SUPPORTED) == 0) && ((Ia32MiscEnable.Dwords.High & DISABLE_MASTER_TURBO_MODE) != 0)) {
    //
    // Turbo Mode is available but globally disabled for the all logical
    // processors in this processor package.
    // BIOS can enable Turbo Mode by IA32_MISC_ENABLE MSR 1A0h bit [38] = 0.
    //
    This->PpmFlags |= PPM_TURBO;
    // DEBUG ((EFI_D_ERROR, "\n\tTurbo mode : Available but disabled"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_TURBO"));
  } else if ((Cpuid06.RegEax & TURBO_MODE_SUPPORTED) == TURBO_MODE_SUPPORTED) {
    //
    // Turbo Mode is factory-configured as available and enabled for all logical processors in this processor package.
    // This case handles the cases where turbo mode is enabled before PPM gets chance to enable it
    //
    This->PpmFlags |= PPM_TURBO;
    // DEBUG ((EFI_D_ERROR, "\n\tTurbo mode : Available and Enabled"));
    // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_TURBO"));
  }

  //
  // Restore the CPUID limit setting.
  //
  if (CpuidLimitingEnabled == TRUE) {
    Ia32MiscEnable.Qword = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
    Ia32MiscEnable.Qword |= LIMIT_CPUID;
    AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);
  }

  //
  // Set the T-states flag
  //
  This->PpmFlags |= PPM_TSTATES;
  // DEBUG ((EFI_D_ERROR, "\n\tTState: Default Enabled"));
  // DEBUG ((EFI_D_ERROR, "\n\t-->PPM_TSTATES"));

  //
  // Set the S0ix flag
  //
  This->PpmFlags |= PPM_S0ix;

  return EFI_SUCCESS;
}


VOID
ApSafeDisablePowerControl (
  IN OUT VOID        *Buffer
  )
/*++

Routine Description:

  This function will disable Power Control configuration.

Arguments:

  Buffer        Pointer to the function parameters passed in.
                (Pointer to the PPM_PROCESSOR_SUPPORT_PROTOCOL_2 instance.)

Returns:

  EFI_SUCCESS             Processor MSR setting is saved.

--*/
{
  MSR_REGISTER            PowerCtl;

  PowerCtl.Qword = AsmReadMsr64 (MSR_POWER_CTL);
  PowerCtl.Dwords.Low &= ~C1E_ENABLE;
  AsmWriteMsr64 (MSR_POWER_CTL, PowerCtl.Qword);


  return;
}


VOID
ApSafeEnablePowerControl (
  IN OUT VOID        *Buffer
  )
/*++

Routine Description:

  This function will Enable Power Control configuration.

Arguments:

  Buffer        Pointer to the function parameters passed in.
                (Pointer to the PPM_PROCESSOR_SUPPORT_PROTOCOL_2 instance.)

Returns:

  EFI_SUCCESS             Processor MSR setting is saved.

--*/
{
  MSR_REGISTER            PowerCtl;

  PowerCtl.Qword = AsmReadMsr64 (MSR_POWER_CTL);
  PowerCtl.Dwords.Low |= C1E_ENABLE;
  AsmWriteMsr64 (MSR_POWER_CTL, PowerCtl.Qword);

  return;
}

STATIC
EFI_STATUS
EnableCStatesValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN     UINT16                             C2IoAddress,
  IN     UINT16                             CsmIoAddress
  )
/*++

Routine Description:

  Enables C-State support as specified by the input flags on all logical
  processors and sets associated timing requirements in the chipset.

Arguments:

  This          Pointer to the protocol instance
  C2IoAddress   IO address to generate C2 states (PM base + 014 usually)
  CsmIoAddress  IO trap address for CSM generated Cx requests (see PMG_IO_BASE_ADDR and PMG_CST_CONFIG_CONTROL MSR)

Returns:

  EFI_SUCCESS   Processor C state support configured successfully.

--*/
{
  ENABLE_CSTATE_PARAMS  EnableCStateParameters;
  //MSR_REGISTER          TempMsr;
  //UINT16                CoreCount;
  // DEBUG ((EFI_D_ERROR, "\nEnableCStatesValleyview()"));
  // @NOTE: This code segment may not be valid in VLV due to I/O legacy support
  //        for C-states is no longer available. But the code is remained.
  //
  // Load the C-State parameters to pass to the core function.
  //
  EnableCStateParameters.This = This;
  EnableCStateParameters.C2IoAddress = C2IoAddress;
  EnableCStateParameters.CsmIoAddress = CsmIoAddress;
  //
  // Enable C-States on all logical processors.
  //
  RunOnAllLogicalProcessors(ApSafeEnableCStates, &EnableCStateParameters);

  //
  // During OS detection, we will switch back to SMI coordination if we have Win XP-SP1
  //
  //EnableCStateIoRedirectionValleyview (This);

  //
  // Configure supported enhanced C-states
  // C1E enable bit in Power Ctl MSR has package scope
  //
  //
  // Enable supported states
  //
  if (This->PpmFlags & PPM_C1E) {
    RunOnAllLogicalProcessors(ApSafeEnablePowerControl, This);
    // DEBUG ((EFI_D_ERROR, "\n\tMSR(1FC) before configuring C1E: %X %X", PowerCtl.Dwords.High, PowerCtl.Dwords.Low));
  } else {
    RunOnAllLogicalProcessors(ApSafeDisablePowerControl, This);
    // DEBUG ((EFI_D_ERROR, "\n\tMSR(1FC) after configuring C1E: %X %X\n", PowerCtl.Dwords.High, PowerCtl.Dwords.Low));

  }



  return EFI_SUCCESS;
}

VOID
ApSafeEnableCStates (
  IN OUT VOID        *Buffer
  )
/*++

Routine Description:

  Enable C-State support as specified by the input flags on a logical processor.
    Configure BIOS C1 Coordination (SMI coordination)
    Enable IO redirection coordination
    Choose proper coordination method
    Configure extended C-States

  This function must be MP safe.

Arguments:

  Buffer        Pointer to a ENABLE_CSTATE_PARAMS containing the necessary
                information to enable C-States

Returns:

  EFI_SUCCESS   Processor C-State support configured successfully.

--*/
{
  //MSR_REGISTER                      Ia32MiscEnable;
  MSR_REGISTER                      PmCfgCtrl;
  MSR_REGISTER                      IoCaptAddr;
//  MSR_REGISTER                      IoBaseAddr;
  MSR_REGISTER                      BblCrCtl3;
  PPM_PROCESSOR_SUPPORT_PROTOCOL_2  *This;
  UINT16                            C2IoAddress;
  UINT16                            CsmIoAddress;

  // DEBUG ((EFI_D_ERROR, "\nApSafeEnableCStates()"));
  //
  // Extract parameters from the buffer
  //
  This = ((ENABLE_CSTATE_PARAMS*) Buffer)->This;
  C2IoAddress = ((ENABLE_CSTATE_PARAMS*) Buffer)->C2IoAddress;
  CsmIoAddress = ((ENABLE_CSTATE_PARAMS*) Buffer)->CsmIoAddress;

  //
  // If C-states are disabled in setup, configure PACK_LIM in CLOCK_CST_CONFIG_CONTROL
  //
  if (!(This->PpmFlags & PPM_C_STATES)) {
    PmCfgCtrl.Qword = AsmReadMsr64 (MSR_PM_CFG_CTRL);
    PmCfgCtrl.Dwords.Low &= ~CSTATE_LIMIT_MASK;
    AsmWriteMsr64 (MSR_PM_CFG_CTRL, PmCfgCtrl.Qword);
    return;
  }

  //
  // Set c-state package limit to the highest C-state enabled
  //
  PmCfgCtrl.Qword = AsmReadMsr64 (MSR_PM_CFG_CTRL);
  PmCfgCtrl.Dwords.Low &= ~CSTATE_LIMIT_MASK;
  if (This->PpmFlags & PPM_C7) {
    PmCfgCtrl.Dwords.Low |= CSTATE_LIMIT_C7;
    PmCfgCtrl.Dwords.Low |= DYNAMIC_L2_ENABLE;
    /*
      When GV3 ratio is BELOW or EQUAL to this ratio, L2-reduction
      will be allowed in C6;
      When Ratio is ABOVE this point, Expand will be requested;
      If Ratio=0, then this feature is OFF (ratio will not be taken
      into account for L2-shrink decision making).
      */
    PmCfgCtrl.Dwords.Low &= ~L2_SHRINK_THRESHOLD_MASK;
    //Program L2_SHRINK_THRESH to Max Ratio
    if (This->PpmFlags & PPM_S0ix) {
      PmCfgCtrl.Dwords.Low |= mMinBusRatio << L2_SHRINK_THRESHOLD_OFFSET;
    } else {
      PmCfgCtrl.Dwords.Low |= mMaxBusRatio << L2_SHRINK_THRESHOLD_OFFSET;
    }

  }
  //If CPU support C6
  else if (This->PpmFlags & PPM_C6) {
    PmCfgCtrl.Dwords.Low |= CSTATE_LIMIT_C6;
    PmCfgCtrl.Dwords.Low |= DYNAMIC_L2_ENABLE;
    /*
      When GV3 ratio is BELOW or EQUAL to this ratio, L2-reduction
      will be allowed in C6;
      When Ratio is ABOVE this point, Expand will be requested;
      If Ratio=0, then this feature is OFF (ratio will not be taken
      into account for L2-shrink decision making).
      */
    PmCfgCtrl.Dwords.Low &= ~L2_SHRINK_THRESHOLD_MASK;
    //Program L2_SHRINK_THRESH to Max Ratio
    if (This->PpmFlags & PPM_S0ix) {
      PmCfgCtrl.Dwords.Low |= mMinBusRatio << L2_SHRINK_THRESHOLD_OFFSET;
    } else {
      PmCfgCtrl.Dwords.Low |= mMaxBusRatio << L2_SHRINK_THRESHOLD_OFFSET;
    }
  } else if (This->PpmFlags & PPM_C4) {
    PmCfgCtrl.Dwords.Low |= CSTATE_LIMIT_C4;
  } else if (This->PpmFlags & PPM_C2) {
    PmCfgCtrl.Dwords.Low |= CSTATE_LIMIT_C2;
  } else if (This->PpmFlags & PPM_C1) {
    PmCfgCtrl.Dwords.Low |= CSTATE_LIMIT_C1;
  }
  AsmWriteMsr64 (MSR_PM_CFG_CTRL, PmCfgCtrl.Qword);

  //
  // Valleyview specific configuration of I/O capture and I/O coordination SMI MSR
  // Configure the base port and range in the MSR to match LVL_X settings in ACPI tables
  //
  //
  // Set I/O capture base port and range
  //
  IoCaptAddr.Qword = AsmReadMsr64 (MSR_PMG_IO_CAPTURE_BASE);
  //
  // Mask off CST range and set the CST range
  //
  IoCaptAddr.Dwords.Low &= ~IO_CAPT_RANGE_MASK;
  if (This->PpmFlags & PPM_C7) {
    IoCaptAddr.Dwords.Low |= IO_CAPT_LVL4;
  } else if (This->PpmFlags & PPM_C6) {
    IoCaptAddr.Dwords.Low |= IO_CAPT_LVL3;
  } else if (This->PpmFlags & PPM_C4) {
    IoCaptAddr.Dwords.Low |= IO_CAPT_LVL2;
  }

  //
  // Set the base CST address
  //
  IoCaptAddr.Dwords.Low &= ~(IO_CAPT_LVL2_BASE_ADDR_MASK);
  IoCaptAddr.Dwords.Low |= C2IoAddress;

  AsmWriteMsr64 (MSR_PMG_IO_CAPTURE_BASE, IoCaptAddr.Qword);

  if (This->PpmFlags & PPM_C4) {
    //
    // Set the L2 Way Chunk Size to 1/4 before enabling Deep C4.
    // From Core 8 Release 57
    //
    BblCrCtl3.Qword = AsmReadMsr64 (MSR_BBL_CR_CTL3);
//  BblCrCtl3.Dwords.Low &= ~L2_WAY_CHUNK_SZ_MASK;
//  BblCrCtl3.Dwords.Low |= L2_WAY_CHUNK_SZ_32;
//  PPM_100. ; Set for 1 way chunk size.
    BblCrCtl3.Dwords.Low &= ~L2_WAY_CHUNK_SZ_MASK;
    BblCrCtl3.Dwords.Low |= L2_WAY_CHUNK_SZ_4;

//    BblCrCtl3.Dwords.Low &= ~L2_WAY_RED_MIN_MASK;
//    BblCrCtl3.Dwords.Low |= L2_WAY_RED_MIN_0_AUTO;

    AsmWriteMsr64 (MSR_BBL_CR_CTL3, BblCrCtl3.Qword);
  }
  return;
}

STATIC
EFI_STATUS
InitThermalValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT PPM_PLATFORM_POLICY_PROTOCOL       *PPMPolicy
  )
/*++

Routine Description:

  This will perform general thermal initialization other then TM1, TM2, or
  PROCHOT# on all logical processors.

Arguments:

  This            Pointer to the protocol instance

Returns:

  EFI_SUCCESS     General thermal initialization completed successfully

--*/
{
  MSR_REGISTER MsrVal;

  //
  // Run thermal code on all logical processors.
  //
  RunOnAllLogicalProcessors(ApSafeInitThermal, PPMPolicy);

  // core thermal initialization

  MsrVal.Qword = AsmReadMsr64 (MSR_CPU_THERM_CFG1);
  MsrVal.Dwords.Low = 0x00000305;
  AsmWriteMsr64(MSR_CPU_THERM_CFG1, MsrVal.Qword);

  MsrVal.Qword = AsmReadMsr64 (MSR_CPU_THERM_CFG2);
  MsrVal.Dwords.Low = 0x0405500D;
  AsmWriteMsr64(MSR_CPU_THERM_CFG2, MsrVal.Qword);

  MsrVal.Qword = AsmReadMsr64 (MSR_PACKAGE_POWER_SKU_UNIT);
  MsrVal.Dwords.Low = 0x00000505;
  AsmWriteMsr64(MSR_PACKAGE_POWER_SKU_UNIT, MsrVal.Qword);

  return EFI_SUCCESS;
}


VOID
ApSafeInitThermal (
  IN OUT VOID        *Buffer
  )
/*++

Routine Description:

  This will perform general thermal initialization other then TM1, TM2, or PROCHOT#.
  This currently disables TM2 if enabled by default.  EnableTm2Valleyview will be called
  if TM2 is desired.

Arguments:

  This          Pointer to the protocol instance
  PpmFlags      Processor power management feature flags

Returns:

  EFI_SUCCESS     General thermal initialization completed successfully

--*/
{
  MSR_REGISTER                      Ia32MiscEnable;
  MSR_REGISTER                      EbcHardPoweron;
  //MSR_REGISTER                      PicSensCfg;
  PPM_PLATFORM_POLICY_PROTOCOL      *This;

  // DEBUG ((EFI_D_ERROR, "\nApSafeInitThermal()"));
  //
  // Extract parameters from the buffer
  //
  This = (PPM_PLATFORM_POLICY_PROTOCOL *) Buffer;

  /* @NOTE: This is not applicable to SLM in VLV
  if (!(This->PpmFlags & PPM_TM2)) {
    //
    // Disable TM2 in the CPU MSR
    //
    Ia32MiscEnable.Qword = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
    Ia32MiscEnable.Qword &= ~TM2_ENABLE;
    AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);
  }
  */
  //
  // Configure Adaptive thermal monitor. IA32_MISC_ENABLE[3]
  //
  Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLES);
  Ia32MiscEnable.Dwords.Low &= ~TM_ENABLE;
  if (This->FunctionEnables.EnableTm) {
    Ia32MiscEnable.Dwords.Low |= TM_ENABLE;
  }

  AsmWriteMsr64 (MSR_IA32_MISC_ENABLES, Ia32MiscEnable.Qword);

  // @NOTE: Inherit similar setting from CDV to VLV.
  // Set the Stop-Enable (STEN) bit for Mermom so internal clocks stop during
  // AutoHalt or Stop Clock states.
  //
  EbcHardPoweron.Qword = AsmReadMsr64 (EFI_MSR_EBC_HARD_POWERON);
  EbcHardPoweron.Qword |= STOP_ENABLE;
  AsmWriteMsr64 (EFI_MSR_EBC_HARD_POWERON, EbcHardPoweron.Qword);

  /* @NOTE: This MSR is dropped in SLM core.
  PicSensCfg.Qword = AsmReadMsr64 (MSR_PIC_SENS_CFG);
  PicSensCfg.Dwords.Low &= ~(IMVP_OPTIMIZATION_DIS);
  AsmWriteMsr64 (MSR_PIC_SENS_CFG, PicSensCfg.Qword);
  */
  return;
}


STATIC
EFI_STATUS
EnableTmValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Enable TM1

Arguments:

  This          Pointer to the protocol instance

Returns:

  None

--*/
{

  //
  // Enable TM1 on all logical processors.
  //
  RunOnAllLogicalProcessors(ApSafeEnableTm, This);

  return EFI_SUCCESS;
}


VOID
ApSafeEnableTm (
  IN OUT VOID        *Buffer
  )
/*++

Routine Description:

  This function will enable TM.

  This function must be MP safe.

Arguments:

  Buffer        Pointer to the function parameters passed in.
                (Pointer to the PPM_PROCESSOR_SUPPORT_PROTOCOL_2 instance.)

Returns:

  EFI_SUCCESS             TM1 enabled

--*/
{
  /* @IMPORTANT NOTES:
   *  Enable Thermal Monitor features. Thermal throttling should take effect
   *  when operating thermal conditions are exceeded. TM2 style throttling is
   *  automatically engaged if GS3_EN is set. Otherwise, TM1 style throttling
   *  will be engaged. Thermal interrupts to the CPU should also be disabled
   *  if TM_EN is 0. Additionally, the Turbo range of operation should be
   *  disallowed when TM_EN=0.
   *  TM_EN can only be set if FB_THERM_THROT_DIS_FUSE=0.
   *    0 - Thermal throttling is disabled
   *    1 - Thermal throttling is enabled.
   */

  //PPM_PROCESSOR_SUPPORT_PROTOCOL_2  *This;
  //MSR_REGISTER                      PicSensCfg;
  MSR_REGISTER                      Ia32MiscEnable;

  // DEBUG ((EFI_D_ERROR, "\nApSafeEnableTm()"));
  //
  // Extract parameters from the buffer
  //
  //This = (PPM_PROCESSOR_SUPPORT_PROTOCOL_2*) Buffer;

  //
  // Enable TM1 in the CPU MSR
  //
  Ia32MiscEnable.Qword = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
  Ia32MiscEnable.Qword |= TM_ENABLE;
  AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);

  return;
}

#if 0
STATIC
EFI_STATUS
EnableTm2Valleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT FVID_TABLE                         *FvidPointer
  )
/*++

Routine Description:

  Enable TM2

Arguments:

  This          Pointer to the protocol instance
  PpmFlags      Processor power management feature flags

Returns:

  EFI_SUCCESS   TM2 enabled successfully

--*/
{
  //
  // Enable TM2 on all logical processors.
  //
  RunOnAllLogicalProcessors(ApSafeEnableTm2, This);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ApSafeEnableTm2 (
  IN OUT VOID        *Buffer
  )
/*++

Routine Description:

  Enables TM2 on a logical processor.

  This function must be MP safe.

Arguments:

  Buffer        Unused

Returns:

  EFI_SUCCESS   TM2 enabled successfully

--*/
{
  MSR_REGISTER                Ia32MiscEnable;

  //
  // Enable TM2 in the CPU MSR
  //
  Ia32MiscEnable.Qword = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
  Ia32MiscEnable.Qword |= TM2_ENABLE;
  AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);

  return EFI_SUCCESS;
}
#endif


STATIC
EFI_STATUS
EnableProcHotValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Enables the bi-directional PROCHOT# signal on all logical processors.

Arguments:

  This          Pointer to the protocol instance

Returns:

  EFI_SUCCESS   PROCHOT# configured successfully

--*/
{
  MSR_REGISTER  CpuThermCfg2;
  // DEBUG ((EFI_D_ERROR, "\nEnableProcHotValleyview()"));
  //
  // Enable PROCHOT# in the CPU MSR if TM is enabled,
  //  else disable it.
  //
  CpuThermCfg2.Qword = AsmReadMsr64 (MSR_CPU_THERM_CFG2);
  if (This->PpmFlags & (PPM_TM)) {
    CpuThermCfg2.Qword |= PHOT_ENABLE;
    // DEBUG ((EFI_D_ERROR, "\n\tEnable PROCHOT#"));
  } else {
    //
    // Clear the PPM_PROC_HOT flag so that the correct state is reflected.
    //
    This->PpmFlags &= ~PPM_PROC_HOT;
    CpuThermCfg2.Qword &= ~PHOT_ENABLE;
    // DEBUG ((EFI_D_ERROR, "\n\tDisable PROCHOT#"));
  }

  AsmWriteMsr64 (MSR_CPU_THERM_CFG2, CpuThermCfg2.Qword);

  return EFI_SUCCESS;
}


VOID
ApSafeDisableGv3 (
  IN OUT VOID        *Buffer
  )
/*++

Routine Description:

  Enables GV3 support in a logical processor.

  This function must be MP safe.

Arguments:

  Buffer      Pointer to the function parameters passed in.
              (Pointer to the PPM_PROCESSOR_SUPPORT_PROTOCOL_2 instance.)

Returns:

  EFI_SUCCESS

--*/
{
  MSR_REGISTER                        Ia32MiscEnable;

  //
  // Enable GV3 in the CPU MSR
  //
  Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLES);
  Ia32MiscEnable.Qword &= ~GV3_ENABLE;
  AsmWriteMsr64 (MSR_IA32_MISC_ENABLES, Ia32MiscEnable.Qword);

  return;
}


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
{
  MSR_REGISTER  Ia32PerfCtl;

  // DEBUG ((EFI_D_ERROR, "\nApSafeSetBootPState()"));
  Ia32PerfCtl.Qword = AsmReadMsr64 (MSR_IA32_PERF_CTL);
  Ia32PerfCtl.Qword &= ~P_STATE_TARGET_MASK;
  if ( mPpmProcessorSupportProtocolValleyview.PpmFlags & PPM_BOOT_P_ST_HFM ) {
    // Set to HFM
    Ia32PerfCtl.Qword |= LShiftU64 (mMaxBusRatio, P_STATE_TARGET_OFFSET);
    Ia32PerfCtl.Qword |= mMaxVid, 0xFF;
    // DEBUG ((EFI_D_ERROR, "\n\tBooting with HFM)"));
  } else {
    // Set to LFM
    Ia32PerfCtl.Qword |= LShiftU64 (mMinBusRatio, P_STATE_TARGET_OFFSET);
    Ia32PerfCtl.Qword |= mMinVid, 0xFF;
    // DEBUG ((EFI_D_ERROR, "\n\tBooting with LFM)"));
  }
  AsmWriteMsr64 (MSR_IA32_PERF_CTL, Ia32PerfCtl.Qword);
  return;
}

STATIC
EFI_STATUS
SetBootPState (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Set processor P state to HFM or LFM.

Arguments:

  This          Pointer to the protocol instance

Returns:
  EFI_UNSUPPORTED EIST not supported.
  EFI_SUCCESS     Processor P state has been set.

--*/
{
  MSR_REGISTER  Ia32MiscEnable;
  BOOLEAN       EistEnabled;

  // DEBUG ((EFI_D_ERROR, "\nSetBootPState()"));
  //
  // This function will be executed when EIST is enabled and EIST is capable
  // So processor can be switched to HFM
  //
  if ((mCpuid01.RegEcx & CPUXFF_GV3) == 0) {
    return EFI_UNSUPPORTED;
  }

  //
  // Enable EIST
  //
  Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLES);
  EistEnabled = (BOOLEAN)RShiftU64((Ia32MiscEnable.Qword & GV3_ENABLE),GV3_ENABLE_OFFSET);
  //
  // Check whether EIST is disabled in setup.
  //
  if ((This->PpmFlags & PPM_GV3) == 0)  {
    EistEnabled = 0;
  }
  //
  // If EIST is disabled, temporarily enable it
  //
  if (EistEnabled == 0) {
    RunOnAllLogicalProcessors (ApSafeEnableGv3, This);
  }

  //
  // Set P-state to HFM on all cores
  //
  RunOnAllLogicalProcessors (ApSafeSetBootPState, This);

  //
  // Disable EIST if we enabled it previously
  //
  if (EistEnabled == 0) {
    RunOnAllLogicalProcessors (ApSafeDisableGv3, This);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EnableTscValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT FVID_TABLE                         *FvidPointer,
  IN     UINT16                             iFSBFrequency,
  IN OUT UINT8                              *PpmCstTmrFlags,
  IN OUT UINTN                              *PpmTscCorrFactor,
  IN OUT UINTN                              *PpmTscCorrFactorRem
  )
/*++

Routine Description:

  Initialize the TSC support.

Arguments:

  This                                 Pointer to the protocol instance
  FvidPointer                          Pointer to a table to be updated
  iFSBFrequency                        Processor core iFSB Frequency
  PpmCstTmrFlags                       Flag of update TSC MSR if C3 or C4
  PpmTscCorrFactor                     TSC correction factor
  PpmTscCorrFactorRem                  TSC correction factor remainder

Returns:

  EFI_SUCCESS   Processor TSC support configured successfully.

--*/
{
  UINT64              FrequencyId;

  // DEBUG ((EFI_D_ERROR, "\nEnableTscValleyview()"));
  //
  // Initialize the TSC update variables for Valleyview
  //
  *PpmCstTmrFlags = CST_UPDT_TSC;

  //
  // Initialize the FVID tables, so that the maximum ratio setting is identified.
  //
  InitFvidTableValleyview (This, FvidPointer, FVID_MAX_STATES, FVID_MIN_STEP_SIZE, FALSE);
  ASSERT (FvidPointer->FvidHeader.Gv3States != 0);

  //
  // Get the maximum frequency.
  //

  FrequencyId = mMaxBusRatio;
  //
  // Direct multiply Core IFSB with Max Ratio
  FrequencyId = MultU64x32 (FrequencyId, iFSBFrequency);

  //
  // Divide by timer base frequency
  // Save value and remainder
  //
  *PpmTscCorrFactor = (UINTN) DivU64x32Remainder (FrequencyId, CST_DATA_TBASE, (UINT32 *)PpmTscCorrFactorRem);
  //*PpmTscCorrFactor = (UINTN) DivU64x32 (FrequencyId, CST_DATA_TBASE);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
S3SaveMsrValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Save processor MSR runtime settings for S3.

Arguments:

  This          Pointer to the protocol instance

Returns:

  EFI_SUCCESS   Processor MSR setting is saved.

--*/
{
  UINT32   Index;

  // DEBUG ((EFI_D_ERROR, "\nS3SaveMsrValleyview()"));

  for (Index = 0; Index < sizeof (mMsrValues) / sizeof (EFI_MSR_VALUES); Index++) {
    // DEBUG ((EFI_D_ERROR, "\n\tSaving MSR(%X) = %X", mMsrValues[Index].Index,  mMsrValues[Index].Value));
    mMsrValues[Index].Value = AsmReadMsr64 (mMsrValues[Index].Index);
  }

  return  EFI_SUCCESS;
}

STATIC
EFI_STATUS
S3RestoreMsrValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Restore processor MSR runtime settings for S3.

Arguments:

  This          Pointer to the protocol instance

Returns:

  EFI_SUCCESS   Processor MSR setting is restored.

--*/
{
  //
  // Restore MSR's on all logical processors.
  //
  RunOnAllLogicalProcessors(ApSafeRestoreMsr, This);

  return EFI_SUCCESS;
}


VOID
ApSafeRestoreMsr (
  IN OUT VOID        *Buffer
  )
/*++

Routine Description:

  This function will restore MSR settings.

  This function must be MP safe.

Arguments:

  Buffer        Unused

Returns:

  EFI_SUCCESS   MSR restored

--*/
{
  UINT32  Index;

  // DEBUG ((EFI_D_ERROR, "\nApSafeRestoreMsr() for S3"));

  for (Index = 0; Index < sizeof (mMsrValues) / sizeof (EFI_MSR_VALUES); Index++) {
    // DEBUG ((EFI_D_ERROR, "\n\tRestoring MSR(%X) = %X", mMsrValues[Index].Index,  mMsrValues[Index].Value));
    AsmWriteMsr64 (mMsrValues[Index].Index, mMsrValues[Index].Value);
  }

  AsmWriteMsr64 (MSR_POWER_MISC, AsmReadMsr64(MSR_POWER_MISC) | ENABLE_IA_UNTRUSTED_MODE);
  return;
}

STATIC
EFI_STATUS
EnableCStateIoRedirectionValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Enables IO redirection C-State control on all logical processors.
  This should only be enabled if the OS and processor driver support
  independent C-State control for C2 and higher.

  This will cause the processor to capture LVL_X reads and perform the requested
  C-State transition.

  This cannot be enabled when SMI based C-State coordination is enabled,
  so this function will disable SMI based C-State coordination.

Arguments:

  This          Pointer to the protocol instance

Returns:

  EFI_SUCCESS   Processor IO redirection C-State control enabled.

--*/
{
  //
  // Enable C-State I/O redirection on all logical processors.
  //
  RunOnAllLogicalProcessors(ApSafeEnableCStateIoRedirection, This);

  return EFI_SUCCESS;
}


VOID
ApSafeEnableCStateIoRedirection (
  IN OUT VOID        *Buffer
  )
/*++

Routine Description:

  Enables C-State I/O redirection on a logical processor.

  This function must be MP safe.

Arguments:

  Buffer        Unused

Returns:

  EFI_SUCCESS   Processor IO redirection C-State control enabled.

--*/
{
  MSR_REGISTER    PmCfgCtrl;

  //
  // Enable I/O redirection control
  //
  PmCfgCtrl.Qword = AsmReadMsr64 (MSR_PM_CFG_CTRL);
  PmCfgCtrl.Dwords.Low |= MWAIT_IO_REDIR;
  AsmWriteMsr64 (MSR_PM_CFG_CTRL, PmCfgCtrl.Qword);

  return;
}

STATIC
EFI_STATUS
DisableCStateIoRedirectionValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Disables C-State IO redirection on all logical processors.

Arguments:

  This          Pointer to the protocol instance

Returns:

  EFI_SUCCESS   Processor IO redirection C-State control disabled.

--*/
{
  //
  // Disable C-State I/O redirection on all logical processors.
  //
  RunOnAllLogicalProcessors(ApSafeDisableCStateIoRedirection, This);

  return EFI_SUCCESS;
}


VOID
ApSafeDisableCStateIoRedirection (
  IN OUT VOID        *Buffer
  )
/*++

Routine Description:

  Disables C-State IO redirection on a logical processor.

  This function must be MP safe.

Arguments:

  Buffer        Unused

Returns:

  EFI_SUCCESS   Processor IO redirection C-State control disabled.

--*/
{
  MSR_REGISTER    PmCfgCtrl;

  //
  // Disable I/O redirection C-State control
  //
  PmCfgCtrl.Qword = AsmReadMsr64 (MSR_PM_CFG_CTRL);
  PmCfgCtrl.Dwords.Low &= ~MWAIT_IO_REDIR;
  AsmWriteMsr64 (MSR_PM_CFG_CTRL, PmCfgCtrl.Qword);

  return;
}

STATIC
VOID
ApSafeDisableTurboMode (
  IN OUT VOID      *Buffer
  )
/*++

Routine Description:

  - Disable Turbo Mode at package level

Arguments:

  This          Pointer to the protocol instance
  PPMPolicy     Pointer to policy protocol instance

Returns:

  None

--*/
{
  MSR_REGISTER    Ia32MiscEnableMsr;

  //
  // Set Turbo Mode disable bit in IA32 Misc Enable MSR
  //
  Ia32MiscEnableMsr.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLES);
  Ia32MiscEnableMsr.Dwords.High |= DISABLE_MASTER_TURBO_MODE;
  AsmWriteMsr64 (MSR_IA32_MISC_ENABLES, Ia32MiscEnableMsr.Qword);
}


STATIC
VOID
ApSafeEnableTurboMode (
  IN OUT VOID      *Buffer
  )
/*++

Routine Description:

  - Enables Turbo Mode at package level

Arguments:

  This          Pointer to the protocol instance
  PPMPolicy     Pointer to policy protocol instance

Returns:

  None

--*/
{
  MSR_REGISTER    Ia32MiscEnableMsr;
  /*MSR_REGISTER    TempMsr;
  UINT16          Turbo4Cratio; */
  //
  // Clear Turbo Mode disable bit in IA32 Misc Enable MSR
  //
  Ia32MiscEnableMsr.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLES);
  Ia32MiscEnableMsr.Dwords.High &= ~DISABLE_MASTER_TURBO_MODE;
  AsmWriteMsr64 (MSR_IA32_MISC_ENABLES, Ia32MiscEnableMsr.Qword);
  /*//
  // Temp WA, Max Ratio-2, requested from Punit team.
  //
  TempMsr.Qword    = AsmReadMsr64 (MSR_IACORE_TURBO_RATIOS);
  Turbo4Cratio  = ((UINT32)(TempMsr.Dwords.Low & MAX_RATIO_4C_MASK))>>24;
  if((Turbo4Cratio - mMaxBusRatio) >= 2)
  {
    AsmWriteMsr64 (MSR_TURBO_RATIO_LIMIT, 0x02020202);
  } */
}


STATIC
EFI_STATUS
InitGv3Valleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT FVID_TABLE                         *FvidPointer,
  IN OUT PPM_PLATFORM_POLICY_PROTOCOL       *PPMPolicy
  )
/*++

Routine Description:

  Initializes required structures for P-State table creation and enables GV3
  support in the processor.

Arguments:

  This          Pointer to the protocol instance
  FvidPointer   Table to update, must be initialized.
  PPMPolicy     Pointer to policy protocol instance

Returns:

  EFI_SUCCESS

--*/
{
  MSR_REGISTER    TempMsr;
// MSR_REGISTER    FlexRatioMsr;
  MSR_REGISTER    PlatformIdMsr;

  // DEBUG ((EFI_D_ERROR, "\nInitGv3Valleyview) for S3"));

  PlatformIdMsr.Qword = AsmReadMsr64 (EFI_MSR_IA32_PLATFORM_ID);
  // If not fuse disabled, then the Max ratio is programmable.
  // if (!(PlatformIdMsr.Dwords.Low & RATIO_LOCKED) && (PPMPolicy->FlexRatioVid & 0x10000)) {
  //   	 if ((PPMPolicy->FlexRatioVid & 0xFFFF) != 0) {
  //     FlexRatioMsr.Qword = PPMPolicy->FlexRatioVid;
  //     AsmWriteMsr64 (MSR_FLEX_RATIO, FlexRatioMsr.Qword);
  //     // DEBUG ((EFI_D_ERROR, "\n\tRatio not locked!. Programming FlexRatioVid MSR(0x194) = %X", FlexRatioMsr.Qword));
  //    }
  // }


  //
  // Test for Turbo Mode supported and initialize if true.
  //

  //
  // Start: Workaround for sighting 4683480, 4683441 for Q8XA IVI SKU
  //
  if(mTurboModeNotAvailable==0) {
    //
    // End: Workaround for sighting 4683480, 4683441 for Q8XA IVI SKU
    //
    if (This->PpmFlags & PPM_TURBO) {
      RunOnAllLogicalProcessors (ApSafeEnableTurboMode, This);
    } else {
      RunOnAllLogicalProcessors (ApSafeDisableTurboMode, This);
    }
    //
    // Start: Workaround for sighting 4683480, 4683441 for Q8XA IVI SKU
    //
  }
  //
  // End: Workaround for sighting 4683480, 4683441 for Q8XA IVI SKU
  //

  //
  // Initialize the FVID tables.
  //
  InitFvidTableValleyview (This, FvidPointer, FVID_MAX_STATES, FVID_MIN_STEP_SIZE, FALSE);
  ASSERT (FvidPointer->FvidHeader.Gv3States != 0);

  //
  // Enable GV3 on all logical processors.
  //
  RunOnAllLogicalProcessors(ApSafeEnableGv3, This);

  //
  // Program Primary Power Plane Current Limit's
  //
  if( PPMPolicy->PrimaryPlaneCurrentLimit != AUTO ) {
    TempMsr.Qword = AsmReadMsr64 (MSR_PRIMARY_PLANE_CURRENT_CONFIG);
    TempMsr.Dwords.Low &= ~CURRENT_LIMIT_MASK;
    TempMsr.Dwords.Low |= PPMPolicy->PrimaryPlaneCurrentLimit;
    AsmWriteMsr64 (MSR_PRIMARY_PLANE_CURRENT_CONFIG, TempMsr.Qword);
  }
  return EFI_SUCCESS;
}


VOID
ApSafeEnableGv3 (
  IN OUT VOID        *Buffer
  )
/*++

Routine Description:

  Enables GV3 support in a logical processor.

  This function must be MP safe.

Arguments:

  Buffer      Pointer to the function parameters passed in.
              (Pointer to the PPM_PROCESSOR_SUPPORT_PROTOCOL_2 instance.)

Returns:

  EFI_SUCCESS

--*/
{
  PPM_PROCESSOR_SUPPORT_PROTOCOL_2    *This;
  MSR_REGISTER                        Ia32MiscEnable;
  MSR_REGISTER                        PmCfgCtrl;
  MSR_REGISTER                        PowerMisc;

  // DEBUG ((EFI_D_ERROR, "\nApSafeEnableGv3()"));
  //
  // Extract parameters from the buffer.
  //
  This = (PPM_PROCESSOR_SUPPORT_PROTOCOL_2*) Buffer;

  //
  // Enable GV3 in the CPU MSR
  //
  Ia32MiscEnable.Qword = AsmReadMsr64 (MSR_IA32_MISC_ENABLES);
  Ia32MiscEnable.Qword |= GV3_ENABLE;
  AsmWriteMsr64 (MSR_IA32_MISC_ENABLES, Ia32MiscEnable.Qword);

  PowerMisc.Qword = AsmReadMsr64 (MSR_POWER_MISC);

  // If CMP is disabled, disable hardware coordination.
  //
  if (!(This->PpmFlags & PPM_CMP)) {
    PmCfgCtrl.Qword = AsmReadMsr64 (MSR_PM_CFG_CTRL);
    PmCfgCtrl.Qword |= HW_COORD_DIS;
    AsmWriteMsr64 (MSR_PM_CFG_CTRL, PmCfgCtrl.Qword);
    // DEBUG ((EFI_D_ERROR, "\n\tGV3 - Disable Hardware Coordination."));
  }

  return;
}

STATIC
EFI_STATUS
InitFvidTableValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT FVID_TABLE                         *FvidPointer,
  IN     UINT16                             MaxNumberOfStates,
  IN     UINT16                             MinStepSize,
  IN     BOOLEAN                            CreateDefaultTable
  )
/*++

Routine Description:

  This function updates the table provided with the FVID data for the processor.
  If CreateDefaultTable is TRUE, a minimam FVID table will be provided.
  The maximum number of states must be greater then or equal to two.
  The table should be initialized in such a way as for the caller to determine if the
  table was updated successfully.  This function should be deprecated in the future when
  Release 8 is integrated in favor of the EIST protocol calculating FVID information.

Arguments:

  This                Pointer to the protocol instance
  FvidPointer         Pointer to a table to be updated
  MaxNumberOfStates   Number of entries in the table pointed to by FvidPointer
  MinStepSize         Minimum step size for generating the FVID table
  CreateDefaultTable  Create default FVID table rather then full state support

Returns:

  EFI_SUCCESS     Table pointed to FvidPointer updated with desired FVID states

--*/
{
  EFI_STATUS    Status;
//  MSR_REGISTER  PerformanceStatusMsr;
  MSR_REGISTER  PlatformIdMsr;
  MSR_REGISTER  IaCoreRatios;
  MSR_REGISTER  IaCoreVids;
//  MSR_REGISTER  FsbClockVcc;
  MSR_REGISTER  ClockFlexMax;

  // DEBUG ((EFI_D_ERROR, "\nInitFvidTableValleyview()"));
  //
  // If the FVID tables have already been created, return.
  //
  if (FvidPointer[0].FvidHeader.Gv3States != 0) {
    return EFI_SUCCESS;
  }
  PlatformIdMsr.Qword = AsmReadMsr64 (EFI_MSR_IA32_PLATFORM_ID);
  /*
  //
  // Get the Bus Ratio and VID information for the processor
  //
  //
  // Maximum VID is in Platform ID MSR bits 5:0 MAXIMUM_VCC_VID and bit 23 MAXIMUM_VCC_VID_BIT6; bus ratio is bits 12:8
  //


  mMaxVid |= PlatformIdMsr.Bytes.FirstByte & VID_MAX_PREFLX_BYTE_MASK;
  mMaxBusRatio = PlatformIdMsr.Bytes.SecondByte & RATIO_MAX_PFLX_BYTE_MASK;
  */

  IaCoreRatios.Qword = AsmReadMsr64 (MSR_IACORE_RATIOS);
  mMaxBusRatio  = IaCoreRatios.Bytes.ThirdByte;

  //
  // Get Maximum Efficiency bus ratio (LFM) from IACORE_RATIOS MSR Bits[15:8]
  //
  mMinBusRatio = IaCoreRatios.Bytes.SecondByte;

  //
  // Get the Vid for the processor
  //
  //
  // Get Maximum Non-Turbo Vid (HFM) from IACORE_VIDS MSR Bits[23:16]
  //
  IaCoreVids.Qword = AsmReadMsr64 (MSR_IACORE_VIDS);
  mMaxVid  = IaCoreVids.Bytes.ThirdByte;

  //
  // Get Maximum Efficiency VID (LFM) from IACORE_VIDS MSR Bits[15:8]
  //
  mMinVid = IaCoreVids.Bytes.SecondByte;

  //
  // If flex settings are enabled, they must be used in place of max. Ratio/VID
  // defined in the MSR_PLATFORM_ID MSR used above.  In addition, if flex settings are
  // enabled, the maximum frequency is limited an Turbo Mode is unavailable.
  //
  // NOTE: This code does not provide facilities to program the flex settings
  // as such code would best be suited for the PEI phase, as a processor
  // reset may be required after programming.
  //
  if (!(PlatformIdMsr.Dwords.Low & RATIO_LOCKED)) {
    ClockFlexMax.Qword = AsmReadMsr64 (MSR_FLEX_RATIO);
    if (ClockFlexMax.Dwords.Low & ENABLE_FLEX) {
      This->PpmFlags &= ~(PPM_TURBO);
      mMaxVid = ClockFlexMax.Bytes.FirstByte & VID_FLEX_BYTE_MASK;
      mMaxBusRatio = ClockFlexMax.Bytes.SecondByte & RATIO_FLEX_BYTE_MASK;
    }
  }
  /*
  //
  // Minimum VID is in MSR_FSB_CLOCK_VCC  bits 54:48
  //
  PerformanceStatusMsr.Qword = AsmReadMsr64 (MSR_FSB_CLOCK_VCC);
  mMinVid = PerformanceStatusMsr.Bytes.SeventhByte & VID_MIN_FUSE_MASK;
  //
  // Minimum Ratio is in MSR_IA32_PERF_STS bits 28:24
  //
  PerformanceStatusMsr.Qword = AsmReadMsr64 (MSR_IA32_PERF_STS);
  mMinBusRatio  = PerformanceStatusMsr.Bytes.FouthByte & BUS_RATIO_MIN_MASK;
  */

  //
  // Create FVID table
  //
  if (CreateDefaultTable) {
    CreateDefaultFvidTable (FvidPointer);
    This->PpmFlags &= ~(PPM_TURBO | PPM_DYNAMIC_FSB);
  } else {
    Status = CreateFvidTable (This, FvidPointer, MaxNumberOfStates, MinStepSize);
    if (EFI_ERROR (Status)) {
      CreateDefaultFvidTable (FvidPointer);
      This->PpmFlags &= ~(PPM_TURBO | PPM_DYNAMIC_FSB);
    }
  }

  return EFI_SUCCESS;
}

STATIC
VOID
CreateDefaultFvidTable (
  IN OUT FVID_TABLE     *FvidPointer
  )
/*++

Routine Description:

  Create default FVID table with max and min states only.

Arguments:

  FvidPointer         Pointer to a table to be updated

Returns:

  None.

--*/
{

  //
  // Fill in the header.
  //
  FvidPointer[0].FvidHeader.Stepping     = (mCpuid01.RegEax & CPUID_FULL_FAMILY_MODEL_STEPPING);
  FvidPointer[0].FvidHeader.MaxVid       = mMaxVid;
  FvidPointer[0].FvidHeader.MaxBusRatio  = mMaxBusRatio;
  FvidPointer[0].FvidHeader.Gv3States    = 2;

  //
  // First entry is state 0, highest state.
  //
  FvidPointer[1].FvidState.State     = 0;
  FvidPointer[1].FvidState.Vid       = mMaxVid;
  FvidPointer[1].FvidState.BusRatio  = mMaxBusRatio;
  FvidPointer[1].FvidState.Power     = FVID_MAX_POWER_MIDVIEW;  // @TODO: Need to update this constant. Currently unknown.
  //
  // power is calculated in milliwatts
  //
  //FvidPointer[1].FvidState.Power = (mPackageTdp * 1000);

  //
  // Second entry is state 1, lowest state.
  //
  FvidPointer[2].FvidState.State     = 1;
  FvidPointer[2].FvidState.Vid       = (UINT16) mMinVid;
  FvidPointer[2].FvidState.BusRatio  = (UINT16) mMinBusRatio;
  FvidPointer[2].FvidState.Power     = FVID_MIN_POWER_MIDVIEW;  // @TDO: Need to update this constant. Currently unknown.

  /* @NOTE: Example of relative calculation for Power Estimates
  //
  // Relative Power calculation section 16.6.4 NHM BWG
  //
  M = (mMaxBusRatio - FvidPointer[2].FvidState.BusRatio) * 625;
  M = (110000 - M);
  M = DivU64x32 (M, 11);
  M = DivU64x32 (MultU64x64 (M, M), 1000);

  //
  // power is calculated in milliwatts
  //
  W = (((FvidPointer[2].FvidState.BusRatio * 100000) / mMaxBusRatio) / 100);
  W = DivU64x32 (MultU64x32 (MultU64x64 (W, DivU64x32 (M, 100)), mPackageTdp), 1000);
  FvidPointer[2].FvidState.Power = (UINT16) W;
  */
}

STATIC
EFI_STATUS
CreateFvidTable (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT FVID_TABLE     *FvidPointer,
  IN     UINT16         MaxNumberOfStates,
  IN     UINT16         MinStepSize
  )
/*++

Routine Description:

  Create an FVID table based on the algorithm provided by the BIOS writer's guide.

Arguments:

  This                Pointer to the protocol instance
  FvidPointer         Pointer to a table to be updated
  MaxNumberOfStates   Number of entries in the table pointed to by FvidPointer
  MinStepSize         Minimum step size for generating the FVID table

Returns:

  EFI_SUCCESS           FVID table created successfully.
  EFI_INVALID_PARAMETER The VID and/or bus ratio ranges don't permit FVID table calculation;
                        a default FVID table should be constructed.

--*/
{
  UINT16                BusRatioRange;
  UINT16                PowerRange;
  UINT16                StepSize;
  UINT16                NumberOfStates;
  UINT16                CurrentBusRatio;
  UINT16                i;
  UINT16                Turbo;
//  UINT64                M;
//  UINT64                W;
  UINT16                BusRatioRangeX2;
  UINT16                VidRange;
  UINT16                ReducedStep =0;
  DEBUG((EFI_D_ERROR,"\n\n   ==  ==  CreateFvidTable  ==  ==\n\n"));

  //
  // Determine the bus ratio range
  //
  BusRatioRange = mMaxBusRatio - mMinBusRatio;
  if ( ((INT16) BusRatioRange < 0) || ( MaxNumberOfStates == 0 )) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Determine the bus ratio range (Mask off the N/2 bit.)
  //
  BusRatioRange = (mMaxBusRatio & RATIO_BYTE_MASK)  - mMinBusRatio;
  if ((INT16)BusRatioRange < 0) {
    return EFI_INVALID_PARAMETER;
  }
  Turbo = ((This->PpmFlags & PPM_TURBO) ? 1 : 0);
  //
  // Determine step size desired
  // Step size is BusRatioRange / max states, rounded down to an even state
  //
  StepSize = MinStepSize;

  DEBUG((EFI_D_ERROR, "BusRatioRange:[%04x]      StepSize:[%04x]      Turbo:[%04x]\n", BusRatioRange, StepSize, Turbo));

  //
  // Determine the number of states
  //  No Of States =  (BusRatioRange / step size) + 1; Add one to BusRatioRange to account for LFM
  //     eg. LFM = 8 HFM = 10, BusRatioRange = 2, StepSize = 1 No Of States = (2 / 1) + 1 = 3 (8, 9, 10)
  //   For No of States above 16...
  //     StepSize = (No of States / Max No of States) + 1
  //     eg. LFM = 8 HFM = 28, BusRatioRange = 20
  //        StepSize = (21 / 16) + 1 = 2
  //        No Of States = (20 / 2) + 1 = 11 (8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28)
  //
  NumberOfStates = ( BusRatioRange / StepSize ) + 1;
  DEBUG((EFI_D_ERROR, "NumberOfStates:[%04x]\n", NumberOfStates));

  if (NumberOfStates >= (MaxNumberOfStates - 1) ) {
    //
    // Too many P-states, so let's change the StepSize to limit the P-state table size
    //
    StepSize = ( NumberOfStates / MaxNumberOfStates ) + 1;
    NumberOfStates = ( BusRatioRange / StepSize ) + 1;
  }

  //
  // Ensure we have at least two states
  //
  if ((NumberOfStates == 0) && (BusRatioRange != 0)) {
    //
    // If the bus ratio range is smaller than step size then
    // initialize to minimum number of states
    //
    NumberOfStates = 1;
  }

  DEBUG((EFI_D_ERROR, "BusRatioRange:[%04x]      StepSize:[%04x]      Turbo:[%04x]\n", BusRatioRange, StepSize, Turbo));

  //
  // Create a bus ratio range multiplied by 2 to take N/2 into account in calculations.
  //
  BusRatioRangeX2 = BusRatioRange * 2;

  //
  // Determine the VID range
  // Do not check for VID range to be zero because some of the ULV and LV processors
  // will have the same HFM and LFM VID values
  //
  VidRange = mMaxVid - mMinVid;

  //
  // Determine the Power range
  //
  PowerRange = FVID_MAX_POWER_MIDVIEW - FVID_MIN_POWER_MIDVIEW;
/*
  //
  // Determine step size desired
  // Step size is BusRatioRange / max states, rounded down to an even state
  // because step size must be at least 2 and a multiple of 2.
  //
  StepSize = BusRatioRange / (MaxNumberOfStates - 1);
  StepSize = StepSize - (StepSize & 0x01);
  if (StepSize < MinStepSize) {
    StepSize = MinStepSize;
  }

  //
  // Determine the number of states
  // Equal to the range / step size
  //
  NumberOfStates = BusRatioRange / StepSize;
  if (NumberOfStates >= MaxNumberOfStates) {
    NumberOfStates = MaxNumberOfStates - 1;
  }

  //
  // Ensure we have at least two states
  //
  if ((NumberOfStates == 0) && (BusRatioRangeX2 != 0)) {
    //
    // If the bus ratio range is smaller than step size then
    // initialize to minimum number of states
    //
    NumberOfStates = 1;
  }

  NumberOfStates++;
*/
  //
  // Fill in the table header
  //
  FvidPointer[0].FvidHeader.Stepping     = (mCpuid01.RegEax & CPUID_FULL_FAMILY_MODEL_STEPPING);
  FvidPointer[0].FvidHeader.MaxVid       =  (Turbo ? mTurboVid : mMaxVid);
  FvidPointer[0].FvidHeader.MaxBusRatio  = mMaxBusRatio;
  FvidPointer[0].FvidHeader.Gv3States    =  (UINT16) (NumberOfStates < MaxNumberOfStates ? (NumberOfStates + Turbo) : NumberOfStates);

  DEBUG ((EFI_D_ERROR, "FVID[0].FvidHeader.MaxVid = %x\n", FvidPointer[0].FvidHeader.MaxVid));
  DEBUG ((EFI_D_ERROR, "FVID[0].FvidHeader.MaxBusRatio = %x\n", FvidPointer[0].FvidHeader.MaxBusRatio));
  DEBUG ((EFI_D_ERROR, "FVID[0].FvidHeader.Gv3States = %x\n\n", FvidPointer[0].FvidHeader.Gv3States));

  if (This->PpmFlags & PPM_TURBO) {
    //
    // Fill in the first state
    //
    FvidPointer[1].FvidState.State     = 0;
    FvidPointer[1].FvidState.Vid       = mTurboVid;
    FvidPointer[1].FvidState.BusRatio  = mTurboBusRatio;
    //
    // power is calculated in milliwatts
    //
    //FvidPointer[1].FvidState.Power = mPackageTdp;
    FvidPointer[1].FvidState.Power     = FVID_MAX_POWER_MIDVIEW;

    DEBUG ((EFI_D_ERROR, "FVID[1].State = %x\n", FvidPointer[1].FvidState.State));
    DEBUG ((EFI_D_ERROR, "FVID[1].BusRatio = %x\n", FvidPointer[1].FvidState.BusRatio));
    DEBUG ((EFI_D_ERROR, "FVID[1].vid = %x\n", FvidPointer[1].FvidState.Vid));
    DEBUG ((EFI_D_ERROR, "FVID[1].Power = %x\n\n", FvidPointer[1].FvidState.Power));

    ///
    /// Reserve on P-State for Max Turbo
    ///
    if (NumberOfStates == MaxNumberOfStates) {
      NumberOfStates--;
    }

  }
  //
  // Fill in the first state
  //
  FvidPointer[1 + Turbo].FvidState.State     = Turbo;
  FvidPointer[1 + Turbo].FvidState.Vid       = mMaxVid;
  FvidPointer[1 + Turbo].FvidState.BusRatio  = mMaxBusRatio;
  //
  // power is calculated in milliwatts
  //
  //FvidPointer[1 + Turbo].FvidState.Power = mPackageTdp;
  FvidPointer[1 + Turbo].FvidState.Power     = FVID_MAX_POWER_MIDVIEW;

  DEBUG ((EFI_D_ERROR, "FVID[%02d].State = %x\n", (1 + Turbo), FvidPointer[1 + Turbo].FvidState.State));
  DEBUG ((EFI_D_ERROR, "FVID[%02d].BusRatio = %x\n", (1 + Turbo), FvidPointer[1 + Turbo].FvidState.BusRatio ));
  DEBUG ((EFI_D_ERROR, "FVID[%02d].vid = %x\n", (1 + Turbo), FvidPointer[1 + Turbo].FvidState.Vid ));
  DEBUG( (EFI_D_ERROR, "FVID[%02d].Power = %x\n\n", (1 + Turbo), FvidPointer[1 + Turbo].FvidState.Power));

  //
  // Fill in the table starting at the last entry
  // The algorithm is available in the processor BIOS writer's guide.
  //
  CurrentBusRatio = mMaxBusRatio;

  DEBUG ((EFI_D_ERROR, "\n\t-->REST of VID ENTRY\n"));

  for (i = 1; i < NumberOfStates; i++) {
    CurrentBusRatio = CurrentBusRatio - StepSize;
    if (CurrentBusRatio == 8) {
      FvidPointer[0].FvidHeader.Gv3States -= 1;
      ReducedStep ++;
      DEBUG ((EFI_D_ERROR, "change FVID[0].FvidHeader.Gv3States = %x\n\n", FvidPointer[0].FvidHeader.Gv3States));
      continue;
    }
    
    FvidPointer[i + 1 + Turbo- ReducedStep].FvidState.State     = i + Turbo - ReducedStep;
    FvidPointer[i + 1 + Turbo- ReducedStep].FvidState.BusRatio  = CurrentBusRatio;

    if (BusRatioRange != 0) {
      FvidPointer[i + 1 + Turbo- ReducedStep].FvidState.Vid       = ((CurrentBusRatio - mMinBusRatio) * VidRange * 2) / BusRatioRangeX2 + mMinVid;
    } else {
      FvidPointer[i + 1 + Turbo- ReducedStep].FvidState.Vid       = mMinVid;
    }

    if (((CurrentBusRatio - mMinBusRatio) * VidRange * 2) % BusRatioRangeX2) {
      //
      // Round up if there is a remainder to remain above the minimum voltage
      //
      FvidPointer[i + 1 + Turbo- ReducedStep].FvidState.Vid++;
    }

    if (BusRatioRange != 0) {
      FvidPointer[i + 1 + Turbo- ReducedStep].FvidState.Power      = ((CurrentBusRatio - mMinBusRatio) * PowerRange * 2) / BusRatioRangeX2 + FVID_MIN_POWER_MIDVIEW;
    } else {
      FvidPointer[i + 1 + Turbo- ReducedStep].FvidState.Power     = FVID_MIN_POWER_MIDVIEW;
    }

    if ( NumberOfStates - i == 1) {
      FvidPointer[i + 1 + Turbo - ReducedStep].FvidState.BusRatio = mMinBusRatio;
      FvidPointer[i + 1 + Turbo - ReducedStep].FvidState.Vid = mMinVid;
      FvidPointer[i + 1 + Turbo - ReducedStep].FvidState.Power = FVID_MIN_POWER_MIDVIEW;
    }

    DEBUG ((EFI_D_ERROR, "FVID[%02d].State = %x\n", (i + 1 + Turbo- ReducedStep), FvidPointer[i + 1 + Turbo- ReducedStep].FvidState.State));
    DEBUG ((EFI_D_ERROR, "FVID[%02d].BusRatio = %x\n", (i + 1 + Turbo- ReducedStep), FvidPointer[i + 1 + Turbo- ReducedStep].FvidState.BusRatio ));
    DEBUG ((EFI_D_ERROR, "FVID[%02d].Vid = %x\n", (i + 1 + Turbo- ReducedStep), FvidPointer[i + 1 + Turbo- ReducedStep].FvidState.Vid ));
    DEBUG( (EFI_D_ERROR, "FVID[%02d].Power = %x\n\n", (i + 1 + Turbo- ReducedStep), FvidPointer[i + 1 + Turbo- ReducedStep].FvidState.Power));
  }

  DEBUG((EFI_D_ERROR,"\n\n"));

  return EFI_SUCCESS;
}


UINT8 GetConvertedTime(
  IN UINT32 TimeInSeconds
  )
/*++

Routine Description:

  - Private helper function to convert various Turbo Power Limit Time from Seconds to CPU units

Arguments:

  TimeInSeconds     Time in seconds

Returns:

  Converted time in CPU units

--*/
{
  UINT8         ConvertedPowerLimitTime;
  UINT8         Index;

  //
  // Convert seconds to MSR value. Since not all values are programmable, we'll select
  // the entry from mapping table which is either equal to the user selected value. OR to a value in the mapping table
  // which is closest (but less than) to the user-selected value.
  //
  ConvertedPowerLimitTime = mSecondsToMsrValueMapTable[0][1];

  for (Index = 0; mSecondsToMsrValueMapTable[Index][0] != END_OF_TABLE; Index++) {
    if (TimeInSeconds == mSecondsToMsrValueMapTable[Index][0]) {
      ConvertedPowerLimitTime = mSecondsToMsrValueMapTable[Index][1];
      break;
    }

    if (TimeInSeconds > mSecondsToMsrValueMapTable[Index][0]) {
      ConvertedPowerLimitTime = mSecondsToMsrValueMapTable[Index][1];
    } else {
      break;
      break;
    }
  }
  return ConvertedPowerLimitTime;
}

VOID
UpdateCurrentPowerInfo (
  )
/*++

Routine Description:

  Get the updated power configuration register values

Arguments:

  None

Returns:

  None

--*/
{

  MSR_REGISTER        TempMsr;
//  UINTN               remainder;

  TempMsr.Qword        = AsmReadMsr64 (MSR_PLATFORM_INFO);
  //
  // Check if TDP Limit is programmable
  //  Platform Info MSR (0xCE) [29]
  //
  mTdpLimitProgrammble = 1;

  TempMsr.Qword        = AsmReadMsr64 (MSR_PACKAGE_POWER_SKU_UNIT);
  //
  // Get Power Unit MSR [3:0]
  // The actual unit value is calculated by 1mW*Power(2,POWER_UNIT)..Reset value of 5 represents 32mW units.
  //
  mProcessorPowerUnit  = (TempMsr.Bytes.FirstByte & PACKAGE_POWER_UNIT_MASK);

  if (mProcessorPowerUnit == 0) {
    mProcessorPowerUnit = 1;
  } else {
    mProcessorPowerUnit = (UINT8) LShiftU64 (2, (mProcessorPowerUnit - 1));
  }
  // @Note: This field should be initialized by PUnit to IACORE_GUAR_TDP_FUSE + SOC_GUAR_TDP_FUSE
  //            Get the pre-si estimated settings for the Turbo from Punit.
  mPackageTdp      = 0x1F;
  mPackageMaxPower = 0xFF;
  mPackageMinPower = 0x3F;
  //TempMsr.Qword        = AsmReadMsr64 (MSR_TURBO_POWER_LIMIT);
  //if (TempMsr.Dwords.Low > 0) {
  // BIOS will program the POWER_LIMIT1
  //
  // Get Processor TDP value in mWatts
  //
  //mPackageTdp          = (UINT16) DivU64x32 ((TempMsr.Dwords.Low & PACKAGE_TDP_POWER_MASK), mProcessorPowerUnit, &remainder);
  //mPackageTdp          = (UINT16) (TempMsr.Dwords.Low & PACKAGE_TDP_POWER_MASK) *  mProcessorPowerUnit;

  //
  // Maximum allowed power limit value in TURBO_POWER_LIMIT_MSR and PRIMARY_PLANE_POWER_LIMIT_MSR
  // in units specified by PACKAGE_POWER_SKU_UNIT_MSR
  //
  //mPackageMaxPower     = (UINT16)(TempMsr.Dwords.High & PACKAGE_MAX_POWER_MASK);
  //mPackageMaxPower  = mPackageTdp;
  //
  // Minimum allowed power limit value in TURBO_POWER_LIMIT_MSR and PRIMARY_PLANE_POWER_LIMIT_MSR
  // in units specified by PACKAGE_POWER_SKU_UNIT_MSR
  //
  //mPackageMinPower     = (UINT16)RShiftU64((TempMsr.Dwords.Low & PACKAGE_MIN_POWER_MASK), 16);
  //mPackageMinPower     = mPackageTdp;
  //}  else  {
  // @TODO: Must provide a safe value OR retrieve from Fuse CPU_TDP + SOC_POWER_BUDGET[SOC_TDP].
  //mPackageTdp      = 0;
  //mPackageMaxPower = 0;
  //mPackageMinPower = 0;
  //}
  return;

}

STATIC
EFI_STATUS
RunOnAllLogicalProcessors (
  IN OUT EFI_AP_PROCEDURE   Procedure,
  IN OUT VOID               *Buffer
  )
/*++

Routine Description:

  Runs the specified procedure on all logical processors, passing in the
  parameter buffer to the procedure.

Arguments:

  Procedure     The function to be run.
  Buffer        Pointer to a parameter buffer.

Returns:

  None

--*/
{
  UINTN      Index;
  EFI_STATUS Status;
  //
  // Run the procedure on all logical processors.
  //
  (*Procedure) (Buffer);
  for (Index = 1; Index < gSmst->NumberOfCpus; Index++) {
    Status = EFI_NOT_READY;
    while (Status != EFI_SUCCESS ) {
      Status = gSmst->SmmStartupThisAp (Procedure, Index, Buffer);
      if ( Status != EFI_SUCCESS ) {
        //
        // SmmStartupThisAp might return failure if AP is busy executing some other code. Let's wait for sometime and try again.
        //
        //PchPmTimerStall (PPM_WAIT_PERIOD);
      }
    }
  }

  return EFI_SUCCESS;
}


STATIC
VOID
ConfigureTurboPowerLimitsValleyview (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT PPM_PLATFORM_POLICY_PROTOCOL     *PPMPolicy
  )
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
{
// @NOTE: This may be redundant in BIOS boot flow as P-unit will intialize this MSR with Package TDP.
  MSR_REGISTER        PKGTurboCfg1Msr;
  MSR_REGISTER        CPUTurboWkldCfg1Msr;
  MSR_REGISTER        PP1TurboPowerLimitMsr;
  MSR_REGISTER        TurboPowerLimitMsr;
  UINT8               ConvertedPowerLimit1Time;
  UINT16              ProgramedPowerLimit1;
  UINT16              ConvertedPowerLimit2;
  UINT16              ProgramedPowerLimit2;

  //
  // Get the updated power congigurations as these values may be reconfigured by the Punit
  //
  UpdateCurrentPowerInfo ();
  // Get the pre-si estimated settings for the Turbo from Punit.
  // mPackageTdp = 0x1F;
  // mPackageMaxPower = 0xFF;
  // mPackageMinPower = 0x3F;
  if (mTdpLimitProgrammble) {
    TurboPowerLimitMsr.Qword  = AsmReadMsr64 (MSR_TURBO_POWER_LIMIT);
    //
    // Initialize the Power Limit 1 and Power Limit 1 enable bit
    //  Power Limit 1: Turbo Power Limit MSR  [14:0]
    //  Power Limit 1 Enable: Turbo Power Limit MSR  [15]
    //
    //
    // By default, program the value of IACORE_GUAR_TDP_FUSE + SOC_GUAR_TDP_FUSE to Package TDP limit
    //
    ProgramedPowerLimit1 = mPackageMinPower;
    if (PPMPolicy->TurboSettings.PowerLimit1 != AUTO) {
      //
      // mPackageMinPower, mPackageMaxPower and mPackageTdp are in watts.
      // they can directly compare to PPMPolicy->pTurboSettings->PowerLimit1
      //
      ProgramedPowerLimit1 = PPMPolicy->TurboSettings.PowerLimit1;
      if (PPMPolicy->TurboSettings.PowerLimit1 < mPackageMinPower) {
        //
        // If new Power Limit 1 is < mPackageMinPower, program Power Limit 1 to mPackageMinPower
        //
        ProgramedPowerLimit1 = mPackageMinPower;
      } else if ( PPMPolicy->TurboSettings.PowerLimit1 > mPackageMaxPower ) {
        //
        // If new Power Limit 1 is not within bounds, program Power Limit 1 to Package TDP limit
        //
        ProgramedPowerLimit1 = mPackageMaxPower;
      }
      //
      // PPMPolicy->pTurboSettings->PowerLimit1 is in watts. We need to convert it to
      // CPU Power unit, specified in PACKAGE_POWER_SKU_UNIT_MSR[3:0].
      // Since we are converting from Watts to CPU power units, multiply by
      // PACKAGE_POWER_SKU_UNIT_MSR[3:0].
      //
    }

    TurboPowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_MASK;
    TurboPowerLimitMsr.Dwords.Low |= (UINT32) (ProgramedPowerLimit1);
    // DEBUG ((EFI_D_ERROR, "New Power Limit 1  %d watt (%d in CPU power unit)\n", PPMPolicy->TurboSettings.PowerLimit1,ConvertedPowerLimit1));

    //
    // Force Power Limit 1 override to be enabled
    //
    TurboPowerLimitMsr.Dwords.Low |= POWER_LIMIT_ENABLE;

    //
    // Program Power Limit 1 (Long Duration Turbo) Time Window
    //  If PowerLimit1Time is AUTO OR If PowerLimit1Time is > MAX_POWER_LIMIT_1_TIME_IN_SECONDS program default values
    //
    if ( ( PPMPolicy->TurboSettings.PowerLimit1Time == AUTO ) || ( PPMPolicy->TurboSettings.PowerLimit1Time > MAX_POWER_LIMIT_1_TIME_IN_SECONDS ) ) {
      //
      // For Mobile, default value is 5 seconds
      //
      PPMPolicy->TurboSettings.PowerLimit1Time = 5;

    }
    ConvertedPowerLimit1Time = GetConvertedTime (PPMPolicy->TurboSettings.PowerLimit1Time);

    //
    //  Configure Power Limit 1 (Long Duration Turbo) time windows: Turbo Power Limit MSR [23:17]
    //
    TurboPowerLimitMsr.Dwords.Low &= ~POWER_LIMIT_1_TIME_MASK;
    TurboPowerLimitMsr.Dwords.Low |= (UINT32) LShiftU64(ConvertedPowerLimit1Time, 17);

    //
    //  Initialize Short Duration Power limit and enable bit
    //    Short duration Power Limit: Turbo Power Limit MSR (0x450h) [46:32]
    //    Short duration Power Limit Enable:Turbo Power Limit MSR (0x450h) [47]
    //
    // PPMPolicy->pTurboSettings->PowerLimit2 value is in watts. We need to convert it to
    // CPU Power unit, specified in PACKAGE_POWER_SKU_UNIT_MSR[3:0].
    // Since we are converting from Watts to CPU power units, multiply by
    // PACKAGE_POWER_SKU_UNIT_MSR[3:0]
    //
    ProgramedPowerLimit2 = mPackageMaxPower;
    ConvertedPowerLimit2 = mPackageMaxPower;
    ConvertedPowerLimit2 = ConvertedPowerLimit2 * mProcessorPowerUnit;
    if (PPMPolicy->TurboSettings.PowerLimit2 != AUTO) {
      //
      // mPackageMinPower, mPackageMaxPower and mPackageTdp are in watts.
      // they can directly compare to PPMPolicy->pTurboSettings->PowerLimit1
      //
      ProgramedPowerLimit2 = PPMPolicy->TurboSettings.PowerLimit2;
      if (PPMPolicy->TurboSettings.PowerLimit2 < mPackageMinPower) {
        //
        // If new Power Limit 2 is < mPackageMinPower, program Power Limit 2 to mPackageMinPower
        //
        ProgramedPowerLimit2 = mPackageMinPower;
      } else if ( PPMPolicy->TurboSettings.PowerLimit2 > mPackageMaxPower ) {
        //
        // If new Power Limit 2 is not within bounds, program Power Limit 2 to Package TDP limit
        //
        ProgramedPowerLimit2 = mPackageMaxPower;
      }
      //
      // PPMPolicy->pTurboSettings->PowerLimit2 is in watts. We need to convert it to
      // CPU Power unit, specified in PACKAGE_POWER_SKU_UNIT_MSR[3:0].
      // Since we are converting from Watts to CPU power units, multiply by
      // PACKAGE_POWER_SKU_UNIT_MSR[3:0].
      //
      ConvertedPowerLimit2 = (ProgramedPowerLimit2 * mProcessorPowerUnit);
    }
    TurboPowerLimitMsr.Dwords.High &= ~POWER_LIMIT_MASK;
    TurboPowerLimitMsr.Dwords.High |= (UINT32) (ProgramedPowerLimit2);
    // set PKG_PWR_LIM2_TIME = 0x0, 1s
    TurboPowerLimitMsr.Dwords.High &= ~POWER_LIMIT_1_TIME_MASK;
    // force Power Limit 2 override to be enabled
    TurboPowerLimitMsr.Dwords.High |= POWER_LIMIT_ENABLE;
    // DEBUG ((EFI_D_ERROR, "New Power Limit 2  %d watt (%d in CPU power unit)\n", PPMPolicy->TurboSettings.PowerLimit2,ConvertedPowerLimit2));

    // Set up registers for energy management and dynamic power limiting
    TurboPowerLimitMsr.Dwords.Low = 0x003880FA;
    TurboPowerLimitMsr.Dwords.High = 0x00000000;
    AsmWriteMsr64 (MSR_TURBO_POWER_LIMIT, TurboPowerLimitMsr.Qword);
  }

  // Configure PP1_TURBO_POWER_LIMIT [23:17]  = 0x0, 1 second time duration
  PP1TurboPowerLimitMsr.Qword = AsmReadMsr64 (MSR_PRIMARY_PLANE_TURBO_POWER_LIMIT);
  PP1TurboPowerLimitMsr.Dwords.Low &= ~PP_PWR_LIM_TIME_MASK;
  PP1TurboPowerLimitMsr.Dwords.Low |= PP_PWR_LIM_TIME;
  AsmWriteMsr64 (MSR_PRIMARY_PLANE_TURBO_POWER_LIMIT, PP1TurboPowerLimitMsr.Qword);

  // Configure PKG_TURBO_CFG1
  PKGTurboCfg1Msr.Qword = AsmReadMsr64 (MSR_PKG_TURBO_CFG1);
  // set TURBOMODE = 0x2, Dynamic Turbo
  //PKGTurboCfg1Msr.Dwords.Low &= ~TURBOMODE_MASK;
  //PKGTurboCfg1Msr.Dwords.Low |= TURBOMODE;
  // set ICCMAX_CTRL = 0x2, Reduce frequency to nCore Turbo before allowing c6 exits
  //PKGTurboCfg1Msr.Dwords.Low &= ~ICCMAX_CTRL_MASK;
  //PKGTurboCfg1Msr.Dwords.Low |= ICCMAX_CTRL;
  // set ICCMAX_CTRL_EN
  //PKGTurboCfg1Msr.Dwords.Low |= ICCMAX_CTRL_EN;
  // set SOC_TDP_EN
  //PKGTurboCfg1Msr.Dwords.Low |= SOC_TDP_EN;
  // set SOC_TDP_POLICY = 0x3, 50/50 split
  //PKGTurboCfg1Msr.Dwords.Low &= ~SOC_TDP_POLICY_MASK;
  //PKGTurboCfg1Msr.Dwords.Low |= SOC_TDP_POLICY;
  // set MIN_ENERGY = 0x1, 16uJ minimum credit.. Basically allow Turbo asap
  //PKGTurboCfg1Msr.Dwords.Low &= ~MIN_ENERGY_MASK;
  //PKGTurboCfg1Msr.Dwords.Low |= MIN_ENERGY;
  // set Msr 0x670 = 0x00000002 (Dynamic Turbo Mode)
  PKGTurboCfg1Msr.Dwords.Low &= 0x0;
  PKGTurboCfg1Msr.Dwords.Low |= 0x00000702;
  AsmWriteMsr64 (MSR_PKG_TURBO_CFG1, PKGTurboCfg1Msr.Qword);

  // Configure CPU_TURBO_WKLD_CFG1
  CPUTurboWkldCfg1Msr.Qword = AsmReadMsr64 (MSR_CPU_TURBO_WKLD_CFG1);
  // set C0STL_AFACTOR_THRESH = 0x6, 0.25 threshold to affect AFACTOR
  //CPUTurboWkldCfg1Msr.Dwords.Low &= ~C0STL_AFACTOR_THRESH_MASK;
  //CPUTurboWkldCfg1Msr.Dwords.Low |= C0STL_AFACTOR_THRESH;
  // set IPC_THRESH0 = 0x3, An IPC ratio below the specified value will be mapped to 0.375
  //CPUTurboWkldCfg1Msr.Dwords.Low &= ~IPC_THRESH0_MASK;
  //CPUTurboWkldCfg1Msr.Dwords.Low |= IPC_THRESH0;
  // set IPC_THRESH1 = 0x4, An IPC ratio between IPC_THRESH0 and IPC_THRESH1 will be mapped to 0.5
  //CPUTurboWkldCfg1Msr.Dwords.Low &= ~IPC_THRESH1_MASK;
  //CPUTurboWkldCfg1Msr.Dwords.Low |= IPC_THRESH1;
  // set IPC_THRESH2 = 0x8, An IPC ratio between IPC_THRESH1 and IPC_THRESH2 will be mapped to 1.0
  //CPUTurboWkldCfg1Msr.Dwords.Low &= ~IPC_THRESH2_MASK;
  //CPUTurboWkldCfg1Msr.Dwords.Low |= IPC_THRESH2;
  // set IPC_THRESH3 = 0xB, An IPC ratio between IPC_THRESH2 and IPC_THRESH3 will be mapped to 1.375
  //CPUTurboWkldCfg1Msr.Dwords.Low &= ~IPC_THRESH3_MASK;
  //CPUTurboWkldCfg1Msr.Dwords.Low |= IPC_THRESH3;
  // set IPC_THRESH4 = 0xC, An IPC ratio between IPC_THRESH3 and IPC_THRESH4 will be mapped to 1.5
  //CPUTurboWkldCfg1Msr.Dwords.Low &= ~IPC_THRESH4_MASK;
  //CPUTurboWkldCfg1Msr.Dwords.Low |= IPC_THRESH4;
  // set Msr 0x671 = 0x00001B14 (power meter weights)
  CPUTurboWkldCfg1Msr.Dwords.Low &= 0x0;
  CPUTurboWkldCfg1Msr.Dwords.Low |= 0x200B;
  AsmWriteMsr64 (MSR_CPU_TURBO_WKLD_CFG1, CPUTurboWkldCfg1Msr.Qword);

}

