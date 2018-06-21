//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
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
// $Header: /Alaska/SOURCE/Core/Runtime/Flash.c 10    11/11/11 3:06p Artems $
//
// $Revision: 10 $
//
// $Date: 11/11/11 3:06p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    Flash.c
//
// Description: Flash Driver Implementation
//
//<AMI_FHDR_END>
//**********************************************************************
#include <AmiDxeLib.h>
#include <Flash.h>
#include <Ffs.h>
#include <RomLayout.h>
#include <AmiHobs.h>
#include <Protocol/FlashProtocol.h>
#include <Library/AmiCriticalSectionLib.h>

typedef struct
{
    ROM_AREA *RomLayout;
    UINT32 WriteEnableStatus;
    CRITICAL_SECTION Cs;
    UINT32 RomLayoutSize;
} FLASH_DRIVER_MAILBOX;

typedef struct
{
    FLASH_PROTOCOL Flash;
    FLASH_DRIVER_MAILBOX *MailBox;
} FLASH_PROTOCOL_PRIVATE;

typedef struct
{
    ROM_AREA *RomArea;
    UINT32 AreaSize ;
    BOOLEAN RestoreSignature;                                           // [ EIP341497 ]
} UPDATED_AREA_DESCRIPTOR;

//-----Prototypes------------------------------------
EFI_STATUS EFIAPI FlashDriverRead(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
);
EFI_STATUS EFIAPI FlashDriverErase(
    VOID* FlashAddress, UINTN Size
);
EFI_STATUS EFIAPI FlashDriverWrite(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
);
EFI_STATUS EFIAPI FlashDriverUpdate(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
);
EFI_STATUS EFIAPI FlashDriverDeviceWriteEnable();
EFI_STATUS EFIAPI FlashDriverDeviceWriteDisable();

EFI_STATUS EFIAPI FlashDriverReadExt(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
);
EFI_STATUS EFIAPI FlashDriverEraseExt(
    VOID* FlashAddress, UINTN Size
);
EFI_STATUS EFIAPI FlashDriverWriteExt(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
);
EFI_STATUS EFIAPI FlashDriverUpdateExt(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
);
EFI_STATUS EFIAPI FlashDriverDeviceWriteEnableExt();
EFI_STATUS EFIAPI FlashDriverDeviceWriteDisableExt();

extern const UINTN FlashEmpty;
extern const BOOLEAN FlashNotMemoryMapped;

#define BEGIN_CRITICAL_SECTION(Cs) \
    { EFI_STATUS Status = BeginCriticalSection(Cs);\
      ASSERT(Status==EFI_SUCCESS || Status==EFI_ACCESS_DENIED);\
      if (EFI_ERROR(Status)) return Status;\
    }
#define END_CRITICAL_SECTION(Cs) VERIFY_EFI_ERROR(EndCriticalSection(Cs))

//---Flash data protocole------------------------------------------
FLASH_PROTOCOL_PRIVATE FlashData =
{
    {
        FlashDriverReadExt, FlashDriverEraseExt, FlashDriverWriteExt, FlashDriverUpdateExt,
        FlashDriverDeviceWriteEnableExt, FlashDriverDeviceWriteDisableExt
    },
    NULL
};

ROM_AREA *RomLayout = NULL;
#define MAX_NUMBER_OF_UPDATED_AREAS 10
UPDATED_AREA_DESCRIPTOR UpdatedArea[MAX_NUMBER_OF_UPDATED_AREAS];
INT32 NumberOfUpdatedAreas=0;
CRITICAL_SECTION Cs;

