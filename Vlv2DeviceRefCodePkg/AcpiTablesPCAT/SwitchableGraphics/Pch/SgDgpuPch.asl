/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - SG Reference Code                               *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c) 2010 - 2014 Intel Corporation. All rights reserved    *;
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
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/

External(\_SB.PCI0.GFX0._DOD, MethodObj)
External(\_SB.PCI0.GFX0.DD01._ADR, MethodObj)
External(\_SB.PCI0.GFX0.DD02._ADR, MethodObj)
External(\_SB.PCI0.GFX0.DD03._ADR, MethodObj)
External(\_SB.PCI0.GFX0.DD04._ADR, MethodObj)
External(\_SB.PCI0.GFX0.DD05._ADR, MethodObj)
External(\_SB.PCI0.GFX0.DD06._ADR, MethodObj)
External(\_SB.PCI0.GFX0.DD07._ADR, MethodObj)
External(\_SB.PCI0.GFX0.DD08._ADR, MethodObj)

External(\_SB.PCI0.GFX0.DD01._DGS, MethodObj)
External(\_SB.PCI0.GFX0.DD02._DGS, MethodObj)
External(\_SB.PCI0.GFX0.DD03._DGS, MethodObj)
External(\_SB.PCI0.GFX0.DD04._DGS, MethodObj)
External(\_SB.PCI0.GFX0.DD05._DGS, MethodObj)
External(\_SB.PCI0.GFX0.DD06._DGS, MethodObj)
External(\_SB.PCI0.GFX0.DD07._DGS, MethodObj)
External(\_SB.PCI0.GFX0.DD08._DGS, MethodObj)

External(\_SB.PCI0.GFX0.DD02._DCS, MethodObj)

External(\_SB.PCI0.GFX0.DD02._BCL, MethodObj)
External(\_SB.PCI0.GFX0.DD02._BQC, MethodObj)
External(\_SB.PCI0.GFX0.DD02._BCM, MethodObj)
External(\_SB.PCI0.RP01, DeviceObj)
External(\RPA4)
External(\EECP)
External(\XBAS)
External(\IOBA)
External(\HLRS)
External(\PWEN)
External(\SGMD)
External(\SGGP)
External(\RPBA)
External(\EPA1)

External(\_SB.GPO2, DeviceObj)
External(\_SB.GPO2.AVBL, IntObj)
External(\_SB.GPO2.RP1R, IntObj)
External(\_SB.GPO2.RP1P, IntObj)


Name (RPA0, 0x001C0000)   // Root Port 0 Address

Scope(\_SB.PCI0.RP01)
{
    //
    // Define a Memory Region that will allow access to the PCH root port Register Block.
    //

    OperationRegion(RPCX,SystemMemory,\RPBA,0x340)
    Field(RPCX,AnyAcc,NoLock,Preserve)
    {
        Offset(0),
        PVID,   16,
        PDID,   16,
        Offset(0x50),                   // LCTL - Link Control Register of (PCI Express* -> B00:D28:F04)
        ASPM,   2,                      // 1:0, ASPM //Not referenced in code
        ,       2,
        LNKD,   1,                      // Link Disable
        Offset(0x328), //PCIESTS1 - PCI Express Status 1
        ,       19,
        LNKS,   4,     //Link Status (LNKSTAT) {22:19}
    }

    //-----------------------------------------
    // Runtime Device Power Management - Begin
    //-----------------------------------------
    // Note:
    //      Runtime Device Power Management can be achieved by using _PRx or _PSx or both

    //
    // Name: PC05
    // Description: Declare a PowerResource object for RP01 slot device
    //
    PowerResource(PC05, 0, 0)
    {
      Name(_STA, One)

      Method(_ON, 0, Serialized)
      {
        \_SB.GPO2._ON()
        Store(One, _STA)
        
      }

      Method(_OFF, 0, Serialized)
      {
		\_SB.GPO2._OFF()	
        Store(Zero, _STA)
      }
    } //End of PowerResource(PC05, 0, 0)

    Name(_PR0,Package(){PC05})
    Name(_PR2,Package(){PC05})
    Name(_PR3,Package(){PC05})


    Method(_S0W, 0)
    {
      Return(4) //D3cold is supported
    }

    //-----------------------------------------
    // Runtime Device Power Management - End
    //-----------------------------------------

    Device(PEGP) { // (PCI Express* -> B00:D28:F00) Slot Device D0F0
      Name(_ADR, 0x00000000)
      
//      Method(_PRW, 0) { Return(GPRW(0x09, 4)) } // can wakeup from S4 state
    } // (PCI Express* -> B00:D28:F00) Slot Device D0F0

    Device(PEGA) { // (PCI Express* -> B00:D28:F00) Slot Device D0F1
      Name(_ADR, 0x00000001)

      OperationRegion(ACAP, PCI_Config, 0x40,0x14)      
      Field(ACAP,DWordAcc, NoLock,Preserve)
      {
        Offset(0x10),
        LCT1,   16,  // Link Control register
      }
//      Method(_PRW, 0) { Return(GPRW(0x09, 4)) } // can wakeup from S4 state
    } // (PCI Express* -> B00:D28:F00) Slot Device D0F1
}  // end of Scope(\_SB.PCI0.RP01)


