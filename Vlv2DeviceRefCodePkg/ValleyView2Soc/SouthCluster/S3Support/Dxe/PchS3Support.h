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
  PchS3Support.h

  @brief
  Header file for PCH S3 Support driver

**/
#ifndef _PCH_S3_SUPPORT_DRIVER_H_
#define _PCH_S3_SUPPORT_DRIVER_H_

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include <Protocol/BootScriptSave/BootScriptSave.h>
#include <Protocol/LoadPe32Image/LoadPe32Image.h>
#include <Protocol/LoadedImage/LoadedImage.h>
#include <Protocol/FirmwareVolume/FirmwareVolume.h>
#else

//
// Driver Consumed Protocol Prototypes
//
#include <Protocol/BootScriptSave.h>
#include <Protocol/LoadPe32Image.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/FirmwareVolume2.h>
#endif

//
// Driver Produced Protocol Prototypes
//
#include <Protocol/PchS3Support.h>

#include "PchAccess.h"
#include <Library/PchPlatformLib.h>
#ifndef ECP_FLAG
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#endif
#include <Guid/PchInitVar.h>

#ifndef ECP_FLAG
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#endif
#define PCH_S3_PEIM_VARIABLE_NAME L"PchS3Peim"

typedef struct {
  EFI_PHYSICAL_ADDRESS  EntryPointAddress;
  UINTN                 PeimSize;
} PCH_S3_PEIM_VARIABLE;

#define EFI_PCH_S3_IMAGE_GUID {0x271dd6f2, 0x54cb, 0x45e6, {0x85, 0x85, 0x8c, 0x92, 0x3c, 0x1a, 0xc7, 0x6}}
//
// Function prototypes
//
EFI_STATUS
EFIAPI
PchS3SetDispatchItem (
  IN     EFI_PCH_S3_SUPPORT_PROTOCOL   *This,
  IN     EFI_PCH_S3_DISPATCH_ITEM      *DispatchItem,
  OUT    EFI_PHYSICAL_ADDRESS          *S3DispatchEntryPoint
  )
/**

  @brief
  Set an item to be dispatched at S3 resume time. At the same time, the entry point
  of the PCH S3 support image is returned to be used in subsequent boot script save
  call

  @param[in] This                 Pointer to the protocol instance.
  @param[in] DispatchItem         The item to be dispatched.
  @param[in] S3DispatchEntryPoint The entry point of the PCH S3 support image.

  @retval EFI_STATUS              Successfully completed.
  @retval EFI_OUT_OF_RESOURCES    Out of resources.

**/
;

EFI_STATUS
SetPchInitVariable (
  VOID
  )
/**

  @brief
  Set the Pch Init Variable for consumption by PchInitS3 PEIM.
  bugbug: expect to extend the variable service to support <4G EfiReservedMemory
  variable storage, such that memory consumption is flexible and more economical.

  @param[in] VOID

  @retval None

**/
;

EFI_STATUS
LoadPchS3Image (
  OUT   UINT32          *ImageEntryPoint
  )
/**

  @brief
  Load the PCH S3 Image into Efi Reserved Memory below 4G.

  @param[in] ImageEntryPoint      The ImageEntryPoint after success loading

  @retval EFI_STATUS

**/
;


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
  );

EFI_STATUS
GetImage (
  IN  EFI_GUID           *NameGuid,
  IN  EFI_SECTION_TYPE   SectionType,
  OUT VOID               **Buffer,
  OUT UINTN              *Size
  );

EFI_STATUS
GetImageEx (
  IN  EFI_HANDLE         ImageHandle,
  IN  EFI_GUID           *NameGuid,
  IN  EFI_SECTION_TYPE   SectionType,
  OUT VOID               **Buffer,
  OUT UINTN              *Size,
  BOOLEAN                WithinImageFv
  );

#endif
