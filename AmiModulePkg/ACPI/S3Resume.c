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

/** @file S3Resume.c
    Restore configuration state from S3 resume.

**/

#include <Pei.h>
#include <Ppi/S3Resume.h>
#include <Ppi/S3Resume2.h>
#include <ppi/BootScriptExecuter.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <AmiPeiLib.h>
#include <token.h>
#include <setup.h>
#include <AmiAcpiS3.h>
#include "AcpiPeiS3Func.h"
#include "AMICSPLIBInc.H"
#include <ACPI50.h>
#include <Library\CpuCspLib.h> //EIP135119

#pragma pack(1)
typedef struct {
    UINT16   GdtLimit;
    UINT64  *GdtBase;
} PTR_GDT_DESCS;
#pragma pack()

extern UINT32 RealModeThunkStart;
extern UINT32 RealModeThunkSize;

EFI_GUID gEfiPeiS3ResumePpiGuid = EFI_PEI_S3_RESUME_PPI_GUID;
//EFI_GUID gEfiPeiS3Resume2PpiGuid = EFI_PEI_S3_RESUME2_PPI_GUID;
EFI_GUID gEfiPeiBootScriptExecuterPpiGuid = EFI_PEI_BOOT_SCRIPT_EXECUTER_PPI_GUID;
EFI_GUID gEfiPeiEndOfPeiPhasePpiGuid = EFI_PEI_END_OF_PEI_PHASE_PPI_GUID;
extern EFI_GUID gEfiPeiReadOnlyVariablePpiGuid;
EFI_GUID gEfiSetupGuid = SETUP_GUID;
extern EFI_GUID gAmiGlobalVariableGuid;
extern EFI_GUID gS3ResumeDonePpiGuid; //EIP149462

CHAR16		gSetupVariable[]	= L"Setup";

VOID RealModeThunk(PTR_GDT_DESCS *GdtDesc, UINT32 Firmware_Waking_Vector, BOOLEAN UseCall);
typedef VOID(*REAL_MODE_THUNK_FUNCTION)(PTR_GDT_DESCS*, UINT32, BOOLEAN);
//Boot Script Executer module initialization routine
EFI_STATUS InitBootScriptExecuter(
//		IN EFI_FFS_FILE_HEADER	*FfsHeader,
//		IN EFI_PEI_SERVICES		**PeiServices
    IN       EFI_PEI_FILE_HANDLE   FileHandle,
    IN CONST EFI_PEI_SERVICES     **PeiServices 
	);
//S3 Resume PPI routine
//Defined in this file
EFI_STATUS S3RestoreConfig(
	IN EFI_PEI_SERVICES **PeiServices
);
EFI_STATUS S3RestoreConfig2(
	IN EFI_PEI_S3_RESUME2_PPI *This
);
//To be copied to memory.
UINT64 gGDT[] = {
	0,					//NULL_SEL
	0x00009a000000ffff,	//CODE_SEL 0x08	 16-bit code selector, limit of 4K
	0x000093000000ffff	//DATA_SEL 0x10, 16-bit data selector Data 0-ffffffff
};

//PPI to be installed
EFI_PEI_S3_RESUME_PPI S3ResumePpi = {S3RestoreConfig};
EFI_PEI_S3_RESUME2_PPI S3ResumePpi2 = {S3RestoreConfig2};

