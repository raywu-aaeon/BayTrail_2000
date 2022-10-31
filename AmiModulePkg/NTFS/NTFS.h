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
// Name: NTFS.h
//
// Description: Header file for Simple File System driver
//
//<AMI_FHDR_END>
//**********************************************************************

//----------------------------------------------------------------------

#ifndef _NTFS_
#define _NTFS_

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------

#include <Efi.h>
#include <Token.h>
#include <Dxe.h>
#include <AmiDxeLib.h>
#include "Protocol\PciIo.h"
#include "Protocol\DevicePath.h"
#include "protocol\DriverBinding.h"
#include "protocol\BlockIo.h"
#include "protocol\DiskIo.h"
#include "Protocol\PDiskInfo.h"
#include "Protocol\SimpleFileSystem.h"
#include "Protocol\UnicodeCollation.h"
#include "Protocol\LoadedImage.h"
#include <Protocol\ComponentName.h>
#if AMI_FILE_SYSTEM_ID_PROTOCOL_SUPPORT
#include <Protocol\AmiFileSystemId.h>
#endif

//----------------------------------------------------------------------

#define     NTFS_DRIVER_VERSION         0x010000
//
// File Attributes (Flags)
// The below flags are the only ones common to both FAT and NTFS.
//
#define     ATTR_READ_ONLY          0x00000001
#define     ATTR_HIDDEN             0x00000002
#define     ATTR_SYSTEM             0x00000004
#define     ATTR_ARCHIVE            0x00000020
//
// The below attribute is not supported in NTFS, but we'll translate the NTFS
// equivalent to this for compatibility with EFI_FILE_INFO.
//
#define     ATTR_DIRECTORY          0x00000010
//
// None of the below attributes are supported in EFI_FILE_INFO. They will be
// stripped, and files having the ATTR_SPARCE, ATTR_COMPRESSED, or 
// ATTR_ENCRYPTED flags will not be included in directory listings.
//
#define     ATTR_DEVICE             0x00000040
#define     ATTR_NORMAL             0x00000080
#define     ATTR_TEMPORARY          0x00000100
#define     ATTR_SPARCE             0x00000200
#define     ATTR_REPARSE            0x00000400
#define     ATTR_COMPRESSED         0x00000800
#define     ATTR_OFFLINE            0x00001000
#define     ATTR_NOT_INDEXED        0x00002000
#define     ATTR_ENCRYPTED          0x00004000
//
// The below attribute will be translated to the equivalent FAT value.
//
#define     ATTR_DIRECTORY_NTFS     0x10000000

//
// NTFS MFT File Record Attributes (there are others, but we only use these).
//
#define     FR_ATTRIBUTE_NAME           0x00000030
#define     FR_ATTRIBUTE_VOLUME_NAME    0x00000060
#define     FR_ATTRIBUTE_DATA           0x00000080
#define     FR_ATTRIBUTE_INDEX_ROOT     0x00000090
#define     FR_ATTRIBUTE_INDEX_ALLOC    0x000000A0

//
// Normal size of a Master File Table File Record. 
// Larger if sectors > 1024 bytes.
//
#define     FILE_RECORD_SIZE            0x400 

//
// Normal size of an Index record
//
#define     INDEX_RECORD_SIZE           0x1000

//
// Maximum size of a record number
//
#define     MAXIMUM_RECORD_NUMBER       0x0000FFFFFFFFFFFF
//
// The smallest an Index Entry can be is 0x58 bytes.
//
#define     MINIMUM_ENTRY_SIZE          0x58

#define     MEMBLOCKSIZE                0x8000
#define     MEMALLOCSIZE                0x80
#define     MAX_OPEN_FHS                0x20
#define     FSI_Free_Count              488
#define     FSI_Nxt_Free                492

#define     EFI_TPL_CALLBACK            TPL_CALLBACK

#define EFI_SIGN_32(A, B, C, D)         ( (A) | (B << 8) | (C << 16) | (D << 24) )

