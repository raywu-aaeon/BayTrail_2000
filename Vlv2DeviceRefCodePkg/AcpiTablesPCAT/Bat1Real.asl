/**@file

@copyright
 Copyright (c) 1999 - 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.

This file contains an 'Intel Peripheral Driver' and is uniquely
 identified as "Intel Reference Module" and is licensed for Intel
 CPUs and chipsets under the terms of your license agreement with
 Intel or your vendor. This file may be modified by the user, subject
 to additional terms of the license agreement.
**/
/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Baytrail            *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved   *;
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

/* Already defined in GloblNvs.asl
Scope(\)
{
  // these fields come from the Global NVS area
  Field(GNVS,AnyAcc,Lock,Preserve)
  {
    Offset(30), // Battery Support Registers:
    BNUM, 8,    //   (30) Battery Number Present
    Offset(32),
    B1SC, 8,    //   (32) Battery 1 Stored Capacity
    Offset(35),
    B1SS, 8,    //   (35) Battery 1 Stored Status
  }
}
*/
// Define the Real Battery 1 Control Method.

Device(BAT1)
{
  Name(_HID,EISAID("PNP0C0A"))

  Name(_UID,1)

  Method(_STA,0)
  {
    If(And(BNUM,1))         // Battery 1 present?
    {
      Return(0x001F)  // Yes.  Show it.
    }

    Return(0x000B)    // No.  Hide it.
  }

  Method(_BIF,0,Serialized)
  {
    Name(BPKG, Package()
    {
      0x00000000, // Power Unit = mWh.
      0x00002710,     // 10000 mWh Design Capacity.
      0x00002710,     // 10000 mWh Last Full Charge.
      0x00000001, // Secondary Battery Technology.
      0xFFFFFFFF, // Unknown Design Voltage.
      0x000003E8, // 10% Warning Level.
      0x00000190, // 4% Low Level.
      0x00000064, // 1% Granularity Level 1.
      0x00000064, // 1% Granularity Level 2.
      "CRB Battery 1",
      "Battery 1",
      "Real",
      "-Real Battery 1-"
    })

    // Fix up the design capacity
    If (ECON)
    {
      Store(\_SB.PCI0.LPCB.H_EC.B1DC, Local1)
      Store(Multiply(Local1,10), Index(BPKG,1))

      // Fix up the full charge capacity
      //EC will return zero B1FC in some conditions, BIOS need to make sure it's non-zero value.      
      Store(10, Local2)
      While(Local2)
      {        
        Store(\_SB.PCI0.LPCB.H_EC.B1FC, Local0)
        If(LEqual(Local0, Zero))
        {
          Sleep(200)
          Decrement(Local2)
        } else
        {
          Store(0, Local2)
        }
      }
      
      Store(Multiply(Local0,10) , Index(BPKG,2))

      // Fix up the design voltage
      Store(\_SB.PCI0.LPCB.H_EC.B1FV, Index(BPKG,4))
      
      If(Local0)
      {
        Store(Divide(Multiply(Local0, 10),10), Index(BPKG,5))
        Store(Divide(Multiply(Local0, 10),25), Index(BPKG,6))
        Store(Divide(Multiply(Local1, 10),100), Index(BPKG,7))
      }
    }

    Return (BPKG)
  }

  Method(_BIX,0, Serialized)
  {
    Name(BIXP, Package()      //Data Package for _BIX
    {
      0x00,       // Integer Revision
      0x00000000, // Power Unit 0 = mWh, 1=mAh.
      0xFFFFFFFF, // Unknown Design Capacity.
      0xFFFFFFFF, // Unknown Last Full Charge.
      0x00000001, // Battery Technology.(0x00 - Primary(Non-Rechargeable), 0x001 - Secondary(Rechargeable))
      0xFFFFFFFF, // Unknown Design Voltage.
      0x00000000, // 10% Warning Level.
      0x00000000, // 4% Low Level.
      0xFFFFFFFF, // Cycle Count, MUST not be 0xFFFFFFFF
      0xFFFFFFFF, // Measurement Accuracy 95%
      0xFFFFFFFF, // Max sampling time, MSFT No specific requirement.
      0xFFFFFFFF, // Min sampling time, MSFT No specific requirement.
      0x00000000, // Max averaging interval, MSFT No specific requirement.
      0x00000000, // Min averaging interval, MSFT No specific requirement.
      0x00000001, // Battery capacity granularity 1
      0x00000001, // Battery capacity granularity 2
      "Harris Beach", // Model number         //String (ASCIIZ)
      "123456789",    //Serial Number         //String (ASCIIZ)
      "LION",                 // Battery type         //String (ASCIIZ)
      "Intel SR 1"    // OEM information      //String (ASCIIZ)
    })
    Store(\_SB.PCI0.LPCB.H_EC.B1DC, Local0)
    Store(Multiply(Local0,10), Index(BIXP,2))

    // Fix up the full charge capacity
    Store(\_SB.PCI0.LPCB.H_EC.B1FC, Local1)    
    Store(Multiply(Local1,10) , Index(BIXP,3))

    // Fix up the design voltage
    Store(\_SB.PCI0.LPCB.H_EC.B1FV, Local2)    
    Store(Local2, Index(BIXP,5))


    If(\_SB.PCI0.LPCB.H_EC.B1FC)
    {
      Store(Divide(Multiply(Local1, 10),10), Index(BIXP,6))
      Store(Divide(Multiply(Local1, 10),25), Index(BIXP,7))
      Store(Divide(Multiply(Local0, 10),100), Index(BIXP,8))
      //        Store(0x100,  Index(BIXP,8))
      Store(0x40,   Index(BIXP,9))
      Store(0x320,  Index(BIXP,10))
      Store(0x251C, Index(BIXP,11))
    }

    Return (BIXP)
  }

  Method(_BST,0,Serialized)
  {
    Name(PKG1,Package()
    {
      0xFFFFFFFF, // Battery State.
      0xFFFFFFFF, // Battery Present Rate. (in mWh)
      0xFFFFFFFF, // Battery Remaining Capacity. (in mWh)
      0xFFFFFFFF  // Battery Present Voltage. (in mV)
    })

    // Fix up the Battery Status.
    Store(B1ST, Local0)
    Store(And(Local0, 0x07),Index(PKG1,0))
    If(And(Local0, 0x01))
    {
      // Calculate discharge rate
      // Return Rate in mW since we report _BIF data in mW
      Store(Multiply(B1DI, B1FV), Local0)
      Store(Divide(Local0, 1000), Local0)
      Store(Local0, Index(PKG1,1))
    }
    Else
    {
      // Check Battery State
      If(And(Local0, 0x02))
      {
        // Calculate charge rate
        // Return Rate in mW since we report _BIF data in mW
        Store(Multiply(B1CI, B1FV), Local0)
        Store(Divide(Local0, 1000), Local0)
        Store(Local0, Index(PKG1,1))
      }
    }
    
    //Full Battery Capacity in mWh
    //EC will return zero B1FC in some conditions, BIOS need to make sure it's non-zero value.
    //Otherwise Remaining Capacity will be calculated as Zero.
    Store(10, Local1)
    While(Local1)
    {
      Store(Multiply(\_SB.PCI0.LPCB.H_EC.B1FC, 10), Local0)
      If(LEqual(Local0, Zero))
      {
        Sleep(200)
        Decrement(Local1)
      } else
      {
        Store(0, Local1)
      }
    }
 
    // Calculate Current Capacity in mWh =
    // Full Charge Capacity  * Current Capacity (%) /100
    Store(Divide(Multiply(\_SB.PCI0.LPCB.H_EC.B1CC, Local0), 100), Index(PKG1,2))
    
    // Report Battery Present Voltage (mV)
    Store(B1FV, Index(PKG1,3))

    Return(PKG1)
  }

  Method(_BTP,1)
  {
    // arg0 = Trip Point, sent to EC as Threshold.
    // transfer input value from mWh to %
    If(LAnd(LNotEqual(B1FC,0),LNotEqual(B1FV,0)))
    {
        Store (Divide (Arg0, 10), Local0)
	Store (And (ShiftRight (Local0, 0), 0x00FF), BTPL)
	Store (And (ShiftRight (Local0, 8), 0x00FF), BTPH)
    }

    Return()
  }

  // Return that everything runs off Battery.

  Method(_PCL,0)
  {
    Return(\_SB)
  }

  Method(BTMP,0,Serialized)
  {
    Name(TPKG, Package()
    {
      0x00000000 // Battery Temp = K.
    })

    // Fix up the temp 
    If (ECON)
    {
      // BATL, and BATH - units 0.1K
      ShiftLeft(\_SB.PCI0.LPCB.H_EC.BATH,8, Local0)
      Or(\_SB.PCI0.LPCB.H_EC.BATL, Local0, Local0)
      Store(Divide(Local0,10), Index(TPKG,0))
    }

    Return (TPKG)
  }

}

