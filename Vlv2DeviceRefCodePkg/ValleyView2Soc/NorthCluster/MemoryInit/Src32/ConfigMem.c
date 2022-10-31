/*++

Copyright (c) 2005-2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  ConfigMem.c

Abstract:

  Memory Configuration for Valleyview.

--*/

#include "ConfigMem.h"
#include "ConfigMemData.h"
#include "MchRegs.h"
#include "IoAccess.h"
#include "OemHooks.h"
#include <Guid/PlatformInfo.h>

#ifndef ECP_FLAG
#include <Library/DebugLib.h>
#endif
#include "../Mmrc/IpBlocks/VLVA0/Include/MMRCProjectLibraries.h"

STATUS
ProgDdrTimingControl (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
/*++

Routine Description:

  Programs the DRAM timing and control registers.

Arguments:

  CurrentMrcData:   Detected DDR timing parameters for installed memory



Returns:
  None

--*/
{

  UINT8  DdrFreqInx;
  UINT8  TCL;
  UINT8  TRP, TRCD, TRAS, TRFC, TWR, TWTR, TRRD, TRTP, TFAW;
  UINT8  WL = 5;

  RegDTR0       Dtr0;
  RegDTR1       Dtr1;
  RegDTR2       Dtr2;
  RegDTR3       Dtr3;
  RegDTR4       Dtr4;

  RegSCRMSEED SCRMSEEDreg;

  DdrFreqInx = CurrentMrcData->DdrFreq - MINDDR;
  SCRMSEEDreg.field.SCRMDIS = 1;	//Scrambler Disabled
  SCRMSEEDreg.field.SCRMSEED = 0;

  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_SCRMSEED_OFFSET, SCRMSEEDreg.raw);
  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_SCRMLO_OFFSET, 0);
  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_SCRMHI_OFFSET, 0);

  Dtr0.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DTR0_OFFSET);
  Dtr1.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DTR1_OFFSET);
  Dtr2.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DTR2_OFFSET);
  Dtr3.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DTR3_OFFSET);
  Dtr4.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DTR4_OFFSET);

#if defined DDR3_SUPPORT && DDR3_SUPPORT
    if (DDR3_DETECTED) {
        WL = 5 + (CurrentMrcData->DdrFreq);
    }
#endif

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (LPDDR3_DETECTED) {
        WL = 3 + CurrentMrcData->DdrFreq;

       if (CurrentMrcData->MemoryDown == 0) {
            switch (CurrentMrcData->DdrFreq) {
                case DDRFREQ_800:
                	WL = 3;
                    CurrentMrcData->Tcl = 6;
                    CurrentMrcData->TimingData[MRC_DATA_TRP] = 8;
                    CurrentMrcData->TimingData[MRC_DATA_TRCD] = 8;
                    CurrentMrcData->TimingData[MRC_DATA_TRAS] = 17;
                    CurrentMrcData->TimingData[MRC_DATA_TWR] = 6;
                    CurrentMrcData->TimingData[MRC_DATA_TWTR] = 3;
                    CurrentMrcData->TimingData[MRC_DATA_TRRD] = 4;
                    CurrentMrcData->TimingData[MRC_DATA_TRTP] = 3;
                    CurrentMrcData->TimingData[MRC_DATA_TFAW] = 20;
                break;
                case DDRFREQ_1066:
                    CurrentMrcData->Tcl = 8;
                    CurrentMrcData->TimingData[MRC_DATA_TRP] = 10;
                    CurrentMrcData->TimingData[MRC_DATA_TRCD] = 10;
                    CurrentMrcData->TimingData[MRC_DATA_TRAS] = 23;
                    CurrentMrcData->TimingData[MRC_DATA_TWR] = 8;
                    CurrentMrcData->TimingData[MRC_DATA_TWTR] = 4;
                    CurrentMrcData->TimingData[MRC_DATA_TRRD] = 6;
                    CurrentMrcData->TimingData[MRC_DATA_TRTP] = 4;
                    CurrentMrcData->TimingData[MRC_DATA_TFAW] = 28;
                break;
                case DDRFREQ_1333:
                	WL = 6;
                default:
                    CurrentMrcData->Tcl = 10;
                    CurrentMrcData->TimingData[MRC_DATA_TRP] = 12;
                    CurrentMrcData->TimingData[MRC_DATA_TRCD] = 12;
                    CurrentMrcData->TimingData[MRC_DATA_TRAS] = 28;
                    CurrentMrcData->TimingData[MRC_DATA_TWR] = 10;
                    CurrentMrcData->TimingData[MRC_DATA_TWTR] = 5;
                    CurrentMrcData->TimingData[MRC_DATA_TRRD] = 7;
                    CurrentMrcData->TimingData[MRC_DATA_TRTP] = 5;
                    CurrentMrcData->TimingData[MRC_DATA_TFAW] = 34;
                    break;
            }
       }
    }

#endif	//LPDDR3_SUPPORT

    TCL  = CurrentMrcData->Tcl;
    TRP  = CurrentMrcData->TimingData[MRC_DATA_TRP];
    TRCD = CurrentMrcData->TimingData[MRC_DATA_TRCD];
    TRAS = CurrentMrcData->TimingData[MRC_DATA_TRAS];
    TRFC = CurrentMrcData->TimingData[MRC_DATA_TRFC];
    TWR  = CurrentMrcData->TimingData[MRC_DATA_TWR];
    TWTR = CurrentMrcData->TimingData[MRC_DATA_TWTR];
    TRRD = CurrentMrcData->TimingData[MRC_DATA_TRRD];
    TRTP = CurrentMrcData->TimingData[MRC_DATA_TRTP];
    TFAW = CurrentMrcData->TimingData[MRC_DATA_TFAW];

    CurrentMrcData->WL = WL;
	Dtr0.field.dramFrequency = CurrentMrcData->DdrFreq;
	Dtr0.field.tRP  = TRP - 0x5;
	Dtr0.field.tRCD = TRCD - 0x5;
	Dtr0.field.tCL = TCL - 5;

	Dtr1.field.tWCL = WL - 3;
	Dtr1.field.tRAS = TRAS -14;
	Dtr1.field.tRRD = TRRD - 0x4;

	Dtr0.field.PMEDLY = 3;
	Dtr3.field.PWD_DLY = 6;


#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (LPDDR3_DETECTED) {
    	Dtr1.field.tCMD = 0; //LPDDR3 always set to 0 (1N)
		Dtr1.field.tCCD = 0;

    Dtr1.field.tFAW = 5 + (CurrentMrcData->DdrFreq * 4);	//Formula valid for LPDDR-800 and LPDDR-1066
    Dtr1.field.tRTP = MAX(TRTP, (3+CurrentMrcData->DdrFreq)) - 0x3;
	Dtr1.field.tWTP = WL + 0x4 + 1 + TWR - 14;

    Dtr2.field.tRRDR = 2;
    Dtr2.field.tRRDD = 2;
    Dtr2.field.tWWDR = 0;

    Dtr2.field.tWWDD = 0;
    Dtr2.field.tRWDR = 5 + CurrentMrcData->DdrFreq;
    Dtr2.field.tRWDD = 5 + CurrentMrcData->DdrFreq;
    Dtr3.field.tRWSR = 5 + CurrentMrcData->DdrFreq  + 1;
    Dtr3.field.tWRDR = 2;
    Dtr3.field.tWRDD = 1;
    Dtr3.field.tWRSR = 4 + WL + TWTR + 1 - 11;
	
	if ((CurrentMrcData->SiRevisionID >= 0x05) && (CurrentMrcData->SiRevisionID <= 0x0C)) {
        if (CurrentMrcData->DdrFreq == DDRFREQ_1066) {
    		Dtr3.field.tXP = 3;
		} else {
			Dtr3.field.tXP = 1 + CurrentMrcData->DdrFreq;
		}
	} else {
        if ((CurrentMrcData->DdrFreq == DDRFREQ_1066)||(CurrentMrcData->DdrFreq == DDRFREQ_800)) {
    		Dtr3.field.tXP = 0;
		} else if (CurrentMrcData->DdrFreq == DDRFREQ_1333) {
			Dtr3.field.tXP = 1;
		}
	}
	Dtr0.field.CKEDLY = 0x0;
	MsgBus32AndThenOr(VLV_UNIT_BUNIT, BUNIT_BISOC_OFFSET, 0xFFFFFF80, 0xF);

    Dtr0.field.tCL = 1 + (CurrentMrcData->DdrFreq * 2);

	if (CurrentMrcData->DdrFreq == DDRFREQ_1066) {
		Dtr3.field.tRWSR = 0x6;
	}
    if (CurrentMrcData->DdrFreq == DDRFREQ_1333) {
        Dtr2.field.tRWDR = 9;
        Dtr2.field.tRWDD = 9;
        Dtr3.field.tWRDR = 1;
        Dtr3.field.tRWSR = 9;
        Dtr1.field.tFAW = 0xC;
        Dtr3.field.tWRDD = 0;
    }
  }
#endif	//LPDDR3_SUPPORT

#if defined DDR3_SUPPORT && DDR3_SUPPORT
    if (DDR3_DETECTED) {
    	Dtr1.field.tCMD = 0;		//1N
		Dtr1.field.tCCD = 0;

    Dtr1.field.tFAW = ((TFAW+1)>>1) - 5;
	Dtr1.field.tWTP = WL + 0x4 + TWR - 14;

    Dtr2.field.tRRDR = 1;
    Dtr2.field.tRRDD = 1;
    Dtr2.field.tWWDR = 2;
    Dtr2.field.tWWDD = 2;
    Dtr2.field.tRWDR = 2;
    Dtr2.field.tRWDD = 1;
    Dtr3.field.tWRDR = 2;
    Dtr3.field.tWRDD = 2;
    Dtr3.field.tWRSR = 4 + WL + TWTR - 11 +1;
	
    if (CurrentMrcData->DdrFreq == DDRFREQ_800) {
    	Dtr3.field.tRWSR = TCL - 5;
		Dtr1.field.tRTP = MAX(TRTP, 4) - 0x3;
    } else if(CurrentMrcData->DdrFreq == DDRFREQ_1066) {
    	Dtr3.field.tRWSR = TCL - 5;
		Dtr1.field.tRTP = MAX(TRTP, 4) - 0x3;
	    Dtr2.field.tRWDR = 2;
	    Dtr3.field.tWRSR = 3;
    } else if(CurrentMrcData->DdrFreq == DDRFREQ_1333) {
    	Dtr3.field.tRWSR = 3;
		Dtr1.field.tRTP = MAX(TRTP, 5) - 0x3;
		Dtr3.field.tWRSR = 5;
    }

    if (CurrentMrcData->DdrFreq == DDRFREQ_800) {
        Dtr3.field.tXP = MAX(0, 1 - Dtr1.field.tCMD);
    } else {
        Dtr3.field.tXP = MAX(0, 2 - Dtr1.field.tCMD);
	}
	Dtr0.field.CKEDLY = 0x1;
	MsgBus32AndThenOr(VLV_UNIT_BUNIT, BUNIT_BISOC_OFFSET, 0xFFFFFF80, 0x10);
 }
#endif	//DDR3_SUPPORT

#if defined DDR3_SUPPORT && DDR3_SUPPORT
    if (DDR3_DETECTED) {
    	Dtr4.field.WRODTSTRT = Dtr1.field.tCMD;
    	Dtr4.field.WRODTSTOP = Dtr1.field.tCMD;
    	Dtr4.field.RDODTSTRT = Dtr1.field.tCMD + Dtr0.field.tCL - Dtr1.field.tWCL + 2;//Convert from WL (DRAM clocks)  to VLV indx
    	Dtr4.field.RDODTSTOP = Dtr1.field.tCMD + Dtr0.field.tCL - Dtr1.field.tWCL + 2;
    	Dtr4.field.TRGSTRDIS = 0;
    }
