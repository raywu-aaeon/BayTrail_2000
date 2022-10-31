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


// Define the following External variables to prevent a WARNING when
// using ASL.EXE and an ERROR when using IASL.EXE.

External(PDC0)
External(PDC1)
External(PDC2)
External(PDC3)
External(CFGD)
External(\_PR.CPU0._PPC, IntObj)
External(\_SB.PCI0.LPCB.TPM.PTS, MethodObj)
External(\_SB.STR3, DeviceObj)
//External(\_SB.STR3.ISRH, MethodObj)
External(\_SB.I2C1.BATC, DeviceObj)
External(\_SB.DPTF, DeviceObj)
External(\_SB.TCHG, DeviceObj)
External(\_SB.IAOE.PTSL)
External(\_SB.IAOE.WKRS)
External(\_TZ.TZ01, DeviceObj)  // EIP163305+

//
// Create a Global MUTEX.
//
Mutex(MUTX,0)

// Define Port 80 as an ACPI Operating Region to use for debugging.  Please
// note that the Intel CRBs have the ability to ouput an entire DWord to
// Port 80h for debugging purposes, so the model implemented here may not be
// able to be used on OEM Designs.

OperationRegion(PRT0,SystemIO,0x80,4)
Field(PRT0,DwordAcc,Lock,Preserve)
{
  P80H, 32
}

// Port 80h Update:
//              Update 8 bits of the 32-bit Port 80h.
//
//      Arguments:
//              Arg0:   0 = Write Port 80h, Bits 7:0 Only.
//                      1 = Write Port 80h, Bits 15:8 Only.
//                      2 = Write Port 80h, Bits 23:16 Only.
//                      3 = Write Port 80h, Bits 31:24 Only.
//              Arg1:   8-bit Value to write
//
//      Return Value:
//              None

Method(P8XH,2,Serialized)
{
  If(LEqual(Arg0,0))            // Write Port 80h, Bits 7:0.
  {
    Store(Or(And(P80D,0xFFFFFF00),Arg1),P80D)
  }

  If(LEqual(Arg0,1))            // Write Port 80h, Bits 15:8.
  {
    Store(Or(And(P80D,0xFFFF00FF),ShiftLeft(Arg1,8)),P80D)
  }

  If(LEqual(Arg0,2))            // Write Port 80h, Bits 23:16.
  {
    Store(Or(And(P80D,0xFF00FFFF),ShiftLeft(Arg1,16)),P80D)
  }

  If(LEqual(Arg0,3))            // Write Port 80h, Bits 31:24.
  {
    Store(Or(And(P80D,0x00FFFFFF),ShiftLeft(Arg1,24)),P80D)
  }

  Store(P80D,P80H)
}

//
// Define SW SMI port as an ACPI Operating Region to use for generate SW SMI.
//
OperationRegion (SPRT, SystemIO, 0xB2, 2)
Field (SPRT, ByteAcc, Lock, Preserve)
{
  SSMP, 8
}

// The _PIC Control Method is optional for ACPI design.  It allows the
// OS to inform the ASL code which interrupt controller is being used,
// the 8259 or APIC.  The reference code in this document will address
// PCI IRQ Routing and resource allocation for both cases.
//
// The values passed into _PIC are:
//       0 = 8259
//       1 = IOAPIC

Method(\_PIC,1)
{
  Store(Arg0,GPIC)
  Store(Arg0,PICM)
}

OperationRegion(SWC0, SystemIO, 0x610, 0x0F)
Field(SWC0, ByteAcc, NoLock, Preserve)
{
  G1S, 8,      //SWC GPE1_STS
  Offset(0x4),
  G1E, 8,
  Offset(0xA),
  G1S2, 8,     //SWC GPE1_STS_2
  G1S3, 8      //SWC GPE1_STS_3
}

//AMI_OVERRIDE - system resumed from S3 last time,then S4 sleep smi would not work - EIP130577 >>
OperationRegion (SWC1, SystemIO, \PMBS, 0x34)
Field(SWC1, DWordAcc, NoLock, Preserve)
{
  Offset(0x02), //AMI_OVERRIDE - EIP135835 S3 power button issue
  PM1E, 16,      //GPE0_STS //AMI_OVERRIDE - EIP135835 S3 power button issue
  Offset(0x20),
  G0S, 32,      //GPE0_STS
  Offset(0x28),
  G0EN, 32,      //GPE0_EN
  Offset(0x30),
  SSMI, 32      //SMI_EN
}
//AMI_OVERRIDE - system resumed from S3 last time,then S4 sleep smi would not work - EIP130577 <<

// Prepare to Sleep.  The hook is called when the OS is about to
// enter a sleep state.  The argument passed is the numeric value of
// the Sx state.

