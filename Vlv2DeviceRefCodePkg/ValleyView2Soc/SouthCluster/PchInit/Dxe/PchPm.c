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
  PchPm.c

  @brief
  Initializes PCH power management features.

**/
#include "PchInit.h"

EFI_STATUS
ConfigureClockGating (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN  UINT32                              RootComplexBar,
  IN  UINT32                              PmcBase,
  IN  UINT32                              SpiBase,
  IN  UINT32                              FuncDisableReg
  )
/**

  @brief
  Perform Clock Gating programming
  Enables clock gating in various PCH interfaces and the registers must be restored during S3 resume.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] PmcBase              PmcBase value of this PCH device
  @param[in] SpiBase              SpiBase value of this PCH device
  @param[in] FuncDisableReg       The Function Disable Register

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINT32      RegData32;
  UINTN       PciD31F3RegBase;
  UINTN       AzaliaBase;

  DEBUG ((EFI_D_INFO, "ConfigureClockGating() Start\n"));

  PciD31F3RegBase = MmPciAddress (0, PchPlatformPolicy->BusNumber, 31, 3, 0);
  AzaliaBase = MmPciAddress (0,
                 PchPlatformPolicy->BusNumber,
                 PCI_DEVICE_NUMBER_PCH_AZALIA,
                 PCI_FUNCTION_NUMBER_PCH_AZALIA,
                 0
                 );
  ///
  /// 4
  /// USB 1.1 / USB 2.0 / USB 3.0
  ///
  ConfigureUsbClockGating (PchPlatformPolicy, RootComplexBar);
  //
  // Intel High Definition Audio (HDA) controller.
  //
  if (FuncDisableReg & B_PCH_PMC_FUNC_DIS_AZALIA) {
    //
    // If the HD Audio Controller is not being used, D27:F0 can be disabled and statically gated. Only statically
    // gate the Intel High Definition Audio controller if it is not being used in the system by setting RCBA + 20h[21].
    //
    MmioOr32 ((UINTN) (RootComplexBar + R_PCH_RCRB_HCGE), (UINT32) (B_PCH_RCRB_HCGE_AZSCG));
  } else {
    //
    // When the Intel High Definition Audio controller is used in the system,
    // dynamic clock gating can be used by setting RCBA + 20h[22].
    // Besides that, system BIOS is required to set D27:F0:43h [2:0] = 101b for dynamic clock gating.
    //
    MmioOr32 ((UINTN) (RootComplexBar + R_PCH_RCRB_HCGE), (UINT32) (B_PCH_RCRB_HCGE_AZDCG));
    MmioAndThenOr8 ((UINTN) (AzaliaBase + R_PCH_HDA_TM1),
                    (UINT8)~(B_PCH_HDA_TM1_MDCGEN | B_PCH_HDA_TM1_IDCGEN | B_PCH_HDA_TM1_ODCGEN),
                    (UINT8) (B_PCH_HDA_TM1_MDCGEN | B_PCH_HDA_TM1_ODCGEN));
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint8,
      (UINTN) (AzaliaBase + R_PCH_HDA_TM1),
      1,
      (VOID *) (UINTN) (AzaliaBase + R_PCH_HDA_TM1)
      );
  }

  //
  // SPI Clock gating.
  // Enable SPI clock gating by programming SBASE + C0h [2:0] to 111b
  // Clear the "Functional Clock Gating Disable" and the "SideBand Control Gating Clock Defeature" bits, SBASE + 0x100 [10:9]
  //
  MmioOr32 (
    (UINTN) (SpiBase + R_PCH_SPI_AFC),
    (UINT32) (B_PCH_SPI_AFC_INF_DCGE | B_PCH_SPI_AFC_CORE_DCGE)
    );
  RegData32 = MmioRead32 (SpiBase + R_PCH_SPI_AFC);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (SpiBase + R_PCH_SPI_AFC),
    1,
    &RegData32
    );
  //
  // SMBus
  // Enable SMBus dynamic clock gating by setting D31:F3:80h [8, 10, 12 and 14] = 0b respectively
  //
  MmioAnd16 ((UINTN) (PciD31F3RegBase + R_PCH_SMBUS_SMBSM),
             (UINT16)~(B_PCH_SMBUS_SMBSM_BBDCGDIS | B_PCH_SMBUS_SMBSM_TRDCGDIS | B_PCH_SMBUS_SMBSM_LNDCGDIS | B_PCH_SMBUS_SMBSM_PHDCGDIS));
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint16,
    (UINTN) (PciD31F3RegBase + R_PCH_SMBUS_SMBSM),
    1,
    (VOID *) (UINTN) (PciD31F3RegBase + R_PCH_SMBUS_SMBSM)
  );

//
//
// Set PMC_MTPMC3 bit2=1 to enable PCIe dynamic clock gating
  if(PchPlatformPolicy->PciExpressConfig->PcieDynamicGating) {
    MmioOr32(PMC_BASE_ADDRESS + R_PCH_PMC_MTPMC3, BIT2);
  } else {
    MmioAnd32(PMC_BASE_ADDRESS + R_PCH_PMC_MTPMC3, (UINT32)(~BIT2));
  }
  RegData32 = MmioRead32 (PMC_BASE_ADDRESS + R_PCH_PMC_MTPMC3);
  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (PMC_BASE_ADDRESS + R_PCH_PMC_MTPMC3),
    1,
    &RegData32
  );
//
//

  DEBUG ((EFI_D_INFO, "ConfigureClockGating() End\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
ConfigureMiscPm (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN  UINT32                              PmcBase,
  IN  UINT16                              GpioBase
  )
/*++

Routine Description:

  Configure miscellaneous power management settings

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance
  PmcBase                 PMC base address of this PCH device
  GpioBase                GPIO base address of this PCH device

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
{
  UINT32  RegData32;
  UINT16  RegData16;
  UINT32  RegData32Tmp;
  DEBUG ((EFI_D_INFO, "ConfigureMiscPm() Start\n"));

  //
  // Clear power / reset status bits on PCH Corporate
  //
  RegData32 = 0;
  /*if (PchPlatformPolicy->MiscPmConfig->PowerResetStatusClear.MeWakeSts) {
    RegData32 |= B_PCH_RCRB_PRSTS_ME_WAKE_STS;
  }

  if (PchPlatformPolicy->MiscPmConfig->PowerResetStatusClear.MeHrstColdSts) {
    RegData32 |= B_PCH_PMC_PRSTS_SEC_HRST_COLD_STS;
  }

  if (PchPlatformPolicy->MiscPmConfig->PowerResetStatusClear.MeHrstWarmSts) {
    RegData32 |= B_PCH_PMC_PRSTS_SEC_HRST_WARM_STS;
  }

  if (PchPlatformPolicy->MiscPmConfig->PowerResetStatusClear.MeHostPowerDn) {
    RegData32 |= B_PCH_PMC_PRSTS_SEC_HOST_PWRDN;
  }*/

  if (PchPlatformPolicy->MiscPmConfig->PowerResetStatusClear.WolOvrWkSts) {
    RegData32 |= B_PCH_PMC_PRSTS_WOL_OVR_WK_STS;
  }

  MmioOr32 (PmcBase + R_PCH_PMC_PRSTS, RegData32);
  RegData32Tmp = 0xFFFFFFFF;
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    PmcBase + R_PCH_PMC_PRSTS,
    &RegData32,   // OR mask
    &RegData32Tmp // AND mask
  );

  //
  // Handle wake policy
  // Don't need to record in S3 script as R_PCH_LPC_GEN_PMCON_3 is in RTC and SUS power well
  //
  RegData16 = MmioRead16 (PmcBase + R_PCH_PMC_GEN_PMCON_1) &
              (UINT16) (~(B_PCH_PMC_GEN_PMCON_PME_B0_S5_DIS +
                          //B_PCH_PMC_GEN_PMCON_SLP_LAN_DEFAULT +
                          B_PCH_PMC_GEN_PMCON_WOL_ENABLE_OVERRIDE));

  if (PchPlatformPolicy->MiscPmConfig->WakeConfig.PmeB0S5Dis) {
    RegData16 |= B_PCH_PMC_GEN_PMCON_PME_B0_S5_DIS;
  }
