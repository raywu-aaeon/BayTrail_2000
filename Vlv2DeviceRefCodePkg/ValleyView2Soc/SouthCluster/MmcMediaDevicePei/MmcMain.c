/*++
This file contains an 'Intel Peripheral Driver' and is
licensed for Intel CPUs and chipsets under the terms of your
license agreement with Intel or your vendor.  This file may
be modified by the user, subject to additional terms of the
license agreement
--*/
/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

/*++
Module Name:

MediaDeviceDriver.c

Abstract:

UEFI Driver Entry and Binding support.

--*/

#include "MediaDeviceDriver.h"
#include "SdHostDriver.h"



/*++

Routine Description:

Entry point for EFI drivers.

Arguments:

ImageHandle - EFI_HANDLE
SystemTable - EFI_SYSTEM_TABLE

Returns:

EFI_SUCCESS         Success
EFI_DEVICE_ERROR    Fail

--*/
EFI_STATUS
EFIAPI
MmcMainEntryPoint(
#ifdef ECP_FLAG
  IN  EFI_FFS_FILE_HEADER    *FileHandle,
  IN  EFI_PEI_SERVICES       **PeiServices
#else
  IN EFI_PEI_FILE_HANDLE     FileHandle,
  IN CONST EFI_PEI_SERVICES  **PeiServices
#endif
  )
{
  EFI_STATUS Status;

  Status = SdHostDriverEntryPoint(FileHandle, PeiServices);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Fail to Init SD Host controller \n"));
    return Status;
  }
  Status = MediaDeviceDriverEntryPoint(FileHandle, PeiServices);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Fail to Init eMMC Card \n"));
    return Status;
  }
  return EFI_SUCCESS;
}
