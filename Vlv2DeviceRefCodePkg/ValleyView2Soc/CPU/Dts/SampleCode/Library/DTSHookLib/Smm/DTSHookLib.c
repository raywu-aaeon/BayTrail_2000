/*++

Copyright (c)  2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

    DTSHookLib.c

Abstract:

  Digital Thermal Sensor (DTS) SMM Library.
  This SMM Library configures and supports the DigitalThermalSensor features
  for the platform.

--*/
/*++
 This file contains an 'Intel Peripheral Driver' and is
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may
 be modified by the user, subject to additional terms of the
 license agreement
--*/

#include <Library/DTSHookLib.h>

EFI_STATUS
InitializeDtsHookLib (
  VOID
  )
/*++

Routine Description:

  Prepare data and protocol for Dts Hooe Lib

Arguments:

  None

Returns:

  EFI_SUCCESS - Initialize complete

--*/
{
  //
  //Nothing to do on CRB.
  //
  return EFI_SUCCESS;
}

VOID
PlatformHookBeforeGenerateSCI (
  VOID
  )
/*++

Routine Description:

  Platform may need to register some data to private data structure before generate
  software SMI or SCI.

Arguments:

  None

Returns:

  None
--*/
{
  //
  //Nothing to do on CRB.
  //
}

UINT8
ReadPlatformThermalDiode(
  VOID
  )
/*++

Routine Description:

  Read CPU temperature from platform diode

Arguments:

  None

Returns:

  TemperatureOfDiode   -  Return the CPU temperature of platform diode

--*/
{
  UINT8                               CurrentTemperatureOfDiode=0;
  EFI_STATUS                          Status;

  //
  // Call KSC to get Diode Temperature
  //

  while (CurrentTemperatureOfDiode == 0) {
    Status = SendKscCommand (KSC_C_GET_DTEMP);
    if (Status == EFI_SUCCESS) {
      ReceiveKscData ((UINT8 *)&CurrentTemperatureOfDiode);
    }
  }
  return CurrentTemperatureOfDiode;

}

VOID
PlatformEventOutOfSpec (
  VOID
  )
/*++

Routine Description:

  When system temperature out of specification, do platform specific programming to prevent
  system damage.

Arguments:

  None

Returns:

  None

--*/
{
  EFI_STATUS                          Status;
  //
  // Handle critical event by shutting down via EC
  //
  Status = InitializeKscLib ();
  if (Status == EFI_SUCCESS) {
    SendKscCommand (KSC_C_SYSTEM_POWER_OFF);
  }
}


