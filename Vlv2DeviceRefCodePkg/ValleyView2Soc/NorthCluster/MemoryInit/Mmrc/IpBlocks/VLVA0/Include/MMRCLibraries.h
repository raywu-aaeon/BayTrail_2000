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

  MMRCLibraries.h

Abstract:

  Library specific macros and function declarations used within the MMRC.

--*/
#ifndef _MMRCLIBRARIES_H_
#define _MMRCLIBRARIES_H_
#include "Types.h"
#include "MMRCProjectDefinitions.h"

#define PHYINIT 1

//late CA train
#define MAX_CMDS    2
#define LOOKING_FOR_FAILURE 0
#define FOUND_FAILURE       1
#define CMDLOWHIGH_NONE   0
#define CMDLOWHIGH_LOW    1
#define CMDLOWHIGH_HIGH   2
#define CMDLOWHIGH_BOTH   3
#define MIN_RANK_BIT                 30
#define MAX_DUNIT_ROW_ADDRESS_BIT   (MIN_RANK_BIT - 1)
#define OFFSET_HIGH_LOW_SHIFT       (MAX_DUNIT_ROW_ADDRESS_BIT - 6 - 15)
#define LATECMD_STEPSIZE  1
#define LATECMD_SWEEP_RANGE		3
#define RMT_CMD_SWEEP_RANGE		2

#define UPM_PERCENT_LATECMD    10     // Percentage of Unit Interval (LPDDR3: HALF_CLK)
#define PERFORMANCE       0

#define MAX_2XCLK     15    // determined by the bitend-Bitstart of assigned register.
#define DELAY_ARRAY     CONV2(0x0000)
#define DB_LATE       45    //55=BERRYVILLE
#define DB_EARLY      15    //12=BERRYVILLE
#define RCVEN_NUM_SAMPLES   3
#define RCVEN_SAMPLE_DLY    32
//BIT 0 indicate DELAY
#define SET                       2
#define SET_DL                    1
#define POLL                      4
#define CHECK					  10
#define DELAY                     3
#define Printf               printf
#define DIMM_U8     Rank_pu8/2
#define DIMM(x)     (x/2)
#define Rank(x)     (x%2)

#define NOT_MET 0     // move to MMRCLibraries.h
#define MET 1
#define MET2 2
#define UP 1
#define DN 0

#define COUNT_UP_FOR_ONE    1
#define COUNT_DN_FOR_ZERO   0
#define FINE_FOUND        2

#define SKIP_ACTION             0
#define TAKE_ACTION             1
#define PFCT_GLOBAL             0

#define LOC_FLAGS_U(x) (((x[6])>>5) & 1)

#define CHANNELRank_ENABLE(enable, tst)                       (((enable>>tst) & 1)  == 1)

#define RDWR_NOT_DONE                  0
#define RDWR_DONE                      1
#define RDWR_LOW                       0
#define RDWR_HIGH                      1

#define ROUND_UP(x, y)  ((x + (y/2))/y)
#define PERCENT_MAX_MARGIN(x)        (ROUND_UP(4*x, 10))
#define EW_PERCENTAGE_VREF(x)        (ROUND_UP(9*x, 10))
#define WEIGHT_CONDITION_VREF(x)     (ROUND_UP(4*x, 10))
#define EW_PERCENTAGE_DELAY(x)       (ROUND_UP(9*x, 10))
#define WEIGHT_CONDITION_DELAY(x)    (ROUND_UP(5*x, 10))


typedef enum {
  ChNone = 0xFE,
  CH_NONE = 0xFE,
  ChAll = 0xFF,
  CH_ALL = 0xFF
}CHANNEL_DEFINATION;

typedef enum {
  RECEIVE_ENABLE,
  FINE_WRITE_LEVELING,
  COARSE_WRITE_LEVELING,
  READ_VREF,
  READ_DELAY,
  WRITE_DELAY,
  WRITE_VREF,
  CMD_DELAY
} MarginType;


