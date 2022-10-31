.code
;--------------------------------------------------------------
;void local_irq_save(
; UINT32 * eflags)
;
;
; PROC: the compiler will automatically insert prolog and epilog
local_irq_save PROC eflagsPtr:QWORD
  mov rcx, eflagsPtr
  pushfq
  pop rax
  mov qword ptr [rcx],rax
  cli
  ret
local_irq_save ENDP
 
;--------------------------------------------------------------
;void local_irq_restore(
; UINT32  eflags)
;
;
local_irq_restore PROC eflags:QWORD
  mov rax, eflags
  push rax
  popfq
  ret
local_irq_restore  ENDP
;--------------------------------------------------------------
;void local_irq_restore(
; UINT32  eflags)
;
;

EfiReadTsc PROC 
  rdtsc
  shl     rdx, 20h
  or      rax, rdx
  ret
EfiReadTsc ENDP
  END
