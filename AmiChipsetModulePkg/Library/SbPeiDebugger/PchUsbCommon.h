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
  PchUsbCommon.h

  @brief 
  Header file for the PCH USB Common Driver.
  
**/
#ifndef _USB_COMMON_H_
#define _USB_COMMON_H_

#include "PchAccess.h"
#include <Library/PchPlatformLib.h>
#include "PchInitCommon.h"
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>

#define USB_PR_CASE_0       0
#define USB_PR_CASE_1       1
#define USB_PR_CASE_2       2
#define TEN_MS_TIMEOUT      10000
#define PORT_RESET_TIMEOUT  10  /// 100 ms timeout for xHCI port reset

typedef struct {
  UINT8   Device;
  UINT8   Function;
} USB_CONTROLLER;

EFI_STATUS
EFIAPI
CommonUsbInit (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINT32                      EhciMmioBase,
  IN  UINT32                      XhciMmioBase,
  IN  UINT8                       BusNumber,
  IN  UINT32                      RootComplexBar,
  OUT UINT32                      *FuncDisableReg,
  IN  UINT8                       Revision
  )
/**

  @brief 
  Configures PCH USB controller

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] EhciMmioBase         Memory base address of EHCI Controller
  @param[in] XhciMmioBase         Memory base address of XHCI Controller
  @param[in] BusNumber            PCI Bus Number of the PCH device
  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] FuncDisableReg       Function Disable Register
  @param[in] Revision             The policy revision used for backward compatible check

  @retval EFI_INVALID_PARAMETER   The parameter of PchPlatformPolicy is invalid
  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
EFIAPI
CommonEhciHcsInit (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINT32                      EhciMmioBase,
  IN  UINT8                       BusNumber,
  IN  UINT8                       Revision,
  IN  UINT16                      LpcDeviceId,
  IN  UINT32                      RootComplexBar
  )
/**

  @brief 
  Performs basic configuration of PCH EHCI controller.

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] EhciMmioBase         Memory base address of EHCI Controller
  @param[in] BusNumber            PCI Bus Number of the PCH device
  @param[in] Revision             The policy revision used for backward compatible check
  @param[in] LpcDeviceId          The device ID of LPC
  @param[in] RootComplexBar       RootComplexBar value of this PCH device

  @retval EFI_SUCCESS             The function completed successfully

**/
;

VOID
CommonXhciHcInit (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINT32                      XhciMmioBase,
  IN  UINT8                       Revision,
  IN  UINT16                      LpcDeviceId,
  IN  UINTN                       XhciPciMmBase
  )
/**

  @brief 
  Performs basic configuration of PCH USB3 (xHCI) controller.

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] XhciMmioBase         Memory base address of xHCI Controller
  @param[in] Revision             The policy revision used for backward compatible check
  @param[in] LpcDeviceId          The device ID of LPC
  @param[in] XhciPciMmBase        XHCI PCI Base Address

  @retval None

**/
;

VOID
PerformXhciEhciPortSwitchingFlow (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINT32                      XhciMmioBase,
  IN  UINT8                       Revision,
  IN  UINT16                      LpcDeviceId,
  IN  UINTN                       XhciPciMmBase,
  IN  UINTN                       PciD31F0RegBase
  )
/**

  @brief 
  Performs basic configuration of PCH USB3 (xHCI) controller.

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] XhciMmioBase         Memory base address of XHCI Controller
  @param[in] Revision             The policy revision used for backward compatible check
  @param[in] LpcDeviceId          The device ID of LPC
  @param[in] XhciPciMmBase        XHCI PCI Base Address
  @param[in] PciD31F0RegBase      LPC PCI Base Address

  @retval None

**/
;

VOID
GetXhciPortCountAndSetPortRoutingMask (
  IN  UINTN                       XhciPciMmBase,
  OUT UINTN                       *HsPortCount,
  OUT UINTN                       *HsUsbrPortCount,
  OUT UINTN                       *SsPortCount
  )
/**

  @brief 
  Retrieves information about number of implemented xHCI ports
  and sets appropriate port mask registers.

  @param[in] XhciPciMmBase        XHCI PCI Base Address
  @param[in] HsPortCount          Count of High Speed Ports
  @param[in] HsUsbrPortCount      Count of USBr Port
  @param[in] SsPortCount          Count of Super Speed Ports

  @retval None

**/
;

