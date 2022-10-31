/** @file
  This module implements TrEE Protocol.
  
Copyright (c) 2012, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials 
are licensed and made available under the terms and conditions of the BSD License 
which accompanies this distribution.  The full text of the license may be found at 
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, 
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/PeImage.h>
#include <IndustryStandard/SmBios.h>
#include <IndustryStandard/TcpaAcpi.h>

#include <Guid/GlobalVariable.h>
#include <Guid/SmBios.h>
#include <Guid/HobList.h>
#include <Guid/TcgEventHob.h>
#include <Guid/EventGroup.h>
//#include <Guid/EventExitBootServiceFailed.h>
#include <Guid/ImageAuthentication.h>
#include <Guid/TpmInstance.h>

#include <Protocol/DevicePath.h>
#include <Protocol/AcpiTable.h>
#include <Protocol/VariableWrite.h>
#include <Protocol/TrEEProtocol.h>

#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/HobLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/Tpm2CommandLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>
#include <Library/Tpm2DeviceLib.h>
#include <Library/HashLib.h>
#include <BaseCryptLib.h>

#define TPM_BASE_ADDRESS            0xfed40000

typedef struct {
  CHAR16                                 *VariableName;
  EFI_GUID                               *VendorGuid;
} VARIABLE_TYPE;

#define  EFI_TCG_LOG_AREA_SIZE        0x10000  // TBD - PCD?
#define  TREE_DEFAULT_MAX_COMMAND_SIZE        0x1000
#define  TREE_DEFAULT_MAX_RESPONSE_SIZE       0x1000

// The authorization value may be no larger than the digest produced by the hash algorithm used for context integrity.
#define MAX_NEW_AUTHORIZATION_SIZE        SHA512_DIGEST_SIZE

typedef struct _TCG_DXE_DATA {
  TREE_BOOT_SERVICE_CAPABILITY      BsCap;
  EFI_TCG_CLIENT_ACPI_TABLE         *TcgClientAcpiTable;
  EFI_TCG_SERVER_ACPI_TABLE         *TcgServerAcpiTable;
  UINTN                             EventLogSize;
  UINT8                             *LastEvent;
  BOOLEAN                           EventLogStarted;
  BOOLEAN                           EventLogTruncated;
} TCG_DXE_DATA;

EFI_TCG_CLIENT_ACPI_TABLE           mTcgClientAcpiTemplate = {
  {
    EFI_ACPI_3_0_TRUSTED_COMPUTING_PLATFORM_ALLIANCE_CAPABILITIES_TABLE_SIGNATURE,
    sizeof (mTcgClientAcpiTemplate),
    0x02                      //Revision
    //
    // Compiler initializes the remaining bytes to 0
    // These fields should be filled in in production
    //
  },
  0,                          // 0 for PC Client Platform Class
  0,                          // Log Area Max Length
  (EFI_PHYSICAL_ADDRESS) (SIZE_4GB - 1)  // Log Area Start Address
};

//
// The following EFI_TCG_SERVER_ACPI_TABLE default setting is just one example,
// the TPM device connectes to LPC, and also defined the ACPI _UID as 0xFF,
// this _UID can be changed and should match with the _UID setting of the TPM 
// ACPI device object  
//
EFI_TCG_SERVER_ACPI_TABLE           mTcgServerAcpiTemplate = {
  {
    EFI_ACPI_3_0_TRUSTED_COMPUTING_PLATFORM_ALLIANCE_CAPABILITIES_TABLE_SIGNATURE,
    sizeof (mTcgServerAcpiTemplate),
    0x02                      //Revision
    //
    // Compiler initializes the remaining bytes to 0
    // These fields should be filled in in production
    //
  },
  1,                          // 1 for Server Platform Class
  0,                          // Reserved
  0,                          // Log Area Max Length
  (EFI_PHYSICAL_ADDRESS) (SIZE_4GB - 1), // Log Area Start Address
  0x0100,                     // TCG Specification revision 1.0
  2,                          // Device Flags
  0,                          // Interrupt Flags
  0,                          // GPE
  {0},                        // Reserved 3 bytes
  0,                          // Global System Interrupt
  {
    EFI_ACPI_3_0_SYSTEM_MEMORY,
    0,
    0,
    EFI_ACPI_3_0_BYTE,
    TPM_BASE_ADDRESS          // Base Address
  },
  0,                          // Reserved
  {0},                        // Configuration Address
  0xFF,                       // ACPI _UID value of the device, can be changed for different platforms
  0,                          // ACPI _UID value of the device, can be changed for different platforms
  0,                          // ACPI _UID value of the device, can be changed for different platforms
  0                           // ACPI _UID value of the device, can be changed for different platforms
};

TCG_DXE_DATA                 mTcgDxeData = {
  {
    sizeof (mTcgDxeData.BsCap),     // Size
    { 1, 0 },                       // StructureVersion
    { 1, 0 },                       // ProtocolVersion
    TREE_BOOT_HASH_ALG_SHA1,        // HashAlgorithmBitmap
    TREE_EVENT_LOG_FORMAT_TCG_1_2,  // SupportedEventLogs
    TRUE,                           // TrEEPresentFlag
    TREE_DEFAULT_MAX_COMMAND_SIZE,  // MaxCommandSize
    TREE_DEFAULT_MAX_RESPONSE_SIZE, // MaxResponseSize
    0                               // ManufacturerID
  },
  &mTcgClientAcpiTemplate,
  &mTcgServerAcpiTemplate,
};

UINTN  mBootAttempts  = 0;
CHAR16 mBootVarName[] = L"BootOrder";

VARIABLE_TYPE  mVariableType[] = {
  {EFI_SECURE_BOOT_MODE_NAME,    &gEfiGlobalVariableGuid},
  {EFI_PLATFORM_KEY_NAME,        &gEfiGlobalVariableGuid},
  {EFI_KEY_EXCHANGE_KEY_NAME,    &gEfiGlobalVariableGuid},
  {EFI_IMAGE_SECURITY_DATABASE,  &gEfiImageSecurityDatabaseGuid},
  {EFI_IMAGE_SECURITY_DATABASE1, &gEfiImageSecurityDatabaseGuid},
};

/**
  Measure PE image into TPM log based on the authenticode image hashing in
  PE/COFF Specification 8.0 Appendix A.

  Caution: This function may receive untrusted input.
  PE/COFF image is external input, so this function will validate its data structure
  within this image buffer before use.

  @param[in]  PCRIndex       TPM PCR index
  @param[in]  ImageAddress   Start address of image buffer.
  @param[in]  ImageSize      Image size
  @param[out] DigestList     Digeest list of this image.

  @retval EFI_SUCCESS            Successfully measure image.
  @retval EFI_OUT_OF_RESOURCES   No enough resource to measure image.
  @retval other error value
**/
EFI_STATUS
EFIAPI
MeasurePeImageAndExtend (
  IN  UINT32                    PCRIndex,
  IN  EFI_PHYSICAL_ADDRESS      ImageAddress,
  IN  UINTN                     ImageSize,
  OUT TPML_DIGEST_VALUES        *DigestList
  );

