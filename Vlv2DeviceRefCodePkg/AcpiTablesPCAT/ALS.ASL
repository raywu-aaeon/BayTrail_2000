/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Cedartrail         *;
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



Device(ALSD)
{
  Name(_HID,"ACPI0008")
  Name(_CID,EISAID("PNP0C01"))

  Method(_STA,0)
  {
    If(LEqual(ALSE,2))
    {
      Return(0x000F)          // ALS Enabled.  Show it.
    }

    Return(0x0000)                  // ALS Disabled.  Hide it.
  }

  Method(_ALI)
  {
    Return (Or(ShiftLeft(LHIH,8),LLOW))
  }

// TODO
// Method(_ALT)
// {
// Return(3000)
// }

// TODO - These are "best guess" values.

  Name(_ALR, Package()
  {
    Package() {70,  0},
    Package() {73,  10},
    Package() {85,  80},
    Package() {100, 300},
    Package() {150, 1000}
  })

// TODO - CRB designs don't require this as the KSC performas the polling
// and notifies ASL via Q70 when a change occurs.
//
// Method(_ALP)
// {
// }

}
