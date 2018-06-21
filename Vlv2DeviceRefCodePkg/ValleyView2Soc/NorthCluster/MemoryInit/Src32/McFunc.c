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

  McFunc.c

Abstract:

  This file contain memory DIMM initialization function.

--*/

#include "McFunc.h"
#include "MrcFunc.h"
#include "MchRegs.h"
#include "SCRegs.h"
#include "IoAccess.h"


STATUS
FindCoreFrequency (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  )
{
  UINT8 CoreFreq = 0;
  UINT32 buffer32;

  //DDR Freq 20[26:23] 001:800, 010:1066, 100:1333
  buffer32 = MsgBus32Read(VLV_UNIT_CCK, 0x0C);
  CoreFreq = (UINT8)((buffer32 & (BIT26|BIT25|BIT24|BIT23)) >> 23);

  switch (CoreFreq) {
    case 1: CurrentMrcData->DdrFreqCap = CurrentMrcData->CoreFreq = DDRFREQ_800; break;
    case 2: CurrentMrcData->DdrFreqCap = CurrentMrcData->CoreFreq = DDRFREQ_1066; break;
    case 4: CurrentMrcData->DdrFreqCap = CurrentMrcData->CoreFreq = DDRFREQ_1333; break;
    default: CurrentMrcData->DdrFreqCap = CurrentMrcData->CoreFreq = DDRFREQ_1066; break;
  }

  buffer32 = MsgBus32Read(VLV_UNIT_DUNIT, MC_DFUSESTAT_OFFSET);

  CurrentMrcData->NumBitDRAMCap = (buffer32 >> 16) & 0x1;
  CurrentMrcData->MaxMemSizeCap = (buffer32 >>1 ) & 0x7;

  CurrentMrcData->EccEnabled = ~(buffer32)& 0x1;

  return SUCCESS;

}

STATUS
GetPlatformSettings (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  )
{

  PciCfg32Write_CF8CFC(MC_BUS, 0x00, 0x00, 0xD8, 0);
  PciCfg32Write_CF8CFC(MC_BUS, 0x00, 0x00, 0xD0, ((VLV_CMD_READ_REG) | (VLV_UNIT_BUNIT << 16)) + (BUNIT_BECREG_OFFSET << 8) + 0xF0);
  CurrentMrcData->EcBase = PciCfg32Read_CF8CFC(MC_BUS, 0x00, 0x00, 0xD4) & 0xFFFFFFFE;
  
  //Settting ECBase  
  if (CurrentMrcData->EcBase == 0) {
    CurrentMrcData->EcBase = EC_BASE;
    
    PciCfg32Write_CF8CFC(MC_BUS, 0x00, 0x00, 0xD4, (CurrentMrcData->EcBase|BIT0));
    PciCfg32Write_CF8CFC(MC_BUS, 0x00, 0x00, 0xD8, 0);
    PciCfg32Write_CF8CFC(MC_BUS, 0x00, 0x00, 0xD0, (VLV_CMD_WRITE_REG  + (VLV_UNIT_BUNIT << 16) + (BUNIT_BECREG_OFFSET << 8) + 0xF0));
  }

  if (CurrentMrcData->SmbusBar == 0) {
	CurrentMrcData->SmbusBar = PciCfg16Read(CurrentMrcData->EcBase, MC_BUS, 0x1F, 0x3, 0x20) & ~(BIT1|BIT0);
  }
  
  return SUCCESS;
}

