// Modular MMRC Version 1.00
// Modular MRC Working Group
// Filename: MMRCProjectLibraries.h
// Date:     8/8/2012

#ifndef _MMRCPROJECTLIBRARIES_H_
#define _MMRCPROJECTLIBRARIES_H_
#include "Types.h"
#include "MMRCLibraries.h"
#include "MMRCProjectData.h"
#include "MMRCProjectRegisters.h"


//
// Layout definitions  of the elements array.
//
#define NUM_DELAY_ELEMENTS      2                     // Number of Delay Elements in each linear delay.
#define NUM_CC_ELEMENTS         2                     // Number of Clock Crossings in each linear delay.
#define NUM_ELEMENTS_PER_ALGO   (NUM_DELAY_ELEMENTS+NUM_CC_ELEMENTS)  // Total number of elements per linear delay
#define ALGO_REG_INDEX          0xf0                  // Starting index in the TYPE field for algos.
#define INDIV_ELEMENT_INDEX     (NUM_ALGOS * NUM_ELEMENTS_PER_ALGO) // Starting index of non-linear delay elements.
#define STROBE_LANES_PER_MODULE 2                     // Stobe lanes per module.
// 
// Index of individual elements supported by the Get/set
//
#define REG_UNDEFINED    0xFF
#define OFFSET_UNDEFINED 0xFFFF

#define RCVN_SMP         INDIV_ELEMENT_INDEX + 2
#define RCVN_MIN         INDIV_ELEMENT_INDEX + 1
#define RCVN_ONEX        INDIV_ELEMENT_INDEX + 0
#define RCVN_TWOX        RCVN_INDEX*NUM_ELEMENTS_PER_ALGO + 0
#define RCVN_PI          RCVN_INDEX*NUM_ELEMENTS_PER_ALGO + 1
#define RCVN_CC0         RCVN_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS + 0
#define RCVN_CC1         RCVN_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS + 1
#define RCVN_VREF        REG_UNDEFINED

#define WDQS_SMP         INDIV_ELEMENT_INDEX + 5
#define WDQS_MIN         INDIV_ELEMENT_INDEX + 4
#define WDQS_ONEX        INDIV_ELEMENT_INDEX + 3
#define WDQS_TWOX        WDQS_INDEX*NUM_ELEMENTS_PER_ALGO + 0
#define WDQS_PI          WDQS_INDEX*NUM_ELEMENTS_PER_ALGO + 1
#define WDQS_CC0         WDQS_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS + 0
#define WDQS_CC1         WDQS_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS + 1
#define WDQS_VREF        REG_UNDEFINED

#define WDQ_SMP          REG_UNDEFINED
#define WDQ_MIN          INDIV_ELEMENT_INDEX + 6
#define WDQ_ONEX         REG_UNDEFINED
#define WDQ_TWOX         WDQ_INDEX*NUM_ELEMENTS_PER_ALGO + 0
#define WDQ_PI           WDQ_INDEX*NUM_ELEMENTS_PER_ALGO + 1
#define WDQ_CC0          WDQ_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS + 0
#define WDQ_CC1          WDQ_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS + 1
#define WDQ_VREF         REG_UNDEFINED

#define RDQS_SMP          REG_UNDEFINED
#define RDQS_MIN          REG_UNDEFINED
#define RDQS_ONEX         REG_UNDEFINED
#define RDQS_TWOX         REG_UNDEFINED
#define RDQS_PI           RDQS_INDEX*NUM_ELEMENTS_PER_ALGO + 1
#define RDQS_CC0          REG_UNDEFINED
#define RDQS_CC1          REG_UNDEFINED
#define RDQS_VREF         INDIV_ELEMENT_INDEX + 7

#define WCLK_SMP          REG_UNDEFINED
#define WCLK_MIN          REG_UNDEFINED
#define WCLK_ONEX         REG_UNDEFINED
#define WCLK_TWOX         WCLK_INDEX*NUM_ELEMENTS_PER_ALGO + 0
#define WCLK_PI           WCLK_INDEX*NUM_ELEMENTS_PER_ALGO + 1
#define WCLK_CC0          WCLK_INDEX*NUM_ELEMENTS_PER_ALGO + 2
#define WCLK_CC1          WCLK_INDEX*NUM_ELEMENTS_PER_ALGO + 3
#define WCLK_VREF         REG_UNDEFINED

