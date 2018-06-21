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

  Memory.h

Abstract:

  This file contain defination and macro used by MRC.

--*/

#ifndef _MEMORY_H_
#define _MEMORY_H_

#pragma pack(1)

#define MMRC_FORCEODT_ON            0x01
#define MMRC_FORCEODT_OFF	        0x00

#define MMRC_JEDEC_CMD_MR(x)                     (((x) & 0xFF) << 4)
#define MMRC_JEDEC_CMD_DATA(x)                   (((x) & 0xFF) << 12)
#define MMRC_JEDEC_CMD_RANK(x)                   ((x & 0x3) << 22)

#define MMRC_JEDEC_CMD_NOP                       0x00000007  /**< NOP Command (CA0=1, CA1=1, CA2=1, CA3=0) */
#define MMRC_JEDEC_CMD_MRW                       0x00000000  /**< MRW Command (CA0=0, CA1=0, CA2=0, CA3=0) */
#define MMRC_JEDEC_CMD_MRR                       0x00000008  /**< MRR Command (CA0=0, CA1=0, CA2=0, CA3=1) */
#define MMRC_JEDEC_CMD_PRECHARGE_ALL             0x0000001B  /**< PRECHARGE ALL Command (CA0=1, CA1=1, CA2=0, CA3=1, CA4r=1) */
#define MMRC_JEDEC_CMD_MRW_RESET                 0x000003F0  /**< MRW(Reset) Command (CA0=0, CA1=0, CA2=0, CA3=0) */

#define MMRC_JEDEC_MR10				(0xA << 4)
#define MMRC_JEDEC_MR01				(0x1 << 4)
#define MMRC_JEDEC_MR02				(0x2 << 4)
#define MMRC_JEDEC_MR03				(0x3 << 4)

#define MMRC_JEDEC_CMD_POST_INIT_CAL	(0xFF)

#define MMRC_JEDEC_MR1_BL8			(0x3)
#define MMRC_JEDEC_MR1_BT_SEQ			(0x0 << 3)
#define MMRC_JEDEC_MR1_BT_INT			(0x1 << 3)
#define MMRC_JEDEC_MR1_WC				(0x0 << 4)
#define MMRC_JEDEC_MR1_NWC			(0x1 << 4)
#define MMRC_JEDEC_MR1_nWR10			(0x0 << 5)
#define MMRC_JEDEC_MR1_nWR6			(0x4 << 5)
#define MMRC_JEDEC_MR1_nWR8			(0x6 << 5)


#define MMRC_JEDEC_MR2_RL6WL3			(0x4)
#define MMRC_JEDEC_MR2_RL8WL4			(0x6)

#define MMRC_JEDEC_MR2_RL10WL6		(0x8)
#define MMRC_JEDEC_MR2_WRE_LT9		(0x0 << 4)
#define MMRC_JEDEC_MR2_WRE_GT9    	(0x1 << 4)
#define MMRC_JEDEC_MR2_WL_SETA    	(0x0 << 6)
#define MMRC_JEDEC_MR2_WL_SETB    	(0x1 << 6)
#define MMRC_JEDEC_MR2_WRLVL_DIS    	(0x0 << 7)
#define MMRC_JEDEC_MR2_WRLVL_ENB    	(0x1 << 7)

#define MMRC_JEDEC_MR3_OHM_343		(0x1)
#define MMRC_JEDEC_MR3_OHM_40			(0x2)
#define MMRC_JEDEC_MR3_OHM_48			(0x3)
#define MMRC_JEDEC_MR3_OHM_60			(0x4)

//
// Frequencies
//
#define MMRC_DDRFREQ_800           0x00
#define MMRC_DDRFREQ_1066          0x01
#define MMRC_DDRFREQ_1333          0x02
#define MMRC_DDRFREQ_1600          0x03


//
// Memory detection Data structures
//
#ifndef MMRC_JEDEC_CMD_NOP
#define MMRC_JEDEC_CMD_NOP                       	0x00000007  /**< NOP Command (CA0=1, CA1=1, CA2=1, CA3=0) */
#endif

#ifndef MMRC_JEDEC_CMD_RESET
#define MMRC_JEDEC_CMD_RESET     					0x000003F
#endif

#define MMRC_JEDEC_CMD_MRW                       	0x00000000  /**< MRW Command (CA0=0, CA1=0, CA2=0, CA3=0) */
#define MMRC_JEDEC_CMD_POST_INIT_CAL			(0xFF)
#define MMRC_JEDEC_CMD_MR1							0x11
#define MMRC_JEDEC_CMD_MR2							0x12
#define MMRC_JEDEC_CMD_MR3							0x13
#define MMRC_JEDEC_CMD_ODT							0x14

#define JEDEC_PRECHARGEALL      	0x01
#define	JEDEC_LMR               		 	0x02
#define JEDEC_REFRESH          	 	0x03


#define FORCEODT_ON            0x01
#define FORCEODT_OFF          0x00


//
// Values taken on by BootMode global
//
//#define S0Path               BIT0
//#define S3Path               BIT1
//#define S5Path               BIT2
//#define FBPath               BIT3
// Backward compatibility
#define BOOT_ON_S3_EXIT      S3Path

#pragma pack()

#endif
