/*************************************************************************
 *
 * Reference Code
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
#include "../Include/Printf.h"

#ifdef MINIBIOS_BUILD
#include "Uart.h"
#else
#ifdef ECP_FLAG
#include "EdkIIGluePeim.h"
#include <Guid/StatusCodeDataTypeId/StatusCodeDataTypeId.h>

typedef UINTN  *BASE_LIST;
#define _BASE_INT_SIZE_OF(TYPE) ((sizeof (TYPE) + sizeof (UINTN) - 1) / sizeof (UINTN))
#define BASE_ARG(Marker, TYPE)   (*(TYPE *) ((Marker += _BASE_INT_SIZE_OF (TYPE)) - _BASE_INT_SIZE_OF (TYPE)))
#else
#include "PiPei.h"
#include <Library/DebugLib.h>
#include <Guid/StatusCodeDataTypeId.h>
#include <Guid/StatusCodeDataTypeDebug.h>
#include <Library/ReportStatusCodeLib.h>
#endif
#endif

#define EFI_STANDARD_CALLER_ID_GUID \
  {0xC9DCF469, 0xA7C4, 0x11D5, 0x87, 0xDA, 0x00, 0x06, 0x29, 0x45, 0xC3, 0xB9}

UINT16
rcVprintf (
  MMRC_DATA    *ModMrcData,
  CONST INT8 *Format,
  va_list    Marker
)
/*++

Routine Description:

    rcPrintf with stdargs varargs stack frame in place of .... Limited
    support for sizes other than UINT32 to save code space

Arguments:
    Format - String containing characters to print and formating Data.
    Marker - va_list that points to the arguments for Format that are on
                the stack.
Returns:

--*/
{
  return 0;
}

