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

  MMRCLibraries.c

Abstract:

  Modular MMRC static libraries used throughout the MMRC, these files are
  independent of the project.

--*/

#include "../Include/MMRC.h"
#include "../Include/MMRCProjectLibraries.h"
#include "../Include/MMRCProjectRegisters.h"
#include "../Include/MMRCLibraries.h"
#include "../Include/CpgcVlvX0.h"
#include "../../../../Src32/IoAccess.h"

/*++

Routine Description:

  Retrieves the Byteoffset, starting, and ending bit for a given channel, rank and strobe.

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Element:        Information on the description and flags of the desired field.
  Channel:        Channel selection for the requested field.
  Rank:           Rank selection for the requested field.
  Strobe:         Strobe selection for the requested field.
  ByteOffset:     Final ByteOffset based on Channel/Rank/Strobe.
  StartingBit:    Final Starting bit for the field based on Channel/Rank/Strobe.
  EndingBit:      Final Ending bit for the field based on Channel/Rank/Strobe.

Returns:

  Success
  Failure

--*/
static STATUS
GetLocation (
  IN  MMRC_DATA     *ModMrcData,
  IN  SetGetElements Element,
  IN  UINT8          Channel,
  IN  UINT8          Rank,
  IN  UINT8          Strobe,
  OUT UINT16         *ByteOffset,
  OUT UINT8          *StartingBit,
  OUT UINT8          *EndingBit)
{
  //
  // The initial ByteOffset, starting, and ending bits are taken from the Element[] array except
  // when the unique bit is set, which it will be taken from the URank array but only for ranks above 0.
  //
  //if (Element.UniqueIndex == 0x3f || Rank==0) {
  if (Element.UniqueIndex == -1 || Rank==0) {
    *ByteOffset  = Element.Offset;
    *StartingBit = Element.StartingBit;
    *EndingBit   = Element.EndingBit;
  } else {
    *ByteOffset  = URanks[Element.UniqueIndex].rank[Rank-1].Offset;
    *StartingBit = URanks[Element.UniqueIndex].rank[Rank-1].StartingBit;
    *EndingBit   = URanks[Element.UniqueIndex].rank[Rank-1].EndingBit;
  }

  //
  // If the channel selection is enabled, modify the offset by the channeloffset macro.
  // For those fields that are channel independent, the ChannelEnable flag will be clear.
  //
  if (Element.ChannelEnable) {
	  if (Element.Offset < 0x4800) {
		  *ByteOffset += CHANNEL_BYTEOFFSET * Channel;
	  } else {
		  *ByteOffset += CHANNEL_BYTEOFFSET2 * Channel;
	  }
  }

  //
  // If the module selection is enabled, modify the offset by the module offset based on the
  // selected module.
  //
  if (Element.ModuleEnable) {
    *ByteOffset += MODULE_OFFSET * (Strobe / STROBE_LANES_PER_MODULE);
  }

  //
  // The remaining changes include the Rank and SPM (Strobelane per module) Byte/bit offsets.
  //
  *ByteOffset  += Element.RankByteOffset * Rank + Element.SPMByteOffset * (Strobe % STROBE_LANES_PER_MODULE);
  *StartingBit += Element.RankBitOffset * Rank  + Element.SPMBItOffset  * (Strobe % STROBE_LANES_PER_MODULE);
  *EndingBit   += Element.RankBitOffset * Rank  + Element.SPMBItOffset  * (Strobe % STROBE_LANES_PER_MODULE);

  return SUCCESS;
}

void TranslateLinearToRegValue(UINT32 Freq, UINT32 *Value)
{
	if ((*Value) >= mmrcDigitalDLL[Freq].StrVal2) {
		(*Value) += (mmrcDigitalDLL[Freq].minVal2 - mmrcDigitalDLL[Freq].StrVal2);
	}
}

void TranslateRegToLinearValue(UINT32 Freq, UINT32 *Value)
{
	if ((*Value) >= mmrcDigitalDLL[Freq].minVal2) {
		(*Value) -= mmrcDigitalDLL[Freq].minVal2;
		(*Value) += mmrcDigitalDLL[Freq].StrVal2;
	}
}

/*++

Routine Description:

  Based on the linear delay element specified, compute the linear delay.  The linear delay should include the support for analog/digital dll.

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Socket:         Socket to operate on.
  Channel:        Channel to operate on.
  Dimm:           Dimm to operate on.
  Rank:           Rank to operate on.
  Strobe:         Strobe to operate on.
  Bit:
  IoLevel:
  Type:           Register to be accessed, this is pointer to an algorithm.
  Cmd:            Command to read/write from register/cache.
  Value:          Value retrieved.

Returns:

    Success
    Failure

--*/
static STATUS
GetLinearFromRegs(
  IN  MMRC_DATA   *ModMrcData,
  IN  UINT8        Socket,
  IN  UINT8        Channel,
  IN  UINT8        Dimm,
  IN  UINT8        Rank,
  IN  UINT8        Strobe,
  IN  UINT8        Bit,
  IN  UINT8        IoLevel,
  IN  UINT8        Type,
  IN  UINT8        Cmd,
  OUT UINT32      *Value
) {
  UINT8 ElementIndex;       // Index to the Delay/CC being operated upon.
  UINT32 TempValue;         // Temporary variable used throughout the function.
  UINT8 AlgoElementsStart;  // Starting position for the Delay/CC within Element[].
  UINT32 LocalIndex;

  //
  // Initialize the value to 0, this is the value that will get returned.
  //
  *Value = 0;

  //
  // Compute the starting index within the Element[] array for the elements used to assign the linear value.
  //
  AlgoElementsStart = (Type-ALGO_REG_INDEX)*NUM_ELEMENTS_PER_ALGO;
  LocalIndex = (Type-ALGO_REG_INDEX);

  for (ElementIndex = 0; ElementIndex < NUM_DELAY_ELEMENTS; ElementIndex++) {
    //
    // Attempt to read the delay element, if successful accumulate to the value variable.
    // PI element will be incremented by the read value * 1, where all the other elements will
    // be multiplied by the granularity.
    //
    if (GetSetDataSignal (ModMrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, IoLevel, AlgoElementsStart+ElementIndex, RD_REG, &TempValue) == SUCCESS) {
      if (ElementIndex == NUM_DELAY_ELEMENTS -1) {
  		if (ModMrcData->FeatureSettings.MrcDigitalDll) {
  			if (LocalIndex == WDQS_INDEX || LocalIndex == WDQ_INDEX || LocalIndex == RCVN_INDEX || LocalIndex == CMD_INDEX || LocalIndex == CMD_CLKCTL_INDEX)
  			{
  				TranslateRegToLinearValue(ModMrcData->CurrentFrequency, &TempValue);
  			}
  		}

    	  *Value += TempValue * 1;
      } else {
         *Value += TempValue * (Granularity[ElementIndex] * HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency));
      }
    }
  }
  return SUCCESS;
}

/*++

Routine Description:

  Based on the linear delay element specified, it will set the appropriate delay and clock crossing elements to form the linear delay.

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Socket:         Socket to operate on.
  Channel:        Channel to operate on.
  Dimm:           Dimm to operate on.
  Rank:           Rank to operate on.
  Strobe:         Strobe to operate on.
  Bit:
  IoLevel:
  Type:           Register to be accessed, this is pointer to an algorithm.
  Cmd:            Command to read/write from register/cache.
  Value:          Value to Write.

Returns:

  Success
  Failure

--*/
static STATUS
SetRegsToLinear(
  MMRC_DATA   *ModMrcData,
  IN UINT8     Socket,
  IN UINT8     Channel,
  IN UINT8     Dimm,
  IN UINT8     Rank,
  IN UINT8     Strobe,
  IN UINT8     Bit,
  IN UINT8     IoLevel,
  IN UINT8     Type,
  IN UINT8     Cmd,
  IN UINT32   *Value
) {
  UINT8  ElementIndex;          // Index to the Delay/CC being operated upon.
  UINT32 CurrentValue;          // Remaining Linear value to be programmed.
  UINT32 TempValue;             // Temporary variable used throughout the function.
  UINT32 ElementValue;          // Value to be programmed into the specific element.
//  UINT8  RangeIndex;            // Index of ranges for each CC.
  UINT8 AlgoElementsStart;     // Starting position for the Delay/CC within Element[].
  UINT32 PiValue;				// Keep Onex value for CC reference
  UINT32 LocalIndex;

  ElementValue = 0;
  PiValue = 0;
  //
  // Compute the starting index within the Element[] array for the elements used to assign the linear value.
  //
  AlgoElementsStart = ((Type-ALGO_REG_INDEX))*NUM_ELEMENTS_PER_ALGO;
  LocalIndex = (Type-ALGO_REG_INDEX);

  //
  // CurrentValue will contain the remaining linear delay needed to write to the elements.  Initially it
  // should always be the assigned value.
  //
  CurrentValue = *Value;

  for (ElementIndex = 0; ElementIndex < NUM_DELAY_ELEMENTS; ElementIndex++) {
    //
    // For all delay elements, except the PI, the actual value programmed must be based on the Granularity of the element.
    // The PI value, which is the last element of the assignment (NUM_DELAY_ELEMENTS -1) always has a granularity of 1.
    // ElementValue = Actual element to program.
    // TempValue is the linear value of the element programming, for example 2x=2, TempValue = 128...
    //
    if (ElementIndex == NUM_DELAY_ELEMENTS -1 ) {
        ElementValue = CurrentValue / 1;
        TempValue    = ElementValue * 1;
        //Saved Onex Value to be use for Clock Crossings value calculation
        PiValue = TempValue;
		if (ModMrcData->FeatureSettings.MrcDigitalDll) {
			if (LocalIndex == WDQS_INDEX || LocalIndex == WDQ_INDEX || LocalIndex == RCVN_INDEX || LocalIndex == CMD_INDEX || LocalIndex == CMD_CLKCTL_INDEX)
			{
				TranslateLinearToRegValue(ModMrcData->CurrentFrequency, &TempValue);
			}
			ElementValue = TempValue;
		}
    } else {
        ElementValue = CurrentValue / (Granularity[ElementIndex]*HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency));
        TempValue = ElementValue * (Granularity[ElementIndex]*HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency));
    }

    //
    // Attempt to program the element, if successful the subtract the linear value from the total to get
    // the remainder.
    //
    if (GetSetDataSignal (ModMrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, IoLevel, AlgoElementsStart+ElementIndex, FC_WR, &ElementValue) == SUCCESS) {
        CurrentValue -= TempValue;
    }
  }

  //
  // Set the starting index to the beginning of the Clock Crossings.
  //
  AlgoElementsStart += NUM_DELAY_ELEMENTS;

  //Clock Crossings value is depending on OneX value, load Saved OnexValue to use for comparison and calculation
  ElementValue = PiValue;
  for (ElementIndex = 0; ElementIndex < NUM_CC_ELEMENTS; ElementIndex++) {
     //
    // Check if the value of the PI was within the min/max percentage for that clock crossing.  If it
    // is then set the CC to the "invalue".  Otherwise set it to the "outValue".
    if ((ElementValue < (UINT32) (Granularity[NUM_DELAY_ELEMENTS-2]
                             * HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)
                             * cc[ElementIndex].MaxPercent / 100))
                    &&
       (ElementValue >= (UINT32)(Granularity[NUM_DELAY_ELEMENTS-2]
                             * HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)
                             * cc[ElementIndex].MinPercent/ 100)))
                    { TempValue = cc[ElementIndex].InValue; } else {TempValue =  cc[ElementIndex].OutValue;}

    //
    // Program the value to the Clock Crossing.
    //
    GetSetDataSignal (ModMrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, IoLevel, AlgoElementsStart+ElementIndex, FC_WR, &TempValue);
  }

  return SUCCESS;
}

STATUS GetSetDataSignal (
  MMRC_DATA *ModMrcData,
  UINT8                Socket,
  UINT8                Channel,
  UINT8                Dimm,
  UINT8                Rank,
  UINT8                Strobe,
  UINT8                Bit,
  UINT8                IoLevel,
  UINT8                Type,
  UINT8                Cmd,
  UINT32              *Value
)
{
  UINT32               Temp;
    UINT16 ByteOffset;
    UINT8  StartingBit;
    UINT8  EndingBit;
    UINT32 TempValue;
    UINT32 Mask;

  //
  // Map the passed  channel and strobe to the physical channel and strobe using the global floorplan.
  //
  Temp = Channel;
  Channel = FloorPlan[Temp][Strobe].Channel;
  Strobe  = FloorPlan[Temp][Strobe].Strobelane;

  if (Type == REG_UNDEFINED) {
    return STATUS_TYPE_NOT_SUPPORTED;
  }

    //
    // Check if the register is a physical register request or an algorithm linear value request.
    // This is specifically checking if its a physical register request.
    //
    if (Type < ALGO_REG_INDEX) {

      //
      // If the register being requested does not have a location offset, return FAILURE, this is to support the linear calculatino which has
      //
      if (Elements[Type].Offset == OFFSET_UNDEFINED) {
        return STATUS_TYPE_NOT_SUPPORTED;
      }

      //
      // If a cache read, then read directly from the trained values array.
      //
      //if ((Cmd & RD_ONLY)!=0 && (Cmd & RD_REG)==0 &&  Elements[Type].CacheIndex != (UINT8) 0xff) {
	  if ((Cmd & RD_ONLY)!=0 && (Cmd & RD_REG)==0 &&  Elements[Type].CacheIndex != -1) {
        *Value = ModMrcData->Channel[Channel].CachedValues[Elements[Type].CacheIndex][Rank][Strobe];
        return SUCCESS;
      }

      //
      // Get the actual byte/start/ending bit for the element based on channel/rank/strobe.
      //
      GetLocation (ModMrcData, Elements[Type], Channel, Rank, Strobe, &ByteOffset, &StartingBit, &EndingBit);

      //
      // If the command is a read, then read the register, masking the bits, and shifting the bits
      // so that the value is shown starting at bit 0.
      //
      if ((Cmd & RD_REG) ) {
          *Value = MemRegRead (ModMrcData->MrcDebugMsgLevel, DDRIO, Channel, ByteOffset);
          *Value &= (1<<(EndingBit+1))-1;
          *Value >>= StartingBit;
      }

      //
      // If the command is a write, compute the mask, shift the value to the approrpriate bits,
      // and read/modify/write.
      if ((Cmd & FC_WR) ) {
          Mask = ((1<<(EndingBit +1))-1) - ((1<<(StartingBit)) -1);
          TempValue = MemRegRead (ModMrcData->MrcDebugMsgLevel, DDRIO, Channel, ByteOffset);
          TempValue &= ~Mask;
          TempValue |= ((*Value)<<StartingBit) & Mask;
          MemRegWrite(ModMrcData->MrcDebugMsgLevel, DDRIO, Channel, ByteOffset, TempValue);
      }

      //
      // If a cache write, then write directly to the trained values array.
      //
      if ((Cmd & UPD_CACHE)) {
        //
        // Only cache the value if its cachable.
        //
        //if (Elements[Type].CacheIndex != (UINT8) -1) {
		if (Elements[Type].CacheIndex !=  -1) {
          ModMrcData->Channel[Channel].CachedValues[Elements[Type].CacheIndex][Rank][Strobe] = (UINT16)*Value;
        }
      }

    } else {
      //
      // When accessing the algorithm,  do a recursive call back to the Get/Set but for the individual elements.
      //

      //
      // For a force write, decode the value into the registers.
      //
      if ((Cmd & FC_WR)) {
        SetRegsToLinear(ModMrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, IoLevel, Type, Cmd, Value);
      }

      //
      // If a read, then read the registers (only the elements, not cc) and return the linear value.
      //
      if ((Cmd & RD_REG)) {
        GetLinearFromRegs(ModMrcData, Socket, Channel, Dimm, Rank, Strobe, Bit, IoLevel, Type, Cmd, Value);
      }

      //
      // If a cache read, then read directly from the trained values array.
      //
      if ((Cmd & RD_ONLY)!=0 && (Cmd & RD_REG)==0) {
        *Value = ModMrcData->Channel[Channel].Trained_Value.Values[Type-ALGO_REG_INDEX][Rank][Strobe];
      }

      // If a cache write, then write directly to the trained values array.
      //
      if ((Cmd & UPD_CACHE)) {
        ModMrcData->Channel[Channel].Trained_Value.Values[Type - ALGO_REG_INDEX][Rank][Strobe] = (UINT16) *Value;
      }
    }
    return SUCCESS;
}

/*++

Routine Description:

  Restores the minus1 and the linear receive enable values for all active ranks and strobes.  This function will
  be called once per channel so the restoration will only restore the provided channel.

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Channel:        Channel to operate on.

Returns:

    Success
    Failure

--*/
STATUS
ReceiveEnableRestore (
  IN MMRC_DATA   *ModMrcData,
  IN UINT8        Channel
)
{
  UINT32 TempValue;            // Placeholder for the Get/Set values.
  UINT8  Rank;                 // Current rank being restored.
  UINT8  Strobe;               // Current strobe being restored.

  PRINT_FUNCTION_INFO;

  //
  // Entry Hooks
  //
  ReceiveEnableEntryHooks (ModMrcData, Channel);

  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_rcvn_list[0]);

  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
	  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
#if Rank2Rank_SHARING_DISABLED
      //
      // Restore RCVN value only to enabled rank if RANK2RANK Sharing is disabled
      //
    if (ModMrcData->Channel[Channel].RankEnabled[Rank]) {
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, (UINT8)DELAYS_RCVN_DEL, CMD_GET_CACHE, &TempValue);
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, (UINT8) DELAYS_RCVN_DEL, CMD_SET_VAL_FC, &TempValue);

        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RCVN_MIN, CMD_GET_CACHE, &TempValue);
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RCVN_MIN, CMD_SET_VAL_FC, &TempValue);
      }
#else
    //
    // Restore RCVN value to all ranks although RANK2RANK Sharing is enabled
    //
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, (UINT8)DELAYS_RCVN_DEL, CMD_GET_CACHE, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, (UINT8) DELAYS_RCVN_DEL, CMD_SET_VAL_FC, &TempValue);

    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RCVN_MIN, CMD_GET_CACHE, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RCVN_MIN, CMD_SET_VAL_FC, &TempValue);
#endif
	  }
  }

  //
  // Exit hooks, at a minimum should populate the external structure with the training values computed in the algoithm.
  //
  ReceiveEnableExitHooks (ModMrcData, Channel);

  return SUCCESS;
}

/*++

Routine Description:

  Based on the input type, this function will average out the 2x values for all ranks on the given channel.

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Minus1Type:     Selection of signal group to be averaged.
  Channel:        Channel to operate on.

Returns:

    Success
    Failure

--*/
STATUS
AverageDelay (
  IN MMRC_DATA   *ModMrcData,
  IN UINT8        Channel,
  IN UINT8        Minus1type
)
{
  UINT8   Rank;                    // Current Rank being operated on.
  UINT8   Strobe;                  // Current Strobe being operated on
  UINT32  TempValue;               // Temporary storage used for the Get/Set API.
  UINT32  TempValue2;              // Temporary storage used for the Get/Set API.
  UINT16  totalValue[MAX_STROBES]; // Contains the linear value for the given signal group with the application of the minus1.
  UINT8   Rank_present;            // Total number of ranks present on the given channel.
  UINT8   delayType;               // Signal group accessed via the Get/Set API.

  //
  // Initialize RankPresent count to 0.
  //
  Rank_present = 0;

  //
  // Based on the input parameter "Minus1type" determine which signal group will be accessed from with the Get/Set.
  //
  switch (Minus1type) {
    case RCVN_MIN:
      delayType = DELAYS_RCVN_DEL;
      break;
    case WDQS_MIN:
      delayType = DELAYS_WDQS_DEL;
      break;
    case WDQ_MIN:
      delayType = DELAYS_WDQ_DEL;
      break;
    default:
      return FAILURE;
  }

  //
  // Initialize the total values array to 0 for each strobe.
  //
  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
    totalValue[Strobe] = 0;
  }

  //
  // Loop through all active ranks on the channel.
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (ModMrcData->Channel[Channel].RankEnabled[Rank]) {
        //
        // Keep a count of the total number of ranks enabled on the channel.
        //
        Rank_present++;

        //
        // Loop through all the strobes modifying the linear value to include the minus1 value which impacts the linear value by
        // HALF_CLK if enabled and set to 1.
        //
        for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
          GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, delayType, CMD_GET_CACHE, &TempValue);
          GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, Minus1type, CMD_GET_CACHE, &TempValue2);

          //
          // If the minus1 is set to 1, modify the linear value by HALF CLK.
          //
          if (TempValue2 == 0) {
            //TempValue -= HALF_CLK;
            TempValue -= HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
          }
          totalValue[Strobe] += (UINT16) TempValue;
      }
    }
  }

  //
  // Loop through each strobe and rank setting the linear value to totalvalue/ranks... and set the minus1 to 1.
  //
  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
    TempValue = totalValue[Strobe] / Rank_present;
    if (totalValue[Strobe] % Rank_present) {
    	TempValue += 1;
    } 
    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, delayType, CMD_SET_VAL_FC_UC, &TempValue);
        TempValue2 = 1;
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, Minus1type, CMD_SET_VAL_FC_UC, &TempValue2);
    }
  }

  return SUCCESS;
}

/*++

Routine Description:

  Restores the minus1 and the linear receive enable values for all active ranks and strobes.  This function will
  be called once per channel so the restoration will only restore the provided channel.

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Minus1Type:     Selection of signal group to be averaged.
  Channel:        Channel to operate on.

Returns:

    Success
    Failure

--*/

STATUS
SetMinus1Select (
  IN MMRC_DATA   *ModMrcData,
  IN UINT8        Channel,
  IN UINT8        Minus1type
)
{
  UINT8  Rank;
  UINT8  Strobe;
  UINT16 maxValue[MAX_STROBES];
  UINT32 TempValue;
  UINT32 TempValue2;
  UINT8  delayType;

  //
  // Based on the input Minus1Type selection, determine the signal group to be modified with the Get/Set API.
  //
  switch (Minus1type) {
    case RCVN_MIN:
      delayType = DELAYS_RCVN_DEL;
      break;
    case WDQS_MIN:
      delayType = DELAYS_WDQS_DEL;
      break;
    case WDQ_MIN:
      delayType = DELAYS_WDQ_DEL;
      break;
    default:
      return FAILURE;
  }

  //
  // Initialize the maximum DQ and DQS variables to 0.
  //
  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe ++) {
    maxValue[Strobe] = 0;
  }

  //
  // For each strobe on the active ranks of the provided channel, determine the maximum linear delay and store it in the
  // maxDelay array based on the strobe index.
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (ModMrcData->Channel[Channel].RankEnabled[Rank]) {
      for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, delayType, CMD_GET_CACHE, &TempValue);

        if (TempValue > maxValue[Strobe]) {
          maxValue[Strobe] = (UINT16) TempValue;
        }
      }
    }
  }

  // Set. the 2x-1 values.
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (ModMrcData->Channel[Channel].RankEnabled[Rank]) {
      for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
          //
          // Get the current strobes current delay value.
          //
          GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, delayType, CMD_GET_CACHE, &TempValue2);

          //
          // Determine the required -1 value, if not 0 or 1, then an error should be reported.
          //
          //TempValue = ( ( (maxValue[Strobe] | MAXPI_VAL) - TempValue2) / HALF_CLK);
          TempValue = 0;//( ( (maxValue[Strobe] | MAXPI_VAL) - TempValue2) / HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency));
          if (TempValue > 1) {
             // ERROR 2x-1 Value is too large.
             while (1);
          }
          TempValue = 1 - TempValue;

          //
          // Set the minus1 value.
          //
          GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, Minus1type, CMD_SET_VAL_FC_UC, &TempValue);
          GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, Minus1type, CMD_GET_CACHE, &TempValue);
      }
    }
  }

  //
  // After setting the -1, value we need to modify the linear value appropriately... Such that if the minus1 value for the given
  // rank is 0, then the linear value should be increased by HALF_CLK.
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (ModMrcData->Channel[Channel].RankEnabled[Rank]) {
      for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, delayType, CMD_GET_CACHE, &TempValue2);
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, Minus1type, CMD_GET_CACHE, &TempValue);
        if (0 == TempValue) {
          TempValue2 += HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
        }
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, delayType, CMD_SET_VAL_FC_UC, &TempValue2);
      }
    }
  }

  return SUCCESS;
}

STATUS
PerformFifoReset (
  IN  OUT   MMRC_DATA     *ModMrcData,
  IN        UINT8         Channel,
  IN		UINT8         Rank
)
/*++

Routine Description:

  Resets the FIFOs in the PHY.  Its a simple routine but is called many times.

Arguments:

  ModMrcData:       Host structure for all data related to MMRC
  Channel:          Current Channel being examined.


Returns:

  Success
  Failure

--*/
{
  UINT8  Strobe;
  UINT32 TempValue;

  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
    TempValue = FIFO_RESET_ENABLE;
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, FIFORESET, CMD_SET_VAL_FC, &TempValue);
    MrcDelay (MICRO_DEL, 10);
    TempValue = FIFO_RESET_DISABLE;
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, FIFORESET, CMD_SET_VAL_FC, &TempValue);
  }

  return SUCCESS;
}

/*++

Routine Description:

  Performs the Recieve Enable calibration.  All results from the calibration are stored within the ModMrcData within the
  TrainingData substructure.  Channel is the only additional parameter passed in, and the calibration will be performed
  on the channel specified.

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Channel:        Channel to operate on.

Returns:

    Success
    Failure

--*/
STATUS
ReceiveEnable (
  IN MMRC_DATA   *ModMrcData,
  IN UINT8        Channel
)
{
  UINT8             DirectionFlag[MAX_STROBES]; // Direction PI's are changing when doing PI searching. 1->+, 0->-
  UINT8             TimeSample[MAX_STROBES];    // Sampled Value from the phase register.
  UINT8             Rank;                       // Rank being tested.
  UINT8             Strobe;                     // Stobe being tested.
  UINT16            Flag;                       // Flag used for final rcvn steps to Sample 1 on all strobes.
  UINT32            TempValue;                  // Temporary storage element used throughout the algorithm.
  UINT32            TotalFinishStrobes;         // Bitfields that
  STATUS 			Status = SUCCESS;
  PRINT_FUNCTION_INFO;


  //
  // Sets the bit fields that will be used to compare for completion of all stobes.  This could include ECC.
  //
  TotalFinishStrobes = (1 << ModMrcData->MaxDq) - 1;

  //
  // Entry Hooks
  //
  ReceiveEnableEntryHooks (ModMrcData, Channel);

  //
  // HIP Entry
  //
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_rcvn_list[0]);

  if (ModMrcData->EccEnabled) {
	  ReceiveEnableEntryHooks (ModMrcData, 1);
	  ModMrcData->Channel[1].Enabled = 1;
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_rcvn_list[0]);
  }


  //
  // Loop through all enabled ranks performing the rcvn enable training.
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
      if (ModMrcData->Channel[Channel].RankEnabled[Rank]) {
		  //
		  // Setup the CPGC engine to to do a single read from an address within the
		  // selectable rank.  The engine should be setup for LFSR mode.
		  //
#if CPGC_API
        CPGC_Setup(ModMrcData, Channel, Rank);
        CPGC_S_SetupSeq (ModMrcData, Channel, CPGC_SUBSEQINDEX_1, CPGC_SUBSEQINDEX_1, 0x1, 0x0);
#endif

      //
      // Set to 1 and also set the initial 2x clock to the default value.
      //
      for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
        TempValue = 1;
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RCVN_MIN, CMD_SET_VAL_FC_UC, &TempValue);
        TempValue = RCVN_INITIAL2XVAL * HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);

        if ((ModMrcData->CurrentDdrType == TYPE_LPDDR3) && (ModMrcData->CurrentFrequency == FREQ_1333)){
            TempValue = 11 * HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
        }

        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RCVN_DEL, CMD_SET_VAL_FC_UC, &TempValue);
      }

      //
      // Peform a Precharge all
      //
      JedecCmd (ModMrcData, Channel, Rank, (UINT8) JEDEC_PRECHARGEALL, (UINT32) 0xffffffff);

      //
      // Put all strobes into FIFO RESET.
      //
      for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
        TempValue = FIFO_RESET_ENABLE;
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, FIFORESET, CMD_SET_VAL_FC, &TempValue);
      }

      //
      // Large Fine RdLevelling
      //
      //RdWrLevelFineSearch (ModMrcData, Channel, Rank, RCVN_LARGE_STEP, RD_LEVELING, DirectionFlag);
      RdWrLevelFineSearch (ModMrcData, Channel, Rank, HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)/2, RD_LEVELING, DirectionFlag);

      //
      // Medium Fine RdLevelling
      //
      Status = RdWrLevelFineDirectionAndStep (ModMrcData, Channel, Rank, RD_LEVELING, DirectionFlag, HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)/4, (UINT8 * )  "MFne");

      if (Status == FAILURE) return FAILURE;

      //
      // For each strobe, change the direction Flag.
      //
      for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
        DirectionFlag[Strobe] = (~DirectionFlag[Strobe]) & 1;
      }

      //
      // Small Fine RdLevelling
      //
      RdWrLevelFineDirectionAndStep (ModMrcData, Channel, Rank, RD_LEVELING, DirectionFlag, RCVN_SMALL_STEP, (UINT8 * )  "SFne");

      //
      // Add 1/4 CLK to each LANE to put the RCVEN at the center of a 1.
      //
      for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RCVN_DEL, CMD_GET_CACHE, &TempValue);
        TempValue += QTR_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RCVN_DEL, CMD_SET_VAL_FC_UC, &TempValue);
      }

      //
      // Resample the DQS pins, and print the results.
      //
      SampleDQS (ModMrcData, Channel, Rank, RDLEVEL, TimeSample);

      //
      // Go through each strobe subtracting 1 clock if a 0 has not been already sampled.
      //
      for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
        if (TimeSample[Strobe] != 1) {
          // *** TODO
          
          return FAILURE;
        } else {
          GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RCVN_DEL, CMD_GET_CACHE, &TempValue);
          TempValue -= ONE_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
          GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RCVN_DEL, CMD_SET_VAL_FC_UC, &TempValue);
        }
		  }

      //
      // Initialize Flag, this will contain the bit-fields for each strobe that detected a 0.  THe comparison will be
      // with the variable TotalFinishStrobes which is set to all 1's for each stobe.
      //
      Flag = 0;

      //
      // Subtract 1 CLK until a 0 is sampled on all Strobe.
      //

      do {
        //
        // Get the sample for each strobe at the current delay.
        //
        SampleDQS (ModMrcData, Channel, Rank, RDLEVEL, TimeSample);

        //
        // Go through each strobe checking if a 0 is found. If found, set the approporiate bit-field in the flags.
        //
        for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
          //
          // If the current bit-field already sampled a 0, go onto the next strobe as this lane is complete.
          //
          if (Flag & (1 << Strobe) ) {
            continue;
          }

          //
          // If a 1 is sampled on the selected strobe, then a clock must be subtracted; otherwise, if a 0 is
          // sampled, the appropriate done bit shoud be set in the flag variable.
          //
          if (TimeSample[Strobe] == 1) {
            GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RCVN_DEL, CMD_GET_CACHE, &TempValue);
            //TempValue -= ONE_CLK;
            TempValue -= ONE_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
            GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RCVN_DEL, CMD_SET_VAL_FC_UC, &TempValue);
          } else {
            Flag |= (1 << Strobe );
          }
		    }
      } while (Flag != TotalFinishStrobes);

      //
      // Add 1/4 CLK to each LANE to put the RCVEN at the center of a 0.
      //
      for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RCVN_DEL, CMD_GET_CACHE, &TempValue);
        TempValue += QTR_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RCVN_DEL, CMD_SET_VAL_FC_UC, &TempValue);
      }

      //
      // Print out the results from the adding 1/4 clk.
      //
      
      	  if (ModMrcData->CurrentDdrType == TYPE_LPDDR3){
      		  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
      			GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RCVN_DEL, CMD_GET_CACHE, &TempValue);
      			switch (ModMrcData->CurrentFrequency){
      			case FREQ_800:
      			case FREQ_800_DDLL:
      			case FREQ_800_DDLL_BYP_MPLL:
      				break;
      			case FREQ_1066:
      				TempValue -= 13;
      				break;
      			case FREQ_1066_DDLL:
      				TempValue -= 5;
      				break;
      			case FREQ_1333:
     				TempValue += 20;
      				break;
      			default:
      				break;

      			}
      			GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RCVN_DEL, CMD_SET_VAL_FC_UC, &TempValue);
      		  }
            }
    } // End of Rank Check
  }

  //
  // Disable CPGC so that cpu access to memory is enabled.
  //
#if CPGC_API
    CPGC_S_Disable (ModMrcData, Channel);
#endif

  //
  // Resolve the minus1 select.
  //
  SetMinus1Select (ModMrcData, Channel, RCVN_MIN);
