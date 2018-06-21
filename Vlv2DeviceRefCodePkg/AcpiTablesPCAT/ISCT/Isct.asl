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

DefinitionBlock (
  "IsctAcpi.aml",
  "SSDT",
  1,
  "Intel_",
  "IsctTabl",
  0x1000
  )

{ // Start of Definition Block

  External(\_SB.ICNF, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.ISTS)
  External(\_SB.PCI0.LPCB.H_EC.IWRS)
  External(\_SB.PCI0.LPCB.H_EC.S3T0)
  External(\_SB.PCI0.LPCB.H_EC.S3T1)
  External(\_SB.PCI0.LPCB.H_EC.S3T2)
  External(\_SB.PCI0.LPCB.H_EC.S0IS)
  External(\_SB.PCI0.LPCB.H_EC.S3WR)
  External(\_SB.PCI0.LPCB.H_EC.IPNC)
  External(\_SB.PCI0.LPCB.H_EC.LSTE)
  External(\_SB.PCI0.LPCB.H_EC.RPWR)
  External(\_SB.PCI0.GFX0.ASLC)
  External(\_SB.PCI0.GFX0.ASLE)
  External(\_SB.PCI0.GFX0.STAT)
  External(\_SB.PCI0.GFX0.TCHE)
  External(\_SB.PCI0.GFX0.PARD, MethodObj)      // Check if the driver is ready to handle ASLE interrupts


  Scope (\_SB)
  {
    Device (IAOE)
    {
      OperationRegion(ISCT,SystemMemory,0xFFFF0008,0xAA58)
      Field(ISCT,AnyAcc,Lock,Preserve)
      {
        WKRS,  8,      // (0) ISCT Wake Reason
               AOCE,  8,      // (1) ISCT is Enabled
      }

      Name (_HID, "INT33A0")
      Name (_UID, 0x00)

      Name (INSB, 0)
      Name (ABT0, 0)
      Name (ABT1, 0)
      Name (AWDT, 0)

      Name (PTSL, 0)   // Platform Sleep Level      

      //
      // _INI (Initialization Method)
      //
      Method(_INI)
      {
      }

      Method(_STA)
      {
        If(LEqual(And(AOCE, 0x01), 0x01) )
        {
          Return(0x0F)
        } Else
        {
          Return(0x00)
        }
      }

      //
      // GABS - Get ISCT BIOS Enabled Settings
      // Input:   None
      // Return:   Enabled / Disabled Status of ISCT
      // Bits   Description
      // 0   -  ISCT Configured: 0 = Disabled, 1 = Enabled
      // 1   -  ISCT Notification Control: 0 = Unsupported, 1 = Supported
      // 2   -  ISCT WLAN Power Control : 0 = Unsupported, 1 = Supported
      // 3   -  ISCT WWAN Power Control : 0 = Unsupported, 1 = Supported
      // 4       -  1 (Must be set to 1)
      // 5   -  Sleep duration value format : 0 = Actual time, 1 = Duration in seconds
      // 6   -  RF Kill Support (Radio On/Off): 0 = Soft Switch, 1 = Physical Switch
      // 7   -  Reserved
      // Default Values as per BIOS CMOS settings are 0111 0111 (0x77)
      //
      Method (GABS, 0, NotSerialized)
      {
        Return (ICNF)
      }

      //
      // GAOS - Get current status of ISCT Function Status (S0-iSCT Status)
      // Input:   None
      // Return:
      // Bits   Description
      // 0      S0-ISCT Mode: 0 = Disabled, 1 = Enabled
      // 1              AC Status: 0 = On Battery, 1 = On AC
      // 2 - 7  Reserved
      //
      Method (GAOS, 0, NotSerialized)
      {
        Store(\_SB.PCI0.LPCB.H_EC.S0IS, Local0)
        And(Local0, 0xFD, Local0)
        Or(Local0,ShiftLeft(And(\_SB.PCI0.LPCB.H_EC.RPWR,0x01),1),Local0)
        Return (Local0)
      }

      //
      // SAOS - Set AOAC Function Status
      // Input:
      // Bits   Description
      // 0      AOAC Mode: 0 = Disabled, 1 = Enabled
      // 1              AC Status: 0 = On Battery, 1 = On AC
      // 2 - 7  Reserved
      //
      Method (SAOS, 1, NotSerialized)
      {
        Or(0x00, And(Arg0, 0x03), Local0)
        Store(\_SB.PCI0.LPCB.H_EC.S0IS, Local1)

        If(LNOTEqual(And(Local0, 0x01), 0x01))
        {
          If(And(Local1, 0x01))
          {
            // Update Gfx driver of ISCT function is disabled, (This can be called when in S0-ISCT)
            If(And(\_SB.PCI0.GFX0.TCHE, 0x100))
            {
//            If (LNot(\_SB.PCI0.GFX0.PARD()))
//            {
              And(\_SB.PCI0.GFX0.STAT, Not(0x03), \_SB.PCI0.GFX0.STAT)          // STAT[1:0] = 00, Normal Resume to S0

              Or(\_SB.PCI0.GFX0.ASLC, 0x100, \_SB.PCI0.GFX0.ASLC)               // ASLC[8] = 1, ISCT State Change Request

              Store(0x01, \_SB.PCI0.GFX0.ASLE)                                  // Generate ASLE interrupt
//            }
            }
          }
        }

        Store(Local0, \_SB.PCI0.LPCB.H_EC.S0IS)
      }

      //
      // GANS - Get AOAC Notification Status
      // Input:   None
      // Return:
      // Bits   Description
      // 0      AOAC Notification : 0 = Disabled, 1 = Enabled
      // 1 - 7  Reserved
      //
      Method (GANS, 0, NotSerialized)
      {
        Return (INSB)
      }

      //
      // SANS - Set AOAC(ISCT) Notification Status
      // Input:
      // Bits   Description
      // 0      AOAC Notification : 0 = Disabled, 1 = Enabled
      // 1 - 7  Reserved
      //
      Method (SANS, 1, NotSerialized)
      {
        Store(And(Arg0, 0x01), INSB)
      }

      //
      // GWLS - Get WLAN Module Status
      // Input:   None
      // Return:
      // Bits   Description
      // 0      WLAN Wireless Disable (W_DISABLE#) :0 = Disabled, 1 = Enabled
      // 1      WLAN Module Powered in S3: 0 = Disabled, 1 = Enabled
      // 2      WLAN Module Powered in S4: 0 = Disabled, 1 = Enabled
      // 3      WLAN Module Powered in S5: 0 = Disabled, 1 = Enabled
      // 4 - 7  Reserved
      //
      Method (GWLS, 0, NotSerialized)
      {
        // EC updates any change in power control if occured, if no change happens it returns as set in SWLS
        Store(\_SB.PCI0.LPCB.H_EC.IPNC, Local1)

        Or(0x00, And(Local1, 0x0E), Local0)
        Return (Local0)
      }
      //
      // SWLS - Set WLAN Module Status
      // Input:
      // Bits   Description
      // 0      N/A (WLAN Wireless Disable is Read only)  Always set to 0
      // 1      WLAN Module Powered in S3: 0 = Disabled, 1 = Enabled
      // 2      WLAN Module Powered in S4: 0 = Disabled, 1 = Enabled
      // 3      WLAN Module Powered in S5: 0 = Disabled, 1 = Enabled
      // 4 - 7  Reserved
      //
      Method (SWLS, 1, NotSerialized)
      {
        Store(\_SB.PCI0.LPCB.H_EC.IPNC, Local0)
        And(Local0,0xF0, Local0)

        Or(Local0, And(Arg0, 0x0E), Local0)

        // Based on these settings EC will control WLAN power
        Store(Local0, \_SB.PCI0.LPCB.H_EC.IPNC)
      }

      //
      // GWWS - Get WWAN Module Status
      // Input:   None
      // Return:
      // Bits   Description
      // 0      WWAN Wireless Disable (W_DISABLE#) :0 = Disabled, 1 = Enabled
      // 1      WWAN Module Powered in S3: 0 = Disabled, 1 = Enabled
      // 2      WWAN Module Powered in S4: 0 = Disabled, 1 = Enabled
      // 3      WWAN Module Powered in S5: 0 = Disabled, 1 = Enabled
      // 4 - 7  Reserved
      //
      Method (GWWS, 0, NotSerialized)
      {
        // EC updates any change in power control if occured, if no change happens it returns as set in SWWS
        Store(\_SB.PCI0.LPCB.H_EC.IPNC, Local1)
        ShiftRight(Local1, 3, Local1)

        Or(0x00, And(Local1, 0x0E), Local0)
        Return (Local0)
      }

      //
      // SWWS - Set WWAN Module Status
      // Input:
      // Bits   Description
      // 0      N/A (WWAN Wireless Disable is Read only)  Always set to 0
      // 1      WWAN Module Powered in S3: 0 = Disabled, 1 = Enabled
      // 2      WWAN Module Powered in S4: 0 = Disabled, 1 = Enabled
      // 3      WWAN Module Powered in S5: 0 = Disabled, 1 = Enabled
      // 4 - 7  Reserved
      //
      Method (SWWS, 1, NotSerialized)
      {
        Or(0x00, And(Arg0, 0x0E), Local0)

        Store(\_SB.PCI0.LPCB.H_EC.IPNC, Local1)
        And(Local1,0x0E, Local1)
        Or(Local1, ShiftLeft(Local0, 3), Local1)

        // Based on these settings EC will control WWAN power
        Store(Local1, \_SB.PCI0.LPCB.H_EC.IPNC)
      }

      //
      // SASD - Set Intel Smart Connect Technology Sleep Duration
      //
      Method (SASD, 1, NotSerialized)
      {
        Store(Arg0, Local0)
        If(LEqual(And(Local0, 0x80000000), 1))
        {
          //No support for actual time format
        }
        Else
        {
//                      If(LEqual(PTSL, 0x03))  // Temp disabled, to verify with PNP excerciser tool
//                      {
          Store(And(Local0, 0xFF), \_SB.PCI0.LPCB.H_EC.S3T0)
          Store(ShiftRight(And(Local0, 0xFF00), 8), \_SB.PCI0.LPCB.H_EC.S3T1)
          Store(ShiftRight(And(Local0, 0xFF0000), 16), \_SB.PCI0.LPCB.H_EC.S3T2)
//                      }
        }
      }

      //
      // GPWR - Get Platform Wake Reason
      //
      Method (GPWR, 0, NotSerialized)
      {
        Store(And(\_SB.PCI0.LPCB.H_EC.S3WR, 0x03), Local0)
        Store(And(WKRS, 0x0C), Local1)

        If(And(Local1, 0x0C))
        {
          Return(And(Local1, 0x0C))
        }
        ElseIf(And(Local0, 0x03))
        {
          Return(And(Local0, 0x03))
        }
        Else
        {
          Return(0)
        }
      }

      //
      // GAWD - Get ISCT Wake Duration
      //
      Method (GAWD, 0, NotSerialized)
      {
        Return (AWDT)
      }

      //
      // SAWD - Set ISCT Wake Duration
      //
      Method (SAWD, 1, NotSerialized)
      {
        Store (Arg0, AWDT)
      }

      //
      // GPCS - Get Platform Current State i.e., LID is ON or OFF
      //
      Method (GPCS, 0, NotSerialized)
      {
        // 0 = LID Closed, 1 = LID Open.
        Store(\_SB.PCI0.LPCB.H_EC.LSTE, Local0)
        Return (Local0)
      }

    } // Device (IAOE)
  } // Scope (\_SB)
} // End SSDT


