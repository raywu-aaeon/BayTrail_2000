/** @file
  Implement the driver binding protocol.

  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cPort.h"


/**
  Verify the controller type

  This routine determines if an I2C controller is available.

  This routine is called by the UEFI driver framework during connect
  processing.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to test.
  @param [in] RemainingDevicePath  Not used.

  @retval EFI_SUCCESS          This driver supports this device.
  @retval other                This driver does not support this device.

**/
EFI_STATUS
EFIAPI
I2cPortDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN EFI_HANDLE Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
  )
{
  ACPI_EXTENDED_HID_DEVICE_PATH *AcpiPath;
  CONST CHAR8 *CidStr;
  CONST EFI_MMIO_DEVICE_PROTOCOL *MmioDevice;
  VOID *Protocol;
  EFI_STATUS Status;

//  DEBUG (( DEBUG_LOAD, "I2cPortDriverSupported entered\r\n" ));

  Status = gBS->OpenProtocol ( Controller,
                               &gEfiMmioDeviceProtocolGuid,
                               (VOID **)&MmioDevice,
                               DriverBinding->DriverBindingHandle,
                               Controller,
                               EFI_OPEN_PROTOCOL_GET_PROTOCOL );
  if ( !EFI_ERROR ( Status )) {
    //
    //  Found an MMIO device
    //  Locate the CID string
    //
    AcpiPath = (ACPI_EXTENDED_HID_DEVICE_PATH *)MmioDevice->AcpiPath;
    CidStr = NULL;
    if ( 0 == AcpiPath->CID ) {
      CidStr = (CHAR8 *)( AcpiPath + 1 );

      //
      //  Skip over the HID string value
      //
      CidStr += AsciiStrLen ( CidStr ) + 1;

      //
      //  Skip over the UID string value
      //
      CidStr += AsciiStrLen ( CidStr ) + 1;
    }

    //
    //  Determine if this device requires the I2C port driver
    //
    if (( AcpiPath->CID != mCid )
        || (( 0 == mCid )
            && ( 0 != AsciiStrCmp ( CidStr, mCidStr )))) {
      //
      //  Not an I2C controller
      //
      Status = EFI_UNSUPPORTED;
    } else {
      DEBUG (( DEBUG_INFO, "I2C controller found\r\n" ));

      //
      //  Determine if the I2C protocol is already running
      //
      Status = gBS->OpenProtocol ( Controller,
                                   &gEfiI2cMasterProtocolGuid,
                                   &Protocol,
                                   DriverBinding->DriverBindingHandle,
                                   Controller,
                                   EFI_OPEN_PROTOCOL_GET_PROTOCOL );
      if ( !EFI_ERROR ( Status )) {
        DEBUG (( DEBUG_INFO, "I2C controller already started\r\n" ));
        Status = EFI_ALREADY_STARTED;
      } else {
        //
        //  The I2C driver is not running yet
        //
        Status = EFI_SUCCESS;
      }
    }
  }

//  DEBUG (( DEBUG_LOAD, "I2cPortDriverSupported exiting, Status: %r\r\n", Status ));

  //
  //  Return success only when an available I2C controller is detected.
  //
  return Status;
}


/**
  Connect to the I2C controller

  This routine initializes an instance of the I2C driver for this
  controller.

  This routine is called by the UEFI driver framework during connect
  processing if the controller passes the tests in I2cPortDriverSupported.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to work with.
  @param [in] RemainingDevicePath  Not used, always produce all possible children.

  @retval EFI_SUCCESS          This driver is added to Controller.
  @retval other                This driver does not support this device.

**/
EFI_STATUS
EFIAPI
I2cPortDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN EFI_HANDLE Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
  )
{
  I2C_PORT_CONTEXT *I2cPort;
  UINTN LengthInBytes;
  CONST EFI_MMIO_DEVICE_PROTOCOL *MmioDevice;
  EFI_STATUS Status;

  DEBUG (( DEBUG_INFO, "\r\nI2cPortDriverStart entered\r\n" ));

  //
  //  Get the MMIO device protocol
  //
  Status = gBS->OpenProtocol ( Controller,
                               &gEfiMmioDeviceProtocolGuid,
                               (VOID **)&MmioDevice,
                               DriverBinding->DriverBindingHandle,
                               Controller,
                               EFI_OPEN_PROTOCOL_BY_DRIVER );
  if ( !EFI_ERROR ( Status )) {
    //
    //  Found the I2C device
    //  Allocate the I2C context structure
    //
    LengthInBytes = sizeof ( *I2cPort ) + mI2cContextLengthInBytes;
    I2cPort = AllocateZeroPool ( LengthInBytes );
    if ( NULL == I2cPort ) {
      DEBUG (( DEBUG_ERROR, "ERROR - No memory for I2C port driver\r\n" ));
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      DEBUG (( DEBUG_POOL | DEBUG_INFO,
               "0x%016lx: I2cPort allocated\r\n",
               (UINT64)((UINTN)I2cPort )));

      //
      //  Initialize the context structure
      //
      I2cPort->Signature = I2C_PORT_SIGNATURE;
      I2cPort->MmioDevice = MmioDevice;
      if ( 0 != mI2cContextLengthInBytes ) {
        I2cPort->Context = (VOID *)( I2cPort + 1 );
      }

      //
      //  Start the driver
      //
      Status = I2cPortApiStart ( I2cPort );
      if ( !EFI_ERROR ( Status )) {
        //
        //  Install the driver protocol
        //
        Status = gBS->InstallMultipleProtocolInterfaces (
                        &Controller,
                        &gEfiI2cMasterProtocolGuid,
                        &I2cPort->MasterApi,
                        NULL
                        );
        if ( !EFI_ERROR ( Status )) {
          DEBUG (( DEBUG_INIT,
                   "0x%016lx: I2cPort started\r\n",
                   (UINT64)(UINTN)I2cPort ));
        } else {
          //
          //  Release the API resources upon failure
          //
          I2cPortApiStop ( I2cPort );
        }
      }

      //
      //  Release the context structure upon failure
      //
      if ( EFI_ERROR ( Status )) {
        DEBUG (( DEBUG_WARN, "WARNING - Failed to start I2C port, Status: %r\r\n",
                 Status ));
        DEBUG (( DEBUG_POOL | DEBUG_INFO,
                 "0x%016lx: I2cPort released\r\n",
                 (UINT64)(UINTN)I2cPort ));
        FreePool ( I2cPort );
      }
    }
  }

  DEBUG (( DEBUG_INFO, "I2cPortDriverStart exiting, Status: %r\r\n", Status ));

  //
  //  Return the operation status.
  //
  return Status;
}


