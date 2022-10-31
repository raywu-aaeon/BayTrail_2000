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
// $Header: /Alaska/SOURCE/Core/Modules/Recovery/FsRecovery.c 19    5/13/11 5:07p Artems $
//
// $Revision: 19 $
//
// $Date: 5/13/11 5:07p $
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name:        FsRecovery.c
//
// Description: Recovery Filesytem support
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>

#include <AmiPeiLib.h>
#include <Token.h>
#include "FsRecovery.h"

#include <Ppi/DeviceRecoveryModule.h>
#include <Ppi/BlockIo.h>

#include <Guid/AmiRecoveryDevice.h>
#include <FsRecoveryElinks.h>

#define FAT_FILE_NAME_SIZE 11

extern BOOLEAN NtfsRecoverySupport;
UINT8               *ReadBuffer    = NULL;
UINTN               BufferSize     = 0;
UINT8               *FatBuffer     = NULL;
UINTN               FatBufferSize  = 0;
UINT8               *RootBuffer    = NULL;
UINTN               RootBufferSize = 0;
UINT32              RootEntries    = 0;
UINT32              RootSize       = 0;
UINTN               PartCount;
BOOLEAN             IsMbr;
UINT32              GpeCount;
UINT32              PartSector;
MASTER_BOOT_RECORD  Mbr;
DIR_ENTRY           *FatRecoveryFiles[10];
DIR_RECORD          *CdRecoveryFiles[10];
EFI_PEI_SERVICES    **ThisPeiServices;

