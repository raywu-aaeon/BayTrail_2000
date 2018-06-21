/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
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
#define VLV_DFXUNIT     0x05        /*Test Controller SSA-DFX Unit*/
#define VLV_GUNIT       0x06        /*G Unit*/
//#define VLV_XXX       0x07        /*Reserved*/
#define VLV_CUNIT       0x08        /*C Unit*/
//#define VLV_XXX       0x09        /*Reserved*/
#define VLV_sVID        0x0A        /*VID Controller*/
//#define VLV_XXX       0x0B        /*Reserved*/
#define VLV_DDRIO       0x0C        /*DDR IO Unit*/
#define VLV_REUT        0x0D        /*Memory REUT*/
#define VLV_GENX        0x0E        /*Graphics Adapter-Genx*/
#define VLV_DRNG        0x0F        /*Random Number Generator*/
#define VLV_DISPCONT    0x10        /*Display Controller*/
#define VLV_FUSEEPNC    0x11        /*Fuse EndPoint Nort*/
#define VLV_DISPIO      0x12        /*DISPPHY*/
#define VLV_GPIONC      0x13        /*GPIO (North) Controller*/
#define VLV_CCK         0x14        /*Clock Controller*/
#define VLV_DFXSOC      0x15        /*Test Controller DFX-SOC*/
#define VLV_DFXNC       0x16        /*Test Controller DFX-NC*/
#define VLV_DFXLAKEMORE     0x17        /*Test Controller DFX-Lakemore*/
#define VLV_DFXVISA     0x18        /*Test Controller DFX-VISA*/

#define VLV_PSF_PORTA2  0xA2
#define VLV_PSF_PORT45  0x45
#define VLV_PSF_PORT46  0x46
#define VLV_PSF_PORT47  0x47

#define VLV_SMB_PORT55  0x55

#define VLV_IUNIT       0x1C        /*Image Signal Processor-ISP*/
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
//Common for A/B/C/D/T/SVID/CCK/I units
#define VLV_MBR_READ_CMD    0x10000000
#define VLV_MBR_WRITE_CMD   0x11000000

//Common for Gunit/DISPIO/DFXLAKSEMORE/DISPCONT units
#define VLV_MBR_GDISPIOREAD_CMD     0x00000000
#define VLV_MBR_GDISPIOWRITE_CMD    0x01000000

//Common for Smbus units
#define VLV_SMB_REGREAD_CMD 0x04000000 
#define VLV_SMB_REGWRITE_CMD    0x05000000 

//Common for Punit/DFX/GPIONC/DFXSOC/DFXNC/DFXVISA units
#define VLV_MBR_PDFXGPIODDRIOREAD_CMD   0x06000000
#define VLV_MBR_PDFXGPIODDRIOWRITE_CMD  0x07000000

//Msg Bus Registers
#define MC_MCR          0x000000D0      //Cunit Message Control Register
#define MC_MDR          0x000000D4      //Cunit Message Data Register
#define MC_MCRX         0x000000D8      //Cunit Message Control Register Extension
#define MC_MCRXX        0x000000DC   //cunit Message Controller Register Extension 2

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
// Updated to Cpsec-12ww49.4
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
#define AUNIT_AISOCHCTL2    0x22
#define AUNIT_APEERBASE     0x30
#define AUNIT_APEERLIMIT    0x31
#define AUNIT_AVIB          0x32
#define AUNIT_AVQR          0x33
#define AUNIT_ATBR          0x40
#define AUNIT_AB0BR         0x48
#define AUNIT_AB1BR         0x49
#define AUNIT_AB2BR         0x4A
//#define AUNIT_RESERVED    0x52
#define AUNIT_ACFCACV       0x60
#define AUNIT_ASBACV0       0x61
#define AUNIT_ASBACV1       0x62
#define AUNIT_ACF8SAI       0x63
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
#define AUNIT_AIODUSMSK0    0x99
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
#define AUNIT_AARBCTL0      0xC0
#define AUNIT_AARBCTL1      0xC1
#define AUNIT_AARBCTL2      0xC2
#define AUNIT_AARBCTL3      0xC3
#define AUNIT_AARBCTL4      0xC4
#define AUNIT_ADNARBCTL     0xD0
#define AUNIT_AMISR0        0xF0
#define AUNIT_AMISR1        0xF1


