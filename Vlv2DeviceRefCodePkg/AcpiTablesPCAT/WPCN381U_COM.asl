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


Device(UAR5)    // Serial Port UART 5
{
  Name(_HID, EISAID("PNP0501"))

  Name(_UID,3)

  // Status Method for UART 5.

  Method(_STA,0,Serialized)
  {
    // Only report resources to the OS if the SIO Device is
    // not set to Disabled in BIOS Setup.

    If(W381)
    {
      // Set the SIO to UART 5.

      Store(3,WR07)

      // Display UART 5 and return status.

      If(WR30)
      {
        Return(0x000F)
      }

      Return(0x000D)
    }

    Return(0x0000)
  }

  // Disable Method for UART 5.

  Method(_DIS,0,Serialized)
  {
    Store(3,WR07)
    Store(0,WR30)
  }

  // Current Resource Setting Method for UART 5.

  Method(_CRS,0,Serialized)
  {
    // Create the Buffer that stores the Resources to
    // be returned.

    Name(BUF0,ResourceTemplate()
    {
      IO(Decode16,0x03F8,0x03F8,0x01,0x08)
      IRQNoFlags() {4}
    })

    // Set SIO to UART 5.

    Store(3,WR07)

    // Skip this sequence if the UART 5 Port is Disabled
    // in BIOS Setup.

    If(W381)
    {
      // Create pointers to the specific byte.

      CreateByteField(BUF0,0x02,IOL0)
      CreateByteField(BUF0,0x03,IOH0)
      CreateByteField(BUF0,0x04,IOL1)
      CreateByteField(BUF0,0x05,IOH1)
      CreateByteField(BUF0,0x07,LEN0)
      CreateWordField(BUF0,0x09,IRQW)

      // Write IO and Length values into the Buffer.

      Store(WR60,IOH0)
      Store(WR61,IOL0)
      Store(WR60,IOH1)
      Store(WR61,IOL1)
      Store(8,LEN0)

      // Write the IRQ value into the Buffer.

      And(WR70,0x0F,Local0)
      If(Local0)
      {
        ShiftLeft(One,Local0,IRQW)
      }
      Else
      {
        Store(Zero,IRQW)
      }
    }

    Return(BUF0)
  }

  // Possible Resource Setting Method for UART 5.

  // Build a Buffer with all valid COM Port Resources.

  Name(_PRS,ResourceTemplate()
  {
    StartDependentFn(0,2)
    {
      IO(Decode16,0x3F8,0x3F8,0x01,0x08)
      IRQNoFlags() {4}
    }

    StartDependentFn(0,2)
    {
      IO(Decode16,0x3F8,0x3F8,0x01,0x08)
      IRQNoFlags() {4,5,6,7,10,11,12}
    }

    StartDependentFn(0,2)
    {
      IO(Decode16,0x2F8,0x2F8,0x01,0x08)
      IRQNoFlags() {4,5,6,7,10,11,12}
    }

    StartDependentFn(0,2)
    {
      IO(Decode16,0x3E8,0x3E8,0x01,0x08)
      IRQNoFlags() {4,5,6,7,10,11,12}
    }

    StartDependentFn(0,2)
    {
      IO(Decode16,0x2E8,0x2E8,0x01,0x08)
      IRQNoFlags() {4,5,6,7,10,11,12}
    }

    EndDependentFn()
  })


  // Set Resource Setting Method for UART 5.

  Method(_SRS,1,Serialized)
  {
    // Point to the specific information in the passed
    // in Buffer.

    CreateByteField(Arg0,0x02,IOLO)
    CreateByteField(Arg0,0x03,IOHI)
    CreateWordField(Arg0,0x09,IRQW)

    // Set the SIO to UART 5.

    Store(3,WR07)

    // Disable the device.

    Store(0,WR30)

    // Set the Base IO Address.

    Store(IOLO,WR61)
    Store(IOHI,WR60)

    // Set the IRQ.

    FindSetRightBit(IRQW,Local0)
    If(LNotEqual(IRQW,Zero))
    {
      Decrement(Local0)
    }
    Store(Local0,WR70)

    // Enable the device.

    Store(1,WR30)
  }

  // GPE 09, S4
  Name(_PRW, Package() {0x19,4})
  Method(_PSW,1)
  {
    Store(WR70,Local0)

    If (Arg0)
    {
      // Enable as wake event, enable bit4
      Or(Local0,0x10,WR70)
    }
    Else
    {
      // Disable as wake event, disable bit4
      AND(Local0,0x0F,WR70)
    }
  }

  // D0 Method for COM Port.

  Method(_PS0,0,Serialized)
  {
    Store(3,WR07)
    Store(1,WR30)
  }

  // D3 Method for COM Port.

  Method(_PS3,0,Serialized)
  {
    Store(3,WR07)
    Store(0,WR30)
  }
}

