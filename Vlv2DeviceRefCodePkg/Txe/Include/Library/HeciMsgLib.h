/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2006 - 2015 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  HeciMsgLib.h

Abstract:

  Header file for Heci Message functionality

--*/
#ifndef _HECI_MESSAGE_LIB_H_
#define _HECI_MESSAGE_LIB_H_

#include <CoreBiosMsg.h>

#include <Protocol/Heci.h>
#include <Protocol/SeCPlatformPolicy.h>

//
// Reset Request Origin Codes.
//
#define PEI_HECI_REQ_ORIGIN_BIOS_MEMORY_INIT  0x01
#define PEI_HECI_REQ_ORIGIN_BIOS_POST         0x02
#define PEI_HECI_REQ_ORIGIN_AMTBX_LAN_DISABLE 0x03

//
// Unconfiguration Command status
//
#define SEC_UNCONFIG_SUCCESS        0x03
#define SEC_UNCONFIG_IN_PROGRESS    0x01
#define SEC_UNCONFIG_NOT_IN_PROGRESS    0x02
#define SEC_UNCONFIG_ERROR      0x80

//
// End of Post Codes
//
#define HECI_EOP_STATUS_SUCCESS       0x0
#define HECI_EOP_PERFORM_GLOBAL_RESET 0x1

#define EFI_SEC_FW_SKU_VARIABLE_GUID \
  { \
    0xe1a21d94, 0x4a20, 0x4e0e, 0xae, 0x9, 0xa9, 0xa2, 0x1f, 0x24, 0xbb, 0x9e \
  }

typedef struct {
  UINT32  SeCEnabled : 1;          // [0]     SEC enabled/Disabled
  UINT32  Reserved : 12;           // [12:1]   Reserved, must set to 0
  UINT32  AtSupported : 1;        // [13]    AT Support
  UINT32  Reserved2 : 18;           // [31:14]    Intel KVM supported
  UINT32  SeCMinorVer : 16;        // [47:32] SEC FW Minor Version.
  UINT32  SeCMajorVer : 16;        // [63:48] SEC FW Major Version.
  UINT32  SeCBuildNo : 16;         // [79:64] SEC FW Build Number.
  UINT32  SeCHotFixNo : 16;        // [95:80] SEC FW Hotfix Number
} SEC_CAP;

#define SET_LOCK_MASK                   0
#define GET_LOCK_MASK                   1

#define MAX_ASSET_TABLE_ALLOCATED_SIZE  0x2000
#define HECI_HWA_CLIENT_ID              11
#define HWA_TABLE_PUSH_CMD              0

#define MAX_FWU_DATA_SEGMENT_SIZE      128
#define FWU_PWD_MAX_SIZE               32


// Typedef for the commands serviced by the Fw Update service
typedef enum {
  FWU_GET_VERSION = 0,
  FWU_GET_VERSION_REPLY,
  FWU_START,
  //FWU_START2,
  FWU_START_REPLY,
  FWU_DATA,
  FWU_DATA_REPLY,    //5
  FWU_END,
  FWU_END_REPLY,
  FWU_GET_INFO,
  FWU_GET_INFO_REPLY,
  FWU_GET_FEATURE_STATE,  //10
  FWU_GET_FEATURE_STATE_REPLY,
  FWU_GET_FEATURE_CAPABILITY,
  FWU_GET_FEATURE_CAPABILITY_REPLY,
  FWU_GET_PLATFORM_TYPE,
  FWU_GET_PLATFORM_TYPE_REPLY,  //15
  FWU_VERIFY_OEMID,
  FWU_VERIFY_OEMID_REPLY,
  FWU_GET_OEMID,
  FWU_GET_OEMID_REPLY,
  FWU_IMAGE_COMPATABILITY_CHECK,    //20
  FWU_IMAGE_COMPATABILITY_CHECK_REPLY,
  FWU_GET_UPDATE_DATA_EXTENSION,
  FWU_GET_UPDATE_DATA_EXTENSION_REPLY,
  FWU_GET_RESTORE_POINT_IMAGE,
  FWU_GET_RESTORE_POINT_IMAGE_REPLY,
  //FWU_GET_RECOVERY_MODE,
  //FWU_GET_RECOVERY_MODE_REPLY,
  FWU_GET_IPU_PT_ATTRB,
  FWU_GET_IPU_PT_ATTRB_REPLY,
  FWU_GET_FWU_INFO_STATUS,
  FWU_GET_FWU_INFO_STATUS_REPLY,
  GET_ME_FWU_INFO,
  GET_ME_FWU_INFO_REPLY,
  FWU_CONFIRM_LIVE_PING,
  FWU_CONFIRM_LIVE_PING_REPLY,
  FWU_VERIFY_PWD,
  FWU_VERIFY_PWD_REPLY,
  FWU_INVALID_REPLY = 0xFF
} FWU_HECI_MESSAGE_TYPE;

