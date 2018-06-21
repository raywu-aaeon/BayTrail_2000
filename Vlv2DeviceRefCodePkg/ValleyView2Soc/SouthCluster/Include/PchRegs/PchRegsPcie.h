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
  PchRegsPcie.h

  @brief
  Register names for VLV PCI-E root port devices
  
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

--*/
#ifndef _PCH_REGS_PCIE_H_
#define _PCH_REGS_PCIE_H_

#define PCH_PCIE_MAX_ROOT_PORTS                            4

///
/// VLV PCI Express Message Bus
///
#define PCH_PCIE_PHY_PORT_ID                               0xA6  // PCIe PHY Port ID
#define PCH_PCIE_PHY_MMIO_READ_OPCODE                      0x00  // CUnit to PCIe PHY MMIO Read Opcode
#define PCH_PCIE_PHY_MMIO_WRITE_OPCODE                     0x01  // CUnit to PCIe PHY MMIO Write Opcode

///
/// VLV PCI Express Root Ports (D28:F0~F3)
///
#define PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS              28
#define PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_1           0
#define PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_2           1
#define PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_3           2
#define PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_4           3

#define R_PCH_PCIE_ID                                      0x00  // Identifiers
#define B_PCH_PCIE_ID_DID                                  0xFFFF0000 // Device ID
#define V_PCH_PCIE_DEVICE_ID_0                             0x0F48  // PCIE Root Port #1
#define V_PCH_PCIE_DEVICE_ID_1                             0x0F4A  // PCIE Root Port #2
#define V_PCH_PCIE_DEVICE_ID_2                             0x0F4C  // PCIE Root Port #3
#define V_PCH_PCIE_DEVICE_ID_3                             0x0F4E  // PCIE Root Port #4
#define B_PCH_PCIE_ID_VID                                  0x0000FFFF // Vendor ID
#define V_PCH_PCIE_VENDOR_ID                               V_PCH_INTEL_VENDOR_ID

#define R_PCH_PCIE_CMD_PSTS                                0x04  // Device Command; Primary Status
#define S_PCH_PCIE_CMD_PSTS                                4
#define B_PCH_PCIE_CMD_PSTS_DPE                            BIT31 // Detected Parity Error
#define B_PCH_PCIE_CMD_PSTS_SSE                            BIT30 // Signaled System Error
#define B_PCH_PCIE_CMD_PSTS_RMA                            BIT29 // Received Master Abort
#define B_PCH_PCIE_CMD_PSTS_RTA                            BIT28 // Received Target Abort
#define B_PCH_PCIE_CMD_PSTS_STA                            BIT27 // Signaled Target Abort
#define B_PCH_PCIE_CMD_PSTS_DEV_STS                        (BIT26 | BIT25) // Primary DEVSEL# Timing Status
#define B_PCH_PCIE_CMD_PSTS_DPED                           BIT24 // Master Data Parity Error Detected
#define B_PCH_PCIE_CMD_PSTS_FB2BC                          BIT23 // Primary Fast Back to Back Capable
#define B_PCH_PCIE_CMD_PSTS_66MHZ_CAP                      BIT21 // Primary 66 MHz Capable
#define B_PCH_PCIE_CMD_PSTS_CAP_LST                        BIT20 // Capabilities List
#define B_PCH_PCIE_CMD_PSTS_INTR_STS                       BIT19 // Interrupt Status
#define B_PCH_PCIE_CMD_PSTS_ID                             BIT10 // Interrupt Disable
#define B_PCH_PCIE_CMD_PSTS_FBE                            BIT9  // Fast Back to Back Enable
#define B_PCH_PCIE_CMD_PSTS_SEE                            BIT8  // SERR# Enable
#define B_PCH_PCIE_CMD_PSTS_WCC                            BIT7  // Wait Cycle Control
#define B_PCH_PCIE_CMD_PSTS_PER                            BIT6  // Parity Error Response Enable
#define B_PCH_PCIE_CMD_PSTS_VPS                            BIT5  // VGA Palette Snoop
#define B_PCH_PCIE_CMD_PSTS_MWIE                           BIT4  // Memory Write and Invalidate Enable
#define B_PCH_PCIE_CMD_PSTS_SCE                            BIT3  // Special Cycle Enable
#define B_PCH_PCIE_CMD_PSTS_BME                            BIT2  // Bus Master Enable
#define B_PCH_PCIE_CMD_PSTS_MSE                            BIT1  // Memory Space Enable
#define B_PCH_PCIE_CMD_PSTS_IOSE                           BIT0  // I/O Space Enable

#define R_PCH_PCIE_RID_CC                                  0x08  // Revision ID; Class Code
#define B_PCH_PCIE_RID_CC_BCC                              0xFF000000 // Base Class Code
#define B_PCH_PCIE_RID_CC_SCC                              0x00FF0000 // Sub Class Code
#define B_PCH_PCIE_RID_CC_PI                               0x0000FF00 // Programming Interface
#define B_PCH_PCIE_RID_CC_RID                              0x000000FF // Revision ID

#define R_PCH_PCIE_CLS_PLT_HTYPE                           0x0C  // Cache Line size; Primary Latency Timer; Header Type
#define B_PCH_PCIE_CLS_PLT_HTYPE_MFD                       BIT23 // Multi-function Device
#define B_PCH_PCIE_CLS_PLT_HTYPE_HTYPE                     0x007F0000 // Header Type
#define B_PCH_PCIE_CLS_PLT_HTYPE_CT                        0x0000F800 // Latency Count
#define B_PCH_PCIE_CLS_PLT_HTYPE_LS                        0x000000FF // Line Size

#define R_PCH_PCIE_BNUM_SLT                                0x18  // Bus Numbers; Secondary Latency Timer
#define B_PCH_PCIE_BNUM_SLT_SLT                            0xFF000000 // Secondary Latency Timer
#define B_PCH_PCIE_BNUM_SLT_SBBN                           0x00FF0000 // Subordinate Bus Number
#define B_PCH_PCIE_BNUM_SLT_SCBN                           0x0000FF00 // Secondary Bus Number
#define B_PCH_PCIE_BNUM_SLT_PBN                            0x000000FF // Primary Bus Number

#define R_PCH_PCIE_IOBL_SSTS                               0x1C  // I/O Base and Limit; Secondary Status
#define B_PCH_PCIE_IOBL_SSTS_DPE                           BIT31 // Detected Parity Error
#define B_PCH_PCIE_IOBL_SSTS_RSE                           BIT30 // Received System Error
#define B_PCH_PCIE_IOBL_SSTS_RMA                           BIT29 // Received Master Abort
#define B_PCH_PCIE_IOBL_SSTS_RTA                           BIT28 // Received Target Abort
#define B_PCH_PCIE_IOBL_SSTS_STA                           BIT27 // Signaled Target Abort
#define B_PCH_PCIE_IOBL_SSTS_SDTS                          (BIT26 | BIT25) // Secondary DEVSEL# Timing Status
#define B_PCH_PCIE_IOBL_SSTS_DPD                           BIT24 // Data Parity Error Detected
#define B_PCH_PCIE_IOBL_SSTS_SFBC                          BIT23 // Secondary Fast Back to Back Capable
#define B_PCH_PCIE_IOBL_SSTS_SC66                          BIT22 // Secondary 66 MHz Capable
#define B_PCH_PCIE_IOBL_SSTS_IOLA                          0xF000 // I/O Address Limit
#define B_PCH_PCIE_IOBL_SSTS_IOLC                          0x0F00 // I/O Limit Address Capability
#define B_PCH_PCIE_IOBL_SSTS_IOBA                          0x00F0 // I/O Base Address
#define B_PCH_PCIE_IOBL_SSTS_IOBC                          0x000F // I/O Base Address Capability

#define R_PCH_PCIE_MBL                                     0x20  // Memory Base and Limit
#define B_PCH_PCIE_MBL_ML                                  0xFFF00000 // Memory Limit
#define B_PCH_PCIE_MBL_MB                                  0x0000FFF0 // Memory Base

#define R_PCH_PCIE_PMBL                                    0x24  // Prefetchable Memory Base and Limit
#define B_PCH_PCIE_PMBL_PML                                0xFFF00000 // Prefetchable Memory Limit
#define B_PCH_PCIE_PMBL_I64L                               0x000F0000 // 64-bit Indicator
#define B_PCH_PCIE_PMBL_PMB                                0x0000FFF0 // Prefetchable Memory Base
#define B_PCH_PCIE_PMBL_I64B                               0x0000000F // 64-bit Indicator

#define R_PCH_PCIE_PMBU32                                  0x28  // Prefetchable Memory Base Upper 32 Bits
#define B_PCH_PCIE_PMBU32                                  0xFFFFFFFF // Prefetchable Memory Base Upper Portion

#define R_PCH_PCIE_PMLU32                                  0x2C  // Prefetchable Memory Limit Upper 32 Bits
#define B_PCH_PCIE_PMLU32                                  0xFFFFFFFF // Prefetchable Memory Limit Upper Portion

#define R_PCH_PCIE_CAPP                                    0x34  // Capabilities List Pointer
#define B_PCH_PCIE_CAPP                                    0xFF  // Capabilities Pointer

