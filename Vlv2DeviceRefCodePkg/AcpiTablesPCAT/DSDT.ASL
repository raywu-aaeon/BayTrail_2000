/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Sandy Bridge        *;
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

DefinitionBlock (
  "DSDT.aml",
  "DSDT",
  0x02,  // DSDT revision.
  // A Revision field value greater than or equal to 2 signifies that integers
  // declared within the Definition Block are to be evaluated as 64-bit values
  "INTEL", // OEM ID (6 byte string)
  "VLV-SOC", // OEM table ID  (8 byte string)
  0x0 // OEM version of DSDT table (4 byte Integer)
)

// BEGIN OF ASL SCOPE
{
  External(MDBG, MethodObj)

  Method(ADBG, 1, Serialized)
  {

    If(CondRefOf(MDBG))   //check if SSDT is loaded
    {
      Return(MDBG(Arg0))
    }

    Return(0)
  }


// Miscellaneous services enabled in Project
  include ("amlupd.asl")    //AMI_OVERRODE
  include ("token.asl")
  include ("GloblNvs.asl")
  include ("PciTree.asl")
  include ("Pch.asl")
  include ("Vlv.asl")
  include ("CPU.asl")
  include ("Platform.asl")
  include ("THERMAL.ASL")
  include ("PCI_DRC.ASL")
  include ("Video.asl")
  include ("Gpe.asl")

// Sleep states supported by Chipset/Board.
//----------------------------------------------------------------------
// SSx - BIOS setup controlled enabled _Sx Sleep state status
// Values to be written to SLP_TYPE register are provided by SBACPI.SDL (South Bridge ACPI ModulePart)

  Name(\_S0, Package(4) {0x0,0x0,0,0}) // mandatory System state
  Name(\_S3, Package(4) {0x5,0x0,0,0})
  Name(\_S4, Package(4) {0x6,0x0,0,0})
  Name(\_S5, Package(4) {0x7,0x0,0,0}) // mandatory System state

  Method(PTS, 1)          // METHOD CALLED FROM _PTS PRIOR TO ENTER ANY SLEEP STATE
  {
    If(Arg0)            // entering any sleep state
    {
    }
  }
  Method(WAK, 1)          // METHOD CALLED FROM _WAK RIGHT AFTER WAKE UP
  {
  }

}// End of ASL File
