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

  Features.c

Abstract:

--*/

#include "CpuDxe.h"
#include "Features.h"
#include "MiscFuncs.h"
#include "MpCommon.h"
#include "Configuration.h"
#include "CpuRegs.h"
#include <PchAccess.h>
#include <Library/PchPlatformLib.h>

extern EFI_PLATFORM_CPU_PROTOCOL    *mPlatformCpu;
extern EFI_PLATFORM_CPU_INFO        mPlatformCpuInfo;

UINTN                       mWroteMsr;
UINTN                       mOriginalInt13;

UINT8                       mFeatureLock;
UINTN                       mCommonFeatures;
UINTN                       mSetupFeatures;
UINT8                       mDcaDelayValue;
UINT8                       mPsdState;
UINT32                      mMaxCStateValue = 0x10;
UINT32                      mCommonCStateValue;
BOOLEAN                     mProcessorMsrLock;

VOID
EnableDisableBiProchot (
  IN  UINTN     Support
  )
{
  UINT64           MsrValue;

  if (!(mCommonFeatures & PROCHOT_SUPPORT)) {
    return;
  }

  MsrValue = AsmReadMsr64 (EFI_MSR_CPU_THERM_CFG2);
  if ((Support & B_EFI_MSR_CPU_THERM_CFG_BIDIR_PROCHOT) == 0) {
    MsrValue &= ~B_EFI_MSR_CPU_THERM_CFG_BIDIR_PROCHOT;
  } else {
    MsrValue |= B_EFI_MSR_CPU_THERM_CFG_BIDIR_PROCHOT;
  }
  AsmWriteMsr64 (EFI_MSR_CPU_THERM_CFG2, MsrValue);
}

//
// DCA contains processor code and chipset code
// CPU driver has the following assumption on the initialization flow
// 1. Chipset pre-initialization should detect DCA support per chipset capability after PlatformCpu
// 2. If not support, it should update PlatformCpu DCA to disable state
// 3. If support, it should enable the DCA related registers
// 4. CPU initialization for DCA (CPU may change PlatformCpu DCA states per CPU capability)
// 5. Normal chipset driver (IOH) should look at PlatformCpu DCA policy again in PCI enumeratoin
// 6. Chipset enable or disable DCA according to PlatformCpu DCA state
//
UINTN
IsDcaSupported (
  VOID
  )
{
  BOOLEAN                Support;

  Support = 0;
  if (mPlatformCpuInfo.CpuFeatures.Dca) {
    Support = DCA_SUPPORT;
  }

  return Support;
}

UINTN
IsCcxSupported (
  VOID
  )
{
  UINTN                       Support;

  Support = 0;

  //
  // Monitor / Mwait
  //
  if (mPlatformCpuInfo.CpuFeatures.Mwait && mPlatformCpuInfo.CpuCState.MaxCState > 1) {
    //
    // CCx Enable Bit feature is supported on this processor.
    //
    Support = CCX_SUPPORT;

    if (mMaxCStateValue > mPlatformCpuInfo.CpuCState.MaxCState) {
      mMaxCStateValue = mPlatformCpuInfo.CpuCState.MaxCState;
    }
    if (mCommonCStateValue > mPlatformCpuInfo.CpuCState.MaxCState) {
      mCommonCStateValue = mPlatformCpuInfo.CpuCState.MaxCState;
    }
  }

  return Support;
}

