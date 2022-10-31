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
#include <EdkIIGluePeim.h>
#else
#include <Uefi.h>
#include <Library/DebugLib.h>
#endif
#ifdef ECP_FLAG
#include "I2CLib.h"
#include "PmicLib.h"
#else
#include <Library/I2CLib.h>
#include <Library/PmicLib.h>
#endif
#include "PmicRegPei.h"

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
  EFI_STATUS rc=0;

  rc = ByteWriteI2C(PMIC_I2C_BUSNO, PMIC_PAGE_1_I2C_ADDR, Register, 1, &Value);
  if (EFI_SUCCESS != rc)  {
    DEBUG ((DEBUG_INFO, "PmicWrite8 failed :0x%x\n", rc));
    return rc;
  }
  return EFI_SUCCESS;
}


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
  UINT8 value;

  for (; index < length; index++) {
    value = PmicRead8(RegInit[index].regoffset);

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
    DEBUG ((DEBUG_INFO, "PmicBatchRegisterInit 0x%x:0x%x\n", RegInit[index].regoffset, value));
  }
  return EFI_SUCCESS;

}


GpioCfg_st g_GPIO_cfg[]= {
//  {"GPIO0P0", PMIC_REG_GPIO0P0CTLO, PMIC_REG_GPIO0P0CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//"Volumn up" (debounce enable)
//  {"GPIO0P1", PMIC_REG_GPIO0P1CTLO, PMIC_REG_GPIO0P1CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },//"Volumn down" (debounce enable)
  {"GPIO0P2", PMIC_REG_GPIO0P2CTLO, PMIC_REG_GPIO0P2CTLI, DIR_INPUT,  INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PD),  (0) },// board-id0 (input default 0 means no polarity/no debounce/interrupt disable)
//  {"GPIO0P3", PMIC_REG_GPIO0P3CTLO, PMIC_REG_GPIO0P3CTLI, DIR_OUTPUT, INTERRUPT_DIS, (PMIC_MASK_DIR|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PD),  (0) },// i2s-reset  (cmos enable/output-low)
//  {"GPIO0P4", PMIC_REG_GPIO0P4CTLO, PMIC_REG_GPIO0P4CTLI, DIR_OUTPUT, INTERRUPT_DIS, (PMIC_MASK_DIR|PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PD), (0) },// speaker amp (output-low)
  {"GPIO0P5", PMIC_REG_GPIO0P5CTLO, PMIC_REG_GPIO0P5CTLI, DIR_INPUT,  INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PD),  (0) },// board-id 1
  {"GPIO0P6", PMIC_REG_GPIO0P6CTLO, PMIC_REG_GPIO0P6CTLI, DIR_INPUT,  INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PD),  (0) },// board-id 2
//  {"GPIO0P7", PMIC_REG_GPIO0P7CTLO, PMIC_REG_GPIO0P7CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) },// rotate locj button (debounce enable/50K pullup)

//  {"GPIO1P0", PMIC_REG_GPIO1P0CTLO, PMIC_REG_GPIO1P0CTLI, DIR_INPUT,  INTERRUPT_EN,  (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PU),  (PMIC_MASK_ALTFUNCEN|PMIC_MASK_GPIDBNC|PMIC_MASK_INTCNT_BOTH) }, //enable altfunc "UIBTN_B" home screen (debounce enable)
//  {"GPIO1P1", PMIC_REG_GPIO1P1CTLO, PMIC_REG_GPIO1P1CTLI, DIR_OUTPUT, INTERRUPT_DIS, (PMIC_MASK_DIR|PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PD), (0) },// LPC power gate(output-low)
  {"GPIO1P2", PMIC_REG_GPIO1P2CTLO, PMIC_REG_GPIO1P2CTLI, DIR_INPUT,  INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PD),  (0) },// board-id 3