#endif

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (LPDDR3_DETECTED) {
  Dtr4.field.WRODTSTRT = 2;
  Dtr4.field.WRODTSTOP = 0;
  Dtr4.field.RDODTSTRT = 3;
  Dtr4.field.RDODTSTOP = 3;
  Dtr4.field.TRGSTRDIS = 0;
}
#endif

  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DTR0_OFFSET ,Dtr0.raw);
  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DTR1_OFFSET ,Dtr1.raw);
  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DTR2_OFFSET ,Dtr2.raw);
  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DTR3_OFFSET ,Dtr3.raw);
  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DTR4_OFFSET ,Dtr4.raw);

  return SUCCESS;
}

STATUS
ProgDdrControl (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
    RegDSCH	DSCHreg;
    RegDPMC0  DPMC0reg;
    RegDPMC1  DPMC1reg;
    RegBONUS0 BONUS0reg;
    RegDTR3   Dtr3reg;
    RegDCAL   DCALreg;
    RegDECCCTRL DECCCTRLreg;
#if defined DDR3_ECC && DDR3_ECC
    UINT32    regValue;
    if (CurrentMrcData->EccEnabled) {
    	regValue = MsgBus32Read (VLV_UNIT_BUNIT, BUNIT_BDEBUG1_OFFSET);
  	    MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BDEBUG1_OFFSET, regValue|0x1);
    }
#endif	//DDR3_ECC

    DCALreg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DCAL_OFFSET); 
    Dtr3reg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DTR3_OFFSET);
    DPMC0reg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DPMC0_OFFSET);
    DPMC1reg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DPMC1_OFFSET);
    DSCHreg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DSCH_OFFSET);
    DECCCTRLreg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DECCCTRL_OFFSET);
    BONUS0reg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_BONUS0_OFFSET);

    DPMC0reg.field.CLKGTDIS= 0x0;
    DPMC0reg.field.DISPWRDN= 0x0;
    DPMC0reg.field.PCLSTO = 0x4;
    DPMC0reg.field.PCLSWKOK = 0x0;
    DPMC0reg.field.PREAPWDEN = 0x1;
    DPMC0reg.field.PWRGATEDIS = 0x1;

    DSCHreg.field.OOODIS= 0x0;
    DSCHreg.field.OOOST3DIS= 0x1;
    DSCHreg.field.OOOAGETRH = 0x8;
    DSCHreg.field.NEWBYPDIS= 0x0;
    DSCHreg.field.IPREQMAX = 0x7;

    DPMC1reg.field.CMDTRIST = 2;
    DPMC1reg.field.CSTRIST = 1;

	if (DPMC1reg.field.CMDTRIST){
		//CMDTRIST
		//01 ?The DRAM Cmd/Addr pins are tristated only when all enabled CKE pins are low.
		//10 ?The DRAM Cmd/Addr pins are tristated when not driving a valid command

		//Reg 4840 bit 0 to 0.
		RunitMsgBusAnd((UINT32)DDRIO_CMD_CFG_REG0_OFFSET, (UINT32)(~BIT0));
		RunitMsgBusAnd((UINT32)DDRIO_CLKCTL_CMD_CFG_REG2_OFFSET, (UINT32)(~BIT0));
	}

#if defined DDR3_SUPPORT && DDR3_SUPPORT
    if (DDR3_DETECTED) {

#if defined DDR3_ECC && DDR3_ECC
    	if (CurrentMrcData->EccEnabled) {
    		DECCCTRLreg.field.ENCBGEN = 1;

            DECCCTRLreg.field.SBEEN = 1;
    	    DECCCTRLreg.field.DBEEN = 1;
    	} //if Ecc enabled
#endif	//DDR3_ECC

    	BONUS0reg.field.SLOWPDXEN = 0;
		DPMC0reg.field.ENCKTRI = 0x0;
    }
#endif

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (LPDDR3_DETECTED) {
    	DCALreg.field.TQPollEn = Dtr3reg.field.EnDeRate = 0;
    	DPMC0reg.field.ENCKTRI = 0x1;

    	if ((CurrentMrcData->DdrFreq == DDRFREQ_1066) || (CurrentMrcData->DdrFreq == DDRFREQ_1333)) {
		  	BONUS0reg.field.SLOWPDXEN = 1;
		  	DCALreg.field.TQPollPer = 7;
		  	DCALreg.field.TQPollEn = Dtr3reg.field.EnDeRate = 1;
    	}
    	BONUS0reg.field.LPDDRCMDTRI = 1;
    	CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DCAL_OFFSET ,DCALreg.raw);
    	CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DTR3_OFFSET, Dtr3reg.raw);
    }
#endif

  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_BONUS0_OFFSET ,BONUS0reg.raw);
  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DSCH_OFFSET ,DSCHreg.raw);
  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DPMC0_OFFSET ,DPMC0reg.raw);
  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DPMC1_OFFSET, DPMC1reg.raw);
  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DECCCTRL_OFFSET ,DECCCTRLreg.raw);

  return SUCCESS;
}

STATUS
DUnitBlMode (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
    RegDPMC0      DPMC0reg;

    // DUNIT BL MODE
    //0 ?FIxed BL8
    //1 ?On-the-Fly BL8 or BC4
    DPMC0reg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DPMC0_OFFSET);
    DPMC0reg.field.BLMODE = 1;

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (LPDDR3_DETECTED) {
    	DPMC0reg.field.BLMODE = 0;		//Always set BLMODE = 0 ?FIxed BL8 for LPDDR3
    	}
#endif

    if (CurrentMrcData->NumBitDRAMCap) {
        DPMC0reg.field.BLMODE = 0;
    }

    CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DPMC0_OFFSET, DPMC0reg.raw);

  return SUCCESS;
}

STATUS
ProgDdecodeBeforeJedec (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
    RegDRP  DRPreg;
    RegDRFC DRCFreg;
    RegDCAL DCALreg;
    UINT16   bmbound = 0;
    UINT32  Buffer32;
    UINT32  tsegBase;

    DRPreg.raw = 0;

    if ((CurrentMrcData->BootMode == S5Path) || (CurrentMrcData->BootMode == FBPath)){
        DRCFreg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DRFC_OFFSET);
        DRCFreg.field.tREFI=0;
        CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DRFC_OFFSET, DRCFreg.raw);
        DCALreg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DCAL_OFFSET);
        DCALreg.field.ZQCINT=0;
        DCALreg.field.SRXZQCL=0;
        CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DCAL_OFFSET ,DCALreg.raw);
    }

    //DRP DRAM Rank Population Register
	DRPreg.raw = 0;
	DRPreg.raw |= 0xF;    	//  Only for training (rank pop)
    //DDR3 = 0 , LPDDR2 = 1
    DRPreg.field.DRAMtype = CurrentMrcData->DDRType >> 2;

#if defined DDR3_SUPPORT && DDR3_SUPPORT
	if (CurrentMrcData->currentPlatformDesign == BLK_RVP_DDR3L) {
		DRPreg.field.CKECOPY = 1;
	}
#endif

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
	if (LPDDR3_DETECTED) {
        DRPreg.field.CKECOPY = 1;
        DRPreg.field.EnLPDDR3 = 1;
        DRPreg.field.rank2Enabled = 0;
        DRPreg.field.rank3Enabled = 0;
        DRPreg.field.dimm0DevWidth = 2;         //x32
        DRPreg.field.dimm0DevDensity = 2;       //4Gb
        DRPreg.field.dimm1DevWidth = 0;
        DRPreg.field.dimm1DevDensity = 0;
    }
#endif
	DRPreg.field.RANKREMAP = 1;

    CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DRP_OFFSET, DRPreg.raw);

    bmbound = 0xE00;
	//set TSEG base 1M below BMBOUND
    tsegBase = bmbound - 0x1;

    //BMBOUND [31:27] tolud
    Buffer32 = MsgBus32Read(VLV_UNIT_BUNIT, BUNIT_BMBOUND_OFFSET);
    Buffer32&= 0x07FFFFFF;
  	//Shift left 20 bit as BMBOUND value is in MB granularity
    Buffer32|= (bmbound<<20);
    MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BMBOUND_OFFSET, Buffer32);

    //BMBOUNDHI [31:24] -> [35:28]
    Buffer32 = MsgBus32Read(VLV_UNIT_BUNIT, BUNIT_BMBOUND_HI_OFFSET);
    Buffer32&= 0x00FFFFFF;
	//Shift left 24 bit as BMBOUNDHI value is in 256MB granularity
	Buffer32|= 1<<24;
    MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BMBOUND_HI_OFFSET, Buffer32);

  	//SMM
  	//BSMMRRL - Lower Bound (SMMStart) [15:0] - [35:20]
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BSMMRRL_OFFSET, tsegBase);

  	//BSMMRRH - Upper Bound (SMMEnd) [15:0]- [35:20]
  	Buffer32 = MsgBus32Read(VLV_UNIT_BUNIT, BUNIT_BSMMRRH_OFFSET);
  	Buffer32|= (tsegBase) | BIT31;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BSMMRRH_OFFSET, Buffer32);

  return SUCCESS;
}


STATUS
PerformDDR3Reset (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
    UINT32 buffer32;

    buffer32 = CurrentMrcData->DunitMsgBus32Read(DunitPortID[Channel], MC_DRMC_OFFSET);
    buffer32 |= BIT16;
    CurrentMrcData->DunitMsgBus32Write(DunitPortID[Channel], MC_DRMC_OFFSET, buffer32);

    CurrentMrcData->DunitWakeCommand();

    //default value
    buffer32 &= (~BIT16);
    CurrentMrcData->DunitMsgBus32Write(DunitPortID[Channel], MC_DRMC_OFFSET, buffer32);

    return SUCCESS;
}

STATUS
PerformJedecInit (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
/*++

Routine Description:

  Programs the Jedec init for a row of memory.

Arguments:

  CurrentMrcData:   Detected DDR timing parameters for installed memory

Returns:

  None

--*/
{
#if (defined LPDDR2_SUPPORT && LPDDR2_SUPPORT) || (defined DDR3_SUPPORT && DDR3_SUPPORT)
    UINT8   Rank;
    UINT8	DdrFreq;
#endif    
    UINT8   rnk_lu8, chn_lu8, max_chn_lu8;
    RegDTR0 DTR0reg;
    RegDTR1 DTR1reg;
    UINT32  DRPbuffer;
    UINT32  DRMCbuffer;
#if defined DDR3_SUPPORT && DDR3_SUPPORT
    DramInitMisc miscCommand;
    UINT32  rttNom = 0;
#endif
    max_chn_lu8 = 1;
	if (CurrentMrcData->DualChannelEnable) { max_chn_lu8 = MAX_CHANNELS_TOTAL; }
    DTR0reg.raw = CurrentMrcData->DunitMsgBus32Read (VLV_UNIT_DUNIT, MC_DTR0_OFFSET);
    DTR1reg.raw = CurrentMrcData->DunitMsgBus32Read (VLV_UNIT_DUNIT, MC_DTR1_OFFSET);
    DRPbuffer = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DRP_OFFSET);
    DRPbuffer &= 0xF;
    DRMCbuffer = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DRMC_OFFSET);
    DRMCbuffer &= 0xFFFFFFF0;
    DRMCbuffer |= (BIT4|DRPbuffer);
    CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DRMC_OFFSET, DRMCbuffer);