Device(UAR8)    // Serial Port UART 8
{
  Name(_HID, EISAID("PNP0501"))

  Name(_UID,4)

  // Status Method for UART 8.

  Method(_STA,0,Serialized)
  {
    // Only report resources to the OS if the SIO Device is
    // not set to Disabled in BIOS Setup.

    If(W381)
    {
      // Set the SIO to UART 8.

      Store(2,WR07)

      // Display UART 8 and return status.

      If(WR30)
      {
        Return(0x000F)
      }

      Return(0x000D)
    }

    Return(0x0000)
  }

  // Disable Method for UART 8.

  Method(_DIS,0,Serialized)
  {
    Store(2,WR07)
    Store(0,WR30)
  }

  // Current Resource Setting Method for UART 8.

  Method(_CRS,0,Serialized)
  {
    // Create the Buffer that stores the Resources to
    // be returned.

    Name(BUF0,ResourceTemplate()
    {
      IO(Decode16,0x02F8,0x02F8,0x01,0x08)
      IRQNoFlags() {3}
    })

    // Set SIO to UART 8.

    Store(2,WR07)

    // Skip this sequence if the UART 8 Port is Disabled
    // in BIOS Setup.

    If(W381)
    {
      // Create pointers to the specific byte.

      CreateByteField(BUF0,0x02,IOL0)
      CreateByteField(BUF0,0x03,IOH0)
      CreateByteField(BUF0,0x04,IOL1)
      CreateByteField(BUF0,0x05,IOH1)
      CreateByteField(BUF0,0x07,LEN0)
      CreateWordField(BUF0,0x09,IRQW)

      // Write IO and Length values into the Buffer.

      Store(WR60,IOH0)
      Store(WR61,IOL0)
      Store(WR60,IOH1)
      Store(WR61,IOL1)
      Store(8,LEN0)

      // Write the IRQ value into the Buffer.

      And(WR70,0x0F,Local0)
      If(Local0)
      {
        ShiftLeft(One,Local0,IRQW)
      }
      Else
      {
        Store(Zero,IRQW)
      }
    }

    Return(BUF0)
  }

  // Possible Resource Setting Method for UART 8.

  // Build a Buffer with all valid UART 8 Port Resources.

  Name(_PRS,ResourceTemplate()
  {

    StartDependentFn(0,2)
    {
      IO(Decode16,0x2F8,0x2F8,0x01,0x08)
      IRQNoFlags() {3}
    }

    StartDependentFn(0,2)
    {
      IO(Decode16,0x3F8,0x3F8,0x01,0x08)
      IRQNoFlags() {4,5,6,7,10,11,12}
    }

    StartDependentFn(0,2)
    {
      IO(Decode16,0x2F8,0x2F8,0x01,0x08)
      IRQNoFlags() {4,5,6,7,10,11,12}
    }

    StartDependentFn(0,2)
    {
      IO(Decode16,0x3E8,0x3E8,0x01,0x08)
      IRQNoFlags() {4,5,6,7,10,11,12}
    }

    StartDependentFn(0,2)
    {
      IO(Decode16,0x2E8,0x2E8,0x01,0x08)
      IRQNoFlags() {4,5,6,7,10,11,12}
    }

    EndDependentFn()
  })


  // Set Resource Setting Method for UART 8.

  Method(_SRS,1,Serialized)
  {
    // Point to the specific information in the passed
    // in Buffer.

    CreateByteField(Arg0,0x02,IOLO)
    CreateByteField(Arg0,0x03,IOHI)
    CreateWordField(Arg0,0x09,IRQW)

    // Set the SIO to UART 8.

    Store(2,WR07)

    // Disable the device.

    Store(0,WR30)

    // Set the Base IO Address.

    Store(IOLO,WR61)
    Store(IOHI,WR60)

    // Set the IRQ.

    FindSetRightBit(IRQW,Local0)
    If(LNotEqual(IRQW,Zero))
    {
      Decrement(Local0)
    }
    Store(Local0,WR70)

    // Enable the device.

    Store(1,WR30)
  }

  // GPE 09, S4
  Name(_PRW, Package() {0x19,4})
  Method(_PSW,1)
  {
    Store(WR70,Local0)

    If (Arg0)
    {
      // Enable as wake event, enable bit4
      Or(Local0,0x10,WR70)
    }
    Else
    {
      // Disable as wake event, disable bit4
      AND(Local0,0x0F,WR70)
    }
  }
  // D0 Method for UART 8 Port.

  Method(_PS0,0,Serialized)
  {
    Store(2,WR07)
    Store(1,WR30)
  }

  // D3 Method for UART 8 Port.

  Method(_PS3,0,Serialized)
  {
    Store(2,WR07)
    Store(0,WR30)
  }
}
