/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c) 1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  AmbientTempRemote1Part.asl

Abstract:

  Intel ACPI Reference Code for Dynamic Platform & Thermal Framework

--*/

Device(STR2)    // Ambient remote 1 temp sensor, U3A1,ADT7481
{
  Name(_HID, EISAID("INT3403")) // Intel DPTF Temperature Sensor Device
  Name(_UID, 2)                 // Id for this sensor
  Name(PTYP, 0x03)              // Participant type - Generic
  Name(_STR, Unicode ("Ambient remote 1 temp sensor"))
  Name(CTYP, 0x0)               // Device specific cooling policy type
  Name(LTM2, 0x0)                // Last recorded temperature

  // _STA (Status)
  //
  // This object returns the current status of a device.
  //
  // Arguments: (0)
  //   None
  // Return Value:
  //   An Integer containing a device status bitmap:
  //    Bit 0 – Set if the device is present.
  //    Bit 1 – Set if the device is enabled and decoding its resources.
  //    Bit 2 – Set if the device should be shown in the UI.
  //    Bit 3 – Set if the device is functioning properly (cleared if device failed its diagnostics).
  //    Bit 4 – Set if the battery is present.
  //    Bits 5–31 – Reserved (must be cleared).
  //
  Method(_STA)
  {
    If (LEqual(THM1,1))
    {
      Return(0x0F)
    } Else
    {
      Return(0x00)
    }
  }

  // _DTI (Device Temperature Indication)
  //
  // Conveys the temperature of a device's internal temperature sensor to the platform when a temperature trip point
  // is crossed or when a meaningful temperature change occurs.
  //
  // Arguments: (1)
  //   Arg0 - An Integer containing the current value of the temperature sensor (in tenths Kelvin)
  // Return Value:
  //   None
  //
  Method(_DTI, 1)
  {
    Store(Arg0,LTM2)
    Notify(STR2, 0x91) // notify the participant of a trip point change event
  }

  // _NTT (Notification Temperature Threshold)
  //
  // Returns the temperature change threshold for devices containing native temperature sensors to cause
  // evaluation of the _DTI object
  //
  // Arguments: (0)
  //   None
  // Return Value:
  //   An Integer containing the temperature threshold in tenths of degrees Kelvin.
  //
  Method(_NTT, 0)
  {
    Return(2782)  // 5 degree Celcius, this could be a platform policy with setup item
  }

  // _TMP (Temperature)
  //
  // This control method returns the thermal zone’s current operating temperature.
  //
  // Arguments: (0)
  //   None
  // Return Value:
  //   An Integer containing the current temperature of the thermal zone (in tenths of degrees Kelvin)
  //
  Method(_TMP,0,Serialized)
  {
    If(\ECON)
    {
      // Get temperature.
      Return(Add(2732,Multiply(\_SB.PCI0.LPCB.H_EC.ART1,10)))
    } Else
    {
      Return (Zero)
    }
  }

  // Number of Aux Trips available
  Name(PATC, 2)

  // PATx (Participant Programmable Auxiliary Trip) - Sets Aux Trip Point
  //
  // The PATx objects shall take a single integer parameter, in tenths of degree Kelvin, which
  // represents the temperature at which the device should notify the participant driver of
  // an auxiliary trip event. A PATx control method returns no value.
  //
  //  Arguments: (1)
  //    Arg0 - raw temperature in tenths of degree Kelvin
  //  Return Value:
  //    None
  //
  Method(PAT0, 1, Serialized)
  {
    If (\ECON)
    {
      Store (Acquire(\_SB.PATM, 100),Local0)  // save Acquire result so we can check for Mutex acquired
      If (LEqual(Local0, Zero))   // check for Mutex acquired
      {
        Store(Divide(Subtract(Arg0,2732),10), \_SB.PCI0.LPCB.H_EC.TSLT)
        Store(2, \_SB.PCI0.LPCB.H_EC.TSI)
        Store(0x2, \_SB.PCI0.LPCB.H_EC.HYST)
        \_SB.PCI0.LPCB.H_EC.ECMD(0x4A)            //Set Trip point.
        Release(\_SB.PATM)
      }
    }
  }

  // PATx (Participant Programmable Auxiliary Trip) - Sets Aux Trip Point
  //
  // The PATx objects shall take a single integer parameter, in tenths of degree Kelvin, which
  // represents the temperature at which the device should notify the participant driver of
  // an auxiliary trip event. A PATx control method returns no value.
  //
  //  Arguments: (1)
  //    Arg0 - temperature in tenths of degree Kelvin
  //  Return Value:
  //    None
  //
  Method(PAT1, 1, Serialized)
  {
    If (\ECON)
    {
      Store (Acquire(\_SB.PATM, 100),Local0)  // save Acquire result so we can check for Mutex acquired
      If (LEqual(Local0, Zero))   // check for Mutex acquired
      {
        Store(Divide(Subtract(Arg0,2732),10), \_SB.PCI0.LPCB.H_EC.TSHT)
        Store(2, \_SB.PCI0.LPCB.H_EC.TSI)
        Store(0x2, \_SB.PCI0.LPCB.H_EC.HYST)
        \_SB.PCI0.LPCB.H_EC.ECMD(0x4A)            //Set Trip point.
        Release(\_SB.PATM)
      }
    }
  }

  // Thermal Sensor Hysteresis, 2 degrees
  Name(GTSH, 20)

  // _CRT (Critical Temperature)
  //
  // This object, when defined under a thermal zone, returns the critical temperature at which OSPM must shutdown the system.
  //
  //  Arguments: (0)
  //    None
  //  Return Value:
  //    An Integer containing the critical temperature threshold in tenths of degrees Kelvin
  //
  Method(_CRT,0,Serialized)
  {
    Return(Add(2732,Multiply(GCR2,10)))
  }

  // _HOT (Hot Temperature)
  //
  // This optional object, when defined under a thermal zone, returns the critical temperature
  //  at which OSPM may choose to transition the system into the S4 sleeping state.
  //
  //  Arguments: (0)
  //    None
  //  Return Value:
  //    The return value is an integer that represents the critical sleep threshold tenths of degrees Kelvin.
  //
  Method(_HOT,0,Serialized)
  {
    Return(Add(2732,Multiply(Subtract(GCR2, 3),10)))
  }

  // _PSV (Passive)
  //
  // This optional object, if present under a thermal zone, evaluates to the temperature
  //  at which OSPM must activate passive cooling policy.
  //
  //  Arguments: (0)
  //    None
  //  Return Value:
  //    An Integer containing the passive cooling temperature threshold in tenths of degrees Kelvin
  //
  Method(_PSV,0,Serialized)
  {
    If (CTYP)
    {
      //Active
      Return(Add(2732,Multiply(Subtract(PST2, 8),10)))
    } Else    //Passive
    {
      Return(Add(2732,Multiply(PST2,10)))
    }

  }

  // _SCP (Set Cooling Policy)
  //
  //  Arguments: (3)
  //    Arg0 - Mode An Integer containing the cooling mode policy code
  //    Arg1 - AcousticLimit An Integer containing the acoustic limit
  //    Arg2 - PowerLimit An Integer containing the power limit
  //  Return Value:
  //    None
  //
  //  Argument Information:
  //    Mode - 1 = Active, 0 = Passive
  //    Acoustic Limit - Specifies the maximum acceptable acoustic level that active cooling devices may generate.
  //    Values are 1 to 5 where 1 means no acoustic tolerance and 5 means maximum acoustic tolerance.
  //    Power Limit - Specifies the maximum acceptable power level that active cooling devices may consume.
  //    Values are from 1 to 5 where 1 means no power may be used to cool and 5 means maximum power may be used to cool.
  //
  Method(_SCP, 3, Serialized)
  {
    If(LOr(LEqual(Arg0,0),LEqual(Arg0,1)))
    {
      Store(Arg0, CTYP)
      Notify(\_SB.STR2, 0x91)
    }
  }


  // SDBG (DPTF Super Debug option)
  //
  // This control method returns if Platform(BIOS) has enabled Super Debug or not
  //
  // Arguments: None
  // Return Value:
  //   0 - Disabled,
  //   1 - Enabled
  //
  Method(SDBG, 0)
  {
    Return(PDBG)
  }


} // End STR2