VOID
XhciOverCurrentMapping (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINTN                       XhciPciMmBase
  )
/**

  @brief 
  Setup XHCI Over-Current Mapping

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] XhciPciMmBase        XHCI PCI Base Address

  @retval None

**/
;

VOID
EhciOverCurrentMapping (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINTN                       Ehci1PciMmBase
  )
/**

  @brief 
  Setup EHCI Over-Current Mapping

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] Ehci1PciMmBase       EHCI 1 PCI Base Address

  @retval None

**/
;

VOID
EhciPortDisableOverride (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINTN                       Ehci1PciMmBase
  )
/**

  @brief 
  Program Ehci Port Disable Override

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] Ehci1PciMmBase       EHCI 1 PCI Base Address
  @param[in] Ehci2PciMmBase       EHCI 2 PCI Base Address

  @retval None

**/
;

VOID
XhciPortDisableOverride (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINTN                       XhciPciMmBase
  )
/**

  @brief 
  Program Xhci Port Disable Override

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] XhciPciMmBase        XHCI PCI Base Address

  @retval None

**/
;

VOID
EhciUsbrEnable (
  IN  UINTN                       EhciPciMmBase
  )
/**

  @brief 
  Enable EHCI USBR device

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] Ehci1PciMmBase       Ehci 1 PCI Base Address
  @param[in] Ehci2PciMmBase       Ehci 2 PCI Base Address

  @retval None

**/
;

VOID
XhciMemorySpaceOpen (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINT32                      XhciMmioBase,
  IN  UINTN                       XhciPciMmBase
  )
/**

  @brief 
  Program and enable XHCI Memory Space

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] XhciMmioBase         Memory base address of XHCI Controller
  @param[in] XhciPciMmBase        XHCI PCI Base Address

  @retval None

**/
;

VOID
XhciMemorySpaceClose (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINT32                      XhciMmioBase,
  IN  UINTN                       XhciPciMmBase
  )
/**

  @brief 
  Clear and disable XHCI Memory Space

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] XhciMmioBase         Memory base address of XHCI Controller
  @param[in] XhciPciMmBase        XHCI PCI Base Address

  @retval None

**/
;

VOID
EhciPortLengthProgramming (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINT16                      LpcDeviceId,
  IN  UINT32                      RootComplexBar
  )
/**

  @brief 
  USB Initialization per the Port Length

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] LpcDeviceId          The device ID of LPC
  @param[in] RootComplexBar       RootComplexBar value of this PCH device

  @retval EFI_SUCCESS             Successfully completed
  @retval EFI_DEVICE_ERROR        Programming is failed

**/
;

VOID
UsbInitBeforeBoot (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
)
/**

  @brief
  Lock USB registers before boot

  @param[in] PchPlatformPolicy    The PCH Platform Policy

  @retval None

**/
;

VOID
ConfigureUsbClockGating (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN     UINT32                               RootComplexBar
  )
/**

  @brief
  Initialization USB Clock Gating registers

  @param[in] PchPlatformPolicy    The PCH Platform Policy
  @param[in] RootComplexBar       RootComplexBar value of this PCH device

  @retval None

**/
;

VOID
ConfigureEhciClockGating (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN     UINT32                               RootComplexBar
  )
/**

  @brief
  Initialization EHCI Clock Gating registers

  @param[in] PchPlatformPolicy    The PCH Platform Policy
  @param[in] RootComplexBar       RootComplexBar value of this PCH device

  @retval None

**/
;

VOID
ConfigureXhciClockGating (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  )
/**

  @brief
  Initialization XHCI Clock Gating registers

  @param[in] PchPlatformPolicy    The PCH Platform Policy

  @retval None

**/
;

VOID
ConfigureEhciDebug (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINT8                       BusNumber,
  IN  BOOLEAN                     IsEnable
)
/**

  @brief
  Enable/disable EHCI debug capability

  @param[in] UsbConfig            The PCH Platform Policy for USB configuration
  @param[in] BusNumber 
  @param[in] IsEnable
  
  @retval None

**/
;

#endif
