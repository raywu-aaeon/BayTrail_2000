//**********************************************************************
//**********************************************************************
//**                                                                  **
//**         (C)Copyright 1985-2013, American Megatrends, Inc.        **
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
// Name:    NTFSRecovery.c
//
// Description:
//
//<AMI_FHDR_END>
//**********************************************************************
#include <NTFSRecovery.h>
#include <AmiModulePkg\Recovery\FsRecovery\FsRecovery.h>
#include <AmiPeiLib.h>
#include <Token.h>
#include <Ppi/DeviceRecoveryModule.h>
#include <Ppi/BlockIo.h>

#include <Guid/AmiRecoveryDevice.h>

extern BOOLEAN NtfsRecoverySupport;

INDEX_ENTRY         *NtfsRecoveryFiles[10];
UINT8               MFTRunList[256];
UINT8               RootRunList[128];
UINT8               ResidentIndex[256];
BOOLEAN             ResidentPresent;
UINT32              ResidentSize;
extern UINTN               PartCount;
extern BOOLEAN             IsMbr;
extern UINT32              GpeCount;
extern UINT32              PartSector;
extern MASTER_BOOT_RECORD  Mbr;
extern UINT8               *ReadBuffer;
extern UINTN               BufferSize0;
extern UINT8               *FatBuffer;
extern UINTN               FatBufferSize;
extern UINT8               *RootBuffer;
extern UINTN               RootBufferSize ;
extern UINT32              RootEntries ;
extern UINT32              RootSize;

//EFI_PEI_SERVICES    **ThisPeiServices;



