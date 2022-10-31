/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2004 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  TDT.c

Abstract:

  Defines and prototypes for the TDT driver.
  This driver implements the TDT protocol for Theft Deterrence Technology.

--*/
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include "SeCLib.h"
EFI_GUID gEfiTdtProtocolGuid;
EFI_GUID gEfiHeciProtocolGuid;
#endif
#include "Tdt.h"

TDT_INSTANCE          TdtInstance = {
  TDT_PRIVATE_DATA_SIGNATURE,
  NULL,
  {
    (EFI_TDT_AUTHETICATE_CREDENTIAL) AuthenticateCredential,
    (EFI_TDT_SEC_RULE_STATE) GetTdtSeCRule,
    (EFI_TDT_COMPUTE_HASH) ComputeHash,
    (EFI_TDT_GET_NONCE) GetNonce,
    (EFI_TDT_GET_TIMER_INFO) GetTimerInfo,
    (EFI_TDT_GET_RECOVERY_STRING) GetRecoveryString,
    (EFI_TDT_GET_ISV_ID) GetIsvId,
    (EFI_TDT_SEND_ASSERT_STOLEN) SendAssertStolen,
    (EFI_TDT_SET_SUSPEND_STATE) SetSuspendState,
    (EFI_TDT_INIT_WWAN_RECOV) InitWWANREcov,
    (EFI_TDT_GET_WWAN_NIC_STATUS) GetWWANNicStatus

  }
};

EFI_EVENT             mLegacyBootEvent;

#ifdef EFI_DEBUG

VOID
ShowBuffer (
  UINT8  *Message,
  UINT32 Length
  )
/*++

Routine Description:
  This routine displays the debug message in ASCII

Arguments:
  Message  - Message to be displayed
  Length - Length of the message

Returns:
  None

--*/
{
  UINT32  LineBreak;
  UINT32  Index;
  LineBreak = 0;
  Index     = 0;

  while (Length-- > 0) {
    if (LineBreak == 0) {
      DEBUG ((EFI_D_ERROR, "%02x: ", (Index & 0xF0)));
    }

    DEBUG ((EFI_D_ERROR, "%02x ", Message[Index++]));
    LineBreak++;
    if (LineBreak == 16) {
      DEBUG ((EFI_D_ERROR, "\n"));
      LineBreak = 0;
    }

    if (LineBreak == 8) {
      DEBUG ((EFI_D_ERROR, "- "));
    }
  }

  DEBUG ((EFI_D_ERROR, "\n"));
  return ;
}

#endif // End Of EFI_DEBUG

EFI_STATUS
TdtEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
/*++

Routine Description:

  Entry point for the TDT Driver.

Arguments:

  ImageHandle       Image handle of this driver.
  SystemTable       Global system service table.

Returns:

  EFI_SUCCESS           Initialization complete.
  EFI_UNSUPPORTED       The chipset is unsupported by this driver.
  EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.
  EFI_DEVICE_ERROR      Device error, driver exits abnormally.

--*/
{

  EFI_STATUS  Status;

  //
  // Install the EFI_TDT_PROTOCOL interface
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &(TdtInstance.Handle),
                  &gEfiTdtProtocolGuid,
                  &(TdtInstance.TdtProtocol),
                  NULL
                  );

  return Status;
}

EFI_STATUS
EFIAPI
GetTdtSeCRule (
  IN     EFI_TDT_PROTOCOL      *This,
  IN OUT UINT8                 *TdtState,
  IN OUT UINT8                 *TdtLastTheftTrigger,
  IN OUT UINT16                *TdtLockState,
  IN OUT UINT16                *TdtAmPref
  )
