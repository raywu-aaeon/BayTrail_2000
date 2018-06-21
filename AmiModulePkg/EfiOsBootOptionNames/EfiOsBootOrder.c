//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
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
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name: EfiOsBootOrder.c
//
// Description:	This file BootOrder related functions and Created
// Efi Os Boot Option.
//
//<AMI_FHDR_END>
//***********************************************************************

#include <BootOptions.h>
#include "EfiOsNamesFilePathMaps.h"

#ifdef  BootOption_x64
#define EFI_BOOT_FILE_NAME L"\\EFI\\BOOT\\BOOTX64.EFI"
#else
#define EFI_BOOT_FILE_NAME L"\\EFI\\BOOT\\BOOTIA32.EFI"
#endif

#pragma pack(1)
typedef struct _PARTITION_ENTRY {
    CHAR8 ActiveFlag;               	// Bootable or not
    CHAR8 StartingTrack;            	// Not used
    CHAR8 StartingCylinderLsb;      	// Not used
    CHAR8 StartingCylinderMsb;      	// Not used
    CHAR8 PartitionType;            	// 12 bit FAT, 16 bit FAT etc.
    CHAR8 EndingTrack;              	// Not used
    CHAR8 EndingCylinderLsb;        	// Not used
    CHAR8 EndingCylinderMsb;        	// Not used
    UINT32 StartSector;          	// Relative sectors
    UINT32 PartitionLength;         	// Sectors in this partition
} PARTITION_ENTRY;

typedef struct
{
    CHAR16 *FilePath;
    CHAR16 *BootOptionName;
} NameMap;
#pragma pack()

