/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  Valleyview.h

Abstract:

  This header file provides common definitions just for Valleyview-SOC using to avoid including extra module's file.

--*/

#ifndef _MC_H_INCLUDED_
#define _MC_H_INCLUDED_
/*
< Extended Configuration Base Address.*/
#define EC_BASE             0xE0000000

/* VLV Message Bus Units Port ID*/
#define VLV_AUNIT       0x00        /*SSA-A Unit (IO Arbiter)*/
#define VLV_DUNIT       0x01        /*D Unit*/
#define VLV_TUNIT       0x02        /*SSA-T Unit*/
#define VLV_BUNIT       0x03        /*SSA-B Unit*/
#define VLV_PUNIT       0x04        /*P Unit*/
#define VLV_DFXUNIT     0x05        /*SSA-DFX Unit*/
#define VLV_GUNIT       0x06        /*G Unit*/

#define VLV_CUNIT       0x08        /*C Unit*/
#define VLV_sVID        0x0A        /*VID Controller*/
#define VLV_DDRIO       0x0C        /*DDR IO Unit*/
#define VLV_REUT        0x0D        /*Memory REUT*/
#define VLV_DRNG        0x0F        /*Random Number Generator*/
#define VLV_DSIPCONT    0x10        /*Display Controller*/
#define VLV_FUSEEPNC    0x11        /*Fuse EndPoint Nort*/
#define VLV_DISPPHY     0x12        /*eDP0/eDP1/Dp/HDMI Phy (ModPhy)*/
#define VLV_GPIONC      0x13        /*GPIO (North) Controller*/
#define VLV_CCK         0x14        /*Clock Controller*/
#define VLV_DFXNC       0x16        /*DFX-NC*/
#define VLV_DFXSOC      0x15        /*DFX-SOC*/
#define VLV_DFXVISA     0x18        /*DFX-VISA*/
#define VLV_DFXLAKEMORE 0x17        /*DFX-Lakemore*/
//
// DEVICE 0 (Memroy Controller Hub)
//
#define MC_BUS          0x00
#define MC_DEV          0x00
#define MC_DEV2         0x02
#define MC_FUN          0x00
// NC DEV 0 Vendor and Device IDs
#define MC_VID          0x8086
#define MC_DID_OFFSET   0x2         //Device Identification
#define MC_GGC_OFFSET   0x50        //GMCH Graphics Control Register

// VLV Message Bus Register Definitions
//Common for A/B/D/T/SVID/CCK units
#define VLV_MBR_READ_CMD            0x10000000
#define VLV_MBR_WRITE_CMD           0x11000000

//Common for Gunit/DISPPHY units
#define VLV_MBR_GDISPPHYREAD_CMD    0x00000000
#define VLV_MBR_GDISPPHYWRITE_CMD   0x01000000

//Common for Punit/DFX/GPIONC/DFXSOC/DFXNC/DFXLAKSEMORE/DFXVISA units
#define VLV_MBR_PDFXGPIOREAD_CMD    0x06000000
#define VLV_MBR_PDFXGPIOWRITE_CMD   0x07000000

//Msg Bus Registers
#define MC_MCR          0x000000D0      //Cunit Message Control Register
#define MC_MDR          0x000000D4      //Cunit Message Data Register
#define MC_MCRX         0x000000D8      //Cunit Message Control Register Extension


#define MC_DEVEN_OFFSET     0x54        //Device Enable
#define B_DEVEN_D2F0EN      BIT3        // Internal Graphics Engine F0 Enable