/**

  This function dump raw data.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
InternalDumpData (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN  Index;
  for (Index = 0; Index < Size; Index++) {
    DEBUG ((EFI_D_INFO, "%02x", (UINTN)Data[Index]));
  }
}

/**

  This function dump raw data with colume format.

  @param  Data  raw data
  @param  Size  raw data size

**/
VOID
InternalDumpHex (
  IN UINT8  *Data,
  IN UINTN  Size
  )
{
  UINTN   Index;
  UINTN   Count;
  UINTN   Left;

#define COLUME_SIZE  (16 * 2)

  Count = Size / COLUME_SIZE;
  Left  = Size % COLUME_SIZE;
  for (Index = 0; Index < Count; Index++) {
    DEBUG ((EFI_D_INFO, "%04x: ", Index * COLUME_SIZE));
    InternalDumpData (Data + Index * COLUME_SIZE, COLUME_SIZE);
    DEBUG ((EFI_D_INFO, "\n"));
  }

  if (Left != 0) {
    DEBUG ((EFI_D_INFO, "%04x: ", Index * COLUME_SIZE));
    InternalDumpData (Data + Index * COLUME_SIZE, Left);
    DEBUG ((EFI_D_INFO, "\n"));
  }
}

/**
  The EFI_TREE_PROTOCOL GetCapability function call provides protocol
  capability information and state information about the TrEE.

  @param[in]      This               Indicates the calling context
  @param[in, out] ProtocolCapability The caller allocates memory for a TREE_BOOT_SERVICE_CAPABILITY
                                     structure and sets the size field to the size of the structure allocated.
                                     The callee fills in the fields with the EFI protocol capability information
                                     and the current TrEE state information up to the number of fields which
                                     fit within the size of the structure passed in.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
                                 The ProtocolCapability variable will not be populated. 
  @retval EFI_INVALID_PARAMETER  One or more of the parameters are incorrect.
                                 The ProtocolCapability variable will not be populated.
  @retval EFI_BUFFER_TOO_SMALL   The ProtocolCapability variable is too small to hold the full response.
                                 It will be partially populated (required Size field will be set). 
**/
EFI_STATUS
EFIAPI
TreeGetCapability (
  IN EFI_TREE_PROTOCOL                *This,
  IN OUT TREE_BOOT_SERVICE_CAPABILITY *ProtocolCapability
  )
{
  TCG_DXE_DATA                      *TcgData;

  DEBUG ((EFI_D_ERROR, "TreeGetCapability ...\n"));

  if ((This == NULL) || (ProtocolCapability == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  TcgData = &mTcgDxeData;

  if (ProtocolCapability->Size < TcgData->BsCap.Size) {
    ProtocolCapability->Size = TcgData->BsCap.Size;
    return EFI_BUFFER_TOO_SMALL;
  }

  CopyMem (ProtocolCapability, &TcgData->BsCap, TcgData->BsCap.Size);
  DEBUG ((EFI_D_ERROR, "TreeGetCapability - %r\n", EFI_SUCCESS));
  return EFI_SUCCESS;
}

VOID
DumpEventLog (
  IN EFI_PHYSICAL_ADDRESS EventLogLocation,
  IN EFI_PHYSICAL_ADDRESS EventLogLastEntry
  )
{
  TCG_PCR_EVENT_HDR         *EventHdr;
  UINTN                     Index;

  EventHdr = (TCG_PCR_EVENT_HDR *)(UINTN)EventLogLocation;
  while ((UINTN)EventHdr < EventLogLastEntry) {
    DEBUG ((EFI_D_INFO, "  Event:\n"));
    DEBUG ((EFI_D_INFO, "    PCRIndex  - %d\n", EventHdr->PCRIndex));
    DEBUG ((EFI_D_INFO, "    EventType - 0x%08x\n", EventHdr->EventType));
    DEBUG ((EFI_D_INFO, "    Digest    - "));
    for (Index = 0; Index < sizeof(TCG_DIGEST); Index++) {
      DEBUG ((EFI_D_INFO, "%02x ", EventHdr->Digest.digest[Index]));
    }
    DEBUG ((EFI_D_INFO, "\n"));
    DEBUG ((EFI_D_INFO, "    EventSize - 0x%08x\n", EventHdr->EventSize));
    InternalDumpHex ((UINT8 *)(EventHdr + 1), EventHdr->EventSize);
    EventHdr = (TCG_PCR_EVENT_HDR *)((UINTN)EventHdr + sizeof(TCG_PCR_EVENT_HDR) + EventHdr->EventSize);
  }
}

/**
  The EFI_TREE_PROTOCOL Get Event Log function call allows a caller to
  retrieve the address of a given event log and its last entry. 

  @param[in]  This               Indicates the calling context
  @param[in]  EventLogFormat     The type of the event log for which the information is requested.
  @param[out] EventLogLocation   A pointer to the memory address of the event log.
  @param[out] EventLogLastEntry  If the Event Log contains more than one entry, this is a pointer to the
                                 address of the start of the last entry in the event log in memory.
  @param[out] EventLogTruncated  If the Event Log is missing at least one entry because an event would
                                 have exceeded the area allocated for events, this value is set to TRUE.
                                 Otherwise, the value will be FALSE and the Event Log will be complete.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_INVALID_PARAMETER  One or more of the parameters are incorrect
                                 (e.g. asking for an event log whose format is not supported).
**/
EFI_STATUS
EFIAPI
TreeGetEventLog (
  IN EFI_TREE_PROTOCOL     *This,
  IN TREE_EVENT_LOG_FORMAT EventLogFormat,
  OUT EFI_PHYSICAL_ADDRESS *EventLogLocation,
  OUT EFI_PHYSICAL_ADDRESS *EventLogLastEntry,
  OUT BOOLEAN              *EventLogTruncated
  )
{
  TCG_DXE_DATA                      *TcgData;

  DEBUG ((EFI_D_ERROR, "TreeGetEventLog ...\n"));

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (EventLogFormat != TREE_EVENT_LOG_FORMAT_TCG_1_2) {
    return EFI_INVALID_PARAMETER;
  }

  TcgData = &mTcgDxeData;
  if (!TcgData->BsCap.TrEEPresentFlag) {
    if (EventLogLocation != NULL) {
      *EventLogLocation = 0;
    }
    if (EventLogLastEntry != NULL) {
      *EventLogLastEntry = 0;
    }
    if (EventLogTruncated != NULL) {
      *EventLogTruncated = FALSE;
    }
    return EFI_SUCCESS;
  }

  if (EventLogLocation != NULL) {
    if (PcdGet8 (PcdTpmPlatformClass) == TCG_PLATFORM_TYPE_CLIENT) {
      *EventLogLocation = TcgData->TcgClientAcpiTable->Lasa;
    } else {
      *EventLogLocation = TcgData->TcgServerAcpiTable->Lasa;
    }
    DEBUG ((EFI_D_ERROR, "TreeGetEventLog (EventLogLocation - %x)\n", *EventLogLocation));
  }

  if (EventLogLastEntry != NULL) {
    if (!TcgData->EventLogStarted) {
      *EventLogLastEntry = (EFI_PHYSICAL_ADDRESS)(UINTN)0;
    } else {
      *EventLogLastEntry = (EFI_PHYSICAL_ADDRESS)(UINTN)TcgData->LastEvent;
    }
    DEBUG ((EFI_D_ERROR, "TreeGetEventLog (EventLogLastEntry - %x)\n", *EventLogLastEntry));
  }

  if (EventLogTruncated != NULL) {
    *EventLogTruncated = TcgData->EventLogTruncated;
    DEBUG ((EFI_D_ERROR, "TreeGetEventLog (EventLogTruncated - %x)\n", *EventLogTruncated));
  }

  DEBUG ((EFI_D_ERROR, "TreeGetEventLog - %r\n", EFI_SUCCESS));

  // Dump Event Log for debug purpose
//  DumpEventLog (*EventLogLocation, *EventLogLastEntry);

  return EFI_SUCCESS;
}

/**
  Add a new entry to the Event Log.

  @param[in, out] EventLogPtr   Pointer to the Event Log data.  
  @param[in, out] LogSize       Size of the Event Log.  
  @param[in]      MaxSize       Maximum size of the Event Log.
  @param[in]      NewEventHdr   Pointer to a TCG_PCR_EVENT_HDR data structure.  
  @param[in]      NewEventData  Pointer to the new event data.  
  
  @retval EFI_SUCCESS           The new event log entry was added.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.

**/
EFI_STATUS
TcgCommLogEvent (
  IN OUT  UINT8                     **EventLogPtr,
  IN OUT  UINTN                     *LogSize,
  IN      UINTN                     MaxSize,
  IN      TCG_PCR_EVENT_HDR         *NewEventHdr,
  IN      UINT8                     *NewEventData
  )
{
  UINT32                            NewLogSize;

  NewLogSize = sizeof (*NewEventHdr) + NewEventHdr->EventSize;
  if (NewLogSize + *LogSize > MaxSize) {
    return EFI_OUT_OF_RESOURCES;
  }

  *EventLogPtr += *LogSize;
  *LogSize += NewLogSize;
  CopyMem (*EventLogPtr, NewEventHdr, sizeof (*NewEventHdr));
  CopyMem (
    *EventLogPtr + sizeof (*NewEventHdr),
    NewEventData,
    NewEventHdr->EventSize
    );
  return EFI_SUCCESS;
}

/**
  Add a new entry to the Event Log.

  @param[in] TcgData       TCG_DXE_DATA structure.
  @param[in] NewEventHdr   Pointer to a TCG_PCR_EVENT_HDR data structure.  
  @param[in] NewEventData  Pointer to the new event data.  
  
  @retval EFI_SUCCESS           The new event log entry was added.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.

**/
EFI_STATUS
EFIAPI
TcgDxeLogEventI (
  IN      TCG_DXE_DATA              *TcgData,
  IN      TCG_PCR_EVENT_HDR         *NewEventHdr,
  IN      UINT8                     *NewEventData
  )
{
  EFI_STATUS   Status;

  if (PcdGet8 (PcdTpmPlatformClass) == TCG_PLATFORM_TYPE_CLIENT) {
    TcgData->LastEvent = (UINT8*)(UINTN)TcgData->TcgClientAcpiTable->Lasa;
    Status = TcgCommLogEvent (
               &TcgData->LastEvent,
               &TcgData->EventLogSize,
               (UINTN)TcgData->TcgClientAcpiTable->Laml,
               NewEventHdr,
               NewEventData
               );
  } else {
    TcgData->LastEvent = (UINT8*)(UINTN)TcgData->TcgServerAcpiTable->Lasa;
    Status = TcgCommLogEvent (
               &TcgData->LastEvent,
               &TcgData->EventLogSize,
               (UINTN)TcgData->TcgServerAcpiTable->Laml,
               NewEventHdr,
               NewEventData
               );
  }
  
  if (Status == EFI_DEVICE_ERROR) {
    return EFI_DEVICE_ERROR;
  } else if (Status == EFI_OUT_OF_RESOURCES) {
    TcgData->EventLogTruncated = TRUE;
    return EFI_VOLUME_FULL;
  } else if (Status == EFI_SUCCESS) {
    TcgData->EventLogStarted = TRUE;
  }

  return Status;
}

/**
  This function get SHA1 digest from digest list.

  @param DigestList digest list
  @param Sha1Digest SHA1 digest

  @retval EFI_SUCCESS   Sha1Digest is found and returned.
  @retval EFI_NOT_FOUND Sha1Digest is not found.
**/
EFI_STATUS
Tpm2GetSha1FromDigestList (
  IN TPML_DIGEST_VALUES *DigestList,
  IN TPM_DIGEST         *Sha1Digest
  )
{
  UINTN  Index;

  for (Index = 0; Index < DigestList->count; Index++) {
    if (DigestList->digests[Index].hashAlg == TPM_ALG_SHA1) {
      CopyMem (
        Sha1Digest,
        DigestList->digests[Index].digest.sha1,
        SHA1_DIGEST_SIZE
        );
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  Do a hash operation on a data buffer, extend a specific TPM PCR with the hash result,
  and add an entry to the Event Log.

  @param[in]      TcgData       TCG_DXE_DATA structure.
  @param[in]      Flags         Bitmap providing additional information.
  @param[in]      HashData      Physical address of the start of the data buffer 
                                to be hashed, extended, and logged.
  @param[in]      HashDataLen   The length, in bytes, of the buffer referenced by HashData
  @param[in, out] NewEventHdr   Pointer to a TCG_PCR_EVENT_HDR data structure.  
  @param[in]      NewEventData  Pointer to the new event data.  

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  No enough memory to log the new event.
  @retval EFI_DEVICE_ERROR      The command was unsuccessful.

**/
EFI_STATUS
EFIAPI
TcgDxeHashLogExtendEventI (
  IN      TCG_DXE_DATA              *TcgData,
  IN      UINT64                    Flags,
  IN      UINT8                     *HashData,
  IN      UINT64                    HashDataLen,
  IN OUT  TCG_PCR_EVENT_HDR         *NewEventHdr,
  IN      UINT8                     *NewEventData
  )
{
  EFI_STATUS                        Status;
  TPML_DIGEST_VALUES                DigestList;
  EFI_TPL                           OldTpl;

  Status = HashAndExtend (
             NewEventHdr->PCRIndex,
             HashData,
             (UINTN)HashDataLen,
             &DigestList
             );
  if (!EFI_ERROR (Status)) {
    Status = Tpm2GetSha1FromDigestList (&DigestList, &NewEventHdr->Digest);
    if (!EFI_ERROR (Status)) {
      if ((Flags & TREE_EXTEND_ONLY) == 0) {
        //
        // Enter critical region
        //
        OldTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);
        Status = TcgDxeLogEventI (TcgData, NewEventHdr, NewEventData);
        gBS->RestoreTPL (OldTpl);
        //
        // Exit critical region
        //
      }
    }
  }

  return Status;
}

/**
  The EFI_TREE_PROTOCOL HashLogExtendEvent function call provides callers with
  an opportunity to extend and optionally log events without requiring
  knowledge of actual TPM commands. 
  The extend operation will occur even if this function cannot create an event
  log entry (e.g. due to the event log being full). 

  @param[in]  This               Indicates the calling context
  @param[in]  Flags              Bitmap providing additional information.
  @param[in]  DataToHash         Physical address of the start of the data buffer to be hashed. 
  @param[in]  DataToHashLen      The length in bytes of the buffer referenced by DataToHash.
  @param[in]  Event              Pointer to data buffer containing information about the event.

  @retval EFI_SUCCESS            Operation completed successfully.
  @retval EFI_DEVICE_ERROR       The command was unsuccessful.
  @retval EFI_VOLUME_FULL        The extend operation occurred, but the event could not be written to one or more event logs.
  @retval EFI_INVALID_PARAMETER  One or more of the parameters are incorrect.
  @retval EFI_UNSUPPORTED        The PE/COFF image type is not supported.
**/
EFI_STATUS
EFIAPI
TreeHashLogExtendEvent (
  IN EFI_TREE_PROTOCOL    *This,
  IN UINT64               Flags,
  IN EFI_PHYSICAL_ADDRESS DataToHash,
  IN UINT64               DataToHashLen,
  IN TrEE_EVENT           *Event
  )
{
  EFI_STATUS         Status;
  TCG_DXE_DATA       *TcgData;
  TCG_PCR_EVENT_HDR  NewEventHdr;
  TPML_DIGEST_VALUES DigestList;
  EFI_TPL            OldTpl;

  DEBUG ((EFI_D_ERROR, "TreeHashLogExtendEvent ...\n"));

  if ((This == NULL) || (DataToHash == 0) || (Event == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  TcgData = &mTcgDxeData;

  if (!TcgData->BsCap.TrEEPresentFlag) {
    return EFI_UNSUPPORTED;
  }

  if (Event->Size < Event->Header.HeaderSize + sizeof(UINT32)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Event->Header.PCRIndex > MAX_PCR_INDEX) {
    return EFI_INVALID_PARAMETER;
  }
  
  if ((Flags & TREE_EXTEND_ONLY) == 0) {
    if (TcgData->EventLogTruncated) {
      return EFI_VOLUME_FULL;
    }
  }

  if ((Flags & PE_COFF_IMAGE) != 0) {
    NewEventHdr.PCRIndex  = Event->Header.PCRIndex;
    NewEventHdr.EventType = Event->Header.EventType;
    NewEventHdr.EventSize = Event->Size - sizeof(UINT32) - Event->Header.HeaderSize;

    Status = MeasurePeImageAndExtend (
               NewEventHdr.PCRIndex,
               DataToHash,
               (UINTN)DataToHashLen,
               &DigestList
               );
    if (!EFI_ERROR (Status)) {
      Status = Tpm2GetSha1FromDigestList (&DigestList, &NewEventHdr.Digest);
      if (!EFI_ERROR (Status)) {
        if ((Flags & TREE_EXTEND_ONLY) == 0) {
          //
          // Enter critical region
          //
          OldTpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);

          Status = TcgDxeLogEventI (TcgData, &NewEventHdr, Event->Event);

          gBS->RestoreTPL (OldTpl);
          //
          // Exit critical region
          //
        }
      }
    }
  } else {
    NewEventHdr.PCRIndex  = Event->Header.PCRIndex;
    NewEventHdr.EventType = Event->Header.EventType;
    NewEventHdr.EventSize = Event->Size - sizeof(UINT32) - Event->Header.HeaderSize;

    Status = TcgDxeHashLogExtendEventI (
               TcgData,
               Flags,
               (UINT8 *) (UINTN) DataToHash,
               DataToHashLen,
               &NewEventHdr,
               Event->Event
               );
  }
  DEBUG ((EFI_D_ERROR, "TreeHashLogExtendEvent - %r\n", Status));
  return Status;
}

/**
  This service enables the sending of commands to the TrEE.

  @param[in]  This                     Indicates the calling context
  @param[in]  InputParameterBlockSize  Size of the TrEE input parameter block.
  @param[in]  InputParameterBlock      Pointer to the TrEE input parameter block.
  @param[in]  OutputParameterBlockSize Size of the TrEE output parameter block.
  @param[in]  OutputParameterBlock     Pointer to the TrEE output parameter block.

  @retval EFI_SUCCESS            The command byte stream was successfully sent to the device and a response was successfully received.
  @retval EFI_DEVICE_ERROR       The command was not successfully sent to the device or a response was not successfully received from the device.
  @retval EFI_INVALID_PARAMETER  One or more of the parameters are incorrect.
  @retval EFI_BUFFER_TOO_SMALL   The output parameter block is too small. 
**/
EFI_STATUS
EFIAPI
TreeSubmitCommand (
  IN EFI_TREE_PROTOCOL *This,
  IN UINT32            InputParameterBlockSize,
  IN UINT8             *InputParameterBlock,
  IN UINT32            OutputParameterBlockSize,
  IN UINT8             *OutputParameterBlock
  )
{
  TCG_DXE_DATA  *TcgData;
  EFI_STATUS    Status;

  DEBUG ((EFI_D_ERROR, "TreeSubmitCommand ...\n"));

  if ((This == NULL) ||
      (InputParameterBlockSize == 0) || (InputParameterBlock == NULL) ||
      (OutputParameterBlockSize == 0) || (OutputParameterBlock == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  TcgData = &mTcgDxeData;

  if (!TcgData->BsCap.TrEEPresentFlag) {
    return EFI_UNSUPPORTED;
  }

  if (InputParameterBlockSize >= TcgData->BsCap.MaxCommandSize) {
    return EFI_INVALID_PARAMETER;
  }
  if (OutputParameterBlockSize >= TcgData->BsCap.MaxResponseSize) {
    return EFI_INVALID_PARAMETER;
  }

  Status = Tpm2SubmitCommand (
             InputParameterBlockSize,
             InputParameterBlock,
             &OutputParameterBlockSize,
             OutputParameterBlock
             );
  DEBUG ((EFI_D_ERROR, "TreeSubmitCommand - %r\n", Status));
  return Status;
}


EFI_TREE_PROTOCOL mTreeProtocol = {
    TreeGetCapability,
    TreeGetEventLog,
    TreeHashLogExtendEvent,
    TreeSubmitCommand
};

/**
  Initialize the Event Log and log events passed from the PEI phase.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.

**/
EFI_STATUS
EFIAPI
SetupEventLog (
  VOID
  )
{
  EFI_STATUS            Status;
  TCG_PCR_EVENT         *TcgEvent;
  EFI_PEI_HOB_POINTERS  GuidHob;
  EFI_PHYSICAL_ADDRESS  Lasa;
  
  if (PcdGet8 (PcdTpmPlatformClass) == TCG_PLATFORM_TYPE_CLIENT) {
    Lasa = mTcgClientAcpiTemplate.Lasa;
  
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiACPIMemoryNVS,
                    EFI_SIZE_TO_PAGES (EFI_TCG_LOG_AREA_SIZE),
                    &Lasa
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    mTcgClientAcpiTemplate.Lasa = Lasa;
    //
    // To initialize them as 0xFF is recommended 
    // because the OS can know the last entry for that.
    //
    SetMem ((VOID *)(UINTN)mTcgClientAcpiTemplate.Lasa, EFI_TCG_LOG_AREA_SIZE, 0xFF);
    mTcgClientAcpiTemplate.Laml = EFI_TCG_LOG_AREA_SIZE;
  
  } else {
    Lasa = mTcgServerAcpiTemplate.Lasa;
  
    Status = gBS->AllocatePages (
                    AllocateMaxAddress,
                    EfiACPIMemoryNVS,
                    EFI_SIZE_TO_PAGES (EFI_TCG_LOG_AREA_SIZE),
                    &Lasa
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }
    mTcgServerAcpiTemplate.Lasa = Lasa;
    //
    // To initialize them as 0xFF is recommended 
    // because the OS can know the last entry for that.
    //
    SetMem ((VOID *)(UINTN)mTcgServerAcpiTemplate.Lasa, EFI_TCG_LOG_AREA_SIZE, 0xFF);
    mTcgServerAcpiTemplate.Laml = EFI_TCG_LOG_AREA_SIZE;
  }

  GuidHob.Raw = GetHobList ();
  while (!EFI_ERROR (Status) && 
         (GuidHob.Raw = GetNextGuidHob (&gTcgEventEntryHobGuid, GuidHob.Raw)) != NULL) {
    TcgEvent    = GET_GUID_HOB_DATA (GuidHob.Guid);
    GuidHob.Raw = GET_NEXT_HOB (GuidHob);
    Status = TcgDxeLogEventI (
               &mTcgDxeData,
               (TCG_PCR_EVENT_HDR*)TcgEvent,
               TcgEvent->Event
               );
  }

  return Status;
}

/**
  Measure and log an action string, and extend the measurement result into PCR[5].

  @param[in] String           A specific string that indicates an Action event.  
  
  @retval EFI_SUCCESS         Operation completed successfully.
  @retval EFI_DEVICE_ERROR    The operation was unsuccessful.

**/
EFI_STATUS
EFIAPI
TcgMeasureAction (
  IN      CHAR8                     *String
  )
{
  TCG_PCR_EVENT_HDR                 TcgEvent;

  TcgEvent.PCRIndex  = 5;
  TcgEvent.EventType = EV_EFI_ACTION;
  TcgEvent.EventSize = (UINT32)AsciiStrLen (String);
  return TcgDxeHashLogExtendEventI (
           &mTcgDxeData,
           0,
           (UINT8*)String,
           TcgEvent.EventSize,
           &TcgEvent,
           (UINT8 *) String
           );
}

/**
  Measure and log EFI handoff tables, and extend the measurement result into PCR[1].

  @retval EFI_SUCCESS         Operation completed successfully.
  @retval EFI_DEVICE_ERROR    The operation was unsuccessful.

**/
EFI_STATUS
EFIAPI
MeasureHandoffTables (
  VOID
  )
{
  EFI_STATUS                        Status;
  SMBIOS_TABLE_ENTRY_POINT          *SmbiosTable;
  TCG_PCR_EVENT_HDR                 TcgEvent;
  EFI_HANDOFF_TABLE_POINTERS        HandoffTables;

  Status = EfiGetSystemConfigurationTable (
             &gEfiSmbiosTableGuid,
             (VOID **) &SmbiosTable
             );

  if (!EFI_ERROR (Status)) {
    ASSERT (SmbiosTable != NULL);

    TcgEvent.PCRIndex  = 1;
    TcgEvent.EventType = EV_EFI_HANDOFF_TABLES;
    TcgEvent.EventSize = sizeof (HandoffTables);

    HandoffTables.NumberOfTables = 1;
    HandoffTables.TableEntry[0].VendorGuid  = gEfiSmbiosTableGuid;
    HandoffTables.TableEntry[0].VendorTable = SmbiosTable;

    DEBUG ((DEBUG_INFO, "The Smbios Table starts at: 0x%x\n", SmbiosTable->TableAddress));
    DEBUG ((DEBUG_INFO, "The Smbios Table size: 0x%x\n", SmbiosTable->TableLength));

    Status = TcgDxeHashLogExtendEventI (
               &mTcgDxeData,
               0,
               (UINT8*)(UINTN)SmbiosTable->TableAddress,
               SmbiosTable->TableLength,
               &TcgEvent,
               (UINT8*)&HandoffTables
               );
  }

  return Status;
}

/**
  Measure and log Separator event, and extend the measurement result into a specific PCR.

  @param[in] PCRIndex         PCR index.  

  @retval EFI_SUCCESS         Operation completed successfully.
  @retval EFI_DEVICE_ERROR    The operation was unsuccessful.

**/
EFI_STATUS
EFIAPI
MeasureSeparatorEvent (
  IN      TPM_PCRINDEX              PCRIndex
  )
{
  TCG_PCR_EVENT_HDR                 TcgEvent;
  UINT32                            EventData;

  DEBUG ((EFI_D_ERROR, "MeasureSeparatorEvent Pcr - %x\n", PCRIndex));

  EventData = 0;
  TcgEvent.PCRIndex  = PCRIndex;
  TcgEvent.EventType = EV_SEPARATOR;
  TcgEvent.EventSize = (UINT32)sizeof (EventData);
  return TcgDxeHashLogExtendEventI (
           &mTcgDxeData,
           0,
           (UINT8 *)&EventData,
           sizeof (EventData),
           &TcgEvent,
           (UINT8 *)&EventData
           );
}

/**
  Measure and log an EFI variable, and extend the measurement result into a specific PCR.

  @param[in]  PCRIndex          PCR Index.  
  @param[in]  EventType         Event type.  
  @param[in]  VarName           A Null-terminated string that is the name of the vendor's variable.
  @param[in]  VendorGuid        A unique identifier for the vendor.
  @param[in]  VarData           The content of the variable data.  
  @param[in]  VarSize           The size of the variable data.  
 
  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.

**/
EFI_STATUS
EFIAPI
MeasureVariable (
  IN      TPM_PCRINDEX              PCRIndex,
  IN      TCG_EVENTTYPE             EventType,
  IN      CHAR16                    *VarName,
  IN      EFI_GUID                  *VendorGuid,
  IN      VOID                      *VarData,
  IN      UINTN                     VarSize
  )
{
  EFI_STATUS                        Status;
  TCG_PCR_EVENT_HDR                 TcgEvent;
  UINTN                             VarNameLength;
  EFI_VARIABLE_DATA_TREE            *VarLog;

  ASSERT ((VarSize == 0 && VarData == NULL) || (VarSize != 0 && VarData != NULL));

  DEBUG ((EFI_D_ERROR, "TrEEDxe: MeasureVariable (Pcr - %x, EventType - %x, ", (UINTN)PCRIndex, (UINTN)EventType));
  DEBUG ((EFI_D_ERROR, "VariableName - %s, VendorGuid - %g)\n", VarName, VendorGuid));

  VarNameLength      = StrLen (VarName);
  TcgEvent.PCRIndex  = PCRIndex;
  TcgEvent.EventType = EventType;
  TcgEvent.EventSize = (UINT32)(sizeof (*VarLog) + VarNameLength * sizeof (*VarName) + VarSize
                        - sizeof (VarLog->UnicodeName) - sizeof (VarLog->VariableData));

  VarLog = (EFI_VARIABLE_DATA_TREE*)AllocatePool (TcgEvent.EventSize);
  if (VarLog == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  VarLog->VariableName       = *VendorGuid;
  VarLog->UnicodeNameLength  = VarNameLength;
  VarLog->VariableDataLength = VarSize;
  CopyMem (
     VarLog->UnicodeName,
     VarName,
     VarNameLength * sizeof (*VarName)
     );
  if (VarSize != 0) {
    CopyMem (
       (CHAR16 *)VarLog->UnicodeName + VarNameLength,
       VarData,
       VarSize
       );
  }

  if (EventType == EV_EFI_VARIABLE_DRIVER_CONFIG) {
    //
    // Digest is the event data (EFI_VARIABLE_DATA_TREE)
    //
    Status = TcgDxeHashLogExtendEventI (
               &mTcgDxeData,
               0,
               (UINT8*)VarLog,
               TcgEvent.EventSize,
               &TcgEvent,
               (UINT8*)VarLog
               );
  } else {
    Status = TcgDxeHashLogExtendEventI (
               &mTcgDxeData,
               0,
               (UINT8*)VarData,
               VarSize,
               &TcgEvent,
               (UINT8*)VarLog
               );
  }
  FreePool (VarLog);
  return Status;
}

/**
  Returns the status whether get the variable success. The function retrieves 
  variable  through the UEFI Runtime Service GetVariable().  The 
  returned buffer is allocated using AllocatePool().  The caller is responsible
  for freeing this buffer with FreePool().

  If Name  is NULL, then ASSERT().
  If Guid  is NULL, then ASSERT().
  If Value is NULL, then ASSERT().

  @param[in]  Name  The pointer to a Null-terminated Unicode string.
  @param[in]  Guid  The pointer to an EFI_GUID structure
  @param[out] Value The buffer point saved the variable info.
  @param[out] Size  The buffer size of the variable.

  @return EFI_OUT_OF_RESOURCES      Allocate buffer failed.
  @return EFI_SUCCESS               Find the specified variable.
  @return Others Errors             Return errors from call to gRT->GetVariable.

**/
EFI_STATUS
EFIAPI
GetVariable3 (
  IN CONST CHAR16    *Name,
  IN CONST EFI_GUID  *Guid,
  OUT VOID           **Value,
  OUT UINTN          *Size OPTIONAL
  )
{
  EFI_STATUS  Status;
  UINTN       BufferSize;

  ASSERT (Name != NULL && Guid != NULL && Value != NULL);

  //
  // Try to get the variable size.
  //
  BufferSize = 0;
  *Value     = NULL;
  if (Size != NULL) {
    *Size  = 0;
  }
  
  Status = gRT->GetVariable ((CHAR16 *) Name, (EFI_GUID *) Guid, NULL, &BufferSize, *Value);
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return Status;
  }

  //
  // Allocate buffer to get the variable.
  //
  *Value = AllocatePool (BufferSize);
  ASSERT (*Value != NULL);
  if (*Value == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Get the variable data.
  //
  Status = gRT->GetVariable ((CHAR16 *) Name, (EFI_GUID *) Guid, NULL, &BufferSize, *Value);
  if (EFI_ERROR (Status)) {
    FreePool(*Value);
    *Value = NULL;
  }

  if (Size != NULL) {
    *Size = BufferSize;
  }

  return Status;
}


/**
  Read then Measure and log an EFI variable, and extend the measurement result into a specific PCR.

  @param[in]  PCRIndex          PCR Index.  
  @param[in]  EventType         Event type.  
  @param[in]   VarName          A Null-terminated string that is the name of the vendor's variable.
  @param[in]   VendorGuid       A unique identifier for the vendor.
  @param[out]  VarSize          The size of the variable data.  
  @param[out]  VarData          Pointer to the content of the variable.  
 
  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.

**/
EFI_STATUS
EFIAPI
ReadAndMeasureVariable (
  IN      TPM_PCRINDEX              PCRIndex,
  IN      TCG_EVENTTYPE             EventType,
  IN      CHAR16                    *VarName,
  IN      EFI_GUID                  *VendorGuid,
  OUT     UINTN                     *VarSize,
  OUT     VOID                      **VarData
  )
{
  EFI_STATUS                        Status;

  Status = GetVariable3 (VarName, VendorGuid, VarData, VarSize);
  if (EventType == EV_EFI_VARIABLE_DRIVER_CONFIG) {
    if (EFI_ERROR (Status)) {
      //
      // It is valid case, so we need handle it.
      //
      *VarData = NULL;
      *VarSize = 0;
    }
  } else {
    if (EFI_ERROR (Status)) {
      return Status;
    }
    ASSERT (*VarData != NULL);
  }

  Status = MeasureVariable (
             PCRIndex,
             EventType,
             VarName,
             VendorGuid,
             *VarData,
             *VarSize
             );
  return Status;
}

/**
  Read then Measure and log an EFI boot variable, and extend the measurement result into PCR[5].

  @param[in]   VarName          A Null-terminated string that is the name of the vendor's variable.
  @param[in]   VendorGuid       A unique identifier for the vendor.
  @param[out]  VarSize          The size of the variable data.  
  @param[out]  VarData          Pointer to the content of the variable.  
 
  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.

**/
EFI_STATUS
EFIAPI
ReadAndMeasureBootVariable (
  IN      CHAR16                    *VarName,
  IN      EFI_GUID                  *VendorGuid,
  OUT     UINTN                     *VarSize,
  OUT     VOID                      **VarData
  )
{
  return ReadAndMeasureVariable (
           5,
           EV_EFI_VARIABLE_BOOT,
           VarName,
           VendorGuid,
           VarSize,
           VarData
           );
}

/**
  Read then Measure and log an EFI Secure variable, and extend the measurement result into PCR[7].

  @param[in]   VarName          A Null-terminated string that is the name of the vendor's variable.
  @param[in]   VendorGuid       A unique identifier for the vendor.
  @param[out]  VarSize          The size of the variable data.  
  @param[out]  VarData          Pointer to the content of the variable.  
 
  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.

**/
EFI_STATUS
EFIAPI
ReadAndMeasureSecureVariable (
  IN      CHAR16                    *VarName,
  IN      EFI_GUID                  *VendorGuid,
  OUT     UINTN                     *VarSize,
  OUT     VOID                      **VarData
  )
{
  return ReadAndMeasureVariable (
           7,
           EV_EFI_VARIABLE_DRIVER_CONFIG,
           VarName,
           VendorGuid,
           VarSize,
           VarData
           );
}

/**
  Measure and log all EFI boot variables, and extend the measurement result into a specific PCR.

  The EFI boot variables are BootOrder and Boot#### variables.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.

**/
EFI_STATUS
EFIAPI
MeasureAllBootVariables (
  VOID
  )
{
  EFI_STATUS                        Status;
  UINT16                            *BootOrder;
  UINTN                             BootCount;
  UINTN                             Index;
  VOID                              *BootVarData;
  UINTN                             Size;

  Status = ReadAndMeasureBootVariable (
             mBootVarName,
             &gEfiGlobalVariableGuid,
             &BootCount,
             (VOID **) &BootOrder
             );
  if (Status == EFI_NOT_FOUND) {
    return EFI_SUCCESS;
  }
  ASSERT (BootOrder != NULL);

  if (EFI_ERROR (Status)) {
    FreePool (BootOrder);
    return Status;
  }

  BootCount /= sizeof (*BootOrder);
  for (Index = 0; Index < BootCount; Index++) {
    UnicodeSPrint (mBootVarName, sizeof (mBootVarName), L"Boot%04x", BootOrder[Index]);
    Status = ReadAndMeasureBootVariable (
               mBootVarName,
               &gEfiGlobalVariableGuid,
               &Size,
               &BootVarData
               );
    if (!EFI_ERROR (Status)) {
      FreePool (BootVarData);
    }
  }

  FreePool (BootOrder);
  return EFI_SUCCESS;
}

/**
  Measure and log all EFI Secure variables, and extend the measurement result into a specific PCR.

  The EFI boot variables are BootOrder and Boot#### variables.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.

**/
EFI_STATUS
EFIAPI
MeasureAllSecureVariables (
  VOID
  )
{
  EFI_STATUS                        Status;
  VOID                              *Data;
  UINTN                             DataSize;
  UINTN                             Index;

  Status = EFI_NOT_FOUND;
  for (Index = 0; Index < sizeof(mVariableType)/sizeof(mVariableType[0]); Index++) {
    Status = ReadAndMeasureSecureVariable (
               mVariableType[Index].VariableName,
               mVariableType[Index].VendorGuid,
               &DataSize,
               &Data
               );
    if (!EFI_ERROR (Status)) {
      if (Data != NULL) {
        FreePool (Data);
      }
    }
  }

  return EFI_SUCCESS;
}

/**
  Measure and log launch of FirmwareDebugger, and extend the measurement result into a specific PCR.

  @retval EFI_SUCCESS           Operation completed successfully.
  @retval EFI_OUT_OF_RESOURCES  Out of memory.
  @retval EFI_DEVICE_ERROR      The operation was unsuccessful.

**/
EFI_STATUS
EFIAPI
MeasureLaunchOfFirmwareDebugger (
  VOID
  )
{
  TCG_PCR_EVENT_HDR                 TcgEvent;

  TcgEvent.PCRIndex  = 7;
  TcgEvent.EventType = EV_EFI_ACTION;
  TcgEvent.EventSize = sizeof(FIRMWARE_DEBUGGER_EVENT_STRING) - 1;
  return TcgDxeHashLogExtendEventI (
           &mTcgDxeData,
           0,
           (UINT8 *)FIRMWARE_DEBUGGER_EVENT_STRING,
           sizeof(FIRMWARE_DEBUGGER_EVENT_STRING) - 1,
           &TcgEvent,
           (UINT8 *)FIRMWARE_DEBUGGER_EVENT_STRING
           );
}

/**
  Measure and log all Secure Boot Policy, and extend the measurement result into a specific PCR.

  Platform firmware adhering to the policy must therefore measure the following values into PCR[7]: (in order listed)
   - The contents of the SecureBoot variable
   - The contents of the PK variable
   - The contents of the KEK variable
   - The contents of the EFI_IMAGE_SECURITY_DATABASE variable
   - The contents of the EFI_IMAGE_SECURITY_DATABASE1 variable
   - Separator
   - Entries in the EFI_IMAGE_SECURITY_DATABASE that are used to validate EFI Drivers or EFI Boot Applications in the boot path

  NOTE: Because of the above, UEFI variables PK, KEK, EFI_IMAGE_SECURITY_DATABASE,
  EFI_IMAGE_SECURITY_DATABASE1 and SecureBoot SHALL NOT be measured into PCR[3].

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context
**/
VOID
EFIAPI
MeasureSecureBootPolicy (
  IN EFI_EVENT                      Event,
  IN VOID                           *Context
  )
{
  EFI_STATUS  Status;
  VOID        *Protocol;

  Status = gBS->LocateProtocol (&gEfiVariableWriteArchProtocolGuid, NULL, (VOID **)&Protocol);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "MeasureSecureBootPolicy - FALSE\n"));
    return;
  }
  DEBUG ((EFI_D_ERROR, "MeasureSecureBootPolicy - TRUE\n"));

  if (PcdGetBool (PcdFirmwareDebuggerInitialized)) {
    Status = MeasureLaunchOfFirmwareDebugger ();
    DEBUG ((EFI_D_ERROR, "MeasureLaunchOfFirmwareDebugger - %r\n", Status));
  }

  Status = MeasureAllSecureVariables ();
  DEBUG ((EFI_D_ERROR, "MeasureAllSecureVariables - %r\n", Status));

  Status = MeasureSeparatorEvent (7);
  DEBUG ((EFI_D_ERROR, "MeasureSeparatorEvent - %r\n", Status));

  return ;
}

/**
  Ready to Boot Event notification handler.

  Sequence of OS boot events is measured in this event notification handler.

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context

**/
VOID
EFIAPI
OnReadyToBoot (
  IN      EFI_EVENT                 Event,
  IN      VOID                      *Context
  )
{
  EFI_STATUS                        Status;
  TPM_PCRINDEX                      PcrIndex;

  if (mBootAttempts == 0) {

    //
    // Measure handoff tables.
    //
    Status = MeasureHandoffTables ();
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "HOBs not Measured. Error!\n"));
    }

    //
    // Measure BootOrder & Boot#### variables.
    //
    Status = MeasureAllBootVariables ();
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Boot Variables not Measured. Error!\n"));
    }

    //
    // 1. This is the first boot attempt.
    //
    Status = TcgMeasureAction (
               EFI_CALLING_EFI_APPLICATION
               );
    ASSERT_EFI_ERROR (Status);

    //
    // 2. Draw a line between pre-boot env and entering post-boot env.
    //
    for (PcrIndex = 0; PcrIndex < 7; PcrIndex++) {
      Status = MeasureSeparatorEvent (PcrIndex);
      ASSERT_EFI_ERROR (Status);
    }

    //
    // 3. Measure GPT. It would be done in SAP driver.
    //

    //
    // 4. Measure PE/COFF OS loader. It would be done in SAP driver.
    //

    //
    // 5. Read & Measure variable. BootOrder already measured.
    //
  } else {
    //
    // 6. Not first attempt, meaning a return from last attempt
    //
    Status = TcgMeasureAction (
               EFI_RETURNING_FROM_EFI_APPLICATOIN
               );
    ASSERT_EFI_ERROR (Status);
  }

  DEBUG ((EFI_D_INFO, "TPM2 TrEEDxe Measure Data when ReadyToBoot\n"));
  //
  // Increase boot attempt counter.
  //
  mBootAttempts++;
}

/**
  Install TCG ACPI Table when ACPI Table Protocol is available.

  A system's firmware uses an ACPI table to identify the system's TCG capabilities 
  to the Post-Boot environment. The information in this ACPI table is not guaranteed 
  to be valid until the Host Platform transitions from pre-boot state to post-boot state.  

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context
**/
VOID
EFIAPI
InstallAcpiTable (
  IN EFI_EVENT                      Event,
  IN VOID                           *Context
  )
{
  UINTN                             TableKey;
  EFI_STATUS                        Status;
  EFI_ACPI_TABLE_PROTOCOL           *AcpiTable;

  Status = gBS->LocateProtocol (&gEfiAcpiTableProtocolGuid, NULL, (VOID **)&AcpiTable);
  if (EFI_ERROR (Status)) {
    return;
  }

  if (PcdGet8 (PcdTpmPlatformClass) == TCG_PLATFORM_TYPE_CLIENT) {
    Status = AcpiTable->InstallAcpiTable (
                            AcpiTable,
                            &mTcgClientAcpiTemplate,
                            sizeof (mTcgClientAcpiTemplate),
                            &TableKey
                            );
  } else {
    Status = AcpiTable->InstallAcpiTable (
                            AcpiTable,
                            &mTcgServerAcpiTemplate,
                            sizeof (mTcgServerAcpiTemplate),
                            &TableKey
                            );
  }
  ASSERT_EFI_ERROR (Status);
}

/**
  Exit Boot Services Event notification handler.

  Measure invocation and success of ExitBootServices.

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context

**/
VOID
EFIAPI
OnExitBootServices (
  IN      EFI_EVENT                 Event,
  IN      VOID                      *Context
  )
{
  EFI_STATUS    Status;

  //
  // Measure invocation of ExitBootServices,
  //
  Status = TcgMeasureAction (
             EFI_EXIT_BOOT_SERVICES_INVOCATION
             );
  ASSERT_EFI_ERROR (Status);

  //
  // Measure success of ExitBootServices
  //
  Status = TcgMeasureAction (
             EFI_EXIT_BOOT_SERVICES_SUCCEEDED
             );
  ASSERT_EFI_ERROR (Status);
}

/**
  Exit Boot Services Failed Event notification handler.

  Measure Failure of ExitBootServices.

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context

**/
VOID
EFIAPI
OnExitBootServicesFailed (
  IN      EFI_EVENT                 Event,
  IN      VOID                      *Context
  )
{
  EFI_STATUS    Status;

  //
  // Measure Failure of ExitBootServices,
  //
  Status = TcgMeasureAction (
             EFI_EXIT_BOOT_SERVICES_FAILED
             );
  ASSERT_EFI_ERROR (Status);

}

/**
  The function install TrEE protocol.
  
  @retval EFI_SUCCESS     TrEE protocol is installed.
  @retval other           Some error occurs.
**/
EFI_STATUS
InstallTrEE (
  VOID
  )
{
  EFI_STATUS        Status;
  EFI_HANDLE        Handle;

  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiTrEEProtocolGuid,
                  &mTreeProtocol,
                  NULL
                  );
  return Status;
}

