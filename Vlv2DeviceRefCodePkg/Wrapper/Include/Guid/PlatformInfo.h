/*++

Copyright (c)  1999 - 2009, Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PlatformInfo.h

Abstract:

  GUID used for Platform Info Data entries in the HOB list.

--*/

#ifndef _PLATFORM_INFO_GUID_H_
#define _PLATFORM_INFO_GUID_H_

#include <PiPei.h>

#include <Library/HobLib.h>
#include <Library/IoLib.h>
//#include <Library/DebugLib.h>
#include <Library/SmbusLib.h>
//#include <Ppi/Smbus.h>
#include <IndustryStandard/Smbus.h>

#define PLATFORM_INFO_REVISION = 1      // revision id for current platform information struct
//
// Start::BayLake Board Defines
//
#define BOARD_REVISION_DEFAULT = 0xff
#define UNKNOWN_FABID		0x0F
#define FAB_ID_MASK			0x0F
#define BOARD_ID_2   0x01
#define BOARD_ID_1   0x40
#define BOARD_ID_0   0x04

#define BOARD_ID_DT_CRB     0x0
#define BOARD_ID_DT_VLVR    0x1
#define BOARD_ID_SVP_VLV    0xC
#define BOARD_ID_SVP_EV_VLV 0xD
//
// End::BayLake Board Defines
//

//
// Start::Alpine Valley Board Defines
//
//#define BOARD_ID_AV_SVP     1492 //0x5D4, AlpineValley Board id
#define DC_ID_DDR3L      0x00
#define DC_ID_DDR3       0x04
#define DC_ID_LPDDR3     0x02
#define DC_ID_LPDDR2     0x06
#define DC_ID_DDR4       0x01
#define DC_ID_DDR3L_ECC  0x05
#define DC_ID_NO_MEM     0x07
//
// End::Alpine Valley Board Defines
//

//
// Start::Bayley Bay Board ID
//
//#define BOARD_ID_AV_SVP     1492 //0x5D4, AlpineValley Board id
//#define DC_ID_DDR3L      0x00
//#define DC_ID_DDR3       0x04
//#define DC_ID_LPDDR3     0x02
//#define DC_ID_LPDDR2     0x06
//#define DC_ID_DDR4       0x01
//#define DC_ID_DDR3L_ECC  0x05
//#define DC_ID_NO_MEM     0x07
//
// End::Bayley Bay Board Defines
//

#define MAX_FAB_ID_RETRY_COUNT  100
#define MAX_FAB_ID_CHECK_COUNT  3

#define PLATFORM_INFO_HOB_REVISION	0x1

#define EFI_PLATFORM_INFO_GUID \
  { \
    0x1e2acc41, 0xe26a, 0x483d, 0xaf, 0xc7, 0xa0, 0x56, 0xc3, 0x4e, 0x8, 0x7b \
  }

extern EFI_GUID gEfiPlatformInfoGuid;


typedef enum {
    FlavorUnknown = 0,
    //
    // Mobile
    //
    FlavorMobile = 1,
    //
    // Desktop
    //
    FlavorDesktop = 2,
    //
    // Tablet
    //
    FlavorTablet = 3
} PLATFORM_FLAVOR;

#pragma pack(1)

typedef struct {
    UINT16  PciResourceIoBase;
    UINT16  PciResourceIoLimit;
    UINT32  PciResourceMem32Base;
    UINT32  PciResourceMem32Limit;
    UINT64  PciResourceMem64Base;
    UINT64  PciResourceMem64Limit;
    UINT64  PciExpressBase;
    UINT32  PciExpressSize;
    UINT8   PciHostAddressWidth;
    UINT8   PciResourceMinSecBus;
} EFI_PLATFORM_PCI_DATA;

typedef struct {
    UINT8 CpuAddressWidth;
    UINT32 CpuFamilyStepping;
} EFI_PLATFORM_CPU_DATA;

