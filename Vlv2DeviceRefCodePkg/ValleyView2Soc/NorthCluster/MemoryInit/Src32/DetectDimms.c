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

  DetectDimms.c

Abstract:

  Memory Detection and spd programming for Valleyview.

--*/

#include "DetectDimms.h"
#include "IoAccess.h"
#include "OemHooks.h"
#ifdef ECP_FLAG
#include "EdkIIGluePeim.h"
#else
#include <Library/DebugLib.h>
#endif
#include "../Mmrc/IpBlocks/VLVA0/Include/MMRCProjectLibraries.h"
#include <Library/MemoryDownLib.h>  //AMI_OVERRIDE - EIP168616 memory down function
 
MRC_DETECTDIMM_TASK_FUNCTION DetectDimmTasks[] = {
//  &GetSpdData,
  &FindCoreFrequency,
  &IdentifyDimms,
  &FindTclTacTClk,
  &FindTrasTrpTrcd,
  &CalcDimmConfig,
  NULL
};

UINT8 DDR3_RelevantSpdBytes[] = {
	SPD_MEMORY_TYPE         , //2     	// Memory type
	SPD_DDR3_MODULE         , //3       // Module type (3:0)
	SPD_DDR3_DENBANK        , //4       // Density (6:4), Banks (3:0)
	SPD_DDR3_ADDRESS        , //5       // Row (5:3), Column (2:0) address
	SPD_DDR3_VOLTAGE		, //6		    // [2]:1 - 1.2xV operable (DDR3L), [1]:1 - 1.35V operable (DDR3U), [0]:0 - 1.5V operable (DDR3)
	SPD_DDR3_ORG            , //7       // Ranks (5:3),device width (2:0)
	SPD_DDR3_WIDTH          , //8       // Bus width ext (4:3) 00 - No bit ext, 01 = 8Bit(ECC) Ext, Bus width (2:0)
	SPD_DDR3_MTBDD          , //10      // Medium Timebase (MTB) Dividend
	SPD_DDR3_MTBDS          , //11      // Medium Timebase (MTB) Divisor
	SPD_DDR3_TCLK           , //12      // Minimum cycle time (tCKmin)
	SPD_DDR3_CLL            , //14      // CAS latency supported, low byte
	SPD_DDR3_CLH            , //15      // CAS latency supported, high byte
	SPD_DDR3_TAA            , //16      // Minimum CAS latency time (tAA)
	SPD_DDR3_TWR            , //17      // Minimum write recovery time (tWR)
	SPD_DDR3_TRCD           , //18      // Minimum RAS to CAS time (tRCD)
	SPD_DDR3_TRRD           , //19      // Minimum RA to RA time (tRRD)
	SPD_DDR3_TRP            , //20      // Minimum precharge time (tRP)
	SPD_DDR3_TRASRC         , //21      // Upper nibbles for tRAS (7:4), tRC (3:0)
	SPD_DDR3_TRAS           , //22      // Minimum active to precharge (tRAS)
	SPD_DDR3_TRC            , //23      // Minimum active to active/refresh (tRC)
	SPD_DDR3_TRFCL          , //24      // Minimum refresh recovery (tRFC), low byte
	SPD_DDR3_TRFCH          , //25      // Minimum refresh recovery (tRFC), high byte
	SPD_DDR3_TWTR           , //26      // Minimum internal wr to rd cmd (tWTR)
	SPD_DDR3_TRTP           , //27      // Minimum internal rd to pc cmd (tRTP)	
	SPD_DDR3_TFAWH          , //28      // Upper Nibble for tFAW
	SPD_DDR3_TFAWL          , //29      // Minimum Four Activate Window Delay Time (tFAWmin), Least Significant Byte
	SPD_DDR3_ADD_MAPPING    , //63      // Address Mapping (Odd Rank Mirror)
	SPD_DDR3_MANUFACTURER_ID_LO,    //117
    SPD_DDR3_MANUFACTURER_ID_HI,    //118
    SPD_DDR3_MANUFACTURE_LOCATION,  //119
    SPD_DDR3_MANUFACTURE_DATE_LO,  //120
    SPD_DDR3_MANUFACTURE_DATE_HI,  //121
    SPD_DDR3_SERIAL_NUMBER_1,      //122
    SPD_DDR3_SERIAL_NUMBER_2,      //123
    SPD_DDR3_SERIAL_NUMBER_3,      //124
    SPD_DDR3_SERIAL_NUMBER_4,      //125
};

UINT16 FrequencyMultiplier[C_MAXDDR] = {
//  0.001 ns granularity
  2500, // 800MHz  (2.500 ns)
  1875, // 1066MHz (1.875 ns)
  1500, // 1333MHz (1.500 ns)
  1250  // 1600MHz (1.250 ns)
};
UINT16 TaaMin[C_MAXDDR] = {
//  0.001 ns granularity
    12500, // 800MHz  (12.500 ns)
    11250, // 1066MHz (11.250 ns)
    10500, // 1333MHz (10.500 ns)
    10000  // 1600MHz (10.000 ns)
};

UINT32 tFAWmin[C_MAXDDR][2] = {
 //1KB, 2KB
  {320, 400},  // 800MHz
  {300, 400},  //1066MHz
  {240, 360},  //1333MHz
  {240, 320}   //1600MHz
};

UINT8 tRASmin[C_MAXDDRTYPE][C_MAXDDR] = 
{
// DDR3
  {15, 20, 24, 28},
//LPDDR3
  {17, 23,28,32}  //place holder only
};

