/*++

Copyright (c) 1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

	OemHooks.c
	
Abstract: 

	This file include all the OEM hooks for MRC.

--*/

#include "OemHooks.h"
#include "SCRegs.h"
#include "IoAccess.h"
#include "ConfigMem.h"
#include "MchRegs.h"


VOID
Checkpoint (
  UINT16 Content
  )
/*++

Routine Description:

  Output a progress data to port 80 for debug purpose
  Could be safely overriden to
    send checkpoints elsewhere, such as port 0x84 or a serial port

--*/
{
  IoOut16(0x80, Content);
};


STATUS
OemTrackInitComplete (
  MRC_PARAMETER_FRAME           *CurrentMrcData, UINT8 Channel
  )
{
  UINT8 ResetRequired;
  UINT32 PMCON1;
  UINT8 SLPS4MinAssertWidth;
  UINT32 PmcBase;

  PmcBase = PciCfg32Read_CF8CFC (
		      DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC,
		      R_PCH_LPC_PMC_BASE
              );
  PmcBase &= B_PCH_LPC_PMC_BASE_BAR;

  //Read PMC_BASE + 0x20 GEN_PMCON1 register bit [23] - DRAM Initialization Scratchpad Bit which indicates BIOS DRAM Initialization status on previous boot
  PMCON1 = Mmio32Read (PmcBase + R_PCH_PMC_GEN_PMCON_1);

  PMCON1 &= ~BIT16;  // Do not clear PWR_FLR bit

  if (PMCON1 & B_PCH_PMC_GEN_PMCON_DRAM_INIT) {
    //Clear PMC_BASE + 0x20 GEN_PMCON1 register bit [23] to initiate global reset
    PMCON1 &= ~B_PCH_PMC_GEN_PMCON_DRAM_INIT;
    ResetRequired = 1;
  } else {
    //Set PMC_BASE + 0x20 GEN_PMCON1 register bit [23] to mark beginning of DRAM Initialization
    //(Will be cleared on successful completion of MRC)
    PMCON1 |= B_PCH_PMC_GEN_PMCON_DRAM_INIT;
    ResetRequired = 0;
  }

  if (PMCON1 & B_PCH_PMC_GEN_PMCON_MIN_SLP_S4) {

    PMCON1 |= B_PCH_PMC_GEN_PMCON_MIN_SLP_S4;

    // TODO: OEM to set The correct minimum assertion width which,
    // can be discovered by measuring the V_SM ramp down time.
    // Measure from the time that SLP_S4# signal is fully asserted to the time that V_SM falls to 0V
    // Set PMC_BASE + 0x20 GEN_PMCON1 register [5:4] = '11b' (i.e. SLP_S4# minimum assertion width of 1 to 2 seconds)
    SLPS4MinAssertWidth = (BIT5|BIT4);
    PMCON1 =  (PMCON1 & ~(BIT5|BIT4)) | (SLPS4MinAssertWidth & (BIT5|BIT4));

    //Set PMC_BASE + 0x20 GEN_PMCON1 register [3] SLP_S4# Assertion Stretch Enable= `1b'
    PMCON1 |= BIT3;

  }
  Mmio32Write (PmcBase + R_PCH_PMC_GEN_PMCON_1, PMCON1);

  if (ResetRequired) {
    // Write 0x0E to IO 0xCF9 to trigger power cycling reset
    IoOut8(0xCF9, 0xE);
  }

  return SUCCESS;
}