//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsNtfs
//
// Description:
//  Checks if given data block describes NTFS structure
//
// Input:
//  BOOT_SECTOR *pBpb - pointer to data block to check
//
// Output:
//  TRUE - data block is a NTFS structure
//  FALSE - data block is not a NTFS structure
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsNtfs(
    IN BOOT_SECTOR *pBpb )
{
    return pBpb->NumFATs == 0
           && pBpb->TotSec16 == 0
           && pBpb->TotSec32 == 0
           && pBpb->Fat.Ntfs.TotSec64 != 0
           && pBpb->OEMName[0] == 0x4E // Name must be "NTFS"
           && pBpb->OEMName[1] == 0x54
           && pBpb->OEMName[2] == 0x46
           && pBpb->OEMName[3] == 0x53
           && pBpb->Signature == 0xAA55
           && (pBpb->jmp[0] == 0xEB || pBpb->jmp[0] == 0xE9);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetRunListElementData
//
// Description: (NTFS) Retrieves the count and cluster of a run from a run list element
//
// Parameters:  UINT8   **pRunList - Pointer to the run list, updated if
//                                   UpdateList is TRUE.
//              UINT64  *ClusterCount - Length of this run in clusters.
//              UINT64  *Cluster - Starting cluster of this run.
//              BOOLEAN UpdateList - Update list pointer to next element if TRUE.
//
// Return value: EFI_STATUS Status (EFI_SUCCESS or EFI_END_OF_FILE)
//
// Modified:
//
// Referral(s):
//
// NOTE(S):     A run list element has 3 parts -- a size byte, a Cluster
//              count, and a Cluster Number.
//              The low nibble of the size byte is the size of the Count
//              in bytes. The high nibble is the size of the Offset in
//              bytes. The element is therefore 1 + (low nibble) + (high
//              nibble) bytes long.
//              The cluster number is a signed number. The new cluster is
//              added to the old one to get the result. So if the new
//              cluster lies before the old one on the disk, it will be
//              a negative number.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
GetRunListElementData (
    UINT8   **pRunList,
    UINT64  *ClusterCount,
    UINT64  *Cluster,
    BOOLEAN UpdateList
    )
{
    UINT64 TempCount = 0;
    UINT64 TempCluster = 0;
    UINT64 LeftFill = 0;
    UINT8  LowNibble;
    UINT8  HighNibble;
    UINT8  i, HighByte;
    UINT8  *RunListPtr;

//
// If the size byte is 0, we have reached the end of the file.
//
    RunListPtr = *pRunList;
    if (RunListPtr[0] == 0)
    {
        return EFI_END_OF_FILE;
    }

    LowNibble = RunListPtr[0] & 0xF;
    HighNibble = RunListPtr[0] >> 4;
    RunListPtr++;
//
// Get run length.
//
    for (i=LowNibble; i>0; i--)
    {
        TempCount = Shl64(TempCount, 8);
        TempCount += RunListPtr[i-1];
    }
    RunListPtr += LowNibble;
//
// Get the run offset.
//
    HighByte = RunListPtr[HighNibble-1];
    for (i=HighNibble; i>0; i--)
    {
        TempCluster = Shl64(TempCluster, 8);
        TempCluster += RunListPtr[i-1];
    }
    RunListPtr += HighNibble;
//
// If the offset is negative, left-fill the empty bytes with 0xFF.
//
    if ((HighByte & 0x80) && (HighNibble < 8))
    {
        for (i=8; i>HighNibble; i--)
        {
            LeftFill = Shr64(LeftFill, 8);
            LeftFill |= 0xFF00000000000000;
        }
        TempCluster |= LeftFill;
    }

    *Cluster += TempCluster;
    *ClusterCount = TempCount;
    if (UpdateList) *pRunList = RunListPtr;
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetFrAttribute
//
// Description: (NTFS) Retrieves a File Record Attribute by it's number from a
//              File Record.
//
// Parameters:  UINT8   *BufferIn - Pointer to a buffer containing a file record
//              UINT8   AttributeNumber - Number of the attribute to retrieve
//              UINTN   **BufferOut - Points to the attribute in the buffer
//
// Return value: EFI_STATUS Status (EFI_SUCCESS or EFI_NOT_FOUND)
//
// Modified:
//
// Referral(s):
//
// Note(s):     Attributes are in sequential order, so, for example,
//              if we're looking for 30, and we find 10 and then 40,
//              we know there is no 30 in the record.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
GetFrAttribute (
    UINT8 *BufferIn,
    UINT8 AttributeNumber,
    UINT8 **BufferOut
    )
{
    UINT8   *TempBuffer;

    TempBuffer = BufferIn;

//
// Point to 1st attribute.
//
    TempBuffer += ((MFT_FILE_RECORD*)TempBuffer)->FR_AttributeOffset;
//
// Search for the attribute.
//
    while (TempBuffer[0] != AttributeNumber)
    {
        if (TempBuffer[0] > AttributeNumber) return EFI_NOT_FOUND;
        if (TempBuffer[0] == 0xFF) return EFI_NOT_FOUND;
        if ( ((FR_ATTR_HEADER_RES*)TempBuffer)->AHR_Length == 0 )
            return EFI_NOT_FOUND;
        TempBuffer += ((FR_ATTR_HEADER_RES*)TempBuffer)->AHR_Length;
    }

    *BufferOut = TempBuffer;
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetFileRecord
//
// Description: (NTFS) Returns the file record specified by MFTRecordNo in a buffer.
//
// Parameters:  VOLUME_INFO *Vi - Volume Info Structure
//              BOOT_SECTOR *Bs - Boot sector structure
//              UINT64 MFTRecordNo - MFT Record number to get
//              UINT8 *Buffer - Buffer to read record into
//              UINT64 *MFTSector - Sector where record found
//
// Return value: EFI_STATUS Status (EFI_SUCCESS or EFI_NOT_FOUND)
//
// Modified:
//
// Referral(s):
//
// NOTE(S):     The File Records in the Master File Table are numbered
//              sequentially. We just have to count our way through the
//              MFT's run list until we find it.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
GetFileRecord (
    EFI_PEI_SERVICES **PeiServices,
    RC_VOL_INFO      *Volume,
    BOOT_SECTOR      *Bs,
    UINT64           MFTRecordNo,
    UINT8            *FrBuffer,
    UINT64           *MFTSector OPTIONAL
    )
{
    EFI_STATUS Status;
    UINT8      *pRunList;
    UINT64     Cluster;
    UINT64     Sector;
    UINT64     Count;
    UINT64     ByteCount;
    UINT32     SecPerRecord = 1;
    UINT64     RecordCount;
    UINT32     BytesPerCluster;
    UINT64     Offset;
    UINT32     RecordSize;

    Cluster = 0;
    pRunList = &MFTRunList[0];

    MFTRecordNo &= MAXIMUM_RECORD_NUMBER; // Isolate number part

    Status = GetRunListElementData(&pRunList, &Count, &Cluster, TRUE);
    BytesPerCluster = Bs->BytsPerSec * Bs->SecPerClus;
    ByteCount = Mul64(Count, BytesPerCluster);
    if ( Bs->BytsPerSec <= FILE_RECORD_SIZE ) {
        SecPerRecord = FILE_RECORD_SIZE / Bs->BytsPerSec;
        RecordSize = FILE_RECORD_SIZE;
    } else { // Special case for 4k sectors
        SecPerRecord = 1;
        RecordSize = (UINT32)Bs->BytsPerSec;
    }
//###DEBUG CHANGE NEEDED LATER ////
// In NTFS, the cluster size can be 512 bytes to 4096 bytes.
// File records are 1024 bytes
// For now, we're going to assume a cluster size of 1024 bytes or more.
////////////////////////////

    Sector = Mul64(Cluster, Bs->SecPerClus);
    RecordCount = 0;
    do {
        if (ByteCount > 0)
        {
            Sector += SecPerRecord;
            ByteCount -= RecordSize;
        } else { // We've used up a run, read from the next one.
            Status = GetRunListElementData(&pRunList, &Count, &Cluster, TRUE);
            if (EFI_ERROR(Status)) return EFI_NOT_FOUND;
            ByteCount = Mul64(Count, BytesPerCluster);
            Sector = Mul64(Cluster, Bs->SecPerClus);
            continue;
        }
        RecordCount++;
    } while (RecordCount < MFTRecordNo); // Record numbers are 0-based.
//
// We found the sector of the file record wanted. Now read it.
//
    Offset = Mul64( Sector, Bs->BytsPerSec );
    Status = ReadDevice( PeiServices, Volume, Offset, RecordSize, FrBuffer );
    if (EFI_ERROR(Status)) return EFI_NOT_FOUND;
//
// A File recored begins with "FILE". Check it.
//
    if ( (FrBuffer[0] != 0x46) || \
         (FrBuffer[1] != 0x49) || \
         (FrBuffer[2] != 0x4C) || \
         (FrBuffer[3] != 0x45) ) return EFI_NOT_FOUND;

    *MFTSector = Sector; // Return sector where the record was found
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReadNTFSFile
//
// Description:
//  Reads a file from a device formatted in NTFS.
//
// Input:
// Parameters:  VOLUME_INFO *Vi - Volume Info Structure
//              BOOT_SECTOR *Bs - Boot sector structure
//              UINT8       *RunList - Run List of file to read
//              VIOD        *Buffer - Buffer to read into
//              UINT64      *Size - Size of file to read
//
// Output:
//  EFI_STATUS - possible return values
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ReadNTFSFile(
    IN EFI_PEI_SERVICES **PeiServices,
    RC_VOL_INFO         *Volume,
    BOOT_SECTOR         *Bs,
    UINT8               *RunList,
    VOID                *Buffer,
    UINT64              *Size )
{
    UINT64      TotalContiguousBytes;
    UINT64      TotalBytesRead = 0;
    UINT64      AbsByte;
    UINT64      AccessBytes;
    UINT64      ClusterCount;
    UINT64      Cluster = 0;
    EFI_STATUS  Status;

    Status = GetRunListElementData(&RunList, &ClusterCount,
                                       &Cluster, TRUE);
    do {
        TotalContiguousBytes = Mul64(ClusterCount,
                                     Bs->SecPerClus);
        TotalContiguousBytes = Mul64(TotalContiguousBytes,
                                     Bs->BytsPerSec);
        if ( TotalContiguousBytes > *Size) AccessBytes = *Size;

        else AccessBytes = TotalContiguousBytes;

        AbsByte = Mul64( Cluster, Bs->SecPerClus );
        AbsByte = Mul64( AbsByte, Bs->BytsPerSec );

        if (AccessBytes == 0) {
            return EFI_VOLUME_CORRUPTED; // Will happen if early EOF.
        }

        Status = ReadDevice( PeiServices,
                             Volume,
                             AbsByte,
                             (UINTN)AccessBytes,
                             Buffer );

        if (EFI_ERROR(Status)) {
            break;
        }

        (UINT8 *)Buffer += AccessBytes;
        TotalBytesRead +=AccessBytes;

        *Size   -= AccessBytes;

        if (AccessBytes == TotalContiguousBytes)
        {
            Status = GetRunListElementData (&RunList, &ClusterCount,
                                            &Cluster, TRUE);
            if (EFI_ERROR(Status)) break; // Error here means EOF.
        }

    } while (*Size);

    *Size = (UINT32)TotalBytesRead;
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReadNTFSRoot
//
// Description:
//  Prepares given volume for read operations. Reads NTFS root directory.
//
// Input:
//  IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//  IN  OUT RC_VOL_INFO *Volume - pointer to volume description structure
//  IN  BOOT_SECTOR *Bs - pointer to MBR
//
// Output:
//  EFI_STATUS - possible return values
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ReadNTFSRoot(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT RC_VOL_INFO  *Volume,
    IN BOOT_SECTOR      *Bs )
{
    UINT8       FrBuffer[FILE_RECORD_SIZE]; // This needs to be 4096 for 4k secs.
    UINT8       *Buffer2;
    UINT8       *Buffer3;
    UINT8       *pRunList;
    EFI_STATUS  Status;
    UINT64      TotalSectors;
    UINT64      DataSectors;
    UINT64      Temp64;
    UINT32      Temp32;
    UINT16      Temp16;
    UINT32      IndexSize;
    UINT64      Cluster = 0;
    UINT64      ClusterCount;
    UINT64      TmpRootSize;
    UINTN       RootPages;
    EFI_PHYSICAL_ADDRESS Allocate;

    TotalSectors = Bs->Fat.Ntfs.TotSec64;
    DataSectors  = TotalSectors - Bs->RsvdSecCnt;

    Temp64 = Mul64( Bs->Fat.Ntfs.MFTClus, Bs->SecPerClus );
    Volume->FatOffset = Mul64( Temp64, Bs->BytsPerSec ); // For NTFS, FatOffset is MFT Offset

    //
    // Read the first file record of the MFT, to get the MFT run list.
    //
    Status = ReadDevice( PeiServices, Volume, Volume->FatOffset, FILE_RECORD_SIZE, FrBuffer );
    if ( EFI_ERROR( Status )) {
        return Status;
    }
    Buffer2 = &FrBuffer[0];
    Status = GetFrAttribute( Buffer2, FR_ATTRIBUTE_DATA, &Buffer2 ); // Get data attribute
    if ( EFI_ERROR( Status )) {
        return Status;
    }
    Buffer2 += ((FR_ATTR_HEADER_NONRES*)Buffer2)->AHNR_RunOffset; // Point to run list
    MemCpy( MFTRunList, Buffer2, 256 ); // Copy MFT run list
    //
    // Get the root directory file record, to get its run list.
    //
    Buffer2 = &FrBuffer[0];
    Status = GetFileRecord( PeiServices, Volume, Bs, 5, Buffer2, NULL ); // Root is always record no. 5
    if ( EFI_ERROR( Status )) {
        return Status;
    }
    //
    // Check for a resident index. It will be in the Index Root Attribute.
    // If one if found, it will be saved for searching later.
    //
    ResidentPresent = FALSE;
    Buffer3 = Buffer2;
    Status = GetFrAttribute( Buffer2, FR_ATTRIBUTE_INDEX_ROOT, &Buffer3 );
    if ( Status == EFI_SUCCESS) { // Root Attribute found
        Temp16 = ((FR_ATTR_HEADER_RES*)Buffer3)->AHR_InfoOffset;
        Buffer3 += Temp16;
        IndexSize = ((FR_INDEX_ROOT_ATTRIBUTE*)Buffer3)->IRA_TotalSize;
        Temp32 = ((FR_INDEX_ROOT_ATTRIBUTE*)Buffer3)->IRA_Offset;
        Buffer3 += Temp32 + EFI_FIELD_OFFSET(FR_INDEX_ROOT_ATTRIBUTE, IRA_Offset);
        if (IndexSize >= MINIMUM_ENTRY_SIZE) { // Resident index is not empty
            MemCpy ( ResidentIndex, Buffer3, IndexSize );
            ResidentPresent = TRUE;
            ResidentSize = IndexSize;
        }
    }
    //
    // Now, check for a non-resident index.
    //
    Status = GetFrAttribute( Buffer2, FR_ATTRIBUTE_INDEX_ALLOC, &Buffer2 );
    if ( EFI_ERROR( Status )) {
        return Status;
    }
    Buffer2 += ((FR_ATTR_HEADER_NONRES*)Buffer2)->AHNR_RunOffset; // Point to run list
    MemCpy( RootRunList, Buffer2, 128 ); // Copy Root run list
    //
    // Calculate root directory size
    //
    pRunList = &RootRunList[0];
    TmpRootSize = 0;
    Cluster = 0;
    do {
        Status = GetRunListElementData( &pRunList, &ClusterCount, &Cluster, TRUE );
        if ( Status == EFI_SUCCESS ) TmpRootSize += ClusterCount;
    } while ( Status == EFI_SUCCESS );
    TmpRootSize = Mul64 ( TmpRootSize, Bs->SecPerClus );
    TmpRootSize = Mul64 ( TmpRootSize, Bs->BytsPerSec );

    Buffer2 = &FrBuffer[0];
    RootPages = EFI_SIZE_TO_PAGES( (UINTN)TmpRootSize );
    Status = (*PeiServices)->AllocatePages( PeiServices, EfiBootServicesData, RootPages, &Allocate );
    if ( EFI_ERROR( Status )) {
        return Status;
    }
    RootBuffer     = (UINT8*)((UINTN)Allocate);
    RootBufferSize = EFI_PAGES_TO_SIZE( RootPages );
    MemSet( RootBuffer, RootBufferSize, 0 );

    pRunList = &RootRunList[0];
    Status = ReadNTFSFile( PeiServices, Volume, Bs, pRunList, RootBuffer, &TmpRootSize );
    RootSize = (UINT32)TmpRootSize;

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ProcessNTFSVolume
//
// Description:
//  Reads recovery capsule from NTFS volume
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
EFI_STATUS ProcessNTFSVolume(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT RC_VOL_INFO  *Volume,
    IN OUT UINTN        *FileSize,
    OUT VOID            *Buffer )
{
    EFI_STATUS           Status;
    BOOT_SECTOR          Bs;
    UINT32               i;
    UINTN               NumberOfFiles;
    UINT64              MFTRecord;
    UINT8               *TmpBuffer;
    UINT8               *pRunList;
    UINT64              TmpFileSize;

    Status = ReadDevice( PeiServices, Volume, 0, 512, &Bs );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    if (!IsNtfs( &Bs )) {
        return EFI_NOT_FOUND;
    }

    Status = ReadNTFSRoot( PeiServices, Volume, &Bs );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    AmiGetFileListFromNtfsVolume(PeiServices, (UINT8*)RootBuffer, RootSize, &NumberOfFiles, NtfsRecoveryFiles);

    if ( NumberOfFiles == 0 )
        return EFI_NOT_FOUND;

    for(i = 0; i < NumberOfFiles; i++) {
        if ( *FileSize < NtfsRecoveryFiles[i]->INDE_RealSize )
            continue;

        TmpBuffer = (UINT8*)Buffer;
        TmpFileSize = NtfsRecoveryFiles[i]->INDE_RealSize;
        //
        // Get the file's MFT record number, and from that it's run list
        //
        MFTRecord = NtfsRecoveryFiles[i]->INDE_MFTRecord & MAXIMUM_RECORD_NUMBER;
        Status = GetFileRecord( PeiServices, Volume, &Bs, MFTRecord, TmpBuffer, NULL );
        Status = GetFrAttribute( TmpBuffer, FR_ATTRIBUTE_DATA, &TmpBuffer );
        if ( EFI_ERROR( Status )) {
            return Status;
        }
        TmpBuffer += ((FR_ATTR_HEADER_NONRES*)TmpBuffer)->AHNR_RunOffset; // Point to run list
        MemCpy( RootRunList, TmpBuffer, 128 ); // Copy the file's run list
        //
        // Read the file into the provided buffer
        //
        pRunList = &RootRunList[0];
        Status = ReadNTFSFile( PeiServices, Volume, &Bs, pRunList, Buffer, &TmpFileSize );
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
//----------------------------------------------------------------------
//
// Procedure:   AmiGetFileListFromNtfsVolume
//
// Description:
//  Gets a list of valid recovery files from an NTFS volume.
//  As currently written, gets only one file.
//
// Input:
//  UINT8 *Root - Pointer to a buffer containing the root directory
//  UINT32 RootSize - Size of the root directory
//  UINTN *NumberOfFiles - Pointer to number of files found
//  INDEX_ENTRY **Buffer - Pointer to buffer containing index entry of
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
VOID AmiGetFileListFromNtfsVolume(
    IN EFI_PEI_SERVICES **PeiServices,
    IN  UINT8               *Root,
    IN  UINT32              RootSize,
    OUT UINTN               *NumberOfFiles,
    OUT INDEX_ENTRY         **Buffer
)
{

    UINT8       i;
    UINT32      IndexSize;
    UINT32      IndexSize2;
    UINTN       Offset;
    UINT8       *TmpPtr;
    CHAR16      *NamePtr;
    CHAR8       TmpFileName[13];
    UINT8       LfnSize;
    UINT16      EntrySize;
    INDEX_ENTRY *IndxPtr;
    VOID        *FileName;
    UINTN       FileSize;
    EFI_STATUS  Status;

    *NumberOfFiles = 0;     //no files found yet

    Status = GetRecoveryFileInfo(PeiServices, &FileName, &FileSize, NULL);
    if(EFI_ERROR(Status))
        return;

    if (ResidentPresent) { // If resident index found...
        TmpPtr = &ResidentIndex[0];
        IndexSize = ResidentSize;

        do { // loop inside the index
            EntrySize = ((INDEX_ENTRY*)TmpPtr)->INDE_EntrySize;
            LfnSize = ((INDEX_ENTRY*)TmpPtr)->INDE_NameLength;
            if (LfnSize > 12) LfnSize = 12; // Limit name to 12 chars
            NamePtr = &((INDEX_ENTRY*)TmpPtr)->INDE_Name[0];
            for ( i=0; i<LfnSize; i++ )
            {
                TmpFileName[i] = (CHAR8)(CHAR16)NamePtr[i];
            }
            TmpFileName[i] = 0; // Zero-terminate name

            if(!EFI_ERROR(FileSearch((CHAR8*)FileName, TmpFileName, FALSE, LfnSize))) {
                IndxPtr = (INDEX_ENTRY*)&TmpPtr[0];
                Buffer[*NumberOfFiles] = IndxPtr; // Save pointer to this entry
                *NumberOfFiles = 1;
                return;
            }

            TmpPtr += EntrySize;
            IndexSize -= EntrySize;
            if ( IndexSize < MINIMUM_ENTRY_SIZE ) break;

        } while (IndexSize);
    }

    do { // do loop handling indexes in the root
        Offset = 0;
        // Look for "INDX", start of index record
        if ( (Root[0] == 0x49) && \
             (Root[1] == 0x4E) && \
             (Root[2] == 0x44) && \
             (Root[3] == 0x58) )
        {
            IndexSize = ((INDEX_RECORD*)Root)->INDR_IndxEntrySize;
            IndexSize2 = IndexSize;
            Offset += ((INDEX_RECORD*)Root)->INDR_IndxEntryOff + \
                        EFI_FIELD_OFFSET(INDEX_RECORD, INDR_IndxEntryOff);
            TmpPtr = Root;
            TmpPtr += Offset; // Point to first entry in index
            if (IndexSize < MINIMUM_ENTRY_SIZE) { // Empty index
                return;
            }
        } else return; // no index found

        do { // loop inside the index
            EntrySize = ((INDEX_ENTRY*)TmpPtr)->INDE_EntrySize;
            LfnSize = ((INDEX_ENTRY*)TmpPtr)->INDE_NameLength;
            if (LfnSize > 12) LfnSize = 12; // Limit name to 12 chars
            NamePtr = &((INDEX_ENTRY*)TmpPtr)->INDE_Name[0];
            for ( i=0; i<LfnSize; i++ )
            {
                TmpFileName[i] = (CHAR8)(CHAR16)NamePtr[i];
            }
            TmpFileName[i] = 0; // Zero-terminate name

            if(!EFI_ERROR(FileSearch((CHAR8*)FileName, TmpFileName, FALSE, LfnSize))) {
                IndxPtr = (INDEX_ENTRY*)&TmpPtr[0];
                Buffer[*NumberOfFiles] = IndxPtr; // Save pointer to this entry
                *NumberOfFiles = 1;
                return;
            }

            TmpPtr += EntrySize;
            IndexSize -= EntrySize;
            if ( IndexSize < MINIMUM_ENTRY_SIZE ) break;

        } while (IndexSize);
        if ( IndexSize2 < 0x1000 ) {
            IndexSize2 = 0x1000;
        } else {
            IndexSize2 = (IndexSize2 + 0x100) & 0xffffff00 ; // Round off
        }
        *Root += IndexSize2;
        RootSize -= IndexSize2;

    } while (RootSize);
}



//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ProcessNTFSDevice
//
// Description:
//  Reads recovery capsule from NTFS device. Tries to discover primary partitions
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

EFI_STATUS ProcessNTFSDevice(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT RC_VOL_INFO  *Volume,
    IN OUT UINTN        *FileSize,
    OUT VOID            *Buffer )
{
    EFI_STATUS          Status;
    EFI_STATUS          Status2;

    //
    // Assume the volume is floppy-formatted.
    //
    Volume->PartitionOffset = 0;
    Status = ProcessNTFSVolume( PeiServices, Volume, FileSize, Buffer );

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
            Status2 = ProcessNTFSVolume( PeiServices, Volume, FileSize, Buffer );
            if ( !EFI_ERROR(Status2) ) {
                return Status2;
            }
        }
    } while (Status == EFI_SUCCESS);

    return EFI_NOT_FOUND;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