STATUS FillDimmsParam (MRC_PARAMETER_FRAME *CurrentMrcData, MRC_DRAM_INPUT Dram_Input)
{
    UINT8 Rank, Socket, Channel=0;
#if defined DDR3_SUPPORT && DDR3_SUPPORT
    UINT8 DDRType_index;
#endif

	if ( Dram_Input.DIMM_MemDown) {
    	CurrentMrcData->DDRType = (UINT8)(Dram_Input.DRAM_Type);
    }

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
    if (CurrentMrcData->DDRType == DDRType_LPDDR3) {
        FindCoreFrequency (CurrentMrcData);
        CurrentMrcData->DdrFreq = CurrentMrcData->DdrFreqCap;

        if ( Dram_Input.DIMM_MemDown) {
            for (Channel = 0; Channel < MAX_CHANNELS_TOTAL; Channel++) {
            	if (Dram_Input.Rank_En[Channel][0]) {
                  CurrentMrcData->Channel[Channel].Enabled = 1;
              }
              
              for (Rank = 0; Rank< RANKS_PER_CHANNEL; Rank++) {
                CurrentMrcData->Channel[Channel].RankPresent[Rank] = Dram_Input.Rank_En[Channel][Rank];
              }
    
              for (Socket = 0; Socket < MAX_SLOTS; Socket++) {
                CurrentMrcData->Channel[Channel].DimmPresent[Socket] = CurrentMrcData->Channel[Channel].RankPresent[Socket];
    
                CurrentMrcData->Channel[Channel].DimmDataWidth[Socket] = (UINT8)(Dram_Input.DIMM_DWidth[Channel][Socket]);
                CurrentMrcData->Channel[Channel].DimmSize[Socket] = (UINT8)(Dram_Input.DIMM_Density[Channel][Socket]);
                CurrentMrcData->Channel[Channel].DimmSides[Socket] = (UINT8)(Dram_Input.DIMM_Sides[Channel][Socket]);
              }
            }
          CurrentMrcData->MemoryDown = 1;
        } else {
            //BayLake
              	//1 rank, x32 width, 4Gb density
                CurrentMrcData->Channel[0].Enabled = 1;
                //2ch
                CurrentMrcData->Channel[1].Enabled = 1;

                for (Rank = 0; Rank < CurrentMrcData->RankIndex; Rank++) {
                	CurrentMrcData->Channel[0].RankPresent[Rank] = 1;
                	CurrentMrcData->Channel[1].RankPresent[Rank] = 1;
                }

                for (Socket = 0; Socket < MAX_SLOTS; Socket++) {
                	CurrentMrcData->Channel[0].DimmDataWidth[Socket] = 2;
                	CurrentMrcData->Channel[0].DimmSize[Socket] = 2;
                	CurrentMrcData->Channel[1].DimmDataWidth[Socket] = 2;
                	CurrentMrcData->Channel[1].DimmSize[Socket] = 2;
                }

                CurrentMrcData->TimingData[MRC_DATA_TRAS] = tRASmin[1][CurrentMrcData->DdrFreq];
                CurrentMrcData->MemoryDown = 0;
        }
    }
#endif	//LPDDR3_SUPPORT

#if defined DDR3_SUPPORT && DDR3_SUPPORT
    if (CurrentMrcData->DDRType < DDRType_DDR3All) {
    //DDR3L
    	    FindCoreFrequency (CurrentMrcData);
		    for (Channel = 0; Channel < MAX_CHANNELS_TOTAL; Channel++) {
		      for (Rank = 0; Rank< RANKS_PER_CHANNEL; Rank++) {
		        CurrentMrcData->Channel[Channel].RankPresent[Rank] = Dram_Input.Rank_En[Channel][Rank];
		      }

		      if (Dram_Input.Rank_En[Channel][0]) {
		          CurrentMrcData->Channel[Channel].Enabled = 1;
		      }
		
		      for (Socket = 0; Socket < MAX_SLOTS; Socket++) {
		        CurrentMrcData->Channel[Channel].DimmPresent[Socket] = CurrentMrcData->Channel[Channel].RankPresent[Socket];
		
		        CurrentMrcData->Channel[Channel].DimmDataWidth[Socket] = (UINT8)(Dram_Input.DIMM_DWidth[Channel][Socket]);
		        CurrentMrcData->Channel[Channel].DimmSize[Socket] = (UINT8)(Dram_Input.DIMM_Density[Channel][Socket]);
		        CurrentMrcData->Channel[Channel].DimmSides[Socket] = (UINT8)(Dram_Input.DIMM_Sides[Channel][Socket]);
		        CurrentMrcData->Channel[Channel].DimmBusWidth[Socket] = (UINT8)(Dram_Input.DIMM_BusWidth[Channel][Socket]);
		      }
		    }
		
		    CurrentMrcData->DdrFreq = (UINT8)(Dram_Input.DRAM_Speed);
		    CurrentMrcData->DDRType = (UINT8)(Dram_Input.DRAM_Type);
		
		    if(CurrentMrcData->DDRType < DDRType_DDR3All){
		    	DDRType_index = 0;
		    }else{
		    	DDRType_index = 1;
		    }
		
		    CurrentMrcData->Tcl = (UINT8)(Dram_Input.tCL);
		
		    CurrentMrcData->TimingData[MRC_DATA_TRP] = CurrentMrcData->TimingData[MRC_DATA_TRCD] = (UINT8)(Dram_Input.tRP_tRCD);
		    CurrentMrcData->TimingData[MRC_DATA_TRAS] = tRASmin[DDRType_index & 0x3][CurrentMrcData->DdrFreq];
		    CurrentMrcData->TimingData[MRC_DATA_TWR] = (UINT8)(Dram_Input.tWR);
		    CurrentMrcData->TimingData[MRC_DATA_TWTR] = (UINT8)(Dram_Input.tWTR);
		    CurrentMrcData->TimingData[MRC_DATA_TRRD] = (UINT8)(Dram_Input.tRRD);
		    CurrentMrcData->TimingData[MRC_DATA_TRTP] = (UINT8)(Dram_Input.tRTP);
		    CurrentMrcData->TimingData[MRC_DATA_TFAW] = (UINT8)(Dram_Input.tFAW);
    		CurrentMrcData->MemoryDown = 1;
    }
#endif	//DDR3_SUPPORT

    return SUCCESS;
}

STATUS
DetectDimms (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  )
{
  UINTN  i;
  UINTN  buffer;
  STATUS Status;
  UINT8  Channel;
  UINT32 IoBase;
  
  //
  // Configure GPIO muxes for SMBUS
  //
  IoBase = Mmio32Read (0xe00f804c) & 0xFFFFC000;

  Mmio32Write (IoBase + 0x5A0, 0x2003CC81);

  Mmio32Write (IoBase + 0x580, 0x2003CC81);

  Mmio32Write (IoBase + 0x5C0, 0x2003CC81);

  Status = GetPlatformSettings (CurrentMrcData);
  if (Status == FAILURE) {
    return FAILURE;
  }

  //
  // Initialize the data needed for DetectDimms.
  //
  for (Channel = 0; Channel < MAX_CHANNELS_TOTAL; Channel++) {
	CurrentMrcData->TotalDimm[Channel] = 0;
    //DEBUG ((EFI_D_INFO, "Calling GetSpdData function  \n"));
    Status = GetSpdData (CurrentMrcData, Channel, &DDR3_RelevantSpdBytes[0], ((sizeof DDR3_RelevantSpdBytes)/(sizeof DDR3_RelevantSpdBytes[0])));
    if (Status == FAILURE) {
      //AMI_OVERRIDE - EIP144915 if error occurred, the expected behavior return status and execute "MRC_PEI_REPORT_ERROR_CODE" >>
      return FAILURE;
      //  MRC_DEADLOOP();
      //AMI_OVERRIDE - EIP144915 if error occurred, the expected behavior return status and execute "MRC_PEI_REPORT_ERROR_CODE" <<
    }
  }
  
  for (i = 0; DetectDimmTasks[i] != (UINT32) NULL; i++) {
      Status = DetectDimmTasks[i](CurrentMrcData);

      if (Status == FAILURE) {
    	  //DEBUG ((EFI_D_ERROR, "fail at function %d \n",i));
    	  //MRC_DEADLOOP(); //AMI_OVERRIDE - EIP165363 if no memory connected, the expected behavior return status.
    	  buffer=i;
    	  return FAILURE;
      }
  }

  return Status;
}