#if Rank2Rank_SHARING_DISABLED
#else
  //
  // If Rank 2 Rank is enabled, then we need to average out all the Rcvn across the ranks and set all ranks to the same Value.
  //
  AverageDelay (ModMrcData, Channel, RCVN_MIN);
#endif

  //
  // HIP Exit routine.
  //
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_rcvn_list[1]);

  //
  // Exit hooks, at a minimum should populate the external structure with the training values computed in the algoithm.
  //
  ReceiveEnableExitHooks (ModMrcData, Channel);

  if (ModMrcData->EccEnabled) {
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_rcvn_list[1]);
	  ReceiveEnableExitHooks (ModMrcData, 1);
  }

  return Status;
}

/*++

Routine Description:

  Does a read or write sample from the provided channel and rank based on the the readwriteflag.  THe results are reported
  in the finalResults variable which is an array of bytes with each element being a strobelane.  The number of samples being
  taken is a maximum of NUM_SAMPLES, but if the sample set has SAMPLETHRESH of all 1's or 0's, the sampling can stop earlier.

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Channel:        Channel to operate on.
  Rank:           Rank to operate on.
  ReadWriteFlag:  Switch between doing a read request or a write request to the DRAMs.
  FinalResults:   Array of chars proving the sample results for each strobe lane, being 1 or 0.

Returns:

    Success
    Failure

--*/
static STATUS
SampleDQS (
  IN  MMRC_DATA  *ModMrcData,
  IN  UINT8       Channel,
  IN  UINT8       Rank,
  IN  UINT8       ReadWriteFlag,
  OUT UINT8      *FinalResults
)
{
  UINT8         Strobe;               // Strobe Index for currently active.
  UINT32        Address;              // Location of memory to read from when sampling DQS.
  UINT32        TempValue;            // Place holder for the read results, not used just required for the read.
  UINT8         Sample;               // Result of reading the Sample register for phase alignment.
  UINT8         SampleIndex;          // Loop Counter for performing the total number of samples.
  INT8          Results[MAX_STROBES]; // Internal signed accumulation of the results, this will Count
                                      // +1 when a 1 is sampled, -1 when a 0 is sampled.
  UINT8         SignalType;           // Sampler Type.
  UINT32        FinishBL;             // Holds whether a specific strobe has completed the total required samples.
  UINT32        TotalFinishStrobes;   // Strobe signature when all strobes have passed the required number of samples.

  //
  // Initialize variables.
  //
  TotalFinishStrobes = (1 << ModMrcData->MaxDq) - 1;
  TempValue = 0;
  FinishBL = 0;

  //
  // Based on the rdwr Flag, determine the signal Type needed to read for the sampler.
  //
  if (ReadWriteFlag == RDLEVEL) {
    SignalType = RCVN_SMP;
  } else {
    SignalType = WDQS_SMP;
  }

  //
  // Obtain an Address that falls within the specified channel/rank.
  //
  Address = GetAddress (ModMrcData, Channel, Rank);

  //
  // Zero out the signed accumulated results before starting the sampling intervals.
  //
  MmrcMemSet(Results, 0, ModMrcData->MaxDq);

  //
  // Set the sample index to the maximum number of samples required.  This is a project specific compile switch.
  //
  SampleIndex = NUMSAMPLES;

  //
  // Loop on the total number of samples, but sampling may stop earlier if threshold of 1's or 0's is reached.
  //
  while (--SampleIndex) {
    //
    // Perform the required memory access.
    //
#if CPGC_API
      CPGC_S_ClearErrors (ModMrcData, Channel);
      CPGC_S_StartTest (ModMrcData, Channel, Rank);
      while (CPGC_S_PollTest (ModMrcData, Channel) == CPGC_STS_TEST_BUSY) {
      }
      CPGC_S_StopTest (ModMrcData, Channel);
#else	//CPGC_API
      if (ReadWriteFlag == RDLEVEL) {
        TempValue = * ( (volatile UINT32 *) Address);
      } else {
        * ( (volatile UINT32 *) Address) = TempValue;
      }
#endif	//CPGC_API

    //
    // Go through each strobe lane, checking if the threshold has been reached.
    for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {

      //
      // If the strobe lane has hit its threshold, then go to the next strobe lane.
      //
      if (FinishBL & (1 << Strobe) ) {
        continue;
      }

      //
      // Read the sample register.
      //
      GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, SignalType, CMD_GET_REG, &TempValue);
      Sample = (UINT8) TempValue;

      //
      // Accumulate the results for that strobe lane, by adding a 1 if a 1 is sample, subtracting a 1 if a 0 is sampled.
      //
      Results[Strobe] += ( ( Sample * 2) - 1 );

      //
      // If the resultant count is above the threshold, then set the bit-field for the Strobe lane to a 1.
      //
      if ( (Results[Strobe]  >= SAMPLETHRESH) || Results[Strobe]  <= (SAMPLETHRESH * -1) ) {
        FinishBL |= (1 << Strobe );
      }
    }

    //
    // If all strobe lanes have hit the threshold, then break out of the loop early.
    //
    if (FinishBL == TotalFinishStrobes) {
      break;  //finished all bytelane, get all 1 or 0
    }
  }  // while (--SampleIndex)

  //
  // The return results, should see if more 0's then return 0 otherwise return 1.. Zero implies take 1.
  //
  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
    if (Results[Strobe] >= 0) {
      FinalResults[Strobe] = 1;
    } else {
      FinalResults[Strobe] = 0;
    }
  }

  return SUCCESS;
}

/*++

Routine Description:

  Given a direction and step size, the routine will modify the PI delays for each strobe of the given Channel and Rank
  until the desired level is sampled.  The desired level and the direction that will be taken by the incrment
  is stored in the direction Flag.

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Channel:        Channel to operate on.
  Rank:           Rank to operate on.
  rdwrFlag:       RDLEVEL->Read Request, WRLEVEL->Write Request.
  directionFlag:  Specifies the direction the PI should move for every strobe when doing marginning.
  IncrementSize:  The delta step size to apply when margining.

Returns:

    Success
    Failure

--*/
static STATUS
RdWrLevelFineDirectionAndStep (
  IN MMRC_DATA  *ModMrcData,
  IN UINT8       Channel,
  IN UINT8       Rank,
  IN UINT8       ReadWriteFlag,
  IN UINT8      *DirectionFlag,
  IN UINT8       IncrementSize,
  IN UINT8      *Label
)
{
  UINT16              FoundFlagTotal;             // Flag to state the number of strobes that have sampled the desired Value.
  UINT8               Test1[MAX_STROBES];         // Storage for the actual sampled Value.
  UINT8               Strobe;                     // Variable for looping on all valid strobes.
  UINT32              TempValue;                  // 32-bit temporary storage Value.
  UINT8               SignalType;                 // Selection for signal group to modify.
  UINT32              TotalFinishStrobes;

  TotalFinishStrobes = (1 << ModMrcData->MaxDq) - 1;

  //
  // Determine the signal Type.
  //
  if (ReadWriteFlag == RD_LEVELING) {
    SignalType = DELAYS_RCVN_DEL;
  } else {
    SignalType = DELAYS_WDQS_DEL;
  }

  // Search the delays until all strobes have found the desired level, DirectionFlag.
  FoundFlagTotal = 0;

  do {
	    for (Strobe=0; Strobe< ModMrcData->MaxDq; Strobe++) {
	    	
	    	if (FoundFlagTotal & (1 << Strobe) ) {
	    		continue;  //if finished
	    	}
	    	
	    	GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, SignalType, CMD_GET_CACHE, &TempValue);
	    	TempValue += (DirectionFlag[Strobe] * 2 - 1) * IncrementSize;
	    	GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, SignalType, CMD_SET_VAL_FC_UC, &TempValue);

	    	if ( TempValue > (UINT32)(7*ONE_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)) ){
	    		return FAILURE;
	    	}
        }
	    // Read the current Sample once.
	    // Read the current phase Sample storing the result in the Test1 array.
	    SampleDQS (ModMrcData, Channel, Rank, ReadWriteFlag, Test1);

	    
	    for (Strobe=0; Strobe< ModMrcData->MaxDq; Strobe++) {
	    	if (FoundFlagTotal & (1 << Strobe) ) {
	    		continue;  //if finished
	    	}

	    	if (Test1[Strobe] == DirectionFlag[Strobe]) {
	    		FoundFlagTotal |= (1 << Strobe );
	    	}
	    }

	    
	    // Continue looping until all the strobes report back true.
  } while (FoundFlagTotal != TotalFinishStrobes);


  return SUCCESS;
}

/*++

Routine Description:

  For the provided channela and rank, the function will perform increments of coarsestep to the linear delays using the
  direction specified in diretionFlag, which if 0 implies negative, 1 implies positive.  The sampler will do either a read
  or a write based on teh read/write flag.

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Channel:        Channel to operate on.
  CoarseStep:     PI delta when determining the starting PI and direction.
  ReadWriteFlag:  RDLEVEL->Read Request, WRLEVEL->Write Request.
  DirectionFlag:  Specifies the direction the PI should move for every strobe when doing marginning.

Returns:

    Success
    Failure

--*/
static STATUS
RdWrLevelFineSearch (
  IN MMRC_DATA   *ModMrcData,
  IN UINT8        Channel,
  IN UINT8        Rank,
  IN UINT32       CoarseStep,
  IN UINT8        ReadWriteFlag,
  IN UINT8       *DirectionFlag
)
{
  UINT8             Strobe;                    // Current stobe operating on.
  UINT8             TimeSample0[MAX_STROBES];  // A sample results for all strobe lanes from doing one memory access.
  UINT8             TimeSample1[MAX_STROBES];  // A sample result for all strobe lanes from doing one memory access.
  UINT32            TempValue;                 // Temporary placehold for the Get/Set assignments.
  UINT8             SignalType;                // Input to the Get/Set based on the Readwriteflag.

  //
  // Determine the signal Type used for the Get/Set signal.
  //
  if (ReadWriteFlag == RD_LEVELING) {
    SignalType = DELAYS_RCVN_DEL;
  } else {
    SignalType = DELAYS_WDQS_DEL;
  }

  //
  // Determine the signal Value by sampling the DQS for each strobe storing the results in TimeSample0.
  //
  SampleDQS (ModMrcData, Channel, Rank, ReadWriteFlag, TimeSample0);

  //
  // Increment the Delay settings for each strobe by "CoarseStep" which is a passed in parameter.
  //
  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, SignalType, CMD_GET_CACHE, &TempValue);
    TempValue += CoarseStep;
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, SignalType, CMD_SET_VAL_FC_UC, &TempValue);
  }

  //
  // Determine the new signal Value by resampling the DQS for each strobe, storeing the results in pTimesample1.
  //
  SampleDQS (ModMrcData, Channel, Rank, ReadWriteFlag, TimeSample1);

  //
  // Based on the results from the samples, either keep the PI on its current pi, or restore it to the sample0
  // setting (- coarseStepSize).  Also, set the direction Flag for each strobe.
  //
  for (Strobe=0; Strobe< ModMrcData->MaxDq; Strobe++) {
    if (TimeSample0[Strobe] == 0 && TimeSample1[Strobe] == 0) {
      //
      // Set to timeSample1 and set the direction to count up for a 1.
      //
      DirectionFlag[Strobe] = COUNT_UP_FOR_ONE;
    } else if ( TimeSample0[Strobe] == 0 && TimeSample1[Strobe] == 1) {
      //
      // Set to timeSample0 and set the direcdtion to count up for a 1.
      //
      GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, SignalType, CMD_GET_CACHE, &TempValue);
      TempValue -= CoarseStep;
      GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, SignalType, CMD_SET_VAL_FC_UC, &TempValue);
      DirectionFlag[Strobe] = COUNT_UP_FOR_ONE;
    } else if ( TimeSample0[Strobe] == 1 && TimeSample1[Strobe] == 0) {
      //
      // Set to timeSample0 and set the direction to count down for a 0.
      //
      GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, SignalType, CMD_GET_CACHE, &TempValue);
      TempValue -= CoarseStep;
      GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, SignalType, CMD_SET_VAL_FC_UC, &TempValue);
      DirectionFlag[Strobe] = COUNT_DN_FOR_ZERO;
    } else {
      //
      // Set to timeSample0 and the direction to count down for a 0.
      //
      GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, SignalType, CMD_GET_CACHE, &TempValue);
      TempValue -= CoarseStep;
      GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, SignalType, CMD_SET_VAL_FC_UC, &TempValue);
      DirectionFlag[Strobe] = COUNT_DN_FOR_ZERO;
    }
  }

  return SUCCESS;
}


/*++

Routine Description:

   Given a pointer to a serial list of assignments which can include the "encoded" assignment, the name of the      *
   register, and the name of the fields, the routine will go though each assignment decoding the, and performing    *
   the request Action.  The printout at this level is only for the header for each assignment.                      *

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Channel:        Channel to operate on.
  PhyInitList:    CHar array of encoded assignments that will be decoded, tested, and executed.
Returns:

    Success
    Failure

--*/
STATUS
DecodeAndExeRegAssignment (
  MMRC_DATA         *ModMrcData,
  UINT8              Channel,
  PHYINIT_LIST       PhyInitList
)
{
  UINT32   LengthCurrentAssign;                // Holds the length of the assignment in bytes.
  UINT32   PfctValue;                          // Based on the current PFCT, holds the Value for the current assignment.
  UINT8    PfctStatus;                         // Flag to determine if any of the PFCT conditions were met on the current assign.
  UINT8    Action;                             // Holds the Action of the current assign.
  UINT8    Counter = 0;                        // Counter for displaying results on the output when enabled.
  UINT8    SizeLocation;                       // Holds the length in bytes of the location portion of the assign.
  UINT8    SizeAction;                         // Holds the length in bytes of the Action portion of the assign.
  UINT8    *CurrentEncLoc;                     // Holds the location of the current assign. being decoded.
  UINT32   Delay;                              // Holds the Delay for the assignment.  0 if not needed.
  PFCT_VARIATIONS   PFCTVariations[MaxPfct];   // Holds all permutations of PFCTs

  // The next two entries holds the pointer to the array of register and fields names which are used for displaying
  // when enabled.
  //  const regstr_t    *pRegstr    = PhyInitList.regString;
  //  const detregstr_t    *pDetRegstr = PhyInitList.detRegString;

  //
 // Decode the PFCT variations
  //
  CreatePFCTSel (ModMrcData, PFCTVariations);

  //
  // Begin by assigning the Current encoded location to the start of the assignments.
  //
  CurrentEncLoc                     = (UINT8 *) PhyInitList.regAssign;

  //
  // Preset the Delay with 0, it will be changed to a non-zero Value if setup.
  //
  Delay = 0;


  //
  // The last assignment of the assignment list has the MACRO "ASSIGNDONE" which is the signature to stop decoding.
  //
  while (*CurrentEncLoc != ASSIGNDONE) {

    //
    // Determine the length of the Location field in bytes.
    //
    SizeLocation = (LOC_FLAGS_U (CurrentEncLoc) == 0) ? (8) : (8 + ( (MAX_RANKS - 1) * 2) );

    //
    // Determine the length of the Action field in  bytes.
    // The least significant bit being a one implies their is a Delay.
    //
    SizeAction = ( (CurrentEncLoc[SizeLocation] & 1) == 1) ? (5) : (1);

    //
    // If the size is 5, then there is a Delay, extract the Delay Value.
    //
    if (SizeAction == 5) {
      Delay = * (UINT32 *) &CurrentEncLoc[SizeLocation + 1];
    }

    //
    // Extract the Action from the current assign.
    //
    Action = CurrentEncLoc[SizeLocation];

    //
    // The total length of the current assignment is the sum of the location fields, the Action fields, and the Value fields.
    //
    if (Action != DELAY) {
      LengthCurrentAssign = SizeLocation + SizeAction
                            + DecodeValue (ModMrcData, Channel, &CurrentEncLoc[SizeLocation + SizeAction], &PfctValue, &PfctStatus, PFCTVariations) ;
    } else {
      LengthCurrentAssign = SizeLocation + SizeAction;
      PfctStatus = TAKE_ACTION;
      PfctValue = 0;
    }

    //
    // If the Action was a ACTIVE Action, then perform the assignment.
    //
    if (PfctStatus == TAKE_ACTION) {

      //
      // If the assignment is a DELAY, simply print out the delay and then delay, otherwise take the action
      // Which will do the correcdt assignment and print out the value.
      //
      if (Action == DELAY) {
        
        MrcDelay (NANO_DEL, Delay);
      } else {
        AssignValue (ModMrcData, Channel, CurrentEncLoc, PfctValue, Action, Delay, Counter, PhyInitList);
      }

      //
      // Increment the Counter for the next assignment.
      //
      Counter++;
    }


    //
    // Add the length of the current assignment to the current encoded location to the the location of the next assignment.
    //
    CurrentEncLoc += LengthCurrentAssign;
  }

  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: DecodeValue()                                                                                   *
//*********************************************************************************************************************
// Date: 04/25/2012                                                                                                   *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//    currentAssign:  Pointer to the beginning of the Value section of the current assignment.                        *
//    Value:          Return Value that was decided upon.                                                             *
//    Status:         Flag stating wether the Value is valid or not, non-valid implys no condition to set.            *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Length: Returns the length of the encoded Value section for the current assignment.                             *
//*********************************************************************************************************************
// Description:                                                                                                       *
//   The Value portion of the DDRPHY Assignment contains a dPFCT Flag which is one byte, followed by one or more      *
//   conditional/Value pairs.  If the PFCT is 0, then the assignment is considered global, and a condition is not     *
//   required, only a Value.  The condition is based on the current condition of the system and the condition of the  *
//   Flag, if the condition is met, the Value is taken.  In this way, the Value is in esense a if/then/elseif/else... *
//   system.
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
static UINT32
DecodeValue (
  MMRC_DATA *ModMrcData,
  UINT8               Channel,
  UINT8               *CurAssignValuePtr,
  UINT32              *Value,
  UINT8               *Status,
  PFCT_VARIATIONS    *PFCTVariations
)
{
  UINT8   PfctFlag;         // Holds the Value of the PFCT which is the first byte within the Value section.
  UINT8   PfctDynamic;      // Holds the dynamic bit portion of the PFCT.
  UINT8   NumCondBytes;     // Holds the number of conditional-Value pairs in the VALUE portion of the assignment.
  UINT8   TestVal;          // PFCT Bits based on the runtime environment.
  UINT8   NumberCVs;    // Number of Condition/Value combinations.
  UINT8   Index;
  UINT8   Counter;
  UINT32  PfctValue;
  UINT8   CondFlag;

  // First portion of the Value section  is the dPFCT Bits.
  PfctFlag   = (CurAssignValuePtr[0] & 0x0f);  // The lower 4 Bits of the PFCT Flag are used.
  PfctDynamic = (CurAssignValuePtr[0] >> 7) & 1; // The MSb of the PFCT Flag is the dynamic Flag.

  // Assume the Status is that the assignment will be taken.
  *Status = TAKE_ACTION;

  // If the pfct condition is 0's, then the assignment MUST be global, there will be no condition just the 4 byte
  // Value.
  if (PfctFlag == PFCT_GLOBAL) {
    // Get the PFCT Value from the fields.  This can be either the actual Value (PfctDynamic = 0) or can
    // be an Index into the dynamic table (PfctDynamic = 1).
    PfctValue = * (UINT32 *) (&CurAssignValuePtr[1]);
    if (PfctDynamic == 1) {
      DynamicAssignment1 (ModMrcData, Channel, (UINT8) PfctValue, &PfctValue);
    }
    //Copy the decided upon Value to the element to be returned from the function.
    *Value = PfctValue;

    // return the Count, when global, just the Flag and the Value which is 5 bytes.
    return 5;
  }

  // To reach this point, the PFCT Flag must have been non-zero so non-global.
  // The next byte is the total number of conditional-Value pairs..
  NumberCVs = CurAssignValuePtr[1];

  Index = 2;
  for (Counter = 0; Counter < NumberCVs; Counter++) {
    NumCondBytes = PFCTVariations[PfctFlag].length;
    CondFlag = 1;
    //  if ((Pfct->FreqDependent == 1) && Pfct->Frequency) & Condition->Frequency && Pcft->Platform & Condition->Platform && )
    while (NumCondBytes > 0 && CondFlag == 1) {
      TestVal = (UINT8) ( (PFCTVariations[PfctFlag].pfct >> ( (NumCondBytes - 1) * 8) ) & 0xff);
      if ( (CurAssignValuePtr[Index + NumCondBytes - 1]  & TestVal) != TestVal) {
        CondFlag = 0;
      }
      NumCondBytes--;
    }
    if (CondFlag == 1) {
      PfctValue     =   * (UINT32 *) &CurAssignValuePtr[Index + PFCTVariations[PfctFlag].length];
      if (PfctDynamic == 1) {
        DynamicAssignment1 (ModMrcData, Channel, (UINT8) PfctValue, &PfctValue);
      }
      *Value = PfctValue;
      return 2 + NumberCVs * (PFCTVariations[PfctFlag].length + 4);
    } else {
      Index += PFCTVariations[PfctFlag].length + 4;
    }
  }

  // If this portion of the code is reached, then the Action is not needed.
  *Status = SKIP_ACTION;
  PfctValue = 0xffffffff;
  return 2 + NumberCVs * (PFCTVariations[PfctFlag].length + 4);
}

//*********************************************************************************************************************
// Funtion Name: AssignValue()                                                                                   *
//*********************************************************************************************************************
// Date: 04/25/2012                                                                                                   *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         SUCCESS/FAIL result                                                                             *
//*********************************************************************************************************************
// Description:                                                                                                       *
//                             *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
static STATUS
AssignValue (
  MMRC_DATA           *ModMrcData,
  UINT8                Channel,
  UINT8                *Ptr,
  UINT32               Value,
  UINT8                Action,
  UINT32               Delay,
  UINT8                Counter,
  PHYINIT_LIST        Ddrphyinit_block
)
{
  UINT8     LoopMod;
  UINT8     LoopRank;
  UINT8     LoopByteLaneWithinModule;
  UINT8     NumMod;
  UINT8     NumRank;
  UINT8     NumByteLaneWithinModule;
  UINT16    RankOffsetByte = 0;
  UINT16    RankOffsetBit = 0;
  UINT16    ByteLaneWithinModuleOffsetByte = 0;
  UINT16    ByteLaneWithinModuleOffsetBit = 0;
  UINT8     Module;
  UINT8     Rank;
  UINT8     ByteLaneWithinModule;
  IOSF_REG IosfReg;
  UINT8     LoopCounter = 0;
  UINT8     SkipIosf;
  UINT8     PerChannelAssignFlag;

#if DDRPHYINIT_DETAILS
  UINT8  sLoop, sb, eb;
#endif
  UINT32 readValue;
  PerChannelAssignFlag = (Ptr[7] & 0x80) >> 7;
  LoopMod = (Ptr[7] & 0x40) >> 6;
  LoopRank = (Ptr[7] & 0x1f) > 0 ? 1 : 0;
  LoopByteLaneWithinModule = (Ptr[6] & 0x1f) > 0 ? 1 : 0;

  //
  // If the register assignment has the GlobalAssignFlag set and the Per channel assignment flag was not set, then return.
  //

  if (LoopRank == 1)  {
    //*** TODO review
    RankOffsetByte = gRankToRankOffset[ (Ptr[7] & 0x1f) >> 0].byteoffset;
    RankOffsetBit  = gRankToRankOffset[ (Ptr[7] & 0x1f) >> 0].bitoffset;
  }

  if (LoopByteLaneWithinModule == 1)  {
    //** TODO review
    ByteLaneWithinModuleOffsetByte = gByteLaneOffsetWithinModule[ (Ptr[6] & 0x1f) >> 0].byteoffset;
    ByteLaneWithinModuleOffsetBit  = gByteLaneOffsetWithinModule[ (Ptr[6] & 0x1f) >> 0].bitoffset;
  }

  if (LoopMod == 1) {
    NumMod = MAX_DQ_MODULES;
  } else {
    NumMod = 1;
  }
  if (LoopRank == 1) {
    NumRank = MAX_RANKS;
  } else {
    NumRank = 1;
  }
  if (LoopByteLaneWithinModule == 1) {
    NumByteLaneWithinModule = MAX_BYTELANES_PER_DQ_MODULE;
  } else {
    NumByteLaneWithinModule = 1;
  }

  if (ModMrcData->Channel[Channel].Enabled) {
    for (Module = 0; Module < NumMod; Module++) {
      if (BROADCAST_SUP && Module > 0) {
        SkipIosf = 1;
      } else {
        SkipIosf = 0;
      }
      for (Rank = 0; Rank < NumRank; Rank++) {
        if ( (LoopRank == 1 && (ModMrcData->Channel[Channel].RankEnabled[Rank] == 1) ) || LoopRank == 0) {
          for (ByteLaneWithinModule = 0; ByteLaneWithinModule < NumByteLaneWithinModule; ByteLaneWithinModule++) {
            LoopCounter++;
			if (PerChannelAssignFlag != 0)
			{
				IosfReg.offset = * (UINT16 *) Ptr + Channel * CHANNEL_OFFSET + Module * MODULE_OFFSET + Rank * RankOffsetByte + ByteLaneWithinModule * ByteLaneWithinModuleOffsetByte;
			}
			else
			{
				IosfReg.offset = * (UINT16 *) Ptr + Module * MODULE_OFFSET + Rank * RankOffsetByte + ByteLaneWithinModule * ByteLaneWithinModuleOffsetByte;
			}
            IosfReg.mask   = (* (UINT32 *) &Ptr[2] << (Rank * RankOffsetBit + ByteLaneWithinModule * ByteLaneWithinModuleOffsetBit) ) & 0xffffffff;
            IosfReg.sb      = 0;

            if (Module == 0 && LoopMod == 1 && BROADCAST_SUP) {
              
            } else if (LoopCounter > 1 && SkipIosf == 1) {
              
            } else {
              
            }

            if (Action == SET || Action == SET_DL)  {
              if (SkipIosf == 0) {
                MemFieldWrite (ModMrcData, DDRIO, Channel, IosfReg, Value);
              }
              if (Action == SET_DL) {
            	  MrcDelay (NANO_DEL, Delay);
              }
            } else if (Action == POLL) {
              do {
                readValue = MemFieldRead (ModMrcData, DDRIO, Channel, IosfReg);
              } while (readValue != Value);
            } else if (Action == CHECK) {
            	readValue = MemFieldRead (ModMrcData, DDRIO, Channel, IosfReg);
            	if (readValue != Value) {
                    IoOut8(0xCF9, 0xE);
                    MRC_DEADLOOP();
            	}
            }
          }
        }
      }
    }
  }
  return SUCCESS;
}

/*++

Routine Description:

  Performs the Fine Write Leveling.  This is reusing many of the same routines used with the Receive Enable
  calibration.

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Channel:        Channel to operate on.

Returns:

    Success
    Failure

--*/
STATUS
FineWriteLeveling (
  IN MMRC_DATA   *ModMrcData,
  IN UINT8        Channel
)
{
  UINT8   DirectionFlag[MAX_STROBES]; // Direction PI's are changing when doing PI searching. 1->+, 0->-
  UINT8   Rank;                       // Current Rank being operated on.
  UINT8   Strobe;                     // Current Strobe being operated on.
  UINT32  TempValue;                  // Temporary place value for Get/Set API.
  UINT8   ConfigureRank;              // Flag stating which rank is currently being configured.
  STATUS  Status = SUCCESS;
  PRINT_FUNCTION_INFO;

  //
  // Entry Hooks
  //
  FineWriteLevelingEntryHooks (ModMrcData);

  //
  // HIP Entry
  //
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_wrlvlfine_list[0]);

  if (ModMrcData->EccEnabled) {
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_wrlvlfine_list[0]);
  }


  //
  // Loop through all enabled ranks performing the rcvn enable training.
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (ModMrcData->Channel[Channel].RankEnabled[Rank]) {

      //
		  // Setup the CPGC engine to to do a single read from an address within the
		  // selectable rank.  The engine should be setup for LFSR mode.    #if CPGC_API
      //
#if CPGC_API
      CPGC_Setup(ModMrcData, Channel, Rank);
      CPGC_S_SetupSeq (ModMrcData, Channel, CPGC_SUBSEQINDEX_0, CPGC_SUBSEQINDEX_0, 0x1, 0x0);
#endif

      //
      // Issue the ForceODT ON command and print out the label.
      //
      if (ModMrcData->CurrentDdrType <= TYPE_DDR3L_ECC) {
    	  ForceODT (ModMrcData->MrcDebugMsgLevel, Channel, Rank, FORCEODT_ON);
      }
      //
      // Initialize the DQ and DQS PtrMinus1Select for each strobe, and print out the label.
      //
      for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
        TempValue = 1;
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQ_MIN, CMD_SET_VAL_FC_UC, &TempValue);
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQS_MIN, CMD_SET_VAL_FC_UC, &TempValue);
      }
      
      //
      // Issue a precharge all.
      //
      JedecCmd (ModMrcData, Channel, Rank, (UINT8) JEDEC_PRECHARGEALL, (UINT32) 0xffffffff);

      //
      // Put the active rank in WL Mode, all others enabled ranks in QBUF Off.
      // skipping the disabled ranks.
      //
      if (ModMrcData->CurrentDdrType <= TYPE_DDR3L_ECC) {
    	  for (ConfigureRank = 0; ConfigureRank < MAX_RANKS; ConfigureRank++) {
    		  if (ModMrcData->Channel[Channel].RankEnabled[ConfigureRank]) {
				//
				// Initialize the Jedec Command variable with the MR1 bits and also set the driver impedance.
				 //
				TempValue = JEDEC_MR1;
				TempValue |= JEDEC_OUTPUTDRIVERIMPEDANCE;     // Set Output Driver Impendance Control to RZQ/7

				//
				// Configured ranks set RTT_TARGET and wr levellin, non-target ranks set RTT_NONTARGET & QBUF disable.
				//
				if (Rank == ConfigureRank) {
					TempValue |= JEDEC_RTTTARGET;
					TempValue |= WRITELEVEL_ENABLE;
				} else {
					TempValue |= JEDEC_RTTNONTARGET;
					TempValue |= JEDEC_QBUF_DIS;
				}

				//
				// Using the built command in TempValue, send the JEDEC command out.
				//
				JedecCmd (ModMrcData, Channel, ConfigureRank, JEDEC_LMR, TempValue);
    		  }
    	  }
        } else {
              /* Issue MRW( all MRs ) Command */
              JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_MR2, 0x1 );
              MrcDelay (NANO_DEL, 15);                    //delay 10 us

             JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_NOP, 0x0 );
        }

      //
      // Put the phy into write levelling mode.
      //
      TempValue = PHYENTERWRLVL;
      GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, 0, 0, 0, WRL_MODE, CMD_SET_VAL_FC, &TempValue);
      if (ModMrcData->EccEnabled){
    	  GetSetDataSignal (ModMrcData, 0, 1, 0, Rank, 0, 0, 0, WRL_MODE, CMD_SET_VAL_FC, &TempValue);
      }

      //
      // Set WDQS to WCLK initially for each strobe.
      //
      
      for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WCLK_DEL, CMD_GET_REG, &TempValue);
        GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQS_DEL, CMD_SET_VAL_FC_UC, &TempValue);
      }

      //
      // Large Fine WrLevelling
      //
      //RdWrLevelFineSearch (ModMrcData, Channel, Rank, RCVN_LARGE_STEP, WR_LEVELING, DirectionFlag);
      RdWrLevelFineSearch (ModMrcData, Channel, Rank, HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)/2, WR_LEVELING, DirectionFlag);

      //
      // Medium Fine WrLevelling
      //
      Status = RdWrLevelFineDirectionAndStep (ModMrcData, Channel, Rank, WR_LEVELING, DirectionFlag, HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)/4, (UINT8 * ) "MFne");

      if (Status == FAILURE) return FAILURE;
      //
      // For each strobe, change the direction Flag.
      //
      for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
        DirectionFlag[Strobe] = (~DirectionFlag[Strobe]) & 1;
      }

      //
      // Small Fine WrLevelling
      //
      Status = RdWrLevelFineDirectionAndStep (ModMrcData, Channel, Rank, WR_LEVELING, DirectionFlag, RCVN_SMALL_STEP, (UINT8 * ) "SFne");

      if (Status == FAILURE) return FAILURE;
      //
      // Take the phy out of write levelling mode.
      //
      
      TempValue = PHYEXITWRLVL;
      GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, 0, 0, 0, WRL_MODE, CMD_SET_VAL_FC, &TempValue);
      if (ModMrcData->EccEnabled){
    	  GetSetDataSignal (ModMrcData, 0, 1, 0, Rank, 0, 0, 0, WRL_MODE, CMD_SET_VAL_FC, &TempValue);
      }

      //
      // Take the DRAMS out of write levelling mode.
      //
      if (ModMrcData->CurrentDdrType <= TYPE_DDR3L_ECC) {
		  for (ConfigureRank = 0; ConfigureRank < MAX_RANKS; ConfigureRank++) {
			if (ModMrcData->Channel[Channel].RankEnabled[ConfigureRank] ) {
				TempValue = JEDEC_MR1;
				TempValue |= JEDEC_OUTPUTDRIVERIMPEDANCE;
				if ((ModMrcData->Channel[1].Enabled == 0) && (ModMrcData->MemoryDown == 1)){
					if ((ModMrcData->Channel[Channel].RankEnabled[0] == 1) && (ModMrcData->Channel[Channel].RankEnabled[1] == 0)){
#if defined DISABLED_RTT_NOM && DISABLED_RTT_NOM
						TempValue |= JEDEC_RTTTARGET_0;
#else
						TempValue |= JEDEC_RTTTARGET_120;
#endif
					}
				} else {
#if defined DISABLED_RTT_NOM && DISABLED_RTT_NOM
					TempValue |= JEDEC_RTTTARGET_0;
#else
					TempValue |= JEDEC_RTTTARGET_40;
#endif
				}



				JedecCmd (ModMrcData, Channel, ConfigureRank, JEDEC_LMR, TempValue);
			}
		  }
        } else {
              /* Issue MRW( all MRs ) Command */
              JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_MR2, 0x0 );
              MrcDelay (NANO_DEL, 15);                    //delay 10 us

             JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_NOP, 0x0 );

           }

      //
      // Force ODT Off.
      //
      if (ModMrcData->CurrentDdrType <= TYPE_DDR3L_ECC) {
    	  ForceODT (ModMrcData->MrcDebugMsgLevel, Channel, Rank, FORCEODT_OFF);
      }
    }
  }

  //
  // Disable CPGC
  //
