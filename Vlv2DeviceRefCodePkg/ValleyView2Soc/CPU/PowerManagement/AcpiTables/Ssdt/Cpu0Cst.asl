
/*-----------------------------------------------------------------------------
-------------------------------------------------------------------------------

 Intel(R) Confidential

 Intel Silvermont Processor Power Management BIOS Reference Code

 Copyright (c) 2006-2013 Intel Corporation

 Filename:    CPU0CST.ASL

 Revision:    Refer to Readme

 Date:        Refer to Readme

--------------------------------------------------------------------------------
-------------------------------------------------------------------------------

 This Processor Power Management BIOS Source Code is furnished under license
 and may only be used or copied in accordance with the terms of the license.
 The information in this document is furnished for informational use only, is
 subject to change without notice, and should not be construed as a commitment
 by Intel Corporation. Intel Corporation assumes no responsibility or liability
 for any errors or inaccuracies that may appear in this document or any
 software that may be provided in association with this document.

 Except as permitted by such license, no part of this document may be
 reproduced, stored in a retrieval system, or transmitted in any form or by
 any means without the express written consent of Intel Corporation.

 WARNING: You are authorized and licensed to install and use this BIOS code
 ONLY on an IST PC. This utility may damage any system that does not
 meet these requirements.

    An IST PC is a computer which
    (1) Is capable of seamlessly and automatically transitioning among
    multiple performance states (potentially operating at different
    efficiency ratings) based upon power source changes, END user
    preference, processor performance demand, and thermal conditions; and
    (2) Includes an Intel Pentium II processors, Intel Pentium III
    processor, Mobile Intel Pentium III Processor-M, Mobile Intel Pentium 4
    Processor-M, Intel Pentium M Processor, or any other future Intel
    processors that incorporates the capability to transition between
    different performance states by altering some, or any combination of,
    the following processor attributes: core voltage, core frequency, bus
    frequency, number of processor cores available, or any other attribute
    that changes the efficiency (instructions/unit time-power) at which the
    processor operates.

-------------------------------------------------------------------------------
-------------------------------------------------------------------------------

NOTES:
    (1) <TODO> - IF the trap range and port definitions do not match those
    specified by this reference code, this file must be modified IAW the
    individual implmentation.

--------------------------------------------------------------------------------
------------------------------------------------------------------------------*/
/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/                                                                      


