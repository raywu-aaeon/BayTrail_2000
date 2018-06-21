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
  ScPolicyInitDxe.c

  @brief
  This file is SampleCode for Intel PCH DXE Platform Policy initialization.

--*/

#include <Library/ScPolicyInitDxe.h>
#include <Library/PchPlatformLib.h>

#define SW_SMI_BIOS_LOCK              0xA9
#define PCI_CLASS_NETWORK             0x02
#define PCI_CLASS_NETWORK_ETHERNET    0x00
#define PCI_CLASS_NETWORK_OTHER       0x80

EFI_GUID                        gDxePchPolicyUpdateProtocolGuid = DXE_PCH_POLICY_UPDATE_PROTOCOL_GUID;

DXE_PCH_POLICY_UPDATE_PROTOCOL    mDxePchPolicyUpdate   = { 0 };
DXE_PCH_PLATFORM_POLICY_PROTOCOL  mPchPolicyData        = { 0 };
PCH_DEVICE_ENABLING               mPchDeviceEnabling    = { 0 };
PCH_USB_CONFIG                    mPchUsbConfig         = { 0 };
PCH_MISC_PM_CONFIG                mPchMiscPmConfig      = { 0 };
PCH_DEFAULT_SVID_SID              mPchDefaultSvidSid    = { 0 };
PCH_LOCK_DOWN_CONFIG              mPchLockDownConfig    = { 0 };
PCH_LPC_SIRQ_CONFIG               mSerialIrqConfig      = { 0 };
PCH_PWR_OPT_CONFIG                mPchPwrOptConfig      = { 0 };
PCH_LPSS_CONFIG                   mLpssConfig           = { 0 };
PCH_SCC_CONFIG                    mSccConfig            = { 0 };
PCH_SMBUS_CONFIG                  mSmbusConfig          = { 0 };
PCH_SATA_CONFIG                   mSataConfig           = { 0 };
PCH_PCI_EXPRESS_CONFIG            mPciExpressConfig     = { 0 };
PCH_AZALIA_CONFIG                 mAzaliaConfig         = { 0 };


UINT8 SmbusRsvdAddresses[4] = {
    DIMM1_SMBUS_ADDRESS,
    DIMM2_SMBUS_ADDRESS,
    DIMM3_SMBUS_ADDRESS,
    DIMM4_SMBUS_ADDRESS
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
  {0x8086, 0x08B0, 0xff, PCI_CLASS_NETWORK, PCI_CLASS_NETWORK_OTHER, PchPcieAspmL1}
};