#pragma pack(1)


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        REGIONS
//
// Description: Enumeration of volume regions
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef enum {
    RESERVED_REGION,
    MFT_REGION,
    DIRECTORY_REGION,
    DATA_REGION
} REGIONS;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        ACCESS_METHOD
//
// Description: Enumeration of access methods (read or write)
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef enum {
    READ_BLOCK,
    WRITE_BLOCK
} ACCESS_METHOD;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        DIR_ENTRY_NTFS
//
// Description: Structure of a NTFS directory entry (Index entry)
//
// Referrals:
//
// Notes:       This structure is identical to the Index Entry structure
//              Time stamps in NTFS are the number of 100ns units since
//              January 1, 1601.
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _DIR_ENTRY_NTFS {
    UINT64 DIR_MFTRecord;           // 0x00 - MFT Record number containing this entry
    UINT16 DIR_EntrySize;           // 0x08 - Size of this entry
    UINT16 DIR_NameOffset;          // 0x0a - Offset to file name
    UINT16 DIR_IndexFlags;          // 0x0C - Index Flags
    UINT16 DIR_Padding;
    UINT64 DIR_MFTParentRec;        // 0x10 - MFT Parent Record no.
    UINT64 DIR_CreationTime;        // 0x18 - File Creation Time
    UINT64 DIR_AlteredTime;         // 0x20 - File Altered Time
    UINT64 DIR_MFTChangedTime;      // 0x28 - MFT Record Changed Time
    UINT64 DIR_ReadTime;            // 0x30 - File Read Time
    UINT64 DIR_AllocSize;           // 0x38 - Allocated File Size
    UINT64 DIR_RealSize;            // 0x40 - Real File Size
    UINT64 DIR_FileFlags;           // 0x48 - File Flags (like DOS flags plus more)
    UINT8 DIR_NameLength;           // 0x50 - Length of File name
    UINT8 DIR_NameType;             // 0x51 - File Name type
                                    //        0x00 - POSIX
                                    //        0x01 - Unicode
                                    //        0x02 - DOS
                                    //        0x03 - Both
    CHAR16 DIR_Name[1];             // 0x52 - File Name starts here
} DIR_ENTRY_NTFS;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        VOLUME_BPB
//
// Description: Structure of the Volume BIOS Parameter Block for NTFS volumes.
//
// Referrals:   http://www.ntfs.com/ntfs-partition-boot-sector.htm
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct  _VOLUME_BPB {
    UINT8 BS_JmpBoot[3];        // 0x00
    UINT8 BS_OEMName[8];        // 0x03
    UINT16 BPB_BytePerSec;      // 0x0b
    UINT8 BPB_SecPerClus;       // 0x0d
    UINT16 BPB_RsvdSecCnt;      // 0x0e
    UINT8 BPB_NumFATs;          // 0x10 - always 0
    UINT16 BPB_RootEntCnt;      // 0x11 - always 0
    UINT16 BPB_TotSec16;        // 0x13 - not used
    UINT8 BPB_Media;            // 0x15 - always 0xF8
    UINT16 BPB_FATSz16;         // 0x16 - always 0
    UINT16 BPB_SecPerTrk;       // 0x18
    UINT16 BPB_NumHeads;        // 0x1a
    UINT32 BPB_Hiddsec;         // 0x1c
    UINT32 BPB_TotSec32;        // 0x20 - always 0
    UINT32 BPB_FATSz32;         // 0x24 - not used
    UINT64 BPB_TotSec64;        // 0x28 - Total sectors on volume
    UINT64 BPB_MFT;             // 0x30 - Master File Table cluster
    UINT64 BPB_MFTMirr;         // 0x38 - MFT Mirror cluster
    UINT32 BPB_CPFR;            // 0x40 - Cluster per File Record
    UINT32 BPB_CPIB;            // 0x44 - Clusters per Index Block
    UINT64 BPB_Serial;          // 0x48 - Volume Serial No.
    UINT32 BPB_CkSum;           // 0x50 - Checksum
} VOLUME_BPB;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MFT_FILE_RECORD
//
// Description: Structure of a Master File Table file record.
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _MFT_FILE_RECORD
{
    CHAR8 FR_Name[4];               // 0x00 - Always "FILE"
    UINT16 FR_FixupSeqOffset;       // 0x04 - Fixup sequence offset
    UINT16 FR_FixupSeqSize;         // 0x06 - Fixup sequence size
    UINT64 FR_LogSeqNumber;         // 0x08
    UINT16 FR_SequenceNumber;       // 0x10
    UINT16 FR_HardLinkCount;        // 0x12
    UINT16 FR_AttributeOffset;      // 0x14 - Offset to Attributes
    UINT16 FR_Flags;                // 0x16 - File Record Flags
                                    //        0x01 - Record is in use
                                    //        0x02 - Record is a directory
    UINT32 FR_RealSize;             // 0x18 - Real size of record
    UINT32 FR_AllocatedSize;        // 0x1C - Allocated size of record
    UINT64 FR_BaseRecordNumber;     // 0x20 - Zero if this is a base record
} MFT_FILE_RECORD;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        FR_ATTR_HEADER_RES
//
// Description: Structure of a File Record resident attribute header.
//
// Referrals:
//
// Note(s):     The "Name" in this structure is not a file name, but the
//              name of the attribute. Not used in this driver.
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _FR_ATTR_HEADER_RES
{
    UINT32 AHR_Type;                // 0x00 - Attribute Type
                                    //        0x30 - File Name
                                    //        0x80 - Data
    UINT32 AHR_Length;              // 0x04 - Attribute Length
    UINT8 AHR_NonResFlag;           // 0x08 - Non-Resident Flag
    UINT8 AHR_NameLength;           // 0x09 - Name Length (if named)
    UINT16 AHR_NameOffset;          // 0x0A - Offset to Name (if named)
    UINT16 AHR_Flags;               // 0x0C - Attribute Hdr. Flags 
                                    //        0x0001 - Compressed
                                    //        0x4000 - Encrypted
                                    //        0x8000 - Sparse
    UINT16 AHR_AttributeId;         // 0x0E
    UINT32 AHR_InfoLength;          // 0x10 - Attribute Information Length
    UINT16 AHR_InfoOffset;          // 0x14 - Offset to this Attribute's 
                                    //        Information
    UINT16 AHR_IndexFlag;           // 0x16
} FR_ATTR_HEADER_RES;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        FR_ATTR_HEADER_NONRES
//
// Description: Structure of a File Record non-resident attribute header.
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _FR_ATTR_HEADER_NONRES
{
    UINT32 AHNR_Type;               // 0x00 - Attribute Type
                                    //        0x30 - File Name
                                    //        0x80 - Data
    UINT32 AHNR_Length;             // 0x04 - Attribute Length
    UINT8 AHNR_NonResFlag;          // 0x08 - Non-Resident Flag
    UINT8 AHNR_NameLength;          // 0x09 - Name Length (if named)
    UINT16 AHNR_NameOffset;         // 0x0A - Offset to Name (if named)
    UINT16 AHNR_Flags;              // 0x0C - Flags
                                    //        0x0001 - Compressed
                                    //        0x4000 - Encrypted
                                    //        0x8000 - Sparse
    UINT16 AHNR_AttributeId;        // 0x0E
    UINT64 AHNR_StartingVCN;        // 0x10 - Staeting Virtual Cluster Number
    UINT64 AHNR_LastVCN;            // 0x18 - Last Virtual Cluster Number
    UINT16 AHNR_RunOffset;          // 0x20 - Offset to data runs
                                    //        (Usually 0x40 for a 
                                    //        Data attribute)
} FR_ATTR_HEADER_NONRES;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        FR_NAME_ATTRIBUTE (0x30)
//
// Description: Structure of a Master File Table Name Attribute.
//
// Referrals:
//
// Notes:       Time stamps in NTFS are the number of 100ns units since
//              January 1, 1601.
//              This attribute is preceeded by a resident attribute header.
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _FR_NAME_ATTRIBUTE
{
    UINT64 NA_PDReference;          // 0x00 - Parent Directory Reference 
    UINT64 NA_CreationTime;         // 0x08 - File Creation Time
    UINT64 NA_AlteredTime;          // 0x10 - File Altered Time
    UINT64 NA_MFTChangedTime;       // 0x18 - MFT Record Changed Time
    UINT64 NA_ReadTime;             // 0x20 - File Read Time
    UINT64 NA_AllocatedSize;        // 0x28 - Allocated Size of File
    UINT64 NA_RealSize;             // 0x30 - Real size of File
    UINT32 NA_Flags;                // 0x38 - File Flags
                                    //        Note: These flags correspond to 
                                    //        DOS flags up to 0x20 (Archive).
                                    //        If other flags are set, reject
                                    //        the file.
    UINT32 NA_Reparse;              // 0x3C
    UINT8 NA_NameLength;            // 0x40 - Length of file name in Words
    UINT8 NA_NameType;              // 0x41 - File Name type
                                    //        0x00 - POSIX
                                    //        0x01 - Unicode
                                    //        0x02 - DOS
                                    //        0x03 - Both
    CHAR16 NA_Name[1];              // 0x42 - File Name (unicode)
} FR_NAME_ATTRIBUTE;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        FR_DATA_ATTRIBUTE (0x80)
//
// Description: Structure of a Master File Table Data Attribute.
//
// Referrals:
//
// Notes:       This attribute can be preceeded by a resident or non-resident
//              attribute header. If resident, the data will be the actual
//              file data. If non-resident, the data will be a run list
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _FR_DATA_ATTRIBUTE
{
    UINT8 DA_Data[1];   // 0x00 - Run List or data
} FR_DATA_ATTRIBUTE;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        FR_INDEX_ROOT_ATTRIBUTE (0x90)
//
// Description: Structure of a Master File Table Index Root Attribute.
//
// Referrals:
//
// Notes:       A small index will have it's entries inside this attribute.
//              This attribute is preceeded by a resident attribute header.
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _FR_INDEX_ROOT_ATTRIBUTE
{
    UINT32 IRA_Type;
    UINT32 IRA_Collation;
    UINT32 IRA_Size;
    UINT8 IRA_ClustersPerIndexRec;
    UINT8 Padding1[3];
    UINT32 IRA_Offset;      // 0x10 - Offset to Index entries (if small index)
                            // The above offset is from here, 
                            // not from beginning.
    UINT32 IRA_TotalSize;
    UINT32 IRA_AllocSize;
    UINT8 IRA_Flags;        // 0x1C - 0 = small index, 1 = large index
    UINT8 Padding2[3];
} FR_INDEX_ROOT_ATTRIBUTE;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        FR_INDEX_ALLOC_ATTRIBUTE (0xA0)
//
// Description: Structure of a Master File Table Index Allocation Attribute.
//
// Referrals:
//
// Notes:       A large index will be in a separate index file, and the 
//              run list in this attribute will indicate where it is.
//              This attribute is preceeded by a non-resident attribute header
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _FR_INDEX_ALLOC_ATTRIBUTE
{
    UINT8 IAA_RunList[1];   // 0x00 - Run list of index record
} FR_INDEX_ALLOC_ATTRIBUTE;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        INDEX_RECORD
//
// Description: Structure of an Index record.
//
// Referrals:
//
// Notes:       In NTFS, the Index is equivalent to a directory in FAT. A large
//              index will be contained in an Index Record, while a small index
//              will be contained within the Master File Table File Record for
//              the index, in the Index Root Attribute. Index Records are 
//              files, the way FAT directories are files, but not every 
//              "directory" will have a file. See the structure INDEX_ENTRY 
//              for the actual "directory entry" format.
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _INDEX_RECORD
{
    CHAR8 INDR_Name[4];              // 0x00 - Always "INDX"
    UINT16 INDR_UpSeqOff;            // 0x04 - Offset to update sequence
    UINT16 INDR_UpSeqSize;           // 0x06 - Size of update sequence
    UINT64 INDR_LogSeqNo;            // 0x08 - Log sequence number
    UINT64 INDR_VCN;                 // 0x10 - Virtual Cluster Number
    UINT32 INDR_IndxEntryOff;        // 0x18 - Offset to Index Entries
                                     //        (from here, not from start)
    UINT32 INDR_IndxEntrySize;       // 0x1C - Size of Index Entries
    UINT32 INDR_IndxEntryAlloc;      // 0x20 - Allocated size of Entries
    UINT8 INDR_LeafNode;             // 0x24 - 1 if not leaf node
    UINT8 INDR_Padding[3];           // 0x25 - Padding
    UINT16 INDR_UpdateSequence;      // 0x28 - Update Sequence
    UINT16 INDR_SeqenceArray[1];     // 0x2A - Sequence Array
} INDEX_RECORD;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        INDEX_ENTRY
//
// Description: Structure of an Index record entry.
//
// Referrals:
//
// Notes:       In NTFS, the index is the equivalent of a directory in FAT,
//              so the index entry is the equivalent of a directory entry.
//              The lower 4 bytes of the MFT reference is the number of the
//              file's File Record in the MFT. The lower 4 bytes of the MFT
//              parent reference is the number of the File Record for the
//              Index that this entry is in.
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _INDEX_ENTRY
{
    UINT64 INDE_MFTRecord;      // 0x00 - MFT reference of this entry's file
    UINT16 INDE_EntrySize;      // 0x08 - Size of this entry
    UINT16 INDE_NameOffset;     // 0x0a - Offset to file name
    UINT16 INDE_IndexFlags;     // 0x0C - Index Flags
    UINT16 INDE_Padding;
    UINT64 INDE_MFTParentRec;   // 0x10 - MFT refernece of this entry's parent
    UINT64 INDE_CreationTime;   // 0x18 - File Creation Time
    UINT64 INDE_AlteredTime;    // 0x20 - File Altered Time
    UINT64 INDE_MFTChangedTime; // 0x28 - MFT Record Changed Time
    UINT64 INDE_ReadTime;       // 0x30 - File Read Time
    UINT64 INDE_AllocSize;      // 0x38 - Allocated File Size
    UINT64 INDE_RealSize;       // 0x40 - Real File Size
    UINT64 INDE_FileFlags;      // 0x48 - File Flags (like DOS flags + more)
    UINT8 INDE_NameLength;      // 0x50 - Length of File name
    UINT8 INDE_NameType;        // 0x51 - File Name type
                                //        0x00 - POSIX
                                //        0x01 - Unicode
                                //        0x02 - DOS
                                //        0x03 - Both
    CHAR16 INDE_Name[1];        // 0x52 - File Name (Unicode, not terminated)
} INDEX_ENTRY;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        VOLUME_INFO
//
// Description: Structure containing all Volume Information
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _VOLUME_INFO {

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL SimpleFileSystemProtocol; 
                                // This should be the first entry
                                // in this structure
    EFI_HANDLE VolumeHandle;
    EFI_STATUS VolumeStatus;    // Global status that can be used 
                                // by any open handles
    VOLUME_BPB VolumeBPB;
    BOOLEAN ValidVolumeID;
    CHAR16 VolumeID[128];       // Volume name can be up to 127 chars.
    CHAR16 *NTFSName;
    UINT8 BytesPerSecPowerof2;
    UINT32 BytesPerCluster;
    UINT64 TotalSectors;
    UINT64 DataSectors;
    UINT64 CountOfDataClusters;
//
// Volume information for EFI_FILE_INFO
//    
    UINT64 VolumeSize;
    UINT32 FreeSpaceinClusters;
    UINT64 FreeSpaceinSector1;
    UINT32 BlockSize;

    BOOLEAN ReadOnly;
    UINT32 LastFreeCluster;
    UINT32 FreeClusterBlocks;

    EFI_DISK_IO_PROTOCOL *DiskIo;
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
    UINT32 MediaID;
    EFI_UNICODE_COLLATION_PROTOCOL *UnicodeCollationInterface;

    UINT32 AllocatedMemorySize;
    UINT32 TotalDirEntryList;
    UINT32 AllowedMFTCacheSize;
    UINT32 AllowedDirCacheSize;
    UINT32 MFTCacheSize;
    UINT32 DirCacheSize;
    UINT32 *TempBuffer;
    UINT32 TempBufferSize;
    UINT32 *CacheBuffer;
    BOOLEAN TempBufferInUse;
    struct _FILE_HANDLE * RootFH;
    DLIST OpenFHs;
    DLIST OpenFIs;
    DLIST CacheList;
    DLIST MemList;
    UINT8 MFTRunList[MFT_RUN_LIST_SIZE];    // Master File Table run list
} VOLUME_INFO;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        FILE_HANDLE
//
// Description: Structure containing File Handle information
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _FILE_HANDLE {

    VOLUME_INFO *VolumeInterface;
    EFI_STATUS HandleStatus;
    BOOLEAN ROOTDIR;
    CHAR16 *FileNameLFN;            // Points to the Filename
    DIR_ENTRY_NTFS DirectoryEntry;  // Index entry for file
    UINT32 MFTReference;            // MFT reference no. of this file
    UINT32 MFTParent;               // MFT reference no. of this file's parent
    UINT16 InstancesCount;          // No of Instances opened so far
    BOOLEAN DirEntryChanged;
    BOOLEAN DirListChanged;
    UINT8 ResidentFile;             // Resident file Flag
                                    // 0 = Non-Resident
                                    // 1 = Resident
                                    // 2 = Both (Indexes only)
                                    // (The data is in the file record.)
    struct _FILE_HANDLE *Parent;    // Points to the parent directory
    DLIST DirList;                  // List if Directory Entries
    DLINK ViFHLink;                 // This link is for the DLIST OpenFHs 
                                    // in VOLUME_INFO
    DLIST ChildList;
    DLINK ChildLink;
    UINT8 FileRunList[FILE_RUN_LIST_SIZE];  // This file's run list.

} FILE_HANDLE;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        FILE_HANDLE_INSTANCE
//
// Description: Structure containing File Handle Instance information
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _FILE_HANDLE_INSTANCE {

    EFI_FILE_PROTOCOL FileHandle; // Should be the first entry 
                                  // in the structure
    EFI_STATUS HandleInstanceStatus;
    FILE_HANDLE *pFH;
    UINT64 OpenMode;              // Open Modes
    UINT64 Position;              //
    UINT64 CurrentCluster;        // will point to cluster number where the
                                  // position is pointing currently
    UINT32 CurrentClusterOffset;  // will point to offset in cluster where
                                  // the position is pointing currently
    UINT64 CurrentRunLength;      // Length of current run (in clusters)
    UINT8  *pRunList;             // Run list pointer
    BOOLEAN MEDIA_NOT_VALID;      // Will be true if for any reason current
                                  // instances cannot use the volume
                                  // Eg: Change in Media
    DLINK ViFILink;               // This link is for the DLIST OpenFIs
                                  // in the VOLUME_INFO
} FILE_HANDLE_INSTANCE;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        DIR_ENTRY_LIST
//
// Description: Structure containing directory entry information
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {

    DIR_ENTRY_NTFS DirectoryEntry;  // NTFS index entry
    CHAR16 *FileNameLFN;            // Points to the Filename
    UINT32 Number;                  // Directory entry sequence number.
    DLINK DirLink;                  

} DIR_ENTRY_LIST;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        CACHE_HEADER
//
// Description: Structure containing cache header information
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {

    UINT64 AbsoluteOffset;
    UINT64 AbsoluteOffsetEnd;
    UINT8 *Buffer;
    BOOLEAN DIRTY_FLAG;
    REGIONS DataRegion;
    DLINK CacheLink;

} CACHE_HEADER;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MEM_HEADER
//
// Description: Structure containing memory header information
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {

    UINTN AddresMap;
    UINT32 AddresMapLength;
    UINTN BufferStart;
    UINTN BufferEnd;
    DLINK MemLink;

} MEM_HEADER;