//smBiosMemory.c use this
//PunitDriver.c
#define MC_TSEGMB_OFFSET    0xAC        //TSEG Memory Base
//
//VLV Units Registers Definition
//Register Symbol       Register Start      //Register Name
//
// VLV SSA-AUnit (IO Aribter)  Register Offset
//
#define AUNIT_ACRCP         0x00
#define AUNIT_ACRRAC        0x01
#define AUNIT_ACRWAC        0x02
#define AUNIT_ADCRCP        0x03
#define AUNIT_ADCRRAC       0x04
#define AUNIT_ADCRWAC       0x05
#define AUNIT_ATCCRCP       0x06
#define AUNIT_ATCCRRAC      0x07
#define AUNIT_ATCCRWAC      0x08
#define AUNIT_ASECCRCP      0x09
#define AUNIT_ASECCRRAC     0x0A
#define AUNIT_ASECCRWAC     0x0B
#define AUNIT_ACF8          0x10
#define AUNIT_ADMIOCMP      0x11
#define AUNIT_ACKGATE       0x18
#define AUNIT_AISOCHCTL     0x20
#define AUNIT_AVCCTL        0x21
#define AUNIT_APEERBASE     0x30
#define AUNIT_APEERLIMIT    0x31
#define AUNIT_AVIB          0x32
#define AUNIT_AVQR          0x33
#define AUNIT_ATBR          0x40
#define AUNIT_AB0BR         0x48
#define AUNIT_AB1BR         0x49
#define AUNIT_AB2BR         0x4A
#define AUNIT_AB3BR         0x4B
#define AUNIT_ACFCACV       0x60
#define AUNIT_ASBACV0       0x61
#define AUNIT_ASBACV1       0x62
#define AUNIT_AMIRRORCTL    0x8A
#define AUNIT_AIODCTL0      0x90
#define AUNIT_AIODCTL1      0x91
#define AUNIT_AIODCTL2      0x92
#define AUNIT_AIODUSMCH0    0x93
#define AUNIT_AIODUSMCH1    0x94
#define AUNIT_AIODUSMCH2    0x95
#define AUNIT_AIODUSMCH3    0x96
#define AUNIT_AIODUSMCH4    0x97
#define AUNIT_AIODUSMCH5    0x98
#define AUNIT_IODUSMSK0     0x99
#define AUNIT_AIODUSMSK1    0x9A
#define AUNIT_AIODUSMSK2    0x9B
#define AUNIT_AIODUSMSK3    0x9C
#define AUNIT_AIODUSMSK4    0x9D
#define AUNIT_AIODUSMSK5    0x9E
#define AUNIT_AIODDSMCH0    0x9F
#define AUNIT_AIODDSMCH1    0xA0
#define AUNIT_AIODDSMCH2    0xA1
#define AUNIT_AIODDSMCH3    0xA2
#define AUNIT_AIODDSMCH4    0xA3
#define AUNIT_AIODDSMCH5    0xA4
#define AUNIT_AIODDSMSK0    0xA5
#define AUNIT_AIODDSMSK1    0xA6
#define AUNIT_AIODDSMSK2    0xA7
#define AUNIT_AIODDSMSK3    0xA8
#define AUNIT_AIODDSMSK4    0xA9
#define AUNIT_AIODDSMSK5    0xAA

