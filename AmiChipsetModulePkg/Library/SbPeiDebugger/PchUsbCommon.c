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
  PchUsbCommon.c
  
  @brief 
  Initializes PCH USB Controllers. 

**/
#include "PchUsbCommon.h"
#include <token.h>

#define XHCI_4590929_WORKAROUND          1


const USB_CONTROLLER EhciControllersMap[PchEhciControllerMax] = {
  {
    PCI_DEVICE_NUMBER_PCH_USB,
    PCI_FUNCTION_NUMBER_PCH_EHCI
  }
};

const UINTN PORTSCxUSB2[] = {
  R_PCH_XHCI_PORTSC01USB2,
  R_PCH_XHCI_PORTSC02USB2,
  R_PCH_XHCI_PORTSC03USB2,
  R_PCH_XHCI_PORTSC04USB2,
  R_PCH_XHCI_PORTSC05USB2,
  R_PCH_XHCI_PORTSC06USB2
};

const UINTN PORTSCxUSB3[] = {
  R_PCH_XHCI_PORTSC1USB3
};

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
{
  UINTN           PciD31F0RegBase;
  UINTN           XhciPciMmBase;
  UINTN           Ehci1PciMmBase;
  UINT16          LpcDeviceId;
  UINT16          AcpiBase;
  UINT16          RegData16;
  UINT32          PmcBase;

//(EIP112014-)  DEBUG ((EFI_D_INFO, "CommonUsbInit() - Start\n"));

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
  Ehci1PciMmBase  = MmPciAddress (
                      0,
                      BusNumber,
                      EhciControllersMap[PchEhci1].Device,
                      EhciControllersMap[PchEhci1].Function,
                      0
                    );

  PmcBase      = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
  AcpiBase     = MmioRead16 (PciD31F0RegBase + R_PCH_LPC_ACPI_BASE) & B_PCH_LPC_ACPI_BASE_BAR;
  LpcDeviceId  = (UINT16) ((MmioRead16 (PciD31F0RegBase + R_PCH_LPC_REG_ID) & B_PCH_LPC_DEVICE_ID) >> 16);

  ///
  /// Init USB Host Controllers
  ///
  CommonEhciHcsInit (
    UsbConfig,
    EhciMmioBase,
    BusNumber,
    Revision,
    LpcDeviceId,
    RootComplexBar
    );

  ///
  /// Assign memory resources
  ///
  XhciMemorySpaceOpen (
    UsbConfig,
    XhciMmioBase,
    XhciPciMmBase
    );

  CommonXhciHcInit (
    UsbConfig,
    XhciMmioBase,
    Revision,
    LpcDeviceId,
    XhciPciMmBase
    );

  ///
  /// Init Port Switching Flow
  ///
  PerformXhciEhciPortSwitchingFlow (
    UsbConfig,
    XhciMmioBase,
    Revision,
    LpcDeviceId,
    XhciPciMmBase,
    PciD31F0RegBase
    );

  ///
  /// Setup USB Over-Current Mapping.
  ///
  EhciOverCurrentMapping (
    UsbConfig,
    Ehci1PciMmBase
    );

  XhciOverCurrentMapping (
    UsbConfig,
    XhciPciMmBase
    );

  ///
  /// USB Initialization per Port Length
  ///
  EhciPortLengthProgramming (
    UsbConfig,
    LpcDeviceId,
    RootComplexBar
    );

  ///
  /// Support USB Per-Port Disable Control Override feature
  ///
  if (0) {
//  if (UsbConfig->UsbPerPortCtl == PCH_DEVICE_ENABLE) {
    ///
    /// Open the Per-Port Disable Control Override
    ///
    RegData16 = IoRead16 ((UINTN) ((UINT64) (AcpiBase + R_PCH_UPRWC)));
    RegData16 |= B_PCH_UPRWC_WR_EN;
    IoWrite16 ((UINTN) ((UINT64) (AcpiBase + R_PCH_UPRWC)), RegData16);
    PCH_INIT_COMMON_SCRIPT_IO_WRITE (
      EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
      EfiBootScriptWidthUint16,
      (UINTN) (AcpiBase + R_PCH_UPRWC),
      1,
      &RegData16
      );

    EhciPortDisableOverride (
      UsbConfig,
      Ehci1PciMmBase
      );

    XhciPortDisableOverride (
      UsbConfig,
      XhciPciMmBase
      );

    ///
    /// Close the Per-Port Disable Control Override
    ///
    RegData16 &= (~B_PCH_UPRWC_WR_EN);
    IoWrite16 ((UINTN) ((UINT64) (AcpiBase + R_PCH_UPRWC)), RegData16);
    PCH_INIT_COMMON_SCRIPT_IO_WRITE (
      EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
      EfiBootScriptWidthUint16,
      (UINTN) (AcpiBase + R_PCH_UPRWC),
      1,
      &RegData16
      );
  }
  ///
  /// Support USBR feature
  ///
  if (0) {
//  if (UsbConfig->Ehci1Usbr == PCH_DEVICE_ENABLE &&
//      UsbConfig->Usb20Settings[PchEhci1].Enable == PCH_DEVICE_ENABLE) {
    EhciUsbrEnable (Ehci1PciMmBase);
  }
  ///
  /// Clear memory resources
  ///
  XhciMemorySpaceClose (
    UsbConfig,
    XhciMmioBase,
    XhciPciMmBase
    );

  ///
  /// Check to disable USB Controllers
  ///
  if (0) {
//  if (UsbConfig->Usb20Settings[PchEhci1].Enable == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3hot state.
    ///
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting EHCI into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (Ehci1PciMmBase + R_PCH_EHCI_PWR_CNTL_STS), V_PCH_EHCI_PWR_CNTL_STS_PWR_STS_D3);
    PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
      EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
      EfiBootScriptWidthUint32,
      (UINTN) (Ehci1PciMmBase + R_PCH_EHCI_PWR_CNTL_STS),
      1,
      (VOID *) (UINTN) (Ehci1PciMmBase + R_PCH_EHCI_PWR_CNTL_STS)
      );
    ///
    /// Program function disable bit in IOSF2SPXB bridge.
    ///
    MmioOr32 ((UINTN) (RootComplexBar + 0x220), 0x01);
    PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
      EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
      EfiBootScriptWidthUint32,
      (UINTN) (RootComplexBar + 0x220),
      1,
      (VOID *) (UINTN) (RootComplexBar + 0x220)
      );
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_USB;
  }
  if (1) {
//  if (UsbConfig->Usb30Settings.Mode == PCH_XHCI_MODE_OFF) {
//(EIP112014-)     DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting xHCI into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS), B_PCH_XHCI_PWR_CNTL_STS_PWR_STS);
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_USH;
    MmioOr32 ((PmcBase + R_PCH_PMC_FUNC_DIS2),B_PCH_PMC_FUNC_DIS2_USH_SS_PHY );    
  }

  //EHCI debug is only availble when EHCI is enabled
  if (1) {
//  if (UsbConfig->Usb20Settings[PchEhci1].Enable == PCH_DEVICE_ENABLE) { 
    if (1) {
//    if (UsbConfig->EhciDebug == PCH_EHCI_DEBUG_ON){
//(EIP112014-)      DEBUG ((EFI_D_INFO, "enable  EHCI debug\n"));
      ConfigureEhciDebug (UsbConfig, BusNumber, TRUE);
    } else {
//(EIP112014-)      DEBUG ((EFI_D_INFO, "disable EHCI debug\n"));
      ConfigureEhciDebug (UsbConfig, BusNumber, FALSE);
    }
  }
