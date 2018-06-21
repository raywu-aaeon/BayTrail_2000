//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2015, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

/** @file
  Common file for both the DXE and the SMM drivers. Contains the mian code
  for FlashSmi module.
  
  FlashSmi module will hook FLASH_PROTOCOL funtions to genrate SMI to use
  functions in SMM.
  
**/

//----------------------------------------------------------------------------
// Includes
// Statements that include other files
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include <Protocol\SmmBase.h>
#include <Protocol\SmmSwDispatch2.h>
#include <Protocol\FlashProtocol.h>
#include <Protocol\SmmCommunication.h>

//----------------------------------------------------------------------------
// Function Externs

//----------------------------------------------------------------------------
// Local prototypes

typedef struct
{
    UINT32      Signature;
    UINT32      FlashAddress;
    EFI_GUID    VarGuid;
    UINTN       Size;
    EFI_STATUS  Status;
    UINT16      Subfunction;
    CHAR16      DummyName[50];
    UINT32      DataBuffer;

} SMI_FLASH_UPDATE;

typedef struct
{
    BOOLEAN Busy;
    UINT8   IntState[2];

} FLASH_SMI_CRITICAL_SECTION;

#define FSMI_SIGNATURE ('F'+('S'<<8)+(('M'+('I'<<8))<<16))//FSMI
//----------------------------------------------------------------------------
// Local Variables
#define FLASH_SMI_GUID \
{0x4052ACA8, 0x8D90, 0x4F5A, 0xBF, 0xE8, 0xB8, 0x95, 0xB1, 0x64, 0xE4, 0x82}
#define BDS_CONNECT_DRIVERS_PROTOCOL_GUID \
{0x3aa83745, 0x9454, 0x4f7a, 0xa7, 0xc0, 0x90, 0xdb, 0xd0, 0x2f, 0xab, 0x8e}

EFI_GUID    gFlashSmiGuid = FLASH_SMI_GUID;
EFI_GUID    gBdsConnectDriversProtocolGuid = BDS_CONNECT_DRIVERS_PROTOCOL_GUID;
VOID        *gFlashBuffer = NULL;
FLASH_PROTOCOL  *gFlash, *gFlashSmm;
FLASH_SMI_CRITICAL_SECTION FlashSmiCs = {FALSE, {0, 0}};
FLASH_ERASE      gSavedFlashErase;
FLASH_READ_WRITE gSavedFlashWrite, gSavedFlashUpdate;
FLASH_ERASE      gSavedFlashEraseSmm;
FLASH_READ_WRITE gSavedFlashWriteSmm;
FLASH_READ_WRITE gSavedFlashUpdateSmm;
EFI_SMM_COMMUNICATION_PROTOCOL  *gSmmCommunicate = NULL;