#define RTTNOM_DIS      0x0000
#define RTTNOM_60ohm    0x0004
#define RTTNOM_120ohm   0x0040
#define RTTNOM_40ohm    0x0044
#define RTTNOM_20ohm    0x0200
#define RTTNOM_30ohm    0x0204

#define OUTIMP_40ohm    0x0000
#define OUTIMP_34ohm    0x0002

#define WRITELEVEL_ENABLE 0x0080

#define JEDEC_QBUF_DIS 0x1000
#define JEDEC_MR1      0x10000

#define DELAYREGS_WDQS    0
#define DELAYREGS_RCVN    1
#define DELAYREGS_WDQ   2
#define DELAYREGS_WCMD    3
#define DELAYREGS_WCTL    4
#define DELAYREGS_WCLK    5
#define DELAYREGS_RDQS    6

#define EVEN_MODE               0x01
#define ODD_MODE                0x00

#define LOW                     0x00
#define HIGH                    0x01


#define SAMPLE_HIGH       1
#define SAMPLE_LOW        0
#define RESET_DISABLE     1
#define RESET_ENABLE      0
#define COUNT_UP_FOR_ONE  1
#define COUNT_DN_FOR_ZERO 0
#define FINE_FOUND        2


#define TOTAL_BL_PERRANK    MAX_DQ_MODULES * MAX_BYTELANES_PER_DQ_MODULE
#define TOTAL_BL      MAX_CHANNELS*MAX_RANKS*MAX_DQ_MODULES*MAX_BYTELANES_PER_DQ_MODULE

// RD/WR Levelling
#define RD_LEVELING     1
#define WR_LEVELING     2

// 
// Definition of the HALF/QTR/ONE Clock lengths.
// These are all frequency dependent.
#define HALF_CLK_(DigitalDllEn, FreqIndex)   HalfClk[DigitalDllEn][FreqIndex]
#define QTR_CLK_(DigitalDllEn, FreqIndex)    HalfClk[DigitalDllEn][FreqIndex] / 2
#define ONE_CLK_(DigitalDllEn, FreqIndex)    HalfClk[DigitalDllEn][FreqIndex] *2
#define MAXPI_VAL_(DigitalDllEn, FreqIndex)  HalfClk[DigitalDllEn][FreqIndex]-1
#define ONE_CLK   128
#define HALF_CLK  64
#define QTR_CLK   32
#define MAXPI_VAL 63
// Macros for converting Little-Endian to Big-Endian.
#define CONV1(x)                (x)
#define CONV2(x)                ((x>>0)&0xff), ((x>>8)&0xff)
#define CONV3(x)                ((x>>0)&0xff), ((x>>8)&0xff), ((x>>16)&0xff)
#define CONV4(x)                ((x>>0)&0xff), ((x>>8)&0xff), ((x>>16)&0xff), ((x>>24)& 0xff)

// Macros for MASK/VAL which are used in the projectlibraries.c declarations.
#define MASK(x)                 CONV4(x)
#define VAL(x)                  CONV4(x)
#define Rank_U8                   Rank_pu8%2
#define eRCVN                     1
#define eWDQ                      2
#define eWDQS                     7
#define eRCVNM1                   3
#define eRCVNSMP                  4
#define eWDQSM1                   5
#define eWDQM1                    6
#define RDLEVEL              1
#define eWRLVL                    8
#define eWCLK                     9
#define eWDQSSMP                  10
#define eFIFORST                  11
#define eRDQS                     12
#define eRDVREF                   13

#define ASSIGNDONE          (0xff)
#define DETREGLIST_NULL          ((DETAILED_REGISTER_STRING *) 0xFFFFFFFF)
#define REGLIST_NULL             ((REGISTER_STRING *)    0xFFFFFFFF)

