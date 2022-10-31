/** @file
  Declare the I2C enumeration support

  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __I2C_ENUM_LIB_H__
#define __I2C_ENUM_LIB_H__

#include <Library/UefiLib.h>
#include <Library/DebugLib.h>

#include <Protocol/I2cAcpi.h>
#include <Protocol/I2cBusConfigurationManagement.h>
///
/// "I2Ce"
///
#define I2C_ENUM_SIGNATURE       0x65433249

///
/// I2C bus configuration declaration
///
/// This structure provides information about an I2C bus configuration
/// including:
///  The list of I2C devices added to the I2C bus when this
///  configuration is enabled
///  Platform specific data for the driver
///
///
typedef struct {
  ///
  /// Index of parent I2C bus configuration, specifies the array
  /// index of the next I2C bus configuration that is closer to
  /// the host controller.  Note that I2c bus configuration zero
  /// is attached to the host controller.
  ///
  UINTN ParentI2cBusConfiguration;

  ///
  /// Frequency for this I2C bus configuration in Hertz
  ///
  UINTN I2cBusFrequencyHertz;

  ///
  /// Number of devices on this I2C bus configuration
  ///
  UINTN DeviceCount;

  ///
  /// List of devices on this configuration of the I2C bus
  ///
  CONST EFI_I2C_DEVICE *DeviceList;
} I2C_BUS_CONFIGURATION;


///
///  I2C enumeration interface
///
typedef struct {
  ///
  /// ID of this structure
  ///
  UINTN Signature;

  ///
  /// Enumeration API
  ///
  EFI_I2C_ACPI_PROTOCOL AcpiApi;
  EFI_I2C_BUS_CONFIGURATION_MANAGEMENT_PROTOCOL ConfigApi;

  ///
  /// Maximun number of I2C bus configurations
  ///
  UINTN I2cBusConfigurationCount;

  ///
  /// Array of I2C bus configurations
  ///
  CONST I2C_BUS_CONFIGURATION *I2cBusConfigurationArray;
} I2C_ENUM_CONTEXT;

#define I2C_ENUM_CONTEXT_FROM_PROTOCOL(a) CR (a, I2C_ENUM_CONTEXT, AcpiApi, I2C_ENUM_SIGNATURE) ///< Locate I2C_ENUM_CONTEXT from protocol
#define I2C_ENUM_CONTEXT_FROM_CONFIG_PROTOCOL(a) CR (a, I2C_ENUM_CONTEXT, ConfigApi, I2C_ENUM_SIGNATURE) ///< Locate I2C_ENUM_CONTEXT from protocol


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
  @retval EFI_INVALID_PARAMETER NextDevice was NULL
  @retval EFI_NO_MAPPING        PreviousDevice does not point to a valid
                                EFI_I2C_DEVICE structure.

**/
EFI_STATUS
EFIAPI
I2cEnumerate (
  IN CONST EFI_I2C_ACPI_PROTOCOL *This,
  IN OUT CONST EFI_I2C_DEVICE **Device
  );

#endif  //  __I2C_ENUM_LIB_H__
