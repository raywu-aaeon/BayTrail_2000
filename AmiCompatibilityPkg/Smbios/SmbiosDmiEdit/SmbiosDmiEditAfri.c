//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2015, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//

/** @file SmbiosDmiEditAfri.c
    This file contains AMI Firmware Runtime Interface (AFRI) registration
    and handler codes

**/

#include <Token.h>
#include <AmiDxeLib.h>
#include <FlashPart.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Protocol/AfriProtocol.h>
#include <Protocol/AmiSmbios.h>
#include "SmbiosDmiEdit.h"
#include "SmbiosDmiEditAfri.h"

#if defined(SMBIOS_DMIEDIT_DATA_LOC) && (SMBIOS_DMIEDIT_DATA_LOC == 0)
#include <Protocol/SmbiosGetFlashDataProtocol.h>
#endif                                          // SMBIOS_DMIEDIT_DATA_LOC

//----------------------------------------------------------------------------
//  External Variables
//----------------------------------------------------------------------------
#if SMBIOS_2X_SUPPORT
extern  SMBIOS_TABLE_ENTRY_POINT    *SmbiosTableEntryPoint;
extern  UINTN                       Smb2xTablePhysAddress;
#endif                                          // SMBIOS_2X_SUPPORT
#if SMBIOS_3X_SUPPORT
extern  SMBIOS_3X_TABLE_ENTRY_POINT *SmbiosV3TableEntryPoint;
extern  UINTN                       Smb3xTablePhysAddress;
#endif                                          // SMBIOS_3X_SUPPORT
extern  UINT8                       *ScratchBufferPtr;
extern  UINT16                      MaximumBufferSize;
extern  VOID                        *StringTable[];
extern  UINTN                       StringTableSize;
#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || (SMBIOS_DMIEDIT_DATA_LOC == 2)
extern  CHAR16                      *DmiArrayVar;
extern  UINT8                       *DmiData;
extern  CHAR16                      *Var;
#endif                                          // SMBIOS_DMIEDIT_DATA_LOC

//----------------------------------------------------------------------------
//  External Function Declaration
//----------------------------------------------------------------------------
extern  VOID EnableShadowWrite();
extern  VOID DisableShadowWrite();
extern  VOID WriteOnceStatusInit(VOID);
extern  BOOLEAN FindStructureType(
                        IN OUT UINT8    **Buffer,
                        IN OUT UINT8    **StructureFoundPtr,
                        IN     UINT8    SearchType,
                        IN     UINT8    Instance        // 1-based
                        );

extern  BOOLEAN FindString(
                        IN OUT UINT8    **BufferPtr,
                        IN     UINT8    StringNumber    // 1-based
                        );

// For Smbios version 2.x
#if (SMBIOS_2X_SUPPORT == 1)
extern UINT16 GetSmbiosV2Info(
                        IN OUT  GET_SMBIOS_INFO     *p
                        );

extern UINT16 GetSmbiosV2Structure(
                        IN OUT  GET_SMBIOS_STRUCTURE    *p
                        );

extern UINT16 SetSmbiosV2Structure(
                        IN OUT  SET_SMBIOS_STRUCTURE     *p
                        );
#endif                                          // SMBIOS_2X_SUPPORT

// For Smbios version 3.x
#if (SMBIOS_3X_SUPPORT == 1)
extern UINT16 GetSmbiosV3Info(
                        IN OUT  GET_SMBIOS_V3_INFO  *p
                        );

extern UINT16 GetSmbiosV3Structure(
                        IN OUT  GET_SMBIOS_V3_STRUCTURE    *p
                        );

extern UINT16 SetSmbiosV3Structure(
                        IN SET_SMBIOS_V3_STRUCTURE  *p
                        );
#endif                                          // SMBIOS_3X_SUPPORT

//----------------------------------------------------------------------------
//  Function Declaration
//----------------------------------------------------------------------------
VOID
GetSmbiosPointerCallback (IN EFI_EVENT Event, IN VOID *Context);

BOOLEAN
IsGotSmbiosPointers (VOID);

VOID
GetSmbiosPointers (VOID);

