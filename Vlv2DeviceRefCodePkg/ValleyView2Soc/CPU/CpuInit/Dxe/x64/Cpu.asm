      TITLE   Cpu.asm: Assembly code for the IA-32 resources

;-------------------------------------------------------------------------------
;
; Copyright (c) 2005 -2007 Intel Corporation. All rights reserved
; This software and associated documentation (if any) is furnished
; under a license and may only be used or copied in accordance
; with the terms of the license. Except as permitted by such
; license, no part of this software or documentation may be
; reproduced, stored in a retrieval system, or transmitted in any
; form or by any means without the express written consent of
; Intel Corporation.
;
;
; Module Name:
;
;   Cpu.asm
; 
; Abstract:
; 
;
;-------------------------------------------------------------------------------

text    SEGMENT




;------------------------------------------------------------------------------
;  UINTN
;  CpuReadCr0 (
;    VOID
;    )
;------------------------------------------------------------------------------
CpuReadCr0 PROC      PUBLIC
    mov   rax, cr0
    ret
CpuReadCr0  ENDP

;------------------------------------------------------------------------------
;  VOID
;  CpuWriteCr0 (
;    UINTN  Value
;    )
;------------------------------------------------------------------------------
CpuWriteCr0 PROC        PUBLIC
    mov   cr0, rcx
    ret
CpuWriteCr0  ENDP

;------------------------------------------------------------------------------
;  UINTN
;  CpuReadCr3 (
;    VOID
;    )
;------------------------------------------------------------------------------
CpuReadCr3 PROC        PUBLIC
    mov   rax, cr3
    ret
CpuReadCr3  ENDP

;------------------------------------------------------------------------------
;  UINTN
;  CpuWriteCr3 (
;    VOID
;    )
;------------------------------------------------------------------------------
CpuWriteCr3 PROC        PUBLIC
    mov   cr3, rcx
    ret
CpuWriteCr3  ENDP


;------------------------------------------------------------------------------
;  UINTN
;  CpuFlushTlb (
;    VOID
;    )
;------------------------------------------------------------------------------
CpuFlushTlb PROC        PUBLIC
    mov   rax, cr3
    mov   cr3, rax
    ret
CpuFlushTlb  ENDP


;------------------------------------------------------------------------------
; UINTN
; CpuSetPower2 (
;   IN  UINTN   Input
;   );
;------------------------------------------------------------------------------
CpuSetPower2 PROC        PUBLIC
    bsr   rdx, rcx
    bts   rax, rdx
    ret
CpuSetPower2  ENDP

;------------------------------------------------------------------------------
; UINT64
; CpuReadTsc (
;   VOID
;   );
;------------------------------------------------------------------------------
CpuReadTsc PROC        PUBLIC
    rdtsc
    ret
CpuReadTsc  ENDP

;------------------------------------------------------------------------------
; UINT64
; CpuSwitchStacks (
;   IN  UINTN EntryPoint,   // rcx
;   IN  UINTN Parameter1,   // rdx
;   IN  UINTN NewStack,     // r8
;   IN  UINTN NewBsp        // r9 - Only used on IPF
;   );
;------------------------------------------------------------------------------
CpuSwitchStacks PROC        PUBLIC
    mov   rsp,  r8       ; rsp = NewStack
    push  rdx            ; Parameter1
    call  rcx            ; rcx = EntryPoint
 ;
 ; no ret as we have a new stack and we jumped to the new location
 ;     
CpuSwitchStacks  ENDP

;------------------------------------------------------------------------------
; UINT64
; CpuSwitchStacks2Args (
;   IN  UINTN EntryPoint,   // rcx
;   IN  UINTN Parameter1,   // rdx
;   IN  UINTN Parameter2,   // r8  
;   IN  UINTN NewStack,     // r9
;   IN  UINTN Bsp           // Only used on IPF
;   );
;
; BSP not used on IA-32
;
;------------------------------------------------------------------------------
CpuSwitchStacks2Args PROC        PUBLIC     
    mov   rsp,  r8       ; rsp = NewStack
    push  r8             ; Parameter2
    push  rdx            ; Parameter1
    call  rcx            ; rcx = EntryPoint
 ;
 ; no ret as we have a new stack and we jumped to the new location
 ;     
 CpuSwitchStacks2Args  ENDP


;------------------------------------------------------------------------------
; UINT16
; CpuCodeSegment (
;   VOID
;   );
;------------------------------------------------------------------------------
CpuCodeSegment PROC        PUBLIC  
    xor   eax, eax
    mov   eax, cs
    ret
CpuCodeSegment  ENDP


;------------------------------------------------------------------------------
; VOID
; CpuBreak (
;   VOID
;   );
;------------------------------------------------------------------------------
CpuBreak PROC   PUBLIC
    int 3
    ret
CpuBreak  ENDP


;------------------------------------------------------------------------------
; VOID
; CpuLoadGlobalDescriptorTable (
;   VOID  *Table16ByteAligned
;   );
;------------------------------------------------------------------------------
CpuLoadGlobalDescriptorTable PROC   PUBLIC
    lgdt  FWORD PTR [rcx]
    ret
CpuLoadGlobalDescriptorTable  ENDP

CpuInitSelectors PROC   PUBLIC
        int 68h
        ret
CpuInitSelectors    ENDP 
;------------------------------------------------------------------------------
; VOID
; CpuLoadInterruptDescriptorTable (
;   VOID  *Table16ByteAligned
;   );
;------------------------------------------------------------------------------
CpuLoadInterruptDescriptorTable PROC   PUBLIC
    lidt  FWORD PTR [rcx]
    ret
CpuLoadInterruptDescriptorTable  ENDP


text  ENDS
END


