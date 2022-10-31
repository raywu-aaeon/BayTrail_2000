/** @file
  Implement the I2C bus protocol.

  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cBus.h"


/**
  Enumerate the I2C bus

  This routine walks the platform specific data describing the
  I2C bus to create the I2C devices where driver GUIDs were
  specified.

  @param[in] I2cBus       Address of an I2C_BUS_CONTEXT structure
  @param[in] Controller   Handle to the controller

  @retval EFI_SUCCESS     The bus is successfully configured

**/
EFI_STATUS
I2cBusEnumerate (
  IN I2C_BUS_CONTEXT *I2cBus,
  IN EFI_HANDLE Controller
  )
{
  CONST EFI_I2C_DEVICE *Device;
  EFI_HANDLE Handle;
  I2C_DEVICE_CONTEXT *I2cDevice;
  I2C_DEVICE_CONTEXT *I2cDevicePrevious;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusEnumerate entered\r\n" ));

  //
  //  Assume success
  //
  Status = EFI_SUCCESS;

  //
  //  Walk the list of I2C devices on this bus
  //
  Device = NULL;
  do {
    //
    //  Get the next I2C device
    //
    Status = I2cBus->I2cAcpi->Enumerate ( I2cBus->I2cAcpi,
                                          &Device );
    if ( EFI_ERROR ( Status )) {
      if ( EFI_NO_MAPPING != Status ) {
        break;
      }
      Device = NULL;
      Status = EFI_SUCCESS;
    }
    if ( NULL != Device ) {
      //
      //  Another device is on the I2C bus
      //  Determine if the device info is valid
      //
      if (( NULL != Device->DevicePath )
          && ( sizeof ( *Device->DevicePath ) < DevicePathNodeLength ( Device->DevicePath ))
          && ( 0 < Device->SlaveAddressCount )
          && ( NULL != Device->SlaveAddressArray )) {
        //
        //  Allocate the I2C device context
        //
        I2cDevice = AllocateZeroPool ( sizeof ( *I2cDevice ));
        DEBUG (( DEBUG_I2C_DEVICE,
                 "0x%016Lx: Bus allocating I2C Device 0x%016Lx\r\n",
                 (UINT64)(UINTN)I2cBus,
                 (UINT64)(UINTN)I2cDevice
               ));
        if ( NULL == I2cDevice ) {
          DEBUG (( DEBUG_ERROR, "ERROR - No memory for I2C device structure!\r\n" ));
          Status = EFI_OUT_OF_RESOURCES;
          break;
        }

        //
        //  Initialize the device context
        //
        I2cDevice->Signature = I2C_DEVICE_SIGNATURE;
        I2cDevice->I2cBus = I2cBus;
        I2cDevice->Device = Device;

        //
        //  Build the I/O protocol
        //
        I2cDevice->BusApi.StartRequest = &I2cBusStartRequest;
        I2cDevice->BusApi.HardwareRevision = Device->HardwareRevision;
        I2cDevice->BusApi.SlaveAddressCount = Device->SlaveAddressCount;
        I2cDevice->BusApi.MaximumReceiveBytes = I2cBus->I2cHost->MaximumReceiveBytes;
        I2cDevice->BusApi.MaximumTransmitBytes = I2cBus->I2cHost->MaximumTransmitBytes;
        I2cDevice->BusApi.MaximumTotalBytes = I2cBus->I2cHost->MaximumTotalBytes;

        //
        //  Build the device path
        //
        Status = I2cBusDevicePathAppend ( I2cDevice, Controller );
        if ( EFI_ERROR ( Status )) {
          //
          //  Out of resources
          //
          break;
        }

        //
        //  Install the protocol
        //
        Handle = NULL;
        Status = gBS->InstallMultipleProtocolInterfaces (
                        &Handle,
                        &gEfiI2cBusProtocolGuid,
                        &I2cDevice->BusApi,
                        &gEfiDevicePathProtocolGuid,
                        I2cDevice->DevPath,
                        NULL );
        if ( EFI_ERROR ( Status )) {
          DEBUG (( DEBUG_ERROR,
                   "ERROR - Failed to install the device protocol, Status: %r\r\n",
                   Status ));
          break;
        } else {
          //
          //  Synchronize with the other threads
          //
          TplPrevious = gBS->RaiseTPL ( TPL_I2C_SYNC );

          //
          //  Add this device to the device list
          //
          I2cDevicePrevious = I2cBus->DeviceListTail;
          if ( NULL == I2cDevicePrevious ) {
            I2cBus->DeviceListHead = I2cDevice;
          } else {
            I2cDevicePrevious->Next = I2cDevice;
          }
          I2cBus->DeviceListTail = I2cDevice;

          //
          //  Release the thread synchronization
          //
          gBS->RestoreTPL ( TplPrevious );
        }
      }
    }
  } while ( NULL != Device );

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusEnumerate exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Perform an I2C operation on the device

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  N.B. The typical consumers of this API are the I2C bus driver and
  on rare occasions the I2C test application.  Extreme care must be
  taken by other consumers of this API to prevent confusing the
  third party I2C drivers due to a state change at the I2C device
  which the third party I2C drivers did not initiate.  I2C platform
  drivers may use this API within these guidelines.

  This routine queues an operation to the I2C controller for execution
  on the I2C bus.

  As an upper layer driver writer, the following need to be provided
  to the platform vendor:

  1.  ACPI CID value or string - this is used to connect the upper layer
      driver to the device.
  2.  Slave address array guidance when the I2C device uses more than one
      slave address.  This is used to access the blocks of hardware within
      the I2C device.

  @param[in] This               Address of an EFI_I2C_BUS_PROTOCOL
                                structure
  @param[in] SlaveAddressIndex  Index into an array of slave addresses for
                                the I2C device.  The values in the array are
                                specified by the board designer, with the
                                driver writer providing the slave address
                                order.  For devices that have a single
                                slave address, this value must be zero.
                                If the I2C device uses more than one slave
                                address then the upper level driver writer
                                needs to specify the order of entries in the
                                slave address array.
  @param[in] Event              Event to set for asynchronous operations,
                                NULL for synchronous operations
  @param[in] RequestPacket      Address of an EFI_I2C_REQUEST_PACKET
                                structure describing the I2C operation
  @param[out] I2cStatus         Optional buffer to receive the I2C operation
                                completion status

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ABORTED           The request did not complete because the driver
                                was shutdown.
  @retval EFI_ACCESS_DENIED     Invalid SlaveAddressIndex value
  @retval EFI_BAD_BUFFER_SIZE   The WriteBytes or ReadBytes buffer size is too large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
                                This could indicate the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_INVALID_PARAMETER TPL is too high
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller can not distinguish
                                when the NACK occurred.
  @retval EFI_NOT_FOUND         I2C slave address exceeds maximum address
  @retval EFI_NOT_READY         I2C bus is busy or operation pending, wait for
                                the event and then read status pointed to by
                                the request packet.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.

**/
EFI_STATUS
EFIAPI
I2cBusStartRequest (
  IN CONST EFI_I2C_BUS_PROTOCOL *This,
  IN UINTN SlaveAddressIndex,
  IN EFI_EVENT Event OPTIONAL,
  IN CONST EFI_I2C_REQUEST_PACKET *RequestPacket,
  OUT EFI_STATUS *I2cStatus OPTIONAL
  )
{
  CONST EFI_I2C_DEVICE *Device;
  I2C_BUS_CONTEXT *I2cBus;
  CONST EFI_I2C_HOST_PROTOCOL *I2cHost;
  I2C_DEVICE_CONTEXT *I2cDevice;
  EFI_STATUS Status;

  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusStartRequest entered\r\n" ));

  //
  //  Assume failure
  //
  Status = EFI_ACCESS_DENIED;

  //
  //  Validate the I2C slave index
  //
  I2cDevice = I2C_DEVICE_CONTEXT_FROM_PROTOCOL ( This );
  if ( SlaveAddressIndex < I2cDevice->Device->SlaveAddressCount ||
       SlaveAddressIndex & 0x400
     ) {
    //
    //  Locate the host protocol
    //
    I2cBus = I2cDevice->I2cBus;
    I2cHost = I2cBus->I2cHost;
    Device = I2cDevice->Device;

    //
    //  Display the operation
    //
    DEBUG (( DEBUG_I2C_OPERATION,
             "I2cBus: %d\r\nSlaveAddress: %d\r\nWriteBytes: %d\r\nWriteBuffer: 0x%016Lx\r\nReadBytes: %d\r\nReadBuffer: 0x%016Lx\r\nTimeout: %d\r\n",
             Device->I2cBusConfiguration,
             (SlaveAddressIndex & 0x400) ? (SlaveAddressIndex & 0x3FF) : Device->SlaveAddressArray [ SlaveAddressIndex ],
             RequestPacket->WriteBytes,
             (UINT64)(UINTN)RequestPacket->WriteBuffer,
             RequestPacket->ReadBytes,
             (UINT64)(UINTN)RequestPacket->ReadBuffer,
             RequestPacket->Timeout ));

    //
    //  Start the I2C operation
    //
    //If the 12th bit of SlaveAddressIndex is set, it indicates this call is for debug.
    Status = I2cHost->QueueRequest ( I2cHost,
                                     Device->I2cBusConfiguration,
                                     (SlaveAddressIndex & 0x400) ? (SlaveAddressIndex & 0x3FF) : Device->SlaveAddressArray [ SlaveAddressIndex ],
                                     Event,
                                     RequestPacket,
                                     I2cStatus );
  }
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusStartRequest exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Start the I2C driver

  This routine allocates the necessary resources for the driver.

  This routine is called by I2cBusDriverStart to complete the driver
  initialization.

  @param [in] I2cBus          Address of an I2C_BUS_CONTEXT structure
  @param[in] Controller       Handle to the controller

  @retval EFI_SUCCESS         Driver API properly initialized

**/
EFI_STATUS
I2cBusApiStart (
  IN I2C_BUS_CONTEXT *I2cBus,
  IN EFI_HANDLE Controller
  )
{
  EFI_STATUS Status;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusApiStart entered\r\n" ));

  //
  //  Build the I2C bus protocol
  //  Enumerate the I2C bus
  //
  Status = I2cBusEnumerate ( I2cBus, Controller );

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusApiStart exiting, Status: %r\r\n", Status ));

  //
  //  Return the startup status
  //
  return Status;
}


/**
  Stop the I2C driver

  This routine releases the resources allocated by I2cBusApiStart.

  This routine is called by I2cBusDriverStop to initiate the driver
  shutdown.

  @param [in] I2cBus          Address of an I2C_BUS_CONTEXT structure

**/
VOID
I2cBusApiStop (
  IN I2C_BUS_CONTEXT *I2cBus
  )
{
  EFI_HANDLE *Handle;
  EFI_HANDLE *HandleArray;
  EFI_HANDLE *HandleArrayEnd;
  UINTN HandleCount;
  I2C_DEVICE_CONTEXT *I2cDevice;
  I2C_DEVICE_CONTEXT *I2cDevicePrevious;
  EFI_I2C_BUS_PROTOCOL *I2cBusProtocol;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Display entry
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusApiStop entered\r\n" ));

  //
  //  Locate the I2C devices
  //
  Status = gBS->LocateHandleBuffer ( ByProtocol,
                                     &gEfiI2cBusProtocolGuid,
                                     NULL,
                                     &HandleCount,
                                     &HandleArray );
  if ( !EFI_ERROR ( Status ))  {
    Handle = HandleArray;
    HandleArrayEnd = &Handle [ HandleCount ];
    while ( HandleArrayEnd > Handle ) {
      //
      //  Remove the driver stack
      //
      Status = gBS->OpenProtocol ( *Handle,
                                   &gEfiI2cBusProtocolGuid,
                                   (VOID **)&I2cBusProtocol,
                                   mI2cBusDriverBinding.DriverBindingHandle,
                                   *Handle,
                                   EFI_OPEN_PROTOCOL_BY_DRIVER | EFI_OPEN_PROTOCOL_EXCLUSIVE );
      if ( EFI_ERROR ( Status )) {
        DEBUG (( DEBUG_ERROR,
                 "ERROR - Failed to remove I2C driver stack, Status: %r\r\n",
                 Status ));
      } else {
        //
        //  Done with the protocol
        //
        I2cDevice = I2C_DEVICE_CONTEXT_FROM_PROTOCOL ( I2cBusProtocol );
        gBS->CloseProtocol ( *Handle,
                             &gEfiI2cBusProtocolGuid,
                             mI2cBusDriverBinding.DriverBindingHandle,
                             *Handle );
        if(I2cDevice->I2cBus != I2cBus) {
          //
          // This I2cDevice is on other I2cBus
          //
          Handle += 1;
          continue;
        }

        //
        //  Remove this protocol
        //
        Status = gBS->UninstallMultipleProtocolInterfaces ( *Handle,
                 &gEfiI2cBusProtocolGuid,
                 &I2cDevice->BusApi,
                 &gEfiDevicePathProtocolGuid,
                 I2cDevice->DevPath,
                 NULL );
        if ( EFI_ERROR ( Status )) {
          DEBUG (( DEBUG_ERROR,
                   "ERROR - Failed to uninstall gEfiI2cBusProtocol, Status: %r\r\n",
                   Status ));
        } else {
          //
          //  Synchronize with the other threads
          //
          TplPrevious = gBS->RaiseTPL ( TPL_I2C_SYNC );

          //
          //  Locate this device in the list
          //
          I2cDevicePrevious = I2cBus->DeviceListHead;
          if ( I2cDevice == I2cDevicePrevious ) {
            //
            //  The device is at the head of the list
            //
            I2cBus->DeviceListHead = I2cDevice->Next;
            I2cDevicePrevious = NULL;
          } else {
            //
            //  Locate the device in the middle of the list
            //
            while ( I2cDevice != I2cDevicePrevious->Next ) {
              I2cDevicePrevious = I2cDevicePrevious->Next;
            }

            //
            //  Remove the device form the middle of the list
            //
            I2cDevicePrevious->Next = I2cDevice->Next;
          }

          //
          //  Remove the device from the end of the list if necessary
          //
          if ( I2cBus->DeviceListTail == I2cDevice ) {
            I2cBus->DeviceListTail = I2cDevicePrevious;
          }

          //
          //  Release the thread synchronization
          //
          gBS->RestoreTPL ( TplPrevious );

          //
          //  Display the device
          //
          DEBUG (( DEBUG_I2C_DEVICE,
                   "0x%016Lx: Bus freeing I2C Device 0x%x on I2C bus configuration %d\r\n",
                   (UINT64)(UINTN)I2cDevice,
                   I2cDevice->Device->SlaveAddressArray [ 0 ],
                   I2cDevice->Device->I2cBusConfiguration ));

          //
          //  Free this device
          //
          FreePool ( I2cDevice );
        }
      }

      //
      //  Set the next handle
      //
      Handle += 1;
    }

    //
    //  Done with the handles
    //
    FreePool ( HandleArray );
  }

  //
  //  Display exit
  //
  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cBusApiStop exiting\r\n" ));
}
