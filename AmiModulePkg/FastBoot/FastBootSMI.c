//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/PTT/FastBootSMI.c 1     3/11/11 10:07p Bibbyyeh $
//
// $Revision: 1 $
//
// $Date: 3/11/11 10:07p $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  FastBootSMI.c
//
// Description:
// Implementation of fast boot smi functionality
//
//<AMI_FHDR_END>
//*************************************************************************

#include <AmiDxeLib.h>
#include <Token.h>
#include <Setup.h>

#if PI_SPECIFICATION_VERSION >= 0x1000A
#include <Protocol\SmmCpu.h>
#include <Protocol\SmmBase2.h>
#include <Protocol\SmmSwDispatch2.h>
#define RETURN(status) {return status;}
#else
#include <Protocol\SmmBase.h>
#include <Protocol\SmmSwDispatch.h>
#define RETURN(status) {return ;}
#endif

extern EFI_GUID gEfiSmmSwDispatch2ProtocolGuid;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SwSmiHandler
//
// Description: SwSmiHandler
//
// Input: 
// IN EFI_HANDLE                   DispatchHandle,
// IN EFI_SMM_SW_DISPATCH_CONTEXT  *DispatchContext
//
// Output:  
// VOID
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

#if PI_SPECIFICATION_VERSION >= 0x1000A
EFI_STATUS 
SwSmiHandler (
  IN EFI_HANDLE DispatchHandle,
  IN CONST VOID  *Context OPTIONAL,
  IN OUT VOID    *CommBuffer OPTIONAL,
  IN OUT UINTN   *CommBufferSize OPTIONAL )
#else
VOID SwSmiHandler (
  IN EFI_HANDLE                  DispatchHandle,
  IN EFI_SMM_SW_DISPATCH_CONTEXT *DispatchContext )
#endif
{

    EFI_STATUS  Status;
    EFI_GUID    SetupGuid = SETUP_GUID;
    UINTN       SetupDataSize = sizeof(SETUP_DATA);    
    SETUP_DATA  SetupData;
    
    Status = pRS->GetVariable(
                    L"Setup", 
                    &SetupGuid, 
                    NULL, 
                    &SetupDataSize, 
                    &SetupData);
    if(EFI_ERROR(Status)) RETURN(Status);
    
    SetupData.FastBoot = 0;      

    Status = pRS->SetVariable(
                    L"Setup", 
                    &SetupGuid,
                    EFI_VARIABLE_NON_VOLATILE | \
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | \
                    EFI_VARIABLE_RUNTIME_ACCESS,
                    SetupDataSize, 
                    &SetupData);
    
    RETURN(Status);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InSmmFunction
//
// Description: InSmmFunction
//
// Input:
// IN EFI_HANDLE ImageHandle, 
// IN EFI_SYSTEM_TABLE *SystemTable
//
// Output:      
// EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS InSmmFunction( 
    IN EFI_HANDLE ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
    EFI_SMM_SW_REGISTER_CONTEXT     SwSmiContext;
    EFI_SMM_SW_DISPATCH2_PROTOCOL    *SwSmiDispatch;
#else
    EFI_SMM_SW_DISPATCH_CONTEXT     SwSmiContext;
    EFI_SMM_SW_DISPATCH_PROTOCOL    *SwSmiDispatch;
#endif

	EFI_HANDLE	SwSmiHandle;
	EFI_STATUS	Status;

	SwSmiContext.SwSmiInputValue = FAST_BOOT_DISABLE_SWSMI;

#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
	Status = InitAmiSmmLib( ImageHandle, SystemTable );
	if (EFI_ERROR(Status)) return Status;

    Status = pSmstPi->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid, NULL, &SwSmiDispatch);
#else
    Status = pBS->LocateProtocol (&gEfiSmmSwDispatchProtocolGuid, NULL, &SwSmiDispatch);
#endif

	if (EFI_ERROR(Status)) return Status;

	Status = SwSmiDispatch->Register(
		SwSmiDispatch,
		SwSmiHandler,
		&SwSmiContext,
		&SwSmiHandle
	);
   
	return Status;
	
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FastBootSMIEntryPoint
//
// Description: FastBootSMIEntryPoint
//
// Input:       	
// IN EFI_HANDLE		ImageHandle,
// IN EFI_SYSTEM_TABLE	*SystemTable
//
// Output:      
// EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS FastBootSMIEntryPoint(
	IN EFI_HANDLE		ImageHandle,
	IN EFI_SYSTEM_TABLE	*SystemTable
)
{
	InitAmiLib(ImageHandle,SystemTable);

	return InitSmmHandler(ImageHandle, SystemTable, InSmmFunction, NULL);
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************