#pragma pack()


//----------------------------------------------------------------------
// FUNCTION PROTOTYPES
//----------------------------------------------------------------------
EFI_STATUS
NTFSSupported(
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE                  Controller,
    IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
    );

EFI_STATUS
NTFSStart (
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE                  Controller,
    IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
    );

EFI_STATUS
NTFSStop (
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE                  Controller,
    IN UINTN                       NumberOfChildren,
    IN EFI_HANDLE                  *ChildHandleBuffer
    );

EFI_STATUS
NTFSInitUnicodeCollation (
    IN EFI_DRIVER_BINDING_PROTOCOL        *This,
    IN OUT EFI_UNICODE_COLLATION_PROTOCOL **gNTFSUnicodeCollationInterface
    );

EFI_STATUS
DetectInstallVolume(
    IN EFI_DISK_IO_PROTOCOL  *DiskIo,
    IN EFI_BLOCK_IO_PROTOCOL *BlkIo,
    IN EFI_HANDLE            Handle
    );

EFI_STATUS
DetectVolume (
    VOLUME_INFO *VolumeInterface,
    BOOLEAN     StatusCheck
    );

BOOLEAN
FindNameAttribute (
    UINT8  *Buffer,
    UINT32 *Offset,
    UINT32 *NameAttribute,
    UINT32 RunList,
    UINT32 *BufferSize,
    UINT32 *NameCount
    );

