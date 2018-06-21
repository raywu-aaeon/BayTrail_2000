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

  Header file for all external hooks used by modular MRC.

--*/

#ifndef _MMRC_HOOKS_H_
#define _MMRC_HOOKS_H_

#include "Types.h"
#include "Printf.h"
#include "../../IpBlocks/VLVA0/Include/MMRCProjectData.h"
#include "RegAccess.h"
#ifndef  MINIBIOS_BUILD
#endif

//
// Preset each rank to 1024MB's.
//
#ifndef RANK_SIZE
#define RANK_SIZE 1024*1024*1024
#endif

#define ODD_MODE_BITSHIFTS    16
#define EVEN_MODE_BITSHIFTS   1

//
// Randome seens for Victim/Aggressor modes
//
#define CPGC_LFSR_VICTIM_SEED   0xF294BA21 // Random seed for victim.
#define CPGC_LFSR_AGRESSOR_SEED 0xEBA7492D // Random seed for aggressor.

//
// PATTERN MODES
//
#define PATTERN_VICAGG        0
#define LFSR_VICAGG           1
#define LFSR                  2

//
// Message Bus Commands
//
#define CMD_READ_REG            0x10000000      /**< Read Message Bus Register Command */
#define CMD_WRITE_REG           0x11000000      /**< Write Message Bus Register Command */
#define CMD_DRAM_INIT           0x68000000      /**< JEDEC Command */
#define CMD_WAKE                0xCA000000      /**< Wake Command */
#define CMD_SUSPEND             0xCC000000      /**< Suspend Command */

//
// Bit defs
//
#ifndef BIT0
#define BIT0                 0x00000001
#define BIT1                 0x00000002
#define BIT2                 0x00000004
#define BIT3                 0x00000008
#define BIT4                 0x00000010
#define BIT5                 0x00000020
#define BIT6                 0x00000040
#define BIT7                 0x00000080
#define BIT8                 0x00000100
#define BIT9                 0x00000200
#define BIT10                0x00000400
#define BIT11                0x00000800
#define BIT12                0x00001000
#define BIT13                0x00002000
#define BIT14                0x00004000
#define BIT15                0x00008000
#define BIT16                0x00010000
#define BIT17                0x00020000
#define BIT18                0x00040000
#define BIT19                0x00080000
#define BIT20                0x00100000
#define BIT21                0x00200000
#define BIT22                0x00400000
#define BIT23                0x00800000
#define BIT24                0x01000000
#define BIT25                0x02000000
#define BIT26                0x04000000
#define BIT27                0x08000000
#define BIT28                0x10000000
#define BIT29                0x20000000
#define BIT30                0x40000000
#define BIT31                0x80000000
#endif

#ifndef MRS_DEFINED
VOID DramInitCommand (
  IN          UINT8       MrcDebugMsgLevel,
  IN          UINT8       Channel,
  IN          UINT32      Data
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
;

VOID DramWakeCommand (
  IN          UINT8       MrcDebugMsgLevel,
  IN          UINT8       Channel
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
;

VOID
DramSuspendCommand (
  IN          UINT8       MrcDebugMsgLevel,
  IN          UINT8       Channel
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
;
#endif

STATUS
ReceiveEnableEntryHooks (
  MMRC_DATA   *ModMrcData,
  UINT8        Channel
);

STATUS
ReceiveEnableExitHooks (
  MMRC_DATA   *ModMrcData,
  UINT8        Channel
);

STATUS
ReadVrefEntryHooks (
  MMRC_DATA *ModMrcData
);

STATUS
ReadVrefExitHooks (
  MMRC_DATA *ModMrcData
);

STATUS
FineWriteLevelingEntryHooks (
  MMRC_DATA *ModMrcData
);

STATUS
FineWriteLevelingExitHooks (
  MMRC_DATA *ModMrcData
);

STATUS
CoarseWriteLevelingEntryHooks (
  MMRC_DATA *ModMrcData
);

STATUS
CoarseWriteLevelingExitHooks (
  MMRC_DATA *ModMrcData
);

STATUS
ReadTrainingEntryHooks (
  MMRC_DATA *ModMrcData
);

STATUS
ReadTrainingExitHooks (
  MMRC_DATA *ModMrcData
);

STATUS
WriteTrainingEntryHooks (
  MMRC_DATA *ModMrcData
);

STATUS
WriteTrainingExitHooks (
  MMRC_DATA *ModMrcData
);

STATUS 
JedecCmd (
  MMRC_DATA   *ModMrcData,
  UINT8       Channel,
  UINT8       Rank,
  UINT8       cmd,
  UINT32      Data
);

STATUS ForceODT (
  UINT8       MrcDebugMsgLevel,
  UINT8       Channel, 
  UINT8       Rank, 
  UINT8       Value
  );

#endif