STATUS
IdentifyDimms (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  )
/*++

Routine Description:

This function uses SPD data to detect the presence of unbuffered DDR DIMMs

Arguments:

SpdData:      The SPD data for the sockets

CurrentMrcData:   Detected DDR timing parameters for installed memory


Returns:

None

--*/
{
  UINT8 					CurrentSocket;
  MEMINIT_SPD_DATA			*CurrentSpdData;
  UINT8						Sides = 0;
  UINT8						DimmPopulation = 0;
  UINT8 					voltage = 0xFF;
  UINT8  Channel;

  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
//    if (!CurrentMrcData->Channel[Channel].Enabled) {
//     continue;
//    }
    for (CurrentSocket = 0; CurrentSocket < MAX_SLOTS; CurrentSocket++) {
        CurrentSpdData = &(CurrentMrcData->SpdData[CurrentSocket+(Channel*MAX_SLOTS)]);

        if ( CurrentSpdData->SpdPresent == FALSE ) {
            CurrentMrcData->Channel[Channel].RankPresent[CurrentSocket*2] = FALSE;
            CurrentMrcData->Channel[Channel].RankPresent[CurrentSocket*2+1] = FALSE;
            if (Channel == 1){
            	CurrentMrcData->Channel[Channel].Enabled = 0;
            }
              //DEBUG ((EFI_D_INFO, "C%d.D%d: Identify Dimm false.\n",Channel, CurrentSocket));
            continue; //keep the following check
        }
        DimmPopulation++;
        CurrentMrcData->Channel[Channel].RankPresent[(CurrentSocket*2)] = TRUE;
        CurrentMrcData->Channel[Channel].Enabled = 1;
//;-----------------------------------------------------------------------;
//; STEP 4: Verify that all DIMMs present are DDR2 or DDR3 DIMMs.
//;  See Section 5.2.1 for information on the SDRAM Type field of the SPD.
//;-----------------------------------------------------------------------;
        switch ( CurrentSpdData->Buffer[SPD_MEMORY_TYPE]) {
        case SPD_DDR3: //DDR3 
            CurrentMrcData->DDRType = DDRType_DDR3;	
        break;
        default:       //Not support DIMMS
        	//DEBUG (( EFI_D_INFO, "C%d.D%d: Dimm not supported d\n",Channel, CurrentSocket));
        return FAILURE;
        break;
        }

        //get voltage
        voltage &= (CurrentSpdData->Buffer[SPD_DDR3_VOLTAGE] & (BIT2|BIT1|BIT0));

//;-----------------------------------------------------------------------;
//; STEP 5: Verify that all DIMMs present are Un-buffered SODIMM.
//;  See Section 5.2.1 for information on the SDRAM Type field of the SPD.
//;-----------------------------------------------------------------------;

	    if ( ((CurrentSpdData->Buffer[SPD_DDR3_MODULE] & SPD_DDR3_MTYPE_MASK) !=  SPD_DDR3_SODIMM)
	         && ((CurrentSpdData->Buffer[SPD_DDR3_MODULE] & SPD_DDR3_MTYPE_MASK) !=  SPD_DDR3_ECCSODIMM) ){
        //DEBUG (( EFI_D_ERROR, "not supported dimm type\n"));

        return FAILURE;
        }

//;-----------------------------------------------------------------------;
//; Verify DIMMs Module Nominal Voltage Level, VDD
//[2]:1 - 1.2xV operable (DDR3L), [1]:1 - 1.35V operable (DDR3U), [0]:0 - 1.5V operable (DDR3)
//A value on bits 2~0 of 000 implies that the device supports nominal operable voltage of 1.5 V only.
//A value on bits 2~0 of 010 implies that the device supports nominal operable voltages of 1.35 V and 1.5 V.
//A value on bits 2~0 of 110 implies that the device supports nominal operable voltages of 1.2X V, 1.35 V, or 1.5 V.
//A value on bits 2~0 of 111 implies that the device supports nominal operable voltages of 1.2X V or 1.35 V. The device is furthermore endurant to 1.5 V.
//;-----------------------------------------------------------------------;
	    //check for DDR3/3L/3U
	    switch (voltage) {
	    case 0:
	    	CurrentMrcData->DDRType = DDRType_DDR3;
	    	//DEBUG (( EFI_D_INFO, "C%d.D%d: DDR3 SDRAM Memory type\n",Channel, CurrentSocket));
	    	break;

	    case 2:
	    case 3:
		case 6:
	    case 7:
	    	CurrentMrcData->DDRType = DDRType_DDR3L;
	    	//DEBUG (( EFI_D_INFO, "C%d.D%d: DDR3L SDRAM Memory type\n",Channel, CurrentSocket));
	    	if ((CurrentSpdData->Buffer[SPD_DDR3_MODULE] & SPD_DDR3_MTYPE_MASK) ==  SPD_DDR3_ECCSODIMM) {
	    		CurrentMrcData->DDRType = DDRType_DDR3ECC;
		    	//DEBUG (( EFI_D_INFO, "C%d.D%d: DDR3L ECC SDRAM Memory type\n",Channel, CurrentSocket));
	    	}

	    	break;
/*
	    case 4:
	    case 5:
	    	CurrentMrcData->DDRType = DDRType_DDR3U;
	    	DEBUG (( EFI_D_INFO, "C%d.D%d: DDR3U SDRAM Memory type\n",Channel, CurrentSocket));
	    	break;
*/
	    default:
	    	CurrentMrcData->DDRType = DDRType_DDR3;
	    	//DEBUG (( EFI_D_INFO, "C%d.D%d: DDR3 SDRAM Memory type\n",Channel, CurrentSocket));
	    	break;
	    }

	    if( (CurrentMrcData->DDRType != DDRType_DDR3L) && (CurrentMrcData->DDRType != DDRType_DDR3ECC) ){
	    	DEBUG ((EFI_D_ERROR, "Warning! System is booted with unsupported memory type in channel %d!\n", Channel));
	    	DEBUG ((EFI_D_ERROR, "Please reboot the system with supported memory type!\n"));
			CHECKPOINT(0x1E);
	    	MRC_DEADLOOP();
	    }

	    if ( ((CurrentMrcData->EccEnabled == 1) && (CurrentMrcData->DDRType != DDRType_DDR3ECC)) ||
	    	 ((CurrentMrcData->EccEnabled == 0) && (CurrentMrcData->DDRType == DDRType_DDR3ECC)) )	{
		    DEBUG ((EFI_D_ERROR, "Warning! System and the DIMM should both have ECC supported or both with nonECC!\n"));
		    DEBUG ((EFI_D_ERROR, "Please reboot the system with matching memory type!\n"));
			CHECKPOINT(0x1E);
	    	MRC_DEADLOOP();
	    }


//;-----------------------------------------------------------------------;
//; STEP 6:  On Valleyview systems, if ECC DIMMs are detected, the System BIOS
//; will HALT the system (ECC DIMMs are not supported). If both ECC
//; and non-ECC DIMMs are detected in the system, the System BIOS should not
//; allow the system to boot. See Section 5.2.1 (DDR2) or
//; Section 5.2.2 (DDR3) for information on the Module Configuration
//; Type field of the SPD.
//;-----------------------------------------------------------------------;
    //if ((CurrentSpdData->Buffer[SPD_DDR3_WIDTH] & SPD_DDR3_BWE_MASK) != 0) {
      //ECC DIMM
      //return FAILURE;
        CurrentMrcData->Channel[Channel].DimmECC[CurrentSocket] = (CurrentSpdData->Buffer[SPD_DDR3_WIDTH] & SPD_DDR3_BWE_MASK) >> 3;
    //}

//;-----------------------------------------------------------------------;
//; STEP 8: For DDR2, verify that all DIMMs are x8 or x16 width
//; and the number of banks is 4 or 8.  See Section 5.2.1 for
//; information on the SDRAM Width field of the SPD.
//; For DDR3, verify that all DIMMs are x8 or x16 width and
//; the numbers of banks is 8.  See Section 5.2.2 for information
//; on SDRAM width field and number of banks field.
//;-----------------------------------------------------------------------;
    
        CurrentMrcData->Channel[Channel].DimmDataWidth[CurrentSocket] = (CurrentSpdData->Buffer[SPD_DDR3_ORG] & 0x7) - 1;
        CurrentMrcData->Channel[Channel].DimmBanks[CurrentSocket] = ((CurrentSpdData->Buffer[SPD_DDR3_DENBANK] & 0x70) == 0) ? 1 : 0;
    
        if ((CurrentSpdData->Buffer[SPD_DDR3_DENBANK] & 0xF) < 2){
			//DEBUG (( EFI_D_ERROR, "not supported denbank %x\n",CurrentSpdData->Buffer[SPD_DDR3_DENBANK]));
            return FAILURE;
        }
    
        CurrentMrcData->Channel[Channel].DimmSize[CurrentSocket] = (CurrentSpdData->Buffer[SPD_DDR3_DENBANK] & 0xF) - 2;
        CurrentMrcData->Channel[Channel].DimmColumnAddressLines[CurrentSocket] = (CurrentSpdData->Buffer[SPD_DDR3_ADDRESS] & 0x7);
        CurrentMrcData->Channel[Channel].DimmRowAddressLines[CurrentSocket] = ((CurrentSpdData->Buffer[SPD_DDR3_ADDRESS] >> 3) & 0x7);
        Sides = (CurrentSpdData->Buffer[SPD_DDR3_ORG] >> 3) & 0x07; 

	    CurrentMrcData->Channel[Channel].DimmBusWidth[CurrentSocket] = (CurrentSpdData->Buffer[SPD_DDR3_WIDTH] & 0x7);
        if (CurrentMrcData->Channel[Channel].DimmBanks[CurrentSocket] > 1) // i.e. not 4 or 8 banks
        return FAILURE;

        if (CurrentMrcData->Channel[Channel].DimmDataWidth[CurrentSocket] > 1) //i.e. not x8/x16
        return FAILURE;

        if (CurrentMrcData->Channel[Channel].DimmSize[CurrentSocket] > 3) //i.e. not 1Gb/ 2Gb/ 4Gb / 8Gb
        return FAILURE;

        CurrentMrcData->Channel[Channel].DimmSides[CurrentSocket] = Sides;
        if (Sides > 0) {
            CurrentMrcData->Channel[Channel].RankPresent[(CurrentSocket*2)+1] = TRUE;
        }

        CurrentMrcData->Channel[Channel].DimmMirror[CurrentSocket] = CurrentSpdData->Buffer[SPD_DDR3_ADD_MAPPING] & 0x1;

//;-----------------------------------------------------------------------;
//; STEP 7: Verify that all DIMMs are single-sided or double-sided.
//;  See Section 5.2.1 for information on the SDRAM Type field of the SPD.
//;-----------------------------------------------------------------------;
        if (Sides > 1) {
		//DEBUG (( EFI_D_ERROR, "more than sides %x\n",Sides));
        return FAILURE;
        }

    }//end the for loop for each CurrentSocket
  } //if Channel

    // Check for memory present
    if (DimmPopulation != 0) {
    //DEBUG (( EFI_D_ERROR, " success \n"));
        return SUCCESS;
    } else {
    //DEBUG (( EFI_D_ERROR, "fail \n"));
        return FAILURE;
    }
}