//(EIP112014-)  DEBUG ((EFI_D_INFO, "CommonUsbInit() - End\n"));

  return EFI_SUCCESS;
}

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
{
  UINTN           EhciPciMmBase;
  UINT8           Index;
  UINT16          PciCmd;
  BOOLEAN         SkipRst;
  UINT32          DwordReg;
  UINT16          WordReg;

  for (Index = 0; Index < PchEhciControllerMax; Index++) {
    EhciPciMmBase = MmPciAddress (0,
                      BusNumber,
                      EhciControllersMap[Index].Device,
                      EhciControllersMap[Index].Function,
                      0
                    );
    ///
    /// Set EHCI structural parameter
    ///
    if (0) {
//    if (UsbConfig->Usb20Settings[Index].Enable == PCH_DEVICE_DISABLE) {
      MmioWrite32 (EhciPciMmBase + R_PCH_EHCI_MEM_BASE, 0);
      MmioWrite16 (EhciPciMmBase + R_PCH_EHCI_COMMAND_REGISTER, 0);
    } else {
      PciCmd = 0;
      ///
      /// For some cases, like usb debug mode, the Ehci memory resource will have been assigned and
      /// enabled here. If so, then set SkipRst flag to skip the steps that are for Ehci memory
      /// resource clear and host controller reset
      ///
      if ((MmioRead32 (EhciPciMmBase + R_PCH_EHCI_MEM_BASE) == 0) &&
          !(MmioRead16 (EhciPciMmBase + R_PCH_EHCI_COMMAND_REGISTER) & B_PCH_EHCI_COMMAND_MSE)) {
        MmioWrite32 ((EhciPciMmBase + R_PCH_EHCI_MEM_BASE), EhciMmioBase);
        PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
          EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
          EfiBootScriptWidthUint32,
          (UINTN) (EhciPciMmBase + R_PCH_EHCI_MEM_BASE),
          1,
          (VOID *) (UINTN) (EhciPciMmBase + R_PCH_EHCI_MEM_BASE)
          );
        ///
        /// Clear SkipRst flag
        ///
        SkipRst = FALSE;
      } else {
        ///
        /// Use the memory address of Ehci controller that has been assigned before initialization
        /// to do the programming.
        ///
        EhciMmioBase = MmioRead32 (EhciPciMmBase + R_PCH_EHCI_MEM_BASE);
        ///
        /// Save Pci command register
        ///
        PciCmd = MmioRead16 (EhciPciMmBase + R_PCH_EHCI_COMMAND_REGISTER);
        ///
        /// Set SkipRst flag
        ///
        SkipRst = TRUE;
      }

      MmioOr16 (
        (EhciPciMmBase + R_PCH_EHCI_COMMAND_REGISTER),
        (UINT16) (B_PCH_EHCI_COMMAND_MSE | B_PCH_EHCI_COMMAND_BME)
        );
      PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
        EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
        EfiBootScriptWidthUint16,
        (UINTN) (EhciPciMmBase + R_PCH_EHCI_COMMAND_REGISTER),
        1,
        (VOID *) (UINTN) (EhciPciMmBase + R_PCH_EHCI_COMMAND_REGISTER)
        );
      ///
      /// VLV BIOS Spec Rev 0.6.2 Section 12.8
      /// Additional Programming Requirements during USB Initialization
      ///
      /// Step 1
      /// Program D29:MEM_BASE + 20h [1] = 1b.
      /// This should be done before FS/LS initialization and also after S4/S5
      ///
      /// For some cases, like usb debug mode, we will skip this step, in case something will be destroyed
      /// after doing host controller reset
      ///
      if (!SkipRst) {
        MmioOr16 ((EhciMmioBase + R_PCH_EHCI_USB2CMD), B_PCH_EHCI_USB2CMD_HCRESET);
      }
      ///
      /// Step 2
      /// Enable S0 PLL Shutdown
      /// Program the following registers on D29 F0 PCI CFG space
      /// Program EHCI D29:F0 + 0x7A [12][10][7][6][4][3][2][1] = 1b
      ///
      WordReg = MmioRead16 (EhciPciMmBase + R_PCH_EHCI_ESUBFEN);
      WordReg |= (UINT16) (B_PCH_EHCI_ESUBFEN_S0PSEN | B_PCH_EHCI_ESUBFEN_S0PSSSS | B_PCH_EHCI_ESUBFEN_S0PSCFS | B_PCH_EHCI_ESUBFEN_S0PSD3S);
      WordReg |= (UINT16) (B_PCH_EHCI_ESUBFEN_S0PSRDPOS | B_PCH_EHCI_ESUBFEN_S0PSRDDCS | B_PCH_EHCI_ESUBFEN_S0PSRDDS | B_PCH_EHCI_ESUBFEN_S0PSRDSS);
      MmioWrite16 (
        (UINTN) (EhciPciMmBase + R_PCH_EHCI_ESUBFEN),
        WordReg
        );
      PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
        EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
        EfiBootScriptWidthUint16,
        (UINTN) (EhciPciMmBase + R_PCH_EHCI_ESUBFEN),
        1,
        (VOID *) (UINTN) (EhciPciMmBase + R_PCH_EHCI_ESUBFEN)
        );
      ///
      /// Step 3
      /// Enable SB local clock gating.
      /// Program EHCI D29:F0 + 0x7C [14][3][2] = 1b
      ///
      DwordReg = MmioRead32 (EhciPciMmBase + R_PCH_EHCI_EHCSUSCFG);
      DwordReg |= (UINT32) (B_PCH_EHCI_EHCSUSCFG_RPESROED | B_PCH_EHCI_EHCSUSCFG_EEDFHEPFD);
      MmioWrite32 (
        (UINTN) (EhciPciMmBase + R_PCH_EHCI_EHCSUSCFG),
        DwordReg
        );
      PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
        EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
        EfiBootScriptWidthUint32,
        (UINTN) (EhciPciMmBase + R_PCH_EHCI_EHCSUSCFG),
        1,
        (VOID *) (UINTN) (EhciPciMmBase + R_PCH_EHCI_EHCSUSCFG)
        );

      DwordReg = MmioRead32 (EhciPciMmBase + R_PCH_EHCI_EHCIIR3);
      DwordReg |= (UINT32) (B_PCH_EHCI_EHCIIR3_PCEPEDP);
      MmioWrite32 (
        (UINTN) (EhciPciMmBase + R_PCH_EHCI_EHCIIR3),
        DwordReg
        );
      PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
        EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
        EfiBootScriptWidthUint32,
        (UINTN) (EhciPciMmBase + R_PCH_EHCI_EHCIIR3),
        1,
        (VOID *) (UINTN) (EhciPciMmBase + R_PCH_EHCI_EHCIIR3)
        );
      ///
      /// Step 5
      /// Program the following in USB2 SHIP to enable dynamic clock gating.
      /// PortId = 0x43, Offset 0x4001 = 0x000000CE
      ///
      PchMsgBusAndThenOr32 (
        0x43,
        0x4001,
        DwordReg,
        0xFFFFFF00,
        0x000000CE,
        0x06,
        0x07
      );
      ///
      /// Step 6
      /// Program the following registers on the IOSF/SPXB Bridge Private CFG space
      ///

      //
      // Program RCBA + 200h [31:0] = 1h
      //
      DwordReg = MmioRead32 (RootComplexBar + 0x200);
      DwordReg = (UINT32) (BIT0);
      MmioWrite32 (
        (UINTN) (RootComplexBar + 0x200),
        DwordReg
        );
      PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
        EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
        EfiBootScriptWidthUint32,
        (UINTN) (RootComplexBar + 0x200),
        1,
        (VOID *) (UINTN) (RootComplexBar + 0x200)
        );

      //
      // Program RCBA + 204h [31:0] = 2h
      //
      DwordReg = MmioRead32 (RootComplexBar + 0x204);
      DwordReg = (UINT32) (BIT1);
      MmioWrite32 (
        (UINTN) (RootComplexBar + 0x204),
        DwordReg
        );
      PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
        EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
        EfiBootScriptWidthUint32,
        (UINTN) (RootComplexBar + 0x204),
        1,
        (VOID *) (UINTN) (RootComplexBar + 0x204)
        );

      //
      // Program RCBA + 208h [31:0] = 0h
      //
      DwordReg = MmioRead32 (RootComplexBar + 0x208);
      DwordReg = (UINT32) (0x00);
      MmioWrite32 (
        (UINTN) (RootComplexBar + 0x208),
        DwordReg
        );
      PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
        EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
        EfiBootScriptWidthUint32,
        (UINTN) (RootComplexBar + 0x208),
        1,
        (VOID *) (UINTN) (RootComplexBar + 0x208)
        );

      //
      // Program RCBA + 240h [4] = 0h
      // Program RCBA + 240h [3:0] = 2h
      //
      DwordReg = MmioRead32 (RootComplexBar + 0x240);
      DwordReg &= (UINT32) ~(BIT4 | BIT3 | BIT2 | BIT1 | BIT0);
      DwordReg |= (UINT32) (BIT4 | BIT1);
      MmioWrite32 (
        (UINTN) (RootComplexBar + 0x240),
        DwordReg
        );
      PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
        EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
        EfiBootScriptWidthUint32,
        (UINTN) (RootComplexBar + 0x240),
        1,
        (VOID *) (UINTN) (RootComplexBar + 0x240)
        );
      ///
      /// Program RCBA + 318h [9] = 0h
      /// Program RCBA + 318h [8] = 0h
      /// Program RCBA + 318h [6] = 0h
      /// Program RCBA + 318h [5] = 0h
      /// Program RCBA + 318h [4] = 0h
      /// Program RCBA + 318h [3] = 0h
      /// Program RCBA + 318h [2] = 1h
      /// Program RCBA + 318h [1] = 1h
      /// Program RCBA + 318h [0] = 1h
      ///
      DwordReg = MmioRead32 (RootComplexBar + 0x318);
      DwordReg &= (UINT32) ~(BIT9 | BIT8 | BIT6 | BIT5 | BIT4 | BIT3);
      DwordReg |= (UINT32) (BIT2 | BIT1 | BIT0);
      MmioWrite32 (
        (UINTN) (RootComplexBar + 0x318),
        DwordReg
        );
      PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
        EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
        EfiBootScriptWidthUint32,
        (UINTN) (RootComplexBar + 0x318),
        1,
        (VOID *) (UINTN) (RootComplexBar + 0x318)
        );

      //
      // Program RCBA + 31Ch [3:2] = 0h
      // Program RCBA + 31Ch [1:0] = 3h
      //
      DwordReg = MmioRead32 (RootComplexBar + 0x31C);
      DwordReg &= (UINT32) ~(BIT3 | BIT2);
      DwordReg |= (UINT32) (BIT1 | BIT0);
      MmioWrite32 (
        (UINTN) (RootComplexBar + 0x31C),
        DwordReg
        );
      PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
        EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
        EfiBootScriptWidthUint32,
        (UINTN) (RootComplexBar + 0x31C),
        1,
        (VOID *) (UINTN) (RootComplexBar + 0x31C)
        );

      if (SkipRst) {
        ///
        /// Restore PCI command register
        ///
        MmioWrite16 ((EhciPciMmBase + R_PCH_EHCI_COMMAND_REGISTER), PciCmd);
      } else {
        ///
        /// Clear memory resource and command register after initialization
        ///
        MmioAnd16 (
          (EhciPciMmBase + R_PCH_EHCI_COMMAND_REGISTER),
          (UINT16)~(B_PCH_EHCI_COMMAND_MSE | B_PCH_EHCI_COMMAND_BME)
          );
        MmioWrite32 ((EhciPciMmBase + R_PCH_EHCI_MEM_BASE), 0);
      }
    }
  }

  return EFI_SUCCESS;
}

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
{
  ///
  /// Check if XHCI disabled, Exit function.
  ///
  if (1) {
//  if (UsbConfig->Usb30Settings.Mode == PCH_XHCI_MODE_OFF) {
    return;
  }
  ///
  /// VLV BIOS Spec Rev 0.5, Section 32.2 xHCI controller setup
  ///
  /// Step 7
  /// Set xHCIBAR + 0Ch[31:16] = 200h
  ///
  MmioAndThenOr32 (
    (XhciMmioBase + R_PCH_XHCI_HCSPARAMS3),
    (UINT32) 0x0000FFFF,
    (UINT32) 0x02000000
    );
  ///
  /// Step 8
  /// Set xHCIBAR + 0Ch[7:0] = 0Ah
  ///
  MmioAndThenOr32 (
    (XhciMmioBase + R_PCH_XHCI_HCSPARAMS3),
    (UINT32) 0xFFFFFF00,
    (UINT32) 0x0000000A
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciMmioBase + R_PCH_XHCI_HCSPARAMS3),
    1,
    (VOID *) (UINTN) (XhciMmioBase + R_PCH_XHCI_HCSPARAMS3)
    );
  ///
  /// Set D20:F0:44h[2:0] to 111b
  /// Set D20:F0:44h[7:6] to 01b
  /// Set D20:F0:44h[9:8] to 01b
  /// Note: Only update D20:F0:44h by word since D20:F0:44h[31] is write-once bit
  ///
  MmioAndThenOr16 (
    XhciPciMmBase + R_PCH_XHCI_XHCC2,
    (UINT16) ~(B_PCH_XHCI_XHCC2_RDREQSZCTRL),
    (UINT16) (V_PCH_XHCI_XHCC2_RDREQSZCTRL_64)
    );
  MmioAndThenOr16 (
    XhciPciMmBase + R_PCH_XHCI_XHCC2,
    (UINT16) ~(B_PCH_XHCI_XHCC2_SWADMIL1IHB),
    (UINT16) (BIT6)
    );
  MmioAndThenOr16 (
    XhciPciMmBase + R_PCH_XHCI_XHCC2,
    (UINT16) ~(B_PCH_XHCI_XHCC2_SWACXIHB),
    (UINT16) (BIT8)
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint16,
    (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCC2),
    1,
    (VOID *) (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCC2)
    );
  ///
  /// Step 6
  /// Set xHCIBAR + 8058h[16,8] to 1b, 0b
  ///
  MmioAndThenOr32 (
    (XhciMmioBase + 0x8058),
    (UINT32)~(BIT8),
    (UINT32) (BIT16)
    );

  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciMmioBase + 0x8058),
    1,
    (VOID *) (UINTN) (XhciMmioBase + 0x8058)
    );
  ///
  /// Step 9
  /// Set xHCIBAR + 8094h[23] to 1b
  ///
  MmioOr32 (XhciMmioBase + 0x8094, (UINT32) (BIT23));
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciMmioBase + 0x8094),
    1,
    (VOID *) (UINTN) (XhciMmioBase + 0x8094)
    );
  ///
  /// Step 3
  /// Set xHCIBAR + 80E0h[16] to 0b
  /// Set xHCIBAR + 80E0h[9] to 0b
  /// Set xHCIBAR + 80E0h[6] to 1b
  ///
  MmioAndThenOr32 (
    XhciMmioBase + 0x80E0,
    (UINT32) ~(BIT16 | BIT9),
    (UINT32) (BIT6)
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciMmioBase + 0x80E0),
    1,
    (VOID *) (UINTN) (XhciMmioBase + 0x80E0)
    );
  ///
  /// Step 1
  /// Set xHCIBAR + 8110h[20,11,2] to 1b, 1b, 0b
  ///
  MmioAndThenOr32 (
    (XhciMmioBase + 0x8110),
    (UINT32)~(BIT2),
    (UINT32) (BIT20 | BIT11)
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciMmioBase + 0x8110),
    1,
    (VOID *) (UINTN) (XhciMmioBase + 0x8110)
    );
  ///
  /// Set xHCIBAR + 8144h[8] to 1b
  ///
  MmioOr32 (XhciMmioBase + 0x8144, (UINT32) (BIT8));
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciMmioBase + 0x8144),
    1,
    (VOID *) (UINTN) (XhciMmioBase + 0x8144)
    );
  ///
  /// Set xHCIBAR + 8154h[21,13,3] to 0b, 1b, 0b
  ///
  MmioAndThenOr32 (
    (XhciMmioBase + 0x8154),
    (UINT32)~(BIT21 | BIT3),
    (UINT32) (BIT13)
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciMmioBase + 0x8154),
    1,
    (VOID *) (UINTN) (XhciMmioBase + 0x8154)
    );
  ///
  /// Set xHCIBAR + 816Ch[19,18,17] to 0b for A0 stepping, 1b for other steppings
  /// Set xHCIBAR + 816Ch[16] to 0b
  /// Set xHCIBAR + 816Ch[14] to 1b
  /// Set xHCIBAR + 816Ch[13:12] to 00b
  /// Set xHCIBAR + 816Ch[11:8] to 0000b
  /// Set xHCIBAR + 816Ch[7] to 1b
  /// Set xHCIBAR + 816Ch[6] to 0b
  /// Set xHCIBAR + 816Ch[5] to 1b
  /// Set xHCIBAR + 816Ch[4] to 1b
  /// Set xHCIBAR + 816Ch[3] to 1b
  /// Set xHCIBAR + 816Ch[2] to 1b
  /// Set xHCIBAR + 816Ch[1] to 0b
  /// Set xHCIBAR + 816Ch[0] to 0b
  ///
  MmioAndThenOr32 (
    (XhciMmioBase + 0x816C),
    (UINT32) (0xFFF08000),
    (UINT32) (0x000040BC)
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciMmioBase + 0x816C),
    1,
    (VOID *) (UINTN) (XhciMmioBase + 0x816C)
    );
	