#define R_PCH_PCIE_INTR_BCTRL                              0x3C  // Interrupt Information; Bridge Control
#define B_PCH_PCIE_INTR_BCTRL_DTSE                         BIT27 // Discard Timer SERR# Enable
#define B_PCH_PCIE_INTR_BCTRL_DTS                          BIT26 // Discard Timer Status
#define B_PCH_PCIE_INTR_BCTRL_SDT                          BIT25 // Secondary Discard Timer
#define B_PCH_PCIE_INTR_BCTRL_PDT                          BIT24 // Primary Discard Timer
#define B_PCH_PCIE_INTR_BCTRL_FBE                          BIT23 // Fast Back to Back Enable
#define B_PCH_PCIE_INTR_BCTRL_SBR                          BIT22 // Secondary Bus Reset
#define B_PCH_PCIE_INTR_BCTRL_MAM                          BIT21 // Master Abort Mode
#define B_PCH_PCIE_INTR_BCTRL_V16                          BIT20 // VGA 16-bit Decode
#define B_PCH_PCIE_INTR_BCTRL_VE                           BIT19 // VGA Enable
#define B_PCH_PCIE_INTR_BCTRL_IE                           BIT18 // ISA Enable
#define B_PCH_PCIE_INTR_BCTRL_SE                           BIT17 // SERR# Enable
#define B_PCH_PCIE_INTR_BCTRL_PERE                         BIT16 // Parity Error Response Enable
#define B_PCH_PCIE_INTR_BCTRL_IPIN                         0xFF00 // Interrupt Pin
#define B_PCH_PCIE_INTR_BCTRL_ILINE                        0x00FF // Interrupt Line

#define R_PCH_PCIE_CLIST_XCAP                              0x40  // Capabilities List; PCI Express Capabilities
#define B_PCH_PCIE_CLIST_XCAP_IMN                          0x3E000000 // Interrupt Message Number
#define B_PCH_PCIE_CLIST_XCAP_SI                           BIT24 // Slot Implemented
#define B_PCH_PCIE_CLIST_XCAP_DT                           0x00F00000 // Device / Port Type
#define B_PCH_PCIE_CLIST_XCAP_CV                           0x000F0000 // Capability Version
#define B_PCH_PCIE_CLIST_XCAP_NEXT                         0x0000FF00 // Next Capabilities
#define B_PCH_PCIE_CLIST_XCAP_CID                          0x000000FF // Capability ID

#define R_PCH_PCIE_DCAP                                    0x44  // Device Capabilities
#define S_PCH_PCIE_DCAP                                    4
#define B_PCH_PCIE_DCAP_CSPS                               0x0C000000 // Captured Slot Power Limit Scale (Not Supported)
#define B_PCH_PCIE_DCAP_CSPV                               0x03FC0000 // Captured Slot Power Limit Value (Not Supported)
#define B_PCH_PCIE_DCAP_RBER                               BIT15 // Role Based Error Reporting
#define B_PCH_PCIE_DCAP_PIP                                BIT14 // Reserved, previously was Power Indicator Present
#define B_PCH_PCIE_DCAP_AIP                                BIT13 // Reserved, previously was Attention Indicator Present
#define B_PCH_PCIE_DCAP_ABP                                BIT12 // Reserved, previously was Attention Button Present
#define B_PCH_PCIE_DCAP_E1AL                               0x00000E00 // Endpoint L1 Acceptable Latency
#define B_PCH_PCIE_DCAP_E0AL                               0x000001C0 // Endpoint L0 Acceptable Latency
#define B_PCH_PCIE_DCAP_ETFS                               BIT5  // Extended Tag Field Supported
#define B_PCH_PCIE_DCAP_PFS                                0x00000018 // Phantom Function Supported
#define B_PCH_PCIE_DCAP_MPS                                0x00000007 // Max Payload Size Supported

#define R_PCH_PCIE_DCTL_DSTS                               0x48  // Device Control; Device Status
#define S_PCH_PCIE_DCTL_DSTS                               4
#define B_PCH_PCIE_DCTL_DSTS_TDP                           BIT21 // Transactions Pending
#define B_PCH_PCIE_DCTL_DSTS_APD                           BIT20 // AUX Power Detected
#define B_PCH_PCIE_DCTL_DSTS_URD                           BIT19 // Unsupported Request Detected
#define B_PCH_PCIE_DCTL_DSTS_FED                           BIT18 // Fatal Error Detected
#define B_PCH_PCIE_DCTL_DSTS_NFED                          BIT17 // Non-Fatal Error Detected
#define B_PCH_PCIE_DCTL_DSTS_CED                           BIT16 // Correctable Error Detected
#define B_PCH_PCIE_DCTL_DSTS_MRRS                          0x7000 // Max Read Request Size
#define B_PCH_PCIE_DCTL_DSTS_ENS                           BIT11 // Enable No Snoop
#define B_PCH_PCIE_DCTL_DSTS_APME                          BIT10 // Aux Power PM Enable
#define B_PCH_PCIE_DCTL_DSTS_PFE                           BIT9  // Phantom Function Enable (Not Supported)
#define B_PCH_PCIE_DCTL_DSTS_ETFE                          BIT8  // Extended Tag Field Enable (Not Supported)
#define B_PCH_PCIE_DCTL_DSTS_MPS                           (BIT7 | BIT6 | BIT5) // Max Payload Size
#define B_PCH_PCIE_DCTL_DSTS_ERO                           BIT4 // Enable Relaxed Ordering (Not Supported)
#define B_PCH_PCIE_DCTL_DSTS_URE                           BIT3 // Unsupported Request Reporting Enable
#define B_PCH_PCIE_DCTL_DSTS_FEE                           BIT2 // Fatal Error Reporting Enable
#define B_PCH_PCIE_DCTL_DSTS_NFE                           BIT1 // Non-Fatal Error Reporting Enable
#define B_PCH_PCIE_DCTL_DSTS_CEE                           BIT0 // Correctable Error Reporting Enable

#define R_PCH_PCIE_LCAP                                    0x4C  // Link Capabilities
#define B_PCH_PCIE_LCAP_PN                                 0xFF000000 // Port Number
#define V_PCH_PCIE_LCAP_PN1                                (1 << 24) // Port Number 1
#define V_PCH_PCIE_LCAP_PN2                                (2 << 24) // Port Number 2
#define V_PCH_PCIE_LCAP_PN3                                (3 << 24) // Port Number 3
#define V_PCH_PCIE_LCAP_PN4                                (4 << 24) // Port Number 4
#define V_PCH_PCIE_LCAP_PN5                                (5 << 24) // Port Number 5
#define V_PCH_PCIE_LCAP_PN6                                (6 << 24) // Port Number 6
#define V_PCH_PCIE_LCAP_PN7                                (7 << 24) // Port Number 7
#define V_PCH_PCIE_LCAP_PN8                                (8 << 24) // Port Number 8
#define B_PCH_PCIE_LCAP_LBNC                               BIT21 // Link Bandwidth Notification Capability
#define B_PCH_PCIE_LCAP_LARC                               BIT20 // Link Active Reporting Capable
#define B_PCH_PCIE_LCAP_SDERC                              BIT19 // Surprise Down Error Reporting Capable
#define B_PCH_PCIE_LCAP_CPM                                BIT18 // Clock Power Management
#define B_PCH_PCIE_LCAP_EL1                                (BIT17 | BIT16 | BIT15) // L1 Exit Latency
#define B_PCH_PCIE_LCAP_EL0                                (BIT14 | BIT13 | BIT12) // L0s Exit Latency
#define B_PCH_PCIE_LCAP_APMS                               (BIT11 | BIT10) // Active State Link PM Support
#define V_PCH_PCIE_LCAP_APMS_L0S                           (1 << 10) // L0s Entry Supported
#define V_PCH_PCIE_LCAP_APMS_L0S_L1                        (3 << 10) // Both L0s and L1 Supported
#define B_PCH_PCIE_LCAP_MLW                                0x000003F0 // Maximum Link Width
#define B_PCH_PCIE_LCAP_SLS                                0x0000000F // Supported Link Speeds

#define R_PCH_PCIE_LCTL_LSTS                               0x50  // Link Control; Link Status
#define B_PCH_PCIE_LCTL_LSTS_LABS                          BIT31 // Link Autonomous Bandwidth Status
#define B_PCH_PCIE_LCTL_LSTS_LBMS                          BIT30 // Link Bandwidth Management Status
#define B_PCH_PCIE_LCTL_LSTS_DLLA                          BIT29 // Link Active
#define B_PCH_PCIE_LCTL_LSTS_SCC                           BIT28 // Slot Clock Configuration
#define B_PCH_PCIE_LCTL_LSTS_LT                            BIT27 // Link Training
#define B_PCH_PCIE_LCTL_LSTS_LTE                           BIT26 // Reserved, previously was Link Training Error
#define B_PCH_PCIE_LCTL_LSTS_NLW                           0x03F00000 // Negotiated Link Width
#define V_PCH_PCIE_LCTL_LSTS_NLW_1                         0x00100000
#define V_PCH_PCIE_LCTL_LSTS_NLW_2                         0x00200000
#define V_PCH_PCIE_LCTL_LSTS_NLW_4                         0x00400000
#define B_PCH_PCIE_LCTL_LSTS_LS                            0x000F0000 // Current Link Speed
#define B_PCH_PCIE_LCTL_LSTS_LABIE                         BIT11 // Link Autonomous Bandwidth Interrupt Enable
#define B_PCH_PCIE_LCTL_LSTS_LBMIE                         BIT10 // Link Bandwidth Management Interrupt Enable
#define B_PCH_PCIE_LCTL_LSTS_HAWD                          BIT9  // Hardware Autonomous Width Disable
#define B_PCH_PCIE_LCTL_LSTS_ES                            BIT7  // Extended Synch
#define B_PCH_PCIE_LCTL_LSTS_CCC                           BIT6  // Common Clock Configuration
#define B_PCH_PCIE_LCTL_LSTS_RL                            BIT5  // Retrain Link
#define B_PCH_PCIE_LCTL_LSTS_LD                            BIT4  // Link Disable
#define B_PCH_PCIE_LCTL_LSTS_RCBC                          BIT3  // Read Completion Boundary
#define B_PCH_PCIE_LCTL_LSTS_ASPM                          (BIT1 | BIT0) // Active State Link PM Control
#define V_PCH_PCIE_LCTL_LSTS_ASPM_L0S                      1     // L0s Entry Enabled
#define V_PCH_PCIE_LCTL_LSTS_ASPM_L1                       2     // L1 Entry Enable
#define V_PCH_PCIE_LCTL_LSTS_ASPM_L0S_L1                   3     // L0s and L1 Entry Enabled

