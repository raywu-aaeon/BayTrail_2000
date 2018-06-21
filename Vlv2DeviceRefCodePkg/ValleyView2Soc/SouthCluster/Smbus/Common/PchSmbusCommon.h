/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2004 - 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

  @file
  PchSmbusCommon.h

  @brief
  PCH Smbus Protocol

**/
#ifndef _PCH_SMBUS_COMMON_H
#define _PCH_SMBUS_COMMON_H

#ifdef ECP_FLAG
#include "PchSmbus.h"
#include "EfiSmbus.h"
#else
#include <IndustryStandard/SmBus.h>
#endif

///
/// Definitions
///
#define STALL_PERIOD                10 * STALL_ONE_MICRO_SECOND     /// 10 microseconds
#define STALL_TIME                  STALL_ONE_SECOND                /// 1 second
#define BUS_TRIES                   3       /// How many times to retry on Bus Errors
#define SMBUS_NUM_RESERVED          38      /// Number of device addresses that are reserved by the SMBus spec.
#define SMBUS_ADDRESS_ARP           0xC2 >> 1
#define SMBUS_DATA_PREPARE_TO_ARP   0x01
#define SMBUS_DATA_RESET_DEVICE     0x02
#define SMBUS_DATA_GET_UDID_GENERAL 0x03
#define SMBUS_DATA_ASSIGN_ADDRESS   0x04
#define SMBUS_GET_UDID_LENGTH       17      /// 16 byte UDID + 1 byte address
///
/// Private data and functions
///
typedef struct _SMBUS_INSTANCE  SMBUS_INSTANCE;
#ifndef ECP_FLAG

typedef
UINT8
(EFIAPI *SMBUS_IO_READ) (
  IN      UINT8                     Offset
  );

typedef
VOID
(EFIAPI *SMBUS_IO_WRITE) (
  IN      UINT8                     Offset,
  IN      UINT8                     Data
  );

typedef
BOOLEAN
(EFIAPI *SMBUS_IO_DONE) (
  IN      UINT8                     *StsReg
  );
#endif

#define PCH_SMBUS_PRIVATE_DATA_SIGNATURE  SIGNATURE_32 ('p', 's', 'm', 'b')

UINT32
SmbusGetIoBase (
  VOID
  )
/**

  @brief
  Get SMBUS IO Base address

  @param[in] None

  @retval UINT32                  The SMBUS IO Base Address

**/
;

UINT8
SmbusIoRead (
  IN      UINT8           Offset
  )
/**

  @brief
  This function provides a standard way to read PCH Smbus IO registers.

  @param[in] Offset               Register offset from Smbus base IO address.

  @retval UINT8                   Returns data read from IO.

**/
;

VOID
SmbusIoWrite (
  IN      UINT8           Offset,
  IN      UINT8           Data
  )
/**

  @brief
  This function provides a standard way to write PCH Smbus IO registers.

  @param[in] Offset               Register offset from Smbus base IO address.
  @param[in] Data                 Data to write to register.

  @retval None.

**/
;

BOOLEAN
IoDone (
  IN      UINT8           *StsReg
  )
/**

  @brief
  This function provides a standard way to check if an SMBus transaction has
  completed.

  @param[in] StsReg               Not used for input. On return, contains the
                                  value of the SMBus status register.

  @retval TRUE                    Transaction is complete
  @retval FALSE                   Otherwise.

**/
;

EFI_STATUS
AcquireBus (
  VOID
  )
/**

  @brief
  Check if it's ok to use the bus.

  @param[in] None

  @retval EFI_SUCCESS             SmBus is acquired and it's safe to send commands.
  @retval EFI_TIMEOUT             SmBus is busy, it's not safe to send commands.

**/
;

EFI_STATUS
SmbusExec (
  IN      EFI_SMBUS_DEVICE_ADDRESS  SlaveAddress,
  IN      EFI_SMBUS_DEVICE_COMMAND  Command,
  IN      EFI_SMBUS_OPERATION       Operation,
  IN      BOOLEAN                   PecCheck,
  IN OUT  UINTN                     *Length,
  IN OUT  VOID                      *Buffer
  )
/**

  @brief
  This function provides a standard way to execute Smbus protocols
  as defined in the SMBus Specification. The data can either be of
  the Length byte, word, or a block of data. The resulting transaction will be
  either the SMBus Slave Device accepts this transaction or this function
  returns with an error

  @param[in] SlaveAddress         Smbus Slave device the command is directed at
  @param[in] Command              Slave Device dependent
  @param[in] Operation            Which SMBus protocol will be used
  @param[in] PecCheck             Defines if Packet Error Code Checking is to be used
  @param[in, out] Length          How many bytes to read. Must be 0 <= Length <= 32 depending on Operation
                                  It will contain the actual number of bytes read/written.
  @param[in, out] Buffer          Contain the data read/written.

  @retval EFI_SUCCESS             The operation completed successfully.
  @exception EFI_UNSUPPORTED      The operation is unsupported.

  @retval EFI_INVALID_PARAMETER   Length or Buffer is NULL for any operation besides
                                  quick read or quick write.
  @retval EFI_TIMEOUT             The transaction did not complete within an internally
                                  specified timeout period, or the controller is not
                                  available for use.
  @retval EFI_DEVICE_ERROR        There was an Smbus error (NACK) during the operation.
                                  This could indicate the slave device is not present
                                  or is in a hung condition.

**/
;

VOID
InitializeSmbusRegisters (
  VOID
  )
/**

  @brief
  This function initializes the Smbus Registers.

  @param[in] None.

  @retval None.

**/
;
#endif
