// Modular MMRC Version 1.00
// Modular MRC Working Group
// Filename: MMRCProjectLibraries.c
// Date:     8/9/2012

#include "../Include/MMRCProjectRegisters.h"
#include "../Include/MMRCProjectLibraries.h"
#include "../../../Hooks/Include/Printf.h"

//***********************************************************************************************
// WRLvlFine
const UINT8 HIPEntry_WrLvlFine[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
    DQCTL_009C           , MASK(0x100003fc), CMxx_x, SET               , xxxxx,                         VAL(0x10000154),                         // WDQTristate=0x01, DQSStrobe=0x55
    CMPCTRL_6800         , MASK(0x00000002), xxxx_x, SET               , xxxxx,                         VAL(0x00000000),                         // RCOMP=0x00
    OVRCTL_B0_0094       , MASK(0x00ffff00), CMxx_x, SET               , xxxxT,          2,  T(3),      VAL(0x003fa800),     T(4),   VAL(0x003fa800),                   // DiffAmp Override
    OVRCTL_B1_0098       , MASK(0x00ffff00), CMxx_x, SET               , xxxxT,          2,  T(3),      VAL(0x003fa800),     T(4),   VAL(0x003fa800),                   // DiffAmp Override
	LATCTL0_B0_0080      , MASK(0x0000001f), CMxx_x, SET               , dxxxx,                         VAL(0x4), 
	LATCTL0_B1_0084      , MASK(0x0000001f), CMxx_x, SET               , dxxxx,                         VAL(0x4), 
    ASSIGNDONE,
};

const UINT8 HIPExit_WrLvlFine[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
	DQCTL_009C           , MASK(0x100003fc), CMxx_x, SET               , xxxxx,                         VAL(0x00000154),  	// WDQTristate=0x01, DQSStrobe=0x55
	ASSIGNDONE,
};

const PHYINIT_LIST    HIP_wrlvlfine_list[] = {
//  RegAssignment
  { HIPEntry_WrLvlFine},
  { HIPExit_WrLvlFine},
};

//***********************************************************************************************
// WRLvlCoarse
const UINT8 HIPEntry_WRLvlCoarse[] = {
    // Offset          	Mask           	  Flags   Act     DELAY , dPFCT,    NC  Cond     	Value       	Cond	 Value
    //
	RXDQSPICODEB1_0058, MASK(0x7F7F7F7F), CMxx_x, SET    ,   	xxFxx,     6, F(1)		, VAL(0x24242424), F(2)		, VAL(0x23232323), F(4)		, VAL(0x20202020), F(24)	, VAL(0x0A0A0A0A),  F(32) 	, VAL(0x08080808), F(64), VAL(0x06060606),

	RXDQSPICODEB0_005C, MASK(0x7F7F7F7F), CMxx_x, SET    ,    	xxFxx,     6, F(1) 		, VAL(0x24242424), F(3)		, VAL(0x23232323), F(4)		, VAL(0x22222222), F(24) 	, VAL(0x0A0A0A0A),  F(32) 	, VAL(0x08080808), F(64), VAL(0x06060606),

	LATCTL0_B0_0080   , MASK(0x003f0000), CMxx_x, SET    ,		xxFxx,     3, F(1) 		, VAL(0x000c0000), F(2)		, VAL(0x000c0000), F(4) 	, VAL(0x00100000),
	LATCTL0_B1_0084   , MASK(0x003f0000), CMxx_x, SET    ,		xxFxx,     3, F(1) 		, VAL(0x000c0000), F(2) 	, VAL(0x000c0000), F(4)  	, VAL(0x00100000),

//need to test:
	LATCTL0_B0_0080   , MASK(0x003f0000), CMxx_x, SET    ,		xxFxT,     4, FT(59,4)	, VAL(0x00100000), FT(1,3)	, VAL(0x000c0000), FT(2,3) 	, VAL(0x000d0000)  , FT(68,3), VAL(0x00100000),
	LATCTL0_B1_0084   , MASK(0x003f0000), CMxx_x, SET    ,		xxFxT,     4, FT(59,4)	, VAL(0x00100000), FT(1,3) 	, VAL(0x000c0000), FT(2,3)	, VAL(0x000d0000)  , FT(68,3), VAL(0x00100000),

    ASSIGNDONE,
};

