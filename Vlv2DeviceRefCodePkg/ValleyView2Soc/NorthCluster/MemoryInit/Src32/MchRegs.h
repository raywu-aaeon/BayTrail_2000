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

  MchRegs.h

Abstract:

  This file include MCH register defination.

--*/

#ifndef _MCHREGS_H_
#define _MCHREGS_H_


// DEVICE 0 (Memroy Controller Hub)
#define MC_BUS                    0x00  //Bus 0
#define MC_DEV                    0x00  //Device 0
#define MC_FUN                    0x00  //Function 0

#define MC_DEV_FUN                (MC_DEV << 3)  //Device 0, function 0

//
// NB DEV 0 Vendor and Device IDs
//
#define MC_VID                    0x8086
#define MC_DID_MASK               0xFFF0

#define B_ESMRAMC_TSEG_SZ         0x06
#define V_TSEG_SZ_1M              0x01
#define V_TSEG_SZ_2M              0x02
#define V_TSEG_SZ_8M              0x08

/* VLV Message Bus Units Port ID*/
#define VLV_UNIT_AUNIT              0x00          	/**< A Unit (IO Arbiter) */
#define VLV_UNIT_DUNIT              0x01          	/**< D Unit */
#define VLV_UNIT_DUNIT0             0x01          	/**< D Unit Channle 0*/
#define VLV_UNIT_DUNIT1             0x07          	/**< D Unit Channle 1 */

#define VLV_UNIT_TUNIT              0x02         	/**< H Unit */
#define VLV_UNIT_BUNIT				0x03		  	/**< B Unit */
#define VLV_UNIT_GUNIT              0x06          	/**< GVD Unit */
#define VLV_UNIT_FUSEEPNC	        0x11    		/*Fuse EndPoint Nort*/
#define VLV_UNIT_DDRIO              0x0C          	/**< DDR IO Unit */
#define VLV_UNIT_REUT               0x0D          	/**< REUT Unit */
#define VLV_UNIT_CCK 				0x14 			/*CCK */

//
// VLV SSA-BUnit (System Memory Arbiter)  Register Offset
//

#define	BUNIT_BBANKMASK_OFFSET  	0x08		//BUnit Bank Mask (BBANKMASK)
#define	BUNIT_BROWMASK_OFFSET  		0x09		//BUnit Row Mask (BROWMASK)
#define	BUNIT_BRANKMASK_OFFSET  	0x0A		//BUnit Rank Mask (BRANKMASK)
#define	BUNIT_BISOC_OFFSET  		0x11		//BUnit ISOC Configuration (BISOC)

#define	BUNIT_BMRCP_OFFSET  		0x20		//BUnit Memory Range Control Policy (BMRCP)
#define	BUNIT_BMRRAC_OFFSET  		0x21		//BUnit Memory Range Read Access Control (BMRRAC)
#define	BUNIT_BMRWAC_OFFSET  		0x22		//BUnit Memory Range Write Access Control (BMRWAC)
#define	BUNIT_BNOCACHE_OFFSET 		0x23		//BUnit Non-Cached Region (BNOCACHE)
#define	BUNIT_BNOCACHECTL_OFFSET  	0x24		//BUnit Non-Cached Region (BNOCACHECTL)
#define	BUNIT_BMBOUND_OFFSET  		0x25		//BMBOUND - Bunit Memory/IO Boundary Register
#define	BUNIT_BMBOUND_HI_OFFSET  	0x26		//BMBOUND HI - Bunit Memory/IO HI Boundary Register
#define	BUNIT_BECREG_OFFSET  		0x27		//BECREG - Bunit Extended Configuration Space Config
#define	BUNIT_BMISC_OFFSET   		0x28		//BMISC - Bunit Miscellaneous Configuration Register
#define	BUNIT_BSMRCP_OFFSET  		0x2B		//BUnit System Management Range Control Policy (BSMRCP)
#define	BUNIT_BSMRRAC_OFFSET  		0x2C		//BUnit SMM Range Read Access Control (BSMRRAC)
#define	BUNIT_BSMRWAC_OFFSET  		0x2D		//BUnit SMM Range Write Access Control (BSMRWAC)
#define	BUNIT_BSMMRRL_OFFSET  		0x2E		//BSMMRRL - Bunit System Management Range Register Low
#define	BUNIT_BSMMRRH_OFFSET  		0x2F		//BSMMRRH - Bunit System Management Range Register Hi
#define	BUNIT_BC0AHASHCFG_OFFSET	0x30		//BC0AHASHCFG - Channel 0 Bunit Address Hash Configuration register
#define	BUNIT_BC1AHASHCFG_OFFSET	0x31		//BC1AHASHCFG - Channel 1 Bunit Address Hash Configuration register