//
// VLV SSA-BUnit (System Memory Arbiter)  Register Offset
//
//Updated by YJ 1 March 2011 referred to SSA BUNIT HAS 0.9
#define BUNIT_BSECCP        0x00        //BUnit Security Control Policy (BSECCP)
#define BUNIT_BSECRAC       0x01        //BUnit Security Register Read Access Control (BSECRAC)
#define BUNIT_BSECWAC       0x02        //BUnit Security Register Write Access Control (BSECWAC)
#define BUNIT_BARBCTRL0     0x03        //BUnit Arbiter Control (BARBCTRL0)
#define BUNIT_BARBCTRL1     0x04        //BUnit Arbiter Control (BARBCTRL1)
#define BUNIT_BARBCTRL2     0x05        //BUnit Arbiter Control (BARBCTRL2)
#define BUNIT_BARBCTRL3     0x06        //BUnit Arbiter Control (BARBCTRL3)
#define BUNIT_BWFLUSH       0x07        //BUnit Write Flush Policy (BWFLUSH)
#define BUNIT_BBANKMASK     0x08        //BUnit Bank Mask (BBANKMASK)
#define BUNIT_BROWMASK      0x09        //BUnit Row Mask (BROWMASK)
#define BUNIT_BRANKMASK     0x0A        //BUnit Rank Mask (BRANKMASK)
#define BUNIT_BALIMIT0      0x0B        //BUnit Agent Limit Control (BALIMIT0)
#define BUNIT_BALIMIT1      0x0C        //BUnit Agent Limit Control (BALIMIT1)
#define BUNIT_BALIMIT2      0x0D        //BUnit Agent Limit Control (BALIMIT2)
#define BUNIT_BALIMIT3      0x0E        //BUnit Agent Limit Control (BALIMIT3)
#define BUNIT_BARES0        0x0F        //BUnit Agent Reservations (BARES0)
#define BUNIT_BARES1        0x10        //BUnit Agent Reservations (BARES1)
#define BUNIT_BISOC         0x11        //BUnit ISOC Configuration (BISOC)
#define BUNIT_BCOSCAT       0x12        //BUnit COS Category (BCOSCAT)
#define BUNIT_BDPT          0x13        //BUnit Dynamic Prefetch Throttle (BDPT)
#define BUNIT_BFLWT         0x14        //BUnit Flush Weights (BFLWT)
#define BUNIT_BBWC          0x15        //BUnit Bandwidth Counters (BBWC)
#define BUNIT_BSCHCTRL0     0x18        //Bunit Scheduler Control (BSCHCTRL0)
#define BUNIT_BSCHCTRL1     0x19        //Bunit Scheduler Control (BSCHCTRL1)
#define BUNIT_BIMRDATA          0x1A        //BUnit Protected Memory Region Data Value (BIMRDATA)
#define BUNIT_BPMRVCTL          0x1B        //BUnit Protected Memory Range Violation Control (BPMRVCTL)
#define BUNIT_B_SECURITY_STAT0  0x1C        //B_SECURITY_STAT0
#define BUNIT_B_SECURITY_STAT1  0x1D        //B_SECURITY_STAT0


#define BUNIT_BMRCP         0x20        //BUnit Memory Range Control Policy (BMRCP)
#define BUNIT_BMRRAC        0x21        //BUnit Memory Range Read Access Control (BMRRAC)
#define BUNIT_BMRWAC        0x22        //BUnit Memory Range Write Access Control (BMRWAC)
#define BUNIT_BNOCACHE      0x23        //BUnit Non-Cached Region (BNOCACHE)
#define BUNIT_BNOCACHECTL   0x24        //BUnit Non-Cached Region (BNOCACHECTL)
#define BUNIT_BMBOUND       0x25        //BMBOUND - Bunit Memory/IO Boundary Register
#define BUNIT_BMBOUND_HI    0x26        //BMBOUND HI - Bunit Memory/IO HI Boundary Register
#define BUNIT_BECREG        0x27        //BECREG - Bunit Extended Configuration Space Config
#define BUNIT_BMISC         0x28        //BMISC - Bunit Miscellaneous Configuration Register
#define     B_BMISC_RESDRAM 0x01    //Bit 0 - When this bit is set, reads targeting E-segment are routed to DRAM.
#define     B_BMISC_RFSDRAM 0x02    //Bit 1 - When this bit is set, reads targeting F-segment are routed to DRAM.
#define BUNIT_BSMRCP        0x2B        //BUnit System Management Range Control Policy (BSMRCP)
#define     BSMRCP_SMM_CTRL_REG_LOCK                0x00    //Dont allow any access to the register until the system is reset
#define BUNIT_BSMRRAC       0x2C        //BUnit SMM Range Read Access Control (BSMRRAC)
#define BUNIT_BSMRWAC       0x2D        //BUnit SMM Range Write Access Control (BSMRWAC)
#define     BSMRRAC_SMM_WRITE_OPEN_FOR_ALL_CORE     0xFF    //Allow access only to all CPU HOST
#define     BSMRRAC_SMM_WRITE_CLOSED_FOR_IA_SMM     0x04    //Allow access only to CPU HOST IA SMM
#define     BSMRWAC_SMM_WRITE_OPEN_FOR_ALL_CORE     0xFF    //Allow access only to all CPU HOST
#define     BSMRWAC_SMM_WRITE_CLOSED_FOR_IA_SMM     0x04    //Allow access only to CPU HOST IA SMM
#define BUNIT_BSMMRRL       0x2E        //BSMMRRL - Bunit System Management Range Register Low
#define BUNIT_BSMMRRH       0x2F        //BSMMRRH - Bunit System Management Range Register Hi