Scope(\_SB.PCI0.RP01.PEGP)
{
    Name (ONOF, 0x1) //Endpoint On-Off flag status. Assume Endpoint is ON by default {1-ON, 0-OFF}
    Name (IVID, 0xFFFF) //Invalid Vendor ID
    Name (ELCT, 0x00000000)
    Name (HVID, 0x0000)
    Name (HDID, 0x0000)

    OperationRegion(PCIS,SystemMemory,\EPA1,0xF0)
    Field(PCIS, AnyAcc, Lock, Preserve)
    {
        Offset(0x0),
        DVID, 16,
        Offset(0xB),
        CBCC, 8,
        Offset(0x2C),
        SVID, 16,
        SDID, 16,
        Offset(0x4C),
        WVID, 16,
        WDID, 16,
    }

    OperationRegion(PCAP,SystemMemory,Add(\EPA1,0x48),0x40)
    Field(PCAP,DWordAcc, NoLock,Preserve)
    {
        Offset(0x10),
        LCTL,   16,                      // Link Control register
    }

    Method (_INI)
    {
        Store (0x0, \_SB.PCI0.RP01.PEGP._ADR)
    }
    
// Note: Some of methods are related to Switchable Graphics Support where OS is not involved to turn ON/OFF the card for Hybrid Graphics. 
// To remove unwanted methods, it requires to communicate with DGPU Card drivers team. So those are left as it is for now.

    Method(HGON,0,Serialized)
    {
        P8XH(0,0xD6)
        P8XH(1,0x00)
		
        If (LEqual(CCHK(1), 0))
        {
          P8XH(0,0xD6)
          P8XH(1,0xC0)
          Store("\_SB.PCI0.RP05.PEGP.HGON is not allowed to execute ", Debug)
          Return ()
        }

        Store(1, ONOF) //Indicate Endpoint is in ON state

		//Power on the dGPU card
	    \_SB.GPO2._ON()

        // Enable the Root Port Link by setting Link Disable bit to 1
        Store(0,LNKD)

        //wait until link has trained to x2. Verify        
        While(LLess(LNKS,7))
        {
                Sleep(1)
        }

        // Re-store the DGPU SSID
        Store(HVID,WVID)
        Store(HDID,WDID)

        // Re-store the Link Control register - Common Clock Control and ASPM
        Or(And(ELCT,0x0043),And(LCTL,0xFFBC),LCTL)
        Or(And(ELCT,0x0043),And(\_SB.PCI0.RP01.PEGA.LCT1,0xFFBC),\_SB.PCI0.RP01.PEGA.LCT1)

        Return ()
    }
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#if defined (ASL_SgTpv_SUPPORT) && (ASL_SgTpv_SUPPORT == 1)
#else
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform <<
    Method(_ON,0,Serialized)
    {
        HGON()

        //Ask OS to do a PnP rescan
        Notify(\_SB.PCI0.RP01,0)

        Return ()
    }
#endif //AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform
    Method(HGOF,0,Serialized)
    {
        P8XH(0,0xD6)
        P8XH(1,0x0F)

        If (LEqual(CCHK(0), 0))
        {
          P8XH(0,0xD6)
          P8XH(1,0xCF)

          Return ()
        }
        Store(0, ONOF) //Indicate Endpoint is in OFF state

        // Save the Link Control register
        Store(LCTL,ELCT)

        // Save the DGPU SSID
        Store(SVID,HVID)
        Store(SDID,HDID)

        //Force disable the Root Port link
        Store(1, LNKD)

        //Wait till link is actually in disabled state
        While(LNotEqual(LNKS,0))
        {
            Sleep(1)
        }

        //Power-off the dGPU card
        \_SB.GPO2._OFF()
		
        Return ()
    }
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#if defined (ASL_SgTpv_SUPPORT) && (ASL_SgTpv_SUPPORT == 1)
#else
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform <<      
    Method(_OFF,0,Serialized)
    {

        HGOF()

        //Ask OS to do a PnP rescan
        Notify(\_SB.PCI0.RP01,0)

        Return ()
    }
#endif //AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform
    Method(EPON, 0, Serialized)
    {
      Store(1, ONOF) //Indicate Endpoint is in ON state

      Return ()
    }

    Method(_STA,0,Serialized)
    {
      //
      // Check SGMode and dGPU Present Detect GPIO for SG system
      //
      If(And(SGMD,0x0F))
      {
        If(LNotEqual(SGGP,0x01))
        {
           Return(0x0F)
        }

        // To detect dGPU: Check Device is present and which belongs to display controllers type also.
        If(LNotEqual(DVID,0xFFFF))
        {
            If(LEqual(CBCC,0x3)) // Base Class Code 03h which is referring all types of display controllers
            {
               Return(0x0F)
            }
        }
	  
        Return(0x00)
      }

      //
      // For non-SG system check for valid Vendor Id
      //
      If(LNotEqual(DVID,0xFFFF))
      {
          Return(0x0F)
      }
      Return(0x00)
    }


    Method(_DOD,0)
    {
        Return (\_SB.PCI0.GFX0._DOD())
    }


    Device(DD01)
    {
            Method(_ADR,0,Serialized)
            {
                    Return(\_SB.PCI0.GFX0.DD01._ADR())
            }

            // Device Current State.
            Method(_DCS,0)
            {

            }

            // Device Get State.

            Method(_DGS,0)
            {
                    // Return the Next State.
                    Return(\_SB.PCI0.GFX0.DD01._DGS())
            }

            // Device Set State.

            // _DSS Table:
            //
            //  BIT31   BIT30   Execution
            //  0       0       Don't implement.
            //  0       1       Cache change.  Nothing to Implement.
            //  1       0       Don't Implement.
            //  1       1       Display Switch Complete.  Implement.

            Method(_DSS,1)
            {
                    // Do nothing here in the OpRegion model.  OEMs may choose to
                    // update internal state if necessary.
            }
    }

    Device(DD02)
    {
            Method(_ADR,0,Serialized)
            {
                    Return(\_SB.PCI0.GFX0.DD02._ADR())
            }

            // Device Current State.

            Method(_DCS,0)
            {
                    // Get the Current Display State.
                    Return(\_SB.PCI0.GFX0.DD02._DCS())
            }

            // Device Get State.

            Method(_DGS,0)
            {
                    // Return the Next State.
                    Return(\_SB.PCI0.GFX0.DD02._DGS())
            }

            // Device Set State.

            Method(_DSS,1)
            {
                    // Do nothing here in the OpRegion model.  OEMs may choose to
                    // update internal state if necessary.
            }

/*
            Method(_DDC,1)
            {
                    If(Lor(LEqual(\_SB.PCI0.GFX0.PHED,1),LEqual(\_SB.PCI0.GFX0.PHED,2)))
                    {
                        Name(DDC2,Buffer (256) {0x0})
                        Store(\_SB.PCI0.GFX0.BDDC,DDC2)
                        Return(DDC2)
                    }
                    Return(Buffer(256){0x0})
            }
*/
            Method(_BCL,0)
            {
                    Return(\_SB.PCI0.GFX0.DD02._BCL())
            }

            Method(_BQC,0)
            {
                    Return(\_SB.PCI0.GFX0.DD02._BQC())
            }
            
            Method(_BCM,1)
            {
                    Return(\_SB.PCI0.GFX0.DD02._BCM(Arg0))
            }

    }

    Device(DD03)
    {
            Method(_ADR,0,Serialized)
            {
                    Return(\_SB.PCI0.GFX0.DD03._ADR())
            }

            // Device Current State.

            Method(_DCS,0)
            {
                    // Get the Current Display State.
            }

            // Device Get State.

            Method(_DGS,0)
            {
                    // Return the Next State.
                    Return(\_SB.PCI0.GFX0.DD03._DGS())
            }

            // Device Set State.

            Method(_DSS,1)
            {
                    // Do nothing here in the OpRegion model.  OEMs may choose to
                    // update internal state if necessary.
            }
    }

    Device(DD04)
    {
            Method(_ADR,0,Serialized)
            {
                    Return(\_SB.PCI0.GFX0.DD04._ADR())
            }

            // Device Current State.

            Method(_DCS,0)
            {
                    // Get the Current Display State.
            }

            // Device Get State.

            Method(_DGS,0)
            {
                    // Return the Next State.
                    Return(\_SB.PCI0.GFX0.DD04._DGS())
            }

            // Device Set State.

            Method(_DSS,1)
            {
                    // Do nothing here in the OpRegion model.  OEMs may choose to
                    // update internal state if necessary.
            }
            
    }

    Device(DD05)
    {
            Method(_ADR,0,Serialized)
            {
                    Return(\_SB.PCI0.GFX0.DD05._ADR())
            }

            // Device Current State.

            Method(_DCS,0)
            {
                    // Get the Current Display State.
            }

            // Device Get State.

            Method(_DGS,0)
            {
                    // Return the Next State.
                    Return(\_SB.PCI0.GFX0.DD05._DGS())
            }

            // Device Set State.

            Method(_DSS,1)
            {
                    // Do nothing here in the OpRegion model.  OEMs may choose to
                    // update internal state if necessary.
            }
    }

    Device(DD06)
    {
            Method(_ADR,0,Serialized)
            {
                    Return(\_SB.PCI0.GFX0.DD06._ADR())
            }

            // Device Current State.

            Method(_DCS,0)
            {
                    // Get the Current Display State.
            }

            // Device Get State.

            Method(_DGS,0)
            {
                    // Return the Next State.
                    Return(\_SB.PCI0.GFX0.DD06._DGS())
            }

            // Device Set State.

            Method(_DSS,1)
            {
                    // Do nothing here in the OpRegion model.  OEMs may choose to
                    // update internal state if necessary.
            }
    }
    
    Device(DD07)
    {
            Method(_ADR,0,Serialized)
            {
                    Return(\_SB.PCI0.GFX0.DD07._ADR())
            }

            // Device Current State.

            Method(_DCS,0)
            {
                    // Get the Current Display State.
            }

            // Device Get State.

            Method(_DGS,0)
            {
                    // Return the Next State.
                    Return(\_SB.PCI0.GFX0.DD07._DGS())
            }

            // Device Set State.

            Method(_DSS,1)
            {
                    // Do nothing here in the OpRegion model.  OEMs may choose to
                    // update internal state if necessary.
            }
    }

    Device(DD08)
    {
            Method(_ADR,0,Serialized)
            {
                    Return(\_SB.PCI0.GFX0.DD08._ADR())
            }

            // Device Current State.

            Method(_DCS,0)
            {
                    // Get the Current Display State.
            }

            // Device Get State.

            Method(_DGS,0)
            {
                    // Return the Next State.
                    Return(\_SB.PCI0.GFX0.DD08._DGS())
            }

            // Device Set State.

            Method(_DSS,1)
            {
                    // Do nothing here in the OpRegion model.  OEMs may choose to
                    // update internal state if necessary.
            }
    }


  //
  // Name: CCHK
  // Description: Function to check whether _ON/_OFF sequence is allowed to execute for the given RP01 controller or not
  // Input: Arg0 -> 0 means _OFF sequence, 1 means _ON sequence
  // Return: 0 - Don't execute the flow, 1 - Execute the flow
  //
  Method(CCHK,1)
  {
    // Check for RP01 controller presence
    If(LEqual(PVID, IVID))
    {
      Return(0)
    }

    //If Endpoint is not present[already disabled] before executing _OFF then don't call the _OFF method
    //If Endpoint is present[already enabled] before executing _ON then don't call the _ON method
    If(LEqual(Arg0, 0))
    {
      //_OFF sequence condition check
      If(LEqual(ONOF, 0))
      {
        Return(0)
      }
    }
    ElseIf(LEqual(Arg0, 1))
    {
      //_ON sequence condition check
      If(LEqual(ONOF, 1))
      {
        Return(0)
      }
    }

    Return(1)
  } // End of Method(CCHK,1)

}

