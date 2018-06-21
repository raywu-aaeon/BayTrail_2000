/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2015 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  PchInit.c

  @brief
  This is the driver that initializes the Intel PCH.

--*/
#ifndef ECP_FLAG
#include <Library/UefiDriverEntryPoint.h>
#endif
#include "PchInit.h"
#include "PchRegs.h"
#ifdef ECP_FLAG
EFI_GUID gEfiPchInfoProtocolGuid = EFI_PCH_INFO_PROTOCOL_GUID;
EFI_GUID gDxePchPlatformPolicyProtocolGuid = DXE_PCH_PLATFORM_POLICY_PROTOCOL_GUID;
EFI_GUID gEfiPchS3SupportProtocolGuid = EFI_PCH_S3_SUPPORT_PROTOCOL_GUID;
#else
#include <Library/PchAslUpdateLib.h>
#endif
#include <IndustryStandard/Pci22.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciEnumerationComplete.h>

///
/// Global Variables
///
EFI_HANDLE  mImageHandle;
EFI_PCH_S3_SUPPORT_PROTOCOL           *mPchS3Support;
///
/// Local function prototypes
///
EFI_STATUS
InitializePchDevice (
  IN OUT PCH_INSTANCE_PRIVATE_DATA           *PchInstance,
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN     UINT32                              RootComplexBar,
  IN     UINT32                              PmcBase,
  IN     UINT32                              IlbBase,
  IN     UINT32                              SpiBase,
  IN     UINT32                              MPhyBase,
  IN     UINT16                              AcpiBase,
  IN     UINT16                              GpioBase
  );

EFI_STATUS
ProgramSvidSid (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  );

EFI_STATUS
ConfigureIrqAtBoot (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  );

VOID
EFIAPI
PchExitBootServicesEvent (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  );

VOID
EFIAPI
PchInitBeforeBoot (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  );

EFI_STATUS
ConfigureMiscAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  );

VOID
EFIAPI
PchInitAfterPciEnumeration (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  );

EFI_STATUS
EFIAPI
PchInitEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/**

  @brief
  This is the standard EFI driver point that detects
  whether there is an PCH southbridge in the system
  and if so, initializes the chip.

  @param[in] ImageHandle          Handle for the image of this driver
  @param[in] SystemTable          Pointer to the EFI System Table

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_OUT_OF_RESOURCES    Do not have enough resources to initialize the driver

**/
{
  EFI_STATUS                        Status;
  UINT8                             BusNumber;
  UINT32                            RootComplexBar;
  UINT32                            PmcBase;
  UINT32                            IoBase;
  UINT32                            IlbBase;
  UINT32                            SpiBase;
  UINT32                            MphyBase;
  DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy;
  UINTN                             NumHandles;
  EFI_HANDLE                        *HandleBuffer;
  UINT32                            Index;
  PCH_INSTANCE_PRIVATE_DATA         *PchInstance;
  UINT16                            AcpiBase;
  UINT16                            GpioBase;
  UINTN                             PciD31F0RegBase;
  //AMI_OVERRIDE - CSP20140401_22 Fix S3 wake issue when CRID enabled >>
  // CRID support for special BYT-D sku only
  PCH_STEPPING                      stepping;
  UINT32                            Data32Or;
  //AMI_OVERRIDE - CSP20140401_22 Fix S3 wake issue when CRID enabled <<

  DEBUG ((EFI_D_INFO, "PchInitEntryPoint() Start\n"));

  PchInstance       = NULL;
  PchPlatformPolicy = NULL;

  mImageHandle = ImageHandle;
#ifdef ECP_FLAG
  INITIALIZE_SCRIPT(ImageHandle, SystemTable);
#endif

  ///
  /// Retrieve all instances of PCH Platform Policy protocol
  ///
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gDxePchPlatformPolicyProtocolGuid,
                  NULL,
                  &NumHandles,
                  &HandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (Index = 0; Index < NumHandles; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gDxePchPlatformPolicyProtocolGuid,
                    (VOID **) &PchPlatformPolicy
                    );
    ASSERT_EFI_ERROR (Status);

    ///
    /// Allocate and install the PCH Info protocol
    ///
    BusNumber = PchPlatformPolicy->BusNumber;
    PciD31F0RegBase = MmPciAddress (0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPC,
                        PCI_FUNCTION_NUMBER_PCH_LPC,
                        0
                      );
    RootComplexBar  = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_RCBA) & B_PCH_LPC_RCBA_BAR;
    PmcBase         = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
    IoBase          = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_IO_BASE) & B_PCH_LPC_IO_BASE_BAR;
    IlbBase         = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_ILB_BASE) & B_PCH_LPC_ILB_BASE_BAR;
    SpiBase         = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_SPI_BASE) & B_PCH_LPC_SPI_BASE_BAR;
    MphyBase        = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_MPHY_BASE) & B_PCH_LPC_MPHY_BASE_BAR;
    AcpiBase        = MmioRead16 (PciD31F0RegBase + R_PCH_LPC_ACPI_BASE) & B_PCH_LPC_ACPI_BASE_BAR;
    GpioBase        = MmioRead16 (PciD31F0RegBase + R_PCH_LPC_GPIO_BASE) & B_PCH_LPC_GPIO_BASE_BAR;

    DEBUG ((EFI_D_INFO, "PCH Base Addresses:\n-------------------\n"));
    DEBUG ((EFI_D_INFO, "  RCBA     0x%X\n", RootComplexBar));
    DEBUG ((EFI_D_INFO, "  PmcBase  0x%X\n", PmcBase));
    DEBUG ((EFI_D_INFO, "  IoBase   0x%X\n", IoBase));
    DEBUG ((EFI_D_INFO, "  IlbBase  0x%X\n", IlbBase));
    DEBUG ((EFI_D_INFO, "  SpiBase  0x%X\n", SpiBase));
    DEBUG ((EFI_D_INFO, "  MphyBase 0x%X\n", MphyBase));
    DEBUG ((EFI_D_INFO, "  AcpiBase 0x%X\n", AcpiBase));
    DEBUG ((EFI_D_INFO, "  GpioBase 0x%X\n", GpioBase));
    DEBUG ((EFI_D_INFO, "-------------------\n"));

    ASSERT (RootComplexBar != 0);
    ASSERT (PmcBase != 0);
    ASSERT (IoBase != 0);
    ASSERT (IlbBase != 0);
    ASSERT (SpiBase != 0);
    ASSERT (MphyBase != 0);
    ASSERT (AcpiBase != 0);
#if GPIO_ACCESS
    ASSERT (GpioBase != 0);
#endif

    ///
    /// Dump whole DXE_PCH_PLATFORM_POLICY_PROTOCOL and serial out.
    ///
#if defined(PCH_DEBUG_INFO) && PCH_DEBUG_INFO == 1
    PchDumpPlatformProtocol (PchPlatformPolicy);
#endif
    ///
    /// Initialize the PCH device
    ///
    InitializePchDevice (PchInstance, PchPlatformPolicy, RootComplexBar, PmcBase, IlbBase, SpiBase, MphyBase, AcpiBase, GpioBase);

    //AMI_OVERRIDE - CSP20140401_22 Fix S3 wake issue when CRID enabled >>
    // CRID support for special BYT-D sku only
    if (PchPlatformPolicy->DeviceEnabling->Crid == PCH_DEVICE_ENABLE) {
      stepping = PchStepping();
      if (stepping >= PchB3) {
        MmioOr32 ((UINTN) (PmcBase + R_PCH_PMC_CRID), (UINT32) 0x01);
    
        Data32Or = (UINT32) 0x01;
        S3BootScriptSaveMemWrite(
          EfiBootScriptWidthUint32,
          (UINTN) (PmcBase + R_PCH_PMC_CRID),
          1,
          &Data32Or
          );
        }
    }
    //AMI_OVERRIDE - CSP20140401_22 Fix S3 wake issue when CRID enabled <<
      
    PchInstance = AllocateZeroPool (sizeof (PCH_INSTANCE_PRIVATE_DATA));
    if (PchInstance == NULL) {
      ASSERT (FALSE);
      return EFI_OUT_OF_RESOURCES;
    }
    PchInstance->PchInfo.Revision   = PCH_INFO_PROTOCOL_REVISION_1;
    PchInstance->PchInfo.BusNumber  = BusNumber;
    PchInstance->PchInfo.RCVersion  = PCH_RC_VERSION;

    Status = gBS->InstallMultipleProtocolInterfaces (
                    &(HandleBuffer[Index]),
                    &gEfiPchInfoProtocolGuid,
                    &(PchInstance->PchInfo),
                    NULL
                    );
  }

  (gBS->FreePool) (HandleBuffer);
  DEBUG ((EFI_D_INFO, "PchInitEntryPoint() End\n"));

  return EFI_SUCCESS;
}

