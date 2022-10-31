//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/Legacy Serial Redirection/LegacySmmSredir.c 3     6/17/11 4:41a Rameshr $
//
// $Revision: 3 $
//
// $Date: 6/17/11 4:41a $
//****************************************************************************
//****************************************************************************
//<AMI_FHDR_START>
//****************************************************************************
//
// Name:	LegacySmmSredir.C
//
// Description:	Legacy console redirection SMM support
//****************************************************************************
//<AMI_FHDR_END>

#include "token.h"
#include "Protocol/LegacySredir.h"
#include "AmiDxeLib.h"
#include <Protocol\DevicePath.h>
#include <Protocol\LoadedImage.h>
#include <AmiSmm.h>

#if PI_SPECIFICATION_VERSION >= 0x1000A
#include <Protocol\SmmCpu.h>
#include <Protocol\SmmBase2.h>
#include <Protocol\SmmSwDispatch2.h>
#define RETURN(status) {return status;}

EFI_SMM_BASE2_PROTOCOL          *gSmmBase2;
EFI_SMM_CPU_PROTOCOL            *gSmmCpu;
#else
#include <Protocol\SmmBase.h>
#include <Protocol\SmmSwDispatch.h>
#define RETURN(status) {return;}
#endif

EFI_GUID gSwSmiCpuTriggerGuid = SW_SMI_CPU_TRIGGER_GUID;

#pragma pack(1)
typedef struct {
    UINT32          MMIOAddress;
    UINT8           FuncNo;
    UINT8           ReadWrite;      
    UINT8           Offset;
    UINT8           Value;
    UINT8           Count;
    UINT32          BufferAddress;
} SREDIR_INPUT_PARAMETER;
#pragma pack()

EFI_STATUS
LegacySredirSmmEntryPoint(
    IN EFI_HANDLE                ImageHandle,
    IN EFI_SYSTEM_TABLE          *SystemTable
 );