#define CMD_SMP          REG_UNDEFINED
#define CMD_MIN          REG_UNDEFINED
#define CMD_ONEX         REG_UNDEFINED
#define CMD_TWOX         CMD_INDEX*NUM_ELEMENTS_PER_ALGO + 0
#define CMD_PI           CMD_INDEX*NUM_ELEMENTS_PER_ALGO + 1
#define CMD_CC0          CMD_INDEX*NUM_ELEMENTS_PER_ALGO + 2
#define CMD_CC1          CMD_INDEX*NUM_ELEMENTS_PER_ALGO + 3
#define CMD_VREF         REG_UNDEFINED

#define CMD_CLKCTL_SMP 		REG_UNDEFINED
#define CMD_CLKCTL_MIN    	REG_UNDEFINED
#define CMD_CLKCTL_ONEX    	REG_UNDEFINED
#define CMD_CLKCTL_TWOX    	CMD_CLKCTL_INDEX*NUM_ELEMENTS_PER_ALGO + 0
#define CMD_CLKCTL_PI       CMD_CLKCTL_INDEX*NUM_ELEMENTS_PER_ALGO + 1
#define CMD_CLKCTL_CC0      CMD_CLKCTL_INDEX*NUM_ELEMENTS_PER_ALGO + 2
#define CMD_CLKCTL_CC1      CMD_CLKCTL_INDEX*NUM_ELEMENTS_PER_ALGO + 3
#define CMD_CLKCTL_VREF     REG_UNDEFINED

#define WCTL_SMP          	REG_UNDEFINED
#define WCTL_MIN          	REG_UNDEFINED
#define WCTL_ONEX         	REG_UNDEFINED
#define WCTL_TWOX         	WCTL_INDEX*NUM_ELEMENTS_PER_ALGO + 0
#define WCTL_PI          	WCTL_INDEX*NUM_ELEMENTS_PER_ALGO + 1
#define WCTL_CC0          	WCTL_INDEX*NUM_ELEMENTS_PER_ALGO + 2
#define WCTL_CC1          	WCTL_INDEX*NUM_ELEMENTS_PER_ALGO + 3
#define WCTL_VREF         	REG_UNDEFINED

#define WCTL_CMD_SMP          	REG_UNDEFINED
#define WCTL_CMD_MIN          	REG_UNDEFINED
#define WCTL_CMD_ONEX         	REG_UNDEFINED
#define WCTL_CMD_TWOX         	WCTL_CMD_INDEX*NUM_ELEMENTS_PER_ALGO + 0
#define WCTL_CMD_PI          	WCTL_CMD_INDEX*NUM_ELEMENTS_PER_ALGO + 1
#define WCTL_CMD_CC0          	WCTL_CMD_INDEX*NUM_ELEMENTS_PER_ALGO + 2
#define WCTL_CMD_CC1          	WCTL_CMD_INDEX*NUM_ELEMENTS_PER_ALGO + 3
#define WCTL_CMD_VREF         	REG_UNDEFINED

#define FIFORESET         		INDIV_ELEMENT_INDEX + 8
#define WRL_MODE          		INDIV_ELEMENT_INDEX + 9
#define TRD               		INDIV_ELEMENT_INDEX + 10
#define MCHODT_DELAY         	INDIV_ELEMENT_INDEX + 11
#define DIFFAMP_DELAY          	INDIV_ELEMENT_INDEX + 12
#define MCHODT_LENGTH          	INDIV_ELEMENT_INDEX + 13
#define DIFFAMP_LENGTH         	INDIV_ELEMENT_INDEX + 14
//
// Algorithm indexes, 
//
#define RCVN_INDEX              0
#define WDQS_INDEX              1
#define WDQ_INDEX               2
#define RDQS_INDEX              3
#define WCLK_INDEX              4
#define CMD_INDEX              	5
#define CMD_CLKCTL_INDEX		6
#define WCTL_INDEX              7
#define WCTL_CMD_INDEX          8

//
// MACROS to identify RCVN elements/algorithms
//
#define DELAYS_RCVN_DEL      	(ALGO_REG_INDEX + RCVN_INDEX)
#define DELAYS_WDQS_DEL   		(ALGO_REG_INDEX + WDQS_INDEX)
#define DELAYS_WDQ_DEL         	(ALGO_REG_INDEX + WDQ_INDEX)
#define DELAYS_RDQS_DEL        	(ALGO_REG_INDEX + RDQS_INDEX)
#define DELAYS_WCLK_DEL        	(ALGO_REG_INDEX + WCLK_INDEX)
#define DELAYS_CMD_DEL        	(ALGO_REG_INDEX + CMD_INDEX)
#define DELAYS_CMD_CLKCTL_DEL	(ALGO_REG_INDEX + CMD_CLKCTL_INDEX)
#define DELAYS_WCTL_DEL			(ALGO_REG_INDEX + WCTL_INDEX)