#if XHCI_4590929_WORKAROUND 
  ///
  /// Set xHCIBAR + 8174h[31,28,24] to 0b, 1b, 1b
  /// Set xHCIBAR + 8174h[23:12] to 0h
  /// Set xHCIBAR + 8174h[11:0] to 0h
  ///
  MmioAndThenOr32 (
    (XhciMmioBase + 0x8174),
    (UINT32) (0x00),
    (UINT32) (BIT28 | BIT24)
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciMmioBase + 0x8174),
    1,
    (VOID *) (UINTN) (XhciMmioBase + 0x8174)
    );
#endif	
  ///
  /// Set xHCIBAR + 8178h[12:0] to 0h
  ///
  MmioAndThenOr32 (
    (XhciMmioBase + 0x8178),
    (UINT32)~(0xFFFFE000),
    (UINT32) (0x00)
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciMmioBase + 0x8178),
    1,
    (VOID *) (UINTN) (XhciMmioBase + 0x8178)
    );
  ///
  /// Set xHCIBAR + 8164h[7:0] to FFh
  ///
  MmioOr8 (XhciMmioBase + 0x8164, (UINT8) (0xFF));
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint8,
    (UINTN) (XhciMmioBase + 0x8164),
    1,
    (VOID *) (UINTN) (XhciMmioBase + 0x8164)
    );
}

