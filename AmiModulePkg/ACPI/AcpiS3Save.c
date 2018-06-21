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



/** @file AcpiS3Save.c
    ACPI S3 support functions

**/

#include <Efi.h>
#include <Dxe.h>
#include <Hob.h>
#include <Protocol/BootScriptSave.h>
#include <Protocol/MpService.h>
#include <AmiDxeLib.h>
#include <Acpi20.h>
#include <AmiAcpiS3.h>
#include <token.h>

EFI_BOOT_SCRIPT_SAVE_PROTOCOL   *gBootScriptSave;
EFI_MP_SERVICES_PROTOCOL        *gMpServices;
CHAR16		gAcpiGlobalVariable[] = ACPI_GLOBAL_VARIABLE;

EFI_GUID	gAcpi20TableGuid		= ACPI_20_TABLE_GUID;
EFI_GUID    gAcpi11TAbleGuid 	    = ACPI_10_TABLE_GUID;
//EFI_GUID	gEfiBootScriptSaveGuid	= EFI_BOOT_SCRIPT_SAVE_GUID;
static EFI_GUID gEfiBootScriptSaveGuid = EFI_BOOT_SCRIPT_SAVE_PROTOCOL_GUID;
EFI_GUID	gEfiAcpiVariableGuid	= EFI_ACPI_VARIABLE_GUID;
EFI_GUID	gHobListGuid			= HOB_LIST_GUID;
EFI_GUID    gEfiMpServicesGuid      = EFI_MP_SERVICES_PROTOCOL_GUID;

//Declaration of Boot Script Save module initializarion routine
EFI_STATUS InitBootScript(
	IN EFI_HANDLE ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
	);


/**
    This function returns address of memory where FACS ACPI table resides

         
    @param VOID

          
    @retval Address of FACS table

    @note  
  The routine may fail if the FACS table is in a different location for 
  ACPI 1.0 and ACPI 2.0 (e.g. 1 above 4G and 1 below 4G). WIN98 will read the
  RSDT, and WINXP will read the XSDT. If the XSDT and RSDT aren't pointing to
  the same tables, a S3 resume failure will occur.
  Currently, the variable from Intel only supports one FACS table.

**/

VOID GetAcpiFacsTable(EFI_PHYSICAL_ADDRESS *FacsTable)
{
    RSDT_PTR_20 *RSDP = NULL;
    RSDT_20     *RSDT = NULL;
    XSDT_20     *XSDT = NULL;
    FACP_20     *FADT = 0;
    UINT32      i;
    BOOLEAN     Ver1 = FALSE;

	// Initialize each pointer to 0
	for (i = 0; i < 3; i++) FacsTable[i] = 0;

    RSDP = GetEfiConfigurationTable(pST,&gAcpi20TableGuid);
    if (!RSDP) 
    {
        RSDP = GetEfiConfigurationTable(pST,&gAcpi11TAbleGuid);
        Ver1 = TRUE;
    }
    if (!RSDP) return;   

    RSDT = (RSDT_20*)RSDP->RsdtAddr;    // 32-bit pointer table
    if (!Ver1) XSDT = (XSDT_20*)RSDP->XsdtAddr;    // 64-bit pointer table.

	// Get XSDT FACS Pointers
	if (XSDT) {
        UINT32 NumPtrs = (XSDT->Header.Length - sizeof(ACPI_HDR)) / 8;
        for(i = 0; i < NumPtrs; ++i) {
            if (((ACPI_HDR*)XSDT->Ptrs[i])->Signature == 'PCAF') {
                FADT = (FACP_20*)XSDT->Ptrs[i];
				FacsTable[0] = (EFI_PHYSICAL_ADDRESS)FADT->X_FIRMWARE_CTRL;
				FacsTable[1] = (EFI_PHYSICAL_ADDRESS)FADT->FIRMWARE_CTRL;
                break;
            }
        }
    }

	// Get RSDT FACS Pointer
    if (RSDT) {
        UINT32 NumPtrs = (RSDT->Header.Length - sizeof(ACPI_HDR)) / 4;
        for(i = 0; i < NumPtrs; ++i) {
            if (((ACPI_HDR*)RSDT->Ptrs[i])->Signature == 'PCAF') {
                FADT = (FACP_20*)RSDT->Ptrs[i];
                FacsTable[2] = (EFI_PHYSICAL_ADDRESS)FADT->FIRMWARE_CTRL;
				break;
            }
        }
    } 
}

/**
    This function will be called when ReadyToBoot event will be signaled and 
    will update data, needed for S3 resume control flow.

         
    @param Event signalled event
    @param Context calling context

          
    @retval VOID

**/

