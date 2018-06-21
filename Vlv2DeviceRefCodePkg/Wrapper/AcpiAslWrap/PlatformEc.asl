/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Sandy Bridge        *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c)  1999 - 2014 Intel Corporation. All rights reserved   *;
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

Scope(\_SB.PCI0.LPCB)
{

Device(H_EC)  // Hitachi Embedded Controller
{
  Name(_HID, EISAID("PNP0C09"))

  Name(_UID,1)

  Method(_STA)
  {
    Store(0,ECON) //EIP132855
    Store(3, \_SB.PCI0.GFX0.CLID)
    Return(0x00) //Hide device
  }

//EIP162868 >>
#if ASL_DPTF_EC_METHOD_SUPPORT
  Method(B1CC,0)
  {
    Return (ASL_B1CC_METHOD)
  }
  
  Method(B1ST,0)
  {
    Return (ASL_B1ST_METHOD)
  }
  
  Method(B2CC,0)
  {
    Return (ASL_B2CC_METHOD)
  }
  
  Method(B2ST,0)
  {
    Return (ASL_B2ST_METHOD)
  }
  
  Method(RPWR,0)
  {
    Return (ASL_RPWR_METHOD)
  }
    
  Method(LIDS,0)
  {
    Return (ASL_LIDS_METHOD)
  }
    
  Method(LSTE,0)
  {
    Return (ASL_LSTE_METHOD)
  }

  Method(VPWR,0)
  {
    Return (ASL_VPWR_METHOD)
  }

  Method(S3WR,0)
  {
    Return (ASL_S3WR_METHOD)
  }

  Method(TMPR,0)
  {
    Return (ASL_TMPR_METHOD)
  }
      
  Method(LTMP,0)
  {
    Return (ASL_LTMP_METHOD)
  }
#else

  Name(B1CC, 0)
  Name(B1ST, 0)
  Name(B2CC, 0)
  Name(B2ST, 0)
  Name(RPWR, 0)
  Name(LIDS, 1) //CSP20130808
  Name(LSTE, 1) //CSP20130808
  Name(VPWR, 0)
  Name(S3WR, 0)

  Name(TMPR, 0)
  Name(LTMP, 0)
#endif

  Name(FNSL, 0)
  Name(FDCY, 0)
//EIP162868 <<
  Name(ECAV, 0) //EIP132855
  Name(TSSR, 0) //EIP132855
  
  //EIP175655 >>
  Name(BATL, 0)
  Name(BATH, 0)
  Name(BCLT, 0)
  Name(PB10, 1)
  //EIP175655 <<  
  //EIP132855 >>
  Method(ECMD,1,Serialized)
  {
      Return (0xFF)
  }
  //EIP132855 <<

  Device(BAT0)
  {
    Name(_HID,EISAID("PNP0C0A"))

    Name(_UID,0)

    Method(_STA,0)
    {
      Return(0)		// Hide device 
    }
  }

  Device(BAT1)
  {
    Name(_HID,EISAID("PNP0C0A"))

    Name(_UID,1)

    Method(_STA,0)
    {
      Return(0)		// Hide device 
    }
    
    //EIP132855 >>
    Method(_BST,0)
    {
      Name(PKG1,Package() {
        0x00000000, // Battery State.
        0x00000000, // Battery Present Rate. (in mWh)
        0x00000000, // Battery Remaining Capacity. (in mWh)
        0x00000000  // Battery Present Voltage. (in mV)
      })    
      Return(PKG1)
    }
    //EIP132855 <<
  }

  Device(BAT2)
  {
    Name(_HID,EISAID("PNP0C0A"))

    Name(_UID,2)

    Method(_STA,0)
    {
      Return(0)		// Hide device 
    }
  }

} // end of H_EC
}// end scope Scope(\_SB.PCI0.LPCB)
  // System Bus

//EIP175655 >>
/*
//Scope(\_SB)
//{

  // Define a Lid Switch.

//  Device(LID0)
//  {
//    Name(_HID,EISAID("PNP0C0D"))

//    Method(_STA)
//    {
//      Return(0x00)
//    }

    //EIP132855 >>
//    Method(_LID,0)
//    {
      // 0 = Closed, 1 = Open.
//      Return(0x01)
//    }
    //EIP132855 <<
//  }
//}//end scope _SB
*/
//EIP175655 <<
