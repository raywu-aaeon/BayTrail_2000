
#include "FotaDxe.h"


EFI_GUID BiosCapsuleFromAosGuid = { 0xCD193840, 0x2881, 0x9567, { 0x39, 0x28, 0x38, 0xc5, 0x97, 0x53, 0x49, 0x77 }};

extern EFI_GUID gBdsAllDriversConnectedProtocolGuid;
extern EFI_GUID gAosFirmwareClassGuid;

extern EFI_GUID gEfiPartTypeSystemPartGuid;  //<EIP153486+>

EFI_EVENT             mFotaEvent = NULL;
EFI_PHYSICAL_ADDRESS  mCapsuleAddress = 0xFFFFFFFF;

//<EIP153486+> >>>
VOID
EFIAPI
FotaAllDriversConnected2 (
    IN      EFI_EVENT                 Event,
    IN      VOID                      *Context
  );

EFI_STATUS
FindUpdateImageOnDevice (
    VOID
    );

EFI_STATUS
SaveUpdateImageOnDevice (
    VOID
    );
//<EIP153486+> <<<


EFI_STATUS
FindCapsuleHob (VOID)
{
    VOID                      *FirstHob;
    EFI_HOB_UEFI_CAPSULE      *CapsuleHob;
    EFI_PHYSICAL_ADDRESS      CapsuleAddress;
    EFI_CAPSULE_HEADER        *FwCapsuleVolume;
    
    FirstHob = GetFirstHob (EFI_HOB_TYPE_UEFI_CAPSULE);
    if (FirstHob == NULL) return EFI_UNSUPPORTED;

    CapsuleHob = (EFI_HOB_UEFI_CAPSULE*)FirstHob;
    
    CapsuleAddress  = CapsuleHob->BaseAddress;
    FwCapsuleVolume = (EFI_CAPSULE_HEADER*) CapsuleAddress;

    if (CompareGuid(&FwCapsuleVolume->CapsuleGuid, &gAosFirmwareClassGuid)) {
      mCapsuleAddress = CapsuleAddress;
      DEBUG((EFI_D_ERROR, "AOS Capsule found in Capsule Volume Hob %x \r\n", mCapsuleAddress));
      return EFI_SUCCESS;
    }

    return EFI_UNSUPPORTED;
}

EFI_STATUS
GetValidIFWI(void **pBuffer, UINT64 *pSize)
{
    EFI_STATUS                  Status;
    EFI_GUID                    CapsuleGuid;
    EFI_CAPSULE_HEADER          *CapsuleHeader = NULL;
    EFI_CAPSULE_HEADER          *CapsuleHeaderArray[1];
    EFI_PHYSICAL_ADDRESS        sc = (EFI_PHYSICAL_ADDRESS)0;

    if (mCapsuleAddress == 0xFFFFFFFF) {
      DEBUG((EFI_D_ERROR, "AOS Capsule is not found in Capsule Volume Hob.\r\n"));
      return EFI_SUCCESS;
    }

    CapsuleGuid = gAosFirmwareClassGuid;
    CapsuleHeader = (EFI_CAPSULE_HEADER *)mCapsuleAddress;
    CapsuleHeader->Flags = CAPSULE_FLAGS_INITIATE_RESET | CAPSULE_FLAGS_PERSIST_ACROSS_RESET | CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE;

    CapsuleHeaderArray[0] = CapsuleHeader;

    DEBUG((EFI_D_ERROR, "dispatch the capsule!! \r\n"));
    Status = gRT->UpdateCapsule(CapsuleHeaderArray,1,sc);
            
    // We should never reach here.
    FreePool(CapsuleHeader);
    return Status;
}

VOID
EFIAPI
FotaAllDriversConnected (
    IN      EFI_EVENT                 Event,
    IN      VOID                      *Context
  )
{
    EFI_STATUS    Status;
    VOID          *FileBuffer;
    UINT64        FileSize;
    
//    if (mFotaEvent != Event)
//      return;

    FileSize = 0;
    FileBuffer = NULL;

    DEBUG((EFI_D_INFO,"[FotaAllDriversConnected] GetValidIFWI() Start!!\n"));
    SaveUpdateImageOnDevice();  //<EIP153486+>
    Status = GetValidIFWI(&FileBuffer, &FileSize);

    DEBUG((EFI_D_INFO,"[FotaAllDriversConnected] FOTA() Exit!!\n"));
    gBS->CloseEvent (Event);
}

