#------------------------------------------------------------------------------
#
# Copyright (c) 2013 Intel Corporation. All rights reserved
# This software and associated documentation (if any) is furnished
# under a license and may only be used or copied in accordance
# with the terms of the license. Except as permitted by such
# license, no part of this software or documentation may be
# reproduced, stored in a retrieval system, or transmitted in any
# form or by any means without the express written consent of
# Intel Corporation.
#
#
# Module Name:
#
#   Cpu.S
#
# Abstract:
#
#   Assembly code of Cpu
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(InitXmm)
ASM_GLOBAL ASM_PFX(CacheInvd)

.equ    IA32_CR4_OSFXSR,        0x200
.equ    IA32_CR4_OSXMMEXCPT,    0x400
.equ    IA32_CR0_MP,            0x2

.equ    IA32_CPUID_SSE2,        0x02000000
.equ    IA32_CPUID_SSE2_B,      26

.code:
#------------------------------------------------------------------------------
#   Set up flags in CR4 for XMM instruction enabling
#------------------------------------------------------------------------------
ASM_PFX(InitXmm):
    push    %ebx

    # Check whether SSE2 is supported
    movl    $1, %eax
    cpuid
    bt      $IA32_CPUID_SSE2_B, %edx
    jnc     L0

    # Enable XMM
    movl    %cr0, %eax
    orl     $IA32_CR0_MP, %eax
    movl    %eax, %cr0
    movl    %cr4, %eax
    orl     $IA32_CR4_OSFXSR, %eax
    orl     $IA32_CR4_OSXMMEXCPT, %eax
    movl    %eax, %cr4

L0:
    pop     %ebx
    ret



#------------------------------------------------------------------------------
#   Invalidate cache
#------------------------------------------------------------------------------
ASM_PFX(CacheInvd):
    invd
    ret
#CacheInvd  ENDP