#define BUNIT_BDBCP         0x38        //BUnit Debug Control Policy (BDBCP)
#define BUNIT_BDRRAC        0x39        //BUnit Debug Config Register Read Access Control (BDRRAC)
#define BUNIT_BDRWAC        0x3A        //Bunit Debug Config Register Write Access Control (BCRWAC)
#define BUNIT_BDEBUG0       0x3B        //Bunit Debug Register 0 (BDEBUG0)
#define BUNIT_BDEBUG1       0x3C        //Bunit Debug Register 1 (BDEBUG1)
#define BUNIT_BCTRL         0x3D        //Bunit Control (BCTRL)
#define BUNIT_BTHCTRL       0x3E        //BUnit Throttling Control (BTHCTRL)
#define BUNIT_BTHMASK       0x3F        //Bunit Throttling Masks (BTHMASK)

#define BUNIT_BIACP         0x40        //BUnit IA Core Control Policy (BIACP)
#define BUNIT_BIARAC        0x41        //Bunit IA Core Config Register Read Access Control (BIARAC)
#define BUNIT_BIAWAC        0x42        //Bunit IA Core Config Register Write Access Control (BIAWAC)

#define BUNIT_BEXMCP        0x43        //BUnit Extended Micro-Code Control Policy (BEXMCP)
#define BUNIT_BEXMRAC       0x44        //BUnit Extended Micro-Code Read Access Control (BEXMRAC)
#define BUNIT_BEXMWAC       0x45        //BUnit Extended Micro-Code Write Access Control (BEXMWAC)
#define BUNIT_EXML          0x46        //Extended Micro-Code Range Low (EXML)
#define BUNIT_EXMH          0x47        //Extended Micro-Code Range High (EXMH)

#define BUNIT_LP0Mode       0x48        //Logical Processor 0 Mode Register (LP0Mode)
#define BUNIT_LP1Mode       0x49        //Logical Processor 1 Mode Register (LP1Mode)
#define BUNIT_LP2Mode       0x4A        //Logical Processor 2 Mode Register (LP2Mode)
#define BUNIT_LP3Mode       0x4B        //Logical Processor 3 Mode Register (LP3Mode)

#define BUNIT_EMONCTL0      0x50        //BUnit EMON Control 0 (EMONCTL0)
#define BUNIT_EMONCTL1      0x51        //BUnit EMON Control 1 (EMONCTL1)
#define BUNIT_EMONCTL2      0x52        //BUnit EMON Control 2 (EMONCTL2)
#define BUNIT_EMONCTL3      0x53        //BUnit EMON Control 3 (EMONCTL3)

