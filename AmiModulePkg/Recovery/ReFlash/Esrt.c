//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file
  This file contains Reflash driver ESRT table related code

**/

#include <Token.h>
#include <AmiHobs.h>
#include <AmiDxeLib.h>
#include <Capsule.h>
#include <Setup.h>
#include <Ppi/FwVersion.h>
#include <Protocol/AmiReflashProtocol.h>
#include <Protocol/FirmwareVolume2.h>
#include "ReFlash.h"

#define MAX_ESRT_ENTRIES 1

#define FW_TYPE_UNKNOWN         0
#define FW_TYPE_SYSTEM_FIRMWARE 1
#define FW_TYPE_DEVICE_FIRMWARE 2
#define FW_TYPE_FIRMWARE_DRIVER 3

#define FW_UPDATE_STATUS_SUCCESS              0
#define FW_UPDATE_STATUS_UNKNOWN_ERROR        1
#define FW_UPDATE_STATUS_OUT_OF_RESOURCES     2
#define FW_UPDATE_STATUS_INCORRECT_VERSION    3
#define FW_UPDATE_STATUS_INVALID_FORMAT       4
#define FW_UPDATE_STATUS_AUTHENTICATION_ERROR 5

#define EFI_SYSTEM_RESOURCE_TABLE_GUID \
    { 0xb122a263, 0x3661, 0x4f68, 0x99, 0x29, 0x78, 0xf8, 0xb0, 0xd6, 0x21, 0x80 }

#define FID_FFS_FILE_NAME_GUID \
    { 0x3fd1d3a2, 0x99f7, 0x420b, 0xbc, 0x69, 0x8b, 0xb1, 0xd4, 0x92, 0xa3, 0x32 }

#define FID_FFS_FILE_SECTION_GUID \
    { 0x2EBE0275, 0x6458, 0x4AF9, 0x91, 0xED, 0xD3, 0xF4, 0xED, 0xB1, 0x00, 0xAA }

#pragma pack(push, 1)
typedef struct _FIRMWARE_RESOURCE_ENTRY {
    EFI_GUID    FwClass;                    //!< Firmware class ID
    UINT32      FwType;                     //!< Firmware type ID
    UINT32      FwVersion;                  //!< Firmware version
    UINT32      LowestSupportedFwVersion;   //!< Lowest supported version of firmware
    UINT32      CapsuleFlags;               //!< Flags to be set in recovery capsule
    UINT32      LastAttemptVersion;         //!< Version of last used firmware update
    UINT32      LastAttemptStatus;          //!< Last firmware update status
} FIRMWARE_RESOURCE_ENTRY;

typedef struct _EFI_ESRT_TABLE { 
    UINT32                  FwResourceCount;            //!< Number of resources in ESRT table
    UINT32                  FwMaxResources;             //!< Max supported number of resources
    UINT64                  FwResourceVersion;          //!< Resource version
    FIRMWARE_RESOURCE_ENTRY Entries[MAX_ESRT_ENTRIES];  //!< Resource description entry
} EFI_ESRT_TABLE;

#pragma pack(pop)

static EFI_GUID SystemFirmwareUpdateClass = W8_FW_UPDATE_IMAGE_CAPSULE_GUID;
static EFI_GUID SystemResourceTableGuid = EFI_SYSTEM_RESOURCE_TABLE_GUID;
static EFI_GUID guidHob = HOB_LIST_GUID;
static W8_IMAGE_CAPSULE *Image;
extern UINT8 *RecoveryBuffer;
extern EFI_GUID gAmiGlobalVariableGuid;


EFI_STATUS GetDisplayImage(
    IN EFI_REFLASH_PROTOCOL *This,
    OUT UINTN               *CoordinateX,
    OUT UINTN               *CoordinateY,
    OUT VOID                **ImageAddress
);

EFI_STATUS FwUpdate(
        IN EFI_REFLASH_PROTOCOL *This
);

EFI_REFLASH_PROTOCOL AmiReflashProtocol = {
    FwUpdate,
    GetDisplayImage
};

