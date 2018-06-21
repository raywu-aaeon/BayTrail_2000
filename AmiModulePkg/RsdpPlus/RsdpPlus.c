//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
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
// Name:  		RsdpPlus.c
//
// Description:	Entry point for RsdpPlus initialization.
//
//<AMI_FHDR_END>
//**********************************************************************

#include <AmiDxeLib.h>
#include <Acpi30.h>
#include <AmiCspLib.h>
#include <Protocol/ManageShadowProtocol.h>

EFI_GUID gManageShadowRamProtocolGuid = MANAGE_SHADOW_RAM_PROTOCOL_GUID;
EFI_GUID gAcpi20TableGuid		= ACPI_20_TABLE_GUID;
EFI_GUID gAcpi11TAbleGuid 	    = ACPI_10_TABLE_GUID;

RSDT_PTR_20 *RSDP = NULL;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: 	RsdpPlusEntryPoint
//
// Description: Entry point for RsdpPlus initialization.
//				Register a ConOutStarted protocol call back function.
//				 
//				
// Input:	EFI_HANDLE			ImageHandle
//			EFI_SYSTEM_TABLE 	*SystemTable
//
// Output:	EFI_STATUS	Status
//      
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS RsdpPlusEntryPoint(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
)
{
	EFI_STATUS	Status = EFI_NOT_FOUND;
	BOOLEAN		ExitToSetupFlag;
	MANAGE_SHADOW_RAM_PROTOCOL	*ManageShadowRamProtocol;
	UINTN DataSize;     
	
	InitAmiLib(ImageHandle,SystemTable);

	RSDP = GetEfiConfigurationTable(pST,&gAcpi20TableGuid);
    if (!RSDP) 
    {
        RSDP = GetEfiConfigurationTable(pST,&gAcpi11TAbleGuid);
    }
    if (!RSDP) return EFI_NOT_FOUND; 
	
	TRACE((-1,"Rsdp Sig [%lx] \n",RSDP->Signature));
	TRACE((-1,"Rsdp XSDT addr [%x] \n",RSDP->XsdtAddr));
	TRACE((-1,"Rsdp RSDT addr [%x] \n",RSDP->RsdtAddr));

	Status = pBS->LocateProtocol(&gManageShadowRamProtocolGuid,NULL,&ManageShadowRamProtocol);
	ASSERT_EFI_ERROR(Status);

	DataSize = sizeof(ExitToSetupFlag);
	Status = pRS->GetVariable(
	             L"Exitflag",
	             &gManageShadowRamProtocolGuid,
	             NULL,
	             &DataSize,
	             &ExitToSetupFlag );

	if(EFI_ERROR(Status))
		Status = ManageShadowRamProtocol->HeapToF000((UINT8*)RSDP,16,(UINTN)sizeof(RSDT_PTR_20), NULL);

	return EFI_SUCCESS;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
