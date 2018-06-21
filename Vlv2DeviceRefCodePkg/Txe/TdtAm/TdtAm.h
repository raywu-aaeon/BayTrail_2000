/*++

Copyright (c) 2004-2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  TDTAm.h

Abstract:

  TDT authetication module for using TDT DXE driver.
  This driver uses the TDT protocol, HECI Protocol and TDT Platform Policy to implement
  Theft Deterrence Technology AM module.

--*/
/*++
 This file contains an 'Intel Peripheral Driver' and is
 licensed for Intel CPUs and chipsets under the terms of your
 license agreement with Intel or your vendor.  This file may
 be modified by the user, subject to additional terms of the
 license agreement
--*/

#ifndef _TDTAM_H_
#define _TDTAM_H_

#include "PiDxe.h"


#include <Library/SeCLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Protocol/FirmwareVolume.h>
#include <Protocol/Heci.h>
#include <Protocol/Tdt.h>

#include <Guid/GlobalVariable.h>
#include <Protocol/TdtPlatformPolicy.h>
#include <Protocol/TdtOperation.h>

#define TDTAM_SETUP_PASSWORD_LENGTH 49
#define TDTAM_SETUP_TOKEN_LENGTH    20
#define TDTAM_SRTK_BASE32_INPUT     32

#define NONCE_LENGTH          16
#define STR_NONCE_LENGTH      33

#define TIMER_ONE_SECOND    (10 * 1000 * 1000)
#define TIMER_HALF_SECOND   (TIMER_ONE_SECOND / 2)
#define CharIsUpper(c) ((c >= L'A') && (c <= L'Z'))
#define CharIsLower(c) ((c >= L'a') && (c <= L'z'))
#define CharIsAlpha(c) (CharIsUpper(c) || CharIsLower(c))
#define CharIsNumeric(c) ((c >= L'0') && (c <= L'9'))
#define CharIsAlphaNumeric(c) (CharIsAlpha(c) || CharIsNumeric(c))
#define PBA_FAILED_THRESHOLD 3
#define TDT_SMS_RECOVERY_MAX_TIMEOUT  60 //Wait for 60 seconds

#define MAX_CMD_LEN             160
#define USER_PASSWORD_HASH      20
#define TDT_MSG_VERSION_MAJOR   2
#define TDT_MSG_VERSION_MINOR   0
#define TDT_MSG_VERSION(_maj, _min) (((_maj)<<16) | ((_min) & 0x0000ffff))
#define TDT_MSG_CURRENT_VERSION     TDT_MSG_VERSION(TDT_MSG_VERSION_MAJOR, TDT_MSG_VERSION_MINOR)


#define STR_LENGTH_WITH_NETWORK_AVAILABLE 15

//
// This enumeration defines the AT-p credential types.
//
typedef enum _TDT_CREDENTIAL_TYPE {
  TDT_CREDENTIAL_TYPE_RSVD = 0,
  /* User defined recovery passphrase */
  TDT_CREDENTIAL_TYPE_USER_PASSPHRASE,
  /* Server generated random token */
  TDT_CREDENTIAL_TYPE_SRTK,
  /* Boundary check */
  TDT_CREDENTIAL_TYPE_MAX
} TDT_CREDENTIAL_TYPE;


EFI_STATUS
EFIAPI
TdtAmEntryPoint (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  )

/*++

Routine Description:

  Entry point for the AT Driver.

Arguments:

  ImageHandle       Image handle of this driver.
  SystemTable       Global system service table.

Returns:

  EFI_SUCCESS           Initialization complete.
  EFI_UNSUPPORTED       The chipset is unsupported by this driver.
  EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.
  EFI_DEVICE_ERROR      Device error, driver exits abnormally.

--*/
;

EFI_STATUS 
TimerCreateTimer (
  EFI_EVENT        *Event,
  EFI_EVENT_NOTIFY Callback,
  VOID             *Context,
  EFI_TIMER_DELAY  Delay,
  UINT64           Trigger,
  EFI_TPL          CallBackTPL
  )

/*++

Routine Description:
  This creates a Timer for checking user input

Arguments:
  EFI_EVENT          - EFI_EVENT
  EFI_EVENT_NOTIFY - A call back funtion that needs to be called
  This Context
  Time delay
  EFI_TPL - A callback TPL

Returns:
  EFI_SUCCESS           The function completed successfully.
--*/
;

EFI_STATUS 
TimerStopTimer (
  EFI_EVENT *Event
  )
