;-------------------------------------------------------------------------------
;
; Copyright (c)  1999 - 2007 Intel Corporation. All rights reserved
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
;   HtFuncs32.asm
; 
; Abstract:
; 
;   This is the assembly code for HT (Hyper-threading) support
;
;-------------------------------------------------------------------------------

include  Htequ.inc 

;-------------------------------------------------------------------------------------
;RendezvousFunnelProc  procedure follows. All APs execute their procedure. This
;procedure serializes all the AP processors through an Init sequence. It must be
;noted that APs arrive here very raw...ie: real mode, no stack.
;ALSO THIS PROCEDURE IS EXECUTED BY APs ONLY ON 16 BIT MODE. HENCE THIS PROC
;IS IN MACHINE CODE.
;-------------------------------------------------------------------------------------
;RendezvousFunnelProc (&WakeUpBuffer,MemAddress);

.686p
.model  flat        
.code        


RendezvousFunnelProc   PROC  PUBLIC
RendezvousFunnelProcStart::

;Step-1: Grab a lock. At this point CS = 0x(vv00) and ip= 0x0.

        db 8ch,0c8h                                ; mov        ax,cs
        db 8eh,0d8h                                ; mov        ds,ax
        db 8eh,0c0h                                ; mov        es,ax
        db 8eh,0d0h                                ; mov        ss,ax 
        db 33h,0c0h                                ; xor        ax,ax
        db 8eh,0e0h                                ; mov        fs,ax
        db 8eh,0e8h                                ; mov        gs,ax

        db 0BEh, 1Ch, 0Eh                          ; mov        si, BufferStartLocation
        db 66h,  8Bh, 1Ch                          ; mov        ebx,dword ptr [si]

        db 0BFh, 20h, 0Eh                          ; mov        di, PmodeOffsetLocation
        db 66h,  8Bh, 05h                          ; mov        eax,dword ptr [di]
        db 8Bh,  0F8h                              ; mov        di, ax
        db 83h,  0EFh,06h                          ; sub        di, 06h
        db 66h,  03h, 0C3h                         ; add        eax,ebx
        db 66h,  89h, 05h                          ; mov        dword ptr [di],eax

        db 0BEh, 10h, 0Eh                          ; mov        si, GdtrProfile
        db 66h                                     ; db         66h
        db 2Eh,  0Fh, 01h, 14h                     ; lgdt       fword ptr cs:[si]

        db 33h,  0C0h                              ; xor        ax,  ax
        db 8Eh,  0D8h                              ; mov        ds,  ax
        
        db 0Fh,  20h, 0C0h                         ; mov        eax, cr0                        ;Get control register 0
        db 66h,  83h, 0C8h, 03h                    ; or         eax, 000000003h                 ;Set PE bit (bit #0) & MP
        db 0Fh,  22h, 0C0h                         ; mov        cr0, eax

        db 66h,  67h, 0EAh                         ; far jump
        dd 0h                                      ; 32-bit offset
        dw 20h                                     ; 16-bit selector

NemInit::                                          ; protected mode entry point

        db 66h,  0B8h, 18h,  00h                   ; mov        ax, 18h
        db 66h,  8Eh,  0D8h                        ; mov        ds, ax
        db 66h,  8Eh,  0C0h                        ; mov        es, ax
        db 66h,  8Eh,  0E0h                        ; mov        fs, ax
        db 66h,  8Eh,  0E8h                        ; mov        gs, ax
        db 66h,  8Eh,  0D0h                        ; mov        ss, ax            ; Flat mode setup.

        mov  esi, ebx
        
        mov  edi, esi
        add  edi, StartStateLocation
        mov  eax, 1
        mov  dword ptr [edi], eax

        mov  edi, esi
        add  edi, LockLocation
        mov  eax, NotVacantFlag
TestLock::
        xchg dword ptr [edi], eax
        cmp  eax, NotVacantFlag
        jz   TestLock

ProgramStack::

        mov  edi, esi
        add  edi, StackSizeLocation
        mov  eax, dword ptr [edi]
        mov  edi, esi
        add  edi, StackStartAddressLocation
        add  eax, dword ptr [edi]
        mov  esp, eax
        mov  dword ptr [edi], eax

Releaselock::

        mov  eax, VacantFlag
        mov  edi, esi
        add  edi, LockLocation
        xchg dword ptr [edi], eax

CProcedureInvoke::

        mov  edi, esi
        add  edi, MtrrValuesAddressLocation
        mov  eax, dword ptr [edi]
        push eax
        
        mov  eax, esi
        add  eax, LockLocation
        push eax
        
        mov  edi, esi
        add  edi, CProcedureLocation
        mov  eax, dword ptr [edi]

        call eax
        add  esp, 8
        
        cli
        hlt
        jmp  $-2

RendezvousFunnelProc   ENDP
RendezvousFunnelProcEnd::


;-------------------------------------------------------------------------------------
;  AsmGetAddressMap (&AddressMap);
;-------------------------------------------------------------------------------------
AsmGetAddressMap   PROC  near C  PUBLIC

        mov eax, RendezvousFunnelProcStart
        ret
        
AsmGetAddressMap   ENDP

AsmGetPmodeOffset PROC  near C PUBLIC

        mov eax, NemInit - RendezvousFunnelProcStart
        ret
        
AsmGetPmodeOffset ENDP 

AsmCliHltLoop PROC near C PUBLIC
        cli
        hlt
        jmp $-2
AsmCliHltLoop ENDP

END
