/*++

Copyright (c) 2011 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  SCRegs.h

Abstract:

  This file include definition for ICH register.

--*/

#ifndef _ICHREGS_H_
#define _ICHREGS_H_

//
// Definitions beginning with "R_" are registers
// Definitions beginning with "B_" are bits within registers
// Definitions beginning with "V_" are meaningful values of bits within the registers
//

#define PMC_BASE_ADDRESS                 0xfed03000  //temp -- need to read from D31..
#define SPI_BASE_ADDRESS                 0xfed01000

#define DEFAULT_PCI_BUS_NUMBER_PCH       0

#define PCI_DEVICE_NUMBER_PCH_LPC        31
#define PCI_FUNCTION_NUMBER_PCH_LPC      0

#define R_PCH_LPC_PMC_BASE               0x44
#define   B_PCH_LPC_PMC_BASE_BAR         0xFFFFFE00

#define R_PCH_PMC_GEN_PMCON_1            0x20

//#define R_PCH_PMC_GEN_PMCON_2            0x42
#define   B_PCH_PMC_GEN_PMCON_DRAM_INIT  BIT23
#define   B_PCH_PMC_GEN_PMCON_MIN_SLP_S4 BIT18

//#define R_PCH_PMC_GEN_PMCON_3           0x44

#define R_PCH_PCH_HPET                  0xFED00000

#define R_PCH_PCH_HPET_GCFG             0x10
#define   B_PCH_PCH_HPET_GCFG_LRE       BIT1
#define   B_PCH_PCH_HPET_GCFG_EN        BIT0

#endif