/*************** OUTSIDE SMM **********************************/
// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: FlashDrvVirtAddressChange
//
// Description: Function updates the current pointer to FlashData protocol functions and
//              UpdatedArea pointed to by Address to be the proper value for the new address map
//
// Input:
//  IN EFI_EVENT                Event       Signalled event
//  IN VOID                     *Context    Calling context
//
// Output:      NON
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>
VOID FlashDrvVirtAddressChange (
    IN EFI_EVENT Event, IN VOID *Context
)
{
    VOID **p;
    INT32 i;
    FlashVirtualFixup(pRS);
    //Fixup protocol member functions
    p=(VOID**)&FlashData.Flash.Write;
    
    do
    {
        pRS->ConvertPointer(0, p++);
    }
    while (p!=(VOID**)&FlashData.Flash.DeviceWriteDisable);
    
    pRS->ConvertPointer(0, &FlashData.MailBox);
    
//    if (RomLayout!=NULL) pRS->ConvertPointer(0, &RomLayout);
    
    for (i=0; i<NumberOfUpdatedAreas; i++)
    {
        pRS->ConvertPointer(0, &UpdatedArea[i].RomArea);
    }
    pRS->ConvertPointer(0,&Cs);
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: FlashDriverInit
//
// Description: Creates necessary structures and instatlls Flash services before SMM mode.
//
// Input:
//  IN EFI_HANDLE               ImageHandle     Image Handle
//  IN EFI_SYSTEM_TABLE         *SystemTable    Pointer to System Table
//
// Output:      EFI_STATUS
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

EFI_STATUS FlashDriverInit(
    IN EFI_HANDLE ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    static EFI_GUID HobListGuid = HOB_LIST_GUID;
    static EFI_GUID AmiRomLayoutHobGuid = AMI_ROM_LAYOUT_HOB_GUID;
    EFI_STATUS Status;
    EFI_HANDLE Handler = NULL;
    ROM_LAYOUT_HOB *RomLayoutHob;
    UINTN MailboxSize = sizeof(FLASH_DRIVER_MAILBOX);
    UINTN RomLayoutSize;
    
    InitAmiRuntimeLib(ImageHandle, SystemTable, NULL, FlashDrvVirtAddressChange);

//Create mailbox
    VERIFY_EFI_ERROR(pBS->AllocatePool(EfiRuntimeServicesData, MailboxSize, (VOID **)&FlashData.MailBox));
//Get ROM layout
    RomLayoutHob = GetEfiConfigurationTable(SystemTable, &HobListGuid);
    if (RomLayoutHob != NULL) {
        if (!EFI_ERROR(FindNextHobByGuid(&AmiRomLayoutHobGuid, &RomLayoutHob))) {
            ROM_AREA *Area;
            RomLayoutSize = RomLayoutHob->Header.Header.HobLength - sizeof(ROM_LAYOUT_HOB);
            VERIFY_EFI_ERROR(pBS->AllocatePool(EfiBootServicesData, RomLayoutSize, &RomLayout));
            pBS->CopyMem(RomLayout, RomLayoutHob + 1, RomLayoutSize);   //save ROM Layout for future use

            for (Area = RomLayout; Area->Size != 0; Area++) {   //update address
                Area->Address -= FlashDeviceBase;
            }

            //pass ROM layout to SMM driver
            FlashData.MailBox->RomLayout = RomLayout;
            FlashData.MailBox->RomLayoutSize = (UINT32)RomLayoutSize;
        } else {
            FlashData.MailBox->RomLayout = NULL;
            FlashData.MailBox->RomLayoutSize = 0;
        }
    }

//Fill MailBox 
    FlashData.MailBox->WriteEnableStatus = 0;
    Status = CreateCriticalSection(&Cs);
    ASSERT_EFI_ERROR(Status);
    FlashData.MailBox->Cs = Cs;

    Status = pBS->InstallProtocolInterface(&Handler, &gFlashProtocolGuid, EFI_NATIVE_INTERFACE, &FlashData.Flash);
    ASSERT_EFI_ERROR(Status);

    return Status;
}

/*************** INSIDE SMM **********************************/
// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: FlashDriverSmmInit
//
// Description: Reinitialize necessary structures and instatlls Flash services in SMM mode.
//
// Input:
//  IN EFI_HANDLE               ImageHandle     Image Handle
//  IN EFI_SYSTEM_TABLE         *SystemTable    Pointer to System Table
//
// Output:      EFI_STATUS
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

EFI_STATUS FlashDriverSmmInit(
    IN EFI_HANDLE ImageHandle, 
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    EFI_STATUS Status;
    EFI_HANDLE Handler = NULL;
    FLASH_PROTOCOL_PRIVATE *NotSmmFlash;

    VERIFY_EFI_ERROR(pBS->LocateProtocol(&gFlashProtocolGuid, NULL, &NotSmmFlash));

    //Reasign MailBox
    FlashData.MailBox = NotSmmFlash->MailBox;
    Cs = FlashData.MailBox->Cs;

   //Save SMM copy of ROM layout
    if(FlashData.MailBox->RomLayoutSize != 0) {
        VERIFY_EFI_ERROR(pSmst->SmmAllocatePool(EfiRuntimeServicesData, FlashData.MailBox->RomLayoutSize, &RomLayout));
        pBS->CopyMem(RomLayout, FlashData.MailBox->RomLayout, FlashData.MailBox->RomLayoutSize);
    }

    Status = pBS->InstallProtocolInterface(&Handler, &gFlashSmmProtocolGuid, EFI_NATIVE_INTERFACE, &FlashData.Flash);
    ASSERT_EFI_ERROR(Status);
    return Status;
}

/*************** INSIDE and OUSIDE SMM ************************/

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: BeforeErase
//
// Description: Invalidates FV by destroying the signature before erasing flash.
//
// Input:
//  VOID*       FlashAddress    Pointer to address of a flash 
//
// Output:      NON
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

VOID BeforeErase(VOID* FlashAddress)
{
    ROM_AREA *Area;
    UINT32 Data;                                                        // [ EIP341497 ]

    if (   RomLayout == NULL
            || NumberOfUpdatedAreas == MAX_NUMBER_OF_UPDATED_AREAS
       ) return;
    //---For Areas in Rom Layout------------------------------------------   
    for (Area=RomLayout; Area->Size!=0; Area++)
    {
    //---If this is area we are looking for-------------------------------
        if (Area->Address+FlashDeviceBase==(EFI_PHYSICAL_ADDRESS)(UINTN)FlashAddress)
        {
            if (Area->Type!=RomAreaTypeFv) return;

// [ EIP341497 ] +>
/*
            UpdatedArea[NumberOfUpdatedAreas++].RomArea = Area;
            // Invalidate FV by destroying the signature
            FlashDriverWrite(
                &((EFI_FIRMWARE_VOLUME_HEADER*)FlashAddress)->Signature,
                sizeof(UINT32),
                (UINT32*)&FlashEmpty
*/
            UpdatedArea[NumberOfUpdatedAreas].AreaSize = Area->Size;
			UpdatedArea[NumberOfUpdatedAreas].RomArea = Area;
			UpdatedArea[NumberOfUpdatedAreas++].RestoreSignature = FALSE;
            // Invalidate FV by destroying the signature
            Data = (UINT32)~FlashEmpty;			
            FlashDriverWrite(
                &((EFI_FIRMWARE_VOLUME_HEADER*)FlashAddress)->Signature,
                sizeof(UINT32),
                &Data
// [ EIP341497 ] +<
            );
            return;
        }
    }
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: BeforeWrite
//
// Description: Invalidates FV by destroying the signature before writing to flash.
//
// Input:
//  VOID*       FlashAddress    Pointer to address of a flash 
//  UINTN       Size            Size to write
//  VOID*       DataBuffer      Poiner to the Data Buffer
//
// Output:      NON
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

VOID BeforeWrite(VOID* FlashAddress, UINTN Size, VOID* DataBuffer)
{
    EFI_FIRMWARE_VOLUME_HEADER* Fv;
    INT32 i;

    for (i=0; i<NumberOfUpdatedAreas; i++)
    {
        //---Invalidate FV by destroying the signature
        //---(alter data being programmed)
        if (  UpdatedArea[i].RomArea->Address+FlashDeviceBase
                ==(EFI_PHYSICAL_ADDRESS)(UINTN)FlashAddress
           )
        {
            Fv = (EFI_FIRMWARE_VOLUME_HEADER*)DataBuffer;
            
            if (   Size < sizeof(EFI_FIRMWARE_VOLUME_HEADER)
                    || Fv->Signature != FV_SIGNATURE
               )   //The data being programmed is not FV, don't do anything
            {
                UpdatedArea[i] = UpdatedArea[NumberOfUpdatedAreas-1];
                NumberOfUpdatedAreas--;
            }
            
            else
            {
                Fv->Signature = (UINT32)FlashEmpty;
                UpdatedArea[i].AreaSize = (UINT32)Fv->FvLength;
				UpdatedArea[i].RestoreSignature = TRUE;                 // [ EIP341497 ]
            }
            
            return;
        }
    }
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: AfterWrite
//
// Description: Restores FV signature after writing to flash.
//
// Input:
//  VOID*       FlashAddress    Pointer to address of a flash 
//  UINTN       Size            Size to write
//  VOID*       DataBuffer      Poiner to the Data Buffer
//
// Output:      NON
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

VOID AfterWrite(VOID* FlashAddress, UINTN Size, VOID* DataBuffer)
{
    EFI_PHYSICAL_ADDRESS Address;
    EFI_PHYSICAL_ADDRESS AreaAddress;
    INT32 i;
    
    Address = (EFI_PHYSICAL_ADDRESS)(UINTN)FlashAddress;
    for (i=0; i<NumberOfUpdatedAreas; i++)
    {
        AreaAddress = UpdatedArea[i].RomArea->Address+FlashDeviceBase;
        
        if (AreaAddress==Address)
        {
            //Restore original data
            ((EFI_FIRMWARE_VOLUME_HEADER*)DataBuffer)->Signature=FV_SIGNATURE;
        }
        
        if (AreaAddress+UpdatedArea[i].AreaSize==Address+Size)
        {
            UINT32 FvSignature = FV_SIGNATURE;
            //Restore FV signature
            FlashDriverWrite(
                &((EFI_FIRMWARE_VOLUME_HEADER*)AreaAddress)->Signature,
                sizeof(UINT32),
                &FvSignature
            );
            UpdatedArea[i] = UpdatedArea[NumberOfUpdatedAreas-1];
            NumberOfUpdatedAreas--;
        }
    }
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: AfterRead
//
// Description: Hook that is being called after the read operation.
//
// Input:
//  VOID*       FlashAddress    Pointer to address of a flash 
//  UINTN       Size            Size to read
//
// Output:      NON
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

VOID AfterRead(VOID* FlashAddress, UINTN Size, VOID* DataBuffer)
{
    INT32 i;

    if (FlashData.MailBox->WriteEnableStatus==0) return;
    // If we are in the middle of flash update 
    // (FlashData.MailBox->WriteEnableStatus is not zero), 
    // it may happen that the FV signature has not been restored yet 
    // (typically happens during partial FV updates).
    // Let's return the proper value. The signature will be restored later.
    for (i=0; i<NumberOfUpdatedAreas; i++)
    {
        EFI_FIRMWARE_VOLUME_HEADER* Fv 
            = (EFI_FIRMWARE_VOLUME_HEADER*)(
                UpdatedArea[i].RomArea->Address+FlashDeviceBase
              );
        if ( Fv == FlashAddress && Size >= EFI_FIELD_OFFSET(EFI_FIRMWARE_VOLUME_HEADER, Attributes))
        {
            ((EFI_FIRMWARE_VOLUME_HEADER*)DataBuffer)->Signature = FV_SIGNATURE;
            return;
        }
    }
}

// Protocl Functions Implementation
// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: FlashDriverRead
//
// Description: Implementation of the Read Function of the Flash protocol
//
// Input:
//  VOID*       FlashAddress    Pointer to address of a flash 
//  UINTN       Size            Size to write
//  VOID*       DataBuffer      Poiner to the Data Buffer to write
//
// Output:      NON
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

EFI_STATUS EFIAPI FlashDriverRead(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
)
{
    BOOLEAN Status;

    if (FlashData.MailBox->WriteEnableStatus!=0)
    {
        Status = FlashRead(FlashAddress, DataBuffer, (UINT32)Size);
    }
    else
    {
        UINTN i;
        
        for (i=0; i<Size; i++)
            ((UINT8*)DataBuffer)[i] = ((UINT8*)FlashAddress)[i];
            
        Status = TRUE;
    }
    return (Status) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

#define INT_SIZE sizeof(INTN)
#define FLASH_EMPTY_BYTE ((UINT8)FlashEmpty)

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: IsClean
//
// Description: Function to loops through the buffer and check if it's empty
//
// Input:
//  IN UINT8*   Address    Pointer to address to check
//  IN UINTN    Size    Size of data to check
//
// Output:      TRUE or FALSE
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

BOOLEAN IsClean(IN UINT8 *Address, IN UINTN Size)
{
    // loops through the buffer and check if it's empty
    if (!( (UINTN)Address & (INT_SIZE-1) ))
    {
        for ( ; Size >= INT_SIZE; Size -= INT_SIZE, Address += INT_SIZE)
        {
            if (FlashNotMemoryMapped)
            {
                UINTN nData=0;	
		        FlashDriverRead (Address, INT_SIZE, (UINT8*)&nData );
		        if (nData != FlashEmpty) return FALSE;
            }
            else
                if (*(UINTN*)Address != FlashEmpty) return FALSE;
	    }

    }
    
    // the address may not be INT_SIZE aligned
    // check the remaining part of the buffer
    for ( ; Size > 0; Size--, Address++)
    {
        if (FlashNotMemoryMapped)
        {
            UINT8 nData=0;	
		    FlashDriverRead (Address, 1, &nData );
		    if (nData != FLASH_EMPTY_BYTE) return FALSE;
        }
        else
            if (*Address != FLASH_EMPTY_BYTE) return FALSE;

	}

    return TRUE;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: FlashDriverErase
//
// Description: Implementation of the Erase Function of the Flash protocol
//
// Input:
//  VOID*       FlashAddress    Pointer to address of a flash to erase 
//  UINTN       Size            Size to erase
//
// Output:      EFI_SUCCESS or EFI_DEVICE_ERROR depending on result
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

EFI_STATUS EFIAPI FlashDriverErase(VOID* FlashAddress, UINTN Size)
{
    BOOLEAN Status = TRUE;
    UINT8 *Address = (UINT8*)FlashAddress;
    
    //---Enable write---------------------------------------------
    FlashDriverDeviceWriteEnable();
    
    for (; Status && Size>0; Address+=FlashBlockSize, Size-=FlashBlockSize)
    {
        //--- If block clean already - continue to next-----------
        if (IsClean(Address, FlashBlockSize)) continue;
        //---If not - Erase block---------------------------------
        FlashBlockWriteEnable(Address);
        Status=FlashEraseBlock(Address);
        FlashBlockWriteDisable(Address);
    }
    //---Disable Write-------------------------------------------
    FlashDriverDeviceWriteDisable();
    return (Status) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: FlashDriverWrite
//
// Description: Implementation of the Write Function of the Flash protocol
//
// Input:
//  VOID*       FlashAddress    Pointer to address of a flash to write to 
//  UINTN       Size            Size to write
//  VOID*       DataBuffer -    pointer to data to write to the flash part
//
// Output:      EFI_SUCCESS or EFI_DEVICE_ERROR depending on result
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

EFI_STATUS EFIAPI FlashDriverWrite(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
)
{
    BOOLEAN Status = TRUE;
    UINT8 *Address = (UINT8*)FlashAddress;
    UINT8 *Data = (UINT8*)DataBuffer;
    UINT8 *BlockAddress = (UINT8*)BLOCK(Address); //Align to the block size
    UINTN PartialSize;
    
    FlashDriverDeviceWriteEnable();
    //---If FlashAddress was not the begining of flash----------------
    if (BlockAddress!=Address)
    {
        PartialSize = FlashBlockSize - (Address - BlockAddress);
        
        if (PartialSize > Size) PartialSize = Size;
        //---Write data and update address----------------------------
        FlashBlockWriteEnable(BlockAddress);
        Status=FlashProgram(Address, Data, (UINT32)PartialSize);
        FlashBlockWriteDisable(BlockAddress);
        Address = BlockAddress + FlashBlockSize;
        Size -= PartialSize;
        Data += PartialSize; 
    }
    //---Else Write data and update address----------------------------
    for (; Status && Size>=FlashBlockSize
            ; Address+=FlashBlockSize, Size-=FlashBlockSize, Data+=FlashBlockSize
        )
    {
        FlashBlockWriteEnable(Address);
        Status=FlashProgram(Address, Data, FlashBlockSize);
        FlashBlockWriteDisable(Address);
    }
    //--- If last chunk < FlashBlockSize-------------------------------
    if (Size!=0 && Status)
    {
        FlashBlockWriteEnable(Address);
        Status=FlashProgram(Address, Data, (UINT32)Size);
        FlashBlockWriteDisable(Address);
    }
    
    FlashDriverDeviceWriteDisable();
    return (Status) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: FlashDriverUpdate
//
// Description: Implementation of the Update Function of the Flash protocol 
//
// Input:
//  VOID*       FlashAddress    Pointer to address of a flash to update (must be aligned to FlashBlockSize)
//  UINTN       Size            Size to update (must be multiple of FlashBlockSize)
//  VOID*       DataBuffer -    pointer to data to write to the flash part
//
// Output:      EFI_SUCCESS or EFI_DEVICE_ERROR depending on result
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

EFI_STATUS EFIAPI FlashDriverUpdate(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
)
{
    BOOLEAN Status = TRUE;
    UINT8 *Address = (UINT8*)FlashAddress;
    UINT8 *Data = (UINT8*)DataBuffer;
    
    FlashDriverDeviceWriteEnable();
    
    for (; Status && Size>=FlashBlockSize
            ; Address+=FlashBlockSize, Size-=FlashBlockSize, Data+=FlashBlockSize
        )
    {
        //---Writes a block checking is flash block clean or equal-----------
        Status = FlashWriteBlock(Address, Data);
    }
    
    FlashDriverDeviceWriteDisable();
    return (Status) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: FlashDriverDeviceWriteEnable
//
// Description: Enables writes to flash device 
//
// Input:       NON
// 
//
// Output:      EFI_SUCCESS
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

EFI_STATUS EFIAPI FlashDriverDeviceWriteEnable()
{
    FlashData.MailBox->WriteEnableStatus++;
    
    if (FlashData.MailBox->WriteEnableStatus==1)
    {
        FlashDeviceWriteEnable();
    }
    
    return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: FlashDriverDeviceWriteDisable
//
// Description: Disables writes to flash device
//
// Input:       NON
// 
//
// Output:      EFI_SUCCESS
//
//--------------------------------------------------------------------------
// <AMI_PHDR_END>

EFI_STATUS EFIAPI FlashDriverDeviceWriteDisable()
{
    if (FlashData.MailBox->WriteEnableStatus!=0)
    {
        FlashData.MailBox->WriteEnableStatus--;
        
        if (FlashData.MailBox->WriteEnableStatus==0)
        {
            FlashDeviceWriteDisable();
        }
    }
    
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI FlashDriverReadExt(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
){
    EFI_STATUS Status;
    BEGIN_CRITICAL_SECTION(Cs);
    Status = FlashDriverRead(FlashAddress,Size,DataBuffer);
    AfterRead(FlashAddress, Size, DataBuffer);
    END_CRITICAL_SECTION(Cs);
    return Status;
}

EFI_STATUS EFIAPI FlashDriverEraseExt(
    VOID* FlashAddress, UINTN Size
){
    EFI_STATUS Status;

    if (Size==0) return EFI_SUCCESS;
    //---If Addres in not alligned properly or Size is not multiple to FlashBlockSize
    //---Abort----------------------------------
    if ((UINT8*)BLOCK(FlashAddress)!=FlashAddress || Size%FlashBlockSize!=0)
        return EFI_UNSUPPORTED;

    BEGIN_CRITICAL_SECTION(Cs);
    //---Invalidate FV by destroying the signature----------------
    BeforeErase(FlashAddress);
    Status = FlashDriverErase(FlashAddress, Size);
    END_CRITICAL_SECTION(Cs);
    return Status;
}

EFI_STATUS EFIAPI FlashDriverWriteExt(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
){
    EFI_STATUS Status;
    if (Size==0) return EFI_SUCCESS;

    BEGIN_CRITICAL_SECTION(Cs);
    //---Invalidate FV by destroying the signature--------------------
    BeforeWrite(FlashAddress, Size, DataBuffer);
    Status = FlashDriverWrite(FlashAddress,Size,DataBuffer);
    //---Restore FV signature-------------------------------------------
    AfterWrite(FlashAddress, Size, DataBuffer);
    END_CRITICAL_SECTION(Cs);
    return Status;
}

EFI_STATUS EFIAPI FlashDriverUpdateExt(
    VOID* FlashAddress, UINTN Size, VOID* DataBuffer
){
    EFI_STATUS Status;

    if (Size==0) return EFI_SUCCESS;
    
    if ((UINT8*)BLOCK(FlashAddress)!=FlashAddress || Size%FlashBlockSize!=0)
        return EFI_UNSUPPORTED;

    BEGIN_CRITICAL_SECTION(Cs);
    BeforeErase(FlashAddress);
    BeforeWrite(FlashAddress, Size, DataBuffer);
    Status = FlashDriverUpdate(FlashAddress,Size,DataBuffer);
    AfterWrite(FlashAddress, Size, DataBuffer);
    END_CRITICAL_SECTION(Cs);
    return Status;
}

EFI_STATUS EFIAPI FlashDriverDeviceWriteEnableExt(){
    EFI_STATUS Status;
    BEGIN_CRITICAL_SECTION(Cs);
    Status = FlashDriverDeviceWriteEnable();
    END_CRITICAL_SECTION(Cs);
    return Status;
}

EFI_STATUS EFIAPI FlashDriverDeviceWriteDisableExt(){
    EFI_STATUS Status;
    UINT32 OldWriteEnableStatus;

    BEGIN_CRITICAL_SECTION(Cs);
    OldWriteEnableStatus = FlashData.MailBox->WriteEnableStatus;
    Status = FlashDriverDeviceWriteDisable();
    if (OldWriteEnableStatus!=0 && FlashData.MailBox->WriteEnableStatus==0)
    {
        // Before disabling the flash wtrites
        // restore the destroyed FV signatures (if any).
        // In case of full FV update, the FV signature is restored 
        // in the AfterWrite function once the last FV block is updated. 
        // In case of partial FV update when last FV block is not updated, 
        // the FV Signature is restored either here or in the BeforeRead function.
        INT32 i;
        for (i=0; i<NumberOfUpdatedAreas; i++)
        {
			if (UpdatedArea[i].RestoreSignature){                       // [ EIP341497 ]
                EFI_FIRMWARE_VOLUME_HEADER* Fv 
                    = (EFI_FIRMWARE_VOLUME_HEADER*)(
                        UpdatedArea[i].RomArea->Address+FlashDeviceBase
                      );
                UINT32 FvSignature = FV_SIGNATURE;
                //Restore FV signature
                FlashDriverWrite(
                    &Fv->Signature, sizeof(UINT32), &FvSignature
                );
//            UpdatedArea[i] = UpdatedArea[NumberOfUpdatedAreas-1];     // [ EIP341497 ]
//            NumberOfUpdatedAreas--;                                   // [ EIP341497 ]
			}                                                           // [ EIP341497 ]
        }
		NumberOfUpdatedAreas = 0;                                       // [ EIP341497 ]
        FlashDeviceWriteDisable();
    }

    END_CRITICAL_SECTION(Cs);
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FlashDriverEntry
//
// Description: This function is the entry point for this module. This function
//              installs Flash protocols before and after SMM.
//
// Input:       ImageHandle Image handle
//              SystemTable Pointer to the system table
//
// Output:      Return Status based on errors that occurred while waiting for
//              time to expire.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS EFIAPI FlashDriverEntry(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable
)
{
    InitAmiLib(ImageHandle,SystemTable);
    return InitSmmHandlerEx(
               ImageHandle, SystemTable, FlashDriverSmmInit, FlashDriverInit
           );
}
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
