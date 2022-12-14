/*-----------------------------------------------------------------------------
-------------------------------------------------------------------------------

 Intel(R) Confidential

 Intel Platform Processor Power Management BIOS Reference Code

 Copyright (c) 2007 Intel Corporation

 Filename:      APTST.ASL

 Revision:      Refer to Readme

 Date:          Refer to Readme

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


DefinitionBlock(
        "APTST.aml",
        "SSDT",
        0x01,
        "PmRef",
        "ApTst",
        0x3000
        )
{
        External(\_PR.CPU1, DeviceObj)
        External(\_PR.CPU2, DeviceObj)
        External(\_PR.CPU3, DeviceObj)          
        External(\_PR.CPU0._PTC)
        External(\_PR.CPU0._TSS)
        External(PDC0)
        External(CFGD)
        External(MPEN)

        Scope(\_PR.CPU1)
        {
                Name(_TPC, 0)   // All T-States are available

                //
                // T-State Control/Status interface
                //
                Method(_PTC, 0)
                {
                        Return(\_PR.CPU0._PTC)
                }

                Method(_TSS, 0)
                {
                        Return(\_PR.CPU0._TSS)
                }

                //
                // T-State Dependency
                //
                Method(_TSD, 0)
                {
                        //
                        // IF four cores are supported/enabled && !(direct access to MSR)
                        //    Report 4 processors and SW_ANY as the coordination
                        // IF two cores are supported/enabled && !(direct access to MSR)
                        //    Report 2 processors and SW_ANY as the coordination type
                        //  ELSE
                        //    Report 1 processor and SW_ALL as the coordination type (domain 1)
                        //
                        //  CFGD[23] = Four cores enabled
                        //  CFGD[24] = Two or more cores enabled
                        //  PDCx[2] = OSPM is capable of direct access to On
                        //              Demand throttling MSR
                        //

                If(LNot(And(PDC0,4)))
                {
                                Return(Package(){       // SW_ANY
                                        Package(){
                                                5,                // # entries.
                                                0,                // Revision.
                                                0,                // Domain #.
                                                0xFD,           // Coord Type- SW_ANY
                                                MPEN          // # processors.
                                        }
                                })
                }
                Return(Package(){               // SW_ALL
                        Package(){
                                5,                        // # entries.
                                0,                        // Revision.
                                1,                        // Domain #.
                                0xFC,                   // Coord Type- SW_ALL
                                1               // # processors.
                        }
                })
                }
        }  // End of CPU1

        Scope(\_PR.CPU2)
        {
                Name(_TPC, 0)   // All T-States are available

                //
                // T-State Control/Status interface
                //
                Method(_PTC, 0)
                {
                        Return(\_PR.CPU0._PTC)
                }

                Method(_TSS, 0)
                {
                        Return(\_PR.CPU0._TSS)
                }

                //
                // T-State Dependency
                //
                Method(_TSD, 0)
                {
                        //
                        // IF four cores are supported/enabled && !(direct access to MSR)
                        //    Report 4 processors and SW_ANY as the coordination
                        // IF two cores are supported/enabled && !(direct access to MSR)
                        //    Report 2 processors and SW_ANY as the coordination type
                        //  ELSE
                        //    Report 1 processor and SW_ALL as the coordination type (domain 1)
                        //
                        //  CFGD[23] = Four cores enabled
                        //  CFGD[24] = Two or more cores enabled
                        //  PDCx[2] = OSPM is capable of direct access to On
                        //              Demand throttling MSR
                        //

                If(LNot(And(PDC0,4)))
                {
                                Return(Package(){       // SW_ANY
                                        Package(){
                                                5,                // # entries.
                                                0,                // Revision.
                                                0,                // Domain #.
                                                0xFD,           // Coord Type- SW_ANY
                                                MPEN          // # processors.
                                        }
                                })
                }
                Return(Package(){               // SW_ALL
                        Package(){
                                5,                        // # entries.
                                0,                        // Revision.
                                1,                        // Domain #.
                                0xFC,                   // Coord Type- SW_ALL
                                1                // # processors.
                        }
                })
                }
        }  // End of CPU2

        Scope(\_PR.CPU3)
        {
                Name(_TPC, 0)   // All T-States are available

                //
                // T-State Control/Status interface
                //
                Method(_PTC, 0)
                {
                        Return(\_PR.CPU0._PTC)
                }

                Method(_TSS, 0)
                {
                        Return(\_PR.CPU0._TSS)
                }

                //
                // T-State Dependency
                //
                Method(_TSD, 0)
                {
                        //
                        // IF four cores are supported/enabled && !(direct access to MSR)
                        //    Report 4 processors and SW_ANY as the coordination
                        // IF two cores are supported/enabled && !(direct access to MSR)
                        //    Report 2 processors and SW_ANY as the coordination type
                        //  ELSE
                        //    Report 1 processor and SW_ALL as the coordination type (domain 1)
                        //
                        //  CFGD[23] = Four cores enabled
                        //  CFGD[24] = Two or more cores enabled
                        //  PDCx[2] = OSPM is capable of direct access to On
                        //              Demand throttling MSR
                        //

                If(LNot(And(PDC0,4)))
                {
                                Return(Package(){       // SW_ANY
                                        Package(){
                                                5,                // # entries.
                                                0,                // Revision.
                                                0,                // Domain #.
                                                0xFD,           // Coord Type- SW_ANY
                                                MPEN          // # processors.
                                        }
                                })
                }
                Return(Package(){               // SW_ALL
                        Package(){
                                5,                        // # entries.
                                0,                        // Revision.
                                1,                        // Domain #.
                                0xFC,                   // Coord Type- SW_ALL
                                1                // # processors.
                        }
                })
                }
        }  // End of CPU3
} // End of Definition Block