#if defined(SMBIOS_DMIEDIT_DATA_LOC) && (SMBIOS_DMIEDIT_DATA_LOC == 0)
EFI_STATUS
GetFlashDataLocation(VOID);
#endif                                          // SMBIOS_DMIEDIT_DATA_LOC

VOID
DmiEditSsiVirtAddressChange (
    IN EFI_EVENT Event,
    IN VOID *Context
);

EFI_STATUS
AmiFriHandler (
    IN  EFI_HANDLE  DispatchHandle,
    IN  CONST VOID  *DispatchContext,
    IN  OUT VOID    *CommBuffer,
    IN  OUT UINTN   *CommBufferSize
);

//----------------------------------------------------------------------------
//  Global Variables
//----------------------------------------------------------------------------
EFI_GUID    gDmiEditFriGuid = DMIEDIT_FRI_GUID;
EFI_HANDLE  DmiEditFriHandle = NULL;
EFI_EVENT	DmiEditAfriVirAddrChangeEvent;
UINT8       Buffer[2 * FLASH_BLOCK_SIZE];
UINT8       ScratchBuffer[2 * FLASH_BLOCK_SIZE];
UINT8       *gVirtBufferPtr = Buffer;
UINT8       *gFlashBlockPtr;
UINT8       *gVirtFlashBlockPtr;
UINTN       gDmiEditTableSize;
UINTN       gDmiEditTableOffset;            // Offset from start of flash block
UINTN       gCurrentDmiEditOffset;          // Offset from start of flash block
BOOLEAN     DmiEditTableIn2Blocks = FALSE;

EFI_EVENT   SmbiosPointerEvent;
EFI_GUID    EfiSmbiosPointerGuid  = AMI_SMBIOS_POINTER_GUID;

//----------------------------------------------------------------------------
//  Functions
//----------------------------------------------------------------------------

/**
    SmbiosDmiEditAfri support driver entry point

    @param  ImageHandle
    @param  SystemTable

    @return EFI_STATUS

**/
EFI_STATUS
SmbiosDmiEditAfriInstall(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    EFI_STATUS              Status;
    AMI_FRI_PROTOCOL        *AmiFriProtocol = NULL;

	InitAmiLib(ImageHandle, SystemTable);

    Status = SystemTable->BootServices->LocateProtocol(&gAmiFriProtocolGuid,
                                                       NULL,
                                                       (VOID **)&AmiFriProtocol
                                                      );
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    WriteOnceStatusInit();

    Status = AmiFriProtocol->RegisterAfriHandler(AmiFriHandler, &gDmiEditFriGuid, &DmiEditFriHandle);
    ASSERT_EFI_ERROR(Status);

    Status = pBS->CreateEventEx(EVT_NOTIFY_SIGNAL,
    							TPL_CALLBACK,
                                GetSmbiosPointerCallback,
                                NULL,
                                &EfiSmbiosPointerGuid,
                                &SmbiosPointerEvent
                                );
    ASSERT_EFI_ERROR(Status);

    Status = pBS->CreateEventEx(EVT_NOTIFY_SIGNAL,
                                TPL_NOTIFY,
                                DmiEditSsiVirtAddressChange,
                                NULL,
                                &gEfiEventVirtualAddressChangeGuid,
                                &DmiEditAfriVirAddrChangeEvent
                                );
    ASSERT_EFI_ERROR(Status);

    return Status;
}

/**
    Callback function to get various Smbios Pointers

    @param  Event
    @param  *Context

    @return None

**/
VOID
GetSmbiosPointerCallback(
    IN EFI_EVENT    Event,
    IN VOID         *Context
)
{
    GetSmbiosPointers();
#if defined(SMBIOS_DMIEDIT_DATA_LOC) && (SMBIOS_DMIEDIT_DATA_LOC == 0)
    GetFlashDataLocation();
    gVirtFlashBlockPtr = gFlashBlockPtr;
#endif                                          // SMBIOS_DMIEDIT_DATA_LOC
    pBS->CloseEvent(Event);
}

