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
IFNDEF EFI_MEMORY_INIT

        .586P

.XLIST
        include cpu.i
.LIST

_TEXT16         SEGMENT PARA USE16 PUBLIC 'CODE16'
        extern   _returnFromCarInit:NEAR
        extern   _returnFromCarExit:NEAR
_TEXT16         ENDS

_TEXT16         SEGMENT
                ALIGN 16

;------------------------------------------------------------------------------
; MtrrInitTable:
;       Table of values out of which MTRRs are initialized for CAR activation.
;------------------------------------------------------------------------------
MtrrInitTable   LABEL BYTE
                DW      MTRR_DEF_TYPE
                DW      MTRR_FIX_64K_00000
                DW      MTRR_FIX_16K_80000
                DW      MTRR_FIX_16K_A0000
                DW      MTRR_FIX_4K_C0000
                DW      MTRR_FIX_4K_C8000
                DW      MTRR_FIX_4K_D0000
                DW      MTRR_FIX_4K_D8000
                DW      MTRR_FIX_4K_E0000
                DW      MTRR_FIX_4K_E8000
                DW      MTRR_FIX_4K_F0000
                DW      MTRR_FIX_4K_F8000
                DW      MTRR_PHYS_BASE_0
                DW      MTRR_PHYS_MASK_0
                DW      MTRR_PHYS_BASE_1
                DW      MTRR_PHYS_MASK_1
                DW      MTRR_PHYS_BASE_2
                DW      MTRR_PHYS_MASK_2
                DW      MTRR_PHYS_BASE_3
                DW      MTRR_PHYS_MASK_3
                DW      MTRR_PHYS_BASE_4
                DW      MTRR_PHYS_MASK_4
                DW      MTRR_PHYS_BASE_5
                DW      MTRR_PHYS_MASK_5
                DW      MTRR_PHYS_BASE_6
                DW      MTRR_PHYS_MASK_6
                DW      MTRR_PHYS_BASE_7
                DW      MTRR_PHYS_MASK_7
MtrrInitTableLen EQU (($ - MtrrInitTable) / 2)

;------------------------------------------------------------------------------
; _CarInit:
;       Initializes Caches-as-RAM (CAR) of the processor using No-Fill mode.
;
;	If CPU with Family 6, Model F is detected, enables cacheability of CODE_REGION
;
; Input:
;       Processor Operating Mode:  32-bit protected mode with flat selectors
;       DS, ES, SS, FS, GS
;          = Selector that points to a descriptor with
;               Base  = 000000000h
;               Limit = 0FFFFFFFFh
;               Present, DPL = 0, Expand-up, Read-Write, Granularity = 4K
;
; Output:
;       ESP = Highest dword processor address in CAR.
;
; Destroys:
;       EAX, EBX, ECX, EDX, ESI, EDI, EBP.
;       Processor MTRRs as needed.
;
; Note:
;------------------------------------------------------------------------------
_CarInit        PROC    NEAR    PUBLIC
                ;
                ; Initialize all MTRRs to a known state
                ;
                mov     esi, OFFSET cs:MtrrInitTable    ; Get a pointer to the table
                mov     edi, MtrrInitTableLen           ; Get the total length
                xor     eax, eax                        ; Clear the low dword to write
                xor     edx, edx                        ; Clear the high dword to write

initMtrrLoop:
                movzx   ecx, WORD PTR cs:[esi]          ; Put the MTRR offset into ECX
                wrmsr
                add     esi, 2                          ; Increment to the next offset
                dec     edi                             ; Reduce the count by 1
                jnz     initMtrrLoop                    ; Until we've reached the table length

                ;
                ; Disable fast strings feature
                ;