//
// VLV SSA-BUnit (System Memory Arbiter)  Register Offset
// Updated to Cpsec-12ww49.4
#define BUNIT_BSECCP        0x00
#define BUNIT_BSECRAC       0x01
#define BUNIT_BSECWAC       0x02
#define BUNIT_BARBCTRL0     0x03
#define BUNIT_BARBCTRL1     0x04
#define BUNIT_BARBCTRL2     0x05
#define BUNIT_BARBCTRL3     0x06
#define BUNIT_BWFLUSH       0x07
#define BUNIT_BBANKMASK     0x08
#define BUNIT_BROWMASK      0x09
#define BUNIT_BRANKMASK     0x0A
#define BUNIT_BALIMIT0      0x0B
#define BUNIT_BALIMIT1      0x0C
#define BUNIT_BALIMIT2      0x0D
#define BUNIT_BALIMIT3      0x0E
#define BUNIT_BARES0        0x0F
#define BUNIT_BARES1        0x10
#define BUNIT_BISOC         0x11
#define BUNIT_BCOSCAT       0x12
#define BUNIT_BDPT          0x13
#define BUNIT_BFLWT         0x14
#define BUNIT_BBWC          0x15
#define BUNIT_BISOCWT       0x16
#define BUNIT_BSCHCTRL0     0x18
//#define BUNIT_RESERVED    0x19
#define BUNIT_BIMRDATA      0x1a
#define BUNIT_BPMRVCTL      0x1b
#define BUNIT_B_SECURITY_STAT0  0x1c
#define BUNIT_B_SECURITY_STAT1  0x1d
#define BUNIT_B_SECURITY_STAT2  0x1e
#define BUNIT_BMRCP         0x20
#define BUNIT_BMRRAC        0x21
#define BUNIT_BMRWAC        0x22
#define BUNIT_BNOCACHE      0x23
#define BUNIT_BNOCACHECTL   0x24
#define BUNIT_BMBOUND       0x25
#define BUNIT_BMBOUND_HI    0x26
#define BUNIT_BECREG        0x27
#define BUNIT_BMISC         0x28
#define     B_BMISC_RESDRAM     0x01    //Bit 0 - When this bit is set, reads targeting E-segment are routed to DRAM.
#define     B_BMISC_RFSDRAM     0x02    //Bit 1 - When this bit is set, reads targeting F-segment are routed to DRAM.
#define BUNIT_BSMRCP        0x2B
#define     BSMRCP_SMM_CTRL_REG_LOCK    0x00    //Dont allow any access to the register until the system is reset
#define     BSMRCP_SMM_CTRL_REG_IA_SMM  0x04    //IA_SMM access 
#define     BSMRCP_SMM_CTRL_REG_PUNIT   0x100   //Punit trusted
#define BUNIT_BSMRRAC       0x2C
#define     BSMRRAC_SMM_WRITE_OPEN_FOR_ALL_CORE 0xFF    //Allow access only to all CPU HOST
#define     BSMRRAC_SMM_WRITE_CLOSED_FOR_IA_SMM 0x04    //Allow access only to CPU HOST IA SMM
#define BUNIT_BSMRWAC       0x2D
#define     BSMRWAC_SMM_WRITE_OPEN_FOR_ALL_CORE 0xFF    //Allow access only to all CPU HOST
#define     BSMRWAC_SMM_WRITE_CLOSED_FOR_IA_SMM 0x04    //Allow access only to CPU HOST IA SMM
#define BUNIT_BSMMRRL           0x2E
#define BUNIT_BSMMRRH           0x2F
#define BUNIT_BC0AHASHCFG       0x30
#define BUNIT_BDBCP             0x38
#define BUNIT_BDRRAC            0x39
#define BUNIT_BDRWAC            0x3A
#define BUNIT_BDEBUG0           0x3B
#define BUNIT_BDEBUG1           0x3C
#define BUNIT_BCTRL             0x3D
#define BUNIT_BTHCTRL           0x3E
#define BUNIT_BTHMASK           0x3F
#define BUNIT_BIACP             0x40
#define BUNIT_BIARAC            0x41
#define BUNIT_BIAWAC            0x42
#define BUNIT_BEXMCP            0x43
#define BUNIT_BEXMRAC           0x44
#define BUNIT_BEXMWAC           0x45
#define BUNIT_EXML              0x46
#define BUNIT_EXMH              0x47
#define BUNIT_LP0Mode           0x48
#define BUNIT_LP1Mode           0x49
#define BUNIT_LP2Mode           0x4A
#define BUNIT_LP3Mode           0x4B
#define BUNIT_MCi_CTL_LOW       0x54
#define BUNIT_MCi_CTL_HIGH      0x55
#define BUNIT_MCi_STATUS_LOW    0x56
#define BUNIT_MCi_STATUS_HIGH   0x57
#define BUNIT_MCi_ADDR_LOW      0x58
#define BUNIT_MCi_ADDR_HIGH     0x59
#define BUNIT_BCERRTHRESH_LOW   0x5A
#define BUNIT_BCERRTHRESH_HIGH  0x5B
#define BUNIT_BMCMODE_LOW       0x5C
#define BUNIT_BMCMODE_HIGH      0x5D
#define BUNIT_BIMR0CP           0x60
#define BUNIT_BIMR1CP           0x61
#define BUNIT_BIMR2CP           0x62
#define BUNIT_BIMR3CP           0x63
#define BUNIT_BIMR4CP           0x64
#define BUNIT_BIMR5CP           0x65
#define BUNIT_BIMR6CP           0x66
#define BUNIT_BIMR7CP           0x67
#define BUNIT_BIMR0L            0x80
#define BUNIT_BIMR0H            0x81
#define BUNIT_BIMR0RAC          0x82
#define BUNIT_BIMR0WAC          0x83
#define BUNIT_BIMR1L            0x84
#define BUNIT_BIMR1H            0x85
#define BUNIT_BIMR1RAC          0x86
#define BUNIT_BIMR1WAC          0x87
#define BUNIT_BIMR2L            0x88
#define BUNIT_BIMR2H            0x89
#define BUNIT_BIMR2RAC          0x8a
#define BUNIT_BIMR2WAC          0x8b
#define BUNIT_BIMR3L            0x8c
#define BUNIT_BIMR3H            0x8d
#define BUNIT_BIMR3RAC          0x8e
#define BUNIT_BIMR3WAC          0x8f
#define BUNIT_BIMR4L            0x90
#define BUNIT_BIMR4H            0x91
#define BUNIT_BIMR4RAC          0x92
#define BUNIT_BIMR4WAC          0x93
#define BUNIT_BIMR5L            0x94
#define BUNIT_BIMR5H            0x95
#define BUNIT_BIMR5RAC          0x96
#define BUNIT_BIMR5WAC          0x97
#define BUNIT_BIMR6L            0x98
#define BUNIT_BIMR6H            0x99
#define BUNIT_BIMR6RAC          0x9a
#define BUNIT_BIMR6WAC          0x9b
#define BUNIT_BIMR7L            0x9c
#define BUNIT_BIMR7H            0x9d
#define BUNIT_BIMR7RAC          0x9e
#define BUNIT_BIMR7WAC          0x9f
#define BUNIT_PTIBASE           0x0100
#define BUNIT_PTIRSIZE          0x0101
#define BUNIT_PTIWWMODCFG       0x0102
#define BUNIT_PTIUCOUNTER       0x0103
#define BUNIT_PTITSELOP         0x0104
#define BUNIT_PTITSELGRP        0x0105
#define BUNIT_PTI0CTL           0x0110
#define BUNIT_PTI0SAIMATCH      0x0111
#define BUNIT_PTI0IDIREQ        0x0112
#define BUNIT_PTI0ADDRHI1       0x0113
#define BUNIT_PTI0ADDRHI0       0x0114
#define BUNIT_PTI0ADDRLO1       0x0115
#define BUNIT_PTI0ADDRLO0       0x0116
#define BUNIT_PTI0DATA          0x0117
#define BUNIT_PTI0DMASK         0x0118
#define BUNIT_PTI1CTL           0x0120
#define BUNIT_PTI1SAIMATCH      0x0121
#define BUNIT_PTI1IDIREQ        0x0122
#define BUNIT_PTI1ADDRHI1       0x0123
#define BUNIT_PTI1ADDRHI0       0x0124
#define BUNIT_PTI1ADDRLO1       0x0125
#define BUNIT_PTI1ADDRLO0       0x0126
#define BUNIT_PTI1DATA          0x0127
#define BUNIT_PTI1DMASK         0x0128
#define BUNIT_PTI2CTL           0x0130
#define BUNIT_PTI2SAIMATCH      0x0131
#define BUNIT_PTI2IDIREQ        0x0132
#define BUNIT_PTI2ADDRHI1       0x0133
#define BUNIT_PTI2ADDRHI0       0x0134
#define BUNIT_PTI2ADDRLO1       0x0135
#define BUNIT_PTI2ADDRLO0       0x0136
#define BUNIT_PTI2DATA          0x0137
#define BUNIT_PTI2DMASK         0x0138
#define BUNIT_PTI3CTL           0x0140
#define BUNIT_PTI3SAIMATCH      0x0141
#define BUNIT_PTI3IDIREQ        0x0142
#define BUNIT_PTI3ADDRHI1       0x0143
#define BUNIT_PTI3ADDRHI0       0x0144
#define BUNIT_PTI3ADDRLO1       0x0145
#define BUNIT_PTI3ADDRLO0       0x0146
#define BUNIT_PTI3DATA          0x0147
#define BUNIT_PTI3DMASK         0x0148
#define BUNIT_MISRCTL           0x0150
#define BUNIT_MISRS2CREQSIG     0x0151
#define BUNIT_MISRC2SREQSIG     0x0152
#define BUNIT_MISRS2CDATASIG    0x0153
#define BUNIT_MISRC2SDATASIG    0x0154
#define BUNIT_MISRRPLSIG        0x0155
#define BUNIT_BDBGCTL           0x0160
#define BUNIT_BDBGADD           0x0161
#define BUNIT_BDBGDAT           0x0162
//
// VLV SSA-CUnit (Message Bus Controller)  Register Offset
// Updated to Cpsec-12ww49.4
#define CUNIT_REG_DEVICEID                0x00
#define CUNIT_CFG_REG_PCISTATUS           0x01
#define CUNIT_CFG_REG_CLASSCODE           0x02
#define CUNIT_CFG_REG_HDR_TYPE            0x03
#define CUNIT_CFG_REG_STRAP_SSID          0x0B
#define CUNIT_SB_MSG_REG                  0x34
#define CUNIT_SB_DATA_REG                 0x35
#define CUNIT_SB_PCKET_ADDR_EXT           0x36
#define CUNIT_SB_PACKET_REG_RW            0x37
#define CUNIT_SB_PCKET_ADDR_EXT_FUNNYIO   0x38
#define CUNIT_SB_PCKET_REG_RW_FUNNYIO     0x39
#define CUNIT_SB_PACKET_FUNNYIO_ADDR_EXT1 0x3A
#define CUNIT_SB_PACKET_FUNNYIO_REG1      0x3B
#define CUNIT_SCRATCHPAD_REG              0x3C
#define CUNIT_MANUFACTURING_ID            0x3E
#define CUNIT_LOCAL_CONTROL_MODE          0x40
#define CUNIT_ACCESS_CTRL_VIOL            0x41
#define CUNIT_PDM_REGISTER                0x42
#define CUNIT_SSA_REGIONAL_TRUNKGATE_CTL  0x43
#define CUNIT_MCRS_SAI                    0x45
#define CUNIT_MDR_SAI                     0x46
#define CUNIT_SB_PACKET_FUNNYIO_ADDR_EXT2 0x58
#define CUNIT_SB_PACKET_FUNNYIO_REG2      0x59
#define CUNIT_SB_PACKET_FUNNYIO_ADDR_EXT3 0x5A
#define CUNIT_SB_PACKET_FUNNYIO_REG3      0x5B