extern FsRecovery_Devices FSRECOVERY_LIST EndOfFsRecoverySupport;
FsRecovery_Devices *FsRecoverySupport[] = {
        FSRECOVERY_LIST NULL
};
//***************************************************************************

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   toupper
//
// Description: 
//  Converts lower case characters to upper case
//
// Input:       
//  IN CHAR8 c - character to convert
//
// Output:      
//  CHAR8 - converted character value
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
CHAR8 toupper(
    IN CHAR8 c )
{
    return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FileCompare
//
// Description: 
//  This function takes a filename and a filename pattern and tries to make 
//  a match.  It can account for Wild characters * and ? in the pattern. Also,
//  the pattern can be multiple comma separated patterns.
//
// Input:       
//  IN CHAR8 *RecoveryFileName - recover file pattern string comma separated
//  IN CHAR8 *FsFilename - file name to check against the pattern string
//  IN BOOLEAN IgnoreSpacesInFilename - If true, ignore spaces in FsFilename when pattern string is a period
//
// Output:      
//  TRUE - Pattern matches filename
//  FALSE - Pattern doesn't match filename
//
// Notes:
//  RecoveryFileNamePattern is taken from the RECOVERY_ROM token and should look
//  like this:   *.rom,ab??rt??.bio,123.bin  etc.
//  The Parameter IgnoreSpacesInFilename is for use with file systems that pad
//  spaces into filenames and don't use periods.  If TRUE, it matches a period 
//  in the pattern to any number of spaces in the filename.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN FileCompare(
    IN CHAR8 *RecoveryFileNamePattern, 
    IN CHAR8 *FsFilename, 
    IN BOOLEAN IgnoreSpacesInFilename,
    IN UINT32 FileNameLength)
{
    CHAR8  ch1, ch;
    UINTN len1, len2;

    for (; ;) {
        // check length of string
        len1 = Strlen(FsFilename);
        len2 = Strlen(RecoveryFileNamePattern);

        // if len1 is 0 then at end of filename
        if(!len1 || !FileNameLength)
        {
            // if len2 is 0 then also at end of pattern so file name is 
            //  equal to a pattern
            if(!len2 )
                return TRUE;
            else
            {
                // if len2 is a comma, then it is the same as len2 == 0 so 
                //  file name is equal to a pattern
                ch1 = *RecoveryFileNamePattern;
                if (ch1 == ',')
                {
                    return TRUE;
                }
                // if not a comma or 0, then file does not fit the pattern
                return FALSE;
            }
        }

        // get next character of the pattern
        ch1 = *RecoveryFileNamePattern;
        RecoveryFileNamePattern ++;

        switch (ch1) 
        {
            case ',':
                return TRUE;
                break;

            // wild character, it must deal with any number of matching characters 
            //  in the file name string
            case '*':                               
                while (*FsFilename) 
                {
                    if (FileCompare (RecoveryFileNamePattern, FsFilename, IgnoreSpacesInFilename, FileNameLength)) 
                    {
                        return TRUE;
                    }        
                    FsFilename ++;
                    FileNameLength--;
                }
                return FileCompare (RecoveryFileNamePattern, FsFilename, IgnoreSpacesInFilename, FileNameLength);
                break;

            // wild character, it must deal with a matching character in the file name string
            case '?':                               
                if (!*FsFilename) 
                {
                    return FALSE;
                }

                FsFilename ++;
                FileNameLength--;
                break;

            // in this case statement the case '.' must be directly above the default case.
            // if IgnoreSpacesInFilename is FALSE, it is supposed to fall through into default.
            // If IgnoreSpacesInFilenameis TRUE, process the period as a check for spaces character.
            //  then once we skip over all spaces, if there are any, then it moves to the 
            //  next character in the pattern
            case '.':
                // FAT, spaces added to file name to make 8 3  -- no period in the filename either
                if (IgnoreSpacesInFilename == TRUE)  
                {
                    ch = *FsFilename;

                    while ((ch == ' ') && (ch != 0))
                    {
                        FsFilename ++;
                        FileNameLength--;
                        ch = *FsFilename;
                    }
                    break;
                }
                // CDFS, no spaces and there is a period.  Let it fall through to the default case
            default:
                ch = *FsFilename;
                if (toupper(ch) != toupper(ch1)) 
                {
                    return FALSE;
                }
                FsFilename ++;
                FileNameLength--;
                break;
        }
    }        
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FileSearch
//
// Description: 
//  Wrapper function for parsing through a pattern string with multiple entries
//  Each entry is comma separated and can include wild characters like * and ?
//  The Function can handle CDFS and FAT file systems.  
//
// Input:       
//  IN CHAR8 *RecoveryFileName - recover file pattern string comma separated
//  IN CHAR8 *FsFilename - file name to check against the pattern string
//  IN BOOLEAN IgnoreSpacesInFilename - If true, ignore spaces in FsFilename when pattern string is a period
//
// Output:      
//  EFI_SUCCESS - File name fits one of the Pattern strings in RecoveryFileName
//  EFI_INVALID_PARAMETER - one of the strings is NULL or file name doesn't fit pattern string
//
// Notes:
//  
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS FileSearch(
    IN CHAR8 *RecoveryFileName, 
    IN CHAR8 *FsFilename, 
    IN BOOLEAN IgnoreSpacesInFilename,
    IN UINT32   FileNameLength)
{
    CHAR8 *RecStrPtr = RecoveryFileName;
    CHAR8 *FilenamePtr = FsFilename;

    if (*RecStrPtr == 0) 
    {
        return EFI_INVALID_PARAMETER;
    }

    if (*FsFilename == 0)
    {
        return EFI_INVALID_PARAMETER;
    }   

    if (FileNameLength == 0 ) 
    {
        return EFI_INVALID_PARAMETER;
    }   
   

    // loop until all possibilities listed in the RecoveryFileName are exhausted
    do {
        // Now compare the current possiblity to the current filename
        FilenamePtr = FsFilename;

        if (*RecStrPtr == ',') 
            RecStrPtr++;

        if (FileCompare(RecStrPtr, FsFilename, IgnoreSpacesInFilename, FileNameLength) == TRUE)
            return EFI_SUCCESS;
        
        while (*RecStrPtr != ',' && *RecStrPtr != 0)
            RecStrPtr ++;

    } while (*RecStrPtr != 0);

    return EFI_INVALID_PARAMETER;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ConvertToFatFileName
//
// Description: 
//  Converts file name from "XXXXX.XXX" form to FAT form
//
// Input:       
//  IN  CHAR8 *inFileName - pointer to input file name
//  OUT CHAR8 *outFileName - pointer to output file name
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ConvertToFatFileName(
    IN CHAR8  *inFileName,
    OUT CHAR8 *outFileName )
{
    UINT32 i = 0;
    UINT32 j = 0;

    for ( i = 0; inFileName[i] && inFileName[i] != '.'; i++ ) {
        outFileName[i] =     toupper( inFileName[i] );
    }
    j = i;

    for (; i < 8; i++ ) {
        outFileName[i] = ' ';
    }

    if ( inFileName[j] == '.' ) {
        for ( j++; inFileName[j]; i++, j++ ) {
            outFileName[i] = toupper( inFileName[j] );
        }
    }

    for (; i < 11; i++ ) {
        outFileName[i] = ' ';
    }
    outFileName[i] = 0;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReadDevice
//
// Description: Reads data from Block device
//
// Input:       
//  IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//  IN  RC_VOL_INFO *Volume - pointer to volume description structure
//  IN  UINT64 Start - starting offset in bytes
//  IN  UINTN Size - size of the data to read
//  VOID *Buffer - pointer to buffer to store data
//
// Output:      
//  EFI_SUCCESS
//  Errors
//      - returns either the error status from Allocate Pages or Read Blocks
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ReadDevice(
    IN EFI_PEI_SERVICES **PeiServices,
    IN RC_VOL_INFO      *Volume,
    IN UINT64           Start,
    IN UINTN            Size,
    OUT VOID            *Buffer )
{
    UINT64               StartLba;
    UINTN                ActualSize;
    UINTN                ActualPages;
    UINT32               StartOffset;
    EFI_STATUS           Status;
    EFI_PHYSICAL_ADDRESS Allocate;

    Start += Volume->PartitionOffset;

    if ( Volume->BlockSize == 4096 )  {
        StartLba    = Shr64( Start, 12 );
        StartOffset = (UINT32)( Start & 0xfff );
    } else if ( Volume->BlockSize == 2048 )  {
        StartLba    = Shr64( Start, 11 );
        StartOffset = (UINT32)( Start & 0x7ff );
    } else {
        StartLba    = Shr64( Start, 9 );
        StartOffset = (UINT32)( Start & 0x1ff );
    }

    ActualSize  = ((StartOffset + Size + Volume->BlockSize - 1) / Volume->BlockSize) * Volume->BlockSize;
    ActualPages = EFI_SIZE_TO_PAGES( ActualSize );

    if ( BufferSize < EFI_PAGES_TO_SIZE( ActualPages )) {
        Status = (*PeiServices)->AllocatePages( PeiServices, EfiBootServicesData, ActualPages, &Allocate );

        if ( EFI_ERROR( Status )) {
            return Status;
        }

        ReadBuffer = (UINT8*)((UINTN)Allocate);
        BufferSize = EFI_PAGES_TO_SIZE( ActualPages );
    }

    Status = Volume->BlkIo->ReadBlocks( PeiServices,
                                        Volume->BlkIo,
                                        Volume->Device,
                                        StartLba,
                                        ActualSize,
                                        ReadBuffer );

    if ( !EFI_ERROR( Status )) {
        MemCpy( Buffer, ReadBuffer + StartOffset, Size );
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsFat
//
// Description: 
//  Checks if given data block describes FAT structure
//
// Input:       
//  BOOT_SECTOR *pBpb - pointer to data block to check
//
// Output:      
//  TRUE - data block is a FAT structure
//  FALSE - data block is not a FAT structure
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsFat(
    IN BOOT_SECTOR *pBpb )
{
    return pBpb->BytsPerSec <= 4096
           && pBpb->SecPerClus && pBpb->SecPerClus <= 128
           && pBpb->RsvdSecCnt
           && pBpb->NumFATs
           && pBpb->Signature == 0xAA55
           && (pBpb->jmp[0] == 0xEB || pBpb->jmp[0] == 0xE9);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetClustersCount
//
// Description: 
//  Returns number of clusters for given cluster chain
//
// Input:       
//  IN  UINT8 FatType - FAT type (FAT12, FAT16 or FAT32)
//  IN  UINT32 CurrentCluster - first cluster of cluster chain
//  UINT32 *NextCluster - first cluster of next cluster chain if there is break
//  IN  BOOLEAN Continuous - if TRUE, returns only number of subsequent clusters in chain
//                           if FALSE, returns total number of clusters in cluster chain
//
// Output:      
//  UINT32 - number of clusters
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32 GetClustersCount(
    IN UINT8   FatType,
    IN UINT32  CurrentCluster,
    OUT UINT32 *NextCluster,
    IN BOOLEAN Continuous )
{
    UINT32 Count = 0;
    UINT32 WorkCluster;
    UINT32 Cluster = CurrentCluster;

    if ( FatType == FAT16 ) {
        UINT16 *Fat16 = (UINT16*)FatBuffer;
        while ( TRUE )
        {
            Count++;
            WorkCluster = Fat16[Cluster];

            if ( WorkCluster > 0xfff8 ) {
                *NextCluster = 0;
                break;
            }

            if ( WorkCluster != Cluster + 1 && Continuous ) {
                *NextCluster = WorkCluster;
                break;
            }
            Cluster = WorkCluster;
        }
    } else if ( FatType == FAT32 ) {
        UINT32 *Fat32 = (UINT32*)FatBuffer;
        while ( TRUE )
        {
            Count++;
            WorkCluster = Fat32[Cluster] & 0xfffffff;

            if ( WorkCluster > 0xffffff8 ) {
                *NextCluster = 0;
                break;
            }

            if ( WorkCluster != Cluster + 1 && Continuous ) {
                *NextCluster = WorkCluster;
                break;
            }
            Cluster = WorkCluster;
        }
    } else {
        while ( TRUE ) {
            Count++;
            WorkCluster = *(UINT16*)(FatBuffer + Cluster + Cluster / 2);
            WorkCluster = (Cluster & 1) ? WorkCluster >> 4 : WorkCluster & 0xfff;

            if ( WorkCluster > 0xff8 ) {
                *NextCluster = 0;
                break;
            }

            if ( WorkCluster != Cluster + 1 && Continuous ) {
                *NextCluster = WorkCluster;
                break;
            }
            Cluster = WorkCluster;
        }
    }
    return Count;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetFatData
//
// Description: 
//  Reads data from FAT device
//
// Input:       
//  IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//  IN  RC_VOL_INFO *Volume - pointer to volume description structure
//  IN  UINT32 FirstCluster - starting cluster
//  IN  UINTN Size - size of the data to read
//  OUT VOID *Buffer - pointer to buffer to store data
//
// Output:      
//  EFI_SUCCESS - correctly read all FAT data
//  EFI_ABORTED - should never get this.
//  Other - any errors reported from ReadDevice function
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetFatData(
    IN EFI_PEI_SERVICES **PeiServices,
    IN RC_VOL_INFO      *Volume,
    IN UINT32           FirstCluster,
    IN UINTN            Size,
    OUT VOID            *Buffer )
{
    EFI_STATUS Status;
    UINT32     Count;
    UINT32     NextCluster;
    UINT32     Cluster = FirstCluster;
    UINTN      SizeToRead;

    while ( TRUE )
    {
        SizeToRead = Size;
        Count      = GetClustersCount( Volume->FatType, Cluster, &NextCluster, TRUE );
        SizeToRead = (SizeToRead > Count * Volume->BytesPerCluster) ? Count * Volume->BytesPerCluster : SizeToRead;
        Status     = ReadDevice( PeiServices,
                                 Volume,
                                 Volume->DataOffset + Mul64((UINT64)(Cluster - 2), Volume->BytesPerCluster),
                                 SizeToRead,
                                 Buffer );

        if ( EFI_ERROR( Status ) || NextCluster == 0 ) {
            return Status;
        }

        Cluster         = NextCluster;
        (UINT8*)Buffer += SizeToRead;
        Size           -= SizeToRead;
    }
    return EFI_ABORTED;     //should never get here
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetFatType
//
// Description: 
//  Prepares given volume for read operations. Reads FAT table, root directory,
//  determines FAT type
//
// Input:       
//  IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//  IN  OUT RC_VOL_INFO *Volume - pointer to volume description structure
//  IN  BOOT_SECTOR *Bs - pointer to MBR or diskette FAT data
//
// Output:      
//  EFI_STATUS - possible return values from ReadDevice, AllocatePages, GetFatData
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetFatType(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT RC_VOL_INFO  *Volume,
    IN BOOT_SECTOR      *Bs )
{
    EFI_STATUS           Status;
    UINT32               TotalSectors;
    UINT32               FatSectors;
    UINT32               RootSectors;
    UINT32               DataSectors;
    UINT32               RootSize;
    UINT32               FatSize;
    UINT32               DataClusters;
    UINT32               RootClusters;
    UINT32               DummyCluster;
    UINTN                FatPages;
    UINTN                RootPages;
    EFI_PHYSICAL_ADDRESS Allocate;

    FatSectors   = (Bs->FATSz16 != 0) ? Bs->FATSz16 : Bs->Fat.Fat32.FATSz32;
    FatSize      = FatSectors * Bs->BytsPerSec;
    TotalSectors = (Bs->TotSec16 != 0) ?  Bs->TotSec16 : Bs->TotSec32;
    RootSectors  = ((Bs->RootEntCnt * 32) + (Bs->BytsPerSec - 1)) / Bs->BytsPerSec;
    RootSize     = RootSectors * Bs->BytsPerSec;
    DataSectors  = TotalSectors - RootSectors - FatSectors * Bs->NumFATs - Bs->RsvdSecCnt;
    DataClusters = DataSectors / Bs->SecPerClus;

    Volume->FatOffset       = Bs->RsvdSecCnt * Bs->BytsPerSec;
    Volume->RootOffset      = Volume->FatOffset + FatSectors * Bs->NumFATs * Bs->BytsPerSec;
    Volume->DataOffset      = Volume->RootOffset + RootSize;
    Volume->BytesPerCluster = Bs->BytsPerSec * Bs->SecPerClus;
    Volume->FatType         = (DataClusters >= 65525) ? FAT32 : ((DataClusters < 4085) ? FAT12 : FAT16);

    RootEntries = Bs->RootEntCnt;

    //
    //Read FAT table
    //
    FatPages = EFI_SIZE_TO_PAGES( FatSize );

    if ( FatBufferSize < EFI_PAGES_TO_SIZE( FatPages )) {
        Status = (*PeiServices)->AllocatePages( PeiServices, EfiBootServicesData, FatPages, &Allocate );

        if ( EFI_ERROR( Status )) {
            return Status;
        }

        FatBuffer     = (UINT8*)((UINTN)Allocate);
        FatBufferSize = EFI_PAGES_TO_SIZE( FatPages );
    }
    MemSet( FatBuffer, FatBufferSize, 0 );
    Status = ReadDevice( PeiServices, Volume, Volume->FatOffset, FatSize, FatBuffer );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    //
    //Read Root directory
    //
    if ( RootSize == 0 ) {      
        //    
        //in case of FAT32 it will be so at this time
        //
        RootClusters = GetClustersCount( FAT32, Bs->Fat.Fat32.RootClus, &DummyCluster, FALSE );
        RootSize    = RootClusters * Volume->BytesPerCluster;
        RootEntries = RootSize / 32;
    }

    RootPages = EFI_SIZE_TO_PAGES( RootSize );

    if ( RootBufferSize < EFI_PAGES_TO_SIZE( RootPages )) {
        Status = (*PeiServices)->AllocatePages( PeiServices, EfiBootServicesData, RootPages, &Allocate );

        if ( EFI_ERROR( Status )) {
            return Status;
        }

        RootBuffer     = (UINT8*)((UINTN)Allocate);
        RootBufferSize = EFI_PAGES_TO_SIZE( RootPages );
    }
    MemSet( RootBuffer, RootBufferSize, 0 );

    if ( Volume->FatType == FAT32 ) {
        Status = GetFatData( PeiServices, Volume, Bs->Fat.Fat32.RootClus, RootSize, RootBuffer );
    }
    else {
        Status = ReadDevice( PeiServices, Volume, Volume->RootOffset, RootSize, RootBuffer );
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ProcessFatVolume
//
// Description: 
//  Reads recovery capsule from FAT volume
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
EFI_STATUS ProcessFatVolume(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT RC_VOL_INFO  *Volume,
    IN OUT UINTN        *FileSize,
    OUT VOID            *Buffer )
{
    EFI_STATUS           Status;
    BOOT_SECTOR          Bs;
    UINT32               i;
    UINT32               FirstFileCluster;
    UINTN                NumberOfFiles;

    Status = ReadDevice( PeiServices, Volume, 0, 512, &Bs );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    if ( !IsFat( &Bs )) {
        return EFI_NOT_FOUND;
    }

    Status = GetFatType( PeiServices, Volume, &Bs );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    AmiGetFileListFromFatVolume((DIR_ENTRY*)RootBuffer, RootEntries, &NumberOfFiles, FatRecoveryFiles);
    
    if ( NumberOfFiles == 0 )
        return EFI_NOT_FOUND;

    for(i = 0; i < NumberOfFiles; i++) {
        if ( *FileSize < FatRecoveryFiles[i]->FileSize )
            continue;

        FirstFileCluster = (FatRecoveryFiles[i]->FirstClusterHi << 16) + FatRecoveryFiles[i]->FirstClusterLo;

        Status = GetFatData( PeiServices, Volume, FirstFileCluster, FatRecoveryFiles[i]->FileSize, Buffer );
        if(EFI_ERROR(Status))
            continue;

        if(AmiIsValidFile(Buffer, FatRecoveryFiles[i]->FileSize)) {
            *FileSize = FatRecoveryFiles[i]->FileSize;
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ProcessFatDevice
//
// Description: 
//  Reads recovery capsule from FAT device. First treat device as 
//  non-partitioned device. If failed tries to discover primary partitions and 
//  search for capsule there.
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
EFI_STATUS ProcessFatDevice(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT RC_VOL_INFO  *Volume,
    IN OUT UINTN        *FileSize,
    OUT VOID            *Buffer )
{
    EFI_STATUS         Status;
    EFI_STATUS          Status2;  //<EIP153486+>
//<EIP153486-> >>>
/*
    UINT32             i;
    MASTER_BOOT_RECORD Mbr;
    RC_VOL_INFO        SaveVolume;

    //
    //save volume info
    //
    SaveVolume = *Volume;
    if ( !( PcdGetBool(PcdFatRecoverySupport)) )
    {
        return EFI_UNSUPPORTED;
    }
*/
//<EIP153486-> <<<

    //
    //assume first sector is FAT
    //
    Status = ProcessFatVolume( PeiServices, Volume, FileSize, Buffer );

    if ( !EFI_ERROR( Status )) {
        return Status;
    }

//<EIP153486-> >>>
/*
    //
    //restore volume info
    //
    *Volume = SaveVolume;
*/
//<EIP153486> <<<

    //
    //sector 0 doesn't contain FAT table, check for MBR
    //

    //
    //read sector 0
    //
    Status = ReadDevice( PeiServices, Volume, 0, 512, &Mbr );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    if ( Mbr.Sig != 0xaa55 ) {
        return EFI_NOT_FOUND;
    }

//<EIP153486-> >>>
/*
    for ( i = 0; i < 4; i++ )
    {
        if ( Mbr.PartRec[i].OSType == 0
             || Mbr.PartRec[i].OSType == 5           //extended partition not supported
             || Mbr.PartRec[i].OSType == 15          //extended partition not supported
             || Mbr.PartRec[i].SizeInLba == 0
             || Mbr.PartRec[i].StartingLba == 0 ) {
            continue;
        }

        Volume->PartitionOffset += Mbr.PartRec[i].StartingLba * 512;
        Status = ProcessFatVolume( PeiServices, Volume, FileSize, Buffer );

        if ( !EFI_ERROR( Status )) {
            return Status;
        }

        *Volume = SaveVolume;
    }
*/
//<EIP153486-> <<<

//<EIP153486+> >>>
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
            Status2 = ProcessFatVolume( PeiServices, Volume, FileSize, Buffer );
            if ( !EFI_ERROR(Status2) ) {
                return Status2;
            }
        }
    } while (Status == EFI_SUCCESS);
//<EIP153486+> <<<

    return EFI_NOT_FOUND;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ProcessPrimaryVolume
//
// Description: 
//  Reads recovery capsule from ISO9660 device
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
EFI_STATUS ProcessPrimaryVolume(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT RC_VOL_INFO  *Volume,
    IN OUT UINTN        *FileSize,
    OUT VOID            *Buffer )
{
    EFI_STATUS                Status;
    PRIMARY_VOLUME_DESCRIPTOR PriVol;
    UINT32                    RootSize;
    UINTN                     RootPages;
    EFI_PHYSICAL_ADDRESS      Allocate;
    UINTN                     NumberOfFiles;
    UINT32                    i;

    Volume->PartitionOffset = 0;
    Status = ReadDevice( PeiServices, Volume, 16 * Volume->BlockSize, Volume->BlockSize, &PriVol );

    //
    //check that we read CD
    //
    if ( PriVol.Type != 1 || MemCmp( PriVol.StandardId, "CD001", 5 )) {
        return EFI_NOT_FOUND;
    }

    //
    //read root directory
    //
    RootSize  = PriVol.RootDir.DataLength;
    RootPages = EFI_SIZE_TO_PAGES( RootSize );

    if ( RootBufferSize < EFI_PAGES_TO_SIZE( RootPages )) {
        Status = (*PeiServices)->AllocatePages( PeiServices, EfiBootServicesData, RootPages, &Allocate );

        if ( EFI_ERROR( Status )) {
            return Status;
        }

        RootBuffer     = (UINT8*)((UINTN)Allocate);
        RootBufferSize = EFI_PAGES_TO_SIZE( RootPages );
    }
    MemSet( RootBuffer, RootBufferSize, 0 );

    Status = ReadDevice( PeiServices, Volume, PriVol.RootDir.ExtentOffset * Volume->BlockSize, RootSize, RootBuffer );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    AmiGetFileListFromPrimaryVolume((DIR_RECORD*)RootBuffer, RootSize, &NumberOfFiles, CdRecoveryFiles);

    if(NumberOfFiles == 0)
        return EFI_NOT_FOUND;

    for(i = 0; i < NumberOfFiles; i++) {
        if ( *FileSize < CdRecoveryFiles[i]->DataLength )
            continue;

        Status = ReadDevice( PeiServices, Volume, CdRecoveryFiles[i]->ExtentOffset * Volume->BlockSize, CdRecoveryFiles[i]->DataLength, Buffer );
        if(EFI_ERROR(Status)) 
            continue;

        if(AmiIsValidFile(Buffer, CdRecoveryFiles[i]->DataLength)) {
            *FileSize = CdRecoveryFiles[i]->DataLength;
            return EFI_SUCCESS;
        }
    }
    return EFI_NOT_FOUND;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ProcessCd
//
// Description: 
//  Reads recovery capsule ATAPI device. First search for recovery capsule in
//  primary volume. If not found tries to process Eltorito images
//
// Input:       
//  IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//  IN  OUT RC_VOL_INFO *Volume - pointer to volume description structure
//  CHAR8 *FileName - recovery capsule file name
//  UINTN *FileSize - pointer to size of provided buffer
//  OUT VOID *Buffer - pointer to buffer to store data
//
// Output:      
//  EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ProcessCd(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT RC_VOL_INFO  *Volume,
    IN OUT UINTN        *FileSize,
    OUT VOID            *Buffer )
{
    EFI_STATUS                    Status;
    UINT8                         Data[2048];       //space for 1 block
    BOOT_RECORD_VOLUME_DESCRIPTOR *BootDesc;
    INITIAL_DEFAULT_ENTRY         *Entry;

    if ( !(Volume->BlockSize == 2048 && PcdGetBool(PcdCdRecoverySupport)) ){
        return EFI_UNSUPPORTED;
    }

    Status = ProcessPrimaryVolume( PeiServices, Volume, FileSize, Buffer );

    if ( !EFI_ERROR( Status )) {
        return Status;
    }

    //
    //file not found in primary volume, check Eltorito partitions
    //
    Status = ReadDevice( PeiServices, Volume, 17 * Volume->BlockSize, Volume->BlockSize, Data );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    //
    //check if it is Eltorito
    //
    BootDesc = (BOOT_RECORD_VOLUME_DESCRIPTOR*)Data;

    if ( BootDesc->BootRecordIndicator != 0
         || MemCmp( &(BootDesc->ISO9660Identifier),    "CD001",                   5 )
         || BootDesc->DescriptorVersion != 1
         || MemCmp( &(BootDesc->BootSystemIdentifier), "EL TORITO SPECIFICATION", 23 )) {
        return EFI_NOT_FOUND;
    }

    //
    //it is Eltorito, read boot catalog
    //
    Status = ReadDevice( PeiServices, Volume, BootDesc->BootCatalogFirstSector * Volume->BlockSize, Volume->BlockSize, Data );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    Entry                   = (INITIAL_DEFAULT_ENTRY*) &Data[32];
    Volume->PartitionOffset = Entry->LoadRBA * Volume->BlockSize;
    Status                  = ProcessFatDevice( PeiServices, Volume, FileSize, Buffer );
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FsRecoveryRead
//
// Description: 
//  Search for recovery capsule file on all file system devices
//
// Input:       
//  IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//  IN  CHAR8 *FileName - recovery capsule file name
//  IN UINTN DeviceIndex - device index for given BlockIo PPI
//  IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *pBlockIo - pointer to BlockIo PPI
//  UINTN *FileSize - pointer to size of provided buffer
//  VOID *Buffer - pointer to buffer to store data
//
// Output:      
//  EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS FsRecoveryRead(
    IN EFI_PEI_SERVICES              **PeiServices,
    IN UINTN                         DeviceIndex,
    IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *pBlockIo,
    IN OUT UINTN                     *pSize,
    OUT VOID                         *pBuffer )
{
    EFI_STATUS             Status;
    RC_VOL_INFO            Volume;
    EFI_PEI_BLOCK_IO_MEDIA Media;
    UINT32                 Index;

    if ( !pBlockIo || !pSize || *pSize && !pBuffer ) {
        return EFI_INVALID_PARAMETER;
    }

    MemSet( &Volume, sizeof(RC_VOL_INFO), 0 );

    Status = pBlockIo->GetBlockDeviceMediaInfo( PeiServices, pBlockIo, DeviceIndex, &Media );

    if ( EFI_ERROR( Status )) {
        return Status;
    }

    if ( !Media.MediaPresent ) {
        return EFI_NOT_FOUND;
    }

    Volume.BlkIo     = pBlockIo;
    Volume.Device    = DeviceIndex;
    Volume.BlockSize = Media.BlockSize;
    for(Index = 0; FsRecoverySupport[Index]!=NULL; Index++){
        Status = FsRecoverySupport[Index](PeiServices, &Volume, pSize, pBuffer);
        if (Status == EFI_SUCCESS) break;
    }


    return Status;
}

/************************************************************************/
/*              Device Recovery Module PPI                              */
/************************************************************************/
EFI_STATUS
FwCapsuleInfo (
  IN EFI_PEI_SERVICES  **PeiServices,
  IN OUT VOID          **pCapsuleName,
  IN OUT UINTN         *pCapsuleSize,
  OUT   BOOLEAN        *ExtendedVerification
)
#if defined(SecFlashUpd_SUPPORT) && SecFlashUpd_SUPPORT == 1
;
#else
{
    return EFI_SUCCESS;
}
#endif

EFI_STATUS GetRecoveryFileInfo(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT VOID         **pCapsuleName,
    IN OUT UINTN        *pCapsuleSize,
    OUT    BOOLEAN      *ExtendedVerification
)
{
    if(pCapsuleName != NULL)
        *pCapsuleName = "AMI.ROM";

    if(pCapsuleSize != NULL)
        *pCapsuleSize = (UINTN) PcdGet32 (PcdRecoveryImageSize);

    if(ExtendedVerification != NULL)
        *ExtendedVerification = FALSE;

    return FwCapsuleInfo(PeiServices, pCapsuleName, pCapsuleSize, ExtendedVerification);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetNumberRecoveryCapsules
//
// Description:
//  GetNumberRecoveryCapsules function of ppi 
//  EFI_PEI_DEVICE_RECOVERY_MODULE_PPI.
//
// Input:
//
// Output:
//  EFI_SUCCESS - number of recovery capsules returned
//  EFI_INVALID_PARAMETER - the pointer NumberRecoveryCapsules is NULL
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetNumberRecoveryCapsules(
    IN EFI_PEI_SERVICES                   **PeiServices,
    IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI *This,
    OUT UINTN                             *NumberRecoveryCapsules )
{
    if ( !NumberRecoveryCapsules ) {
        return EFI_INVALID_PARAMETER;
    }
    *NumberRecoveryCapsules = 1;
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetRecoveryCapsuleInfo
//
// Description:
//  GetRecoveryCapsuleInfo function of ppi EFI_PEI_DEVICE_RECOVERY_MODULE_PPI 
//  for any block devices including floppies, USB keys, CD-ROMs and HDDs.
//
// Input:
//  IN EFI_PEI_SERVICES **PeiServices - pointer to PeiServices Structure
//  IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI *This - pointer to the PPI structure
//  IN UINTN CapsuleInstance - value indicating the instance of the PPI
//  OUT UINTN *Size - Size of the recovery capsule
//  OUT EFI_GUID *CapsuleType OPTIONAL - Type of recovery capsule
//
// Output:
//  EFI_SUCCESS - Parameters are valid and output parameters are updated
//  EFI_INVALID_PARAMETER - Size pointer is NULL
//  EFI_NOT_FOUND - asking for a 1 or greater instance of the PPI
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetRecoveryCapsuleInfo(
    IN EFI_PEI_SERVICES                   **PeiServices,
    IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI *This,
    IN UINTN                              CapsuleInstance,
    OUT UINTN                             *Size,
    OUT EFI_GUID                          *CapsuleType )
{
    EFI_STATUS Status;

    if ( !Size ) {
        return EFI_INVALID_PARAMETER;
    }

    if ( CapsuleInstance > 0 ) {
        return EFI_NOT_FOUND;
    }

    Status = GetRecoveryFileInfo(PeiServices, NULL, Size, NULL);
    if(EFI_ERROR(Status))
        return Status;

    if ( CapsuleType ) {
        *CapsuleType = gBlockDeviceCapsuleGuid;
    }
    return EFI_SUCCESS;
}

#define NUMBER_OF_RETRIES 3

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   LoadRecoveryCapsule
//
// Description:
//  Locates all EFI_PEI_RECOVERY_BLOCK_IO_PPI PPIs.  Calls function 
//  GetNumberOfBlockDevices.  For each block device, calls the function 
//  FsRecoveryRead, to find the recovery image named in var sAmiRomFile.
//
// Input:
//  IN EFI_PEI_SERVICES **PeiServices - pointer to PeiServices Structure 
//  IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI *This - pointer to the PPI structure
//  IN UINTN CapsuleInstance - value indicating the instance of the PPI
//  OUT VOID *Buffer - contains information read from the block device
//
// Output:
//  EFI_SUCCESS - File read from recovery media
//  EFI_INVALID_PARAMETER - Buffer is a NULL pointer
//  EFI_NOT_FOUND - asking for a 1 or greater instance of the PPI
//  Other - return error values from LocatePpi or FsRecoveryRead
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS LoadRecoveryCapsule(
    IN EFI_PEI_SERVICES                   **PeiServices,
    IN EFI_PEI_DEVICE_RECOVERY_MODULE_PPI *This,
    IN UINTN                              CapsuleInstance,
    OUT VOID                              *Buffer )
{
    EFI_STATUS             Status;
    EFI_PEI_PPI_DESCRIPTOR *pDummy;
    UINTN                  i;
    UINTN                  RecoveryCapsuleSize;
    BOOLEAN                ExtendedVerification;

    PEI_TRACE((-1, PeiServices, "..BLOCK DEVICE.."));

    if ( !Buffer ) {
        return EFI_INVALID_PARAMETER;
    }

    if ( CapsuleInstance > 0 ) {
        return EFI_NOT_FOUND;
    }

    Status = GetRecoveryFileInfo(PeiServices, NULL, &RecoveryCapsuleSize, &ExtendedVerification);
    if ( EFI_ERROR( Status ))
        return Status;

    i = 0;

    do
    {
        EFI_PEI_RECOVERY_BLOCK_IO_PPI *pBlockIo;
        UINTN                         NumberBlockDevices;
        UINTN                         j;
        UINTN                         Size;
        Status = (*PeiServices)->LocatePpi( PeiServices, &gEfiPeiVirtualBlockIoPpiGuid, i++, &pDummy, &pBlockIo );

        if ( EFI_ERROR( Status )) {
            break;
        }

        if (EFI_ERROR( Status = pBlockIo->GetNumberOfBlockDevices(
                                            PeiServices, pBlockIo, &NumberBlockDevices))) {
            continue;
        }

        for ( j = 0; j < NumberBlockDevices; j++ ) {
            UINTN k;
            Size = RecoveryCapsuleSize;

            for ( k = 0; k < NUMBER_OF_RETRIES; k++ )
            {
                Status = FsRecoveryRead(
                    PeiServices, j, pBlockIo,
                    &Size, Buffer
                    );

                if ( !EFI_ERROR(Status) ){
                    if(ExtendedVerification || Size == RecoveryCapsuleSize )
                        return EFI_SUCCESS;
                }
            }
        }
    } while ( TRUE );
    return Status;
}

/************************************************************************/
/*              Entry Point                                             */
/************************************************************************/
EFI_PEI_DEVICE_RECOVERY_MODULE_PPI BlockDeviceRecoveryModule = {
    GetNumberRecoveryCapsules, GetRecoveryCapsuleInfo, LoadRecoveryCapsule
};

// PPI to be installed
static EFI_PEI_PPI_DESCRIPTOR      BlockDeviceRecoveryPpiList[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gEfiPeiDeviceRecoveryModulePpiGuid, &BlockDeviceRecoveryModule
    }
};


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   FsRecoveryPeimEntry
//
// Description:
//  Installs EFI_PEI_DEVICE_RECOVERY_MODULE_PPI for loading recovery 
//  images from block devices such as floppies, USB keys, HDDs or CD-ROMs
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS FsRecoveryPeimEntry(
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices )
{
    ThisPeiServices = PeiServices;
    return (*PeiServices)->InstallPpi( PeiServices, BlockDeviceRecoveryPpiList );
}

//************************** AMI custom eLink implementation *******************

VOID AmiGetFileListFromPrimaryVolume(
    IN  DIR_RECORD          *Root,
    IN  UINT32              RootSize,
    OUT UINTN               *NumberOfFiles,
    OUT DIR_RECORD          **Buffer
)
{
    UINT32 Count = 0;
    CHAR8  *DirFileName;
    UINT32 FileNameSize;

    VOID *FileName;
    UINTN FileSize;
    EFI_STATUS Status;


    *NumberOfFiles = 0;     //no files found yet

    Status = GetRecoveryFileInfo(ThisPeiServices, &FileName, &FileSize, NULL);
    if(EFI_ERROR(Status))
        return;

    //
    //find file in root directory
    //
    while ( Count < RootSize ) {
        DirFileName = (CHAR8*)(Root + 1);

        if(Root->Length == 0)
            return;

        // Find the length of the file name.  The ISO9660 spec has the following structure
        // up to 8 characters then a '.' then up to 3 more characters then a ';' 
        //  then the digits that make up a number between 0 and 32767
        // The filename search uses all characters up to the ';'
        FileNameSize = 0;
        while(DirFileName[FileNameSize] != ';' && (FileNameSize < Root->LengthOfFileId)) 
            FileNameSize++;

        if (!EFI_ERROR(FileSearch((CHAR8*)FileName, DirFileName, FALSE, FileNameSize))) {
            Buffer[*NumberOfFiles] = Root;
            (*NumberOfFiles)++;
            break;
        }

        Count += Root->Length;
        (UINT8 *)Root += Root->Length;
    }
}

VOID AmiGetFileListFromFatVolume(
    IN  DIR_ENTRY           *Root,
    IN  UINT32              RootEntries,
    OUT UINTN               *NumberOfFiles,
    OUT DIR_ENTRY           **Buffer
)
{
    UINT32 i;

    VOID *FileName;
    UINTN FileSize;
    EFI_STATUS Status;

    *NumberOfFiles = 0;     //no files found yet

    Status = GetRecoveryFileInfo(ThisPeiServices, &FileName, &FileSize, NULL);
    if(EFI_ERROR(Status))
        return;

    //
    //Find file in root directory
    //
    for(i = 0; i < RootEntries; i++) {
        if((Root[i].FileName[0] == 0xE5) || (Root[i].FileName[0] == 0)) 
            continue;

        if(!EFI_ERROR(FileSearch((CHAR8*)FileName, Root[i].FileName, TRUE, FAT_FILE_NAME_SIZE))) {
            Buffer[*NumberOfFiles] = &Root[i];
            *NumberOfFiles = 1;
            break;
        }
    }
}

BOOLEAN AmiIsValidFile(
    IN VOID  *FileData,
    IN UINTN FileSize
)
{
    return TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FindNextPartition
//
// Description:
//  Finds the next partition on the volume, and sets the VolumeOffset in
//  the RC_VOL_INFO structure.
//
// Input:
//  IN  EFI_PEI_SERVICES **PeiServices - pointer to PEI services
//  IN  OUT RC_VOL_INFO *Volume - pointer to volume description structure
//
// Output:
//  EFI_STATUS
//
// Note:
//  This function uses the following global variables:
//  UINTN PartCount - Counter to keep track of search
//  BOOLEAN IsMbr - True if looking for MBR partitions
//  UINT32 GpeCount - GUID Partition Entry count
//  UINT32 PartSector - Starting sector of partition
//  PartCount and PartSector must be seeded to 0, and IsMbr must be
//  seeded to TRUE before the first call.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS FindNextPartition(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT RC_VOL_INFO  *Volume )
{
    EFI_STATUS          Status;
    UINT64              Offset;
    GUID_BOOT_RECORD    *Gbr;
    GUID_TABLE_HEADER   *Gth;
    UINT32              GpeSize;
    UINT32              i;
    UINT32              TempSector;

    //
    // Check for MBR partitions
    //
    if ( IsMbr ) {
        while ( PartCount < 4 ) {
            if ( Mbr.PartRec[PartCount].OSType == 0xEE ) {
                IsMbr = FALSE; // Mark GUID partition found
                PartCount = 0; // Reset counter
                break;
            }

            if ( Mbr.PartRec[PartCount].OSType == 5
                 || Mbr.PartRec[PartCount].OSType == 15 ) { // Extended partition
                PartSector += Mbr.PartRec[PartCount].StartingLba;
                Volume->PartitionOffset = Mul64( 512, PartSector );
                Status = ReadDevice( PeiServices, Volume, 0, 512, &Mbr );
                if ( EFI_ERROR( Status )) {
                    return Status;
                }
                PartCount = 0;
            }

            if ( Mbr.PartRec[PartCount].OSType == 0
                 || Mbr.PartRec[PartCount].SizeInLba == 0
                 || Mbr.PartRec[PartCount].StartingLba == 0 ) {
                PartCount++; // Check next partition
                continue;
            }

            TempSector = Mbr.PartRec[PartCount].StartingLba + PartSector;
            Volume->PartitionOffset = Mul64( 512, TempSector );
            PartCount++;
            return EFI_SUCCESS;
        }
    }
    if ( IsMbr ) return EFI_NOT_FOUND; // No MBR partitions were found

    //
    // Check for GUID partitions
    //
    if ( PartCount == 0 ) {
        Offset = Mul64( 1, Volume->BlockSize );
        Status = ReadDevice( PeiServices, Volume, Offset, 512, &Mbr );
        if ( EFI_ERROR( Status )) {
            return Status;
        }
        Gth = (GUID_TABLE_HEADER*)&Mbr.BootCode[0];
        if ( (Gth->Signature[0] == 0x45) && // Check for "EFI"
             (Gth->Signature[1] == 0x46) &&
             (Gth->Signature[2] == 0x49) )
        {
            GpeCount = Gth->EntryCount;
            GpeSize = Gth->EntrySize;
            //
            // We only support entry size of 128 for now.
            //
            if ( GpeSize != 128 ) return EFI_NOT_FOUND; //
            Offset = Mul64( 2, Volume->BlockSize );
            //
            // Read in the first entry in the partition table
            //
            Status = ReadDevice( PeiServices, Volume, Offset, 512, &Mbr );
            if ( EFI_ERROR( Status )) {
                return Status;
            }
        } else return EFI_NOT_FOUND; // Table header not found.
    }

    while ( PartCount < GpeCount ) {
        i = PartCount % 4;
        if ( (i == 0) && (PartCount != 0) ) {
            Offset = Mul64( 2+(PartCount/512), Volume->BlockSize );
            Status = ReadDevice( PeiServices, Volume, Offset, 512, &Mbr );
            if ( EFI_ERROR( Status )) {
                return Status;
            }
        }
        PartCount++;
        Gbr = (GUID_BOOT_RECORD*)&Mbr.BootCode[0];
        Volume->PartitionOffset = Mul64( Gbr->GuidPart[i].FirstLba, Volume->BlockSize );
        return EFI_SUCCESS;
    }

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