STATUS
FindTclTacTClk (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  )
/*++

Routine Description:

  This function uses SPD data to determine the timings for the memory channel

Arguments:

  SpdData:      The SPD data for the sockets

  CurrentMrcData:   Detected DDR timing parameters for installed memory

Returns:

   None
--*/
{
    UINT8             CurrentSocket;
    UINT8             MyCasLat;
    UINT8             SupportedCasLat;
    UINT8             LowCasLat;
    UINT8             MemFreq;
    UINT8             CommonCasLat;
    MEMINIT_SPD_DATA  *CurrentSpdData;
    UINT8           CLdesired;
    UINT16          tckminall;
    UINT16          taaminall;
    UINT16          tck;
    UINT16          taa;
    UINT8	        i;
    UINT16            MtbInPs;
    UINT8             Channel;

    CommonCasLat    = 0;
    SupportedCasLat = 0xFE; // CL4 through CL10 supported by the MCH (ddr3)

//;-----------------------------------------------------------------------;
//; STEP 9: Verify a common CAS latency is supported by all DIMMs and the MCH.
//;  See Section 5.2.1 for information on the SDRAM Type field of the SPD.
//;-----------------------------------------------------------------------;

  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    if (!CurrentMrcData->Channel[Channel].Enabled) {
      continue;
    }
    for (CurrentSocket = 0;  CurrentSocket < MAX_SLOTS; CurrentSocket++) {
        CurrentSpdData = &(CurrentMrcData->SpdData[CurrentSocket+(Channel*CurrentSocket)]);

        if (CurrentMrcData->Channel[Channel].RankPresent[CurrentSocket*2])  {
            SupportedCasLat	&= CurrentSpdData->Buffer[SPD_DDR3_CLL];
        }
    }
  }
  
    if (SupportedCasLat == 0) {
      SupportedCasLat = BIT6;  // CL6
    }

