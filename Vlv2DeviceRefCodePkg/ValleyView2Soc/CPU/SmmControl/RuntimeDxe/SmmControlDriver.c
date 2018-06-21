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

  SmmControlDriver.c

Abstract:

  This is the driver that publishes the SMM Control Protocol.

--*/
#include "SmmControlDriver.h"

#include <Library/PchPlatformLib.h>
#ifndef ECP_FLAG
#include <Library/PcdLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Guid/EventGroup.h>
#include <Protocol/SmmControl.h>
#endif
STATIC SMM_CONTROL_PRIVATE_DATA mSmmControl;
UINT32                          AcpiBase;

#ifdef ECP_FLAG
VOID
EFIAPI
SmmControlVirtualAddressChangeEvent (
  IN EFI_EVENT                  Event,
  IN VOID                       *Context
  )
/**

  @brief
  Fixup internal data pointers so that the services can be called in virtual mode.

  @param[in] Event                The event registered.
  @param[in] Context              Event context.

  @retval None.

**/
{
  gRT->ConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mSmmControl.SmmControl.Trigger));
  gRT->ConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mSmmControl.SmmControl.Clear));
  gRT->ConvertPointer (EFI_INTERNAL_POINTER, (VOID *) &(mSmmControl.SmmControl.GetRegisterInfo));
}
#else
/**
  Notification function of EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE.

  This is a notification function registered on EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event.
  It convers pointer to new virtual address.

  @param[in]  Event        Event whose notification function is being invoked.
  @param[in]  Context      Pointer to the notification function's context.

**/
VOID
EFIAPI
SetVirtualAddressNotify (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EfiConvertPointer (0x0, (VOID **)&mSmmControl.SmmControl.Trigger);
  EfiConvertPointer (0x0, (VOID **)&mSmmControl.SmmControl.Clear);
  EfiConvertPointer (0x0, (VOID **)&mSmmControl.SmmControl.GetRegisterInfo);
}
#endif


EFI_STATUS
EFIAPI
SmmControlDriverEntryInit (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++

Routine Description:

  This is the constructor for the SMM Control protocol

Arguments:

  ImageHandle                   Handle for the image of this driver
  SystemTable                   Pointer to the EFI System Table

Returns:

  Results of the installation of the SMM Control Protocol

--*/
{
  EFI_STATUS  Status;
  EFI_EVENT                Event;

  if (!IsPchSupported ()) {
    DEBUG ((EFI_D_ERROR, "SMM Control Protocol not supported due to no proper VLV PCU found!\n"));
    Status = EFI_UNSUPPORTED;
  }

  DEBUG ((EFI_D_INFO, "SmmControlDriverEntryInit() Start\n"));

  //
  // Get the Power Management I/O space base address.  We assume that
  // this base address has already been programmed if this driver is
  // being run.
  //
  AcpiBase = MmioRead32 (
              PchPciDeviceMmBase (DEFAULT_PCI_BUS_NUMBER_PCH,
              PCI_DEVICE_NUMBER_PCH_LPC,
              PCI_FUNCTION_NUMBER_PCH_LPC) + R_PCH_LPC_ACPI_BASE
              ) & B_PCH_LPC_ACPI_BASE_BAR;

  Status = EFI_SUCCESS;
  if (AcpiBase != 0) {
    //
    // Install the instance of the protocol
    //
    mSmmControl.Signature                       = SMM_CONTROL_PRIVATE_DATA_SIGNATURE;
    mSmmControl.Handle                          = ImageHandle;

    mSmmControl.SmmControl.Trigger              = Activate;
    mSmmControl.SmmControl.Clear                = Deactivate;
    mSmmControl.SmmControl.GetRegisterInfo      = GetRegisterInfo;
    mSmmControl.SmmControl.MinimumTriggerPeriod = 0;

    ///
    /// Create event on SetVirtualAddressMap() to convert mSmmControl from a physical address to a virtual address
    ///
#ifdef ECP_FLAG
    Status = (gBS->CreateEventEx) (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    SmmControlVirtualAddressChangeEvent,
                    NULL,
                    &gEfiEventVirtualAddressChangeGuid,
                    &Event
                    );
#else
    Status = gBS->CreateEventEx (
                    EVT_NOTIFY_SIGNAL,
                    TPL_NOTIFY,
                    SetVirtualAddressNotify,
                    NULL,
                    &gEfiEventVirtualAddressChangeGuid,
                    &Event
                    );
#endif
    ASSERT_EFI_ERROR (Status);
    //
    // Install our protocol interfaces on the device's handle
    //
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &mSmmControl.Handle,
                    &gEfiSmmControlProtocolGuid,
                    &mSmmControl.SmmControl,
                    NULL
                    );
  } else {
    Status = EFI_DEVICE_ERROR;
  }
  //
  // Disable any PCH SMIs that, for whatever reason, are asserted after the boot.
  //
  DisablePendingSmis ();

  DEBUG ((EFI_D_INFO, "SmmControlDriverEntryInit() End\n"));

  return Status;
}

