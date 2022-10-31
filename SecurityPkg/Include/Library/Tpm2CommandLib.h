/** @file
  This library is used by other modules to send TPM2 command.

Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _TPM2_COMMAND_LIB_H_
#define _TPM2_COMMAND_LIB_H_

#include <IndustryStandard/Tpm20.h>

/**
  This command starts a hash or an Event sequence.
  If hashAlg is an implemented hash, then a hash sequence is started.
  If hashAlg is TPM_ALG_NULL, then an Event sequence is started.

  @param[in]  HashAlg           The hash algorithm to use for the hash sequence
                                An Event sequence starts if this is TPM_ALG_NULL.
  @param[out] SequenceHandle    A handle to reference the sequence
 
  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2HashSequenceStart (
  IN TPMI_ALG_HASH   HashAlg,
  OUT TPMI_DH_OBJECT *SequenceHandle
  );

/**
  This command is used to add data to a hash or HMAC sequence.
  The amount of data in buffer may be any size up to the limits of the TPM.
  NOTE: In all TPM, a buffer size of 1,024 octets is allowed.

  @param[in] SequenceHandle    Handle for the sequence object
  @param[in] Buffer            Data to be added to hash
 
  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2SequenceUpdate (
  IN TPMI_DH_OBJECT   SequenceHandle,
  IN TPM2B_MAX_BUFFER *Buffer
  );

/**
  This command adds the last part of data, if any, to an Event sequence and returns the result in a digest list.
  If pcrHandle references a PCR and not TPM_RH_NULL, then the returned digest list is processed in
  the same manner as the digest list input parameter to TPM2_PCR_Extend() with the pcrHandle in each
  bank extended with the associated digest value.

  @param[in]  PcrHandle         PCR to be extended with the Event data
  @param[in]  SequenceHandle    Authorization for the sequence
  @param[in]  Buffer            Data to be added to the Event
  @param[out] Results           List of digests computed for the PCR
 
  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2EventSequenceComplete (
  IN TPMI_DH_PCR         PcrHandle,
  IN TPMI_DH_OBJECT      SequenceHandle,
  IN TPM2B_MAX_BUFFER    *Buffer,
  OUT TPML_DIGEST_VALUES *Results
  );

/**
  This command adds the last part of data, if any, to a hash/HMAC sequence and returns the result.

  @param[in]  SequenceHandle    Authorization for the sequence
  @param[in]  Buffer            Data to be added to the hash/HMAC
  @param[out] Result            The returned HMAC or digest in a sized buffer
 
  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2SequenceComplete (
  IN TPMI_DH_OBJECT      SequenceHandle,
  IN TPM2B_MAX_BUFFER    *Buffer,
  OUT TPM2B_DIGEST       *Result
  );

/**
  Send Startup command to TPM2.

  @param[in] StartupType           TPM_SU_CLEAR or TPM_SU_STATE

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2Startup (
  IN      TPM_SU             StartupType
  );

/**
  Send Shutdown command to TPM2.

  @param[in] ShutdownType           TPM_SU_CLEAR or TPM_SU_STATE.

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2Shutdown (
  IN      TPM_SU             ShutdownType
  );

/**
  This command causes the TPM to perform a test of its capabilities.
  If the fullTest is YES, the TPM will test all functions.
  If fullTest = NO, the TPM will only test those functions that have not previously been tested.

  @param[in] FullTest    YES if full test to be performed
                         NO if only test of untested functions required

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2SelfTest (
  IN TPMI_YES_NO          FullTest
  );

/**
  This command removes all TPM context associated with a specific Owner.

  @param[in] AuthHandle        TPM_RH_LOCKOUT or TPM_RH_PLATFORM+{PP}
  @param[in] AuthSession       Auth Session context
 
  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2Clear (
  IN TPMI_RH_CLEAR             AuthHandle,
  IN TPMS_AUTH_SESSION_COMMAND *AuthSession OPTIONAL
  );

/**
  Disables and enables the execution of TPM2_Clear().

  @param[in] AuthHandle        TPM_RH_LOCKOUT or TPM_RH_PLATFORM+{PP}
  @param[in] AuthSession       Auth Session context
  @param[in] Disable           YES if the disableOwnerClear flag is to be SET,
                               NO if the flag is to be CLEAR.

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2ClearControl (
  IN TPMI_RH_CLEAR             AuthHandle,
  IN TPMS_AUTH_SESSION_COMMAND *AuthSession, OPTIONAL
  IN TPMI_YES_NO               Disable
  );

/**
  This command allows the authorization secret for a hierarchy or lockout to be changed using the current
  authorization value as the command authorization.

  @param[in] AuthHandle        TPM_RH_LOCKOUT, TPM_RH_ENDORSEMENT, TPM_RH_OWNER or TPM_RH_PLATFORM+{PP}
  @param[in] AuthSession       Auth Session context
  @param[in] NewAuth           New authorization secret

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2HierarchyChangeAuth (
  IN TPMI_RH_HIERARCHY_AUTH    AuthHandle,
  IN TPMS_AUTH_SESSION_COMMAND *AuthSession,
  IN TPM2B_AUTH                *NewAuth
  );

/**
  This replaces the current EPS with a value from the RNG and sets the Endorsement hierarchy controls to
  their default initialization values.

  @param[in] AuthHandle        TPM_RH_PLATFORM+{PP}
  @param[in] AuthSession       Auth Session context

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2ChangeEPS (
  IN TPMI_RH_PLATFORM          AuthHandle,
  IN TPMS_AUTH_SESSION_COMMAND *AuthSession
  );

/**
  This replaces the current PPS with a value from the RNG and sets platformPolicy to the default
  initialization value (the Empty Buffer).

  @param[in] AuthHandle        TPM_RH_PLATFORM+{PP}
  @param[in] AuthSession       Auth Session context

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2ChangePPS (
  IN TPMI_RH_PLATFORM          AuthHandle,
  IN TPMS_AUTH_SESSION_COMMAND *AuthSession
  );

/**
  This command enables and disables use of a hierarchy.

  @param[in] AuthHandle        TPM_RH_ENDORSEMENT, TPM_RH_OWNER or TPM_RH_PLATFORM+{PP}
  @param[in] AuthSession       Auth Session context
  @param[in] Hierarchy         Hierarchy of the enable being modified
  @param[in] State             YES if the enable should be SET,
                               NO if the enable should be CLEAR

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2HierarchyControl (
  IN TPMI_RH_HIERARCHY         AuthHandle,
  IN TPMS_AUTH_SESSION_COMMAND *AuthSession,
  IN TPMI_RH_HIERARCHY         Hierarchy,
  IN TPMI_YES_NO               State
  );

/**
  This command cancels the effect of a TPM lockout due to a number of successive authorization failures.
  If this command is properly authorized, the lockout counter is set to zero.

  @param[in]  LockHandle            LockHandle
  @param[in]  AuthSession           Auth Session context

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2DictionaryAttackLockReset (
  IN  TPMI_RH_LOCKOUT           LockHandle,
  IN  TPMS_AUTH_SESSION_COMMAND *AuthSession
  );

/**
  This command cancels the effect of a TPM lockout due to a number of successive authorization failures.
  If this command is properly authorized, the lockout counter is set to zero.

  @param[in]  LockHandle            LockHandle
  @param[in]  AuthSession           Auth Session context
  @param[in]  NewMaxTries           Count of authorization failures before the lockout is imposed
  @param[in]  NewRecoveryTime       Time in seconds before the authorization failure count is automatically decremented
  @param[in]  LockoutRecovery       Time in seconds after a lockoutAuth failure before use of lockoutAuth is allowed

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2DictionaryAttackParameters (
  IN  TPMI_RH_LOCKOUT           LockHandle,
  IN  TPMS_AUTH_SESSION_COMMAND *AuthSession,
  IN  UINT32                    NewMaxTries,
  IN  UINT32                    NewRecoveryTime,
  IN  UINT32                    LockoutRecovery
  );

/**
  This command is used to read the public area and Name of an NV Index.

  @param[in]  NvIndex            The NV Index.
  @param[out] NvPublic           The public area of the index.
  @param[out] NvName             The Name of the nvIndex.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2NvReadPublic (
  IN      TPMI_RH_NV_INDEX          NvIndex,
  OUT     TPM2B_NV_PUBLIC           *NvPublic,
  OUT     TPM2B_NAME                *NvName
  );

/**
  This command is used to cause an update to the indicated PCR.
  The digests parameter contains one or more tagged digest value identified by an algorithm ID.
  For each digest, the PCR associated with pcrHandle is Extended into the bank identified by the tag (hashAlg).

  @param[in] PcrHandle   Handle of the PCR
  @param[in] Digests     List of tagged digest values to be extended

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2PcrExtend (
  IN      TPMI_DH_PCR               PcrHandle,
  IN      TPML_DIGEST_VALUES        *Digests
  );

/**
  This command is used to cause an update to the indicated PCR.
  The data in eventData is hashed using the hash algorithm associated with each bank in which the
  indicated PCR has been allocated. After the data is hashed, the digests list is returned. If the pcrHandle
  references an implemented PCR and not TPM_ALG_NULL, digests list is processed as in
  TPM2_PCR_Extend().
  A TPM shall support an Event.size of zero through 1,024 inclusive.

  @param[in]  PcrHandle   Handle of the PCR
  @param[in]  EventData   Event data in sized buffer
  @param[out] Digests     List of digest

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2PcrEvent (
  IN      TPMI_DH_PCR               PcrHandle,
  IN      TPM2B_EVENT               *EventData,
     OUT  TPML_DIGEST_VALUES        *Digests
  );

/**
  This command returns the values of all PCR specified in pcrSelect.

  @param[in]  PcrSelectionIn     The selection of PCR to read.
  @param[out] PcrUpdateCounter   The current value of the PCR update counter.
  @param[out] PcrSelectionOut    The PCR in the returned list.
  @param[out] PcrValues          The contents of the PCR indicated in pcrSelect.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2PcrRead (
  IN      TPML_PCR_SELECTION        *PcrSelectionIn,
     OUT  UINT32                    *PcrUpdateCounter,
     OUT  TPML_PCR_SELECTION        *PcrSelectionOut,
     OUT  TPML_DIGEST               *PcrValues
  );

/**
  This command returns various information regarding the TPM and its current state.

  The capability parameter determines the category of data returned. The property parameter 
  selects the first value of the selected category to be returned. If there is no property 
  that corresponds to the value of property, the next higher value is returned, if it exists.
  The moreData parameter will have a value of YES if there are more values of the requested 
  type that were not returned.
  If no next capability exists, the TPM will return a zero-length list and moreData will have 
  a value of NO.

  NOTE: 
  To simplify this function, leave returned CapabilityData for caller to unpack since there are 
  many capability categories and only few categories will be used in firmware. It means the caller
  need swap the byte order for the feilds in CapabilityData.

  @param[in]  Capability         Group selection; determines the format of the response.
  @param[in]  Property           Further definition of information. 
  @param[in]  PropertyCount      Number of properties of the indicated type to return.
  @param[out] MoreData           Flag to indicate if there are more values of this type.
  @param[out] CapabilityData     The capability data.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapability (
  IN      TPM_CAP                   Capability,
  IN      UINT32                    Property,
  IN      UINT32                    PropertyCount,
  OUT     TPMI_YES_NO               *MoreData,
  OUT     TPMS_CAPABILITY_DATA      *CapabilityData
  );

/**
  This command returns the information of TPM Family.

  This function parse the value got from TPM2_GetCapability and return the Family.

  @param[out] Family             The Family of TPM. (a 4-octet character string)
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityFamily (
  OUT     CHAR8                     *Family
  );

/**
  This command returns the information of TPM manufacture ID.

  This function parse the value got from TPM2_GetCapability and return the TPM manufacture ID.

  @param[out] ManufactureId      The manufacture ID of TPM.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityManufactureID (
  OUT     UINT32                    *ManufactureId
  );

/**
  This command returns the information of TPM FirmwareVersion.

  This function parse the value got from TPM2_GetCapability and return the TPM FirmwareVersion.

  @param[out] FirmwareVersion1   The FirmwareVersion1.
  @param[out] FirmwareVersion2   The FirmwareVersion2.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityFirmwareVersion (
  OUT     UINT32                    *FirmwareVersion1,
  OUT     UINT32                    *FirmwareVersion2
  );

/**
  This command returns the information of the maximum value for commandSize and responseSize in a command.

  This function parse the value got from TPM2_GetCapability and return the max command size and response size

  @param[out] MaxCommandSize     The maximum value for commandSize in a command.
  @param[out] MaxResponseSize    The maximum value for responseSize in a command.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityMaxCommandResponseSize (
  OUT UINT32                    *MaxCommandSize,
  OUT UINT32                    *MaxResponseSize
  );

/**
  This command returns Returns a list of TPMS_ALG_PROPERTIES. Each entry is an
  algorithm ID and a set of properties of the algorithm. 

  This function parse the value got from TPM2_GetCapability and return the list.

  @param[out] AlgList      List of algorithm.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilitySupportedAlg (
  OUT TPML_ALG_PROPERTY      *AlgList
  );

/**
  This command returns the information of TPM LockoutCounter.

  This function parse the value got from TPM2_GetCapability and return the LockoutCounter.

  @param[out] LockoutCounter     The LockoutCounter of TPM.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityLockoutCounter (
  OUT     UINT32                    *LockoutCounter
  );

/**
  This command returns the information of TPM LockoutInterval.

  This function parse the value got from TPM2_GetCapability and return the LockoutInterval.

  @param[out] LockoutInterval    The LockoutInterval of TPM.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityLockoutInterval (
  OUT     UINT32                    *LockoutInterval
  );

/**
  This command returns the information of TPM InputBufferSize.

  This function parse the value got from TPM2_GetCapability and return the InputBufferSize.

  @param[out] InputBufferSize    The InputBufferSize of TPM.
                                 the maximum size of a parameter (typically, a TPM2B_MAX_BUFFER)
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityInputBufferSize (
  OUT     UINT32                    *InputBufferSize
  );

//
// Help function
//

/**
  Copy AuthSessionIn to TPM2 command buffer.

  @param [in]  AuthSessionIn   Input AuthSession data
  @param [out] AuthSessionOut  Output AuthSession data in TPM2 command buffer

  @return AuthSession size
**/
UINT32
EFIAPI
CopyAuthSessionCommand (
  IN      TPMS_AUTH_SESSION_COMMAND *AuthSessionIn, OPTIONAL
  OUT     UINT8                     *AuthSessionOut
  );

/**
  Copy AuthSessionIn from TPM2 response buffer.

  @param [in]  AuthSessionIn   Input AuthSession data in TPM2 response buffer
  @param [out] AuthSessionOut  Output AuthSession data

  @return AuthSession size
**/
UINT32
EFIAPI
CopyAuthSessionResponse (
  IN      UINT8                      *AuthSessionIn,
  OUT     TPMS_AUTH_SESSION_RESPONSE *AuthSessionOut OPTIONAL
  );

/**
  Return size of digest.

  @param[in] HashAlgo  Hash algorithm

  @return size of digest
**/
UINT16
EFIAPI
GetHashSizeFromAlgo (
  IN TPMI_ALG_HASH    HashAlgo
  );

#endif