//
// VLV SSA-CUnit (Message Bus Controller)  Register Offset
// Accessing by PCI Config B0D0F0
// Updated to Cpsec-12ww49.4
#define CUNIT_PCICFG_REG_DEVICEID       0x00
#define CUNIT_PCICFG_CFG_REG_PCISTATUS  0x04
#define CUNIT_PCICFG_CFG_REG_CLASSCODE  0x08
#define CUNIT_PCICFG_CFG_REG_HDR_TYPE   0x0C
#define CUNIT_PCICFG_CFG_REG_STRAP_SSID 0x2C
#define CUNIT_PCICFG_MSG_CTRL_REG       0xD0
#define CUNIT_PCICFG_MSG_DATA_REG       0xD4
#define CUNIT_PCICFG_MSG_CTRL_REG_EXT   0xD8
#define CUNIT_PCICFG_MSG_CTRL_PACKET_REG            0xDC
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_ADDR_EXT0    0xE0
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_REG0         0xE4
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_ADDR_EXT1    0xE8
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_REG1         0xEC
#define CUNIT_PCICFG_SCRATCHPAD_REG     0xF0
#define CUNIT_PCICFG_MANUFACTURING_ID   0xF8
#define CUNIT_PCICFG_LOCAL_CONTROL_MODE 0x100
#define CUNIT_PCICFG_ACCESS_CTRL_VIOL   0x104
#define CUNIT_PCICFG_PDM_REGISTER       0x108
#define CUNIT_PCICFG_SSA_REGIONAL_TRUNKGATE_CTL     0x10C
#define CUNIT_PCICFG_MCRS_SAI           0x114
#define CUNIT_PCICFG_MDR_SAI            0x118
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_ADDR_EXT2    0x160
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_REG2         0x164
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_ADDR_EXT3    0x168
#define CUNIT_PCICFG_SB_PACKET_FUNNYIO_REG3         0x16C