// AMI_OVERRIDE - EIP309084 >>
VOID
BytIBwgAddendumWa (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
{
  // BayTrail-I BWG Addendum 2.5 section A.9
  MmioWrite32(IO_BASE_ADDRESS + 0x1130, 0x2003CC82);  // HV_DDI0_HPD
  MmioWrite32(IO_BASE_ADDRESS + 0x1180, 0x2003CC82);  // HV_DDI1_HPD
}
// AMI_OVERRIDE - EIP309084 <<

EFI_STATUS
InitializePchDevice (
  IN OUT PCH_INSTANCE_PRIVATE_DATA           *PchInstance,
  IN     DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN     UINT32                              RootComplexBar,
  IN     UINT32                              PmcBase,
  IN     UINT32                              IlbBase,
  IN     UINT32                              SpiBase,
  IN     UINT32                              MphyBase,
  IN     UINT16                              AcpiBase,
  IN     UINT16                              GpioBase
  )
/**

  @brief
  Initialize the PCH device according to the PCH Platform Policy protocol

  @param[in] PchInstance          PCH instance private data. May get updated by this function
  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar value of this PCH device
  @param[in] PmcBase              PMC base address of this PCH device
  @param[in] IlbBase              iLB base address of this PCH device
  @param[in] SpiBase              SPI base address of this PCH device
  @param[in] MphyBase             MPHY base address of this PCH device
  @param[in] AcpiBase             ACPI IO base address of this PCH device
  @param[in] GpioBase             GPIO base address of this PCH device

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  EFI_STATUS  Status;
  BOOLEAN     AzaliaEnable;
  UINT32      FuncDisableReg;
  UINTN       PciD31F0RegBase;
  VOID        *Registration;
  EFI_EVENT   LegacyBootEvent;
  EFI_EVENT   ExitBootServicesEvent;
  EFI_EVENT   ReadyToBootEvent;
  PCH_STEPPING stepping;
  EFI_EVENT   Event;
  VOID        *RegistrationExitPmAuth;

  PciD31F0RegBase = MmPciAddress (0,
                      DEFAULT_PCI_BUS_NUMBER_PCH,
                      PCI_DEVICE_NUMBER_PCH_LPC,
                      PCI_FUNCTION_NUMBER_PCH_LPC,
                      0
                      );

  DEBUG ((EFI_D_INFO, "InitializePchDevice() Start\n"));

  FuncDisableReg  = MmioRead32 (PmcBase + R_PCH_PMC_FUNC_DIS);
  ///
  ///
  /// Miscellaneous power management handling
  ///
  Status = ConfigureMiscPm (PchPlatformPolicy, PmcBase, GpioBase);
  ASSERT_EFI_ERROR (Status);
  ///
  /// Additional power management setting
  ///
  Status = ConfigureAdditionalPm (PchPlatformPolicy);
  ASSERT_EFI_ERROR (Status);
  ///
  /// Deep Sx Enabling
  ///
  Status = ProgramDeepSx (PchPlatformPolicy, RootComplexBar);
  ASSERT_EFI_ERROR (Status);
  ///
  /// S0ix Enabling
  ///
  if (PchPlatformPolicy->S0ixSupport == PCH_DEVICE_ENABLE) {
    Status = ConfigureS0ix (PchPlatformPolicy, PmcBase);
    ASSERT_EFI_ERROR (Status);
  }

  //
  // GPE event mapping to GPIO event under AcpiHwRed mode.
  //
  if (PchPlatformPolicy->AcpiHWRed == PCH_DEVICE_ENABLE) {
    ConfigureAcpiHwRed(AcpiBase);
  }

  ///
  /// Perform PCH initialization sequence
  ///
  Status = ConfigureMiscItems (PchPlatformPolicy, RootComplexBar, PmcBase, IlbBase, &FuncDisableReg);
  ASSERT_EFI_ERROR (Status);
#if (_SLE_COMP_)
  AzaliaEnable = FALSE;
#else
  ///
  /// Detect and initialize the type of codec present in the system
  ///
  Status = ConfigureAzalia (PchPlatformPolicy, RootComplexBar, &AzaliaEnable);
  ASSERT_EFI_ERROR (Status);
#endif
  ///
  /// Check to disable Azalia controller
  ///
  DEBUG ((EFI_D_ERROR, "AzaliaEnable = %d \n",AzaliaEnable));
  if (!AzaliaEnable) {
    FuncDisableReg |= B_PCH_PMC_FUNC_DIS_AZALIA;
  }
  stepping = PchStepping();
  if (stepping < PchC0) {
    if (PchPlatformPolicy->DeviceEnabling->Azalia == PCH_DEVICE_DISABLE) {
      ///
      /// Create a Ready to Boot event.
      ///
      Status = EfiCreateEventReadyToBootEx (
                TPL_CALLBACK,
                HDA_4683125_WA,
                NULL,
                &ReadyToBootEvent
                );
      ASSERT_EFI_ERROR (Status);
    }
  }

  ///
  ///  Configure OTG Device
  ///
  Status = ConfigureOtg (PchPlatformPolicy, &FuncDisableReg);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Configure USB
  ///
  Status = ConfigureUsb (PchPlatformPolicy, RootComplexBar, &FuncDisableReg);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Initialize PCIE root ports
  ///
  Status = PchInitRootPorts (PchPlatformPolicy, RootComplexBar, PmcBase, AcpiBase, &FuncDisableReg);
  ASSERT_EFI_ERROR (Status);

  ///
  ///  Configure SATA Controllers
  ///
  Status = ConfigureSata (PchPlatformPolicy, RootComplexBar, &FuncDisableReg, GpioBase);
  ASSERT_EFI_ERROR (Status);

  ///
  ///  Configure LPE Devices
  ///
  Status = ConfigureLpe (PchPlatformPolicy, &FuncDisableReg);
  ASSERT_EFI_ERROR (Status);

  ///
  ///  Configure LPIO Devices
  ///
  Status = ConfigureLpss (PchPlatformPolicy, &FuncDisableReg);
  ASSERT_EFI_ERROR (Status);

  ///
  ///  Configure SCC Devices
  ///
  Status = ConfigureScc (PchPlatformPolicy, &FuncDisableReg);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Set the VLV Function Disable Register
  ///
  MmioWrite32 ((UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS), (UINT32) (FuncDisableReg));

  ///
  /// Reads back for posted write to take effect
  ///
  MmioRead32 ((UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS));

  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS),
    1,
    &FuncDisableReg
  );

  //
  // Reads back for posted write to take effect
  //
  S3BootScriptSaveMemReadWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS),
    &FuncDisableReg,  /// Data to be ORed
    &FuncDisableReg   /// Data to be ANDed
  );

  ///
  /// Perform clock gating register settings
  /// No clock gating configuration is required for now until there is fix needed by BIOS.
  ///
  Status = ConfigureClockGating (PchPlatformPolicy, RootComplexBar, PmcBase, SpiBase, FuncDisableReg);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Configure IOAPIC
  ///
  Status = ConfigureIoApic (PchPlatformPolicy, IlbBase);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Program Subsystem Vendor ID & Subsystem ID
  ///
// AMI_OVERRIDE - SvidSid will be programming by Sbpei.c. >>
//  Status = ProgramSvidSid (PchPlatformPolicy);
//  ASSERT_EFI_ERROR (Status);
// AMI_OVERRIDE - SvidSid will be programming by Sbpei.c. <<

// AMI_OVERRIDE - EIP309084 >>
  Status = EfiCreateEventReadyToBootEx (
            TPL_CALLBACK,
            BytIBwgAddendumWa,
            NULL,
            &ReadyToBootEvent
            );
  ASSERT_EFI_ERROR (Status);
// AMI_OVERRIDE - EIP309084 <<  

  ///
  /// Create an ExitPmAuth protocol call back event.
  ///
//  EfiCreateProtocolNotifyEvent (
//    &gExitPmAuthProtocolGuid,
//    TPL_CALLBACK,
//    PchInitBeforeBoot,
//    NULL,
//    &Registration
//  );

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  PchInitAfterPciEnumeration,
                  NULL,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->RegisterProtocolNotify (
                  &gExitPmAuthProtocolGuid,
                  Event,
                  &RegistrationExitPmAuth
                  );
  ASSERT_EFI_ERROR (Status);

  ///
  /// Create an gEfiPciEnumerationCompleteProtocolGuid protocol call back event.
  ///
  EfiCreateProtocolNotifyEvent (
      &gEfiPciEnumerationCompleteProtocolGuid,
      TPL_CALLBACK,
      PchInitBeforeBoot,
      &gEfiPciEnumerationCompleteProtocolGuid,
      &Registration
  );
  
  
  
  ///
  /// Create events for PCH to do the task before ExitBootServices/LegacyBoot.
  /// It is guaranteed that only one of two events below will be signalled
  ///
  Status = gBS->CreateEvent (
                  EVT_SIGNAL_EXIT_BOOT_SERVICES,
                  TPL_CALLBACK,
                  PchExitBootServicesEvent,
                  NULL,
                  &ExitBootServicesEvent
                  );
  ASSERT_EFI_ERROR (Status);

  Status = EfiCreateEventLegacyBootEx (
             TPL_CALLBACK,
             PchExitBootServicesEvent,
             NULL,
             &LegacyBootEvent
             );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "InitializePchDevice() End\n"));

  return Status;
}

EFI_STATUS
ProgramSvidSid (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  )
/**

  @brief
  Program Pch devices Subsystem Vendor Identifier (SVID) and Subsystem Identifier (SID).

  @param[in] PchPlatformPolicy  The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINT8                           Index;
  UINT16                          EhciAccessCntl;
  UINT8                           BusNumber;
  UINTN                           PciEAddressBase;
  UINT8                           DeviceNumber;
  UINT8                           FunctionNumber;
  UINT8                           SvidRegOffset;
  BOOLEAN                         IsPchEhci;
  UINT32                          SidSvid;
  STATIC PCH_SVID_SID_INIT_ENTRY  SvidSidInitTable[] = {
    {
      PCI_DEVICE_NUMBER_PCH_SMBUS,
      PCI_FUNCTION_NUMBER_PCH_SMBUS,
      R_PCH_SMBUS_SVID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPC,
      PCI_FUNCTION_NUMBER_PCH_LPC,
      R_PCH_LPC_SS
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_SPI,
      PCI_FUNCTION_NUMBER_PCH_LPSS_SPI,
      R_PCH_LPSS_SPI_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_HSUART,
      PCI_FUNCTION_NUMBER_PCH_LPSS_HSUART1,
      R_PCH_LPSS_HSUART_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_HSUART,
      PCI_FUNCTION_NUMBER_PCH_LPSS_HSUART0,
      R_PCH_LPSS_HSUART_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_PWM,
      PCI_FUNCTION_NUMBER_PCH_LPSS_PWM1,
      R_PCH_LPSS_PWM_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_PWM,
      PCI_FUNCTION_NUMBER_PCH_LPSS_PWM0,
      R_PCH_LPSS_PWM_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_DMAC0,
      PCI_FUNCTION_NUMBER_PCH_LPSS_DMAC,
      R_PCH_LPSS_DMAC_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_USB,
      PCI_FUNCTION_NUMBER_PCH_EHCI,
      R_PCH_EHCI_SVID
    },
    {
      PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
      PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_4,
      R_PCH_PCIE_SVID
    },
    {
      PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
      PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_3,
      R_PCH_PCIE_SVID
    },
    {
      PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
      PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_2,
      R_PCH_PCIE_SVID
    },
    {
      PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
      PCI_FUNCTION_NUMBER_PCH_PCIE_ROOT_PORT_1,
      R_PCH_PCIE_SVID
    },

    {
      PCI_DEVICE_NUMBER_PCH_AZALIA,
      PCI_FUNCTION_NUMBER_PCH_AZALIA,
      R_PCH_HDA_SVID
    },
    {
      26,//SEC device
      0,
      0x2c
    },
    {
      2,//VGA device
      0,
      0x2c
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
      PCI_FUNCTION_NUMBER_PCH_LPSS_I2C6,
      R_PCH_LPSS_I2C_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
      PCI_FUNCTION_NUMBER_PCH_LPSS_I2C5,
      R_PCH_LPSS_I2C_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
      PCI_FUNCTION_NUMBER_PCH_LPSS_I2C4,
      R_PCH_LPSS_I2C_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
      PCI_FUNCTION_NUMBER_PCH_LPSS_I2C3,
      R_PCH_LPSS_I2C_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
      PCI_FUNCTION_NUMBER_PCH_LPSS_I2C2,
      R_PCH_LPSS_I2C_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
      PCI_FUNCTION_NUMBER_PCH_LPSS_I2C1,
      R_PCH_LPSS_I2C_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_I2C,
      PCI_FUNCTION_NUMBER_PCH_LPSS_I2C0,
      R_PCH_LPSS_I2C_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPSS_DMAC1,
      PCI_FUNCTION_NUMBER_PCH_LPSS_DMAC,
      R_PCH_LPSS_DMAC_SSID
    },
    //eMMC 4.5 device
    {
      PCI_DEVICE_NUMBER_PCH_SCC_SDIO_3,
      PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,
      R_PCH_SCC_SDIO_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_OTG,
      PCI_FUNCTION_NUMBER_PCH_OTG,
      R_PCH_OTG_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_LPE,
      PCI_FUNCTION_NUMBER_PCH_LPE,
      R_PCH_LPE_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_XHCI,
      PCI_FUNCTION_NUMBER_PCH_XHCI,
      R_PCH_XHCI_SVID
    },
    {
      PCI_DEVICE_NUMBER_PCH_SATA,
      PCI_FUNCTION_NUMBER_PCH_SATA,
      R_PCH_SATA_SS
    },
    {
      PCI_DEVICE_NUMBER_PCH_SCC_SDIO_2,
      PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,
      R_PCH_SCC_SDIO_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_SCC_SDIO_1,
      PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,
      R_PCH_SCC_SDIO_SSID
    },
    {
      PCI_DEVICE_NUMBER_PCH_SCC_SDIO_0,
      PCI_FUNCTION_NUMBER_PCH_SCC_SDIO,
      R_PCH_SCC_SDIO_SSID
    }
  };

  DEBUG ((EFI_D_INFO, "ProgramSvidSid() Start\n"));

  EhciAccessCntl  = 0;
  BusNumber       = PchPlatformPolicy->BusNumber;
  SidSvid         = (UINT32) (((PchPlatformPolicy->DefaultSvidSid->SubSystemId) << 16) | (PchPlatformPolicy->DefaultSvidSid->SubSystemVendorId));

  if ((PchPlatformPolicy->DefaultSvidSid->SubSystemVendorId != 0) ||
      (PchPlatformPolicy->DefaultSvidSid->SubSystemId != 0)) {
    for (Index = 0; Index < (sizeof (SvidSidInitTable) / sizeof (PCH_SVID_SID_INIT_ENTRY)); Index++) {
      DeviceNumber    = SvidSidInitTable[Index].DeviceNumber;
      FunctionNumber  = SvidSidInitTable[Index].FunctionNumber;
      SvidRegOffset   = SvidSidInitTable[Index].SvidRegOffset;
      PciEAddressBase = MmPciAddress (0,
                          BusNumber,
                          DeviceNumber,
                          FunctionNumber,
                          0
                          );
      ///
      /// Skip if the device is disabled
      ///
      if (MmioRead16 (PciEAddressBase) != V_PCH_INTEL_VENDOR_ID) {
        continue;
      }

      IsPchEhci = IS_PCH_EHCI (DeviceNumber, FunctionNumber);

      ///
      /// Set EHCI devices WRT_RDONLY bit (D29:F0:80h, bit 0) to 1, to make SVID and SID registers are writable
      ///
      if (IsPchEhci) {
        EhciAccessCntl = MmioRead16 ((UINTN) (PciEAddressBase + R_PCH_EHCI_ACCESS_CNTL));
        MmioOr16 ((UINTN) (PciEAddressBase + R_PCH_EHCI_ACCESS_CNTL), B_PCH_EHCI_ACCESS_CNTL_WRT_RDONLY);
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint16,
          (UINTN) (PciEAddressBase + R_PCH_EHCI_ACCESS_CNTL),
          1,
          (VOID *) (UINTN) (PciEAddressBase + R_PCH_EHCI_ACCESS_CNTL)
          );
      }

      ///
      /// Program VLV devices Subsystem Vendor Identifier (SVID) and Subsystem Identifier (SID)
      ///
      DEBUG ((EFI_D_INFO, "Writing SVID/SID for B%d/D%d/F%d\n", BusNumber, DeviceNumber, FunctionNumber));

      MmioWrite32 (
        (UINTN) (PciEAddressBase + SvidRegOffset),
        SidSvid
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (PciEAddressBase + SvidRegOffset),
        1,
        (VOID *) (UINTN) (PciEAddressBase + SvidRegOffset)
        );

      ///
      /// Restore the EHCI devices WRT_RDONLY bit (D29:F0:80h, bit 0) value
      ///
      if (IsPchEhci) {
        MmioWrite16 ((UINTN) (PciEAddressBase + R_PCH_EHCI_ACCESS_CNTL), EhciAccessCntl);
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint16,
          (UINTN) (PciEAddressBase + R_PCH_EHCI_ACCESS_CNTL),
          1,
          &EhciAccessCntl
          );
      }
    }
  }

  DEBUG ((EFI_D_INFO, "ProgramSvidSid() End\n"));

  return EFI_SUCCESS;
}



EFI_STATUS
ConfigureIrqAtBoot (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  )
/**

  @brief
  Program Pch devices dedicated IRQ#.

  @param[in] PchPlatformPolicy  The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINTN                                      Index;
  UINTN                                      TblIndex;
  UINTN                                      Segment;
  UINTN                                      BusNum;
  UINTN                                      DevNum;
  UINTN                                      FunNum;
  UINTN                                      HandleCount;
  EFI_STATUS                                 Status;
  EFI_HANDLE                                 *Handles;
  EFI_PCI_IO_PROTOCOL                        *PciIo;
  STATIC PCH_IRQ_INIT_ENTRY                  IrqInitTable[] = {
    /*Bus  Dev  Fun  Irq*/
    {  0,   21,  0,   29 }, /* LPE Audio  */
    {  0,   24,  1,   32 }, /* I2C1       */
    {  0,   24,  2,   33 }, /* I2C2       */
    {  0,   24,  3,   34 }, /* I2C3       */
    {  0,   24,  4,   35 }, /* I2C4       */
    {  0,   24,  5,   36 }, /* I2C5       */
    {  0,   24,  6,   37 }, /* I2C6       */
    {  0,   24,  7,   38 }, /* I2C7       */
    {  0,   30,  3,   39 }, /* HSUART1    */
    {  0,   30,  4,   40 }, /* HSUART1    */
    {  0,   30,  5,   41 }, /* SPI1       */
    {  0,   30,  0,   42 }, /* LI01 DMA   */
    {  0,   24,  0,   43 }, /* LI02 DMA   */
    {  0,   23,  0,   44 }, /* MIPI-HSI   */
    {  0,   16,  0,   45 }, /* SDIO1/eMMC */
    {  0,   17,  0,   46 }, /* SDIO2/SDIO */
    {  0,   18,  0,   47 }, /* SDIO3/SD   */
    {  0,    2,  0,    7 }, /* DISPLAY    */
    {  0,   29,  0,   11 }, /* USB        */
    {  0,   20,  0,   23 }, /* OTG        */
    {  0,   22,  0,   16 }, /* XHCI       */
    {  0,   26,  0,   23 }, /* SEC        */
    {  0,   27,  0,   22 }  /* HD Audio   */
  };

  DEBUG ((EFI_D_ERROR, "ConfigureIrqAtBoot() Start\n"));

  if (PchPlatformPolicy->LpssConfig->LpssPciModeEnabled) {

    Status = gBS->LocateHandleBuffer (
                    ByProtocol,
                    &gEfiPciIoProtocolGuid,
                    NULL,
                    &HandleCount,
                    &Handles
                    );

    DEBUG ((EFI_D_ERROR, "Status = %r\n", Status));

    if (EFI_ERROR(Status)) {
      return Status;
    }

    DEBUG ((EFI_D_ERROR, "Status = %r\n", Status));

    for(Index = 0; Index < HandleCount; Index ++) {
      Status = gBS->HandleProtocol (
                      Handles[Index],
                      &gEfiPciIoProtocolGuid,
                      (VOID **) &PciIo
                      );
      if (EFI_ERROR(Status)) {
        continue;
      }

      Status = PciIo->GetLocation(
                        PciIo,
                        &Segment,
                        &BusNum,
                        &DevNum,
                        &FunNum
                        );

      for (TblIndex = 0; TblIndex < (sizeof (IrqInitTable) / sizeof (PCH_IRQ_INIT_ENTRY)); TblIndex++) {
        if((BusNum == IrqInitTable[TblIndex].BusNumber) &&
            (DevNum == IrqInitTable[TblIndex].DeviceNumber) &&
            (FunNum == IrqInitTable[TblIndex].FunctionNumber)) {

          Status = PciIo->Pci.Write(
                                PciIo,
                                EfiPciIoWidthUint8,
                                0x3c,//Offset 0x3c :PCI Interrupt Line
                                1,
                                &IrqInitTable[TblIndex].Irq
                                );

          DEBUG ((EFI_D_ERROR, "Writing IRQ#%d for B%d/D%d/F%d\n", IrqInitTable[TblIndex].Irq, BusNum, DevNum, FunNum));
        } else {
          continue;
        }

      }

    }

    DEBUG ((EFI_D_ERROR, "ConfigureIrqAtBoot() End\n"));
  }
  return EFI_SUCCESS;
}

