/** @file
  Declare the platform input for the I2C Port Driver

  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __I2C_PORT_H__
#define __I2C_PORT_H__

#include <Library/UefiLib.h>

///
/// Set the CID value for the I2C driver
/// Use mCidStr if mCid value is zero
///
extern CONST UINT32 mCid;

///
/// Set the CID value for the I2C driver
/// Set to NULL when mCid is non-zero
///
extern CONST CHAR8 *CONST mCidStr;

///
/// Port driver context size
///
extern CONST UINTN mI2cContextLengthInBytes;

///
/// Controller name string table
///
extern CONST EFI_UNICODE_STRING_TABLE mControllerNameStringTable[];

///
/// Driver name string table
///
extern CONST EFI_UNICODE_STRING_TABLE mDriverNameStringTable[];

///
/// The maximum number of bytes the I2C host controller
/// is able to receive from the I2C bus.
///
extern CONST UINT32 mMaximumReceiveBytes;

///
/// The maximum number of bytes the I2C host controller
/// is able to send on the I2C bus.
///
extern CONST UINT32 mMaximumTransmitBytes;

///
/// The maximum number of bytes in the I2C bus transaction.
///
extern CONST UINT32 mMaximumTotalBytes;


/**
  Set the I2C controller bus clock frequency.

  The software and controller do a best case effort of using the specified
  frequency for the I2C bus.  If the frequency does not match exactly then
  the controller will use a slightly lower frequency for the I2C to avoid
  exceeding the operating conditions for any of the I2C devices on the bus.
  For example if 400 KHz was specified and the controller's divide network
  only supports 402 KHz or 398 KHz then the controller would be set to 398
  KHz.  However if the desired frequency is 400 KHz and the controller only
  supports 1 MHz and 100 KHz then this routine would return EFI_UNSUPPORTED.

  @param[in] This           Address of the library's I2C context structure
  @param[in] PlatformData   Address of the platform configuration data
  @param[in] BusClockHertz  New I2C bus clock frequency in Hertz

  @retval RETURN_SUCCESS      The bus frequency was set successfully.
  @retval RETURN_UNSUPPORTED  The controller does not support this frequency.

**/
RETURN_STATUS
I2cBusFrequencySet (
  IN VOID *This,
  IN CONST VOID *PlatformData,
  IN UINTN BusClockHertz
  );

/**
  Reset the I2C controller and configure it for use

  The controller's I2C bus frequency is set to 100 KHz.

  @param[in] This           Address of the library context structure
  @param[in] PlatformData   Address of the platform configuration data

**/
VOID
I2cReset (
  IN VOID *This,
  IN CONST VOID *PlatformData
  );

/**
  Start an I2C operation on the controller

  This function initiates an I2C operation on the controller.

  N.B. This API supports only one operation, no queuing support
  exists at this layer.

  The operation is performed by selecting the I2C device with its slave
  address and then sending all write data to the I2C device.  If read data
  is requested, a restart is sent followed by the slave address and then
  the read data is clocked into the I2C controller and placed in the read
  buffer.  When the operation completes, the status value is returned and
  then the event is set.

  @param[in]  This          Address of the library context structure
  @param[in]  PlatformData  Address of the platform configuration data
  @param[in]  SlaveAddress  Address of the device on the I2C bus.
  @param[in]  WriteBytes    Number of bytes to send
  @param[in]  WriteBuffer   Address of buffer containing data to send
  @param[in]  ReadBytes     Number of bytes to read
  @param[out] ReadBuffer    Address of buffer to receive data

  @retval RETURN_SUCCESS            The operation completed successfully.
  @retval RETURN_DEVICE_ERROR       There was an I2C error (NACK) during the operation.
                                    This could indicate the slave device is not present.
  @retval RETURN_INVALID_PARAMETER  NULL specified for pConfig
  @retval RETURN_NOT_FOUND          SlaveAddress exceeds maximum address
  @retval RETURN_NOT_READY          I2C bus is busy or operation pending, wait for
                                    the event and then read status.
  @retval RETURN_NO_RESPONSE        The I2C device is not responding to the
                                    slave address.  EFI_DEVICE_ERROR may also be
                                    returned if the controller can not distinguish
                                    when the NACK occurred.
  @retval RETURN_OUT_OF_RESOURCES   Insufficient memory for I2C operation
  @retval RETURN_TIMEOUT            The transaction did not complete within an internally
                                    specified timeout period.

**/
RETURN_STATUS
I2cStartRequest (
  IN  VOID *This,
  IN  CONST VOID *PlatformData,
  IN  UINTN SlaveAddress,
  IN  UINTN WriteBytes,
  IN  UINT8 *WriteBuffer,
  IN  UINTN ReadBytes,
  OUT UINT8 *ReadBuffer,
  IN  UINT32 Timeout
  );

#endif  //  __I2C_PORT_H__