const UINT8 HIPExit_WRLvlCoarse[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
    ASSIGNDONE,
};

const PHYINIT_LIST    HIP_wrlvlcoarse_list[] = {
//  RegAssignment
  { HIPEntry_WRLvlCoarse},
  { HIPExit_WRLvlCoarse},
};

//***********************************************************************************************
// RdVref
const UINT8 HIPEntry_RdVref[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
    ASSIGNDONE,
};

const UINT8 HIPExit_RdVref[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
    ASSIGNDONE,
};

const PHYINIT_LIST    HIP_rdvref_list[] = {
//  RegAssignment
  { HIPEntry_RdVref},
  { HIPExit_RdVref},
};

//***********************************************************************************************
// Rd
const UINT8 HIPEntry_Rd[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
    ASSIGNDONE,
};

const UINT8 HIPExit_Rd[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
    ASSIGNDONE,
};

const PHYINIT_LIST    HIP_rd_list[] = {
//  RegAssignment
  { HIPEntry_Rd},
  { HIPExit_Rd},
};
//***********************************************************************************************
// Wr
const UINT8 HIPEntry_Wr[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
    ASSIGNDONE,
};

const UINT8 HIPExit_Wr[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
    ASSIGNDONE,
};

const PHYINIT_LIST    HIP_wr_list[] = {
//  RegAssignment
  { HIPEntry_Wr},
  { HIPExit_Wr},
};

//***********************************************************************************************
// Rcv Enable
const UINT8 HIPEntry_Rcvn[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
   B0RXIOBUFCTL_0010    , MASK(0x00000080), CMxx_x, SET               , xxxxx,                         VAL(0x00000000),
   B1RXIOBUFCTL_0020    , MASK(0x00000080), CMxx_x, SET               , xxxxx,                         VAL(0x00000000),
   // lpddr3 mch odt off
   OVRCTL_B0_0094       , MASK(0x00ffff00), CMxx_x, SET               , xxxxT,          2,  T(3),      VAL(0x003fa800),     T(4),   VAL(0x003fa800),                   // DiffAmp Override
   OVRCTL_B1_0098       , MASK(0x00ffff00), CMxx_x, SET               , xxxxT,          2,  T(3),      VAL(0x003fa800),     T(4),   VAL(0x003fa800),                   // DiffAmp Override
   LATCTL0_B0_0080      , MASK(0x00001f00), CMxx_x, SET               , dxxxx,                         VAL(0x3),
   LATCTL0_B1_0084      , MASK(0x00001f00), CMxx_x, SET               , dxxxx,                         VAL(0x3),
   ASSIGNDONE,
};

const UINT8 HIPExit_Rcvn[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
    ASSIGNDONE,
};

const PHYINIT_LIST    HIP_rcvn_list[] = {
//  RegAssignment
  { HIPEntry_Rcvn},
  { HIPExit_Rcvn},
};

//***********************************************************************************************
// CA training
const UINT8 HIPEntry_CAtrain[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
	OVRCTL_B0_0094       , MASK(0x00ffff00), CMxx_x, SET               , xxxxT,          2,  T(3),      VAL(0x003fa800),     T(4),   VAL(0x003fa800),                   // DiffAmp Override
	OVRCTL_B1_0098       , MASK(0x00ffff00), CMxx_x, SET               , xxxxT,          2,  T(3),      VAL(0x003fa800),     T(4),   VAL(0x003fa800),                   // DiffAmp Override
	TIMING_CTRL_010C     , MASK(0x10000000), CMxx_x, SET               , xxxxx,                         VAL(0x10000000),

    ASSIGNDONE,
};

const UINT8 HIPExit_CAtrain[] = {
    // Offset                   Mask            Flags   Act     DELAY       , dPFCT,    NC  Cond             Value            Cond             Value
    //
	TIMING_CTRL_010C    , MASK(0x10000000),CMxx_x, SET               , xxxxx,                         VAL(0x00000000),
    ASSIGNDONE,
};

const PHYINIT_LIST    HIP_catrain_list[] = {
//  RegAssignment
  { HIPEntry_CAtrain},
  { HIPExit_CAtrain},
};