/**
  Event to locate EFI_SMM_COMMUNICATION_PROTOCOL.

  @param Event
  @param Context

  @return VOID
**/
VOID
SmmCommunicationReady(
    IN EFI_EVENT    Event, 
    IN VOID         *Context
)
{
    EFI_STATUS  Status;

    pBS->CloseEvent(Event);

    Status = pBS->LocateProtocol(
                    &gEfiSmmCommunicationProtocolGuid,
                    NULL,
                    &gSmmCommunicate );
}
/**
  Use EFI_SMM_COMMUNICATION_PROTOCOL to generate SMI.
  
  @param VOID

  @return EFI_STATUS
  @retval EFI_SUCCESS Generate SMI successful
**/
EFI_STATUS
GenerateFlashSmi(
    VOID
)
{
    EFI_STATUS                  Status;
    EFI_SMM_COMMUNICATE_HEADER  SmmCommunicateHdr;
    UINTN                       SmmCommunicateHdrSize;

    MemCpy( &SmmCommunicateHdr.HeaderGuid, &gFlashSmiGuid, sizeof(EFI_GUID) );
    SmmCommunicateHdr.MessageLength = 1;
    SmmCommunicateHdr.Data[0] = 0;
    SmmCommunicateHdrSize = sizeof(EFI_SMM_COMMUNICATE_HEADER);

    Status = gSmmCommunicate->Communicate(
                                gSmmCommunicate,
                                &SmmCommunicateHdr,
                                &SmmCommunicateHdrSize );

    return Status;
}
//----------------------------------------------------------------------------
// Function Definitions
/**
  This function calls when critical section begins. It disables interupts, 
  and Smi and fills CRITICAL_SECTION structure fields.
  
  @param Cs pointer to CRITICAL_SECTION structure
  
  @return EFI_STATUS
  @retval EFI_SUCCESS
  @retval EFI_ACCESS_DENIED
**/
EFI_STATUS 
FlashSmiBeginCriticalSection (
    FLASH_SMI_CRITICAL_SECTION      *Cs
)
{
    if (Cs->Busy) return EFI_ACCESS_DENIED;
    Cs->IntState[0] = IoRead8(0x21);
    Cs->IntState[1] = IoRead8(0xa1);
    IoWrite8 (0x21, 0xff);
    IoWrite8 (0xa1, 0xff);
    Cs->Busy = TRUE;
    return EFI_SUCCESS;
}
/**
  This function calls when critical section ends. It enable interupts,
  and Smi and fills CRITICAL_SECTION structure fields.
  
  @param Cs pointer to CRITICAL_SECTION structure
  
  @return EFI_STATUS
  @retval EFI_SUCCESS
**/
EFI_STATUS
FlashSmiEndCriticalSection (
    FLASH_SMI_CRITICAL_SECTION      *Cs
)
{
    Cs->Busy = FALSE;
    IoWrite8 (0x21, Cs->IntState[0]);
    IoWrite8 (0xa1, Cs->IntState[1]);
    return EFI_SUCCESS;
}
/**
  Update the region of the flash part with the provided data.
  
  @param FlashAddress Pointer to address of a flash to update
  @param Size Size to update
  @param DataBuffer Pointer to the data to write into the flash part
  
  @return EFI_STATUS
  @retval EFI_SUCCESS The data was written successfully
  @retval EFI_DEVICE_ERROR An error was encountered while writing the data
**/
EFI_STATUS EFIAPI FlashDriverUpdateSmi (
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
)
{
    EFI_STATUS          Status = EFI_SUCCESS;
    SMI_FLASH_UPDATE    *Buffer;

    Status = EFI_NO_RESPONSE;
    if ((gFlashBuffer != NULL) && (gFlash != NULL)) { 
        if (EFI_ERROR(FlashSmiBeginCriticalSection(&FlashSmiCs))) 
            return EFI_ACCESS_DENIED;
        Buffer = (SMI_FLASH_UPDATE*)gFlashBuffer;
        MemSet (Buffer, sizeof(SMI_FLASH_UPDATE), 0);
        Buffer->Subfunction = 'Fu';
        Buffer->Signature = FSMI_SIGNATURE;
        Buffer->FlashAddress = (UINT32)FlashAddress;
        Buffer->Size = Size;
        Buffer->DataBuffer = (UINT32)DataBuffer;
        GenerateFlashSmi();
        Status = Buffer->Status;
        MemSet (Buffer, sizeof(SMI_FLASH_UPDATE), 0);
        FlashSmiEndCriticalSection (&FlashSmiCs);
    }
    if (Status == EFI_NO_RESPONSE) {
        Status = gSavedFlashUpdate (FlashAddress, Size, DataBuffer);
    }
    return Status;
}
/**
  Write the passed data from DataBuffer into the flash part at FlashAddress

  @param FlashAddress The address in the flash part to write the data
  @param Size The size of the data to write
  @param DataBuffer Pointer to the buffer of data to write into the flash part

  @return EFI_STATUS
  @retval EFI_SUCCESS The data was written successfully
  @retval EFI_DEVICE_ERROR An error was encountered while writing the data
**/
EFI_STATUS EFIAPI FlashDriverWriteSmi (
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
)
{
    EFI_STATUS          Status = EFI_SUCCESS;
    SMI_FLASH_UPDATE    *Buffer;

    Status = EFI_NO_RESPONSE;
    if ((gFlashBuffer != NULL) && (gFlash != NULL)) { 
        if (EFI_ERROR(FlashSmiBeginCriticalSection(&FlashSmiCs))) 
            return EFI_ACCESS_DENIED;
        Buffer = (SMI_FLASH_UPDATE*)gFlashBuffer;
        MemSet (Buffer, sizeof(SMI_FLASH_UPDATE), 0);
        Buffer->Subfunction = 'Fw';
        Buffer->Signature = FSMI_SIGNATURE;
        Buffer->FlashAddress = (UINT32)FlashAddress;
        Buffer->Size = Size;
        Buffer->DataBuffer = (UINT32)DataBuffer;
        GenerateFlashSmi();
        Status = Buffer->Status;
        MemSet (Buffer, sizeof(SMI_FLASH_UPDATE), 0);
        FlashSmiEndCriticalSection (&FlashSmiCs);
    }
    if (Status == EFI_NO_RESPONSE) {
        Status = gSavedFlashWrite (FlashAddress, Size, DataBuffer);
    }
    return Status;
}
/**
  Erase the flash part starting at the passed Address.

  @param FlashAddress Pointer to the address of the flash to erase
  @param Size Size, in bytes, of the flash to erase

  @return EFI_STATUS
  @retval EFI_SUCCESS The desired portion of the flash was erased
  @retval EFI_DEVICE_ERROR An error was encountered while erasing the flash
**/
EFI_STATUS EFIAPI FlashDriverEraseSmi (
    VOID* FlashAddress, UINTN Size
)
{
    EFI_STATUS          Status = EFI_SUCCESS;
    SMI_FLASH_UPDATE    *Buffer;
    
    Status = EFI_NO_RESPONSE;
    if ((gFlashBuffer != NULL) && (gFlash != NULL)) { 
        if (EFI_ERROR(FlashSmiBeginCriticalSection(&FlashSmiCs))) 
            return EFI_ACCESS_DENIED;
        Buffer = (SMI_FLASH_UPDATE*)gFlashBuffer;
        MemSet (Buffer, sizeof(SMI_FLASH_UPDATE), 0);
        Buffer->Subfunction = 'Fe';
        Buffer->Signature = FSMI_SIGNATURE;
        Buffer->FlashAddress = (UINT32)FlashAddress;
        Buffer->Size = Size;
        GenerateFlashSmi();
        Status = Buffer->Status;
        MemSet (Buffer, sizeof(SMI_FLASH_UPDATE), 0);
        FlashSmiEndCriticalSection (&FlashSmiCs);
    }
    if (Status == EFI_NO_RESPONSE) {
        gSavedFlashErase (FlashAddress, Size);
    }
    return Status;
}
/**
  This function will be called when Connect Drivers Protocol is installed
  and will update FlashProtocol function in RunTime.

  @param Event signalled event
  @param Context calling context

  @return VOID
**/
VOID FlashSmiConnectDrivers (
    IN EFI_EVENT    Event, 
    IN VOID         *Context
)
{
    pBS->CloseEvent(Event);
    if (gFlashBuffer == NULL) return;

    if (gFlash != NULL)
    {
        gSavedFlashUpdate = gFlash->Update;
        gSavedFlashErase = gFlash->Erase;
        gSavedFlashWrite = gFlash->Write;
        gFlash->Update = FlashDriverUpdateSmi;
        gFlash->Erase = FlashDriverEraseSmi;
        gFlash->Write = FlashDriverWriteSmi;
    }
}
/**
  This function will be called when Ready To Boot and will restore oringinal
  FlashProtocol function.

  @param Event signalled event
  @param Handle handle

  @return EFI_STATUS
**/
EFI_STATUS
FlashSmiReadyToBoot (
  EFI_EVENT           Event,
  VOID                *Handle
  )
{
    pBS->CloseEvent(Event);
    gFlash->Update = gSavedFlashUpdate;
    gFlash->Erase = gSavedFlashErase;
    gFlash->Write = gSavedFlashWrite;

    return EFI_SUCCESS;
}
/**
  This function will convert pointer.

  @param Event signalled event
  @param Context calling context

  @return VOID
**/
VOID FlashSmiVirtualFixup (
    IN EFI_EVENT    Event, 
    IN VOID         *Context
)
{
    pRS->ConvertPointer (0, &gFlashBuffer);
}
/**
  Main function in boot time.

  @param ImageHandle image handle
  @param SystemTable pointer to EFI_SYSTEM_TABLE

  @return VOID
**/
EFI_STATUS NotInSmmFunction(
    IN EFI_HANDLE          ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
)
{
    EFI_STATUS              Status;
    EFI_EVENT               EvtVirtualFixup, EvtConnectDrivers;
    EFI_EVENT               EvtReadyToBoot;
    VOID                    *RegConnectDrivers =  NULL;
    EFI_EVENT               EvtSmmCommunicationReady;
    VOID                    *RegSmmCommunicationReady;

    Status = pBS->LocateProtocol(
                    &gEfiSmmCommunicationProtocolGuid,
                    NULL,
                    &gSmmCommunicate );
    if( EFI_ERROR(Status) )
    {
        Status = RegisterProtocolCallback(
                    &gEfiSmmCommunicationProtocolGuid,
                    SmmCommunicationReady,
                    NULL,
                    &EvtSmmCommunicationReady,
                    &RegSmmCommunicationReady );
    }

    // Locate FlashProtocol.
    Status = pBS->LocateProtocol ( &gFlashProtocolGuid, NULL, &gFlash );
    if (EFI_ERROR(Status)) return Status;

    gFlashBuffer = NULL;
    Status = pBS->AllocatePool ( EfiRuntimeServicesData, \
                                 sizeof(SMI_FLASH_UPDATE), \
                                 &gFlashBuffer );
    if (EFI_ERROR(Status)) return EFI_SUCCESS;

    MemSet (gFlashBuffer, sizeof(SMI_FLASH_UPDATE), 0);

    Status = pRS->SetVariable ( L"FlashSmiBuffer", \
                                &gFlashSmiGuid, \
                                EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                                sizeof(gFlashBuffer), \
                               &gFlashBuffer  );
    if (EFI_ERROR(Status)) return EFI_SUCCESS;

    Status = pBS->CreateEvent ( EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE, \
                                TPL_CALLBACK, \
                                FlashSmiVirtualFixup, \
                                NULL, \
                                &EvtVirtualFixup );
    if (EFI_ERROR(Status)) return EFI_SUCCESS;

    Status = RegisterProtocolCallback ( &gBdsConnectDriversProtocolGuid, \
                                        FlashSmiConnectDrivers, \
                                        NULL, \
                                        &EvtConnectDrivers, \
                                        &RegConnectDrivers );

    Status = CreateReadyToBootEvent( TPL_CALLBACK, FlashSmiReadyToBoot, \
                                    NULL, &EvtReadyToBoot );

    return Status;
}
/**
  Main SMI handler function.

  @param DispatchHandle The unique handle assigned to this handler by SmiHandlerRegister()
  @param Context Points to an optional handler context which was specified when the handler was registered
  @param CommBuffer A pointer to memory will be conveyed from a non-SMM environment into an SMM environment
  @param CommBufferSize The size of the CommBuffer

  @return EFI_STATUS
**/
EFI_STATUS FlashSmiHandler (
  IN EFI_HANDLE  DispatchHandle,
  IN CONST VOID  *Context         OPTIONAL,
  IN OUT VOID    *CommBuffer      OPTIONAL,
  IN OUT UINTN   *CommBufferSize  OPTIONAL
)
{
    SMI_FLASH_UPDATE        *SmmFlashUpdate;
    EFI_STATUS              Status;

    if( gFlashBuffer == NULL )
        return EFI_UNSUPPORTED;

    SmmFlashUpdate = (SMI_FLASH_UPDATE*)gFlashBuffer;
    if (SmmFlashUpdate->Signature != FSMI_SIGNATURE) return EFI_UNSUPPORTED;
    
    switch (SmmFlashUpdate->Subfunction)
    {
        // FlashUpdate call
        case 'Fu':
        {
            if (gFlashSmm == NULL) return EFI_NOT_READY;
            SmmFlashUpdate = (SMI_FLASH_UPDATE*)gFlashBuffer;
            Status = gSavedFlashUpdateSmm ((VOID*)(SmmFlashUpdate->FlashAddress), \
                                        SmmFlashUpdate->Size, \
                                        (VOID*)(SmmFlashUpdate->DataBuffer));
        }
        break;

        // FlashErase call
        case 'Fe':
        {
            if (gFlashSmm == NULL) return EFI_NOT_READY;
            SmmFlashUpdate = (SMI_FLASH_UPDATE*)gFlashBuffer;
            Status = gSavedFlashEraseSmm ((VOID*)(SmmFlashUpdate->FlashAddress), \
                                                SmmFlashUpdate->Size);
        }
        break;
        
        // FlashWrite call
        case 'Fw':
        {
            if (gFlashSmm == NULL) return EFI_NOT_READY;
            SmmFlashUpdate = (SMI_FLASH_UPDATE*)gFlashBuffer;
            Status = gSavedFlashWriteSmm ((VOID*)(SmmFlashUpdate->FlashAddress), \
                                        SmmFlashUpdate->Size, \
                                        (VOID*)(SmmFlashUpdate->DataBuffer));
        }
        break;

        // WriteEnable call
        case 'We':
        {
            if (gFlashSmm == NULL) return EFI_NOT_READY;
            Status = gFlashSmm->DeviceWriteEnable ();
        }
        break;
        
        default: return EFI_UNSUPPORTED;
    }
    
    SmmFlashUpdate->Status = Status;
    SmmFlashUpdate->Subfunction = 0;
    // Invalidate Flash SMI Buffer. 
    SmmFlashUpdate->Signature = ~FSMI_SIGNATURE; 
    return EFI_SUCCESS;
}
/**
  Main function in SMM.

  @param ImageHandle image handle
  @param SystemTable pointer to EFI_SYSTEM_TABLE

  @return VOID
**/
EFI_STATUS InSmmFunction(
    IN EFI_HANDLE          ImageHandle,
    IN EFI_SYSTEM_TABLE    *SystemTable
)
{
    EFI_STATUS                      Status;
    UINTN                           VariableSize;
    EFI_HANDLE                      Handle = NULL;

    // Locate SmmFlashProtocol.
    Status = pBS->LocateProtocol ( &gFlashSmmProtocolGuid, NULL, &gFlashSmm );
    if (EFI_ERROR(Status)) gFlashSmm = NULL;

    if( gFlashSmm != NULL )
    {
        gSavedFlashEraseSmm = gFlashSmm->Erase;
        gSavedFlashWriteSmm = gFlashSmm->Write;
        gSavedFlashUpdateSmm = gFlashSmm->Update;
    }

    // Get Flash SMI runimte memory buffer.
    VariableSize = sizeof(gFlashBuffer);
    Status = pRS->GetVariable ( L"FlashSmiBuffer", \
                                &gFlashSmiGuid, \
                                NULL, \
                                &VariableSize, \
                                &gFlashBuffer );
    if ((EFI_ERROR(Status)) || (gFlashBuffer == NULL)) return Status;

    // Register Flash Software SMI.
    Status = pSmst->SmiHandlerRegister(
                        FlashSmiHandler,
                        &gFlashSmiGuid,
                        &Handle );

    return Status;
}
/**
  Module entry point.

  @param ImageHandle image handle
  @param SystemTable pointer to EFI_SYSTEM_TABLE

  @return VOID
**/
EFI_STATUS
FlashSmiEntry (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
)
{
    EFI_BOOT_MODE   Bootmode;

    InitAmiLib(ImageHandle, SystemTable);

    Bootmode = GetBootMode();
    if( (Bootmode == BOOT_ON_FLASH_UPDATE) ||
        (Bootmode == BOOT_IN_RECOVERY_MODE) )
    {
        return InitSmmHandlerEx(
                    ImageHandle,
                    SystemTable,
                    InSmmFunction,
                    NotInSmmFunction );
    }
    else
    {
        return EFI_SUCCESS;
    }
}
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2015, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
