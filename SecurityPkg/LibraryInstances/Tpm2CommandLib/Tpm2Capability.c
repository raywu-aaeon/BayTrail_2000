/** @file
  Implement TPM2 Capability related command.

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
  TPM_CAP                   Capability;
  UINT32                    Property;
  UINT32                    PropertyCount;
} TPM2_GET_CAPABILITY_COMMAND;

typedef struct {
  TPM2_RESPONSE_HEADER      Header;
  TPMI_YES_NO               MoreData;
  TPMS_CAPABILITY_DATA      CapabilityData;
} TPM2_GET_CAPABILITY_RESPONSE;

#pragma pack()

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
  )
{
  EFI_STATUS                        Status;
  TPM2_GET_CAPABILITY_COMMAND       SendBuffer;
  TPM2_GET_CAPABILITY_RESPONSE      RecvBuffer;
  UINT32                            SendBufferSize;
  UINT32                            RecvBufferSize;

  //
  // Construct command
  //
  SendBuffer.Header.tag = SwapBytes16(TPM_ST_NO_SESSIONS);
  SendBuffer.Header.commandCode = SwapBytes32(TPM_CC_GetCapability);

  SendBuffer.Capability = SwapBytes32 (Capability);
  SendBuffer.Property = SwapBytes32 (Property);
  SendBuffer.PropertyCount = SwapBytes32 (PropertyCount);
 
  SendBufferSize = (UINT32) sizeof (SendBuffer);
  SendBuffer.Header.paramSize = SwapBytes32 (SendBufferSize);
    
  //
  // send Tpm command
  //
  RecvBufferSize = sizeof (RecvBuffer);
  Status = Tpm2SubmitCommand (SendBufferSize, (UINT8 *)&SendBuffer, &RecvBufferSize, (UINT8 *)&RecvBuffer );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (RecvBufferSize <= sizeof (TPM2_RESPONSE_HEADER) + sizeof (UINT8)) {
    return EFI_DEVICE_ERROR;
  }

  //
  // Return the response
  //
  *MoreData = RecvBuffer.MoreData;
  //
  // Does not unpack all possiable property here, the caller should unpack it and note the byte order.
  //
  CopyMem (CapabilityData, &RecvBuffer.CapabilityData, RecvBufferSize - sizeof (TPM2_RESPONSE_HEADER) - sizeof (UINT8));
  
  return EFI_SUCCESS;
}

/**
  This command returns the information of TPM manufacture ID.

  This function parse the value got from TPM2_GetCapability and return the TPM manufacture ID.

  @param[out] ManufactureId      The manufacture ID of TPM.
  
  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
**/
EFI_STATUS
EFIAPI
Tpm2GetCapabilityFamily (
  OUT     CHAR8                     *Family
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status; 

  Status = Tpm2GetCapability (
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_FAMILY_INDICATOR, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  CopyMem (Family, &TpmCap.data.tpmProperties.tpmProperty->value, 4);

  return EFI_SUCCESS;
}

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
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status; 

  Status = Tpm2GetCapability (
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_MANUFACTURER, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *ManufactureId = SwapBytes32 (TpmCap.data.tpmProperties.tpmProperty->value);

  return EFI_SUCCESS;
}

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
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status; 

  Status = Tpm2GetCapability (
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_FIRMWARE_VERSION_1, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *FirmwareVersion1 = SwapBytes32 (TpmCap.data.tpmProperties.tpmProperty->value);

  Status = Tpm2GetCapability (
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_FIRMWARE_VERSION_2, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *FirmwareVersion2 = SwapBytes32 (TpmCap.data.tpmProperties.tpmProperty->value);

  return EFI_SUCCESS;
}

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
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status;

  Status = Tpm2GetCapability (
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_MAX_COMMAND_SIZE, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *MaxCommandSize = SwapBytes32 (TpmCap.data.tpmProperties.tpmProperty->value);

  Status = Tpm2GetCapability (
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_MAX_RESPONSE_SIZE, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *MaxResponseSize = SwapBytes32 (TpmCap.data.tpmProperties.tpmProperty->value);
  return EFI_SUCCESS; 
}

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
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  UINTN                   Index;
  EFI_STATUS              Status;
 
  Status = Tpm2GetCapability (
             TPM_CAP_ALGS, 
             1, 
             MAX_CAP_ALGS, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  
  CopyMem (AlgList, &TpmCap.data.algorithms, sizeof (TPML_ALG_PROPERTY));

  AlgList->count = SwapBytes32 (AlgList->count);
  for (Index = 0; Index < AlgList->count; Index++) {
    AlgList->algProperties[Index].alg = SwapBytes16 (AlgList->algProperties[Index].alg);
    WriteUnaligned32 ((UINT32 *)&AlgList->algProperties[Index].algProperties, SwapBytes32 (ReadUnaligned32 ((UINT32 *)&AlgList->algProperties[Index].algProperties)));
  }

  return EFI_SUCCESS;
}

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
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status; 

  Status = Tpm2GetCapability (
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_LOCKOUT_COUNTER, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *LockoutCounter = SwapBytes32 (TpmCap.data.tpmProperties.tpmProperty->value);

  return EFI_SUCCESS;
}

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
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status; 

  Status = Tpm2GetCapability (
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_LOCKOUT_INTERVAL, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *LockoutInterval = SwapBytes32 (TpmCap.data.tpmProperties.tpmProperty->value);

  return EFI_SUCCESS;
}

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
  )
{
  TPMS_CAPABILITY_DATA    TpmCap;
  TPMI_YES_NO             MoreData;
  EFI_STATUS              Status; 

  Status = Tpm2GetCapability (
             TPM_CAP_TPM_PROPERTIES, 
             TPM_PT_INPUT_BUFFER, 
             1, 
             &MoreData, 
             &TpmCap
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  *InputBufferSize = SwapBytes32 (TpmCap.data.tpmProperties.tpmProperty->value);

  return EFI_SUCCESS;
}
