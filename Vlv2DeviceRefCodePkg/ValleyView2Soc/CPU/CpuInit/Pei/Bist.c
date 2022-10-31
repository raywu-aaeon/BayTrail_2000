/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    BIST.c

Abstract:

    EFI 2.0 PEIM to store BIST

Revision History

--*/

#include "PeiProcessor.h"
#include "Bist.h"

EFI_STATUS
EFIAPI
BuildBistHob (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PeiServices       - GC_TODO: add argument description
  NotifyDescriptor  - GC_TODO: add argument description
  Ppi               - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS                            Status;
  EFI_BOOT_MODE                         BootMode;
  EFI_SEC_PLATFORM_INFORMATION_PPI      *SecPlatformInformationPpi;
  UINT64                                InformationSize;
  EFI_SEC_PLATFORM_INFORMATION_RECORD   *SecPlatformInformation;
  VOID                                  *Hob;

  Status = (**PeiServices).GetBootMode ((CONST EFI_PEI_SERVICES **) PeiServices, &BootMode);
  if (!EFI_ERROR (Status) && (BootMode == BOOT_ON_S3_RESUME)) {
    return EFI_SUCCESS;
  }

  Status = (**PeiServices).LocatePpi (
                            (CONST EFI_PEI_SERVICES **) PeiServices,
                            &gEfiSecPlatformInformationPpiGuid, // GUID
                            0,                                  // INSTANCE
                            NULL,                               // EFI_PEI_PPI_DESCRIPTOR
                            (VOID **) &SecPlatformInformationPpi          // PPI
                            );

  if (Status == EFI_NOT_FOUND) {
    return EFI_SUCCESS;
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }

  InformationSize         = 0;
  SecPlatformInformation  = NULL;
  Status = SecPlatformInformationPpi->PlatformInformation (
                                        (CONST EFI_PEI_SERVICES **) PeiServices,
                                        &InformationSize,
                                        SecPlatformInformation
                                        );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    Status = (*PeiServices)->AllocatePool (
                              (CONST EFI_PEI_SERVICES **) PeiServices,
                              (UINTN) InformationSize,
                              (VOID **) &SecPlatformInformation
                              );

    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = SecPlatformInformationPpi->PlatformInformation (
                                          (CONST EFI_PEI_SERVICES **) PeiServices,
                                          &InformationSize,
                                          SecPlatformInformation
                                          );
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Hob = BuildGuidDataHob (
          &gEfiHtBistHobGuid,
          SecPlatformInformation,
          (UINTN) InformationSize
          );
  if (Hob == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
  } else {
    Status = EFI_SUCCESS;
  }
  return Status;
}
