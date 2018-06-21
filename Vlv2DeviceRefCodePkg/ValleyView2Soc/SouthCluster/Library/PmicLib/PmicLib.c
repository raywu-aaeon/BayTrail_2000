/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

/** @file
  Library for accessing PMIC registers.

**/
#ifdef ECP_FLAG
#include <EdkIIGlueDxe.h>
#include "I2CLib.h"
#include "PmicLib.h"
#else
#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/I2CLib.h>
#include <Library/PmicLib.h>
#include <Library/TimerLib.h>

#endif
#include "PmicReg.h"

#include <Guid/PlatformInfo.h>
/**
  Reads an 8-bit PMIC register.

  Reads the 8-bit PMIC register specified by Register.
  The 8-bit read value is returned.

  @param  Register  The PMIC register to read.

  @return The value read.

**/
UINT8
EFIAPI
PmicRead8 (
  IN      UINT8                     Register
  )
{
  UINT8 value=0;
  EFI_STATUS rc=0;

  rc = ByteReadI2C(PMIC_I2C_BUSNO, PMIC_PAGE_1_I2C_ADDR, Register, 1, &value);
  if (EFI_SUCCESS != rc)  {
    DEBUG ((DEBUG_INFO, "PmicRead8 failed :0x%x\n", rc));
    return 0xff;
  }
  return value;
}


/**
  Writes an 8-bit PMIC register.

  Writes the 8-bit PMIC register specified by Register with the value specified by Value
  and returns Value.

  If 8-bit PMIC register operations are not supported, then ASSERT().

  @param  Register  The PMIC register to write.
  @param  Value The value to write to the PMIC register.

  @return The value written the PMIC register.

**/
EFI_STATUS
EFIAPI
PmicWrite8 (
  IN      UINT8                     Register,
  IN      UINT8                     Value
  )
{
  //UINT8 value=0;
  EFI_STATUS rc=0;

  DEBUG ((DEBUG_INFO, "PmicWrite8:---------------0x%x,0x%x\n", &Value,Value));
  rc = ByteWriteI2C(PMIC_I2C_BUSNO, PMIC_PAGE_1_I2C_ADDR, Register, 1, &Value);
  if (EFI_SUCCESS != rc)  {
    DEBUG ((DEBUG_INFO, "PmicWrite8 failed :0x%x\n", rc));
    return rc;
  }
  return EFI_SUCCESS;
}


UINT8
EFIAPI
PmicRead8_page0 (
  IN      UINT8                     Register
  )
{
  UINT8 value=0;
  EFI_STATUS rc=0;

  rc = ByteReadI2C(PMIC_I2C_BUSNO, PMIC_PAGE_0_I2C_ADDR, Register, 1, &value);
  if (EFI_SUCCESS != rc)  {
    DEBUG ((DEBUG_INFO, "PmicRead8_page0 failed :0x%x\n", rc));
    return 0xff;
  }
  return value;
}

EFI_STATUS
EFIAPI
PmicWrite8_page0 (
  IN      UINT8                     Register,
  IN      UINT8                     Value
  )
{
  //UINT8 value=0;
  EFI_STATUS rc=0;

  DEBUG ((DEBUG_INFO, "PmicWrite8_page0:---------------0x%x,0x%x\n", &Value,Value));
  rc = ByteWriteI2C(PMIC_I2C_BUSNO, PMIC_PAGE_0_I2C_ADDR, Register, 1, &Value);
  if (EFI_SUCCESS != rc)  {
    DEBUG ((DEBUG_INFO, "PmicWrite8_page0 failed :0x%x\n", rc));
    return rc;
  }
  return EFI_SUCCESS;
}