typedef struct {
    UINT8 SysIoApicEnable;
    UINT8 SysSioExist;
} EFI_PLATFORM_SYS_DATA;

typedef struct {
    UINT32  MemTolm;
    UINT32  MemMaxTolm;
    UINT32  MemTsegSize;
    UINT32  MemTsegBase;
    UINT32  MemIedSize;
    UINT32  MemIgdSize;
    UINT32  MemIgdBase;
    UINT32  MemIgdGttSize;
    UINT32  MemIgdGttBase;
    UINT64  MemMir0;
    UINT64  MemMir1;
    UINT32  MemConfigSize;
    UINT16  MmioSize;
} EFI_PLATFORM_MEM_DATA;

typedef struct {
    UINT32 IgdOpRegionAddress;    // IGD OpRegion Starting Address
    UINT8  IgdBootType;           // IGD Boot Display Device
    UINT8  IgdPanelType;          // IGD Panel Type CMOs option
    UINT8  IgdTvFormat;           // IGD TV Format CMOS option
    UINT8  IgdTvMinor;            // IGD TV Minor Format CMOS option
    UINT8  IgdPanelScaling;       // IGD Panel Scaling
    UINT8  IgdBlcConfig;          // IGD BLC Configuration
    UINT8  IgdBiaConfig;          // IGD BIA Configuration
    UINT8  IgdSscConfig;          // IGD SSC Configuration
    UINT8  IgdDvmtMemSize;        // IGD DVMT Memory Size
    UINT8  IgdFunc1Enable;        // IGD Function 1 Enable
    UINT8  IgdHpllVco;            // HPLL VCO
    UINT8  IgdSciSmiMode;         // GMCH SMI/SCI mode (0=SCI)
    UINT8  IgdPAVP;               // IGD PAVP data
} EFI_PLATFORM_IGD_DATA;

typedef enum {
  BOARD_ID_AV_SVP         = 0x0,    // Alpine Valley Board
  BOARD_ID_BL_RVP         = 0x2,    // BayLake Board (RVP)
  BOARD_ID_BL_FFRD8       = 0x3,    // BayLake Board (FFRD)
  BOARD_ID_BL_FFRD        = 0x4,    // BayLake Board (FFRD)
  BOARD_ID_BL_RVP_DDR3L   = 0x5,    // BayLake Board (FFRD)
  BOARD_ID_BL_STHI        = 0x7,    // PPV- STHI Board
  BOARD_ID_BB_RVP         = 0x20,   // Bayley Bay Board
  BOARD_ID_BS_RVP         = 0x30,   // Bakersport Board
  BOARD_ID_CVH            = 0x90    // Crestview Hills

} BOARD_ID_LIST;

//(CSP20130313E+)>>
typedef enum {
    FAB1 = 0,
    FAB2 = 1,
    FAB3 = 2
} FAB_ID_LIST;
//(CSP20130313E+)<<

typedef enum {
    PR0  = 0,   // FFRD10 PR0
    PR05 = 1,   // FFRD10 PR0.3 and PR 0.5
    PR1  = 2,   // FFRD10 PR1
    PR11 = 3    // FFRD10 PR1.1
} FFRD_ID_LIST;

//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
typedef enum {
    FFRD_8_PR0  = 1,  // FFRD8 PR0
    FFRD_8_PR1  = 2   // FFRD8 PR1
} FFRD_8_ID_LIST;
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<

//
// VLV2 GPIO GROUP OFFSET
//
#define GPIO_SCORE_OFFSET	0x0000
#define GPIO_NCORE_OFFSET	0x1000
#define GPIO_SSUS_OFFSET	0x2000

//
// GPIO Initialization Data Structure for BayLake
// SC = SCORE, SS= SSUS
// Note: NC doesn't support GPIO functionality in IO access mode, only support in MMIO access mode
//

//
//  IO space
//