static EFI_PEI_PPI_DESCRIPTOR gPpiList[] = {
  	{(EFI_PEI_PPI_DESCRIPTOR_PPI),
    &gEfiPeiS3ResumePpiGuid,
    &S3ResumePpi},

    {(EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gEfiPeiS3Resume2PpiGuid,
    &S3ResumePpi2}
};

static EFI_PEI_PPI_DESCRIPTOR gEndOfPpiList[] = { 
	EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
	&gEfiPeiEndOfPeiPhasePpiGuid,
    NULL 
};

//EIP149462 >>
static EFI_PEI_PPI_DESCRIPTOR gPpiS3ResumeDoneTable[] = { 
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gS3ResumeDonePpiGuid,
  NULL 
};
//EIP149462 <<

VOID InitLongModeExt(
	IN EFI_PEI_SERVICES **PeiServices,
	IN VOID *Function,
	IN VOID *Parameter1,
	IN VOID *Parameter2,
    IN UINT8 NumMemBits
);

/**
    Restore configuration state from S3 resume.

    @param PeiServices double pointer to Pei Services 

    @retval EFI_STATUS, based on a result

**/
EFI_STATUS S3RestoreConfig(
	IN EFI_PEI_SERVICES **PeiServices
)
{
	EFI_STATUS      Status;
    UINT32          i;
	PTR_GDT_DESCS   *GdtDesc;
	UINT64          *Gdt;

	ACPI_VARIABLE_SET                   *AcpiVariableSet;
	EFI_PEI_BOOT_SCRIPT_EXECUTER_PPI    *BootScript;
	EFI_PHYSICAL_ADDRESS                BootScriptTable;
#if S3_VIDEO_REPOST_SUPPORT == 1
	EFI_PEI_READ_ONLY_VARIABLE2_PPI     *ReadOnlyVariable;
	SETUP_DATA                          SetupData;
	BOOLEAN                             IsSetupDataValid;
	UINTN                               VariableSize=sizeof(SETUP_DATA);
#endif
	void(*X_Firmware_Waking_Vector)(); 
	UINT32 Firmware_Waking_Vector;
    UINTN VarSize = sizeof(UINT32);
    EFI_FPDT_STRUCTURE                 *FpdtVar;

	PEI_PERF_START(PeiServices,S3RESUME_TOK,NULL,0);
    PEI_PROGRESS_CODE(PeiServices,PEI_S3_STARTED);
#if S3_VIDEO_REPOST_SUPPORT == 1
	Status = (*PeiServices)->LocatePpi(
		PeiServices,
		&gEfiPeiReadOnlyVariable2PpiGuid,
		0, NULL,
		&ReadOnlyVariable		
	);
	ASSERT_PEI_ERROR(PeiServices, Status);
#endif

	Status = (*PeiServices)->LocatePpi (
		PeiServices,
		&gEfiPeiBootScriptExecuterPpiGuid,
		0,
		NULL,
		&BootScript
		);
	ASSERT_PEI_ERROR (PeiServices, Status);
    if (EFI_ERROR(Status)){
        PEI_ERROR_CODE(PeiServices, PEI_S3_BOOT_SCRIPT_ERROR, EFI_ERROR_MAJOR);
        return Status;
    }

	AcpiVariableSet = GetAcpiS3Info(PeiServices);
    if (!AcpiVariableSet){
		PEI_TRACE((-1, PeiServices, "Smm S3 Resume -- No Acpi Global Variable\n"));
		PEI_ERROR_CODE(PeiServices, PEI_S3_RESUME_ERROR, EFI_ERROR_MAJOR);
        return EFI_NOT_FOUND;
    }

	BootScriptTable = AcpiVariableSet->AcpiBootScriptTable;

#if S3_VIDEO_REPOST_SUPPORT == 1
	VariableSize = sizeof(SETUP_DATA);
	Status = ReadOnlyVariable->GetVariable(
        ReadOnlyVariable,
		gSetupVariable, &gEfiSetupGuid,
		NULL,
		&VariableSize, 	&SetupData
	);
	IsSetupDataValid = !EFI_ERROR(Status);
#endif

    PEI_PROGRESS_CODE(PeiServices,PEI_S3_BOOT_SCRIPT);
	PEI_TRACE((-1, PeiServices, "Smm S3 Resume -- Executing Boot Script Table.\n"));
	Status = BootScript->Execute(
		PeiServices,
		BootScript,
		BootScriptTable,
		NULL
	);
	if (EFI_ERROR(Status))
	{
		PEI_TRACE((-1, PeiServices, "Smm S3 Resume -- Error executing Boot Script Table.\n"));
		PEI_ERROR_CODE(PeiServices, PEI_S3_BOOT_SCRIPT_ERROR, EFI_ERROR_MAJOR);
	}

//for legacy free systems it will be infinite loop

	//keyboard init
	#define KBC_CMDSTS_PORT 0x64
	#define KBC_DATA_PORT   0x60
	#define KBC_IBF			0x02
    if( IoRead8(KBC_CMDSTS_PORT) != 0xff){
    	IoWrite8(KBC_CMDSTS_PORT, 0xaa);
	    for (;;) {
		    if (!(IoRead8(KBC_CMDSTS_PORT) & KBC_IBF)) {
			    break;
		    }
	    }
	}

	_asm cli

    //allocate memory for GDT and copy to Thunk to 16-bit.
	(*PeiServices)->AllocatePool(PeiServices,sizeof(gGDT), &Gdt);
	
	//Allocate memory for descriptor.
	(*PeiServices)->AllocatePool(PeiServices,sizeof(PTR_GDT_DESCS), &GdtDesc);
	GdtDesc->GdtLimit = sizeof(gGDT)-1;
	GdtDesc->GdtBase = Gdt;	

#if S3_VIDEO_REPOST_SUPPORT == 1
    if (IsSetupDataValid && SetupData.S3ResumeVideoRepost) {
        //The following code executes video option ROM at c000:0003.
        //For the video option ROM, a thunk is needed to 16-bit.
        //The Thunk and the area for 16-bit stack are located starting at 
        //ACPI_THUNK_REAL_MODE_SEGMENT * 16 with a length ACPI_THUNK_STACK_TOP.

        VOID *RealModeThunkSave;

        PEI_PROGRESS_CODE(PeiServices,PEI_S3_VIDEO_REPOST);
        //Allocate memory to perserve the the thunk region.
		Status = (*PeiServices)->AllocatePool(PeiServices, ACPI_THUNK_STACK_TOP, &RealModeThunkSave);
    	ASSERT_PEI_ERROR (PeiServices, Status);

        //Save the thunk and stack region.
        MemCpy(RealModeThunkSave, (VOID*)(ACPI_THUNK_REAL_MODE_SEGMENT * 16), ACPI_THUNK_STACK_TOP);    //Save Region to copy
        //Copy the thunk code.
        MemCpy((VOID*)(ACPI_THUNK_REAL_MODE_SEGMENT * 16), (VOID*)RealModeThunkStart, RealModeThunkSize);

        //Open 0xc0000 region--the video option ROM.
        NBPeiProgramPAMRegisters(
            PeiServices,
            0xc0000,
            0x10000,
            LEGACY_REGION_UNLOCK,
            NULL
        );

        //Call video option rom: C000:0003.
    	for(i = 0; i < sizeof(gGDT)/sizeof(UINT64); ++i) Gdt[i] = gGDT[i];  //Set GDT for Thunk to 16-bit
        ((REAL_MODE_THUNK_FUNCTION)(ACPI_THUNK_REAL_MODE_SEGMENT * 16))(GdtDesc, 0xc0000003, TRUE);

        //Close c000 region
        NBPeiProgramPAMRegisters(
            PeiServices,
            0xc0000,
            0x10000,
            LEGACY_REGION_LOCK,
            NULL
        );

        //Restore region where THUNK and 16-bit stack was copied over.
        MemCpy((VOID*)(ACPI_THUNK_REAL_MODE_SEGMENT * 16), RealModeThunkSave, ACPI_THUNK_STACK_TOP);    //Restore region.
    }
#endif

    (*PeiServices)->InstallPpi(PeiServices, gEndOfPpiList);
    
	//EIP149462 >>
    // Install S3ResumeDonePpi
    (*PeiServices)->InstallPpi(PeiServices, gPpiS3ResumeDoneTable);
    //EIP149462 <<

	PEI_TRACE((-1, PeiServices, "Smm S3 resume -- ACPI Mode Enable.\n"));
	if (!AcpiVariableSet->AcpiFacsTable[0] && \
		!AcpiVariableSet->AcpiFacsTable[1] && \
		!AcpiVariableSet->AcpiFacsTable[2])
	{
		PEI_TRACE((-1, PeiServices, "Smm S3 Resume -- No ACPI FACS Table\n"));
		PEI_ERROR_CODE(PeiServices, PEI_S3_RESUME_ERROR, EFI_ERROR_MAJOR);
	}

	PEI_PERF_END(PeiServices,S3RESUME_TOK,NULL,0);

    //Time  = GetCpuTimer();
    
    Status = PeiGetVariable(
        PeiServices,
        L"FPDT_Variable_NV", &gAmiGlobalVariableGuid,
		NULL, &VarSize, &FpdtVar

    );

    //EIP135119 >>
    WriteMsr(0x20e, (UINT64)0);
    WriteMsr(0x20f, (UINT64)0);
    //EIP135119 <<

    if (!EFI_ERROR (Status))
    {
        if (((PERF_TAB_HEADER*)(UINT8*)(FpdtVar->S3Pointer))->Signature == 0x54503353)
        {
            BASIC_S3_RESUME_PERF_REC    *S3PerRec;
            UINT64                      NanoTime;
            S3PerRec = (BASIC_S3_RESUME_PERF_REC*)((UINT8*)(FpdtVar->S3Pointer) + sizeof(PERF_TAB_HEADER));
            if ((FpdtVar->NanoFreq !=0) && (S3PerRec->Header.PerfRecType == 0)) 
            {
            	NanoTime = Div64 (Mul64 (GetCpuTimer (), (UINTN)FpdtVar->NanoFreq), 1000000 , NULL);
            
                S3PerRec->AverageResume = Div64 ((Mul64 (S3PerRec->AverageResume, (UINTN)(S3PerRec->ResumeCount)) + NanoTime), 
                                                        (UINTN)(S3PerRec->ResumeCount + 1), NULL);
                S3PerRec->FullResume = NanoTime;
                S3PerRec->ResumeCount++;
            
            }
        }
    }
    
	//If given control to X_Firmware_Waking_Vector. It will not return.
	// Search each FACS table for a valid vector
	for (i = 0; i < 3; i++ ) {
		if (!AcpiVariableSet->AcpiFacsTable[i]) continue;
		PEI_TRACE((-1, PeiServices, "Smm S3 Resume -- Trying FACS table #%d.\n", i + 1));
		X_Firmware_Waking_Vector = (void(*)())*(VOID**)((UINT8*)AcpiVariableSet->AcpiFacsTable[i] + 24);
		if (X_Firmware_Waking_Vector)
		{
			PEI_TRACE((-1, PeiServices, "Smm S3 Resume -- Waking in protected mode.\n"));
        	PEI_PROGRESS_CODE(PeiServices,PEI_S3_OS_WAKE);
            PEI_TRACE((-1, PeiServices, "Smm S3 Vector %x.\n", X_Firmware_Waking_Vector));
            if (((UINT32*)AcpiVariableSet->AcpiFacsTable[i] + 36) && 1) 
            {
                PEI_TRACE((-1, PeiServices, "Smm S3 Resume -- Waking in protected mode 64 bit enable.\n"));
                InitLongModeExt (PeiServices, X_Firmware_Waking_Vector, NULL, NULL, 12);
            }
            else
			    X_Firmware_Waking_Vector(); // Will not return	
		}

		Firmware_Waking_Vector = *(UINT32*)((UINT8*)AcpiVariableSet->AcpiFacsTable[i] + 12);
		if (Firmware_Waking_Vector)
		{
    	    UINT32  RealModeSegOff;

			PEI_TRACE((-1, PeiServices, "Smm S3 Resume -- Waking in real mode.\n"));
            PEI_TRACE((-1, PeiServices, "Smm S3 Vector %x.\n", Firmware_Waking_Vector));
    	    RealModeSegOff = ((Firmware_Waking_Vector & ~0xf) << 12) + (Firmware_Waking_Vector & 0xf);

    	    //Execute this thunk from ROM.
    		for(i = 0; i < sizeof(gGDT)/sizeof(UINT64); ++i) Gdt[i] = gGDT[i];  //Set GDT for Thunk to 16-bit
        	PEI_PROGRESS_CODE(PeiServices,PEI_S3_OS_WAKE);
			RealModeThunk(GdtDesc, RealModeSegOff, FALSE);	//Will not return.
		}
	}

	PEI_TRACE((-1, PeiServices, "Smm S3 Resume -- No waking vector.\n"));
	PEI_ERROR_CODE(PeiServices, PEI_S3_OS_WAKE_ERROR, EFI_ERROR_MAJOR);

	return EFI_UNSUPPORTED;
}
/**
    Restore configuration state from S3 resume.

    @param  This pointer to EFI_PEI_S3_RESUME2_PPI

    @retval EFI_STATUS, based on result.

**/
EFI_STATUS S3RestoreConfig2(
	IN EFI_PEI_S3_RESUME2_PPI *This
)
{
    EFI_PEI_SERVICES **PeiServices;

    if (This != &S3ResumePpi2) return EFI_INVALID_PARAMETER;

    PeiServices = GetPeiServicesTablePointer ();
    
    return S3RestoreConfig(PeiServices);

}

EFI_STATUS S3ResumeEntryPoint(
//  IN EFI_FFS_FILE_HEADER       *FfsHeader,
//  IN EFI_PEI_SERVICES          **PeiServices
    IN       EFI_PEI_FILE_HANDLE   FileHandle,
    IN CONST EFI_PEI_SERVICES     **PeiServices 
  )
{
    //Initialize Boot Script Executer module
    InitBootScriptExecuter(FileHandle,PeiServices);
	return (*PeiServices)->InstallPpi(PeiServices,gPpiList);
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
