/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2007 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  TdtHi.h

Abstract:

  Definition of TDT Host Interface (TDTHI) Protocol messages between the
  TDT Service and HOST Applications.

--*/
#ifndef _TDT_HI_H
#define _TDT_HI_H

#define TDT_COMMAND         0
#define TDT_RESPONSE        1

#define TDT_SEC_RULE_ID      0xd0000
#define TDT_SEC_RULE_GROUP   0x03
#define TDT_SEC_RULE_COMMAND 0x02

//
// #pragma warning (disable: 4214 4200)
//
#pragma pack(1)
//
// /////////////////////////////////////////////////////////////////////////////
//                        TDTHI Protocol Identifier                          //
///////////////////////////////////////////////////////////////////////////////
//
// TDT Host Interface protocol GUID
//
#define TDTHI_PROTOCOL_GUID \
  { \
    0x3C4852D6, 0xD47B, 0x4f46, 0xB0, 0x5E, 0xB5, 0xED, 0xC1, 0xAA, 0x43, 0x0A \
  }

#define TDTHI_PROTOCOL_FIXED_GUID \
  { \
    0xfa8f55e8, 0xab22, 0x42dd, 0xb9, 0x16, 0x7d, 0xce, 0x39, 0x00, 0x25, 0x74 \
  }

//
// TDT Host Interface protocol version (for SEC FW internal use)
//
#define TDTHI_PROTOCOL_VERSION_SEC 1

//
// TDT Host Interface protocol major version
//
#define TDTHI_PROTOCOL_VERSION_MAJOR  4

//
// TDT Host Interface protocol minor version
//
#define TDTHI_PROTOCOL_VERSION_MINOR  0

//
// /////////////////////////////////////////////////////////////////////////////
//                         TDTHI Framing Structures                          //
///////////////////////////////////////////////////////////////////////////////
//
// Defines the TDTHI header and creates a type name.
//
// @dotfile TdtHiHeader.dot "TDTHI Header"
//
typedef struct _TDTHI_HEADER {
  //
  // The version number for the TDTHI protocol.
  //
  struct {
    /**
     * The major version number. The major version number shall advance if
     * and only if backwards compatibility is broken by a change to the
     * protocol. If SEC firmware doesn't specifically support the version
     * number supplied in the header, an error shall be returned.
     */
    UINT8 Major;

    /**
     * The minor version number. All minor versions under a given major
     * version shall be backwards compatible with prevision minor versions
     * under that major version. If the version number supplied in the
     * header is higher than the minor version number supported by SEC
     * firmware, an error shall be returned.
     */
    UINT8 Minor;
  } Version;

  /**
   * Specifies the command or response in the message.
   */
  struct {
    /**
     * The operation with which the command is associated.
     */
    UINT16  Code : 7;

    /**
     * Command/Response indicator:
     * 0 The message contains a command.
     * 1 The message contains a response.
     */
    UINT16  IsResponse : 1;

    /**
     * The functional category of the command
     *
     * @see _TDTHI_CMD_GROUP
     */
    UINT16  Category : 8;
  } Command;

  /**
   * The length in bytes of the message. The length applies to the message
   * body (excludes the header and signature fields).
   */
  UINT32  Length;
} TDTHI_HEADER;

/**
 * Maximum blob data size.
 */
#define TDT_BLOB_LENGTH_MAX 256

/**
 *   Structure that defines the format of the Blob to be stored and retrieved
 *   for the ISV software.
 */
typedef struct _TDT_BLOB {
  /** The length of the data to be securely stored */
  UINT32  Length;

  /** The data to be securely stored */
  UINT8   Value[TDT_BLOB_LENGTH_MAX];
} TDT_BLOB;

/**
 * Maximum nonce length.
 */
#define TDT_NONCE_LENGTH  16

/**
 * Type definition for a nonce. To prevent replay of commands, a nonce
 * shall be added to each command message. A nonce is a value that is used
 * once. Ideally, each nonce value should not be used more than once during
 * the lifetime of the TDT laptop computer. However, a nonce generation function
 * that guarantees a very low probability of nonce values being repeated shall
 * suffice.
 */
typedef UINT8 TDT_NONCE[TDT_NONCE_LENGTH];

/**
 * Command Completion Codes. Each response message contains a completion code
 * that indicates whether the command completed successfully, and the nature
 * of the failure if it did not.
 */
typedef enum _TDTHI_COMPLETION_CODE {
  /**
   * The command completed successfully.
  */
  TDTHI_COMPCODE_SUCCESS          = 0,

  /**
   * The command failed due to an internal error.
  */
  TDTHI_COMPCODE_INTERNAL_FAILURE,

  /**
   * The command is not recognized, or is recognized but not supported.
   */
  TDTHI_COMPCODE_INVALID_OPERATION,

  /**
   * A parameter value does not satisfy the requirements of the command.
   */
  TDTHI_COMPCODE_INVALID_PARAM    = 6,

  /**
   * The value of the length field in the message header is outside the
   * supported range for the command.
   */
  TDTHI_COMPCODE_INVALID_MSG_LEN,

  /**
   * The command failed, but no information about the nature of the failure is available.
   */
  TDTHI_COMPCODE_GENERAL_FAILURE,

  /**
   * Maximum value for a completion code.
   */
  TDTHI_COMPCODE_MAXIMUM
} TDTHI_COMPLETION_CODE;

//
// Useful definitions for basic commands (commands without a body)
//

/**
 * Type name for an unsigned command message that has no body (header only).
 */
typedef TDTHI_HEADER  TDTHI_BASIC_CMD;

/**
 * Definition and type name for a basic response. The basic response has a
 * header and a minimal body (just the Completion Code).
 */
typedef struct _TDTHI_BASIC_RSP {
  /** Message header */
  TDTHI_HEADER          Header;

  /** Completion code */
  TDTHI_COMPLETION_CODE CompletionCode;
} TDTHI_BASIC_RSP;