EFI_STATUS
EFIAPI
SmmTrigger (
  IN UINT8   Data
  )
/*++

Routine Description:

  Trigger the software SMI

Arguments:

  Data                          The value to be set on the software SMI data port

Returns:

  EFI_SUCCESS                   Function completes successfully

--*/
{
  UINT32  OutputData;
  UINT32  OutputPort;
  BOOLEAN LUsb;

  //
  // Enable the SW SMI
  //
  OutputPort  = AcpiBase + R_PCH_SMI_EN;
  OutputData  = IoRead32 ((UINTN) OutputPort);
  OutputData |= (B_PCH_SMI_EN_APMC | B_PCH_SMI_EN_GBL_SMI);
//  DEBUG (
//    (EFI_D_INFO,
//    "The SMI Control Port at address %x will be written to %x.\n",
//    OutputPort,
//    OutputData)
//    );
  IoWrite32 (
    (UINTN) OutputPort,
    (UINT32) (OutputData)
    );

  if ((OutputData & B_PCH_SMI_EN_LEGACY_USB2) == B_PCH_SMI_EN_LEGACY_USB2) {
    LUsb = 1;
    IoWrite32 (AcpiBase + R_PCH_SMI_EN, (UINT32) (OutputData & (UINT32)~B_PCH_SMI_EN_LEGACY_USB2));
  } else {
    LUsb = 0;
  }

  OutputPort  = R_PCH_APM_CNT;
  OutputData  = Data;

  //
  // Generate the SW SMI
  //
  IoWrite8 (
    (UINTN) OutputPort,
    (UINT8) (OutputData)
    );

  if (LUsb) {
    IoWrite32(AcpiBase + R_PCH_SMI_EN, (UINT32) (IoRead32(AcpiBase + R_PCH_SMI_EN) | B_PCH_SMI_EN_LEGACY_USB2));
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmmClear (
  VOID
  )
/*++

Routine Description:

  Clear the SMI status

Arguments:

  None

Returns:

  EFI_SUCCESS                   The function completes successfully
  EFI_DEVICE_ERROR              Something error occurred

--*/
{
  EFI_STATUS  Status;
  UINT32      OutputData;
  UINT32      OutputPort;

  Status = EFI_SUCCESS;

  //
  // Clear the Power Button Override Status Bit, it gates EOS from being set.
  //
  OutputPort  = AcpiBase + R_PCH_ACPI_PM1_STS;
  OutputData  = B_PCH_ACPI_PM1_STS_PRBTNOR;
  IoWrite16 (
    (UINTN) OutputPort,
    (UINT16) (OutputData)
    );

  //
  // Clear the APM SMI Status Bit
  //
  OutputPort  = AcpiBase + R_PCH_SMI_STS;
  OutputData  = B_PCH_SMI_STS_APM;
  IoWrite32 (
    (UINTN) OutputPort,
    (UINT32) (OutputData)
    );

  //
  // Set the EOS Bit
  //
  OutputPort  = AcpiBase + R_PCH_SMI_EN;
  OutputData  = IoRead32 ((UINTN) OutputPort);
  OutputData |= B_PCH_SMI_EN_EOS;
  IoWrite32 (
    (UINTN) OutputPort,
    (UINT32) (OutputData)
    );

  //
  // There is no need to read EOS back and check if it is set.
  // This can lead to a reading of zero if an SMI occurs right after the SMI_EN port read
  // but before the data is returned to the CPU.
  // SMM Dispatcher should make sure that EOS is set after all SMI sources are processed.
  //
  return Status;
}


EFI_STATUS
EFIAPI
Activate (
  IN      EFI_SMM_CONTROL_PROTOCOL                      * This,
  IN OUT  INT8                                          *ArgumentBuffer OPTIONAL,
  IN OUT  UINTN                                         *ArgumentBufferSize OPTIONAL,
  IN      BOOLEAN                                       Periodic OPTIONAL,
  IN      UINTN                                         ActivationInterval OPTIONAL
  )
/*++

Routine Description:

    This routine generates an SMI

  Arguments:
    This
    ArgumentBuffer
    ArgumentBufferSize
    Periodic
    ActivationInterval

Returns:

    EFI Status describing the result of the operation
    EFI_INVALID_PARAMETER       Some parameter value passed is not supported

--*/
{
  EFI_STATUS  Status;
  UINT8       Data;

  if (Periodic) {
    return EFI_INVALID_PARAMETER;
  }

  if (ArgumentBuffer == NULL) {
    Data = 0xFF;
  } else {
    if (ArgumentBufferSize == NULL || *ArgumentBufferSize != 1) {
      return EFI_INVALID_PARAMETER;
    }

    Data = *ArgumentBuffer;
  }
  //
  // Clear any pending the APM SMI
  //
  Status = SmmClear ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return SmmTrigger (Data);
}

EFI_STATUS
EFIAPI
Deactivate (
  IN  EFI_SMM_CONTROL_PROTOCOL             *This,
  IN  BOOLEAN                              Periodic OPTIONAL
  )
/*++

  Routine Description:
    This routine clears an SMI

  Arguments:
    This                        The EFI SMM Control protocol instance
    Periodic                    Periodic or not

  Returns:
    EFI Status describing the result of the operation
    EFI_INVALID_PARAMETER       Some parameter value passed is not supported

--*/
{
  if (Periodic) {
    return EFI_INVALID_PARAMETER;
  }

  return SmmClear ();
}

EFI_STATUS
EFIAPI
GetRegisterInfo (
  IN      EFI_SMM_CONTROL_PROTOCOL      *This,
  IN OUT  EFI_SMM_CONTROL_REGISTER      *SmiRegister
  )
/*++

  Routine Description:
    This routine gets SMM control register information

  Arguments:
    This                        The SMM Control protocol instance
    SmiRegister                 Output parameter: the SMI control register information is returned

  Returns:
    EFI_INVALID_PARAMETER       Parameter SmiRegister is NULL
    EFI_SUCCESS                 Function completes successfully

--*/
{
  if (SmiRegister == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SmiRegister->SmiTriggerRegister = R_PCH_APM_CNT;
  SmiRegister->SmiDataRegister    = R_PCH_APM_STS;
  return EFI_SUCCESS;
}

VOID
EFIAPI
DisablePendingSmis (
  VOID
  )
/*++

Routine Description:

  Disable all pending SMIs

Arguments:

  None

Returns:

  None

--*/
{
  UINT32  Data;
  UINT32  Port;
  BOOLEAN SciEn;

  //
  // Determine whether an ACPI OS is present (via the SCI_EN bit)
  //
  Port  = AcpiBase + R_PCH_ACPI_PM1_CNT;
  Data  = IoRead16 ((UINTN) Port);
  SciEn = (BOOLEAN) ((Data & B_PCH_ACPI_PM1_CNT_SCI_EN) == B_PCH_ACPI_PM1_CNT_SCI_EN);

  if (!SciEn) {
    //
    // Clear any SMIs that double as SCIs (when SCI_EN==0)
    //
    Port  = AcpiBase + R_PCH_ACPI_PM1_STS;
    Data  = 0xFFFF;
    IoWrite16 ((UINTN) Port, (UINT16) (Data));

    Port  = AcpiBase + R_PCH_ACPI_PM1_EN;
    Data  = 0x0000;
    IoWrite16 ((UINTN) Port, (UINT16) (Data));

    Port  = AcpiBase + R_PCH_ACPI_PM1_CNT;
    Data  = 0x0000;
    IoWrite16 ((UINTN) Port, (UINT16) (Data));

    Port  = AcpiBase + R_PCH_ACPI_GPE0a_STS;
    Data  = 0xFFFFFFFF;
    IoWrite32 ((UINTN) Port, (UINT32) (Data));

    Port  = AcpiBase + R_PCH_ACPI_GPE0a_EN;
    Data  = 0x00000000;
    IoWrite32 ((UINTN) Port, (UINT32) (Data));
  }
  //
  // Clear and disable all SMIs that are unaffected by SCI_EN
  //
  Port  = AcpiBase + R_PCH_ALT_GP_SMI_EN;
  Data  = 0x0000;
  IoWrite16 ((UINTN) Port, (UINT16) (Data));

  Port  = AcpiBase + R_PCH_ALT_GP_SMI_STS;
  Data  = 0xFFFF;
  IoWrite16 ((UINTN) Port, (UINT16) (Data));

  Port  = AcpiBase + R_PCH_SMI_STS;
  Data  = 0xFFFFFFFF;
  IoWrite32 ((UINTN) Port, (UINT32) (Data));

  //
  // (Make sure to write this register last -- EOS re-enables SMIs for the PCH)
  //
  Port  = AcpiBase + R_PCH_SMI_EN;
  Data  = IoRead32 ((UINTN) Port);
  //
  // clear all bits except those tied to SCI_EN
  //
  Data &= B_PCH_SMI_EN_BIOS_RLS;
  //
  // enable SMIs and specifically enable writes to APM_CNT.
  //
  Data |= B_PCH_SMI_EN_GBL_SMI | B_PCH_SMI_EN_APMC;
  //
  //  NOTE: Default value of EOS is set in PCH, it will be automatically cleared Once the PCH asserts SMI# low,
  //  we don't need to do anything to clear it
  //
  IoWrite32 ((UINTN) Port, (UINT32) (Data));

}