/**
  This function returns current FID descriptor
  
  @param Fid     Pointer where to store FID descriptor

  @retval EFI_SUCCESS Layout returned successfully
  @retval other       error occured during execution

**/
EFI_STATUS GetFidFromFv(
    OUT VOID *Fid
)
{
    static EFI_GUID FidFileName = FID_FFS_FILE_NAME_GUID;
    EFI_STATUS Status;
    EFI_HANDLE *FvHandle;
    UINTN FvCount;
    UINTN i;
    UINTN BufferSize;
    VOID *Buffer;

	Status = pBS->LocateHandleBuffer(ByProtocol, &gEfiFirmwareVolume2ProtocolGuid, NULL, &FvCount, &FvHandle);
	if (EFI_ERROR(Status)) 
        return Status;

	for(i = 0; i < FvCount; i++)
	{
	    EFI_FIRMWARE_VOLUME_PROTOCOL *Fv;
	    UINT32 AuthStatus;
        Status = pBS->HandleProtocol(FvHandle[i], &gEfiFirmwareVolume2ProtocolGuid, &Fv);
		if (EFI_ERROR(Status)) 
            continue;
        Buffer = 0;
        BufferSize = 0;
	    Status = Fv->ReadSection(Fv, &FidFileName, EFI_SECTION_FREEFORM_SUBTYPE_GUID, 0, &Buffer, &BufferSize, &AuthStatus);
        TRACE((-1, "extracted section with guid %g\n", (EFI_GUID *)Buffer));
        if (!EFI_ERROR(Status)) {
            Buffer = (UINT8 *)Buffer + sizeof(EFI_GUID);
            MemCpy(Fid, Buffer, sizeof(FW_VERSION));
            Buffer = (UINT8 *)Buffer - sizeof(EFI_GUID);
            pBS->FreePool(Buffer);
            return EFI_SUCCESS;
        }
	}
	pBS->FreePool(FvHandle);
	return EFI_NOT_FOUND;
}

/**
  This function returns FID descriptor stored in provided buffer
  
  @param Fid     Pointer where to store FID descriptor
  @param Buffer  Pointer to the buffer to be searched

  @retval EFI_SUCCESS       Layout returned successfully
  @retval EFI_NOT_FOUND     There is no FID descriptor in buffer

**/
EFI_STATUS GetFidFromBuffer(
    OUT VOID *Fid,
    IN VOID *Buffer
)
{
    static EFI_GUID FidSectionGuid = FID_FFS_FILE_SECTION_GUID;
    UINT32 Signature;
    UINT32 *SearchPointer;

    SearchPointer = (UINT32 *)((UINT8 *)Buffer - sizeof(EFI_GUID) + FLASH_SIZE);
    Signature = FidSectionGuid.Data1;

    do {
        if(*SearchPointer == Signature) {
            if(!guidcmp(&FidSectionGuid, (EFI_GUID *)SearchPointer)) {
                SearchPointer = (UINT32 *)((UINT8 *)SearchPointer + sizeof(EFI_GUID));
                MemCpy(Fid, SearchPointer, sizeof(FW_VERSION));
                return EFI_SUCCESS;
            }
        }
    } while(SearchPointer-- >= (UINT32 *)Buffer);

    return EFI_NOT_FOUND;
}

/**
  This function returns firmware version from FID descriptor
  
  @param Image   Pointer to the recovery image (or NULL if current
                 image to be used)

  @return Firmware version

**/
UINT32 GetVersionFromFid(
    VOID *Image OPTIONAL
)
{
    FW_VERSION Fid;
    UINT32 Version;
    EFI_STATUS Status;

    if(Image == NULL)
    {
        Status = GetFidFromFv(&Fid);
        Version = (PROJECT_MAJOR_VERSION << 2) + PROJECT_MINOR_VERSION;
        TRACE((-1, "Version = %X\n", Version));
    	return Version;
    }
    else
        Status = GetFidFromBuffer(Image, &Fid);

    if(EFI_ERROR(Status))
        return 0;

    Version = Fid.ProjectMajorVersion[0] + Fid.ProjectMajorVersion[1] + Fid.ProjectMajorVersion[2];
    Version <<= 16;
    Version += Fid.ProjectMinorVersion[0] + Fid.ProjectMinorVersion[1] + Fid.ProjectMinorVersion[2];
    return Version;
   // return 1;
}