// This is the first byte in the assignment which provides the libraries the Type and amount of conditional values.
#define xxxxx                   0x00
#define xxxxT                   0x01
#define xxxCx                   0x02
#define xxxCT                   0x03
#define xxFxx                   0x04
#define xxFxT                   0x05
#define xxFCx                   0x06
#define xxFCT                   0x07
#define xPxxx                   0x08
#define xPxxT                   0x09
#define xPxCx                   0x0a
#define xPxCT                   0x0b
#define xPFxx                   0x0c
#define xPFxT                   0x0d
#define xPFCx                   0x0e
#define xPFCT                   0x0f
#define dxxxx                   0x80
#define dxxxT                   0x81
#define dxxCx                   0x82
#define dxxCT                   0x83
#define dxFxx                   0x84
#define dxFxT                   0x85
#define dxFCx                   0x86
#define dxFCT                   0x87
#define dPxxx                   0x88
#define dPxxT                   0x89
#define dPxCx                   0x8a
#define dPxCT                   0x8b
#define dPFxx                   0x8c
#define dPFxT                   0x8d
#define dPFCx                   0x8e
#define dPFCT                   0x8f

// The Macro defintions for the CMRB flags which are part of the location
// assignment and is always 2 bytes in size.
//
// [15]   - C
// [14]   - M
// [11:8] - R
// [3:0]  - B
// [13]   - U


#define xxxx_x         CONV2(0x0)
#define xMxx_x         CONV2(0x4000)
#define Cxxx_x         CONV2(0x8000)
#define CMxx_x         CONV2(0xC000)
#define CM1x_x         CONV2(0xC200)

#define MILLI_DEL 0
#define MICRO_DEL 1
#define NANO_DEL 2

#define CMD_GET   18
#define CMD_SET   16

#define RD_REG      0x01
#define RD_ONLY     0x02
#define WR_OFF      0x04
#define FC_WR       0x08
#define UPD_CACHE   0x10
#define FC_WR_PRINT 0x20
#define FC_PRINT    0x40

//Read Value from cache
#define CMD_GET_CACHE     (RD_ONLY)
//Read from register
#define CMD_GET_REG     (RD_ONLY|RD_REG)
//Read from register and update Value to cache
#define CMD_GET_REG_UC      (RD_ONLY|RD_REG|UPD_CACHE)
//Write to register with offset to Cache Value, apply Value condition checking and update final Value to cache
#define CMD_SET_OFFCAC_UC   (WR_OFF|UPD_CACHE)
//Write to register with offset to Reg Value, apply Value condition checking and update final Value to cache
#define CMD_SET_OFFREG_UC   (WR_OFF|RD_REG|UPD_CACHE)
//Write to register with offset to Cache Value, force write with NO Value condition checking and update final Value to cache
#define CMD_SET_OFFCAC_FC_UC  (WR_OFF|FC_WR|UPD_CACHE)
//Write to register with offset to Reg Value, force write with NO Value condition checking and update final Value to cache
#define CMD_SET_OFFREG_FC_UC  (WR_OFF|FC_WR|RD_REG|UPD_CACHE)
//Write to register with input Value, apply Value condition checking and update final Value to cache
#define CMD_SET_VAL_UC      (UPD_CACHE)
//Write to register with input Value, apply Value condition checking
#define CMD_SET_VAL     0
//Write to register with input Value, force write with NO Value condition checking and update final Value to cache
#define CMD_SET_VAL_FC_UC   (FC_WR|UPD_CACHE)
//Write to register with input Value, force write with NO Value condition checking
#define CMD_SET_VAL_FC      (FC_WR)

#define  STATUS_PASS  0
#define  STATUS_FAIL  1
#define  STATUS_TYPE_NOT_SUPPORTED  2
#define  STATUS_CMD_NOT_SUPPORTED 3
#define  STATUS_LIMIT 4

enum CPFC_STATUS {
  CPGC_STS_SUCCESS = 0,
  CPGC_STS_FAILURE,
  CPGC_STS_TEST_BUSY,
  CPGC_STS_TEST_DONE,
  CPGC_STS_TEST_ERROR,
  CPGC_STS_NOT_SUPPORTED
};