//AMI_OVERRIDE >>
#ifdef AMI_ACPI_SUPPORT
Method(PPTS,1)
#else
Method(_PTS,1)
#endif
//AMI_OVERRIDE <<
{
  Store(SSMI, SSEN) //AMI_OVERRIDE - EIP130577 system resumed from S3 last time,then S4 sleep smi would not work - 
  Store(PM1E, SPM1) //AMI_OVERRIDE - EIP135835 S3 power button issue
  Store(0,P80D)         // Zero out the entire Port 80h DWord.
  P8XH(0,Arg0)          // Output Sleep State to Port 80h, Byte 0.

  If (LEqual(OSEL, 1))
  {
    If(LEqual(\_SB.PWRB.UPPS, 0x1))
    {
      Store(0, \_SB.PCI0.LPCB.H_EC.PB10)
    }
  }
  //clear the 3 SWC status bits
  Store(Ones, G1S3)
  Store(Ones, G1S2)
  Store(1, G1S)

  //set SWC GPE1_EN
  Store(1,G1E)

  //clear GPE0_STS
  Store(Ones, G0S)

  //
  // Save Sleep state if iSCT is present
  //
  If(LEqual(And(ICNF,0x01), 0x01))
  {
    Store(Arg0, \_SB.IAOE.PTSL)
  }
  // Update Lid state befor S3 or S4 resume
  Store(\_SB.PCI0.LPCB.H_EC.LSTE, LIDS)

  Store (LIDS, LLID)
  If(LEqual(Arg0,3))   // If S3 Resume
  {
    Or (LLID, 0x80, LLID)           // set S3 flag
  }
  

  //
  // Check whether TPM Module support is included in the project or not
  //
  If(CondRefOf(TCGM))   // Check if TCGM defined
  {
    //
    // Call TPM PTS method
    //
    \_SB.PCI0.LPCB.TPM.PTS (Arg0)
  }

  If(LEqual(Arg0,3))   // If S3 Suspend
  {
    //
    // Disable Digital Thermal Sensor function when doing S3 suspend
    //
    If(CondRefOf(DTSE))
    {
      If(LGreaterEqual(DTSE, 0x01))
      {
        Store(30, DTSF) // DISABLE_UPDATE_DTS_EVERY_SMI
        Store(0xD0, SSMP) // DTS SW SMI
      }
    }
  }
}

// Wake.  This hook is called when the OS is about to wake from a
// sleep state.  The argument passed is the numeric value of the
// sleep state the system is waking from.
Name(LLID, 0)           ///////////// dbg [a]
Name (WAKS, 0)          // Wake Status
//AMI_OVERRIDE >>
#ifdef AMI_ACPI_SUPPORT
Method(PWAK,1,Serialized)
#else
Method(_WAK,1,Serialized)
#endif
//AMI_OVERRIDE <<
{
  Store(SSEN, SSMI) //AMI_OVERRIDE - EIP130577 system resumed from S3 last time,then S4 sleep smi would not work
  Store(SPM1, PM1E) //AMI_OVERRIDE - EIP135835 S3 power button issue
  Store (Arg0, WAKS)
  P8XH(1,0xAB) // Beginning of _WAK.

  If(NEXP)
  {
    // Reinitialize the Native PCI Express after resume
    If(And(OSCC,0x02))
    {
      \_SB.PCI0.NHPG()
    }

    If(And(OSCC,0x04)) // PME control granted?
    {
      \_SB.PCI0.NPME()
    }
  }

// AMI_OVERRIDE - Fixed S3 and S4 resume fail when ISCT disable. >>
  If(LEqual(And(ICNF,0x01), 0x01)) {
    Notify(\_SB.IAOE, 0x00)
  }
// AMI_OVERRIDE - Fixed S3 and S4 resume fail when ISCT disable. <<

  If(LEqual(PFLV,FMBL))
  {
    Store(1,ECON)

    // Update Lid state after S3 or S4 resume
    Store(\_SB.PCI0.LPCB.H_EC.LSTE, LIDS)

    Store (LIDS, LLID)
    If(LEqual(Arg0,3))   // If S3 Resume
    {
      Or (LLID, 0x80, LLID)           // set S3 flag
    }

    If(IGDS)
    {
      If (LEqual(LIDS, 0))
      {
        Store(0x80000000,\_SB.PCI0.GFX0.CLID)
      }
      If (LEqual(LIDS, 1))
      {
        Store(0x80000003,\_SB.PCI0.GFX0.CLID)
      }
    }
#if defined ASL_SX_NOTIFY_LID0 && ASL_SX_NOTIFY_LID0 == 1 //AMI_OVERRIDE - EIP145306 After wake on lan , panel should keep black screen
    Notify(\_SB.LID0,0x80)
#endif //AMI_OVERRIDE - EIP145306 After wake on lan , panel should keep black screen
    //
    // if battery has changed from previous state after wake up / PowerOn, then update the Power State
    //
    Store(0,BNUM)
    Or(BNUM,ShiftRight(And(\_SB.PCI0.LPCB.H_EC.B1ST,0x08),3),BNUM)

    If(LEqual(BNUM,0))
    {
      If(LNotEqual(\_SB.PCI0.LPCB.H_EC.VPWR,PWRS))
      {
        Store(\_SB.PCI0.LPCB.H_EC.VPWR,PWRS)
        // Perform needed ACPI Notifications.
        PNOT()
      }
    }
    Else
    {
      If(LNotEqual(\_SB.PCI0.LPCB.H_EC.RPWR,PWRS))
      {
        Store(\_SB.PCI0.LPCB.H_EC.RPWR,PWRS)
        // Perform needed ACPI Notifications.
        PNOT()
      }
    }

  }

  If(LOr(LEqual(Arg0,3), LEqual(Arg0,4)))   // If S3 or S4 Resume
  {
    If(LEqual(PFLV,FMBL))
    {
      If(LEqual(Arg0,3))
      {
        If(LEqual(And(ICNF, 0x01), 0x01))
        {
          If(And(\_SB.PCI0.GFX0.TCHE, 0x100))
          {
            Store(And(\_SB.PCI0.LPCB.H_EC.S3WR, 0x03), Local0)
            If(LOr(And(Local0, 0x02), And(\_SB.IAOE.WKRS, 0x08)))
            {
              Or(\_SB.PCI0.GFX0.STAT, 0x01,\_SB.PCI0.GFX0.STAT)       // Update Gfx driver to turn off display
            }
            Else
            {
              And(\_SB.PCI0.GFX0.STAT, 0xFFFFFFFC,\_SB.PCI0.GFX0.STAT)
            }
          }

        }
      }
      If(LEqual(Arg0,4))
      {
        If(LEqual(And(ICNF,0x01), 0x01))
        {
          Or(\_SB.PCI0.GFX0.PCON, 0x60, \_SB.PCI0.GFX0.PCON)
        }
        Else
        {
          And(\_SB.PCI0.GFX0.PCON, 0xFFFFFF9F, \_SB.PCI0.GFX0.PCON)
        }
        // Perform needed ACPI Notifications.
        PNOT()
      }

      //
      // Enable Digital Thermal Sensor function after resume from S3
      //
      If(CondRefOf(DTSE))
      {
        If(LGreaterEqual(DTSE, 0x01))
        {
          Store(20, DTSF) // INIT_DTS_FUNCTION_AFTER_S3
          Store(0xD0, SSMP) // DTS SW SMI
          // EIP163305 >>
          If(CondRefOf(\_TZ.TZ01)) {
            Notify(\_TZ.TZ01,0x80)
          }
          // EIP163305 <<
        }
      }
    }

    // If CMP is enabled, we may need to restore the C-State and/or
    // P-State configuration, as it may have been saved before the
    // configuration was finalized based on OS/driver support.
    //
    //   CFGD[24]  = Two or more cores enabled
    //
    If(And(CFGD,0x01000000))
    {
      //
      // If CMP and the OSYS is WinXP SP1, we will enable C1-SMI if
      // C-States are enabled.
      //
      //   CFGD[7:4] = C4, C3, C2, C1 Capable/Enabled
      //
      //
    }

    // Windows XP SP2 does not properly restore the P-State
    // upon resume from S4 or S3 with degrade modes enabled.
    // Use the existing _PPC methods to cycle the available
    // P-States such that the processor ends up running at
    // the proper P-State.
    //
    // Note:  For S4, another possible W/A is to always boot
    // the system in LFM.
    //
    If(LEqual(OSYS,2002))
    {
      If(And(CFGD,0x01))
      {
        If(LGreater(\_PR.CPU0._PPC,0))
        {
          Subtract(\_PR.CPU0._PPC,1,\_PR.CPU0._PPC)
          PNOT()
          Add(\_PR.CPU0._PPC,1,\_PR.CPU0._PPC)
          PNOT()
        }
        Else
        {
          Add(\_PR.CPU0._PPC,1,\_PR.CPU0._PPC)
          PNOT()
          Subtract(\_PR.CPU0._PPC,1,\_PR.CPU0._PPC)
          PNOT()
        }
      }
    }
  }
#ifdef WIN7_SUPPORT
  If(LOr(LEqual(Arg0,3), LEqual(Arg0,4)))  // If S3 or S4 Resume
  {
    //
    // To support Win8, RapidStart resume from G3 and resume from DeepSx state
    //
    \_SB.PCI0.XHC1.XWAK()
  }
#endif
  
#ifndef AMI_ACPI_SUPPORT //AMI_OVERRIDE
  Return(Package() {0,0})
#endif //AMI_OVERRIDE
}