#if CPGC_API
  CPGC_S_Disable (ModMrcData, Channel);
#endif

  //
  // HIP Exit
  //
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_wrlvlfine_list[1]);

  //
  // Exit Hook
  //
  FineWriteLevelingExitHooks (ModMrcData);

  if (ModMrcData->EccEnabled){
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_wrlvlfine_list[1]);
  }
  return Status;
}

//*********************************************************************************************************************
// Funtion Name: WrLvlFineRestore()                                                                              *
//*********************************************************************************************************************
// Date: 04/25/2012                                                                                                   *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         SUCCESS/FAIL result                                                                             *
//*********************************************************************************************************************
// Description:                                                                                                       *
//                             *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
STATUS
WriteLevelingRestore (
  MMRC_DATA      *ModMrcData,
  UINT8                     Channel
)
{
  UINT32 TempValue;
  UINT8 Rank;
  UINT8 Strobe;

  PRINT_FUNCTION_INFO;

  // 2.0 HIP Entry
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_wrlvlcoarse_list[0]);
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_wrlvlfine_list[0]);

  //
  // HIP Exit
  //
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_wrlvlfine_list[1]);

  if (ModMrcData->EccEnabled){
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_wrlvlcoarse_list[0]);
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_wrlvlfine_list[0]);

	  //
	  // HIP Exit
	  //
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_wrlvlfine_list[1]);
  }

  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
	  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
#if Rank2Rank_SHARING_DISABLED
    //
    // Restore WDQ and WDQS value only to enabled rank if RANK2RANK Sharing is disabled
    //
    //
    // Check if the Rank is enabled.
    //
    if (ModMrcData->Channel[Channel].RankEnabled[Rank] == 1) {
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQS_MIN, CMD_GET_CACHE, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQS_MIN, CMD_SET_VAL_FC_UC, &TempValue);

    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQ_MIN, CMD_GET_CACHE, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQ_MIN, CMD_SET_VAL_FC, &TempValue);

    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQS_DEL, CMD_GET_CACHE, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQS_DEL, CMD_SET_VAL_FC, &TempValue);

    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQ_DEL, CMD_GET_CACHE, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQ_DEL, CMD_SET_VAL_FC, &TempValue);
    }
#else
    //
    // Restore WDQ and WDQS value to all ranks although RANK2RANK Sharing is enabled
    //
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQS_MIN, CMD_GET_CACHE, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQS_MIN, CMD_SET_VAL_FC, &TempValue);

    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQ_MIN, CMD_GET_CACHE, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQ_MIN, CMD_SET_VAL_FC, &TempValue);

    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQS_DEL, CMD_GET_CACHE, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQS_DEL, CMD_SET_VAL_FC, &TempValue);

    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQ_DEL, CMD_GET_CACHE, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQ_DEL, CMD_SET_VAL_FC, &TempValue);
#endif
	  }
  }

  return SUCCESS;
}

/*++

Routine Description:

  Performs Coarse Write Leveling.  This is reusing many of the same routines used with the Receive Enable
  calibration.

Arguments:

  ModMrcData:     Contains input params, output params, and global storage.
  Channel:        Channel to operate on.

Returns:

    Success
    Failure

--*/
STATUS
CoarseWriteLeveling (
  MMRC_DATA   *ModMrcData,
  UINT8        Channel
)
{
  UINT8   Rank;
  UINT8   Strobe;
  UINT32  TempValue;
  UINT32  byteMask;
  UINT32              compareFlag;
  UINT32              repeatCacheline;
  UINT32              changeValue;
  STATUS  Status;
#if !CPGC_API
  UINT32              TargetAddress;
  UINT32              numberCacheLines;
  UINT8               CacheLinesArray[1024 + 64];
  UINT8              *CacheLines = (UINT8 *) ( ( ( (UINT32) (&CacheLinesArray[0]) ) & 0xfffffff0) + 0x10);
  UINT8               Patterns[] = {0x00, 0x3c, 0xc3, 0xf0, 0x0f};
  UINT32              loop;
#endif

  Status = SUCCESS;
  repeatCacheline = 1;
  changeValue = 2 * ONE_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);

  PRINT_FUNCTION_INFO;

  // 1.0 SIP Entry
  CoarseWriteLevelingEntryHooks (ModMrcData);

  // 2.0 HIP Entry
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_wrlvlcoarse_list[0]);

  if (ModMrcData->EccEnabled){
	  // 2.0 HIP Entry
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_wrlvlcoarse_list[0]);
  }


  // 3.0 Training
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
	  //
	  // Only perform WRLevel training on the ranks that are enabled.
	  //
	  if (ModMrcData->Channel[Channel].RankEnabled[Rank] ) {

#if CPGC_API
		  CPGC_Setup(ModMrcData, Channel, Rank);
		  CPGC_S_SetupSeq (ModMrcData, Channel, CPGC_SUBSEQINDEX_0, CPGC_SUBSEQINDEX_1, 0x1, 0x0);
#endif
		  //
		  // Perform a precharge.
		  //

		  JedecCmd (ModMrcData, Channel, Rank, (UINT8) JEDEC_PRECHARGEALL, (UINT32) 0xffffffff);

		  //
		  // Perform a FIFO Reset.
		  //
		  PerformFifoReset(ModMrcData, Channel,Rank);

		  //
		  // Increment the WDQS by 2 CLKS tethering WDQ.
		  //
		  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
			  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQS_DEL, CMD_GET_CACHE, &TempValue);
			  TempValue += 2 * ONE_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
			  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQS_DEL, CMD_SET_VAL_FC_UC, &TempValue);
			  TempValue -= QTR_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
			  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQ_DEL, CMD_SET_VAL_FC_UC, &TempValue);
		  }

		  //
		  // Determine the bytemask based on the cpu configuration.
		  //
		  BytelaneMask (ModMrcData, &byteMask);

#if !CPGC_API
		  BuildCacheArrays (ModMrcData, CacheLines, Patterns, sizeof (Patterns), &numberCacheLines);

		  TargetAddress = GetAddress (ModMrcData, Channel, Rank);
		  Enable16KMTRR (ModMrcData, TargetAddress, 0x20E);
#endif

		  do {
			  compareFlag = 0;
#if CPGC_API
			  CPGC_S_ClearErrors (ModMrcData, Channel);
			  CPGC_S_StartTest (ModMrcData, Channel, Rank);
			  while (CPGC_S_PollTest (ModMrcData, Channel) == CPGC_STS_TEST_BUSY);
			  CPGC_S_StopTest (ModMrcData, Channel);
			  CPGC_S_CheckErrors (ModMrcData, Channel, (UINT32*) NULL, (UINT32*) NULL, &compareFlag, (UINT8*) NULL);
#else
			  for (loop = 0; loop < numberCacheLines; loop++ ) {
				  LoadXmm03WithPattern (ModMrcData, &CacheLines[loop * 64]);
				  BurstOutCachelinesViaXmm (ModMrcData, TargetAddress, repeatCacheline);
				  ReadRamIntoCache (ModMrcData, TargetAddress, repeatCacheline);
				  CompareCacheWithXmm47 (ModMrcData, TargetAddress, repeatCacheline, &TempValue);
				  compareFlag |= TempValue;
			  }
#endif
			  
			  if (changeValue == (UINT32)(2 * ONE_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)) ) {
				  changeValue = 1 * ONE_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
				  
			  } else {
				  
			  }

			  if (compareFlag != 0) {
				  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
					  if ( (compareFlag & (byteMask << Strobe) ) != 0) {
						  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQS_DEL, CMD_GET_CACHE, &TempValue);
						  TempValue -= ONE_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);

						  //check negative value
						  if ( TempValue > (UINT32)(5 * ONE_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)) ){
							  Status = FAILURE;
							  break;
						  }

						  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQS_DEL, CMD_SET_VAL_FC_UC, &TempValue);
						  TempValue -= QTR_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
						  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQ_DEL, CMD_SET_VAL_FC_UC, &TempValue);
					  }
					  // Reset the FIFOs

					  TempValue = FIFO_RESET_ENABLE;
					  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, FIFORESET, CMD_SET_VAL_FC, &TempValue);
					  TempValue = FIFO_RESET_DISABLE;
					  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, FIFORESET, CMD_SET_VAL_FC, &TempValue);
				  }
			  }
			  if (Status == FAILURE) {
				  
				  return FAILURE;
			  }
		  } while (compareFlag != 0);
	  }
  }	//For( Rank)

#if CPGC_API
  CPGC_S_Disable (ModMrcData, Channel);
#endif

  // Resolve the minus1 select.
  SetMinus1Select (ModMrcData, Channel, WDQS_MIN);
  SetMinus1Select (ModMrcData, Channel, WDQ_MIN);
#if Rank2Rank_SHARING_DISABLED
#else

  // If Rank 2 Rank is enabled, then we need to average out all the Rcvn across the ranks and set all ranks to the same Value.
  AverageDelay (ModMrcData, Channel, WDQS_MIN);
  AverageDelay (ModMrcData, Channel, WDQ_MIN);

#endif

  // 4.0 HIP Entry
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_wrlvlcoarse_list[1]);

  if (ModMrcData->EccEnabled){
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_wrlvlcoarse_list[1]);
  }

  // 5.0 SIP Entry
  CoarseWriteLevelingExitHooks (ModMrcData);

  return Status;
}




//*********************************************************************************************************************
// Funtion Name: BytelaneMask()                                                                                  *
//*********************************************************************************************************************
// Date: 04/25/2012                                                                                                   *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         SUCCESS/FAIL result                                                                             *
//*********************************************************************************************************************
// Description:                                                                                                       *
//                             *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
static STATUS
BytelaneMask (
  MMRC_DATA *ModMrcData,
  UINT32 *Mask
)
{
#if CPGC_API
  *Mask = 0x1;
#else
  *Mask = 0x00;
  do {
    *Mask <<= BUSWIDTH;
    *Mask |= 0x00000001;
  } while (*Mask < 0x00010000);
  *Mask &= 0xffff;
#endif
  return SUCCESS;
}

#if !CPGC_API
//*********************************************************************************************************************
// Funtion Name: FillCacheArray()                                                                                *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
static STATUS
FillCacheArray (
  MMRC_DATA *ModMrcData,
  UINT8 *Ptr,
  UINT8 Data,
  UINT8 *Offset
)
{
  UINT8 DataPosition;
  //  UINT8 offset=0;
  UINT8 Pattern;
  UINT8 BusWidth;

  *Offset = 0;
  for (DataPosition = 0; DataPosition < 8; DataPosition++) {
    if ( ( (Data >> DataPosition) & 1 ) == 0 ) {
      Pattern = ZEROS;
    } else {
      Pattern = ONES;
    }
    for (BusWidth = 0; BusWidth < BUSWIDTH; BusWidth++) {
      Ptr[ (*Offset) ++] = Pattern;
    }
  }
  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: BuildCacheArrays()                                                                              *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
static STATUS
BuildCacheArrays (
  MMRC_DATA *ModMrcData,
  UINT8 *CL,
  UINT8 *Patterns,
  UINT32 NumberPatterns,
  UINT32 *NumCL
)
{
  UINT32 ClIndex = 0;
  UINT32 PatternIndex;
  UINT32 ClSize = 0;
  UINT8  TempValue;

  for (PatternIndex = 0; PatternIndex < NumberPatterns; PatternIndex++ ) {
    FillCacheArray (ModMrcData, CL + (ClIndex * 64) + ClSize, Patterns[PatternIndex], &TempValue);
    ClSize += TempValue;
    if (ClSize >= 64) {
      ClIndex += 1;
      ClSize = 0;
    }
  }

  *NumCL = ClIndex + 1;
  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: Enable16KMTRR()                                                                                 *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
static STATUS
Enable16KMTRR (
  MMRC_DATA *ModMrcData,
  UINT32               TargetAddress,
  UINT32               Msr
)
{
  volatile UINT32 MtrrAddr;

  MtrrAddr = TargetAddress & 0xFFFFC000; // 16K align
  //mtrr_addr |= 0x06; // set Type to WB
  MtrrAddr |= 0x01; // set Type to WC
  // enable a cacheable region at ADDRESS
#ifndef _MSC_EXTENSIONS
  asm ( // set Variable MTRR at ADDRESS to WB
    "mov %0,%%eax;"
    "mov $0x00000000,%%edx;"
    "mov $0x0000020E,%%ecx;"
    "wrmsr;"
    :/* no outputs */
    :"c" (MtrrAddr)
    :"%eax", "%edx"
  );
  asm ( // set size to 16K and enable
    "mov $0xFFFFC800,%%eax;"
    "mov $0x00000000,%%edx;"
    "mov $0x0000020F,%%ecx;"
    "wrmsr;"
    :/* no outputs */
    :/* no inputs */
    :"%eax", "%ecx", "%edx"
  );
#else
  _asm mov eax, MtrrAddr;
  _asm mov edx, 0x00000000;
  _asm mov ecx, Msr;
  _asm wrmsr; // set Variable MTRR at ADDRESS to WB
  _asm mov eax, 0xFFFFC800;
  _asm mov edx, 0x00000000;
  _asm mov ecx, Msr;
  _asm add ecx, 1;
  _asm wrmsr; // set size to 16K and enable
#endif // VS_ENV

  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: LoadXmm03WithPattern()                                                                          *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************

static STATUS LoadXmm03WithPattern (
  MMRC_DATA *ModMrcData,
  UINT8               *CacheLines
)
{
#ifndef _MSC_EXTENSIONS
	UINT32 CacheLinesAddress = (UINT32)CacheLines;
  asm ("movl %0,%%eax;"
       "movdqa 0x00(%%eax),%%xmm0;"
       "movdqa 0x10(%%eax),%%xmm1;"
       "movdqa 0x20(%%eax),%%xmm2;"
       "movdqa 0x30(%%eax),%%xmm3;"
       :/* no outputs */
       :"r" (CacheLinesAddress)
       :"%eax"
      );
#else
  _asm mov eax, CacheLines;
  _asm movdqa xmm0, [eax+0x00];
  _asm movdqa xmm1, [eax+0x10];
  _asm movdqa xmm2, [eax+0x20];
  _asm movdqa xmm3, [eax+0x30];
#endif // _MSC_EXTENSIONS

  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: BurstOutCachelinesViaXmm()                                                                      *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
static STATUS
BurstOutCachelinesViaXmm (
  MMRC_DATA *ModMrcData,
  UINT32               Address,
  UINT32               CacheLineRepeatCount
)
{
#ifndef _MSC_EXTENSIONS
  asm ( // write to RAM from CACHE
    "mov %1,%%eax;" // CacheLineRepeatCount
    "mov %0,%%ebx;" // Address
    "mrc_wr_loop:"
    "movntdq %%xmm0,0x00(%%ebx);"
    "movntdq %%xmm1,0x10(%%ebx);"
    "movntdq %%xmm2,0x20(%%ebx);"
    "movntdq %%xmm3,0x30(%%ebx);"
    "add $0x40,%%ebx;" // next cacheline
    "dec %%eax;"
    "jnz mrc_wr_loop;"
    "sfence;"
    :
    :"m" (Address), "m" (CacheLineRepeatCount)
    :"%eax", "%ebx"
  );
#else
  __asm {
    // write to RAM from CACHE
    mov eax, CacheLineRepeatCount;
    mov ebx, Address;
    mrc_wr_loop:
    movntdq [ebx+0x00], xmm0;
    movntdq [ebx+0x10], xmm1;
    movntdq [ebx+0x20], xmm2;
    movntdq [ebx+0x30], xmm3;
    add ebx, 0x40;
    dec eax;
    jnz mrc_wr_loop;
    sfence;
  };
#endif // _MSC_EXTENSIONS
  return SUCCESS;
};

                    //*********************************************************************************************************************
                    // Funtion Name: ReadRamIntoCache()                                                                              *
                    //*********************************************************************************************************************
                    // Date:  04/25/2012                                                                                                  *
                    //*********************************************************************************************************************
                    // Input:                                                                                                             *
                    //    ModMrcData: Contains input params, output params, and global storage.                                       *
                    //*********************************************************************************************************************
                    // Return:                                                                                                            *
                    //    Status:         Pass/Fail result from requesing the Sample.                                                     *
                    //*********************************************************************************************************************
                    // Description:                                                                                                       *
                    //*********************************************************************************************************************
                    // Modifications:                                                                                                     *
                    //    Date      : Name   : Description                                                                                *
                    //    04/25/2012:        : Initial Creation                                                                           *
                    //*********************************************************************************************************************
                    static STATUS
                    ReadRamIntoCache (
                      MMRC_DATA *ModMrcData,
                      UINT32               TargetAddress,
                      UINT32               CacheLineRepeatCount
                    )
{
#ifndef _MSC_EXTENSIONS
  asm ("mov %1,%%eax;" // burst_cnt
       "mov %0,%%ebx;" // Address
       "mrc_rd_loop:"
       "prefetcht0 0x00(%%ebx);" // read cacheline at (EBX)
       "add $0x40,%%ebx;" // next cacheline
       "dec %%eax;"
       "jnz mrc_rd_loop;"
       "lfence;"
       :
       :"m" (TargetAddress), "m" (CacheLineRepeatCount)
       :"%eax", "%ebx", "%ecx", "%edx"
      );
#else
  __asm {
    // write to RAM from CACHE
    mov eax, CacheLineRepeatCount;
    mov ebx, TargetAddress;
    ReadRAMIntoCacheLoop:
    prefetcht0 [ebx];
    add ebx, 0x40;
    dec eax;
    jnz ReadRAMIntoCacheLoop;
    lfence;
    mov eax, [ebx-0x40];
  };
#endif // _MSC_EXTENSIONS

  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: CompareCacheWithXmm47()                                                                         *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
static STATUS
CompareCacheWithXmm47 (
  MMRC_DATA *ModMrcData,
  UINT32               TargetAddress,
  UINT32               CacheLineRepeatCount,
  UINT32              *EncodedFailPtr
)
{
  volatile UINT32 EncodedFail = 0;
  volatile UINT32 FailingAddress = 0;

#ifndef _MSC_EXTENSIONS
  asm ( "movl %3,%%eax;" // burst_cnt
        "movl %2,%%ebx;" // Address
        "mrc_verify_loop:"
        "movdqa %%xmm0,%%xmm4;"     // Place the original cachline values into XMM4-7 for inplace comparison.
        "movdqa %%xmm1,%%xmm5;"
        "movdqa %%xmm2,%%xmm6;"
        "movdqa %%xmm3,%%xmm7;"
        "mov $0x0000FFFF,%%edx;"    // assume passing condition
        "pcmpeqb 0x00(%%ebx),%%xmm4;"   // XMM0 has the failing bytes for 1st OWORD in cacheline (0x00 is a failure)
        "pmovmskb %%xmm4,%%ecx;"        // ECX = packed WORD result for 1st OWORD in cacheline
        "and %%ecx,%%edx;"              // EDX = packed WORD result for cacheline (cumulative)
        "pcmpeqb 0x10(%%ebx),%%xmm5;"   // XMM1 has the failing bytes for 2nd OWORD in cacheline (0x00 is a failure)
        "pmovmskb %%xmm5,%%ecx;"        // ECX = packed WORD result for 2nd OWORD in cacheline
        "and %%ecx,%%edx;"              // EDX = packed WORD result for cacheline (cumulative)
        "pcmpeqb 0x20(%%ebx),%%xmm6;"   // XMM2 has the failing bytes for 3rd OWORD in cacheline (0x00 is a failure)
        "pmovmskb %%xmm6,%%ecx;"        // ECX = packed WORD result for 3rd OWORD in cacheline
        "and %%ecx,%%edx;"              // EDX = packed WORD result for cacheline (cumulative)
        "pcmpeqb 0x30(%%ebx),%%xmm7;"   // XMM3 has the failing bytes for 4th OWORD in cacheline (0x00 is a failure)
        "pmovmskb %%xmm7,%%ecx;"        // ECX = packed WORD result for 4th OWORD in cacheline
        "and %%ecx,%%edx;"              // EDX = packed WORD result for cacheline (cumulative)
        "cmp $0x0000FFFF,%%edx;"    // check for failure
        "jne mrc_return_fail;"      // failure detected
        "add $0x40,%%ebx;"
        "dec %%eax;"
        "jnz mrc_verify_loop;" // finish the dataset (BURST_CNT)
        "jmp mrc_return_pass;" // everything passed
        "mrc_return_fail:"
        "xor $0xFFFFFFFF,%%edx;" // invert to make 0xFF a failure
        "and $0x0000FFFF,%%edx;" // only 16 Bits are valid
        "jmp mrc_verify_end;"
        "mrc_return_pass:"
        "mov $0x00,%%edx;"
        "mrc_verify_end:"
        "mov %%edx,%0;" // encoded WORD in edx goes to encoded_fail
        "mov %%ebx,%1;" // Address of failure goes to failing_addr
        :"=m"(EncodedFail), "=m"(FailingAddress)
        :"m"(TargetAddress), "m"(CacheLineRepeatCount)
        :"%eax", "%ebx", "%ecx", "%edx"
      );
#else
  __asm {
    mov eax, CacheLineRepeatCount;
    mov ebx, TargetAddress;
    mrc_verify_loop:
    movdqa xmm4, xmm0;
    movdqa xmm5, xmm1;
    movdqa xmm6, xmm2;
    movdqa xmm7, xmm3;
    mov edx, 0x0000FFFF;
    pcmpeqb xmm4, [ebx+0x00];
    pmovmskb ecx, xmm4;
    and edx, ecx;
    pcmpeqb xmm5, [ebx+0x10];
    pmovmskb ecx, xmm5;
    and edx, ecx;
    pcmpeqb xmm6, [ebx+0x20];
    pmovmskb ecx, xmm6;
    and edx, ecx;
    pcmpeqb xmm7, [ebx+0x30];
    pmovmskb ecx, xmm7;
    and edx, ecx;
    cmp edx, 0x0000FFFF;
    jne mrc_return_fail;
    add ebx, 0x40;
    dec eax;
    jnz mrc_verify_loop;
    jmp mrc_return_pass;
    mrc_return_fail:
    xor edx, 0xFFFFFFFF;
    and edx, 0x0000FFFF;
    jmp mrc_verify_end;
    mrc_return_pass:
    mov edx, 0x00;
    mrc_verify_end:
    mov EncodedFail, edx;
    mov FailingAddress, ebx;
  };
#endif // _MSC_EXTENSIONS

  *EncodedFailPtr = EncodedFail;
  return SUCCESS;
}
#endif

STATUS AverageRDQS (MMRC_DATA *ModMrcData, UINT8 Channel)
{
  UINT32 TempValue;
  UINT32 totalValue[MAX_STROBES];
  UINT8  Rank, Strobe, Rank_present;

  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
	  totalValue[Strobe] = 0;
  }

  //FOREACH_CHANNEL_BEGIN (Channel);

  Rank_present = 0;
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
	  if (ModMrcData->Channel[Channel].RankEnabled[Rank] == 1) {
		  Rank_present++;

		  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
			  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RDQS_DEL, CMD_GET_CACHE, &TempValue);
			  totalValue[Strobe] += TempValue;
		  }
	  }
  }

  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
	  TempValue = totalValue[Strobe] / Rank_present;
	  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
		  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RDQS_DEL, CMD_SET_VAL_FC_UC, &TempValue);
	  }
  }

  //FOREACH_CHANNEL_END;

  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: ReadTraining()                                                                                       *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
STATUS
ReadTraining (
  MMRC_DATA         *ModMrcData,
  UINT8              Channel
)
{

  PRINT_FUNCTION_INFO;

  // 1.0 SIP Entry
  ReadTrainingEntryHooks (ModMrcData);

  // 2.0 HIP Entry
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_rdvref_list[0]);

  if (ModMrcData->EccEnabled){
	  // 2.0 HIP Entry
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_rdvref_list[0]);
  }
  //3.0 Training

  ReadWriteTrain (ModMrcData,
                  Channel,
                  READ_DELAY,
                  HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)/2,
                  TrainStep[ModMrcData->FeatureSettings.MrcDigitalDll],  //RDTRAIN_MARGINSTEP,
                  RDVREFTRAIN_LOWBOUNDARY,
                  RDVREFTRAIN_UPBOUNDARY
                 );

#if Rank2Rank_SHARING_DISABLED
#else
  //average rdqs and store in rank0
  AverageRDQS (ModMrcData, Channel);

#endif

  // 5.0 HIP Entry
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_rdvref_list[1]);

  if (ModMrcData->EccEnabled){
	  // 2.0 HIP Entry
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_rdvref_list[1]);
  }

  // 6.0 SIP Entry
  ReadTrainingExitHooks (ModMrcData);

  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: WriteTraining()                                                                                       *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
STATUS
WriteTraining (
  MMRC_DATA *ModMrcData,
  UINT8                Channel
)
{

  PRINT_FUNCTION_INFO;

  // 1.0 SIP Entry
  WriteTrainingEntryHooks (ModMrcData);

  // 2.0 HIP Entry
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_wr_list[0]);

  if (ModMrcData->EccEnabled){
	  // 2.0 HIP Entry
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_wr_list[0]);
  }

 if (ModMrcData->FeatureSettings.MrcDigitalDll == 0) {
  //3.0 Training
  ReadWriteTrain (ModMrcData,
                  Channel,
                  WRITE_DELAY,
                  WRTRAIN_MIDPOINTVREF,
                  TrainStep[ModMrcData->FeatureSettings.MrcDigitalDll], //WRTRAIN_MARGINSTEP,
                  WRTRAIN_LOWBOUNDARY,
                  WRTRAIN_UPBOUNDARY
                 );
  } else {
    ReadWriteTrain (ModMrcData,
                  Channel,
                  WRITE_DELAY,
                  0xF,
                  TrainStep[ModMrcData->FeatureSettings.MrcDigitalDll],//WRTRAIN_MARGINSTEP,
                  WRTRAIN_LOWBOUNDARY,
                  0x1F
                 );
  }

  // Resolve the minus1 select.
  SetMinus1Select (ModMrcData, Channel, WDQ_MIN);

#if Rank2Rank_SHARING_DISABLED
#else
  // If Rank 2 Rank is enabled, then we need to average out all the Rcvn across the ranks and set all ranks to the same Value.
  AverageDelay (ModMrcData, Channel, WDQ_MIN);

#endif

  // 5.0 HIP Entry
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_wr_list[1]);

  if (ModMrcData->EccEnabled){
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_wr_list[1]);
  }
  // 6.0 SIP Exit
  WriteTrainingExitHooks (ModMrcData);

  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: ReadVrefTraining()                                                                                   *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
STATUS
ReadVrefTraining (
  MMRC_DATA *ModMrcData,
  UINT8                Channel
)
{
  PRINT_FUNCTION_INFO;

  // 1.0 SIP Entry
  ReadVrefEntryHooks (ModMrcData);

  // 2.0 HIP Entry
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_rd_list[0]);

  if (ModMrcData->EccEnabled){
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_rd_list[0]);
  }
  //3.0 Training
  ReadWriteTrain (ModMrcData,
                  Channel,
                  READ_VREF,
                  RDVREFTRAIN_MIDPOINTVREF,
                  TrainStep[ModMrcData->FeatureSettings.MrcDigitalDll],//RDVREFTRAIN_MARGINSTEP,
                  RDTRAIN_LOWBOUNDARY,
                  (HALF_CLK_(2, ModMrcData->CurrentFrequency)) -1
                 );

  // 4.0 HIP Entry
  DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_rd_list[1]);

  if (ModMrcData->EccEnabled){
	  DecodeAndExeRegAssignment (ModMrcData, 1, HIP_rd_list[1]);
  }
  // 5.0 SIP Entry
  ReadVrefExitHooks (ModMrcData);

  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: FillGoldenBuffer()                                                                              *
//*********************************************************************************************************************
// Date: 04/25/2012                                                                                                   *
//*********************************************************************************************************************
// Input:                                                                                                             *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Shifted Value: The resultant of the shift.                                                                      *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
#if !CPGC_API
static STATUS
FillGoldenBuffer (
  MMRC_DATA *ModMrcData,
  UINT32              *PatternIndex,
  UINT32               BufferAddress
)
{
  UINT16        BufferIndex              = 0;
  UINT32        VictimPatterns[]         = VICTIMPATTERNS;
  UINT32        AggressorPatterns[]     = AGGRESSORPATTERNS;
  UINT8         FlybyShift[MAX_STROBES] = FLYBY_SHIFT;

  while ( (BufferIndex < 1024) & (*PatternIndex < NUMBERPATTERNS) ) { ///**** change
    GeneratePattern (ModMrcData, VictimPatterns[*PatternIndex / 17], AggressorPatterns[*PatternIndex / 17], FlybyShift, (*PatternIndex) % 17, 16, ( (*PatternIndex) % 17 == 17), (void *) (BufferAddress + BufferIndex) );
    *PatternIndex    += 1;
    BufferIndex += PATTERN_SIZE;
  }

  // To get to this point, either the buffer is full or PatternIndex has reached the max.
  while (BufferIndex < 1024) {
    GeneratePattern (ModMrcData, VictimPatterns[0], AggressorPatterns[0], FlybyShift, 0, 16, 0, (void *) (BufferAddress + BufferIndex) );
    BufferIndex += PATTERN_SIZE;
  }

  return SUCCESS;
}
#endif

STATIC
STATUS
CpgcSInit (
  IN  OUT   MMRC_DATA     *ModMrcData,
  IN        UINT8         Channel
)
/*++

Routine Description:

  This API will setup the most basic configuration to use the CPGC engine


Arguments:

  ModMrcData:       Host structure for all data related to MMRC
  Channel:          Current Channel being examined.
  Rank:             Current Rank being examined
  SelBurstErrCheckEn:Enable/Disables the Burst Errors Check

Returns:

  Success
  Failure

--*/
{
  CPGC_S_Enable (ModMrcData, Channel);

  CPGC_S_StopTest (ModMrcData, Channel);
  if (ModMrcData->EccEnabled) {
      CPGC_S_EnableErrors (ModMrcData, Channel,  0xff, 0xff, 0xffffffff, 0xffffffff, 0xff);
  } else {
      CPGC_S_EnableErrors (ModMrcData, Channel,  0xff, 0xff, 0xffffffff, 0xffffffff, 0x0 );
  }

  CPGC_S_ClearErrors (ModMrcData, Channel);

  return SUCCESS;
}


STATIC
STATUS
CpgcSetupforSweep(
  IN  OUT   MMRC_DATA     *ModMrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN		UINT8 CPGCNumBursts,
  IN		UINT8 CPGCExpLoopCnt
){
  UINT32 Address;

  CpgcSInit(ModMrcData,Channel);
  // Get the correct Address range for the given channel and rank.
  Address = GetAddress (ModMrcData, Channel, Rank) >> 6;

  CPGC_S_SetupSubseq (ModMrcData, Channel,  CPGC_SUBSEQINDEX_0, CPGC_SUBSEQ_TYPE_WOREF, CPGCNumBursts, Address, 0x1, 0, 0, 10, 0, 3, 0);
  CPGC_S_SetupSubseq (ModMrcData, Channel,  CPGC_SUBSEQINDEX_1, CPGC_SUBSEQ_TYPE_ROREF, CPGCNumBursts, Address, 0x1, 0, 0, 10, 0, 3, 0);
  CPGC_S_SetupPatternControl (ModMrcData, Channel);
  CPGC_S_SetupSeq (ModMrcData, Channel, CPGC_SUBSEQINDEX_0, CPGC_SUBSEQINDEX_1, CPGCExpLoopCnt, 0x0);

  return SUCCESS;
}

