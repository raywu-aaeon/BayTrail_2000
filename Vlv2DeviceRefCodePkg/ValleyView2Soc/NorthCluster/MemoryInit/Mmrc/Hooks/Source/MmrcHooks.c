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

  MmrcHooks.h

Abstract:

  Modular MRC calls functions in this file which perform tasks
  which are outside the realm of DDRIO initialization. Things
  like print routines, DUNIT routines which the MMRC needs,
  JEDEC init, etc, will be placed here.

--*/
#include "../Include/MmrcHooks.h"
#include "../../IpBlocks/VLVA0/Include/MMRCProjectLibraries.h"
#include "IoAccess.h"

#ifndef MRS_DEFINED
VOID
DramInitCommand (
  IN          UINT8                 MrcDebugMsgLevel,
  IN          UINT8                 Channel,
  IN          UINT32                Data
)
/*++

Routine Description:

  Sends a init command to DDR memory


Arguments:

  MrcDebugMsgLevel: Message level at which the current MRC run is operating
  Channel:          Current Channel being examined.
  Data:             Value to be sent

Returns:

  None

--*/
{
  UINT8 PortId;
  PortId = 0x01;
  
  Mmio32Write (EC_BASE + 0xD8, 0);
  Mmio32Write (EC_BASE + 0xD4, Data);
  Mmio32Write (EC_BASE + 0xD0, ( (CMD_DRAM_INIT) | (PortId << 16) | (0xF0) ) );
}


VOID
DramWakeCommand (
  IN          UINT8                 MrcDebugMsgLevel,
  IN          UINT8                 Channel
)
/*++

Routine Description:

  Sends a wake command to DDR memory


Arguments:

  MrcDebugMsgLevel: Message level at which the current MRC run is operating
  Channel:          Current Channel being examined.

Returns:

  None

--*/
{
  UINT8 PortId;
  PortId = 0x01;
  
  Mmio32Write (EC_BASE + 0xD0, ( (CMD_WAKE) | (PortId << 16) | (0xF0) ) );
}

VOID
DramSuspendCommand (
  IN          UINT8                MrcDebugMsgLevel,
  IN          UINT8                Channel
)
/*++

Routine Description:

  Sends a suspend command to DDR memory


Arguments:

  MrcDebugMsgLevel: Message level at which the current MRC run is operating
  Channel:          Current Channel being examined.

Returns:

  None

--*/
{
  UINT8 PortId;
  PortId = 0x01;
  
  Mmio32Write (EC_BASE + 0xD0, ( (CMD_SUSPEND) | (PortId << 16) | (0xF0) ) );
}
#endif

STATUS
ReceiveEnableEntryHooks (
  MMRC_DATA   *ModMrcData,
  UINT8        Channel
)
{
  PRINT_FUNCTION_INFO_MAX;
  return SUCCESS;
}

STATUS
ReceiveEnableExitHooks (
  MMRC_DATA *ModMrcData,
  UINT8        Channel
)
{
  PRINT_FUNCTION_INFO_MAX;
  return SUCCESS;
}

STATUS
ReadVrefEntryHooks (
  MMRC_DATA *ModMrcData
)
{
  PRINT_FUNCTION_INFO_MAX;
  return SUCCESS;
}

STATUS
ReadVrefExitHooks (
  MMRC_DATA *ModMrcData
)
{
  PRINT_FUNCTION_INFO_MAX;
  return SUCCESS;
}

STATUS
FineWriteLevelingEntryHooks (
  MMRC_DATA *ModMrcData
)
{
  PRINT_FUNCTION_INFO_MAX;
  return SUCCESS;
}

STATUS
FineWriteLevelingExitHooks (
  MMRC_DATA *ModMrcData
)
{
  PRINT_FUNCTION_INFO_MAX;
  return SUCCESS;
}

STATUS
CoarseWriteLevelingEntryHooks (
  MMRC_DATA *ModMrcData
)
{
  PRINT_FUNCTION_INFO_MAX;
  return SUCCESS;
}

STATUS
CoarseWriteLevelingExitHooks (
  MMRC_DATA *ModMrcData
)
{
  PRINT_FUNCTION_INFO_MAX;
  return SUCCESS;
}

STATUS
ReadTrainingEntryHooks (
  MMRC_DATA *ModMrcData
)
{
  PRINT_FUNCTION_INFO_MAX;
  return SUCCESS;
}

