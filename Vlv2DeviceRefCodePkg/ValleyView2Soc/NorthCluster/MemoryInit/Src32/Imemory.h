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

  Imemory.h

Abstract:

  This file contain defination and macro used by MRC.

--*/

#ifndef _IMEMORY_H_
#define _IMEMORY_H_

#pragma pack(1)

#define MIN(a,b)             (((a) < (b)) ? (a) : (b))
#define MAX(a,b)             (((a) > (b)) ? (a) : (b))

#define CHECKPOINT(CP)       (Checkpoint(CP))

/* Message Bus Commands */
#define VLV_CMD_READ_REG            0x10000000      /**< Read Message Bus Register Command */
#define VLV_CMD_WRITE_REG           0x11000000      /**< Write Message Bus Register Command */
#define VLV_CMD_DRAM_INIT           0x68000000      /**< JEDEC Command */
#define VLV_CMD_WAKE                0xCA000000      /**< Wake Commamd */

#ifndef DDR3_SUPPORT
#define DDR3_SUPPORT    1
#endif

#ifndef LPDDR2_SUPPORT
#define LPDDR2_SUPPORT  0
#endif

#ifndef LPDDR3_SUPPORT
#define LPDDR3_SUPPORT  1
#endif

#ifndef DDR3_ECC
#define DDR3_ECC 1
#endif

#if defined LPDDR2_SUPPORT && LPDDR2_SUPPORT
#define LPDDR2_DETECTED      (CurrentMrcData->DDRType == DDRType_LPDDR2)
#endif

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
#define LPDDR3_DETECTED      (CurrentMrcData->DDRType == DDRType_LPDDR3)
#endif

#if defined DDR3_SUPPORT && DDR3_SUPPORT
#define DDR3_DETECTED      (CurrentMrcData->DDRType < DDRType_DDR3All)
#endif

//---------------------------------------------------------------------
//       JEDEC related IOP's
//---------------------------------------------------------------------
#define JEDEC_CMD_MR(x)                     (((x) & 0xFF) << 4)
#define JEDEC_CMD_DATA(x)                   (((x) & 0xFF) << 12)
#define JEDEC_CMD_RANK(x)                   ((x & 0x3) << 22)

#if defined LPDDR3_SUPPORT && LPDDR3_SUPPORT
/* LPDDR3 JEDEC Commands */
#define LPDDR3_JEDEC_CMD_NOP                       0x00000007  /**< NOP Command (CA0=1, CA1=1, CA2=1, CA3=0) */
#define LPDDR3_JEDEC_CMD_MRW                       0x00000000  /**< MRW Command (CA0=0, CA1=0, CA2=0, CA3=0) */
#define LPDDR3_JEDEC_CMD_MRR                       0x00000008  /**< MRR Command (CA0=0, CA1=0, CA2=0, CA3=1) */
#define LPDDR3_JEDEC_CMD_PRECHARGE_ALL             0x0000001B  /**< PRECHARGE ALL Command (CA0=1, CA1=1, CA2=0, CA3=1, CA4r=1) */
#define LPDDR3_JEDEC_CMD_MRW_RESET                 0x000003F0  /**< MRW(Reset) Command (CA0=0, CA1=0, CA2=0, CA3=0) */

#define LPDDR3_JEDEC_MR10				(0xA << 4)
#define LPDDR3_JEDEC_MR01				(0x1 << 4)
#define LPDDR3_JEDEC_MR02				(0x2 << 4)
#define LPDDR3_JEDEC_MR03				(0x3 << 4)

#define LPDDR3_JEDEC_CMD_POST_INIT_CAL	(0xFF)

#define LPDDR3_JEDEC_MR1_BL8			(0x3)
#define LPDDR3_JEDEC_MR1_BL16			(0x4)
#define LPDDR3_JEDEC_MR1_BT_SEQ			(0x0 << 3)
#define LPDDR3_JEDEC_MR1_BT_INT			(0x1 << 3)
#define LPDDR3_JEDEC_MR1_WC				(0x0 << 4)
#define LPDDR3_JEDEC_MR1_NWC			(0x1 << 4)
#define LPDDR3_JEDEC_MR1_nWR10			(0x0 << 5)
#define LPDDR3_JEDEC_MR1_nWR11			(0x1 << 5)
#define LPDDR3_JEDEC_MR1_nWR12			(0x2 << 5)
#define LPDDR3_JEDEC_MR1_nWR5			(0x3 << 5)
#define LPDDR3_JEDEC_MR1_nWR6			(0x4 << 5)
#define LPDDR3_JEDEC_MR1_nWR7			(0x5 << 5)
#define LPDDR3_JEDEC_MR1_nWR8			(0x6 << 5)