STATUS
CpgcSetupForCMD(
  IN  OUT   MMRC_DATA     *ModMrcData,
  IN        UINT8         Channel,
  IN        UINT8         Rank,
  IN		UINT8 CPGCNumBursts,
  IN		UINT8 CPGCExpLoopCnt
)
{
  UINT32 Address;
  UINT8  CpgcDat;

  CpgcSInit(ModMrcData,Channel);

  for (CpgcDat = CPGC_DPAT0; CpgcDat <= CPGC_DPAT15; CpgcDat++) {
    if (CpgcDat == CPGC_DPAT0 || CpgcDat == CPGC_DPAT8) {
      CPGC_S_SetupPattern (ModMrcData, Channel, CpgcDat, 0xCC, 0x00, 0x00, 0x00);
    } else {
      CPGC_S_SetupPattern (ModMrcData, Channel, CpgcDat, 0xAA, 0x00, 0x00, 0x00);
    }
  }
  Address = GetAddress (ModMrcData, Channel, Rank) >> 6;
  CPGC_S_SetupSubseq (ModMrcData, Channel,  CPGC_SUBSEQINDEX_0, CPGC_SUBSEQ_TYPE_WOREF, CPGCNumBursts, Address, 0x0, 1, 0, 0xA, 0x15AD, 3, 0);
  CPGC_S_SetupSubseq (ModMrcData, Channel,  CPGC_SUBSEQINDEX_1, CPGC_SUBSEQ_TYPE_ROREF, CPGCNumBursts, Address, 0x0, 1, 0, 0xA, 0x15AD, 3, 0);
  CPGC_S_SetupSubseq (ModMrcData, Channel,  CPGC_SUBSEQINDEX_2, CPGC_SUBSEQ_TYPE_WOREF, CPGCNumBursts, Address, 0x0, 1, 0, 0xA, 0x444E, 3 + OFFSET_HIGH_LOW_SHIFT, OFFSET_HIGH_LOW_SHIFT);
  CPGC_S_SetupSubseq (ModMrcData, Channel,  CPGC_SUBSEQINDEX_3, CPGC_SUBSEQ_TYPE_ROREF, CPGCNumBursts, Address, 0x0, 1, 0, 0xA, 0x444E, 3 + OFFSET_HIGH_LOW_SHIFT, OFFSET_HIGH_LOW_SHIFT);
  //
  // Setup the 2 subsequences.
  //
  CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_0, CPGC_UNISEQINDEX_LFSR, CPGC_LFSR_VICTIM_SEED, 0);
  CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_1, CPGC_UNISEQINDEX_LFSR, CPGC_LFSR_AGRESSOR_SEED, 0);
  CPGC_S_SetupSeq (ModMrcData, Channel, CPGC_SUBSEQINDEX_0, CPGC_SUBSEQINDEX_3, CPGCExpLoopCnt, 0x0);
  Cpgc_S_SetupCADB(ModMrcData, Channel, 1);

  return SUCCESS;
}
//*********************************************************************************************************************
// Funtion Name: MarginSweep()                                                                                   *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
static STATUS
MarginSweep (
  MMRC_DATA *ModMrcData,
  UINT8 sweepType,
  UINT8       Channel,
  UINT8 Rank,
  UINT32 goldenAddr,
  UINT16 *PfLimits,
  UINT8  *pStrobeEnable,
  UINT8 CPGCNumBursts,
  UINT8 CPGCExpLoopCnt
)
{
  UINT8   Strobe, allPassFlag;
  UINT16  byteLaneCompareMask, bytelaneCompareResult, byteMask;
  UINT8   lowHighFlag, RankColapse;
  UINT32  PatternIndex;
  UINT32  TempValue;
  UINT8   CmdLoop;                  // Loop through the commands
#if CPGC_API
  UINT32  VictimPatterns[]        = VICTIMPATTERNS;
  UINT32  NumberFails;
  UINT32  NumberOfPatterns;
#endif

  // Compure the mask needed to compare each bytelane
  BytelaneMask (ModMrcData, &TempValue);
  byteLaneCompareMask = (UINT16) TempValue;

#if CPGC_API

  //
  // Setup CPGC either for normal sweep or CMD sweep
  //
  if (sweepType == CMD_DELAY) {
    CpgcSetupForCMD(ModMrcData,Channel,Rank, CPGCNumBursts,CPGCExpLoopCnt);
  } else {
    CpgcSetupforSweep(ModMrcData, Channel, Rank, CPGCNumBursts,CPGCExpLoopCnt);
  }

  //
  // Determine the number of pattenrs required for the Margin to complete successfully.
  //
  switch (ModMrcData->PatternMode) {
  case PATTERN_VICAGG:
    NumberOfPatterns = NUMBERPATTERNS*(ODD_MODE_BITSHIFTS + EVEN_MODE);
    break;
  case LFSR_VICAGG:
    NumberOfPatterns = (ODD_MODE_BITSHIFTS + EVEN_MODE_BITSHIFTS);
    break;
  case LFSR:
    NumberOfPatterns = 1;
    break;
  default:
    NumberOfPatterns = NUMBERPATTERNS;
  };
#endif
  // Given the location of the golden Data, attempt to copy the Data to ram and read them back.
  // The results for every Channel/Rank are provided in the compareResutls array as a 16-bit
  // Value where each bit corresponds to a bytelane.
  do {
	  NumberFails = 0;
	  PatternIndex = 0;

	  while (PatternIndex < RDWR_NUMBERPATTERNS) {
#if !CPGC_API
		  FillGoldenBuffer (ModMrcData, &PatternIndex, goldenAddr);
#else
		  CPGC_LoadCpgcPattern(ModMrcData, Channel, VictimPatterns[PatternIndex/17], PatternIndex%17);
		  PatternIndex++;
#endif
		  for (lowHighFlag = LOW; lowHighFlag <= HIGH; lowHighFlag++) {
			  do {
				  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
					  if (pStrobeEnable[Strobe] == NOT_MET) {
						  if (sweepType == READ_DELAY) {
							  for (RankColapse = 0; RankColapse < MAX_RANKS; RankColapse++) {
								  if (ModMrcData->Channel[Channel].RankEnabled[RankColapse]) {
									  TempValue = PfLimits[ (Strobe * 2) + lowHighFlag];
									  GetSetDataSignal (ModMrcData, 0, Channel, 0, RankColapse, Strobe, 0, 0, DELAYS_RDQS_DEL, CMD_SET_VAL_FC_UC, &TempValue);
								  }
							  }
						  } else if (sweepType == WRITE_DELAY) {
							  TempValue = PfLimits[ (Strobe * 2) + lowHighFlag];
							  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQ_DEL, CMD_SET_VAL_FC_UC, &TempValue);
						  } else if (sweepType == READ_VREF){
							  TempValue = PfLimits[ (Strobe * 2) + lowHighFlag];
							  TempValue = LinearToPhysicalVrefCodes[TempValue];
							  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RDQS_VREF, CMD_SET_VAL_FC, &TempValue);
						  } else if (sweepType == CMD_DELAY){
							  TempValue = PfLimits[ (Strobe * 2) + lowHighFlag];
							  for (CmdLoop=0; CmdLoop < MAX_CMDS; CmdLoop++ ) {
								  GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL + CmdLoop, CMD_SET_VAL_FC, &TempValue);
							  }
							  //Exit for loop for CMD_DELAY as CMD_DELAY doesn't have strobe
							  break;
						  }else{ //(sweepType == WRITE_VREF)
							  //To do: Write platform VREF
						  }
						  TempValue = FIFO_RESET_ENABLE;
						  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, FIFORESET, CMD_SET_VAL_FC, &TempValue);
						  TempValue = FIFO_RESET_DISABLE;
						  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, FIFORESET, CMD_SET_VAL_FC, &TempValue);
					  }
				  }

				  if (sweepType == READ_DELAY) {
#if CPGC_API
                  if (ModMrcData->EccEnabled) {
                      bytelaneCompareResult = 0x1ff;
                  } else {
                      bytelaneCompareResult = 0xff;
                  }
					  for (RankColapse = 0; RankColapse < MAX_RANKS; RankColapse++) {
						  if (ModMrcData->Channel[Channel].RankEnabled[RankColapse]) {
							  CPGC_S_ClearErrors (ModMrcData, Channel);
							  CPGC_S_StartTest (ModMrcData, Channel, Rank);
							  while (CPGC_S_PollTest (ModMrcData, Channel) == CPGC_STS_TEST_BUSY);
							  CPGC_S_StopTest (ModMrcData, Channel);
							  CPGC_S_CheckErrors (ModMrcData, Channel, (UINT32*) NULL, (UINT32*) NULL, &TempValue, (UINT8*) NULL);

							  TempValue = ~TempValue;
							  bytelaneCompareResult &= (UINT16) TempValue;
						  }

#else
						  bytelaneCompareResult = 0xffff;
						  for (RankColapse = 0; RankColapse < MAX_RANKS; RankColapse++) {
							  if (ModMrcData->Channel[Channel].RankEnabled[RankColapse]) {
								  CompareGoldenWithDRAMPatterns (ModMrcData, goldenAddr, Channel, RankColapse, (UINT16 *) &TempValue);
								  bytelaneCompareResult &= TempValue;
							  }
#endif
						  }
					  }
					  else {
#if CPGC_API
						  CPGC_S_ClearErrors (ModMrcData, Channel);
						  CPGC_S_StartTest (ModMrcData, Channel, Rank);
						  while (CPGC_S_PollTest (ModMrcData, Channel) == CPGC_STS_TEST_BUSY);
						  CPGC_S_StopTest (ModMrcData, Channel);
						  CPGC_S_CheckErrors (ModMrcData, Channel, (UINT32*) NULL, (UINT32*) NULL, &TempValue, (UINT8*) NULL);
						  bytelaneCompareResult = (UINT16) ~TempValue;
#else
						  CompareGoldenWithDRAMPatterns (ModMrcData, goldenAddr, Channel, Rank, &bytelaneCompareResult);
#endif
					  }

					  //
					  // Perform a FIFO Reset.
					  //
					  PerformFifoReset(ModMrcData, Channel,Rank);

					  if (sweepType == CMD_DELAY){
						  MrcDelay (NANO_DEL, 15);                    //delay 10 us
				          LPDDR3_JEDECInit (ModMrcData, Channel);
					  }

					  allPassFlag = 1;
					  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
						  if (pStrobeEnable[Strobe] == NOT_MET) {
							  byteMask = byteLaneCompareMask << Strobe;
							  if ( (bytelaneCompareResult & byteMask) != byteMask) {
								  if (lowHighFlag == LOW) {
									  if (PfLimits[ (Strobe * 2) + LOW] < PfLimits[ (Strobe * 2) + HIGH]) {
										  PfLimits[ (Strobe * 2) + LOW] += 1;
										  allPassFlag = 0;
									  }
								  } else {
									  if (PfLimits[ (Strobe * 2) + HIGH] > PfLimits[ (Strobe * 2) + LOW]) {
										  PfLimits[ (Strobe * 2) + HIGH] -= 1;
										  allPassFlag = 0;
									  }
								  }
							  }

							  if (sweepType == CMD_DELAY){
							  	  //Exit for loop for CMD_DELAY as CMD_DELAY doesn't have strobe	
								  break;
							  }
						  }
					  }
				  }
				  while (allPassFlag == 0);
			  }
		  }
	  }
	  while (NumberFails > 0) ;
  //return globalChange;

#if CPGC_API
	  CPGC_S_Disable (ModMrcData, Channel);
#endif

	  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: GeneratePattern()                                                                               *
//*********************************************************************************************************************
// Date: 04/25/2012                                                                                                   *
//*********************************************************************************************************************
// Input:                                                                                                             *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Shifted Value: The resultant of the shift.                                                                      *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
#if !CPGC_API
static STATUS
GeneratePattern (
  MMRC_DATA *ModMrcData,
  UINT32               VictimPattern,
  UINT32               AggressorPattern,
  UINT8                ByteShift[],
  UINT8                BitShift,
  UINT8                VictimRepeat,
  UINT8                EvenOddFlag,
  void                 *BufferPv
)
{

  UINT8  Strobe;
  UINT8  BitPosition;
  UINT8  TimeSample;
  UINT32 VictimPatternStrobe[MAX_STROBES];
  UINT32 AggressorPatternStrobe[MAX_STROBES];
  UINT64 VictimMask;
  UINT64 AggressorMask;
  UINT64 VictimBits;
  UINT64 AggressorBits;
  UINT64 Bits;

  // Step 1: Using the byteShift, VictimPattern, and AggressorPattern input parameters, create the victim/aggressor
  // patterns that will be used on each BL.  Thereby creating victimPatternBL[] and aggessorPatternBL[].
  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
    VictimPatternStrobe[Strobe] = VictimPattern << ByteShift[Strobe];
    AggressorPatternStrobe[Strobe] = AggressorPattern << ByteShift[Strobe];
  }

  //Step 2:  Create the VictimMask and AggressorMask which will provide the bit fields which should have the victim
  // and aggressor Bits accordingly, this should use the victimShift.
  VictimMask = 0;
  Bits = 1;
  for (BitPosition = 0; BitPosition < MAX_STROBES * 8; BitPosition += VictimRepeat) {
    VictimMask |= (UINT64) (Shl64 (Bits, BitPosition) );
  }
  AggressorMask = ~VictimMask;

  // Step 3: Shift the aggressor and victim masks by the BitShift input Parameters.
  AggressorMask = (Shl64 (AggressorMask, BitShift) ) | ( Shr64 (AggressorMask, (MAX_STROBES * 8 - BitShift) ) );
  VictimMask = (Shl64 (VictimMask, BitShift) ) | (Shr64 (VictimMask, (MAX_STROBES * 8 - BitShift) ) );

  for (TimeSample = 0; TimeSample < 32; TimeSample++) {
    VictimBits = 0;
    AggressorBits = 0;
    for (Strobe = 0; Strobe < MAX_STROBES; Strobe ++) {
      if ( ( (VictimPatternStrobe[Strobe] >> TimeSample) & 1 ) == 1) {
        VictimBits |= (UINT64) (0xff << (8 * Strobe) );
      }
      if (EvenOddFlag == ODD_MODE) { // Odd
        if ( ( (AggressorPatternStrobe[Strobe] >> TimeSample) & 1 ) == 1) {
          AggressorBits |= 0xff << (8 * Strobe);
        }
      }
    }

    if (EvenOddFlag == ODD_MODE) {
      // Odd Mode
      Bits = (VictimBits & VictimMask) | (AggressorBits & AggressorMask);
    } else {
      // Even Mode
      Bits = VictimBits;
    }

    switch (MAX_STROBES) {
    case 1:
      * ( (UINT8 *) BufferPv) = (UINT8) Bits;
      BufferPv = ((UINT8 *)BufferPv) + 1;
      break;
    case 2:
      * ( (UINT16 *) BufferPv) = (UINT16) Bits;
      BufferPv = ((UINT16 *)BufferPv) + 1;
      break;
    case 4:
      * ( (UINT32 *) BufferPv) = (UINT32) Bits;
      BufferPv = ((UINT32 *)BufferPv) + 1;
      break;
    default:
      * ( (UINT64 *) BufferPv) = (UINT64) Bits;
      BufferPv = ((UINT64 *)BufferPv) + 1;
    }
  } // End of TimeSample;

  return SUCCESS;
}


//*********************************************************************************************************************
// Funtion Name: Shl64()                                                                                         *
//*********************************************************************************************************************
// Date: 04/25/2012                                                                                                   *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    Value: 64-bit Value that will be shifted                                                                        *
//    Shift: The number of Bits to shift the Value by.                                                                *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Shifted Value: The resultant of the shift.                                                                      *
//*********************************************************************************************************************
// Description:                                                                                                       *
// This function does a shift-left of the 64-bit Value.  This is because there are no primitives within the compiler  *
// to do a 64-bit shift.  The function is written in assembly to optimize the shift.                                  *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************

static UINT64
Shl64 (
  UINT64 Value,          // 64-bit Value to be shifted.
  UINT8  Shift            // Amount to shift by.
)
{
#ifndef _MSC_EXTENSIONS
  return (Value << Shift);
#else
  _asm {
    mov edx, dword Ptr Value[4] // Load Eax,Edx with the 64-bit Value.
    mov eax, dword Ptr Value
    mov cl, Shift                   // Load CL with the shift amount.

    cmp cl, 32                      // If the shift is more then 32, then copy edx into eax.
    jb less_32                          // and zero out eax, then shift edx by x-32, otherwise shift by x.

    mov   edx, eax                    // This is the case shift>32, then copy edx into eax.
    xor   eax, eax                    // This is the case shift>32, then zero out eax.
    less_32:
    shld  edx, eax, cl                // Shift eax/edx into edx by cl which is modulo 32.
    shl   eax, cl                     // Shift eax itself.
  }
#endif
}

//*********************************************************************************************************************
// Funtion Name: Shr64()                                                                                         *
//*********************************************************************************************************************
// Date: 04/25/2012                                                                                                   *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    Value: 64-bit Value that will be shifted                                                                        *
//    Shift: The number of Bits to shift the Value by.                                                                *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Shifted Value: The resultant of the shift.                                                                      *
//*********************************************************************************************************************
// Description:                                                                                                       *
// This function does a shift-left of the 64-bit Value.  This is because there are no primitives within the compiler  *
// to do a 64-bit shift.  The function is written in assembly to optimize the shift.                                  *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************

static UINT64
Shr64 (
  UINT64 Value,            // 64-bit Value to shift.
  UINT8  Shift                  // The amount to shift by.
)
{
#ifndef _MSC_EXTENSIONS
  return (Value >> Shift);
#else
  _asm {
    mov edx, dword Ptr Value[4]      // Load Eax,Edx with the 64-bit Value.
    mov eax, dword Ptr Value
    mov cl, Shift                     // Load CL with the shift amount.

    cmp cl, 32                        // If the shift is more then 32, then copy edx into eax.
    jb less_32                            // and zero out eax, then shift edx by x-32, otherwise shift by x.

    mov   eax, edx                      // This is the case shift>32, then copy edx into eax.
    xor   edx, edx                      // This is the case shift>32, then zero out eax.
    less_32:
    shrd  eax, edx, cl                  // Shift eax/edx into edx by cl which is modulo 32.
    shr   edx, cl                       // Shift eax itself.
  }
#endif
}

//*********************************************************************************************************************
// Funtion Name: CompareGoldenWithDRAMPatterns()                                                                 *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************

static STATUS CompareGoldenWithDRAMPatterns (
  MMRC_DATA *ModMrcData,
  UINT32               GoldenPatternAddress,
  UINT8                Channel,
  UINT8                Rank,
  UINT16              *CompareResults
)
{
  UINT32    TargetAddress;
  UINT8    *Ptr;
  UINT8     Strobe;
  UINT32    TempValue;

  UINT64  AllOnes[4] = {0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff, 0xffffffffffffffff};

  Ptr = (UINT8 *) AllOnes;
  Ptr = (UINT8 *) ( ( ( (UINT32) Ptr) + 15 ) & 0xfffffff0);

  // Get the correct Address range for the given channel and rank.
  TargetAddress = GetAddress (ModMrcData, Channel, Rank);

  // Enable a 16K buffer at the target Address.
  Enable16KMTRR (ModMrcData, TargetAddress, 0x20E);

  //
  // Perform a FIFO Reset.
  //
  PerformFifoReset(ModMrcData, Channel,Rank);

  // Transfer the memory buffer from the golden area to the test area.
  TransferMemory (ModMrcData, (UINT8 *) TargetAddress, (UINT8 *) GoldenPatternAddress, 16, 1, (UINT32 *) Ptr);

  //
  // Perform a FIFO Reset.
  //
  PerformFifoReset(ModMrcData, Channel,Rank);

  // Fetch the target area back into cache.
  ReadRamIntoCache (ModMrcData, TargetAddress, 32);

  // Compare the readback Data with the golden Data for the given channel and rank.
  CompareMemory (ModMrcData, (UINT8 *) GoldenPatternAddress, (UINT8 *) TargetAddress, 16, (UINT32 *) Ptr, 1, CompareResults);

  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: TransferMemory()                                                                                *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
static STATUS
TransferMemory (
  MMRC_DATA *ModMrcData,
  UINT8               *DstAddr,
  UINT8               *SrcAddr,
  UINT32               CacheLines,
  UINT8                InvertFlag,
  UINT32              *AllOnes
)
{
#ifndef _MSC_EXTENSIONS
  asm (
    "mov %0,%%eax;"
    "mov %1,%%ebx;"
    "mov %2,%%ecx;"
    "mov %3, %%dl;"
    "mov %4,%%esi;"
    "TransferMemoryLoop:"
    "movdqa 0x00(%%eax),%%xmm0;"
    "movdqa 0x10(%%eax),%%xmm1;"
    "movdqa 0x20(%%eax),%%xmm2;"
    "movdqa 0x30(%%eax),%%xmm3;"
    "movdqa 0x40(%%eax),%%xmm4;"
    "movdqa 0x50(%%eax),%%xmm5;"
    "movdqa 0x60(%%eax),%%xmm6;"
    "movdqa 0x70(%%eax),%%xmm7;"
    "movntdq %%xmm0, 0x00(%%ebx);"
    "movntdq %%xmm1, 0x10(%%ebx);"
    "movntdq %%xmm2, 0x20(%%ebx);"
    "movntdq %%xmm3, 0x30(%%ebx);"
    "movntdq %%xmm4, 0x40(%%ebx);"
    "movntdq %%xmm5, 0x50(%%ebx);"
    "movntdq %%xmm6, 0x60(%%ebx);"
    "movntdq %%xmm7, 0x70(%%ebx);"
    "add $0x80,%%ebx;"
    "add $0x80,%%eax;"
    "cmp $0x0, %%dl;"
    "je SKIP_INVERT_1;"
    "pxor 0x00(%%esi),%%xmm0;"
    "pxor 0x00(%%esi),%%xmm1;"
    "pxor 0x00(%%esi),%%xmm2;"
    "pxor 0x00(%%esi),%%xmm3;"
    "pxor 0x00(%%esi),%%xmm4;"
    "pxor 0x00(%%esi),%%xmm5;"
    "pxor 0x00(%%esi),%%xmm6;"
    "pxor 0x00(%%esi),%%xmm7;"
    "movntdq %%xmm0,0x00(%%ebx);"
    "movntdq %%xmm1,0x10(%%ebx);"
    "movntdq %%xmm2,0x20(%%ebx);"
    "movntdq %%xmm3,0x30(%%ebx);"
    "movntdq %%xmm4,0x40(%%ebx);"
    "movntdq %%xmm5,0x50(%%ebx);"
    "movntdq %%xmm6,0x60(%%ebx);"
    "movntdq %%xmm7,0x70(%%ebx);"
    "add $0x80,%%ebx;"
    "SKIP_INVERT_1:"
    "sub $0x2,%%ecx;"
    "cmp $0x0,%%ecx;"
    "jnz TransferMemoryLoop;"
    :/* no outputs */
    :"m" (SrcAddr), "m" (DstAddr), "m" (CacheLines), "m" (InvertFlag), "m" (AllOnes)
    :"%eax", "%ebx", "%esi", "%edx", "%ecx"
  );
#else
  __asm {
    mov eax, SrcAddr;
    mov ebx, DstAddr;
    mov ecx, CacheLines;
    mov dl, InvertFlag;
    mov esi, AllOnes;
    TransferMemoryLoop:
    movdqa xmm0, [eax+0x00];
    movdqa xmm1, [eax+0x10];
    movdqa xmm2, [eax+0x20];
    movdqa xmm3, [eax+0x30];
    movdqa xmm4, [eax+0x40];
    movdqa xmm5, [eax+0x50];
    movdqa xmm6, [eax+0x60];
    movdqa xmm7, [eax+0x70];
    movntdq [ebx+0x00], xmm0;
    movntdq [ebx+0x10], xmm1;
    movntdq [ebx+0x20], xmm2;
    movntdq [ebx+0x30], xmm3;
    movntdq [ebx+0x40], xmm4;
    movntdq [ebx+0x50], xmm5;
    movntdq [ebx+0x60], xmm6;
    movntdq [ebx+0x70], xmm7;
    add ebx, 0x80;
    add eax, 0x80;
    cmp dl, 0;
    je SKIP_INVERT;
    pxor xmm0, [esi];
    pxor xmm1, [esi];
    pxor xmm2, [esi];
    pxor xmm3, [esi];
    pxor xmm4, [esi];
    pxor xmm5, [esi];
    pxor xmm6, [esi];
    pxor xmm7, [esi];
    movntdq [ebx+0x00], xmm0;
    movntdq [ebx+0x10], xmm1;
    movntdq [ebx+0x20], xmm2;
    movntdq [ebx+0x30], xmm3;
    movntdq [ebx+0x40], xmm4;
    movntdq [ebx+0x50], xmm5;
    movntdq [ebx+0x60], xmm6;
    movntdq [ebx+0x70], xmm7;
    add ebx, 0x80;
    SKIP_INVERT:
    sub ecx, 2;
    cmp ecx, 0;
    jnz TransferMemoryLoop;
  }
#endif // VS_ENV

  return SUCCESS;
}

//*********************************************************************************************************************
// Funtion Name: CompareMemory()                                                                                 *
//*********************************************************************************************************************
// Date:  04/25/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input params, output params, and global storage.                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result from requesing the Sample.                                                     *
//*********************************************************************************************************************
// Description:                                                                                                       *
//*********************************************************************************************************************
// Modifications:                                                                                                     *
//    Date      : Name   : Description                                                                                *
//    04/25/2012:        : Initial Creation                                                                           *
//*********************************************************************************************************************
static STATUS
CompareMemory (
  MMRC_DATA *ModMrcData,
  UINT8               *GoldenAddrress,
  UINT8               *TestAddr,
  UINT32               CacheLines,
  UINT32              *AllOnes,
  UINT8                InvertFlag,
  UINT16              *Results
)
{
  UINT16  CompareResults;
#ifndef _MSC_EXTENSIONS
  asm (
    "mov %1,%%eax;"
    "mov %2,%%ebx;"
    "mov %3,%%esi;"
    "mov %4,%%edi;"
    "mov $0x0000FFFF,%%edx;"
    "CompareMemLoop:"
    "movdqa 0x00(%%eax),%%xmm0;"        // Copy golden cacheline into xmm0:7
    "movdqa 0x10(%%eax),%%xmm1;"
    "movdqa 0x20(%%eax),%%xmm2;"
    "movdqa 0x30(%%eax),%%xmm3;"
    "movdqa 0x40(%%eax),%%xmm4;"
    "movdqa 0x50(%%eax),%%xmm5;"
    "movdqa 0x60(%%eax),%%xmm6;"
    "movdqa 0x70(%%eax),%%xmm7;"
    "pcmpeqb 0x00(%%ebx),%%xmm0;"
    "pmovmskb %%xmm0,%%ecx;"
    "and %%ecx,%%edx;"            // Compare each octword to the target Address, accumulating results into EDX.
    "pcmpeqb 0x10(%%ebx),%%xmm1;"
    "pmovmskb %%xmm1,%%ecx;"
    "and %%ecx,%%edx;"
    "pcmpeqb 0x20(%%ebx),%%xmm2;"
    "pmovmskb %%xmm2,%%ecx;"
    "and %%ecx,%%edx;"
    "pcmpeqb 0x30(%%ebx),%%xmm3;"
    "pmovmskb %%xmm3,%%ecx;"
    "and %%ecx,%%edx;"
    "pcmpeqb 0x40(%%ebx),%%xmm4;"
    "pmovmskb %%xmm4,%%ecx;"
    "and %%ecx,%%edx;"            // Compare each octword to the target Address, accumulating results into EDX.
    "pcmpeqb 0x50(%%ebx),%%xmm5;"
    "pmovmskb %%xmm5,%%ecx;"
    "and %%ecx,%%edx;"            // Compare each octword to the target Address, accumulating results into EDX.
    "pcmpeqb 0x60(%%ebx),%%xmm6;"
    "pmovmskb %%xmm6,%%ecx;"
    "and %%ecx,%%edx;"            // Compare each octword to the target Address, accumulating results into EDX.
    "pcmpeqb 0x70(%%ebx),%%xmm7;"
    "pmovmskb %%xmm7,%%ecx;"
    "and %%ecx,%%edx;"            // Compare each octword to the target Address, accumulating results into EDX.
    "mov %5,%%cl;"
    "cmp $1,%%cl;"
    "jne SKIP_INVERT;"
    "add $0x80,%%ebx;"
    "movdqa 0x00(%%eax),%%xmm0;"        // Copy golden cacheline into xmm0:7
    "movdqa 0x10(%%eax),%%xmm1;"
    "movdqa 0x20(%%eax),%%xmm2;"
    "movdqa 0x30(%%eax),%%xmm3;"
    "movdqa 0x40(%%eax),%%xmm4;"
    "movdqa 0x50(%%eax),%%xmm5;"
    "movdqa 0x60(%%eax),%%xmm6;"
    "movdqa 0x70(%%eax),%%xmm7;"
    "pxor   (%%edi),%%xmm0;"
    "pxor   (%%edi),%%xmm1;"
    "pxor   (%%edi),%%xmm2;"
    "pxor   (%%edi),%%xmm3;"
    "pxor   (%%edi),%%xmm4;"
    "pxor   (%%edi),%%xmm5;"
    "pxor   (%%edi),%%xmm6;"
    "pxor   (%%edi),%%xmm7;"
    "pcmpeqb 0x00(%%ebx),%%xmm0;"
    "pmovmskb %%xmm0,%%ecx;"
    "and %%ecx,%%edx;"            // Compare each octword to the target Address, accumulating results into EDX.
    "pcmpeqb 0x10(%%ebx),%%xmm1;"
    "pmovmskb %%xmm1,%%ecx;"
    "and %%ecx,%%edx;"
    "pcmpeqb 0x20(%%ebx),%%xmm2;"
    "pmovmskb %%xmm2,%%ecx;"
    "and %%ecx,%%edx;"
    "pcmpeqb 0x30(%%ebx),%%xmm3;"
    "pmovmskb %%xmm3,%%ecx;"
    "and %%ecx,%%edx;"
    "pcmpeqb 0x40(%%ebx),%%xmm4;"
    "pmovmskb %%xmm4,%%ecx;"
    "and %%ecx,%%edx;"          // Compare each octword to the target Address, accumulating results into EDX.
    "pcmpeqb 0x50(%%ebx),%%xmm5;"
    "pmovmskb %%xmm5,%%ecx;"
    "and %%ecx,%%edx;"          // Compare each octword to the target Address, accumulating results into EDX.
    "pcmpeqb 0x60(%%ebx),%%xmm6;"
    "pmovmskb %%xmm6,%%ecx;"
    "and %%ecx,%%edx;"          // Compare each octword to the target Address, accumulating results into EDX.
    "pcmpeqb 0x70(%%ebx),%%xmm7;"
    "pmovmskb %%xmm7,%%ecx;"
    "and %%ecx,%%edx;"          // Compare each octword to the target Address, accumulating results into EDX.
    "SKIP_INVERT:"
    "cmp $0x00000000,%%edx;"      // If all bytelanes are failing, immediately exit the loop as no more test is required.
    "je CompareMemoryDone;"
    "add $0x80,%%ebx;"
    "add $0x80,%%eax;"
    "sub $0x02,%%esi;"
    "cmp $0x00,%%esi;"
    "jne CompareMemLoop;"
    "CompareMemoryDone:"
    "mov %%dx,%0;"
    :"=m" (CompareResults)
    :"m" (GoldenAddrress), "m" (TestAddr), "m" (CacheLines), "m" (AllOnes), "m" (InvertFlag)
    :"%eax", "%ebx", "%esi", "%edi", "%ecx", "%edx"
  );
#else
  __asm {
    mov eax, GoldenAddrress;
    mov ebx, TestAddr;
    mov esi, CacheLines;
    mov edi, AllOnes;
    mov edx, 0x0000FFFF;          // Initialize EDX=results criteria, default=0xffff
    CompareMemoryLoop:
    movdqa xmm0, [eax+0x00];        // Copy golden cacheline into xmm0:7
    movdqa xmm1, [eax+0x10];
    movdqa xmm2, [eax+0x20];
    movdqa xmm3, [eax+0x30];
    movdqa xmm4, [eax+0x40];
    movdqa xmm5, [eax+0x50];
    movdqa xmm6, [eax+0x60];
    movdqa xmm7, [eax+0x70];
    pcmpeqb xmm0, [ebx+0x00];
    pmovmskb ecx, xmm0;
    and edx, ecx;           // Compare each octword to the target Address, accumulating results into EDX.
    pcmpeqb xmm1, [ebx+0x10];
    pmovmskb ecx, xmm1;
    and edx, ecx;
    pcmpeqb xmm2, [ebx+0x20];
    pmovmskb ecx, xmm2;
    and edx, ecx;
    pcmpeqb xmm3, [ebx+0x30];
    pmovmskb ecx, xmm3;
    and edx, ecx;
    pcmpeqb xmm4, [ebx+0x40];
    pmovmskb ecx, xmm4;
    and edx, ecx;           // Compare each octword to the target Address, accumulating results into EDX.
    pcmpeqb xmm5, [ebx+0x50];
    pmovmskb ecx, xmm5;
    and edx, ecx;           // Compare each octword to the target Address, accumulating results into EDX.
    pcmpeqb xmm6, [ebx+0x60];
    pmovmskb ecx, xmm6;
    and edx, ecx;           // Compare each octword to the target Address, accumulating results into EDX.
    pcmpeqb xmm7, [ebx+0x70];
    pmovmskb ecx, xmm7;
    and edx, ecx;           // Compare each octword to the target Address, accumulating results into EDX.
    mov cl, InvertFlag;
    cmp cl, 1
    jne SKIP_INVERT
    add ebx, 0x80;
    movdqa xmm0, [eax+0x00];        // Copy golden cacheline into xmm0:7
    movdqa xmm1, [eax+0x10];
    movdqa xmm2, [eax+0x20];
    movdqa xmm3, [eax+0x30];
    movdqa xmm4, [eax+0x40];
    movdqa xmm5, [eax+0x50];
    movdqa xmm6, [eax+0x60];
    movdqa xmm7, [eax+0x70];
    pxor   xmm0, [edi];
    pxor   xmm1, [edi];
    pxor   xmm2, [edi];
    pxor   xmm3, [edi];
    pxor   xmm4, [edi];
    pxor   xmm5, [edi];
    pxor   xmm6, [edi];
    pxor   xmm7, [edi];
    pcmpeqb xmm0, [ebx+0x00];
    pmovmskb ecx, xmm0;
    and edx, ecx;           // Compare each octword to the target Address, accumulating results into EDX.
    pcmpeqb xmm1, [ebx+0x10];
    pmovmskb ecx, xmm1;
    and edx, ecx;
    pcmpeqb xmm2, [ebx+0x20];
    pmovmskb ecx, xmm2;
    and edx, ecx;
    pcmpeqb xmm3, [ebx+0x30];
    pmovmskb ecx, xmm3;
    and edx, ecx;
    pcmpeqb xmm4, [ebx+0x40];
    pmovmskb ecx, xmm4;
    and edx, ecx;           // Compare each octword to the target Address, accumulating results into EDX.
    pcmpeqb xmm5, [ebx+0x50];
    pmovmskb ecx, xmm5;
    and edx, ecx;           // Compare each octword to the target Address, accumulating results into EDX.
    pcmpeqb xmm6, [ebx+0x60];
    pmovmskb ecx, xmm6;
    and edx, ecx;           // Compare each octword to the target Address, accumulating results into EDX.
    pcmpeqb xmm7, [ebx+0x70];
    pmovmskb ecx, xmm7;
    and edx, ecx;           // Compare each octword to the target Address, accumulating results into EDX.

    SKIP_INVERT:
    cmp edx, 0x00000000;          // If all bytelanes are failing, immediately exit the loop as no more test is required.
    je CompareMemoryDone;
    add ebx, 0x80;
    add eax, 0x80;
    sub esi, 0x02;
    cmp esi, 0x00;
    jne CompareMemoryLoop;
    CompareMemoryDone:
    mov CompareResults, dx;
  };
#endif // _MSC_EXTENSIONS
  *Results = CompareResults;

  return SUCCESS;
}

static STATUS
DisableMTRR (
  MMRC_DATA *ModMrcData,
  UINT32               Msr
)
{
#ifndef _MSC_EXTENSIONS
  asm ("mov $0xFFFFC000,%%eax;"
       "mov $0x00000000,%%edx;"
       "mov $0x0000020E,%%ecx;"
       "wrmsr;"
       :/* no outputs */
       :/* no inputs */
       :"%eax", "%ecx", "%edx"
      );
#else
  _asm mov eax, 0xFFFFC000;
  _asm mov edx, 0x00000000;
  _asm mov ecx, Msr;
  _asm wrmsr;
#endif // VS_ENV
  return SUCCESS;
}
#endif

STATUS ReadTrainRestore (MMRC_DATA *ModMrcData,
                         UINT8                Channel)
{
  UINT32 TempValue;
  UINT8  Rank;
  UINT8  Strobe;

  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
	  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
#if Rank2Rank_SHARING_DISABLED
  // Check if the Channel and Rank are enabled.
  if (ModMrcData->Channel[Channel].RankEnabled[Rank] == 1) {
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RDQS_DEL, CMD_GET_CACHE, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RDQS_DEL, CMD_SET_VAL_FC_UC, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RDQS_VREF, CMD_GET_CACHE, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RDQS_VREF, CMD_SET_VAL_FC_UC, &TempValue);
  }
#else
  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RDQS_DEL, CMD_GET_CACHE, &TempValue);
  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RDQS_DEL, CMD_SET_VAL_FC, &TempValue);

  if (Rank == 0) {
  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RDQS_VREF, CMD_GET_CACHE, &TempValue);
  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RDQS_VREF, CMD_SET_VAL_FC, &TempValue);
  }