#if GPIO_ACCESS
  RegData32Tmp = IoRead32 ((UINTN) (GpioBase + R_PCH_GPIO_SC_LVL)) & (~BIT29);

  if (PchPlatformPolicy->MiscPmConfig->WakeConfig.WolEnableOverride) {
    //
    // PCH BIOS Spec Rev 0.7.0 Section 10.5 Wake-On-LAN (WOL) Implementation
    // Step 1
    // "SLP_LAN# Default Value (SLP_LAN_DEFAULT)", D31:F0:A4h:[8], bit must be
    // set to 1b to power up the phy after G3 and other global reset scenarios
    //
    //RegData16 |= B_PCH_PMC_GEN_PMCON_SLP_LAN_DEFAULT;
    //
    // Step 2
    // BIOS should ideally set the GPIO[29] to output mode driving a high level
    // (Note that GPIO29 is always 1b when read back, please refer to PCH EDS for
    // more details). However if enabling WOL in all Sx states, then step #1 could
    // be done without step #2
    //
    RegData32Tmp |= BIT29;
    //
    // Step 3
    // Set "WOL Enable Override", D31:F0:A4h:[13], bit to 1b to guarantee the
    // LAN-Wakes are enabled at the Power Management Controller, even in surprise
    // S5 cases such as power loss/return and Power Button Override
    //
    RegData16 |= B_PCH_PMC_GEN_PMCON_WOL_ENABLE_OVERRIDE;
    //
    // Step 4
    // "PME_B0_EN", ABASE + Offset 28h[13], bit must be programmed to enable wakes
    // from S1-S4 at the Power Management Controller
    // Done in ASL code(_PRW)
    //
  }
