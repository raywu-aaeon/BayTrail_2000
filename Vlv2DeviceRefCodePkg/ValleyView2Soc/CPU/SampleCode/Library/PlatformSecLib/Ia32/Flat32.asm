;
; This file contains an 'Intel Peripheral Driver' and is      
; licensed for Intel CPUs and chipsets under the terms of your
; license agreement with Intel or your vendor.  This file may 
; be modified by the user, subject to additional terms of the 
; license agreement                                           
;
;------------------------------------------------------------------------------
;
; Copyright (c) 1999 - 2013, Intel Corporation. All rights reserved.<BR>
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
;  Flat32.asm
;
; Abstract:
;
;  This is the code that goes from real-mode to protected mode.
;  It consumes the reset vector.
;
;------------------------------------------------------------------------------
  INCLUDE Platform.inc
  INCLUDE Ia32.inc
  INCLUDE Chipset.inc
  INCLUDE SecCore.inc

.686p
.xmm
.model small, c

;PUBLIC  TopOfCar
EXTRN   SecStartup:NEAR
;EXTRN   MicrocodeStart:DWORD
;EXTRN   MicrocodeEnd:DWORD

; ECP porting
EXTRN   PcdGet32 (PcdFlashNvStorageMicrocodeBase):DWORD
EXTRN   PcdGet32 (PcdFlashNvStorageMicrocodeSize):DWORD
EXTRN   PcdGet32 (PcdFlashAreaBaseAddress):DWORD
EXTRN   PcdGet32 (PcdFlashAreaSize):DWORD
EXTRN   PcdGet32 (PcdTemporaryRamBase):DWORD
EXTRN   PcdGet32 (PcdTemporaryRamSize):DWORD
EXTRN   PcdGet64 (PcdPciExpressBaseAddress):QWORD
EXTRN   PcdGet32 (PcdFlashMicroCodeAddress):DWORD
EXTRN   PcdGet32 (PcdFlashMicroCodeSize):DWORD
EXTRN   PcdGet32 (PcdFlashMicroCode2Address):DWORD

_TEXT_REALMODE      SEGMENT PARA PUBLIC USE16 'CODE'
                    ASSUME  CS:_TEXT_REALMODE, DS:_TEXT_REALMODE

;------------------------------------------------------------------------------
;
;  SEC "Security" Code module.
;
;  Transition to non-paged flat-model protected mode from a
;  hard-coded GDT that provides exactly two descriptors.
;  This is a bare bones transition to protected mode only
;  used for while in PEI and possibly DXE.
;
;  IA32 specific cache as RAM modules
;
;  After enabling protected mode, a far jump is executed to
;  TransferToPEI using the newly loaded GDT.
;  This code also enables the Cache-as-RAM
;
;  RETURNS:    none
;
;  MMX Usage:
;              MM0 = BIST State
;              MM1 = Current Package Physical Info
;                    [7:0]   = Cluster ID
;                    [15:8]  = Total Prossor pacakge detected in system
;                    [16] = BAD CMOS Flag
;                    [17] = AuburnDale or ClarksField
;                           [0] = AuburnDale
;                           [1] = ClarksField
;                    [18] = Contain SEC reset flag
;                           CPU Only Reset Flag
;                    [19] = Contain SEC reset flag
;                           Power Good Reset Flag
;                    [23:20] = Reserved
;                    [31:24] = Reserved
;              MM2 = store common MAX & MIN ratio
;              MM3 = Patch Revision
;              MM4 = Patch Pointer
;              MM5 = Reserved
;              MM6 = Reserved
;              MM7 = Used in CALL_MMX & RET_ESI micaro
;
;------------------------------------------------------------------------------

; Nehalem Reset Boot Flow Start

align 4
Flat32Start PROC NEAR C PUBLIC
  ;
  ; Save BIST state in MM0
  ;
  fninit                                ; clear any pending Floating point exceptions
  movd    mm0, eax
  cli
  
;----------------------------------------------------------------------------------------
; "Merlin" support
;----------------------------------------------------------------------------------------
  xor     eax, eax
  mov     es, ax
  mov     ax, cs
  mov     ds, ax

;******************************************************************************
; BEGIN WARM-START CHANGE
;******************************************************************************
;
; PLATFORM-SPECIFIC EQUATES!
; These equates define an address which has the following requirements
; on the target platform:
; 1. After booting DOS, the memory is not used by other DOS applications
;    or drivers (thus very platform/configuration specific). 
;    Minimum of roughly 8 bytes required.
; 2. The memory contents and address range are not affected by an INIT
; 3. By default, after booting DOS, the first 4 bytes at this address 
;    contain either 0 (cleared memory) or 0xFFFFFFFF.
; 4. After booting DOS, the memory is writable
;
; It's expected that a manual inspection (using ITP) is performed to ensure
; that the requirements are met. If the manual inspection fails, then a 
; different address must be identified, the below two equates must be
; changed accordingly, and the platform firmware must be rebuilt.
; Note that simply changing the platform hardware configuration could
; break this firmware because drivers may be loaded differently in
; memory, potentially using the address arbitrarily chosen here.
; 
  ;
  ; Check if value in magic address contains non-zero/non-FF value.
  ; It should actually contain executable code, typically a jmp 
  ; instruction.
  ;
  mov  ax, MAGIC_SEG
  mov es, ax
  mov al, BYTE PTR es:[MAGIC_ADDRESS_IN_SEG]

  ; Check for zero value
  cmp al, 0EAh ; EA is the FAR JMP opcode that Merlin inserts
  jnz NotWarmStart

  ;
  ; Check APIC_BASE_MSR.BIT8 to see if we're the BSP
  ;
  mov cx, MSR_APIC_BASE
  rdmsr
  test ah, 1
  jz TightLoop
  ;
  ; We're the BSP, so jump to the magic address. 
  ;
  DB  0EAh
  DW  MAGIC_ADDRESS_IN_SEG
  DW  MAGIC_SEG

  ; Not reached
NotWarmStart:

;******************************************************************************
; END WARM-START CHANGE
;******************************************************************************

  ;
  ; Enter Protected mode.
  ; 
  STATUS_CODE (01h)                     ; BSP_PROTECTED_MODE_START
  mov     esi,  OFFSET GdtDesc
  DB      66h
  lgdt    fword ptr cs:[si]
  mov     eax, cr0                      ; Get control register 0
  or      eax, 00000003h                ; Set PE bit (bit #0) & MP bit (bit #1)
  mov     cr0, eax                      ; Activate protected mode
  mov     eax, cr4                      ; Get control register 4
  or      eax, 00000600h                ; Set OSFXSR bit (bit #9) & OSXMMEXCPT bit (bit #10)
  mov     cr4, eax

  ;
  ; Now we're in Protected16
  ; Set up the selectors for protected mode entry
  ;
  mov     ax, SYS_DATA_SEL
  mov     ds, ax
  mov     es, ax
  mov     fs, ax
  mov     gs, ax
  mov     ss, ax

  ;
  ; Go to Protected32
  ;
  mov     esi, offset NemInitLinearAddress
  jmp     fword ptr cs:[si]

TightLoop:
  cli
  hlt
  jmp     TightLoop

Flat32Start ENDP
_TEXT_REALMODE      ENDS

_TEXT_PROTECTED_MODE      SEGMENT PARA PUBLIC USE32 'CODE'
                          ASSUME  CS:_TEXT_PROTECTED_MODE, DS:_TEXT_PROTECTED_MODE

CALL_MMX macro   RoutineLabel

  local   ReturnAddress
  mov     esi, offset ReturnAddress
  movd    mm7, esi                      ; save ReturnAddress into MM7
  jmp     RoutineLabel
ReturnAddress:

endm

RET_ESI  macro

  movd    esi, mm7                      ; restore ESP from MM7
  jmp     esi

endm

CALL_EBP macro   RoutineLabel

  local   ReturnAddress
  mov     ebp, offset ReturnAddress
  jmp     RoutineLabel
ReturnAddress:

endm

RET_EBP  macro

  jmp     ebp                           ; restore ESP from EBP

endm

align 4
ProtectedModeSECStart PROC NEAR PUBLIC

  STATUS_CODE (02h)
  STATUS_CODE (03h)

  CALL_MMX  PlatformInitialization
  STATUS_CODE (04h)


   ; w/a that sets bits[17:16] of REG_SB_BIOS_CONFIG(0x06) before setting bits[1:0] of BIOS REST CPL(0x5) below.
   ; The default value of these bits is 0x0, so if we dont set them here, then Punit will PG Dfx; and then
   ; even if we set the bits to 0x3 later, they will not take affect and Dfx will Stay PG'ed.
   ; In other words, the only valid flow for these bits later in code is 1->0,  not 0->1.
   ;
   ; bits[17:16] will get written again in Pei Phase(LmSetup.c) based on PDM/Dfx setting in BIOS-Setup under Debug options
        mov     edx, 0CF8h        ;config MCD
        mov     eax, 800000d4h
        out     dx, eax

        mov     edx, 0CFCh        ;set Bit16 and Bit17
        mov     eax, 30000h
        out     dx, eax

        mov     edx, 0CF8h        ;config MCR
        mov     eax, 800000d0h
        out     dx, eax

        mov     edx, 0CFCh        ;write_opcode+portID+offset+(WriteEnByte = 4, for byte2)
        mov     eax, 007040640h
        out     dx, eax

   ; For A-step, bits 0 and 1 need to be set after patch load and before MRC (first possible reset)
   ; Without setting BIOS_RESET_DONE on A0, Bios will stuck at vlvdxe after GT PowerManagement
   ; Could be done later in flow for B-step, but no need to move and have it in 2 places.
   ;
   ;Program BIOS_RESET_DONE (bit0) and BIOS_ALL_DONE (bit1) in PUNIT.BIOS_RESET_CPL, Port 0x4, Offset 0x5
   ;BIOS_ALL_DONE is newly added bit for B0.
        mov     edx, 0CF8h        ;config MCD
        mov     eax, 800000d4h
        out     dx, eax

        mov     edx, 0CFCh        ;set Bit0 and Bit1
        mov     eax, 3
        out     dx, eax

        mov     edx, 0CF8h        ;config MCR
        mov     eax, 800000d0h
        out     dx, eax

        mov     edx, 0CFCh        ;write_opcode+portID+offset(WriteEnByte = 1, for byte0)
        mov     eax, 007040510h
        out     dx, eax


  STATUS_CODE (09h)
  CALL_MMX  InitializeNEM

  STATUS_CODE (0Ah)
  CALL_MMX  EstablishStack              ; for CPU SV

  STATUS_CODE (0Bh)
  
  jmp  CallPeiCoreEntryPoint

ProtectedModeSECStart ENDP

ProtectedModeEntryPoint PROC NEAR PUBLIC

  RET_ESI

ProtectedModeEntryPoint  ENDP

  ;STATUS_CODE (03h)
PlatformInitialization    PROC    NEAR    PRIVATE

    ;
    ;  Check the S3 Boot Mode Code Begins here
    ;
    ;   If S3 mode jump to S3mode:
    ;   
    
    mov eax, 08000f800h + 040h                          ; PCI_LPC_BASE + R_PCH_LPC_ACPI_BASE
    mov dx,  0CF8h
    out dx,  eax
    mov ax,  0400h + 002h                               ; ACPI_BASE_ADDRESS + B_PCH_LPC_ACPI_BASE_EN
    add dx,  4
    out dx,  ax

    mov     dx, 0400h + 004h                            ; ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT
    in      ax, dx
    cmp     ax, 1400h
    je      S3mode

    ;
    ;  Check the S3 Boot Mode Code Ends here
    ;

    
    ;
    ;  Fast boot optimization Code Begins here
    ;
    ;   Enable Enhanced Intel SpeedStep Technology
    ;   and configure to run in High Frequency Mode
    ;        
   
    mov     ecx, IA32_PLATFORM_ID       ; To get Platform ID.
    rdmsr
    and    eax, BIT17                  ; Check if speed step supported bit is set
    jnz      @F                        ; if no (bit is 0), not to set speed step enable bit. 

    ; 1) Enable Intel SpeedStep Technology - BIT16 
    mov ecx, MSR_IA32_MISC_ENABLES 
    rdmsr
    mov edi, eax   ; Save eax
    mov esi, edx   ; Save edx
    or eax, BIT16 
    wrmsr
    
    mov ecx, MSR_IACORE_RATIOS 
    rdmsr
    shr eax, 8
    and eax, 0ff00h
    mov ebx, eax    ; Bus Ratio from bit 16:23 to bit 8:15

    mov ecx, MSR_IACORE_VIDS 
    rdmsr
    shr eax, 16
    and eax, 0ffh   ; VID from bit 16:23 to bit 0:7
    or eax, ebx     ; Combine BusRatio and VID

    ; 2) Program Bus and VID to max
    ;IA32_PERF_CTL:
    ;  BUS_RATIO_SEL[12:8], VID_SEL[6:0]
    
    xor edx, edx
    mov ecx, MSR_IA32_PERF_CTL
    wrmsr
    
    ; 3) Restore MSR_IA32_MISC_ENABLES Settings
    mov ecx, MSR_IA32_MISC_ENABLES 
    mov eax, edi   ; Restore eax 
    mov edx, esi   ; Restore edx
    wrmsr
    ;
    ;  Fast boot optimization Code ends here
    ;
@@:
    

    ;
    ;  S3 Boot Mode jump to here
    ;
    
    S3mode:

  ;
  ; Program ECBASE and enable EC space in BUNIT.BECREG  Port 0x3, Offset 0x27
  ;
  mov                   eax,    0800000d4h
  mov                   dx,     0CF8h
  out                   dx,     eax
  mov                   eax,    0e0000000h OR 1                 ; MKF_EC_BASE_ADDRESS
  mov                   dx,     0CFCh
  out                   dx,     eax
  mov                   eax,    0800000d0h
  mov                   dx,     0CF8h
  out                   dx,     eax
  mov                   eax,    0110327f0h
  mov                   dx,     0CFCh
  out                   dx,     eax

;Update Microcode

 CALL_EBP  VeryEarlyMicrocodeUpdate      

  ;
  ; Program and enable all known base addresses
  ;

  ;
  ; Program and enable SPI Base Address
  ;
  mov   edi,  0e0000000h + 0F8054h                              ; MKF_EC_BASE_ADDRESS + 0:1F:0:54
  mov   DWORD PTR [edi], 0fed01000h                             ; MKF_SPI_BASE_ADDRESS Set base address.
  mov   DWORD PTR [edi], 0fed01000h OR 2h                       ; MKF_SPI_BASE_ADDRESS Set enable bit

   mov ecx, 10
   mov edi, offset PlatformInitTable