/*
// Get Buffer:
//              This method will take a buffer passed into the method and
//              create then return a smaller buffer based on the pointer
//              and size parameters passed in.
//
//      Arguments:
//              Arg0:   Pointer to start of new Buffer in passed in Buffer.
//              Arg1:   Size of Buffer to create.
//              Arg2:   Original Buffer
//
//      Return Value:
//              Newly created buffer.

Method(GETB,3,Serialized)
{
  Multiply(Arg0,8,Local0)                       // Convert Index.
  Multiply(Arg1,8,Local1)                       // Convert Size.
  CreateField(Arg2,Local0,Local1,TBF3)  // Create Buffer.

  Return(TBF3)                          // Return Buffer.
}
*/
// Power Notification:
//              Perform all needed OS notifications during a
//              Power Switch.
//
//      Arguments:
//              None
//
//      Return Value:
//              None

Method(PNOT,0,Serialized)
{
  // If MP enabled and driver support is present, notify both
  // processors.

  If(MPEN)
  {
    If(And(PDC0,0x0008))
    {
      Notify(\_PR.CPU0,0x80)    // Eval CPU0 _PPC.

      If(And(PDC0,0x0010))
      {
        Sleep(100)
        Notify(\_PR.CPU0,0x81)  // Eval _CST.
      }
    }

    If(And(PDC1,0x0008))
    {
      Notify(\_PR.CPU1,0x80)    // Eval CPU1 _PPC.

      If(And(PDC1,0x0010))
      {
        Sleep(100)
        Notify(\_PR.CPU1,0x81)  // Eval _CST.
      }
    }

    If(And(PDC2,0x0008))
    {
      Notify(\_PR.CPU2,0x80)    // Eval CPU2 _PPC.

      If(And(PDC2,0x0010))
      {
        Sleep(100)
        Notify(\_PR.CPU2,0x81)  // Eval _CST.
      }
    }

    If(And(PDC3,0x0008))
    {
      Notify(\_PR.CPU3,0x80)    // Eval CPU3 _PPC.

      If(And(PDC3,0x0010))
      {
        Sleep(100)
        Notify(\_PR.CPU3,0x81)  // Eval _CST.
      }
    }
  }
  Else
  {
    Notify(\_PR.CPU0,0x80)      // Eval _PPC.
    Sleep(100)
    Notify(\_PR.CPU0,0x81)      // Eval _CST
  }

  // Update the Battery 1 Stored Capacity and Stored Status.
  // Battery 0 information is always accurrate.

  If(LEqual(PFLV,FMBL))
  {
    Store(\_SB.PCI0.LPCB.H_EC.B1CC,B1SC)
    Store(\_SB.PCI0.LPCB.H_EC.B1ST,B1SS)

    // Perform update to all Batteries in the System.
    If(LGreaterEqual(OSYS,2006))    // Vista and Win7 later on OS
    {
      Notify(\_SB.PCI0.LPCB.H_EC.BAT0,0x81)       // Eval BAT0 _BIF.
      Notify(\_SB.PCI0.LPCB.H_EC.BAT1,0x81)       // Eval BAT1 _BIF.
    }
    Else
    {
      Notify(\_SB.PCI0.LPCB.H_EC.BAT0,0x80)       // Eval BAT0 _BST.
      Notify(\_SB.PCI0.LPCB.H_EC.BAT1,0x80)       // Eval BAT1 _BST.
    }
  }
  If (LAnd(LEqual(1,DPTE),LEqual(1,CHGR))){
    Notify(\_SB.TCHG, 0x80) // Notification sent to DPTF Charger participant driver for reevaluation after AC/DC transtion has occurred. 
  }

}

