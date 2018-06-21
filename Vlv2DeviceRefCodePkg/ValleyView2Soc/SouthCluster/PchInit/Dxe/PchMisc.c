/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  PchMisc.c

  @brief
  Miscellaneous PCH initialization tasks

**/
#include "PchInit.h"

EFI_STATUS
ConfigureMiscItems (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN      UINT32                              RootComplexBar,
  IN      UINT32                              PmcBase,
  IN      UINT32                              IlbBase,
  IN OUT  UINT32                              *FuncDisableReg
  )
/**

  @brief
  Perform miscellany PCH initialization

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] PmcBase              PmcBase value of this PCH device
  @param[in] IlbBase              IlbBase value of this PCH device
  @param[in] FuncDisableReg       The value of Function disable register

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINT8   RegData8;
  UINT16  RegData16;

  DEBUG ((EFI_D_INFO, "ConfigureMiscItems() Start\n"));

  ///
  /// VLV BIOS Spec Rev 0.5, Section 28.2 Serial IRQs
  /// The only System BIOS requirement to use IRQs as a serial IRQ is to enable the function in IBASE + 0x60 [12] and
  /// select continuous or quiet mode, IBASE + 0x10 [7].
  /// VLV requires that the System BIOS first set the SERIRQ logic to continuous mode operation for at least one frame
  /// before switching it into quiet mode operation. This operation should be performed during the normal boot sequence
  /// as well as a resume from STR (S3).
  ///
  RegData16  = MmioRead16 (IlbBase + R_PCH_ILB_OIC) & (UINT16) ~(B_PCH_ILB_OIC_SIRQEN);
  RegData8   = MmioRead8  (IlbBase + R_PCH_ILB_SERIRQ_CNT);

  if (PchPlatformPolicy->SerialIrqConfig->SirqEnable == TRUE) {
    ///
    /// Set the SERIRQ logic to continuous mode
    ///
    RegData16 |= (UINT16) B_PCH_ILB_OIC_SIRQEN;
    RegData8  |= (UINT8)  B_PCH_ILB_SERIRQ_CNT_SIRQMD;
  }

  MmioWrite16 (IlbBase + R_PCH_ILB_OIC, RegData16);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (IlbBase + R_PCH_ILB_OIC),
    1,
    (VOID *) (UINTN) (IlbBase + R_PCH_ILB_OIC)
  );

  MmioWrite8 (IlbBase + R_PCH_ILB_SERIRQ_CNT, RegData8);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint8,
    (UINTN) (IlbBase + R_PCH_ILB_SERIRQ_CNT),
    1,
    (VOID *) (UINTN) (IlbBase + R_PCH_ILB_SERIRQ_CNT)
  );

  ///
  /// VLV BIOS Spec Rev 0.5, Section 39.7.1 RTC Resets
  /// VLV will set the RTC_PWR_STS bit (PBASE + 0x20h [2]) when the RTCRST# pin goes low.
  /// The System BIOS shouldn't rely on the RTC RAM contents when the RTC_PWR_STS bit is set.
  /// BIOS should clear this bit by writing a 0 to this bit position.
  /// This bit isn't cleared by any reset function.
  ///
  MmioAnd8 ((UINTN) (PmcBase + R_PCH_PMC_GEN_PMCON_1), (UINT8) (~(B_PCH_PMC_GEN_PMCON_RTC_PWR_STS)));

  ///
  /// VLV BIOS Spec Rev 0.5, Section 39.1 Handling Status Registers
  /// System BIOS must set 1’b1 to clear the following registers
  /// during power-on and resuming from Sx sleep state.
  ///
  /// PBASE + 0x00 [4] (PMC_HOST_WAKE_STS) = 1’b1
  /// PBASE + 0x00 [5] (WOL_OVR_WK_STS) = 1’b1
  ///
  MmioOr8 (
    (UINTN) (PmcBase + R_PCH_PMC_PRSTS),
    (UINT8) (B_PCH_PMC_PRSTS_HOST_WAKE_STS | B_PCH_PMC_PRSTS_WOL_OVR_WK_STS)
    );
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint8,
    (UINTN) (PmcBase + R_PCH_PMC_PRSTS),
    1,
    (VOID *) (UINTN) (PmcBase + R_PCH_PMC_PRSTS)
    );

  ///
  /// VLV BIOS Spec Update Rev 0.5, Section 18.9
  /// Enabling SLP_S3# and SLP_S4# Stretch
  /// Refer VLV EDS for details
  ///
  RegData16 = MmioRead16 (PmcBase + R_PCH_PMC_GEN_PMCON_1) &
              (UINT16) (~(B_PCH_PMC_GEN_PMCON_SLP_S3_MAW |
                          B_PCH_PMC_GEN_PMCON_SLP_S4_MAW));

  switch (PchPlatformPolicy->MiscPmConfig->PchSlpS3MinAssert) {
    case PchSlpS360us:
      RegData16 |= V_PCH_PMC_GEN_PMCON_SLP_S3_MAW_60US;
      break;

    case PchSlpS31ms:
      RegData16 |= V_PCH_PMC_GEN_PMCON_SLP_S3_MAW_1MS;
      break;

    case PchSlpS350ms:
    default:
      RegData16 |= V_PCH_PMC_GEN_PMCON_SLP_S3_MAW_50MS;
      break;

    case PchSlpS32s:
      RegData16 |= V_PCH_PMC_GEN_PMCON_SLP_S3_MAW_2S;
      break;
  }

  switch (PchPlatformPolicy->MiscPmConfig->PchSlpS4MinAssert) {
    case PchSlpS4PchTime:
      RegData16 &= (UINT16) (~B_PCH_PMC_GEN_PMCON_SLP_S4_ASE);
      break;

    case PchSlpS41s:
      RegData16 |= V_PCH_PMC_GEN_PMCON_SLP_S4_MAW_1S | B_PCH_PMC_GEN_PMCON_SLP_S4_ASE;
      break;

    case PchSlpS42s:
      RegData16 |= V_PCH_PMC_GEN_PMCON_SLP_S4_MAW_2S | B_PCH_PMC_GEN_PMCON_SLP_S4_ASE;
      break;

    case PchSlpS43s:
      RegData16 |= V_PCH_PMC_GEN_PMCON_SLP_S4_MAW_3S | B_PCH_PMC_GEN_PMCON_SLP_S4_ASE;
      break;

    case PchSlpS44s:
    default:
      RegData16 |= V_PCH_PMC_GEN_PMCON_SLP_S4_MAW_4S | B_PCH_PMC_GEN_PMCON_SLP_S4_ASE;
      break;
  }

  if (PchPlatformPolicy->MiscPmConfig->SlpStrchSusUp == PCH_DEVICE_DISABLE) {
    RegData16 |= B_PCH_PMC_GEN_PMCON_DISABLE_SX_STRETCH;
  }

  MmioWrite16 (PmcBase + R_PCH_PMC_GEN_PMCON_1, RegData16);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN)(PmcBase + R_PCH_PMC_GEN_PMCON_1),
    1,
    (VOID *) (UINTN) (PmcBase + R_PCH_PMC_GEN_PMCON_1)
    );

  ///
  /// VLV BIOS Spec Rev 0.5, Section 28.2 Serial IRQs
  /// The only System BIOS requirement to use IRQs as a serial IRQ is to enable the function
  /// in IBASE + 0x60 [12] and select continuous or quiet mode, IBASE + 0x10 [7].
  ///
  if ((PchPlatformPolicy->SerialIrqConfig->SirqEnable == TRUE) &&
      (PchPlatformPolicy->SerialIrqConfig->SirqMode == PchQuietMode)) {
    MmioAnd8 (IlbBase + R_PCH_ILB_SERIRQ_CNT, (UINT8) ~B_PCH_ILB_SERIRQ_CNT_SIRQMD);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint8,
      (UINTN) (IlbBase + R_PCH_ILB_SERIRQ_CNT),
      1,
      (VOID *) (UINTN) (IlbBase + R_PCH_ILB_SERIRQ_CNT)
      );
  }

  DEBUG ((EFI_D_INFO, "ConfigureMiscItems() End\n"));

  return EFI_SUCCESS;
}
