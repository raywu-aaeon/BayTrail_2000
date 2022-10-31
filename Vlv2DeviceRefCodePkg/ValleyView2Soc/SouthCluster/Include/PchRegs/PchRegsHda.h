/**
  This file contains an 'Intel Peripheral Driver' and uniquely        
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your   
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  PchRegsHda.h

  @brief
  Register names for PCH High Definition Audio device.
  
  Conventions:
  
  - Prefixes:
    Definitions beginning with "R_" are registers
    Definitions beginning with "B_" are bits within registers
    Definitions beginning with "V_" are meaningful values of bits within the registers
    Definitions beginning with "S_" are register sizes
    Definitions beginning with "N_" are the bit position
  - In general, PCH registers are denoted by "_PCH_" in register names
  - Registers / bits that are different between PCH generations are denoted by 
    "_PCH_<generation_name>_" in register/bit names. e.g., "_PCH_VLV_"
  - Registers / bits that are different between SKUs are denoted by "_<SKU_name>"
    at the end of the register/bit names
  - Registers / bits of new devices introduced in a PCH generation will be just named 
    as "_PCH_" without <generation_name> inserted.

**/
#ifndef _PCH_REGS_HDA_H_
#define _PCH_REGS_HDA_H_

///
/// Azalia Controller Registers (D27:F0)
///
#define PCI_DEVICE_NUMBER_PCH_AZALIA       27
#define PCI_FUNCTION_NUMBER_PCH_AZALIA     0

#define R_PCH_HDA_DEVVENID                 0x00  // Device / Vendor ID
#define B_PCH_HDA_DEVVENID_DEVICE_ID       0xFFFF0000 // Device ID
#define B_PCH_HDA_DEVVENID_VENDOR_ID       0x0000FFFF // Vendor ID
#define V_PCH_HDA_DEVVENID_VENDOR_ID       V_PCH_INTEL_VENDOR_ID // Intel Vendor ID
#define V_PCH_HDA_DEVICE_ID_0              0x0F04
#define V_PCH_HDA_DEVICE_ID_1              0x0F05

#define R_PCH_HDA_STSCMD                   0x04  // Status Command
#define B_PCH_HDA_STSCMD_SSE               BIT30 // SERR Status
#define B_PCH_HDA_STSCMD_RMA               BIT29 // Received Master Abort
#define B_PCH_HDA_STSCMD_CAP_LST           BIT20 // Capabilities List
#define B_PCH_HDA_STSCMD_INTR_STS          BIT19 // Interrupt Status
#define B_PCH_HDA_STSCMD_INTR_DIS          BIT10 // Interrupt Disable
#define B_PCH_HDA_STSCMD_SERR_EN           BIT8 // SERR Enable
#define B_PCH_HDA_STSCMD_BME               BIT2 // Bus Master Enable
#define B_PCH_HDA_STSCMD_MSE               BIT1 // Memory Space Enable

#define R_PCH_HDA_RID_CC                   0x08  // Revision ID and Class Code
#define B_PCH_HDA_RID_CC_BCC               0xFF000000 // Base Class Code
#define B_PCH_HDA_RID_CC_SCC               0x00FF0000 // Sub Class Code
#define B_PCH_HDA_RID_CC_PI                0x0000FF00 // Programming Interface
#define B_PCH_HDA_RID_CC_RID               0x000000FF // Revision Identification

#define R_PCH_HDA_CLS                      0x0C  // Cache Line Size
#define B_PCH_HDA_CLS                      0xFF  // Cache Line Size

#define R_PCH_HDA_LT                       0x0D  // Latency Timer
#define B_PCH_HDA_LT                       0xFF  // Latency Timer

#define R_PCH_HDA_HEADTYPE                 0x0E  // Header Type
#define B_PCH_HDA_HEADTYPE                 0xFF  // Header Type

#define R_PCH_HDA_HDBARL                   0x10  // HDA CTL Memory BAR Lower
#define B_PCH_HDA_HDBARL_LBA               0xFFFFC000 // Lower Base Address
#define B_PCH_HDA_HDBARL_PREF              BIT3  // Prefetchable
#define B_PCH_HDA_HDBARL_ADDRNG            (BIT2 | BIT1) // Address Range
#define B_PCH_HDA_HDBARL_SPTYP             BIT0  // Space Type (Memory)
#define V_PCH_HDA_HDBAR_SIZE               (1 << 14)

#define R_PCH_HDA_HDBARU                   0x14  // HDA CTL Memory BAR Upper
#define B_PCH_HDA_HDBARU_UBA               0xFFFFFFFF // Upper Base Address

#define R_PCH_HDA_SVID                     0x2C  // Sub System Vendor ID
#define B_PCH_HDA_SVID                     0xFFFF

#define R_PCH_HDA_SID                      0x2E  // Sub System ID
#define B_PCH_HDA_SID                      0xFFFF

#define R_PCH_HDA_CAPPTR                   0x34  // Capabilities Pointer
#define B_PCH_HDA_CAPPTR                   0xFF  // Capabilities Pointer

#define R_PCH_HDA_INTLN                    0x3C // Interrupt Line
#define B_PCH_HDA_INTLN                    0xFF // Interrupt Line

#define R_PCH_HDA_INTPN                    0x3D // Interrupt Pin
#define B_PCH_HDA_INTPN                    0x0F // Interrupt Pin

#define R_PCH_HDA_HDCTL                    0x40  // Azalia Control
#define B_PCH_HDA_HDCTL_MODE               BIT0  // Azalia or AC97

#define R_PCH_HDA_IOBC                     0x42  // IO Buffer Control
#define B_PCH_HDA_IOBC_ASRC                (BIT5 | BIT4) // Audio Buffer Slew Rate Control
#define B_PCH_HDA_IOBC_AVDDIS              BIT2  // Automatic Voltage Detector Disable
#define B_PCH_HDA_IOBC_MVSEL               BIT1  // Manual Voltage Select
#define B_PCH_HDA_IOBC_VMODE               BIT0  // Voltage Mode

#define R_PCH_HDA_TM1                      0x43  // Test Mode 1
#define B_PCH_HDA_TM1_FRESET               BIT7  // Fast Reset
#define B_PCH_HDA_TM1_ACCD                 BIT6  // Audio Crypto Disable
#define B_PCH_HDA_TM1_BCSS                 BIT4  // Bclk Source Select
#define B_PCH_HDA_TM1_HAPD                 BIT3  // HD Audio PCI/PCIe # Device
#define B_PCH_HDA_TM1_MDCGEN               BIT2  // Misc Dynamic Clock Gating Enable
#define B_PCH_HDA_TM1_IDCGEN               BIT1  // IDMA Dynamic Clock Gating Enable
#define B_PCH_HDA_TM1_ODCGEN               BIT0  // ODMA Dynamic Clock Gating Enable

