/** @file
  Configure the I2C buses

  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cMmioConfig.h"

//----------------------------------------------------------------------
//  ACPI paths
//----------------------------------------------------------------------

UINT8 mAcpi_TEST1_100K [ ] = {
  ACPI_DEVICE_PATH,
  ACPI_EXTENDED_DP,
  41, 0,
  0, 0, 0, 0,
  1, 0, 0, 0,
  0, 0, 0, 0,
  'I', '2', 'C', '0', '1', '\\', 'S', 'F', 'F', 'F', 'F', 0,
  0,
  'I', '2', 'C', '0', '1', '\\', 'S', 'F', 'F', 'F', 'F', 0
};

UINT8 mAcpi_TEST1_400K [ ] = {
  ACPI_DEVICE_PATH,
  ACPI_EXTENDED_DP,
  51, 0,
  0, 0, 0, 0,
  1, 0, 0, 0,
  0, 0, 0, 0,
  'I', '2', 'C', '0', '1', '\\', 'S', 'F', 'F', 'F', 'F', '\\', '4', '0', '0', 'K',  0,
  0,
  'I', '2', 'C', '0', '1', '\\', 'S', 'F', 'F', 'F', 'F', '\\', '4', '0', '0', 'K',  0,
};

//----------------------------------------------------------------------
//  I2C devices
//----------------------------------------------------------------------



CONST UINT32 mSlave_TEST1_100K[] = {
  0xFFFF
};

CONST UINT32 mSlave_TEST1_400K[] = {
  0xFFFF
};


CONST EFI_I2C_DEVICE mI2c1DeviceList_100K [ ] = {
//  HRV, Device Path                                              Seg   Cnt                        Addr
  {  1,  (CONST EFI_DEVICE_PATH_PROTOCOL *)&mAcpi_TEST1_100K[ 0 ],  0, DIM ( mSlave_TEST1_100K),   &mSlave_TEST1_100K[ 0 ] }   //  TEST device, 100K
};

CONST EFI_I2C_DEVICE mI2c1DeviceList_400K [ ] = {
//  HRV, Device Path                                              Seg   Cnt                        Addr
  {  1,  (CONST EFI_DEVICE_PATH_PROTOCOL *)&mAcpi_TEST1_400K[ 0 ],  1, DIM ( mSlave_TEST1_400K),   &mSlave_TEST1_400K[ 0 ] }   //  TEST device, 400K
};

//----------------------------------------------------------------------
//  Bus configurations
//----------------------------------------------------------------------

CONST I2C_BUS_CONFIGURATION mI2c1BusConfiguration [ ] = {
  {
    0,                      //  Parent I2C bus configuration
    100000,                    //  Max I2C bus frequency in Hz
    DIM ( mI2c1DeviceList_100K ),//  Number of devices in the bus configuration
    &mI2c1DeviceList_100K [ 0 ]  //  List of devices in the bus configuration
  },
  {
    0,                      //  Parent I2C bus configuration
    400000,                    //  Max I2C bus frequency in Hz
    DIM ( mI2c1DeviceList_400K ),//  Number of devices in the bus configuration
    &mI2c1DeviceList_400K [ 0 ]  //  List of devices in the bus configuration
  }
};

//----------------------------------------------------------------------
//  I2C enumeration support
//----------------------------------------------------------------------

I2C_ENUM_CONTEXT mI2c1BusEnum = {
  I2C_ENUM_SIGNATURE,
  &I2cEnumerate,
  {
    &I2cBusConfiguration,
    2
  },
  DIM ( mI2c1BusConfiguration ),
  &mI2c1BusConfiguration [ 0 ]
};

//----------------------------------------------------------------------
//  I2C controller configuration
//----------------------------------------------------------------------

CONST I2C_PIO_PLATFORM_CONTEXT mI2c1ControllerConfig = {
  0x0ULL,          //  Base address
  100 * 1000 * 1000       //  Input frequency in Hertz
};

void patchI2c1AcpiPath()
{
  mAcpi_TEST1_100K[2]   = sizeof(mAcpi_TEST1_100K);
  mAcpi_TEST1_400K[2]   = sizeof(mAcpi_TEST1_400K);
}