RegInit_st g_Diaglog_ThermRegInit[]= {
  /*
  ENABLE:0x3f
  All SYSTHERM_HIGH0 : 0x43
  All SYSTERM_HIGH1:    0x41
  all SYSTHERM_LOW1:   0xfa
  */
  {PMIC_REG_SYS0_THRMALRT0_H, REG_OVERRIDE, 0xFF, 0x4C},
  {PMIC_REG_SYS0_THRMALRT0_L, REG_OVERRIDE, 0xFF, 0x0},
  {PMIC_REG_SYS0_THRMALRT1_H, REG_OVERRIDE, 0xFF, 0x4C},
  {PMIC_REG_SYS0_THRMALRT1_L, REG_OVERRIDE, 0xFF, 0x00},

  {PMIC_REG_SYS1_THRMALRT0_H, REG_OVERRIDE, 0xFF, 0x4C},
  {PMIC_REG_SYS1_THRMALRT0_L, REG_OVERRIDE, 0xFF, 0x0},
  {PMIC_REG_SYS1_THRMALRT1_H, REG_OVERRIDE, 0xFF, 0x4C},
  {PMIC_REG_SYS1_THRMALRT1_L, REG_OVERRIDE, 0xFF, 0x0},

  {PMIC_REG_SYS2_THRMALRT0_H, REG_OVERRIDE, 0xFF, 0xCD}, // A0_P/A0_Alert/67 C
  {PMIC_REG_SYS2_THRMALRT0_L, REG_OVERRIDE, 0xFF, 0x08},
  {PMIC_REG_SYS2_THRMALRT1_H, REG_OVERRIDE, 0xFF, 0x4C},
  {PMIC_REG_SYS2_THRMALRT1_L, REG_OVERRIDE, 0xFF, 0x0},

  {PMIC_REG_TS_ENABLE,        REG_OVERRIDE,    0xFF, 0x3f},    //enable all thermsistors
  //PMIC GPIO interrupt mask
  //{PMIC_REG_MGPIO0IRQS0,       REG_OVERRIDE, 0xFF, 0x7c},    //bit 0 1 7 vol-up vol-down rotate
  //{PMIC_REG_MGPIO1IRQS0,       REG_OVERRIDE, 0xFF, 0xfe},    //bit 0 home
  //{PMIC_REG_MGPIO0IRQSX,       REG_OVERRIDE, 0x0, 0x0},           //by default
  //{PMIC_REG_MGPIO1IRQSX,       REG_OVERRIDE, 0xFF, 0xfe},    //bit 0 only for home key wakeup.

};
RegInit_st g_Rohm_ThermRegInit[]= {
  {PMIC_REG_SYS0_THRMALRT0_H, REG_OVERRIDE, 0xFF, 0x4C},  // the values are from Peter on B0 PO
  {PMIC_REG_SYS0_THRMALRT0_L, REG_OVERRIDE, 0xFF, 0x00},  // enabled but threshold is 0
  {PMIC_REG_SYS0_THRMALRT1_H, REG_OVERRIDE, 0xFF, 0x4C},  //
  {PMIC_REG_SYS0_THRMALRT1_L, REG_OVERRIDE, 0xFF, 0x00},
  {PMIC_REG_SYS0_THRMCRIT,    REG_OVERRIDE, 0xFF, 0x46},  // 90C

  {PMIC_REG_SYS1_THRMALRT0_H, REG_OVERRIDE, 0xFF, 0x4C},
  {PMIC_REG_SYS1_THRMALRT0_L, REG_OVERRIDE, 0xFF, 0x00},
  {PMIC_REG_SYS1_THRMALRT1_H, REG_OVERRIDE, 0xFF, 0x4C},
  {PMIC_REG_SYS1_THRMALRT1_L, REG_OVERRIDE, 0xFF, 0x00},
  {PMIC_REG_SYS1_THRMCRIT,    REG_OVERRIDE, 0xFF, 0x46},

  {PMIC_REG_SYS2_THRMALRT0_H, REG_OVERRIDE, 0xFF, 0xCD},   // A0_P/A0_Alert/67 C
  {PMIC_REG_SYS2_THRMALRT0_L, REG_OVERRIDE, 0xFF, 0x08},
  {PMIC_REG_SYS2_THRMALRT1_H, REG_OVERRIDE, 0xFF, 0x4C},
  {PMIC_REG_SYS2_THRMALRT1_L, REG_OVERRIDE, 0xFF, 0x00},
  {PMIC_REG_SYS2_THRMCRIT,    REG_OVERRIDE, 0xFF, 0x46},

  {PMIC_REG_BAT0_THRMALRT0_H, REG_OVERRIDE, 0xFF, 0x3D},   // battery #0 alert0 50C   (disabled)
  {PMIC_REG_BAT0_THRMALRT0_L, REG_OVERRIDE, 0xFF, 0x97},
  {PMIC_REG_BAT0_THRMALRT1_H, REG_OVERRIDE, 0xFF, 0x3D},   // alert 1:45C
  {PMIC_REG_BAT0_THRMALRT1_L, REG_OVERRIDE, 0xFF, 0xCA},
  {PMIC_REG_BAT0_THRMCRIT_H,  REG_OVERRIDE, 0xFF, 0x6A},   // 75C
  {PMIC_REG_BAT0_THRMCRIT_L,  REG_OVERRIDE, 0xFF, 0xF0},   //-55C

  {PMIC_REG_BAT1_THRMALRT0_H, REG_OVERRIDE, 0xFF, 0x3D},   //disabled
  {PMIC_REG_BAT1_THRMALRT0_L, REG_OVERRIDE, 0xFF, 0x97},
  {PMIC_REG_BAT1_THRMALRT1_H, REG_OVERRIDE, 0xFF, 0x3D},
  {PMIC_REG_BAT1_THRMALRT1_L, REG_OVERRIDE, 0xFF, 0xCA},
  {PMIC_REG_BAT1_THRMCRIT_H,  REG_OVERRIDE, 0xFF, 0x6A},
  {PMIC_REG_BAT1_THRMCRIT_L,  REG_OVERRIDE, 0xFF, 0xF0},

  {PMIC_REG_PMIC_THRMALRT0_H, REG_OVERRIDE, 0xFF, 0xFE},   // 110C
  {PMIC_REG_PMIC_THRMALRT0_L, REG_OVERRIDE, 0xFF, 0x17},
  {PMIC_REG_PMIC_THRMALRT1_H, REG_OVERRIDE, 0xFF, 0x7E},   // 100C
  {PMIC_REG_PMIC_THRMALRT1_L, REG_OVERRIDE, 0xFF, 0x2A},
  {PMIC_REG_PMIC_THRMCRIT,    REG_OVERRIDE, 0xFF, 0xFF},   // 123C     overflow MSB1 bit in the design. so only value higher than 123 is acceptable.


  {PMIC_REG_TS_ENABLE,        REG_OVERRIDE, 0xFF, 0x27},    // 27  (enable all except BAT0 and BAT1)
  //{PMIC_REG_TS_ENABLE,        REG_CLEAR,    PMIC_MASK_ALL_EN, PMIC_MASK_ALL_EN},    //disable all thermsistor for PO
  //{PMIC_REG_TS__CRIT_ENABLE,  REG_CLEAR,    PMIC_MASK_ALL_EN, PMIC_MASK_ALL_EN},    //disable all cirtical thermal theshold for PO

  {PMIC_REG_THRMMONCTL0,      REG_OVERRIDE,  0xFF, 0xB},   //enable thermal automatic monitoring timer, 1s sample interval
  //{PMIC_REG_THRMMONCTL1,      REG_OVERRIDE, 0x0, 0x0},                             //not touch

  //{PMIC_REG_ALERT0LOCK,       REG_OVERRIDE, PMIC_MASK_A0LOCK, PMIC_MASK_A0LOCK},    //lock A0 l0ck as per PMIC spec suggested.

  //PMIC GPIO interrupt mask
  //{PMIC_REG_MGPIO0IRQS0,       REG_OVERRIDE, 0xFF, 0x7c},    //bit 0 1 7 vol-up vol-down rotate
  //{PMIC_REG_MGPIO1IRQS0,       REG_OVERRIDE, 0xFF, 0xfe},    //bit 0 home
  //{PMIC_REG_MGPIO0IRQSX,       REG_OVERRIDE, 0x0, 0x0},           //by default
  //{PMIC_REG_MGPIO1IRQSX,       REG_OVERRIDE, 0xFF, 0xfe},    //bit 0 only for home key wakeup.

};




