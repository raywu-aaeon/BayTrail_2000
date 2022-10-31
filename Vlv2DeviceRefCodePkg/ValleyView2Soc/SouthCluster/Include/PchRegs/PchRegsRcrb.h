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
  PchRegsRcrb.h

  @brief
  Register names for VLV Chipset Configuration Registers
  
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
#ifndef _PCH_REGS_RCRB_H_
#define _PCH_REGS_RCRB_H_

///
/// Chipset Configuration Registers (Memory space)
/// RCBA
///
#define R_PCH_RCRB_GCS                    0x00  // General Control and Status
#define B_PCH_RCRB_GCS_BBSIZE             (BIT30 | BIT29) // Boot Block Size
#define B_PCH_RCRB_GCS_BBS                (BIT11 | BIT10) // Boot BIOS Straps
#define V_PCH_RCRB_GCS_BBS_SPI            (3 << 10) // Boot BIOS strapped to SPI
#define V_PCH_RCRB_GCS_BBS_LPC            (0 << 10) // Boot BIOS strapped to LPC
#define B_PCH_RCRB_GCS_TS                 BIT1 // Top Swap
#define B_PCH_RCRB_GCS_BILD               BIT0 // BIOS Interface Lock-Down

#define R_PCH_RCRB_HCGE                   0x20  // HDA Clock Gating Control
#define B_PCH_RCRB_HCGE_AZDCG             BIT22 // Azalia Dynamic Clock Gating Enable
#define B_PCH_RCRB_HCGE_AZSCG             BIT21 // Azalia Static Clock Gating Enable

#define R_PCH_RCRB_HDA_V0CTL              0x24  // HD Audio VC0 Resource Control
#define B_PCH_RCRB_HDA_V0CTL_EN           BIT31 // Virtual Channel Enable
#define B_PCH_RCRB_HDA_V0CTL_ID           (BIT26 | BIT25 | BIT24) // Virtual Channel Identifier
#define B_PCH_RCRB_HDA_V0CTL_ETVM         0xFF00 // Extended TC/VC Map
#define B_PCH_RCRB_HDA_V0CTL_TVM          0xFE  // Transaction Class / Virtual Channel Map
#define B_PCH_RCRB_HDA_V0CTL_TVM0         BIT0  // Transaction Class / Virtual Channel Map [0]

#define R_PCH_RCRB_HDA_V1CTL              0x28  // HD Audio VC1 Resource Control
#define B_PCH_RCRB_HDA_V1CTL_EN           BIT31 // Virtual Channel Enable
#define B_PCH_RCRB_HDA_V1CTL_ID           (BIT26 | BIT25 | BIT24) // Virtual Channel Identifier
#define B_PCH_RCRB_HDA_V1CTL_ETVM         0xFF00 // Extended TC/VC Map
#define B_PCH_RCRB_HDA_V1CTL_TVM          0xFE  // Transaction Class / Virtual Channel Map
#define B_PCH_RCRB_HDA_V1CTL_TVM0         BIT0  // Transaction Class / Virtual Channel Map [0]

#define R_PCH_RCRB_HDA_VPCTL              0x2C  // HD Audio VCp Resource Control
#define B_PCH_RCRB_HDA_VPCTL_EN           BIT31 // Virtual Channel Enable
#define B_PCH_RCRB_HDA_VPCTL_ID           (BIT27 | BIT26 | BIT25 | BIT24) // Virtual Channel Identifier
#define B_PCH_RCRB_HDA_VPCTL_ETVM         0xFF00 // Extended TC/VC Map
#define B_PCH_RCRB_HDA_VPCTL_TVM          0xFE  // Transaction Class / Virtual Channel Map
#define B_PCH_RCRB_HDA_VPCTL_TVM0         BIT0  // Transaction Class / Virtual Channel Map [0]

#define R_PCH_RCRB_USB2_VPCTL             0x40  // USB2 VCp Resource Control
#define B_PCH_RCRB_USB2_VPCTL_EN          BIT31 // Virtual Channel Enable
#define B_PCH_RCRB_USB2_VPCTL_ID          (BIT27 | BIT26 | BIT25 | BIT24) // Virtual Channel Identifier
#define B_PCH_RCRB_USB2_VPCTL_ETVM        0xFF00 // Extended TC/VC Map
#define B_PCH_RCRB_USB2_VPCTL_TVM         0xFE  // Transaction Class / Virtual Channel Map
#define B_PCH_RCRB_USB2_VPCTL_TVM0        BIT0  // Transaction Class / Virtual Channel Map [0]

#endif
