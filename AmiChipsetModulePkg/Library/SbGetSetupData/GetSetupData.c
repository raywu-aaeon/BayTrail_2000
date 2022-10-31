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
//----------------------------------------------------------------------
//
// Name:        GetSetupData.c
//
// Description: Custom South Bridge setup data behavior implementation
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>


#include <Setup.h>
#include <PiPei.h>
#include <AmiPeiLib.h>
//#include <AmiDxeLib.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Library/SbPolicy.h>
#include <Library/ElinkLib.h>
#include <Library/HobLib.h>
#include <Guid/HobList.h>
#include <Library/DebugLib.h>
#include <Guid\MemoryTypeInformation.h> //EIP131494

//---------------------------------------------------------------------------
// Constant, Macro and Type Definition(s)
//---------------------------------------------------------------------------
// Constant Definition(s)

// Macro Definition(s)

// Type Definition(s)

// Function Prototype(s)

//---------------------------------------------------------------------------
// Variable and External Declaration(s)
//---------------------------------------------------------------------------

// GUID Definition(s)

static EFI_GUID gSetupGuid = SETUP_GUID;

// Protocol/Ppi Definition(s)

// External Declaration(s)

// Function Definition(s)

VOID *
EFIAPI
CopyMem (
  OUT VOID       *DestinationBuffer,
  IN CONST VOID  *SourceBuffer,
  IN UINTN       Length
  );