programddtable:
   mov esi, DWORD PTR [edi]
   mov eax, DWORD PTR [edi+4]
   mov DWORD PTR [esi], eax
   add edi, 8
   loop programddtable;

   mov ecx, 3
programdwtable:
   mov esi, DWORD PTR [edi]
   mov eax, DWORD PTR [edi+4]
   mov WORD PTR [esi], ax
   add edi, 8
   loop programdwtable;
    
   mov esi, DWORD PTR [edi]
   mov eax, DWORD PTR [edi+4]
   mov BYTE PTR [esi], al
   add edi, 8
    
  ;
  ; End program and enable all known base addresses
  ;
        
  ;
  ; Check RTC power well first
  ;
  mov     edi, 0fed03000h + 020h                        ; PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1
  mov     ax,  WORD PTR [edi]
  test    ax,  0200h                                                                    ; B_PCH_PMC_GEN_PMCON_GEN_RST_STS
  jz      check_RTC_PWR_STS

force_cold_boot_path:
  mov     cx, ax                                                ; Save
    
  mov     dx, 0400h + 004h                                      ; ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT
  in      ax, dx
  and     ax, NOT (01C00h)                                      ; V_PCH_ACPI_PM1_CNT_S5 ; Clear sleep type field SLP_TYP [12:10]
  out     dx, ax

  mov     ax, cx                                                ; restore

check_RTC_PWR_STS:
  test    ax, 004h                                              ; B_PCH_PMC_GEN_PMCON_RTC_PWR_STS
  jz      no_RTC_pwr_failure
    
  ;
  ; According to VLV BIOS Specification, the following sequence must be programmed
  ; in order to ensure RTC state has been initialized.
  ;
  ; The System BIOS should execute the sequence below if the RTC_PWR_STS bit is set before memory initialization.
  ; This will ensure that the RTC state machine has been initialized.
  ;  1. If the RTC_PWR_STS bit is set, steps 2 through 5 should be executed.
  ;  2. Set RTC Register 0Ah[6:4] to '110' or '111'.
  ;  3. Set RTC Register 0Bh[7].
  ;  4. Set RTC Register 0Ah[6:4] to '010'.
  ;  5. Clear RTC Register 0Bh[7].
    
init_RTC_state_machine:
    
  ;
  ; Set RTC Register 0Ah[6:4] to '110' or '111'.
  ;
  mov     al, 0Ah
  out     070h, al
  nop                  ;delay
  nop                  ;delay
  mov     al, 066h
  out     071h, al
  nop                  ;delay
  nop                  ;delay

  ;
  ; Set RTC Register 0Bh[7].
  ;
  mov     al, 0Bh
  out     070h, al
  nop                  ;delay
  nop                  ;delay
  in      al, 071h
  nop                  ;delay
  nop                  ;delay
  or      al, 080h
  out     071h, al
  nop                  ;delay
  nop                  ;delay

  ;
  ; Set RTC Register 0Ah[6:4] to '010'.
  ;
  mov     al, 0Ah
  out     070h, al
  nop                  ;delay
  nop                  ;delay
  mov     al, 026h
  out     071h, al
  nop                  ;delay
  nop                  ;delay

  ;
  ; Clear RTC Register 0Bh[7].
  ;
  mov     al, 0Bh
  out     070h, al
  nop                  ;delay
  nop                  ;delay
  in      al, 071h
  nop                  ;delay
  nop                  ;delay
  and     al, NOT 080h
  out     071h, al
  nop                  ;delay
  nop                  ;delay

CMOS_PS2_VCC_VNN_CONTROL           equ 034h		
; Set default PS2_VCC_VNN Control to 0 if RTC bad
  mov     al, CMOS_PS2_VCC_VNN_CONTROL
  out     070h, al
  nop                  ;delay
  nop                  ;delay
  mov     al, 00h
  out     071h, al

no_RTC_pwr_failure:
  ;
  ; Program PS2EN_VNN_VCC [bits 3:2] in PUNIT.BIOS_CONFIG Register, Port 0x04, Offset 0x06
  ; PS2EN_VNN_VCC is newly added bits in C0

  mov	edx, 0CF8h        
  mov	eax, 800000d0h    ;config Message Bus Control Register (MCR)
  out	dx, eax

  mov	edx, 0CFCh        
  mov	eax, 06040610h   ;Read_opcode[bits31:24]+portID[bits23:16]+offset[bits15:8]+ByteEnable[7:4]+0  (ByteEnable = 1 for byte0)
  out	dx, eax
	
  mov	edx, 0CF8h        
  mov	eax, 800000d4h    ;config Message Data Register (MDR) 
  out	dx, eax

  mov	edx, 0CFCh        ;get status of Vcc and Vnn for PS2
  in	eax, dx	
  mov edi, eax		  ; save eax
	    
; Read PS2_VCC_VNN setting from CMOS
  mov     al, CMOS_PS2_VCC_VNN_CONTROL
  out     070h, al
  nop                  ;delay
  nop                  ;delay
  in      al, 071h
  test    al, 03h		; bits 00 - disable, 3 - enable both Vcc and Vnn for PS2
  jz      disable_PS2_Vcc_Vnn
	    
  mov	edx, 0CF8h        
  mov	eax, 800000d4h    ;config Message Data Register (MDR) 
  out	dx, eax

  mov	edx, 0CFCh        ;set Bit3 and Bit2 to enable Vcc and Vnn for PS2
  mov	eax, edi		  ;restore eax
  or    eax, 0ch		  ;bit3:2 = 11 PS2 enabled for VCC and VNN rails for low power scenarios in SOC S0 state
  out	dx, eax
  jmp   update_PS2_Vcc_Vnn
 
disable_PS2_Vcc_Vnn:
  mov	edx, 0CF8h        
  mov	eax, 800000d4h    ;config Message Data Register (MDR) 
  out	dx, eax

  mov	edx, 0CFCh        ;clear Bit3 and Bit2 to disable Vcc and Vnn for PS2
  mov	eax, edi		  ;restore eax
  and   eax, 0ffffff03h	  ;bit3:2 = 00 PS2 disabled for VCC and VNN rails for low power scenarios in SOC S0 state
  out	dx, eax