//
// /////////////////////////////////////////////////////////////////////////////
//                            TDT Command Groups                             //
///////////////////////////////////////////////////////////////////////////////
//

/**
 * Related TDTHI commands are grouped into several categories. Each command
 * code contains the code of its category. See #TDTHI_HEADER.
 */
typedef enum _TDTHI_CMD_GROUP {
  /**
   * Reserved.
   */
  TDTHI_CMD_GROUP_RESERVED        = 0,

  /**
   * Commands related to the SEC's theft protection capabilities.
   *
   * @see _TDTHI_THEFT_DETECT_GRP_CMD
   */
  TDTHI_CMD_GROUP_THEFT_DETECTION,

  /**
   * Commands related to non-volatile data storage services provided to
   * the FDE/FLE software present on the TDT laptop computer.
   *
   * @see _TDTHI_DATA_STORE_GRP_CMD
   */
  TDTHI_CMD_GROUP_DATA_STORAGE,

  /**
   * Commands related to securely recovering the TDT laptop computer for
   * TDT disablement actions.
   *
   * @see _TDTHI_RECOVERY_GRP_CMD
   */
  TDTHI_CMD_GROUP_RECOVERY,

  /**
   * Commands related to TDTHI infrastructure.
   *
   * @see _TDTHI_GENERAL_GRP_CMD
   */
  TDTHI_CMD_GROUP_GENERAL,

  /**
   * Event notifications serve to inform the SEC TDT Service when an
   * event occurs in the PBA or TDT server.
   *
   * @see _TDTHI_NOTIFICATION_GRP_CMD
   */
  TDTHI_CMD_GROUP_NOTIFICATIONS,

  /**
   * 3G NIC commands allow BIOS to initialize and request Radio
   * status from FW
   *
   * @see _TDTHI_3G_NIC_GRP_CMD
   */
  TDTHI_CMD_GROUP_3G_NIC,

  /**
   *Secure Boot related commands
   *
   *@see _TDTHI_SECBOOT_GRP_CMD
   */
  TDTHI_CMD_GROUP_SECBOOT,
   
  /**
   * Boundary check.
   */
  TDTHI_CMD_GROUP_MAX
} TDTHI_CMD_GROUP;

//
// /////////////////////////////////////////////////////////////////////////////
//              TDTHI THEFT DETECTION Group Commands                         //
///////////////////////////////////////////////////////////////////////////////
//

/**
 * Theft detection commands control and configure the theft protection
 * capabilities of the SEC TDT Service.
 */
typedef enum _TDTHI_THEFT_DETECT_GRP_CMD {
  /**
   * Returns the TDT state of the monitored system.
   *
   * @see TDTHI_GET_STATE_CMD
   */
  TDTHI_THEFT_DETECT_GRP_GET_STATE_CMD,

  /**
   * Returns the state of the specified timer.
   *
   * @see TDTHI_GET_TIMER_INFO_CMD
   */
  TDTHI_THEFT_DETECT_GRP_GET_TIMER_INFO_CMD,

  /**
   * Resets the specified timer to its configured reset interval.
   *
   * @see TDTHI_RESET_TIMER_CMD
   */
  TDTHI_THEFT_DETECT_GRP_RST_TIMER_CMD,

  /**
   * Configures the actions that the ME TDT Service shall take if the
   * specified event occurs.
   *
   * @see TDTHI_SET_POLICY_CMD
   */
  TDTHI_THEFT_DETECT_GRP_SET_POLICY_CMD,

  /**
   * Returns the actions that the SEC TDT Service shall take if the
   * specified event occurs.
   *
   * @see TDTHI_GET_POLICY_CMD
   */
  TDTHI_THEFT_DETECT_GRP_GET_POLICY_CMD,

  /**
   * Sets the reset interval for the specified timer. The timer counts down
   * and expires if it reaches 0.
   *
   * @see TDTHI_SET_TIMER_INTERVAL_CMD
   */
  TDTHI_THEFT_DETECT_GRP_SET_TIMER_INTERVAL_CMD,

  /**
   * Asserts that the TDT laptop is stolen. The SEC TDT Service shall proceed
   * immediately to the Stolen state, executing all actions in the
   * AssertStolenPolicy.
   *
   * @see TDTHI_ASSERT_STOLEN_CMD
   */
  TDTHI_THEFT_DETECT_GRP_ASSERT_STOLEN_CMD,

  /**
   * Same as the AssertStolen command but not signed.
   *
   * @see TDTHI_ASSERT_STOLEN_CMD
   */
  TDTHI_THEFT_DETECT_GRP_UNSIGNED_ASSERT_STOLEN_CMD,

  /**
   * Boundary check.
   */
  TDTHI_THEFT_DETECT_GRP_MAX
} TDTHI_THEFT_DETECT_GRP_CMD;

/**
 * This flag indicates that TDT is in the STOLEN state and the policies are set
 * to "do nothing". The mask must be applied to the TDT state rule.
 *
 * @ NOTE This flag is only cleared when the FW receives an AssertStolen
 * message.
 */
#define TDT_PSEUDO_STOLEN_STATE_MASK  0x00000080

/**
 * Enumeration of TDT theft triggers IDs
 */
typedef enum _TDT_THEFT_TRIGGER {
  /**
   * None.
   */
  TDT_THEFT_TRIGGER_NA                              = 0,

  /**
   * Theft trigger identifier for disable timer expiration
   */
  TDT_THEFT_TRIGGER_DISABLE_TIMER_EXPIRATION,

  /**
   * Theft trigger identifier for command driven disablement.
   */
  TDT_THEFT_TRIGGER_ASSERT_STOLEN,

  /**
   * Theft trigger identifier for PBA logon failure exceeded threshold.
   */
  TDT_THEFT_TRIGGER_THRESHOLD_EXCEEDED,

  /**
   * Theft trigger identifier for platform attack detection.
   */
  TDT_THEFT_TRIGGER_ATTACK_DETECTED,

  /**
   * Bounday check.
   */
  TDT_THEFT_TRIGGER_MAX
} TDT_THEFT_TRIGGER;