#endif
  MmioWrite16 (PmcBase + R_PCH_PMC_GEN_PMCON_1, RegData16);
#if GPIO_ACCESS
  //
  // Programmed the output level of GPIO29(SLP_LAN#) according to the platform policy "WolEnableOverride".
  //
  IoWrite32 (
    (UINTN) (GpioBase + R_PCH_GPIO_SC_LVL),
    RegData32Tmp
  );
  S3BootScriptSaveIoWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (GpioBase + R_PCH_GPIO_SC_LVL),
    1,
    &RegData32Tmp
  );
#endif
  DEBUG ((EFI_D_INFO, "ConfigureMiscPm() End\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
ConfigureAdditionalPm (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  )
/**

  @brief
  Configure additional power management settings

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINT32                USBPllCfgs = 0;
  UINTN                 EhciPciMmBase;
  UINT32                Buffer32 = 0;

  DEBUG ((EFI_D_INFO, "ConfigureAdditionalPm() PchPlatformPolicy->EhciPllCfgEnable = 0x%x \n",PchPlatformPolicy->EhciPllCfgEnable));

  if(PchPlatformPolicy->EhciPllCfgEnable) {

    /*
    (a) Bridge setting (using PythonSV):
    vlv.usb.ehci.usb_bridge.ch1ctrl=0x1
    vlv.usb.ehci.usb_bridge.portctrl0=0x1
    vlv.usb.ehci.usb_bridge.portctrl1=0x2
    vlv.usb.ehci.usb_bridge.portctrl2=0
    vlv.usb.ehci.usb_bridge.pmctl=0x9E  (IOSF PRI clock gating=0 due to bug)
    vlv.usb.ehci.usb_bridge.bdgdbgctl=0x7
    vlv.usb.ehci.usb_bridge.bdgctl=0x3

      PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_CH1CTRL,
      Buffer32,
      ~(B_IOSF_SPXB_TCX1),
      BIT0,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );


      PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PORTCTL0,
      Buffer32,
      ~(B_IOSF_SPXB_PORT0_DEVICE_BW_TYPE),
      BIT0,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );


      PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PORTCTL1,
      Buffer32,
      ~(B_IOSF_SPXB_PORT1_DEVICE_BW_TYPE),
      BIT1,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );


      PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PORTCTL2,
      Buffer32,
      ~(B_IOSF_SPXB_PORT2_DEVICE_BW_TYPE),
      0x00,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );


      PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PMCTL,
      Buffer32,
      0x00,
      (B_IOSF_SPXB_PMCTL_CLK_GATE_EN |
       B_IOSF_SPXB_PMCTL_CLK_US_GATE_EN |
       B_IOSF_SPXB_PMCTL_CLK_DS_GATE_EN |
       B_IOSF_SPXB_PMCTL_IOSFSB_GATE_EN |
       B_IOSF_SPXB_PMCTL_IOSFSB_GATE_EN |
       B_IOSF_SPXB_PMCTL_SB_TK_GT_EN),
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );


      PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_BDGDBGCTL,
      Buffer32,
      0x00,
      0x07,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );


      PchMsgBusAndThenOr32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_BDGCTL,
      Buffer32,
      0x00,
      (B_IOSF_SPXB_IOSF_PASS_THROUGH |
      B_IOSF_SPXB_PASS_THROUGH),
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );
    */

//Dump Msg Registers
    PchMsgBusRead32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_CH1CTRL,
      USBPllCfgs,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );
    DEBUG ((EFI_D_INFO, "ConfigureAdditionalPm() R_IOSF_SPXB_CH1CTRL = 0x%x \n",USBPllCfgs));

    PchMsgBusRead32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PORTCTL0,
      USBPllCfgs,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );
    DEBUG ((EFI_D_INFO, "ConfigureAdditionalPm() R_IOSF_SPXB_PORTCTL0 = 0x%x \n",USBPllCfgs));

    PchMsgBusRead32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PORTCTL1,
      USBPllCfgs,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );
    DEBUG ((EFI_D_INFO, "ConfigureAdditionalPm() R_IOSF_SPXB_PORTCTL1 = 0x%x \n",USBPllCfgs));

    PchMsgBusRead32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PORTCTL2,
      USBPllCfgs,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );
    DEBUG ((EFI_D_INFO, "ConfigureAdditionalPm() R_IOSF_SPXB_PORTCTL2 = 0x%x \n",USBPllCfgs));

    PchMsgBusRead32 (
      IOSF_SPXB_PORT_ID,
      R_IOSF_SPXB_PMCTL,
      USBPllCfgs,
      IOSF_SPXB_PRIVATE_READ_OPCODE,
      IOSF_SPXB_PRIVATE_WRITE_OPCODE
      );
    DEBUG ((EFI_D_INFO, "ConfigureAdditionalPm() R_IOSF_SPXB_PMCTL = 0x%x \n",USBPllCfgs));


    /*
    (b) Controller setting (using PythonSV):
    vlv.usb.ehci.controller.afemctltmsfe.s0pllshutdownen_0 = 1
    vlv.usb.ehci.controller.afemctltmsfe.s0pllshutdownrmhdspwroff_0 = 1
    vlv.usb.ehci.controller.pm_cs.pmeen_0  = 0
    vlv.usb.ehci.controller.ulscs.smionusbee_0 = 0
    vlv.usb.ehci.controller.ulscs.smionpce_0 =0
    vlv.usb.ehci.controller.ulscs.smionossoe_0 = 0
    vlv.usb.ehci.controller.ulscs.smionpce_0 = 0
    vlv.usb.ehci.controller.ulscs.smionusbce_0 = 0
    */
    EhciPciMmBase = MmPciAddress (0,
                                  0,
                                  PCI_DEVICE_NUMBER_PCH_USB,
                                  PCI_FUNCTION_NUMBER_PCH_EHCI,
                                  0
                                  );
    DEBUG ((EFI_D_INFO, "ConfigureAdditionalPm() EhciPciMmBase = 0x%x \n",EhciPciMmBase));

    MmioAnd32(
      (EhciPciMmBase + R_PCH_EHCI_AFEMCTLTM),
      (UINT32) (B_PCH_EHCI_AFEMCTLTM_S0PLLSHUTDOWNEN_0)
      );

    MmioAnd32(
      (EhciPciMmBase + R_PCH_EHCI_AFEMCTLTM),
      (UINT32) (B_PCH_EHCI_AFEMCTLTM_S0PLLSHUTDOWNRMHDSPWROFF_0)
      );
    Buffer32 = MmioRead32(EhciPciMmBase + R_PCH_EHCI_AFEMCTLTM);
    DEBUG ((EFI_D_INFO, "ConfigureAdditionalPm() R_PCH_EHCI_AFEMCTLTM = 0x%x \n",Buffer32));
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (EhciPciMmBase + R_PCH_EHCI_AFEMCTLTM),
      1,
      &Buffer32
      );

    MmioAnd32(
      (EhciPciMmBase + R_PCH_EHCI_PWR_CNTL_STS),
      (UINT32)(~(B_PCH_EHCI_PWR_CNTL_STS_PME_EN))
      );
    Buffer32 = MmioRead32(EhciPciMmBase + R_PCH_EHCI_PWR_CNTL_STS);
    DEBUG ((EFI_D_INFO, "ConfigureAdditionalPm() R_PCH_EHCI_PWR_CNTL_STS = 0x%x \n",Buffer32));
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (EhciPciMmBase + R_PCH_EHCI_PWR_CNTL_STS),
      1,
      &Buffer32
      );

  // Don't set INT disable, breaks USB in Linux
