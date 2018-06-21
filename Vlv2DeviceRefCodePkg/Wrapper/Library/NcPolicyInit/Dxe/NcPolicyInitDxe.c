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
  NcPolicyInitDxe.c

  @brief
  This file is SampleCode for Intel PCH DXE Platform Policy initialization.

--*/

#include <Library/NcPolicyInitDxe.h>
#include <Library/SbPolicy.h>
#include "PchRegs.h"
#include <Library/PchPlatformLib.h>
#include <CpuRegs.h>    //(EIP134992+)

DXE_VLV_PLATFORM_POLICY_PROTOCOL mDxePlatformVlvPolicy;
///
/// Function implementations
///
EFI_STATUS
EFIAPI
NcPolicyInitDxe(
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable,
    IN NB_SETUP_DATA            *VlvPolicyData
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
    DXE_VLV_PLATFORM_POLICY_PROTOCOL *DxePlatformVlvPolicy;
    EFI_STATUS                      Status;
    EFI_HANDLE                      Handle;
    SB_SETUP_DATA                   PchPolicyData;
//(EIP134992+)>>
    EFI_CPUID_REGISTER              CpuBrandString[3];
    UINT8                           *PtrCpuBrandString=NULL;
    UINT8                           *CPUversion=NULL;
    UINTN                           Count=0,VersionID=0;
    UINTN                           StrSize;
//(EIP134992+)<<

    ZeroMem(&mDxePlatformVlvPolicy, sizeof(DXE_VLV_PLATFORM_POLICY_PROTOCOL));

    DxePlatformVlvPolicy = &mDxePlatformVlvPolicy;

    GetSbSetupData((VOID*)gRT, &PchPolicyData, FALSE);

    DxePlatformVlvPolicy->MaxInverterPWM = VlvPolicyData->MaxInverterPWM;
    DxePlatformVlvPolicy->MinInverterPWM = VlvPolicyData->MinInverterPWM;

    DxePlatformVlvPolicy->EnableRenderStandby = VlvPolicyData->EnableRenderStandby;
    DxePlatformVlvPolicy->GraphicsPerfAnalyzers = VlvPolicyData->GraphicsPerfAnalyzers;
    DxePlatformVlvPolicy->ForceWake = VlvPolicyData->ForceWake;
    DxePlatformVlvPolicy->PmWeights = VlvPolicyData->PmWeights;
    DxePlatformVlvPolicy->EuControl = VlvPolicyData->EuControl;
    DxePlatformVlvPolicy->PmLock = VlvPolicyData->PmLock;
    DxePlatformVlvPolicy->PavpMode = VlvPolicyData->PavpMode;
    DxePlatformVlvPolicy->DopClockGating = VlvPolicyData->DopClockGating;
    DxePlatformVlvPolicy->IgdPanelFeatures.PFITStatus = VlvPolicyData->PanelScaling;
    DxePlatformVlvPolicy->IgdPanelFeatures.LidStatus= VlvPolicyData->LidStatus;
    DxePlatformVlvPolicy->S0ixSupported = VlvPolicyData->S0ixSupported;
    DxePlatformVlvPolicy->DptfSettings.Clpm = VlvPolicyData->Clpm;
    DxePlatformVlvPolicy->DptfSettings.EnableDppm = VlvPolicyData->EnableDppm;
    DxePlatformVlvPolicy->DptfSettings.EnableDptf = VlvPolicyData->EnableDptf;
    DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp0 = VlvPolicyData->GenericCriticalTemp0;
    DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp1 = VlvPolicyData->GenericCriticalTemp1;
    DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp2 = VlvPolicyData->GenericCriticalTemp2;
    DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp3 = VlvPolicyData->GenericCriticalTemp3;
    DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp4 = VlvPolicyData->GenericCriticalTemp4;
    DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp0 = VlvPolicyData->GenericPassiveTemp0;
    DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp1 = VlvPolicyData->GenericPassiveTemp1;
    DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp2 = VlvPolicyData->GenericPassiveTemp2;
    DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp3 = VlvPolicyData->GenericPassiveTemp3;
    DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp4 = VlvPolicyData->GenericPassiveTemp4;
    DxePlatformVlvPolicy->DptfSettings.LPOEnable = VlvPolicyData->LPOEnable;
    DxePlatformVlvPolicy->DptfSettings.LPOPerformanceControlSetting = VlvPolicyData->LPOPerformanceControlSetting;
    DxePlatformVlvPolicy->DptfSettings.LPOPowerControlSetting = VlvPolicyData->LPOPowerControlSetting;
    DxePlatformVlvPolicy->DptfSettings.LPOStartPState = VlvPolicyData->LPOStartPState;
    DxePlatformVlvPolicy->DptfSettings.LPOStepSize = VlvPolicyData->LPOStepSize;
    DxePlatformVlvPolicy->DptfSettings.ProcCriticalTemp = VlvPolicyData->CriticalThermalTripPoint;
    DxePlatformVlvPolicy->DptfSettings.ProcPassiveTemp = VlvPolicyData->PassiveThermalTripPoint;
    DxePlatformVlvPolicy->DptfSettings.SuperDebug = VlvPolicyData->SuperDebug;
    DxePlatformVlvPolicy->DptfSettings.AmbientTripPointChange = VlvPolicyData->AmbientTripPointChange;

//(EIP134992+)>>
    {
      AsmCpuid (EFI_CPUID_BRAND_STRING1, &CpuBrandString[0].RegEax, &CpuBrandString[0].RegEbx, &CpuBrandString[0].RegEcx, &CpuBrandString[0].RegEdx);
      AsmCpuid (EFI_CPUID_BRAND_STRING2, &CpuBrandString[1].RegEax, &CpuBrandString[1].RegEbx, &CpuBrandString[1].RegEcx, &CpuBrandString[1].RegEdx);
      AsmCpuid (EFI_CPUID_BRAND_STRING3, &CpuBrandString[2].RegEax, &CpuBrandString[2].RegEbx, &CpuBrandString[2].RegEcx, &CpuBrandString[2].RegEdx);
      PtrCpuBrandString = (UINT8*) &CpuBrandString;
      StrSize = AsciiStrSize(PtrCpuBrandString);
      CPUversion = AllocatePool(sizeof(UINT64));
      while(Count < StrSize) {
        if(*(PtrCpuBrandString+Count)=='N' || *(PtrCpuBrandString+Count) == 'J') {
          if(*(PtrCpuBrandString+Count+1)>='0' && *(PtrCpuBrandString+Count+1)<='9') {
            AsciiStrnCpy(CPUversion,PtrCpuBrandString+Count+1,4);
            VersionID = AsciiStrHexToUintn(CPUversion);
            break;
          }
        }
        Count++;
      }
      
      if(*(PtrCpuBrandString+Count) == 'N') {
        switch(VersionID){
        case 0x3510:
        case 0x3520: //EIP155583 
        case 0x3530: //EIP155583 
        case 0x3540: 
          DxePlatformVlvPolicy->DptfSettings.SdpProfile = 1;
          break;
        case 0x2910:
        case 0x2920: //EIP155583 
        case 0x2930: //EIP155583 
        case 0x2940: 
          DxePlatformVlvPolicy->DptfSettings.SdpProfile = 2;
          break;
        case 0x2810:
        case 0x2815: //EIP155583 
        case 0x2820: //EIP155583 
        case 0x2830: //EIP155583 
        case 0x2840: 
          DxePlatformVlvPolicy->DptfSettings.SdpProfile = 3;
          break;
        case 0x2805:
        case 0x2806: //EIP155583 
        case 0x2807: //EIP155583 
        case 0x2808: 
          DxePlatformVlvPolicy->DptfSettings.SdpProfile = 4;
          break;
        default:
          DxePlatformVlvPolicy->DptfSettings.SdpProfile = 0;
        }
      } else if(*(PtrCpuBrandString+Count) == 'J') {
        switch(VersionID){
        case 0x2850:
        case 0x2900: //EIP155583 
          DxePlatformVlvPolicy->DptfSettings.SdpProfile = 5;
          break;
        case 0x1850:
        case 0x1900: //EIP155583 
          DxePlatformVlvPolicy->DptfSettings.SdpProfile = 6;
          break;
        case 0x1750:
        case 0x1800: //EIP155583 
          DxePlatformVlvPolicy->DptfSettings.SdpProfile = 7;
          break;
        default:
          DxePlatformVlvPolicy->DptfSettings.SdpProfile = 0;
        }
      }
    }
//(EIP134992+)<<
    
    if(VlvPolicyData->IgdTurboEn == 2){
        if (PchStepping()>= PchB0){
           DxePlatformVlvPolicy->EnableIGDTurbo = 1;
         } else {
           DxePlatformVlvPolicy->EnableIGDTurbo = 0;
        }
    }  else {
        DxePlatformVlvPolicy->EnableIGDTurbo = VlvPolicyData->IgdTurboEn;
    }

    if ( PchPolicyData.PchAzalia ) {
        DxePlatformVlvPolicy->AudioTypeSupport = HD_AUDIO;
    } else if ( PchPolicyData.Lpe ){
        DxePlatformVlvPolicy->AudioTypeSupport = LPE_AUDIO;
    } else {
        DxePlatformVlvPolicy->AudioTypeSupport = NO_AUDIO;
    }

    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->MaxInverterPWM : %x \n",DxePlatformVlvPolicy->MaxInverterPWM));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->MinInverterPWM : %x \n",DxePlatformVlvPolicy->MinInverterPWM));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->EnableRenderStandby : %x \n",DxePlatformVlvPolicy->EnableRenderStandby));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->GraphicsPerfAnalyzers : %x \n",DxePlatformVlvPolicy->GraphicsPerfAnalyzers));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->ForceWake : %x \n",DxePlatformVlvPolicy->ForceWake));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->PmWeights : %x \n",DxePlatformVlvPolicy->PmWeights));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->EuControl : %x \n",DxePlatformVlvPolicy->EuControl));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->PmLock : %x \n",DxePlatformVlvPolicy->PmLock));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->PavpMode : %x \n",DxePlatformVlvPolicy->PavpMode));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DopClockGating : %x \n",DxePlatformVlvPolicy->DopClockGating));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->ForceWake : %x \n",DxePlatformVlvPolicy->ForceWake));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->IgdPanelFeatures.PFITStatus : %x \n",DxePlatformVlvPolicy->IgdPanelFeatures.PFITStatus));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->IgdPanelFeatures.LidStatus : %x \n",DxePlatformVlvPolicy->IgdPanelFeatures.LidStatus));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->S0ixSupported : %x \n",DxePlatformVlvPolicy->S0ixSupported));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.Clpm : %x \n",DxePlatformVlvPolicy->DptfSettings.Clpm));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.EnableDppm : %x \n",DxePlatformVlvPolicy->DptfSettings.EnableDppm));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.SdpProfile : %x \n",DxePlatformVlvPolicy->DptfSettings.SdpProfile));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.EnableDptf : %x \n",DxePlatformVlvPolicy->DptfSettings.EnableDppm));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp0 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp0));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp1 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp1));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp2 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp2));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp3 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp3));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp4 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericCriticalTemp4));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp0 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp0));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp1 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp1));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp2 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp2));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp3 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp3));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp1 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp1));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp2 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp2));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp3 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp3));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp4 : %x \n",DxePlatformVlvPolicy->DptfSettings.GenericPassiveTemp4));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.LPOEnable : %x \n",DxePlatformVlvPolicy->DptfSettings.LPOEnable));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.LPOPerformanceControlSetting : %x \n",DxePlatformVlvPolicy->DptfSettings.LPOPerformanceControlSetting));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.LPOPowerControlSetting : %x \n",DxePlatformVlvPolicy->DptfSettings.LPOPowerControlSetting));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.LPOStartPState : %x \n",DxePlatformVlvPolicy->DptfSettings.LPOStartPState));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.LPOStepSize : %x \n",DxePlatformVlvPolicy->DptfSettings.LPOStepSize));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.ProcCriticalTemp : %x \n",DxePlatformVlvPolicy->DptfSettings.ProcCriticalTemp));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.ProcPassiveTemp : %x \n",DxePlatformVlvPolicy->DptfSettings.ProcPassiveTemp));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->DptfSettings.SuperDebug : %x \n",DxePlatformVlvPolicy->DptfSettings.SuperDebug));
    DEBUG((EFI_D_INFO, "DxePlatformVlvPolicy->EnableIGDTurbo : %x \n",DxePlatformVlvPolicy->EnableIGDTurbo));     //(EIP115594)

    Handle = NULL;
    Status = gBS->InstallProtocolInterface(
                 &Handle,
                 &gDxeVlvPlatformPolicyGuid,
                 EFI_NATIVE_INTERFACE,
                 DxePlatformVlvPolicy
             );
    ASSERT_EFI_ERROR(Status);

    return Status;

}