#define R_PCH_PCIE_SLCAP                                   0x54  // Slot Capabilities
#define S_PCH_PCIE_SLCAP                                   4
#define B_PCH_PCIE_SLCAP_PSN                               0xFFF80000 // Physical Slot Number
#define N_PCH_PCIE_SLCAP_PSN                               19
#define B_PCH_PCIE_SLCAP_NCCS                              BIT18 // No Command Completed Support
#define B_PCH_PCIE_SLCAP_EMIP                              BIT17 // Electromechanical Interlock Present
#define B_PCH_PCIE_SLCAP_SLS                               (BIT16 | BIT15) // Slot Power Limit Scale
#define B_PCH_PCIE_SLCAP_SLV                               0x00007F80 // Slot Power Limit Value
#define B_PCH_PCIE_SLCAP_HPC                               BIT6  // Hot Plug Capable
#define B_PCH_PCIE_SLCAP_HPS                               BIT5  // Hot Plug Surprise
#define B_PCH_PCIE_SLCAP_PIP                               BIT4  // Power Indicator Present
#define B_PCH_PCIE_SLCAP_AIP                               BIT3  // Attention Indicator Present
#define B_PCH_PCIE_SLCAP_MSP                               BIT2  // MRL Sensor Present
#define B_PCH_PCIE_SLCAP_PCP                               BIT1  // Power Controller Present
#define B_PCH_PCIE_SLCAP_ABP                               BIT0  // Attention Buttion Present

#define R_PCH_PCIE_SLCTL_SLSTS                             0x58  // Slot Control; Slot Status
#define S_PCH_PCIE_SLCTL_SLSTS                             4
#define B_PCH_PCIE_SLCTL_SLSTS_DLLSC                       BIT24 // Data Link Layer State Changed
#define B_PCH_PCIE_SLCTL_SLSTS_PDS                         BIT22 // Presence Detect State
#define B_PCH_PCIE_SLCTL_SLSTS_MS                          BIT21 // MRL Sensor State
#define B_PCH_PCIE_SLCTL_SLSTS_PDC                         BIT19 // Presence Detect Changed
#define B_PCH_PCIE_SLCTL_SLSTS_MSC                         BIT18 // MRL Sensor Changed
#define B_PCH_PCIE_SLCTL_SLSTS_PFD                         BIT17 // Power Fault Detected
#define B_PCH_PCIE_SLCTL_SLSTS_DLLSCE                      BIT12 // Data Link Layer State Changed Enable
#define B_PCH_PCIE_SLCTL_SLSTS_PCC                         BIT10 // Power Controller Control
#define B_PCH_PCIE_SLCTL_SLSTS_HPE                         BIT5  // Hot Plug Interrupt Enable
#define B_PCH_PCIE_SLCTL_SLSTS_CCE                         BIT4  // Command Completed Interrupt Enable
#define B_PCH_PCIE_SLCTL_SLSTS_PDE                         BIT3  // Presence Detect Changed Enable

#define R_PCH_PCIE_RCTL                                    0x5C  // Root Control
#define S_PCH_PCIE_RCTL                                    2
#define B_PCH_PCIE_RCTL_PIE                                BIT3  // PME Interrupt Enable
#define B_PCH_PCIE_RCTL_SFE                                BIT2  // System Error on Fatal Error Enable
#define B_PCH_PCIE_RCTL_SNE                                BIT1  // System Error on Non-Fatal Error Enable
#define B_PCH_PCIE_RCTL_SCE                                BIT0  // System Error on Correctable Error Enable

#define R_PCH_PCIE_RSTS                                    0x60  // Root Status
#define S_PCH_PCIE_RSTS                                    4
#define B_PCH_PCIE_RSTS_PP                                 BIT17 // PME PEnding
#define B_PCH_PCIE_RSTS_PS                                 BIT16 // PME Status
#define B_PCH_PCIE_RSTS_RID                                0x0000FFFF // PME Requestor ID

#define R_PCH_PCIE_DCAP2                                   0x64  // Device Capabilities 2
#define B_PCH_PCIE_DCAP2_OBFFS                             (BIT19 | BIT18) // Optimized Buffer Flush / Fill Supported
#define B_PCH_PCIE_DCAP2_LTRMS                             BIT11 // LTR Mechanism Supported
#define B_PCH_PCIE_DCAP2_CTDS                              BIT4  // Completion Timeout Disable Supported
#define B_PCH_PCIE_DCAP2_CTRS                              0xF   // Completion Timeout Ranges Supported
#define V_PCH_PCIE_DCAP2_CTRS_UNSUPPORTED                  0x0
#define V_PCH_PCIE_DCAP2_CTRS_RANGE_A                      0x1   // 50 us to 10 ms
#define V_PCH_PCIE_DCAP2_CTRS_RANGE_B                      0x2   // 10 ms to 250 ms
#define V_PCH_PCIE_DCAP2_CTRS_RANGE_C                      0x4   // 250 ms to 4 s
#define V_PCH_PCIE_DCAP2_CTRS_RANGE_D                      0x8   // 4 s to 64 s

#define R_PCH_PCIE_DCTL2_DSTS2                             0x68  // Device Control 2; Device Status 2
#define B_PCH_PCIE_DCTL2_DSTS2_OBFFEN                      (BIT14 | BIT13) // Optimized Buffer Flush / Fill Enable
#define B_PCH_PCIE_DCTL2_DSTS2_LTRME                       BIT10 // LTR Mechanism Enable
#define B_PCH_PCIE_DCTL2_DSTS2_CTD                         BIT4  // Completion Timeout Disable
#define B_PCH_PCIE_DCTL2_DSTS2_CTV                         0xF   // Completion Timeout Value
#define V_PCH_PCIE_DCTL2_DSTS2_CTV_DEFAULT                 0x0
#define V_PCH_PCIE_DCTL2_DSTS2_CTV_40MS_50MS               0x5
#define V_PCH_PCIE_DCTL2_DSTS2_CTV_160MS_170MS             0x6
#define V_PCH_PCIE_DCTL2_DSTS2_CTV_400MS_500MS             0x9
#define V_PCH_PCIE_DCTL2_DSTS2_CTV_1P6S_1P7S               0xA

#define R_PCH_PCIE_LCTL2_LSTS2                             0x70  // Link Control 2; Link Status 2
#define B_PCH_PCIE_LCTL2_LSTS2_CDL                         BIT16 // Current De-emphasis Level
#define B_PCH_PCIE_LCTL2_LSTS2_CD                          BIT12 // Compliance De-emphasis
#define B_PCH_PCIE_LCTL2_LSTS2_CSOS                        BIT11 // Compliance SOS
#define B_PCH_PCIE_LCTL2_LSTS2_EMC                         BIT10 // Enter Modified Compliance
#define B_PCH_PCIE_LCTL2_LSTS2_TM                          (BIT9 | BIT8 | BIT7) // Transmit Margin
#define B_PCH_PCIE_LCTL2_LSTS2_SD                          BIT6  // Selectable De-emphasis
#define B_PCH_PCIE_LCTL2_LSTS2_HASD                        BIT5  // Reserved. Hardware Autonomous Speed Disable
#define B_PCH_PCIE_LCTL2_LSTS2_EC                          BIT4  // Enter Compliance
#define B_PCH_PCIE_LCTL2_LSTS2_TLS                         (BIT3 | BIT2 | BIT1 | BIT0) // Target Link Speed

#define R_PCH_PCIE_MID_MC                                  0x80  // Message Signaled Interrupt Identifiers; Message Signaled Interrupt Message Control
#define S_PCH_PCIE_MID_MC                                  4
#define B_PCH_PCIE_MID_MC_C64                              BIT23 // 64 Bit Address Capable
#define B_PCH_PCIE_MID_MC_MME                              (BIT22 | BIT21 | BIT20) // Multiple Message Enable
#define B_PCH_PCIE_MID_MC_MMC                              0x000E0000 // Multiple Message Capable
#define B_PCH_PCIE_MID_MC_MSIE                             BIT16 // MSI Enable
#define B_PCH_PCIE_MID_MC_NEXT                             0xFF00 // Next Pointer
#define B_PCH_PCIE_MID_MC_CID                              0x00FF // Capability ID

#define R_PCH_PCIE_MA                                      0x84  // Message Signaled Interrupt Message Address
#define S_PCH_PCIE_MA                                      4
#define B_PCH_PCIE_MA_ADDR                                 0xFFFFFFFC // Address

#define R_PCH_PCIE_MD                                      0x88  // Message Signaled Interrupt Message data
#define S_PCH_PCIE_MD                                      2
#define B_PCH_PCIE_MD_DATA                                 0xFFFF // Data

#define R_PCH_PCIE_SVCAP                                   0x90  // Subsystem Vendor Capability
#define S_PCH_PCIE_SVCAP                                   2
#define B_PCH_PCIE_SVCAP_NEXT                              0xFF00 // Next Capability
#define B_PCH_PCIE_SVCAP_CID                               0x00FF // Capability Identifier

#define R_PCH_PCIE_SVID                                    0x94  // Subsystem Vendor IDs
#define S_PCH_PCIE_SVID                                    4
#define B_PCH_PCIE_SVID_SID                                0xFFFF0000 // Subsystem Identifier
#define B_PCH_PCIE_SVID_SVID                               0x0000FFFF // Subsystem Vendor Identifier

