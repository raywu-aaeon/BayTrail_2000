/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  INTELISPDev2.ASL

Abstract:

  ISP Exist as B0D2F0 Software ACPI Reference Code for the ValleyView Family.

--*/
////Device ISP0
Device(ISP0)
{
  Name(_ADR, 0x0F38)
  //Name (_HID, "80860F38")
  //Name (_CID, "80860F38")
  Name(_DDN, "VLV2 ISP - 80860F38")
  Name(_UID, 0x01)

  Method (_STA, 0x0, NotSerialized)
  {
    If(LEqual(ISPD,1))   //Dev2 need report ISP0 as GFX0 child
    {
      Return (0xF)
    }
    Else
    {
      Return (0x0)
    }
  }
  Name(SBUF,ResourceTemplate ()
  {
    Memory32Fixed (ReadWrite, 0x00000000, 0x00400000, ISP0)
  })
  Method(_CRS, 0x0, NotSerialized)
  {
    Return (SBUF)
  }
  Method (_SRS, 0x1, NotSerialized)
  {
  }
  Method (_DIS, 0x0, NotSerialized)
  {
  }
  Method(_DSM, 0x4, NotSerialized)
  {
    If (LEqual (Arg0, 0x01))
    {
      ///Switch ISP to D3H
      Return (0x01)
    }
    Elseif (LEqual (Arg0, 0x02))
    {
      //Switch ISP to D0
      Return (0x02)
    }
    Else
    {
      //Do nothing
      Return (0x0F)
    }
  }
} ///End ISP0

