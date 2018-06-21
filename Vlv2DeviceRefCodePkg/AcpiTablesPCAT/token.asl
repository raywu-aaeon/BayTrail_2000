/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Sandy Bridge        *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved    *;
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

Name(SP3O, 0x2e)        // Super I/O (Winbond 3xx) Index/Data configuration port for ASL.
#ifndef AMI_ACPI_SUPPORT //AMI_OVERRIDE
Name(IO4B, 0xa20)       // Super I/O (Winbond 3xx) GPIO base address
Name(IO4L, 0x20)        // Super I/O (Winbond 3xx) GPIO base address length
Name(SP1O, 0x4e)        // Super IO Index/Data configuration port for ASL.

Name(SMBS, 0xefa0)      // SMBus I/O Register Base Address
Name(SMBL, 0x20)        // SMBus I/O Register Range

Name(PMBS, 0x400)       // ASL alias for ACPI I/O base address.
Name(SMIP, 0xb2)        // I/O port to trigger SMI
Name(GPBS, 0x500)       // GPIO Register Block address
Name(APCB, 0xfec00000)  // Default I/O APIC(s) memory start address, 0x0FEC00000 - default, 0 - I/O APIC's disabled
Name(APCL, 0x1000)      // I/O APIC(s) memory decoded range, 0x1000 - default, 0 - I/O APIC's not decoded 
#endif //AMI_OVERRIDE
Name(PFDR, 0xfed03034)  // PMC Function Disable Register
Name(PMCB, 0xfed03000)  // PMC Base Address
Name(PCLK, 0xfed03060)  // PMC Clock Control Register
Name(PUNB, 0xfed05000)  // PUNIT Base Address
Name(IBAS, 0xfed08000)  // ILB Base Address
#ifndef AMI_ACPI_SUPPORT //AMI_OVERRIDE
Name(SRCB, 0xfed1c000)  // RCBA (Root Complex Base Address)
Name(SRCL, 0x1000)      // RCBA length
Name(HPTB, 0xfed00000)  // Same as HPET_BASE_ADDRESS for ASL use 
#endif //AMI_OVERRIDE
Name(MCHB, 0xfed14000)  // 
Name(MCHL, 0x4000)      // 
Name(EGPB, 0xfed19000)  // 
Name(EGPL, 0x1000)      // 
Name(DMIB, 0xfed18000)  // 
Name(DMIL, 0x1000)      // 
Name(IFPB, 0xfed14000)  // 
Name(IFPL, 0x1000)      // 
#ifndef AMI_ACPI_SUPPORT //AMI_OVERRIDE
Name(PEBS, 0xe0000000)  // 
Name(PELN, 0x10000000)  // 
#endif //AMI_OVERRIDE
Name(FMBL, 0x1) // Platform Flavor - Mobile flavor for ASL code.
Name(FDTP, 0x2) // Platform Flavor - Desktop flavor for ASL code.
Name(GCDD, 0x1) // GET_CURRENT_DISPLAY_DEVICE_SMI
Name(DSTA, 0xa) // DISPLAY_SWITCH_TOGGLE_ACPI_SMI
Name(DSLO, 0x2) // DISPLAY_SWITCH_LID_OPEN_ACPI_SMI
Name(DSLC, 0x3) // DISPLAY_SWITCH_LID_CLOSE_ACPI_SMI
Name(PITS, 0x10)        // POPUP_ICON_TOGGLE_SMI
Name(SBCS, 0x12)        // SET_BACKLIGHT_CONTROL_SMI
Name(SALS, 0x13)        // SET_ALI_LEVEL_SMI
Name(LSSS, 0x2a)        // LID_STATE_SWITCH_SMI
Name(PSSS, 0x2b)        // POWER_STATE_SWITCH_SMI
Name(SOOT, 0x35)        // SAVE_OSB_OS_TYPE_SMI
Name(ESCS, 0x48)        // ENABLE_SMI_C_STATE_COORDINATION_SMI
Name(SDGV, 0x1c)        // UHCI Controller HOST_ALERT's bit offset within the GPE block. GPIO[0:15] corresponding to GPE[16:31]
Name(ACPH, 0xde)        // North Bridge Scratchpad Data Register for patch ACPI.
#ifndef AMI_ACPI_SUPPORT //AMI_OVERRIDE
Name(ASSB, 0x0) // ACPI Sleep State Buffer for BIOS Usage.
Name(AOTB, 0x0) // ACPI OS Type Buffer for BIOS Usage.
Name(AAXB, 0x0) // ACPI Auxiliary Buffer for BIOS Usage.
Name(PEHP, 0x1) // _OSC: Pci Express Native Hot Plug Control
Name(SHPC, 0x0) // _OSC: Standard Hot Plug Controller (SHPC) Native Hot Plug control
Name(PEPM, 0x1) // _OSC: Pci Express Native Power Management Events control
Name(PEER, 0x1) // _OSC: Pci Express Advanced Error Reporting control
Name(PECS, 0x1) // _OSC: Pci Express Capability Structure control
Name(ITKE, 0x0) // This will be overridden by the ITK module.
#endif //AMI_OVERRIDE
Name(FTBL, 0x4) // Platform Flavor - Tablet flavor for ASL code.
