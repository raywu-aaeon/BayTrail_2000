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
  PchRegsLan.h

  @brief
  Register names for PCH LAN device.
  
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
#ifndef _PCH_REGS_LAN_H_
#define _PCH_REGS_LAN_H_

///
/// LAN Controller Registers (D25:F0)
///
#define PCI_BUS_NUMBER_PCH_LAN        0
#define PCI_DEVICE_NUMBER_PCH_LAN     25
#define PCI_FUNCTION_NUMBER_PCH_LAN   0

#define R_PCH_LAN_VENDOR_ID           0x00  //  Vendor ID
#define V_PCH_LAN_VENDOR_ID           V_PCH_INTEL_VENDOR_ID // Intel Vendor ID

#define R_PCH_LAN_DEVICE_ID           0x02  // Device ID
#define V_PCH_LAN_DEVICE_ID_0         0x153E

#define R_PCH_LAN_CMD_STS             0x04  // Command Status Register
#define B_PCH_LAN_CMD_STS_DPE         BIT31 // Detected Parity Error
#define B_PCH_LAN_CMD_STS_SSE         BIT30 // Signaled System Error
#define B_PCH_LAN_CMD_STS_RMA         BIT29 // Received Master Abort
#define B_PCH_LAN_CMD_STS_RTA         BIT28 // Received Target Abort
#define B_PCH_LAN_CMD_STS_STA         BIT27 // Signaled Target Abort
#define B_PCH_LAN_CMD_STS_DEV_STS     (BIT26 | BIT25) // DEVSEL Timing
#define B_PCH_LAN_CMD_STS_DPED        BIT24 // Data Parity Reported
#define B_PCH_LAN_CMD_STS_FB2BC       BIT23 // Fast Back-to-Back Capable
#define B_PCH_LAN_CMD_STS_66MHZ_CAP   BIT21 // 66Mhz Capable
#define B_PCH_LAN_CMD_STS_CAP_LST     BIT20 // New Capabilities
#define B_PCH_LAN_CMD_STS_INTR_STS    BIT19 // Interrupt Status
#define B_PCH_LAN_CMD_STS_INTR_DIS    BIT10 // Interrupt Disable
#define B_PCH_LAN_CMD_STS_FBE         BIT9  // Fast Back-to-Back Enable
#define B_PCH_LAN_CMD_STS_SERR_EN     BIT8  // SERR# Enable
#define B_PCH_LAN_CMD_STS_WCC         BIT7  // Wait Cycle Enable
#define B_PCH_LAN_CMD_STS_PER         BIT6  // Parity Error Response
#define B_PCH_LAN_CMD_STS_PSE         BIT5  // Palette Snoop Enable
#define B_PCH_LAN_CMD_STS_PMWE        BIT4  // MWI Enable
#define B_PCH_LAN_CMD_STS_SCE         BIT3  // Special Cycle Monitoring
#define B_PCH_LAN_CMD_STS_BME         BIT2  // Bus Master Enable
#define B_PCH_LAN_CMD_STS_MSE         BIT1  // Memory Access Enable
#define B_PCH_LAN_CMD_STS_IOSE        BIT0  // I/O Access Enable

#define R_PCH_LAN_RID_CC              0x08  // Revision ID & Class Code
#define R_PCH_LAN_RID_CC_CC           0xFFFFFF00 // Class Code
#define B_PCH_LAN_RID_CC_RID          0x000000FF // Revision ID

#define R_PCH_LAN_CACHE               0x0C  // Cache
#define B_PCH_LAN_CACHE_BIST          0xFF000000 // BIST
#define B_PCH_LAN_CACHE_HEADTYPE      0x00FF0000 // Header Type
#define B_PCH_LAN_CACHE_PLT           0x0000FF00 // Latency Timer
#define B_PCH_LAN_CACHE_CLS           0x000000FF // Cache Line Size