/**
    Converts global pointers to virtual addresses

    @param  Event
    @param  *Context

    @return None

**/
VOID
DmiEditSsiVirtAddressChange (
    IN EFI_EVENT    Event,
    IN VOID         *Context
)
{
    UINTN   i;
    VOID    *PtrArray[] = { // Make sure PtrArray is using un-converted pointer
#if (SMBIOS_2X_SUPPORT == 1)
                            &SmbiosTableEntryPoint->TableAddress,
                            &SmbiosTableEntryPoint,
#endif                                          // SMBIOS_2X_SUPPORT
#if (SMBIOS_3X_SUPPORT == 1)
                            &SmbiosV3TableEntryPoint->TableAddress,
                            &SmbiosV3TableEntryPoint,
#endif                                          // SMBIOS_3X_SUPPORT
                            &ScratchBufferPtr,
#if defined(SMBIOS_DMIEDIT_DATA_LOC) && (SMBIOS_DMIEDIT_DATA_LOC == 0)
                            &gFlashData,
                            &gVirtBufferPtr,
                            &gVirtFlashBlockPtr,
#else
                            &DmiArrayVar,
                            &DmiData,
                            &Var,
#endif                                          // SMBIOS_DMIEDIT_DATA_LOC
                            &StringTable
                          };

    if (!IsGotSmbiosPointers()) {
        return;
    }

#if (SMBIOS_2X_SUPPORT == 1)
    Smb2xTablePhysAddress = SmbiosTableEntryPoint->TableAddress;
#endif
#if (SMBIOS_3X_SUPPORT == 1)
    Smb3xTablePhysAddress = SmbiosV3TableEntryPoint->TableAddress;
#endif

    FlashVirtualFixup(pRS);

    for (i = 0; i < sizeof(PtrArray)/sizeof(VOID*); i++) {
        pRS->ConvertPointer(0, PtrArray[i]);
    }

    for (i = 0; i < StringTableSize/sizeof(VOID*); i++) {
        pRS->ConvertPointer(0, StringTable[i]);
    }
}

/**
    Find the Flash Data file in the FV

    @param  None

    @return EFI_STATUS

**/
EFI_STATUS
GetFlashDataLocation (VOID)
{
    EFI_STATUS      Status;
    UINTN           i;
    UINTN           index;
    UINT32          *BufferPtr;
    BOOLEAN         DmiEditDataFound = FALSE;

    for (i = 0; i < FV_BB_BLOCKS; i++) {
        gFlashBlockPtr = FLASH_BB_BASE_ADDR + (UINT8*)(i * FLASH_BLOCK_SIZE);
        FlashRead(gFlashBlockPtr, Buffer, FLASH_BLOCK_SIZE);

        for (index = 0; index < FLASH_BLOCK_SIZE; index += 4) {
            BufferPtr = (UINT32*)&Buffer[index];
            if (*BufferPtr == 0x5f415342) {     // "_ASB"
                gDmiEditTableSize = *(UINT16*)&Buffer[index + 4];
                gDmiEditTableOffset = index + 8;

                // if DmiEdit data table spans across block boundary, read next block also
                if ((gDmiEditTableOffset + gDmiEditTableSize) > FLASH_BLOCK_SIZE) {
                    DmiEditTableIn2Blocks = TRUE;
                    FlashRead(gFlashBlockPtr + FLASH_BLOCK_SIZE, &Buffer[FLASH_BLOCK_SIZE], FLASH_BLOCK_SIZE);
                }

                DmiEditDataFound = TRUE;
                break;
            }
        }

        if (DmiEditDataFound) {
            Status = EFI_SUCCESS;
            break;
        }

        Status = EFI_NOT_FOUND;
    }

    return Status;
}