typedef struct delaysrange_s {
  UINT16     range_min;
  UINT16     range_max;
} DELAY_RANGE;

typedef struct ccrange_s {
  UINT16      range_min;
  UINT16      range_max;
  UINT16      range_in;
  UINT16      range_out;
  const UINT8 *delay_element_ptr;
} CC_RANGE;

typedef struct {
  const UINT8 *del_el;
  const UINT8 *del_cc;
  const UINT8 *del_minus1;
  const UINT8 *del_sample;
  const UINT8 *del_onex;
  UINT8       SigSt;
  UINT8       SigCt;
  UINT8       CCSt;
  UINT8       CCCnt;
  UINT8       Minus1Ind;
  UINT8       SMPInd;
  UINT8       OneXInd;
} DELAY_SIGNAL_GROUP;

typedef struct {
  UINT16  offset;   // Register Offset relative to the DDRPHy.
  UINT32  mask;     // Starting bit within the register.
  UINT8   sb;
} IOSF_REG;

//
// ClockCrossing structure provides when the specific clock crossing will change
// based on the linear PI values.  The range is specified in (%) of the PI range
// since the PI range is dynamic based on analog/digitial dll.
//
typedef struct {
  UINT16      MinPercent;       // Minimum PI value for InValue to be programmed.
  UINT16      MaxPercent;       // Maximum PI value for InValue to be programmed.
  UINT16      InValue;          // In Range Value.
  UINT16      OutValue;         // Out of Range Value.
} ClockCrossings;

typedef struct {
  UINT8  valid;
  IOSF_REG  data[MAX_RANKS][MAX_BYTELANES_PER_DQ_MODULE*MAX_DQ_MODULES];
} cachedLocations;

// 
// Elements is the main structure for identifying the location for an individual register
// assignment with the Get/Set API.
//
typedef struct {
  UINT16 Offset;              // Offset to the Initial Register Location.
  UINT8  StartingBit;         // Starting bit position within the register.
  UINT8  EndingBit;           // Ending bit position within the register.
  UINT8  ChannelEnable:1;     // Chn enable bit, when set, Chn selection affecds offset.
  UINT8  ModuleEnable:1;      // Mod enable bit, when set, Mod selection affects offset.
  INT8  UniqueIndex;       	  // When not -1, Rank positions are in another array.
  INT8  CacheIndex;          // When not -1, the value is cached within cache structure.
  UINT8  RankByteOffset;       // Rank-to-Rank byte offset.
  UINT8  RankBitOffset;       // Rank-to-Rank bit offset.
  INT8  SPMByteOffset;        // Strobe-to-Strobe Per Module Byte offset.
  INT8  SPMBItOffset;        // Sttobe-to-Strboe Per Module Bit offset.
} SetGetElements;


//
// When specifying a unique location, the elements required are 
// offset/starting/ending bits.
//
typedef struct {
  UINT16 Offset;              // Byte offset for the specific rank. 
  UINT8 StartingBit;          // Starting bit position for the specific rank.
  UINT8 EndingBit;            // Ending bit position for the specific rank.
} UniqueRank;

//
// The Unique Ranks structure.
// The uniqueRank will contain MAX_RANKS - 1 unique locations as RANK[0] will be specified
// in the main element structure.
//
typedef struct {
  UniqueRank rank[MAX_RANKS-1]; // Rank location.
} UniqueRanks;

// 
// The structure definition for the floorplan which is given a channel and strobelane, to provide a physical
// channel and strobelane.
//
typedef struct {
  UINT8 Channel;
  UINT8 Strobelane;
} FLOORPLAN;

#define DELAYSRANGE_ASSIGNDONE {0xffff, 0xffff}
#define CCRANGE_ASSIGNDONE     {0xffff, 0xffff, 0xffff, 0xffff, (UINT8 *) 0}

//  _MMRCLIBRARIES_H_

typedef struct {
  INT8         bitoffset;
  INT16        byteoffset;
} LP_TABLE;