//
// System Bus
//
Scope(\_SB)
{
  Name(CRTT, 110) // Processor critical temperature
  Name(ACTT, 77)  // Active temperature limit for processor participant
  Name(GCR0, 70)  // Critical temperature for Generic participant 0 in degree celsius
  Name(GCR1, 70)  // Critical temperature for Generic participant 1 in degree celsius
  Name(GCR2, 70)  // Critical temperature for Generic participant 2 in degree celsius
  Name(GCR3, 70)  // Critical temperature for Generic participant 3 in degree celsius
  Name(GCR4, 70)  // Critical temperature for Generic participant 4 in degree celsius
  Name(GCR5, 70)  // Critical temperature for Generic participant 5 in degree celsius
  Name(GCR6, 70)  // Critical temperature for Generic participant 6 in degree celsius
  Name(PST0, 60)  // Passive temperature limit for Generic Participant 0 in degree celsius
  Name(PST1, 60)  // Passive temperature limit for Generic Participant 1 in degree celsius
  Name(PST2, 60)  // Passive temperature limit for Generic Participant 2 in degree celsius
  Name(PST3, 60)  // Passive temperature limit for Generic Participant 3 in degree celsius
  Name(PST4, 60)  // Passive temperature limit for Generic Participant 4 in degree celsius
  Name(PST5, 60)  // Passive temperature limit for Generic Participant 5 in degree celsius
  Name(PST6, 60)  // Passive temperature limit for Generic Participant 6 in degree celsius
  Name(PDBG, 0)   // DPTF Super debug option
  Name(PDPM, 1)   // DPTF DPPM enable
  Name(DLPO, Package()
  {
    0x1, // Revision
    0x1, // LPO Enable
    0x1, // LPO StartPState
    25,  // LPO StepSize
    0x1, // LPOPowerControlSetting, 1 - SMT, 2- Core offlinin
    0x1, // LPOPerformanceControlSetting, 1 - SMT, 2- Core offlinin
  })
  Name(BRQD, 0x00) // This is used to determine if DPTF display participant requested Brightness level change
  // or it is from Graphics driver. Value of 1 is for DPTF else it is 0

// For HG Support
      Name(OSCI, 0)  // \_SB._OSC DWORD2 input
      Name(OSCO, 0)  // \_SB._OSC DWORD2 output

  Method(_INI,0)
  {
    // NVS has stale DTS data.  Get and update the values
    // with current temperatures.   Note that this will also
    // re-arm any AP Thermal Interrupts.
    // Read temperature settings from global NVS
    Store(DPCT, CRTT)
    Store(Subtract(DPPT, 8), ACTT)                      // Active Trip point = Passive trip point - 8
    Store(DGC0, GCR0)
    Store(DGC0, GCR1)
    Store(DGC1, GCR2)
    Store(DGC1, GCR3)
    Store(DGC1, GCR4)
    Store(DGC2, GCR5)
    Store(DGC2, GCR6)
    Store(DGP0, PST0)
    Store(DGP0, PST1)
    Store(DGP1, PST2)
    Store(DGP1, PST3)
    Store(DGP1, PST4)
    Store(DGP2, PST5)
    Store(DGP2, PST6)


    // Update DPTF Super Debug option
    Store(DDBG, PDBG)


    // Update DPTF LPO Options
    Store(LPOE, Index(DLPO,1))
    Store(LPPS, Index(DLPO,2))
    Store(LPST, Index(DLPO,3))
    Store(LPPC, Index(DLPO,4))
    Store(LPPF, Index(DLPO,5))
    Store(DPME, PDPM)
  }

// For HG Support
      //Arg0 -- A buffer containing UUID
      //Arg1 -- An Interger containing a Revision ID of the buffer format
      //Arg2 -- An interger containing a count of entries in Arg3
      //Arg3 -- A buffer containing a list of DWORD capacities
      Method(_OSC, 4, NotSerialized)
      {
        // Check for proper UUID
        If(LEqual(Arg0, ToUUID("0811B06E-4A27-44F9-8D60-3CBBC22E7B48")))
        {
          CreateDWordField(Arg3,0,CDW1)     //bit1,2 is always clear
          CreateDWordField(Arg3,4,CDW2)     //Table 6-147 from ACPI spec

          Store(CDW2, OSCI)                 // Save DWord2
          Or(OSCI, 0x4, OSCO)               // Only allow _PR3 support

          If(LNotEqual(Arg1,One))
          {
            Or(CDW1,0x08,CDW1)            // Unknown revision
          }

          If(LNotEqual(OSCI, OSCO))
          {
            Or(CDW1,0x10,CDW1)            // Capabilities bits were masked
          }

          Store(OSCO, CDW2)                 // Replace DWord2
          
          Store("_OSC Return value", Debug)
          Store(Arg3, Debug)
          
          Return(Arg3)
        } Else
        {
          Or(CDW1,4,CDW1)                   // Unrecognized UUID
          Return(Arg3)
        }
      }// End _OSC

  // Define a (Control Method) Power Button.
  Device(PWRB)
  {
    Name(_HID,EISAID("PNP0C0C"))

    // GPI_SUS0 = GPE16 = Waketime SCI.  The PRW isn't working when
    // placed in any of the logical locations ( PS2K, PS2M),
    // so a Power Button Device was created specifically
    // for the WAKETIME_SCI PRW.

    Name(_PRW, Package(){16,0}) //AMI_OVERRIDE - CRB is unstable for S4 wake automaiclly.
    //
    // Power button status flag
    //
    Name(PBST, 1)

    //
    // Up Press Register flag. Set when OS register to recieve the up press of the power button
    //
    Name(UPPS, 0)

    //
    // Status of Power Button Level when EC is in mode where SCI is sent for both press and release of power button
    //
    Name(PBLV, 0)

    Method(_STA, 0)
    {
      If (LAnd(LEqual(ECON,1), PBST)){ 
        Return(0x0F)
      }
      Return(0x00)
    }

    Method(PBUP, 0) {
      If(UPPS) {
        Notify(\_SB.PWRB, 0xC0) // Send release notification to Power Button device
      }
    }

    Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj}) {
      // Compare passed in UUID to supported UUID.
      If (LEqual(Arg0, ToUUID ("9C355BCB-35FA-44f7-8A67-447359C36A03")))
      {
        If (LEqual(0,ToInteger(Arg1)))        // Revision 0.
        {
          Switch (ToInteger(Arg2))            // Switch to Function Index.
           {
             //
             // Function 0, Query of supported functions.
             //

             Case (0)
             {
               Return (Buffer() {0x07}) // Supports Function 1,2.
             }

             //
             // Function 1, Windows Compatible Button Array Power Button Properties.
             //

             Case (1)
             {
               Store(0x1, UPPS) // OS has registered to recieve notify on release of power button
              //
              // Place EC into mode where SCI is sent for both press and release of power button
              //
              \_SB.PCI0.LPCB.H_EC.ECMD(0x74)
              Store(1, \_SB.PCI0.LPCB.H_EC.PB10)
              Return(0x00)

             }

             //
             // Function 2, Power Button Level.
             //
             Case (2) 
             {
               If (UPPS) { // If OS has registered to recieve notify on release of power button
                 If (LNot(PBLV)) {
                   Return(0x00) // Power button not pressed  
                 } Else {
                  Return(0x01) // Power button pressed 
                 }
               }
             }
           } // End Switch statement
        } // End Revision check
      } // End UUID check

      //
      // If the code falls through to this point, just return a buffer of 0.
      //
      Return (Buffer() {0x00})
    } // End _DSM Method
  }

  Device(SLPB)
  {
    Name(_HID, EISAID("PNP0C0E"))
  } // END SLPB
  
