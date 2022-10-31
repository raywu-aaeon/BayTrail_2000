/*************************************************************************
 *
 * Memory Reference Code
 *
 * ESS - Enterprise Silicon Software
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2006 - 2011 Intel Corporation All Rights Reserved.
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
 * disclosed in any way without Intel's prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you
 * by disclosure or delivery of the Materials, either expressly,
 * by implication, inducement, estoppel or otherwise. Any license
 * under such intellectual property rights must be express and
 * approved by Intel in writing.
 *
 ************************************************************************/

#ifndef _printf_h
#define _printf_h
#include "Types.h"
#ifndef MINIBIOS_BUILD
#ifdef ECP_FLAG
#include "Tiano.h"
#else
#include "PiPei.h"
#endif
#endif

#include "../../IpBlocks/VLVA0/Include/MMRCProjectData.h"

// No MRC message print
#define SDBG_NONE       0

// Show Rank Margin Tool, Platform Info, Training Result, MRC Flow, PHY View Table, MCU Timing Table
#define SDBG_MIN        BIT0

// Show JEDEC MRS, 1D eyes & 2D eyes diagram for RD/WR training useful for EV.
#define SDBG_MED        BIT1

// Print all register writes to DUNIT and DDRIOPHY, Training flow with all intermediate detail steps
#define SDBG_MAX        BIT2

// Show all register accesses.
#define SDBG_TRACE      BIT3

// Show any test messages - these are ones MRC developers can use for debug but don't typically want displayed in the release BIOS.
#define SDBG_TST        BIT4

// Use for temporary debug of issues. Do not release any MRC code with this print level in use.
#define SDBG_DEBUG      BIT5

// Show Rank Margin Tool, Platform Info, Training Result, MRC Flow, PHY View Table, MCU Timing Table
// Show JEDEC MRS, 1D eyes & 2D eyes diagram for RD/WR training useful for EV.
// Default mode
#define SDBG_TYPICAL    SDBG_MIN + SDBG_MED

// Show Rank Margin Tool, Platform Info, Training Result, MRC Flow, PHY View Table, MCU Timing Table
// Show JEDEC MRS, 1D eyes & 2D eyes diagram for RD/WR training useful for EV.
// Print all register writes to DUNIT and DDRIOPHY, Training flow with all intermediate detail steps
#define SDBG_VERBOSE    SDBG_MIN + SDBG_MED + SDBG_MAX

// Show Rank Margin Tool, Platform Info, Training Result, MRC Flow, PHY View Table, MCU Timing Table
// Show JEDEC MRS, 1D eyes & 2D eyes diagram for RD/WR training useful for EV.
// Print all register writes to DUNIT and DDRIOPHY, Training flow with all intermediate detail steps
// Show all register accesses.
#define SDBG_FULL      	SDBG_MIN + SDBG_MAX + SDBG_MED + SDBG_TRACE


#ifndef ASM_INC
#define TAB_STOP            4
#define LEFT_JUSTIFY        0x01
#define PREFIX_SIGN         0x02
#define PREFIX_BLANK        0x04
#define COMMON_PREFIX_ZERO  0x08
#define LONG_TYPE           0x10

#define INT_SIGNED          0x20
#define COMA_TYPE           0x40
#define LONG_LONG_TYPE      0x80

#define CHAR_CR             0x0d
#define CHAR_LF             0x0a


#ifndef INT32_MAX
#define INT32_MAX             0x7fffffffU
#endif

#ifndef va_start
typedef INT8   *va_list;
#define _INTSIZEOF(n)   ((sizeof (n) + sizeof (UINT32) - 1) &~(sizeof (UINT32) - 1))
#define va_start(ap, v) (ap = (va_list) & v + _INTSIZEOF (v))
#define va_arg(ap, t)   (*(t *) ((ap += _INTSIZEOF (t)) - _INTSIZEOF (t)))
#define va_end(ap)      (ap = (va_list) 0)
#endif

//#define isdigit(_c)     (((_c) >= '0') && ((_c) <= '9'))
#define ishexdigit(_c)  (((_c) >= 'a') && ((_c) <= 'f'))
#endif // ASM_INC

UINT16
rcPrintf (
//  IN EFI_PEI_SERVICES   **PeiServices,
  UINT8        MrcDebugMsgLevel,
  UINT8        MsgLevel,
  CONST INT8  *Format,
  ...
)
/*++

Routine Description:

  Prints string to serial output

Arguments:

  MrcDebugMsgLevel  - Message level at which the current MRC run is operating
  MsgLevel          - The message level for this particular rcPrintf call
  Format            - Format string for output

Returns:

    0 - Success
    1 - Failure

--*/
;

#endif // _printf_h
