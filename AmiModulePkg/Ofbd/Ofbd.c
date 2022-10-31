//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
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
// $Header: /AptioV/Source/Modules/Ofbd/Ofbd.c $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	Ofbd.c
//
// Description: 
// This is the main file of OFBD module. We setup OFBD SMI handler here. Please also refer to Aptio SMM Module Porting
// guide.
//
//<AMI_FHDR_END>
//**********************************************************************
#include <Efi.h>
#include <Token.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#if PI_SPECIFICATION_VERSION >= 0x1000A
#include <Protocol/SmmCpu.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>
#define RETURN(status) {return status;}
#else
#include <Protocol/SmmBase.h>
#include <Protocol/SmmSwDispatch.h>
#define RETURN(status) {return ;}
#endif
#include <Protocol/DevicePath.h>
#include <AmiSmm.h>
#include <OfbdFuncInc.h>	//Build directory
#include <OfbdFuncElinks.h>	//Build directory

EFI_GUID gSwSmiCpuTriggerGuid = SW_SMI_CPU_TRIGGER_GUID;
#if PI_SPECIFICATION_VERSION >= 0x1000A
EFI_GUID gEfiSmmCpuProtocolGuid = EFI_SMM_CPU_PROTOCOL_GUID;
EFI_SMM_BASE2_PROTOCOL          *gSmmBase2;
EFI_SMM_CPU_PROTOCOL            *gSmmCpu;
#else
EFI_GUID gEfiSmmSwDispatchProtocolGuid = EFI_SMM_SW_DISPATCH_PROTOCOL_GUID;
#endif

static OFBD_INIT_PARTS_FUNC * OFBDInitPartsTbl[] =
{
    OFBD_INIT_FUNC_LIST
    NULL
};

static OFBD_INIT_SMM_FUNC * OFBDInSMMFuncTbl[] =
{
    OFBD_IN_SMM_FUNC_LIST
    NULL
};

