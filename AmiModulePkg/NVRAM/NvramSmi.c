//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//****************************************************************************

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name: SmiVariable.c
//
// Description: Interface to a subset of EFI Framework protocols using 
// legacy interfaces that will allow external software to access EFI 
// protocols in a legacy environment.
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>


#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include "NvramSmi.h"
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmCommunication.h>


EFI_HANDLE  VarSmiHandle = NULL;

EFI_SMM_BASE2_PROTOCOL      *InternalSmmBase2 = NULL;

EFI_GUID    NvramSmiGuid = NVRAM_SMI_GUID;

VOID        *gNvramBuffer = NULL;
EFI_EVENT   EvtNvramSmi;
VOID        *RegNvramSmi;


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NvramSmiHandler
//
// Description: The SMI handler for Nvram services.
//
// Input:       NONE
//
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS NvramSmiHandler ()
{
    EFI_STATUS       	Status = EFI_DEVICE_ERROR;
    SMI_VARIABLE       *SmmVarBuffer;

    SmmVarBuffer = (SMI_VARIABLE*)gNvramBuffer;

    if (SmmVarBuffer->Signature != NVAR_SIGNATURE){
    	SmmVarBuffer->Status = Status;
    	return EFI_SUCCESS;
    }

    switch (SmmVarBuffer->Subfunction){

        case SMI_SET_VARIABLE:
        //TRACE((-1,"SmiNVRAM Handler. SMI_SET_VARIABLE \n"));
        Status = pRS->SetVariable ( (CHAR16*)((UINT8*)&SmmVarBuffer->VarData + SmmVarBuffer->VarSize),
                                    &SmmVarBuffer->VarGuid,
                                    SmmVarBuffer->VarAttrib,
                                    SmmVarBuffer->VarSize,
                                    &SmmVarBuffer->VarData);
        break;
#if NVRAM_SMI_FULL_PROTECTION == 1
        case SMI_GET_VARIABLE:
        //TRACE((-1,"SmiNVRAM Handler. SMI_GET_VARIABLE \n"));

        Status = pRS->GetVariable ( (CHAR16*) &SmmVarBuffer->VarData,
                                    &SmmVarBuffer->VarGuid,
                                    &SmmVarBuffer->VarAttrib,
                                    &SmmVarBuffer->VarSize,
                                    &SmmVarBuffer->VarData);
        break;

        case SMI_GET_NEXT_VAR_NAME:
        //TRACE((-1,"SmiNVRAM Handler. SMI_GET_NEXT_VAR_NAME \n"));
        Status = pRS->GetNextVariableName ( &SmmVarBuffer->VarSize,
                                    (CHAR16*) &SmmVarBuffer->VarData,
                                    &SmmVarBuffer->VarGuid);
        break;

        case SMI_QUERY_VAR_INFO:
        //TRACE((-1,"SmiNVRAM Handler. SMI_QUERY_VAR_INFO \n"));
        Status = pRS->QueryVariableInfo ( SmmVarBuffer->VarAttrib,
                                    &SmmVarBuffer->MaxVarStorageSize,
                                    &SmmVarBuffer->RemVarStorageSize,
                                    &SmmVarBuffer->MaxVarSize);
        break;
#endif //#if NVRAM_SMI_FULL_PROTECTION == 1
        default:
        SmmVarBuffer->Status = Status;
        return EFI_SUCCESS;
	}
    //TRACE((-1,"SmiNVRAM Handler. END\n"));
    SmmVarBuffer->Status = Status;
    SmmVarBuffer->Subfunction = 0;
    SmmVarBuffer->Signature = ~NVAR_SIGNATURE;

    return EFI_SUCCESS;
}


EFI_STATUS NvramSmiHandler1 (
  IN     EFI_HANDLE                                DispatchHandle,
  IN     CONST VOID                               *Context,
  IN OUT VOID                                      	*CommBuffer,
  IN OUT UINTN                                     	*CommBufferSize
  )
{
    //TRACE((-1,"NvramSmiHandler1\n"));
    if (gNvramBuffer == NULL)
        return EFI_DEVICE_ERROR;
    //TRACE((-1,"NvramSmiHandler11\n"));
    return NvramSmiHandler();

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NvramSmiEntry
//
// Description: Registration of the SMI function.
//
// Input:       EFI_HANDLE          - ImageHandle
//              EFI_SYSTEM_TABLE*   - SystemTable
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS NvramSmiInSmmFunction(
    IN EFI_HANDLE          		ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
)
{
    EFI_STATUS  								Status;
    EFI_SMM_SYSTEM_TABLE2  	*pSmst2           = NULL;
    UINTN  										VariableSize = sizeof(gNvramBuffer);

    TRACE((-1, "In NvramSmi InSmmFunction\n"));

    Status = pRS->GetVariable ( L"NvramSmiBuffer",
                                &NvramSmiGuid,
                                NULL,
                                &VariableSize,
                                &gNvramBuffer );
    if ((EFI_ERROR(Status)) || (gNvramBuffer == NULL)) return EFI_NOT_FOUND;

    Status = SystemTable->BootServices->LocateProtocol (
                                        &gEfiSmmBase2ProtocolGuid,
                                        NULL,
                                        (VOID **)&InternalSmmBase2
                                        );
    ASSERT_EFI_ERROR (Status);

    Status = InternalSmmBase2->GetSmstLocation (InternalSmmBase2, &pSmst2);
    ASSERT_EFI_ERROR (Status);
    ASSERT (pSmst2 != NULL);
    Status = pSmst2->SmiHandlerRegister(NvramSmiHandler1, &NvramSmiGuid, &VarSmiHandle);
    ASSERT_EFI_ERROR (Status);

    return Status;
}

EFI_STATUS
EFIAPI
NvramSmiEntry(
    IN EFI_HANDLE          		 ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    EFI_STATUS Status;

    Status = InitAmiSmmLib (ImageHandle, SystemTable);

    NvramSmiInSmmFunction (ImageHandle, SystemTable);
    ASSERT_EFI_ERROR(Status);

    return Status;
}


//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