/**
 * Enumeration of TDT timer types.
 */
typedef enum _TDT_TIMER_ID {
  /**
   * Reserved.
   */
  TDT_TID_RSVD                                      = 0,

  /**
   * The disable timer is a variation on the watchdog timer. It is periodically
   * reset by ISV software to indicate that the system is behaving as expected.
   * If it is not reset before it expires, the SEC TDT Service shall execute the
   * protective actions configured in its disable timer policy.
   * The first line of defense when the system begins to exhibit behavior that
   * indicates it has been stolen is the ISV software, which may invoke the
   * TDTHI SetPolicy function to protect the platform.
   * The disable timer is designed to protect the system if the ISV software
   * is disabled. In this case, the ISV software will be unable to reset the
   * disable timer before it expires, and the SEC TDT Service will take the
   * configured protective actions.
   */
  TDT_TID_DISABLE_TIMER,

  /**
   * The unlock timer used when the TDT laptop computer is locked. The ME TDT Service
   * sets the recovery timer when it sees that the TDT state is Stolen during
   * the host boot flow". If the user doesn't successfully complete the
   * @ref platform_enable_flow "recovery flow before the unlock timer expires,
   * the SEC TDT Service shall power down the platform immediately.
   * The variable UnlockTimerInterval shall be configurable within a range
   * that is long enough to facilitate a bona fide recovery attempt, but short
   * enough to prevent exploitation of the platform by a thief that does not
   * have the recovery credential.
   */
  TDT_TID_UNLOCK_TIMER,

  /**
   * The grace timer allows the user of a TDT laptop computer time to take
   * actions to cause a ResetTimer command to be issued by the TDT service.
   * The grace period shall be applied if and only if the SEC TDT service
   * determines that the Disable Timer logically expired while the TDT
   * laptop computer was in Sx (sleeping).
   */
  TDT_TID_GRACE_TIMER,

  /**
   * The PBA Logon Timer shall be used to count down the allotted time for
   * PBA Logon. The SEC TDT Service shall start a PBA Logon Timer upon
   * transition from S4/S5 to S0. If the SEC TDT Service receives a property
   * authenticated PBA Logon notification message  before the PBA Logon Timer
   * expires, it shall stop the timer and allow operation to continue normally.
   * If instead the PBA Logon Timer expires before the SEC TDT Service has
   * received a properly authenticated PBA Logon notification message, the
   * SEC TDT Service shall power down the system immediately, but shall not
   * transition to the Stolen state.
   */
  TDT_TID_PBA_LOGON_TIMER,

  /**
   * The activity timer is used to extend the period of the Disable Timer in
   * the event that a TDTHI client has recently invoked a TDTHI command when
   * the Disable Timer expires. The intent is to allow the client to complete
   * a command flow that may include a ResetTimer command.
   * The TDT activity timer shall be reset each time a TDTHI command is completed.
   * The interval of the TDT activity timer shall be 30 seconds. This interval
   * shall not be externally configurable.
   */
  TDT_TID_ACTIVITY_TIMER,

  /**
   * Bounday check.
   */
  TDT_TID_MAX
} TDT_TIMER_ID;

/**
 * A timer with an interval of -1 means that the timer is disabled. Applies to
 * TDT_TIMER_CONFIG and TDT_TIMER_INFO.
 */
#define TDT_TIMER_DISABLED  0xFFFFFFFF

/**
 * Maximum timer value besides the disabled timer value. This value is due to
 * the maximum timer value allowed by the FW resources.
 */
#define TDT_TIMER_MAX (TDT_TIMER_DISABLED / 1024)

/**
 * Remaining grace timer in which the user is not assumed not to be able to
 * complete a TimerResetCmd due to OS context loading from S3 -> S0.
 *
 * Value is 30 seconds.
 */
#define TDT_LOW_GRACE_THRESHOLD 30

/**
 * Minumium DISABLE timer value (in seconds)
 */
#define TDT_TMR_DISABLE_MIN 60

/**
 * Minumium DISABLE timer value (in seconds)
 */
#define TDT_TMR_DISABLE_MAX TDT_TIMER_DISABLED

/**
 * Minumium UNLOCK timer value (in seconds)
 */
#define TDT_TMR_UNLOCK_MIN  60

/**
 * Minumium UNLOCK timer value (in seconds)
 */
#define TDT_TMR_UNLOCK_MAX  TDT_TIMER_MAX

/**
 * Minumium GRACE timer value (in seconds)
 */
#define TDT_TMR_GRACE_MIN 60

/**
 * Minumium GRACE timer value (in seconds)
 */
#define TDT_TMR_GRACE_MAX TDT_TIMER_MAX

/**
 * Minumium PBALOGON timer value (in seconds)
 */
#define TDT_TMR_PBALOGON_MIN  60

/**
 * Minumium PBALOGON timer value (in seconds)
 */
#define TDT_TMR_PBALOGON_MAX  TDT_TIMER_DISABLED

/**
 * Structure that contains the configuration and state information for
 * a specified timer.
 */
typedef struct _TDT_TIMER_INFO {
  /**
   * Identifies the timer that shall be configured. See #TDT_TIMER_ID.
   */
  UINT8   Id;

  /**
   * Padding for now
   */
  UINT8   Reserved[3];

  /**
   * The interval configured in the timer, in seconds.
   */
  UINT32  Interval;

  /**
   * The number of ticks remaining in the current timer countdown.
   */
  UINT32  TimeLeft;
} TDT_TIMER_INFO;

/**
 * Structure defining the PBAM Configuration.
 */
typedef struct _TDT_POLICY_PBA_CONFIG {
  UINT8 RunAfterPost : 1;
  UINT8 IsInstalled : 1;
  UINT8 Reserved : 6;
} TDT_POLICY_PBA_CONFIG;