#if defined DDR3_SUPPORT && DDR3_SUPPORT
    if (CurrentMrcData->DDRType == DDRType_DDR3) {
    	for (chn_lu8 = 0; chn_lu8 < max_chn_lu8; chn_lu8++) {
    		for (rnk_lu8 = 0; rnk_lu8 < RANKS_PER_CHANNEL; rnk_lu8++) {
    		// Skip to next populated rank
    		if (CurrentMrcData->Channel[Channel].RankPresent[rnk_lu8] == 0 )
    		{ continue; }

    		miscCommand.raw = 0;
    		miscCommand.field.command = 7;
    		miscCommand.field.rankSelect = rnk_lu8;
    		DramInitCommand (DunitPortID[chn_lu8], miscCommand.raw);
    		}
    	}
    }
#endif
	//DUNIT ODT
    DRMCbuffer = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DRMC_OFFSET);
    DRMCbuffer &= 0xFFFF00E0;
    DRMCbuffer |= 0x1000;
    CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DRMC_OFFSET, DRMCbuffer);


#if defined DDR3_SUPPORT && DDR3_SUPPORT
	if (CurrentMrcData->DDRType < DDRType_DDR3All)  {

        DramInitDDR3MRS0 mrs0Command;
        DramInitDDR3EMR1 emrs1Command;
        DramInitDDR3EMR2 emrs2Command;
        DramInitDDR3EMR3 emrs3Command;

        // setup for emrs 2
        emrs2Command.raw = 0;
        emrs2Command.field.bankAddress = 2;
        emrs2Command.field.CWL = CurrentMrcData->WL - 5;
		
    	switch (CurrentMrcData->override_dram_ODT_value) {
		case DRAM_ODT_60:
			emrs2Command.raw |= (DDR3_EMRS2_RTTWR_60 << 6);
			break;
		case DRAM_ODT_120:
			emrs2Command.raw |= (DDR3_EMRS2_RTTWR_120 << 6);
			break;
		case DRAM_ODT_DEFAULT:
		default:
			emrs2Command.raw |= (DDR3_EMRS2_RTTWR_120 << 6);
			break;
    	}

      switch (CurrentMrcData->overrideAutoSelfRefreshASR) {
      case AUTO_SELF_REFRESH_ON:
      case AUTO_SELF_REFRESH_OFF:
      	emrs2Command.field.ASR = CurrentMrcData->overrideAutoSelfRefreshASR;
      	break;
      case AUTO_SELF_REFRESH_DEFAULT:
    	default:
      	emrs2Command.field.ASR = 0;
      	break;
      }



        // setup for emrs 3
        emrs3Command.raw = 0;
        emrs3Command.field.bankAddress = 3;

        // setup for emrs 1
        emrs1Command.raw = 0;
        rttNom = 0;
        emrs1Command.field.bankAddress = 1;
        emrs1Command.field.dllEnabled = 0; // 0 = Enable , 1 = Disable
        emrs1Command.field.DIC0 = 1;

#if defined DISABLED_RTT_NOM && DISABLED_RTT_NOM
        rttNom = (DDR3_EMRS1_RTTNOM_0 << 6); //multiplex address
#else
        rttNom = (DDR3_EMRS1_RTTNOM_40 << 6); //multiplex address
#endif

        if ((CurrentMrcData->DualChannelEnable == 0) && (CurrentMrcData->MemoryDown == 1)){
        	if ((CurrentMrcData->Channel[Channel].RankPresent[0] == 1) && (CurrentMrcData->Channel[Channel].RankPresent[1] == 0)){
#if defined DISABLED_RTT_NOM && DISABLED_RTT_NOM
        		rttNom = (DDR3_EMRS1_RTTNOM_0 << 6); //multiplex address
#else
        		rttNom = (DDR3_EMRS1_RTTNOM_120 << 6); //multiplex address
#endif
        	}
        }

        emrs1Command.raw |= rttNom;


        // setup for mrs 0
        mrs0Command.raw = 0;
        mrs0Command.field.bankAddress = 0;
        mrs0Command.field.dllReset = 1;

        // DUNIT BL MODE
        //0 ?FIxed BL8
        //1 ?On-the-Fly BL8 or BC4
        mrs0Command.field.BL = 1;
        if (CurrentMrcData->NumBitDRAMCap) {
        	mrs0Command.field.BL = 0;
        }

        if (CurrentMrcData->DdrFreq == DDRFREQ_800) {
        	mrs0Command.field.PPD = 0;
        } else {
        	mrs0Command.field.PPD = 1;
        }

        mrs0Command.field.casLatency = DTR0reg.field.tCL + 1;

        if (CurrentMrcData->TimingData[MRC_DATA_TWR] < 10) {
        	mrs0Command.field.writeRecovery = CurrentMrcData->TimingData[MRC_DATA_TWR] - 4 ;
        } else {
        	mrs0Command.field.writeRecovery = (CurrentMrcData->TimingData[MRC_DATA_TWR] >> 1) & 0x7;
        }

        DdrFreq = CurrentMrcData->DdrFreq - MINDDR; // Use DDR 800 to index zero

        for (chn_lu8 = 0; chn_lu8 < max_chn_lu8; chn_lu8++) {
        	for (Rank = 0; Rank < RANKS_PER_CHANNEL; Rank++) {
        		// Skip to next populated rank
        		if (CurrentMrcData->Channel[Channel].RankPresent[Rank] == 0) { continue; }

        		emrs2Command.field.rankSelect = Rank;
        		DramInitCommand (DunitPortID[chn_lu8], emrs2Command.raw);

        		emrs3Command.field.rankSelect = Rank;
        		DramInitCommand (DunitPortID[chn_lu8], emrs3Command.raw);

        		emrs1Command.field.rankSelect = Rank;
        		DramInitCommand (DunitPortID[chn_lu8], emrs1Command.raw);

        		mrs0Command.field.rankSelect = Rank;
        		DramInitCommand (DunitPortID[chn_lu8], mrs0Command.raw);

        		miscCommand.raw = 0;
        		miscCommand.field.command = 6;
        		miscCommand.field.multAddress = BIT10;
        		miscCommand.field.rankSelect = Rank;
        		DramInitCommand (DunitPortID[chn_lu8], miscCommand.raw);
        	} //end for (Rank = 0; Rank < 4; Rank++)
		} //end of chn
	}
#endif // #if DDR3_SUPPORT

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (CurrentMrcData->DDRType == DDRType_LPDDR3) {
		UINT32 dramPRECommand = 0;
	    UINT32 dramZQCALCommand = 0;
    	UINT32 dramRESETCommand = 0;
	    UINT32 mr01, mr02, mr03, tWR, RLWL=4;
	    max_chn_lu8 = 1;

        if (CurrentMrcData->DualChannelEnable) { max_chn_lu8 = MAX_CHANNELS_TOTAL; }

        for (chn_lu8 = 0; chn_lu8 < max_chn_lu8; chn_lu8++) {
	    for (rnk_lu8 = 0; rnk_lu8 < RANKS_PER_CHANNEL; rnk_lu8++) {
		    // Skip to next populated rank
	        if (CurrentMrcData->Channel[chn_lu8].RankPresent[rnk_lu8] == 0 ) { continue; }

    		/* Issue NOP for tINIT3 (>=200us) */
		    //CurrentMrcData->DunitInitCommand(VLV_UNIT_DUNIT,LPDDR3_JEDEC_CMD_NOP|JEDEC_CMD_RANK(rnk_lu8) );
	        DramInitCommand (DunitPortID[chn_lu8], LPDDR3_JEDEC_CMD_NOP|JEDEC_CMD_RANK(rnk_lu8) );
			MrcDelay (1, 250);

			dramPRECommand = (LPDDR3_JEDEC_CMD_PRECHARGE_ALL| JEDEC_CMD_RANK(rnk_lu8) );
			//precharge all
			//CurrentMrcData->DunitInitCommand (VLV_UNIT_DUNIT,dramPRECommand);
			DramInitCommand (DunitPortID[chn_lu8],dramPRECommand);
			MrcDelay (1, 200);
		    /* Issue MRW(RESET) Command */
    		dramRESETCommand = (LPDDR3_JEDEC_CMD_MRW_RESET| LPDDR3_JEDEC_CMD_MRW);
    		DramInitCommand (DunitPortID[chn_lu8], dramRESETCommand | JEDEC_CMD_RANK(rnk_lu8));

    		/* Issue NOP for at least tINIT4 (1us) */
		    //CurrentMrcData->DunitInitCommand(VLV_UNIT_DUNIT,LPDDR3_JEDEC_CMD_NOP|JEDEC_CMD_RANK(rnk_lu8) );
    		DramInitCommand(DunitPortID[chn_lu8], LPDDR3_JEDEC_CMD_NOP|JEDEC_CMD_RANK(rnk_lu8) );
    		MrcDelay (1, 20);

    		dramZQCALCommand = 0;
		    dramZQCALCommand = (LPDDR3_JEDEC_CMD_MRW + JEDEC_CMD_MR(10) + JEDEC_CMD_DATA(LPDDR3_JEDEC_CMD_POST_INIT_CAL));
    		//CurrentMrcData->DunitInitCommand (VLV_UNIT_DUNIT,dramZQCALCommand |JEDEC_CMD_RANK(rnk_lu8));
		    DramInitCommand (DunitPortID[chn_lu8], dramZQCALCommand |JEDEC_CMD_RANK(rnk_lu8));

           switch (CurrentMrcData->DdrFreq) {

           case DDRFREQ_800:
                RLWL = LPDDR3_JEDEC_MR2_RL6WL3;
                tWR = LPDDR3_JEDEC_MR1_nWR6;
             break;

           case DDRFREQ_1066:
                RLWL = LPDDR3_JEDEC_MR2_RL8WL4;
                tWR = LPDDR3_JEDEC_MR1_nWR8;
            break;

           case DDRFREQ_1333:
                RLWL = LPDDR3_JEDEC_MR2_RL10WL6;
                tWR = LPDDR3_JEDEC_MR1_nWR10;
            break;

           default:
                RLWL = LPDDR3_JEDEC_MR2_RL10WL6;
                tWR = LPDDR3_JEDEC_MR1_nWR10;
                break;
            }

    		mr01 = (LPDDR3_JEDEC_CMD_MRW + JEDEC_CMD_MR(1) + JEDEC_CMD_DATA(tWR + LPDDR3_JEDEC_MR1_BL8) );
		    mr02 = (LPDDR3_JEDEC_CMD_MRW + JEDEC_CMD_MR(2) + JEDEC_CMD_DATA(RLWL + LPDDR3_JEDEC_MR2_WRE_LT9 + LPDDR3_JEDEC_MR2_WL_SETA + LPDDR3_JEDEC_MR2_WRLVL_DIS));
    		mr03 = (LPDDR3_JEDEC_CMD_MRW + JEDEC_CMD_MR(3) + JEDEC_CMD_DATA(LPDDR3_JEDEC_MR3_OHM_40));

             /* Issue MRW(MR2) Command */
             //CurrentMrcData->DunitInitCommand (VLV_UNIT_DUNIT, mr02 | JEDEC_CMD_RANK(rnk_lu8));
    		DramInitCommand (DunitPortID[chn_lu8], mr02 | JEDEC_CMD_RANK(rnk_lu8));
             
             /* Issue NOP for at least tMRW (5 clocks) */
            //CurrentMrcData->DunitInitCommand(VLV_UNIT_DUNIT, LPDDR3_JEDEC_CMD_NOP|JEDEC_CMD_RANK(rnk_lu8) );
    		DramInitCommand (DunitPortID[chn_lu8], LPDDR3_JEDEC_CMD_NOP|JEDEC_CMD_RANK(rnk_lu8) );
			MrcDelay (1, 10);

    		/* Issue MRW(MR1) Command */
		    //CurrentMrcData->DunitInitCommand (VLV_UNIT_DUNIT, mr01 | JEDEC_CMD_RANK(rnk_lu8) );
			DramInitCommand (DunitPortID[chn_lu8], mr01 | JEDEC_CMD_RANK(rnk_lu8) );
    		
            //CurrentMrcData->DunitInitCommand(VLV_UNIT_DUNIT, LPDDR3_JEDEC_CMD_NOP|JEDEC_CMD_RANK(rnk_lu8) );
			DramInitCommand(DunitPortID[chn_lu8], LPDDR3_JEDEC_CMD_NOP|JEDEC_CMD_RANK(rnk_lu8) );

			MrcDelay (1, 10);

    		/* Issue MRW(MR3) Command */
		   // CurrentMrcData->DunitInitCommand (VLV_UNIT_DUNIT, mr03 | JEDEC_CMD_RANK(rnk_lu8));
			DramInitCommand (DunitPortID[chn_lu8], mr03 | JEDEC_CMD_RANK(rnk_lu8));
    		
 		   //CurrentMrcData->DunitInitCommand(VLV_UNIT_DUNIT,LPDDR3_JEDEC_CMD_NOP|JEDEC_CMD_RANK(rnk_lu8) );
			DramInitCommand(DunitPortID[chn_lu8],LPDDR3_JEDEC_CMD_NOP|JEDEC_CMD_RANK(rnk_lu8) );

			MrcDelay (1, 10);
            //CurrentMrcData->DunitInitCommand (VLV_UNIT_DUNIT, 0x70b0 | JEDEC_CMD_RANK(rnk_lu8));
            //Enable ODT, Rzq/1
            DramInitCommand (DunitPortID[chn_lu8], 0x70b0 | JEDEC_CMD_RANK(rnk_lu8));
 	    } //RANK_PER_SLOT
        } //CHANNEL
	}