VOID
ConfigureXhciClockGating (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  )
/**

  @brief
  Initialization XHCI Clock Gating registers

  @param[in] PchPlatformPolicy    The PCH Platform Policy
  @param[in] RootComplexBar       RootComplexBar value of this PCH device

  @retval None

**/
{
  UINT32      XhccCfg;
  UINTN       XhciPciMmBase;
  UINT8       Data8;
  UINT16      Data16;
  UINT32      Data32And;
  UINT32      Data32Or;
  XhciPciMmBase = MmPciAddress (
                    0,
                    PchPlatformPolicy->BusNumber,
                    PCI_DEVICE_NUMBER_PCH_XHCI,
                    PCI_FUNCTION_NUMBER_PCH_XHCI,
                    0
                    );
  ///
  /// VLV BIOS Spec Rev 0.5, Section 32.2.1 xHCI controller setup
  /// Step 10
  /// Set D20:F0:40h[21:19] to 110b
  /// Set D20:F0:40h[18] to 1b
  /// Set D20:F0:40h[10:8] to 001b
  ///
  XhccCfg  = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_XHCC1);
  XhccCfg &= (UINT32) ~(B_PCH_XHCI_XHCC1_URD | B_PCH_XHCI_XHCC1_IIL1E | B_PCH_XHCI_XHCC1_L23HRAWC);
  XhccCfg |= (UINT32) (V_PCH_XHCI_XHCC1_IIL1E_1024 | B_PCH_XHCI_XHCC1_XHCIL1E | V_PCH_XHCI_XHCC1_L23HRAWC_128);
  Data16   = (UINT16)XhccCfg;
  Data8    = (UINT8)(XhccCfg >> 16);
  MmioWrite16 (XhciPciMmBase + R_PCH_XHCI_XHCC1, Data16);
  MmioWrite8  (XhciPciMmBase + R_PCH_XHCI_XHCC1 + 2, Data8);
  ///
  /// Step 11
  /// Set D20:F0:44h[5:3] to 001b
  ///
  MmioAndThenOr16 (
    (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCC2),
    (UINT16) ~(B_PCH_XHCI_XHCC2_L1FP2CGWC),
    (UINT16) (V_PCH_XHCI_XHCC2_L1FP2CGWC_128)
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint8,
    (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCC2),
    1,
    (VOID *) (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCC2)
    );
  ///
  ///  Enabling Clock Gating
  /// 4. USB 1.1 / USB 2.0 / USB 3.0
  /// c. USB 3.0
  /// Step c.2
  /// Set D20:F0:A0h[19] to 0b
  /// Set D20:F0:A0h[18] to 1b
  ///
  MmioAndThenOr32 (
    (UINTN) (XhciPciMmBase + 0xA0),
    (UINT32) ~(BIT19),
    (UINT32) (BIT18)
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciPciMmBase + 0xA0),
    1,
    (VOID *) (UINTN) (XhciPciMmBase + 0xA0)
    );
  ///
  ///  Enabling Clock Gating
  /// 4. USB 1.1 / USB 2.0 / USB 3.0
  /// c. USB 3.0
  /// Step c.2
  /// Set D20:F0:A4h[15:0] to 0h
  ///
  MmioAnd16 ((UINTN) (XhciPciMmBase + 0xA4), (UINT16) 0x00);
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint16,
    (UINTN) (XhciPciMmBase + 0xA4),
    1,
    (VOID *) (UINTN) (XhciPciMmBase + 0xA4)
    );
  ///
  /// Set D20:F0:B0h[18] to 0b
  /// Set D20:F0:B0h[17] to 0b
  /// Set D20:F0:B0h[14] to 0b
  /// Set D20:F0:B0h[13] to 0b
  ///
  MmioAnd32 (
    (UINTN) (XhciPciMmBase + 0xB0),
    (UINT32) ~(BIT18 | BIT17 | BIT14 | BIT13)
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCLKGTEN),
    1,
    (VOID *) (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCLKGTEN)
    );
  ///
  /// Step c.1
  /// Set D20:F0:50h[28] to 0b
  /// Set D20:F0:50h[27] to 1b
  /// Set D20:F0:50h[26] to 0b
  /// Set D20:F0:50h[25] to 1b
  /// Set D20:F0:50h[24] to 1b
  /// Set D20:F0:50h[23:20] to 1100b
  /// Set D20:F0:50h[19:16] to 1110b
  /// Set D20:F0:50h[15] to 0b
  /// Set D20:F0:50h[14] to 1b
  /// Set D20:F0:50h[13] to 1b
  /// Set D20:F0:50h[12] to 0b
  /// Set D20:F0:50h[11:10] to 11b
  /// Set D20:F0:50h[9:8] to 10b
  /// Set D20:F0:50h[7:5] to 010b
  /// Set D20:F0:50h[4] to 1b
  /// Set D20:F0:50h[3] to 1b
  /// Set D20:F0:50h[2] to 1b
  /// Set D20:F0:50h[1] to 1b
  /// Set D20:F0:50h[0] to 1b
  ///
  Data32And = (UINT32) 0x00;
  Data32Or  = (UINT32) (0x0BCE6E5F);
  MmioAndThenOr32 (
    (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCLKGTEN),
    Data32And,
    Data32Or
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCLKGTEN),
    1,
    (VOID *) (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCLKGTEN)
    );
}

VOID
UsbInitBeforeBoot (
  IN  DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
)
/**

  @brief
  Lock USB registers before boot

  @param[in] PchPlatformPolicy    The PCH Platform Policy

  @retval None

**/
{
  UINT32  XhccCfg;
  UINT32  XhccCfg2;
  UINTN   XhciPciMmBase;
  XhciPciMmBase   = MmPciAddress (
                      0,
                      PchPlatformPolicy->BusNumber,
                      PCI_DEVICE_NUMBER_PCH_XHCI,
                      PCI_FUNCTION_NUMBER_PCH_XHCI,
                      0
                      );
  if (0) {
//  if (PchPlatformPolicy->UsbConfig->Usb30Settings.Mode != PCH_XHCI_MODE_OFF) {
    ///
    /// VLV BIOS Spec xHCI controller setup
    /// Note:
    /// D20:F0:40h is write once register.
    /// Unsupported Request Detected bit is write clear
    ///
    XhccCfg = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_XHCC1);
    XhccCfg &= (UINT32) ~(B_PCH_XHCI_XHCC1_URD);
    ///
    /// VLV BIOS Spec Rev 0.5, Section 32.2.3 Locking xHCI Register Settings
    /// VLV BIOS Spec Locking xHCI Register settings
    /// After xHCI is initialized, BIOS should lock the xHCI configuration registers to RO.
    /// This prevent any unintended changes. There is also a lockdown feature for OverCurrent
    /// registers. BIOS should set these bits to lock down the settings prior to end of POST.
    /// 1. Set Access Control bit at D20:F0:40h[31] to 1b to lock xHCI register settings.
    /// 2. Set OC Configuration Done bit at D20:F0:44h[31] to lock overcurrent mappings from
    ///    further changes.
    ///
    XhccCfg2 = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_XHCC2);
    ///
    /// D20:F0:44h[31] is a RWO bit
    /// So we program D20:F0:44h[25][24:22] here
    /// I/O DMA Request Boundary Crossing Control set to 1b
    /// IDMA Write Request Size Control set to 111b
    ///
    XhccCfg2 |= (UINT32) (B_PCH_XHCI_XHCC2_DREQBCC);
    XhccCfg2 |= (UINT32) (B_PCH_XHCI_XHCC2_IDMARRSC);
    XhccCfg2 |= (UINT32) (B_PCH_XHCI_XHCC2_OCCFDONE);
    MmioWrite32 (XhciPciMmBase + R_PCH_XHCI_XHCC2, (UINT32) (XhccCfg2));
    PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
      EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
      EfiBootScriptWidthUint32,
      (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCC2),
      1,
      (VOID *) (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCC2)
      );