#define R_PCH_HDA_DCKCTL                   0x4C  // Docking Control
#define B_PCH_HDA_DCKCTL_DA                BIT0  // Dock Attach

#define R_PCH_HDA_DCKSTS                   0x4D  // Docking Status
#define B_PCH_HDA_DCKSTS_DS                BIT7  // Docking Support
#define B_PCH_HDA_DCKSTS_DM                BIT0  // Docking Mated

#define R_PCH_HDA_PID                      0x50  // Power Management Capability ID
#define B_PCH_HDA_PID_NEXT                 0xFF00 // Next Capability
#define B_PCH_HDA_PID_CAP                  0x00FF // Capability ID

#define R_PCH_HDA_PC                       0x52  // Power Management Capabilities
#define B_PCH_HDA_PC_PME                   0xF800 // PME Status
#define B_PCH_HDA_PC_D2_SUP                BIT10 // D2 State, not supported
#define B_PCH_HDA_PC_D1_SUP                BIT9  // D1 State, not supported
#define B_PCH_HDA_PC_AUX                   (BIT8 | BIT7 | BIT6) // Aux Current
#define B_PCH_HDA_PC_DSI                   BIT5  // Device Specific Initialization
#define B_PCH_HDA_PC_PMEC                  BIT3  // PME Clock
#define B_PCH_HDA_PC_VER                   (BIT2 | BIT1 | BIT0) // Version

#define R_PCH_HDA_PCS                      0x54  // Power Management Control and Status
#define B_PCH_HDA_PCS_DATA                 0xFF000000 // Data, does not apply
#define B_PCH_HDA_PCS_CCE                  BIT23 // Bus Power Control Enable, does not apply
#define B_PCH_HDA_PCS_PMES                 BIT15 // PME Status
#define B_PCH_HDA_PCS_PMEE                 BIT8  // PME Enable
#define B_PCH_HDA_PCS_PS                   (BIT1 | BIT0) // Power State - D0/D3 Hot
#define V_PCH_HDA_PCS_PS0                  0x00
#define V_PCH_HDA_PCS_PS3                  0x03

#define R_PCH_HDA_MID                      0x60  // MSI Capability ID
#define B_PCH_HDA_MID_NEXT                 0xFF00 // Next Capability
#define B_PCH_HDA_MID_CAP                  0x00FF // capability ID

#define R_PCH_HDA_MMC                      0x62  // MSI Message Control
#define B_PCH_HDA_MMC_64ADD                BIT7  // 64-bit Address Support
#define B_PCH_HDA_MMC_MME                  (BIT6 | BIT5 | BIT4) // Multiple Message Enable
#define B_PCH_HDA_MMC_MMC                  (BIT3 | BIT2 | BIT1) // Multiple Message Capable
#define B_PCH_HDA_MMC_ME                   BIT0  // MSI Enable

#define R_PCH_HDA_MMLA                     0x64  // MSI Lower Address
#define B_PCH_HDA_MMLA                     0xFFFFFFFC // MSI Lower Address

#define R_PCH_HDA_MMUA                     0x68  // MSI Upper Address
#define B_PCH_HDA_MMUA                     0xFFFFFFFF  // MSI Upper Address

#define R_PCH_HDA_MMD                      0x6C  // MSI Data
#define B_PCH_HDA_MMD                      0xFFFF // MSI Data

#define R_PCH_HDA_PXID                     0x70  // PCIe Capability ID
#define B_PCH_HDA_PXID_NEXT                0xFF00 // Next capability
#define B_PCH_HDA_PXID_CAP                 0x00FF // Capability ID

#define R_PCH_HDA_PXC                      0x72  // PCI Express Capabilities
#define B_PCH_HDA_PXC_IMN                  0x3E00 // Interrupt Message Number
#define B_PCH_HDA_PXC_SI                   BIT8  // Slot Implemented
#define B_PCH_HDA_PXC_DPT                  0x00F0 // Device / Port Type
#define B_PCH_HDA_PXC_CV                   0x000F // Capability Version

#define R_PCH_HDA_DEVCAP                   0x74  // Device Capabilities
#define B_PCH_HDA_DEVCAP_FLR               BIT28 // Function Level Reset
#define B_PCH_HDA_DEVCAP_SPLS              (BIT27 | BIT26) // Captured Slot Power Limit Scale
#define B_PCH_HDA_DEVCAP_SPLV              0x03FC0000 // Captured Slot Power Limit Value
#define B_PCH_HDA_DEVCAP_PWRIP             BIT14 // Power Indicator Present
#define B_PCH_HDA_DEVCAP_ATTNIP            BIT13 // Attention Indicator Present
#define B_PCH_HDA_DEVCAP_ATTNBP            BIT12 // Attention Button Present
#define B_PCH_HDA_DEVCAP_EL1AL             (BIT11 | BIT10 | BIT9) // Endpoint L1 Acceptable Latency
#define B_PCH_HDA_DEVCAP_EL0AL             (BIT8 | BIT7 | BIT6) // Endpoint L0s Acceptable Latency
#define B_PCH_HDA_DEVCAP_ETFS              BIT5  // Extended Tag Field Support
#define B_PCH_HDA_DEVCAP_PFS               (BIT4 | BIT3) // Phantom Functions Supported
#define B_PCH_HDA_DEVCAP_MPSS              (BIT2 | BIT1 | BIT0) // Max Payload Size Supported

#define R_PCH_HDA_DEVC                     0x78  // Device Control
#define B_PCH_HDA_DEVC_IF                  BIT15 // Initiate FLR
#define B_PCH_HDA_DEVC_MRRS                (BIT14 | BIT13 | BIT12) // Max Read Request Size
#define B_PCH_HDA_DEVC_NSNPEN              BIT11 // Enable No Snoop
#define B_PCH_HDA_DEVC_APE                 BIT10 // Auxiliary Power PM Enable
#define B_PCH_HDA_DEVC_PFE                 BIT9  // Phantom Function Enable
#define B_PCH_HDA_DEVC_ETFE                BIT8  // Extended Tag Field Enable
#define B_PCH_HDA_DEVC_MPS                 (BIT7 | BIT6 | BIT5) // Extended Tag Field Enable
#define B_PCH_HDA_DEVC_ERO                 BIT4  // Enable Relaxed Ordering
#define B_PCH_HDA_DEVC_URRE                BIT3  // Unsupported Request Reporting Enable
#define B_PCH_HDA_DEVC_FERE                BIT2  // Fatal Error Reporting Enable
#define B_PCH_HDA_DEVC_NFERE               BIT1  // Non-Fatal Error Reporting Enable
#define B_PCH_HDA_DEVC_CERE                BIT0  // Correctable Error Reporting Enable