#define	BUNIT_BDEBUG0_OFFSET  		0x3B		//Bunit Debug Register 0 (BDEBUG0)
#define	BUNIT_BDEBUG1_OFFSET  		0x3C		//Bunit Debug Register 1 (BDEBUG1)
#define	BUNIT_BCTRL_OFFSET  		0x3D		//Bunit Control (BCTRL)
#define	BUNIT_BTHCTRL_OFFSET  		0x3E		//BUnit Throttling Control (BTHCTRL)
#define	BUNIT_BTHMASK_OFFSET  		0x3F		//Bunit Throttling Masks (BTHMASK)

#define	BUNIT_BIMR0L_OFFSET  		0x80		//BUnit Isolated Memory Region 0 Low (BIMR0L)
#define	BUNIT_BIMR0H_OFFSET  		0x81		//BUnit Isolated Memory Region 0 High (BIMR0H)
#define	BUNIT_BIMR0RAC_OFFSET  		0x82		//BUnit Isolated Memory Region 0 Read Access Control (BIMR0RAC)
#define	BUNIT_BIMR0WAC_OFFSET  		0x83		//BUnit Isolated Memory Region 0 Write Access Control (BIMR0WAC)
#define	BUNIT_BIMR1L_OFFSET  		0x84		//BUnit Isolated Memory Region 1 Low (BIMR1L)
#define	BUNIT_BIMR1H_OFFSET  		0x85		//BUnit Isolated Memory Region 1 High (BIMR1H)
#define	BUNIT_BIMR1RAC_OFFSET  		0x86		//BUnit Isolated Memory Region 1 Read Access Control (BIMR1RAC)
#define	BUNIT_BIMR1WAC_OFFSET  		0x87		//BUnit Isolated Memory Region 1 Write Access Control (BIMR1WAC)
#define	BUNIT_BIMR2L_OFFSET  		0x88		//BUnit Isolated Memory Region 2 Low (BIMR2L)
#define	BUNIT_BIMR2H_OFFSET  		0x89		//BUnit Isolated Memory Region 2 High (BIMR2H)
#define	BUNIT_BIMR2RAC_OFFSET  		0x8A		//BUnit Isolated Memory Region 2 Read Access Control (BIMR2RAC)
#define	BUNIT_BIMR2WAC_OFFSET  		0x8B		//BUnit Isolated Memory Region 2 Write Access Control (BIMR2WAC)
#define	BUNIT_BIMR3L_OFFSET  		0x8C		//BUnit Isolated Memory Region 3 Low (BIMR3L)
#define	BUNIT_BIMR3H_OFFSET  		0x8D		//BUnit Isolated Memory Region 3 High (BIMR3H)
#define	BUNIT_BIMR3RAC_OFFSET  		0x8E		//BUnit Isolated Memory Region 3 Read Access Control (BIMR3RAC)
#define	BUNIT_BIMR3WAC_OFFSET  		0x8F		//BUnit Isolated Memory Region 3 Write Access Control (BIMR3WAC)

#define DDRIO_DQ0_RK2RKCTL_OFFSET               0xA8
#define DDRIO_CMD_CFG_REG0_OFFSET				0x4840
#define DDRIO_CLKCTL_CMD_CFG_REG2_OFFSET        0x58B8


//VLV DUnit Register Symbol           	Register Start
#define MC_DRP_OFFSET                   0x0
#define MC_DTR0_OFFSET                  0x1
#define MC_DTR1_OFFSET                  0x2
#define MC_DTR2_OFFSET                  0x3
#define MC_DTR3_OFFSET                  0x4
#define MC_DTR4_OFFSET                  0x5
#define MC_DPMC0_OFFSET                 0x6
#define MC_DPMC1_OFFSET                 0x7
#define MC_DRFC_OFFSET                  0x8
#define MC_DSCH_OFFSET                  0x9
#define MC_DCAL_OFFSET                  0xA
#define MC_DRMC_OFFSET                  0xB
#define MC_PMSTS_OFFSET                 0xC
#define MC_DCO_OFFSET                   0xF
#define MC_DTRC_OFFSET                  0x10

