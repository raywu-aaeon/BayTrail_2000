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

  PchRegsLpe.h

Abstract:

  Register names for VLV Low Power Audio device.

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
#ifndef _PCH_REGS_LPE_H_
#define _PCH_REGS_LPE_H_

///
/// LPE Config Registers (D21:F0)
///
#define PCI_DEVICE_NUMBER_PCH_LPE          21
#define PCI_FUNCTION_NUMBER_PCH_LPE        0

#define R_PCH_LPE_DEVVENID                 0x00  // Device / Vendor ID
#define B_PCH_LPE_DEVVENID_DEVICE_ID       0xFFFF0000 // Device ID
#define B_PCH_LPE_DEVVENID_VENDOR_ID       0x0000FFFF // Vendor ID
#define V_PCH_LPE_DEVVENID_VENDOR_ID       V_PCH_INTEL_VENDOR_ID // Intel Vendor ID
#define V_PCH_LPE_DEVICE_ID_0              0x0F28

#define R_PCH_LPE_STSCMD                   0x04  // Status Command
#define B_PCH_LPE_STSCMD_RMA               BIT29 // Received Master Abort
#define B_PCH_LPE_STSCMD_RCA               BIT28 // RCA
#define B_PCH_LPE_STSCMD_CAP_LST           BIT20 // Capabilities List
#define B_PCH_LPE_STSCMD_INTR_STS          BIT19 // Interrupt Status
#define B_PCH_LPE_STSCMD_INTR_DIS          BIT10 // Interrupt Disable
#define B_PCH_LPE_STSCMD_SERR_EN           BIT8  // SERR Enable
#define B_PCH_LPE_STSCMD_BME               BIT2  // Bus Master Enable
#define B_PCH_LPE_STSCMD_MSE               BIT1  // Memory Space Enable

#define R_PCH_LPE_RID_CC                   0x08  // Revision ID and Class Code
#define B_PCH_LPE_RID_CC_BCC               0xFF000000 // Base Class Code
#define B_PCH_LPE_RID_CC_SCC               0x00FF0000 // Sub Class Code
#define B_PCH_LPE_RID_CC_PI                0x0000FF00 // Programming Interface
#define B_PCH_LPE_RID_CC_RID               0x000000FF // Revision Identification

#define R_PCH_LPE_BAR0                     0x10  // BAR 0
#define B_PCH_LPE_BAR0_BA                  0xFFE00000 // Base Address
#define V_PCH_LPE_BAR0_SIZE                0x200000
#define N_PCH_LPE_BAR0_ALIGNMENT           21
#define B_PCH_LPE_BAR0_PREF                BIT3  // Prefetchable
#define B_PCH_LPE_BAR0_ADDRNG              (BIT2 | BIT1) // Address Range
#define B_PCH_LPE_BAR0_SPTYP               BIT0  // Space Type (Memory)
//#define V_PCH_LPE_BAR0_SIZE                (1 << 12)

#define R_PCH_LPE_BAR1                     0x14  // BAR 1
#define B_PCH_LPE_BAR1_BA                  0xFFFFF000 // Base Address
#define B_PCH_LPE_BAR1_PREF                BIT3  // Prefetchable
#define B_PCH_LPE_BAR1_ADDRNG              (BIT2 | BIT1) // Address Range
#define B_PCH_LPE_BAR1_SPTYP               BIT0  // Space Type (Memory)
#define V_PCH_LPE_BAR1_SIZE                (1 << 12)

#define R_PCH_LPE_SSID                     0x2C  // Sub System ID
#define B_PCH_LPE_SSID_SID                 0xFFFF0000 // Sub System ID
#define B_PCH_LPE_SSID_SVID                0x0000FFFF // Sub System Vendor ID

#define R_PCH_LPE_ERBAR                    0x30  // Expansion ROM BAR
#define B_PCH_LPE_ERBAR_BA                 0xFFFFFFFF // Expansion ROM Base Address

#define R_PCH_LPE_CAPPTR                   0x34  // Capability Pointer
#define B_PCH_LPE_CAPPTR_CPPWR             0xFF  // Capability Pointer Power

#define R_PCH_LPE_INTR                     0x3C  // Interrupt
#define B_PCH_LPE_INTR_ML                  0xFF000000 // Max Latency
#define B_PCH_LPE_INTR_MG                  0x00FF0000
#define B_PCH_LPE_INTR_IP                  0x00000F00 // Interrupt Pin
#define B_PCH_LPE_INTR_IL                  0x000000FF // Interrupt Line

#define R_PCH_LPE_PCS                      0x84  // PME Control Status
#define B_PCH_LPE_PCS_PMESTS               BIT15 // PME Status
#define B_PCH_LPE_PCS_PMEEN                BIT8  // PME Enable
#define B_PCH_LPE_PCS_NSS                  BIT3  // No Soft Reset
#define B_PCH_LPE_PCS_PS                   (BIT1 | BIT0) // Power State

///
/// LPE Private Space
///
#define PCH_LPE_PORT_ID                    0x58  // LPE Private Space PortID
#define PCH_LPE_PRIVATE_READ_OPCODE        0x06  // CUnit to LPE Private Space Read Opcode
#define PCH_LPE_PRIVATE_WRITE_OPCODE       0x07  // CUnit to LPE Private Space Write Opcode

#define R_PCH_LPE_PCICFGCTR1               0x500 // PCI Configuration Control 1
#define B_PCH_LPE_PCICFGCTR1_IPIN1         (BIT11 | BIT10 | BIT9 | BIT8) // Interrupt Pin
#define B_PCH_LPE_PCICFGCTR1_B1D1          BIT7  // BAR 1 Disable
#define B_PCH_LPE_PCICFGCTR1_PS            0x7C  // PME Support
#define B_PCH_LPE_PCICFGCTR1_ACPI_INT_EN1  BIT1  // ACPI Interrupt Enable
#define B_PCH_LPE_PCICFGCTR1_PCI_CFG_DIS1  BIT0  // PCI Configuration Space Disable

#endif
