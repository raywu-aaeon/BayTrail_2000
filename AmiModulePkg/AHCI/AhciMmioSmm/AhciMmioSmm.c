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

/** @file AhciMmioSmm.c
    Mmio Access routines.
**/

//---------------------------------------------------------------------------

#include "Token.h"
#include "AmiDxeLib.h"
#include <Protocol/SmmCpu.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <AmiSmm.h>

//---------------------------------------------------------------------------

#if SMMGetMemoryType_SUPPORT
#include <Protocol/SmmGetMemoryTypeProtocol.h>
SMM_GET_MEMORY_TYPE_PROTOCOL    *SmmGetMemoryTypeProtocol=NULL;
#endif

EFI_SMM_BASE2_PROTOCOL    *gSmmBase2;
EFI_SMM_SYSTEM_TABLE2     *pSmst2;
EFI_GUID gEfiSmmCpuProtocolGuid = EFI_SMM_CPU_PROTOCOL_GUID;
EFI_SMM_CPU_PROTOCOL      *gSmmCpu=NULL;

#define RETURN(status) {return status;}

EFI_STATUS
AhciMmioSmmInSmmFunction(
    IN EFI_HANDLE          ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
);

/**
    Read from AHCI Base address

    @param AhciBaseAddress 

    @retval OUT UINT32  Value    

**/

UINT32 
ReadAhciBase (
    IN  UINT32  AhciBaseAddress
)
{

#if SMMGetMemoryType_SUPPORT
    if(SmmGetMemoryTypeProtocol != NULL ) {
        if( SmmGetMemoryTypeProtocol->SmmGetMemoryType(AhciBaseAddress) != EfiMemoryMappedIO) {
            return 0xFFFFFFFF;
        }
    }
#endif

    return *(UINT32*)(AhciBaseAddress);
}

/**
    Write to the Ahci Base Address

    @param AhciBaseAddress 
    @param WriteValue 

    @retval VOID

**/

EFI_STATUS WriteAhciBase (
    IN  UINT32  AhciBaseAddress,
    IN  UINT32  WriteValue
)
{
#if SMMGetMemoryType_SUPPORT
    if(SmmGetMemoryTypeProtocol != NULL ) {
        if( SmmGetMemoryTypeProtocol->SmmGetMemoryType(AhciBaseAddress) != EfiMemoryMappedIO) {
            return EFI_NOT_FOUND;
        }
    }
#endif

    *(UINT32*)(AhciBaseAddress)=WriteValue;
    return EFI_SUCCESS;
}

/**
    Smi handler for the AHCI_MMIO_SWSMI Sw Smi 

    @param    DispatchHandle  - EFI Handle
    @param    DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT

    @retval 

**/