#define MC_SSKPD0_OFFSET                0x4A
#define MC_SSKPD1_OFFSET                0x4B
#define MC_BONUS0_OFFSET                0x50
#define MC_BONUS1_OFFSET                0x51
#define MC_DECCCTRL_OFFSET              0x60
#define MC_DFUSESTAT_OFFSET             0x70
#define MC_SCRMSEED_OFFSET              0x80
#define MC_SCRMLO_OFFSET                0x81
#define MC_SCRMHI_OFFSET                0x82

/*
 * Register Field Definitions
 *
 */

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 rank0Enabled       :1;             /**< BIT [0]   Rank 0 Enable */
        UINT32 rank1Enabled       :1;             /**< BIT [1]   Rank 1 Enable */
        UINT32 rank2Enabled       :1;             /**< BIT [2]   Rank 2 Enable */
        UINT32 rank3Enabled       :1;             /**< BIT [3]   Rank 3 Enable */
        UINT32 dimm0DevWidth      :2;             /**< BIT [5:4] DIMM 0 Device Width (Rank0&1)  */
        UINT32 dimm0DevDensity    :2;             /**< BIT [7:6] DIMM 0 Device Density          */
        UINT32 reserved1          :1;
        UINT32 dimm1DevWidth      :2;             /**< BIT [10:9]  DIMM 1 Device Width (Rank2&3)  */
        UINT32 dimm1DevDensity    :2;             /**< BIT [12:11] DIMM 1 Device Density          */
        UINT32 reserved2          :1;
        UINT32 RSIEN              :1;             /**< BIT [14] Rank Select Interleaving Enable */
        UINT32 reserved3          :1;
        UINT32 dimmFlip           :1;
        UINT32 RANKREMAP          :1;             /**< BIT [17] Rank Remap */
        UINT32 CKECOPY            :1;             /**< BIT[18] CKE copy */
        UINT32 reserved4          :1;
        UINT32 dimm0Mirror        :1;
        UINT32 dimm1Mirror        :1;
        UINT32 DRAMtype           :1;               /**< BIT [22] */
        UINT32 EnLPDDR3           :1;
        UINT32 reserved5          :8;
    } field;
} RegDRP;                                           /**< DRAM Rank Population and Interface Register */
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 dramFrequency      :2;             /**< DRAM Frequency (000=800,001=1033,010=1333) */
        UINT32 reserved1          :2;
        UINT32 tRP                :4;             /**< bit [7:4]   Precharge to Activate Delay  */
        UINT32 tRCD               :4;             /**< bit [11:8]  Activate to CAS Delay  */
        UINT32 tCL                :3;             /**< bit [14:12] CAS Latency  */
        UINT32 reserved4          :1;
        UINT32 tXS                :1;             /**< SRX Delay  */
        UINT32 reserved5          :1;
        UINT32 tXSDLL             :1;             /**< SRX To DLL Delay  */
        UINT32 reserved6          :1;
        UINT32 tZQCS              :1;             /**< bit [20] ZQTS recovery Latncy  */
        UINT32 reserved7          :1;
        UINT32 tZQCL              :1;             /**< bit [22] ZQCL recovery Latncy  */
        UINT32 reserved8          :1;
        UINT32 PMEDLY             :2;             /**< bit [25:24] Power mode entry delay  */
        UINT32 reserved9          :2;
        UINT32 CKEDLY             :4;               /**< bit [31:28]  */
    } field;
} RegDTR0;                                          /**< DRAM Timing Register 0 */
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 tWCL               :3;             /**< bit [2:0] CAS Write Latency */
        UINT32 reserved1          :1;
        UINT32 tCMD               :2;             /**< bit [5:4] Command transport duration */
        UINT32 reserved2          :2;
        UINT32 tWTP               :4;             /**< Write to Precharge */
        UINT32 tCCD               :2;             /**< CAS to CAS delay */
        UINT32 reserved4          :2;
        UINT32 tFAW               :4;             /**< Four bank Activation Window*/
        UINT32 tRAS               :4;             /**< Row Activation Period: */
        UINT32 tRRD               :2;             /**<Row activation to Row activation Delay */
        UINT32 reserved5          :2;
        UINT32 tRTP               :3;             /**<Read to Precharge Delay */
        UINT32 reserved6          :1;
    } field;
} RegDTR1;                                          /**< DRAM Timing Register 1 */
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 tRRDR              :3;             /**< RD to RD from different ranks, same DIMM */
        UINT32 reserved1          :1;
        UINT32 tRRDD              :3;             /**< RD to RD from different DIMM */
        UINT32 reserved2          :1;
        UINT32 tWWDR              :3;             /**< WR to WR from different ranks, same DIMM. */
        UINT32 reserved3          :1;
        UINT32 tWWDD              :3;             /**< WR to WR from different DIMMs. */
        UINT32 reserved4          :1;
        UINT32 tRWDR              :4;             /**< bit [19:16] RD to WR from different ranks, same DIMM. */
        UINT32 reserved5          :1;
        UINT32 tRWDD              :4;             /**< bit [24:21] RD to WR from different DIMM. */
        UINT32 reserved6          :7;
    } field;
} RegDTR2;                                          /**< DRAM Timing Register 1 */
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 tWRDR              :3;             /**< WR to RD from different ranks, same DIMM. */
        UINT32 reserved1          :1;
        UINT32 tWRDD              :3;             /**< WR to RD from different DIMM. */
        UINT32 reserved2          :1;
        UINT32 tRWSR              :4;             /**< RD to WR Same Rank. */
        UINT32 reserved3          :1;
        UINT32 tWRSR              :4;             /**< bit [16:13] WR to RD Same Rank. */
        UINT32 reserved4          :5;
        UINT32 tXP                :2;             /**< bit [23:21] Time from CKE set on to any command. */
        UINT32 PWD_DLY            :4;             /**< bit [27:24] Extended Power-Down Delay. */
        UINT32 EnDeRate           :1;             /**< bit [28]  */
        UINT32 DeRateOvr          :1;             /**< bit [29] */
        UINT32 DeRateStat         :1;             /**< bit [30] */
        UINT32 reserved5          :1;
    } field;
} RegDTR3;                                          /**< DRAM Timing Register 3 */
#pragma pack()