//  {"GPIO1P3", PMIC_REG_GPIO1P3CTLO, PMIC_REG_GPIO1P3CTLI, DIR_OUTPUT, INTERRUPT_DIS, (PMIC_MASK_DIR|PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PD), (0) },// NC (output-low)
  {"GPIO1P4", PMIC_REG_GPIO1P4CTLO, PMIC_REG_GPIO1P4CTLI, DIR_INPUT,  INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PD),  (0) },// memcfg-id0
  {"GPIO1P5", PMIC_REG_GPIO1P5CTLO, PMIC_REG_GPIO1P5CTLI, DIR_INPUT,  INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PD),  (0) },// memcfg-id1
  {"GPIO1P6", PMIC_REG_GPIO1P6CTLO, PMIC_REG_GPIO1P6CTLI, DIR_INPUT,  INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PD),  (0) },// fab-id0
  {"GPIO1P7", PMIC_REG_GPIO1P7CTLO, PMIC_REG_GPIO1P7CTLI, DIR_INPUT,  INTERRUPT_DIS, (PMIC_MASK_DRV|PMIC_MASK_REN|PMIC_MASK_RVAL_50K_PD),  (0) },// fab-id1
};


EFI_STATUS
EFIAPI
PmicGpioInit (void *PlatformInfo)
{
  UINT32 index = 0;
  UINT8 value, regoffset;
  UINT32 length = sizeof(g_GPIO_cfg)/sizeof(GpioCfg_st);
  DEBUG ((DEBUG_INFO, "PmicGpioInit\n"));

  for (; index < length; index++) {
    regoffset = g_GPIO_cfg[index].OutputReg;
    value = PmicRead8(regoffset);
    value &= ~PMIC_MASK_OUTPUT_ALL;
    value |= (g_GPIO_cfg[index].GPOCtrlVal & PMIC_MASK_OUTPUT_ALL);
    PmicWrite8(regoffset, value);

    regoffset = g_GPIO_cfg[index].InputReg;
    value = PmicRead8(regoffset);
    value &= ~PMIC_MASK_INPUT_ALL;
    value |= (g_GPIO_cfg[index].GPICtrlVal & PMIC_MASK_INPUT_ALL);
    PmicWrite8(regoffset, value);

    DEBUG ((DEBUG_INFO, "%a 0x%x:0x%x 0x%x:0x%x\n", g_GPIO_cfg[index].PinName, g_GPIO_cfg[index].OutputReg, g_GPIO_cfg[index].GPOCtrlVal, g_GPIO_cfg[index].InputReg, g_GPIO_cfg[index].GPICtrlVal));
  }
  //
  // Read GPIO1P1, if it is default value 0x14, then do nothing, if it is 0x34, then pull up the pin to ungate LPC power
  //
  value = PmicRead8(0x3C);
  if (value == 0x34) {
    PmicWrite8(0x3C, 0x14);
  }
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
  UINT8 value;
  //request capturing BATID.
  value = PmicRead8(PMIC_REG_MANCONV0);
  value |=  PMIC_MASK_BATID;
  PmicWrite8(PMIC_REG_MANCONV0, value);
  //Bit automatically clears after the conversion is completed.
  while(PmicRead8(PMIC_REG_MANCONV0) & PMIC_MASK_BATID);

  return ((PmicRead8(PMIC_REG_BATIDRSLTH) & 0x3) << 8 ) | (PmicRead8(PMIC_REG_BATIDRSLTL) & 0xff);
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

EFI_STATUS
EFIAPI
PmicSetVDDQ (void)
{
  //VDDQ 1.35V for DDR3L
  PmicWrite8(PMIC_REG_VDDQCNT, 0x80);
  return EFI_SUCCESS;
}


/**
  Get Memory Config ID

  @param  NULL

  @return  Memory Config ID

**/
UINT8
EFIAPI
PmicGetMemCfgID(void)
{
  UINT8 value;
  value = (PmicRead8(PMIC_REG_GPIO1P4CTLI) & 0x01);
  value |=( (PmicRead8(PMIC_REG_GPIO1P5CTLI) & 0x01) << 1);
  return value;
}

/**
  Get FAB ID

  @param  NULL

  @return  FAB ID

**/
UINT8
EFIAPI
PmicGetFABID(void)
{
  UINT8 value;
  value = (PmicRead8(PMIC_REG_GPIO1P6CTLI) & 0x01);
  value |= ((PmicRead8(PMIC_REG_GPIO1P7CTLI) & 0x01) << 1);
  return value;
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


