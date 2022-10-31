CALCWr      MACRO   r:REQ, w:REQ
    xor     r, [w - 14*4]
    xor     r, [w - 8*4]
    xor     r, [w - 3*4]
    rol     r, 1
    mov     [w], r
            ENDM

CALCW       MACRO   r:REQ, w:REQ
    mov     r, [w - 16*4]
    CALCWr  r, w
            ENDM

S_ABC       MACRO   A, B, C, K, idxr:REQ
    ror     B, 2                        ; B >>>= 2
    mov     C, B                        ; C <- B >>> 2
    mov     B, A                        ; B <- A
    rol     A, 5                        ; A <<<= 5
    add     A, eax                      ; A += F(B, C, D) + E
    mov     eax, [idxr]                 ; eax <- Wi
    lea     idxr, [idxr + 4]
    add     A, K                        ; A += K
    add     A, eax                      ; A += Wi
            ENDM
;
; Round1: F(B, C, D) = BC + ~BD
;
SHA1_R1     MACRO   A, B, C, D, E, K, idxr:REQ
    mov     eax, B                      ; eax <- B
    mov     r11d, C
    not     eax                         ; eax <- ~B
    and     C, B                        ; C <- BC
    and     eax, D                      ; eax <- ~BD
    or      eax, C                      ; eax <- F(B, C, D)
    add     eax, E                      ; eax <- F(B, C, D) + E
    mov     E, D                        ; E <- D
    mov     D, r11d                     ; D <- C
    S_ABC   A, B, C, K, idxr
    CALCWr  eax, idxr + 15*4
            ENDM

;
; Round2: F(B, C, D) = B^C^D
;
SHA1_R2     MACRO   A, B, C, D, E, K, idxr:REQ
    SHA1_R4 A, B, C, D, E, K, idxr
    CALCWr  eax, idxr + 15*4
            ENDM

;
; Round3: F(B, C, D) = BC + BD + CD = B(C + D) + CD
;
SHA1_R3     MACRO   A, B, C, D, E, K, idxr:REQ
    mov     eax, C                      ; eax <- C
    mov     r11d, C
    or      eax, D                      ; eax <- C + D
    and     C, D                        ; C <- CD
    and     eax, B                      ; eax <- B(C + D)
    or      eax, C                      ; eax <- F(B, C, D)
    add     eax, E                      ; eax <- F(B, C, D) + E
    mov     E, D                        ; E <- D
    mov     D, r11d                     ; D <- C
    S_ABC   A, B, C, K, idxr
    CALCWr  eax, idxr + 15*4
            ENDM

;
; Round4: F(B, C, D) = B^C^D
;
SHA1_R4     MACRO   A, B, C, D, E, K, idxr:REQ
    mov     eax, B
    xor     eax, C
    xor     eax, D                      ; eax <- F(B, C, D)
    add     eax, E                      ; eax += E
    mov     E, D                        ; E <- D
    mov     D, C                        ; D <- C
    S_ABC   A, B, C, K, idxr
            ENDM

SHA1RND     MACRO   SHA1_R, K:REQ
    or      rcx, 20
@@:
    SHA1_R  ebx, edx, esi, edi, r10d, K, rbp
    loop    @B
            ENDM

SHA1_transform  PROC    USES    rbx rsi rdi
    push    rbp                         ; save rbp
    add     rsp, -80*4
    mov     rbp, rsp
    push    rcx                         ; stack is 10h aligned
    mov     rbx, rcx                    ; rbx <- addr of S
    push    10h
    pop     rcx                         ; rcx <- 10h
@@:
    mov     eax, [rdx + rcx*4 - 4]      ; rdx = addr of M
    bswap   eax
    mov     [rbp + rcx*4 - 4], eax
    loop    @B
    mov     edx, [rbx + 4]              ; edx <- S[1]
    mov     esi, [rbx + 8]              ; esi <- S[2]
    mov     edi, [rbx + 12]             ; edi <- S[3]
    mov     r10d, [rbx + 16]            ; r10d <- S[4]
    mov     ebx, [rbx]                  ; ebx <- S
    SHA1RND SHA1_R1, 5a827999h
    SHA1RND SHA1_R2, 6ed9eba1h
    SHA1RND SHA1_R3, 8f1bbcdch
    CALCW   eax, rbp + 16*4
    or      rcx, 3
@@:
    CALCW   eax, rbp + 16*4 + rcx*4
    loop    @B
    SHA1RND SHA1_R4, 0ca62c1d6h
    pop     rax                         ; rax <- addr of States
    add     [rax], ebx
    add     [rax + 4], edx
    add     [rax + 8], esi
    add     [rax + 12], edi
    add     [rax + 16], r10d
    leave
    ret
SHA1_transform  ENDP

SHA1_bs64shl3   PROC
    mov     rax, rcx
    shl     rax, 3
    bswap   rax
    ret
SHA1_bs64shl3   ENDP

