/*++

Copyright (c) 1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    Spd.h
    
Abstract: 

    This file include all the spd data defination.

--*/

#ifndef _SPD_H_
#define _SPD_H_

#define MAX_SPD_ADDR   125//63 //SPD Index of highest byte used by this reference code

//---------------------------------------------------------------------
//       SDRAM SPD field definitions
//---------------------------------------------------------------------
// SPD register offsets
#define SPD_MEMORY_TYPE             2       // Memory type
#define SPD_DDR3_MODULE             3       // Module type (3:0)
#define SPD_DDR3_DENBANK            4       // Density (6:4), Banks (3:0)
#define SPD_DDR3_ADDRESS            5       // Row (5:3), Column (2:0) address
#define SPD_DDR3_VOLTAGE            6       // [2]:1 - 1.2xV operable (DDR3L), [1]:1 - 1.35V operable (DDR3U), [0]:0 - 1.5V operable (DDR3)
#define SPD_DDR3_ORG                7       // Ranks (5:3),device width (2:0)
#define SPD_DDR3_WIDTH              8       // Bus width ext (4:3) 00 - No bit ext, 01 = 8Bit(ECC) Ext, Bus width (2:0)
#define SPD_DDR3_MTBDD              10      // Medium Timebase (MTB) Dividend
#define SPD_DDR3_MTBDS              11      // Medium Timebase (MTB) Divisor
#define SPD_DDR3_TCLK               12      // Minimum cycle time (tCKmin)
#define SPD_DDR3_CLL                14      // CAS latency supported, low byte
#define SPD_DDR3_CLH                15      // CAS latency supported, high byte
#define SPD_DDR3_TAA                16      // Minimum CAS latency time (tAA)
#define SPD_DDR3_TWR                17      // Minimum write recovery time (tWR)
#define SPD_DDR3_TRCD               18      // Minimum RAS to CAS time (tRCD)
#define SPD_DDR3_TRRD               19      // Minimum RA to RA time (tRRD)
#define SPD_DDR3_TRP                20      // Minimum precharge time (tRP)
#define SPD_DDR3_TRASRC             21      // Upper nibbles for tRAS (7:4), tRC (3:0)
#define SPD_DDR3_TRAS               22      // Minimum active to precharge (tRAS)
#define SPD_DDR3_TRC                23      // Minimum active to active/refresh (tRC)
#define SPD_DDR3_TRFCL              24      // Minimum refresh recovery (tRFC), low byte
#define SPD_DDR3_TRFCH              25      // Minimum refresh recovery (tRFC), high byte
#define SPD_DDR3_TWTR               26      // Minimum internal wr to rd cmd (tWTR)
#define SPD_DDR3_TRTP               27      // Minimum internal rd to pc cmd (tRTP) 
#define SPD_DDR3_TFAWH              28      // Upper Nibble for tFAW
#define SPD_DDR3_TFAWL              29      // Minimum Four Activate Window Delay Time (tFAWmin), Least Significant Byte
#define SPD_DDR3_ADD_MAPPING        63      // Address Mapping (Odd Rank Mirror)
#define SPD_DDR3_MANUFACTURER_ID_LO     117     
#define SPD_DDR3_MANUFACTURER_ID_HI     118
#define SPD_DDR3_MANUFACTURE_LOCATION   119
#define SPD_DDR3_MANUFACTURE_DATE_LO    120
#define SPD_DDR3_MANUFACTURE_DATE_HI    121
#define SPD_DDR3_SERIAL_NUMBER_1        122 
#define SPD_DDR3_SERIAL_NUMBER_2        123 
#define SPD_DDR3_SERIAL_NUMBER_3        124
#define SPD_DDR3_SERIAL_NUMBER_4        125

// SPD DDR3 register definitions
#define SPD_DDR3_UNBUFFERED     0x02    // DDR3 Unbuffered Memory Type value
#define SPD_DDR3_SODIMM         0x03    
#define SPD_DDR3_MTYPE_MASK     0x0F    // DDR3 Memory Type mask
#define SPD_DDR3_BWE_MASK       0x18    // DDR3 Bus Width Extension mask
#define SPD_DDR3_VOL_MASK       0x07    // DDR3 Voltage Mask
//---------------------------------------------------------------------
//       SDRAM SPD equates
//---------------------------------------------------------------------
#define SPD_DDR                 0x07    // DDR Memory type (SPD byte 2)
#define SPD_DDR2                0x08    // DDR2 Memory type (SPD byte 2)
#define SPD_DDR3                0x0B    // DDR3 Memory type (SPD byte 2)

#define SPD_DDR_UNBUFFERED      0x1B    // DDR Unbuffered Memory Type Mask
#define SPD_DDR2_UNBUFFERED     0x02    // DDR2 Unbuffered Memory Type Mask
//SO_DIMM_BEGIN
#define SPD_DDR2_SODIMM         0x04    // DDR2 SO-DIMM Memory Type Mask
#define SPD_DDR3_SODIMM         0x03    // DDR3 SO-DIMM Memory Type Mask
//SO_DIMM_END
#define SPD_VAL_SDR_TYPE        4
#define SPD_VAL_DDR_TYPE        SPD_DDR
#define SPD_VAL_DDR2_TYPE       SPD_DDR2

