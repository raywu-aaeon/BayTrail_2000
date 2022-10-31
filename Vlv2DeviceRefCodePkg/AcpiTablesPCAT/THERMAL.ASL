/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Valleyview          *;
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


// THERMAL.ASL represents a Thermal Zone to be used for testing on the
// Customer Reference Boards.

External(\_SB.DPTF.CTOK, MethodObj)

Scope(\_TZ)
{
  // Notes:
  //    1) WIN2K strictly uses interrupt driven thermal events.
  //    2) Temperature values are stored in tenths of Kelvin to
  //            eliminate the decimal place.
  //    3) Kelvin = Celsius + 273.2.
  //    4) All temperature must be >= 289K.

  // Fan 0 = Package Processor Fan.

  PowerResource(FN00,0,0)
  {
    Method(_STA,0,Serialized)
    {
      // Return Virtual Fan 0 status.

      Return(VFN0)
    }

    Method(_ON,0,Serialized)
    {
      // Set Virtual Fan 0 On.

      Store(1,VFN0)
      Store(1, \_SB.PCI0.LPCB.H_EC.FNSL)      // Set bit-0 to select CPU Fan.
      Store(50, \_SB.PCI0.LPCB.H_EC.FDCY)     // Fan set to 50% duty cycle.
      \_SB.PCI0.LPCB.H_EC.ECMD(0x1A)          // Fan command.
    }

    Method(_OFF,0,Serialized)
    {
      // Set Virtual Fan 0 Off.

      Store(0,VFN0)
      Store(1, \_SB.PCI0.LPCB.H_EC.FNSL)      // Set bit-0 to select CPU Fan.
      Store(0, \_SB.PCI0.LPCB.H_EC.FDCY)      // Fan set to 0% duty cycle.
      \_SB.PCI0.LPCB.H_EC.ECMD(0x1A)          // Fan command.
    }
  }

  // Associate Virtual Fan 0 Power Resource with the FAN0 Device.

  Device(FAN0)
  {
    Name(_HID, EISAID("PNP0C0B"))
    Name(_UID,0)
    Name(_PR0, Package(1) {FN00})
  }

  // Thermal Zone 1 = DTS Thermal Zone.

  ThermalZone(TZ01)
  {
    // Return the temperature at which the OS initiates Active Cooling.

    Method(_AC0,0,Serialized)
    {
      if(LEqual(ACTT, 0))
      {
        Store( 50, ACTT)
      }
      Return(Add(2732,Multiply(ACTT,10)))
    }

    // Returns the Device to turn on when _ACx is exceeded.

    Name(_AL0, Package(1) {FAN0})

    // Return the temperature at which the OS must perform a Critcal
    // Shutdown.

    Method(_CRT,0,Serialized)
    {
      Return(Add(2732,Multiply(CRTT,10)))
    }

    // Notifies ASL Code the current cooling mode.
    //      0 - Active cooling
    //      1 - Passive cooling

    Method(_SCP,1,Serialized)
    {
      Store(Arg0,CTYP)
    }

    // _TMP (Temperature)
    //
    // Return the highest of the CPU temperatures to the OS.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   An Integer containing the current temperature of the thermal zone (in tenths of degrees Kelvin)
    //
    Method(_TMP,0,Serialized)
    {
      //
      // If DPTF enabled, return highest temperature from EC thermal sensors
      //
      If(LAnd(DPTE,\ECON))
      {
        If(LGreater(\_SB.PCI0.LPCB.H_EC.LTMP, \_SB.PCI0.LPCB.H_EC.TMPR))
        {
          Return(Add(2732,Multiply(\_SB.PCI0.LPCB.H_EC.LTMP,10)))
        } Else
        {
          Return(Add(2732,Multiply(\_SB.PCI0.LPCB.H_EC.TMPR,10)))
        }
        //
        // ElseIf DTS enabled, return higher temperature of the 2 cores
        //
      } ElseIf(DTSE)
      {
        If(LGreater(DTS2, DTS1))
        {
          Store(DTS2,Local0)
        } else
        {
          Store(DTS1,Local0)
        }
        Return(Add(2732,Multiply(Local0,10)))
        //
        // Else return a static value if both EC and DTS are unavailable.
        //
      } Else
      {
        Return(3000) // (3000-2732)/10 = 26.8 degree C
      }
    }

    // Return the Processor(s) used for Passive Cooling.

    Method(_PSL,0,Serialized)
    {
      If(LEqual(MPEN, 4))
      {
        //  CMP - Throttling controls all four logical CPUs.
        Return(Package() {\_PR.CPU0,\_PR.CPU1,\_PR.CPU2,\_PR.CPU3})
      }

      If(MPEN)
      {
        //  CMP - Throttling controls both CPUs.

        Return(Package() {\_PR.CPU0,\_PR.CPU1})
      }

      Return(Package() {\_PR.CPU0})
    }

    // Returns the temperature at which the OS initiates CPU throttling.

    Method(_PSV,0,Serialized)
    {
      Return(Add(2732,Multiply(PSVT,10)))
    }

    // Returns TC1 value used in the passive cooling formula.

    Method(_TC1,0,Serialized)
    {
      Return(TC1V)
    }

    // Returns TC2 value used in the passive cooling formula.

    Method(_TC2,0,Serialized)
    {
      Return(TC2V)
    }

    // Returns the sampling period used in the passive cooling formula.

    Method(_TSP,0,Serialized)
    {
      Return(TSPV)
    }

    // Returns Hot Temperature

    Method(_HOT,0,Serialized)
    {
      Subtract(CRTT, 5, Local0)
      Return(Add(2732,Multiply(Local0,10)))
    }
  }
}