typedef struct {
    UINT32  Use_Sel_SC0;
    UINT32  Use_Sel_SC1;
    UINT32  Use_Sel_SC2;
    UINT32  Use_Sel_SS;

    UINT32  Io_Sel_SC0;
    UINT32  Io_Sel_SC1;
    UINT32  Io_Sel_SC2;
    UINT32  Io_Sel_SS;

    UINT32  GP_Lvl_SC0;
    UINT32  GP_Lvl_SC1;
    UINT32  GP_Lvl_SC2;
    UINT32  GP_Lvl_SS;

    UINT32  TPE_SC0;
    UINT32  TPE_SS;

    UINT32  TNE_SC0;
    UINT32  TNE_SS;

    UINT32  TS_SC0;
    UINT32  TS_SS;

    UINT32  WE_SS;
} CFIO_INIT_STRUCT;



/////////////////////////////////////////
//    CFIO PAD configuration Registers //
/////////////////////////////////////////

//
// Memory space
//

typedef union {
    UINT32 dw;
    struct {
        UINT32 Func_Pin_Mux:3;  // 0:2 Function of CFIO selection
        UINT32 ipslew:2; // 3:4 Pad (P) Slew Rate Controls PAD slew rate check Width
        UINT32 inslew:2; // 5:6 Pad (N) Slew Rate Controls PAD slew rate
        UINT32 Pull_assign:2; // 7:8 Pull assignment
        UINT32 Pull_strength:2; // 9:10 Pull strength
        UINT32 Bypass_flop:1; // 11 Bypass flop
        UINT32 Filter_en:1; // 12 Filter Enable
        UINT32 Hist_ctrl:2; // 13:14 hysteresis control
        UINT32 Hist_enb:1; // 15 Hysteresis enable, active low
        UINT32 Delay_line:6; // 16:21 Delay line values - Delay values for input or output
        UINT32 Reserved:3; // 22:24 Reserved
        UINT32 TPE:1; // 25 Trigger Positive Edge Enable
        UINT32 TNE:1; // 26 Trigger Negative Edge Enable
        UINT32 DirectIrqEn:1; // 27 //CSP20130930 change setting with iBIOS setting 
        UINT32 Reserved2:2; // 28:29 Reserved  //CSP20130930 change setting with iBIOS setting 
        UINT32 i1p5sel:1; // 30
        UINT32 IODEN:1; // 31 : Open Drain enable. Active high
    } r;
} PAD_CONF0;

typedef union {
    UINT32 dw;
    struct {
        UINT32 instr:16; // 0:15 Pad (N) strength.
        UINT32 ipstr:16; // 16:31 Pad (P) strength.
    } r;
} PAD_CONF1;

typedef union {
    UINT32 dw;
    struct {
        UINT32 pad_val:1; // 0 These registers are implemented as dual read/write with dedicated storage each.
        UINT32 ioutenb:1; // 1 output enable
        UINT32 iinenb:1; // 2 input enable
        UINT32 Reserved:29; // 3:31 Reserved
    } r;
} PAD_VAL;

typedef union {
    UINT32 GPI;
    struct {
        UINT32 ihbpen:1; // 0 Pad high by pass enable
        UINT32 ihbpinen:1; // 1 Pad high by pass input
        UINT32 instaticen:1; // 2 TBD
        UINT32 ipstaticen:1; // 3 TBD
        UINT32 Overide_strap_pin :1; // 4 DFX indicates if it wants to override the strap pin value on this pad, if exists
        UINT32 Overide_strap_pin_val:1; // 5 In case DFX need to override strap pin value and it exist for the specific pad, this value will beused
        UINT32 TestMode_Pin_Mux:3; // 6:9 DFX Pin Muxing
    } r;
} PAD_DFT;

// GPIO_USAGE value need to matche the PAD_VAL input/output enable bits
typedef enum {
  Native = 0xFF,  // Native, no need to set PAD_VALUE
  GPI = 2,    // GPI, input only in PAD_VALUE
  GPO = 0,    // W/A for PAD_VAL silicon readback issue //4GPO, output only in PAD_VALUE
  //GPO = 4,    // GPO, output only in PAD_VALUE
  GPIO = 0,      // GPIO, input & output
  TRISTS = 6,      // Tri-State
  GPIO_NONE
} GPIO_USAGE;

