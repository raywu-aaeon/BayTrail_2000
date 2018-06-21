/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Valleyview          *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved   *;
;*    This software and associated documentation (if any) is furnished    *;
;*    under a license and may only be used or copied in accordance        *;
;*    with the terms of the license. Except as permitted by such          *;
;*    license, no part of this software or documentation may be           *;
;*    reproduced, stored in a retrieval system, or transmitted in any     *;
;*    form or by any means without the express written consent of         *;
;*    Intel Corporation.                                                  *;
;*                                                                        *;
;*                                                                        *;
;**************************************************************************/
/*++
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
--*/



// NOTE:  The _PDC Implementation is out of the scope of this
// reference code.  Please see the latest Hyper-Threading Technology
// Reference Code for complete implementation details.

Scope(\_PR)
{
  Processor(CPU0,         // Unique name for Processor 0.
            1,                        // Unique ID for Processor 0.
            0x00,                 // CPU0 ACPI P_BLK address = ACPIBASE + 10h.
            0)                        // CPU0 ICH7M P_BLK length = 6 bytes.
  {}

  Processor(CPU1,         // Unique name for Processor 1.
            2,                        // Unique ID for Processor 1.
            0x00,
            0)                    // CPU1 P_BLK length = 6 bytes.
  {}

  Processor(CPU2,         // Unique name for Processor 2.
            3,                        // Unique ID for Processor 2.
            0x00,
            0)                    // CPU2 P_BLK length = 6 bytes.
  {}

  Processor(CPU3,         // Unique name for Processor 3.
            4,                        // Unique ID for Processor 3.
            0x00,
            0)                    // CPU3 P_BLK length = 6 bytes.
  {}
}     // End _PR


