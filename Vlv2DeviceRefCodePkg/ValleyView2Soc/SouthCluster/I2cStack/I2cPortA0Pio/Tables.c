/** @file
  Driver tables
  
  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cPortDxe.h"
#include "I2cPort.h"

///
/// Set the FIFO size
///
#define FIFO_SIZE_IN_BYTES    (UINT32)-1

///
/// Set the CID value for the I2C driver
/// Use mCidStr if mCid value is zero
///
CONST UINT32 mCid = 0;

///
/// Set the CID value for the I2C driver
/// Set to NULL when mCid is non-zero
///
CONST CHAR8 *CONST mCidStr = "INTC33B1";

///
/// Controller name string table
///
CONST EFI_UNICODE_STRING_TABLE mControllerNameStringTable[] = {
  { "eng", L"I2C Port A0" },
  { NULL , NULL }
};

///
/// Driver name string table
///
CONST EFI_UNICODE_STRING_TABLE mDriverNameStringTable[] = {
  { "eng", L"I2C Port A0 PIO Driver" },
  { NULL , NULL }
};

///
/// The maximum number of bytes the I2C host controller
/// is able to receive from the I2C bus.
///
CONST UINT32 mMaximumReceiveBytes = FIFO_SIZE_IN_BYTES;

///
/// The maximum number of bytes the I2C host controller
/// is able to send on the I2C bus.
///
CONST UINT32 mMaximumTransmitBytes = FIFO_SIZE_IN_BYTES;

///
/// The maximum number of bytes in the I2C bus transaction.
///
CONST UINT32 mMaximumTotalBytes = FIFO_SIZE_IN_BYTES;


/**
  Driver protocol GUID
**/
EFI_GUID *mpDriverProtocol = &gEfiI2cMasterProtocolGuid;

/**
  Component Name Protocol support
**/
CONST EFI_COMPONENT_NAME_PROTOCOL mComponentNameProtocol = {
  DlGetDriverName,
  DlGetControllerName,
  "eng"
};

/**
  Driver library support
**/
CONST DL_DRIVER_LIB mDriverLib = {
  //
  //  Component name protocol support
  //
  &mComponentNameProtocol,
  NULL,
  &mControllerNameStringTable[0],
  &mDriverNameStringTable[0],

  //
  //  Driver binding protocol support
  //
  &mI2cPortDriverBinding,

  //
  //  Loaded image protocol support
  //
  DlDriverUnload,
};