#endif  //end of LPDDR3_SUPPORT

  return SUCCESS;
}

STATUS PerformWake (MRC_PARAMETER_FRAME *CurrentMrcData, UINT8 Channel)
{
    //wake message,  ZQCAL send by Dunit.
	CurrentMrcData->DunitWakeCommand();

    return SUCCESS;
}

STATUS
SetDDRInitializationComplete (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
  RegDCO DCOreg;

  DCOreg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DCO_OFFSET);
  DCOreg.field.PMICTL = 0;          //0 - PRI owned by BUnit
  DCOreg.field.IC = 1;              //1 - initialization complete
  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DCO_OFFSET ,DCOreg.raw);

  return SUCCESS;
}

UINT16 GGCToGMSSizeMapping[17] = { 0,32,64,96,128,160,192,224,256,288,320,352,384,416,448,480,512};
UINT16 GGCToGTTSizeMapping[3] = { 0,1,2};

STATUS
ProgMemoryMappingRegisters (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
  UINT32    MmioAllocation;
  UINT16    GMSsize = 0;
  UINT16    GTTsize;
  UINT16    TSEGsize;
  UINT32    IedSize;

//local
  UINT32  TOM;
  UINT8   ReclaimEn;
  UINT32  BMBOUND;
  UINT32  BMBOUNDHI;
  UINT32  SeCUmaBase;
#ifdef FTPM_ENABLE
  UINT32  SeCfTPMUmaBase;
#endif
  UINT32  GMSbase, GTTbase;
  UINT32  TSEGbase;
  UINT32  TempTSEGbase;
  UINT32  Buffer32;

  UINT32  GGC;
  GGC = PciCfg32Read_CF8CFC(0, 2, 0, 0x50);
  if (((GGC & (BIT7|BIT6|BIT5|BIT4|BIT3))>>3)<17) {
      GMSsize = GGCToGMSSizeMapping[(GGC & (BIT7|BIT6|BIT5|BIT4|BIT3))>>3];
  }
  else {
      return FAILURE;
  }
  if (((GGC & (BIT9|BIT8))>>8)<3) {
      GTTsize = GGCToGTTSizeMapping[(GGC & (BIT9|BIT8))>>8];
  }
  else {
      return FAILURE;
  }

  // 1 - 1MB
  //TSEGsize = 1; //CurrentMrcData->OemMrcData.TsegSize;
  TSEGsize = CurrentMrcData->OemMrcData.TsegSize;

  MmioAllocation = CurrentMrcData->OemMrcData.MmioSize;  // MMIO space in MB

  //Memory Map Programming Steps: (!!!All varaiables are in 1 MB!!!)
  //1. Determine TOM, which is defined by the total physical memory size.
  TOM = CurrentMrcData->C0MemorySize;

  //4. Determine TOLUD, which is the minimum value by comparing between "4GB minus
  //MMIO size" and "TOM minus ME size".
  BMBOUND = MIN(0x1000-MmioAllocation, TOM);

  //5. Determine whether reclaim is available. If "TOM minus ME size" is greater than "the
  //value of TOLUD" by more than 64MB, then memory reclaim is available to enable.
  BMBOUNDHI=TOM>>8;   //BMBOUNDHI value here is in 256MB granularity
  ReclaimEn = FALSE;
  if (TOM > BMBOUND) {
    ReclaimEn = TRUE;
  }

  //Note: BIOS must rounddown "TOLUD" and rounddown "TOM minus ME size" to 64MB aligned
  //if relaim is enabled.
  if (ReclaimEn) {
    BMBOUNDHI=  TOM+MmioAllocation;   		//in MB
    BMBOUNDHI= ((BMBOUNDHI&~(0xFF))>>8);    //convert to granularity of 256MB
  }

#ifdef SEC_SUPPORT_FLAG

#ifdef FTPM_ENABLE
  SeCfTPMUmaBase = BMBOUND - CurrentMrcData->SeCfTPMUmaSize;
  CurrentMrcData->SeCfTPMUmaBase = SeCfTPMUmaBase;
  SeCUmaBase = SeCfTPMUmaBase - CurrentMrcData->SeCUmaSize;
  CurrentMrcData->SeCUmaBase = SeCUmaBase;
#else
  SeCUmaBase = BMBOUND - CurrentMrcData->SeCUmaSize;
   CurrentMrcData->SeCUmaBase = SeCUmaBase;
#endif

#else
  SeCUmaBase = BMBOUND;
#ifdef FTPM_ENABLE
  SeCfTPMUmaBase = BMBOUND;
#endif
#endif
  //9. Determine GFX Memory base, which is calculated by the value of TOLUD minus GFX size.
  GMSbase = SeCUmaBase - GMSsize;
  GTTbase = GMSbase - GTTsize;
  
  //11. Determine TSEG base, which is calculated by the value of TOLUD minus GFX size minus TSEG size.
  TempTSEGbase = GTTbase - (GTTbase & TSEGsize - 1);
  TSEGbase = TempTSEGbase - TSEGsize;

///////////////////////////////////////////////////////
//Program the registers
///////////////////////////////////////////////////////

  //BMBOUND [31:27]
  Buffer32 = MsgBus32Read(VLV_UNIT_BUNIT, BUNIT_BMBOUND_OFFSET);
  Buffer32&= 0x07FFFFFF;
  //Shift left 20 bit as BMBOUND value is in MB granularity
  //Bit 0 = Send Boot Vector to DRAM:
  Buffer32|= (BMBOUND<<20);
  MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BMBOUND_OFFSET, Buffer32);

  //BMBOUNDHI [31:24] -> [35:28]
  Buffer32 = MsgBus32Read(VLV_UNIT_BUNIT, BUNIT_BMBOUND_HI_OFFSET);
  Buffer32&= 0x00FFFFFF;
  //Shift left 24 bit as BMBOUNDHI value is in 256M granularity
  Buffer32|= (BMBOUNDHI<<24);
  MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BMBOUND_HI_OFFSET, Buffer32);

  //SMM
  //BSMMRRL [15:0] - Lower Bound (SMMStart)
  //These bits are compared with bits 35:20 of the incoming address to determine the lower 1MB aligned value of the protected range.
  Buffer32 = MsgBus32Read(VLV_UNIT_BUNIT, BUNIT_BSMMRRL_OFFSET);
  Buffer32&=0x0;
  //TSEGbase is in 1MB granularity
  Buffer32|= TSEGbase;
  MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BSMMRRL_OFFSET, Buffer32);

  //BSMMRRH [15:0] - Upper Bound (SMMEnd)
  //These bits are compared with bits 35:20 of the incoming address to determine the upper 1MB aligned value of the protected range.
  //[29] - Asset Classification (AC)[0]: Trace Enable
  //[30] - Asset Classification (AC)[1]: Implicit WB Enable
  //[31] - SMM Enable
  Buffer32 = MsgBus32Read(VLV_UNIT_BUNIT, BUNIT_BSMMRRH_OFFSET);
  Buffer32&=0x0;
  //TSEGbase is in 1MB granularity
  if (CurrentMrcData->PcdCpuIEDEnabled) {
    //
    // Ensure IEDSize is 
    // 
    IedSize = CurrentMrcData->IedSize;
    IedSize = IedSize >> 0x14;
    // Ideally 
    if (IedSize <= (TSEGsize-IedSize)) {
      Buffer32|= ((TSEGbase+TSEGsize-1-IedSize) + BIT30 + BIT31);
    } 
    else {
      //
      // Something is not right here. Therefore do not proceed.
      //
      MRC_DEADLOOP();
    }
    
  } 
  else {
    Buffer32|= ((TSEGbase+TSEGsize-1) + BIT30 + BIT31);
  }
  MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BSMMRRH_OFFSET, Buffer32);

  //Bunit.BSMRRAC
  Buffer32= 0x14;
  MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BSMRRAC_OFFSET, Buffer32);

  //Bunit.BSMRWAC
  Buffer32= 0x14;
  MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BSMRWAC_OFFSET, Buffer32);

  //Internal Gfx Stolen
  Buffer32 = PciCfg32Read_CF8CFC(0, 2, 0, 0x5C);
  Buffer32&= 0xFFFFF;
  Buffer32|= GMSbase<<20;
  PciCfg32Write_CF8CFC(0, 2, 0, 0x5C,Buffer32);

  //Gfx GTT Stolen
  Buffer32 = PciCfg32Read_CF8CFC(0, 2, 0, 0x70);
  Buffer32&= 0xFFFFF;
  Buffer32|= GTTbase<<20;
  PciCfg32Write_CF8CFC(0, 2, 0, 0x70,Buffer32);

  //Bunit.BIMR0L
  Buffer32= GTTbase<<10;
  MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR0L_OFFSET, Buffer32);

  //Bunit.BIMR0H
  Buffer32= ((SeCUmaBase<<10)-1);
  Buffer32 |= BIT31;
  MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR0H_OFFSET, Buffer32);


  //Bunit.BIMR0RAC
  Buffer32= 0x600;
  MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR0RAC_OFFSET, Buffer32);

  //Bunit.BIMR0WAC
  MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR0WAC_OFFSET, Buffer32);


#ifdef SEC_SUPPORT_FLAG
#ifdef FTPM_ENABLE

  	//Bunit.BIMR1L
  	Buffer32= SeCUmaBase<<10;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR1L_OFFSET, Buffer32);

  	//Bunit.BIMR1H
  	Buffer32= (SeCfTPMUmaBase<<10)-1;
  	Buffer32 |= BIT31;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR1H_OFFSET, Buffer32);

  	//Bunit.BIMR1RAC
  	Buffer32= 0x200;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR1RAC_OFFSET, Buffer32);

  	//Bunit.BIMR1WAC
  	Buffer32= 0x200;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR1WAC_OFFSET, Buffer32);
	
  	//Bunit.BIMR1L
  	Buffer32= SeCfTPMUmaBase<<10;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR2L_OFFSET, Buffer32);

  	//Bunit.BIMR1H
  	Buffer32= ((BMBOUND<<10)-1);
  	Buffer32 |= BIT31;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR2H_OFFSET, Buffer32);

  	//Bunit.BIMR1RAC
  	Buffer32=0x29F;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR2RAC_OFFSET, Buffer32);

  	//Bunit.BIMR1WAC
  	Buffer32= 0x29F;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR2WAC_OFFSET, Buffer32);