VOID
DetermineBootMode (
  MRC_PARAMETER_FRAME   *CurrentMrcData
  )
{
    UINT32   PMCON1_32;
    UINT8   WarmResetOccurred = 0;
    UINT8   DramInSelfRefresh = 0;
    UINT32  buffer32, PmcBase;
  
    buffer32 = MsgBus32Read(VLV_UNIT_DUNIT, MC_PMSTS_OFFSET); //(UINT8) ((McMmio32Read(MC_MMIO_PMSTS) & BIT8) >> 8);
    WarmResetOccurred = (UINT8)((buffer32 & BIT8) >> 8);
    DramInSelfRefresh = (UINT8)(buffer32 & BIT0);

    if (CurrentMrcData->BootMode == S3Path) {

    // S3 resume from BIOS

    } else if (WarmResetOccurred) {
      if (DramInSelfRefresh) {
       // Because PMSTS Warm Reset Indicator can be set by a reset before Memory Init has begun,
       // Use the EPDBONUS Byte 0 register as an indicator that Memory Init has never completed and treat as cold boot
       CurrentMrcData->BootMode = S0Path;
      } else {
          IoOut8(0xCF9, 0xE);
          MRC_DEADLOOP();
      }
    } 

    PmcBase = PciCfg32Read_CF8CFC (
          DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC,
          R_PCH_LPC_PMC_BASE
		  );
    PmcBase &= B_PCH_LPC_PMC_BASE_BAR;

    //Check PMC_BASE + 0x20 GEN_PMCON1 register bit = 1b indicates BIOS has not completed DRAM Initialization on previous boot
    PMCON1_32 = Mmio32Read (PmcBase + R_PCH_PMC_GEN_PMCON_1);

    if (PMCON1_32 & B_PCH_PMC_GEN_PMCON_DRAM_INIT) {
        if (CurrentMrcData->BootMode == FBPath)
        CurrentMrcData->BootMode = S5Path; }

     return;
}

BOOLEAN
CheckColdBootRequired (
  MRC_PARAMETER_FRAME   *CurrentMrcData,
  UINT8 Channel
  )
{
    UINT8                      CurrentSocket;
    MEMINIT_SPD_DATA           *CurrentSpdData;
    UINT8 i; 
    UINT8 *pData8;

    FindCoreFrequency (CurrentMrcData);
    if ((CurrentMrcData->OemMrcData.CoreFreq) != CurrentMrcData->DdrFreqCap) {
        return TRUE;
    }

    for (CurrentSocket = 0; CurrentSocket < MAX_SLOTS; CurrentSocket++){
    	if (CurrentMrcData->Channel[Channel].DimmPresent[CurrentSocket] == 0) {
    		if ((CurrentMrcData->OemMrcData.Channel[Channel].FastBootData[CurrentSocket].ManuIDlo == 0) &&
    				(CurrentMrcData->OemMrcData.Channel[Channel].FastBootData[CurrentSocket].ManuIDhi == 0)) {
    			continue;
    		} else {
    			return TRUE;
    		}
    	}

    	CurrentSpdData = &(CurrentMrcData->SpdData[(Channel*MAX_SLOTS+CurrentSocket)]);
    	memcpy (&(CurrentMrcData->Channel[Channel].FastBootData[CurrentSocket]), &(CurrentMrcData->OemMrcData.Channel[Channel].FastBootData[CurrentSocket]), sizeof(FASTBOOTDATA));
    	pData8 = (UINT8*)(&(CurrentMrcData->Channel[Channel].FastBootData[CurrentSocket]));
      
    	if (CurrentMrcData->Channel[Channel].FastBootData[CurrentSocket].TotalDimm != CurrentMrcData->TotalDimm[Channel]) {
    		return TRUE;
    	}
    	//do comparison
    	for (i=0; i < 9; i++) {
    		if ((*(pData8+i)) != CurrentSpdData->Buffer[SPD_DDR3_MANUFACTURER_ID_LO+i]) {
    			CurrentSocket = MAX_SLOTS;
    			return TRUE;
    		}
    	} //for i
    } //end of for socket
    return FALSE;
}

STATUS
McEnableHPET (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
  /*volatile UINT32 Data32;
  UINT32 IchRcbaBaseAddr;

  IchRcbaBaseAddr = PciCfg32Read_CF8CFC (
        PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC,
        R_PCH_LPC_RCBA
        );
  IchRcbaBaseAddr &= ~BIT0;*/

  // Set HPET Timer enable to start counter spinning
  Mmio32Or (R_PCH_PCH_HPET + R_PCH_PCH_HPET_GCFG, B_PCH_PCH_HPET_GCFG_EN);

  return SUCCESS;
}