;Not supported for vlv
;                mov     ecx, IA32_MISC_ENABLE           ; Fast Strings feature control in Misc Enable
;                rdmsr
;                and     eax, NOT FAST_STRING_ENABLE_BIT
;                wrmsr

                ;
                ; Setup stack region to be Write-Back cacheable
                ;
							; Load the writeback cache value
                mov     eax, DATA_STACK_BASE_ADDRESS OR MTRR_MEMORY_TYPE_WB
                xor     edx, edx                        ; clear upper dword
                mov     ecx, MTRR_PHYS_BASE_0           ; Load the MTRR index
                wrmsr                                   ; the value in MTRR_PHYS_BASE_0

							; turn on the Valid flag
                mov     eax, DATA_STACK_SIZE_MASK OR MTRR_PHYSMASK_VALID
                mov     edx, 0Fh                        ; For 36bit addressing
                mov     ecx, MTRR_PHYS_MASK_0           ; Load the MTRR index
                wrmsr                                   ; the value in MTRR_PHYS_BASE_0

		mov	ecx, BBL_CR_CTL3
		rdmsr
		bt	eax, 0 ; Check if L2 is hardware enabled
		jnc	NoL2Cache
		bt	eax, 23 ; Check if L2 present
		jc	NoL2Cache
		or	eax, 100h ; Enable L2
		wrmsr

                ;
                ; Setup code region to be Write-Protected cacheable
                ;
							; Load the write protect cache value
                mov     eax, CODE_REGION_BASE_ADDRESS OR MTRR_MEMORY_TYPE_WP
                xor     edx, edx                        ; clear upper dword
                mov     ecx, MTRR_PHYS_BASE_1           ; Load the MTRR index
                wrmsr                                   ; the value in MTRR_PHYS_BASE_0

							; turn on the Valid flag
                mov     eax, CODE_REGION_SIZE_MASK OR MTRR_PHYSMASK_VALID
                mov     edx, 0Fh                        ; For 36bit addressing
                mov     ecx, MTRR_PHYS_MASK_1           ; Load the MTRR index
                wrmsr                                   ; the value in MTRR_PHYS_BASE_0


		; Enable the L2 cache by setting bit 8 of BBL_CR_CTL3 MSR
		mov	ecx, BBL_CR_CTL3
		rdmsr
		bt	eax, 0				; Check if L2 is HW enabled
		jnc	NoL2Cache
		bt	eax, 23				; Check if L2 is present
		jc	NoL2Cache
		or	eax, 100h			; Enable L2
		wrmsr
NoL2Cache:

		; Enable MTRR's by setting DEF_TYP.DEF_TYPE_E bit
                mov     ecx, MTRR_DEF_TYPE              ; Load the MTRR default type index
                rdmsr
                or      eax, MTRR_DEF_TYPE_E            ; Enable MTRR's
                wrmsr

                ; Enable the cache and explicitly invalidate its contents
                ;
                mov     eax, cr0
                and     eax, NOT (CR0_CACHE_DISABLE + CR0_NO_WRITE)
		invd
                mov     cr0, eax

                ;
		; Establish valid tags in the data/stack cache
		;	and explicitly write to each location to set "Modified" flag
                ;
                cld
                mov     eax, 05AA55AA5h
                mov     edi, DATA_STACK_BASE_ADDRESS
                mov     ecx, DATA_STACK_SIZE / 4
                rep     stos DWORD PTR es:[edi]

		mov	esp, DATA_STACK_BASE_ADDRESS + DATA_STACK_SIZE - 4

		jmp	_returnFromCarInit

_CarInit        ENDP


;------------------------------------------------------------------------------
; _CarExit:
;       Exits Caches-as-RAM (CAR) of the processor.
;
; Input:
;       Processor Operating Mode: 16-bit real mode BUT with flat, 32-bit addressability
;       from 0 thru 4GB.
;       CS = Real-mode segment address of the segment this code is executing out of.
;       DS = Selector that points to a descriptor with
;               Base  = 000000000h
;               Limit = 0FFFFFFFFh
;               Present, DPL = 0, Expand-up, Read-Write, Granularity = 4K
;       ES, SS, FS, GS = Same settings as DS.
;
; Output:
;       Processor cache restored to default state
;
; Destroys:
;       EAX, EBX, ECX, EDX, ESI, EDI, EBP.
;
; Note:
;------------------------------------------------------------------------------
_CarExit        PROC    NEAR    PUBLIC


                ;
                ; Restore MTRRs to their default state
                ;
                mov     esi, OFFSET cs:MtrrInitTable    ; Get a pointer to the table
                mov     edi, MtrrInitTableLen           ; Get the total length
                xor     eax, eax                        ; Clear the low dword to write
                xor     edx, edx                        ; Clear the high dword to write

initMtrrLoop:
                movzx   ecx, WORD PTR cs:[esi]          ; Put the MTRR offset into ECX
                wrmsr
                add     esi, 2                          ; Increment to the next offset
                dec     edi                             ; Reduce the count by 1
                jnz     initMtrrLoop                    ; Until we've reached the table length

		;
		; Disable the MTRRs
		;
                mov     ecx, MTRR_DEF_TYPE              ; Load the MTRR default type index
                rdmsr
                and     eax, NOT MTRR_DEF_TYPE_E            ; Disable MTRR's
                wrmsr

                ;
                ; Invalidate the cache
                ;
                invd

                ; Disable the cache
                ;
                mov     eax, cr0
                or      eax, (CR0_CACHE_DISABLE + CR0_NO_WRITE)
                mov     cr0, eax

                ;
                ; Enable fast strings feature
                ;
                mov     ecx, IA32_MISC_ENABLE           ; Fast String Ops control in Misc Enable
                rdmsr
                or      eax, FAST_STRING_ENABLE_BIT
                wrmsr

                jmp     _returnFromCarExit

_CarExit        ENDP

_TEXT16 ENDS

ENDIF
END