#define HBM_CMD_ENUM        0x04
#define HBM_CMD_ENUM_REPLY  0x84
#define HBM_CMD_CLIENT_PROP        0x05
#define HBM_CMD_CLIENT_PROP_REPLY  0x85
#define HBM_CMD_CONNECT       0x06
#define HBM_CMD_CONNECT_REPLY 0x86
#define HBM_CMD_DISCONNECT         0x07
#define HBM_CMD_DISCONNECT_REPLY   0x87
#define HBM_CMD_FLOW_CONTROL       0x08

#define HECI_HBM_MSG_ADDR          0
#define HECI_MKHI_FWU_GROUP_ID     0x06
#define HECI_MKHI_FWUSTATUS_CMD_ID 0x02

#define ONE_SECOND_TIMEOUT  1000000
#define FWU_TIMEOUT         90


#pragma pack(1)

typedef enum _HWAI_TABLE_TYPE_INDEX {
  HWAI_TABLE_TYPE_INDEX_FRU_DEVICE  = 0,
  HWAI_TABLE_TYPE_INDEX_MEDIA_DEVICE,
  HWAI_TABLE_TYPE_INDEX_SMBIOS,
  HWAI_TABLE_TYPE_INDEX_ASF,
  HWAI_TABLE_TYPE_INDEX_MAX         = 4,
} HWAI_TABLE_TYPE_INDEX;

typedef struct _SINGLE_TABLE_POSITION {
  UINT16  Offset;
  UINT16  Length;
} SINGLE_TABLE_POSITION;

typedef struct _TABLE_PUSH_DATA {
  SINGLE_TABLE_POSITION Tables[HWAI_TABLE_TYPE_INDEX_MAX];
  UINT8                 TableData[1];
} TABLE_PUSH_DATA;

typedef union {
  UINT32  Data;
  struct {
    UINT32  MessageLength : 16;
    UINT32  Command : 4;          // only supported command would be HWA_TABLE_PUSH=0;
    UINT32  Reserved : 12;
  } Fields;
} AU_MESSAGE_HEADER;

typedef struct _AU_TABLE_PUSH_MSG {
  AU_MESSAGE_HEADER Header;
  TABLE_PUSH_DATA   Data;
} AU_TABLE_PUSH_MSG;

typedef struct _UPDATE_FLAGS {
  UINT32  RestorePoint: 1;
  UINT32  RestartOperation: 1;
  UINT32  UserRollback: 1;
  UINT32  Reserved: 29;
} UPDATE_FLAGS;

typedef struct {
  UINT32  Data1;
  UINT16  Data2;
  UINT16  Data3;
  UINT8   Data4[8];
} OEM_UUID;

typedef struct _VERSION {
  UINT16 Major;
  UINT16 Minor;
  UINT16 Hotfix;
  UINT16 Build;
} VERSION;

typedef struct _FWU_GET_VERSION_MSG {
  UINT32 MessageType;
} FWU_GET_VERSION_MSG;

typedef struct _FWU_GET_VERSION_MSG_REPLY {
  UINT32 MessageType;
  UINT32 Status;
  UINT32 Sku;
  UINT32 PCHVer;
  UINT32 Vendor;
  UINT32 LastFwUpdateStatus;
  UINT32 HwSku;
  VERSION CodeVersion;
  VERSION AMTVersion;
  UINT16  EnabledUpdateInterfaces;
  UINT16  SvnInFlash;
  UINT32  DataFormatVersion;
  UINT32  LastUpdateResetType;
} FWU_GET_VERSION_MSG_REPLY;

typedef struct _FWU_GET_OEMID_MSG {
  UINT32  MessageType;
} FWU_GET_OEMID_MSG;

typedef struct _FWU_GET_OEMID_MSG_REPLY {
  UINT32  MessageType;
  UINT32  Status;
  OEM_UUID OemId;
} FWU_GET_OEMID_MSG_REPLY;

