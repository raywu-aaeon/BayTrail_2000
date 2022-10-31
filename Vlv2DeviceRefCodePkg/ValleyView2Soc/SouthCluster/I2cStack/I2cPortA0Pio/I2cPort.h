/** @file
  I2C Port Driver Declarations

  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _I2C_PORT_H
#define _I2C_PORT_H

#include <Uefi.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DriverLib.h>
#include <Library/PcdLib.h>
#include <Protocol/I2cMaster.h>
#include <Protocol/MmioDevice.h>
#include "I2cPortDxe.h"
///
/// Debug I2C operation
///
#define DEBUG_I2C_OPERATION           0x40000000

///
/// Debug routine entry and exit
///
#define DEBUG_I2C_ROUTINE_ENTRY_EXIT  0x20000000

///
/// "I2Cp"
///
#define I2C_PORT_SIGNATURE            0x70433249

extern EFI_DRIVER_BINDING_PROTOCOL mI2cPortDriverBinding;
//extern CONST EFI_I2C_PORT_PROTOCOL mI2cPortProtocol;

///
/// I2C host context
///
/// Each I2C port instance uses an I2C_PORT_CONTEXT structure
/// to maintain its context.
///
typedef struct {
  ///
  /// Structure identification
  ///
  UINTN Signature;

  ///
  /// Upper level API
  ///
  EFI_I2C_MASTER_PROTOCOL MasterApi;

  ///
  /// Library context
  ///
  VOID *Context;

  ///
  /// Platform specific data
  ///
  CONST EFI_MMIO_DEVICE_PROTOCOL *MmioDevice;
} I2C_PORT_CONTEXT;

///
/// Locate I2C_PORT_CONTEXT from protocol
///
#define I2C_PORT_CONTEXT_FROM_MASTER_PROTOCOL(a)  CR (a, I2C_PORT_CONTEXT, MasterApi, I2C_PORT_SIGNATURE)


/**
  Start the I2C port driver

  This routine allocates the necessary resources for the driver.

  This routine is called by I2cPortDriverStart to complete the driver
  initialization.

  @param[in] I2cPort          Address of an I2C_PORT_CONTEXT structure

  @retval EFI_SUCCESS         Driver API properly initialized

**/
EFI_STATUS
I2cPortApiStart (
  IN I2C_PORT_CONTEXT *I2cPort
  );

/**
  Stop the I2C driver

  This routine releases the resources allocated by I2cApiStart.

  This routine is called by I2cPortDriverStop to initiate the driver
  shutdown.

  @param[in] I2cPort          Address of an I2C_PORT_CONTEXT structure

**/
VOID
I2cPortApiStop (
  IN I2C_PORT_CONTEXT *I2cPort
  );

#endif  //  _I2C_PORT_H
