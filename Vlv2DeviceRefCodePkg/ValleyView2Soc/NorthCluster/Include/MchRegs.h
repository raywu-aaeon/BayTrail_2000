/*++

Copyright (c) 2005-2009 Intel Corporation. All rights reserved
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
#define VLV_UNIT_AUNIT              0x00            /**< A Unit (IO Arbiter) */
#define VLV_UNIT_DUNIT              0x01            /**< D Unit */
#define VLV_UNIT_TUNIT              0x02            /**< H Unit */
#define VLV_UNIT_BUNIT              0x03            /**< B Unit */
#define VLV_UNIT_GUNIT              0x06            /**< GVD Unit */
#define VLV_UNIT_GUNIT2             0x07            /**< GVD Unit */
#define VLV_UNIT_FUSEEPNC           0x11            /*Fuse EndPoint Nort*/
#define VLV_UNIT_DDRIO              0x0C            /**< DDR IO Unit */
#define VLV_UNIT_REUT               0x0D            /**< REUT Unit */
#define VLV_UNIT_CCK                0x14            /*CCK */
//
// VLV SSA-BUnit (System Memory Arbiter)  Register Offset
//
//Updated by YJ 1 March 2011 referred to SSA BUNIT HAS 0.9
#define BUNIT_BSECCP_OFFSET         0x00        //BUnit Security Control Policy (BSECCP)
#define BUNIT_BSECRAC_OFFSET        0x01        //BUnit Security Register Read Access Control (BSECRAC)
#define BUNIT_BSECWAC_OFFSET        0x02        //BUnit Security Register Write Access Control (BSECWAC)
#define BUNIT_BARBCTRL0_OFFSET      0x03        //BUnit Arbiter Control (BARBCTRL0)
#define BUNIT_BARBCTRL1_OFFSET      0x04        //BUnit Arbiter Control (BARBCTRL1)
#define BUNIT_BARBCTRL2_OFFSET      0x05        //BUnit Arbiter Control (BARBCTRL2)
#define BUNIT_BARBCTRL3_OFFSET      0x06        //BUnit Arbiter Control (BARBCTRL3)
#define BUNIT_BWFLUSH_OFFSET        0x07        //BUnit Write Flush Policy (BWFLUSH)
#define BUNIT_BBANKMASK_OFFSET      0x08        //BUnit Bank Mask (BBANKMASK)
#define BUNIT_BROWMASK_OFFSET       0x09        //BUnit Row Mask (BROWMASK)
#define BUNIT_BRANKMASK_OFFSET      0x0A        //BUnit Rank Mask (BRANKMASK)
#define BUNIT_BALIMIT0_OFFSET       0x0B        //BUnit Agent Limit Control (BALIMIT0)
#define BUNIT_BALIMIT1_OFFSET       0x0C        //BUnit Agent Limit Control (BALIMIT1)
#define BUNIT_BALIMIT2_OFFSET       0x0D        //BUnit Agent Limit Control (BALIMIT2)
#define BUNIT_BALIMIT3_OFFSET       0x0E        //BUnit Agent Limit Control (BALIMIT3)
#define BUNIT_BARES0_OFFSET         0x0F        //BUnit Agent Reservations (BARES0)
#define BUNIT_BARES1_OFFSET         0x10        //BUnit Agent Reservations (BARES1)
#define BUNIT_BISOC_OFFSET          0x11        //BUnit ISOC Configuration (BISOC)
#define BUNIT_BCOSCAT_OFFSET        0x12        //BUnit COS Category (BCOSCAT)
#define BUNIT_BDPT_OFFSET           0x13        //BUnit Dynamic Prefetch Throttle (BDPT)
#define BUNIT_BFLWT_OFFSET          0x14        //BUnit Flush Weights (BFLWT)
#define BUNIT_BBWC_OFFSET           0x15        //BUnit Bandwidth Counters (BBWC)
#define BUNIT_BSCHCTRL0_OFFSET      0x18        //Bunit Scheduler Control (BSCHCTRL0)
#define BUNIT_BSCHCTRL1_OFFSET      0x19        //Bunit Scheduler Control (BSCHCTRL1)
#define BUNIT_BIMRDATA_OFFSET           0x1A        //BUnit Protected Memory Region Data Value (BIMRDATA)
#define BUNIT_BPMRVCTL_OFFSET           0x1B        //BUnit Protected Memory Range Violation Control (BPMRVCTL)
#define BUNIT_B_SECURITY_STAT0_OFFSET   0x1C        //B_SECURITY_STAT0
#define BUNIT_B_SECURITY_STAT1_OFFSET   0x1D        //B_SECURITY_STAT0


#define BUNIT_BMRCP_OFFSET          0x20        //BUnit Memory Range Control Policy (BMRCP)
#define BUNIT_BMRRAC_OFFSET         0x21        //BUnit Memory Range Read Access Control (BMRRAC)
#define BUNIT_BMRWAC_OFFSET         0x22        //BUnit Memory Range Write Access Control (BMRWAC)
#define BUNIT_BNOCACHE_OFFSET       0x23        //BUnit Non-Cached Region (BNOCACHE)
#define BUNIT_BNOCACHECTL_OFFSET    0x24        //BUnit Non-Cached Region (BNOCACHECTL)
#define BUNIT_BMBOUND_OFFSET        0x25        //BMBOUND - Bunit Memory/IO Boundary Register
#define BUNIT_BMBOUND_HI_OFFSET     0x26        //BMBOUND HI - Bunit Memory/IO HI Boundary Register
#define BUNIT_BECREG_OFFSET         0x27        //BECREG - Bunit Extended Configuration Space Config
#define BUNIT_BMISC_OFFSET          0x28        //BMISC - Bunit Miscellaneous Configuration Register
#define BUNIT_BSMRCP_OFFSET         0x2B        //BUnit System Management Range Control Policy (BSMRCP)
#define BUNIT_BSMRRAC_OFFSET        0x2C        //BUnit SMM Range Read Access Control (BSMRRAC)
#define BUNIT_BSMRWAC_OFFSET        0x2D        //BUnit SMM Range Write Access Control (BSMRWAC)
#define BUNIT_BSMMRRL_OFFSET        0x2E        //BSMMRRL - Bunit System Management Range Register Low
#define BUNIT_BSMMRRH_OFFSET        0x2F        //BSMMRRH - Bunit System Management Range Register Hi

#define BUNIT_BDBCP_OFFSET          0x38        //BUnit Debug Control Policy (BDBCP)
#define BUNIT_BDRRAC_OFFSET         0x39        //BUnit Debug Config Register Read Access Control (BDRRAC)
#define BUNIT_BDRWAC_OFFSET         0x3A        //Bunit Debug Config Register Write Access Control (BCRWAC)
#define BUNIT_BDEBUG0_OFFSET        0x3B        //Bunit Debug Register 0 (BDEBUG0)
#define BUNIT_BDEBUG1_OFFSET        0x3C        //Bunit Debug Register 1 (BDEBUG1)
#define BUNIT_BCTRL_OFFSET          0x3D        //Bunit Control (BCTRL)
#define BUNIT_BTHCTRL_OFFSET        0x3E        //BUnit Throttling Control (BTHCTRL)
#define BUNIT_BTHMASK_OFFSET        0x3F        //Bunit Throttling Masks (BTHMASK)

#define BUNIT_BIACP_OFFSET          0x40        //BUnit IA Core Control Policy (BIACP)
#define BUNIT_BIARAC_OFFSET         0x41        //Bunit IA Core Config Register Read Access Control (BIARAC)
#define BUNIT_BIAWAC_OFFSET         0x42        //Bunit IA Core Config Register Write Access Control (BIAWAC)

#define BUNIT_BEXMCP_OFFSET         0x43        //BUnit Extended Micro-Code Control Policy (BEXMCP)
#define BUNIT_BEXMRAC_OFFSET        0x44        //BUnit Extended Micro-Code Read Access Control (BEXMRAC)
#define BUNIT_BEXMWAC_OFFSET        0x45        //BUnit Extended Micro-Code Write Access Control (BEXMWAC)
#define BUNIT_EXML_OFFSET           0x46        //Extended Micro-Code Range Low (EXML)
#define BUNIT_EXMH_OFFSET           0x47        //Extended Micro-Code Range High (EXMH)

#define BUNIT_LP0Mode_OFFSET        0x48        //Logical Processor 0 Mode Register (LP0Mode)
#define BUNIT_LP1Mode_OFFSET        0x49        //Logical Processor 1 Mode Register (LP1Mode)
#define BUNIT_LP2Mode_OFFSET        0x4A        //Logical Processor 2 Mode Register (LP2Mode)
#define BUNIT_LP3Mode_OFFSET        0x4B        //Logical Processor 3 Mode Register (LP3Mode)

#define BUNIT_EMONCTL0_OFFSET       0x50        //BUnit EMON Control 0 (EMONCTL0)
#define BUNIT_EMONCTL1_OFFSET       0x51        //BUnit EMON Control 1 (EMONCTL1)
#define BUNIT_EMONCTL2_OFFSET       0x52        //BUnit EMON Control 2 (EMONCTL2)
#define BUNIT_EMONCTL3_OFFSET       0x53        //BUnit EMON Control 3 (EMONCTL3)

