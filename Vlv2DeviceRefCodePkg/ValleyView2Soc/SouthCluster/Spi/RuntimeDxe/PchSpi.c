/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 2004 - 2015 Intel Corporation. All rights reserved
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

  PCH SPI Runtime Driver implements the SPI Host Controller Compatibility Interface.

--*/
#include "PchSpi.h"
#ifdef ECP_FLAG
EFI_GUID gEfiBootScriptSaveProtocolGuid = EFI_BOOT_SCRIPT_SAVE_PROTOCOL_GUID;
EFI_GUID gEfiSpiProtocolGuid = EFI_SPI_PROTOCOL_GUID;
EFI_GUID gEfiSpiDataProtocolGuid = EFI_SPI_DATA_PROTOCOL_GUID;
#else
#include <Guid/EventGroup.h>
#include <TianoApi.h>
#include <Library/UefiLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/S3BootScriptLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/DxeRuntimePciLibPciExpress.h>
#include <Library/UefiRuntimeLib.h>
#endif

//
// Global variables
//
SPI_INSTANCE  *mSpiInstance;
EFI_SPI_DATA_PROTOCOL mSpiDataInfoProtocol;
EFI_EVENT     mVirtualAddressChangeEvent = NULL;
static CONST UINT32 mSpiRegister[] = {
  R_PCH_SPI_SSFCS,
  R_PCH_SPI_PREOP,
  R_PCH_SPI_OPMENU0,
  R_PCH_SPI_OPMENU1,
  R_PCH_SPI_LVSCC,
  R_PCH_SPI_UVSCC
  };

//
// Function implementations
//
VOID
PchSpiVirtualddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
/*++

Routine Description:

  Fixup internal data pointers so that the services can be called in virtual mode.

Arguments:

  Event     The event registered.
  Context   Event context. Not used in this event handler.

Returns:

  None.

--*/
{
  EfiConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mSpiInstance->SpiBase));
  EfiConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mSpiInstance->SpiProtocol.Init));
  EfiConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mSpiInstance->SpiProtocol.Lock));
  EfiConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mSpiInstance->SpiProtocol.Execute));
  EfiConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mSpiInstance));
}

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
  EFI_STATUS                      Status;
  UINT64                          BaseAddress;
  UINT64                          Length;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR GcdMemorySpaceDescriptor;
  EFI_GCD_MEMORY_SPACE_DESCRIPTOR LpcMemorySpaceDescriptor;
  UINT64                          Attributes;

  DEBUG ((EFI_D_INFO, "InstallPchSpi() Start\n"));

  Status = PciLibConstructor ();
  ASSERT_EFI_ERROR (Status);

  //
  // Allocate Runtime memory for the SPI protocol instance.
  //
  mSpiInstance = AllocateRuntimeZeroPool (sizeof (SPI_INSTANCE));
  if (mSpiInstance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Initialize the SPI protocol instance
  //
  Status = SpiProtocolConstructor (mSpiInstance);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Install the EFI_SPI_PROTOCOL interface
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &(mSpiInstance->Handle),
                  &gEfiSpiProtocolGuid,
                  &(mSpiInstance->SpiProtocol),
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    FreePool (mSpiInstance);
    return EFI_DEVICE_ERROR;
  }
  //
  // Set SPI memory space in GCD to be RUNTIME so that the range will be supported in
  // virtual address mode in EFI aware OS runtime.
  // It will assert if SPI memory space is not allocated
  // The caller is responsible for the existence and allocation of the SPI memory spaces
  //
  BaseAddress = (UINTN) (mSpiInstance->SpiBase);
  Length      = 0x1000;

  Status      = gDS->GetMemorySpaceDescriptor (BaseAddress, &GcdMemorySpaceDescriptor);
  ASSERT_EFI_ERROR (Status);

  Attributes = GcdMemorySpaceDescriptor.Attributes | EFI_MEMORY_RUNTIME;

  Status = gDS->SetMemorySpaceAttributes (
                  BaseAddress,
                  Length,
                  Attributes
                  );
  ASSERT_EFI_ERROR (Status);

  //
  //  LPC memory space
  //
  BaseAddress = (UINTN) PcdGet64 (PcdPciExpressBaseAddress) + (EFI_PHYSICAL_ADDRESS) PCI_LIB_ADDRESS (
                                                                                                      DEFAULT_PCI_BUS_NUMBER_PCH,
                                                                                                      PCI_DEVICE_NUMBER_PCH_LPC,
                                                                                                      PCI_FUNCTION_NUMBER_PCH_LPC,
                                                                                                      0
                                                                                                      );
  Length  = 4096;

  Status  = gDS->GetMemorySpaceDescriptor (BaseAddress, &LpcMemorySpaceDescriptor);
  ASSERT_EFI_ERROR (Status);

  Attributes = LpcMemorySpaceDescriptor.Attributes | EFI_MEMORY_RUNTIME;

  Status = gDS->SetMemorySpaceAttributes (
                  BaseAddress,
                  Length,
                  Attributes
                  );
  ASSERT_EFI_ERROR (Status);

  Status = PciLibRegisterMemory (
            PCI_LIB_ADDRESS (DEFAULT_PCI_BUS_NUMBER_PCH,
            PCI_DEVICE_NUMBER_PCH_LPC,
            PCI_FUNCTION_NUMBER_PCH_LPC,
            0),
            (UINTN) Length
            );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->CreateEventEx (
                  EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  PchSpiVirtualddressChangeEvent,
                  NULL,
                  &gEfiEventVirtualAddressChangeGuid,
                  &mVirtualAddressChangeEvent
                  );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_INFO, "InstallPchSpi() End\n"));

  return EFI_SUCCESS;
}