#endif
	  }
  }

  return SUCCESS;
}

static STATUS
ReadWriteTrain (
  MMRC_DATA *ModMrcData,
  UINT8                Channel,
  UINT8                MarginType,
  UINT8                MarginMidpoint,
  INT8                 MarginStep,
  UINT16               LowerBoundary,
  UINT16               UpperBoundary
)
{
  UINT8     Rank;
  UINT8     Strobe;
  UINT8     RankColapse;
  UINT8  	Loop;
  UINT16    MaxMargin[MAX_STROBES];
  UINT16    MinMarginThreshold[MAX_STROBES];
  UINT32    CenterCumWeightedEW[MAX_STROBES];
  UINT32    CumWeightedEW[MAX_STROBES];
  UINT32    Results[MAX_STROBES];
  UINT8     MarginOffsetBoundaries[MAX_STROBES][2];
  UINT32    MarginOffsetAtMaxMarginWindow[MAX_STROBES];
  UINT16    PfLimitHistory[64][MAX_STROBES][2];
  UINT16    EwPercentage;
  UINT16    WeightedCondition;
  INT8      MarginOffset;
  UINT16     FlagDnDone, FlagUpDone;
  UINT16    PfLimits[MAX_STROBES][2];
  UINT8     AllBLPassMinMarginThreasholdFlag;
  UINT8     ThresholdMet[2][MAX_STROBES];
  UINT16    MarginWindow;
  UINT8     MarginSweepType;
  UINT32    TempValue;
  UINT8		WriteDelayLowBoundary[MAX_STROBES];
  UINT8		WriteDelayUpBoundary[MAX_STROBES];
  UINT8		WriteDelayBoundaryLength = 0;
  UINT8     GoldenBuffer[1024 + 16];
  UINT8    *GoldenBufferPtr = (UINT8 *) ( ( ( (UINT32) (&GoldenBuffer[0]) ) & 0xfffffff0) + 0x10);
  UINT32	EyeDataArrayOffset;
  VREF_PI_DATA *EyeData;
  UINT8     InitValue = 0;
  UINT16    TotalMargin = 0;
  
  MarginSweepType = 0;
  EwPercentage              = 0;
  WeightedCondition         = 0;
  EyeDataArrayOffset = 0;

  MmrcMemSet (&PfLimitHistory[0][0], 0xFF,  sizeof (UINT16) * MAX_VREF_PI * MAX_STROBES * 2 );
  //Initialize variable below to 0
  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
	  MaxMargin[Strobe] = 0;
	  MinMarginThreshold[Strobe] = 0;
      CenterCumWeightedEW[Strobe] = 0;
	  CumWeightedEW[Strobe] = 0;
      MarginOffsetAtMaxMarginWindow[Strobe]  = 0;
      MarginOffsetBoundaries[Strobe][0]      = 0;
      MarginOffsetBoundaries[Strobe][1]      = 0;
      ThresholdMet[UP][Strobe]    = NOT_MET;
      ThresholdMet[DN][Strobe]    = NOT_MET;
	  WriteDelayLowBoundary[Strobe] = 0;
	  WriteDelayUpBoundary[Strobe] = 0;
  }

  // Loop through each Channel and Rank and for those ranks that are enabled, perform the training.
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {

	  EyeData = (VREF_PI_DATA *)&ModMrcData->Channel[Channel].EyeData[0];

	  if (ModMrcData->Channel[Channel].RankEnabled[Rank]) {
		  JedecCmd (ModMrcData, Channel, Rank, (UINT8) JEDEC_PRECHARGEALL, (UINT32) 0xffffffff);


		  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
			  if (MarginType == WRITE_DELAY) {
				  TempValue = 1;
				  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQ_MIN, CMD_SET_VAL_FC_UC, &TempValue);
			  }
              MaxMargin[Strobe]           = 0;
              MinMarginThreshold[Strobe]  = 0;
              CenterCumWeightedEW[Strobe] = 0;
              CumWeightedEW[Strobe]       = 0;
              MarginOffsetAtMaxMarginWindow[Strobe]  = 0;
              MarginOffsetBoundaries[Strobe][0]      = 0;
              MarginOffsetBoundaries[Strobe][1]      = 0;
			  ThresholdMet[UP][Strobe] = NOT_MET;
		  	  ThresholdMet[DN][Strobe]	= NOT_MET;
		  }

		  MarginOffset = 0;
		  FlagDnDone = RDWR_NOT_DONE;
          FlagUpDone = RDWR_NOT_DONE;

		  do {
			  // Step 2.  Set marginning reference, to perform the sweep.
			  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
				  if (MarginType == READ_DELAY) {
					  TempValue = MarginMidpoint + MarginOffset;
					  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RDQS_DEL, CMD_SET_VAL_FC_UC, &TempValue);
					  MarginSweepType = READ_VREF;
					  EyeDataArrayOffset = TempValue;
				  } else if (MarginType == WRITE_DELAY) {
					  MarginSweepType = WRITE_DELAY;
				  } else { // if (MarginType == READ_VREF)  {
					  TempValue = MarginMidpoint + MarginOffset;
					  EyeDataArrayOffset = TempValue;
					  TempValue = LinearToPhysicalVrefCodes[TempValue];
					  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RDQS_VREF, CMD_SET_VAL_FC_UC, &TempValue);
					  MarginSweepType = READ_DELAY;
				  }
			  }

			  // Step 2. Program the initial values into the pfLimit structure.
			  if (MarginType == WRITE_DELAY) {
				  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
					  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQS_DEL, CMD_GET_CACHE, &TempValue);

					  if (TempValue < (UINT32)(HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)) *3 / 2) {
						  PfLimits[Strobe][LOW]  = 0;
						  PfLimits[Strobe][HIGH] = (UINT16) (TempValue + HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency));
					  } else {
						  PfLimits[Strobe][LOW]  = (UINT16) (TempValue - (HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency) * 3 / 2));
						  PfLimits[Strobe][HIGH] = (UINT16) (TempValue + HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency));
					  }
					  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQS_MIN, CMD_GET_CACHE, &TempValue);
					  if (TempValue == 0) {
						  if (PfLimits[Strobe][LOW] < HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)) {
							  PfLimits[Strobe][LOW]   = 0;
							  PfLimits[Strobe][HIGH] -= HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
						  } else {
							  PfLimits[Strobe][LOW] -= HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
							  PfLimits[Strobe][HIGH] -= HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
						  }
					  }

					  // Store WRITE Delay Low and High boundary for 1D eye diagram
					  WriteDelayLowBoundary[Strobe]= (UINT8) PfLimits[Strobe][LOW];
					  WriteDelayUpBoundary[Strobe] = (UINT8) PfLimits[Strobe][HIGH];
				  }
			  } else {
				  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
					  PfLimits[Strobe][LOW]  = LowerBoundary;
					  PfLimits[Strobe][HIGH] = UpperBoundary;
				  }
			  }

			  // Run the training algo to get the margin Data.
			  if (MarginOffset >= 0 ) {
				  TempValue = UP;
			  } else {
				  TempValue = DN;
			  }

			  MarginSweep (ModMrcData,
                   MarginSweepType,
                   Channel,
                   Rank,
                   (UINT32) GoldenBufferPtr,
                   & (PfLimits[0][0]),
                   &ThresholdMet[TempValue][0],
                   6,	//CPGCNumBursts
                   4);	//CPGCExpLoopCnt

			  // Continue shmooing until all BLs pass the percent max margin rule.
			  AllBLPassMinMarginThreasholdFlag = TRUE;    // Initialized the FlagDone to assume that all BLs are passing the percent margin rule.

			  if (MarginType != WRITE_DELAY) {
				  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
					  if ( ( (MarginOffset >= 0) & (ThresholdMet[UP][Strobe] == NOT_MET) ) ||
							  ( (MarginOffset < 0) & (ThresholdMet[DN][Strobe] == NOT_MET) ) ) {
						  if (PfLimits[Strobe][HIGH] < PfLimits[Strobe][LOW]) {
							  PfLimits[Strobe][HIGH] = 0;
							  PfLimits[Strobe][LOW] = 0;
						  }
						  MarginWindow =  PfLimits[Strobe][HIGH] -  PfLimits[Strobe][LOW];
						  if (MarginWindow > MaxMargin[Strobe]) {
							  MaxMargin[Strobe] = MarginWindow;
                              MarginOffsetAtMaxMarginWindow[Strobe] = MarginMidpoint + MarginOffset;
							  MinMarginThreshold[Strobe] = PERCENT_MAX_MARGIN (MaxMargin[Strobe]);
						  }
						  if (MarginWindow > MinMarginThreshold[Strobe]) {
							  AllBLPassMinMarginThreasholdFlag = FALSE;
						  } else {
							  if (MarginOffset >= 0) {
								  ThresholdMet[UP][Strobe] = MET;
							  } else {
								  ThresholdMet[DN][Strobe] = MET;
							  }
						  }
						  //CumEW[Strobe]       += MarginWindow;
						  //CumWeightedEW[Strobe]   += (UINT16) (MarginWindow * (MarginMidpoint + MarginOffset) );
					  }

                      PfLimitHistory[EyeDataArrayOffset][Strobe][LOW]  = PfLimits[Strobe][LOW];
                      PfLimitHistory[EyeDataArrayOffset][Strobe][HIGH] = PfLimits[Strobe][HIGH];
					  if (MarginType == READ_VREF){
						  //
						  // Keep Left and Right RDQS PI value on READ VREF Training for 2D Eye Diagram
						  //
						  EyeData[EyeDataArrayOffset].Left[Rank][Strobe]= (UINT8)PfLimits[Strobe][LOW];
						  EyeData[EyeDataArrayOffset].Right[Rank][Strobe]= (UINT8)PfLimits[Strobe][HIGH];
					  }
				  }
			  } else{
				  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
					  //
					  // Get the higher value of Write Delay boundary length for 1D Eye diagram
					  //
					  if( (WriteDelayUpBoundary[Strobe] - WriteDelayLowBoundary[Strobe]) > WriteDelayBoundaryLength){
						  WriteDelayBoundaryLength = WriteDelayUpBoundary[Strobe] - WriteDelayLowBoundary[Strobe];
					  }
					  //
					  // Align Left and Right delay value to 0
					  //
					  EyeData[EYEDATA_1D_VREFPI_OFFSET].Left[Rank][Strobe]= (UINT8)PfLimits[Strobe][LOW] - WriteDelayLowBoundary[Strobe];
					  EyeData[EYEDATA_1D_VREFPI_OFFSET].Right[Rank][Strobe]= WriteDelayUpBoundary[Strobe] - (UINT8)PfLimits[Strobe][HIGH];
				  }
			  }

			  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
				  if (MarginOffset >= 0) {
				      if ((ThresholdMet[UP][Strobe] == MET2) || (ThresholdMet[UP][Strobe] == MET) ) {
					      if (MarginType == READ_VREF){
							  // Set EyeData to 0xFF if Threshold is Met
							  EyeData[EyeDataArrayOffset].Left[Rank][Strobe]= 0xFF;
							  EyeData[EyeDataArrayOffset].Right[Rank][Strobe]= 0xFF;
						  }
                          PfLimitHistory[EyeDataArrayOffset][Strobe][LOW]  = 0xFFFF;
                          PfLimitHistory[EyeDataArrayOffset][Strobe][HIGH] = 0xFFFF;
					  }
				  
					  if (ThresholdMet[UP][Strobe] == MET2) {

					  } else if (ThresholdMet[UP][Strobe] == MET) {

						  ThresholdMet[UP][Strobe] = MET2;
					  } else {

                          MarginOffsetBoundaries[Strobe][UP] = MarginMidpoint + MarginOffset;
					  }
				  } else {
				      if ((ThresholdMet[DN][Strobe] == MET2) || (ThresholdMet[DN][Strobe] == MET) ){
					      if (MarginType == READ_VREF){
							  // Set EyeData to 0xFF if Threshold is Met
							  EyeData[EyeDataArrayOffset].Left[Rank][Strobe]= 0xFF;
							  EyeData[EyeDataArrayOffset].Right[Rank][Strobe]= 0xFF;
						  }
                          PfLimitHistory[EyeDataArrayOffset][Strobe][LOW]  = 0xFFFF;
                          PfLimitHistory[EyeDataArrayOffset][Strobe][HIGH] = 0xFFFF;
					  }
					  if (ThresholdMet[DN][Strobe] == MET2) {

					  } else if (ThresholdMet[DN][Strobe] == MET) {

						  ThresholdMet[DN][Strobe] = MET2;
					  } else {

                          MarginOffsetBoundaries[Strobe][DN] = MarginMidpoint + MarginOffset;
					  }
				  }
			  }
			  
			  if (MarginType == WRITE_DELAY) {
				  FlagDnDone = RDWR_DONE;
				  FlagUpDone = RDWR_DONE;
			  } else {
				  //Based on the MinMarginThreshold Flags, set the next offset or finish testing.
				  if (MarginOffset > 0) {
					  if ( (AllBLPassMinMarginThreasholdFlag == RDWR_DONE) || ( (MarginOffset + MarginStep ) > (MarginMidpoint-1)) ) {  //> 31
						  FlagUpDone = RDWR_DONE;
					  }
					  if (FlagDnDone == RDWR_DONE) {
						  MarginOffset += MarginStep;
					  } else {
						  MarginOffset = MarginOffset * -1;
					  }
				  } else {
					  if ( (AllBLPassMinMarginThreasholdFlag == RDWR_DONE) || ( (MarginOffset - MarginStep) < (0-MarginMidpoint)) ) {  // < -32
						  FlagDnDone = RDWR_DONE;
					  }
					  if (FlagUpDone == RDWR_DONE) {
						  MarginOffset -= MarginStep;
					  } else {
						  MarginOffset = MarginOffset * -1 + MarginStep;
					  }
				  }
			  } //end of MarginType
		  } while (FlagDnDone != RDWR_DONE || FlagUpDone != RDWR_DONE);

		  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
			  if (MarginType == WRITE_DELAY) {
				  Results[Strobe] = (PfLimits[Strobe][HIGH] +  PfLimits[Strobe][LOW]) / 2;
			  } else {
                  FlagDnDone = 0;
                  FlagUpDone = 0;
                  if (MarginType == READ_VREF) {
                        EwPercentage = EW_PERCENTAGE_VREF (MaxMargin[Strobe]);
                  } else if (MarginType == WRITE_DELAY || MarginType == READ_DELAY) {
                      EwPercentage = EW_PERCENTAGE_DELAY (MaxMargin[Strobe]);
                  } else {
                      MRC_DEADLOOP ();
                  }

				  if (MarginMidpoint % 2) {  //odd value
				      InitValue = 1;
				  } else {
				      InitValue = 0;
				  }
				  TotalMargin = 0;
                  for (TempValue = InitValue; TempValue < (UINT32)(2*MarginMidpoint); TempValue += MarginStep) {
                      MarginWindow = PfLimitHistory[TempValue][Strobe][HIGH] - PfLimitHistory[TempValue][Strobe][LOW];
                      if (MarginWindow > EwPercentage) {
                         CumWeightedEW[Strobe] += (MarginWindow * TempValue);
                         TotalMargin += MarginWindow;
                      } else {
                          if (MarginType == READ_VREF) {
                              CumWeightedEW[Strobe] += (WEIGHT_CONDITION_VREF (MarginWindow) * TempValue);
                              TotalMargin += WEIGHT_CONDITION_VREF (MarginWindow);
                          } else if (MarginType == WRITE_DELAY || MarginType == READ_DELAY) {
                              CumWeightedEW[Strobe] += (WEIGHT_CONDITION_DELAY (MarginWindow) *TempValue);
                              TotalMargin += WEIGHT_CONDITION_DELAY (MarginWindow);
                          } else {
                              MRC_DEADLOOP ();
                          }
                      }
                  }
                  if (TotalMargin == 0) {
                	  MRC_DEADLOOP ();
                  }

                 Results[Strobe] = ROUND_UP(CumWeightedEW[Strobe], TotalMargin);

              }

			  //Results[Strobe] = (CumWeightedEW[Strobe] + CumEW[Strobe] / 2) / CumEW[Strobe];

			  TempValue = Results[Strobe];
			  if (MarginType == READ_DELAY) {
				  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RDQS_DEL, CMD_SET_VAL_FC_UC, &TempValue);
				  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RDQS_VREF, CMD_GET_CACHE, &TempValue);
				  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RDQS_VREF, CMD_SET_VAL_FC_UC, &TempValue);
			  } else if (MarginType == WRITE_DELAY) {
				  // When margining wr Delay, there is no VREF
				  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQ_DEL, CMD_SET_VAL_FC_UC, &TempValue);
			  } else if (MarginType == READ_VREF)  {
				  // Store the VREF results as they will be changed again during RDDELAY, the final results will be stored at the
				  // completion of the RDDELAY.
				  //pVrefFinal[Strobe] = Results[Strobe];
				  TempValue = LinearToPhysicalVrefCodes[TempValue];
				  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RDQS_VREF, CMD_SET_VAL_FC_UC, &TempValue);
			  }
		  }

		  
		  
#if !CPGC_API
		  DisableMTRR (ModMrcData, 0x20E);
#endif

		  if (MarginType == READ_VREF) {
			  //populate trained vref Value to all ranks in cache since vref is shared for all ranks
			  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
				  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RDQS_VREF, CMD_GET_CACHE, &TempValue);
				  for (RankColapse = 0; RankColapse < MAX_RANKS; RankColapse++) {
					  if (Rank != RankColapse) {
						  GetSetDataSignal (ModMrcData, 0, Channel, 0, RankColapse, Strobe, 0, 0, RDQS_VREF, CMD_SET_VAL_FC_UC, &TempValue);

						  //populate RDQS Left Right PI value to all ranks in Eyedata since vref is shared for all ranks and READ_VREF Training will do once
						  if (ModMrcData->Channel[Channel].RankEnabled[RankColapse]) {
        	  					for(Loop=0; Loop < MAX_VREF_PI; Loop++){
        		  					EyeData[Loop].Left[RankColapse][Strobe]= EyeData[Loop].Left[Rank][Strobe];
        		  					EyeData[Loop].Right[RankColapse][Strobe]= EyeData[Loop].Right[Rank][Strobe];
							  }
						  }
					  }
				  }
			  }
			  Rank = MAX_RANKS;
		  } // end of vref
	  }
  }

  return SUCCESS;
}

STATUS
SearchRmt (
	IN  OUT   MMRC_DATA *ModMrcData,
	IN        UINT8		Channel
)
{
  UINT8 Rank;
  UINT8 DebugMsgLevelSave;

  //Skip if MRC Debug Message is disabled to expedite boot time
  if (ModMrcData->FeatureSettings.MrcRMTSupport) {
    //
    // Turn off debug messages for RMT training steps.
    //
    DebugMsgLevelSave = ModMrcData->MrcDebugMsgLevel;
    ModMrcData->MrcDebugMsgLevel = SDBG_NONE;

    //Force ODT to Dunit Auto
	if (ModMrcData->CurrentDdrType <= TYPE_DDR3L_ECC) {
		ForceODT (ModMrcData->MrcDebugMsgLevel, Channel, 0, FORCEODT_OFF);
	}

	//
    // sweep CMD values
    //
    if (ModMrcData->CurrentDdrType == TYPE_LPDDR3){
  	    ModMrcData->FeatureSettings.MrcCPGCNumBursts = 11;
  	    ModMrcData->FeatureSettings.MrcCPGCExpLoopCnt = 10;
    	RmtDqDqsVrefSearch (ModMrcData,Channel,CMD_DELAY);
    }

	ModMrcData->FeatureSettings.MrcCPGCNumBursts = 6;
	ModMrcData->FeatureSettings.MrcCPGCExpLoopCnt = 4;

    //
    // sweep PI delays
    //
    RmtDqDqsVrefSearch (ModMrcData,Channel,READ_DELAY);
    RmtDqDqsVrefSearch (ModMrcData,Channel,WRITE_DELAY);


    //
    // sweep Vref values
    //
    RmtDqDqsVrefSearch (ModMrcData,Channel,READ_VREF);
    RmtDqDqsVrefSearch (ModMrcData,Channel,WRITE_VREF);



    //
    // Restore debug messages.
    //
    ModMrcData->MrcDebugMsgLevel = DebugMsgLevelSave;

    rcPrintf (ModMrcData->MrcDebugMsgLevel, SDBG_MIN, "START_RMT: \n");
    rcPrintf (ModMrcData->MrcDebugMsgLevel, SDBG_MIN, "                   \t RxDqLeft RxDqRight RxVLow RxVHigh TxDqLeft TxDqRight CmdLeft CmdRight\n");
    rcPrintf (ModMrcData->MrcDebugMsgLevel, SDBG_MIN, "------------------------------------------------------------------------------------------------\n");
    for (Rank = 0; Rank < MAX_RANKS; Rank ++) {
      if (ModMrcData->Channel[Channel].RankEnabled[Rank] == 0) {
        continue;
      }

      if (ModMrcData->CurrentDdrType <= TYPE_DDR3L_ECC) {
    	  ModMrcData->Channel[Channel].RMT_Data[Rank][CmdLeft] = 0;
    	  ModMrcData->Channel[Channel].RMT_Data[Rank][CmdRight] = 0;
      }

      rcPrintf (
    		  ModMrcData->MrcDebugMsgLevel, SDBG_MIN, "Channel %d Rank %d \t %3d %8d %9d %6d %7d %8d %10d %7d\n",
        Channel,	  
        Rank,
        ModMrcData->Channel[Channel].RMT_Data[Rank][RxDqLeft],
        ModMrcData->Channel[Channel].RMT_Data[Rank][RxDqRight],
        ModMrcData->Channel[Channel].RMT_Data[Rank][RxVLow],
        ModMrcData->Channel[Channel].RMT_Data[Rank][RxVHigh],
        ModMrcData->Channel[Channel].RMT_Data[Rank][TxDqLeft],
        ModMrcData->Channel[Channel].RMT_Data[Rank][TxDqRight],
        ModMrcData->Channel[Channel].RMT_Data[Rank][CmdLeft],
        ModMrcData->Channel[Channel].RMT_Data[Rank][CmdRight]
      );
    }
    rcPrintf (ModMrcData->MrcDebugMsgLevel, SDBG_MIN, "STOP_RMT: \n");
    rcPrintf (ModMrcData->MrcDebugMsgLevel, SDBG_MIN, "CMD module is per channel only and without Rank differentiation\n");
  }

  return SUCCESS;
}