Scope(\_SB.GPO2)
{
   //ACTION TODO:
   //........................................................................................
   //While powering up the slot again, the only requirement is that the Reset# should be 
   //de-asserted after the power to slot is up (Standard requirement as per PCIe spec).

   //Note:
   //Before power enable, and after power enable, the reset should be in hold condition.
   //Some delay is given for power rails and clocks to become stable.
   //So during this period, reset must not be released.
   //........................................................................................

   Method(_ON,0,Serialized)
   {
      if(LEqual (\_SB.GPO2.AVBL, 1))
      {
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#if defined (ASL_SgTpv_SUPPORT) && (ASL_SgTpv_SUPPORT == 1)
        #if ASL_ACTIVE_dGPU_HOLD_RST==1
	        Store( 0x00, \_SB.GPO2.RP1R)
        #else
	        Store( 0x01, \_SB.GPO2.RP1R)
        #endif
	        Store( ASL_ACTIVE_dGPU_PWR_EN, \_SB.GPO2.RP1P)
		Sleep(30)
	        Store( ASL_ACTIVE_dGPU_HOLD_RST, \_SB.GPO2.RP1R)
		Sleep(10)
#else
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform <<
		Store( 0x01, \_SB.GPO2.RP1R)
		Store( 0x00, \_SB.GPO2.RP1P)
		Sleep(30)
		Store( 0x00, \_SB.GPO2.RP1R)
		Sleep(10)
#endif //AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform      
      }	
   }

   //ACTION TODO:
   //........................................................................................
   //To turn off the power to the slot, all you would need to do is assert the RESET# 
   //and then take off the power using the power enable GPIO.
   //Once the power goes off, the clock request from the slot to the PCH is also turned off, 
   //so no clocks will be going to the PCIe slot anymore.
   //........................................................................................

   Method(_OFF,0,Serialized)
   {
      if(LEqual (\_SB.GPO2.AVBL, 1))
      {
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#if defined (ASL_SgTpv_SUPPORT) && (ASL_SgTpv_SUPPORT == 1)
	#if ASL_ACTIVE_dGPU_HOLD_RST==1
	        Store( 0x00, \_SB.GPO2.RP1R)
        #else
	        Store( 0x01, \_SB.GPO2.RP1R)
        #endif
    
	#if ASL_ACTIVE_dGPU_HOLD_RST==1
	        Store( 0x00, \_SB.GPO2.RP1P)	
	#else    
	        Store( 0x01, \_SB.GPO2.RP1P)
	#endif
#else
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform <<
		Store( 0x01, \_SB.GPO2.RP1R)
		Store( 0x01, \_SB.GPO2.RP1P)
#endif //AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform
      }	
   }

}  // end of Scope(\_SB.GPO2)