#define R_PCH_HDA_DEVS                     0x7A  // Device Status
#define B_PCH_HDA_DEVS_TP                  BIT5  // Transactions Pending
#define B_PCH_HDA_DEVS_AUXPD               BIT4  // AUX Power Detected
#define B_PCH_HDA_DEVS_URD                 BIT3  // Unsupported Request Detected
#define B_PCH_HDA_DEVS_FED                 BIT2  // Fatal Error Detected
#define B_PCH_HDA_DEVS_NFED                BIT1  // Non-Fatal Error Detected
#define B_PCH_HDA_DEVS_CED                 BIT0  // Correctable Error Detected

#define R_PCH_HDA_SEM1                     0xC0  // Shadowed Extended Mode 1
#define B_PCH_HDA_SEM1_LE                  0xF0000000 // Loopback Enable
#define B_PCH_HDA_SEM1_FCQ                 (BIT27 | BIT26) // Free Count Request
#define B_PCH_HDA_SEM1_PS                  BIT25 // Phase Select
#define B_PCH_HDA_SEM1_BB                  BIT24 // Boundary Break
#define B_PCH_HDA_SEM1_CP                  (BIT23 | BIT22 | BIT21) // Corb Pace
#define B_PCH_HDA_SEM1_ROE                 BIT20 // Relaxed Ordering Enable
#define B_PCH_HDA_SEM1_EPIE                BIT19 // Error Present Interrupt Enable
#define B_PCH_HDA_SEM1_EPS                 BIT18 // Error Present Status
#define B_PCH_HDA_SEM1_BCD                 BIT17 // Block C3 Disable
#define B_PCH_HDA_SEM1_FBC                 BIT16 // Force Block C3
#define B_PCH_HDA_SEM1_LBID                BIT15 // Level Based Interrupts Disable
#define B_PCH_HDA_SEM1_KE                  BIT14 // KHz Enable
#define B_PCH_HDA_SEM1_DESTS               (BIT13 | BIT12) // Dock Enable Signal Transition Select
#define B_PCH_HDA_SEM1_DHCD                BIT11 // DPIB Hardware Control Disable
#define B_PCH_HDA_SEM1_FRS                 (BIT10 | BIT9) // FIFO Ready Select 1
#define B_PCH_HDA_SEM1_C64BP               BIT8  // Crossing 64B Policy
#define B_PCH_HDA_SEM1_RCR                 0xFF  // Repeat Count Resets

#define R_PCH_HDA_SEM2                     0xC4  // Shadowed Extended Mode 2
#define B_PCH_HDA_SEM2_FSRWE               BIT31 // FIFO Size Register Write Enable
#define B_PCH_HDA_SEM2_SDBSC               (BIT30 | BIT29 | BIT28) // Shared Data Buffer Size Configuration
#define B_PCH_HDA_SEM2_BSMT                (BIT27 | BIT26) // Buffer Size Minimum Threshold
#define B_PCH_HDA_SEM2_EECW                (BIT25 | BIT24 | BIT23) // EM2 Ecoable Core Well
#define B_PCH_HDA_SEM2_LS                  BIT22 // Legacy Select
#define B_PCH_HDA_SEM2_IDFGRS              (BIT21 | BIT20 | BIT19) // Initial Data Fetch Grant Rate Select
#define B_PCH_HDA_SEM2_BDGRS               (BIT18 | BIT17 | BIT16) // Buffer Descriptor Grant Select
#define B_PCH_HDA_SEM2_CALEE               BIT15 // CPU Access L1 Exit Enable
#define B_PCH_HDA_SEM2_OBMLEE              BIT14 // Other Bus Master L1 Exit Enable
#define B_PCH_HDA_SEM2_LSE                 BIT13 // L1 Support Enable
#define B_PCH_HDA_SEM2_CBLEE               BIT12 // Cx Break L1 Exit Enable
#define B_PCH_HDA_SEM2_RFE                 BIT11 // RIRB Fix Enable
#define B_PCH_HDA_SEM2_MQRDAD              BIT10 // Middle QW Request De-Assertion Disable
#define B_PCH_HDA_SEM2_FGMD                BIT9  // Frame Get Mask Disable
#define B_PCH_HDA_SEM2_CRPCD               BIT8  // Corb Reset Pointer Change Disable
#define B_PCH_HDA_SEM2_EE                  0xFC  // EM2 Ecoable
#define B_PCH_HDA_SEM2_IACE                BIT1  // Internal Audio Codec Enable
#define B_PCH_HDA_SEM2_HAID                BIT0  // HD Audio IO Disabled

#define R_PCH_HDA_SEM3_LOW                 0xC8  // Shadowed Extended Mode 3 (Low DW)
#define B_PCH_HDA_SEM3_LOW_RLET            0xF8000000 // RIRB L1 Entry Threshold
#define B_PCH_HDA_SEM3_LOW_ISLEXITT        0x07800000 // Input Stream L1 Exit Threshold
#define B_PCH_HDA_SEM3_LOW_EECWB22         BIT22 // EM3 Ecoable Core Well Bit 22
#define B_PCH_HDA_SEM3_LOW_ISLENTRYT       0x003F0000 // Input Stream L1 Entry Threshold
#define B_PCH_HDA_SEM3_LOW_EECWB13         BIT13 // EM3 Ecoable Core Well Bit 13
#define B_PCH_HDA_SEM3_LOW_ISRWS           0x00001F80 // Input Stream Request Watermark Select
#define B_PCH_HDA_SEM3_LOW_EECWB6          0x0000007F // EM3 Ecoable Core Well Bit 6