typedef struct {
  IOSF_REG  *TwoX;
  IOSF_REG  *pi;
  IOSF_REG  *dBe;
  IOSF_REG  *dBs;
  IOSF_REG    *smp;
  UINT16    *Value;
  UINT8      *encoded;
  UINT8   decodedFlag;
} ALGO_REGS;

typedef struct {
  const UINT8     *regAssign;
} PHYINIT_LIST;

typedef struct {
  UINT32     Index;
  UINT32     DataHigh;
  UINT32     DataLow;
} MSR_REG;


typedef struct {
  UINT8 halfClk;
  UINT8 minVal1;
  UINT8 maxVal1;
  UINT8 StrVal1;
  UINT8 minVal2;
  UINT8 maxVal2;
  UINT8 StrVal2;
} DIGITAL_DLL_LIST;

enum enumPCFT {
  Pfct =  0,
  PfctT,
  PfctC,
  PfctCT,
  PfctF,
  PfctFT,
  PfctFC,
  PfctFCT,
  PfctP,
  PfctPT,
  PfctPC,
  PfctPCT,
  PfctPF,
  PfctPFT,
  PfctPFC,
  PfctPFCT,
  MaxPfct,
};

typedef enum {
  TRAS = 0,
  TRP,
  TRCD,
  TWR,
  TRFC,
  TWTR,
  TRRD,
  TRTP,
  TFAW,
  TCCD,
  TWTP,
  TWCL,
  TCMD,
  TCL,
  MAX_TIMING_DATA
} TimingDataType;

// Basic definitions used throughout the code.
//
typedef struct {
  UINT32 pfct;
  UINT8  length;
} PFCT_VARIATIONS;

typedef struct  {
  PFCT_VARIATIONS PFCTVariations[MaxPfct];
} LOCAL_PARAMS;

typedef struct {
  UINT16  Values[NUM_ALGOS][MAX_RANKS][MAX_STROBES];
} TRAINING_SETTING;

typedef struct {
  UINT8 Left[MAX_RANKS][MAX_STROBES];
  UINT8 Right[MAX_RANKS][MAX_STROBES];
} VREF_PI_DATA;


//
// Rank Marging Tool Data Index
//
typedef enum {
  RxDqLeft,
  RxDqRight,
  RxVLow,
  RxVHigh,
  TxDqLeft,
  TxDqRight,
  TxVLow,
  TxVHigh,
  CmdLeft,
  CmdRight,
  MaxRMTData,
} RmtData;

typedef struct {
  BOOLEAN                   Enabled;
  UINT32                    TotalMem;
  UINT8                     DimmFrequency[MAX_DIMMS_PER_CHANNEL];
  BOOLEAN                   RankEnabled[MAX_RANKS];
  UINT8                     TimingData[MAX_TIMING_DATA];
  UINT8                     DimmPresent[MAX_DIMMS_PER_CHANNEL];
  INT16                     RMT_Data[MAX_RANKS][MaxRMTData];
  TRAINING_SETTING          Trained_Value;
  VREF_PI_DATA 				EyeData[MAX_VREF_PI];
  UINT16                    CachedValues[NUM_CACHE_ELEMENTS][MAX_RANKS][MAX_STROBES];
} MMRC_CHANNEL;

typedef struct _FEATURE_SETTINGS {
  UINT8 MrcScramberEnable;
  UINT8 MrcDigitalDll;
  //UINT8	MrcDDRIOMPLL;
  INT8	MrcDynamicDDRPhySettings;
  UINT8	MrcDDRIOTrainingTrafficMode;
  UINT8	MrcDDRIOCompOverride;
  UINT8	MrcDDRIOClkGatingPM;
  UINT8 MrcBurstLegthMode;
  UINT8 MrcRMTSupport;
  UINT8 MrcCPGCExpLoopCnt;
  UINT8 MrcCPGCNumBursts;
} FEATURE_SETTINGS;

