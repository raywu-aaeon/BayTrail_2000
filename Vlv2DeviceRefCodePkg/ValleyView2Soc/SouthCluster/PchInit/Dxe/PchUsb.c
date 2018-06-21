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
  Initializes PCH USB Controllers.

**/
#include "PchInit.h"
#include <Token.h> //AMI_OVERRIDE

VOID
ConfigureXhciAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  )
/**

  @brief
  Configures ports of the PCH USB3 (xHCI) controller
  just before OS boot.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_INVALID_PARAMETER   The parameter of PchPlatformPolicy is invalid
  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINTN   PciD20F0RegBase;
  UINT32  PortMask;
// AMI_OVERRIDE - Support AmiDebugRx - EIP112014 >>
//  DEBUG ((EFI_D_INFO, "ConfigureXhciAtBoot() Start\n"));
// AMI_OVERRIDE - Support AmiDebugRx - EIP112014 <<
  PciD20F0RegBase = MmPciAddress (
                      0,
                      PchPlatformPolicy->BusNumber,
                      PCI_DEVICE_NUMBER_PCH_XHCI,
                      PCI_FUNCTION_NUMBER_PCH_XHCI,
                      0
                      );

  ///
  /// VLV BIOS Spec Rev 0.5.0
  /// When the BIOS does not have xHCI pre-boot software available:
  /// Section 32.1.2 xHCI Enabled mode
  /// BIOS should route the Ports to the EHCI controller and prior to OS boot
  /// it should route the ports to the xHCI controller.
  ///
  if ((PchPlatformPolicy->UsbConfig->Usb30Settings.Mode == PCH_XHCI_MODE_ON) &&
      (PchPlatformPolicy->UsbConfig->Usb30Settings.PreBootSupport == PCH_DEVICE_DISABLE)) {
    ///
    /// VLV BIOS Spec Rev 0.5.0 Section 32.2.5 Routing of switchable USB Ports to
    /// xHCI Controller
    /// Step 1
    /// Done in GetXhciPortsNumber()
    /// Step 2
    /// Program D20:F0:D8h[5:0] to the value of xHCI D20:F0:DCh[5:0]
    ///
    PortMask = MmioRead32 (PciD20F0RegBase + R_PCH_XHCI_USB3PRM);

    MmioAndThenOr32 (
      PciD20F0RegBase + R_PCH_XHCI_USB3PR,
      (UINT32)~B_PCH_XHCI_USB3PR_USB3SSEN,
      PortMask
      );
    ///
    /// Step 3
    /// Program D20:F0:D0h[14:0] to the value of xHCI D20:F0:D4h[15:0]
    ///
    PortMask = MmioRead32 (PciD20F0RegBase + R_PCH_XHCI_USB2PRM);

    MmioAndThenOr32 (
      PciD20F0RegBase + R_PCH_XHCI_USB2PR,
      (UINT32)~B_PCH_XHCI_USB2PR_USB2HCSEL,
      PortMask
      );
    ///
    /// Note: Registers USB3PR[5:0] and USB2PR[14:0] are located in SUS well so BIOS doesn't
    ///       need to restore them during S3 resume, but needs to restore corresponding mask
    ///       registers. For iFFS resume from G3 state support, HC Switch driver will call
    ///       _OSC method to restore USB2PR and USB3PR.
    ///
    ///
  }

// AMI_OVERRIDE - Support AmiDebugRx - EIP112014 >>
//  DEBUG ((EFI_D_INFO, "ConfigureXhciAtBoot() End\n"));
// AMI_OVERRIDE - Support AmiDebugRx - EIP112014 <<
}

EFI_STATUS
ConfigureUsb (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN      UINT32                              RootComplexBar,
  IN OUT  UINT32                              *FuncDisableReg
  )
/**

  @brief
  Configures PCH USB controller

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar address of this PCH device
  @param[in] FuncDisableReg       Function Disable Register

  @retval EFI_INVALID_PARAMETER   The parameter of PchPlatformPolicy is invalid
  @retval EFI_SUCCESS             The function completed successfully

**/
{
  EFI_STATUS            Status;
  UINT8                 BusNumber;
  PCH_USB_CONFIG        *UsbConfig;
  UINT32                UsbFuncDisable;
  EFI_PHYSICAL_ADDRESS  EhciMemBaseAddress;
  EFI_PHYSICAL_ADDRESS  XhciMemBaseAddress;
// AMI_OVERRIDE - Fix Ctrl+Home recovery and AFUWIN recovery - EIP126330 EIP126249 >>
  UINTN					PciD31F0RegBase;
  UINTN					XhciPciMmBase;
  UINT32				PmcBase;
// AMI_OVERRIDE - Fix Ctrl+Home recovery and AFUWIN recovery - EIP126330 EIP126249 <<

  DEBUG ((EFI_D_INFO, "ConfigureUsb() Start\n"));

  BusNumber          = PchPlatformPolicy->BusNumber;
  UsbConfig          = PchPlatformPolicy->UsbConfig;
  EhciMemBaseAddress = 0x0ffffffff;

  Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateMaxAddressSearchBottomUp,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  N_PCH_EHCI_MEM_ALIGN,
                  V_PCH_EHCI_MEM_LENGTH,
                  &EhciMemBaseAddress,
                  mImageHandle,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  XhciMemBaseAddress = 0x0ffffffff;

  Status = gDS->AllocateMemorySpace (
                  EfiGcdAllocateMaxAddressSearchBottomUp,
                  EfiGcdMemoryTypeMemoryMappedIo,
                  N_PCH_XHCI_MEM_ALIGN,
                  V_PCH_XHCI_MEM_LENGTH,
                  &XhciMemBaseAddress,
                  mImageHandle,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {

    gDS->FreeMemorySpace (
           EhciMemBaseAddress,
           V_PCH_EHCI_MEM_LENGTH
           );

    return Status;
  }

  UsbFuncDisable = *FuncDisableReg;

  Status = CommonUsbInit (
             UsbConfig,
             (UINT32) EhciMemBaseAddress,
             (UINT32) XhciMemBaseAddress,
             BusNumber,
             RootComplexBar,
             &UsbFuncDisable,
             PchPlatformPolicy->Revision
             );
  *FuncDisableReg = UsbFuncDisable;

// AMI_OVERRIDE - Fix Ctrl+Home recovery and AFUWIN recovery - EIP126330 EIP126249 >>
  if (UsbConfig->Usb30Settings.Mode == PCH_XHCI_MODE_OFF) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting xHCI into D3 Hot State.\n"));
    PciD31F0RegBase = MmPciAddress (
	                      0,
	                      BusNumber,
	                      PCI_DEVICE_NUMBER_PCH_LPC,
	                      PCI_FUNCTION_NUMBER_PCH_LPC,
	                      0
	                    );
    XhciPciMmBase   = MmPciAddress (
	                      0,
	                      BusNumber,
	                      PCI_DEVICE_NUMBER_PCH_XHCI,
	                      PCI_FUNCTION_NUMBER_PCH_XHCI,
	                      0
	                    );
	  PmcBase      = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
    MmioOr32 ((UINTN) (XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS), B_PCH_XHCI_PWR_CNTL_STS_PWR_STS);
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_USH;
    MmioOr32 ((PmcBase + R_PCH_PMC_FUNC_DIS2),B_PCH_PMC_FUNC_DIS2_USH_SS_PHY );    
  }
// AMI_OVERRIDE - Fix Ctrl+Home recovery and AFUWIN recovery - EIP126330 EIP126249 <<
  ///
  /// Free allocated resources
  ///
  gDS->FreeMemorySpace (
        EhciMemBaseAddress,
        V_PCH_EHCI_MEM_LENGTH
        );

  gDS->FreeMemorySpace (
        XhciMemBaseAddress,
        V_PCH_XHCI_MEM_LENGTH
        );
  DEBUG ((EFI_D_INFO, "ConfigureUsb() End\n"));

  return EFI_SUCCESS;
}