/**
  Disconnect from the I2C port controller.

  This routine disconnects from the I2C controller.

  This routine is called by DriverUnload when the I2C port driver
  is being unloaded.

  @param [in] DriverBinding        Protocol instance pointer.
  @param [in] Controller           Handle of device to stop driver on.
  @param [in] NumberOfChildren     How many children need to be stopped.
  @param [in] ChildHandleBuffer    Not used.

  @retval EFI_SUCCESS          This driver is removed Controller.
  @retval EFI_DEVICE_ERROR     The device could not be stopped due to a device error.
  @retval other                This driver was not removed from this device.

**/
EFI_STATUS
EFIAPI
I2cPortDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN  EFI_HANDLE Controller,
  IN  UINTN NumberOfChildren,
  IN  EFI_HANDLE *ChildHandleBuffer
  )
{
  I2C_PORT_CONTEXT *I2cPort;
  EFI_I2C_MASTER_PROTOCOL *I2cMasterProtocol;
  EFI_STATUS Status;

  DEBUG (( DEBUG_INFO, "I2cPortDriverStop entered\r\n" ));

  //
  //  Disconnect any connected drivers and locate the context
  //  structure
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiI2cMasterProtocolGuid,
                  (VOID**)&I2cMasterProtocol,
                  DriverBinding->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE );
  if ( !EFI_ERROR ( Status )) {
    I2cPort = I2C_PORT_CONTEXT_FROM_MASTER_PROTOCOL ( I2cMasterProtocol );

    //
    //  Done with the i2C port protocol
    //
    Status = gBS->CloseProtocol ( Controller,
                                  &gEfiI2cMasterProtocolGuid,
                                  DriverBinding->DriverBindingHandle,
                                  Controller );
    if ( !EFI_ERROR ( Status )) {
      //
      //  Remove the I2C port protocol
      //
      Status = gBS->UninstallMultipleProtocolInterfaces (
                    Controller,
                    &gEfiI2cMasterProtocolGuid,
                    I2cMasterProtocol,
                    NULL );
      if ( !EFI_ERROR ( Status )) {
        //
        //  Stop the driver
        //
        DEBUG (( DEBUG_INIT,
                 "0x%016lx: I2cPort stopped\r\n",
                 (UINT64)(UINTN)I2cPort ));
        I2cPortApiStop ( I2cPort );

        //
        //  Release the I2C controller
        //
        gBS->CloseProtocol ( Controller,
                             &gEfiMmioDeviceProtocolGuid,
                             DriverBinding->DriverBindingHandle,
                             Controller );

        //
        //  Release the context
        //
        DEBUG (( DEBUG_POOL | DEBUG_INFO,
                 "0x%016lx: I2cPort released\r\n",
                 (UINT64)(UINTN)I2cPort ));
        FreePool ( I2cPort );
      } else {
        DEBUG (( DEBUG_ERROR,
                 "ERROR - Failed to uninstall I2C master protocol, Status: %r\r\n",
                 Status ));
      }
    } else {
      DEBUG (( DEBUG_ERROR,
               "ERROR - Failed to close I2C master protocol, Status: %r\r\n",
               Status ));
    }
  }

  DEBUG (( DEBUG_INFO, "I2cPortDriverStop exiting, Status: %r\r\n", Status ));

  //
  //  Return the stop status
  //
  return Status;
}


/**
  Driver binding protocol support
**/
EFI_DRIVER_BINDING_PROTOCOL mI2cPortDriverBinding = {
  I2cPortDriverSupported,
  I2cPortDriverStart,
  I2cPortDriverStop,
  0x10,
  NULL,
  NULL
};