///
/// Function implementations
///
EFI_STATUS
EFIAPI
ScPolicyInitDxe(
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable,
    IN SB_SETUP_DATA            *PchPolicyData
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
    EFI_HANDLE              Handle;
    EFI_STATUS              Status;
    UINT8                   PortIndex;
//EIP176554 >>
    SB_HDA_VERB_TABLE       *AzaliaVerbTable;
    UINT8                   VerbTableNum = 0;
//EIP176554 <<
#if CRB_VERB_TABLE_SUPPORT
    UINT32                  VerbIndex;
#endif

    ///
    /// General initialization
    ///
    mPchPolicyData.Revision             = PchPolicyData->SbPolicyVersion;
    mPchPolicyData.BusNumber            = 0;
    mPchPolicyData.LockDownConfig->BiosInterface = PCH_DEVICE_ENABLE;

    mPchPolicyData.DeviceEnabling       = &mPchDeviceEnabling;
    mPchPolicyData.UsbConfig            = &mPchUsbConfig;

    mPchPolicyData.MiscPmConfig         = &mPchMiscPmConfig;
    mPchPolicyData.DefaultSvidSid       = &mPchDefaultSvidSid;
    mPchPolicyData.LockDownConfig       = &mPchLockDownConfig;
    mPchPolicyData.SerialIrqConfig      = &mSerialIrqConfig;
    mPchPolicyData.PwrOptConfig         = &mPchPwrOptConfig;
    mPchPolicyData.LpssConfig           = &mLpssConfig;
    mPchPolicyData.SccConfig            = &mSccConfig;
    mPchPolicyData.SmbusConfig          = &mSmbusConfig;
    mPchPolicyData.SataConfig           = &mSataConfig;
    mPchPolicyData.PciExpressConfig     = &mPciExpressConfig;
    mPchPolicyData.AzaliaConfig         = &mAzaliaConfig;

    mPchPolicyData.S0ixSupport          = PchPolicyData->S0ixSupport;    //(EIP113298)
    mPchPolicyData.EhciPllCfgEnable     = PchPolicyData->EhciPllCfgEnable;
    mPchPolicyData.SataOddPort			= PchPolicyData->SataOddPort; //EIP149024
    ///
    /// VLV BIOS Spec Rev 0.5 Section 26.8 Flash Security Recommendations,
    /// Intel strongly recommends that BIOS sets the BIOS Interface Lock Down bit. Enabling this bit
    /// will mitigate malicious software attempts to replace the system BIOS option ROM with its own code.
    /// We always enable this as a platform policy.
    ///
    mPchLockDownConfig.BiosInterface          = PCH_DEVICE_ENABLE;
    mPchLockDownConfig.GlobalSmi              = PchPolicyData->GlobalSmi;
    mPchLockDownConfig.RtcLock                = PCH_DEVICE_ENABLE;
    ///
    /// While BiosLock is enabled, BIOS can only be modified from SMM after ExitPmAuth.
    ///
    mPchLockDownConfig.BiosLock               = PchPolicyData->BiosWpd; //EIP130725 >>
    mPchLockDownConfig.PchBiosLockSwSmiNumber = SW_SMI_BIOS_LOCK;
    ///
    /// Initialize policy to default values when variable isn't found.
    ///
    mPchDeviceEnabling.LpeEnabled         = PchPolicyData->Lpe;
    // DeviceEnables
    mPchDeviceEnabling.Lan                = PchPolicyData->Lan;
    mPchDeviceEnabling.Azalia             = PchPolicyData->PchAzalia;
    mPchDeviceEnabling.Sata               = PchPolicyData->PchSata;
    mPchDeviceEnabling.Smbus              = PchPolicyData->Smbus;

    // CRID support for special BYT-D sku only
    mPchDeviceEnabling.Crid               = PchPolicyData->PchCrid;
    
    // USB Device 29 configuration
    mPchUsbConfig.Usb20Settings[0].Enable = PchPolicyData->PchUsb20;
    mPchUsbConfig.UsbPerPortCtl           = PchPolicyData->PchUsbPerPortCtl;

    if(PchPolicyData->PchUsbPerPortCtl != PCH_DEVICE_DISABLE) {
        for(PortIndex = 0; PortIndex < PCH_USB_MAX_PHYSICAL_PORTS; PortIndex++) {
            mPchUsbConfig.PortSettings[PortIndex].Enable = PchPolicyData->PchUsbPort[PortIndex];
            mPchUsbConfig.PortSettings[PortIndex].Dock   = PchPolicyData->PchUsbDock[PortIndex];
            mPchUsbConfig.PortSettings[PortIndex].Panel  = PchPolicyData->PchUsbPanel[PortIndex];
        }
    }

    mPchUsbConfig.Usb30Settings.XhciStreams  = PchPolicyData->PchUsb30Streams;
    mPchUsbConfig.Usb30Settings.Mode  = PchPolicyData->PchUsb30Mode;
    mPchUsbConfig.UsbOtgSettings.Enable = PchPolicyData->PchUsbOtg;

    for(PortIndex = 0; PortIndex < PCH_USB_MAX_PHYSICAL_PORTS; PortIndex++) {
        mPchUsbConfig.Usb20PortLength[PortIndex]   = PchPolicyData->Usb20PortLength[PortIndex];
        mPchUsbConfig.Usb20OverCurrentPins[PortIndex]   = PchPolicyData->Usb20OverCurrentPins[PortIndex];
    }
    mPchUsbConfig.Usb30OverCurrentPins[0]   = PchPolicyData->Usb30OverCurrentPins[0];

    mPchUsbConfig.EhciDebug           =  PchPolicyData->PchEhciDebug;

    mPchUsbConfig.Ehci1Usbr               = PchPolicyData->Ehci1Usbr;
    mPchPolicyData.XhciWorkaroundSwSmiNumber = (PchPolicyData->OsSelect == 1) ? 0 : XHCI_WORKAROUND_SW_SMI; //EIP171355
    mPchUsbConfig.UsbXhciLpmSupport       = PchPolicyData->UsbXhciLpmSupport;


    mPchPwrOptConfig.NumOfDevLtrOverride      = 0;
    mPchPwrOptConfig.DevLtrOverride           = NULL;

    //
    // Remove XHCI Pre-Boot Driver setup option selection from end-user view and automate loading of USB 3.0 BIOS driver based on XhciMode selection
    //
    switch(PchPolicyData->PchUsb30Mode) {
    case 0: // Disabled
        mPchUsbConfig.Usb30Settings.PreBootSupport = 0;
        break;
    case 1: // Enabled
        mPchUsbConfig.Usb30Settings.PreBootSupport = 1;
        break;
    case 2: // Auto
        mPchUsbConfig.Usb30Settings.PreBootSupport = 0;
        break;
    case 3: // Smart Auto
        mPchUsbConfig.Usb30Settings.PreBootSupport = 1;
        break;
    default:
        mPchUsbConfig.Usb30Settings.PreBootSupport  = PchPolicyData->PchUsbPreBootSupport; //CSP20130723_C
        break;
    }

    for (PortIndex = 0; PortIndex < PCH_PCIE_MAX_ROOT_PORTS; PortIndex++) {
      mPciExpressConfig.RootPort[PortIndex].Enable                        = PchPolicyData->PcieRootPortEn[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].SlotImplemented               = PchPolicyData->PcieRootPortEn[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].FunctionNumber                = PortIndex;
      mPciExpressConfig.RootPort[PortIndex].PhysicalSlotNumber            = PortIndex;
      mPciExpressConfig.RootPort[PortIndex].Aspm                          = PchPolicyData->PcieRootPortAspm[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].PmSci                         = PchPolicyData->PcieRootPortPMCE[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].ExtSync                       = PchPolicyData->PcieRootPortESE[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].HotPlug                       = PchPolicyData->PcieRootPortHPE[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].AdvancedErrorReporting        = PCH_DEVICE_DISABLE;
      mPciExpressConfig.RootPort[PortIndex].UnsupportedRequestReport      = PchPolicyData->PcieRootPortURE[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].FatalErrorReport              = PchPolicyData->PcieRootPortFEE[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].NoFatalErrorReport            = PchPolicyData->PcieRootPortNFE[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].CorrectableErrorReport        = PchPolicyData->PcieRootPortCEE[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].PmeInterrupt                  = 0;
      mPciExpressConfig.RootPort[PortIndex].SystemErrorOnFatalError       = PchPolicyData->PcieRootPortSFE[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].SystemErrorOnNonFatalError    = PchPolicyData->PcieRootPortSNE[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].SystemErrorOnCorrectableError = PchPolicyData->PcieRootPortSCE[PortIndex];
      mPciExpressConfig.RootPort[PortIndex].CompletionTimeout             = PchPciECompletionTO_Default;
    }
    mPciExpressConfig.TempRootPortBusNumMin             = 0x5;
    mPciExpressConfig.TempRootPortBusNumMax             = 0x7;
    mPciExpressConfig.RootPortClockGating               = 1;
    mPciExpressConfig.PcieDynamicGating                 = PchPolicyData->PcieDynamicGating;

    mPciExpressConfig.NumOfDevAspmOverride = sizeof (mDevAspmOverride) / sizeof (PCH_PCIE_DEVICE_ASPM_OVERRIDE);
    mPciExpressConfig.DevAspmOverride      = mDevAspmOverride;
      
    ///
    /// MiscPm Configuration
    ///
    mPchMiscPmConfig.WakeConfig.WolEnableOverride         = PchPolicyData->WakeOnLanS5;
    mPchMiscPmConfig.PowerResetStatusClear.MeWakeSts      = PchPolicyData->MeWakeSts;
    mPchMiscPmConfig.PowerResetStatusClear.MeHrstColdSts  = PchPolicyData->MeHrstColdSts;
    mPchMiscPmConfig.PowerResetStatusClear.MeHrstWarmSts  = PchPolicyData->MeHrstWarmSts;

    mPchMiscPmConfig.PchSlpS3MinAssert                    = PchSlpS350ms;
    mPchMiscPmConfig.PchSlpS4MinAssert                    = PchSlpS44s;
    mPchMiscPmConfig.SlpStrchSusUp                        = PCH_DEVICE_DISABLE;
    mPchMiscPmConfig.SlpLanLowDc                          = PchPolicyData->SlpLanLowDc;

  //
  // SATA configuration
  //
  for (PortIndex = 0; PortIndex < PCH_AHCI_MAX_PORTS; PortIndex++) {
    if (PchPolicyData->SataInterfaceMode == 0) {
      mSataConfig.PortSettings[PortIndex].Enable   = PCH_DEVICE_ENABLE;
      mSataConfig.LegacyMode                       = PCH_DEVICE_ENABLE;
    } else {
      mSataConfig.PortSettings[PortIndex].Enable   = PCH_DEVICE_ENABLE;
      mSataConfig.LegacyMode                       = PCH_DEVICE_DISABLE;
    }
    if(PchPolicyData->PchSata == 1){
      mSataConfig.PortSettings[PortIndex].Enable   = PchPolicyData->SataPort[PortIndex];
    } else {
      mSataConfig.PortSettings[PortIndex].Enable   = PCH_DEVICE_DISABLE;
    }
//    if(0 == PortIndex){
      mSataConfig.PortSettings[PortIndex].HotPlug    = PchPolicyData->SataHotPlug[PortIndex];
//    } else if(1 == PortIndex){
//      mSataConfig.PortSettings[PortIndex].HotPlug    = PCH_DEVICE_DISABLE;
//    }

    mSataConfig.PortSettings[PortIndex].SpinUp     = PchPolicyData->SataSpinUp[PortIndex];
    mSataConfig.PortSettings[PortIndex].MechSw     = PchPolicyData->SataMechanicalSw[PortIndex];
  }
  mSataConfig.RaidAlternateId                 = PchPolicyData->SataAlternateId;
  mSataConfig.Raid0                           = PchPolicyData->SataRaidR0;
  mSataConfig.Raid1                           = PchPolicyData->SataRaidR1;
  mSataConfig.Raid10                          = PchPolicyData->SataRaidR10;
  mSataConfig.Raid5                           = PchPolicyData->SataRaidR5;
  mSataConfig.Irrt                            = PchPolicyData->SataRaidIrrt;
  mSataConfig.OromUiBanner                    = PchPolicyData->SataRaidOub;
  mSataConfig.HddUnlock                       = PchPolicyData->SataHddlk;
  mSataConfig.LedLocate                       = PchPolicyData->SataLedl;
  mSataConfig.IrrtOnly                        = PchPolicyData->SataRaidIooe;
  mSataConfig.SalpSupport                     = PchPolicyData->SalpSupport;
  mSataConfig.TestMode                        = PchPolicyData->SataTestMode;
  mSataConfig.SpeedSupport                    = PchPolicyData->SataSpeedSupport; //EIP147898

    ///
    /// Initialize Serial IRQ Config
    ///
    mSerialIrqConfig.SirqEnable       = PchPolicyData->SirqEnable;
    mSerialIrqConfig.SirqMode         = PchPolicyData->SirqMode;

    ///
    /// Initialize LPIO Config
    ///
    mLpssConfig.LpssPciModeEnabled    = PchPolicyData->LpssPciModeEnabled;
    mLpssConfig.Dma1Enabled    = PchPolicyData->LpssDma1Enabled;
    mLpssConfig.I2C0Enabled    = PchPolicyData->LpssI2C0Enabled;
    mLpssConfig.I2C1Enabled    = PchPolicyData->LpssI2C1Enabled;
    mLpssConfig.I2C2Enabled    = PchPolicyData->LpssI2C2Enabled;
    mLpssConfig.I2C3Enabled    = PchPolicyData->LpssI2C3Enabled;
    mLpssConfig.I2C4Enabled    = PchPolicyData->LpssI2C4Enabled;
    mLpssConfig.I2C5Enabled    = PchPolicyData->LpssI2C5Enabled;
    mLpssConfig.I2C6Enabled    = PchPolicyData->LpssI2C6Enabled;

    mLpssConfig.Dma0Enabled    = PchPolicyData->LpssDma0Enabled;
    mLpssConfig.Pwm0Enabled    = PchPolicyData->LpssPwm0Enabled;
    mLpssConfig.Pwm1Enabled    = PchPolicyData->LpssPwm1Enabled;
    mLpssConfig.Hsuart0Enabled = PchPolicyData->LpssHsuart0Enabled;
    mLpssConfig.Hsuart1Enabled = PchPolicyData->LpssHsuart1Enabled;
    mLpssConfig.SpiEnabled     = PchPolicyData->LpssSpiEnabled;
    ///
    /// Set SCC configuration according to setup value.
    ///

    mSccConfig.HsiEnabled    = PchPolicyData->MipiHsi;
    mSccConfig.SdcardEnabled = PchPolicyData->SdcardEnabled;
    mSccConfig.SdioEnabled   = PchPolicyData->SdioEnabled;
    mSccConfig.eMMCEnabled   = PchPolicyData->eMMCEnabled;
    mSccConfig.SdCardSDR25Enabled = PchPolicyData->LpssSdCardSDR25Enabled; //EIP144689 
    mSccConfig.SdCardDDR50Enabled = PchPolicyData->LpssSdCardDDR50Enabled; //EIP144689 

  //
  // Reserved SMBus Address
  //
  mSmbusConfig.NumRsvdSmbusAddresses = 4;
  mSmbusConfig.RsvdSmbusAddressTable = SmbusRsvdAddresses;

    if (PchPolicyData->eMMCEnabled== 1) {// Auto detection mode
    switch (PchStepping()) {
      case PchA0:
      case PchA1:
        DEBUG ((EFI_D_ERROR, "Auto Detect: A0/A1 SCC eMMC 4.41 Configuration\n"));
        mSccConfig.eMMCEnabled            = 1;
        mSccConfig.eMMC45Enabled          = 0;
        mSccConfig.eMMC45DDR50Enabled     = 0;
        mSccConfig.eMMC45HS200Enabled     = 0;
        mSccConfig.eMMC45RetuneTimerValue = 0;
        break;
      case PchB0:
      case PchB1:
      case PchB2:    
        DEBUG ((EFI_D_ERROR, "Auto Detect: B0 SCC eMMC 4.5 Configuration\n"));
        mSccConfig.eMMCEnabled            = 0;
        mSccConfig.eMMC45Enabled          = 1;
        mSccConfig.eMMC45DDR50Enabled     = PchPolicyData->eMMC45DDR50Enabled;
        mSccConfig.eMMC45HS200Enabled     = PchPolicyData->eMMC45HS200Enabled;
        mSccConfig.eMMC45RetuneTimerValue = PchPolicyData->eMMC45RetuneTimerValue;
        break;
      default:
	  //EIP147670 >>
        DEBUG ((EFI_D_ERROR, "Unknown Steppting, using eMMC 4.5 Configuration\n"));
        mSccConfig.eMMCEnabled            = 0;
        mSccConfig.eMMC45Enabled          = 1;
        mSccConfig.eMMC45DDR50Enabled     = PchPolicyData->eMMC45DDR50Enabled;
        mSccConfig.eMMC45HS200Enabled     = PchPolicyData->eMMC45HS200Enabled;
        mSccConfig.eMMC45RetuneTimerValue = PchPolicyData->eMMC45RetuneTimerValue;
	  //EIP147670 <<
        break;
    }
   }else if (PchPolicyData->eMMCEnabled == 2) { // eMMC 4.41 
      DEBUG ((EFI_D_ERROR, "A0/A1 SCC eMMC 4.41 Configuration\n"));
      mSccConfig.eMMCEnabled            = 1;
      mSccConfig.eMMC45Enabled          = 0;
      mSccConfig.eMMC45DDR50Enabled     = 0;
      mSccConfig.eMMC45HS200Enabled     = 0;
      mSccConfig.eMMC45RetuneTimerValue = 0;

   } else if (PchPolicyData->eMMCEnabled == 3) { // eMMC 4.5
        DEBUG ((EFI_D_ERROR, "B0 SCC eMMC 4.5 Configuration\n"));
        mSccConfig.eMMCEnabled            = 0;
        mSccConfig.eMMC45Enabled          = 1;
        mSccConfig.eMMC45DDR50Enabled     = PchPolicyData->eMMC45DDR50Enabled;
        mSccConfig.eMMC45HS200Enabled     = PchPolicyData->eMMC45HS200Enabled;
        mSccConfig.eMMC45RetuneTimerValue = PchPolicyData->eMMC45RetuneTimerValue;
       
   } else { // Disable eMMC controllers
        DEBUG ((EFI_D_ERROR, "Disable eMMC controllers\n"));
        mSccConfig.eMMCEnabled            = 0;
        mSccConfig.eMMC45Enabled          = 0;
        mSccConfig.eMMC45DDR50Enabled     = 0;
        mSccConfig.eMMC45HS200Enabled     = 0;
        mSccConfig.eMMC45RetuneTimerValue = 0;
   }
  
  //
  // AzaliaConfig
  //
  mAzaliaConfig.Pme       = PchPolicyData->AzaliaPme;
  mAzaliaConfig.HdmiCodec = PchPolicyData->HdmiCodec;
  mAzaliaConfig.DS        = PchPolicyData->AzaliaDs;
  mAzaliaConfig.DA        = PchPolicyData->AzaliaDa;
  mAzaliaConfig.AzaliaVCi = PchPolicyData->AzaliaVCiEnable;

  //
  // Program the default Sub System Vendor Device Id
  //
  mPchPolicyData.DefaultSvidSid->SubSystemVendorId = VLV_DEFAULT_VID;
  mPchPolicyData.DefaultSvidSid->SubSystemId       = VLV_DEFAULT_SID;

//EIP176554 >>
    Status = SbHdaVerbTableOverride (&AzaliaVerbTable, &VerbTableNum, VERB_TBL_LIB_PEI);
    if (Status == EFI_SUCCESS) {
      mAzaliaConfig.AzaliaVerbTableNum  = VerbTableNum;
      mAzaliaConfig.AzaliaVerbTable     = (PCH_AZALIA_VERB_TABLE *) AzaliaVerbTable;
      mAzaliaConfig.ResetWaitTimer      = 300;
      DEBUG((EFI_D_INFO, "mAzaliaConfig.AzaliaVerbTableNum = %x.\n",mAzaliaConfig.AzaliaVerbTableNum));
#if CRB_VERB_TABLE_SUPPORT
      for (VerbIndex=0; VerbIndex<mAzaliaConfig.AzaliaVerbTableNum ; VerbIndex++){
        if(mAzaliaConfig.AzaliaVerbTable[VerbIndex].VerbTableHeader.VendorDeviceId == 0x80862882){ //EIP136267
          DEBUG((EFI_D_INFO, "Find Intel VLV HDMI Verb table.\n"));
  		
          if (PchPolicyData->HdmiCodecPortB == PCH_DEVICE_DISABLE) {
            mAzaliaConfig.AzaliaVerbTable[VerbIndex].VerbTableData[3] |= 0x40;
          }
          if (PchPolicyData->HdmiCodecPortC == PCH_DEVICE_DISABLE) {
            mAzaliaConfig.AzaliaVerbTable[VerbIndex].VerbTableData[7] |= 0x40;
          }
          if (PchPolicyData->HdmiCodecPortD == PCH_DEVICE_DISABLE) {
            mAzaliaConfig.AzaliaVerbTable[VerbIndex].VerbTableData[11] |= 0x40;
          }
          break;
        }
      }
#endif
    }
//EIP176554 <<

    DEBUG((EFI_D_INFO, "mPchPolicyData.Revision :%x \n",mPchPolicyData.Revision));
    DEBUG((EFI_D_INFO, "mPchPolicyData.BusNumber :%x \n",mPchPolicyData.BusNumber));
    DEBUG((EFI_D_INFO, "mPchPolicyData.DeviceEnabling :%x \n",mPchPolicyData.DeviceEnabling));
    DEBUG((EFI_D_INFO, "mPchPolicyData.UsbConfig :%x \n",mPchPolicyData.UsbConfig));
    DEBUG((EFI_D_INFO, "mPchPolicyData.MiscPmConfig :%x \n",mPchPolicyData.MiscPmConfig));
    DEBUG((EFI_D_INFO, "mPchPolicyData.DefaultSvidSid :%x \n",mPchPolicyData.DefaultSvidSid));
    DEBUG((EFI_D_INFO, "mPchPolicyData.LockDownConfig :%x \n",mPchPolicyData.LockDownConfig));
    DEBUG((EFI_D_INFO, "mPchPolicyData.SerialIrqConfig :%x \n",mPchPolicyData.SerialIrqConfig));
    DEBUG((EFI_D_INFO, "mPchPolicyData.PwrOptConfig :%x \n",mPchPolicyData.PwrOptConfig));
    DEBUG((EFI_D_INFO, "mPchPolicyData.LpssConfig :%x \n",mPchPolicyData.LpssConfig));
    DEBUG((EFI_D_INFO, "mPchPolicyData.SccConfig :%x \n",mPchPolicyData.SccConfig));
    DEBUG((EFI_D_INFO, "mPchLockDownConfig.BiosInterface :%x \n",mPchLockDownConfig.BiosInterface));
    DEBUG((EFI_D_INFO, "mPchLockDownConfig.GlobalSmi :%x \n",mPchLockDownConfig.GlobalSmi));
    DEBUG((EFI_D_INFO, "mPchLockDownConfig.RtcLock :%x \n",mPchLockDownConfig.RtcLock));
    DEBUG((EFI_D_INFO, "mPchLockDownConfig.BiosLock :%x \n",mPchLockDownConfig.BiosLock));
    DEBUG((EFI_D_INFO, "mPchLockDownConfig.PchBiosLockSwSmiNumber :%x \n",mPchLockDownConfig.PchBiosLockSwSmiNumber));
    DEBUG((EFI_D_INFO, "mPchDeviceEnabling.LpeEnabled :%x \n",mPchDeviceEnabling.LpeEnabled));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.PortSettings[0].Enable :%x \n",mPchUsbConfig.PortSettings[0].Enable));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.PortSettings[1].Enable :%x \n",mPchUsbConfig.PortSettings[1].Enable));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.PortSettings[2].Enable :%x \n",mPchUsbConfig.PortSettings[2].Enable));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.PortSettings[3].Enable :%x \n",mPchUsbConfig.PortSettings[3].Enable));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.PortSettings[0].Panel :%x \n",mPchUsbConfig.PortSettings[0].Panel));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.PortSettings[1].Panel :%x \n",mPchUsbConfig.PortSettings[1].Panel));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.PortSettings[2].Panel :%x \n",mPchUsbConfig.PortSettings[2].Panel));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.PortSettings[3].Panel :%x \n",mPchUsbConfig.PortSettings[3].Panel));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.PortSettings[0].Dock :%x \n",mPchUsbConfig.PortSettings[0].Dock));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.PortSettings[1].Dock :%x \n",mPchUsbConfig.PortSettings[1].Dock));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.PortSettings[2].Dock :%x \n",mPchUsbConfig.PortSettings[2].Dock));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.PortSettings[3].Dock :%x \n",mPchUsbConfig.PortSettings[3].Dock));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Usb20Settings[0].Enable :%x \n",mPchUsbConfig.Usb20Settings[0].Enable));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Usb30Settings.PreBootSupport :%x \n",mPchUsbConfig.Usb30Settings.PreBootSupport));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Usb30Settings.Mode :%x \n",mPchUsbConfig.Usb30Settings.Mode));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.UsbOtgSettings.Enable :%x \n",mPchUsbConfig.UsbOtgSettings.Enable));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Usb20OverCurrentPins[0] :%x \n",mPchUsbConfig.Usb20OverCurrentPins[0]));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Usb20OverCurrentPins[1]  :%x \n",mPchUsbConfig.Usb20OverCurrentPins[1]));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Usb20OverCurrentPins[2]  :%x \n",mPchUsbConfig.Usb20OverCurrentPins[2]));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Usb20OverCurrentPins[3]  :%x \n",mPchUsbConfig.Usb20OverCurrentPins[3]));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Usb30OverCurrentPins[0] :%x \n",mPchUsbConfig.Usb30OverCurrentPins[0]));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Usb20PortLength[0] :%x \n",mPchUsbConfig.Usb20PortLength[0]));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Usb20PortLength[1] :%x \n",mPchUsbConfig.Usb20PortLength[1]));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Usb20PortLength[2] :%x \n",mPchUsbConfig.Usb20PortLength[2]));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Usb20PortLength[3] :%x \n",mPchUsbConfig.Usb20PortLength[3]));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.UsbPerPortCtl :%x \n",mPchUsbConfig.UsbPerPortCtl));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.Ehci1Usbr :%x \n",mPchUsbConfig.Ehci1Usbr));
    DEBUG((EFI_D_INFO, "mPchUsbConfig.UsbXhciLpmSupport :%x \n",mPchUsbConfig.UsbXhciLpmSupport));
    DEBUG((EFI_D_INFO, "mPchPwrOptConfig.NumOfDevLtrOverride :%x \n",mPchPwrOptConfig.NumOfDevLtrOverride));
    DEBUG((EFI_D_INFO, "mPchPwrOptConfig.DevLtrOverride :%x \n",mPchPwrOptConfig.DevLtrOverride));
    DEBUG((EFI_D_INFO, "mPchMiscPmConfig.WakeConfig.WolEnableOverride :%x \n",mPchMiscPmConfig.WakeConfig.WolEnableOverride));
    DEBUG((EFI_D_INFO, "mPchMiscPmConfig.PchSlpS3MinAssert :%x \n",mPchMiscPmConfig.PchSlpS3MinAssert));
    DEBUG((EFI_D_INFO, "mPchMiscPmConfig.PchSlpS4MinAssert :%x \n",mPchMiscPmConfig.PchSlpS4MinAssert));
    DEBUG((EFI_D_INFO, "mPchMiscPmConfig.SlpStrchSusUp :%x \n",mPchMiscPmConfig.SlpStrchSusUp));
    DEBUG((EFI_D_INFO, "mPchMiscPmConfig.SlpLanLowDc :%x \n",mPchMiscPmConfig.SlpLanLowDc));
    DEBUG((EFI_D_INFO, "mSerialIrqConfig.SirqEnable :%x \n",mSerialIrqConfig.SirqEnable));
    DEBUG((EFI_D_INFO, "mSerialIrqConfig.SirqMode :%x \n",mSerialIrqConfig.SirqMode));
    DEBUG((EFI_D_INFO, "mLpssConfig.Dma1Enabled :%x \n",mLpssConfig.Dma1Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.I2C0Enabled :%x \n",mLpssConfig.I2C0Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.I2C1Enabled :%x \n",mLpssConfig.I2C1Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.I2C2Enabled :%x \n",mLpssConfig.I2C2Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.I2C3Enabled :%x \n",mLpssConfig.I2C3Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.I2C4Enabled :%x \n",mLpssConfig.I2C4Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.I2C5Enabled :%x \n",mLpssConfig.I2C5Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.I2C6Enabled :%x \n",mLpssConfig.I2C6Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.Dma0Enabled :%x \n",mLpssConfig.Dma0Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.Pwm0Enabled :%x \n",mLpssConfig.Pwm0Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.Pwm1Enabled :%x \n",mLpssConfig.Pwm1Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.Hsuart0Enabled :%x \n",mLpssConfig.Hsuart0Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.Hsuart1Enabled :%x \n",mLpssConfig.Hsuart1Enabled));
    DEBUG((EFI_D_INFO, "mLpssConfig.SpiEnabled :%x \n",mLpssConfig.SpiEnabled));
    DEBUG((EFI_D_INFO, "mSccConfig.HsiEnabled :%x \n",mSccConfig.HsiEnabled));
    DEBUG((EFI_D_INFO, "mSccConfig.SdcardEnabled :%x \n",mSccConfig.SdcardEnabled));
    DEBUG((EFI_D_INFO, "mSccConfig.SdioEnabled :%x \n",mSccConfig.SdioEnabled));
    DEBUG((EFI_D_INFO, "mSccConfig.eMMCEnabled :%x \n",mSccConfig.eMMCEnabled));
    DEBUG((EFI_D_INFO, "mSccConfig.eMMC45Enabled :%x \n",mSccConfig.eMMC45Enabled));
    DEBUG((EFI_D_INFO, "mSccConfig.eMMC45DDR50Enabled :%x \n",mSccConfig.eMMC45DDR50Enabled));
    DEBUG((EFI_D_INFO, "mSccConfig.eMMC45HS200Enabled :%x \n",mSccConfig.eMMC45HS200Enabled));
    DEBUG((EFI_D_INFO, "mSccConfig.eMMC45RetuneTimerValue :%x \n",mSccConfig.eMMC45RetuneTimerValue));
    Handle = NULL;
    mDxePchPolicyUpdate.Revision  = DXE_PCH_POLICY_UPDATE_PROTOCOL_REVISION_1;

    Status = gBS->InstallMultipleProtocolInterfaces(
                 &Handle,
                 &gDxePchPlatformPolicyProtocolGuid,
                 &mPchPolicyData,
                 &gDxePchPolicyUpdateProtocolGuid,
                 &mDxePchPolicyUpdate,
                 NULL
             );
    ASSERT_EFI_ERROR(Status);

    return Status;

}
