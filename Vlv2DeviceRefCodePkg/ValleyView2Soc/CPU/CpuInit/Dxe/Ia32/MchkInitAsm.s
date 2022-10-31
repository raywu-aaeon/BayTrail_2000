#------------------------------------------------------------------------------
#
# Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
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
#   MchkInitAsm.S
# 
# Abstract:
# 
#   This is the assembly code for EnableMCE()
#
#------------------------------------------------------------------------------

ASM_GLOBAL ASM_PFX(EnableMCE)
ASM_PFX(EnableMCE):
  push    %ebp               # C prolog
  movl    %esp, %ebp
  
  movl    %cr4, %eax
  orl     $0x40, %eax
  movl    %eax, %cr4

  pop     %ebp
  ret
#EnableMCE  endp