#else

  	//Bunit.BIMR1L
  	Buffer32= SeCUmaBase<<10;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR1L_OFFSET, Buffer32);

  	//Bunit.BIMR1H
  	Buffer32= ((BMBOUND<<10)-1);
  	Buffer32 |= BIT31;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR1H_OFFSET, Buffer32);

  	//Bunit.BIMR1RAC
  	Buffer32= 0x200;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR1RAC_OFFSET, Buffer32);

  	//Bunit.BIMR1WAC
  	Buffer32= 0x200;
  	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BIMR1WAC_OFFSET, Buffer32);
	
#endif	
	
#endif	//SEC_SUPPORT_FLAG
  return SUCCESS;
}

#if 0
STATUS
ReadLPDDR3MemoryInfo (MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
    UINT32 mr08;
    RegDCAL regDCAL;
    UINT8 Socket;

    mr08 = (LPDDR3_JEDEC_CMD_MRR + JEDEC_CMD_MR(8));
	  /* Issue MRR(MR8) Command */
    DramInitCommand (DunitPortID[Channel], mr08 | JEDEC_CMD_RANK(0) );
    MrcDelay(1,10);

    regDCAL.raw = MsgBus32Read(DunitPortID[Channel], MC_DCAL_OFFSET);

    for (Socket = 0; Socket < MAX_SLOTS; Socket++) {
        if (CurrentMrcData->Channel[Channel].RankPresent[Socket*2]) {

            CurrentMrcData->Channel[Channel].DimmSize[Socket] = ((regDCAL.field.MRRData >> 2 ) & 0x7 ) - 4;
            if (CurrentMrcData->Channel[Channel].DimmSize[Socket] >= 4 ) {
                return FAILURE; //not support >8Gb
             }

            if ( ((regDCAL.field.MRRData >> 6 ) & 0x3) == 0) {
                CurrentMrcData->Channel[Channel].DimmDataWidth[Socket] = 2;  //x32
            } else {
                CurrentMrcData->Channel[Channel].DimmDataWidth[Socket] = 1;  //x16
             }
        }
    }

    return SUCCESS;
}
#endif

STATUS
ProgDraDrb (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
/*++

Routine Description:

  Programs the DRAM timing and control registers.
  Assuming MAX_SOCKETS = 4, MAX_SIDES = 2

Arguments:

  CurrentMrcData:   Detected DDR timing parameters for installed memory

Returns:

  None

--*/
{
    UINT8   Rank, Socket, Channel_local;
    UINT16  C0DRB_Total; //in 64MB
    UINT8   t_C0Rankpop;
    RegDRP  DRPreg;
    UINT16  MemSizeToUse = 0;
#if defined DDR3_SUPPORT && DDR3_SUPPORT
    UINT8   loop;
    UINT8 Device_width = 4;
#endif
    UINT8	size_index = 0;
    UINT8   index = 0;
    UINT32  *pAddressMapTable;
//  0000 = 256 Mb 0001 = 512 Mb  0010 = 1 Gb  0011 = 2 Gb
//  0100 = 4 Gb   0101 = 8 Gb    0110 = 16 Gb
    UINT16 DRAM_capacity = 256;	//Mb
//  000 = 4 bits  001 = 8 bits  010 = 16 bits  011 = 32 bits

    UINT8 NumberOfRank;
    UINT32 RankSize;
    UINT8 numBits;
    RegBTHCTRL BTHCTRLreg;
    RegDCO Dco;
    RegBunitAHASHCFG BunitAHASHCFG;

    DRPreg.raw = 0;
    BunitAHASHCFG.raw = 0;

    if (CurrentMrcData->NumBitDRAMCap) {
        numBits = 32;
    } else {
    	numBits = 64;
    }

    if ((CurrentMrcData->BootMode == S5Path) || (CurrentMrcData->BootMode == FBPath)){
        Dco.raw = MsgBus32Read(VLV_UNIT_DUNIT0, MC_DCO_OFFSET);
        Dco.field.IC = 0;              //0 - initialization complete
        CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DCO_OFFSET ,Dco.raw);
    }

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (CurrentMrcData->DDRType == DDRType_LPDDR3) {

	    if( (CurrentMrcData->SiRevisionID == 1) || (CurrentMrcData->SiRevisionID == 2) ||(CurrentMrcData->SiRevisionID == 3) ||(CurrentMrcData->SiRevisionID == 4)){
	    } else {
    	   for (Channel_local = 0; Channel_local < MAX_CHANNELS_TOTAL; Channel_local++) {
    	   	 if (CurrentMrcData->Channel[Channel_local].Enabled == 0) continue;
           }
	    }
	}
#endif  //end of LPDDR3_SUPPORT

    C0DRB_Total = 0;
    NumberOfRank = 0;
    //program dimm width bit 4 and bit 8
    //SDRAM CAPACITY / 8 * PRIMARY BUS WIDTH íA SDRAM WIDTH * RANKS
    //    in detectdimm.c: CurrentMrcData->D_Size[CurrentSocket] = (CurrentSpdData->Buffer[SPD_DDR3_DENBANK] & 0xF);
    //Program DIMM density bit 5 and bit 9

	for (Channel_local = 0; Channel_local < MAX_CHANNELS_TOTAL; Channel_local++) {
		if (CurrentMrcData->Channel[Channel_local].Enabled == 0) continue;
		t_C0Rankpop = 0;
		//both channels need to have same number of rank
	    for (Rank = 0; Rank < RANKS_PER_CHANNEL; Rank++) {
          if ( CurrentMrcData->Channel[Channel_local].RankPresent[Rank] ){ // Is the rank present?
            t_C0Rankpop |= (1 << Rank);  //from Rank0 to Rank7

          }
        }

	  	DRPreg.raw = t_C0Rankpop;
		//check if DDR3 or LPDDRx
	   	DRPreg.field.DRAMtype = CurrentMrcData->DDRType >> 2;
	   	for (Socket = 0; Socket < MAX_SLOTS; Socket++) {
			CurrentMrcData->Channel[Channel_local].SlotMem[Socket] = 0;
			if (CurrentMrcData->Channel[Channel_local].RankPresent[Socket*2]) { 
#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
				if (CurrentMrcData->DDRType == DDRType_LPDDR3) {
			    	 DRPreg.field.dimm0DevWidth = CurrentMrcData->Channel[Channel_local].DimmDataWidth[Socket];
			    	 DRPreg.field.dimm0DevDensity = CurrentMrcData->Channel[Channel_local].DimmSize[Socket];
			    	 if (Channel_local == 1) {
			    	 DRPreg.field.dimm0DevWidth = CurrentMrcData->Channel[Channel_local].DimmDataWidth[Socket];
			    	 DRPreg.field.dimm0DevDensity = CurrentMrcData->Channel[Channel_local].DimmSize[Socket];
			    	 }
			    	 NumberOfRank = CurrentMrcData->Channel[Channel_local].RankPresent[0] + CurrentMrcData->Channel[Channel_local].RankPresent[1];

			    	 CurrentMrcData->Channel[Channel_local].SlotMem[Socket] = (DRAM_capacity << (CurrentMrcData->Channel[Channel_local].DimmSize[Socket]+2))/8 * numBits/( 8 << CurrentMrcData->Channel[Channel_local].DimmDataWidth[Socket]) *NumberOfRank;
				}
#endif	//LPDDR3_SUPPORT
#if defined DDR3_SUPPORT && DDR3_SUPPORT
				if (CurrentMrcData->DDRType < DDRType_DDR3All) {

					DRPreg.raw |= ((CurrentMrcData->Channel[Channel_local].DimmDataWidth[Socket] & 0x3) << (4 + Socket*5));    //dimm width shift to bit 4 or bit 9

					DRPreg.raw |= ((CurrentMrcData->Channel[Channel_local].DimmSize[Socket] & 0x3) << (6 + Socket*5)); //dimm density shift to bit 6 and bit 11
					CurrentMrcData->Channel[Channel_local].SlotMem[Socket] = ( (DRAM_capacity << (CurrentMrcData->Channel[Channel_local].DimmSize[Socket]+2))
																	/ 8 * numBits
																	/ (Device_width << (CurrentMrcData->Channel[Channel_local].DimmDataWidth[Socket]+1)) 
																	* (CurrentMrcData->Channel[Channel_local].DimmSides[Socket]+1)); 
				}
#endif	//DDR3_SUPPORT
				C0DRB_Total = C0DRB_Total + CurrentMrcData->Channel[Channel_local].SlotMem[Socket];            
			}
    	}

		MemSizeToUse = CurrentMrcData->Channel[0].SlotMem[0];
		if (CurrentMrcData->DDRType < DDRType_DDR3All) {
			DRPreg.field.dimm0Mirror = CurrentMrcData->Channel[Channel].DimmMirror[0];
			DRPreg.field.dimm1Mirror = CurrentMrcData->Channel[Channel].DimmMirror[0];
		}

#if defined DDR3_SUPPORT && DDR3_SUPPORT
		if (CurrentMrcData->currentPlatformDesign == BLK_RVP_DDR3L) {
			DRPreg.field.CKECOPY = 1;
		}
#endif

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
		if (CurrentMrcData->DDRType == DDRType_LPDDR3) {
            DRPreg.field.EnLPDDR3 = 1;
            DRPreg.field.CKECOPY = 1;
		}
#endif  //end of LPDDR3_SUPPORT

		if (CurrentMrcData->Channel[Channel].RankPresent[0] & CurrentMrcData->Channel[Channel].RankPresent[1]) {
			DRPreg.field.RANKREMAP = 1;
		}

		DRPreg.field.RSIEN= 1;
		MsgBus32Write(DunitPortID[Channel_local], MC_DRP_OFFSET, DRPreg.raw);
	}  // channel

	BunitAHASHCFG.field.DramAddressHashEn = 1;
	BunitAHASHCFG.field.RankInterleaveEn = 1;

	if (CurrentMrcData->NumBitDRAMCap) {
	    BunitAHASHCFG.field.MemoryIntWidth = 0; //32bit
	} else {
		BunitAHASHCFG.field.MemoryIntWidth = 1; //64bit
	}
	BunitAHASHCFG.field.rank0Enabled = CurrentMrcData->Channel[0].RankPresent[0];
	BunitAHASHCFG.field.rank1Enabled = CurrentMrcData->Channel[0].RankPresent[1];
	BunitAHASHCFG.field.rank2Enabled = 0;
	BunitAHASHCFG.field.rank3Enabled = 0;

	BunitAHASHCFG.field.dimm0PartWidth = CurrentMrcData->Channel[0].DimmDataWidth[0];
	RankSize = CurrentMrcData->Channel[0].SlotMem[0]/(CurrentMrcData->Channel[0].RankPresent[0] + CurrentMrcData->Channel[0].RankPresent[1]);
	RankSize = RankSize/128;
	BunitAHASHCFG.field.dimm0Rank0Size = 0;

	if(RankSize != 1){
		do{
			RankSize = RankSize >> 1;
			BunitAHASHCFG.field.dimm0Rank0Size++;
		}while(RankSize != 1);
	}
	BunitAHASHCFG.field.dimm1PartWidth = 0;	//Don't care
	BunitAHASHCFG.field.dimm1Rank2Size = 0; //Don't care
	MsgBus32Write(VLV_UNIT_BUNIT, BUNIT_BC0AHASHCFG_OFFSET, BunitAHASHCFG.raw);

    CurrentMrcData->C0MemorySize = C0DRB_Total; //change it to 1MB

    BTHCTRLreg.raw = MsgBus32Read(VLV_UNIT_BUNIT, BUNIT_BTHCTRL_OFFSET);

#if defined DDR3_SUPPORT && DDR3_SUPPORT
    if (CurrentMrcData->DDRType < DDRType_DDR3All) {
	      size_index = (BitScanReverse8((MemSizeToUse>>8)&0xFF) );

        if (CurrentMrcData->NumBitDRAMCap == 0) {
            pAddressMapTable = &DDR3_AddressMapTable_64[0][0];
            size_index -= 1;
        }
         else
            pAddressMapTable = &DDR3_AddressMapTable_32[0][0];

        index = (UINT8)( size_index*4);
        BTHCTRLreg.raw |= ( *(pAddressMapTable + index + 3) );
        MsgBus32Write(VLV_UNIT_BUNIT, BunitAddressMap[3], BTHCTRLreg.raw);
        for (loop = 0; loop < 4; loop++) {
            MsgBus32Write(VLV_UNIT_BUNIT, BunitAddressMap[loop], *(pAddressMapTable + index + loop) );
        }
    }
#endif	//DDR3_SUPPORT

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (CurrentMrcData->DDRType == DDRType_LPDDR3) {
	      size_index = (BitScanReverse8((MemSizeToUse>>7)&0xFF) );

        if (CurrentMrcData->NumBitDRAMCap == 0) {
            pAddressMapTable = &LPDDR2_AddressMapTable_64[0][0];
            size_index -= 1;
        }
         else
            pAddressMapTable = &LPDDR2_AddressMapTable_32[0][0];

        index = (UINT8)(size_index*4);

        //rankPick
        MsgBus32Write(VLV_UNIT_BUNIT, BunitAddressMap[3], (BTHCTRLreg.raw | ( *(pAddressMapTable + index + 3) )));
        //rank
        MsgBus32Write(VLV_UNIT_BUNIT, BunitAddressMap[2], ( *(pAddressMapTable + index + 2) ));
        //row
        if ((CurrentMrcData->Channel[Channel].DimmDataWidth[DRPreg.field.dimmFlip] == 2) && (MemSizeToUse == 512)) {  //x32 && 512MB
            MsgBus32Write(VLV_UNIT_BUNIT, BunitAddressMap[0], *(pAddressMapTable + index)>>1);
            MsgBus32Write(VLV_UNIT_BUNIT, BunitAddressMap[1], *(pAddressMapTable + index + 1)|0x8);
        } else if ((CurrentMrcData->Channel[Channel].DimmDataWidth[DRPreg.field.dimmFlip] == 2) && (MemSizeToUse == 1024)) { //x32 && 1024MB
            MsgBus32Write(VLV_UNIT_BUNIT, BunitAddressMap[0], *(pAddressMapTable + index));
            MsgBus32Write(VLV_UNIT_BUNIT, BunitAddressMap[1], *(pAddressMapTable + index + 1));
        } else {
            MsgBus32Write(VLV_UNIT_BUNIT, BunitAddressMap[0], *(pAddressMapTable + index));
            MsgBus32Write(VLV_UNIT_BUNIT, BunitAddressMap[1], *(pAddressMapTable + index + 1));
        }
    }
#endif	//LPDDR3_SUPPORT

    if ((CurrentMrcData->BootMode == S5Path) || (CurrentMrcData->BootMode == FBPath)){
        Dco.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DCO_OFFSET);
        Dco.field.IC = 1;              //0 - initialization complete
        CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DCO_OFFSET ,Dco.raw);
    }
  return SUCCESS;
}

