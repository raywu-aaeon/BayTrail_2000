//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        SbPolicy.h
//
// Description: South Bridge setup data header file, define all the South
//              Bridge setup items and a structures in this file.
//
// Notes:       The context of the SB_SETUP_DATA may be able to copy from
//              SB.SD directly
//
//<AMI_FHDR_END>
//*************************************************************************
#include <Setup.h>
#include <Pi\PiHob.h>

#ifndef  _AMI_SB_POLICY_H   //To Avoid this header get compiled twice
#define  _AMI_SB_POLICY_H

//====hob
// {2CD84488-72E6-48a7-BA26-4F4043ACE182}
#define AMI_SB_PLATFORM_INFO_HOB_GUID \
    {0x2cd84488, 0x72e6, 0x48a7, 0xba, 0x26, 0x4f, 0x40, 0x43, 0xac, 0xe1, 0x82}

typedef struct  _SB_SETUP_DATA  SB_SETUP_DATA;
#define PCH_PCIE_MAX_ROOT_PORTS             4
#define PCH_USB_MAX_PHYSICAL_PORTS          4      /// Max Physical Connector EHCI + XHCI, not counting virtual ports like USB-R.
#define PCH_EHCI_MAX_PORTS                  4      /// Counting ports behind RMHs 8 from EHCI-1 and 6 from EHCI-2, not counting EHCI USB-R virtual ports.
#define PCH_HSIC_MAX_PORTS                  2
#define PCH_XHCI_MAX_USB3_PORTS             1
#define PCH_AHCI_MAX_PORTS                  2

#pragma pack(push, 1)