STATUS
RmtDqDqsVrefSearch (
  IN  OUT   MMRC_DATA *ModMrcData,
  IN        UINT8                 Channel,
  IN        UINT8                 Mode
)
{
  UINT8		Strobe, Rank;
  UINT8		TrainRank;
  UINT32	curValue[MAX_STROBES];
  UINT32	RdqsValue[MAX_RANKS][MAX_STROBES];
  INT16   	rmtMinLeftMargin;
  INT16   	rmtMinRigthMragin;
  INT16   	rmtBlMargin;
  INT16   	rmtBlCenter;
  INT16   	rmtDistance;
  UINT32 	TempValue;
  UINT8		RMTDataLeftLowOffset;
  UINT8		RMTDataRightHiOffset;
  UINT16    PfLimits[MAX_STROBES][2];
  UINT8     ThresholdMet[MAX_STROBES];
  UINT8     GoldenBuffer[1024 + 16];
  UINT8    *GoldenBufferPtr = (UINT8 *) ( ( ( (UINT32) (&GoldenBuffer[0]) ) & 0xfffffff0) + 0x10);
  UINT8  	CmdLoop;                  // Loop through the commands
  UINT32 	CmdValue[MAX_CMDS];       // Default values for the linear commands

  rmtMinLeftMargin = 0;
  rmtMinRigthMragin = 0;
  rmtBlMargin = 0;
  rmtBlCenter = 0;
  rmtDistance = 0;

  for (Strobe = 0; Strobe < MAX_STROBES; Strobe++) {
	  //Initialize it to 0
	  curValue[Strobe] = 0;
  }

  for (CmdLoop=0; CmdLoop < MAX_CMDS; CmdLoop++ ) {
  		CmdValue[CmdLoop] = 0;
  }

  //
  // Read Rdqs PI value on all Ranks and backup it to RdqsValue[] before calling MarginSweep
  // MarginSweep function on READ_DELAY will update current trained Rank value to all Ranks
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank ++) {
#if Rank2Rank_SHARING_DISABLED
	  TrainRank = Rank;
#else
	  TrainRank = 0;
#endif
	  if (ModMrcData->Channel[Channel].RankEnabled[Rank] == 0) {
		  continue;
	  }
	  if (Mode == READ_DELAY) {
		  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
			  GetSetDataSignal (ModMrcData, 0, Channel, 0, TrainRank, Strobe, 0, 0, DELAYS_RDQS_DEL, CMD_GET_CACHE, &RdqsValue[Rank][Strobe]);
		  }
	  }
  }

  for (Rank = 0; Rank < MAX_RANKS; Rank ++) {
	  if (ModMrcData->Channel[Channel].RankEnabled[Rank] == 0) {
		  continue;
	  }

#if Rank2Rank_SHARING_DISABLED
	  TrainRank = Rank;
#else
	  TrainRank = 0;
#endif
	  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
		  //
		  //Save all current value and will be use to restore back the value after calling MarginSweep ()
		  switch (Mode) {

		  case READ_DELAY:
			  //Do nothing as it is done early
			  break;
		  case READ_VREF:
			  //RD_VREF value need to get from cache as CMD_GET_REG on DELAYS_RD_VREF doesn't support VREF code conversion to linear value
			  GetSetDataSignal (ModMrcData, 0, Channel, 0, TrainRank, Strobe, 0, 0, RDQS_VREF, CMD_GET_CACHE, &TempValue);
			  curValue[Strobe] = PhysicalToLinearVrefCodes[TempValue];
			  break;
		  case WRITE_DELAY:
			  GetSetDataSignal (ModMrcData, 0, Channel, 0, TrainRank, Strobe, 0, 0, DELAYS_WDQ_DEL, CMD_GET_CACHE, &curValue[Strobe]);
			  break;
	      case CMD_DELAY :
	    	  //
	    	  // Read the Initial Command values for each group storing them in the CmdValue array for each group.
	    	  //
	    	  for (CmdLoop=0; CmdLoop < MAX_CMDS; CmdLoop++ ) {
	    	  	GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL + CmdLoop, CMD_GET_CACHE, &CmdValue[CmdLoop]);
	    	  }
	    	  break;
	      case WRITE_VREF:
			  //Temporary set it to Center of VREF
			  curValue[Strobe] = 0x20;
			  break;
		  default:
			  //
		      // Signal error
		      //
		      break;
		  }

		  //Set NOT_MET to allow MarginSweep function sweep across all boundary
		  ThresholdMet[Strobe] = NOT_MET;
	  }

	  switch (Mode) {
	  case READ_DELAY:
		  RMTDataLeftLowOffset = RxDqLeft;
		  RMTDataRightHiOffset = RxDqRight;

		  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
			  PfLimits[Strobe][LOW]  = RDTRAIN_LOWBOUNDARY;
			  PfLimits[Strobe][HIGH] = HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);//(HALF_CLK_(2, ModMrcData->CurrentFrequency)) -1;//RDTRAIN_UPBOUNDARY;
		  }

		  //
		  // RMT sweep of the RDQS PI delays when Vref is fixed
		  //
		  break;

	  case READ_VREF:
		  RMTDataLeftLowOffset = RxVLow;
		  RMTDataRightHiOffset = RxVHigh;

		  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
			  PfLimits[Strobe][LOW]  = RDVREFTRAIN_LOWBOUNDARY;
			  PfLimits[Strobe][HIGH] = RDVREFTRAIN_UPBOUNDARY;
		  }

		  //
		  // RMT sweep of the RVref when PI is fixed
		  //
		  break;
	  case WRITE_DELAY:
		  RMTDataLeftLowOffset = TxDqLeft;
		  RMTDataRightHiOffset = TxDqRight;

		  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
			  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQS_DEL, CMD_GET_CACHE, &TempValue);

			  if (TempValue < HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)) {
				  PfLimits[Strobe][LOW]  = 0;
				  PfLimits[Strobe][HIGH] = (UINT16) (TempValue + HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency));
			  } else {
				  PfLimits[Strobe][LOW]  = (UINT16) (TempValue - HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency));
				  PfLimits[Strobe][HIGH] = (UINT16) (TempValue + HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency));
			  }

			  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, WDQS_MIN, CMD_GET_CACHE, &TempValue);
			  if (TempValue == 0) {
				  if (PfLimits[Strobe][LOW] < HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)) {
					  PfLimits[Strobe][LOW]   = 0;
					  PfLimits[Strobe][HIGH] -= HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
				  } else {
					  PfLimits[Strobe][LOW] -= HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
					  PfLimits[Strobe][HIGH] -= HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
				  }
			  }
		  }

		  //
		  // RMT sweep of the WDQ PI delay when WREF is fixed
		  //
		  break;

	  case WRITE_VREF:
		  RMTDataLeftLowOffset = TxVLow;
		  RMTDataRightHiOffset = TxVHigh;

		  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
			  PfLimits[Strobe][LOW]  = WRVREFTRAIN_LOWBOUNDARY;
			  PfLimits[Strobe][HIGH] = WRVREFTRAIN_UPBOUNDARY;
		  }

		  //
		  // RMT sweep of the WVREF delay when PI is fixed
		  //
        break;

	  case CMD_DELAY :
	      RMTDataLeftLowOffset = CmdLeft;
	      RMTDataRightHiOffset = CmdRight;

	      TempValue = CmdValue[0];

	      //Fixed upper and lower boundary on strobe 0 for PfLimits array
	      if (TempValue < HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)) {
	    	  if(ModMrcData->FeatureSettings.MrcDigitalDll){
            PfLimits[0][LOW]  = 0;
	    		  PfLimits[0][HIGH] = (UINT16) (TempValue + RMT_CMD_SWEEP_RANGE);
	    	  } else {
	    		  PfLimits[0][LOW]  = 0;
	    		  PfLimits[0][HIGH] = (UINT16) (TempValue + HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency));
	    	  }
	      } else {
	    	  if(ModMrcData->FeatureSettings.MrcDigitalDll){
	    		  PfLimits[0][LOW]  = (UINT16) (TempValue - RMT_CMD_SWEEP_RANGE);
	    		  PfLimits[0][HIGH] = (UINT16) (TempValue + RMT_CMD_SWEEP_RANGE);
	    	  } else {
	    		  PfLimits[0][LOW]  = (UINT16) (TempValue - HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency));
	    		  PfLimits[0][HIGH] = (UINT16) (TempValue + HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency));
	    	  }
	      }
	      break;

	  default:
		  //
	      // Signal Error
	      //
	      RMTDataLeftLowOffset = 0;
	      RMTDataRightHiOffset = 0;
	      return FAILURE;
    }
	  //
	  // Perform sweep
	  //
	  MarginSweep (
	      ModMrcData,
	      Mode,
	      Channel,
	      Rank,
	      (UINT32) GoldenBufferPtr,
	      &(PfLimits[0][0]),
	      ThresholdMet,
          ModMrcData->FeatureSettings.MrcCPGCNumBursts,		//CPGCNumBursts
          ModMrcData->FeatureSettings.MrcCPGCExpLoopCnt		//CPGCExpLoopCnt
	    );

    //
    // Clean the margins for next rank
    //
    rmtMinLeftMargin = 0;
    rmtMinRigthMragin = 0;

    //
    // Check for valid number of left and right
    // Calculate the total window and
    // Process for the worst margin
    //
    for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
    	if (Mode == READ_DELAY) {
    		curValue[Strobe] = RdqsValue[Rank][Strobe];
    	} else if (Mode == CMD_DELAY){
    		curValue[Strobe] = CmdValue[0];
    	}

    	rmtBlCenter = PfLimits[Strobe][HIGH] - (MARGIN_DISTANCE (PfLimits[Strobe][LOW], PfLimits[Strobe][HIGH]) / 2);
    	rmtBlCenter -= (UINT16)curValue[Strobe];
        //
        // Read Left or Low margin
        //
        rmtBlMargin = PfLimits[Strobe][LOW] - (UINT16)curValue[Strobe];
        rmtDistance = MARGIN_DISTANCE (rmtBlMargin, rmtBlCenter);

        if (rmtMinLeftMargin == 0 || rmtDistance < rmtMinLeftMargin) {
          ModMrcData->Channel[Channel].RMT_Data[Rank][RMTDataLeftLowOffset] = rmtBlMargin;
          rmtMinLeftMargin = rmtDistance;
        }
        //
        // Read Right or High margin
        //
        rmtBlMargin = MARGIN_DISTANCE (PfLimits[Strobe][HIGH], (UINT16)curValue[Strobe]);
        rmtDistance = MARGIN_DISTANCE (rmtBlMargin, rmtBlCenter);
        if (rmtMinRigthMragin == 0 || rmtDistance < rmtMinRigthMragin) {
          ModMrcData->Channel[Channel].RMT_Data[Rank][RMTDataRightHiOffset] = rmtBlMargin;
          rmtMinRigthMragin = rmtDistance;
        }

        if (ModMrcData->Channel[Channel].RMT_Data[Rank][RMTDataLeftLowOffset] > 0){
        	IoOut8(0xCF9, 0xE);
        	MRC_DEADLOOP();
        }

        if (Mode == CMD_DELAY){
        	ModMrcData->Channel[Channel].RMT_Data[1][RMTDataLeftLowOffset] = 0;
        	ModMrcData->Channel[Channel].RMT_Data[1][RMTDataRightHiOffset] = 0;
        	//Exit for loop for CMD_DELAY as CMD_DELAY doesn't have strobe
        	break;
        }
    }//end of for Strobe

    //
    // Restore the PI or Vref value for this Rank after calling MarginSweep function
    //
    for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
    	if (Mode == READ_VREF){
    		TempValue = LinearToPhysicalVrefCodes[curValue[Strobe]];
    		GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RDQS_VREF, CMD_SET_VAL_FC_UC, &TempValue);
    	} else if (Mode ==  WRITE_DELAY) {
    		GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_WDQ_DEL, CMD_SET_VAL_FC_UC, &curValue[Strobe]);
    	}else {//(mode ==  WRITE_VREF) or (mode ==  READ_DELAY)

    	}
    }

    if (Mode ==  CMD_DELAY) {
    	for (CmdLoop=0; CmdLoop < MAX_CMDS; CmdLoop++ ) {
    		GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL + CmdLoop, CMD_SET_VAL_FC, &CmdValue[CmdLoop]);
    	}
		//Exit for loop of Rank
    	Rank = MAX_RANKS;
    }

  }   //end of for rank

  // Restore back Rdqs delay on all enabled Ranks at end of the function as all Ranks's Rqds delay will be changed in MarginSweep function
  // MarginSweep function on READ_DELAY will update current trained Rank value to all Ranks
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank ++) {
	  if (ModMrcData->Channel[Channel].RankEnabled[Rank] == 0) {
		  continue;
	  }
	  if (Mode == READ_DELAY) {
		  for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
			  GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, DELAYS_RDQS_DEL, CMD_SET_VAL_FC_UC, &RdqsValue[Rank][Strobe]);
		  }
	  }
  }
  return SUCCESS;

}  // RmtDqDqsVrefSearch

STATUS
CreatePFCTSel (
  MMRC_DATA *ModMrcData,
  PFCT_VARIATIONS     *PFCTSelect
)
{
  UINT8  CurrentPlatform;
  UINT8  CurrentFrequency;
  UINT8  CurrentConfiguration;
  UINT8  CurrentType;

  CurrentPlatform = ModMrcData->CurrentPlatform;
  CurrentFrequency = ModMrcData->CurrentFrequency;
  CurrentConfiguration = ModMrcData->CurrentConfiguration;
  CurrentType = ModMrcData->CurrentDdrType;

  PFCTSelect[Pfct].pfct     = 0;
  PFCTSelect[PfctT].pfct    = (1 << CurrentType);
  PFCTSelect[PfctC].pfct    = (1 << CurrentConfiguration);
  PFCTSelect[PfctCT].pfct   = ( (1 << CurrentConfiguration) << (NUM_TYPE) ) | (1 << CurrentType);
  PFCTSelect[PfctF].pfct    = (1 << CurrentFrequency);
  PFCTSelect[PfctFT].pfct   = ( (1 << CurrentFrequency) << (NUM_TYPE) ) | (1 << CurrentType);
  PFCTSelect[PfctFC].pfct   = ( (1 << CurrentFrequency) << (NUM_CONF) ) | (1 << CurrentConfiguration);
  PFCTSelect[PfctFCT].pfct  = ( (1 << CurrentFrequency) << (NUM_CONF + NUM_TYPE) ) | ( (1 << CurrentConfiguration) << (NUM_TYPE) ) | ( (1 << CurrentType) );
  PFCTSelect[PfctP].pfct    = (1 << CurrentPlatform);
  PFCTSelect[PfctPT].pfct   = ( (1 << CurrentPlatform) << (NUM_TYPE) ) | (1 << CurrentType);
  PFCTSelect[PfctPC].pfct   = ( (1 << CurrentPlatform) << (NUM_CONF) ) | (1 << CurrentConfiguration);
  PFCTSelect[PfctPCT].pfct  = ( (1 << CurrentPlatform) << (NUM_CONF + NUM_TYPE) ) | ( (1 << CurrentConfiguration) << (NUM_TYPE) ) | (1 << CurrentType);
  PFCTSelect[PfctPF].pfct   = ( (1 << CurrentPlatform) << (NUM_FREQ) ) | (1 << CurrentFrequency);
  PFCTSelect[PfctPFT].pfct  = ( (1 << CurrentPlatform) << (NUM_FREQ + NUM_TYPE) ) | ( (1 << CurrentFrequency) << (NUM_TYPE) ) | (1 << CurrentType);
  PFCTSelect[PfctPFC].pfct  = ( (1 << CurrentPlatform) << (NUM_FREQ + NUM_CONF) ) | ( (1 << CurrentFrequency) << (NUM_CONF) ) | (1 << CurrentConfiguration);
  PFCTSelect[PfctPFCT].pfct = ( (1 << CurrentPlatform) << (NUM_FREQ + NUM_CONF + NUM_TYPE) ) | ( (1 << CurrentFrequency) << (NUM_CONF + NUM_TYPE) ) | ( (1 << CurrentConfiguration) << (NUM_TYPE) ) | (1 << CurrentType);

  PFCTSelect[Pfct].length     = 0;
  PFCTSelect[PfctT].length    = (NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctC].length    = (NUM_CONF - 1) / 8 + 1;
  PFCTSelect[PfctCT].length   = (NUM_CONF + NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctF].length    = (NUM_FREQ - 1) / 8 + 1;
  PFCTSelect[PfctFT].length   = (NUM_FREQ + NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctFC].length   = (NUM_FREQ + NUM_CONF - 1) / 8 + 1;
  PFCTSelect[PfctFCT].length  = (NUM_FREQ + NUM_CONF + NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctP].length    = (NUM_PLAT - 1) / 8 + 1;
  PFCTSelect[PfctPT].length   = (NUM_PLAT + NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctPC].length   = (NUM_PLAT + NUM_CONF - 1) / 8 + 1;
  PFCTSelect[PfctPCT].length  = (NUM_PLAT + NUM_CONF + NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctPF].length   = (NUM_PLAT + NUM_FREQ - 1) / 8 + 1;
  PFCTSelect[PfctPFT].length  = (NUM_PLAT + NUM_FREQ + NUM_TYPE - 1) / 8 + 1;
  PFCTSelect[PfctPFC].length  = (NUM_PLAT + NUM_FREQ + NUM_CONF - 1) / 8 + 1;
  PFCTSelect[PfctPFCT].length = (NUM_PLAT + NUM_FREQ + NUM_CONF + NUM_TYPE - 1) / 8 + 1;

  return SUCCESS;
}

/*********************************************************************************************************************
Function Name: CPGC_S_Enable()
*********************************************************************************************************************
Date:  07/25/2012
*********************************************************************************************************************
Input:
    Channel: Specified the channel number to enable CPGC
*********************************************************************************************************************
Return:
    Always return CPGC_STS_SUCCESS
*********************************************************************************************************************
Description:
This function will enabled the CPGC engine based on the input Channel to access the DIMM.
It should be noted that once the CPGC access has been enabled, the CPU must not access the RAM as it will hang the system.
This function will return PASS always, as there is no reason why the CPGC engine cannot be enabled
*********************************************************************************************************************
Modifications:
    Date      : Name   : Description
    07/25/2012:        : Initial Creation
********************************************************************************************************************/
static
STATUS
CPGC_S_Enable (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel
)
{
  RegDCO  DCO;

  //Read DUNIT DRAM Control Operation register
  DCO.raw = MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, MC_DCO_OFFSET);
  //set PMICTL to 1 connect Dunit PMI to REUT.
  DCO.field.PMICTL = 1;
  //set PMIDIS to 1 to disable the PMI interface from both Bunit and REUT.
  DCO.field.PMIDIS = 0;
  //Write final Value to DUNIT DRAM Control Operation register
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, MC_DCO_OFFSET, DCO.raw);

  return CPGC_STS_SUCCESS;
}

/*********************************************************************************************************************
Function Name: CPGC_S_Disable()
*********************************************************************************************************************
Date:  07/25/2012
*********************************************************************************************************************
Input:
    Channel: Specified the channel number to enable CPGC
*********************************************************************************************************************
Return:
    Always return CPGC_STS_SUCCESS
*********************************************************************************************************************
Description:
This function will disable the CPGC engine base on input Channel, and control of the DIMMs will return to the CPU.
This function will return PASS always, as there is no reason why the CPGC engine cannot be disabled.
*********************************************************************************************************************
Modifications:
    Date      : Name   : Description
    07/25/2012:        : Initial Creation
********************************************************************************************************************/
static
STATUS
CPGC_S_Disable (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel
)
{
  RegDCO  DCO;

  //Read DUNIT DRAM Control Operation register
  DCO.raw = MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, MC_DCO_OFFSET);
  //set PMICTL to 0 connect Dunit PMI to Bunit.
  DCO.field.PMICTL = 0;
  //set PMIDIS to 0 to enable the PMI interface from both Bunit and REUT.
  DCO.field.PMIDIS = 0;
  //Write final Value to DUNIT DRAM Control Operation register
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, MC_DCO_OFFSET, DCO.raw);

  return CPGC_STS_SUCCESS;
}

/*********************************************************************************************************************
Function Name: CPGC_S_SetupSeq()
*********************************************************************************************************************
Date:  07/25/2012
*********************************************************************************************************************
Input:
    Channel:  Specified the channel number of CPGC
    SubsequenceStart: Specified Pointer to first subsequence in a command sequence loop.
            Please use define below
            CPGC_SUBSEQINDEX_0
          CPGC_SUBSEQINDEX_1
          CPGC_SUBSEQINDEX_2
          CPGC_SUBSEQINDEX_3
    SubsequenceEnd:   Specified Pointer to last subsequence in a command sequence loop
              Please use define below
            CPGC_SUBSEQINDEX_0
          CPGC_SUBSEQINDEX_1
          CPGC_SUBSEQINDEX_2
          CPGC_SUBSEQINDEX_3
    LoopCount:    Specified number of the command sequence loops to issue to 2^(EXP_LOOP_CNT?1) before a test exits normally
*********************************************************************************************************************
Example:
  CPGC_S_SetupSeq(Channel, CPGC_SUBSEQINDEX_0, CPGC_SUBSEQINDEX_0,0x1,0x0);
*********************************************************************************************************************
Return:
    CPGC_STS_NOT_SUPPORTED: If SubsequenceStart OR SubsequenceEnd not within allowed number
  CPGC_STS_SUCCESS:   If Setup sequence is successfully done
*********************************************************************************************************************
Description:
This is the control of the main sequence within the CPGC engine.
As there are 4 allowed subsequencers within the system, this API can select specify start subsequence and end subsequence
and the number of interactions to perform on the selected subsequencers
The "LoopCount" is the number of iterations that will be taken before the test exits normally.
The "LoopCount" field is in the format of 2N-1 so if n=1, then 20 = 1 or 1 iteration is completed.
If n=5, 25-1=24=16 iterations will be taken.  If n=0, the system will loop on indefinitely across the programmed subsequencers.
*********************************************************************************************************************
Modifications:
    Date      : Name   : Description
    07/25/2012:        : Initial Creation
********************************************************************************************************************/
static
STATUS
CPGC_S_SetupSeq (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel,
  UINT8 SubsequenceStart,
  UINT8 SubsequenceEnd,
  UINT8 LoopCount,
  UINT8 StopOnWrap
)
{
  CPGC_CMD_CTL_STRUCT CPGC_CMD_CTL;

  //Checking if SubsequenceStart and SubsequenceEnd a valid sub sequence number
  if ( (SubsequenceStart > CPGC_SUBSEQINDEX_3 ) || (SubsequenceEnd > CPGC_SUBSEQINDEX_3 ) ) {
    return CPGC_STS_NOT_SUPPORTED;
  }

  //Read CPGC Command Sequence Control register
  CPGC_CMD_CTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_CTL_REG);
  //Set Exponential Loop Count (EXP_LOOP_CNT) as per input LoopCount
  CPGC_CMD_CTL.Bits.exp_loop_cnt = LoopCount;
  //Set Initialization Mode (INIT_MODE) to "0b01" ?ACTIVE MODE
  CPGC_CMD_CTL.Bits.init_mode = 1;
  //Set Sequence End pointer (SEQ_END_PTR) as per input SubsequenceEnd
  CPGC_CMD_CTL.Bits.seq_end_ptr = SubsequenceEnd;
  //Set Sequence Start pointer (SEQ_STRT_PTR) as per input SubsequenceStart
  CPGC_CMD_CTL.Bits.seq_strt_ptr = SubsequenceStart;
  //Set Stop On Wrap (STOP_ON_WRAP) to disable
  CPGC_CMD_CTL.Bits.stop_on_wrap = StopOnWrap;
  //Write final Value to CPGC Command Sequence Control register
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_CTL_REG, CPGC_CMD_CTL.Data);

  return CPGC_STS_SUCCESS;
}

static
STATUS
CPGC_S_SetupSubseq (
  IN        MMRC_DATA     *ModMrcData,
  IN        UINT8         Channel,
  IN        UINT8         SubsequenceIndex,
  IN        UINT8         Type,
  IN        UINT8         BurstNumber,
  IN        UINT32        Address,
  IN        UINT8         ResetAddressFlag,
  IN        UINT8         VarScramble,
  IN        UINT8         SubsequenceWait,
  IN        UINT8         VarWidth,
  IN        UINT16        VarSegment,
  IN        UINT8         VarHiShift,
  IN        UINT8         VarLowShift
)
/*++

Routine Description:

  This API used to configure specified subsequencers "Type", "Number of Bursts", "Reset Address" and  "Address".
  The Type field specified what Type of Action is to be performed by the subsequencer, the available types are:
  The BurstNumber is the number of consecutive cache lines that will be accesses by the subsequence.
  The BurstNumber is in the form of 2^(BurstNumber - 1)., so if BurstNumber=1, then cache line = 1. The range of BurstNumber is from 0-15.
  BurstNumber = 0 implies an infinite number of cache lines
  Address is the starting Address of the memory that will be operated by the subsequencer.


Arguments:

  ModMrcData:       Host structure for all data related to MMRC
  Channel:          Current Channel being examined.
  SubsequenceIndex: Specified sub sequence Index to be configured.
                    Please use define below
                    CPGC_SUBSEQINDEX_0
                    CPGC_SUBSEQINDEX_1
                    CPGC_SUBSEQINDEX_2
                    CPGC_SUBSEQINDEX_3
  Type:             Specified Type of commands to be performed by the subsequencer
                    Please use define below
                    CPGC_SUBSEQ_TYPE_RO:  Read
                    CPGC_SUBSEQ_TYPE_WO:  Write
                    CPGC_SUBSEQ_TYPE_RW:  Read -> Write
                    CPGC_SUBSEQ_TYPE_WR:  write -> Read
                    CPGC_SUBSEQ_TYPE_ROREF: Read (Refresh Blocked)
                    CPGC_SUBSEQ_TYPE_WOREF: Write (Refresh Blocked)
                    CPGC_SUBSEQ_TYPE_RWREF: Read -> Write (Refresh Blocked)
                    CPGC_SUBSEQ_TYPE_WRREF: Write -> Read (Refresh Blocked)
    BurstNumber:    Specified the number of consecutive cache lines that will be accesses by the subsequence
    Address:        Specified the starting Address right shifted by 6 of the memory that will be operated by the subsequencer.
    ResetAddressFlag: Bit Flag to force the subsequence to reload its initial Address from each time before it starts.
    VarScramble:    If set to 1 and using LFSR with 0 seed, CPGC will target the same address over and over.
    SubsequenceWait: Number of cycles between the end of a subsequence an the beginning of the next one
    VarWidth:       The width of the VAR Address segment for the sequence
    VarSegment:     The width of the VAR Address segment for the command sequence
    VarHiShift:     The upper (VarWidth - 3) bits of the VAR address segment will be shifted to the left by this value
    VarLowShift:    The lower 3 bits of the VAR address segment will be shifted to the left by this value
 
 
Returns:

  Success
  Failure

--*/
{
  CPGC_CMD_SEQ0CTL_STRUCT CPGC_CMD_SEQCTL;
  CPGC_CMD_SEQ0VAR_STRUCT CPGC_CMD_SEQVAR;
  CPGC_CMD_SEQ0FIX_STRUCT CPGC_CMD_SEQFIX;
  CPGC_CMD_CAP_STRUCT     CPGC_CMD_CAP;
  //
  // Read CPGC Command Sequence Capability register
  //
  CPGC_CMD_CAP.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_CAP_REG);
  //
  // Check if Refresh Block Capability is supported
  //
  if ( (CPGC_CMD_CAP.Bits.ref_blk_cap == 0) && (Type & 0x08) ) {
    return CPGC_STS_NOT_SUPPORTED;
  }
  //
  // Check if Sub sequence Index is within allowed number
  //
  if (SubsequenceIndex > CPGC_SUBSEQINDEX_3 ) {
    return CPGC_STS_NOT_SUPPORTED;
  }
  //
  // Check if burst number is not more than supported 0xF cache lines
  //
  if (BurstNumber > 0xF) {
    return CPGC_STS_NOT_SUPPORTED;
  }
  //
  // Read CPGC Subsequence Control/Variable Address/Wrap Address registers according to Index number
  //
  switch (SubsequenceIndex) {
  case CPGC_SUBSEQINDEX_0:
    CPGC_CMD_SEQCTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ0CTL_REG);
    CPGC_CMD_SEQVAR.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ0VAR_REG);
    CPGC_CMD_SEQFIX.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ0FIX_REG);
    break;
  case CPGC_SUBSEQINDEX_1:
    CPGC_CMD_SEQCTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ1CTL_REG);
    CPGC_CMD_SEQVAR.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ1VAR_REG);
    CPGC_CMD_SEQFIX.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ1FIX_REG);
    break;
  case CPGC_SUBSEQINDEX_2:
    CPGC_CMD_SEQCTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ2CTL_REG);
    CPGC_CMD_SEQVAR.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ2VAR_REG);
    CPGC_CMD_SEQFIX.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ2FIX_REG);
    break;
  case CPGC_SUBSEQINDEX_3:
    CPGC_CMD_SEQCTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ3CTL_REG);
    CPGC_CMD_SEQVAR.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ3VAR_REG);
    CPGC_CMD_SEQFIX.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ3FIX_REG);
    break;
  }
  //
  // Set Subsequence Type (SUBSEQ_TYPE) as per input Type
  //
  CPGC_CMD_SEQCTL.Bits.subseq_type = Type;
  //
  // Set Number of Bursts (NUM_BURSTS) as per input BurstNumber
  //
  CPGC_CMD_SEQCTL.Bits.num_bursts = BurstNumber;
  //
  // Set Reset Address (RESET_ ADDR) as per input ResetAddressFlag
  //
  CPGC_CMD_SEQCTL.Bits.reset_addr = ResetAddressFlag;
  //
  // Set Request Data Size (REQ_DATA_SIZE) to 1 ?64 Byte Transactions,
  //
  if (ModMrcData->MaxDq <= (MAX_STROBES/2)) {
      CPGC_CMD_SEQCTL.Bits.req_data_size = 0;//32B
  } else {
	  CPGC_CMD_SEQCTL.Bits.req_data_size = 1;  //64B
  }

  //
  // Set Subsequence Wait (SUBSEQ_WAIT) to the passed in parameter.
  //
  CPGC_CMD_SEQCTL.Bits.subseq_wait = SubsequenceWait;
  CPGC_CMD_SEQCTL.Bits.subseq_wait_ref_blk = 1;
  CPGC_CMD_SEQCTL.Bits.comp_on_wrap = 0;

  CPGC_CMD_SEQVAR.Bits2.var_hi_shift = VarHiShift;
  CPGC_CMD_SEQVAR.Bits2.var_lo_shift = VarLowShift;
  CPGC_CMD_SEQVAR.Bits2.var_scramble = VarScramble;
  CPGC_CMD_SEQVAR.Bits2.var_segment = VarSegment;
  CPGC_CMD_SEQVAR.Bits2.var_width = VarWidth;
  //
  // Set starting Address for the subsequence as per input Address
  //
  CPGC_CMD_SEQFIX.Bits2.fix_segment = Address;
  //
  // Write final Value to CPGC Subsequence Control/Variable Address/Wrap Address registers according to Index number
  //
  switch (SubsequenceIndex) {
  case CPGC_SUBSEQINDEX_0:
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ0CTL_REG, CPGC_CMD_SEQCTL.Data);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ0VAR_REG, CPGC_CMD_SEQVAR.Data);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ0FIX_REG, CPGC_CMD_SEQFIX.Data);
    break;
  case CPGC_SUBSEQINDEX_1:
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ1CTL_REG, CPGC_CMD_SEQCTL.Data);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ1VAR_REG, CPGC_CMD_SEQVAR.Data);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ1FIX_REG, CPGC_CMD_SEQFIX.Data);
    break;
  case CPGC_SUBSEQINDEX_2:
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ2CTL_REG, CPGC_CMD_SEQCTL.Data);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ2VAR_REG, CPGC_CMD_SEQVAR.Data);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ2FIX_REG, CPGC_CMD_SEQFIX.Data);
    break;
  case CPGC_SUBSEQINDEX_3:
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ3CTL_REG, CPGC_CMD_SEQCTL.Data);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ3VAR_REG, CPGC_CMD_SEQVAR.Data);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ3FIX_REG, CPGC_CMD_SEQFIX.Data);
    break;
  }

  return CPGC_STS_SUCCESS;
}

static
STATUS
CPGC_S_SetUnisequencer (
  IN OUT    MMRC_DATA  *ModMrcData,
  IN        UINT8       Channel,
  IN        UINT8       UniSequencerIndex,
  IN        UINT8       UniSequencerMode,
  IN        UINT32      UniSequencerDataPattern,
  IN        UINT8       UniSequencerReloadEnable
)
/*++

Routine Description:

  This API is used to configure specified Unified Sequencer Data Pattern mode and Data Pattern

Arguments:

  ModMrcData:       Host structure for all data related to MMRC
  Channel:          Current Channel being examined.
  UniSequencerIndex:Specified Unified sequencer number to be configured.
                    Please use define below
                    CPGC_UNISEQINDEX_0
                    CPGC_UNISEQINDEX_1
                    CPGC_UNISEQINDEX_2
  UniSequencerMode: Specified the operational mode for unified sequence
                    Please use define below
                    CPGC_UNISEQINDEX_LMN: LMN Mode
                    CPGC_UNISEQINDEX_PBM: Pattern Buffer Mode
  UniSequencerDataPattern:  Specified Data Pattern

 
Returns:

  Success
  Failure

--*/
{
  CPGC_DPAT_MUXCTL_STRUCT CPGC_DPAT_MUXCTL;
  // 
  // Read CPGC Data Pattern Mux Control register
  //
  CPGC_DPAT_MUXCTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_DPAT_MUXCTL_REG);
  // 
  // Set Data Pattern Unified Sequencer X Mode (UNISEQX_MODE) as per input UniSequencerIndex
  // Set Data Pattern to specified Index number as per input UniSequencerMode
  //
  switch (UniSequencerIndex) {
  case CPGC_UNISEQINDEX_0:
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_DPAT_UNISEQ0_REG, UniSequencerDataPattern);
    CPGC_DPAT_MUXCTL.Bits.uniseq0_mode = UniSequencerMode;
    break;
  case CPGC_UNISEQINDEX_1:
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_DPAT_UNISEQ1_REG, UniSequencerDataPattern);
    CPGC_DPAT_MUXCTL.Bits.uniseq1_mode = UniSequencerMode;
    break;
  case CPGC_UNISEQINDEX_2:
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_DPAT_UNISEQ2_REG, UniSequencerDataPattern);
    CPGC_DPAT_MUXCTL.Bits.uniseq2_mode = UniSequencerMode;
    break;
  default:
    return CPGC_STS_NOT_SUPPORTED;
    break;
  }
  // 
  // Disable Unified Sequencer Reload Enable
  //
  CPGC_DPAT_MUXCTL.Bits.uniseq_reload_en = UniSequencerReloadEnable;
  CPGC_DPAT_MUXCTL.Bits.ecc_disable = !(ModMrcData->EccEnabled);

  //
  // Write final Value to CPGC Data Pattern Mux Control register
  //
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_DPAT_MUXCTL_REG, CPGC_DPAT_MUXCTL.Data);
  return CPGC_STS_SUCCESS;
}

/*********************************************************************************************************************
Function Name: CPGC_S_SetupPattern()
*********************************************************************************************************************
Date:  07/25/2012
*********************************************************************************************************************
Input:
    Channel:  Specified the channel number of CPGC
    PatternIndex: Specified Pattern Index to be configured.
            Please use define below
            CPGC_DPAT0
          CPGC_DPAT1
          CPGC_DPAT2
          CPGC_DPAT4
          CPGC_DPAT5
          CPGC_DPAT6
          CPGC_DPAT7
          CPGC_DPAT8
          CPGC_DPAT9
          CPGC_DPAT10
          CPGC_DPAT11
          CPGC_DPAT12
          CPGC_DPAT13
          CPGC_DPAT14
          CPGC_DPAT15
    PatternBufDataL0:Specified the Pattern Buffer of Data Line 0
    PatternBufDataL1:Specified the Pattern Buffer of Data Line 1
    PatternBufDataL2:Specified the Pattern Buffer of Data Line 2
    PatternBufDataL3:Specified the Pattern Buffer of Data Line 3
*********************************************************************************************************************
Example:
  CPGC_S_SetupPattern(Channel, CPGC_DPAT0,0xAA, 0x00, 0x00, 0x00);
*********************************************************************************************************************
Return:
    CPGC_STS_FAILURE: If Pattern Index is not within allowed Index number
  CPGC_STS_SUCCESS: If Setup Pattern is successfully done
*********************************************************************************************************************
Description:
This API is used to setup the individual patterns for each of the 16 DQ buffers that are valid.
Because the bus is 64-Bits and there are only 16 DQ Buffers, setting DQ0's buffer also affects DQ16, DQ32, DQ48.
*********************************************************************************************************************
Modifications:
    Date      : Name   : Description
    07/25/2012:        : Initial Creation
********************************************************************************************************************/
static
STATUS
CPGC_S_SetupPattern (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel,
  UINT8 PatternIndex,
  UINT8 PatternBufDataL0,
  UINT8 PatternBufDataL1,
  UINT8 PatternBufDataL2,
  UINT8 PatternBufDataL3
)
{
  CPGC_DPAT_EXTBUF0_STRUCT  CPGC_DPAT_EXTBUF0;

  if (PatternIndex <= CPGC_DPAT15)  {
    //Set patter buffer of Data Line 0-3
    CPGC_DPAT_EXTBUF0.Bits.data_line0 = PatternBufDataL0;
    CPGC_DPAT_EXTBUF0.Bits.data_line1 = PatternBufDataL1;
    CPGC_DPAT_EXTBUF0.Bits.data_line2 = PatternBufDataL2;
    CPGC_DPAT_EXTBUF0.Bits.data_line3 = PatternBufDataL3;
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_DPAT_EXTBUF0_REG + PatternIndex, CPGC_DPAT_EXTBUF0.Data);
    return CPGC_STS_SUCCESS;
  } else {
    return CPGC_STS_FAILURE;
  }
}