/**
  Init Thermal theshold

  @param  Register  The PMIC register to write.
  @param  Value The value to write to the PMIC register.

  @return The value written the PMIC register.

**/
EFI_STATUS
EFIAPI
PmicBatchRegisterInit (
  IN      RegInit_st               *RegInit,
  IN      UINT32                    length
  )
{
  UINT32 index = 0;
  UINT8 value = 0;

  for (; index < length; index++) {
    //value = PmicRead8(RegInit[index].regoffset);

    if (0x0 == RegInit[index].mask)  continue;    //bypass register if mask is 0x0

    switch(RegInit[index].action) {
      case REG_OVERRIDE:
        value &= ~(RegInit[index].mask);
        value |= (RegInit[index].value & RegInit[index].mask);
        PmicWrite8(RegInit[index].regoffset, value);
        break;

      case REG_CLEAR:
        value &= ~(RegInit[index].mask);
        PmicWrite8(RegInit[index].regoffset, value);
        break;

      case REG_AND:
        value &= (RegInit[index].value & RegInit[index].mask);
        PmicWrite8(RegInit[index].regoffset, value);
        break;

      case REG_OR:
        value |= (RegInit[index].value & RegInit[index].mask);
        PmicWrite8(RegInit[index].regoffset, value);
        break;
    }
    DEBUG ((DEBUG_INFO, "PmicBatchRegisterInit ----------------end 0x%x:0x%x:0x%x\n", \
            RegInit[index].regoffset, value,PmicRead8(RegInit[index].regoffset)));

  }
  return EFI_SUCCESS;

}


EFI_STATUS
EFIAPI
PmicThermInit (void)
{
  UINT8 DevId=0, RevId=0;
  UINT32 length ;

  PmicGetDevID(&DevId, &RevId);

  if ((DevId == PMIC_DEV_DIALOG) || (DevId == PMIC_DEV_DIALOG_1))  {
    length = sizeof(g_Diaglog_ThermRegInit)/sizeof(RegInit_st);
    DEBUG ((DEBUG_INFO, "g_Diaglog_ThermRegInit\n"));
    return PmicBatchRegisterInit(g_Diaglog_ThermRegInit, length);
  }

  if (DevId == PMIC_DEV_ROHM) {
    length = sizeof(g_Rohm_ThermRegInit)/sizeof(RegInit_st);
    DEBUG ((DEBUG_INFO, "g_Rohm_ThermRegInit\n"));
    return PmicBatchRegisterInit(g_Rohm_ThermRegInit, length);

  }

  return EFI_SUCCESS;
}

GpioCfg_st g_GPIO_PR1_cfg[]= {
  {"GPIO0P0", PMIC_REG_GPIO0P0CTLO, PMIC_REG_GPIO0P0CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//"Volumn up" (debounce enable)
  {"GPIO0P1", PMIC_REG_GPIO0P1CTLO, PMIC_REG_GPIO0P1CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//"Volumn down" (debounce enable)
  {"GPIO1P3", PMIC_REG_GPIO1P3CTLO, PMIC_REG_GPIO1P3CTLI, DIR_OUTPUT, INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PU|PMIC_MASK_DOUT| PMIC_MASK_DIR),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//PR1 TOUCH'S powergating
  {"GPIO0P7", PMIC_REG_GPIO0P7CTLO, PMIC_REG_GPIO0P7CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },// rotate lock button (debounce enable/50K pullup)
  {"GPIO1P0", PMIC_REG_GPIO1P0CTLO, PMIC_REG_GPIO1P0CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PU|PMIC_MASK_ALTFUNCEN),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) }, //enable altfunc "UIBTN_B" home screen (debounce enable)
};

GpioCfg_st g_GPIO_FFRD8_cfg[]= {
  {"GPIO0P0", PMIC_REG_GPIO0P0CTLO, PMIC_REG_GPIO0P0CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//"Volumn up" (debounce enable)
  {"GPIO0P1", PMIC_REG_GPIO0P1CTLO, PMIC_REG_GPIO0P1CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//"Volumn down" (debounce enable)
  {"GPIO0P3", PMIC_REG_GPIO0P3CTLO, PMIC_REG_GPIO0P3CTLI, DIR_OUTPUT, INTERRUPT_DIS, (PMIC_MASK_RVAL_50K_PU|PMIC_MASK_DOUT| PMIC_MASK_DIR),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },// I2S_RESET_N_CONN (cmos/pull down)
  {"GPIO0P4", PMIC_REG_GPIO0P4CTLO, PMIC_REG_GPIO0P4CTLI, DIR_OUTPUT, INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PD|PMIC_MASK_DIR),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },// PMIC VIBE enable (pull down)
  {"GPIO0P7", PMIC_REG_GPIO0P7CTLO, PMIC_REG_GPIO0P7CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },// rotate lock button (debounce enable/50K pullup)
  {"GPIO1P0", PMIC_REG_GPIO1P0CTLO, PMIC_REG_GPIO1P0CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PU|PMIC_MASK_ALTFUNCEN),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) }, //enable altfunc "UIBTN_B" home screen (debounce enable)
  {"GPIO1P3", PMIC_REG_GPIO1P3CTLO, PMIC_REG_GPIO1P3CTLI, DIR_OUTPUT, INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PU|PMIC_MASK_DOUT| PMIC_MASK_DIR),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//PR1 TOUCH'S powergating
};

