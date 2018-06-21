;-------------------------------------------------------------------------------
;
; Copyright (c)  1999 - 2003 Intel Corporation. All rights reserved
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
;   MPCommon32.asm
;
; Abstract:
;
;   This is the assembly code for MP/HT (Multiple-processor / Hyper-threading)
;   support
;
;-------------------------------------------------------------------------------
.686p
.model  flat
.data
.stack
.code
.MMX
.XMM

 include  Htequ.inc
PAUSE32   MACRO
            DB      0F3h
            DB      090h
            ENDM

;-------------------------------------------------------------------------------
;  AsmAcquireMPLock (&Lock);
;-------------------------------------------------------------------------------
AsmAcquireMPLock   PROC  near C  PUBLIC

        pushad
        mov         ebp,esp

        mov         al, NotVacantFlag
        mov         ebx, dword ptr [ebp+24h]
TryGetLock:
        lock xchg   al, byte ptr [ebx]
        cmp         al, VacantFlag
        jz          LockObtained

        PAUSE32
        jmp         TryGetLock

LockObtained:
        popad
        ret
AsmAcquireMPLock   ENDP

;-------------------------------------------------------------------------------
;  AsmReleaseMPLock (&Lock);
;-------------------------------------------------------------------------------------
AsmReleaseMPLock   PROC  near C  PUBLIC

        pushad
        mov         ebp,esp

        mov         al, VacantFlag
        mov         ebx, dword ptr [ebp+24h]
        lock xchg   al, byte ptr [ebx]

        popad
        ret
AsmReleaseMPLock   ENDP

;-------------------------------------------------------------------------------
;  AsmGetGdtrIdtr (&Gdt, &Idt);
;-------------------------------------------------------------------------------------
AsmGetGdtrIdtr   PROC  near C  PUBLIC

        pushad
        mov         ebp,esp

        sgdt        fword ptr GdtDesc
        lea         esi, GdtDesc
        mov         edi, dword ptr [ebp+24h]
        mov         dword ptr [edi], esi

        sidt        fword ptr IdtDesc
        lea         esi, IdtDesc
        mov         edi, dword ptr [ebp+28h]
        mov         dword ptr [edi], esi

        popad
        ret
AsmGetGdtrIdtr   ENDP

GdtDesc::                             ; GDT descriptor
                    DW      03fh      ; GDT limit
                    DW      0h        ; GDT base and limit will be
                    DW      0h        ; filled using sgdt

IdtDesc::                             ; IDT descriptor
                    DW      0h        ; IDT limit
                    DW      0h        ; IDT base and limit will be
                    DW      0h        ; filled using sidt

END