#define BUNIT_BIMR0CP       0x60        //BUnit Isolated Memory Region 0 Control Policy (BIMR0CP)
#define BUNIT_BIMR1CP       0x61        //BUnit Isolated Memory Region 1 Control Policy (BIMR1CP)
#define BUNIT_BIMR2CP       0x62        //BUnit Isolated Memory Region 2 Control Policy (BIMR2CP)
#define BUNIT_BIMR3CP       0x63        //BUnit Isolated Memory Region 3 Control Policy (BIMR3CP)
#define BUNIT_BIMR4CP       0x64        //BUnit Isolated Memory Region 4 Control Policy (BIMR4CP)
#define BUNIT_BIMR5CP       0x65        //BUnit Isolated Memory Region 5 Control Policy (BIMR5CP)
#define BUNIT_BIMR6CP       0x66        //BUnit Isolated Memory Region 6 Control Policy (BIMR6CP)
#define BUNIT_BIMR7CP       0x67        //BUnit Isolated Memory Region 7 Control Policy (BIMR7CP)
#define BUNIT_BIMR8CP       0x68        //BUnit Isolated Memory Region 8 Control Policy (BIMR8CP)
#define BUNIT_BIMR9CP       0x69        //BUnit Isolated Memory Region 9 Control Policy (BIMR9CP)
#define BUNIT_BIMR10CP      0x6A        //BUnit Isolated Memory Region 10 Control Policy (BIMR10CP)
#define BUNIT_BIMR11CP      0x6B        //BUnit Isolated Memory Region 11 Control Policy (BIMR11CP)
#define BUNIT_BIMR12CP      0x6C        //BUnit Isolated Memory Region 12 Control Policy (BIMR12CP)
#define BUNIT_BIMR13CP      0x6D        //BUnit Isolated Memory Region 13 Control Policy (BIMR13CP)
#define BUNIT_BIMR14CP      0x6E        //BUnit Isolated Memory Region 14 Control Policy (BIMR14CP)
#define BUNIT_BIMR15CP      0x6F        //BUnit Isolated Memory Region 15 Control Policy (BIMR15CP)
#define BUNIT_BIMR16CP      0x70        //BUnit Isolated Memory Region 16 Control Policy (BIMR16CP)
#define BUNIT_BIMR17CP      0x71        //BUnit Isolated Memory Region 17 Control Policy (BIMR17CP)
#define BUNIT_BIMR18CP      0x72        //BUnit Isolated Memory Region 18 Control Policy (BIMR18CP)
#define BUNIT_BIMR19CP      0x73        //BUnit Isolated Memory Region 19 Control Policy (BIMR19CP)
#define BUNIT_BIMR20CP      0x74        //BUnit Isolated Memory Region 20 Control Policy (BIMR20CP)
#define BUNIT_BIMR21CP      0x75        //BUnit Isolated Memory Region 21 Control Policy (BIMR21CP)
#define BUNIT_BIMR22CP      0x76        //BUnit Isolated Memory Region 22 Control Policy (BIMR22CP)
#define BUNIT_BIMR23CP      0x77        //BUnit Isolated Memory Region 23 Control Policy (BIMR23CP)
#define BUNIT_BIMR24CP      0x78        //BUnit Isolated Memory Region 24 Control Policy (BIMR24CP)
#define BUNIT_BIMR25CP      0x79        //BUnit Isolated Memory Region 25 Control Policy (BIMR25CP)
#define BUNIT_BIMR26CP      0x7A        //BUnit Isolated Memory Region 26 Control Policy (BIMR26CP)
#define BUNIT_BIMR27CP      0x7B        //BUnit Isolated Memory Region 27 Control Policy (BIMR27CP)
#define BUNIT_BIMR28CP      0x7C        //BUnit Isolated Memory Region 28 Control Policy (BIMR28CP)
#define BUNIT_BIMR29CP      0x7D        //BUnit Isolated Memory Region 29 Control Policy (BIMR29CP)
#define BUNIT_BIMR30CP      0x7E        //BUnit Isolated Memory Region 30 Control Policy (BIMR30CP)
#define BUNIT_BIMR31CP      0x7F        //BUnit Isolated Memory Region 31 Control Policy (BIMR31CP)