EFI_STATUS
LockPciDevCap (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy
  )
/**

  @brief
  Lock Pci devices capabilities.

  @param[in] PchPlatformPolicy  The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINT8       BusNumber;
  UINTN       PciEAddressBase;
  UINT8       Device;
  UINT8       Function;
  UINT16      Reg16;
  UINT8       CapId;

  BusNumber = PchPlatformPolicy->BusNumber;
  DEBUG ((EFI_D_INFO, "LockPciDevCap() BusNumber = 0x%x\n",BusNumber));

  for (Device = 0; Device <= PCI_MAX_DEVICE; Device++) {
    for (Function = 0; Function <= PCI_MAX_FUNC; Function++) {

      PciEAddressBase = MmPciAddress (0,
                          BusNumber,
                          Device,
                          Function,
                          0
                          );

      Reg16 = MmioRead16(
                (UINTN) (PciEAddressBase + PCI_VENDOR_ID_OFFSET)
                );

      if(Reg16 == 0xFFFF)
        continue;

      DEBUG ((EFI_D_INFO, "LockPciDevCap() Bus = 0x%x,Dev = 0x%x,Func = 0x%x\n",BusNumber,Device,Function));
      DEBUG ((EFI_D_INFO, "LockPciDevCap() PciEAddressBase = 0x%x\n",PciEAddressBase));

      Reg16 =MmioRead16(
               (UINTN) (PciEAddressBase + PCI_PRIMARY_STATUS_OFFSET)
               );

      DEBUG ((EFI_D_INFO, "LockPciDevCap() StatusReg = 0x%x\n",Reg16));
      if((Reg16 & EFI_PCI_STATUS_CAPABILITY)) {

        DEBUG ((EFI_D_INFO, "LockPciDevCap() StatusReg = 0x%x\n",Reg16));

        CapId =  MmioRead8 (
                   (UINTN) (PciEAddressBase + PCI_CAPBILITY_POINTER_OFFSET)
                   );

        MmioWrite8(
          (UINTN) (PciEAddressBase + PCI_CAPBILITY_POINTER_OFFSET),
          CapId
          );
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint8,
          (UINTN) (PciEAddressBase + PCI_CAPBILITY_POINTER_OFFSET),
          1,
          &CapId
          );
        DEBUG ((EFI_D_INFO, "LockPciDevCap() CapId = 0x%x\n",CapId));

        while(CapId != 0x00) {

          Reg16 =  MmioRead16 (
                     (UINTN) (PciEAddressBase + CapId)
                     );
          MmioWrite16(
            (UINTN) (PciEAddressBase + CapId),
            Reg16
            );
          S3BootScriptSaveMemWrite (
            EfiBootScriptWidthUint16,
            (UINTN) (PciEAddressBase + CapId),
            1,
            &Reg16
            );
          DEBUG ((EFI_D_INFO, "LockPciDevCap() While CAPBILITY = 0x%x\n",Reg16));
          Reg16 = (Reg16 & 0xFF00);
          Reg16 >>= 0x08;
          CapId = (UINT8)Reg16;
          DEBUG ((EFI_D_INFO, "LockPciDevCap() While CapId = 0x%x\n",CapId));
        }
      }
    }
  }
  return EFI_SUCCESS;
}
EFI_STATUS
PciERWORegInit (
  IN      DXE_PCH_PLATFORM_POLICY_PROTOCOL    *PchPlatformPolicy,
  IN      UINT32                              RootComplexBar,
  IN OUT  UINT32                              *FuncDisableReg
  )
/**

  @brief
  Initialize R/WO Registers that described in PCH BIOS Spec

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance
  @param[in] RootComplexBar       RootComplexBar address of this PCH device
  @param[in] FuncDisableReg       The value of Function disable register

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINTN   Index;
  UINT8   RootPortFunction;
  UINTN   RPBase;
  UINT16  Data16;
  UINT8   Data8;
  UINT8   BusNumber;
  UINTN   AzaliaBase;
  UINT32  Data32;

  DEBUG ((EFI_D_INFO, "PciERWORegInit() Start\n"));

  BusNumber = PchPlatformPolicy->BusNumber;

  ///
  /// PCH BIOS Spec Rev 1.1.0, Section 5.12 R/WO Registers, Table 5-4
  /// System BIOS must read the register and write the same value back to the register
  /// before passing control to the operating system.
  /// Dev:Func/Type Register Offset Register Name                            Bits
  /// D27:F0         074h            Device Capabilities                      28,11:6
  /// D28:F0-F7    04Ch            Link Capabilities                        11:10
  /// D28:F0-F7    094h            Subsystem Vendor ID                      31:0
  /// D28:F0-F7    054h            Hot Plug Capable / Surprise              6:5
  /// D28:F0-F7    034h            Capabilities Pointer                     7:0
  /// D28:F0-F7    040h            Primary IDE Timing                       15:8
  /// D28:F0-F7     064h            Device Capabilities 2                    11
  /// D28:F0-F7    080h            Message Signaled Interrupt Capability ID 15:8
  /// D28:F0-F7    090h            Port Mapping Register                    15:8
  ///
  for (Index = 0; Index < PCH_PCIE_MAX_ROOT_PORTS; Index++) {
    if (((*FuncDisableReg) & (B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0 << Index)) == 0) {
      RootPortFunction  = PchPlatformPolicy->PciExpressConfig->RootPort[Index].FunctionNumber;
      RPBase            = MmPciAddress (0, BusNumber, PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS, RootPortFunction, 0);
      Data32            = MmioRead32 (RPBase + R_PCH_PCIE_LCAP);
      MmioWrite32 (RPBase + R_PCH_PCIE_LCAP, Data32);
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (RPBase + R_PCH_PCIE_LCAP),
        1,
        &Data32
        );

      Data32 = MmioRead32 (RPBase + R_PCH_PCIE_SVID);
      MmioWrite32 (RPBase + R_PCH_PCIE_SVID, Data32);
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (RPBase + R_PCH_PCIE_SVID),
        1,
        &Data32
        );

      Data32 = MmioRead32 (RPBase + R_PCH_PCIE_SLCAP);
      MmioWrite32 (RPBase + R_PCH_PCIE_SLCAP, Data32);
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (RPBase + R_PCH_PCIE_SLCAP),
        1,
        &Data32
        );
      ///
      /// Added PCIe register to be lockdown
      ///
      Data8 = MmioRead8 (RPBase + R_PCH_PCIE_CAPP);
      MmioWrite8 (RPBase + R_PCH_PCIE_CAPP, Data8);
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint8,
        (UINTN) (RPBase + R_PCH_PCIE_CAPP),
        1,
        &Data8
        );

      Data16 = MmioRead16 (RPBase + R_PCH_PCIE_CLIST_XCAP);
      MmioWrite16 (RPBase + R_PCH_PCIE_CLIST_XCAP, Data16);
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint16,
        (UINTN) (RPBase + R_PCH_PCIE_CLIST_XCAP),
        1,
        &Data16
        );

      Data16 = MmioRead16 (RPBase + R_PCH_PCIE_MID_MC);
      MmioWrite16 (RPBase + R_PCH_PCIE_MID_MC, Data16);
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint16,
        (UINTN) (RPBase + R_PCH_PCIE_MID_MC),
        1,
        &Data16
        );

      Data16 = MmioRead16 (RPBase + R_PCH_PCIE_SVCAP);
      MmioWrite16 (RPBase + R_PCH_PCIE_SVCAP, Data16);
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint16,
        (UINTN) (RPBase + R_PCH_PCIE_SVCAP),
        1,
        &Data16
        );

      Data32 = MmioRead32 (RPBase + R_PCH_PCIE_DCAP2);
      MmioWrite32 (RPBase + R_PCH_PCIE_DCAP2, Data32);
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (RPBase + R_PCH_PCIE_DCAP2),
        1,
        &Data32
        );
    }
  }

  if ((*FuncDisableReg & B_PCH_PMC_FUNC_DIS_AZALIA) == 0) {
    AzaliaBase = MmPciAddress (0,
                   BusNumber,
                   PCI_DEVICE_NUMBER_PCH_AZALIA,
                   PCI_FUNCTION_NUMBER_PCH_AZALIA,
                   0
                 );

    Data32 = MmioRead32 (AzaliaBase + R_PCH_HDA_DEVCAP);
    MmioWrite32 (AzaliaBase + R_PCH_HDA_DEVCAP, Data32);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (AzaliaBase + R_PCH_HDA_DEVCAP),
      1,
      &Data32
      );
  }

  DEBUG ((EFI_D_INFO, "PciERWORegInit() End\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
SetPciePmS3Item (
  IN  UINT8                         RootPortBus,
  IN  UINT8                         RootPortDevice,
  IN  UINT8                         RootPortFunc,
  IN  PCH_PCI_EXPRESS_ASPM_CONTROL  RootPortAspm,
  IN  UINT8                         NumOfDevAspmOverride,
  IN  PCH_PCIE_DEVICE_ASPM_OVERRIDE *DevAspmOverride,
  IN  UINT8                         TempBusNumberMin,
  IN  UINT8                         TempBusNumberMax,
  IN  UINT8                         NumOfDevLtrOverride,
  IN  PCH_PCIE_DEVICE_LTR_OVERRIDE  *DevLtrOverride
  )
/**

  @brief
  Set a Root Port Downstream devices ASPM S3 dispatch item, this function may assert if any error happend

  @param[in] RootPortBus          Pci Bus Number of the root port
  @param[in] RootPortDevice       Pci Device Number of the root port
  @param[in] RootPortFunc         Pci Function Number of the root port
  @param[in] RootPortAspm         Root port Aspm configuration
  @param[in] NumOfDevAspmOverride Number of Device specific ASPM policy override items
  @param[in] DevAspmOverride      Pointer to array of Device specific ASPM policy override items
  @param[in] TempBusNumberMin     Minimal temp bus number that can be assigned to the root port (as secondary
                                  bus number) and its down stream switches
  @param[in] TempBusNumberMax     Maximal temp bus number that can be assigned to the root port (as subordinate
                                  bus number) and its down stream switches

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  EFI_STATUS                                      Status;
#ifndef ECP_FLAG
  EFI_BOOT_SCRIPT_SAVE_PROTOCOL                   *mBootScriptSave;
#endif
  STATIC EFI_PCH_S3_SUPPORT_PROTOCOL              *PchS3Support;
  STATIC EFI_PCH_S3_PARAMETER_PCIE_SET_PM         S3ParameterSetPm;
  STATIC EFI_PCH_S3_DISPATCH_ITEM                 S3DispatchItem = {
    PchS3ItemTypePcieSetPm,
    &S3ParameterSetPm
  };
  EFI_PHYSICAL_ADDRESS                            S3DispatchEntryPoint;

  if (!PchS3Support) {
    ///
    /// Get the PCH S3 Support Protocol
    ///
    Status = gBS->LocateProtocol (
                    &gEfiPchS3SupportProtocolGuid,
                    NULL,
                    (VOID **) &PchS3Support
                    );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  S3ParameterSetPm.RootPortBus           = RootPortBus;
  S3ParameterSetPm.RootPortDevice        = RootPortDevice;
  S3ParameterSetPm.RootPortFunc          = RootPortFunc;
  S3ParameterSetPm.RootPortAspm          = RootPortAspm;
  S3ParameterSetPm.NumOfDevAspmOverride  = NumOfDevAspmOverride;
  S3ParameterSetPm.DevAspmOverrideAddr   = (UINT32) (UINTN) DevAspmOverride;
  S3ParameterSetPm.TempBusNumberMin      = TempBusNumberMin;
  S3ParameterSetPm.TempBusNumberMax      = TempBusNumberMax;

  S3ParameterSetPm.NumOfDevLtrOverride  = NumOfDevLtrOverride;
  S3ParameterSetPm.DevLtrOverrideAddr   = (UINT32) (UINTN) DevLtrOverride;
  Status = PchS3Support->SetDispatchItem (
                          PchS3Support,
                          &S3DispatchItem,
                          &S3DispatchEntryPoint
                          );
  ASSERT_EFI_ERROR (Status);
  //
  // Save the script dispatch item in the Boot Script
  //
#ifdef ECP_FLAG
  S3BootScriptSaveDispatch(S3DispatchEntryPoint);
#else
  //S3BootScriptSaveDispatch((VOID *)(UINTN) S3DispatchEntryPoint);
  Status = gBS->LocateProtocol (
                  &gEfiBootScriptSaveProtocolGuid,
                  NULL,
                  (VOID **) &mBootScriptSave
                  );

  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    0,
                    EFI_BOOT_SCRIPT_DISPATCH_OPCODE,
                    S3DispatchEntryPoint
                    );
#endif
  return Status;
}

VOID
EFIAPI
PchExitBootServicesEvent (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
/**

  @brief
  PCH initialization before ExitBootServices / LegacyBoot events
  Useful for operations which must happen later than at EndOfPost event

  @param[in] Event                A pointer to the Event that triggered the callback.
  @param[in] Context              A pointer to private data registered with the callback function.

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  EFI_STATUS                        Status;
  UINTN                             NumHandles;
  EFI_HANDLE                        *HandleBuffer;
  DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy;
  UINTN                             Index;

  ///
  /// Closed the event to avoid call twice
  ///
  gBS->CloseEvent (Event);

  ///
  /// Retrieve all instances of PCH Platform Policy protocol
  ///
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gDxePchPlatformPolicyProtocolGuid,
                  NULL,
                  &NumHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Failed to locate handle buffer for PCH Policy protocol.\n"));
    return;
  }

  for (Index = 0; Index < NumHandles; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gDxePchPlatformPolicyProtocolGuid,
                    (VOID **) &PchPlatformPolicy
                    );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Failed to find PCH Policy protocol.\n"));
      return;
    }

    ConfigureXhciAtBoot (PchPlatformPolicy);
  }

  return;
}

EFI_STATUS
ConfigureMiscAtBoot (
  IN DXE_PCH_PLATFORM_POLICY_PROTOCOL *PchPlatformPolicy
  )
/**

  @brief
  Do any final miscellaneous initialization.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  UINTN       PciD31F0RegBase;
  UINT32      IoBase;
  EFI_STATUS  AcpiTablePresent;

  DEBUG ((EFI_D_INFO, "ConfigureMiscAtBoot() Start\n"));

  ///
  /// Initialize Variables
  ///
  PciD31F0RegBase  = 0;
  IoBase           = 0;
  AcpiTablePresent = EFI_NOT_FOUND;
  ///
  /// Locate ACPI table
  ///
  AcpiTablePresent = InitializePchAslUpdateLib ();
  ///
  /// Update LPE device ACPI variables
  ///
  if (!EFI_ERROR (AcpiTablePresent)) {
    PciD31F0RegBase = MmPciAddress (0,
                                    DEFAULT_PCI_BUS_NUMBER_PCH,
                                    PCI_DEVICE_NUMBER_PCH_LPC,
                                    PCI_FUNCTION_NUMBER_PCH_LPC,
                                    0
                                    );
    IoBase          = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_IO_BASE) & B_PCH_LPC_IO_BASE_BAR;
    ///
    /// Update CFIO BAR in ASL
    ///
    UpdateResourceTemplateAslCode (
      (SIGNATURE_32 ('C', 'F', 'I', '0')),
      (SIGNATURE_32 ('R', 'B', 'U', 'F')),
      AML_MEMORY32_FIXED_OP,
      1,
      0x04,
      &IoBase,
      sizeof (IoBase)
      );
  }

  DEBUG ((EFI_D_INFO, "ConfigureMiscAtBoot() End\n"));

  return EFI_SUCCESS;
}

VOID
EFIAPI
PchInitAfterPciEnumeration (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
/*++

Routine Description:

  PCH initialization before Boot Script Table is closed

Arguments:

  Event             A pointer to the Event that triggered the callback.
  Context           A pointer to private data registered with the callback function.

Returns:

  EFI_SUCCESS       The function completed successfully

  --*/
{
  EFI_STATUS                        Status;
  UINTN                             NumHandles;
  EFI_HANDLE                        *HandleBuffer;
  UINTN                             Index;
  DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy;
  UINTN   PciD29F0RegBase;
  UINTN   PciD20F0RegBase;
  UINT32  EhciBase;
  UINT32  XhciBase;
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, "PchInitAfterPciEnumeration - Callback.\n"));
  //
  // Closed the event to avoid call twice when launch shell
  //
  gBS->CloseEvent (Event);

  //
  // Retrieve all instances of PCH Platform Policy protocol
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gDxePchPlatformPolicyProtocolGuid,
                  NULL,
                  &NumHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Failed to locate handle buffer for PCH Policy protocol.\n"));
    return;
  }
  //
  // Find the matching PCH Policy protocol
  //
  for (Index = 0; Index < NumHandles; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gDxePchPlatformPolicyProtocolGuid,
                    (VOID **) &PchPlatformPolicy
                    );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Failed to find PCH Policy protocol.\n"));
      return;
    }

    Status = LockPciDevCap(PchPlatformPolicy);
    Status = ConfigureMiscAtBoot (PchPlatformPolicy);
    Status = ConfigureLpeAtBoot (PchPlatformPolicy);
    Status = ConfigureOtgAtBoot (PchPlatformPolicy);
    Status = ConfigureLpssAtBoot (PchPlatformPolicy);
    Status = ConfigureSccAtBoot (PchPlatformPolicy);