EFI_STATUS
OpenVolume (
    IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
    OUT EFI_FILE_PROTOCOL              **Root
    );

EFI_STATUS
OpenRoot(
    IN VOLUME_INFO     *VolumeInfo,
    IN OUT FILE_HANDLE **Root
    );

EFI_STATUS
CreateFileHandleInstance (
    IN FILE_HANDLE              *Root,
    IN OUT FILE_HANDLE_INSTANCE **FileInstance,
    IN UINT64                   OpenMode
    );

void
InitEFIFileInterface (
    EFI_FILE_PROTOCOL *EFIFileInterface
    );

EFI_STATUS
OpenVolume (
    IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
    OUT EFI_FILE_PROTOCOL              **Root
    );

EFI_STATUS
OpenFileHandle (
    IN EFI_FILE_PROTOCOL  *This,
    OUT EFI_FILE_PROTOCOL **NewHandle,
    IN CHAR16             *FileName,
    IN UINT64             OpenMode,
    IN UINT64             Attributes
    );

EFI_STATUS
CloseFileHandle (
    IN EFI_FILE_PROTOCOL *This
    );

EFI_STATUS
DeleteFileHandle (
    IN EFI_FILE_PROTOCOL *This
    );

EFI_STATUS
ReadFileHandle (
    IN EFI_FILE_PROTOCOL *This,
    IN OUT UINTN         *BufferSize,
    OUT VOID             *Buffer
    );