//;-----------------------------------------------------------------------;
//;  Step 10: Determine a common frequency and CAS latency that can be supported. Frequencies
//;  supported by the chipset may be limited by the MCH CAPID0 register D0.F0.R 0E0h [xx:xx].
//;-----------------------------------------------------------------------;
    MemFreq = CurrentMrcData->DdrFreqCap;
    // Chipset is capable of all memory frequencies <= MAX_DDR_FREQ
    if (MemFreq > MAXDDR) {
        MemFreq = MAXDDR;
    }

    //
    // Try each supported Frequency start with highest performance
	// Setup initial tAA and tCK min values based on the max frequency of the chipset.
	// Then cycle thru the dimms looking for the worst case value.
	tckminall = FrequencyMultiplier[MemFreq];
	taaminall = TaaMin[MemFreq-MINDDR];

    for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
        if (!CurrentMrcData->Channel[Channel].Enabled) {
            continue;
        }
	    for (CurrentSocket = 0; CurrentSocket < MAX_SLOTS; CurrentSocket++) {
		
		    CurrentSpdData = &(CurrentMrcData->SpdData[CurrentSocket+(Channel*MAX_SLOTS)]);
		    //MtbInPs = 1000 * CurrentSpdData->Buffer[SPD_DDR3_MTBDD] / CurrentSpdData->Buffer[SPD_DDR3_MTBDS];

    		if (CurrentMrcData->Channel[Channel].RankPresent[CurrentSocket*2]) {
		      MtbInPs = 1000 * CurrentSpdData->Buffer[SPD_DDR3_MTBDD] / CurrentSpdData->Buffer[SPD_DDR3_MTBDS];
			    tck = CurrentSpdData->Buffer[SPD_DDR3_TCLK] * MtbInPs;
			    if( tck > tckminall ) {
				    tckminall = tck;
		    	}
    			taa = CurrentSpdData->Buffer[SPD_DDR3_TAA] * MtbInPs;
		    	if( taa > taaminall ) {
				    taaminall = taa;
			    }
		    }
	    } //for dimm
    }  //for channel

	// Look for non JEDEC standard tCK value. Round up if found.
	for( i = MINDDR; i < MAXDDR; i++ ) {
		if( tckminall == FrequencyMultiplier[i] )
			break;
		if( tckminall > FrequencyMultiplier[i] ) {
			if (i) 
			    --i;
			tckminall = FrequencyMultiplier[i];
			break;
		}
	}
	MemFreq = i;

	// Desired CL is taa/tck rounded up to nearest integer.
	CLdesired = (UINT8) (taaminall / tckminall);
	if (taaminall % tckminall) {
		CLdesired++;
	}

	LowCasLat = 0;	// Initialize to satisfy compiler checking

	// Round desired CL up to next supported value.
	MyCasLat = SupportedCasLat;
	while (MyCasLat != 0) {
		LowCasLat = BitScanReverse8(MyCasLat) + ADJUST_TCL;
		if( LowCasLat == CLdesired )
			break;
		MyCasLat &= ~ (UINT8) (1 << BitScanReverse8 (MyCasLat));
	}

	// Check for (CL * Tck) <= 20ns
	if( (LowCasLat * FrequencyMultiplier[MemFreq]) > 20000 )  {
		return FAILURE;
    }

	CurrentMrcData->DdrFreq = MemFreq;
	CurrentMrcData->Tcl = LowCasLat;  
    //DEBUG (( EFI_D_ERROR, "calc freq and tcl %d.\n",MemFreq));
    return SUCCESS;
}

STRUCT_TIMING_DATA TrasTrpTrcdTbl[MRC_DATA_MAX_TIMING_DATA] = {
  { MRC_DATA_TRAS, MAX_SUPPORTED_TRAS, MIN_SUPPORTED_TRAS, SPD_DDR3_TRAS,  SPD_DDR3_TRASRC, 0x0F },
  { MRC_DATA_TRP,  MAX_SUPPORTED_TRP,  MIN_SUPPORTED_TRP,  SPD_DDR3_TRP,   0,               0    },
  { MRC_DATA_TRCD, MAX_SUPPORTED_TRCD, MIN_SUPPORTED_TRCD, SPD_DDR3_TRCD,  0,               0    },
  { MRC_DATA_TWR,  MAX_SUPPORTED_TWR,  MIN_SUPPORTED_TWR,  SPD_DDR3_TWR,   0,               0    },
  { MRC_DATA_TRFC, MAX_SUPPORTED_TRFC, MIN_SUPPORTED_TRFC, SPD_DDR3_TRFCL, SPD_DDR3_TRFCH,  0xFF },
  { MRC_DATA_TWTR, MAX_SUPPORTED_TWTR, MIN_SUPPORTED_TWTR, SPD_DDR3_TWTR,  0,               0    },
  { MRC_DATA_TRRD, MAX_SUPPORTED_TRRD, MIN_SUPPORTED_TRRD, SPD_DDR3_TRRD,  0,               0    },
  { MRC_DATA_TRTP, MAX_SUPPORTED_TRTP, MIN_SUPPORTED_TRTP, SPD_DDR3_TRTP,  0,               0    },
  { MRC_DATA_TFAW, MAX_SUPPORTED_TFAW, MIN_SUPPORTED_TFAW, SPD_DDR3_TFAWL, SPD_DDR3_TFAWH,  0x0F }  
};

STATUS
FindTrasTrpTrcd (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  )
/*++

Routine Description:

  This function uses SPD data to determine timings.

Arguments:

  SpdData:      The SPD data for the sockets

  CurrentMrcData:   Detected DDR timing parameters for installed memory

Returns:

  None

--*/
{
  UINT8                      CurrentSocket;
  MEMINIT_SPD_DATA           *CurrentSpdData;
  UINT8                      FoundGoodValue;
  UINT8                      j;
  UINT8                      i;
  UINT32                     Temp=0;
  UINT32                     MinSPDVal[MRC_DATA_MAX_TIMING_DATA];
  STRUCT_TIMING_DATA         *TimingCalcPtr = NULL;
  UINT32                     ProposedTime;
  UINT8                      DdrFreqInx = CurrentMrcData->DdrFreq-MINDDR;
  UINT16                     MtbInPs = 0;
  UINT8    PageSize_Check = 0;
  UINT8    Channel;

  memset(MinSPDVal, 0, sizeof(MinSPDVal));

  for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    if (!CurrentMrcData->Channel[Channel].Enabled) {
      continue;
    }
    for (CurrentSocket = 0; CurrentSocket < MAX_SLOTS; CurrentSocket++) {
        if (CurrentMrcData->Channel[Channel].RankPresent[CurrentSocket*2] == 0) {
            continue; //skip the non-present DIMMs
        }
        CurrentSpdData = &(CurrentMrcData->SpdData[CurrentSocket +(Channel*MAX_SLOTS)]);
        MtbInPs = 1000 * CurrentSpdData->Buffer[SPD_DDR3_MTBDD] / CurrentSpdData->Buffer[SPD_DDR3_MTBDS];

        for (j = 0; j< MRC_DATA_MAX_TIMING_DATA; j++) {
            Temp  = MtbInPs * ((((CurrentSpdData->Buffer[TrasTrpTrcdTbl[j].DDR3_HighSPDByte] & TrasTrpTrcdTbl[j].DDR3_HighSPDByteMask) << 8)
                           +CurrentSpdData->Buffer[TrasTrpTrcdTbl[j].DDR3_LowSPDByte]));		

    	    MinSPDVal[j] = MAX(MinSPDVal[j], Temp);
        }
        if ((CurrentMrcData->Channel[Channel].DimmDataWidth[CurrentSocket]  + CurrentMrcData->Channel[Channel].DimmColumnAddressLines[CurrentSocket] ) > 1){
            PageSize_Check =1;
        }
    }
  }  //for channel

  //check for tFAW min 
  if (MinSPDVal[MRC_DATA_TFAW] < (tFAWmin[DdrFreqInx][PageSize_Check]*MtbInPs)) {
      MinSPDVal[MRC_DATA_TFAW] = tFAWmin[DdrFreqInx][PageSize_Check] * MtbInPs;
  }
 
  //;-----------------------------------------------------------------------;
  //; STEP 11: Determine the smallest common tRAS for all DIMMs. See Section 5.3.3 for details.
  //; STEP 12: Determine the smallest common tRP for all DIMMs. See Section 5.3.4 for details.
  //; STEP 13: Determine the smallest common tRCD for all DIMMs. See Section 5.3.5 for details.                              
  //; STEP 15: Verify all DIMMs support burst length of 8. See Section 5.2.1 for details.
  //; STEP 16: Determine the smallest common tWR for all DIMMs. See Section 5.3.6 for details.
  //; STEP 17: Determine the smallest common tRFC for all DIMMs. See Section 5.3.7 for details.
  //; STEP 18: Determine the smallest common tWTR for all DIMMs. See Section 5.3.8 for details.
  //; STEP 19: Determine the smallest common tRRD for all DIMMs. See Section 5.3.9 for details.
  //; STEP 20: Determine the smallest common tRTP for all DIMMs. See Section 5.3.10 for details.
  //;-----------------------------------------------------------------------;
  //
  // Outer loop j loops though all SPD data timings in TimingDataTbl and
  // gets them calculated and stored in CurrentMrcData->TimingData array.
  //
	TimingCalcPtr = &TrasTrpTrcdTbl[0];
	if (TimingCalcPtr == NULL)
		return FAILURE;
  
    for (j = 0; j < MRC_DATA_MAX_TIMING_DATA; j++) {
        //
        // Inner loop i loops through the min to max supported timing data looking for
        // best possible timing data.
        //
        FoundGoodValue = FALSE;
        if (MinSPDVal[j]!= 0) {
          for (i = TimingCalcPtr->DDR3_MinMCHVal; i <= TimingCalcPtr->MaxMCHVal; i++) {
            ProposedTime = i * FrequencyMultiplier[DdrFreqInx];
            if (ProposedTime >= MinSPDVal[j]) {
              FoundGoodValue = TRUE;
              break;
            }
          }
        }
        if ((FoundGoodValue == TRUE)) {
          if (j == MRC_DATA_TRFC) {
            // Special case for tRFC round up to next even number of clocks
            i = (i+1) & 0xFE;
          }
          CurrentMrcData->TimingData[j] = i;
        } else {
          // CurrentMrcData->TimingData[j] = TimingCalcPtr->MaxMCHVal;
    
          return FAILURE;
        }
        TimingCalcPtr++;
    } //end for (different timing parameters)
	//  DEBUG (( EFI_D_ERROR, "calculate timing \n"));

  return SUCCESS;
}