VOID
EnableDisableCcxState (
  IN  UINTN     Support
  )
{
  UINT64           MsrValue;

  if (!(mCommonFeatures & CCX_SUPPORT)) {
    return;
  }

  if (Support & CCX_SUPPORT) {
    MsrValue = AsmReadMsr64 (EFI_MSR_PMG_CST_CONFIG);
    //
    // For Native MWAIT and IO Redirection Support
    // Enable IO MWAIT translation for processor core C-state control.
    // Also limit the package to the max supported C-state.
    //
    MsrValue &= ~(B_EFI_MSR_PMG_CST_CONFIG_PACKAGE_C_STATE_LIMIT);
    MsrValue |= mCommonCStateValue;
    MsrValue |= B_EFI_MSR_PMG_CST_CONFIG_IO_MWAIT_REDIRECTION_ENABLE;

    if (mPlatformCpuInfo.CpuType == EnumPineview) {
      MsrValue &= ~(B_EFI_MSR_PMG_CST_CONFIG_CSM_TRIGGER_MASK);
      //
      // Set this to prevent VID switching to the C4-value during C4 entry. This applied to DeepC4 too.
      //
      MsrValue |= B_EFI_MSR_PMG_CST_CONFIG_C4VID_DISABLE;
    }

    EfiWriteMsrWithScript (EFI_MSR_PMG_CST_CONFIG, MsrValue);

    MsrValue = AsmReadMsr64 (EFI_MSR_PMG_IO_CAPTURE_ADDR);
  }
}

UINTN
IsC1eSupported (
  VOID
  )
{
  BOOLEAN                Support;

  Support = C1E_SUPPORT;

  return Support;
}

VOID
EnableDisableC1e (
  IN  UINTN     Support
  )
{
  UINT64           MsrValue;

  if (!(mCommonFeatures & C1E_SUPPORT)) {
    return;
  }

  MsrValue = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
  if ((Support & C1E_SUPPORT) == 0) {
    MsrValue &= ~B_EFI_MSR_IA32_MISC_ENABLE_C1E_EN;
  } else {
    MsrValue |=  B_EFI_MSR_IA32_MISC_ENABLE_C1E_EN;
  }
  AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, MsrValue);
}

UINTN
IsGv3Supported (
  VOID
  )
{
  BOOLEAN                Support;

  Support = 0;

  if (mPlatformCpuInfo.CpuFeatures.Eist) {
    //
    // EIST Enable Bit feature is supported on this processor.
    //
    Support = GV3_SUPPORT;
  }

  return Support;
}

VOID
EnableDisableGv3 (
  IN  UINTN     Support
  )
{
  UINT64           MsrValue;
  UINT64           DesiredRatio;

  if (!(mCommonFeatures & GV3_SUPPORT)) {
    return;
  }

  DesiredRatio = AsmReadMsr64 (EFI_MSR_CLOCK_FLEX_MAX);
  DesiredRatio = (DesiredRatio >> N_EFI_MSR_CLOCK_FLEX_MAX_FLEX_RATIO);
  DesiredRatio = DesiredRatio & B_EFI_MSR_CLOCK_FLEX_MAX_FLEX_RATIO_MASK;

  if ((Support & GV3_SUPPORT) == 0) {
    MsrValue = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
    MsrValue &= ~B_EFI_MSR_IA32_MISC_ENABLE_EIST;
    AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, MsrValue);
  } else {
    MsrValue = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
    MsrValue |=  B_EFI_MSR_IA32_MISC_ENABLE_EIST;
    AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, MsrValue);

    //
    // Lock EIST setting
    //
    MsrValue = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
    MsrValue |=  B_EFI_MSR_IA32_MISC_ENABLE_EIST_LOCK;
    AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, MsrValue);
  }
}

UINTN
IsXdSupported (
  VOID
  )
{
  BOOLEAN                Support;

  Support = 0;

  if (mPlatformCpuInfo.CpuFeatures.ExtXd) {
    Support = XD_SUPPORT;
  }

  return Support;
}

VOID
EnableDisableXd (
  IN  UINTN     Support
  )
{
  UINT64           MsrValue;

  if (!(mCommonFeatures & XD_SUPPORT)) {
    return;
  }

  MsrValue = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
  if (Support & XD_SUPPORT) {
    MsrValue &= ~B_EFI_MSR_IA32_MISC_ENABLE_XD;
  } else {
    MsrValue |=  B_EFI_MSR_IA32_MISC_ENABLE_XD;
  }
  AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, MsrValue);
}

UINTN
IsVmxSupported (
  IN  VOID
  )