//---------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbOemSetupCallbacks
//
// Description: This function calls registered callbacks for OEM/custom setup.
//
// Input:       *Services    - Pointer to PeiServices or RuntimeServices
//                             structure  
//              *SbSetupData - Pointer to custom setup data to return
//              *SetupData   - Pointer to system setup data.
//              Pei          - Pei flag. If TRUE we are in PEI phase
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID SbOemSetupCallbacks (
    IN VOID              *Services,
    IN OUT SB_SETUP_DATA *SbSetupData,
    IN SETUP_DATA        *SetupData,
    IN BOOLEAN           Pei )
{
  UINT32                  ElinkPtr;
  AMI_HOOK_LINK           *AmiHookLink;
  UINT32                  Index;
  SB_OEM_SETUP_CALLBACK   *Elink;

  ElinkPtr = ElinkGet (PcdToken(PcdSbOemSetupElink));

  if (ElinkPtr == 0) {
    return;
  }

  AmiHookLink = (AMI_HOOK_LINK *) ElinkPtr;

  for (Index = 0; Index < ELINK_ARRAY_NUM; Index++) {
    if (AmiHookLink->ElinkArray[Index] == NULL) {
      break;
    }
    Elink = AmiHookLink->ElinkArray[Index];
    Elink(Services, SbSetupData, SetupData, Pei);
  }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetSbSetupData
//
// Description: This function returns custom setup data from system SetupData
//              variable 
//
// Input:       *Services         - Pointer to PeiServices or RuntimeServices
//                                  structure  
//              *SbSetupData      - Pointer to custom setup data to return
//              Pei               - Pei flag. If TRUE we are in PEI phase
//
// Output:      None
//
// Notes:       PORTING REQUIRED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID GetSbSetupData (
    IN VOID                 *Services,
    IN OUT SB_SETUP_DATA    *SbSetupData,
    IN BOOLEAN              Pei
)
{
    EFI_STATUS                      Status;
#if XHCI_WAKE_WORKAROUND
    EFI_STATUS                      Status2;
#endif    
    SETUP_DATA                      SetupData;
    EFI_PEI_SERVICES                **PeiServices;
    EFI_RUNTIME_SERVICES            *RunServices = NULL; //EIP164253 
    EFI_PEI_READ_ONLY_VARIABLE2_PPI *ReadOnlyVariable = NULL;
    UINTN                           VariableSize = sizeof(SETUP_DATA);
//    EFI_GUID                        HobListGuid = HOB_LIST_GUID;
    AMI_SB_PLATFORM_INFO_HOB        *SbPlatformInfoHob;
    UINT32                         Index;
#ifdef AMI_SB_SETUPDATA_PEI
    VOID                            *Hob;
#else
    EFI_PEI_HOB_POINTERS            GuidHob;
    SB_SETUP_DATA                   *SbVarInfoHob;
    CHAR16                          SbPlatformInfoVar[] = L"SBPlatformInfo";
#endif
    EFI_GUID                        gAmiSbPlatformInfoHobGuid = AMI_SB_PLATFORM_INFO_HOB_GUID;
    UINT16                          UsbPortLength[PCH_USB_MAX_PHYSICAL_PORTS] = {USB_PORTS_LENGTH};
    UINT8                           UsbOverCurrentMapping[PCH_USB_MAX_PHYSICAL_PORTS] = {USB_OVER_CURRENT_MAPPING_SETTINGS};
    UINT8                           Usb30OverCurrentMapping[PCH_XHCI_MAX_USB3_PORTS] = {USB30_OVER_CURRENT_MAPPING_SETTINGS};

    if(Pei)
      PeiServices = (EFI_PEI_SERVICES **)Services;
    else
      RunServices = (EFI_RUNTIME_SERVICES *)Services;

#ifdef AMI_SB_SETUPDATA_PEI
      // Found the SbPlatformInfoHob
    if(Pei) {
      Status = (*PeiServices)->GetHobList(PeiServices, (VOID**)&Hob);
      SbPlatformInfoHob = (AMI_SB_PLATFORM_INFO_HOB*)Hob;
      while (!EFI_ERROR(Status = FindNextHobByType(EFI_HOB_TYPE_GUID_EXTENSION, &SbPlatformInfoHob))) {
        if (guidcmp(&SbPlatformInfoHob->EfiHobGuidType.Name, &gAmiSbPlatformInfoHobGuid)==0) {
          (*PeiServices)->CopyMem(SbSetupData, &SbPlatformInfoHob->SbPolicyData, sizeof(SB_SETUP_DATA));
          return;
        }
      }
    }
#else
    VariableSize = sizeof(SB_SETUP_DATA);
    Status = RunServices->GetVariable( SbPlatformInfoVar, \
                                       &gAmiSbPlatformInfoHobGuid, \
                                       NULL, \
                                       &VariableSize, \
                                       SbSetupData );
    if(!EFI_ERROR(Status)) {
        return;
    }
    GuidHob.Raw = GetHobList ();
    if (GuidHob.Raw != NULL) {
      GuidHob.Raw = GetNextGuidHob (&gAmiSbPlatformInfoHobGuid, GuidHob.Raw);
      if (GuidHob.Raw != NULL) {
        SbVarInfoHob = GET_GUID_HOB_DATA (GuidHob.Guid);
        CopyMem(SbSetupData, SbVarInfoHob, sizeof(SB_SETUP_DATA));
        Status = RunServices->SetVariable( SbPlatformInfoVar, \
                                           &gAmiSbPlatformInfoHobGuid, \
                                           EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                                           sizeof(SB_SETUP_DATA), \
                                           SbSetupData);
        return;
      }
    }
#endif

    VariableSize = sizeof(SETUP_DATA);
    if (Pei) {
        Status = (*PeiServices)->LocatePpi( PeiServices, \
                                            &gEfiPeiReadOnlyVariable2PpiGuid, \
                                            0, \
                                            NULL, \
                                            &ReadOnlyVariable );

        if (!EFI_ERROR(Status)) {
            Status = ReadOnlyVariable->GetVariable( ReadOnlyVariable, \
                                                    L"Setup", \
                                                    &gSetupGuid, \
                                                    NULL, \
                                                    &VariableSize, \
                                                    &SetupData );
        }
    } else {
        Status = RunServices->GetVariable( L"Setup", \
                                           &gSetupGuid, \
                                           NULL, \
                                           &VariableSize, \
                                           &SetupData );
    }

    SbSetupData->SbPolicyVersion = SB_POLICY_VERSION; // This value must be updated if
                                          // the structure of SB_SETUP_DATA 
                                          // is changed.
                                          // Porting Required

    // Update data for SB Policy
    if(EFI_ERROR(Status)){
      // DeviceEnables
      SbSetupData->Lan                = 1;
      SbSetupData->PchAzalia          = 1;
      SbSetupData->PchSata            = 1;
      SbSetupData->Smbus              = 1;
      SbSetupData->Lpe                = 2;
      SbSetupData->Ehci1Usbr          = 0;      //Need to check this.

      // USB Device 29 configuration
//CSP20130828 >>
#if defined(AMIDEBUG_RX_SUPPORT) && AMIDEBUG_RX_SUPPORT
      SbSetupData->PchUsb20           = 1;
#else 
      SbSetupData->PchUsb20           = 0;      
#endif
//CSP20130828 <<
//EIP127537 >> 
      #if EOP_USB_PER_PORT_CTRL
      SbSetupData->PchUsbPerPortCtl   = 0;
      #else
      SbSetupData->PchUsbPerPortCtl   = 1;
      #endif
//EIP127537 << 
      // USB port Enable
      SbSetupData->PchUsbPort[0]      = 1;
      SbSetupData->PchUsbPort[1]      = 1;
      SbSetupData->PchUsbPort[2]      = 1;
      SbSetupData->PchUsbPort[3]      = 1;

      // xHCI (USB 3.0) related settings from setup variable
#if defined(AMIDEBUG_RX_SUPPORT) && AMIDEBUG_RX_SUPPORT
      SbSetupData->PchUsb30Streams    = 0;
      SbSetupData->PchUsb30Mode       = 0;
      SbSetupData->PchUsbPreBootSupport = 0; //EIP131027
      SbSetupData->UsbXhciLpmSupport  = 0;
#else
      SbSetupData->PchUsb30Streams    = 1;
      SbSetupData->PchUsb30Mode       = 1;
      SbSetupData->PchUsbPreBootSupport = 1; //CSP20130723_C
      SbSetupData->UsbXhciLpmSupport  = 1;
#endif

      SbSetupData->PchEhciDebug     = 0; //P20130628
      SbSetupData->EhciPllCfgEnable = 1; //P20130628
      // USB Otg Enable
      SbSetupData->PchUsbOtg          = 0;
      SbSetupData->PchUsbVbusOn       = 1;

      // USB Dock
      SbSetupData->PchUsbDock[0]      = 0;
      SbSetupData->PchUsbDock[1]      = 0;
      SbSetupData->PchUsbDock[2]      = 0;
      SbSetupData->PchUsbDock[3]      = 0;
      // USB Panel
      SbSetupData->PchUsbPanel[0]     = 1;
      SbSetupData->PchUsbPanel[1]     = 1;
      SbSetupData->PchUsbPanel[2]     = 0;
      SbSetupData->PchUsbPanel[3]     = 0;

      for (Index=0;Index<PCH_USB_MAX_PHYSICAL_PORTS;Index++) {
      	// USB PortLength
      	SbSetupData->Usb20PortLength[Index]  = UsbPortLength[Index];
      	// USB 2.0 OverCurrentPins
      	SbSetupData->Usb20OverCurrentPins[Index] = UsbOverCurrentMapping[Index];
      }

      // USB 3.0 OverCurrentPins
      SbSetupData->Usb30OverCurrentPins[0] = Usb30OverCurrentMapping[0];

      //PCIe Config Root 0
      SbSetupData->PcieRootPortEn[0]      = 1;
      SbSetupData->PcieRootPortAspm[0]    = 0;
      SbSetupData->PcieRootPortPMCE[0]    = 1;
      SbSetupData->PcieRootPortESE[0]     = 1;
      SbSetupData->PcieRootPortHPE[0]     = 0;
      SbSetupData->PcieRootPortURE[0]     = 0;
      SbSetupData->PcieRootPortFEE[0]     = 0;
      SbSetupData->PcieRootPortNFE[0]     = 0;
      SbSetupData->PcieRootPortCEE[0]     = 0;
      SbSetupData->PcieRootPortSFE[0]     = 0;
      SbSetupData->PcieRootPortSNE[0]     = 0;
      SbSetupData->PcieRootPortSCE[0]     = 0;
      SbSetupData->PcieRootPortSpeed[0]   = 0;
      //PCIe Config Root 1
      SbSetupData->PcieRootPortEn[1]      = 1;
      SbSetupData->PcieRootPortAspm[1]    = 0;
      SbSetupData->PcieRootPortPMCE[1]    = 1;
      SbSetupData->PcieRootPortESE[1]     = 1;
      SbSetupData->PcieRootPortHPE[1]     = 0;
      SbSetupData->PcieRootPortURE[1]     = 0;
      SbSetupData->PcieRootPortFEE[1]     = 0;
      SbSetupData->PcieRootPortNFE[1]     = 0;
      SbSetupData->PcieRootPortCEE[1]     = 0;
      SbSetupData->PcieRootPortSFE[1]     = 0;
      SbSetupData->PcieRootPortSNE[1]     = 0;
      SbSetupData->PcieRootPortSCE[1]     = 0;
      SbSetupData->PcieRootPortSpeed[1]   = 0;
      //PCIe Config Root 2
      SbSetupData->PcieRootPortEn[2]      = 1;
      SbSetupData->PcieRootPortAspm[2]    = 0;
      SbSetupData->PcieRootPortPMCE[2]    = 1;
      SbSetupData->PcieRootPortESE[2]     = 1;
      SbSetupData->PcieRootPortHPE[2]     = 0;
      SbSetupData->PcieRootPortURE[2]     = 0;
      SbSetupData->PcieRootPortFEE[2]     = 0;
      SbSetupData->PcieRootPortNFE[2]     = 0;
      SbSetupData->PcieRootPortCEE[2]     = 0;
      SbSetupData->PcieRootPortSFE[2]     = 0;
      SbSetupData->PcieRootPortSNE[2]     = 0;
      SbSetupData->PcieRootPortSCE[2]     = 0;
      SbSetupData->PcieRootPortSpeed[2]   = 0;
      //PCIe Config Root 3
      SbSetupData->PcieRootPortEn[3]      = 1;
      SbSetupData->PcieRootPortAspm[3]    = 0;
      SbSetupData->PcieRootPortPMCE[3]    = 1;
      SbSetupData->PcieRootPortESE[3]     = 1;
      SbSetupData->PcieRootPortHPE[3]     = 0;
      SbSetupData->PcieRootPortURE[3]     = 0;
      SbSetupData->PcieRootPortFEE[3]     = 0;
      SbSetupData->PcieRootPortNFE[3]     = 0;
      SbSetupData->PcieRootPortCEE[3]     = 0;
      SbSetupData->PcieRootPortSFE[3]     = 0;
      SbSetupData->PcieRootPortSNE[3]     = 0;
      SbSetupData->PcieRootPortSCE[3]     = 0;
      SbSetupData->PcieRootPortSpeed[3]   = 0;

      //
      // SATA configuration
      //
      //SATA Port 0
      SbSetupData->SataPort[0]            = 1;
      SbSetupData->SataHotPlug[0]         = 0;
      SbSetupData->SataMechanicalSw[0]    = 0;
      SbSetupData->ExternalSata[0]        = 0;
      SbSetupData->SataSpinUp[0]          = 0;
      SbSetupData->SolidStateDrive[0]     = 0;
      //SATA Port 1
      SbSetupData->SataPort[1]            = 1;
      SbSetupData->SataHotPlug[1]         = 0;
      SbSetupData->SataMechanicalSw[1]    = 0;
      SbSetupData->ExternalSata[1]        = 0;
      SbSetupData->SataSpinUp[1]          = 0;
      SbSetupData->SolidStateDrive[1]     = 0;

      SbSetupData->SataInterfaceMode      = 0;
      SbSetupData->SataAlternateId        = 0;
      SbSetupData->SataRaidR0             = 1;
      SbSetupData->SataRaidR1             = 1;
      SbSetupData->SataRaidR10            = 1;
      SbSetupData->SataRaidR5             = 1;
      SbSetupData->SataRaidIrrt           = 1;
      SbSetupData->SataRaidOub            = 1;
      SbSetupData->SataHddlk              = 1;
      SbSetupData->SataLedl               = 1;
      SbSetupData->SataRaidIooe           = 1;
      SbSetupData->SataTestMode           = 0;
      SbSetupData->SataSpeedSupport       = 2;  //EIP147898
      SbSetupData->SataOddPort       	  = 2;  //EIP149024
      SbSetupData->SalpSupport            = 1;
      SbSetupData->SataLegacyMode         = 1;
      SbSetupData->SataControllerSpeed    = 2;

      // AzaliaConfig
      SbSetupData->AzaliaPme              = 1;
      SbSetupData->HdmiCodec              = 0;
      SbSetupData->AzaliaDs               = 1;
      SbSetupData->AzaliaDa               = 0;
      SbSetupData->AzaliaVCiEnable        = 0;
      SbSetupData->HdmiCodecPortB         = 0;
      SbSetupData->HdmiCodecPortC         = 0;
      SbSetupData->HdmiCodecPortD         = 0;

      // Set LPSS configuration according to setup value.
      SbSetupData->OsSelect               = 0; //CSP20130910 - Match RC 1.1.0
      SbSetupData->LpssPciModeEnabled     = 0;
      SbSetupData->LpssDma1Enabled        = 1;
      SbSetupData->LpssI2C0Enabled        = 1;
      SbSetupData->LpssI2C1Enabled        = 1;
      SbSetupData->LpssI2C2Enabled        = 1;
      SbSetupData->LpssI2C3Enabled        = 1;
      SbSetupData->LpssI2C4Enabled        = 1;
      SbSetupData->LpssI2C5Enabled        = 1;
      SbSetupData->LpssI2C6Enabled        = 1;
//EIP158981 >>
      SbSetupData->NfcEnable              = 0;
      SbSetupData->TouchPadEnable         = 0; 
      SbSetupData->I2CTouchAddress        = 0x4C;
//EIP158981 <<
      SbSetupData->LpssDma0Enabled        = 1;
      SbSetupData->LpssPwm0Enabled        = 1;
      SbSetupData->LpssPwm1Enabled        = 1;
      SbSetupData->LpssHsuart0Enabled     = 1;
      SbSetupData->LpssHsuart1Enabled     = 1;
      SbSetupData->LpssSpiEnabled         = 1;

      // Set SCC configuration according to setup value.
      SbSetupData->eMMCEnabled        = 1;
      SbSetupData->SdioEnabled        = 1;
      SbSetupData->SdcardEnabled      = 1;
      SbSetupData->MipiHsi            = 1;

//(EIP120879+)>>
        SbSetupData->eMMC45DDR50Enabled        = 1;
        SbSetupData->eMMC45HS200Enabled         = 0;
        SbSetupData->eMMC45RetuneTimerValue   = 0x08;
//(EIP120879+)<<
      SbSetupData->SecureErase            = 0; //CSP20130910 - Match RC 1.1.0
        
      // MiscPm Configuration
      SbSetupData->WakeOnLanS5            = 0;
      SbSetupData->SlpLanLowDc            = 1;
      SbSetupData->MeWakeSts              = 1;
      SbSetupData->MeHrstColdSts          = 1;
      SbSetupData->MeHrstWarmSts          = 1;

      // Enable / disable serial IRQ according to setup value.
      SbSetupData->SirqEnable             = 1;

      // Set Serial IRQ Mode Select according to setup value.
      SbSetupData->SirqMode               = DEFAULT_SIRQ_MODE; //EIP133059

      SbSetupData->PchCrid                = 0; //EIP150551
      
      // Hpet
      SbSetupData->HpetEnable         = 1;

      // Uart
      SbSetupData->UartDebugEnable				    = DEFAULT_INTERNAL_UART_DEBUG_ENABLE;  //EIP133060
      SbSetupData->PcuUart1							= SOC_UART_PRESENT;  //EIP133060
      SbSetupData->PcuUart2							= 0;      
      
      //Temp
      SbSetupData->BspSelection = 0;
      SbSetupData->ProcessorFlexibleRatio = 0;
      SbSetupData->ProcessorBistEnable = 0;
      SbSetupData->ProcessorVmxEnable = 1;
      SbSetupData->ActiveProcessorCores = 0;
      SbSetupData->ProcessorHyperThreadingDisable = 0;
      SbSetupData->PmicEnable = 0;
      SbSetupData->LastState = 2;
      SbSetupData->S0ixSupport = 1;
      SbSetupData->WittEnable = 0;    //(EIP120879+)
 
      SbSetupData->GlobalSmi = 1; //EIP144291      
      SbSetupData->BiosWpd = 0; //EIP130725
      SbSetupData->PcieDynamicGating = 0; //CSP20131018
      SbSetupData->LpssSdCardSDR25Enabled      = 0; //EIP144689
      SbSetupData->LpssSdCardDDR50Enabled     = 1; //EIP144689       
    } else {
      // DeviceEnables
      SbSetupData->Lan                = SetupData.Lan;
      SbSetupData->PchAzalia          = SetupData.PchAzalia;
      SbSetupData->PchSata            = SetupData.PchSata;
      SbSetupData->Smbus              = 1;
      SbSetupData->Lpe                  = SetupData.Lpe;
      SbSetupData->Ehci1Usbr       = 0;      //Need to check this.

      // USB Device 29 configuration
      //CSP20140423_23 >>
      if(SetupData.OsSelect == 2)
      {
        SbSetupData->PchUsb20           = SetupData.PchUsb20;
      }
      else
      {
        SbSetupData->PchUsb20           = SetupData.PchUsb20W8;
      }
      //CSP20140423_23 <<
//EIP127537 >> 	  
      #if EOP_USB_PER_PORT_CTRL
      SbSetupData->PchUsbPerPortCtl   = 0;
      #else
      SbSetupData->PchUsbPerPortCtl   = SetupData.PchUsbPerPortCtl;
      #endif
//EIP127537 <<	  

      for (Index=0;Index<PCH_USB_MAX_PHYSICAL_PORTS;Index++) {
				// USB port Enable
				SbSetupData->PchUsbPort[Index]      = SetupData.PchUsbPort[Index];
        // USB PortLength
				SbSetupData->Usb20PortLength[Index]     = UsbPortLength[Index];
	      // USB 2.0 OverCurrentPins
	      SbSetupData->Usb20OverCurrentPins[Index]     = UsbOverCurrentMapping[Index];
      }

      // xHCI (USB 3.0) related settings from setup variable
      SbSetupData->PchUsb30Streams    = SetupData.PchUsb30Streams;
      //EIP158981 >>
      if(SetupData.OsSelect == 2)
      {
        SbSetupData->PchUsb30Mode       = SetupData.PchUsb30Mode;
      }
      else
      {
        SbSetupData->PchUsb30Mode       = SetupData.PchUsb30ModeW8;
      }
      //EIP158981 <<
      SbSetupData->PchUsbPreBootSupport = SetupData.SbUsbPreBootSupport; //CSP20130723_C
      SbSetupData->UsbXhciLpmSupport  = SetupData.UsbXhciLpmSupport;
     
//EIP131494 >>	  
#if XHCI_WAKE_WORKAROUND
      VariableSize = 0;
      // This variable is saved in BDS stage. Now read it back
      if (Pei) {
    	  Status2 = ReadOnlyVariable->GetVariable( ReadOnlyVariable, \
                                                  L"MemoryTypeInformation",\
                                                  &gEfiMemoryTypeInformationGuid,\
                                                  NULL,\
                                                  &VariableSize,\
                                                  NULL);
      } else {
    	  Status2 = RunServices->GetVariable( L"MemoryTypeInformation",
                                            &gEfiMemoryTypeInformationGuid,
                                            NULL,
                                            &VariableSize,
                                            NULL);
      }
      
      if (Status2 == EFI_NOT_FOUND) {
    	  //
    	  // Set PchUsb30Mode to "Disabled" for the first boot.
    	  //
    	  SbSetupData->PchUsb30Mode       = 0;
          SbSetupData->PchUsb20           = 1;      
      }
#endif
//EIP131494 <<

      SbSetupData->PchEhciDebug       = SetupData.PchEhciDebug; //P20130628
      SbSetupData->EhciPllCfgEnable   = SetupData.EhciPllCfgEnable; //P20130628
      // USB Otg Enable
      SbSetupData->PchUsbOtg          = SetupData.PchUsbOtg;
      SbSetupData->PchUsbVbusOn       = SetupData.PchUsbVbusOn;
      // USB Dock
      SbSetupData->PchUsbDock[0]      = 0;
      SbSetupData->PchUsbDock[1]      = 0;
      SbSetupData->PchUsbDock[2]      = 0;
      SbSetupData->PchUsbDock[3]      = 0;
      // USB Panel
      SbSetupData->PchUsbPanel[0]     = 1;
      SbSetupData->PchUsbPanel[1]     = 1;
      SbSetupData->PchUsbPanel[2]     = 0;
      SbSetupData->PchUsbPanel[3]     = 0;

      // USB 3.0 OverCurrentPins
      SbSetupData->Usb30OverCurrentPins[0]     = Usb30OverCurrentMapping[0];
//EIP154014 >>
      if (SbSetupData->PchUsb30Mode == 2 || SbSetupData->PchUsb30Mode == 3) {
        SbSetupData->PchUsb20 = 1;
      }
//EIP154014 <<
// EIP128872 >>
#if CSM_SUPPORT
#if FORCE_TO_EHCI_IN_CSM
      //
      // Force to use EHCI controller while CSM Support is enabled.
      //
      if (SetupData.CsmSupport == 1) {
    	  SbSetupData->PchUsb20           = 1;
    	  SbSetupData->PchUsb30Mode       = 0;
      }
#endif
#endif
// EIP128872 <<

      //PCIe Config Root 0
      for (Index=0;Index<PCH_PCIE_MAX_ROOT_PORTS;Index++) {
				SbSetupData->PcieRootPortEn[Index]      = SetupData.PcieRootPortEn[Index];
				SbSetupData->PcieRootPortAspm[Index]    = SetupData.PcieRootPortAspm[Index];
				SbSetupData->PcieRootPortPMCE[Index]    = SetupData.PcieRootPortPMCE[Index];
				SbSetupData->PcieRootPortESE[Index]     = SetupData.PcieRootPortESE[Index];
				SbSetupData->PcieRootPortHPE[Index]     = SetupData.PcieRootPortHPE[Index];
				SbSetupData->PcieRootPortURE[Index]     = SetupData.PcieRootPortURE[Index];
				SbSetupData->PcieRootPortFEE[Index]     = SetupData.PcieRootPortFEE[Index];
				SbSetupData->PcieRootPortNFE[Index]     = SetupData.PcieRootPortNFE[Index];
				SbSetupData->PcieRootPortCEE[Index]     = SetupData.PcieRootPortCEE[Index];
				SbSetupData->PcieRootPortSFE[Index]     = SetupData.PcieRootPortSFE[Index];
				SbSetupData->PcieRootPortSNE[Index]     = SetupData.PcieRootPortSNE[Index];
				SbSetupData->PcieRootPortSCE[Index]     = SetupData.PcieRootPortSCE[Index];
				SbSetupData->PcieRootPortSpeed[Index]   = SetupData.PcieRootPortSpeed[Index];
      }

      //
      // SATA configuration
      //
      for (Index=0;Index<PCH_AHCI_MAX_PORTS;Index++) {
				SbSetupData->SataPort[Index]            = SetupData.SataPort[Index];
				SbSetupData->SataHotPlug[Index]         = SetupData.SataHotPlug[Index];
				SbSetupData->SataMechanicalSw[Index]    = SetupData.SataMechanicalSw[Index];
				SbSetupData->ExternalSata[Index]        = SetupData.ExternalSata[Index];
				SbSetupData->SataSpinUp[Index]          = SetupData.SataSpinUp[Index];
				SbSetupData->SolidStateDrive[Index]     = SetupData.SolidStateDrive[Index];
      }

      SbSetupData->SataInterfaceMode      = SetupData.SataInterfaceMode;
      SbSetupData->SataAlternateId        = 0;		// Intel BIOS didn't open Raid option.
      SbSetupData->SataRaidR0             = 1;		// Intel BIOS didn't open Raid option.
      SbSetupData->SataRaidR1             = 1;		// Intel BIOS didn't open Raid option.
      SbSetupData->SataRaidR10            = 1;		// Intel BIOS didn't open Raid option.
      SbSetupData->SataRaidR5             = 1;		// Intel BIOS didn't open Raid option.
      SbSetupData->SataRaidIrrt           = 1;		// Intel BIOS didn't open Raid option.
      SbSetupData->SataRaidOub            = 1;		// Intel BIOS didn't open Raid option.
      SbSetupData->SataHddlk              = 1;		// Intel BIOS didn't open Raid option.
      SbSetupData->SataLedl               = 1;		// Intel BIOS didn't open Raid option.
      SbSetupData->SataRaidIooe           = 1;		// Intel BIOS didn't open Raid option.
      SbSetupData->SataTestMode           = SetupData.SataTestMode;
      SbSetupData->SataSpeedSupport       = SetupData.SataSpeedSupport;  //EIP147898
      SbSetupData->SataOddPort       	  = SetupData.SataOddPort;  //EIP149024
      SbSetupData->SalpSupport            = 1;
      SbSetupData->SataLegacyMode         = 1;
      SbSetupData->SataControllerSpeed    = 2;

      // AzaliaConfig
      SbSetupData->AzaliaPme              = SetupData.AzaliaPme;
      SbSetupData->HdmiCodec              = SetupData.HdmiCodec;
      SbSetupData->AzaliaDs               = SetupData.AzaliaDs;
      SbSetupData->AzaliaDa               = 0;
      SbSetupData->AzaliaVCiEnable        = SetupData.AzaliaVCiEnable;
      //(EIP136267+)>>
      SbSetupData->HdmiCodecPortB         = SetupData.HdmiCodecPortB;
      SbSetupData->HdmiCodecPortC         = SetupData.HdmiCodecPortC;
      SbSetupData->HdmiCodecPortD         = 0;
      //(EIP136267+)<<

      // Set LPSS configuration according to setup value.
      SbSetupData->OsSelect               = SetupData.OsSelect; //CSP20130910 - Match RC 1.1.0
      SbSetupData->LpssPciModeEnabled     = SetupData.LpssPciModeEnabled;
      SbSetupData->LpssDma1Enabled        = SetupData.LpssDma1Enabled;
      SbSetupData->LpssI2C0Enabled        = SetupData.LpssI2C0Enabled;
      SbSetupData->LpssI2C1Enabled        = SetupData.LpssI2C1Enabled;
      SbSetupData->LpssI2C2Enabled        = SetupData.LpssI2C2Enabled;
      SbSetupData->LpssI2C3Enabled        = SetupData.LpssI2C3Enabled;
      SbSetupData->LpssI2C4Enabled        = SetupData.LpssI2C4Enabled;
      SbSetupData->LpssI2C5Enabled        = SetupData.LpssI2C5Enabled;
      SbSetupData->LpssI2C6Enabled        = SetupData.LpssI2C6Enabled;
//EIP158981 >>
      SbSetupData->NfcEnable              = SetupData.NfcEnable;
      SbSetupData->TouchPadEnable         = SetupData.TouchPadEnable; 
      SbSetupData->I2CTouchAddress        = SetupData.I2CTouchAddress;
//EIP158981 <<      
      SbSetupData->LpssDma0Enabled        = SetupData.LpssDma0Enabled;
      SbSetupData->LpssPwm0Enabled        = SetupData.LpssPwm0Enabled;
      SbSetupData->LpssPwm1Enabled        = SetupData.LpssPwm1Enabled;
      SbSetupData->LpssHsuart0Enabled     = SetupData.LpssHsuart0Enabled;
      SbSetupData->LpssHsuart1Enabled     = SetupData.LpssHsuart1Enabled;
      SbSetupData->LpssSpiEnabled         	  = SetupData.LpssSpiEnabled;

      // Set SCC configuration according to setup value.
      SbSetupData->eMMCEnabled         = SetupData.eMMCEnabled;
      SbSetupData->SdioEnabled            = SetupData.SdioEnabled;
      SbSetupData->SdcardEnabled        = SetupData.SdcardEnabled;
      SbSetupData->MipiHsi                   = SetupData.MipiHsi;

//(EIP120879+)>>
      SbSetupData->eMMC45DDR50Enabled        = SetupData.eMMC45DDR50Enabled;
      SbSetupData->eMMC45HS200Enabled         = SetupData.eMMC45HS200Enabled;
      SbSetupData->eMMC45RetuneTimerValue   = 0x08;
//(EIP120879+)<<
      SbSetupData->SecureErase            = SetupData.SecureErase; //CSP20130910 - Match RC 1.1.0
      
      // MiscPm Configuration
      SbSetupData->WakeOnLanS5            = SetupData.WakeOnLanS5;
      SbSetupData->SlpLanLowDc            = SetupData.SlpLanLowDc;
      SbSetupData->MeWakeSts              = 1;
      SbSetupData->MeHrstColdSts          = 1;
      SbSetupData->MeHrstWarmSts          = 1;

      // Enable / disable serial IRQ according to setup value.
      SbSetupData->SirqEnable             = 1;

      // Set Serial IRQ Mode Select according to setup value.
      SbSetupData->SirqMode               = SetupData.SirqMode; //EIP133059

      SbSetupData->PchCrid                = SetupData.PchCrid; //EIP150551
      
      // Hpet
      SbSetupData->HpetEnable         = SetupData.HpetEnable;

      // Uart
      SbSetupData->UartDebugEnable			    = SetupData.UartDebugEnable; //EIP133060 
      SbSetupData->PcuUart1						= SOC_UART_PRESENT; //EIP133060
      SbSetupData->PcuUart2						= 0;          
      
      //Temp
      SbSetupData->BspSelection = 0;
      SbSetupData->ProcessorFlexibleRatio = 0;
      SbSetupData->ProcessorBistEnable = 0;
      SbSetupData->ProcessorVmxEnable = 1;
      SbSetupData->ActiveProcessorCores = 0;
      SbSetupData->ProcessorHyperThreadingDisable = 0;
      SbSetupData->PmicEnable = 0;
      SbSetupData->LastState = SetupData.LastState;
      SbSetupData->S0ixSupport = SetupData.PpmS0ix;
      SbSetupData->WittEnable = SetupData.WittEnable;;    //(EIP120879+)

      SbSetupData->GlobalSmi = SetupData.GlobalSmi; //EIP144291
      SbSetupData->BiosWpd = SetupData.BiosWpd; //EIP130725
      SbSetupData->PcieDynamicGating = SetupData.PcieDynamicGating; //CSP20131018
      SbSetupData->LpssSdCardSDR25Enabled      = SetupData.LpssSdCardSDR25Enabled; //EIP144689
      SbSetupData->LpssSdCardDDR50Enabled     = SetupData.LpssSdCardDDR50Enabled; //EIP144689         
      SbSetupData->WdtEnabled = SetupData.WdtEnabled;
      SbSetupData->WdtUnit = SetupData.WdtUnit;
      SbSetupData->WdtTimer = SetupData.WdtTimer;
    }

    if (EFI_ERROR(Status)) {
        // Porting Start
        //  Update SB_SETUP_DATA according to the default values.
        //####SbSetupData->Audio = 1;
        // Porting End

        SbOemSetupCallbacks( Services, SbSetupData, NULL, Pei );
    } else {
        // Porting Start
        //  Update SB_SETUP_DATA according to the setup datas.
        //####SbSetupData->Audio = SetupData.Audio;
        //####.....
        // Porting End

        SbOemSetupCallbacks( Services, SbSetupData, &SetupData, Pei );
    }

    if(Pei){
        Status = (*PeiServices)->CreateHob(
                    PeiServices,
                    EFI_HOB_TYPE_GUID_EXTENSION,
                    sizeof(AMI_SB_PLATFORM_INFO_HOB),
                    &SbPlatformInfoHob);

        if (!EFI_ERROR(Status)) {
            (*PeiServices)->SetMem((VOID *)&(SbPlatformInfoHob->SbPolicyData), (sizeof(AMI_SB_PLATFORM_INFO_HOB) - sizeof(EFI_HOB_GUID_TYPE)), 0);
            SbPlatformInfoHob->EfiHobGuidType.Name = gAmiSbPlatformInfoHobGuid;
        }

        (*PeiServices)->CopyMem(&SbPlatformInfoHob->SbPolicyData, SbSetupData, sizeof(SB_SETUP_DATA));
    }
}

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
