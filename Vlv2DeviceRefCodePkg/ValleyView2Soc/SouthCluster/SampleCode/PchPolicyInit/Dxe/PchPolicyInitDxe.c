/**
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
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
  PchPolicyInitDxe.c

  @brief
  This file is SampleCode for Intel PCH DXE Platform Policy initialization.

--*/

#include "PchPolicyInitDxe.h"
#ifdef ECP_FLAG
EFI_GUID gDxePchPlatformPolicyProtocolGuid = DXE_PCH_PLATFORM_POLICY_PROTOCOL_GUID;
#else
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Protocol/PchPlatformPolicy.h>
#include <IndustryStandard/Pci22.h>
#endif

#define SW_SMI_BIOS_LOCK              0xA9
#define PCI_CLASS_NETWORK             0x02
#define PCI_CLASS_NETWORK_ETHERNET    0x00
#define PCI_CLASS_NETWORK_OTHER       0x80

DXE_PCH_PLATFORM_POLICY_PROTOCOL  mPchPolicyData        = { 0 };
PCH_DEVICE_ENABLING               mPchDeviceEnabling    = { 0 };
PCH_USB_CONFIG                    mPchUsbConfig         = { 0 };
PCH_PCI_EXPRESS_CONFIG            mPchPciExpressConfig  = { 0 };
PCH_SATA_CONFIG                   mPchSataConfig        = { 0 };
PCH_AZALIA_CONFIG                 mPchAzaliaConfig      = { 0 };
PCH_SMBUS_CONFIG                  mPchSmbusConfig       = { 0 };
PCH_MISC_PM_CONFIG                mPchMiscPmConfig      = { 0 };
PCH_DEFAULT_SVID_SID              mPchDefaultSvidSid    = { 0 };
PCH_LOCK_DOWN_CONFIG              mPchLockDownConfig    = { 0 };
PCH_LPC_SIRQ_CONFIG               mSerialIrqConfig      = { 0 };
PCH_PWR_OPT_CONFIG                mPchPwrOptConfig      = { 0 };
PCH_LPSS_CONFIG                   mLpssConfig           = { 0 };
PCH_SCC_CONFIG                    mSccConfig            = { 0 };