NameMap FILE_NAME_MAPS[] = { EfiOsFilePathMaps {NULL,NULL} };	//(EIP103672+)

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name: GetHdNode
//
// Description:
//  Locates HD node in DevicePath associated with Handle.
//
// Input:
//  IN EFI_HANDLE Handle - Handle with DevicePath protocol for which HD
//      node should be located.
//
// Output:
//  OUT EFI_DEVICE_PATH_PROTOCOL** DevPath - Pointer to HD node, if found.
//
// Returns:
//  EFI_SUCCESS - HD node was found and returned.
//  EFI_NOT_FOUND - No HD node was found.
//  Other errors possible if Handle does not have DevicePath protocol.
//
// Modified:
//  None.
//
// Referrals:
//  HandleProtocol()
//  isEndNode()
//  NEXT_NODE()
//
// Notes:
//  None.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetHdNode (
    IN EFI_HANDLE Handle,
    OUT EFI_DEVICE_PATH_PROTOCOL** DevPath
)
{
    EFI_STATUS Status;
    EFI_DEVICE_PATH_PROTOCOL* DevicePath;

    *DevPath = NULL;

    // Get DevicePath attached to handle.
    Status = pBS->HandleProtocol (
                 Handle,
                 &gEfiDevicePathProtocolGuid,
                 &DevicePath
             );
    if (EFI_ERROR(Status)) {
        TRACE((-1, "HandleProtocol: %r\n", Status));
        return Status;
    }

    // Find hard drive node.
    while (!isEndNode(DevicePath)) {

        if ((DevicePath->Type == MEDIA_DEVICE_PATH) &&
                (DevicePath->SubType == MEDIA_HARDDRIVE_DP)) {

            *DevPath = DevicePath;
            return EFI_SUCCESS;
        }

        DevicePath = NEXT_NODE(DevicePath);
    }

    // HD node was not found.  Return error.
    return EFI_NOT_FOUND;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	GetGptPartitionHandle
//
// Description:	search GPT HDD and return Hard disk handle
//
// Input:		EFI_DEVICE_PATH_PROTOCOL *DevicePath
//
// Output:		EFI_HANDLE
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_HANDLE GetGptPartitionHandle(EFI_DEVICE_PATH_PROTOCOL *DevicePath)
{
    EFI_STATUS	Status;
    EFI_HANDLE	*Handle, TempHandle = NULL;
    UINTN	Count, i;

    HARDDRIVE_DEVICE_PATH* BootParitionDevicePath  = (HARDDRIVE_DEVICE_PATH*)DevicePath;

    //get list of available Block I/O devices
    Status = pBS->LocateHandleBuffer(ByProtocol,&gEfiBlockIoProtocolGuid,NULL,&Count,&Handle);
    if (EFI_ERROR(Status)) return NULL;

    for ( i=0; i<Count; i++ )
    {
        EFI_BLOCK_IO_PROTOCOL		*BlockIo;
        EFI_DEVICE_PATH_PROTOCOL	*PartitionDevicePath, *TmpDevicePath;
        HARDDRIVE_DEVICE_PATH*		PartitionNode;

        Status = pBS->HandleProtocol(Handle[i],&gEfiBlockIoProtocolGuid,&BlockIo);
        if (EFI_ERROR(Status))
            continue;

        // if this is not partition, continue
        if (!BlockIo->Media->LogicalPartition)
            continue;

        Status = pBS->HandleProtocol(Handle[i],&gEfiDevicePathProtocolGuid,&PartitionDevicePath);
        if (EFI_ERROR(Status))
            continue;

        // Get last node of the device path. It should be partition node
        PartitionNode = (HARDDRIVE_DEVICE_PATH*)PartitionDevicePath;

        for ( TmpDevicePath = PartitionDevicePath;
                !isEndNode(TmpDevicePath);
                TmpDevicePath=NEXT_NODE(TmpDevicePath) )
        {
            PartitionNode = (HARDDRIVE_DEVICE_PATH*)TmpDevicePath;
        }

        //Check if our partition matches Boot partition
        if (PartitionNode->Header.Type != MEDIA_DEVICE_PATH || PartitionNode->Header.SubType != MEDIA_HARDDRIVE_DP)
            continue;

        if ( PartitionNode->PartitionNumber == BootParitionDevicePath->PartitionNumber &&
                PartitionNode->SignatureType == BootParitionDevicePath->SignatureType &&
                !MemCmp(PartitionNode->Signature,BootParitionDevicePath->Signature,16) )
        {
            //Match found
            TempHandle = Handle[i];
            break;
        }
    }

    pBS->FreePool(Handle);
    return TempHandle;
}

//(EIP126686+)>
EFI_STATUS CompareHddDevicePath( EFI_DEVICE_PATH_PROTOCOL *DevDp1, EFI_DEVICE_PATH_PROTOCOL *DevDp2 )
{

    if ( DevDp1->Type == MEDIA_DEVICE_PATH &&
            DevDp1->SubType == MEDIA_HARDDRIVE_DP )
    {
        if (MemCmp(DevDp1+1, DevDp2+1, sizeof(HARDDRIVE_DEVICE_PATH)-sizeof(EFI_DEVICE_PATH_PROTOCOL)) == 0) //Skip Header EFI_DEVICE_PATH_PROTOCOL.
        {
            DevDp1 = NEXT_NODE(DevDp1);
            if ( DevDp1->Type == MEDIA_DEVICE_PATH &&
                    DevDp1->SubType == MEDIA_FILEPATH_DP ) Wcsupr( (CHAR16*)DevDp1+1 );

            DevDp2 = NEXT_NODE(DevDp2);
            if ( DevDp2->Type == MEDIA_DEVICE_PATH &&
                    DevDp2->SubType == MEDIA_FILEPATH_DP ) Wcsupr( (CHAR16*)DevDp2+1 );

            if (MemCmp(DevDp1, DevDp2, DPLength(DevDp2)) == 0)
                return EFI_SUCCESS;
        }
    }

    return EFI_NOT_FOUND;
}
//<(EIP126686+)

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	CheckBootOptionMatch
//
// Description:	Search each BootOptionList and compare device path.
//
// Input:		EFI_DEVICE_PATH_PROTOCOL *DevicePath
//
// Output:		EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CheckBootOptionMatch (
    IN EFI_DEVICE_PATH_PROTOCOL* HdDevPath
)
{
    DLINK *Link;
    BOOT_OPTION *Option;

    FOR_EACH_BOOT_OPTION(BootOptionList,Link,Option) {

        //(EIP126686+)>
        if ( CompareHddDevicePath(Option->FilePathList, HdDevPath) == EFI_SUCCESS )
            return EFI_SUCCESS;
        //<(EIP126686+)
    }
    return EFI_NOT_FOUND;
}

