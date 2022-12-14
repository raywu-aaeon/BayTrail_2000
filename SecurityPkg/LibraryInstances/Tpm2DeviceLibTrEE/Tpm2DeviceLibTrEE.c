/** @file
  Ihis library is TPM2 TREE protocol lib.

Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Protocol/TrEEProtocol.h>
#include <IndustryStandard/Tpm20.h>

EFI_TREE_PROTOCOL  *mTreeProtocol = NULL; 

/**
  This service enables the sending of commands to the TPM2.

  @param[in]      InputParameterBlockSize  Size of the TPM2 input parameter block.
  @param[in]      InputParameterBlock      Pointer to the TPM2 input parameter block.
  @param[in,out]  OutputParameterBlockSize Size of the TPM2 output parameter block.
  @param[in]      OutputParameterBlock     Pointer to the TPM2 output parameter block.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small. 
**/
EFI_STATUS
EFIAPI
Tpm2SubmitCommand (
  IN UINT32            InputParameterBlockSize,
  IN UINT8             *InputParameterBlock,
  IN OUT UINT32        *OutputParameterBlockSize,
  IN UINT8             *OutputParameterBlock
  )
{
  EFI_STATUS                Status;
  TPM2_RESPONSE_HEADER      *Header;

  if (mTreeProtocol == NULL) {
    Status = gBS->LocateProtocol (&gEfiTrEEProtocolGuid, NULL, (VOID **) &mTreeProtocol);
    if (EFI_ERROR (Status)) {
      //
      // TrEE protocol is not installed. So, TPM2 is not present.
      //
      DEBUG ((EFI_D_ERROR, "Tpm2SubmitCommand - TrEE - %r\n", Status));
      return EFI_NOT_FOUND;
    }
  }
  //
  // Assume when TrEE Protocol is ready, RequestUseTpm already done.
  //
  Status = mTreeProtocol->SubmitCommand (
                            mTreeProtocol,
                            InputParameterBlockSize,
                            InputParameterBlock,
                            *OutputParameterBlockSize,
                            OutputParameterBlock
                            );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  Header = (TPM2_RESPONSE_HEADER *)OutputParameterBlock;
  *OutputParameterBlockSize = SwapBytes32 (Header->paramSize);

  return EFI_SUCCESS;
}

/**
  This service requests use TPM2.

  @retval EFI_SUCCESS      Get the control of TPM2 chip.
  @retval EFI_NOT_FOUND    TPM2 not found.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2RequestUseTpm (
  VOID
  )
{
  EFI_STATUS   Status;

  if (mTreeProtocol == NULL) {
    Status = gBS->LocateProtocol (&gEfiTrEEProtocolGuid, NULL, (VOID **) &mTreeProtocol);
    if (EFI_ERROR (Status)) {
      //
      // TrEE protocol is not installed. So, TPM2 is not present.
      //
      DEBUG ((EFI_D_ERROR, "Tpm2RequestUseTpm - TrEE - %r\n", Status));
      return EFI_NOT_FOUND;
    }
  }
  //
  // Assume when TrEE Protocol is ready, RequestUseTpm already done.
  //
  return EFI_SUCCESS;
}

/**
  This service register TPM2 device.

  @param Tpm2Device  TPM2 device

  @retval EFI_SUCCESS          This TPM2 device is registered successfully.
  @retval EFI_UNSUPPORTED      System does not support register this TPM2 device.
  @retval EFI_ALREADY_STARTED  System already register this TPM2 device.
**/
EFI_STATUS
EFIAPI
Tpm2RegisterTpm2DeviceLib (
  IN TPM2_DEVICE_INTERFACE   *Tpm2Device
  )
{
  return EFI_UNSUPPORTED;
}
