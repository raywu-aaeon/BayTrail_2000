/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c) 1999 - 2014 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  Dptf.asl

Abstract:

  Intel ACPI Reference Code for Dynamic Platform & Thermal Framework
  on Intel Clovertrail Platforms
  Revision: See Relnotes.txt

--*/



DefinitionBlock (
  "Dptf.aml",
  "SSDT",
  1,
  "DptfTb",
  "DptfTab",
  0x1000
  )
{

  External(\_SB.PCI0, DeviceObj)
  External(\_SB.PCI0.GFX0, DeviceObj)
  External(\_SB.PCI0.GFX0.DD1F, DeviceObj)
  External(\_SB.PCI0.GFX0.DD1F._DCS, MethodObj)
  External(\_SB.PCI0.GFX0.DD1F._BCL, MethodObj)
  External(\_SB.PCI0.GFX0.DD1F._BQC, MethodObj)
  External(\_SB.PCI0.GFX0.DD1F._BCM, MethodObj)
  External(\_SB.TCPU, DeviceObj)
  External(\_SB.CRTT)
  External(\_SB.ACTT)
  External(\_SB.GCR0)
  External(\_SB.GCR1)
  External(\_SB.GCR2)
  External(\_SB.GCR3)
  External(\_SB.GCR4)
  External(\_SB.GCR5)
  External(\_SB.GCR6)
  External(\_SB.PST0)
  External(\_SB.PST1)
  External(\_SB.PST2)
  External(\_SB.PST3)
  External(\_SB.PST4)
  External(\_SB.PST5)
  External(\_SB.PST6)
  External(\_SB.PDBG, IntObj)

  External(\_SB.THM0, IntObj)
  External(\_SB.THM1, IntObj)
  External(\_SB.THM2, IntObj)

  External(\_SB.DPTE, IntObj)
  External(\_SB.SDP1, IntObj)
  External(\PWRS, IntObj)

  External(\_SB.PCI0.LPCB.H_EC, DeviceObj)

  External(ECON, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.TMPR, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.LTMP, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.ART1, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.ALT1, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.ART2, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.DM0T, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.D0TL, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.ECMD, MethodObj)
  External(\_SB.PCI0.LPCB.H_EC.BCLT, FieldUnitObj)

  External(\_SB.DSOC, IntObj)
  External(\_SB.CHGR, IntObj)
  External(\_SB.DDSP, IntObj)

  External(\_SB.MIPL, IntObj) // (828) PPCC Minimum Power Limit
  External(\_SB.MAPL, IntObj) // (830) PPCC Maximum Power Limit

  External(\ADBG, MethodObj)

  External(PWRE, IntObj)  // EnablePowerDevice
  External(\_SB.PCI0.LPCB.H_EC.BAT1._BST, MethodObj)

  External(\_SB.PCI0.LPCB.H_EC.ECAV, IntObj)
  External(\_SB.PCI0.LPCB.H_EC.B1RC, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.B1FC, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.B1ML, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.B1MH, FieldUnitObj)

  External(\_SB.PCI0.LPCB.H_EC.TSI, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.HYST, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.TSHT, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.TSLT, FieldUnitObj)
  External(\_SB.PCI0.LPCB.H_EC.TSSR, FieldUnitObj)

  External(\_PR.CPU0, DeviceObj)
  External(\_PR.CPU1, DeviceObj)
  External(\_PR.CPU2, DeviceObj)
  External(\_PR.CPU3, DeviceObj)

  External(\_PR.CPU0._PPC, IntObj)

  Name(BDLI, 2) // Brightness depth limit index - index 2 corresponds to 30%
  Name(BDHI, 12) // Brightness peformance/power limit (ceil) index - 12 corresponds to 80%

  Scope(\_SB)
  {

    //
    // DPTF Thermal Zone Device
    //
    //

    Device(DPTF)
    {
      //
      // Intel DPTF Thermal Framework Device
      //
      Name(_HID, EISAID("INT3400"))

      //
      // All Intel DPPM Supported Policies are loaded in this SSDT.
      // OEM's could separate these into individual SSDT's if desired.
      //
      Name (DPSP, Package()
      {
        //
        // DPPM Passive Policy
        //
        ToUUID("42A441D6-AE6A-462B-A84B-4A8CE79027D3"),
      })

      Name (DCSP, Package()
      {
        //
        // DPPM Crtical Policy
        //
        ToUUID("97C68AE7-15FA-499c-B8C9-5DA81D606E0A"),
      })

      Name (DCPP, Package()
      {
        //
        // DPPM Cooling policy
        //
        ToUUID("16CAF1B7-DD38-40ED-B1C1-1B8A1913D531"),
      })

      //
      // Note: there are four GUID packages in TMPP and four matching store statements in IDSP.
      // These must always match to prevent an overrun.
      //
      Name(TMPP,Package()
      {
        ToUUID("00000000-0000-0000-0000-000000000000"),
        ToUUID("00000000-0000-0000-0000-000000000000"),
        ToUUID("00000000-0000-0000-0000-000000000000"),
      })

      // IDSP (Intel DPTF Supported Policies)
      //
      // This object evaluates to a package of packages, with each package containing the UUID
      // values to represent a policy implemented and supported by the Intel DPTF software stack.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   Package of Guid packages
      //
      Method(IDSP, 0, Serialized, , PkgObj)
      {
        Name(TMPI,0)

        Store(DeRefOf(Index(DPSP,0)), Index(TMPP,TMPI))
        Increment(TMPI)

        Store(DeRefOf(Index(DCSP,0)), Index(TMPP,TMPI))
        Increment(TMPI)

        Store(DeRefOf(Index(DCPP,0)), Index(TMPP, TMPI))

        Return(TMPP)
      }

      // _STA (Status)
      //
      // This object returns the current status of a device.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   An Integer containing a device status bitmap:
      //    Bit 0 - Set if the device is present.
      //    Bit 1 - Set if the device is enabled and decoding its resources.
      //    Bit 2 - Set if the device should be shown in the UI.
      //    Bit 3 - Set if the device is functioning properly (cleared if device failed its diagnostics).
      //    Bit 4 - Set if the battery is present.
      //    Bits 5-31 - Reserved (must be cleared).
      //
      Method(_STA)
      {
        If (LEqual(DPTE,1))
        {
          Return(0x0F)
        } Else
        {
          Return(0x00)
        }
      }

      Name(PDRI,Package()
      {
        Package(){10, \_SB.TCPU, 0x0, Package(){0x00010002, 0}} //Turbo
      })

      Name(PDR1,Package()
      {
        Package(){100, \_SB.TCPU, 0x0, Package(){0x00010000, 4000,    //PL1 N2805 fanless 10"
                                                 0x00010001, 8000  }},//PL2
        Package(){100, \_SB.TDSP, 0xA, Package(){0x00050000, 100}}    //Display brightness
      })

      // PDRT (power device relation table)
      //
      // This object evaluates to a package of packages that indicates the relation between charge rates and target devices.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   A variable-length package of packages as described below.
      //
      // Return Value Information
      //   Package {
      //   Package [0] // Package
      //   ....
      //   Package [n] // Package
      //   }
      //
      //   Each sub-Package contains the elements described below:
      //   Package {
      //     ChargeRate    // Integer (DWORD)
      //     TargetDevice  // Integer (DWORD)
      //     PTYP          // Integer (DWORD)
      //     ControlBuffer // Package
      //   }
      //
      Method(PDRT)
      {
        If(LEqual(PWRS, Zero))
        {
          Return(PDRI) // DC means not charging
        }
        Else
        {
          Store(MIPL,Index(DeRefOf(Index (DeRefOf (Index (PDR1, 0)),3)),1)) // Package domain, PL1
          Store(MAPL,Index(DeRefOf(Index (DeRefOf (Index (PDR1, 0)),3)),3)) // Package domain, PL2
          Return(PDR1)            // N2805 fanless 10"
        }
      }

      // ETRM (Intel DPTF Participant List)
      //
      // The ETRM object evaluates to a package of packages each of which describes one participant device.
      //
      Name(ETRM, Package()
      {
        Package() {\_SB.TCPU,  "INT3401",    0x06, "0"}, // Processor Device
        Package() {\_SB.STR0,  "INT3403",    0x06, "0"}, // CPU remote temp sensor, U1H5,ADT7421
        Package() {\_SB.STR1,  "INT3403",    0x06, "1"}, // CPU local temp sensor, U1H5,ADT7421
        Package() {\_SB.STR2,  "INT3403",    0x06, "2"}, // Ambient remote 1 temp sensor, U3A1,ADT7481
        Package() {\_SB.STR3,  "INT3403",    0x06, "3"}, // Ambient local temp sensor, U3A1,ADT7481
        Package() {\_SB.STR4,  "INT3403",    0x06, "4"}, // Ambient remote 2 temp sensor, U3A1,ADT7481
        Package() {\_SB.STR5,  "INT3403",    0x06, "5"}, // DDR3 On-board TS temp remote U2F1,ADM1032
        Package() {\_SB.STR6,  "INT3403",    0x06, "6"}, // DDR3 local temp U2F1,ADM1032
        Package() {\_SB.TDSP,  "INT3406",    0x06, "0"}, // Display participant
        Package() {\_SB.TCHG,  "INT3403",    0x06, "7"}, // charger participant
      })

      Name(TRTI, Package()
      {
        Package() {\_SB.TCPU, \_SB.TCPU, 100, 50, Zero, Zero, Zero, Zero},

        Package() {\_SB.TCPU, \_SB.STR0, 100, 50, Zero, Zero, Zero, Zero},
        Package() {\_SB.TCHG, \_SB.STR0, 80, 300, Zero, Zero, Zero, Zero},

        Package() {\_SB.TDSP, \_SB.STR1, 90, 300, Zero, Zero, Zero, Zero},
        Package() {\_SB.TCPU, \_SB.STR1, 100, 150, Zero, Zero, Zero, Zero},
        Package() {\_SB.TCHG, \_SB.STR1, 80, 300, Zero, Zero, Zero, Zero},

        Package() {\_SB.TCPU, \_SB.STR2, 100, 150, Zero, Zero, Zero, Zero},
        Package() {\_SB.TCHG, \_SB.STR2, 80, 300, Zero, Zero, Zero, Zero},

        Package() {\_SB.TCPU, \_SB.STR3, 100, 150, Zero, Zero, Zero, Zero},
        Package() {\_SB.TCHG, \_SB.STR3, 80, 300, Zero, Zero, Zero, Zero},

        Package() {\_SB.TCPU, \_SB.STR4, 100, 150, Zero, Zero, Zero, Zero},
        Package() {\_SB.TCHG, \_SB.STR4, 80, 300, Zero, Zero, Zero, Zero},

        Package() {\_SB.TCPU, \_SB.STR5, 100, 150, Zero, Zero, Zero, Zero},
        Package() {\_SB.TCHG, \_SB.STR5, 80, 300, Zero, Zero, Zero, Zero},

        Package() {\_SB.TCPU, \_SB.STR6, 100, 150, Zero, Zero, Zero, Zero},
        Package() {\_SB.TCHG, \_SB.STR6, 80, 300, Zero, Zero, Zero, Zero},
      })

      // _TRT (Thermal Relationship Table)
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   A variable-length Package containing a list of Thermal Relationship Packages as described below.
      //
      // Return Value Information
      //   Package {
      //   ThermalRelationship[0] // Package
      //    ...
      //   ThermalRelationship[n] // Package
      //   }
      //
      Method(_TRT)
      {
        Return(TRTI)
      } // End _TRT

      // TRTR
      // Evaluates to an integer value that defines the revision of the _TRT object. The following values are valid:
      //  0: Traditional TRT as defined by the ACPI Specification.
      //  1: Simplified TRT as defined in sections 15.2.3 and 15.2.5.2.
      //     All other values are invalid and are treated as failure and cause the Policy to fail to load.
      //
      //
      Name(TRTR, 1)

      // _OSC (Operating System Capabilities)
      //
      // This object is evaluated by each DPTF policy implementation to communicate to the platform of the existence and/or control transfer.
      //
      // Arguments: (4)
      //   Arg0 - A Buffer containing a UUID
      //   Arg1 - An Integer containing a Revision ID of the buffer format
      //   Arg2 - An Integer containing a count of entries in Arg3
      //   Arg3 - A Buffer containing a list of DWORD capabilities
      // Return Value:
      //   A Buffer containing a list of capabilities
      //
      Method(_OSC, 4,Serialized,,BuffObj, {BuffObj,IntObj,IntObj,BuffObj})
      {
        Name(NUMP,0)
        Name (UID2,ToUUID("FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF"))

        // Point to Status DWORD in the Arg3 buffer (STATUS)
        CreateDWordField(Arg3, 0, STS1)

        // Point to Caps DWORDs of the Arg3 buffer (CAPABILITIES)
        CreateDWordField(Arg3, 4, CAP1)

        //
        // _OSC needs to validate the UUID and Revision.
        //
        // IF Unrecognized UUID
        //  Return Unrecognized UUID _OSC Failure
        // IF Unsupported Revision
        //  Return Unsupported Revision _OSC Failure
        //
        //    STS0[0] = Reserved
        //    STS0[1] = _OSC Failure
        //    STS0[2] = Unrecognized UUID
        //    STS0[3] = Unsupported Revision
        //    STS0[4] = Capabilities masked
        //
        // Get the platform UUID's that are available, this will be a package of packages.
        //
        IDSP()                     // initialize TMPP with GUID's
        Store(SizeOf(TMPP),NUMP)   // how many GUID's in the package?

        // Note:  The comparison method used is necessary due to
        // limitations of certain OSes which cannot perform direct
        // buffer comparisons.
        //
        // Create a set of "Input" UUID fields.
        //
        CreateDWordField(Arg0, 0x0, IID0)
        CreateDWordField(Arg0, 0x4, IID1)
        CreateDWordField(Arg0, 0x8, IID2)
        CreateDWordField(Arg0, 0xC, IID3)
        //
        // Create a set of "Expected" UUID fields.
        //
        CreateDWordField(UID2, 0x0, EID0)
        CreateDWordField(UID2, 0x4, EID1)
        CreateDWordField(UID2, 0x8, EID2)
        CreateDWordField(UID2, 0xC, EID3)
        //
        // Compare the input UUID to the list of UUID's in the system.
        //
        While(NUMP)
        {
          //
          // copy one uuid from TMPP to UID2
          //
          Store(DeRefOf (Index (TMPP, Subtract(NUMP,1))),UID2)
          //
          // Verify the input UUID matches the expected UUID.
          //
          If(LAnd(LAnd(LEqual(IID0, EID0), LEqual(IID1, EID1)), LAnd(LEqual(IID2, EID2), LEqual(IID3, EID3))))
          {
            Break  // break out of while loop when matching UUID is found
          }
          Decrement(NUMP)
        }

        If(LEqual(NUMP,0))
        {
          //
          // Return Unrecognized UUID _OSC Failure
          //
          And(STS1,0xFFFFFF00,STS1)
          Or(STS1,0x6,STS1)
          Return(Arg3)
        }

        If(LNot(LEqual(Arg1, 1)))
        {
          //
          // Return Unsupported Revision _OSC Failure
          //
          And(STS1,0xFFFFFF00,STS1)
          Or(STS1,0xA,STS1)
          Return(Arg3)
        }

        If(LNot(LEqual(Arg2, 2)))
        {
          //
          // Return Argument 3 Buffer Count not sufficient
          //
          And(STS1,0xFFFFFF00,STS1)
          Or(STS1,0x2,STS1)
          Return(Arg3)
        }

        // TODO: disable/enable policy according to Arg3.DWORD2.Bit0

        Return(Arg3)
      } // _OSC

      // KTOC (Kelvin to Celsius)
      //
      // This control method converts from 10ths of degree Kelvin to Celsius.
      //
      // Arguments: (1)
      //   Arg0 - Temperature in 10ths of degree Kelvin
      // Return Value:
      //   Temperature in Celsius
      //
      Method(KTOC,1, Serialized)
      {
        Return(Divide(Subtract(Arg0,2732),10))
      }

      // CTOK (Celsius to Kelvin)
      //
      // This control method converts from Celsius to 10ths of degree Kelvin.
      //
      // Arguments: (1)
      //   Arg0 - Temperature in Celsius
      // Return Value:
      //   Temperature in 10ths of degree Kelvin
      //
      Method(CTOK, 1, Serialized)
      {
        Return(Add(2732,Multiply(Arg0,10)))
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


    } // End DPTF Device
  } // End \_SB Scope


  Scope(\_SB)
  {

    //
    // Create a Mutex for PATx methods to prevent Sx resume race condition problems asscociated with EC commands.
    //
    Mutex(PATM, 0)

    //
    // These devices are currently used by DPPM policies only.
    // Refer to the specific technology BIOS specification.
    //

    Include("CpuTempRemotePart.asl")
    Include("CpuTempLocalPart.asl")
    Include("AmbientTempRemote1Part.asl")
    Include("AmbientTempLocalPart.asl")
    Include("AmbientTempRemote2Part.asl")
    Include("DDRTempRemotePart.asl")
    Include("DDRTempLocalPart.asl")

    Device(TDSP)
    {
      Name(_HID, EISAID("INT3406"))  // Intel DPTF Display Device

      // _STA (Status)
      //
      // This object returns the current status of a device.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   An Integer containing a device status bitmap:
      //    Bit 0 - Set if the device is present.
      //    Bit 1 - Set if the device is enabled and decoding its resources.
      //    Bit 2 - Set if the device should be shown in the UI.
      //    Bit 3 - Set if the device is functioning properly (cleared if device failed its diagnostics).
      //    Bit 4 - Set if the battery is present.
      //    Bits 5-31 - Reserved (must be cleared).
      //
      Method(_STA)
      {
        If(LEqual(DDSP,0))
        {
          Return(0x00)
        }
        Return(0x0F)
      }
      // DDDL ( DPTF Display Depth Limit)
      //
      // The DDDL object indicates dynamically a lower limit on the brightness control levels currently supported by the platform
      // for the participant. Value returned indicates a Power/Percentage value that is in the _BCL brightness list.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   Index in Brightness level list
      Method(DDDL, 0)
      {
        Return(BDLI)
      }

      // DDPC ( DPTF Display Power/Performance Control)
      //
      // The DDPC object indicates dynamically a higher limit (ceiling) on the brightness control levels currently supported by
      // the platform for the participant.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   Index in Brightness level list
      Method(DDPC, 0)
      {
        Return(BDHI)
      }

      // Query List of Brightness Control Levels Supported.
      Method(_BCL,0)
      {
        // List of supported brightness levels in the following sequence.

        // Level when machine has full power.
        // Level when machine is on batteries.
        // Other supported levels.

        Return(Package() {80, 50, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100})
      }

      // Set the Brightness Level.
      Method (_BCM,1)
      {

        // Set the requested level if it is between 0 and 100%.
        If(LAnd(LGreaterEqual(Arg0,0),LLessEqual(Arg0,100)))
        {
          // Call Graphics device's BCM method to set brightness
          \_SB.PCI0.GFX0.DD1F._BCM(Arg0)
        }
      }

      // Brightness Query Current level.
      Method (_BQC,0)
      {
        Return(\_SB.PCI0.GFX0.DD1F._BQC())
      }

      Method (_DCS, 0)
      {
        Return(\_SB.PCI0.GFX0.DD1F._DCS())
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


    } // End TDSP Device

    Device(TCHG)
    {
      Name(_HID, EISAID("INT3403"))   // Intel DPTF Charger Participant
      Name(_UID, 7)
      Name(PTYP, 0x0B)
      Name(_STR, Unicode ("Intel DPTF Charger Participant"))

      // _STA (Status)
      //
      // This object returns the current status of a device.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   An Integer containing a device status bitmap:
      //    Bit 0 - Set if the device is present.
      //    Bit 1 - Set if the device is enabled and decoding its resources.
      //    Bit 2 - Set if the device should be shown in the UI.
      //    Bit 3 - Set if the device is functioning properly (cleared if device failed its diagnostics).
      //    Bit 4 - Set if the battery is present.
      //    Bits 5-31 - Reserved (must be cleared).
      //
      Method(_STA)
      {
        If(LEqual(CHGR,0))
        {
          Return(0x00)
        }
        Return(0x0F)
      }

      //limit from 1460ma to 3572ma for beta qulity test.
      Name (PPSS, Package()   // Participant Performance Supported States
      {
        Package ()            // three Power/Performance State Data
        {
          0x00000000,         //N/A Performance
          0x00000000,         //N/A Power
          0x00000000,         //N/A TransitionLatency
          0x00000000,         //N/A Linear
          0x00000000,         //Control
          0x00000880,         //RawPerformance //2.1A
          "mA",               //RawUnit
          0x00000000,         //Reserved
        },
        Package ()            // four Power/Performance State Data
        {
          0x00000000,         //N/A Performance
          0x00000000,         //N/A Power
          0x00000000,         //N/A TransitionLatency
          0x00000000,         //N/A Linear
          0x00000001,         //Control
          0x00000800,         //RawPerformance //2.0A
          "mA",               //RawUnit
          0x00000000,         //Reserved
        },
        Package ()            // six Power/Performance State Data
        {
          0x00000000,         //N/A Performance
          0x00000000,         //N/A Power
          0x00000000,         //N/A TransitionLatency
          0x00000000,         //N/A Linear
          0x00000002,         //Control
          0x00000600,         //RawPerformance //1.5A
          "mA",               //RawUnit
          0x00000000,         //Reserved
        },
        Package ()            // six Power/Performance State Data
        {
          0x00000000,         //N/A Performance
          0x00000000,         //N/A Power
          0x00000000,         //N/A TransitionLatency
          0x00000000,         //N/A Linear
          0x00000003,         //Control
          0x00000400,         //RawPerformance //1.0A
          "mA",               //RawUnit
          0x00000000,         //Reserved
        },
        Package ()            // six Power/Performance State Data
        {
          0x00000000,         //N/A Performance
          0x00000000,         //N/A Power
          0x00000000,         //N/A TransitionLatency
          0x00000000,         //N/A Linear
          0x00000004,         //Control
          0x00000200,         //RawPerformance //0.5A
          "mA",               //RawUnit
          0x00000000,         //Reserved
        },
        Package ()            // six Power/Performance State Data
        {
          0x00000000,         //N/A Performance
          0x00000000,         //N/A Power
          0x00000000,         //N/A TransitionLatency
          0x00000000,         //N/A Linear
          0x00000005,         //Control
          0x00000000,         //RawPerformance //0A
          "mA",               //RawUnit
          0x00000000,         //Reserved
        }
      }) // End of PPSS object

      Method(PPPC, 0)         // Performance Present Capabilities method
      {
        //Locals definition
        // local1 _PSR method result
        // local2 PPSS index

        Store(SizeOf(PPSS), local2)
        // Convert size to index
        Decrement(local2)

        // Check if the charging is disabled, in this case return the last power state
        If(LEqual(PWRS, Zero))
        {
          Return(local2)
        }
        Else
        {
          // return the highest state (zero)
          Return(0)
        }
      } // End of PPPC Method

    // SPPC (Set Participant Performance Capability)
    //
    // SPPC is a control method object that takes one integer parameter that will indicate the maximum 
    // allowable P-State for OSPM to use at any given time.
    //
    // Arguments: (1)
    //   Arg0 - integer
    // Return Value:
    //   None
    //
    Method(SPPC,1,Serialized)
    {
      // bios translates P-state to charge amps and writes CMD/DATA to EC
      If(LLessEqual(ToInteger(Arg0), Subtract(SizeOf(PPSS),1))){ // bounds check requested P-state
        Store(DeRefOf(Index (DeRefOf (Index (PPSS, Arg0)), 5)),Local1) // get Charge Rate amp value
        Store(Local1, \_SB.PCI0.LPCB.H_EC.BCLT) // write Charge Rate amp value to EC
        \_SB.PCI0.LPCB.H_EC.ECMD(0x12) // Set Charge Rate
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


    } // End TCHG Device

  } // End \_SB

  //
  // EC support code
  //
  Scope(\_SB.PCI0.LPCB.H_EC)   // Open scope to Embedded Controller
  {

    // _QF1 (Query - Embedded Controller Query F1)
    //
    // Handler for EC generated SCI number F1.
    //
    // Arguments: (0)
    //   None
    // Return Value:
    //   None
    //
    Method(_QF1)
    {
      // Thermal sensor threshold crossing event handler
      Store(\_SB.PCI0.LPCB.H_EC.TSSR, Local0)
      While(Local0)   // Ensure that events occuring during execution
      {
        // of this handler are not dropped
        Store(0, \_SB.PCI0.LPCB.H_EC.TSSR) // clear all status bits
        If(And(Local0, 0x40))
        {
          // DDR3 local temp threshold crossed
          Notify(\_SB.STR6, 0x90)
        }
        If(And(Local0, 0x20))
        {
          // DDR3 remote temp threshold crossed
          Notify(\_SB.STR5, 0x90)
        }
        If(And(Local0, 0x10))
        {
          // Ambient remote 2 temp threshold crossed
          Notify(\_SB.STR4, 0x90)
        }
        If(And(Local0, 0x8))
        {
          // Ambient local temp threshold crossed
          Notify(\_SB.STR3, 0x90)
        }
        If(And(Local0, 0x4))
        {
          // Ambient remote 1 temp threshold crossed
          Notify(\_SB.STR2, 0x90)
        }
        If(And(Local0, 0x2))
        {
          // CPU local temp threshold crossed
          Notify(\_SB.STR1, 0x90)
        }
        If(And(Local0, 0x1))
        {
          // CPU remote temp threshold crossed
          Notify(\_SB.STR0, 0x90)
        }
        Store(\_SB.PCI0.LPCB.H_EC.TSSR, Local0)
      }
    }

  } // End \_SB.PCI0.LPCB.H_EC Scope

} // End SSDT

