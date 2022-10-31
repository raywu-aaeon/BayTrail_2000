#------------------------------------------------------------------------------
#
# Copyright (c) 1999 - 2013 Intel Corporation. All rights reserved
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
#   MpProc.S
# 
# Abstract:
# 
#   This is the code that supports MP
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(MpMtrrSynchUpEntry)
ASM_PFX(MpMtrrSynchUpEntry):
    #
    # Enter no fill cache mode, CD=1(Bit30), NW=0 (Bit29)
    #
    movl %cr0, %eax
    andl $0x0DFFFFFFF, %eax
    orl  $0x040000000, %eax
    movl %eax, %cr0
    #
    # Flush cache
    #
    wbinvd
    #
    # Clear PGE flag Bit 7
    #
    movl %cr4, %eax
    movl %eax, %edx
    andl $0x0FFFFFF7F, %eax
    movl %eax, %cr4
    #
    # Flush all TLBs
    #
    movl %cr3, %eax
    movl %eax, %cr3
    
    movl %edx, %eax
    ret
#MpMtrrSynchUpEntry  ENDP

ASM_GLOBAL ASM_PFX(MpMtrrSynchUpExit)
ASM_PFX(MpMtrrSynchUpExit):
    push    %ebp             # C prolog
    movl    %esp, %ebp
    #
    # Flush all TLBs the second time
    #
    movl %cr3, %eax
    movl %eax, %cr3
    #
    # Enable Normal Mode caching CD=NW=0, CD(Bit30), NW(Bit29)
    #
    movl %cr0, %eax
    andl $0x09FFFFFFF, %eax
    movl %eax, %cr0
    #
    # Set PGE Flag in CR4 if set
    #
    movl 8(%ebp), %eax
    movl %eax, %cr4
    
    pop %ebp
    ret
#MpMtrrSynchUpExit  ENDP