//
// HalfClk:
// This is the possible values for the HALF CLOCK duration in PIs.  When in analog
// DLL mode, the number of PIs are 64, otherwise, its based on the frequency.
//
const UINT8 HalfClk[3][NUM_FREQ] = {
//  800        1066 1333 800DDLL   800DLLL_BY	1066DDLL 	1333DDLL
    { 64,     64,     64,     1,     1,      	1, 			1},       // AnalogDLL
    {  1,      1,      1,     26,    26,        22, 		18},       // DigitalDLL
    { 64,     64,     64,     32,    32,        32, 		32}       // RDQS
};

//Training Steps for analog and digital DLL
const UINT8 TrainStep[2] = {2, 1};

//
// Clock Crossings:  
// This provides the value to program into the clock crossing field.  If the
// value falls within the % range, then apply the in-range value, otherwise,
// program the out-of-range value.
//
/*
//                       MinPer  MaxPer  InValue  OutValue
ClockCrossings cc[] = { {   15,     45,     0,        1    },   // CC0 - dBe
                        {    15,     63,     0,        1    }    // CC1 - dBs
                      };
                      */
//                       MinPer  MaxPer  InValue  OutValue
ClockCrossings cc[] = { {   25,     72,     0,        1    },   // CC0 - dBe
                        {    0,     25,     1,        0    }    // CC1 - dBs
						};

//
// Granularity:
// Specifies that linear offset to be applied for one tick on the given element. The
// Value is in terms of HalfClk... since this number can change based on Digital/
// Analog DLL and frequency. 0 -> a step size of 1 UI, 1..x > 1*HALF_CLK
//
                             //2x,  pi
const UINT8 Granularity[] = {  1,  0};

//
// SetGet Individual Elements:
// List of all elements that can be accessed by the Get/Set.   The index into the array 
// is the element to access.  The first MAX_ALGO elements are hardcoded and are used for
// algorithm access, but can also be used for independent access.
//

