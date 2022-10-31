//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1987-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//**********************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
// Revision History
// ----------------
// $Log: $
// 
//**********************************************************************

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name:        CpuS3Pei.c
//
// Description:
//  CPU SMM relocation for S3 resume.
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <CpuS3Pei.h>

EFI_GUID gEfiPeiBootScriptExecuterPpiGuid = EFI_PEI_BOOT_SCRIPT_EXECUTER_PPI_GUID;
VOID WriteEbp(UINT32 NewVal);

#define  DISTANCE_TO_SAVED_STACKADDR    0x10

#pragma pack(1)
typedef union {
  struct {
    UINT32  LimitLow    : 16;
    UINT32  BaseLow     : 16;
    UINT32  BaseMid     : 8;
    UINT32  Type        : 4;
    UINT32  System      : 1;
    UINT32  Dpl         : 2;
    UINT32  Present     : 1;
    UINT32  LimitHigh   : 4;
    UINT32  Software    : 1;
    UINT32  Reserved    : 1;
    UINT32  DefaultSize : 1;
    UINT32  Granularity : 1;
    UINT32  BaseHigh    : 8;
  } Bits;
  UINT64  Uint64;
} IA32_GDT;
#pragma pack()

//
// Global Descriptor Table (GDT)
//
GLOBAL_REMOVE_IF_UNREFERENCED IA32_GDT mGdtEntries[] = {
/* selector { Global Segment Descriptor                              } */
/* 0x00 */  {{0,      0,  0,  0,    0,  0,  0,  0,    0,  0, 0,  0,  0}},
/* 0x08 */  {{0,      0,  0,  0,    0,  0,  0,  0,    0,  0, 0,  0,  0}},
/* 0x10 */  {{0xFFFF, 0,  0,  0xB,  1,  0,  1,  0xF,  0,  0, 1,  1,  0}},
/* 0x18 */  {{0xFFFF, 0,  0,  0x3,  1,  0,  1,  0xF,  0,  0, 1,  1,  0}},
/* 0x20 */  {{0,      0,  0,  0,    0,  0,  0,  0,    0,  0, 0,  0,  0}},
/* 0x28 */  {{0xFFFF, 0,  0,  0xB,  1,  0,  1,  0xF,  0,  0, 0,  1,  0}},
/* 0x30 */  {{0xFFFF, 0,  0,  0x3,  1,  0,  1,  0xF,  0,  0, 0,  1,  0}},
/* 0x38 */  {{0xFFFF, 0,  0,  0xB,  1,  0,  1,  0xF,  0,  1, 0,  1,  0}},
/* 0x40 */  {{0,      0,  0,  0,    0,  0,  0,  0,    0,  0, 0,  0,  0}},
};

