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
  I2CAccess.h

  @brief 
  Macros that simplify accessing PCH devices's PCI registers.

  ** NOTE ** these macros assume the PCH device is on BUS 0

**/
#ifndef _I2C_ACCESS_H_
#define _I2C_ACCESS_H_

///
/// Memory Mapped PCI Access macros
///

#include "I2CIoLibPei.h"
#include <token.h>      // AMI_OVERRIDE

#define DEFAULT_PCI_BUS_NUMBER_PCH             0

#define PCI_DEVICE_NUMBER_PCH_LPC              31
#define PCI_FUNCTION_NUMBER_PCH_LPC            0

#define R_PCH_LPC_ACPI_BASE                    0x40              // ABASE, 16bit
#define R_PCH_LPC_ACPI_BASEADR                 0x400            // ABASE, 16bit
#define B_PCH_LPC_ACPI_BASE_EN                 BIT1              // Enable Bit
#define B_PCH_LPC_ACPI_BASE_BAR                0x0000FF80   // Base Address, 128 Bytes
#define V_PCH_ACPI_PM1_TMR_MAX_VAL             0x1000000 // The timer is 24 bit overflow
#define B_PCH_ACPI_PM1_TMR_VAL                 0xFFFFFF // The timer value mask

#define R_PCH_ACPI_PM1_TMR                     0x08              // Power Management 1 Timer
#define V_PCH_ACPI_PM1_TMR_FREQUENCY           3579545         // Timer Frequency


#define PchLpcPciCfg8(Register) I2CLibPeiMmioRead8 (MmPciAddress (0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, 0, Register))

//#define PCIEX_BASE_ADDRESS                     0xE0000000
#define PCI_EXPRESS_BASE_ADDRESS               ((VOID *) (UINTN) PCIEX_BASE_ADDRESS)

#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PCI_EXPRESS_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )
#endif