//
// For SV team usage.
//
#ifndef _SVBIOS_
    XhccCfg |= (UINT32) (B_PCH_XHCI_XHCC1_ACCTRL);
    MmioWrite32 (XhciPciMmBase + R_PCH_XHCI_XHCC1, XhccCfg);
    PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
      EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
      EfiBootScriptWidthUint32,
      (UINTN) (XhciPciMmBase + R_PCH_XHCI_XHCC1),
      1,
      &XhccCfg
      );
#endif
  }
}

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
{
  UINT32      UsbPort;
  UINTN       PortResetTimeout;
  UINTN       HsPortCount;
  UINTN       HsUsbrPortCount;
  UINTN       SsPortCount;
  UINT32      PortMask;
  UINT8       UsbPortRouting;
  UINT32      Data32;

  ///
  /// Check if XHCI disabled, Exit function.
  ///
  if (1) {
//  if (UsbConfig->Usb30Settings.Mode == PCH_XHCI_MODE_OFF) {
    return;
  }
  ///
  /// Retrieves information about number of implemented xHCI ports and sets
  /// appropriate port mask registers
  /// Get the xHCI port number and program xHCI Port Routing Mask register
  ///
  GetXhciPortCountAndSetPortRoutingMask (
    XhciPciMmBase,
    &HsPortCount,
    &HsUsbrPortCount,
    &SsPortCount
    );

  ///
  /// Do nothing for this case
  ///
  UsbPortRouting = USB_PR_CASE_0;

  if (1) {
//  if ( (UsbConfig->Usb30Settings.PreBootSupport == PCH_DEVICE_DISABLE) ||
//      ((UsbConfig->Usb30Settings.PreBootSupport == PCH_DEVICE_ENABLE) &&
//       (UsbConfig->Usb30Settings.Mode == PCH_XHCI_MODE_AUTO))) {
    ///
    /// VLV BIOS Spec Rev 0.5
    /// When the BIOS does not have xHCI pre-boot software available:
    /// Section 32.1.1.2 xHCI Enabled mode
    /// BIOS should route the Ports to the EHCI controller and prior to OS boot
    /// it should route the ports to the xHCI controller.
    /// Section 32.1.1.3 xHCI Auto mode
    /// BIOS should route the Ports to the EHCI controller
    ///
    /// When the BIOS has xHCI pre-boot software available:
    /// Section 32.1.2.3 xHCI Auto mode
    /// BIOS should route the Ports to the EHCI controller
    ///
    /// For above cases, BIOS should follow Section 13.2.5 to route the
    /// USB Ports to EHCI Controller.
    ///
    UsbPortRouting = USB_PR_CASE_1;
  }

  if (0) {
//  if ((UsbConfig->Usb30Settings.PreBootSupport == PCH_DEVICE_ENABLE) &&
//      (UsbConfig->Usb30Settings.Mode == PCH_XHCI_MODE_ON)) {
    ///
    /// VLV BIOS Spec Rev 0.5.
    /// When the BIOS has xHCI pre-boot software available:
    /// Section 32.1.2.2 xHCI Enabled mode
    /// BIOS should route the Ports to the xHCI controller
    ///
    /// For the above case, BIOS should follow Section 13.2.6 to route the
    /// USB Ports to xHCI Controller.
    ///
    UsbPortRouting = USB_PR_CASE_2;
  }

  if ((SsPortCount != 0) &&
      (UsbPortRouting == USB_PR_CASE_1) &&
      ((MmioRead32 (XhciPciMmBase + R_PCH_XHCI_USB2PR) != 0) ||
       (MmioRead32 (XhciPciMmBase + R_PCH_XHCI_USB3PR) != 0))) {
    ///
    /// VLV BIOS Spec Rev 0.5 Section 32.2.5 Routing of switchable USB Ports to
    /// EHCI Controller
    /// Step 1
    /// Retrieve information about the number of implemented xHCI ports and set appropriate
    /// port mask registers
    /// Done in GetXhciPortCountAndSetPortRoutingMask()
    /// Step 2
    /// Based on available number of ports (from step 1) initiate port reset to enabled ports
    /// where USB 2.0 device is connected
    ///
    /// 2.a. For Port #1, if xHCIBAR + 480h [0] is sets then
    /// 2.b. Issue port reset by sets xHCIBAR + 480h [4] to 1b
    /// 2.f. Repeat steps #a to #e for all the USB2.0 ports.
    ///
    for (UsbPort = 0; UsbPort < HsPortCount; UsbPort++) {
      if (MmioRead32 (XhciMmioBase + PORTSCxUSB2[UsbPort]) & B_PCH_XHCI_PORTSCXUSB2_CCS) {
        MmioAndThenOr32 (
          XhciMmioBase + PORTSCxUSB2[UsbPort],
          (UINT32)~(B_PCH_XHCI_PORTSCXUSB2_PED),
          B_PCH_XHCI_PORTSCXUSB2_PR
          );
      }
    }
    ///
    /// 2.c. Poll for port reset bit at steps #b to be cleared or timeout at 100ms
    ///
    PortResetTimeout = 0;
    do {
      Data32 = 0;
      for (UsbPort = 0; UsbPort <= HsPortCount; UsbPort++) {
        Data32 |= MmioRead32 (XhciMmioBase + PORTSCxUSB2[UsbPort]);
      }
      PchPmTimerStall (TEN_MS_TIMEOUT);
      PortResetTimeout++;
    } while ((Data32 & B_PCH_XHCI_PORTSCXUSB2_PR) &&
            (PortResetTimeout < PORT_RESET_TIMEOUT));
    ///
    /// 2.d. Program D20:F0:D0h[5:0] to 0
    ///
    MmioAnd32 (
      XhciPciMmBase + R_PCH_XHCI_USB2PR,
      (UINT32)~B_PCH_XHCI_USB2PR_USB2HCSEL
      );

    ///
    /// 2.e. Clear all the port's status by program xHCIBAR + 480h [23:17] to 1111111b
    ///
    for (UsbPort = 0; UsbPort < HsPortCount; UsbPort++) {
      MmioAndThenOr32 (
        XhciMmioBase + PORTSCxUSB2[UsbPort],
        (UINT32)~ (B_PCH_XHCI_PORTSCXUSB2_PED),
        B_PCH_XHCI_PORTSCXUSB2_CEC |
        B_PCH_XHCI_PORTSCXUSB2_PLC |
        B_PCH_XHCI_PORTSCXUSB2_PRC |
        B_PCH_XHCI_PORTSCXUSB2_OCC |
        B_PCH_XHCI_PORTSCXUSB2_WRC |
        B_PCH_XHCI_PORTSCXUSB2_PEC |
        B_PCH_XHCI_PORTSCXUSB2_CSC
        );
    }

    if (HsUsbrPortCount > 0) {
      MmioAndThenOr32 (
        XhciMmioBase + PORTSCxUSB2[HsPortCount],
        (UINT32)~ (B_PCH_XHCI_PORTSCXUSB2_PED),
        B_PCH_XHCI_PORTSCXUSB2_CEC |
        B_PCH_XHCI_PORTSCXUSB2_PLC |
        B_PCH_XHCI_PORTSCXUSB2_PRC |
        B_PCH_XHCI_PORTSCXUSB2_OCC |
        B_PCH_XHCI_PORTSCXUSB2_WRC |
        B_PCH_XHCI_PORTSCXUSB2_PEC |
        B_PCH_XHCI_PORTSCXUSB2_CSC
        );
    }
    ///
    /// Step 3
    /// Initiate warm reset to all USB 3.0 ports
    ///
    /// 3.a. For Port #1,  sets xHCIBAR + 570h [31]
    /// 3.e. Repeat steps #a to #e for all the USB3.0 ports.
    ///
    for (UsbPort = 0; UsbPort < SsPortCount; UsbPort++) {
      MmioAndThenOr32 (
        XhciMmioBase + PORTSCxUSB3[UsbPort],
        (UINT32)~ (B_PCH_XHCI_PORTSCXUSB3_PED),
        B_PCH_XHCI_PORTSCXUSB3_WPR
        );
    }
    ///
    /// 3.b. Program D20:F0:D8h[5:0] to 0h.
    ///
    MmioAnd32 (
      XhciPciMmBase + R_PCH_XHCI_USB3PR,
      (UINT32)~B_PCH_XHCI_USB3PR_USB3SSEN
      );
    ///
    /// 3.c. Poll for warm reset bit at steps #a to be cleared or timeout at 100ms
    ///
    PortResetTimeout = 0;
    do {
      Data32 = 0;
      for (UsbPort = 0; UsbPort < SsPortCount; UsbPort++) {  //EIP132696
        Data32 |= MmioRead32 (XhciMmioBase + PORTSCxUSB3[UsbPort]);
      }
      PchPmTimerStall (TEN_MS_TIMEOUT);
      PortResetTimeout++;
    } while ((Data32 & B_PCH_XHCI_PORTSCXUSB3_PR) &&
            (PortResetTimeout < PORT_RESET_TIMEOUT));
    ///
    /// 3.d. Clear all the port's status by program xHCIBAR + 570h [23:17] to 1111111b.
    ///
    for (UsbPort = 0; UsbPort < SsPortCount; UsbPort++) {
      MmioAndThenOr32 (
        XhciMmioBase + PORTSCxUSB3[UsbPort],
        (UINT32)~ (B_PCH_XHCI_PORTSCXUSB3_PED),
        B_PCH_XHCI_PORTSCXUSB3_CEC |
        B_PCH_XHCI_PORTSCXUSB3_PLC |
        B_PCH_XHCI_PORTSCXUSB3_PRC |
        B_PCH_XHCI_PORTSCXUSB3_OCC |
        B_PCH_XHCI_PORTSCXUSB3_WRC |
        B_PCH_XHCI_PORTSCXUSB3_PEC |
        B_PCH_XHCI_PORTSCXUSB3_CSC
        );
    }
    ///
    /// Step 4
    /// Set the Run bit of xHCI controller, xHCIBAR +80h[0] to 1b
    ///
    MmioOr32 (
      XhciMmioBase + R_PCH_XHCI_USBCMD,
      B_PCH_XHCI_USBCMD_RS
      );
    ///
    /// Step 5
    /// Then clear the Run bit of xHCI controller, xHCIBAR +80h[0] to 0b
    ///
    MmioAnd32 (
      XhciMmioBase + R_PCH_XHCI_USBCMD,
      (UINT32)~B_PCH_XHCI_USBCMD_RS
      );
  } else if ((SsPortCount != 0) && (UsbPortRouting == USB_PR_CASE_2)) {
    ///
    /// VLV BIOS Spec Rev 0.5 Section 32.2.5 Routing of switchable USB Ports to
    /// xHCI Controller
    /// Step 1
    /// Retrieve information about the number of implemented xHCI ports and set appropriate
    /// port mask registers
    /// Done in GetXhciPortCountAndSetPortRoutingMask()
    /// Step 2
    /// Program D20:F0:D8h[5:0] to the value of xHCI D20:F0:DCh[5:0]
    ///
    PortMask = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_USB3PRM) & (UINT32) B_PCH_XHCI_USB3PR_USB3SSENM;

    MmioAndThenOr32 (
      XhciPciMmBase + R_PCH_XHCI_USB3PR,
      (UINT32)~B_PCH_XHCI_USB3PR_USB3SSEN,
      PortMask
      );
    ///
    /// Step 3
    /// Program D20:F0:D0h[14:0] to the value of xHCI D20:F0:D4h[14:0]
    ///
    PortMask = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_USB2PRM) & (UINT32) B_PCH_XHCI_USB2PR_USB2HCSELM;

    MmioAndThenOr32 (
      XhciPciMmBase + R_PCH_XHCI_USB2PR,
      (UINT32)~B_PCH_XHCI_USB2PR_USB2HCSEL,
      PortMask
      );
  }
}

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
{
  UINT32  HsPortEnableMask;
  UINT32  SsPortEnableMask;

  ///
  /// VLV BIOS Spec Rev 0.5.0
  /// Section 32.2.4 Routing of switchable USB Ports to EHCI Controller
  /// Section 32.2.5 Routing of switchable USB Ports to xHCI Controller
  ///
  /// Step 1.a Check xHCI D20:F0:E0h[2:1] to get HS Port Count.
  ///
  switch (MmioRead32 (XhciPciMmBase + R_PCH_XHCI_FUS) & B_PCH_XHCI_FUS_HSPRTCNT) {
  case V_PCH_XHCI_FUS_HSPRTCNT_11B:
    ///
    /// If the value is 11b: Set xHCI D20:F0:D4h[13:0] to 00FFh. Number of HS ports is 8.
    ///
    *HsPortCount      = V_PCH_XHCI_FUS_HSPRTCNT_11B_CNT;
    HsPortEnableMask  = V_PCH_XHCI_FUS_HSPRTCNT_11B_MASK;
    break;

  case V_PCH_XHCI_FUS_HSPRTCNT_10B:
    ///
    /// If the value is 10b: Set xHCI D20:F0:D4h[13:0] to 03FFh. Number of HS ports is 10.
    ///
    *HsPortCount      = V_PCH_XHCI_FUS_HSPRTCNT_10B_CNT;
    HsPortEnableMask  = V_PCH_XHCI_FUS_HSPRTCNT_10B_MASK;
    break;

  case V_PCH_XHCI_FUS_HSPRTCNT_01B:
    ///
    /// If the value is 01b: Set xHCI D20:F0:D4h[13:0] to 0FFFh. Number of HS ports is 12.
    ///
    *HsPortCount      = V_PCH_XHCI_FUS_HSPRTCNT_01B_CNT;
    HsPortEnableMask  = V_PCH_XHCI_FUS_HSPRTCNT_01B_MASK;
    break;

  case V_PCH_XHCI_FUS_HSPRTCNT_00B:
    ///
    /// If the value is 00b: Set xHCI D20:F0:D4h[13:0] to 3FFFh. Number of HS ports is 14
    ///
  default:
    *HsPortCount      = V_PCH_XHCI_FUS_HSPRTCNT_00B_CNT;
    HsPortEnableMask  = V_PCH_XHCI_FUS_HSPRTCNT_00B_MASK;
    break;
  }
  ///
  /// Override High Speed Port Count for VLV A0
  /// VLV A0 only have 6 HS ports
  ///
  if (*HsPortCount > 6) {
    *HsPortCount = 6;
    HsPortEnableMask = 0x003F;
  }
  ///
  /// Step 1.b Check xHCI D20:F0:E0h[4:3] to get SS Port Count.
  ///
  switch (MmioRead32 (XhciPciMmBase + R_PCH_XHCI_FUS) & B_PCH_XHCI_FUS_SSPRTCNT) {
  case V_PCH_XHCI_FUS_SSPRTCNT_11B:
    ///
    /// If the value is 11b: Set xHCI D20:F0:DCh[5:0] to 000000b. Number of SS ports is 0.
    ///
    *SsPortCount      = V_PCH_XHCI_FUS_SSPRTCNT_11B_CNT;
    SsPortEnableMask  = V_PCH_XHCI_FUS_SSPRTCNT_11B_MASK;
    break;

  case V_PCH_XHCI_FUS_SSPRTCNT_10B:
    ///
    /// If the value is 10b: Set xHCI D20:F0:DCh[5:0] to 000011b. Number of SS ports is 2.
    ///
    *SsPortCount      = V_PCH_XHCI_FUS_SSPRTCNT_10B_CNT;
    SsPortEnableMask  = V_PCH_XHCI_FUS_SSPRTCNT_10B_MASK;
    break;

  case V_PCH_XHCI_FUS_SSPRTCNT_01B:
    ///
    /// If the value is 01b: Set xHCI D20:F0:DCh[5:0] to 001111b. Number of SS ports is 4.
    ///
    *SsPortCount      = V_PCH_XHCI_FUS_SSPRTCNT_01B_CNT;
    SsPortEnableMask  = V_PCH_XHCI_FUS_SSPRTCNT_01B_MASK;
    break;

  case V_PCH_XHCI_FUS_SSPRTCNT_00B:
    ///
    /// If the value is 00b: Set xHCI D20:F0:DCh[5:0] to 111111b. Number of SS ports is 6.
    ///
  default:
    *SsPortCount      = V_PCH_XHCI_FUS_SSPRTCNT_00B_CNT;
    SsPortEnableMask  = V_PCH_XHCI_FUS_SSPRTCNT_00B_MASK;
    break;
  }
  ///
  /// Override Super Speed Port Count for VLV A0
  /// VLV A0 only have 1 SS port
  ///
  if (*SsPortCount > 1) {
    *SsPortCount = 1;
    SsPortEnableMask = 0x0001;
  }
  ///
  /// Step 1.c Check xHCI D20:F0:E0h[5] to know if USBr is enabled.
  ///
  switch (MmioRead32 (XhciPciMmBase + R_PCH_XHCI_FUS) & B_PCH_XHCI_FUS_USBR) {
  case V_PCH_XHCI_FUS_USBR_EN:
    ///
    /// If 0b: Set xHCI D20:F0:D4[6] to 1b. USBr port is enabled.
    ///
    *HsUsbrPortCount = 1;
    HsPortEnableMask |= BIT6;
    break;

  case V_PCH_XHCI_FUS_USBR_DIS:
    ///
    /// If 1b: Set xHCI D20:F0:D4[6] to 0b. USBr port is disabled.
    ///
    *HsUsbrPortCount = 0;
    HsPortEnableMask &= (~BIT6);
    break;
  }
  ///
  /// Set xHCI USB2 Port Routing Mask register (D20:F0:D4h[8:0])
  /// per HS Port Enable Mask value
  ///
  MmioAndThenOr32 (
    XhciPciMmBase + R_PCH_XHCI_USB2PRM,
    ~ (UINT32) B_PCH_XHCI_USB2PR_USB2HCSELM,
    HsPortEnableMask
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciPciMmBase + R_PCH_XHCI_USB2PRM),
    1,
    (VOID *) (UINTN) (XhciPciMmBase + R_PCH_XHCI_USB2PRM)
    );
  ///
  /// Set xHCI USB3 Port Routing Mask register (D20:F0:DCh[5:0])
  /// per SS Port Enable Mask value
  ///
  MmioAndThenOr32 (
    XhciPciMmBase + R_PCH_XHCI_USB3PRM,
    ~ (UINT32) B_PCH_XHCI_USB3PR_USB3SSENM,
    SsPortEnableMask
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciPciMmBase + R_PCH_XHCI_USB3PRM),
    1,
    (VOID *) (UINTN) (XhciPciMmBase + R_PCH_XHCI_USB3PRM)
    );
}

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
{
  ///
  ///  BIOS responsibility on Overcurrent protection.
  ///  ----------------------------------------------
  ///  There are 8 total overcurrent pins
  ///  which can be map to 14 USB2 ports and 6 USB3 ports.
  ///  On a given physical connector,
  ///  one OC pin is shared between the USB2 (HS) pins and USB3 (SS) pins.
  ///  USB2 (HS) pins are programmable to be owned by either XHCI or EHCI.
  ///  OC pins are associated to the current owner.
  ///  USB2 (HS) ports 1-8 use OC pins 1-4, ports 9-14 use OC pins 4-8
  ///  USB3 (SS) ports has the flexibility in pairing with any of the OC pins.
  ///  It is ok to map multiple ports to a single pin.
  ///  It is not ok to map a single ports to a multiple pins.
  ///  All USB ports routed out of the package must have Overcurrent protection.
  ///  USB Ports not routed out from the package should not be assigned OC pins.
  ///
  UINT32  Index;
  UINT32  CurrentIndex;
  UINT32  XhciHsOcm1;
  UINT32  XhciSsOcm1;
  UINT32  XhciSsOcm2;
  UINT8   UsbOverCurrentMapping[PCH_USB_MAX_PHYSICAL_PORTS] = {USB_OVER_CURRENT_MAPPING_SETTINGS};
  UINT8   Usb30OverCurrentMapping[PCH_XHCI_MAX_USB3_PORTS] = {USB30_OVER_CURRENT_MAPPING_SETTINGS};
  
  ///
  /// Check if XHCI disabled, Exit function.
  ///
  if (1) {
//  if (UsbConfig->Usb30Settings.Mode == PCH_XHCI_MODE_OFF) {
    return;
  }
  ///
  /// Find the corresponding register and set the bits
  ///
  XhciHsOcm1  = 0;
  XhciSsOcm1  = 0;
  XhciSsOcm2  = 0;

  for (Index = 0; Index < PCH_USB_MAX_PHYSICAL_PORTS; Index++) {
    if (UsbOverCurrentMapping[Index] == PchUsbOverCurrentPinSkip) {
      ///
      /// No OC pin assigned, skip this port
      ///
    } else {
      if (Index < 8) {
        ///
        /// Port 0-7: OC0 - OC3
        ///
        if (UsbOverCurrentMapping[Index] > PchUsbOverCurrentPin3) {
          ASSERT (FALSE);
          continue;
        }

        CurrentIndex = UsbOverCurrentMapping[Index] * 8 + Index;
        XhciHsOcm1 |= (UINT32) (BIT0 << CurrentIndex);
      }
    }
  }

  for (Index = 0; Index < PCH_XHCI_MAX_USB3_PORTS; Index++) {
    if (Usb30OverCurrentMapping[Index] == PchUsbOverCurrentPinSkip) {
      ///
      /// No OC pin assigned, skip this port
      ///
    } else {
      ///
      /// Port 0-5: OC0 - OC7
      ///
      if (Usb30OverCurrentMapping[Index] < PchUsbOverCurrentPin4) {
        CurrentIndex = Usb30OverCurrentMapping[Index] * 8 + Index;
        XhciSsOcm1 |= (UINT32) (BIT0 << CurrentIndex);
      } else {
        CurrentIndex = (Usb30OverCurrentMapping[Index] - 4) * 8 + Index;
        XhciSsOcm2 |= (UINT32) (BIT0 << CurrentIndex);
      }
    }
  }
  ///
  /// OCM registers are in the suspend well.
  ///
  MmioWrite32 (XhciPciMmBase + R_PCH_XHCI_U2OCM1, XhciHsOcm1);
  MmioWrite32 (XhciPciMmBase + R_PCH_XHCI_U3OCM1, XhciSsOcm1);
  MmioWrite32 (XhciPciMmBase + R_PCH_XHCI_U3OCM2, XhciSsOcm2);
}

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
{
  UINT32          Index;
  UINT32          CurrentIndex;
  UINT32          Ehci1Ocm;
  UINT8           UsbOverCurrentMapping[PCH_USB_MAX_PHYSICAL_PORTS] = {USB_OVER_CURRENT_MAPPING_SETTINGS};

  Ehci1Ocm  = 0;

  for (Index = 0; Index < PCH_USB_MAX_PHYSICAL_PORTS; Index++) {
    if (UsbOverCurrentMapping[Index] == PchUsbOverCurrentPinSkip) {
      ///
      /// No OC pin assigned, skip this port
      ///
    } else {
        ///
        /// Port 0~7  -> OC0~3
        ///
        if (UsbOverCurrentMapping[Index] > PchUsbOverCurrentPin3) {
          ASSERT (FALSE);
          continue;
        }

        CurrentIndex = UsbOverCurrentMapping[Index] * 8 + Index;
        Ehci1Ocm |= (UINT32) (BIT0 << CurrentIndex);
    }
  }
  ///
  /// EHCI1OCM and EHCI2OCM are in the suspend well.
  ///
  if (1) {
//  if (UsbConfig->Usb20Settings[PchEhci1].Enable == PCH_DEVICE_ENABLE) {
    MmioWrite32 (Ehci1PciMmBase + R_PCH_EHCI_OCMAP, Ehci1Ocm);
  }
}

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

  @retval None