STATUS
CalcDimmConfig (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  )
/*++

Routine Description:

  This function uses SPD data to determine timings.

Arguments:

  CurrentMrcData:   Detected DDR timing parameters for installed memory

Returns:

  DDR3 Sets MB DimmConfigChannel to
  // Config 0 - NC_NC
  // Config 1 - NC_x8SS                 (RC B)
  // Config 2 - NC_x16DS                (RC A)
  // Config 3 - NC_x8DS                 (RC F)
  // Config 4 - NC_x16SS                (RC C)
  // Config 5 - x8SS_x8SS               (RC B)
  // Config 6 - x16DS_x16DS             (RC A)
  // Config 7 - x8DS_x8DS               (RC F)  not supported
  // Config 8 - x16SS_x16SS             (RC C)

#define CONFIG_NCRCB                   		0
#define CONFIG_NCRCC                        1
#define CONFIG_NCRCF                        2
#define CONFIG_NCRCA                        3
#define CONFIG_NCRCD                        4
#define CONFIG_NCRCE                        5
--*/
{
  UINT8 Config, Channel;

  //
  //  STEP: Determine the DIMM configuration for each channel
  //
    Config = 0xF;

    for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {
    	CurrentMrcData->Channel[Channel].DimmConfigChannel  = 0;
		if (!CurrentMrcData->Channel[Channel].Enabled) {
		  continue;
		}

			if (CurrentMrcData->Channel[Channel].RankPresent[1] || CurrentMrcData->Channel[Channel].RankPresent[0] ) {
			   if (CurrentMrcData->Channel[Channel].DimmDataWidth[0] == 1 && CurrentMrcData->Channel[Channel].DimmSides[0] == 1) {
				 Config = 3;//2;
			   } else if (CurrentMrcData->Channel[Channel].DimmDataWidth[0] == 0 && CurrentMrcData->Channel[Channel].DimmSides[0] == 1) {
				 Config = 2;//3;
			   } else if (CurrentMrcData->Channel[Channel].DimmDataWidth[0] == 1 && CurrentMrcData->Channel[Channel].DimmSides[0] == 0) {
				 Config = 1;//4;
			   } else {
				 Config = 0;//1;
			   }
		   //}
			}
		CurrentMrcData->Channel[Channel].DimmConfigChannel = Config;
    }
		if ( (CurrentMrcData->Channel[0].DimmConfigChannel == 0xF) && (CurrentMrcData->Channel[1].DimmConfigChannel == 0xF ) ) {
			return FAILURE;
		}
		//DEBUG (( EFI_D_ERROR, "dimm config %d.\n",Config));
    return SUCCESS;
}


STATUS
ReadSpdByteFromSmbus(
  UINT16                SmbusBase,
  UINT8                 SpdAddress,
  UINT8                 Offset,
  UINT8                 *Buffer
)
/*++

Routine Description:

This function reads SPD information from a DIMM.
Implementation is light on error handling and must be customized for
a particular platform.

Arguments:

Dimm          DIMM to read from
Offset        Offset in DIMM
Buffer        Return buffer

Returns:

--*/
{
  UINT8  SmbStatus;
  UINT8  RetryCount = 10;

  //SmbStatus Bits of interest
  //[6] = IUS (In Use Status)
  //[4] = FAIL
  //[3] = BERR (Bus Error = transaction collision)
  //[2] = DERR (Device Error = Illegal Command Field, Unclaimed Cycle, Host Device Timeout, CRC Error)
  //[1] = INTR (Successful completion of last command)
  //[0] = HOST BUSY
  while (RetryCount--) {

    // Wait for HSTS.HBSY to be clear
    do { SmbStatus = (UINT8) IoIn8(SmbusBase+0); } while ((SmbStatus & BIT0) != 0);

    // Clear all status bits
    IoOut8(SmbusBase+0, 0xFE);

    // Set offset to read
    IoOut8(SmbusBase+3, (Offset & 0x7F) );

    // Set smbus address (Device) to read
    IoOut8(SmbusBase+4, (SpdAddress|1));

    // Set "Read Byte" protocol and start bit
    IoOut8(SmbusBase+2, 0x48);

    // poll until any of FAIL, BERR, DERR, INTR
    do { SmbStatus = (UINT8) IoIn8(SmbusBase+0); } while ((SmbStatus & (BIT4|BIT3|BIT2|BIT1)) == 0);


    if ((SmbStatus & (BIT4|BIT3|BIT2)) == 0) {
      // INTR, but not (FAIL, BERR, DERR)
      *Buffer = (UINT8) IoIn8(SmbusBase+5);  // Host Data Register
      IoOut8(SmbusBase+0, (SmbStatus|BIT6) );  // Set to clear HSTS.IUS (In-Use Status)

      return SUCCESS;
    }
  }

  return FAILURE;
}

