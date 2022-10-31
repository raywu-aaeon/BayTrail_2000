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
  PchSmmSx.c

  @brief
  File to contain all the hardware specific stuff for the Smm Sx dispatch protocol.

**/
#include "PchSmmHelpers.h"

const PCH_SMM_SOURCE_DESC SX_SOURCE_DESC = {
  PCH_SMM_NO_FLAGS,
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_PCH_SMI_EN
      },
      S_PCH_SMI_EN,
      N_PCH_SMI_EN_ON_SLP_EN
    },
    NULL_BIT_DESC_INITIALIZER
  },
  {
    {
      {
        ACPI_ADDR_TYPE,
        R_PCH_SMI_STS
      },
      S_PCH_SMI_STS,
      N_PCH_SMI_STS_ON_SLP_EN
    }
  }
};

VOID
SxGetContext (
  IN  DATABASE_RECORD    *Record,
  OUT PCH_SMM_CONTEXT    *Context
  )
/**

  @brief
  Get the Sleep type

  @param[in] Record               No use
  @param[in] Context              The context that includes SLP_TYP bits to be filled

  @retval None

**/
{
  UINT32  Pm1Cnt;

  Pm1Cnt = IoRead32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT));

  ///
  /// By design, the context phase will always be ENTRY
  ///
  Context->Sx.Phase = SxEntry;

  ///
  /// Map the PM1_CNT register's SLP_TYP bits to the context type
  ///
  switch (Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) {
    case V_PCH_ACPI_PM1_CNT_S0:
      Context->Sx.Type = SxS0;
      break;

    case V_PCH_ACPI_PM1_CNT_S1:
      Context->Sx.Type = SxS1;
      break;

    case V_PCH_ACPI_PM1_CNT_S3:
      Context->Sx.Type = SxS3;
      break;

    case V_PCH_ACPI_PM1_CNT_S4:
      Context->Sx.Type = SxS4;
      break;

    case V_PCH_ACPI_PM1_CNT_S5:
      Context->Sx.Type = SxS5;
      break;

    default:
      ASSERT (FALSE);
      break;
  }
}

BOOLEAN
SxCmpContext (
  IN PCH_SMM_CONTEXT     *Context1,
  IN PCH_SMM_CONTEXT     *Context2
  )
/**

  @brief
  Check whether sleep type of two contexts match

  @param[in] Context1             Context 1 that includes sleep type 1
  @param[in] Context2             Context 2 that includes sleep type 2

  @retval FALSE                   Sleep types match
  @retval TRUE                    Sleep types don't match

**/
{
  return (BOOLEAN) (Context1->Sx.Type == Context2->Sx.Type);
}
typedef struct {
  UINT8 Device;
  UINT8 Function;
} USB_CONTROLLER;

VOID
UsbS02SxWorkaround (
  VOID
  )
