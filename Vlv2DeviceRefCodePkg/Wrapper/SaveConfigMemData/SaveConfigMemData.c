//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:  SaveConfigMemData.c
//
// Description:	Sset memory information in NVRAM for MemoryInit. 
//
//<AMI_FHDR_END>
//**********************************************************************
#include "SaveConfigMemData.h"

EFI_STATUS SaveConfigMemDataEntryPoint(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS          						Status = EFI_NOT_FOUND;
    EFI_PEI_HOB_POINTERS            GuidHob;
    MRC_PARAMS_SAVE_RESTORE					*MrcParamsHob;
    VOID                						*VariableData;
    UINTN               						BufferSize1;
    UINTN               						BufferSize2;

  GuidHob.Raw = GetHobList ();
  if (GuidHob.Raw != NULL) {
    GuidHob.Raw = GetNextGuidHob (&gEfiMemoryConfigDataGuid, GuidHob.Raw);
    if (GuidHob.Raw != NULL) {
    	MrcParamsHob = GET_GUID_HOB_DATA (GuidHob.Guid);
    	BufferSize1 = GET_GUID_HOB_DATA_SIZE(GuidHob.Guid);
    	BufferSize2 = BufferSize1;

	    Status = gBS->AllocatePool (EfiBootServicesData, BufferSize1, (VOID**)&VariableData);
	    ASSERT_EFI_ERROR (Status);

	    Status = gRT->GetVariable (EfiMemoryConfigVariable,
                                &gEfiVlv2VariableGuid,
                                NULL,
                                &BufferSize1,
                                VariableData
                                );
	    ASSERT_EFI_ERROR (Status);

      if (BufferSize1!=BufferSize2 || CompareMem(MrcParamsHob, VariableData, BufferSize2)) {
      	DEBUG((EFI_D_ERROR, "MRC Parameter is not correct in NVRAM.\n"));
        Status = gRT->SetVariable (EfiMemoryConfigVariable,
                                &gEfiVlv2VariableGuid,
                                (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS),
                                BufferSize2,
                                MrcParamsHob
                                );
  	    ASSERT_EFI_ERROR (Status);
      }

      FreePool (VariableData);
      return Status;
    }
  }
	return Status;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
