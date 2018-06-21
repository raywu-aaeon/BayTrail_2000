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

  CpuDptf.asl

Abstract:

  Intel ACPI Reference Code for Intel Extended Thermal Model support
  on Intel Clovertrail Platforms
  Revision: See Relnotes.txt

--*/


DefinitionBlock (
  "CpuDptf.aml",
  "SSDT",
  1,
  "CpuDpf",
  "CpuDptf",
  0x1000
  )
{

  External(\_SB.CRTT, IntObj)
  External(\_SB.ACTT, IntObj)
  External(\_SB.PSVT, IntObj)

  External(\_PR.CPU0, DeviceObj)
  External(\_PR.CPU1, DeviceObj)
  External(\_PR.CPU2, DeviceObj)
  External(\_PR.CPU3, DeviceObj)

  External(\_SB.PDBG, IntObj)

  External(\_PR.CPU0._PSS, MethodObj)
  External(\_PR.CPU0.NPSS, PkgObj)
  External(\_PR.CPU0._PPC, IntObj)
  External(\_PR.CPU0._TSS, MethodObj)
  External(\_PR.CPU0._PTC, MethodObj)
  External(\_PR.CPU0._TSD, MethodObj)
  External(\_PR.CPU0._TPC, IntObj)
  External(\_PR.CPU0._TDL, MethodObj)

  External(\_SB.PCI0, DeviceObj)

  External(\_SB.DPTF, DeviceObj)
  External(\_SB.DPTF.CTOK, MethodObj)
  External(\_SB.DPSR, IntObj)
  External(\_SB.SDP1, IntObj)
  External(\_SB.DLPO, PkgObj)
  External(\_SB.STEP, IntObj)
  External(\_SB.MBID, DeviceObj)

  Scope(\_SB)
  {
    Device(TCPU)
    {
      External (PPCS)
      Name(_HID, EISAID("INT3401"))  // Intel Dptf Processor Device, 1 per package
      Name(_UID,0)
      Name(CTYP, 0x0) // Device specific cooling policy type
      Name(CINT,4)
      Name(LSTM,0)  // Last temperature reported
      Name(MED4, 0xE00000D4)
      Name(MED0, 0xE00000D0)

      //
      //  Application Exclusion list
      //  This object evaluates to a package of strings representing the application names that the
      //  DPTF processor participant will exclude from core off lining when LPO is triggered.
      //  Note: There is a limit of 256 strings for this package. If the list is a bigger number than
      //  the limit, it is not effective to use LPO.
      //
      Name(AEXL, Package()
      {
        "Svchost.exe",
        "dllhost.exe",
        "smss.exe",
        "WinSAT.exe"
      })

      // PPCC (Participant Power Control Capabilities)
      //
      // The PPCC object evaluates to a package of packages that indicates to DPTF processor
      // participant the power control capabilities.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   PPCC package of packages
      //
      Method(PPCC,0,Serialized,,PkgObj)
      {
        If(LEqual(SDP1,4))
        {
          Return(PPC1)            // N2805 fanless 10"
        }
        ElseIf (LLess(SDP1,4))
        {
          Return(PPC2)            // all other mobile SKUs
        }
        Else
        {
          Return(PPC3)            // all desktops
        }
      }

      Name (PPC1, Package()
      {
        0x2, //Revision

        Package ()   //Power Limit 1
        {
          0,    //PowerLimitIndex, 0 for Power Limit 1
          3000, //PowerLimitMinimum in mW
          4300, //PowerLimitMaximum
          1000, //TimeWindowMinimum
          1000, //TimeWindowMaximum
          200   //StepSize
        },

        Package ()   //Power Limit 2
        {
          1,    //PowerLimitIndex, 1 for Power Limit 2
          8000, //PowerLimitMinimum
          8000, //PowerLimitMaximum
          1000, //TimeWindowMinimum
          1000, //TimeWindowMaximum
          1000  //StepSize
        }
      })

      Name (PPC2, Package()
      {
        0x2, //Revision

        Package ()   //Power Limit 1
        {
          0,    //PowerLimitIndex, 0 for Power Limit 1
          4500, //PowerLimitMinimum in mW
          7500, //PowerLimitMaximum
          1000, //TimeWindowMinimum
          1000, //TimeWindowMaximum
          200   //StepSize
        },

        Package ()   //Power Limit 2
        {
          1,    //PowerLimitIndex, 1 for Power Limit 2
          8000, //PowerLimitMinimum
          8000, //PowerLimitMaximum
          1000, //TimeWindowMinimum
          1000, //TimeWindowMaximum
          1000  //StepSize
        }
      })

      Name (PPC3, Package()
      {
        0x2, //Revision

        Package ()   //Power Limit 1
        {
          0,    //PowerLimitIndex, 0 for Power Limit 1
          8500, //PowerLimitMinimum in mW
          8500, //PowerLimitMaximum
          1000, //TimeWindowMinimum
          1000, //TimeWindowMaximum
          200   //StepSize
        },

        Package ()   //Power Limit 2
        {
          1,    //PowerLimitIndex, 1 for Power Limit 2
          8000, //PowerLimitMinimum
          8000, //PowerLimitMaximum
          1000, //TimeWindowMinimum
          1000, //TimeWindowMaximum
          1000  //StepSize
        }
      })

      Name(CLPO,  Package()
      {
        0x1, // Revision
        0x0, // LPO Enable
        0x1, // LPO StartPState
        25,  // LPO StepSize
        0x1, // LPOPowerControlSetting, 1 - SMT, 2- Core offlinin
        0x1, // LPOPerformanceControlSetting, 1 - SMT, 2- Core offlinin
      })

      Method(_INI)
      {
        // Update CLPO parameters from BIOS setup
        Store(DeRefOf(Index(DLPO,1)), Index(CLPO,1))
        Store(DeRefOf(Index(DLPO,2)), Index(CLPO,2))
        Store(DeRefOf(Index(DLPO,3)), Index(CLPO,3))
        Store(DeRefOf(Index(DLPO,4)), Index(CLPO,4))
        Store(DeRefOf(Index(DLPO,5)), Index(CLPO,5))
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
        If(LEqual(DPSR,0))
        {
          Return(0x00)
        }
        Return(0x0F)
      }

      External(\PUNB)

      Method (_CRS, 0, Serialized)
      {

        Name (ABUF, ResourceTemplate ()
        {
          Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, ,, ) {86}
        })

        Name (BBUF, ResourceTemplate ()
        {
          Interrupt (ResourceConsumer, Level, ActiveLow, Exclusive, ,, ) {86}
          Memory32Fixed(ReadWrite,0xFED05000,0x800,PBAR)
        })

        CreateDwordField(BBUF, PBAR._BAS, PUNI)
        Store(PUNB, PUNI)

        If (LLessEqual(STEP, 0x04))
        {
          //A stepping
          Return(ABUF)
        } Else
        {
          //B settping and above
          Return(BBUF)
        }
      }

      // MBIW (Message Bus Write)
      //  Writes given data to specified port using MBI
      //
      // Arguments: (4)
      //  Arg0: Port number
      //  Arg1: Register Offset
      //  Arg2: Indicates write data type, 0-Byte, 1-Word, 2-DWord
      //  Arg3: Data to be written
      //
      // Return value: (0)
      //  None
      Method(MBIW,4, Serialized)
      {
        Store(Arg3, MED4)
        If(LEqual(Arg2, 0))
        {
          Store(0x10, Local1)
        }
        ElseIf (LEqual(Arg2, 1))
        {
          Store(0x30, Local1)
        }
        Else
        {
          Store(0xF0, Local1)
        }

        Or(ShiftLeft(Arg0, 16), ShiftLeft(Arg1, 8),local0)
        Or(Local0, Local1, local0)
        Or(Local0, 0x11000000, local0)

        Store(Local0, MED0)
      }

      // MBIR (Message Bus Interface Read)
      //  Reads data from specified port using MBI
      //
      // Arguments: (4)
      //  Arg0: Port number
      //  Arg1: Register Offset
      //  Arg2: Indicates data type, 0-Byte, 1-Word, 2-DWord
      //  Arg3: Data read
      //
      // Return value: (0)
      //  Data read will be stored in Arg3
      Method(MBIR,4, Serialized)
      {
        If(LEqual(Arg2, 0))
        {
          Store(0x10, Local1)
        }
        ElseIf (LEqual(Arg2, 1))
        {
          Store(0x30, Local1)
        }
        Else
        {
          Store(0xF0, Local1)
        }
        Or(ShiftLeft(Arg0, 16), ShiftLeft(Arg1, 8),local0)
        Or(Local0, Local1, local0)
        Or(Local0, 0x10000000, local0)

        Store(Local0, MED0)
        Store(MED4, Arg3)
      }

      // _PPC (Performance Present Capabilities)
      //
      // This optional object is a method that dynamically indicates to OSPM the number of performance states currently
      // supported by the platform.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   An Integer containing the range of states supported
      //   0 - States 0 through nth state are available (all states available)
      //   1 - States 1 through nth state are available
      //   2 - States 2 through nth state are available
      //   ...
      //   n - State n is available only
      //
      Method(_PPC,0)
      {
        Return (0)
      }

      // SPPC (Set Participant Performance Capability)
      //
      // SPPC is a control method object that takes one integer parameter that will indicate the maximum allowable
      // P-State for OSPM to use at any given time.
      //
      // Arguments: (1)
      //   Arg0 - integer
      // Return Value:
      //   None
      //
      Method(SPPC,1,Serialized)
      {
        Store ("cpudptf: SPPC Called", Debug)
        Store(Arg0, \_PR.CPU0._PPC) // Note: P000._PPC is an Integer not a Method

        Notify(\_PR.CPU0, 0x80)  // Tell P000 driver to re-eval _PPC
        Notify(\_PR.CPU1, 0x80)  // Tell P000 driver to re-eval _PPC
        Notify(\_PR.CPU2, 0x80)  // Tell P000 driver to re-eval _PPC
        Notify(\_PR.CPU3, 0x80)  // Tell P000 driver to re-eval _PPC
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
        Store(Arg0,LSTM)

        Notify(TCPU, 0x91) // notify the participant of a trip point change event

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

      // _PSS (Performance Supported States)
      //
      // This optional object indicates to OSPM the number of supported processor performance states that any given system can support.
      //
      // Arguments: (1)
      //   None
      // Return Value:
      //   A variable-length Package containing a list of Pstate sub-packages as described below
      //
      // Return Value Information
      //   Package {
      //   PState [0] // Package - Performance state 0
      //   ....
      //   PState [n] // Package - Performance state n
      //   }
      //
      // Stub for the Actual CPU _PSS method.
      //
      Method(_PSS,,,,PkgObj)
      {
        If(CondRefOf(\_PR.CPU0._PSS))
        {
          // Ensure _PSS is present
          Return(\_PR.CPU0._PSS())
        } Else
        {
          Return(Package() {Package(){0, 0, 0, 0, 0, 0}})
        }
      }

      // _TSS (Throttling Supported States)
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   A variable-length Package containing a list of Tstate sub-packages as described below
      //
      // Return Value Information
      //   Package {
      //   TState [0] // Package - Throttling state 0
      //   ....
      //   TState [n] // Package - Throttling state n
      //   }
      //
      Method(_TSS,,,,PkgObj)
      {
        If(CondRefOf(\_PR.CPU0._TSS))
        {
          // Ensure _TSS is present
          Return(\_PR.CPU0._TSS())
        } Else
        {
          Return(Package(){Package(){0, 0, 0, 0, 0}})
        }
      }

      // _TPC (Throttling Present Capabilities)
      //
      // This optional object is a method that dynamically indicates to OSPM the number of throttling states currently supported by the platform.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   An Integer containing the number of states supported:
      //   0 - states 0 .. nth state available (all states available)
      //   1 - state 1 .. nth state available
      //   2 - state 2 .. nth state available
      //   ...
      //   n ?state n available only
      //
      Method(_TPC)
      {
        If(CondRefOf(\_PR.CPU0._TPC))
        {
          // Ensure _TPC is present
          Return(\_PR.CPU0._TPC)
        } Else
        {
          Return(0)
        }
      }

      // _PTC (Processor Throttling Control)
      //
      // _PTC is an optional object that defines a processor throttling control interface alternative to the I/O address spaced-based P_BLK throttling control register (P_CNT)
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   A Package as described below
      //
      // Return Value Information
      //   Package {
      //     ControlRegister // Buffer (Resource Descriptor)
      //     StatusRegister // Buffer (Resource Descriptor)
      //   }
      //
      Method(_PTC,,,,PkgObj)
      {
        If(CondRefOf(\_PR.CPU0._PTC))
        {
          // Ensure _PTC is present
          Return(\_PR.CPU0._PTC())
        } Else
        {
          Return(Package(){
            Buffer(){0},
            Buffer(){0}
          })
        }
      }

      // _TSD (T-State Dependency)
      //
      // This optional object provides T-state control cross logical processor dependency information to OSPM.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   A Package containing T-state dependency information as described below
      //
      // Return Value Information
      //   Package {
      //     NumEntries    // Integer
      //     Revision      // Integer (BYTE)
      //     Domain        // Integer (DWORD)
      //     CoordType     // Integer (DWORD)
      //     NumProcessors // Integer (DWORD)
      //   }
      //
      Method(_TSD,,,,PkgObj)
      {
        If(CondRefOf(\_PR.CPU0._TSD))
        {
          // Ensure _TSD is present
          Return(\_PR.CPU0._TSD())
        } Else
        {
          Return(Package(){Package(){5, 0, 0, 0, 0}})
        }
      }

      // _TDL (T-state Depth Limit)
      //
      // This optional object evaluates to the _TSS entry number of the lowest power throttling state that OSPM may use.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   An Integer containing the Throttling Depth Limit _TSS entry number:
      //   0 - throttling disabled.
      //   1 - state 1 is the lowest power T-state available.
      //   2 - state 2 is the lowest power T-state available.
      //   ...
      //   n - state n is the lowest power T-state available.
      //
      Method(_TDL)
      {
        Store ("cpudptf: _TDL Called", Debug)
        If(CondRefOf(\_PR.CPU0._TDL))
        {
          Return(\_PR.CPU0._TDL())
        } Else
        {
          Return(0)
        }
      }

      // _PDL (P-state Depth Limit)
      //
      // This optional object evaluates to the _PSS entry number of the lowest performance P-state that OSPM may use when performing passive thermal control.
      //
      // Arguments: (0)
      //   None
      // Return Value:
      //   An Integer containing the P-state Depth Limit _PSS entry number:
      //   Integer containing the P-state Depth Limit _PSS entry number:
      //   0 - P0 is the only P-state available for OSPM use
      //   1 - state 1 is the lowest power P-state available
      //   2 - state 2 is the lowest power P-state available
      //   ...
      //   n - state n is the lowest power P-state available
      //
      Method(_PDL, 0, Serialized)
      {
        Store ("cpudptf: _PDL Called", Debug)
        If(CondRefOf(\_PR.CPU0._PSS))
        {
          Name ( LFMI, 0)
          Store (SizeOf(\_PR.CPU0._PSS()), LFMI)
          Decrement(LFMI)    // Index of LFM entry in _PSS
          Return(LFMI)
        } Else
        {
          Return(0)
        }
      }

      // DPPM related stuff
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
        Return(Add(2732,Multiply(CRTT,10)))
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
        Return(Add(2732,Multiply(Subtract(CRTT, 3),10)))
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
        If(CTYP)
        {
          Return(Add(2732,Multiply(ACTT,10)))  // Active Cooling Policy
        } Else
        {
          Return(Add(2732,Multiply(PSVT,10)))  // Passive Cooling Policy
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
          Notify(\_SB.TCPU, 0x91)
        }
      }
      // Hystersis, 2 Degree clesius
      Name(GTSH, 20)


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


    } // End Device(TCPU)
  } // End Scope(\_SB)
} // End SSDT





