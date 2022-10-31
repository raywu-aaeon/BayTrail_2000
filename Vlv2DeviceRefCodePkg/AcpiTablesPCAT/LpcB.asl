/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Valleyview          *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved    *;
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


// LPC Bridge - Device 31, Function 0
// Define the needed LPC registers used by ASL.

scope(\_SB)
{
  OperationRegion(ILBR, SystemMemory, \IBAS, 0x8C)
  Field(ILBR, AnyAcc, NoLock, Preserve)
  {
    Offset(0x08), // 0x08
    PARC,   8,
    PBRC,   8,
    PCRC,   8,
    PDRC,   8,
    PERC,   8,
    PFRC,   8,
    PGRC,   8,
    PHRC,   8,
    Offset(0x88), // 0x88
    ,       3,
    UI3E,   1,
    UI4E,   1
  }

  Include ("98_LINK.ASL")
}

OperationRegion(LPC0, PCI_Config, 0x00, 0xC0)
Field(LPC0, AnyAcc, NoLock, Preserve)
{
  Offset(0x08), // 0x08
  SRID,   8,  // Revision ID
  Offset(0x080), // 0x80
  C1EN,   1, // COM1 Enable
  ,      31
}

//AMI_OVERRIDE
#if defined(ASL_EC_SUPPORT) && (ASL_EC_SUPPORT==1)
  Include ("EC.ASL")
#else
  Include ("Wrapper\AcpiAslWrap\PlatformEC.asl")
#endif
//AMI_OVERRIDE
Include ("LPC_DEV.ASL")

Include ("WPCN381U_SIO.asl")

// Define the KBC_COMMAND_REG-64, KBC_DATA_REG-60 Registers as an ACPI Operating
// Region.  These registers will be used to skip kbd mouse
// resource settings if not present.

OperationRegion(PKBS, SystemIO, 0x60, 0x05)
Field(PKBS, ByteAcc, Lock, Preserve)
{
  PKBD, 8,
  ,     8,
  ,     8,
  ,     8,
  PKBC, 8
}

#ifndef AMI_ACPI_SUPPORT //AMI_OVERRIDE - SIO module have the same asl code
Device(PS2K)            // PS2 Keyboard
{
  Name(_HID,"MSFT0001")
  Name(_CID,EISAID("PNP0303"))

  Method(_STA)
  {
    // Only report resources to the OS if the Keyboard is present

    If(And(LEqual(PKBD,0xFF), LEqual(PKBC, 0xFF)))
    {
      Return(0x0000)
    }

    Return(0x000F)
  }

  Name(_CRS,ResourceTemplate()
  {
    IO(Decode16,0x60,0x60,0x01,0x01)
    IO(Decode16,0x64,0x64,0x01,0x01)
    IRQ(Edge,ActiveHigh,Exclusive) {0x01}
  })

  Name(_PRS, ResourceTemplate()
  {
    StartDependentFn(0, 0)
    {
      FixedIO(0x60,0x01)
      FixedIO(0x64,0x01)
      IRQNoFlags() {1}
    }
    EndDependentFn()
  })

}

Device(PS2M)            // PS/2 Mouse
{
  Name(_HID,"MSFT0003")
  Name(_CID,EISAID("PNP0F13"))

  Method(_STA)
  {
    // Only report resources to the OS if the Mouse is present

    If(And(LEqual(PKBD,0xFF), LEqual(PKBC, 0xFF)))
    {
      Return(0x0000)
    }

    Return(0x000F)
  }

  Name(_CRS,ResourceTemplate()
  {
    IRQ(Edge,ActiveHigh,Exclusive) {0x0C}
  })

  Name(_PRS, ResourceTemplate()
  {
    StartDependentFn(0, 0)
    {
      IRQNoFlags() {12}
    }
    EndDependentFn()
  })
}
#endif //AMI_OVERRIDE - SIO module have the same asl code