#define R_PCH_HDA_SEM3_HIGH                0xCC  // Shadowed Extended Mode 3 (High DW)
#define B_PCH_HDA_SEM3_HIGH_RSRP           (BIT31 | BIT30 | BIT29 | BIT28) // RIRB Solicited Response Pacing
#define B_PCH_HDA_SEM3_HIGH_EECWB5958      (BIT27 | BIT26) // E3 Ecoable Core Well Bit 59 58
#define B_PCH_HDA_SEM3_HIGH_ISLPT          0x03F00000 // Input Stream Low Priority Threshold
#define B_PCH_HDA_SEM3_HIGH_ISBSEFA        (BIT19 | BIT18) // Input Stream Buffer Size Extra Frame Allocation
#define B_PCH_HDA_SEM3_HIGH_EECWB4741      0x0000FE00 // EM3 Ecoable Core Well Bit 47 41
#define B_PCH_HDA_SEM3_HIGH_ISDBT4K        0x000001E0 // Input Stream Data Buffer To 4K
#define B_PCH_HDA_SEM3_HIGH_RLET           0x0000001F // RIRB L1 Exit Threshold

#define R_PCH_HDA_SEM4_LOW                 0xD0  // Shadowed Extended Mode 4 (Low DW)
#define B_PCH_HDA_SEM4_LOW_CLET            0xF8000000 // Corb L1 Entry Threshold
#define B_PCH_HDA_SEM4_LOW_OSLEXITT        0x07800000 // Output Stream L1 Exit Threshold
#define B_PCH_HDA_SEM4_LOW_EECWB22         BIT22 // EM4 Ecoable Core Well Bit 22
#define B_PCH_HDA_SEM4_LOW_OSLENTRYT       0x003F0000 // Output Stream L1 Entry Threshold
#define B_PCH_HDA_SEM4_LOW_OSFBSA          (BIT15 | BIT14) // Output Stream Fixed Buffer Size Allocation
#define B_PCH_HDA_SEM4_LOW_OSRWS           0x00003F80 // Output Stream Request Watermark Select
#define B_PCH_HDA_SEM4_LOW_EECWB60         0x0000007F // EM4 Ecoable Core Well Bits 6 0

#define R_PCH_HDA_SEM4_HIGH                0xD4  // Shadowed Extended Mode 4 (High DW)
#define B_PCH_HDA_SEM4_HIGH_EECWB6358      0xFC000000 // EM4 Ecoable Core Well Bits 63 58
#define B_PCH_HDA_SEM4_HIGH_OSLPT          0x03F00000 // Output Stream Low Priority Threshold
#define B_PCH_HDA_SEM4_HIGH_OSBSEFA        (BIT19 | BIT18) // Output Stream Buffer Size Extra Frame Allocation
#define B_PCH_HDA_SEM4_HIGH_OSBSMFA        (BIT17 | BIT16) // Output Stream Buffer Size Minimum Frame Allocation
#define B_PCH_HDA_SEM4_HIGH_EECWB4741      0x0000FE00 // EM4 Ecoable Core Well Bits 47 41
#define B_PCH_HDA_SEM4_HIGH_OSDBT4K        0x000001E0 // Output Stream Data Buffer To 4K
#define B_PCH_HDA_SEM4_HIGH_CLET           0x0000001F // Corb L1 Exit Threshold

#define R_PCH_HDA_MANID                    0xF8  // Manufacturer ID
#define B_PCH_HDA_MANID_DPID               0x0F000000 // Dot Portion Process ID
#define B_PCH_HDA_MANID_SID                0x00FF0000 // Stepping ID
#define B_PCH_HDA_MANID_MNFR               0x0000FF00 // Manufacturer
#define B_PCH_HDA_MANID_PPID               0x000000FF // Process Portion Process ID

#define R_PCH_HDA_VCCAP                    0x100 // Virtual Channel Cap Header
#define B_PCH_HDA_VCCAP_NCO                0xFFF00000 // Next capability Offset
#define B_PCH_HDA_VCCAP_CAPVER             0x000F0000 // Capability Version
#define B_PCH_HDA_VCCAP_PCIEEC             0x0000FFFF // PCI Express Extended Capability ID

#define R_PCH_HDA_PVCCAP1                  0x104 // Port VC Capability
#define B_PCH_HDA_PVCCAP1_PATES            0x00000C00 // Port Arbitration Table Entry Size
#define B_PCH_HDA_PVCCAP1_RC               0x00000300 // Reference Clock
#define B_PCH_HDA_PVCCAP1_LPEVCC           0x00000070 // Low Priority Extended VC Count
#define B_PCH_HDA_PVCCAP1_EVCC             0x00000007 // Extended VC Count

#define R_PCH_HDA_PVCCAP2                  0x108 // Port VC Capability 2
#define B_PCH_HDA_PVCCAP2_VCATO            0xFF000000 // VC Arbitration Table Offset
#define B_PCH_HDA_PVCCAP2_VCAC             0x000000FF // VC Arbitration Capability

#define R_PCH_HDA_PVCCTL                   0x10C // VC Port Control
#define B_PCH_HDA_PVCCTL_VCAS              0x000E // VC Arbitration Select
#define B_PCH_HDA_PVCCTL_LVCAT             0x0001 // Load VC Arbitration Table

#define R_PCH_HDA_PVCSTS                   0x10E // Port VC Status
#define B_PCH_HDA_PVCSTS_VCATS             BIT0  // VC Arbitration Table Status

#define R_PCH_HDA_VC0CAP                   0x110 // VC0 Resource Capability
#define S_PCH_HDA_VC0CAP                   4
#define B_PCH_HDA_VC0CAP_PATO              0xFF000000 // Port Arbitration Table Offset
#define B_PCH_HDA_VC0CAP_MTS               0x007F0000 // Maximum Time Slot
#define B_PCH_HDA_VC0CAP_RST               BIT15 // Reject Snoop Transactions
#define B_PCH_HDA_VC0CAP_APS               BIT14 // Advanced Packet Switching
#define B_PCH_HDA_VC0CAP_PAC               0x000000FF // Port Arbitration Capability

#define R_PCH_HDA_VC0CTL                   0x114 // VC0 Resource Control
#define S_PCH_HDA_VC0CTL                   4
#define B_PCH_HDA_VC0CTL_VC0EN             BIT31 // VC0 Enable
#define B_PCH_HDA_VC0CTL_VC0ID             0x07000000 // VC0 ID
#define B_PCH_HDA_VC0CTL_PAS               0x000E0000 // Port Arbitration Select
#define B_PCH_HDA_VC0CTL_LPAT              BIT16 // Load Port Arbitration Table
#define B_PCH_HDA_VC0CTL_TCVC0_MAP         0x000000FE // TC / VC0 Map

#define R_PCH_HDA_VC0STS                   0x11A // VC0 Resource Status
#define S_PCH_HDA_VC0STS                   2
#define B_PCH_HDA_VC0STS_VC0NP             BIT1 // VC0 Negotiation Pending
#define B_PCH_HDA_VC0STS_PATS              BIT0 // Port Arbitration Table Status