/**
    Get various pointers from NVRAM

    @param  None

    @return None

**/
VOID
GetSmbiosPointers (VOID)
{
    UINTN       Size;

#if SMBIOS_2X_SUPPORT
    Size = sizeof(SMBIOS_TABLE_ENTRY_POINT*);
    pRS->GetVariable(L"SmbiosEntryPointTable",
                            &gAmiSmbiosNvramGuid,
                            NULL,
                            &Size,
                            &SmbiosTableEntryPoint);
#if DMIEDIT_DEBUG_TRACE
    TRACE((-1, "SmbiosDataTable at %016lx\n", SmbiosTableEntryPoint));
#endif
#endif                                          // SMBIOS_2X_SUPPORT

#if SMBIOS_3X_SUPPORT
    Size = sizeof(SMBIOS_3X_TABLE_ENTRY_POINT*);
    pRS->GetVariable(L"SmbiosV3EntryPointTable",
                            &gAmiSmbiosNvramGuid,
                            NULL,
                            &Size,
                            &SmbiosV3TableEntryPoint);
#if DMIEDIT_DEBUG_TRACE
    TRACE((-1, "SmbiosV3EntryPointTable at %016lx\n", SmbiosV3TableEntryPoint));
#endif
#endif                                          // SMBIOS_3X_SUPPORT

    Size = sizeof(UINT8*);
    pRS->GetVariable(L"SmbiosScratchBuffer",
                            &gAmiSmbiosNvramGuid,
                            NULL,
                            &Size,
                            &ScratchBufferPtr);
#if DMIEDIT_DEBUG_TRACE
    TRACE((-1, "Scratch Buffer at %016lx\n", ScratchBufferPtr));
#endif

    Size = sizeof(UINT16);
    pRS->GetVariable(L"MaximumTableSize",
                            &gAmiSmbiosNvramGuid,
                            NULL,
                            &Size,
                            &MaximumBufferSize);
}

/**
    Check if all required pointers are valid

    @param  None

    @return Boolean
                TRUE  All pointers are valid
                FALSE Invalid pointer
**/
BOOLEAN
IsGotSmbiosPointers (VOID)
{
    UINT32 IsNotGot = 0;

#if SMBIOS_2X_SUPPORT
    if (SmbiosTableEntryPoint == NULL) {
        IsNotGot |= 0x1 << 0;
    }
#endif                                          // SMBIOS_2X_SUPPORT

#if SMBIOS_3X_SUPPORT
    if (SmbiosV3TableEntryPoint == NULL) {
        IsNotGot |= 0x1 << 1;
    }
#endif                                          // SMBIOS_3X_SUPPORT

    if (ScratchBufferPtr == NULL) {
        IsNotGot |= 0x1 << 2;
    }

    if (MaximumBufferSize == 0) {
        IsNotGot |= 0x1 << 3;
    }

    if (IsNotGot) {
        return FALSE;
    }

    return TRUE;
}