/**
 * Optional union defining the PBAM Configuration.
 */
typedef union _TDT_POLICY_PBA_CONFIG_U {
  UINT8                 Value;
  TDT_POLICY_PBA_CONFIG Bits;
} TDT_POLICY_PBA_CONFIG_U;

/**
 * Enumeration of the available Authentication Modules
 */
/*
typedef enum _TDT_AM_SELECTION
{
  TDT_AM_SELECTION_TDTAM                            = 0,
  TDT_AM_SELECTION_PBAM,
  TDT_AM_SELECTION_MAX
} TDT_AM_SELECTION;
*/

/**
 * Enumeration of the platform lock state
 */
typedef enum _TDT_PLATFORM_LOCKSTATE {
  TDT_PLATFORM_LOCKSTATE_NOTLOCKED                  = 0,  /** Platform may be stolen but not locked  */
  TDT_PLATFORM_LOCKSTATE_LOCKED,                          /** Platfom is stolen and locked  */
  TDT_PLATFORM_LOCKSTATE_MAX
} TDT_PLATFORM_LOCKSTATE;

//
// Command definition structures
//

/**
 * The GetTDTState command. Returns the TDT state of the laptop computer.
 *
 * Sender:        TDT AM | PBA
 * Signing Key:   None   | None
 *
 * @dotfile TdtHiGetStateCmd.dot "GetState command"
 *
 * @see TDTHI_GET_STATE_RSP
 */
typedef TDTHI_BASIC_CMD TDTHI_GET_STATE_CMD;

/**
 * The GetTDTState response.
 *
 * @dotfile TdtHiGetStateRsp.dot "GetState response"
 *
 */
typedef struct _TDTHI_GET_STATE_RSP {
  TDTHI_HEADER          Header;
  TDTHI_COMPLETION_CODE CompletionCode;
  TDT_STATE_INFO        StateInfo;
} TDTHI_GET_STATE_RSP;

/**
 * The GetTimerInfo command. Returns configuration and state information about
 * the specified timer.
 *
 * Sender:        PBA
 * Signing Key:   PBASK
 *
 * @dotfile TdtHiGetTimerInfoCmd.dot "GetTimerInfo command"
 *
 * @see TDTHI_GET_TIMER_INFO_RSP
 */
typedef struct _TDTHI_GET_TIMER_INFO_CMD {
  TDTHI_HEADER  Header;

  /** see TDT_TIMER_ID for timer values */
  UINT8         TimerId;
} TDTHI_GET_TIMER_INFO_CMD;

/**
 * TDTHI_GET_TIMER_INFO response.
 *
 * @dotfile TdtHiGetTimerInfoRsp.dot "GetTimerInfo response"
 */
typedef struct _TDTHI_GET_TIMER_INFO_RSP {
  TDTHI_HEADER          Header;
  TDTHI_COMPLETION_CODE CompletionCode;
  TDT_TIMER_INFO        TimerInfo;
} TDTHI_GET_TIMER_INFO_RSP;

//
// /////////////////////////////////////////////////////////////////////////////
//              TDTHI RECOVERY Group Commands                                //
///////////////////////////////////////////////////////////////////////////////
//
typedef enum _TDTHI_RECOVERY_GRP_CMD {
  //
  // SetCredential stores the recovery credential in non-volatile storage,
  // where it is used for validation purposes when the credential is presented
  // by a user during recovery flow.
  //
  // @see #TDTHI_SET_CREDENTIAL_CMD
  //
  TDTHI_RECOVERY_GRP_SET_CREDENTIAL_CMD,
  //
  // AuthenticateRecoveryCredential presents the a credential to the ME TDT
  // Service for validation during the recovery flow.
  //
  // @see TDTHI_AUTHENTICATE_CREDENTIAL_CMD
  //
  TDTHI_RECOVERY_GRP_AUTH_CREDENTIAL_CMD,
  //
  // The ComputeHash command computes a specified hash of input data. Maximum
  // size of the digest value shall be 384 bits (48 bytes). This command may
  // be used to preprocess authentication tokens if the specified hash
  //  algorithm is not supported by the requester.
  //
  // @see TDTHI_COMPUTE_HASH_CMD
  //
  TDTHI_RECOVERY_GRP_COMPUTE_HASH_CMD,
  //
  // The DeAssertStolen command is used to recovery the platfrom from stolen state
  // by PBAM or TDT server
  // @see TDTHI_DEASSERT_STOLEN_CMD
  //
  TDTHI_RECOVERY_GRP_DEASSERT_STOLEN_CMD,
  //
  // The GetIsvId command exports the identity of the service provider (ISV)
  // that activated the TDT service
  // @see TDTHI_GET_ISVID_CMD
  //
  TDTHI_RECOVERY_GRP_GET_ISVID_CMD,
  //
  // Bounday check.
  //
  TDTHI_RECOVERY_GRP_MAX
} TDTHI_RECOVERY_GRP_CMD;

/**
 * This enumeration defines the credential types.
 */
typedef enum _TDT_CREDENTIAL_TYPE {
  /**
   * Reserved.
   */
  TDT_CREDENTIAL_TYPE_RSVD                          = 0,

  /**
   * User defined recovery passphrase
   */
  TDT_CREDENTIAL_TYPE_USER_PASSPHRASE,

  /**
   * Server generated random token
   */
  TDT_CREDENTIAL_TYPE_SRTK,

  /**
    * Server suspend token
    */
  TDT_CREDENTIAL_TYPE_SSTK,

  /**
    * Base key
    */
  TDT_CREDENTIAL_TYPE_BK,

  /**
    * Boundary check.
    */
  TDT_CREDENTIAL_TYPE_MAX
} TDT_CREDENTIAL_TYPE;

/**
 * This enumeration defines the salt IDs.
 */