/**

  @brief
  VLV BIOS Spec Rev 0.5, Section 31.8.1
  Additional Programming Requirements prior to enter
  S4/S5

  @param[in] None

  @retval None

**/
{
  UINT8           Index;
  UINTN           EhciPciRegBase;
  UINT32          UsbBar;
  UINT16          CmdReg;
  UINT16          PowerState;
  USB_CONTROLLER  EhciControllersMap[PchEhciControllerMax] = {
    {
      PCI_DEVICE_NUMBER_PCH_USB,
      PCI_FUNCTION_NUMBER_PCH_EHCI
    }
  };

  ///
  /// System BIOS must execute the following steps prior to enter S4/S5.
  ///
  for (Index = 0; Index < PchEhciControllerMax; Index++) {
    ///
    /// Step 1
    /// Read "Memory Base Address (MEM_BASE) Register" of D29:F0
    ///
    EhciPciRegBase  = MmPciAddress (0, 0, EhciControllersMap[Index].Device, EhciControllersMap[Index].Function, 0);
    UsbBar          = MmioRead32 (EhciPciRegBase + R_PCH_EHCI_MEM_BASE) & B_PCH_EHCI_MEM_BASE_BAR;
    CmdReg          = MmioRead16 (EhciPciRegBase + R_PCH_EHCI_COMMAND_REGISTER);
    PowerState      = MmioRead16 (EhciPciRegBase + R_PCH_EHCI_PWR_CNTL_STS);

    if (UsbBar != 0xFFFFFFFF) {
      ///
      /// Check if the Ehci device is in D3 power state
      ///
      if ((PowerState & B_PCH_EHCI_PWR_CNTL_STS_PWR_STS) == V_PCH_EHCI_PWR_CNTL_STS_PWR_STS_D3) {
        ///
        /// Step 2
        /// Set "Power State" bit of PWR_CNTL_STS register, D26/D29:F0:54h [1:0] = 0h
        ///
        MmioWrite16 (EhciPciRegBase + R_PCH_EHCI_PWR_CNTL_STS, (PowerState &~B_PCH_EHCI_PWR_CNTL_STS_PWR_STS));
        ///
        /// Step 3
        /// Write back the value from step 1 to the "Memory Base Address (MEM_BASE) Register" of D26/D29:F0
        ///
        MmioWrite32 (EhciPciRegBase + R_PCH_EHCI_MEM_BASE, UsbBar);
        ///
        /// Step 4
        /// Enable "Memory Space Enable (MSE)" bit, set D26/D29:F0:04h [1] = 1b.
        ///
        MmioOr16 (
          EhciPciRegBase + R_PCH_EHCI_COMMAND_REGISTER,
          (UINT16) (B_PCH_EHCI_COMMAND_MSE)
          );
      }
      ///
      /// Step 5
      /// Clear "Asynchronous Schedule Enable" and "Periodic Schedule Enable" bits, if "Run/Stop (RS)" bit, MEM_BASE + offset 20h [0] = 1b.
      /// Proceed to steps below if "Run/Stop (RS)" bit, MEM_BASE + offset 20h [0] = 0b.
      ///
      if (!(MmioRead32 (UsbBar + R_PCH_EHCI_USB2CMD) & B_PCH_EHCI_USB2CMD_RS)) {
        MmioAnd32 (UsbBar + R_PCH_EHCI_USB2CMD, (UINT32)~(B_PCH_EHCI_USB2CMD_ASE | B_PCH_EHCI_USB2CMD_PSE));
        MmioOr32 (UsbBar + R_PCH_EHCI_USB2CMD, B_PCH_EHCI_USB2CMD_RS);
      }
      ///
      /// Step 6
      /// If "Port Enabled/Disabled" bit of Port N Status and Control (PORTSC) Register is set, MEM_BASE + 64h [2] = 1b,
      /// proceed steps below else continue with S4/S5.
      ///
      if ((MmioRead32 (UsbBar + R_PCH_EHCI_PORTSC0) & B_PCH_EHCI_PORTSC0_PORT_EN_DIS)) {
        ///
        /// Step 7
        /// Ensure that "Suspend" bit of Port N Status and Control (PORTSC) Register is set, MEM_BASE + 64h [7] = 1b.
        ///
        if (!(MmioRead32 (UsbBar + R_PCH_EHCI_PORTSC0) & B_PCH_EHCI_PORTSC0_SUSPEND)) {
          MmioOr32 (UsbBar + R_PCH_EHCI_PORTSC0, B_PCH_EHCI_PORTSC0_SUSPEND);
        }
        ///
        /// Step 8
        /// Set delay of 25ms
        ///
        PchPmTimerStall (25 * 1000);
        ///
        /// Step 9
        /// Clear "Run/Stop (RS)" bit, MEM_BASE + offset 20h [0] = 0b.
        ///
        MmioAnd32 (UsbBar + R_PCH_EHCI_USB2CMD, (UINT32)~(B_PCH_EHCI_USB2CMD_RS));
      }
      ///
      /// If the EHCI device is in D3 power state before executing this WA
      ///
      if ((PowerState & B_PCH_EHCI_PWR_CNTL_STS_PWR_STS) == V_PCH_EHCI_PWR_CNTL_STS_PWR_STS_D3) {
        ///
        /// Restore PCI Command Register
        ///
        MmioWrite16 (EhciPciRegBase + R_PCH_EHCI_COMMAND_REGISTER, CmdReg);
        ///
        /// Set "Power State" bit of PWR_CNTL_STS register to D3 state, D26/D29:F0:54h [1:0] = 3h
        ///
        MmioWrite16 (EhciPciRegBase + R_PCH_EHCI_PWR_CNTL_STS, PowerState);
      }
      ///
      /// Step 10
      /// Continue with S4/S5
      ///
    }
  }
}

VOID
XhciSxWorkaround (
  VOID
  )
/**

  @brief
  Additional xHCI Controller Configurations Prior to Entering S3/S4/S5

  @param[in] None

  @retval None

**/