#define R_PCH_PCIE_PMCAP_PMC                               0xA0  // Power Management Capability; PCI Power Management Capabilities
#define S_PCH_PCIE_PMCAP_PMC                               4
#define B_PCH_PCIE_PMCAP_PMC_PMES                          0xF8000000 // PME Support
#define B_PCH_PCIE_PMCAP_PMC_D2S                           BIT26 // D2 Support
#define B_PCH_PCIE_PMCAP_PMC_D1S                           BIT25 // D1 Support
#define B_PCH_PCIE_PMCAP_PMC_AC                            0x01C00000 // Aux Current
#define B_PCH_PCIE_PMCAP_PMC_DSI                           BIT21 // Device Specific Initialization
#define B_PCH_PCIE_PMCAP_PMC_PMEC                          BIT19 // PME Clock
#define B_PCH_PCIE_PMCAP_PMC_VS                            0x00070000 // Version
#define B_PCH_PCIE_PMCAP_PMC_NEXT                          0x0000FF00 // Next Capability
#define B_PCH_PCIE_PMCAP_PMC_CID                           0x000000FF // Capability Identifier

#define R_PCH_PCIE_PMCS                                    0xA4  // PCI Power Management Control and Status
#define S_PCH_PCIE_PMCS                                    4
#define B_PCH_PCIE_PMCS_BPCE                               BIT23 // Bus Power / Clock Control Enable
#define B_PCH_PCIE_PMCS_B23S                               BIT22 // B2 / B3 Support
#define B_PCH_PCIE_PMCS_PMES                               BIT15 // PME Status
#define B_PCH_PCIE_PMCS_PMEE                               BIT8  // PME Enable
#define B_PCH_PCIE_PMCS_NSR                                BIT3  // No Soft Reset
#define B_PCH_PCIE_PMCS_PS                                 (BIT1 | BIT0) // Power State
#define V_PCH_PCIE_PMCS_D0                                 0x00  // D0 State
#define V_PCH_PCIE_PMCS_D3H                                0x03  // D3 Hot State

#define R_PCH_PCIE_CHCFG                                   0xD0  // Channel Configuration
#define B_PCH_PCIE_CHCFG_CRE                               BIT31 // Config Received Enable
#define B_PCH_PCIE_CHCFG_IORE                              BIT30 // I/O Received Enable
#define B_PCH_PCIE_CHCFG_UPSD                              BIT24 // Upstream Posted Split Disable
#define B_PCH_PCIE_CHCFG_UNSD                              BIT23 // Upstream Non-Posted Split Disable
#define B_PCH_PCIE_CHCFG_NPAS                              0x007C0000 // Non-Posted Pre-Allocation Size
#define B_PCH_PCIE_CHCFG_DCGEISMA                          BIT17 // Dynamic Clock Gating Enable on ISM Active
#define B_PCH_PCIE_CHCFG_NPAP                              BIT16 // Non-Posted Pre-Allocation Policy
#define B_PCH_PCIE_CHCFG_UNRS                              BIT15 // Upstream Non-Posted Request Size
#define B_PCH_PCIE_CHCFG_UPRS                              BIT14 // Upstream Posted Request Size
#define B_PCH_PCIE_CHCFG_UNRD                              (BIT13 | BIT12) // Upstream Non-Posted Request Delay
#define B_PCH_PCIE_CHCFG_RBMS                              BIT11 // Retry Buffers Minimum Size
#define B_PCH_PCIE_CHCFG_MRNPC                             BIT9  // Minimum Receive Non-Posted Credits
#define B_PCH_PCIE_CHCFG_MRPC                              BIT8  // Minimum Receive Posted Credits

#define R_PCH_PCIE_MPC2                                    0xD4  // Miscellaneous Port Configuration 2
#define S_PCH_PCIE_MPC2                                    4
#define B_PCH_PCIE_MPC2_PTNFAE                             BIT12 // Poisoned TLP Non-Fatal Advisory Error Enable
#define B_PCH_PCIE_MPC2_IPF                                BIT11 // IOSF Packet Fast Transmit Mode
#define B_PCH_PCIE_MPC2_ITCUM                              BIT10 // IOSF Transaction Credit Update Mode
#define B_PCH_PCIE_MPC2_TLPF                               BIT9  // Transaction Layer Packet Fast Transmit Mode
#define B_PCH_PCIE_MPC2_CAM                                BIT8  // PCIe Credit Allocated Update Mode
#define B_PCH_PCIE_MPC2_CDRM                               BIT7  // COM De-Skew Recovery Mechanism
#define B_PCH_PCIE_MPC2_LSTP                               BIT6  // Link Speed Training Policy
#define B_PCH_PCIE_MPC2_IEIME                              BIT5  // Infer Electrical Idle Mechanism Enable
#define B_PCH_PCIE_MPC2_ASPMCOEN                           BIT4  // ASPM Control Override Enable
#define B_PCH_PCIE_MPC2_ASPMCO                             (BIT3 | BIT2) // APSM Control OverrideEOI Forwarding Disable
#define V_PCH_PCIE_MPC2_ASPMCO_DISABLED                    0     // Disabled
#define V_PCH_PCIE_MPC2_ASPMCO_L0S                         (1 << 2) // L0s Entry Enabled
#define V_PCH_PCIE_MPC2_ASPMCO_L1                          (2 << 2) // L1 Entry Enabled
#define V_PCH_PCIE_MPC2_ASPMCO_L0S_L1                      (3 << 2) // L0s and L1 Entry Enabled
#define B_PCH_PCIE_MPC2_EOIFD                              BIT1  // EOI Forwarding Disable
#define B_PCH_PCIE_MPC2_L1CTM                              BIT0  // L1 Completion Timeout Mode

#define R_PCH_PCIE_MPC                                     0xD8  // Miscellaneous Port Configuration
#define S_PCH_PCIE_MPC                                     4
#define B_PCH_PCIE_MPC_PMCE                                BIT31 // Power Management SCI Enable
#define B_PCH_PCIE_MPC_HPCE                                BIT30 // Hot Plug SCI Enable
#define B_PCH_PCIE_MPC_LHO                                 BIT29 // Link Hold Off
#define B_PCH_PCIE_MPC_ATE                                 BIT28 // Address Translator Enable
#define B_PCH_PCIE_MPC_MMBNCE                              BIT27 // MCTP MEssage Bus Number Check Enable
#define B_PCH_PCIE_MPC_IRBNCE                              BIT26 // Invalid Received Bus Number Check Enable
#define B_PCH_PCIE_MPC_IRRCE                               BIT25 // Invalid Received Range Check Enable
#define B_PCH_PCIE_MPC_BMERCE                              BIT24 // BME Received Check Enable
#define B_PCH_PCIE_MPC_SRL                                 BIT23 // Secured Register Lock
#define B_PCH_PCIE_MPC_FORCEDET                            BIT22 // Detect Override
#define B_PCH_PCIE_MPC_FCDL1E                              BIT21 // Flow Control During L1 Entry
#define B_PCH_PCIE_MPC_UCEL                                (BIT20 | BIT19 | BIT18) // Unique Clock Exit Latency
#define B_PCH_PCIE_MPC_CCEL                                (BIT17 | BIT16 | BIT15) // Common Clock Exit Latency
#define B_PCH_PCIE_MPC_PCIEGEN2DIS                         BIT14 // PCIe Gen2 Speed Disable
#define B_PCH_PCIE_MPC_MCTPBOP                             BIT13 // MCTP Bus Owner Policy
#define B_PCH_PCIE_MPC_RATS                                BIT12 // Reject ATS
#define B_PCH_PCIE_MPC_AT                                  (BIT11 | BIT10 | BIT9 | BIT8) // Address Translator
#define B_PCH_PCIE_MPC_PAE                                 BIT7  // Port I/OxApic Enable
#define B_PCH_PCIE_MPC_FCP                                 (BIT6 | BIT5 | BIT4) // Flow Control Update Policy
#define B_PCH_PCIE_MPC_MCTPSE                              BIT3  // MCTP Support Enable
#define B_PCH_PCIE_MPC_BT                                  BIT2  // Bridge Type
#define B_PCH_PCIE_MPC_HPME                                BIT1  // Hot Plug SMI Enable
#define N_PCH_PCIE_MPC_HPME                                1
#define B_PCH_PCIE_MPC_PMME                                BIT0  // Power Management SMI Enable

#define R_PCH_PCIE_SMSCS                                   0xDC  // SMI / SCI Status
#define S_PCH_PCIE_SMSCS                                   4
#define B_PCH_PCIE_SMSCS_PMCS                              BIT31 // Power Management SCI Status
#define B_PCH_PCIE_SMSCS_HPCS                              BIT30 // Hot Plug SCI Status
#define B_PCH_PCIE_SMSCS_HPLAS                             BIT4  // Hot Plug Link Active State Changed SMI Status
#define N_PCH_PCIE_SMSCS_HPLAS                             4
#define B_PCH_PCIE_SMSCS_HPCCM                             BIT3  // Hot Plug Command Completed SMI Status
#define B_PCH_PCIE_SMSCS_HPABM                             BIT2  // Hot Plug Attention Button SMI Status
#define B_PCH_PCIE_SMSCS_HPPDM                             BIT1  // Hot Plug Presence Detect SMI Status
#define N_PCH_PCIE_SMSCS_HPPDM                             1
#define B_PCH_PCIE_SMSCS_PMMS                              BIT0  // Power Management SMI Status

