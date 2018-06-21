/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  MkhiMsgs.h

Abstract:

  MKHI Messages
Revision History

--*/
#ifndef _MKHI_MSGS_H
#define _MKHI_MSGS_H

#pragma pack(1)

#define BIOS_FIXED_HOST_ADDR          0
#define PREBOOT_FIXED_SEC_ADDR         7
#define BIOS_ASF_HOST_ADDR            1

#define HECI_CORE_MESSAGE_ADDR        0x07
#define HECI_ASF_MESSAGE_ADDR         0x02
#define HECI_FSC_MESSAGE_ADDR         0x03
#define HECI_POLICY_MANAGER_ADDR      0x05
#define HECI_TDT_MESSAGE_ADDR         0x05  // Added to support TDT
#define HECI_SEC_PASSWORD_SERVICE_ADDR 0x06
#define HECI_ICC_MESSAGE_ADDR         0x08
#define HECI_TR_MESSAGE_ADDR          0x09
#define HECI_SPI_MESSAGE_ADDR         0x0A

#define NON_BLOCKING                  0
#define BLOCKING                      1
#define LONG_BLOCKING                 2

//
// command handle by HCI
//
#define GEN_GET_MKHI_VERSION_CMD      0x01
#define GEN_GET_MKHI_VERSION_CMD_ACK  0x81
#define GEN_GET_FW_VERSION_CMD        0x02
#define GEN_GET_FW_VERSION_CMD_ACK    0x82
#define GEN_UNCFG_WO_PWD_CMD          0x0D
#define GEN_UNCFG_WO_PWD_CMD_ACK      0x8D
#define GEN_SET_MFG_MRST_AND_HALT_CMD 0x10
#define GEN_EXIT_BOOT_SERVICES_CMD    0x20

#define FWCAPS_GET_RULE_CMD           0x02
#define FWCAPS_SET_RULE_CMD           0x03

#define TDT_SEC_RULE_ID                0xd0000

//
// Enums for Result field of MHKI Header
//
#define SEC_SUCCESS                  0x00
#define SEC_ERROR_ALIAS_CHECK_FAILED 0x01
#define SEC_INVALID_MESSAGE          0x02
#define SEC_M1_DATA_OLDER_VER        0x03
#define SEC_M1_DATA_INVALID_VER      0x04
#define SEC_INVALID_M1_DATA          0x05

//
// MDES
//
#define MDES_ENABLE_MKHI_CMD      0x09
#define MDES_ENABLE_MKHI_CMD_ACK  0x89

//
// Manageability State Control
//
#define FIRMWARE_CAPABILITY_OVERRIDE_CMD      0x14
#define FIRMWARE_CAPABILITY_OVERRIDE_CMD_ACK  0x94

//
// Unconfiguration
//
#define SEC_UNCONFIGURATION_CMD     0x0d
#define SEC_UNCONFIGURATION_CMD_ACK 0x8D
#define SEC_UNCONFIGURATION_STATUS  0x0e
#define SEC_UNCONFIGURATION_STATUS_ACK  0x8e



//
// Typedef for GroupID
//
typedef enum {
  MKHI_CBM_GROUP_ID   = 0,
  MKHI_PM_GROUP_ID,
  MKHI_PWD_GROUP_ID,
  MKHI_FWCAPS_GROUP_ID,
  MKHI_APP_GROUP_ID,
  MKHI_SPI_GROUP_ID,
  MKHI_MDES_GROUP_ID  = 8,
  MKHI_MAX_GROUP_ID,
  MKHI_GEN_GROUP_ID   = 0xFF
} MKHI_GROUP_ID;

//
// Typedef for AT State
//
typedef enum _TDT_STATE {
  TDT_STATE_INACTIVE  = 0,
  TDT_STATE_ACTIVE,
  TDT_STATE_STOLEN,
  TDT_STATE_SUSPEND,
  TDT_STATE_MAX
} TDT_STATE;

