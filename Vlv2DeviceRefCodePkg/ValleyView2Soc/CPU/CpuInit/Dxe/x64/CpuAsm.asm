  page    ,132
  title   CPU ARCHITECTURAL DXE PROTOCOL ASSEMBLY HOOKS
;------------------------------------------------------------------------------
;
; Copyright (c) 2005 -2006 Intel Corporation. All rights reserved
; This software and associated documentation (if any) is furnished
; under a license and may only be used or copied in accordance
; with the terms of the license. Except as permitted by such
; license, no part of this software or documentation may be
; reproduced, stored in a retrieval system, or transmitted in any
; form or by any means without the express written consent of
; Intel Corporation.
;
; Module Name:
;
;   CpuAsm.asm
; 
; Abstract:
; 
;   Assembly code of the implementation of X64 CPU architectural protocol
;
;------------------------------------------------------------------------------


text  SEGMENT
EXTERN  InitializeFloatingPointUnits:PROC ;AMI_OVERRIDE - EIP164769 SCT Execution Test-fail

EXTRN mOriginalInt13:QWORD
EXTRN mWroteMsr:QWORD
EXTRN mErrorCodeFlag:DWORD         ; Error code flags for exceptions

ExternalVectorTablePtr QWORD 0 ; point to the external interrupt vector table

InitializeExternalVectorTablePtr PROC  PUBLIC
  mov  ExternalVectorTablePtr, rcx
  ret
InitializeExternalVectorTablePtr ENDP
;
;
;
;------------------------------------------------------------------------------
;  Generic IDT Vector Handlers for the Host. They are all the same so they
;  will compress really well.
;
;  By knowing the return address for Vector 00 you can can calculate the
;  vector number by looking at the call CommonInterruptEntry return address.
;  (return address - (AsmIdtVector00 + 5))/8 == IDT index
;
;------------------------------------------------------------------------------

ALIGN   8

PUBLIC  AsmIdtVector00

AsmIdtVector00 LABEL BYTE
REPEAT  256
        call  CommonInterruptEntry
        dw ( $ - AsmIdtVector00 - 5 ) / 8 ; vector number
        nop
ENDM


;---------------------------------------;
; CommonInterruptEntry                  ;
;---------------------------------------;
; The follow algorithm is used for the common interrupt routine.

;
; +---------------------+ <-- 16-byte aligned ensured by processor
; +    Old SS           +
; +---------------------+
; +    Old RSP          +
; +---------------------+
; +    RFlags           +
; +---------------------+
; +    CS               +
; +---------------------+
; +    RIP              +
; +---------------------+
; +    Error Code       +
; +---------------------+
; + RCX / Vector Number +
; +---------------------+
; +    RBP              +
; +---------------------+ <-- RBP, 16-byte aligned
;

CommonInterruptEntry PROC  PUBLIC  
  cli
  ;
  ; All interrupt handlers are invoked through interrupt gates, so
  ; IF flag automatically cleared at the entry point
  ;
  ;
  ; Calculate vector number
  ;
  xchg    rcx, [rsp]               ; get the return address of call, actually, it is the address of vector number.
  movzx   ecx, word ptr [rcx]
  cmp     ecx, 32         ; Intel reserved vector for exceptions?
  jae     NoErrorCode
  bt      mErrorCodeFlag, ecx
  jc      @F

NoErrorCode:
  ;
  ; Push a dummy error code on the stack
  ; to maintain coherent stack map
  ;
  push    [rsp]
  mov     qword ptr [rsp + 8], 0
@@:       
  push    rbp
  mov     rbp, rsp

  ;
  ; Since here the stack pointer is 16-byte aligned, so
  ; EFI_FX_SAVE_STATE_X64 of EFI_SYSTEM_CONTEXT_x64
  ; is 16-byte aligned
  ;

;; UINT64  Rdi, Rsi, Rbp, Rsp, Rbx, Rdx, Rcx, Rax;
;; UINT64  R8, R9, R10, R11, R12, R13, R14, R15;
  push r15
  push r14
  push r13
  push r12
  push r11
  push r10
  push r9
  push r8
  push rax
  push qword ptr [rbp + 8]         ; RCX
  push rdx
  push rbx
  push qword ptr [rbp + 48]        ; RSP
  push qword ptr [rbp]             ; RBP
  push rsi
  push rdi

;; UINT64  Gs, Fs, Es, Ds, Cs, Ss;  insure high 16 bits of each is zero
  movzx   rax, word ptr [rbp + 56]
  push    rax                      ; for ss
  movzx   rax, word ptr [rbp + 32]
  push    rax                      ; for cs
  mov     rax, ds
  push    rax
  mov     rax, es
  push    rax
  mov     rax, fs
  push    rax
  mov     rax, gs
  push    rax

  mov     [rbp + 8], rcx           ; save vector number