typedef enum _TDT_HASH_ALGO_ID {
  /**
   * Reserved.
   */
  TDT_HASH_ALGO_ID_RSVD                             = 0,

  /**
   * SHA-1 algorithm (160-bit output)
   */
  TDT_HASH_ALGO_ID_SHA1,

  /**
   * Boundary check
   */
  TDT_HASH_ALGO_ID_MAX
} TDT_HASH_ALGO_ID;

/**
 * Defines the maximum length of a user passpharse recovery hash (will be 32 in 2009).
 */
#define TDT_USR_PASS_HASH_LENGTH_MAX  20

/**
 * Defines the maximum length of a SRTK recovery token
*/
#define TDT_SRTK_LENGTH_MAX 32

/**
 * Maximium credential length.
 */
#define TDT_CREDENTIAL_VALUE_LENGTH_MAX TDT_SRTK_LENGTH_MAX

#define TDT_PASSWORD_LENGTH             64

/**
 * Structure that defines a credential for storage, validation, and export operations.
 */
typedef struct _TDT_CREDENTIAL {
  /**
   * The credential type
   */
  TDT_CREDENTIAL_TYPE Type;

  /**
   * The credential length
   */
  UINT32              Length;

  /**
   * The credential value
   */
  UINT8               Value[1]; // Need a pointer but cannot make a zero length array
} TDT_CREDENTIAL;

/**
 * The AuthenticateCredential command. Sets the value of the specified
 * credential in the ME TDT Service.
 *
 * Sender:        TDT AM
 * Signing Key:   None
 *
 * @dotfile TdtHiAuthenticateCredentialCmd.dot "AuthenticateCredential command"
 *
 * @see TDTHI_AUTHENTICATE_CREDENTIAL_RSP
 */
typedef struct _TDTHI_AUTHENTICATE_CREDENTIAL_CMD {
  TDTHI_HEADER    Header;
  TDT_CREDENTIAL  Credential;
} TDTHI_AUTHENTICATE_CREDENTIAL_CMD;

/**
 * The AuthenticateCredential response.
 *
 * @dotfile TdtHiAuthenticateCredentialRsp.dot "AuthenticateCredential response"
 */
typedef struct _TDTHI_AUTHENTICATE_CREDENTIAL_RSP {
  TDTHI_HEADER          Header;
  TDTHI_COMPLETION_CODE CompletionCode;
  UINT8                 Authenticated;
} TDTHI_AUTHENTICATE_CREDENTIAL_RSP;

/**
 * Definition for the maximum hash output size.
 */
#define TDT_MAX_HASH_OUTPUT_SIZE  48

/**
 * The ComputeHash command. Computes a specified hash of input data. Maximum
 * size of the digest value shall be 384 bits (48 bytes).
 *
 * Sender:        TDT AM
 * Signing Key:   None
 *
 * @dotfile TdtHiComputeHashCmd.dot "ComputeHash command"
 *
 * @see TDTHI_COMPUTE_HASH_RSP
 */
typedef struct _TDTHI_COMPUTE_HASH_CMD {
  TDTHI_HEADER  Header;

  /** see TDT_HASH_ALGO_ID for values */
  UINT8         Algorithm;
  UINT8         InputLength;
  UINT8         InputBuffer[TDT_MAX_HASH_OUTPUT_SIZE];
} TDTHI_COMPUTE_HASH_CMD;

/**
 * The ComputeHash response.
 *
 * @dotfile TdtHiComputeHashRsp.dot "ComputeHash response"
 *
 */
typedef struct _TDTHI_COMPUTE_HASH_RSP {
  TDTHI_HEADER          Header;
  TDTHI_COMPLETION_CODE CompletionCode;

  /** see TDT_HASH_ALGO_ID for values */
  UINT8                 OutputLength;
  UINT8                 OutputBuffer[TDT_MAX_HASH_OUTPUT_SIZE];
} TDTHI_COMPUTE_HASH_RSP;

/**
 * The DeassertStolen command. Recover the laptop from stolen state. The SEC TDT
 * Service shall proceed immediately to the NotStolen state.
 *
 * Sender:        ISV SW
 * Signing Key:   PBASK, TSSK
 *
 * @dotfile TdtHiDeassertStolenCmd.dot "DeassertStolen command"
 *
 * @see TDTHI_DEASSERT_STOLEN_RSP
 */
typedef TDTHI_BASIC_CMD TDTHI_DEASSERT_STOLEN_CMD;

typedef TDTHI_BASIC_RSP TDTHI_DEASSERT_STOLEN_RSP;

//
// The GetIsvId command exports the identity of the service provider (ISV) that
// activated the TDT service. This allows for system recovery in cases where the user
// may not be sure who to contact when he/she is unable to recover the system locally.
// Note that this command is allowed only while the system in the STOLEN state
//
// Sender:        TDT AM
// Signing Key:   None
//
// * @dotfile
//
// @see TDTHI_GET_ISVID_RSP
//
typedef TDTHI_BASIC_CMD TDTHI_GET_ISVID_CMD;

typedef struct _TDTHI_GET_ISVID_RSP {
  TDTHI_HEADER          Header;
  TDTHI_COMPLETION_CODE CompletionCode;
  UINT32                IsvId;
} TDTHI_GET_ISVID_RSP;

