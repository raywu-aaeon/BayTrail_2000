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
  PchPlatformLib.c

  @brief
  PCH Platform Lib implementation.

**/
#include "PchPlatformLibrary.h"

VOID
EFIAPI
PchPmTimerStall (
  IN  UINTN   Microseconds
  )
/**

  @brief
  Delay for at least the request number of microseconds.
  This function would be called by runtime driver, please do not use any MMIO marco here.

  @param[in] Microseconds         Number of microseconds to delay.

  @retval NONE

**/
{
  UINTN   Ticks;
  UINTN   Counts;
  UINTN   CurrentTick;
  UINTN   OriginalTick;
  UINTN   RemainingTick;
  UINT16  AcpiBase;


#if (_SLE_COMP_ || VP_FLAG)
  Microseconds = 0;
#endif  //_SLE_

  if (Microseconds == 0) {
    return;
  }

  ///
  /// Please use PciRead here, it will link to MmioRead
  /// if the caller is a Runtime driver, please use PchDxeRuntimePciLibPciExpress library, refer
  /// PciExpressRead() on Library\DxeRuntimePciLibPciExpress\DxeRuntimePciLibPciExpress.c for the details.
  /// For the rest please use EdkIIGlueBasePciLibPciExpress library
  ///
  AcpiBase = PciRead16 (
               PCI_LIB_ADDRESS (
               DEFAULT_PCI_BUS_NUMBER_PCH,
               PCI_DEVICE_NUMBER_PCH_LPC,
               PCI_FUNCTION_NUMBER_PCH_LPC,
               R_PCH_LPC_ACPI_BASE)
               ) & B_PCH_LPC_ACPI_BASE_BAR;

  OriginalTick  = IoRead32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_TMR)) & B_PCH_ACPI_PM1_TMR_VAL;
  CurrentTick   = OriginalTick;

  ///
  /// The timer frequency is 3.579545 MHz, so 1 ms corresponds 3.58 clocks
  ///
  Ticks = Microseconds * 358 / 100 + OriginalTick + 1;

  ///
  /// The loops needed by timer overflow
  ///
  Counts = Ticks / V_PCH_ACPI_PM1_TMR_MAX_VAL;

  ///
  /// Remaining clocks within one loop
  ///
  RemainingTick = Ticks % V_PCH_ACPI_PM1_TMR_MAX_VAL;

  ///
  /// not intend to use TMROF_STS bit of register PM1_STS, because this adds extra
  /// one I/O operation, and maybe generate SMI
  ///
  while ((Counts != 0) || (RemainingTick > CurrentTick)) {
    CurrentTick = IoRead32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_TMR)) & B_PCH_ACPI_PM1_TMR_VAL;
    ///
    /// Check if timer overflow
    ///
    if ((CurrentTick < OriginalTick)) {
      if (Counts != 0) {
        Counts--;
      } else {
        ///
        /// If timer overflow and Counts equ to 0, that means we already stalled more than
        /// RemainingTick, break the loop here
        ///
        break;
      }
    }

    OriginalTick = CurrentTick;
  }
}

BOOLEAN
EFIAPI
PchIsSpiDescriptorMode (
  IN  UINTN   SpiBase
  )
/**

  @brief
  Check whether SPI is in descriptor mode

  @param[in] SpiBase              The PCH SPI Base Address

  @retval TRUE                    SPI is in descriptor mode
  @retval FALSE                   SPI is not in descriptor mode

**/
{
  if ((MmioRead16 (SpiBase + R_PCH_SPI_HSFS) & B_PCH_SPI_HSFS_FDV) == B_PCH_SPI_HSFS_FDV) {
    MmioAndThenOr32 (
      SpiBase + R_PCH_SPI_FDOC,
      (UINT32) (~(B_PCH_SPI_FDOC_FDSS_MASK | B_PCH_SPI_FDOC_FDSI_MASK)),
      (UINT32) (V_PCH_SPI_FDOC_FDSS_FSDM | R_PCH_SPI_FDBAR_FLVALSIG)
      );
    if ((MmioRead32 (SpiBase + R_PCH_SPI_FDOD)) == V_PCH_SPI_FDBAR_FLVALSIG) {
      return TRUE;
    } else {
      return FALSE;
    }
  } else {
    return FALSE;
  }
}

// Silicon Steppings

PCH_STEPPING
EFIAPI
PchStepping (
  VOID
  )