**/
{
  UINT32          Index;

  for (Index = 0; Index < PCH_USB_MAX_PHYSICAL_PORTS; Index++) {
    if ((Index < 8)) {
//    if ((Index < 8) && (UsbConfig->Usb20Settings[PchEhci1].Enable == PCH_DEVICE_ENABLE)) {
      ///
      /// EHCI1 PDO for Port 0 to 7
      ///
      if (0) {
//      if (UsbConfig->PortSettings[Index].Enable == PCH_DEVICE_DISABLE) {
        MmioOr8 (Ehci1PciMmBase + R_PCH_EHCI_PDO, (UINT8) (B_PCH_EHCI_PDO_DIS_PORT0 << Index));
      } else {
        MmioAnd8 (Ehci1PciMmBase + R_PCH_EHCI_PDO, (UINT8) ~(B_PCH_EHCI_PDO_DIS_PORT0 << Index));
      }
    }
  }
}

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
{
  UINT32          Index;
  UINT32          XhciUsb2Pdo;
  UINT32          XhciUsb3Pdo;

  ///
  /// Check if XHCI disabled, Exit function.
  ///
  if (1) {
//  if (UsbConfig->Usb30Settings.Mode == PCH_XHCI_MODE_OFF) {
    return;
  }
  ///
  /// XHCI PDO for HS
  ///
  XhciUsb2Pdo = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_USB2PDO) & B_PCH_XHCI_USB2PDO_MASK;
  for (Index = 0; Index < PCH_USB_MAX_PHYSICAL_PORTS; Index++) {
    if (0) {
//    if (UsbConfig->PortSettings[Index].Enable == PCH_DEVICE_DISABLE) {
      XhciUsb2Pdo |= (UINT32) (B_PCH_XHCI_USB2PDO_DIS_PORT0 << Index);
    } else {
      XhciUsb2Pdo &= (UINT32)~(B_PCH_XHCI_USB2PDO_DIS_PORT0 << Index);
    }
  }
  ///
  /// XHCI PDO for SS
  ///
  XhciUsb3Pdo = MmioRead32 (XhciPciMmBase + R_PCH_XHCI_USB3PDO) & B_PCH_XHCI_USB3PDO_MASK;
  for (Index = 0; Index < PCH_XHCI_MAX_USB3_PORTS; Index++) {
    if (0) {
//    if (UsbConfig->PortSettings[Index].Enable == PCH_DEVICE_DISABLE) {
      XhciUsb3Pdo |= (UINT32) (B_PCH_XHCI_USB3PDO_DIS_PORT0 << Index);
    } else {
      XhciUsb3Pdo &= (UINT32)~(B_PCH_XHCI_USB3PDO_DIS_PORT0 << Index);
    }
  }
  ///
  /// USB2PDO and USB3PDO are Write-Once registers and bits in them are in the SUS Well.
  ///
  MmioWrite32 (XhciPciMmBase + R_PCH_XHCI_USB2PDO, XhciUsb2Pdo);
  MmioWrite32 (XhciPciMmBase + R_PCH_XHCI_USB3PDO, XhciUsb3Pdo);
}

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
{
  ///
  /// NOTE: EHCI USBR Enable
  /// EHCI1_USBr_en and EHCI2_USBr_en are mutually exclusive. Both cannot be set to 1 at any one time.
  /// SW must ensure at any one time, only 1 EHCI should have the bit set.
  ///
  MmioOr16 (EhciPciMmBase + 0x7A, (UINT16) BIT8);
}

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
{
  ///
  /// Check if XHCI disabled, Exit function.
  ///
  if (1) {
//  if (UsbConfig->Usb30Settings.Mode == PCH_XHCI_MODE_OFF) {
    return;
  }
  ///
  /// Assign memory resources
  ///
  MmioWrite32 (XhciPciMmBase + R_PCH_XHCI_MEM_BASE, XhciMmioBase);
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciPciMmBase + R_PCH_XHCI_MEM_BASE),
    1,
    (VOID *) (UINTN) (XhciPciMmBase + R_PCH_XHCI_MEM_BASE)
    );

  MmioOr16 (
    XhciPciMmBase + R_PCH_XHCI_COMMAND_REGISTER,
    (UINT16) (B_PCH_XHCI_COMMAND_MSE | B_PCH_XHCI_COMMAND_BME)
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint16,
    (UINTN) (XhciPciMmBase + R_PCH_XHCI_COMMAND_REGISTER),
    1,
    (VOID *) (UINTN) (XhciPciMmBase + R_PCH_XHCI_COMMAND_REGISTER)
    );
}

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
{
  ///
  /// Check if XHCI disabled, Exit function.
  ///
  if (1) {
//  if (UsbConfig->Usb30Settings.Mode == PCH_XHCI_MODE_OFF) {
    return;
  }
  ///
  /// Clear memory resources
  ///
  MmioAnd16 (
    XhciPciMmBase + R_PCH_XHCI_COMMAND_REGISTER,
    (UINT16)~(B_PCH_XHCI_COMMAND_MSE | B_PCH_XHCI_COMMAND_BME)
    );
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint16,
    (UINTN) (XhciPciMmBase + R_PCH_XHCI_COMMAND_REGISTER),
    1,
    (VOID *) (UINTN) (XhciPciMmBase + R_PCH_XHCI_COMMAND_REGISTER)
    );

  MmioWrite32 ((XhciPciMmBase + R_PCH_XHCI_MEM_BASE), 0);
  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) (XhciPciMmBase + R_PCH_XHCI_MEM_BASE),
    1,
    (VOID *) (UINTN) (XhciPciMmBase + R_PCH_XHCI_MEM_BASE)
    );
}

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
{
  ///
  /// VLV BIOS Spec Rev 0.5.0 Section 12.10
  /// Additional Programming Requirements during USB Initialization
  /// Step 7
  /// System BIOS is required to program these registers during USB
  /// port initialization. Please refer to PCH EDS or EDS Spec Update
  /// for detailed register settings for different platform.
  ///
}