/*++

Routine Description:

  This function sends a request to SEC Kernel and to find out if TDT is supported and also what is the rule
  data. The rule data defines the state of the TDT Platform. This call also replaces the GetTDTState call for
  BIOS AM module.

Arguments:

  This                - The TDT instance of TDT protocol
  TdtState             - Pointer to AT State Information
  TdtLastTheftTrigger  - Pointer to Variable holding the cause of last AT Stolen Stae
  TdtLockState         - Pointer to variable indicating whether AT is locked or not
  TdtAmPref            - Pointer to variable indicating whether TDTAM or PBA should be used

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
{

  GET_TDT_SEC_RULE_CMD SeCTdtRuleCmd;
  GET_TDT_SEC_RULE_RSP SeCTdtRuleRsp;
  UINT32              HeciLength;
  EFI_STATUS          Status;
  EFI_HECI_PROTOCOL   *Heci;

  SetMem ((VOID *) &SeCTdtRuleCmd, sizeof (GET_TDT_SEC_RULE_CMD), 0);
  SetMem ((VOID *) &SeCTdtRuleRsp, sizeof (GET_TDT_SEC_RULE_RSP), 0);

  //
  // For GetTDTState
  //
  SeCTdtRuleCmd.MKHIHeader.Fields.Command    = TDT_SEC_RULE_COMMAND; //0x02;
  SeCTdtRuleCmd.MKHIHeader.Fields.IsResponse = TDT_COMMAND;         //0;
  SeCTdtRuleCmd.MKHIHeader.Fields.GroupId    = TDT_SEC_RULE_GROUP;   //0x03;
  SeCTdtRuleCmd.MKHIHeader.Fields.Reserved   = 0;
  SeCTdtRuleCmd.MKHIHeader.Fields.Result     = 0;
  SeCTdtRuleCmd.RuleId                       = TDT_SEC_RULE_ID;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GetTdtSeCRule: Locating for HECI Driver Failed!, Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (GET_TDT_SEC_RULE_CMD);

  DEBUG ((EFI_D_ERROR, "TDT::SeCTdtRuleCmd: HeciLength = %x\n", HeciLength));
  PERF_START (NULL, "TdTRuleCmd", NULL, 0);
  Status = Heci->SendMsg (
                  (UINT32 *) &SeCTdtRuleCmd,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_CORE_MESSAGE_ADDR
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GetTdtSeCRule failed to send message over HECI!(SendMsg), Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (GET_TDT_SEC_RULE_RSP);
  DEBUG ((EFI_D_ERROR, "SeCTdtRule Rsp length = %x\n", HeciLength));
  Status = Heci->ReadMsg (
                  BLOCKING,
                  (UINT32 *) &SeCTdtRuleRsp,
                  &HeciLength
                  );
  PERF_END (NULL, "TdTRuleCmd", NULL, 0);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GetTdtSeCRule failed to receive message over HECI!(ReadMsg), Status = %r\n", Status));
    return Status;
  }

  DEBUG ((EFI_D_ERROR, "SeCTdtRuleRsp.MKHIHeader.Fields.IsResponse = %x\n", SeCTdtRuleRsp.MKHIHeader.Fields.IsResponse));
  DEBUG ((EFI_D_ERROR, "SeCTdtRuleRsp.RuleId = %x\n", SeCTdtRuleRsp.RuleId));
  DEBUG ((EFI_D_ERROR, "SeCTdtRuleRsp.RuleData = %x\n", SeCTdtRuleRsp.TdtRuleData));
  DEBUG ((EFI_D_ERROR, "SeCTdtRuleRsp.RuleDataLength = %x\n", SeCTdtRuleRsp.RuleDataLength));

  DEBUG ((EFI_D_ERROR, "SeCTdtRule Rsp length = %x\n", HeciLength));

  if (HeciLength == sizeof (GET_TDT_SEC_RULE_RSP)) {
    *TdtState             = SeCTdtRuleRsp.TdtRuleData.State;
    *TdtLastTheftTrigger  = SeCTdtRuleRsp.TdtRuleData.LastTheftTrigger;
    *TdtLockState         = SeCTdtRuleRsp.TdtRuleData.flags.LockState;
    *TdtAmPref            = SeCTdtRuleRsp.TdtRuleData.flags.AuthenticateModule;
    return EFI_SUCCESS;
  } else {

    return EFI_DEVICE_ERROR;
  }

}

EFI_STATUS
EFIAPI
AuthenticateCredential (
  IN     EFI_TDT_PROTOCOL               *This,
  IN     UINT8                          *PassPhrase,
  IN     UINT32                         *PassType,
  IN OUT UINT8                          *IsAuthenticated
  )
/*++

Routine Description:

  This function sends a request to SEC TDT Services to validate TDT recovery credentials. The user input is
  captured in UTF-16 format and then passed to this funtion. This function converts the User recovery password into
  a HASH by using Salt & Nonce and then send the password HASH to SEC TDT Services for validation. SEC TDT Service compares the
  Password HASH and returns either pass or fail.

Arguments:
  This        - The address of protocol
  PassPhrase  - Passphrase that needs to be authenticated sent to SEC
  PassType    - Password type user or server generated
  IsAuthenticated  - The return of the password match 1 for success and 0 for fail

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
{
  UINT32                            HeciLength;
  EFI_STATUS                        Status;
  EFI_HECI_PROTOCOL                 *Heci;
  TDTHI_AUTHENTICATE_CREDENTIAL_CMD *TdtAuthCmd;
  TDTHI_AUTHENTICATE_CREDENTIAL_RSP TdtAuthRsp;

  TdtAuthCmd = AllocateZeroPool (sizeof (TDTHI_AUTHENTICATE_CREDENTIAL_CMD) + TDT_PASSWORD_LENGTH);
  if (TdtAuthCmd == NULL) {
    FreePool (TdtAuthCmd);
    return EFI_OUT_OF_RESOURCES;
  }

  SetMem ((VOID *) &TdtAuthRsp, sizeof (TDTHI_AUTHENTICATE_CREDENTIAL_RSP), 0);

  TdtAuthCmd->Credential.Type              = *PassType;
  TdtAuthCmd->Credential.Length            = (UINT32) TDT_USR_PASS_HASH_LENGTH_MAX;

  TdtAuthCmd->Header.Version.Minor         = TDTHI_PROTOCOL_VERSION_MINOR;           // 1;
  TdtAuthCmd->Header.Version.Major         = TDTHI_PROTOCOL_VERSION_MAJOR;           // 0;
  TdtAuthCmd->Header.Command.Category      = TDTHI_CMD_GROUP_RECOVERY;               // 0x3
  TdtAuthCmd->Header.Command.IsResponse    = TDT_COMMAND;                            // 0 is for Command
  TdtAuthCmd->Header.Command.Code          = TDTHI_RECOVERY_GRP_AUTH_CREDENTIAL_CMD; // 0x1
  //
  // TDT_CREDENTIAL has extra UINT8 (can't have zero length array like FW) that must be subtracted
  //
  TdtAuthCmd->Header.Length = sizeof (TDT_CREDENTIAL) + TDT_USR_PASS_HASH_LENGTH_MAX - sizeof (UINT8);

  DEBUG ((EFI_D_ERROR, "TDT::AutheticateCrdential Password Type:  %x\n", *PassType));
  HeciLength = sizeof (TDTHI_AUTHENTICATE_CREDENTIAL_CMD) + TDT_USR_PASS_HASH_LENGTH_MAX - sizeof (UINT8);

#ifdef EFI_DEBUG
  DEBUG ((EFI_D_ERROR, "TDT::AutheticateCrdential TdtAuthCmd Body 1: \n"));
  DEBUG_CODE (
    ShowBuffer ((UINT8 *) TdtAuthCmd, HeciLength);
  );
#endif

  CopyMem (&TdtAuthCmd->Credential.Value, PassPhrase, TDT_USR_PASS_HASH_LENGTH_MAX);

#ifdef EFI_DEBUG
  DEBUG ((EFI_D_ERROR, "TDT::AutheticateCrdential TdtAuthCmd Body 2: \n"));
  DEBUG_CODE (
    ShowBuffer ((UINT8 *) TdtAuthCmd, HeciLength);
  );
  DEBUG ((EFI_D_ERROR, "TDT::AutheticateCrdential PassPhrase Body 3: \n"));
  DEBUG_CODE (
    ShowBuffer ((UINT8 *) PassPhrase, TdtAuthCmd->Credential.Length);
  );
#endif

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "AutheticateCrdential: Locating for HECI Driver Failed!, Status = %r\n", Status));
    FreePool (TdtAuthCmd);
    return Status;
  }

#ifdef EFI_DEBUG
  DEBUG ((EFI_D_ERROR, "TDT::AutheticateCrdential PassPhrase Length:  %x\n", TdtAuthCmd->Credential.Length));
  DEBUG ((EFI_D_ERROR, "TDT::AutheticateCrdential TdtAuthCmd HeciLength:  %x\n", HeciLength));
  DEBUG ((EFI_D_ERROR, "TDT::AutheticateCrdential TdtAuthCmd Body 3: \n"));
  DEBUG_CODE (
    ShowBuffer ((UINT8 *) TdtAuthCmd, HeciLength);
  );
#endif

  Status = Heci->SendMsg (
                  (UINT32 *) TdtAuthCmd,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_TDT_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "AutheticateCrdential failed to send message over HECI!(SendMsg), Status = %r\n", Status));
    FreePool (TdtAuthCmd);
    return Status;
  }

  FreePool (TdtAuthCmd);

  HeciLength = sizeof (TDTHI_AUTHENTICATE_CREDENTIAL_RSP);
  Status = Heci->ReadMsg (
                  BLOCKING,
                  (UINT32 *) &TdtAuthRsp,
                  &HeciLength
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "AutheticateCrdential failed to receive message over HECI!(ReadMsg), Status = %r\n", Status));
    return Status;
  }

#ifdef EFI_DEBUG
  DEBUG ((EFI_D_ERROR, "TDT::AutheticateCrdential TdtAuthCmd Body 4:  %x\n", TdtAuthCmd));
  DEBUG_CODE (
    ShowBuffer ((UINT8 *) TdtAuthCmd, HeciLength);
  );
  DEBUG ((EFI_D_ERROR, "TDT::AutheticateCrdential TdtAuthRsp HeciLength:  %x\n", HeciLength));
  DEBUG ((EFI_D_ERROR, "TDT::AutheticateCrdential TdtAuthRsp.CompletionCode:  %x\n", TdtAuthRsp.CompletionCode));
  DEBUG ((EFI_D_ERROR, "TDT::AutheticateCrdential TdtAuthRsp.Header.Command.IsResponse:  %x\n", TdtAuthRsp.Header.Command.IsResponse));
  DEBUG ((EFI_D_ERROR, "checkRecoveryPassword TdtAuthRsp.Authenticated:  %d\n", TdtAuthRsp.Authenticated));
#endif

  //
  // Assuming 0 is for success
  //
  *IsAuthenticated = TdtAuthRsp.Authenticated;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ComputeHash (
  IN     EFI_TDT_PROTOCOL             *This,
  IN     UINT8                        *PassPhrase,
  IN OUT UINT8                        *Hash
  )
/*++


Routine Description:
  This API compute the SHA1 hash of the user enterted password

Arguments:
  This        - The address of protocol
  PassPhrase  - The passphrase for which SHA1 hash to be computed
  Hash        - The return value of the SHA1 hash

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
{

  UINT32                  HeciLength;
  EFI_STATUS              Status;
  EFI_HECI_PROTOCOL       *Heci;

  TDTHI_COMPUTE_HASH_CMD  TdtComputeHashCmd;
  TDTHI_COMPUTE_HASH_RSP  TdtComputeHashRsp;

  SetMem ((VOID *) &TdtComputeHashCmd, sizeof (TDTHI_COMPUTE_HASH_CMD), 0);
  SetMem ((VOID *) &TdtComputeHashRsp, sizeof (TDTHI_COMPUTE_HASH_RSP), 0);

  DEBUG ((EFI_D_ERROR, "TDT::TdtComputeHashRsp DEBUG in ComputHash\n"));

  TdtComputeHashCmd.Header.Version.Minor      = TDTHI_PROTOCOL_VERSION_MINOR; // 1
  TdtComputeHashCmd.Header.Version.Major      = TDTHI_PROTOCOL_VERSION_MAJOR; // 0
  TdtComputeHashCmd.Header.Command.Category   = TDTHI_CMD_GROUP_RECOVERY;
  TdtComputeHashCmd.Header.Command.IsResponse = TDT_COMMAND;                  // 0
  TdtComputeHashCmd.Header.Command.Code       = TDTHI_RECOVERY_GRP_COMPUTE_HASH_CMD;

  TdtComputeHashCmd.Algorithm                 = TDT_HASH_ALGO_ID_SHA1;
  TdtComputeHashCmd.InputLength               = (UINT8) AsciiStrLen ((CHAR8 *) PassPhrase);

  //
  // Check for length
  //
  //
  // 0- Length 0 only header with command is send as message
  //
  TdtComputeHashCmd.Header.Length = sizeof (TDTHI_COMPUTE_HASH_CMD) -
                                    48 -
                                    sizeof (TDTHI_HEADER) +
                                    TdtComputeHashCmd.InputLength;

  //
  // sizeof(PassPhrase)
  //
  CopyMem (&TdtComputeHashCmd.InputBuffer, PassPhrase, TdtComputeHashCmd.InputLength);
  HeciLength = sizeof (TDTHI_COMPUTE_HASH_CMD) - 48 + TdtComputeHashCmd.InputLength;

#ifdef EFI_DEBUG
  DEBUG ((EFI_D_ERROR, "TDT::TdtComputeHashCmd message length = %x\n", HeciLength));
  DEBUG_CODE (
    ShowBuffer ((UINT8 *) &TdtComputeHashCmd, HeciLength);
  );
  DEBUG ((EFI_D_ERROR, "TDT::Look for UINT8Pass\n"));
  DEBUG_CODE (
    ShowBuffer ((UINT8 *) PassPhrase, TdtComputeHashCmd.InputLength);
  );
#endif

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtComputeHash: Locating for HECI Driver Failed!, Status = %r\n", Status));
    return Status;
  }

  Status = Heci->SendMsg (
                  (UINT32 *) &TdtComputeHashCmd,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_TDT_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtComputeHashCmd failed to send message over HECI!(SendMsg), Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (TDTHI_COMPUTE_HASH_RSP);
  Status = Heci->ReadMsg (
                  BLOCKING,
                  (UINT32 *) &TdtComputeHashRsp,
                  &HeciLength
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtComputeHashRsp failed to receive message over HECI!(ReadMsg), Status = %r\n", Status));
    return Status;
  }

  DEBUG ((EFI_D_ERROR, "TDT::TdtComputeHashRsp response message length %x\n", HeciLength));
  DEBUG ((EFI_D_ERROR, "TdtComputeHashRsp.CompletionCode = %x\n", TdtComputeHashRsp.CompletionCode));
  DEBUG ((EFI_D_ERROR, "TdtComputeHashRsp.OutputLength = %x\n", TdtComputeHashRsp.OutputLength));

  if (TdtComputeHashRsp.OutputLength) {
    DEBUG ((EFI_D_ERROR, "TdtComputeHashRsp.OutputLength = %x\n", TdtComputeHashRsp.OutputLength));

  }

  //
  // sizeof(hash)
  //
  CopyMem (Hash, &TdtComputeHashRsp.OutputBuffer, TdtComputeHashRsp.OutputLength);

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
GetTimerInfo (
  IN     EFI_TDT_PROTOCOL               *This,
  IN OUT UINT32                         *Interval,
  IN OUT UINT32                         *TimeLeft
  )
/*++

Routine Description:
  This API get the TDT Unlock Timer values

Arguments:
  This        - The address of protocol
  Interval  -  The return value of the Unlock Time Interval that was set by TDT Server
  TimeLeft - The Timeleft in the Unlock Timer

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
{

  UINT32                    HeciLength;
  EFI_STATUS                Status;
  EFI_HECI_PROTOCOL         *Heci;

  TDTHI_GET_TIMER_INFO_CMD  TdtGetTimerInfoCmd;
  TDTHI_GET_TIMER_INFO_RSP  TdtGetTimerInfoRsp;

  SetMem ((VOID *) &TdtGetTimerInfoCmd, sizeof (TDTHI_GET_TIMER_INFO_CMD), 0);
  SetMem ((VOID *) &TdtGetTimerInfoRsp, sizeof (TDTHI_GET_TIMER_INFO_RSP), 0);

  TdtGetTimerInfoCmd.Header.Version.Minor      = TDTHI_PROTOCOL_VERSION_MINOR;                            // 1
  TdtGetTimerInfoCmd.Header.Version.Major      = TDTHI_PROTOCOL_VERSION_MAJOR;                            // 0
  TdtGetTimerInfoCmd.Header.Command.Category   = TDTHI_CMD_GROUP_THEFT_DETECTION;                         // 1
  TdtGetTimerInfoCmd.Header.Command.IsResponse = TDT_COMMAND;                                             // 0
  TdtGetTimerInfoCmd.Header.Command.Code       = TDTHI_THEFT_DETECT_GRP_GET_TIMER_INFO_CMD;               // 1
  TdtGetTimerInfoCmd.Header.Length             = sizeof(TDTHI_GET_TIMER_INFO_CMD) - sizeof(TDTHI_HEADER); // 0- Length 0 only header with command is send as message

  TdtGetTimerInfoCmd.TimerId = TDT_TID_UNLOCK_TIMER;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtGetTimerInfo: Locating for HECI Driver Failed!, Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (TDTHI_GET_TIMER_INFO_CMD);

  Status = Heci->SendMsg (
                  (UINT32 *) &TdtGetTimerInfoCmd,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_TDT_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtGetTimerInfo failed to send message over HECI!(SendMsg), Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (TDTHI_GET_TIMER_INFO_RSP);

  DEBUG ((EFI_D_ERROR, "TDT::TdtGetTimerInfo response message length  = %x\n", HeciLength));

  Status = Heci->ReadMsg (
                  BLOCKING,
                  (UINT32 *) &TdtGetTimerInfoRsp,
                  &HeciLength
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtGetTimerInfoRsp failed to receive message over HECI!(ReadMsg), Status = %r\n", Status));
    return Status;
  }

#ifdef EFI_DEBUG
  DEBUG ((EFI_D_ERROR, "TDT::TdtGetTimerInfo response message length = %x\n", HeciLength));
  DEBUG ((EFI_D_ERROR, "TdtGetTimerInfoRsp.CompletionCode = %x\n", TdtGetTimerInfoRsp.CompletionCode));
  DEBUG ((EFI_D_ERROR, "TdtGetTimerInfoRsp.TimerInfo.Interval = %x\n", TdtGetTimerInfoRsp.TimerInfo.Interval));
  DEBUG ((EFI_D_ERROR, "TdtGetTimerInfoRsp.TimerInfo.TimeLeft = %x\n", TdtGetTimerInfoRsp.TimerInfo.TimeLeft));
#endif

  *Interval = TdtGetTimerInfoRsp.TimerInfo.Interval;
  *TimeLeft = TdtGetTimerInfoRsp.TimerInfo.TimeLeft;

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
GetNonce (
  IN     EFI_TDT_PROTOCOL             *This,
  IN OUT UINT8                        *Nonce
  )
/*++

Routine Description:
  This gets the ME nonce

Arguments:
  This        - The address of protocol
  Nonce  -  The return value of the 16 Byte nonce received from SEC

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
{

  UINT32              HeciLength;
  EFI_STATUS          Status;
  EFI_HECI_PROTOCOL   *Heci;

  TDTHI_GET_NONCE_CMD TdtGetNonceCmd;
  TDTHI_GET_NONCE_RSP TdtGetNonceRsp;

  SetMem ((VOID *) &TdtGetNonceCmd, sizeof (TDTHI_GET_NONCE_CMD), 0);
  SetMem ((VOID *) &TdtGetNonceRsp, sizeof (TDTHI_GET_NONCE_RSP), 0);

  TdtGetNonceCmd.Version.Minor      = TDTHI_PROTOCOL_VERSION_MINOR;                       // 1
  TdtGetNonceCmd.Version.Major      = TDTHI_PROTOCOL_VERSION_MAJOR;                       // 0
  TdtGetNonceCmd.Command.Category   = TDTHI_CMD_GROUP_GENERAL;
  TdtGetNonceCmd.Command.IsResponse = TDT_COMMAND;                                        // 0
  TdtGetNonceCmd.Command.Code       = TDTHI_GENERAL_GRP_GET_NONCE_CMD;
  TdtGetNonceCmd.Length             = sizeof(TDTHI_GET_STATE_CMD) - sizeof(TDTHI_HEADER); // 0- Length 0 only header with command is send as message

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtGetNonce: Locating for HECI Driver Failed!, Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (TDTHI_GET_NONCE_CMD);

  Status = Heci->SendMsg (
                  (UINT32 *) &TdtGetNonceCmd,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_TDT_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtGetNonce failed to send message over HECI!(SendMsg), Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (TDTHI_GET_NONCE_RSP);
  Status = Heci->ReadMsg (
                  BLOCKING,
                  (UINT32 *) &TdtGetNonceRsp,
                  &HeciLength
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtGetNonce failed to receive message over HECI!(ReadMsg), Status = %r\n", Status));
    return Status;
  }

#ifdef EFI_DEBUG
  DEBUG ((EFI_D_ERROR, "TDT::TdtGetNonce response message length %x\n", HeciLength));
  DEBUG ((EFI_D_ERROR, "TdtGetNonceRsp.CompletionCode = %x\n", TdtGetNonceRsp.CompletionCode));
  DEBUG ((EFI_D_ERROR, "TdtGetNonceRsp.Nonce = %x\n", TdtGetNonceRsp.Nonce));
  DEBUG_CODE (
    ShowBuffer ((UINT8 *) &TdtGetNonceRsp.Nonce, TDT_NONCE_LENGTH);
  );
#endif

  if (!TdtGetNonceRsp.CompletionCode) {
    DEBUG ((EFI_D_ERROR, "TdtGetNonceRsp.CompletionCode = %x\n", TdtGetNonceRsp.CompletionCode));
  }

  CopyMem (Nonce, &TdtGetNonceRsp.Nonce, TDT_NONCE_LENGTH);

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
GetRecoveryString (
  IN     EFI_TDT_PROTOCOL             *This,
  IN     UINT32                       *StringId,
  IN OUT UINT8                        *IsvString,
  IN OUT UINT32                       *IsvStringLength

  )
/*++

Routine Description:
  This retrives the ISV String stored by TDT Server that BIOS will display during Platform lock state

Arguments:
  This            - The address of protocol
  StringId        - The String buffer ID to retrive the ISV String
  IsvString       - 256 Bytes of ISV string array, the
  IsvStringLength - The String length

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
{

  UINT32                      HeciLength;
  EFI_STATUS                  Status;
  EFI_HECI_PROTOCOL           *Heci;

  TDTHI_GET_VENDOR_STRING_CMD TdtIsvStringCmd;
  TDTHI_GET_VENDOR_STRING_RSP TdtIsvStringRsp;

  SetMem ((VOID *) &TdtIsvStringCmd, sizeof (TDTHI_GET_VENDOR_STRING_CMD), 0);
  SetMem ((VOID *) &TdtIsvStringRsp, sizeof (TDTHI_GET_VENDOR_STRING_RSP), 0);
  //
  // Setting the length of IsvString to 0 here.
  //
  *IsvStringLength                          = 0;

  TdtIsvStringCmd.Header.Version.Minor      = TDTHI_PROTOCOL_VERSION_MINOR; // 1
  TdtIsvStringCmd.Header.Version.Major      = TDTHI_PROTOCOL_VERSION_MAJOR; // 0
  TdtIsvStringCmd.Header.Command.Category   = TDTHI_CMD_GROUP_DATA_STORAGE;
  TdtIsvStringCmd.Header.Command.IsResponse = TDT_COMMAND;                  // 0
  TdtIsvStringCmd.Header.Command.Code       = TDTHI_DATA_STORE_GRP_GET_VENDOR_STRING_CMD;
  TdtIsvStringCmd.Header.Length             = sizeof(TDTHI_GET_VENDOR_STRING_CMD) - sizeof(TDTHI_HEADER);
  TdtIsvStringCmd.Id                        = (UINT8) * StringId;

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtIsvStringCmd: Locating for HECI Driver Failed!, Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (TDTHI_GET_VENDOR_STRING_CMD);

  PERF_START (NULL, "RecoveryString", NULL, 0);
  Status = Heci->SendMsg (
                  (UINT32 *) &TdtIsvStringCmd,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_TDT_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtIsvStringCmd failed to send message over HECI!(SendMsg), Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (TDTHI_GET_VENDOR_STRING_RSP);
  Status = Heci->ReadMsg (
                  BLOCKING,
                  (UINT32 *) &TdtIsvStringRsp,
                  &HeciLength
                  );

  PERF_END (NULL, "RecoveryString", NULL, 0);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtIsvStringRsp failed to receive message over HECI!(ReadMsg), Status = %r\n", Status));
    return Status;
  }

#ifdef EFI_DEBUG
  DEBUG ((EFI_D_ERROR, "TDT::TdtIsvStringRsp response message length = %x\n", HeciLength));
  DEBUG ((EFI_D_ERROR, "TdtIsvStringRsp.CompletionCode = %x\n", TdtIsvStringRsp.CompletionCode));
  DEBUG ((EFI_D_ERROR, "TdtIsvStringRsp.String.Length = %x\n", TdtIsvStringRsp.String.Length));
  DEBUG_CODE (
    ShowBuffer ((UINT8 *) &TdtIsvStringRsp.String.Value, (UINT32) TdtIsvStringRsp.String.Length);
  );
#endif

  if ((TdtIsvStringRsp.CompletionCode == 0) &&
      (TdtIsvStringRsp.String.Length > 0 && TdtIsvStringRsp.String.Length <= 256)
     ) {

    CopyMem (IsvString, &TdtIsvStringRsp.String.Value, TdtIsvStringRsp.String.Length);
    *IsvStringLength = TdtIsvStringRsp.String.Length;
    return EFI_SUCCESS;
  }

  return EFIERR (TdtIsvStringRsp.CompletionCode);

}

EFI_STATUS
EFIAPI
SendAssertStolen (
  IN     EFI_TDT_PROTOCOL             *This,
  IN OUT UINT8                        *CompletionCode
  )
/*++

Routine Description:
  This send an AssertStolen Message to SEC when OEM has set the AllowAssertStolen bit to be accepted by BIOS.

Arguments:
  This            - The address of protocol
  CompletionCode  - The return SEC Firmware return code for this request

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
{

  UINT32                  HeciLength;
  EFI_STATUS              Status;
  EFI_HECI_PROTOCOL       *Heci;
  TDTHI_ASSERT_STOLEN_CMD TdtAssertStolenCmd;
  TDTHI_ASSERT_STOLEN_RSP TdtAssertStolenRsp;

  //
  // Initialize Variables
  //
  SetMem ((VOID *) &TdtAssertStolenCmd, sizeof (TDTHI_ASSERT_STOLEN_CMD), 0);
  SetMem ((VOID *) &TdtAssertStolenRsp, sizeof (TDTHI_ASSERT_STOLEN_RSP), 0);

  //
  // Populate TdtAssertStolenCmd
  //
  TdtAssertStolenCmd.Header.Version.Minor      = TDTHI_PROTOCOL_VERSION_MINOR;                           // 2
  TdtAssertStolenCmd.Header.Version.Major      = TDTHI_PROTOCOL_VERSION_MAJOR;                           // 0
  TdtAssertStolenCmd.Header.Command.Category   = TDTHI_CMD_GROUP_THEFT_DETECTION;
  TdtAssertStolenCmd.Header.Command.IsResponse = TDT_COMMAND;                                            // 0
  TdtAssertStolenCmd.Header.Command.Code       = TDTHI_THEFT_DETECT_GRP_UNSIGNED_ASSERT_STOLEN_CMD;
  TdtAssertStolenCmd.Header.Length             = sizeof(TDTHI_ASSERT_STOLEN_CMD) - sizeof(TDTHI_HEADER); // 0- Length

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "SendAssertStolen: Locating for HECI Driver Failed!, Status = %r\n", Status));
    return Status;
  }
  //
  // Send TdtAssertStolenCmd Request
  //
  HeciLength = sizeof (TDTHI_ASSERT_STOLEN_CMD);

  Status = Heci->SendMsg (
                  (UINT32 *) &TdtAssertStolenCmd,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_TDT_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtAssertStolenCmd failed to send message over HECI!(SendMsg), Status = %r\n", Status));
    return Status;
  }
  //
  // Receive TdtAssertStolenCmd Response
  //
  HeciLength = sizeof (TDTHI_ASSERT_STOLEN_RSP);
  Status = Heci->ReadMsg (
                  BLOCKING,
                  (UINT32 *) &TdtAssertStolenRsp,
                  &HeciLength
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TdtAssertStolenRsp failed to receive message over HECI!(ReadMsg), Status = %r\n", Status));
    return Status;
  }

  DEBUG ((EFI_D_ERROR, "TDT::TdtAssertStolen response message length %x\n", HeciLength));
  DEBUG ((EFI_D_ERROR, "TdtAssertStolenRsp.CompletionCode = %x\n", TdtAssertStolenRsp.CompletionCode));

  *CompletionCode = TdtAssertStolenRsp.CompletionCode;

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
GetIsvId (
  IN     EFI_TDT_PROTOCOL             *This,
  IN OUT UINT32                       *IsvId
  )
/*++

Routine Description:
  This receives the ISV ID from SEC and display the ID, when the platform is in stolen state

Arguments:
  This        - The address of protocol
  IsvId       - The pointer to 4 byte ISV ID

Returns:
  EFI_SUCCESS   The function completed successfully.

--*/
{

  UINT32              HeciLength;
  EFI_STATUS          Status;
  EFI_HECI_PROTOCOL   *Heci;

  TDTHI_GET_ISVID_CMD TdtGetIsvIdCmd;
  TDTHI_GET_ISVID_RSP TdtGetIsvIdRsp;

  SetMem ((VOID *) &TdtGetIsvIdCmd, sizeof (TDTHI_GET_ISVID_CMD), 0);
  SetMem ((VOID *) &TdtGetIsvIdRsp, sizeof (TDTHI_GET_ISVID_RSP), 0);

  TdtGetIsvIdCmd.Version.Minor      = TDTHI_PROTOCOL_VERSION_MINOR;
  TdtGetIsvIdCmd.Version.Major      = TDTHI_PROTOCOL_VERSION_MAJOR;
  TdtGetIsvIdCmd.Command.Category   = TDTHI_CMD_GROUP_RECOVERY;
  TdtGetIsvIdCmd.Command.IsResponse = TDT_COMMAND;
  TdtGetIsvIdCmd.Command.Code       = TDTHI_RECOVERY_GRP_GET_ISVID_CMD;
  TdtGetIsvIdCmd.Length             = sizeof (TDTHI_GET_ISVID_CMD) - sizeof (TDTHI_HEADER);

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GetIsvId: Locating for HECI Driver Failed!, Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (TDTHI_GET_ISVID_CMD);

  Status = Heci->SendMsg (
                  (UINT32 *) &TdtGetIsvIdCmd,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_TDT_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GetIsvId failed to send message over HECI!(SendMsg), Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (TDTHI_GET_ISVID_RSP);
  Status = Heci->ReadMsg (
                  BLOCKING,
                  (UINT32 *) &TdtGetIsvIdRsp,
                  &HeciLength
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GetIsvId failed to receive message over HECI!(ReadMsg), Status = %r\n", Status));
    return Status;
  }

  DEBUG ((EFI_D_ERROR, "TDT::TdtGetIsvIdRsp response message length = %x\n", HeciLength));
  DEBUG ((EFI_D_ERROR, "TdtGetIsvIdRsp.CompletionCode = %x\n", TdtGetIsvIdRsp.CompletionCode));
  DEBUG ((EFI_D_ERROR, "TdtGetIsvIdRsp.IsvId = %x\n", TdtGetIsvIdRsp.IsvId));

  if (!TdtGetIsvIdRsp.CompletionCode) {
    *IsvId = TdtGetIsvIdRsp.IsvId;
  }

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
SetSuspendState (
  IN     EFI_TDT_PROTOCOL             *This,
  IN     UINT32                       TransitionState,
  IN     UINT8                        *Token
  )
/*++

Routine Description:
  This requests FW to enter or exit Suspend mode based on user input

Arguments:
  This                - The address of protocol
  TransitionState     - 0: Exit Suspend Mode
                        1: Enter Suspend Mode
  Token               - SRTK generated Token

Returns:
  EFI_SUCCESS   The function completed successfully.

--*/
{
  UINT32                      HeciLength;
  EFI_STATUS                  Status;
  EFI_HECI_PROTOCOL           *Heci;

  TDTHI_SET_SUSPEND_STATE_CMD *TdtSetSuspendStateCmd;
  TDTHI_SET_SUSPEND_STATE_RSP TdtSetSuspendStateRsp;

  TdtSetSuspendStateCmd = AllocateZeroPool (sizeof (TDTHI_SET_SUSPEND_STATE_CMD) + TDT_USR_PASS_HASH_LENGTH_MAX);
  if (TdtSetSuspendStateCmd == NULL) {
    FreePool (TdtSetSuspendStateCmd);
    return EFI_OUT_OF_RESOURCES;
  }

  SetMem ((VOID *) &TdtSetSuspendStateRsp, sizeof (TDTHI_SET_SUSPEND_STATE_RSP), 0);

  TdtSetSuspendStateCmd->Header.Version.Minor      = TDTHI_PROTOCOL_VERSION_MINOR;      // 1
  TdtSetSuspendStateCmd->Header.Version.Major      = TDTHI_PROTOCOL_VERSION_MAJOR;      // 0
  TdtSetSuspendStateCmd->Header.Command.Category   = TDTHI_CMD_GROUP_GENERAL;           // 04
  TdtSetSuspendStateCmd->Header.Command.IsResponse = TDT_COMMAND;                       // 0
  TdtSetSuspendStateCmd->Header.Command.Code       = TDTHI_GENERAL_GRP_SET_SUSPEND_CMD; // 8
  TdtSetSuspendStateCmd->TransitionState           = TransitionState;

  //
  // TDT_CREDENTIAL has extra UINT8 (can't have zero length array like FW) that must be subtracted
  //
  TdtSetSuspendStateCmd->Header.Length = (sizeof (TDTHI_SET_SUSPEND_STATE_CMD) - sizeof (UINT8)) - (sizeof (TDTHI_HEADER)) + TDT_USR_PASS_HASH_LENGTH_MAX;
  TdtSetSuspendStateCmd->Credential.Type    = TDT_CREDENTIAL_TYPE_SSTK;
  TdtSetSuspendStateCmd->Credential.Length  = TDT_USR_PASS_HASH_LENGTH_MAX;
  CopyMem (TdtSetSuspendStateCmd->Credential.Value, Token, TDT_USR_PASS_HASH_LENGTH_MAX);

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "SetSuspendState: Locating for HECI Driver Failed!, Status = %r\n", Status));
    FreePool (TdtSetSuspendStateCmd);
    return Status;
  }

  HeciLength = sizeof (TDTHI_SET_SUSPEND_STATE_CMD) - sizeof (UINT8) + TDT_USR_PASS_HASH_LENGTH_MAX;

  Status = Heci->SendMsg (
                  (UINT32 *) TdtSetSuspendStateCmd,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_TDT_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "SetSuspendState failed to send message over HECI!(SendMsg), Status = %r\n", Status));
    FreePool (TdtSetSuspendStateCmd);
    return Status;
  }

  FreePool (TdtSetSuspendStateCmd);

  HeciLength = sizeof (TDTHI_SET_SUSPEND_STATE_RSP);
  Status = Heci->ReadMsg (
                  BLOCKING,
                  (UINT32 *) &TdtSetSuspendStateRsp,
                  &HeciLength
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "SetSuspendState failed to receive message over HECI!(ReadMsg), Status = %r\n", Status));
    return Status;
  }

  DEBUG ((EFI_D_ERROR, "TDT::SetSuspendState response message length %x\n", HeciLength));
  DEBUG ((EFI_D_ERROR, "TdtSetSuspendStateRsp.CompletionCode = %x\n", TdtSetSuspendStateRsp.CompletionCode));

  if (TdtSetSuspendStateRsp.CompletionCode) {
    return EFIERR (TdtSetSuspendStateRsp.CompletionCode);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
InitWWANREcov (
  IN     EFI_TDT_PROTOCOL             *This
  )
/*++

Routine Description:
  This instructs FW that a WWAN recovery is
  desired and thus the Radio needs to be initialized

Arguments:
  This             - The address of protocol

Returns:
  EFI_SUCCESS       The function completed successfully.

--*/
{

  UINT32                    HeciLength;
  EFI_STATUS                Status;
  EFI_HECI_PROTOCOL         *Heci;

  TDTHI_INIT_WWAN_RECOV_CMD TdtInitWWANRecoveryCmd;
  TDTHI_INIT_WWAN_RECOV_RSP TdtInitWWANRecoveryRsp;

  SetMem ((VOID *) &TdtInitWWANRecoveryCmd, sizeof (TDTHI_INIT_WWAN_RECOV_CMD), 0);
  SetMem ((VOID *) &TdtInitWWANRecoveryRsp, sizeof (TDTHI_INIT_WWAN_RECOV_RSP), 0);

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "InitWWANREcov: Locating for HECI Driver Failed!, Status = %r\n", Status));
    return Status;
  }

  TdtInitWWANRecoveryCmd.Header.Version.Minor      = TDTHI_PROTOCOL_VERSION_MINOR;                               // 1
  TdtInitWWANRecoveryCmd.Header.Version.Major      = TDTHI_PROTOCOL_VERSION_MAJOR;                               // 2
  TdtInitWWANRecoveryCmd.Header.Command.Category   = TDTHI_CMD_GROUP_3G_NIC;                                     // 06
  TdtInitWWANRecoveryCmd.Header.Command.IsResponse = TDT_COMMAND;                                                // 0
  TdtInitWWANRecoveryCmd.Header.Command.Code       = TDTHI_3G_NIC_GRP_INIT_CMD;                                  // 3
  TdtInitWWANRecoveryCmd.Header.Length             = sizeof (TDTHI_INIT_WWAN_RECOV_CMD) - sizeof (TDTHI_HEADER); // 0- Length 0 only header with command is send as message

  HeciLength = sizeof (TDTHI_INIT_WWAN_RECOV_CMD);
  Status = Heci->SendMsg (
                  (UINT32 *) &TdtInitWWANRecoveryCmd,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_TDT_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "InitWWANREcov failed to send message over HECI!(SendMsg), Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (TDTHI_INIT_WWAN_RECOV_RSP);
  Status = Heci->ReadMsg (
                  BLOCKING,
                  (UINT32 *) &TdtInitWWANRecoveryRsp,
                  &HeciLength
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "InitWWANREcov failed to receive message over HECI!(ReadMsg), Status = %r\n", Status));
    return Status;
  }

  DEBUG ((EFI_D_ERROR, "TDT::InitWWANREcov response message length %x\n", HeciLength));
  DEBUG ((EFI_D_ERROR, "TdtInitWWANRecoveryRsp.CompletionCode = %x\n", TdtInitWWANRecoveryRsp.CompletionCode));

  if (TdtInitWWANRecoveryRsp.CompletionCode) {
    return (EFI_STATUS) EFIERR (TdtInitWWANRecoveryRsp.CompletionCode);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
GetWWANNicStatus (
  IN     EFI_TDT_PROTOCOL             *This,
  IN OUT UINT8                        *RadioStatus,
  IN OUT UINT8                        *NetworkStatus
  )
/*++

Routine Description:
  This queries FW of the NIC Radio Status

Arguments:
  This              - The address of protocol
  RadioStatus       - 0: Radio Off
                      1: Radio On
  NetworkStatus     - 0: Detached
                      1: Attached
Returns:
  EFI_SUCCESS         The function completed successfully.

--*/
{
  UINT32                HeciLength;
  EFI_STATUS            Status;
  EFI_HECI_PROTOCOL     *Heci;

  TDTHI_WWAN_STATUS_CMD TdtWWANStatusCmd;
  TDTHI_WWAN_STATUS_RSP TdtWWANStatusRsp;

  SetMem ((VOID *) &TdtWWANStatusCmd, sizeof (TDTHI_WWAN_STATUS_CMD), 0);
  SetMem ((VOID *) &TdtWWANStatusRsp, sizeof (TDTHI_WWAN_STATUS_RSP), 0);

  Status = gBS->LocateProtocol (
                  &gEfiHeciProtocolGuid,
                  NULL,
                  (VOID **) &Heci
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GetWWANNicStatus: Locating for HECI Driver Failed!, Status = %r\n", Status));
    return Status;
  }

  TdtWWANStatusCmd.Header.Version.Minor      = TDTHI_PROTOCOL_VERSION_MINOR;                           // 1
  TdtWWANStatusCmd.Header.Version.Major      = TDTHI_PROTOCOL_VERSION_MAJOR;                           // 0
  TdtWWANStatusCmd.Header.Command.Category   = TDTHI_CMD_GROUP_3G_NIC;                                 // 06
  TdtWWANStatusCmd.Header.Command.IsResponse = TDT_COMMAND;                                            // 0
  TdtWWANStatusCmd.Header.Command.Code       = TDTHI_3G_NIC_GRP_QUERY_CMD;                             // 4
  TdtWWANStatusCmd.Header.Length             = sizeof (TDTHI_WWAN_STATUS_CMD) - sizeof (TDTHI_HEADER); // 0- Length 0 only header with command is send as message

  HeciLength = sizeof (TDTHI_WWAN_STATUS_CMD);
  Status = Heci->SendMsg (
                  (UINT32 *) &TdtWWANStatusCmd,
                  HeciLength,
                  BIOS_FIXED_HOST_ADDR,
                  HECI_TDT_MESSAGE_ADDR
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GetWWANNicStatus failed to send message over HECI!(SendMsg), Status = %r\n", Status));
    return Status;
  }

  HeciLength = sizeof (TDTHI_WWAN_STATUS_RSP);
  Status = Heci->ReadMsg (
                  BLOCKING,
                  (UINT32 *) &TdtWWANStatusRsp,
                  &HeciLength
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "GetWWANNicStatus failed to receive message over HECI!(ReadMsg), Status = %r\n", Status));
    return Status;
  }

  DEBUG ((EFI_D_ERROR, "TDT::GetWWANNicStatus response message length %x\n", HeciLength));
  DEBUG ((EFI_D_ERROR, "TdtWWANStatusRsp.CompletionCode = %x\n", TdtWWANStatusRsp.CompletionCode));

  if (TdtWWANStatusRsp.CompletionCode) {
    return (EFI_STATUS) EFIERR (TdtWWANStatusRsp.CompletionCode);
  }

  *RadioStatus    = TdtWWANStatusRsp.RadioStatus;
  *NetworkStatus  = TdtWWANStatusRsp.NetworkStatus;

  return EFI_SUCCESS;
}
