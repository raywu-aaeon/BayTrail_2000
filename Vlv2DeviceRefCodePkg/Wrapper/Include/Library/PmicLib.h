/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
  Header file of PMIC hardware definition.

  Copyright (c) 2006-1012, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation
  (if any) is furnished under a license and may only be used or
  copied in accordance with the terms of the license.  Except as
  permitted by such license, no part of this software or
  documentation may be reproduced, stored in a retrieval system, or
  transmitted in any form or by any means without the express written
  consent of Intel Corporation.

  Module Name:  PmicLib.h

**/

#ifndef _PMIC_LIB_H_
#define _PMIC_LIB_H_

#define PMIC_DEV_ROHM    0x1F
#define PMIC_DEV_DIALOG  0x00
#define PMIC_DEV_DIALOG_1  0x2B

typedef enum {
    REG_OVERRIDE    = 0,
    REG_AND         = 1,
    REG_OR          = 2,
    REG_CLEAR       = 3
} RegAction;


typedef struct RegInit {
    UINT8     regoffset;
    RegAction action;
    UINT8     mask;
    UINT8     value;
} RegInit_st;



typedef enum {
    DIR_INPUT  = 0,
    DIR_OUTPUT = 1,
} GpioAttrib;

typedef struct Gpio {
    UINT8 *PinName;
    UINT8 OutputReg;
    UINT8 InputReg;
    GpioAttrib Direction;
    UINT8 IsIntr;       //is it an interrupt
    UINT8 GPOCtrlVal;
    UINT8 GPICtrlVal;
} GpioCfg_st;



#define PMIC_I2C_BUSNO        6     //I2C6 is used. index from 0 
//Separated registers into two pages: page 0 (accessible through I2C bus address 0x5E) and page 1 (accessible through I2C bus address 0x6E).
//Page 0 is for OTP. Assigned addresses to registers on page 1.
#define PMIC_PAGE_0_I2C_ADDR  0x5E
#define PMIC_PAGE_1_I2C_ADDR  0x6E


#define INTERRUPT_EN   1
#define INTERRUPT_DIS  0


UINT8
EFIAPI
PmicRead8(
    IN      UINT8                     Register
);

EFI_STATUS
EFIAPI
PmicWrite8(
    IN      UINT8                     Register,
    IN      UINT8                     Value
);

EFI_STATUS
EFIAPI
PmicThermInit(void);

EFI_STATUS
EFIAPI
PmicGpioInit(void *PlatformInfo);

EFI_STATUS
EFIAPI
PmicIntrInit(void);

EFI_STATUS
EFIAPI
PmicBcuInit(void);

EFI_STATUS
EFIAPI
PmicMiscInit(void);

EFI_STATUS
EFIAPI
PmicPage0Init (void *Profile);

UINT8
EFIAPI
PmicRead8_page0 (
  IN      UINT8                     Register
  );

EFI_STATUS
EFIAPI
PmicWrite8_page0 (
  IN      UINT8                     Register,
  IN      UINT8                     Value
  );


EFI_STATUS
EFIAPI
PmicVbusControl(BOOLEAN bTurnOn);

EFI_STATUS
EFIAPI
PmicVhostControl(BOOLEAN bTurnOn);

EFI_STATUS
EFIAPI
PmicGetDevID(UINT8 *DevId, UINT8 *RevId);

UINT16
EFIAPI
PmicGetBATID(void);

UINT8
EFIAPI
PmicGetBoardID(void);

UINT8
EFIAPI
PmicGetMemCfgID(void);

UINT8
EFIAPI
PmicGetFABID(void);

UINT16
EFIAPI
PmicGetVBAT(void);

EFI_STATUS
EFIAPI
PmicSetVIDDecayWA(void);

BOOLEAN
EFIAPI
PmicIsACOn(void);

BOOLEAN
EFIAPI
PmicIsPwrBtnPressed(void);


BOOLEAN
EFIAPI
PmicIsUIBtnPressed(void);

UINT16
EFIAPI
PmicGetResetCause(void);


EFI_STATUS
EFIAPI
PmicDebugRegDump(void);



#endif
