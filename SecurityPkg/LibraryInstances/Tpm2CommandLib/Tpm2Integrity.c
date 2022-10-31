/** @file
  Implement TPM2 Integrity related command.

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
  TPMI_DH_PCR               PcrHandle;
  UINT32                    AuthorizationSize;
  TPMS_AUTH_SESSION_COMMAND AuthSessionPcr;
  TPML_DIGEST_VALUES        DigestValues;
} TPM2_PCR_EXTEND_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER       Header;
  UINT32                     ParameterSize;
  TPMS_AUTH_SESSION_RESPONSE AuthSessionPcr;
} TPM2_PCR_EXTEND_RESPONSE;

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPMI_DH_PCR               PcrHandle;
  UINT32                    AuthorizationSize;
  TPMS_AUTH_SESSION_COMMAND AuthSessionPcr;
  TPM2B_EVENT               EventData;
} TPM2_PCR_EVENT_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER       Header;
  UINT32                     ParameterSize;
  TPML_DIGEST_VALUES         Digests;
  TPMS_AUTH_SESSION_RESPONSE AuthSessionPcr;
} TPM2_PCR_EVENT_RESPONSE;

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPML_PCR_SELECTION        PcrSelectionIn;
} TPM2_PCR_READ_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER      Header;
  UINT32                    PcrUpdateCounter;
  TPML_PCR_SELECTION        PcrSelectionOut;
  TPML_DIGEST               PcrValues;
} TPM2_PCR_READ_RESPONSE;

#pragma pack()

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
  )
{
  EFI_STATUS                        Status;
  TPM2_PCR_EXTEND_COMMAND           Cmd;
  TPM2_PCR_EXTEND_RESPONSE          Res;
  UINT32                            CmdSize;
  UINT32                            RespSize;
  UINT32                            ResultBufSize;
  UINT8                             *Buffer;
  UINTN                             Index;
  UINT32                            SessionInfoSize;
  UINT16                            DigestSize;

  SetMem(&Cmd, sizeof(Cmd), 0);
  Cmd.Header.tag         = SwapBytes16(TPM_ST_SESSIONS);
  Cmd.Header.commandCode = SwapBytes32(TPM_CC_PCR_Extend);
  Cmd.PcrHandle          = SwapBytes32(PcrHandle);


  //
  // Add in Auth session
  //
  Buffer = (UINT8 *)&Cmd.AuthSessionPcr;
  
  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (NULL, Buffer);
  Buffer += SessionInfoSize;
  Cmd.AuthorizationSize = SwapBytes32(SessionInfoSize);
  
  //Digest Count
  WriteUnaligned32 ((UINT32 *)Buffer, SwapBytes32(Digests->count));
  Buffer += sizeof(UINT32);
  
  //Digest
  for (Index = 0; Index < Digests->count; Index++) {
    WriteUnaligned16 ((UINT16 *)Buffer, SwapBytes16(Digests->digests[Index].hashAlg));
    Buffer += sizeof(UINT16);
    DigestSize = GetHashSizeFromAlgo (Digests->digests[Index].hashAlg);
    if (DigestSize == 0) {
      DEBUG ((EFI_D_ERROR, "Unknown hash algorithm %d\r\n", Digests->digests[Index].hashAlg));
      return EFI_DEVICE_ERROR;
    }
    CopyMem(
      Buffer,
      &Digests->digests[Index].digest,
      DigestSize
      );
    Buffer += DigestSize;
  }

  CmdSize              = (UINT32)((UINTN)Buffer - (UINTN)&Cmd);
  Cmd.Header.paramSize = SwapBytes32(CmdSize);

  ResultBufSize = sizeof(Res);
  Status = Tpm2SubmitCommand (CmdSize, (UINT8 *)&Cmd, &ResultBufSize, (UINT8 *)&Res);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "Tpm2PcrExtend: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = SwapBytes32(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "Tpm2PcrExtend: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (SwapBytes32(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "Tpm2PcrExtend: Response Code error! 0x%08x\r\n", SwapBytes32(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  //
  // Unmarshal the response
  //

  // None

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS                        Status;
  TPM2_PCR_EVENT_COMMAND            Cmd;
  TPM2_PCR_EVENT_RESPONSE           Res;
  UINT32                            CmdSize;
  UINT32                            RespSize;
  UINT32                            ResultBufSize;
  UINT8                             *Buffer;
  UINTN                             Index;
  UINT32                            SessionInfoSize;
  UINT16                            DigestSize;

  Cmd.Header.tag         = SwapBytes16(TPM_ST_SESSIONS);
  Cmd.Header.commandCode = SwapBytes32(TPM_CC_PCR_Event);
  Cmd.PcrHandle          = SwapBytes32(PcrHandle);

  //
  // Add in Auth session
  //
  Buffer = (UINT8 *)&Cmd.AuthSessionPcr;

  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (NULL, Buffer);
  Buffer += SessionInfoSize;
  Cmd.AuthorizationSize = SwapBytes32(SessionInfoSize);

  // Event
  WriteUnaligned16 ((UINT16 *)Buffer, SwapBytes16(EventData->t.size));
  Buffer += sizeof(UINT16);

  CopyMem (Buffer, EventData->t.buffer, EventData->t.size);
  Buffer += EventData->t.size;
  
  CmdSize              = (UINT32)((UINTN)Buffer - (UINTN)&Cmd);
  Cmd.Header.paramSize = SwapBytes32(CmdSize);

  ResultBufSize = sizeof(Res);
  Status = Tpm2SubmitCommand (CmdSize, (UINT8 *)&Cmd, &ResultBufSize, (UINT8 *)&Res);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "Tpm2PcrEvent: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = SwapBytes32(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "Tpm2PcrEvent: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (SwapBytes32(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "Tpm2PcrEvent: Response Code error! 0x%08x\r\n", SwapBytes32(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  //
  // Unmarshal the response
  //
  Buffer = (UINT8 *)&Res.Digests;

  Digests->count = SwapBytes32 (ReadUnaligned32 ((UINT32 *)Buffer));
  Buffer += sizeof(UINT32);
  for (Index = 0; Index < Digests->count; Index++) {
    Digests->digests[Index].hashAlg = SwapBytes16 (ReadUnaligned16 ((UINT16 *)Buffer));
    Buffer += sizeof(UINT16);
    DigestSize = GetHashSizeFromAlgo (Digests->digests[Index].hashAlg);
    if (DigestSize == 0) {
      DEBUG ((EFI_D_ERROR, "Unknown hash algorithm %d\r\n", Digests->digests[Index].hashAlg));
      return EFI_DEVICE_ERROR;
    }
    CopyMem(
      &Digests->digests[Index].digest,
      Buffer,
      DigestSize
      );
    Buffer += DigestSize;
  }

  return EFI_SUCCESS;
}

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
  )
{
  EFI_STATUS                        Status;
  TPM2_PCR_READ_COMMAND             SendBuffer;
  TPM2_PCR_READ_RESPONSE            RecvBuffer;
  UINT32                            SendBufferSize;
  UINT32                            RecvBufferSize;
  UINTN                             Index;
  TPML_DIGEST                       *PcrValuesOut;
  TPM2B_DIGEST                      *Digests;

  //
  // Construct command
  //
  SetMem(&SendBuffer, sizeof(SendBuffer), 0);
  SendBuffer.Header.tag = SwapBytes16(TPM_ST_NO_SESSIONS);
  SendBuffer.Header.commandCode = SwapBytes32(TPM_CC_PCR_Read);
 
  SendBuffer.PcrSelectionIn.count = SwapBytes32(PcrSelectionIn->count);
  for (Index = 0; Index < PcrSelectionIn->count; Index++) {
    SendBuffer.PcrSelectionIn.pcrSelections[Index].hash = SwapBytes16(PcrSelectionIn->pcrSelections[Index].hash);
    SendBuffer.PcrSelectionIn.pcrSelections[Index].sizeofSelect = PcrSelectionIn->pcrSelections[Index].sizeofSelect;
    CopyMem (&SendBuffer.PcrSelectionIn.pcrSelections[Index].pcrSelect, &PcrSelectionIn->pcrSelections[Index].pcrSelect, sizeof(SendBuffer.PcrSelectionIn.pcrSelections[Index].pcrSelect));
  }

  SendBufferSize = sizeof(SendBuffer.Header) + sizeof(SendBuffer.PcrSelectionIn.count) + sizeof(SendBuffer.PcrSelectionIn.pcrSelections[0]) * PcrSelectionIn->count;
  SendBuffer.Header.paramSize = SwapBytes32 (SendBufferSize);

  //
  // send Tpm command
  //
  RecvBufferSize = sizeof (RecvBuffer);
  Status = Tpm2SubmitCommand (SendBufferSize, (UINT8 *)&SendBuffer, &RecvBufferSize, (UINT8 *)&RecvBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (RecvBufferSize < sizeof (TPM2_RESPONSE_HEADER)) {
    DEBUG ((EFI_D_ERROR, "Tpm2PcrRead - RecvBufferSize Error - %x\n", RecvBufferSize));
    return EFI_DEVICE_ERROR;
  }
  if (SwapBytes32(RecvBuffer.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "Tpm2PcrRead - responseCode - %x\n", SwapBytes32(RecvBuffer.Header.responseCode)));
    return EFI_NOT_FOUND;
  }

  //
  // Return the response
  //

  //
  // PcrUpdateCounter
  //
  if (RecvBufferSize < sizeof (TPM2_RESPONSE_HEADER) + sizeof(RecvBuffer.PcrUpdateCounter)) {
    DEBUG ((EFI_D_ERROR, "Tpm2PcrRead - RecvBufferSize Error - %x\n", RecvBufferSize));
    return EFI_DEVICE_ERROR;
  }
  *PcrUpdateCounter = SwapBytes32(RecvBuffer.PcrUpdateCounter);

  //
  // PcrSelectionOut
  //
  if (RecvBufferSize < sizeof (TPM2_RESPONSE_HEADER) + sizeof(RecvBuffer.PcrUpdateCounter) + sizeof(RecvBuffer.PcrSelectionOut.count)) {
    DEBUG ((EFI_D_ERROR, "Tpm2PcrRead - RecvBufferSize Error - %x\n", RecvBufferSize));
    return EFI_DEVICE_ERROR;
  }
  PcrSelectionOut->count = SwapBytes32(RecvBuffer.PcrSelectionOut.count);
  if (RecvBufferSize < sizeof (TPM2_RESPONSE_HEADER) + sizeof(RecvBuffer.PcrUpdateCounter) + sizeof(RecvBuffer.PcrSelectionOut.count) + sizeof(RecvBuffer.PcrSelectionOut.pcrSelections[0]) * PcrSelectionOut->count) {
    DEBUG ((EFI_D_ERROR, "Tpm2PcrRead - RecvBufferSize Error - %x\n", RecvBufferSize));
    return EFI_DEVICE_ERROR;
  }
  for (Index = 0; Index < PcrSelectionOut->count; Index++) {
    PcrSelectionOut->pcrSelections[Index].hash = SwapBytes16(RecvBuffer.PcrSelectionOut.pcrSelections[Index].hash);
    PcrSelectionOut->pcrSelections[Index].sizeofSelect = RecvBuffer.PcrSelectionOut.pcrSelections[Index].sizeofSelect;
    CopyMem (&PcrSelectionOut->pcrSelections[Index].pcrSelect, &RecvBuffer.PcrSelectionOut.pcrSelections[Index].pcrSelect, sizeof(RecvBuffer.PcrSelectionOut.pcrSelections[Index].pcrSelect));
  }

  //
  // PcrValues
  //
  PcrValuesOut = (TPML_DIGEST *)((UINT8 *)&RecvBuffer + sizeof (TPM2_RESPONSE_HEADER) + sizeof(RecvBuffer.PcrUpdateCounter) + sizeof(RecvBuffer.PcrSelectionOut.count) + sizeof(RecvBuffer.PcrSelectionOut.pcrSelections[0]) * PcrSelectionOut->count);
  PcrValues->count = SwapBytes32(PcrValuesOut->count);
  Digests = PcrValuesOut->digests;
  for (Index = 0; Index < PcrValues->count; Index++) {
    PcrValues->digests[Index].t.size = SwapBytes16(Digests->t.size);
    CopyMem (&PcrValues->digests[Index].t.buffer, &Digests->t.buffer, PcrValues->digests[Index].t.size);
    Digests = (TPM2B_DIGEST *)((UINT8 *)Digests + sizeof(Digests->t.size) + PcrValues->digests[Index].t.size);
  }

  return EFI_SUCCESS;
}
