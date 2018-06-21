/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2006 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  SeCLib.c

Abstract:

  Implementation file for SeC functionality

--*/

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include "SeCLib.h"

EFI_GUID gDxePlatformSeCPolicyGuid;
#else
#include <MkhiMsgs.h>
#include <Library/DebugLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/UefiBootServicesTableLib.h>
#endif

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
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;

  return Status;
}

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
{
  EFI_STATUS              Status;
  GEN_GET_FW_CAPSKU       MsgGenGetFwCapsSku;
  GEN_GET_FW_CAPS_SKU_ACK MsgGenGetFwCapsSkuAck;

  Status = HeciGetFwCapsSkuMsg (&MsgGenGetFwCapsSku, &MsgGenGetFwCapsSkuAck);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (((MsgGenGetFwCapsSkuAck.MKHIHeader.Fields.Command) == FWCAPS_GET_RULE_CMD) &&
      ((MsgGenGetFwCapsSkuAck.MKHIHeader.Fields.IsResponse) == 1) &&
      (MsgGenGetFwCapsSkuAck.MKHIHeader.Fields.Result == 0)
     ) {
    *FwCapsSku = MsgGenGetFwCapsSkuAck.Data.FWCapSku;
  }

  return EFI_SUCCESS;
}

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

  RuleData -
    PlatformBrand,
    IntelSeCFwImageType,
    SuperSku,
    PlatformTargetMarketType,
    PlatformTargetUsageType

Returns:

  EFI_STATUS

--*/
{
  EFI_STATUS  Status;

  Status = HeciGetPlatformTypeMsg (RuleData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return EFI_SUCCESS;
}

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
{
  EFI_STATUS          Status;
  GEN_GET_FW_VER_ACK  MsgGenGetFwVersionAck;
  //  EFI_DEADLOOP(); need to do debug here
  Status = HeciGetFwVersionMsg (&MsgGenGetFwVersionAck);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((MsgGenGetFwVersionAck.MKHIHeader.Fields.Command == GEN_GET_FW_VERSION_CMD) &&
      (MsgGenGetFwVersionAck.MKHIHeader.Fields.IsResponse == 1) &&
      (MsgGenGetFwVersionAck.MKHIHeader.Fields.Result == 0)
     ) {
    *MsgGenGetFwVersionAckData = MsgGenGetFwVersionAck.Data;
  }

  return EFI_SUCCESS;
}

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
{
  EFI_STATUS              Status;
  DXE_SEC_POLICY_PROTOCOL  *mDxePlatformSeCPolicy;
  SECFWCAPS_SKU            FwCapsSku;

  //
  // Get the SEC platform policy.
  //
  Status = gBS->LocateProtocol (&gDxePlatformSeCPolicyGuid, NULL, (VOID **) &mDxePlatformSeCPolicy);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "No SEC Platform Policy Protocol available"));
    return EFI_UNSUPPORTED;
  }

  SECCapability->SeCEnabled = 1;

  FwCapsSku.Data          = mDxePlatformSeCPolicy->SeCConfig.FwCapsSku;
  if (FwCapsSku.Fields.IntelAT) {
    SECCapability->AtSupported = 1;
  }

  if (FwCapsSku.Fields.KVM) {
//    SECCapability->IntelKVM = 1;
  }

  if (mDxePlatformSeCPolicy->SeCConfig.PlatformBrand == INTEL_AMT_BRAND) {
//    SECCapability->IntelAmtFw        = 1;
//  SECCapability->LocalWakeupTimer  = 1;
  }

  if (mDxePlatformSeCPolicy->SeCConfig.PlatformBrand == INTEL_STAND_MANAGEABILITY_BRAND) {
//    SECCapability->IntelAmtFwStandard = 1;
  }

  SECCapability->SeCMinorVer  = mDxePlatformSeCPolicy->SeCVersion.CodeMinor;
  SECCapability->SeCMajorVer  = mDxePlatformSeCPolicy->SeCVersion.CodeMajor;
  SECCapability->SeCBuildNo   = mDxePlatformSeCPolicy->SeCVersion.CodeBuildNo;
  SECCapability->SeCHotFixNo  = mDxePlatformSeCPolicy->SeCVersion.CodeHotFix;

  return Status;
}

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
{
  return EFI_SUCCESS;
}

EFI_STATUS
GetAtStateInfo (
  IN OUT UINT8                 *AtState,
  IN OUT UINT8                 *AtLastTheftTrigger,
  IN OUT UINT16                *AtLockState,
  IN OUT UINT16                *AtAmPref
  )
/*++

  Routine Description:
    Get AT State Information From Stored SEC platform policy

  Arguments:
  AtState                    - Pointer to AT State Information
  AtLastTheftTrigger         - Pointer to Variable holding the cause of last AT Stolen Stae
  AtLockState                - Pointer to variable indicating whether AT is locked or not
  AtAmPref                   - Pointer to variable indicating whether TDTAM or PBA should be used

  Returns:
    EFI_SUCCESS     - Always return EFI_SUCCESS

--*/
{
  EFI_STATUS              Status;
  DXE_SEC_POLICY_PROTOCOL  *mDxePlatformSeCPolicy;

  //
  // Get the SEC platform policy.
  //
  Status = gBS->LocateProtocol (&gDxePlatformSeCPolicyGuid, NULL, (VOID **) &mDxePlatformSeCPolicy);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "No SEC Platform Policy Protocol available"));
    return EFI_UNSUPPORTED;
  }

  *AtState            = mDxePlatformSeCPolicy->AtConfig.AtState;
  *AtLastTheftTrigger = mDxePlatformSeCPolicy->AtConfig.AtLastTheftTrigger;
  *AtLockState        = mDxePlatformSeCPolicy->AtConfig.AtLockState;
  *AtAmPref           = mDxePlatformSeCPolicy->AtConfig.AtAmPref;

  return EFI_SUCCESS;
}