typedef struct _FWU_VERIFY_OEMID_MSG {
  UINT32 MessageType;
  OEM_UUID OemId;
} FWU_VERIFY_OEMID_MSG;

typedef struct _FWU_VERIFY_OEMID_MSG_REPLY {
  UINT32 MessageType;
  UINT32 Status;
} FWU_VERIFY_OEMID_MSG_REPLY;

typedef struct _FWU_START_MSG {
  UINT32 MessageType;
  UINT32 Length;                               //Length of update image
  UINT8  UpdateType;                           //0 Full update, 1 parital update
  UINT8  PasswordLength;                       //Length of password not include NULL. For VLV2, no POR
  UINT8  PasswordData[FWU_PWD_MAX_SIZE];       //Password data not include NULL byte. For VLV2, no POR
  UINT32 IpuIdTobeUpdated;                     //Only for partial FWU
  UINT32 UpdateEnvironment;                    //0 default to normal update, 1 for emergency IFR update
  UPDATE_FLAGS UpdateFlags;
  OEM_UUID OemId;
  UINT32 Resv[4];
} FWU_START_MSG;

typedef struct _FWU_START_MSG_REPLY {
  UINT32 MessageType;
  UINT32 Status;
  UINT32 Resv[4];
} FWU_START_MSG_REPLY;

typedef struct _FWU_DATA_MSG {
  UINT32 MesageType;
  UINT32 Length;
  UINT8  Reserved[3];
  UINT8  Data[1];
} FWU_DATA_MSG;

typedef struct _FWU_DATA_MSG_REPLY {
  UINT32 MessageType;
  UINT32 Status;
} FWU_DATA_MSG_REPLY;

typedef struct _FWU_END_MSG {
  UINT32 MessageType;
} FWU_END_MSG;

typedef struct _FWU_END_MSG_REPLY {
  UINT32 MessageType;
  UINT32 Status; // 0 indicate success, else failure
  UINT32 ResetType;
  UINT32 Resv[4];
} FWU_END_MSG_REPLY;

typedef struct _HBM_ENUM_MSG {
  UINT8  Cmd;
  UINT8  Resv[3];
} HBM_ENUM_MSG;

typedef struct _HBM_ENUM_MSG_REPLY {
  UINT8  CmdReply;
  UINT8  Resv[3];
  UINT32  ValidAddresses[8];   //Valid addresses. totally 256 bits. Earch bit corresponding to one address.
} HBM_ENUM_MSG_REPLY;

typedef struct _HBM_CLIENT_PROP_MSG {
  UINT8  Cmd;
  UINT8  Address;
  UINT8  Resv[2];
} HBM_CLIENT_PROP_MSG;

typedef struct _HBM_CLIENT_RPOP_MSG_REPLY {
  UINT8 CmdReply;
  UINT8 Address;
  UINT8 Status;
  UINT8 Resv;
  OEM_UUID ProtocolName;
  UINT8 ProtocolVersion;
  UINT8 MaximumConnections;
  UINT8 FixedAddress;
  UINT8 SingleRecvBuffer;
  UINT32 MaxMessageLength;
} HBM_CLIENT_PROP_MSG_REPLY;

typedef struct _HBM_CONNECT_MSG {
  UINT8  Cmd;
  UINT8  SecAddress;
  UINT8  HostAddress;
  UINT8  Resv;
} HBM_CONNECT_MSG;

typedef struct _HBM_CONNECT_MSG_REPLY {
  UINT8  CmdReply;
  UINT8  SecAddress;
  UINT8  HostAddress;
  UINT8  Status;
} HBM_CONNECT_MSG_REPLY;

typedef struct _HBM_DISCONNECT_MSG {
  UINT8  Cmd;
  UINT8  SecAddress;
  UINT8  HostAddress;
  UINT8  Resv;
} HBM_DISCONNECT_MSG;

typedef struct _HBM_DISCONNECT_MSG_REPLY {
  UINT8  CmdReply;
  UINT8  SecAddress;
  UINT8  HostAddress;
  UINT8  Status;
} HBM_DISCONNECT_MSG_REPLY;

typedef struct _HBM_FLOW_CONTROL_MSG {
  UINT8  Cmd;
  UINT8  SecAddress;
  UINT8  HostAddress;
  UINT8  Resv[5];
} HBM_FLOW_CONTROL_MSG;


