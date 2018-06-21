//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2016, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
/**
 * @file
 * Source file for the PEI driver. This file contains the code to trigger 
 * the SWSMI that will restore the boot scripts into regular memory from 
 * inside of SMM.
 */
#include <AmiPeiLib.h>
#include <Token.h>
#include <AmiAcpiS3.h>
#include <Ppi/SmmControl.h>
#include <Ppi/S3Resume2.h>

/// Private structure used to define the override S3 Resume structure and the original structure
typedef struct{
    EFI_PEI_S3_RESUME2_PPI Ppi; ///< This module's copy of the S3 Resume PPI
    EFI_PEI_S3_RESUME2_PPI *OriginalPpi; ///< The original copy of S3 Resume installed by the system
} S3_RESUME2_PRIVATE;

//PPI to be installed
EFI_PEI_PPI_DESCRIPTOR S3ResumePpiListTemplate = {
    EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gEfiPeiS3Resume2PpiGuid,
    NULL
};

/**
 * Function that attempts to trigger a SWSMI to restore the boot script 
 * tables from SMM memory into regular system memory. This function is 
 * called only when the system is resuming from S3.
 * 
 * @param PeiServices Pointer to the PEI services table 
 * 
 * @return EFI_STATUS
 * @retval EFI_SUCCESS The SwSmi was triggered
 * @retval EFI_NOT_FOUND The PEI_SMM_CONTROL_PPI could not be found
 */
EFI_STATUS TriggerSwSmi(IN CONST EFI_PEI_SERVICES **PeiServices){
    EFI_STATUS Status;
    PEI_SMM_CONTROL_PPI *SmmControl;
    INT8 SmiCommand;
    UINTN Size;
    
    // Trigger boot script restoring SMI

    // What's the best method of SW SMI generation in PEI?
    // We're sticking with SmmControl PPI.
    // If it's not available in your project, replace the code below...
    Status = (*PeiServices)->LocatePpi(
        PeiServices, &gPeiSmmControlPpiGuid, 0, NULL, (VOID **)&SmmControl
    );
    if (EFI_ERROR (Status)){
        PEI_TRACE((TRACE_ALWAYS,PeiServices,"BootScriptHide: ERROR: SmmControl PPI not found. Can't restore the boot script.\n"));
        PEI_TRACE((TRACE_ALWAYS,PeiServices,"If PPI is not available in the project, replace code of the TriggerSwSmi function in BootScriptHidePei.c with the chipset specific SW SMI trigerring code.\n"));
        PEI_TRACE((TRACE_ALWAYS,PeiServices,"  NOTE: Simple write to 0xB2 may not work. You may have to enable SW SMI in one of the SB registers.\n"));
        PEI_TRACE((TRACE_ALWAYS,PeiServices,"  If SW SMI generation succeeds, you should see \"Boot script has been restored\" debug message.\n"));
        ASSERT_PEI_ERROR(PeiServices,EFI_ABORTED);
        return Status;
    }
    SmiCommand = BOOT_SCRIPT_RESTORE_SW_SMI_VALUE;
    Size = sizeof(SmiCommand);
    Status = SmmControl->Trigger(PeiServices, SmmControl, &SmiCommand, &Size, FALSE, 0);
    if (EFI_ERROR (Status)){
        PEI_TRACE((TRACE_ALWAYS,PeiServices,"BootScriptHide: ERROR: SmmControl->Trigger failed with status %r. Can't restore the boot script.\n",Status));
        ASSERT_PEI_ERROR(PeiServices,EFI_ABORTED);
        return Status;
    }
    return EFI_SUCCESS;
}

/**
 * Function called when the system encounters an error while trying to restore the 
 * boot scripts into regular system memory. If this function is called, it is 
 * because there is an error in the system, and S3 resume cannot be completed
 * 
 * @param PeiServices Pointer to the PEI services table
 */
VOID ErrorHandler(IN CONST EFI_PEI_SERVICES **PeiServices){
    // If something went wrong and we were unable to restore the boot script, system is vulnerable.
    // One one to go back to safety is to issue a system reset, which will change boot path from S3 resume to a normal boot.
    PEI_TRACE((TRACE_ALWAYS,PeiServices,"BootScriptHide: Couldn't restore the boot script. Resetting...\n"));
    (*PeiServices)->ResetSystem(PeiServices);
    PEI_TRACE((TRACE_ALWAYS,PeiServices,"BootScriptHide: Couldn't reset. Dead-looping...\n"));
    ASSERT_PEI_ERROR(PeiServices,EFI_ABORTED);
    EFI_DEADLOOP();
}
/**
 * This function can be considered a hook.  This function is used to replace the S3Resume 
 * PPI's RestoreConfig function.  The original S3Resume PPI's RestoreConfig is saved, and 
 * is called at the end of this function. This function will be attempt to trigger the 
 * SWSMI to restore the boot script tables into regular system memory.
 * 
 * @param This Pointer to the EFI_PEI_S3_RESUME2_PPI Protocol
 * 
 * @return EFI_STATUS The return value from running the original RestoreConfig function
 */