#define BUNIT_BIMR0CP_OFFSET        0x60        //BUnit Isolated Memory Region 0 Control Policy (BIMR0CP)
#define BUNIT_BIMR1CP_OFFSET        0x61        //BUnit Isolated Memory Region 1 Control Policy (BIMR1CP)
#define BUNIT_BIMR2CP_OFFSET        0x62        //BUnit Isolated Memory Region 2 Control Policy (BIMR2CP)
#define BUNIT_BIMR3CP_OFFSET        0x63        //BUnit Isolated Memory Region 3 Control Policy (BIMR3CP)
#define BUNIT_BIMR4CP_OFFSET        0x64        //BUnit Isolated Memory Region 4 Control Policy (BIMR4CP)
#define BUNIT_BIMR5CP_OFFSET        0x65        //BUnit Isolated Memory Region 5 Control Policy (BIMR5CP)
#define BUNIT_BIMR6CP_OFFSET        0x66        //BUnit Isolated Memory Region 6 Control Policy (BIMR6CP)
#define BUNIT_BIMR7CP_OFFSET        0x67        //BUnit Isolated Memory Region 7 Control Policy (BIMR7CP)
#define BUNIT_BIMR8CP_OFFSET        0x68        //BUnit Isolated Memory Region 8 Control Policy (BIMR8CP)
#define BUNIT_BIMR9CP_OFFSET        0x69        //BUnit Isolated Memory Region 9 Control Policy (BIMR9CP)
#define BUNIT_BIMR10CP_OFFSET       0x6A        //BUnit Isolated Memory Region 10 Control Policy (BIMR10CP)
#define BUNIT_BIMR11CP_OFFSET       0x6B        //BUnit Isolated Memory Region 11 Control Policy (BIMR11CP)
#define BUNIT_BIMR12CP_OFFSET       0x6C        //BUnit Isolated Memory Region 12 Control Policy (BIMR12CP)
#define BUNIT_BIMR13CP_OFFSET       0x6D        //BUnit Isolated Memory Region 13 Control Policy (BIMR13CP)
#define BUNIT_BIMR14CP_OFFSET       0x6E        //BUnit Isolated Memory Region 14 Control Policy (BIMR14CP)
#define BUNIT_BIMR15CP_OFFSET       0x6F        //BUnit Isolated Memory Region 15 Control Policy (BIMR15CP)
#define BUNIT_BIMR16CP_OFFSET       0x70        //BUnit Isolated Memory Region 16 Control Policy (BIMR16CP)
#define BUNIT_BIMR17CP_OFFSET       0x71        //BUnit Isolated Memory Region 17 Control Policy (BIMR17CP)
#define BUNIT_BIMR18CP_OFFSET       0x72        //BUnit Isolated Memory Region 18 Control Policy (BIMR18CP)
#define BUNIT_BIMR19CP_OFFSET       0x73        //BUnit Isolated Memory Region 19 Control Policy (BIMR19CP)
#define BUNIT_BIMR20CP_OFFSET       0x74        //BUnit Isolated Memory Region 20 Control Policy (BIMR20CP)
#define BUNIT_BIMR21CP_OFFSET       0x75        //BUnit Isolated Memory Region 21 Control Policy (BIMR21CP)
#define BUNIT_BIMR22CP_OFFSET       0x76        //BUnit Isolated Memory Region 22 Control Policy (BIMR22CP)
#define BUNIT_BIMR23CP_OFFSET       0x77        //BUnit Isolated Memory Region 23 Control Policy (BIMR23CP)
#define BUNIT_BIMR24CP_OFFSET       0x78        //BUnit Isolated Memory Region 24 Control Policy (BIMR24CP)
#define BUNIT_BIMR25CP_OFFSET       0x79        //BUnit Isolated Memory Region 25 Control Policy (BIMR25CP)
#define BUNIT_BIMR26CP_OFFSET       0x7A        //BUnit Isolated Memory Region 26 Control Policy (BIMR26CP)
#define BUNIT_BIMR27CP_OFFSET       0x7B        //BUnit Isolated Memory Region 27 Control Policy (BIMR27CP)
#define BUNIT_BIMR28CP_OFFSET       0x7C        //BUnit Isolated Memory Region 28 Control Policy (BIMR28CP)
#define BUNIT_BIMR29CP_OFFSET       0x7D        //BUnit Isolated Memory Region 29 Control Policy (BIMR29CP)
#define BUNIT_BIMR30CP_OFFSET       0x7E        //BUnit Isolated Memory Region 30 Control Policy (BIMR30CP)
#define BUNIT_BIMR31CP_OFFSET       0x7F        //BUnit Isolated Memory Region 31 Control Policy (BIMR31CP)

#define BUNIT_BIMR0L_OFFSET         0x80        //BUnit Isolated Memory Region 0 Low (BIMR0L)
#define BUNIT_BIMR0H_OFFSET         0x81        //BUnit Isolated Memory Region 0 High (BIMR0H)
#define BUNIT_BIMR0RAC_OFFSET       0x82        //BUnit Isolated Memory Region 0 Read Access Control (BIMR0RAC)
#define BUNIT_BIMR0WAC_OFFSET       0x83        //BUnit Isolated Memory Region 0 Write Access Control (BIMR0WAC)
#define BUNIT_BIMR1L_OFFSET         0x84        //BUnit Isolated Memory Region 1 Low (BIMR1L)
#define BUNIT_BIMR1H_OFFSET         0x85        //BUnit Isolated Memory Region 1 High (BIMR1H)
#define BUNIT_BIMR1RAC_OFFSET       0x86        //BUnit Isolated Memory Region 1 Read Access Control (BIMR1RAC)
#define BUNIT_BIMR1WAC_OFFSET       0x87        //BUnit Isolated Memory Region 1 Write Access Control (BIMR1WAC)
#define BUNIT_BIMR2L_OFFSET         0x88        //BUnit Isolated Memory Region 2 Low (BIMR2L)
#define BUNIT_BIMR2H_OFFSET         0x89        //BUnit Isolated Memory Region 2 High (BIMR2H)
#define BUNIT_BIMR2RAC_OFFSET       0x8A        //BUnit Isolated Memory Region 2 Read Access Control (BIMR2RAC)
#define BUNIT_BIMR2WAC_OFFSET       0x8B        //BUnit Isolated Memory Region 2 Write Access Control (BIMR2WAC)
#define BUNIT_BIMR3L_OFFSET         0x8C        //BUnit Isolated Memory Region 3 Low (BIMR3L)
#define BUNIT_BIMR3H_OFFSET         0x8D        //BUnit Isolated Memory Region 3 High (BIMR3H)
#define BUNIT_BIMR3RAC_OFFSET       0x8E        //BUnit Isolated Memory Region 3 Read Access Control (BIMR3RAC)
#define BUNIT_BIMR3WAC_OFFSET       0x8F        //BUnit Isolated Memory Region 3 Write Access Control (BIMR3WAC)

#define BUNIT_PTIBASE_OFFSET        0x100       //BUnit PTI Trace Agent Base Address (PTIBASE)
#define BUNIT_PTIRSIZE_OFFSET       0x101       //BUnit PTI Trace Agent Region Size (PTIRSIZE)
#define BUNIT_PTIWWMODCFG_OFFSET    0x102       //BUnit WINDOW WATCHER PTI MODE CONFIG (PTIWWMODCFG)
#define BUNIT_PTIUCOUNTER_OFFSET    0x103       //BUnit PTI Microsecond Counter (PTIUCOUNTER)
#define BUNIT_PTITSELOP_OFFSET      0x104       //BUnit PTI Trigger Select Observation Point (PTITSELOP)
#define BUNIT_PTITSELGRP_OFFSET     0x105       //BUnit PTI Trigger Select Group (PTITSELGRP)
#define BUNIT_PTI0CTL_OFFSET        0x110       //BUnit PTI Snoop Agent 0 Control (PTI0CTL)
#define BUNIT_PTI0SAIMATCH_OFFSET   0x111       //BUnit PTI Snoop Agent 0 SAI Index Match (PTI0SAIMATCH))
#define BUNIT_PTI0IDIREQ_OFFSET     0x112       //BUnit PTI Snoop Agent 0 IDI Request Filter (PTI0IDIREQ)
#define BUNIT_PTI0ADDRHI1_OFFSET    0x113       //BUnit PTI Snoop Agent 0 Address Hi Bound1 (PTI0ADDRHI1)
#define BUNIT_PTI0ADDRHI0_OFFSET    0x114       //BUnit PTI Snoop Agent 0 Address Hi Bound0 (PTI0ADDRHI0))
#define BUNIT_PTI0ADDRLO1_OFFSET    0x115       //BUnit PTI Snoop Agent 0 Address Lo Bound1 (PTI0ADDRLO1)
#define BUNIT_PTI0ADDRLO0_OFFSET    0x116       //BUnit PTI Snoop Agent 0 Address Lo Bound0 (PTI0ADDRLO0)
#define BUNIT_PTI0DATA_OFFSET       0x117       //BUnit PTI Snoop Agent 0 Data Filter (PTI0DATA)
#define BUNIT_PTI0DMASK_OFFSET      0x118       //BUnit PTI Snoop Agent 0 Data Mask (PTI0DMASK)


//      VLV RUnit(DDRIO) Register Symbol            Register Start
#define DDRIO_DQMODULE_BYTE01                                   0x0
#define DDRIO_DQMODULE_BYTE23                                   0x800
#define DDRIO_DQMODULE_BYTE45                                   0x1000
#define DDRIO_DQMODULE_BYTE67                                   0x1800
#define DDRIO_COMMAND                                           0x4800
#define DDRIO_CLOCKCONTROL                                      0x5800
#define DDRIO_COMP                                              0x6800
#define DDRIO_COMP2                                             0x6900
#define DDRIO_PLL                                               0x7800
#define DDRIO_ALL_DQMODULE                                      0xF000