STATUS
ChangeSelfRefreshSetting (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
 	RegDRFC DRFCreg;
	RegDCAL DCALreg;
	RegDPMC0 DPMC0reg;
	UINT32 DRMCbuffer;

	//DUNIT ODT
    DRMCbuffer = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DRMC_OFFSET);
    DRMCbuffer &= 0xFFFF00E0;
#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (CurrentMrcData->DDRType == DDRType_LPDDR3) {
    	DRMCbuffer |= 0x1000;
    }
#endif
#if defined DDR3_SUPPORT && DDR3_SUPPORT
if (CurrentMrcData->DDRType  < DDRType_DDR3All) {
		DRMCbuffer |= 0x0000;
	}
#endif
    CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DRMC_OFFSET, DRMCbuffer);

	DRFCreg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DRFC_OFFSET);

#if defined DDR3_SUPPORT && DDR3_SUPPORT
	if (DDR3_DETECTED) {
		DRFCreg.field.tREFI = 0x3;
	}
#endif
#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
		if (LPDDR3_DETECTED) {
			DRFCreg.field.tREFI = 0x2;
		}
#endif
	DRFCreg.field.REFWMLO = 0xD;
	DRFCreg.field.REFWMHI = 0xD;
	DRFCreg.field.REFWMPNC = 0xD;
	DRFCreg.field.REFCNTMAX = 0x2;
	DRFCreg.field.REFSKEWDIS = 0x0;
	DRFCreg.field.REFDBTCLR = 0x1;

	CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DRFC_OFFSET, DRFCreg.raw);

	DCALreg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DCAL_OFFSET);
	DCALreg.field.ZQCINT=3;
	CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DCAL_OFFSET ,DCALreg.raw);

	DPMC0reg.raw = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_DPMC0_OFFSET);

	DPMC0reg.field.ENPHYCLKGATE = 0x1;
	DPMC0reg.field.SREntryDelay = 0x40;
	DPMC0reg.field.DYNSREN = 0x1;
	DPMC0reg.field.PMOPCODE = 0x7;
	DPMC0reg.field.ENCORECLKGATE = 0x1;

	CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DPMC0_OFFSET ,DPMC0reg.raw);

	return SUCCESS;
}

STATUS
ClearSelfRefresh (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
	UINT32 buffer32;

	// clear the PMSTS Channel Self Refresh bits
	buffer32 = CurrentMrcData->DunitMsgBus32Read(VLV_UNIT_DUNIT, MC_PMSTS_OFFSET);
	CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_PMSTS_OFFSET, buffer32|BIT0);

	return SUCCESS;
}

STATUS ProgSFRVolSel (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	if ((CurrentMrcData->currentPlatformDesign == BLK_RVP_DDR3L) || (CurrentMrcData->currentPlatformDesign == BBY_25x27_4LAYERS_DDR3L_MEMDOWN)){
		MMRCSFRVolSel1CH (&(CurrentMrcData->ModMrcData), Channel);
	} else {
		MMRCSFRVolSel (&(CurrentMrcData->ModMrcData), Channel);
	}
    return SUCCESS;
}

STATUS ProgMpllSetup (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
    MMRCPLLInit (&(CurrentMrcData->ModMrcData), Channel);
    return SUCCESS;
}

STATUS ProgStaticDdrSetup (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
    MMRCDDRStaticInit2 (&(CurrentMrcData->ModMrcData), Channel);
#if defined DDR3_ECC && DDR3_ECC
    if (CurrentMrcData->EccEnabled) {
    	CurrentMrcData->ModMrcData.Channel[1].Enabled = 1;
    	MMRCDDRStaticInit2 (&(CurrentMrcData->ModMrcData), 1);
    }
#endif	//DDR3_ECC
	if ((CurrentMrcData->currentPlatformDesign == BLK_RVP_DDR3L) || (CurrentMrcData->currentPlatformDesign == BBY_25x27_4LAYERS_DDR3L_MEMDOWN)) {
		MMRCPI2XDB_1CH (&(CurrentMrcData->ModMrcData), Channel);
	}
#if defined DDR3_SUPPORT && DDR3_SUPPORT
	if (CurrentMrcData->currentPlatformDesign == BBY_25x27_RAMBI_DDR3L_MEMDOWN) {
		MMRCRxPow (&(CurrentMrcData->ModMrcData), Channel);
	}
#endif
   return SUCCESS;
}

STATUS ProgStaticInitPerf (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	MMRCDDRStaticInitPerf (&(CurrentMrcData->ModMrcData), Channel);
#if defined DDR3_ECC && DDR3_ECC
    if (CurrentMrcData->EccEnabled) {
    	MMRCDDRStaticInitPerf (&(CurrentMrcData->ModMrcData), 1);
    }
#endif	//DDR3_ECC
   return SUCCESS;
}
STATUS ProgStaticPwrClkGating (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	MMRCDDRStaticPwrClkGating (&(CurrentMrcData->ModMrcData), Channel);
#if defined DDR3_ECC && DDR3_ECC
    if (CurrentMrcData->EccEnabled) {
    	MMRCDDRStaticPwrClkGating (&(CurrentMrcData->ModMrcData), 1);
    }
#endif	//DDR3_ECC
   return SUCCESS;
}

STATUS ControlDDR3Reset (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
#if defined DDR3_SUPPORT && DDR3_SUPPORT
    if (DDR3_DETECTED) {
    	MMRCDDR3Reset  (&(CurrentMrcData->ModMrcData), Channel);
	}
#endif
	return SUCCESS;
}

STATUS EnableVreg (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
    MMRCDLLInit (&(CurrentMrcData->ModMrcData), Channel);
#if defined DDR3_ECC && DDR3_ECC
    if (CurrentMrcData->EccEnabled) {
    	MMRCDLLInit (&(CurrentMrcData->ModMrcData), 1);
    }
#endif	//DDR3_ECC
    return SUCCESS;
}

STATUS ProgHmc (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	if (CurrentMrcData->ModMrcData.FeatureSettings.MrcDigitalDll ==  0) {
		MMRCHMCInit (&(CurrentMrcData->ModMrcData), Channel);
	}
    return SUCCESS;
}

STATUS ProgReadWriteFifoPtr (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
    MMRCWrPointerInit (&(CurrentMrcData->ModMrcData), Channel);
    return SUCCESS;
}

STATUS ProgComp(MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
#if defined DDR3_SUPPORT && DDR3_SUPPORT
  if((CurrentMrcData->currentPlatformDesign == BLK_RVP_DDR3L) || (CurrentMrcData->currentPlatformDesign == BBY_25x27_4LAYERS_DDR3L_MEMDOWN)){
    MMRCCompInit1_1CH (&(CurrentMrcData->ModMrcData), Channel);
  } else {
    if ((DDR3_DETECTED) && (CurrentMrcData->DualChannelEnable == 0)){
      MMRCCompInit1_1CH (&(CurrentMrcData->ModMrcData), Channel);
    } else {
      MMRCCompInit1 (&(CurrentMrcData->ModMrcData), Channel);
    }
  }
#else
  MMRCCompInit1 (&(CurrentMrcData->ModMrcData), Channel);
#endif

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if ((CurrentMrcData->Channel[Channel].RankPresent[0] == 1) && (CurrentMrcData->Channel[Channel].RankPresent[1] == 0)){
    	if (CurrentMrcData->DDRType == DDRType_LPDDR3 && CurrentMrcData->DdrFreq == DDRFREQ_1333){
    		MMRCCompInit_LP31333 (&(CurrentMrcData->ModMrcData), Channel);
    	}
    }
#endif	//LPDDR3_SUPPORT

#if defined DDR3_SUPPORT && DDR3_SUPPORT
    if(CurrentMrcData->MemoryDown == 1){
		switch (CurrentMrcData->currentPlatformDesign){
		case BBY_25x27_RAMBI_DDR3L_MEMDOWN:
		case BBY_25x27_DDR3L_MEMDOWN:
		case BBY_25x27_4LAYERS_DDR3L_MEMDOWN:
    		MMRC3LMB_COMP (&(CurrentMrcData->ModMrcData), Channel);
			break;
		default:
    		 break;
        }
		
    	if (CurrentMrcData->currentPlatformDesign == BBY_25x27_RAMBI_DDR3L_MEMDOWN) {
    		MMRCCPUODT (&(CurrentMrcData->ModMrcData), Channel);
    	} else {
        	switch (CurrentMrcData->override_cpu_ODT_value) {
    		case CPU_ODT_60:
    			RunitMsgBusAndThenOr((UINT32)0x6918, (UINT32)(0xFFFF80FF), 0x00001A00);
    			RunitMsgBusAndThenOr((UINT32)0x6A18, (UINT32)(0xFFFF80FF), 0x00001A00);
    			break;
    		case CPU_ODT_80:
    			RunitMsgBusAndThenOr((UINT32)0x6918, (UINT32)(0xFFFF80FF), 0x00001100);
    			RunitMsgBusAndThenOr((UINT32)0x6A18, (UINT32)(0xFFFF80FF), 0x00001100);
    			break;
    		case CPU_ODT_100:
    			RunitMsgBusAndThenOr((UINT32)0x6918, (UINT32)(0xFFFF80FF), 0x00000600);
    			RunitMsgBusAndThenOr((UINT32)0x6A18, (UINT32)(0xFFFF80FF), 0x00000600);
    			break;
    		case CPU_ODT_120:
    			RunitMsgBusAndThenOr((UINT32)0x6918, (UINT32)(0xFFFF80FF), 0x00002200);
    			RunitMsgBusAndThenOr((UINT32)0x6A18, (UINT32)(0xFFFF80FF), 0x00002200);
    			break;
    		case CPU_ODT_150:
    			RunitMsgBusAndThenOr((UINT32)0x6918, (UINT32)(0xFFFF80FF), 0x00002B00);
    			RunitMsgBusAndThenOr((UINT32)0x6A18, (UINT32)(0xFFFF80FF), 0x00002B00);
    			break;
    		case CPU_ODT_DEFAULT:
        default:
    		 break;
        	}
    	}
    }
#endif

    MMRCCompInit2 (&(CurrentMrcData->ModMrcData), Channel);

    return SUCCESS;
}