#define RCVN_DELAY(x)            (RCVN_INDEX*NUM_ELEMENTS_PER_ALGO)+x
#define RCVN_CC(x)               (RCVN_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS)+x
#define WDQS_DELAY(x)            (WDQS_INDEX*NUM_ELEMENTS_PER_ALGO)+x
#define WDQS_CC(x)               (WDQS_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS)+x
#define WDQ_DELAY(x)             (WDQ_INDEX*NUM_ELEMENTS_PER_ALGO)+x
#define WDQ_CC(x)                (WDQ_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS)+x
#define RDQS_DELAY(x)            (RDQS_INDEX*NUM_ELEMENTS_PER_ALGO)+x
#define RDQS_CC(x)               (RDQS_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS)+x
#define WCLK_DELAY(x)            (WCLK_INDEX*NUM_ELEMENTS_PER_ALGO)+x
#define WCLK_CC(x)               (WCLK_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS)+x
#define CMD_DELAY(x)            (CMD_INDEX*NUM_ELEMENTS_PER_ALGO)+x
#define CMD_CC(x)              	(CMD_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS)+x
#define CMD_CLKCTL_DELAY(x) 	(CMD_CLKCTL_INDEX*NUM_ELEMENTS_PER_ALGO)+x
#define CMD_CLKCTL_CC(x)     	(CMD_CLKCTL_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS)+x
#define WCTL_DELAY(x) 			(WCTL_INDEX*NUM_ELEMENTS_PER_ALGO)+x
#define WCTL_CC(x)     			(WCTL_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS)+x
#define WCTL_CMD_DELAY(x) 		(WCTL_CMD_INDEX*NUM_ELEMENTS_PER_ALGO)+x
#define WCTL_CMD_CC(x)     		(WCTL_CMD_INDEX*NUM_ELEMENTS_PER_ALGO + NUM_DELAY_ELEMENTS)+x

#define	DDRPORT             	0x0c
#define CPGCPORT              	0x0d
#define MCUPORT                	0x01
#define CPGCMODE_WR            	0
#define CPGCMODE_R             	1
#define CPGCMODE_W	            2
#define CPGC_ENABLED           	0
#define CPGC_API 				1

#define	CPGC_UNISEQINDEX_LMN	0x0
#define	CPGC_UNISEQINDEX_PBM	0x1
#define	CPGC_UNISEQINDEX_LFSR 	0x2

#define	CPGC_UNISEQINDEX_0	0x0
#define	CPGC_UNISEQINDEX_1	0x1
#define	CPGC_UNISEQINDEX_2	0x2

#define	CPGC_DPAT0	0x0
#define	CPGC_DPAT1	0x1
#define	CPGC_DPAT2	0x2
#define	CPGC_DPAT3	0x3
#define	CPGC_DPAT4	0x4
#define	CPGC_DPAT5	0x5
#define	CPGC_DPAT6	0x6
#define	CPGC_DPAT7	0x7
#define	CPGC_DPAT8	0x8
#define	CPGC_DPAT9	0x9
#define	CPGC_DPAT10	0xa
#define	CPGC_DPAT11	0xb
#define	CPGC_DPAT12	0xc
#define	CPGC_DPAT13	0xd
#define	CPGC_DPAT14	0xe
#define	CPGC_DPAT15	0xf

#define CPGC_SUBSEQINDEX_0 0x0
#define CPGC_SUBSEQINDEX_1 0x1
#define CPGC_SUBSEQINDEX_2 0x2
#define CPGC_SUBSEQINDEX_3 0x3

#define	CPGC_SUBSEQ_TYPE_RO	0x0
#define	CPGC_SUBSEQ_TYPE_WO	0x1
#define	CPGC_SUBSEQ_TYPE_RW	0x2
#define	CPGC_SUBSEQ_TYPE_WR	0x3
#define	CPGC_SUBSEQ_TYPE_ROREF	0x8
#define	CPGC_SUBSEQ_TYPE_WOREF	0x9
#define	CPGC_SUBSEQ_TYPE_RWREF	0xa
#define	CPGC_SUBSEQ_TYPE_WRREF	0xb

