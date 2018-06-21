/*++
  This file contains an 'Intel Peripheral Driver' and uniquely        
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your   
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PchRegsScc.h

Abstract:

  Register names for VLV SCC module.
  
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
#ifndef _PCH_REGS_SCC_H_
#define _PCH_REGS_SCC_H_


//
// SCC Modules Registers
//

//
// SCC SDIO Modules
// PCI Config Space Registers
//
#define PCI_DEVICE_NUMBER_PCH_SCC_SDIO_0         16
#define PCI_DEVICE_NUMBER_PCH_SCC_SDIO_1         17
#define PCI_DEVICE_NUMBER_PCH_SCC_SDIO_2         18
#define PCI_DEVICE_NUMBER_PCH_SCC_SDIO_3         23

#define PCI_FUNCTION_NUMBER_PCH_SCC_SDIO         0

#define R_PCH_SCC_SDIO_DEVVENDID                 0x00  // Device ID & Vendor ID
#define B_PCH_SCC_SDIO_DEVVENDID_DID             0xFFFF0000 // Device ID
#define B_PCH_SCC_SDIO_DEVVENDID_VID             0x0000FFFF // Vendor ID
#define V_PCH_SCC_SDIO_DEVVENDID_VID_EMMC        0x0F14  // SDIO_0 for eMMC
#define V_PCH_SCC_SDIO_DEVVENDID_VID_SD          0x0F16  // SDIO_2 for SD card

#define R_PCH_SCC_SDIO_STSCMD                    0x04  // Status & Command
#define B_PCH_SCC_SDIO_STSCMD_RMA                BIT29 // RMA
#define B_PCH_SCC_SDIO_STSCMD_RCA                BIT28 // RCA
#define B_PCH_SCC_SDIO_STSCMD_CAPLIST            BIT20 // Capability List
#define B_PCH_SCC_SDIO_STSCMD_INTRSTS            BIT19 // Interrupt Status
#define B_PCH_SCC_SDIO_STSCMD_INTRDIS            BIT10 // Interrupt Disable
#define B_PCH_SCC_SDIO_STSCMD_SERREN             BIT8  // SERR# Enable
#define B_PCH_SCC_SDIO_STSCMD_BME                BIT2  // Bus Master Enable
#define B_PCH_SCC_SDIO_STSCMD_MSE                BIT1  // Memory Space Enable

#define R_PCH_SCC_SDIO_REVCC                     0x08  // Revision ID & Class Code
#define B_PCH_SCC_SDIO_REVCC_CC                  0xFFFFFF00 // Class Code
#define B_PCH_SCC_SDIO_REVCC_RID                 0x000000FF // Revision ID

#define R_PCH_SCC_SDIO_CLHB                      0x0C
#define B_PCH_SCC_SDIO_CLHB_MULFNDEV             BIT23 // Multi Function Device
#define B_PCH_SCC_SDIO_CLHB_HT                   0x007F0000 // Header Type
#define B_PCH_SCC_SDIO_CLHB_LT                   0x0000FF00 // Latency Timer
#define B_PCH_SCC_SDIO_CLHB_CLS                  0x000000FF // Cache Line Size

#define R_PCH_SCC_SDIO_BAR                       0x10  // BAR
#define B_PCH_SCC_SDIO_BAR_BA                    0xFFFFF000 // Base Address
#define V_PCH_SCC_SDIO_BAR_SIZE                  0x1000
#define N_PCH_SCC_SDIO_BAR_ALIGNMENT             12
#define B_PCH_SCC_SDIO_BAR_SI                    0x00000FF0 // Size Indicator
#define B_PCH_SCC_SDIO_BAR_PF                    BIT3  // Prefetchable
#define B_PCH_SCC_SDIO_BAR_TYPE                  (BIT2 | BIT1) // Type
#define B_PCH_SCC_SDIO_BAR_MS                    BIT0  // Message Space

#define R_PCH_SCC_SDIO_BAR1                      0x14  // BAR 1
#define B_PCH_SCC_SDIO_BAR1_BA                   0xFFFFF000 // Base Address
#define B_PCH_SCC_SDIO_BAR1_SI                   0x00000FF0 // Size Indicator
#define B_PCH_SCC_SDIO_BAR1_PF                   BIT3  // Prefetchable
#define B_PCH_SCC_SDIO_BAR1_TYPE                 (BIT2 | BIT1) // Type
#define B_PCH_SCC_SDIO_BAR1_MS                   BIT0  // Message Space

#define R_PCH_SCC_SDIO_SSID                      0x2C  // Sub System ID
#define B_PCH_SCC_SDIO_SSID_SID                  0xFFFF0000 // Sub System ID
#define B_PCH_SCC_SDIO_SSID_SVID                 0x0000FFFF // Sub System Vendor ID

#define R_PCH_SCC_SDIO_ERBAR                     0x30  // Expansion ROM BAR
#define B_PCH_SCC_SDIO_ERBAR_BA                  0xFFFFFFFF // Expansion ROM Base Address

#define R_PCH_SCC_SDIO_CAPPTR                    0x34  // Capability Pointer
#define B_PCH_SCC_SDIO_CAPPTR_CPPWR              0xFF  // Capability Pointer Power

#define R_PCH_SCC_SDIO_INTR                      0x3C  // Interrupt
#define B_PCH_SCC_SDIO_INTR_ML                   0xFF000000 // Max Latency
#define B_PCH_SCC_SDIO_INTR_MG                   0x00FF0000
#define B_PCH_SCC_SDIO_INTR_IP                   0x00000F00 // Interrupt Pin
#define B_PCH_SCC_SDIO_INTR_IL                   0x000000FF // Interrupt Line

#define R_PCH_SCC_SDIO_PCAPID                    0x80  // Power Capability ID
#define B_PCH_SCC_SDIO_PCAPID_PS                 0xF8000000 // PME Support
#define B_PCH_SCC_SDIO_PCAPID_VS                 0x00070000 // Version
#define B_PCH_SCC_SDIO_PCAPID_NC                 0x0000FF00 // Next Capability
#define B_PCH_SCC_SDIO_PCAPID_PC                 0x000000FF // Power Capability

#define R_PCH_SCC_SDIO_PCS                       0x84  // PME Control Status
#define B_PCH_SCC_SDIO_PCS_PMESTS                BIT15 // PME Status
#define B_PCH_SCC_SDIO_PCS_PMEEN                 BIT8  // PME Enable
#define B_PCH_SCC_SDIO_PCS_NSS                   BIT3  // No Soft Reset
#define B_PCH_SCC_SDIO_PCS_PS                    (BIT1 | BIT0) // Power State

#define R_PCH_SCC_SDIO_GEN_REGRW1                0xA0 //General Purpose Read Write Register1
#define R_PCH_SCC_SDIO_GEN_REGRW2                0xA4 //General Purpose Read Write Register2
#define B_PCH_SCC_SDIO_CAP_REG_SEL_HW            0x0 //capabilty will come from hardware
#define B_PCH_SCC_SDIO_CAP_REG_SEL_GEN           BIT31   //capabilty will come from GEN PCI Register

#define R_PCH_SCC_SDIO_GEN_REGRW3                0xA8 //General Purpose Read Write Register3
#define R_PCH_SCC_SDIO_GEN_REGRW4                0xAc //General Purpose Read Write Register4
#define R_PCH_SCC_SDIO_MANID                     0xF8  // Manufacturer ID
#define B_PCH_SCC_SDIO_MANID_MANID               0xFFFFFFFF // Manufacturer ID

//
// SCC SDIO Module
// MMIO Space Register
//
#define R_PCH_SCC_SDIO_MEM_TIMEOUT_CTL           0x2E  // Timeout Control
#define B_PCH_SCC_SDIO_MEM_TIMEOUT_CTL_DTCV      0x0F  // Data Timeout Counter Value

#define R_PCH_SCC_SDIO_MEM_CESHC2                0x3C  // Auto CMD12 Error Status Register & Host Control 2
#define R_PCH_SCC_SDIO_MEM_CAP1                  0x40  // Capability register bits 0-31
#define R_PCH_SCC_SDIO_MEM_CAP2                  0x44  // Capability register bits 32-63
#define B_PCH_SCC_SDIO_MEM_CESHC2_ASYNC_INT      BIT30 // Asynchronous Interrupt Enable
//
// SCC HSI Module
// PCI Config Space Registers
//
#define PCI_DEVICE_NUMBER_PCH_SCC_HSI            23
#define PCI_FUNCTION_NUMBER_PCH_SCC_HSI          0

#define R_PCH_SCC_HSI_DEVVENDID                  0x00  // Device ID & Vendor ID
#define B_PCH_SCC_HSI_DEVVENDID_DID              0xFFFF0000 // Device ID
#define B_PCH_SCC_HSI_DEVVENDID_VID              0x0000FFFF // Vendor ID

#define R_PCH_SCC_HSI_STSCMD                     0x04  // Status & Command
#define B_PCH_SCC_HSI_STSCMD_RMA                 BIT29 // RMA
#define B_PCH_SCC_HSI_STSCMD_RCA                 BIT28 // RCA
#define B_PCH_SCC_HSI_STSCMD_CAPLIST             BIT20 // Capability List
#define B_PCH_SCC_HSI_STSCMD_INTRSTS             BIT19 // Interrupt Status
#define B_PCH_SCC_HSI_STSCMD_INTRDIS             BIT10 // Interrupt Disable
#define B_PCH_SCC_HSI_STSCMD_SERREN              BIT8  // SERR# Enable
#define B_PCH_SCC_HSI_STSCMD_BME                 BIT2  // Bus Master Enable
#define B_PCH_SCC_HSI_STSCMD_MSE                 BIT1  // Memory Space Enable

#define R_PCH_SCC_HSI_REVCC                      0x08  // Revision ID & Class Code
#define B_PCH_SCC_HSI_REVCC_CC                   0xFFFFFF00 // Class Code
#define B_PCH_SCC_HSI_REVCC_RID                  0x000000FF // Revision ID

#define R_PCH_SCC_HSI_CLHB                       0x0C
#define B_PCH_SCC_HSI_CLHB_MULFNDEV              BIT23 // Multi Function Device
#define B_PCH_SCC_HSI_CLHB_HT                    0x007F0000 // Header Type
#define B_PCH_SCC_HSI_CLHB_LT                    0x0000FF00 // Latency Timer
#define B_PCH_SCC_HSI_CLHB_CLS                   0x000000FF // Cache Line Size

#define R_PCH_SCC_HSI_BAR                        0x10  // BAR
#define B_PCH_SCC_HSI_BAR_BA                     0xFFFFF000 // Base Address
#define V_PCH_SCC_HSI_BAR_SIZE                   0x1000
#define N_PCH_SCC_HSI_BAR_ALIGNMENT              12
#define B_PCH_SCC_HSI_BAR_SI                     0x00000FF0 // Size Indicator
#define B_PCH_SCC_HSI_BAR_PF                     BIT3  // Prefetchable
#define B_PCH_SCC_HSI_BAR_TYPE                   (BIT2 | BIT1) // Type
#define B_PCH_SCC_HSI_BAR_MS                     BIT0  // Message Space

#define R_PCH_SCC_HSI_BAR1                       0x14  // BAR 1
#define B_PCH_SCC_HSI_BAR1_BA                    0xFFFFF000 // Base Address
#define B_PCH_SCC_HSI_BAR1_SI                    0x00000FF0 // Size Indicator
#define B_PCH_SCC_HSI_BAR1_PF                    BIT3  // Prefetchable
#define B_PCH_SCC_HSI_BAR1_TYPE                  (BIT2 | BIT1) // Type
#define B_PCH_SCC_HSI_BAR1_MS                    BIT0  // Message Space

#define R_PCH_SCC_HSI_SSID                       0x2C  // Sub System ID
#define B_PCH_SCC_HSI_SSID_SID                   0xFFFF0000 // Sub System ID
#define B_PCH_SCC_HSI_SSID_SVID                  0x0000FFFF // Sub System Vendor ID

#define R_PCH_SCC_HSI_ERBAR                      0x30  // Expansion ROM BAR
#define B_PCH_SCC_HSI_ERBAR_BA                   0xFFFFFFFF // Expansion ROM Base Address

#define R_PCH_SCC_HSI_CAPPTR                     0x34  // Capability Pointer
#define B_PCH_SCC_HSI_CAPPTR_CPPWR               0xFF  // Capability Pointer Power

#define R_PCH_SCC_HSI_INTR                       0x3C  // Interrupt
#define B_PCH_SCC_HSI_INTR_ML                    0xFF000000 // Max Latency
#define B_PCH_SCC_HSI_INTR_MG                    0x00FF0000
#define B_PCH_SCC_HSI_INTR_IP                    0x00000F00 // Interrupt Pin
#define B_PCH_SCC_HSI_INTR_IL                    0x000000FF // Interrupt Line

#define R_PCH_SCC_HSI_PCAPID                     0x80  // Power Capability ID
#define B_PCH_SCC_HSI_PCAPID_PS                  0xF8000000 // PME Support
#define B_PCH_SCC_HSI_PCAPID_VS                  0x00070000 // Version
#define B_PCH_SCC_HSI_PCAPID_NC                  0x0000FF00 // Next Capability
#define B_PCH_SCC_HSI_PCAPID_PC                  0x000000FF // Power Capability

#define R_PCH_SCC_HSI_PCS                        0x84  // PME Control Status
#define B_PCH_SCC_HSI_PCS_PMESTS                 BIT15 // PME Status
#define B_PCH_SCC_HSI_PCS_PMEEN                  BIT8  // PME Enable
#define B_PCH_SCC_HSI_PCS_NSS                    BIT3  // No Soft Reset
#define B_PCH_SCC_HSI_PCS_PS                     (BIT1 | BIT0) // Power State

#define R_PCH_SCC_HSI_MANID                      0xF8  // Manufacturer ID
#define B_PCH_SCC_HSI_MANID_MANID                0xFFFFFFFF // Manufacturer ID

// 
// GPSCORE
//
#define CFIO_SCORE_SB_PORT_ID                      0x48
#define DLL_CTRL_SCORE_MDL_FSM_CTRL                0x4970
#define DLL_INIT_SCORE_MDL_CF_INIT                 0x4964
#define DLL_WR_PATH_SCORE_MDL_WRITE_PATH_C_F_ADDR  0x4950
#define DLL_WR_PATH1_MUX_SCORE_DLL_WRITE_PATH1_MUX 0x4954
#define DLL_WR_PATH2_MUX_SCORE_DLL_WRITE_PATH2_MUX 0x4958
#define DLL_WR_PATH3_MUX_SCORE_DLL_WRITE_PATH3_MUX 0x495C
#define PAD_SDMMC1_CLK_PCONF0                      0x43e0
#define SDMMC1_CLK_PCONF1                          0x43E4
#define SDMMC2_CLK_PCONF1                          0x4324
#define SDMMC3_CLK_PCONF1                          0x42B4
#define DLL_VALS_SCORE_MDL_FSM_VALS                0x496C
#define MDL_FSM_VALS_MASK                          (UINT32)~(0xFFFFFFFF << 19)
///
/// SCC Private Space
///
#define PCH_SCC_EP_PORT_ID                       0x63  // SCC EP Private Space PortID
#define PCH_SCC_EP_PRIVATE_READ_OPCODE           0x06  // CUnit to SCC EP Private Space Read Opcode
#define PCH_SCC_EP_PRIVATE_WRITE_OPCODE          0x07  // CUnit to SCC EP Private Space Write Opcode

#define R_PCH_SCC_EP_IOSFCTL                     0x00  // IOSF Control
#define B_PCH_SCC_EP_IOSFCTL_NSNPDIS             BIT7  // Non-Snoop Disable

#define R_PCH_SCC_EP_PCICFGCTR1                  0x500 // PCI Configuration Control 1 - eMMC
#define B_PCH_SCC_EP_PCICFGCTR1_IPIN1            (BIT11 | BIT10 | BIT9 | BIT8) // Interrupt Pin
#define B_PCH_SCC_EP_PCICFGCTR1_B1D1             BIT7  // BAR 1 Disable
#define B_PCH_SCC_EP_PCICFGCTR1_PS               0x7C  // PME Support
#define B_PCH_SCC_EP_PCICFGCTR1_ACPI_INT_EN1     BIT1  // ACPI Interrupt Enable
#define B_PCH_SCC_EP_PCICFGCTR1_PCI_CFG_DIS1     BIT0  // PCI Configuration Space Disable

#define R_PCH_SCC_EP_PCICFGCTR2                  0x504 // PCI Configuration Control 2 - SD Card
#define B_PCH_SCC_EP_PCICFGCTR2_IPIN1            (BIT11 | BIT10 | BIT9 | BIT8) // Interrupt Pin
#define B_PCH_SCC_EP_PCICFGCTR2_B1D1             BIT7  // BAR 1 Disable
#define B_PCH_SCC_EP_PCICFGCTR2_PS               0x7C  // PME Support
#define B_PCH_SCC_EP_PCICFGCTR2_ACPI_INT_EN1     BIT1  // ACPI Interrupt Enable
#define B_PCH_SCC_EP_PCICFGCTR2_PCI_CFG_DIS1     BIT0  // PCI Configuration Space Disable

#define R_PCH_SCC_EP_PCICFGCTR3                  0x508 // PCI Configuration Control 3 - SDIO
#define B_PCH_SCC_EP_PCICFGCTR3_IPIN1            (BIT11 | BIT10 | BIT9 | BIT8) // Interrupt Pin
#define B_PCH_SCC_EP_PCICFGCTR3_B1D1             BIT7  // BAR 1 Disable
#define B_PCH_SCC_EP_PCICFGCTR3_PS               0x7C  // PME Support
#define B_PCH_SCC_EP_PCICFGCTR3_ACPI_INT_EN1     BIT1  // ACPI Interrupt Enable
#define B_PCH_SCC_EP_PCICFGCTR3_PCI_CFG_DIS1     BIT0  // PCI Configuration Space Disable

#define R_PCH_SCC_EP_PCICFGCTR4                  0x50C // PCI Configuration Control 4 - EMMC45
#define B_PCH_SCC_EP_PCICFGCTR4_IPIN1            (BIT11 | BIT10 | BIT9 | BIT8) // Interrupt Pin
#define B_PCH_SCC_EP_PCICFGCTR4_B1D1             BIT7  // BAR 1 Disable
#define B_PCH_SCC_EP_PCICFGCTR4_PS               0x7C  // PME Support
#define B_PCH_SCC_EP_PCICFGCTR4_ACPI_INT_EN1     BIT1  // ACPI Interrupt Enable
#define B_PCH_SCC_EP_PCICFGCTR4_PCI_CFG_DIS1     BIT0  // PCI Configuration Space Disable

#define R_PCH_SCC_EP_GENREGRW1                   0x600 // GEN_REGRW1
#define B_PCH_SCC_EP_GENREGRW1_SDIO_VOLT         (BIT4 | BIT5) // SDIO voltage support 3.3/3
#define V_PCH_SCC_EP_GENREGRW1_SDIO_VOLT_V3      0x30 // 3.0V Support in the Controller's capabilities.

#endif