/**

  @brief
  Return Pch stepping type

  @param[in] None

  @retval PCH_STEPPING            Pch stepping type

**/
{
  UINT8 RevId;

  RevId = MmioRead8 (
          MmPciAddress (0,
            DEFAULT_PCI_BUS_NUMBER_PCH,
            PCI_DEVICE_NUMBER_PCH_LPC,
            PCI_FUNCTION_NUMBER_PCH_LPC,
            R_PCH_LPC_RID_CC)
          );

  switch (RevId) {
    case V_PCH_LPC_RID_0:
    case V_PCH_LPC_RID_1:
      return PchA0;
      break;

    case V_PCH_LPC_RID_2:
    case V_PCH_LPC_RID_3:
      return PchA1;
      break;

    case V_PCH_LPC_RID_4:
    case V_PCH_LPC_RID_5:
      return PchB0;
      break;

    case V_PCH_LPC_RID_6:
    case V_PCH_LPC_RID_7:
      return PchB1;
      break;

    case V_PCH_LPC_RID_8:
    case V_PCH_LPC_RID_9:
      return PchB2;
      break;

    case V_PCH_LPC_RID_A:
    case V_PCH_LPC_RID_B:
      return PchB3;
      break;

    case V_PCH_LPC_RID_C:
    case V_PCH_LPC_RID_D:
      return PchC0;
      break;

    default:
      return PchSteppingMax;
      break;

  }
}

BOOLEAN
IsPchSupported (
  VOID
  )
/**

  @brief
  Determine if PCH is supported

  @param[in] None

  @retval TRUE                    PCH is supported
  @retval FALSE                   PCH is not supported

**/
{
#if !(_SIMIC_ || _SLE_HYB_ || _SLE_COMP_ || VP_FLAG)
  UINT32  Identifiers;
  UINT16  PcuVendorId;
  UINT16  PcuDeviceId;

  Identifiers = MmioRead32 (
                  MmPciAddress (0,
                  DEFAULT_PCI_BUS_NUMBER_PCH,
                  PCI_DEVICE_NUMBER_PCH_LPC,
                  PCI_FUNCTION_NUMBER_PCH_LPC,
                  R_PCH_LPC_REG_ID)
                );

  PcuDeviceId = (UINT16) ((Identifiers & B_PCH_LPC_DEVICE_ID) >> 16);
  PcuVendorId = (UINT16) (Identifiers & B_PCH_LPC_VENDOR_ID);

  ///
  /// Verify that this is a supported chipset
  ///
  if (PcuVendorId != (UINT16) V_PCH_LPC_VENDOR_ID || !IS_PCH_VLV_LPC_DEVICE_ID (PcuDeviceId)) {
    DEBUG ((EFI_D_ERROR, "VLV SC code doesn't support the PcuDeviceId: 0x%04x!\n", PcuDeviceId));
    return FALSE;
  }
#endif
  return TRUE;
}

VOID
EFIAPI
PchAlternateAccessMode (
  IN  UINTN         IlbBase,
  IN  BOOLEAN       AmeCtrl
  )
/**

  This function can be called to enable/disable Alternate Access Mode

  @param[in] IlbBase              The PCH ILB Base Address
  @param[in] AmeCtrl              If TRUE, enable Alternate Access Mode.
                                  If FALSE, disable Alternate Access Mode.

  @retval NONE

**/
{
  UINT32  Data32Or;
  UINT32  Data32And;

  Data32Or  = 0;
  Data32And = 0xFFFFFFFF;

  if (AmeCtrl == TRUE) {
    ///
    /// Enable Alternate Access Mode
    /// Note: The RTC Index field (including the NMI mask at bit7) is write-only
    /// for normal operation and can only be read in Alt Access Mode.
    ///
    Data32Or  = (UINT32) (B_PCH_ILB_MC_AME);
  }

  if (AmeCtrl == FALSE) {
    ///
    /// Disable Alternate Access Mode
    ///
    Data32And = (UINT32) ~(B_PCH_ILB_MC_AME);
  }

  ///
  /// Program Alternate Access Mode Enable bit
  ///
  MmioAndThenOr32 (
    IlbBase + R_PCH_ILB_MC,
    Data32And,
    Data32Or
    );

  ///
  /// Reads back for posted write to take effect
  ///
  MmioRead32(IlbBase + R_PCH_ILB_MC);
}