typedef enum {
    LO = 0,
    HI = 1,
    NA = 0xFF
} GPO_D4;

//(CSP20130313C+)>>
typedef enum {
    F0 = 0,
    F1 = 1,
    F2 = 2,
    F3 = 3,
    F4 = 4,
    F5 = 5,
    F6 = 6,
    F7 = 7
} GPIO_FUNC_NUM;
//(CSP20130313C+)<<

typedef enum {
    YES = 1,
    NO = 0
} INT_CAPABLE;

// Mapping to CONF0 bit 27:24
// Note: Assume "Direct Irq En" is not set, unless specially notified
//(CSP20130221D+)>>
typedef enum {
    TRIG_ = 0,

    TRIG_Edge_High = /*BIT3 |*/ BIT1,	// Positive Edge (Rasing)
    TRIG_Edge_Low  = /*BIT3 |*/ BIT2,	// Negative Edge (Falling)
    TRIG_Edge_Both = /*BIT3 |*/ BIT2 | BIT1,	// Both Edge
    TRIG_Level_High= /*BIT3 |*/ BIT1 | BIT0,	// Level High
    TRIG_Level_Low = /*BIT3 |*/ BIT2 | BIT0,	// Level Low
} INT_TYPE;

typedef enum {
    P_20K_H,	// Pull Up 20K
    P_20K_L,	// Pull Down 20K
    P_10K_H,	// Pull Up 10K
    P_10K_L,	// Pull Down 10K
    P_2K_H, // Pull Up 2K //CSP20130930 change setting with iBIOS setting 
    P_2K_L, // Pull Down 2K //CSP20130930 change setting with iBIOS setting 
    P_NONE      // Pull None
} PULL_TYPE;
//(CSP20130221D+)<<

#ifdef EFI_DEBUG
#define GPIO_INIT_ITEM(pad_name, usage, gpod4, func, int_cap, int_type, pull, offset) {pad_name, usage, gpod4, func, /*int_cap,*/ TRIG_##int_type, P_##pull, offset}
#else
#define GPIO_INIT_ITEM(pad_name, usage, gpod4, func, int_cap, int_type, pull, offset) {          usage, gpod4, func, /*int_cap,*/ TRIG_##int_type, P_##pull, offset}
#endif

//
// GPIO CONF & PAD Initialization Data Structure for BayLake GPIOs bits
// NC = NCORE, SC = SCORE, SS= SSUS
//

typedef struct {

#ifdef EFI_DEBUG
    char			pad_name[32];// GPIO Pin Name for debug purpose
#endif

    GPIO_USAGE		usage;		// GPIO pin used as Native mode or GPI/GPO/GPIO mode
    GPO_D4          gpod4;      // GPO default value
    GPIO_FUNC_NUM	func;		// Function Number (F0~F7)
    //INT_CAPABLE		int_cap;	// YES: Int capable; NO: Int not capable (Not actually used in code)
    INT_TYPE		int_type;	// Edge or Level trigger, low or high active
    PULL_TYPE		pull;		// Pull Up or Down
    UINT8			offset;		// Equal with (PCONF0 register offset >> 4 bits)
} GPIO_CONF_PAD_INIT;


