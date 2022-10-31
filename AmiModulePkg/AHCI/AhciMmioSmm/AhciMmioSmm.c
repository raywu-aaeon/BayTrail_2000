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
// $Header: /Alaska/SOURCE/Modules/AHCI/INT13/AhciMmioSmm/AhciMmioSmm.c 3     8/13/12 2:25a Rameshr $
//
// $Revision: 3 $
//
// $Date: 8/13/12 2:25a $
//****************************************************************************

//<AMI_FHDR_START>
//****************************************************************************
//
// Name:    AhciMmioSmm.C
//
// Description: Mmio Access routines.
//****************************************************************************
//<AMI_FHDR_END>

#include "token.h"
#include "AmiDxeLib.h"
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)&&(CORE_COMBINED_VERSION>=0x4028B)
#include <Protocol/SmmCpu.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>
#else
#include <Protocol/SmmBase.h>
#include <Protocol/SmmSwDispatch.h>
#endif
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <AmiSmm.h>

#if SMMGetMemoryType_SUPPORT
#include <Protocol\SmmGetMemoryTypeProtocol.h>
SMM_GET_MEMORY_TYPE_PROTOCOL     *SmmGetMemoryTypeProtocol=NULL;
#endif
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)&&(CORE_COMBINED_VERSION>=0x4028B)
EFI_SMM_BASE2_PROTOCOL          *gSmmBase2;
EFI_SMM_SYSTEM_TABLE2           *pSmst2;
EFI_GUID gEfiSmmCpuProtocolGuid = EFI_SMM_CPU_PROTOCOL_GUID;
EFI_SMM_CPU_PROTOCOL            *gSmmCpu=NULL;
#define RETURN(status) {return status;}
#else
EFI_GUID gEfiSmmSwDispatchProtocolGuid = EFI_SMM_SW_DISPATCH_PROTOCOL_GUID;
EFI_GUID gSwSmiCpuTriggerGuid = SW_SMI_CPU_TRIGGER_GUID;
#define RETURN(status) {return ;}
#endif

EFI_STATUS
AhciMmioSmmInSmmFunction(
    IN EFI_HANDLE                ImageHandle,
    IN EFI_SYSTEM_TABLE          *SystemTable
 );