// Define a Lid Switch.
  Device(LID0)
  {
    Name(_HID,EISAID("PNP0C0D"))

    Method(_STA)
    {
      If(LEqual(ECON,1))
      {
        Return(0x0F)
      }
      Return(0x00)
    }

    Method(_LID,0)
    {
      // 0 = Closed, 1 = Open.

      Return(\_SB.PCI0.LPCB.H_EC.LSTE)
    }
  }


  Scope(PCI0)
  {
    Method(_INI,0)
    {
      // Determine the OS and store the value, where:
      //
      //   OSYS = 2000 = WIN2000.
      //   OSYS = 2001 = WINXP, RTM or SP1.
      //   OSYS = 2002 = WINXP SP2.
      //   OSYS = 2006 = Vista.
      //   OSYS = 2009 = Windows 7 and Windows Server 2008 R2.
      //   OSYS = 2012 = Windows 8 and Windows Server 2012.
      //   OSYS = 2013 = Windows Blue.
      //
      // Assume Windows 2000 at a minimum.

      Store(2000,OSYS)

      // Check for a specific OS which supports _OSI.

      If(CondRefOf(\_OSI,Local0))
      {
        // Linux returns _OSI = TRUE for numerous Windows
        // strings so that it is fully compatible with
        // BIOSes available in the market today.  There are
        // currently 2 known exceptions to this model:
        //      1) Video Repost - Linux supports S3 without
        //              requireing a Driver, meaning a Video
        //              Repost will be required.
        //      2) On-Screen Branding - a full CMT Logo
        //              is limited to the WIN2K and WINXP
        //              Operating Systems only.

        // Use OSYS for Windows Compatibility.

        If(\_OSI("Windows 2001"))   // Windows XP
        {
          Store(2001,OSYS)
        }

        If(\_OSI("Windows 2001 SP1"))   // Windows XP SP1
        {
          Store(2001,OSYS)
        }

        If(\_OSI("Windows 2001 SP2"))   // Windows XP SP2
        {
          Store(2002,OSYS)
        }

        If(\_OSI("Windows 2006"))   // Windows Vista
        {
          Store(2006,OSYS)
        }

        If(\_OSI("Windows 2009"))   // Windows 7 or Windows Server 2008 R2
        {
          Store(2009,OSYS)
        }
        If(\_OSI("Windows 2012"))   // Windows 8 or Windows Server 2012
        {
          Store(2012,OSYS)
        }
        If(\_OSI("Windows 2013"))   //Windows Blue
        {
          Store(2013,OSYS)
        }

        //
        // If CMP is enabled, enable SMM C-State
        // coordination.  SMM C-State coordination
        // will be disabled in _PDC if driver support
        // for independent C-States deeper than C1
        // is indicated.
      }
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base >>
#if defined (ASL_TBT_PCI0_INI_SUPPORT) && (ASL_TBT_PCI0_INI_SUPPORT == 1)
      ASL_TBT_INI               // \_SB.PCI0.RP0$(TBT_RPNum).TINI()
#endif
//AMI_OVERRIDE - EIP150027 hotplug resource for thunderbolt base <<
    }

    Method(NHPG,0,Serialized)
    {
//    Store(0,^RP01.HPEX)       // clear the hot plug SCI enable bit
//    Store(0,^RP02.HPEX)       // clear the hot plug SCI enable bit
//    Store(0,^RP03.HPEX)       // clear the hot plug SCI enable bit
//    Store(0,^RP04.HPEX)       // clear the hot plug SCI enable bit
//    Store(1,^RP01.HPSX)       // clear the hot plug SCI status bit
//    Store(1,^RP02.HPSX)       // clear the hot plug SCI status bit
//    Store(1,^RP03.HPSX)       // clear the hot plug SCI status bit
//    Store(1,^RP04.HPSX)       // clear the hot plug SCI status bit
    }

    Method(NPME,0,Serialized)
    {
//    Store(0,^RP01.PMEX)       // clear the PME SCI enable bit
//    Store(0,^RP02.PMEX)       // clear the PME SCI enable bit
//    Store(0,^RP03.PMEX)       // clear the PME SCI enable bit
//    Store(0,^RP04.PMEX)       // clear the PME SCI enable bit
//    Store(1,^RP01.PMSX)       // clear the PME SCI status bit
//    Store(1,^RP02.PMSX)       // clear the PME SCI status bit
//    Store(1,^RP03.PMSX)       // clear the PME SCI status bit
//    Store(1,^RP04.PMSX)       // clear the PME SCI status bit
    }
  } // end Scope(PCI0)

  Device (GPED)   //virtual GPIO device for ASL based AC/Battery/Expection notification
  {
    Name (_ADR, 0)
    Name (_HID, "INT0002")
    Name (_CID, "INT0002")
    Name (_DDN, "Virtual GPIO controller" )
    Name (_UID, 1)

    Method (_CRS, 0x0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        Interrupt (ResourceConsumer, Level, ActiveHigh, Exclusive, ,, ) {0x9} // Was 9
      })
      Return (RBUF)
    }

    Method (_STA, 0x0, NotSerialized)
    {
      Return(0x0)
    }

    Method (_AEI, 0x0, Serialized)
    {
      Name(RBUF, ResourceTemplate()
      {
        GpioInt(Edge, ActiveHigh, ExclusiveAndWake, PullDown,,"\\_SB.GPED",) {2} //pin 2
      })
      Return(RBUF)
    }

    Method(_E02)   // _Exx method will be called when interrupt is raised
    {
      If (LEqual (PWBS, 1))
      {
        Store (1, PWBS)      //Clear PowerButton Status
      }
      If (LEqual (PMEB, 1))
      {
        Store (1, PMEB)      //Clear PME_B0_STS
      }
      If (LEqual (\_SB.PCI0.SATA.PMES, 1))
      {
        Store (1, \_SB.PCI0.SATA.PMES)
        Notify (\_SB.PCI0.SATA, 0x02)
      }
      //
      // eMMC 4.41
      //
      If (LAnd(LEqual (\_SB.PCI0.EM41.PMES, 1), LEqual(PCIM, 1)))
      {
        Store (1, \_SB.PCI0.EM41.PMES)
        Notify (\_SB.PCI0.EM41, 0x02)
      }

      //
      // eMMC 4.5
      //
      If (LAnd(LEqual (\_SB.PCI0.EM45.PMES, 1), LEqual(PCIM, 1)))
      {
        Store (1, \_SB.PCI0.EM45.PMES)
        Notify (\_SB.PCI0.EM45, 0x02)
      }

      If (LEqual(HDAD, 0))
      {
        If (LEqual (\_SB.PCI0.HDEF.PMES, 1))
        {
          Store (1, \_SB.PCI0.HDEF.PMES)
          Notify (\_SB.PCI0.HDEF, 0x02)
        }
      }

      If (LEqual (\_SB.PCI0.EHC1.PMES, 1))
      {
        Store (1, \_SB.PCI0.EHC1.PMES)
        Notify (\_SB.PCI0.EHC1, 0x02)
      }
      If (LEqual (\_SB.PCI0.XHC1.PMES, 1))
      {
        Store (1, \_SB.PCI0.XHC1.PMES)
        Notify (\_SB.PCI0.XHC1, 0x02)
      }
      If (LEqual (\_SB.PCI0.SEC0.PMES, 1))
      {
        Or (\_SB.PCI0.SEC0.PMES, Zero, \_SB.PCI0.SEC0.PMES)
        Notify (\_SB.PCI0.SEC0, 0x02)
      }
    }
  } //  Device (GPED)
  
