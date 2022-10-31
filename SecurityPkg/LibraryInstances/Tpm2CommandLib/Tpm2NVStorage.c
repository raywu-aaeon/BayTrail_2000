/** @file
  Implement TPM2 NVStorage related command.

Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <IndustryStandard/Tpm20.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

#pragma pack(1)

#define RC_NV_ReadPublic_nvIndex            (TPM_RC_H + TPM_RC_1)

typedef struct {
  TPM2_COMMAND_HEADER       Header;
  TPMI_RH_NV_INDEX          NvIndex;
} TPM2_NV_READPUBLIC_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER      Header;
  TPM2B_NV_PUBLIC           NvPublic;
  TPM2B_NAME                NvName;
} TPM2_NV_READPUBLIC_RESPONSE;

#pragma pack()

/**
  This command is used to read the public area and Name of an NV Index.

  @param[in]  NvIndex            The NV Index.
  @param[out] NvPublic           The public area of the index.
  @param[out] NvName             The Name of the nvIndex.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_NOT_FOUND          The command was returned successfully, but NvIndex is not found.
**/
EFI_STATUS
EFIAPI
Tpm2NvReadPublic (
  IN      TPMI_RH_NV_INDEX          NvIndex,
  OUT     TPM2B_NV_PUBLIC           *NvPublic,
  OUT     TPM2B_NAME                *NvName
  )
{
  EFI_STATUS                        Status;
  TPM2_NV_READPUBLIC_COMMAND        SendBuffer;
  TPM2_NV_READPUBLIC_RESPONSE       RecvBuffer;
  UINT32                            SendBufferSize;
  UINT32                            RecvBufferSize;
  UINT16                            NvPublicSize;
  UINT16                            NvNameSize;
  UINT8                             *Buffer;
  TPM_RC                            ResponseCode;

  //
  // Construct command
  //
  SendBuffer.Header.tag = SwapBytes16(TPM_ST_NO_SESSIONS);
  SendBuffer.Header.commandCode = SwapBytes32(TPM_CC_NV_ReadPublic);

  SendBuffer.NvIndex = SwapBytes32 (NvIndex);
 
  SendBufferSize = (UINT32) sizeof (SendBuffer);
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
    DEBUG ((EFI_D_ERROR, "Tpm2NvReadPublic - RecvBufferSize Error - %x\n", RecvBufferSize));
    return EFI_DEVICE_ERROR;
  }
  ResponseCode = SwapBytes32(RecvBuffer.Header.responseCode);
  if (ResponseCode != TPM_RC_SUCCESS) {
    DEBUG ((EFI_D_ERROR, "Tpm2NvReadPublic - responseCode - %x\n", SwapBytes32(RecvBuffer.Header.responseCode)));
  }
  switch (ResponseCode) {
  case TPM_RC_SUCCESS:
    // return data
    break;
  case TPM_RC_HANDLE + RC_NV_ReadPublic_nvIndex: // TPM_RC_NV_DEFINED:
    return EFI_NOT_FOUND;
  case TPM_RC_VALUE + RC_NV_ReadPublic_nvIndex:
    return EFI_INVALID_PARAMETER;
  default:
    return EFI_DEVICE_ERROR;
  }

  if (RecvBufferSize <= sizeof (TPM2_RESPONSE_HEADER) + sizeof (UINT16) + sizeof(UINT16)) {
    DEBUG ((EFI_D_ERROR, "Tpm2NvReadPublic - RecvBufferSize Error - %x\n", RecvBufferSize));
    return EFI_NOT_FOUND;
  }

  //
  // Basic check
  //
  NvPublicSize = SwapBytes16 (RecvBuffer.NvPublic.t.size);
  NvNameSize = SwapBytes16 (ReadUnaligned16 ((UINT16 *)((UINT8 *)&RecvBuffer + sizeof(TPM2_RESPONSE_HEADER) + sizeof(UINT16) + NvPublicSize)));

  if (RecvBufferSize != sizeof(TPM2_RESPONSE_HEADER) + sizeof(UINT16) + NvPublicSize + sizeof(UINT16) + NvNameSize) {
    DEBUG ((EFI_D_ERROR, "Tpm2NvReadPublic - RecvBufferSize Error - NvPublicSize %x, NvNameSize %x\n", RecvBufferSize, NvNameSize));
    return EFI_NOT_FOUND;
  }

  //
  // Return the response
  //
  CopyMem (NvPublic, &RecvBuffer.NvPublic, sizeof(UINT16) + NvPublicSize);
  NvPublic->t.size = NvPublicSize;
  NvPublic->t.nvPublic.nvIndex = SwapBytes32 (NvPublic->t.nvPublic.nvIndex);
  NvPublic->t.nvPublic.nameAlg = SwapBytes16 (NvPublic->t.nvPublic.nameAlg);
  WriteUnaligned32 ((UINT32 *)&NvPublic->t.nvPublic.attributes, SwapBytes32 (ReadUnaligned32 ((UINT32 *)&NvPublic->t.nvPublic.attributes)));
  NvPublic->t.nvPublic.authPolicy.t.size = SwapBytes16 (NvPublic->t.nvPublic.authPolicy.t.size);
  Buffer = (UINT8 *)&NvPublic->t.nvPublic.authPolicy;
  Buffer += sizeof(UINT16) + NvPublic->t.nvPublic.authPolicy.t.size;
  NvPublic->t.nvPublic.dataSize = SwapBytes16 (ReadUnaligned16 ((UINT16 *)Buffer));

  CopyMem (NvName, (UINT8 *)&RecvBuffer + sizeof(TPM2_RESPONSE_HEADER) + sizeof(UINT16) + NvPublicSize, NvNameSize);
  NvName->t.size = NvNameSize;
  
  return EFI_SUCCESS;
}
