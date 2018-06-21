/**************************************************************************;
;*                                                                        *;
;*    Intel Confidential                                                  *;
;*                                                                        *;
;*    Intel Corporation - ACPI Reference Code for the Valleyview          *;
;*    Family of Customer Reference Boards.                                *;
;*                                                                        *;
;*                                                                        *;
;*    Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved   *;
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


//
// Defined as an SSDT to be able to dynamically load based on BIOS
// setup options
//
DefinitionBlock (
  "PTID.aml",
  "SSDT",
  0x01,
  "TrmRef",
  "PtidDevc",
  0x1000
  )


{
  Scope(\_SB)
  {
    // External References to the actual data locations that stores
    // various temperature and power values (either from EC or by
    // other means)
    //
    // TODO: Update as per platform definition

    External(DTS1)  // DTS Core 0 Temp
    External(DTS2)  // DTS Core 1 Temp
    Device(PTID)
    {
      // Note: _CID must be PNP0C02. This will be updated in a
      // future revision of the BIOS Writers Guide.

      Name(_HID, EISAID("INT340E"))
      Name(_CID, EISAID("PNP0C02"))

      // The IVER object evaluates to an integer that represents the version of this interface.
      // The upper two bytes indicate the major version and the
      // lower two bytes indicate the minor version. Based on Ref 24682
      //
      Name(IVER, 0x00020001)                  // Version 2.1

      Method (_STA)
      {
        Return (0x0F)
      }

      // Note: TMPV and PWRV must be returned as single package.
      // This will be updated in a future revision of the BIOS
      // Writers Guide.

      // TODO: Update as per platform definition

      // Changed the device class type as per the ref 24682 PTID BWG
      // for all the TMPV, PWRV, OSDV
      //
      Name(TMPV, Package()
      {
        //DeviceClass type //Name of Temperature Value          // Placeholder
        0x00000000, "CPU Thermal Diode Temperature",            0x80000000,
        0x00000000, "CPU Core 0 DTS",                                       0x80000000,
        0x00000000, "CPU Core 1 DTS",                                       0x80000000,
        0x00000000, "CPU VR (IMVP) Temperature",                        0x80000000,
        0x00000004, "Heat Exchanger Fan Temperature",           0x80000000,
        0x0000000A, "Skin Temperature",                                     0x80000000,
        0x0000000A, "Ambient Temperature",                              0x80000000
      })

      // TODO: Update as per platform definition

      Name(PWRV, Package()
      {
        // DeviceClass type   //Name of Power Value                             // Placeholder
        0x00000000,       "CPU Power",                                          0x80000000,
        0x00000001,       "Gfx Core Power",                                     0x80000000,
        0x0000000A,       "System Power",                                       0x80000000,
        0x00000000,       "CPU Average Power",                          0x80000000,
        0x00000001,       "Gfx Core Average Power",                     0x80000000,
        0x0000000A,       "System Average Power",                       0x80000000
      })

      // OSDV should have Device class Type as one of the index
      // as per the Ref 24682 PTID spec BWG
      //
      Name(OSDV, Package()
      {
        // DeviceClass type // Descriptive Name   //Unit                                // Placeholder
        0x00000004,      "CPU Fan Speed",      "RPM",           0x80000000,
        0x00000004,      "GMCH Fan Speed ",    "RPM",           0x80000000,

      })

      Method (SDSP)
      {
        Return(10)      // Sampling period .
      }
    }
  } // end \_SB scope
} // end SSDT
