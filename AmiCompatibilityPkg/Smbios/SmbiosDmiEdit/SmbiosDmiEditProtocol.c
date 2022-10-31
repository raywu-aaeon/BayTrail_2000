//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//

//**********************************************************************//
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************//
//**********************************************************************//

#include <AmiDxeLib.h>
#include <Token.h>
#include <Protocol\AmiSmbios.h>
#include "SmbiosDmiEdit.h"

typedef enum {
	AmiSmbiosFlashDataProtocolCallback,
	FlashProtocolCallback
} PROTOCOL_CALLBACK_TYPE;

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || (SMBIOS_DMIEDIT_DATA_LOC != 2)
#include <Protocol\SmbiosGetFlashDataProtocol.h>
#include <Protocol\FlashProtocol.h>

FLASH_PROTOCOL					*FlashProtocol;
PROTOCOL_CALLBACK_TYPE			ProtocolCallbackType;
#endif                                  // SMBIOS_DMIEDIT_DATA_LOC

EFI_HANDLE           			gImageHandle;
EFI_GUID    					gEfiAmiDmieditSmbiosProtocolGuid = AMI_DMIEDIT_SMBIOS_GUID;

EFI_SMBIOS_DMIEDIT_PROTOCOL     SmbiosDmieditProtocol = {DmiEditNonSmiHandler};

//----------------------------------------------------------------------------
//  External Variables
//----------------------------------------------------------------------------
extern SMBIOS_TABLE_ENTRY_POINT	*SmbiosTableEntryPoint;
extern UINT8                    *ScratchBufferPtr;
extern UINT16					MaximumBufferSize;

//----------------------------------------------------------------------------
//  External Function Declaration
//----------------------------------------------------------------------------
extern VOID EnableShadowWrite();
extern VOID DisableShadowWrite();
extern VOID GetSmbiosTableF000 (VOID);

extern UINT16 GetSmbiosInfo(
    IN OUT  GET_SMBIOS_INFO   *p
);

extern UINT16 GetSmbiosStructure(
    IN OUT  GET_SMBIOS_STRUCTURE    *p
);

extern UINT16 SetSmbiosStructure(
    IN SET_SMBIOS_STRUCTURE    *p
);

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || (SMBIOS_DMIEDIT_DATA_LOC != 2)
extern FLASH_DATA_INFO GetFlashDataInfo(
    IN TABLE_INFO   *RecordInfo
);
#endif