//
// MKHI host message header. This header is part of HECI message sent from MEBx via
// Host Configuration Interface (HCI). ME Configuration Manager or Power Configuration
// Manager also include this header with appropriate fields set as part of the
// response message to the HCI.
//
typedef union _MKHI_MESSAGE_HEADER {
  UINT32  Data;
  struct {
    UINT32  GroupId : 8;
    UINT32  Command : 7;
    UINT32  IsResponse : 1;
    UINT32  Reserved : 8;
    UINT32  Result : 8;
  } Fields;
} MKHI_MESSAGE_HEADER;
//
// C_ASSERT(sizeof(MKHI_MESSAGE_HEADER) == 4);
//

//
// End of Post ACK message
//
typedef struct _CBM_EOP_ACK_DATA {
  UINT32  RequestedActions;
} CBM_EOP_ACK_DATA;

typedef struct _GEN_END_OF_POST_ACK {
  MKHI_MESSAGE_HEADER Header;
  CBM_EOP_ACK_DATA    Data;
} GEN_END_OF_POST_ACK;

typedef struct _GEN_EXIT_BOOT_SERVICES {
  MKHI_MESSAGE_HEADER Header;
}GEN_EXIT_BOOT_SERVICES;

typedef struct _GEN_EXIT_BOOT_SERVICES_ACK {
  MKHI_MESSAGE_HEADER Header;
}GEN_EXIT_BOOT_SERVICES_ACK;

typedef union _MKHI_VERSION {
  UINT32  Data;
  struct {
    UINT32  Minor : 16;
    UINT32  Major : 16;
  } Fields;
} MKHI_VERSION;

typedef struct _FW_VERSION {
  UINT32  CodeMinor : 16;
  UINT32  CodeMajor : 16;
  UINT32  CodeBuildNo : 16;
  UINT32  CodeHotFix : 16;
  UINT32  RcvyMinor : 16;
  UINT32  RcvyMajor : 16;
  UINT32  RcvyBuildNo : 16;
  UINT32  RcvyHotFix : 16;
} FW_VERSION;

//
// MKHI version messages
//
typedef struct _GEN_GET_MKHI_VERSION {
  MKHI_MESSAGE_HEADER MKHIHeader;
} GEN_GET_MKHI_VERSION;

typedef struct _GET_MKHI_VERSION_ACK_DATA {
  MKHI_VERSION  MKHIVersion;
} GET_MKHI_VERSION_ACK_DATA;

typedef struct _GEN_GET_MKHI_VERSION_ACK {
  MKHI_MESSAGE_HEADER       MKHIHeader;
  GET_MKHI_VERSION_ACK_DATA Data;
} GEN_GET_MKHI_VERSION_ACK;

typedef struct _GEN_SET_MFG_MRST_AND_HALT {
  MKHI_MESSAGE_HEADER       MKHIHeader;
} GEN_SET_MFG_MRST_AND_HALT;

//
// FW version messages
//
typedef struct _GEN_GET_FW_VER {
  MKHI_MESSAGE_HEADER MKHIHeader;
} GEN_GET_FW_VER;

typedef struct _GEN_GET_FW_VER_ACK_DATA {
  UINT32  CodeMinor   : 16; // Same as firmware fields
  UINT32  CodeMajor   : 16; // same as firmware fields
  UINT32  CodeBuildNo : 16; // same as firmware fields
  UINT32  CodeHotFix  : 16; // same as firmware fields
  UINT32  RcvyMinor   : 16; // Same as firmware fields
  UINT32  RcvyMajor   : 16; // same as firmware fields
  UINT32  RcvyBuildNo : 16; // same as firmware fields
  UINT32  RcvyHotFix  : 16; // same as firmware fields
  UINT32  FITCMinor   : 16; // same as firmware fields
  UINT32  FITCMajor   : 16; // same as firmware fields
  UINT32  FITCBuildNo : 16; // same as firmware fields
  UINT32  FITCHotFix  : 16; // same as firmware fields

} GEN_GET_FW_VER_ACK_DATA;

typedef struct _GEN_GET_FW_VER_ACK {
  MKHI_MESSAGE_HEADER     MKHIHeader;
  GEN_GET_FW_VER_ACK_DATA Data;
} GEN_GET_FW_VER_ACK;

//
// Unconfig without password messages
//
typedef struct _GEN_UNCFG_WO_PWD {
  MKHI_MESSAGE_HEADER MKHIHeader;
} GEN_UNCFG_WO_PWD;