//(EIP103672+)>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	GetEfiOsBootNameItemCount
//
// Description:	search GPT HDD and return Hard disk handle
//
// Input:		EFI_DEVICE_PATH_PROTOCOL *DevicePath
//
// Output:		EFI_HANDLE
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT16 GetEfiOsBootNameItemCount(void)
{
    UINT16 ItemCount=0;

    do {

        if ( FILE_NAME_MAPS[ItemCount].FilePath == NULL ) break;

        ItemCount++;

    } while (1);

    return ItemCount;
}
//<(EIP103672+)

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	GetGptPartitionHandle
//
// Description:	search GPT HDD and return Hard disk handle
//
// Input:		EFI_DEVICE_PATH_PROTOCOL *DevicePath
//
// Output:		EFI_HANDLE
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CreateEfiOsBootOption(VOID)
{
    EFI_STATUS Status;
    EFI_HANDLE* HandleBuffer = NULL;
    UINTN HandleCount;
    UINTN i;
    UINTN j;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* SimpleFileSystem = NULL;
    EFI_FILE_PROTOCOL *FileProtocol = NULL;
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
    BOOLEAN WinBootManager;
    UINT16 AUTO_BOOT_ENTRY_COUNT;						//(EIP103672+)

    Status = pBS->LocateHandleBuffer (
                 ByProtocol,
                 &gEfiSimpleFileSystemProtocolGuid,
                 NULL,
                 &HandleCount,
                 &HandleBuffer
             );
    if (EFI_ERROR(Status)) {
        TRACE((-1, "LocateHandleBuffer: %r\n", Status));
        return Status;
    }
    AUTO_BOOT_ENTRY_COUNT = GetEfiOsBootNameItemCount();			//(EIP103672+)

    // For each handle found, check if eLink files exist.
    for (i = 0; i < HandleCount; i++) {

        Status = pBS->HandleProtocol (
                     HandleBuffer[i],
                     &gEfiSimpleFileSystemProtocolGuid,
                     &SimpleFileSystem
                 );
        if (EFI_ERROR(Status)) {
            TRACE((-1, "HandleProtocol(SimpleFileSystem): %r\n", Status));
            continue;
        }

        Status=pBS->HandleProtocol( HandleBuffer[i], &gEfiBlockIoProtocolGuid, &BlkIo );
        if ( EFI_ERROR(Status) || BlkIo->Media->RemovableMedia ) continue;	//skip removable device

        Status = SimpleFileSystem->OpenVolume (
                     SimpleFileSystem,
                     &FileProtocol
                 );
        if (EFI_ERROR(Status)) {
            TRACE((-1, "OpenVolume: %r\n", Status));
            continue;
        }

        WinBootManager = FALSE;
        // Loop through all eLink-specified files.
        for (j = 0; j < AUTO_BOOT_ENTRY_COUNT; j++) {
            EFI_FILE_PROTOCOL* NewFileProtocol = NULL;

            //Windows boot manager created, skip Bootx64.efi or Bootia32.efi
            if ( WinBootManager &&
                    !Wcscmp( FILE_NAME_MAPS[j].FilePath, EFI_BOOT_FILE_NAME) ) continue;

            Status = FileProtocol->Open (
                         FileProtocol,
                         &NewFileProtocol,
                         FILE_NAME_MAPS[j].FilePath,
                         EFI_FILE_MODE_READ,
                         0
                     );

            TRACE((-1, "Open(%S): %r\n", FILE_NAME_MAPS[j].FilePath, Status));

            if (EFI_ERROR(Status)) continue;
            //Windows Boot Manager (bootmgfw.efi) exist, record flag.
            if ( !Wcscmp( FILE_NAME_MAPS[j].FilePath, L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi") )
                WinBootManager = TRUE;

            NewFileProtocol->Close(NewFileProtocol);

            {
                UINTN OptionSize;
                UINT8* BytePtr;
                EFI_DEVICE_PATH_PROTOCOL *DevicePath = NULL;
                EFI_DEVICE_PATH_PROTOCOL *HdDevPath = NULL;
                FILEPATH_DEVICE_PATH *FpDevPath = NULL;
                BOOT_OPTION *Option;


                // Find total size of new boot option.
                OptionSize = 	sizeof(HARDDRIVE_DEVICE_PATH) + // Partition node
                              sizeof(FILEPATH_DEVICE_PATH) + // FilePath node
                              ((Wcslen(FILE_NAME_MAPS[j].FilePath) ) * sizeof(CHAR16)) +	//(EIP120976)
                              sizeof(EFI_DEVICE_PATH_PROTOCOL); //+ // End node			//(EIP103870)
//            			sizeof(UINT16); // Signature "0x55AA"					//(EIP103870-)

                Status = pBS->AllocatePool (
                             EfiBootServicesData,
                             OptionSize,
                             &DevicePath);

                if (EFI_ERROR(Status)) {
                    TRACE((-1, "AllocatePool %r\n", Status));
                    return Status;
                }


                // Get HD node of device path associated with handle.
                Status = GetHdNode (
                             HandleBuffer[i],
                             &HdDevPath );

                if (EFI_ERROR(Status)) {
                    TRACE((-1, "GetHdNode: %r\n", Status));
                    continue;
                }

                BytePtr = (UINT8*)DevicePath;
                // Copy to FilePath.
                MemCpy(BytePtr, HdDevPath, NODE_LENGTH(HdDevPath));

                // Point to next node.
                BytePtr += NODE_LENGTH(HdDevPath);
                FpDevPath = (FILEPATH_DEVICE_PATH*)BytePtr;

                // Set Filepath node.
                FpDevPath->Header.Type = MEDIA_DEVICE_PATH;
                FpDevPath->Header.SubType = MEDIA_FILEPATH_DP;
                SET_NODE_LENGTH(&(FpDevPath->Header), 4 + (UINT16)((Wcslen(FILE_NAME_MAPS[j].FilePath) + 1) * sizeof(CHAR16)));

                // Set Filepath PathName.
                MemCpy(FpDevPath->PathName, FILE_NAME_MAPS[j].FilePath, (Wcslen(FILE_NAME_MAPS[j].FilePath) + 1) * sizeof(CHAR16));

                // Point to next node.
                BytePtr += NODE_LENGTH(&(FpDevPath->Header));
                ((EFI_DEVICE_PATH_PROTOCOL*)BytePtr)->Type = END_DEVICE_PATH;
                ((EFI_DEVICE_PATH_PROTOCOL*)BytePtr)->SubType = END_ENTIRE_SUBTYPE;
                SET_NODE_LENGTH((EFI_DEVICE_PATH_PROTOCOL*)BytePtr, END_DEVICE_PATH_LENGTH);

                // Point to signature.
                BytePtr += END_DEVICE_PATH_LENGTH;
//		*((UINT16*)BytePtr)=0x55AA;	// Signature "0x55AA"					//(EIP103870-)

                if ( CheckBootOptionMatch( DevicePath ) == EFI_SUCCESS )
                {
                    TRACE((-1,"CheckBootOptionMatch Matched.....\n" ));
                    continue;
                }

                Option = CreateBootOption(BootOptionList);
                Option->Attributes = 0x01;

                pBS->AllocatePool (
                    EfiBootServicesData,
                    ((Wcslen(FILE_NAME_MAPS[j].BootOptionName) + 1) * sizeof(CHAR16)),
                    &Option->Description);

                MemCpy(Option->Description,
                       FILE_NAME_MAPS[j].BootOptionName,
                       ((Wcslen(FILE_NAME_MAPS[j].BootOptionName) + 1) * sizeof(CHAR16)) );

                Option->FilePathList = DevicePath;
                Option->FilePathListLength = OptionSize;
            }
        }//for (j = 0; j < AUTO_BOOT_ENTRY_COUNT; j++)
    }

    if ( HandleBuffer )
        pBS->FreePool(HandleBuffer);

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	AdjustEfiOsBootOrder
//
// Description:	Os HDD detect and remove boot option.
//
// Input: none
//
// Output: none
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID AdjustEfiOsBootOrder(VOID)
{
    UINT16 NewBootOrder[50];
    UINT16 BootIndex=0;
    UINT16 BootOrder_Flag[50];
    UINT16 *BootOrder = NULL;
    UINTN BootOrderSize = 0;
    EFI_STATUS Status;
    BOOLEAN UpdateBootOrder=FALSE, SkipThisBootOrder=FALSE;
    UINTN i, j;
    EFI_LOAD_OPTION *NvramOption = NULL;
    UINTN NvramOptionSize;

    TRACE((-1,"EfiOsBootOrder.....\n"));

    pBS->SetMem( BootOrder_Flag, sizeof(BootOrder_Flag), 0);

    Status=GetEfiVariable(
               L"BootOrder", &EfiVariableGuid, NULL, &BootOrderSize, &BootOrder);

    if (EFI_ERROR(Status)) return;
    for (i=0; i<BootOrderSize/sizeof(UINT16); i++)
    {
        UINTN DescriptionSize;
        EFI_DEVICE_PATH_PROTOCOL *ScrDevicePath;
        CHAR16 BootStr[9];

        TRACE((-1,"Get Boot Option Boot%04X\n", BootOrder[i]));
        if ( BootOrder_Flag[BootOrder[i]] ) continue;

        // Get Boot Option
        NvramOption = NULL;
        NvramOptionSize =0;
        Swprintf(BootStr,L"Boot%04X",BootOrder[i]);
        Status=GetEfiVariable(
                   BootStr, &EfiVariableGuid, NULL, &NvramOptionSize, &NvramOption
               );
        if (EFI_ERROR(Status)) continue;

        DescriptionSize = (Wcslen((CHAR16*)(NvramOption+1))+1)*sizeof(CHAR16);
        ScrDevicePath =(EFI_DEVICE_PATH_PROTOCOL*)((UINT8*)(NvramOption+1)+DescriptionSize);
        SkipThisBootOrder=FALSE;

        if ( ScrDevicePath->Type == MEDIA_DEVICE_PATH
                && ScrDevicePath->SubType == MEDIA_HARDDRIVE_DP )
        {
            EFI_HANDLE GptHandle;
            TRACE((-1,"EfiOsBootOrder.c :: BootOrder[%x]=%x %S\n", i, BootOrder[i], (CHAR16*)(NvramOption+1) ));
            GptHandle = GetGptPartitionHandle(ScrDevicePath);
            TRACE((-1,"EfiOsBootOrder.c :: GptHandle=%d\n", GptHandle));
            BootOrder_Flag[BootOrder[i]] = 1;	//record bootorder flag
            if ( GptHandle != NULL)
            {
                for (j=0; j<BootOrderSize/sizeof(UINT16); j++)
                {
                    CHAR16 BootStr2[9];
                    UINTN NvramOptionSize2 = 0;
                    EFI_LOAD_OPTION *NvramOption2 = NULL;
                    EFI_DEVICE_PATH_PROTOCOL *DevicePath = NULL;

                    if ( BootOrder_Flag[BootOrder[j]] ) continue;

                    TRACE((-1,"EfiOsBootOrder.c :: Search BootOrder[%x]=%x\n", j, BootOrder[j]));
                    // Get Boot Option
                    NvramOption2 = NULL;
                    NvramOptionSize2 =0;
                    Swprintf(BootStr2,L"Boot%04X",BootOrder[j]);
                    Status=GetEfiVariable(
                               BootStr2, &EfiVariableGuid, NULL, &NvramOptionSize2, &NvramOption2
                           );
                    if (EFI_ERROR(Status)) continue;

                    DescriptionSize = (Wcslen((CHAR16*)(NvramOption2+1))+1)*sizeof(CHAR16);
                    DevicePath =(EFI_DEVICE_PATH_PROTOCOL*)((UINT8*)(NvramOption2+1)+DescriptionSize);

//					if( !MemCmp(ScrDevicePath, DevicePath, DPLength(ScrDevicePath)) )         //(EIP126686-)
                    if ( CompareHddDevicePath(ScrDevicePath, DevicePath) == EFI_SUCCESS )     //(EIP126686+)
                    {
                        TRACE((-1,"EfiOsBootOrder.c :: Matched BootOrder[%x]=%x %S\n", j, BootOrder[j], (CHAR16*)(NvramOption2+1) ));

                        //Change Boot Order Priority
                        {
                            UINT16 temp;
                            if ( NvramOptionSize2 == (DescriptionSize+NvramOption2->FilePathListLength+sizeof(EFI_LOAD_OPTION)) ) //(EIP126866+)
                            {                                                                                                    //(EIP126866+)
                                BootOrder_Flag[BootOrder[j]] = 1;
                                BootOrder_Flag[BootOrder[i]] = 0;

                                temp=BootOrder[j];
                                BootOrder[j]=BootOrder[i];
                                BootOrder[i]=temp;

                                //Clear variable Boot####
                                Status = pRS->SetVariable(
                                             BootStr2, &EfiVariableGuid,
                                             BOOT_VARIABLE_ATTRIBUTES, 0, NULL);
                                SkipThisBootOrder = TRUE;
                                UpdateBootOrder = TRUE;
                            }                                                                                                  //(EIP126866+)>>
                            else
                            {
                                BootOrder_Flag[BootOrder[j]] = 1;
                                Status = pRS->SetVariable(
                                             BootStr, &EfiVariableGuid,
                                             BOOT_VARIABLE_ATTRIBUTES, 0, NULL);

                                NewBootOrder[BootIndex]=BootOrder[j];
                                TRACE((-1,"EfiOsBootOrder.c :: NewBootOrder[%x]=%x %S\n",BootIndex, BootOrder[j], (CHAR16*)(NvramOption+1)));
                                BootIndex++;
                                SkipThisBootOrder = TRUE;
                                UpdateBootOrder = TRUE;
                            }
                        }

                        pBS->FreePool(NvramOption2);
                        break;
                    }
                    else
                        pBS->FreePool(NvramOption2);
                }

            }
            else
                //GPT HDD NOT FOUND, Remove This BootOrder
            {
                if ( ((HARDDRIVE_DEVICE_PATH*)ScrDevicePath)->MBRType == MBR_TYPE_EFI_PARTITION_TABLE_HEADER)
                {
                    //Clear variable Boot####
                    Status = pRS->SetVariable(
                                 BootStr, &EfiVariableGuid,
                                 BOOT_VARIABLE_ATTRIBUTES, 0, NULL);

                    SkipThisBootOrder = TRUE;
                    UpdateBootOrder = TRUE;
                }
            }
        }

        if ( !SkipThisBootOrder )
        {
            NewBootOrder[BootIndex]=BootOrder[i];
            TRACE((-1,"EfiOsBootOrder.c :: NewBootOrder[%x]=%x %S\n",BootIndex, BootOrder[i], (CHAR16*)(NvramOption+1)));
            BootIndex++;
        }

        pBS->FreePool(NvramOption);
    }

    if ( UpdateBootOrder )
    {
        TRACE((-1,"EfiOsBootOrder.c :: Update BootOrder\n" ));

        pRS->SetVariable(
            L"BootOrder", &EfiVariableGuid,
            BOOT_VARIABLE_ATTRIBUTES, BootIndex * sizeof(UINT16), NewBootOrder);
    }
    pBS->FreePool(BootOrder);

}

#if (CSM_SUPPORT == 1) && (RemoveLegacyGptHddDevice == 1)
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   RemoveLegacyGptHdd
//
// Description: Determine if this boot device is a UEFI HDD
//
// Input:       BOOT_DEVICE *Device - the device in question
//
// Output:      BOOLEAN - TRUE - Device is a UEFI HDD Boot Device and it should
//                              be removed legacy device.
//                        FALSE - Device is not a UEFI hdd and it should be left
//                              in the boot order.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN RemoveLegacyGptHdd(BOOT_DEVICE *Device) {
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
    EFI_STATUS Status;
    UINT8 *Buffer = NULL;
    UINTN index;
    PARTITION_ENTRY *pEntries;

    if (   Device->DeviceHandle == INVALID_HANDLE
            || Device->BbsEntry == NULL
       ) return FALSE;

    if ( Device->BbsEntry->DeviceType != BBS_HARDDISK ) return FALSE;

    Status=pBS->HandleProtocol(
               Device->DeviceHandle, &gEfiBlockIoProtocolGuid, &BlkIo
           );

    if (EFI_ERROR(Status) || BlkIo->Media->RemovableMedia) return FALSE;	//USB device?

    Status = pBS->AllocatePool( EfiBootServicesData, BlkIo->Media->BlockSize, &Buffer );
    if ( Buffer == NULL ) return FALSE;

    // read the first sector
    BlkIo->ReadBlocks ( BlkIo,
                        BlkIo->Media->MediaId,
                        0,
                        BlkIo->Media->BlockSize,
                        (VOID*)Buffer);

    if (Buffer[0x1fe] == 0x55 && Buffer[0x1ff] == 0xaa)	//MBR Signature
    {
        pEntries=(PARTITION_ENTRY *)(Buffer+0x1be);

        for (index=0; index<4; index++)
        {
            if ( pEntries[index].PartitionType == 0xee) 	//Check GPT Partition?
            {
                pBS->FreePool( Buffer );
                return TRUE;			//Set Can't Boot.
            }
        } //for(index=0;index<4;index++)
    }//if(Buffer[0x1fe] == 0x55 && Buffer[0x1ff] == 0xaa)

    pBS->FreePool( Buffer );
    return FALSE;
}
#endif
//***********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
