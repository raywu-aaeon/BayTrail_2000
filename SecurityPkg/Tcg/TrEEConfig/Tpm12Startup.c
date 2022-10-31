/** @file
  Implement TPM1.2 Startup related command.

Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <IndustryStandard/Tpm12.h>
#include <Library/BaseMemoryLib.h>
#include <Library/BaseLib.h>

#pragma pack(1)

typedef struct {
  TPM_RQU_COMMAND_HDR   Hdr;
  TPM_STARTUP_TYPE      TpmSt;
} TPM_CMD_START_UP;

typedef struct {
  TPM_RSP_COMMAND_HDR   Hdr;
} TPM_RSP_START_UP;

#pragma pack()

/**
  This service enables the sending of commands to the TPM12.

  @param[in]      InputParameterBlockSize  Size of the TPM12 input parameter block.
  @param[in]      InputParameterBlock      Pointer to the TPM12 input parameter block.
  @param[in,out]  OutputParameterBlockSize Size of the TPM12 output parameter block.
  @param[in]      OutputParameterBlock     Pointer to the TPM12 output parameter block.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small. 
**/
EFI_STATUS
EFIAPI
Tpm12SubmitCommand (
  IN UINT32            InputParameterBlockSize,
  IN UINT8             *InputParameterBlock,
  IN OUT UINT32        *OutputParameterBlockSize,
  IN UINT8             *OutputParameterBlock
  );

/**
  Send Startup command to TPM1.2.

  @param TpmSt           Startup Type.

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm12Startup (
  IN TPM_STARTUP_TYPE          TpmSt
  )
{
  EFI_STATUS                        Status;
  UINT32                            TpmRecvSize;
  UINT32                            TpmSendSize;
  TPM_CMD_START_UP                  SendBuffer;
  TPM_RSP_START_UP                  RecvBuffer;
  UINT32                            ReturnCode;

  //
  // send Tpm command TPM_ORD_Startup
  //
  TpmRecvSize               = sizeof (TPM_RSP_START_UP);
  TpmSendSize               = sizeof (TPM_CMD_START_UP);
  SendBuffer.Hdr.tag        = SwapBytes16 (TPM_TAG_RQU_COMMAND);
  SendBuffer.Hdr.paramSize  = SwapBytes32 (TpmSendSize);
  SendBuffer.Hdr.ordinal    = SwapBytes32 (TPM_ORD_Startup);
  SendBuffer.TpmSt          = SwapBytes16 (TpmSt);

  Status = Tpm12SubmitCommand (TpmSendSize, (UINT8 *)&SendBuffer, &TpmRecvSize, (UINT8 *)&RecvBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ReturnCode = SwapBytes32(RecvBuffer.Hdr.returnCode);
  switch (ReturnCode) {
  case TPM_SUCCESS:
  case TPM_INVALID_POSTINIT:
    // In warm reset, TPM may response TPM_INVALID_POSTINIT
    return EFI_SUCCESS;
  default:
    return EFI_DEVICE_ERROR;
  }
}