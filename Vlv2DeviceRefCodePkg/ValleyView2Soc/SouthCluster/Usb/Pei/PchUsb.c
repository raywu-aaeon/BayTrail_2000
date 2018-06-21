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
  PchUsb.c

  @brief
  VLV USB PEI Init

**/
#include "PchUsb.h"
#ifdef ECP_FLAG
EFI_GUID gPchUsbPolicyPpiGuid = PCH_USB_POLICY_PPI_GUID;
#endif

EFI_STATUS
InitializePchUsb (
#ifdef ECP_FLAG
  IN EFI_FFS_FILE_HEADER       *FileHandle,
  IN EFI_PEI_SERVICES          **PeiServices
#else
  IN EFI_PEI_FILE_HANDLE             *FfsHeader,
  IN CONST EFI_PEI_SERVICES          **PeiServices
#endif
)
/**

  @brief
  Initialize PCH USB PEIM

  @param[in] FfsHeader            Not used.
  @param[in] PeiServices          General purpose services available to every PEIM.

  @retval EFI_SUCCESS             The PCH USB PEIM is initialized successfully
  @retval Others                  All other error conditions encountered result in an ASSERT.

**/
{
  EFI_STATUS          Status;
  PCH_USB_POLICY_PPI  *UsbPolicyPpi;

  DEBUG ((EFI_D_INFO, "InitializePchUsb() Start\n"));

  ///
  /// Locate UsbPolicy PPI
  ///
  Status = (**PeiServices).LocatePpi (
                             PeiServices,
                             &gPchUsbPolicyPpiGuid,
                             0,
                             NULL,
                             (VOID **) &UsbPolicyPpi
                             );
  ASSERT_EFI_ERROR (Status);

  if (Status == EFI_SUCCESS) {
    ///
    /// Enable USB controller and install PeiUsbControllerPpi for USB recovery function
    ///
    switch (UsbPolicyPpi->Mode) {
      case EHCI_MODE:
        DEBUG ((EFI_D_ERROR, "Usb Recovery Mode : EHCI !\n"));
        DEBUG ((EFI_D_ERROR, "EhciMemBaseAddr:%0x!\n", UsbPolicyPpi->EhciMemBaseAddr));
        DEBUG ((EFI_D_ERROR, "EhciMemLength:%0x!\n", UsbPolicyPpi->EhciMemLength));
        InitForEHCI (PeiServices, UsbPolicyPpi);
        break;

      default:
        DEBUG ((EFI_D_ERROR, "*** Error: Invalid parameter or VLV does not have UHCI!\n"));
        ASSERT_EFI_ERROR (EFI_INVALID_PARAMETER);
        break;
    }
  }

  DEBUG ((EFI_D_INFO, "InitializePchUsb() End\n"));

  return Status;

}
