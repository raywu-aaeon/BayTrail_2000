//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2009, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/BIN/Modules/ACPI/Template/S3Support/S3Restore/AcpiPeiS3Func.c 1     2/03/11 4:09p Oleksiyy $
//
// $Revision: 1 $
//
// $Date: 2/03/11 4:09p $
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:    AcpiPeiS3Func.c
//
//  Description: ACPI S3 PEI support functions
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <EFI.h>
#include <Pei.h>
//#include <Ppi/ReadOnlyVariable.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <AmiPeiLib.h>
#include <AmiAcpiS3.h>

CHAR16		gAcpiGlobalVariable[]	= ACPI_GLOBAL_VARIABLE;
EFI_GUID	gEfiAcpiVariableGuid	= EFI_ACPI_VARIABLE_GUID;

//EFI_GUID gEfiPeiReadOnlyVariablePpiGuid = EFI_PEI_READ_ONLY_VARIABLE_PPI_GUID;


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   GetAcpiS3Info
//
//  Description:
//  This function reads ACPI_VARIABLE_SET data from NVRAM and returns pointer to it 
//
//  Input:
//  IN EFI_PEI_SERVICES **PeiServices - pointer to pointer to PEI services
//
//  Output:
//  ACPI_VARIABLE_SET* - pointer to ACPI_VARIABLE_SET structure (NULL if error occured)
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

ACPI_VARIABLE_SET * GetAcpiS3Info(
	IN EFI_PEI_SERVICES **PeiServices
)
{
	EFI_PEI_READ_ONLY_VARIABLE2_PPI		*ReadOnlyVariable;
	ACPI_VARIABLE_SET					*AcpiVariableSet = NULL;

	UINTN		VariableSize = sizeof(ACPI_VARIABLE_SET*);
	EFI_STATUS	Status;

	Status = (*PeiServices)->LocatePpi(
		PeiServices,
		&gEfiPeiReadOnlyVariable2PpiGuid,//&gEfiPeiReadOnlyVariablePpiGuid,
		0,
		NULL,
		&ReadOnlyVariable		
	);
	ASSERT_PEI_ERROR(PeiServices, Status);

	Status = ReadOnlyVariable->GetVariable(
		ReadOnlyVariable,//PeiServices,
		gAcpiGlobalVariable,
		&gEfiAcpiVariableGuid,
		NULL,
		&VariableSize,
		&AcpiVariableSet
	);
	if (EFI_ERROR(Status)) return NULL;
	return AcpiVariableSet;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2009, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

