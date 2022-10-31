//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

#include <PiPei.h>
#include <AmiStatusCodes.h>
#include <AmiHobs.h>
#include <Token.h>

#include <Ppi/RecoveryModule.h>
#include <Ppi/DeviceRecoveryModule.h>
#include <Ppi/PeiRecoverySerialModePpi.h>

#include <Guid/AmiRecoveryDevice.h>

#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/AmiReportFVLib.h>
#include <Library/ReportStatusCodeLib.h>


#define BLOCK  &gBlockDeviceCapsuleGuid
#define SERIAL &gSerialCapsuleGuid
#define OEM    &gOemCapsuleGuid

EFI_GUID* RecoveryDeviceOrder[] = {RECOVERY_DEVICE_ORDER NULL};

EFI_STATUS LoadRecoveryCapsule(
	IN EFI_PEI_SERVICES **PeiServices,
	IN struct _EFI_PEI_RECOVERY_MODULE_PPI *This
);

EFI_PEI_RECOVERY_MODULE_PPI RecoveryModule = {LoadRecoveryCapsule};

// PPI to be installed
static EFI_PEI_PPI_DESCRIPTOR RecoveryPpiList[] =
{ 
	{
		EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
		&gEfiPeiRecoveryModulePpiGuid, &RecoveryModule
	}	
};

EFI_PHYSICAL_ADDRESS RecoveryBuffer = 0;
UINTN RecoveryBufferSize = 0;
RECOVERY_IMAGE_HOB *pRecoveryHob;

EFI_STATUS ProcessUpdateCapsule(
    IN EFI_PEI_SERVICES **PeiServices,
	RECOVERY_IMAGE_HOB *RecoveryHob
);

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:	ReadCapsule
//
// Description:	Called by FindRecoveryDevice.  Calls LoadRecoveryCapsule
//      function of the passed in ppi EFI_PEI_DEVICE_RECOVERY_MODULE_PPI 
//      to get recovery image.  If found, an HOB is created for this 
//      recovery image.  
//
//---------------------------------------------------------------------- 
//<AMI_PHDR_END>

EFI_STATUS VerifyFwImage (
  IN EFI_PEI_SERVICES  **PeiServices,
  IN VOID              **Buffer,
  IN OUT UINT32         *Size,
  OUT UINT32            *FailedTask
)
#if defined(SecFlashUpd_SUPPORT) && SecFlashUpd_SUPPORT == 1
;
#else
{ 
    if(FailedTask)
        *FailedTask = 0;
    return EFI_SUCCESS; 
}
#endif

EFI_STATUS ReadCapsule(
	IN EFI_PEI_SERVICES **PeiServices,
	EFI_PEI_DEVICE_RECOVERY_MODULE_PPI *pDRM, 
	UINTN CapsuleInstance, UINTN Size
)
{
	EFI_STATUS           Status;
    UINT32               FailedStage;

    if(Size > RecoveryBufferSize){
		Status = (*PeiServices)->AllocatePages(PeiServices, EfiBootServicesCode, (Size >> 12) + 1, &RecoveryBuffer);
        if (EFI_ERROR(Status)) 
            return Status;

        RecoveryBufferSize = Size;
    }

	DEBUG((EFI_D_INFO | EFI_D_LOAD, "Loading Recovery Image..."));

	Status = pDRM->LoadRecoveryCapsule(PeiServices, pDRM, CapsuleInstance, (VOID*)RecoveryBuffer);

	DEBUG((EFI_D_INFO | EFI_D_LOAD, "done. Status: %r\n",Status));

	if (EFI_ERROR(Status)) 
        return Status;

    Status = VerifyFwImage(PeiServices, (VOID**)&RecoveryBuffer, (UINT32*)&Size, (UINT32*)&FailedStage ); 
    pRecoveryHob->FailedStage = FailedStage;
    pRecoveryHob->Status = (UINT8)Status;
    if (EFI_ERROR(Status )) {
        REPORT_STATUS_CODE(EFI_ERROR_CODE | EFI_ERROR_MAJOR, PEI_RECOVERY_INVALID_CAPSULE);
        return Status;
    }

    pRecoveryHob->Address = RecoveryBuffer;

	return Status;
}

