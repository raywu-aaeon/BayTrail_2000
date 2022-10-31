//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2009, American Megatrends, Inc.        **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**   5555 Oakbrook Pkwy, Building 200,Norcross, Georgia 30093  **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//


//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	SynTCGAslVar.c
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************

#include <EFI.h>
#include <AmiDxeLib.h>
#include "SynTCGAslVar.h"

#include <Token.h>
#include <Protocol\Reset.h>


#if PI_SPECIFICATION_VERSION >= 0x1000A
#include <Protocol/SmmCpu.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>
#else
#include <Protocol/SmmBase.h>
#include <Protocol/SmmSwDispatch.h>
#endif

#if PI_SPECIFICATION_VERSION >= 0x1000A
EFI_GUID gEfiSmmCpuProtocolGuid = EFI_SMM_CPU_PROTOCOL_GUID;
EFI_SMM_BASE2_PROTOCOL          *gSmmBase2;
EFI_SMM_CPU_PROTOCOL            *gSmmCpu;
#else
EFI_GUID gEfiSmmSwDispatchProtocolGuid = EFI_SMM_SW_DISPATCH_PROTOCOL_GUID;
#endif

extern EFI_GUID gEfiGlobalVariableGuid;

#if PI_SPECIFICATION_VERSION >= 0x1000A
EFI_STATUS 
SMI_Emul_ResetSystem (
        IN  EFI_HANDLE                  DispatchHandle,
        IN CONST VOID                   *Context OPTIONAL,
        IN OUT VOID                     *CommBuffer OPTIONAL,
        IN OUT UINTN                    *CommBufferSize OPTIONAL
)
#else
VOID SMI_Emul_ResetSystem (
    IN  EFI_HANDLE                  DispatchHandle,
    IN  EFI_SMM_SW_DISPATCH_CONTEXT *DispatchContext
)
#endif
{
    TRACE((-1, "Enter SMI_Emul_ResetSystem()\n"));
    SyncTCGAsl_ResetSystem();
#if PI_SPECIFICATION_VERSION >= 0x1000A
    return EFI_SUCCESS;
#endif
}

EFI_STATUS InSmmFunction(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_HANDLE                      Handle;
    EFI_STATUS                      Status;
    
#if PI_SPECIFICATION_VERSION >= 0x1000A
    EFI_SMM_SW_DISPATCH2_PROTOCOL    *pSwDispatch = NULL;
    EFI_SMM_SW_REGISTER_CONTEXT      SwContext = {SYNN};
#else
    EFI_SMM_SW_DISPATCH_PROTOCOL    *pSwDispatch;
    EFI_SMM_SW_DISPATCH_CONTEXT     SwContext = {SYNN};
#endif
    
#if PI_SPECIFICATION_VERSION >= 0x1000A
    Status = InitAmiSmmLib( ImageHandle, SystemTable );
    
    Status = pBS->LocateProtocol(&gEfiSmmBase2ProtocolGuid, NULL, &gSmmBase2);
    
    if (EFI_ERROR(Status)) return EFI_SUCCESS;

    Status = pSmmBase->GetSmstLocation (gSmmBase2, &pSmst);
    if (EFI_ERROR(Status)) return EFI_SUCCESS;


    Status = pSmst->SmmLocateProtocol( \
                        &gEfiSmmSwDispatch2ProtocolGuid, NULL, &pSwDispatch);
    if (EFI_ERROR(Status)) return EFI_SUCCESS;


    Status = pSmst->SmmLocateProtocol(&gEfiSmmCpuProtocolGuid, NULL, &gSmmCpu);
    if (EFI_ERROR(Status)) return EFI_SUCCESS;

#else
    VERIFY_EFI_ERROR(pBS->LocateProtocol(
                          &gEfiSmmSwDispatchProtocolGuid, NULL, &pSwDispatch));
#endif

    TRACE((TRACE_ALWAYS, "SMIFlash: Registering SMI_Emul_ResetSystem SMI B2[0x%x]\n", SYNN));

//    SwContext.SwSmiInputValue = ASL_SMI_TRigNum;
    Status                    = pSwDispatch->Register( pSwDispatch,
                                                       SMI_Emul_ResetSystem,
                                                       &SwContext,
                                                       &Handle );
    ASSERT_EFI_ERROR( Status );
    return Status;
}

EFI_STATUS SynTCGAslVarEntryPoint(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS                   Status;

    InitAmiLib(ImageHandle,SystemTable);
    TRACE((-1, "Enter SynTCGAslVarSMMEntryPoint(...)\n"));

    Status = InSmmFunction( ImageHandle, SystemTable );

    return EFI_SUCCESS;
}
//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2009, American Megatrends, Inc.        **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**   5555 Oakbrook Pkwy, Building 200,Norcross, Georgia 30093  **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//