/*++

Routine Description:
  This stops the Timer created by TimerCreateTimer()

Arguments:
  EFI_EVENT          - EFI_EVENT

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
;

UINT8 
CheckRecoveryPassword (
  EFI_EVENT Event,
  VOID *ParentImageHandle
  )
/*++

Routine Description:
  This function check for the AT password entered by the user.

Arguments:
  EFI_EVENT          - EFI_EVENT that invoked this function
  ParentImageHandle - the parent proccess handle that invoked this function

Returns:
  None

--*/
;

VOID 
ProcessSuspendMode (
  EFI_EVENT Event,
  VOID        *ParentImageHandle,
  BOOLEAN     *SetTdtEnterSuspendState,
  UINT8       *TdtEnterSuspendState
  )
/*++

Routine Description:
  This function checks whether to put system into or pull out of Suspend mode

Arguments:
  EFI_EVENT          - EFI_EVENT that invoked this function
  ParentImageHandle - the parent proccess handle that invoked this function

Returns:
  None

--*/
;

VOID 
SetPwdTimeOut (
  IN  EFI_EVENT                Event,
  IN  VOID                     *Context
  )
/*++

Routine Description:
  This creates a Password Timeout for user input

Arguments:
  EFI_EVENT          - EFI_EVENT
  Pointer to a Boolean value for timeout

Returns:
  Boolean value for Timeout
--*/
;

EFI_STATUS 
GetRecoveryPassword (
  UINT8 *PasswordEntered,
  UINTN PasswordLength,
  UINTN *pTimeOut,
  UINT32 TimeLeft,
  UINT8 *NonceStr,
  CHAR16 *PasswordIn,
  UINT8 LastTrigger,
  UINTN usrRsp
  )
/*++

Routine Description:
  This GetRecoveryPassword() process the AT recovery password user input.

Arguments:
  PasswordEntered - Pointer to an array of ASCII user input
  PasswordLength - Integer value for password length
  pTimeOut - Integer value of password Timeout
  TimeLeft - UINT32 value of Timeleft to enter password
  NonceStr - Pointer to an array of ASCII nonce
  PasswordIn - Pointer to an array of UNICODE user input
  LastTrigger - Reason for AT stolen state in ASCII
  usrRsp - User response in Integer

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
;


int 
Base32Decode (
  UINT8 *encodedStr,
  UINT8 *decodedData
  )
/*++

Routine Description:
  This routine converts BASE32 values in Hex (ASCII).

Arguments:
  encodedStr  - Array of BASE32 encoded input in UINT8
  decodedData - Array of Hex (UINT8) i.e. BASE32 to Hex converted data

Returns:
  1 for Success and 0 - for failuers.

--*/
;


EFI_STATUS  
Base32Encode (
  UINT8 *encodedStr,
  UINTN *encodedLen,
  UINT8 *rawData,
  UINTN rawDataLen
  )
/*++

Routine Description:
  This routine converts the Hex into Base32

Arguments:
  encodedStr - UINT8 Pointer for the BASE32 encoded strings
  encodedLen - Length of the encoded BASE32 strings
  rawData -   UINT8 pointer for Hex data that needs to be converted to BASE32
  rawDataLen - Length og Hex data

Returns:
  EFI_SUCCESS           The function completed successfully.

--*/
;


VOID 
Uint8ToUnicode (
  IN  UINT8     *AsciiString,
  OUT CHAR16    *UnicodeString
  )
/*++

Routine Description:
  This routine converts UINT8 to CHAR16 i.e. Unicode

Arguments:
  AsciiString - Array of Ascii in UINT8 that needs to converted to CHAR16 unicode
  UnicodeString - Pointer to CHAR16 for converted Unicde Strings

Returns:
  UnicodeString as Pointer

--*/
;


#define MAX_HEX_BYTES 20


VOID
DecimalToHexString (
  UINT8 *decStr,
  UINT8 *u8Hex,
  UINTN hexIndex
  );

EFI_STATUS 
Get3gTdtMsg (
  UINT8  *TdtSmsMsg,
  UINT32 *MsgLength
  )
/*++

Routine Description:
  This routine receives unlock/lock message for TDT platform recovery/lock

Arguments:
  TdtSmsMsg  - TDT Unlock password received over SMS
  MsgId - Message ID that is received
  MsgLength - Length of message

Returns:
  EFI_SUCCESS   The function completed successfully.

--*/
;


EFI_STATUS 
Init3g (
  EFI_TDT_PROTOCOL          *pTdt
  )
/*++

Routine Description:
  This routine check for TDT3g protocol support

Arguments:
  none
Returns:
  EFI_SUCCESS   The function completed successfully.

--*/
;



