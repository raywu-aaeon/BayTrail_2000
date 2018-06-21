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
  SmmControlDriver.c

  @brief
  This is the driver that publishes the SMM Control Protocol.

**/
#include "SmmControlDriver.h"

#include <Library/PchPlatformLib.h>
#ifndef ECP_FLAG
#include <Library/PcdLib.h>

#include <Protocol/SmmControl.h>

#include <TianoApi.h>
#endif

STATIC SMM_CONTROL_PRIVATE_DATA mSmmControl;
UINT32                          AcpiBase;

VOID
DisablePendingSmis (
  VOID
  );

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

EFI_STATUS
EFIAPI
SmmControlDriverEntryInit (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/**

  @brief
  This is the constructor for the SMM Control protocol

  @param[in] ImageHandle          Handle for the image of this driver
  @param[in] SystemTable          Pointer to the EFI System Table

  @retval EFI_STATUS              Results of the installation of the SMM Control Protocol

**/
{
  EFI_STATUS  Status;

  if (!IsPchSupported ()) {
    DEBUG ((EFI_D_ERROR, "SMM Control Protocol not supported due to no proper VLV PCU found!\n"));
    Status = EFI_UNSUPPORTED;
  }

  DEBUG ((EFI_D_INFO, "SmmControlDriverEntryInit() Start\n"));

  ///
  /// Get the Power Management I/O space base address.  We assume that
  /// this base address has already been programmed if this driver is
  /// being run.
  ///
  AcpiBase = MmioRead32 (
               MmPciAddress (0,
                 DEFAULT_PCI_BUS_NUMBER_PCH,
                 PCI_DEVICE_NUMBER_PCH_LPC,
                 PCI_FUNCTION_NUMBER_PCH_LPC,
                 R_PCH_LPC_ACPI_BASE
               )
             ) & B_PCH_LPC_ACPI_BASE_BAR;

  Status = EFI_SUCCESS;
  if (AcpiBase != 0) {
    ///
    /// Install the instance of the protocol
    ///
    mSmmControl.Signature                       = SMM_CONTROL_PRIVATE_DATA_SIGNATURE;
    mSmmControl.Handle                          = ImageHandle;

    mSmmControl.SmmControl.Trigger              = Activate;
    mSmmControl.SmmControl.Clear                = Deactivate;
    mSmmControl.SmmControl.GetRegisterInfo      = GetRegisterInfo;
    mSmmControl.SmmControl.MinimumTriggerPeriod = 0;

    ///
    /// Install our protocol interfaces on the device's handle
    ///
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &mSmmControl.Handle,
                    &gEfiSmmControlProtocolGuid,
                    &mSmmControl.SmmControl,
                    NULL
                    );
  } else {
    Status = EFI_DEVICE_ERROR;
  }
  ///
  /// Disable any PCH SMIs that, for whatever reason, are asserted after the boot.
  ///
  DisablePendingSmis ();

  DEBUG ((EFI_D_INFO, "SmmControlDriverEntryInit() End\n"));

  return Status;
}

EFI_STATUS
EFIAPI
SmmTrigger (
  IN UINT8   Data
  )