update_PS2_Vcc_Vnn:
  mov	edx, 0CF8h        
  mov	eax, 800000d0h    ;config Message Bus Control Register (MCR)
  out	dx, eax

  mov	edx, 0CFCh        ;Write_opcode[bits31:24]+portID[bits23:16]+offset[bits15:8]+ByteEnable[7:4]+0  (ByteEnable = 1 for byte0)
  mov	eax, 007040610h
  out	dx, eax
  
  ;
  ; Enable SPI Prefetch
  ;
  mov           edi, 0fed01000h + 0FCh                      ; SPI_BASE_ADDRESS BIOS Control Register
  or            Dword Ptr [edi], 08h                        ; Bits [3:2] = '10' - enable prefetching and caching.

  ;
  ; Program 8259 Interrupt Controller to disable all interrupts
  ;
  mov     al, 0FFh
  out     21h, al     ; Mask off all interrupts in master 8259
  out     0a1h, al    ; Mask off all interrupts in slave 8259

  ;
  ; Halt TCO Timer
  ;
  mov     dx, 0400h + 068h                                  ; ACPI_BASE_ADDRESS + R_PCH_TCO_CNT
  in      ax, dx
  or      ax, 0800h                                         ; B_PCH_TCO_CNT_TMR_HLT
  out     dx, ax

  ;
  ; Clear the Second Timeout Status bit by writing 1
  ;
  ;mov     dx,  0400h + 064h                                ; ACPI_BASE_ADDRESS + R_PCH_TCO_STS
  ;in      eax, dx
  ;or      eax, 020000h                                     ; B_PCH_TCO_STS_SECOND_TO
  ;out     dx,  eax
    
  ;
  ; Enable SMBUS Host Controller
  ;
  mov     eax, 08000fb00h + 040h                            ; PCI_SMBUS_BASE + R_PCH_SMBUS_HOSTC
  mov     dx,  0CF8h
  out     dx,  eax
  mov     al,  001h                                         ; B_PCH_SMBUS_HOSTC_HST_EN
  add     dx,  4
  out     dx,  al

  mov       dx,  0efa0h +       00Dh                        ; SMBUS_BASE_ADDRESS + R_PCH_SMBUS_AUXC
  xor       al,  al
  out       dx,  al

  ;
  ; Check to see if 0xCF9 Global Reset bit is set. if set clear it.
  ;
  mov     edi, 0fed03000h + 048h                            ; MKF_PMC_BASE_ADDRESS + R_PCH_PMC_PMIR
  mov     eax, DWORD PTR [edi]
  test    eax, 00100000h                                    ; B_PCH_PMC_PMIR_CF9GR  ; Check whether 0xCF9 Global Reset bit is set
  jz      GlobalresetClear                                  ; If no, continue
  and     eax, NOT (00100000h)                              ; B_PCH_PMC_PMIR_CF9GR  ; Clear 0xCF9 Global Reset bit
  mov     DWORD PTR [edi], eax

GlobalresetClear:
  ;
  ; Clear HPET Timer 0 Lower and Upper Comparator Value.
  ;
  xor     eax, eax
  mov     esi, 0fed00108h                                   ; HPET_COMP_1
  mov     Dword Ptr [esi], eax
  mov     esi, 0fed0010ch                                   ; HPET_COMP_2
  mov     Dword ptr [esi], eax

  ;
  ; Halt TCO Timer
  ;
  mov     dx, 0468h
  in      ax, dx
  or      ax, BIT11
  out     dx, ax

  ;
  ; Clear the Second TO status bit
  ;
  ;mov     dx, 0466h
  ;in      ax, dx
  ;or      ax, BIT1
  ;out     dx, ax

  ;
  ; Disable internal UART (from CM)
  ;
  
    mov edi,  0fed08000h + 088h                             ;MKF_ILB_BASE_ADDRESS + R_PCH_ILB_IRQE
    mov eax,  DWORD PTR [edi]
    and eax,  NOT 018h                                      ;B_PCH_ILB_IRQE_UARTIRQEN ; Disable UARTs IRQ3 & IRQ4
    mov DWORD PTR [edi], eax

    mov eax, PCI_LPC_BASE + 080h                            ;R_PCH_LPC_UART_CTRL
    mov dx,  0CF8h
    out dx,  eax
    mov al,  0h  ;for 1 uart3, 0 for sio uart.
    add dx,  4
    out dx,  al


  ;
  ; Ported from NBSECInit.ASM
  ;

  ; Read Bunit.BMISC Port 0x3, Offset 0x28 bit 1 to chk F-segment set
  ; Determine if INIT or Hard Reset
  mov   edx, 0CF8h                                          ;config MCRX
  mov   eax, 0800000d8h
  out   dx, eax

  mov   edx, 0CFCh                                          ;out zero on MCRx
  xor   eax, eax
  out   dx, eax

  mov   edx, 0CF8h                                          ;config MCR
  mov   eax, 0800000d0h
  out   dx, eax

  mov   edx, 0CFCh
  mov   eax, 0100328f0h
  out   dx, eax

  mov   edx, 0CF8h                                          ;config MDR to read Data
  mov   eax, 0800000d4h
  out   dx, eax

  mov   edx, 0CFCh
  in    eax,  dx
  bt    eax, 001h                                          ;chk bit offset 1
  jnb   @f
 
reset:
  ;
  ; Do a hard Reset if INIT.
  ;
  mov   al, 6
  mov   dx, 0cf9h
  out   dx, al                                          ;hard reset
  jmp   $

@@:

  ;Enable Device
  mov   dx, 0cf8h
  mov   eax,80000054h
  out   dx, eax
  add   dx, 4
  mov   eax,0b8000019h
  out   dx, eax

  ;
  ; Ported from NBSECInit.ASM end
  ;

  RET_ESI

PlatformInitialization    ENDP

;  STATUS_CODE (07h)
VeryEarlyMicrocodeUpdate    PROC    NEAR    PRIVATE

    mov     ecx, IA32_BIOS_SIGN_ID
    rdmsr                               ; CPU PatchID -> EDX
    cmp     edx, 0                      ; If microcode has been updated
    jnz     UpdateOver                  ; Skip if patch already loaded


    movd    mm3, edx                    ; Reset patch revision in mm3
    mov     ecx, IA32_PLATFORM_ID       ; To get Platform ID.
    rdmsr
    shr     edx, 18                     ; EDX[0-2] = Platform ID.
    and     dx, 07h                     ; DX = Platform ID.
    mov     si, dx                      ; Save Platform ID in FS.
    mov     eax, 01h                    ; To get CPU signature.
    cpuid                               ; EAX = CPU signature.
    movd     mm5,eax
    mov     cx, si                      ; CX = Platform ID
    xor     edx, edx
    bts     dx, cx                      ; EDX = Platform ID bit.

    movd     mm6,edx
    mov     esi, PcdGet32 (PcdFlashMicroCodeAddress)
    mov     ebx, esi
    mov     bx,  FVHEADER_LEN_OFF
    movzx   ebx, WORD PTR [ebx]
    add     esi, ebx
    add     si,  FFSHEADER_LEN ; add FFS header

    mov     edi, PcdGet32 (PcdFlashMicroCodeAddress)
    mov     ebx, PcdGet32 (PcdFlashMicroCodeSize)
    add     edi, ebx                          ;End addr of uCodes.

    ; EAX = CPU signature.
    ; EDX = Platform ID bit.
    ; ESI = Abs addr of contiguous uCode blocks.
    ; EDI = Abs addr of contiguous uCode blocks end.


CheckPatch:
    movd     eax,mm5
    movd     edx,mm6
   
    cmp     (UpdateHeaderStruc PTR ds:[esi]).dProcessorSignature, eax;Sig matched?
    jnz     CheckUnprogrammed         ; No.

    test    (UpdateHeaderStruc PTR ds:[esi]).dProcessorFlags, edx;Platform matched?
    jnz     FoundMatch                ; Yes.


CheckUnprogrammed:
    mov     ebx, (UpdateHeaderStruc PTR ds:[esi]).dDataSize
    cmp     ebx, 0FFFFFFFFh
    ;je      Unprogrammed
    je      UpdateOver
    
    cmp     (UpdateHeaderStruc PTR ds:[esi]).dLoaderRevision, 1
    je      CheckExtdHdrs

Unprogrammed:    
    ;mov     ebx, 1024                   ; Unprogrammed space, 1KB checks
        
    mov    ebx, (UpdateHeaderStruc PTR ds:[esi]).dTotalSize        
    jmp     PoinToNextBlock           ; for backword compatibility.

