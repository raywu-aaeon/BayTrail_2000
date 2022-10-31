/** @file
  Implement the device path extension

  Copyright (c) 2011 - 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cBus.h"
#include <Library/DevicePathLib.h>

/**
  Driver path template
**/

CONST STATIC EFI_DEVICE_PATH_PROTOCOL mEndOfPath = {
  END_DEVICE_PATH_TYPE,
  END_ENTIRE_DEVICE_PATH_SUBTYPE,
  {
    END_DEVICE_PATH_LENGTH,
    0
  }
};


/**
  Create a path for the I2C device

  Append the I2C slave path to the I2C master controller path.

  @param [in] I2cDevice     Address of an I2C_DEVICE_CONTEXT structure.
  @param[in] Controller     Handle to the controller

**/
EFI_STATUS
I2cBusDevicePathAppend (
  IN I2C_DEVICE_CONTEXT *I2cDevice,
  IN EFI_HANDLE Controller
  )
{
  UINT8 *Buffer;
  EFI_DEVICE_PATH_PROTOCOL *DevPath;
  UINTN LengthInBytes;
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusDevicePathAppend entered\r\n" ));

  //
  //  Locate the existing device path
  //
  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID**) &DevPath
                  );
  if ( !EFI_ERROR (Status)) {
    //
    //  Allocate a buffer
    //
    LengthInBytes = DevicePathNodeLength ( I2cDevice->Device->DevicePath );
    Buffer = AllocateZeroPool ( LengthInBytes + sizeof ( mEndOfPath ));
    if ( NULL == Buffer ) {
      DEBUG (( DEBUG_ERROR,
               "ERROR - Failed to allocate device path buffer!\r\n" ));
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      //
      //  Build the new device path
      //
      CopyMem ( Buffer,
                I2cDevice->Device->DevicePath,
                LengthInBytes );
      CopyMem ( &Buffer [ LengthInBytes ],
                &mEndOfPath,
                sizeof ( mEndOfPath ));
      I2cDevice->DevPath = AppendDevicePath ( DevPath,
                                              (EFI_DEVICE_PATH_PROTOCOL *)Buffer );
      if ( NULL == I2cDevice->DevPath ) {
        Status = EFI_OUT_OF_RESOURCES;
      }

      //
      //  Free the buffer
      //
      FreePool ( Buffer );
    }
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusDevicePathAppend exiting, Status: %r\r\n", Status ));
  return Status;
}