static OFBD_INIT_SMM_FUNC * OFBDNotInSMMFunc[] =
{
    OFBD_NOT_SMM_FUNC_LIST
    NULL
};

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   OFBDSMIHandler
//
// Description: OFBD SMI Handlers.
//
// Input:
//      IN	EFI_HANDLE					DispatchHandle,
//      IN	EFI_SMM_SW_DISPATCH_CONTEXT	*DispatchContext
// Output:
//      EFI_STATUS
//
//<AMI_PHDR_END>
//**********************************************************************
#if PI_SPECIFICATION_VERSION >= 0x1000A
EFI_STATUS 
OFBDSMIHandler (
        IN  EFI_HANDLE                  DispatchHandle,
		IN CONST VOID                   *Context OPTIONAL,
		IN OUT VOID                     *CommBuffer OPTIONAL,
		IN OUT UINTN                    *CommBufferSize OPTIONAL
)
#else
VOID OFBDSMIHandler (
    IN  EFI_HANDLE                  DispatchHandle,
    IN  EFI_SMM_SW_DISPATCH_CONTEXT *DispatchContext
)
#endif
{
    EFI_STATUS  Status = EFI_SUCCESS;
#if PI_SPECIFICATION_VERSION < 0x1000A    
    EFI_SMM_CPU_SAVE_STATE	*pCpuSaveState = NULL;
    SW_SMI_CPU_TRIGGER		*SwSmiCpuTrigger = NULL;
#endif
    UINTN		i = 0;
    UINTN       Cpu = /*pSmstPi->CurrentlyExecutingCpu - 1;*/(UINTN)-1;
    UINT8		Data = 0;
    UINT64		BuffAddr = 0;
    UINT32		HighBufferAddress = 0;
    UINT32		LowBufferAddress = 0;
    OFBD_HDR    *OFBDHeader = NULL;
    UINT8       OFBDDataHandled = 0;

#if PI_SPECIFICATION_VERSION >= 0x1000A

	Cpu = ((EFI_SMM_SW_CONTEXT*)CommBuffer)->SwSmiCpuIndex;
    //
    // Found Invalid CPU number, return 
    //
    if(Cpu == (UINTN)-1) RETURN(Status);

    Status = gSmmCpu->ReadSaveState ( gSmmCpu, \
                                      4, \
                                      EFI_SMM_SAVE_STATE_REGISTER_RBX, \
                                      Cpu, \
                                      &LowBufferAddress );
    Status = gSmmCpu->ReadSaveState ( gSmmCpu, \
                                      4, \
                                      EFI_SMM_SAVE_STATE_REGISTER_RCX, \
                                      Cpu, \
                                      &HighBufferAddress );
                                                                                                      
    Data = ((EFI_SMM_SW_CONTEXT*)CommBuffer)->CommandPort;
    	
#else
    for (i = 0; i < pSmstPi->NumberOfTableEntries; ++i) {
        if (guidcmp(&pSmstPi->SmmConfigurationTable[i].VendorGuid, \
                                        &gSwSmiCpuTriggerGuid) == 0) {
            break;
        }
    }

    //If found table, check for the CPU that caused the software Smi.
    if (i != pSmstPi->NumberOfTableEntries) {
        SwSmiCpuTrigger = pSmstPi->SmmConfigurationTable[i].VendorTable;
        Cpu = SwSmiCpuTrigger->Cpu;
    }

    Data = (UINT8)DispatchContext->SwSmiInputValue;

    pCpuSaveState = (EFI_SMM_CPU_SAVE_STATE *)pSmstPi->CpuSaveState;
    HighBufferAddress = pCpuSaveState[Cpu].Ia32SaveState.ECX;
    LowBufferAddress = pCpuSaveState[Cpu].Ia32SaveState.EBX;
#endif
    BuffAddr = HighBufferAddress;
    BuffAddr = Shl64(BuffAddr, 32);
    BuffAddr += LowBufferAddress;

    //TRACE((-1,"\nOFBD address is:%x ------\n",BuffAddr));

    OFBDHeader = (OFBD_HDR *)BuffAddr;
    if ((Data == OFBD_SW_SMI_VALUE) && (OFBDHeader->OFBD_SIG == 'DBFO'))
    {
        OFBDHeader->OFBD_VER = OFBD_VERSION;
        OFBDHeader->OFBD_RS |= OFBD_RS_SUPPORT;

        for (i = 0; OFBDInitPartsTbl[i] != NULL; i++)
		{
        	OFBDInitPartsTbl[i]((VOID *)BuffAddr, &OFBDDataHandled);
		}
    }
    
    RETURN(Status);
}
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   InSmmFunction
//
// Description: Install OFBD SMI Handlers.
//
// Input:
//      IN EFI_HANDLE           ImageHandle
//      OUT EFI_SYSTEM_TABLE    *SystemTable
// Output:
//      EFI_STATUS
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS InSmmFunction(
    IN EFI_HANDLE ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS  Status;
    EFI_HANDLE  Handle;
    UINT16 i = 0;

#if PI_SPECIFICATION_VERSION >= 0x1000A
    EFI_SMM_SW_DISPATCH2_PROTOCOL    *pSwDispatch = NULL;
    EFI_SMM_SW_REGISTER_CONTEXT      SwContext = {OFBD_SW_SMI_VALUE};
#else
    EFI_SMM_SW_DISPATCH_PROTOCOL    *pSwDispatch;
    EFI_SMM_SW_DISPATCH_CONTEXT     SwContext = {OFBD_SW_SMI_VALUE};
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

    Status = pSwDispatch->Register(pSwDispatch, OFBDSMIHandler, &SwContext, \
                                                                     &Handle);
    ASSERT_EFI_ERROR(Status);

    for (i = 0; OFBDInSMMFuncTbl[i] != NULL; i++)
    {
    	OFBDInSMMFuncTbl[i]();
    }

    return Status;
}
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   NotInSmmFunction
//
// Description: Install OFBD SMI Handlers.
//
// Input:
//      IN EFI_HANDLE           ImageHandle
//      OUT EFI_SYSTEM_TABLE    *SystemTable
// Output:
//      EFI_STATUS
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS NotInSmmFunction(
    IN EFI_HANDLE ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable)
{
    UINT8 i;
    for (i = 0; OFBDNotInSMMFunc[i] != NULL; i++) OFBDNotInSMMFunc[i]();
    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	AmiOFBDEntryPoint
//
// Description: The main entry point of OFBD module.
//
// Input:
//      IN EFI_HANDLE           ImageHandle
//      OUT EFI_SYSTEM_TABLE    *SystemTable
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS AmiOFBDEntryPoint (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable )
{
    // 1. Setup AMI library
    InitAmiLib(ImageHandle, SystemTable);
    
    // 2. Utilize EfiLib to init
    return InitSmmHandler(ImageHandle, SystemTable, InSmmFunction, NULL);
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
