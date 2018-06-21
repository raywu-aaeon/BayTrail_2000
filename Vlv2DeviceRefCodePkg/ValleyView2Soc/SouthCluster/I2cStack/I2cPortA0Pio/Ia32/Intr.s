
#--------------------------------------------------------------
#UINT64
#EfiReadTsc (
#  VOID
#  )
#
ASM_GLOBAL ASM_PFX (EfiReadTsc)
ASM_PFX(EfiReadTsc):
    rdtsc
    ret
# EfiReadTsc  ENDP

#--------------------------------------------------------------
#void local_irq_save(
# UINT32 * eflags)
#
#
# PROC: the compiler will automatically insert prolog and epilog
ASM_GLOBAL ASM_PFX (local_irq_save)
ASM_PFX(local_irq_save):
  push   %ebp
  movl   %esp,%ebp
  
  movl   0x8(%ebp),%ecx
  pushfl
  pop    %eax
  movl   %eax, (%ecx)
  cli
  
  pop    %ebp
  ret
#local_irq_save ENDP
 
#--------------------------------------------------------------
#void (
# UINT32  eflags)
#
#
ASM_GLOBAL ASM_PFX (local_irq_restore)
ASM_PFX(local_irq_restore):
  push   %ebp
  movl   %esp, %ebp
  
  movl   0x8(%ebp), %eax
  
  push   %eax
  popfl
  
  pop    %ebp
  ret
#local_irq_restore  ENDP
