/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Valleyview          *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c)  1999 - 2014 Intel Corporation. All rights reserved   *;
;*    This software and associated documentation (if any) is furnished    *;
;*    under a license and may only be used or copied in accordance        *;
;*    with the terms of the license. Except as permitted by such          *;
;*    license, no part of this software or documentation may be           *;
;*    reproduced, stored in a retrieval system, or transmitted in any     *;
;*    form or by any means without the express written consent of         *;
;*    Intel Corporation.                                                  *;
;*                                                                        *;
;*                                                                        *;
;**************************************************************************/
/*++
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
--*/



// Define a Global region of ACPI NVS Region that may be used for any
// type of implementation.  The starting offset and size will be fixed
// up by the System BIOS during POST.  Note that the Size must be a word
// in size to be fixed up correctly.

OperationRegion(GNVS,SystemMemory,0xFFFF0000,0xAA55)
Field(GNVS,AnyAcc,Lock,Preserve)
{
  Offset(0),       // Miscellaneous Dynamic Registers:
  OSYS,   16,      //   (00) Operating System
  SMIF,   8,       //   (02) SMI Function Call (ASL to SMI via I/O Trap)
  PRM0,   8,       //   (03) SMIF - Parameter 0
  PRM1,   8,       //   (04) SMIF - Parameter 1
  SCIF,   8,       //   (05) SCI Function Call (SMI to ASL via _L00)
  PRM2,   8,       //   (06) SCIF - Parameter 0
  PRM3,   8,       //   (07) SCIF - Parameter 1
  LCKF,   8,       //   (08) Global Lock Function Call (EC Communication)
  PRM4,   8,       //   (09) LCKF - Parameter 0
  PRM5,   8,       //   (10) LCKF - Parameter 1
  P80D,   32,      //   (11) Port 80 Debug Port Value
  LIDS,   8,       //   (15) Lid State (Lid Open = 1)
  PWRS,   8,       //   (16) Power State (AC Mode = 1)
  DBGS,   8,       //   (17) Debug State
  Offset(18),      // Thermal Policy Registers:
  THOF,   8,       //   (18) Enable Thermal Offset for KSC
  ACT1,   8,       //   (19) Active Trip Point 1
  ACTT,   8,       //   (20) Active Trip Point
  PSVT,   8,       //   (21) Passive Trip Point
  TC1V,   8,       //   (22) Passive Trip Point TC1 Value
  TC2V,   8,       //   (23) Passive Trip Point TC2 Value
  TSPV,   8,       //   (24) Passive Trip Point TSP Value
  CRTT,   8,       //   (25) Critical Trip Point
  DTSE,   8,       //   (26) Digital Thermal Sensor Enable
  DTS1,   8,       //   (27) Digital Thermal Sensor 1 Reading
  DTS2,   8,       //   (28) Digital Thermal Sensor 2 Reading
  DTSF,   8,       //   (29) DTS SMI Function Call
  Offset(30),      // Battery Support Registers:
  BNUM,   8,       //   (30) Battery Number Present
  B0SC,   8,       //   (31) Battery 0 Stored Capacity
  B1SC,   8,       //   (32) Battery 1 Stored Capacity
  B2SC,   8,       //   (33) Battery 2 Stored Capacity
  B0SS,   8,       //   (34) Battery 0 Stored Status
  B1SS,   8,       //   (35) Battery 1 Stored Status
  B2SS,   8,       //   (36) Battery 2 Stored Status
  Offset(40),      // CPU Identification Registers:
  APIC,   8,       //   (40) APIC Enabled by SBIOS (APIC Enabled = 1)
  MPEN,   8,       //   (41) Number of Logical Processors if MP Enabled != 0
  PCP0,   8,       //   (42) PDC Settings, Processor 0
  PCP1,   8,       //   (43) PDC Settings, Processor 1
  PPCM,   8,       //   (44) Maximum PPC state
  PPMF,   32,      //   (45) PPM Flags (Same as CFGD)
  Offset(50),      // SIO CMOS Configuration Registers:
  NATP,   8,       //   (50)
  CMAP,   8,       //   (51) WINB COM A Port
  CMBP,   8,       //   (52) WINB COM B Port
  LPTP,   8,       //   (53) WINB LPT Port
  FDCP,   8,       //   (54) WINB FDC Port
  CMCP,   8,       //   (55) NPCE COM Port
  CIRP,   8,       //   (56)
  W381,   8,       //   (57)      W8374L
  NPCE,   8,       //   (58)      NPCE791x
  Offset(60),      // Internal Graphics Registers:
  IGDS,   8,       //   (60) IGD State (Primary Display = 1)
  TLST,   8,       //   (61) Display Toggle List Selection
  CADL,   8,       //   (62) Current Attached Device List
  PADL,   8,       //   (63) Previous Attached Device List
  CSTE,   16,      //   (64) Current Display State
  NSTE,   16,      //   (66) Next Display State
  SSTE,   16,      //   (68) Set Display State
  NDID,   8,       //   (70) Number of Valid Device IDs
  DID1,   32,      //   (71) Device ID 1
  DID2,   32,      //   (75) Device ID 2
  DID3,   32,      //   (79) Device ID 3
  DID4,   32,      //   (83) Device ID 4
  DID5,   32,      //   (87) Device ID 5
  KSV0,   32,      //   (91) First four bytes of AKSV (mannufacturing mode)
  KSV1,   8,       //   (95) Fifth byte of AKSV (mannufacturing mode)
  Offset(103), // Backlight Control Registers:
  BLCS,   8,       //   (103) Backlight Control Support
  BRTL,   8,       //   (104) Brightness Level Percentage
  Offset(105), // Ambiant Light Sensor Registers:
  ALSE,   8,       //   (105) ALS Enable
  ALAF,   8,       //   (106) Ambient Light Adjusment Factor
  LLOW,   8,       //   (107) LUX Low Value
  LHIH,   8,       //   (108) LUX High Value
  Offset(110), // EMA Registers:
  EMAE,   8,       //   (110) EMA Enable
  EMAP,   16,      //   (111) EMA Pointer
  EMAL,   16,      //   (113) EMA Length
  Offset(116), // MEF Registers:
  MEFE,   8,       //   (116) MEF Enable
  Offset(117), // PCIe Dock:
  DSTS,   8,       //   (117) PCIe Dock Status
  Offset(120), // TPM Registers:
  TPMP,   8,       //   (120) TPM Present - Obsolete since revision 1
  TPME,   8,       //   (121) TPM Enabled - Obsolete since revision 1
  MORD,   8,       //   (122) Memory Overwrite Request Data
  TCGP,   8,       //   (123) Used for save the Mor and/or physical presence paramter
  PPRP,   32,      //   (124) Physical Presence request operation response
  PPRQ,   8,       //   (125) Physical Presence request operation
  LPPR,   8,       //   (129) Last Physical Presence request operation
  Offset(130), //
  GTF0,   56,      //   (130) GTF Task File Buffer for Port 0
  GTF2,   56,      //   (137) GTF Task File Buffer for Port 2
  IDEM,   8,       //   (144) IDE Mode (Compatible\Enhanced)
  GTF1,   56,  //   (145) GTF Task File Buffer for Port 1
  Offset(170), // IGD OpRegion/Software SCI base address
  ASLB,   32,      //   (170) IGD OpRegion base address
  Offset(174), // IGD OpRegion/Software SCI shared data
  IBTT,   8,       //   (174) IGD Boot Display Device
  IPAT,   8,       //   (175) IGD Panel Type CMOs option
  ITVF,   8,       //   (176) IGD TV Format CMOS option
  ITVM,   8,       //   (177) IGD TV Minor Format CMOS option
  IPSC,   8,       //   (178) IGD Panel Scaling
  IBLC,   8,       //   (179) IGD BLC Configuration
  IBIA,   8,       //   (180) IGD BIA Configuration
  ISSC,   8,       //   (181) IGD SSC Configuration
  I409,   8,       //   (182) IGD 0409 Modified Settings Flag
  I509,   8,       //   (183) IGD 0509 Modified Settings Flag
  I609,   8,       //   (184) IGD 0609 Modified Settings Flag
  I709,   8,       //   (185) IGD 0709 Modified Settings Flag
  IDMM,   8,       //   (186) IGD DVMT Mode
  IDMS,   8,       //   (187) IGD DVMT Memory Size
  IF1E,   8,       //   (188) IGD Function 1 Enable
  HVCO,   8,       //   (189) HPLL VCO
  NXD1,   32,      //   (190) Next state DID1 for _DGS
  NXD2,   32,      //   (194) Next state DID2 for _DGS
  NXD3,   32,      //   (198) Next state DID3 for _DGS
  NXD4,   32,      //   (202) Next state DID4 for _DGS
  NXD5,   32,      //   (206) Next state DID5 for _DGS
  NXD6,   32,      //   (210) Next state DID6 for _DGS
  NXD7,   32,      //   (214) Next state DID7 for _DGS
  NXD8,   32,      //   (218) Next state DID8 for _DGS
  GSMI,   8,       //   (222) GMCH SMI/SCI mode (0=SCI)
  PAVP,   8,       //   (223) IGD PAVP data
  Offset(225),
  OSCC,   8,       //   (225) PCIE OSC Control
  NEXP,   8,       //   (226) Native PCIE Setup Value
  Offset(235), // Global Variables
  DSEN,   8,       //   (235) _DOS Display Support Flag.
  ECON,   8,       //   (236) Embedded Controller Availability Flag.
  GPIC,   8,       //   (237) Global IOAPIC/8259 Interrupt Mode Flag.
  CTYP,   8,       //   (238) Global Cooling Type Flag.
  L01C,   8,       //   (239) Global L01 Counter.
  VFN0,   8,       //   (240) Virtual Fan0 Status.
  VFN1,   8,       //   (241) Virtual Fan1 Status.
  Offset(256),
  NVGA,   32,      //   (256) NVIG opregion address
  NVHA,   32,      //   (260) NVHM opregion address
  AMDA,   32,      //   (264) AMDA opregion address
  DID6,   32,      //   (268) Device ID 6
  DID7,   32,      //   (272) Device ID 7
  DID8,   32,      //   (276) Device ID 8
  EPBA,	  32,	   //   (280)  EndpointBaseAddress;   PEG Endpoint PCIe Base Address
  CPSP,   32,      //   (284) PEG Endpoint Capability Structure Presence (Bit 0: Virtual Channel Capability)  
  EECB,   32,	   //   (288) EndpointPcieCapBaseAddress; PEG Endpoint PCIe Capability Structure Base Address
  EVCB,   32,      //   (292) EndpointVcCapBaseAddress;  PEG Endpoint Virtual Channel Capability Structure Base Address
  XBAS,   32,      //   (296) Any Device's PCIe Config Space Base Address
  OBS1,   32,      //   (300) Occupied Buses - from 0 to 31
  OBS2,   32,      //   (304) Occupied Buses - from 32 to 63
  OBS3,   32,      //   (308) Occupied Buses - from 64 to 95
  OBS4,   32,      //   (312) Occupied Buses - from 96 to 127
  OBS5,   32,      //   (316) Occupied Buses - from 128 to 159
  OBS6,   32,      //   (320) Occupied Buses - from 160 to 191
  OBS7,   32,      //   (324) Occupied Buses - from 192 to 223
  OBS8,   32,      //   (328) Occupied Buses - from 224 to 255
    
  Offset(332),
  USEL,   8,    // (332) UART Selection
  PU1E,   8,    // (333) PCU UART 1 Enabled
  PU2E,   8,    // (334) PCU UART 2 Enabled

  LPE0, 32,     // (335) LPE Bar0
  LPE1, 32,     // (339) LPE Bar1
  LPE2, 32,     // (343) LPE Bar2

  Offset(347),
  ACST,   8,       //   (347) For AC powered configuration option - IST
  BTST,   8,       //   (348) For Battery powered configuration option - IST
  PFLV,   8,       //   (349) Platform Flavor

  Offset(351),
  ICNF,   8,   //   (351) ISCT / AOAC Configuration
  XHCI,   8,   //   (352) xHCI controller mode
  PMEN,   8,   //   (353) PMIC enable/disable

  LPEE,   8,   //   (354) LPE enable/disable
  ISPA,   32,  //   (355) ISP Base Addr
  ISPD,   8,    //  (359) ISP Device Selection 0: Disabled; 1: PCI Device 2; 2: PCI Device 3

  offset(360),  // ((4+8+6)*4+2)*4=296
  //
  // Lpss controllers
  //
  PCIB,     32,
  PCIT,     32,
  D10A,     32,  //DMA1
  D10L,     32,
  D11A,     32,
  D11L,     32,
  P10A,     32,  //  PWM1
  P10L,     32,
  P11A,     32,
  P11L,     32,
  P20A,     32,  //  PWM2
  P20L,     32,
  P21A,     32,
  P21L,     32,
  U10A,     32,  // UART1
  U10L,     32,
  U11A,     32,
  U11L,     32,
  U20A,     32,  // UART2
  U20L,     32,
  U21A,     32,
  U21L,     32,
  SP0A,     32,  // SPI
  SP0L,     32,
  SP1A,     32,
  SP1L,     32,

  D20A,     32,  //DMA2
  D20L,     32,
  D21A,     32,
  D21L,     32,
  I10A,     32,  //  I2C1
  I10L,     32,
  I11A,     32,
  I11L,     32,
  I20A,     32,  //  I2C2
  I20L,     32,
  I21A,     32,
  I21L,     32,
  I30A,     32,  //  I2C3
  I30L,     32,
  I31A,     32,
  I31L,     32,
  I40A,     32,  //  I2C4
  I40L,     32,
  I41A,     32,
  I41L,     32,
  I50A,     32,  //  I2C5
  I50L,     32,
  I51A,     32,
  I51L,     32,
  I60A,     32,  //  I2C6
  I60L,     32,
  I61A,     32,
  I61L,     32,
  I70A,     32,  //  I2C7
  I70L,     32,
  I71A,     32,
  I71L,     32,
  //
  // Scc controllers
  //
  eM0A,     32,  //  EMMC
  eM0L,     32,
  eM1A,     32,
  eM1L,     32,
  SI0A,     32,  //  SDIO
  SI0L,     32,
  SI1A,     32,
  SI1L,     32,
  SD0A,     32,  //  SDCard
  SD0L,     32,
  SD1A,     32,
  SD1L,     32,
  MH0A,     32,  //
  MH0L,     32,
  MH1A,     32,
  MH1L,     32,

  offset(657),
  HLPS,     8,   //(657) Hide Devices
  offset(658),
  OSEL,     8,      //(658) OS Seletion - Windows/Android

  offset(659),  // VLV1 DPTF
  SDP1,     8,      //(659) An enumerated value corresponding to SKU
  DPTE,     8,      //(660) DPTF Enable
  THM0,     8,      //(661) System Thermal 0
  THM1,     8,      //(662) System Thermal 1
  THM2,     8,      //(663) System Thermal 2
  THM3,     8,      //(664) System Thermal 3
  THM4,     8,      //(665) System Thermal 3
  CHGR,     8,      //(666) DPTF Changer Device
  DDSP,     8,      //(667) DPTF Display Device
  DSOC,     8,      //(668) DPTF SoC device
  DPSR,     8,      //(669) DPTF Processor device
  DPCT,     32,     //(670) DPTF Processor participant critical temperature
  DPPT,     32,     //(674) DPTF Processor participant passive temperature
  DGC0,     32,     //(678) DPTF Generic sensor0 participant critical temperature
  DGP0,     32,     //(682) DPTF Generic sensor0 participant passive temperature
  DGC1,     32,     //(686) DPTF Generic sensor1 participant critical temperature
  DGP1,     32,     //(690) DPTF Generic sensor1 participant passive temperature
  DGC2,     32,     //(694) DPTF Generic sensor2 participant critical temperature
  DGP2,     32,     //(698) DPTF Generic sensor2 participant passive temperature
  DGC3,     32,     //(702) DPTF Generic sensor3 participant critical temperature
  DGP3,     32,     //(706) DPTF Generic sensor3 participant passive temperature
  DGC4,     32,     //(710)DPTF Generic sensor3 participant critical temperature
  DGP4,     32,     //(714)DPTF Generic sensor3 participant passive temperature
  DLPM,     8,      //(718) DPTF Current low power mode setting
  DSC0,     32,     //(719) DPTF Critical threshold0 for SCU
  DSC1,     32,     //(723) DPTF Critical threshold1 for SCU
  DSC2,     32,     //(727) DPTF Critical threshold2 for SCU
  DSC3,     32,     //(731) DPTF Critical threshold3 for SCU
  DSC4,     32,     //(735) DPTF Critical threshold3 for SCU
  DDBG,     8,      //(739) DPTF Super Debug option. 0 - Disabled, 1 - Enabled
  LPOE,     32,     //(740) DPTF LPO Enable
  LPPS,     32,     //(744) P-State start index
  LPST,     32,     //(748) Step size
  LPPC,     32,     //(752) Power control setting
  LPPF,     32,     //(756) Performance control setting
  DPME,     8,      //(760) DPTF DPPM enable/disable
  BCSL,     8,      //(761) Battery charging solution 0-CLV 1-ULPMC
  NFCS,     8,      //(762) NFCx Select 1: NFC1    2:NFC2
  PCIM,     8,      //(763) EMMC device 0-ACPI mode, 1-PCI mode
  TPMA,     32,     //(764)
  TPML,     32,     //(768)
  ITSA,      8,     //(772) I2C Touch Screen Address
  S0IX,     8,      //(773) S0ix status
  SDMD,     8,      //(774) SDIO Mode
  EMVR,     8,      //(775) eMMC controller version
  BMBD,     32,     //(776) BM Bound
  FSAS,     8,      //(780) FSA Status
  BDID,     8,      //(781) Board ID
  FBID,     8,      //(782) FAB ID
  OTGM,     8,      //(783) OTG mode
  STEP,     8,      //(784) Stepping ID
  WITT,     8,      //(785) Enable Test Device connected to I2C for WHCK test.
  SOCS,     8,      //(786) provide the SoC stepping infomation
  AMTE,     8,      //(787) Ambient Trip point change
  UTS,      8,      //(788) Enable Test Device connected to URT for WHCK test.
  SCPE,     8,      //(789) Allow higher performance on AC/USB - Enable/Disable
  Offset(792),
  EDPV,     8,      //(792) Check for eDP display device 
  DIDX,     32,     //(793) Device ID for eDP device

  //
  // Hybrid Graphics (HG) Support
  //
  Offset(797),    
  SGMD, 8,      // (797) SG Mode (0=Disabled, 1=SG Muxed, 2=SG Muxless, 3=DGPU Only)
  SGFL, 8,      // (798) SG Feature List
  PWOK, 8,      // (799) dGPU PWROK GPIO assigned
  HLRS, 8,      // (800) dGPU HLD RST GPIO assigned
  PWEN, 8,      // (801) dGPU PWR Enable GPIO assigned
  PRST, 8,      // (802) dGPU Present Detect GPIO assigned
  EECP, 8,      // (803) PEG Endpoint PCIe Capability Structure Offset
  EVCP, 16,     // (804) PEG Endpoint Virtual Channel Capability Structure Offset
  IOBA, 32,     // (806) IO Base Address
  SGGP, 8,      // (810) SG GPIO Support
  NXDX, 32,     // (811) Next state DID for eDP
  PCSL, 8,      // (815) The lowest C-state for the package
  SC7A, 8,      // (816) Run-time C7 Allowed feature (0=Disabled, 1=Enabled)
  NFCE, 8,      // (817) NFC enable/disable
  TPDE, 8,      // (818) Touch Pad enable/disable
  RPBA, 32,     // (819 - 822) PCH Root Port Base Address
  EPA1, 32,     // (823 - 826) PCH End Point Base Address
  PB1E, 8,      // (827) Virtual Power Button Support 
                          //   Bit0: Enable/Disable Virtual Button (0 - Disable; 1- Enable)
                          //   Bit1: Internal Flag
                          //   Bit2: Rotation Lock flag, 0:unlock, 1:lock
                          //   Bit3: Slate/Laptop Mode Flag, 0: Slate, 1: Laptop / CalmShell
                          //   Bit4: Undock / Dock Flag, 0: Undock, 1: Dock
                          //   Bit5, 6: reserved for future use.
                          //   Bit7: EC 10sec PB Override state for S3/S4 wake up. 
  MIPL, 16,     // (828) PPCC Minimum Power Limit
  MAPL, 16,     // (830) PPCC Maximum Power Limit
  PPSZ, 16,     // (832) PPCC power step size
}