//
// //////////////////////////////////////////////////////////////////////////////
//                      TDTHI General Group Commands                          //
////////////////////////////////////////////////////////////////////////////////
//
typedef enum _TDTHI_GENERAL_GRP_CMD {
  /**
   * GetNonce returns a nonce generated by the SEC TDT Service. The purpose of
   * this nonce is to prevent replay of TDT command messages.
   *
   * @see TDTHI_GET_NONCE_CMD
   */
  TDTHI_GENERAL_GRP_GET_NONCE_CMD,

  /**
   * GetTDTCapabilities requests that the SEC TDT Service return the
   * capabilities it has implemented.
   *
   * @see TDTHI_GET_TDT_CAPABILITIES_CMD
   */
  TDTHI_GENERAL_GRP_GET_TDT_CAPABILITIES_CMD,

  /**
   * The SetPublicKey command saves the specified public key of an RSA
   * key-pair for use by the SEC TDT Service.
   *
   * @see TDTHI_GET_PUBLIC_KEY_CMD
   */
  TDTHI_GENERAL_GRP_SET_PUBLIC_KEY_CMD,

  /**
   * The GetPublicKey command returns the specified public key of an RSA
   * key-pair owned by the SEC TDT Service.
   *
   * @see TDTHI_GET_PUBLIC_KEY_CMD
   */
  TDTHI_GENERAL_GRP_GET_PUBLIC_KEY_CMD,

  /**
   * The GetEventHistory command retrieves entries from the TDT Event History
   * in the SEC.
   *
   * @see TDTHI_GET_EVENT_HISTORY_CMD
   */
  TDTHI_GENERAL_GRP_GET_EVENT_HISTORY_CMD,

  /**
   * The ClearEventHistory command deletes all entries from the TDT Event
   * History in the SEC.
   *
   * @see TDTHI_CLEAR_EVENT_HISTORY_CMD
   */
  TDTHI_GENERAL_GRP_CLEAR_EVENT_HISTORY_CMD,

  /**
   * The SetSysTime command synchronizes the time between the FW with the
   * TDT server.
   *
   * @see TDTHI_SET_SYS_TIME_CMD
   */
  TDTHI_GENERAL_GRP_SET_SYS_TIME_CMD,

  /**
   * The Set Supend Mode command requests FW transition
   * into or out of Suspend Mode.
   */
  TDTHI_GENERAL_GRP_SET_SUSPEND_CMD                 = 8,

  /**
   * Bounday check.
   */
  TDTHI_GENERAL_GRP_MAX
} TDTHI_GENERAL_GRP_CMD;

//
// //////////////////////////////////////////////////////////////////////////////
//                      TDTHI 3G WWAN Commands                          //
////////////////////////////////////////////////////////////////////////////////
//
typedef enum _TDTHI_3G_NIC_GRP_CMD {
  /**
   * 3G NIC Init Command tells FW to intialize the NIC radio
   */
  TDTHI_3G_NIC_GRP_INIT_CMD                         = 3,

  /**
   * The 3G NIC Query Command asks FW about the status of the NIC radio
   */
  TDTHI_3G_NIC_GRP_QUERY_CMD,

  /**
   * Bounday check.
   */
  TDTHI_3G_NIC_GRP_MAX
} TDTHI_3G_NIC_GRP_CMD;

/**
 * The GetNonce command. Requests that the ME TDT Service generate and return
 * a nonce to be used in a subsequent TDTHI command. The nonce prevents
 * replay of a command.
 *
 * Sender:        ISV SW
 * Signing Key:   Any
 *
 * @dotfile TdtHiGetNonceCmd.dot "GetNonce command"
 *
 * @see TDTHI_GET_NONCE_RSP
 */
typedef TDTHI_BASIC_CMD TDTHI_GET_NONCE_CMD;

/**
 * The GetNonce response.
 *
 * @dotfile TdtHiGetNonceRsp.dot "GetNonce response"
 */
typedef struct _TDTHI_GET_NONCE_RSP {
  TDTHI_HEADER          Header;
  TDTHI_COMPLETION_CODE CompletionCode;
  TDT_NONCE             Nonce;
} TDTHI_GET_NONCE_RSP;

//
// //////////////////////////////////////////////////////////////////////////////
//              TDTHI NOTIFICATION Group Commands                             //
////////////////////////////////////////////////////////////////////////////////
//
typedef enum _TDTHI_NOTIFICATION_GRP_CMD {
  /**
   * A PBA sends a PBALogon notification message to notify the SEC TDT service
   * of a successful user logon.
   *
   * @see TDTHI_PBA_LOGON
   */
  TDTHI_NOTIFICATION_GRP_PBA_LOGON,

  /**
   * Bounday check.
   */
  TDTHI_NOTIFICATION_GRP_MAX
} TDTHI_NOTIFICATION_GRP_CMD;

/**
 * PBALogon notification. Notifies the SEC TDT service of a successful user logon.
 *
 * Sender:        PBA
 * Signing Key:   PBASK
 *
 * @dotfile TdtHiPbaLogonNotify.dot "PBALogon notification"
 *
 * @see TDTHI_GET_NONCE_RSP
 */
typedef TDTHI_BASIC_CMD TDTHI_PBA_LOGON_NOTIFY;

/**
 * The GetNonce response.
 *
 * @dotfile TdtHiPbaLogonAck.dot "PBALogon ack"
 */
typedef TDTHI_BASIC_RSP TDTHI_PBA_LOGON_ACK;

//
// /////////////////////////////////////////////////////////////////////////////
//              TDTHI DATA STORAGE Group Commands                            //
///////////////////////////////////////////////////////////////////////////////
//
typedef enum _TDTHI_DATA_STORE_GRP_CMD {
  /**
    * SetBlob stores data in the SEC TDT FDE/FLE ISV non-volatile storage area.
    *
    * @see #TDTHI_SET_BLOB_CMD.
    */
  TDTHI_DATA_STORE_GRP_SET_BLOB_CMD,

  /**
    * GetBlob returns the data stored in the SEC TDT FDE/FLE ISV non-volatile
    * storage area.
    *
    * @see TDTHI_GET_BLOB_CMD.
    */
  TDTHI_DATA_STORE_GRP_GET_BLOB_CMD,

  /**
    * Stores a vendor string the SEC's non-volatile storage area.
    *
    * @see TDTHI_SET_VENDOR_STRING_CMD
    */
  TDTHI_DATA_STORE_GRP_SET_VENDOR_STRING_CMD,

  /**
    * Retrieves the vendor string from the SEC's non-volatile storage area.
    *
    * @see TDTHI_GET_VENDOR_STRING_CMD
    */
  TDTHI_DATA_STORE_GRP_GET_VENDOR_STRING_CMD,

  /**
    * Bounday check.
    */
  TDTHI_DATA_STORE_GRP_MAX
} TDTHI_DATA_STORE_GRP_CMD;