#define R_PCH_PCIE_RWC_RPDCGEN_RPPGEN                      0xE0  // Resume Well Control; Root Port Dynamic Clock Gate Enable; Root Port Power Gating Enable
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_PTOTOP               BIT22
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_L23R2DT              BIT19 // L23 Ready to Detect State
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_L23ER                BIT18
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_RPPGM                BIT17 // Root Port Power Gating Mode
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_RPPGEN               BIT16 // Root Port Power Gate Enable
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_RPSCGEN              BIT15 // PCIe Root Port Static Clock Gate Enable
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_LCLKREQEN            BIT13 // PCIe Link CLKREQ Enable
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_BBCLKREQEN           BIT12 // PCIe Backbone CLKREQ Enable
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_SRDLCGEN             BIT11 // Shared Resource Dynamic Link Clock Gate Enable
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_SRDBCGEN             BIT10 // Shared Resource Dynamic Backbone Clock Gate Enable
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_RPDLCGEN             BIT9  // Root Port Dynamic Link Clock Gate Enable
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_RPDBCGEN             BIT8  // Root Port Dynamic Backbone Clock Gate Enable
#define B_PCH_PCIE_RWC_RPDCGEN_RPPGEN_RCB                  (BIT3 | BIT2 | BIT1) // Resume Chicken Bits

#define R_PCH_PCIE_LTRL                                    0xE4  // LTR Latency
#define B_PCH_PCIE_LTRL_LTRNSLF                            0xFFFF0000 // LTR No-Snoop Latency Field
#define B_PCH_PCIE_LTRL_LTRSLF                             0x0000FFFF // LTR Snoop Latency Field

#define R_PCH_PCIE_PWRCTL                                  0xE8  // Power Control
#define S_PCH_PCIE_PWRCTL                                  4
#define B_PCH_PCIE_PWRCTL_DBUPI                            BIT15 // De-skew Buffer Unloaded Pointer Increment
#define B_PCH_PCIE_PWRCTL_ILSVRX                           BIT14 // Invalid LTR Latency Scale Value Received
#define B_PCH_PCIE_PWRCTL_TXSWING                          BIT13 // Analog PHY Transmitter Voltage Swing
#define B_PCH_PCIE_PWRCTL_HRBF                             BIT10 // Hot Removal Bug Fix
#define B_PCH_PCIE_PWRCTL_SELAQ                            BIT9  // Squelch Exit Lane Active
#define B_PCH_PCIE_PWRCTL_RPTFSMWI                         BIT8  // Reset Progress Tracker FSM When Idle
#define B_PCH_PCIE_PWRCTL_SCTEM                            BIT7  // Synthesized Completion Termination Encoding Masking
#define B_PCH_PCIE_PWRCTL_PCSCEIOST                        BIT6  // Polling Compliance Speed Change EIOS Transmission
#define B_PCH_PCIE_PWRCTL_ISCT                             BIT5  // Invalid Speed Change Transition
#define B_PCH_PCIE_PWRCTL_LTRR                             BIT4  // LTR Received
#define B_PCH_PCIE_PWRCTL_RPSEWL                           (BIT3 | BIT2) // Root Port Squelch Exit Wait Latency
#define B_PCH_PCIE_PWRCTL_RPL1SQPOL                        BIT1  // Root Port L1 Squelch Polling
#define B_PCH_PCIE_PWRCTL_RPDTSQPOL                        BIT0  // Root Port Detect Squelch Polling

#define R_PCH_PCIE_DC                                      0xEC  // Decode Control
#define B_PCH_PCIE_DC_ECD                                  BIT3  // Extended Config Disable
#define R_PCH_PCIE_DC_PCIBEM                               BIT2  // PCI Bus Emulation Mode
#define R_PCH_PCIE_DC_SDCDID                               BIT1  // Subtractive Decode Compatibility Device ID
#define R_PCH_PCIE_DC_SDE                                  BIT0  // Subtractive Decode Enable

#define R_PCH_PCIE_IECS                                    0xF0  // IOSF Primary Control And Status
#define B_PCH_PCIE_IECS_PRIC                               (BIT14 | BIT13 | BIT12) // IOSF Primary ISM Idle Counter
#define B_PCH_PCIE_IECS_IMPS                               (BIT10 | BIT9 | BIT8) // IOSF Max Payload Size
#define B_PCH_PCIE_IECS_IMRS                               (BIT6 | BIT5 | BIT4) // IOSF Max Read Request Size
#define B_PCH_PCIE_IECS_URD                                BIT1  // Unsupported Request Detect
#define B_PCH_PCIE_IECS_URRE                               BIT0  // Unsupported Request Reporting Enable

#define R_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL                0xF4  // Physical Layer and AFE Control; Physical Layer and AFE Control 2; Physical Layer and AFE Control 3; IOSF Sideband Control and Status
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_ISFSIDED       BIT29 // IOSF Sideband Fuse / Strap / ID Distribution Error Detected
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_ISURD          BIT28 // IOSF Sideband Unsupported Request Detected
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_SIID           (BIT27 | BIT26) // IOSF Sideband Interface Idle Counter
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_SBIC           (BIT25 | BIT24) // IOSF Sideband ISM Idle Counter
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_PLLACBGD       BIT17 // PLL Abutment Clock Buffer Gate Disable
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_TXHSDEEMPCTL   BIT16 // TX Half Swing Deemphasis Control
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_TDFT           (BIT15 | BIT14) // Transmit Datapath Flush Timer
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_TXCFGCHGWAIT   (BIT13 | BIT12) // Transmit Configuration Change Wait Time
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_PXPPLLWAIT     (BIT11 | BIT10 | BIT9) // PCI Express PLL Wait
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_PXPPLLOFFEN    BIT8  // PCI Express PLL Off Enable
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_PSE            BIT7  // Port Stagger Enable
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_LSD            (BIT6 | BIT5) // Lane Stagger Delay
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_PORTSTGWAITDIS BIT4  // Port Staggering Wait State Disable
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_STGCFG         (BIT3 | BIT2) // Staggering Configuration
#define B_PCH_PCIE_PHYCTL_PHYCTL2_IOSFSBCTL_SDM            (BIT1 | BIT0) // Stagger Delay Multiplier

#define R_PCH_PCIE_MANID                                   0xF8  // Manufacturer's ID
#define B_PCH_PCIE_MANID_DPID                              0x0F000000 // Dot Portion of Process ID
#define B_PCH_PCIE_MANID_SID                               0x00FF0000 // Stepping Identifier
#define B_PCH_PCIE_MANID_MID                               0x0000FF00 // Manufacturing Identifier
#define B_PCH_PCIE_MANID_PD                                0x000000FF // Process / Dot

#define R_PCH_PCIE_STRPFUSECFG                             0xFC  // Strap and Fuse Configuration
#define B_PCH_PCIE_STRPFUSECFG_USB3PCIEP2MODE              (BIT31 | BIT30) // USB3 PCIe Port 2 Mode
#define B_PCH_PCIE_STRPFUSECFG_SERM                        BIT29 // Server Error Reporting Mode
#define B_PCH_PCIE_STRPFUSECFG_FSTRST                      BIT28 // Fast Reset Mode Strap
#define B_PCH_PCIE_STRPFUSECFG_PXIP                        (BIT27 | BIT26 | BIT25 | BIT24) // PCI Express Interrupt Pin
#define B_PCH_PCIE_STRPFUSECFG_SATAP5PCIEP2MODE            (BIT23 | BIT22) // SATA Port 5 PCIe Port 2 Mode Strap
#define B_PCH_PCIE_STRPFUSECFG_SATAP4PCIEP1MODE            (BIT21 | BIT20) // SATA Port 4 PCIe Port 1 Mode Strap
#define B_PCH_PCIE_STRPFUSECFG_GBEPCIEPEN                  BIT19 // GbE Over PCI Express Port Enable Strap
#define B_PCH_PCIE_STRPFUSECFG_GBEPCIEPORTSEL              (BIT18 | BIT17 | BIT16) // GbE PCIe Port Select Strap
#define B_PCH_PCIE_STRPFUSECFG_RPC                         (BIT15 | BIT14) // Root Port Configuration Strap
#define B_PCH_PCIE_STRPFUSECFG_SATAP5PCIEP2MODEFUSE        (BIT13 | BIT12) // SATA Port 5 PCIe Port 2 Mode Fuse
#define B_PCH_PCIE_STRPFUSECFG_SATAP4PCIEP1MODEFUSE        (BIT11 | BIT10) // SAPA Port 4 PCIe Port 1 Mode Fuse
#define B_PCH_PCIE_STRPFUSECFG_MPHYIOPMDIS                 BIT9  // mPHY I/O PM Disable Fuse
#define B_PCH_PCIE_STRPFUSECFG_PLLSHTDWNDIS                BIT8  // PLL Shut Down Disable Fuse
#define B_PCH_PCIE_STRPFUSECFG_STPGATEDIS                  BIT7  // Static Power Gating Disable Fuse
#define B_PCH_PCIE_STRPFUSECFG_ASPMDIS                     BIT6  // ASPM Disable Fuse
#define B_PCH_PCIE_STRPFUSECFG_LDCGDIS                     BIT5  // Link Dynamic Clock Gating Disable Fuse
#define B_PCH_PCIE_STRPFUSECFG_LTCGDIS                     BIT4  // Link Trunk Clock Gating Disable Fuse
#define B_PCH_PCIE_STRPFUSECFG_BDCGDIS                     BIT3  // Backbone Dynamic Clock Gating Disable Fuse
#define B_PCH_PCIE_STRPFUSECFG_DESKTOPMOB                  BIT2  // Desktop or Mobile Fuse
#define B_PCH_PCIE_STRPFUSECFG_USB3PCIEPORT1MODE           (BIT1 | BIT0) // USB3 PCIe Port 1 Mode

#define R_PCH_PCIE_AECH                                    0x100 // Advanced Error Reporting Capability Header
#define B_PCH_PCIE_AECH_NCO                                0xFFF00000 // Next Capability Offset
#define B_PCH_PCIE_AECH_CV                                 0x000F0000 // Capability Version
#define B_PCH_PCIE_AECH_CID                                0x0000FFFF // Capability ID

