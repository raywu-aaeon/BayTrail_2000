/** @file ModMRC_types.h
*/
#ifndef _MODMRC_TYPES_H_
#define _MODMRC_TYPES_H_

#include "Types.h"
#include "Imemory.h"

/*
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned long       uint32_t;
typedef unsigned long long  uint64_t;



typedef char                int8_t;
typedef short               int16_t;
typedef long                int32_t;
typedef long long           int64_t;
*/

// Passed in Debug Level Support
#define MMRC_DBG_TRAIN_RDLVL    0x01000000
#define MMRC_DBG_TRAIN_WRLVL    0x02000000
#define MMRC_DBG_DDRPHY_BLOCKS  0x20000000

typedef UINT32      mmrc_size_t;

#define FORCEODT_ON         1

#define MMRC_MAX_CHANNELS   MAX_CHANNELS
#define MMRC_MAX_RANKS      MAX_RANKS
#define MMRC_MAX_BYTELANES_PER_MODULE       2
#define MMRC_MAX_MODULES    4

#define DELAYREGS_WDQS      0
#define DELAYREGS_RCVN      1
#define DELAYREGS_WDQ       2
#define DELAYREGS_WCMD      3
#define DELAYREGS_WCTL      4
#define DELAYREGS_WCLK      5
#define DELAYREGS_RDQS      6

#define ENABLE  1
#define DISABLE 0

#ifndef NULL
#define NULL                0xffffffff
#endif

#define MAX_FREQS                   2
#define MAX_TYPES                   1
#define MAX_CONFIGS                 7
#define MAX_PLATS                   1

#define FLAG_REGOFFSET              0
//#define FLAG_CRMB                 1

#define SIZE_OFFSET                 4
#define SIZE_REGOFFSET              2
#define SIZE_REGMASK                4
#define SIZE_CRMB                   4
#define SIZE_POWERON                1
#define SIZE_DELAY                  4
#define SIZE_LOOP                   1
#define SIZE_PLAT                   1
#define SIZE_FLAGS                  1
#define SIZE_VALUE                  4
#define SIZE_BITOFFSETLVL           1

#define CEIL(num,den) (num+den-1)/den

#define BC                          1<<7
#define NOBC                        0
#define NOLOOP                      0

//#define TRUE                      1
//#define FALSE                     0

#define JEDEC_LDR(data)             jedec_cmd(LDR, data);

#define CHNCRMB(x)      (UINT8) ((x >> 24) & 0xff)
#define RNKCRMB(x)      (UINT8) ((x >> 16) & 0xff)
#define MODCRMB(x)      (UINT8) ((x >>  8) & 0xff)
#define BLMCRMB(x)      (UINT8) ((x >>  0) & 0xff)

// JEDEC COMMANDS
#define JEDEC_PRECHARGEALL          1
#define JEDEC_LMR                   2
#define JEDEC_REFRESH               3

#define FLAG_EXT_FLAG(flag)         ((flag >> 7) & 1)
#define FLAG_CRMB(flag)             ((flag >> 6) & 1)
#define FLAG_OFFSET(flag)           ((flag >> 5) & 1)
#define FLAG_VALUE(flag)            ((flag >> 4) & 1)
#define FLAG_FREQ(flag)             ((flag >> 3) & 1)
#define FLAG_TYPE(flag)             ((flag >> 2) & 1)
#define FLAG_CONFIG(flag)           ((flag >> 1) & 1)
#define FLAG_LOOP(flag)             ((flag >> 0) & 1)

#define FLAG2_DELAY(flag)           ((flag >> 6) & 1)
#define FLAG2_PLAT(flag)            ((flag >> 5) & 1)
#define FLAG2_CHNOFFSET(flag)       ((flag >> 4) & 1)
#define FLAG2_RNKOFFSET(flag)       ((flag >> 3) & 1)
#define FLAG2_MODOFFSET(flag)       ((flag >> 2) & 1)
#define FLAG2_BLMOFFSET(flag)       ((flag >> 1) & 1)
#define FLAG2_BITOFFSET(flag)       ((flag >> 0) & 1)

#define FLAG3_BITOFFSETLVL(flag)    ((flag >> 1) & 1)
#define FLAG3_POWERON(flag)         ((flag >> 0) & 1)