/*++

Routine Description:

  Check on the processor if VT/LT is supported.

Arguments:

  None

Returns:

  None.

--*/
{
  UINTN                        Support;

  Support = 0;

  //
  // Get CPUID to check if the processor supports Vanderpool Technology.
  //
  if (mPlatformCpuInfo.CpuFeatures.Vt) {
    //
    // VT is supported.
    //
    Support |= VT_SUPPORT;
  }

  if (mPlatformCpuInfo.CpuFeatures.Lt) {
    //
    // LT is supported.
    //
    Support |= LT_SUPPORT;
  }

  return Support;
}

/**
  Detect if AES supported or not

  @retval AES_SUPPORT if supported or 0 if not supported
**/
UINTN
IsAesSupported (
  VOID
  )
{

  UINT32             RegEcx;
  UINTN              Support;

  Support = 0;
  AsmCpuid (EFI_CPUID_VERSION_INFO, NULL, NULL, &RegEcx, NULL);

  if ((RegEcx & BIT25) != 0) {
    Support = AES_SUPPORT;
  }

  return Support;
}

/**
  Enable / Disable AES on the processor.

  @param[in] Support  - To enable or disable AES feature.
**/
VOID
EnableDisableAes (
  IN UINTN Support
  )
{
  UINT64 MsrValue;

  if (!(mCommonFeatures & AES_SUPPORT) || (IsSecondaryThread ())) {
    return;
  }

  ///
  /// The processor was manufacted with AES-NI feature
  ///
  MsrValue = AsmReadMsr64 (MSR_IA32_FEATURE_CONFIG);

  ///
  /// Check the Feature Lock Bit.
  /// If it is already set, which indicates we are executing POST
  /// due to a warm RESET (i.e., PWRGOOD was not de-asserted).
  ///
  if ((MsrValue & B_IA32_FEATURE_CONFIG_LOCK) == 0) {
    if (Support & AES_SUPPORT) {
      ///
      /// Enabled AES, writes of 00b, 01b pr 10b to the MSR will result in AES enable.
      /// Should lock this MSR always, so write 01b to the MSR.
      ///
      MsrValue &= ~B_IA32_FEATURE_CONFIG_AES_DIS;
      MsrValue |= B_IA32_FEATURE_CONFIG_LOCK;
    } else {
      ///
      /// To disable AES, system BIOS must write 11b to this MSR.
      ///
      MsrValue |= (B_IA32_FEATURE_CONFIG_AES_DIS + B_IA32_FEATURE_CONFIG_LOCK);
    }

    EfiWriteMsrWithScript (MSR_IA32_FEATURE_CONFIG, MsrValue);
  }
  return;
}

VOID
EnableDisableVmxSmrr (
  IN  UINTN     Support
  )
