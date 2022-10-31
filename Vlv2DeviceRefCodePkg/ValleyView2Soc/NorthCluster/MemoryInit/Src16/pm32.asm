;**************************************************************************
;*                                                                        *
;*      Intel Restricted Secret                                           *
;*                                                                        *
;*      Valleyview Memory Reference Code                                   *
;*                                                                        *
;*      Copyright (c) 2012 Intel Corp.                                    *
;*                                                                        *
;*      This program has been developed by Intel Corporation.             *
;*      Licensee has Intel's permission to incorporate this source code   *
;*      into their product, royalty free.  This source code may NOT be    *
;*      redistributed to anyone without Intel's written permission.       *
;*                                                                        *
;*      Intel specifically disclaims all warranties, express or           *
;*      implied, and all liability, including consequential and other     *
;*      indirect damages, for the use of this code, including liability   *
;*      for infringement of any proprietary rights, and including the     *
;*      warranties of merchantability and fitness for a particular        *
;*      purpose.  Intel does not assume any responsibility for any        *
;*      errors which may appear in this code nor any responsibility to    *
;*      update it.                                                        *
;*                                                                        *
;**************************************************************************
;**************************************************************************
;*                                                                        *
;*  PURPOSE:                                                              *
;*                                                                        *
;*      This file contains sample code to initialize processor cache      *
;*      and pass control to the Memory Reference Code                     *
;*                                                                        *
;**************************************************************************
IFNDEF EFI_MEMORY_INIT

	.586P

	include cpu.i
	; Files generated during build time, copied from OUT32 by makefile
	include mrc.i
	include mrcentrypoints.i

;-----------------------------------------------------------------------;
; CALL_DI                                                               ;
;-----------------------------------------------------------------------;
CALL_DI macro RoutineLabel
	local	ReturnAddress
	mov	edi, offset ReturnAddress
	jmp	RoutineLabel
ReturnAddress:
endm

;-----------------------------------------------------------------------;
; RET_DI                                                                ;
;-----------------------------------------------------------------------;
RET_DI macro
	jmp edi
endm

CNR_ExID              	EQU 060F0h
WFD_ExID              	EQU 06170h

MC_MMIO_BASE          	EQU 0FED14000h             ; MCH MMIO register base
SMBUS_BASE_ADDRESS    	EQU 0EFA0h
PMC_BASE_ADDRESS      	EQU 0FED03000h
SMBM_BASE_ADDRESS		EQU	0fed04000h
IO_BASE_ADDRESS			EQU	0fed06000h
ILB_BASE_ADDRESS		EQU	0fed08000h
MPHY_BASE_ADDRESS		EQU	0fef00000h
SB_RCBA					EQU	0fed1c000h
ACPI_BASE_ADDRESS		EQU	0400h
GPIO_BASE_ADDRESS		EQU	0500h
.LIST

; Set MRC_LINEAR_BASE to the Below-1M alias of the flash block
MRC_LINEAR_BASE       EQU (MRC_ADDRESS + 0FFF00000h)

MC_BUS_DEV_FUN        EQU 000h
MC_REG_RID_OFFSET     EQU 008h
MC_MCHBAR_OFFSET      EQU 048h
R_PCH_LPC_PMC_BASE    EQU 044h
  B_PCH_LPC_PMC_BASE_EN            EQU     BIT1
MC_PCIEXBAR_OFFSET    EQU 060h

R_PCH_PMC_PM_CFG 	  		EQU 08h  ;// Power Management Configuration
B_PCH_PMC_PM_CFG_NO_REBOOT 	EQU 10h  ;// No Reboot Strap

BIT0    EQU     001h
BIT1    EQU     002h
BIT2    EQU     004h
BIT3    EQU     008h
BIT4    EQU     010h
BIT7    EQU     080h
BIT9    EQU     0200h
BIT11   EQU     0800h
BIT17   EQU     020000h
BIT20   EQU     00100000h