UINT8 mSmbusRsvdAddresses[4] = {
  0xA0,
  0xA2,
  0xA4,
  0xA6
};
PCH_PCIE_DEVICE_ASPM_OVERRIDE mDevAspmOverride[] = {
  ///
  /// TI XIO2200A PCI Express to PCI Bus Translation Bridge with 1394a OHCI and Two-Port PHY
  ///
  {0x104C, 0x8231, 0xff, 0xff, 0xff, PchPcieAspmL1},
  ///
  /// All Intel Ethernet LAN controller
  ///
  {0x8086, 0xffff, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_ETHERNET, PchPcieAspmL1},
  ///
  /// Intel PRO/Wireless
  ///
  {0x8086, 0x422b, 0xff, 0xff, 0xff, PchPcieAspmL1},
  {0x8086, 0x422c, 0xff, 0xff, 0xff, PchPcieAspmL1},
  {0x8086, 0x4238, 0xff, 0xff, 0xff, PchPcieAspmL1},
  {0x8086, 0x4239, 0xff, 0xff, 0xff, PchPcieAspmL1},
  ///
  /// Intel WiMAX/WiFi Link
  ///
  {0x8086, 0x0082, 0xff, 0xff, 0xff, PchPcieAspmL1},
  {0x8086, 0x0085, 0xff, 0xff, 0xff, PchPcieAspmL1},
  {0x8086, 0x0083, 0xff, 0xff, 0xff, PchPcieAspmL1},
  {0x8086, 0x0084, 0xff, 0xff, 0xff, PchPcieAspmL1},
  {0x8086, 0x0086, 0xff, 0xff, 0xff, PchPcieAspmL1},
  {0x8086, 0x0087, 0xff, 0xff, 0xff, PchPcieAspmL1},
  {0x8086, 0x0088, 0xff, 0xff, 0xff, PchPcieAspmL1},
  {0x8086, 0x0089, 0xff, 0xff, 0xff, PchPcieAspmL1},
  {0x8086, 0x008F, 0xff, 0xff, 0xff, PchPcieAspmL1},
  {0x8086, 0x0090, 0xff, 0xff, 0xff, PchPcieAspmL1},
  ///
  /// Intel Crane Peak WLAN NIC
  ///
  {0x8086, 0x08AE, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  {0x8086, 0x08AF, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  ///
  /// Intel Crane Peak w/BT WLAN NIC
  ///
  {0x8086, 0x0896, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  {0x8086, 0x0897, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  ///
  /// Intel Kelsey Peak WiFi, WiMax
  ///
  {0x8086, 0x0885, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  {0x8086, 0x0886, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  ///
  /// Intel Centrino Wireless-N 105
  ///
  {0x8086, 0x0894, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  {0x8086, 0x0895, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  ///
  /// Intel Centrino Wireless-N 135
  ///
  {0x8086, 0x0892, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  {0x8086, 0x0893, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  ///
  /// Intel Centrino Wireless-N 2200
  ///
  {0x8086, 0x0890, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  {0x8086, 0x0891, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  ///
  /// Intel Centrino Wireless-N 2230
  ///
  {0x8086, 0x0887, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  {0x8086, 0x0888, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  ///
  /// Intel Centrino Wireless-N 6235
  ///
  {0x8086, 0x088E, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  {0x8086, 0x088F, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  ///
  /// Intel WilkinsPeak 1 Wifi
  ///
  {0x8086, 0x08B3, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  {0x8086, 0x08B4, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  ///
  /// Intel WilkinsPeak 2 Wifi
  ///
  {0x8086, 0x08B1, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  {0x8086, 0x08B2, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  ///
  /// Intel WilkinsPeak PF
  ///
  {0x8086, 0x08B0, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1},
  {0x168C, 0x0032, 0xff, 0xff, 0xff, PchPcieAspmDisabled}
};
///
/// Function implementations
///
EFI_STATUS
EFIAPI
PchPolicyInitDxeEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
/*++

  @brief
  Initilize Intel PCH DXE Platform Policy

  @param[in] ImageHandle          Image handle of this driver.
  @param[in] SystemTable          Global system service table.

  @retval EFI_SUCCESS             Initialization complete.
  @exception EFI_UNSUPPORTED      The chipset is unsupported by this driver.
  @retval EFI_OUT_OF_RESOURCES    Do not have enough resources to initialize the driver.
  @retval EFI_DEVICE_ERROR        Device error, driver exits abnormally.

**/
{
  EFI_HANDLE  Handle;
  EFI_STATUS  Status;
  UINT8       PortIndex;

  ///
  /// General initialization
  ///
  mPchPolicyData.Revision             = DXE_PCH_PLATFORM_POLICY_PROTOCOL_REVISION_1;
  mPchPolicyData.BusNumber            = 0;

  mPchPolicyData.DeviceEnabling       = &mPchDeviceEnabling;
  mPchPolicyData.UsbConfig            = &mPchUsbConfig;
  mPchPolicyData.PciExpressConfig     = &mPchPciExpressConfig;
  mPchPolicyData.SataConfig           = &mPchSataConfig;
  mPchPolicyData.AzaliaConfig         = &mPchAzaliaConfig;
  mPchPolicyData.SmbusConfig          = &mPchSmbusConfig;

  mPchPolicyData.MiscPmConfig         = &mPchMiscPmConfig;
  mPchPolicyData.DefaultSvidSid       = &mPchDefaultSvidSid;
  mPchPolicyData.LockDownConfig       = &mPchLockDownConfig;
  mPchPolicyData.SerialIrqConfig      = &mSerialIrqConfig;
  mPchPolicyData.PwrOptConfig         = &mPchPwrOptConfig;
  mPchPolicyData.LpssConfig           = &mLpssConfig;
  mPchPolicyData.SccConfig            = &mSccConfig;

  ///
  /// VLV BIOS Spec Rev 0.5 Section 26.8 Flash Security Recommendations,
  /// Intel strongly recommends that BIOS sets the BIOS Interface Lock Down bit. Enabling this bit
  /// will mitigate malicious software attempts to replace the system BIOS option ROM with its own code.
  /// We always enable this as a platform policy.
  ///
  mPchLockDownConfig.BiosInterface          = PCH_DEVICE_ENABLE;
  mPchLockDownConfig.GlobalSmi              = PCH_DEVICE_ENABLE;
  mPchLockDownConfig.RtcLock                = PCH_DEVICE_ENABLE;
  ///
  /// While BiosLock is enabled, BIOS can only be modified from SMM after ExitPmAuth.
  ///
  mPchLockDownConfig.BiosLock               = PCH_DEVICE_DISABLE;
  mPchLockDownConfig.PchBiosLockSwSmiNumber = SW_SMI_BIOS_LOCK;
  ///
  /// Initialize policy to default values when variable isn't found.
  ///
  mPchDeviceEnabling.Azalia             = 2;
  mPchDeviceEnabling.Sata               = PCH_DEVICE_ENABLE;
  mPchDeviceEnabling.Smbus              = PCH_DEVICE_ENABLE;
  mPchDeviceEnabling.LpeEnabled         = PCH_DEVICE_DISABLE;

  mPchUsbConfig.PortSettings[0].Enable  = PCH_DEVICE_ENABLE;
  mPchUsbConfig.PortSettings[1].Enable  = PCH_DEVICE_ENABLE;
  mPchUsbConfig.PortSettings[2].Enable  = PCH_DEVICE_ENABLE;
  mPchUsbConfig.PortSettings[3].Enable  = PCH_DEVICE_ENABLE;

  // CRID support for special BYT-D sku only
  mPchDeviceEnabling.Crid               = PCH_DEVICE_DISABLE;
  ///
  /// The following setting is only available for Desktop
  /// Please program it per the layout of each port on the platform board.
  ///
  mPchUsbConfig.PortSettings[0].Panel   = PCH_USB_FRONT_PANEL;
  mPchUsbConfig.PortSettings[1].Panel   = PCH_USB_FRONT_PANEL;
  mPchUsbConfig.PortSettings[2].Panel   = PCH_USB_BACK_PANEL;
  mPchUsbConfig.PortSettings[3].Panel   = PCH_USB_BACK_PANEL;

  ///
  /// The following setting is only available for Mobile
  /// Please program it per the layout of each port on the platform board.
  ///
  mPchUsbConfig.PortSettings[0].Dock          = PCH_DEVICE_DISABLE;
  mPchUsbConfig.PortSettings[1].Dock          = PCH_DEVICE_DISABLE;
  mPchUsbConfig.PortSettings[2].Dock          = PCH_DEVICE_DISABLE;
  mPchUsbConfig.PortSettings[3].Dock          = PCH_DEVICE_DISABLE;

  mPchUsbConfig.Usb20Settings[0].Enable = PCH_DEVICE_ENABLE;

  mPchUsbConfig.Usb30Settings.PreBootSupport  = PCH_DEVICE_DISABLE;
  ///
  /// VLV BIOS Spec Rev 0.5 Section 32.1 xHCI controller options in Reference Code
  /// Please refer to Table 13-1 in PCH BIOS Spec for USB Port Operation with no xHCI
  /// pre-boot software.
  /// Please refer to Table 13-2 in PCH BIOS Spec for USB Port Operation with xHCI
  /// pre-boot software.
  ///
  /// The xHCI modes that available in BIOS are:
  /// Disabled   - forces only USB 2.0 to be supported in the OS. The xHCI controller is turned off
  ///              and hidden from the PCI space.
  /// Enabled    - allows USB 3.0 to be supported in the OS. The xHCI controller is turned on. The
  ///              shareable ports are routed to the xHCI controller. OS needs to provide drivers
  ///              to support USB 3.0.
  /// Auto       - This mode uses ACPI protocol to provide an option that enables the xHCI controller
  ///              and reroute USB ports via the _OSC ACPI method call. Note, this mode switch requires
  ///              special OS driver support for USB 3.0.
  /// Smart Auto - This mode is similar to Auto, but it adds the capability to route the ports to xHCI
  ///              or EHCI according to setting used in previous boots (for non-G3 boot) in the pre-boot
  ///              environment. This allows the use of USB 3.0 devices prior to OS boot. Note, this mode
  ///              switch requires special OS driver support for USB 3.0 and USB 3.0 software available
  ///              in the pre-boot enviroment.
  /// Recommendations:
  ///  - If BIOS supports xHCI pre-boot driver then use Smart Auto mode as default
  ///  - If BIOS does not support xHCI pre-boot driver then use AUTO mode as default
  ///
  if (mPchUsbConfig.Usb30Settings.PreBootSupport == PCH_DEVICE_ENABLE) {
    mPchUsbConfig.Usb30Settings.Mode = PCH_XHCI_MODE_SMARTAUTO;
  } else {
    mPchUsbConfig.Usb30Settings.Mode = PCH_XHCI_MODE_AUTO;
  }

  mPchUsbConfig.UsbOtgSettings.Enable = PCH_DEVICE_DISABLE;

  mPchUsbConfig.Usb20OverCurrentPins[0]   = 0;
  mPchUsbConfig.Usb20OverCurrentPins[1]   = 0;
  mPchUsbConfig.Usb20OverCurrentPins[2]   = 1;
  mPchUsbConfig.Usb20OverCurrentPins[3]   = 1;

  mPchUsbConfig.Usb30OverCurrentPins[0]   = 0;

  mPchUsbConfig.Usb20PortLength[0]        = 0x32;
  mPchUsbConfig.Usb20PortLength[1]        = 0x32;
  mPchUsbConfig.Usb20PortLength[2]        = 0x110;
  mPchUsbConfig.Usb20PortLength[3]        = 0x112;

  mPchUsbConfig.UsbPerPortCtl           = PCH_DEVICE_DISABLE;
  mPchUsbConfig.Ehci1Usbr               = PCH_DEVICE_DISABLE;
  mPchPolicyData.XhciWorkaroundSwSmiNumber = 0xF3;
  mPchPciExpressConfig.PcieDynamicGating      = PCH_DEVICE_DISABLE;

  ///
  /// PCI Express related settings from setup variable
  ///
  mPchPciExpressConfig.RootPortClockGating    = PCH_DEVICE_ENABLE;
  mPchPciExpressConfig.TempRootPortBusNumMin  = 0x5;
  mPchPciExpressConfig.TempRootPortBusNumMax  = 0x7;

  for (PortIndex = 0; PortIndex < PCH_PCIE_MAX_ROOT_PORTS; PortIndex++) {
    mPchPciExpressConfig.RootPort[PortIndex].Enable             = PCH_DEVICE_ENABLE;
    mPchPciExpressConfig.RootPort[PortIndex].FunctionNumber     = PortIndex;
    mPchPciExpressConfig.RootPort[PortIndex].PhysicalSlotNumber = PortIndex;
    mPchPciExpressConfig.RootPort[PortIndex].Aspm               = 4;
    mPchPciExpressConfig.RootPort[PortIndex].SlotImplemented    = 1;
  }

  mPchPciExpressConfig.NumOfDevAspmOverride = sizeof (mDevAspmOverride) / sizeof (PCH_PCIE_DEVICE_ASPM_OVERRIDE);
  mPchPciExpressConfig.DevAspmOverride      = mDevAspmOverride;
  mPchPwrOptConfig.NumOfDevLtrOverride      = 0;
  mPchPwrOptConfig.DevLtrOverride           = NULL;

  for (PortIndex = 0; PortIndex < PCH_AHCI_MAX_PORTS; PortIndex++) {
    mPchSataConfig.PortSettings[PortIndex].Enable           = PCH_DEVICE_ENABLE;
    mPchSataConfig.PortSettings[PortIndex].HotPlug          = PCH_DEVICE_DISABLE;
    mPchSataConfig.PortSettings[PortIndex].MechSw           = PCH_DEVICE_DISABLE;
    mPchSataConfig.PortSettings[PortIndex].External         = PCH_DEVICE_DISABLE;
  }

  mPchSataConfig.RaidAlternateId  = PCH_DEVICE_DISABLE;
  mPchSataConfig.Raid0            = PCH_DEVICE_ENABLE;
  mPchSataConfig.Raid1            = PCH_DEVICE_ENABLE;
  mPchSataConfig.Raid10           = PCH_DEVICE_ENABLE;
  mPchSataConfig.Raid5            = PCH_DEVICE_ENABLE;
  mPchSataConfig.Irrt             = PCH_DEVICE_ENABLE;
  mPchSataConfig.OromUiBanner     = PCH_DEVICE_ENABLE;
  mPchSataConfig.HddUnlock        = PCH_DEVICE_ENABLE;
  mPchSataConfig.LedLocate        = PCH_DEVICE_ENABLE;
  mPchSataConfig.IrrtOnly         = PCH_DEVICE_ENABLE;
  mPchSataConfig.SalpSupport      = PCH_DEVICE_ENABLE;
  mPchSataConfig.LegacyMode       = PCH_DEVICE_ENABLE;
  mPchSataConfig.SpeedSupport     = PchSataSpeedSupportGen2;
  ///
  /// AzaliaConfig
  ///
  mPchAzaliaConfig.Pme            = PCH_DEVICE_DISABLE;
  mPchAzaliaConfig.ResetWaitTimer = 300;
  mPchAzaliaConfig.AzaliaVCi      = PCH_DEVICE_ENABLE;
  ///
  /// Reserved SMBus Address
  ///
  mPchSmbusConfig.NumRsvdSmbusAddresses = 4;
  mPchSmbusConfig.RsvdSmbusAddressTable = mSmbusRsvdAddresses;

  ///
  /// MiscPm Configuration
  ///
  mPchMiscPmConfig.WakeConfig.WolEnableOverride         = PCH_DEVICE_DISABLE;

  mPchMiscPmConfig.PchSlpS3MinAssert                    = PchSlpS350ms;
  mPchMiscPmConfig.PchSlpS4MinAssert                    = PchSlpS44s;
  mPchMiscPmConfig.SlpStrchSusUp                        = PCH_DEVICE_DISABLE;
  mPchMiscPmConfig.SlpLanLowDc                          = PCH_DEVICE_DISABLE;
  mPchAzaliaConfig.DA = 0;

  ///
  /// Initialize Serial IRQ Config
  ///
  mSerialIrqConfig.SirqEnable       = TRUE;
  mSerialIrqConfig.SirqMode         = PchContinuousMode;

  ///
  /// Initialize LPIO Config
  ///
  mLpssConfig.Dma1Enabled    = PCH_DEVICE_DISABLE;
  mLpssConfig.I2C0Enabled    = PCH_DEVICE_DISABLE;
  mLpssConfig.I2C1Enabled    = PCH_DEVICE_DISABLE;
  mLpssConfig.I2C2Enabled    = PCH_DEVICE_DISABLE;
  mLpssConfig.I2C3Enabled    = PCH_DEVICE_DISABLE;
  mLpssConfig.I2C4Enabled    = PCH_DEVICE_DISABLE;
  mLpssConfig.I2C5Enabled    = PCH_DEVICE_DISABLE;
  mLpssConfig.I2C6Enabled    = PCH_DEVICE_DISABLE;

  mLpssConfig.Dma0Enabled    = PCH_DEVICE_DISABLE;
  mLpssConfig.Pwm0Enabled    = PCH_DEVICE_DISABLE;
  mLpssConfig.Pwm1Enabled    = PCH_DEVICE_DISABLE;
  mLpssConfig.Hsuart0Enabled = PCH_DEVICE_DISABLE;
  mLpssConfig.Hsuart1Enabled = PCH_DEVICE_DISABLE;
  mLpssConfig.SpiEnabled     = PCH_DEVICE_DISABLE;
  ///
  /// Initialize SCC Config
  ///

  mSccConfig.HsiEnabled    = PCH_DEVICE_ENABLE;
  mSccConfig.SdcardEnabled = PCH_DEVICE_ENABLE;
  mSccConfig.SdioEnabled   = PCH_DEVICE_ENABLE;
  mSccConfig.eMMCEnabled   = PCH_DEVICE_ENABLE;


  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gDxePchPlatformPolicyProtocolGuid,
                  &mPchPolicyData,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;

}
