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

	OemHooks.h
	
Abstract: 

	This file the include all the external OEM Hooks.

--*/

#ifndef _OEMHOOKS_H_
#define _OEMHOOKS_H_

#include "Mrc.h"

//
// MRC Report Status Code
//
// MrcConfigMemProgressCodeBase=0x20 => PC from 0x21 to 0x2F
#define MRC_MEMORY_CONFIG_PC_BASE      0x20

enum {
  CPU_ODT_DEFAULT = 1,
  CPU_ODT_60 = 2,
  CPU_ODT_80 = 3,
  CPU_ODT_100 = 4,
  CPU_ODT_120 = 5,
  CPU_ODT_150 = 6,
} CPU_ODT_DEFINATION;

enum {
  DRAM_ODT_DEFAULT = 1,
  DRAM_ODT_60 = 2,
  DRAM_ODT_120 = 3,
} DRAM_ODT_DEFINATION;

enum {
  DRAM_RTT_NOM_DISABLED = 0,
  DRAM_RTT_NOM_ENABLED = 1,
  DRAM_RTT_NOM_DEFAULT = 2,
} DRAM_RTT_NOM_DEFINATION;

enum {
  AUTO_SELF_REFRESH_OFF = 0,
  AUTO_SELF_REFRESH_ON = 1,
  AUTO_SELF_REFRESH_DEFAULT = 2,
} AUTO_SELF_REFRESH_DEFINATION;

enum {
  DEFAULT_PLATFORM_DESIGN = 0,
  BLK_RVP_DDR3L = 1,
  BBY_25x27_DDR3L_MEMDOWN = 2,
  BBY_25x27_RAMBI_DDR3L_MEMDOWN = 3,
  BBY_25x27_4LAYERS_DDR3L_MEMDOWN = 4,
}CURRENT_PLATFORM_DESIGN;

#pragma pack(1)
typedef struct {
        UINT8    Rank_En[MAX_CHANNELS_TOTAL][RANKS_PER_CHANNEL];        /**< Ranks Present with MAX_RANKS defined in Imemory.h */
        UINT8    DIMM_DWidth[MAX_CHANNELS_TOTAL][MAX_SLOTS];  /**< DIMM0 DRAM device data width 0:x8, 1:x16, 2:x32*/
        UINT8    DIMM_Density[MAX_CHANNELS_TOTAL][MAX_SLOTS]; /**< DIMM0 DRAM device data width 0:1Gbit, 1:2Gbit, 2:4Gbit, 3:8Gbit*/
        UINT8    DIMM_BusWidth[MAX_CHANNELS_TOTAL][MAX_SLOTS];/**< 0:8bits; 1:16bits, 2:32bits, 3:64bits */
        UINT8    DIMM_Sides[MAX_CHANNELS_TOTAL][MAX_SLOTS];   /**< rank per dimm 0:1rank, 1:2ranks */
        UINT8    DRAM_Speed;                /**< 0:800, 1:1066, 2:1333, 3:1600 */
        UINT8    DRAM_Type;                 /**< 0:DDR3, 1:DDR3L, 2:DDR3U, 4:LPDDR2, 5:LPDDR3, 6:DDR4 */
        UINT8    DIMM_MemDown;              /**< 0:DIMM, 1:Memory Down */     
        UINT8    tCL;                       /**< actual CL */
        UINT8    tRP_tRCD;                  /**< TRP and tRCD in dram clk - 5:12.5ns, 6:15ns, 7:*/
        UINT8    tWR;                       /**< in dram clk  */
        UINT8    tWTR;                      /**< in dram clk  */
        UINT8    tRRD;                      /**< in dram clk  */
        UINT8    tRTP;                      /**< in dram clk  */
        UINT8    tFAW;                      /**< in dram clk  */
} MRC_DRAM_INPUT;
#pragma pack()

VOID
Checkpoint (
  UINT16 Content
  );

STATUS
OemTrackInitComplete (
  MRC_PARAMETER_FRAME           *CurrentMrcData, UINT8 Channel
  );

#endif