VOID
ShowBuffer (
  UINT8 *Message,
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
;

#define TDT_MAX_MSG_LENGTH 512

typedef enum _TDTHI_COMPLETION_CODE {
  TDTHI_COMPCODE_SUCCESS = 0,
  TDTHI_COMPCODE_INTERNAL_FAILURE,
  TDTHI_COMPCODE_INVALID_OPERATION,
  TDTHI_COMPCODE_INVALID_PARAM = 6,
  TDTHI_COMPCODE_INVALID_MSG_LEN,
  TDTHI_COMPCODE_GENERAL_FAILURE,
  TDTHI_COMPCODE_MAXIMUM
} TDTHI_COMPLETION_CODE;


typedef enum _TDTHI_SUSPEND_STATE {
  TDTHI_SUSPEND_EXIT = 0,
  TDTHI_SUSPEND_ENTER
} TDTHI_SUSPEND_STATE;


#define TDT_NONCE_LENGTH   16
typedef unsigned char TDT_NONCE[TDT_NONCE_LENGTH];

//TDT host agent to TDT Control server messages
typedef enum {
  InitializeRendezvous = 1,
  ContinueRendezvous,
  InitializeTdtClient,
  RendezvousInProgress,
  MAX_RENDEZVOUS = 10,
  BeginEnrollment = 11,
  EndEnrollment,
  CreatePermit,
  PermitInstalled,
  MAX_ES = 20,
  ClientServerAIB_ACK,
  ClientServerAIB_NACK,
  ClientServerAIB_Location,
  ServerClientAIB_Kill,
  ServerClientAIB_AMPasswd,
  ServerClientAIB_SRTK,
  MAX_AIB = 30
} TDT_ClientToServerMsg_t;


//typedef struct _TDT_ClientServerSMSMsgHdr
//{
//  UINT32 Version;
//  TDT_ClientToServerMsg_t msgType;
//  UINT32 msgLength;
//} TDT_ClientServerSMSMsgHdr;

#define TDT_MAX_CLIENT_ID   31
typedef struct {
  UINT8   length;
  UINT8   id[TDT_MAX_CLIENT_ID];
} TDT_ClientId;

typedef struct {
  UINT32 Version;
  TDT_ClientId ClientId;
  TDT_ClientToServerMsg_t msgType;
  UINT32 msgLength;
} TDT_ClientServerSMSMsgHdr;


typedef struct _TDT_ClientServerSMSMsg {
  TDT_ClientServerSMSMsgHdr hdr;
  UINT8 msg[TDT_MAX_MSG_LENGTH];
} TDT_ClientServerSMSMsg;


typedef struct _TDT_ClientServerSMSAck_t {
  TDT_ClientServerSMSMsgHdr hdr;
  //TDTHI_COMPLETION_CODE lastMsgCode;
  TDT_STATE clientState;
  TDT_NONCE meNonce;
} TDT_ClientServerSMSAck_t;

BOOLEAN 
PEMSMSDecode (
  UINT8 *pPEMString,
  UINT32 lineLength,
  UINT8 *pDecodedData,
  UINT32 *pBufLength
  )
/*++

Routine Description:
  This routine encode the SMS body (excluding header) into PEM i.e. BASE64

Arguments:
  pPEMString  - Array of UINT8 BASE64 strings
  lineLength - Length of string
  pDecodedData - Pointer for the return value of the decoded Base64
  pBufLength - length of return array

Returns:
  1 for Success and 0 - for failuers.

--*/
;

BOOLEAN 
PEMSMSEncode (
  UINT8 *pData,
  UINT32 dataLength,
  UINT8 *pPEMString,
  UINT32 *pBufLength
  )
/*++

Routine Description:
  This routine encode the SMS body (excluding header) into PEM i.e. BASE64

Arguments:
  pData  - Array of UINT8 strings for SMS messgae
  dataLength - Length of string
  pPEMString - Pointer for the return value of the encoded Base64
  pBufLength - length of return array

Returns:
  1 for Success and 0 - for failuers.

--*/
;

BOOLEAN 
Base16Encode (
  UINT8 *pData,
  UINT32 dataLength,
  UINT8 *pEncodedData,
  UINT32 *pBufLength
  )
/*++

Routine Description:
  This routine encodes BASE16 header message of the SMS

Arguments:
  pData  - Array of UINT8 strings for SMS messgae
  dataLength - Length of string
  pEncodedData - Pointer for the return value of the encoded Base16
  pBufLength - length of return array

Returns:
  1 for Success and 0 - for failuers.

--*/
;

BOOLEAN 
Base16Decode (
  UINT8 *pString,
  UINT32 stringLen,
  UINT8 *pDecodedData,
  UINT32 *pBufLength
  )

/*++

Routine Description:
  This routine decodes BASE16 header message of the SMS

Arguments:
  pString  - Array of UINT8 strings received in SMS messgae
  stringLen - Length of string
  pDecodedData - Pointer for the return value of the decoded Base16
  pBufLength - length of return array

Returns:
  1 for Success and 0 - for failuers.

--*/
;

UINT8 
DecodeBase16Char (
  UINT8 base16char
  )
/*++

Routine Description:
  This routine decodes Hex characters

Arguments:
  base16char  - Hex characters

Returns:
  1 for Success and 0 - for failuers.

--*/
;

UINT8 
DecodePEMChar (
  UINT8 pemChar
  )

/*++

Routine Description:
  This routine decodes PEM characters

Arguments:
  pemChar  - PEM i.e. BASE64 characters

Returns:
  1 for Success and 0 - for failuers.

--*/
;



EFI_STATUS
ValidatePreferredAM (
  UINT8 *TdtState,
  UINT16 *TdtAmPref
  );

EFI_STATUS
DisplayIsvStrings ();


#define ISV_PLATFORM_ID_LENGTH  16
#define SERVER_SHORTCODE_LENGTH 16
#define DEFAULT_LANGUAGE_STRING 4
#define RECOVERY_STRING_LENGTH  256
#define MX_SMS_MESSAGES         99

#pragma pack(1)
typedef struct {

  UINT8 BiosLanguageSupport[DEFAULT_LANGUAGE_STRING];
  // 0-Do not look for Lock message from BIOS,
  // 1-Look for Lock message in BIOS
  UINT8 BiosLockOverWwan;

  // WWAN usage policy defined, if the BIOS can use 3G/wwan or not. This is based
  // user opting out/in during enrollment for 3G use
  UINT8 WwanOptOut;

  // Defines the number which will be used by BIOS before making BIOS AM as a
  // default recovery mechanism
  UINT16 PbaOverRideThreshold;

  // The value used by BIOS to try sending GPS location when AT stolen
  UINT8 BiosLocationBeconing;

  // This will be used for Notebook transfer authorization request
  UINT8  PlatformAuthReq;

  // This will be used for Notebook transfer authorization acknowledgement
  UINT8  PlatformAuthAck;

  // This will be used for AT Server short code
  UINT8  ServerShortCode[SERVER_SHORTCODE_LENGTH];

  // This allow ISV to set unique platform ID and will be used for displayed on need basis
  UINT16  IsvPlatformId[ISV_PLATFORM_ID_LENGTH];

} TDT_BIOS_RECOVERY_CONFIG;


/**
 * This enumeration defines the vendor string identifiers.
 */
typedef enum _TDT_VENDOR_STRING_ID {
  /**
   * Reserved.
   */
  TDT_VENDOR_STRING_ID_RSVD = 0,
  /**
   * User defined recovery passphrase
   */
  TDT_VENDOR_STRING_ID_RECOVERY_HELP,

  //TDT_VENDOR_STRING_ID_RECOVERY_HELP,
  /**
   * Server generated random token
   */
  TDT_CUSTOM_RECOVERY_ID_CONFIGURATIONS,
  /**
   * Boundary check.
   */
  TDT_VENDOR_STRING_ID_MAX
} TDT_VENDOR_STRING_ID;

#define TDT_BASE_DELAY_TIME   5
#define MIN_SYSTEM_RUN_TIME   60

#pragma pack()
EFI_STATUS
GetRecoveryConfig(
  TDT_BIOS_RECOVERY_CONFIG  *RecoveryConfig,
  UINT32                    *RecoveryStringLength
  );


EFI_STATUS
GetCurrentLang (
  OUT     UINT8              *Lang
  );

BOOLEAN
CheckTdtStatus(
  IN UINT32          SeCStatus
  );

VOID
PrepareDisplayWarnToShutDownSystem (
  );

VOID
SetTdtInfo(
  );

EFI_STATUS
GetPlatformTdtInfo (
  OUT TDT_INFOMATION *TdtInfo
  );

EFI_STATUS
GetPlatformTdtOperation (
  OUT TDT_PERFORM_OPERATION_ID *TdtOperation
  );

EFI_STATUS
InstallTdtOperation(
  IN EFI_HANDLE ImageHandle
  );

EFI_STATUS
PerformTdtOperation (
  BOOLEAN     *SetTdtEnterSuspendState,
  UINT8       *TdtEnterSuspendState
  );

#endif
