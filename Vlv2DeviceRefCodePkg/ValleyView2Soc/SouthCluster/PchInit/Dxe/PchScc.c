/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PchScc.c

Abstract:

  Initializes PCH SCC Device

--*/
#include "PchInit.h"

#define SCC_4591489_WORKAROUND          1

extern EFI_GUID gEfiEventExitBootServicesGuid;

EFI_STATUS
SetNslewPslew(UINT32 Value)
{
  UINT32                Buffer32 = 0;
  //
  // 0x48c0
  // set cfio_regs_mmc1_ELECTRICAL.nslew/pslew = 0x0/3
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    0x48C0,
    Buffer32,
    (UINT32)(~(0x3 << 4 | 0x3 << 2 )),
    (UINT32)(Value << 4 | Value << 2),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  // 0x48C4
  // set cfio_regs_mmc1_clk_ELECTRICAL.nslew/pslew = 0x0/3
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    0x48C4,
    Buffer32,
    (UINT32)(~(0x3 << 4 | 0x3 << 2 )),
    (UINT32)(Value << 4 | Value << 2),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );
  return EFI_SUCCESS;
}

VOID
EFIAPI
OnExitToConfigureEMMCSignal(
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS                       Status;
  DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPchPolicy;

  Status  = gBS->LocateProtocol (&gDxePchPlatformPolicyProtocolGuid, NULL, (VOID **) &PchPlatformPchPolicy);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Locate the gDxePchPlatformPolicyProtocolGuid Failed\n"));
  } else {
    if (PchPlatformPchPolicy->SccConfig->eMMCEnabled) {
      DEBUG ((EFI_D_ERROR, "Using eMMC 4.41\n"));
    } else if (PchPlatformPchPolicy->SccConfig->eMMC45Enabled) {
      DEBUG ((EFI_D_ERROR, "Using eMMC 4.5 \n"));
      if (PchPlatformPchPolicy->SccConfig->eMMC45DDR50Enabled) {
        DEBUG ((EFI_D_ERROR, "Overwrite the nslew/pslew -> 0 \n"));
        SetNslewPslew(0x0);
      }
      if (PchPlatformPchPolicy->SccConfig->eMMC45HS200Enabled) {
        DEBUG ((EFI_D_ERROR, "Overwrite the nslew/pslew -> 3 \n"));
        SetNslewPslew(0x3);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "eMMC device is not enabled!!!\n"));
    }
  }

  return;
}

EFI_STATUS
ConfigureEMMC45 (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy,
  IN     UINTN SccPciMmBase
  )