//  MmioOr32(
//        (EhciPciMmBase + R_PCH_EHCI_COMMAND_REGISTER),
//        (UINT32)(B_PCH_EHCI_COMMAND_INTR_DIS)
//        );
    Buffer32 = MmioRead32(EhciPciMmBase + R_PCH_EHCI_COMMAND_REGISTER);
    DEBUG ((EFI_D_INFO, "ConfigureAdditionalPm() R_PCH_EHCI_CMD_STS = 0x%x \n",Buffer32));
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (EhciPciMmBase + R_PCH_EHCI_COMMAND_REGISTER),
      1,
      &Buffer32
      );

    MmioAnd32(
      (EhciPciMmBase + R_PCH_EHCI_LEGEXT_CS),
      (UINT32)(~B_PCH_EHCI_LEGEXT_CS_SMIERR_EN)
      );

    MmioAnd32(
      (EhciPciMmBase + R_PCH_EHCI_LEGEXT_CS),
      (UINT32)(~B_PCH_EHCI_LEGEXT_CS_SMIPCD_EN)
      );

    MmioAnd32(
      (EhciPciMmBase + R_PCH_EHCI_LEGEXT_CS),
      (UINT32)(~B_PCH_EHCI_LEGEXT_CS_SMIOS_EN)
      );

    MmioAnd32(
      (EhciPciMmBase + R_PCH_EHCI_LEGEXT_CS),
      (UINT32)(~B_PCH_EHCI_LEGEXT_CS_SMIPCD_EN)
      );

    MmioAnd32(
      (EhciPciMmBase + R_PCH_EHCI_LEGEXT_CS),
      (UINT32)(~B_PCH_EHCI_LEGEXT_CS_SMICOMP_EN)
      );

    Buffer32 = MmioRead32(EhciPciMmBase + R_PCH_EHCI_LEGEXT_CS);
    DEBUG ((EFI_D_INFO, "ConfigureAdditionalPm() R_PCH_EHCI_LEGEXT_CS = 0x%x \n",Buffer32));
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (EhciPciMmBase + R_PCH_EHCI_LEGEXT_CS),
      1,
      &Buffer32
      );

  }

  return EFI_SUCCESS;
}