STATUS
ReadTrainingExitHooks (
  MMRC_DATA   *ModMrcData
)
{
  PRINT_FUNCTION_INFO_MAX;

  return SUCCESS;
}

STATUS
WriteTrainingEntryHooks (
  MMRC_DATA *ModMrcData
)
{
  PRINT_FUNCTION_INFO_MAX;
  return SUCCESS;
}

STATUS
WriteTrainingExitHooks (
  MMRC_DATA *ModMrcData
)
{
  PRINT_FUNCTION_INFO_MAX;
  return SUCCESS;
}


STATUS 
JedecCmd (
  MMRC_DATA   *ModMrcData,
  UINT8       Channel,
  UINT8       Rank,
  UINT8       cmd,
  UINT32      Data
)
{
	DramInitMisc miscCommand;
	DramInitDDR3EMR1 emrs1Command;
    UINT32 dramZQCALCommand = 0;
	UINT32 dramRESETCommand = 0;
	UINT32 ba;
	UINT32 dramPRECommand = 0;
    UINT32 mr01, mr02, mr03, tWR, RLWL=4;
#if MAX_CHANNELS == 1
    UINT8 Dunit_PortID[MAX_CHANNELS] = {0x1};
#endif
#if MAX_CHANNELS == 2
    UINT8 Dunit_PortID[MAX_CHANNELS] = {0x1, 0x7};
#endif

    emrs1Command.raw = (Data & 0xFFFF) << 6;

    switch (cmd) {
		case JEDEC_PRECHARGEALL:	{
			if (ModMrcData->CurrentDdrType <= TYPE_DDR3L_ECC) {
				miscCommand.raw = 0;
				miscCommand.field.command = 2;
				miscCommand.field.rankSelect = Rank;
				miscCommand.field.multAddress = BIT10;
				DramInitCommand(Dunit_PortID[Channel], miscCommand.raw);
			} else {
				dramPRECommand = (MMRC_JEDEC_CMD_PRECHARGE_ALL| MMRC_JEDEC_CMD_RANK(Rank) );
				//precharge all
				DramInitCommand (Dunit_PortID[Channel], dramPRECommand);
			}
		} break;

		case JEDEC_REFRESH :	{
			miscCommand.raw = 0;
			miscCommand.field.command = 1;
			miscCommand.field.rankSelect = Rank;
			DramInitCommand(Dunit_PortID[Channel], miscCommand.raw);
		} break;


		case JEDEC_LMR: {
			ba = (Data>>16) & 0x7;
			//emrs1Command.raw = 0;
			emrs1Command.field.rankSelect = Rank;
			//emrs1Command.field.data = (data & 0xffff);
			emrs1Command.field.bankAddress = ba;
			DramInitCommand(Dunit_PortID[Channel], emrs1Command.raw);
		} break;

		case MMRC_JEDEC_CMD_NOP: {
			DramInitCommand(Dunit_PortID[Channel], MMRC_JEDEC_CMD_NOP|(Rank << 22) );
		} break;

		case MMRC_JEDEC_CMD_RESET: {
			dramRESETCommand = ((MMRC_JEDEC_CMD_RESET << 4) | MMRC_JEDEC_CMD_MRW);
			DramInitCommand(Dunit_PortID[Channel], dramRESETCommand|(Rank << 22) );
		} break;

		case MMRC_JEDEC_CMD_POST_INIT_CAL: {
			dramZQCALCommand = (MMRC_JEDEC_CMD_MRW + MMRC_JEDEC_CMD_MR(10) + MMRC_JEDEC_CMD_DATA(MMRC_JEDEC_CMD_POST_INIT_CAL));
			DramInitCommand(Dunit_PortID[Channel], dramZQCALCommand|(Rank << 22) );

		}  break;

		case MMRC_JEDEC_CMD_MR1: {

			   if ((ModMrcData->CurrentFrequency == FREQ_800) || (ModMrcData->CurrentFrequency == FREQ_800_DDLL) || (ModMrcData->CurrentFrequency == FREQ_800_DDLL_BYP_MPLL)) {
						tWR = MMRC_JEDEC_MR1_nWR6;
			   } else if ((ModMrcData->CurrentFrequency == FREQ_1066) || (ModMrcData->CurrentFrequency == FREQ_1066_DDLL)) {
						tWR = MMRC_JEDEC_MR1_nWR8;
				} else {
						tWR = MMRC_JEDEC_MR1_nWR10;
				}
			   mr01 = (MMRC_JEDEC_CMD_MRW + JEDEC_CMD_MR(1) + MMRC_JEDEC_CMD_DATA(tWR + MMRC_JEDEC_MR1_BL8) );
			   DramInitCommand(Dunit_PortID[Channel], mr01 | MMRC_JEDEC_CMD_RANK(Rank) );
		} break;

		case MMRC_JEDEC_CMD_MR2: {
			   if ((ModMrcData->CurrentFrequency == FREQ_800) || (ModMrcData->CurrentFrequency == FREQ_800_DDLL) || (ModMrcData->CurrentFrequency == FREQ_800_DDLL_BYP_MPLL)) {
						RLWL = MMRC_JEDEC_MR2_RL6WL3;
			   } else if ((ModMrcData->CurrentFrequency == FREQ_1066) || (ModMrcData->CurrentFrequency == FREQ_1066_DDLL)) {
					   RLWL = MMRC_JEDEC_MR2_RL8WL4;
			   } else {
						RLWL = MMRC_JEDEC_MR2_RL10WL6;
			   }
               	if (Data == 0) {
			   		mr02 = (MMRC_JEDEC_CMD_MRW + MMRC_JEDEC_CMD_MR(2) + MMRC_JEDEC_CMD_DATA(RLWL + MMRC_JEDEC_MR2_WRE_LT9 + MMRC_JEDEC_MR2_WL_SETA + MMRC_JEDEC_MR2_WRLVL_DIS)); }
        		else {
               		mr02 = (MMRC_JEDEC_CMD_MRW + MMRC_JEDEC_CMD_MR(2) + MMRC_JEDEC_CMD_DATA(RLWL + MMRC_JEDEC_MR2_WRE_LT9 + MMRC_JEDEC_MR2_WL_SETA + MMRC_JEDEC_MR2_WRLVL_ENB));
                }
			   DramInitCommand(Dunit_PortID[Channel], mr02 | MMRC_JEDEC_CMD_RANK(Rank) );
		} break;
		case MMRC_JEDEC_CMD_MR3:
			mr03 = (MMRC_JEDEC_CMD_MRW + MMRC_JEDEC_CMD_MR(3) + MMRC_JEDEC_CMD_DATA(MMRC_JEDEC_MR3_OHM_40));
			DramInitCommand(Dunit_PortID[Channel], mr03 | MMRC_JEDEC_CMD_RANK(Rank) );
			break;

		case MMRC_JEDEC_CMD_ODT:
			DramInitCommand(Dunit_PortID[Channel], (0x70b0 | MMRC_JEDEC_CMD_RANK(Rank)) );
			break;
		default:
			break;
	}
	return SUCCESS;
}