#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 WRODTSTRT          :2;             /**< RD to RD from different ranks, same DIMM */
        UINT32 reserved1          :2;             /**< RD to RD from different DIMM */
        UINT32 WRODTSTOP          :3;             /**< WR to WR from different ranks, same DIMM. */
        UINT32 reserved2          :1;             /**< WR to WR from different DIMMs. */
        UINT32 RDODTSTRT          :3;             /**< RD to WR from different ranks, same DIMM. */
        UINT32 reserved3          :1;             /**< RD to WR from different DIMM. */
        UINT32 RDODTSTOP          :3;             /**< WR to RD from different ranks, same DIMM. */
        UINT32 reserved4          :1;             /**< WR to RD from different DIMM. */
        UINT32 TRGSTRDIS          :1;             /**< RD to WR Same Rank. */
        UINT32 RDODTDIS           :1;
        UINT32 WRBODTDIS          :1;
        UINT32 reserved5          :13;
    } field;
} RegDTR4;                                          /**< DRAM Timing Register 3 */
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 SREntryDelay       :8;             /**< Self-Refresh Entry Delay: */
        UINT32 PMOPCODE    		  :5;             /**< SPID Power Mode Opcode */
        UINT32 reserved1          :3;
        UINT32 PCLSTO             :3;             /**< Page Close Timeout Period */
        UINT32 reserved2          :1;
        UINT32 PCLSWKOK           :1;             /**< Wake Allowed For Page Close Timeout */
        UINT32 PREAPWDEN          :1;             /**< Send Precharge All to rank before entering Power-Down mode. */
        UINT32 reserved3          :1;
        UINT32 DYNSREN            :1;             /**< bit [23] Dynamic Self-Refresh */
        UINT32 CLKGTDIS           :1;             /**< bit [24] Clock Gating Disabled*/
        UINT32 DISPWRDN           :1;             /**< bit [25] Disable Power Down*/
        UINT32 BLMODE             :1;             /**< bit [26] Selects the Burst Length mode*/
        UINT32 PWRGATEDIS         :1;             /**< bit [27]  */
        UINT32 REUTCLKGTDIS       :1;             /**< bit [28]  */
        UINT32 ENPHYCLKGATE       :1;             /**< bit [29] */
        UINT32 ENCKTRI            :1;             /**< bit [30] */
        UINT32 ENCORECLKGATE      :1;             /**< bit [31] */
    } field;
} RegDPMC0;                                           /**< DRAM Power Management Control Register 0 */
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 CSTRIST            :1;
        UINT32 reserved1          :3;
        UINT32 CMDTRIST           :2;              /**< Tristate COmmand & Address */
        UINT32 reserved2          :26;
    } field;
} RegDPMC1;                                           /**< DRAM Power Management Control Register 1*/
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 REFWMLO            :4;             /**< Refresh Opportunistic Watermark */
        UINT32 REFWMHI            :4;             /**< Refresh High Watermark*/
        UINT32 REFWMPNC           :4;             /**< Refresh Panic Watermark */
        UINT32 tREFI              :3;             /**< bit [14:12] Refresh Period */
        UINT32 reserved1          :1;
        UINT32 REFCNTMAX          :2;             /**< Refresh Max tREFI Interval */
        UINT32 reserved2          :2;
        UINT32 REFSKEWDIS         :1;             /**< tREFI counters */
        UINT32 REFDBTCLR          :1;
        UINT32 reserved3          :2;
        UINT32 CuRefRate          :3;             /**< bit [26:24] */
        UINT32 reserved4          :5;
    } field;
} RegDRFC;                                           /**< DRAM Refresh Control Register*/
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 OOOAGETRH          :5;             /**< Out-of-Order Aging Threshold */
        UINT32 reserved1          :3;
        UINT32 OOODIS             :1;             /**< Out-of-Order Disable */
        UINT32 OOOST3DIS          :1;             /**< Out-of-Order Disabled when RequestBD_Status is 3. */
        UINT32 reserved2          :2;
        UINT32 NEWBYPDIS          :1;
        UINT32 reserved3          :3;
        UINT32 IPREQMAX           :3;             /** < Max In-Progress Requests stored in MC */
        UINT32 reserved4          :13;
    } field;
} RegDSCH;                                           /**< DRAM Scheduler Control Register */
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 reserved1          :8;
        UINT32 ZQCINT             :3;             /**< ZQ Calibration Short Interval: */
        UINT32 reserved2          :1;
        UINT32 SRXZQCL            :2;             /** < ZQ Calibration Length */
        UINT32 ZQCalType          :1;
        UINT32 ZQCalStart         :1;
        UINT32 TQPollStart        :1;
        UINT32 TQPollRS           :2;
        UINT32 reserved3          :1;
        UINT32 TQPollEn           :1;
        UINT32 TQPollPer          :3;
        UINT32 MRRData            :8;               /**< bit[31:24] */
    } field;
} RegDCAL;                                          /**< DRAM Calibration Control*/
#pragma pack()