#pragma pack()

EFI_STATUS
HeciCheckFwuInProgress(
  IN OUT UINT8 *InProgress
  )
;

EFI_STATUS
HeciVerifyOemId(
  IN  OEM_UUID *OemId,
  IN  UINT8  SecAddress
  );

EFI_STATUS
HeciFwuWaitOnLastUpdate(
  )
/*++

    Routing Description

        Waits for last firmware update to be finished. Blocking call that might at most stall the system for 90 seconds.

    Arguments:
        N/A

    Returns:

           EFI_STATUS

--*/
;


EFI_STATUS
HeciConnectFwuInterface(
  IN OUT UINT8 *SecAddress,
  OUT UINT32 *MaxBufferSize
  )
/*++

    Routing Description

        Setup a dynamic connection for FWU.

    Arguments:
        SecAddress - Returns the SecAddress to be used by other FWU APIs
        MaxBufferSize - Specifies the maximum buffer size the FWU link allows.

    Returns:

           EFI_STATUS

--*/
;


EFI_STATUS
HeciDisconnectFwuInterface(
  IN UINT8 SecAddress,
  IN UINT32 MaxBufferSize
  )
/*++

      Routing Description

        Tear down a dynamic connection for FWU.

    Arguments:
        SecAddress - Returns the SecAddress to be used by other FWU APIs

    Returns:

        EFI_STATUS

--*/
;

EFI_STATUS
HeciBiDirectionFlowControl(
  IN UINT8 SecAddress
  )
/*++

    Routing Description

        Called by each FWU request API when both sec and host needs to inform each other  it's ready to accept any input.

    Arguments:
            SecAddress - the dynamic sec address the flow control

    Returns:

            EFI_STATUS

    --*/
;

EFI_STATUS
HeciHostToSecFlowControl(
  IN UINT8 SecAddress
  )
/*++

    Routing Description

        Called by each FWU request API when host needs to inform SEC it's ready to accept any input.

    Arguments:
        SecAddress - the dynamic sec address the flow control

    Returns:

        EFI_STATUS

--*/
;

EFI_STATUS
HeciSecToHostFlowControl()
/*++

      Routing Description

        Called by each FWU request API when there's a flow control message expected on the link.

      Arguments:
        N/A

      Returns:

        EFI_STATUS

--*/
;

EFI_STATUS
HeciSendFwuGetVersionMsg(
  OUT VERSION*  Version,
  IN UINT8 SecAddress
  )
/*++

    Routine Description:

            Get firmware version from FWU interface instead of MKHI.

    Argurments:
       Version - Returns the version number of current running SEC FW.
       SecAddress - Dynamic sec address for FWU connection

     Returns:

       EFI_STATUS

--*/
;

EFI_STATUS
HeciSendFwuStartMsg(
  IN UINT32 ImageLength,
  IN OEM_UUID *OemId,
  IN UINT8 SecAddress,
  IN UINT32 MaxBufferSize
  )
/*++

Routine Description:

    Send Local Firmware Update start message to firmware thru HECI. THe HECI link might be disconnectd passively
    by the SEC side due to unexpected error. Supports full update only.

Argurments:
   ImageLength  - Firmware image length to be updated
   OemId - The OemId to be passed to firmware for verification
   SecAddress - Dynamic sec address for FWU connection

 Returns:

   EFI_STATUS

--*/
;


EFI_STATUS
HeciSendFwuDataMsg(
  IN UINT8*  FwuData,
  IN UINT32  FwuDataSize,
  IN UINT8  SecAddress,
  IN UINT32 MaxBufferSize
  )
/*++

Routine Description:

    Send Local Firmware Update Data to firmware thru HECI.

Argurments:
   FwuData  - Firmware update data to be updated
   FwuDataSize - Firmware update data size to be passed to firmware
   SecAddress - Dynamic Sec Address used by FWU client
   MaxBufferSize - the max buffer length for FWU client allowed by SEC. Retrieved from the connect API.


 Returns:

   EFI_STATUS

--*/
;

EFI_STATUS
HeciSendFwuEndMsg(
  OUT UINT32 *ResetType,
  IN  UINT8  SecAddress
  )
/*++

    Routine Description:

        Send Local Firmware Update Data to firmware thru HECI

    Argurments:
       ResetType    -Reset type required for finishing firmware update.

     Returns:

       EFI_STATUS

--*/
;