DefinitionBlock (
    "CPU0CST.aml",
    "SSDT",
    1,
    "PmRef",
    "Cpu0Cst",
    0x3001
    )
{
    External(\_PR.CPU0, DeviceObj)
    External(PWRS)
    External(CFGD)
    External(PDC0)

    Scope(\_PR.CPU0)
    {
        OperationRegion (DEB0, SystemIO, 0x80, 1)    //DBG
        Field (DEB0, ByteAcc,NoLock,Preserve)        //DBG
        { DBG8, 8,}                    //DBG

        Method (_CST, 0)
        {
            Store(0x60,DBG8) //DBG

            // IF CMP is supported, but independent C-States beyond C1 are
            // not supported; return C1 Halt and rely on BIOS based software
            // coordination
            //
            //   CFGD[24] = CMP support
            //   PDCx[4]  = 0 - OS does not support ind. C2/C3 in MP systems
            //
            // Note:  SMI will be generated when both processor enter the
            // Halt state.
            //
            If(LAnd(And(CFGD,0x01000000), LNot(And(PDC0,0x10))))
            {
                Store(0x61,DBG8) //DBG
                Return(Package() {
                    1,
                    Package()
                    {   // C1 halt, but with BIOS coordination
                        ResourceTemplate(){Register(FFixedHW, 0, 0, 0)},
                        1,
                        157,
                        1000
                    }
                })
            }

            // IF MWAIT extensions are supported, use them.
            //
            //  IF C6 capable/enabled AND Battery
            //        Report MWAIT C1, C2, C6 w/ BM_STS avoidance
            //  ELSE IF C4 capable/enabled AND Battery
            //        Report MWAIT C1, C2, C4 w/ BM_STS avoidance
            //  ELSE IF C3 capable/enabled
            //      Report MWAIT C1, C2, C3 w/ BM_STS avoidance
            //  ELSE IF C2 capable/enabled
            //        Report MWAIT C1, C2
            //  ELSE
            //        Report MWAIT C1
            //
            //   CFGD[21] = 1 - MWAIT extensions supported
            //   CFGD[13] = 1 - C7  Capable/Enabled
            //   CFGD[12] = 1 - C6S Capable/Enabled
            //   CFGD[11] = 1 - C6  Capable/Enabled
            //   CFGD[7]  = 1 - C4  Capable/Enabled
            //   CFGD[5]  = 1 - C3  Capable/Enabled
            //   PDCx[9]  = 1 - OS  supports MWAIT extensions
            //   PDCx[8]  = 1 - OS  supports MWAIT for C1
            //            (Inferred from PDCx[9] = 1.)
            //   PDCx[4]  = 1 - OS supports independent C2/C3 in MP systems
            //    or
            //   NOT CMP  (Inferred from previous check.)
            //
            If(LAnd(And(CFGD, 0x200000), And(PDC0,0x200)))
            {
                //
                // <TODO> The implementor may wish to only report C1-C2
                // when on AC power.  In this case, the IF clause below can
                // be modified to something like:
                //
                // "If(LAnd(And(CFGD,0x200), LNot(PWRS)))"
                //
                // Which uses the power state of the system (PWRS) to
                // determine whether to allow deepers states.
                //
                //   IF C7 supported AND on battery
                //    report MWAIT C1, C6, C7
                //
                //   CFGD[13] = C7  Capable/Enabled
                //   CFGD[11] = C6  Capable/Enabled
                //
                // S0ix and C-states matrix for VLV2:
                //----------------------------------------------------------------------
                //||S0ix options  ||C-state options ||Descriptions  ||ACPI C-states   ||
                //||------------------------------------------------------------------||
                //||S0ix enabled  ||C7              ||C6FS+S0ix hint||C1=C1 (00);     ||
                //||              ||                ||              ||C2=C6NS (0x51); ||
                //||              ||                ||              ||C3=S0i3-FS(0x64)||
                //||------------------------------------------------------------------||
                //||S0ix disabled ||C1              ||C1            ||C1=C1 (00)      ||
                //||              ||--------------------------------------------------||
                //||              ||C6              ||C6NS          ||C1=C1 (00);     ||
                //||              ||                ||              ||C2=C6NS (0x51)  ||
                //||              ||--------------------------------------------------||
                //||              ||C7              ||C6FS          ||C1=C1 (00);     ||
                //||              ||                ||              ||C2=C6NS (0x51)  ||
                //||              ||                ||              ||C3=C6FS (0x52)  ||
                //----------------------------------------------------------------------
              If(LAnd(And(CFGD,0x2000),And(CFGD,0x40000000))) // Setup Max C-State = C7[bit13],S0ix Enable[bit30]
                {
                    Store(0x77,DBG8) //DBG
                    Return( Package()
                    {
                        3,
                        Package()
                        {   // C1, MWAIT
                            ResourceTemplate(){Register(FFixedHW, 1, 2, 0x00, 1)},
                            1,
                            1,
                            1000
                        },
                        Package()
                        {
                            // C6, MWAIT Extension with Incremental L2 Shrink
                            // ResourceTemplate(){Register(FFixedHW, 1, 2, 0x50, 1)},
                            // C6, MWAIT Extension with No L2 Shrink
                            ResourceTemplate(){Register(FFixedHW, 1, 2, 0x51, 1)},
                            2,
                            500,
                            10
                        },
                        Package()
                        {
                            // C7, MWAIT Extension with Full L2 Shrink
                            ResourceTemplate(){Register(FFixedHW, 1, 2, 0x64, 1)},
                            3,
                            1500,   //PnP setting, 1.5 ms for worst-case exit latency
                            10
                        }
                    })
                }


             If(LAnd(And(CFGD,0x2000),LNot(And(CFGD,0x40000000)))) // Setup Max C-State = C7[bit13],S0ix Disable[bit30]
                {
                    Store(0x67,DBG8) //DBG
                    Return( Package()
                    {
                        3,
                        Package()
                        {   // C1, MWAIT
                            ResourceTemplate(){Register(FFixedHW, 1, 2, 0x00, 1)},
                            1,
                            1,
                            1000
                        },
                        Package()
                        {
                            // C6, MWAIT Extension with Incremental L2 Shrink
                            // ResourceTemplate(){Register(FFixedHW, 1, 2, 0x50, 1)},
                            // C6 = C6NS, MWAIT Extension with No L2 Shrink
                            ResourceTemplate(){Register(FFixedHW, 1, 2, 0x51, 1)},
                            2,
                            500,
                            10
                        },
                        Package()
                        {
                            // C7 = C6FS without S0ix hint, MWAIT Extension with Full L2 Shrink
                            ResourceTemplate(){Register(FFixedHW, 1, 2, 0x52, 1)},
                            3,
                            1500,   //PnP setting, 1.5 ms for worst-case exit latency
                            10
                        }
                    })
                }

                If(And(CFGD,0x800)) // Setup Max C-State = C6
                {
                    Store(0x76,DBG8) //DBG
                    Return( Package()
                    {
                        2,
                        Package()
                        {   // C1, MWAIT
                            ResourceTemplate(){Register(FFixedHW, 1, 2, 0x00, 1)},
                            1,
                            1,
                            1000
                        },
                        Package()
                        {
                            // C6, MWAIT Extension with Incremental L2 Shrink
                            // ResourceTemplate(){Register(FFixedHW, 1, 2, 0x50, 1)},
                            // C6, MWAIT Extension with No L2 Shrink
                            ResourceTemplate(){Register(FFixedHW, 1, 2, 0x51, 1)},
                            2,
                            500,
                            10
                        }
                    })
                }
                //
                // IF no deeper C-States are supported; report MWAIT C1.
                //
                Store(0x71,DBG8) //DBG
                Return(Package()
                {
                    1,
                    Package()
                    {   // C1, MWAIT
                        ResourceTemplate () {Register(FFixedHW, 1, 2, 0x00, 1)},
                        1,
                        1,
                        1000
                    }
                })
            }

            // IF C1 MWAIT is supported, use it.
            //
            //   IF C6 capable/enabled AND Battery
            //        Report MWAIT C1, C2, C6
            //   ELSE IF C4 capable/enabled AND Battery
            //        Report MWAIT C1, Standard C2, C4
            //   ELSE IF C3 capable/enabled
            //        Report MWAIT C1, Standard C2, C3
            //   ELSE IF C2 capable/enabled
            //        Report MWAIT C1, Standard C2
            //   ELSE
            //        Report MWAIT C1
            //
            //   CFGD[21] = 1 - MWAIT extensions supported
            //   CFGD[11]  = 1 - C6 Capable/Enabled
            //   CFGD[7]  = 1 - C4 Capable/Enabled
            //   CFGD[5]  = 1 - C3 Capable/Enabled
            //   PDCx[9]  = 0 - OS supports MWAIT extensions
            //            (Inferred from previous checks.)
            //   PDCx[8]  = 1 - OS supports MWAIT for C1
            //   PDCx[4]  = 1 - OS supports independent C2/C3 in MP systems
            //    or
            //   NOT CMP  (Inferred from previous check.)
            //
            If(LAnd(And(CFGD, 0x200000), And(PDC0,0x100)))
            {
                //
                // <TODO> The implementor may wish to only report C1-C2
                // when on AC power.  In this case, the IF clause below can
                // be modified to something like:
                //
                // "If(LAnd(And(CFGD,0x200), LNot(PWRS)))"
                //
                // Which uses the power state of the system (PWRS) to
                // determine whether to allow deepers states.
                //
                //
                //   IF C6 supported AND battery
                //        Report MWAIT C1, I/O C4/C6
                //
                //   CFGD[13] = C7  Capable/Enabled
                //   CFGD[11] = C6 Capable/Enabled
                //
                If(And(CFGD,0x2000))
                {
                    Store(0x87,DBG8) //DBG
                    Return( Package()
                    {
                        3,
                        Package()
                        {   // C1, MWAIT
                            ResourceTemplate () {Register(FFixedHW, 1, 2, 0x00, 1)},
                            1,
                            1,
                            1000
                        },
                        Package()
                        {   // C6, LVL_3
                            ResourceTemplate () {Register(SystemIO, 8, 0, 0x416)},
                            2,
                            500,
                            100
                        },
                        Package()
                        {   // C7, LVL_4
                            ResourceTemplate () {Register(SystemIO, 8, 0, 0x417)},
                            3,
                            5000,
                            10
                        }
                    })
                }
                If(And(CFGD,0x800))
                {
                    Store(0x86,DBG8) //DBG
                    Return( Package()
                    {
                        2,
                        Package()
                        {   // C1, MWAIT
                            ResourceTemplate () {Register(FFixedHW, 1, 2, 0x00, 1)},
                            1,
                            1,
                            1000
                        },
                        Package()
                        {   // C6, LVL_3
                            ResourceTemplate () {Register(SystemIO, 8, 0, 0x416)},
                            2,
                            500,
                            10
                        }
                    })
                }
                //
                // IF no deeper C-States are supported; report MWAIT C1.
                //
                Store(0x81,DBG8) //DBG
                Return(Package()
                {
                    1,
                    Package()
                    {   // C1, MWAIT
                        ResourceTemplate () {Register(FFixedHW, 1, 2, 0x00, 1)},
                        1,
                        1,
                        1000
                    }
                })
            }

            // ELSE IF MWAIT is NOT supported:
            //
            //   IF C6 capable/enabled and Battery
            //        Report C1, C2, C6
            //   ELSE IF C4 capable/enabled and Battery
            //        Report C1, C2, C4
            //   ELSE IF C3 capable/enabled
            //        Report C1, C2, C3
            //   ELSE IF C2 capable/enabled
            //        Report C1, C2
            //   ELSE Report C1
            //
            //   CFGD[21] = 0 - MWAIT extensions not supported
            //   PDCx[9]  = 0 - OS does not support MWAIT extensions
            //            (Inferred from previous checks.)
            //   PDCx[8]  = 0 - OS does not support MWAIT for C1
            //            (Inferred from previous checks.)
            //   PDCx[4]  = 1 - OS supports independent C2/C3 in MP systems
            //    or
            //   NOT CMP  (Inferred from previous check.)
            //
            // <TODO> The implementor may wish to only report C1-C2
            // when on AC power.  In this case, the IF clause below can
            // be modified to something like:
            //
            // "If(LAnd(And(CFGD,0x200), LNot(PWRS)))"
            //
            // Which uses the power state of the system (PWRS) to
            // determine whether to allow deepers states.
            //
            //
            //   IF AC power AND C6 supported; report C1/C3/C6
            //
            //   CFGD[13] = C7 Capable/Enabled
            //   CFGD[11] = C6 Capable/Enabled
            //
            If(And(CFGD,0x2000))
            {
                Store(0x97,DBG8) //DBG
                Return(Package()
                {
                    // Added ACPI C4 c-state package for Linux
                    3,
                    Package()
                    {   // C1
                        ResourceTemplate () {Register(FFixedHW, 0, 0, 0)},
                        1,
                        1,
                        1000
                    },
                    Package()
                    {   // C6
                        ResourceTemplate () {Register(SystemIO, 8, 0, 0x416)},
                        2,
                        500,
                        100
                    },
                    Package()
                    {   // C7
                        ResourceTemplate () {Register(SystemIO, 8, 0, 0x417)},
                        3,
                        5000,
                        10
                    }
                })
            }
            If(And(CFGD,0x800))
            {
                Store(0x96,DBG8) //DBG
                Return(Package()
                {
                    2,
                    Package()
                    {   // C1
                        ResourceTemplate () {Register(FFixedHW, 0, 0, 0)},
                        1,
                        1,
                        1000
                    },
                    Package()
                    {   // C6
                        ResourceTemplate () {Register(SystemIO, 8, 0, 0x416)},
                        2,
                        500,
                        10
                    }
                })
            }
            //
            // IF no deeper C-States are supported; report C1.
            //
            Store(0x91,DBG8) //DBG
            Return(Package()
            {
                1,
                Package()
                {   // C1
                    ResourceTemplate () {Register(FFixedHW, 0, 0, 0)},
                    1,
                    1,
                    1000
                }
            })
        }
    }
}