VOID
ConfigureUsbClockGating (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN     UINT32                               RootComplexBar
)
{
  ConfigureEhciClockGating (PchPlatformPolicy, RootComplexBar);
  ConfigureXhciClockGating (PchPlatformPolicy);
}

VOID
ConfigureEhciClockGating (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN     UINT32                               RootComplexBar
)
{
  UINTN           EhciPciMmBase;
  UINT8           Index;

  ///
  /// PCH BIOS Spec Rev 0.5.5, Section 19.10 Enabling Clock Gating
  /// 4. USB 1.1 / USB 2.0 / USB 3.0
  /// a. Enable dynamic clock gating by setting
  ///
  for (Index = 0; Index < PchEhciControllerMax; Index++) {
    EhciPciMmBase = MmPciAddress (
                      0,
                      PchPlatformPolicy->BusNumber,
                      EhciControllersMap[Index].Device,
                      EhciControllersMap[Index].Function,
                      0
                      );
    ///
    /// Step 3
    /// Enable SB local clock gating.
    /// Program EHCI D29:F0 + 0x7C [14] = 1b
    ///
    MmioOr32 (
      (UINTN) (EhciPciMmBase + R_PCH_EHCI_EHCSUSCFG),
      (UINT32) (B_PCH_EHCI_EHCSUSCFG_SLCGE)
      );
    PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
      EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
      EfiBootScriptWidthUint32,
      (UINTN) (EhciPciMmBase + R_PCH_EHCI_EHCSUSCFG),
      1,
      (VOID *) (UINTN) (EhciPciMmBase + R_PCH_EHCI_EHCSUSCFG)
      );
    ///
    /// Program RCBA + 284h [31:0] = BEh
    ///
    
	///
    /// Disable the IOSF primary clock trunk gating functionality
    /// PortId = 0x49, Offset 0x84 = 0x9E
    ///
    MmioWrite32 (
      (UINTN) (RootComplexBar + 0x284),
      (UINT32) (0x9E)
      );
    PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
      EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
      EfiBootScriptWidthUint32,
      (UINTN) (RootComplexBar + 0x284),
      1,
      (VOID *) (UINTN) (RootComplexBar + 0x284)
      );
  }
}