EFI_STATUS
HeciSendFwuGetOemIdMsg(
  OUT  OEM_UUID  *OemId,
  IN   UINT8 SecAddress
  )
/*++

    Routine Description:

       Send Local Firmware Update Data to firmware thru HECI

    Argurments:
       OemId    -The OEM ID returned by firmware.
       SecAddress - Dynamic sec address for FWU.

     Returns:

       EFI_STATUS

--*/
;

EFI_STATUS
HeciFwuQueryUpdateStatus(
  IN OUT UINT32 *Percentage,
  IN OUT UINT32 *CurrentStage,
  IN OUT UINT32 *TotalStages,
  IN OUT UINT32 *LastUpdateStatus,
  IN OUT UINT32 *LastResetType,
  IN OUT UINT8  *InProgress
  )
/*++

    Routine Description:

         Query firmware update progress thru HECI

    Argurments:
         Percentage - Update percentage
         CurrentStage - curent stage of the update
         TotalStages  - Total stages of the update
         LastUpdateStatus - The latest error code for the update
         LastResetType - Last reset type required for the update
         InProgress - Indicates if the update is in progress.

    Returns:

       EFI_STATUS

--*/

;


EFI_STATUS
HeciAssetUpdateFwMsg (
  IN  TABLE_PUSH_DATA *AssetTableData,
  IN  UINT16          TableDataSize
  )
/*++

Routine Description:

  Send Hardware Asset Tables to firmware through HECI.

Arguments:

  AssetTableData - Hardware Asset Table Data
  TablesDataSize - Size of Asset table

Returns:

    EFI_STATUS

--*/
;

EFI_STATUS
HeciSendCbmResetRequest (
  IN  UINT8                             ResetOrigin,
  IN  UINT8                             ResetType
  )
/*++

Routine Description:

  Send Core BIOS Reset Request Message through HECI.

Arguments:

  ResetOrigin - Reset source
  ResetType   - Global or Host reset

Returns:

    EFI_STATUS

--*/
;


EFI_STATUS
HeciSendEndOfPostMessage (
  IN  VOID
  )
/*++

Routine Description:

  Send End of Post Request Message through HECI.

Arguments:

  None

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
HeciGetFwCapsSkuMsg (
  IN OUT GEN_GET_FW_CAPSKU       *MsgGenGetFwCapsSku,
  IN OUT GEN_GET_FW_CAPS_SKU_ACK *MsgGenGetFwCapsSkuAck
  )
/*++

Routine Description:

  Send Get Firmware SKU Request to SEC

Arguments:

  MsgGenGetFwCapsSku - Return message for Get Firmware Capability SKU
  MsgGenGetFwCapsSkuAck - Return message for Get Firmware Capability SKU ACK

Returns:
  EFI_STATUS

--*/
;

EFI_STATUS
HeciGetFwVersionMsg (
  IN OUT GEN_GET_FW_VER_ACK     *MsgGenGetFwVersionAck
  )
/*++

Routine Description:

  Send Get Firmware Version Request to SEC

Arguments:

  MsgGenGetFwVersionAck - Return themessage of FW version

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
HeciTrConfigMsg (
  IN  EFI_HECI_PROTOCOL  *Heci,
  IN  TR_CONFIG          *TrConfig
  )
/*++

Routine Description:

  Send Thermal Reporting message through HECI.

Arguments:
  Heci        The pointer of Heci protocol
  TrConfig    Thermal Reporting Configuration Setting

Returns:

  EFI_STATUS.

--*/
;

EFI_STATUS
HeciHmrfpoEnable (
  IN  UINT64                          Nonce,
  OUT UINT8                           *Result
  )
/*++

Routine Description:

  Sends a message to SEC to unlock a specified SPI Flash region for writing and receiving a response message.
  It is recommended that HMRFPO_ENABLE MEI message needs to be sent after all OROMs finish their initialization.

Arguments:

  Nonce  - Nonce received in previous HMRFPO_ENABLE Response Message
  Result -  HMRFPO_ENABLE response

Returns:

  EFI_STATUS.

--*/
;

EFI_STATUS
HeciHmrfpoLock (
  OUT UINT64                          *Nonce,
  OUT UINT32                          *FactoryDefaultBase,
  OUT UINT32                          *FactoryDefaultLimit,
  OUT UINT8                           *Result
  )
