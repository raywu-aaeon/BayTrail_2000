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
// Name: Info.c
//
// Description:  NTFS driver information handling functions
//
//<AMI_FHDR_END>
//**********************************************************************

//----------------------------------------------------------------------

#include "NTFS.h"

//----------------------------------------------------------------------

extern EFI_GUID gEfiFileSystemVolumeLabelGuid;
extern EFI_GUID gEfiFileInfoGuid;
extern EFI_GUID gEfiFileSystemInfoGuid;

//----------------------------------------------------------------------

//
// Tables of days per month, in normal years, and leap years.
//
UINT8   DPM[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0 };
UINT8   DPML[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31, 0 };
//
// Define leap year
//
#define LEAPYEAR (Year % 400 == 0) || ((Year % 4 == 0) && (Year % 100 != 0))

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       GetInfoFileHandle
//
// Description:     Returns file info, system info, or the label of a volume
//                  depending on information type specified
//
// Parameters:      EFI_FILE_PROTOCOL *This - Pointer to this instance of
//                                            file protocol
//                  EFI_GUID *InformationType - Pointer to information type
//                                              requested
//                  UINTN *BufferSize - Pointer to size of buffer
//                  VOID *Buffer - Pointer to buffer for returned info
//
// Return value:    EFI_STATUS - Status of the operation
//
// Modified:
//
// Referral(s):
//
// NOTE(S):
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
GetInfoFileHandle(
    IN EFI_FILE_PROTOCOL *This,
    IN EFI_GUID          *InformationType,
    IN OUT UINTN         *BufferSize,
    OUT VOID             *Buffer
    )
{

    EFI_STATUS           Status = EFI_UNSUPPORTED;
    FILE_HANDLE_INSTANCE *fhi = (FILE_HANDLE_INSTANCE *)This;
    FILE_HANDLE          *Pfh = fhi->pFH;
    VOLUME_INFO          *vi = Pfh->VolumeInterface;
    EFI_TPL              SaveTpl;
    EFI_TPL              NextTpl;


    Status = ValidateCurrentStatus (fhi);

    if (EFI_ERROR(Status)) return Status;
//
//  Compare the GUID, to see which info is needed.
//    
    NextTpl = FindNextTpl (EFI_TPL_CALLBACK);
    SaveTpl = pBS->RaiseTPL (NextTpl);
    if (!guidcmp(InformationType, &gEfiFileInfoGuid))
        Status = GetfileInfo (fhi, BufferSize, Buffer);

    else  {
        if (!guidcmp(InformationType, &gEfiFileSystemInfoGuid))
            Status = GetSystemInfo (vi, BufferSize,
                                    (EFI_FILE_SYSTEM_INFO *)Buffer);

        else {
            if (!guidcmp(InformationType, &gEfiFileSystemVolumeLabelGuid))
                Status = GetVolumeLabel (vi, BufferSize, Buffer);

            else {
                pBS->RestoreTPL(SaveTpl);
                return EFI_UNSUPPORTED;
            }
        }
    }

    pBS->RestoreTPL(SaveTpl);    
    return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       GetfileInfo
//
// Description:     Returns information on a file
//
// Parameters:      FILE_HANDLE_INSTANCE *fh1 - Pointer to file handle
//                                              instance
//                  UINTN *BufferSize - Pointer to size of buffer
//                  VOID *Buffer - Ponter to buffer for returned data
//
// Return value:    EFI_STATUS - Status of the operation
//
// Modified:
//
// Referral(s):
//
// NOTE(S):
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
GetfileInfo (
    IN FILE_HANDLE_INSTANCE *fhi,
    IN OUT UINTN            *BufferSize,
    OUT VOID                *Buffer
    )
{

    FILE_HANDLE *Pfh = fhi->pFH;
    return (GetFileInfoFromFH (Pfh, BufferSize, Buffer));

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       GetSystemInfo
//
// Description:     Returns file system info on a volume
//
// Parameters:      VOLUME_INFO *vi - Pointer to volume info structure
//                  UINTN *BufferSize - Pointer to size of buffer
//                  VOID *Buffer - Ponter to buffer for returned data
//
// Return value:    EFI_STATUS - Status of the operation
//
// Modified:
//
// Referral(s):
//
// NOTE(S):
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
GetSystemInfo (
    IN VOLUME_INFO           *vi,
    IN OUT UINTN             *BufferSize,
    OUT EFI_FILE_SYSTEM_INFO *Buffer
    )
{

    UINTN Length = 0;

    GetVolumeLabel (vi, &Length, Buffer);

    if (*BufferSize < Length + \
        EFI_FIELD_OFFSET(EFI_FILE_SYSTEM_INFO, VolumeLabel)) {

        *BufferSize = Length + \
            EFI_FIELD_OFFSET(EFI_FILE_SYSTEM_INFO, VolumeLabel);
        return EFI_BUFFER_TOO_SMALL;

    }

    Buffer->Size = Length + \
        EFI_FIELD_OFFSET(EFI_FILE_SYSTEM_INFO, VolumeLabel);
    Buffer->ReadOnly = vi->ReadOnly;
    Buffer->VolumeSize = vi->VolumeSize;
    Buffer->FreeSpace = 0; // We can't do free space
    Buffer->BlockSize = vi->BytesPerCluster;
    GetVolumeLabel (vi, BufferSize, ((UINT8 *)Buffer) + \
                    EFI_FIELD_OFFSET(EFI_FILE_SYSTEM_INFO,
                    VolumeLabel));
    *BufferSize +=  EFI_FIELD_OFFSET(EFI_FILE_SYSTEM_INFO, VolumeLabel);

    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       GetVolumeLabel
//
// Description:     Returns label of a volume
//
// Parameters:      VOLUME_INFO *vi - Pointer to volume info structure
//                  UINTN *BufferSize - Pointer to size of buffer
//                  VOID *Buffer - Ponter to buffer for returned data
//
// Return value:    EFI_STATUS - Status of the operation
//
// Modified:
//
// Referral(s):
//
// NOTE(S):
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
GetVolumeLabel (
    IN VOLUME_INFO *vi,
    IN OUT UINTN   *BufferSize,
    OUT VOID       *Buffer
    )
{

    EFI_STATUS  Status;
    EFI_STATUS  Status2;
    UINT8       *TempBuffer;
    UINTN       Length;
    FILE_HANDLE *fh = vi->RootFH;

//
// If the volume ID has already been read, it will be in the
// Volume Info structure.
//    
    if (vi->ValidVolumeID) {
        Length = Wcslen (vi->VolumeID) << 1;

        if (*BufferSize < (Length + 2)) {
            *BufferSize = Length + 2;
            return EFI_BUFFER_TOO_SMALL;
        }

        Wcscpy (Buffer, vi->VolumeID);
        *BufferSize = Length + 2;
        return EFI_SUCCESS;

    } else {
        if (*BufferSize < 2) {
            Status = EFI_BUFFER_TOO_SMALL; // To pass SCT test.

        } else Status = EFI_NOT_FOUND;
    }
//
// The volume label hasn't been read yet. Since FAT expects the
// root directory to be read in order to get the label, read it now
// for compatibility.
//
    if (!fh->DirList.pHead) ReadAllDirectoryEntries (fh);
    
    GetTempBuffer (vi, &TempBuffer);
//
// The volume name is record no. 3 in the MFT.
//    
    Status2 = GetFileRecord (vi, 3, TempBuffer, NULL);
    if (EFI_ERROR(Status2)) 
    {
        ReleaseTempBuffer (vi);
        return Status;
    }
//
// Find the Volume Name Attribute, then get the name length and
// point to the name.
//    
    Status2 = GetFrAttribute ( TempBuffer, FR_ATTRIBUTE_VOLUME_NAME, &TempBuffer );
    if (Status2 == EFI_SUCCESS)
    {
        Length = (UINTN)((FR_ATTR_HEADER_RES*)TempBuffer)->AHR_InfoLength;
        TempBuffer += ((FR_ATTR_HEADER_RES*)TempBuffer)->AHR_InfoOffset;
//
// Copy the name to the Volume Info structure
//    
        Zeromemory (vi->VolumeID, sizeof (vi->VolumeID));
        pBS->CopyMem (&vi->VolumeID[0], TempBuffer, Length);
        vi->ValidVolumeID = TRUE;

        if (*BufferSize < (Length + 2)) {
            *BufferSize = Length + 2;
            ReleaseTempBuffer (vi);
            return EFI_BUFFER_TOO_SMALL;
        }
        
        Status = EFI_SUCCESS;
        Wcscpy (Buffer, vi->VolumeID);
        *BufferSize = Length + 2;
    }
//    
// If there is no volume label, we must return a null label of size 2.
//
    else if (Status2 == EFI_NOT_FOUND) 
    {
        if (*BufferSize >= 2) Zeromemory (Buffer, 2);
        
        *BufferSize = 2;
        Status = EFI_SUCCESS;
        
    } else if (Status == EFI_BUFFER_TOO_SMALL) *BufferSize = 2;
    
    ReleaseTempBuffer (vi);
    return Status;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       GetFileInfoFromFH
//
// Description:     Returns information on a file from it's handle
//
// Parameters:      FILE_HANDLE_INSTANCE *fh1 - Pointer to file handle
//                                              instance
//                  UINTN *BufferSize - Pointer to size of buffer
//                  VOID *Buffer - Ponter to buffer for returned data
//
// Return value:    EFI_STATUS - Status of the operation
//
// Modified:
//
// Referral(s):
//
// NOTE(S):
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
GetFileInfoFromFH (
    FILE_HANDLE  *Pfh,
    IN OUT UINTN *BufferSize,
    OUT VOID     *Buffer
    )
{
    EFI_FILE_INFO *buffer = Buffer;
    UINTN         Length;

    if (Pfh->ROOTDIR && !Pfh->DirList.pHead) ReadAllDirectoryEntries (Pfh);

    Length = (UINT32)Wcslen(Pfh->FileNameLFN) * sizeof (CHAR16) + 2;
    Length += EFI_FIELD_OFFSET(EFI_FILE_INFO,FileName);

    if (*BufferSize < Length) {
        *BufferSize = Length;
        return EFI_BUFFER_TOO_SMALL;
    }

    *BufferSize = Length;
    buffer->Size = Length;
    buffer->FileSize = Pfh->DirectoryEntry.DIR_RealSize;
    buffer->PhysicalSize = Pfh->DirectoryEntry.DIR_AllocSize;

    NTFSToEfiTime (&(buffer->CreateTime),
                   Pfh->DirectoryEntry.DIR_CreationTime);
    NTFSToEfiTime (&(buffer->LastAccessTime),
                   Pfh->DirectoryEntry.DIR_ReadTime);
    NTFSToEfiTime (&(buffer->ModificationTime),
                   Pfh->DirectoryEntry.DIR_AlteredTime);
    buffer->Attribute = Pfh->DirectoryEntry.DIR_FileFlags & \
                            EFI_FILE_VALID_ATTR;
    Wcscpy (buffer->FileName, Pfh->FileNameLFN);
//###DEBUG This may not be needed.
    if ( (buffer->Attribute & ATTR_DIRECTORY) && \
         (Wcslen(buffer->FileName) == 0) )
    { // remove other flags from root directory
        buffer->Attribute = ATTR_DIRECTORY; 
    }
//###DEBUG End    
    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       GetFileInfoFromDel
//
// Description:     Returns information on a file from DEL entry
//
// Parameters:      DIR_ENTRY_LIST *Del - Ponter to DEL entry
//                  UINTN *BufferSize - Pointer to size of buffer
//                  VOID *Buffer - Ponter to buffer for returned data
//
// Return value:    EFI_STATUS - Status of the operation
//
// Modified:
//
// Referral(s):
//
// NOTE(S):
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
GetFileInfoFromDel (
    DIR_ENTRY_LIST  *Del,
    IN OUT UINTN    *BufferSize,
    OUT VOID        *Buffer
    )
{
    EFI_FILE_INFO *buffer = Buffer;
    UINTN         Length;

    Length = (UINT32)Wcslen(Del->FileNameLFN) * sizeof (CHAR16) + 2;
    Length += EFI_FIELD_OFFSET(EFI_FILE_INFO,FileName);

    if (*BufferSize < Length) {
        *BufferSize = Length;
        return EFI_BUFFER_TOO_SMALL;
    }

    *BufferSize = Length;
    buffer->Size = Length;
    buffer->FileSize = Del->DirectoryEntry.DIR_RealSize;
    buffer->PhysicalSize = Del->DirectoryEntry.DIR_AllocSize;
    NTFSToEfiTime (&(buffer->CreateTime),
                   Del->DirectoryEntry.DIR_CreationTime);
    NTFSToEfiTime (&(buffer->LastAccessTime),
                   Del->DirectoryEntry.DIR_ReadTime);
    NTFSToEfiTime (&(buffer->ModificationTime),
                   Del->DirectoryEntry.DIR_AlteredTime);
    buffer->Attribute = Del->DirectoryEntry.DIR_FileFlags & \
                            EFI_FILE_VALID_ATTR;
    Wcscpy (buffer->FileName, Del->FileNameLFN);
//###DEBUG This may not be needed.
//    if ( (buffer->Attribute & ATTR_DIRECTORY) && \
//         (Wcslen(buffer->FileName) == 0) )
//    { // remove other flags from root directory
//        buffer->Attribute = ATTR_DIRECTORY; 
//    }
//###DEBUG End

    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   NTFSToEfiTime
//
// Description: Converts time/date in NTFS format to EFI format
//
// Parameters:  EFI_TIME *EfiTime - Ponter to returned time in EFI format
//              UINT64   NTFSTime - NTFS time (100 ns units since 1601)
//
// Return value: None
//
// Modified:
//
// Referral(s):
//
// NOTE(S):     NTFS Time is the number of 100ns units since Jan 1, 1601.
//              The signifigance of this date is that it is the beginning
//              of the first full century of the Gregorian Calendar.
//              The time displayed in Shell will only match the time
//              displayed in Windows if the Windows time zone is set to
//              Coordinated Universal Time (UTC). Otherwise, it will be
//              skewed by the time zone setting.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
NTFSToEfiTime (
    EFI_TIME *EfiTime,
    UINT64   NTFSTime
    )
{
    UINT64 Remainder64;
    UINT32 Remainder32;
    UINT16 Year;
    UINT8  Month;
    UINT32 Day;
    UINT32 LastDay;
    UINT32 PrevLD;
    UINT8  i;
    UINT64 Temp;

    EfiTime->TimeZone = EFI_UNSPECIFIED_TIMEZONE;
    EfiTime->Pad1 = 0;
    EfiTime->Daylight = 0;
    EfiTime->Pad2 = 0;
//
// In the calculations below,
// 315360000000000 is the number of 100ns units in a year.
// 864000000000 is the number of 100ns units in a day.
// 36000000000 is the number of 100ns units in an hour.
// 600000000 is the number of 100ns units in a minute.
// 10000000 is the number of 100ns units in a second    
//    
// Because calendars are 1-based (there is no day 0), we have to add
// a day's worth of 100-ns units to make these calcualtions come out
// correct.
//
    Temp = NTFSTime + 864000000000;
//
// Calculate the year
//
    Year = 1601;
    for (Temp; Temp > 315360000000000; Temp -= 315360000000000)
    {
        if ( LEAPYEAR ) 
            Temp -= 864000000000; // Subtract an extra day for leap year
        Year++;
    }
//
// We have the year. From what's left, get the day, hour, minute, second,
// and nanosecond.
//
    Day = (UINT32)Div64x64x64(Temp, 864000000000, &Remainder64);
    if (Day == 0) // Special handling for last day of year
    {
        Year -= 1;
        if ( LEAPYEAR ) Day = 366; else Day = 365;
    }
    EfiTime->Year = Year;
    EfiTime->Hour = (UINT8)Div64x64x64(Remainder64, 36000000000, &Remainder64);
    EfiTime->Minute = (UINT8)Div64(Remainder64, 600000000, (UINTN*)&Remainder32);
    EfiTime->Second = (UINT8)(Remainder32 / 10000000); 
    Remainder32 = Remainder32 % 10000000;
    EfiTime->Nanosecond = Remainder32 * 100;
//    
// "Day" now contains the ordinal date. We have to convert that to month and day.
//    
    Month = 1; LastDay = 31; PrevLD = 0;
    if ( LEAPYEAR )
    {
        for (i=1; i<13; i++)
        {
            if (Day > LastDay) 
                {Month += 1; PrevLD = LastDay; LastDay += DPML[i];}
        }
    } else {
        for (i=1; i<13; i++)
        {
            if (Day > LastDay) 
                {Month += 1; PrevLD = LastDay; LastDay += DPM[i];}
        }
    }
    EfiTime->Month = Month;
    EfiTime->Day = (UINT8)(Day - PrevLD);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   EfiToNTFSTime
//
// Description: Converts date/time in EFI format to NTFS format
//
// Parameters:  EFI_TIME EfiTime - Time in EFI format
//              UINT64   *NTFSTime - Time in NTFS format
//
// Return value: None
//
// Modified:
//
// Referral(s):
//
// NOTE(S):     NTFS Time is the number of 100ns units since Jan 1, 1601.
//              This function is not used, but will be needed if write
//              capability is added to this driver.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
EfiToNTFSTime (
    EFI_TIME EfiTime,
    UINT64   *NTFSTime
    )
{
    UINT64 Units;
    UINT32 Temp32;
    UINT16 Year;
    UINT16 Days = 0;
    UINT8  i;

    Year = EfiTime.Year;
    Temp32 = (Year - 1601) * 100;
    Temp32 = Temp32 * 36524; // Temp32 now has days*100
    Units = Mul64(8640000000, Temp32); // Units now has the year in 100ns units

    if ( LEAPYEAR )
    { // If leap year
        for (i = 0; i < EfiTime.Month; i++)
        {
            Days += DPML[i];
        }
    } else { // Normal year
        for (i = 0; i < EfiTime.Month; i++)
        {
            Days += DPM[i];
        }
    }

    Days += EfiTime.Day;

    Units = Units + Mul64(864000000000, Days);
    Units = Units + Mul64(36000000000, EfiTime.Hour);
    Units += (EfiTime.Minute * 600000000);
    Units += (EfiTime.Second * 10000000);
    Units += (EfiTime.Nanosecond / 100);

    *NTFSTime = Units;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetRunListElementData
//
// Description: Retrieves the count and cluster of a run from a run list element
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
// Procedure:   GetNameAndRunlist
//
// Description: Retrieves the Name Attribute information and the Data
//              Attribute information (Run List) from a File Record
//
// Parameters:  UINT8   *Buffer - Pointer to a buffer containing a file record
//              UINT32  **pName - Points to the Name attribute information.
//              UINT64  **pRunList - Points to the Run List.
//
// Return value:    EFI_STATUS Status (EFI_SUCCESS or EFI_NOT_FOUND)
//
// Modified:
//
// Referral(s):
//
// NOTE(S):
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
GetNameAndRunlist (
    UINT8 *Buffer,
    UINT8 **pName,
    UINT8 **pRunList
    )
{
    EFI_STATUS Status;
    UINT8      *TmpBuffer;

//
// Search for Name attribute.
//         
    TmpBuffer = NULL;
    Status = GetFrAttribute (Buffer, FR_ATTRIBUTE_NAME, &TmpBuffer);
    if (EFI_ERROR(Status)) return Status;
//
// We don't support non-resident names.
//    
    if ( ((FR_ATTR_HEADER_RES*)TmpBuffer)->AHR_NonResFlag ) return EFI_NOT_FOUND;
//    
// We don't support Compressed, Encrypted, or Sparse files
// (see definition of AHR_Flags).
//    
    if ( ((FR_ATTR_HEADER_RES*)TmpBuffer)->AHR_Flags ) return EFI_NOT_FOUND;
//    
// Get the pointer to the name information.
//    
    TmpBuffer += ((FR_ATTR_HEADER_RES*)TmpBuffer)->AHR_InfoOffset;
    *pName = TmpBuffer;
//
// Search for Data attribute.
//    
    Status = GetFrAttribute (Buffer, FR_ATTRIBUTE_DATA, &TmpBuffer);
    if (EFI_ERROR(Status)) return Status;
    TmpBuffer += ((FR_ATTR_HEADER_NONRES*)TmpBuffer)->AHNR_RunOffset;
    *pRunList = TmpBuffer;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   GetFrAttribute
//
// Description: Retrieves a File Record Attribute by it's number from a
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
// Check for "FILE", file record marker.
//    
	if ( *((UINT32*)BufferIn) != EFI_SIGN_32 ('F', 'I', 'L', 'E') ) \
		return EFI_NOT_FOUND;
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
// Description: Returns the file record specified by MFTRecordNo in a buffer.
//
// Parameters:  VOLUME_INFO *Vi - Volume Info Structure
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
    VOLUME_INFO *Vi,
    UINT64      MFTRecordNo,
    UINT8       *Buffer,
    UINT64      *MFTSector OPTIONAL
    )
{
    EFI_STATUS Status;
    UINT8      *pRunList;
    UINT64     Cluster;
    UINT64     Sector;
    UINT64     Count;
    UINT64     ByteCount;
    UINT32     SecPerRecord;
    UINT64     RecordCount;
    UINT32     RecordSize;

    pRunList = &Vi->MFTRunList[0];
    Cluster = 0;

    MFTRecordNo &= MAXIMUM_RECORD_NUMBER; // Isolate number part

    Status = GetRunListElementData(&pRunList, &Count, &Cluster, TRUE);
    ByteCount = Mul64(Count, Vi->BytesPerCluster);

    if (Vi->BytesPerCluster == 512) {
        if (ByteCount % 2) {
            TRACE((-1, "\nERROR -- Cluster has odd no. of sectors\n"));
            return EFI_NOT_FOUND;
        }
        ByteCount = ByteCount / 2;
    }

    if ( Vi->VolumeBPB.BPB_BytePerSec <= FILE_RECORD_SIZE ) {
        SecPerRecord = FILE_RECORD_SIZE / Vi->VolumeBPB.BPB_BytePerSec;
        RecordSize = FILE_RECORD_SIZE;
    } else { // Special case for 4k sectors
        SecPerRecord = 1;
        RecordSize = (UINT32)Vi->VolumeBPB.BPB_BytePerSec;
    }

    Sector = Mul64(Cluster, Vi->VolumeBPB.BPB_SecPerClus);
    RecordCount = 0;
    do {
        if (ByteCount > 0)
        {
            Sector += SecPerRecord;
            ByteCount -= RecordSize;
        } else { // We've used up a run, read from the next one.
            Status = GetRunListElementData(&pRunList, &Count, &Cluster, TRUE);
            if (EFI_ERROR(Status)) return EFI_NOT_FOUND;
            ByteCount = Mul64(Count, Vi->BytesPerCluster);

            if (Vi->BytesPerCluster == 512) {
                if (ByteCount % 2) {
                    TRACE((-1, "\nERROR -- Cluster has odd no. of sectors\n"));
                    return EFI_NOT_FOUND;
                }
                ByteCount = ByteCount / 2;
            }

            Sector = Mul64(Cluster, Vi->VolumeBPB.BPB_SecPerClus);
            continue;
        }
        RecordCount++;
    } while (RecordCount < MFTRecordNo); // Record numbers are 0-based.
//
// We found the sector of the file record wanted. Now read it.
//
    Status = FsReadMedia (Vi,
                          Buffer,
                          Sector,
                          0,
                          RecordSize,
                          MFT_REGION);
    if (EFI_ERROR(Status)) return EFI_NOT_FOUND;
//
// A File recored begins with "FILE". Check it.
//
	if ( *((UINT32*)Buffer) != EFI_SIGN_32 ('F', 'I', 'L', 'E') ) \
		return EFI_NOT_FOUND;

    if ( MFTSector )
    *MFTSector = Sector; // Return sector where the record was found

//
// Check if fixup is needed, and apply it if so
//
    Status = ApplyFixup (Vi, Buffer);

    if (EFI_ERROR(Status))
        return EFI_NOT_FOUND;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   ApplyFixup
//
// Description: Applies fixup on a File or Index record if necessary
//
// Parameters:  VOLUME_INFO *Vi - Volume Info Structure
//              UINT8 *Buffer - Buffer containing File or Index record
//
// Return value: EFI_STATUS Status (EFI_SUCCESS or EFI_VOLUME_CORRUPTED)
//
// Modified:
//
// Referral(s):
//
// NOTE(S):     The "Fixup" is a method NTFS uses to protect File
//              and Index records from bad sectors.
//              For more information, see "NTFS Documentation" by
//              Richard Russon and Yuval Fledel, chapter 4, section 14,
//              "Concept - Fixup".
//              For a record that requires fixup, the last two bytes
//              of each sector are stored in a "Fixup array", and are
//              replaced with a fixup number from the record's header. 
//              This function compares those last sector bytes with
//              the fixup number, and replaces them with the
//              stored originals, and reports a corrupt volume if
//              there is a comparison mis-match.
//              4K Sectors: For 4K native sectors, fixups are done
//              as if sectors were still 512 bytes. The last two bytes
//              of every 512-byte piece are compared.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
ApplyFixup (
    VOLUME_INFO *vi,
    UINT8       *Buffer
)
{
    UINT16      *FixupPtr;
    UINT16      *ArrayPtr;
    UINT16      *FixupArray;
    UINT16      FixupCount;

    //check for signature 'FILE' or 'INDX'
    if ( *((UINT32*)Buffer) != EFI_SIGN_32 ('F', 'I', 'L', 'E') && \
         *((UINT32*)Buffer) != EFI_SIGN_32 ('I', 'N', 'D', 'X') ) {
        return EFI_NOT_FOUND;
    }

    FixupArray = (UINT16*)(Buffer + \
        ((MFT_FILE_RECORD*)Buffer)->FR_FixupSeqOffset);
    FixupCount = (((MFT_FILE_RECORD*)Buffer)->FR_FixupSeqSize) - 1;

    // Check if fixup needed to be applied
    if (*FixupArray == 0) {
        return EFI_SUCCESS;
    }

    ArrayPtr = FixupArray + 1; // Array starts after the check word.
    // Point FixpuPtr to the last word of a sector or 512-byte piece
    FixupPtr = (UINT16*)(Buffer + 510);

    // Apply fixup
    do {
        if (*FixupPtr != *FixupArray) {
            return EFI_VOLUME_CORRUPTED;
        }

        *FixupPtr = *ArrayPtr;
        ArrayPtr++; // Point to next array element
        FixupPtr += 512/sizeof(*FixupPtr); // Point to next sector or piece
    } while (--FixupCount);

    *FixupArray = 0;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   CheckFlags
//
// Description: Check's a file's flags for valid values. If the file has
//              the NTFS directory flag set, it is changed to the FAT
//              directory flag.
//
// Parameters:  DIR_ENTRY_NTFS *DirectoryEntry
//
// Return value: EFI_STATUS
//
// Modified:
//
// Referral(s):
//
// NOTE(S):     EFI_FILE_INFO expects FAT-type flags. The Directory flag is
//              different in NTFS, and must be changed.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
CheckFlags (
    DIR_ENTRY_NTFS *DirectoryEntry
    )
{
//
// Check for unsupported file types (Compressed, Encrypted, or Sparce)
//
    if (DirectoryEntry->DIR_FileFlags & (ATTR_SPARCE | ATTR_COMPRESSED | \
                                         ATTR_ENCRYPTED) )
    {
        return EFI_UNSUPPORTED;
    }
//
// For now, we're not going to support duplicate DOS entries.
//
    if (DirectoryEntry->DIR_NameType == 2) return EFI_UNSUPPORTED;
//
// Check for the NTFS directory flag, and replace it with the FAT flag.
//
    if (DirectoryEntry->DIR_FileFlags & ATTR_DIRECTORY_NTFS)
    {
        DirectoryEntry->DIR_FileFlags &= ~ATTR_DIRECTORY_NTFS;
        DirectoryEntry->DIR_FileFlags |= ATTR_DIRECTORY;
//
// Some NTFS directories have something in the size fields. Zero it here.
//
        DirectoryEntry->DIR_AllocSize = 0;
        DirectoryEntry->DIR_RealSize = 0;
    }
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   Div64x64x64
//
// Description: Divides a 64-bit number by a 64-bit number, and
//              returns a 64-bit quotient and remainder.
//
// Parameters:  UINT64  Dividend
//              UINT64  Divisor
//              UINT64  *Remainder
//
// Return value: 64-bit Quotient
//
// Modified:
//
// Referral(s):
//
// NOTE(S):     
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

UINT64
Div64x64x64 (
    UINT64  Dividend,
    UINT64  Divisor,
    UINT64  *Remainder OPTIONAL
    )
{

#if EFIx64
    if (Remainder != NULL) {
        *Remainder = Dividend % Divisor;
    }
    return Dividend / Divisor;
#else

    UINT32  RemainderHi;
    UINT32  RemainderLo;
    UINT32  DividendHi;
    UINT32  DividendLo;
    UINT32  DivisorHi;
    UINT32  DivisorLo;
    UINT32  QuotientHi;
    UINT32  QuotientLo;
    UINT32  Count;

    DividendHi = (UINT32)(Shr64(Dividend, 32));
    DividendLo = (UINT32)(Dividend & 0xffffffff);
    DivisorHi = (UINT32)(Shr64(Divisor, 32));
    DivisorLo = (UINT32)(Divisor & 0xffffffff);
//
// The following is slow, but it is only used when directories are listed
// by the user, in time-stamp calculations.
//
    __asm {
        mov     edx, DividendHi
        mov     eax, DividendLo
        mov     ecx, DivisorHi
        mov     ebx, DivisorLo

        mov     Count, 64       ;Count off 64 bits
        sub     esi, esi        ;Set remainder to zero
        sub     edi, edi
    BitLoop:
	    shl     eax, 1          ;Perform 128-bit shift
        rcl     edx, 1         
        rcl     edi, 1          
        rcl     esi, 1
        cmp     esi, ecx        ;Compare hi dwords of Rem,
        ja      GoesInto        ; Divisor.
        jb      TryNext
        cmp     edi, ebx        ;Compare lo dwords.
        jb      TryNext
    GoesInto:
		sub     edi, ebx        ;Remainder = Remainder -
        sbb     esi, ecx        ;                Divisor
        inc     eax             ;Set lo bit of EAX
    TryNext:
	    dec     Count           ;Repeat 64 times.
        jne     BitLoop

        mov     QuotientHi, edx
        mov     QuotientLo, eax
        mov     RemainderHi, esi
        mov     RemainderLo, edi
    }

    if (Remainder != NULL)
    {
        *Remainder = Shl64(RemainderHi, 32) + RemainderLo;
    }

    return Shl64(QuotientHi, 32) + QuotientLo;
    #endif
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   FindNextTpl
//
// Description: Returns the higher of a desired TPL level or the current
//              next higher TPL level
//
// Parameters:  EFI_TPL TplLevel -- desired level to raise to
//
// Return Value: EFI_TPL
//
// Modified:
//
// Referral(s):
//
// NOTE(S):     
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_TPL
FindNextTpl (
    EFI_TPL     TplLevel
    )
{
    EFI_TPL NextTpl;

    NextTpl = pBS->RaiseTPL (TPL_HIGH_LEVEL);
    pBS->RestoreTPL (NextTpl);
    if (NextTpl <= TplLevel) NextTpl = TplLevel;
    return NextTpl;
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
