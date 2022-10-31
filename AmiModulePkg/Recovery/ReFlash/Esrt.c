//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
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
// $Header: /Alaska/BIN/Core/Modules/Recovery/Esrt.c 2     8/02/12 11:59a Artems $
//
// $Revision: 2 $
//
// $Date: 8/02/12 11:59a $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    Esrt.c
//
// Description:
//
//<AMI_FHDR_END>
//**********************************************************************

#include <Token.h>
#include <AmiHobs.h>
#include <AmiDxeLib.h>
#include <Capsule.h>
#include <Setup.h>
#include <PPI/FwVersion.h>
#include <Protocol/AmiReflashProtocol.h>
#include "Reflash.h"

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
    EFI_GUID    FwClass;
    UINT32      FwType;
    UINT32      FwVersion;
    UINT32      LowestSupportedFwVersion;
    UINT32      CapsuleFlags;
    UINT32      LastAttemptVersion;
    UINT32      LastAttemptStatus;
} FIRMWARE_RESOURCE_ENTRY;

typedef struct _EFI_ESRT_TABLE { 
    UINT32                  FwResourceCount;
    UINT32                  FwMaxResources;
    UINT64                  FwResourceVersion;
    FIRMWARE_RESOURCE_ENTRY Entries[MAX_ESRT_ENTRIES];
} EFI_ESRT_TABLE;

#pragma pack(pop)

static EFI_GUID SystemFirmwareUpdateClass = W8_FW_UPDATE_IMAGE_CAPSULE_GUID;  //(EIP144785)
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
            (UINT8 *)Buffer += sizeof(EFI_GUID);
            MemCpy(Fid, Buffer, sizeof(FW_VERSION));
            (UINT8 *)Buffer -= sizeof(EFI_GUID);
            pBS->FreePool(Buffer);
            return EFI_SUCCESS;
        }
	}
	pBS->FreePool(FvHandle);
	return EFI_NOT_FOUND;
}

//(EIP144785) <<
EFI_STATUS GetFidFromBuffer(
    IN VOID *Buffer,
    OUT VOID *Fid
)
//(EIP144785) <<
{
    static EFI_GUID FidSectionGuid = FID_FFS_FILE_SECTION_GUID;
    UINT32 Signature;
    UINT32 *SearchPointer;

    SearchPointer = (UINT32 *)((UINT8 *)Buffer - sizeof(EFI_GUID) + FLASH_SIZE);
    Signature = FidSectionGuid.Data1;

    do {
        if(*SearchPointer == Signature) {
            if(!guidcmp(&FidSectionGuid, (EFI_GUID *)SearchPointer)) {
                (UINT8 *)SearchPointer += sizeof(EFI_GUID);
                MemCpy(Fid, SearchPointer, sizeof(FW_VERSION));
                return EFI_SUCCESS;
            }
        }
    } while(SearchPointer-- >= (UINT32 *)Buffer);

    return EFI_NOT_FOUND;
}

UINT32 GetVersionFromFid(
    VOID *Image OPTIONAL
)
{
//(EIP144785) >>
    FW_VERSION Fid;
    UINT32 Version;
    EFI_STATUS Status;

    if(Image == NULL)
    {
        Status = GetFidFromFv(&Fid);
        Version = (CRB_PROJECT_MAJOR_VERSION << 2) + CRB_PROJECT_MINOR_VERSION;
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
//(EIP144785) <<
}

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

EFI_STATUS IsWin8Update(
    BOOLEAN RecoveryFailed
)
{
    AMI_CAPSULE_HOB *Hob;
    static EFI_GUID AmiCapsuleHobGuid = AMI_CAPSULE_HOB_GUID;
    static EFI_GUID ImageCapsuleGuid = W8_SCREEN_IMAGE_CAPSULE_GUID;
    static EFI_GUID guidBootFlow = BOOT_FLOW_VARIABLE_GUID;
    EFI_HANDLE Handle = NULL;
    EFI_STATUS Status;
    UINT32 BootFlow = BOOT_FLOW_CONDITION_OS_UPD_CAP;

    Hob = GetEfiConfigurationTable(pST, &guidHob);
    if(Hob == NULL)
        return EFI_NOT_FOUND;

    do {
        Status = FindNextHobByGuid(&AmiCapsuleHobGuid, &Hob);
        if(!EFI_ERROR(Status) && !guidcmp(&(Hob->CapsuleGuid), &ImageCapsuleGuid))
            break;
    } while(!EFI_ERROR(Status));

    if(EFI_ERROR(Status))   //no image hob - we're not on OS FW update path
        return Status;

    if(RecoveryFailed) { //we're on OS FW update path, but recovery can't be performed
        InstallEsrtTable();
        return EFI_UNLOAD_IMAGE;
    }

//save image capsule pointer
    Image = (W8_IMAGE_CAPSULE *)(VOID *)(UINTN)(Hob->CapsuleData);
//install reflash protocol
    Status = pBS->InstallMultipleProtocolInterfaces(
		                        &Handle,
		                        &gAmiReflashProtocolGuid, 
                                &AmiReflashProtocol,
		                        NULL);
    if(EFI_ERROR(Status))
        return Status;

//set boot flow
    pRS->SetVariable(L"BootFlow", &guidBootFlow, EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof(BootFlow), &BootFlow);
    return Status;
}

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

EFI_STATUS FwUpdate(
        IN EFI_REFLASH_PROTOCOL *This
)
{
    EFI_STATUS Status;
    UINT32 Version;

/* set version we're upgrading to */
    Version = GetVersionFromFid(RecoveryBuffer);
    Status = pRS->SetVariable(FW_VERSION_VARIABLE, &gAmiGlobalVariableGuid,
                              EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                              sizeof(UINT32), &Version);

    Status = Prologue(FALSE);
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
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
