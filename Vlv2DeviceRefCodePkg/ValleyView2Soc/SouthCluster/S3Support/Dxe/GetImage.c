/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  GetImage.c

  @brief
  Functions that loads image from FV

**/
#include "PchS3Support.h"
#ifndef ECP_FLAG
#include <Protocol/FirmwareVolume2.h>
#endif

EFI_STATUS
GetImageFromFv (
#ifdef ECP_FLAG
  IN  EFI_FIRMWARE_VOLUME_PROTOCOL          *Fv,
#else
  IN  EFI_FIRMWARE_VOLUME2_PROTOCOL         *Fv,
#endif
  IN  EFI_GUID                              *NameGuid,
  IN  EFI_SECTION_TYPE                      SectionType,
  OUT VOID                                  **Buffer,
  OUT UINTN                                 *Size
  )
/**

  @brief
  Get an image from firmware volume

  @param[in] Fv                     The firmware volume protocol instance
  @param[in] NameGuid               GUID of file
  @param[in] SectionType            Type of section
  @param[out] Buffer                Image content buffer
  @param[out] Size                  Image size

  @retval EFI_SUCCESS               Successfully completed.
  @retval EFI_WARN_BUFFER_TOO_SMALL Too small a buffer is to contain image content
  @retval EFI_ACCESS_DENIED         Access firmware volume error.

**/
{
  EFI_STATUS              Status;
  EFI_FV_FILETYPE         FileType;
  EFI_FV_FILE_ATTRIBUTES  Attributes;
  UINT32                  AuthenticationStatus;

  ///
  /// Read desired section content in NameGuid file
  ///
  *Buffer = NULL;
  *Size   = 0;
  Status = Fv->ReadSection (
                 Fv,
                 NameGuid,
                 SectionType,
                 0,
                 Buffer,
                 Size,
                 &AuthenticationStatus
                 );

  if (EFI_ERROR (Status) && (SectionType == EFI_SECTION_TE)) {
    ///
    /// Try reading PE32 section, since the TE section does not exist
    ///
    *Buffer = NULL;
    *Size   = 0;
    Status = Fv->ReadSection (
                   Fv,
                   NameGuid,
                   EFI_SECTION_PE32,
                   0,
                   Buffer,
                   Size,
                   &AuthenticationStatus
                   );
  }

  if (EFI_ERROR (Status) && ((SectionType == EFI_SECTION_TE) || (SectionType == EFI_SECTION_PE32))) {
    ///
    /// Try reading raw file, since the desired section does not exist
    ///
    *Buffer = NULL;
    *Size   = 0;
    Status = Fv->ReadFile (
                   Fv,
                   NameGuid,
                   Buffer,
                   Size,
                   &FileType,
                   &Attributes,
                   &AuthenticationStatus
                   );
  }

  return Status;
}

EFI_STATUS
GetImage (
  IN  EFI_GUID           *NameGuid,
  IN  EFI_SECTION_TYPE   SectionType,
  OUT VOID               **Buffer,
  OUT UINTN              *Size
  )
/**

  @brief
  Wrapper for GetImageEx

  @param[in] NameGuid             File Name GUID
  @param[in] SectionType          Sectio type
  @param[in] Buffer               Buffer to contain image
  @param[in] Size                 Image size

  @retval EFI_INVALID_PARAMETER   Invalid parameter
  @retval EFI_NOT_FOUND           Can not find the file
  @retval EFI_SUCCESS             Successfully completed

**/
{
  return GetImageEx (NULL, NameGuid, SectionType, Buffer, Size, FALSE);
}

EFI_STATUS
GetImageEx (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_GUID           *NameGuid,
  IN  EFI_SECTION_TYPE   SectionType,
  OUT VOID               **Buffer,
  OUT UINTN              *Size,
  BOOLEAN                WithinImageFv
  )
/**

  @brief
  Get specified image from a firmware volume.

  @param[in] ImageHandle          Image handle for the loaded driver
  @param[in] NameGuid             File name GUID
  @param[in] SectionType          Section type
  @param[in] Buffer               Bufer to contain image
  @param[in] Size                 Image size
  @param[in] WithinImageFv        Whether or not in a firmware volume

  @retval EFI_INVALID_PARAMETER   Invalid parameter
  @retval EFI_NOT_FOUND           Can not find the file
  @retvalEFI_SUCCESS             Successfully completed

**/
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         HandleCount;
  UINTN                         Index;
  EFI_LOADED_IMAGE_PROTOCOL     *LoadedImage;
#ifdef ECP_FLAG
  EFI_FIRMWARE_VOLUME_PROTOCOL  *ImageFv;
  EFI_FIRMWARE_VOLUME_PROTOCOL  *Fv;
#else
  EFI_FIRMWARE_VOLUME2_PROTOCOL *ImageFv;
  EFI_FIRMWARE_VOLUME2_PROTOCOL *Fv;
#endif

  DEBUG ((EFI_D_INFO, "GetImageEx() Start\n"));

  if (ImageHandle == NULL && WithinImageFv) {
    return EFI_INVALID_PARAMETER;
  }

  Status  = EFI_NOT_FOUND;
  ImageFv = NULL;
  if (ImageHandle != NULL) {
    Status = gBS->HandleProtocol (
                    ImageHandle,
                    &gEfiLoadedImageProtocolGuid,
                    (VOID **) &LoadedImage
                    );
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = gBS->HandleProtocol (
                    LoadedImage->DeviceHandle,
#ifdef ECP_FLAG
                    &gEfiFirmwareVolumeProtocolGuid,
#else
                    &gEfiFirmwareVolume2ProtocolGuid,
#endif
                    (VOID **) &ImageFv
                    );
    if (!EFI_ERROR (Status)) {
      Status = GetImageFromFv (ImageFv, NameGuid, SectionType, Buffer, Size);
    }
  }

  if (Status == EFI_SUCCESS || WithinImageFv) {
    return Status;
  }

  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
#ifdef ECP_FLAG
                  &gEfiFirmwareVolumeProtocolGuid,
#else
                  &gEfiFirmwareVolume2ProtocolGuid,
#endif
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  ///
  /// Find desired image in all Fvs
  ///
  for (Index = 0; Index < HandleCount; ++Index) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
#ifdef ECP_FLAG
                    &gEfiFirmwareVolumeProtocolGuid,
#else
                    &gEfiFirmwareVolume2ProtocolGuid,
#endif
                    (VOID **) &Fv
                    );

    if (EFI_ERROR (Status)) {
      (gBS->FreePool) (HandleBuffer);
      return Status;
    }

    if (ImageFv != NULL && Fv == ImageFv) {
      continue;
    }

    Status = GetImageFromFv (Fv, NameGuid, SectionType, Buffer, Size);

    if (!EFI_ERROR (Status)) {
      break;
    }
  }
  (gBS->FreePool) (HandleBuffer);

  ///
  /// Not found image
  ///
  if (Index == HandleCount) {
    return EFI_NOT_FOUND;
  }

  DEBUG ((EFI_D_INFO, "GetImageEx() End\n"));

  return EFI_SUCCESS;
}
