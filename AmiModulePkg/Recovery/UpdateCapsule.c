//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013 American Megatrends, Inc.         **
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
// $Header: Alexp $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
// 
//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name: EfiCapsuleRecoveryPPI.c - Recovery PPI
//
// Description:    Implements EFI_PEI_RECOVERY_BLOCK_IO_PPI forAmi Capsule HOB.
// Capsule Recovery to be invoked separately from generic Recovery PPI 
//----------------------------------------------------------------------
//<AMI_FHDR_END>
#include <Token.h>
#include <AmiPeiLib.h>
#include <AmiHobs.h>
#include <Hob.h>
#include <FlashUpd.h>
#include <Capsule.h>

// Definitions

#ifndef EFI_HOB_TYPE_CV
#define EFI_HOB_TYPE_CV EFI_HOB_TYPE_UEFI_CAPSULE
typedef EFI_HOB_UEFI_CAPSULE EFI_HOB_CAPSULE_VOLUME;
#endif // PI BACKWARD_COMPATIBLE_MODE


// Debug
#define W8_FW_UPDATE_IMAGE_CAPSULE_GUID \
    { 0x7039436b, 0x6acf, 0x433b, 0x86, 0xa1, 0x36, 0x8e, 0xc2, 0xef, 0x7e, 0x1f }

static EFI_GUID W8FwUpdateImageCapsuleGuid = W8_FW_UPDATE_IMAGE_CAPSULE_GUID;
// end debug


static EFI_PHYSICAL_ADDRESS gCapsuleAddress = 0;
static UINT64               gCapsuleLength = 0;

//----------------------------------------------------------------------------
// Function Prototypes
//----------------------------------------------------------------------------
EFI_STATUS VerifyFwImage(
  IN CONST EFI_PEI_SERVICES  **PeiServices, //EIP155099
  IN OUT VOID          **pCapsule,
  IN OUT UINT32         *pCapsuleSize,
  IN OUT UINT32         *FailedVTask
);

//----------------------------------------------------------------------------
// Function Definitions
//----------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:	FindFWCapsuleHOB
//
// Description:	Locates Aptio FW Capsule in Capsule Hob 
//
// Input:   EFI_PEI_SERVICES **PeiServices
// Output:  EFI_PHYSICAL_ADDRESS pointer to FW CApsule
//---------------------------------------------------------------------- 
//<AMI_PHDR_END>
EFI_STATUS
FindFWCapsuleHOB
(
    IN    EFI_PEI_SERVICES        **PeiServices
){
    EFI_HOB_CAPSULE_VOLUME  *pHob;
    EFI_PHYSICAL_ADDRESS        CapsuleAddress;
    UINT64                      CapsuleLength;
    EFI_CAPSULE_HEADER         *FWCapsuleVolume;
    EFI_GUID gFWCapsuleGuid   = APTIO_FW_CAPSULE_GUID;
    BOOLEAN                     CapsuleFlag = FALSE; //EIP155099

    (*PeiServices)->GetHobList(PeiServices, &pHob);
// attempt to locate capsule volume hob
    while (!EFI_ERROR(FindNextHobByType(EFI_HOB_TYPE_CV, &pHob)))
    {
        // if capsule volume hob is found, determine the capsule's location
        CapsuleAddress = pHob->BaseAddress;
        CapsuleLength  = pHob->Length;
        FWCapsuleVolume = (EFI_CAPSULE_HEADER*) CapsuleAddress;

        //EIP155099 >>
        if (!guidcmp( &FWCapsuleVolume->CapsuleGuid,  &W8FwUpdateImageCapsuleGuid)) {
            CapsuleLength = pHob->Length - FWCapsuleVolume->HeaderSize;
            CapsuleAddress = CapsuleAddress + FWCapsuleVolume->HeaderSize;
            CapsuleFlag = TRUE;
        }
		
		if(CapsuleLength != 0 && FWCapsuleVolume->CapsuleImageSize <= CapsuleLength &&
            !(guidcmp(&(FWCapsuleVolume->CapsuleGuid), &gFWCapsuleGuid))) {
            CapsuleFlag = TRUE;
        }
        
        if (CapsuleFlag) {
            PEI_TRACE((-1, PeiServices, "FW Capsule found in Capsule Volume Hob %x\n",CapsuleAddress));
            gCapsuleLength = CapsuleLength;
            gCapsuleAddress = CapsuleAddress;
            return EFI_SUCCESS;
        }
		//EIP155099 <<
    }

    PEI_TRACE((-1,PeiServices, "FW capsule HOB not found\n"));
    return EFI_NOT_FOUND;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:	LoadUpdateCapsule
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
EFI_STATUS LoadUpdateCapsule(
	IN EFI_PEI_SERVICES **PeiServices,
	RECOVERY_IMAGE_HOB *RecoveryHob
)
{
	EFI_STATUS Status;
    UINTN       Size;
    EFI_PHYSICAL_ADDRESS       CapsuleAddress;

	PEI_TRACE((TRACE_DXEIPL, PeiServices, "Loading Recovery Image..."));

// Locate Capsule Hob
    if(!gCapsuleAddress || !gCapsuleLength) return EFI_NOT_FOUND;

    Size = (UINTN)gCapsuleLength;
    CapsuleAddress = (EFI_PHYSICAL_ADDRESS)gCapsuleAddress;
    Status = VerifyFwImage(PeiServices, (VOID**)&CapsuleAddress, (UINT32*)&Size,(UINT32*)&RecoveryHob->FailedStage ); 
    RecoveryHob->Status = (UINT8)Status;
    RecoveryHob->Address = CapsuleAddress;
    if (EFI_ERROR(Status)) {
	    PEI_ERROR_CODE(PeiServices, PEI_RECOVERY_INVALID_CAPSULE, EFI_ERROR_MAJOR);
    }else{
    	PEI_PROGRESS_CODE(PeiServices,PEI_RECOVERY_CAPSULE_LOADED);
	}
	return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:	ProcessUpdateCapsule
//
// Description:	Entry point.  Installs EFI_PEI_RECOVERY_MODULE_PPI ppi 
//              (which has function LoadRecoveryCapsule).  
//
//---------------------------------------------------------------------- 
//<AMI_PHDR_END>
EFI_STATUS ProcessUpdateCapsule(
    IN EFI_PEI_SERVICES **PeiServices,
	IN RECOVERY_IMAGE_HOB *RecoveryHob
)
{
	EFI_STATUS Status;

    Status = FindFWCapsuleHOB(PeiServices);
    if (!EFI_ERROR(Status)) {
		Status = LoadUpdateCapsule(PeiServices,RecoveryHob);
	}
    if (EFI_ERROR(Status)) {
		PEI_ERROR_CODE(PeiServices, PEI_RECOVERY_NO_CAPSULE, EFI_ERROR_MAJOR);	
    }
	return Status;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013 American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
