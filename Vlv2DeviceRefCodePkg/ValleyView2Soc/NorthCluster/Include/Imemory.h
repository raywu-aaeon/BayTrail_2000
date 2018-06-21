/*++

Copyright (c) 2005-2009 Intel Corporation. All rights reserved
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
//#define FLOOR(a,b)           ((a) - ((a) % (b)))
//#define CEILING(a,b)         (a + (1 - ((a) % (b))))
#define CEILDIVU(a,b)        (((a)+(b)-(1))/(b))
#define CEILDIVG(a,b,g)      (g*(CEILDIVU(CEILDIVU(a,b),g)))
//#define ABS(a,b)             (((a) < (b)) ? ((b) - (a)) : ((a) - (b)))
#define CHECKPOINT(CP)       (Checkpoint(CP))

#define HPET_1US             0x000F
#define HPET_1MS             0x37EF

#define NODEFAULT __assume(0)

#define MIN_COLUMNS          0x09             // Minimum column size chipset supports
#define MIN_ROWS             0x0D             // Minimum row size chipset supports

/* Message Bus Commands */
#define VLV_CMD_READ_REG            0x10000000      /**< Read Message Bus Register Command */
#define VLV_CMD_WRITE_REG           0x11000000      /**< Write Message Bus Register Command */
#define VLV_CMD_DRAM_INIT           0x68000000      /**< JEDEC Command */
#define VLV_CMD_WAKE                0xCA000000      /**< Wake Commamd */

//---------------------------------------------------------------------
//       JEDEC related IOP's
//---------------------------------------------------------------------

#define JEDEC_MODE_SEL       (BIT3|BIT2|BIT1)
#define JEDEC_EMRS_MODE      (BIT5|BIT4)
#define JEDEC_WL_MODE        (BIT6)

#define NOP_COMMAND          (BIT1)
#define PRE_CHARGE_COMMAND   (BIT2)
#define MRS_COMMAND          (BIT2|BIT1)
#define EMRS_COMMAND         (BIT3)
#define EMRS1_COMMAND        (EMRS_COMMAND|BIT4)
#define EMRS2_COMMAND        (EMRS_COMMAND|BIT5)
#define EMRS3_COMMAND        (EMRS_COMMAND|BIT5|BIT4)
#define ZQCAL_COMMAND        (BIT3|BIT1)
#define CBR_COMMAND          (BIT3|BIT2)
#define NORMAL_OP_COMMAND    (BIT3|BIT2|BIT1)

/* JEDEC Commands */
#define JEDEC_CMD_NOP               0x00000007      /**< NOP Command */
#define JEDEC_CMD_PRECHARGE_ALL     0x00010002      /**< PRECHARGE ALL Command */
#define JEDEC_CMD_EMRS2             0x00000010      /**< EMRS(2) Command */
#define JEDEC_CMD_EMRS3             0x00000018      /**< EMRS(3) Command */
#define JEDEC_CMD_EMRS1             0x00000008      /**< EMRS(1) Command */
#define JEDEC_CMD_MRS               0x00000000      /**< MRS Command */
#define JEDEC_CMD_AUTO_REFRESH      0x00010001      /**< AUTO REFRESH Command */

