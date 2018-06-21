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
// Description: Custom North Bridge setup data behavior implementation
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>


#include <Setup.h>
#include <PiPei.h>
#include <AmiPeiLib.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Library/NbPolicy.h>
#include <Library/ElinkLib.h>
#include <Library/HobLib.h>
#include <Guid/HobList.h>

//---------------------------------------------------------------------------
// Constant, Macro and Type Definition(s)
//---------------------------------------------------------------------------
// Constant Definition(s)

// Macro Definition(s)

// Type Definition(s)

// Function Prototype(s)

VOID *
EFIAPI
CopyMem (
  OUT VOID       *DestinationBuffer,
  IN CONST VOID  *SourceBuffer,
  IN UINTN       Length
  );

//---------------------------------------------------------------------------
// Variable and External Declaration(s)
//---------------------------------------------------------------------------

// GUID Definition(s)

static EFI_GUID gSetupGuid = SETUP_GUID;

// Protocol/Ppi Definition(s)

// External Declaration(s)

// Function Definition(s)

//---------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbOemSetupCallbacks
//
// Description: This function calls registered callbacks for OEM/custom setup.
//
// Input:       *Services    - Pointer to PeiServices or RuntimeServices
//                             structure  
//              *NbSetupData - Pointer to custom setup data to return
//              *SetupData   - Pointer to system setup data.
//              Pei          - Pei flag. If TRUE we are in PEI phase
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID NbOemSetupCallbacks (
    IN VOID                 *Services,
    IN OUT NB_SETUP_DATA    *NbSetupData,
    IN SETUP_DATA           *SetupData,
    IN BOOLEAN              Pei
  )
{
  UINT32                  ElinkPtr;
  AMI_HOOK_LINK           *AmiHookLink;
  UINT32                  Index;
  NB_OEM_SETUP_CALLBACK   *Elink;

  ElinkPtr = ElinkGet (PcdToken(PcdNbOemSetupElink));

  if (ElinkPtr == 0) {
    return;
  }

  AmiHookLink = (AMI_HOOK_LINK *) ElinkPtr;

  for (Index = 0; Index < ELINK_ARRAY_NUM; Index++) {
    if (AmiHookLink->ElinkArray[Index] == NULL) {
      break;
    }
    Elink = AmiHookLink->ElinkArray[Index];
    Elink(Services, NbSetupData, SetupData, Pei);
  }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetNbSetupData
//
// Description: This function returns custom setup data from system SetupData
//              variable 
//
// Input:       *Services    - Pointer to PeiServices or RuntimeServices
//                             structure  
//              *NbSetupData - Pointer to custom setup data to return
//              Pei          - Pei flag. If TRUE we are in PEI phase
//
// Output:      None
//
// Notes:       PORTING REQUIRED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID GetNbSetupData (
    IN VOID                 *Services,
    IN OUT NB_SETUP_DATA    *NbSetupData,
    IN BOOLEAN              Pei
)
{
    EFI_STATUS                      Status;
    SETUP_DATA                      SetupData;
    EFI_PEI_SERVICES                **PeiServices;
    EFI_RUNTIME_SERVICES            *RunServices = NULL; //EIP164253
    EFI_PEI_READ_ONLY_VARIABLE2_PPI *ReadOnlyVariable = NULL;
    UINTN                           VariableSize = sizeof(SETUP_DATA);
    AMI_NB_PLATFORM_INFO_HOB        *NbPlatformInfoHob;
#ifdef AMI_NB_SETUPDATA_PEI
    VOID                            *Hob;
#else
    EFI_PEI_HOB_POINTERS            GuidHob;
    NB_SETUP_DATA                   *NbVarInfoHob;
    CHAR16                          NBPlatformInfoVar[] = L"NBPlatformInfo";
#endif
    EFI_GUID                        gAmiNbPlatformInfoHobGuid = AMI_NB_PLATFORM_INFO_HOB_GUID;

    if(Pei)
      PeiServices = (EFI_PEI_SERVICES **)Services;
    else
      RunServices = (EFI_RUNTIME_SERVICES *)Services;

#ifdef AMI_NB_SETUPDATA_PEI
    // Found the NbPlatformInfoHob
    if(Pei) {
      Status = (*PeiServices)->GetHobList(PeiServices, (VOID**)&Hob);
      NbPlatformInfoHob = (AMI_NB_PLATFORM_INFO_HOB*)Hob;
      while (!EFI_ERROR(Status = FindNextHobByType(EFI_HOB_TYPE_GUID_EXTENSION, &NbPlatformInfoHob))) {
        if (guidcmp(&NbPlatformInfoHob->EfiHobGuidType.Name, &gAmiNbPlatformInfoHobGuid)==0) {
          (*PeiServices)->CopyMem(NbSetupData, &NbPlatformInfoHob->NbPolicyData, sizeof(NB_SETUP_DATA));
          return;
        }
      }
    }
#else
    VariableSize = sizeof(NB_SETUP_DATA);
    Status = RunServices->GetVariable( NBPlatformInfoVar, \
                                       &gAmiNbPlatformInfoHobGuid, \
                                       NULL, \
                                       &VariableSize, \
                                       NbSetupData );
    if(!EFI_ERROR(Status)) {
        return;
    }
    GuidHob.Raw = GetHobList ();
    if (GuidHob.Raw != NULL) {
      GuidHob.Raw = GetNextGuidHob (&gAmiNbPlatformInfoHobGuid, GuidHob.Raw);
      if (GuidHob.Raw != NULL) {
        NbVarInfoHob = GET_GUID_HOB_DATA (GuidHob.Guid);
        CopyMem(NbSetupData, NbVarInfoHob, sizeof(NB_SETUP_DATA));
        Status = RunServices->SetVariable( NBPlatformInfoVar, \
                                           &gAmiNbPlatformInfoHobGuid, \
                                           EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                                           sizeof(NB_SETUP_DATA), \
                                           NbSetupData);
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

    NbSetupData->NbPolicyVersion = NB_POLICY_VERSION; // This value must be updated if
                                          // the structure of NB_SETUP_DATA 
                                          // is changed.
                                          // Porting required.

    // Update data for NC Policy
    if(EFI_ERROR(Status)){
      NbSetupData->InternalGraphics       = 1;
      NbSetupData->PrimaryDisplay         = 3;
      NbSetupData->PavpMode               = 1; //CSP20131018
      NbSetupData->IgdDvmt50PreAlloc      = 2;
      NbSetupData->ApertureSize           = 2;
      NbSetupData->GttSize                = 2;
      NbSetupData->ISPEn                  = 1;
      NbSetupData->ISPDevSel              = 1;
      NbSetupData->EnablePS2ForVccVnn     = 0; //EIP154014
      NbSetupData->EnableRenderStandby    = 1;
      NbSetupData->IgdTurboEn             = 1;
      NbSetupData->IgdSciSmiMode          = 0;
      NbSetupData->IgdTvFormat            = 0;
      NbSetupData->IgdTvMinor             = 0;
      NbSetupData->IgdSscConfig           = 1;
      NbSetupData->IgdBiaConfig           = 0;
      NbSetupData->IgdBlcConfig           = 0;
      NbSetupData->IgdDvmtMemSize         = 2;
      NbSetupData->IgdState               = 1;
      NbSetupData->BacklightControlSupport	= 2;
      NbSetupData->BrightnessPercentage = 100;
      NbSetupData->DeviceId1 = 0x80000100 ;
      NbSetupData->DeviceId2 = 0x80000400 ;
      NbSetupData->DeviceId3 = 0x80000200 ;
      NbSetupData->DeviceId4 = 0x04;
      NbSetupData->DeviceId5 = 0x05;
      NbSetupData->NumberOfValidDeviceId = 4 ;
      NbSetupData->CurrentDeviceList = 0x0F ;
      NbSetupData->PreviousDeviceList = 0x0F ;

      NbSetupData->AlsEnable = 0x01 ;
      //MRC
      NbSetupData->FastBoot               = 0;      // Need to check this that MRC is decided to enable this by BootMode == BOOT_WITH_MINIMAL_CONFIGURATION".
      NbSetupData->DynSR                  = 1;      // Need to check this that MRC is set 1 directly.

      //Temp
      NbSetupData->MaxInverterPWM         = 0; // Not to be used now.
      NbSetupData->MinInverterPWM         = 0; // Not to be used now.
      NbSetupData->GraphicsPerfAnalyzers  = 0; // This variable haven't doing anything.
      NbSetupData->ForceWake = 1;
      NbSetupData->PmWeights = 1;
      NbSetupData->EuControl = 1;
      NbSetupData->PmLock = 0;
      NbSetupData->DopClockGating = 1;
      NbSetupData->IgdFlatPanel = 1;
      NbSetupData->PanelScaling = 0;
      NbSetupData->LidStatus= 1;
      NbSetupData->S0ixSupported = 1;

      //DPTF_SETTINGS
      NbSetupData->Clpm = 3;
      NbSetupData->EnableDppm = 0;
      NbSetupData->EnableDptf = 0;
      NbSetupData->DptfSysThermal0 = 0;
      NbSetupData->DptfSysThermal1 = 0;
      NbSetupData->DptfSysThermal2 = 0;
      NbSetupData->DptfSysThermal3 = 0;
      NbSetupData->DptfSysThermal4 = 0;
      NbSetupData->DptfChargerDevice = 0;
      NbSetupData->DptfDisplayDevice = 0;
      NbSetupData->DptfSocDevice = 0;
      NbSetupData->DptfProcessor = 0;
      NbSetupData->GenericCriticalTemp0 = 70;
      NbSetupData->GenericCriticalTemp1 = 70;
      NbSetupData->GenericCriticalTemp2 = 70;
      NbSetupData->GenericCriticalTemp3 = 0;
      NbSetupData->GenericCriticalTemp4 = 0;
      NbSetupData->GenericPassiveTemp0 = 60;
      NbSetupData->GenericPassiveTemp1 = 60;
      NbSetupData->GenericPassiveTemp2 = 60;
      NbSetupData->GenericPassiveTemp3 = 0;
      NbSetupData->GenericPassiveTemp4 = 0;
      NbSetupData->LPOEnable = 0;
      NbSetupData->LPOPerformanceControlSetting = 0;
      NbSetupData->LPOPowerControlSetting = 0;
      NbSetupData->LPOStartPState = 0;
      NbSetupData->LPOStepSize = 0;
      NbSetupData->CriticalThermalTripPoint = 0;
      NbSetupData->PassiveThermalTripPoint = 0;
      NbSetupData->PassiveTc1Value = 0x01;
      NbSetupData->PassiveTc2Value = 0x05;
      NbSetupData->PassiveTspValue = 0x32;      
      NbSetupData->AmbientTripPointChange = 0;
      NbSetupData->SuperDebug = 0;

//EIP150350 >>
      NbSetupData->PrimaryDisplay 	= 3;  	  
#if CSM_SUPPORT
    //VBIOS
      NbSetupData->LcdPanelType 	= 0;
      NbSetupData->LcdPanelScaling 	= 0;
      NbSetupData->IgdBootType 		= 0;
      NbSetupData->IgdLcdBlc 		= 0;
      NbSetupData->ActiveLFP 		= 1;
      NbSetupData->DisplayPipeB 	= 0;    
      NbSetupData->SdvoPanelType 	= 0;    
#endif 
//EIP150350 <<
    } else {
      NbSetupData->InternalGraphics = SetupData.InternalGraphics;
      NbSetupData->PrimaryDisplay = SetupData.PrimaryDisplay;
      NbSetupData->PavpMode       = SetupData.PavpMode; //CSP20131018
      NbSetupData->IgdDvmt50PreAlloc = SetupData.IgdDvmt50PreAlloc;
      NbSetupData->ApertureSize = SetupData.ApertureSize;
      NbSetupData->GttSize = SetupData.GttSize;
      NbSetupData->ISPEn = SetupData.ISPEn;
      NbSetupData->ISPDevSel = SetupData.ISPDevSel;
      NbSetupData->EnablePS2ForVccVnn = SetupData.EnablePS2ForVccVnn; //EIP154014
      NbSetupData->EnableRenderStandby = SetupData.EnableRenderStandby;
      NbSetupData->IgdTurboEn = SetupData.IgdTurboEn;
      NbSetupData->IgdSciSmiMode       = 0;
      NbSetupData->IgdTvFormat           = 0;
      NbSetupData->IgdTvMinor             = 0;
      NbSetupData->IgdSscConfig           = SetupData.SscEn; //EIP127925
      NbSetupData->IgdBiaConfig           = 0;
      NbSetupData->IgdBlcConfig           = 0;
      NbSetupData->IgdDvmtMemSize    = SetupData.IgdDvmtGfxMem;
      NbSetupData->IgdState                    = 1;
      NbSetupData->BacklightControlSupport	= 2;
      NbSetupData->BrightnessPercentage = 100;
      NbSetupData->DeviceId1 = 0x80000100 ;
      NbSetupData->DeviceId2 = 0x80000400 ;
      NbSetupData->DeviceId3 = 0x80000200 ;
      NbSetupData->DeviceId4 = 0x04;
      NbSetupData->DeviceId5 = 0x05;
      NbSetupData->NumberOfValidDeviceId = 4 ;
      NbSetupData->CurrentDeviceList = 0x0F ;
      NbSetupData->PreviousDeviceList = 0x0F ;

      NbSetupData->AlsEnable = SetupData.AlsSupport ;
      //MRC
      NbSetupData->FastBoot               = SetupData.MrcFastBoot;      // Need to check this that MRC is decided to enable this by BootMode == BOOT_WITH_MINIMAL_CONFIGURATION".
      NbSetupData->DynSR                  = SetupData.DynSR;      // Need to check this that MRC is set 1 directly.      

      //Temp
      NbSetupData->MaxInverterPWM         = 0; // Not to be used now.
      NbSetupData->MinInverterPWM          = 0; // Not to be used now.
      NbSetupData->GraphicsPerfAnalyzers  = 0; // This variable haven't doing anything.
      NbSetupData->ForceWake = 1;
      NbSetupData->PmWeights = 1;
      NbSetupData->EuControl = 1;
      NbSetupData->PmLock = 0;
        NbSetupData->DopClockGating =SetupData.DopCG;
      NbSetupData->IgdFlatPanel = SetupData.IgdFlatPanel;
      NbSetupData->PanelScaling = SetupData.PanelScaling;
      NbSetupData->LidStatus= SetupData.LidStatus;
      NbSetupData->S0ixSupported = SetupData.PpmS0ix;
      if(SetupData.PpmS0ix){    //(CSP20130328A+)
        // override the EnableRenderstandby
        NbSetupData->EnableRenderStandby = 1;
      }

      //DPTF_SETTINGS
      NbSetupData->Clpm = SetupData.Clpm;
      NbSetupData->EnableDppm = SetupData.EnableDppm;
      NbSetupData->EnableDptf = SetupData.EnableDptf;
      // now temple disable the thermal
      NbSetupData->DptfSysThermal0 =  SetupData.SystemThermalSensor1;
      NbSetupData->DptfSysThermal1 = SetupData.SystemThermalSensor2;
      NbSetupData->DptfSysThermal2 =  SetupData.SystemThermalSensor3;
      NbSetupData->DptfSysThermal3 =  0;//SetupData.SystemThermalSensor4;
      NbSetupData->DptfSysThermal4 =  0;//SetupData.SystemThermalSensor5;
      NbSetupData->DptfChargerDevice = SetupData.ChargerParticipant;
      NbSetupData->DptfDisplayDevice = SetupData.DisplayParticipant;
      NbSetupData->DptfSocDevice = SetupData.SocParticipant;
      NbSetupData->DptfProcessor = SetupData.DptfProcessor;
      NbSetupData->GenericCriticalTemp0 = SetupData.GenericCriticalTemp0;
      NbSetupData->GenericCriticalTemp1 = SetupData.GenericCriticalTemp1;
      NbSetupData->GenericCriticalTemp2 = SetupData.GenericCriticalTemp2;
      NbSetupData->GenericCriticalTemp3 = 0;//SetupData.GenericCriticalTemp3;
      NbSetupData->GenericCriticalTemp4 = 0;//SetupData.GenericCriticalTemp4;
      NbSetupData->GenericPassiveTemp0 = SetupData.GenericPassiveTemp0;
      NbSetupData->GenericPassiveTemp1 = SetupData.GenericPassiveTemp1;
      NbSetupData->GenericPassiveTemp2 = SetupData.GenericPassiveTemp2;
      NbSetupData->GenericPassiveTemp3 = 0;//SetupData.GenericPassiveTemp3;
      NbSetupData->GenericPassiveTemp4 = 0;//SetupData.GenericPassiveTemp4;
      NbSetupData->LPOEnable = SetupData.LPOEnable;
      NbSetupData->LPOPerformanceControlSetting = SetupData.LPOPerformanceControlSetting;
      NbSetupData->LPOPowerControlSetting = SetupData.LPOPowerControlSetting;
      NbSetupData->LPOStartPState = SetupData.LPOStartPState;
      NbSetupData->LPOStepSize = SetupData.LPOStepSize;
      NbSetupData->CriticalThermalTripPoint = SetupData.CriticalThermalTripPoint;
      NbSetupData->PassiveThermalTripPoint = SetupData.PassiveThermalTripPoint;
      NbSetupData->PassiveTc1Value = 0x01;
      NbSetupData->PassiveTc2Value = 0x05;
      NbSetupData->PassiveTspValue = 0x32;
      NbSetupData->AmbientTripPointChange =  0;//SetupData.AmbientTripPointChange; //P20130628
      NbSetupData->SuperDebug = SetupData.SuperDebug;      

//EIP150350 >>
      NbSetupData->PrimaryDisplay 	= SetupData.PrimaryDisplay;       
#if CSM_SUPPORT
    //VBIOS
      NbSetupData->LcdPanelType 	= SetupData.LcdPanelType;
      NbSetupData->LcdPanelScaling 	= SetupData.LcdPanelScaling;
      NbSetupData->IgdBootType 		= SetupData.IgdBootType;
      NbSetupData->IgdLcdBlc 		= SetupData.IgdLcdBlc;
      NbSetupData->ActiveLFP 		= SetupData.ActiveLFP;
      NbSetupData->DisplayPipeB 	= SetupData.DisplayPipeB;    
      NbSetupData->SdvoPanelType 	= SetupData.SdvoPanelType;    
#endif   
//EIP150350 <<
    }

    if (EFI_ERROR(Status)) {
        // Porting Start
        // Update NB_SETUP_DATA according to the default values.

        //#### NbSetupData->SpreadSpectrum = 0;
        // Porting End
        NbOemSetupCallbacks( Services, NbSetupData, NULL, Pei );
    } else {
        // Porting Start
        // Update NB_SETUP_DATA according to the setup datas.

        //#### NbSetupData->SpreadSpectrum = SetupData->SpreadSpectrum;

        // Porting End
        NbOemSetupCallbacks( Services, NbSetupData, &SetupData, Pei );
    }

    if(Pei){
        Status = (*PeiServices)->CreateHob(
                    PeiServices,
                    EFI_HOB_TYPE_GUID_EXTENSION,
                    sizeof(AMI_NB_PLATFORM_INFO_HOB),
                    &NbPlatformInfoHob);

        if (!EFI_ERROR(Status)) {
            (*PeiServices)->SetMem((VOID *)&(NbPlatformInfoHob->NbPolicyData), (sizeof(AMI_NB_PLATFORM_INFO_HOB) - sizeof(EFI_HOB_GUID_TYPE)), 0);
            NbPlatformInfoHob->EfiHobGuidType.Name = gAmiNbPlatformInfoHobGuid;
        }

        (*PeiServices)->CopyMem(&NbPlatformInfoHob->NbPolicyData, NbSetupData, sizeof(NB_SETUP_DATA));

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