EFI_STATUS
WriteFileHandle (
    IN EFI_FILE_PROTOCOL *This,
    IN OUT UINTN         *BufferSize,
    IN VOID              *Buffer
    );

EFI_STATUS
GetPositionFileHandle (
    IN EFI_FILE_PROTOCOL *This,
    IN UINT64            *Position
    );

EFI_STATUS
SetPositionFileHandleThis (
    IN EFI_FILE_PROTOCOL *This,
    OUT UINT64           Position
    );


EFI_STATUS
SetPositionFileHandle (
    IN FILE_HANDLE_INSTANCE *fhi,
    IN UINT64               Position
    );

EFI_STATUS
GetInfoFileHandle (
    IN EFI_FILE_PROTOCOL *This,
    IN EFI_GUID          *InformationType,
    IN OUT UINTN         *BufferSize,
    OUT VOID             *Buffer
    );


EFI_STATUS
SetInfoFileHandle (
    IN EFI_FILE_PROTOCOL *This,
    IN EFI_GUID          *InformationType,
    IN UINTN             BufferSize,
    IN VOID              *Buffer
    );

EFI_STATUS
FlushFileHandle (
    IN EFI_FILE_PROTOCOL *This
    );

EFI_STATUS
ProcessOpenFileHandle(
    IN FILE_HANDLE_INSTANCE *ParentFi,
    IN CHAR16               **FileName,
    OUT EFI_FILE_PROTOCOL   **NewHandle,
    IN UINT64               OpenMode,
    IN UINT64               Attributes
    );