//
// IA32 Gdt register
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST IA32_DESCRIPTOR mGdt = {
  sizeof (mGdtEntries) - 1,
  (UINTN) mGdtEntries
  };

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: ReturnEntryPoint
//
// Description: Function for return to AmiCpuS3PeiEntry entry point.
//
// Input:
//      IN UINT32		*pStack
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
ReturnEntryPoint (
  IN UINT32		*pStack
  )
{
  DEBUG ((EFI_D_ERROR, "ReturnEntryPoint()\n"));
  WriteEbp((UINT32)pStack);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: S3ResumeCPU
//
// Description: Relocation smm base.
//
// Input:
//      VOID
//
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
EFIAPI
S3ResumeCPU (
  VOID
  )
{
  EFI_STATUS						Status;
  PEI_SMM_ACCESS_PPI                *SmmAccess;
  UINTN                             Index;
  ACPI_S3_CONTEXT                   *AcpiS3Context=NULL;
  EFI_PEI_READ_ONLY_VARIABLE2_PPI   *VariableServices;
  EFI_PHYSICAL_ADDRESS              TempEfiBootScriptExecutorVariable;
  EFI_PHYSICAL_ADDRESS              TempAcpiS3Context;
  BOOT_SCRIPT_EXECUTOR_VARIABLE     *EfiBootScriptExecutorVariable;
  UINTN                             VarSize;
  EFI_SMRAM_DESCRIPTOR              *SmramDescriptor;
  SMM_S3_RESUME_STATE               *SmmS3ResumeState;
  VOID                              *GuidHob;
  UINT32							*pStack;
  UINT32							SavedStackAddr;
	  
  __asm {
     mov    SavedStackAddr, ebp     //Save Stack address 
  }
  pStack = (UINT32*)SavedStackAddr;

  Status = PeiServicesLocatePpi (
                            &gPeiSmmAccessPpiGuid,
                            0,
                            NULL,
                            (VOID **) &SmmAccess
                            );
  for (Index = 0; !EFI_ERROR (Status); Index++) {
    Status = SmmAccess->Open ((EFI_PEI_SERVICES **)GetPeiServicesTablePointer (), SmmAccess, Index);
  }

  Status = PeiServicesLocatePpi (
                            &gEfiPeiReadOnlyVariable2PpiGuid,
                            0,
                            NULL,
                            (VOID **) &VariableServices
                            );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  VarSize = sizeof (EFI_PHYSICAL_ADDRESS);
  Status = RestoreLockBox (
             &gEfiAcpiVariableGuid,
             &TempAcpiS3Context,
             &VarSize
             );
  ASSERT_EFI_ERROR (Status);
  
  AcpiS3Context = (ACPI_S3_CONTEXT *)(UINTN)TempAcpiS3Context;
  ASSERT (AcpiS3Context != NULL);
  Status = RestoreLockBox (
             &gEfiAcpiS3ContextGuid,
             NULL,
             NULL
             );
  ASSERT_EFI_ERROR (Status);

  Status = PeiServicesLocatePpi (
                            &gEfiPeiBootScriptExecuterPpiGuid,
                            0,
                            NULL,
                            (VOID **) &TempEfiBootScriptExecutorVariable
                            );


  EfiBootScriptExecutorVariable = (BOOT_SCRIPT_EXECUTOR_VARIABLE *) (UINTN) TempEfiBootScriptExecutorVariable;

  DEBUG (( EFI_D_ERROR, "AcpiS3Context = %x\n", AcpiS3Context));
  DEBUG (( EFI_D_ERROR, "Waking Vector = %x\n", ((EFI_ACPI_2_0_FIRMWARE_ACPI_CONTROL_STRUCTURE *) ((UINTN) (AcpiS3Context->AcpiFacsTable)))->FirmwareWakingVector));
  DEBUG (( EFI_D_ERROR, "AcpiS3Context->AcpiFacsTable = %x\n", AcpiS3Context->AcpiFacsTable));
  DEBUG (( EFI_D_ERROR, "AcpiS3Context->S3NvsPageTableAddress = %x\n", AcpiS3Context->S3NvsPageTableAddress));
  DEBUG (( EFI_D_ERROR, "AcpiS3Context->S3DebugBufferAddress = %x\n", AcpiS3Context->S3DebugBufferAddress));
  DEBUG (( EFI_D_ERROR, "EfiBootScriptExecutorVariable->BootScriptExecutorEntrypoint = %x\n", EfiBootScriptExecutorVariable->BootScriptExecutorEntrypoint));

  //
  // Additional step for BootScript integrity - we only handle BootScript and BootScriptExecutor.
  // Script dispatch image and context (parameter) are handled by platform.
  // We just use restore all lock box in place, no need restore one by one.
  //
  Status = RestoreAllLockBoxInPlace ();
  ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR (Status)) {
    // Something wrong
    CpuDeadLoop ();
  }

  //
  // Attempt to use content from SMRAM first
  //
  GuidHob = GetFirstGuidHob (&gEfiAcpiVariableGuid);
  if (GuidHob != NULL) {
    SmramDescriptor = (EFI_SMRAM_DESCRIPTOR *) GET_GUID_HOB_DATA (GuidHob);
    SmmS3ResumeState = (SMM_S3_RESUME_STATE *)(UINTN)SmramDescriptor->CpuStart;
    SmmS3ResumeState->ReturnCs           = AsmReadCs ();
    SmmS3ResumeState->ReturnEntryPoint   = (EFI_PHYSICAL_ADDRESS)(UINTN)ReturnEntryPoint;
    SmmS3ResumeState->ReturnContext1     = (EFI_PHYSICAL_ADDRESS)(UINTN)pStack;
    SmmS3ResumeState->ReturnContext2     = 0;
	//To prevent ruining SavedStackAddr content, we add distance between ReturnStackPointer and SavedStackAddr.
    SmmS3ResumeState->ReturnStackPointer = (EFI_PHYSICAL_ADDRESS)(UINTN)(SavedStackAddr - DISTANCE_TO_SAVED_STACKADDR);



    DEBUG (( EFI_D_ERROR, "SMM S3 Signature                = %x\n", SmmS3ResumeState->Signature));
    DEBUG (( EFI_D_ERROR, "SMM S3 Stack Base               = %x\n", SmmS3ResumeState->SmmS3StackBase));
    DEBUG (( EFI_D_ERROR, "SMM S3 Stack Size               = %x\n", SmmS3ResumeState->SmmS3StackSize));
    DEBUG (( EFI_D_ERROR, "SMM S3 Resume Entry Point       = %x\n", SmmS3ResumeState->SmmS3ResumeEntryPoint));
    DEBUG (( EFI_D_ERROR, "SMM S3 CR0                      = %x\n", SmmS3ResumeState->SmmS3Cr0));
    DEBUG (( EFI_D_ERROR, "SMM S3 CR3                      = %x\n", SmmS3ResumeState->SmmS3Cr3));
    DEBUG (( EFI_D_ERROR, "SMM S3 CR4                      = %x\n", SmmS3ResumeState->SmmS3Cr4));
    DEBUG (( EFI_D_ERROR, "SMM S3 Return CS                = %x\n", SmmS3ResumeState->ReturnCs));
    DEBUG (( EFI_D_ERROR, "SMM S3 Return Entry Point       = %x\n", SmmS3ResumeState->ReturnEntryPoint));
    DEBUG (( EFI_D_ERROR, "SMM S3 Return Context1          = %x\n", SmmS3ResumeState->ReturnContext1));
    DEBUG (( EFI_D_ERROR, "SMM S3 Return Context2          = %x\n", SmmS3ResumeState->ReturnContext2));
    DEBUG (( EFI_D_ERROR, "SMM S3 Return Stack Pointer     = %x\n", SmmS3ResumeState->ReturnStackPointer));
    DEBUG (( EFI_D_ERROR, "SMM S3 Smst                     = %x\n", SmmS3ResumeState->Smst));
	
    //
    // Disable interrupt of Debug timer.
    //
    SaveAndSetDebugTimerInterrupt (FALSE);

    if (SmmS3ResumeState->Signature == SMM_S3_RESUME_SMM_32) {
        SwitchStack (
        (SWITCH_STACK_ENTRY_POINT)(UINTN)SmmS3ResumeState->SmmS3ResumeEntryPoint,
        (VOID *)AcpiS3Context,
        0,
        (VOID *)(UINTN)(SmmS3ResumeState->SmmS3StackBase + SmmS3ResumeState->SmmS3StackSize)
        );
    }
    if (SmmS3ResumeState->Signature == SMM_S3_RESUME_SMM_64) {
      //
      // Switch to long mode to complete resume.
      //

      //
      // Need to make sure the GDT is loaded with values that support long mode and real mode.
      //
      AsmWriteGdtr (&mGdt);
      AsmWriteCr3 ((UINTN)SmmS3ResumeState->SmmS3Cr3);
      AsmEnablePaging64 (
        0x38,
        SmmS3ResumeState->SmmS3ResumeEntryPoint,
        (UINT64)(UINTN)AcpiS3Context,
        0,
        SmmS3ResumeState->SmmS3StackBase + SmmS3ResumeState->SmmS3StackSize
        );
    }
  }

  return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure: AmiCpuS3PeiEntry
//
// Description: Entry point.
//
// Input:
//      IN EFI_PEI_FILE_HANDLE       FileHandle,
//		IN CONST EFI_PEI_SERVICES    **PeiServices
//
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
EFIAPI
AmiCpuS3PeiEntry (
  IN EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  )
{
  EFI_STATUS			Status;
  EFI_BOOT_MODE			BootMode = 0;
  PEI_SMM_ACCESS_PPI	*SmmAccess;
  UINTN					Index = 0;
  VOID					*GuidHob;
  
  DEBUG ((EFI_D_ERROR, "AmiCpuS3PeiEntry\n"));
  (*PeiServices)->GetBootMode(PeiServices, &BootMode);
      
  if (BootMode != BOOT_ON_S3_RESUME) 
	return EFI_SUCCESS;

  __asm {
	pushad
  }
  
  S3ResumeCPU();
  
  __asm {
	popad
  }
  
  GuidHob = GetFirstGuidHob (&gEfiAcpiVariableGuid);
  if (GuidHob != NULL) {
    //
    // Last step for SMM - send SMI for initialization
    //

    //
    // Send SMI to APs
    //    
    SendSmiIpiAllExcludingSelf ();
    //
    // Send SMI to BSP
    //
    SendSmiIpi (GetApicId ());

    Status = PeiServicesLocatePpi (
                              &gPeiSmmAccessPpiGuid,
                              0,
                              NULL,
                              (VOID **) &SmmAccess
                              );

    DEBUG ((EFI_D_ERROR, "Close all SMRAM regions before executing boot script\n"));

    for (Index = 0, Status = EFI_SUCCESS; !EFI_ERROR (Status); Index++) {
      Status = SmmAccess->Close ((EFI_PEI_SERVICES **)GetPeiServicesTablePointer (), SmmAccess, Index);
    }

    DEBUG ((EFI_D_ERROR, "Lock all SMRAM regions before executing boot script\n"));

    for (Index = 0, Status = EFI_SUCCESS; !EFI_ERROR (Status); Index++) {
      Status = SmmAccess->Lock ((EFI_PEI_SERVICES **)GetPeiServicesTablePointer (), SmmAccess, Index);
    }
  }
  
  DEBUG ((EFI_D_ERROR, "AmiCpuS3PeiEntry end\n"));

  return EFI_SUCCESS;
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1987-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
