/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/**@file
  Gpio setting for multiplatform.

  This file includes package header files, library classes.

  Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
   This software and associated documentation (if any) is furnished
   under a license and may only be used or copied in accordance
   with the terms of the license. Except as permitted by such
   license, no part of this software or documentation may be
   reproduced, stored in a retrieval system, or transmitted in any
   form or by any means without the express written consent of
   Intel Corporation.
**/

#ifndef _BOARDGPIOS_H_
#define _BOARDGPIOS_H_

//////////////////////////////////////////////////////////////////////
#include <PiPei.h>
#include "PchAccess.h"
#include "PlatformBaseAddresses.h"
#include <../MultiPlatformLib.h>
#include <Library/IoLib.h>
#include <Library/HobLib.h>
#include <Guid/PlatformInfo.h>
#include <Ppi/Smbus.h>


// ***************************** Baley Bay Platform GPIO definition ***********************************
//
//  Netbook
//

#define BB_NB_GPIO_USE_SEL_VAL_0_31        0x00000000
#define BB_NB_GPIO_USE_SEL_VAL_32_63       0x00000000
#define BB_NB_GPIO_USE_SEL_VAL_64_70       0x00000000
#define BB_NB_GPIO_USE_SEL_VAL_SUS         0x00000000

#define BB_NB_GPIO_IO_SEL_VAL_0_31         0x00000000
#define BB_NB_GPIO_IO_SEL_VAL_32_63        0x00000000
#define BB_NB_GPIO_IO_SEL_VAL_64_70        0x00000000
#define BB_NB_GPIO_IO_SEL_VAL_SUS          0x00000000

#define BB_NB_GPIO_LVL_VAL_0_31            0x00000000
#define BB_NB_GPIO_LVL_VAL_32_63           0x00000000
#define BB_NB_GPIO_LVL_VAL_64_70           0x00000000
#define BB_NB_GPIO_LVL_VAL_SUS             0x00000000

#define BB_NB_GPIO_TPE_VAL_0_31            0x00000000
#define BB_NB_GPIO_TPE_VAL_SUS             0x00000000

#define BB_NB_GPIO_TNE_VAL_0_31            0x00000000
#define BB_NB_GPIO_TNE_VAL_SUS             0x00000000

#define BB_NB_GPIO_TS_VAL_0_31             0x00000000
#define BB_NB_GPIO_TS_VAL_SUS              0x00000000

CFIO_INIT_STRUCT	     mNB_BaleyBayCfioInitData = {
    BB_NB_GPIO_USE_SEL_VAL_0_31,
    BB_NB_GPIO_USE_SEL_VAL_32_63,
    BB_NB_GPIO_USE_SEL_VAL_64_70,
    BB_NB_GPIO_USE_SEL_VAL_SUS,

    BB_NB_GPIO_IO_SEL_VAL_0_31,
    BB_NB_GPIO_IO_SEL_VAL_32_63,
    BB_NB_GPIO_IO_SEL_VAL_64_70,
    BB_NB_GPIO_IO_SEL_VAL_SUS,

    BB_NB_GPIO_LVL_VAL_0_31,
    BB_NB_GPIO_LVL_VAL_32_63,
    BB_NB_GPIO_LVL_VAL_64_70,
    BB_NB_GPIO_LVL_VAL_SUS,

    BB_NB_GPIO_TPE_VAL_0_31,
    BB_NB_GPIO_TPE_VAL_SUS,
    BB_NB_GPIO_TNE_VAL_0_31,
    BB_NB_GPIO_TNE_VAL_SUS,

    BB_NB_GPIO_TS_VAL_0_31,
    BB_NB_GPIO_TS_VAL_SUS
};

// Note: Comment the GPIO definition if want to skip the specific PIN init.