BOOLEAN
FindMatchingFH (
    IN VOLUME_INFO  *Vi,
    IN CHAR16       *NextCompName,
    IN FILE_HANDLE  *fh,
    OUT FILE_HANDLE **fh1
    );

EFI_STATUS
ReadAllDirectoryEntries (
    IN FILE_HANDLE *fh
    );

EFI_STATUS
CreateFHFromDirEntryList(
    IN DIR_ENTRY_LIST *Del,
    IN FILE_HANDLE    *fh,
    OUT FILE_HANDLE   **fh1
    );

BOOLEAN
FindMatchingDirEntry (
    IN FILE_HANDLE     *fh,
    IN CHAR16          *NextCompName,
    OUT DIR_ENTRY_LIST **Del
    );

BOOLEAN
FindMatchingSFNDirEntry (
    IN FILE_HANDLE     *fh,
    IN CHAR8           *ShortFileName,
    OUT DIR_ENTRY_LIST **Del
    );

EFI_STATUS
AddToDirList (
    FILE_HANDLE    *fh,
    DIR_ENTRY_NTFS DirectoryEntry,
    CHAR16         *lfn,
    UINT32         Number
    );

EFI_STATUS
GetDirListAtPosition(
    FILE_HANDLE    *Pfh,
    UINT64         Position,
    DIR_ENTRY_LIST **Del
    );

EFI_STATUS
GetNextCompName (
    IN OUT CHAR16 **FileName,
    IN CHAR16     *NextCompName
    );

void
RemoveDirEntries (
    IN FILE_HANDLE *Rfh,
    IN FILE_HANDLE *fh
    );

EFI_STATUS
RemoveFH (
    FILE_HANDLE *fh
    );

EFI_STATUS
FreeUpDirEntries (
    VOLUME_INFO *vi,
    FILE_HANDLE *pfh
    );

