  page    ,132
  title   MP ASSEMBLY HOOKS
;------------------------------------------------------------------------------
;
; Copyright (c) 1999 - 2006 Intel Corporation. All rights reserved
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
;   MpProc.Asm
;
; Abstract:
;
;   This is the code that supports MP
;
;------------------------------------------------------------------------------

.686p
.model  flat
.data
.stack
.code
.MMX
.XMM
_MpMtrrSynchUpEntry  PROC    NEAR    PUBLIC
    ;
    ; Enter no fill cache mode, CD=1(Bit30), NW=0 (Bit29)
    ;
    mov eax, cr0
    and eax, 0DFFFFFFFh
    or  eax, 040000000h
    mov cr0, eax
    ;
    ; Flush cache
    ;
    wbinvd
    ;
    ; Clear PGE flag Bit 7
    ;
    mov eax, cr4
    mov edx, eax
    and eax, 0FFFFFF7Fh
    mov cr4, eax
    ;
    ; Flush all TLBs
    ;
    mov eax, cr3
    mov cr3, eax

    mov eax, edx

    ret

_MpMtrrSynchUpEntry  ENDP

_MpMtrrSynchUpExit  PROC    NEAR    PUBLIC

    push    ebp             ; C prolog
    mov     ebp, esp
    ;
    ; Flush all TLBs the second time
    ;
    mov eax, cr3
    mov cr3, eax
    ;
    ; Enable Normal Mode caching CD=NW=0, CD(Bit30), NW(Bit29)
    ;
    mov eax, cr0
    and eax, 09FFFFFFFh
    mov cr0, eax
    ;
    ; Set PGE Flag in CR4 if set
    ;
    mov eax, dword ptr [ebp + 8]
    mov cr4, eax

    pop ebp

    ret

_MpMtrrSynchUpExit  ENDP

  END

