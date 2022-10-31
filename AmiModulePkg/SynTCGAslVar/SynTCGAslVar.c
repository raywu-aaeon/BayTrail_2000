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
//#include <Protocol\SmmSwDispatch.h>
#include <Token.h>
#include <Protocol\Reset.h>

#define IN
#define OUT

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

EFI_STATUS UpdateTcgASLMemAddr(
    VOID );

EFI_STATUS UpdateTcgASLFieldData(
    VOID );
EFI_STATUS Reset_RS_ResetSystem_Handle(
    VOID );

VOID Reset_RS_ResetSystem_Handle_CallBack(
    IN EFI_EVENT ev,
    IN VOID      *ctx );
VOID
EFIAPI
VirtualAddressChangeCallBack (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  );

EFI_STATUS NotInSmmFunction(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    static EFI_EVENT    ReadToBootEvent;
    EFI_STATUS          Status = EFI_SUCCESS;
    
#if defined(USING_SMI) && USING_SMI==0
    InitAmiRuntimeLib(
        ImageHandle, SystemTable, 
            Reset_RS_ResetSystem_Handle_CallBack, 
            VirtualAddressChangeCallBack
    );
#endif

     // First Update the TCG ASL PPI Memroy Address.
    VERIFY_EFI_ERROR( UpdateTcgASLMemAddr() );

    VERIFY_EFI_ERROR( UpdateTcgASLFieldData() );

    return Status;
}

static EFI_RESET_SYSTEM 				OrigResetSys = NULL;

VOID
SyncTCGResetSystem (
	IN EFI_RESET_TYPE	ResetType,
	IN EFI_STATUS    	ResetStatus,
	IN UINTN         	DataSize,
	IN VOID        	    *ResetData OPTIONAL
){
    SyncTCGAsl_ResetSystem();
    if( OrigResetSys ){
        (*OrigResetSys)( ResetType, ResetStatus, DataSize, ResetData );
        return;
    }

    TRACE(( -1, "TCG Reset System Hook Failed\n"));
	return;
}

VOID Reset_RS_ResetSystem_Handle_CallBack(
    IN EFI_EVENT ev,
    IN VOID      *ctx )
{
//    EFI_STATUS  Status = EFI_SUCCESS;

    TRACE((-1, "Enter Reset_RS_ResetSystem_Handle_CallBack(...)\n"));

    if( NULL == pRS->ResetSystem )
        return;

    OrigResetSys = pRS->ResetSystem;

    pRS->ResetSystem = SyncTCGResetSystem;

    return;
}

VOID
EFIAPI
VirtualAddressChangeCallBack (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
{
    EFI_STATUS ConvertGlobalVar();

    ConvertGlobalVar();
    pRS->ConvertPointer (
        0,
        (VOID **) &OrigResetSys
        );
}

EFI_STATUS SynTCGAslVarEntryPoint(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS                   Status;

    InitAmiLib(ImageHandle,SystemTable);
    TRACE((-1, "Enter SynTCGAslVarEntryPoint(...)\n"));

    Status = NotInSmmFunction( ImageHandle, SystemTable );

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