#define FLAG_REGOFFSET_CRMB(flag)   ((flag >> 7) & 1)


#define BYTES_TO_LONG(x)        (((UINT32)x[0])<<24) | (((UINT32)x[1])<<16) | (((UINT32)x[2])<<8) | (((UINT32)x[3])<<0)
#define BYTES_TO_RSE(x)         (((UINT32)x[2])<<24) | (((UINT32)x[3])<<16) | (((UINT32)x[0])<<8) | (((UINT32)x[1])<<0)
#define BYTES_TO_SHORT(x)       (((UINT16)x[0])<<8) | (((UINT16)x[1])<<0)
/*
#define FOREACH_MOD_BL_BEGIN(mod,bl)    for (mod=0; mod< MAX_MODULES; mod++) { for (bl=0; bl< MAX_BYTELANES_PER_MODULE; bl++) {
#define FOREACH_MOD_BL_END      }}
#define FOREACH_BL_MOD_BEGIN(bl, mod)   for (bl=0; bl< MAX_BYTELANES_PER_MODULE; mod++) { for (mod=0; mod< MAX_MODULES; mod++) {
#define FOREACH_BL_MOD_END      }}
#define FOREACH_CHN_RNK_BEGIN(chn,rnk)  for (chn=0; chn< MAX_CHANNELS; chn++) { for (rnk=0; rnk< MAX_RANKS; rnk++) {
#define FOREACH_CHN_RNK_END     }}
#define FOREACH_RNK_BEGIN(rnk)  for (rnk=0; rnk<MAX_RANKS; rnk++) {
#define FOREACH_RNK_END         }
#define FOREACH_CHN_BEGIN(chn)  for (chn=0; chn< MAX_CHANNELS; chn++) {
#define FOREACH_CHN_END         }
*/
#define CHNENABLE(chn)          ((globalInput.channelEnable_u8>>chn) & 1)
#define RNKENABLE(chn,rnk)      ((globalInput.rankEnable_u8[chn]>>rnk) & 1)
#define RD_LEVELING         1
#define WR_LEVELING         2

#define MARGIN_RDVREF    1
#define MARGIN_RDDELAY   2
#define MARGIN_WRDELAY   3
// Format for every field descriptions within the SOC.
typedef struct IOSF_sb_Reg_s {
  UINT32    offset;                                 // Register Offset relative to the DDRPHy.
  UINT32  start_bit;                                // Starting bit within the register.
  UINT32    stop_bit;                               // Ending bit within the register.
} IOSF_sb_Reg_t;

typedef struct regDesc_s {
  UINT8   totalSize_u8;
  IOSF_sb_Reg_t reg;
  UINT16  offset_u16;
  UINT32  mask_u32;
  UINT32  crmb_u32;
  UINT32  value_u32;
  UINT32  delay_u32;
  UINT8   po_u8;
  UINT8   flags_u8;
  UINT8   flags2_u8;
  UINT8     flags3_u8;
  UINT8     loop_u8;
  UINT8     plat_u8;
  INT16  chnOffset_s16;
  INT16  rnkOffset_s16;
  INT16  modOffset_s16;
  INT16  blmOffset_s16;
  INT16     bitOffset_s16;
  UINT8     bitOffsetLevel_u8;        //00 -> BL, 01->MOD, 10->RK, 11->CH
  UINT8     pFreqTypeConfig_u8[MAX_FREQS*MAX_TYPES*MAX_CONFIGS];
  UINT8  freqTypeConfigSize_u8;
} RegDesc_t;


typedef struct CacheElement_s {
  UINT8       active_u8;
  UINT32  value_u32;
} CacheElement_t;


//Enumeration of PerSelection method which is used on reproducing the
//same values on multiple registers.
typedef enum enum_reg_flags_per_selection {
  e_per_bl          = 0x00,                             // Loop on every BL, every module, and every channel.
  e_per_rank        = 0x01,                             // Loop on every Rank, every BL, every module, and every channel.
  e_per_module      = 0x02,                             // Loop on every module and every channel.
  e_per_channel     = 0x03,                             // Loop on every channel.
  e_per_global      = 0x04,                             // No loop, write once.
} reg_flags_per_selection_t;

