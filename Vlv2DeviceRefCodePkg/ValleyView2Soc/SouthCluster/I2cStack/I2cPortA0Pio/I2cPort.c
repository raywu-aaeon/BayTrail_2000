/** @file
  Implement the I2C host protocol.

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
  Set the I2C controller bus clock frequency.

  This routine must be called at or below TPL_NOTIFY.

  The software and controller do a best case effort of using the specified
  frequency for the I2C bus.  If the frequency does not match exactly then
  the controller will use a slightly lower frequency for the I2C to avoid
  exceeding the operating conditions for any of the I2C devices on the bus.
  For example if 400 KHz was specified and the controller's divide network
  only supports 402 KHz or 398 KHz then the controller would be set to 398
  KHz.  However if the desired frequency is 400 KHz and the controller only
  supports 1 MHz and 100 KHz then this routine would return EFI_UNSUPPORTED.

  @param[in] This           Address of an EFI_I2C_MASTER_PROTOCOL
                            structure
  @param[in] BusClockHertz  New I2C bus clock frequency in Hertz

  @retval EFI_SUCCESS       The bus frequency was set successfully.
  @retval EFI_UNSUPPORTED   The controller does not support this frequency.

**/
EFI_STATUS
EFIAPI
I2cPortBusFrequencySet (
  IN CONST EFI_I2C_MASTER_PROTOCOL *This,
  IN UINTN BusClockHertz
  )
{
  I2C_PORT_CONTEXT *I2cPort;
  EFI_STATUS Status;

  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cPortBusFrequencySet entered\r\n" ));

  //
  //  Locate the configuration
  //
  I2cPort = I2C_PORT_CONTEXT_FROM_MASTER_PROTOCOL ( This );

  //
  //  Display the operation
  //
  DEBUG (( DEBUG_I2C_OPERATION,
           "0x%016Lx: Setting I2C bus frequency to %d KHz\r\n",
           (UINT64)(UINTN)I2cPort,
           BusClockHertz / 1000 ));

  //
  //  Set the I2C bus speed
  //
  Status = I2cBusFrequencySet ( I2cPort->Context,
                                I2cPort->MmioDevice->DriverResources,
                                BusClockHertz );

  DEBUG (( DEBUG_I2C_OPERATION,
           "0x%016Lx: I2C bus frequency set, Status: %r\r\n",
           (UINT64)(UINTN)I2cPort,
           Status ));

  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cPortBusFrequencySet exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Reset the I2C controller and configure it for use

  This routine must be called at or below TPL_NOTIFY.

  The controller's I2C bus frequency is set to 100 KHz.

  @param[in]     This       Address of an EFI_I2C_MASTER_PROTOCOL
                            structure

**/
VOID
EFIAPI
I2cPortReset (
  IN CONST EFI_I2C_MASTER_PROTOCOL *This
  )
{
  I2C_PORT_CONTEXT *I2cPort;

  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cPortReset entered\r\n" ));

  //
  //  Locate the configuration
  //
  I2cPort = I2C_PORT_CONTEXT_FROM_MASTER_PROTOCOL ( This );

  //
  //  Display the operation
  //
  DEBUG (( DEBUG_I2C_OPERATION,
           "0x%016Lx: Resetting I2C controller\r\n",
           (UINT64)(UINTN)I2cPort ));

  //
  //  Reset the host
  //
  I2cReset ( I2cPort->Context,
             I2cPort->MmioDevice->DriverResources );

  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cPortReset exiting\r\n" ));
}


/**
  Start an I2C operation on the controller

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  N.B. The typical consumer of this API is the I2C host driver.
  Extreme care must be taken by other consumers of this API to
  prevent confusing the third party I2C drivers due to a state
  change at the I2C device which the third party I2C drivers did
  not initiate.  I2C platform drivers may use this API within
  these guidelines.

  This function initiates an I2C operation on the controller.

  N.B. This API supports only one operation, no queuing support
  exists at this layer.

  The operation is performed by selecting the I2C device with its slave
  address and then sending all write data to the I2C device.  If read data
  is requested, a restart is sent followed by the slave address and then
  the read data is clocked into the I2C controller and placed in the read
  buffer.  When the operation completes, the status value is returned and
  then the event is set.

  @param[in] This           Address of an EFI_I2C_MASTER_PROTOCOL
                            structure
  @param[in] SlaveAddress   Address of the device on the I2C bus.
  @param[in] Event          Event to set for asynchronous operations,
                            NULL for synchronous operations
  @param[in] RequestPacket  Address of an EFI_I2C_REQUEST_PACKET
                            structure describing the I2C operation
  @param[out] I2cStatus     Optional buffer to receive the I2C operation
                            completion status

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_ABORTED           The request did not complete because the driver
                                was shutdown.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
                                This could indicate the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_INVALID_PARAMETER TPL is too high
  @retval EFI_NOT_FOUND         SlaveAddress exceeds maximum address
  @retval EFI_NOT_READY         I2C bus is busy or operation pending, wait for
                                the event and then read status pointed to by
                                the request packet.
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller can not distinguish
                                when the NACK occurred.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.

**/
EFI_STATUS
EFIAPI
I2cPortMasterStartRequest (
  IN CONST EFI_I2C_MASTER_PROTOCOL *This,
  IN UINTN SlaveAddress,
  IN EFI_EVENT Event OPTIONAL,
  IN CONST EFI_I2C_REQUEST_PACKET *RequestPacket,
  OUT EFI_STATUS *I2cStatus OPTIONAL
  )
{
  I2C_PORT_CONTEXT *I2cPort;
  EFI_STATUS Status;

  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cPortMasterStartRequest entered\r\n" ));

  //
  //  Locate the configuration
  //
  I2cPort = I2C_PORT_CONTEXT_FROM_MASTER_PROTOCOL ( This );

  //
  //  Display the request
  //
  DEBUG (( DEBUG_I2C_OPERATION,
           "0x%016Lx: Starting I2C request packet 0x%016Lx, Slave: 0x%03x\r\n",
           (UINT64)(UINTN)I2cPort,
           (UINT64)(UINTN)RequestPacket,
           SlaveAddress ));
  DEBUG (( DEBUG_I2C_OPERATION,
           "I2cPort\r\n0x  %03x: SlaveAddress\r\n  %d: WriteBytes\r\n  0x%016Lx: WriteBuffer\r\n  %d: ReadBytes\r\n  0x%016Lx: ReadBuffer\r\n  0x%016Lx: Timeout\r\n",
           SlaveAddress,
           RequestPacket->WriteBytes,
           (UINT64)(UINTN)RequestPacket->WriteBuffer,
           RequestPacket->ReadBytes,
           (UINT64)(UINTN)RequestPacket->ReadBuffer,
           (UINT64)RequestPacket->Timeout ));

  //
  //  Validate the buffer sizes
  //
  if (( RequestPacket->WriteBytes > mMaximumTransmitBytes )
      || ( RequestPacket->ReadBytes > mMaximumReceiveBytes )
      || (( RequestPacket->WriteBytes + RequestPacket->ReadBytes ) > mMaximumTotalBytes )) {
    Status = EFI_BAD_BUFFER_SIZE;
  } else {
    //
    //  Start an I2C operation on the host
    //
    Status = I2cStartRequest ( I2cPort->Context,
                               I2cPort->MmioDevice->DriverResources,
                               SlaveAddress,
                               RequestPacket->WriteBytes,
                               RequestPacket->WriteBuffer,
                               RequestPacket->ReadBytes,
                               RequestPacket->ReadBuffer,
                               RequestPacket->Timeout
                               );
  }

  //
  //  Return the status
  //
  if ( NULL != I2cStatus ) {
    DEBUG (( DEBUG_I2C_OPERATION,
             "0x%016Lx: Returning status for I2C request packet 0x%016Lx, Status: %r\r\n",
             (UINT64)(UINTN)I2cPort,
             (UINT64)(UINTN)RequestPacket,
             Status ));
    *I2cStatus = Status;
  }

  //
  //  Set the event
  //
  if ( NULL != Event ) {
    DEBUG (( DEBUG_I2C_OPERATION,
             "0x%016Lx: Signalling event for I2C request packet 0x%016Lx, Event:0x%016Lx\r\n",
             (UINT64)(UINTN)I2cPort,
             (UINT64)(UINTN)RequestPacket,
             (UINT64)(UINTN)Event));
    Status = gBS->SignalEvent ( Event );
    ASSERT ( EFI_SUCCESS == Status );

    //
    //  Fake an asynchronous operation
    //
    Status = EFI_NOT_READY;
  }

  DEBUG (( DEBUG_I2C_ROUTINE_ENTRY_EXIT, "I2cPortMasterStartRequest exiting, Status: %r\r\n", Status ));
  return Status;
}


/**
  Start the I2C driver

  This routine allocates the necessary resources for the driver.

  This routine is called by I2cHostDriverStart to complete the driver
  initialization.

  @param [in] I2cHost         Address of an I2C_HOST_CONTEXT structure

  @retval EFI_SUCCESS         Driver API properly initialized

**/
EFI_STATUS
I2cPortApiStart (
  IN I2C_PORT_CONTEXT *I2cPort
  )
{
  EFI_STATUS Status;

  DEBUG (( DEBUG_LOAD, "I2cPortApiStart entered\r\n" ));

  //
  //  Build the I2C master protocol
  //
  I2cPort->MasterApi.BusFrequencySet = I2cPortBusFrequencySet;
  I2cPort->MasterApi.Reset = I2cPortReset;
  I2cPort->MasterApi.StartRequest = I2cPortMasterStartRequest;
  I2cPort->MasterApi.MaximumReceiveBytes = mMaximumReceiveBytes;
  I2cPort->MasterApi.MaximumTransmitBytes = mMaximumTransmitBytes;
  I2cPort->MasterApi.MaximumTotalBytes = mMaximumTotalBytes;

  //
  //  The port is successfully configured
  //
  Status = EFI_SUCCESS;

  DEBUG (( DEBUG_LOAD, "I2cPortApiStart exiting, Status: %r\r\n", Status ));

  //
  //  Return the startup status
  //
  return Status;
}


/**
  Stop the I2C driver

  This routine releases the resources allocated by I2cApiStart.

  This routine is called by I2cPortDriverStop to initiate the driver
  shutdown.

  @param [in] I2cPort         Address of an I2C_PORT_CONTEXT structure

**/
VOID
I2cPortApiStop (
  IN I2C_PORT_CONTEXT *I2cPort
  )
{
  DEBUG (( DEBUG_LOAD, "I2cPortApiStop entered\r\n" ));
  DEBUG (( DEBUG_LOAD, "I2cPortApiStop exiting\r\n" ));
}