//
// VLV SSA-TUnit (CPU Bus Interface Controller)  Register Offset
// Updated to Cpsec-12ww49.4
#define TUNIT_INTR_REDIR_CTL    0x00
#define TUNIT_X2B_ARB_CTL0      0x01
#define TUNIT_APIC_CTL          0x02
#define TUNIT_CTL               0x03
#define TUNIT_MISC_CTL          0x04
#define TUNIT_CLKGATE_CTL       0x05
#define TUNIT_X2BARB_CTL1       0x06
#define TUNIT_PSMI_CTL          0x07
#define TUNIT_MONADDR_LPID0     0x10
#define TUNIT_MONADDR_LPID1     0x11
#define TUNIT_MONADDR_LPID2     0x12
#define TUNIT_MONADDR_LPID3     0x13
#define TUNIT_IDI0_SNPCNTR      0x24
#define TUNIT_IDI1_SNPCNTR      0x25
#define TUNIT_SEMAPHORE         0x30
#define TUNIT_SCRPAD0           0x31
#define TUNIT_SCRPAD1           0x32
#define TUNIT_SCRPAD2           0x33
#define TUNIT_SCRPAD3           0x34
#define TUNIT_SECURITY_CTL_DBG  0x40
#define TUNIT_SECURITY_RD_CTL_DBG   0x41
#define TUNIT_SECURITY_WR_CTL_DBG   0x42
#define TUNIT_SECURITY_CTL_UCD      0x44
#define TUNIT_SECURITY_RD_CTL_UCD   0x45
#define TUNIT_SECURITY_WR_CTL_UCD   0x46
#define TUNIT_SECURITY_CTL_PWR      0x48
#define TUNIT_SECURITY_RD_CTL_PWR   0x49
#define TUNIT_SECURITY_WR_CTL_PWR   0x4A
#define TUNIT_SECURITY_CTL_SECCTL   0x4C
#define TUNIT_SECURITY_RD_CTL_SECCTL    0x4D
#define TUNIT_SECURITY_WR_CTL_SECCTL    0x4E
#define TUNIT_SECURITY_STAT0    0x50
#define TUNIT_SECURITY_STAT1    0x51
#define TUNIT_APICINFO_LPID0    0x60
#define TUNIT_APICINFO_LPID1    0x61
#define TUNIT_APICINFO_LPID2    0x62
#define TUNIT_APICINFO_LPID3    0x63
#define TUNIT_MCERR_STATUS      0x70
#define TUNIT_IERR_STATUS       0x71
#define TUNIT_MCERR_STATUS_UPDATE    0x72
#define TUNIT_IERR_STATUS_UPDATE     0x73
#define TUNIT_ERR_LOGSTATUS     0x74