UINT16
rcPrintf (
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
{

  if ( (MrcDebugMsgLevel & MsgLevel) != 0) {
    {
    	  UINT64          Buffer[(EFI_STATUS_CODE_DATA_MAX_SIZE / sizeof (UINT64)) + 1];
    	  EFI_DEBUG_INFO  *DebugInfo;
    	  UINTN           TotalSize;
    	  VA_LIST         VaListMarker;
    	  BASE_LIST       BaseListMarker;
    	  CHAR8           *FormatString;
    	  BOOLEAN         Long;
    	  //
    	  // Compute the total size of the record.
    	  // Note that the passing-in format string and variable parameters will be constructed to
    	  // the following layout:
    	  //
    	  //         Buffer->|------------------------|
    	  //                 |         Padding        | 4 bytes
    	  //      DebugInfo->|------------------------|
    	  //                 |      EFI_DEBUG_INFO    | sizeof(EFI_DEBUG_INFO)
    	  // BaseListMarker->|------------------------|
    	  //                 |           ...          |
    	  //                 |   variable arguments   | 12 * sizeof (UINT64)
    	  //                 |           ...          |
    	  //                 |------------------------|
    	  //                 |       Format String    |
    	  //                 |------------------------|<- (UINT8 *)Buffer + sizeof(Buffer)
    	  //
    	  TotalSize = 4 + sizeof (EFI_DEBUG_INFO) + 12 * sizeof (UINT64) + AsciiStrSize (Format);

    	  //
    	  // If the TotalSize is larger than the maximum record size, then return
    	  //
    	  if (TotalSize > sizeof (Buffer)) {
    	    return 0xFF;
    	  }

    	  //
    	  // Fill in EFI_DEBUG_INFO
    	  //
    	  // Here we skip the first 4 bytes of Buffer, because we must ensure BaseListMarker is
    	  // 64-bit aligned, otherwise retrieving 64-bit parameter from BaseListMarker will cause
    	  // exception on IPF. Buffer starts at 64-bit aligned address, so skipping 4 types (sizeof(EFI_DEBUG_INFO))
    	  // just makes address of BaseListMarker, which follows DebugInfo, 64-bit aligned.
    	  //
    	  DebugInfo             = (EFI_DEBUG_INFO *)(Buffer) + 1;
    	  DebugInfo->ErrorLevel = EFI_D_INFO;
    	  BaseListMarker        = (BASE_LIST)(DebugInfo + 1);
    	  FormatString          = (CHAR8 *)((UINT64 *)(DebugInfo + 1) + 12);

    	  //
    	  // Copy the Format string into the record
    	  //
    	  AsciiStrCpy (FormatString, Format);

    	  //
    	  // The first 12 * sizeof (UINT64) bytes following EFI_DEBUG_INFO are for variable arguments
    	  // of format in DEBUG string, which is followed by the DEBUG format string.
    	  // Here we will process the variable arguments and pack them in this area.
    	  //
    	  VA_START (VaListMarker, Format);
    	  for (; *Format != '\0'; Format++) {
    	    //
    	    // Only format with prefix % is processed.
    	    //
    	    if (*Format != '%') {
    	      continue;
    	    }
    	    Long = FALSE;
    	    //
    	    // Parse Flags and Width
    	    //
    	    for (Format++; TRUE; Format++) {
    	      if (*Format == '.' || *Format == '-' || *Format == '+' || *Format == ' ') {
    	        //
    	        // These characters in format field are omitted.
    	        //
    	        continue;
    	      }
    	      if (*Format >= '0' && *Format <= '9') {
    	        //
    	        // These characters in format field are omitted.
    	        //
    	        continue;
    	      }
    	      if (*Format == 'L' || *Format == 'l') {
    	        //
    	        // 'L" or "l" in format field means the number being printed is a UINT64
    	        //
    	        Long = TRUE;
    	        continue;
    	      }
    	      if (*Format == '*') {
    	        //
    	        // '*' in format field means the precision of the field is specified by
    	        // a UINTN argument in the argument list.
    	        //
    	        BASE_ARG (BaseListMarker, UINTN) = VA_ARG (VaListMarker, UINTN);
    	        continue;
    	      }
    	      if (*Format == '\0') {
    	        //
    	        // Make no output if Format string terminates unexpectedly when
    	        // looking up for flag, width, precision and type.
    	        //
    	        Format--;
    	      }
    	      //
    	      // When valid argument type detected or format string terminates unexpectedly,
    	      // the inner loop is done.
    	      //
    	      break;
    	    }

    	    //
    	    // Pack variable arguments into the storage area following EFI_DEBUG_INFO.
    	    //
    	    if ((*Format == 'p') && (sizeof (VOID *) > 4)) {
    	      Long = TRUE;
    	    }
    	    if (*Format == 'p' || *Format == 'X' || *Format == 'x' || *Format == 'd') {
    	      if (Long) {
    	        BASE_ARG (BaseListMarker, INT64) = VA_ARG (VaListMarker, INT64);
    	      } else {
    	        BASE_ARG (BaseListMarker, int) = VA_ARG (VaListMarker, int);
    	      }
    	    } else if (*Format == 's' || *Format == 'S' || *Format == 'a' || *Format == 'g' || *Format == 't') {
    	      BASE_ARG (BaseListMarker, VOID *) = VA_ARG (VaListMarker, VOID *);
    	    } else if (*Format == 'c') {
    	      BASE_ARG (BaseListMarker, UINTN) = VA_ARG (VaListMarker, UINTN);
    	    } else if (*Format == 'r') {
    	      BASE_ARG (BaseListMarker, RETURN_STATUS) = VA_ARG (VaListMarker, RETURN_STATUS);
    	    }

    	    //
    	    // If the converted BASE_LIST is larger than the 12 * sizeof (UINT64) allocated bytes, then ASSERT()
    	    // This indicates that the DEBUG() macro is passing in more argument than can be handled by
    	    // the EFI_DEBUG_INFO record
    	    //
    	    //ASSERT ((CHAR8 *)BaseListMarker <= FormatString);

    	    //
    	    // If the converted BASE_LIST is larger than the 12 * sizeof (UINT64) allocated bytes, then return
    	    //
    	    if ((CHAR8 *)BaseListMarker > FormatString) {
    	    	return 0xFF;
    	    }
    	  }
    	  VA_END (VaListMarker);

    	  //
    	  // Send the DebugInfo record
    	  //
    	  REPORT_STATUS_CODE_EX (
    	    EFI_DEBUG_CODE,
    	    (EFI_SOFTWARE_DXE_BS_DRIVER | EFI_DC_UNSPECIFIED),
    	    0,
    	    NULL,
    	    &gEfiStatusCodeDataTypeDebugGuid,
    	    DebugInfo,
    	    TotalSize
    	    );
    } // Scope End
  }
  return SUCCESS;
}

