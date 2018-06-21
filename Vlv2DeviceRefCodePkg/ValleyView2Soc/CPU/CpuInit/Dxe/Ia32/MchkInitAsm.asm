;------------------------------------------------------------------------------
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
;   MchkInitAsm.asm
;
; Abstract:
;
;   This is the assembly code for EnableMCE()
;
;------------------------------------------------------------------------------

.686p
.model  flat
.data
.stack
.code
.MMX
.XMM

_EnableMCE  proc near public
  push    ebp               ; C prolog
  mov     ebp, esp

  mov     eax, cr4
  or      eax, 40h
  mov     cr4, eax

  pop     ebp
  ret
_EnableMCE  endp


  end