EFI_STATUS
FreeUpFHEntries (
    VOLUME_INFO *vi,
    FILE_HANDLE *pfh
    );

EFI_STATUS
RemoveAllDirList(
    FILE_HANDLE *lfh
    );


EFI_STATUS
RemoveDirList (
    FILE_HANDLE    *fh,
    DIR_ENTRY_LIST *Del
    );

BOOLEAN
LfnIsValid (
    CHAR16 *Name
    );

VOID
FixName (
    CHAR16 *Name1,
    CHAR16 *Name2,
    BOOLEAN ShortName
    );

EFI_STATUS
fsAllocateMemory (
    VOLUME_INFO *vi,
    UINT32      Size,
    VOID        **Buffer,
    BOOLEAN     CLEAR
    );

EFI_STATUS
fsDeAllocateMemory (
    VOLUME_INFO *vi,
    void        *Buffer
    );

EFI_STATUS
GetVolumeLabel (
    VOLUME_INFO  *vi,
    IN OUT UINTN *BufferSize,
    OUT VOID     *Buffer
    );

EFI_STATUS
GetfileInfo (
    IN FILE_HANDLE_INSTANCE *fhi,
    IN OUT UINTN            *BufferSize,
    OUT VOID                *Buffer
    );

EFI_STATUS
GetFileInfoFromFH (
    FILE_HANDLE  *Pfh,
    IN OUT UINTN *BufferSize,
    OUT VOID     *Buffer
    );

EFI_STATUS
GetFileInfoFromDel (
    DIR_ENTRY_LIST  *Del,
    IN OUT UINTN    *BufferSize,
    OUT VOID        *Buffer
    );

VOID
NTFSToEfiTime (
    EFI_TIME *EfiTime,
    UINT64   NTFSTime
    );

VOID
EfiToNTFSTime (
    EFI_TIME EfiTime,
    UINT64   *NTFSTime
    );

EFI_STATUS
GetSystemInfo (
    IN VOLUME_INFO           *vi,
    IN OUT UINTN             *BufferSize,
    OUT EFI_FILE_SYSTEM_INFO *Buffer
    );

UINT32
ReturnFreeSpace (
    IN VOLUME_INFO *vi
    );

EFI_STATUS
GetVolumeLabel (
    IN VOLUME_INFO *vi,
    IN OUT UINTN   *BufferSize,
    OUT VOID       *Buffer
    );

BOOLEAN
FindShortName (
    UINT8  *Buffer,
    UINT32 *Offset,
    UINT32 BufferSize
    );

EFI_STATUS
FsReadMedia (
    VOLUME_INFO *vi,
    UINT8       *Buffer,
    UINT64      Sector,
    UINT32      Offset,
    UINT32      ByteCount,
    REGIONS     DataRegion
    );

EFI_STATUS
FsWriteMedia (
    VOLUME_INFO *vi,
    UINT8       *Buffer,
    UINT64      SectorCount,
    UINT32      Offset,
    UINT32      ByteCount,
    REGIONS     DataRegion
    );

void
HandleDiskIoError (
    VOLUME_INFO *vi,
    EFI_STATUS  Status

    );

ExtractShortFileName (
    VOLUME_INFO         *Vi,
    CHAR16              *lfn,
    UINT8               *Buffer
    );

EFI_STATUS
CheckCached (
    VOLUME_INFO  *vi,
    UINT64       AbsoluteOffset,
    UINT32       ByteCount,
    UINT8        **cBuffer,
    UINT32       *cBufferLength,
    CACHE_HEADER **ch,
    REGIONS      DataRegion
    );

BOOLEAN
IsCacheHit (
    VOLUME_INFO  *vi,
    REGIONS      DataRegion,
    UINT64       AbsoluteOffset,
    UINT32       ByteCount,
    UINT8        **cBuffer,
    UINT32       *cBufferLength,
    CACHE_HEADER **ch1
    );

void
CreateSpaceforCache (
    VOLUME_INFO  *vi,
    UINT32       ByteCount,
    REGIONS      DataRegion,
    CACHE_HEADER **ch1
    );

EFI_STATUS
ReadClusterOfMFT (
    IN FILE_HANDLE *fh,
    IN UINT32      Cluster,
    UINT8          *Buffer,
    UINT32         *BufferSize
    );

EFI_STATUS
FindFreeClusters (
    IN VOLUME_INFO *Vol,
    IN UINT32      From,
    IN UINT32      To,
    OUT UINT32     *StartCluster,
    IN OUT UINT32  *FreeClusters,
    OUT UINT32     *TotalFree
    );

EFI_STATUS
FindNextFreeCluster (
    IN VOLUME_INFO *Vol,
    OUT UINT32     *Cluster
    );

EFI_STATUS
GetFreeClusterCount (
    IN VOLUME_INFO *Vol,
    OUT UINT32     *FreeClustersCount,
    OUT UINT32     *FreeClusterBlock,
    OUT UINT32     *FreeBlockCount
    );

EFI_STATUS
GetClusterCount (
    IN VOLUME_INFO *Vol,
    IN OUT UINT32  Cluster
    );