{
  UINTN       PciD31F0RegBase;
  UINT32      PmcBase;
  UINTN       XhciPciMmBase;

  PciD31F0RegBase = MmPciAddress (0,
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_LPC,
                      PCI_FUNCTION_NUMBER_PCH_LPC,
                      0
                    );
  PmcBase         = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
  ///
  /// Check if XHCI controller is enabled
  ///
  if ((MmioRead32 (PmcBase + R_PCH_PMC_FUNC_DIS) & (UINT32) B_PCH_PMC_FUNC_DIS_USH) != 0) {
    return ;
  }
  XhciPciMmBase = MmPciAddress (
                    0,
                    DEFAULT_PCI_BUS_NUMBER_PCH,
                    PCI_DEVICE_NUMBER_PCH_XHCI,
                    PCI_FUNCTION_NUMBER_PCH_XHCI,
                    0
                    );
  ///
  /// Set D3hot state - 11b
  ///
  MmioOr16 ((XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS), (UINT16) V_PCH_XHCI_PWR_CNTL_STS_PWR_STS_D3);
  PchPmTimerStall (10 * 1000);

  ///
  /// Set "PME Enable" bit of PWR_CNTL_STS register, D20:F0:74h[8] = 1h
  ///
  MmioOr16 ((XhciPciMmBase + R_PCH_XHCI_PWR_CNTL_STS), (UINT16) (B_PCH_XHCI_PWR_CNTL_STS_PME_EN));
}

VOID
PchSmmSxGoToSleep (
  VOID
  )
/**

  @brief
  When we get an SMI that indicates that we are transitioning to a sleep state,
  we need to actually transition to that state.  We do this by disabling the
  "SMI on sleep enable" feature, which generates an SMI when the operating system
  tries to put the system to sleep, and then physically putting the system to sleep.

  @param[in] None

  @retval None.

**/
{
  UINT32  Pm1Cnt;

  ///
  /// Flush cache into memory before we go to sleep. It is necessary for S3 sleep
  /// because we may update memory in SMM Sx sleep handlers -- the updates are in cache now
  ///
  AsmWbinvd ();

  ///
  /// Disable SMIs
  ///
  PchSmmClearSource (&SX_SOURCE_DESC);
  PchSmmDisableSource (&SX_SOURCE_DESC);

  ///
  /// Get Power Management 1 Control Register Value
  ///
  Pm1Cnt = IoRead32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT));

  ///
  /// VLV BIOS Spec Rev 0.5, Section 31.8.1
  /// Additional Programming Requirements prior to enter S4/S5
  ///
  if (((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S4) ||
      ((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S5)) {
    UsbS02SxWorkaround ();
  }
  if (((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S3) ||
      ((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S4) ||
      ((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S5)) {
    XhciSxWorkaround ();
  }
  if (((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S3) ||
      ((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S4) ||
      ((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S5)) {
  }
  
  /// AMI_OVERRIDE for EIP159890 >>
  IoAnd8 ((UINTN) (AcpiBase + R_PCH_SMI_EN), (UINT8) ~(B_PCH_SMI_EN_SWSMI_TMR));
  /// AMI_OVERRIDE for EIP159890 <<

  ///
  /// Now that SMIs are disabled, write to the SLP_EN bit again to trigger the sleep
  ///
  Pm1Cnt |= B_PCH_ACPI_PM1_CNT_SLP_EN;

  IoWrite32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT), Pm1Cnt);

  ///
  /// Should only proceed if wake event is generated.
  ///
  if ((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SLP_TYP) == V_PCH_ACPI_PM1_CNT_S1) {
    while (((IoRead16 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_STS))) & B_PCH_ACPI_PM1_STS_WAK) == 0x0);
  } else {
    while(TRUE);
  }
  ///
  /// The system just went to sleep. If the sleep state was S1, then code execution will resume
  /// here when the system wakes up.
  ///
  Pm1Cnt = IoRead32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT));

  if ((Pm1Cnt & B_PCH_ACPI_PM1_CNT_SCI_EN) == 0) {
    ///
    /// An ACPI OS isn't present, clear the sleep information
    ///
    Pm1Cnt &= ~B_PCH_ACPI_PM1_CNT_SLP_TYP;
    Pm1Cnt |= V_PCH_ACPI_PM1_CNT_S0;

    IoWrite32 ((UINTN) (AcpiBase + R_PCH_ACPI_PM1_CNT), Pm1Cnt);
  }

  PchSmmClearSource (&SX_SOURCE_DESC);
  PchSmmEnableSource (&SX_SOURCE_DESC);
}