EFI_STATUS
EFIAPI
FotaDxeEntry (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
    VOID          *FotaRegistration;
    EFI_STATUS    Status;
    UINTN         VarSize;
    UINTN         AosCapsule = 0;
  
    DEBUG((EFI_D_INFO, "Fota Dxe entry!!\n"));

    if (FindCapsuleHob() == EFI_SUCCESS) {
      mFotaEvent = EfiCreateProtocolNotifyEvent (
                    &gBdsAllDriversConnectedProtocolGuid,
                    TPL_CALLBACK,
                    FotaAllDriversConnected,
                    NULL,
                    &FotaRegistration
                    );
    } else {
      VarSize = sizeof(AosCapsule);
      Status = gRT->GetVariable (
                      L"CapsuleFromAos",
                      &BiosCapsuleFromAosGuid,
                      NULL,
                      &VarSize,
                      &AosCapsule
                      );
      if (!EFI_ERROR(Status)) {
        if ((AosCapsule == 1) || (AosCapsule == 2)) {
          if (AosCapsule == 1) {
            //
            // Need to do AOS Capsule function, but failed to find AOS Capsule.
            // Need to set AosCapsule to 2, or the system would boot to Recovery
            // menu in ReFlash.c.
            //
            AosCapsule = 2;
            DEBUG((EFI_D_ERROR, "Try to do AOS Capsule function, but failed to find AOS Capsule.\r\n"));
          } else {
            //
            // Failed to do AOS Capsule function in last boot time.
            // Need to set AosCapsule to 0, or the system would not boot to
            // Recovery menu in ReFlash.c while do other Capsule recovery.
            //
            AosCapsule = 0;
            DEBUG((EFI_D_ERROR, "Clear AosCapsule variable.\r\n"));
          }
          gRT->SetVariable (
                L"CapsuleFromAos",
                &BiosCapsuleFromAosGuid,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                sizeof(AosCapsule),
                &AosCapsule
                );
        }
      }
//<EIP153486+> >>>
      mFotaEvent = EfiCreateProtocolNotifyEvent (
                    &gBdsAllDriversConnectedProtocolGuid,
                    TPL_CALLBACK,
                    FotaAllDriversConnected2,
                    NULL,
                    &FotaRegistration
                    );
//<EIP153486+> <<<
    }

    return EFI_SUCCESS;
}


//<EIP153486+> >>>
#define CAP_UPDATE_BINARY L"BIOSUpdate.FV"

typedef struct {
  CHAR16    *FileName;
  CHAR16    *String;
} FILE_NAME_TO_STRING;

FILE_NAME_TO_STRING mFlashFileNames[] = {
  { CAP_UPDATE_BINARY }
};

typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           PciDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_PCI_DEVICE_PATH;

PLATFORM_PCI_DEVICE_PATH gEmmcDevPath0 = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE (0x00, 0x10),
  gEndEntire
};

PLATFORM_PCI_DEVICE_PATH gEmmcDevPath1 = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE (0x00, 0x17),
  gEndEntire
};

PLATFORM_PCI_DEVICE_PATH gSataDevPath0 = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE (0x00, 0x13),
  gEndEntire
};