/*++

Routine Description:

  Sends a message to SEC to lock a specified SPI Flash region for writing and receiving a response message.

Arguments:

  Nonce               - Random number generated by Ignition SEC FW. When BIOS
                              want to unlock region it should use this value
                              in HMRFPO_ENABLE Request Message
  FactoryDefaultBase  - The base of the factory default calculated from the start of the SEC region.
                        BIOS sets a Protected Range (PR) register¡¦s "Protected Range Base" field with this value
                         + the base address of the region.
  FactoryDefaultLimit - The length of the factory image.
                        BIOS sets a Protected Range (PR) register¡¦s "Protected Range Limit" field with this value

  Result              - Status report

Returns:

  EFI_STATUS.

--*/
;

EFI_STATUS
HeciHmrfpoGetStatus (
  OUT UINT8                           *Result
  )
/*++

Routine Description:

  System BIOS sends this message to get status for HMRFPO_LOCK message.

Arguments:

  Result -  HMRFPO_GET_STATUS response

Returns:

  EFI_STATUS.

--*/
;

EFI_STATUS
HeciQueryKvmRequest (
  IN  UINT32                         QueryType,
  OUT UINT32                         *ResponseCode
  )
/*++

Routine Description:

  KVM support.

Arguments:

  QueryType:
    0 - Query Request
    1 - Cancel Request

  ResponseCode:
    1h - Continue, KVM session established.
    2h - Continue, KVM session cancelled.

Returns:

  EFI_STATUS.

--*/
;

EFI_STATUS
HeciGetLocalFwUpdate (
  OUT UINT32         *RuleData
  )
/*++

Routine Description:

  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to query the local firmware update interface status.

Arguments:

  RuleData -
     1 - local firmware update interface enable
     0 - local firmware update interface disable

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
HeciSetLocalFwUpdate (
  IN UINT8         RuleData
  )
/*++

Routine Description:

  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to enable or disable the local firmware update interface.
  The firmware allows a single update once it receives the enable command

Arguments:

  RuleData -
     1 - local firmware update interface enable
     0 - local firmware update interface disable

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
HeciSetSeCEnableMsg (
  IN VOID
  )
/*++

Routine Description:

  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to enable the ME State.
  The firmware allows a single update once it receives the enable command

Arguments:

  None

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
HeciSetSeCDisableMsg (
  UINT8 ruleData
  )
/*++

Routine Description:

  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to disable the SEC State.
  The firmware allows a single update once it receives the disable command

Arguments:

  ruleData      0- disable    1-enable

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
HeciHaltTxe (
  )
/*++

Routine Description:

  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to halt TXE.
  The firmware will go through a reset and then only load BUP. This state won't be changed until
  a host reset(warm or cold).

Arguments:

  N/A

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
HeciGetPlatformTypeMsg (
  OUT PLATFORM_TYPE_RULE_DATA   *RuleData
  )
/*++

Routine Description:

  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP)
  on the boot where host wants to get Ibex Peak platform type.
  One of usages is to utilize this command to determine if the platform runs in
  1.5M or 5M size firmware.

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
;

EFI_STATUS
HeciAmtBiosSynchInfo (
  OUT UINT32         *RuleData
  )
/*++

Routine Description:

  This message is sent by the BIOS on the boot where the host wants to get the firmware provisioning state.
  The firmware will respond to AMT BIOS SYNCH INFO message even after the End of Post.

Arguments:

  RuleData -
    Bit [2:0] Reserved
    Bit [4:3] Provisioning State
      00 - Pre -provisioning
      01 - In -provisioning
      02 - Post ¡Vprovisioning
    Bit [31:5] Reserved

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
HeciGetOemTagMsg (
  OUT UINT32         *RuleData
  )
/*++

Routine Description:

  The firmware will respond to GET OEM TAG message even after the End of Post (EOP).

Arguments:

  RuleData - Default is zero. Tool can create the OEM specific OEM TAG data.

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
EFIAPI
HeciSetIccClockEnables (
  IN UINT32 Enables,
  IN UINT32 EnablesMask,
  IN UINT32 Parameters,
  IN UINT64 Nonce
  )
/*++
Routine Description:
  Enables/disables clocks. Used to turn off clocks in unused pci/pcie slots.
Arguments:
  Enables - each bit means corresponding clock should be turned on (1) or off (0)
  EnablesMask - each bit means corresponding enable bit is valid (1) or should be ignored (0)
  Parameters - bit0 = 'retain clock enables at resume for S3', other bits = reserved, must be 0
  Nonce - secret number used to validate caller
Returns:
  EFI_UNSUPPORTED
  EFI_DEVICE_ERROR
--*/
;