/* LPDDR2 JEDEC Commands */
#define LPDDR2_JEDEC_CMD_PRECHARGE_ALL  0x1B
#define LPDDR2_JEDEC_CMD_MRW            0x0
#define LPDDR2_JEDEC_CMD_MRR            0x80
#define LPDDR2_JEDEC_CMD_RESET          (0x3f << 4)
#define LPDDR2_JEDEC_MR10               (0xA << 4)
#define LPDDR2_JEDEC_MR01               (0x1 << 4)
#define LPDDR2_JEDEC_MR02               (0x2 << 4)
#define LPDDR2_JEDEC_MR03               (0x3 << 4)
#define LPDDR2_JEDEC_CMD_POST_INIT_CAL  (0xFF << 12)    //CA2 fall edge
#define LPDDR2_JEDEC_MR1_BT_SEQ         (0x0 << 15)
#define LPDDR2_JEDEC_MR1_BT_INT         (0x1 << 15)
#define LPDDR2_JEDEC_MR1_WC             (0x0 << 16)
#define LPDDR2_JEDEC_MR1_NWC            (0x1 << 16)
#define LPDDR2_JEDEC_MR1_nWR3           (0x1 << 17)
#define LPDDR2_JEDEC_MR1_nWR4           (0x2 << 17)
#define LPDDR2_JEDEC_MR1_nWR5           (0x3 << 17)
#define LPDDR2_JEDEC_MR1_nWR6           (0x4 << 17)
#define LPDDR2_JEDEC_MR1_nWR7           (0x5 << 17)
#define LPDDR2_JEDEC_MR1_nWR8           (0x6 << 17)
#define LPDDR2_JEDEC_MR2_RL3            (0x1 << 12)
#define LPDDR2_JEDEC_MR3_OHM_343        (0x1 << 12)
#define LPDDR2_JEDEC_MR3_OHM_40         (0x2 << 12)
#define LPDDR2_JEDEC_MR3_OHM_48         (0x3 << 12)
#define LPDDR2_JEDEC_MR3_OHM_60         (0x4 << 12)

//
// Maximum number of SDRAM channels supported by the memory controller
//
#define MAX_CHANNELS         1

//
// Maximum number of DIMM sockets supported by each channel
//
#define MAX_SLOTS            1

//
// Maximum number of sides supported per DIMM
//
#define MAX_SIDES            2

//
// Maximum number of DIMM sockets supported by the memory controller
//
#define MAX_SOCKETS          (MAX_CHANNELS * MAX_SLOTS)

//
// Maximum number of rows supported by the memory controller
//
#define MAX_RANKS            (MAX_SOCKETS * MAX_SIDES)


#define RNK_PER_SLOT    2
#define MAXDQ           8
#define MAXECC          0
#define MAX_MODULES     4
#define MAX_BYTELANES_PER_MODULE    2

#define MAX_RCOMP_GROUP      7
#define MAX_DQ_MODULE        4
#define MAX_DQ_STEP          3
#define MAX_COMMAND_STEP     3
#define MAX_CLOCK_STEP       3
#define MAX_PLL_STEP         16
#define MAX_COMP_STEP        105//67//81//4//80
#define MAX_VREGEN           7//6
#define MAX_MDLLEN           7//6
#define MAX_PIEN             7//6
#define MAX_RXEN             10//8
#define MAX_MCHODTEN          10
#define MAX_DIMM_STEP        48
#define MAX_DIMM_TYPE        20//19
#define MAX_DATAMOD_STEP     11//8//36
#define MAX_DATAMOD_IND_STEP 13
#define MAX_CMDCLKCTRL_STEP  21

//
// Minimum and Maximum CL
//
#define MINCLINX             5
#define MAXCLINX             11
#define C_MAXCLTBL           (MAXCLINX-MINCLINX+1)

//
// SPD constraints in DRAM clocks
//
#define MIN_SUPPORTED_TCL    5
#define MAX_SUPPORTED_TCL    11
#define ADJUST_TCL      4
#define ADJUST_TRP      3
#define ADJUST_TRCD     3
#define ADJUST_TRAS     9
#define ADJUST_TWR      0
#define ADJUST_TRFC     15
#define ADJUST_TWTR     0
#define ADJUST_TRRD     0
#define ADJUST_TRTP     0
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

typedef enum {
  DDRType_DDR3 = 0,
  DDRType_DDR3L = 1,
  DDRType_DDR3U = 2,
  DDRType_DDR3All = 3,
  DDRType_LPDDR2 = 4,
  DDRType_LPDDR3 = 5,
  DDRType_DDR4 = 6
};

#define C_MAXDDRTYPE 3

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

//
// Memory detection data structures
//

typedef struct _TCLKTACENTRY {
  UINT8           Tclk;
  UINT8           Tac;
} TCLKTACENTRY;


//
// Values taken on by BootMode global
//
#define S0Path               BIT0
#define S3Path               BIT1
#define S5Path               BIT2
#define FBPath               BIT3
// Backward compatibility
#define BOOT_ON_S3_EXIT      S3Path

#pragma pack()

#endif