STATUS ProgComp2(MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
    MMRCCompInit2 (&(CurrentMrcData->ModMrcData), Channel);
    return SUCCESS;
}

STATUS SetIOBUFACT (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
    MMRCIOBUFACTInit (&(CurrentMrcData->ModMrcData), Channel);
    return SUCCESS;
}

STATUS PreJedecInit (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
#if defined DDR3_ECC && DDR3_ECC
    if (CurrentMrcData->EccEnabled) {
	CurrentMrcData->ModMrcData.Channel[1].Enabled = 0;
    }
#endif	//DDR3_ECC
    MMRCPreJedecInit (&(CurrentMrcData->ModMrcData), Channel);
    return SUCCESS;
}

STATUS MMRC_RcvnTrain (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	return ReceiveEnable (&(CurrentMrcData->ModMrcData), Channel);
}

STATUS MMRC_RcvnRestore (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
    return ReceiveEnableRestore (&(CurrentMrcData->ModMrcData), Channel);
}

STATUS MMRC_WrLvlFineTrain (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
    return FineWriteLeveling (&(CurrentMrcData->ModMrcData), Channel);
}


STATUS MMRC_WrLvlCoarseTrain (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
    return CoarseWriteLeveling (&(CurrentMrcData->ModMrcData), Channel);
}

STATUS MMRC_WrLvlRestore (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
    return WriteLevelingRestore (&(CurrentMrcData->ModMrcData), Channel);
}

STATUS MMRC_RdVrefTrain (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	return ReadVrefTraining (&(CurrentMrcData->ModMrcData), Channel);
}

STATUS MMRC_RdTrain (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	return ReadTraining (&(CurrentMrcData->ModMrcData), Channel);
}
STATUS MMRC_WrTrain (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	return WriteTraining (&(CurrentMrcData->ModMrcData), Channel);
}
STATUS MMRC_RdTrainRestore (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	return ReadTrainRestore (&(CurrentMrcData->ModMrcData), Channel);
}

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
STATUS MMRC_CATrain (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (LPDDR3_DETECTED) {
    return LPDDR3_CATraining (&(CurrentMrcData->ModMrcData), Channel);
    } else
#endif
    return SUCCESS;
}

STATUS MMRC_CATrainingRestore (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (LPDDR3_DETECTED) {
	CATrainingRestore (&(CurrentMrcData->ModMrcData), Channel);
    }
#endif
	return SUCCESS;
}

STATUS MMRC_LateCATrain (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (LPDDR3_DETECTED) {
    return LPDDR3_LateCATraining (&(CurrentMrcData->ModMrcData), Channel);
    } else
#endif
    return SUCCESS;
}
#endif	//LPDDR3_SUPPORT

STATUS MMRC_PerformanceSetting (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	PerformanceSetting (&(CurrentMrcData->ModMrcData), Channel);

    return SUCCESS;
}

STATUS MMRC_PowerGatingSetting (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	MMRCDDRStaticPowerGatingEnable (&(CurrentMrcData->ModMrcData), Channel);

#if defined DDR3_SUPPORT && DDR3_SUPPORT
	if ((DDR3_DETECTED) && (CurrentMrcData->DualChannelEnable == 0) ){
		MMRCDDRStaticInit1CH (&(CurrentMrcData->ModMrcData), Channel);
	}
#endif
	if(CurrentMrcData->NumBitDRAMCap){
		MMRCCH0_X32 (&(CurrentMrcData->ModMrcData), Channel);
		MMRCCH1_X32 (&(CurrentMrcData->ModMrcData), Channel);
	}

    return SUCCESS;
}

STATUS WarmResetMPLLBypass (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	if ((CurrentMrcData->DdrFreq == DDRFREQ_800) && (CurrentMrcData->ModMrcData.FeatureSettings.MrcDigitalDll == 0) ){
		MMRCWarmResetMPLLBypass (&(CurrentMrcData->ModMrcData), Channel);
	}

	return SUCCESS;
}
STATUS MMRC_SearchRmt (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	SearchRmt (&(CurrentMrcData->ModMrcData), Channel);
    return SUCCESS;
}

#if defined DDR3_ECC && DDR3_ECC
STATUS MMRC_InitializeMemory (
    MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel)
{
	if (CurrentMrcData->EccEnabled) {
        CPGC_InitializeMemory  (&(CurrentMrcData->ModMrcData), Channel);
	}
	return SUCCESS;
}
#endif

STATUS MMRC_Init(MRC_PARAMETER_FRAME *CurrentMrcData, UINT8 Chan){
	UINT8 Channel = 0;
	UINT8 temp;

	CurrentMrcData->ModMrcData.CpuFrequency = MmrcCpuFrequencyRead();
	CurrentMrcData->ModMrcData.CATraining_tsc_count = 0;
	CurrentMrcData->ModMrcData.CATraining_tsc_duration_low = 0;
	CurrentMrcData->ModMrcData.CATraining_tsc_duration_hgh = 0;
	CurrentMrcData->ModMrcData.CurrentFrequency = CurrentMrcData->DdrFreq;
	CurrentMrcData->ModMrcData.CurrentDdrType = CurrentMrcData->DDRType >> 1;
	CurrentMrcData->ModMrcData.CurrentPlatform = CurrentMrcData->currentPlatform;
	CurrentMrcData->ModMrcData.CurrentConfiguration = CurrentMrcData->Channel[0].DimmConfigChannel;
	CurrentMrcData->ModMrcData.EccEnabled = CurrentMrcData->EccEnabled;
	CurrentMrcData->ModMrcData.SiRevisionID = CurrentMrcData->SiRevisionID;
	CurrentMrcData->ModMrcData.FeatureSettings.MrcDigitalDll = 0;
	CurrentMrcData->ModMrcData.PatternMode = LFSR_VICAGG;
// AMI_OVERRIDE - (EIP134470+) Follow Intel document #524172 to change two parameters when using RMT. >>
// AMI_OVERRIDE - Create a token to enable \ disable MRC debug message. - (P121412A+)>>
#if defined(MRC_DEBUG_SUPPORT) && MRC_DEBUG_SUPPORT
	  CurrentMrcData->ModMrcData.MrcDebugMsgLevel = SDBG_VERBOSE;
	  CurrentMrcData->ModMrcData.FeatureSettings.MrcRMTSupport = MRC_RMT_SUPPORT;
#else
	  CurrentMrcData->ModMrcData.MrcDebugMsgLevel = SDBG_NONE;
	  CurrentMrcData->ModMrcData.FeatureSettings.MrcRMTSupport = 0;
#endif
// AMI_OVERRIDE - Create a token to enable \ disable MRC debug message. - (P121412A+)<<
	  CurrentMrcData->ModMrcData.FeatureSettings.MrcBurstLegthMode = 1;
#if defined(MRC_RMT_SUPPORT) && MRC_RMT_SUPPORT
	  CurrentMrcData->ModMrcData.FeatureSettings.MrcCPGCNumBursts = 12;
	  CurrentMrcData->ModMrcData.FeatureSettings.MrcCPGCExpLoopCnt = 14;
#else
    CurrentMrcData->ModMrcData.FeatureSettings.MrcCPGCNumBursts = 6;
    CurrentMrcData->ModMrcData.FeatureSettings.MrcCPGCExpLoopCnt = 4;
#endif
// AMI_OVERRIDE - (EIP134470 +) Follow Intel document #524172 to change two parameters when using RMT.<<
	if (CurrentMrcData->SiRevisionID <= 4) {
		CurrentMrcData->ModMrcData.FeatureSettings.MrcDigitalDll = 0;
	} else {
		if (CurrentMrcData->DDRType == DDRType_LPDDR3) {
			CurrentMrcData->ModMrcData.FeatureSettings.MrcDigitalDll = 1;
		}
	}

	if (CurrentMrcData->DDRType == DDRType_LPDDR3 && CurrentMrcData->DdrFreq == DDRFREQ_800 && CurrentMrcData->ModMrcData.FeatureSettings.MrcDigitalDll == 1) {
		CurrentMrcData->ModMrcData.CurrentFrequency = FREQ_800_DDLL_BYP_MPLL;
		CurrentMrcData->ModMrcData.CurrentDdrType = TYPE_LPDDR3;
	} else if (CurrentMrcData->DDRType == DDRType_LPDDR3 && CurrentMrcData->DdrFreq == DDRFREQ_1333){
		CurrentMrcData->ModMrcData.CurrentFrequency = FREQ_1333;
		CurrentMrcData->ModMrcData.FeatureSettings.MrcDigitalDll = 0;
	} else if (CurrentMrcData->DDRType == DDRType_LPDDR3 && CurrentMrcData->DdrFreq == DDRFREQ_1066 && CurrentMrcData->ModMrcData.FeatureSettings.MrcDigitalDll == 1) {
		CurrentMrcData->ModMrcData.CurrentFrequency = FREQ_1066_DDLL;
	} else if (CurrentMrcData->DDRType < DDRType_DDR3All) {
		CurrentMrcData->ModMrcData.FeatureSettings.MrcDigitalDll = 0;
		CurrentMrcData->ModMrcData.CurrentFrequency = CurrentMrcData->DdrFreq;
	} else {
		CurrentMrcData->ModMrcData.CurrentFrequency = CurrentMrcData->DdrFreq;
	}

	if (CurrentMrcData->EccEnabled) {
		CurrentMrcData->ModMrcData.MaxDq = 9;
	} else {
		CurrentMrcData->ModMrcData.MaxDq = 8;
	}

	if (CurrentMrcData->NumBitDRAMCap) {
		CurrentMrcData->ModMrcData.MaxDq = 4;
		CurrentMrcData->ModMrcData.FeatureSettings.MrcBurstLegthMode = 0;
	}

	for (Channel = 0; Channel < MAX_CHANNELS_TOTAL; Channel++) {
		//CurrentMrcData->ModMrcData.CurrentConfiguration = CurrentMrcData->Channel[Channel].DimmConfigChannel;
		CurrentMrcData->ModMrcData.Channel[Channel].Enabled = CurrentMrcData->Channel[Channel].Enabled;
		CurrentMrcData->ModMrcData.Channel[Channel].TimingData[TCL] = CurrentMrcData->Tcl;

		for (temp = 0; temp < MAX_SLOTS; temp++)
			CurrentMrcData->ModMrcData.Channel[Channel].DimmFrequency[temp] = CurrentMrcData->DdrFreq;

		for (temp = 0; temp < RANKS_PER_CHANNEL; temp++) {
			CurrentMrcData->ModMrcData.Channel[Channel].RankEnabled[temp] = CurrentMrcData->Channel[Channel].RankPresent[temp];
		}
	}
	CurrentMrcData->ModMrcData.MemoryDown = CurrentMrcData->MemoryDown;

	return SUCCESS;
}

