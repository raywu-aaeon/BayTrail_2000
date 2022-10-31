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
// Name: Open.c
//
// Description:  Handles Opening File Handles.
//
//<AMI_FHDR_END>
//**********************************************************************

//----------------------------------------------------------------------

#include "NTFS.h"

//----------------------------------------------------------------------

extern EFI_GUID gEfiFileSystemVolumeLabelGuid;
extern EFI_GUID gEfiFileInfoGuid;
extern EFI_GUID gEfiFileSystemInfoGuid;

BOOLEAN RootSearch = FALSE;
UINT8 RootRun[32]; // Root directory run list

//----------------------------------------------------------------------


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   OpenVolume
//
// Description: Opens a file system Volume.
//
// Parameters:
//  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This - File System Protocol Instance
//  OUT EFI_FILE_PROTOCOL               **Root
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
OpenVolume (
    IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
    OUT EFI_FILE_PROTOCOL              **Root
    )
{

    EFI_STATUS           Status;
    FILE_HANDLE          *RootHandle = NULL;
    FILE_HANDLE_INSTANCE *RootInstance = NULL;
    VOLUME_INFO          *VolumeInfo = (VOLUME_INFO *)This;
    UINT32               Length;

    if (VolumeInfo->RootFH && (VolumeInfo->VolumeStatus == EFI_MEDIA_CHANGED ||\
        VolumeInfo->VolumeStatus == EFI_NO_MEDIA)) { // Media changed earlier
        FreeUpResources (VolumeInfo, VolumeInfo->VolumeStatus); 
    }

    if (VolumeInfo->RootFH) {
        if (VolumeInfo->BlkIo->Media->RemovableMedia) {
            Status = DetectVolume(VolumeInfo, TRUE); // Just reads the sector 0

            if (EFI_ERROR(Status)) {
                FreeUpResources (VolumeInfo, Status);
                goto ContinueOpen;
            }
        }

        RootHandle = VolumeInfo->RootFH;  // Everything is fine

    } else {
ContinueOpen:
        VolumeInfo->VolumeStatus = EFI_UNSUPPORTED;
        VolumeInfo->BlockSize = VolumeInfo->BlkIo->Media->BlockSize;
//###DEBUG  VolumeInfo->ReadOnly = VolumeInfo->BlkIo->Media->ReadOnly;
        VolumeInfo->ReadOnly = TRUE; // Always read-only
        VolumeInfo->MediaID = VolumeInfo->BlkIo->Media->MediaId;
        Status = DetectVolume(VolumeInfo, FALSE);

        if (EFI_ERROR(Status)) return Status;

//
// Only count Data Sectors when determining Volume Size, to match DOS
//
        VolumeInfo->VolumeSize = Mul64(VolumeInfo->DataSectors, \
                                 (UINT32)VolumeInfo->VolumeBPB.BPB_BytePerSec);
        VolumeInfo->FreeSpaceinClusters = (UINTN)-1; // Means not calculated yet
        VolumeInfo->FreeSpaceinSector1 = (UINTN)-1;
        Status= OpenRoot(VolumeInfo, &RootHandle);
        VolumeInfo->LastFreeCluster = (UINTN)-1;

        if (EFI_ERROR(Status)) return Status;

        VolumeInfo->RootFH = RootHandle;
        DListAdd(&(VolumeInfo->OpenFHs), &(RootHandle->ViFHLink));
        VolumeInfo->TotalDirEntryList = 0;
    }

//
// Record the length of the temporary buffer for this volume.
//
    Length = 4096;
    if ((VolumeInfo->BytesPerCluster * 2) > Length) 
        Length = (VolumeInfo->BytesPerCluster * 2);
    VolumeInfo->TempBufferSize = Length;

    if (!VolumeInfo->NTFSName) {
        Status = pBS->AllocatePool (EfiBootServicesData, 
				    80, 
				    (VOID**)&VolumeInfo->NTFSName);
        if (EFI_ERROR(Status)) return Status;
        Wcscpy (VolumeInfo->NTFSName, L"NTFS ");
        Length = (UINT32) Wcslen(VolumeInfo->NTFSName);

        Wcscpy (VolumeInfo->NTFSName + Length, L"File System [");
        Length = (UINT32) Wcslen(VolumeInfo->NTFSName);

        if (VolumeInfo->VolumeSize < 0x100000)  { //Less than 1MB
            ItowEx(Shr64 (VolumeInfo->VolumeSize, 10), 
		           VolumeInfo->NTFSName + Length, 10, FALSE);
            Length = (UINT32) Wcslen(VolumeInfo->NTFSName);
            Wcscpy (VolumeInfo->NTFSName + Length, L"KB]");

        } else if (VolumeInfo->VolumeSize < 0x40000000)  { //Less than 1GB
            ItowEx(Shr64 (VolumeInfo->VolumeSize, 20), 
		           VolumeInfo->NTFSName + Length, 10, FALSE);
            Length = (UINT32) Wcslen(VolumeInfo->NTFSName);
            Wcscpy (VolumeInfo->NTFSName + Length, L"MB]");

        } else {
            ItowEx(Shr64 (VolumeInfo->VolumeSize, 30), 
		           VolumeInfo->NTFSName + Length, 10, FALSE);
            Length = (UINT32) Wcslen(VolumeInfo->NTFSName);
            Wcscpy (VolumeInfo->NTFSName + Length, L"GB]");
        }
    }
//
//  Create a FILE_HANDLE_INSTANCE
//
    Status = CreateFileHandleInstance(RootHandle, 
				      &RootInstance, 
				      EFI_FILE_MODE_READ);
//
//  Return FILE_HANDLE_INSTANCE
//
    if (Status == EFI_SUCCESS) *Root = (EFI_FILE_PROTOCOL *)RootInstance;

    VolumeInfo->VolumeStatus = Status;
    return Status;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   OpenRoot
//
// Description: Opens the Root Directory on the Volume.
//
// Parameters:
//  IN VOLUME_INFO        *VolumeInfo - Volume Info Structure
//  IN OUT FILE_HANDLE    **Root - Root directory handle
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:       This function will copy the MFT's run list to the volume 
//              interface structure, and the root index's run list to the 
//              root file handle's structure.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
OpenRoot(
    IN VOLUME_INFO     *VolumeInfo,
    IN OUT FILE_HANDLE **Root
    )
{

    EFI_STATUS        Status;
    FILE_HANDLE       *RootHandle;
    UINT8             *pNameAttribute;
    UINT8             *pRunList;
    UINT32            BytesReadPerLoop;
    UINT8             *Buffer;
    UINT8             *TempBuffer;
    UINT64            Sector;
    UINT64            Cluster;

    Status = fsAllocateMemory(VolumeInfo, sizeof(FILE_HANDLE),
                              (VOID**)&RootHandle, TRUE);

    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;

    Status = fsAllocateMemory(VolumeInfo, 4,
                              (VOID**)&(RootHandle->FileNameLFN), FALSE);

    RootHandle->FileNameLFN[0]= 0;
    RootHandle->ROOTDIR = TRUE;
    RootHandle->VolumeInterface = VolumeInfo;
    RootHandle->DirectoryEntry.DIR_FileFlags = EFI_FILE_DIRECTORY;
    RootHandle->HandleStatus = EFI_SUCCESS;
    RootHandle->ResidentFile = 0; // Assume root is non-resident for now

    RootHandle->MFTReference = 5; // Root directory reference is always 5
    RootHandle->MFTParent = 5; // /Root directory is it's own parent

    BytesReadPerLoop = VolumeInfo->BytesPerCluster;

    // For 512-byte clusters, read 2 at a time
    if (BytesReadPerLoop == 512) BytesReadPerLoop *= 2;

    GetTempBuffer (VolumeInfo, &Buffer);
    Cluster = VolumeInfo->VolumeBPB.BPB_MFT;

//
// Read one cluster of the MFT into the temp buffer.
//
    Sector = ClusterToSector(VolumeInfo, Cluster);
    Status = FsReadMedia (VolumeInfo,
                          Buffer,
                          (UINT64)Sector,
                          0,
                          BytesReadPerLoop,
                          MFT_REGION);
    if (EFI_ERROR(Status)) goto RootError;
    pRunList = NULL;
    pNameAttribute = NULL;
    Status = GetNameAndRunlist(Buffer, &pNameAttribute, &pRunList);
    if ( EFI_ERROR(Status)) goto RootError;

//
// Check for a valid Master File Table (probably not needed)
// (MFT) name must be "$MFT" (in Unicode), and name length must be 4.
//
    if ( (((FR_NAME_ATTRIBUTE *)pNameAttribute)->NA_Name[0] != 0x24) || \
         (((FR_NAME_ATTRIBUTE *)pNameAttribute)->NA_Name[1] != 0x4D) || \
         (((FR_NAME_ATTRIBUTE *)pNameAttribute)->NA_Name[2] != 0x46) || \
         (((FR_NAME_ATTRIBUTE *)pNameAttribute)->NA_Name[3] != 0x54) || \
         (((FR_NAME_ATTRIBUTE *)pNameAttribute)->NA_NameLength != 4) ) 
        Status = EFI_NOT_FOUND;
    if ( EFI_ERROR(Status)) goto RootError;

//
// Copy the Master File Table's run list to the Volume Info structure
//
    pBS->CopyMem( &VolumeInfo->MFTRunList[0], pRunList, MFT_RUN_LIST_SIZE);

//
// Get the root directory's file record. It is always no. 5 in the MFT.
//
    Status = GetFileRecord( VolumeInfo, 5, Buffer, NULL );
    if (EFI_ERROR(Status)) goto RootError;

//
// Build the root handle's directory entry.
// Building it from the name attribute is faster than finding it in the index.
//
    Zeromemory (&RootHandle->DirectoryEntry, sizeof(DIR_ENTRY_NTFS));
    RootHandle->DirectoryEntry.DIR_MFTRecord = 0x0005000000000005;
    RootHandle->DirectoryEntry.DIR_EntrySize = MINIMUM_ENTRY_SIZE;
    RootHandle->DirectoryEntry.DIR_NameOffset = 0x44;
    TempBuffer = NULL;
    Status = GetFrAttribute( Buffer, FR_ATTRIBUTE_NAME, &TempBuffer );
    if (EFI_ERROR(Status)) goto RootError;
    TempBuffer += ((FR_ATTR_HEADER_RES *)TempBuffer)->AHR_InfoOffset;
    pBS->CopyMem( &RootHandle->DirectoryEntry.DIR_MFTParentRec,
                  TempBuffer, sizeof(FR_NAME_ATTRIBUTE));
    CheckFlags(&RootHandle->DirectoryEntry); // Fix the directory flag

//
// Check to see if the root has a resident portion
//
    Status = GetFrAttribute( Buffer, FR_ATTRIBUTE_INDEX_ROOT, &TempBuffer );
    if (!EFI_ERROR(Status))
    {
        if ( ((FR_ATTR_HEADER_RES*)TempBuffer)->AHR_Length > \
               MINIMUM_ENTRY_SIZE )
        {
            RootHandle->ResidentFile = 2; // Mark both resident and non-res.
        }
    }        
//
// Get the Index Allocation Attribute, to find the root directory's run list.
//
    Status = GetFrAttribute( Buffer, FR_ATTRIBUTE_INDEX_ALLOC, &Buffer );
    if (EFI_ERROR(Status)) {
        if (RootHandle->ResidentFile == 2) {
            RootHandle->ResidentFile = 1; // Root dir has only resident portion
        } else {
            goto RootError;
        }
    }
//
// Copy the root directory's run list to the handle structure
//
    if (RootHandle->ResidentFile != 1) {
        Buffer += ((FR_ATTR_HEADER_NONRES*)Buffer)->AHNR_RunOffset;
        pBS->CopyMem( &RootHandle->FileRunList[0], Buffer, FILE_RUN_LIST_SIZE );
    }

    Status = EFI_SUCCESS;

    DListInit(&(RootHandle->DirList));
    *Root = RootHandle;

RootError:
    ReleaseTempBuffer (VolumeInfo);
    return Status;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   CreateFileHandleInstance
//
// Description: Creates an instance of a file handle
//
// Parameters:
//  IN FILE_HANDLE              *fh - Handle of the file to work on
//  IN OUT FILE_HANDLE_INSTANCE **fhi - File handle instance
//  IN UINT64                   OpenMode - Open mode (read, write, both)
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:       In this driver, only Read mode is supported.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
CreateFileHandleInstance (
    IN FILE_HANDLE              *fh,
    IN OUT FILE_HANDLE_INSTANCE **fhi,
    IN UINT64                   OpenMode
    )
{

    EFI_STATUS           Status;
    VOLUME_INFO          *Vi = fh->VolumeInterface;
    FILE_HANDLE_INSTANCE *nfi;
    UINT64               Cluster = 0;
    UINT64               ClusterCount = 0;
    UINT8                *pRunList;

    Status = fsAllocateMemory(Vi, sizeof(FILE_HANDLE_INSTANCE),
                              (VOID**)&nfi, TRUE);

    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;

    if ( (fh->ResidentFile == 0) || (fh->ResidentFile == 2) ) 
    { // Runlist and cluster are only needed for non-resident file.
        pRunList = &fh->FileRunList[0];
        Status = GetRunListElementData(&pRunList, &ClusterCount,
                                       &Cluster, FALSE);
        if (EFI_ERROR(Status)) return Status;
    }
    nfi->Position = 0;
    nfi->HandleInstanceStatus = EFI_SUCCESS;
    nfi->CurrentCluster = Cluster; // cluster of the file
    nfi->CurrentClusterOffset = 0;
    nfi->CurrentRunLength = ClusterCount;
    nfi->pRunList = &fh->FileRunList[0]; // Initialize run list pointer
    nfi->pFH = fh;
    nfi->OpenMode = OpenMode;
    InitEFIFileInterface(&(nfi->FileHandle));
    fh->InstancesCount++;
    DListAdd(&(Vi->OpenFIs), &(nfi->ViFILink));
    *fhi = nfi;
    return EFI_SUCCESS;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   InitEFIFileInterface
//
// Description: Initializes the EFI_FILE_PROTOCOL interface
//
// Parameters:
//  EFI_FILE_PROTOCOL *EFIFileInterface - Interface structure
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals: OpenProtocol CloseProtocol
//
// Notes:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

void
InitEFIFileInterface (
    EFI_FILE_PROTOCOL *EFIFileInterface
    )
{

    EFIFileInterface->Revision = NTFS_DRIVER_VERSION;
    EFIFileInterface->Open = OpenFileHandle;
    EFIFileInterface->Close = CloseFileHandle;
    EFIFileInterface->Delete = DeleteFileHandle;
    EFIFileInterface->Read = ReadFileHandle;
    EFIFileInterface->Write = WriteFileHandle;
    EFIFileInterface->GetPosition = GetPositionFileHandle;
    EFIFileInterface->SetPosition = SetPositionFileHandleThis;
    EFIFileInterface->GetInfo = GetInfoFileHandle;
    EFIFileInterface->SetInfo = SetInfoFileHandle;
    EFIFileInterface->Flush = FlushFileHandle;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   OpenFileHandle
//
// Description: Opens a file handle (when a file is opened)
//
// Parameters:
//  IN EFI_FILE_PROTOCOL  *This - File System Protocol instance
//  OUT EFI_FILE_PROTOCOL **NewHandle - Handle of open file
//  IN CHAR16             *FileName - Unicode name of file
//  IN UINT64             OpenMode - Open mode (read, write, both)
//  IN UINT64             Attributes - Attributes of open file
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
OpenFileHandle (
    IN EFI_FILE_PROTOCOL  *This,
    OUT EFI_FILE_PROTOCOL **NewHandle,
    IN CHAR16             *FileName,
    IN UINT64             OpenMode,
    IN UINT64             Attributes
    )
{

    EFI_STATUS           Status;
    FILE_HANDLE_INSTANCE *Fi= (FILE_HANDLE_INSTANCE *) This;
	FILE_HANDLE				*ParentFh;
	VOLUME_INFO				*Vi;
    EFI_TPL                 SaveTpl;
    EFI_TPL                 NextTpl;

//  Validate Inputs
    switch (OpenMode) {
        case (EFI_FILE_MODE_READ):
        case (EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE):
        case (EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE):
            break;
    default:
        return EFI_INVALID_PARAMETER;
    }

    NextTpl = FindNextTpl (EFI_TPL_CALLBACK);
    SaveTpl = pBS->RaiseTPL (NextTpl);
	Status = ValidateCurrentStatus (Fi);
	if (EFI_ERROR(Status)) 
    {
        pBS->RestoreTPL(SaveTpl);
        return Status;
    }

    ParentFh = Fi->pFH;
    Vi = ParentFh->VolumeInterface;

	if (Vi->ReadOnly && OpenMode & EFI_FILE_MODE_WRITE ) 
    {
        pBS->RestoreTPL(SaveTpl);
        return EFI_WRITE_PROTECTED;
    }

    if (FileName == NULL) 
    {
        pBS->RestoreTPL(SaveTpl);
        return EFI_NOT_FOUND;
    }

    if ( Attributes & ~EFI_FILE_VALID_ATTR) 
    {
        pBS->RestoreTPL(SaveTpl);
        return EFI_NOT_FOUND;
    }

    Status = ProcessOpenFileHandle (Fi, &FileName, NewHandle,
                                    OpenMode, Attributes);
    pBS->RestoreTPL(SaveTpl);    

    return Status;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   ReadFileHandle
//
// Description: Read from a file (normal file or directory)
//
// Parameters:
//  IN EFI_FILE_PROTOCOL *This - File System Protocol instance
//  IN OUT UINTN         *BufferSize - Size of read buffer
//  OUT VOID             *Buffer - Points to read buffer
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
ReadFileHandle (
    IN EFI_FILE_PROTOCOL *This,
    IN OUT UINTN         *BufferSize,
    OUT VOID             *Buffer
    )
{

    EFI_STATUS              Status = EFI_UNSUPPORTED;
    FILE_HANDLE_INSTANCE    *fhi = (FILE_HANDLE_INSTANCE *)This;
    FILE_HANDLE             *Pfh;
    DIR_ENTRY_LIST          *Del;
    EFI_TPL                 SaveTpl;
    EFI_TPL                 NextTpl;

//
// This is a FAT limitation, but since the application invoking this
// driver is expecting FAT, we'll keep it.
//    
    if (*BufferSize > 0xffffffff) return EFI_INVALID_PARAMETER;

    Status = ValidateCurrentStatus (fhi);
    if (EFI_ERROR(Status)) return Status;
    Pfh = fhi->pFH;

    NextTpl = FindNextTpl (EFI_TPL_CALLBACK);
    SaveTpl = pBS->RaiseTPL (NextTpl);
//
// Check if this Instance is a Directory.
//
    if (Pfh->DirectoryEntry.DIR_FileFlags & ATTR_DIRECTORY) {
//
// Check if Dir List has been changed. If so, release the list and 
// read it again.
//
        if (Pfh->DirListChanged == TRUE) {
            RemoveAllDirList (Pfh);
            Pfh->DirListChanged = FALSE;
        }
//
// Check if the file Handle has a valid DIR_ENTRY_LIST
//
        if (!Pfh->DirList.pHead) {
            Status = ReadAllDirectoryEntries (Pfh);
            
            if (EFI_ERROR(Status)) {
                pBS->RestoreTPL(SaveTpl);
                return Status;
            }
        }
//
// Get the directory entry at the file handle instance position.
//
        Status = GetDirListAtPosition (Pfh, fhi->Position, &Del);
        if (EFI_ERROR(Status))
        {
            *BufferSize = 0;    //  End of entries found
            pBS->RestoreTPL(SaveTpl);
            return EFI_SUCCESS;
        }
//
// Fill the EFI_FILE_INFO structure with the entry's information.
//
        Status = GetFileInfoFromDel (Del, BufferSize, Buffer);
        if (EFI_ERROR(Status))
        { // The error will be Buffer Too Small, and the true size is returned.
            pBS->RestoreTPL(SaveTpl);
            return Status;
        } else {
            fhi->Position++;
            pBS->RestoreTPL(SaveTpl);
            return EFI_SUCCESS;
        }

    } else {
//
// It is a File.
// Buffer size is on input is required to be less than 4G.
//
        Status = ReadFromFile (fhi, Buffer, (UINT32*)BufferSize, DATA_REGION);
    }
    pBS->RestoreTPL(SaveTpl);
    return Status;
    
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   WriteFileHandle
//
// Description: Writes to a file
//
// Parameters:
//  IN EFI_FILE_PROTOCOL *This,
//  IN OUT UINTN         *BufferSize,
//  IN VOID              *Buffer
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
WriteFileHandle (
    IN EFI_FILE_PROTOCOL *This,
    IN OUT UINTN         *BufferSize,
    IN VOID              *Buffer
    )
{

    return EFI_WRITE_PROTECTED;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   SetInfoFileHandle
//
// Description: Sets file info
//
// Parameters:
//  IN EFI_FILE_PROTOCOL  *This,
//  IN EFI_GUID           *InformationType,
//  IN UINTN              BufferSize,
//  IN VOID               *Buffer
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
SetInfoFileHandle(
    IN EFI_FILE_PROTOCOL *This,
    IN EFI_GUID          *InformationType,
    IN UINTN             BufferSize,
    IN VOID              *Buffer
    )
{

    return EFI_WRITE_PROTECTED;
}



//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   DeleteFileHandle
//
// Description: Deletes a file
//
// Parameters:
//  IN EFI_FILE_PROTOCOL *This - File Protocol instance
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
DeleteFileHandle (
    IN EFI_FILE_PROTOCOL *This
    )
{

    return EFI_WARN_DELETE_FAILURE;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   FlushFileHandle
//
// Description: Flushes a file (writes any portion left in cache)
//
// Parameters:
//  IN EFI_FILE_PROTOCOL *This - File Protocol instance
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
FlushFileHandle (
    IN EFI_FILE_PROTOCOL *This
    )
{

    return EFI_ACCESS_DENIED;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   ProcessOpenFileHandle
//
// Description: Performs whatever is needed to process an open handle
//              (create dir. entry, create instance, allocate clusters)
//
// Parameters:
//  IN  FILE_HANDLE_INSTANCE  *ParentFi - Parent file handle instance
//  IN  CHAR16                **FileName - Unicode file name
//  OUT EFI_FILE_PROTOCOL     **NewHandle - New file handle
//  IN UINT64                 OpenMode - Open mode (read, write, both)
//  IN UINT64                 Attributes - File attributes
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
ProcessOpenFileHandle(
    IN FILE_HANDLE_INSTANCE *Fi,
    IN CHAR16               **FileName,
    OUT EFI_FILE_PROTOCOL   **NewHandle,
    IN UINT64               OpenMode,
    IN UINT64               Attributes
    )
{

    EFI_STATUS           Status;
    FILE_HANDLE          *fh = Fi->pFH, *fh1;
    VOLUME_INFO          *Vi = fh->VolumeInterface;
    UINT32               Length;
    CHAR16               NextCompName[260];
    FILE_HANDLE_INSTANCE *fhi;
    DIR_ENTRY_LIST       *Del;
    CHAR16               CurrentDir[] = {'.', 0};
    CHAR16               ParentDir[] = {'.', '.', 0};
    UINT8                *Buffer;
    UINT8                *TmpBuf;
    UINT64               Sector;
    BOOLEAN              ShortNameFlag = FALSE;

    if (EFI_ERROR(fh->HandleStatus)) return fh->HandleStatus;

    if ((*FileName)[0] == '\\') {
        fh = Vi->RootFH;
        (*FileName) += 1;
    }
    
//
//  Check for '\\' at the end of the FileName.
// If so remove it. Eg: \windows\system
//
    Length = (UINT32) Wcslen(*FileName);
    
    if (Length > 2 && (*FileName)[Length-1] == '\\') (*FileName)[Length-1] = 0;
    
    Status = EFI_SUCCESS;
    
    while (!EFI_ERROR(Status)) {
    
        if (EFI_ERROR(GetNextCompName (FileName, NextCompName))) return EFI_NOT_FOUND;
        
        if (NextCompName[0] == 0) break;
        
        if (!LfnIsValid (NextCompName)) return EFI_NOT_FOUND;
        
        if (!Wcscmp(NextCompName, CurrentDir)) continue;
        
        if (!Wcscmp(NextCompName, ParentDir)) {
            if (fh == Vi->RootFH) {
                Status = EFI_NOT_FOUND;
                break;
            }
            
            fh = fh->Parent;
            
            if (!fh) {
                Status = EFI_NOT_FOUND;
                break;
            }
            
            continue;
        }
        
//
// A component name is present. Parent should be directory. Else return error
//
        if (!(fh->DirectoryEntry.DIR_FileFlags & ATTR_DIRECTORY)) {
            Status = EFI_NOT_FOUND;
            break;
        }
//
// Check whether a match can be found for nextcompname
//
        if (FindMatchingFH(Vi, NextCompName, fh, &fh1)) {
            fh = fh1;
            Status = EFI_SUCCESS;
            continue;
        }
//
// See if the Linked List is Valid
//
        if (!fh->DirList.pHead) {
            ReadAllDirectoryEntries (fh);
            if (EFI_ERROR(Status)) return Status;
        }

        Status = EFI_NOT_FOUND;
//
// Add to the List in Volume Interface
//        
        if (FindMatchingDirEntry (fh, NextCompName, &Del)) {
            Status = CreateFHFromDirEntryList(Del, fh, &fh1);

            if (Status == EFI_SUCCESS) {
                fh = fh1;
            }
        }

    }

    if (EFI_ERROR(Status)) {
//    
// Make sure NextCompName is the last compname.
//
        if ((*FileName[0]) != 0) return EFI_NOT_FOUND;
//        
// If it is a file that needs to be openend, and not found create one.
// For directories return error.
//        
        if (OpenMode & EFI_FILE_MODE_CREATE) {
            return EFI_UNSUPPORTED; // Except we don't support create
        } else return EFI_NOT_FOUND;
    }
//
// Get the run list of the file if it is not the root directory
//
    if (fh->ROOTDIR == FALSE) {

        GetTempBuffer (Vi, &Buffer);
        Status = GetFileRecord (Vi,
                                fh->DirectoryEntry.DIR_MFTRecord,
                                Buffer, &Sector);
        if (EFI_ERROR(Status))
        {
            ReleaseTempBuffer(Vi);
            return EFI_NOT_FOUND;
        }
//
// If this record is a directory...
// Test for a non-resident directory (Index Allocation present)
//
        if ( ((MFT_FILE_RECORD*)Buffer)->FR_Flags & 0x02) 
        { 
            if ( GetFrAttribute (Buffer, \
                                 FR_ATTRIBUTE_INDEX_ALLOC, \
                                 &TmpBuf) == EFI_SUCCESS)
            { // For a non-resident file, copy the run list.
                TmpBuf += ((FR_ATTR_HEADER_NONRES*)TmpBuf)->AHNR_RunOffset;
                pBS->CopyMem (&fh->FileRunList[0], 
                              &TmpBuf[0], FILE_RUN_LIST_SIZE);
                fh->ResidentFile = 0; // Mark non-resident file
                if (GetFrAttribute (Buffer, \
                                    FR_ATTRIBUTE_INDEX_ROOT, \
                                    &TmpBuf) == EFI_SUCCESS)
                {
                    if ( ((FR_ATTR_HEADER_RES*)TmpBuf)->AHR_Length > \
                           MINIMUM_ENTRY_SIZE )
                    {
                        fh->ResidentFile = 2; // Mark both file
                    }
                }
            } else {
                fh->ResidentFile = 1; // Mark resident file
            }
//
// This record is a file.
//
        } else { 
            Status = GetFrAttribute (Buffer, FR_ATTRIBUTE_DATA, &Buffer);
            if (EFI_ERROR(Status)) 
            { // If it doesn't have a data attribute, it could be a meta-file
                ReleaseTempBuffer(Vi);
                return EFI_ACCESS_DENIED;
            }
//
// If this is a non-resident file, copy the run list into the handle structure.
//
            if ( ((FR_ATTR_HEADER_RES*)Buffer)->AHR_NonResFlag )
            {
                Buffer += ((FR_ATTR_HEADER_NONRES*)Buffer)->AHNR_RunOffset;
                pBS->CopyMem (&fh->FileRunList[0],
                              &Buffer[0], FILE_RUN_LIST_SIZE);
                fh->ResidentFile = 0; // Mark non-resident file
            } else {
                fh->ResidentFile = 1; // Mark resident file
			}
        }
        ReleaseTempBuffer(Vi);
    }

    Status = CreateFileHandleInstance (fh, &fhi, OpenMode);
    *NewHandle = (EFI_FILE_PROTOCOL *) fhi;

    return Status;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   ReadAllDirectoryEntries
//
// Description:
//
// Input:
//  IN FILE_HANDLE    *fh,
//
// Output:
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
ReadAllDirectoryEntries (
    IN FILE_HANDLE *fh
    )
{

    EFI_STATUS  Status;
    CHAR16      *lfnBuffer;
    UINT16      LfnSize;
    UINT16      EntrySize;
    INT32       IndexSize;
    UINT32      BytesPerLoop;
    UINT8       *Buffer;
    UINT8       *BufferPtr;
    UINT8       *TmpPtr;
    CHAR16      *NamePtr;
    UINT16      Temp16;
    UINT32      Temp32;
    UINTN       Offset;
    VOLUME_INFO *Vi = fh->VolumeInterface;
    UINT32      EntryNumber = 0;
    BOOLEAN     FirstReadFlag = TRUE;
    UINT8       *TmpBuf;
    UINT32      TmpSize;
    UINT8       *pRunList;
    UINT64      ClusterCount;
    UINT64      Cluster;
    UINT64      TmpCluster;

    if (Vi->TotalDirEntryList > DIRECTORTY_ENTRIES_CACHED)
        FreeUpDirEntries (Vi, NULL); // Free up entries if cache space used up.

    BytesPerLoop = Vi->BytesPerCluster; // Get bytes to read in a loop.

    GetTempBuffer (Vi, &Buffer);
    BufferPtr = Buffer; // remember where buffer is

    if ( (fh->ResidentFile == 0) && fh->FileRunList[0] == 0 )
    {
//
// If the resident flag is FALSE, and the run list is 0, it means that this
// directory does not have the residency or run list info in it's handle
// structure. Get it now.
//
        Status = GetFileRecord (Vi,
                                fh->DirectoryEntry.DIR_MFTRecord,
                                Buffer, NULL);
        if (EFI_ERROR(Status))
        {
            ReleaseTempBuffer(Vi);
            return EFI_NOT_FOUND;
        }

        if ( GetFrAttribute (Buffer, \
                             FR_ATTRIBUTE_INDEX_ALLOC, \
                             &TmpBuf) == EFI_SUCCESS)
        { // For a non-resident file, copy the run list.
            TmpBuf += ((FR_ATTR_HEADER_NONRES*)TmpBuf)->AHNR_RunOffset;
            pBS->CopyMem (&fh->FileRunList[0], 
                          &TmpBuf[0], FILE_RUN_LIST_SIZE);
            fh->ResidentFile = 0; // Mark non-resident file
            if (GetFrAttribute (Buffer, \
                                FR_ATTRIBUTE_INDEX_ROOT, \
                                &TmpBuf) == EFI_SUCCESS)
            {
                if ( ((FR_ATTR_HEADER_RES*)TmpBuf)->AHR_Length > \
                       MINIMUM_ENTRY_SIZE )
                {
                    fh->ResidentFile = 2; // Mark both file
                }
            }
        } else {
            fh->ResidentFile = 1; // Mark resident file
        }

    }

    pRunList = &fh->FileRunList[0]; // Get the run list for this file handle,

    ClusterCount = 0;
    Cluster = 0;
    TmpCluster = 0;

    do {
        Buffer = BufferPtr; // reset buffer if  necessary
        TmpPtr = Buffer;
        Offset = 0;
//
// A directory can be resident, non-resident, or both. If it is non-resident,
// Read the directory file. If both, get the resident part first.
//
        if ( ((fh->ResidentFile == 1) || (fh->ResidentFile == 2)) && \
             FirstReadFlag )
        {
//
// Small directories don't have Index records, so just get the file
// record for it. Combination directories can have both, so get the
// file record part first.
//
            if (Vi->BytesPerCluster < 1024) BytesPerLoop = FILE_RECORD_SIZE;
            Status = GetFileRecord (Vi,
                                    fh->DirectoryEntry.DIR_MFTRecord,
                                    Buffer, NULL);
            if (EFI_ERROR(Status))
            {
                ReleaseTempBuffer(Vi);
                return EFI_NOT_FOUND;
            }
        } else {
//
// For large directories, read from the Index record.
//
            UINT32      ClustersToRead;
            UINT32      ClusterSize;

            BytesPerLoop = INDEX_RECORD_SIZE;
            TmpSize = INDEX_RECORD_SIZE;
            ClusterSize = Vi->BytesPerCluster;
            ClustersToRead = TmpSize / ClusterSize;

            if (ClustersToRead == 0) {
                ClustersToRead = 1;
                ClusterSize = TmpSize;
            }
            TmpPtr = Buffer;
            
            do {
                UINT64      Sector;
                UINT32      ReadCount;
                
                if (ClusterCount == 0) {
                    Status = GetRunListElementData (&pRunList,
                                                    &ClusterCount, &Cluster, TRUE);
                    if (EFI_ERROR(Status)) {
                        ReleaseTempBuffer (Vi);
                        return EFI_SUCCESS; // End of directory reached.
                    }
                    TmpCluster = Cluster;
                }

                if (ClusterCount > ClustersToRead) {
                    ReadCount = ClustersToRead;
                } else {
                    ReadCount = (UINT32)ClusterCount;
                }
                
                Sector = ClusterToSector(Vi, TmpCluster);
                Status = FsReadMedia (Vi,
                                      TmpPtr,
                                      Sector,
                                      0,
                                      ClusterSize * ReadCount,
                                      DIRECTORY_REGION);
                if (EFI_ERROR(Status)) {
                    ReleaseTempBuffer (Vi);
                    return Status;
                }
                TmpPtr += (ClusterSize * ReadCount);
                ClustersToRead -= ReadCount;
                ClusterCount -= ReadCount;
                TmpCluster += ReadCount;
            } while (ClustersToRead > 0);
            TmpPtr = Buffer; // Reset pointer
        }
//
// Check if fixup is needed, and apply it if so.
// This will error out if we don't have a proper record.
// The only way it will error out is if we  have read past the end
// of the directory, so just return with success if it does.
//
        Status = ApplyFixup (Vi, Buffer);
        if (EFI_ERROR(Status)) {
            ReleaseTempBuffer(Vi);
            return EFI_SUCCESS; // End of directory reached.
        }

//
// If we loaded a "FILE" record (small directory)...
// The directory will be in the Index Root attribute.
//            
        if ( *((UINT32*)Buffer) == EFI_SIGN_32 ('F', 'I', 'L', 'E') )
        {
            GetFrAttribute( Buffer, FR_ATTRIBUTE_INDEX_ROOT, &TmpPtr );
            Temp16 = ((FR_ATTR_HEADER_RES*)TmpPtr)->AHR_InfoOffset;
            TmpPtr += Temp16;
            IndexSize = ((FR_INDEX_ROOT_ATTRIBUTE*)TmpPtr)->IRA_TotalSize;
            Temp32 = ((FR_INDEX_ROOT_ATTRIBUTE*)TmpPtr)->IRA_Offset;
            TmpPtr += Temp32 + EFI_FIELD_OFFSET(FR_INDEX_ROOT_ATTRIBUTE, IRA_Offset);
            Offset = TmpPtr - Buffer;
            if (IndexSize < MINIMUM_ENTRY_SIZE) { // Empty index
                ReleaseTempBuffer (Vi);
                return EFI_SUCCESS;
            }
        }
//
// If an "INDX" record (large directory)...
//                
        if ( *((UINT32*)Buffer) == EFI_SIGN_32 ('I', 'N', 'D', 'X') )
        {
            IndexSize = ((INDEX_RECORD*)Buffer)->INDR_IndxEntrySize;
            Offset += ((INDEX_RECORD*)Buffer)->INDR_IndxEntryOff + \
                        EFI_FIELD_OFFSET(INDEX_RECORD, INDR_IndxEntryOff);
            TmpPtr = Buffer;
            TmpPtr += Offset;
            if (IndexSize < MINIMUM_ENTRY_SIZE) { // Empty index
                ReleaseTempBuffer (Vi);
                return EFI_SUCCESS;
            }
        }

        while ( Offset < BytesPerLoop ) {

            EntrySize = ((INDEX_ENTRY*)TmpPtr)->INDE_EntrySize;

            LfnSize = ((INDEX_ENTRY*)TmpPtr)->INDE_NameLength * 2;

            Status = fsAllocateMemory (Vi, (UINT32)(LfnSize + 2),
                                           (VOID**)&lfnBuffer, FALSE);
            if (EFI_ERROR(Status)) {
                ReleaseTempBuffer (Vi);
                return Status;
            }
//
// Copy the file's name to the buffer reserved for it.
//
            NamePtr = &((INDEX_ENTRY*)TmpPtr)->INDE_Name[0];
            pBS->CopyMem (lfnBuffer, NamePtr, LfnSize);
            lfnBuffer[LfnSize / 2] = 0; // Zero-terminate name

//###DEBUG For now, we're not going to support duplicate short names.
// Files copied from a FAT medium with a long name will have a
// duplicate entry for the short name. Hide that entry.
//
//####          if ( ((INDEX_ENTRY*)TmpPtr)->INDE_NameType == 2 )
//####          {
//####              ((INDEX_ENTRY*)TmpPtr)->INDE_FileFlags |= ATTR_HIDDEN;
//####          }
//###DEBUG END

// Set all meta-file flags to hidden and system. Meta-files have a record
// number less than 16.
//
            if ( (((INDEX_ENTRY*)TmpPtr)->INDE_MFTRecord & \
                    MAXIMUM_RECORD_NUMBER) < 16 )
            {
                ((INDEX_ENTRY*)TmpPtr)->INDE_FileFlags |= \
                 (ATTR_HIDDEN | ATTR_SYSTEM);
            }
//
// Check the file's flags and only add it to the list if it's one we support.
//
            if (CheckFlags ((DIR_ENTRY_NTFS*)TmpPtr) == EFI_SUCCESS)
            {

//
// Mask the file size fields to the Windows implementation limit
//
                ((DIR_ENTRY_NTFS*)TmpPtr)->DIR_AllocSize &= 0xFFFFFFFFFFF;
                ((DIR_ENTRY_NTFS*)TmpPtr)->DIR_RealSize &= 0xFFFFFFFFFFF;
//
// Create a Dir Entry List and add it to the Parent File Handle
//
                Status = AddToDirList (fh, *(DIR_ENTRY_NTFS *)TmpPtr,
                                       lfnBuffer, EntryNumber );
                if (EFI_ERROR(Status)) {
                    ReleaseTempBuffer (Vi);
                    return Status;
                }
                EntryNumber++; // Count this entry
            }

            Offset += EntrySize;
            TmpPtr += EntrySize;
            IndexSize -= EntrySize;
//
// Test for the end of the current index or index part.
// This test makes 3 checks:
// 1. If the parent record no. is not 5 (root dir), it check for a
//    record no. of 0.
// 2. It checks to see if the next possible entry size is 0.
// 3. It checks to see if the remaining index is too small for an entry.
//
            if ( ( ((((INDEX_ENTRY*)TmpPtr)->INDE_MFTParentRec & \
                      MAXIMUM_RECORD_NUMBER) != 5) && \
                 ((((INDEX_ENTRY*)TmpPtr)->INDE_MFTRecord & \
                      MAXIMUM_RECORD_NUMBER) == 0) ) || \
                 (IndexSize < MINIMUM_ENTRY_SIZE) || \
                 (((INDEX_ENTRY*)TmpPtr)->INDE_EntrySize == 0) )
            {
                if (fh->ResidentFile == 1)
                {
                    ReleaseTempBuffer (Vi);
                    return EFI_SUCCESS;
                } else {
                    FirstReadFlag = FALSE;
                    break;
                }
            } // End of EOF check
        } // End of while loop
        FirstReadFlag = FALSE;
    } while (IndexSize > 0);

    ReleaseTempBuffer (Vi);

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   AddToDirList
//
// Description: Adds a directory entry to DirList
//
// Parameters:
//  FILE_HANDLE    *fh - File Handle
//  DIR_ENTRY_NTFS DirectoryEntry - Entry to be added to list
//  CHAR16         *lfn - Pointer to file name (Unicode)
//  UINT32         Number - Sequence number of the entry
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals: ReadAllDirectoryEntries
//
// Notes:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AddToDirList (
    FILE_HANDLE    *fh,
    DIR_ENTRY_NTFS DirectoryEntry,
    CHAR16         *lfn,
    UINT32         Number
    )
{
    DIR_ENTRY_LIST *Del;
    EFI_STATUS     Status;

    Status = fsAllocateMemory(fh->VolumeInterface,
                              sizeof(DIR_ENTRY_LIST),
                              (VOID**)&Del, FALSE);

    if (EFI_ERROR(Status)) return Status;

    pBS->CopyMem (&(Del->DirectoryEntry),
                  &DirectoryEntry, sizeof(DIR_ENTRY_NTFS));
    Del->FileNameLFN = lfn;
    Del->Number = Number;
    DListAdd(&(fh->DirList), &(Del->DirLink));
    fh->VolumeInterface->TotalDirEntryList++;
    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   CreateFHFromDirEntryList
//
// Description: Creates a file handle from directory entry list
//
// Parameters:
//  IN DIR_ENTRY_LIST *Del - Directory entry list
//  IN FILE_HANDLE    *fh - Directory file handle
//  OUT FILE_HANDLE   **fh1 - New file handle
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals: ReadFileHandle, ProcessOpenFileHandle
//
// Notes: Should not link the FH to any LIST
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
CreateFHFromDirEntryList(
    IN DIR_ENTRY_LIST *Del,
    IN FILE_HANDLE    *fh,
    OUT FILE_HANDLE   **fh1
    )
{

    FILE_HANDLE *nfh;
    EFI_STATUS  Status;
//
// Allocate memory needed for handle and file name
//    
    Status = fsAllocateMemory(fh->VolumeInterface,
                              sizeof(FILE_HANDLE),
                              (VOID**)&nfh, TRUE);
    if (EFI_ERROR(Status)) return Status;

    Status = fsAllocateMemory(fh->VolumeInterface,
                              (UINT32)((Wcslen(Del->FileNameLFN) *\
                                  sizeof(CHAR16)) + 2),
                              (VOID**)&(nfh->FileNameLFN), FALSE);
    if (EFI_ERROR(Status)) return Status;
//
// Initialize FILE_HANDLE_STRUCTURE
//    
    nfh->VolumeInterface = fh->VolumeInterface;
    nfh->HandleStatus = EFI_SUCCESS;
    nfh->ROOTDIR = FALSE;
    pBS->CopyMem (&(nfh->DirectoryEntry),
                  &(Del->DirectoryEntry),
                  sizeof(DIR_ENTRY_NTFS));
    Wcscpy(nfh->FileNameLFN, Del->FileNameLFN);
    nfh->Parent = fh;
    *fh1 = nfh;
//
// Add the new FH to the parent's Child List
//    
    DListAdd(&(fh->ChildList), &(nfh->ChildLink));
//
// Add to VolumeInterface List
//    
    DListAdd(&(fh->VolumeInterface->OpenFHs), &(nfh->ViFHLink));

    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   FindMatchingFH
//
// Description: Finds the Child FH, whose name matches with the input.
//              In the Attributes, check whther both are file or Directory
//
// Parameters:
//  IN VOLUME_INFO  *Vi - Volume info structure
//  IN CHAR16       *NextCompName - Unicode name to compare
//  IN FILE_HANDLE  *fh - Parent file handle
//  OUT FILE_HANDLE **fh1 - Child file handle
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals: ProcessOpenFileHandle
//
// Notes:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN
FindMatchingFH (
    IN VOLUME_INFO  *Vi,
    IN CHAR16       *NextCompName,
    IN FILE_HANDLE  *fh,
    OUT FILE_HANDLE **fh1
    )
{

    DLINK       *fhlink = fh->ChildList.pHead;
    FILE_HANDLE *fh2;

    for ( ; fhlink; ) {
        fh2 = OUTTER(fhlink, ChildLink, FILE_HANDLE);

        if (!Wcscmpcaseinsensitive (fh2->FileNameLFN, NextCompName)) {
            if (fh2->HandleStatus == EFI_SUCCESS) {
                *fh1 = fh2;
                return TRUE;
            }
        }

        fhlink = fhlink->pNext;
    }

    return FALSE;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   FindMatchingDirEntry
//
// Description: Finds a directory entry matching the input name
//
// Parameters:
//  IN FILE_HANDLE     *fh - Directory file handle
//  IN CHAR16          *NextCompName - Unicode name to match
//  OUT DIR_ENTRY_LIST **Del - Directory list where found
//
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals: ProcessOpenFileHandle
//
// Notes:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN
FindMatchingDirEntry (
    IN FILE_HANDLE     *fh,
    IN CHAR16          *NextCompName,
    OUT DIR_ENTRY_LIST **Del
    )
{

    DLINK          *dlink = fh->DirList.pHead;
    DIR_ENTRY_LIST *del;

    for ( ; dlink; ) {
        del = OUTTER(dlink, DirLink, DIR_ENTRY_LIST);

        if (!Wcscmpcaseinsensitive (del->FileNameLFN, NextCompName)) {
            *Del = del;
            return TRUE;
        }

        dlink = dlink->pNext;
    }

    return FALSE;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   Wcscmpcaseinsensitive
//
// Description: Case insensitive unicode string compare
//
// Parameters:
//  CHAR16 *string1 - First string
//  CHAR16 *string2 - Second string
//
// Return Value:
//  INT - Difference where mis-matched
//
// Modified:
//
// Referrals:   FindMatchingFH, FindMatchingDirEntry
//
// Notes:       Returns 0 if strings match.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

int Wcscmpcaseinsensitive(
    CHAR16 *string1,
    CHAR16 *string2
    )
{

    CHAR16 ch1, ch2;

    while (*string1) {
        if (*string1 >=0x61 && *string1<=0x7a) ch1 = *string1 - 0x20;

        else ch1 = *string1;

        if (*string2 >=0x61 && *string2<=0x7a) ch2 = *string2 - 0x20;

        else ch2 = *string2;

        if (ch1 != ch2) break;

        *string1++;
        *string2++;
    }

    return *string1 - *string2;     // Don't use ch1 - ch2
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetDirListAtPosition
//
// Description: Get directory list at current position in directory file
//
// Parameters:
//  FILE_HANDLE       *Pfh - Parent file handle
//  UINT64            Position - Current position
//  DIR_ENTRY_LIST    **Del - Directory entry list
//
// Return Value:
//
// Modified:
//
// Referrals: ReadFileHandle
//
// Notes:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
GetDirListAtPosition(
    FILE_HANDLE    *Pfh,
    UINT64         Position,
    DIR_ENTRY_LIST **Del
    )
{

    EFI_STATUS     Status = EFI_NOT_FOUND;
    DLINK          *dlink = Pfh->DirList.pHead;
    DIR_ENTRY_LIST *LocalDel = NULL;

    if (!dlink) return Status;

    while (dlink) {

        LocalDel = OUTTER(dlink, DirLink, DIR_ENTRY_LIST);
        if ( (LocalDel->Number == (UINT32)Position) && dlink )
        {
            *Del = LocalDel;
            return EFI_SUCCESS;
        }

        dlink = dlink->pNext;

    };

    return Status;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   CloseFileHandle
//
// Description: Closes an open file
//
// Parameters:
//  IN EFI_FILE_PROTOCOL *This - File Protocol instance
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals:   DeleteFileHandle
//
// Notes:       Some of this coding may not be necessary for a
//              read-only driver.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
CloseFileHandle (
    IN EFI_FILE_PROTOCOL *This
    )
{

    EFI_STATUS           Status;
    FILE_HANDLE_INSTANCE *fhi = (FILE_HANDLE_INSTANCE *)This;
    FILE_HANDLE          *Pfh = fhi->pFH;
    VOLUME_INFO          *vi;

//
// Device Removal or Media Not Present
//    
    if (!Pfh) {
        Status = pBS->FreePool(fhi);
        return EFI_SUCCESS;
    }
    vi = Pfh->VolumeInterface;
//###DEBUG This not needed for read-only
//    if (Pfh->DirEntryChanged == TRUE) {
//        UpdateDirListFromFHDir(Pfh);
//        Pfh->DirEntryChanged = FALSE;
//    }
//###DEBUG End
    DListDelete (&(vi->OpenFIs), &(fhi->ViFILink));
    fsDeAllocateMemory (vi, fhi);

    if (vi->TotalDirEntryList > DIRECTORTY_ENTRIES_CACHED) {
        if (vi->OpenFHs.Size > MAX_OPEN_FHS) FreeUpFHEntries (vi, NULL);

        FreeUpDirEntries (vi, Pfh->Parent);
    }
//
// This should be always after FreeUpDirEntries. When called from
// DeleteFileHandle will have problem otherwise.
//
    if (Pfh) Pfh->InstancesCount -= 1;

    FlushFileHandle(This);

    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   FreeUpFHEntries
//
// Description: Release all file handles whose instance count is 0
//
// Parameters:
//  VOLUME_INFO       *vi - Volume info
//  FILE_HANDLE       *pfh - Parent file handle
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals: CloseFileHandle
//
// Notes:
//  Don't remove pfh
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
FreeUpFHEntries (
    VOLUME_INFO *vi,
    FILE_HANDLE *pfh
    )
{

    DLINK       *dlink = vi->OpenFHs.pHead;
    FILE_HANDLE *lfh;
//
// Go through all the FHs in Volume Interface and release all whose Instance
// count is 0, no DIR list is present and childlist is NULL
// Don't delete:
// 1. If Instance count is more than 0
// 2. If Dir List is present,
// 3. If it is a Root Dir,
// 4. If Child List is present,
// 5. If it is a i/p fh
// 6. If the i/p FH is the parent of the fh.
//    
    for ( ; dlink; ) {
        lfh = OUTTER(dlink, ViFHLink, FILE_HANDLE);
        dlink = dlink->pNext;

        if (lfh->InstancesCount == 0 && !(lfh->DirList.pHead) && \
            !(lfh->ROOTDIR) && !(lfh->ChildList.pHead) && lfh != pfh) 
            RemoveFH(lfh);
    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   FreeUpDirEntries
//
// Description: Free up cached directory entries
//
// Parameters:
//  VOLUME_INFO       *vi - Volume info
//  FILE_HANDLE       *pfh - Parent file handle
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals: ReadAllDirectoryEntries, CloseFileHandle
//
// Notes:
//  Don't remove pfh
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
FreeUpDirEntries (
    VOLUME_INFO *vi,
    FILE_HANDLE *pfh
    )
{

    DLINK       *dlink = vi->OpenFHs.pHead;
    FILE_HANDLE *lfh;

    for ( ; dlink; ) {
        if (vi->TotalDirEntryList <= DIRECTORTY_ENTRIES_CACHED) break;

        lfh = OUTTER(dlink, ViFHLink, FILE_HANDLE);

        if (lfh != pfh && lfh->DirList.pHead) RemoveAllDirList(lfh);

        dlink = dlink->pNext;
    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   RemoveAllDirList
//
// Description: Removes all directory lists from cache (needs to be re-read)
//
// Parameters:
//  FILE_HANDLE *lfh - list file handle
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals: ReadFileHandle, DeleteFileHandle, FreeUpDirEntries
//
// Notes:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
RemoveAllDirList(
    FILE_HANDLE *lfh
    )
{

    DLINK          *dlink = lfh->DirList.pHead;
    DIR_ENTRY_LIST *Del;

    for ( ; dlink; ) {
        Del = OUTTER(dlink, DirLink, DIR_ENTRY_LIST);
        dlink = dlink->pNext;
        RemoveDirList (lfh, Del);
    }

    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   RemoveDirList
//
// Description: Removes a directory list from cache
//
// Parameters:
//  FILE_HANDLE    *fh - Directory file handle
//  DIR_ENTRY_LIST *Del - Directory list
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals: DeleteFileHandle, RemoveAllDirList
//
// Notes:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
RemoveDirList (
    FILE_HANDLE    *fh,
    DIR_ENTRY_LIST *Del
    )
{

    fh->VolumeInterface->TotalDirEntryList--;

    fsDeAllocateMemory (fh->VolumeInterface, Del->FileNameLFN);
    DListDelete (&(fh->DirList), &(Del->DirLink));
    fsDeAllocateMemory (fh->VolumeInterface, Del);
    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetTempBuffer
//
// Description: Gets a temporary buffer
//
// Parameters:
//    VOLUME_INFO     *Vi - Volume info
//    VOID            **Buffer - Buffer
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals: ProcessOpenFileHandle, ReadAllDirectoryEntries, CloseFileHandle
//
// Notes:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
GetTempBuffer (
    VOLUME_INFO *Vi,
    VOID        **Buffer
    )
{

    *Buffer = (UINT8 *)(Vi->TempBuffer);
    Vi->TempBufferInUse = TRUE;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   ReleaseTempBuffer
//
// Description: Releases a temporary buffer
//
// Parameters:
//  VOLUME_INFO     *Vi - Volume Info
//
// Return Value:
//  EFI_STATUS
//
// Modified:
//
// Referrals: ProcessOpenFileHandle, ReadAllDirectoryEntries, CloseFileHandle
//
// Notes:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
ReleaseTempBuffer (
    VOLUME_INFO *Vi
    )
{
    Vi->TempBufferInUse = FALSE;
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