EFI_STATUS
HeciLockIccRegisters (
  IN UINT8       AccessMode,
  IN OUT UINT32  *Mask,
  IN OUT UINT64  *Nonce
  )
/*++
Routine Description:
  Sets or reads Lock mask on ICC registers.
Arguments:
  AccessMode - 0 - set, 1 - get
  Mask -       mask of registers to become (for 'set' mode) or are (for 'get' mode) locked. Each bit represents a register. 0=lock, 1=don't lock
  Nonce -      this secret number must be used for every attempt to lock registers except first
Returns:
  EFI_UNSUPPORTED
  EFI_INVALID_PARAMETER
  EFI_DEVICE_ERROR
--*/
;

EFI_STATUS
HeciGetIccProfile (
  OUT UINT8*Profile
  )
/*++
Routine Description:
  retrieves the number of currently used ICC clock profile
Arguments:
  Profile - number of current ICC clock profile
Returns:
  EFI_UNSUPPORTED
--*/
;

EFI_STATUS
HeciSetIccProfile (
  IN UINT8 Profile
  )
/*++
Routine Description:
  Sets ICC clock profile to be used on next and following boots
Arguments:
  Profile - number of profile to be used
Returns:
  EFI_UNSUPPORTED
  EFI_DEVICE_ERROR
--*/
;

EFI_STATUS
HeciMdesCapabilityEnableMsg (
  OUT UINT8        *Data
  )
/*++

Routine Description:

  This message is used to enable the IntelR SEC Debug Event Service capability.
  Once firmware receives this message, the firmware will enable MDES capability.
  The firmware will automatically disable MDES capability after receiving End Of Post.

Arguments:

  Data -
    0x00   : Enable Success
    Others : Enable Failure

Returns:

  EFI_STATUS

--*/
;


EFI_STATUS
HeciGetAtFwStateInfoMsg (
  IN OUT AT_STATE_STRUCT       *AtStateInfo
  )
/*++

  Routine Description:
  Get AT State Information From FW

  Arguments:
  AtStateInfo                  - Pointer to structure to hold AT FW Data
Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
HeciFwFeatureStateOverride (
  IN UINT32  EnableState,
  IN UINT32  DisableState
  )
/*++

Routine Description:

  Sends the MKHI Enable/Disable manageability message

Arguments:

  EnableState - Enable Bit Mask
  DisableState - Disable Bit Mask

Returns:

  EFI_STATUS

--*/
;

EFI_STATUS
HeciGetFwFeatureStateMsg (
  OUT SECFWCAPS_SKU                *RuleData
  )
/*++

Routine Description:

  The Get FW Feature Status message is based on MKHI interface.
  This command is used by BIOS/IntelR MEBX to get firmware runtime status.
  The GET FW RUNTIME STATUS message doesn't need to check the HFS.
  FWInitComplete value before sending the command.
  It means this message can be sent regardless of HFS.FWInitComplete.

Arguments:

    RuleData    - SECFWCAPS_SKU message

Returns:

  EFI_STATUS

--*/
;
EFI_STATUS
HeciSeCUnconfigurationMsg (
  UINT32    *CmdStatus
  )
/*++

Routine Description:

  This message is sent by the BIOS or IntelR MEBX prior to the End of Post (EOP) on the boot
  where host wants to disable the SEC State.
  The firmware allows a single update once it receives the disable command

Arguments:

  None

Returns:

  EFI_STATUS

--*/
;
EFI_STATUS
HeciSeCUnconfigurationStatusMsg (
  UINT32    *CmdStatus
  )
/*++

Routine Description:

  This message is sent by the BIOS prior to the End of Post (EOP) on the boot
  where host wants to Unconfigure the SEC State.

Arguments:

  UINT32    *CmdStatus  // return the data
        0 = SEC_UNCONFIG_SUCCESS
        1 = SEC_UNCONFIG_IN_PROGRESS
        2 = SEC_UNCONFIG_NOT_IN_PROGRESS
        3 = SEC_UNCONFIG_ERROR



Returns:

  EFI_STATUS

--*/
;



#endif