#define SPD_REGISTERED          0x02

#define SPD_CL_2        0x04
#define SPD_CL_3        0x08
#define SPD_CL_4        0x10
#define SPD_CL_5        0x20
#define SPD_CL_6        0x40

#define SPD_DDR_200     0xA0
#define SPD_DDR_266     0x75
#define SPD_DDR_333     0x60
#define SPD_DDR_400     0x50
#define SPD_DDR_500     0x40
#define SPD_DDR_533     0x3D
#define SPD_DDR_667     0x30
#define SPD_DDR_800     0x25
#define SPD_DDR_1067    0x19
#define SPD_DDR_1333    0x15

//---------------------------------------------------------------------
//       JEDEC related IOP's
//---------------------------------------------------------------------

#define DDR3_MRS0_BA            (0)
#define DDR3_MRS0_BURST_LEN4    (BIT1)
#define DDR3_MRS0_BURST_LEN8    (0)
#define DDR3_MRS0_BURST_TYP     (BIT3)
#define DDR3_MRS0_CL_ADJ        4   // CAS Latency [A6:A4]
#define DDR3_MRS0_CL_5          (BIT4)
#define DDR3_MRS0_CL_6          (BIT5)
#define DDR3_MRS0_CL_7          (BIT5|BIT4)
#define DDR3_MRS0_CL_8          (BIT6)
#define DDR3_MRS0_CL_9          (BIT6|BIT4)
#define DDR3_MRS0_CL_10         (BIT6|BIT5)
#define DDR3_MRS0_DLL_RST       (BIT8)
#define DDR3_MRS0_WR_ADJ        9   // Write Recovery [A11:A9]
#define DDR3_MRS0_PD_MODE       (BIT12)

#define DDR3_EMRS1_BA           (1)
#define DDR3_EMRS1_DIC_40       (0)
#define DDR3_EMRS1_DIC_34       (BIT1)
#define DDR3_EMRS1_RTTNOM_0     (0)
#define DDR3_EMRS1_RTTNOM_60    (BIT2)
#define DDR3_EMRS1_RTTNOM_120   (BIT6)
#define DDR3_EMRS1_RTTNOM_40    (BIT6|BIT2)
#define DDR3_EMRS1_RTTNOM_20    (BIT9)
#define DDR3_EMRS1_RTTNOM_30    (BIT9|BIT2)
//#define DDR3_EMRS1_RTTNOM_ADJ   2   // RTT_NOM [A9,A6,A2] => [A7,A4,A0] to save smaller size as UINT8
#define DDR3_EMRS1_RTTNOM_ADJ   6   // RTT_NOM offset 6 in the struct
#define DDR3_EMRS1_RTTNOM_0_A   (DDR3_EMRS1_RTTNOM_0  >>DDR3_EMRS1_RTTNOM_ADJ)
#define DDR3_EMRS1_RTTNOM_60_A  (DDR3_EMRS1_RTTNOM_60 >>DDR3_EMRS1_RTTNOM_ADJ)
#define DDR3_EMRS1_RTTNOM_120_A (DDR3_EMRS1_RTTNOM_120>>DDR3_EMRS1_RTTNOM_ADJ)
#define DDR3_EMRS1_RTTNOM_40_A  (DDR3_EMRS1_RTTNOM_40 >>DDR3_EMRS1_RTTNOM_ADJ)
#define DDR3_EMRS1_RTTNOM_20_A  (DDR3_EMRS1_RTTNOM_20 >>DDR3_EMRS1_RTTNOM_ADJ)
#define DDR3_EMRS1_RTTNOM_30_A  (DDR3_EMRS1_RTTNOM_30 >>DDR3_EMRS1_RTTNOM_ADJ)
#define DDR3_EMRS1_WL_EN        (BIT7)
#define DDR3_EMRS1_OB_EN        (BIT12)

#define DDR3_EMRS2_BA           (2)
#define DDR3_EMRS2_CAS_WL_ADJ   3   // CAS Write Latency [A5:A3]
#define DDR3_EMRS2_RTTWR_0      (0)
#define DDR3_EMRS2_RTTWR_60     (BIT9)
#define DDR3_EMRS2_RTTWR_120    (BIT10)
#define DDR3_EMRS2_RTTWR_ADJ    9   // RTT_WR [A10:A9] => [A1:A0] to save smaller size since as UINT8
#define DDR3_EMRS2_RTTWR_0_A    (DDR3_EMRS2_RTTWR_0  >>DDR3_EMRS2_RTTWR_ADJ)
#define DDR3_EMRS2_RTTWR_60_A   (DDR3_EMRS2_RTTWR_60 >>DDR3_EMRS2_RTTWR_ADJ)
#define DDR3_EMRS2_RTTWR_120_A  (DDR3_EMRS2_RTTWR_120>>DDR3_EMRS2_RTTWR_ADJ)
#endif

