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

  MrcFunc.c

Abstract:

  Utility MRC function for Valleyview.

--*/

#include "McFunc.h"
#include "MrcFunc.h"
#include "ConfigMem.h"
#include "OemHooks.h"
#include "IoAccess.h"
#ifndef ECP_FLAG
#include <Library/BaseMemoryLib.h>
#endif

#ifdef __GNUC__
unsigned char _BitScanForward(UINT32 *Index, unsigned long Mask)
{
	__asm__ (
	"bsfl %0, %0;"
	:"=a"(*Index)
	:"0" (Mask)
	:
	);

	return (unsigned char)(0 != Mask);
}
#else
#pragma intrinsic(_BitScanForward)
#endif
UINT8
BitScanForward8 (
  UINT8                   Input
  )
/*++

Routine Description:

  Returns the position of the least significant bit set in the input parameter.
  If the input value is zero, the output is undefined.
Arguments:

  Input:    The value to be scanned

Returns:

  UINT8:    bit position.

--*/
{
   UINT32 Index;
    _BitScanForward(&Index, Input);
   return (UINT8) Index;
}

#ifdef __GNUC__
unsigned char _BitScanReverse(UINT32 *Index, unsigned long Mask)
{
	__asm__ (
	"bsrl %0, %0;"
	:"=a"(*Index)
	:"0" (Mask)
	:
	);

	return (unsigned char)(0 != Mask);
}
#else
#pragma intrinsic(_BitScanReverse)
#endif

UINT8
BitScanReverse8 (
  UINT8                   Input
  )
/*++

Routine Description:

  Returns the position of the most significant bit set in the input parameter.
  If the input value is zero, the output is undefined.
Arguments:

  Input:    The value to be scanned

Returns:

  UINT8:    bit position.

--*/
{
  UINT32 Index;
  _BitScanReverse(&Index, Input);
  return (UINT8) Index;
  
}

STATUS
FillInputStructure (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
    UINT8                      CurrentSocket;
    MEMINIT_SPD_DATA           *CurrentSpdData;
    UINT8 i; 
    UINT8 *pData8;

    //change according to the channel present, VLV: read from fuse.
    CurrentMrcData->Channel[0].Enabled = 1;

  // Fill MRC_PARAMETER_FRAME.MRC_PARAMS_SAVE_RESTORE
  if (CurrentMrcData->BootMode != S5Path) {
    if (CurrentMrcData->OemMrcData.MrcParamsValidFlag) {
#ifdef ECP_FLAG      
      memcpy (CurrentMrcData, &CurrentMrcData->OemMrcData, sizeof(MRC_PARAMS_SAVE_RESTORE));
#else
      CopyMem (CurrentMrcData, &CurrentMrcData->OemMrcData, sizeof(MRC_PARAMS_SAVE_RESTORE));
#endif
    } else {
      IoOut8(0xCF9, 0xE);
      //return FAILURE;
    }
  }
	else {
  	//copy even for cold boot and warm reset.
		memcpy (&(CurrentMrcData->ScramblerSeed), &(CurrentMrcData->OemMrcData.ScramblerSeed), sizeof(CurrentMrcData->ScramblerSeed));

		if (CurrentMrcData->DDRType < DDRType_DDR3All){
        for (Channel = 0; Channel < MAX_CHANNELS; Channel++) {

            for (CurrentSocket = 0; CurrentSocket < MAX_SLOTS; CurrentSocket++){

                CurrentSpdData = &(CurrentMrcData->SpdData[(Channel*MAX_SLOTS+CurrentSocket)]);
                pData8 = (UINT8*)(&(CurrentMrcData->Channel[Channel].FastBootData[CurrentSocket]));

                for (i = 0; i < 9; i++) {
                     *(pData8+i) = CurrentSpdData->Buffer[SPD_DDR3_MANUFACTURER_ID_LO+i] ;
            	}
                *(pData8+9) = CurrentMrcData->TotalDimm[Channel];

            } //end of for socket
        }  //end of channel
		}
    }

   if ((CurrentMrcData->Channel[0].Enabled == 1) && (CurrentMrcData->Channel[1].Enabled == 1)) {
		CurrentMrcData->DualChannelEnable = 1;
		
   } else {
		CurrentMrcData->DualChannelEnable = 0;
   }
		
	if (CurrentMrcData->DualChannelEnable) {  //dual
		CurrentMrcData->DunitMsgBus32Write = MsgBus32WriteDualDunit;
		CurrentMrcData->DunitMsgBus32Read = MsgBus32Read;
		CurrentMrcData->DunitInitCommand = DramInitCommandDualDunit;
		CurrentMrcData->DunitWakeCommand = DramWakeCommandDualDunit;
	} else {
		CurrentMrcData->DunitMsgBus32Write = MsgBus32Write;
		CurrentMrcData->DunitMsgBus32Read = MsgBus32Read;
		CurrentMrcData->DunitInitCommand = DramInitCommand;
		CurrentMrcData->DunitWakeCommand = DramWakeCommand;
	} 
    return SUCCESS;
}

STATUS
FillOutputStructure (
  MRC_PARAMETER_FRAME   *CurrentMrcData, UINT8 Channel
  )
{
  // Fill MRC_OEM_FRAME.MRC_PARAMS_SAVE_RESTORE
#ifdef ECP_FLAG      
  memcpy (&CurrentMrcData->OemMrcData, CurrentMrcData, sizeof(MRC_PARAMS_SAVE_RESTORE));
#else
  CopyMem (&CurrentMrcData->OemMrcData, CurrentMrcData, sizeof(MRC_PARAMS_SAVE_RESTORE));
#endif  
  
  return SUCCESS;
}