typedef struct _GEN_UNCFG_WO_PWD_ACK {
  MKHI_MESSAGE_HEADER MKHIHeader;
} GEN_UNCFG_WO_PWD_ACK;

//
// Get Firmware Capability MKHI
//
typedef struct _GET_RULE_DATA {
  UINT32  RuleId;
} GET_RULE_DATA;

typedef struct _GEN_GET_FW_CAPSKU {
  MKHI_MESSAGE_HEADER MKHIHeader;
  GET_RULE_DATA       Data;
} GEN_GET_FW_CAPSKU;

typedef union _RULE_ID {
  UINT32  Data;
  struct {
    UINT32  RuleTypeId : 16;
    UINT32  FeatureId : 8;
    UINT32  Reserved : 8;
  } Fields;
} RULE_ID;

typedef struct _SET_RULE_DATA {
  RULE_ID RuleId;
  UINT8   RuleDataLen;
  UINT32   RuleData;
} SET_RULE_DATA;

typedef struct _SET_RULE_ACK_DATA {
  UINT32  RuleId;
} SET_RULE_ACK_DATA;

typedef struct _GEN_SET_FW_CAPSKU {
  MKHI_MESSAGE_HEADER MKHIHeader;
  SET_RULE_DATA       Data;
} GEN_SET_FW_CAPSKU;

typedef struct _GEN_SET_FW_CAPSKU_ACK {
  MKHI_MESSAGE_HEADER MKHIHeader;
  SET_RULE_ACK_DATA   Data;
} GEN_SET_FW_CAPSKU_ACK;
typedef union _SECFWCAPS_SKU {
  UINT32  Data;
  struct {
    UINT32  Reserved : 5;        // [4:0] Reserved
    UINT32  IntelAT : 1;          // [5] IntelR Anti-Theft (AT)
    UINT32  Reserved1 : 4;         // [9:6] Reserved
    UINT32  IntelMPC : 1;         // [10] IntelR Power Sharing Technology (MPC)
    UINT32  IccOverClocking : 1;  // [11] ICC Over Clocking
    UINT32  PAVP : 1;             // [12] Protected Audio Video Path (PAVP)
    UINT32  Reserved2 : 4;        // [16:13] Reserved
    UINT32  IPV6 : 1;             // [17] IPV6
    UINT32  KVM : 1;              // [18] KVM Remote Control (KVM)
    UINT32  OCH : 1;              // [19] Outbreak Containment Heuristic (OCH)
    UINT32  VLAN : 1;             // [20] Virtual LAN (VLAN)
    UINT32  TLS : 1;              // [21] TLS
    UINT32  Reserved4 : 1;        // [22] Reserved
    UINT32  WLAN : 1;             // [23] Wireless LAN (WLAN)
    UINT32  Reserved5 : 8;        // [31:24] Reserved
  } Fields;
} SECFWCAPS_SKU;

typedef struct _TDT_STATE_FLAG {
  UINT16  LockState : 1;          /** Indicate whether the platform is locked */
  UINT16  AuthenticateModule : 1; /** Preferred Authentication Module */
  UINT16  Reserved : 14;
} TDT_STATE_FLAG;

typedef struct _TDT_STATE_INFO {
  UINT8           State;
  UINT8           LastTheftTrigger;
  TDT_STATE_FLAG  flags;
} TDT_STATE_INFO;

typedef struct {
  UINT8   AtState;                // State of AT FW
  UINT8   AtLastTheftTrigger;     // Reason for the last trigger
  UINT16  AtLockState;            // If AT Fw locked?
  UINT16  AtAmPref;               // TDTAM or PBA
} AT_STATE_STRUCT;

typedef enum  _TDT_AM_SELECTION {
  TDT_AM_SELECTION_TDTAM = 0,
  TDT_AM_SELECTION_PBAM,
  TDT_AM_SELECTION_MAX
} TDT_AM_SELECTION;

typedef struct _GEN_GET_FW_CAPS_SKU_ACK_DATA {
  UINT32        RuleID;
  UINT8         RuleDataLen;
  SECFWCAPS_SKU  FWCapSku;
} GEN_GET_FW_CAPS_SKU_ACK_DATA;