// Enumeration list for a register assignment when dependent on frequency.
typedef enum enum_reg_flags_frequency {
  e_freq_800      = 0x01,                             // Value specific for 800 MHz.
  e_freq_1066     = 0x02,                             // Value specific for 1066 MHz.
  e_freq_1333     = 0x04,                             // Value specific for 1333 MHz.
  e_freq_1600     = 0x08,                             // Value specific for 1600 MHz.
  e_freq_1866     = 0x10,                             // Value specific for 1866 MHz.
} reg_flags_frequency_t;

// Enumeration list if a register can be cached, such that a Read/modify/write does not have
// to read if it has been read once already.
typedef enum enum_reg_flags_cacheable {
  e_cache_off     = 0x00,                             // Register is non-cacheable, must be re-read.
  e_cache_on      = 0x01,                             // Register is cachable.
} reg_flags_cacheable_t;

// Enumeration list on whether a register assignment is for reading or writing.
// when reading, the register is polled until the specific bits equal the value.
// or until the timeout period is set.
typedef enum enum_reg_flags_hwpoll {
  e_poll_on       = 0x00,                             // Assignment is a read, poll until value is present.
  e_poll_off      = 0x01,                             // Assignment is a write.
} reg_flags_hwpoll_t;

// Enumeration list of all module types, this is used for when looping on modules,
// the loop will occur on module types.
typedef enum enum_reg_flags_module {
  e_mod_RDLVL     = 0x01,                             // Rx Rcvn
  e_mod_RDQS      = 0x02,                             // Rx DQS
  e_mod_WCMD      = 0x04,                             // Tx CMD
  e_mod_WCTL      = 0x08,                             // Tx CTL  //Data cmd clk ctrl ecc pll comp cmdctl
  e_mod_WDQ       = 0x10,                             // Tx DQ
  e_mod_WRLVL     = 0x11,                             // Tx DQS
} reg_flags_module_t;

// Enumeration list of boot methods, based on how the system is booting the specific register
// assignment will occur.
typedef enum enum_reg_flags_boot {
  e_boot_cold     = 0x01,                             // Powering on the system for the first time.
  e_boot_warm     = 0x02,                             // Software reset.
  e_boot_s3       = 0x04,                             // Suspend to Ram.
  e_boot_quick    = 0x08,                             // ??
} reg_flags_boot_t;

// Flags is composed of the 6 fields as specified above.
typedef struct register_flags_s {
  UINT8                           rsvd:1;             // 1 bit  unused.
  reg_flags_per_selection_t       per_selection:3;    // Looping options on register assignment.
  reg_flags_frequency_t           frequency:5;        // Frequencies supported on register assignmetn.
  reg_flags_cacheable_t           reg_cacheable:1;    // Register can be stored for future use.
  reg_flags_hwpoll_t              hwpoll:1;           // Read/Write
  reg_flags_module_t              module:6;           // Current module.
  reg_flags_boot_t                boot:4;             // Boot methos supported on register assignment.
  //reg_flags_plat_t              plat:4;
} register_flags_t;

// Register assignment structure.
typedef struct regValue_s {
  UINT32                      reg;                // Offset within DDRPHY for current register.
  UINT32                      value;              // Value to be read/written.
  UINT32                      mask;               // mask of bits that should be modified.
  UINT8                       task;               // Task this assigment should display when running.
  UINT32                      delay;              // On Write, length in uSec to wait before continuing.
  // On Read,  length to wait until "value" measure before continuing.
  register_flags_t            flags;              // Flags associated with this register assignment.
#ifdef DEBUG_REGNAME
  char                    *name;              // Debug hook to display the register being modified.
#endif
} regValue_t;

#define RTTNOM_DIS          0x0000
#define RTTNOM_60ohm        0x0004
#define RTTNOM_120ohm       0x0040
#define RTTNOM_40ohm        0x0044
#define RTTNOM_20ohm        0x0200
#define RTTNOM_30ohm        0x0204

#define OUTIMP_40ohm        0x0000
#define OUTIMP_34ohm        0x0002

#define WRITELEVEL_ENABLE   0x0080

