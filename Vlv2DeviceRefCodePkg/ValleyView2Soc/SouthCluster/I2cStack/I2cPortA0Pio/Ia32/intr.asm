.586p
.MODEL FLAT,C
.code

;UINT64
;EfiReadTsc (
;  VOID
;  )
EfiReadTsc PROC C PUBLIC
    rdtsc
    ret
EfiReadTsc  ENDP

;--------------------------------------------------------------
;void local_irq_save(
; UINT32 * eflags)
;
;
; PROC: the compiler will automatically insert prolog and epilog
local_irq_save PROC C PUBLIC eflagsPtr:DWORD
  mov ecx, eflagsPtr
  pushfd
  pop eax
  mov dword ptr [ecx],eax
  cli
  ret
local_irq_save ENDP
 
;--------------------------------------------------------------
;void local_irq_restore(
; UINT32  eflags)
;
;
local_irq_restore PROC C PUBLIC eflags:DWORD
  mov eax, eflags
  push eax
  popfd
  ret
local_irq_restore  ENDP
END 
;--------------------------------------------------------------
;void local_irq_restore(
; UINT32  eflags)
;
;