//SV DDRIO Training Traffic Mode Option
#define	CPGC_CFGC_LFSR			0
#define	CPGC_VICAGR_FIXED		1
#define	CPGC_VICAGR_LFSR		2
#define	CPGC_CPU_MODE			3

extern const PHYINIT_LIST    HIP_rcvn_list[];
extern const PHYINIT_LIST    HIP_rdvref_list[];
extern const PHYINIT_LIST    HIP_rd_list[];
extern const PHYINIT_LIST    HIP_wr_list[];
extern const PHYINIT_LIST    HIP_wrlvlfine_list[];
extern const PHYINIT_LIST    HIP_wrlvlcoarse_list[];
extern const PHYINIT_LIST    HIP_catrain_list[];

extern const LP_TABLE    gRankToRankOffset[];
extern const LP_TABLE    gByteLaneOffsetWithinModule[];

extern const UINT32 cpu_base_freq_mhz[];
extern const UINT8 CPGCPortID[MAX_CHANNELS];
extern const DIGITAL_DLL_LIST mmrcDigitalDLL[];
extern const FLOORPLAN FloorPlan[MAX_CHANNELS][MAX_STROBES];
extern const UINT8     HalfClk[3][NUM_FREQ];
extern const SetGetElements           Elements[];
extern const UINT8                    Granularity[];
extern const UniqueRanks              URanks[];
extern ClockCrossings                 cc[];
extern const UINT8 LinearToPhysicalVrefCodes[];
extern const UINT8 PhysicalToLinearVrefCodes[];
extern const UINT8 TrainStep[2];

extern const  UINT32 CApattern0[];
extern const  UINT32 CApattern1[];
extern const  UINT32 CApattern2[];
#define NUM_DDRPHY_BLOCKS     5


// 
// Set/Get API Defines
// 
// 
//#define ASSIGN_DONE   (0xFF) 
#define DELAY_ARRAY_DETAILS ( 0)
// 
// Delays Data Types
// 
#define DELAY_ARRAYS_DT_DELAY     0x10
#define DELAY_ARRAYS_DT_SAMPLE    0x20
#define DELAY_ARRAYS_DT_MINUSONE  0x30
#define DELAY_ARRAYS_DT_OTHER     0x40
#define DELAY_ARRAYS_DT_1X        0x50
// 
// Delays Object Types
// 
#define DELAY_ARRAYS_OT_RCVN_DEL    0x0
#define DELAY_ARRAYS_OT_WDQ_DEL     0x1
#define DELAY_ARRAYS_OT_WDQS_DEL    0x2
#define DELAY_ARRAYS_OT_WCLK_DEL    0x3
#define DELAY_ARRAYS_OT_RDQS_DEL    0x4
#define DELAY_ARRAYS_OT_CMD_DEL     0x5
#define DELAY_ARRAYS_OT_RD_VREF     0x6
#define DELAY_ARRAYS_OT_WR_VREF     0x7
// 
// Delays Objects
// 
#define DELAY_ARRAYS_RCVN_DEL       (DELAY_ARRAYS_OT_RCVN   | DELAY_ARRAYS_DT_DELAY)
#define DELAY_ARRAYS_WDQ_DEL        (DELAY_ARRAYS_OT_WDQ    | DELAY_ARRAYS_DT_DELAY)
#define DELAY_ARRAYS_WDQS_DEL       (DELAY_ARRAYS_OT_WDQS   | DELAY_ARRAYS_DT_DELAY)
#define DELAY_ARRAYS_WCLK_DEL       (DELAY_ARRAYS_OT_WCLK   | DELAY_ARRAYS_DT_DELAY)
#define DELAY_ARRAYS_RD_VREF        (DELAY_ARRAYS_OT_RDVREF | DELAY_ARRAYS_DT_DELAY)
#define DELAY_ARRAYS_WR_VREF        (DELAY_ARRAYS_OT_WRVREF | DELAY_ARRAYS_DT_DELAY)
#define DELAY_ARRAYS_CMD_DEL        (DELAY_ARRAYS_OT_CMD    | DELAY_ARRAYS_DT_DELAY)

