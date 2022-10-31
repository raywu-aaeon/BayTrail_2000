//**********************************************************************
//**********************************************************************
//**                                                                  **
//**         (C)Copyright 1985-2013, American Megatrends, Inc.         **
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
// Revision History
// ----------------
// $Log: $
// 
// 
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    EXTRecovery.c
//
// Description: 
//
//<AMI_FHDR_END>
//**********************************************************************
#include <AmiPeiLib.h>
#include <Token.h>
#include <Ppi/DeviceRecoveryModule.h>
#include <Ppi/BlockIo.h>
#include <Guid/AmiRecoveryDevice.h>
#include <AmiModulePkg\Recovery\FsRecovery\FsRecovery.h>
#include <EXTRecovery.h>



extern UINTN               PartCount;
extern BOOLEAN             IsMbr;
extern UINT32              GpeCount;
extern UINT32              PartSector;
extern MASTER_BOOT_RECORD  Mbr;
extern UINT8               *ReadBuffer;
extern UINT8               *RootBuffer;
extern UINTN               RootBufferSize ;
extern UINT32              RootEntries ;
extern UINT32              RootSize;


DIR_ENTRY_EXT       *ExtRecoveryFiles[10];
UINT32              InodeBlock;     // First block of inode table
UINT8               Indirect;
UINT32              *BlockList;
UINT32              *BlockList2;