#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 CKEVAL             :4;             //< CKE[3:0]
        UINT32 CKEMODE            :1;             //< CKE pin overide
        UINT32 reserved1          :3;             //< CKE Value
        UINT32 ODTVAL             :4;             //< ODTVAL[3:0]
        UINT32 ODTMODE            :1;             //< ODT pin override
        UINT32 reserved2          :3;
        UINT32 COLDWAKE           :1;
        UINT32 reserved3          :15;
    } field;
} RegDRMC;                                           /**< DRAM Power Management Control Register 1*/
#pragma pack()


#define REFRESH_DISABLED    0
/*** INTEL ONLY - BEGIN ***/
#define REFRESH_128_CLOCKS  1
/*** INTEL ONLY - END ***/
#define REFRESH_3_9_US      2
#define REFRESH_7_8_US      3

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 DRPLOCK            :1;             /**<DRP lock bit */
        UINT32 reserved1          :7;
        UINT32 REUTLOCK           :1;             /**<REUT lock bit*/
        UINT32 reserved2          :19;
        UINT32 PMICTL             :1;             /**< PRI Control Select: */
        UINT32 PMIDIS             :1;
        UINT32 DIOIC              :1;             /** < DDRIO initialization is complete */
        UINT32 IC                 :1;             /**< D-unit Initialization Complete */
    } field;
} RegDCO;                                           /**< DRAM Controller Operation Register*/
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 BLKRD              :1;             /**< bit[0] block read */
        UINT32 BLKWR              :1;             /**< bit[1] block write */
        UINT32 BLKACT             :1;             /**< bit[2] block activate */
        UINT32 BLKPRE             :1;             /**< bit[3] block precharge */
        UINT32 BLKIPRQNF          :1;             /**< bit[4] Block IPREQ Until Full */
        UINT32 BLKPHASEB          :1;             /**< bit[5] */
        UINT32 reserved1          :2;
        UINT32 CATSTART           :1;             /**< bit[8] CA training start */
        UINT32 CATRS              :2;             /**< bit[10:9] CA training Rank select */
        UINT32 reserved2          :1;
        UINT32 CATLPCNT           :2;             /**< bit[13:12] loop count */
        UINT32 CATMASK            :2;             /**< bit[15:14] CA training mask */
        UINT32 CATFAIL            :1;             /**< bit[16] CA training fail */
        UINT32 CATSTATUS          :6;             /**< bit[22:17] CA training status */
        UINT32 reserved3          :9;
    } field;
} RegDTRC;                                          /**< DRAM Training Control*/
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 SCRMSEED           :18;
        UINT32 reserved3          :13;
        UINT32 SCRMDIS            :1;
    } field;
} RegSCRMSEED;                                     /**< Dynamic Data Scrambler Seed Register*/
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 DISEARLYSRX        :1;
        UINT32 PERFMONEN          :1;
        UINT32 SLOWPDXEN          :1;
        UINT32 CATBUGFIX          :1;
        UINT32 MRRCMDDLY          :2;
        UINT32 LPDDRCMDTRI        :1;
        UINT32 reserved0          :25;
    } field;
} RegBONUS0;                                     /**< Dynamic Data Scrambler Seed Register*/
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32 DramAddressHashEn  :1;             /**< */
        UINT32 RankInterleaveEn   :1;             /**< */
        UINT32 MemoryIntWidth     :1;             /**< */
        UINT32 rank0Enabled       :1;             /**< */
        UINT32 rank1Enabled       :1;             /**< */
        UINT32 rank2Enabled       :1;             /**< */
        UINT32 rank3Enabled       :1;
        UINT32 dimm0PartWidth     :2;             /**< */
        UINT32 dimm0Rank0Size     :3;             /**< */
        UINT32 reserved2          :1;
        UINT32 dimm1PartWidth     :2;             /**< */
        UINT32 dimm1Rank2Size     :3;             /**< */
        UINT32 reserved3          :14;
    } field;
} RegBunitAHASHCFG;                                 /**< Address Hash Configuration */
#pragma pack()