;; UINT64  Rip;
  push    qword ptr [rbp + 24]

;; UINT64  Gdtr[2], Idtr[2];
  sub     rsp, 16
  sidt    fword ptr [rsp]
  sub     rsp, 16
  sgdt    fword ptr [rsp]

;; UINT64  Ldtr, Tr;
  xor     rax, rax
  str     ax
  push    rax
  sldt    ax
  push    rax

;; UINT64  RFlags;
  push    qword ptr [rbp + 40]

;; UINT64  Cr0, Cr1, Cr2, Cr3, Cr4, Cr8;
  mov     rax, cr8
  push    rax
  mov     rax, cr4
  or      rax, 208h
  mov     cr4, rax
  push    rax
  mov     rax, cr3
  push    rax
  mov     rax, cr2
  push    rax
  xor     rax, rax
  push    rax
  mov     rax, cr0
  push    rax

;; UINT64  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
  mov     rax, dr7
  push    rax
;; clear Dr7 while executing debugger itself
  xor     rax, rax
  mov     dr7, rax

  mov     rax, dr6
  push    rax
;; insure all status bits in dr6 are clear...
  xor     rax, rax
  mov     dr6, rax

  mov     rax, dr3
  push    rax
  mov     rax, dr2
  push    rax
  mov     rax, dr1
  push    rax
  mov     rax, dr0
  push    rax

;; FX_SAVE_STATE_X64 FxSaveState;

  sub rsp, 512
  mov rdi, rsp
  db 0fh, 0aeh, 00000111y              ;fxsave [rdi]

;; UINT32  ExceptionData;
  push    qword ptr [rbp + 16]

;; call into exception handler
  mov     rcx, [rbp + 8]
  mov     rax, ExternalVectorTablePtr  ; get the interrupt vectors base
  mov     rax, [rax + rcx * 8]
  or      rax, rax                     ; NULL?

  je    nonNullValue;

;; Prepare parameter and call
;  mov     rcx, [rbp + 8]
  mov     rdx, rsp
  ;
  ; Per X64 calling convention, allocate maximum parameter stack space
  ; and make sure RSP is 16-byte aligned
  ;
  sub     rsp, 4 * 8 + 8
  call    rax
  add     rsp, 4 * 8 + 8

nonNullValue:
  cli
;; UINT64  ExceptionData;
  add     rsp, 8

;; FX_SAVE_STATE_X64 FxSaveState;

  mov rsi, rsp
  db 0fh, 0aeh, 00001110y              ; fxrstor [rsi]
  add rsp, 512

;; UINT64  Dr0, Dr1, Dr2, Dr3, Dr6, Dr7;
  pop     rax
  mov     dr0, rax
  pop     rax
  mov     dr1, rax
  pop     rax
  mov     dr2, rax
  pop     rax
  mov     dr3, rax
;; skip restore of dr6.  We cleared dr6 during the context save.
  add     rsp, 8
  pop     rax
  mov     dr7, rax

;; UINT64  Cr0, Cr1, Cr2, Cr3, Cr4, Cr8;
  pop     rax
  mov     cr0, rax
  add     rsp, 8                       ; not for Cr1
  pop     rax
  mov     cr2, rax
  pop     rax
  mov     cr3, rax
  pop     rax
  mov     cr4, rax
  pop     rax
  mov     cr8, rax

;; UINT64  RFlags;
  pop     qword ptr [rbp + 40]

;; UINT64  Ldtr, Tr;
;; UINT64  Gdtr[2], Idtr[2];
;; Best not let anyone mess with these particular registers...
  add     rsp, 48

;; UINT64  Rip;
  pop     qword ptr [rbp + 24]

;; UINT64  Gs, Fs, Es, Ds, Cs, Ss;
  pop     rax
  ; mov     gs, rax ; not for gs
  pop     rax
  ; mov     fs, rax ; not for fs
  ; (X64 will not use fs and gs, so we do not restore it)
  pop     rax
  mov     es, rax
  pop     rax
  mov     ds, rax
  pop     qword ptr [rbp + 32]         ; for cs
  pop     qword ptr [rbp + 56]         ; for ss

;; UINT64  Rdi, Rsi, Rbp, Rsp, Rbx, Rdx, Rcx, Rax;
;; UINT64  R8, R9, R10, R11, R12, R13, R14, R15;
  pop     rdi
  pop     rsi
  add     rsp, 8                       ; not for rbp
  pop     qword ptr [rbp + 48]         ; for rsp
  pop     rbx
  pop     rdx
  pop     rcx
  pop     rax
  pop     r8
  pop     r9
  pop     r10
  pop     r11
  pop     r12
  pop     r13
  pop     r14
  pop     r15

  mov     rsp, rbp
  pop     rbp
  add     rsp, 16
  iretq

CommonInterruptEntry ENDP


LongMode PROC  PUBLIC