VOID
VsccS3SaveRestore (
  EFI_EVENT           Event,
  VOID                *ParentImageHandle
)
/*++
Routine Description:

  Save SPI UVSCC/LVSCC register into S3 resume script table.

Arguments:

  Event                   The event that triggered this notification function
  ParentImageHandle       Pointer to the notification functions context

Returns:

  None

--*/
{
  UINTN Index;
#ifdef ECP_FLAG
  INITIALIZE_SCRIPT (ParentImageHandle, gST);
#else

//  INITIALIZE_SCRIPT (ParentImageHandle, gST);
#endif
  //
  // Save SPI Registers for S3 resume usage
  //
  for (Index = 0; Index < sizeof (mSpiRegister) / sizeof (UINT32); Index++) {
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (mSpiInstance->SpiBase + mSpiRegister[Index]),
      1,
      (VOID *) (UINTN) (mSpiInstance->SpiBase + mSpiRegister[Index])
      );
  }

}

VOID
EFIAPI
SpiPhaseInit (
  VOID
  )
/*++
Routine Description:

  This function is a a hook for Spi Dxe phase specific initialization

Arguments:

  None

Returns:

  None

--*/
{
  EFI_STATUS                    Status;
  VOID                          *Registration;
  UINTN                         Index;
  EFI_BOOT_SCRIPT_SAVE_PROTOCOL *mBootScriptSave = NULL;

  //
  // Disable SMM BIOS write protect if it's not a SMM protocol
  //
  MmioAnd8 (
    mSpiInstance->SpiBase + R_PCH_SPI_BCR,
    (UINT8) (~B_PCH_SPI_BCR_SMM_BWP)
    );

  //
  // Locate the S3 resume scripting protocol
  //
  Status = gBS->LocateProtocol (&gEfiBootScriptSaveProtocolGuid, NULL, (VOID **) &mBootScriptSave);
  if (Status == EFI_SUCCESS) {
    //
    // Save SPI Registers for S3 resume usage
    //
    for (Index = 0; Index < sizeof (mSpiRegister) / sizeof (UINT32); Index++) {
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (mSpiInstance->SpiBase + mSpiRegister[Index]),
        1,
        (VOID *) (UINTN) (mSpiInstance->SpiBase + mSpiRegister[Index])
        );
    }
  } else if (Status == EFI_NOT_FOUND) {
    //
    // Create event for the SPI flash VSCC registers S3 save/restore.
    //
    EfiCreateProtocolNotifyEvent (
      &gEfiBootScriptSaveProtocolGuid,
      TPL_CALLBACK,
      VsccS3SaveRestore,
      NULL,
      &Registration
      );
  } else {
    ASSERT_EFI_ERROR (Status);
  }
  
  ///
  /// Initialize and Install the SPI Data protocol
  ///
  mSpiDataInfoProtocol.BiosSize = mSpiInstance->SpiInitTable.BiosSize;
  mSpiDataInfoProtocol.BiosStartMemoryAddress = 0xFFFFFFFF - mSpiDataInfoProtocol.BiosSize + 1;
  DEBUG ((EFI_D_INFO, "SPI : BiosStartMemoryAddress: %x, BiosSize: %x\n", mSpiDataInfoProtocol.BiosStartMemoryAddress, mSpiDataInfoProtocol.BiosSize));
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &(mSpiInstance->Handle),
                  &gEfiSpiDataProtocolGuid,
                  &mSpiDataInfoProtocol,
                  NULL
                  );
  if (EFI_ERROR(Status))
  {
    ASSERT(FALSE);
  }

  return;
}