#define R_PCH_HDA_VCICAP                   0x11C // VCi Resource Capabilities
#define S_PCH_HDA_VCICAP                   4
#define B_PCH_HDA_VCICAP_PATO              0xFF000000 // Port Arbitration Table Offset
#define B_PCH_HDA_VCICAP_MTS               0x007F0000 // Maximum Time Slots
#define B_PCH_HDA_VCICAP_RST               BIT15 // Reject Snoop Transactions
#define B_PCH_HDA_VCICAP_APS               BIT14 // Advanced Packet Switching
#define B_PCH_HDA_VCICAP_PAC               0x000000FF // Port Arbitration Capability

#define R_PCH_HDA_VCICTL                   0x120 // VCi Control Register
#define S_PCH_HDA_VCICTL                   4
#define B_PCH_HDA_VCICTL_EN                BIT31 // VCi Enable
#define B_PCH_HDA_VCICTL_ID                (BIT26 | BIT25 | BIT24) // VCi ID
#define V_PCH_HDA_VCICTL_PAS               0x000E0000 // Port Arbitration Select
#define V_PCH_HDA_VCICTL_LPAT              BIT16 // Load Port Arbitration Table
#define B_PCH_HDA_VCICTL_TCVCI_MAP         0x000000FE // TC / VCi Map

#define R_PCH_HDA_VCISTS                   0x126 // VCi Resource Status
#define S_PCH_HDA_VCISTS                   1
#define B_PCH_HDA_VCISTS_VCINP             BIT1  // VCi Negotiation Pending
#define B_PCH_HDA_VCISTS_PATS              BIT0  // Port Arbitration Table Status

#define R_PCH_HDA_RCCAP                    0x130 // Root Complex Link Declaration Capability Header
#define B_PCH_HDA_RCCAP_NCO                0xFFF00000 // Next Capability Offset
#define B_PCH_HDA_RCCAP_CV                 0x000F0000 // Capability Version
#define B_PCH_HDA_RCCAP_PCIEECID           0x0000FFFF // PCI Express Extended Capability ID

#define R_PCH_HDA_ESD                      0x134 // Element Self Description
#define B_PCH_HDA_ESD_PN                   0xFF000000 // Port Number
#define B_PCH_HDA_ESD_CID                  0x00FF0000 // Number of Link Entries
#define B_PCH_HDA_ESD_ELTYP                0x0000000F // Element Type

#define R_PCH_HDA_L1DESC                   0x140 // Link 1 Description
#define S_PCH_HDA_L1DESC                   4
#define B_PCH_HDA_LIDESC_TPN               0xFF000000 // Target Port Number
#define B_PCH_HDA_LIDESC_TCID              0x00FF0000 // Target Component ID
#define B_PCH_HDA_LIDESC_LT                BIT1  // Link Type
#define B_PCH_HDA_LIDESC_LV                BIT0  // Link Valid

#define R_PCH_HDA_L1ADDL                   0x148 // Link 1 Lower Address
#define B_PCH_HDA_L1ADDL_LNK1LA            0xFFFFC000 // Link 1 Lower Address

#define R_PCH_HDA_L1ADDU                   0x14C // Link 1 Upper Address
#define B_PCH_HDA_L1ADDU                   0xFFFFFFFF // Link 1 Upper Address


//
// Intel High Definition Audio Memory Mapped Configuration Registers
//
#define R_HDA_GCAP                         0x00  // Global Capabilities
#define S_HDA_GCAP                         2
#define B_HDA_GCAP_NOSSUP                  0xF000 // Number of Output Streams Supported
#define B_HDA_GCAP_NISSUP                  0x0F00 // Number of Input Streams Supported
#define B_HDA_GCAP_NBSSUP                  0x00F8 // Number of Bidirectional Streams Supported
#define B_HDA_GCAP_NSDOS                   (BIT2 | BIT1) // Number of Serial Data Out Signals
#define B_HDA_GCAP_64ADSUP                 BIT0  // 64bit Address Supported

#define R_HDA_VMIN                         0x02  // Minor Version
#define B_HDA_VMIN_MV                      0xFF  // Minor Version

#define R_HDA_VMAJ                         0x03  // Major Version
#define B_HDA_VMAJ_MV                      0xFF  // Major Version

#define R_HDA_OUTPAY                       0x04  // Output Payload Capability
#define B_HDA_OUTPAY_CAP                   0xFFFF // Output Payload Capability

#define R_HDA_INPAY                        0x06  // Input Payload Capability
#define B_HDA_INPAY_CAP                    0xFFFF // Input Payload Capability

#define R_HDA_GCTL                         0x08  // Global Control
#define B_HDA_GCTL_AURE                    BIT8  // Accept Unsolicited Response Enable
#define B_HDA_GCTL_FC                      BIT1  // Flush Control
#define B_HDA_GCTL_CRST                    BIT0  // Controller Reset

#define R_HDA_WAKEEN                       0x0C  // Wake Enable
#define B_HDA_WAKEEN_SDI_3                 BIT3  // SDIN Wake Enable Flag 3
#define B_HDA_WAKEEN_SDI_2                 BIT2  // SDIN Wake Enable Flag 2
#define B_HDA_WAKEEN_SDI_1                 BIT1  // SDIN Wake Enable Flag 1
#define B_HDA_WAKEEN_SDI_0                 BIT0  // SDIN Wake Enable Flag 0

#define R_HDA_STATESTS                     0x0E  // Wake Status
#define B_HDA_STATESTS_SDIN3               BIT3  // SDIN State Change 3
#define B_HDA_STATESTS_SDIN2               BIT2  // SDIN State Change 2
#define B_HDA_STATESTS_SDIN1               BIT1  // SDIN State Change 1
#define B_HDA_STATESTS_SDIN0               BIT0  // SDIN State Change 0

#define R_HDA_GSTS                         0x10  // Global Status
#define B_HDA_GSTS_FS                      BIT1  // Flush Status

#define R_HDA_OUTSTRMPAY                   0x18  // Output Stream Payload Capability
#define S_HDA_OUTSTRMPAY                   2
#define B_HDA_OUTSTRMPAY_OUTSTRMPAY        0xFFFF // Output Stream Payload Capability

#define R_HDA_INSTRMPAY                    0x1A  // Input Stream Payload Capability
#define B_HDA_INSTRMPAY_INSTRMPAY          0xFFFF // Input Stream Payload Capability

