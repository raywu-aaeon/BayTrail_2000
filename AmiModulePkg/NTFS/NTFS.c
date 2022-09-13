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
// Name: NTFS.c
//
// Description: Installs File System Protocol for an NTFS volume
//
//<AMI_FHDR_END>
//**********************************************************************

//----------------------------------------------------------------------

#include "NTFS.h"

//----------------------------------------------------------------------

EFI_GUID gNTFSBusDriverBindingProtocolGuid  = EFI_DRIVER_BINDING_PROTOCOL_GUID;
EFI_GUID guidLoadedImage                    = EFI_LOADED_IMAGE_PROTOCOL_GUID;
#ifndef EFI_COMPONENT_NAME2_PROTOCOL_GUID //old Core
EFI_GUID gComponentNameProtocolGuid         = EFI_COMPONENT_NAME_PROTOCOL_GUID;
#else
EFI_GUID gComponentNameProtocolGuid         = EFI_COMPONENT_NAME2_PROTOCOL_GUID;
#endif
EFI_GUID gEfiBlockIoProtocolGuid            = EFI_BLOCK_IO_PROTOCOL_GUID;
//EFI_GUID gEfiDiskIoProtocolGuid             = EFI_DISK_IO_PROTOCOL_GUID;
//EFI_GUID gEfiGlobalVariableGuid             = EFI_GLOBAL_VARIABLE; //AaeonCore_000_RayWu, REMOVE 2015/02/11
#ifndef EFI_UNICODE_COLLATION2_PROTOCOL_GUID //old Core
EFI_GUID gEfiUnicodeCollationProtocolGuid   = EFI_UNICODE_COLLATION_PROTOCOL_GUID;
#else
//EFI_GUID gEfiUnicodeCollationProtocolGuid   = EFI_UNICODE_COLLATION2_PROTOCOL_GUID;
#endif
EFI_GUID gEfiFileInfoGuid                   = EFI_FILE_INFO_ID;
EFI_GUID gEfiFileSystemInfoGuid             = EFI_FILE_SYSTEM_INFO_ID;
EFI_GUID gEfiFileSystemVolumeLabelGuid      = EFI_FILE_SYSTEM_VOLUME_LABEL_ID;

EFI_UNICODE_COLLATION_PROTOCOL     *gNTFSUnicodeCollationInterface = NULL;

extern EFI_COMPONENT_NAME_PROTOCOL gNTFSDriverNames;
extern EFI_GUID                    guidFS;

EFI_DRIVER_BINDING_PROTOCOL        gNTFSDriverBinding = {
    NTFSSupported,
    NTFSStart,
    NTFSStop,
    NTFS_DRIVER_VERSION,   // version
    NULL,                  // ImageHandle
    NULL                   // DriverBindingHandle
};

#if AMI_FILE_SYSTEM_ID_PROTOCOL_SUPPORT
static UINT64 ReturnFileSystemId(
    VOID
) { return AMI_FILE_SYSTEM_NTFS; }

static AMI_FILE_SYSTEM_ID_PROTOCOL gFileSystemId = {
    ReturnFileSystemId
};
#endif