typedef struct _SB_SETUP_DATA {
    // SB Setup header
    UINT32      SbPolicyVersion;

    //
    // Sc Policy
    //
    // DeviceEnables
    UINT8       Lan;
    UINT8       PchAzalia;
    UINT8       PchSata;
    UINT8       Smbus;          //Need to check this.
    UINT8       Lpe;
    UINT8       Ehci1Usbr;      //Need to check this.

    UINT8       HpetEnable;

    // USB Device 29 configuration
    UINT8       PchUsb20;
    UINT8       PchUsb20W8;  //CSP20140423_23
    UINT8       PchUsbPerPortCtl;
    // USB port Enable
    UINT8       PchUsbPort[PCH_USB_MAX_PHYSICAL_PORTS];
    // xHCI (USB 3.0) related settings from setup variable
    UINT8       PchUsb30Streams;
    UINT8       PchUsb30Mode;
    UINT8       PchUsb30ModeW8; //EIP158981
    UINT8       PchUsbPreBootSupport; //CSP20130723_C
    UINT8       UsbXhciLpmSupport;
    // USB Otg Enable
    UINT8       PchUsbOtg;
    UINT8       PchUsbVbusOn;       //OTG VBUS control
    // USB Dock
    UINT8       PchUsbDock[PCH_USB_MAX_PHYSICAL_PORTS];
    // USB Panel
    UINT8       PchUsbPanel[PCH_USB_MAX_PHYSICAL_PORTS];
    UINT16      Usb20PortLength[PCH_USB_MAX_PHYSICAL_PORTS];
    UINT8       Usb20OverCurrentPins[PCH_USB_MAX_PHYSICAL_PORTS];
    UINT8       Usb30OverCurrentPins[PCH_XHCI_MAX_USB3_PORTS];
    UINT8       PchEhciDebug; //P20130628
    UINT8       EhciPllCfgEnable; //P20130628

    // PCIe Config
    UINT8       PcieRootPortEn[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8       PcieRootPortAspm[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8       PcieRootPortPMCE[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8       PcieRootPortESE[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8       PcieRootPortHPE[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8       PcieRootPortURE[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8       PcieRootPortFEE[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8       PcieRootPortNFE[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8       PcieRootPortCEE[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8       PcieRootPortSFE[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8       PcieRootPortSNE[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8       PcieRootPortSCE[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8       PcieRootPortSpeed[PCH_PCIE_MAX_ROOT_PORTS];
    
	//EIP150027 >>
    // PCI Bridge Resources
    UINT8   	ExtraBusRsvd[PCH_PCIE_MAX_ROOT_PORTS];
    UINT16  	PcieMemRsvd[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8   	PcieMemRsvdalig[PCH_PCIE_MAX_ROOT_PORTS];
    UINT16  	PciePFMemRsvd[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8   	PciePFMemRsvdalig[PCH_PCIE_MAX_ROOT_PORTS];
    UINT8   	PcieIoRsvd[PCH_PCIE_MAX_ROOT_PORTS];
	//EIP150027 <<

    // SATA Config
    UINT8   SataPort[PCH_AHCI_MAX_PORTS];
    UINT8   SataHotPlug[PCH_AHCI_MAX_PORTS];
    UINT8   SataMechanicalSw[PCH_AHCI_MAX_PORTS];
    UINT8   ExternalSata[PCH_AHCI_MAX_PORTS];
    UINT8   SataSpinUp[PCH_AHCI_MAX_PORTS];
    UINT8   SolidStateDrive[PCH_AHCI_MAX_PORTS];
    UINT8   SataInterfaceMode;
    UINT8   SataAlternateId;
    UINT8   SataRaidR0;
    UINT8   SataRaidR1;
    UINT8   SataRaidR10;
    UINT8   SataRaidR5;
    UINT8   SataRaidIrrt;
    UINT8   SataRaidOub;
    UINT8   SataHddlk;
    UINT8   SataLedl;
    UINT8   SataRaidIooe;
    UINT8   SataTestMode;
    UINT8   SataSpeedSupport; //EIP147898
    UINT8   SataOddPort; //EIP149024
    UINT8   SalpSupport;
    UINT8   SataLegacyMode;
    UINT8   SataControllerSpeed;

    // AzaliaConfig
    UINT8       AzaliaPme;
    UINT8       HdmiCodec;
    UINT8       AzaliaDs;
    UINT8       AzaliaDa;
    UINT8       AzaliaVCiEnable;
    UINT8       HdmiCodecPortB;
    UINT8       HdmiCodecPortC;
    UINT8       HdmiCodecPortD;

    // Set LPSS configuration according to setup value.
    UINT8       OsSelect; //CSP20130910 - Match RC 1.1.0
    UINT8       LpssPciModeEnabled;
    UINT8       LpssDma1Enabled;
    UINT8       LpssI2C0Enabled;
    UINT8       LpssI2C1Enabled;
    UINT8       LpssI2C2Enabled;
    UINT8       LpssI2C3Enabled;
    UINT8       LpssI2C4Enabled;
    UINT8       LpssI2C5Enabled;
    UINT8       LpssI2C6Enabled;
//EIP158981 >>  
    UINT8       NfcEnable;
    UINT8       TouchPadEnable;
    UINT8       I2CTouchAddress;
//EIP158981 <<  
    UINT8       LpssDma0Enabled;
    UINT8       LpssPwm0Enabled;
    UINT8       LpssPwm1Enabled;
    UINT8       LpssHsuart0Enabled;
    UINT8       LpssHsuart1Enabled;
    UINT8       LpssSpiEnabled;

    // Set SCC configuration according to setup value.
    UINT8       eMMCEnabled;
    UINT8       SdioEnabled;
    UINT8       SdcardEnabled;
    UINT8       MipiHsi;
    UINT8       eMMC45Enabled;      /// Determines if SCC eMMC 4.5 enabled  //(EIP120879+)>>
    UINT8       eMMC45DDR50Enabled;  /// Determines if DDR50 enabled for eMMC 4.5
    UINT8       eMMC45HS200Enabled;  /// Determines if HS200nabled for eMMC 4.5
    UINT8       eMMC45RetuneTimerValue;  /// Determines retune timer value. //(EIP120879+)<<
    UINT8       SecureErase; //CSP20130910 - Match RC 1.1.0
    
    // MiscPm Configuration
    UINT8       WakeOnLanS5;
    UINT8       SlpLanLowDc;
    UINT8       MeWakeSts;
    UINT8       MeHrstColdSts;
    UINT8       MeHrstWarmSts;

    // Enable / disable serial IRQ according to setup value.
    UINT8       SirqEnable;

    // Set Serial IRQ Mode Select according to setup value.
    UINT8       SirqMode;

    // Select CRID 
    UINT8		PchCrid; //EIP150551
    
    // BIOS INT13 Emulation for USB Mass Devices
    UINT8         UsbBIOSINT13DeviceEmulation;
    UINT8         UsbBIOSINT13DeviceEmulationLockHide;

    // BIOS INT13 Emulation Size for USB Mass Devices
    UINT16        UsbBIOSINT13DeviceEmulationSize;
    UINT8         UsbBIOSINT13DeviceEmulationSizeLockHide;

    // USB Zip Emulation Type
    UINT8         UsbZipEmulation;
    UINT8         UsbZipEmulationLockHide;

    // System ports
    UINT8         Serial;
    UINT8         Serial2;
    UINT8         ParallelMode;
    UINT8         UsbLegacy;
    
    // Uart
    UINT8       UartDebugEnable;   		// 332 UART debug Enabled 0: Disbale; 1: Enable  //EIP133060
    UINT8       PcuUart1;               // 333 PCU UART 1 Enabled
    UINT8       PcuUart2;               // 334 PCU UART 2 Enabled

    //Temp
    UINT8       BspSelection;
    UINT8       ProcessorFlexibleRatio;
    UINT8       ProcessorBistEnable;
    UINT8       ProcessorVmxEnable;
    UINT8       ActiveProcessorCores;
    UINT8       ProcessorHyperThreadingDisable;
    UINT8       SmartMode;
    UINT8		BatterySolution;
    UINT8		NFCnSelect;
    // Other items
    // .....
    // ..
    // .
    UINT8       LastState;
    UINT8       PmicEnable;
    UINT8       S0ixSupport;    //(EIP113298+)
    UINT8	    WittEnable;    //(EIP120879+)
    
    UINT8       GlobalSmi; //EIP144291
    UINT8 	    BiosWpd; //EIP130725
    UINT8       PcieDynamicGating; //CSP20131018
    UINT8       LpssSdCardSDR25Enabled; //EIP144689
    UINT8       LpssSdCardDDR50Enabled; //EIP144689
} SB_SETUP_DATA;

typedef struct _AMI_SB_PLATFORM_INFO_HOB {
    EFI_HOB_GUID_TYPE EfiHobGuidType;
    SB_SETUP_DATA SbPolicyData;
} AMI_SB_PLATFORM_INFO_HOB;

#pragma pack(pop)

typedef VOID (SB_OEM_SETUP_CALLBACK)(
    IN VOID                 *Services,
    IN OUT SB_SETUP_DATA    *SbSetupData,
    IN SETUP_DATA           *SetupData,
    IN BOOLEAN              Pei
);


VOID GetSbSetupData(
    IN VOID                 *Service,
    IN OUT SB_SETUP_DATA    *SbSetupData,
    IN BOOLEAN              Pei
);

//CSP20140314_21>>
//CSP20130930>>
typedef struct _ADJUST_GPIO_SETTING {
    UINT32                    Offset;
    UINT32                    AndData;
    UINT32                    OrData;
} ADJUST_GPIO_SETTING;
//CSP20130930<<
//CSP20140314_21<<
#endif

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
