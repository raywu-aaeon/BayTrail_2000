//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
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
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name: FileMFTHandler.c
//
// Description:  Handles file operation that involve the Master File Table.
//
//<AMI_FHDR_END>
//**********************************************************************

#include "NTFS.h"

//----------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   ReadWriteExistingBlock
//
// Description: Reads or writes an existing file. If writing, writes
//              only an existing block, with no new allocation.
//
// Parameters:
//  IN FILE_HANDLE_INSTANCE *File - File handle instance
//  IN OUT VOID             *Buffer - Output biffer
//  IN OUT UINT32           *Size - Size written or read
//  IN REGIONS              DataRegion - Region of disk used (dir, fat, normal)
//  IN ACCESS_METHOD        Method - Method (read or write)
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:       In NTFS, a file may not take up any individual disk space
//              at all. If it is small, it's data may reside entirely within
//              the MFT file record for the file, in the Data Attribute.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
ReadWriteExistingBlock (
    IN FILE_HANDLE_INSTANCE *File,
    IN OUT VOID             *Buffer,
    IN OUT UINT32           *Size,
    IN REGIONS              DataRegion,
    IN ACCESS_METHOD        Method
    )
{
    EFI_STATUS  Status = EFI_SUCCESS;
    VOLUME_INFO *Vol = File->pFH->VolumeInterface;
    UINT64      TotalContiguousBytes;
    UINT64      TotalBytesRead = 0;
    UINT32      AbsSectorOffset;
    UINT64      AccessBytes;
    UINT64      Position = File->Position;
    UINT64      AbsSector;
    UINT64      ClusterCount;
    UINT64      Cluster = 0;
    UINT8       *pRunList;
    UINT8       *TempBuffer;
    UINT32      ResidentSize;
    UINT32      RemainingSize;

    if (Method == WRITE_BLOCK) return EFI_UNSUPPORTED;

//
// Process a resident file (data in the Data Attribute).
//
    if (File->pFH->ResidentFile == 1)
    {
        GetTempBuffer (Vol, &TempBuffer);
        Status = GetFileRecord (Vol, 
                                File->pFH->DirectoryEntry.DIR_MFTRecord, 
                                TempBuffer, NULL);
        if (EFI_ERROR(Status))
        {
            ReleaseTempBuffer (Vol);
            return Status;
        }        
        Status = GetFrAttribute (TempBuffer, FR_ATTRIBUTE_DATA, &TempBuffer);
        if (EFI_ERROR(Status))
        {
            ReleaseTempBuffer (Vol);
            return Status;
        }        
        ResidentSize = ((FR_ATTR_HEADER_RES*)TempBuffer)->AHR_InfoLength;
        TempBuffer += ((FR_ATTR_HEADER_RES*)TempBuffer)->AHR_InfoOffset;
        TempBuffer += Position; // Point to where we are in the file
        RemainingSize = *Size - (UINT32)Position;
        if (RemainingSize > ResidentSize) RemainingSize = ResidentSize;
//        
// Copy the file data to the buffer.
//        
        pBS->CopyMem (Buffer, TempBuffer, RemainingSize);
        ReleaseTempBuffer (Vol);
        *Size = RemainingSize;
        File->Position += RemainingSize;
        return EFI_SUCCESS;        
    }

//
// Process a non-resident file.
//
    pRunList = File->pRunList;

//
// ReCalculate right RunList offset and get current Cluster chain
//
    while ( Status = GetRunListElementData(&pRunList, &ClusterCount, &Cluster, TRUE) == EFI_SUCCESS )
    {
        if ( (File->CurrentCluster >= Cluster) && \
            (File->CurrentCluster < (Cluster + ClusterCount)) )
        {
            break;
        }
    }

    if (EFI_ERROR(Status))
    {
        *Size = 0;
        return Status; // The error will be EOF.
    }

    do {

        TotalContiguousBytes = Mul64(ClusterCount - (File->CurrentCluster - Cluster),
                                     Vol->VolumeBPB.BPB_SecPerClus);

        TotalContiguousBytes = Shl64(TotalContiguousBytes,
                                     Vol->BytesPerSecPowerof2);
        TotalContiguousBytes -= File->CurrentClusterOffset;

        if ( TotalContiguousBytes > *Size) AccessBytes = *Size;

        else AccessBytes = TotalContiguousBytes;

        AbsSector = ClusterToSector(Vol, File->CurrentCluster);
        AbsSector += Div64(File->CurrentClusterOffset,
                           Vol->VolumeBPB.BPB_BytePerSec,
                           (UINTN*)&AbsSectorOffset);

        if ((Method == READ_BLOCK) && (AccessBytes == 0)) {
            return EFI_VOLUME_CORRUPTED; // Will happen if early EOF.
        }

        if (Method == READ_BLOCK) {
            Status = FsReadMedia (Vol,
                                  Buffer,
                                  AbsSector,
                                  AbsSectorOffset,
                                  (UINT32)AccessBytes,
                                  DATA_REGION);

        } else { // Not used -- left here in case driver is expanded.
            Status = FsWriteMedia (Vol,
                                   Buffer,
                                   AbsSector,
                                   AbsSectorOffset,
                                   (UINT32)AccessBytes,
                                   DATA_REGION);
        }

        if (EFI_ERROR(Status)) {
            break;
        }

        (UINT8 *)Buffer += AccessBytes;
        TotalBytesRead +=AccessBytes;
//###DEBUG
// Size was left at 32 bits for FAT compatibility. In NTFS, the file size
// can be 64 bits.
//###DEBUG End        
        *Size   -= (UINT32)AccessBytes; 
        Position += AccessBytes;
        File->Position = Position;

        File->CurrentCluster += Div64((AccessBytes + \
                                       File->CurrentClusterOffset), 
                                       Vol->BytesPerCluster, 
                                       (UINTN*)&(File->CurrentClusterOffset));

        if ( (AccessBytes == TotalContiguousBytes) || \
             (File->CurrentCluster == (Cluster + ClusterCount)) )
        {
            if (*Size)
            {
                Status = GetRunListElementData (&pRunList, &ClusterCount,
                                            &Cluster, TRUE);
                if (EFI_ERROR(Status)) break; // Error here means EOF.
                File->CurrentCluster = Cluster;
                File->CurrentRunLength = ClusterCount;
            }
        }
    } while (*Size);

    *Size = (UINT32)TotalBytesRead;
    return Status;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   ReadFromFile
//
// Description: Reads a specified number of bytes from a file into a
//              specified buffer, beginning at a specified position
//              in the file. Updates Size to number of bytes read.
//              Updates Position to position after data read.
//
// Parameters:
//  IN FILE_HANDLE_INSTANCE *File - File handle instance
//  IN OUT VOID             *Buffer - Buffer to read into
//  IN OUT UINT32           *Size - Size to read (updated with size read)
//  IN REGIONS              DataRegion - Region of disk (dir, fat, normal)
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
ReadFromFile (
    IN FILE_HANDLE_INSTANCE *File,
    IN OUT VOID             *Buffer,
    IN OUT UINT32           *Size,
    IN REGIONS              DataRegion
    )
{
    EFI_STATUS    Status;
    UINT64        Position = File->Position;
    ACCESS_METHOD Method = READ_BLOCK;

//
// If Position (Zero Based) beyond EOF?
//    
    if (Position >= File->pFH->DirectoryEntry.DIR_RealSize) {
        *Size = 0;

        if (Position == File->pFH->DirectoryEntry.DIR_RealSize)
            return EFI_SUCCESS;

        else
            return EFI_DEVICE_ERROR;
    }
//
// If bytes to be read exceed the file size, truncate it.
//###DEBUG Size can be 64 bits in NTFS.
//
    if (((UINT32)Position + *Size) > \
            (UINT32)File->pFH->DirectoryEntry.DIR_RealSize)
        *Size = (UINT32)(File->pFH->DirectoryEntry.DIR_RealSize - Position);

    Status = ReadWriteExistingBlock (File,
                                     Buffer,
                                     Size,
                                     DataRegion,
                                     Method);
    return Status;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   ClusterToSector
//
// Description: Converts a custer number to an absolute sector number
//              (relative to the beginning of the volume).
//
// Parameters:
//  IN VOLUME_INFO      *Vol,         The volume structure
//  OUT UINTN           Cluster       The cluster to be converted
//
// Return value:
//  Returns the sector number.
//
// Modified:
//
// Referrals:
//
// Notes:       Cluster to Sector conversion in NTFS is straightforward,
//              since both are 0-based.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

UINT64
ClusterToSector (
    IN VOLUME_INFO *Vol,
    OUT UINT64     Cluster
    )
{
    UINT64 Sector;

    Sector = Mul64( Cluster, Vol->VolumeBPB.BPB_SecPerClus );
    return Sector;
}


//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
