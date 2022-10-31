/*************************************************************************
 *
 * Avoton Memory Reference Code
 *
 * ESS - Enterprise Silicon Software
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2006 - 2012 Intel Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel
 * Corporation or its suppliers or licensors. Title to the Material
 * remains with Intel Corporation or its suppliers and licensors.
 * The Material contains trade secrets and proprietary and confidential
 * information of Intel or its suppliers and licensors. The Material
 * is protected by worldwide copyright and trade secret laws and treaty
 * provisions.  No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed, or
 * disclosed in any way without IntelÆs prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly,
 * by implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by Intel in writing.
 ************************************************************************
 *
 *  PURPOSE:
 *
 *      This file contains memory detection and initialization for
 *      FBD2 and DDR3 modules compliant with JEDEC specification.
 *
 ************************************************************************/

#ifndef _UART_H
#define _UART_H

#include "MMrc.h"

#define TAB_STOP        4
#define LEFT_JUSTIFY    0x01
#define PREFIX_SIGN     0x02
#define PREFIX_BLANK    0x04
#ifndef PREFIX_ZERO
#define PREFIX_ZERO     0x08
#endif
#define LONG_TYPE       0x10

#define INT_SIGNED      0x20
#define COMA_TYPE       0x40
#define LONG_LONG_TYPE  0x80

#define CHAR_CR         0x0d
#define CHAR_LF         0x0a

typedef INT8   *va_list;

#define INT32_MAX   0x7fffffffU
#ifndef va_start
typedef INT8   *va_list;
#define _INTSIZEOF(n)   ((sizeof (n) + sizeof (UINT32) - 1) &~(sizeof (UINT32) - 1))
#define va_start(ap, v) (ap = (va_list) & v + _INTSIZEOF (v))
#define va_arg(ap, t)   (*(t *) ((ap += _INTSIZEOF (t)) - _INTSIZEOF (t)))
#define va_end(ap)      (ap = (va_list) 0)
#endif

#define isdigit(_c) ( ((_c) >= '0') && ((_c) <= '9'))

#define PUTCC(_c,_CharCount)   putchar(_c); _CharCount++
int printf (UINT32 PeiServices, UINT32 ErrorLevel, const char *Format, ...);
void PrintSize (UINT16 size);
int vprintf (UINT32 ErrorLevel, const char *Format, va_list Marker);

void UintnToStr (UINT32 Value, char *Str, UINT32 Width, UINT32 Flags, UINT32 Base);
UINT32 StrToNumber (char **String);

char putchar (char c);
char getchar (void);

#define REG_LOGICAL_DEVICE          0x07
#define BASE_ADDRESS_HIGH           0x60
#define BASE_ADDRESS_LOW            0x61
#define ACTIVATE                    0x30

#define RECEIVER_BUFFER             0x00
#define TRANSMIT_HOLDING            0x00
#define DIVISOR_LATCH_LOW           0x00

#define DIVISOR_LATCH_HIGH          0x01
#define INTERRUPT_ENABLE            0x01
#define DISABLE_INTERRUPTS          0x00

#define INTERRUPT_IDENTIFICATION    0x02
#define FIFO_CONTROL                0x02
#define FIFO_DISABLE_MASK           0xfe

#define LINE_CONTROL                0x03
#define DIVISOR_LATCH_ACCESS        0x80
#define NO_STOP                     0x00
#define NO_PARITY                   0x00
#define BITS8                       0x03

#define MODEM_CONTROL               0x04
#define LINE_STATUS                 0x05
#define TRANS_HOLDING_REG_EMPTY     0x20
#define DATA_READY                  0x01

#define MODEM_STATUS                0x06
#define SCRATCH                     0x07

#define DATA_BITS           3

#define STOP_BIT            1
#define PARITY_BITS         0
#define BREAK_BIT           0
#define DLAB_BIT            1
// 1 - Divisor Latch Enabled

// Base Address Register 0
#define PCI_BAR0                0x0010
#define PCI_CMD                 0x04
//
// Uart COM1 IO Space
//
#define COM1_BASE_ADDR      0x03F8

#define SCR_OFFSET          7

#endif //ifndef _UART_H