GpioCfg_st g_GPIO_FFRD8_PR1_cfg[]= {
  {"GPIO0P0", PMIC_REG_GPIO0P0CTLO, PMIC_REG_GPIO0P0CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//"Volumn up" (debounce enable)
  {"GPIO0P1", PMIC_REG_GPIO0P1CTLO, PMIC_REG_GPIO0P1CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//"Volumn down" (debounce enable)
  {"GPIO0P3", PMIC_REG_GPIO0P3CTLO, PMIC_REG_GPIO0P3CTLI, DIR_OUTPUT, INTERRUPT_DIS, (PMIC_MASK_RVAL_50K_PU|PMIC_MASK_DOUT| PMIC_MASK_DIR),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },// I2S_RESET_N_CONN (cmos/pull down)
  {"GPIO0P4", PMIC_REG_GPIO0P4CTLO, PMIC_REG_GPIO0P4CTLI, DIR_OUTPUT, INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PD|PMIC_MASK_DIR),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },// PMIC VIBE enable (pull down)
  {"GPIO0P7", PMIC_REG_GPIO0P7CTLO, PMIC_REG_GPIO0P7CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },// rotate lock button (debounce enable/50K pullup)
  {"GPIO1P0", PMIC_REG_GPIO1P0CTLO, PMIC_REG_GPIO1P0CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PU|PMIC_MASK_ALTFUNCEN),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) }, //enable altfunc "UIBTN_B" home screen (debounce enable)
  {"GPIO1P3", PMIC_REG_GPIO1P3CTLO, PMIC_REG_GPIO1P3CTLI, DIR_OUTPUT, INTERRUPT_DIS, (PMIC_MASK_RVAL_50K_PU|PMIC_MASK_DOUT| PMIC_MASK_DIR),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//FFRD8 PR1 TOUCH'S powergating
};

GpioCfg_st g_GPIO_cfg[]= {
  {"GPIO0P0", PMIC_REG_GPIO0P0CTLO, PMIC_REG_GPIO0P0CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//"Volumn up" (debounce enable)
  {"GPIO0P1", PMIC_REG_GPIO0P1CTLO, PMIC_REG_GPIO0P1CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//"Volumn down" (debounce enable)
  {"GPIO0P7", PMIC_REG_GPIO0P7CTLO, PMIC_REG_GPIO0P7CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },// rotate lock button (debounce enable/50K pullup)

  {"GPIO1P0", PMIC_REG_GPIO1P0CTLO, PMIC_REG_GPIO1P0CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PU|PMIC_MASK_ALTFUNCEN),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) }, //enable altfunc "UIBTN_B" home screen (debounce enable)
};

GpioCfg_st g_Pmic_Gpio_Lpc_Cfg[]= {
  {"GPIO1P1", PMIC_REG_GPIO1P1CTLO, PMIC_REG_GPIO1P1CTLI, DIR_OUTPUT, INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_RVAL_50K_PD|PMIC_MASK_DIR),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//PMIC_GPIO_1_LPC  GPO low 0x3c:0x34

};


EFI_STATUS
EFIAPI
PmicGpioToggleForLpcConfig()
{
  UINT32 index = 0;
  UINT8 value, regoffset;
  GpioCfg_st *m_GPIO_cfg;
  UINT32 length;

  DEBUG ((DEBUG_INFO, "PmicGpioToggleConfig\n"));

  //pay attention whether it's platform related
  m_GPIO_cfg = g_Pmic_Gpio_Lpc_Cfg;
  length = sizeof(g_Pmic_Gpio_Lpc_Cfg)/sizeof(GpioCfg_st);

  for (; index < length; index++) {
    regoffset = m_GPIO_cfg[index].OutputReg;
    value = PmicRead8(regoffset);
    value &= ~PMIC_MASK_OUTPUT_ALL;
    value |= (m_GPIO_cfg[index].GPOCtrlVal & PMIC_MASK_OUTPUT_ALL);
    PmicWrite8(regoffset, value);

    regoffset = m_GPIO_cfg[index].InputReg;
    value = PmicRead8(regoffset);
    value &= ~PMIC_MASK_INPUT_ALL;
    value |= (m_GPIO_cfg[index].GPICtrlVal & PMIC_MASK_INPUT_ALL);
    PmicWrite8(regoffset, value);

    DEBUG ((DEBUG_INFO, "%s 0x%x:0x%x 0x&x:0x%x\n", m_GPIO_cfg[index].PinName, m_GPIO_cfg[index].OutputReg, m_GPIO_cfg[index].GPOCtrlVal, m_GPIO_cfg[index].InputReg, m_GPIO_cfg[index].GPICtrlVal));
  }

  return EFI_SUCCESS;
}