/**
  This function installs ESRT table
  
  @retval EFI_SUCCESS   Table installed successfully
  @retval other         Some error occured during execution

**/
EFI_STATUS InstallEsrtTable(
    VOID
)
{
    static EFI_GUID RecoveryHobGuid = AMI_RECOVERY_IMAGE_HOB_GUID;
    EFI_HOB_HANDOFF_INFO_TABLE *pHit;
    EFI_STATUS Status;
    UINTN Size = sizeof(UINT32);
    UINT32 Version;
    EFI_ESRT_TABLE *EsrtTable;

    Status = pBS->AllocatePool(EfiRuntimeServicesData, sizeof(EFI_ESRT_TABLE), &EsrtTable);
    if(EFI_ERROR(Status))
        return Status;

    EsrtTable->FwResourceCount = 1;
    EsrtTable->FwMaxResources = MAX_ESRT_ENTRIES;
    EsrtTable->FwResourceVersion = 1;

    EsrtTable->Entries[0].FwClass = SystemFirmwareUpdateClass;
    EsrtTable->Entries[0].FwType = FW_TYPE_SYSTEM_FIRMWARE;
    EsrtTable->Entries[0].FwVersion = GetVersionFromFid(NULL);
    EsrtTable->Entries[0].LowestSupportedFwVersion = EsrtTable->Entries[0].FwVersion;     //no rollback allowed
    EsrtTable->Entries[0].CapsuleFlags = 0;

    Status = pRS->GetVariable(FW_VERSION_VARIABLE, &gAmiGlobalVariableGuid, NULL, &Size, &Version);
    if(!EFI_ERROR(Status)) {
        EsrtTable->Entries[0].LastAttemptVersion = Version;
        EsrtTable->Entries[0].LastAttemptStatus = FW_UPDATE_STATUS_UNKNOWN_ERROR;
    } else {
        pHit = GetEfiConfigurationTable(pST, &guidHob);
        if(pHit != NULL && !EFI_ERROR(FindNextHobByGuid(&RecoveryHobGuid, &pHit))) {
            EsrtTable->Entries[0].LastAttemptVersion = GetVersionFromFid((VOID *)(UINTN)((RECOVERY_IMAGE_HOB*)pHit)->Address);
            switch(((RECOVERY_IMAGE_HOB*)pHit)->FailedStage) {
                case 0:     //no authentication/verification error  
                    EsrtTable->Entries[0].LastAttemptStatus = FW_UPDATE_STATUS_UNKNOWN_ERROR;
                    break;
                case 1:
                    EsrtTable->Entries[0].LastAttemptStatus = FW_UPDATE_STATUS_INVALID_FORMAT;
                    break;
                case 2:
                    EsrtTable->Entries[0].LastAttemptStatus = FW_UPDATE_STATUS_AUTHENTICATION_ERROR;
                    break;
                case 3:
                    EsrtTable->Entries[0].LastAttemptStatus = FW_UPDATE_STATUS_AUTHENTICATION_ERROR;
                    break;
                case 4:
                    EsrtTable->Entries[0].LastAttemptStatus = FW_UPDATE_STATUS_INCORRECT_VERSION;
                    break;
                default:
                    EsrtTable->Entries[0].LastAttemptStatus = FW_UPDATE_STATUS_UNKNOWN_ERROR;
                    break;
            }
        } else {    //recovery hob not found - regular boot
            EsrtTable->Entries[0].LastAttemptVersion = EsrtTable->Entries[0].FwVersion;
            EsrtTable->Entries[0].LastAttemptStatus = FW_UPDATE_STATUS_SUCCESS;
        }
    }

    Status = pBS->InstallConfigurationTable(&SystemResourceTableGuid, EsrtTable);
    return Status;
}