/*++

Routine Description:

  Enable VT/LT on the processor.

Arguments:

  Support  - To enable or disable VT/LT feature.

Returns:

  None.

--*/
{
  UINT64    Ia32FeatCtrl;
  UINT64    NewFeatCtrl;

  Ia32FeatCtrl = AsmReadMsr64 (EFI_MSR_IA32_FEATURE_CONTROL);
  NewFeatCtrl  = Ia32FeatCtrl;

  //
  // Check the Feature Lock Bit.
  // If it is already set, which indicates we are executing POST
  // due to a warm RESET (i.e., PWRGOOD was not de-asserted).
  //
  if ((Ia32FeatCtrl & B_EFI_MSR_IA32_FEATURE_CONTROL_LOCK) == 0) {
    //
    // If Vmx is Disabled, Enable it.
    //
    if (mCommonFeatures & VT_SUPPORT) {
      if (Support & VT_SUPPORT) {
        NewFeatCtrl |=  B_EFI_MSR_IA32_FEATURE_CONTROL_EVT;
      } else {
        NewFeatCtrl &= ~B_EFI_MSR_IA32_FEATURE_CONTROL_EVT;
      }
    }

    if (mCommonFeatures & LT_SUPPORT) {
      if (Support & LT_SUPPORT) {
        //
        // MSR Lock will be done later.
        //
        NewFeatCtrl |=  (B_EFI_MSR_IA32_FEATURE_CONTROL_ELT | B_EFI_MSR_IA32_FEATURE_CONTROL_SLFE | B_EFI_MSR_IA32_FEATURE_CONTROL_SGE);
      } else {
        NewFeatCtrl &= ~(B_EFI_MSR_IA32_FEATURE_CONTROL_ELT | B_EFI_MSR_IA32_FEATURE_CONTROL_SLFE | B_EFI_MSR_IA32_FEATURE_CONTROL_SGE);
      }
    }

    if (mPlatformCpuInfo.Msr.Smrr &&
       (mPlatformCpuInfo.CpuType == EnumPineview || mPlatformCpuInfo.CpuType == EnumCedarview)) {
      //
      // Enable Smrr.
      //
      NewFeatCtrl |= B_EFI_MSR_IA32_FEATURE_CONTROL_SMRR;
      mProcessorMsrLock = 1;
    }

    if (Ia32FeatCtrl != NewFeatCtrl) {
      EfiWriteMsrWithScript (EFI_MSR_IA32_FEATURE_CONTROL, NewFeatCtrl);
    }
  } else {
    //
    // BIOS needs to always save a set of EFI_MSR_IA32_FEATURE_CONTROL MSR for S3 resume purpose.
    // Mask off the lock bit as the bit will be set later in LockFeatureBit().
    // Note: EFI_MSR_IA32_FEATURE_CONTROL is write once and will be cleared when the PowerOk signal de-asserted. Eg. During S3.
    //
    NewFeatCtrl &= ~B_EFI_MSR_IA32_FEATURE_CONTROL_LOCK;
    EfiWriteToScript (EFI_MSR_IA32_FEATURE_CONTROL, NewFeatCtrl);
  }
}

VOID
LockFeatureBit (
  IN  BOOLEAN   LockFeatureEnable
  )
/*++

Routine Description:

  Lock VT/LT feature bits on the processor.
  Set "CFG Lock" (MSR 0E2h Bit[15]

Arguments:

  None

Returns:

  None.

--*/
{
  UINT64    Ia32FeatCtrl;

  if (!LockFeatureEnable) {
    return;
  }

  //
  // MSR 3Ah Lock
  //
  Ia32FeatCtrl = AsmReadMsr64 (EFI_MSR_IA32_FEATURE_CONTROL);

  //
  // Assumming that the EFI_MSR_IA32_FEATURE_CONTROL MSR write programming is done correctly by BIOS.
  // BIOS needs to save the MSR for S3 resume.
  // Note: EFI_MSR_IA32_FEATURE_CONTROL is write once and will be cleared when the PowerOk signal de-asserted. Eg. During S3.
  //
  if ((Ia32FeatCtrl & B_EFI_MSR_IA32_FEATURE_CONTROL_LOCK) == 0) {
    //
    // Set Feature Lock bits.
    //
    Ia32FeatCtrl |= B_EFI_MSR_IA32_FEATURE_CONTROL_LOCK;
    EfiWriteMsrWithScript (EFI_MSR_IA32_FEATURE_CONTROL, Ia32FeatCtrl);
  } else {
    //
    // Save to script only since MSR already been written
    //
    EfiWriteToScript (EFI_MSR_IA32_FEATURE_CONTROL, Ia32FeatCtrl);
  }

  //
  // Workaround for S3 wake hang issue. Don't lock the C State Msr
  //
  //
  // MSR 0E2h for C State Lock
  //
  //Ia32FeatCtrl = AsmReadMsr64 (EFI_MSR_PMG_CST_CONFIG);
  //
  //if ((Ia32FeatCtrl & B_EFI_MSR_PMG_CST_CONFIG_CST_CONTROL_LOCK) == 0) {
  //
  // Set CFG Lock
  //
  //  Ia32FeatCtrl = AsmReadMsr64 (EFI_MSR_PMG_CST_CONFIG);
  //  Ia32FeatCtrl |= B_EFI_MSR_PMG_CST_CONFIG_CST_CONTROL_LOCK;
  //  EfiWriteMsrWithScript (EFI_MSR_PMG_CST_CONFIG, Ia32FeatCtrl);
  //}
  //
  // Workaround for S3 wake hang issue - Done
  //
}