// Let PCI bus driver configure IRQs
//    Status = ConfigureIrqAtBoot (PchPlatformPolicy);
  }
  //
  // Update xHCI EHCI Mbar address to asl code.
  //
  
  PciD29F0RegBase = MmPciAddress (
                    0,
                    0,
                    PCI_DEVICE_NUMBER_PCH_USB,
                    PCI_FUNCTION_NUMBER_PCH_EHCI,
                    0
                    );
  EhciBase = MmioRead32 (PciD29F0RegBase + R_PCH_EHCI_MEM_BASE);
  if( EhciBase != 0xFFFFFFFF ){
    EhciBase &= B_PCH_EHCI_MEM_BASE_BAR;
    UpdateOperationRegionAslCode(SIGNATURE_32 ('E', 'B', 'A', 'S'), EhciBase);
  }
  
  PciD20F0RegBase = MmPciAddress (
                    0,
                    0,
                    PCI_DEVICE_NUMBER_PCH_XHCI,
                    PCI_FUNCTION_NUMBER_PCH_XHCI,
                    0
                    );
  XhciBase = MmioRead32 (PciD20F0RegBase + R_PCH_XHCI_MEM_BASE);
  if ( XhciBase != 0xFFFFFFFF ) {
    XhciBase &= B_PCH_XHCI_MEM_BASE_BA; 
    UpdateOperationRegionAslCode(SIGNATURE_32 ('X', 'B', 'A', 'S'), XhciBase);                            
  }
  return;
}

