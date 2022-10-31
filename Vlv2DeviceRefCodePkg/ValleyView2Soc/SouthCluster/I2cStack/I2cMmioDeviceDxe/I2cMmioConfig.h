/** @file

  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _I2C_MMIO_CONFIG_H
#define _I2C_MMIO_CONFIG_H

#include <Uefi.h>
#include <Library/I2cPort_platform.h>
//#include <Library/I2cMmioConfigLib.h>

#include <Protocol/I2cAcpi.h>
#include <Protocol/I2cHost.h>
#include <Protocol/I2cBusConfigurationManagement.h>
#include <Protocol/MmioDevice.h>
#include <Protocol/GlobalNvsArea.h>
#include "I2cEnumLib.h"

#define GLOBAL_NVS_OFFSET(Field)    (UINTN)((CHAR8*)&((EFI_GLOBAL_NVS_AREA*)0)->Field - (CHAR8*)0)
#define EC_BASE           0xE0000000
#define MC_MCR            0x000000D0  // Cunit Message Control Register
#define MC_MDR            0x000000D4  // Cunit Message Data Register
#define MC_MCRX           0x000000D8  // Cunit Message Control Register Extension
#define MSG_BUS_ENABLED   0x000000F0
#define MSGBUS_MASKHI     0xFFFFFF00
#define MSGBUS_MASKLO     0x000000FF
#define MESSAGE_DWORD_EN  BIT4 | BIT5 | BIT6 | BIT7

typedef struct _I2C_DEVICE_INFO {
  UINTN        Segment;
  UINTN        BusNum;
  UINTN        DeviceNum;
  UINTN        FunctionNum;
  UINTN        GNVSOffset;
} I2C_DEVICE_INFO;


//----------------------------------------------------------------------
//  Constants
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//  Macros
//----------------------------------------------------------------------

/**
  Macro to compute the number of elements in an array.
**/
#define DIM(x)    ( sizeof ( x ) / sizeof ( x[0] ))

//----------------------------------------------------------------------
//  External data
//----------------------------------------------------------------------

extern CONST EFI_MMIO_DEVICE_PROTOCOL gMmioDeviceList [ ];

/**
  I2C controller configurations
**/

extern CONST I2C_PIO_PLATFORM_CONTEXT mI2c0ControllerConfig;
extern CONST I2C_PIO_PLATFORM_CONTEXT mI2c1ControllerConfig;
extern CONST I2C_PIO_PLATFORM_CONTEXT mI2c2ControllerConfig;
extern CONST I2C_PIO_PLATFORM_CONTEXT mI2c3ControllerConfig;
extern CONST I2C_PIO_PLATFORM_CONTEXT mI2c4ControllerConfig;
extern CONST I2C_PIO_PLATFORM_CONTEXT mI2c5ControllerConfig;
extern CONST I2C_PIO_PLATFORM_CONTEXT mI2c6ControllerConfig;



extern void patchI2c0AcpiPath();
extern void patchI2c1AcpiPath();
extern void patchI2c2AcpiPath();
extern void patchI2c3AcpiPath();
extern void patchI2c4AcpiPath();
extern void patchI2c5AcpiPath();
extern void patchI2c6AcpiPath();





/**
  I2C bus enumeration support
**/

extern I2C_ENUM_CONTEXT mI2c0BusEnum;
extern I2C_ENUM_CONTEXT mI2c1BusEnum;
extern I2C_ENUM_CONTEXT mI2c2BusEnum;
extern I2C_ENUM_CONTEXT mI2c3BusEnum;
extern I2C_ENUM_CONTEXT mI2c4BusEnum;
extern I2C_ENUM_CONTEXT mI2c5BusEnum;
extern I2C_ENUM_CONTEXT mI2c6BusEnum;


//----------------------------------------------------------------------
//  Support routines
//----------------------------------------------------------------------

/**
  Enable access to an I2C bus configuration.

  This routine must be called at or below TPL_NOTIFY.  For synchronous
  requests this routine must be called at or below TPL_CALLBACK.

  Reconfigure the switches and multiplexers in the I2C bus to enable
  access to a specific I2C bus configuration.  Also select the maximum
  clock frequency for this I2C bus configuration.

  This routine uses the I2C Master protocol when the platform routine
  needs to perform I2C operations on the local bus.  This eliminates
  any recursion in the I2C stack for configuration operations on the
  local bus.  This works because the local I2C bus is idle while the
  I2C bus configuration is being enabled.

  The platform layer must perform I2C operations on other I2C busses
  by using the EFI_I2C_HOST_PROTOCOL or third party driver interface
  for the specific device.  This requirement is because the I2C host
  driver controls the flow of requests to the I2C controller.  Use the
  EFI_I2C_HOST_PROTOCOL when the device is not defined in the platform's
  ACPI tables.  Use the third party driver when it is available or
  EFI_I2C_IO_PROTOCOL when the third party driver is not available
  but the device is defined in the platform's ACPI tables.

  @param[in]  This            Address of an EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL
                              structure
  @param[in]  I2cBusConfiguration Index value into a list or array of I2C bus
                                  configurations.
  @param[in]  Event           Event to set when the operation is
                              complete.
  @param[out] Status          Buffer to receive the operation
                              status.

  @return  When Event is NULL, EnableI2cBusConfiguration operates synchrouously
  and returns the I2C completion status as its return value.  In this case it is
  recommended to use NULL for I2cStatus.  The values returned from
  EnableI2cBusConfiguration are:

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_BAD_BUFFER_SIZE   The WriteBytes or ReadBytes buffer size is too large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
                                One possible cause is that the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_INVALID_PARAMETER TPL is too high
  @retval EFI_LOAD_ERROR        Controller handle is NULL
  @retval EFI_NO_MAPPING        Invalid I2cBusConfiguration value
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller cannot distinguish
                                when the NACK occurred.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.

  @return   When Event is not NULL, EnableI2cBusConfiguration synchronously returns
  EFI_NOT_READY indicating that the I2C operation was started asynchronously.  The
  following values are returned upon the completion of the I2C operation when I2cStatus
  is not NULL:

  @retval EFI_SUCCESS           The operation completed successfully.
  @retval EFI_BAD_BUFFER_SIZE   The WriteBytes or ReadBytes buffer size is too large.
  @retval EFI_DEVICE_ERROR      There was an I2C error (NACK) during the operation.
                                One possible cause is that the slave device is not present.
  @retval EFI_INVALID_PARAMETER RequestPacket is NULL
  @retval EFI_INVALID_PARAMETER TPL is too high
  @retval EFI_LOAD_ERROR        Controller handle is NULL
  @retval EFI_NO_MAPPING        Invalid I2cBusConfiguration value
  @retval EFI_NO_RESPONSE       The I2C device is not responding to the
                                slave address.  EFI_DEVICE_ERROR may also be
                                returned if the controller cannot distinguish
                                when the NACK occurred.
  @retval EFI_OUT_OF_RESOURCES  Insufficient memory for I2C operation
  @retval EFI_TIMEOUT           The transaction did not complete within an internally
                                specified timeout period.

**/
EFI_STATUS
EFIAPI
I2cBusConfiguration (
  IN CONST EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL *This,
  IN UINTN I2cBusConfiguration,
  IN CONST EFI_I2C_MASTER_PROTOCOL * I2cMaster,
  IN EFI_EVENT Event OPTIONAL,
  IN EFI_STATUS *Status OPTIONAL
  );



//----------------------------------------------------------------------

#endif  //  _I2C_MMIO_CONFIG_H