#define DELAY_ARRAYS_RCVN_MINUS1    (DELAY_ARRAYS_OT_RCVN   | DELAY_ARRAYS_DT_MINUSONE)
#define DELAY_ARRAYS_WDQ_MINUS1     (DELAY_ARRAYS_OT_WDQ    | DELAY_ARRAYS_DT_MINUSONE)
#define DELAY_ARRAYS_WDQS_MINUS1    (DELAY_ARRAYS_OT_WDQS   | DELAY_ARRAYS_DT_MINUSONE)
#define DELAY_ARRAYS_WCLK_MINUS1    (DELAY_ARRAYS_OT_WCLK   | DELAY_ARRAYS_DT_MINUSONE)
#define DELAY_ARRAYS_CMD_MINUS1     (DELAY_ARRAYS_OT_CMD    | DELAY_ARRAYS_DT_MINUSONE)

#define DELAY_ARRAYS_RCVN_SAMPLE    (DELAY_ARRAYS_OT_RCVN   | DELAY_ARRAYS_DT_SAMPLE)
#define DELAY_ARRAYS_WDQ_SAMPLE     (DELAY_ARRAYS_OT_WDQ    | DELAY_ARRAYS_DT_SAMPLE)
#define DELAY_ARRAYS_WDQS_SAMPLE    (DELAY_ARRAYS_OT_WDQS   | DELAY_ARRAYS_DT_SAMPLE)
#define DELAY_ARRAYS_WCLK_SAMPLE    (DELAY_ARRAYS_OT_WCLK   | DELAY_ARRAYS_DT_SAMPLE)
#define DELAY_ARRAYS_CMD_SAMPLE     (DELAY_ARRAYS_OT_CMD    | DELAY_ARRAYS_DT_SAMPLE)

//
// The 1X delay type needs to be support for each signal group.
//
#define DELAY_ARRAYS_RCVN_ONEX      (DELAY_ARRAYS_OT_RCVN_DEL   | DELAY_ARRAYS_DT_1X)
#define DELAY_ARRAYS_WDQ_ONEX       (DELAY_ARRAYS_OT_WDQ_DEL    | DELAY_ARRAYS_DT_1X)
#define DELAY_ARRAYS_WDQS_ONEX      (DELAY_ARRAYS_OT_WDQS_DEL   | DELAY_ARRAYS_DT_1X)
#define DELAY_ARRAYS_WCLK_ONEX      (DELAY_ARRAYS_OT_WCLK_DEL   | DELAY_ARRAYS_DT_1X)
#define DELAY_ARRAYS_RDQS_ONEX      (DELAY_ARRAYS_OT_RDQS_DEL   | DELAY_ARRAYS_DT_1X)
#define DELAY_ARRAYS_CMD_ONEX       (DELAY_ARRAYS_OT_CMD_DEL    | DELAY_ARRAYS_DT_1X)

#define DELAY_ARRAYS_OTH_FIFORESET_OFF     (0x0)
#define DELAY_ARRAYS_OTH_WRL_MODE_OFF      (0x1)

#define DELAY_ARRAYS_FIFORESET      (DELAY_ARRAYS_DT_OTHER  | DELAY_ARRAYS_OTH_FIFORESET_OFF)
#define DELAY_ARRAYS_WRL_MODE       (DELAY_ARRAYS_DT_OTHER  | DELAY_ARRAYS_OTH_WRL_MODE_OFF)

#define	MSR_CPU_BASE_FREQ 		0xCD
#define MSR_CPU_BASE_FREQ_MASK 0x03
#define MSR_CPU_BASE_FREQ_SB	0x0
#define MSR_CPU_MAX_RATIO 		0x66a
#define MSR_CPU_MAX_RATIO_MASK 0x3F0000
#define MSR_CPU_MAX_RATIO_SB	0x10

#define DYN_DIGITALDLL 0xA

#define PRINTF_SUP				       0x01        // Support for Generic Printf using Printf.
#define MINUS1_SUP               0x01        // Support for the Minus1 features.
#define BROADCAST_SUP            0x00        // Support for Broadcasting during phyinit's when looping on modules.
#define SIPGLOBAL                0x01        // Enable for SIP to be performed in each training or once globalley
#define DDRPHYINIT_DETAILS       0x00        // When printing PhyInit, support for detailed output information.

// RdVefTraining Parameters
#define RCVN_INITIAL2XVAL           7
#define RDVREFTRAIN_MIDPOINTVREF 0x14
#define RDVREFTRAIN_MARGINSTEP   0x02
#define RDVREFTRAIN_LOWBOUNDARY  0x00
#define RDVREFTRAIN_UPBOUNDARY   0x28

// RdTraining Parameters
#define RDTRAIN_MIDPOINTVREF 0x20
#define RDTRAIN_MARGINSTEP   0x02
#define RDTRAIN_LOWBOUNDARY  0x00
#define RDTRAIN_UPBOUNDARY   0x3f