STATUS
McDisableHPET (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
  volatile UINT32 Data32;
  /*UINT32 IchRcbaBaseAddr;
  
  IchRcbaBaseAddr = PciCfg32Read_CF8CFC (
        PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC,
        R_PCH_LPC_RCBA
        );
  IchRcbaBaseAddr &= ~BIT0;*/

  // Clear HPET Timer enable to stop counter spinning
  Mmio32And (R_PCH_PCH_HPET + R_PCH_PCH_HPET_GCFG, ~(B_PCH_PCH_HPET_GCFG_LRE | B_PCH_PCH_HPET_GCFG_EN));
  Data32 = Mmio32Read (R_PCH_PCH_HPET + R_PCH_PCH_HPET_GCFG); // Read back to flush posted write

  //Set BMISC2 bit 2 to let Fsegment& Esegment access to DRAM instead of DMI
  Data32 = MsgBus32Read(VLV_UNIT_BUNIT,BUNIT_BMISC_OFFSET);
  Data32|=(BIT0+BIT1);
  if(CurrentMrcData->SiRevisionID > 4){
	  //VLV B0 BMISC PnP setting [8:5] = 4’b1110
	  Data32|=(BIT8+BIT7+BIT6);
	  Data32 &= ~BIT5;
  }
  MsgBus32Write(VLV_UNIT_BUNIT,BUNIT_BMISC_OFFSET,Data32);


  return SUCCESS;
}

STATUS
SetInitDone (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
  RegDCO DCOreg;
  UINT32 PmcBase;

  PmcBase = PciCfg32Read_CF8CFC (
		  DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, PCI_FUNCTION_NUMBER_PCH_LPC,
		  R_PCH_LPC_PMC_BASE
		  );
  PmcBase &= B_PCH_LPC_PMC_BASE_BAR;

  //Set PMC_BASE + 0x20 GEN_PMCON1 register bit [23] = 0b to indicate System BIOS has completed DRAM Initialization.
  Mmio32And (PmcBase + R_PCH_PMC_GEN_PMCON_1, ~B_PCH_PMC_GEN_PMCON_DRAM_INIT);
  
  DCOreg.raw = MsgBus32Read(VLV_UNIT_DUNIT, MC_DCO_OFFSET);

  if(CurrentMrcData->SiRevisionID > 4){
    DCOreg.field.DRPLOCK = 1;
    DCOreg.field.REUTLOCK = 1;
  } else {
    DCOreg.field.DRPLOCK = 0;
    DCOreg.field.REUTLOCK = 0;
  }

  CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_DCO_OFFSET ,DCOreg.raw);

  return SUCCESS;
}

// 32 bit LFSR with characteristic polinomial:  X^32 + X^22 +X^2 + X^1
// The function takes pointer to previous 32 bit value and modifies it to next value.
// 
void Lfsr32 (UINT32 *LfsrWordPtr)
{
  UINT32 bit;
  UINT32 lfsr;
  UINT8 i;
  lfsr = *LfsrWordPtr;

  for (i=0; i<32; i++) {
    bit  =   1 ^  (lfsr & BIT0)        ;
    bit  = bit ^ ((lfsr & BIT1)  >>  1);
    bit  = bit ^ ((lfsr & BIT2)  >>  2);
    bit  = bit ^ ((lfsr & BIT22) >> 22);
    lfsr = ((lfsr >> 1) | (bit << 31));
  }
  *LfsrWordPtr = lfsr;

  return;
}