CheckExtdHdrs:

    add     ebx, SIZEOF(UpdateHeaderStruc)
    cmp     ebx, (UpdateHeaderStruc PTR ds:[esi]).dTotalSize
    jae     TryNextPatch              ; No extd hdrs.
    mov     ecx, DWORD PTR ds:[esi + ebx]
    jcxz    TryNextPatch              ; No extd hdrs. (OK to use CX instead of ECX).
    add     ebx, 20                   ; Point to the first Extd Sig.
NextSig:
    cmp     eax, DWORD PTR ds:[esi + ebx] ;Sig matched?
    jne     @F
    test    edx, DWORD PTR ds:[esi + ebx + 4] ;Platform matched?
    jnz     FoundMatch
@@:
    add     ebx, 12
    loop    NextSig

TryNextPatch: ;ebx:c0b4
    mov     ebx, (UpdateHeaderStruc PTR ds:[esi]).dTotalSize

Assure2KAlignment:
    add     ebx, 7FFh    
    and     ebx, 0FFFFF800h

PoinToNextBlock:

    add     esi, ebx    

    cmp     esi, edi                  ; first time esi:FFF903E0 ;edi:FFFA0000
                                      ; second time esi:FFF9C4A0

    jb      CheckPatch                ; Check with all patches.

    jmp     UpdateOver                ; No matching patch found.

FoundMatch:
;              MM3 = Patch Revision
;              MM4 = Patch Pointer

    movd    ebx, mm3
    cmp     (UpdateHeaderStruc PTR ds:[esi]).dUpdateRevision, ebx
    jbe     TryNextPatch
    mov     ebx, (UpdateHeaderStruc PTR ds:[esi]).dUpdateRevision


StoreRevPtr:
    movd    mm3, ebx                    ; save Patch Revision
    movd    mm4, esi                    ; save Patch Pointer

LoadPatch:
    mov     ecx, IA32_BIOS_UPDT_TRIG
    mov     eax, esi                    ; EAX - Abs addr of uCode patch.
    add     eax, SIZEOF(UpdateHeaderStruc)  ; EAX - Abs addr of uCode data.
    xor     edx, edx                    ; EDX:EAX - Abs addr of uCode data.
   
    wrmsr                               ; Trigger uCode load.
    jmp     TryNextPatch

UpdateOver:

;check the stepping at first.
    mov eax, 08000f800h + 08h                           ; PCI_LPC_BASE + RID
    mov dx,  0CF8h
    out dx,  eax                                ; PMC_BASE_ADDRESS + B_PCH_LPC_PMC_BASE_EN
    add dx,  4
    in  al,  dx

    cmp al, 0FFh      ;it is tricky here. if it is B0, B1, we should get 0xff here because of the Patch in a right case,
                      ;or we don't need to set these 4 MSRs.
    jne     TouchEnd
    
    mov     ecx, 12Ch
    mov     eax, 0FFFFFFFFh
    mov     edx, 07FFFFFFFh  ; all bits except the bit63(not lock)
    wrmsr  

    mov     ecx, 12Dh
    mov     eax, 0FFFFFFFFh
    mov     edx, 07FFFFFFFh  ; all bits except the bit63(not lock)
    wrmsr  

    mov     ecx, 12Eh
    mov     eax, 0FFFFFFFFh
    mov     edx, 07FFFFFFFh  ; all bits except the bit63(not lock)
    wrmsr  

    mov     ecx, 12Fh
    mov     eax, 0FFFFFFFFh
    mov     edx, 07FFFFFFFh  ; all bits except the bit63(not lock)
    wrmsr  
TouchEnd:

    RET_EBP

VeryEarlyMicrocodeUpdate    ENDP


;  STATUS_CODE (09h)
;************************************************************
; Description:
;
;   This function initializes the Cache for Data, Stack, and Code
;   as specified in the  BIOS Writer's Guide.
;************************************************************
InitializeNEM    PROC    NEAR    PRIVATE

  ;
  ;  Enable cache for use as stack and for caching code
  ;  The algorithm is specified in the processor BIOS writer's guide
  ;

  ;
  ;  Ensure that the system is in flat 32 bit protected mode. 
  ;
  ;  Platform Specific - configured earlier
  ;
  ;  Ensure that only one logical processor in the system is the BSP.
  ;  (Required step for clustered systems).
  ;
  ;  Platform Specific - configured earlier
  
  ;  Ensure all APs are in the Wait for SIPI state.
  ;  This includes all other logical processors in the same physical processor
  ;  as the BSP and all logical processors in other physical processors.
  ;  If any APs are awake, the BIOS must put them back into the Wait for
  ;  SIPI state by issuing a broadcast INIT IPI to all excluding self.
  ;
  mov     edi, APIC_ICR_LO               ; 0FEE00300h - Send INIT IPI to all excluding self 
  mov     eax, ORAllButSelf + ORSelfINIT ; 0000C4500h
  mov     [edi], eax

@@:
  mov     eax, [edi]
  bt      eax, 12                       ; Check if send is in progress
  jc      @B                            ; Loop until idle

  ;
  ;   Load microcode update into BSP.
  ;
  ;   Ensure that all variable-range MTRR valid flags are clear and 
  ;   IA32_MTRR_DEF_TYPE MSR E flag is clear.  Note: This is the default state
  ;   after hardware reset.
  ;
  ;   Platform Specific - MTRR are usually in default state.
  ;

  ;
  ;   Initialize all fixed-range and variable-range MTRR register fields to 0.
  ;
   mov   ecx, IA32_MTRR_CAP         ; get variable MTRR support
   rdmsr
   movzx ebx, al                    ; EBX = number of variable MTRR pairs
   shl   ebx, 2                     ; *4 for Base/Mask pair and WORD size
   add   ebx, MtrrCountFixed * 2    ; EBX = size of  Fixed and Variable MTRRs

   xor   eax, eax                   ; Clear the low dword to write
   xor   edx, edx                   ; Clear the high dword to write
   ;;;mov   ebx, MtrrCount * 2      ; ebx <- sizeof MtrrInitTable