;
; Defines for PCH DEVICE 31
;
DEFAULT_PCI_BUS_NUMBER_PCH         EQU     00h

PCI_DEVICE_NUMBER_PCH_LPC          EQU     31
PCI_FUNCTION_NUMBER_PCH_LPC        EQU     0

PCI_DEVICE_NUMBER_PCH_SMBUS        EQU     31
PCI_FUNCTION_NUMBER_PCH_SMBUS      EQU     3

;
; Word equate for Bus, Device, Function number for I/O Controller Hub
;
SB_LPC_BUS_DEV_FUNC                EQU     (DEFAULT_PCI_BUS_NUMBER_PCH SHL 8) + ((PCI_DEVICE_NUMBER_PCH_LPC   SHL 3) + PCI_FUNCTION_NUMBER_PCH_LPC)
SB_SMBUS_BUS_DEV_FUNC              EQU     (DEFAULT_PCI_BUS_NUMBER_PCH SHL 8) + ((PCI_DEVICE_NUMBER_PCH_SMBUS SHL 3) + PCI_FUNCTION_NUMBER_PCH_SMBUS)

;
; Define the equates here
;
PCI_LPC_BASE                       EQU     80000000h + (SB_LPC_BUS_DEV_FUNC SHL 8)
PCI_SMBUS_BASE                     EQU     80000000h + (SB_SMBUS_BUS_DEV_FUNC SHL 8)

;
; PCU related registers and bits
;
R_PCH_LPC_ACPI_BASE                EQU     040h
  B_PCH_LPC_ACPI_BASE_EN           EQU     BIT1
R_PCH_LPC_PMC_BASE                 EQU     044h
  B_PCH_LPC_PMC_BASE_EN            EQU     BIT1
R_PCH_LPC_GPIO_BASE                EQU     048h
  B_PCH_LPC_GPIO_BASE_EN           EQU     BIT1
R_PCH_LPC_IO_BASE                  EQU     04Ch
  B_PCH_LPC_IO_BASE_EN             EQU     BIT1
R_PCH_LPC_ILB_BASE                 EQU     050h
  B_PCH_LPC_ILB_BASE_EN            EQU     BIT1
R_PCH_LPC_MPHY_BASE                EQU     058h
  B_PCH_LPC_MPHY_BASE_EN           EQU     BIT1
R_PCH_LPC_RCBA                     EQU     0F0h
  B_PCH_LPC_RCBA_EN                EQU     BIT0
R_PCH_LPC_UART_CTRL                EQU     080h
  B_PCH_LPC_UART_CTRL_COM2_EN      EQU     BIT1
  B_PCH_LPC_UART_CTRL_COM1_EN      EQU     BIT0

;
; ILB related registers and bits
;
R_PCH_ILB_ACPI_CNT                 EQU     000h
R_PCH_ILB_IRQE                     EQU     088h
B_PCH_ILB_IRQE_UARTIRQEN           EQU     018h

;
; PMC related registers and bits
;
R_PCH_PMC_GEN_PMCON_1              EQU     020h            ; General PM Configuration 1
  B_PCH_PMC_GEN_PMCON_RTC_PWR_STS  EQU     BIT2
  B_PCH_PMC_GEN_PMCON_GEN_RST_STS  EQU     BIT9
R_PCH_PMC_PMIR                     EQU     048h            ; Power Management Initialization Register
  B_PCH_PMC_PMIR_CF9GR             EQU     BIT20

;
; ACPI related registers and bits
;
R_PCH_ACPI_PM1_CNT                 EQU     004h
  V_PCH_ACPI_PM1_CNT_S5            EQU     01C00h
R_PCH_TCO_STS                      EQU     064h
  B_PCH_TCO_STS_SECOND_TO          EQU     BIT17
R_PCH_TCO_CNT                      EQU     068h
  B_PCH_TCO_CNT_TMR_HLT            EQU     BIT11