//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsExt
//
// Description:
//  Checks if given data block describes EXT Superblock structure
//
// Input:
//  VOLUME_SB *pSb - pointer to data block to check
//
// Output:
//  TRUE - data block is a EXT Superblock structure
//  FALSE - data block is not a EXT Superblock structure
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsExt(
    IN VOLUME_SB *pSb )
{
    return pSb->SB_Magic == 0xEF53
           && pSb->SB_BlockSize < 4
           && pSb->SB_FirstBlock < 2
           && pSb->SB_FreeBlocks < pSb->SB_TotalBlocks
           && pSb->SB_FreeInodes < pSb->SB_TotalInodes;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetNextBlock
//
// Description: (EXT) Retrieves the next block number from an Inode
//              block list.
//
// Parameters:  VOLUME_INFO *Vi - Volume Info Structure
//              VOLUME_SB   *Sb - Superblock structure
//              VOLUME_IT   *Inode - Inode table structure
//              UINT32  *BlockNo - Sequential number of the block
//              UINT32  *Block - Next block of the file
//              BOOLEAN UpdateList - Update block no. to next block if TRUE.
//
// Return value: EFI_STATUS Status (EFI_SUCCESS or EFI_END_OF_FILE)
//
// Modified:
//
// Referral(s):
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetNextBlock(
    EFI_PEI_SERVICES **PeiServices,
    RC_VOL_INFO      *Volume,
    VOLUME_SB        *Sb,
    VOLUME_IT        *Inode,
    UINT32           *BlockNo,
    UINT32           *Block,
    BOOLEAN          UpdateList )
{
    UINT32      *BlockListPtr;
    UINT32      TmpBlock;
    UINT32      TmpBlock2;
    UINT32      TmpBlkNo;
    UINT32      IBlkCnt;
    UINT64      Offset;
    UINT32      BlockSize;
    UINT32      NosPerBlock;
    UINTN       BlockPages;
    EFI_STATUS  Status;
    EFI_PHYSICAL_ADDRESS Allocate;

    BlockSize = 1024 << Sb->SB_BlockSize;
    NosPerBlock = BlockSize / 4;
    TmpBlkNo = *BlockNo;

    // Process direct blocks (0-11)
    if ((TmpBlkNo < 12) && (Indirect == 0))
    {
        BlockListPtr = &Inode->Alloc.Ext2.Blocks[0];
        *Block = BlockListPtr[TmpBlkNo];
    }

    // Process single indirect blocks (12-(256+11))
    if ((TmpBlkNo >= 12) && (TmpBlkNo < NosPerBlock+12) && (Indirect != 1))
    {
        Indirect = 1;
        TmpBlock = Inode->Alloc.Ext2.Blocks[12];
        Offset = Mul64(TmpBlock, BlockSize);
        ReadDevice (PeiServices, Volume, Offset, BlockSize, &BlockList[0]);
    }

    if ((TmpBlkNo >= 12) && (TmpBlkNo < NosPerBlock+12) && (Indirect == 1))
    {
        BlockPages = EFI_SIZE_TO_PAGES( BlockSize );
        Status = (*PeiServices)->AllocatePages( PeiServices, EfiBootServicesData, BlockPages, &Allocate );
        if ( EFI_ERROR( Status )) {
            return EFI_END_OF_FILE;
        }
        BlockList = (UINT32*)((UINTN)Allocate);

        BlockListPtr = &BlockList[0];
        TmpBlock = TmpBlkNo - 12;
        *Block = BlockListPtr[TmpBlock];
    }

    // Process double indirect blocks ((256+12)-(65536+256+11))
    if ((TmpBlkNo >= (NosPerBlock+12)) && (Indirect != 2))
    {
        Indirect = 2;
        BlockPages = EFI_SIZE_TO_PAGES( BlockSize );
        Status = (*PeiServices)->AllocatePages( PeiServices, EfiBootServicesData, BlockPages, &Allocate );
        if ( EFI_ERROR( Status )) {
            return EFI_END_OF_FILE;
        }
        BlockList2 = (UINT32*)((UINTN)Allocate);

        TmpBlock = Inode->Alloc.Ext2.Blocks[13];
        Offset = Mul64(TmpBlock, BlockSize);
        ReadDevice (PeiServices, Volume, Offset, BlockSize, &BlockList[0]);
    }

    if ((TmpBlkNo >= (NosPerBlock+12)) && (Indirect == 2))
    {
        TmpBlock = TmpBlkNo - 12;
        IBlkCnt = TmpBlock / NosPerBlock;
        if (TmpBlock % NosPerBlock == 0)
        {
            // Read another set of nos. into BlockList2
            TmpBlock2 = BlockList[IBlkCnt];
            Offset = Mul64(TmpBlock2, BlockSize);
            ReadDevice (PeiServices, Volume, Offset, BlockSize, &BlockList2[0]);
            BlockListPtr = &BlockList2[0];
        }
        TmpBlock -= (NosPerBlock * IBlkCnt);
        *Block = BlockListPtr[TmpBlock];
    }

    if (UpdateList)
    {
        TmpBlkNo++;
        *BlockNo = TmpBlkNo;
    }
    if (*Block == 0)
    {
        return EFI_END_OF_FILE;
    } else {
        return EFI_SUCCESS;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReadExtFile
//
// Description:
//  Reads a file from a device formatted in Ext(n).
//
// Input:
// Parameters:  VOLUME_INFO *Vi - Volume Info Structure
//              VOLUME_SB   *Sb - Superblock structure
//              VOLUME_IT   *Inode - Inode table structure
//              VIOD        *Buffer - Buffer to read into
//              UINT64      *Size - Size of file to read
//
// Output:
//  EFI_STATUS - possible return values
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ReadExtFile(
    IN EFI_PEI_SERVICES **PeiServices,
    RC_VOL_INFO         *Volume,
    VOLUME_SB           *Sb,
    VOLUME_IT           *Inode,
    VOID                *Buffer,
    UINT64              *Size )
{
    EFI_STATUS  Status;
    UINT32      BlockSize;
    UINT32      Block;
    UINT32      BlockNo;
    UINT64      Offset;
    UINT64      TotalBytesRead = 0;
    UINT16      ExtentCount;
    UINT64      BigBlock;
    UINT32      ReadSize;
    UINT16      i;

    BlockSize = 1024 << Sb->SB_BlockSize;
    BlockNo = 0;

    //
    // Check for which allocation method to use for reading the file
    //
    if ((Inode->Alloc.Ext4.Header.EH_Magic == 0xF30A) && \
        (Inode->Alloc.Ext4.Header.EH_Max == 4))
    {
    //
    // Use the EXT4 allocation method
    //
        ExtentCount = Inode->Alloc.Ext4.Header.EH_Extents;

        for (i=0; i<ExtentCount; i++)
        {
            BigBlock = Inode->Alloc.Ext4.Extent[i].EE_BlockLo + \
                       Shl64(Inode->Alloc.Ext4.Extent[i].EE_BlockHi, 0x20);
            Offset = Mul64(BigBlock, BlockSize);
            ReadSize = BlockSize * Inode->Alloc.Ext4.Extent[i].EE_Length;
            if (*Size <= ReadSize)
            {
                Status = ReadDevice (PeiServices, Volume, Offset, (UINTN)*Size, Buffer);
                if (EFI_ERROR(Status)) {
                    *Size = TotalBytesRead;
                    return Status;
                } else {
                    TotalBytesRead += *Size;
                    *Size = TotalBytesRead;
                    return EFI_SUCCESS;
                }
            }

            Status = ReadDevice (PeiServices, Volume, Offset, ReadSize, Buffer);
            if (EFI_ERROR(Status)) return Status;
            *Size -= ReadSize;
            TotalBytesRead += ReadSize;
        } // End of read loop

        return EFI_VOLUME_CORRUPTED; // Shouldn't get here

    } else {
    //
    // Use the EXT2, EXT3 allocation method
    //
        Status = GetNextBlock ( PeiServices,
                                Volume,
                                Sb,
                                Inode,
                                &BlockNo,
                                &Block,
                                TRUE );
        if (EFI_ERROR(Status)) // Zero-length file
        {
            *Size = 0;
            return Status;
        }

        do
        {
            Offset = Mul64 (BlockSize, Block);

            if (*Size <= BlockSize)
            {
                Status = ReadDevice (PeiServices, Volume, Offset, (UINTN)*Size, Buffer);
                if (EFI_ERROR(Status)) {
                    *Size = TotalBytesRead;
                    return Status;
                } else {
                    TotalBytesRead += *Size;
                    *Size = TotalBytesRead;
                    return EFI_SUCCESS;
                }
            }

            Status = ReadDevice (PeiServices, Volume, Offset, BlockSize, Buffer);
            if (EFI_ERROR(Status)) return Status;
            *Size -= BlockSize;
            TotalBytesRead += BlockSize;

            Status = GetNextBlock ( PeiServices,
                                    Volume,
                                    Sb,
                                    Inode,
                                    &BlockNo,
                                    &Block,
                                    TRUE );
            if (EFI_ERROR(Status)) // EOF found
            {
                *Size = TotalBytesRead;
                return EFI_SUCCESS;
            }
        }
        while (*Size);

        return EFI_VOLUME_CORRUPTED; // Shouldn't get here
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReadExtRoot
//
// Description:
//  Prepares given volume for read operations. Reads Ext(n) root directory.
//
// Input:
//  IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//  IN  OUT RC_VOL_INFO *Volume - pointer to volume description structure
//  IN  VOLUME_SB *Sb - pointer to Superblock
//
// Output:
//  EFI_STATUS - possible return values
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ReadExtRoot(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT RC_VOL_INFO  *Volume,
    IN VOLUME_SB        *Sb )
{
    EFI_STATUS          Status;
    UINT32              BlockSize;
    UINT16              InodeSize;
    UINT32              BgdtBlock;
    VOLUME_BGDT         Bgdt;
    VOLUME_IT           RootInode;
    UINT64              Offset;
    UINT64              TmpRootSize;
    UINTN               RootPages;
    EFI_PHYSICAL_ADDRESS Allocate;

    BlockSize = 1024 << Sb->SB_BlockSize;
    if (BlockSize == 1024)
    {
        BgdtBlock = 2;
    } else {
        BgdtBlock = 1;
    }

    // Read in the Block Group Descriptor Table
    Offset = Mul64(BlockSize, BgdtBlock);
    Status = ReadDevice (PeiServices,
                         Volume,
                         Offset,
                         sizeof(VOLUME_BGDT),
                         &Bgdt);
    if (EFI_ERROR(Status))
    {
        return EFI_NOT_FOUND;
    }

    InodeBlock = Bgdt.BGDT_InodeTableBlk;
    InodeSize = Sb->SB_InodeStrucSize;

    // The root directory's inode is always the 2nd inode in the
    // inode table. Read in that inode.
    Offset = Mul64(BlockSize, InodeBlock) + InodeSize;
    Status = ReadDevice (PeiServices,
                         Volume,
                         Offset,
                         sizeof(VOLUME_IT),
                         &RootInode);
    if (EFI_ERROR(Status))
    {
        return EFI_NOT_FOUND;
    }

    TmpRootSize = RootInode.IT_SizeLo + Shl64(RootInode.IT_SizeHi, 0x20);

    RootPages = EFI_SIZE_TO_PAGES( (UINTN)TmpRootSize );
    Status = (*PeiServices)->AllocatePages( PeiServices, EfiBootServicesData, RootPages, &Allocate );
    if ( EFI_ERROR( Status )) {
        return Status;
    }
    RootBuffer     = (UINT8*)((UINTN)Allocate);
    RootBufferSize = EFI_PAGES_TO_SIZE( RootPages );
    MemSet( RootBuffer, RootBufferSize, 0 );

    Status = ReadExtFile( PeiServices,
                          Volume,
                          Sb,
                          &RootInode,
                          RootBuffer,
                          &TmpRootSize );
    RootSize = (UINT32)TmpRootSize;

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ProcessExtVolume
//
// Description:
//  Reads recovery capsule from Ext(n) volume
//
// Input:
//  IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//  IN  OUT RC_VOL_INFO *Volume - pointer to volume description structure
//  IN  CHAR8 *FileName - recovery capsule file name
//  IN  UINTN *FileSize - pointer to size of provided buffer
//  OUT VOID *Buffer - pointer to buffer to store data
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ProcessExtVolume(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT RC_VOL_INFO  *Volume,
    IN OUT UINTN        *FileSize,
    OUT VOID            *Buffer )
{
    EFI_STATUS          Status;
    VOLUME_SB           Sb;
    VOLUME_IT           FileInode;
    UINT32              i;
    UINT32              Inode;
    UINT64              TmpFileSize;
    UINTN               NumberOfFiles;
    UINT64              Offset;
    UINT32              BlockSize;
    UINT16              InodeSize;
    UINT8               *TmpPtr;

    Status = ReadDevice( PeiServices, Volume, 0, 512, &Sb );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    // On an EXT(n) volume, the first sector will be all zeros
    TmpPtr = (UINT8*)&Sb.SB_TotalInodes;
    for (i=0; i<512; i++)
    {
        if ((UINT8)TmpPtr[i] != 0)
        {
            return EFI_NOT_FOUND; // Not an EXT(n) volume
        }
    }

    // The Superblock is always 1024 bytes in on the volume
    Status = ReadDevice( PeiServices, Volume, 1024, 512, &Sb );

    if (!IsExt( &Sb )) {
        return EFI_NOT_FOUND;
    }

    Status = ReadExtRoot( PeiServices, Volume, &Sb );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    AmiGetFileListFromExtVolume(PeiServices, (UINT8*)RootBuffer, RootSize, &NumberOfFiles, ExtRecoveryFiles);

    if ( NumberOfFiles == 0 )
        return EFI_NOT_FOUND;

    BlockSize = 1024 << Sb.SB_BlockSize;
    InodeSize = Sb.SB_InodeStrucSize;
    for(i = 0; i < NumberOfFiles; i++) {
        // An EXT(n) directory  entry only contains the name and inode, so we have to
        // read the inode to get the size.
        Inode = ExtRecoveryFiles[i]->DIR_Inode;
        Offset = Mul64(BlockSize, InodeBlock) + \
                 Mul64((Inode-1), InodeSize);
        Status = ReadDevice (PeiServices,
                             Volume,
                             Offset,
                             sizeof(VOLUME_IT),
                             &FileInode);
        if (EFI_ERROR(Status)) continue;

        TmpFileSize = FileInode.IT_SizeLo + Shl64(FileInode.IT_SizeHi, 0x20);
        if ( *FileSize < (UINTN)TmpFileSize )
            continue;
        Status = ReadExtFile( PeiServices,
                              Volume,
                              &Sb,
                              &FileInode,
                              Buffer,
                              &TmpFileSize );
        if ( EFI_ERROR( Status )) {
            return Status;
        }

        if(AmiIsValidFile(Buffer, (UINTN)TmpFileSize)) {
            *FileSize = (UINTN)TmpFileSize;
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ProcessExtDevice
//
// Description:
//  Reads recovery capsule from Ext(n) device. Tries to discover primary partitions
//  and search for capsule there.
//
// Input:
//  IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//  IN  OUT RC_VOL_INFO *Volume - pointer to volume description structure
//  IN  CHAR8 *FileName - recovery capsule file name
//  IN  UINTN *FileSize - pointer to size of provided buffer
//  OUT VOID *Buffer - pointer to buffer to store data
//
// Output:
//  EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ProcessExtDevice(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT RC_VOL_INFO  *Volume,
    IN OUT UINTN        *FileSize,
    OUT VOID            *Buffer )
{
    EFI_STATUS          Status;
    EFI_STATUS          Status2;

    Volume->PartitionOffset = 0;
    //
    // Assume the volume is floppy-formatted.
    //
    Status = ProcessExtVolume( PeiServices, Volume, FileSize, Buffer );

    if ( !EFI_ERROR( Status )) {
        return Status;
    }

    //
    // Not floppy formatted, look for partitions. Read sector 0 (MBR).
    //
    Status = ReadDevice( PeiServices, Volume, 0, 512, &Mbr );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    if ( Mbr.Sig != 0xaa55 ) {
        return EFI_NOT_FOUND;
    }

    PartCount = 0;
    PartSector = 0;
    IsMbr = TRUE;

    //
    // Locate all partitions. Check each one for the recovery file.
    // The recovery file will be loaded if it is found, and this
    // function will return EFI_SUCCESS.
    //
    do {
        Status = FindNextPartition( PeiServices, Volume );
        if ( !EFI_ERROR(Status) ) {
            Status2 = ProcessExtVolume( PeiServices, Volume, FileSize, Buffer );
            if ( !EFI_ERROR(Status2) ) {
                return Status2;
            }
        }
    } while (Status == EFI_SUCCESS);

    return EFI_NOT_FOUND;
}



//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   AmiGetFileListFromExtVolume
//
// Description:
//  Gets a list of valid recovery files from an EXT(n) volume.
//  As currently written, gets only one file.
//
// Input:
//  UINT8 *Root - Pointer to a buffer containing the root directory
//  UINT32 RootSize - Size of the root directory
//  UINTN *NumberOfFiles - Pointer to number of files found
//  DIR_ENTRY_EXT **Buffer - Pointer to buffer containing index entry of
//    the file that was found.
//
// Output:
//  None - returned in variables.
//
// Notes:
//  This is an e-linked function, which can be replaced.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID AmiGetFileListFromExtVolume(
    IN EFI_PEI_SERVICES **PeiServices,
    IN  UINT8               *Root,
    IN  UINT32              RootSize,
    OUT UINTN               *NumberOfFiles,
    OUT DIR_ENTRY_EXT       **Buffer
)
{

    EFI_STATUS              Status;
    DIR_ENTRY_EXT           *TmpPtr;
    UINT16                  EntryLength;
    UINT8                   NameLength;
    VOID                    *FileName;
    UINTN                   FileSize;
    UINT8                   i;
    CHAR8                   TmpFileName[13];

    *NumberOfFiles = 0;     //no files found yet

    Status = GetRecoveryFileInfo(PeiServices, &FileName, &FileSize, NULL);
    if(EFI_ERROR(Status))
        return;

    do { // do loop handling entries in the root

        TmpPtr = (DIR_ENTRY_EXT*)&Root[0];
        EntryLength = TmpPtr->DIR_EntryLength;
        if (EntryLength == 0) break; // End of directory, file not found
        NameLength = TmpPtr->DIR_NameLength;
        if (NameLength > 12) NameLength = 12;
        for ( i=0; i<NameLength; i++ )
        {
            TmpFileName[i] = TmpPtr->DIR_Name[i];
        }
        TmpFileName[i] = 0; // Zero-terminate name

        if(!EFI_ERROR(FileSearch((CHAR8*)FileName,
                                 TmpFileName,
                                 FALSE,
                                 NameLength))) {
            Buffer[*NumberOfFiles] = TmpPtr; // Save pointer to this entry
            *NumberOfFiles = 1;
            return;
        }

        Root += EntryLength;
        RootSize -= EntryLength;

    } while (RootSize);

}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**         (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