EFI_STATUS
EFIAPI
PmicGpioInit (void *PlatformInfo)
{
  UINT32 index = 0;
  UINT8 value, regoffset;
  GpioCfg_st *m_GPIO_cfg = g_GPIO_cfg;
  EFI_PLATFORM_INFO_HOB *mPlatformInfo = NULL;
  UINT32 length = sizeof(g_GPIO_cfg)/sizeof(GpioCfg_st);

  DEBUG ((DEBUG_INFO, "PmicGpioInit\n"));


  if (PlatformInfo == NULL) {
    //
    // Uses default GPIO table.
    //
    m_GPIO_cfg = g_GPIO_cfg;
    length = sizeof(g_GPIO_cfg)/sizeof(GpioCfg_st);
  } else {
    mPlatformInfo = (EFI_PLATFORM_INFO_HOB *)PlatformInfo;
    if (mPlatformInfo->BoardId == BOARD_ID_BL_FFRD && mPlatformInfo->BoardRev >= PR1) {
      //
      // PR1 uses different GPIO table
      //
      m_GPIO_cfg = g_GPIO_PR1_cfg;
      length = sizeof(g_GPIO_PR1_cfg)/sizeof(GpioCfg_st);
    }
    if ((mPlatformInfo->BoardId == BOARD_ID_BL_FFRD8) && (mPlatformInfo->BoardRev == FFRD_8_PR0)) {
      //
      // FFRD8 table (PMIC_VIBE_EN setting diffs from FFRD10 PR1)
      //
      m_GPIO_cfg = g_GPIO_FFRD8_cfg;
      length = sizeof(g_GPIO_FFRD8_cfg)/sizeof(GpioCfg_st);
    }
    if ((mPlatformInfo->BoardId == BOARD_ID_BL_FFRD8) && (mPlatformInfo->BoardRev >= FFRD_8_PR1)) {
      // FFRD8 PR1 table, GPIO1 P3 is different from FFRD PR0
      // Modify the Output Reg
      // The configuration required for the Touch power control line is PUSH-PULL,
      // Open drain will not work for FFRD8 PR1
      m_GPIO_cfg = g_GPIO_FFRD8_PR1_cfg;
      length = sizeof(g_GPIO_FFRD8_PR1_cfg)/sizeof(GpioCfg_st);
    }
  }

  for (; index < length; index++) {
    regoffset = m_GPIO_cfg[index].OutputReg;
    value = PmicRead8(regoffset);
    value &= ~PMIC_MASK_OUTPUT_ALL;
    value |= (m_GPIO_cfg[index].GPOCtrlVal & PMIC_MASK_OUTPUT_ALL);
    PmicWrite8(regoffset, value);

    regoffset = m_GPIO_cfg[index].InputReg;
    value = PmicRead8(regoffset);
    value &= ~PMIC_MASK_INPUT_ALL;
    value |= (m_GPIO_cfg[index].GPICtrlVal & PMIC_MASK_INPUT_ALL);
    PmicWrite8(regoffset, value);

    DEBUG ((DEBUG_INFO, "%a 0x%x:0x%x 0x%x:0x%x\n", m_GPIO_cfg[index].PinName, m_GPIO_cfg[index].OutputReg, m_GPIO_cfg[index].GPOCtrlVal, m_GPIO_cfg[index].InputReg, m_GPIO_cfg[index].GPICtrlVal));
  }
  return EFI_SUCCESS;
}

RegInit_st g_IntrRegInit[]= {
  {PMIC_REG_MGPIO0IRQS0,   REG_OVERRIDE, 0xff, 0xff},    //disable all GPIO interrupts
  {PMIC_REG_MGPIO1IRQS0,   REG_OVERRIDE, 0xff, 0xff},
  {PMIC_REG_MGPIO0IRQSX,   REG_OVERRIDE, 0xff, 0xff},
  {PMIC_REG_MGPIO1IRQSX,   REG_OVERRIDE, 0xff, 0xff},
  {PMIC_REG_MADCIRQ0,      REG_OVERRIDE, 0xff, 0xff},
  {PMIC_REG_MADCIRQ1,      REG_OVERRIDE, 0x1f, 0x1f},
  {PMIC_REG_MCHGRIRQS0,    REG_CLEAR,    PMIC_MASK_MCHGR,  PMIC_MASK_MCHGR}, //enable external charger interrupt
  {PMIC_REG_MCHGRIRQSX,    REG_CLEAR,    PMIC_MASK_MCHGR,  PMIC_MASK_MCHGR}, //enable external charger interrupt
  {PMIC_REG_MPWRSRCIRQS0,  REG_CLEAR,    (PMIC_MASK_BATDET|PMIC_MASK_DCINDET|PMIC_MASK_VBUSDET), (PMIC_MASK_BATDET|PMIC_MASK_DCINDET|PMIC_MASK_VBUSDET)   }, //enable power src interrupt
  {PMIC_REG_MPWRSRCIRQSX,  REG_CLEAR,    (PMIC_MASK_BATDET|PMIC_MASK_DCINDET|PMIC_MASK_VBUSDET), (PMIC_MASK_BATDET|PMIC_MASK_DCINDET|PMIC_MASK_VBUSDET)   },
  {PMIC_REG_MIRQLVL1,      REG_OVERRIDE,  PMIC_MASK_ALL_IRQ,  (PMIC_MASK_MPWRSRC|PMIC_MASK_MCHGRINT|PMIC_MASK_MADC|PMIC_MASK_MGPIO)},  //only enable 4kinds of intrs
  {PMIC_REG_MTHRMIRQ0,     REG_OVERRIDE,  0xff, 0xff}, //disable all THerm intrs
  {PMIC_REG_MTHRMIRQ1,     REG_OVERRIDE,  0xf,  0xf},
  {PMIC_REG_MTHRMIRQ2,     REG_OVERRIDE,  0x3f, 0x3f},

  {PMIC_REG_GPIO0IRQ,      REG_OVERRIDE, 0xff, 0xff},     //write clear
  {PMIC_REG_GPIO1IRQ,      REG_OVERRIDE, 0xff, 0xff},
};