//----------------------------------------------------------------------------
//  Function Declaration
//----------------------------------------------------------------------------
VOID
SmbiosDmiEditCallbackHandler(
	IN EFI_EVENT	Event,
	IN VOID			*Context
);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SmbiosDmiEditProtocolEntryPoint
//
// Description: SmbiosDmiEditProtocol driver entry point
//
// Input:       IN EFI_HANDLE           ImageHandle,
//              IN EFI_SYSTEM_TABLE     *SystemTable
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SmbiosDmiEditProtocolEntryPoint(
    IN EFI_HANDLE           ImageHandle,
	IN EFI_SYSTEM_TABLE     *SystemTable
)
{
	EFI_STATUS				Status;
    AMI_SMBIOS_PROTOCOL		*AmiSmbiosProtocol;

	InitAmiLib(ImageHandle, SystemTable);
	gImageHandle = ImageHandle;

    Status = pBS->LocateProtocol (&gAmiSmbiosProtocolGuid, NULL, &AmiSmbiosProtocol);
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    SmbiosTableEntryPoint = AmiSmbiosProtocol->SmbiosGetTableEntryPoint();
    ScratchBufferPtr = AmiSmbiosProtocol->SmbiosGetScratchBufferPtr();
	MaximumBufferSize = AmiSmbiosProtocol->SmbiosGetBufferMaxSize();

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
{
	EFI_EVENT   			ProtocolEvent;
	VOID        			*Registration;

	//
    // Register callback to install DmieditSmbiosProtocol
	// in case DmiEdit Data is in BootBlock
    //

	//
    // Register SmbiosGetFlashDataProtocol Callback
    //
	ProtocolCallbackType = AmiSmbiosFlashDataProtocolCallback;
    Status = RegisterProtocolCallback(
                            &gAmiSmbiosFlashDataProtocolGuid,
                            SmbiosDmiEditCallbackHandler,
                            &ProtocolCallbackType,
                            &ProtocolEvent,
                            &Registration
                        );
    if (EFI_ERROR(Status)) {
        return Status;
    }

    pBS->SignalEvent(ProtocolEvent);	// Check if AmiSmbiosFlashDataProtocol already started

    //
    // Register FlashProtocol Callback
    //
	ProtocolCallbackType = FlashProtocolCallback;
    Status = RegisterProtocolCallback(
                            &gFlashProtocolGuid,
                            SmbiosDmiEditCallbackHandler,
                            &ProtocolCallbackType,
                            &ProtocolEvent,
                            &Registration
                        );

    pBS->SignalEvent(ProtocolEvent);	// Check if FlashProtocol already started
}
#else
    //
    // Install DmieditSmbiosProtocol here in case DmiEdit Data is in Nvram
    //
	pBS->InstallMultipleProtocolInterfaces(
								 &gImageHandle,
								 &gEfiAmiDmieditSmbiosProtocolGuid,
								 &SmbiosDmieditProtocol,
								 NULL
								 );
	TRACE((-1, "AmiDmieditSmbiosProtocol installed\n"));
#endif                                  // SMBIOS_DMIEDIT_DATA_LOC

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   DmiEditNonSmiHandler
//
// Description: Handler for SmbiosDmiEditProtocol
//
// Input:       IN UINT8    Data
//              IN UINT64   pCommBuff
//
// Output:      None
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32
DmiEditNonSmiHandler(
    IN UINT8    Data,
    IN UINT64   pCommBuff
)
{
    UINT32      eax;
    VOID        *pInterface;

	TRACE((-1, "SMISMBIOSHandler Data: 0x%X \n",Data));
	pInterface = (void*)pCommBuff;

	GetSmbiosTableF000();

    switch(Data) {
        case 0x50:
                    eax = GetSmbiosInfo(pInterface);
	TRACE((-1, "eax : 0x%X \n",eax));
                    break;
        case 0x51:
                    eax = GetSmbiosStructure(pInterface);
	TRACE((-1, "eax : 0x%X \n",eax));
                    break;
        case 0x52:
                    EnableShadowWrite();
                    eax = SetSmbiosStructure(pInterface);
                    DisableShadowWrite();
	TRACE((-1, "eax : 0x%X \n",eax));
    }
    return eax;
}

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SmbiosDmiEditCallbackHandler
//
// Description: Install SmbiosDmieditProtocol when both FlashProtocol
//				and AmiSmbiosFlashDataProtocol are installed
//
// Input:       IN EFI_EVENT	Event,
//              IN VOID     	*Context
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
SmbiosDmiEditCallbackHandler(
	IN EFI_EVENT	Event,
	IN VOID			*Context
)
{
	EFI_STATUS						Status;
    EFI_SMBIOS_FLASH_DATA_PROTOCOL  *SmbiosFlashDataProtocol;
    static BOOLEAN					AmiSmbiosFlashDataProtocolInstalled = FALSE;
    static BOOLEAN					FlashProtocolInstalled = FALSE;

    if (*(PROTOCOL_CALLBACK_TYPE*)Context == AmiSmbiosFlashDataProtocolCallback) {
    	TRACE((-1, "AmiSmbiosFlashDataProtocol installed\n"));
		Status = pBS->LocateProtocol (&gAmiSmbiosFlashDataProtocolGuid, NULL, &SmbiosFlashDataProtocol);
		ASSERT_EFI_ERROR(Status);
		if (Status == EFI_SUCCESS) {
			SmbiosFlashDataProtocol->GetFlashTableInfo(SmbiosFlashDataProtocol, &gFlashData, &gFlashDataSize);
			AmiSmbiosFlashDataProtocolInstalled = TRUE;
		}
		else return;
    }

    if (*(PROTOCOL_CALLBACK_TYPE*)Context == FlashProtocolCallback) {
    	TRACE((-1, "FlashProtocol installed\n"));
		Status = pBS->LocateProtocol (&gFlashProtocolGuid, NULL, &FlashProtocol);
		ASSERT_EFI_ERROR(Status);
		if (Status == EFI_SUCCESS) {
			FlashProtocolInstalled = TRUE;
		}
		else return;
    }

    if (AmiSmbiosFlashDataProtocolInstalled && FlashProtocolInstalled) {
		pBS->InstallMultipleProtocolInterfaces(
									 &gImageHandle,
									 &gEfiAmiDmieditSmbiosProtocolGuid,
									 &SmbiosDmieditProtocol,
									 NULL
									 );
    	TRACE((-1, "AmiDmieditSmbiosProtocol installed\n"));
    }

	pBS->CloseEvent(Event);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateSmbiosTable
//
// Description: Searches the Flash Data Table for a record of Type and
//              Offset. If found, the existing data will be replaced with
//              the new data, else the data will be added as a new record.
//
// Input:       IN TABLE_INFO  TableInfo,
//              IN UINT8       *Data
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16
UpdateSmbiosTable(
    IN TABLE_INFO  *TableInfo,
    IN UINT8       *Data
)
{
    UINT16              	i;
    UINT8                   *BufferPtr = NULL;
    UINT32              	SpaceAvailable;
    FLASH_DATA_INFO     	FlashDataInfo;
    EFI_PHYSICAL_ADDRESS    FlashDataBlock;
    UINT8                   BlockCount;
    UINT8                   *DataBuffer;
    UINTN                   RemainingSize;

    FlashDataInfo = GetFlashDataInfo(TableInfo);

    // Check Size
    SpaceAvailable = (UINT32)((UINT8*)gFlashData + gFlashDataSize - FlashDataInfo.EndOfData);
    if (FlashDataInfo.Location) SpaceAvailable += FlashDataInfo.Size + sizeof(TABLE_INFO);

    if (sizeof(TABLE_INFO) + TableInfo->Size > SpaceAvailable) {
        return DMI_ADD_STRUCTURE_FAILED;
    }

    // Determine the base block that contains the DmiEdit data
    FlashDataBlock = (UINTN)gFlashData & (0xFFFFFFFF - FLASH_BLOCK_SIZE + 1);

    // Check to see if it spans more than one block
    if (((UINTN)gFlashData + FLASHDATA_SIZE) > (FlashDataBlock + FLASH_BLOCK_SIZE)) {
        BlockCount = 2;
    }
    else {
        BlockCount = 1;
    }

    // Allocate Flash Data buffer and get the data
    // Note: additional 4K reserved for data manipulation
    pBS->AllocatePool(EfiBootServicesData, BlockCount * FLASH_BLOCK_SIZE, &DataBuffer);
    FlashProtocol->Read((UINT8*)FlashDataBlock, BlockCount * FLASH_BLOCK_SIZE, DataBuffer);

    if (FlashDataInfo.Location) {
        // Some data is already present for input type
        // Determine location of existing data
        BufferPtr = DataBuffer + \
                    (UINTN)gFlashData - FlashDataBlock + \
                    (UINTN)FlashDataInfo.Location - (UINTN)gFlashData;      // Existing DmiEdit data location

        // Calculate remaining size from the existing data location
        // to end of DmiEdit data storage block
        RemainingSize = (UINTN)FlashDataInfo.EndOfData - \
                        (UINTN)FlashDataInfo.Location - \
                        (sizeof(TABLE_INFO) + ((TABLE_INFO*)BufferPtr)->Size);

        // Copy the remaining data (without the existing data) to location
        // after input data entry insertion
        MemCpy(BufferPtr + sizeof(TABLE_INFO) + TableInfo->Size,
                BufferPtr + sizeof(TABLE_INFO) + ((TABLE_INFO*)BufferPtr)->Size,
                RemainingSize);

        // In case new data size is smaller than existing one, clear old data
        // from (EndOfData - size difference) to EndOfData to 0xff
        if (((TABLE_INFO*)BufferPtr)->Size > TableInfo->Size) {
            UINT8       *DataEndPtr;

            RemainingSize = ((TABLE_INFO*)BufferPtr)->Size - TableInfo->Size;
            DataEndPtr = DataBuffer + \
                        ((UINTN)FlashDataInfo.EndOfData - FlashDataBlock) - \
                        RemainingSize;

            for (i = 0; i < RemainingSize; ++i) {
                *DataEndPtr++ = 0xff;
            }
        }
    }
    else {
        // Determine the end location of current DmiEdit data
        BufferPtr = DataBuffer + \
                    (UINTN)gFlashData - FlashDataBlock + \
                    (UINTN)FlashDataInfo.EndOfData - (UINTN)gFlashData;     // End of DmiEdit data
    }

    // Insert data
    *(TABLE_INFO *)BufferPtr = *TableInfo;
    BufferPtr += sizeof(TABLE_INFO);

    for(i = 0; i < TableInfo->Size; ++i) {
        *BufferPtr++ = Data[i];
    }

    // Update DmiEdit data block
    FlashProtocol->Update((UINT8*)FlashDataBlock, BlockCount * FLASH_BLOCK_SIZE, DataBuffer);

    pBS->FreePool(DataBuffer);

    return 0;
}
#endif							// SMBIOS_DMIEDIT_DATA_LOC

//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//