/**
 * This enumeration defines the vendor string identifiers.
 */
typedef enum _TDT_VENDOR_STRING_ID {
  /**
    * Reserved.
    */
  TDT_VENDOR_STRING_ID_RSVD                         = 0,

  /**
    * User defined recovery passphrase
    */
  TDT_VENDOR_STRING_ID_RECOVERY_HELP,
  //
  // TDT_VENDOR_STRING_ID_RECOVERY_HELP,
  //

  /**
    * Server generated random token
    */
  TDT_CUSTOM_RECOVERY_ID_CONFIGURATIONS,

  /**
    * Boundary check.
    */
  TDT_VENDOR_STRING_ID_MAX
} TDT_VENDOR_STRING_ID;

/**
 * Maximum blob data size.
 */
#define TDT_VENDOR_STRING_LENGTH_MAX  256

/**
 * The SetVendorString command. Stores a vendor string the SEC's
 * non-volatile storage area.
 *
 * Sender:        ISV SW
 * Signing Key:   TSSK
 *
 * @see TDTHI_SET_VENDOR_STRING_RSP
 */
typedef struct _TDTHI_SET_VENDOR_STRING_CMD {
  TDTHI_HEADER  Header;
  UINT8         Id;
  UINT8         Reserved[3];
  TDT_BLOB      String;
} TDTHI_SET_VENDOR_STRING_CMD;

/**
 * The SetVendorString response.
 */
typedef TDTHI_BASIC_RSP TDTHI_SET_VENDOR_STRING_RSP;

/**
 * The GetVendorString command. Retrieves the vendor string data from the SEC's
 * non-volatile storage area.
 *
 * Sender:        ISV SW
 * Signing Key:   None
 *
 * @see TDTHI_GET_VENDOR_STRING_RSP
 */
typedef struct _TDTHI_GET_VENDOR_STRING_CMD {
  TDTHI_HEADER  Header;
  UINT8         Id;
} TDTHI_GET_VENDOR_STRING_CMD;

/**
 * The GetVendorString response.
 *
 * Sender:        ISV SW
 * Signing Key:   None
 *
 */
typedef struct _TDTHI_GET_VENDOR_STRING_RSP {
  TDTHI_HEADER          Header;
  TDTHI_COMPLETION_CODE CompletionCode;
  TDT_BLOB              String;
} TDTHI_GET_VENDOR_STRING_RSP;

/**
 * The SendAssertStolen Request
 *
 * Sender:        ISV SW
 * Signing Key:   None
 *
 */
typedef struct _TDTHI_ASSERT_STOLEN_CMD {
  TDTHI_HEADER  Header;
  UINT32        DelayTime;
} TDTHI_ASSERT_STOLEN_CMD;

/**
 * The SendAssertStolen response.
 */
typedef TDTHI_BASIC_RSP TDTHI_ASSERT_STOLEN_RSP;

/**
 * The Set Supend State Request Message
 *
 * Sender:        BIOS
 * Signing Key:   SRTK One Time Token
 *
 */
typedef struct _TDTHI_SET_SUSPEND_STATE_CMD {
  TDTHI_HEADER    Header;
  UINT32          TransitionState;  // 0=Exit, 1=Enter
  TDT_CREDENTIAL  Credential;
} TDTHI_SET_SUSPEND_STATE_CMD;

/**
 * The Set Supend State Response Message
 *
 * Sender:        FW
 * Signing Key:   None
 *
 */
typedef struct _TDTHI_SET_SUSPEND_STATE_RSP {
  TDTHI_HEADER  Header;
  UINT32        CompletionCode;
} TDTHI_SET_SUSPEND_STATE_RSP;

/**
 * The Initialize WWAN Recovery Request Message
 *
 * Sender:        BIOS
 * Signing Key:   None
 *
 */
typedef struct _TDTHI_INIT_WWAN_RECOV_CMD {
  TDTHI_HEADER  Header;
} TDTHI_INIT_WWAN_RECOV_CMD;

/**
 * The Initialize WWAN Recovery Response Message
 *
 * Sender:        FW
 * Signing Key:   None
 *
 */
typedef struct _TDTHI_INIT_WWAN_RECOV_RSP {
  TDTHI_HEADER  Header;
  UINT32        CompletionCode;
} TDTHI_INIT_WWAN_RECOV_RSP;

/**
 * The Initialize WWAN Status Request Message
 *
 * Sender:        BIOS
 * Signing Key:   None
 *
 */
typedef struct _TDTHI_WWAN_STATUS_CMD {
  TDTHI_HEADER  Header;
} TDTHI_WWAN_STATUS_CMD;

/**
 * The Initialize WWAN Status Response Message
 *
 * Sender:        FW
 * Signing Key:   None
 *
 */
typedef struct _TDTHI_WWAN_STATUS_RSP {
  TDTHI_HEADER  Header;
  UINT32        CompletionCode;
  UINT8         RadioStatus;
  UINT8         NetworkStatus;
} TDTHI_WWAN_STATUS_RSP;

////////////////////////////////////////////////////////////////////////////////
//              TDTHI SECURE BOOT Commands                           //
////////////////////////////////////////////////////////////////////////////////

/**
 * TDT Host Interface SecBoot group commands.  This is the group of subcommands
 * that can be processed in the TDTHI_CMD_GROUP_SECBOOT.
 */