/* alns
//
// This HOB definition must be consistent with what is created in the
// PlatformInfo protocol definition.  This way the information in the
// HOB can be directly copied upon the protocol and only the strings
// will need to be updated.
//
typedef struct _MULTIPLATFORM_INFO {
    UINT8                       RevisonId;              // Structure Revision ID
    PLATFORM_FLAVOR             PlatformFlavor;         // Platform Flavor
    BOARD_ID_LIST               BoardId;                // Board ID
    UINT8                       BoardRev;               // Board Revision
    EFI_GUID                    PlatformGuid;           // Platform Guid
    GPIO_INIT_STRUCT            PlatformGpioInit;
  UINT16                      IohSku;
  UINT8                       IohRevision;
  UINT16                      IchSku;
  UINT8                       IchRevision;
  EFI_PLATFORM_PCI_DATA       PciData;
  EFI_PLATFORM_CPU_DATA       CpuData;
  EFI_PLATFORM_MEM_DATA       MemData;
  EFI_PLATFORM_SYS_DATA       SysData;
  EFI_PLATFORM_IGD_DATA       IgdData;
} MULTIPLATFORM_INFO;


//
// This HOB definition.
//
typedef struct _PLATFORM_INFORMATION_HOB {
    EFI_HOB_GUID_TYPE           Header;
    MULTIPLATFORM_INFO          PlatformInfo;
} MULTIPLATFORM_INFORMATION_HOB;
*/
//typedef UINT64 BOARD_FEATURES

///
/// SgMode settings
///
typedef enum {
  SgModeDisabled = 0,
  SgModeMuxed,
  SgModeMuxless,
  SgModeDgpu,
  SgModeMax
} SG_MODE;

///
/// SA Info HOB
///
typedef struct _SG_INFO_HOB {
//  EFI_HOB_GUID_TYPE EfiHobGuidType;
  UINT8             RevisionId;     ///< Revision ID
  SG_MODE           SgMode;
  BOOLEAN           SgGpioSupport;  ///< 1=Supported; 0=Not Supported
  UINT8             SgDgpuPwrOK;
  UINT8             SgDgpuHoldRst;
  UINT8             SgDgpuPwrEnable;
  UINT8             SgDgpuPrsnt;
} SG_INFO_HOB;

typedef struct _EFI_PLATFORM_INFO_HOB {
    UINT16                      PlatformType; // Platform Type
    UINT8                       BoardId;             // Board ID
    UINT8                       BoardRev;            // Board Revision
    PLATFORM_FLAVOR             PlatformFlavor;      // Platform Flavor
    UINT8                       DDRDaughterCardCh0Id;// DDR daughter card channel 0 id
    UINT8                       DDRDaughterCardCh1Id;// DDR daughter card channel 1 id
    UINT8                       ECOId;               // ECO applied on platform
    UINT16                      IohSku;
    UINT8                       IohRevision;
    UINT16                      IchSku;
    UINT8                       IchRevision;
    UINT32                      SsidSvid;
    UINT16                      AudioSubsystemDeviceId;
    UINT8                       AcpiOemId[6];
    UINT8                       AcpiOemTableId[8];
    UINT16                      MemCfgID;
    UINT16                      FABID;
    SG_INFO_HOB                 SgInfo;
    UINT8                       RevisonId;           // Structure Revision ID
    EFI_PLATFORM_PCI_DATA       PciData;
    EFI_PLATFORM_CPU_DATA       CpuData;
    EFI_PLATFORM_MEM_DATA       MemData;
    EFI_PLATFORM_SYS_DATA       SysData;
    EFI_PLATFORM_IGD_DATA       IgdData;
    CFIO_INIT_STRUCT*           PlatformCfioData;
    GPIO_CONF_PAD_INIT*         PlatformGpioData_NC;
    GPIO_CONF_PAD_INIT*         PlatformGpioData_SC;
    GPIO_CONF_PAD_INIT*         PlatformGpioData_SUS;
//  BOARD_FEATURES              BoardFeatures;
} EFI_PLATFORM_INFO_HOB;

#pragma pack()

EFI_STATUS
GetPlatformInfoHob(
    IN CONST EFI_PEI_SERVICES           **PeiServices,
    OUT EFI_PLATFORM_INFO_HOB     **PlatformInfoHob
);


EFI_STATUS
InstallPlatformClocksNotify(
    IN CONST EFI_PEI_SERVICES           **PeiServices
);

EFI_STATUS
InstallPlatformSysCtrlGPIONotify(
    IN CONST EFI_PEI_SERVICES           **PeiServices
);

#endif
