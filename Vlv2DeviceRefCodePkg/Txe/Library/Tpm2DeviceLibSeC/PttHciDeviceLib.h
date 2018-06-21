/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement

@copyright
  Copyright (c) 2012 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

@file
  PttHciDeviceLib.h

@brief
  Platform Trust Technology (FTPM) HCI Device Library

**/
#ifndef _PTT_HCI_DEVICE_LIB_H_
#define _PTT_HCI_DEVICE_LIB_H_

///
/// Default Timeout values
/// @todo these values should be adjusted on actual silicon after power-on
///
#define PTT_HCI_TIMEOUT_A         500          ///< 500 microseconds
#define PTT_HCI_TIMEOUT_B         10 * 1000    ///< 10ms (max command processing time in PK-TPM ca. 3ms)
#define PTT_HCI_TIMEOUT_C         1000 * 1000  ///< 1s
#define PTT_HCI_POLLING_PERIOD    140          ///< Poll register every 140 microsecondss

/// TPM2_RESPONSE_HEADER size (10B) aligned to dword
#define PTT_HCI_RESPONSE_HEADER_SIZE  12 ///< 12B

/**

  Checks whether FTPM is enabled (FTPM_STS::FTPM_EN).

  @retval    TRUE    FTPM is enabled.
  @retval    FALSE   FTPM is disabled. All LT writes will be dropped.
                     All LT reads will be returned with read data value of all 0��s.
                     The bit can only be written once per ME power cycle.

**/
BOOLEAN
EFIAPI
PttHciPresenceCheck();

/**

  Checks whether TPM2_Startup command has been executed (FTPM_STS::STARTUP_EXEC).
  If command was executed, it should not be redundantly issued again.

  @retval    TRUE    Startup command executed already.
  @retval    FALSE   Startup command not executed yet.

**/
BOOLEAN
EFIAPI
PttHciStartupExecuted();

/**

  Sets FTPM_CMD and CA_START register to a defined value to indicate that a command is
  available for processing.
  Any host write to this register shall result in an interrupt to the ME firmware.

  @retval EFI_SUCCESS   Register successfully written.
  @retval TBD

**/
EFI_STATUS
EFIAPI
PttHciRequestCommandExec();

/**

  Checks whether the value of a FTPM register satisfies the input BIT setting.

  @param[in]  Register     Address port of register to be checked.
  @param[in]  BitSet       Check these data bits are set.
  @param[in]  BitClear     Check these data bits are clear.
  @param[in]  TimeOut      The max wait time (unit MicroSecond) when checking register.

  @retval     EFI_SUCCESS  The register satisfies the check bit.
  @retval     EFI_TIMEOUT  The register can't run into the expected status in time.

**/
EFI_STATUS
EFIAPI
PttHciWaitRegisterBits(
  IN      EFI_PHYSICAL_ADDRESS    RegAddress,
  IN      UINT32    BitSet,
  IN      UINT32    BitClear,
  IN      UINT32    TimeOut
  );

/**

  Sends command to FTPM for execution.

  @param[in] FtpmBuffer   Buffer for TPM command data.
  @param[in] DataLength   TPM command data length.

  @retval EFI_SUCCESS     Operation completed successfully.
  @retval EFI_TIMEOUT     The register can't run into the expected status in time.

**/
EFI_STATUS
EFIAPI
PttHciSend(
  IN     UINT8      *FtpmBuffer,
  IN     UINT32     DataLength
  );

/**

  Receives response data of last command from FTPM.

  @param[out] FtpmBuffer        Buffer for response data.
  @param[out] RespSize          Response data length.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_TIMEOUT           The register can't run into the expected status in time.
  @retval EFI_DEVICE_ERROR      Unexpected device status.
  @retval EFI_BUFFER_TOO_SMALL  Response data is too long.

**/
EFI_STATUS
EFIAPI
PttHciReceive(
  OUT     UINT8     *FtpmBuffer,
  OUT     UINT32    *RespSize
  );

#endif