//----------------------------------------------------------------------


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       NTFSEntryPoint
//
// Description:     Installs gNTFSDriverBinding protocol
//
// Parameters:      EFI_HANDLE ImageHandle - Image handle for this driver
//                                           image
//                  EFI_SYSTEM_TABLE *SystemTable - pointer to the EFI
//                                                  system table
//
// Return value:    EFI_STATUS - Status of the operation
//
// Modified:
//
// Referral(s):
//
// NOTE(S):
//  Here is the control flow of this function:
//  1. Initialize Ami Lib.
//  2. Install Driver Binding Protocol
//  3. Return EFI_SUCCESS.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS NTFSEntryPoint(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
    )
{
    EFI_STATUS                Status;
    EFI_LOADED_IMAGE_PROTOCOL *ThisImage;

    gNTFSDriverBinding.DriverBindingHandle=ImageHandle;
    gNTFSDriverBinding.ImageHandle=ImageHandle;

                  InitAmiLib(ImageHandle, SystemTable);
    Status = pBS->InstallMultipleProtocolInterfaces(
        &ImageHandle,
        &gNTFSBusDriverBindingProtocolGuid,&gNTFSDriverBinding,
        &gComponentNameProtocolGuid, &gNTFSDriverNames,
        NULL
        );

    Status = pBS->OpenProtocol (
        ImageHandle,
        &guidLoadedImage,
        (VOID **)&ThisImage,
        ImageHandle,
        ImageHandle,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    if (EFI_ERROR (Status)) return Status;

    ThisImage->Unload = UnloadNTFSDriver;
    return Status;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       UnloadNTFSDriver
//
// Description:     Unloads this driver
//
// Parameters:      EFI_HANDLE ImageHandle - Image handle for this driver
//                                           image
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
UnloadNTFSDriver (
    IN EFI_HANDLE ImageHandle
    )
{
    EFI_HANDLE *Buffer;
    UINTN      NoOfHandles, i;
    EFI_STATUS Status;

    Status = pBS->LocateHandleBuffer (AllHandles, NULL, NULL,
                                      &NoOfHandles, &Buffer);

    if (EFI_ERROR(Status)) return Status;

    for (i = 0; i < NoOfHandles; i++) {
        pBS->DisconnectController(Buffer[i], ImageHandle, NULL);
    }

    if (Buffer) pBS->FreePool(Buffer);

    Status = pBS->UninstallMultipleProtocolInterfaces(
        ImageHandle,
        &gNTFSBusDriverBindingProtocolGuid,&gNTFSDriverBinding,
        &gComponentNameProtocolGuid, &gNTFSDriverNames,
        NULL);

    return Status;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       NTFSSupported
//
// Description:     Checks whether Simple File System Protocol can be
//                  installed on the controller or not.
//
// Parameters:      EFI_DRIVER_BINDING_PROTOCOL *This - Pointer to this
//                                               instance of driver binding
//                                               protocol
//                  EFI_HANDLE Controller - Handle for this controller
//                  EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
//
// Return value:    EFI_STATUS - Status of the operation
//
// Modified:
//
// Referral(s):
//
//  NOTE(S):        DiskIO and BlockIO should have been installed on the
//                  controller.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
NTFSSupported(
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE                  Controller,
    IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
    )
{

    EFI_STATUS            Status;
    EFI_DISK_IO_PROTOCOL  *DiskIo;
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
//
// Check if DiskIO can be opened by EFI_OPEN_PROTOCOL_BY_DRIVER
//    
    Status = pBS->OpenProtocol( Controller,
                                &gEfiDiskIoProtocolGuid,
                                (VOID **)&DiskIo,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_BY_DRIVER );

    if (EFI_ERROR (Status)) return EFI_UNSUPPORTED;
//
// Close DiskIO Protocol
//    
    Status = pBS->CloseProtocol (
        Controller,
        &gEfiDiskIoProtocolGuid,
        This->DriverBindingHandle,
        Controller);
//
// Check if BlockIO can be opened by EFI_OPEN_PROTOCOL_GET_PROTOCOL
//    
    Status = pBS->OpenProtocol( Controller,
                                &gEfiBlockIoProtocolGuid,
                                (VOID **)&BlkIo,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    if (EFI_ERROR (Status)) return EFI_UNSUPPORTED;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       NTFSStart
//
// Description:     Installs Simple File Ssystem Protocol
//
// Parameters:      EFI_DRIVER_BINDING_PROTOCOL *This - Pointer to this
//                                               instance of driver binding
//                                               protocol
//                  EFI_HANDLE Controller - Handle for this controller
//                  EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
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
NTFSStart (
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE                  Controller,
    IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
    )
{

    EFI_STATUS            Status;
    EFI_DISK_IO_PROTOCOL  *DiskIo;
    EFI_BLOCK_IO_PROTOCOL *BlkIo;
//
// Check if DiskIO can be opened by EFI_OPEN_PROTOCOL_BY_DRIVER
//    
    Status = pBS->OpenProtocol( Controller,
                                &gEfiDiskIoProtocolGuid,
                                (VOID **)&DiskIo,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_BY_DRIVER );

    if (EFI_ERROR (Status)) return EFI_UNSUPPORTED;
//
// Check if BlockIO can be opened by EFI_OPEN_PROTOCOL_GET_PROTOCOL
//    
    Status = pBS->OpenProtocol( Controller,
                                &gEfiBlockIoProtocolGuid,
                                (VOID **)&BlkIo,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    if (EFI_ERROR (Status)) { // Close DiskIO Protocol
        Status = pBS->CloseProtocol (
            Controller,
            &gEfiDiskIoProtocolGuid,
            This->DriverBindingHandle,
            Controller);
        return EFI_UNSUPPORTED;
    }
//
// Get the UnicodeCollation driver
//
    Status = NTFSInitUnicodeCollation (This, &gNTFSUnicodeCollationInterface);

    if (EFI_ERROR (Status)) { // Close DiskIO Protocol
        Status = pBS->CloseProtocol (
            Controller,
            &gEfiDiskIoProtocolGuid,
            This->DriverBindingHandle,
            Controller);
        return EFI_UNSUPPORTED;
    }

    Status = DetectInstallVolume(DiskIo, BlkIo, Controller);

    if (EFI_ERROR(Status)) { // Close DiskIO Protocol
        Status = pBS->CloseProtocol (
            Controller,
            &gEfiDiskIoProtocolGuid,
            This->DriverBindingHandle,
            Controller);
        return Status;
    }


    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       NTFSStop
//
// Description:     Uninstall Simple File system Protocol
//
// Parameters:      EFI_DRIVER_BINDING_PROTOCOL *This - Pointer to this
//                                               instance of driver binding
//                                               protocol
//                  EFI_HANDLE Controller - Handle for this controller
//                  EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
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
NTFSStop (
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE                  Controller,
    IN UINTN                       NumberOfChildren,
    IN EFI_HANDLE                  *ChildHandleBuffer
    )
{

    EFI_STATUS                      Status;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *NTFS;
    VOLUME_INFO                     *vi;

    Status = pBS->OpenProtocol ( Controller,
                                 &guidFS,
                                 &NTFS,
                                 This->DriverBindingHandle,
                                 Controller,
                                 EFI_OPEN_PROTOCOL_GET_PROTOCOL);

    if (EFI_ERROR(Status)) return EFI_UNSUPPORTED;

    vi = (VOLUME_INFO *)NTFS;

    if (NumberOfChildren) return EFI_UNSUPPORTED;

    Status = pBS->UninstallMultipleProtocolInterfaces(
        (vi->VolumeHandle),
        &guidFS, &(vi->SimpleFileSystemProtocol),
        NULL);

    if (EFI_ERROR(Status)) return Status;
//
// Close DISK_IO Protocol
//    
    Status = pBS->CloseProtocol (
        Controller,
        &gEfiDiskIoProtocolGuid,
        This->DriverBindingHandle,
        Controller);

    if (EFI_ERROR(Status)) return Status;

    Status = fsDeAllocateMemory(vi, vi->TempBuffer);

    Status = FreeUpResources (vi, EFI_NO_MEDIA);

    Status = pBS->FreePool(vi->CacheBuffer);

    Status = pBS->FreePool(vi);

    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       DetectInstallVolume
//
// Description:     Detects whether a valid volume is present or not
//
// Parameters:      EFI_DISK_IO_PROTOCOL *DiskIo - Pointer to disk IO
//                                                 structure
// Parameters:      EFI_BLOCK_IO_PROTOCOL *DiskIo - Pointer to boock IO
//                                                  structure
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
DetectInstallVolume(
    IN EFI_DISK_IO_PROTOCOL  *DiskIo,
    IN EFI_BLOCK_IO_PROTOCOL *BlkIo,
    IN EFI_HANDLE            Handle
    )
{

    EFI_STATUS   Status;
    VOLUME_INFO  *vi;
    UINT32       TempBufferSize, i, MFT_Cache_Blocks, Dir_Cache_Blocks, \
                 TotalCacheMemory, TotalCacheList;
    UINTN        CacheAddress, CacheListAddress;
    CACHE_HEADER *ch;

    Status = pBS->AllocatePool (EfiBootServicesData,
                                sizeof(VOLUME_INFO),
                                (VOID**)&vi);
//
// No need to close IDE_CONTROLLER_PROTOCOL
//    
    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;

    Zeromemory (vi, sizeof(VOLUME_INFO));

    vi->DiskIo = DiskIo;
    vi->BlkIo = BlkIo;
    vi->MediaID = vi->BlkIo->Media->MediaId;
    Status = DetectVolume(vi, FALSE);

    if (EFI_ERROR(Status)) {
        pBS->FreePool(vi);
        return Status;
    }

    vi->UnicodeCollationInterface = gNTFSUnicodeCollationInterface;

    vi->SimpleFileSystemProtocol.OpenVolume = OpenVolume;
    vi->SimpleFileSystemProtocol.Revision = EFI_FILE_PROTOCOL_REVISION;
    vi->VolumeHandle = Handle;
    DListInit(&(vi->OpenFHs));
    DListInit(&(vi->OpenFIs));
    DListInit(&(vi->CacheList));
//
// Allocate TempBuffer as max cluster size * 2. In NTFS, the max
// cluster size is 4096    
//    
    TempBufferSize = 4096;

    if ((vi->BytesPerCluster * 2) > TempBufferSize)
        TempBufferSize = vi->BytesPerCluster *2;

    Status = fsAllocateMemory(vi, TempBufferSize,
                              (VOID**)&vi->TempBuffer, FALSE);
    vi->TempBufferSize = TempBufferSize;
//
// Init the cache Buffers
//    
    MFT_Cache_Blocks = vi->AllowedMFTCacheSize / vi->BytesPerCluster;
    Dir_Cache_Blocks = vi->AllowedDirCacheSize / vi->BytesPerCluster;

    TotalCacheMemory = MFT_Cache_Blocks * vi->BytesPerCluster +
                       Dir_Cache_Blocks * vi->BytesPerCluster;

    TotalCacheList = MFT_Cache_Blocks * sizeof (CACHE_HEADER) +
                     Dir_Cache_Blocks * sizeof (CACHE_HEADER);

    Status = pBS->AllocatePool (EfiBootServicesData,
                                TotalCacheMemory + TotalCacheList,
                                (VOID**)&vi->CacheBuffer);

    CacheAddress = (UINTN) (vi->CacheBuffer);
    CacheListAddress = (UINTN) (vi->CacheBuffer) + TotalCacheMemory;

    for (i = 0; i < MFT_Cache_Blocks + Dir_Cache_Blocks; i++) {
        ch = (CACHE_HEADER *)CacheListAddress;
        ch->AbsoluteOffset = 0;
        ch->AbsoluteOffsetEnd = 0;
        ch->Buffer = (UINT8 *) CacheAddress;
        ch->DIRTY_FLAG = FALSE;

        if ( i < MFT_Cache_Blocks) ch->DataRegion = MFT_REGION;

        else ch->DataRegion = DIRECTORY_REGION;

        DListAdd(&(vi->CacheList), &(ch->CacheLink));
        CacheAddress += vi->BytesPerCluster;
        CacheListAddress += sizeof (CACHE_HEADER);
    }

    Status = pBS->InstallMultipleProtocolInterfaces (
        &(vi->VolumeHandle),
        &guidFS, &(vi->SimpleFileSystemProtocol),
#if AMI_FILE_SYSTEM_ID_PROTOCOL_SUPPORT
        &gAmiFileSystemIdProtocolGuid, &gFileSystemId,
#endif
        NULL);

    if (EFI_ERROR(Status)) {
        pBS->FreePool(vi);
        return EFI_UNSUPPORTED;
    }

    return EFI_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       DetectVolume
//
// Description:     Detects the presence of a valid NTFS volume
//
// Parameters:      VOLUME_INFO *vi - Pointer to the volume info structure
//                  BOOLEAN StatusCheck - Only reads MBR if true
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
DetectVolume (
    VOLUME_INFO *vi,
    BOOLEAN     StatusCheck
    )
{

    EFI_STATUS Status;
    VOLUME_BPB *Bpb;
    UINT8      *Buffer;
    BOOLEAN    VALID_NTFS = TRUE;

    Status = fsAllocateMemory(vi, vi->BlkIo->Media->BlockSize,
                              (VOID**)&Buffer, FALSE);

    if (EFI_ERROR(Status)) return EFI_OUT_OF_RESOURCES;
//
// Read Logical Sector 0
//    
    Status= vi->DiskIo->ReadDisk (vi->DiskIo, vi->MediaID, 0, 
                                  vi->BlkIo->Media->BlockSize, Buffer);

    if (EFI_ERROR(Status) || StatusCheck) {
        fsDeAllocateMemory(vi, Buffer);
        vi->VolumeStatus = Status;
        return Status;
    }

    Bpb = (VOLUME_BPB *)Buffer;
//
// Check for 0x55aa at offset 510.
//    
    if ( *(UINT16 *) ((UINT8 *)Buffer + 510) != 0xaa55) {
        fsDeAllocateMemory(vi, Buffer);
        return EFI_UNSUPPORTED;
    }
//
// Check for valid jmp instruction
//
    if (Buffer[0] != 0xEB && Buffer[0] != 0xE9 ) VALID_NTFS = FALSE;
//
// Check for valid Bytes per Sector
//    
    switch (Bpb->BPB_BytePerSec) {
    case 512:
        vi->BytesPerSecPowerof2 = 9;
        break;
    case 1024:
        vi->BytesPerSecPowerof2 = 10;
        break;
    case 2048:
        vi->BytesPerSecPowerof2 = 11;
        break;
    case 4096:
        vi->BytesPerSecPowerof2 = 12;
        break;
    default:
        VALID_NTFS = FALSE;
    }
//
// Check for Valid Sectors per cluster
//
    if (Bpb->BPB_SecPerClus != 1 && Bpb->BPB_SecPerClus != 2 && \
        Bpb->BPB_SecPerClus != 4 && Bpb->BPB_SecPerClus != 8 && \
        Bpb->BPB_SecPerClus != 16 && Bpb->BPB_SecPerClus != 32 && \
        Bpb->BPB_SecPerClus != 64 && Bpb->BPB_SecPerClus != 128)
        VALID_NTFS = FALSE;
//
// Check for Reserved Sector Count
//        
    if (Bpb->BPB_RsvdSecCnt != 0) VALID_NTFS = FALSE;
//
// TotSec16 and TotSec32 must be zero, and TotSec64 must be non-zero
//    
    if ((Bpb->BPB_TotSec16 != 0) || (Bpb->BPB_TotSec32 != 0)) 
        VALID_NTFS = FALSE;
    if (Bpb->BPB_TotSec64 == 0) VALID_NTFS = FALSE;
//
// Check for valid OEM name ("NTFS")
//    
	if ( *((UINT32*)Bpb->BS_OEMName) != EFI_SIGN_32 ('N', 'T', 'F', 'S') ) \
		VALID_NTFS = FALSE;

    if (!VALID_NTFS) goto error_check;

    vi->TotalSectors = Bpb->BPB_TotSec64;
//
// If we need this, we'll have to calculate it later
//    
    vi->DataSectors = vi->TotalSectors - (UINT64)Bpb->BPB_RsvdSecCnt;

    vi->CountOfDataClusters = Div64(vi->DataSectors,
                                    (UINTN)Bpb->BPB_SecPerClus, NULL);
//
// There could be a few sectors at the "end" of the disk that will not 
// completely fill a cluster. Make sure these are not counted in the
// DataSectors.
//    
    vi->DataSectors = Mul64(vi->CountOfDataClusters, Bpb->BPB_SecPerClus);
//
// Copy Sector Zero Contends to VOLUME_BPB in vi
//    
    pBS->CopyMem (&(vi->VolumeBPB), Buffer, sizeof(VOLUME_BPB));

    vi->BytesPerCluster = (UINT32)vi->VolumeBPB.BPB_SecPerClus << \
                            vi->BytesPerSecPowerof2;
//
// Update CacheSize
//    
    vi->AllowedMFTCacheSize = MFT_CACHE_SIZE;

    if (MFT_CACHE_SIZE < vi->BytesPerCluster << 1)
        vi->AllowedMFTCacheSize = vi->BytesPerCluster << 1;

    vi->AllowedDirCacheSize = DIR_CACHE_SIZE;

    if (DIR_CACHE_SIZE < vi->BytesPerCluster << 1)
        vi->AllowedDirCacheSize = vi->BytesPerCluster << 1;

error_check:
    fsDeAllocateMemory(vi, Buffer);

    if (VALID_NTFS == FALSE) {
        vi->VolumeStatus = EFI_UNSUPPORTED;
        return EFI_UNSUPPORTED;
    }

    vi->VolumeStatus = EFI_SUCCESS;
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:   NTFSInitUnicodeCollation
//
// Description: Find the language Interface
//
// Parameters:  EFI_DRIVER_BINDING_PROTOCOL *This - Pointer to this
//                                           instance of driver binding
//                                           protocol
//              UINT8 *LandCode - Pointer to the language code
//              EFI_UNICODE_COLLATION_PROTOCOL *gNTFSUnicodeCollationInterface
//                                             - Pointer to pointer to unicode
//                                             collation interfact structure
//
// Return value: EFI_STATUS - Status of the operation
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
NTFSInitUnicodeCollation (
    IN EFI_DRIVER_BINDING_PROTOCOL        *This,
    IN OUT EFI_UNICODE_COLLATION_PROTOCOL **gNTFSUnicodeCollationInterface
    )
{
    EFI_STATUS Status;

    if (*gNTFSUnicodeCollationInterface != NULL) return EFI_SUCCESS;
//
// Locate all Unicode Collation Protocol
//    
    Status = pBS->LocateProtocol(
        &gEfiUnicodeCollationProtocolGuid, NULL,
        gNTFSUnicodeCollationInterface
        );
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:     Zeromemory
//
// Description:   Zeros the contents of a specified memory location
//
// Parameters:    VOID *Buffer - location of memory to zero
//                UINT32 Size - Size in bytes to zero
//
// Return Value:  none
//
// Modified:
//
// Referral(s):
//
// NOTE(S):       For testing under Aptio and Alaska,
//                This routine has been copied.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

void
Zeromemory (
    void   *Buffer,
    UINT32 Size
    )
{
    UINT8 *Ptr;
    Ptr = Buffer;

    while (Size--) {
        *(Ptr++) = 0;
    }

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