int
Wcscmpcaseinsensitive(
    CHAR16 *string1,
    CHAR16 *string2
    );

EFI_STATUS
ReadFromFile (
    IN FILE_HANDLE_INSTANCE *File,
    IN OUT VOID             *Buffer,
    IN OUT UINT32           *Size,
    IN REGIONS              DataRegion
    );

EFI_STATUS
AllocateFirstCluster (
    IN FILE_HANDLE_INSTANCE *File,
    OUT UINT32              *Cluster
    );

EFI_STATUS
AllocateAndLinkClusters (
    IN VOLUME_INFO *Vol,
    IN UINT32      BeginCluster,
    IN UINT32      Size,
    OUT UINT32     *FirstCluster,
    OUT UINT32     *ClusterCount
    );

static int AllocateAndLinkNewCluster (
    VOLUME_INFO *Vol,
    IN UINT32   PrevCluster,
    OUT UINT32  *Cluster
    );

EFI_STATUS
GetContiguousClusters (
    IN VOLUME_INFO *Vol,
    IN UINT32      Size,
    IN OUT UINT32  Cluster,
    OUT UINT32     *LastCluster,
    OUT UINT32     *Count
    );

VOID
GetAbsSectorInfo (
    IN VOLUME_INFO *Vol,
    IN UINT32      Cluster,
    IN UINT32      ClusterOffset,
    OUT UINT32     *Sector,
    OUT UINT32     *SectorOffset
    );

EFI_STATUS
GetClusterPosition (
    IN FILE_HANDLE_INSTANCE *File,
    IN UINT64               Position,
    OUT UINT64              *Cluster,
    OUT UINT64              *ClusterOffset
    );

UINT32
GetEocValue (
    IN VOLUME_INFO *Vol
    );

EFI_STATUS
ValidateCurrentStatus (
    FILE_HANDLE_INSTANCE *fhi
    );

VOID
GetSectorAddressDir (
    FILE_HANDLE *fh,
    UINT32      Cluster,
    UINT32      ClusterOffset,
    UINT32      *Sector,
    UINT32      *SectorOffset
    );

EFI_STATUS
UpdateAccAndWriteTime (
    IN OUT DIR_ENTRY_LIST *del
    );

EFI_STATUS
UpdateAccDate(
    IN OUT DIR_ENTRY_LIST *del
    );

BOOLEAN
UpdateAccDateRequired (
    IN OUT DIR_ENTRY_LIST *del
    );

EFI_STATUS
GenShortFilename (
    IN VOLUME_INFO *Vol,
    IN CHAR16      *InPath,
    OUT CHAR8      *ShortName
    );

EFI_STATUS
FreeUpResources (
    VOLUME_INFO *vi,
    EFI_STATUS  Status
    );

EFI_STATUS
UnloadNTFSDriver (
    IN EFI_HANDLE ImageHandle
    );

void
Zeromemory (
    void   *Buffer,
    UINT32 Size
    );

EFI_STATUS
InternalMemAllocate (
    VOLUME_INFO *vi,
    UINT32      Size,
    VOID        **Buffer,
    BOOLEAN     CLEAR
    );

EFI_STATUS
AddBlocktoInternalMemManager (
    VOLUME_INFO *vi
    );

EFI_STATUS
GetMemory (
    MEM_HEADER *Mem,
    UINT32     Blocks,
    VOID       **Buffer,
    BOOLEAN    CLEAR
    );

EFI_STATUS
FreeMemory (
    VOLUME_INFO *vi,
    VOID        *Buffer
    );

VOID
GetTempBuffer (
    VOLUME_INFO *vi,
    VOID        **Buffer
    );

VOID
ReleaseTempBuffer (
    VOLUME_INFO *vi
    );

EFI_STATUS GetRunListElementData(
    UINT8   **pRunList,
    UINT64  *ClusterCount,
    UINT64  *Cluster,
    BOOLEAN UpdateList
    );

EFI_STATUS
GetNameAndRunlist (
    UINT8 *Buffer,
    UINT8 **pName,
    UINT8 **pRunList
    );

EFI_STATUS
GetFrAttribute (
    UINT8 *BufferIn,
    UINT8 AttributeNumber,
    UINT8 **BufferOut
    );

EFI_STATUS
GetFileRecord (
    VOLUME_INFO *Vi,
    UINT64      MFTRecordNo,
    UINT8       *Buffer,
    ...
    );

EFI_STATUS
CheckFlags (
    DIR_ENTRY_NTFS *DirectoryEntry
    );

UINT64
ClusterToSector (
    IN VOLUME_INFO *Vol,
    IN UINT64      Cluster
    );

UINT64
Div64x64x64 (
    UINT64  Dividend,
    UINT64  Divisor,
    ...
    );

EFI_TPL
FindNextTpl (
    EFI_TPL     TplLevel
    );

VOID EfiDebugPrint (
    IN UINTN ErrorLevel,
    IN CHAR8 *Format,
    ...
    );

EFI_STATUS
ApplyFixup (
    VOLUME_INFO *vi,
    UINT8       *Buffer
);

/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif

#endif

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