#define R_HDA_INTCTL                       0x20  // Interrupt Control
#define B_HDA_INTCTL_GIE                   BIT31 // Global Interrupt Enable
#define B_HDA_INTCTL_CIE                   BIT30 // Controller Interrupt Enable
#define B_HDA_INTCTL_SIE_OS4               BIT7  // Stream Interrupt Enable - Output Stream 4
#define B_HDA_INTCTL_SIE_OS3               BIT6  // Stream Interrupt Enable - Output Stream 3
#define B_HDA_INTCTL_SIE_OS2               BIT5  // Stream Interrupt Enable - Output Stream 2
#define B_HDA_INTCTL_SIE_OS1               BIT4  // Stream Interrupt Enable - Output Stream 1
#define B_HDA_INTCTL_SIE_IS4               BIT3  // Stream Interrupt Enable - Input Stream 4
#define B_HDA_INTCTL_SIE_IS3               BIT2  // Stream Interrupt Enable - Input Stream 3
#define B_HDA_INTCTL_SIE_IS2               BIT1  // Stream Interrupt Enable - Input Stream 2
#define B_HDA_INTCTL_SIE_IS1               BIT0  // Stream Interrupt Enable - Input Stream 1

#define R_HDA_INTSTS                       0x24  // Interrupt Status
#define B_HDA_INTSTS_GIS                   BIT31 // Global Interrupt Status
#define B_HDA_INTSTS_CIS                   BIT30 // Controller Interrupt Status
#define B_HDA_INTSTS_SIS_OS4               BIT7  // Stream Interrupt Status - Output Stream 4
#define B_HDA_INTSTS_SIS_OS3               BIT6  // Stream Interrupt Status - Output Stream 3
#define B_HDA_INTSTS_SIS_OS2               BIT5  // Stream Interrupt Status - Output Stream 2
#define B_HDA_INTSTS_SIS_OS1               BIT4  // Stream Interrupt Status - Output Stream 1
#define B_HDA_INTSTS_SIS_IS4               BIT3  // Stream Interrupt Enable - Input Stream 4
#define B_HDA_INTSTS_SIS_IS3               BIT2  // Stream Interrupt Enable - Input Stream 3
#define B_HDA_INTSTS_SIS_IS2               BIT1  // Stream Interrupt Enable - Input Stream 2
#define B_HDA_INTSTS_SIS_IS1               BIT0  // Stream Interrupt Enable - Input Stream 1

#define R_HDA_WALCLK                       0x30  // Wall Clock Counter
#define B_HDA_WALCLK_WCC                   0xFFFFFFFF // Wall Clock Counter

#define R_HDA_SSYNC                        0x38  // Stream Synchronization
#define S_HDA_SSYNC                        4
#define B_HDA_SSYNC_OS4                    BIT7  // Stream Synchronization - Output Stream 4
#define B_HDA_SSYNC_OS3                    BIT6  // Stream Synchronization - Output Stream 3
#define B_HDA_SSYNC_OS2                    BIT5  // Stream Synchronization - Output Stream 2
#define B_HDA_SSYNC_OS1                    BIT4  // Stream Synchronization - Output Stream 1
#define B_HDA_SSYNC_IS4                    BIT3  // Stream Synchronization - Input Stream 4
#define B_HDA_SSYNC_IS3                    BIT2  // Stream Synchronization - Input Stream 3
#define B_HDA_SSYNC_IS2                    BIT1  // Stream Synchronization - Input Stream 2
#define B_HDA_SSYNC_IS1                    BIT0  // Stream Synchronization - Input Stream 1

#define R_HDA_CORBLBASE                    0x40  // CORB Lower Base Address
#define B_HDA_CORBLBASE_BA                 0xFFFFFF80 // CORB Lower Base Address
#define B_HDA_CORBLBASE_UB                 0x0000007F // CORB Lower Base Unimplemented Bits

#define R_HDA_CORBUBASE                    0x44  // CORB Upper Base Address
#define B_HDA_CORBUBASE_BA                 0xFFFFFFFF // CORB Upper Base Address

#define R_HDA_CORBWP                       0x48  // CORB Write Pointer
#define B_HDA_CORBWP                       0x000000FF // CORB Write Pointer

#define R_HDA_CORBRP                       0x4A  // CORB Read Pointer
#define B_HDA_CORBRP_PRST                  BIT15 // CORB Read Pointer Reset
#define B_HDA_CORBRP_RP                    0x00FF // CORB Read Pointer

#define R_HDA_CORBCTL                      0x4C  // CORB Control
#define B_HDA_CORBCTL_DMA_EN               BIT1  // Enable CORB DMA Engine
#define B_HDA_CORBCTL_MEMERRINTR_EN        BIT0  // CORB Memory Error Interrupt Enable

#define R_HDA_CORBST                       0x4D  // CORB Status
#define B_HDA_CORBST_CMEI                  BIT0  // CORB Memory Error Indication

#define R_HDA_CORBSIZE                     0x4E  // CORB Size
#define B_HDA_CORBSIZE_CAP                 0xF0  // CORB Size Capability
#define B_HDA_CORBSIZE_SIZE                (BIT1 | BIT0) // CORB Size

#define R_HDA_RIRBLBASE                    0x50  // RIRB Lower Base Address
#define B_HDA_RIRBLBASE_BA                 0xFFFFFF80 // RIRB Lower Base Address
#define B_HDA_RIRBLBASE_UB                 0x0000007F // RIRB Lower Base Unimplemented Bits

#define R_HDA_RIRBUBASE                    0x54  // RIRB Upper Base Address
#define B_HDA_RIRBUBASE_BA                 0xFFFFFFFF // RIRB Upper Base Address

#define R_HDA_RIRBWP                       0x58  // RIRB Write Pointer
#define B_HDA_RIRBWP_RST                   BIT15 // RIRB Write Pointer Reset
#define B_HDA_RIRBWP_WP                    0x00FF // RIRB Write Pointer

#define R_HDA_RINTCNT                      0x5A  // Response Interrupt Count
#define B_HDA_RINTCNT                      0x00FF // Number of Response Interrupt Count

#define R_HDA_RIRBCTL                      0x5C  // RIRB Control
#define B_HDA_RIRBCTL_ROIC                 BIT2  // Response Overrun Interrupt Control
#define B_HDA_RIRBCTL_DMA                  BIT1  // RIRB DMA Enable
#define B_HDA_RIRBCTL_RIC                  BIT0  // Response Interrupt Control

#define R_HDA_RIRBSTS                      0x5D  // RIRB Status
#define B_HDA_RIRBSTS_ROIS                 BIT2  // Response Overrun Interrupt Status
#define B_HDA_RIRBSTS_RI                   BIT0  // Response Interrupt