#ifdef WIN8_SUPPORT 
  //--------------------
  //  GPIO
  //--------------------
  Device (GPO0)
  {
    Name (_ADR, 0)
    Name (_HID, "INT33FC")
    Name (_CID, "INT33FC")
    Name (_DDN, "ValleyView2 General Purpose Input/Output (GPIO) controller" )
    Name (_UID, 1)
    Method (_CRS, 0x0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        //IO(Decode16,0x0500,0x0500,0x01,0xFF)
        Memory32Fixed (ReadWrite, 0x0FED0C000, 0x00001000)
        Interrupt(ResourceConsumer, Level, ActiveLow, Shared, , , ) {49}

      })
      Return (RBUF)
    }

    Method (_STA, 0x0, NotSerialized)
    {
      //
      // GPO driver will report present if any of below New IO bus exist
      //
      If (LOr(LEqual(L11D, 0), LEqual(L12D, 0))) // LPIO1 PWM #1 or #2 exist
      { Return(0xF) }
      If (LOr(LEqual(L13D, 0), LEqual(L14D, 0))) // LPIO1 HS-UART #1 or #2 exist
      { Return(0xF) }
      If (LOr(LEqual(L15D, 0), LEqual(SD1D, 0))) // LPIO1 SPI or SCC SDIO #1 exist
      { Return(0xF) }
      If (LOr(LEqual(SD2D, 0), LEqual(SD3D, 0))) // SCC SDIO #2 or #3 exist
      { Return(0xF) }
      If (LOr(LEqual(L21D, 0), LEqual(L22D, 0))) // LPIO2 I2C #1 or #2 exist
      { Return(0xF) }
      If (LOr(LEqual(L23D, 0), LEqual(L24D, 0))) // LPIO2 I2C #3 or #4 exist
      { Return(0xF) }
      If (LOr(LEqual(L25D, 0), LEqual(L26D, 0))) // LPIO2 I2C #5 or #6 exist
      { Return(0xF) }
      If (LEqual(L27D, 0))                       // LPIO2 I2C #7 exist
      { Return(0xF) }
      If (LEqual(SGMD, 0x02))                    // SG Mode enabled (Mux-less)
      { Return(0xF) }
      
      Return(0x0)
    }

    // Track status of GPIO OpRegion availability for this controller
    Name(AVBL, 0)
    Method(_REG,2)
    {
      If (Lequal(Arg0, 8))
      {
        Store(Arg1, ^AVBL)
      }
    }
    //Manipulate GPIO line using GPIO operation regions.
    //Name (GMOD, ResourceTemplate () {   //One method of creating a Connection for OpRegion accesses in Field definitions
    //is creating a named object that refers to the connection attributes
    //  GpioIo (Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0") {53,59}  //BT
    //})

      OperationRegion(GPOP, SystemIo, \GPBS, 0x50)
      Field(GPOP, ByteAcc, NoLock, Preserve) {
      Offset(0x28), // cfio_ioreg_SC_GP_LVL_63_32_ - [GPIO_BASE_ADDRESS] + 28h
          ,  21,
      BTD3,  1,     //This field is not used. Pin not defined in schematics. Closest is GPIO_S5_35 - COMBO_BT_WAKEUP
      Offset(0x48), // cfio_ioreg_SC_GP_LVL_95_64_ - [GPIO_BASE_ADDRESS] + 48h
          ,  30,
      SHD3,  1      //GPIO_S0_SC_95 - SENS_HUB_RST_N
    } 