const SetGetElements Elements[] = {
 // Cache  Offet         		Sb  Eb   ChE  MdE,  	Unique Cache  RB  Rb   SB  Sb
 {    PTRCTL0           		, 8 , 11,   1,   1,   -1,     -1 ,  0 , 0 , 0, 12  },  // ALGO_RCVN: DELAY0  - 2x
 {    DLLPICODER0B0  			, 24, 29,   1,   1,   -1,     -1 , 0 , 0 , -4, 0  },  // ALGO_RCVN: DELAY1  - Pi
 {    DBCTL1            		, 2 , 2 ,   1,   1,    -1,     -1 , 0 , 0 , 0 , 3  },  // ALGO_RCVN: CC0     - dBe
 {    DBCTL1            		, 8 , 8 ,   1,   1,    -1,     -1 , 0 , 0 , 0 , 3  },  // ALGO_RCVN: CC1     - dBs
 {    PTRCTL0           		, 4 ,  7,   1,   1,   -1,     -1 , 0 , 0 , 0, 12  },  // WDQS_DELAY0  - 2x
 {    DLLPICODER0B0  			, 16, 21,   1,   1,   -1,     -1 , 0 , 0 , -4, 0  },  // WDQS_DELAY1  - Pi
 {    DBCTL1            		, 1 , 1 ,   1,   1,    -1,     -1 , 0 , 0 , 0 , 3  },  // WDQS_CC0     - dBe
 {    DBCTL1            		, 7 , 7 ,   1,   1,    -1,     -1 , 0 , 0 , 0 , 3  },  // WDQS_CC1     - dBs
 {    PTRCTL0           		, 0 ,  3,   1,   1,   -1,     -1 , 0 , 0 , 0, 12  },  // WDQ_DELAY0  - 2x
 {    DLLPICODER0B0  			, 8 , 13,   1,   1,   -1,     -1 , 0 , 0 , -4, 0  },  // WDQ_DELAY1  - Pi
 {    DBCTL1            		, 0 , 0 ,   1,   1,    -1,     -1 , 0 , 0 , 0 , 3  },  // WDQ_CC0     - dBe
 {    DBCTL1            		, 6 , 6 ,   1,   1,    -1,     -1 , 0 , 0 , 0 , 3  },  // WDQ_CC1     - dBs
 {    OFFSET_UNDEFINED			, 0 , 0 ,  0 ,  0 ,   -1 ,     -1 , 0 , 0 , 0,  0  },  // RDQS_DELAY0  - 2x
 {    RXDQSPICODEB0     		, 0,  5 ,  1 ,  1 ,  -1 ,     -1 , 0 , 0 , -4, 0  },  // RDQS_DELAY1  - Pi
 {    OFFSET_UNDEFINED			, 0 , 0 ,  0 ,  0 ,   -1 ,     -1 , 0 , 0 , 0 , 0  },  // RDQS_CC0     - dBe
 {    OFFSET_UNDEFINED			, 0 , 0 ,  0 ,  0 ,   -1 ,     -1 , 0 , 0 , 0 , 0  },  // RDQS_CC1     - dBs
 {    POINTER_REG       		, 8 , 11,  1 ,  0 ,  -1 ,     -1 , 0 , 4 , 0,  0  },  // WCLK_DELAY0  - 2x
 {    DLLPICODER1       		, 16, 21,  1 ,  0 ,   6 ,     -1 , 0 , 0 , 0,  0  },  // WCLK_DELAY1  - Pi
 {    CFG_REG1          		, 0 , 0 ,  1 ,  0 ,   -1,     -1 , 0 , 1 ,  0 ,  0},  // WCLK_CC0     - dBe
 {    CFG_REG1          		, 8 , 8 ,  1 ,  0 ,   -1,     -1 , 0 , 1 ,  0 ,  0},  // WCLK_CC1     - dBs
 {    POINTER_REG__4844			, 8 , 11,  1 ,  0 ,  -1 ,     -1 , 0 , 0 , 0,  0  },  // CMD_DELAY0  - 2x
 {    DLLPICODER1__4824 		, 16, 21,  1 ,  0 ,  -1 ,     -1 , 0 , 0 , 0,  0  },  // CMD_DELAY1  - Pi
 {    CFG_REG0__4840			, 16, 16,  1 ,  0 ,  -1 ,     -1 , 0 , 0 , 0 , 0  },  // CMD_CC0     - dBe
 {    CFG_REG0__4840			, 17, 17,  1 ,  0 ,  -1 ,     -1 , 0 , 0 , 0 , 0  },  // CMD_CC1     - dBs
 {    CMD_PTRCTL__58B4   		, 8 , 11,  1 ,  0 ,  -1 ,     -1 , 0 , 0 , 0,  0  },   // CMD_CLKCTL_DELAY0  - 2x
 {    DLLPICODER1  				, 8, 13,   1 ,  0 ,   -1 ,     -1 , 0 , 0 , 0,  0  },  // CMD_CLKCTL_DELAY1  - Pi
 {    CMD_PTRCTL__58B4			, 1 , 1 ,  1 ,  0 ,   -1 ,     -1 , 0 , 0 , 0 , 0  },  // CMD_CLKCTL_CC0     - dBe
 {    CMD_PTRCTL__58B4			, 0 , 0 ,  1 ,  0 ,   -1 ,     -1 , 0 , 0 , 0 , 0  },  // CMD_CLKCTL_CC1     - dBs
 {    POINTER_REG       		, 24, 27,  1 ,  0 ,   7 ,     -1 , 0 , 0 ,  0,   0},  //  WCTL_DELAY0  - 2x
 {    DLLPICODER0       		, 8 , 13,  1 ,  0 ,   8 ,     -1 , 0 , 0 ,  0,   0},  //  WCTL_DELAY1  - Pi
 {    CFG_REG1         	 		, 12, 12,  1 ,  0 ,   9 ,     -1 , 0 , 0 ,  0 ,  0},  //  WCTL_CC0     - dBe
 {    CFG_REG1          		,  4,  4,  1 ,  0 ,   10,     -1 , 0 , 0 ,  0 ,  0},  //  WCTL_CC1     - dBs
 {    POINTER_REG__4844       	, 12, 15,  1 ,  0 ,   -1 ,    -1 , 0 , 4 ,  0,   0},  //  WCTL_CMD_DELAY0  - 2x
 {    DLLPICODER0__4820       	, 0 , 5,   1 ,  0 ,   11 ,    -1 , 0 , 0 ,  0,   0},  //  WCTL_CMD_DELAY1  - Pi
 {    CFG_REG0__4840         	, 24, 24,  1 ,  0 ,   -1 ,    -1 , 0 , 1 ,  0 ,  0},  //  WCTL_CMD_CC0     - dBe
 {    CFG_REG0__4840          	, 18, 18,  1 ,  0 ,   -1,     -1 , 0 , 1 ,  0 ,  0},  //  WCTL_CMD_CC1     - dBs

 {    LATCTL0_B0        		, 8 , 12,  1 ,   1,  -1 ,     -1 , 0 , 0 , 4,  0  },  // RCVN_ONEX    + 0
 {    RK2RK_PTRCTL      		, 18, 18,  1 ,   1,  -1 ,      0 , 0 , 3 , 0, -16 },  // RCVN_MIN1    + 1
 {    DQTRAINSTS        		, 1 , 1 ,  1 ,   1,  -1 ,     -1 , 0 , 0 , 0 , -1 },  // RCVN_SMP     + 2
 {    LATCTL0_B0       	 		, 0 ,  4,  1 ,   1,  -1 ,     -1 , 0 , 0 , 4,  0  },  // WDQS_ONEX    + 3
 {    RK2RK_PTRCTL      		, 17, 17,  1 ,   1,  -1 ,      1 , 0 , 3 , 0, -16 },  // WDQS_MIN1    + 4
 {    DQTRAINSTS        		, 9 , 9 ,  1 ,   1,  -1 ,     -1 , 0 , 0 , 0 , -1 },  // WDQS_SMP     + 5
 {    RK2RK_PTRCTL      		, 16, 16,  1 ,   1,  -1 ,      2 , 0 , 3 , 0, -16 },  // WDQ_MIN1     + 6
 {    B0VREFCTL         		, 8 , 14,  1 ,  1 ,  -1 ,      3 , 0 , 0,  16,  0 },  // RDQS_VREF:   + 7
 {    PTRCTL1           		, 8 ,  8,  1 ,  1 ,  -1 ,     -1 , 0 , 0,  0 ,  0 },  // FIFORst:     + 8
 {    DDR3RESETCTL      		, 16, 16,  1 ,  1 ,  -1 ,     -1 , 0 , 0,  0 ,  0 },  // WR_Level:    + 9
 {    LATCTL0_B0        		, 16, 21,  1,   1 ,   -1,     -1 , 0 , 0 , 4,   0},   // TRD          + 10

 {    LATCTL1__0088        		,  8, 12,  1,   1 ,   -1,     -1 , 0 , 0 , 0,   16},  // MCHODT_DELAY + 11
 {    LATCTL1__0088        		,  0,  4,  1,   1 ,   -1,     -1 , 0 , 0 , 0,   16},  // DIFFAMP_DELAY + 12

 {    ONDURCTL_B0__008C      	,  16, 21, 1,   1 ,   -1,     -1 , 0 , 0 , 4,   0},  // MCHODT_LENGTH + 13
 {    ONDURCTL_B0__008C        	,  8,  13, 1,   1 ,   -1,     -1 , 0 , 0 , 4,   0},  // DIFFAMP_LENGTH + 14

};