// *********************************************** For Bayley Bay FAB3 Board **************************************************************
GPIO_CONF_PAD_INIT mNB_BB_FAB3_GpioInitData_NC[] = {
//              Pad Name          GPIO Number     Used As   GPO Default   Function#     INT Capable   Interrupt Type   PULL H/L    MMIO Offset
    GPIO_INIT_ITEM("HV_DDI0_HPD       GPIONC_0 "     ,Native   ,NA           ,F2           ,             ,                ,20K_H      ,0x13),
    GPIO_INIT_ITEM("HV_DDI0_DDC_SDA   GPIONC_1 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x12),
    GPIO_INIT_ITEM("HV_DDI0_DDC_SCL   GPIONC_2 "     ,Native   ,NA           ,F2           ,             ,                ,20K_H      ,0x11),
    GPIO_INIT_ITEM("PANEL0_VDDEN      GPIONC_3 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x14),
    GPIO_INIT_ITEM("PANEL0_BKLTEN     GPIONC_4 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x15),
    GPIO_INIT_ITEM("PANEL0_BKLTCTL    GPIONC_5 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x16),
    GPIO_INIT_ITEM("HV_DDI1_HPD       GPIONC_6 "     ,Native   ,NA           ,F2           ,             ,                ,20K_H      ,0x18),
    GPIO_INIT_ITEM("HV_DDI1_DDC_SDA   GPIONC_7 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x19),
    GPIO_INIT_ITEM("HV_DDI1_DDC_SCL   GPIONC_8 "     ,Native   ,NA           ,F2           ,             ,                ,20K_H      ,0x17),
    GPIO_INIT_ITEM("PANEL1_VDDEN      GPIONC_9 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x10),
    GPIO_INIT_ITEM("PANEL1_BKLTEN     GPIONC_10"     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x0e),
    GPIO_INIT_ITEM("PANEL1_BKLTCTL    GPIONC_11"     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x0f),
    GPIO_INIT_ITEM("GP_INTD_DSI_TE1   GPIONC_12"     ,GPI      ,NA           ,F0           ,YES          ,                ,20K_L      ,0x0c),
    GPIO_INIT_ITEM("HV_DDI2_DDC_SDA   GPIONC_13"     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x1a),
    GPIO_INIT_ITEM("HV_DDI2_DDC_SCL   GPIONC_14"     ,GPI      ,NA           ,F0           ,             ,                ,20K_H      ,0x1b),
    GPIO_INIT_ITEM("GP_CAMERASB00     GPIONC_15"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x01),
    GPIO_INIT_ITEM("GP_CAMERASB01     GPIONC_16"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x04),
    GPIO_INIT_ITEM("GP_CAMERASB02     GPIONC_17"     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x08),
    GPIO_INIT_ITEM("GP_CAMERASB03     GPIONC_18"     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x0b),
    GPIO_INIT_ITEM("GP_CAMERASB04     GPIONC_19"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x00),
    GPIO_INIT_ITEM("GP_CAMERASB05     GPIONC_20"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x03),
    GPIO_INIT_ITEM("GP_CAMERASB06     GPIONC_21"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x06),
    GPIO_INIT_ITEM("GP_CAMERASB07     GPIONC_22"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x0a),
    GPIO_INIT_ITEM("GP_CAMERASB08     GPIONC_23"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x0d),
    GPIO_INIT_ITEM("GP_CAMERASB09     GPIONC_24"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x02),
    GPIO_INIT_ITEM("GP_CAMERASB10     GPIONC_25"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x05),
    GPIO_INIT_ITEM("GP_CAMERASB11     GPIONC_26"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x09),
};

GPIO_CONF_PAD_INIT mNB_BB_FAB3_GpioInitData_SC[] = {
//              Pad Name          GPIO Number     Used As   GPO Default   Function#     INT Capable   Interrupt Type   PULL H/L    MMIO Offset
    GPIO_INIT_ITEM("SATA_GP0          GPIOC_0  "     ,GPI      ,NA           ,F0           ,YES          ,Edge_Both       ,20K_L      ,0x55),
    GPIO_INIT_ITEM("SATA_GP1          GPIOC_1  "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x59),
    GPIO_INIT_ITEM("SATA_LEDN         GPIOC_2  "     ,Native   ,NA           ,F1           ,             ,Edge_Both       ,20K_H      ,0x5d),
    GPIO_INIT_ITEM("PCIE_CLKREQ0B     GPIOC_3  "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x60),
    GPIO_INIT_ITEM("PCIE_CLKREQ1B     GPIOC_4  "     ,Native   ,NA           ,F1           ,             ,Edge_Both       ,20K_H      ,0x63),
    GPIO_INIT_ITEM("PCIE_CLKREQ2B     GPIOC_5  "     ,Native   ,NA           ,F1           ,             ,Edge_Both       ,20K_H      ,0x66),
    GPIO_INIT_ITEM("PCIE_CLKREQ3B     GPIOC_6  "     ,Native   ,NA           ,F1           ,             ,Edge_Both       ,20K_H      ,0x62),
    GPIO_INIT_ITEM("PCIE_CLKREQ4B     GPIOC_7  "     ,Native   ,NA           ,F2           ,YES          ,Edge_Both       ,20K_H      ,0x65),
    GPIO_INIT_ITEM("HDA_RSTB          GPIOC_8  "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x22),
    GPIO_INIT_ITEM("HDA_SYNC          GPIOC_9  "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x25),
    GPIO_INIT_ITEM("HDA_CLK           GPIOC_10 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x24),
    GPIO_INIT_ITEM("HDA_SDO           GPIOC_11 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x26),
    GPIO_INIT_ITEM("HDA_SDI0          GPIOC_12 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x27),
    GPIO_INIT_ITEM("HDA_SDI1          GPIOC_13 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x23),
    GPIO_INIT_ITEM("HDA_DOCKRSTB      GPIOC_14 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x28),
    GPIO_INIT_ITEM("HDA_DOCKENB       GPIOC_15 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x54),
    GPIO_INIT_ITEM("SDMMC1_CLK        GPIOC_16 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x3e),
    GPIO_INIT_ITEM("SDMMC1_D0         GPIOC_17 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x3d),
    GPIO_INIT_ITEM("SDMMC1_D1         GPIOC_18 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x40),
    GPIO_INIT_ITEM("SDMMC1_D2         GPIOC_19 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x3b),
    GPIO_INIT_ITEM("SDMMC1_D3_CD_B    GPIOC_20 "     ,Native   ,NA           ,F1           ,YES          ,                ,20K_H      ,0x36),
    GPIO_INIT_ITEM("MMC1_D4_SD_WE     GPIOC_21 "     ,Native   ,NA           ,F1           ,YES          ,                ,20K_H      ,0x38),
    GPIO_INIT_ITEM("MMC1_D5           GPIOC_22 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x3c),
    GPIO_INIT_ITEM("MMC1_D6           GPIOC_23 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x37),
    GPIO_INIT_ITEM("MMC1_D7           GPIOC_24 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x3f),
    GPIO_INIT_ITEM("SDMMC1_CMD        GPIOC_25 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x39),
    GPIO_INIT_ITEM("MMC1_RESET_B      GPIOC_26 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x33),
    GPIO_INIT_ITEM("SDMMC2_CLK        GPIOC_27 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x32),
    GPIO_INIT_ITEM("SDMMC2_D0         GPIOC_28 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x35),
    GPIO_INIT_ITEM("SDMMC2_D1         GPIOC_29 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x2f),
    GPIO_INIT_ITEM("SDMMC2_D2         GPIOC_30 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x34),
    GPIO_INIT_ITEM("SDMMC2_D3_CD_B    GPIOC_31 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x31),
    GPIO_INIT_ITEM("SDMMC2_CMD        GPIOC_32 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x30),
    GPIO_INIT_ITEM("SDMMC3_CLK        GPIOC_33 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x2b),
    GPIO_INIT_ITEM("SDMMC3_D0         GPIOC_34 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x2e),
    GPIO_INIT_ITEM("SDMMC3_D1         GPIOC_35 "     ,Native   ,NA           ,F1           ,YES          ,                ,20K_H      ,0x29),
    GPIO_INIT_ITEM("SDMMC3_D2         GPIOC_36 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x2d),
    GPIO_INIT_ITEM("SDMMC3_D3         GPIOC_37 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x2a),
    GPIO_INIT_ITEM("SDMMC3_CD_B       GPIOC_38 "     ,Native   ,NA           ,F1           ,YES          ,                ,20K_H      ,0x3a),
    GPIO_INIT_ITEM("SDMMC3_CMD        GPIOC_39 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x2c),
    GPIO_INIT_ITEM("SDMMC3_1P8_EN     GPIOC_40 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x5f),
    GPIO_INIT_ITEM("SDMMC3_PWR_EN_B   GPIOC_41 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x69),
    GPIO_INIT_ITEM("LPC_AD0           GPIOC_42 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x46),
    GPIO_INIT_ITEM("LPC_AD1           GPIOC_43 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x44),
    GPIO_INIT_ITEM("LPC_AD2           GPIOC_44 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x43),
    GPIO_INIT_ITEM("LPC_AD3           GPIOC_45 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x42),
    GPIO_INIT_ITEM("LPC_FRAMEB        GPIOC_46 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x45),
    GPIO_INIT_ITEM("LPC_CLKOUT0       GPIOC_47 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x47),
    GPIO_INIT_ITEM("LPC_CLKOUT1       GPIOC_48 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x41),
    GPIO_INIT_ITEM("LPC_CLKRUNB       GPIOC_49 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x48),
    GPIO_INIT_ITEM("ILB_SERIRQ        GPIOC_50 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x56),
    GPIO_INIT_ITEM("SMB_DATA          GPIOC_51 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x5a),
    GPIO_INIT_ITEM("SMB_CLK           GPIOC_52 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x58),
    GPIO_INIT_ITEM("SMB_ALERTB        GPIOC_53 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x5c),
    GPIO_INIT_ITEM("SPKR              GPIOC_54 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x67),
    GPIO_INIT_ITEM("MHSI_ACDATA       GPIOC_55 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x4d),
    GPIO_INIT_ITEM("MHSI_ACFLAG       GPIOC_56 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_H      ,0x4f),
    GPIO_INIT_ITEM("MHSI_ACREADY      GPIOC_57 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x53),
    GPIO_INIT_ITEM("MHSI_ACWAKE       GPIOC_58 "     ,GPI      ,NA           ,F0           ,YES          ,                ,20K_L      ,0x4e),
    GPIO_INIT_ITEM("MHSI_CADATA       GPIOC_59 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x51),
    GPIO_INIT_ITEM("MHSI_CAFLAG       GPIOC_60 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_H      ,0x50),
    GPIO_INIT_ITEM("MHSI_CAREADY      GPIOC_61 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x52),
    GPIO_INIT_ITEM("GP_SSP_2_CLK      GPIOC_62 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x0d),
    GPIO_INIT_ITEM("GP_SSP_2_FS       GPIOC_63 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x0c),
    GPIO_INIT_ITEM("GP_SSP_2_RXD      GPIOC_64 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x0f),
    GPIO_INIT_ITEM("GP_SSP_2_TXD      GPIOC_65 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x0e),
    GPIO_INIT_ITEM("SPI1_CS0_B        GPIOC_66 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x11),
    GPIO_INIT_ITEM("SPI1_MISO         GPIOC_67 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x12),
    GPIO_INIT_ITEM("SPI1_MOSI         GPIOC_68 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x13),
    GPIO_INIT_ITEM("SPI1_CLK          GPIOC_69 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x10),
    GPIO_INIT_ITEM("UART1_RXD         GPIOC_70 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x02),
    GPIO_INIT_ITEM("UART1_TXD         GPIOC_71 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x01),
    GPIO_INIT_ITEM("UART1_RTS_B       GPIOC_72 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x00),
    GPIO_INIT_ITEM("UART1_CTS_B       GPIOC_73 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x04),
    GPIO_INIT_ITEM("UART2_RXD         GPIOC_74 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x06),
    GPIO_INIT_ITEM("UART2_TXD         GPIOC_75 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x07),
    GPIO_INIT_ITEM("UART2_RTS_B       GPIOC_76 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x09),
    GPIO_INIT_ITEM("UART2_CTS_B       GPIOC_77 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x08),
    GPIO_INIT_ITEM("I2C0_SDA          GPIOC_78 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x21),
    GPIO_INIT_ITEM("I2C0_SCL          GPIOC_79 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x20),
    GPIO_INIT_ITEM("I2C1_SDA          GPIOC_80 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x1f),
    GPIO_INIT_ITEM("I2C1_SCL          GPIOC_81 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x1e),
    GPIO_INIT_ITEM("I2C2_SDA          GPIOC_82 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x1d),
    GPIO_INIT_ITEM("I2C2_SCL          GPIOC_83 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x1b),
    GPIO_INIT_ITEM("I2C3_SDA          GPIOC_84 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x19),
    GPIO_INIT_ITEM("I2C3_SCL          GPIOC_85 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x1c),
    GPIO_INIT_ITEM("I2C4_SDA          GPIOC_86 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x1a),
    GPIO_INIT_ITEM("I2C4_SCL          GPIOC_87 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x17),
    GPIO_INIT_ITEM("I2C5_SDA          GPIOC_88 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x15),
    GPIO_INIT_ITEM("I2C5_SCL          GPIOC_89 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x14),
    GPIO_INIT_ITEM("I2C6_SDA          GPIOC_90 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x18),
    GPIO_INIT_ITEM("I2C6_SCL          GPIOC_91 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x16),
    GPIO_INIT_ITEM("I2C_NFC_SDA       GPIOC_92 "     ,Native   ,NA           ,F0           ,             ,                ,20K_H      ,0x05),
    GPIO_INIT_ITEM("I2C_NFC_SCL       GPIOC_93 "     ,Native   ,NA           ,F0           ,             ,                ,20K_H      ,0x03),
    GPIO_INIT_ITEM("PWM0              GPIOC_94 "     ,GPI      ,NA           ,F0           ,YES          ,                ,20K_L      ,0x0a),
    GPIO_INIT_ITEM("PWM1              GPIOC_95 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_H      ,0x0b),
    GPIO_INIT_ITEM("PLT_CLK0          GPIOC_96 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x6a),
    GPIO_INIT_ITEM("PLT_CLK1          GPIOC_97 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x57),
    GPIO_INIT_ITEM("PLT_CLK2          GPIOC_98 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x5b),
    GPIO_INIT_ITEM("PLT_CLK3          GPIOC_99 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x68),
    GPIO_INIT_ITEM("PLT_CLK4          GPIOC_100"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x61),
    GPIO_INIT_ITEM("PLT_CLK5          GPIOC_101"     ,Native   ,NA           ,F1           ,YES          ,                ,20K_L      ,0x64),
};

GPIO_CONF_PAD_INIT mNB_BB_FAB3_GpioInitData_SUS[] = {
//              Pad Name          GPIO Number     Used As   GPIO Default  Function#     INT Capable   Interrupt Type   PULL H/L    MMIO Offset
    GPIO_INIT_ITEM("GPIO_SUS0         GPIO_SUS0"     ,GPI      ,NA           ,F0           ,YES          ,Level_High      ,20K_H      ,0x1d),
    GPIO_INIT_ITEM("GPIO_SUS1         GPIO_SUS1"     ,Native   ,NA           ,F6           ,YES          ,                ,20K_L      ,0x21),
    GPIO_INIT_ITEM("GPIO_SUS2         GPIO_SUS2"     ,Native   ,NA           ,F6           ,YES          ,Edge_Low        ,20K_H      ,0x1e),
    GPIO_INIT_ITEM("GPIO_SUS3         GPIO_SUS3"     ,GPI      ,NA           ,F6           ,YES          ,                ,20K_H      ,0x1f),
    GPIO_INIT_ITEM("GPIO_SUS4         GPIO_SUS4"     ,GPI      ,NA           ,F0           ,YES          ,Edge_High       ,20K_L      ,0x20),
    GPIO_INIT_ITEM("GPIO_SUS5         GPIO_SUS5"     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x22),
    GPIO_INIT_ITEM("GPIO_SUS6         GPIO_SUS6"     ,GPI      ,NA           ,F0           ,YES          ,                ,20K_L      ,0x24),
    GPIO_INIT_ITEM("GPIO_SUS7         GPIO_SUS7"     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x23),
    GPIO_INIT_ITEM("SEC_GPIO_SUS8     GPIO_SUS8"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x26),
    GPIO_INIT_ITEM("SEC_GPIO_SUS9     GPIO_SUS9"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x25),
    GPIO_INIT_ITEM("SEC_GPIO_SUS10    GPIO_SUS10"    ,GPI      ,NA           ,F0           ,YES          ,Level_High      ,20K_H      ,0x12),
    GPIO_INIT_ITEM("SUSPWRDNACK       GPIOS_11 "     ,Native   ,NA           ,F0           ,             ,                ,20K_L      ,0x07),
    GPIO_INIT_ITEM("PMU_SUSCLK        GPIOS_12 "     ,Native   ,NA           ,F0           ,YES          ,                ,20K_L      ,0x0b),
    GPIO_INIT_ITEM("PMU_SLP_S0IX_B    GPIOS_13 "     ,Native   ,NA           ,F0           ,             ,                ,20K_L      ,0x14),
    GPIO_INIT_ITEM("PMU_SLP_LAN_B     GPIOS_14 "     ,GPO      ,NA           ,F1           ,             ,                ,20K_H      ,0x11),
    GPIO_INIT_ITEM("PMU_WAKE_B        GPIOS_15 "     ,Native   ,NA           ,F0           ,YES          ,Level_High      ,20K_H      ,0x01),
    GPIO_INIT_ITEM("PMU_PWRBTN_B      GPIOS_16 "     ,Native   ,NA           ,F0           ,YES          ,                ,20K_H      ,0x08),
    GPIO_INIT_ITEM("PMU_WAKE_LAN_B    GPIOS_17 "     ,GPI      ,NA           ,F1           ,YES          ,Level_High      ,20K_H      ,0x0a),
    GPIO_INIT_ITEM("SUS_STAT_B        GPIOS_18 "     ,Native   ,NA           ,F0           ,YES          ,                ,20K_L      ,0x13),
    GPIO_INIT_ITEM("USB_OC0_B         GPIOS_19 "     ,Native   ,NA           ,F0           ,YES          ,                ,20K_H      ,0x0c),
    GPIO_INIT_ITEM("USB_OC1_B         GPIOS_20 "     ,Native   ,NA           ,F0           ,             ,                ,20K_H      ,0x00),
    GPIO_INIT_ITEM("SPI_CS1_B         GPIOS_21 "     ,GPO      ,NA           ,F1           ,             ,                ,20K_L      ,0x02),
    GPIO_INIT_ITEM("GPIO_DFX0         GPIOS_22 "     ,GPO      ,NA           ,F0           ,YES          ,Level_Low       ,20K_H      ,0x17),
    GPIO_INIT_ITEM("GPIO_DFX1         GPIOS_23 "     ,GPIO     ,NA           ,F0           ,YES          ,Level_Low       ,20K_L      ,0x27),
    GPIO_INIT_ITEM("GPIO_DFX2         GPIOS_24 "     ,GPIO     ,NA           ,F0           ,YES          ,Level_Low       ,20K_L      ,0x1c),
    GPIO_INIT_ITEM("GPIO_DFX3         GPIOS_25 "     ,GPIO     ,NA           ,F0           ,YES          ,                ,20K_L      ,0x1b),
    GPIO_INIT_ITEM("GPIO_DFX4         GPIOS_26 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x16),
    GPIO_INIT_ITEM("GPIO_DFX5         GPIOS_27 "     ,GPI      ,NA           ,F0           ,YES          ,Edge_Both       ,20K_H      ,0x15),
    GPIO_INIT_ITEM("GPIO_DFX6         GPIOS_28 "     ,GPI      ,NA           ,F0           ,YES          ,Edge_Both       ,20K_H      ,0x18),
    GPIO_INIT_ITEM("GPIO_DFX7         GPIOS_29 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x19),
    GPIO_INIT_ITEM("GPIO_DFX8         GPIOS_30 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_H      ,0x1a),
    GPIO_INIT_ITEM("USB_ULPI_0_CLK    GPIOS_31 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x33),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA0  GPIOS_32 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_H      ,0x38),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA1  GPIOS_33 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x36),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA2  GPIOS_34 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x31),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA3  GPIOS_35 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x37),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA4  GPIOS_36 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x30),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA5  GPIOS_37 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_H      ,0x39),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA6  GPIOS_38 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x32),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA7  GPIOS_39 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x3a),
    GPIO_INIT_ITEM("USB_ULPI_0_DIR    GPIOS_40 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_H      ,0x34),
    GPIO_INIT_ITEM("USB_ULPI_0_NXT    GPIOS_41 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_H      ,0x35),
    GPIO_INIT_ITEM("USB_ULPI_0_STP    GPIOS_42 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_H      ,0x3b),
    GPIO_INIT_ITEM("USB_ULPI_0_REFCLK GPIOS_43 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x28),
};

// *********************************************** For Bayley Bay FAB2/FAB1 Boards **************************************************************
GPIO_CONF_PAD_INIT mNB_BB_FAB2_GpioInitData_NC[] = {
//              Pad Name          GPIO Number     Used As   GPO Default   Function#     INT Capable   Interrupt Type   PULL H/L    MMIO Offset
    GPIO_INIT_ITEM("HV_DDI0_HPD       GPIONC_0 "     ,Native   ,NA           ,F2           ,             ,                ,20K_H      ,0x13),
    GPIO_INIT_ITEM("HV_DDI0_DDC_SDA   GPIONC_1 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x12),
    GPIO_INIT_ITEM("HV_DDI0_DDC_SCL   GPIONC_2 "     ,Native   ,NA           ,F2           ,             ,                ,20K_H      ,0x11),
    GPIO_INIT_ITEM("PANEL0_VDDEN      GPIONC_3 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x14),
    GPIO_INIT_ITEM("PANEL0_BKLTEN     GPIONC_4 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x15),
    GPIO_INIT_ITEM("PANEL0_BKLTCTL    GPIONC_5 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x16),
    GPIO_INIT_ITEM("HV_DDI1_HPD       GPIONC_6 "     ,Native   ,NA           ,F2           ,             ,                ,20K_H      ,0x18),
    GPIO_INIT_ITEM("HV_DDI1_DDC_SDA   GPIONC_7 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x19),
    GPIO_INIT_ITEM("HV_DDI1_DDC_SCL   GPIONC_8 "     ,Native   ,NA           ,F2           ,             ,                ,20K_H      ,0x17),
    GPIO_INIT_ITEM("PANEL1_VDDEN      GPIONC_9 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x10),
    GPIO_INIT_ITEM("PANEL1_BKLTEN     GPIONC_10"     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x0e),
    GPIO_INIT_ITEM("PANEL1_BKLTCTL    GPIONC_11"     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x0f),
    GPIO_INIT_ITEM("GP_INTD_DSI_TE1   GPIONC_12"     ,GPI      ,NA           ,F0           ,YES          ,                ,20K_L      ,0x0c),
    GPIO_INIT_ITEM("HV_DDI2_DDC_SDA   GPIONC_13"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x1a),
    GPIO_INIT_ITEM("HV_DDI2_DDC_SCL   GPIONC_14"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x1b),
    GPIO_INIT_ITEM("GP_CAMERASB00     GPIONC_15"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x01),
    GPIO_INIT_ITEM("GP_CAMERASB01     GPIONC_16"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x04),
    GPIO_INIT_ITEM("GP_CAMERASB02     GPIONC_17"     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x08),
    GPIO_INIT_ITEM("GP_CAMERASB03     GPIONC_18"     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x0b),
    GPIO_INIT_ITEM("GP_CAMERASB04     GPIONC_19"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x00),
    GPIO_INIT_ITEM("GP_CAMERASB05     GPIONC_20"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x03),
    GPIO_INIT_ITEM("GP_CAMERASB06     GPIONC_21"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x06),
    GPIO_INIT_ITEM("GP_CAMERASB07     GPIONC_22"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x0a),
    GPIO_INIT_ITEM("GP_CAMERASB08     GPIONC_23"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x0d),
    GPIO_INIT_ITEM("GP_CAMERASB09     GPIONC_24"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x02),
    GPIO_INIT_ITEM("GP_CAMERASB10     GPIONC_25"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x05),
    GPIO_INIT_ITEM("GP_CAMERASB11     GPIONC_26"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x09),
};

GPIO_CONF_PAD_INIT mNB_BB_FAB2_GpioInitData_SC[] = {
//              Pad Name          GPIO Number     Used As   GPO Default   Function#     INT Capable   Interrupt Type   PULL H/L    MMIO Offset
    GPIO_INIT_ITEM("SATA_GP0          GPIOC_0  "     ,Native   ,NA           ,F1           ,YES          ,Edge_Both       ,20K_L      ,0x55),
    GPIO_INIT_ITEM("SATA_GP1          GPIOC_1  "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x59),
    GPIO_INIT_ITEM("SATA_LEDN         GPIOC_2  "     ,Native   ,NA           ,F1           ,             ,Edge_Both       ,20K_H      ,0x5d),
    GPIO_INIT_ITEM("PCIE_CLKREQ0B     GPIOC_3  "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x60),
    GPIO_INIT_ITEM("PCIE_CLKREQ1B     GPIOC_4  "     ,Native   ,NA           ,F1           ,             ,Edge_Both       ,20K_H      ,0x63),
    GPIO_INIT_ITEM("PCIE_CLKREQ2B     GPIOC_5  "     ,Native   ,NA           ,F1           ,             ,Edge_Both       ,20K_H      ,0x66),
    GPIO_INIT_ITEM("PCIE_CLKREQ3B     GPIOC_6  "     ,Native   ,NA           ,F1           ,             ,Edge_Both       ,20K_H      ,0x62),
    GPIO_INIT_ITEM("PCIE_CLKREQ4B     GPIOC_7  "     ,GPI      ,NA           ,F0           ,YES          ,Edge_Both       ,20K_H      ,0x65),
    GPIO_INIT_ITEM("HDA_RSTB          GPIOC_8  "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x22),
    GPIO_INIT_ITEM("HDA_SYNC          GPIOC_9  "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x25),
    GPIO_INIT_ITEM("HDA_CLK           GPIOC_10 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x24),
    GPIO_INIT_ITEM("HDA_SDO           GPIOC_11 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x26),
    GPIO_INIT_ITEM("HDA_SDI0          GPIOC_12 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x27),
    GPIO_INIT_ITEM("HDA_SDI1          GPIOC_13 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x23),
    GPIO_INIT_ITEM("HDA_DOCKRSTB      GPIOC_14 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x28),
    GPIO_INIT_ITEM("HDA_DOCKENB       GPIOC_15 "     ,Native   ,NA           ,F2           ,             ,                ,20K_L      ,0x54),
    GPIO_INIT_ITEM("SDMMC1_CLK        GPIOC_16 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x3e),
    GPIO_INIT_ITEM("SDMMC1_D0         GPIOC_17 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x3d),
    GPIO_INIT_ITEM("SDMMC1_D1         GPIOC_18 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x40),
    GPIO_INIT_ITEM("SDMMC1_D2         GPIOC_19 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x3b),
    GPIO_INIT_ITEM("SDMMC1_D3_CD_B    GPIOC_20 "     ,Native   ,NA           ,F1           ,YES          ,                ,20K_H      ,0x36),
    GPIO_INIT_ITEM("MMC1_D4_SD_WE     GPIOC_21 "     ,Native   ,NA           ,F1           ,YES          ,                ,20K_H      ,0x38),
    GPIO_INIT_ITEM("MMC1_D5           GPIOC_22 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x3c),
    GPIO_INIT_ITEM("MMC1_D6           GPIOC_23 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x37),
    GPIO_INIT_ITEM("MMC1_D7           GPIOC_24 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x3f),
    GPIO_INIT_ITEM("SDMMC1_CMD        GPIOC_25 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x39),
    GPIO_INIT_ITEM("MMC1_RESET_B      GPIOC_26 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x33),
    GPIO_INIT_ITEM("SDMMC2_CLK        GPIOC_27 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x32),
    GPIO_INIT_ITEM("SDMMC2_D0         GPIOC_28 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x35),
    GPIO_INIT_ITEM("SDMMC2_D1         GPIOC_29 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x2f),
    GPIO_INIT_ITEM("SDMMC2_D2         GPIOC_30 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x34),
    GPIO_INIT_ITEM("SDMMC2_D3_CD_B    GPIOC_31 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x31),
    GPIO_INIT_ITEM("SDMMC2_CMD        GPIOC_32 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x30),
    GPIO_INIT_ITEM("SDMMC3_CLK        GPIOC_33 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x2b),
    GPIO_INIT_ITEM("SDMMC3_D0         GPIOC_34 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x2e),
    GPIO_INIT_ITEM("SDMMC3_D1         GPIOC_35 "     ,Native   ,NA           ,F1           ,YES          ,                ,20K_H      ,0x29),
    GPIO_INIT_ITEM("SDMMC3_D2         GPIOC_36 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x2d),
    GPIO_INIT_ITEM("SDMMC3_D3         GPIOC_37 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x2a),
    GPIO_INIT_ITEM("SDMMC3_CD_B       GPIOC_38 "     ,Native   ,NA           ,F1           ,YES          ,                ,20K_H      ,0x3a),
    GPIO_INIT_ITEM("SDMMC3_CMD        GPIOC_39 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x2c),
    GPIO_INIT_ITEM("SDMMC3_1P8_EN     GPIOC_40 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x5f),
    GPIO_INIT_ITEM("SDMMC3_PWR_EN_B   GPIOC_41 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x69),
    GPIO_INIT_ITEM("LPC_AD0           GPIOC_42 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x46),
    GPIO_INIT_ITEM("LPC_AD1           GPIOC_43 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x44),
    GPIO_INIT_ITEM("LPC_AD2           GPIOC_44 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x43),
    GPIO_INIT_ITEM("LPC_AD3           GPIOC_45 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x42),
    GPIO_INIT_ITEM("LPC_FRAMEB        GPIOC_46 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x45),
    GPIO_INIT_ITEM("LPC_CLKOUT0       GPIOC_47 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x47),
    GPIO_INIT_ITEM("LPC_CLKOUT1       GPIOC_48 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x41),
    GPIO_INIT_ITEM("LPC_CLKRUNB       GPIOC_49 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x48),
    GPIO_INIT_ITEM("ILB_SERIRQ        GPIOC_50 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x56),
    GPIO_INIT_ITEM("SMB_DATA          GPIOC_51 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x5a),
    GPIO_INIT_ITEM("SMB_CLK           GPIOC_52 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x58),
    GPIO_INIT_ITEM("SMB_ALERTB        GPIOC_53 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x5c),
    GPIO_INIT_ITEM("SPKR              GPIOC_54 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x67),
    GPIO_INIT_ITEM("MHSI_ACDATA       GPIOC_55 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x4d),
    GPIO_INIT_ITEM("MHSI_ACFLAG       GPIOC_56 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_H      ,0x4f),
    GPIO_INIT_ITEM("MHSI_ACREADY      GPIOC_57 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x53),
    GPIO_INIT_ITEM("MHSI_ACWAKE       GPIOC_58 "     ,GPI      ,NA           ,F0           ,YES          ,                ,20K_L      ,0x4e),
    GPIO_INIT_ITEM("MHSI_CADATA       GPIOC_59 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x51),
    GPIO_INIT_ITEM("MHSI_CAFLAG       GPIOC_60 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x50),
    GPIO_INIT_ITEM("MHSI_CAREADY      GPIOC_61 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x52),
    GPIO_INIT_ITEM("GP_SSP_2_CLK      GPIOC_62 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x0d),
    GPIO_INIT_ITEM("GP_SSP_2_FS       GPIOC_63 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x0c),
    GPIO_INIT_ITEM("GP_SSP_2_RXD      GPIOC_64 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x0f),
    GPIO_INIT_ITEM("GP_SSP_2_TXD      GPIOC_65 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x0e),
    GPIO_INIT_ITEM("SPI1_CS0_B        GPIOC_66 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x11),
    GPIO_INIT_ITEM("SPI1_MISO         GPIOC_67 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x12),
    GPIO_INIT_ITEM("SPI1_MOSI         GPIOC_68 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x13),
    GPIO_INIT_ITEM("SPI1_CLK          GPIOC_69 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x10),
    GPIO_INIT_ITEM("UART1_RXD         GPIOC_70 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x02),
    GPIO_INIT_ITEM("UART1_TXD         GPIOC_71 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x01),
    GPIO_INIT_ITEM("UART1_RTS_B       GPIOC_72 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x00),
    GPIO_INIT_ITEM("UART1_CTS_B       GPIOC_73 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x04),
    GPIO_INIT_ITEM("UART2_RXD         GPIOC_74 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x06),
    GPIO_INIT_ITEM("UART2_TXD         GPIOC_75 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x07),
    GPIO_INIT_ITEM("UART2_RTS_B       GPIOC_76 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x09),
    GPIO_INIT_ITEM("UART2_CTS_B       GPIOC_77 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x08),
    GPIO_INIT_ITEM("I2C0_SDA          GPIOC_78 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x21),
    GPIO_INIT_ITEM("I2C0_SCL          GPIOC_79 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x20),
    GPIO_INIT_ITEM("I2C1_SDA          GPIOC_80 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x1f),
    GPIO_INIT_ITEM("I2C1_SCL          GPIOC_81 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x1e),
    GPIO_INIT_ITEM("I2C2_SDA          GPIOC_82 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x1d),
    GPIO_INIT_ITEM("I2C2_SCL          GPIOC_83 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x1b),
    GPIO_INIT_ITEM("I2C3_SDA          GPIOC_84 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x19),
    GPIO_INIT_ITEM("I2C3_SCL          GPIOC_85 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x1c),
    GPIO_INIT_ITEM("I2C4_SDA          GPIOC_86 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x1a),
    GPIO_INIT_ITEM("I2C4_SCL          GPIOC_87 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x17),
    GPIO_INIT_ITEM("I2C5_SDA          GPIOC_88 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x15),
    GPIO_INIT_ITEM("I2C5_SCL          GPIOC_89 "     ,Native   ,NA           ,F1           ,             ,                ,20K_H      ,0x14),
    GPIO_INIT_ITEM("I2C6_SDA          GPIOC_90 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_H      ,0x18),
    GPIO_INIT_ITEM("I2C6_SCL          GPIOC_91 "     ,Native   ,NA           ,F2           ,             ,                ,20K_H      ,0x16),
    GPIO_INIT_ITEM("I2C_NFC_SDA       GPIOC_92 "     ,Native   ,NA           ,F0           ,             ,                ,20K_H      ,0x05),
    GPIO_INIT_ITEM("I2C_NFC_SCL       GPIOC_93 "     ,Native   ,NA           ,F0           ,             ,                ,20K_H      ,0x03),
    GPIO_INIT_ITEM("PWM0              GPIOC_94 "     ,GPI      ,NA           ,F0           ,YES          ,                ,20K_L      ,0x0a),
    GPIO_INIT_ITEM("PWM1              GPIOC_95 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x0b),
    GPIO_INIT_ITEM("PLT_CLK0          GPIOC_96 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x6a),
    GPIO_INIT_ITEM("PLT_CLK1          GPIOC_97 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x57),
    GPIO_INIT_ITEM("PLT_CLK2          GPIOC_98 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x5b),
    GPIO_INIT_ITEM("PLT_CLK3          GPIOC_99 "     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x68),
    GPIO_INIT_ITEM("PLT_CLK4          GPIOC_100"     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x61),
    GPIO_INIT_ITEM("PLT_CLK5          GPIOC_101"     ,Native   ,NA           ,F1           ,YES          ,                ,20K_L      ,0x64),
};

GPIO_CONF_PAD_INIT mNB_BB_FAB2_GpioInitData_SUS[] = {
//              Pad Name          GPIO Number     Used As   GPIO Default  Function#     INT Capable   Interrupt Type   PULL H/L    MMIO Offset
    GPIO_INIT_ITEM("GPIO_SUS0         GPIO_SUS0"     ,GPI      ,NA           ,F0           ,YES          ,Level_High      ,20K_H      ,0x1d),
    GPIO_INIT_ITEM("GPIO_SUS1         GPIO_SUS1"     ,Native   ,NA           ,F6           ,YES          ,                ,20K_L      ,0x21),
    GPIO_INIT_ITEM("GPIO_SUS2         GPIO_SUS2"     ,Native   ,NA           ,F6           ,YES          ,Edge_Low        ,20K_H      ,0x1e),
    GPIO_INIT_ITEM("GPIO_SUS3         GPIO_SUS3"     ,GPI      ,NA           ,F0           ,YES          ,                ,20K_H      ,0x1f),
    GPIO_INIT_ITEM("GPIO_SUS4         GPIO_SUS4"     ,GPI      ,NA           ,F0           ,YES          ,Edge_High       ,20K_L      ,0x20),
    GPIO_INIT_ITEM("GPIO_SUS5         GPIO_SUS5"     ,Native   ,NA           ,F1           ,             ,                ,20K_L      ,0x22),
    GPIO_INIT_ITEM("GPIO_SUS6         GPIO_SUS6"     ,GPI      ,NA           ,F0           ,YES          ,                ,20K_L      ,0x24),
    GPIO_INIT_ITEM("GPIO_SUS7         GPIO_SUS7"     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x23),
    GPIO_INIT_ITEM("SEC_GPIO_SUS8     GPIO_SUS8"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x26),
    GPIO_INIT_ITEM("SEC_GPIO_SUS9     GPIO_SUS9"     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x25),
    GPIO_INIT_ITEM("SEC_GPIO_SUS10    GPIO_SUS10"    ,GPI      ,NA           ,F0           ,YES          ,Level_High      ,20K_H      ,0x12),
    GPIO_INIT_ITEM("SUSPWRDNACK       GPIOS_11 "     ,Native   ,NA           ,F0           ,             ,                ,20K_L      ,0x07),
    GPIO_INIT_ITEM("PMU_SUSCLK        GPIOS_12 "     ,Native   ,NA           ,F0           ,YES          ,                ,20K_L      ,0x0b),
    GPIO_INIT_ITEM("PMU_SLP_S0IX_B    GPIOS_13 "     ,Native   ,NA           ,F0           ,             ,                ,20K_L      ,0x14),
    GPIO_INIT_ITEM("PMU_SLP_LAN_B     GPIOS_14 "     ,GPO      ,NA           ,F1           ,             ,                ,20K_H      ,0x11),
    GPIO_INIT_ITEM("PMU_WAKE_B        GPIOS_15 "     ,Native   ,NA           ,F0           ,YES          ,Level_High      ,20K_H      ,0x01),
    GPIO_INIT_ITEM("PMU_PWRBTN_B      GPIOS_16 "     ,Native   ,NA           ,F0           ,YES          ,                ,20K_H      ,0x08),
    GPIO_INIT_ITEM("PMU_WAKE_LAN_B    GPIOS_17 "     ,Native   ,NA           ,F0           ,YES          ,Level_High      ,20K_H      ,0x0a),
    GPIO_INIT_ITEM("SUS_STAT_B        GPIOS_18 "     ,Native   ,NA           ,F0           ,YES          ,                ,20K_L      ,0x13),
    GPIO_INIT_ITEM("USB_OC0_B         GPIOS_19 "     ,Native   ,NA           ,F0           ,YES          ,                ,20K_H      ,0x0c),
    GPIO_INIT_ITEM("USB_OC1_B         GPIOS_20 "     ,Native   ,NA           ,F0           ,             ,                ,20K_H      ,0x00),
    GPIO_INIT_ITEM("SPI_CS1_B         GPIOS_21 "     ,GPO      ,NA           ,F1           ,             ,                ,20K_L      ,0x02),
    GPIO_INIT_ITEM("GPIO_DFX0         GPIOS_22 "     ,GPO      ,NA           ,F0           ,YES          ,Level_Low       ,20K_H      ,0x17),
    GPIO_INIT_ITEM("GPIO_DFX1         GPIOS_23 "     ,GPIO     ,NA           ,F0           ,YES          ,Level_Low       ,20K_L      ,0x27),
    GPIO_INIT_ITEM("GPIO_DFX2         GPIOS_24 "     ,GPIO     ,NA           ,F0           ,YES          ,Level_Low       ,20K_L      ,0x1c),
    GPIO_INIT_ITEM("GPIO_DFX3         GPIOS_25 "     ,GPIO     ,NA           ,F0           ,YES          ,                ,20K_L      ,0x1b),
    GPIO_INIT_ITEM("GPIO_DFX4         GPIOS_26 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x16),
    GPIO_INIT_ITEM("GPIO_DFX5         GPIOS_27 "     ,GPI      ,NA           ,F0           ,YES          ,Edge_Both       ,20K_H      ,0x15),
    GPIO_INIT_ITEM("GPIO_DFX6         GPIOS_28 "     ,GPI      ,NA           ,F0           ,YES          ,Edge_Both       ,20K_H      ,0x18),
    GPIO_INIT_ITEM("GPIO_DFX7         GPIOS_29 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x19),
    GPIO_INIT_ITEM("GPIO_DFX8         GPIOS_30 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_H      ,0x1a),
    GPIO_INIT_ITEM("USB_ULPI_0_CLK    GPIOS_31 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x33),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA0  GPIOS_32 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x38),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA1  GPIOS_33 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_L      ,0x36),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA2  GPIOS_34 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x31),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA3  GPIOS_35 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x37),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA4  GPIOS_36 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x30),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA5  GPIOS_37 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x39),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA6  GPIOS_38 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x32),
    GPIO_INIT_ITEM("USB_ULPI_0_DATA7  GPIOS_39 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x3a),
    GPIO_INIT_ITEM("USB_ULPI_0_DIR    GPIOS_40 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_H      ,0x34),
    GPIO_INIT_ITEM("USB_ULPI_0_NXT    GPIOS_41 "     ,GPO      ,NA           ,F0           ,             ,                ,20K_H      ,0x35),
    GPIO_INIT_ITEM("USB_ULPI_0_STP    GPIOS_42 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_H      ,0x3b),
    GPIO_INIT_ITEM("USB_ULPI_0_REFCLK GPIOS_43 "     ,GPI      ,NA           ,F0           ,             ,                ,20K_L      ,0x28),
};
//(CSP20130313C+)<<

#endif
