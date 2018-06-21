/** @file
  Implement TPM2 Hierarchy related command.

Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <IndustryStandard/UefiTcgPlatform.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

#pragma pack(1)

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPMI_RH_CLEAR             AuthHandle;
  UINT32                    AuthorizationSize;
  TPMS_AUTH_SESSION_COMMAND AuthSession;
} TPM2_CLEAR_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER       Header;
  UINT32                     ParameterSize;
  TPMS_AUTH_SESSION_RESPONSE AuthSession;
} TPM2_CLEAR_RESPONSE;

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPMI_RH_CLEAR             AuthHandle;
  UINT32                    AuthorizationSize;
  TPMS_AUTH_SESSION_COMMAND AuthSession;
  TPMI_YES_NO               Disable;
} TPM2_CLEAR_CONTROL_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER       Header;
  UINT32                     ParameterSize;
  TPMS_AUTH_SESSION_RESPONSE AuthSession;
} TPM2_CLEAR_CONTROL_RESPONSE;

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPMI_RH_HIERARCHY_AUTH    AuthHandle;
  UINT32                    AuthorizationSize;
  TPMS_AUTH_SESSION_COMMAND AuthSession;
  TPM2B_AUTH                NewAuth;
} TPM2_HIERARCHY_CHANGE_AUTH_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER       Header;
  UINT32                     ParameterSize;
  TPMS_AUTH_SESSION_RESPONSE AuthSession;
} TPM2_HIERARCHY_CHANGE_AUTH_RESPONSE;

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPMI_RH_PLATFORM          AuthHandle;
  UINT32                    AuthorizationSize;
  TPMS_AUTH_SESSION_COMMAND AuthSession;
} TPM2_CHANGE_EPS_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER       Header;
  UINT32                     ParameterSize;
  TPMS_AUTH_SESSION_RESPONSE AuthSession;
} TPM2_CHANGE_EPS_RESPONSE;

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPMI_RH_PLATFORM          AuthHandle;
  UINT32                    AuthorizationSize;
  TPMS_AUTH_SESSION_COMMAND AuthSession;
} TPM2_CHANGE_PPS_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER       Header;
  UINT32                     ParameterSize;
  TPMS_AUTH_SESSION_RESPONSE AuthSession;
} TPM2_CHANGE_PPS_RESPONSE;

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPMI_RH_HIERARCHY         AuthHandle;
  UINT32                    AuthorizationSize;
  TPMS_AUTH_SESSION_COMMAND AuthSession;
  TPMI_RH_HIERARCHY         Hierarchy;
  TPMI_YES_NO               State;
} TPM2_HIERARCHY_CONTROL_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER       Header;
  UINT32                     ParameterSize;
  TPMS_AUTH_SESSION_RESPONSE AuthSession;
} TPM2_HIERARCHY_CONTROL_RESPONSE;

#pragma pack()

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
  )
{
  EFI_STATUS                        Status;
  TPM2_CLEAR_COMMAND                Cmd;
  TPM2_CLEAR_RESPONSE               Res;
  UINT32                            ResultBufSize;
  UINT32                            CmdSize;
  UINT32                            RespSize;
  UINT8                             *Buffer;
  UINT32                            SessionInfoSize;

  Cmd.Header.tag         = SwapBytes16(TPM_ST_SESSIONS);
  Cmd.Header.commandCode = SwapBytes32(TPM_CC_Clear);
  Cmd.AuthHandle         = SwapBytes32(AuthHandle);

  //
  // Add in Auth session
  //
  Buffer = (UINT8 *)&Cmd.AuthSession;

  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (AuthSession, Buffer);
  Buffer += SessionInfoSize;
  Cmd.AuthorizationSize = SwapBytes32(SessionInfoSize);

  CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
  Cmd.Header.paramSize   = SwapBytes32(CmdSize);

  ResultBufSize = sizeof(Res);
  Status = Tpm2SubmitCommand (CmdSize, (UINT8 *)&Cmd, &ResultBufSize, (UINT8 *)&Res);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "Clear: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = SwapBytes32(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "Clear: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (SwapBytes32(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "Clear: Response Code error! 0x%08x\r\n", SwapBytes32(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  //
  // Unmarshal the response
  //

  // None

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS                        Status;
  TPM2_CLEAR_CONTROL_COMMAND        Cmd;
  TPM2_CLEAR_CONTROL_RESPONSE       Res;
  UINT32                            ResultBufSize;
  UINT32                            CmdSize;
  UINT32                            RespSize;
  UINT8                             *Buffer;
  UINT32                            SessionInfoSize;

  SetMem(&Cmd, sizeof(Cmd), 0);
  Cmd.Header.tag         = SwapBytes16(TPM_ST_SESSIONS);
  Cmd.Header.commandCode = SwapBytes32(TPM_CC_ClearControl);
  Cmd.AuthHandle         = SwapBytes32(AuthHandle);

  //
  // Add in Auth session
  //
  Buffer = (UINT8 *)&Cmd.AuthSession;

  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (AuthSession, Buffer);
  Buffer += SessionInfoSize;
  Cmd.AuthorizationSize = SwapBytes32(SessionInfoSize);

  // disable
  *(UINT8 *)Buffer = Disable;
  Buffer += sizeof(UINT8);

  CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
  Cmd.Header.paramSize   = SwapBytes32(CmdSize);

  ResultBufSize = sizeof(Res);
  Status = Tpm2SubmitCommand (CmdSize, (UINT8 *)&Cmd, &ResultBufSize, (UINT8 *)&Res);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "ClearControl: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = SwapBytes32(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "ClearControl: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (SwapBytes32(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "ClearControl: Response Code error! 0x%08x\r\n", SwapBytes32(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  //
  // Unmarshal the response
  //

  // None

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS                           Status;
  TPM2_HIERARCHY_CHANGE_AUTH_COMMAND   Cmd;
  TPM2_HIERARCHY_CHANGE_AUTH_RESPONSE  Res;
  UINT32                               CmdSize;
  UINT32                               RespSize;
  UINT8                                *Buffer;
  UINT32                               SessionInfoSize;
  UINT8                                *ResultBuf;
  UINT32                               ResultBufSize;

  //
  // Construct command
  //
  Cmd.Header.tag          = SwapBytes16(TPM_ST_SESSIONS);
  Cmd.Header.paramSize    = SwapBytes32(sizeof(Cmd));
  Cmd.Header.commandCode  = SwapBytes32(TPM_CC_HierarchyChangeAuth);
  Cmd.AuthHandle          = SwapBytes32(AuthHandle);

  //
  // Add in Auth session
  //
  Buffer = (UINT8 *)&Cmd.AuthSession;

  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (AuthSession, Buffer);
  Buffer += SessionInfoSize;
  Cmd.AuthorizationSize = SwapBytes32(SessionInfoSize);

  // New Authorization size
  WriteUnaligned16 ((UINT16 *)Buffer, SwapBytes16(NewAuth->t.size));
  Buffer += sizeof(UINT16);

	// New Authorizeation
  CopyMem(Buffer, NewAuth->t.buffer, NewAuth->t.size);
  Buffer += NewAuth->t.size;

  CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
  Cmd.Header.paramSize = SwapBytes32(CmdSize);

  ResultBuf     = (UINT8 *) &Res;
  ResultBufSize = sizeof(Res);

  //
  // Call the TPM
  //
  Status = Tpm2SubmitCommand (
             CmdSize, 
             (UINT8 *)&Cmd, 
             &ResultBufSize,
             ResultBuf
             );

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "HierarchyChangeAuth: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = SwapBytes32(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "HierarchyChangeAuth: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if ((SwapBytes32(Res.Header.responseCode) != TPM_RC_SUCCESS) && 
      (SwapBytes32(Res.Header.responseCode) != TPM_RC_FAILURE)) {
    DEBUG((EFI_D_ERROR,"HierarchyChangeAuth: Response Code error! 0x%08x\r\n", SwapBytes32(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS                Status;
  TPM2_CHANGE_EPS_COMMAND   Cmd;
  TPM2_CHANGE_EPS_RESPONSE  Res;
  UINT32                    CmdSize;
  UINT32                    RespSize;
  UINT8                     *Buffer;
  UINT32                    SessionInfoSize;
  UINT8                     *ResultBuf;
  UINT32                    ResultBufSize;

  //
  // Construct command
  //
  Cmd.Header.tag          = SwapBytes16(TPM_ST_SESSIONS);
  Cmd.Header.paramSize    = SwapBytes32(sizeof(Cmd));
  Cmd.Header.commandCode  = SwapBytes32(TPM_CC_ChangeEPS);
  Cmd.AuthHandle          = SwapBytes32(AuthHandle);

  //
  // Add in Auth session
  //
  Buffer = (UINT8 *)&Cmd.AuthSession;

  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (AuthSession, Buffer);
  Buffer += SessionInfoSize;
  Cmd.AuthorizationSize = SwapBytes32(SessionInfoSize);

  CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
  Cmd.Header.paramSize = SwapBytes32(CmdSize);

  ResultBuf     = (UINT8 *) &Res;
  ResultBufSize = sizeof(Res);

  //
  // Call the TPM
  //
  Status = Tpm2SubmitCommand (
             CmdSize, 
             (UINT8 *)&Cmd, 
             &ResultBufSize,
             ResultBuf
             );

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "ChangeEPS: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = SwapBytes32(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "ChangeEPS: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (SwapBytes32(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG((EFI_D_ERROR,"ChangeEPS: Response Code error! 0x%08x\r\n", SwapBytes32(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS                Status;
  TPM2_CHANGE_PPS_COMMAND   Cmd;
  TPM2_CHANGE_PPS_RESPONSE  Res;
  UINT32                    CmdSize;
  UINT32                    RespSize;
  UINT8                     *Buffer;
  UINT32                    SessionInfoSize;
  UINT8                     *ResultBuf;
  UINT32                    ResultBufSize;

  //
  // Construct command
  //
  Cmd.Header.tag          = SwapBytes16(TPM_ST_SESSIONS);
  Cmd.Header.paramSize    = SwapBytes32(sizeof(Cmd));
  Cmd.Header.commandCode  = SwapBytes32(TPM_CC_ChangePPS);
  Cmd.AuthHandle          = SwapBytes32(AuthHandle);

  //
  // Add in Auth session
  //
  Buffer = (UINT8 *)&Cmd.AuthSession;

  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (AuthSession, Buffer);
  Buffer += SessionInfoSize;
  Cmd.AuthorizationSize = SwapBytes32(SessionInfoSize);

  CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
  Cmd.Header.paramSize = SwapBytes32(CmdSize);

  ResultBuf     = (UINT8 *) &Res;
  ResultBufSize = sizeof(Res);

  //
  // Call the TPM
  //
  Status = Tpm2SubmitCommand (
             CmdSize, 
             (UINT8 *)&Cmd, 
             &ResultBufSize,
             ResultBuf
             );

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "ChangePPS: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = SwapBytes32(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "ChangePPS: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (SwapBytes32(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG((EFI_D_ERROR,"ChangePPS: Response Code error! 0x%08x\r\n", SwapBytes32(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS                       Status;
  TPM2_HIERARCHY_CONTROL_COMMAND   Cmd;
  TPM2_HIERARCHY_CONTROL_RESPONSE  Res;
  UINT32                           CmdSize;
  UINT32                           RespSize;
  UINT8                            *Buffer;
  UINT32                           SessionInfoSize;
  UINT8                            *ResultBuf;
  UINT32                           ResultBufSize;

  //
  // Construct command
  //
  SetMem(&Cmd, sizeof(Cmd), 0);
  Cmd.Header.tag          = SwapBytes16(TPM_ST_SESSIONS);
  Cmd.Header.paramSize    = SwapBytes32(sizeof(Cmd));
  Cmd.Header.commandCode  = SwapBytes32(TPM_CC_HierarchyControl);
  Cmd.AuthHandle          = SwapBytes32(AuthHandle);

  //
  // Add in Auth session
  //
  Buffer = (UINT8 *)&Cmd.AuthSession;

  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (AuthSession, Buffer);
  Buffer += SessionInfoSize;
  Cmd.AuthorizationSize = SwapBytes32(SessionInfoSize);

  WriteUnaligned32 ((UINT32 *)Buffer, SwapBytes32(Hierarchy));
  Buffer += sizeof(UINT32);

  *(UINT8 *)Buffer = State;
  Buffer += sizeof(UINT8);

  CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
  Cmd.Header.paramSize = SwapBytes32(CmdSize);

  ResultBuf     = (UINT8 *) &Res;
  ResultBufSize = sizeof(Res);

  //
  // Call the TPM
  //
  Status = Tpm2SubmitCommand (
             CmdSize, 
             (UINT8 *)&Cmd, 
             &ResultBufSize,
             ResultBuf
             );

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "HierarchyControl: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = SwapBytes32(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "HierarchyControl: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (SwapBytes32(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG((EFI_D_ERROR,"HierarchyControl: Response Code error! 0x%08x\r\n", SwapBytes32(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}