#define R_HDA_RIRBSIZE                     0x5E  // RIRB Size
#define B_HDA_RIRBSIZE_CAP                 0xF0  // RIRB Size Capability
#define B_HDA_RIRBSIZE_SIZE                0x03  // RIRB Size

#define R_HDA_IC                           0x60  // Immediate Command
#define B_HDA_IC                           0xFFFFFFFF // Immediate Command Write

#define R_HDA_IR                           0x64  // Immediate Response
#define B_HDA_IR                           0xFFFFFFFF // Immediate Response Read

#define R_HDA_ICS                          0x68  // Immediate Command Status
#define B_HDA_ICS_IRV                      BIT1  // Immediate Result Valid
#define B_HDA_ICS_ICB                      BIT0  // Immediate Command Busy

#define R_HDA_DPLBASE                      0x70  // DMA Position Lower Base Address
#define B_HDA_DPLBASE_LBA                  0xFFFFFF80 // DMA Position Lower Base Address
#define B_HDA_DPLBASE_LBU                  0x0000007E // DMA Position Lower Base Unimplemented Bits
#define B_HDA_DPLBASE_BUF_EN               0x00000001 // DMA Position Buffer Enable

#define R_HDA_DPUBASE                      0x74  // DMA Position Upper Base Address
#define B_HDA_DPUBASE_UBA                  0xFFFFFFFF // DMA Position Upper Base Address

#define R_HDA_ISD0CTL_STS                  0x80  // Input Stream Descriptor 0 Control and Status
#define R_HDA_ISD1CTL_STS                  0xA0  // Input Stream Descriptor 1 Control and Status
#define R_HDA_ISD2CTL_STS                  0xC0  // Input Stream Descriptor 2 Control and Status
#define R_HDA_ISD3CTL_STS                  0xE0  // Input Stream Descriptor 3 Control and Status
#define R_HDA_OSD0CTL_STS                  0x100 // Output Stream Descriptor 0 Control and Status
#define R_HDA_OSD1CTL_STS                  0x120 // Output Stream Descriptor 1 Control and Status
#define R_HDA_OSD2CTL_STS                  0x140 // Output Stream Descriptor 2 Control and Status
#define R_HDA_OSD3CTL_STS                  0x160 // Output Stream Descriptor 3 Control and Status
#define B_HDA_XSDXCTL_STS_FIFORDY          BIT29 // FIFO Ready
#define B_HDA_XSDXCTL_STS_DE               BIT28 // Descriptor Error
#define B_HDA_XSDXCTL_STS_FIFO_ERROR       BIT27 // FIFO Error
#define B_HDA_XSDXCTL_STS_BUF_COMPINTR_STS BIT26 // Buffer Completion Interrupt Status
#define B_HDA_XSDXCTL_STS_SN               (BIT23 | BIT22 | BIT21 | BIT20) // Stream Number
#define B_HDA_XSDXCTL_STS_BDC              BIT19 // Bidirectional Direction Control
#define B_HDA_XSDXCTL_STS_TP               BIT18 // Traffic Priority
#define B_HDA_XSDXCTL_STS_SC               (BIT17 | BIT16) // Stripe Control
#define B_HDA_XSDXCTL_STS_DEIE             BIT4  // Descriptor Error Interrupt Enable
#define B_HDA_XSDXCTL_STS_FIFO_ERRINT_EN   BIT3  // FIFO Error Interrupt Enable
#define B_HDA_XSDXCTL_STS_INTR_ONCOMP_EN   BIT2  // Interrupt On Completion Enable
#define B_HDA_XSDXCTL_STS_RUN              BIT1  // Stream Run
#define B_HDA_XSDXCTL_STS_SRST             BIT0  // Stream Reset

#define R_HDA_SDLPIB_IN_0                  0x84  // Input Stream Descriptor 0 Link Position in Buffer
#define R_HDA_SDLPIB_IN_1                  0xA4  // Input Stream Descriptor 1 Link Position in Buffer
#define R_HDA_SDLPIB_IN_2                  0xC4  // Input Stream Descriptor 2 Link Position in Buffer
#define R_HDA_SDLPIB_IN_3                  0xE4  // Input Stream Descriptor 3 Link Position in Buffer
#define R_HDA_SDLPIB_OUT_0                 0x104 // Output Stream Descriptor 0 Link Position in Buffer
#define R_HDA_SDLPIB_OUT_1                 0x124 // Output Stream Descriptor 1 Link Position in Buffer
#define R_HDA_SDLPIB_OUT_2                 0x144 // Output Stream Descriptor 2 Link Position in Buffer
#define R_HDA_SDLPIB_OUT_3                 0x164 // Output Stream Descriptor 3 Link Position in Buffer
#define B_HDA_SDLPIB_BUFFER                0xFFFFFFFF // Link Position In Buffer

#define R_HDA_SDCBL_IN_0                   0x88  // Input Stream Descriptor 0 Cyclic Buffer Length
#define R_HDA_SDCBL_IN_1                   0xA8  // Input Stream Descriptor 1 Cyclic Buffer Length
#define R_HDA_SDCBL_IN_2                   0xC8  // Input Stream Descriptor 2 Cyclic Buffer Length
#define R_HDA_SDCBL_IN_3                   0xE8  // Input Stream Descriptor 3 Cyclic Buffer Length
#define R_HDA_SDCBL_OUT_0                  0x108 // Output Stream Descriptor 0 Cyclic Buffer Length
#define R_HDA_SDCBL_OUT_1                  0x128 // Output Stream Descriptor 1 Cyclic Buffer Length
#define R_HDA_SDCBL_OUT_2                  0x148 // Output Stream Descriptor 2 Cyclic Buffer Length
#define R_HDA_SDCBL_OUT_3                  0x168 // Output Stream Descriptor 3 Cyclic Buffer Length
#define B_HDA_SDCBL_BUFLNG                 0xFFFFFFFF // Cyclic Buffer Length

#define R_HDA_SDLVI_IN_0                   0x8C  // Input Stream Descriptor 0 Last Valid Index
#define R_HDA_SDLVI_IN_1                   0xAC  // Input Stream Descriptor 1 Last Valid Index
#define R_HDA_SDLVI_IN_2                   0xCC  // Input Stream Descriptor 2 Last Valid Index
#define R_HDA_SDLVI_IN_3                   0xEC  // Input Stream Descriptor 3 Last Valid Index
#define R_HDA_SDLVI_OUT_0                  0x10C // Output Stream Descriptor 0 Last Valid Index
#define R_HDA_SDLVI_OUT_1                  0x12C // Output Stream Descriptor 1 Last Valid Index
#define R_HDA_SDLVI_OUT_2                  0x14C // Output Stream Descriptor 2 Last Valid Index
#define R_HDA_SDLVI_OUT_3                  0x16C // Output Stream Descriptor 3 Last Valid Index
#define B_HDA_SDLVI_LVI                    0x00FF // Last Valid Index

