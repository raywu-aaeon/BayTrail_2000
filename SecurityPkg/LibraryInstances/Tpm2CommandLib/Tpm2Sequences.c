/** @file
  Implement TPM2 Sequences related command.

Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/DebugLib.h>
#include <Library/Tpm2DeviceLib.h>

#pragma pack(1)

typedef struct {
  TPM2_COMMAND_HEADER  Header;
  TPM2B_AUTH           Auth;
  TPMI_ALG_HASH        HashAlg;
} TPM2_HASH_SEQUENCE_START_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER  Header;
  TPMI_DH_OBJECT        SequenceHandle;
} TPM2_HASH_SEQUENCE_START_RESPONSE;

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPMI_DH_OBJECT            SequenceHandle;
  UINT32                    AuthorizationSize;
  TPMS_AUTH_SESSION_COMMAND AuthSessionSeq;
  TPM2B_MAX_BUFFER          Buffer;
} TPM2_SEQUENCE_UPDATE_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER       Header;
  UINT32                     ParameterSize;
  TPMS_AUTH_SESSION_RESPONSE AuthSessionSeq;
} TPM2_SEQUENCE_UPDATE_RESPONSE;

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPMI_DH_PCR               PcrHandle;
  TPMI_DH_OBJECT            SequenceHandle;
  UINT32                    AuthorizationSize;
  TPMS_AUTH_SESSION_COMMAND AuthSessionPcr;
  TPMS_AUTH_SESSION_COMMAND AuthSessionSeq;
  TPM2B_MAX_BUFFER          Buffer;
} TPM2_EVENT_SEQUENCE_COMPLETE_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER       Header;
  UINT32                     ParameterSize;
  TPML_DIGEST_VALUES         Results;
  TPMS_AUTH_SESSION_RESPONSE AuthSessionPcr;
  TPMS_AUTH_SESSION_RESPONSE AuthSessionSeq;
} TPM2_EVENT_SEQUENCE_COMPLETE_RESPONSE;

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPMI_DH_OBJECT            SequenceHandle;
  UINT32                    AuthorizationSize;
  TPMS_AUTH_SESSION_COMMAND AuthSessionSeq;
  TPM2B_MAX_BUFFER          Buffer;
  TPMI_RH_HIERARCHY         Hierarchy;
} TPM2_SEQUENCE_COMPLETE_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER       Header;
  UINT32                     ParameterSize;
  TPM2B_DIGEST               Digest;
  TPMS_AUTH_SESSION_RESPONSE AuthSessionSeq;
} TPM2_SEQUENCE_COMPLETE_RESPONSE;

#pragma pack()

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
  )
{
  EFI_STATUS                        Status;
  TPM2_HASH_SEQUENCE_START_COMMAND  Cmd;
  TPM2_HASH_SEQUENCE_START_RESPONSE Res;
  UINT32                            CmdSize;
  UINT32                            RespSize;
  UINT8                             *Buffer;
  UINT32                            ResultBufSize;

  ZeroMem(&Cmd, sizeof(Cmd));

  //
  // Construct command
  //
  Cmd.Header.tag         = SwapBytes16(TPM_ST_NO_SESSIONS);
  Cmd.Header.commandCode = SwapBytes32(TPM_CC_HashSequenceStart);

  Buffer = (UINT8 *)&Cmd.Auth;

  // auth = nullAuth
  WriteUnaligned16 ((UINT16 *)Buffer, SwapBytes16(0));
  Buffer += sizeof(UINT16);

  // hashAlg
  WriteUnaligned16 ((UINT16 *)Buffer, SwapBytes16(HashAlg));
  Buffer += sizeof(UINT16);

  CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
  Cmd.Header.paramSize = SwapBytes32(CmdSize);

  //
  // Call the TPM
  //
  ResultBufSize = sizeof(Res);
  Status = Tpm2SubmitCommand (CmdSize, (UINT8 *)&Cmd, &ResultBufSize, (UINT8 *)&Res);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "HashSequenceStart: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = SwapBytes32(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "HashSequenceStart: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (SwapBytes32(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "HashSequenceStart: Response Code error! 0x%08x\r\n", SwapBytes32(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  //
  // Unmarshal the response
  //

  // sequenceHandle
  *SequenceHandle = SwapBytes32(Res.SequenceHandle);

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS                    Status;
  TPM2_SEQUENCE_UPDATE_COMMAND  Cmd;
  TPM2_SEQUENCE_UPDATE_RESPONSE Res;
  UINT32                        CmdSize;
  UINT32                        RespSize;
  UINT8                         *BufferPtr;
  UINT32                        SessionInfoSize;
  UINT32                        ResultBufSize;

  ZeroMem(&Cmd, sizeof(Cmd));

  //
  // Construct command
  //
  Cmd.Header.tag = SwapBytes16(TPM_ST_SESSIONS);
  Cmd.Header.commandCode = SwapBytes32(TPM_CC_SequenceUpdate);
  Cmd.SequenceHandle = SwapBytes32(SequenceHandle);

  //
  // Add in Auth session
  //
  BufferPtr = (UINT8 *)&Cmd.AuthSessionSeq;

  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (NULL, BufferPtr);
  BufferPtr += SessionInfoSize;
  Cmd.AuthorizationSize = SwapBytes32(SessionInfoSize);

  // buffer.size
  WriteUnaligned16 ((UINT16 *)BufferPtr, SwapBytes16(Buffer->t.size));
  BufferPtr += sizeof(UINT16);

  CopyMem(BufferPtr, &Buffer->t.buffer, Buffer->t.size);
  BufferPtr += Buffer->t.size;

  CmdSize = (UINT32)(BufferPtr - (UINT8 *)&Cmd);
  Cmd.Header.paramSize = SwapBytes32(CmdSize);

  //
  // Call the TPM
  //
  ResultBufSize = sizeof(Res);
  Status = Tpm2SubmitCommand (CmdSize, (UINT8 *)&Cmd,&ResultBufSize, (UINT8 *)&Res);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "SequenceUpdate: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = SwapBytes32(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "SequenceUpdate: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (SwapBytes32(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "SequenceUpdate: Response Code error! 0x%08x\r\n", SwapBytes32(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  //
  // Unmarshal the response
  //

  // None

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS                            Status;
  TPM2_EVENT_SEQUENCE_COMPLETE_COMMAND  Cmd;
  TPM2_EVENT_SEQUENCE_COMPLETE_RESPONSE Res;
  UINT32                                CmdSize;
  UINT32                                RespSize;
  UINT8                                 *BufferPtr;
  UINT32                                SessionInfoSize;
  UINT32                                SessionInfoSize2;
  UINT32                                Index;
  UINT32                                ResultBufSize;
  UINT16                                DigestSize;

  ZeroMem(&Cmd, sizeof(Cmd));

  //
  // Construct command
  //
  Cmd.Header.tag = SwapBytes16(TPM_ST_SESSIONS);
  Cmd.Header.commandCode = SwapBytes32(TPM_CC_EventSequenceComplete);
  Cmd.PcrHandle = SwapBytes32(PcrHandle);
  Cmd.SequenceHandle = SwapBytes32(SequenceHandle);

  //
  // Add in pcrHandle Auth session
  //
  BufferPtr = (UINT8 *)&Cmd.AuthSessionPcr;

  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (NULL, BufferPtr);
  BufferPtr += SessionInfoSize;

  // sessionInfoSize
  SessionInfoSize2 = CopyAuthSessionCommand (NULL, BufferPtr);
  BufferPtr += SessionInfoSize2;
  Cmd.AuthorizationSize = SwapBytes32(SessionInfoSize + SessionInfoSize2);

  // buffer.size
  WriteUnaligned16 ((UINT16 *)BufferPtr, SwapBytes16(Buffer->t.size));
  BufferPtr += sizeof(UINT16);

  CopyMem(BufferPtr, &Buffer->t.buffer[0], Buffer->t.size);
  BufferPtr += Buffer->t.size;

  CmdSize = (UINT32)(BufferPtr - (UINT8 *)&Cmd);
  Cmd.Header.paramSize = SwapBytes32(CmdSize);

  //
  // Call the TPM
  //
  ResultBufSize = sizeof(Res);
  Status = Tpm2SubmitCommand (CmdSize, (UINT8 *)&Cmd, &ResultBufSize, (UINT8 *)&Res);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "EventSequenceComplete: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = SwapBytes32(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "EventSequenceComplete: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (SwapBytes32(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "EventSequenceComplete: Response Code error! 0x%08x\r\n", SwapBytes32(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  //
  // Unmarshal the response
  //

  BufferPtr = (UINT8 *)&Res.Results;

  // count
  Results->count = SwapBytes32(ReadUnaligned32 ((UINT32 *)BufferPtr));
  BufferPtr += sizeof(UINT32);

  for (Index = 0; Index < Results->count; Index++) {
    Results->digests[Index].hashAlg = SwapBytes16(ReadUnaligned16 ((UINT16 *)BufferPtr));
    BufferPtr += sizeof(UINT16);

    DigestSize = GetHashSizeFromAlgo (Results->digests[Index].hashAlg);
    if (DigestSize == 0) {
      DEBUG ((EFI_D_ERROR, "EventSequenceComplete: Unknown hash algorithm %d\r\n", Results->digests[Index].hashAlg));
      return EFI_DEVICE_ERROR;
    }
    CopyMem(
      &Results->digests[Index].digest,
      BufferPtr,
      DigestSize
      );
    BufferPtr += DigestSize;
  }

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS                            Status;
  TPM2_SEQUENCE_COMPLETE_COMMAND        Cmd;
  TPM2_SEQUENCE_COMPLETE_RESPONSE       Res;
  UINT32                                CmdSize;
  UINT32                                RespSize;
  UINT8                                 *BufferPtr;
  UINT32                                SessionInfoSize;
  UINT32                                ResultBufSize;

  ZeroMem(&Cmd, sizeof(Cmd));

  //
  // Construct command
  //
  Cmd.Header.tag = SwapBytes16(TPM_ST_SESSIONS);
  Cmd.Header.commandCode = SwapBytes32(TPM_CC_SequenceComplete);
  Cmd.SequenceHandle = SwapBytes32(SequenceHandle);

  //
  // Add in Auth session
  //
  BufferPtr = (UINT8 *)&Cmd.AuthSessionSeq;

  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (NULL, BufferPtr);
  BufferPtr += SessionInfoSize;
  Cmd.AuthorizationSize = SwapBytes32(SessionInfoSize);

  // buffer.size
  WriteUnaligned16 ((UINT16 *)BufferPtr, SwapBytes16(Buffer->t.size));
  BufferPtr += sizeof(UINT16);

  CopyMem(BufferPtr, &Buffer->t.buffer[0], Buffer->t.size);
  BufferPtr += Buffer->t.size;

  // Hierarchy
  WriteUnaligned32 ((UINT32 *)BufferPtr, SwapBytes32 (TPM_RH_NULL));
  BufferPtr += sizeof (UINT32);

  CmdSize = (UINT32)(BufferPtr - (UINT8 *)&Cmd);
  Cmd.Header.paramSize = SwapBytes32(CmdSize);

  //
  // Call the TPM
  //
  ResultBufSize = sizeof(Res);
  Status = Tpm2SubmitCommand (CmdSize, (UINT8 *)&Cmd, &ResultBufSize, (UINT8 *)&Res);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "SequenceComplete: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = SwapBytes32(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "SequenceComplete: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (SwapBytes32(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "SequenceComplete: Response Code error! 0x%08x\r\n", SwapBytes32(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  //
  // Unmarshal the response
  //

  BufferPtr = (UINT8 *)&Res.Digest;

  // digestSize
  Result->t.size = SwapBytes16(ReadUnaligned16 ((UINT16 *)BufferPtr));
  BufferPtr += sizeof(UINT16);

  CopyMem(
    Result->t.buffer,
    BufferPtr,
    Result->t.size
    );

  return EFI_SUCCESS;
}