EFI_STATUS EFIAPI S3RestoreConfig2(IN EFI_PEI_S3_RESUME2_PPI  *This){
    const CHAR16 AcpiGlobalVariable[] = ACPI_GLOBAL_VARIABLE;
    const EFI_GUID EfiAcpiVariableGuid = EFI_ACPI_VARIABLE_GUID;
    EFI_STATUS Status;
    S3_RESUME2_PRIVATE *S3Resume2Ppi = (S3_RESUME2_PRIVATE*)This;
    EFI_PEI_SERVICES **PeiServices = GetPeiServicesTablePointer();
    ACPI_VARIABLE_SET *AcpiVariableSet;
    UINTN VariableSize = sizeof(AcpiVariableSet);
    EFI_PHYSICAL_ADDRESS    AcpiReservedMemoryBase;
    
    PEI_TRACE((TRACE_ALWAYS,PeiServices,"BootScriptHide: Successfully trapped S3RestoreConfig2 call.\n"));

    Status = PeiGetVariable(PeiServices,AcpiGlobalVariable,&EfiAcpiVariableGuid,NULL,&VariableSize, &AcpiVariableSet);
    if (EFI_ERROR (Status)){
        PEI_TRACE((TRACE_ALWAYS,PeiServices,"BootScriptHide: Can't read variable %S. Status = %r.\n", AcpiGlobalVariable, Status));
        ErrorHandler(PeiServices);
    }
    // We are using AcpiReservedMemoryBase field as a communication mail box between this PEIM 
    // and boot script restoring SMI handler.
    // We are setting the field to BOOT_SCRIPT_SAVE_SW_SMI_VALUE and SMI handler if succeeds sets it to BOOT_SCRIPT_RESTORE_SW_SMI_VALUE.
    // Preserve original AcpiReservedMemoryBase value to restore it once we are done.
    AcpiReservedMemoryBase = AcpiVariableSet->AcpiReservedMemoryBase;
    AcpiVariableSet->AcpiReservedMemoryBase = ~(AcpiVariableSet->AcpiReservedMemoryBase);
    Status = TriggerSwSmi(GetPeiServicesTablePointer());
    if (EFI_ERROR (Status))  ErrorHandler(PeiServices);
    if (AcpiVariableSet->AcpiReservedMemoryBase != BOOT_SCRIPT_RESTORE_SW_SMI_VALUE){
        PEI_TRACE((TRACE_ALWAYS,PeiServices,"BootScriptHide: Something went wrong. SW SMI handler failed to restore the boot script.\n"));
        ErrorHandler(PeiServices);
    }
    
    // Restore original AcpiReservedMemoryBase value.
    AcpiVariableSet->AcpiReservedMemoryBase = AcpiReservedMemoryBase;

    PEI_TRACE((TRACE_ALWAYS,PeiServices,"BootScriptHide: Calling original S3RestoreConfig2\n"));
    return S3Resume2Ppi->OriginalPpi->S3RestoreConfig2(S3Resume2Ppi->OriginalPpi);
}
/**
 * Module entry point for the BootScripeHidePei module. This module does nothing if the system is 
 * not in the S3 resume path. If the system is in the S3 resume path, then the module will use the 
 * installed S3Resume PPI to populate a new copy of the S3Resume PPI that will contain the 
 * S3RestoreConfig2 function instead of the original S3RestoreConfig2 function.
 * 
 * @param FileHandle The file handle associated with this PEIM
 * @param PeiServices Pointer to the PEI Services table
 * 
 * @return EFI_STATUS
 */
EFI_STATUS EFIAPI BootScriptHidePeiEntryPoint (IN EFI_PEI_FILE_HANDLE FileHandle, IN CONST EFI_PEI_SERVICES **PeiServices){
    
    EFI_STATUS Status;
    EFI_BOOT_MODE BootMode;
    EFI_PEI_PPI_DESCRIPTOR *S3ResumePpiList;
    S3_RESUME2_PRIVATE *S3Resume2Ppi;
    EFI_PEI_S3_RESUME2_PPI *OriginalS3Resume2Ppi;
    EFI_PEI_PPI_DESCRIPTOR *OrignalS3Resume2PpiDescriptor;
    
    Status = (*PeiServices)->GetBootMode( PeiServices, &BootMode );
    if ( EFI_ERROR(Status) ||  BootMode != BOOT_ON_S3_RESUME) return EFI_UNSUPPORTED;
    // We can't trigger SW SMI just yet because we can't be sure that it will work.
    // Perhaps SMM initialization is yet to be done by other PEIMs.
    // We need to delay SW SMI generation to a latter point.
    // One one to do it is S3Resume2 PPI hijacking.
    Status = (*PeiServices)->LocatePpi(
        PeiServices, &gEfiPeiS3Resume2PpiGuid, 0, &OrignalS3Resume2PpiDescriptor, (VOID **)&OriginalS3Resume2Ppi
    );
    if ( EFI_ERROR(Status) ) return Status;

    Status = (*PeiServices)->AllocatePool(PeiServices, sizeof(S3ResumePpiListTemplate)+sizeof(*S3Resume2Ppi), &S3ResumePpiList);
    if (EFI_ERROR(Status)) return Status;
    *S3ResumePpiList=S3ResumePpiListTemplate;
    S3Resume2Ppi = (S3_RESUME2_PRIVATE*)(S3ResumePpiList+1);
    S3Resume2Ppi->Ppi.S3RestoreConfig2 = S3RestoreConfig2;
    S3ResumePpiList->Ppi = &S3Resume2Ppi->Ppi;
    S3Resume2Ppi->OriginalPpi = OriginalS3Resume2Ppi;

    Status = (*PeiServices)->ReInstallPpi(PeiServices,OrignalS3Resume2PpiDescriptor,S3ResumePpiList);
    if (EFI_ERROR(Status)){
        PEI_TRACE((TRACE_ALWAYS,PeiServices,"BootScriptHide: Can't replace S3Resume2 PPI.\n"));
        ErrorHandler(PeiServices);
    }
    return Status;
}
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2016, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