InitMtrrLoop:
   add   ebx, -2
   movzx ecx, WORD PTR cs:MtrrInitTable[ebx]  ; ecx <- address of mtrr to zero
   wrmsr
   jnz   InitMtrrLoop                   ; loop through the whole table

  ;
  ;   Configure the default memory type to un-cacheable (UC) in the 
  ;   IA32_MTRR_DEF_TYPE MSR.
  ;
  mov     ecx, MTRR_DEF_TYPE            ; Load the MTRR default type index
  rdmsr
  and     eax, NOT (00000CFFh)          ; Clear the enable bits and def type UC.
  wrmsr

  ; Configure MTRR_PHYS_MASK_HIGH for proper addressing above 4GB
  ; based on the physical address size supported for this processor
  ; This is based on read from CPUID EAX = 080000008h, EAX bits [7:0]
  ;
  ; Examples: 
  ;  MTRR_PHYS_MASK_HIGH = 00000000Fh  For 36 bit addressing
  ;  MTRR_PHYS_MASK_HIGH = 0000000FFh  For 40 bit addressing
  ;
  mov   eax, 80000008h                  ; Address sizes leaf
  cpuid  
  sub   al, 32
  movzx eax, al
  xor   esi, esi
  bts   esi, eax
  dec   esi                             ; esi <- MTRR_PHYS_MASK_HIGH

  ;
  ;   Configure the DataStack region as write-back (WB) cacheable memory type
  ;   using the variable range MTRRs.
  ;

  ;
  ; Set the base address of the DataStack cache range
  ;
  mov     eax, PcdGet32 (PcdTemporaryRamBase)
  or      eax, MTRR_MEMORY_TYPE_WB
                                        ; Load the write-back cache value
  xor     edx, edx                      ; clear upper dword
  mov     ecx, MTRR_PHYS_BASE_0         ; Load the MTRR index
  wrmsr                                 ; the value in MTRR_PHYS_BASE_0

  ;
  ; Set the mask for the DataStack cache range
  ; Compute MTRR mask value:  Mask = NOT (Size - 1)
  ;
  mov  eax, PcdGet32 (PcdTemporaryRamSize)
  dec  eax
  not  eax
  or   eax, MTRR_PHYS_MASK_VALID
                                        ; turn on the Valid flag
  mov  edx, esi                         ; edx <- MTRR_PHYS_MASK_HIGH
  mov  ecx, MTRR_PHYS_MASK_0            ; For proper addressing above 4GB
  wrmsr                                 ; the value in MTRR_PHYS_BASE_0

  ;
  ;   Configure the BIOS code region as write-protected (WP) cacheable
  ;   memory type using a single variable range MTRR.
  ;
  ;   Platform Specific - ensure region to cache meets MTRR requirements for
  ;   size and alignment.
  ;

  ;
  ; Set the base address of the CodeRegion cache range
  ;
  mov     eax, PcdGet32 (PcdFlashAreaSize)
  mov     edi, PcdGet32 (PcdFlashAreaBaseAddress)

  ;
  ; Round up to page size
  ;
  mov     ecx, eax                      ; Save
  and     ecx, 0FFFF0000h               ; Number of pages in 64K
  and     eax, 0FFFFh                   ; Number of "less-than-page" bytes
  jz      Rounded
  mov     eax, 10000h                   ; Add the whole page size

Rounded:
  add     eax, ecx                      ; eax - rounded up code cache size

  ;
  ; Define "local" vars for this routine
  ; Note that mm0 is used to store BIST result for BSP,
  ; mm1 is used to store the number of processor and BSP APIC ID,
  ; mm2 is used to store common MAX & MIN ratio
  ;
  CODE_SIZE_TO_CACHE    TEXTEQU  <mm3>
  CODE_BASE_TO_CACHE    TEXTEQU  <mm4>
  NEXT_MTRR_INDEX       TEXTEQU  <mm5>
  NEXT_MTRR_SIZE        TEXTEQU  <mm6>
  ;
  ; Initialize "locals"
  ;
  sub     ecx, ecx
  movd    NEXT_MTRR_INDEX, ecx          ; Count from 0 but start from MTRR_PHYS_BASE_1

  ;
  ; Save remaining size to cache
  ;
  movd    CODE_SIZE_TO_CACHE, eax       ; Size of code cache region that must be cached
  movd    CODE_BASE_TO_CACHE, edi       ; Base code cache address

NextMtrr:
  ;
  ; Get remaining size to cache
  ;
  movd    eax, CODE_SIZE_TO_CACHE
  and     eax, eax
  jz      CodeRegionMtrrdone            ; If no left size - we are done
  ;
  ; Determine next size to cache.
  ; We start from bottom up. Use the following algorythm:
  ; 1. Get our own alignment. Max size we can cache equals to our alignment
  ; 2. Determine what is bigger - alignment or remaining size to cache.
  ;    If aligment is bigger - cache it.
  ;      Adjust remaing size to cache and base address
  ;      Loop to 1.
  ;    If remaining size to cache is bigger
  ;      Determine the biggest 2^N part of it and cache it.
  ;      Adjust remaing size to cache and base address
  ;      Loop to 1.
  ; 3. End when there is no left size to cache or no left MTRRs
  ;
  movd    edi, CODE_BASE_TO_CACHE
  bsf     ecx, edi                      ; Get index of lowest bit set in base address
  ;
  ; Convert index into size to be cached by next MTRR
  ;
  mov     edx, 1h
  shl     edx, cl                       ; Alignment is in edx
  cmp     edx, eax                      ; What is bigger, alignment or remaining size?
  jbe     gotSize                       ; JIf aligment is less
  ;
  ; Remaining size is bigger. Get the biggest part of it, 2^N in size
  ;
  bsr     ecx, eax                      ; Get index of highest set bit
  ;
  ; Convert index into size to be cached by next MTRR
  ;
  mov     edx, 1
  shl     edx, cl                       ; Size to cache

GotSize:
  mov     eax, edx
  movd    NEXT_MTRR_SIZE, eax           ; Save

  ;
  ; Compute MTRR mask value:  Mask = NOT (Size - 1)
  ;
  dec     eax                           ; eax - size to cache less one byte
  not     eax                           ; eax contains low 32 bits of mask
  or      eax, MTRR_PHYS_MASK_VALID     ; Set valid bit

  ;
  ; Program mask register
  ;
  mov     ecx, MTRR_PHYS_MASK_1         ; setup variable mtrr
  movd    ebx, NEXT_MTRR_INDEX
  add     ecx, ebx

  mov     edx, esi                      ; edx <- MTRR_PHYS_MASK_HIGH
  wrmsr
  ;
  ; Program base register
  ;
  sub     edx, edx
  mov     ecx, MTRR_PHYS_BASE_1         ; setup variable mtrr
  add     ecx, ebx                      ; ebx is still NEXT_MTRR_INDEX

  movd    eax, CODE_BASE_TO_CACHE
  or      eax, MTRR_MEMORY_TYPE_WP      ; set type to write protect
  wrmsr
  ;
  ; Advance and loop
  ; Reduce remaining size to cache
  ;
  movd    ebx, CODE_SIZE_TO_CACHE
  movd    eax, NEXT_MTRR_SIZE
  sub     ebx, eax
  movd    CODE_SIZE_TO_CACHE, ebx

  ;
  ; Increment MTRR index
  ;
  movd    ebx, NEXT_MTRR_INDEX
  add     ebx, 2
  movd    NEXT_MTRR_INDEX, ebx
  ;
  ; Increment base address to cache
  ;
  movd    ebx, CODE_BASE_TO_CACHE 
  movd    eax, NEXT_MTRR_SIZE
  add     ebx, eax
  movd    CODE_BASE_TO_CACHE, ebx 

  jmp     NextMtrr

CodeRegionMtrrdone:
;  ; Program the variable MTRR's MASK register for WDB
;  ; (Write Data Buffer, used in MRC, must be WC type)
;  ;
;  mov     ecx, MTRR_PHYS_MASK_1
;  movd    ebx, NEXT_MTRR_INDEX
;  add     ecx, ebx
;  mov     edx, esi                                          ; edx <- MTRR_PHYS_MASK_HIGH
;  mov     eax, WDB_REGION_SIZE_MASK OR MTRR_PHYS_MASK_VALID ; turn on the Valid flag
;  wrmsr