typedef struct {
  // outside of MMRC.
  BOOLEAN                   EccEnabled;
  UINT8                     MaxDq;
  FEATURE_SETTINGS  		FeatureSettings;
  UINT8                     MrcDebugMsgLevel;
  UINT8                     CurrentPlatform;
  UINT8                     CurrentFrequency;
  UINT8                     CurrentConfiguration;
  UINT8                     CurrentDdrType;
  UINT8						SiRevisionID;
  MMRC_CHANNEL              Channel[MAX_CHANNELS];
  UINT8                     PatternMode;
  UINT8                  MemoryDown;
  UINT32                 CpuFrequency;            
  UINT32                 CATraining_tsc_duration_low; 
  UINT32                 CATraining_tsc_duration_hgh; 
  UINT32                 CATraining_tsc_count;    
} MMRC_DATA;


typedef struct task_desc_s {
  UINT8           PostCode;
  UINT8           BootMode;
  STATUS          (*Function) (MMRC_DATA *, UINT8);
  UINT8           Channel;
} TASK_DESCRIPTOR;

extern STATUS JedecCmd (MMRC_DATA *, UINT8, UINT8, UINT8, UINT32);
extern UINT32 GetAddress (MMRC_DATA *, UINT8, UINT8);
extern STATUS ReceiveEnableEntryHooks (MMRC_DATA *ModMrcData, UINT8 Channel);
extern STATUS ReceiveEnableExitHooks (MMRC_DATA *ModMrcData, UINT8 Channel);
extern STATUS ReadVrefEntryHooks (MMRC_DATA *ModMrcData);
extern STATUS ReadVrefExitHooks (MMRC_DATA *ModMrcData);
extern STATUS ReadTrainingEntryHooks (MMRC_DATA *ModMrcData);
extern STATUS ReadTrainingExitHooks (MMRC_DATA *ModMrcData);
extern STATUS WriteTrainingEntryHooks (MMRC_DATA *ModMrcData);
extern STATUS WriteTrainingExitHooks (MMRC_DATA *ModMrcData);
extern STATUS FineWriteLevelingEntryHooks (MMRC_DATA *ModMrcData);
extern STATUS FineWriteLevelingExitHooks (MMRC_DATA *ModMrcData);
extern STATUS CoarseWriteLevelingEntryHooks (MMRC_DATA *ModMrcData);
extern STATUS CoarseWriteLevelingExitHooks (MMRC_DATA *ModMrcData);
extern STATUS ForceODT (UINT8 MrcDebugMsgLevel, UINT8 Channel, UINT8 Rank, UINT8 Value );
extern STATUS ReadTrainRestore (MMRC_DATA *, UINT8);
extern STATUS MrcDelay (UINT8  Type, UINT32 Delay);

//********************************************************************************************************************
// Function Declarations
//********************************************************************************************************************
STATUS
CPGC_Setup(
  IN MMRC_DATA   *ModMrcData,
  IN UINT8       Channel,
  IN UINT8       Rank
);

static
STATUS
CPGC_LoadCpgcPattern (
  MMRC_DATA     *ModMrcData,
  UINT8         Channel,
  UINT32        VictimPattern,
  UINT8         PatternIndex
);

STATUS
ReceiveEnableRestore (
  MMRC_DATA *ModMrcData,
  UINT8       Channel
);

static STATUS
SetRegsToLinear(
  MMRC_DATA *ModMrcData,
  UINT8      Socket,
  UINT8      Channel,
  UINT8      Dimm,
  UINT8      Rank,
  UINT8      Strobe,
  UINT8      Bit,
  UINT8      IoLevel,
  UINT8      Type,
  UINT8      Cmd,
  UINT32    *Value
);

STATUS
ReceiveEnable (
  MMRC_DATA *ModMrcData,
  UINT8       Channel
);

STATUS
WriteLevelingRestore (
  MMRC_DATA *ModMrcData,
  UINT8       Channel
);

STATUS FineWriteLeveling (
  MMRC_DATA *ModMrcData,
  UINT8       Channel
);

STATUS
CoarseWriteLeveling (
  MMRC_DATA *ModMrcData,
  UINT8       Channel
);