#if (MAX_RANKS ==4) 
                                    // R1               R2                   R3
                               //Off ,   Sb,    Eb    Offset  Sb   Eb     Offset  Sb    Eb                              
const UniqueRanks URanks[] = {{{{DBCTL1, 0x18, 0x18},{DBCTL0, 0x8, 0x8}, {DBCTL0, 0x18, 0x18}}},    // RCVN-dBs
                              {{{DBCTL1, 0x12, 0x12},{DBCTL0, 0x2, 0x2}, {DBCTL0, 0x12, 0x12}}},    // RCVN-dBe
                              {{{DBCTL1, 0x17, 0x17},{DBCTL0, 0x7, 0x7}, {DBCTL0, 0x17, 0x17}}},    // WDQS-dBs
                              {{{DBCTL1, 0x11, 0x11},{DBCTL0, 0x1, 0x1}, {DBCTL0, 0x11, 0x11}}},    // WDQS-dBe
                              {{{DBCTL1, 0x16, 0x16},{DBCTL0, 0x6, 0x6}, {DBCTL0, 0x16, 0x16}}},    // WDQ -dBs
                              {{{DBCTL1, 0x10, 0x10},{DBCTL0, 0x0, 0x0}, {DBCTL0, 0x10, 0x10}}},    // WDQ -dBe
                              {{{DLLPICODER0, 0, 5}, { DLLPICODER0, 0x10, 0x15}, {DLLPICODER1, 24, 29}}}, // WCLK-PI
                              {{{POINTER_REG, 24, 27}, { POINTER_REG, 28, 31}, {POINTER_REG, 28, 31}}}, // WCTL-2x
                              {{{DLLPICODER0, 8 , 13}, { DLLPICODER0, 24, 29}, {DLLPICODER0, 24, 29}}}, // WCTL-PI
                              {{{CFG_REG1   , 12, 12}, { CFG_REG1   , 13, 13}, {CFG_REG1   , 13, 13}}}, // WCTL-dBe
                              {{{CFG_REG1   , 4 , 4 }, { CFG_REG1    , 5, 5 }, {CFG_REG1   , 5 , 5 }}}, // WCTL-dBs
                              {{{DLLPICODER0__4820, 16 , 21 }, { DLLPICODER1__4824, 24, 29 }, {DLLPICODER0__4820, 8 , 13 }}}, // WCTL_CMD_DELAY1  - Pi
};
#endif