#define R_PCH_PCIE_UES                                     0x104 // Uncorrectable Error Status
#define S_PCH_PCIE_UES                                     4
#define B_PCH_PCIE_UES_URE                                 BIT20 // Unsupported Request Error Status
#define B_PCH_PCIE_UES_EE                                  BIT19 // ECRC Error Status
#define B_PCH_PCIE_UES_MT                                  BIT18 // Malformed TLP Status
#define B_PCH_PCIE_UES_RO                                  BIT17 // Receiver Overflow Status
#define B_PCH_PCIE_UES_UC                                  BIT16 // Unexpected Completion Status
#define B_PCH_PCIE_UES_CA                                  BIT15 // Completer Abort Status
#define B_PCH_PCIE_UES_CT                                  BIT14 // Completion Timeout Status
#define B_PCH_PCIE_UES_FCPE                                BIT13 // Flow Control Protocol Error Status (Not Supported)
#define B_PCH_PCIE_UES_PT                                  BIT12 // Poisoned TLP Status
#define B_PCH_PCIE_UES_DLPE                                BIT4  // DataLink Protocol Error Status
#define B_PCH_PCIE_UES_TE                                  BIT0  // Training Error Status (Not Supported)

#define R_PCH_PCIE_UEM                                     0x108 // Uncorrectable Error Mask
#define S_PCH_PCIE_UEM                                     4
#define B_PCH_PCIE_UEM_URE                                 BIT20 // Unsupported Request Error Mask
#define B_PCH_PCIE_UEM_EE                                  BIT19 // ECRC Error Mask
#define B_PCH_PCIE_UEM_MT                                  BIT18 // Malformed TLP Mask
#define B_PCH_PCIE_UEM_RO                                  BIT17 // Receiver Overflow Mask
#define B_PCH_PCIE_UEM_UC                                  BIT16 // Unexpected Completion Mask
#define B_PCH_PCIE_UEM_CA                                  BIT15 // Completer Abort Mask
#define B_PCH_PCIE_UEM_CT                                  BIT14 // Completion Timeout Mask
#define B_PCH_PCIE_UEM_FCPE                                BIT13 // Flow Control Protocol Error Mask (Not Supported)
#define B_PCH_PCIE_UEM_PT                                  BIT12 // Poisoned TLP Mask
#define B_PCH_PCIE_UEM_DLPE                                BIT4  // DataLink Protocol Error Mask
#define B_PCH_PCIE_UEM_TE                                  BIT0  // Training Error Mask (Not Supported)

#define R_PCH_PCIE_UEV                                     0x10C // Uncorrectable Error Severity
#define S_PCH_PCIE_UEV                                     4
#define B_PCH_PCIE_UEV_URE                                 BIT20 // Unsupported Request Error Severity
#define B_PCH_PCIE_UEV_EE                                  BIT19 // ECRC Error Severity
#define B_PCH_PCIE_UEV_MT                                  BIT18 // Malformed TLP Severity
#define B_PCH_PCIE_UEV_RO                                  BIT17 // Receiver Overflow Severity
#define B_PCH_PCIE_UEV_UC                                  BIT16 // Unexpected Completion Severity
#define B_PCH_PCIE_UEV_CA                                  BIT15 // Completion Abort Severity
#define B_PCH_PCIE_UEV_CT                                  BIT14 // Completion Timeout Severity
#define B_PCH_PCIE_UEV_FCPE                                BIT13 // Flow Control Protocol Error Severity
#define B_PCH_PCIE_UEV_PT                                  BIT12 // Poisoned TLP Severity
#define B_PCH_PCIE_UEV_DLPE                                BIT4  // Data Link Protocol Error Severity
#define B_PCH_PCIE_UEV_TE                                  BIT0  // Training Error Severity (Not Supported)

#define R_PCH_PCIE_CES                                     0x110 // Correctable Error Status
#define S_PCH_PCIE_CES                                     4
#define B_PCH_PCIE_CES_ANFES                               BIT13 // Advisory Non-Fatal Error Status
#define B_PCH_PCIE_CES_RTT                                 BIT12 // Replay Timer Timeout Status
#define B_PCH_PCIE_CES_RNR                                 BIT8  // Replay Number Rollover Status
#define B_PCH_PCIE_CES_BD                                  BIT7  // Bad DLLP Status
#define B_PCH_PCIE_CES_BT                                  BIT6  // Bad TLP Status
#define B_PCH_PCIE_CES_RE                                  BIT0  // Receiver Error Status

#define R_PCH_PCIE_CEM                                     0x114 // Correctable Error Mask
#define S_PCH_PCIE_CEM                                     4
#define B_PCH_PCIE_CEM_ANFEM                               BIT13 // Advisory Non-Fatal Error Mask
#define B_PCH_PCIE_CEM_RTT                                 BIT12 // Replay Timer Timeout Mask
#define B_PCH_PCIE_CEM_RNR                                 BIT8  // Replay Number Rollover Mask
#define B_PCH_PCIE_CEM_BD                                  BIT7  // Bad DLLP Mask
#define B_PCH_PCIE_CEM_BT                                  BIT6  // Bad TLP Mask
#define B_PCH_PCIE_CEM_RE                                  BIT0  // Receiver Error Mask

#define R_PCH_PCIE_AECC                                    0x118 // Advanced Error Capabilities and Control
#define S_PCH_PCIE_AECC                                    4
#define B_PCH_PCIE_AECC_ECE                                BIT8  // ECRC Check Enable
#define B_PCH_PCIE_AECC_ECC                                BIT7  // ECRC Check Capable
#define B_PCH_PCIE_AECC_EGE                                BIT6  // ECRC Generation Enable
#define B_PCH_PCIE_AECC_EGC                                BIT5  // ECRC Generation Capable
#define B_PCH_PCIE_AECC_FEP                                0x0000001F // First Error Pointer

#define R_PCH_PCIE_HL_DW1                                  0x11C // Header Log DW1

#define R_PCH_PCIE_HL_DW2                                  0x120 // Header Log DW2

#define R_PCH_PCIE_HL_DW3                                  0x124 // Header Log DW3

#define R_PCH_PCIE_HL_DW4                                  0x128 // Header Log DW4

#define R_PCH_PCIE_REC                                     0x12C // Root Error Command
#define B_PCH_PCIE_REC_FERE                                BIT2  // Fatal Error Report Enable
#define B_PCH_PCIE_REC_NERE                                BIT1  // Non-Fatal Error Report Enable
#define B_PCH_PCIE_REC_CERE                                BIT0  // Correctable Error Report Enable

#define R_PCH_PCIE_RES                                     0x130 // Root Error Status
#define S_PCH_PCIE_RES                                     4
#define B_PCH_PCIE_RES_AEMN                                0xF8000000 // Advanced Error Interrupt Message Number
#define B_PCH_PCIE_RES_FEMR                                BIT6  // Fatal Error Messages Received
#define B_PCH_PCIE_RES_NFEMR                               BIT5  // Non-Fatal Error Messages Received
#define B_PCH_PCIE_RES_FUF                                 BIT4  // First Uncorrectable Fatal
#define B_PCH_PCIE_RES_MENR                                BIT3  // Multiple ERR_FATAL / NONFATAL Received
#define B_PCH_PCIE_RES_ENR                                 BIT2  // ERR_FATAL / NONFATAL Received
#define B_PCH_PCIE_RES_MCR                                 BIT1  // Multiple ERR_COR Received
#define B_PCH_PCIE_RES_CR                                  BIT0  // ERR_COR Received

#define R_PCH_PCIE_ESID                                    0x134 // Error Source Identification
#define B_PCH_PCIE_ESID_EFNFSID                            0xFFFF0000 // ERR_FATAL / NONFATAL Source Identification
#define B_PCH_PCIE_ESID_ECSID                              0x0000FFFF // ERR_COR Source Identification

#define R_PCH_PCIE_PCIENFTS                                0x314 // PCI Express NFTS
#define B_PCH_PCIE_PCIENFTS_G2UCNFTS                       0xFF000000 // Gen2 Unique Clock N_FTS
#define B_PCH_PCIE_PCIENFTS_G2CCNFTS                       0x00FF0000 // Gen2 Common Clock N_FTS
#define B_PCH_PCIE_PCIENFTS_G1UCNFTS                       0x0000FF00 // Gen1 Unique Clock N_FTS
#define B_PCH_PCIE_PCIENFTS_G1CCNFTS                       0x000000FF // Gen1 Common Clock N_FTS

#define R_PCH_PCIE_PCIEL0SC                                0x318 // PCI Express L0s Control
#define B_PCH_PCIE_PCIEL0SC_G2ASL0SPL                      0xFF000000 // Gen2 Active State L0s Preparation Latency
#define B_PCH_PCIE_PCIEL0SC_G1ASL0SPL                      0x00FF0000 // Gen1 Active State L0s Preparation Latency
#define B_PCH_PCIE_PCIEL0SC_ANFTSO                         0x0000FF00 // Adaptive N_FTS Offset
#define B_PCH_PCIE_PCIEL0SC_ANFTSEN                        BIT7  // Adaptive N_FTS Enable
#define B_PCH_PCIE_PCIEL0SC_TXL0SRXENTRY                   BIT6  // Tx L0s Tx Entry
#define B_PCH_PCIE_PCIEL0SC_TXL0SRXEXIT                    (BIT5 | BIT4) // Tx L0s Rx Exit Control
#define B_PCH_PCIE_PCIEL0SC_G2L0SIC                        (BIT3 | BIT2) // Gen2 L0s Entry Idle Control
#define B_PCH_PCIE_PCIEL0SC_G1L0SIC                        (BIT1 | BIT0) // Gen1 L0s Entry Idle Control