EFI_STATUS
SaveUpdateImageOnDevice (
    VOID
    )
{
    EFI_STATUS                          Status;
    UINTN                               HandleArrayCount;
    EFI_HANDLE                          *HandleArray;
    DXE_PCH_PLATFORM_POLICY_PROTOCOL    *DxePlatformPchPolicy;
    BOOLEAN                             IsEmmc45 = FALSE;
    EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
    UINT32                              Index;
    UINTN                               CompareResult = 0;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL     *Fs;
    EFI_FILE                            *Root;
    EFI_FILE                            *FileHandle;
    UINTN                               FileSize = 0;
    VOID                                *FileBuffer  = NULL;
    EFI_CAPSULE_HEADER                  *CapsuleHeader = NULL;

    Status = gBS->LocateHandleBuffer( ByProtocol, &gEfiPartTypeSystemPartGuid, //&gEfiSimpleFileSystemProtocolGuid,
                                      NULL, &HandleArrayCount, &HandleArray );
    if( EFI_ERROR(Status) )
        return Status;

    Status  = gBS->LocateProtocol( &gDxePchPlatformPolicyProtocolGuid, NULL,
                                   (VOID **)&DxePlatformPchPolicy);
    if( !EFI_ERROR(Status) )
    {
        if ( DxePlatformPchPolicy->SccConfig->eMMCEnabled )
        {
            IsEmmc45 = FALSE;
        }
        else if( DxePlatformPchPolicy->SccConfig->eMMC45Enabled )
        {
            IsEmmc45 = TRUE;
        }
        else
        {
            IsEmmc45 = FALSE;
        }
    }
        
    Status = EFI_NOT_FOUND;

    for( Index = 0; Index < HandleArrayCount; Index++ )
    {
        Status = gBS->HandleProtocol( HandleArray[Index],
                                      &gEfiDevicePathProtocolGuid,
                                      (VOID **)&DevicePath );
        if( EFI_ERROR(Status) )
            continue;

        if(IsEmmc45)
        {
            CompareResult = CompareMem( DevicePath, &gEmmcDevPath1,
                                        sizeof(gEmmcDevPath1) - 4 );
        }
        else
        {
            CompareResult = CompareMem( DevicePath, &gEmmcDevPath0,
                                        sizeof(gEmmcDevPath0) - 4 );
        }
        
        if( CompareResult )
        {
            CompareResult = CompareMem( DevicePath, &gSataDevPath0,
                                        sizeof(gSataDevPath0) - 4 );
            if( CompareResult )
                continue;
        }
    
        Status = gBS->HandleProtocol( HandleArray[Index],
                                      &gEfiSimpleFileSystemProtocolGuid,
                                      (VOID**)&Fs );
        if( EFI_ERROR(Status) )
            continue;

        Status = Fs->OpenVolume( Fs, &Root );
        if( EFI_ERROR(Status) )
            continue;

        Status = Root->Open( Root, &FileHandle,
                             mFlashFileNames[0].FileName,
                             EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE,
                             0 );
        if( EFI_ERROR(Status) )
            continue;

        if( FileHandle == NULL )
            continue;
            
        CapsuleHeader = (EFI_CAPSULE_HEADER*)mCapsuleAddress;
        FileSize = CapsuleHeader->CapsuleImageSize - CapsuleHeader->HeaderSize;
        FileBuffer = AllocateZeroPool( FileSize );
        if( FileBuffer == NULL )
            continue;

        CopyMem( FileBuffer, (VOID*)(mCapsuleAddress + CapsuleHeader->HeaderSize), FileSize );

        Status = FileHandleWrite( FileHandle, &FileSize, FileBuffer );
        if( EFI_ERROR(Status) )
        {
            FreePool( FileBuffer );
            continue;
        }
        
        Status =  FileHandleClose( FileHandle );
        if( EFI_ERROR(Status) )
            break;
    }
    
    return Status;
}


