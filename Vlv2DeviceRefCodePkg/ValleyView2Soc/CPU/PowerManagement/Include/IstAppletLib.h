/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  IstAppletLib.h

Abstract:

  IST applet support.  This is needed for IST to function on Win2K.

  Refer to the Geyserville III Technology BIOS Porting Guide Revision 1.0
 for details on the IST applet interface definitions.

  Acronyms:
    PPM   Processor Power Management
    GV    Geyserville
    TM    Thermal Monitor
    IST   Intel(R) Speedstep technology
    HT    Hyper-Threading Technology

--*/

#ifndef _IST_APPLET_LIB_H_
#define _IST_APPLET_LIB_H_

//
// Statements that include other files
//
#include "Tiano.h"

//
// This is not a standard protocol, as it is never published.
// It is more of a dynamic internal library.
//
#include EFI_PROTOCOL_DEPENDENCY (PpmProcessorSupport2)

//
// IST Interface Commands:
//
// The following equates define the SMM interface commands for IST. The OS
// commands have no relation to the Applet (APP) command value.  The OS
// command values may be modified; however, the OS command value must match
// what is reported in the Fixed ACPI Description Table for ACPI 2.0 which
// adds an SMI command for enabling processor performance control. The APP
// command value must match the command value that setup for the applet and
// recorded in the registry.
//
#define OS_COMMAND        0x80  // OS indicates IST control.
#define INIT_COMMAND      0x81  // BIOS indicates IST init.
#define APP_COMMAND       0x82  // Applet indicates IST control.
#define OS_REQUEST        0x83  // OS transition request.
#define C_STATE_COMMAND   0x85  // OS indicates C state control.

//
// Function prototypes
//
EFI_STATUS
InitializeIstAppletLib (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  );
/*++

Routine Description:

  Initialize the IST Applet support.  This will install SMI handlers, etc.

Arguments:

  This            Pointer to the protocol instance

Returns:

  EFI_STATUS      The library was initialized successfully

--*/

#endif