#if (MAX_RANKS ==2) 
                                    // R1               
                               //Off ,   Sb,    Eb    
const UniqueRanks URanks[] = {{{{DBCTL1, 0x18, 0x18}}},    // RCVN-dBs
                              {{{DBCTL1, 0x12, 0x12}}},    // RCVN-dBe
                              {{{DBCTL1, 0x17, 0x17}}},    // WDQS-dBs
                              {{{DBCTL1, 0x11, 0x11}}},    // WDQS-dBe
                              {{{DBCTL1, 0x16, 0x16}}},    // WDQ -dBs
                              {{{DBCTL1, 0x10, 0x10}}},    // WDQ -dBe
                              {{{DLLPICODER0, 0, 5}}}, // WCLK-PI
                              {{{POINTER_REG, 24, 27}}}, // WCTL-2x
                              {{{DLLPICODER0, 8 , 13}}}, // WCTL-PI
                              {{{CFG_REG1   , 12, 12}}}, // WCTL-dBe
                              {{{CFG_REG1   , 4 , 4 }}}, // WCTL-dBs
                              {{{DLLPICODER0__4820, 16 , 21 }}}, // WCTL_CMD_DELAY1  - Pi

};
#endif

const UINT8 LinearToPhysicalVrefCodes[] = { // lowest to highest
    0x27, 0x26, 0x25, 0x24, 0x23, 0x22, 0x21, 0x20, 0x00, 0x01, // 0 - 9
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0xB, // 10 -19
    0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, // 20 - 29
    0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, // 30 - 39
  };
const UINT8 PhysicalToLinearVrefCodes[] = { // lowest to highest
	  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
	  0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B,
	  0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
	  0x26, 0x27, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00,
  };