#define BUNIT_BIMR0L        0x80        //BUnit Isolated Memory Region 0 Low (BIMR0L)
#define BUNIT_BIMR0H        0x81        //BUnit Isolated Memory Region 0 High (BIMR0H)
#define BUNIT_BIMR0RAC      0x82        //BUnit Isolated Memory Region 0 Read Access Control (BIMR0RAC)
#define BUNIT_BIMR0WAC      0x83        //BUnit Isolated Memory Region 0 Write Access Control (BIMR0WAC)
#define BUNIT_BIMR1L        0x84        //BUnit Isolated Memory Region 1 Low (BIMR1L)
#define BUNIT_BIMR1H        0x85        //BUnit Isolated Memory Region 1 High (BIMR1H)
#define BUNIT_BIMR1RAC      0x86        //BUnit Isolated Memory Region 1 Read Access Control (BIMR1RAC)
#define BUNIT_BIMR1WAC      0x87        //BUnit Isolated Memory Region 1 Write Access Control (BIMR1WAC)
#define BUNIT_BIMR2L        0x88        //BUnit Isolated Memory Region 2 Low (BIMR2L)
#define BUNIT_BIMR2H        0x89        //BUnit Isolated Memory Region 2 High (BIMR2H)
#define BUNIT_BIMR2RAC      0x8A        //BUnit Isolated Memory Region 2 Read Access Control (BIMR2RAC)
#define BUNIT_BIMR2WAC      0x8B        //BUnit Isolated Memory Region 2 Write Access Control (BIMR2WAC)
#define BUNIT_BIMR3L        0x8C        //BUnit Isolated Memory Region 3 Low (BIMR3L)
#define BUNIT_BIMR3H        0x8D        //BUnit Isolated Memory Region 3 High (BIMR3H)
#define BUNIT_BIMR3RAC      0x8E        //BUnit Isolated Memory Region 3 Read Access Control (BIMR3RAC)
#define BUNIT_BIMR3WAC      0x8F        //BUnit Isolated Memory Region 3 Write Access Control (BIMR3WAC)

#define BUNIT_PTIBASE       0x100       //BUnit PTI Trace Agent Base Address (PTIBASE)
#define BUNIT_PTIRSIZE      0x101       //BUnit PTI Trace Agent Region Size (PTIRSIZE)
#define BUNIT_PTIWWMODCFG   0x102       //BUnit WINDOW WATCHER PTI MODE CONFIG (PTIWWMODCFG)
#define BUNIT_PTIUCOUNTER   0x103       //BUnit PTI Microsecond Counter (PTIUCOUNTER)
#define BUNIT_PTITSELOP     0x104       //BUnit PTI Trigger Select Observation Point (PTITSELOP)
#define BUNIT_PTITSELGRP    0x105       //BUnit PTI Trigger Select Group (PTITSELGRP)
#define BUNIT_PTI0CTL       0x110       //BUnit PTI Snoop Agent 0 Control (PTI0CTL)
#define BUNIT_PTI0SAIMATCH  0x111       //BUnit PTI Snoop Agent 0 SAI Index Match (PTI0SAIMATCH))
#define BUNIT_PTI0IDIREQ    0x112       //BUnit PTI Snoop Agent 0 IDI Request Filter (PTI0IDIREQ)
#define BUNIT_PTI0ADDRHI1   0x113       //BUnit PTI Snoop Agent 0 Address Hi Bound1 (PTI0ADDRHI1)
#define BUNIT_PTI0ADDRHI0   0x114       //BUnit PTI Snoop Agent 0 Address Hi Bound0 (PTI0ADDRHI0))
#define BUNIT_PTI0ADDRLO1   0x115       //BUnit PTI Snoop Agent 0 Address Lo Bound1 (PTI0ADDRLO1)
#define BUNIT_PTI0ADDRLO0   0x116       //BUnit PTI Snoop Agent 0 Address Lo Bound0 (PTI0ADDRLO0)
#define BUNIT_PTI0DATA      0x117       //BUnit PTI Snoop Agent 0 Data Filter (PTI0DATA)
#define BUNIT_PTI0DMASK     0x118       //BUnit PTI Snoop Agent 0 Data Mask (PTI0DMASK)
//
// VLV SSA-CUnit (Message Bus Controller)  Register Offset
//
#define CUNIT_REG_DEVICEID          0x00
#define CUNIT_CFG_REG_PCISTATUS     0x04
#define CUNIT_CFG_REG_CLASSCODE     0x08
#define CUNIT_CFG_REG_HDR_TYPE      0x0C
#define CUNIT_CFG_REG_STRAP_SSID    0x2C
#define CUNIT_SB_PACKET_REG         0xD0
#define CUNIT_SB_DATA_REG           0xD4
#define CUNIT_SB_PCKET_ADDR_EXT     0xD8
#define CUNIT_SB_PACKET_REG_RW      0xDC
#define CUNIT_SB_PCKET_ADDR_EXT_FUNNYIO 0xE0
#define CUNIT_SB_PCKET_REG_RW_FUNNYIO   0xE4
#define CUNIT_SCRATCHPAD_REG        0xF0
#define CUNIT_MANUFACTURING_ID      0xF8
#define CUNIT_LOCAL_CONTROL_MODE    0x100
#define CUNIT_ACCESS_CTRL_VIOL      0x104