STATUS
GetSpdData (
  MRC_PARAMETER_FRAME   *CurrentMrcData,
  UINT8 Channel,
  UINT8 *SpdTable,
  UINT8 TableLen
  )
/*++

Routine Description:

This function reads SPD data and determines which slots are populated.

Arguments:

SpdData:        Pointer to an array for storing SPD for each socket.

Returns:

--*/
{
  STATUS  Status;
  UINT8   CurrentDimmSocket;
  UINT8   i;
  MEMINIT_SPD_DATA  *SpdData;
  UINT8	Length = VF_SC_BYTE_LEN;

  UINT32 IoBase;
//AMI_OVERRIDE - EIP168616 memory down function >>
#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 1)  
  UINT8  *DimmSpdTbl; 
#endif 
//AMI_OVERRIDE - EIP168616 memory down function <<
  
  //
  // Configure GPIO muxes for SMBUS
  //
  IoBase = Mmio32Read (0xe00f804c) & 0xFFFFC000;

  Mmio32Write (IoBase + 0x5A0, 0x2003CC81);

  Mmio32Write (IoBase + 0x580, 0x2003CC81);

  Mmio32Write (IoBase + 0x5C0, 0x2003CC81);

  //
  // Now examine each socket
  //
  for (CurrentDimmSocket = 0; CurrentDimmSocket < MAX_SLOTS; CurrentDimmSocket++) {

    SpdData = &CurrentMrcData->SpdData[CurrentDimmSocket +(Channel*MAX_SLOTS)];
//AMI_OVERRIDE - EIP140916 memory down function	>>
#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 1)
//AMI_OVERRIDE - EIP168616 memory down function >> 
    DimmSpdTbl = GetDimmSpdTbl (Channel, CurrentDimmSocket);
//AMI_OVERRIDE - EIP168616 memory down function <<
    
    if (DimmSpdTbl == NULL) {
        Status = FAILURE;
    } else {
        SpdData->Buffer[SPD_MEMORY_TYPE] = DimmSpdTbl[SPD_MEMORY_TYPE];
        Status = SUCCESS;
    }
#else
    Status = MrcSmbusExec(
						CurrentMrcData->SmbusBar,
						CurrentMrcData->OemMrcData.SPDAddressTable[Channel][CurrentDimmSocket],
						SMBUS_READ_BYTE, 
						SPD_MEMORY_TYPE, 
						&Length, 
						&SpdData->Buffer[SPD_MEMORY_TYPE]);
#endif
//AMI_OVERRIDE - EIP140916 memory down function <<
    if (Status != SUCCESS) {
      SpdData->SpdPresent = FALSE;
      DEBUG (( EFI_D_INFO, "C%d.D%d: SPD not present.\n",Channel, CurrentDimmSocket));
      continue;
    }


    //DEBUG (( EFI_D_INFO,  "C%d.D%d: SPD Address 0x%X \n",
    //		Channel, CurrentDimmSocket,
    //		CurrentMrcData->OemMrcData.SPDAddressTable[Channel][CurrentDimmSocket]));

    switch (SpdData->Buffer[SPD_MEMORY_TYPE]) {
	  case SPD_DDR3: //DDR3
            for (i = 0; i < TableLen; i++){
			//AMI_OVERRIDE - EIP140916 memory down function >>
				#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 1)
            	SpdData->Buffer[*(SpdTable+i)] = DimmSpdTbl[*(SpdTable+i)];
            	Status = SUCCESS;
				#else
    			Status = MrcSmbusExec(
						CurrentMrcData->SmbusBar,
						CurrentMrcData->OemMrcData.SPDAddressTable[Channel][CurrentDimmSocket],
						SMBUS_READ_BYTE, 
						*(SpdTable+i), 
						&Length, 
						&SpdData->Buffer[*(SpdTable+i)] );
				#endif
			//AMI_OVERRIDE - EIP140916 memory down function <<
	            //DEBUG (( EFI_D_INFO, "C%d.D%d: SPD byte %d = 0x%x\n",Channel, CurrentDimmSocket, * (SpdTable + i), SpdData->Buffer[*(SpdTable+i)]));
            }
        break;
      default:
    	  //DEBUG (( EFI_D_INFO, "C%d.D%d: GetSpdData failed.\n", Channel, CurrentDimmSocket));
    	  Status =  FAILURE;
        break;
    };

    if (Status != SUCCESS) {
      SpdData->SpdPresent = FALSE;
      //DEBUG (( EFI_D_INFO, "C%d.D%d: SPD present false.\n", Channel, CurrentDimmSocket));
    } else {
      SpdData->SpdPresent = TRUE;
      CurrentMrcData->Channel[Channel].DimmPresent[CurrentDimmSocket] = 1;
      CurrentMrcData->TotalDimm[Channel]++;
    }
  }

  return SUCCESS;
}

STATUS
AcquireBus (
    UINT16	SmbusBase
  )
/*++

Routine Description:

  This routine attempts to acquire the SMBus

Arguments:

  None

Returns:
  
  FAILURE as failed
  SUCCESS as passed

--*/
{
  UINT8 StsReg;

  StsReg  = 0;
  StsReg  = (UINT8)IoIn8(SmbusBase + R_PCH_SMBUS_HSTS);
  if (StsReg & B_PCH_SMBUS_IUS) {
    return FAILURE;
  } else if (StsReg & B_PCH_SMBUS_HBSY) {
    //
    // Clear Status Register and exit
    //
    // Wait for HSTS.HBSY to be clear
    do { StsReg = (UINT8) IoIn8(SmbusBase+R_PCH_SMBUS_HSTS); } while ((StsReg & B_PCH_SMBUS_HBSY) != 0);

    // Clear all status bits
    IoOut8(SmbusBase+R_PCH_SMBUS_HSTS, 0xFE);
    return SUCCESS;
  } else {
    //
    // Clear out any odd status information (Will Not Clear In Use)
    //
	IoOut8(SmbusBase+R_PCH_SMBUS_HSTS, StsReg);
    return SUCCESS;
  }
}

STATUS
MrcSmbusExec (
  UINT16 SmbusBase,
  UINT8 SlvAddr,
  UINT8 Operation,
  UINT8 Offset,
  UINT8 *Length,
  UINT8 *Buffer
  )
