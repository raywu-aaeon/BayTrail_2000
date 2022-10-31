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


// General Purpose Events.  This Scope handles the Run-time and
// Wake-time SCIs.  The specific method called will be determined by
// the _Lxx value, where xx equals the bit location in the General
// Purpose Event register(s).

Scope(\_GPE)
{

  // PCI Express Hot-Plug caused the wake event.

  Method(_L01)
  {
    Add(L01C,1,L01C)  // Increment L01 Entry Count.

    P8XH(0,0x01)      // Output information to Port 80h.
    P8XH(1,L01C)


    // Check Root Port 1 for a Hot Plug Event if the Port is
    // enabled.

    If(LAnd(LEqual(RP1D,0),\_SB.PCI0.RP01.HPSX))
    {
      // Delay for 100ms to meet the timing requirements
      // of the PCI Express Base Specification, Revision
      // 1.0A, Section 6.6 ("...software must wait at
      // least 100ms from the end of reset of one or more
      // device before it is permitted to issue
      // Configuration Requests to those devices").

//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base >>
#if defined ASL_Thunderbolt_SUPPORT && ASL_Thunderbolt_SUPPORT == 1
      If(LNotEqual(\TBRP,0x01)){
        Sleep(100)
      }
#else
      Sleep(100)
#endif
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base <<

      If(\_SB.PCI0.RP01.PDCX)
      {
        // Clear all status bits first.

        Store(1,\_SB.PCI0.RP01.PDCX)
        Store(1,\_SB.PCI0.RP01.HPSX)

        //
        // PCH BIOS Spec Update Rev 1.03, Section 8.9 PCI Express* Hot-Plug BIOS Support
        // In addition, BIOS should intercept Presence Detect Changed interrupt, enable L0s on
        // hot plug and disable L0s on hot unplug. BIOS should also make sure the L0s is
        // disabled on empty slots prior booting to OS.
        //
        If(LNot(\_SB.PCI0.RP01.PDSX))
        {
          // The PCI Express slot is empty, so disable L0s on hot unplug
          //
          Store(0,\_SB.PCI0.RP01.L0SE)

        }

        // Perform proper notification
        // to the OS.

//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base >>
#if defined ASL_Thunderbolt_SUPPORT && ASL_Thunderbolt_SUPPORT == 1
        If(LNotEqual(\TBRP,0x01)){
            Notify(\_SB.PCI0.RP01,0)
        }
#else
        Notify(\_SB.PCI0.RP01,0)
#endif
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base <<
      }
      Else
      {
        // False event.  Clear Hot-Plug Status
        // then exit.

        Store(1,\_SB.PCI0.RP01.HPSX)
      }
    }

    // Check Root Port 2 for a Hot Plug Event if the Port is
    // enabled.

    If(LAnd(LEqual(RP2D,0),\_SB.PCI0.RP02.HPSX))
    {
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base >>
#if defined ASL_Thunderbolt_SUPPORT && ASL_Thunderbolt_SUPPORT == 1
      If(LNotEqual(\TBRP,0x02)){
            Sleep(100)
      }
#else
      Sleep(100)
#endif
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base <<

      If(\_SB.PCI0.RP02.PDCX)
      {
        Store(1,\_SB.PCI0.RP02.PDCX)
        Store(1,\_SB.PCI0.RP02.HPSX)

        If(LNot(\_SB.PCI0.RP02.PDSX))
        {
          Store(0,\_SB.PCI0.RP02.L0SE)
        }
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base >>
#if defined ASL_Thunderbolt_SUPPORT && ASL_Thunderbolt_SUPPORT == 1
        If(LNotEqual(\TBRP,0x02)){
            Notify(\_SB.PCI0.RP02,0)
        }
#else
        Notify(\_SB.PCI0.RP02,0)
#endif
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base <<

      }
      Else
      {
        Store(1,\_SB.PCI0.RP02.HPSX)
      }
    }

    // Check Root Port 3 for a Hot Plug Event if the Port is
    // enabled.

    If(LAnd(LEqual(RP3D,0),\_SB.PCI0.RP03.HPSX))
    {
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base >>
#if defined ASL_Thunderbolt_SUPPORT && ASL_Thunderbolt_SUPPORT == 1
      If(LNotEqual(\TBRP,0x03)){
        Sleep(100)
      }
#else
      Sleep(100)
#endif
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base <<
      If(\_SB.PCI0.RP03.PDCX)
      {
        Store(1,\_SB.PCI0.RP03.PDCX)
        Store(1,\_SB.PCI0.RP03.HPSX)

        If(LNot(\_SB.PCI0.RP03.PDSX))
        {
          Store(0,\_SB.PCI0.RP03.L0SE)
        }
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base >>
#if defined ASL_Thunderbolt_SUPPORT && ASL_Thunderbolt_SUPPORT == 1
        If(LNotEqual(\TBRP,0x03)){
          Notify(\_SB.PCI0.RP03,0)
        }
#else
        Notify(\_SB.PCI0.RP03,0)
#endif
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base <<
      }
      Else
      {
        Store(1,\_SB.PCI0.RP03.HPSX)
      }
    }

    // Check Root Port 4 for a Hot Plug Event if the Port is
    // enabled.

    If(LAnd(LEqual(RP4D,0),\_SB.PCI0.RP04.HPSX))
    {
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base >>    
#if defined ASL_Thunderbolt_SUPPORT && ASL_Thunderbolt_SUPPORT == 1
      If(LNotEqual(\TBRP,0x04)){
        Sleep(100)
      }
#else
      Sleep(100)
#endif
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base <<

      If(\_SB.PCI0.RP04.PDCX)
      {
        Store(1,\_SB.PCI0.RP04.PDCX)
        Store(1,\_SB.PCI0.RP04.HPSX)

        If(LNot(\_SB.PCI0.RP04.PDSX))
        {
          Store(0,\_SB.PCI0.RP04.L0SE)
        }
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base >>
#if defined ASL_Thunderbolt_SUPPORT && ASL_Thunderbolt_SUPPORT == 1
        If(LNotEqual(\TBRP,0x04)){
            Notify(\_SB.PCI0.RP04,0)
        }
#else
        Notify(\_SB.PCI0.RP04,0)
#endif
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base <<
      }
      Else
      {
        Store(1,\_SB.PCI0.RP04.HPSX)
      }
    }

  }

  //
  // Software GPE caused the event.
  //
  Method(_L02)
  {
    // Clear GPE status bit.
    Store(0,GPEC)
    //
    // Handle DTS Thermal Events.
    //
    External(DTSE, IntObj)
    If(CondRefOf(DTSE))
    {
      If(LGreaterEqual(DTSE, 0x01))
      {
        // EIP163305 >>
        If(CondRefOf(\_TZ.TZ01)) {
          Notify(\_TZ.TZ01,0x80)
        }
        // EIP163305 <<
        // P8XH(0,0xEE)
      }
    }
  }

  //
  // PUNIT SCI event.
  //
  Method(_L04)
  {
    // Clear the PUNIT Status Bit.
    Store(1, PSCI)
  }


  //
  // IGD OpRegion SCI event (see IGD OpRegion/Software SCI BIOS SPEC).
  //
  Method(_L05)
  {
    If(LAnd(\_SB.PCI0.GFX0.GSSE, LNot(GSMI)))   // Graphics software SCI event?
    {
      \_SB.PCI0.GFX0.GSCI()     // Handle the SWSCI
    }
  }

  //
  // This PME event (PCH's GPE #9) is received on one or more of the PCI Express* ports or
  // an assert PMEGPE message received via DMI
  //
  Method(_L09, 0)
  {
    //
    // If the Root Port is enabled, run PCI_EXP_STS handler
    //
    If(LEqual(RP1D,0))
    {
      \_SB.PCI0.RP01.HPME()
      Notify(\_SB.PCI0.RP01, 0x02)
    }

    If(LEqual(RP2D,0))
    {
      \_SB.PCI0.RP02.HPME()
      Notify(\_SB.PCI0.RP02, 0x02)
    }

    If(LEqual(RP3D,0))
    {
      \_SB.PCI0.RP03.HPME()
      Notify(\_SB.PCI0.RP03, 0x02)
    }

    If(LEqual(RP4D,0))
    {
      \_SB.PCI0.RP04.HPME()
      Notify(\_SB.PCI0.RP04, 0x02)
    }

  }

  //
  // This PME event (PCH's GPE #13) is received when any PCH internal device with PCI Power Management capabilities
  // on bus 0 asserts the equivalent of the PME# signal.
  //
  Method(_L0D, 0)
  {
    If(LAnd(\_SB.PCI0.EHC1.PMEE, \_SB.PCI0.EHC1.PMES))
    {
      If(LNotEqual(OSEL, 1))
      {
        Store(1, \_SB.PCI0.EHC1.PMES) //Clear PME status
        Store(0, \_SB.PCI0.EHC1.PMEE) //Disable PME
      }
      Notify(\_SB.PCI0.EHC1, 0x02)
    }
    If(LAnd(\_SB.PCI0.XHC1.PMEE, \_SB.PCI0.XHC1.PMES))
    {
      If(LNotEqual(OSEL, 1))
      {
        Store(1, \_SB.PCI0.XHC1.PMES) //Clear PME status
        Store(0, \_SB.PCI0.XHC1.PMEE) //Disable PME
      }
      Notify(\_SB.PCI0.XHC1, 0x02)
    }
    If(LAnd(\_SB.PCI0.HDEF.PMEE, \_SB.PCI0.HDEF.PMES))
    {
      If(LNotEqual(OSEL, 1))
      {
        Store(1, \_SB.PCI0.HDEF.PMES) //Clear PME status
        Store(0, \_SB.PCI0.HDEF.PMEE) //Disable PME
      }
      Notify(\_SB.PCI0.HDEF, 0x02)
    }
  }


  //
  // GPI09 = LPC_SIO_PME        //Need to verify this method again
  //
  Method (_L19)
  {
    P8XH(0,0x19)

    //
    // Clear the 3 SIO SWC status bits
    //
    Store(Ones, G1S3)
    Store(Ones, G1S2)
    Store(1, G1S)

    //
    // Clear GPE0_STS
    //
    Store(Ones, G0S)

    Notify(\_SB.PCI0.LPCB.WPCN, 0x02)
  }

  //
  // SUS_GPI7 = GPE23 = SMCEXT_SMI
  //
  Method (_L17)
  {
    P8XH(0,0x17)

    //
    // Clear GPE0_STS
    //
    Store(Ones, G0S)
    Notify(\_SB.PCI0.LPCB.WPCN, 0x02)
  }

  //
  // SUS_GPI0 = GPE16 GPI13 = EC WAKETIME SCI
  //
  Method (_L10)
  {
    P8XH(0,0x10)

    //
    // Clear GPE0_STS
    //
    Store(Ones, G0S)
  }
}