#define DDRIO_DQ0_OBSCKEBBCTL_OFFSET                            0x00
#define DDRIO_DQ0_DLLTXCTL_OFFSET                               0x04
#define DDRIO_DQ0_DLLRXCTL_OFFSET                               0x08
#define DDRIO_DQ0_MDLLCTL_OFFSET                                0x0C
#define DDRIO_DQ0_B0RXIOBUFCTL_OFFSET                           0x10
#define DDRIO_DQ0_B0VREFCTL_OFFSET              0x14
#define DDRIO_DQ0_B0RXOFFSET0_OFFSET                            0x18
#define DDRIO_DQ0_B0RXOFFSET1_OFFSET                            0x1C
#define DDRIO_DQ0_B1RXIOBUFCTL_OFFSET                           0x20
#define DDRIO_DQ0_B1VREFCTL_OFFSET                              0x24
#define DDRIO_DQ0_B1RXOFFSET1_OFFSET                            0x28
#define DDRIO_DQ0_B1RXOFFSET0_OFFSET                            0x2C
#define DDRIO_DQ0_DFTCTL_OFFSET                                 0x30
#define DDRIO_DQ0_DQTRAINSTS_OFFSET                             0x34
#define DDRIO_DQ0_DLLPICODER0B1_OFFSET                          0x38
#define DDRIO_DQ0_DLLPICODER0B0_OFFSET                          0x3C
#define DDRIO_DQ0_DLLPICODER1B1_OFFSET                          0x40
#define DDRIO_DQ0_DLLPICODER1B0_OFFSET                          0x44
#define DDRIO_DQ0_DLLPICODER2B1_OFFSET                          0x48
#define DDRIO_DQ0_DLLPICODER2B0_OFFSET                          0x4C
#define DDRIO_DQ0_DLLPICODER3B1_OFFSET                          0x50
#define DDRIO_DQ0_DLLPICODER3B0_OFFSET                          0x54
#define DDRIO_DQ0_RXDQSPICODEB1_OFFSET                          0x58
#define DDRIO_DQ0_RXDQSPICODEB0_OFFSET                          0x5C
#define DDRIO_DQ0_RXDQPICODEB1R32_OFFSET                        0x60
#define DDRIO_DQ0_RXDQPICODEB1R10_OFFSET                        0x64
#define DDRIO_DQ0_RXDQPICODEB0R32_OFFSET                        0x68
#define DDRIO_DQ0_RXDQPICODEB0R10_OFFSET                        0x6C
#define DDRIO_DQ0_PTRCTL0_OFFSET                                0x70
#define DDRIO_DQ0_PTRCTL1_OFFSET                                0x74
#define DDRIO_DQ0_DBCTL0_OFFSET                                 0x78
#define DDRIO_DQ0_DBCTL1_OFFSET                                 0x7C
#define DDRIO_DQ0_LATCTL0_B0_OFFSET                             0x80
#define DDRIO_DQ0_LATCTL0_B1_OFFSET                             0x84
#define DDRIO_DQ0_LATCTL1_OFFSET                                0x88
#define DDRIO_DQ0_ONDURCTL_B0_OFFSET                            0x8C
#define DDRIO_DQ0_ONDURCTL_B1_OFFSET                            0x90
#define DDRIO_DQ0_OVRCTL_B0_OFFSET                              0x94
#define DDRIO_DQ0_OVRCTL_B1_OFFSET                              0x98
#define DDRIO_DQ0_DQCTL_OFFSET                                  0x9C
#define DDRIO_DQ0_RK2RKCHG_PTRCTRL_B0_OFFSET                    0xA0
#define DDRIO_DQ0_RK2RKCHG_PTRCTRL_B1_OFFSET                    0xA4
#define DDRIO_DQ0_RK2RKCTL_OFFSET                               0xA8
#define DDRIO_DQ0_RK2RK_PTRCTL_OFFSET                           0xAC
#define DDRIO_DQ0_RK2RKLAT_B0_OFFSET                            0xB0
#define DDRIO_DQ0_RK2RKLAT_B1_OFFSET                            0xB4
#define DDRIO_DQ0_CLKALIGN_REG0_OFFSET                          0xB8
#define DDRIO_DQ0_CLKALIGN_REG1_OFFSET                          0xBC
#define DDRIO_DQ0_CLKALIGN_REG2_OFFSET                          0xC0
#define DDRIO_DQ0_CLKALIGN_STS0_OFFSET                          0xC4
#define DDRIO_DQ0_CLKALIGN_STS1_OFFSET                          0xC8
#define DDRIO_DQ0_CLOCK_GATE_OFFSET                             0xCC
#define DDRIO_DQ0_COMPSLV_1_B0_OFFSET                           0xD0
#define DDRIO_DQ0_COMPSLV_1_B1_OFFSET                           0xD4
#define DDRIO_DQ0_COMPSLV_2_B0_OFFSET                           0xD8
#define DDRIO_DQ0_COMPSLV_2_B1_OFFSET                           0xDC
#define DDRIO_DQ0_COMPSLV_3_B0_OFFSET                           0xE0
#define DDRIO_DQ0_COMPSLV_3_B1_OFFSET                           0xE4
#define DDRIO_DQ0_VISACTRL0_OFFSET                              0xE8
#define DDRIO_DQ0_VISACTRL1_OFFSET                              0xEC
#define DDRIO_DQ0_VISAPATGEN_OFFSET                             0xF0
#define DDRIO_DQ0_VISACSTR_OFFSET                               0xF4
#define DDRIO_DQ0_IOCTL1_OFFSET                                 0x11C
#define DDRIO_DQ1_OBSCKEBBCTL_OFFSET                            0x0800
#define DDRIO_DQ1_DLLTXCTL_OFFSET                               0x0804
#define DDRIO_DQ1_DLLRXCTL_OFFSET                               0x0808
#define DDRIO_DQ1_MDLLCTL_OFFSET                                0x080C
#define DDRIO_DQ1_B0RXIOBUFCTL_OFFSET                           0x0810
#define DDRIO_DQ1_B0VREFCTL_OFFSET                              0x0814
#define DDRIO_DQ1_B0RXOFFSET0_OFFSET                            0x0818
#define DDRIO_DQ1_B0RXOFFSET1_OFFSET                            0x081C
#define DDRIO_DQ1_B1RXIOBUFCTL_OFFSET                           0x0820
#define DDRIO_DQ1_B1VREFCTL_OFFSET                              0x0824
#define DDRIO_DQ1_B1RXOFFSET1_OFFSET                            0x0828
#define DDRIO_DQ1_B1RXOFFSET0_OFFSET                            0x082C
#define DDRIO_DQ1_DFTCTL_OFFSET                                 0x0830
#define DDRIO_DQ1_DQTRAINSTS_OFFSET                             0x0834
#define DDRIO_DQ1_DLLPICODER0B1_OFFSET                          0x0838
#define DDRIO_DQ1_DLLPICODER0B0_OFFSET                          0x083C
#define DDRIO_DQ1_DLLPICODER1B1_OFFSET                          0x0840
#define DDRIO_DQ1_DLLPICODER1B0_OFFSET                          0x0844
#define DDRIO_DQ1_DLLPICODER2B1_OFFSET                          0x0848
#define DDRIO_DQ1_DLLPICODER2B0_OFFSET                          0x084C
#define DDRIO_DQ1_DLLPICODER3B1_OFFSET                          0x0850
#define DDRIO_DQ1_DLLPICODER3B0_OFFSET                          0x0854
#define DDRIO_DQ1_RXDQSPICODEB1_OFFSET                          0x0858
#define DDRIO_DQ1_RXDQSPICODEB0_OFFSET                          0x085C
#define DDRIO_DQ1_RXDQPICODEB1R32_OFFSET                        0x0860
#define DDRIO_DQ1_RXDQPICODEB1R10_OFFSET                        0x0864
#define DDRIO_DQ1_RXDQPICODEB0R32_OFFSET                        0x0868
#define DDRIO_DQ1_RXDQPICODEB0R10_OFFSET                        0x086C
#define DDRIO_DQ1_PTRCTL0_OFFSET                                0x0870
#define DDRIO_DQ1_PTRCTL1_OFFSET                                0x0874
#define DDRIO_DQ1_DBCTL0_OFFSET                                 0x0878
#define DDRIO_DQ1_DBCTL1_OFFSET                                 0x087C
#define DDRIO_DQ1_LATCTL0_B0_OFFSET                             0x0880
#define DDRIO_DQ1_LATCTL0_B1_OFFSET                             0x0884
#define DDRIO_DQ1_LATCTL1_OFFSET                                0x0888
#define DDRIO_DQ1_ONDURCTL_B0_OFFSET                            0x088C
#define DDRIO_DQ1_ONDURCTL_B1_OFFSET                            0x0890
#define DDRIO_DQ1_OVRCTL_B0_OFFSET                              0x0894
#define DDRIO_DQ1_OVRCTL_B1_OFFSET                              0x0898
#define DDRIO_DQ1_DQCTL_OFFSET                                  0x089C
#define DDRIO_DQ1_RK2RKCHG_PTRCTRL_B0_OFFSET                    0x08A0
#define DDRIO_DQ1_RK2RKCHG_PTRCTRL_B1_OFFSET                    0x08A4
#define DDRIO_DQ1_RK2RKCTL_OFFSET                               0x08A8
#define DDRIO_DQ1_RK2RK_PTRCTL_OFFSET                           0x08AC
#define DDRIO_DQ1_RK2RKLAT_B0_OFFSET                            0x08B0
#define DDRIO_DQ1_RK2RKLAT_B1_OFFSET                            0x08B4
#define DDRIO_DQ1_CLKALIGN_REG0_OFFSET                          0x08B8
#define DDRIO_DQ1_CLKALIGN_REG1_OFFSET                          0x08BC
#define DDRIO_DQ1_CLKALIGN_REG2_OFFSET                          0x08C0
#define DDRIO_DQ1_CLKALIGN_STS0_OFFSET                          0x08C4
#define DDRIO_DQ1_CLKALIGN_STS1_OFFSET                          0x08C8
#define DDRIO_DQ1_COMPSLV_1_B0_OFFSET                           0x08CC
#define DDRIO_DQ1_COMPSLV_1_B1_OFFSET                           0x08D0
#define DDRIO_DQ1_COMPSLV_2_B0_OFFSET                           0x08D4
#define DDRIO_DQ1_COMPSLV_2_B1_OFFSET                           0x08D8
#define DDRIO_DQ1_COMPSLV_3_B0_OFFSET                           0x08DC
#define DDRIO_DQ1_COMPSLV_3_B1_OFFSET                           0x08E0
#define DDRIO_DQ2_OBSCKEBBCTL_OFFSET                            0x1000
#define DDRIO_DQ2_DLLTXCTL_OFFSET                               0x1004
#define DDRIO_DQ2_DLLRXCTL_OFFSET                               0x1008
#define DDRIO_DQ2_MDLLCTL_OFFSET                                0x100C
#define DDRIO_DQ2_B0RXIOBUFCTL_OFFSET                           0x1010
#define DDRIO_DQ2_B0VREFCTL_OFFSET                              0x1014
#define DDRIO_DQ2_B0RXOFFSET0_OFFSET                            0x1018
#define DDRIO_DQ2_B0RXOFFSET1_OFFSET                            0x101C
#define DDRIO_DQ2_B1RXIOBUFCTL_OFFSET                           0x1020
#define DDRIO_DQ2_B1VREFCTL_OFFSET                              0x1024
#define DDRIO_DQ2_B1RXOFFSET1_OFFSET                            0x1028
#define DDRIO_DQ2_B1RXOFFSET0_OFFSET                            0x102C
#define DDRIO_DQ2_DFTCTL_OFFSET                                 0x1030
#define DDRIO_DQ2_DQTRAINSTS_OFFSET                             0x1034
#define DDRIO_DQ2_DLLPICODER0B1_OFFSET                          0x1038
#define DDRIO_DQ2_DLLPICODER0B0_OFFSET                          0x103C
#define DDRIO_DQ2_DLLPICODER1B1_OFFSET                          0x1040
#define DDRIO_DQ2_DLLPICODER1B0_OFFSET                          0x1044
#define DDRIO_DQ2_DLLPICODER2B1_OFFSET                          0x1048
#define DDRIO_DQ2_DLLPICODER2B0_OFFSET                          0x104C
#define DDRIO_DQ2_DLLPICODER3B1_OFFSET                          0x1050
#define DDRIO_DQ2_DLLPICODER3B0_OFFSET                          0x1054
#define DDRIO_DQ2_RXDQSPICODEB1_OFFSET                          0x1058
#define DDRIO_DQ2_RXDQSPICODEB0_OFFSET                          0x105C
#define DDRIO_DQ2_RXDQPICODEB1R32_OFFSET                        0x1060
#define DDRIO_DQ2_RXDQPICODEB1R10_OFFSET                        0x1064
#define DDRIO_DQ2_RXDQPICODEB0R32_OFFSET                        0x1068
#define DDRIO_DQ2_RXDQPICODEB0R10_OFFSET                        0x106C
#define DDRIO_DQ2_PTRCTL0_OFFSET                0x1070
#define DDRIO_DQ2_PTRCTL1_OFFSET                0x1074
#define DDRIO_DQ2_DBCTL0_OFFSET                 0x1078
#define DDRIO_DQ2_DBCTL1_OFFSET                 0x107C
#define DDRIO_DQ2_LATCTL0_B0_OFFSET             0x1080
#define DDRIO_DQ2_LATCTL0_B1_OFFSET             0x1084
#define DDRIO_DQ2_LATCTL1_OFFSET                0x1088
#define DDRIO_DQ2_ONDURCTL_B0_OFFSET                            0x108C
#define DDRIO_DQ2_ONDURCTL_B1_OFFSET                            0x1090
#define DDRIO_DQ2_OVRCTL_B0_OFFSET              0x1094
#define DDRIO_DQ2_OVRCTL_B1_OFFSET              0x1098
#define DDRIO_DQ2_DQCTL_OFFSET                  0x109C
#define DDRIO_DQ2_RK2RKCHG_PTRCTRL_B0_OFFSET                    0x10A0
#define DDRIO_DQ2_RK2RKCHG_PTRCTRL_B1_OFFSET                    0x10A4
#define DDRIO_DQ2_RK2RKCTL_OFFSET               0x10A8
#define DDRIO_DQ2_RK2RK_PTRCTL_OFFSET                           0x10AC
#define DDRIO_DQ2_RK2RKLAT_B0_OFFSET                            0x10B0
#define DDRIO_DQ2_RK2RKLAT_B1_OFFSET                            0x10B4
#define DDRIO_DQ2_CLKALIGN_REG0_OFFSET                          0x10B8
#define DDRIO_DQ2_CLKALIGN_REG1_OFFSET                          0x10BC
#define DDRIO_DQ2_CLKALIGN_REG2_OFFSET                          0x10C0
#define DDRIO_DQ2_CLKALIGN_STS0_OFFSET                          0x10C4
#define DDRIO_DQ2_CLKALIGN_STS1_OFFSET                          0x10C8
#define DDRIO_DQ2_COMPSLV_1_B0_OFFSET                           0x10CC
#define DDRIO_DQ2_COMPSLV_1_B1_OFFSET                           0x10D0
#define DDRIO_DQ2_COMPSLV_2_B0_OFFSET                           0x10D4
#define DDRIO_DQ2_COMPSLV_2_B1_OFFSET                           0x10D8
#define DDRIO_DQ2_COMPSLV_3_B0_OFFSET                           0x10DC
#define DDRIO_DQ2_COMPSLV_3_B1_OFFSET                           0x10E0
#define DDRIO_DQ3_OBSCKEBBCTL_OFFSET                            0x1800
#define DDRIO_DQ3_DLLTXCTL_OFFSET                               0x1804
#define DDRIO_DQ3_DLLRXCTL_OFFSET                               0x1808
#define DDRIO_DQ3_MDLLCTL_OFFSET                                0x180C
#define DDRIO_DQ3_B0RXIOBUFCTL_OFFSET                           0x1810
#define DDRIO_DQ3_B0VREFCTL_OFFSET                              0x1814
#define DDRIO_DQ3_B0RXOFFSET0_OFFSET                            0x1818
#define DDRIO_DQ3_B0RXOFFSET1_OFFSET                            0x181C
#define DDRIO_DQ3_B1RXIOBUFCTL_OFFSET                           0x1820
#define DDRIO_DQ3_B1VREFCTL_OFFSET                              0x1824
#define DDRIO_DQ3_B1RXOFFSET1_OFFSET                            0x1828
#define DDRIO_DQ3_B1RXOFFSET0_OFFSET                            0x182C
#define DDRIO_DQ3_DFTCTL_OFFSET                                 0x1830
#define DDRIO_DQ3_DQTRAINSTS_OFFSET                             0x1834
#define DDRIO_DQ3_DLLPICODER0B1_OFFSET                          0x1838
#define DDRIO_DQ3_DLLPICODER0B0_OFFSET                          0x183C
#define DDRIO_DQ3_DLLPICODER1B1_OFFSET                          0x1840
#define DDRIO_DQ3_DLLPICODER1B0_OFFSET                          0x1844
#define DDRIO_DQ3_DLLPICODER2B1_OFFSET                          0x1848
#define DDRIO_DQ3_DLLPICODER2B0_OFFSET                          0x184C
#define DDRIO_DQ3_DLLPICODER3B1_OFFSET                          0x1850
#define DDRIO_DQ3_DLLPICODER3B0_OFFSET                          0x1854
#define DDRIO_DQ3_RXDQSPICODEB1_OFFSET                          0x1858
#define DDRIO_DQ3_RXDQSPICODEB0_OFFSET                          0x185C
#define DDRIO_DQ3_RXDQPICODEB1R32_OFFSET                        0x1860
#define DDRIO_DQ3_RXDQPICODEB1R10_OFFSET                        0x1864
#define DDRIO_DQ3_RXDQPICODEB0R32_OFFSET                        0x1868
#define DDRIO_DQ3_RXDQPICODEB0R10_OFFSET                        0x186C
#define DDRIO_DQ3_PTRCTL0_OFFSET                                0x1870
#define DDRIO_DQ3_PTRCTL1_OFFSET                                0x1874
#define DDRIO_DQ3_DBCTL0_OFFSET                                 0x1878
#define DDRIO_DQ3_DBCTL1_OFFSET                                 0x187C
#define DDRIO_DQ3_LATCTL0_B0_OFFSET                             0x1880
#define DDRIO_DQ3_LATCTL0_B1_OFFSET                             0x1884
#define DDRIO_DQ3_LATCTL1_OFFSET                                0x1888
#define DDRIO_DQ3_ONDURCTL_B0_OFFSET                            0x188C
#define DDRIO_DQ3_ONDURCTL_B1_OFFSET                            0x1890
#define DDRIO_DQ3_OVRCTL_B0_OFFSET                              0x1894
#define DDRIO_DQ3_OVRCTL_B1_OFFSET                              0x1898
#define DDRIO_DQ3_DQCTL_OFFSET                                  0x189C
#define DDRIO_DQ3_RK2RKCHG_PTRCTRL_B0_OFFSET                    0x18A0
#define DDRIO_DQ3_RK2RKCHG_PTRCTRL_B1_OFFSET                    0x18A4
#define DDRIO_DQ3_RK2RKCTL_OFFSET                               0x18A8
#define DDRIO_DQ3_RK2RK_PTRCTL_OFFSET                           0x18AC
#define DDRIO_DQ3_RK2RKLAT_B0_OFFSET                            0x18B0
#define DDRIO_DQ3_RK2RKLAT_B1_OFFSET                            0x18B4
#define DDRIO_DQ3_CLKALIGN_REG0_OFFSET                          0x18B8
#define DDRIO_DQ3_CLKALIGN_REG1_OFFSET                          0x18BC
#define DDRIO_DQ3_CLKALIGN_REG2_OFFSET                          0x18C0
#define DDRIO_DQ3_CLKALIGN_STS0_OFFSET                          0x18C4
#define DDRIO_DQ3_CLKALIGN_STS1_OFFSET                          0x18C8
#define DDRIO_DQ3_COMPSLV_1_B0_OFFSET                           0x18CC
#define DDRIO_DQ3_COMPSLV_1_B1_OFFSET                           0x18D0
#define DDRIO_DQ3_COMPSLV_2_B0_OFFSET                           0x18D4
#define DDRIO_DQ3_COMPSLV_2_B1_OFFSET                           0x18D8
#define DDRIO_DQ3_COMPSLV_3_B0_OFFSET                           0x18DC
#define DDRIO_DQ3_COMPSLV_3_B1_OFFSET                           0x18E0