/*++

Routine Description:

  This routine reads SysCtl registers

Arguments:

  SmbusBase -  SMBUS Base Address
  SlvAddr - Targeted Smbus Slave device address
  Operation - Which SMBus protocol will be used
  Offset - Offset of the register
  Length - Number of bytes
  Buffer - Buffer contains values read from registers

Returns:

  SUCCESS as passed
  Others as failed

--*/
{
  STATUS      Status;
  UINT8       AuxcReg;
  UINT8       SmbusOperation = 0;
  UINT8       StsReg;
  UINT8       SlvAddrReg;
  UINT8       HostCmdReg;
  UINT8       BlockCount = 0;
  BOOLEAN     BufferTooSmall;
  UINT8       Index;
  UINT8       *CallBuffer;
  UINT8  	  RetryCount = BUS_TRIES;

  //
  // MrcSmbusExec supports byte and block read.
  // Only allow Byte or block access
  if (!((*Length  == VF_SC_BYTE_LEN) || (*Length == VF_SC_BLOCK_LEN))) {
    return FAILURE;
  }

  //
  // See if its ok to use the bus based upon INUSE_STS bit.
  //
  Status = AcquireBus (SmbusBase);
  if (Status == FAILURE) {
    return Status;
  }

  CallBuffer = Buffer;

  //SmbStatus Bits of interest
  //[6] = IUS (In Use Status)
  //[4] = FAIL
  //[3] = BERR (Bus Error = transaction collision)
  //[2] = DERR (Device Error = Illegal Command Field, Unclaimed Cycle, Host Device Timeout, CRC Error)
  //[1] = INTR (Successful completion of last command)
  //[0] = HOST BUSY

  //
  // This is the main operation loop.  If the operation results in a Smbus
  // collision with another master on the bus, it attempts the requested
  // transaction again at least BUS_TRIES attempts.
  //
  while (RetryCount--) {
    //
    // Operation Specifics (pre-execution)
    //
    Status          = SUCCESS;
    SlvAddrReg      = SlvAddr;
    HostCmdReg      = Offset;
    AuxcReg         = 0;

	switch (Operation) {

	case SMBUS_WRITE_BYTE:
      	IoOut8 (SmbusBase+R_PCH_SMBUS_HD0, CallBuffer[0]);
		SmbusOperation = V_PCH_SMBUS_SMB_CMD_BYTE_DATA;
	break;

    case SMBUS_READ_BYTE:
      	SmbusOperation = V_PCH_SMBUS_SMB_CMD_BYTE_DATA;
	  	SlvAddrReg |= B_PCH_SMBUS_RW_SEL_READ;
      	if (*Length < 1) {
        	Status = FAILURE;
      	}
      	*Length = 1;
	break;

    case SMBUS_WRITE_BLOCK:
      	SmbusOperation  = V_PCH_SMBUS_SMB_CMD_BLOCK;
      	IoOut8 (SmbusBase+R_PCH_SMBUS_HD0, *(UINT8 *) Length);
      	BlockCount = (UINT8) (*Length);
      	if ((*Length < 1) || (*Length > 32)) {
        	Status = FAILURE;
        	break;
      	}
      	AuxcReg |= B_PCH_SMBUS_E32B;
	break;

    case SMBUS_READ_BLOCK:
      	SmbusOperation = V_PCH_SMBUS_SMB_CMD_BLOCK;
      	SlvAddrReg |= B_PCH_SMBUS_RW_SEL_READ;
      	if ((*Length < 1) || (*Length > 32)) {
        	Status = FAILURE;
        	break;
      	}
      	AuxcReg |= B_PCH_SMBUS_E32B;
	break;

    default:
      	Status = FAILURE;
	break;
    }

    //
    // Set Auxiliary Control register
    //
    IoOut8 (SmbusBase+R_PCH_SMBUS_AUXC, AuxcReg);

    //
    // Reset the pointer of the internal buffer
    //
    IoIn8 (SmbusBase+R_PCH_SMBUS_HCTL);

    //
    // Now that the 32 byte buffer is turned on, we can write th block data
    // into it
    //
    if (Operation == SMBUS_WRITE_BLOCK) {
      for (Index = 0; Index < BlockCount; Index++) {
        //
        // Write next byte
        //
        IoOut8 (SmbusBase+R_PCH_SMBUS_HBD, CallBuffer[Index]);
      }
    }

    //
    // Set SMBus slave address for the device to read
    //
    IoOut8(SmbusBase+R_PCH_SMBUS_TSA, SlvAddrReg);

    // 
    //
    // Set Command register for the offset to read
    //
    IoOut8(SmbusBase+R_PCH_SMBUS_HCMD, HostCmdReg );

    //
    // Set Control Register to Set "operation command" protocol and start bit
    //
    IoOut8(SmbusBase+R_PCH_SMBUS_HCTL, (UINT8) (SmbusOperation + B_PCH_SMBUS_START));

    //
    // Wait for IO to complete
    //
	do { StsReg = (UINT8) IoIn8(SmbusBase+0); } while ((StsReg & (BIT4|BIT3|BIT2|BIT1)) == 0);

	if (StsReg & B_PCH_SMBUS_DERR) {
      Status = FAILURE;
      break;
    } 
	else if (StsReg & B_PCH_SMBUS_BERR) {
      //
      // Clear the Bus Error for another try
      //
      Status = FAILURE;
       IoOut8(SmbusBase+R_PCH_SMBUS_HSTS, B_PCH_SMBUS_BERR);
      //
      // Clear Status Registers
      //
       IoOut8(SmbusBase+R_PCH_SMBUS_HSTS, B_PCH_SMBUS_HSTS_ALL);
       IoOut8(SmbusBase+R_PCH_SMBUS_AUXS, B_PCH_SMBUS_CRCE);

      continue;
    }

    //
    // successfull completion
    // Operation Specifics (post-execution)
    //
    switch (Operation) {

    case SMBUS_READ_BYTE:
      CallBuffer[0] = (UINT8)(IoIn8 (SmbusBase+R_PCH_SMBUS_HD0));
      break;

    case SMBUS_WRITE_BLOCK:
      IoOut8(SmbusBase+R_PCH_SMBUS_HSTS, B_PCH_SMBUS_BYTE_DONE_STS);
      break;

    case SMBUS_READ_BLOCK:
      BufferTooSmall = FALSE;
      //
      // Find out how many bytes will be in the block
      //
      BlockCount = (UINT8)(IoIn8 (SmbusBase+R_PCH_SMBUS_HD0));
      if (*Length < BlockCount) {
        BufferTooSmall = TRUE;
      } else {
        for (Index = 0; Index < BlockCount; Index++) {
          //
          // Read the byte
          //
          CallBuffer[Index] = (UINT8)IoIn8 (SmbusBase+R_PCH_SMBUS_HBD);
        }
      }

      *Length = BlockCount;
      if (BufferTooSmall) {
        Status = FAILURE;
      }
      break;

    default:
      break;
    };

    if ((StsReg & B_PCH_SMBUS_BERR) && (Status != FAILURE)) {
      //
      // Clear the Bus Error for another try
      //
      Status = FAILURE;
      IoOut8(SmbusBase+R_PCH_SMBUS_HSTS, B_PCH_SMBUS_BERR);

      continue;
    } else {
      break;
    }
  }

  //
  // Clear Status Registers and exit
  //
  IoOut8(SmbusBase+R_PCH_SMBUS_HSTS, B_PCH_SMBUS_HSTS_ALL);
  IoOut8(SmbusBase+R_PCH_SMBUS_AUXS, B_PCH_SMBUS_CRCE);
  IoOut8(SmbusBase+R_PCH_SMBUS_AUXC, 0);
  return Status;
}
