/*++
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
--*/
/*++

Copyright (c) 2012 - 2014 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  GlobalNvsArea.h

Abstract:

  Definition of the global NVS area protocol.  This protocol
  publishes the address and format of a global ACPI NVS buffer used as a communications
  buffer between SMM code and ASL code.
  The format is derived from the ACPI reference code, version 0.95.

  Note:  Data structures defined in this protocol are not naturally aligned.

--*/


#ifndef _GLOBAL_NVS_AREA_H_
#define _GLOBAL_NVS_AREA_H_

//
// Includes
//
#define GLOBAL_NVS_DEVICE_ENABLE 1
#define GLOBAL_NVS_DEVICE_DISABLE 0

//
// Forward reference for pure ANSI compatibility
//

//EFI_FORWARD_DECLARATION (EFI_GLOBAL_NVS_AREA_PROTOCOL);

//
// Global NVS Area Protocol GUID
//
#define EFI_GLOBAL_NVS_AREA_PROTOCOL_GUID \
{ 0x74e1e48, 0x8132, 0x47a1, 0x8c, 0x2c, 0x3f, 0x14, 0xad, 0x9a, 0x66, 0xdc }
//
// Revision id - Added TPM related fields
//
#define GLOBAL_NVS_AREA_RIVISION_1       1
//
// Extern the GUID for protocol users.
//
extern EFI_GUID gEfiGlobalNvsAreaProtocolGuid;

