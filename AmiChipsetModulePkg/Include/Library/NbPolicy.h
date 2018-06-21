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
// Name:        NbPolicy.h
//
// Description: North Bridge setup data header file, define all the North
//              Bridge setup items and a structures in this file. 
//
// Notes:       The context of the NB_SETUP_DATA may be able to copy from
//              NB.SD directly 
//
//<AMI_FHDR_END>
//*************************************************************************
#include <Setup.h>
#include <Pi\PiHob.h>

#ifndef _AMI_NB_SETUP_POLICY_H // To Avoid this header get compiled twice
#define _AMI_NB_SETUP_POLICY_H

#ifdef __cplusplus
extern "C" {
#endif

//====hob
// {F6E49CBF-C35B-4679-A775-F57FBC034706}
#define AMI_NB_PLATFORM_INFO_HOB_GUID \
    {0xf6e49cbf, 0xc35b, 0x4679, 0xa7, 0x75, 0xf5, 0x7f, 0xbc, 0x3, 0x47, 0x6}

#pragma pack(push, 1)

typedef struct _NB_SETUP_DATA  NB_SETUP_DATA;

typedef struct _NB_SETUP_DATA {
    // NB Setup header
    UINT32  NbPolicyVersion;

    // IGD Policy
    UINT8       InternalGraphics;
    UINT8       PrimaryDisplay;
    UINT8       PavpMode; //CSP20131018
    UINT8       IgdDvmt50PreAlloc;
    UINT8       ApertureSize;
    UINT8       GttSize;
    UINT8       ISPEn;
    UINT8       ISPDevSel;
    UINT8       EnablePS2ForVccVnn; //EIP154014
    UINT8       EnableRenderStandby;
    UINT8       IgdTurboEn;    //(EIP114446)
    UINT8       IgdSciSmiMode;
    UINT8       IgdTvFormat;
    UINT8       IgdTvMinor;
    UINT8       IgdSscConfig;
    UINT8       IgdBiaConfig;
    UINT8       IgdBlcConfig;
    UINT8       IgdDvmtMemSize;  
    UINT8       BacklightControlSupport;
    UINT8       BrightnessPercentage;
    UINT8       IgdState;
    UINT32      DeviceId1;
    UINT32      DeviceId2;
    UINT32      DeviceId3;
    UINT32      DeviceId4;
    UINT32      DeviceId5;
    UINT8       NumberOfValidDeviceId;
    UINT8       CurrentDeviceList ;
    UINT8       PreviousDeviceList ;

    UINT8       AlsEnable;    //(CSP20130313A+)
    //MRC
    UINT8       FastBoot;      // Need to check this that MRC is decided to enable this by BootMode == BOOT_WITH_MINIMAL_CONFIGURATION".
    UINT8       DynSR;      // Need to check this that MRC is set 1 directly.

    //Temp
    UINT8       MaxInverterPWM; // Not to be used now.
    UINT8       MinInverterPWM; // Not to be used now.
    UINT8       GraphicsPerfAnalyzers; // This variable haven't doing anything.
    UINT8       ForceWake;
    UINT8       PmWeights;
    UINT8       EuControl;
    UINT8       PmLock;
    UINT8       DopClockGating;
    UINT8       IgdFlatPanel;
    UINT8       PanelScaling;
    UINT8       LidStatus;
    UINT8       S0ixSupported;
    UINT8       Clpm; // Current low power mode
    UINT8       EnableDppm; // DPTF: Controls DPPM Policies (enabled/disabled)
    UINT8       EnableDptf; // Option to enable/disable DPTF
    UINT8       AmbientTripPointChange; //P20130628
    UINT8       DptfSysThermal0;
    UINT8       DptfSysThermal1;
    UINT8       DptfSysThermal2;
    UINT8       DptfSysThermal3;  
    UINT8       DptfSysThermal4;    	
    UINT8       DptfChargerDevice;
    UINT8       DptfDisplayDevice;
    UINT8       DptfSocDevice;      
    UINT8       DptfProcessor;
    UINT8       GenericCriticalTemp0; // Critical temperature value for generic sensor0 participant
    UINT8       GenericCriticalTemp1; // Critical temperature value for generic sensor1 participant
    UINT8       GenericCriticalTemp2; // Critical temperature value for generic sensor2 participant
    UINT8       GenericCriticalTemp3; // Critical temperature value for generic sensor3 participant
    UINT8       GenericCriticalTemp4; // Critical temperature value for generic sensor4 participant
    UINT8       GenericPassiveTemp0; // Passive temperature value for generic sensor0 participant
    UINT8       GenericPassiveTemp1; // Passive temperature value for generic sensor1 participant
    UINT8       GenericPassiveTemp2; // Passive temperature value for generic sensor2 participant
    UINT8       GenericPassiveTemp3; // Passive temperature value for generic sensor3 participant
    UINT8       GenericPassiveTemp4; // Passive temperature value for generic sensor4 participant
    UINT8       LPOEnable;  // DPTF: Instructs the policy to use Active Cores if they are available. If this option is set to 0, then policy does not use any active core controls ?even if they are available
    UINT8       LPOPerformanceControlSetting; // DPTF: Instructs the policy whether to use Core offliing or SMT offlining if Active core control is enabled to be used in P1 or when performance control is applied.1 ?SMT Off lining 2- Core Off lining
    UINT8       LPOPowerControlSetting; // DPTF: Instructs the policy whether to use Core offliing or SMT offlining if Active core control is enabled to be used in P0 or when power control is applied. 1 ?SMT Off lining 2- Core Off lining
    UINT8       LPOStartPState; // DPTF: Instructs the policy when to initiate Active Core control if enabled. Returns P state index.
    UINT8       LPOStepSize;  // DPTF: Instructs the policy to take away logical processors in the specified percentage steps
    UINT8       CriticalThermalTripPoint; // Processor critical temperature
    UINT8       PassiveThermalTripPoint;  // Processor passive temperature
    UINT8       PassiveTc1Value;
    UINT8       PassiveTc2Value;
    UINT8       PassiveTspValue;
    UINT8       SuperDebug; // DPTF Super debug option
    
//EIP150350 >>	
#if CSM_SUPPORT
    //VBIOS
    UINT8   	LcdPanelType;
    UINT8   	LcdPanelScaling;
    UINT8   	IgdBootType;
    UINT8   	IgdLcdBlc;
    UINT8   	ActiveLFP;
    UINT8   	DisplayPipeB;    
    UINT8   	SdvoPanelType;    
#endif 
//EIP150350 <<
    
    // Other items
    // .....
    // ..
    // .
} NB_SETUP_DATA;

typedef struct _AMI_NB_PLATFORM_INFO_HOB
{
    EFI_HOB_GUID_TYPE EfiHobGuidType;
    NB_SETUP_DATA NbPolicyData;
} AMI_NB_PLATFORM_INFO_HOB;

#pragma pack(pop)

typedef VOID (NB_OEM_SETUP_CALLBACK) (
    IN VOID                 *Services,
    IN OUT NB_SETUP_DATA    *NbSetupData,
    IN SETUP_DATA           *SetupData,
    IN BOOLEAN              Pei
);


VOID GetNbSetupData (
    IN VOID                 *Service,
    IN OUT NB_SETUP_DATA    *NbSetupData,
    IN BOOLEAN              Pei
);

/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
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