/*********************************************************************************************************************
Function Name: CPGC_S_SetupPatternControl()
*********************************************************************************************************************
Date:  07/25/2012
*********************************************************************************************************************
Input:
    Channel:  Specified the channel number of CPGC
*********************************************************************************************************************
Example:
  CPGC_S_SetupPatternControl(Channel);
*********************************************************************************************************************
Return:
  CPGC_STS_SUCCESS: If CPGC_S_SetupPatternControl is successfully done
*********************************************************************************************************************
Description:
This API is used to setup the Data Pattern buffer and MUX control
*********************************************************************************************************************
Modifications:
    Date      : Name   : Description
    07/25/2012:        : Initial Creation
********************************************************************************************************************/
static
STATUS
CPGC_S_SetupPatternControl (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel
)
{
  CPGC_DPAT_BUFCTL_STRUCT CPGC_DPAT_BUFCTL;
  CPGC_DPAT_MUXCTL_STRUCT CPGC_DPAT_MUXCTL;
  CPGC_DPAT_INVDCCTL_STRUCT CPGC_DPAT_INVDCCTL;

  //Read CPGC Data Pattern Buffer Control register
  CPGC_DPAT_BUFCTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_DPAT_BUFCTL_REG);
  //Read CPGC Data Pattern Mux Control register
  CPGC_DPAT_MUXCTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_DPAT_MUXCTL_REG);
  //Read CPGC Data Pattern Inversion/DC Control register
  CPGC_DPAT_INVDCCTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_DPAT_INVDCCTL_REG);

  //Set Buffer Starting Pointer (BUF_STRT_PTR) and Buffer End Pointer (BUF_END_PTR) to 0, so that it only using data_line0
  CPGC_DPAT_BUFCTL.Bits.buf_strt_ptr = 0;
  CPGC_DPAT_BUFCTL.Bits.buf_end_ptr = 0;
  //Disable Buffer Pointer Increment Enable and the buffer entry pointed to BUF_STRT_PTR will be used for the entire test.
  CPGC_DPAT_BUFCTL.Bits.buf_ptr_inc_en = 0;

  //Disable Unified Sequencer Reload Enable
  CPGC_DPAT_MUXCTL.Bits.uniseq_reload_en = 0;
  // ECC
  CPGC_DPAT_MUXCTL.Bits.ecc_disable = !(ModMrcData->EccEnabled);

  //Enable Extended Data pattern Buffer Rotation and set the rate to 3
  CPGC_DPAT_INVDCCTL.Bits.mask_rotate_en = 1;
  CPGC_DPAT_INVDCCTL.Bits.mask_rotate_rate = 3;

  //Write final Value to CPGC Pattern Buffer Control registe
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_DPAT_BUFCTL_REG, CPGC_DPAT_BUFCTL.Data);
  //Write final Value to CPGC Data Pattern Mux Control register
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_DPAT_MUXCTL_REG, CPGC_DPAT_MUXCTL.Data);
  //Write final Value to CPGC Data Pattern Inversion/DC Control register
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_DPAT_INVDCCTL_REG, CPGC_DPAT_INVDCCTL.Data);

  return CPGC_STS_SUCCESS;
}

/*********************************************************************************************************************
Function Name: CPGC_S_ClearErrors()
*********************************************************************************************************************
Date:  07/25/2012
*********************************************************************************************************************
Input:
    Channel:  Specified the channel number of CPGC
*********************************************************************************************************************
Example:
  CPGC_S_ClearErrors(Channel);
*********************************************************************************************************************
Return:
  CPGC_STS_SUCCESS: If CPGC_S_ClearErrors is successfully done
*********************************************************************************************************************
Description:
This API will clear the error Bits within the CPGC engine so that a new test can be executed and the results will be valid
*********************************************************************************************************************
Modifications:
    Date      : Name   : Description
    07/25/2012:        : Initial Creation
********************************************************************************************************************/
static
STATUS
CPGC_S_ClearErrors (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel
)
{
  CPGC_ERR_CTL_STRUCT CPGC_ERR_CTL;

  //Read CPGC Error Checker Control register
  CPGC_ERR_CTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_ERR_CTL_REG);

  //Set Clear All Errors (CLR_ALL_ERR) bit to clear all error registers and error Status
  CPGC_ERR_CTL.Bits.clr_all_err = 1;
  //Write final Value to CPGC Error Checker Control register
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_ERR_CTL_REG, CPGC_ERR_CTL.Data);

  return CPGC_STS_SUCCESS;
}

/*********************************************************************************************************************
Function Name: CPGC_S_EnableErrors()
*********************************************************************************************************************
Date:  07/25/2012
*********************************************************************************************************************
Input:
    Channel:    Specified the channel number of CPGC
    SelBurstErrCheckEn: Specified Burst Error Check enabled for Burst 0 - 7
                Bit 0 ?Burst 0 (CAS command 0)
              Bit 1 ?Burst 1 (CAS command 1)
              Bit 2 ?Burst 2 (CAS command 2)
              Bit 3 ?Burst 3 (CAS command 3)
              Bit 4 ?Burst 4 (CAS command 4)
              Bit 5 ?Burst 5 (CAS command 5)
              Bit 6 ?Burst 6 (CAS command 6)
              Bit 7 ?Burst 7 (CAS command 7)
    SelChunkErrCheckEn: Specified which chunk within the burst of Data to check for errors.
    ErrCheckLowMask:  Specified which chunk within the burst of Data to check for errors.
    ErrCheckHighMask: Specified which chunk within the burst of Data to check for errors.
    ErrCheckECCMask:  Specified which chunk within the burst of Data to check for errors.
*********************************************************************************************************************
Example:
  CPGC_S_EnableErrors(Channel,  0xff, 0xff, 0xffffffff, 0xffffffff, 0x0 );
*********************************************************************************************************************
Return:
  CPGC_STS_SUCCESS: If CPGC_S_EnableErrors is successfully done
*********************************************************************************************************************
Description:
This API is used to enable the Error checker and specified Error checked function.
The 8 Burst (CAS command) and chunk are bit-field specified in "SelBurstErrCheckEn" and "SelChunkErrCheckEn"
The 64 DQ lines are bit-field specified between "ErrCheckLowMask" and "ErrCheckHighMask"
The ECC byte lane check is bit-field specified in "ErrCheckECCMask"
*********************************************************************************************************************
Modifications:
    Date      : Name   : Description
    07/25/2012:        : Initial Creation
********************************************************************************************************************/
static
STATUS
CPGC_S_EnableErrors (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel,
  UINT8 SelBurstErrCheckEn,
  UINT8 SelChunkErrCheckEn,
  UINT32 ErrCheckLowMask,
  UINT32 ErrCheckHighMask,
  UINT32 ErrCheckECCMask)
{
  CPGC_ERR_CTL_STRUCT           CPGC_ERR_CTL;
  CPGC_ERR_LANE_MASK_LO_STRUCT  CPGC_ERR_LANE_MASK_LO;
  CPGC_ERR_LANE_MASK_HI_STRUCT  CPGC_ERR_LANE_MASK_HI;
  CPGC_ERR_LANE_XMASK_STRUCT    CPGC_ERR_LANE_XMASK;
  //Read CPGC Error Checker Control register
  CPGC_ERR_CTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_ERR_CTL_REG);

  //Set Selective Burst Error Check Enable as per input SelBurstErrCheckEn
  CPGC_ERR_CTL.Bits.burst_errchk_en = SelBurstErrCheckEn;
  //Set Selective Chunk Error Check Enable as per input SelChunkErrCheckEn
  CPGC_ERR_CTL.Bits.chunk_errchk_en = SelChunkErrCheckEn;
  //Set Stop On Error "0b00" ?Never stop: Prevents any error from stopping the test.
  CPGC_ERR_CTL.Bits.stop_on_error = 0;

  //Set Low and High Data lanes (DQ pins) error checking as per input ErrCheckLowMask and ErrCheckHighMask
  CPGC_ERR_LANE_MASK_LO.Data = ErrCheckLowMask;
  CPGC_ERR_LANE_MASK_HI.Data = ErrCheckHighMask;
  //Set ECC Data lanes (DQ pins) error checking as per input ErrCheckECCMask
  CPGC_ERR_LANE_XMASK.Data = ErrCheckECCMask;

  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_ERR_CTL_REG, CPGC_ERR_CTL.Data);
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_ERR_LANE_MASK_LO_REG, CPGC_ERR_LANE_MASK_LO.Data);
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_ERR_LANE_MASK_HI_REG, CPGC_ERR_LANE_MASK_HI.Data);
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_ERR_LANE_XMASK_REG, CPGC_ERR_LANE_XMASK.Data);

  return CPGC_STS_SUCCESS;
}

/*********************************************************************************************************************
Function Name: CPGC_S_CheckErrors()
*********************************************************************************************************************
Date:  07/25/2012
*********************************************************************************************************************
Input:
    Channel:  Specified the channel number of CPGC
*********************************************************************************************************************
Example:
  CPGC_S_CheckErrors(Channel, NULL, NULL, &(UINT8)compareFlag, NULL);
*********************************************************************************************************************
Return:
  CPGC_STS_SUCCESS: If CPGC_S_CheckErrors is successfully done
*********************************************************************************************************************
Description:
This API will report out the error Status for byte level, bit level and ECC lane
If the check low/high Bits variables are non-NULL, the bit field results are copied into the pointer location.
Similarly, if the Byte Lane or ECC or non-null, these results are copied into their respective locations.
*********************************************************************************************************************
Modifications:
    Date      : Name   : Description
    07/25/2012:        : Initial Creation
********************************************************************************************************************/
static
STATUS
CPGC_S_CheckErrors (
  MMRC_DATA  *ModMrcData,
  UINT8   Channel,
  UINT32 *LowBitsErrStat,
  UINT32 *HighBitsErrStat,
  UINT32  *ByteLanesErrStat,
  UINT8  *ECCErrStat
)
{
  CPGC_ERR_STAT_LO_STRUCT CPGC_ERR_STAT_LO;
  CPGC_ERR_STAT_HI_STRUCT CPGC_ERR_STAT_HI;
  CPGC_ERR_XSTAT_STRUCT   CPGC_ERR_XSTAT;
  //Read CPGC Byte Group 0-3 Lane Error Status register
  CPGC_ERR_STAT_LO.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_ERR_STAT_LO_REG);
  //Read CPGC Byte Group 4-7 Lane Error Status register
  CPGC_ERR_STAT_HI.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_ERR_STAT_HI_REG);
  //Read CPGC Extended Error Status register
  CPGC_ERR_XSTAT.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_ERR_XSTAT_REG);

  //Copy Error Status Value to each respective pointer
  if (LowBitsErrStat != NULL) {
    *LowBitsErrStat = CPGC_ERR_STAT_LO.Data;
  }
  if (HighBitsErrStat != NULL) {
    *HighBitsErrStat = CPGC_ERR_STAT_HI.Data;
  }
  if (ByteLanesErrStat != NULL) {
    *ByteLanesErrStat = (UINT8) CPGC_ERR_XSTAT.Bits.bytegrp_err_stat;
    if (Channel == 1) {
		UINT32 compareFlag_bit0 = (*ByteLanesErrStat) & 0x00000001;
		UINT32 compareFlag_bit2 = ((*ByteLanesErrStat) & 0x00000004) >> 2;
		(*ByteLanesErrStat) &= 0xFFFFFFFA;
		(*ByteLanesErrStat) |= (compareFlag_bit0 << 2);
		(*ByteLanesErrStat) |= (compareFlag_bit2);
  	}
	if (ModMrcData->EccEnabled) {
    *ByteLanesErrStat |= (CPGC_ERR_XSTAT.Bits.ecc_lane_err_stat << 8);
	}
  }
  if (ECCErrStat != NULL) {
    *ECCErrStat = (UINT8) CPGC_ERR_XSTAT.Bits.ecc_lane_err_stat;
  }
  return CPGC_STS_SUCCESS;
}

/*********************************************************************************************************************
Function Name: CPGC_S_StartTest()
*********************************************************************************************************************
Date:  07/25/2012
*********************************************************************************************************************
Input:
    Channel:  Specified the channel number of CPGC
*********************************************************************************************************************
Example:
  CPGC_S_StartTest(Channel);
*********************************************************************************************************************
Return:
  CPGC_STS_SUCCESS: If CPGC_S_StartTest is successfully done
*********************************************************************************************************************
Description:
This API will start the test that has been setup prior.
The function returns immediately, the software should then use the CPGC_S_PollTest() to check when the test is completed.
*********************************************************************************************************************
Modifications:
    Date      : Name   : Description
    07/25/2012:        : Initial Creation
********************************************************************************************************************/
static
STATUS
CPGC_S_StartTest (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel,
  UINT8       Rank
)
{
  CPGC_CMD_CTL_STRUCT CPGC_CMD_CTL;
  //CPGC_CMD_SEQ0FIX_STRUCT CPGC_CMD_SEQFIX;

  //CPGC_CMD_SEQFIX.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ0FIX_REG);
  //CPGC_CMD_SEQFIX.Bits2.fix_segment = GetAddress (ModMrcData, Channel, Rank) >> 6;
  //MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ0FIX_REG, CPGC_CMD_SEQFIX.Data);

  //CPGC_CMD_SEQFIX.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ1FIX_REG);
  //CPGC_CMD_SEQFIX.Bits2.fix_segment = GetAddress (ModMrcData, Channel, Rank) >> 6;
  //MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_SEQ1FIX_REG, CPGC_CMD_SEQFIX.Data);

  //Read CPGC BCommand Sequence Control register
  CPGC_CMD_CTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_CTL_REG);
  //Set Start Test (START_TEST) = "1" to start CPGC test
  CPGC_CMD_CTL.Bits.start_test = 1;
  //Write final Value to CPGC BCommand Sequence Control register
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_CTL_REG, CPGC_CMD_CTL.Data);

  return CPGC_STS_SUCCESS;
}

/*********************************************************************************************************************
Function Name: CPGC_S_StopTest()
*********************************************************************************************************************
Date:  07/25/2012
*********************************************************************************************************************
Input:
    Channel:  Specified the channel number of CPGC
*********************************************************************************************************************
Example:
  CPGC_S_StopTest(Channel);
*********************************************************************************************************************
Return:
  CPGC_STS_SUCCESS: If CPGC_S_StopTest is successfully done
*********************************************************************************************************************
Description:
This API will stop the current test that is running.
If the test is executed with a fixed number of iterations, the test will complete on its own.
The system cannot start another test, until the StopTest is executed.
*********************************************************************************************************************
Modifications:
    Date      : Name   : Description
    07/25/2012:        : Initial Creation
********************************************************************************************************************/
static
STATUS
CPGC_S_StopTest (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel
)
{
  CPGC_CMD_CTL_STRUCT CPGC_CMD_CTL;

  //Read CPGC BCommand Sequence Control register
  CPGC_CMD_CTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_CTL_REG);
  //Set Stop Test (STOP_TEST) = "1" to Stop CPGC test
  CPGC_CMD_CTL.Bits.stop_test = 1;
  //Write final Value to CPGC BCommand Sequence Control register
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_CTL_REG, CPGC_CMD_CTL.Data);
  return CPGC_STS_SUCCESS;
}

/*********************************************************************************************************************
Function Name: CPGC_S_PollTest()
*********************************************************************************************************************
Date:  07/25/2012
*********************************************************************************************************************
Input:
    Channel:  Specified the channel number of CPGC
*********************************************************************************************************************
Example:
  CPGC_S_PollTest(Channel);
*********************************************************************************************************************
Return:
  CPGC_STS_TEST_DONE: If CPGC test is completed
  CPGC_STS_TEST_BUSY: If CPGC test is running
  CPGC_STS_TEST_ERROR: If CPGC test neither completed or running
*********************************************************************************************************************
Description:
This API will return the Status of current test that is running.
*********************************************************************************************************************
Modifications:
    Date      : Name   : Description
    07/25/2012:        : Initial Creation
********************************************************************************************************************/
static
STATUS
CPGC_S_PollTest (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel
)
{
  CPGC_CMD_TESTSTAT_STRUCT CPGC_CMD_TESTSTA;
  UINT8 Status;

  CPGC_CMD_TESTSTA.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CMD_TESTSTAT_REG);
  //Return test Status
  Status = (CPGC_CMD_TESTSTA.Bits.test_done == 1) ? CPGC_STS_TEST_DONE : (CPGC_CMD_TESTSTA.Bits.test_busy == 1) ? CPGC_STS_TEST_BUSY : CPGC_STS_TEST_ERROR;
  return Status;
}

//*********************************************************************************************************************
// Funtion Name: MrcDelay()                                                                                 *
//*********************************************************************************************************************
// Date:  06/21/2012                                                                                                  *
//*********************************************************************************************************************
// Input:                                                                                                             *
//    ModMrcData: Contains input Type ,Delay                                       *
//*********************************************************************************************************************
// Return:                                                                                                            *
//    Status:         Pass/Fail result                                                     *
//*********************************************************************************************************************
// Description: Delay will execute a Delay according to Type (milli, micro, nano)                                                                                            *
//*********************************************************************************************************************
STATUS MrcDelay (
  UINT8  Type,
  UINT32 Delay
)
{
  UINT32  TscStartLow;
  UINT32  TscStartHigh;
  UINT32  TscEndLow;
  UINT32  TscEndHigh;
  UINT32  TargetTickLow;
  UINT32  TargetTickHigh;
  UINT32  TscDiffLow;
  UINT32  TscDiffHigh;
  UINT32  CpuFrequency;
  UINT32  Temp;   // Divisor or multiplier

  Temp = 1000;

  //TSC Value is increasing base on 1/(CPU base frequency x CPU MAX ratio)
  CpuFrequency = MmrcCpuFrequencyRead();

  //Calculate the Delay in TSC tick Value
  if (Type == MILLI_DEL) {
#ifndef _MSC_EXTENSIONS
    asm (
		"movl %2,%%eax;"
		"mull %3;"
		"mull %4;"
		"movl %%eax, %0;"
		"movl %%edx, %1;"
		:"=m"(TargetTickLow),"=m"(TargetTickHigh)
        :"m"(CpuFrequency),"m"(Temp),"m"(Delay)
		:"%eax","%edx"
	);
#else
    _asm mov eax, CpuFrequency;
    _asm mul Temp;          //multiplier = 1000
    _asm mul Delay;
    _asm mov TargetTickLow, eax;
    _asm mov TargetTickHigh, edx;
#endif // VS_ENV
  } else if (Type == MICRO_DEL) {
#ifndef _MSC_EXTENSIONS
    asm (
		"movl %2,%%eax;"
		"mull %3;"
		"movl %%eax, %0;"
		"movl %%edx, %1;"
		:"=m"(TargetTickLow),"=m"(TargetTickHigh)
        :"m"(CpuFrequency),"m"(Delay)
		:"%eax","%edx"
    );
#else
    _asm mov eax, CpuFrequency;
    _asm mul Delay;
    _asm mov TargetTickLow, eax;
    _asm mov TargetTickHigh, edx;
#endif // VS_ENV
  } else if (Type == NANO_DEL) {
#ifndef _MSC_EXTENSIONS
    asm (
		"movl %1,%%eax;"
		"mull %2;"
		"divl %3;"
		"movl %%eax, %0;"
        :"=m"(TargetTickLow)
		:"m"(CpuFrequency),"m"(Delay),"m"(Temp)
		:"%eax","%edx"
    );
#else
    _asm mov eax, CpuFrequency;
    _asm mul Delay;
    _asm div Temp;          //divisor = 1000
    _asm mov TargetTickLow, eax;
#endif // VS_ENV
    TargetTickHigh = 0;
  }

#ifndef _MSC_EXTENSIONS
  asm (
		"rdtsc;"
		"movl %%eax, %0;"
		"movl %%edx, %1;"
		:"=m"(TscStartLow),"=m"(TscStartHigh)
        :
		:"%eax","edx"
  );
#else
  //read current TSC Value
  _asm rdtsc;
  _asm mov TscStartLow, eax;
  _asm mov TscStartHigh, edx;
#endif // VS_ENV

  do {
#ifndef _MSC_EXTENSIONS
    asm (
		"rdtsc;"
		"movl %%eax, %0;"
		"movl %%edx, %1;"
		:"=m"(TscEndLow),"=m"(TscEndHigh)
        :
		:"%eax","edx"
    );
#else
    //read current TSC Value
    _asm rdtsc;
    _asm mov TscEndLow, eax;
    _asm mov TscEndHigh, edx;
#endif // VS_ENV

    if (TscStartLow <= TscEndLow) {
      TscDiffLow = TscEndLow - TscStartLow;
      if (TscStartHigh <= TscEndHigh) {
        TscDiffHigh = TscEndHigh - TscStartHigh;
      } else {
        TscDiffHigh = TscStartHigh - TscEndHigh - 1;
        TscDiffLow = 0xffffffff - TscDiffLow;
      }
    } else {
      TscDiffLow = TscStartLow - TscEndLow;
      if (TscStartHigh < TscEndHigh) {
        TscDiffHigh = TscEndHigh - TscStartHigh - 1;
        TscDiffLow = 0xffffffff - TscDiffLow;
      } else {
        TscDiffHigh = TscStartHigh - TscEndHigh;
      }
    }
  }
  //Wait until target TSC Value is time out
  while ( (TargetTickLow >= TscDiffLow) || (TargetTickHigh > TscDiffHigh) );

  return SUCCESS;
}

void MmrcReadMsr (
  MSR_REG *MsrReg
)
{
  UINT32  DataHigh;
  UINT32  DataLow;
  UINT32  Index;

  Index = MsrReg->Index;

#ifndef _MSC_EXTENSIONS
  asm (
		"movl %2, %%ecx;"
		"rdmsr;"
		"movl %%edx, %0;"
		"movl %%eax, %1;"
		:"=m"(DataHigh),"=m"(DataLow)
		:"m"(Index)
		:"%eax","%ecx","edx"
  );
#else
  _asm mov ecx, Index
  _asm rdmsr
  _asm mov DataHigh, edx
  _asm mov DataLow, eax
#endif // VS_ENV

  MsrReg->DataHigh = DataHigh;
  MsrReg->DataLow = DataLow;

}

void MmrcWriteMsr (
  MSR_REG *MsrReg
)
{
  UINT32  DataHigh;
  UINT32  DataLow;
  UINT32  Index;

  Index = MsrReg->Index;
  DataHigh = MsrReg->DataHigh;
  DataLow = MsrReg->DataLow;
#ifndef _MSC_EXTENSIONS
  asm (
		"movl %0, %%ecx;"
		"movl %1, %%edx;"
		"movl %2, %%eax;"
		"wrmsr;"
        :
		:"m"(Index),"m"(DataHigh),"m"(DataLow)
		:"%eax","%ecx","edx"
  );
#else
  _asm mov ecx, Index
  _asm mov edx, DataHigh
  _asm mov eax, DataLow
  _asm wrmsr
#endif // VS_ENV
}

UINT8 MmrcCpuRatioRead (void){
	UINT32  CpuRatio;
	MSR_REG Msr;

	Msr.Index = MSR_CPU_MAX_RATIO;
	MmrcReadMsr (&Msr);
	//Mask MSR Value and shift Value to bit0
	CpuRatio = (Msr.DataLow & MSR_CPU_MAX_RATIO_MASK) >> MSR_CPU_MAX_RATIO_SB;
	return (UINT8)CpuRatio;
}

UINT32 MmrcCpuFrequencyRead (void){

	UINT32  CpuBaseFrequency;
	UINT8  	CpuRatio;
	UINT32	MsrValue;
	MSR_REG Msr;

	//Read CPU base frequency
	Msr.Index = MSR_CPU_BASE_FREQ;
	MmrcReadMsr (&Msr);
	//Mask MSR Value and shift Value to bit0
	MsrValue = (Msr.DataLow & MSR_CPU_BASE_FREQ_MASK) >> MSR_CPU_BASE_FREQ_SB;
	//CPU base frequency in MHz base on table
	CpuBaseFrequency = cpu_base_freq_mhz[MsrValue];

	CpuRatio = MmrcCpuRatioRead();

	return(CpuBaseFrequency * CpuRatio);
}

UINT32 MemFieldRead (
  MMRC_DATA *ModMrcData,
  UINT8                BoxType,
  UINT8                Instance,
  IOSF_REG             IosfRegister
)
{
  UINT32  Value;
  //
  // Read the entire 32-Bits of the register specified.
  //
  Value = MemRegRead (ModMrcData->MrcDebugMsgLevel, BoxType, Instance, IosfRegister.offset);

  Value &= IosfRegister.mask;

  Value >>= IosfRegister.sb;
  //
  // Return the Value.
  //
  return Value;
}

void MemFieldWrite (
  MMRC_DATA *ModMrcData,
  UINT8                BoxType,
  UINT8                Instance,
  IOSF_REG             IosfRegister,
  UINT32               Value
)
{
  UINT32  TempValue;    // Value passed in is the Value that goes to the Bits, this contains the entire register Value.

  if (IosfRegister.offset == 0xffff) {
    return;
  }

  //unitMsgBusAndThenOr(IosfRegister.offset, ~IosfRegister.mask,  (Value<<IosfRegister.sb));

  // Read the 32-bit register.
  TempValue = MemRegRead (ModMrcData->MrcDebugMsgLevel, BoxType, Instance, IosfRegister.offset);

  // Apply the mask (by inverting and doing AND)
  TempValue &= ~IosfRegister.mask;

  // Shift the inputted Value to the correct start location
  TempValue |=  (Value << IosfRegister.sb);

  // Write the register back.
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, BoxType, Instance, IosfRegister.offset, TempValue);

  return;
}

VOID
MmrcMemSet (
  VOID *DataPtr,
  UINT8 Data,
  UINT32 Size
)
{
  UINT32 i;
   volatile UINT8 *DataPointer = (UINT8*)DataPtr;
  for (i = 0; i < Size; i ++) {
    *DataPointer = Data;
    DataPointer++;
  }

}

STATIC
STATUS
CPGC_LoadCpgcPattern (
  IN  OUT   MMRC_DATA     *ModMrcData,
  IN        UINT8         Channel,
  IN        UINT32        VictimPattern,
  IN        UINT8         PatternIndex
)
{
  UINT32 AggressorPattern;     // Pattern to load on aggressor bits, based on index.
  UINT32 VictimBit;            // Bit position that will be loaded with Victim BIt.
  UINT32 DPATIndex;            // Loop to traverse through all DPAT fields.

  AggressorPattern = 0;
  VictimBit        = 0;
  //
  // In Pattern Victim/Aggressor pattern mode, we are loading a known ISI pattern into the victim bit and the
  // aggressor bits get either the inversion or the same pattern depending on the Even/Odd mode.
  //
  if (ModMrcData->PatternMode == PATTERN_VICAGG) {
    //
    // The PatternIndex is the identifier for mode (even/odd): last pattern is even, all others odd.
    // When in odd mode, the index is the bit to be tested.
    //
    if (PatternIndex == ODD_MODE_BITSHIFTS + EVEN_MODE_BITSHIFTS) {
      AggressorPattern = VictimPattern;
    } else {
      VictimBit = PatternIndex;
      AggressorPattern = ~VictimPattern;
    }
    //
    // In LFSR Victim/Aggressor mode, we are loading the victim bit with a random seen, and loading the
    // the aggressor with a different seed.  When in even mode, the same seed is used for victim and aggressor.
    //
  } else if (ModMrcData->PatternMode == LFSR_VICAGG) {
    //
    // The PatternIndex is the identifier for mode (even/odd): last pattern is even, all others odd.
    // When in odd mode, the index is the bit to be tested.
    //
    if (PatternIndex == ODD_MODE_BITSHIFTS + EVEN_MODE_BITSHIFTS) {
      VictimPattern = CPGC_LFSR_VICTIM_SEED;
      AggressorPattern = CPGC_LFSR_VICTIM_SEED;
    } else {
      VictimBit = PatternIndex;
      VictimPattern = CPGC_LFSR_VICTIM_SEED;
      AggressorPattern = CPGC_LFSR_AGRESSOR_SEED;
    }
  } else if (ModMrcData->PatternMode == LFSR) {
	  //
	  // Nothing to do for Victim/Aggressor as its total Random.
	  //
  } else {
    
    MRC_DEADLOOP ();
  }
  //
  // Load the Unisequencers, only 2 are required, where the first sequencer gets loaded with the victim
  // pattern and the second unisequencer gets loaded with the aggressor pattern.
  // When the mode is PATTERN_VICAGG, the Unisequencers need to be put in PBM mode, but when in LFSRx mode,
  // the unisequencers need to be put in LFSR mode.
  //
  if (ModMrcData->PatternMode == PATTERN_VICAGG) {
    CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_0, CPGC_UNISEQINDEX_PBM, VictimPattern, 0);
    CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_1, CPGC_UNISEQINDEX_PBM, AggressorPattern, 0);
  } else if (ModMrcData->PatternMode == LFSR_VICAGG) {
    CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_0, CPGC_UNISEQINDEX_LFSR, VictimPattern, 0);
    CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_1, CPGC_UNISEQINDEX_LFSR, AggressorPattern, 0);
  } else if (ModMrcData->PatternMode == LFSR) {
    CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_0, CPGC_UNISEQINDEX_LFSR, 0xF0F0F0F0, 0);
    CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_1, CPGC_UNISEQINDEX_LFSR, 0xCCCCCCCC, 0);
    CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_2, CPGC_UNISEQINDEX_LFSR, 0xAAAAAAAA, 0);
  }

  //
  // Load each of the pattern buffers, only one line is being used within the buffers, all others are loaded
  // with 0.  WHen the buffer is loaded with 0xcc, that bit will use uniseq 0 which is the victim Bit, when
  // the buffer is 0xaa, that bit will use uniseq 1 which is the aggressor bit.
  //
  if (ModMrcData->PatternMode == PATTERN_VICAGG || ModMrcData->PatternMode == LFSR_VICAGG) {
    for (DPATIndex = 0; DPATIndex < 16; DPATIndex++) {
	  if (DPATIndex == VictimBit) {
		CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT0, 0xAA, 0x00, 0x00, 0x00);
	  } else {
		CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT0, 0xCC, 0x00, 0x00, 0x00);
	  }
	}
  } else if (ModMrcData->PatternMode == LFSR) {
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT0, 0xAA, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT1, 0xCC, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT2, 0xF0, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT3, 0x33, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT4, 0x55, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT5, 0x0F, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT6, 0xC0, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT7, 0x03, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT8, 0xAA, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT9, 0xCC, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT10, 0xF0, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT11, 0x33, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT12, 0x55, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT13, 0x0F, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT14, 0xC0, 0x00, 0x00, 0x00);
    CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT15, 0x03, 0x00, 0x00, 0x00);
  }

  return SUCCESS;
}


STATUS CPGC_InitializeMemory (
		  IN MMRC_DATA   *ModMrcData,
		  IN UINT8       Channel)
{

	  UINT32 Address;           // PHysical address for the provided channel and rank.
//	  UINT32 CompareFlag;

	  //
	  // Determine the physical address from the provided channel and rank.
	  //
	  Address = (ModMrcData->Channel[Channel].TotalMem << (20 - 6)) - 1;

	  CpgcSInit(ModMrcData,Channel);

	  CPGC_S_SetupSubseq (ModMrcData, Channel,  CPGC_SUBSEQINDEX_0, CPGC_SUBSEQ_TYPE_WO, 0x0, Address, 0, 0, 0, 0, 0, 0, 0);
	  CPGC_S_SetupSeq (ModMrcData, Channel, CPGC_SUBSEQINDEX_0, CPGC_SUBSEQINDEX_0, 0x1, 0x1);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT0, 0xAA, 0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT1, 0xAA, 0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT2, 0xAA, 0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT3, 0xAA, 0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT4, 0xAA, 0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT5, 0xAA, 0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT6, 0xAA, 0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT7, 0xAA, 0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT8, 0xAA, 0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT9, 0xAA, 0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT10, 0xAA,0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT11, 0xAA,0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT12, 0xAA,0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT13, 0xAA,0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT14, 0xAA,0xAA, 0xAA, 0xAA);
	  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT15, 0xAA,0xAA, 0xAA, 0xAA);
	  CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_0, CPGC_UNISEQINDEX_LFSR, 0x00000000, 0);
	  CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_1, CPGC_UNISEQINDEX_LFSR, 0x00000000, 0);
	  CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_2, CPGC_UNISEQINDEX_LFSR, 0x00000000, 0);

	  CPGC_S_ClearErrors (ModMrcData, Channel);
      CPGC_S_StartTest (ModMrcData, Channel, 0);
	  while (CPGC_S_PollTest (ModMrcData, Channel) == CPGC_STS_TEST_BUSY);
	  CPGC_S_StopTest (ModMrcData, Channel);

	  CPGC_S_Disable (ModMrcData, Channel);

	  return SUCCESS;
}