;
; GPIO related registers and bits
;
R_PCH_GPIO_SC_USE_SEL              EQU     000h
R_PCH_GPIO_SC_USE_SEL2             EQU     020h
R_PCH_GPIO_SC_IO_SEL2              EQU     024h

;
; SMBUS related registers and bits
;
R_PCH_SMBUS_PCICMD                 EQU     004h
R_PCH_SMBUS_AUXC                   EQU     00Dh
  B_PCH_SMBUS_PCICMD_MSE           EQU     BIT1
  B_PCH_SMBUS_PCICMD_IOSE          EQU     BIT0
R_PCH_SMBUS_BAR0                   EQU     010h
R_PCH_SMBUS_BAR1                   EQU     014h
R_PCH_SMBUS_BASE                   EQU     020h
R_PCH_SMBUS_HOSTC                  EQU     040h
  B_PCH_SMBUS_HOSTC_HST_EN         EQU     BIT0

_TEXT16         SEGMENT PARA USE16 PUBLIC 'CODE16'

        extern   _CarInit:       NEAR
        extern   _CarExit:       NEAR

        PUBLIC  _returnFromCarInit
        PUBLIC  _returnFromCarExit

ALIGN 16

;------------------------------------------------------------------------------
; _PM32Entry:
;       Initializes CAR, initializes 32-bit protected flat mode and passes
;       control to Memory Reference code. In this sample implementation,
;       entry to this procedure is from a FAR JMP at the reset vector itself.
;
; Input:
;       Nothing.
;
; Output:
;       Nothing.
;
; Destroys:
;       All.
;
; Note:
;------------------------------------------------------------------------------
_PM32Entry      PROC    NEAR    PUBLIC
;	mov	al, 0d3h
;	out	080h, al

	xor	eax, eax
	mov	ax, cs
	mov	ds, ax
	DB	66h
	lgdt	fword ptr cs:GDTDescriptor

	mov	eax, cr0			; Get control register 0
	or	eax, 000000001h			; Set PE bit (bit #0)
	mov	cr0, eax			; Activate protected mode

; Set up selectors for Protected Mode entry.
	mov	ax, SYS_DATA_SEL
	mov	ds, ax
	mov	es, ax
	mov	fs, ax
	mov	gs, ax
	mov	ss, ax

; Execute a 16-bit far jump to reload CS
	DB	0EAh
	DW	OFFSET @f
	DW	F000_CODE_SEL
@@:
; At this point, we are in 16-bit protected mode.
; Jmp to the Cache-As-RAM initialization code.
	jmp     _CarInit
_returnFromCarInit::

; ------------------------------------------------------------------------------------------------------
; PREPARE TO CALL MEMORY REFERENCE CODE. NOTE THAT AT THIS INSTANT, THE PROCESSOR DATA
; SEGMENT REGISTERS ARE SET TO 4GB ADDRESSABILITY MODE, BUT THE INSTRUCTION MODE IS
; STILL 16-BIT MODE
; ------------------------------------------------------------------------------------------------------
_prepareToCallMemRefCode:
	;Program ECBASE and enable EC space in BUNIT.BECREG  Port 0x3, Offset 0x27
  	mov	eax,	0800000d4h
  	mov	dx,	0CF8h
  	out	dx,	eax
  	mov	eax,	0e0000001h
  	mov	dx,	0CFCh
	out	dx,	eax
	mov	eax,	0800000d0h
	mov	dx,	0CF8h
	out	dx,	eax
	mov	eax,	0110327f0h
	mov	dx,	0CFCh
	out	dx,	eax

    ;
    ; Program and enable PMC Base Address
    ;
    mov eax, PCI_LPC_BASE + R_PCH_LPC_PMC_BASE
    mov dx,  0CF8h
    out dx,  eax
    mov eax, PMC_BASE_ADDRESS + B_PCH_LPC_PMC_BASE_EN
    add dx,  4
    out dx,  eax

    ;
    ; Program SMBUS Upper Memory Base Address
    ;
    mov     eax, PCI_SMBUS_BASE + R_PCH_SMBUS_BAR1
    mov     dx,  0CF8h
    out     dx,  eax
    mov     eax, 00h ; Upper Memory Base Address should equals zero.
    add     dx,  4
    out     dx,  eax

    ;
    ; Program SMBUS Lower Memory Base Address
    ;
    mov     eax, PCI_SMBUS_BASE + R_PCH_SMBUS_BAR0
    mov     dx,  0CF8h
    out     dx,  eax
    mov     eax, SMBM_BASE_ADDRESS
    add     dx,  4
    out     dx,  eax

    ;
    ; Program and enable ILB Base Address
    ;
    mov eax, PCI_LPC_BASE + R_PCH_LPC_ILB_BASE
    mov dx,  0CF8h
    out dx,  eax
    mov eax, ILB_BASE_ADDRESS + B_PCH_LPC_ILB_BASE_EN
    add dx,  4
    out dx,  eax

    ;
    ; Program and enable IO Base Address
    ;
    mov eax, PCI_LPC_BASE + R_PCH_LPC_IO_BASE
    mov dx,  0CF8h
    out dx,  eax
    mov eax, IO_BASE_ADDRESS + B_PCH_LPC_IO_BASE_EN
    add dx,  4
    out dx,  eax

    ;
    ; Program and enable MPHY Base Address
    ;
    mov eax, PCI_LPC_BASE + R_PCH_LPC_MPHY_BASE
    mov dx,  0CF8h
    out dx,  eax
    mov eax, MPHY_BASE_ADDRESS + B_PCH_LPC_MPHY_BASE_EN
    add dx,  4
    out dx,  eax

    ;
    ; Program and enable RCBA Base Address
    ;
    mov eax, PCI_LPC_BASE + R_PCH_LPC_RCBA
    mov dx,  0CF8h
    out dx,  eax
    mov eax, SB_RCBA + B_PCH_LPC_RCBA_EN
    add dx,  4
    out dx,  eax

    ;
    ; Program and enable ACPI I/O base address
    ;
    mov eax, PCI_LPC_BASE + R_PCH_LPC_ACPI_BASE
    mov dx,  0CF8h
    out dx,  eax
    mov ax,  ACPI_BASE_ADDRESS + B_PCH_LPC_ACPI_BASE_EN
    add dx,  4
    out dx,  ax

    ;
    ; Program and enable GPIO base address
    ;
    mov eax, PCI_LPC_BASE + R_PCH_LPC_GPIO_BASE
    mov dx,  0CF8h
    out dx,  eax
    mov ax,  GPIO_BASE_ADDRESS + B_PCH_LPC_GPIO_BASE_EN
    add dx,  4
    out dx,  ax

    ;
    ; Program SMBUS I/O Base Address
    ;
    mov     eax, PCI_SMBUS_BASE + R_PCH_SMBUS_BASE
    mov     dx,  0CF8h
    out     dx,  eax
    mov     ax,  SMBUS_BASE_ADDRESS
    add     dx,  4
    out     dx,  ax

    ;
    ; Enable both SMBUS Memory and I/O Space
    ;
    mov     eax, PCI_SMBUS_BASE + R_PCH_SMBUS_PCICMD
    mov     dx,  0CF8h
    out     dx,  eax
    mov     al,  B_PCH_SMBUS_PCICMD_MSE + B_PCH_SMBUS_PCICMD_IOSE
    add     dx,  4
    out     dx,  al

    ;
    ; End program and enable all known base addresses
    ;

    ;
    ; Set SCI interrupt route to IRQ 9.
    ;
    mov   edi,  ILB_BASE_ADDRESS + R_PCH_ILB_ACPI_CNT ; IlbBase + 0x00
    mov   DWORD PTR [edi], 00h                            ; Set 0h as IRQ 9.

    ;
    ; Program 8259 Interrupt Controller to disable all interrupts
    ;
    mov     al, 0FFh
    out     21h, al     ; Mask off all interrupts in master 8259
    out     0a1h, al    ; Mask off all interrupts in slave 8259

    ;
    ; Halt TCO Timer
    ;
    mov     dx, ACPI_BASE_ADDRESS + R_PCH_TCO_CNT
    in      ax, dx
    or      ax, B_PCH_TCO_CNT_TMR_HLT
    out     dx, ax

    ;
    ; Clear the Second Timeout Status bit by writing 1
    ;
    mov     dx,  ACPI_BASE_ADDRESS + R_PCH_TCO_STS
    in      eax, dx
    or      eax, B_PCH_TCO_STS_SECOND_TO
    out     dx,  eax

    ;
    ; Enable SMBUS Host Controller
    ;
    mov     eax, PCI_SMBUS_BASE + R_PCH_SMBUS_HOSTC
    mov     dx,  0CF8h
    out     dx,  eax
    mov     al,  B_PCH_SMBUS_HOSTC_HST_EN
    add     dx,  4
    out     dx,  al

    mov	    dx,  SMBUS_BASE_ADDRESS + R_PCH_SMBUS_AUXC
    xor	    al,  al
    out	    dx,  al

    ;
    ; Check to see if 0xCF9 Global Reset bit is set. if set clear it.
    ;
    mov     edi, PMC_BASE_ADDRESS + R_PCH_PMC_PMIR
    mov     eax, DWORD PTR [edi]
    test    eax, B_PCH_PMC_PMIR_CF9GR      ; Check whether 0xCF9 Global Reset bit is set
    jz      GlobalresetClear	           ; If no, continue
    and     eax, NOT B_PCH_PMC_PMIR_CF9GR  ; Clear 0xCF9 Global Reset bit
    mov     DWORD PTR [edi], eax

GlobalresetClear:

  	; Disable watchdog timer reseting
	mov	eax, PMC_BASE_ADDRESS +  R_PCH_PMC_PM_CFG
	mov	edx, DWORD PTR [eax]
    or  edx, B_PCH_PMC_PM_CFG_NO_REBOOT
	mov	DWORD PTR [eax], edx

; --------------------------------------------------------------------------------
MRC_PARAMETER_FRAME_OFFSET EQU	0

	mov	dx, 0cf8h
	mov	eax, 080000000h OR (0 SHL 16) OR (0 SHL 11) OR (0 SHL 8) OR (0F0h) 
	out	dx, eax
	mov	dx, 0cfch
	mov	eax, 000000048h
	out	dx, eax

	mov	ecx, SIZEOF MRC_PARAMETER_FRAME
	sub	esp, ecx			; Reserve space above top of stack for
						; OEM PLATFORM_HOOKS, CONTEXT and MRC_PARAMETER_FRAME
	mov	ebp, esp			; Initialize EBP for use throughout the routine

	; Initialize local storage to 0
	mov	edi, ebp
	xor	al, al
	rep	stos	byte ptr es:[edi]		; write al (0) to each byte in MRC_PARAMETER_FRAME

	; Setup es, ds, ss, cs to selectors that start at the beginning of MRC in low-flash
	; Leave fs/gs programmed to flat linear segment selector for MMIO accesss
	; Subtract MRC_LINEAR_BASE from esp and ebp to compensate
	mov	ebx, MRC_LINEAR_BASE
	mov	ax, MRC_DATA_SEL
	mov	es, ax
	mov	ds, ax
	mov	ss, ax
	sub	esp, ebx
	sub	ebp, ebx

	; Setup MRC_PARAMETER_FRAME.OemMrcData
	mov	esi, ebp
	call	SetOemMrcData

	CALL_DI	CheckResume
	jnz	NotResume

	; Set CurrentMrcData.BootMode = S3Path Exit to direct ConfigureMemory activity
	mov	[ebp+MRC_PARAMETER_FRAME_OFFSET].MRC_PARAMETER_FRAME.BootMode, S3Path

	jmp	SkipDetectDimms		; Skip DIMM detection portion of MRC during S3 resume

NotResume:
	; Pass MRC_PARAMETER_FRAME pointer in to DetectDimms_FAR in [ecx] for fastcall C calling convention
	mov	ecx, ebp
	add	ecx, MRC_PARAMETER_FRAME_OFFSET

	; Push return address onto the stack manually, so that RETF instruction can get back here
	mov	eax, cs
	push	eax
	mov	eax, offset cs:@f
	push	eax

	;	far jmp MRC_CODE_SEL:DetectDimms_FAR
	DB	066h	; Prefix jump to use 32-bit offset
	DB	0EAh 	; EA cp JMP ptr16:32 Inv. Valid Jump far, absolute, address given in operand
	DD	DetectDimms_FAR
	DW	MRC_CODE_SEL
@@:
	; Check DetectDimms Exit status.  0 == Success
	cmp	al, 0
	je	NoDetectDimmsError
;	mov	al, 0E0h
;	out	080h, al
@@:	;pause
	db	0f3h, 090h

	jmp @b

NoDetectDimmsError:
SkipDetectDimms:

	; Pass MRC_PARAMETER_FRAME pointer in to DetectDimms_FAR in [ecx] for fastcall C calling convention
	mov	ecx, ebp
	add	ecx, MRC_PARAMETER_FRAME_OFFSET

	; Simulate a far call by pushing return pointer on the stack and performing far jump with 32-bit offset
	;	far call	MRC_CODE_SEL:ConfigureMemory_FAR
	mov	eax, cs
	push	eax
	mov	eax, offset cs:@f
	push	eax

	DB	066h	; Prefix jump to use 32-bit offset
	DB	0EAh 	; EA cp JMP ptr16:32 Inv. Valid Jump far, absolute, address given in operand
	DD	ConfigureMemory_FAR
	DW	MRC_CODE_SEL
@@:
	; Check ConfigureMemory Exit status.  0 == Success
	cmp	al, 0
	je	NoConfigureMemoryError
	mov	al, 0E1h
	out	080h, al
@@:	;pause
	db	0f3h, 090h
	jmp @b

NoConfigureMemoryError:

	; Back from MRC code, reload flat selectors
	mov	ax, SYS_DATA_SEL
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	add	esp, MRC_LINEAR_BASE
	add	ebp, MRC_LINEAR_BASE

	; Recover any fields from MRC_PARAMETER_FRAME which are needed for future use
	;mov	bl, BYTE PTR ds:[ebp+MRC_PARAMETER_FRAME_OFFSET].MRC_PARAMETER_FRAME.ME_BIOSAction

	;
	; Parse MRC_PARAMETER_FRAME ErrorLog to take action on any noncritical MRC errors
	;
	mov	ebx, DWORD PTR ds:[ebp+MRC_PARAMETER_FRAME_OFFSET].MRC_PARAMETER_FRAME.ErrorLogIndex
	cmp	ebx, 0
	je	NoErrorLogEntries
NextLogEntry:
	mov	eax, DWORD PTR ds:[ebp+4*ebx+MRC_PARAMETER_FRAME_OFFSET].MRC_PARAMETER_FRAME.ErrorLog

	; Handle situation in which ME is missing or in error.  Perhaps set a flag in CMOS for later display
	cmp	eax, MRC_MCH_CONFIGURATION_ERROR
	jnz	@f
	; Handle situation in which error/warning should be displayed later
@@:
	; ....
	dec	ebx
	jnz	NextLogEntry
NoErrorLogEntries:

	CALL_DI	CheckResume
	jz	SkipMrcParamsSave
	; Save MRC_PARAMS_SAVE_RESTORE
	mov	esi, ebp
	call	MrcParamsSave
SkipMrcParamsSave:

; ------------------------------------------------------------------------------------------------------
	; Tear down CPU Cache-as-RAM
	jmp	_CarExit
_returnFromCarExit::

; Reload ss with a selector permitting 16-bit access, in case of future return to 16-bit real mode
	mov	ax, DATA16_SEL
	mov	ss, ax

; We are still in protected mode, so, next we turn off the PE bit.
turnOffPEBit:
	mov	eax, cr0
	and	eax, NOT 00000001h
	mov	cr0, eax

; Finally, we do a FAR transfer of control such that instead of the selector, the 16-bit code segment
; value and shifted segment addressing becomes effective again.
;	jmp     FAR PTR 0f000h:backToBigRealMode
	DB	0EAh
	DW	OFFSET cs:backToBigRealMode
	DW	0F000h

; At this point, we are truly back to BigReal mode. Note that SS, DS, ES, FS, GS are still in BigReal mode,
; with Base = 0x0000, Limit = 4GB.
backToBigRealMode:

	mov	al, 0d4h
	out	080h, al

; Do memory test here...
	mov	si, offset test_base_memory_end
	jmp	test_base_memory		; Test base memory
test_base_memory_end:

Passed::
	mov	al, 0d5h
	out	080h, al

@@:	;pause
	db	0f3h, 090h
	jmp	@b

;----------------------------------------------------------------------------
	PUBLIC	BootGDTtable

; GDT[0]: 0x00: Null entry, never used.
NULL_SEL        EQU     $ - GDT_BASE            ; Selector [0]
GDT_BASE:
BootGDTtable    DD      0
                DD      0
; Linear code segment descriptor
LINEAR_CODE_SEL EQU     $ - GDT_BASE            ; Selector [0x8]
                DW      0FFFFh                  ; limit 0xFFFFF
                DW      0                       ; base 0
                DB      0
                DB      09Bh                    ; present, ring 0, code, execute/read, non-conforming, accessed
                DB      0CFh                    ; page-granular, 32-bit
                DB      0
; System data segment descriptor
SYS_DATA_SEL    EQU     $ - GDT_BASE            ; Selector [0x10]
                DW      0FFFFh                  ; limit 0xFFFFF
                DW      0                       ; base 0
                DB      0
                DB      093h                    ; present, ring 0, data, read/write, expand-up, accessed
                DB      0CFh                    ; page-granular, 32-bit
                DB      0

; F000h code segment descriptor
F000_CODE_SEL   EQU     $ - GDT_BASE            ; Selector [0x18]
                DW      0FFFFh                  ; limit 0xFFFF
                DW      0000h                   ; base 0xF000
                DB      0Fh
                DB      09Bh                    ; present, ring 0, code, execute/read, non-conforming, accessed
                DB      000h                    ; byte-granular, 16-bit
                DB      0

DATA16_SEL      EQU     $ - GDT_BASE            ; Selector [0x10]
                DW      0FFFFh                  ; limit 0xFFFFF
                DW      0                       ; base 0
                DB      0
                DB      093h                    ; present, ring 0, data, read/write, expand-up, accessed
                DB      000h                    ; 16-bit
                DB      0

; MRC Code/Data selectors
MRC_CODE_SEL    EQU     $ - GDT_BASE            ; Selector [0x18]
                DW      0FFFFh                  ; limit 0xFFFF
                DW      (MRC_LINEAR_BASE AND 000FFFFh)
                DB     	(MRC_LINEAR_BASE AND 0FF0000h) SHR 16;
                DB      09Bh                    ; present, ring 0, code, execute/read, non-conforming, accessed
                DB      0CFh                    ; page-granular, 32-bit
                DB      (MRC_LINEAR_BASE AND 0FF000000h) SHR 24; Base[31:24]

MRC_DATA_SEL    EQU     $ - GDT_BASE            ; Selector [0x10]
                DW      0FFFFh                  ; limit 0xFFFFF
                DW      (MRC_LINEAR_BASE AND 000FFFFh)
                DB     	(MRC_LINEAR_BASE AND 0FF0000h) SHR 16;
                DB      093h                    ; present, ring 0, data, read/write, expand-up, accessed
                DB      0CFh                    ; page-granular, 32-bit
                DB      (MRC_LINEAR_BASE AND 0FF000000h) SHR 24; Base[31:24]

GDT_SIZE        EQU     $ - BootGDTtable        ; Size, in bytes

; Global Descriptor Table Descriptor
GDTDescriptor:                                          ; GDT descriptor
                DW      GDT_SIZE - 1                    ; GDT limit
                DW      LOWWORD OFFSET BootGDTtable     ; GDT base address
                DW      0ffffh

_PM32Entry      ENDP

SetOemMrcData PROC
	; Assume ESI is pointed at MRC_PARAMETER_FRAME
	; OEM Customization needed

	mov	WORD PTR ds:[esi].MRC_PARAMETER_FRAME.OemMrcData.MmioSize, 0400h
	mov	WORD PTR ds:[esi].MRC_PARAMETER_FRAME.OemMrcData.TsegSize, 1

	mov	BYTE PTR ds:[esi].MRC_PARAMETER_FRAME.OemMrcData.SPDAddressTable[0], 0A0h
	mov	BYTE PTR ds:[esi].MRC_PARAMETER_FRAME.OemMrcData.SPDAddressTable[1], 0A2h

	mov	BYTE PTR ds:[esi].MRC_PARAMETER_FRAME.OemMrcData.MrcConfigMemProgressCodeBase, 020h


	; Load MRC_PARAMS_SAVE_RESTORE
	mov	esi, ebp
	call	MrcParamsRestore

	ret
SetOemMrcData ENDP

MrcParamsRestore PROC
	; Assume ESI is pointed at MRC_PARAMETER_FRAME
	; OEM Customization needed

	mov	ecx, SIZEOF MRC_PARAMS_SAVE_RESTORE
NextByte:
	;;;;;;;;;;;;;;;;;;;;
	; Restore data to al
	;;;;;;;;;;;;;;;;;;;;
	;mov	BYTE PTR ds:[esi].MRC_PARAMETER_FRAME.OemMrcData, al
	inc	esi
	loop	NextByte

	ret
MrcParamsRestore ENDP

MrcParamsSave PROC
	; Assume ESI is pointed at MRC_PARAMETER_FRAME
	; OEM Customization needed

	mov	ecx, SIZEOF MRC_PARAMS_SAVE_RESTORE
NextByte:
	mov	al, BYTE PTR ds:[esi].MRC_PARAMETER_FRAME.OemMrcData
	;;;;;;;;;;;;;;;;;;;
	; Save data from al
	;;;;;;;;;;;;;;;;;;;
	inc	esi
	loop	NextByte

	ret
MrcParamsSave ENDP

CheckResume PROC NEAR
	; Check ICH or other platform registers to determine whether this boot is
	; a resume from S3 or a simple power-on
	mov	al, 1
	cmp	al, 0
	RET_DI
CheckResume ENDP

test_base_memory PROC NEAR PUBLIC

 	xor	ax, ax
 	mov	es, ax
	mov	eax, 5AA5A55Ah

	xor	edi, edi
	mov	ecx, 4000h * 8		; 512kb memory = 16k*8 dword
	rep	stosd es:[edi]

	xor	edi, edi
	mov	ecx, 4000h * 8		; 512kb memory = 16k*8 dword
repcheckmem:
	cmp	eax, es:[edi]
	jne	test_base_memory_fail
	add	edi, 4
	dec	ecx
	jnz	repcheckmem
	jmp	si

test_base_memory_fail:
	mov	al, 0E5h		; memory check fail
	out	080h, al
	jmp	$			; Stop if fail with EDI location
	jmp	$			; Stop if fail with EDI location

test_base_memory ENDP


_TEXT16 ENDS

ENDIF

END