VOID
EFIAPI
PchInitBeforeBoot (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
/**

  @brief
  PCH initialization before Boot Script Table is closed

  @param[in] Event                A pointer to the Event that triggered the callback.
  @param[in] Context              A pointer to private data registered with the callback function.

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  EFI_STATUS                        Status;
  UINTN                             NumHandles;
  EFI_HANDLE                        *HandleBuffer;
  UINTN                             Index;
  DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy;
  UINT32                            RootComplexBar;
  UINT32                            FuncDisableReg;

  UINT32                            Data32Or;
  UINTN                             PciD31F3RegBase;
  UINT8                             PortIndex;
  PCH_PCIE_DEVICE_ASPM_OVERRIDE     *S3DevAspmOverrideTbl;
  UINT32                            DevAspmOverrideTblSize;
  PCH_PCI_EXPRESS_ASPM_CONTROL      AspmVal;
  UINTN                             PciD31F0RegBase;
  UINTN                             PciD19F0RegBase;
  UINT16                            AcpiBase;
  UINT32                            RegData32;
  UINT16                            GpioBase;
  UINT32                            PmcBase;
  UINT32                            SpiBase;
  UINT32                            IoBase;
  UINT32                            IlbBase;
  UINT8                             NumOfDevltrOverride;
  UINT8                             Data8Or, Data8And;
  PCH_PCIE_DEVICE_LTR_OVERRIDE      *S3DevLtrOverrideTbl;
  PCH_PCIE_DEVICE_LTR_OVERRIDE      *DevLtrOverrideTbl;
  UINT32                            DevLtrOverrideTblSize;
  VOID        						*ProtocolPointer; //AMI OVERRIDE - EIP130725 The ROM security is not actived>>
  //AMI_OVERRIDE - CSP20140401_22 Fix S3 wake issue when CRID enabled (-)>>
  // CRID support for special BYT-D sku only
  //PCH_STEPPING stepping;
  //AMI_OVERRIDE - CSP20140401_22 Fix S3 wake issue when CRID enabled (-)<<
  STATIC EFI_PCH_S3_SUPPORT_PROTOCOL        *PchS3Support;
  PchS3Support = NULL;

  S3DevLtrOverrideTbl = NULL;
  DevLtrOverrideTbl   = NULL;
  NumOfDevltrOverride = 0;

  DEBUG ((EFI_D_INFO, "PchInitBeforeBoot() Start\n"));

//AMI OVERRIDE - EIP130725 The ROM security is not actived>>
//  Status = gBS->LocateProtocol(&gExitPmAuthProtocolGuid, NULL, &ProtocolPointer);
  Status = gBS->LocateProtocol((EFI_GUID*)Context, NULL, &ProtocolPointer);
  if(EFI_ERROR(Status)) return;
//AMI OVERRIDE - EIP130725 The ROM security is not actived<<

  ///
  /// Closed the event to avoid call twice when launch shell
  ///
  gBS->CloseEvent (Event);

  ///
  /// Retrieve all instances of PCH Platform Policy protocol
  ///
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gDxePchPlatformPolicyProtocolGuid,
                  NULL,
                  &NumHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Failed to locate handle buffer for PCH Policy protocol.\n"));
    return;
  }
  ///
  /// Find the matching PCH Policy protocol
  ///
  for (Index = 0; Index < NumHandles; Index++) {
    Status = gBS->HandleProtocol (
                    HandleBuffer[Index],
                    &gDxePchPlatformPolicyProtocolGuid,
                    (VOID **) &PchPlatformPolicy
                    );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Failed to find PCH Policy protocol.\n"));
      return;
    }
    PciD31F3RegBase = MmPciAddress (0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_SMBUS,
                        PCI_FUNCTION_NUMBER_PCH_SMBUS,
                        0
                      );
    PciD31F0RegBase = MmPciAddress (0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_LPC,
                        PCI_FUNCTION_NUMBER_PCH_LPC,
                        0
                      );
    PciD19F0RegBase = MmPciAddress (0,
                        PchPlatformPolicy->BusNumber,
                        PCI_DEVICE_NUMBER_PCH_SATA,
                        PCI_FUNCTION_NUMBER_PCH_SATA,
                        0
                      );

    RootComplexBar = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_RCBA) & B_PCH_LPC_RCBA_BAR;
    AcpiBase       = MmioRead16 (PciD31F0RegBase + R_PCH_LPC_ACPI_BASE) & B_PCH_LPC_ACPI_BASE_BAR;
    PmcBase        = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_PMC_BASE) & B_PCH_LPC_PMC_BASE_BAR;
    SpiBase        = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_SPI_BASE) & B_PCH_LPC_SPI_BASE_BAR;
    FuncDisableReg = MmioRead32 (PmcBase + R_PCH_PMC_FUNC_DIS);
    GpioBase       = MmioRead16 (PciD31F0RegBase + R_PCH_LPC_GPIO_BASE) & B_PCH_LPC_GPIO_BASE_BAR;
    IoBase         = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_IO_BASE) & B_PCH_LPC_IO_BASE_BAR;
    IlbBase        = MmioRead32 (PciD31F0RegBase + R_PCH_LPC_ILB_BASE) & B_PCH_LPC_ILB_BASE_BAR;

    Status         = PciERWORegInit (PchPlatformPolicy, RootComplexBar, &FuncDisableReg);
    ASSERT_EFI_ERROR (Status);
    ///
    /// Do the Pcie ASPM enable prior to the end of POST
    /// PCH BIOS Spec Rev 0.6.0, Section 8.3.1 ASPM and the PCI Express* Root Ports
    ///
    ///
    /// Allcoate and Copy the entire Aspm override table pointed by DevAspmOverride to < 4G EfiReservedMemory
    /// It's for S3 resume used.
    ///
    DevAspmOverrideTblSize = PchPlatformPolicy->PciExpressConfig->NumOfDevAspmOverride *
                             sizeof (PCH_PCIE_DEVICE_ASPM_OVERRIDE);
    S3DevAspmOverrideTbl = AllocateReservedCopyPool (
                             DevAspmOverrideTblSize,
                             PchPlatformPolicy->PciExpressConfig->DevAspmOverride
                             );
    ASSERT_EFI_ERROR (S3DevAspmOverrideTbl != NULL);

    ///
    /// Allcoate and Copy the entire LTR override table pointed by DevLtrOverride to < 4G EfiReservedMemory
    /// It's used for S3 resume used.
    ///
    if (PchPlatformPolicy->Revision >= DXE_PCH_PLATFORM_POLICY_PROTOCOL_REVISION_1) {
      DevLtrOverrideTbl   = PchPlatformPolicy->PwrOptConfig->DevLtrOverride;
      NumOfDevltrOverride = PchPlatformPolicy->PwrOptConfig->NumOfDevLtrOverride;
      if ((DevLtrOverrideTbl != NULL) && (NumOfDevltrOverride != 0)) {
        DevLtrOverrideTblSize = NumOfDevltrOverride * sizeof (PCH_PCIE_DEVICE_LTR_OVERRIDE);
        S3DevLtrOverrideTbl   = AllocateReservedCopyPool (
                                  DevLtrOverrideTblSize,
                                  DevLtrOverrideTbl
                                  );
        ASSERT_EFI_ERROR (S3DevLtrOverrideTbl != NULL);
      }
    }
    for (PortIndex = 0; PortIndex < PCH_PCIE_MAX_ROOT_PORTS; PortIndex++) {
      if ((FuncDisableReg & (B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0 << PortIndex)) == 0) {
        AspmVal = PchPlatformPolicy->PciExpressConfig->RootPort[PortIndex].Aspm;
        Status = PcieSetPm (
                   PchPlatformPolicy->BusNumber,
                   PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
                   PchPlatformPolicy->PciExpressConfig->RootPort[PortIndex].FunctionNumber,
                   AspmVal,
                   PchPlatformPolicy->PciExpressConfig->NumOfDevAspmOverride,
                   PchPlatformPolicy->PciExpressConfig->DevAspmOverride,
                   PchPlatformPolicy->PciExpressConfig->TempRootPortBusNumMin,
                   PchPlatformPolicy->PciExpressConfig->TempRootPortBusNumMax,
                   NumOfDevltrOverride,
                   DevLtrOverrideTbl
                   );
        Status = SetPciePmS3Item (
                   PchPlatformPolicy->BusNumber,
                   PCI_DEVICE_NUMBER_PCH_PCIE_ROOT_PORTS,
                   PchPlatformPolicy->PciExpressConfig->RootPort[PortIndex].FunctionNumber,
                   AspmVal,
                   PchPlatformPolicy->PciExpressConfig->NumOfDevAspmOverride,
                   S3DevAspmOverrideTbl,
                   PchPlatformPolicy->PciExpressConfig->TempRootPortBusNumMin,
                   PchPlatformPolicy->PciExpressConfig->TempRootPortBusNumMax,
                   NumOfDevltrOverride,
                   S3DevLtrOverrideTbl
                   );
      }
    }

    ///
    /// SPI Flash Programming Guide Section 5.5.2 Vendor Component Lock
    /// It is strongly recommended that BIOS sets the Vendor Component Lock (VCL) bits. VCL applies
    /// the lock to both LVSCC and UVSCC even if LVSCC is not used. Without the VCL bits set, it is
    ///
    MmioOr32 ((UINTN) (SpiBase + R_PCH_SPI_LVSCC), B_PCH_SPI_LVSCC_VCL);
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (SpiBase + R_PCH_SPI_LVSCC),
      1,
      (VOID *) (UINTN) (SpiBase + R_PCH_SPI_LVSCC)
      );

    ///
    /// VLV BIOS Spec , Section 18.5 Additional Power Management Programming
    /// Step 2
    /// Lock "0xCF9 Global Reset" field, PBASE + 0x48 [31] = 1h.
    ///
    /// If SeC is in manufact mode, should not lock the GRST. Move this code to Heci driver.
    ///
    ///    MmioOr32 (
    ///      (UINTN) (PmcBase + R_PCH_PMC_PMIR),
    ///       (UINT32) (B_PCH_PMC_PMIR_CF9LOCK)
    ///         );
    ///     S3BootScriptSaveMemWrite (
    ///       EfiBootScriptWidthUint32,
    ///       (UINTN) (PmcBase + R_PCH_PMC_PMIR),
    ///       1,
    ///       (VOID *) (UINTN) (PmcBase + R_PCH_PMC_PMIR)
    ///         );

    ///
    /// VLV BIOS Spec , Section 18.5 Additional Power Management Programming
    /// Step 3
    /// Set "SLP_S3 / SLP_S4 Stretching Policy Lock-Down" bit, PBASE + 0x24 [18] = 1b.
    ///
    MmioOr32 (
      (UINTN) (PmcBase + R_PCH_PMC_GEN_PMCON_2),
      (UINT32) (B_PCH_PMC_GEN_PMCON_LOCK_S4_STRET_LD)
      );
    S3BootScriptSaveMemWrite (
      EfiBootScriptWidthUint32,
      (UINTN) (PmcBase + R_PCH_PMC_GEN_PMCON_2),
      1,
      (VOID *) (UINTN) (PmcBase + R_PCH_PMC_GEN_PMCON_2)
      );

    ///
    /// VLV BIOS Spec Rev, Section 6.5.1 Flash Security Recommendation
    /// Step 1
    /// Intel strongly recommends that BIOS enables the BIOS Lock Enable (BLE) feature of the PCH.
    /// Left to platform code to register an callback function to handle IchnBiosWp SMI
    ///
    /// Step 2
    /// VLV BIOS Spec, Section 6.5.1
    /// It is recommended to use SMI_LOCK bit (PBASE + 0x24 [4] = 1)
    /// to protect SMI sensitive configuration bits from been changed by malicious code.
    /// When the SMI_LOCK bit is set, writes to the GBL_SMI_EN bit (ABASE + 0x30 [0])
    /// for enabling and disabling SMI generation have no effect.
    ///
    if (PchPlatformPolicy->LockDownConfig->GlobalSmi == PCH_DEVICE_ENABLE) {
      //
      // Save Global SMI Enable bit setting before BIOS enables SMI_LOCK during S3 resume
      //
      RegData32 = IoRead32 ((UINTN)(AcpiBase + R_PCH_SMI_EN));
      RegData32 &= (UINT32) ~(B_PCH_SMI_EN_SWSMI_TMR); //AMI OVERRIDE - EIP128872 Fix of S3 Resume hang at 0xE1 issue.
      S3BootScriptSaveIoWrite (   
        EfiBootScriptWidthUint32,
        (UINTN) (AcpiBase + R_PCH_SMI_EN),
        1,
        &RegData32
        );

      MmioOr8 ((UINTN) (PmcBase + R_PCH_PMC_GEN_PMCON_2), B_PCH_PMC_GEN_PMCON_SMI_LOCK);
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint8,
        (UINTN) (PmcBase + R_PCH_PMC_GEN_PMCON_2),
        1,
        (VOID *) (UINTN) (PmcBase + R_PCH_PMC_GEN_PMCON_2)
        );
    }

    ///
    /// Step 3
    /// BIOS also needs to set the BIOS Interface Lock Down bit
    /// (RCBA + 0x00 [0] = 1’b1 in General Control and Status - BILD).
    /// Setting this bit will prevent writes to the Top Swap bit (GCS.TS, RCBA + 0x00 [1])
    /// and the Boot BIOS Straps (GCS.BBS RCBA + 0x00 [11:10]).
    /// Enabling this bit will mitigate malicious software attempts
    /// to replace the system BIOS option ROM with its own code.
    ///
    if (PchPlatformPolicy->LockDownConfig->BiosInterface == PCH_DEVICE_ENABLE) {
      MmioOr8 ((UINTN) (RootComplexBar + R_PCH_RCRB_GCS), B_PCH_RCRB_GCS_BILD);
      MmioRead8 ((UINTN) (RootComplexBar + R_PCH_RCRB_GCS)); // Read Posted Writes Register
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint8,
        (UINTN) (RootComplexBar + R_PCH_RCRB_GCS),
        1,
        (VOID *) (UINTN) (RootComplexBar + R_PCH_RCRB_GCS)
        );
    }

    ///
    /// VLV BIOS Spec, Section 19.8.4 Additional Programming
    /// Set IBASE + Offset 0x64 [1:0] = 11b
    ///
    if (PchPlatformPolicy->LockDownConfig->RtcLock == PCH_DEVICE_ENABLE) {
      MmioOr32 (
        (UINTN) (IlbBase + R_PCH_ILB_RTC_CONF),
        (UINT32) (B_PCH_ILB_RTC_CONF_UCMOS_LOCK | B_PCH_ILB_RTC_CONF_LCMOS_LOCK)
        );
      S3BootScriptSaveMemWrite (
        EfiBootScriptWidthUint32,
        (UINTN) (IlbBase + R_PCH_ILB_RTC_CONF),
        1,
        (VOID *) (UINTN) (IlbBase + R_PCH_ILB_RTC_CONF)
        );
    }

    ///
    /// VLV BIOS Spec, Section 3.6 Flash Security Recommendation
    /// Step 1
    /// Intel strongly recommends that BIOS enables the BIOS Lock Enable (BLE) feature.
    ///
    /// Generate PchBiosLock SW SMI to register IchnBiosWp callback function in
    /// PchBiosLockSwSmiCallback() to handle TCO BIOSWR SMI
    ///
    if (PchPlatformPolicy->LockDownConfig->BiosLock == PCH_DEVICE_ENABLE) {
      IoWrite8 (R_PCH_APM_CNT, PchPlatformPolicy->LockDownConfig->PchBiosLockSwSmiNumber);
      
	  //AMI OVERRIDE - EIP130725 The ROM security is not actived>>
      S3BootScriptSaveMemWrite (   
        EfiBootScriptWidthUint8,
        (UINTN) (SpiBase + R_PCH_SPI_SCS),
        1,
        (VOID *) (UINTN) (SpiBase + R_PCH_SPI_SCS)
        );
      //AMI OVERRIDE - EIP130725 The ROM security is not actived<<
	  
      S3BootScriptSaveMemWrite (   
        EfiBootScriptWidthUint8,
        (UINTN) (SpiBase + R_PCH_SPI_BCR),
        1,
        (VOID *) (UINTN) (SpiBase + R_PCH_SPI_BCR)
        );
      Data8Or = B_PCH_SPI_BCR_BLE;
      Data8And = B_PCH_SPI_BCR_SMM_BWP;
      S3BootScriptSaveMemReadWrite (
        EfiBootScriptWidthUint8,
        (UINTN) (SpiBase + R_PCH_SPI_BCR),
        &Data8Or,
        &Data8And
        );
    }
  //AMI_OVERRIDE - EIP146629 move CF9 lock to SBdxe.c readytoboot event >>
  ///
  /// Set CF9Lock
  ///
  /*
  MmioOr32 ((UINTN) (PmcBase + R_PCH_PMC_PMIR), B_PCH_PMC_PMIR_CF9LOCK);

  S3BootScriptSaveMemWrite (
    EfiBootScriptWidthUint32,
    (UINTN) (PmcBase + R_PCH_PMC_PMIR),
    1,
    (VOID *) (UINTN) (PmcBase + R_PCH_PMC_PMIR)
  );
  */
  //AMI_OVERRIDE - EIP146629 move CF9 lock to SBdxe.c readytoboot event <<
  
        
    ///
    /// Check to disable Smbus controller
    ///
    if (PchPlatformPolicy->DeviceEnabling->Smbus == PCH_DEVICE_DISABLE) {
      DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Putting SMBus into D3 Hot State.\n"));
      MmioOr32 ((UINTN) (PciD31F3RegBase + R_PCH_SMBUS_PMCSR), B_PCH_SMBUS_PMCSR_PS);
      MmioOr32 ((UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS2), (UINT32) B_PCH_PMC_FUNC_DIS2_SMBUS);
      S3BootScriptSaveMemWrite(

        EfiBootScriptWidthUint32,
        (UINTN) (PciD31F3RegBase + R_PCH_SMBUS_PMCSR),
        1,
        (VOID *) (UINTN) (PciD31F3RegBase + R_PCH_SMBUS_PMCSR)
        );
      S3BootScriptSaveMemWrite(
        EfiBootScriptWidthUint32,
        (UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS2),
        1,
        (VOID *) (UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS2)
        );
      ///
      /// Reads back for posted write to take effect
      ///
      Data32Or = MmioRead32 ((UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS2));
      S3BootScriptSaveMemPoll(
        EfiBootScriptWidthUint32,
        (UINTN) (PmcBase + R_PCH_PMC_FUNC_DIS2),
        &Data32Or,  /// BitMask
        &Data32Or,  /// BitValue
        1,          /// Duration
        1           /// LoopTimes
        );
    }
    UsbInitBeforeBoot (PchPlatformPolicy);

    //EIP227838 >>
        DEBUG ((EFI_D_ERROR | EFI_D_INFO, "Flash Configuration Lockdown.\n"));
        MmioOr16 ((UINTN) (SpiBase + R_PCH_SPI_HSFS), B_PCH_SPI_HSFS_FLOCKDN);
        S3BootScriptSaveMemWrite (
    		    EfiBootScriptWidthUint16,
    		    (UINTN) (SpiBase + R_PCH_SPI_HSFS),
    		    1,
    		    (VOID *) (UINTN) (SpiBase + R_PCH_SPI_HSFS)
    		    );
    //EIP227838 <<

    //AMI_OVERRIDE - CSP20140401_22 Fix S3 wake issue when CRID enabled (-)>>
    // CRID support for special BYT-D sku only
    //if (PchPlatformPolicy->DeviceEnabling->Crid == PCH_DEVICE_ENABLE) {
    //  stepping = PchStepping();
    //  if (stepping >= PchC0) {
    //    MmioOr32 ((UINTN) (PmcBase + R_PCH_PMC_CRID), (UINT32) 0x01);

    //    Data32Or = (UINT32) 0x01;
    //    S3BootScriptSaveMemWrite(
    //      EfiBootScriptWidthUint32,
    //      (UINTN) (PmcBase + R_PCH_PMC_CRID),
    //      1,
    //      &Data32Or
    //      );
    //  	}
    //}
    //AMI_OVERRIDE - CSP20140401_22 Fix S3 wake issue when CRID enabled (-)>>
  }

  if (!PchS3Support) {
     DEBUG ((EFI_D_INFO, "Locating the S3 Support Protocol - PCH Init before Boot\n"));

    ///
    /// Get the PCH S3 Support Protocol
    ///
    Status = gBS->LocateProtocol (
                    &gEfiPchS3SupportProtocolGuid,
                    NULL,
                    (VOID **) &PchS3Support
                    );
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) {
      return;
    }

    Status = PchS3Support->ReadyToLock(PchS3Support);
    if (EFI_ERROR (Status)) {
      return;
    }

  }
  DEBUG ((EFI_D_INFO, "PchInitBeforeBoot() End\n"));

  return;
}
EFI_STATUS
PmTimerStallS3Item (
  IN   UINT32                    DelayTime
  )