//
// Global NVS Area definition
//
#pragma pack (1)
typedef struct {
  //
  // Miscellaneous Dynamic Values, the definitions below need to be matched
  // GNVS definitions in Platform.ASL
  //
  UINT16      OperatingSystem;    // 00
  UINT8       SmiFunction;        // 02   SMI function call via IO Trap
  UINT8       SmiParameter0;      // 03
  UINT8       SmiParameter1;      // 04
  UINT8       SciFunction;        // 05   SCI function call via _L00
  UINT8       SciParameter0;      // 06
  UINT8       SciParameter1;      // 07
  UINT8       GlobalLock;         // 08   Global lock function call
  UINT8       LockParameter0;     // 09
  UINT8       LockParameter1;     // 10
  UINT32      Port80DebugValue;   // 11
  UINT8       LidState;           // 15   Open = 1
  UINT8       PowerState;         // 16   AC = 1
  UINT8       DebugState;         // 17


  //
  // Thermal Policy Values
  //
  UINT8       EnableThermalOffset;                        // 18 ThermalOffset for KSC
  UINT8       Reserved1;                 				  // 19
  UINT8       Reserved2;               					  // 20
  UINT8       PassiveThermalTripPoint;                    // 21
  UINT8       PassiveTc1Value;                            // 22
  UINT8       PassiveTc2Value;                            // 23
  UINT8       PassiveTspValue;                            // 24
  UINT8       CriticalThermalTripPoint;                   // 25
  UINT8       EnableDigitalThermalSensor;                 // 26   DTS Function enable
  UINT8       BspDigitalThermalSensorTemperature;         // 27   Temperature of BSP
  UINT8       ApDigitalThermalSensorTemperature;          // 28   Temperature of AP
  UINT8       DigitalThermalSensorSmiFunction;            // 29   SMI function call via DTS IO Trap

  //
  // Battery Support Values
  //
  UINT8       NumberOfBatteries;      // 30
  UINT8       BatteryCapacity0;       // 31   Battery 0 Stored Capacity
  UINT8       BatteryCapacity1;       // 32   Battery 1 Stored Capacity
  UINT8       BatteryCapacity2;       // 33   Battery 2 Stored Capacity
  UINT8       BatteryStatus0;         // 34   Battery 0 Stored Status
  UINT8       BatteryStatus1;         // 35   Battery 1 Stored Status
  UINT8       BatteryStatus2;         // 36   Battery 2 Stored Status

  // NOTE: Do NOT Change the Offset of Revision Field
  UINT8       Revision;               // 37   Revision of the structure EFI_GLOBAL_NVS_AREA
  UINT8       Reserved3[2];           // 38:39

  //
  // Processor Configuration Values
  //
  UINT8       ApicEnable;             // 40   APIC Enabled by SBIOS (APIC Enabled = 1)
  UINT8       LogicalProcessorCount;  // 41   Processor Count Enabled (MP Enabled != 0)
  UINT8       CurentPdcState0;        // 42   PDC settings, Processor 0
  UINT8       CurentPdcState1;        // 43   PDC settings, Processor 1
  UINT8       MaximumPpcState;        // 44   Maximum PPC state
  UINT32      PpmFlags;               // 45:48 PPM configuration flags, same as CFGD
  UINT8       Reserved4[1];           // 49

  //
  // SIO Configuration Values
  //
  UINT8       DockedSioPresent;       // 50   Dock SIO Present
  UINT8       DockComA;               // 51     COM A Port
  UINT8       DockComB;               // 52     COM B Port
  UINT8       LptP;                   // 53     LPT Port
  UINT8       DockFdc;                // 54     FDC Port
  UINT8       OnboardCom;             // 55   Onboard COM Port
  UINT8       OnboardComCir;          // 56   Onboard COM CIR Port

  UINT8       WPCN381U;           	  // 57
  UINT8       NPCE791x;               // 58
  UINT8       Reserved5[1];           // 59

  //
  // Internal Graphics Device Values
  //
  UINT8       IgdState;               // 60   IGD State (Primary Display = 1)
  UINT8       DisplayToggleList;      // 61   Display Toggle List Selection
  UINT8       CurrentDeviceList;      // 62   Current Attached Device List
  UINT8       PreviousDeviceList;     // 63   Previous Attached Device List
  UINT16      CurrentDisplayState;    // 64   Current Display State
  UINT16      NextDisplayState;       // 66   Next Display State
  UINT16      SetDisplayState;        // 68   Set Display State
  UINT8       NumberOfValidDeviceId;  // 70   Number of Valid Device IDs
  UINT32      DeviceId1;              // 71   Device ID 1
  UINT32      DeviceId2;              // 75   Device ID 2
  UINT32      DeviceId3;              // 79   Device ID 3
  UINT32      DeviceId4;              // 83   Device ID 4
  UINT32      DeviceId5;              // 87   Device ID 5

  UINT32      AKsv0;                  // 91:94 First four bytes of AKSV (manufacturing mode)
  UINT8       AKsv1;                  // 95    Fifth byte of AKSV (manufacturing mode

  UINT8       Reserved6[7];           // 96:102

  //
  // Backlight Control Values
  //
  UINT8       BacklightControlSupport;  // 103  Backlight Control Support
  UINT8       BrightnessPercentage;     // 104  Brightness Level Percentage

  //
  // Ambient Light Sensor Values
  //
  UINT8       AlsEnable;              // 105  Ambient Light Sensor Enable
  UINT8       AlsAdjustmentFactor;    // 106  Ambient Light Adjusment Factor
  UINT8       LuxLowValue;            // 107  LUX Low Value
  UINT8       LuxHighValue;           // 108  LUX High Value

  UINT8       Reserved7[1];           // 109

  //
  // Extended Mobile Access Values
  //
  UINT8       EmaEnable;              // 110  EMA Enable
  UINT16      EmaPointer;             // 111  EMA Pointer
  UINT16      EmaLength;              // 113  EMA Length

  UINT8       Reserved8[1];           // 115

  //
  // Mobile East Fork Values
  //
  UINT8       MefEnable;              // 116 Mobile East Fork Enable

  //
  // PCIe Dock Status
  //
  UINT8       PcieDockStatus;         // 117 PCIe Dock Status

  UINT8       Reserved9[2];           // 118:119

  //
  // TPM Registers
  //
  UINT8       TpmPresent;             // 120 TPM Present
  UINT8       TpmEnable;              // 121 TPM Enable

  UINT8       MorData;                // 122 Memory Overwrite Request Data
  UINT8       TcgParamter;            // 123 Used for save the Mor and/or physical presence paramter
  UINT32      PPResponse;             // 124 Physical Presence request operation response
  UINT8       PPRequest;              // 128 Physical Presence request operation
  UINT8       LastPPRequest;          // 129 Last Physical Presence request operation

  //
  // SATA Values
  //
  UINT8       GtfTaskFileBufferPort0[7];    // 130  GTF Task File Buffer for Port 0
  UINT8       GtfTaskFileBufferPort2[7];    // 137  GTF Task File Buffer for Port 2
  UINT8       IdeMode;                      // 144  IDE Mode (Compatible\Enhanced)
  UINT8       GtfTaskFileBufferPort1[7];    // 145:151 GTF Task File Buffer for Port 1

  UINT8       Reserved11[10];                // 152:161
  UINT64      BootTimeLogAddress;           // 162:169 Boot Time Log Table Address

  UINT32      IgdOpRegionAddress;           // 170  IGD OpRegion Starting Address
  UINT8       IgdBootType;                  // 174  IGD Boot Type CMOS option
  UINT8       IgdPanelType;                 // 175  IGD Panel Type CMOs option
  UINT8       IgdTvFormat;                  // 176  IGD TV Format CMOS option
  UINT8       IgdTvMinor;                   // 177  IGD TV Minor Format CMOS option
  UINT8       IgdPanelScaling;              // 178  IGD Panel Scaling
  UINT8       IgdBlcConfig;                 // 179  IGD BLC Configuration
  UINT8       IgdBiaConfig;                 // 180  IGD BIA Configuration
  UINT8       IgdSscConfig;                 // 181  IGD SSC Configuration
  UINT8       Igd409;                       // 182  IGD 0409 Modified Settings Flag
  UINT8       Igd509;                       // 183  IGD 0509 Modified Settings Flag
  UINT8       Igd609;                       // 184  IGD 0609 Modified Settings Flag
  UINT8       Igd709;                       // 185  IGD 0709 Modified Settings Flag
  UINT8       IgdPowerConservation;         // 186  IGD Power Conservation Feature Flag
  UINT8       IgdDvmtMemSize;               // 187  IGD DVMT Memory Size
  UINT8       IgdFunc1Enable;               // 188  IGD Function 1 Enable
  UINT8       IgdHpllVco;                   // 189  HPLL VCO
  UINT32      NextStateDid1;                // 190  Next state DID1 for _DGS
  UINT32      NextStateDid2;                // 194  Next state DID2 for _DGS
  UINT32      NextStateDid3;                // 198  Next state DID3 for _DGS
  UINT32      NextStateDid4;                // 202  Next state DID4 for _DGS
  UINT32      NextStateDid5;                // 206  Next state DID5 for _DGS
  UINT32      NextStateDid6;                // 210  Next state DID6 for _DGS
  UINT32      NextStateDid7;                // 214  Next state DID7 for _DGS
  UINT32      NextStateDid8;                // 218  Next state DID8 for _DGS
  UINT8       IgdSciSmiMode;                // 222  GMCH SMI/SCI mode (0=SCI)
  UINT8       IgdPAVP;                      // 223  IGD PAVP data
  UINT8       IgdSelfRefresh;               // 224  IGD Self Refresh
  UINT8       PcieOSCControl;               // 225  PCIE OSC Control
  UINT8       NativePCIESupport;            // 226  Native PCI Express Support

  //
  // USB Sideband Deferring Support
  //
  UINT8       HostAlertVector;              // 227  GPE vector used for HOST_ALERT
  UINT8       HostAlertPio;                 // 228  PIO of USB device used for HOST_ALERT

  UINT8       Reserved12[27];               // 229
  UINT32      NvIgOpRegionAddress;          // 256 NVIG support
  UINT32      NvHmOpRegionAddress;          // 260 NVHM support
  UINT32      ApXmOpRegionAddress;          // 264 AMDA support
  UINT32      DeviceId6;                    // 268   Device ID 6
  UINT32      DeviceId7;                    // 272   Device ID 7
  UINT32      DeviceId8;                    // 276   Device ID 8
  UINT32      EndpointBaseAddress;          // 280 PEG Endpoint PCIe Base Address
  UINT32      CapStrPresence;               // 284 PEG Endpoint Capability Structure Presence
  UINT32      EndpointPcieCapBaseAddress;   // 288 PEG Endpoint PCIe Capability Structure Base Address
  UINT32      EndpointVcCapBaseAddress;     // 292 PEG Endpoint Virtual Channel Capability Structure Base Address
  UINT32      XPcieCfgBaseAddress;          // 296 Any Device's PCIe Config Space Base Address
  UINT32      OccupiedBuses1;               // 300 Occupied Buses from 0 to 31
  UINT32      OccupiedBuses2;               // 304 Occupied Buses from 32 to 63
  UINT32      OccupiedBuses3;               // 308 Occupied Buses from 64 to 95
  UINT32      OccupiedBuses4;               // 312 Occupied Buses from 96 to 127
  UINT32      OccupiedBuses5;               // 316 Occupied Buses from 128 to 159
  UINT32      OccupiedBuses6;               // 320 Occupied Buses from 160 to 191
  UINT32      OccupiedBuses7;               // 324 Occupied Buses from 192 to 223
  UINT32      OccupiedBuses8;               // 328 Occupied Buses from 224 to 255
  UINT8       UartSelection;                // 332 UART Interface Selection 0: Internal; 1: SIO
  UINT8       PcuUart1Enable;               // 333 PCU UART 1 Enabled
  UINT8       PcuUart2Enable;               // 334 PCU UART 2 Enabled

  UINT32      LPEBar0;                      // 335~338  LPE Bar0
  UINT32      LPEBar1;                      // 339~342  LPE Bar1

  UINT32      LPEBar2;                      // 343~346  LPE Bar2
  UINT8		  AcSetup;                      // 347 For Ac Powered Config option - IST applet
  UINT8       BatterySetup;                 // 348 For Battery Powered Config option - IST applet
  UINT8       PlatformFlavor;               // 349 0:unknown 1: Mobile; 2: desktop
  UINT8       Reserved13[1];                // 350

  UINT8       IsctCfg;                      // 351 ISCT / AOAC Configuration
  UINT8       XhciMode;                     // 352 xHCI mode
  UINT8       PmicEnable;                   // 353 PMIC enable

  UINT8       LpeEnable;                    // 354 LPE enable
  UINT32      ISPAddr;                      // 355 ISP Base address 
  UINT8       ISPDevSel;                    // 359 ISP device enabled selection 0: Disabled; 1: PCI Device 2; 2: PCI Device 3

  //
  // Lpss controllers 
  //
  UINT32      PCIBottomAddress;            //360  ((4+8+6)*4+2)*4=296   
  UINT32      PCITopAddress;               //364 

  UINT32      LDMA1Addr;                   // 368
  UINT32      LDMA1Len;                    // 372
  UINT32      LDMA11Addr;                  // 376 
  UINT32      LDMA11Len;                   // 380  
  UINT32      PWM1Addr;                    // 384 PWM1
  UINT32      PWM1Len;                     // 388
  UINT32      PWM11Addr;                   // 392
  UINT32      PWM11Len;                    // 396
  UINT32      PWM2Addr;                    // 400 PWM2
  UINT32      PWM2Len;                     // 404
  UINT32      PWM21Addr;                   // 408
  UINT32      PWM21Len;                    // 412
  UINT32      UART1Addr;                   // 416 UART1
  UINT32      UART1Len;                    // 420
  UINT32      UART11Addr;                  // 424 UART1
  UINT32      UART11Len;                   // 428
  UINT32      UART2Addr;                   // 432 UART2
  UINT32      UART2Len;                    // 436
  UINT32      UART21Addr;                  // 440 UART2
  UINT32      UART21Len;                   // 444  
  UINT32      SPIAddr;                     // 448 SPI
  UINT32      SPILen;                      // 452
  UINT32      SPI1Addr;                    // 456 
  UINT32      SPI1Len;                     // 460

  UINT32      LDMA2Addr;                   // 464
  UINT32      LDMA2Len;                    // 468
  UINT32      LDMA21Addr;                  // 472 
  UINT32      LDMA21Len;                   // 476
  UINT32      I2C1Addr;                    // 480 I2C1
  UINT32      I2C1Len;                     // 484
  UINT32      I2C11Addr;                   // 488 I2C1
  UINT32      I2C11Len;                    // 492
  UINT32      I2C2Addr;                    // 496 I2C2
  UINT32      I2C2Len;                     // 500
  UINT32      I2C21Addr;                   // 504 I2C2
  UINT32      I2C21Len;                    // 508
  UINT32      I2C3Addr;                    // 512 I2C3
  UINT32      I2C3Len;                     // 516
  UINT32      I2C31Addr;                   // 520 I2C3
  UINT32      I2C31Len;                    // 524 
  UINT32      I2C4Addr;                    // 528 I2C4
  UINT32      I2C4Len;                     // 532
  UINT32      I2C41Addr;                   // 536 I2C4
  UINT32      I2C41Len;                    // 540
  UINT32      I2C5Addr;                    // 544 I2C5
  UINT32      I2C5Len;                     // 548
  UINT32      I2C51Addr;                   // 552 I2C5
  UINT32      I2C51Len;                    // 556
  UINT32      I2C6Addr;                    // 560 I2C6
  UINT32      I2C6Len;                     // 564
  UINT32      I2C61Addr;                   // 566 I2C6
  UINT32      I2C61Len;                    // 570
  UINT32      I2C7Addr;                    // 574 I2C7
  UINT32      I2C7Len;                     // 578
  UINT32      I2C71Addr;                   // 582 I2C7
  UINT32      I2C71Len;                    // 586 
  //
  // Scc controllers 
  //
  UINT32      eMMCAddr;                    // 590 EMMC
  UINT32      eMMCLen;                     // 594
  UINT32      eMMC1Addr;                   // 598
  UINT32      eMMC1Len;                    // 602
  UINT32      SDIOAddr;                    // 606 SDIO
  UINT32      SDIOLen;                     // 610
  UINT32      SDIO1Addr;                   // 614
  UINT32      SDIO1Len;                    // 618
  UINT32      SDCardAddr;                  // 622 SDCard 
  UINT32      SDCardLen;                   // 626
  UINT32      SDCard1Addr;                 // 630
  UINT32      SDCard1Len;                  // 636
  UINT32	  MipiHsiAddr;				   // 640 MIPI-HSI 
  UINT32	  MipiHsiLen;				   // 644
  UINT32	  MipiHsi1Addr;				   // 648
  UINT32	  MipiHsi1Len;				   // 652

  UINT8       Reserved16[1];                     // 656 reserve offset upto 658
  UINT8       HideLPSSDevices;                   // 657 Hide unsupported LPSS devices when in ACPI mode
  UINT8       OsSelection;                       // 658 OS Selection
  UINT8       SdpProfile;                        // 659  An enumeration corresponding to SKU.
  UINT8       DptfEnable;                        // 660  DPTF Enable/Disable
  UINT8       DptfSysThermal0;                   // 661  Generic Thermal Sensor 0
  UINT8       DptfSysThermal1;                   // 662  Generic Thermal Sensor 1
  UINT8       DptfSysThermal2;                   // 663  Generic Thermal Sensor 2
  UINT8       DptfSysThermal3;                   // 664  Generic Thermal Sensor 3
  UINT8       DptfSysThermal4;                   // 665  Generic Thermal Sensor 4    
  UINT8       DptfCharger;                       // 666  DPTF Changer Device
  UINT8       DptfDisplayDevice;                 // 667  DPTF Display Device
  UINT8       DptfSocDevice;                     // 668  DPTF SoC Device
  UINT8       DptfProcessor;                     // 669  DPTF Processor Device
  UINT32      DptfProcCriticalTemperature;       // 670 DPTF Processor participant critical temperature
  UINT32      DptfProcPassiveTemperature;        // 674 DPTF Processor participant passive temperature
  UINT32      DptfGenericCriticalTemperature0;   // 678 DPTF Generic sensor0 participant critical temperature
  UINT32      DptfGenericPassiveTemperature0;    // 682 DPTF Generic sensor0 participant passive temperature
  UINT32      DptfGenericCriticalTemperature1;   // 686 DPTF Generic sensor1 participant critical temperature
  UINT32      DptfGenericPassiveTemperature1;    // 690 DPTF Generic sensor1 participant passive temperature
  UINT32      DptfGenericCriticalTemperature2;   // 694 DPTF Generic sensor2 participant critical temperature
  UINT32      DptfGenericPassiveTemperature2;    // 698 DPTF Generic sensor2 participant passive temperature
  UINT32      DptfGenericCriticalTemperature3;   // 702 DPTF Generic sensor3 participant critical temperature
  UINT32      DptfGenericPassiveTemperature3;    // 706 DPTF Generic sensor3 participant passive temperature
  UINT32      DptfGenericCriticalTemperature4;   // 710 DPTF Generic sensor4 participant critical temperature
  UINT32      DptfGenericPassiveTemperature4;    // 714 DPTF Generic sensor4 participant passive temperature    
  UINT8       CLpmSetting;                       // 718 DPTF current low power mode setting
  UINT32      DptfCriticalThreshold0;            // 719 Critical Raw Temperature threshold0 for SCU
  UINT32      DptfCriticalThreshold1;            // 723 Critical Raw Temperature threshold1 for SCU
  UINT32      DptfCriticalThreshold2;            // 727 Critical Raw Temperature threshold2 for SCU
  UINT32      DptfCriticalThreshold3;            // 731 Critical Raw Temperature threshold3 for SCU
  UINT32      DptfCriticalThreshold4;            // 735 Critical Raw Temperature threshold4 for SCU    
  UINT8       DptfSuperDbg;                      // 739 DPTF Super Debug option. 0 - Disabled, 1 - Enabled
  UINT32      LPOEnable;                         // 740 DPTF: Instructs the policy to use Active Cores if they are available. 
  UINT32      LPOStartPState;                    // 744 DPTF: Instructs the policy when to initiate Active Core control if enabled. Returns P state index.
  UINT32      LPOStepSize;                       // 748 DPTF: Instructs the policy to take away logical processors in the specified percentage steps
  UINT32      LPOPowerControlSetting;            // 752 DPTF: 
  UINT32      LPOPerformanceControlSetting;      // 756 DPTF:  
  UINT8       DppmEnabled;                       // 760 DPTF: DPPM enable/disable
  UINT8       BatteryChargingSolution;           // 761 0-non ULPMC 1-ULPMC
  //101 bytes
  UINT8       NFCnSelect;                        // 762 NFCx Select 1: NFC1    2:NFC2
  UINT8       LpssSccMode;                       // 763 EMMC device 0-ACPI mode, 1-PCI mode

  UINT32      TPMAddress;                        // 764  
  UINT32      TPMLength;                         // 768

  UINT8       I2CTouchAddress;                   // 772 I2C touch address, 0x4B:RVP   0x4A:FFRD     
  UINT8       S0ix;                              // 773  0 - disabled 1 - enabled
  UINT8       SDIOMode;                          // 774  3 - Default  2 - DDR50
  UINT8       emmcVersion;                       // 775  0 - 4.41 1 - 4.5
  UINT32      BmBound;                           // 776 BM Bound
  UINT8       FsaStatus;                         // 780 0 - Fsa is off, 1- Fsa is on
  //
  // Board Id
  // This field is for the ASL code to know whether this board is Baylake or Bayley Bay etc
  //
  UINT8       BoardID;                           //  781
  UINT8       FabID;                             // 782
  UINT8       OtgMode;                           // 783 0- OTG disable 1- OTG PCI mode
  UINT8       Stepping;                          //  784 Stepping
  UINT8       WittEnable;                        // 785 WITT eanble 0 - disable 1 - enable 

  UINT8       SocStepping;                       // 786 Soc Stepping infomation
  UINT8       AmbientTripPointChange;            // 787 DPTF: Controls whether _ATI changes other participant's trip point(enabled/disabled)
  UINT8       UtsEnable;                         // 788 Uart Test eanble 0 - disable 1 - enable 
  UINT8 	  DptfAllowHigherPerformance;		 // 789  DPTF: Allow higher performance on AC/USB - (Enable/Disable)

  UINT8      SarEnable;                          // 790
  UINT8       PssDeveice;                        // 791 PSS Deveice: 0 - None, 1 - Monzax 2K, 2 - Monzax 8K 
  UINT8       EDPV;                              // 792 Check for eDP display device 
  UINT32      DIDX;                              // 793 Device ID for eDP device
  //
  // Hybrid Graphics (HG) Support  offset 797 
  //
  UINT8       SgMode;                        // (797) SG Mode (0=Disabled, 1=SG Muxed, 2=SG Muxless, 3=DGPU Only)
  UINT8       SgFeatureList;                 // (798) SG Feature list
  UINT8       SgDgpuPwrOK;                   // (799) dGPU PWROK GPIO assigned
  UINT8       SgDgpuHoldRst;                 // (800) dGPU HLD RST GPIO assigned
  UINT8       SgDgpuPwrEnable;               // (801) dGPU PWR Enable GPIO assigned
  UINT8       SgDgpuPrsnt;                   // (802) dGPU Present Detect GPIO assigned
  UINT8       EndpointPcieCapOffset;         // (803) PEG Endpoint PCIe Capability Structure Offset
  UINT16      EndpointVcCapOffset;           // (804) PEG Endpoint Virtual Channel Capability Structure Offset
  UINT32      IoBaseAddress;                 // (806) IO Base Address 
  UINT8       SgGPIOSupport;                 // (810) SG GPIO
  UINT32      NextStateDidEdp;               // (811) Next state DID for eDP
  UINT8       PackageCstateLimit;            // (815) The lowest C-state for the package
  UINT8       C7Allowed;                     // (816) Run-time C7 Allowed feature (0=Disabled, 1=Enabled)
  UINT8       NfcEnable;                     // (817) NFC enable/disable
  UINT8       TouchPadEnable;                // (818) Touch Pad enable/disable
  UINT32      RootPortBaseAddress;           // (819 - 822) PCH Root Port Base Address
  UINT32      EndPointBaseAddress;           // (823 - 826) PCH End Point Base Address 
  UINT8       VirtualButtonSupport;          // (827) Virtual Button Support  
  UINT16      MinPowerLimit;                 // (828) PPCC Minimum Power Limit
  UINT16      MaxPowerLimit;                 // (830) PPCC Maximum Power Limit
  UINT16      PowerStepSize;                 // (832) PPCC power step size
} EFI_GLOBAL_NVS_AREA;
#pragma pack ()

//
// Global NVS Area Protocol
//
typedef struct _EFI_GLOBAL_NVS_AREA_PROTOCOL {
  EFI_GLOBAL_NVS_AREA     *Area;
} EFI_GLOBAL_NVS_AREA_PROTOCOL;

#endif
