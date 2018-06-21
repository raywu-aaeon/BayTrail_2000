/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 2011 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PchSpi.c

Abstract:

  PCH SPI SMM Driver implements the SPI Host Controller Compatibility Interface.

--*/
#include "PchSpi.h"
#include <Library/SmmServicesTableLib.h>
#ifdef ECP_FLAG
EFI_GUID gEfiSmmSpiProtocolGuid = EFI_SMM_SPI_PROTOCOL_GUID;
#endif
//
// Global variables
//
SPI_INSTANCE          *mSpiInstance;

EFI_STATUS
EFIAPI
InstallPchSpi (
  IN EFI_HANDLE            ImageHandle,
  IN EFI_SYSTEM_TABLE      *SystemTable
  )
/*++

Routine Description:

  Entry point for the SPI host controller driver.

Arguments:

  ImageHandle       Image handle of this driver.
  SystemTable       Global system service table.

Returns:

  EFI_SUCCESS           Initialization complete.
  EFI_UNSUPPORTED       The chipset is unsupported by this driver.
  EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.
  EFI_DEVICE_ERROR      Device error, driver exits abnormally.

--*/
{
  EFI_STATUS    Status;

  //
  // Allocate pool for SPI protocol instance
  //
  Status = gSmst->SmmAllocatePool (
                    EfiRuntimeServicesData, // MemoryType don't care
                    sizeof (SPI_INSTANCE),
                    (VOID **) &mSpiInstance
                    );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  if (mSpiInstance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  ZeroMem ((VOID *) mSpiInstance, sizeof (SPI_INSTANCE));
  //
  // Initialize the SPI protocol instance
  //
  Status = SpiProtocolConstructor (mSpiInstance);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Install the SMM EFI_SPI_PROTOCOL interface
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &(mSpiInstance->Handle),
                  &gEfiSmmSpiProtocolGuid,
                  &(mSpiInstance->SpiProtocol),
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    gSmst->SmmFreePool (mSpiInstance);
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

VOID
EFIAPI
SpiPhaseInit (
  VOID
)
/*++
Routine Description:

  This function is a a hook for Spi Smm phase specific initialization

Arguments:

  None

Returns:

  None

--*/
{
  UINTN       Index;
  static CONST UINT32 SpiRegister[] = {
    R_PCH_SPI_SSFCS,
    R_PCH_SPI_PREOP,
    R_PCH_SPI_OPMENU0,
    R_PCH_SPI_OPMENU1,
    R_PCH_SPI_LVSCC,
    R_PCH_SPI_UVSCC
  };

  //
  // Save SPI Registers for S3 resume usage
  //
  for (Index = 0; Index < sizeof (SpiRegister) / sizeof (UINT32); Index++) {
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (mSpiInstance->SpiBase + SpiRegister[Index]),
      1,
      (VOID *) (UINTN) (mSpiInstance->SpiBase + SpiRegister[Index])
      );
  }
}
