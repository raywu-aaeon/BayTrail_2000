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
  PchUsbInit.c

  @brief
  Initializes PCH USB Controllers.

**/
#include "PchInitPeim.h"

EFI_STATUS
EFIAPI
PchUsbInit (
  IN EFI_PEI_SERVICES             **PeiServices
  )
/**

  @brief
  The function performing USB init in PEI phase. This could be used by USB recovery
  or debug features that need USB initialization during PEI phase.
  Note: Before executing this function, please be sure that PCH_INIT_PPI.Initialize
  has been done and PchUsbPolicyPpi has been installed.

  @param[in] PeiServices    General purpose services available to every PEIM

  @retval EFI_SUCCESS       The function completed successfully
  @retval Others            All other error conditions encountered result in an ASSERT.

**/
{
  EFI_STATUS          Status;
  PCH_USB_POLICY_PPI  *PchUsbPolicyPpi;
  PCH_USB_CONFIG      *UsbConfig;
  UINT32              FuncDisableReg;
  UINTN               PciD31F0RegBase;
  UINT32              PmcBase;
  UINT32              MphyBase;
  UINT32              RootComplexBar;

  DEBUG ((EFI_D_INFO, "PchUsbInit() - Start\n"));

  ///
  /// Get PchUsbPolicy PPI for PCH_USB_CONFIG
  ///
  Status = (**PeiServices).LocatePpi (
                             PeiServices,
                             &gPchUsbPolicyPpiGuid,
                             0,
                             NULL,
                             (VOID **) &PchUsbPolicyPpi
                             );

  if (Status == EFI_SUCCESS) {
    UsbConfig       = PchUsbPolicyPpi->UsbConfig;

    PciD31F0RegBase = MmPciAddress (0,
                        DEFAULT_PCI_BUS_NUMBER_PCH,
                        PCI_DEVICE_NUMBER_PCH_LPC,
                        PCI_FUNCTION_NUMBER_PCH_LPC,
                        0
                      );

    PmcBase         = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
    MphyBase        = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_MPHY_BASE) & B_PCH_LPC_MPHY_BASE_BAR;
    RootComplexBar  = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_RCBA) & B_PCH_LPC_RCBA_BAR;
    FuncDisableReg  = MmioRead32 (PmcBase + R_PCH_PMC_FUNC_DIS);

    Status = CommonUsbInit (
               UsbConfig,
               (UINT32) PchUsbPolicyPpi->EhciMemBaseAddr,
               (UINT32) PchUsbPolicyPpi->XhciMemBaseAddr,
               0,
               RootComplexBar,
               &FuncDisableReg,
               PchUsbPolicyPpi->Revision
               );

    ASSERT_EFI_ERROR (Status);

    MmioWrite32 ((UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS), (UINT32) (FuncDisableReg));
    ///
    /// Reads back for posted write to take effect
    ///
    MmioRead32 ((UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS));
  }

  DEBUG ((EFI_D_INFO, "PchUsbInit() - End\n"));

  return Status;

}