STATUS
CPGC_Setup(
  IN MMRC_DATA   *ModMrcData,
  IN UINT8       Channel,
  IN UINT8       Rank
)
{
  UINT32 Address;           // PHysical address for the provided channel and rank.

  //
  // Enable the CPGC Engine, this disable any access to the RAMs except for the CPGC.
  //
  CPGC_S_Enable (ModMrcData, Channel);

  //
  // Determine the physical address from the provided channel and rank.
  //
  Address = GetAddress (ModMrcData, Channel, Rank) >> 6;

  //
  // Make sure the engine is in the stop state.
  //
  CPGC_S_StopTest (ModMrcData, Channel);

  //
  // Enable all errors for each bit of the DQ lines.
  //
  if (ModMrcData->EccEnabled) {
      CPGC_S_EnableErrors (ModMrcData, Channel,  0xff, 0xff, 0xffffffff, 0xffffffff, 0xff);
  } else {
      CPGC_S_EnableErrors (ModMrcData, Channel,  0xff, 0xff, 0xffffffff, 0xffffffff, 0x0 );
  }
  //
  // Make sure no errors are sitting in the error registers, by clearing the error status.
  //
  CPGC_S_ClearErrors (ModMrcData, Channel);

  //
  // Setup the first 2 subsequences to be write-only and read-only.
  //
  CPGC_S_SetupSubseq (ModMrcData, Channel,  CPGC_SUBSEQINDEX_0, CPGC_SUBSEQ_TYPE_WOREF, 0x1, Address, 0x1, 0, 0, 0, 0, 3, 0);
  CPGC_S_SetupSubseq (ModMrcData, Channel,  CPGC_SUBSEQINDEX_1, CPGC_SUBSEQ_TYPE_ROREF, 0x1, Address, 0x1, 0, 0, 0, 0, 3, 0);

  //
  // Setup the 3 subsequences.
  //
  CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_0, CPGC_UNISEQINDEX_LFSR, 0xF0F0F0F0, 0);
  CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_1, CPGC_UNISEQINDEX_LFSR, 0xCCCCCCCC, 0);
  CPGC_S_SetUnisequencer (ModMrcData, Channel, CPGC_UNISEQINDEX_2, CPGC_UNISEQINDEX_LFSR, 0xAAAAAAAA, 0);

  //
  // Setup the patterns for each of the 16-bits to a random index.
  //
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT0, 0xAA, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT1, 0xCC, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT2, 0xF0, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT3, 0x33, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT4, 0x55, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT5, 0x0F, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT6, 0xC0, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT7, 0x03, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT8, 0xAA, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT9, 0xCC, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT10, 0xF0, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT11, 0x33, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT12, 0x55, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT13, 0x0F, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT14, 0xC0, 0x00, 0x00, 0x00);
  CPGC_S_SetupPattern (ModMrcData, Channel, CPGC_DPAT15, 0x03, 0x00, 0x00, 0x00);
  CPGC_S_SetupPatternControl (ModMrcData, Channel);

  return SUCCESS;
}

STATUS
LPDDR3_JEDECInit_Reset (
                IN MMRC_DATA   *ModMrcData,
                IN UINT8 Channel
)
{
	UINT8 Rank;
	UINT32 regValue = 0;

    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    	// Skip to next populated rank
    	if (ModMrcData->Channel[Channel].RankEnabled[Rank] == 0 ) { continue; }
    	JedecCmd (ModMrcData, Channel, Rank, JEDEC_PRECHARGEALL, regValue);

		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_NOP, regValue );
		MrcDelay (NANO_DEL, 10000);                    //delay 10 us

		/* Issue MRW(RESET) Command */
		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_RESET, regValue);

		MrcDelay (NANO_DEL, 10000);
    }
    return SUCCESS;
}

STATUS
LPDDR3_JEDECInit (
                IN MMRC_DATA   *ModMrcData,
                IN UINT8 Channel
)
{
	UINT8 Rank;
	UINT32 regValue = 0;

    for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    	// Skip to next populated rank
    	if (ModMrcData->Channel[Channel].RankEnabled[Rank] == 0 ) { continue; }
    	JedecCmd (ModMrcData, Channel, Rank, JEDEC_PRECHARGEALL, regValue);

		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_NOP, regValue );
		MrcDelay (NANO_DEL, 10000);                    //delay 10 us

		/* Issue MRW(RESET) Command */
		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_RESET, regValue);

		/* Issue NOP for at least tINIT4 (1us) */
		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_NOP, regValue );
		MrcDelay (NANO_DEL, 10000);                    //delay 10 us

		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_POST_INIT_CAL, regValue);

		 /* Issue MRW( all MRs ) Command */
		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_MR2, 0x0);
		MrcDelay (NANO_DEL, 10000);                    //delay 10 us
		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_NOP, 0x0);

		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_MR1, regValue );
		MrcDelay (NANO_DEL, 10000);                    //delay 10 us

		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_NOP, regValue );

		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_MR3, regValue );
		MrcDelay (NANO_DEL, 10000);                    //delay 10 us

		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_NOP, regValue );

		JedecCmd (ModMrcData, Channel, Rank, MMRC_JEDEC_CMD_ODT, regValue );
     }

    return SUCCESS;
}

STATUS
CATrainingRestore (
  IN MMRC_DATA   *ModMrcData,
  IN UINT8        Channel
)
{
	UINT32 TempValue;            // Placeholder for the Get/Set values.

	PRINT_FUNCTION_INFO;

	GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL, CMD_GET_CACHE, &TempValue);
	GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL, CMD_SET_VAL_FC, &TempValue);

	GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_CLKCTL_DEL, CMD_GET_CACHE, &TempValue);
	GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_CLKCTL_DEL, CMD_SET_VAL_FC, &TempValue);

	return SUCCESS;
}

STATUS FirstFailCheck (
	IN UINT8 Channel,
	IN MMRC_DATA *ModMrcData,
	IN OUT UINT32 *StartValue,
	IN OUT UINT16* totalCount
)
{
	UINT8 count, loop;
	UINT32 TempValue, regValue;
	BOOLEAN pass = FALSE;
	INT8 	directionFlag = 1;
    UINT8 numPattern = 0;
  
    count = 0;

    for (loop = LOW; loop <= HIGH; loop++) {
		TempValue = (UINT32) *StartValue;
    	count = 0;
    	if (loop == HIGH) {
    		directionFlag = -1;
       	}

    	do {
			// need to change the fix value of 10, digital_halfclk

			for (numPattern = 0; numPattern < 17; numPattern++) {
					
	
			(*totalCount) += 1;
			regValue = MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10);
			regValue |= BIT8;
			MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10, regValue);

			//
			// Poll until BIT8 = 0
			//
			while (MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10) & BIT8)
			{
					MrcDelay (NANO_DEL, 15);                    //delay 10 us
			}

			if (MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10) & BIT16)
			{
				//
				// Fail case
				//
				TempValue = (UINT32) (TempValue + ( (HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)/6) * directionFlag) );

				GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL, CMD_SET_VAL_FC_UC, &TempValue);
				GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_CLKCTL_DEL, CMD_SET_VAL_FC_UC, &TempValue);
				count++;

          if (count < 6) {numPattern = 0;}
		    	if (loop == HIGH) {
		    		
		    	} else {
		    		
		    	}
			} else {
				pass = TRUE;
				*StartValue = TempValue;
				loop = 2;
				
			}

			LPDDR3_JEDECInit (ModMrcData, Channel);
			
			MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x90, CApattern0[numPattern]);
			MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x91, CApattern1[numPattern]);
			MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x92, CApattern2[numPattern]);
			MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x93, CApattern0[numPattern]);
			MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x94, CApattern1[numPattern]);
            MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x95, CApattern2[numPattern]);

            MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x96, CApattern0[numPattern]);
            MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x97, CApattern1[numPattern]);
            MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x98, CApattern2[numPattern]);
            MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x99, CApattern0[numPattern]);
            MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x9A, CApattern1[numPattern]);
            MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x9B, CApattern2[numPattern]);

		}  //for numPattern
		} while(!pass && (count < 6));
    }

	if (pass) return SUCCESS;
	else return FAILURE;

}

VOID
Cpgc_S_SetupCADB (
  IN  OUT   MMRC_DATA     *ModMrcData,
  IN        UINT8         Channel,
  IN        UINT8         Enable
  )
{
  CPGC_CAPAT_CTL_STRUCT       CPGC_CAPAT_CTL;
  CPGC_CAPAT_UNISEQ0_STRUCT   CPGC_CAPAT_UNISEQ0;
  CPGC_CAPAT_UNISEQ1_STRUCT   CPGC_CAPAT_UNISEQ1;
  CPGC_CAPAT_BUFA0_STRUCT     CPGC_CAPAT_BUFA0;
  CPGC_CAPAT_BUFA1_STRUCT     CPGC_CAPAT_BUFA1;
  CPGC_CAPAT_BUFA2_STRUCT     CPGC_CAPAT_BUFA2;
  CPGC_CAPAT_BUFA3_STRUCT     CPGC_CAPAT_BUFA3;

  CPGC_CAPAT_CTL.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CAPAT_CTL_REG);

  if (Enable) {
    CPGC_CAPAT_CTL.Bits.capat_mode = 2;
  } else  {
    CPGC_CAPAT_CTL.Bits.capat_mode = 0;
  }
  CPGC_CAPAT_CTL.Bits.uniseq1_mode = 2;
  CPGC_CAPAT_CTL.Bits.uniseq0_mode = 2;

  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CAPAT_CTL_REG, CPGC_CAPAT_CTL.Data);

  CPGC_CAPAT_UNISEQ0.Bits.pat_buf = 0x1234;
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CAPAT_UNISEQ0_REG, CPGC_CAPAT_UNISEQ0.Data);

  CPGC_CAPAT_UNISEQ1.Bits.pat_buf = 0x5378;
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CAPAT_UNISEQ1_REG, CPGC_CAPAT_UNISEQ1.Data);

  CPGC_CAPAT_BUFA0.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CAPAT_BUFA0_REG);
  CPGC_CAPAT_BUFA0.Bits.cmd = 0;
  CPGC_CAPAT_BUFA0.Bits.bank_addr = 0;
  CPGC_CAPAT_BUFA0.Bits.row_col_addr = 0;
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CAPAT_BUFA0_REG, CPGC_CAPAT_BUFA0.Data);

  CPGC_CAPAT_BUFA1.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CAPAT_BUFA1_REG);
  CPGC_CAPAT_BUFA1.Bits.cmd = 5;
  CPGC_CAPAT_BUFA1.Bits.bank_addr = 2;
  CPGC_CAPAT_BUFA1.Bits.row_col_addr = 0xaaaa;
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CAPAT_BUFA1_REG, CPGC_CAPAT_BUFA1.Data);

  CPGC_CAPAT_BUFA2.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CAPAT_BUFA2_REG);
  CPGC_CAPAT_BUFA2.Bits.cmd = 2;
  CPGC_CAPAT_BUFA2.Bits.bank_addr = 5;
  CPGC_CAPAT_BUFA2.Bits.row_col_addr = 0x5555;
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CAPAT_BUFA2_REG, CPGC_CAPAT_BUFA2.Data);

  CPGC_CAPAT_BUFA3.Data = MemRegRead (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CAPAT_BUFA3_REG);
  CPGC_CAPAT_BUFA3.Bits.cmd = 7;
  CPGC_CAPAT_BUFA3.Bits.bank_addr = 7;
  CPGC_CAPAT_BUFA3.Bits.row_col_addr = 0xffff;
  MemRegWrite (ModMrcData->MrcDebugMsgLevel, CPGC, Channel, CPGC_CAPAT_BUFA3_REG, CPGC_CAPAT_BUFA3.Data);


}

STATUS
CpgcExecuteTest (
  IN OUT    MMRC_DATA     *ModMrcData,
  IN        UINT8          Channel,
  OUT       UINT32        *CompareFlag
)
/*++

Routine Description:
 
  CPGC routines to begin the CPGC Test, poll for the completion, after the test is done return the the status
  of the test which is a bytelane+ecc bit-test.

Arguments:

  ModMrcData:       Host structure for all data related to MMRC
  Channel:          Current Channel being examined.
  CompareFlag:      Bytelane + ECC pass/failure.

 
Returns:

  Success
  Failure

--*/
{
  CPGC_S_ClearErrors (ModMrcData, Channel);
  CPGC_S_StartTest (ModMrcData, Channel, 0);
  while (CPGC_S_PollTest (ModMrcData, Channel) == CPGC_STS_TEST_BUSY);
  MrcDelay (MILLI_DEL, 10);
  CPGC_S_StopTest (ModMrcData, Channel);
  CPGC_S_CheckErrors (ModMrcData, Channel, (UINT32*) NULL, (UINT32*) NULL, CompareFlag, (UINT8*) NULL);
  return SUCCESS;
}

STATUS
LPDDR3_LateCATraining (
  IN MMRC_DATA   *ModMrcData,
  IN UINT8        Channel
)
/*++

Routine Description:
 
  Late Command Training.  This will margin the Cmd knobs for all the cmd groups tethered together.  It will
  start at the nominial (default) value that was calculated from Early Cmd Training and margin to failure.  When
  a failure occurs, a Jedec reset will occur, and the system will margin the higher side.  This is done for all ranks 
  on the given channel.

Arguments:

  ModMrcData:       Host structure for all data related to MMRC
  Channel:          Current Channel being examined.
 
Returns:

  Success
  Fail

--*/
{
  UINT32 TempValue;                // Placeholder for the Get/Set values.
  UINT8  CmdLoop;                  // Loop through the commands
  UINT32 CmdValue[MAX_CMDS];       // Default values for the linear commands
  UINT8  Rank;                     // Current Rank being operated on.
  UINT32 CmdLower[MAX_CMDS];       // Lower range of the swept range.
  UINT32 CmdHigher[MAX_CMDS];      // Higher range of th swept range.
  UINT32 CmdSweepValue[MAX_CMDS];
  UINT32 LowHigh;
  UINT8  CurrentStatus;
  UINT32 CompareFlag;

#if defined PERFORMANCE && PERFOMRANCE
  UINT32 TscStartLow;
  UINT32 TscStartHigh;
  UINT32 TscStopLow;
  UINT32 TscStopHigh;

  _asm rdtsc;
  _asm mov TscStartLow, eax;
  _asm mov TscStartHigh, edx;
#endif                       


  //
  // Read the Initial Command values for each group storing them in the CmdValue array for each group.
  //
  for (CmdLoop=0; CmdLoop < MAX_CMDS; CmdLoop++ ) {
  	GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL + CmdLoop, CMD_GET_CACHE, &CmdValue[CmdLoop]);
  }

  // Loop through all ranks re-running the same procedure with CPGC testing the specific ranks.
  //
  for (Rank = 0; Rank < MAX_RANKS; Rank++) {
    if (ModMrcData->Channel[Channel].RankEnabled[Rank]) {
      //
      // Issue a precharge all.
      //
      JedecCmd (ModMrcData, Channel, Rank, (UINT8) JEDEC_PRECHARGEALL, (UINT32) 0xffffffff);
      //
      // Initialization
      //
      for (CmdLoop = 0; CmdLoop < MAX_CMDS; CmdLoop++) {
        CmdLower[CmdLoop]       = 0;
        CmdHigher[CmdLoop]      = 0;
        CmdSweepValue[CmdLoop]  = 0;
      }

      CpgcSetupForCMD(ModMrcData, Channel, Rank, 0x6, 0x4);
	  
      //
      // Loop through the high and low range, startin with the low.
      //
      for (LowHigh = 0; LowHigh < 2; LowHigh ++) {
        //
        // Set and copy the swept values in the lower/higher ranges.
        //
        for (TempValue=0; TempValue < MAX_CMDS; TempValue++) {
          if (LowHigh == 0) {
            CmdLower[TempValue]      = CmdValue[TempValue] - LATECMD_STEPSIZE;
            CmdSweepValue[TempValue] = CmdLower[TempValue];
          } else {
            CmdHigher[TempValue]     = CmdValue[TempValue] + LATECMD_STEPSIZE;
            CmdSweepValue[TempValue] = CmdHigher[TempValue];
          }
        }

        CurrentStatus = LOOKING_FOR_FAILURE;
        //
        // Continue looping through each delay until a FAILURE is detected.
        //
        while (CurrentStatus != FOUND_FAILURE) {
          //
          // Set all Cmd values to the specific value.
          //
          for (TempValue = 0; TempValue < MAX_CMDS; TempValue++) {
	          GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL + (UINT8) TempValue, CMD_SET_VAL_FC_UC, &CmdSweepValue[TempValue]);
          }
          //
          // Run CPGC Test.
          //
          CpgcExecuteTest (ModMrcData, Channel, &CompareFlag);

          //CompareFlag = 0 = CPGC Pass
          if (CompareFlag == 0) {
            for (TempValue = 0; TempValue < MAX_CMDS; TempValue++) {
              if (LowHigh == 0) {
                CmdLower[TempValue] = CmdSweepValue[TempValue];
                CmdSweepValue[TempValue] -= LATECMD_STEPSIZE;          
                if(CmdValue[TempValue]-CmdLower[TempValue] > LATECMD_SWEEP_RANGE -1){
                	CurrentStatus = FOUND_FAILURE;
                }
              } else {
                CmdHigher[TempValue] = CmdSweepValue[TempValue];
                CmdSweepValue[TempValue] += LATECMD_STEPSIZE;          
                if(CmdHigher[TempValue] - CmdValue[TempValue]> LATECMD_SWEEP_RANGE -1){
                	CurrentStatus = FOUND_FAILURE;
                }
              }
            }
          } else {
            CurrentStatus = FOUND_FAILURE;
            for (TempValue=0; TempValue < MAX_CMDS; TempValue++) {
	            GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL + (UINT8) TempValue, CMD_SET_VAL_FC, &CmdValue[TempValue]);
            }

            //
            // Perform a FIFO Reset.
            //
            PerformFifoReset(ModMrcData, Channel,Rank);

            MrcDelay (NANO_DEL, 15);                    //delay 10 us
            LPDDR3_JEDECInit (ModMrcData, Channel);
          }
        }
      }
      //
        for (TempValue = 0; TempValue < MAX_CMDS; TempValue++) {
          if ((CmdHigher[TempValue]-CmdValue[TempValue]) <=
             (UINT32) (HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)*UPM_PERCENT_LATECMD/100) ||
              (CmdValue[TempValue]-CmdLower[TempValue]) <=
              (UINT32) (HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)*UPM_PERCENT_LATECMD/100)) {
                MRC_DEADLOOP ();
          }
          CmdValue[TempValue ] = (CmdHigher[TempValue] + CmdLower[TempValue]) / 2;
          GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL + (UINT8) TempValue, CMD_SET_VAL_FC_UC, &CmdValue[TempValue]);
//          PrintCommandLowHigh(ModMrcData->MrcDebugMsgLevel, CMDLOWHIGH_BOTH, CmdLower[TempValue], CmdHigher[TempValue], CmdValue[TempValue]);
        }

    }
	Rank = MAX_RANKS;
  }

  //
  // Disable CPGC so that cpu access to memory is enabled.
  //
#if CPGC_API
    CPGC_S_Disable (ModMrcData, Channel);
#endif

#if defined PERFORMANCE && PERFORMANCE
  //
  // Read the TSC at the end of the function
  //
  _asm rdtsc;                                                                              
  _asm mov TscStopLow, eax;                                                                
  _asm mov TscStopHigh, edx;                                                               
  PerformanceAnalyze(ModMrcData, Channel, Rank, TscStartHigh, TscStartLow, TscStopHigh, TscStopLow);
#endif                                                                                     
	return SUCCESS;
}

STATUS
LPDDR3_CATraining (
		IN MMRC_DATA   *ModMrcData,
		IN UINT8 Channel
  )
{
	UINT32 	regValue, TempValue;
	UINT32 	cmdValue = 0;
	UINT8 	Strobe, Rank;
	UINT16	PfLimits[2];
	UINT8 	DoneFlag = 0;
	UINT16 	count = 0;
	UINT16 remain=0;
	UINT8  	lowHighFlag;
	//BOOLEAN	FirstFail = FALSE;
    UINT8 numPattern = 0;
//  UINT8 loopPattern = 0;
	Strobe = 0;
	Rank = 0;		//no Rank for cmd
    count = 0;
	//
	// HIP Entry
	//
	DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_catrain_list[0]);

	//Read DUNIT DRAM register
	regValue = MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10);
	regValue |= ( BIT14|BIT15);
	MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10, regValue);

    if( (ModMrcData->SiRevisionID == 1) || (ModMrcData->SiRevisionID == 2) ||(ModMrcData->SiRevisionID == 3) ||(ModMrcData->SiRevisionID == 4)){
		
		//Program DCO[29]=1
		regValue = MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0xF);
		regValue |= ( BIT29);
		MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0xF, regValue);

		regValue = 0xFF00FF00;
	}


	MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x90, 0xFFEEFFEE);
	MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x91, 0xFF00FF00);
	MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x92, 0x00FF00FF);
	MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x93, 0xFFEEFFEE);
	MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x94, 0xFF00FF00);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x95, 0x00FF00FF);

    MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x96, 0xF0F0F0F0);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x97, 0xAAAAAAAA);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x98, 0x0F0F0F0F);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x99, 0xF0F0F0F0);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x9A, 0xAAAAAAAA);
    MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x9B, 0x0F0F0F0F);


    //precharge
    JedecCmd (ModMrcData, Channel, Rank, (UINT8) JEDEC_PRECHARGEALL, (UINT32) 0xffffffff);

	// 3.0 Training
	//
	// Get the CMD delay
	//
	GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL, CMD_GET_REG, &cmdValue);

	if (FirstFailCheck (Channel, ModMrcData, &cmdValue, &count) == FAILURE) {
		return FAILURE;
	}

    PfLimits[LOW]  = (UINT16) cmdValue;
	for (lowHighFlag = LOW; lowHighFlag <= HIGH; lowHighFlag++) {

		DoneFlag = 0;
		PfLimits[HIGH] = (UINT16) cmdValue;
		do {

				for (numPattern = 0; numPattern < 17; numPattern++) {

		        TempValue = (UINT32) PfLimits[lowHighFlag];
	    		GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL, CMD_SET_VAL_FC_UC, &TempValue);
	    		GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_CLKCTL_DEL, CMD_SET_VAL_FC_UC, &TempValue);
	    		count++;
				regValue = MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10);
				regValue |= BIT8;
				MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10, regValue);

				//
				// Poll until BIT8 = 0
				//
				while (MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10) & BIT8){
                    MrcDelay (NANO_DEL, 15);                    //delay 10 us
				}

				if (MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10) & BIT16)
				{
					// Fail
                    

                    /* comment out for PO
                    if (count == 1)
                    {
                    	if (FirstFailCheck (Channel, ModMrcData, &cmdValue, &count) == FAILURE) {
                    		return FAILURE;
                    	}
                    	FirstFail = TRUE;
                    	PfLimits[lowHighFlag] = (UINT16)cmdValue;
                    } */

                   	TempValue = (UINT32) cmdValue;

                	GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL, CMD_SET_VAL_FC_UC, &TempValue);
                	GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_CLKCTL_DEL, CMD_SET_VAL_FC_UC, &TempValue);

                    MrcDelay (NANO_DEL, 15);                    //delay 10 us

                    LPDDR3_JEDECInit_Reset (ModMrcData, Channel);

				    /*if (FirstFail) {
				    	DoneFlag = 0;
				    	FirstFail = FALSE;
				    } */
				    //else {
				    	DoneFlag = 1;
				    	numPattern = 17;
				    //}

				} else {

					// PASS case
					if (lowHighFlag == LOW) {
						if (PfLimits[LOW] < (cmdValue + HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)) ) {
							  if (numPattern == 16) {
							    PfLimits[LOW] += 1;
							  }
						} else {
							DoneFlag = 1;
							
						}

					} else {
						if (PfLimits[HIGH] > (cmdValue - HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency)) ) {
							if (numPattern == 16) {
							    PfLimits[ HIGH] -= 1;
							  }
						} else {
                            DoneFlag = 1;
                            
						}
					}
				}
		      MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x90, CApattern0[numPattern]);
	          MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x91, CApattern1[numPattern]);
	          MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x92, CApattern2[numPattern]);
	          MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x93, CApattern0[numPattern]);
	          MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x94, CApattern1[numPattern]);
              MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x95, CApattern2[numPattern]);

              MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x96, CApattern0[numPattern]);
              MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x97, CApattern1[numPattern]);
              MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x98, CApattern2[numPattern]);
              MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x99, CApattern0[numPattern]);
              MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x9A, CApattern1[numPattern]);
              MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x9B, CApattern2[numPattern]);

			} //for numpattern
	    } while ( !DoneFlag );

	}

    if( (ModMrcData->SiRevisionID == 1) || (ModMrcData->SiRevisionID == 2) ||(ModMrcData->SiRevisionID == 3) ||(ModMrcData->SiRevisionID == 4)){
		//silicon bug, do cat multiple of 4 times
		if (count%4) {
			for (remain = 0; remain <(4 - ( count %4 )); remain++) {
				regValue = MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10);
				regValue |= BIT8;
				MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10, regValue);

				
				//
				// Poll until BIT8 = 0
				//
				while (MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0x10) & BIT8)
				{
                    MrcDelay (NANO_DEL, 15);                    //delay 10 us
                 }
			}
		}
	}

	TempValue = (PfLimits[LOW] + PfLimits[ HIGH] ) /2;
	
	
	GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_DEL, CMD_SET_VAL_FC_UC, &TempValue);
    GetSetDataSignal (ModMrcData, 0, Channel, 0, 0, 0, 0, 0, DELAYS_CMD_CLKCTL_DEL, CMD_SET_VAL_FC_UC, &TempValue);

    if( (ModMrcData->SiRevisionID == 1) || (ModMrcData->SiRevisionID == 2) ||(ModMrcData->SiRevisionID == 3) ||(ModMrcData->SiRevisionID == 4)){
    	//Program DCO[29]=0
		regValue = MemRegRead (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0xF);
		regValue &= (~BIT29);
		MemRegWrite (ModMrcData->MrcDebugMsgLevel, DUNIT, Channel, 0xF, regValue);
	}
	//
	// HIP Entry
	//
	DecodeAndExeRegAssignment (ModMrcData, Channel, HIP_catrain_list[1]);

	return SUCCESS;
}

STATUS PerformanceSetting(MMRC_DATA *ModMrcData, UINT8 Channel)
{
	UINT8 Rank;
	UINT8 Strobe;
	UINT32 RcvnLinearValue;
	UINT32 MaxRcvn;
	UINT32 MinRcvn;
	UINT32 TempValue;
	UINT32	Denominator;
	UINT32	tRDValue[MAX_STROBES];
	UINT32	MaxtRDValue;
	UINT32 OdtDiffampDly[MAX_STROBES];
	UINT32 OdtDiffampLen[MAX_STROBES];
    UINT8  TotalBL;
	PRINT_FUNCTION_INFO;

	//Perf enable
	MMRCDDRStaticInitPerfEnable(ModMrcData, Channel);

	if ((ModMrcData->Channel[Channel].RankEnabled[0] == 1) && (ModMrcData->Channel[Channel].RankEnabled[1] == 0)){
		if (ModMrcData->CurrentDdrType == TYPE_LPDDR3 && ModMrcData->CurrentFrequency == FREQ_1333){
			MMRCDDRStaticInitPerfEnableLP31333(ModMrcData, Channel);
		}
    }

	//Temp set to 0
	ModMrcData->FeatureSettings.MrcDynamicDDRPhySettings = 0;
	//
	// Compute the maximum Receive Enable on each Rank.
	//
	Denominator = 2 *HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);

	//Clear MaxtRDValue, MaxRcvn and MinRcvn to 0
	MaxtRDValue = 0;
	MaxRcvn = 0;
	MinRcvn = 0;

	for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
		//
		// For each strobe, restore the Rcvn value and the minus1 select.
		//
		for (Rank = 0; Rank < MAX_RANKS; Rank++) {
			if (ModMrcData->Channel[Channel].RankEnabled[Rank]) {
				//RCVN 2X + PI
				GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, (UINT8)DELAYS_RCVN_DEL, CMD_GET_REG, &TempValue);
				RcvnLinearValue = TempValue;
				//RCVN 1X
				if (GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, (UINT8)RCVN_ONEX, CMD_GET_REG, &TempValue) == SUCCESS) {
					RcvnLinearValue += TempValue * HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency) *2;
				} else {
					//Print error handling message
				}
				//RCVN MIN1
	        	if (GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, RCVN_MIN, CMD_GET_REG, &TempValue)== SUCCESS) {
	        		if (TempValue == 1) {
	        			RcvnLinearValue += HALF_CLK_(ModMrcData->FeatureSettings.MrcDigitalDll, ModMrcData->CurrentFrequency);
	        		}
	        	}else {
					//Print error handling message
				}

	        	//Get the MAX
				if (RcvnLinearValue > MaxRcvn) {
					MaxRcvn = RcvnLinearValue;
				}
				//Get the MIN
				if (MinRcvn == 0){
					MinRcvn = RcvnLinearValue;
				} else if (RcvnLinearValue < MinRcvn){
					MinRcvn = RcvnLinearValue;
				}
			}
		}
	}

	for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
		//tRD Formula:
		if (ModMrcData->CurrentDdrType == TYPE_LPDDR3){
			//LPDDR3
			tRDValue[Strobe] = (MaxRcvn/Denominator) + 5 + ModMrcData->FeatureSettings.MrcDynamicDDRPhySettings;
		} else {
			//DDR3L
			tRDValue[Strobe] = (MaxRcvn/Denominator) + 5 + ModMrcData->FeatureSettings.MrcDynamicDDRPhySettings;
		}

		if((MaxRcvn% Denominator) > (Denominator/2) ){
			tRDValue[Strobe] += 1;
		}

    	//Keep the Max tRD Value
		if (tRDValue[Strobe] > MaxtRDValue) {
			MaxtRDValue = tRDValue[Strobe];
		}

		//MCHODT/Diffampen Launch Formula:
		//2 is because we are pulling back from RCVEN value
		OdtDiffampDly[Strobe] = (MinRcvn/Denominator) - 2;
		if((MinRcvn% Denominator) > (Denominator/2) ){
			OdtDiffampDly[Strobe] += 1;
		}

		OdtDiffampDly[Strobe] += ModMrcData->FeatureSettings.MrcDynamicDDRPhySettings;

		//MCHODT/Diffampen Length Formula:
		if (ModMrcData->CurrentDdrType == TYPE_LPDDR3){
			//2 is because we are pulling back from RCVEN value
			//5 is to cover the entire Read burst (BL8)
			//(2 + 5 + "2") extra "2" is for LPDDR3 tDQSCK variation of 2.3ns
			//(2 + 5 + 2 + "1") extra "1" is for guardband
			OdtDiffampLen[Strobe] = 2 + 5 + 2 + 1 + ModMrcData->FeatureSettings.MrcDynamicDDRPhySettings;
		} else {
			//OdtDiffampLen[Strobe] = (UINT32)(floor( (MaxRcvn[Strobe] - MinRcvn[Strobe])/Denominator));
			OdtDiffampLen[Strobe] = (UINT32)(( (MaxRcvn - MinRcvn)/Denominator));
			if(( (MaxRcvn - MinRcvn)% Denominator) > (Denominator/2) ){
				OdtDiffampLen[Strobe] += 1;
			}
			//2 is because we are pulling back from RCVEN value
			//5 is to cover the entire Read burst (BL8)
			//1 is for guardband
			OdtDiffampLen[Strobe] = OdtDiffampLen[Strobe] + 2 + 5 + 1 + ModMrcData->FeatureSettings.MrcDynamicDDRPhySettings;
		}
	}

	// Rank is Don't care as the formula signals are per channel and strobe
	Rank = 0;

	if (ModMrcData->MaxDq <= 4) {
		TotalBL = 8;
	} else {
		TotalBL = ModMrcData->MaxDq;
	}

	for (Strobe = 0; Strobe < TotalBL; Strobe++) {
		//Program Max tRD value to all strobes
		GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, (UINT8)TRD, CMD_SET_VAL_FC, &MaxtRDValue);
		//Perform a FIFO Reset after programmed tRD
		TempValue = FIFO_RESET_ENABLE;
		GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, FIFORESET, CMD_SET_VAL_FC, &TempValue);
		TempValue = FIFO_RESET_DISABLE;
		GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, FIFORESET, CMD_SET_VAL_FC, &TempValue);
	}

	for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
		GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, (UINT8)DIFFAMP_DELAY, CMD_SET_VAL_FC, &OdtDiffampDly[Strobe]);
	}

	for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
		GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, (UINT8)DIFFAMP_LENGTH, CMD_SET_VAL_FC, &OdtDiffampLen[Strobe]);
	}

	for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
		GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, (UINT8)MCHODT_DELAY, CMD_SET_VAL_FC, &OdtDiffampDly[Strobe]);
	}

	for (Strobe = 0; Strobe < ModMrcData->MaxDq; Strobe++) {
		GetSetDataSignal (ModMrcData, 0, Channel, 0, Rank, Strobe, 0, 0, (UINT8)MCHODT_LENGTH, CMD_SET_VAL_FC, &OdtDiffampLen[Strobe]);
	}
	return SUCCESS;
}

