//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file LegacySmmSredir.c
    Legacy console redirection SMM support
*/

//---------------------------------------------------------------------------

#include "Token.h"
#include "Protocol/LegacySredir.h"
#include "AmiDxeLib.h"
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <AmiSmm.h>
#include <Protocol/SmmCpu.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Library/AmiBufferValidationLib.h>

//---------------------------------------------------------------------------

#define RETURN(status)  {return status;}
EFI_SMM_BASE2_PROTOCOL  *gSmmBase2;
EFI_SMM_SYSTEM_TABLE2   *pSmst2;
EFI_SMM_CPU_PROTOCOL    *gSmmCpu = NULL;

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
LegacySredirSmmEntryPoint (
    IN  EFI_HANDLE          ImageHandle,
    IN  EFI_SYSTEM_TABLE    *SystemTable
);

EFI_STATUS
LegacySredirSMIHandler (
    IN  EFI_HANDLE                  DispatchHandle,
    IN  CONST VOID                  *Context OPTIONAL,
    IN  OUT VOID                    *CommBuffer OPTIONAL,
    IN  OUT UINTN                   *CommBufferSize OPTIONAL
);


/**
    Read the Data from Serial Port

    @param SREDIR_INPUT_PARAMETER SredirParam
    @retval EFI_SUCCESS
*/
EFI_STATUS
ReadSerialPort (
    IN  OUT SREDIR_INPUT_PARAMETER  *SredirParam
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

/**
    Write the Data to Serial Port

    @param SREDIR_INPUT_PARAMETER SredirParam 

    @retval EFI_SUCCESS
*/
EFI_STATUS
WriteSerialPort (
    IN  OUT SREDIR_INPUT_PARAMETER  *SredirParam
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

/**
    Write the buffer of data to Serial port

    @param SREDIR_INPUT_PARAMETER SredirParam 

    @retval EFI_SUCCESS
*/
EFI_STATUS
WriteBufferSerialPort (
    IN  OUT SREDIR_INPUT_PARAMETER    *SredirParam
)
{
    UINT8       i;
    UINT32      TempValue=0;
    UINT8       *DataBuffer=(UINT8*)SredirParam->BufferAddress;
    EFI_STATUS  Status;

    if(SredirParam->Count == 0) {
        return EFI_SUCCESS;
    }

    if( !DataBuffer ) {
        return EFI_INVALID_PARAMETER;
    }

    // Validate Input buffer is valid and not resides in SMRAM region
    Status = AmiValidateMemoryBuffer ( (VOID*)SredirParam->BufferAddress, SredirParam->Count );
    if( Status != EFI_SUCCESS ) {
        RETURN(Status);
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
 
/**
    Legacy Serial Redirection Smm handler function

    @param DispatchHandle  - EFI Handle
    @param DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT
    @param CommBuffer
    @param CommBufferSize

    @retval EFI_STATUS

*/

EFI_STATUS
LegacySredirSMIHandler (
    IN  EFI_HANDLE  DispatchHandle,
    IN  CONST VOID  *Context OPTIONAL,
    IN  OUT VOID    *CommBuffer OPTIONAL,
    IN  OUT UINTN   *CommBufferSize OPTIONAL
)

{
    EFI_STATUS    Status = EFI_SUCCESS;
    UINT64        pCommBuff;
    UINT32        HighBufferAddress = 0;
    UINT32        LowBufferAddress = 0;
    UINTN         Cpu = (UINTN)-1;
    SREDIR_INPUT_PARAMETER  *SredirParam;

    if (CommBuffer != NULL && CommBufferSize != NULL) {
        Cpu = ((EFI_SMM_SW_CONTEXT*)CommBuffer)->SwSmiCpuIndex;
    }

    //
    // Found Invalid CPU number, return
    //
    if(Cpu == (UINTN)-1) RETURN(Status);

    gSmmCpu->ReadSaveState ( gSmmCpu, \
                                      4, \
                                      EFI_SMM_SAVE_STATE_REGISTER_RBX, \
                                      Cpu, \
                                      &LowBufferAddress );
    gSmmCpu->ReadSaveState ( gSmmCpu, \
                                      4, \
                                      EFI_SMM_SAVE_STATE_REGISTER_RCX, \
                                      Cpu, \
                                      &HighBufferAddress );

    pCommBuff            = HighBufferAddress;
    pCommBuff            = Shl64(pCommBuff, 32);
    pCommBuff            += LowBufferAddress;
    SredirParam          =(SREDIR_INPUT_PARAMETER *)pCommBuff; 
    

    // Validate COM port register's MMIO address space are valid and not reside in SMRAM region
    Status = AmiValidateMmioBuffer( (VOID*)SredirParam->MMIOAddress, 8*COM_MMIO_WIDTH );
    if( Status != EFI_SUCCESS ) {
        SredirParam->Value=0;
        RETURN(Status);
    }


    switch(SredirParam->FuncNo)    {

        case 0x1:
                ReadSerialPort(SredirParam);
                break;

        case 0x2:
                WriteSerialPort(SredirParam);
                break;

        case 0x3:
                Status = WriteBufferSerialPort(SredirParam);
                break;

    }

    RETURN(Status);
}

/**
    This function is called from SMM during SMM registration.

        
    @param ImageHandle 
    @param SystemTable 

    @retval EFI_STATUS

*/
EFI_STATUS
LegacySredirInSmmFunction (
    EFI_HANDLE          ImageHandle,
    EFI_SYSTEM_TABLE    *SystemTable
)
{
    EFI_STATUS  Status;
    EFI_HANDLE                      SwHandle = NULL;

    EFI_SMM_SW_DISPATCH2_PROTOCOL    *pSwDispatch;

    EFI_SMM_SW_REGISTER_CONTEXT     SwContext;

    Status = InitAmiSmmLib( ImageHandle, SystemTable );
    if (EFI_ERROR(Status)) return Status;

    Status = pBS->LocateProtocol(&gEfiSmmBase2ProtocolGuid, NULL, &gSmmBase2);

    if (EFI_ERROR(Status)) return Status;

    Status = pSmmBase->GetSmstLocation (gSmmBase2, &pSmst);
    if (EFI_ERROR(Status)) return Status;

    Status = pSmst->SmmLocateProtocol( \
                        &gEfiSmmSwDispatch2ProtocolGuid, NULL, &pSwDispatch);
    if (EFI_ERROR(Status)) return Status;

    Status = pSmst->SmmLocateProtocol(&gEfiSmmCpuProtocolGuid, NULL, &gSmmCpu);
    if (EFI_ERROR(Status)) return Status;

    SwContext.SwSmiInputValue = LEGACY_SREDIR_SWSMI;
    Status = pSwDispatch->Register (pSwDispatch, LegacySredirSMIHandler, &SwContext, &SwHandle);

    return Status;
}

/**
    Legacy Serial Redirection  Smm entry point

    @param Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT
    @param EFI System Table - Pointer to System Table

    @retval EFI_STATUS OR EFI_NOT_FOUND

*/

EFI_STATUS
LegacySredirSmmEntryPoint (
    IN  EFI_HANDLE          ImageHandle,
    IN  EFI_SYSTEM_TABLE    *SystemTable
)
{
    InitAmiLib(ImageHandle, SystemTable);
    return InitSmmHandler(ImageHandle, SystemTable, LegacySredirInSmmFunction, NULL);
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