UINTN
IsXapicSupported (
  VOID
  )
{
  UINTN                  Support;
  UINT64                 MsrValue;

  Support = 0;

  if (mPlatformCpuInfo.CpuFeatures.Xapic) {
    MsrValue = AsmReadMsr64 (EFI_MSR_IA32_APIC_BASE);
    if (MsrValue & (B_EFI_MSR_IA32_APIC_BASE_APIC_GLOBAL_ENABLE | B_EFI_MSR_IA32_APIC_BASE_M_XAPIC)) {
      //
      // XAPIC Mode feature is supported on this processor.
      //
      Support = XAPIC_SUPPORT;
    }
  }

  return Support;
}

VOID
EnableDisableXAPIC (
  IN  UINTN     Support
  )
{
  UINT64           MsrValue;
  UINT64           MsrValue2;

  if (!(mCommonFeatures & XAPIC_SUPPORT)) {
    return;
  }

  MsrValue = AsmReadMsr64 (EFI_MSR_IA32_APIC_BASE);
  MsrValue2 = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
  if (Support & XAPIC_SUPPORT) {
    MsrValue |=  B_EFI_MSR_IA32_APIC_BASE_M_XAPIC;
    MsrValue2 |= B_EFI_MSR_IA32_MISC_ENABLE_FERR_MULTIPLEXING_EN;
  } else {
    MsrValue &= ~B_EFI_MSR_IA32_APIC_BASE_APIC_GLOBAL_ENABLE;
    MsrValue &= ~B_EFI_MSR_IA32_APIC_BASE_M_XAPIC;
    AsmWriteMsr64 (EFI_MSR_IA32_APIC_BASE, MsrValue);
    MsrValue |=  B_EFI_MSR_IA32_APIC_BASE_APIC_GLOBAL_ENABLE;
    MsrValue2 &= ~B_EFI_MSR_IA32_MISC_ENABLE_FERR_MULTIPLEXING_EN;
  }
  AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, MsrValue2);
  AsmWriteMsr64 (EFI_MSR_IA32_APIC_BASE, MsrValue);
}

VOID
EnableDisableEhalt  (
  IN  UINTN     Support
  )
{
  UINT64           MsrValue;

  MsrValue = AsmReadMsr64 (EFI_MSR_POWER_MISC);
  MsrValue |=  B_EFI_MSR_POWER_MISC_ENABLE_ULFM_AUTOCM_MASK;
  AsmWriteMsr64 (EFI_MSR_POWER_MISC, MsrValue);
}

VOID
InitializeFeaturePerSetup (
  IN  MP_SYSTEM_DATA               *MPSystemData
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MpServices  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  mFeatureLock    = VacantFlag;
  mDcaDelayValue  = (UINT8)MPSystemData->DcaPrefetchDelayValue;
  mPsdState       = (UINT8)MPSystemData->PsdState;

  mProcessorMsrLock = MPSystemData->ProcessorMsrLockControl;

  mCommonFeatures  = (UINTN)-1;
  mSetupFeatures  = (UINTN)-1;

  if (!MPSystemData->Gv3Enable) {
    mSetupFeatures &= ~GV3_SUPPORT;
  }

  if (!MPSystemData->ProcessorVmxEnable) {
    mSetupFeatures &= ~VT_SUPPORT;
  }

  if (!MPSystemData->LtEnable) {
    mSetupFeatures &= ~LT_SUPPORT;
  } else {
    mProcessorMsrLock = 1;         // Lock MSR if LT is enabled.
  }

  if (!MPSystemData->AesEnable) {
    mSetupFeatures &= ~AES_SUPPORT;
    DEBUG ((EFI_D_ERROR, "AES Disabled \n"));
  }

  if (!MPSystemData->ExecuteDisableBit) {
    mSetupFeatures &= ~XD_SUPPORT;
  }

  if ((!MPSystemData->CcxEnable) || (!MPSystemData->C1eEnable)) {
    mSetupFeatures &= ~C1E_SUPPORT;
  }

  if (!MPSystemData->DcaEnable) {
    mSetupFeatures &= ~DCA_SUPPORT;
  }

// Let the feature detection to govern this
//  if (!MPSystemData->XapicEnable) {
//    mSetupFeatures &= ~XAPIC_SUPPORT;
//  }

  if (!MPSystemData->CcxEnable) {
    mSetupFeatures &= ~CCX_SUPPORT;
  }

  if (!MPSystemData->BiDirectionalProchot) {
    mSetupFeatures &= ~PROCHOT_SUPPORT;
  }
}