/*++

  Routine Description:

    Set a "PmTimer Stall" S3 dispatch item

  Arguments:

    DelayTime         The request number of microseconds

  Returns:

    EFI_SUCCESS       The function completed successfully

--*/
{
  EFI_STATUS                                  Status;
#ifndef ECP_FLAG
  EFI_BOOT_SCRIPT_SAVE_PROTOCOL               *mBootScriptSave;
#endif
  STATIC EFI_PCH_S3_PARAMETER_PM_TIMER_STALL  S3ParameterPmTimerStall;
  STATIC EFI_PCH_S3_DISPATCH_ITEM             S3DispatchItem = {
    PchS3ItemTypePmTimerStall,
    &S3ParameterPmTimerStall
  };
  EFI_PHYSICAL_ADDRESS                    S3DispatchEntryPoint;

  if (!mPchS3Support) {
    //
    // Get the PCH S3 Support Protocol
    //
    Status = gBS->LocateProtocol (
                    &gEfiPchS3SupportProtocolGuid,
                    NULL,
                    (VOID **) &mPchS3Support
                    );
    ASSERT_EFI_ERROR (Status);

    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  S3ParameterPmTimerStall.DelayTime = DelayTime;
  Status = mPchS3Support->SetDispatchItem (
                            mPchS3Support,
                            &S3DispatchItem,
                            &S3DispatchEntryPoint
                            );
  ASSERT_EFI_ERROR (Status);

  //
  // Save the script dispatch item in the Boot Script
  //
#ifdef ECP_FLAG
  S3BootScriptSaveDispatch(S3DispatchEntryPoint);
#else
  //S3BootScriptSaveDispatch((VOID *)(UINTN) S3DispatchEntryPoint);
  Status = gBS->LocateProtocol (
                  &gEfiBootScriptSaveProtocolGuid,
                  NULL,
                  (VOID **) &mBootScriptSave
                  );

  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    0,
                    EFI_BOOT_SCRIPT_DISPATCH_OPCODE,
                    S3DispatchEntryPoint
                    );
#endif
  return Status;
}