VOID
ConfigureEhciDebug (
  IN  PCH_USB_CONFIG              *UsbConfig,
  IN  UINT8                       BusNumber,
  IN  BOOLEAN                     IsEnable
)
{
  UINTN    EhciDbgPortCtrl;
  UINT32   value;

//  ASSERT ( UsbConfig!=NULL );
  
	IoWrite8 (0x72,0x06);
	IoWrite8 (0x73,0x01);
  
  EhciDbgPortCtrl = MmPciAddress (
                        0,
                        BusNumber,
                        EhciControllersMap[0].Device,
                        EhciControllersMap[0].Function,
                        R_PCH_EHCI_DP_CTRLSTS
                        );
  
	IoWrite8 (0x72,0x06);
	IoWrite8 (0x73,0x02);
  if (TRUE == IsEnable) {
    value = MmioRead32 (EhciDbgPortCtrl);
    value |= (B_PCH_EHCI_DP_OWNER_CNT | B_PCH_EHCI_DP_ENABLED_CNT);
    // Program value 0x50000000 to offset 0xA0 of BAR from Bus0 device29 func0.
    // ENABLED_CNT = 1 OWNER_CNT =1
    MmioWrite32 (EhciDbgPortCtrl, value);
  } else {
    value = MmioRead32 (EhciDbgPortCtrl);
    value &= ~(B_PCH_EHCI_DP_OWNER_CNT | B_PCH_EHCI_DP_ENABLED_CNT);
    MmioWrite32 (EhciDbgPortCtrl, value);
  }
	IoWrite8 (0x72,0x06);
	IoWrite8 (0x73,0x03);
	//(EIP112014-)  DEBUG ((EFI_D_INFO, "ConfigureEhciDebug() - B:D:F (%d %d %d) 0x%x 0x%x\n", BusNumber, EhciControllersMap[0].Device, EhciControllersMap[0].Function, IsEnable, value));

  PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
    EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
    EfiBootScriptWidthUint32,
    (UINTN) EhciDbgPortCtrl,
    1,
    (VOID *) (UINTN) EhciDbgPortCtrl
  );
	IoWrite8 (0x72,0x06);
	IoWrite8 (0x73,0x04);
  
  return;

}