#if 0
    OperationRegion(GPO2, GeneralPurposeIO, 0x0, 0xC)
    //Define PMIO Field definition
    //GPIO opregion accesses are ByteAcc
    Field(\_SB.GPO0.GPO2, ByteAcc, NoLock, Preserve)
    {
      Connection (GpioIo (Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO0") {95}), //Sensor hub
      SHS3, 1
    }
#endif
  }   //  Device (GPO0)

         Device (GPO1)
  {
    Name (_ADR, 0)
    Name (_HID, "INT33FC")
    Name (_CID, "INT33FC")
    Name (_DDN, "ValleyView2 GPNCORE controller" )
    Name (_UID, 2)
    Method (_CRS, 0x0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x0FED0D000, 0x00001000)
        Interrupt(ResourceConsumer, Level, ActiveLow, Shared, , , ) {48}
      })
      Return (RBUF)
    }

    Method (_STA, 0x0, NotSerialized)
    {
      Return(\_SB.GPO0._STA)
    }
  }   //  Device (GPO1)

  Device (GPO2)
  {
    Name (_ADR, 0)
    Name (_HID, "INT33FC")
    Name (_CID, "INT33FC")
    Name (_DDN, "ValleyView2 GPSUS controller" )
    Name (_UID, 3)
    Method (_CRS, 0x0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        Memory32Fixed (ReadWrite, 0x0FED0E000, 0x00001000)
        Interrupt(ResourceConsumer, Level, ActiveLow, Shared, , , ) {50}
      })
      Return (RBUF)
    }

    Method (_STA, 0x0, NotSerialized)
    {
      Return(^^GPO0._STA)
    }

    // Track status of GPIO OpRegion availability for this controller
    Name(AVBL, 0)
    Method(_REG,2)
    {
      If (Lequal(Arg0, 8))
      {
        Store(Arg1, ^AVBL)
      }
    }
    //Manipulate GPIO line using GPIO operation regions.
    Name (GMOD, ResourceTemplate ()     //One method of creating a Connection for OpRegion accesses in Field definitions
    {
      //is creating a named object that refers to the connection attributes
      GpioIo (Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2") {21}  //sus 21+128 BT+WLAN_ENABLE
    })

  OperationRegion(GPOP, SystemIo, \GPBS, 0x100)
  Field(GPOP, ByteAcc, NoLock, Preserve) {
      Offset(0x88),  // cfio_ioreg_SUS_GP_LVL_31_0_ - [GPIO_BASE_ADDRESS] + 88h
          ,  20,
      WFD3,  1
    }
#if 0
  OperationRegion(GPO1, GeneralPurposeIO, 0x0, 0xC)
  Field(\_SB.GPO2.GPO1, ByteAcc, NoLock, Preserve)    // Named object that refers to connection attributes (defined above)
    {
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#if defined (ASL_SgTpv_SUPPORT) && (ASL_SgTpv_SUPPORT == 1)
      Connection (GpioIo (Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2") {ASL_GPIO_dGPU_HOLD_RST}), //USB_ULPI_0_DATA3 Used to reset DGPU Card 
                 RP1R, 1,
      Connection (GpioIo (Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2") {ASL_GPIO_dGPU_PWR_EN}), //USB_ULPI_0_DATA3 Used to Powrer ON/OFF DGPU Card 
                 RP1P, 1
#else
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform <<
      Connection (GpioIo (Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2") {35}), //USB_ULPI_0_DATA3 Used to reset DGPU Card 
                 RP1R, 1,
      Connection (GpioIo (Exclusive, PullDefault, 0, 0, IoRestrictionOutputOnly, "\\_SB.GPO2") {36}), //USB_ULPI_0_DATA3 Used to Powrer ON/OFF DGPU Card 
                 RP1P, 1
#endif //AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform
    }
#else
  OperationRegion(GPO1, SystemIo, \GPBS, 0x100)
  Field(GPO1, ByteAcc, NoLock, Preserve)    // Named object that refers to connection attributes (defined above)
    {
      Offset(0xa8),  // cfio_ioreg_SUS_GP_LVL_43_32_ - [GPIO_BASE_ADDRESS] + 80h + 28h
          ,  3,
          RP1R, 1,   //USB_ULPI_0_DATA3 Used to reset DGPU Card 
          RP1P, 1    //USB_ULPI_0_DATA4 Used to Powrer ON/OFF DGPU Card 
    }
#endif
  }   //  Device (GPO2)
  include ("PchScc.asl")
  include ("PchLpss.asl")

         Scope(I2C7)
  {

  } //End Scope(I2C7)
#endif
  //
  // Device for Message Bus Interface
  //
  Device(MBID)
  {
    Name(_HID, "INT33BD")
    Name(_CID, "INT33BD")
    Name(_HRV, 2)//different from CLT's
    Name(_UID, 1)

    Method (_CRS, 0, Serialized)
    {
      Name (RBUF, ResourceTemplate ()
      {
        Memory32Fixed (
          ReadWrite,
          0xE00000D0, // Address Base
          0xC,        // Address Length (MCR/MDR/MCRX)
        )
      })
      Return (RBUF)
    }


    //
    // custom opregion for MBI access
    //
    OperationRegion (REGS, 0x87, 0x0, 0x30)
    Field (REGS, DWordAcc, NoLock, Preserve)
    {
      PORT, 32,    // Message Port
      REG,  32,    // Message Target Register Address
      DATA, 32,    // Message Data
      MASK, 32,    // Mask bits for modify operation
      BE,   32,    // Message Write Byte enables: 0 - BYTE; 1 - WORD; 2 - DWORD
      OP,   32     // Operations: 0 - read; 1 - write; 2 - modify
    }

    //
    // availability of the custom opregion
    //                                      
    Name (AVBL, 0)
    Method(_REG,2) 
    {
      If (Lequal(Arg0, 0x87))
      {
        Store(Arg1, ^AVBL)
      }
    }

    //
    //  Method Name: READ
    //  Arguments:
    //    Arg0:   PORT
    //    Arg1:   REG
    //    Arg2:   BE
    //  Return Value:
    //  DATA
    //
    Method(READ, 3, Serialized)
    {
      Store(0xFFFFFFFF , Local0)
      If (Lequal (AVBL, 1))
      {
        Store(0, OP)  // must be set at first, do not change!
        Store(Arg0, PORT)
        Store(Arg1, REG)
        Store(Arg2, BE)
        Store(DATA, Local0)
      }
      return(Local0)
    }
  
    //
    //  Method Name: WRIT
    //  Arguments:
    //    Arg0:   PORT
    //    Arg1:   REG
    //    Arg2:   BE
    //    Arg3:   DATA
    //  Return Value: 
    //  NONE
    //  
    Method(WRIT, 4, Serialized) 
    {
      If (Lequal (AVBL, 1))
      {
        Store(1, OP)  // must be set at first, do not change!
        Store(Arg0, PORT)
        Store(Arg1, REG)
        Store(Arg2, BE)
        Store(Arg3, DATA)
      }
    }

    //
    //  Method Name: MODI
    //  Arguments:
    //    Arg0:   PORT
    //    Arg1:   REG
    //    Arg2:   BE
    //    Arg3:   DATA
    //    Arg4:   MASK
    //  Return Value:
    //  NONE
    //
    Method(MODI, 5, Serialized)
    {
      If (Lequal (AVBL, 1))
      {
        Store(2, OP)  // must be set at first, do not change!
        Store(Arg0, PORT)
        Store(Arg1, REG)
        Store(Arg2, BE)
        Store(Arg3, DATA)
        Store(Arg4, MASK)
      }
    }
  }


} // end Scope(\_SB)

Name(PICM, 0)   // Global Name, returns current Interrupt controller mode; updated from _PIC control method