UINT32 get_initial_seed()
{
    UINT32	Data32;
#ifndef _MSC_EXTENSIONS
    asm("pushl %%eax;"
        "pushl %%ebx;"
        "pushl %%ecx;"
        "pushl %%edx;"
        "rdtsc;"
        "mov   %%eax, %0;"
        "popl  %%edx;"
        "popl  %%ecx;"
        "popl  %%ebx;"
        "popl  %%eax;"       
         : "=m"(Data32) : :
    );
#else   
    _asm{
  	 
	   push    eax
	   push    ebx
	   push    ecx
	   push    edx
	   
	   rdtsc
	   mov	   Data32, eax
	   
	   pop	   edx
	   pop	   ecx
	   pop	   ebx
	   pop	   eax		
    }
#endif
    return Data32;
}


STATUS 
SetScrambler (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
    RegSCRMSEED SCRMSEEDreg;
    UINT32 lfsr = 0;
    UINT8 i;

    SCRMSEEDreg.field.SCRMDIS = 0;	//Scrambler Enabled

    //Check if Scrambler Enabled
    if(SCRMSEEDreg.field.SCRMDIS == 0 ){
    	// 32 bit seed is always stored in BIOS NVM.
    	lfsr = CurrentMrcData->ScramblerSeed;

    	if (CurrentMrcData->BootMode == S5Path){  
    		// factory value is 0 and in first boot, a clock based seed is loaded.
    		if (lfsr == 0) {
    			lfsr = get_initial_seed() & 0x0FFFFFFF; // get seed from system clock and make sure it is not all 1's
    		}
    		// need to replace scrambler
    		// get next 32bit LFSR 16 times which is the last part of the previous scrambler vector.
    		else {
    			for (i=0; i<16; i++) {
    				Lfsr32(&lfsr);
    			}
    		}

    		CurrentMrcData->ScramblerSeed = lfsr;  // save new seed.
    		} //if (CurrentMrcData->BootMode == S5Path)

            // In warm boot or S3 exit, we have the previous seed.
            // In cold boot, we have the last 32bit LFSR which is the new seed.
            Lfsr32(&lfsr); // shift to next value
        	SCRMSEEDreg.field.SCRMSEED = lfsr&0x0003FFFF;

            CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_SCRMSEED_OFFSET, SCRMSEEDreg.raw);
            CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_SCRMLO_OFFSET, lfsr&0xAAAAAAAA);
            CurrentMrcData->DunitMsgBus32Write(VLV_UNIT_DUNIT, MC_SCRMHI_OFFSET, lfsr&0xAAAAAAAA);

        }

    return SUCCESS;
}

STATUS DisableRank2RankSwitching (MRC_PARAMETER_FRAME *CurrentMrcData, UINT8 Channel)
{

#if RNK2RNK_SHARING_DISABLED
#else
	UINT8 i = 0;
	for (i = 0; i < TOTAL_MODS; i++) {
		RunitMsgBusAndThenOr((UINT32)(DDRIO_DQ0_RK2RKCTL_OFFSET+(i*0x800)),0x00000000, 0x1F1F0FFF);
    //RunitMsgBusAndThenOr((UINT32)(DDRIO_DQ0_RK2RKCTL_OFFSET|DDRIO_ALL_DQMODULE),0x00000000, 0x1F1F0FFF);
	}
#endif

	return SUCCESS;
}

STATUS ProgBunit (MRC_PARAMETER_FRAME *CurrentMrcData, UINT8 Channel){

	UINT8	NumChannel;
	UINT8	Index;
	UINT32	TempValue;

	NumChannel = 0;

	for(Index = 0; Index < MAX_CHANNELS_TOTAL; Index++){
		if (CurrentMrcData->Channel[Index].Enabled){
			NumChannel++;
		}
	}

	TempValue = MsgBus32Read (VLV_UNIT_BUNIT, BUNIT_BMISC_OFFSET);
	if(NumChannel == 1){
		TempValue = TempValue & (~(BIT3 + BIT4));
	} else if(NumChannel == 2){
		TempValue = TempValue & (~(BIT3 + BIT4));
		TempValue = TempValue + BIT3;
	} else {
		//Use default setting if number of channels is more than 2
	}

	MsgBus32Write (VLV_UNIT_BUNIT, BUNIT_BMISC_OFFSET, TempValue);

	return SUCCESS;
}