EFI_STATUS
EFIAPI
PmicIntrInit (void)
{
  UINT32 length = sizeof(g_IntrRegInit)/sizeof(RegInit_st);
  DEBUG ((DEBUG_INFO, "PmicIntrInit\n"));
  return PmicBatchRegisterInit(g_IntrRegInit, length);
}

RegInit_st g_BcuRegInit[]= {
  {PMIC_REG_VWARNA_CFG,      REG_CLEAR,    PMIC_MASK_VWARNA_EN, PMIC_MASK_VWARNA_EN},  //TODO: require meaningful value from HW teams
  {PMIC_REG_VWARNB_CFG,      REG_CLEAR,    PMIC_MASK_VWARNB_EN, PMIC_MASK_VWARNB_EN},  //disable VWENB
  {PMIC_REG_VCRIT_CFG,       REG_CLEAR,    PMIC_MASK_VCRIT_EN,  PMIC_MASK_VCRIT_EN},   //disable VCRIT
  {PMIC_REG_BCUDISA_BEH,     REG_OVERRIDE, 0x0, 0x0},
  {PMIC_REG_BCUDISB_BEH,     REG_OVERRIDE, 0x0, 0x0},
  {PMIC_REG_BCUDISCRIT_BEH,  REG_OVERRIDE, 0x0, 0x0},
  {PMIC_REG_BCUPROCHOT_B_BEH,REG_OVERRIDE, 0x0, 0x0},
  {PMIC_REG_MBCUIRQ,         REG_OVERRIDE, PMIC_MASK_MBCU_ALL, PMIC_MASK_MBCU_ALL },  //mask all
};

EFI_STATUS
EFIAPI
PmicBcuInit (void)
{
  UINT32 length = sizeof(g_BcuRegInit)/sizeof(RegInit_st);
  DEBUG ((DEBUG_INFO, "PmicBcuInit\n"));
  return PmicBatchRegisterInit(g_BcuRegInit, length);
}

RegInit_st g_MiscRegInit[]= {
// power OFF VPROG_2P85SX & VPROG_1P8SX rails to save more power before OS boots up for FFD8.
  {PMIC_REG_V1P8SXCNT,  REG_OVERRIDE,    PMIC_VR_SEL_BIT | PMIC_VR_EN_BIT,  PMIC_VR_SEL_BIT | PMIC_VR_EN_BIT},      //0-VBUS_EN is controlled by ULPI_VBUS_EN
  {PMIC_REG_V2P85SXCNT, REG_OVERRIDE,    PMIC_VR_SEL_BIT | PMIC_VR_EN_BIT,  PMIC_VR_SEL_BIT | PMIC_VR_EN_BIT},      //0-VBUS_EN is controlled by ULPI_VBUS_EN
  {PMIC_REG_VBUSCNT,    REG_OVERRIDE,    PMIC_VR_SEL_BIT | PMIC_VR_EN_BIT,  PMIC_VR_SEL_BIT | PMIC_VR_EN_BIT},      //0-VBUS_EN is controlled by ULPI_VBUS_EN
  {PMIC_REG_VHDMICNT,   REG_OVERRIDE,    PMIC_VR_SEL_BIT | PMIC_VR_EN_BIT,  PMIC_VR_SEL_BIT | PMIC_VR_EN_BIT},      //0-VBUS_EN is controlled by ULPI_VBUS_EN
  {PMIC_REG_GPIO0P3CTLO,REG_OVERRIDE,    0xFF,                              0x21},  //for PMIC audio  reset GPIO
  {PMIC_REG_PBCONFIG,    REG_OVERRIDE,    0xFF,                             0x2F},   //for power button override.
  {PMIC_REG_GPIO0IRQ,    REG_OVERRIDE,    0xFF,                             0xFF},   //Write clear
  {PMIC_REG_GPIO1IRQ,    REG_OVERRIDE,    0xFF,                             0xFF},   //write clear
  /*  {PMIC_REG_PBCONFIG,   REG_CLEAR,    PMIC_MASK_UIBTNDIS, PMIC_MASK_UIBTNDIS},     //UI button enabled
    {PMIC_REG_LOWBATDET0, REG_OVERRIDE, PMIC_MASK_DCBOOT,   PMIC_MASK_DCBOOT},       //BATLOW_B is de-asserted with AC/DC adapter alone, no battery voltage dependency
    {PMIC_REG_PSDETCTRL,  REG_CLEAR,    PMIC_MASK_BATRMPDEN, PMIC_MASK_BATRMPDEN },  //Battery removal will not shut down system when AC is on
    */
};