;  ;
;  ; Program the variable MTRR's BASE register for WDB
;  ;
;  dec     ecx
;  xor     edx, edx
;  mov     eax, WDB_REGION_BASE_ADDRESS OR MTRR_MEMORY_TYPE_WC
;  wrmsr

  ;
  ; Enable the MTRRs by setting the IA32_MTRR_DEF_TYPE MSR E flag.
  ;
  mov     ecx, MTRR_DEF_TYPE            ; Load the MTRR default type index
  rdmsr
  or      eax, MTRR_DEF_TYPE_E          ; Enable variable range MTRRs
  wrmsr

  ; Step 17:
  ;   Enable the L2 cache by setting bit 8 of the BBL_CR_CTL3 MSR.
  ;
  mov     ecx, MSR_BBL_CR_CTL3                        ; L2 cache configuration MSR
  rdmsr
  bt      eax, B_MSR_BBL_CR_CTL3_L2_NOT_PRESENT       ; Check if L2 present
  jc      NoL2Cache
  bt      eax, B_MSR_BBL_CR_CTL3_L2_HARDWARE_ENABLED  ; Check if L2 is HW enabled
  jnc     NoL2Cache
  or      eax, (1 SHL B_MSR_BBL_CR_CTL3_L2_ENABLED)   ; Enable L2
  wrmsr
  
NoL2Cache:
  ;
  ;   Enable the logical processor's (BSP) cache: execute INVD and set 
  ;   CR0.CD = 0, CR0.NW = 0.
  ;
  mov     eax, cr0
  and     eax, NOT (CR0_CACHE_DISABLE + CR0_NO_WRITE)
  invd
  mov     cr0, eax
  ;
  ;   Enable No-Eviction Mode Setup State by setting
  ;   NO_EVICT_MODE  MSR 2E0h bit [0] = '1'.
  ;
  mov     ecx, NO_EVICT_MODE 
  rdmsr
  or      eax, 1
  wrmsr

  ;
  ;   One location in each 64-byte cache line of the DataStack region
  ;   must be written to set all cache values to the modified state.
  ;
  mov     edi, PcdGet32 (PcdTemporaryRamBase)
  mov     ecx, PcdGet32 (PcdTemporaryRamSize)
  shr     ecx, 6
  mov     eax, CACHE_INIT_VALUE
@@:
  mov  [edi], eax
  sfence
  add  edi, 64
  loopd  @b

  ;
  ;   Enable No-Eviction Mode Run State by setting
  ;   NO_EVICT_MODE MSR 2E0h bit [1] = '1'.
  ;
  mov     ecx, NO_EVICT_MODE
  rdmsr
  or      eax, 2
  wrmsr

  ;
  ; Finished with cache configuration
  ;
  
  ;
  ; Optionally Test the Region...
  ;  
  
  ;
  ; Test area by writing and reading
  ;
  cld
  mov     edi, PcdGet32 (PcdTemporaryRamBase)
  mov     ecx, PcdGet32 (PcdTemporaryRamSize) 
  shr     ecx, 2
  mov     eax, CACHE_TEST_VALUE
TestDataStackArea:
  stosd
  cmp     eax, DWORD PTR [edi-4]
  jnz     DataStackTestFail
  loop    TestDataStackArea 
  jmp     DataStackTestPass

  ;
  ; Cache test failed
  ;
DataStackTestFail:
  STATUS_CODE (0D0h)
  jmp     $

  ;
  ; Configuration test failed
  ;
ConfigurationTestFailed:
  STATUS_CODE (0D1h)
  jmp     $

DataStackTestPass:

  ;
  ; At this point you may continue normal execution.  Typically this would include 
  ; reserving stack, initializing the stack pointer, etc.
  ;

  ;
  ; After memory initialization is complete, please follow the algorithm in the BIOS
  ; Writer's Guide to properly transition to a normal system configuration.
  ; The algorithm covers the required sequence to properly exit this mode.
  ;

  RET_ESI

InitializeNEM    ENDP

;  STATUS_CODE (09h)
EstablishStack    PROC    NEAR    PRIVATE

  ;
  ; Enable STACK
  ;

  RET_ESI

EstablishStack    ENDP

;  STATUS_CODE (0Bh)
CallPeiCoreEntryPoint   PROC    NEAR    PRIVATE
  ;
  ; Set stack top pointer
  ;
  mov     esp, PcdGet32 (PcdTemporaryRamBase)
  add     esp, PcdGet32 (PcdTemporaryRamSize)

  ;
  ; Push CPU count to stack first, then AP's (if there is one)
  ; BIST status, and then BSP's
  ;

  ;
  ; Here work around for BIST
  ;
  ; Get number of BSPs
  movd    ecx, mm1
  movzx   ecx, ch

  ; Save number of BSPs
  push  ecx

GetSBSPBist:
  ; Save SBSP BIST 
  movd  eax, mm0 
  push  eax

  ; Save SBSP APIC ID
  movd  eax, mm1 
  shr   eax, BSPApicIDSaveStart               ; Resume APIC ID 
  push  eax

TransferToSecStartup:



  ; Switch to "C" code
  STATUS_CODE (0Ch)
  ;
  ; Pass entry point of the PEI core
  ;
  mov     edi, PEI_CORE_ENTRY_BASE      ; 0FFFFFFE0h
  push    DWORD PTR ds:[edi]

  ;
  ; Pass BFV into the PEI Core
  ;
  mov     edi, FV_MAIN_BASE             ; 0FFFFFFFCh
  push    DWORD PTR ds:[edi]

  ; ECPoverride: SecStartup entry point needs 4 parameters
  push    PcdGet32 (PcdTemporaryRamBase)

  ;
  ; Pass stack size into the PEI Core
  ;
  push    PcdGet32 (PcdTemporaryRamSize)

  ;
  ; Pass Control into the PEI Core
  ;
  call SecStartup
CallPeiCoreEntryPoint   ENDP

StartUpAp       PROC    NEAR

  mov     esi, HPET_COMP_2
  lock    inc  byte ptr [esi]

  DISABLE_CACHE
;
; Halt the AP and wait for the next SIPI
;
Ap_Halt:
  cli
@@:
  hlt
  jmp     @B
  ret
StartUpAp       ENDP

    
CheckValidCMOS    PROC    NEAR    PRIVATE
  ;
  ; Check CMOS Status
  ;
  mov     edi, 0fed03000h + 020h                        ; MKF_PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1
  mov     ax,  WORD PTR [edi]
  
  ; check PWR_FLR and RTC_PWR_STS status 
  and     ax, BIT2
   
  RET_EBP
CheckValidCMOS    ENDP

PlatformInitTable LABEL DWORD
   dd  0e0000000h + 0F8054h, 0fed01000h          ; MKF_EC_BASE_ADDRESS + 0:1F:0:54; MKF_SPI_BASE_ADDRESS Set base address.
   dd  0e0000000h + 0F8054h, 0fed01000h OR 2h    ; MKF_EC_BASE_ADDRESS + 0:1F:0:54; MKF_SPI_BASE_ADDRESS Set enable bit
   dd 0e00f8000h + 044h, 0fed03000h + 002h       ; PCI_LPC_BASE + R_PCH_LPC_PMC_BASE ; ; PMC_BASE_ADDRESS + B_PCH_LPC_PMC_BASE_EN
   dd 0e00fb000h + 014h, 00h                     ; PCI_SMBUS_BASE + R_PCH_SMBUS_BAR1  ; Upper Memory Base Address should equals zero.
   dd 0e00fb000h + 010h, 0fed04000h              ; PCI_SMBUS_BASE + R_PCH_SMBUS_BAR0  ; SMBM_BASE_ADDRESS
   dd 0e00f8000h + 050h, 0fed08000h + 002h       ; PCI_LPC_BASE + R_PCH_LPC_ILB_BASE  ; ILB_BASE_ADDRESS + B_PCH_LPC_ILB_BASE_EN
   dd 0e00f8000h + 04Ch, 0fed0C000h + 002h       ; PCI_LPC_BASE + R_PCH_LPC_IO_BASE   ; IO_BASE_ADDRESS + B_PCH_LPC_IO_BASE_EN
   dd 0e00f8000h + 058h, 0fef00000h + 002h       ; PCI_LPC_BASE + R_PCH_LPC_MPHY_BASE ; MPHY_BASE_ADDRESS + B_PCH_LPC_MPHY_BASE_EN
   dd 0e00f8000h + 05Ch, 0fed05000h + 002h       ; PCI_LPC_BASE + R_PCH_LPC_PUNIT_BASE ; PUNIT_BASE_ADDRESS + B_PCH_LPC_PUNIT_BASE_EN
   dd 0e00f8000h + 0F0h, 0fed1c000h + 001h       ; PCI_LPC_BASE + R_PCH_LPC_RCBA      ; SB_RCBA + B_PCH_LPC_RCBA_EN