//ECC
#define DDRIO_ECC_OBSCKEBBCTL_OFFSET                            0x2000
#define DDRIO_ECC_DLLTXCTL_OFFSET                               0x2004
#define DDRIO_ECC_DLLRXCTL_OFFSET                               0x2008
#define DDRIO_ECC_MDLLCTL_OFFSET                                0x200C
#define DDRIO_ECC_B0RXIOBUFCTL_OFFSET                           0x2010
#define DDRIO_ECC_B0VREFCTL_OFFSET              0x2014
#define DDRIO_ECC_B0RXOFFSET0_OFFSET                            0x2018
#define DDRIO_ECC_B0RXOFFSET1_OFFSET                            0x201C
#define DDRIO_ECC_B1RXIOBUFCTL_OFFSET                           0x2020
#define DDRIO_ECC_B1VREFCTL_OFFSET                              0x2024
#define DDRIO_ECC_B1RXOFFSET1_OFFSET                            0x2028
#define DDRIO_ECC_B1RXOFFSET0_OFFSET                            0x202C
#define DDRIO_ECC_DFTCTL_OFFSET                                 0x2030
#define DDRIO_ECC_DQTRAINSTS_OFFSET                             0x2034
#define DDRIO_ECC_DLLPICODER0B1_OFFSET                          0x2038
#define DDRIO_ECC_DLLPICODER0B0_OFFSET                          0x203C
#define DDRIO_ECC_DLLPICODER1B1_OFFSET                          0x2040
#define DDRIO_ECC_DLLPICODER1B0_OFFSET                          0x2044
#define DDRIO_ECC_DLLPICODER2B1_OFFSET                          0x2048
#define DDRIO_ECC_DLLPICODER2B0_OFFSET                          0x204C
#define DDRIO_ECC_DLLPICODER3B1_OFFSET                          0x2050
#define DDRIO_ECC_DLLPICODER3B0_OFFSET                          0x2054
#define DDRIO_ECC_RXDQSPICODEB1_OFFSET                          0x2058
#define DDRIO_ECC_RXDQSPICODEB0_OFFSET                          0x205C
#define DDRIO_ECC_RXDQPICODEB1R32_OFFSET                        0x2060
#define DDRIO_ECC_RXDQPICODEB1R10_OFFSET                        0x2064
#define DDRIO_ECC_RXDQPICODEB0R32_OFFSET                        0x2068
#define DDRIO_ECC_RXDQPICODEB0R10_OFFSET                        0x206C
#define DDRIO_ECC_PTRCTL0_OFFSET                                0x2070
#define DDRIO_ECC_PTRCTL1_OFFSET                                0x2074
#define DDRIO_ECC_DBCTL0_OFFSET                                 0x2078
#define DDRIO_ECC_DBCTL1_OFFSET                                 0x207C
#define DDRIO_ECC_LATCTL0_B0_OFFSET                             0x2080
#define DDRIO_ECC_LATCTL0_B1_OFFSET                             0x2084
#define DDRIO_ECC_LATCTL1_OFFSET                                0x2088
#define DDRIO_ECC_ONDURCTL_B0_OFFSET                            0x208C
#define DDRIO_ECC_ONDURCTL_B1_OFFSET                            0x2090
#define DDRIO_ECC_OVRCTL_B0_OFFSET                              0x2094
#define DDRIO_ECC_OVRCTL_B1_OFFSET                              0x2098
#define DDRIO_ECC_DQCTL_OFFSET                                  0x209C
#define DDRIO_ECC_RK2RKCHG_PTRCTRL_B0_OFFSET                    0x20A0
#define DDRIO_ECC_RK2RKCHG_PTRCTRL_B1_OFFSET                    0x20A4
#define DDRIO_ECC_RK2RKCTL_OFFSET                               0x20A8
#define DDRIO_ECC_RK2RK_PTRCTL_OFFSET                           0x20AC
#define DDRIO_ECC_RK2RKLAT_B0_OFFSET                            0x20B0
#define DDRIO_ECC_RK2RKLAT_B1_OFFSET                            0x20B4
#define DDRIO_ECC_CLKALIGN_REG0_OFFSET                          0x20B8
#define DDRIO_ECC_CLKALIGN_REG1_OFFSET                          0x20BC
#define DDRIO_ECC_CLKALIGN_REG2_OFFSET                          0x20C0
#define DDRIO_ECC_CLKALIGN_STS0_OFFSET                          0x20C4
#define DDRIO_ECC_CLKALIGN_STS1_OFFSET                          0x20C8
#define DDRIO_ECC_CLOCK_GATE_OFFSET                             0x20CC
#define DDRIO_ECC_COMPSLV_1_B0_OFFSET                           0x20D0
#define DDRIO_ECC_COMPSLV_1_B1_OFFSET                           0x20D4
#define DDRIO_ECC_COMPSLV_2_B0_OFFSET                           0x20D8
#define DDRIO_ECC_COMPSLV_2_B1_OFFSET                           0x20DC
#define DDRIO_ECC_COMPSLV_3_B0_OFFSET                           0x20E0
#define DDRIO_ECC_COMPSLV_3_B1_OFFSET                           0x20E4
#define DDRIO_ECC_VISA_LANECR0_TOP_OFFSET                       0x20E8
#define DDRIO_ECC_VISA_LANECR1_TOP_OFFSET                       0x20EC
#define DDRIO_ECC_VISA_CONTROLCR_TOP_OFFSET                     0x20F0
#define DDRIO_ECC_VISA_LANECR0_BL_OFFSET                        0x20F4
#define DDRIO_ECC_VISA_LANECR1_BL_OFFSET                        0x20F8
#define DDRIO_ECC_VISA_CONTROLCR_BL_OFFSET                      0x20FC
#define DDRIO_ECC_TIMING_CTRL_OFFSET                            0x210C
#define DDRIO_ECC_DQMOD_CTL_OFFSET                              0x2110
#define DDRIO_ECC_IOBONUSB0_OFFSET                              0x2114
#define DDRIO_ECC_IOBONUSB1_OFFSET                              0x2118
#define DDRIO_ECC_IOCTL1_OFFSET                                 0x211C
#define DDRIO_ECC_IOCTL2_OFFSET                                 0x2120
#define DDRIO_ECC_B0RXOFFSET2_OFFSET                            0x2124
#define DDRIO_ECC_B0RXOFFSET3_OFFSET                            0x2128
#define DDRIO_ECC_B1RXOFFSET2_OFFSET                            0x212C
#define DDRIO_ECC_B1RXOFFSET3_OFFSET                            0x2130
#define DDRIO_ECC_DLLCTL1_OFFSET                                0x2134
#define DDRIO_ECC_CLOCK_GATE2_OFFSET                            0x2138