#define R_PCH_LAN_MBARA               0x10  // Base Address 0
#define B_PCH_LAN_MBARA_BA            0xFFFE0000 // Base Address
#define B_PCH_LAN_MBARA_MSIZE         0x0001FFF0 // Memory Size
#define B_PCH_LAN_MBARA_PM            BIT3  // Prefetchable Memory
#define B_PCH_LAN_MBARA_MT            (BIT2 | BIT1)  // Memory Type
#define B_PCH_LAN_MBARA_MIOS          BIT0  // Memory IO Space

#define R_PCH_LAN_MBARB               0x14  // Base Address 1
#define B_PCH_LAN_MBARB_BA            0xFFFFF000 // Base Address
#define B_PCH_LAN_MBARB_MSIZE         0x00000FF0 // Memory Size
#define B_PCH_LAN_MBARB_PM            BIT3  // Prefetchable Memory
#define B_PCH_LAN_MBARB_MT            (BIT2 | BIT1) // Memory Type
#define B_PCH_LAN_MBARB_MIOS          BIT0  // Memory IO Space

#define R_PCH_LAN_MBARC               0x18  // Base Address 2
#define B_PCH_LAN_MBARC_BA            0xFFFFFFFF // Base Address

#define R_PCH_LAN_MBARD               0x1C  // Base Address 3
#define B_PCH_LAN_MBARD_BA            0xFFFFFFFF // Base Address

#define R_PCH_LAN_MBARE               0x20  // Base Address 4
#define B_PCH_LAN_MBARE_BA            0xFFFFFFFF // Base Address

#define R_PCH_LAN_MBARF               0x24  // Base Address 5
#define B_PCH_LAN_MBARF_BA            0xFFFFFFFF // Base Address

#define R_PCH_LAN_CCP                 0x28  // Cardbus CIS Pointer
#define B_PCH_LAN_CCP_CCP             0xFFFFFFFF // Cardbus CIS Pointer

#define R_PCH_LAN_SVID                0x2C  // Subsystem Vendor ID
#define B_PCH_LAN_SVID_SID            0xFFFF0000 // Subsystem ID
#define B_PCH_LAN_SVID_SVID           0x0000FFFF // Subsystem Vendor ID

#define R_PCH_LAN_ERBA                0x30  // Expansion ROM Base Address
#define B_PCH_LAN_ERBA                0xFFFFFFFF // ROM Base Address

#define R_PCH_LAN_CAP_PTR             0x34  // Capabilities Pointer
#define B_PCH_LAN_CAP_PTR             0xFF  // Capabilities Pointer

#define R_PCH_LAN_INTR                0x3C  // Interrupt
#define B_PCH_LAN_INTR_MAXLAT         0xFF000000 // Max Latency
#define B_PCH_LAN_INTR_MINGRANT       0x00FF0000 // MIN GRANT
#define B_PCH_LAN_INTR_IPIN           0x0000FF00 // Interrupt Pin
#define B_PCH_LAN_INTR_ILINE          0x000000FF // Interrupt Line

#define R_PCH_LAN_ECTL                0x90  // Error Control
#define B_PCH_LAN_ECTL_URRE           BIT1  // URRE
#define B_PCH_LAN_ECTL_URD            BIT0  // URD

#define R_PCH_LAN_CLCTL               0x94  // Clock and Lock Control
#define B_PCH_LAN_CLCTL_LE            BIT15 // Lock Enable
#define B_PCH_LAN_CLCTL_DPRC          BIT4  // Disable PIPE / Free Running Clocks
#define B_PCH_LAN_CLCTL_TSCE          BIT3  // Time Sync Clock Enable
#define B_PCH_LAN_CLCTL_PCE           BIT2  // PIPE Clock Enable
#define B_PCH_LAN_CLCTL_SIOSFCE       BIT1  // SB IOSF Clock Enable
#define B_PCH_LAN_CLCTL_PIOSFCE       BIT0  // Primary IOSF Clock Enable

#define R_PCH_LAN_IOADD               0x98  // IO Address
#define B_PCH_LAN_IOADD_IOADD         0xFFFFFFFF // IO Address