#define LPDDR3_JEDEC_MR2_RL3WL1			(0x1)
#define LPDDR3_JEDEC_MR2_RL6WL3			(0x4)
#define LPDDR3_JEDEC_MR2_RL8WL4			(0x6)
#define LPDDR3_JEDEC_MR2_RL9WL5			(0x7)
#define LPDDR3_JEDEC_MR2_RL10WL6		(0x8)
#define LPDDR3_JEDEC_MR2_RL11WL6		(0x9)
#define LPDDR3_JEDEC_MR2_RL12WL6		(0xA)
#define LPDDR3_JEDEC_MR2_WRE_LT9		(0x0 << 4)
#define LPDDR3_JEDEC_MR2_WRE_GT9    	(0x1 << 4)
#define LPDDR3_JEDEC_MR2_WL_SETA    	(0x0 << 6)
#define LPDDR3_JEDEC_MR2_WL_SETB    	(0x1 << 6)
#define LPDDR3_JEDEC_MR2_WRLVL_DIS    	(0x0 << 7)
#define LPDDR3_JEDEC_MR2_WRLVL_ENB    	(0x1 << 7)

#define LPDDR3_JEDEC_MR3_OHM_343		(0x1)
#define LPDDR3_JEDEC_MR3_OHM_40			(0x2)
#define LPDDR3_JEDEC_MR3_OHM_48			(0x3)
#define LPDDR3_JEDEC_MR3_OHM_60			(0x4)
#endif

//
// Maximum number of SDRAM channels supported by the memory controller
//
#define MAX_CHANNELS_TOTAL         2

//
// Maximum number of DIMM sockets supported by each channel
//
#define MAX_SLOTS            1

//
// Maximum number of sides supported per DIMM
//
#define MAX_SIDES            2

//
// Maximum number of DIMM sockets supported by the memory controller (all channels)
//
#ifndef MAX_DIMMS
#define MAX_DIMMS          (MAX_CHANNELS_TOTAL * MAX_SLOTS)
#endif

//
// Maximum number of rows supported by the memory controller
//
//#ifndef MAX_RANKS
//#define MAX_RANKS        (MAX_SLOTS * MAX_SIDES)//    (MAX_DIMMS * MAX_SIDES)
//#endif

#define RANKS_PER_CHANNEL    (MAX_SLOTS * MAX_SIDES)

#ifndef RANK_SIZE
#define	RANK_SIZE	2*512*1024*1024	// Preset each rank to 1024MB's.
#endif

//
// SPD constraints in DRAM clocks
//
#define MIN_SUPPORTED_TCL    5
#define MAX_SUPPORTED_TCL    11
#define ADJUST_TCL		4
#define ADJUST_TRP		3
#define ADJUST_TRCD		3
#define ADJUST_TRAS		9
#define ADJUST_TWR		0
#define ADJUST_TRFC		15
#define ADJUST_TWTR		0
#define ADJUST_TRRD		0
#define ADJUST_TRTP		0
#define MIN_SUPPORTED_TWR    3
#define MAX_SUPPORTED_TWR    15
#define MIN_SUPPORTED_TRCD   0
#define MAX_SUPPORTED_TRCD   12
#define MIN_SUPPORTED_TRRD   2
#define MAX_SUPPORTED_TRRD   15
#define MIN_SUPPORTED_TRP    0
#define MAX_SUPPORTED_TRP    12
#define MIN_SUPPORTED_TRAS   0
#define MAX_SUPPORTED_TRAS   28
#define MIN_SUPPORTED_TRFC   0
#define MAX_SUPPORTED_TRFC   ((2*2*2*2*2*2*2*2)-1)
#define MIN_SUPPORTED_TWTR   4
#define MAX_SUPPORTED_TWTR   15
#define MIN_SUPPORTED_TRTP   4
#define MAX_SUPPORTED_TRTP   15
#define MIN_SUPPORTED_TFAW   15
#define MAX_SUPPORTED_TFAW   ((2*2*2*2*2*2)-1)

#define MIN_TRRD       4

//Message Bus Mask
#define MSGBUS_MASKHI  0xFFFFFF00
#define MSGBUS_MASKLO  0x000000FF

//
// Frequencies
//
#define DDRFREQ_800           0x00
#define DDRFREQ_1066          0x01
#define DDRFREQ_1333          0x02
#define DDRFREQ_1600          0x03

#define MINDDR               (DDRFREQ_800)
#define MAXDDR               (DDRFREQ_1600)
#define C_MAXDDR             (MAXDDR-MINDDR+1)

#define COREFREQ_800          0x00
#define COREFREQ_1066         0x01
#define COREFREQ_1333         0x02
#define COREFREQ_1600         0x03

#define MINCOREFREQ               (COREFREQ_800)
#define MAXCOREFREQ               (COREFREQ_1333)

#define C_MAXCOREFREQ             (MAXCOREFREQ-MINCOREFREQ+1)

enum {
    DDRType_DDR3 = 0,
    DDRType_DDR3L = 1,	
    DDRType_DDR3ECC = 2,
	DDRType_DDR3All = 3,
    DDRType_LPDDR2 = 4,	
    DDRType_LPDDR3 = 5,
    DDRType_DDR4 = 6
};

#define C_MAXDDRTYPE 2

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

#define BOOT_ON_S3_EXIT      S3Path

#pragma pack()

#endif