//
// VLV SSA-DUnit (System Memory Controller)  Register Offset
// Port ID: 1h
#define DUNIT_DRP       0x00
#define DUNIT_DTR0      0x01
#define DUNIT_DTR1      0x02
#define DUNIT_DTR2      0x03
#define DUNIT_DTR3      0x04
#define DUNIT_DTR4      0x05
#define DUNIT_DPMC0     0x06
#define DUNIT_DPMC1     0x07
#define DUNIT_DRFC      0x08
#define DUNIT_DSCH      0x09
#define DUNIT_DCAL      0x0A
#define DUNIT_DRMC      0x0B
#define DUNIT_PMSTS     0x0C
#define DUNIT_DCO       0x0F
#define DUNIT_DTRC      0x10
#define DUNIT_DCBR      0x12
#define DUNIT_DSTAT     0x20
#define DUNIT_PGTBL     0x21
#define DUNIT_MISRCCCLR     0x31
#define DUNIT_MISRDDCLR     0x32
#define DUNIT_MISRCCSIG     0x34
#define DUNIT_MISRDDSIG     0x35
#define DUNIT_MISRECCSIG    0x37
#define DUNIT_SSKPD0        0x4A
#define DUNIT_SSKPD1        0x4B
#define DUNIT_BONUS0        0x50
#define DUNIT_BONUS1        0x51
#define DUNIT_DECCCTRL      0x60
#define DUNIT_DECCSTAT      0x61
#define DUNIT_DECCSBECNT    0x62
#define DUNIT_DFUSESTAT     0x70
#define DUNIT_DSCRMSEED     0x80
#define DUNIT_DSCRMLO       0x81
#define DUNIT_DSCRMHI       0x82
#define DUNIT_PMSEL0        0xE0
#define DUNIT_PMSEL1        0xE1
#define DUNIT_PMSEL2        0xE2
#define DUNIT_PMSEL3        0xE3
#define DUNIT_PMAUXMAX      0xE8
#define DUNIT_PMAUXMIN      0xE9
#define DUNIT_PMAUX     0xEA