//
// VLV SSA-TUnit (CPU Bus Interface Controller)  Register Offset
//
#define TUNIT_INTR_REDIR_CTL        0x000
#define TUNIT_X2B_ARB_CTL0          0x001
#define TUNIT_APIC_CTL              0x002
#define TUNIT_CTL (TUNIT_CTL)       0x003
#define TUNIT_MISC_CTL              0x004
#define TUNIT_CLKGATE_CTL           0x005
#define TUNIT_X2BARB_CTL1           0x006
#define TUNIT_MONADDR_LPID0         0x010
#define TUNIT_MONADDR_LPID1         0x011
#define TUNIT_MONADDR_LPID2         0x012
#define TUNIT_MONADDR_LPID3         0x013
#define TUNIT_IDI0_SNPCNTR          0x024
#define TUNIT_IDI1_SNPCNTR          0x025
#define TUNIT_SEMAPHORE             0x030
#define TUNIT_SCRPAD0               0x031
#define TUNIT_SCRPAD1               0x032
#define TUNIT_SCRPAD2               0x033
#define TUNIT_SCRPAD3               0x034
#define TUNIT_SECURITY_CTL_DBG      0x040
#define TUNIT_SECURITY_RD_CTL_DBG   0x041
#define TUNIT_SECURITY_WR_CTL_DBG   0x042
#define TUNIT_SECURITY_CTL_UCD      0x044
#define TUNIT_SECURITY_RD_CTL_UCD   0x045
#define TUNIT_SECURITY_WR_CTL_UCD   0x046
#define TUNIT_SECURITY_CTL_PWR      0x048
#define TUNIT_SECURITY_RD_CTL_PWR   0x049
#define TUNIT_SECURITY_WR_CTL_PWR   0x04A
#define TUNIT_SECURITY_CTL_SECCTL   0x04C
#define TUNIT_SECURITY_RD_CTL_SECCTL    0x04D
#define TUNIT_SECURITY_WR_CTL_SECCTL    0x04E
#define TUNIT_SECURITY_STAT0    0x050
#define TUNIT_SECURITY_STAT1    0x051