/**

  @brief
  Trigger the software SMI

  @param[in] Data                 The value to be set on the software SMI data port

  @retval EFI_SUCCESS             Function completes successfully

**/
{
  UINT32  OutputData;
  UINT32  OutputPort;

  ///
  /// Enable the SW SMI
  ///
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

  OutputPort  = R_PCH_APM_CNT;
  OutputData  = Data;

  ///
  /// Generate the SW SMI
  ///
  IoWrite8 (
    (UINTN) OutputPort,
    (UINT8) (OutputData)
    );

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmmClear (
  VOID
  )
/**

  @brief
  Clear the SMI status

  @param[in] None

  @retval EFI_SUCCESS             The function completes successfully
  @retval EFI_DEVICE_ERROR        Something error occurred

**/
{
  EFI_STATUS  Status;
  UINT32      OutputData;
  UINT32      OutputPort;

  Status = EFI_SUCCESS;

  ///
  /// Clear the Power Button Override Status Bit, it gates EOS from being set.
  ///
  OutputPort  = AcpiBase + R_PCH_ACPI_PM1_STS;
  OutputData  = B_PCH_ACPI_PM1_STS_PRBTNOR;

  DEBUG (
    (EFI_D_INFO,
    "The PM1 Status Port at address %x will be written to %x.\n",
    OutputPort,
    OutputData)
    );

  IoWrite16 (
    (UINTN) OutputPort,
    (UINT16) (OutputData)
    );

  ///
  /// Clear the APM SMI Status Bit
  ///
  OutputPort  = AcpiBase + R_PCH_SMI_STS;
  OutputData  = B_PCH_SMI_STS_APM;
  IoWrite32 (
    (UINTN) OutputPort,
    (UINT32) (OutputData)
    );

  ///
  /// Set the EOS Bit
  ///
  OutputPort  = AcpiBase + R_PCH_SMI_EN;
  OutputData  = IoRead32 ((UINTN) OutputPort);
  OutputData |= B_PCH_SMI_EN_EOS;

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

  ///
  /// There is no need to read EOS back and check if it is set.
  /// This can lead to a reading of zero if an SMI occurs right after the SMI_EN port read
  /// but before the data is returned to the CPU.
  /// SMM Dispatcher should make sure that EOS is set after all SMI sources are processed.
  ///
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
/**

  @brief
  This routine generates an SMI

  @param[in] This                 The EFI SMM Control protocol instance
  @param[in] ArgumentBuffer       The buffer of argument
  @param[in] ArgumentBufferSize   The size of the argument buffer
  @param[in] Periodic             Periodic or not
  @param[in] ActivationInterval   Interval of periodic SMI

  @retval EFI Status              Describing the result of the operation
  @retval EFI_INVALID_PARAMETER   Some parameter value passed is not supported

**/
{
  EFI_STATUS  Status;
  UINT8       Data;

  if (Periodic) {
    DEBUG ((EFI_D_WARN, "Invalid parameter\n"));
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
  ///
  /// Clear any pending the APM SMI
  ///
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
/**

  @brief
  This routine clears an SMI

  @param[in] This                 The EFI SMM Control protocol instance
  @param[in] Periodic             Periodic or not

  @retval EFI Status              Describing the result of the operation
  @retval EFI_INVALID_PARAMETER   Some parameter value passed is not supported

**/
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
/**

  @brief
  This routine gets SMM control register information

  @param[in] This                 The SMM Control protocol instance
  @param[in] SmiRegister          Output parameter: the SMI control register information is returned

  @retval EFI_INVALID_PARAMETER   Parameter SmiRegister is NULL
  @retval EFI_SUCCESS             Function completes successfully

**/
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
/**

  @brief
  Disable all pending SMIs

  @param[in] None

  @retval None

**/
{
  UINT32  Data;
  UINT32  Port;
  BOOLEAN SciEn;

  ///
  /// Determine whether an ACPI OS is present (via the SCI_EN bit)
  ///
  Port  = AcpiBase + R_PCH_ACPI_PM1_CNT;
  Data  = IoRead16 ((UINTN) Port);
  SciEn = (BOOLEAN) ((Data & B_PCH_ACPI_PM1_CNT_SCI_EN) == B_PCH_ACPI_PM1_CNT_SCI_EN);

  if (!SciEn) {
    ///
    /// Clear any SMIs that double as SCIs (when SCI_EN==0)
    ///
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
  ///
  /// Clear and disable all SMIs that are unaffected by SCI_EN
  ///
  Port  = AcpiBase + R_PCH_ALT_GP_SMI_EN;
  Data  = 0x0000;
  IoWrite16 ((UINTN) Port, (UINT16) (Data));

  Port  = AcpiBase + R_PCH_ALT_GP_SMI_STS;
  Data  = 0xFFFF;
  IoWrite16 ((UINTN) Port, (UINT16) (Data));

  Port  = AcpiBase + R_PCH_SMI_STS;
  Data  = 0xFFFFFFFF;
  IoWrite32 ((UINTN) Port, (UINT32) (Data));

  ///
  /// (Make sure to write this register last -- EOS re-enables SMIs for the PCH)
  ///
  Port  = AcpiBase + R_PCH_SMI_EN;
  Data  = IoRead32 ((UINTN) Port);
  ///
  /// clear all bits except those tied to SCI_EN
  ///
  Data &= B_PCH_SMI_EN_BIOS_RLS;
  ///
  /// enable SMIs and specifically enable writes to APM_CNT.
  ///
  Data |= B_PCH_SMI_EN_GBL_SMI | B_PCH_SMI_EN_APMC;
  ///
  ///  NOTE: Default value of EOS is set in PCH, it will be automatically cleared Once the PCH asserts SMI# low,
  ///  we don't need to do anything to clear it
  ///
  IoWrite32 ((UINTN) Port, (UINT32) (Data));

}