//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   ReadAhciBase
//
// Description: Read from AHCI Base address
//
// Input:       IN  UINT32  AhciBaseAddress     
//
// Output:      OUT UINT32  Value    
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 ReadAhciBase(
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

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   WriteAhciBase
//
// Description: Write to the Ahci Base Address
//
// Input:       IN  UINT32  AhciBaseAddress
//              IN  UINT32  WriteValue     
//
// Output:      None    
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS WriteAhciBase(
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

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	AhciMmioSmmSMIHandler
//
// Description:	Smi handler for the AHCI_MMIO_SWSMI Sw Smi 
//
// Input:	    DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT
//
// Output:      
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)&&(CORE_COMBINED_VERSION >= 0x4028B)
EFI_STATUS AhciMmioSmmSMIHandler (
	IN EFI_HANDLE       DispatchHandle,
	IN CONST VOID       *DispatchContext OPTIONAL,
	IN OUT VOID         *CommBuffer OPTIONAL,
	IN OUT UINTN        *CommBufferSize OPTIONAL
#else
VOID AhciMmioSmmSMIHandler (
    IN EFI_HANDLE                   DispatchHandle,
    IN EFI_SMM_SW_DISPATCH_CONTEXT  *DispatchContext
#endif
)
{
    EFI_STATUS  Status ;  
    UINTN       Cpu=(UINTN)-1;
    UINT32      FunctionNo;
    UINT32      AhciBaseAddress;
    UINT32      WriteValue;
    UINT32      ReadValue;
#if PI_SPECIFICATION_VERSION < 0x1000A   
    EFI_SMM_CPU_SAVE_STATE  *pCpuSaveState;
    SW_SMI_CPU_TRIGGER      *SwSmiCpuTrigger;
    UINTN       i;
#else
    UINT32      ReturnStatus;
#endif

#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)&&(CORE_COMBINED_VERSION >= 0x4028B)

    Cpu = ((EFI_SMM_SW_CONTEXT*)CommBuffer)->SwSmiCpuIndex;
    //
    // Found Invalid CPU number, return 
    //
    if(Cpu == (UINTN)-1) RETURN(Status);

    Status = gSmmCpu->ReadSaveState ( gSmmCpu, \
                                      4, \
                                      EFI_SMM_SAVE_STATE_REGISTER_RBX, \
                                      Cpu, \
                                      &WriteValue );

    Status = gSmmCpu->ReadSaveState ( gSmmCpu, \
                                      4, \
                                      EFI_SMM_SAVE_STATE_REGISTER_RCX, \
                                      Cpu, \
                                      &FunctionNo );

    Status = gSmmCpu->ReadSaveState ( gSmmCpu, \
                                      4, \
                                      EFI_SMM_SAVE_STATE_REGISTER_RSI, \
                                      Cpu, \
                                      &AhciBaseAddress );
                                                                                                      
    
    	
#else

    for (i = 0; i < pSmst->NumberOfTableEntries; ++i) {
        if (guidcmp(&pSmst->SmmConfigurationTable[i].VendorGuid,&gSwSmiCpuTriggerGuid) == 0) {
        break;
        }
    }

    //
    //If found table, check for the CPU that caused the software Smi.
    //
    if (i != pSmst->NumberOfTableEntries) {
        SwSmiCpuTrigger = pSmst->SmmConfigurationTable[i].VendorTable;
        Cpu = SwSmiCpuTrigger->Cpu;
    }

   
    pCpuSaveState   = pSmst->CpuSaveState;

    FunctionNo = pCpuSaveState[Cpu].Ia32SaveState.ECX;
    AhciBaseAddress = pCpuSaveState[Cpu].Ia32SaveState.ESI;
    WriteValue = pCpuSaveState[Cpu].Ia32SaveState.EBX;

#endif

    //
    // Found Invalid CPU number, return 
    //
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
  
    switch(FunctionNo)  {
        case 0x1:
                ReadValue=ReadAhciBase(AhciBaseAddress);

#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)&&(CORE_COMBINED_VERSION >= 0x4028B)
                gSmmCpu->WriteSaveState(
                                gSmmCpu,
                                4,
                                EFI_SMM_SAVE_STATE_REGISTER_RAX,
                                Cpu,
                                &ReadValue
                                );

                ReturnStatus=0;
                gSmmCpu->WriteSaveState(
                                gSmmCpu,
                                4,
                                EFI_SMM_SAVE_STATE_REGISTER_RCX,
                                Cpu,
                                &ReturnStatus
                                );
#else

                pCpuSaveState[Cpu].Ia32SaveState.EAX=ReadValue;
                pCpuSaveState[Cpu].Ia32SaveState.ECX=00;
#endif

                break;
                 
        case 0x2:
                WriteAhciBase(AhciBaseAddress,WriteValue);

#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)&&(CORE_COMBINED_VERSION >= 0x4028B)
                ReturnStatus=0;
                gSmmCpu->WriteSaveState(
                                gSmmCpu,
                                4,
                                EFI_SMM_SAVE_STATE_REGISTER_RCX,
                                Cpu,
                                &ReturnStatus
                                );
#else
                pCpuSaveState[Cpu].Ia32SaveState.ECX=00;
#endif
                break;
        default:
                //
                // Invalid Function. Return Error.
                //

#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)&&(CORE_COMBINED_VERSION >= 0x4028B)
                ReturnStatus=0xFF;
                gSmmCpu->WriteSaveState(
                                gSmmCpu,
                                4,
                                EFI_SMM_SAVE_STATE_REGISTER_RCX,
                                Cpu,
                                &ReturnStatus
                                );
#else
                pCpuSaveState[Cpu].Ia32SaveState.ECX=0xFF;
#endif
                break;
    }

    RETURN(Status);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:          AhciMmioSmmInSmmFunction
//
// Description:   Regsiter the AHCI_MMIO_SWSMI SMI 
//
// Input:         Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT
//                EFI System Table - Pointer to System Table
//
// Output:        EFI_STATUS OR EFI_NOT_FOUND
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AhciMmioSmmInSmmFunction(
    IN EFI_HANDLE                ImageHandle,
    IN EFI_SYSTEM_TABLE          *SystemTable
 )
{
    EFI_HANDLE	Handle;
    EFI_STATUS	Status;
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)&&(CORE_COMBINED_VERSION >= 0x4028B)
    EFI_SMM_SW_DISPATCH2_PROTOCOL    *pSwDispatch = NULL;
    EFI_SMM_SW_REGISTER_CONTEXT      SwContext;
#else
    EFI_SMM_SW_DISPATCH_PROTOCOL    *pSwDispatch;
    EFI_SMM_SW_DISPATCH_CONTEXT     SwContext;
#endif


#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)&&(CORE_COMBINED_VERSION >= 0x4028B)
    Status = InitAmiSmmLib( ImageHandle, SystemTable );

    Status = pBS->LocateProtocol(&gEfiSmmBase2ProtocolGuid, NULL, (VOID **) &gSmmBase2);
    
    if (EFI_ERROR(Status)) { 
        return Status;
    }

    //
    // We are in SMM, retrieve the pointer to SMM System Table
    //
    Status = gSmmBase2->GetSmstLocation( gSmmBase2, &pSmst2);
    if (EFI_ERROR(Status)) {  
        return EFI_UNSUPPORTED;
    }

    Status  = pSmst2->SmmLocateProtocol( &gEfiSmmSwDispatch2ProtocolGuid, \
                                          NULL, \
                                          (VOID **)&pSwDispatch );

    Status = pSmst2->SmmLocateProtocol(&gEfiSmmCpuProtocolGuid, NULL,  (VOID **)&gSmmCpu);
    if (EFI_ERROR(Status)) {
        return Status;
    }

#else
	Status	= pBS->LocateProtocol(&gEfiSmmSwDispatchProtocolGuid, NULL,  (VOID **)&pSwDispatch);
#endif

	ASSERT_EFI_ERROR(Status);

    if (EFI_ERROR(Status)) { 
        return Status;
    }
		
    SwContext.SwSmiInputValue	= AHCI_MMIO_SWSMI;
    Status	= pSwDispatch->Register(pSwDispatch, AhciMmioSmmSMIHandler, &SwContext, &Handle);
	ASSERT_EFI_ERROR(Status);
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:          AhciMmioSmmEntryPoint
//
// Description:   Ahci MMIO access module entry Point
//
// Input:         Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT
//                EFI System Table - Pointer to System Table
//
// Output:        EFI_STATUS OR EFI_NOT_FOUND
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AhciMmioSmmEntryPoint(
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