in_long_mode::
    ;
    ; Debug Stop
    ;
    jmp   in_long_mode

    ;
    ; We're in long mode, so marshall the arguments to call the
    ; passed in function pointers
    ; Recall
    ;         [ebp][10h] = HobStart
    ;         [ebp][18h] = Stack
    ;         [ebp][20h] = PpisNeededByDxeIplEntryPoint <--- Call this first (for each call, pass HOB pointer)
    ;         [ebp][28h] = DxeCoreEntryPoint            <--- Call this second
    ;
    mov rbx, [rbp+18h]        ; Setup the stack
    mov rsp, rbx              ; On a new stack now

    mov rcx, [rbp+10h]        ; Pass Hob Start in RCX
    mov rax, [rbp+20h]        ; Get the function pointer for 
                              ; PpisNeededByDxeIplEntryPoint into EAX
    call fword ptr [rax]      ; Make the call into PpisNeededByDxeIplEntryPoint

    mov ecx, [rbp+10h]        ; Pass Hob Start in RCX
    mov eax, [rbp+28h]        ; Get the function pointer for 
                              ; DxeCoreEntryPoint into EAX
    call fword ptr [rax]      ; Make the call into Dxe Core

    call CommonInterruptEntry

    mov rdi, CommonInterruptEntry 

    lgdt  fword ptr [rdi]

    lidt  fword ptr [rdi]

    call near ptr [rax]      ; Make the call into PpisNeededByDxeIplEntryPoint

    call rax

    ;
    ; Should never get here.
    ;
no_long_mode:
    jmp   no_long_mode
    ;
    ; WE SHOULD NEVER GET HERE!!!!!!!!!!!!!
    ;
LongMode endp

;------------------------------------------------------------------------------
;  VOID
;  C1eExceptionHandler (
;    VOID
;    )
;------------------------------------------------------------------------------
C1eExceptionHandler    proc     public

  push    rbp
  mov     rbp, rsp

  pushfq
  cli

  ; Verify if GPE was caused by C1e write.
  ; If not, pass control to real exception handler.
  cmp    mWroteMsr, 0
  je     notourexception

  ; Fix the return address on stack to skip offending
  ; code which caused the exception.
  mov    rax, qword ptr [rbp + 8] 
  add    rax, 2
  mov    qword ptr [rbp + 8], rax
  jmp    exit

notourexception:
  popfq
  pop    rbp

  jmp    qword ptr mOriginalInt13

exit:

  popfq
  pop    rbp
  
  add    rsp, 8
  
  iretq
  
C1eExceptionHandler ENDP

EnableMCE  proc  public

  mov     rax, cr4
  or      rax, 40h
  mov     cr4, rax

  ret

EnableMCE  endp

MpMtrrSynchUpEntry  PROC        PUBLIC
    ;
    ; Enter no fill cache mode, CD=1(Bit30), NW=0 (Bit29)
    ;
    mov rax, cr0
    and rax, 0DFFFFFFFh
    or  rax, 040000000h
    mov cr0, rax
    ;
    ; Flush cache
    ;
    wbinvd
    ;
    ; Clear PGE flag Bit 7
    ;
    mov rax, cr4
    mov rdx, rax
    and rax, 0FFFFFF7Fh
    mov cr4, rax
    ;
    ; Flush all TLBs
    ;
    mov rax, cr3
    mov cr3, rax

    mov rax, rdx

    ret

MpMtrrSynchUpEntry  ENDP

MpMtrrSynchUpExit  PROC        PUBLIC
    ;
    ; Flush all TLBs the second time
    ;
    mov rax, cr3
    mov cr3, rax
    ;
    ; Enable Normal Mode caching CD=NW=0, CD(Bit30), NW(Bit29)
    ;
    mov rax, cr0
    and rax, 09FFFFFFFh
    mov cr0, rax
    ;
    ; Set PGE Flag in CR4 if set
    ;
    mov cr4, rcx
    ret

MpMtrrSynchUpExit  ENDP

CpuPause  PROC        PUBLIC

    pause
    ret

CpuPause  ENDP

CpuInitFloatPointUnit  PROC        PUBLIC
    call InitializeFloatingPointUnits ;AMI_OVERRIDE - EIP164769 SCT Execution Test-fail
    ; bugbug
    ; finit
    ret

CpuInitFloatPointUnit  ENDP

CpuDisableInterrupt PROC    PUBLIC

    cli
    ret

CpuDisableInterrupt  ENDP

CpuEnableInterrupt PROC    PUBLIC

    sti
    ret

CpuEnableInterrupt  ENDP

GetCoreNumber PROC    PUBLIC

    push rbx

    mov  eax, 4
    mov  ecx, 0
    cpuid

    shr  eax, 26
    and  eax, 3fh
    inc  al

    pop  rbx

    ret

GetCoreNumber  ENDP

text  ENDS
END
