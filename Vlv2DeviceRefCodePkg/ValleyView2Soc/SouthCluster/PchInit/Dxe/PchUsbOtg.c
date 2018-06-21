/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PchUsbOtg.c

Abstract:

  Initializes PCH USB On-The-Go Device

--*/
#include "PchInit.h"


EFI_STATUS
ConfigureOtg (
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy,
  IN OUT UINT32                            *FuncDisableReg
  )
/*++

Routine Description:

  Configure OTG device.

Arguments:

  PchPlatformPolicy       The PCH Platform Policy protocol instance
  FuncDisableReg          Function Disable Register

Returns:

  EFI_SUCCESS             The function completed successfully

--*/
{
  EFI_STATUS            Status;
  UINTN                 OtgPciMmBase;
  EFI_PHYSICAL_ADDRESS  OtgMmioBase0;
  UINT32                Buffer32;
  UINT32                PmcBase;
  UINTN                 PciD31F0RegBase;
  UINT16                val=0;

  PciD31F0RegBase = MmPciAddress (0,
                        0,
                        PCI_DEVICE_NUMBER_PCH_LPC,
                        PCI_FUNCTION_NUMBER_PCH_LPC,
                        0
                      );

  PmcBase         = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;

  DEBUG ((EFI_D_INFO, "ConfigureOtg() Start\n"));

  OtgPciMmBase = MmPciAddress (0,
                   DEFAULT_PCI_BUS_NUMBER_PCH,
                   PCI_DEVICE_NUMBER_PCH_OTG,
                   PCI_FUNCTION_NUMBER_PCH_OTG,
                   0
                 );
  OtgMmioBase0 = 0;
  Buffer32     = 0;

  if (PchPlatformPolicy->UsbConfig->UsbOtgSettings.Enable == PCH_DEVICE_DISABLE) {
    ///
    /// Put device in D3hot state.
    ///
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting USB OTG into D3 Hot State.\n"));
    MmioOr32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_PMECTLSTS), B_PCH_OTG_PMECTLSTS_POWERSTATE);
    PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
      EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
      EfiBootScriptWidthUint32,
      (UINTN) (OtgPciMmBase + R_PCH_OTG_PMECTLSTS),
      1,
      (VOID *) (UINTN) (OtgPciMmBase + R_PCH_OTG_PMECTLSTS)
      );
    *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_OTG;
    MmioOr32 ((PmcBase + R_PCH_PMC_FUNC_DIS2),B_PCH_PMC_FUNC_DIS2_OTG_SS_PHY );
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS2),
      1,
      (VOID *) (UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS2)
    );
  } else {
    ///
    /// Check if device present
    ///
    if (MmioRead32 (OtgPciMmBase) != 0xFFFFFFFF) {
      Status = gDS->AllocateMemorySpace (
                      EfiGcdAllocateAnySearchBottomUp,
                      EfiGcdMemoryTypeMemoryMappedIo,
                      N_PCH_OTG_BAR0_ALIGNMENT,
                      V_PCH_OTG_BAR0_SIZE,
                      &OtgMmioBase0,
                      mImageHandle,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      if (!EFI_ERROR (Status)) {

        ///
        /// GPIOS_43/USB_ULPI_REFCLK is toggling by default, BIOS should disable it.BYT Platform HSD #4753278
        /// Until boot to OS and load USB driver, this clock will be enabled by driver.
        /// Must use USB device driver 1.0.0.143 and above.
        ///
        MmioOr32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_GEN_REGRW1), B_PCH_OTG_ULPIPHY_REFCLK_DISABLE);
        PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
          EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
          EfiBootScriptWidthUint32,
          (UINTN) (OtgPciMmBase + R_PCH_OTG_GEN_REGRW1),
          1,
          (VOID *) (UINTN) (OtgPciMmBase + R_PCH_OTG_GEN_REGRW1)
          );

        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD), (UINT32) ~(B_PCH_OTG_STSCMD_BME | B_PCH_OTG_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD),
          1,
          (VOID *) (UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD)
          );
        ///
        /// Program BAR 0
        ///
        ASSERT (((OtgMmioBase0 & B_PCH_OTG_BAR0_BA) == OtgMmioBase0) && (OtgMmioBase0 != 0));
        MmioWrite32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_BAR0), (UINT32) (OtgMmioBase0 & B_PCH_OTG_BAR0_BA));
        PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
          EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
          EfiBootScriptWidthUint32,
          (UINTN) (OtgPciMmBase + R_PCH_OTG_BAR0),
          1,
          (VOID *) (UINTN) (OtgPciMmBase + R_PCH_OTG_BAR0)
          );
        ///
        /// Bus Master Enable & Memory Space Enable
        ///
        MmioOr32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD), (UINT32) (B_PCH_OTG_STSCMD_BME | B_PCH_OTG_STSCMD_MSE));
        PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
          EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
          EfiBootScriptWidthUint32,
          (UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD),
          1,
          (VOID *) (UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD)
          );
        ASSERT (MmioRead32 ((UINTN) OtgMmioBase0) != 0xFFFFFFFF);
        DEBUG ((EFI_D_INFO, "OTG OtgMmioBase:0x%x\n", OtgMmioBase0));
        ///
        /// VCO_calibration_start_point_register / VCO calibration code starting point register value = decimal 10
        ///
        MmioAndThenOr16 (
          (UINTN) (OtgMmioBase0 + R_PCH_OTG_VCO_START_CALIBRATION_START_POINT),
          (UINT16) ~(B_PCH_OTG_VCO_START_CALIBRATION_START_POINT_VALUE),
          (UINT16) (0xA)
          );
        val=MmioRead16 ((UINTN) ((OtgMmioBase0 & B_PCH_OTG_BAR0_BA) + R_PCH_OTG_VCO_START_CALIBRATION_START_POINT));
        DEBUG ((EFI_D_INFO, "OTG VCO_calibration_start_point_register:0x%x\n",
                val
               ));

        PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
          EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
          EfiBootScriptWidthUint16,
          (UINTN) (OtgMmioBase0 + R_PCH_OTG_VCO_START_CALIBRATION_START_POINT),
          1,
          (VOID *) (UINTN) (OtgMmioBase0 + R_PCH_OTG_VCO_START_CALIBRATION_START_POINT)
          );

        ///
        /// PLL_CONTROL / PLL_VCO_calibration_trim_code=7
        ///
        MmioAndThenOr16 (
          (UINTN) (OtgMmioBase0 + R_PCH_OTG_PLL_CONTROL),
          (UINT16) ~(R_PCH_OTG_PLL_VCO_CALIBRATION_TRIM_CODE),
          (UINT16) (0x7<<4)
          );
        val=MmioRead16 ((UINTN) ((OtgMmioBase0 & B_PCH_OTG_BAR0_BA) + R_PCH_OTG_PLL_CONTROL));
        DEBUG ((EFI_D_INFO, "PLL_CONTROL:0x%x\n",
                val
               ));

        PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
          EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
          EfiBootScriptWidthUint16,
          (UINTN) (OtgMmioBase0 + R_PCH_OTG_PLL_CONTROL),
          1,
          (VOID *) (UINTN) (OtgMmioBase0 + R_PCH_OTG_PLL_CONTROL)
          );

        ///
        /// Set vlv.usb.xdci_otg.usb3_cadence.otg3_phy.u1_power_state_definition.tx_en = 1
        ///
        MmioOr16 ((UINTN) (OtgMmioBase0 + R_PCH_OTG_U1_POWER_STATE_DEFINITION), B_PCH_OTG_U1_POWER_STATE_DEFINITION_TX_EN);

        PCH_INIT_COMMON_SCRIPT_MEM_WRITE (
          EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
          EfiBootScriptWidthUint16,
          (UINTN) (OtgMmioBase0 + R_PCH_OTG_U1_POWER_STATE_DEFINITION),
          1,
          (VOID *) (UINTN) (OtgMmioBase0 + R_PCH_OTG_U1_POWER_STATE_DEFINITION)
          );

        ///

        ///
        /// Disable Bus Master Enable & Memory Space Enable
        ///
        MmioAnd32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD), (UINT32) ~(B_PCH_OTG_STSCMD_BME | B_PCH_OTG_STSCMD_MSE));
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD),
          1,
          (VOID *) (UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD)
          );
        ///
        /// Clear BAR0
        ///
        MmioWrite32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_BAR0), (UINT32) (0x00));
        gDS->FreeMemorySpace (OtgMmioBase0, (UINT64) V_PCH_OTG_BAR0_SIZE);
      }
    } else {
      DEBUG ((EFI_D_ERROR, "USB3 OTG not present, skipping.\n"));
      PchPlatformPolicy->UsbConfig->UsbOtgSettings.Enable = PCH_DEVICE_DISABLE;
      *FuncDisableReg |= B_PCH_PMC_FUNC_DIS_OTG;
    }
  }

  DEBUG ((EFI_D_INFO, "ConfigureOtg() End\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
ConfigureOtgAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  )
/**

  @brief
  Hide PCI config space of OTG device and do any final initialization.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINTN       OtgPciMmBase;
  UINT32      OtgMmioBase0;
  UINT32      OtgMmioBase1;
  UINT32      Buffer32;
  EFI_STATUS  AcpiTablePresent;

  DEBUG ((EFI_D_INFO, "ConfigureOtgAtBoot() Start\n"));

  ///
  /// Initialize Variables
  ///
  OtgPciMmBase     = 0;
  OtgMmioBase0     = 0;
  OtgMmioBase1     = 0;
  Buffer32         = 0;
  AcpiTablePresent = EFI_NOT_FOUND;
  ///
  /// Locate ACPI table
  ///
  AcpiTablePresent = InitializePchAslUpdateLib ();
  ///
  /// Update OTG device ACPI variables
  ///
  if (!EFI_ERROR (AcpiTablePresent)) {
    if (PchPlatformPolicy->UsbConfig->UsbOtgSettings.Enable == 2) {
      DEBUG ((EFI_D_INFO, "Switching USB3 OTG into ACPI Mode.\n"));
      OtgPciMmBase = MmPciAddress (0,
                       DEFAULT_PCI_BUS_NUMBER_PCH,
                       PCI_DEVICE_NUMBER_PCH_OTG,
                       PCI_FUNCTION_NUMBER_PCH_OTG,
                       0
                     );
      OtgMmioBase0 = MmioRead32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_BAR0)) & B_PCH_OTG_BAR0_BA;
      OtgMmioBase1 = MmioRead32 ((UINTN) (OtgPciMmBase + R_PCH_OTG_BAR1)) & B_PCH_OTG_BAR1_BA;
      ///
      /// Disable PCI Interrupt, Bus Master Enable & Memory Space Enable
      ///
      MmioOr32 (
        (UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD),
        (UINT32) (B_PCH_OTG_STSCMD_INTR_DIS | B_PCH_OTG_STSCMD_BME | B_PCH_OTG_STSCMD_MSE)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD),
        1,
        (VOID *) (UINTN) (OtgPciMmBase + R_PCH_OTG_STSCMD)
        );
      ///
      /// Update BAR0 in ASL
      ///
      UpdateResourceTemplateAslCode (
        (SIGNATURE_32 ('U', 'O', 'T', 'G')),
        (SIGNATURE_32 ('R', 'B', 'U', 'F')),
        AML_MEMORY32_FIXED_OP,
        1,
        0x04,
        &OtgMmioBase0,
        sizeof (OtgMmioBase0)
        );
      ///
      /// Update BAR1 in ASL
      ///
      UpdateResourceTemplateAslCode (
        (SIGNATURE_32 ('U', 'O', 'T', 'G')),
        (SIGNATURE_32 ('R', 'B', 'U', 'F')),
        AML_MEMORY32_FIXED_OP,
        2,
        0x04,
        &OtgMmioBase1,
        sizeof (OtgMmioBase1)
        );
      ///
      /// Switch to ACPI Mode
      ///
      PchMsgBusAndThenOr32 (
        PCH_OTG_PORT_ID,
        R_PCH_OTG_PCICFGCTR1,
        Buffer32,
        0xFFFFFFFF,
        (B_PCH_OTG_PCICFGCTR1_ACPI_INT_EN1 | B_PCH_OTG_PCICFGCTR1_PCI_CFG_DIS1),
        PCH_OTG_PRIVATE_READ_OPCODE,
        PCH_OTG_PRIVATE_WRITE_OPCODE
        );
    }
  }

  DEBUG ((EFI_D_INFO, "ConfigureOtgAtBoot() End\n"));

  return EFI_SUCCESS;
}