/**
    Handles the SMI through AFRI

    @param  EFI_HANDLE  DispatchHandle
            CONST VOID  *Context
            OUT VOID    *CommBuffer
            OUT UINT    *CommBufferSize

    @return EFI_STATUS

**/
EFI_STATUS
AmiFriHandler (
    IN  EFI_HANDLE  DispatchHandle,
    IN  CONST VOID  *Context,
    IN  OUT VOID    *CommBuffer,        // Pointer to input of FN50-52, 58-5A
    IN  OUT UINTN   *CommBufferSize
)
{
    UINT8           FriFn;
    VOID            *pInterface;
    UINT16          RetStatus;

#if DMIEDIT_DEBUG_TRACE
    TRACE((-1, "In AmiFriHandler\n"));
#endif

    if (!IsGotSmbiosPointers()) {      // Don't overwrite the pointer, if got before
        GetSmbiosPointers();
    }

    FriFn = ((FRI_DMI_CTRL*)CommBuffer)->FriFun;
    pInterface = (VOID*)((FRI_DMI_CTRL*)CommBuffer)->BuffAddr;   // Pointer to input of FN50-52, 58-5A

    RetStatus = DMI_FUNCTION_NOT_SUPPORTED;

    switch(FriFn) {
#if (SMBIOS_2X_SUPPORT == 1)
        case 0x50:  if (IsGotSmbiosPointers()) RetStatus = GetSmbiosV2Info(pInterface);
                    ((GET_SMBIOS_INFO*)pInterface)->RetStatus = RetStatus;
                    break;
        case 0x51:  if (IsGotSmbiosPointers()) RetStatus = GetSmbiosV2Structure(pInterface);
                    ((GET_SMBIOS_STRUCTURE*)pInterface)->RetStatus = RetStatus;
                    break;
        case 0x52:  if (IsGotSmbiosPointers()) {
                        EnableShadowWrite();
                        RetStatus = SetSmbiosV2Structure(pInterface);
                        DisableShadowWrite();
                    }
                    ((SET_SMBIOS_V3_STRUCTURE*)pInterface)->RetStatus = RetStatus;
                    break;
#else
        case 0x50:  ((GET_SMBIOS_INFO*)pInterface)->RetStatus = RetStatus;
                    break;
        case 0x51:  ((GET_SMBIOS_STRUCTURE*)pInterface)->RetStatus = RetStatus;
                    break;
        case 0x52:  ((SET_SMBIOS_STRUCTURE*)pInterface)->RetStatus = RetStatus;
                    break;
#endif                                          // SMBIOS_2X_SUPPORT
#if (SMBIOS_3X_SUPPORT == 1)
        case 0x53:  switch(*(UINT8*)pInterface) {
                        case 0x58:  if (IsGotSmbiosPointers()) RetStatus = GetSmbiosV3Info(pInterface);
                                    ((GET_SMBIOS_V3_INFO*)pInterface)->RetStatus = RetStatus;
                                    break;
                        case 0x59:  if (IsGotSmbiosPointers()) RetStatus = GetSmbiosV3Structure(pInterface);
                                    ((GET_SMBIOS_V3_STRUCTURE*)pInterface)->RetStatus = RetStatus;
                                    break;
                        case 0x5a:  if (IsGotSmbiosPointers()) {
                                        EnableShadowWrite();
                                        FlashDeviceWriteEnable();
                                        RetStatus = SetSmbiosV3Structure(pInterface);
                                        FlashDeviceWriteDisable();
                                        DisableShadowWrite();
                                    }
                                    ((SET_SMBIOS_V3_STRUCTURE*)pInterface)->RetStatus = RetStatus;
                                    break;
                        case 0x5b:  if (IsGotSmbiosPointers()) RetStatus = GetSmbiosV3Table(pInterface);
                        			((GET_SMBIOS_V3_TABLE*)pInterface)->RetStatus = RetStatus;
                                    break;
                    }
#else
        case 0x53:  ((GET_SMBIOS_V3_INFO*)pInterface)->RetStatus = RetStatus;
                    break;
#endif                                          // SMBIOS_3X_SUPPORT
    }

#if DMIEDIT_DEBUG_TRACE
    TRACE((-1, "Exiting Smbios SSI Handler - Status = %r\n", RetStatus));
#endif

    return EFI_SUCCESS;
}

#if defined(SMBIOS_DMIEDIT_DATA_LOC) && (SMBIOS_DMIEDIT_DATA_LOC == 0)
/**
    Write to the flash part starting at "Address" for a length
    of "Size"

    @param  VOID    *FlashBlockAddress
            VOID    *Data

    @return EFI_STATUS

**/
EFI_STATUS
BlockWriteToFlash(
    IN VOID    *FlashBlockAddress,
    IN VOID    *Data
)
{
    EFI_STATUS  Status;

    FlashDeviceWriteEnable();
    FlashBlockWriteEnable(FlashBlockAddress);

    FlashEraseBlock(FlashBlockAddress);

    if (FlashProgram(FlashBlockAddress, Data, FLASH_BLOCK_SIZE)) {
        Status = EFI_SUCCESS;
    }
    else {
        Status = EFI_INVALID_PARAMETER;
    }

    FlashBlockWriteDisable(FlashBlockAddress);
    FlashDeviceWriteDisable();

    return Status;
}

/**
    Searches the Flash Data Table for a record of Type and
    Offset. If found, returns the location found, the data size,
    and the end of data

    @param  TABLE_INFO  *RecordInfo

    @return FLASH_DATA_INFO

**/
FLASH_DATA_INFO
GetFlashData(
    IN TABLE_INFO   *RecordInfo
)
{

    TABLE_INFO          *FlashDataPtr;
    FLASH_DATA_INFO     FlashDataInfo = {0, 0, 0};

    FlashDataPtr = (TABLE_INFO*)&Buffer[gDmiEditTableOffset];
    gCurrentDmiEditOffset = 0;

    while (FlashDataPtr->Handle != 0xffff) {
        if (FlashDataPtr->Type == RecordInfo->Type &&
            FlashDataPtr->Handle == RecordInfo->Handle &&
            FlashDataPtr->Offset == RecordInfo->Offset &&
            FlashDataPtr->Flags == RecordInfo->Flags) {
            FlashDataInfo.Location = (UINT8*)FlashDataPtr;
            FlashDataInfo.Size = FlashDataPtr->Size;
            gCurrentDmiEditOffset = (UINT8*)FlashDataPtr - Buffer;
        }

        FlashDataPtr = (TABLE_INFO*)((UINT8*)(FlashDataPtr + 1) + FlashDataPtr->Size);
    }

    FlashDataInfo.EndOfData = (UINT8*)FlashDataPtr;
    if (gCurrentDmiEditOffset == 0) {
        gCurrentDmiEditOffset = (UINT8*)FlashDataPtr - Buffer;
    }

    return FlashDataInfo;
}