/**
  This function checks if we're on Windows 8 firmware update boot path
  
  @param RecoveryFailed Firmware update status flag
  
  @retval TRUE   We're on Windows 8 firmware update boot path
  @retval FALSE  We're not on Windows 8 firmware update boot path

**/
EFI_STATUS IsWin8Update(
    BOOLEAN RecoveryFailed
)
{
//EIP185806 >>
    EFI_HOB_UEFI_CAPSULE *Hob;
    EFI_CAPSULE_HEADER *Capsule;
    static EFI_GUID ImageCapsuleGuid = W8_SCREEN_IMAGE_CAPSULE_GUID;
    static EFI_GUID guidBootFlow = BOOT_FLOW_VARIABLE_GUID;
    EFI_HANDLE Handle = NULL;
    EFI_STATUS Status;
    UINT32 BootFlow = BOOT_FLOW_CONDITION_OS_UPD_CAP;

    Hob = GetEfiConfigurationTable(pST, &guidHob);
    if(Hob == NULL)
        return EFI_NOT_FOUND;

    do {
        Status = FindNextHobByType(EFI_HOB_TYPE_UEFI_CAPSULE, &Hob);
        if(!EFI_ERROR(Status)) { 
            Capsule = (EFI_CAPSULE_HEADER *)(VOID *)(UINTN)Hob->BaseAddress;
            if(!guidcmp(&(Capsule->CapsuleGuid), &ImageCapsuleGuid))
            break;
        }
    } while(!EFI_ERROR(Status));

    if(EFI_ERROR(Status))   //no image hob - we're not on OS FW update path
        return Status;

    if(RecoveryFailed) { //we're on OS FW update path, but recovery can't be performed
        InstallEsrtTable();
        return EFI_UNLOAD_IMAGE;
    }

//save image capsule pointer
    Image = (W8_IMAGE_CAPSULE *)Capsule;
//install reflash protocol
//EIP185806 <<
    Status = pBS->InstallMultipleProtocolInterfaces(
		                        &Handle,
		                        &gAmiReflashProtocolGuid, 
                                &AmiReflashProtocol,
		                        NULL);
    if(!EFI_ERROR(Status))
        //set boot flow
        pRS->SetVariable(L"BootFlow", &guidBootFlow, EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof(BootFlow), &BootFlow);

    return Status;
}

/**
  This function returns Windows 8 firmware update image attributes
  
  @param This           Pointer to the EFI_REFLASH_PROTOCOL instance
  @param CoordinateX    Pointer where to store image left corner horisontal coordinate
  @param CoordinateY    Pointer where to store image left corner vertical coordinate
  @param ImageAddress   Pointer where to store pointer to image data

  @retval EFI_SUCCESS   Image returned successfully
  @retval other         There is some error occured during execution

**/
EFI_STATUS GetDisplayImage(
    IN EFI_REFLASH_PROTOCOL *This,
    OUT UINTN               *CoordinateX,
    OUT UINTN               *CoordinateY,
    OUT VOID                **ImageAddress
)
{
    if(CoordinateX == NULL ||
       CoordinateY == NULL ||
       ImageAddress == NULL)
        return EFI_INVALID_PARAMETER;

    *CoordinateX = Image->ImageOffsetX;
    *CoordinateY = Image->ImageOffsetY;
    *ImageAddress = Image->Image;
    return EFI_SUCCESS;
}

/**
  This function performs Windows 8 firmware update
  
  @param This           Pointer to the EFI_REFLASH_PROTOCOL instance

  @retval EFI_SUCCESS   Firmware updated successfully
  @retval other         There is some error occured during execution

**/
EFI_STATUS FwUpdate(
        IN EFI_REFLASH_PROTOCOL *This
)
{
    EFI_STATUS Status;
    UINT32 Version;

/* set version we're upgrading to */
    Version = GetVersionFromFid(RecoveryBuffer);
    pRS->SetVariable(FW_VERSION_VARIABLE, &gAmiGlobalVariableGuid,
                     EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                     sizeof(UINT32), &Version);

    Status = Prologue(FALSE, TRUE);
    if(EFI_ERROR(Status))
        pRS->ResetSystem(EfiResetCold, Status, 0, NULL);

    Status = FlashWalker(FALSE);
    if(EFI_ERROR(Status))
        pRS->ResetSystem(EfiResetCold, Status, 0, NULL);

    Status = Epilogue();
    return Status;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