/**
  The driver's entry point. It publishes EFI TrEE Protocol.

  @param[in] ImageHandle  The firmware allocated handle for the EFI image.  
  @param[in] SystemTable  A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS     The entry point is executed successfully.
  @retval other           Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
DriverEntry (
  IN    EFI_HANDLE                  ImageHandle,
  IN    EFI_SYSTEM_TABLE            *SystemTable
  )
{
  EFI_STATUS                        Status;
  EFI_EVENT                         Event;
  VOID                              *Registration;
  UINT32                            MaxCommandSize;
  UINT32                            MaxResponseSize;
  TPML_ALG_PROPERTY                 AlgList;
  UINTN                             Index;

  UINT16                            NewAuthorizationSize;
//  UINT8                             NewAuthorization[MAX_NEW_AUTHORIZATION_SIZE];

  if (CompareGuid (PcdGetPtr(PcdTpmInstanceGuid), &gEfiTpmDeviceInstanceNoneGuid) ||
      CompareGuid (PcdGetPtr(PcdTpmInstanceGuid), &gEfiTpmDeviceInstanceTpm12Guid)){
    DEBUG ((EFI_D_ERROR, "No TPM2 instance required!\n"));
    return EFI_UNSUPPORTED;
  }

  Status = Tpm2RequestUseTpm ();
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "TPM not detected!\n"));
    return Status;
  }

  //
  // Fill information
  //
  Status = Tpm2GetCapabilityManufactureID (&mTcgDxeData.BsCap.ManufacturerID);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Tpm2GetCapabilityManufactureID fail!\n"));
  } else {
    DEBUG ((EFI_D_ERROR, "Tpm2GetCapabilityManufactureID - %08x\n", mTcgDxeData.BsCap.ManufacturerID));
  }

  Status = Tpm2GetCapabilityMaxCommandResponseSize (&MaxCommandSize, &MaxResponseSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Tpm2GetCapabilityMaxCommandResponseSize fail!\n"));
  } else {
    mTcgDxeData.BsCap.MaxCommandSize  = (UINT16)MaxCommandSize;
    mTcgDxeData.BsCap.MaxResponseSize = (UINT16)MaxResponseSize;
    DEBUG ((EFI_D_ERROR, "Tpm2GetCapabilityMaxCommandResponseSize - %08x, %08x\n", MaxCommandSize, MaxResponseSize));
  }

  NewAuthorizationSize = SHA1_DIGEST_SIZE;
  Status = Tpm2GetCapabilitySupportedAlg (&AlgList);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Tpm2GetCapabilitySupportedAlg fail!\n"));
  } else {
    DEBUG ((EFI_D_ERROR, "Tpm2GetCapabilitySupportedAlg - %08x\n", AlgList.count));
    for (Index = 0; Index < AlgList.count; Index++) {
      DEBUG ((EFI_D_ERROR, "alg - %x\n", AlgList.algProperties[Index].alg));
      switch (AlgList.algProperties[Index].alg) {
      case TPM_ALG_SHA1:
        mTcgDxeData.BsCap.HashAlgorithmBitmap |= TREE_BOOT_HASH_ALG_SHA1;
        NewAuthorizationSize = SHA1_DIGEST_SIZE;
        break;
      case TPM_ALG_SHA256:
        mTcgDxeData.BsCap.HashAlgorithmBitmap |= TREE_BOOT_HASH_ALG_SHA256;
        NewAuthorizationSize = SHA256_DIGEST_SIZE;
        break;
      case TPM_ALG_SHA384:
        mTcgDxeData.BsCap.HashAlgorithmBitmap |= TREE_BOOT_HASH_ALG_SHA384;
        NewAuthorizationSize = SHA384_DIGEST_SIZE;
        break;
      case TPM_ALG_SHA512:
        mTcgDxeData.BsCap.HashAlgorithmBitmap |= TREE_BOOT_HASH_ALG_SHA512;
        NewAuthorizationSize = SHA512_DIGEST_SIZE;
        break;
      case TPM_ALG_WHIRLPOOL512:
        // TBD: Spec not define TREE_BOOT_HASH_ALG_WHIRLPOOL512
        NewAuthorizationSize = WHIRLPOOL512_DIGEST_SIZE;
        break;
      case TPM_ALG_SM3_256:
        // TBD: Spec not define TREE_BOOT_HASH_ALG_SM3_256
        NewAuthorizationSize = SM3_256_DIGEST_SIZE;
        break;
      }
    }
  }
/*
  //
  // RevokeTrust if needed
  //
  if (PcdGetBool (PcdRevokeTrust)) {
    RandomSeed (NULL, 0);
    RandomBytes (NewAuthorization, sizeof(NewAuthorization));
    Status = Tpm2HierarchyChangeAuth (NewAuthorizationSize, NewAuthorization);
    if (!EFI_ERROR(Status)){
      DEBUG((EFI_D_INFO, "Tpm2HierarchyChangeAuth executed successfully!\n"));
      Status = Tpm2ChangeEPS (NewAuthorizationSize, NewAuthorization);
      if (EFI_ERROR(Status)){
        DEBUG((EFI_D_INFO, "Tpm2ChangeEPS failed!\n"));
      } else {
        DEBUG((EFI_D_INFO, "Tpm2ChangeEPS executed successfully!\n"));
      }
    } else {
      DEBUG((EFI_D_INFO, "Tpm2HierarchyChangeAuth failed!\n"));
    }
    PcdSetBool (PcdRevokeTrust, FALSE);
  }
*/
  if (mTcgDxeData.BsCap.TrEEPresentFlag) {
    //
    // Setup the log area and copy event log from hob list to it
    //
    Status = SetupEventLog ();
    ASSERT_EFI_ERROR (Status);

    //
    // Install ACPI Table
    //
    EfiCreateProtocolNotifyEvent (&gEfiAcpiTableProtocolGuid, TPL_CALLBACK, InstallAcpiTable, NULL, &Registration);

    //
    // Measure handoff tables, Boot#### variables etc.
    //
    Status = EfiCreateEventReadyToBootEx (
               TPL_CALLBACK,
               OnReadyToBoot,
               NULL,
               &Event
               );

    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    OnExitBootServices,
                    NULL,
                    &gEfiEventExitBootServicesGuid,
                    &Event
                    );

    //
    // Measure Exit Boot Service failed 
    //
/*    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    OnExitBootServicesFailed,
                    NULL,
                    &gEventExitBootServicesFailedGuid,
                    &Event
                    );
*/
    //
    // Create event callback, because we need access variable on SecureBootPolicyVariable
    // We should use VariableWriteArch instead of VariableArch, because Variable driver
    // may update SecureBoot value based on last setting.
    //
    {
      EFI_EVENT MyEvent;
      MyEvent = EfiCreateProtocolNotifyEvent (&gEfiVariableWriteArchProtocolGuid, TPL_CALLBACK, MeasureSecureBootPolicy, NULL, &Registration);
      DEBUG ((EFI_D_ERROR, "EfiCreateProtocolNotifyEvent - %x\n", MyEvent));
    }
  }

  //
  // Install TrEEProtocol
  //
  Status = InstallTrEE ();
  DEBUG ((EFI_D_ERROR, "InstallTrEE - %r\n", Status));

  return Status;
}