typedef enum _TDTHI_SECBOOT_GRP_CMD
{
   /**
    * Validate Manifest
    */
   TDTHI_SECBOOT_GRP_VALIDATE_MANIFEST,
  /**
    * Get ARB Data from storage
    */
   TDTHI_SECBOOT_GRP_GET_ARB_DATA,
  /**
    * Set ARB Data to storage -- VALIDATION ONLY
    */
   TDTHI_SECBOOT_GRP_SET_ARB_DATA,
  /**
    * Unsigned OS image allowed?
    */
   TDTHI_SECBOOT_GRP_ALLOW_UNSIGNED_OS,
  /**
	* Get RPMB key
   */
   TDTHI_SECBOOT_GRP_GET_RPMB_KEY,
   /**
	 * Get SEC Reset reason.
     */
   TDTHI_SECBOOT_GRP_GET_RESET_REASON,
   /**
    * Boundary check.
    */
   TDTHI_SECBOOT_GRP_MAX
} TDTHI_SECBOOT_GRP_CMD;

typedef enum _TDTHI_SECBOOT_OPCODE
{
   TDTHI_SECBOOT_OPCODE_VALIDATE_KM_SM = 1,
   TDTHI_SECBOOT_OPCODE_VALIDATE_OSM,
   TDTHI_SECBOOT_OPCODE_VALIDATE_PDRM,
   TDTHI_SECBOOT_OPCODE_QUERY_RESET_REASON,
   TDTHI_SECBOOT_OPCODE_QUERY_RPMB_KEY
} TDTHI_SECBOOT_OPCODE;

typedef TDTHI_BASIC_CMD TDTHI_SECBOOT_CMD;
typedef TDTHI_BASIC_RSP TDTHI_SECBOOT_RSP;

typedef struct _TDTHI_SECBOOT_RSP_MSG
{
  TDTHI_SECBOOT_RSP       Msg;
} TDTHI_SECBOOT_RSP_MSG;

/**
 * Data structures for VALIDATE_MANIFEST Command
 **/
typedef struct _TDTHI_MANIFEST_DATA
{
  TDTHI_SECBOOT_OPCODE  OpCode;
  UINT8                 Padding[12];
  UINT8                 KeyManifest[1024];
  UINT8                 SBManifest[1024];
  UINT8                 Reserved[1008];
} TDTHI_MANIFEST_DATA;

typedef struct _TDTHI_MANIFEST_CMD_MSG
{
  TDTHI_SECBOOT_CMD       Msg;
  TDTHI_MANIFEST_DATA     Data;
} TDTHI_MANIFEST_CMD_MSG;

//
// for PlatTransRegVal
//
#define    LAST_EV_CAUSE_SUS_PWR_RESUME BIT15 // 15  The last event cause recorded was a resumption of Suspend Well power.
#define    LAST_EV_CAUSE_PMC_OTHER      BIT14 // 14  The last event cause recorded was an unspecirifed PMC cause.
#define    LAST_EV_CAUSE_PMC_ERR        BIT13 // 13  The last event cause recorded was a PMC Error.
#define    LAST_EV_CAUSE_SEC_WDG        BIT12 // 12  The last event cause recorded was a SeC Watchdog expiration.
#define    LAST_EV_CAUSE_OTHER_UNCOND_PWR_DN BIT11 // 11 - "1: The last event cause recorded was an Unconditional Power Down (Global Reset Type 8) not caused by a host request
#define    LAST_EV_CAUSE_AT_LOAD_TIMER  BIT10 // 10  An AT Load Timer expiration cause was recorded.
#define    LAST_EV_CAUSE_SEC_REQ        BIT9 //  9  The last event cause recorded was a SeC request
#define    LAST_EV_CAUSE_HOST_REQ       BIT8 //  8  The last event cause recorded was a host request.
#define    LAST_EV_GRST_ENTRY           BIT7 //  7  The last event recorded was a Global Reset (type 7) entry.
#define    LAST_EV_PWR_CYC_RST_ENTRY    BIT6 //  6  The last event recorded was a power cycle reset entry.
#define    LAST_EV_WRM_RST_ENTRY        BIT5 //  5  The last event recorded was a warm reset entry.
#define    LAST_EV_S3_ENTRY             BIT3 //  3  The last event recorded was an S3 entry.
#define    LAST_EV_S4_ENTRY             BIT2 //  2  The last event recorded was an S4 entry.
#define    LAST_EV_S5_ENTRY             BIT1 //  1  The last event recorded was an S5 entry.
#define    LAST_EV_G3_EXIT              BIT0 //  0  The last event recorded was a G3-exit.
//
// for PrstsRegVal
//
#define   PMC_WDT_STATUS                BIT15
//
// for PmcSecTransRegVal
//
#define   LAST_EV_CAUSE_HOST_STATE_TRANS BIT8 //  8  The last event cause recorded was a host state transition.      
#define   LAST_EV_SEC_RST_ENTRY          BIT5 //  5  The last event recorded was a SeC reset entry without power gating.      
#define   LAST_EV_SEC_POWER_OFF_ENTRY    BIT1 //  1  The last event recorded was a SeC power off 

typedef struct
{  
  UINT32   PlatTransRegVal;
  UINT32   PrstsRegVal;
  UINT32   PmcSecTransRegVal;
  UINT32   Reserved[1];
} TDTHI_PLAT_TRANS_DATA;

typedef struct _TDTHI_GET_PLAT_TRANS_RSP_MSG
{
  TDTHI_SECBOOT_RSP       Msg;
  TDTHI_PLAT_TRANS_DATA   Data;
} TDTHI_GET_PLAT_TRANS_RSP_MSG;

/**
 * Data structures for GET/SET ARB Commands
 **/
#define TDTHI_MAX_ARB_ENTRIES 16
#define SB_MAX_IMAGES TDTHI_MAX_ARB_ENTRIES

typedef struct
{
  UINT32    ARBTable[TDTHI_MAX_ARB_ENTRIES];
} TDTHI_ARB_DATA;

typedef struct _TDTHI_ARB_CMD_MSG
{
  TDTHI_SECBOOT_CMD       Msg;
  TDTHI_ARB_DATA          Data;
} TDTHI_ARB_CMD_MSG;



#pragma pack()

#endif // _TDT_HI_H
