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

#include "I2cHost.h"

/**
  Driver protocol GUID
**/
EFI_GUID *mpDriverProtocol = &gEfiI2cHostProtocolGuid;

/**
  Component Name Protocol support
**/
CONST EFI_COMPONENT_NAME_PROTOCOL mComponentNameProtocol = {
  DlGetDriverName,
  DlGetControllerName,
  "eng"
};

/**
  Controller name string table
**/
static CONST EFI_UNICODE_STRING_TABLE mControllerNameStringTable[] = {
  { "eng", L"I2C Host" },
  { NULL , NULL }
};

/**
  Driver name string table
**/
static CONST EFI_UNICODE_STRING_TABLE mDriverNameStringTable[] = {
  { "eng", L"I2C Host Driver" },
  { NULL , NULL }
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
  &mI2cHostDriverBinding,

  //
  //  Loaded image protocol support
  //
  DlDriverUnload,
};
