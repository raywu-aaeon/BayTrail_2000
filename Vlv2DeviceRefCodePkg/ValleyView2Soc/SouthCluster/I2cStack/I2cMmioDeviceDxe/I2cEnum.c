/** @file

  Implement the I2c enumeration Acpi protocol

  Copyright (c) 2012-2013, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#include <Uefi.h>
#include "I2cEnumLib.h"

/**
  Enumerate the I2C devices

  This routine must be called at or below TPL_NOTIFY.

  This function walks the platform specific data to enumerates the
  I2C devices on an I2C bus.

  @param[in]  This              Address of an EFI_I2C_ACPI_PROTOCOL
                                structure.
  @param[in]  Device            Buffer containing the address of an
                                EFI_I2C_DEVICE structure.  Enumeration
                                is started by setting the initial
                                EFI_I2C_DEVICE structure address to NULL.
                                The buffer receives an EFI_I2C_DEVICE
                                structure address for the next I2C device.

  @retval EFI_SUCCESS           The platform data for the next device
                                on the I2C bus was returned successfully.
  @retval EFI_INVALID_PARAMETER Device was NULL
  @retval EFI_NO_MAPPING        Device does not point to a valid
                                EFI_I2C_DEVICE structure.

**/
EFI_STATUS
EFIAPI
I2cEnumerate (
  IN CONST EFI_I2C_ACPI_PROTOCOL *This,
  IN OUT CONST EFI_I2C_DEVICE **Device
  )
{
  CONST I2C_BUS_CONFIGURATION *I2cBusConfiguration;
  CONST I2C_BUS_CONFIGURATION *I2cBusConfigurationEnd;
  CONST EFI_I2C_DEVICE *NextDevice;
  CONST EFI_I2C_DEVICE *NextDeviceEnd;
  BOOLEAN Found;
  I2C_ENUM_CONTEXT *I2cEnumContext;
  CONST EFI_I2C_DEVICE *PreviousDevice;
  EFI_STATUS Status;

  //
  //  Assume the device is not found
  //
  Status = EFI_NO_MAPPING;

  //
  //  Validate the return device
  //
  if ( NULL == Device ) {
    Status = EFI_INVALID_PARAMETER;
  } else {
    //
    //  Walk the list of I2C bus configurations
    //
    I2cEnumContext = I2C_ENUM_CONTEXT_FROM_PROTOCOL ( This );
    I2cBusConfiguration = I2cEnumContext->I2cBusConfigurationArray;
    I2cBusConfigurationEnd = &I2cBusConfiguration [ I2cEnumContext->I2cBusConfigurationCount ];
    Found = FALSE;
    NextDevice = NULL;
    PreviousDevice = *Device;
    while ( I2cBusConfigurationEnd > I2cBusConfiguration ) {
      //
      //  Walk the list of devices
      //
      NextDevice = I2cBusConfiguration->DeviceList;
      NextDeviceEnd = &NextDevice [ I2cBusConfiguration->DeviceCount ];
      while ( NextDeviceEnd > NextDevice ) {
        //
        //  Attempt to locate the current device
        //
        if ( PreviousDevice == NextDevice ) {
          //
          //  Return the next device
          //
          Found = TRUE;
        } else {
          if (( NULL == PreviousDevice ) || Found ) {
            //
            //  Return this device
            //
            break;
          }
        }

        //
        //  Set the next device
        //
        NextDevice += 1;
      }
      if ( NextDeviceEnd > NextDevice ) {
        break;
      }
      NextDevice = NULL;

      //
      //  Set the next I2C bus configuration
      //
      I2cBusConfiguration += 1;
    }

    //
    //  Return this device
    //
    *Device = NextDevice;
    Status = EFI_SUCCESS;
  }

  //
  //  Return the operation status
  //
  return Status;
}