EFI_STATUS
EFIAPI
PmicMiscInit (void)
{
  UINT8 DevId=0, RevId=0;
  UINT32 length = sizeof(g_MiscRegInit)/sizeof(RegInit_st);

  DEBUG ((DEBUG_INFO, "PmicMiscInit------------------\n"));
  PmicBatchRegisterInit(g_MiscRegInit, length);


  PmicGetDevID(&DevId, &RevId);
  // apply for Dialog PMIC only
  if ((DevId == PMIC_DEV_DIALOG) || (DevId == PMIC_DEV_DIALOG_1))  {

    PmicWrite8(PMIC_REG_LOWBATDET0, 0xC7);  //BATLOW_B is de-asserted with AC/DC adapter alone, no battery voltage dependency . LOWBATT threshold 3.1V b'0111
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PmicPage0Init (void *Profile)
{
  UINT8 val=0;
  UINT8 Address;
  UINT8 DevId=0, RevId=0;

  Address = PMIC_REG_PWRSEQCFG;
  PmicGetDevID(&DevId, &RevId);
  // HSD 4943274   WA for Dialog PMIC
  // If register 0x00 = 2Bh, and register 0x01 = B0,
  // use address 0xDF for SRCWAKECFG register, address 0xE0 for PWRSEQCFG (for Dialog B0 or later revision silicon).
  // Otherwise, use address 0xDB for SRCWAKECFG register, address 0xDC for PWRSEQCFG (for Rohm Silicon or Dialog silicon prior to B0)
  // From Lance, hacking
  // It is sufficient to only check register 0x00=2B (which is true for Dialog B0+).
  if (DevId == PMIC_DEV_DIALOG_1) {
    Address = PMIC_REG_PWRSEQCFG_DIALOG_B0;
  }

  // it appies to both A0 and B0 silicon, set DTPWROK to 10ms
  val = PMIC_MASK_SUSPWRDNNACK | PMIC_MASK_VCCPWRCFG | 0x01;
  PmicWrite8_page0(Address, val);  //set to 0xC
  val = PmicRead8_page0(Address);
  DEBUG ((DEBUG_INFO, "PmicPage0Init--------------0x%x:0x%x\n", Address, val));


  // Write  the value 0xEB to PMIC register address 0xF9.  vt.pmic.page0_reg(0xF9,0xEB).
  // ROHM PMIC B1 and later versions of PMIC
  if ((DevId == PMIC_DEV_ROHM) && (RevId >= 0xB1))  {
    PmicWrite8_page0(0xF9, 0xAD); // unlock the register
    PmicWrite8_page0(0xF9, 0xEB); // change the V2P85S and V3P3A to low quiescent current mode
    MicroSecondDelay ( 100 );     //sleep 100us for incorrect address during read
  }
//<EIP150193-> >>>
/*
  if((DevId == PMIC_DEV_DIALOG_1) && (RevId >= PMIC_DIALOG_REVID_C0))  {
    //PMIC sequencer configuration register update for S0ix (overrride default setting)
    //this is to fix issue (Diag C0 VBUS off after S0ix exit)
    PmicWrite8_page0(0x44, 0x2D);
    PmicWrite8_page0(0x45, 0xAD);
    PmicWrite8_page0(0x48, 0xAD);
    PmicWrite8_page0(0x52, 0xAD);
    PmicWrite8_page0(0x53, 0xAD);
    PmicWrite8_page0(0x49, 0xAD);
    PmicWrite8_page0(0x40, 0xAD);
    MicroSecondDelay ( 100 ); 
    DEBUG ((DEBUG_INFO, "Dialog C0 WA\n"));
  }
*/
//<EIP150193-> <<<
  return EFI_SUCCESS;

}


/**
  Turn on or off VBUS for OTG

  @param  bTurnOn    TRUE-turn on VBUS   FALSE-turn off VBUS

**/

EFI_STATUS
EFIAPI
PmicVbusControl (BOOLEAN bTurnOn)
{
  UINT8  value = 0;

  value = PmicRead8(PMIC_REG_VBUSCNT);
  //VR_SEL - 1 VBUS_EN is controlled by this register
  if(TRUE == bTurnOn) {
    value |= (PMIC_VR_SEL_BIT | PMIC_VR_EN_BIT);
  } else {
    value |=  PMIC_VR_SEL_BIT;
    value &= ~PMIC_VR_EN_BIT;
  }

  PmicWrite8(PMIC_REG_VBUSCNT, value);
  return EFI_SUCCESS;
}

/**
  Turn on or off V5P0S (5V VBUS for USB2/3 HOST)

  @param  bTurnOn    TRUE-turn on V5P0S   FALSE-turn off V5P0S

**/

EFI_STATUS
EFIAPI
PmicVhostControl (BOOLEAN bTurnOn)
{
  UINT8  value = 0;

  value = PmicRead8(PMIC_REG_VHOSTCNT);
  //VR_SEL - 1 VBUS_EN is controlled by this register
  if(TRUE == bTurnOn) {
    value |= (PMIC_VR_SEL_BIT | PMIC_VR_EN_BIT);
  } else {
    value |=  PMIC_VR_SEL_BIT;
    value &= ~PMIC_VR_EN_BIT;
  }

  PmicWrite8(PMIC_REG_VHOSTCNT, value);
  return EFI_SUCCESS;
}

/**
  Get vendor id

  @param  NULL

  @return  BATID voltage

**/

EFI_STATUS
EFIAPI
PmicGetDevID (UINT8 *DevId, UINT8 *RevId)
{
  *DevId = PmicRead8(PMIC_REG_ID0);
  *RevId = PmicRead8(PMIC_REG_REV);
  return EFI_SUCCESS;
}


/**
  Get BATID voltage

  @param  NULL

  @return  BATID voltage

**/

UINT16
EFIAPI
PmicGetBATID (void)
{
  UINT8 value, i;
  UINT8 DevId=0, RevId=0;

  //request capturing BATID.
  value = PmicRead8(PMIC_REG_MANCONV0);
  value |=  PMIC_MASK_BATID;

  PmicGetDevID(&DevId, &RevId);
  if (DevId == PMIC_DEV_ROHM) {
    //W/A for Rohm PMIC to solove the BATID issue. to read 55 times here to stablize the Capacitor in Battery.
    for (i = 0; i < 55; i++)
    {
      PmicWrite8(PMIC_REG_MANCONV0, value);
      MicroSecondDelay ( 380 );
    }
  } else
  {
    PmicWrite8(PMIC_REG_MANCONV0, value);
  }
  //Bit automatically clears after the conversion is completed.
  while(PmicRead8(PMIC_REG_MANCONV0) & PMIC_MASK_BATID);

  return ((PmicRead8(PMIC_REG_BATIDRSLTH) & 0x3) << 8 ) | (PmicRead8(PMIC_REG_BATIDRSLTL) & 0xff);
}

/**
  Get VBAT voltage

  @param  NULL

  @return  battery voltage

**/

UINT16
EFIAPI
PmicGetVBAT (void)
{
  UINT8 value;
  //request capturing BATID.
  value = PmicRead8(PMIC_REG_MANCONV0);
  value |=  PMIC_MASK_VBAT;
  PmicWrite8(PMIC_REG_MANCONV0, value);
  //Bit automatically clears after the conversion is completed.
  while(PmicRead8(PMIC_REG_MANCONV0) & PMIC_MASK_VBAT);

  return ((PmicRead8(PMIC_REG_VBATRSLTH) & 0x3) << 8 ) | (PmicRead8(PMIC_REG_VBATRSLTL) & 0xff);
}

#define DELAY_BETWEEN_INSTRUCTION   10
#define DELAY_BETWEEN_INSTRUCTION_1 50

//SetVIDDecay workaround
EFI_STATUS
EFIAPI
PmicSetVIDDecayWA (void)
{
  UINT8 DevId=0, RevId=0;

  PmicGetDevID(&DevId, &RevId);
  // apply for Rohm PMIC only
  if (DevId == PMIC_DEV_ROHM) {
    PmicWrite8_page0(0xFC, 0x59);
    MicroSecondDelay(DELAY_BETWEEN_INSTRUCTION_1);
    PmicWrite8_page0(0xFC, 0x63);
    MicroSecondDelay(DELAY_BETWEEN_INSTRUCTION_1);
    PmicWrite8_page0(0xFC, 0xFF);
    MicroSecondDelay(DELAY_BETWEEN_INSTRUCTION_1);
    PmicWrite8(0xF5, 0x50);
    MicroSecondDelay(DELAY_BETWEEN_INSTRUCTION_1);
    PmicWrite8_page0(0xFC, 0x59);
    MicroSecondDelay(DELAY_BETWEEN_INSTRUCTION_1);
    PmicWrite8_page0(0xFC, 0x63);
    MicroSecondDelay(DELAY_BETWEEN_INSTRUCTION_1);
    PmicWrite8_page0(0xFC, 0x00);
    MicroSecondDelay(DELAY_BETWEEN_INSTRUCTION_1);
    DEBUG ((DEBUG_INFO, "Rohm WA for SetVIDDecay\n"));
  }

  return EFI_SUCCESS;

}


BOOLEAN
EFIAPI
PmicIsACOn (void)
{
  UINT8 value;

  value = PmicRead8(PMIC_REG_SPWRSRC);
  return ((value & PMIC_MASK_DCINDET) == PMIC_MASK_DCINDET);
}

BOOLEAN
EFIAPI
PmicIsPwrBtnPressed(void)
{
  UINT8 value;

  value = PmicRead8(PMIC_REG_PBSTATUS);
  return ((value & PMIC_MASK_PBLVL) != PMIC_MASK_PBLVL);
}

BOOLEAN
EFIAPI
PmicIsUIBtnPressed(void)
{
  UINT8 value;

  value = PmicRead8(PMIC_REG_UIBSTATUS);
  return ((value & PMIC_MASK_UIBLVL) != PMIC_MASK_UIBLVL);
}


UINT16
EFIAPI
PmicGetResetCause (void)
{
  return ((PmicRead8(PMIC_REG_RESETSRC0) << 8) | (PmicRead8(PMIC_REG_RESETSRC1) & 0xff));
}

UINT8
EFIAPI
PmicGetWakeCause (void)
{
  return (PmicRead8(PMIC_REG_WAKESRC) & 0xff);
}


EFI_STATUS
EFIAPI
PmicDebugRegDump (void)
{
  UINT8 reg= 0;

  for (; reg <= PMIC_REG_VREFDQ1CNT; reg++)    {
    DEBUG ((DEBUG_INFO, "reg:0x%x:0x%x\n", reg, PmicRead8(reg)));
  }
  return EFI_SUCCESS;
}

/**
  Get Board ID

  @param  NULL

  @return  Board ID

**/
UINT8
EFIAPI
PmicGetBoardID(void)
{
  UINT8 value = 0;
  value = (PmicRead8(PMIC_REG_GPIO0P2CTLI) & 0x01);
  value |= ((PmicRead8(PMIC_REG_GPIO0P5CTLI) & 0x01) << 1);
  value |= ((PmicRead8(PMIC_REG_GPIO0P6CTLI) & 0x01) << 2);
  value |= ((PmicRead8(PMIC_REG_GPIO1P2CTLI) & 0x01) << 3);
  return value;
}

UINT8
EFIAPI
PmicGetFABID(void)
{
  UINT8 value;
  value = (PmicRead8(PMIC_REG_GPIO1P6CTLI) & 0x01);
  value |= ((PmicRead8(PMIC_REG_GPIO1P7CTLI) & 0x01) << 1);
  return value;
}
