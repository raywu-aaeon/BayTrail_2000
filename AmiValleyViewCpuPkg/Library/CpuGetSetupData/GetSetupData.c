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
// Revision History
// ----------------
// $Log: $
// 
// 
//*************************************************************************

/** @file GetSetupData.c
    Custom CPU setup data behavior implementation

**/

#include <Setup.h>
#include <PiPei.h>
#include <AmiPeiLib.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Library/CpuPolicy.h>
//#include <Library/ElinkLib.h>
#include <Library/HobLib.h>
#include <Guid/HobList.h>

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

/**
    This function initilize Intel Cpu Policty related setup data 
    from system SetupData variable 

    @param *Services    - Pointer to PeiServices or RuntimeServices
        structure  
    @param *CpuSetupData - Pointer to custom setup data to return

    @param Pei          - Pei flag. If TRUE we are in PEI phase

    @retval VOID

    @note  PORTING REQUIRED
**/

VOID GetCpuSetupData (
    IN VOID                 *Services,
    IN OUT CPU_SETUP_DATA    *CpuSetupData,
    IN BOOLEAN              Pei
)
{
    EFI_STATUS                      Status;
    SETUP_DATA                      SetupData;
    EFI_PEI_SERVICES                **PeiServices = NULL;
    EFI_RUNTIME_SERVICES            *RunServices = NULL;
    EFI_PEI_READ_ONLY_VARIABLE2_PPI *ReadOnlyVariable = NULL;
    UINTN                           VariableSize = sizeof(SETUP_DATA);
//    EFI_GUID                        HobListGuid = HOB_LIST_GUID;
    AMI_CPU_PLATFORM_INFO_HOB        *CpuPlatformInfoHob;
#ifdef AMI_CPU_SETUPDATA_PEI
    VOID                            *Hob;
#else
    EFI_PEI_HOB_POINTERS            GuidHob;
    CPU_SETUP_DATA        *CpuVarInfoHob;
    CHAR16                          CPUPlatformInfoVar[] = L"CPUPlatformInfo";
#endif
    EFI_GUID                        gAmiCpuPlatformInfoHobGuid = AMI_CPU_PLATFORM_INFO_HOB_GUID;

    if(Pei)
      PeiServices = (EFI_PEI_SERVICES **)Services;
    else
      RunServices = (EFI_RUNTIME_SERVICES *)Services;

#ifdef AMI_CPU_SETUPDATA_PEI
    // Found the CpuPlatformInfoHob
    if(Pei) {
        Status = (*PeiServices)->GetHobList(PeiServices, (VOID**)&Hob);
        CpuPlatformInfoHob = (AMI_CPU_PLATFORM_INFO_HOB*)Hob;
        while (!EFI_ERROR(Status = FindNextHobByType(EFI_HOB_TYPE_GUID_EXTENSION, &CpuPlatformInfoHob))) {
          if (guidcmp(&CpuPlatformInfoHob->EfiHobGuidType.Name, &gAmiCpuPlatformInfoHobGuid)==0) {
    	      (*PeiServices)->CopyMem(CpuSetupData, &CpuPlatformInfoHob->CpuPolicyData, sizeof(CPU_SETUP_DATA));
            return;
          }
        }
    }	
#else
    VariableSize = sizeof(CPU_SETUP_DATA);
    Status = RunServices->GetVariable( CPUPlatformInfoVar, \
                                       &gAmiCpuPlatformInfoHobGuid, \
                                       NULL, \
                                       &VariableSize, \
                                       CpuSetupData );
    if(!EFI_ERROR(Status)) {
        return;
    }
    GuidHob.Raw = GetHobList ();
    if (GuidHob.Raw != NULL) {
      GuidHob.Raw = GetNextGuidHob (&gAmiCpuPlatformInfoHobGuid, GuidHob.Raw);
      if (GuidHob.Raw != NULL) {
        CpuVarInfoHob = GET_GUID_HOB_DATA (GuidHob.Guid);
        CopyMem(CpuSetupData, CpuVarInfoHob, sizeof(CPU_SETUP_DATA));
        Status = RunServices->SetVariable( CPUPlatformInfoVar, \
                                           &gAmiCpuPlatformInfoHobGuid, \
                                           EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                                           sizeof(CPU_SETUP_DATA), \
                                           CpuSetupData);
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

    CpuSetupData->CpuPolicyVersion = CPU_POLICY_VERSION; // This value must be updated if
                                          // the structure of CPU_SETUP_DATA 
                                          // is changed.
                                          // Porting required.

    // Update data for CPU Policy
    if(EFI_ERROR(Status)){
      UINT32	RegEbx, Index =1;
      AsmCpuidEx (0xb, Index, NULL, &RegEbx, NULL, NULL);
      CpuSetupData->ActiveProcessorCores = (UINT8)RegEbx;

      CpuSetupData->ProcessorVmxEnable = 0;
      CpuSetupData->ProcessorHtMode = 0;
      CpuSetupData->ExecuteDisableBit = 0;
      CpuSetupData->ProcessorCcxEnable = 0;
      CpuSetupData->ProcessorEistEnable = 0;
      CpuSetupData->CpuidMaxValue = 0;
      CpuSetupData->MlcStreamerPrefetcherEnable = 0;
      CpuSetupData->MlcSpatialPrefetcherEnable = 0;
      CpuSetupData->DCUStreamerPrefetcherEnable = 0;
      CpuSetupData->DCUIPPrefetcherEnable = 0;
      CpuSetupData->TurboModeEnable = 0;
      CpuSetupData->ProcessorXEEnable = 0;
      CpuSetupData->ProcessorXapic = 1;
      CpuSetupData->ProcessorVirtualWireMode = 1;
      CpuSetupData->PackageCState = 0xff;
      CpuSetupData->PsdState = 0xfe; //HW_ALL
      
    } else {
      CpuSetupData->ProcessorVmxEnable = SetupData.VT;
      CpuSetupData->ProcessorHtMode = SetupData.HTD;
      CpuSetupData->ExecuteDisableBit = SetupData.XDBit;
      CpuSetupData->ProcessorCcxEnable = 0;
      CpuSetupData->ProcessorEistEnable = SetupData.EIST;
      CpuSetupData->CpuidMaxValue = SetupData.LimitCpuid;
      CpuSetupData->MlcStreamerPrefetcherEnable = SetupData.MlcStreamerPrefetcher;
      CpuSetupData->MlcSpatialPrefetcherEnable = SetupData.MlcSpatialPrefetcher;
      CpuSetupData->DCUStreamerPrefetcherEnable = SetupData.DcuStreamerPrefetch;
      CpuSetupData->DCUIPPrefetcherEnable = SetupData.DcuIpPrefetch;
      CpuSetupData->TurboModeEnable = SetupData.TurboMode;
      CpuSetupData->ProcessorXEEnable = 0;
      CpuSetupData->ProcessorXapic = SetupData.LocalX2Apic;
      CpuSetupData->ProcessorTDCLimitOverrideEnable = 0;
      CpuSetupData->ProcessorTDPLimitOverrideEnable = 0;
      CpuSetupData->ProcessorTDCLimit = SetupData.ProcessorTDCLimit;
      CpuSetupData->ProcessorTDPLimit = SetupData.ProcessorTDPLimit;
      CpuSetupData->RatioLimit1C = SetupData._1CoreRatioLimit;
      CpuSetupData->RatioLimit2C = SetupData._2CoreRatioLimit;
      CpuSetupData->RatioLimit3C = SetupData._3CoreRatioLimit;
      CpuSetupData->RatioLimit4C = SetupData._4CoreRatioLimit;
      CpuSetupData->ProcessorVirtualWireMode = 1;
	  
	  if (SetupData.ActiveCoreCount == 0){
		   UINT32	RegEbx, Index =1;
		   AsmCpuidEx (0xb, Index, NULL, &RegEbx, NULL, NULL);
           CpuSetupData->ActiveProcessorCores = (UINT8)RegEbx;
	  }
	  else{
          CpuSetupData->ActiveProcessorCores = SetupData.ActiveCoreCount;
	  }
      CpuSetupData->PackageCState = SetupData.PackageCState;
      CpuSetupData->PsdState = SetupData.PsdCoordType;
	  
    }

	//Port this funtion if it is required
    //
	//	  OemPpmPolicy(SetupData);
	//

    if(Pei){
        Status = (*PeiServices)->CreateHob(
                    PeiServices,
                    EFI_HOB_TYPE_GUID_EXTENSION,
                    sizeof(AMI_CPU_PLATFORM_INFO_HOB),
                    &CpuPlatformInfoHob);

        if (!EFI_ERROR(Status)) {
            (*PeiServices)->SetMem((VOID *)&(CpuPlatformInfoHob->CpuPolicyData), (sizeof(AMI_CPU_PLATFORM_INFO_HOB) - sizeof(EFI_HOB_GUID_TYPE)), 0);
            CpuPlatformInfoHob->EfiHobGuidType.Name = gAmiCpuPlatformInfoHobGuid;
        }

        (*PeiServices)->CopyMem(&CpuPlatformInfoHob->CpuPolicyData, CpuSetupData, sizeof(CPU_SETUP_DATA));

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
