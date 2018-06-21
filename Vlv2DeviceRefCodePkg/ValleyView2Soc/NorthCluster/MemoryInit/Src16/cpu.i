;**************************************************************************
;*                                                                        *
;*      Intel Restricted Secret                                           *
;*                                                                        *
;*      Valleyview Memory Reference Code                                   *
;*                                                                        *
;*      Copyright (c) 2012 Intel Corp.                                    *
;*                                                                        *
;*      This program has been developed by Intel Corporation.             *
;*      Licensee has Intel's permission to incorporate this source code   *
;*      into their product, royalty free.  This source code may NOT be    *
;*      redistributed to anyone without Intel's written permission.       *
;*                                                                        *
;*      Intel specifically disclaims all warranties, express or           *
;*      implied, and all liability, including consequential and other     *
;*      indirect damages, for the use of this code, including liability   *
;*      for infringement of any proprietary rights, and including the     *
;*      warranties of merchantability and fitness for a particular        *
;*      purpose.  Intel does not assume any responsibility for any        *
;*      errors which may appear in this code nor any responsibility to    *
;*      update it.                                                        *
;*                                                                        *
;**************************************************************************

MTRR_PHYS_BASE_0        EQU 0200h
MTRR_PHYS_MASK_0        EQU 0201h
MTRR_PHYS_BASE_1        EQU 0202h
MTRR_PHYS_MASK_1        EQU 0203h
MTRR_PHYS_BASE_2        EQU 0204h
MTRR_PHYS_MASK_2        EQU 0205h
MTRR_PHYS_BASE_3        EQU 0206h
MTRR_PHYS_MASK_3        EQU 0207h
MTRR_PHYS_BASE_4        EQU 0208h
MTRR_PHYS_MASK_4        EQU 0209h
MTRR_PHYS_BASE_5        EQU 020Ah
MTRR_PHYS_MASK_5        EQU 020Bh
MTRR_PHYS_BASE_6        EQU 020Ch
MTRR_PHYS_MASK_6        EQU 020Dh
MTRR_PHYS_BASE_7        EQU 020Eh
MTRR_PHYS_MASK_7        EQU 020Fh
MTRR_FIX_64K_00000      EQU 0250h
MTRR_FIX_16K_80000      EQU 0258h
MTRR_FIX_16K_A0000      EQU 0259h
MTRR_FIX_4K_C0000       EQU 0268h
MTRR_FIX_4K_C8000       EQU 0269h
MTRR_FIX_4K_D0000       EQU 026Ah
MTRR_FIX_4K_D8000       EQU 026Bh
MTRR_FIX_4K_E0000       EQU 026Ch
MTRR_FIX_4K_E8000       EQU 026Dh
MTRR_FIX_4K_F0000       EQU 026Eh
MTRR_FIX_4K_F8000       EQU 026Fh
MTRR_DEF_TYPE           EQU 02FFh

MTRR_MEMORY_TYPE_UC     EQU 00h
MTRR_MEMORY_TYPE_WC     EQU 01h
MTRR_MEMORY_TYPE_WT     EQU 04h
MTRR_MEMORY_TYPE_WP     EQU 05h
MTRR_MEMORY_TYPE_WB     EQU 06h

MTRR_DEF_TYPE_E         EQU 0800h
MTRR_DEF_TYPE_FE        EQU 0400h
MTRR_PHYSMASK_VALID     EQU 0800h

MSR_XAPIC_BASE          EQU 01Bh
IA32_MISC_ENABLE        EQU 1A0h
FAST_STRING_ENABLE_BIT  EQU 01h

IA32_BIOS_CACHE_AS_RAM  EQU 80h
NO_EVICTION_ENABLE_BIT  EQU 01h

CR0_CACHE_DISABLE       EQU 040000000h
CR0_NO_WRITE            EQU 020000000h

TILE_SIZE			EQU	000020000h		; 128K

IF	(SDV_BIOS NE 0)
CODE_REGION_BASE_ADDRESS	EQU	0FFFE0000h
ELSE
CODE_REGION_BASE_ADDRESS	EQU	0000E0000h
ENDIF
CODE_REGION_SIZE		EQU	TILE_SIZE
CODE_REGION_SIZE_MASK		EQU	(NOT (CODE_REGION_SIZE - 1))

IF	(SDV_BIOS NE 0)
DATA_STACK_BASE_ADDRESS		EQU	0FFEC0000h
ELSE
DATA_STACK_BASE_ADDRESS		EQU	MRC_ADDRESS + 0FFF00000h + 64*1024 + 32*1024
ENDIF
DATA_STACK_SIZE			EQU	8*1024
DATA_STACK_SIZE_MASK		EQU	(NOT (DATA_STACK_SIZE - 1))

BBL_CR_CTL3			EQU	011Eh