EFI_STATUS
FindUpdateImageOnDevice (
    VOID
    )
{
    EFI_STATUS                          Status;
    UINTN                               HandleArrayCount;
    EFI_HANDLE                          *HandleArray;
    DXE_PCH_PLATFORM_POLICY_PROTOCOL    *DxePlatformPchPolicy;
    BOOLEAN                             IsEmmc45 = FALSE;
    EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
    UINT32                              Index;
    UINTN                               CompareResult = 0;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL     *Fs;
    EFI_FILE                            *Root;
    EFI_FILE                            *FileHandle;
    UINT64                              FileSize = 0, TmpFileSize = 0;
    VOID                                *FileBuffer  = NULL;
    EFI_GUID                            CapsuleGuid;
    EFI_CAPSULE_HEADER                  *CapsuleHeader = NULL;
    EFI_CAPSULE_HEADER                  *ech[1];
    EFI_PHYSICAL_ADDRESS                sc = (EFI_PHYSICAL_ADDRESS)0;
    UINTN                               AosCapsule = 0;

    Status = gBS->LocateHandleBuffer( ByProtocol, &gEfiPartTypeSystemPartGuid, //&gEfiSimpleFileSystemProtocolGuid,
                                      NULL, &HandleArrayCount, &HandleArray );
    if( EFI_ERROR(Status) )
        return Status;

    Status  = gBS->LocateProtocol( &gDxePchPlatformPolicyProtocolGuid, NULL,
                                   (VOID **)&DxePlatformPchPolicy);
    if( !EFI_ERROR(Status) )
    {
        if ( DxePlatformPchPolicy->SccConfig->eMMCEnabled )
        {
            IsEmmc45 = FALSE;
        }
        else if( DxePlatformPchPolicy->SccConfig->eMMC45Enabled )
        {
            IsEmmc45 = TRUE;
        }
        else
        {
            IsEmmc45 = FALSE;
        }
    }

    Status = EFI_NOT_FOUND;

    for( Index = 0; Index < HandleArrayCount; Index++ )
    {
        Status = gBS->HandleProtocol( HandleArray[Index],
                                      &gEfiDevicePathProtocolGuid,
                                      (VOID **)&DevicePath );
        if( EFI_ERROR(Status) )
            continue;

        if(IsEmmc45)
        {
            CompareResult = CompareMem( DevicePath, &gEmmcDevPath1,
                                        sizeof(gEmmcDevPath1) - 4 );
        }
        else
        {
            CompareResult = CompareMem( DevicePath, &gEmmcDevPath0,
                                        sizeof(gEmmcDevPath0) - 4 );
        }
        
        if( CompareResult )
        {
            CompareResult = CompareMem( DevicePath, &gSataDevPath0,
                                        sizeof(gSataDevPath0) - 4 );
            if( CompareResult )
                continue;
        }

        Status = gBS->HandleProtocol( HandleArray[Index],
                                      &gEfiSimpleFileSystemProtocolGuid,
                                      (VOID**)&Fs );
        if( EFI_ERROR(Status) )
            continue;

        Status = Fs->OpenVolume( Fs, &Root );
        if( EFI_ERROR(Status) )
            continue;

        Status = Root->Open( Root, &FileHandle,
                             mFlashFileNames[0].FileName,
                             EFI_FILE_MODE_WRITE|EFI_FILE_MODE_READ, 0 );
        if( EFI_ERROR(Status) )
            continue;

        Status = FileHandleGetSize( FileHandle, &FileSize );
        if( EFI_ERROR(Status) )
            continue;

        FileBuffer = AllocateZeroPool((UINTN)FileSize);
        if( FileBuffer == NULL )
            continue;

        TmpFileSize = FileSize;
        Status = FileHandleRead(FileHandle, (UINTN*)&TmpFileSize, FileBuffer);
        if( EFI_ERROR(Status) || (UINT32)FileSize != (UINT32)TmpFileSize )
        {
            FreePool( FileBuffer );
            FileBuffer = NULL;
            continue;
        }

        CapsuleGuid = gAosFirmwareClassGuid;
        CapsuleHeader = (EFI_CAPSULE_HEADER *)AllocateZeroPool((UINTN)FileSize + sizeof(EFI_CAPSULE_HEADER));
        CapsuleHeader->HeaderSize = sizeof(EFI_CAPSULE_HEADER);
        CapsuleHeader->CapsuleImageSize =(UINT32)((UINTN)( FileSize + CapsuleHeader->HeaderSize));
        CapsuleHeader->Flags = CAPSULE_FLAGS_INITIATE_RESET | CAPSULE_FLAGS_PERSIST_ACROSS_RESET;
        CapsuleHeader->CapsuleGuid = CapsuleGuid;
        CopyMem((void *)((UINTN)CapsuleHeader + CapsuleHeader->HeaderSize),FileBuffer, (UINTN)FileSize); 
        ech[0] = CapsuleHeader;
        
        AosCapsule = 1;
        gRT->SetVariable (
                L"CapsuleFromAos",
                &BiosCapsuleFromAosGuid,
                EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                sizeof(AosCapsule),
                &AosCapsule
                );
        
        Status = gRT->UpdateCapsule( ech,1,sc );
        break;
    }
    
    return Status;
}

VOID
EFIAPI
FotaAllDriversConnected2 (
    IN      EFI_EVENT                 Event,
    IN      VOID                      *Context
  )
{
    FindUpdateImageOnDevice();
    
    gBS->CloseEvent (Event);
}
//<EIP153486+> <<<