VOID
CollectProcessorFeature (
  IN OUT VOID  *Buffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MpServices  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  UINTN       Support;

  Support  = 0;
  Support |= PROCHOT_SUPPORT;
  Support |= IsC1eSupported ();
  Support |= IsGv3Supported ();
  Support |= IsXdSupported ();
  Support |= IsVmxSupported ();
  Support |= IsDcaSupported ();
  Support |= IsXapicSupported ();
  Support |= IsCcxSupported ();
  Support |= IsAesSupported ();

  AsmAcquireMPLock  (&mFeatureLock);
  mCommonFeatures &= Support;
  AsmReleaseMPLock  (&mFeatureLock);
}


VOID
ProgramProcessorFeature (
  IN OUT VOID  *Buffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  MpServices  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
//TODO: disable all CPU features first
  UINTN   Supported;
  UINT64  MsrValue;
  BOOLEAN IsBsp;
//
  Supported = mCommonFeatures & mSetupFeatures;
  // All of these PM & TM will be programmed in the respective Reference Code.
  //EnableDisableBiProchot (Supported);
  //EnableDisableC1e       (Supported);
  //EnableDisableCcxState  (Supported);
  //EnableDisableGv3       (Supported);
  EnableDisableXd        (Supported);
////hphang >> The following will generate exception, if SMRR is on
#if (!_SIMIC_) && (!_SLE_HYB_) // Simics might not support SMRR bit turn on
  EnableDisableVmxSmrr   (Supported);
#endif
  EnableDisableXAPIC     (Supported);

  EnableDisableEhalt(Supported);

  // just disable the Faststring according to the request
  // only needed for A0, A1.
  if((PchStepping() == PchA0) || (PchStepping() == PchA1)) {
    MsrValue = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
    MsrValue &= ~BIT0;
    AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, MsrValue);
  } else {
    MsrValue = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
    MsrValue |= BIT0;
    AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, MsrValue);
  }

  EnableDisableAes (Supported);

//
//  //
//  // Finally record the MISC MSR into CPU S3 script table
//  // to avoid access for multiple times
//  //
//    MsrValue = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
//    EfiWriteMsrWithScript (EFI_MSR_IA32_MISC_ENABLE, MsrValue);
//
//  //
//  // Lock Processor Features
//  //
  LockFeatureBit (mProcessorMsrLock);
//
//  //
//  // Program virtual wire mode
//  //
  IsBsp = (AsmReadMsr64 (EFI_MSR_IA32_APIC_BASE) & B_EFI_MSR_IA32_APIC_BASE_BSP) ? TRUE : FALSE;
  ProgramVirtualWireMode (IsBsp);
}

VOID
ProgrameCpuidLimit (
  IN OUT VOID  *Buffer
  )
/**

@brief

  Program CPUID Limit before booting to OS

  @param[in] MpServices  - MP Services Protocol entry


**/
{
  UINT64  MsrValue;

  ///
  /// Move Limit CPUID Maxval configuration here to not impact the BOOT
  /// After setting this, no code can execute CPUID function > 3.
  ///
  CpuMiscEnable (mMPSystemData->LimitCpuidMaximumValue, B_EFI_MSR_IA32_MISC_ENABLE_CPUID_MAX);

  ///
  /// Finally record the MISC MSR into CPU S3 script table
  /// to avoid access for multiple times
  ///
  MsrValue = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
  EfiWriteMsrWithScript (EFI_MSR_IA32_MISC_ENABLE, MsrValue);

  return ;
}

VOID
UpdatePlatformCpuData (
  VOID
  )
{
  mPlatformCpu->CpuCommonFeature = mCommonFeatures;
}