//
// VLV SSA-DUnit (System Memory Controller)  Register Offset
//
#define DUNIT_DRP           0x00
#define DUNIT_DTR0          0x01
#define DUNIT_DTR1          0x02
#define DUNIT_DTR2          0x03
#define DUNIT_DTR3          0x04
#define DUNIT_DPMC0         0x06
#define DUNIT_DPMC1         0x07
#define DUNIT_DRFC          0x08
#define DUNIT_DSCH          0x09
#define DUNIT_DCAL          0x0A
#define DUNIT_DRMC          0x0B
#define DUNIT_PMSTS         0x0C
#define DUNIT_DCO           0x0F
#define DUNIT_DTRC          0x10
#define DUNIT_DCBR          0x12
#define DUNIT_DSTAT         0x20
#define DUNIT_PGTBL         0x21
#define DUNIT_MISRCCCLR     0x31
#define DUNIT_MISRDDCLR     0x32
#define DUNIT_MISRCCSIG     0x34
#define DUNIT_MISRDDSIG     0x35
#define DUNIT_SSKPD0        0x4A
#define DUNIT_SSKPD1        0x4B
#define DUNIT_BONUS0        0x50
#define DUNIT_BONUS1        0x51
#define DUNIT_DECCCTRL      0x60
#define DUNIT_DECCSTAT      0x61
#define DUNIT_DECCSBECNT    0x62
#define DUNIT_PMSEL0        0xE0
#define DUNIT_PMSEL1        0xE1
#define DUNIT_PMSEL2        0xE2
#define DUNIT_PMSEL3        0xE3
#define DUNIT_PMAUXMAX      0xE8
#define DUNIT_PMAUXMIN      0xE9
#define DUNIT_PMAUX         0xEA

//
// PUNIT (Power Management Control)
//
#define PUNIT_CONTROL               0x000
#define PUNIT_THERMAL_SOC_TRIGGER   0x001
#define PUNIT_SOC_POWER_BUDGET      0x002
#define PUNIT_SOC_ENERGY_CREDIT     0x003
#define PUNIT_TURBO_SOC_OVERRIDE    0x004
#define PUNIT_BIOS_RESET_CPL        0x005
#define PUNIT_BIOS_CONFIG           0x006
#define PUNIT_PWRGT_CNT_CTRL        0x060
#define PUNIT_PWRGT_STATUS          0x061
#define PUNIT_PWRGT_INTREN          0x062
#define PUNIT_TAP_PGOFF_OVR         0x063
#define PUNIT_TAP_PGON_OVR          0x064
#define PUNIT_OPTION_REG1           0x065
#define PUNIT_OPTION_REG2           0x066
#define PUNIT_TAP_FWOFF_OVR         0x067
#define PUNIT_TAP_FWON_OVR          0x068
#define PUNIT_OPTION_REG3           0x069
#define PUNIT_OPTION_REG4           0x06A
#define PUNIT_PWRGT_EN_OUT          0x06B
#define PUNIT_PWRGT_RF_EN_OUT       0x06C
#define PUNIT_OPTION_REG5           0x06D
#define PUNIT_PUNIT_STRAPS_REG      0x06E
#define PUNIT_OPTION_REG6           0x06F
#define PUNIT_OPTION_REG7           0x070
#define PUNIT_PCR_EXTERNAL          0x071
#define PUNIT_OPTION_REG8           0x072
#define PUNIT_OPTION_REG9           0x073
#define PUNIT_OPTION_REG10          0x074
#define PUNIT_CPU_RST               0x07B
#define PUNIT_SEND_MSG_TO_MTSC      0x0F0
#define PUNIT_CORE_AONT_DATA_L      0x0F1
#define PUNIT_CORE_AONT_DATA_H      0x0F2
#define PUNIT_AONT_CLOCK_CONFIG     0x0F3

//
// Device 2 Register Equates
//
#define IGD_BUS             0x00
#define IGD_DEV             0x02
#define IGD_FUN_0           0x00
#define IGD_FUN_1           0x01
#define IGD_DEV_FUN         (IGD_DEV << 3)
#define IGD_BUS_DEV_FUN     (MC_BUS << 8) + IGD_DEV_FUN
#define IGD_VID             0x8086
#define IGD_DID             0xA001
#define IGD_MGGC_OFFSET     0x0050      //GMCH Graphics Control Register 0x50
#define IGD_BSM_OFFSET      0x005C      //Base of Stolen Memory
#define IGD_SWSCI_OFFSET    0x00E0      //Software SCI 0xE0 2
#define IGD_ASLE_OFFSET     0x00E4      //System Display Event Register 0xE4 4
#define IGD_ASLS_OFFSET     0x00FC      // ASL Storage
#endif
