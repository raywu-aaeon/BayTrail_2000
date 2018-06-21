/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  PchSmbusLib.c

  @brief
  This file contains routines that support PCH SMBUS FUNCTION

**/
#include "PchSmbusLib.h"

#define mSMBUS_LIB_LENGTH(SmBusAddress)             (((SmBusAddress) >> 16) & 0x3f)

UINTN
EFIAPI
SmBusSeqI2CRead (
  IN  UINTN          SmBusAddress,
  OUT VOID           *Buffer,
  OUT RETURN_STATUS  * Status OPTIONAL
  )
/**

  @brief
  This function provides a standard way to execute Smbus sequential
  I2C Read. This function allows the PCH to perform block reads to
  certain I2C devices, such as serial E2PROMs. Typically these data
  bytes correspond to an offset (address) within the serial memory
  chips.

  @param[in] SmBusAddress         Address that encodes the SMBUS Slave Address,
                                  SMBUS Command, SMBUS Data Length, and PEC.
  @param[in] Buffer               Pointer to the buffer to store the bytes read
                                  from the SMBUS
  @param[in] Status               eturn status for the executed command.

  @retval UINTN                   The number of bytes read

**/
{
  UINTN  Length;

  ASSERT (Buffer != NULL);
  ASSERT (SMBUS_LIB_LENGTH (SmBusAddress) >= 1);
  ASSERT (SMBUS_LIB_RESEARVED (SmBusAddress) == 0);

//  Length = SMBUS_LIB_LENGTH (SmBusAddress);
  Length = mSMBUS_LIB_LENGTH (SmBusAddress);

  return InternalSmBusExec (EfiSmbusReadByte, SmBusAddress, Length, Buffer, Status);
}