#define QBUF_DISABLED       0x1000
#define MR1                 0x10000
/*
typedef struct input_struct {
    uint8_t channelEnable_u8;
    uint8_t rankEnable_u8[5];       // Maximum of 5 channels.
    uint8_t bootMethod_u8;
    uint8_t platSelect_u8;
    uint8_t poSelect_u8;
    uint8_t freqSelect_u8;
    uint8_t DRAMConfigSelect_u8;
    uint8_t DRAMTechSelect_u8;
    uint8_t tcas;
    uint8_t jedec_rttTarget_u8;
    uint8_t jedec_rttNonTarget_u8;
    uint8_t jedec_outputDriverImpedance_u8;
    uint32_t goldenPatternAddr;
} input_t;
*/
// Debug Hooks
#define DEBUG_IOSF          0x000f

#define DEBUG_IOSF_EN       ((DEBUG_IOSF & 0x8000) == 0x8000)
#define DEBUG_IOSF_FUNC     ((DEBUG_IOSF & 0x8001) == 0x8001)
#define DEBUG_IOSF_PARAM    ((DEBUG_IOSF & 0x8003) == 0x8003)
#define DEBUG_IOSFWR_RD     ((DEBUG_IOSF & 0x8004) == 0x8004)
#define DEBUG_IOSFWR_MS     ((DEBUG_IOSF & 0x8008) == 0x8008)
#define DEBUG_IOSFWR_WR     ((DEBUG_IOSF & 0x8010) == 0x8010)
#define DEBUG_IOSFWR_RB     ((DEBUG_IOSF & 0x8020) == 0x8020)       // Readback Verify, print on error.
#define DEBUG_IOSFWR_RBP    ((DEBUG_IOSF & 0x8060) == 0x8060)       // Readback Verify, Print all.

#define DEBUG_RCVN          0x8010

#define DEBUG_RCVN_FUNC     ((DEBUG_RCVN & 0x8001) == 0x8001)
#define DEBUG_RCVN_SET      ((DEBUG_RCVN & 0x8004) == 0x8004)       // RcvenSet with Params
#define DEBUG_RCVN_IND      ((DEBUG_RCVN & 0x800c) == 0x800c)       //
#define DEBUG_RCVN_STEPS    ((DEBUG_RCVN & 0x8010) == 0x8010)       // Show steps of Algorithm

#define DEBUG_WRLVL         0x8010
#define DEBUG_WRLVL_FUNC    ((DEBUG_WRLVL & 0x8001) == 0x8001)
#define DEBUG_WRLVL_STEPS   ((DEBUG_WRLVL & 0x8010) == 0x8010)

#define DEBUG_SET_DELAYREG  0x0003
#define DEBUG_SET_DELAYREG_FUNC     ((DEBUG_SET_DELAYREG & 0x8001) == 0x8001)
#define DEBUG_SET_DELAYREG_IND      ((DEBUG_SET_DELAYREG & 0x8002) == 0x8002)

#define DEBUG_JEDEC         0x8000
#define DEBUG_JEDEC_FUNC    ((DEBUG_JEDEC & 0x8001) == 0x8001)

#define DEBUG_RDWRLEVELFINESEARCH   0x8002
#define DEBUG_RDWRLEVELFINESEARCH_FUNC  ((DEBUG_RDWRLEVELFINESEARCH & 0x8001) == 0x8001)
#define DEBUG_RDWRLEVELFINESEARCH_STEP  ((DEBUG_RDWRLEVELFINESEARCH & 0x8002) == 0x8002)
#define DEBUG_RDWRLEVELFINESEARCH_VALUE ((DEBUG_RDWRLEVELFINESEARCH & 0x8004) == 0x8004)

extern int lprintf(const char *fmt, ...);

// Structure definition of how the starting/ending bits are to be provided to
// the Modular MRC, they are just two 8-bit numbers.  The last-bit of the
// ending bit is the flag to specify whether another Starting/ending is to be
// provided.
typedef struct RoseSE_s {
  UINT8 startingBit_u8;
  UINT8 endingBit_u8:7;
  UINT8 continueFlag_u8:1;
} RoseSE_t;

typedef struct RoseBinData_s {
  UINT16 regoffset_u6;
  RoseSE_t roseSE[10];
} RoseBinData_t;

typedef struct Rose_s {
  UINT16 regOffset_u16;
  UINT32 mask_u32;
} Rose_t;

typedef struct RegLocation_s {
  UINT8 flags_u8;
  Rose_t  rose;
} RegLocation_t;


#endif //_MODMRC_TYPES_H_