STATUS ForceODT (
  UINT8       MrcDebugMsgLevel,
  UINT8       Channel, 
  UINT8       Rank, 
  UINT8       Value
  )
{
	// Always force on the ODT for the given rank.
	// Rnk2 is only enabled.
	UINT32 tempValue_u32;
#if MAX_CHANNELS == 1
    UINT8 Dunit_PortID[MAX_CHANNELS] = {0x1};
#endif
#if MAX_CHANNELS == 2
    UINT8 Dunit_PortID[MAX_CHANNELS] = {0x1, 0x7};
#endif

	if (Value == MMRC_FORCEODT_ON) {
		tempValue_u32 = MsgBus32Read(0x1, 0xb);
		tempValue_u32 &= 0xffffe0ff;
		tempValue_u32 |= 1<<(8+Rank);
  //      tempValue_u32 |= 0xf<<8;
		tempValue_u32 |= 1<<12;
		MsgBus32Write(Dunit_PortID[Channel], 0xb, tempValue_u32);

	} else { // FORCEODT_OFF
		tempValue_u32 = MsgBus32Read(0x1, 0xb);
		tempValue_u32 &= 0xffffe0ff;
		MsgBus32Write(Dunit_PortID[Channel], 0xb, tempValue_u32);
	}
    return SUCCESS;
}

UINT32
GetAddress (MMRC_DATA *ModMrcData, UINT8 Channel, UINT8 Rank)
{
  //return ( (UINT32)( ( Rank * RANK_SIZE) + (RANK_SIZE / 4)  ) );
  return ( (UINT32)( Rank * RANK_SIZE) );
}