typedef BOOLEAN (*PREDICATE)(EFI_GUID *pType, VOID* pContext);

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:	FindRecoveryDevice
//
// Description:	Called by LoadRecoveryCapsule.  
//              Loop: Locate all installed EFI_PEI_DEVICE_RECOVERY_MODULE_PPI 
//              ppis and call ReadCapsule on them.  
//
//---------------------------------------------------------------------- 
//<AMI_PHDR_END>
EFI_STATUS FindRecoveryDevice(
	IN EFI_PEI_SERVICES **PeiServices,
	PREDICATE Criteria, VOID* pContext
){
	EFI_STATUS Status = EFI_SUCCESS;
	BOOLEAN Loaded = FALSE;
	UINTN i = 0;

	do {
		EFI_PEI_DEVICE_RECOVERY_MODULE_PPI *pRecoveryDevice;
		EFI_PEI_PPI_DESCRIPTOR *pDummy;
		UINTN j, n;

		Status = (*PeiServices)->LocatePpi(PeiServices, &gEfiPeiDeviceRecoveryModulePpiGuid, i++, &pDummy, &pRecoveryDevice);
		if (EFI_ERROR(Status)) 
            break;

		Status = pRecoveryDevice->GetNumberRecoveryCapsules(PeiServices, pRecoveryDevice, &n);
		if (EFI_ERROR(Status)) 
            continue;

		for(j = 0; j < n; j++) {
			UINTN Size;
			EFI_GUID CapsuleType;

			Status = pRecoveryDevice->GetRecoveryCapsuleInfo(PeiServices, pRecoveryDevice, j, &Size, &CapsuleType);
			if (EFI_ERROR(Status) || !Criteria(&CapsuleType, pContext)) 
                continue;

			Status = ReadCapsule(PeiServices, pRecoveryDevice, j, Size);
			if (!EFI_ERROR(Status)){
                Loaded = TRUE; 
                REPORT_STATUS_CODE(EFI_PROGRESS_CODE, PEI_RECOVERY_CAPSULE_LOADED);
                break; 
            }
		}
	} while(!Loaded);
	return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:	UnknownType
//
// Description:	Returns TRUE if the parameter, guid CapsuleType, in not 
//              in parameter, List (a list of guids).  
//
//---------------------------------------------------------------------- 
//<AMI_PHDR_END>
BOOLEAN UnknownType(EFI_GUID* CapsuleType, EFI_GUID** List)
{
	EFI_GUID **pType;
	for(pType = List; *pType; pType++) if (CompareGuid(*pType,CapsuleType)) return FALSE;
	return TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:	LoadRecoveryCapsule
//
// Description:	LoadRecoveryCapsule function of EFI_PEI_RECOVERY_MODULE_PPI 
//              ppi.  RecoveryDeviceOrder is a list of guids; each guid 
//              represents a type of recovery device.  We go through 
//              this list and call FindRecoveryDevice for each type of 
//              device. 
//              -This function should not be confused with LoadRecoveryCapsule
//              function of the EFI_PEI_DEVICE_RECOVERY_MODULE_PPI ppi.  
//              -Called by DxeIpl.
//
//---------------------------------------------------------------------- 
//<AMI_PHDR_END>
EFI_STATUS LoadRecoveryCapsule(
	IN EFI_PEI_SERVICES **PeiServices,
	IN EFI_PEI_RECOVERY_MODULE_PPI *This
)
{
	EFI_GUID **ppType;
	EFI_STATUS Status = EFI_NOT_FOUND;
	UINTN i;
	EFI_BOOT_MODE BootMode;

    REPORT_STATUS_CODE(EFI_PROGRESS_CODE, PEI_RECOVERY_STARTED);

// Create Recovery Hob
	Status = (*PeiServices)->CreateHob(
		PeiServices, EFI_HOB_TYPE_GUID_EXTENSION, 
		sizeof(RECOVERY_IMAGE_HOB), &pRecoveryHob);

    if (EFI_ERROR(Status)) 
        return Status;

    pRecoveryHob->Header.Name = gAmiRecoveryImageHobGuid;
    pRecoveryHob->Address = 0;
    pRecoveryHob->FailedStage = 0;
	pRecoveryHob->Status = (UINT8)EFI_NOT_FOUND;

	Status = (*PeiServices)->GetBootMode(PeiServices, &BootMode);
	if (!EFI_ERROR(Status) && BootMode == BOOT_ON_FLASH_UPDATE)
		return ProcessUpdateCapsule(PeiServices, pRecoveryHob);
    
	for(i = 0; i < RECOVERY_SCAN_RETRIES; i++) {
		for(ppType = RecoveryDeviceOrder; *ppType; ppType++) {
			Status = FindRecoveryDevice(PeiServices,CompareGuid,*ppType);
			if (!EFI_ERROR(Status)) 
                return Status;
		}

		Status = FindRecoveryDevice(PeiServices, UnknownType, RecoveryDeviceOrder);
		if (!EFI_ERROR(Status)) break;
	}

    if (EFI_ERROR(Status)) { 
        REPORT_STATUS_CODE(EFI_ERROR_CODE | EFI_ERROR_MAJOR, PEI_RECOVERY_NO_CAPSULE);
        pRecoveryHob->Status = (UINT8)Status;
    }

	return Status;
}

// DO NOT USE InitParts()
//this funciton is created from InitList.c template file during build process
//VOID InitParts(IN EFI_FFS_FILE_HEADER *FfsHeader,IN EFI_PEI_SERVICES **PeiServices);

/**
  This function initialize recovery functionality by installing the recovery PPI.

  @retval EFI_SUCCESS if the interface could be successfully installed.
**/
EFI_STATUS
EFIAPI
RecoveryEntry (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
	//InitParts(FfsHeader,PeiServices);
	return (*PeiServices)->InstallPpi(PeiServices,RecoveryPpiList);
}


// Comment out the following functions until we can finish porting the Recovery module
/*
VOID OemGetFileListFromPrimaryVolume(
    IN  DIR_RECORD          *Root,
    IN  UINT32              RootSize,
    OUT UINTN               *NumberOfFiles,
    OUT DIR_RECORD          **Buffer
);

VOID OemGetFileListFromFatVolume(
    IN  DIR_ENTRY            *Root,
    IN  UINT32               RootEntries,
    OUT UINTN                *NumberOfFiles,
    OUT DIR_ENTRY            **Buffer
);

BOOLEAN OemIsValidFile(
    IN VOID  *FileData,
    IN UINTN FileSize
);


VOID GetFileListFromPrimaryVolume(
    IN  DIR_RECORD          *Root,
    IN  UINT32              RootSize,
    OUT UINTN               *NumberOfFiles,
    OUT DIR_RECORD          **Buffer
)
{
    OemGetFileListFromPrimaryVolume(Root, RootSize, NumberOfFiles, Buffer);
}

VOID GetFileListFromFatVolume(
    IN  DIR_ENTRY            *Root,
    IN  UINT32               RootEntries,
    OUT UINTN                *NumberOfFiles,
    OUT DIR_ENTRY            **Buffer
)
{
    OemGetFileListFromFatVolume(Root, RootEntries, NumberOfFiles, Buffer);
}

BOOLEAN IsValidFile(
    IN VOID  *FileData,
    IN UINTN FileSize
)
{
    return OemIsValidFile(FileData, FileSize);
}

EFI_STATUS OemGetRecoveryFileInfo(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT VOID         **pCapsuleName,
    IN OUT UINTN        *pCapsuleSize,
    OUT    BOOLEAN      *ExtendedVerification
);

EFI_STATUS GetRecoveryFileInfo(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT VOID         **pCapsuleName,
    IN OUT UINTN        *pCapsuleSize,
    OUT    BOOLEAN      *ExtendedVerification
)
{
    if(pCapsuleName != NULL)
        *pCapsuleName = (CHAR8 *)RecoveryFileName;

    if(pCapsuleSize != NULL)
        *pCapsuleSize = (UINTN)RecoveryImageSize;

    if(ExtendedVerification != NULL)
        *ExtendedVerification = FALSE;

    return OemGetRecoveryFileInfo(PeiServices, pCapsuleName, pCapsuleSize, ExtendedVerification);
}

EFI_STATUS AmiGetRecoveryFileInfo(
    IN EFI_PEI_SERVICES **PeiServices,
    IN OUT VOID         **pCapsuleName,
    IN OUT UINTN        *pCapsuleSize,
    OUT    BOOLEAN      *ExtendedVerification
)
{
    return EFI_SUCCESS;
}
*/
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************