#define R_HDA_SDFIFOW_IN_0                 0x8E  // Input Stream Descriptor 0 FIFO Eviction Watermark
#define R_HDA_SDFIFOW_IN_1                 0xAE  // Input Stream Descriptor 1 FIFO Eviction Watermark
#define R_HDA_SDFIFOW_IN_2                 0xCE  // Input Stream Descriptor 2 FIFO Eviction Watermark
#define R_HDA_SDFIFOW_IN_3                 0xEE  // Input Stream Descriptor 3 FIFO Eviction Watermark
#define R_HDA_SDFIFOW_OUT_0                0x10E // Output Stream Descriptor 0 FIFO Eviction Watermark
#define R_HDA_SDFIFOW_OUT_1                0x12E // Output Stream Descriptor 1 FIFO Eviction Watermark
#define R_HDA_SDFIFOW_OUT_2                0x14E // Output Stream Descriptor 2 FIFO Eviction Watermark
#define R_HDA_SDFIFOW_OUT_3                0x16E // Output Stream Descriptor 3 FIFO Eviction Watermark
#define V_HDA_SDFIFOW_FIFOW_32B            0x0004 // 32 Bytes
#define V_HDA_SDFIFOW_FIFOW_64B            0x0005 // 64 Bytes

#define R_HDA_SDFIFOS_IN_0                 0x90  // Input Stream Descriptor 0 FIFO Size
#define R_HDA_SDFIFOS_IN_1                 0xB0  // Input Stream Descriptor 1 FIFO Size
#define R_HDA_SDFIFOS_IN_2                 0xD0  // Input Stream Descriptor 2 FIFO Size
#define R_HDA_SDFIFOS_IN_3                 0xF0  // Input Stream Descriptor 3 FIFO Size
#define R_HDA_SDFIFOS_OUT_0                0x110 // Output Stream Descriptor 0 FIFO Size
#define R_HDA_SDFIFOS_OUT_1                0x130 // Output Stream Descriptor 1 FIFO Size
#define R_HDA_SDFIFOS_OUT_2                0x150 // Output Stream Descriptor 2 FIFO Size
#define R_HDA_SDFIFOS_OUT_3                0x170 // Output Stream Descriptor 3 FIFO Size

#define R_HDA_SDFMT_IN_0                   0x92  // Input Stream Descriptor 0 Format
#define R_HDA_SDFMT_IN_1                   0xB2  // Input Stream Descriptor 1 Format
#define R_HDA_SDFMT_IN_2                   0xD2  // Input Stream Descriptor 2 Format
#define R_HDA_SDFMT_IN_3                   0xF2  // Input Stream Descriptor 3 Format
#define R_HDA_SDFMT_OUT_0                  0x112 // Output Stream Descriptor 0 Format
#define R_HDA_SDFMT_OUT_1                  0x132 // Output Stream Descriptor 1 Format
#define R_HDA_SDFMT_OUT_2                  0x152 // Output Stream Descriptor 2 Format
#define R_HDA_SDFMT_OUT_3                  0x172 // Output Stream Descriptor 3 Format
#define B_HDA_SDFMT_SBR                    BIT14 // Sample Base Rate
#define B_HDA_SDFMT_SBRM                   (BIT13 | BIT12 | BIT11) // Sample Base Rate Multiple
#define B_HDA_SDFMT_SBRD                   (BIT10 | BIT9 | BIT8) // Sample Base Rate Divisor
#define B_HDA_SDFMT_BITS                   (BIT6 | BIT5 | BIT4) // Bits Per Sample
#define B_HDA_SDFMT_CHAN                   (BIT3 | BIT2 | BIT1 | BIT0) // Number of Channels

#define R_HDA_SDBDPL_IN_0                  0x98  // Input Stream Descriptor 0 Buffer Descriptor List Pointer
#define R_HDA_SDBDPL_IN_1                  0xB8  // Input Stream Descriptor 1 Buffer Descriptor List Pointer
#define R_HDA_SDBDPL_IN_2                  0xD8  // Input Stream Descriptor 2 Buffer Descriptor List Pointer
#define R_HDA_SDBDPL_IN_3                  0xF8  // Input Stream Descriptor 3 Buffer Descriptor List Pointer
#define R_HDA_SDBDPL_OUT_0                 0x118 // Output Stream Descriptor 0 Buffer Descriptor List Pointer
#define R_HDA_SDBDPL_OUT_1                 0x138 // Output Stream Descriptor 1 Buffer Descriptor List Pointer
#define R_HDA_SDBDPL_OUT_2                 0x158 // Output Stream Descriptor 2 Buffer Descriptor List Pointer
#define R_HDA_SDBDPL_OUT_3                 0x178 // Output Stream Descriptor 3 Buffer Descriptor List Pointer
#define B_HDA_SDBDPL_LBA                   0xFFFFFF80 // Buffer Descriptor List Lower Base Address

#define R_HDA_SDBDPU_IN_0                  0x9C  // Input Stream Descriptor 0 Buffer Descriptor List Pointer
#define R_HDA_SDBDPU_IN_1                  0xBC  // Input Stream Descriptor 1 Buffer Descriptor List Pointer
#define R_HDA_SDBDPU_IN_2                  0xDC  // Input Stream Descriptor 2 Buffer Descriptor List Pointer
#define R_HDA_SDBDPU_IN_3                  0xFC  // Input Stream Descriptor 3 Buffer Descriptor List Pointer
#define R_HDA_SDBDPU_OUT_0                 0x11C // Output Stream Descriptor 0 Buffer Descriptor List Pointer
#define R_HDA_SDBDPU_OUT_1                 0x13C // Output Stream Descriptor 1 Buffer Descriptor List Pointer
#define R_HDA_SDBDPU_OUT_2                 0x15C // Output Stream Descriptor 2 Buffer Descriptor List Pointer
#define R_HDA_SDBDPU_OUT_3                 0x17C // Output Stream Descriptor 3 Buffer Descriptor List Pointer
#define B_HDA_SDBDPU_LBA                   0xFFFFFFFF // Buffer Descriptor List Upper Base Address

#endif