/**
    Searches the Flash Data Table for a record of Type and
    Offset. If found, the existing data will be replaced with
    the new data, else the data will be added as a new record

    @param  TABLE_INFO  *RecordInfo
    @param  UINT8       *Data

    @return UINT16      Status

**/
UINT16
UpdateSmbiosTable(
    IN TABLE_INFO  *TableInfo,
    IN UINT8       *Data
)
{
    EFI_STATUS          Status;
    FLASH_DATA_INFO     FlashDataInfo;
    UINTN               SpaceAvailable;
    UINT8               *BufferPtr = NULL;
    UINTN               i;
    UINTN               j;
    UINTN               LastDataIndex;
    UINT8               InputDataBuffer[0x80];

    FlashDataInfo = GetFlashData(TableInfo);

    // Check Size
    SpaceAvailable = gDmiEditTableSize - (FlashDataInfo.EndOfData - Buffer);
    if (FlashDataInfo.Location) SpaceAvailable += FlashDataInfo.Size + sizeof(TABLE_INFO);
    if (sizeof(TABLE_INFO) + TableInfo->Size > SpaceAvailable) {
        return DMI_ADD_STRUCTURE_FAILED;
    }

    // Copy TableInfo header into input Buffer
    BufferPtr = InputDataBuffer;
    *(TABLE_INFO *)BufferPtr = *TableInfo;
    BufferPtr += sizeof(TABLE_INFO);

    // Add data to input buffer
    for (i = 0; i < TableInfo->Size; ++i) {
        *BufferPtr++ = Data[i];
    }

    // Copy flash block (2 blocks) to scratch buffer
    MemCpy(ScratchBuffer, Buffer, 2 * FLASH_BLOCK_SIZE);

    // Copy input data to current location
    for (i = 0, j = gCurrentDmiEditOffset; i < TableInfo->Size + sizeof(TABLE_INFO); i++, j++) {
        ScratchBuffer[j] = InputDataBuffer[i];
    }
    LastDataIndex = j;

    // Clear the rest of DmiEdit data in scratch buffer to 0xFF
    for (i = gDmiEditTableOffset + gDmiEditTableSize - j; i < 0; i--, j++) {
        ScratchBuffer[j] = 0xff;
    }

    if (FlashDataInfo.Location) {
        // Skip over old data
        BufferPtr = FlashDataInfo.Location;
        BufferPtr += ((TABLE_INFO*)BufferPtr)->Size + sizeof(TABLE_INFO);
        while (BufferPtr < FlashDataInfo.EndOfData) {
            ScratchBuffer[LastDataIndex++] = *BufferPtr++;
        }
    }

    // Copy ScratchBuffer to Buffer (2 blocks)
    MemCpy(Buffer, ScratchBuffer, 2 * FLASH_BLOCK_SIZE);

    Status = BlockWriteToFlash(gVirtFlashBlockPtr, gVirtBufferPtr);

    // if DmiEdit data table spans across block boundary, flash the next block
    if ((Status == EFI_SUCCESS) && DmiEditTableIn2Blocks) {
        Status = BlockWriteToFlash(gVirtFlashBlockPtr + FLASH_BLOCK_SIZE, gVirtBufferPtr + FLASH_BLOCK_SIZE);
    }

    if (EFI_ERROR(Status)) {
        return DMI_BAD_PARAMETER;
    }
    else {
        return DMI_SUCCESS;
    }
}
#endif                                          // SMBIOS_DMIEDIT_DATA_LOC

//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2015, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//
