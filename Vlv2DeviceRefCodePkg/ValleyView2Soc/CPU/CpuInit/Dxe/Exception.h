/*++

Copyright (c)  1999 - 2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

    Exception.h

Abstract:

    IA32 Exception Includes.



--*/

#ifndef _IA32EXCEPTION_H
#define _IA32EXCEPTION_H

//
// Driver Consumed Protocol Prototypes
//
#ifdef ECP_FLAG
#include <ArchProtocol/Cpu/Cpu.h>
#else
#include <Protocol/Cpu.h>

#define EFI_STATUS_CODE_DATA_TYPE_EXCEPTION_HANDLER_GUID \
  { \
    0x3BC2BD12, 0xAD2E, 0x11D5, {0x87, 0xDD, 0x00, 0x06, 0x29, 0x45, 0xC3, 0xB9} \
  }
#endif



#define INTERRUPT_HANDLER_DIVIDE_ZERO           0x00
#define INTERRUPT_HANDLER_DEBUG                 0x01
#define INTERRUPT_HANDLER_NMI                   0x02
#define INTERRUPT_HANDLER_BREAKPOINT            0x03
#define INTERRUPT_HANDLER_OVERFLOW              0x04
#define INTERRUPT_HANDLER_BOUND                 0x05
#define INTERRUPT_HANDLER_INVALID_OPCODE        0x06
#define INTERRUPT_HANDLER_DEVICE_NOT_AVAILABLE  0x07
#define INTERRUPT_HANDLER_DOUBLE_FAULT          0x08
#define INTERRUPT_HANDLER_COPROCESSOR_OVERRUN   0x09
#define INTERRUPT_HANDLER_INVALID_TSS           0x0A
#define INTERRUPT_HANDLER_SEGMENT_NOT_PRESENT   0x0B
#define INTERRUPT_HANDLER_STACK_SEGMENT_FAULT   0x0C
#define INTERRUPT_HANDLER_GP_FAULT              0x0D
#define INTERRUPT_HANDLER_PAGE_FAULT            0x0E
#define INTERRUPT_HANDLER_RESERVED              0x0F
#define INTERRUPT_HANDLER_MATH_FAULT            0x10
#define INTERRUPT_HANDLER_ALIGNMENT_FAULT       0x11
#define INTERRUPT_HANDLER_MACHINE_CHECK         0x12
#define INTERRUPT_HANDLER_STREAMING_SIMD        0x13

typedef struct {
  EFI_STATUS_CODE_DATA  Header;
  union {
    EFI_SYSTEM_CONTEXT_IA32 SystemContextIa32;
    EFI_SYSTEM_CONTEXT_X64  SystemContextX64;
  } SystemContext;
} CPU_STATUS_CODE_TEMPLATE;

VOID
CommonExceptionHandler (
  IN EFI_EXCEPTION_TYPE   InterruptType,
  IN EFI_SYSTEM_CONTEXT   SystemContext
  );

EFI_STATUS
InitializeException (
  IN  EFI_CPU_ARCH_PROTOCOL *CpuProtocol
  );
#endif