typedef struct _GEN_GET_FW_CAPSKU_ACK {
  MKHI_MESSAGE_HEADER           MKHIHeader;
  GEN_GET_FW_CAPS_SKU_ACK_DATA  Data;
} GEN_GET_FW_CAPS_SKU_ACK;

typedef enum {
  UPDATE_DISABLED     = 0,
  UPDATE_ENABLED
} LOCAL_FW_UPDATE;

typedef enum {
  LOCAL_FW_ALWAYS     = 0,
  LOCAL_FW_NEVER,
  LOCAL_FW_RESTRICTED,
} LOCAL_FW_QUALIFIER;

typedef struct _GEN_LOCAL_FW_UPDATE_DATA {
  UINT32  RuleId;
  UINT8   RuleDataLen;
  UINT32   RuleData;
} GEN_LOCAL_FW_UPDATE_DATA;

typedef struct _GEN_GET_LOCAL_FW_UPDATE {
  MKHI_MESSAGE_HEADER MKHIHeader;
  GET_RULE_DATA       Data;
} GEN_GET_LOCAL_FW_UPDATE;

typedef struct _GEN_GET_LOCAL_FW_UPDATE_ACK {
  MKHI_MESSAGE_HEADER       MKHIHeader;
  GEN_LOCAL_FW_UPDATE_DATA  Data;
} GEN_GET_LOCAL_FW_UPDATE_ACK;

typedef struct _GEN_SET_LOCAL_FW_UPDATE {
  MKHI_MESSAGE_HEADER       MKHIHeader;
  GEN_LOCAL_FW_UPDATE_DATA  Data;
} GEN_SET_LOCAL_FW_UPDATE;

typedef struct _GEN_SET_LOCAL_FW_UPDATE_ACK {
  MKHI_MESSAGE_HEADER MKHIHeader;
  GET_RULE_DATA       Data;
} GEN_SET_LOCAL_FW_UPDATE_ACK;

typedef enum {
  NO_BRAND                        = 0,
  INTEL_AMT_BRAND,
  INTEL_STAND_MANAGEABILITY_BRAND,
  INTEL_LEVEL_III_MANAGEABILITY_UPGRADE_BRAND,
} PLATFORM_BRAND;

typedef enum {
  INTEL_SEC_IGN_FW                 = 1,
  RESERVED_FW,
  INTEL_SEC_1_5MB_FW,
  INTEL_SEC_5MB_FW,
} SEC_IMAGE_TYPE;

#define REGULAR_SKU               0
#define SUPER_SKU                 1

#define PLATFORM_MARKET_CORPORATE 1
#define PLATFORM_MARKET_CONSUMER  2

#define PLATFORM_MOBILE           1
#define PLATFORM_DESKTOP          2
#define PLATFORM_SERVER           4
#define PLATFORM_WORKSTATION      8

typedef union _PLATFORM_TYPE_RULE_DATA {
  UINT32  Data;
  struct {
    UINT32  PlatformTargetUsageType : 4;
    UINT32  PlatformTargetMarketType : 2;
    UINT32  SuperSku : 1;
    UINT32  Reserved : 1;
    UINT32  IntelSeCFwImageType : 4;
    UINT32  PlatformBrand : 4;
    UINT32  Reserved1 : 16;
  } Fields;
} PLATFORM_TYPE_RULE_DATA;

typedef struct _GEN_PLATFORM_TYPE_DATA {
  UINT32                  RuleId;
  UINT8                   RuleDataLen;
  PLATFORM_TYPE_RULE_DATA RuleData;
  UINT8                   Reserved[27];
} GEN_PLATFORM_TYPE_DATA;

typedef struct _GEN_GET_PLATFORM_TYPE {
  MKHI_MESSAGE_HEADER MKHIHeader;
  GET_RULE_DATA       Data;
} GEN_GET_PLATFORM_TYPE;

typedef struct _GEN_GET_PLATFORM_TYPE_ACK {
  MKHI_MESSAGE_HEADER     MKHIHeader;
  GEN_PLATFORM_TYPE_DATA  Data;
} GEN_GET_PLATFORM_TYPE_ACK;

typedef struct _GET_TDT_SEC_RULE_CMD {
  MKHI_MESSAGE_HEADER MKHIHeader;
  UINT32              RuleId;

} GET_TDT_SEC_RULE_CMD;

