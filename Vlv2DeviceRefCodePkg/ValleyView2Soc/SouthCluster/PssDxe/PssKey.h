/*++

Copyright (c)  1999 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PssDxe.h

Abstract:

--*/

#ifndef _PSS_KEY_H_
#define _PSS_KEY_H_

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellCEntryLib.h>
#include <Library/DriverLib.h>
#include <Library/BaseLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiApplicationEntryPoint.h>
#include <Library/DebugLib.h>
#include <Library/BaseCryptLib.h>
#include <Protocol/I2cBus.h>

#define PSS_I2C_CONTROLLER_ID         0

#define PSS_I2C_SLAVE_ADDR            0x6E

#define I2C_ACPI_ID_LEN               16   //I2C0X\\SFFFF\\400K
#define I2C_ADDRESS_MAX               0x3FE
#define I2C_TIMEOUT_DEFAULT           1000
#define I2C_READ_MAX                  (1024*1024*10+1)   //Defined by the tool itself for maximum read length.

#define DID_ACPI_ID_PREFIX            "I2C0"
#define DID_ACPI_ID_SUFFIX            "\\SFFFF"
#define DID_ACPI_ID_SUFFIX_400K       "\\400K"


#define L_ENDIAN                      0
#define B_ENDIAN                      1

#define MONZAX_SIZE_BYTES_RESERVED    22
#define MONZAX_SIZE_BYTES_EPC         18
#define MONZAX_SIZE_BYTES_TID         24
#define MONZAX_SIZE_BYTES_USER_2K     272
#define MONZAX_SIZE_BYTES_USER_8K     1024

#define BASE_ADDRESS_RESERVED         0x00
#define BASE_ADDRESS_EPC              0x16
#define BASE_ADDRESS_USER_2K          0x28
#define BASE_ADDRESS_USER_8K          0x40
#define BASE_ADDRESS_TID_2K           0x138
#define BASE_ADDRESS_TID_8K           0x28
#define BASE_ADDRESS_CLASSID_2K       0x148
#define BASE_ADDRESS_CLASSID_8K       0x28
#define BASE_ADDRESS_MAX_2K           0x14F
#define BASE_ADDRESS_MAX_8K           0x43F

#define UNLOCKER_KEY_LENGTH           16

typedef enum {
  Reserved,
  Epc,
  Tid,
  User
} memory_bank_t;

typedef enum {
  MonzaX_2K_Dura = 0x140,
  MonzaX_8K_Dura = 0x150
} chip_model_t;

typedef struct _PSS_DRIVER_CONTEXT {
  EFI_I2C_BUS_PROTOCOL      *I2cBusProtocol;
  BOOLEAN                   PssVerified;
  BOOLEAN                   PssEnabled;
  UINT8                     PssSerialTID;
  BOOLEAN                   Is2KPart;
} PSS_DRIVER_CONTEXT;


#endif  //_PSS_KEY_H_