// WrTraining Parameters
#define WRTRAIN_MIDPOINTVREF 0x20
#define WRTRAIN_MARGINSTEP   0x02
#define WRTRAIN_LOWBOUNDARY  0x00
#define WRTRAIN_UPBOUNDARY   0x3f

// WrVrefTraining Parameters
#define WRVREFTRAIN_MIDPOINTVREF 0x20
#define WRVREFTRAIN_MARGINSTEP   0x02
#define WRVREFTRAIN_LOWBOUNDARY  0x00
#define WRVREFTRAIN_UPBOUNDARY   0x3f

// 1D/2D Eye Diagram Debug Message Parameters
#define MARGIN_DISTANCE(x,y) ((x)>(y) ? (x)-(y) : (y)-(x))
#define MAX_LINE_SIZE 80
#define	EYEDATA_1D_VREFPI_OFFSET 0

// CATraining Parameters
#define CATRAIN_MIDPOINTVREF 0x20
#define CATRAIN_MARGINSTEP   0x02
#define CATRAIN_LOWBOUNDARY  0x00
#define CATRAIN_UPBOUNDARY   0x3f

// Rcvn Selections
#define RCVN_LARGE_STEP          0x20
#define RCVN_MEDIUM_STEP         0x05
#define RCVN_SMALL_STEP          0x01

#define NUMSAMPLES              8
#define SAMPLETHRESH            4

// Elements based on the design of the chip.
#define CHANNEL_OFFSET           		0x2000
#define MODULE_OFFSET           	 	0x800
#define CHANNEL_CMDCTLOFFSET		0x800

// JEDEC Configurable selections.
#define JEDEC_OUTPUTDRIVERIMPEDANCE 2//0
#define JEDEC_RTTTARGET       0x4//0x6
#define JEDEC_RTTTARGETDIC0   0x2
#define JEDEC_RTTNONTARGET    0x4//0x0
#define JEDEC_RTTTARGET_0     0x0
#define JEDEC_RTTTARGET_40    0x44
#define JEDEC_RTTTARGET_60    0x4
#define JEDEC_RTTTARGET_120   0x40

#define PHYENTERWRLVL         0x1
#define PHYEXITWRLVL          0x0

// Pattern Creations
#define NUMBERPATTERNS             0x10			// Number of Victim/Aggressor Patterns * (repeat +1)= 10 * (16 + 1) = 10*17 = 170  Defined by tool
#define VICTIMPATTERNS             { 0xAAAAAAAA,  0xDB6DB6DB,  0x92492492,  0xEEEEEEEE,  0xCCCCCCCC,  0x88888888,  0xF7BDEF7B,  0xE739CE73,  0xC6318C63,  0x84210842}
#define AGGRESSORPATTERNS          {~0xAAAAAAAA, ~0xDB6DB6DB, ~0x92492492, ~0xEEEEEEEE, ~0xCCCCCCCC, ~0x88888888, ~0xF7BDEF7B, ~0xE739CE73, ~0xC6318C63, ~0x84210842}
#define PATTERN_SIZE               256						//  4 CL for Cedarview = 256 bytes , 2 CL for Berryville
#define FLYBY_SHIFT                {0, 0, 0, 0, 0, 0, 0, 0}

#define FIFO_RESET_DISABLE    0x01
#define FIFO_RESET_ENABLE     0x00
//USER DEFINABLE
#define BUSWIDTH	            0x8		/*1=x8, 2=x16, 4=x32, 8=x64*/
#define ZEROS                  0x00
#define ONES                   0xff
#define	DDRPORT				        0x0c
#define RCVEN_DQS_NUM_SAMPLES		20		// Total number of samples to use during rcven calibration.
#define	RCVEN_DQS_SAMPLE_THRESH		10		// Threshold of when to stop sampling during rcven calibration.
#define CHANNEL_BYTEOFFSET		    0x2000
#define CHANNEL_BYTEOFFSET2		0x800
#define CHANNEL_BITOFFSET		    0
#define RDWR_NUMBERPATTERNS	0x10			// Number of Victim/Aggressor Patterns * (repeat +1)= 10 * (16 + 1) = 10*17 = 170  Defined by tool

extern STATUS DynamicAssignment1 (MMRC_DATA   *ModMrcData, UINT8       Channel, UINT8       Index, UINT32      *Value);

#endif //_MMRCPROJECTLIBRARIES_H_