STATUS
DynamicAssignment1 (
  MMRC_DATA   *ModMrcData,
  UINT8        Channel,
  UINT8        Index,
  UINT32      *Value
)
{
	  switch (Index) {
	  case 0:
	    *Value = ModMrcData->Channel[Channel].TimingData[TCL] - 4;
	    break;
	  case 1:
	    *Value = 1;
	    break;
	  case 2:
	    *Value = 2;
	    break;
	  case 3:
	    *Value = (ModMrcData->Channel[Channel].TimingData[TCL] - 3) << 8 ;
	    break;     //rcven 1x
	  case 4:
	    *Value = ModMrcData->Channel[Channel].TimingData[TWCL] - 2;
	    break;       //wdqs

	  case 5: //WRCMD2DQSSTART
	    *Value = ModMrcData->Channel[Channel].TimingData[TWCL] - 2;
	    
	    break;
	  case 6: //RDCMD2RCVEN
	    *Value = (ModMrcData->Channel[Channel].TimingData[TCL] - 3) << 8;
	    
	    break;
	  case 7: //RDCMD2DATAVALID
	    *Value = (ModMrcData->Channel[Channel].TimingData[TCL] + 5) << 16;
	    
	    break;
	  case 8: //Burst Lenght
		  if (ModMrcData->CurrentDdrType == TYPE_LPDDR3){
			  *Value = 0x0 << 20;	//Always in Fixed mode for LPDDR3
		  } else {
		      if (ModMrcData->FeatureSettings.MrcBurstLegthMode){
		         	*Value = 0x1 << 20;
		      	}  else {
		         	*Value = 0x0 << 20;
		      	}
		  }
	    
	    break;
	  case DYN_DIGITALDLL:  //0xA
	    *Value = ModMrcData->FeatureSettings.MrcDigitalDll;
	    break;
	  default:
	    *Value = 0xffffffff;
	    break;
	  }

  return SUCCESS;
}

const UINT32 cpu_base_freq_mhz[] = {
  83,
  100,
  133,
  116,
};

//
// This structure defines the valid ranges of the PI elemets within 
// the various linear delays.  The layout is that there will always
// be two valid ranges within the full coverate which are able to
// be programmed with out-of-range elements not supported.
//

const DIGITAL_DLL_LIST mmrcDigitalDLL[] = {
//	PI	Min1, 	Max1, Start1, 	Min2, 	Max2, 	Start2
{ 	20,	0, 		9, 		0, 		16, 	25, 	10}, // 800
{ 	22, 0, 		10, 	0, 		16, 	26, 	11}, // 1066
{ 	16, 0, 		7, 		0, 		16, 	23, 	8 }, // 1333
{ 	26, 0, 		12, 	0, 		16, 	28, 	13}, // 800DLL
{ 	26, 0, 		12, 	0, 		16, 	28, 	13}, // 800DLLMPLLby
{ 	22, 0, 		10, 	0, 		16, 	26, 	11}, // 1066DLL
{   18, 0,      8, 		0,      16, 	24, 	9}, // 1333DLLL
};


const UINT8 DUNITPortID[MAX_CHANNELS]= {
		MCUPORT,
		0x07
};
const UINT8 CPGCPortID[MAX_CHANNELS]= {
		CPGCPORT,
		0x9
};

//
// Floorplan layout to map virtual channel and strobe to a physical channel and strobe.
// The array is based on the number of channels and strobes.
//
const FLOORPLAN FloorPlan[MAX_CHANNELS][MAX_STROBES] = {
    //SL0    SL1    SL2    SL3    SL4    SL5    SL6    SL7    SL8(ECC)
  {{0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {0,5}, {0,6}, {0,7}, {1,7}},
  {{1,0}, {1,1}, {1,2}, {1,3}, {1,4}, {1,5}, {1,6}, {1,7}, {0,0}}
};

#define NUMCAPATTERN 16

const  UINT32 CApattern0[NUMCAPATTERN] = {0x003FF000, 0x000003FF, 0x0037F080, 0x003BF040, 
	                                        0x003df020, 0x003ef010, 0x0008037f, 0x000403bf, 0x000203df,
	                                        0x000103ef, 0x003ff3ff, 0x00000000, 0x003ff000, 0x000003ff,
	                                        0x00000000, 0x003ff3ff};
const  UINT32 CApattern1[NUMCAPATTERN] = {0x003FF000, 0x000003FF, 0x0037F080, 0x003BF040, 
	                                        0x003df020, 0x003ef010, 0x0008037f, 0x000403bf, 0x000203df,
	                                        0x000103ef, 0x00000000, 0x003ff3ff, 0x000003ff, 0x003ff000,
	                                        0x000003ff, 0x003ff000};
const  UINT32 CApattern2[NUMCAPATTERN] = {0x003FF000, 0x000003FF, 0x0037F080, 0x003BF040, 
	                                        0x003df020, 0x003ef010, 0x0008037f, 0x000403bf, 0x000203df,
	                                        0x000103ef, 0x003ff3ff, 0x00000000, 0x00000000, 0x000003ff,
	                                        0x000003ff, 0x003ff000};