#define R_PCH_LAN_IODATA              0x9C  // IO Data
#define B_PCH_LAN_IODATA_IODATA       0xFFFFFFFF // IO Data

#define R_PCH_LAN_PMC                 0xC8  // PMC
#define B_PCH_LAN_PMC_PMES            0xF8000000 // PME Support
#define B_PCH_LAN_PMC_NEXT            0x0000FF00 // Next Pointer
#define B_PCH_LAN_PMC_CID             0x0000000FF // Capabilities ID

#define R_PCH_LAN_PMCS                0xCC  // PMCSR
#define B_PCH_LAN_PMCS_PMES           BIT15 // PME Status
#define B_PCH_LAN_PMCS_DSC            (BIT14 | BIT13) // Data Scale
#define B_PCH_LAN_PMCS_DSL            0x1E00 // Data Select
#define V_PCH_LAN_PMCS_DSL0           0x0000
#define V_PCH_LAN_PMCS_DSL3           0x0600
#define V_PCH_LAN_PMCS_DSL4           0x0800
#define V_PCH_LAN_PMCS_DSL7           0x0E00
#define V_PCH_LAN_PMCS_DSL8           0x1000
#define B_PCH_LAN_PMCS_PMEE           BIT8  // PME Enable
#define B_PCH_LAN_PMCS_PS             (BIT1 | BIT0) // Power State
#define V_PCH_LAN_PMCS_PS0            0x00
#define V_PCH_LAN_PMCS_PS3            0x03

#define R_PCH_LAN_MSIC                0xD0  // MSIC
#define B_PCH_LAN_MSIC_MSIE           BIT16 // MSI Enable
#define B_PCH_LAN_MSIC_NEXTP          0x0000FF00 // Next Pointer
#define B_PCH_LAN_MSIC_CAPID          0x000000FF // Capabilities ID

#define R_PCH_LAN_MSIADDLOW           0xD4 // MSI Address Low
#define B_PCH_LAN_MSIADDLOW_ADDLOW    0xFFFFFFFF // MSI Lower Address

#define R_PCH_LAN_MSIADDHIGH          0xD8 // MSI Address High
#define B_PCH_LAN_MSIADDHIGH_ADDHIGH  0xFFFFFFFF // MSI Higher Address

#define R_PCH_LAN_MSID                0xDC  // MSI Data
#define B_PCH_LAN_MSID_MSID           0xFFFF // MSI Data


///
/// Gigabit LAN Capabilities and Status Registers (Memory space)
///
#define R_PCH_MBARA_CTRL              0x00  // Device Control
#define B_PCH_MBARA_CTRL_LCDPD        BIT24 // LCD Power Down

#define R_PCH_MBARA_CE                0x18  // Extended Device Control
#define B_PCH_MBARA_CTRL_PHYPDEN      BIT20 // PHY Power Down Enable

#define R_PCH_MBARA_MDIC              0x20  // MDI Control
#define B_PCH_MBARA_MDIC_R            BIT28 // Ready Bit
#define B_PCH_MBARA_MDIC_OP           (BIT27 | BIT26) // OP-Code
#define B_PCH_MBARA_MDIC_DATA         0xFFFF // Data

#define R_PCH_MBARA_ECC               0xF00 // Extended Configuration Control
#define B_PCH_MBARA_ECC_SWFLAG        BIT5  // SW Semaphore FLAG

#define R_PCH_MBARA_PC                0xF10 // PHY Control
#define B_PCH_MBARA_PC_LIND           BIT2  // LPLU in Non D0a
#define B_PCH_MBARA_PC_LID            BIT1  // LPLU in non D0a

#define R_PCH_MBARA_RAL               0x5400 // Receive Address Low

#define R_PCH_MBARA_RAH               0x5404 // Receive Address High
#define B_PCH_MBARA_RAH_RAH           0xFFFF // Receive Address High

#endif