#define R_PCH_PCIE_PCIEATL                                 0x31C // PCI Express ACK Transmission Latency
#define B_PCH_PCIE_PCIEATL_256BALO                         (BIT26 | BIT25 | BIT24) // 256B MPS Ack Latency Offset
#define B_PCH_PCIE_PCIEATL_G2X1                            (BIT22 | BIT21 | BIT20) // Gen2 x1
#define B_PCH_PCIE_PCIEATL_G2X2                            (BIT18 | BIT17 | BIT16) // Gen2 x2
#define B_PCH_PCIE_PCIEATL_G2X4                            (BIT14 | BIT13 | BIT12) // Gen2 x4
#define B_PCH_PCIE_PCIEATL_G1X1                            (BIT10 | BIT9 | BIT8) // Gen1 x1
#define B_PCH_PCIE_PCIEATL_G1X2                            (BIT6 | BIT5 | BIT4) // Gen1 x2
#define B_PCH_PCIE_PCIEATL_G1X4                            (BIT2 | BIT1 | BIT0) // Gen1 x4

#define R_PCH_PCIE_PCIECFG2                                0x320 // PCI Express Configuration 2
#define S_PCH_PCIE_PCIECFG2                                4
#define B_PCH_PCIE_PCIECFG2_CROAOV                         BIT24 // Completion Relaxed Ordering Attribute Override Value
#define B_PCH_PCIE_PCIECFG2_CROAOE                         BIT23 // Completion Relaxed Ordering Attribute Override Enable
#define B_PCH_PCIE_PCIECFG2_CRSREN                         BIT22 // Completion Retry Status Replay Enable
#define B_PCH_PCIE_PCIECFG2_PMET                           (BIT21 | BIT20) // PME Timeout
#define B_PCH_PCIE_PCIECFG2_SKPOSL                         0x000FFE00 // SKIP Ordered-Set Latency
#define B_PCH_PCIE_PCIECFG2_LATGC                          (BIT8 | BIT7 | BIT6) // Link Arbiter - TLP Grant Count
#define B_PCH_PCIE_PCIECFG2_LAFGC                          (BIT5 | BIT4 | BIT3) // Link Arbiter - FCP Grant Count
#define B_PCH_PCIE_PCIECFG2_LAANGC                         (BIT2 | BIT1 | BIT0) // Link Arbiter - Ack / Nak Grant Count

#define R_PCH_PCIE_PCIEDBG                                 0x324 // PCI Express Debug and Configuration
#define S_PCH_PCIE_PCIEDBG                                 4
#define B_PCH_PCIE_PCIEDBG_TXNFTSADD                       (BIT31 | BIT30 | BIT29) // Transmit nFTS Adder
#define B_PCH_PCIE_PCIEDBG_REUTLPBKME                      BIT28 // REUT Loopback Master Entry
#define B_PCH_PCIE_PCIEDBG_SDCLKSQEXITDBTIMERS             (BIT27 | BIT26) // Side Clock Domain Squelch Exit Debounce Timers
#define B_PCH_PCIE_PCIEDBG_LGCLKSQEXITDBTIMERS             (BIT25 | BIT24) // Link Clock Domain Squelch Exit Debounce Timers
#define B_PCH_PCIE_PCIEDBG_TALS                            0xFF0000 // Training Abort LTSSM State
#define B_PCH_PCIE_PCIEDBG_REUTFLPBKME                     BIT15 // REUT Forced Loopback Master Entry
#define B_PCH_PCIE_PCIEDBG_CTONFAE                         BIT14 // Completion Time-Out Non-fatal Advisory Error Enable
#define B_PCH_PCIE_PCIEDBG_LDSWQRP                         BIT13 // Link Down SWQ Reset Policy
#define B_PCH_PCIE_PCIEDBG_XCLKSURVMODEEN                  BIT12 // Cross-Clock Survivability Mode Enable
#define B_PCH_PCIE_PCIEDBG_NEDLBE                          BIT11 // Near End Digital Loopback Mode Enable
#define B_PCH_PCIE_PCIEDBG_IRFELB                          BIT10 // Initiate Remote Far-end Loop Back
#define B_PCH_PCIE_PCIEDBG_DSB                             BIT9  // De-skew Bypass
#define B_PCH_PCIE_PCIEDBG_REPID                           BIT8  // Receive Error Packet Invalidation Disable
#define B_PCH_PCIE_PCIEDBG_CPRODIS                         BIT7  // De-bounce Clock Edge Policy
#define B_PCH_PCIE_PCIEDBG_CMPLRX                          BIT6  // Compliance Receive
#define B_PCH_PCIE_PCIEDBG_SPCE                            BIT5  // Squelch Propagation Control Enable
#define B_PCH_PCIE_PCIEDBG_LR                              BIT4  // Lane Reversal
#define B_PCH_PCIE_PCIEDBG_DMIL1EDM                        BIT3  // DMI L1 Entry Disable Mask
#define B_PCH_PCIE_PCIEDBG_SCMBB                           BIT2  // Scrambler Bypass
#define B_PCH_PCIE_PCIEDBG_CRCD                            BIT1  // CRC Disable
#define B_PCH_PCIE_PCIEDBG_SNCD                            BIT0  // Sequence Number Checking Disable

#define R_PCH_PCIE_PCIESTS1                                0x328 // PCI Express Status 1
#define B_PCH_PCIE_PCIESTS1_LTSMSTATE                      0x1F000000 // LTSM State
#define B_PCH_PCIE_PCIESTS1_LNKSTAT                        0x00780000 // Link Status
#define B_PCH_PCIE_PCIESTS1_REPLAYNUM                      (BIT18 | BIT17) // Replay Number
#define B_PCH_PCIE_PCIESTS1_DLLRETRY                       BIT16 // Data Link Layer Retry
#define B_PCH_PCIE_PCIESTS1_LANESTAT                       0x0000F000 // Lane Status
#define B_PCH_PCIE_PCIESTS1_NXTTXSEQNUM                    0x00000FFF // Next Transmitted Sequence Number

#define R_PCH_PCIE_PCIESTS2                                0x32C // PCI Express Status 2
#define B_PCH_PCIE_PCIESTS2_P48PNCCWSSCMES                 BIT31 // PCIe Port 4/8 Non-Common Clock With SSC Mode Enable Strap
#define B_PCH_PCIE_PCIESTS2_P37PNCCWSSCMES                 BIT30 // PCIe Port 3/7 Non-Common Clock With SSC Mode Enable Strap
#define B_PCH_PCIE_PCIESTS2_P26PNCCWSSCMES                 BIT29 // PCIe Port 2/6 Non-Common Clock With SSC Mode Enable Strap
#define B_PCH_PCIE_PCIESTS2_P15PNCCWSSCMES                 BIT28 // PCIe Port 1/5 Non-Common Clock With SSC Mode Enable Strap
#define B_PCH_PCIE_PCIESTS2_NXTRCVSEQ                      0x0FFF0000 // Next Receive Sequence Number
#define B_PCH_PCIE_PCIESTS2_LASTACKSEQNUM                  0x00000FFF // Last Acknowledged Sequence Number

#define R_PCH_PCIE_PCIECMMPC                               0x330 // PCI Express Compliance Measurement Mode (CMM) Port Control
#define S_PCH_PCIE_PCIECMMPC                               4
#define B_PCH_PCIE_PCIECMMPC_SYM3SEL                       BIT29 // CMM Symbol [3] Select
#define B_PCH_PCIE_PCIECMMPC_SYM2SEL                       BIT28 // CMM Symbol [2] Select
#define B_PCH_PCIE_PCIECMMPC_SYM1SEL                       BIT27 // CMM Symbol [1] Select
#define B_PCH_PCIE_PCIECMMPC_SYM0SEL                       BIT26 // CMM Symbol [0] Select
#define B_PCH_PCIE_PCIECMMPC_ERRLANENUM                    (BIT23 | BIT22) // CMM Error Lane Number
#define B_PCH_PCIE_PCIECMMPC_INVERT                        (BIT15 | BIT14 | BIT13) // CMM Invert
#define B_PCH_PCIE_PCIECMMPC_SYMERRNUMINV                  (BIT12 | BIT11 | BIT10) // CMM Symbol Error Number Invert
#define B_PCH_PCIE_PCIECMMPC_SYMERRNUM                     (BIT9 | BIT8) // CMM Symbol Error Number
#define B_PCH_PCIE_PCIECMMPC_ERRDET                        BIT7  // CMM Error Detected
#define B_PCH_PCIE_PCIECMMPC_SLNINVCMM                     (BIT6 | BIT5) // Select Lane Number to be Inverted for CMM
#define B_PCH_PCIE_PCIECMMPC_AUTOINVERT                    BIT4  // CMM AutoInvert
#define B_PCH_PCIE_PCIECMMPC_STAT                          BIT3  // CMM Status
#define B_PCH_PCIE_PCIECMMPC_INVEN                         BIT2  // CMM Invert Enable
#define B_PCH_PCIE_PCIECMMPC_START                         BIT0  // CMM Start

#define R_PCH_PCIE_PCIECMMSB                               0x334 // PCI Express Compliance Measurement Mode Symbol Buffer
#define B_PCH_PCIE_PCIECMMSB_DATA3                         0xFF000000 // CMM Data [3]
#define B_PCH_PCIE_PCIECMMSB_DATA2                         0x00FF0000 // CMM Data [2]
#define B_PCH_PCIE_PCIECMMSB_DATA1                         0x0000FF00 // CMM Data [1]
#define B_PCH_PCIE_PCIECMMSB_DATA0                         0x000000FF // CMM Data [0]

#define R_PCH_PCIE_PCIEALC                                 0x338 // PCI Express Compliance Measurement Mode Symbol Buffer
#define B_PCH_PCIE_PCIEALC_ITLRCLD                         BIT29 // Initialize Transaction Layer Receiver Control on Link Down
#define B_PCH_PCIE_PCIEALC_ILLRCLD                         BIT28 // Initialize Link Layer Receiver Control on Link Down
#define B_PCH_PCIE_PCIEALC_BLKPAPC                         BIT27 // Block Polling.Active-> Polling.Configuration
#define B_PCH_PCIE_PCIEALC_BLKDQDA                         BIT26 // Block Detect.Quiet->Detect.Active