EFI_STATUS AhciMmioSmmSMIHandler (
    IN EFI_HANDLE    DispatchHandle,
    IN CONST VOID    *DispatchContext OPTIONAL,
    IN OUT VOID      *CommBuffer OPTIONAL,
    IN OUT UINTN     *CommBufferSize OPTIONAL
)
{
    EFI_STATUS  Status ;  
    UINTN     Cpu;
    UINT32    FunctionNo;
    UINT32    AhciBaseAddress;
    UINT32    WriteValue;
    UINT32    ReadValue;
    UINT32    ReturnStatus;

    Cpu = ((EFI_SMM_SW_CONTEXT*)CommBuffer)->SwSmiCpuIndex;
    //
    // Found Invalid CPU number, return 
    //
    if(Cpu == (UINTN)-1) {
        Status = EFI_NOT_FOUND;
        RETURN(Status);
    }

    gSmmCpu->ReadSaveState (gSmmCpu,
                            4,
                            EFI_SMM_SAVE_STATE_REGISTER_RBX,
                            Cpu,
                            &WriteValue );

    gSmmCpu->ReadSaveState (gSmmCpu,
                            4,
                            EFI_SMM_SAVE_STATE_REGISTER_RCX,
                            Cpu,
                            &FunctionNo );

    gSmmCpu->ReadSaveState (gSmmCpu,
                            4,
                            EFI_SMM_SAVE_STATE_REGISTER_RSI,
                            Cpu,
                            &AhciBaseAddress );

    // Found Invalid CPU number, return 
    if(Cpu == (UINTN) -1) { 
        RETURN(Status);
    }

#if SMMGetMemoryType_SUPPORT

    if(SmmGetMemoryTypeProtocol == NULL) {
        Status = pSmst2->SmmLocateProtocol(&gSmmGetMemoryTypeProtocolGuid,
                                       NULL,
                                       &SmmGetMemoryTypeProtocol);
        if(EFI_ERROR(Status)) {
            SmmGetMemoryTypeProtocol=NULL;
        }
    }

#endif

    switch(FunctionNo) {
        case 0x1:
                ReadValue=ReadAhciBase(AhciBaseAddress);

                gSmmCpu->WriteSaveState(gSmmCpu,
                                        4,
                                        EFI_SMM_SAVE_STATE_REGISTER_RAX,
                                        Cpu,
                                        &ReadValue);

                ReturnStatus=0;
                gSmmCpu->WriteSaveState(gSmmCpu,
                                        4,
                                        EFI_SMM_SAVE_STATE_REGISTER_RCX,
                                        Cpu,
                                        &ReturnStatus);
                break;

        case 0x2:
                WriteAhciBase(AhciBaseAddress,WriteValue);

                ReturnStatus=0;
                gSmmCpu->WriteSaveState(gSmmCpu,
                                        4,
                                        EFI_SMM_SAVE_STATE_REGISTER_RCX,
                                        Cpu,
                                        &ReturnStatus);
                break;
        default:
                // Invalid Function. Return Error.
                ReturnStatus=0xFF;
                gSmmCpu->WriteSaveState(gSmmCpu,
                                        4,
                                        EFI_SMM_SAVE_STATE_REGISTER_RCX,
                                        Cpu,
                                        &ReturnStatus);
                break;
    }
    RETURN(Status);
}

/**
    Register the AHCI_MMIO_SWSMI SMI 

    @param    Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT
    @param    EFI System Table - Pointer to System Table

    @retval    EFI_STATUS OR EFI_NOT_FOUND

**/

EFI_STATUS
AhciMmioSmmInSmmFunction (
    IN EFI_HANDLE          ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
)
{
    EFI_HANDLE	Handle;
    EFI_STATUS	Status;

    EFI_SMM_SW_DISPATCH2_PROTOCOL    *pSwDispatch = NULL;
    EFI_SMM_SW_REGISTER_CONTEXT      SwContext;

    Status = InitAmiSmmLib( ImageHandle, SystemTable );
    if (EFI_ERROR(Status)) { 
        return Status;
    }

    Status = pBS->LocateProtocol(&gEfiSmmBase2ProtocolGuid, NULL, (VOID **) &gSmmBase2);
    if (EFI_ERROR(Status)) { 
        return Status;
    }

    // We are in SMM, retrieve the pointer to SMM System Table
    Status = gSmmBase2->GetSmstLocation( gSmmBase2, &pSmst2);
    if (EFI_ERROR(Status)) {  
        return EFI_UNSUPPORTED;
    }

    Status  = pSmst2->SmmLocateProtocol( &gEfiSmmSwDispatch2ProtocolGuid, \
                                          NULL, \
                                          (VOID **)&pSwDispatch );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    Status = pSmst2->SmmLocateProtocol(&gEfiSmmCpuProtocolGuid, NULL,  (VOID **)&gSmmCpu);
    if (EFI_ERROR(Status)) {
        ASSERT_EFI_ERROR(Status);
        return Status;
    }

    SwContext.SwSmiInputValue	= AHCI_MMIO_SWSMI;
    Status = pSwDispatch->Register(pSwDispatch, AhciMmioSmmSMIHandler, &SwContext, &Handle);
    ASSERT_EFI_ERROR(Status);
    return EFI_SUCCESS;
}

/**
    Ahci MMIO access module entry Point

    @param    Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT
    @param    EFI System Table - Pointer to System Table

    @retval    EFI_STATUS OR EFI_NOT_FOUND

**/

EFI_STATUS
AhciMmioSmmEntryPoint (
    IN EFI_HANDLE                ImageHandle,
    IN EFI_SYSTEM_TABLE          *SystemTable
)
{ 
    InitAmiLib(ImageHandle, SystemTable);
    return InitSmmHandler(ImageHandle, SystemTable, AhciMmioSmmInSmmFunction, NULL);

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