#define DDRIO_CMD_OBSCKEBBCTL_OFFSET                            0x4800
#define DDRIO_CMD_RES0_OFFSET                                   0x4804
#define DDRIO_CMD_DLLTXCTL_OFFSET                               0x4808
#define DDRIO_CMD_DLLRXCTL_OFFSET                               0x480C
#define DDRIO_CMD_MDLLCTL_OFFSET                                0x4810
#define DDRIO_CMD_RCOMP_ODT_OFFSET                              0x4814
#define DDRIO_CMD_DLLPICODER0_OFFSET                            0x4820
#define DDRIO_CMD_DLLPICODER1_OFFSET                            0x4824
#define DDRIO_CMD_RES4_OFFSET                                   0x4828
#define DDRIO_CMD_RES5_OFFSET                                   0x482c
#define DDRIO_CMD_RES6_OFFSET                   0x4830
#define DDRIO_CMD_RES7_OFFSET                   0x4834
#define DDRIO_CMD_RES8_OFFSET                   0x4838
#define DDRIO_CMD_CFG_REG0_OFFSET               0x4840
#define DDRIO_CMD_POINTER_REG_OFFSET                            0x4844
#define DDRIO_CMD_RESERVED0_OFFSET              0x4848
#define DDRIO_CMD_RESERVED1_OFFSET              0x484C
#define DDRIO_CMD_CLKALIGN_REG0_OFFSET                          0x4850
#define DDRIO_CMD_CLKALIGN_REG1_OFFSET                          0x4854
#define DDRIO_CMD_CLKALIGN_REG2_OFFSET                          0x4858
#define DDRIO_CMD_PM_CONFIG0_OFFSET             0x485C
#define DDRIO_CMD_PM_DLYREG0_OFFSET             0x4860
#define DDRIO_CMD_PM_DLYREG1_OFFSET             0x4864
#define DDRIO_CMD_PM_DLYREG2_OFFSET             0x4868
#define DDRIO_CMD_PM_DLYREG3_OFFSET             0x486C
#define DDRIO_CMD_PM_DLYREG4_OFFSET             0x4870
#define DDRIO_CMD_CLKALIGN_STS0_OFFSET                          0x4874
#define DDRIO_CMD_CLKALIGN_STS1_OFFSET                          0x4878
#define DDRIO_CMD_PM_STS0_OFFSET                0x487C
#define DDRIO_CMD_PM_STS1_OFFSET                0x4880
#define DDRIO_CMD_COMPSLV_OFFSET                0x4884
#define DDRIO_CMD_VISA_OFFSET                   0x4888
#define DDRIO_CMD_BONUSCMD0_IO_OFFSET                           0x488C
#define DDRIO_CMD_BONUSCMD1_IO_OFFSET                           0x4890
#define DDRIO_CMD_VISA_LANESEL_OFFSET                           0x4894
#define DDRIO_CMD_VISA_PATGEN_OFFSET                            0x4898
#define DDRIO_CMD_VISA_CUSTR_BYPASS_OFFSET                      0x489C
#define DDRIO_CMD_CLKGATE_OFFSET                                0x48A0
#define DDRIO_CLKCTL_OBSCKEBBCTL_OFFSET                         0x5800
#define DDRIO_CLKCTL_RCOMP_IO_OFFSET                            0x5804
#define DDRIO_CLKCTL_DLLTXCTL_OFFSET                            0x5808
#define DDRIO_CLKCTL_DLLRXCTL_OFFSET                            0x580C
#define DDRIO_CLKCTL_MDLLCTL_OFFSET                             0x5810
#define DDRIO_CLKCTL_RCOMP_ODT_OFFSET                           0x5814
#define DDRIO_CLKCTL_RES2_OFFSET                                0x5818
#define DDRIO_CLKCTL_RES3_OFFSET                                0x581C
#define DDRIO_CLKCTL_DLLPICODER0_OFFSET                         0x5820
#define DDRIO_CLKCTL_DLLPICODER1_OFFSET                         0x5824
#define DDRIO_CLKCTL_RES4_OFFSET                                0x5828
#define DDRIO_CLKCTL_RES5_OFFSET                                0x582C
#define DDRIO_CLKCTL_DDR3RESETCTL_OFFSET                        0x5830
#define DDRIO_CLKCTL_RES7_OFFSET                                0x5834
#define DDRIO_CLKCTL_CFG_REG0_OFFSET                            0x5838
#define DDRIO_CLKCTL_CFG_REG1_OFFSET                            0x5840
#define DDRIO_CLKCTL_POINTER_REG_OFFSET                         0x5844
#define DDRIO_CLKCTL_RESERVED0_OFFSET                           0x5848
#define DDRIO_CLKCTL_RESERVED1_OFFSET                           0x584C
#define DDRIO_CLKCTL_CLKALIGN_REG0_OFFSET                       0x5850
#define DDRIO_CLKCTL_CLKALIGN_REG1_OFFSET                       0x5854
#define DDRIO_CLKCTL_CLKALIGN_REG2_OFFSET                       0x5858
#define DDRIO_CLKCTL_PM_CONFIG0_OFFSET                          0x585C
#define DDRIO_CLKCTL_PM_DLYREG0_OFFSET                          0x5860
#define DDRIO_CLKCTL_PM_DLYREG1_OFFSET                          0x5864
#define DDRIO_CLKCTL_PM_DLYREG2_OFFSET                          0x5868
#define DDRIO_CLKCTL_PM_DLYREG3_OFFSET                          0x586C
#define DDRIO_CLKCTL_PM_DLYREG4_OFFSET                          0x5870
#define DDRIO_CLKCTL_CLKALIGN_STS0_OFFSET                       0x5874
#define DDRIO_CLKCTL_CLKALIGN_STS1_OFFSET                       0x5878
#define DDRIO_CLKCTL_PM_STS0_OFFSET                             0x587C
#define DDRIO_CLKCTL_PM_STS1_OFFSET                             0x5880
#define DDRIO_CLKCTL_COMPSLV_1_OFFSET                           0x5884
#define DDRIO_CLKCTL_COMPSLV_2_OFFSET                           0x5888
#define DDRIO_CLKCTL_COMPSLV_3_OFFSET                           0x588C
#define DDRIO_CLKCTL_VISA_OFFSET                0x5890
#define DDRIO_CLKCTL_BONUSCTL_IO_OFFSET                         0x5894
#define DDRIO_CLKCTL_BONUSCLK_IO_OFFSET                         0x5898
#define DDRIO_CLKCTL_VISA_LANESEL_OFFSET                        0x589C
#define DDRIO_CLKCTL_VISA_PATGEN_OFFSET                         0x58A0
#define DDRIO_CLKCTL_VISA_CUSTR_BYPASS_OFFSET                   0x58A4
#define DDRIO_CLKCTL_CLKGATE_OFFSET             0x58A8
#define DDRIO_CLKCTL_CMD_PTRCTL_OFFSET                          0x58B4
#define DDRIO_CLKCTL_CMD_CFG_REG2_OFFSET                        0x58B8
#define DDRIO_PLL_MPLLCTRL_OFFSET               0x7800
#define DDRIO_PLL_MPLLCTRL1_OFFSET              0x7808
#define DDRIO_PLL_MPLLCSR0_OFFSET               0x7810
#define DDRIO_PLL_MPLLCSR1_OFFSET               0x7814
#define DDRIO_PLL_MPLLCSR2_OFFSET               0x7820
#define DDRIO_PLL_MPLLDFT_OFFSET                0x7828
#define DDRIO_PLL_MPLLMONCTL_OFFSET             0x7830
#define DDRIO_PLL_MPLLMON1CTL_OFFSET                            0x7838
#define DDRIO_PLL_MPLLMON2CTL_OFFSET                            0x783C
#define DDRIO_PLL_SFRTRIM_OFFSET                0x7850
#define DDRIO_PLL_MPLLDFTOUT_OFFSET             0x7858
#define DDRIO_PLL_MPLLDFTOUT1_OFFSET                            0x785C
#define DDRIO_PLL_RES1_OFFSET                   0x7860
#define DDRIO_PLL_RES2_OFFSET                   0x7864
#define DDRIO_PLL_RES3_OFFSET                   0x7868
#define DDRIO_PLL_RES4_OFFSET                   0x786C
#define DDRIO_PLL_RES5_OFFSET                   0x7870
#define DDRIO_PLL_RES6_OFFSET                   0x7874
#define DDRIO_PLL_RES7_OFFSET                   0x7878
#define DDRIO_PLL_RES8_OFFSET                   0x787C
#define DDRIO_PLL_MASTERRSTN_OFFSET             0x7880
#define DDRIO_PLL_PLLLOCKDEL_OFFSET             0x7884
#define DDRIO_PLL_SFRDEL_OFFSET                 0x7888
#define DDRIO_PLL_VISACTRL0_OFFSET              0x78F0
#define DDRIO_PLL_VISACTRL1_OFFSET              0x78F4
#define DDRIO_PLL_VISACTRL2_OFFSET              0x78F8
#define DDRIO_PLL_VISACTRL3_OFFSET              0x78FC
#define DDRIO_COMP_CMPCTRL_OFFSET               0x6800
#define DDRIO_COMP_MSCNTR_OFFSET                0x6808
#define DDRIO_COMP_NMSCNTRL_OFFSET              0x680C
#define DDRIO_COMP_LATCH1CTL_OFFSET             0x6814
#define DDRIO_COMP_VISACTRL_OFFSET              0x6820
#define DDRIO_COMP_VISAPATGEN_OFFSET                            0x6828
#define DDRIO_COMP_CMPBONUSREG_OFFSET                           0x6830
#define DDRIO_COMP_TCOCNTCTRL_OFFSET                            0x683C
#define DDRIO_COMP_DQANAODTPUCTL_OFFSET                         0x6840
#define DDRIO_COMP_DQANAODTPDCTL_OFFSET                         0x6844
#define DDRIO_COMP_DQANADRVPUCTL_OFFSET                         0x6848
#define DDRIO_COMP_DQANADRVPDCTL_OFFSET                         0x684C
#define DDRIO_COMP_DQANADLYPUCTL_OFFSET                         0x6850
#define DDRIO_COMP_DQANADLYPDCTL_OFFSET                         0x6854
#define DDRIO_COMP_DQANATCOPUCTL_OFFSET                         0x6858
#define DDRIO_COMP_DQANATCOPDCTL_OFFSET                         0x685C
#define DDRIO_COMP_CMDANADRVPUCTL_OFFSET                        0x6868
#define DDRIO_COMP_CMDANADRVPDCTL_OFFSET                        0x686C
#define DDRIO_COMP_CMDANADLYPUCTL_OFFSET                        0x6870
#define DDRIO_COMP_CMDANADLYPDCTL_OFFSET                        0x6874
#define DDRIO_COMP_CLKANAODTPUCTL_OFFSET                        0x6880
#define DDRIO_COMP_CLKANAODTPDCTL_OFFSET                        0x6884
#define DDRIO_COMP_CLKANADRVPUCTL_OFFSET                        0x6888
#define DDRIO_COMP_CLKANADRVPDCTL_OFFSET                        0x688C
#define DDRIO_COMP_CLKANADLYPUCTL_OFFSET                        0x6890
#define DDRIO_COMP_CLKANADLYPDCTL_OFFSET                        0x6894
#define DDRIO_COMP_CLKANATCOPUCTL_OFFSET                        0x6898
#define DDRIO_COMP_CLKANATCOPDCTL_OFFSET                        0x689C
#define DDRIO_COMP_DQSANAODTPUCTL_OFFSET                        0x68A0
#define DDRIO_COMP_DQSANAODTPDCTL_OFFSET                        0x68A4
#define DDRIO_COMP_DQSANADRVPUCTL_OFFSET                        0x68A8
#define DDRIO_COMP_DQSANADRVPDCTL_OFFSET                        0x68AC
#define DDRIO_COMP_DQSANADLYPUCTL_OFFSET                        0x68B0
#define DDRIO_COMP_DQSANADLYPDCTL_OFFSET                        0x68B4
#define DDRIO_COMP_DQSANATCOPUCTL_OFFSET                        0x68B8
#define DDRIO_COMP_DQSANATCOPDCTL_OFFSET                        0x68BC
#define DDRIO_COMP_CTLANADRVPUCTL_OFFSET                        0x68C8
#define DDRIO_COMP_CTLANADRVPDCTL_OFFSET                        0x68CC
#define DDRIO_COMP_CTLANADLYPUCTL_OFFSET                        0x68D0
#define DDRIO_COMP_CTLANADLYPDCTL_OFFSET                        0x68D4
#define DDRIO_COMP_CHNLBUFSTATIC_OFFSET                         0x68F0
#define DDRIO_COMP_COMPOBSCNTRL_OFFSET                          0x68F4
#define DDRIO_COMP_COMPBUFFDBG0_OFFSET                          0x68F8
#define DDRIO_COMP_COMPBUFFDBG1_OFFSET                          0x68FC
#define DDRIO_COMP2_CFGMISCCH0_OFFSET                           0x6900
#define DDRIO_COMP2_COMPEN0CH0_OFFSET                           0x6904
#define DDRIO_COMP2_COMPEN1CH0_OFFSET                           0x6908
#define DDRIO_COMP2_COMPEN2CH0_OFFSET                           0x690C
#define DDRIO_COMP2_STATLEGEN0CH0_OFFSET                        0x6910
#define DDRIO_COMP2_STATLEGEN1CH0_OFFSET                        0x6914
#define DDRIO_COMP2_DQVREFCH0_OFFSET                            0x6918
#define DDRIO_COMP2_CMDVREFCH0_OFFSET                           0x691C
#define DDRIO_COMP2_CLKVREFCH0_OFFSET                           0x6920
#define DDRIO_COMP2_DQSVREFCH0_OFFSET                           0x6924
#define DDRIO_COMP2_CTLVREFCH0_OFFSET                           0x6928
#define DDRIO_COMP2_TCOVREFCH0_OFFSET                           0x692C
#define DDRIO_COMP2_DLYSELCH0_OFFSET                            0x6930
#define DDRIO_COMP2_TCODRAMBUFODTCH0_OFFSET                     0x6934
#define DDRIO_COMP2_CCBUFODTCH0_OFFSET                          0x6938
#define DDRIO_COMP2_RXOFFSETCH0_OFFSET                          0x693C
#define DDRIO_COMP2_DQODTPUCTLCH0_OFFSET                        0x6940
#define DDRIO_COMP2_DQODTPDCTLCH0_OFFSET                        0x6944
#define DDRIO_COMP2_DQDRVPUCTLCH0_OFFSET                        0x6948
#define DDRIO_COMP2_DQDRVPDCTLCH0_OFFSET                        0x694C
#define DDRIO_COMP2_DQDLYPUCTLCH0_OFFSET                        0x6950
#define DDRIO_COMP2_DQDLYPDCTLCH0_OFFSET                        0x6954
#define DDRIO_COMP2_DQTCOPUCTLCH0_OFFSET                        0x6958
#define DDRIO_COMP2_DQTCOPDCTLCH0_OFFSET                        0x695C
#define DDRIO_COMP2_CMDDRVPUCTLCH0_OFFSET                       0x6968
#define DDRIO_COMP2_CMDDRVPDCTLCH0_OFFSET                       0x696C
#define DDRIO_COMP2_CMDDLYPUCTLCH0_OFFSET                       0x6970
#define DDRIO_COMP2_CMDDLYPDCTLCH0_OFFSET                       0x6974
#define DDRIO_COMP2_CLKODTPUCTLCH0_OFFSET                       0x6980
#define DDRIO_COMP2_CLKODTPDCTLCH0_OFFSET                       0x6984
#define DDRIO_COMP2_CLKDRVPUCTLCH0_OFFSET                       0x6988
#define DDRIO_COMP2_CLKDRVPDCTLCH0_OFFSET                       0x698C
#define DDRIO_COMP2_CLKDLYPUCTLCH0_OFFSET                       0x6990
#define DDRIO_COMP2_CLKDLYPDCTLCH0_OFFSET                       0x6994
#define DDRIO_COMP2_CLKTCOPUCTLCH0_OFFSET                       0x6998
#define DDRIO_COMP2_CLKTCOPDCTLCH0_OFFSET                       0x699C
#define DDRIO_COMP2_DQSODTPUCTLCH0_OFFSET                       0x69A0
#define DDRIO_COMP2_DQSODTPDCTLCH0_OFFSET                       0x69A4
#define DDRIO_COMP2_DQSDRVPUCTLCH0_OFFSET                       0x69A8
#define DDRIO_COMP2_DQSDRVPDCTLCH0_OFFSET                       0x69AC
#define DDRIO_COMP2_DQSDLYPUCTLCH0_OFFSET                       0x69B0
#define DDRIO_COMP2_DQSDLYPDCTLCH0_OFFSET                       0x69B4
#define DDRIO_COMP2_DQSTCOPUCTLCH0_OFFSET                       0x69B8
#define DDRIO_COMP2_DQSTCOPDCTLCH0_OFFSET                       0x69BC
#define DDRIO_COMP2_CTLDRVPUCTLCH0_OFFSET                       0x69C8
#define DDRIO_COMP2_CTLDRVPDCTLCH0_OFFSET                       0x69CC
#define DDRIO_COMP2_CTLDLYPUCTLCH0_OFFSET                       0x69D0
#define DDRIO_COMP2_CTLDLYPDCTLCH0_OFFSET                       0x69D4
#define DDRIO_COMP2_FNLUPDTCTLCH0_OFFSET                        0x69F0
#define DDRIO_COMP2_DQODTPUCTLCH1_OFFSET                        0x6A40
#define DDRIO_COMP2_DQODTPDCTLCH1_OFFSET                        0x6A44
#define DDRIO_COMP2_DQDRVPUCTLCH1_OFFSET                        0x6A48
#define DDRIO_COMP2_DQDRVPDCTLCH1_OFFSET                        0x6A4C
#define DDRIO_COMP2_DQDLYPUCTLCH1_OFFSET                        0x6A50
#define DDRIO_COMP2_DQDLYPDCTLCH1_OFFSET                        0x6A54
#define DDRIO_COMP2_DQTCOPUCTLCH1_OFFSET                        0x6A58
#define DDRIO_COMP2_DQTCOPDCTLCH1_OFFSET                        0x6A5C
#define DDRIO_COMP2_CMDDRVPUCTLCH1_OFFSET                       0x6A68
#define DDRIO_COMP2_CMDDRVPDCTLCH1_OFFSET                       0x6A6C
#define DDRIO_COMP2_CMDDLYPUCTLCH1_OFFSET                       0x6A70
#define DDRIO_COMP2_CMDDLYPDCTLCH1_OFFSET                       0x6A74
#define DDRIO_COMP2_CLKODTPUCTLCH1_OFFSET                       0x6A80
#define DDRIO_COMP2_CLKODTPDCTLCH1_OFFSET                       0x6A84
#define DDRIO_COMP2_CLKDRVPUCTLCH1_OFFSET                       0x6A88
#define DDRIO_COMP2_CLKDRVPDCTLCH1_OFFSET                       0x6A8C
#define DDRIO_COMP2_CLKDLYPUCTLCH1_OFFSET                       0x6A90
#define DDRIO_COMP2_CLKDLYPDCTLCH1_OFFSET                       0x6A94
#define DDRIO_COMP2_CLKTCOPUCTLCH1_OFFSET                       0x6A98
#define DDRIO_COMP2_CLKTCOPDCTLCH1_OFFSET                       0x6A9C
#define DDRIO_COMP2_DQSODTPUCTLCH1_OFFSET                       0x6AA0
#define DDRIO_COMP2_DQSODTPDCTLCH1_OFFSET                       0x6AA4
#define DDRIO_COMP2_DQSDRVPUCTLCH1_OFFSET                       0x6AA8
#define DDRIO_COMP2_DQSDRVPDCTLCH1_OFFSET                       0x6AAC
#define DDRIO_COMP2_DQSDLYPUCTLCH1_OFFSET                       0x6AB0
#define DDRIO_COMP2_DQSDLYPDCTLCH1_OFFSET                       0x6AB4
#define DDRIO_COMP2_DQSTCOPUCTLCH1_OFFSET                       0x6AB8
#define DDRIO_COMP2_DQSTCOPDCTLCH1_OFFSET                       0x6ABC
#define DDRIO_COMP2_CTLDRVPUCTLCH1_OFFSET                       0x6AC8
#define DDRIO_COMP2_CTLDRVPDCTLCH1_OFFSET                       0x6ACC
#define DDRIO_COMP2_CTLDLYPUCTLCH1_OFFSET                       0x6AD0
#define DDRIO_COMP2_CTLDLYPDCTLCH1_OFFSET                       0x6AD4