/*
 * DRAM Initialization Structures used in JEDEC Message Bus Commands
 *
 */

#pragma pack(1)
typedef union {
        UINT32      raw;
    struct {
        unsigned    command         :3;             /**< Command: 000-MRS,001-Refresh,010-Pre-charge,011-Activate,110-ZQ,111-NOP */
        unsigned    bankAddress     :3;             /**< Bank Address (BA[2:0]) */
        unsigned    BL              :2;             /**< Burst Length, CDV:1*/
        unsigned    CL              :1;             /**< CL Reserved CDV:0 */
        unsigned    RBT             :1;             /**< Read Burst Type */
        unsigned    casLatency      :3;             /**< cas Latency */
        unsigned    TM              :1;             /**< Test mode */
        unsigned    dllReset        :1;             /**< DLL Reset */
        unsigned    writeRecovery   :3;             /**< Write Recovery for Auto Pre-Charge: 001=2,010=3,011=4,100=5,101=6 */
        unsigned    PPD             :1;             /**< DLL Control for Precharge Power-Down CDV:1 */
        unsigned    reserved1       :3;
        unsigned    rankSelect      :4;             /**< Rank Select */
        unsigned    reserved2       :6;
    } field;
} DramInitDDR3MRS0;                                 /**< DDR3 Mode Register Set (MRS) Command */
#pragma pack()