#define R_PCH_PCIE_PCIERTP                                 0x33C // PCI Express Replay Timer Policy
#define B_PCH_PCIE_PCIERTP_RTO256B                         0x0F000000 // 256B MPS Replay Timer Offset
#define B_PCH_PCIE_PCIERTP_G2X1                            0x00F00000 // Gen2 x1
#define B_PCH_PCIE_PCIERTP_G2X2                            0x000F0000 // Gen2 x2
#define B_PCH_PCIE_PCIERTP_G2X4                            0x0000F000 // Gen2 x4
#define B_PCH_PCIE_PCIERTP_G1X1                            0x00000F00 // Gen1 x1
#define B_PCH_PCIE_PCIERTP_G1X2                            0x000000F0 // Gen1 x2
#define B_PCH_PCIE_PCIERTP_G1X4                            0x0000000F // Gen1 x4

#define R_PCH_PCIE_VC0PCC                                  0x340 // VC0 Posted Credits Consumed
#define B_PCH_PCIE_VC0PCC_VC0PCCH                          0x00FF0000 // VC0 Posted Credits Consumed Header
#define B_PCH_PCIE_VC0PCC_VC0PCCD                          0x00000FFF // VC0 Posted Credits Consumed Data

#define R_PCH_PCIE_VC0PCL                                  0x344 // VC0 Posted Credit Limit
#define B_PCH_PCIE_VC0PCL_VC0PCLH                          0x00FF0000 // VC0 Posted Credit Limit Header
#define B_PCH_PCIE_VC0PCL_VC0PCLD                          0x00000FFF // VC0 Posted Credit Limit Data

#define R_PCH_PCIE_VC0PCA                                  0x348 // VC0 Posted Credits Allocated
#define B_PCH_PCIE_VC0PCA_VC0PCAH                          0x00FF0000 // VC0 Posted Credits Allocated Header
#define B_PCH_PCIE_VC0PCA_VC0PCAD                          0x00000FFF // VC0 Posted Credits Allocated Data

#define R_PCH_PCIE_VC0NPCC                                 0x34C // VC0 Non-Posted Credits Consumed
#define B_PCH_PCIE_VC0NPCC_VC0NPCCH                        0x00FF0000 // VC0 Non-Posted Credits Consumed Header
#define B_PCH_PCIE_VC0NPCC_VC0NPCCD                        0x00000FFF // VC0 Non-Posted Credits Consumed Data

#define R_PCH_PCIE_VC0NPCL                                 0x350 // VC0 Non-Posted Credit Limit
#define B_PCH_PCIE_VC0NPCL_VC0NPCLH                        0x00FF0000 // VC0 Non-Posted Credit Limit Header
#define B_PCH_PCIE_VC0NPCL_VC0NPCLD                        0x00000FFF // VC0 Non-Posted Credit Limit Data

#define R_PCH_PCIE_VC0NPCA                                 0x354 // VC0 Non-Posted Credits Allocated
#define B_PCH_PCIE_VC0NPCA_VC0NPCAH                        0x00FF0000 // VC0 Non-Posted Credits Allocated Header
#define B_PCH_PCIE_VC0NPCA_VC0NPCAD                        0x00000FFF // VC0 Non-Posted Credits Allocated Data

#define R_PCH_PCIE_VC0CPCC                                 0x358 // VC0 Completion Credits Consumed
#define B_PCH_PCIE_VC0CPCC_VC0CPCCH                        0x00FF0000 // VC0 Completion Credits Consumed Header
#define B_PCH_PCIE_VC0CPCC_VC0CPCCD                        0x00000FFF // VC0 Completion Credits Consumed Data

#define R_PCH_PCIE_VC0CPCL                                 0x35C // VC0 Completion Credit Limit
#define B_PCH_PCIE_VC0CPCL_VC0CPCLH                        0x00FF0000 // VC0 Completion Credit Limit Header
#define B_PCH_PCIE_VC0CPCL_VC0CPCLD                        0x00000FFF // VC0 Completion Credit Limit Data

#define R_PCH_PCIE_VC0CPCA                                 0x360 // VC0 Completion Credits Allocated
#define B_PCH_PCIE_VC0CPCA_VC0CPCAH                        0x00FF0000 // VC0 Completion Credits Allocated Header
#define B_PCH_PCIE_VC0CPCA_VC0CPCAD                        0x00000FFF // VC0 Completion Credits Allocated Data

#define R_PCH_PCIE_LTROVR                                  0x400 // Latency Tolerance Reporting Override
#define B_PCH_PCIE_LTROVR_LTRNSROVR                        BIT31 // LTR Non-Snoop Requirement Bit Override
#define B_PCH_PCIE_LTROVR_LTRNSLSOVRV                      (BIT28 | BIT27 | BIT26) // LTR Non-Snoop Latency Scale Override
#define B_PCH_PCIE_LTROVR_LTRNSLOVRV                       0x3FF0000 // LTR Non-Snoop Latency Override Value
#define B_PCH_PCIE_LTROVR_LTRSROVR                         BIT15 // LTR Snoop Requirement Bit Override
#define B_PCH_PCIE_LTROVR_LTRSLSOVRV                       (BIT12 | BIT11 | BIT10) // LTR Snoop Latency Scale Override Value
#define B_PCH_PCIE_LTROVR_LTRSLOVRV                        0x3FF // LTR Snoop Latency Override Value

#define R_PCH_PCIE_LTROVR2                                 0x404 // Latency Tolerance Reporting Override 2
#define B_PCH_PCIE_LTROVR2_LTROVRPLCY                      BIT3  // LTR Override Policy
#define B_PCH_PCIE_LTROVR2_LTRCFGLOCK                      BIT2  // LTR Configuration Lock
#define B_PCH_PCIE_LTROVR2_LTRNSOVREN                      BIT1  // LTR Non-Snoop Override Enable
#define B_PCH_PCIE_LTROVR2_LTRSOVREN                       BIT0  // LTR Snoop Override Enable

#define R_PCH_PCIE_PHYCTL4                                 0x408 // Physical Layer and AFE Control 4
#define B_PCH_PCIE_PHYCTL4_SQDIS                           BIT27 // Squelch Disable
#define B_PCH_PCIE_PHYCTL4_PLLCLKVADT                      (BIT26 | BIT25 | BIT24) // PLL Clock Valid Assertion Delay Timer
#define B_PCH_PCIE_PHYCTL4_PLLDISDTMR                      (BIT23 | BIT22 | BIT21) // PLL Disable Delay Timer
#define B_PCH_PCIE_PHYCTL4_GCU                             BIT20 // Grant Count Update
#define B_PCH_PCIE_PHYCTL4_SATAPSGC                        (BIT19 | BIT18 | BIT17 | BIT16) // SATA Port Staggering Grant Count
#define B_PCH_PCIE_PHYCTL4_USB3PSGC                        (BIT15 | BIT14 | BIT13 | BIT12) // USB3 Port Staggering Grant Count
#define B_PCH_PCIE_PHYCTL4_PCIEPSGC                        (BIT11 | BIT10 | BIT9 | BIT8) // PCIe Port Staggering Grant Count
#define B_PCH_PCIE_PHYCTL4_GBEPSGC                         (BIT7 | BIT6 | BIT5 | BIT4) // GbE Port Staggering Grant Count
#define B_PCH_PCIE_PHYCTL4_DMIPSGC                         (BIT3 | BIT2 | BIT1 | BIT0) // DMI Port Staggering Grant Count

#define R_PCH_PCIE_SAIC                                    0x40C // Security Attributes of Initiator Error Status
#define B_PCH_PCIE_SAIC_PSAIESID                           0xFFFF0000 // Primary SAI Error Source Identification
#define B_PCH_PCIE_SAIC_AVDPI                              BIT15 // Access Violation Detected On Primary Interface
#define B_PCH_PCIE_SAIC_SAIVERE                            BIT14 // SAI Violation Error Reporting Enable
#define B_PCH_PCIE_SAIC_FSIAV                              (BIT10 | BIT9) // Fuse / Strap / ID Access Violation
#define B_PCH_PCIE_SAIC_AVDSBI                             BIT8 // Access Violation Detected On Sideband Interface
#define B_PCH_PCIE_SAIC_SSAIESID                           0xFF // Sideband SAI Error Source Identification

#define R_PCH_PCIE_PHYCTL5                                 0x410 // Physical Layer and AFE Control 5
#define B_PCH_PCIE_PHYCTL5_USB3SDP                         0x1F00 // ExpressCard USB3# Select GPIO Debounce Period
#define B_PCH_PCIE_PHYCTL5_PCIEP2SATAP5LO                  BIT5  // PCIe Port 2 SATA Port 5 Lane Owner
#define B_PCH_PCIE_PHYCTL5_PCIEP1SATAP4LO                  BIT4  // PCIe Port 1 SATA Port 4 Lane Owner
#define B_PCH_PCIE_PHYCTL5_PCIEP2USBP5LO                   (BIT3 | BIT2) // PCIe Port 2 USB3 Port 5 Lane Owner
#define B_PCH_PCIE_PHYCTL5_PCIEP1USBP4LO                   (BIT1 | BIT0) // PCIe Port 1 USB3 Port 4 Lane Owner

#define R_PCH_PCIE_PM_EXT_CTL                              0x420 // PCIe PM Extension Control
#define B_PCH_PCIE_PM_EXT_CTL_DLSLSD                       BIT29
#define B_PCH_PCIE_PM_EXT_CTL_L1FSOE                       BIT0

#endif
