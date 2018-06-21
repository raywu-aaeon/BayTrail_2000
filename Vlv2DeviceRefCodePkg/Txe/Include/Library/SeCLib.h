/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2006 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  SeCLib.h

Abstract:

  Header file for SeC functionality

--*/
#ifndef _SEC_LIB_H_
#define _SEC_LIB_H_

#include "SeCPolicyLib.h"
#include "HeciMsgLib.h"

EFI_STATUS
SeCLibInit (
  VOID
  )
/*++

Routine Description:

  Check if SeC is enabled

Arguments:

  None.

Returns:

  None.

--*/
;

EFI_STATUS
HeciGetSeCFwInfo (
  IN OUT SEC_CAP *SECCapability
  )
/*++

  Routine Description:
    Host client gets Firmware update info from SEC client

  Arguments:
    SECCapability - Structure of FirmwareUpdateInfo

  Returns:
    EFI_STATUS

--*/
;

EFI_STATUS
HeciGetFwCapsSku (
  SECFWCAPS_SKU       *FwCapsSku
  )
/*++

Routine Description:

  Send Get Firmware SKU Request to SEC

Arguments:

  FwCapsSku - Return Data from Get Firmware Capabilities MKHI Request

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
HeciGetPlatformType (
  OUT PLATFORM_TYPE_RULE_DATA   *RuleData
  )
/*++

Routine Description:

  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to get Ibex Peak platform type.
  One of usages is to utilize this command to determine if the platform runs in
  4M or 8M size firmware.

Arguments:

  RouleData -
    PlatformBrand,
    IntelSeCFwImageType,
    SuperSku,
    PlatformTargetMarketType,
    PlatformTargetUsageType

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
HeciGetFwVersion (
  IN OUT GEN_GET_FW_VER_ACK_DATA      *MsgGenGetFwVersionAckData
  )
/*++

Routine Description:

  Send Get Firmware Version Request to SEC

Arguments:

  MsgGenGetFwVersionAckData - Return themessage of FW version

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
SeCEmptyEvent (
  EFI_EVENT           Event,
  void                *ParentImageHandle
  )
/*++

  Routine Description:
    Dummy return for SeC signal event use

  Arguments:
  Event             - The event that triggered this notification function
  ParentImageHandle - Pointer to the notification functions context

  Returns:
    EFI_SUCCESS     - Always return EFI_SUCCESS

--*/
;

EFI_STATUS
GetAtStateInfo (
  IN OUT UINT8                  *AtState,
  IN OUT UINT8                  *AtLastTheftTrigger,
  IN OUT UINT16                 *AtLockState,
  IN OUT UINT16                 *AtAmPref
  )
/*++

  Routine Description:
    Get AT State Information From FW

  Arguments:
  AtState                    - Pointer to AT State Information
  AtLastTheftTrigger         - Pointer to Variable holding the cause of last AT Stolen Stae
  AtLockState                - Pointer to variable indicating whether AT is locked or not
  AtAmPref                   - Pointer to variable indicating whether TDTAM or PBA should be used

  Returns:
    EFI_SUCCESS     - Always return EFI_SUCCESS

--*/
;
#endif