#pragma pack(1)
typedef union {
        UINT32      raw;
    struct {
        unsigned    command         :3;             /**< Command: 000-MRS,001-Refresh,010-Pre-charge,011-Activate,110-ZQ,111-NOP */
        unsigned    bankAddress     :3;             /**< Bank Address (BA[2:0]) */
        unsigned    dllEnabled      :1;             /**< CDV=0 */
        unsigned    DIC0            :1;             /**< Output Driver Impedance Control */
        unsigned    rttNom0         :1;             /**< RTT_nom[0] */
        unsigned    MRC_AL          :2;             /**< Additive Latency = 0 */
        unsigned    DIC1            :1;             /**< Reserved */
        unsigned    rttNom1         :1;             /**< RTT_nom[1] */
        unsigned    wlEnabled       :1;             /**< Write Leveling Enable */
        unsigned    reserved1       :1;
        unsigned    rttNom2         :1;             /** < RTT_nom[2] */
        unsigned    reserved2       :1;
        unsigned    TDQS            :1;             /**< TDQS Enable */
        unsigned    Qoff            :1;             /**< Output Buffers Disabled */
        unsigned    reserved3       :3;
        unsigned    rankSelect      :4;             /**< Rank Select */
        unsigned    reserved4       :6;
    } field;
} DramInitDDR3EMR1;                                 /**< DDR3 Extended Mode Register 1 Set (EMRS1) Command */
#pragma pack()

#pragma pack(1)
typedef union {
        UINT32      raw;
    struct {
        UINT32    command         :3;             /**< Command: 000-MRS,001-Refresh,010-Pre-charge,011-Activate,110-ZQ,111-NOP */
        UINT32    bankAddress     :3;             /**< Bank Address (BA[2:0]) */
        UINT32    PASR            :3;             /**< Partial Array Self-Refresh */
        UINT32    CWL             :3;             /**< CAS Write Latency */
        UINT32    ASR             :1;             /**< Auto Self-Refresh */
        UINT32    SRT             :1;             /**< SR Temperature Range = 0*/
        UINT32    reserved1       :1;
        UINT32    rtt_WR          :2;             /**< Rtt_WR */
        UINT32    reserved2       :5;
        UINT32    rankSelect      :4;             /**< Rank Select */
        UINT32    reserved3       :6;
    } field;
} DramInitDDR3EMR2;                                 /**< DDR3 Extended Mode Register 2 Set (EMRS2) Command */
#pragma pack()

#pragma pack(1)
typedef union {
        UINT32      raw;
    struct {
        UINT32    command         :3;             /**< Command: 000-MRS,001-Refresh,010-Pre-charge,011-Activate,110-ZQ,111-NOP */
        UINT32    bankAddress     :3;             /**< Bank Address (BA[2:0]) */
        UINT32    MPR_Location    :2;             /**< MPR Location */
        UINT32    MPR             :1;             /**< MPR: Multi Purpose Register */
        UINT32    reserved1       :13;
        UINT32    rankSelect      :4;             /**< Rank Select */
        UINT32    reserved2       :6;
    } field;
} DramInitDDR3EMR3;                                 /**< DDR3 Extended Mode Register 2 Set (EMRS2) Command */
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32    command         :3;             /**< Command: 000-MRS,001-Refresh,010-Pre-charge,011-Activate,110 ?ZQ Calibration,111-NOP */
        UINT32    bankAddress     :3;             /**< Bank Address (BA[2:0]) */
        UINT32    multAddress     :16;            /**< Multiplexed Address (MA[14:0]) */
        UINT32    rankSelect      :2;             /**< Rank Select */
        UINT32    reserved3       :8;
    } field;
} DramInitMisc;                                     /**< Miscellaneous DDRx Initialization Command */
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32    RANKPICKMASK            :8;
        UINT32    EnableBWCounterUpdates  :1;
        UINT32    CountCoreClocks         :1;
        UINT32    ClearBWCounterOnReads   :1;
        UINT32    reserved1               :5;
        UINT32    AgentThrottleEnable     :16;
    } field;
} RegBTHCTRL;                                     /**< Bunit Throttling Control */
#pragma pack()

#pragma pack(1)
typedef union {
    UINT32    raw;
    struct {
        UINT32    SBEEN            			:1;
        UINT32    DBEEN						:1;
        UINT32    CBOEN         			:3;
        UINT32    SYNSEL   					:2;
        UINT32    CLRSBECNT               	:1;
        UINT32    CBOV     					:8;
        UINT32    reserved1     			:1;
        UINT32    ENCBGEN     				:1;
        UINT32    reserved2     			:14;
    } field;
} RegDECCCTRL;                                     /**< Bunit DECC Control Register */
#pragma pack()

#endif