//
// PUNIT(Power Management Control) Regsiters Offset
// Port ID:4
// updated based on Cspec ww30.1
#define PUNIT_CONTROL               0x00
#define PUNIT_THERMAL_SOC_TRIGGER   0x01
#define PUNIT_SOC_POWER_BUDGET      0x02
#define PUNIT_SOC_ENERGY_CREDIT     0x03
#define PUNIT_TURBO_SOC_OVERRIDE    0x04
#define PUNIT_BIOS_RESET_CPL        0x05
#define PUNIT_BIOS_CONFIG           0x06
#define     B_GFX_TURBO_DIS (BIT7)      //Bit7 - Turbo Disable
#define PUNIT_PKG_TURBO_POWER_LIMIT_L32 0x07
#define PUNIT_PKG_TURBO_POWER_LIMIT_H32 0x08
#define PUNIT_PP1_TURBO_POWER_LIMIT 0x09
#define PUNIT_SPARE_TURBO_REG0      0x0A
#define PUNIT_SPARE_TURBO_REG1      0x0B
#define PUNIT_FW_VISA_REG0          0x0C
#define PUNIT_FW_VISA_REG1          0x0D
#define PUNIT_WARM_RST_INDICATOR    0x0E
#define PUNIT_SAI_VIOLATION         0x0F
#define PUNIT_DFX_DEBUG_STRAPS      0x10
#define PUNIT_SOC_DEBUG_PGE         0x11
#define PUNIT_SOC_DEBUG_FWE         0x12
#define PUNIT_CPU_DEBUG_PGE         0x13
#define PUNIT_CPU_DEBUG_FWE         0x14
#define PUNIT_CSTATEINFO            0x15
#define PUNIT_FWCNTRLINFO           0x16
#define PUNIT_SAIVIOLATION_STS      0x17
#define PUNIT_AG1CP             0x18
#define PUNIT_AG2CP             0x19
#define PUNIT_AG1WAC            0x1A
#define PUNIT_AG2WAC            0x1B
#define PUNIT_CLKGATEOVER       0x1C
#define PUNIT_BIOSCFG_IPDEVTYPE 0x1D
#define PUNIT_S0i2_PREWAKE      0x1E
#define PUNIT_S0i3_PREWAKE      0x1F
#define PUNIT_PKG_POWER_INFO_L32    0x20
#define PUNIT_PKG_POWER_INFO_H32    0x21
#define PUNIT_PKG_PERF_STATUS   0x22
#define PUNIT_DRAM_POWER_INFO_L32   0x23
#define PUNIT_DRAM_POWER_INFO_H32   0x24
#define PUNIT_DRAM_PERF_STATUS  0x25
#define PUNIT_DRAM_ENERGY_STATUS    0x26
#define PUNIT_DRAM_POWER_LIMIT  0x27
#define PUNIT_PP2_POWER_LIMIT   0x28
#define PUNIT_PP2_ENERGY_STATUS 0x29
#define PUNIT_L2SIZE_CONTROL0   0x2A
#define PUNIT_L2SIZE_CONTROL1   0x2B
#define PUNIT_L2SIZE_CONTROL2   0x2C
#define PUNIT_L2SIZE_CONTROL3   0x2D
#define PUNIT_SPARE_PM_CONTROL0 0x2E
#define PUNIT_SPARE_PM_CONTROL1 0x2F
#define PUNIT_VEDSSPM0  0x32
#define PUNIT_VEDSSPM1  0x33
#define PUNIT_DSPSSPM   0x36
#define PUNIT_ISPSSPM0  0x39
#define     B_ISPSSPM0_FUSDIS   BIT26       //Bit26 - When this bit is set, ISP Disable by fuse
#define PUNIT_ISPSSPM1  0x3A
#define PUNIT_MIOSSPM   0x3B
#define PUNIT_GUNIT_SS_PM   0x3E
#define PUNIT_RTC_GTSC_INCRAMT1 0x40
#define PUNIT_RTC_GTSC_INCRAMT2 0x41
#define PUNIT_RTC_GTSC_INCRAMT3 0x42
#define PUNIT_RTC_GTSC_HIGH     0x43
#define PUNIT_RTC_GTSC_LOW      0x44
#define PUNIT_RTC_GTSC_UPDTCYC  0x45
#define PUNIT_CPUTAPMISC_M0_FORCEONE_OVR    0x48
#define PUNIT_CPUTAPMISC_M1_FORCEONE_OVR    0x49
#define PUNIT_CPUTAPMISC_M0_FORCEZERO_OVR   0x4C
#define PUNIT_CPUTAPMISC_M1_FORCEZERO_OVR   0x4D
#define PUNIT_CPUTAPPG_M0_FORCEONE_OVR  0x50
#define PUNIT_CPUTAPPG_M1_FORCEONE_OVR  0x51
#define PUNIT_CPUTAPPG_M0_FORCEZERO_OVR 0x54
#define PUNIT_CPUTAPPG_M1_FORCEZERO_OVR 0x55
#define PUNIT_CPUTAPFW_M0_FORCEONE_OVR  0x58
#define PUNIT_CPUTAPFW_M1_FORCEONE_OVR  0x59
#define PUNIT_CPUTAPFW_M0_FORCEZERO_OVR 0x5C
#define PUNIT_CPUTAPFW_M1_FORCEZERO_OVR 0x5D
#define PUNIT_PWRGT_CNT_CTRL    0x60
#define PUNIT_PWRGT_STATUS      0x61
#define PUNIT_PWRGT_INTREN      0x62
#define PUNIT_TAP_PG_FORCEONE_OVR   0x63
#define PUNIT_TAP_PG_FORCEZERO_OVR  0x64
#define PUNIT_TAP_CLKEN_FORCEONE_OVR    0x65
#define PUNIT_TAP_CLKEN_FORCEZERO_OVR   0x66
#define PUNIT_TAP_FW_FORCEONE_OVR   0x67
#define PUNIT_TAP_FW_FORCEZERO_OVR  0x68
#define PUNIT_TAP_RST_FORCEONE_OVR  0x69
#define PUNIT_TAP_RST_FORCEZERO_OVR 0x6A
#define PUNIT_PWRGT_EN_OUT      0x6B
#define PUNIT_PWRGT_RF_EN_OUT   0x6C
#define PUNIT_PUNIT_INTR_PAYLOAD    0x6D
//#define   PUNIT_RSV_0x6E  0x6E
#define PUNIT_TAPMISC_FORCEONE_OVR  0x6F
#define PUNIT_TAPMISC_FORCEZERO_OVR 0x70
#define PUNIT_PCR_EXTERNAL      0x71
#define PUNIT_TAPMISC2_FORCEONE_OVR 0x72
#define PUNIT_TAPMISC2_FORCEZERO_OVR    0x73
#define PUNIT_OPTION_REG10      0x74
#define PUNIT_PUNIT_CPU_RST     0x7B
#define PUNIT_CPU_SOFT_STRAPS   0x7C
#define PUNIT_DFX_STALL         0x7D
#define PUNIT_PTMC  0x80
#define PUNIT_TTR0  0x81
#define PUNIT_TTR1  0x82
#define PUNIT_TTS   0x83
#define PUNIT_TELB  0x84
#define PUNIT_TELT  0x85
#define PUNIT_TQPR  0x86
//#define   PUNIT_RSV_0x87  0x87
#define PUNIT_GFXT  0x88
#define PUNIT_VEDT  0x89
#define PUNIT_ISPT  0x8C
#define PUNIT_FW_SHAID0 0x90
#define PUNIT_FW_SHAID1 0x91
#define PUNIT_FW_SHAID2 0x92
#define PUNIT_FW_SHAID3 0x93
#define PUNIT_FW_SHAID4 0x94
#define PUNIT_IUNITSETID 0x98
#define PUNIT_M0_CORE_DDL_MIN_VALUE_LOW   0x99
#define PUNIT_M0_CORE_DDL_MIN_VALUE_HIGH  0x9A
#define PUNIT_M1_CORE_DDL_MIN_VALUE_LOW   0x9B
#define PUNIT_M1_CORE_DDL_MIN_VALUE_HIGH  0x9C
#define PUNIT_M0_OPER1_VALUE_LOW_ADDRESS  0x9D
#define PUNIT_M0_OPER1_VALUE_HIGH_ADDRESS 0x9E
#define PUNIT_M1_OPER1_VALUE_LOW_ADDRESS  0x9F
#define PUNIT_FW_OR0    0xA0
#define PUNIT_FW_OR1    0xA1
#define PUNIT_VID_DEBUG 0xA2
#define PUNIT_FW_OR3    0xA3
#define PUNIT_CORE0_AONTL   0xA4
#define PUNIT_CORE0_AONTH   0xA5
#define PUNIT_CORE1_AONTL   0xA6
#define PUNIT_CORE1_AONTH   0xA7
#define PUNIT_FW_OR8    0xA8
#define PUNIT_FW_OR9    0xA9
#define PUNIT_FW_ORA    0xAA
#define PUNIT_FW_ORB    0xAB
#define PUNIT_FW_ORC    0xAC
#define PUNIT_FW_ORD    0xAD
#define PUNIT_FW_ORE    0xAE
#define PUNIT_FW_ORF    0xAF
#define PUNIT_DTSC  0xB0
#define PUNIT_TRR   0xB1
#define PUNIT_PTPS  0xB2
#define PUNIT_PTTS  0xB3
#define PUNIT_PTTSS 0xB4
#define PUNIT_TE_AUX0   0xB5
#define PUNIT_TE_AUX1   0xB6
#define PUNIT_TE_AUX2   0xB7
#define PUNIT_TE_AUX3   0xB8
#define PUNIT_TTE_VRIccMax  0xB9
#define PUNIT_TTE_VRHot     0xBA
#define PUNIT_TTE_XXPROCHOT 0xBB
#define PUNIT_TTE_SLM0  0xBC
#define PUNIT_TTE_SLM1  0xBD
#define PUNIT_BWTE      0xBE
#define PUNIT_TTE_SWT   0xBF
#define PUNIT_PMU_DDR_0 0xC0
#define PUNIT_PMU_DDR_1 0xC1
#define PUNIT_PMU_DDR_2 0xC2
#define PUNIT_PMU_DDR_3 0xC3
#define PUNIT_PMU_DDR_4 0xC4
#define PUNIT_PMU_DDR_5 0xC5
#define PUNIT_PMU_DDR_6 0xC6
#define PUNIT_PMU_DDR_7 0xC7
#define PUNIT_PMU_DDR_ADDR  0xC8
#define PUNIT_S0IDLE_MASK   0xC9
#define PUNIT_S0IX_MASK     0xCA
#define PUNIT_RTC_S0i2_DDL_HIGH       0xCB
#define PUNIT_RTC_S0i3_DDL_LOW        0xCC
#define PUNIT_RTC_S0i3_DDL_HIGH       0xCD
#define PUNIT_GENLC_CZCOUNTER_L32     0xCE
#define PUNIT_GENLC_CZCOUNTER_H32     0xCF
#define PUNIT_GVD_SPARE0    0xD0
#define PUNIT_GPU_EC        0xD1
#define PUNIT_GPU_EC_VIRUS  0xD2
#define PUNIT_GPU_LFM       0xD3
#define PUNIT_GPU_FREQ_REQ  0xD4
#define PUNIT_GPU_TURBO_MIN_ENERGY    0xD5
#define PUNIT_GVD_SPARE1    0xD6
#define PUNIT_GVD_SPARE2    0xD7
#define PUNIT_GPU_FREQ_STS  0xD8
#define PUNIT_GVD_SPARE3    0xD9
#define PUNIT_GVD_SPARE4    0xDA
#define PUNIT_GVD_SPARE5    0xDB
#define PUNIT_MEDIA_TURBO_REQ         0xDC
#define PUNIT_GENLC_COUNTER_L32       0xDD
#define PUNIT_GENLC_COUNTER_H32       0xDE
#define PUNIT_GVD_SPARE6              0xDF
#define PUNIT_M1_OPER2_VALUE_LOW      0xE1
#define PUNIT_RTC_AONT_INCRAMT1       0xE3
#define PUNIT_RTC_AONT_INCRAMT2       0xE4
#define PUNIT_RTC_AONT_INCRAMT3       0xE5
#define PUNIT_RTC_AONT_HIGH           0xE6
#define PUNIT_RTC_AONT_LOW            0xE7
#define PUNIT_M1_OPER1_VALUE_HIGH_ADD 0xEA
#define PUNIT_M0_OPER2_VALUE_LOW_ADD  0xEB
#define PUNIT_M0_OPER2_VALUE_HIGH_ADD 0xEC
#define PUNIT_RTC_S0i2_DDL_LOW        0xED
#define PUNIT_M1_OPER2_VALUE_HIGH     0xEF
//#define   PUNIT_RSV_0xF0
//#define   PUNIT_RSV_0xF1
//#define   PUNIT_RSV_0xF2
#define PUNIT_AONT_CLOCK_CONFIG       0xF3
#define PUNIT_FUSE_BUS0 0xF4
#define PUNIT_FUSE_BUS1 0xF5
#define PUNIT_FUSE_BUS2 0xF6
#define PUNIT_FUSE_BUS3 0xF7
#define PUNIT_FUSE_BUS4 0xFA
#define PUNIT_FUSE_BUS5 0xFB
#define PUNIT_FUSE_BUS6 0xFC
#define PUNIT_FUSE_BUS7 0xFD
#define PUNIT_FUSE_BUS8 0xFE
#define PUNIT_PGOVR_SBRSTOVR_FORCE1   0x100
#define PUNIT_PGOVR_SBRSTOVR_FORCE0   0x101
#define PUNIT_PGOVR_SBRSTOVR1_FORCE1  0x102
#define PUNIT_PGOVR_SBRSTOVR1_FORCE0  0x103
#define PUNIT_MMIO_SPARE0             0x104
#define PUNIT_MMIO_SPARE1             0x105
#define PUNIT_MMIO_SPARE2             0x106
#define PUNIT_MMIO_SPARE3             0x107
#define DPTF_TELB                     0x108
#define DPTF_GFXT                     0x109
#define DPTF_VEDT                     0x10A
#define DPTF_VECT                     0x10B
#define DPTF_VSPT                     0x10C
#define DPTF_ISPT                     0x10D
#define PUNIT_MMIO_SPARE10            0x10E
#define PUNIT_MMIO_SPARE11            0x10F