/*++

Routine Description:

  Configure eMMC4.5 devices.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
{
#define GEN_REGRW1_DDR50_VALUE   0x446CC801   //0b 0100 0100 0110 1100 1100 1000 0000 0001
#define GEN_REGRW2_DDR50_VALUE   0x5          //0b 0000 0000 0000 0000 0000 0000 0000 0101  >> bit 40-43 set to 0
#define GEN_REGRW1_HS200_VALUE   0x446CC801   //0b 0100 0100 0110 1100 1100 1000 0000 0001
#define GEN_REGRW2_HS200_VALUE   0x807        //0b 0000 0000 0000 0000 0000 1000 0000 0111 >> bit 40-43 set to default 8

  UINT32 Data = 0;
  UINT32 Buffer32 = 0;
  EFI_EVENT    Event = NULL;
  EFI_STATUS  Status;
  EFI_EVENT    PmAuthEvent = NULL;
  VOID        *RegistrationExitPmAuth = NULL;

  if (PchPlatformPolicy->SccConfig->eMMC45Enabled == PCH_DEVICE_ENABLE) {

    // Silicon Steppings
    // Stepping >= B1, apply 2ms_card_stable
    switch (PchStepping()) {
      case PchA0:
        break;
      case PchA1:
        break;
      case PchB0:
        break;
      case PchB1:
      default:
        DEBUG ((EFI_D_ERROR, "Enable 2ms_card_stable feature for B1 and later SOC!\n"));
        Data = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW3));
        Data &= ~(BIT24);
        Data |= BIT24;
        MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW3), Data);
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW3),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW3)
          );
        break;
    }
    DEBUG ((EFI_D_INFO, "ConfigureEMMC45: Overwride Capability Register\n"));

    if (PchPlatformPolicy->SccConfig->eMMC45DDR50Enabled) {
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1), GEN_REGRW1_DDR50_VALUE);
      S3BootScriptSaveMemWrite(
        EfiBootScriptWidthUint32,
        (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1),
        1,
        (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1)
        );
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2), GEN_REGRW2_DDR50_VALUE | B_PCH_SCC_SDIO_CAP_REG_SEL_GEN);
      S3BootScriptSaveMemWrite(
        EfiBootScriptWidthUint32,
        (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2),
        1,
        (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2)
        );
    }
    if (PchPlatformPolicy->SccConfig->eMMC45HS200Enabled) {
      Data = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1));
      DEBUG ((EFI_D_INFO, "ConfigureEMMC45: set retume timer value 0x%x\n",PchPlatformPolicy->SccConfig->eMMC45RetuneTimerValue ));
      Data = GEN_REGRW2_HS200_VALUE;
      Data &= ~(0xF<<(40-32));
      Data |= ((PchPlatformPolicy->SccConfig->eMMC45RetuneTimerValue) & 0xF) << (40-32);
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1), GEN_REGRW1_HS200_VALUE);
      S3BootScriptSaveMemWrite(
        EfiBootScriptWidthUint32,
        (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1),
        1,
        (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1)
        );
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2), Data | B_PCH_SCC_SDIO_CAP_REG_SEL_GEN);
      S3BootScriptSaveMemWrite(
        EfiBootScriptWidthUint32,
        (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2),
        1,
        (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2)
        );
    }
    DEBUG ((EFI_D_INFO, "ConfigureEMMC45: New Capability Reg = 0x%x-%x\n", \
            MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2))&0xEFFFFFFF, \
            MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1))));

    //
    // 0x49C0
    //cfio_regs_score_special_bits.sdio1_dummy_loopback_en=1
    //
    PchMsgBusAndThenOr32AddToS3Save (
      CFIO_SCORE_SB_PORT_ID,
      0x49C0,
      Buffer32,
      (UINT32)(~(0x1<<3)),
      (UINT32)(0x1<<3),
      PCH_SCC_EP_PRIVATE_READ_OPCODE,
      PCH_SCC_EP_PRIVATE_WRITE_OPCODE
      );
    //
    // 0x1C
    // CLKGATE_EN_1 . cr_scc_mipihsi_clkgate_en  = 1
    //
    PchMsgBusAndThenOr32AddToS3Save (
      0xA9, // CCU
      0x1C, // Offset
      Buffer32,
      (UINT32) ~(0x1<<27 | 0x1<<26), // AND
      0x1<<26,  // OR
      0x6, //PCH_CCU_READ_OPCODE,
      0x7 //PCH_CCU_WRITE_OPCODE
      );

    DEBUG ((EFI_D_INFO, "Register the call back for eMMC singalling configuration.\n"));
    //
    // Register it with the lowest priority level
    //
    gBS->CreateEvent (
           EVT_NOTIFY_SIGNAL,
           TPL_CALLBACK,
           OnExitToConfigureEMMCSignal,
           NULL,
           &PmAuthEvent
           );

    gBS->RegisterProtocolNotify (
           &gExitPmAuthProtocolGuid,
           PmAuthEvent,
           &RegistrationExitPmAuth
           );

    Status = EfiCreateEventLegacyBootEx (
               TPL_CALLBACK,
               OnExitToConfigureEMMCSignal,
               NULL,
               &Event
               );
    ASSERT_EFI_ERROR (Status);
  } else {
    DEBUG ((EFI_D_INFO, "eMMC 4.5 is disabled\n"));
  }
  return EFI_SUCCESS;
}

EFI_STATUS
ConfigureSdCardCap (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy,
  IN     UINTN SccPciMmBase
  )
/*++

Routine Description:

  Configure SdCard capability register.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
{
#define GEN_REGRW1_SDR25_VALUE   0x76864b2    //0b 0000 0111 0110 1000 1100 0100 1101 0010
#define GEN_REGRW2_SDR25_VALUE   0x0          //0b 0000 0000 0000 0000 0000 0000 0000 0000  >> bit 32-34 set to 0


  if (PchPlatformPolicy->SccConfig->SdcardEnabled == PCH_DEVICE_ENABLE) {

    DEBUG ((EFI_D_INFO, "ConfigureSdCardCap: Overwride Capability Register\n"));

    if (PchPlatformPolicy->SccConfig->SdCardSDR25Enabled) { // When enable SDR25, clen the DDR50, SDR50, SDR104 support
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1), GEN_REGRW1_SDR25_VALUE);
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2), GEN_REGRW2_SDR25_VALUE | B_PCH_SCC_SDIO_CAP_REG_SEL_GEN);
    } else if (PchPlatformPolicy->SccConfig->SdCardDDR50Enabled) { // When enable DDR50, keep existing value
      DEBUG ((EFI_D_INFO, "Do nothing here for DDR50, keep existing settings\n"));
    } else if (PchPlatformPolicy->SccConfig->SdCardDDR50Enabled == PCH_DEVICE_DISABLE) { //when disable DDR50, clean the DDR50 bit
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1), GEN_REGRW1_SDR25_VALUE);
      MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2), GEN_REGRW2_SDR25_VALUE | B_PCH_SCC_SDIO_CAP_REG_SEL_GEN);

    }
    DEBUG ((EFI_D_INFO, "ConfigureSdCardCap: New Capability Reg = 0x%x-%x\n", \
            MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2))&0xEFFFFFFF, \
            MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1))));

  } else {
    DEBUG ((EFI_D_INFO, "SD card is disabled\n"));
  }
  return EFI_SUCCESS;
}



EFI_STATUS
ConfigureDLLSettingForEMMC41_BBAY (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy
  )
/*++

Routine Description:

  Configure DLL setting for eMMC4.5 controller.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
{
  UINT32                Buffer32 = 0;
  UINT32                MDL_FSM_VALS = 0;
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "ConfigureDLLSettingForEMMC41: eMMC DLL Settings for BayleyBay .\n"));
  //
  //DLL settings for eMMC
  //

  //
  // 1. Configure Master DLL
  //

  //
  // C,F Init
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_INIT_SCORE_MDL_CF_INIT,
    Buffer32,
    0,
    0x78000,
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // Configure Swing,FSM for Master DLL
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_CTRL_SCORE_MDL_FSM_CTRL,
    Buffer32,
    0,
    0x133,
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // Run+Local Reset on Master DLL
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_CTRL_SCORE_MDL_FSM_CTRL,
    Buffer32,
    0,
    0x1933,
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 2. Populate Values from DLL
  //
  PchMsgBusRead32 (
    CFIO_SCORE_SB_PORT_ID,
    DLL_VALS_SCORE_MDL_FSM_VALS,
    MDL_FSM_VALS,
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 3. Override Slave Path
  //0x4950
  //  [19] = 1
  //  [18:15] = F
  //  [14:0] = 0FFF
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_WR_PATH_SCORE_MDL_WRITE_PATH_C_F_ADDR,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 20),
    (UINT32)((1<<19) | (MDL_FSM_VALS & MDL_FSM_VALS_MASK)),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 4.Configure Write Path
  // 0x4954
  //   [14:10] = 0xD
  //   [9:5]   = 0xD
  //   [4:0]   = 0xD
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_WR_PATH1_MUX_SCORE_DLL_WRITE_PATH1_MUX,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0xD << 10) | (0xD << 5) | 0xD),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 0x4958
  //   [14:10] = 0xD
  //   [9:5]   = 0xD
  //   [4:0]   = 0xD
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_WR_PATH2_MUX_SCORE_DLL_WRITE_PATH2_MUX,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0xD << 10) | (0xD << 5) | 0xD),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 0x495C
  //   [14:10] = 0xD
  //   [9:5]   = 0xD
  //   [4:0]   = 0xD
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_WR_PATH3_MUX_SCORE_DLL_WRITE_PATH3_MUX,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0xD << 10) | (0xD << 5) | 0xD),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 5.Configure Read Path
  //
  // 0x43E4
  //   [14:10] = 0x4
  //   [9:5]   = 0x2
  //   [4:0]   = 0x5
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    SDMMC1_CLK_PCONF1,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0x0D << 10) | (0x0D << 5) | 0x0D),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 0x4324
  //   [14:10] = 0x3
  //   [9:5]   = 0x3
  //   [4:0]   = 0x5
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    SDMMC2_CLK_PCONF1,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0x0D << 10) | (0x0D << 5) | 0x0D),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 0x42B4
  //   [14:10] = 0x4
  //   [9:5]   = 0x2
  //   [4:0]   = 0x5
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    SDMMC3_CLK_PCONF1,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0x0D << 10) | (0x0D << 5) | 0x0D),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );
  //
  //Set iosf2ocp_private.GENREGRW1.cr_clock_enable_clk_ocp = 01
  //Set iosf2ocp_private.GENREGRW1.cr_clock_enable_clk_xin = 01
  //
  switch (PchStepping()) {
    case PchA0:
    case PchA1:
      DEBUG ((EFI_D_ERROR, "No ACG setting on Ax stepping\n"));
      break;
    case PchB0:
    case PchB1:
    case PchB2:
      DEBUG ((EFI_D_ERROR, "ACG setting for Bx Stepping\n"));
      PchMsgBusAndThenOr32AddToS3Save (
        PCH_SCC_EP_PORT_ID,
        R_PCH_SCC_EP_GENREGRW1,
        Buffer32,
        0xFFFFFFF0,
        (UINT32)(0x1<<0 | 0x1<<2),
        PCH_SCC_EP_PRIVATE_READ_OPCODE,
        PCH_SCC_EP_PRIVATE_WRITE_OPCODE
        );

      break;
    default:
      DEBUG ((EFI_D_ERROR, "Unknown Steppting, No ACG setting on Ax stepping\n"));
      break;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ConfigureDLLSettingForEMMC45_BBAY (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy
  )
/*++

Routine Description:

    Configure DLL setting for eMMC4.41 controller.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
{
  UINT32                Buffer32 = 0;
  UINT32                MDL_FSM_VALS = 0;
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "ConfigureDLLSettingForEMMC45: eMMC DLL Settings for BayleyBay .\n"));

  //
  //DLL settings for eMMC
  //

  //
  // 1. Configure Master DLL
  //

  //
  // C,F Init
  // [18:15]: 4'h0, [14:0]->15'h0000
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_INIT_SCORE_MDL_CF_INIT,
    Buffer32,
    0,
    0x78000,
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // Configure Swing,FSM for Master DLL
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_CTRL_SCORE_MDL_FSM_CTRL,
    Buffer32,
    0,
    0x133,
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // Run+Local Reset on Master DLL
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_CTRL_SCORE_MDL_FSM_CTRL,
    Buffer32,
    0,
    0x1933,
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 2. Populate Values from DLL
  //
  PchMsgBusRead32 (
    CFIO_SCORE_SB_PORT_ID,
    DLL_VALS_SCORE_MDL_FSM_VALS,
    MDL_FSM_VALS,
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 3. Override Slave Path
  //0x4950
  //  [19] = 1
  //  [18:15] = F
  //  [14:0] = 0FFF
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_WR_PATH_SCORE_MDL_WRITE_PATH_C_F_ADDR,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 20),
    (UINT32)((1<<19) | (MDL_FSM_VALS & MDL_FSM_VALS_MASK)),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 4.Configure Write Path
  // 0x4954
  //   [14:10] = 0xD
  //   [9:5]   = 0xD
  //   [4:0]   = 0xD
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_WR_PATH1_MUX_SCORE_DLL_WRITE_PATH1_MUX,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0xD << 10) | (0xD << 5) | 0xD),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 0x4958
  //   [14:10] = 0xD
  //   [9:5]   = 0xD
  //   [4:0]   = 0xD
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_WR_PATH2_MUX_SCORE_DLL_WRITE_PATH2_MUX,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0xD << 10) | (0xD << 5) | 0xD),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 0x495C
  //   [14:10] = 0xD
  //   [9:5]   = 0xD
  //   [4:0]   = 0xD
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    DLL_WR_PATH3_MUX_SCORE_DLL_WRITE_PATH3_MUX,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0xD << 10) | (0xD << 5) | 0xD),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 5.Configure Read Path
  //
  // 0x43E4
  //   [14:10] = 0x4
  //   [9:5]   = 0x2
  //   [4:0]   = 0x5
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    SDMMC1_CLK_PCONF1,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0x0D << 10) | (0x0D << 5) | 0x0D),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 0x4324
  //   [14:10] = 0x3
  //   [9:5]   = 0x3
  //   [4:0]   = 0x5
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    SDMMC2_CLK_PCONF1,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0x0D << 10) | (0x0D << 5) | 0x0D),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 0x42B4
  //   [14:10] = 0x4
  //   [9:5]   = 0x2
  //   [4:0]   = 0x5
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    SDMMC3_CLK_PCONF1,
    Buffer32,
    (UINT32)(0xFFFFFFFF << 15),
    (UINT32)((0x0D << 10) | (0x0D << 5) | 0x0D),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // Below settings is for eMMC45 TX and RX DLL
  //
  //

  // 0x49A4
  //   [20:16] = 0xA // Set to 0xA intead of 0xD per HSD 4949614
  //   [4:0]   0xD
  //
  // Set gpscore.cfio_regs_SCORE_EMMC_45_HSMAX.tx_dat_dly_sel_sdr104 [16:20]
  // Set gpscore.cfio_regs_SCORE_EMMC_45_HSMAX.rx_dly_sel_hs200 [0:4]
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    0x49A4,
    Buffer32,
    (UINT32)~(0x1F | (0x1F << 16)),
    (UINT32)((0x0A << 16) | 0x0D),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );
  // 0x49A8
  //   [20:16] = 0xD
  //   [4:0]   = 0xD
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    0x49A8,
    Buffer32,
    (UINT32)~(0x1F | (0x1F << 16)),
    (UINT32)((0x0D << 16) | 0x0D),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );
  // 0x49AC
  //   [20:16] = 0xD
  //   [4:0]   = 0xD
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    0x49AC,
    Buffer32,
    (UINT32)~(0x1F | (0x1F << 16)),
    (UINT32)((0x0D << 16) | 0x0D),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );
  // 0x49B0
  //   [20:16] = 0xD
  //   [4:0]   = 0xD
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    0x49B0,
    Buffer32,
    (UINT32)~(0x1F | (0x1F << 16)),
    (UINT32)((0x0D << 16) | 0x0D),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );
  // 0x49B4
  //   [20:16] = 0xD
  //   [4:0]   = 0xD
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    0x49B4,
    Buffer32,
    (UINT32)~(0x1F | (0x1F << 16)),
    (UINT32)((0x0D << 16) | 0x0D),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );
  // 0x49B8
  //   [0]   = 0x0
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    0x49B8,
    Buffer32,
    (UINT32)0xFFFFFFFE,
    (UINT32)0x0,
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 0x48c0
  // set cfio_regs_mmc1_ELECTRICAL.nslew/pslew = 0x0
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    0x48C0,
    Buffer32,
    (UINT32)(~(0x3 << 4 | 0x3 << 2 )),
    (UINT32)(0x0 << 4 | 0x0 << 2),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  //
  // 0x48C4
  // set cfio_regs_mmc1_clk_ELECTRICAL.nslew/pslew = 0x0
  //
  PchMsgBusAndThenOr32AddToS3Save (
    CFIO_SCORE_SB_PORT_ID,
    0x48C4,
    Buffer32,
    (UINT32)(~(0x3 << 4 | 0x3 << 2 )),
    (UINT32)(0x0 << 4 | 0x0 << 2),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );
  //
  //Set iosf2ocp_private.GENREGRW1.cr_clock_enable_clk_ocp = 01
  //Set iosf2ocp_private.GENREGRW1.cr_clock_enable_clk_xin = 01
  //
  PchMsgBusAndThenOr32AddToS3Save (
    PCH_SCC_EP_PORT_ID,
    R_PCH_SCC_EP_GENREGRW1,
    Buffer32,
    0xFFFFFFF0,
    (UINT32)(0x1<<0 | 0x1<<2),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );
  return EFI_SUCCESS;
}

EFI_STATUS
ConfigureScc (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy,
  IN OUT UINT32                            *FuncDisableReg
  )
/*++

Routine Description:

  Configure SCC devices.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance
  FuncDisableReg          Function Disable Register

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
{
  EFI_STATUS            Status;
  UINTN                 SccPciMmBase = 0;
  EFI_PHYSICAL_ADDRESS  SccMmioBase0 = 0;
  UINT32                Buffer32 = 0;

  DEBUG ((EFI_D_INFO, "ConfigureScc() Start\n"));
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "eMMC DLL Settings for BBAY.\n"));
  if (PchPlatformPolicy->SccConfig->eMMCEnabled == PCH_DEVICE_ENABLE) {
    ConfigureDLLSettingForEMMC41_BBAY(PchPlatformPolicy);
  } else if (PchPlatformPolicy->SccConfig->eMMC45Enabled == PCH_DEVICE_ENABLE) {
    ConfigureDLLSettingForEMMC45_BBAY(PchPlatformPolicy);
  } 
// AMI_OVERRIDE - Config DLL for AMI SdioDriver <<
  else if ((PchPlatformPolicy->SccConfig->eMMCEnabled == PCH_DEVICE_DISABLE) &&
      ((PchPlatformPolicy->SccConfig->SdioEnabled == PCH_DEVICE_ENABLE) ||
      (PchPlatformPolicy->SccConfig->SdcardEnabled == PCH_DEVICE_ENABLE))) {
// AMI_OVERRIDE - Config DLL for AMI SdioDriver <<
    ConfigureDLLSettingForEMMC41_BBAY(PchPlatformPolicy);
  }

  ///
  /// Enable IOSF Snoop
  ///
  PchMsgBusAndThenOr32AddToS3Save (
    PCH_SCC_EP_PORT_ID,
    R_PCH_SCC_EP_IOSFCTL,
    Buffer32,
    0xFFFFFF7F,
    (B_PCH_SCC_EP_IOSFCTL_NSNPDIS),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

//SCC IOSF2OCP Bridge PortId: 0x63
//Register: 0x0 (IOSFCTL)
//Bit 7 (register bit NSNPDIS)

  ///
  /// SCC eMMC 4.41 is on Bus 0, Dev 16, Func 0
  ///
  SccPciMmBase = MmPciAddress (0,
                   DEFAULT_PCI_BUS_NUMBER_PCH,
                   PCI_DEVICE_NUMBER_PCH_SCC_SDIO_0,
                   PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,
                   0
                 );
  if (PchPlatformPolicy->SccConfig->eMMCEnabled == PCH_DEVICE_DISABLE) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting SCC eMMC 4.41 into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_PCS), B_PCH_SCC_SDIO_PCS_PS);
    S3BootScriptSaveMemWrite(
      EfiBootScriptWidthUint32,
      (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_PCS),
      1,
      (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_PCS)
      );
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_SDIO1;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (SccPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_SCC_SDIO_BAR_ALIGNMENT,
                      V_PCH_SCC_SDIO_BAR_SIZE,
                      &SccMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD), (UINT32) ~(B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((SccMmioBase0 & B_PCH_SCC_SDIO_BAR_BA) == SccMmioBase0) && (SccMmioBase0 != 0));
        MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR), (UINT32) (SccMmioBase0 & B_PCH_SCC_SDIO_BAR_BA));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD), (UINT32) (B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) SccMmioBase0) != 0xFFFFFFFF);
        ///
        /// Set Maximum Timeout to 0x0E
        ///
        MmioWrite8 ((UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL), (UINT8) 0x0E);
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint8,
          (UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL),
          1,
          (VOID *) (UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL)
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD), (UINT32) ~(B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)
          );
        gDS->FreeMemorySpace (SccMmioBase0, (UINT64) V_PCH_SCC_SDIO_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "SCC eMMC 4.41 not present, skipping.\n"));
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_SDIO1;
    }
  }

  ///
  /// SCC SDIO
  ///

  ///
  /// 3.0V Support in the Controller's capabilities, set vlv.scc.iosf2ocp.gen_regrw1.sdio_volt = 3
  ///

  PchMsgBusAndThenOr32AddToS3Save (
    PCH_SCC_EP_PORT_ID,
    R_PCH_SCC_EP_GENREGRW1,
    Buffer32,
    ~B_PCH_SCC_EP_GENREGRW1_SDIO_VOLT,
    (V_PCH_SCC_EP_GENREGRW1_SDIO_VOLT_V3),
    PCH_SCC_EP_PRIVATE_READ_OPCODE,
    PCH_SCC_EP_PRIVATE_WRITE_OPCODE
  );

  SccPciMmBase = MmPciAddress (0,
                   DEFAULT_PCI_BUS_NUMBER_PCH,
                   PCI_DEVICE_NUMBER_PCH_SCC_SDIO_1,
                   PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,
                   0
                 );
  if (PchPlatformPolicy->SccConfig->SdioEnabled == PCH_DEVICE_DISABLE) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting SCC SDIO into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_PCS), B_PCH_SCC_SDIO_PCS_PS);
    S3BootScriptSaveMemWrite(
      EfiBootScriptWidthUint32,
      (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_PCS),
      1,
      (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_PCS)
      );
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_SDIO2;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (SccPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_SCC_SDIO_BAR_ALIGNMENT,
                      V_PCH_SCC_SDIO_BAR_SIZE,
                      &SccMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {

        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD), (UINT32) ~(B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((SccMmioBase0 & B_PCH_SCC_SDIO_BAR_BA) == SccMmioBase0) && (SccMmioBase0 != 0));
        MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR), (UINT32) (SccMmioBase0 & B_PCH_SCC_SDIO_BAR_BA));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD), (UINT32) (B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) SccMmioBase0) != 0xFFFFFFFF);
#if SCC_4591489_WORKAROUND
        //
        // workaround for silicon bug 4591489 to disable suspend/resume support
        // for SDIO
        // vlv.scc.sdmmc2.gen_regrw1[23] = 0
        //
        // Silicon Steppings : Disable Transfer Suspend/Resume for SOC B0 and later
        switch (PchStepping()) {
          case PchA0:
          case PchA1:
            break;

          case PchB0: // B0 and later
          default:
            DEBUG ((EFI_D_ERROR, "Disable Transfer Suspend/Resume support for SOC B0 and later!\n"));
            Buffer32 = MmioRead32 ((UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_CAP1));
            MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1), (Buffer32 & ~BIT23));
            S3BootScriptSaveMemWrite(
              EfiBootScriptWidthUint32,
              (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1),
              1,
              (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW1)
              );
            Buffer32 = MmioRead32 ((UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_CAP2));
            MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2), Buffer32 | B_PCH_SCC_SDIO_CAP_REG_SEL_GEN);
            S3BootScriptSaveMemWrite(
              EfiBootScriptWidthUint32,
              (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2),
              1,
              (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_GEN_REGRW2)
              );
            break;
        }
#endif

        // Baytrail M/D skip below WIFI CS WA due to it caused Android WIFI initialization issue somehow

        ///
        /// Set Maximum Timeout to 0x0E
        ///
        MmioWrite8 ((UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL), (UINT8) 0x0E);
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint8,
          (UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL),
          1,
          (VOID *) (UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL)
          );
        ///
        /// Asynchronous Interrupt Disable, it is W/A for SDIO DDR50 mode
        ///
        MmioAnd32 ((UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_CESHC2), (UINT32)~(B_PCH_SCC_SDIO_MEM_CESHC2_ASYNC_INT));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_CESHC2),
          1,
          (VOID *) (UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_CESHC2)
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD), (UINT32) ~(B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)
          );
        gDS->FreeMemorySpace (SccMmioBase0, (UINT64) V_PCH_SCC_SDIO_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "SCC SDIO not present, skipping.\n"));
      PchPlatformPolicy->SccConfig->SdioEnabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_SDIO2;
    }
  }

  ///
  /// SCC SD Card
  ///
  SccPciMmBase = MmPciAddress (0,
                   DEFAULT_PCI_BUS_NUMBER_PCH,
                   PCI_DEVICE_NUMBER_PCH_SCC_SDIO_2,
                   PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,
                   0
                 );
  if (PchPlatformPolicy->SccConfig->SdcardEnabled == PCH_DEVICE_DISABLE) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting SCC SD Card into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_PCS), B_PCH_SCC_SDIO_PCS_PS);
    S3BootScriptSaveMemWrite(
      EfiBootScriptWidthUint32,
      (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_PCS),
      1,
      (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_PCS)
      );
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_SDIO3;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (SccPciMmBase) != 0xFFFFFFFF) {
         // Override the Capability Register
         ConfigureSdCardCap(PchPlatformPolicy, SccPciMmBase);

      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_SCC_SDIO_BAR_ALIGNMENT,
                      V_PCH_SCC_SDIO_BAR_SIZE,
                      &SccMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD), (UINT32) ~(B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((SccMmioBase0 & B_PCH_SCC_SDIO_BAR_BA) == SccMmioBase0) && (SccMmioBase0 != 0));
        MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR), (UINT32) (SccMmioBase0 & B_PCH_SCC_SDIO_BAR_BA));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD), (UINT32) (B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) SccMmioBase0) != 0xFFFFFFFF);
        ///
        /// Set Maximum Timeout to 0x0E
        ///
        MmioWrite8 ((UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL), (UINT8) 0x0E);
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint8,
          (UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL),
          1,
          (VOID *) (UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL)
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD), (UINT32) ~(B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)
          );
        gDS->FreeMemorySpace (SccMmioBase0, (UINT64) V_PCH_SCC_SDIO_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "SCC SD Card not present, skipping.\n"));
      PchPlatformPolicy->SccConfig->SdcardEnabled = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_SDIO3;
    }
  }


  ///
  /// SCC eMMC 4.5 is on Bus 0, Dev 23, Func 0
  ///
  SccPciMmBase = MmPciAddress (0,
                               DEFAULT_PCI_BUS_NUMBER_PCH,
                               PCI_DEVICE_NUMBER_PCH_SCC_SDIO_3,
                               PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,
                               0
                               );
  if (PchPlatformPolicy->SccConfig->eMMC45Enabled == PCH_DEVICE_DISABLE) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting SCC eMMC 4.5 into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_PCS), B_PCH_SCC_SDIO_PCS_PS);
    S3BootScriptSaveMemWrite(
      EfiBootScriptWidthUint32,
      (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_PCS),
      1,
      (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_PCS)
      );
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_SDIO4;
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (SccPciMmBase) != 0xFFFFFFFF) {

      // Override the Capability Register
      ConfigureEMMC45(PchPlatformPolicy, SccPciMmBase);

      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_SCC_SDIO_BAR_ALIGNMENT,
                      V_PCH_SCC_SDIO_BAR_SIZE,
                      &SccMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD), (UINT32) ~(B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((SccMmioBase0 & B_PCH_SCC_SDIO_BAR_BA) == SccMmioBase0) && (SccMmioBase0 != 0));
        MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR), (UINT32) (SccMmioBase0 & B_PCH_SCC_SDIO_BAR_BA));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD), (UINT32) (B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) SccMmioBase0) != 0xFFFFFFFF);
        ///
        /// Set Maximum Timeout to 0x0E
        ///
        MmioWrite8 ((UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL), (UINT8) 0x0E);
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint8,
          (UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL),
          1,
          (VOID *) (UINTN) (SccMmioBase0 + R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL)
          );
        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD), (UINT32) ~(B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR), (UINT32) (0x00));
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR),
          1,
          (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)
          );
        gDS->FreeMemorySpace (SccMmioBase0, (UINT64) V_PCH_SCC_SDIO_BAR_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "SCC eMMC 4.5 not present, skipping.\n"));
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_SDIO4;
    }
  }


  DEBUG ((EFI_D_INFO, "ConfigureScc() End\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
ConfigureSccAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  )
/**

  @brief
  Hide PCI config space of SCC devices and do any final initialization.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINTN       SccPciMmBase;
  UINT32      SccMmioBase0;
  UINT32      SccMmioBase1;
  UINT32      Buffer32;
  EFI_STATUS  AcpiTablePresent;

  DEBUG ((EFI_D_INFO, "ConfigureSccAtBoot() Start\n"));

  ///
  /// Initialize Variables
  ///
  SccPciMmBase     = 0;
  SccMmioBase0     = 0;
  SccMmioBase1     = 0;
  Buffer32         = 0;
  AcpiTablePresent = EFI_NOT_FOUND;
  ///
  /// Locate ACPI table
  ///
  AcpiTablePresent = InitializePchAslUpdateLib ();
  ///
  /// Update SCC devices ACPI variables
  ///
  if (!EFI_ERROR (AcpiTablePresent)) {

    ///
    /// SCC SDIO
    ///
    if ((PchPlatformPolicy->SccConfig->SdioEnabled == 1) && (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled == 0)) {
      DEBUG ((EFI_D_INFO, "Switching SCC SDIO into ACPI Mode.\n"));
      SccPciMmBase = MmPciAddress (0,
                       DEFAULT_PCI_BUS_NUMBER_PCH,
                       PCI_DEVICE_NUMBER_PCH_SCC_SDIO_1,
                       PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,
                       0
                     );
      SccMmioBase0 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR)) & B_PCH_SCC_SDIO_BAR_BA;
      SccMmioBase1 = MmioRead32 ((UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_BAR1)) & B_PCH_SCC_SDIO_BAR1_BA;
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
        (UINT32) (B_PCH_SCC_SDIO_STSCMD_INTRDIS | B_PCH_SCC_SDIO_STSCMD_BME | B_PCH_SCC_SDIO_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite(
        EfiBootScriptWidthUint32,
        (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD),
        1,
        (VOID *) (UINTN) (SccPciMmBase + R_PCH_SCC_SDIO_STSCMD)
        );

      PchMsgBusAndThenOr32AddToS3Save (
        PCH_SCC_EP_PORT_ID,
        R_PCH_SCC_EP_PCICFGCTR3,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_SCC_EP_PCICFGCTR3_ACPI_INT_EN1 | B_PCH_SCC_EP_PCICFGCTR3_PCI_CFG_DIS1),
        PCH_SCC_EP_PRIVATE_READ_OPCODE,
        PCH_SCC_EP_PRIVATE_WRITE_OPCODE
        );
    }

//
// Do NOT transfer SD card to ACPI mode during ""ReadyToBoot"", it'll cause SD not boot under ACPI mode
// because setup menu is executed after ReadyToBoot event
//
// Transfer this during "ExitBootService" is recommended, so this source code moves into LpssDxe driver.
//
  }

  DEBUG ((EFI_D_INFO, "ConfigureSccAtBoot() End\n"));

  return EFI_SUCCESS;
}