VOID CallbackReadyToBoot(
	IN EFI_EVENT	Event,
	IN VOID			*Context
)
{
	EFI_HOB_RESOURCE_DESCRIPTOR	*ResDescHob;
	ACPI_VARIABLE_SET		*AcpiVariableSet;
	EFI_PHYSICAL_ADDRESS    AcpiMemoryBase;
	EFI_PHYSICAL_ADDRESS	ScriptAddress;
	UINT64					SystemMemoryLength;
	VOID					*FirstHob;
	EFI_STATUS				Status;
    UINTN                   NumCpus = 1;

	static BOOLEAN S3ResumeInfo = FALSE;
	if (S3ResumeInfo) return;

    //Get number of CPUs.
	Status = pBS->LocateProtocol(
        &gEfiMpServicesGuid,
        NULL,
        &gMpServices
    );
	ASSERT_EFI_ERROR(Status);
    if (!EFI_ERROR(Status)) {
        UINTN   NumEnCpus;
        Status = gMpServices->GetNumberOfProcessors( gMpServices, &NumCpus, &NumEnCpus );
        ASSERT_EFI_ERROR(Status);
    }

    AcpiVariableSet = (ACPI_VARIABLE_SET*) 0xFFFFFFFF;
	Status = pBS->AllocatePages(
        AllocateMaxAddress,
		EfiACPIMemoryNVS,
		EFI_SIZE_TO_PAGES(sizeof(ACPI_VARIABLE_SET)),
		(EFI_PHYSICAL_ADDRESS*)&AcpiVariableSet
	);

	ASSERT_EFI_ERROR(Status);
	
	pBS->SetMem(AcpiVariableSet, sizeof(ACPI_VARIABLE_SET),0);

	Status = pBS->LocateProtocol(
		&gEfiBootScriptSaveGuid,
		NULL,
		&gBootScriptSave
		);
	ASSERT_EFI_ERROR(Status);

	Status = gBootScriptSave->CloseTable(
		gBootScriptSave,
		EFI_ACPI_S3_RESUME_SCRIPT_TABLE,
		&ScriptAddress);
	ASSERT_EFI_ERROR(Status);

	// Allocate ACPI reserved memory for S3 resume.
    Status = pBS->AllocatePages (
        AllocateAnyPages,
        EfiACPIMemoryNVS,
        EFI_SIZE_TO_PAGES(S3_BASE_MEMORY_SIZE + S3_MEMORY_SIZE_PER_CPU * NumCpus),
        &AcpiMemoryBase
        );
	ASSERT_EFI_ERROR(Status);
	
	// Calculate the system memory length by memory hobs
	SystemMemoryLength = 0x100000;

	FirstHob = GetEfiConfigurationTable(pST,&gHobListGuid);
	if (!FirstHob) ASSERT_EFI_ERROR(EFI_NOT_FOUND);

	ResDescHob = (EFI_HOB_RESOURCE_DESCRIPTOR*) FirstHob;

	//Find APIC ID Hob.
	while (!EFI_ERROR(Status = FindNextHobByType(EFI_HOB_TYPE_RESOURCE_DESCRIPTOR,&ResDescHob)))
	{
		if (ResDescHob->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY)
		{
			if (ResDescHob->PhysicalStart >= 0x100000)
				SystemMemoryLength += ResDescHob->ResourceLength;
		}
	}
	if (SystemMemoryLength == 0x100000) ASSERT_EFI_ERROR(EFI_NOT_FOUND);

	AcpiVariableSet->AcpiReservedMemoryBase	= (EFI_PHYSICAL_ADDRESS)AcpiMemoryBase;
	AcpiVariableSet->AcpiReservedMemorySize	= S3_BASE_MEMORY_SIZE + S3_MEMORY_SIZE_PER_CPU * (UINT32)NumCpus;
	AcpiVariableSet->AcpiBootScriptTable	= (EFI_PHYSICAL_ADDRESS) ScriptAddress;
	AcpiVariableSet->SystemMemoryLength		= SystemMemoryLength;
	GetAcpiFacsTable(&AcpiVariableSet->AcpiFacsTable[0]);

	Status = pRS->SetVariable(
		gAcpiGlobalVariable,
		&gEfiAcpiVariableGuid,
		EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
		sizeof(UINT32),
		&AcpiVariableSet
	);
	//In case BIOS was updated with preserved NVRAM and AcpiGlobalVariable still has runtime attribute
	if (Status == EFI_INVALID_PARAMETER)
	{
		Status = pRS->SetVariable(
				gAcpiGlobalVariable,
				&gEfiAcpiVariableGuid,
				0,
				0,
				NULL
			);
		ASSERT_EFI_ERROR(Status);
	
	
		Status = pRS->SetVariable(
				gAcpiGlobalVariable,
				&gEfiAcpiVariableGuid,
				EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
				sizeof(UINT32),
				&AcpiVariableSet
			);
	}
	
	ASSERT_EFI_ERROR(Status);

	S3ResumeInfo = TRUE;
}

/**
    This function is ACPI S3 driver entry point 

         
    @param ImageHandle Image handle
    @param SystemTable pointer to system table

          
    @retval EFI_SUCCESS if function executed successfully, error otherwise 

    @note  
  This function also creates ReadyToBoot event to save data 
  needed for S3 resume control flow.

**/

EFI_STATUS AcpiS3SaveEntryPoint(
	IN EFI_HANDLE		ImageHandle,
	IN EFI_SYSTEM_TABLE	*SystemTable
	)
{
	EFI_STATUS	Status;

	InitAmiLib(ImageHandle,SystemTable);

    //Initialize Boot Script Save module
    Status = InitBootScript(ImageHandle,SystemTable);

	return Status;
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
