/**
  This file contains an 'Intel Peripheral Driver' and uniquely        
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your   
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2011 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  PchRegsSpi.h

  @brief
  Register names for PCH SPI device.
  
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
#ifndef _PCH_REGS_SPI_H_
#define _PCH_REGS_SPI_H_

///
/// SPI Host Interface Registers
///
#define R_PCH_SPI_BFPR                       0x00  // BIOS Flash Primary Region Register (32bits)
#define B_PCH_SPI_BFPR_PRL                   0x1FFF0000 // BIOS Flash Primary Region Limit
#define B_PCH_SPI_BFPR_PRB                   0x1FFF // BIOS Flash Primary Region Base

#define R_PCH_SPI_HSFS                       0x04  // Hardware Sequencing Flash Status Register (16bits)
#define B_PCH_SPI_HSFS_FLOCKDN               BIT15 // Flash Configuration Lock-Down
#define B_PCH_SPI_HSFS_FDV                   BIT14 // Flash Descriptor Valid
#define B_PCH_SPI_HSFS_FDOPSS                BIT13 // Flash Descriptor Override Pin-Strap Status
#define B_PCH_SPI_HSFS_SCIP                  BIT5  // SPI Cycle in Progress
#define B_PCH_SPI_HSFS_BERASE_MASK           (BIT4 | BIT3) // Block / Sector Erase Size
#define V_PCH_SPI_HSFS_BERASE_256B           0x00  // Block/Sector = 256 Bytes
#define V_PCH_SPI_HSFS_BERASE_4K             0x01  // Block/Sector = 4K Bytes
#define V_PCH_SPI_HSFS_BERASE_8K             0x10  // Block/Sector = 8K Bytes
#define V_PCH_SPI_HSFS_BERASE_64K            0x11  // Block/Sector = 64K Bytes
#define B_PCH_SPI_HSFS_AEL                   BIT2  // Access Error Log
#define B_PCH_SPI_HSFS_FCERR                 BIT1  // Flash Cycle Error
#define B_PCH_SPI_HSFS_FDONE                 BIT0  // Flash Cycle Done

#define R_PCH_SPI_HSFC                       0x06  // Hardware Sequencing Flash Control Register (16bits)
#define B_PCH_SPI_HSFC_FSMIE                 BIT15 // Flash SPI SMI# Enable
#define B_PCH_SPI_HSFC_FDBC_MASK             0x3F00 // Flash Data Byte Count ( <= 64), Count = (Value in this field) + 1.
#define B_PCH_SPI_HSFC_FCYCLE_MASK           0x0006 // Flash Cycle.
#define V_PCH_SPI_HSFC_FCYCLE_READ           0     // Flash Cycle Read
#define V_PCH_SPI_HSFC_FCYCLE_WRITE          2     // Flash Cycle Write
#define V_PCH_SPI_HSFC_FCYCLE_ERASE          3     // Flash Cycle Block Erase
#define B_PCH_SPI_HSFC_FCYCLE_FGO            BIT0  // Flash Cycle Go.

#define R_PCH_SPI_FADDR                      0x08  // SPI Flash Address
#define B_PCH_SPI_FADDR_MASK                 0x01FFFFFF // SPI Flash Address Mask (0~24bit)

#define R_PCH_SPI_FDATA00                    0x10  // SPI Data 00 (32 bits)
#define R_PCH_SPI_FDATA01                    0x14  // SPI Data 01
#define R_PCH_SPI_FDATA02                    0x18  // SPI Data 02
#define R_PCH_SPI_FDATA03                    0x1C  // SPI Data 03
#define R_PCH_SPI_FDATA04                    0x20  // SPI Data 04
#define R_PCH_SPI_FDATA05                    0x24  // SPI Data 05
#define R_PCH_SPI_FDATA06                    0x28  // SPI Data 06
#define R_PCH_SPI_FDATA07                    0x2C  // SPI Data 07
#define R_PCH_SPI_FDATA08                    0x30  // SPI Data 08
#define R_PCH_SPI_FDATA09                    0x34  // SPI Data 09
#define R_PCH_SPI_FDATA10                    0x38  // SPI Data 10
#define R_PCH_SPI_FDATA11                    0x3C  // SPI Data 11
#define R_PCH_SPI_FDATA12                    0x40  // SPI Data 12
#define R_PCH_SPI_FDATA13                    0x44  // SPI Data 13
#define R_PCH_SPI_FDATA14                    0x48  // SPI Data 14
#define R_PCH_SPI_FDATA15                    0x4C  // SPI Data 15

#define R_PCH_SPI_FRAP                       0x50  // SPI Flash Regions Access Permissions Register
#define B_PCH_SPI_FRAP_BMWAG_MASK            0xFF000000 // Master Write Access Grant MASK
#define B_PCH_SPI_FRAP_BMWAG_SEC             BIT26 // Master Write Access Grant for SEC
#define B_PCH_SPI_FRAP_BMWAG_BIOS            BIT25 // Master Write Access Grant for Host CPU/BIOS
#define B_PCH_SPI_FRAP_BMRAG_MASK            0x00FF0000 // Master Read Access Grant Grant MASK
#define B_PCH_SPI_FRAP_BMRAG_SEC             BIT18 // Master Read Access Grant for SEC
#define B_PCH_SPI_FRAP_BMRAG_BIOS            BIT17 // Master Read Access Grant for Host CPU/BIOS
#define B_PCH_SPI_FRAP_BRWA_MASK             0x0000FF00 // BIOS Region Write Access MASK
#define B_PCH_SPI_FRAP_BRWA_SEC              BIT10 // Region Write Access for Region2 SEC
#define B_PCH_SPI_FRAP_BRWA_BIOS             BIT9  // Region Write Access for Region1 BIOS
#define B_PCH_SPI_FRAP_BRWA_FLASHD           BIT8  // Region Write Access for Region0 Flash Descriptor
#define B_PCH_SPI_FRAP_BRRA_MASK             0x000000FF // BIOS Region Read Access MASK
#define B_PCH_SPI_FRAP_BRRA_SEC              BIT2  // Region Read Access for Region2 SEC
#define B_PCH_SPI_FRAP_BRRA_BIOS             BIT1  // Region Read Access for Region1 BIOS
#define B_PCH_SPI_FRAP_BRRA_FLASHD           BIT0  // Region Read Access for Region0 Flash Descriptor

#define R_PCH_SPI_FREG0_FLASHD               0x54  // Flash Region 0 (Flash Descriptor) (32bits)
#define B_PCH_SPI_FREG0_LIMIT_MASK           0x1FFF0000 // Size, [28:16] here represents limit[24:12]
#define B_PCH_SPI_FREG0_BASE_MASK            0x00001FFF // Base, [12:0]  here represents base [24:12]

#define R_PCH_SPI_FREG1_BIOS                 0x58  // Flash Region 1 (BIOS) (32bits)
#define B_PCH_SPI_FREG1_LIMIT_MASK           0x1FFF0000 // Size, [28:16] here represents limit[24:12]
#define B_PCH_SPI_FREG1_BASE_MASK            0x00001FFF // Base, [12:0]  here represents base [24:12]

#define R_PCH_SPI_FREG2_SEC                  0x5C  // Flash Region 2 (SEC) (32bits)
#define B_PCH_SPI_FREG2_LIMIT_MASK           0x1FFF0000 // Size, [28:16] here represents limit[24:12]
#define B_PCH_SPI_FREG2_BASE_MASK            0x00001FFF // Base, [12:0]  here represents base [24:12]

#define B_PCH_SPI_FREG3_LIMIT_MASK           0x1FFF0000 // Size, [28:16] here represents limit[24:12]
#define B_PCH_SPI_FREG3_BASE_MASK            0x00001FFF // Base, [12:0]  here represents base [24:12]

#define R_PCH_SPI_FREG4_PLATFORM_DATA        0x64  // Flash Region 4 (Platform Data) (32bits)
#define B_PCH_SPI_FREG4_LIMIT_MASK           0x1FFF0000 // Size, [28:16] here represents limit[24:12]
#define B_PCH_SPI_FREG4_BASE_MASK            0x00001FFF // Base, [12:0]  here represents base [24:12]

#define R_PCH_SPI_PR0                        0x74  // Protected Region 0 Register
#define B_PCH_SPI_PR0_WPE                    BIT31 // Write Protection Enable
#define B_PCH_SPI_PR0_PRL_MASK               0x1FFF0000 // Protected Range Limit Mask, [28:16] here represents upper limit of address [24:12]
#define B_PCH_SPI_PR0_RPE                    BIT15 // Read Protection Enable
#define B_PCH_SPI_PR0_PRB_MASK               0x00001FFF // Protected Range Base Mask, [12:0] here represents base limit of address [24:12]

#define R_PCH_SPI_PR1                        0x78  // Protected Region 1 Register
#define B_PCH_SPI_PR1_WPE                    BIT31 // Write Protection Enable
#define B_PCH_SPI_PR1_PRL_MASK               0x1FFF0000 // Protected Range Limit Mask
#define B_PCH_SPI_PR1_RPE                    BIT15 // Read Protection Enable
#define B_PCH_SPI_PR1_PRB_MASK               0x00001FFF // Protected Range Base Mask

#define R_PCH_SPI_PR2                        0x7C  // Protected Region 2 Register
#define B_PCH_SPI_PR2_WPE                    BIT31 // Write Protection Enable
#define B_PCH_SPI_PR2_PRL_MASK               0x1FFF0000 // Protected Range Limit Mask
#define B_PCH_SPI_PR2_RPE                    BIT15 // Read Protection Enable
#define B_PCH_SPI_PR2_PRB_MASK               0x00001FFF // Protected Range Base Mask

#define R_PCH_SPI_PR3                        0x80  // Protected Region 3 Register
#define B_PCH_SPI_PR3_WPE                    BIT31 // Write Protection Enable
#define B_PCH_SPI_PR3_PRL_MASK               0x1FFF0000 // Protected Range Limit Mask
#define B_PCH_SPI_PR3_RPE                    BIT15 // Read Protection Enable
#define B_PCH_SPI_PR3_PRB_MASK               0x00001FFF // Protected Range Base Mask

#define R_PCH_SPI_PR4                        0x84  // Protected Region 4 Register
#define B_PCH_SPI_PR4_WPE                    BIT31 // Write Protection Enable
#define B_PCH_SPI_PR4_PRL_MASK               0x1FFF0000 // Protected Range Limit Mask
#define B_PCH_SPI_PR4_RPE                    BIT15 // Read Protection Enable
#define B_PCH_SPI_PR4_PRB_MASK               0x00001FFF // Protected Range Base Mask

#define R_PCH_SPI_SSFCS                      0x90  // Software Sequencing Flash Control Status Register
#define B_PCH_SPI_SSFCS_SCF_MASK             (BIT26 | BIT25 | BIT24) // SPI Cycle Frequency
#define V_PCH_SPI_SSFCS_SCF_20MHZ            0     // SPI Cycle Frequency = 20MHz
#define V_PCH_SPI_SSFCS_SCF_33MHZ            1     // SPI Cycle Frequency = 33MHz
#define V_PCH_SPI_SSFCS_SCF_50MHZ            4     // SPI Cycle Frequency = 50MHz
#define B_PCH_SPI_SSFCS_SME                  BIT23 // SPI SMI# Enable
#define B_PCH_SPI_SSFCS_DC                   BIT22 // SPI Data Cycle
#define B_PCH_SPI_SSFCS_DBC_MASK             0x3F0000 // SPI Data Byte Count (value here + 1 = count)
#define B_PCH_SPI_SSFCS_COP                  0x7000 // Cycle Opcode Pointer
#define B_PCH_SPI_SSFCS_SPOP                 BIT11 // Sequence Prefix Opcode Pointer
#define B_PCH_SPI_SSFCS_ACS                  BIT10 // Atomic Cycle Sequence
#define B_PCH_SPI_SSFCS_SCGO                 BIT9  // SPI Cycle Go
#define B_PCH_SPI_SSFCS_FRS                  BIT7  // Fast Read Supported
#define B_PCH_SPI_SSFCS_DOFRS                BIT6  // Dual Output Fast Read Supported
#define B_PCH_SPI_SSFCS_AEL                  BIT4  // Access Error Log
#define B_PCH_SPI_SSFCS_FCERR                BIT3  // Flash Cycle Error
#define B_PCH_SPI_SSFCS_CDS                  BIT2  // Cycle Done Status
#define B_PCH_SPI_SSFCS_SCIP                 BIT0  // SPI Cycle in Progress

#define R_PCH_SPI_PREOP                      0x94  // Prefix Opcode Configuration Register (16 bits)
#define B_PCH_SPI_PREOP1_MASK                0xFF00 // Prefix Opcode 1 Mask
#define B_PCH_SPI_PREOP0_MASK                0x00FF // Prefix Opcode 0 Mask

#define R_PCH_SPI_OPTYPE                     0x96  // Opcode Type Configuration
#define B_PCH_SPI_OPTYPE7_MASK               (BIT15 | BIT14) // Opcode Type 7 Mask
#define B_PCH_SPI_OPTYPE6_MASK               (BIT13 | BIT12) // Opcode Type 6 Mask
#define B_PCH_SPI_OPTYPE5_MASK               (BIT11 | BIT10) // Opcode Type 5 Mask
#define B_PCH_SPI_OPTYPE4_MASK               (BIT9 | BIT8) // Opcode Type 4 Mask
#define B_PCH_SPI_OPTYPE3_MASK               (BIT7 | BIT6) // Opcode Type 3 Mask
#define B_PCH_SPI_OPTYPE2_MASK               (BIT5 | BIT4) // Opcode Type 2 Mask
#define B_PCH_SPI_OPTYPE1_MASK               (BIT3 | BIT2) // Opcode Type 1 Mask
#define B_PCH_SPI_OPTYPE0_MASK               (BIT1 | BIT0) // Opcode Type 0 Mask
#define V_PCH_SPI_OPTYPE_RDNOADDR            0x00  // Read cycle type without address
#define V_PCH_SPI_OPTYPE_WRNOADDR            0x01  // Write cycle type without address
#define V_PCH_SPI_OPTYPE_RDADDR              0x02  // Address required; Read cycle type
#define V_PCH_SPI_OPTYPE_WRADDR              0x03  // Address required; Write cycle type

#define R_PCH_SPI_OPMENU0                    0x98  // Opcode Menu Configuration 0 (32bits)
#define R_PCH_SPI_OPMENU1                    0x9C  // Opcode Menu Configuration 1 (32bits)

#define R_PCH_SPI_IND_LOCK                   0xA4  // Indvidual Lock
#define B_PCH_SPI_IND_LOCK_PR0               BIT2  // PR0 LockDown


#define R_PCH_SPI_FDOC                       0xB0  // Flash Descriptor Observability Control Register (32 bits)
#define B_PCH_SPI_FDOC_FDSS_MASK             (BIT14 | BIT13 | BIT12) // Flash Descriptor Section Select
#define V_PCH_SPI_FDOC_FDSS_FSDM             0x0000 // Flash Signature and Descriptor Map
#define V_PCH_SPI_FDOC_FDSS_COMP             0x1000 // Component
#define V_PCH_SPI_FDOC_FDSS_REGN             0x2000 // Region
#define V_PCH_SPI_FDOC_FDSS_MSTR             0x3000 // Master
#define V_PCH_SPI_FDOC_FDSS_VLVS             0x4000 // Soft Straps
#define B_PCH_SPI_FDOC_FDSI_MASK             0x0FFC // Flash Descriptor Section Index

#define R_PCH_SPI_FDOD                       0xB4  // Flash Descriptor Observability Data Register (32 bits)

#define R_PCH_SPI_AFC                        0xC0  // Additional Flash Control Register
#define B_PCH_SPI_AFC_RRWSP                  (BIT7 | BIT6 | BIT5 | BIT4) // Reserved RW Scratch Pad
#define B_PCH_SPI_AFC_SPFP                   BIT3  // Stop Prefetch on Flush Pending
#define B_PCH_SPI_AFC_INF_DCGE               (BIT2 | BIT1) // Flash Controller Interface Dynamic Clock Gating Enable
#define B_PCH_SPI_AFC_CORE_DCGE              BIT0  // Flash Core Dynamic Clock Gating Enable

#define R_PCH_SPI_LVSCC                      0xC4  // Lower Vendor Specific Component Capabilities Register (32 bits)
#define B_PCH_SPI_LVSCC_VCL                  BIT23 // Vendor Component Lock
#define B_PCH_SPI_LVSCC_EO_MASK              0x0000FF00 // Erase Opcode
#define B_PCH_SPI_LVSCC_WEWS                 BIT4  // Write Enable on Write Status
#define B_PCH_SPI_LVSCC_WSR                  BIT3  // Write Status Required
#define B_PCH_SPI_LVSCC_WG_64B               BIT2  // Write Granularity, 0: 1 Byte; 1: 64 Bytes
#define B_PCH_SPI_LVSCC_BSES_MASK            (BIT1 | BIT0) // Block/Sector Erase Size
#define V_PCH_SPI_LVSCC_BSES_256B            0x0   // Block/Sector Erase Size = 256 Bytes
#define V_PCH_SPI_LVSCC_BSES_4K              0x1   // Block/Sector Erase Size = 4K Bytes
#define V_PCH_SPI_LVSCC_BSES_8K              0x2   // Block/Sector Erase Size = 8K Bytes
#define V_PCH_SPI_LVSCC_BSES_64K             0x3   // Block/Sector Erase Size = 64K Bytes

#define R_PCH_SPI_UVSCC                      0xC8  // Upper Vendor Specific Component Capabilities Register (32 bits)
#define B_PCH_SPI_UVSCC_EO_MASK              0x0000FF00 // Erase Opcode
#define B_PCH_SPI_UVSCC_WEWS                 BIT4  // Write Enable on Write Status
#define B_PCH_SPI_UVSCC_WSR                  BIT3  // Write Status Required
#define B_PCH_SPI_UVSCC_WG_64B               BIT2  // Write Granularity, 0: 1 Byte; 1: 64 Bytes
#define B_PCH_SPI_UVSCC_BSES_MASK            (BIT1 | BIT0) // Block/Sector Erase Size
#define V_PCH_SPI_UVSCC_BSES_256B            0x0   // Block/Sector Erase Size = 256 Bytes
#define V_PCH_SPI_UVSCC_BSES_4K              0x1   // Block/Sector Erase Size = 4K Bytes
#define V_PCH_SPI_UVSCC_BSES_8K              0x2   // Block/Sector Erase Size = 8K Bytes
#define V_PCH_SPI_UVSCC_BSES_64K             0x3   // Block/Sector Erase Size = 64K Bytes

#define R_PCH_SPI_FPB                        0xD0  // Flash Partition Boundary
#define B_PCH_SPI_FPB_FPBA_MASK              0x00001FFF // Flash Partition Boundary Address Mask, reflecting FPBA[24:12]

#define R_PCH_SPI_SCS                        0xF8  // SMI Control Status Register
#define S_PCH_SPI_SCS                        1
#define B_PCH_SPI_SCS_SMIWPEN                BIT7  // SMI WPD Enable
#define B_PCH_SPI_SCS_SMIWPST                BIT6  // SMI WPD Status
#define N_PCH_SPI_SCS_SMIWPEN                7
#define N_PCH_SPI_SCS_SMIWPST                6

#define R_PCH_SPI_BCR                        0xFC  // BIOS Control Register
#define S_PCH_SPI_BCR                        1
#define B_PCH_SPI_BCR_SMM_BWP                BIT5  // SMM BIOS Write Protect Disable
#define B_PCH_SPI_BCR_SRC                    (BIT3 | BIT2) // SPI Read Configuration (SRC)
#define V_PCH_SPI_BCR_SRC_PREF_EN_CACHE_EN   0x08  // Prefetch Enable, Cache Enable
#define V_PCH_SPI_BCR_SRC_PREF_DIS_CACHE_DIS 0x04  // Prefetch Disable, Cache Disable
#define V_PCH_SPI_BCR_SRC_PREF_DIS_CACHE_EN  0x00  // Prefetch Disable, Cache Enable
#define B_PCH_SPI_BCR_BLE                    BIT1  // Lock Enable (LE)
#define B_PCH_SPI_BCR_BIOSWE                 BIT0  // Write Protect Disable (WPD)
#define N_PCH_SPI_BCR_BLE                    1
#define N_PCH_SPI_BCR_BIOSWE                 0

#define R_PCH_SPI_TCGC                       0x100 // Trunk Clock Gating Control
#define B_PCH_SPI_TCGC_FCGDIS                BIT10 // Functional Clock Gating Disable
#define B_PCH_SPI_TCGC_SBCGCDEF              BIT9  // Sideband Control Gating Clock Defeature
#define B_PCH_SPI_TCGC_SBCGEN                BIT8  // Sideband Control Gating Clock Enable
#define B_PCH_SPI_TCGC_SBCGCNT               0xFF  // Sideband Control Gating Clock Counter

//
// Flash Descriptor Base Address Region (FDBAR) from Flash Region 0
//
#define R_PCH_SPI_FDBAR_FLVALSIG             0x00  // Flash Valid Signature
#define V_PCH_SPI_FDBAR_FLVALSIG             0x0FF0A55A

#define R_PCH_SPI_FDBAR_FLASH_MAP0           0x04  // Flash MAP 0
#define B_PCH_SPI_FDBAR_NR                   0x07000000 // Number Of Regions
#define B_PCH_SPI_FDBAR_FRBA                 0x00FF0000 // Flash Region Base Address
#define B_PCH_SPI_FDBAR_NC                   0x00000300 // Number Of Components
#define V_PCH_SPI_FDBAR_NC_2                 0x00000100
#define V_PCH_SPI_FDBAR_NC_1                 0x00000000
#define B_PCH_SPI_FDBAR_FCBA                 0x000000FF // Flash Component Base Address

#define R_PCH_SPI_FDBAR_FLASH_MAP1           0x08  // Flash MAP 1
#define B_PCH_SPI_FDBAR_ISL                  0xFF000000 // Strap Length
#define B_PCH_SPI_FDBAR_FISBA                0x00FF0000 // Flash Strap Base Address
#define B_PCH_SPI_FDBAR_NM                   0x00000700 // Number Of Masters
#define B_PCH_SPI_FDBAR_FMBA                 0x000000FF // Flash Master Base Address

#define R_PCH_SPI_FDBAR_FLASH_MAP2           0x0C  // Flash Map 2
#define B_PCH_SPI_FDBAR_ICCRIL               0xFF000000 // ICC Register Init Length
#define B_PCH_SPI_FDBAR_ICCRIBA              0x00FF0000 // ICC Register Init Base Address
#define B_PCH_SPI_FDBAR_CPUSL                0x0000FF00 // CPU Strap Length
#define B_PCH_SPI_FDBAR_FCPUSBA              0x000000FF // Flash CPU Strap Base Address

//
// Flash Component Base Address (FCBA) from Flash Region 0
//
#define R_PCH_SPI_FCBA_FLCOMP                0x00  // Flash Components Register
#define B_PCH_SPI_FLCOMP_DOFRS               BIT30 // Dual Output Fast Read Support
#define B_PCH_SPI_FLCOMP_RIDS_FREQ           (BIT29 | BIT28 | BIT27) // Read ID and Read Status Clock Frequency
#define B_PCH_SPI_FLCOMP_WE_FREQ             (BIT26 | BIT25 | BIT24) // Write and Erase Clock Frequency
#define B_PCH_SPI_FLCOMP_FRCF_FREQ           (BIT23 | BIT22 | BIT21) // Fast Read Clock Frequency
#define B_PCH_SPI_FLCOMP_FR_SUP              BIT20 // Fast Read Support.
#define B_PCH_SPI_FLCOMP_RC_FREQ             (BIT19 | BIT18 | BIT17) // Read Clock Frequency.
#define V_PCH_SPI_FLCOMP_FREQ_20MHZ          0x00
#define B_PCH_SPI_FLCOMP_COMP2_MASK          0x38  // Flash Component 2 Density
#define V_PCH_SPI_FLCOMP_COMP2_512KB         0x00
#define V_PCH_SPI_FLCOMP_COMP2_1MB           0x08
#define V_PCH_SPI_FLCOMP_COMP2_2MB           0x10
#define V_PCH_SPI_FLCOMP_COMP2_4MB           0x18
#define V_PCH_SPI_FLCOMP_COMP2_8MB           0x20
#define V_PCH_SPI_FLCOMP_COMP2_16MB          0x28
#define B_PCH_SPI_FLCOMP_COMP1_MASK          0x07  // Flash Component 1 Density
#define V_PCH_SPI_FLCOMP_COMP1_512KB         0x00
#define V_PCH_SPI_FLCOMP_COMP1_1MB           0x01
#define V_PCH_SPI_FLCOMP_COMP1_2MB           0x02
#define V_PCH_SPI_FLCOMP_COMP1_4MB           0x03
#define V_PCH_SPI_FLCOMP_COMP1_8MB           0x04
#define V_PCH_SPI_FLCOMP_COMP1_16MB          0x05

///
/// Flash Soft Strap Base Address (FISBA) from Flash Region 0
///
#define R_PCH_SPI_STRP_SPI                   0x00  // SPI Soft Straps
#define B_PCH_SPI_STRP_SPI_BPWPE             BIT31 // BIOS PR4 Write Protection Enable
#define B_PCH_SPI_STRP_SPI_BPR4L             0x7FFC0000 // BIOS Protected Range for Limit
#define B_PCH_SPI_STRP_SPI_BPR4B             0x0003FFE0 // BIOS Protected Range for Base
#define B_PCH_SPI_STRP_SPI_DNSWSC            BIT1  // Do Not Sample With SPI Clock
#define B_PCH_SPI_STRP_SPI_SEC_REG_OPEN      BIT0  // SEC Register Open

#define R_PCH_SPI_STRP_PCU0                  0x20  // PCU-EP Soft Straps
#define B_PCH_SPI_STRP_PCU0_BBBS             (BIT0 | BIT1) // SPI Boot Block size
#define B_PCH_SPI_STRP_PCU0_BBBS_256KB       BIT1
#define B_PCH_SPI_STRP_PCU0_BBBS_128KB       BIT0
#define B_PCH_SPI_STRP_PCU0_BBBS_64KB        0x00

#define R_PCH_SPI_STRP_PCU1                  0x30  // PMC Soft Straps 0
#define B_PCH_SPI_STRP_PCU1_LPIO2_F7_DIS     BIT31 // Disable LPIO 2 Function 7
#define B_PCH_SPI_STRP_PCU1_LPIO2_F6_DIS     BIT30 // Disable LPIO 2 Function 6
#define B_PCH_SPI_STRP_PCU1_LPIO2_F5_DIS     BIT29 // Disable LPIO 2 Function 5
#define B_PCH_SPI_STRP_PCU1_LPIO2_F4_DIS     BIT28 // Disable LPIO 2 Function 4
#define B_PCH_SPI_STRP_PCU1_LPIO2_F3_DIS     BIT27 // Disable LPIO 2 Function 3
#define B_PCH_SPI_STRP_PCU1_LPIO2_F2_DIS     BIT26 // Disable LPIO 2 Function 2
#define B_PCH_SPI_STRP_PCU1_LPIO2_F1_DIS     BIT25 // Disable LPIO 2 Function 1
#define B_PCH_SPI_STRP_PCU1_LPIO2_F0_DIS     BIT24 // Disable LPIO 2 Function 0
#define B_PCH_SPI_STRP_PCU1_PCIE3_DIS        BIT23 // Disable PCIe Port 3
#define B_PCH_SPI_STRP_PCU1_PCIE2_DIS        BIT22 // Disable PCIe Port 2
#define B_PCH_SPI_STRP_PCU1_PCIE1_DIS        BIT21 // Disable PCIe Port 1
#define B_PCH_SPI_STRP_PCU1_PCIE0_DIS        BIT20 // Disable PCIe Port 0
#define B_PCH_SPI_STRP_PCU1_SEC_DIS          BIT19 // Disable SeC
#define B_PCH_SPI_STRP_PCU1_USB_DIS          BIT18 // Disable USB
#define B_PCH_SPI_STRP_PCU1_SATA_DIS         BIT17 // Disable SATA
#define B_PCH_SPI_STRP_PCU1_USH_DIS          BIT15 // Disable USH
#define B_PCH_SPI_STRP_PCU1_OTG_DIS          BIT14 // Disable OTG
#define B_PCH_SPI_STRP_PCU1_LPE_DIS          BIT13 // Disable LPE Audio
#define B_PCH_SPI_STRP_PCU1_HDA_DIS          BIT12 // Disable HD Audio
#define B_PCH_SPI_STRP_PCU1_MIPI_DIS         BIT11 // Disable MIPI-HSI
#define B_PCH_SPI_STRP_PCU1_SCC_SDCARD_DIS   BIT10 // Disable SCC SD Card
#define B_PCH_SPI_STRP_PCU1_SCC_SDIO_DIS     BIT9  // Disable SCC SDIO
#define B_PCH_SPI_STRP_PCU1_SCC_EMMC_DIS     BIT8  // Disable SCC eMMC
#define B_PCH_SPI_STRP0_LPSS_F7_DIS          BIT9  // Disable LPSS Function 7
#define B_PCH_SPI_STRP0_LPSS_F6_DIS          BIT8  // Disable LPSS Function 6
#define B_PCH_SPI_STRP0_LPSS_F5_DIS          BIT7  // Disable LPSS Function 5
#define B_PCH_SPI_STRP0_LPSS_F4_DIS          BIT6  // Disable LPSS Function 4
#define B_PCH_SPI_STRP0_LPSS_F3_DIS          BIT5  // Disable LPSS Function 3
#define B_PCH_SPI_STRP0_LPSS_F2_DIS          BIT4  // Disable LPSS Function 2
#define B_PCH_SPI_STRP0_LPSS_F1_DIS          BIT3  // Disable LPSS Function 1
#define B_PCH_SPI_STRP0_LPSS_F0_DIS          BIT2  // Disable LPSS Function 0

#define R_PCH_SPI_STRP_PCU2                  0x34  // PMC Soft Straps 1
#define B_PCH_SPI_STRP_PCU2_LPCCLK_SLC       BIT61 // Select LPC Return Clock Source
#define B_PCH_SPI_STRP_PCU2_LPCCLK1_EN       BIT60 // LPCCLK1 Enable
#define B_PCH_SPI_STRP_PCU2_LPC_GPIO         BIT48 // Usage of the LPC Pins
#define B_PCH_SPI_STRP_PCU2_SSCNOS           0x3E0000000 // SSC Number of Steps
#define B_PCH_SPI_STRP_PCU2_SSCNOSS          0x1F000000 // SSC Number of Substeps
#define B_PCH_SPI_STRP_PCU2_SSCCPSS          0x00FF0000 // SSC Cycles Per Substep
#define B_PCH_SPI_STRP_PCU2_USH_SS_DIS       BIT10 // Disable USH SS
#define B_PCH_SPI_STRP_PCU2_OTG_SS_DIS       BIT9  // Disable OTG SS
#define B_PCH_SPI_STRP_PCU2_SMBUS_DIS        BIT8  // Disable SMBus
#define B_PCH_SPI_STRP_PCU2_SATACLK_SSEN     BIT7  // Enable Spread Spectrum to SATA
#define B_PCH_SPI_STRP_PCU2_PCICLK_SSEN      BIT6  // Enable Spread Spectrum to PCIe
#define B_PCH_SPI_STRP_PCU2_HFHPLLCLK_SSEN   BIT5  // Enable Spread Spectrum to HFHPLL
#define B_PCH_SPI_STRP_PCU2_DISPSSCLK_SSEN   BIT4  // Enable Spread Spectrum to DISPLAY SS
#define B_PCH_SPI_STRP_PCU2_DISBENDCLK_SSEN  BIT3  // Enable Spread Spectrum to DISPLAY BEND
#define B_PCH_SPI_STRP_PCU2_S3_HOT_EN        BIT2  // Supporting S3 Hot
#define B_PCH_SPI_STRP_PCU2_NO_REBOOT        BIT1  // Disable PLTRST After TCO WDT Second Time Expiration
#define B_PCH_SPI_STRP_PCU2_PMC_PATCH_TO_DIS BIT0  // PMC Patch Timeout Disable
#define R_PCH_SPI_STRP_PCIE0                 0x40  // PCIe Soft Straps 0
#define B_PCH_SPI_STRP_PCIE0_FRST            BIT22 // Fast Reset
#define B_PCH_SPI_STRP_PCIE0_RP_CONF         (BIT12 | BIT11) // Root Port Configuration
#define B_PCH_SPI_STRP_PCIE0_LANE_REVERSAL   BIT10 // Lane Reversal
#define B_PCH_SPI_STRP_PCIE0_STGREN          BIT0  // Port Staggering Enable

#define R_PCH_SPI_STRP_PCIE1                 0x44  // PCIe Soft Straps 1
#define B_PCH_SPI_STRP_PCIE1_U3P5PP2M        (BIT13 | BIT12) // USB3 / PCIe Shared Port 5 and PCIe Port 2 Mode
#define B_PCH_SPI_STRP_PCIE1_U3P4PP1M        (BIT13 | BIT12) // USB3 / PCIe Shared Port 4 and PCIe Port 1 Mode
#define B_PCH_SPI_STRP_PCIE1_SP4PP1MS        (BIT11 | BIT10) // SATA / PCIe Shared Port for SATA Port 4 and PCIe Port 1 Mode
#define B_PCH_SPI_STRP_PCIE1_SP5PP2MS        (BIT9 | BIT8) // SATA / PCIe Shared Port for SATA Port 5 and PCIe Port 2 Mode
#define B_PCH_SPI_STRP_PCIE1_PCIEPORTSEL     (BIT3 | BIT2 | BIT1) // GbE PCIe Port Select
#define B_PCH_SPI_STRP_PCIE1_GBE_PCIE_PEN    BIT0  // GbE Over PCIe Enable

#define R_PCH_SPI_STRP_FEP0                  0x48  // Fuse EP Soft Straps 0
#define B_PCH_SPI_STRP_FEP0_PCIELANE4PWREN   BIT11 // PCIe Enables Power for Digital Logic in the Data Lane 4
#define B_PCH_SPI_STRP_FEP0_PCIELANE3PWREN   BIT10 // PCIe Enables Power for Digital Logic in the Data Lane 3
#define B_PCH_SPI_STRP_FEP0_PCIELANE2PWREN   BIT9  // PCIe Enables Power for Digital Logic in the Data Lane 2
#define B_PCH_SPI_STRP_FEP0_PCIELANE1PWREN   BIT8  // PCIe Enables Power for Digital Logic in the Data Lane 1
#define B_PCH_SPI_STRP_FEP0_PCIELANE0PWREN   BIT7  // PCIe Enables Power for Digital Logic in the Data Lane 0
#define B_PCH_SPI_STRP_FEP0_PCIECMNLNPWREN   BIT6  // PCIe Power Enable for Common Logic 0
#define B_PCH_SPI_STRP_FEP0_PCIEREFCLKBUFEN  BIT2  // PCIe Reference Clock Input Buffer Enable from the Core
#define B_PCH_SPI_STRP_FEP0_PCIECRICLKSEL    BIT1  // PCIe Enabled using a Core-Supplied Ref-Clock for CRI Clock
#define B_PCH_SPI_STRP_FEP0_PCIEPLLREFSEL    BIT0  // PCIe PLL Reference Clock Select

#define R_PCH_SPI_STRP_FEP1                  0x4C  // Fuse EP Soft Straps 1
#define B_PCH_SPI_STRP_FEP1_SATASTS2PMC      BIT9  // SATA Status to PMC
#define B_PCH_SPI_STRP_FEP1_SATALANE1PWREN   BIT8  // SATA Enables Power for Digital Logic in the Data Lane 1
#define B_PCH_SPI_STRP_FEP1_SATALANE0PWREN   BIT7  // SATA Enables Power for Digital Logic in the Data Lane 0
#define B_PCH_SPI_STRP_FEP1_SATACMNLNPWREN   BIT6  // SATA Power Enable for Common Logic 0
#define B_PCH_SPI_STRP_FEP1_SATAREFCLKBUFEN  BIT2  // SATA Reference Clock Input Buffer Enable from the Core
#define B_PCH_SPI_STRP_FEP1_SATACRICLKSEL    BIT1  // SATA Enabled using a Core-Supplied Ref-Clock for CRI Clock
#define B_PCH_SPI_STRP_FEP1_SATAPLLREFSEL    BIT0  // SATA PLL Reference Clock Select

#define R_PCH_SPI_STRP_USH                   0x50  // USH USB3 SS PHY
#define B_PCH_SPI_STRP_USH_EDRCR             BIT7  // SS PHY Differential Reference Clock Receiver Enable
#define B_PCH_SPI_STRP_USH_SSPCRCS           BIT6  // PMA Common Internally Generated Reference Clock Select
#define B_PCH_SPI_STRP_USH_SSPPRCF           0x3F  // SS PHY Common Reference Clock Frequency Select

#define R_PCH_SPI_STRP_OTG                   0x54  // OTG
#define B_PCH_SPI_STRP_OTG_EDRCR             BIT7  // SS PHY Differential Reference Clock Receiver Enable
#define B_PCH_SPI_STRP_OTG_SSPCRCS           BIT6  // PMA Common Internally Generated Reference Clock Select
#define B_PCH_SPI_STRP_OTG_SSPPRCF           0x3F  // SS PHY Common Reference Clock Frequency Select

///
/// Descriptor Upper Map Section from Flash Region 0
///
#define R_PCH_SPI_FLASH_UMAP1                0xEFC // Flash Upper Map 1
#define B_PCH_SPI_FLASH_UMAP1_VTL            0x0000FF00 // VSCC Table Length
#define B_PCH_SPI_FLASH_UMAP1_VTBA           0x000000FF // VSCC Table Base Address

#define R_PCH_SPI_VTBA_JID0                  0x00  // JEDEC-ID 0 Register
#define S_PCH_SPI_VTBA_JID0                  4
#define B_PCH_SPI_VTBA_JID0_DID1             0x00FF0000 // SPI Component Device ID 1
#define N_PCH_SPI_VTBA_JID0_DID1             0x10
#define B_PCH_SPI_VTBA_JID0_DID0             0x0000FF00 // SPI Component Device ID 0
#define N_PCH_SPI_VTBA_JID0_DID0             0x08
#define B_PCH_SPI_VTBA_JID0_VID              0x000000FF // SPI Component Vendor ID

#define R_PCH_SPI_VTBA_VSCC0                 0x04  // Vendor Specific Component Capabilities 0
#define S_PCH_SPI_VTBA_VSCC0                 4
#define B_PCH_SPI_VTBA_VSCC0_UCAPS           0xFFFF0000
#define B_PCH_SPI_VTBA_VSCC0_LCAPS           0x0000FFFF
#define B_PCH_SPI_VTBA_VSCC0_EO              0x0000FF00 // Erase Opcode
#define B_PCH_SPI_VTBA_VSCC0_WEWS            BIT4  // Write Enable on Write Status
#define B_PCH_SPI_VTBA_VSCC0_WSR             BIT3  // Write Status Required
#define B_PCH_SPI_VTBA_VSCC0_WG              BIT2  // Write Granularity
#define B_PCH_SPI_VTBA_VSCC0_BES             (BIT1 | BIT0) // Block / Sector Erase Size

#endif