#if PI_SPECIFICATION_VERSION >= 0x1000A
EFI_STATUS
LegacySredirSMIHandler (
	IN  EFI_HANDLE                  DispatchHandle,
    IN  CONST VOID                  *Context OPTIONAL,
	IN  OUT VOID                    *CommBuffer OPTIONAL,
	IN  OUT UINTN                   *CommBufferSize OPTIONAL
);
#else
VOID LegacySredirSMIHandler (
	IN	EFI_HANDLE					DispatchHandle,
	IN	EFI_SMM_SW_DISPATCH_CONTEXT	*DispatchContext
);
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   ReadSerialPort
//
// Description: Read the Data from Serial Port
//
// Input:       SREDIR_INPUT_PARAMETER SredirParam 
//
// Output:      
//
// Returns: 
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ReadSerialPort(
	IN OUT SREDIR_INPUT_PARAMETER	*SredirParam
)
{

    UINT32  TempValue;
#if COM_MMIO_WIDTH == 4
    TempValue=*(UINT32*)(SredirParam->MMIOAddress+(SredirParam->Offset*COM_MMIO_WIDTH));
#else
    #if COM_MMIO_WIDTH == 2
        TempValue=*(UINT16*)(SredirParam->MMIOAddress+(SredirParam->Offset*COM_MMIO_WIDTH));
    #else
        TempValue=*(UINT8*)(SredirParam->MMIOAddress+(SredirParam->Offset*COM_MMIO_WIDTH));
    #endif
#endif    

    SredirParam->Value=(UINT8)TempValue;
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   WriteSerialPort
//
// Description: Write the Data to Serial Port
//
// Input:       SREDIR_INPUT_PARAMETER SredirParam 
//
// Output:      
//
// Returns: 
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS WriteSerialPort(
	IN OUT SREDIR_INPUT_PARAMETER	*SredirParam
)
{
    UINT32  TempValue=(UINT32)SredirParam->Value;

#if COM_MMIO_WIDTH == 4
    *(UINT32*)(SredirParam->MMIOAddress+(SredirParam->Offset*COM_MMIO_WIDTH))=TempValue;
#else
    #if COM_MMIO_WIDTH == 2
        *(UINT16*)(SredirParam->MMIOAddress+(SredirParam->Offset*COM_MMIO_WIDTH))=(UINT16)TempValue;
    #else
        *(UINT8*)(SredirParam->MMIOAddress+(SredirParam->Offset*COM_MMIO_WIDTH))=(UINT8)TempValue;
    #endif
#endif   

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   WriteBufferSerialPort
//
// Description: Write the buffer of data to Serial port
//
// Input:       SREDIR_INPUT_PARAMETER SredirParam 
//
// Output:      
//
// Returns: 
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS WriteBufferSerialPort(
	IN OUT SREDIR_INPUT_PARAMETER	*SredirParam
)
{
    UINT8 i;
    UINT32  TempValue=0;
    UINT8   *DataBuffer=(UINT8*)SredirParam->BufferAddress;

    if(SredirParam->Count == 0) {
        return EFI_SUCCESS;
    }

    for(i=0;i<SredirParam->Count;i++) {
        TempValue=*DataBuffer;

#if COM_MMIO_WIDTH == 4
        *(UINT32*)(SredirParam->MMIOAddress+(SredirParam->Offset*COM_MMIO_WIDTH))=TempValue;
#else
    #if COM_MMIO_WIDTH == 2
            *(UINT16*)(SredirParam->MMIOAddress+(SredirParam->Offset*COM_MMIO_WIDTH))=(UINT16)TempValue;
    #else
            *(UINT8*)(SredirParam->MMIOAddress+(SredirParam->Offset*COM_MMIO_WIDTH))=(UINT8)TempValue;
    #endif
#endif   
        DataBuffer++;
    }

    return EFI_SUCCESS;
}
 
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	LegacySredirSMIHandler
//
// Description:	Legacy Serial Redirection Smm handler function
//
// Input:	    DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT
//
// Output:      
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
#if PI_SPECIFICATION_VERSION >= 0x1000A
EFI_STATUS
LegacySredirSMIHandler (
	IN  EFI_HANDLE                  DispatchHandle,
    IN  CONST VOID                  *Context OPTIONAL,
	IN  OUT VOID                    *CommBuffer OPTIONAL,
	IN  OUT UINTN                   *CommBufferSize OPTIONAL
)
#else
VOID LegacySredirSMIHandler (
	IN	EFI_HANDLE					DispatchHandle,
	IN	EFI_SMM_SW_DISPATCH_CONTEXT	*DispatchContext
)
#endif
{
	EFI_SMM_CPU_SAVE_STATE	*pCpuSaveState = NULL;
	EFI_STATUS  Status = EFI_SUCCESS;
	UINT8		Data;
	UINT64		pCommBuff;
	UINT32		HighBufferAddress = 0;
	UINT32		LowBufferAddress = 0;
    SREDIR_INPUT_PARAMETER  *SredirParam;

    UINTN       Cpu = (UINTN)-1;
#if PI_SPECIFICATION_VERSION < 0x1000A
    SW_SMI_CPU_TRIGGER      *SwSmiCpuTrigger;
    UINTN       i;
#endif

#if PI_SPECIFICATION_VERSION >= 0x1000A

    if (CommBuffer != NULL && CommBufferSize != NULL) {
        Cpu = ((EFI_SMM_SW_CONTEXT*)CommBuffer)->SwSmiCpuIndex;
    }

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

    Data = ((EFI_SMM_SW_CONTEXT*)Context)->CommandPort;

#else

  	for (i = 0; i < pSmst->NumberOfTableEntries; ++i) {
		if (guidcmp(&pSmst->SmmConfigurationTable[i].VendorGuid,&gSwSmiCpuTriggerGuid) == 0) {
			break;
		}
  	}
	
  	//If found table, check for the CPU that caused the software Smi.
  	if (i != pSmst->NumberOfTableEntries) {
		SwSmiCpuTrigger = pSmst->SmmConfigurationTable[i].VendorTable;
		Cpu = SwSmiCpuTrigger->Cpu;
  	}

  	if(Cpu == (UINTN) -1) {
  		RETURN(Status);
  	}

	Data	= (UINT8)DispatchContext->SwSmiInputValue;
	
	pCpuSaveState		= pSmst->CpuSaveState;
	HighBufferAddress	= pCpuSaveState[Cpu].Ia32SaveState.ECX;
	LowBufferAddress	= pCpuSaveState[Cpu].Ia32SaveState.EBX;
#endif

	pCommBuff			= HighBufferAddress;
	pCommBuff			= Shl64(pCommBuff, 32);
	pCommBuff			+= LowBufferAddress;
    SredirParam         =(SREDIR_INPUT_PARAMETER *)pCommBuff; 


	switch(SredirParam->FuncNo)	{

        case 0x1:
                ReadSerialPort(SredirParam);
                break;
                 
        case 0x2:
                WriteSerialPort(SredirParam);
                break;

        case 0x3:
                WriteBufferSerialPort(SredirParam);
                break;

    }

	RETURN(Status);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   LegacySredirInSmmFunction
//
// Description: This function is called from SMM during SMM registration.
//
// Input:
//  IN EFI_HANDLE       ImageHandle
//  IN EFI_SYSTEM_TABLE *SystemTable
//
// Output: EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS LegacySredirInSmmFunction(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    EFI_STATUS  Status;
    EFI_HANDLE                      SwHandle = NULL;
#if PI_SPECIFICATION_VERSION >= 0x1000A
    EFI_SMM_SW_DISPATCH2_PROTOCOL    *pSwDispatch;

    EFI_SMM_SW_REGISTER_CONTEXT     SwContext;
#else
    EFI_SMM_SW_DISPATCH_PROTOCOL    *pSwDispatch;
    EFI_SMM_SW_DISPATCH_CONTEXT     SwContext;
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
    //
    // Register the SDIO SW SMI handler
    //
    Status = pBS->LocateProtocol (&gSwDispatchProtocolGuid, NULL, &pSwDispatch);

    if (EFI_ERROR (Status)) {
        return Status;
    }
#endif

    SwContext.SwSmiInputValue = LEGACY_SREDIR_SWSMI;
    Status = pSwDispatch->Register (pSwDispatch, LegacySredirSMIHandler, &SwContext, &SwHandle);

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:          LegacySredirSmmEntryPoint
//
// Description:   Legacy Serial Redirection  Smm entry point
//
// Input:         Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT
//                EFI System Table - Pointer to System Table
//
// Output:        EFI_STATUS OR EFI_NOT_FOUND
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
LegacySredirSmmEntryPoint(
    IN EFI_HANDLE                ImageHandle,
    IN EFI_SYSTEM_TABLE          *SystemTable
 )
{
	InitAmiLib(ImageHandle, SystemTable);

	return InitSmmHandler(ImageHandle, SystemTable, LegacySredirInSmmFunction, NULL);

}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
