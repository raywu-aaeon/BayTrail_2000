/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2004 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  TDT.h

Abstract:

  Defines and prototypes for the TDT driver.
  This driver implements the TDT protocol for Theft Deterrence Technology.

--*/
#ifndef _DXE_TDT_PROTOCOL_H_
#define _DXE_TDT_PROTOCOL_H_

//
// Define the  protocol GUID
//
#define EFI_TDT_PROTOCOL_GUID \
  { \
    0xbf70067, 0xd53b, 0x42df, 0xb7, 0x70, 0xe9, 0x2c, 0x91, 0xc6, 0x14, 0x11 \
  }

//
// Extern the GUID for protocol users.
//
extern EFI_GUID gEfiTdtProtocolGuid;

//
// Forward reference for ANSI C compatibility
//
typedef struct _EFI_TDT_PROTOCOL EFI_TDT_PROTOCOL;

//
// Protocol definitions
//
typedef
EFI_STATUS
(EFIAPI *EFI_TDT_AUTHETICATE_CREDENTIAL) (
  IN     EFI_TDT_PROTOCOL   *This,
  IN     UINT8              *PassPhrase,
  IN     UINT32             *PassType,
  IN OUT UINT8              *IsAuthenticated
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TDT_SEC_RULE_STATE) (
  IN     EFI_TDT_PROTOCOL   *This,
  IN OUT UINT8              *TdtState,
  IN OUT UINT8              *TdtLastTheftTrigger,
  IN OUT UINT16             *TdtLockState,
  IN OUT UINT16             *TdtAmPref
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TDT_COMPUTE_HASH) (
  IN     EFI_TDT_PROTOCOL   *This,
  IN     UINT8              *PassPhrase,
  IN OUT UINT8              *Hash
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TDT_GET_NONCE) (
  IN     EFI_TDT_PROTOCOL   *This,
  IN OUT UINT8              *Nonce
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TDT_GET_TIMER_INFO) (
  IN     EFI_TDT_PROTOCOL   *This,
  IN OUT UINT32             *Interval,
  IN OUT UINT32             *TimeLeft
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TDT_GET_RECOVERY_STRING) (
  IN     EFI_TDT_PROTOCOL   *This,
  IN     UINT32             *StringId,
  IN OUT UINT8              *IsvString,
  IN OUT UINT32             *IsvStringLength
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TDT_GET_ISV_ID) (
  IN     EFI_TDT_PROTOCOL             *This,
  IN OUT UINT32                       *IsvId
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TDT_SET_SUSPEND_STATE) (
  IN     EFI_TDT_PROTOCOL             *This,
  IN     UINT32                       TransitionState,
  IN     UINT8                        *Token
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TDT_INIT_WWAN_RECOV) (
  IN     EFI_TDT_PROTOCOL             *This
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TDT_GET_WWAN_NIC_STATUS) (
  IN     EFI_TDT_PROTOCOL             *This,
  IN OUT UINT8                        *RadioStatus,
  IN OUT UINT8                        *NetworkStatus
  );

typedef
EFI_STATUS
(EFIAPI *EFI_TDT_SEND_ASSERT_STOLEN) (
  IN     EFI_TDT_PROTOCOL   *This,
  IN OUT UINT8              *CompletionCode
  );

//
// Protocol definition
//
typedef struct _EFI_TDT_PROTOCOL {

  EFI_TDT_AUTHETICATE_CREDENTIAL  AuthenticateCredential;
  EFI_TDT_SEC_RULE_STATE           GetTdtSeCRule;
  EFI_TDT_COMPUTE_HASH            ComputeHash;
  EFI_TDT_GET_NONCE               GetNonce;
  EFI_TDT_GET_TIMER_INFO          GetTimerInfo;
  EFI_TDT_GET_RECOVERY_STRING     GetRecoveryString;
  EFI_TDT_GET_ISV_ID              GetIsvId;
  EFI_TDT_SEND_ASSERT_STOLEN      SendAssertStolen;
  EFI_TDT_SET_SUSPEND_STATE       SetSuspendState;
  EFI_TDT_INIT_WWAN_RECOV         InitWWAN;
  EFI_TDT_GET_WWAN_NIC_STATUS     GetWWANNicStatus;
} EFI_TDT_PROTOCOL;

#endif