MRC_TASK_FUNCTION_DESCRIPTOR ConfigureMemoryTasks[];

STATUS
DispatchFunctionDescriptors (
  MRC_TASK_FUNCTION_DESCRIPTOR  *MrcTasks,
  MRC_PARAMETER_FRAME           *CurrentMrcData
  )
{
  STATUS                        Status;
  MRC_TASK_FUNCTION_DESCRIPTOR  *i;
  UINT8                         BootMode;
  UINT16						PostCode;
  UINT8 Channel = 0;
  UINT8	temp;
  UINT8 RevId;

  BootMode = CurrentMrcData->BootMode ;

  RevId = PciCfg8Read_CF8CFC(0, 31, 0, 0x08);
  CurrentMrcData->SiRevisionID = RevId;

  //VLV A0/A1 #4599624 #4682237
  if( (RevId == 1) || (RevId == 2) ||(RevId == 3) ||(RevId == 4)){
	  if (BootMode == S0Path) {
	  BootMode = CurrentMrcData->BootMode = S5Path;
	  }
  }

  for (i = MrcTasks; (i->TaskFunctionPtr != NULL); i++ ){
    if ((BootMode & i->BootModesToExecute) != 0) {
    	PostCode = (((i->Checkpoint)&0xF)<<8) | (CurrentMrcData->OemMrcData.MrcConfigMemProgressCodeBase+((i->Checkpoint)>>4));
    	if (i->Checkpoint != 0xFF) {
    		CHECKPOINT(PostCode);
    	}


      for (temp = 0; temp < MAX_CHANNELS_TOTAL; temp++) {
        CurrentMrcData->ModMrcData.Channel[temp].TimingData[TWCL] = CurrentMrcData->WL;
		CurrentMrcData->ModMrcData.Channel[temp].TimingData[TCL] = CurrentMrcData->Tcl;
		//only have 1 slot per channel:
		CurrentMrcData->ModMrcData.Channel[temp].TotalMem = CurrentMrcData->Channel[temp].SlotMem[0];
      }

      // Invoke task function
      if (i->Channel == CH_ALL) {
        for (Channel = 0; Channel < MAX_CHANNELS_TOTAL; Channel++) {
          if (!CurrentMrcData->Channel[Channel].Enabled) {
            continue;
          }
          Status = (* (i->TaskFunctionPtr) ) (CurrentMrcData, Channel);
          if (Status == FAILURE) {
            //
            // Expect the task returning failure to explain itself in ErrorLog
            //
        	MRC_DEADLOOP();
            //return FAILURE;
          }
        }
      } else {
    	  Channel = 0;
        Status = (* (i->TaskFunctionPtr) ) (CurrentMrcData, Channel);
        if (Status == FAILURE) {
          //
          // Expect the task returning failure to explain itself in ErrorLog
          //
          MRC_DEADLOOP();
          //return FAILURE;
        }
      }
      BootMode = CurrentMrcData->BootMode;
    }
  }

  return SUCCESS;
}

STATUS
ConfigureMemory (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  )
{
  STATUS    Status;

  // Get all platform data needed for MRC
  Status = GetPlatformSettings (CurrentMrcData);
  if (Status == FAILURE) {
    return FAILURE;
  }

  // Write to PCI Bus0 Dev0 Fun0 offset 0xF0 to indicate the MRC version number
  PciCfg32Write_CF8CFC(0, 0, 0, 0xF0, MRC_VERSION);

  // Determine Platform State
  DetermineBootMode (CurrentMrcData);

  // Execute Configure Memory Tasks
  Status = DispatchFunctionDescriptors ( (MRC_TASK_FUNCTION_DESCRIPTOR*)(&ConfigureMemoryTasks), CurrentMrcData );

  if (Status == FAILURE) {
    return FAILURE;
  }
  return Status;
}

MRC_TASK_FUNCTION_DESCRIPTOR ConfigureMemoryTasks[] = {
  {           0xFF, (S5Path|FBPath|S0Path|S3Path), &FillInputStructure,				CH_NONE},
  {           0xFF, (S5Path|FBPath|S0Path|S3Path), &MMRC_Init,						CH_NONE},
  {           0xFF, (S5Path|FBPath|S0Path|S3Path), &McEnableHPET,					CH_NONE},
  { (0x1<<4)|(0x1), (S5Path|FBPath|S0Path|S3Path), &ClearSelfRefresh,				CH_NONE},
  {           0xFF, (S5Path|FBPath|S0Path|S3Path), &OemTrackInitComplete,			CH_NONE},
  { (0x2<<4)|(0x1), (S5Path|FBPath|       S3Path), &ProgSFRVolSel,					CH_NONE},
  { (0x3<<4)|(0x1), (S5Path|FBPath|S0Path|S3Path), &ProgDdrTimingControl,			CH_NONE},
  { (0x3<<4)|(0x2), (S5Path|FBPath|S0Path|S3Path), &ProgBunit,						CH_NONE},
  { (0x4<<4)|(0x1), (S5Path|FBPath|       S3Path), &ProgMpllSetup,					CH_NONE},
  { (0x4<<4)|(0x2), (S5Path|FBPath|       S3Path), &ProgStaticDdrSetup,				CH_ALL},
  { (0x4<<4)|(0x3), (S5Path|FBPath|       S3Path), &ProgStaticInitPerf,				CH_ALL},
  { (0x4<<4)|(0x4), (S5Path|FBPath|       S3Path), &ProgStaticPwrClkGating,			CH_ALL},
  { (0x4<<4)|(0x5), (S5Path|FBPath|S0Path|S3Path), &DUnitBlMode,					CH_NONE},
  { (0x5<<4)|(0x1), (S5Path|FBPath              ), &ControlDDR3Reset,				CH_NONE},
  { (0x5<<4)|(0x2), (S5Path|FBPath|       S3Path), &EnableVreg,						CH_ALL},
  { (0x5<<4)|(0x3), (S5Path|FBPath|       S3Path), &ProgHmc,						CH_NONE},
  { (0x5<<4)|(0x4), (S5Path|FBPath|       S3Path), &ProgReadWriteFifoPtr,			CH_NONE},
  { (0x5<<4)|(0x5), (S5Path|FBPath|       S3Path), &ProgComp,						CH_NONE},
  { (0x5<<4)|(0x7), (              S0Path       ), &ProgComp2,						CH_NONE},
  { (0x5<<4)|(0x8), (S5Path|FBPath|       S3Path), &SetIOBUFACT,					CH_NONE},
  { (0x6<<4)|(0x1), (S5Path|FBPath              ), &ProgDdecodeBeforeJedec,			CH_NONE},
  { (0x7<<4)|(0x1), (S5Path|FBPath              ), &PerformDDR3Reset,				CH_NONE},
  { (0x7<<4)|(0x2), (S5Path|FBPath|S0Path|S3Path), &PreJedecInit,					CH_NONE},
  { (0x7<<4)|(0x3), (S5Path|FBPath              ), &PerformJedecInit,				CH_NONE},
  { (0x9<<4)|(0x1), (S5Path|FBPath              ), &SetDDRInitializationComplete,	CH_NONE},
#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
  { (0xA<<4)|(0x1), (S5Path                     ), &MMRC_CATrain,					CH_ALL},
  { (0xA<<4)|(0x1), (       FBPath|S0Path|S3Path), &MMRC_CATrainingRestore, 	    CH_ALL},
#endif
  { (0x7<<4)|(0x3), (S5Path                     ), &PerformJedecInit,				CH_NONE},
  { (0xD<<4)|(0x1), (S5Path|FBPath|S0Path|S3Path), &DisableRank2RankSwitching,		CH_NONE},
  { (0xA<<4)|(0x2), (S5Path                     ), &MMRC_RcvnTrain,					CH_ALL},
  { (0xA<<4)|(0x3), (       FBPath|S0Path|S3Path), &MMRC_RcvnRestore,				CH_ALL},
  { (0xA<<4)|(0x4), (S5Path                     ), &MMRC_WrLvlFineTrain,			CH_ALL},
  { (0xA<<4)|(0x5), (S5Path                     ), &MMRC_WrLvlCoarseTrain,			CH_ALL},
  { (0xA<<4)|(0x6), (       FBPath|S0Path|S3Path), &MMRC_WrLvlRestore,				CH_ALL},
  { (0xB<<4)|(0x1), (S5Path                     ), &MMRC_RdVrefTrain,				CH_ALL},
  { (0xB<<4)|(0x2), (S5Path                     ), &MMRC_RdTrain,					CH_ALL},
  { (0xB<<4)|(0x3), (S5Path                     ), &MMRC_WrTrain,					CH_ALL},
  { (0xB<<4)|(0x4), (       FBPath|S0Path|S3Path), &MMRC_RdTrainRestore,			CH_ALL},
#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
  { (0xB<<4)|(0x7), (S5Path                     ), &MMRC_LateCATrain,				CH_ALL},
#endif
  { (0xC<<4)|(0x1), (S5Path|FBPath|S0Path|S3Path), &MMRC_PerformanceSetting,      	CH_ALL},
  { (0xC<<4)|(0x2), (S5Path|FBPath       |S3Path), &MMRC_PowerGatingSetting,     	CH_ALL},
  { (0xC<<4)|(0x3), (S5Path|FBPath          	), &MMRC_SearchRmt,                 CH_ALL},
  { (0xD<<4)|(0x1), (S0Path       				), &WarmResetMPLLBypass,			CH_ALL},
  { (0xD<<4)|(0x2), (S5Path|FBPath|S0Path|S3Path), &ProgDraDrb,						CH_NONE},
  { (0xD<<4)|(0x3), (S5Path|FBPath|S0Path|S3Path), &ProgMemoryMappingRegisters,		CH_NONE},
  { (0xD<<4)|(0x6), (S5Path|FBPath|S0Path|S3Path), &ProgDdrControl,            		CH_NONE},
  { (0xE<<4)|(0x2), (S5Path|FBPath|S0Path|S3Path), &SetScrambler,					CH_NONE},
  { (0xE<<4)|(0x3), (              S0Path|S3Path), &SetDDRInitializationComplete,	CH_NONE},
  { (0xE<<4)|(0x4), (              S0Path|S3Path), &PerformWake,					CH_NONE},
#if defined DDR3_ECC && DDR3_ECC
  { (0xE<<4)|(0x5), (S5Path|FBPath|S0Path       ), &MMRC_InitializeMemory,			CH_NONE},
#endif  
  { (0xE<<4)|(0x6), (S5Path|FBPath|S0Path|S3Path), &ChangeSelfRefreshSetting,		CH_NONE},
  { (0xF<<4)|(0x1), (S5Path|FBPath|S0Path|S3Path), &SetInitDone,					CH_NONE},
  {           0xFF, (S5Path|FBPath|S0Path|S3Path), &McDisableHPET,					CH_NONE},
  {           0xFF, (S5Path|FBPath|S0Path|S3Path), &FillOutputStructure,			CH_NONE},
  { 0, 0, NULL}
};