static STATUS
SampleDQS (
  MMRC_DATA *ModMrcData,
  UINT8       Channel,
  UINT8 Rank,
  UINT8 ReadWriteFlag,
  UINT8 *FinalResults
);

static
STATUS
RdWrLevelFineSearch (
  MMRC_DATA *ModMrcData,
  UINT8       Channel,
  UINT8 Rank,
  UINT32 CoarseStep,
  UINT8 ReadWriteFlag,
  UINT8 *DirectionFlag
);

static
STATUS
RdWrLevelFineDirectionAndStep (
  MMRC_DATA *ModMrcData,
  UINT8       Channel,
  UINT8 Rank,
  UINT8 ReadWriteFlag,
  UINT8 *DirectionFlag,
  UINT8 IncrementSize,
  UINT8 *Label
);

STATUS
DecodeAndExeRegAssignment (
  MMRC_DATA     *ModMrcData,
  UINT8         Channel,
  PHYINIT_LIST  PhyInitList
);

static
STATUS
AssignValue (
  MMRC_DATA     *ModMrcData,
  UINT8         Channel,
  UINT8         *Ptr,
  UINT32        Value,
  UINT8         Action,
  UINT32        Delay,
  UINT8         Counter,
  PHYINIT_LIST  Ddrphyinit_block
);

static
STATUS
BytelaneMask (
  MMRC_DATA *ModMrcData,
  UINT32 *Mask
);

static
STATUS
ReadWriteTrain (
  MMRC_DATA *ModMrcData,
  UINT8       Channel,
  UINT8 MarginType,
  UINT8 MarginMidpoint,
  INT8 MarginStep,
  UINT16 LowerBoundary,
  UINT16 UpperBoundary
);

STATUS
CreatePFCTSel (
  MMRC_DATA *ModMrcData,
  PFCT_VARIATIONS *PFCTSelect
);

STATUS
AverageDelay (
  MMRC_DATA *ModMrcData,
  UINT8 Channel,
  UINT8 Minus1type
);

static
UINT32
DecodeValue (
  MMRC_DATA       *ModMrcData,
  UINT8           Channel,
  UINT8           *CurAssignValuePtr,
  UINT32          *Value,
  UINT8           *Status,
  PFCT_VARIATIONS *PFCTVariations
);

static STATUS
GetLocation (
  IN  MMRC_DATA     *ModMrcData,
  IN  SetGetElements Element,
  IN  UINT8          Channel,
  IN  UINT8          Rank,
  IN  UINT8          Strobe,
  OUT UINT16         *ByteOffset,
  OUT UINT8          *StartingBit,
  OUT UINT8          *EndingBit
);

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
);

static
UINT8
DecodeLocation (
  UINT8 *Ptr,
  IOSF_REG DelayRegs[MAX_RANKS][MAX_BYTELANES_PER_DQ_MODULE*MAX_DQ_MODULES],
  UINT8 Channel
);

STATUS
GetSetDataSignal (
  MMRC_DATA *ModMrcData,
  UINT8 Socket,
  UINT8       Channel,
  UINT8 Dimm,
  UINT8 Rank,
  UINT8 Strobe,
  UINT8 Bit,
  UINT8 IoLevel,
  UINT8 Type,
  UINT8 Cmd,
  UINT32 *Value
);

STATUS
MrcDelay (
  UINT8 Type,
  UINT32 Delay
);

void
MmrcReadMsr (
  MSR_REG *MsrReg
);

void
MmrcWriteMsr (
  MSR_REG *MsrReg
);

UINT32
MmrcCpuIDRead (
  void
);

UINT32
MmrcCpuFrequencyRead(
  void
);

UINT8
MmrcCpuRatioRead(
  void
);

UINT32
MemFieldRead (
  MMRC_DATA *ModMrcData,
  UINT8 BoxType,
  UINT8 Instance,
  IOSF_REG IosfRegister
);