//
// IUNIT(Image Signal Processor-ISP) Regsiters Offset
// Port ID:1ch
// updated to Cspec on ww49.4
// MSG IOSF-SB
//
#define IUNIT_ID                0x00
#define IUNIT_PCICMDSTS         0x04
#define IUNIT_RIDCC             0x08
#define IUNIT_HDR               0x0C
#define IUNIT_ISPMMADR          0x10
#define IUNIT_SSID              0x2C
#define IUNIT_CAPPOINT          0x34
#define IUNIT_INTR              0x3C
#define IUNIT_PMCAP             0x80
#define IUNIT_PMCS              0x84
#define IUNIT_MSI_CAPID         0x90
#define IUNIT_MSI_ADDRESS       0x94
#define IUNIT_MSI_DATA          0x98
#define IUNIT_INTERRUPT_CONTROL 0x9C
#define IUNIT_PERF0 0xB0
#define IUNIT_PERF1 0xB4
#define IUNIT_PERF2 0xB8
#define IUNIT_PERF3 0xBC
#define IUNIT_MISR0 0xC0
#define IUNIT_MISR1 0xC4
#define IUNIT_MISR2 0xC8
#define IUNIT_MISR3 0xCC
#define IUNIT_MANUFACTURING_ID  0xD0
#define IUNIT_ACCESS_CTRL_VIOL  0xD4
#define IUNIT_DEADLINE_STATUS   0xD8
#define IUNIT_AFE_HS_CONTROL    0xDC
#define IUNIT_AFE_RCOMP_CONTROL 0xE0
#define IUNIT_AFE_TRIM_CONTROL  0xE4
#define IUNIT_CSI_CONTROL       0xE8
#define IUNIT_DEADLINE_CONTROL  0xEC
#define IUNIT_RCOMP_STATUS      0xF0
#define IUNIT_RCOMP_CONTROL     0xF4
#define IUNIT_STATUS            0xF8
#define IUNIT_CONTROL           0xFC


//
// GUNIT(Graphic Control) Regsiters Offset
// Port ID:6h
#define GUNIT_GSCKGCTL          0x9028
#define GUNIT_ClockGateDisable1 0x182060
#define GUNIT_ClockGateDisable2 0x182064


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
#define IGD_DID_QS          0x0BE2      //RCOverride -a: Fix the DID error

//
// CCK Registers Offset
// Port ID: 14h
#define CCK_FUSE_REGISTER_0     0x0008
//
// DFX-Lakemore Regsiters Offset
// Port ID:1ch
// updated to Cspec on ww30.1
// MSG IOSF-SB
//
#define DFXLAKEMORE_SOCHAP_SDC  0x00
#define     B_DFXLAKEMORE_SOCHAP_SDC_CE BIT15       //Bit 15 - When this bit is set, SoChap Block is eanbled.

#endif
