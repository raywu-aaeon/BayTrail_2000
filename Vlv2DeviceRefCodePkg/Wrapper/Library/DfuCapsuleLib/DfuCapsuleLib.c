/** @file
  Common driver entry point implementation.
  
  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#include <Uefi.h>
#include <PiDxe.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DevicePath.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/LoadedImage.h>
#include <Guid/Capsule.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DFUCapsuleLib.h>

EFI_HANDLE        ImageHandle;
EFI_SYSTEM_TABLE  *SystemTable;
DFU_UPDATE_STATUS DFUStatus;

EFI_STATUS
EFIAPI
LoadRawFile(
  void ** pFileBuffer,
  UINT64 *pFileSize,
  EFI_GUID DFU_GUID
)
{
  EFI_STATUS  Status;
  EFI_LOADED_IMAGE_PROTOCOL  *LoadedImageProtocol;
  EFI_FIRMWARE_VOLUME2_PROTOCOL  *FwVolProtocol;
#if 0
 // EFI_FIRMWARE_VOLUME2_PROTOCOL         *DataFwVolProtocol;
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH  *FwVolFilePathNode; 
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH  *AlignedDevPathNode;
  EFI_DEVICE_PATH_PROTOCOL  *FilePathNode;
  EFI_SECTION_TYPE  SectionType;
#endif
  UINT8  *FileBuffer;
  UINTN  FileBufferSize;
  EFI_FV_FILETYPE  FileType;
  EFI_FV_FILE_ATTRIBUTES  Attrib;
  UINT32  AuthenticationStatus;
  Status  = gBS->OpenProtocol (
                             ImageHandle,
                             &gEfiLoadedImageProtocolGuid,
                             (VOID **)&LoadedImageProtocol,
                             ImageHandle,
                             NULL,
                             EFI_OPEN_PROTOCOL_GET_PROTOCOL
                             );
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR,"LoadRawFile: OpenProtocol failed\n"));
    return Status;
  }
  //
  // Get the firmware volume protocol where this file resides
  //
  Status  = gBS->HandleProtocol (
                             LoadedImageProtocol->DeviceHandle,
                             &gEfiFirmwareVolume2ProtocolGuid,
                             (VOID **)  &FwVolProtocol
                             );
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR,"LoadRawFile: HandleProtocol failed\n"));
    return Status;
  }
  
#if 0
  //
  // Shall do some extra check to see if it is really contained in the FV?
  // Should be able to find the section of this driver in the the FV.
  //
  FilePathNode  = LoadedImageProtocol->FilePath;
  FwVolFilePathNode = NULL;
  while (!IsDevicePathEnd (FilePathNode)) {
    if (EfiGetNameGuidFromFwVolDevicePathNode ((MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *)FilePathNode)!= NULL) {
      FwVolFilePathNode = (MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *) FilePathNode;
      break;
    }
    FilePathNode    = NextDevicePathNode (FilePathNode);
  }

  if (FwVolFilePathNode != NULL) {
    AlignedDevPathNode = AllocateCopyPool (DevicePathNodeLength (FwVolFilePathNode), FwVolFilePathNode);
    SectionType     = EFI_SECTION_PE32;
    FileBuffer      = NULL;
    FileBufferSize  = 0;
    Status          = FwVolProtocol->ReadSection (
                                       FwVolProtocol,
                                       &(AlignedDevPathNode->FvFileName),
                                       SectionType,
                                       0,
                                       (VOID **) &FileBuffer,
                                       &FileBufferSize,
                                       &AuthenticationStatus
                                       );
    if (EFI_ERROR (Status)) {
      DEBUG((EFI_D_ERROR,"LoadRawFile:: ReadSection failed\n"));
      FreePool (AlignedDevPathNode);
      return Status;
    }

    if (FileBuffer != NULL) {
      FreePool(FileBuffer);
      FileBuffer = NULL;
    }

    //
    // Check the NameGuid of the udpate driver so that it can be
    // used as the CallerId in fault tolerant write protocol
    //
    if (!CompareGuid (&gEfiCallerIdGuid, &(AlignedDevPathNode->FvFileName))) {
      FreePool (AlignedDevPathNode);
      return EFI_NOT_FOUND;
    }
    FreePool (AlignedDevPathNode);
  } else {
    return EFI_NOT_FOUND;
  }

#endif
  //
  // Now try to find the script file. The script file is usually
  // a raw data file which does not contain any sections.
  //
  FileBuffer  = NULL;
  FileBufferSize    = 0;
  Status  = FwVolProtocol->ReadFile (
                                       FwVolProtocol,
                                       &DFU_GUID,
                                       (VOID **) &FileBuffer,
                                       &FileBufferSize,
                                       &FileType,
                                       &Attrib,
                                       &AuthenticationStatus
                                       );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (FileType != EFI_FV_FILETYPE_RAW) {
    return EFI_NOT_FOUND;
  }
  *pFileBuffer = (void *)FileBuffer;
  *pFileSize = (UINT64)FileBufferSize;
  return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
LoadDFUImage (
  void **pFileBuffer,
  UINT64 *pFileSize
  )
{
  EFI_STATUS Status;
  Status = LoadRawFile(pFileBuffer, pFileSize, gEfiConfigFileNameGuid);
  return Status;
}

EFI_STATUS
EFIAPI 
ReportUpdateStatus(
  UINT32 UpdateStatus,
  DFU_FAILURE_REASON Failure
)
{
  EFI_HANDLE handle = NULL;
  EFI_HANDLE  *HandleArray;
  UINTN HandleArrayCount;
  EFI_STATUS Status;
  UINT32 Index;
  DFUStatus.UpdateStatus = UpdateStatus;
  DFUStatus.Failure = Failure;
  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiDFUResultGuid, NULL, &HandleArrayCount, &HandleArray);
  if (EFI_ERROR (Status)) {
    DEBUG((EFI_D_ERROR, "UpdateStatus: LocateHandleBuffer failed \r\n"));
  } else {
    DEBUG((EFI_D_INFO, "UpdateStatus: Uninstall the previously installed protocl instance\r\n"));
    for (Index = 0; Index < HandleArrayCount; Index++) {
      gBS->UninstallProtocolInterface(
        HandleArray[Index],
        &gEfiDFUResultGuid,
        NULL);
    }
  }
  
  Status = gBS->InstallProtocolInterface(
    &handle,
    &gEfiDFUResultGuid,
    EFI_NATIVE_INTERFACE,
    &DFUStatus
  );
  return Status;
}


EFI_STATUS
EFIAPI
CapsuleLibEntryPoint (
  IN EFI_HANDLE        _ImageHandle,
  IN EFI_SYSTEM_TABLE  *_SystemTable
  )
{
  DEBUG((EFI_D_INFO,"CapsuleLibEntryPoint\r\n"));
  ImageHandle = _ImageHandle;
  SystemTable = _SystemTable;
  return EFI_SUCCESS;
}