EFI_STATUS
ProgramDeepSx (
  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  UINT32                              RootComplexBar
  )
/**

  @brief
  Configure deep Sx programming

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  return EFI_SUCCESS;
}

EFI_STATUS
ConfigureS0ix (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN  UINT32                              PmcBase
  )
/**

  @brief
  Configure S0ix settings

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] PmcBase              PmcBase value of this PCH device

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINT32 Data32;

  DEBUG ((EFI_D_INFO, "ConfigureS0ix() Start \n"));

  //
  // Enable S0ix in the registers, Program max_s0ix
  //
  MmioOr32 (PmcBase + R_PCH_PMC_PMIR, B_PCH_PMC_PMIR_MAX_S0IX);

  //
  // Set up the S0IX_WAKE_EN register
  //
  MmioWrite32 (PmcBase + R_PCH_PMC_S0IX_WAKE_EN, ~(UINT32)( \
               B_PCH_PMC_S0IX_WAKE_EN_SHARED_IRQ_FROM_GPSS | \
               B_PCH_PMC_S0IX_WAKE_EN_ORED_DEDICATED_IRQS_FROM_GPSC | \
               B_PCH_PMC_S0IX_WAKE_EN_ORED_DEDICATED_IRQS_FROM_GPSS | \
               B_PCH_PMC_S0IX_WAKE_EN_SHARED_IRQ_FROM_GPNC | \
               B_PCH_PMC_S0IX_WAKE_EN_GPE_FROM_GPSS | \
               B_PCH_PMC_S0IX_WAKE_EN_GPE_FROM_GPSC | \
               B_PCH_PMC_S0IX_WAKE_EN_SHARED_IRQ_FROM_GPSC | \
               B_PCH_PMC_S0IX_WAKE_EN_LPC_CLOCK_RUN));

  //
  // Program C7_SUSBSTATE MSR with wakeup latencies
  // uCode will set it
  //

  //
  // Set up the S0ix ramp-up time
  // Maintain the original value, confirming the bit encoding
  //
  Data32 = MmioRead32 (PmcBase + R_PCH_PMC_S0IX_CTL);
  MmioWrite32 (PmcBase + R_PCH_PMC_S0IX_CTL, Data32 & B_PCH_PMC_S0IX_CTL_S0IX_RR);

  //
  // Set up the Punit exit latency registers
  // Can't found the reg description of Punit exit latency registers.Under confirming
  //

  //
  // Set up the expected SoC behavior when in S0ix
  //
  MmioOr32 (PmcBase + R_PCH_PMC_PMIR, B_PCH_PMC_PMIR_IGNORE_HPET);

  DEBUG ((EFI_D_INFO, "ConfigureS0ix() End \n"));

  return EFI_SUCCESS;
}

VOID
ConfigureAcpiHwRed (
  IN  UINT16                                    AcpiBase
  )
{
  UINT32 Gpe0aEn;
  UINT32 Data32;
  //
  // Silicon Steppings: For B1 and later stepping, disable PME_B0_EN SCI/SMI. Map to virtual GPIO 6
  //
  if (PchStepping()>= PchB1) {
    //
    // 1.  Clear GPE0a_EN.PME_B0_EN
    //
    Gpe0aEn = IoRead32 (AcpiBase + R_PCH_ACPI_GPE0a_EN);
    Gpe0aEn &= ~(UINT32)(B_PCH_ACPI_GPE0a_EN_PME_B0);
    IoWrite32 (AcpiBase + R_PCH_ACPI_GPE0a_EN, Gpe0aEn);

    //
    // 2.  Configure IOBASE.cfio_regs_pad_vgpio_6_PCONF0 as follows:
    //    direct_irq_en = 1'b0, shared IRQ
    //    gd_tne = 1'b0 and gd_tpe = 1'b1, active HIGH
    //    gd_level = 1'b1, level mode
    //
    Data32 = MmioRead32 (IO_BASE_ADDRESS + 0x0710);
    Data32 &= ~(UINT32)(BIT27|BIT26);
    Data32 |= (BIT25|BIT24);
    MmioWrite32 (IO_BASE_ADDRESS + 0x0710, Data32);
  }
}