void
MemFieldWrite (
  MMRC_DATA *ModMrcData,
  UINT8 BoxType,
  UINT8 Instance,
  IOSF_REG IosfRegister,
  UINT32 Value
);

//
// CGPC API
//
static
STATUS
CPGC_S_Enable (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel
);
static
STATUS
CPGC_S_Disable (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel
);

static
STATUS
CPGC_S_SetupSeq (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel,
  UINT8 SubsequenceStart,
  UINT8 SubsequenceEnd,
  UINT8 LoopCount,
  UINT8 StopOnWrap
);

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
);

static
STATUS
CPGC_S_SetUnisequencer (
  IN OUT    MMRC_DATA  *ModMrcData,
  IN        UINT8       Channel,
  IN        UINT8       UniSequencerIndex,
  IN        UINT8       UniSequencerMode,
  IN        UINT32      UniSequencerDataPattern,
  IN        UINT8       UniSequencerReloadEnable
);

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
);

static
STATUS
CPGC_S_SetupPatternControl (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel
);

static
STATUS
CPGC_S_ClearErrors (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel
);

static
STATUS
CPGC_S_EnableErrors (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel,
  UINT8 SelBurstErrCheckEn,
  UINT8 SelChunkErrCheckEn,
  UINT32 ErrCheckLowMask,
  UINT32 ErrCheckHighMask,
  UINT32 ErrCheckECCMask
);

static
STATUS
CPGC_S_CheckErrors (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel,
  UINT32 *LowBitsErrStat,
  UINT32 *HighBitsErrStat,
  UINT32 *ByteLanesErrStat,
  UINT8 *ECCErrStat
);

static
STATUS
CPGC_S_StartTest (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel,
  UINT8       Rank
);

static
STATUS
CPGC_S_StopTest (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel
);

static
STATUS
CPGC_S_PollTest (
  MMRC_DATA  *ModMrcData,
  UINT8       Channel
);

VOID
Cpgc_S_SetupCADB (
  IN  OUT   MMRC_DATA     *ModMrcData,
  IN        UINT8         Channel,
  IN        UINT8         Enable
  );

VOID
MmrcMemSet (
  VOID *DataPtr,
  UINT8 Data,
  UINT32 Size
);

STATUS
SearchRmt (
  MMRC_DATA 	*ModMrcData,
  UINT8  		Channel
);

STATUS
RmtDqDqsVrefSearch (
  MMRC_DATA 	*ModMrcData,
  UINT8      	Channel,
  UINT8      	mode
);

STATUS
CATrainingRestore (
  IN MMRC_DATA   *ModMrcData,
  IN UINT8        Channel
);

STATUS
CPGC_InitializeMemory (
  IN MMRC_DATA   *ModMrcData,
  IN UINT8       Channel
);

STATUS
LPDDR3_JEDECInit (
  IN MMRC_DATA   *ModMrcData,
  IN UINT8 Channel
);

STATUS
PerformFifoReset (
  IN  OUT   MMRC_DATA     *ModMrcData,
  IN        UINT8         Channel,
  IN		UINT8         Rank
);

#define MAX_ELEMENTS  2
#define MAX_CCS       2
#define MAX_CC_RANGES 3
#define CLK 0
#define HLF 1
#define PI 2

typedef struct {
   UINT8  cacheIndex;
   UINT16 location;
} elementStruct;

typedef struct {
  UINT16          location;
  UINT8           cachedIndex;
} delayElement;

typedef struct {
  UINT8  max;
  UINT8  value;
} ccRanges;

typedef struct {
  UINT16          locations;
  UINT8           cachedIndex;
  UINT8           numberRanges;
  ccRanges        ccRange[MAX_CC_RANGES];
} clockCrossings;

typedef struct {
  UINT8            numberCCs;
  clockCrossings   cc[MAX_CCS];
} ccElements;

typedef struct {
  delayElement    delayElement[MAX_ELEMENTS];
  clockCrossings  ccElement[MAX_CCS];
} delayStruct;

#endif