//VLV DUnit Register Symbol             Register Start
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
#define MC_DCBR_OFFSET                  0x12
#define MC_DSTAT_OFFSET                 0x20
#define MC_PGTBL_OFFSET                 0x21
#define MC_MISRCCCLR_OFFSET             0x31
#define MC_MISRDDCLR_OFFSET             0x32
#define MC_MISRCCSIG_OFFSET             0x34
#define MC_MISRDDSIG_OFFSET             0x35
#define MC_SSKPD0_OFFSET                0x4A
#define MC_SSKPD1_OFFSET                0x4B
#define MC_BONUS0_OFFSET                0x50
#define MC_BONUS1_OFFSET                0x51
#define MC_SCRMSEED_OFFSET              0x80
#define MC_SCRMLO_OFFSET                0x81
#define MC_SCRMHI_OFFSET                0x82
#define MC_SCR03_OFFSET                 0x83
#define MC_SCR04_OFFSET                 0x84
#define MC_SCR05_OFFSET                 0x85
#define MC_SCR06_OFFSET                 0x86
#define MC_SCR07_OFFSET                 0x87
#define MC_SCR08_OFFSET                 0x88
#define MC_SCR09_OFFSET                 0x89
#define MC_SCR10_OFFSET                 0x8A
#define MC_SCR11_OFFSET                 0x8B
#define MC_SCR12_OFFSET                 0x8C
#define MC_SCR13_OFFSET                 0x8D
#define MC_SCR14_OFFSET                 0x8E
#define MC_SCR15_OFFSET                 0x8F
#define MC_PMSEL0_OFFSET                0xE0
#define MC_PMSEL1_OFFSET                0xE1
#define MC_PMSEL2_OFFSET                0xE2
#define MC_PMSEL3_OFFSET                0xE3
#define MC_PMAUXMAX_OFFSET              0xE8
#define MC_PMAUXMIN_OFFSET              0xE9
#define MC_PMAUXSEL_OFFSET              0xEA

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
    UINT32 addressMap         :1;             /**< BIT [14]    Address Map select */
    UINT32 reserved3          :1;
    UINT32 dimmFlip           :1;
    UINT32 reserved4          :3;
    UINT32 dimm0Mirror        :1;
    UINT32 dimm1Mirror        :1;
    UINT32 DRAMtype           :1;               /**< BIT [22] */
    UINT32 reserved5          :9;
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
    UINT32 pmeDelay           :2;             /**< bit [25:24] Power mode entry delay  */
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
    UINT32 tCCD               :3;             /**< CAS to CAS delay */
    UINT32 reserved4          :1;
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
    UINT32 tWRSR              :4;             /**< WR to RD Same Rank. */
    UINT32 reserved4          :5;
    UINT32 tXP                :2;             /**< Time from CKE set on to any command. */
    UINT32 PWD_DLY            :4;             /**< Extended Power-Down Delay. */
    UINT32 EnDeRate           :1;
    UINT32 DeRateOvr          :1;
    UINT32 DeRateStat         :1;
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
    UINT32 powerModeOpCode    :5;             /**< SPID Power Mode Opcode */
    UINT32 reserved1          :3;
    UINT32 PCLSTO             :3;             /**< Page Close Timeout Period */
    UINT32 reserved2          :1;
    UINT32 PCLSWKOK           :1;             /**< Wake Allowed For Page Close Timeout */
    UINT32 PREAPWDEN          :1;             /**< Send Precharge All to rank before entering Power-Down mode. */
    UINT32 reserved3          :1;
    UINT32 DYNSREN            :1;             /**< Dynamic Self-Refresh */
    UINT32 CLKGTDIS           :1;             /**< Clock Gating Disabled*/
    UINT32 DISPWRDN           :1;             /**< Disable Power Down*/
    UINT32 BLMODE             :1;             /**< Selects the Burst Length mode*/
    UINT32 PWRGATEDIS         :1;
    UINT32 REUTCLKGTDIS       :1;
    UINT32 reserved4          :3;
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
    UINT32 CuRefRate          :3;
    UINT32 DisRefBW           :1;
    UINT32 reserved4          :4;
  } field;
} RegDRCF;                                           /**< DRAM Refresh Control Register*/
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
    UINT32 reserved3          :5;
    UINT32 MRRData            :8;               /**< bit[31:24] */
  } field;
} RegDCAL;                                          /**< DRAM Calibration Control*/
#pragma pack()

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
    UINT32 BLKRD              :1;             /**< block read */
    UINT32 BLKWR              :1;             /**< block write */
    UINT32 BLKACT             :1;             /**< block activate */
    UINT32 BLKPRE             :1;             /**< block precharge */
    UINT32 BLKIPRQNF          :1;             /**< Block IPREQ Until Full */
    UINT32 reserved1          :17;
  } field;
} RegDTCR;                                          /**< DRAM Training Control*/
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
    UINT32    command         :3;             /**< Command: 000-MRS,001-Refresh,010-Pre-charge,011-Activate,110 ¨C ZQ Calibration,111-NOP */
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