; word    
   dd 0e00f8000h + 040h, 0400h + 002h            ; PCI_LPC_BASE + R_PCH_LPC_ACPI_BASE ; ACPI_BASE_ADDRESS + B_PCH_LPC_ACPI_BASE_EN
   dd 0e00f8000h + 048h, 500h + 002h             ; PCI_LPC_BASE + R_PCH_LPC_GPIO_BASE ; GPIO_BASE_ADDRESS + B_PCH_LPC_GPIO_BASE_EN
   dd 0e00fb000h + 020h, 0efa0h                  ; PCI_SMBUS_BASE + R_PCH_SMBUS_BASE   ; SMBUS_BASE_ADDRESS
; byte  
   dd 0e00fb000h + 04h, 001h + 002h              ; PCI_SMBUS_BASE + R_PCH_SMBUS_PCICM  ; B_PCH_SMBUS_PCICMD_MSE + B_PCH_SMBUS_PCICMD_IOSE


MtrrInitTable   LABEL BYTE
    DW  MTRR_DEF_TYPE
    DW  MTRR_FIX_64K_00000
    DW  MTRR_FIX_16K_80000
    DW  MTRR_FIX_16K_A0000
    DW  MTRR_FIX_4K_C0000
    DW  MTRR_FIX_4K_C8000
    DW  MTRR_FIX_4K_D0000
    DW  MTRR_FIX_4K_D8000
    DW  MTRR_FIX_4K_E0000
    DW  MTRR_FIX_4K_E8000
    DW  MTRR_FIX_4K_F0000
    DW  MTRR_FIX_4K_F8000

MtrrCountFixed EQU (($ - MtrrInitTable) / 2)

    DW  MTRR_PHYS_BASE_0
    DW  MTRR_PHYS_MASK_0
    DW  MTRR_PHYS_BASE_1
    DW  MTRR_PHYS_MASK_1
    DW  MTRR_PHYS_BASE_2
    DW  MTRR_PHYS_MASK_2
    DW  MTRR_PHYS_BASE_3
    DW  MTRR_PHYS_MASK_3
    DW  MTRR_PHYS_BASE_4
    DW  MTRR_PHYS_MASK_4
    DW  MTRR_PHYS_BASE_5
    DW  MTRR_PHYS_MASK_5
    DW  MTRR_PHYS_BASE_6
    DW  MTRR_PHYS_MASK_6
    DW  MTRR_PHYS_BASE_7
    DW  MTRR_PHYS_MASK_7
    DW  MTRR_PHYS_BASE_8
    DW  MTRR_PHYS_MASK_8
    DW  MTRR_PHYS_BASE_9
    DW  MTRR_PHYS_MASK_9
MtrrCount      EQU (($ - MtrrInitTable) / 2)

align 10h
PUBLIC  BootGDTtable

;
; GDT[0]: 0x00: Null entry, never used.
;
NULL_SEL        EQU $ - GDT_BASE        ; Selector [0]
GDT_BASE:
BootGDTtable        DD  0
                    DD  0
;
; Linear data segment descriptor
;
LINEAR_SEL      EQU $ - GDT_BASE        ; Selector [0x8]
    DW  0FFFFh                          ; limit 0xFFFFF
    DW  0                               ; base 0
    DB  0
    DB  092h                            ; present, ring 0, data, expand-up, writable
    DB  0CFh                            ; page-granular, 32-bit
    DB  0
;
; Linear code segment descriptor
;
LINEAR_CODE_SEL EQU $ - GDT_BASE        ; Selector [0x10]
    DW  0FFFFh                          ; limit 0xFFFFF
    DW  0                               ; base 0
    DB  0
    DB  09Bh                            ; present, ring 0, data, expand-up, not-writable
    DB  0CFh                            ; page-granular, 32-bit
    DB  0
;
; System data segment descriptor
;
SYS_DATA_SEL    EQU $ - GDT_BASE        ; Selector [0x18]
    DW  0FFFFh                          ; limit 0xFFFFF
    DW  0                               ; base 0
    DB  0
    DB  093h                            ; present, ring 0, data, expand-up, not-writable
    DB  0CFh                            ; page-granular, 32-bit
    DB  0

;
; System code segment descriptor
;
SYS_CODE_SEL    EQU $ - GDT_BASE        ; Selector [0x20]
    DW  0FFFFh                          ; limit 0xFFFFF
    DW  0                               ; base 0
    DB  0
    DB  09Ah                            ; present, ring 0, data, expand-up, writable
    DB  0CFh                            ; page-granular, 32-bit
    DB  0
;
; Spare segment descriptor
;
SYS16_CODE_SEL  EQU $ - GDT_BASE        ; Selector [0x28]
    DW  0FFFFh                          ; limit 0xFFFFF
    DW  0                               ; base 0
    DB  0Eh                             ; Changed from F000 to E000.
    DB  09Bh                            ; present, ring 0, code, expand-up, writable
    DB  00h                             ; byte-granular, 16-bit
    DB  0
;
; Spare segment descriptor
;
SYS16_DATA_SEL  EQU $ - GDT_BASE        ; Selector [0x30]
    DW  0FFFFh                          ; limit 0xFFFF
    DW  0                               ; base 0
    DB  0
    DB  093h                            ; present, ring 0, data, expand-up, not-writable
    DB  00h                             ; byte-granular, 16-bit
    DB  0

;
; Spare segment descriptor
;
SPARE5_SEL      EQU $ - GDT_BASE        ; Selector [0x38]
    DW  0                               ; limit 0
    DW  0                               ; base 0
    DB  0
    DB  0                               ; present, ring 0, data, expand-up, writable
    DB  0                               ; page-granular, 32-bit
    DB  0
GDT_SIZE        EQU $ - BootGDTtable    ; Size, in bytes

GdtDesc:                                ; GDT descriptor
OffsetGDTDesc   EQU $ - Flat32Start
    DW  GDT_SIZE - 1                    ; GDT limit
    DD  OFFSET BootGDTtable             ; GDT base address

NemInitLinearAddress   LABEL   FWORD
NemInitLinearOffset    LABEL   DWORD
    DD  OFFSET ProtectedModeSECStart    ; Offset of our 32 bit code
    DW  LINEAR_CODE_SEL

;TopOfCar  DD  DATA_STACK_BASE_ADDRESS + DATA_STACK_SIZE

_TEXT_PROTECTED_MODE    ENDS
END
