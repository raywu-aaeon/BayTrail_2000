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
  PchEhci.c

  @brief
  Pch Ehci PPI Init

**/
#include "PchEhci.h"
#ifdef ECP_FLAG
EFI_GUID gPeiUsbControllerPpiGuid = PEI_USB_CONTROLLER_PPI_GUID;
#endif

///
/// PPI interface function
///
STATIC
EFI_STATUS
EFIAPI
GetEhciController (
  IN  EFI_PEI_SERVICES                **PeiServices,
  IN  PEI_USB_CONTROLLER_PPI          *This,
  IN  UINT8                           UsbControllerId,
  OUT UINTN                           *ControllerType,
  OUT UINTN                           *BaseAddress
);

///
/// Globals
///
STATIC PEI_USB_CONTROLLER_PPI   mEhciControllerPpi = { GetEhciController };

STATIC EFI_PEI_PPI_DESCRIPTOR   mPpiList = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gPeiUsbControllerPpiGuid,
  NULL
};

///
/// Helper function
///
STATIC
EFI_STATUS
EnableEhciController (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES  **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES   **PeiServices,
#endif
  IN PCH_EHCI_DEVICE          *PeiPchEhciDev,
  IN UINT8                    UsbControllerId
);

EFI_STATUS
InitForEHCI (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES            **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES      **PeiServices,
#endif
  IN PCH_USB_POLICY_PPI          *UsbPolicyPpi
)
/**

  @brief
  Initialize PCH EHCI PEIM

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] UsbPolicyPpi         PCH Usb Policy PPI

  @retval EFI_SUCCESS             The PCH EHCI PEIM is initialized successfully
  @retval EFI_INVALID_PARAMETER   UsbControllerId is out of range
  @retval EFI_OUT_OF_RESOURCES    Insufficient resources to create database
  @retval Others                  All other error conditions encountered result in an ASSERT.

**/
{
  EFI_STATUS            Status;
  UINTN                 Index;
  PCH_EHCI_DEVICE       *PeiPchEhciDev;
  EFI_BOOT_MODE         BootMode;

  DEBUG ((EFI_D_INFO, "InitForEHCI() Start\n"));

  Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);

  ///
  /// We do not export this in S3 boot path, because it is only for recovery.
  ///
  if (BootMode == BOOT_ON_S3_RESUME) {
    return EFI_SUCCESS;
  }

  PeiPchEhciDev = (PCH_EHCI_DEVICE *) AllocatePool (sizeof (PCH_EHCI_DEVICE));
  if (PeiPchEhciDev == NULL) {
    DEBUG ((EFI_D_ERROR, "Failed to allocate memory for PeiPchEhciDev! \n"));
    return EFI_OUT_OF_RESOURCES;
  }

  PeiPchEhciDev->EhciControllerPpi    = mEhciControllerPpi;
  PeiPchEhciDev->PpiList              = mPpiList;
  PeiPchEhciDev->PpiList.Ppi          = &PeiPchEhciDev->EhciControllerPpi;

  PeiPchEhciDev->TotalEhciControllers = PchEhciControllerMax;

  ///
  /// Assign resources and enable EHCI controllers
  ///
  if (UsbPolicyPpi->EhciMemLength < (EHCI_MEMORY_SPACE * PeiPchEhciDev->TotalEhciControllers)) {
    DEBUG ((EFI_D_ERROR, "The EhciMemLength got from UsbPolicyPpi is less than the required (%0x) !\n", (EHCI_MEMORY_SPACE * PeiPchEhciDev->TotalEhciControllers)));
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < PeiPchEhciDev->TotalEhciControllers; Index++) {
    PeiPchEhciDev->MemBase[Index] = UsbPolicyPpi->EhciMemBaseAddr + EHCI_MEMORY_SPACE * Index;
    Status                        = EnableEhciController (PeiServices, PeiPchEhciDev, (UINT8) Index);
    ASSERT_EFI_ERROR (Status);
  }
  ///
  /// Install USB Controller PPI
  ///
  Status = PeiServicesInstallPpi (&PeiPchEhciDev->PpiList);

  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "InitForEHCI() End\n"));

  return Status;

}
///
/// PPI interface implementation
///
STATIC
EFI_STATUS
EFIAPI
GetEhciController (
  IN  EFI_PEI_SERVICES                **PeiServices,
  IN  PEI_USB_CONTROLLER_PPI          *This,
  IN  UINT8                           UsbControllerId,
  OUT UINTN                           *ControllerType,
  OUT UINTN                           *BaseAddress
  )
/**

  @brief
  Get EHCI controller information

  @param[in] PeiServices          General PEI services
  @param[in] This                 Pointer to the PEI_EHCI_CONTROLLER_PPI
  @param[in] UsbControllerId      The USB controller number
  @param[in] ControllerType       Output: USB controller type
  @param[in] BaseAddress          Output: EHCI controller memory base address

  @retval EFI_INVALID_PARAMETER   UsbControllerId is out of range
  @retval EFI_SUCCESS             Function completes successfully

**/
{
  PCH_EHCI_DEVICE *PeiPchEhciDev;

  PeiPchEhciDev = PCH_EHCI_DEVICE_FROM_THIS (This);

  if (UsbControllerId >= PeiPchEhciDev->TotalEhciControllers) {
    return EFI_INVALID_PARAMETER;
  }

  *ControllerType = PEI_EHCI_CONTROLLER;

  *BaseAddress    = PeiPchEhciDev->MemBase[UsbControllerId];

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EnableEhciController (
#ifdef ECP_FLAG
  IN EFI_PEI_SERVICES               **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES         **PeiServices,
#endif
  IN PCH_EHCI_DEVICE                *PeiPchEhciDev,
  IN UINT8                          UsbControllerId
  )
/**

  @brief
  Enable the EHCI controller

  @param[in] PeiServices          The general PEI services
  @param[in] PeiPchEhciDev        The EHCI device
  @param[in] UsbControllerId      The USB Controller number

  @retval EFI_INVALID_PARAMETER   UsbControllerId is out of range
  @retval EFI_SUCCESS             The function completes successfully

**/
{
  UINTN BaseAddress;

  if (UsbControllerId >= PeiPchEhciDev->TotalEhciControllers) {
    return EFI_INVALID_PARAMETER;
  }

  BaseAddress = PeiPchEhciDev->MemBase[UsbControllerId];

  ///
  /// Assign base address register
  ///
  MmioWrite32 ((PCH_PCIE_EHCI_ADDR (UsbControllerId) + R_PCH_EHCI_MEM_BASE), BaseAddress);

  ///
  /// Enable VLV EHCI register
  ///
  MmioOr16 (
    (PCH_PCIE_EHCI_ADDR (UsbControllerId) + R_PCH_EHCI_COMMAND_REGISTER),
    (UINT16) (B_PCH_EHCI_COMMAND_BME | B_PCH_EHCI_COMMAND_MSE)
  );

  return EFI_SUCCESS;
}