#define DRP_RKEN0               0           //Rank 0 Enabled
#define DRP_RKEN1               1           //Rank 1 Enabled
#define DRP_RKEN2               2           //Rank 2 Enabled
#define DRP_RKEN3               3           //Rank 3 Enabled
#define DRP_DIMMDWID0           4           //DIMM 0 Device Width (Rank0&1)
#define DRP_DIMMDDEN0           5           //DIMM 0 Device Density :2
#define DRP_DIMMDWID1           8           //DIMM 1 Device Width (Rank2&3)
#define DRP_DIMMDDEN1           9           //DIMM 1 Device Density:2
#define DRP_ADDRMAP             12          //Address Map Select
#define DTR0_DFREQ              0           //DRAM Frequency:2
#define DTR0_tRP                4           //Precharge to Activate Delay:3
#define DTR0_tRCD               8           //Activate to CAS Delay:3
#define DTR0_tCL                12          //CAS Latency:3
#define DTR1_tWCL               0           //CAS Write Latency:2
#define DTR1_tCMD               4           //Command transport duration:2
#define DTR1_tWR                8           //Write Recovery time:3
#define DTR1_tWTR               12          //Write Recovery time:2
#define DTR1_tFAW               16          //Four bank Activation Window:4
#define DTR1_tRAS               20          //Row Activation Period:4
#define DTR1_tRRD               24          //Row activation to Row activation Delay:2
#define DTR1_tRTP               28          //Read to Precharge Delay:2
#define DTR2_tRRDR              0           //RD to RD from different ranks, same DIMM:2
#define DTR2_tRRDD              2           //RD to RD from different DIMMs:2
#define DTR2_tWWDR              4           //WR to WR from different ranks:2
#define DTR2_tWWDD              6           //WR to WR from different DIMMs:2
#define DTR2_tRWDR              8           //RD to WR from different ranks, same DIMM:2
#define DTR2_tRWDD              10          //RD to WR from different DIMMs:2
#define DTR2_tWRDR              12          //WR to RD from different ranks, same DIMM:2
#define DTR2_tWRDD              14          //WR to RD from different DIMMs:2
#define DTR2_tRWMD              16          //RD data to WR data or WR data to RD data min delay:2
#define DTR2_tRWSR              18          //RD to WR from same ranks, same DIMM:2
#define DTR2_tXP                20          //Time from CKE set on to any command:2
#define DTR2_tXPDLL             24          //Time from CKE set on to data when DLL-off mode:4
#define DPMC1_CKEOWNER          0           //CKE OWNER
#define DPMC1_CKEVAL            4
#define DPMC1_CSTRIST           8
#define DPMC1_CMDTRIST          10
#define DPMC1_CLKGTDIS          16
#define DPMC1_SRXREFS           17
#define DPMC1_ODTDIS            18
#define DPMC1_MPLLBYPEN         20

#endif