typedef struct _GET_TDT_SEC_RULE_RSP {
  MKHI_MESSAGE_HEADER MKHIHeader;
  UINT32              RuleId;
  UINT8               RuleDataLength;
  TDT_STATE_INFO      TdtRuleData;

} GET_TDT_SEC_RULE_RSP;

typedef struct _GET_FW_FEATURE_STATUS {
  MKHI_MESSAGE_HEADER MKHIHeader;
  GET_RULE_DATA       Data;
} GEN_GET_FW_FEATURE_STATUS;

typedef struct _GET_FW_FEATURE_STATUS_ACK {
  MKHI_MESSAGE_HEADER MKHIHeader;
  UINT32              RuleId;
  UINT8               RuleDataLength;
  SECFWCAPS_SKU        RuleData;
} GEN_GET_FW_FEATURE_STATUS_ACK;

typedef struct _GEN_AMT_BIOS_SYNCH_INFO {
  MKHI_MESSAGE_HEADER MKHIHeader;
  GET_RULE_DATA       Data;
} GEN_AMT_BIOS_SYNCH_INFO;

typedef struct _GEN_AMT_BIOS_SYNCH_INFO_ACK {
  MKHI_MESSAGE_HEADER MKHIHeader;
  UINT32              RuleId;
  UINT8               RuleDataLength;
  UINT32              RuleData;
} GEN_AMT_BIOS_SYNCH_INFO_ACK;

typedef struct _GEN_GET_OEM_TAG_MSG {
  MKHI_MESSAGE_HEADER MKHIHeader;
  GET_RULE_DATA       Data;
} GEN_GET_OEM_TAG_MSG;

typedef struct _GEN_GET_OEM_TAG_MSG_ACK {
  MKHI_MESSAGE_HEADER MKHIHeader;
  UINT32              RuleId;
  UINT8               RuleDataLength;
  UINT32              RuleData;
} GEN_GET_OEM_TAG_MSG_ACK;

typedef struct _GEN_MDES_ENABLE_MKHI_CMD_MSG {
  MKHI_MESSAGE_HEADER MKHIHeader;
  UINT8               Data;
} GEN_MDES_ENABLE_MKHI_CMD_MSG;

//
// Manageability State Control MKHI definitions
//
typedef struct _FIRMWARE_CAPABILITY_OVERRIDE_DATA {
  UINT32  EnableFeature;
  UINT32  DisableFeature;
} FIRMWARE_CAPABILITY_OVERRIDE_DATA;

typedef struct _FIRMWARE_CAPABILITY_OVERRIDE {
  MKHI_MESSAGE_HEADER               MKHIHeader;
  FIRMWARE_CAPABILITY_OVERRIDE_DATA FeatureState;
} FIRMWARE_CAPABILITY_OVERRIDE;

typedef enum _FIRMWARE_CAPABILITY_RESPONSE {
  SET_FEATURE_STATE_ACCEPTED                  = 0,
  SET_FEATURE_STATE_REJECTED
} FIRMWARE_CAPABILITY_RESPONSE;

typedef struct _FIRMWARE_CAPABILITY_OVERRIDE_ACK_DATA {
  FIRMWARE_CAPABILITY_RESPONSE  Response;
} FIRMWARE_CAPABILITY_OVERRIDE_ACK_DATA;

typedef struct _FIRMWARE_CAPABILITY_OVERRIDE_ACK {
  MKHI_MESSAGE_HEADER                   Header;
  FIRMWARE_CAPABILITY_OVERRIDE_ACK_DATA Data;
} FIRMWARE_CAPABILITY_OVERRIDE_ACK;

/*
// UnConfiguration
//
typedef struct _SEC_UNCONFIGURATION_CMD {
  MKHI_MESSAGE_HEADER   MKHIHeader;
} SEC_UNCONFIGURATION_CMD;

typedef struct _SEC_UNCONFIGURATION_ACK {
  MKHI_MESSAGE_HEADER     MKHIHeader;
} SEC_UNCONFIGURATION_CMD_ACK;
*/
#pragma pack()

#endif // _MKHI_MSGS_H
