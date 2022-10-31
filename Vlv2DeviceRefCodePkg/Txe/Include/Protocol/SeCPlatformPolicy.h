/*++
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
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

  SeCPlatformPolicy.h

Abstract:

  Interface definition details between SEC and platform drivers during DXE phase.

--*/
#ifndef _SEC_PLATFORM_POLICY_H_
#define _SEC_PLATFORM_POLICY_H_

//
// SEC policy provided by platform for DXE phase
//
#define DXE_PLATFORM_SEC_POLICY_GUID \
  { \
    0xf8bff014, 0x18fb, 0x4ef9, 0xb1, 0xc, 0xae, 0x22, 0x73, 0x8d, 0xbe, 0xed \
  }

//
// Revision 2: Support TR_CONFIG for Thermal Reporting Configuration to SEC
//
//
// Revision 3: Support SeCReportError function
//
//
// Revision 4: Support for Reserved2 and Reserved3 fields
//
//
// Revision 5: Cleanup unused fields
//
#define DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION    1
#define DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_2  2
#define DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_3  3
#define DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_4  4
#define DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_5  5
#define DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_6  6
#define DXE_PLATFORM_SEC_POLICY_PROTOCOL_REVISION_7  7 // Added SeCFwDowngrade & LocalFwUpd to SEC_CONFIG
extern EFI_GUID gDxePlatformSeCPolicyGuid;
#pragma pack(1)
//
// Length of Block read SMBus message from EC.
// Possible values 1, 2, 5, 9, 10, 14 or 20. This is how many bytes EC wants to receive from MPC SEC FW
//
#define TR_CONFIG_EC_MSG_LEN_1  1
#define TR_CONFIG_EC_MSG_LEN_2  2
#define TR_CONFIG_EC_MSG_LEN_5  5
#define TR_CONFIG_EC_MSG_LEN_9  9
#define TR_CONFIG_EC_MSG_LEN_10 10
#define TR_CONFIG_EC_MSG_LEN_14 14
#define TR_CONFIG_EC_MSG_LEN_20 20

#define TR_CONFIG_PEC_DISABLED  0
#define TR_CONFIG_PEC_ENABLED   1

typedef struct {
  UINT8 TrEnabled;
  UINT8 SMBusECMsgLen;
  UINT8 SMBusECMsgPEC;
  UINT8 DimmNumber;
  UINT8 *SmbusAddress;
} TR_CONFIG;

typedef struct {
  UINT8   AtState;
  UINT8   AtLastTheftTrigger;
  UINT16  AtLockState;
  UINT16  AtAmPref;
} AT_CONFIG;

typedef struct {
  UINT16  CodeMinor;
  UINT16  CodeMajor;
  UINT16  CodeBuildNo;
  UINT16  CodeHotFix;
} SEC_VERSION;

typedef struct {
  //
  // Byte 0, bit definition for functionality enable/disable
  //
  UINT8     SeCFwDownGrade : 1;        // 0: Disabled, 1: Enabled
  UINT8     SeCLocalFwUpdEnabled : 1;  // 0: Disabled, 1: Enabled

  UINT8     Reserved : 1;
  UINT8     Reserved1 : 1;
  UINT8     EndOfPostEnabled : 1; // 0: Disabled; 1: Enabled
  UINT8     Reserved2 : 3;
  //
  // Byte 1-3 Reserved for other bit definitions in future
  //
  UINT8     ByteReserved1[3];

  UINT8     HeciCommunication;
  UINT8     PlatformBrand;
  UINT8     SeCFwImageType;
  //
  // Byte 7-15
  //
  UINT32    FwCapsSku;
  UINT8     ByteReserved[5];

  //
  // Thermal Reporting Configuration to SEC
  //
  TR_CONFIG *TrConfig;
} SEC_CONFIG;

typedef struct {
  UINT32    MaxCommandSize;
  UINT32    MaxResponseSize;
} PTT_CONFIG;

typedef enum {
  MSG_EOP_ERROR             = 0,
  MSG_SEC_FW_UPDATE_FAILED,
  MSG_ASF_BOOT_DISK_MISSING,
  MSG_KVM_TIMES_UP,
  MSG_KVM_REJECTED,
  MSG_HMRFPO_LOCK_FAILURE,
  MSG_HMRFPO_UNLOCK_FAILURE,
  MSG_SEC_FW_UPDATE_WAIT,
  MSG_ILLEGAL_CPU_PLUGGED_IN,
  MSG_KVM_WAIT,
  MAX_ERROR_ENUM
} SEC_ERROR_MSG_ID;

typedef
VOID
(EFIAPI *SEC_REPORT_ERROR) (
  SEC_ERROR_MSG_ID
  );

typedef
VOID
(EFIAPI *SEC_PLATFORM_HOOK) (
  VOID
  );

#pragma pack()
//
// SEC DXE Platform Policiy
//
typedef struct _DXE_SEC_POLICY_PROTOCOL {
  UINT8             Revision;
  SEC_CONFIG        SeCConfig;
  SEC_VERSION       SeCVersion;
  AT_CONFIG         AtConfig;
  PTT_CONFIG        PttConfig;
  SEC_REPORT_ERROR  SeCReportError;
  SEC_PLATFORM_HOOK SeCPlatformHook;
} DXE_SEC_POLICY_PROTOCOL;

#endif
