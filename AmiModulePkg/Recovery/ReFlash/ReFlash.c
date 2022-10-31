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
// $Header: /Alaska/BIN/Core/Modules/Recovery/ReFlash.c 38    7/20/12 10:17a Artems $
//
// $Revision: 38 $
//
// $Date: 7/20/12 10:17a $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    ReFlash.c
//
// Description:
//
//<AMI_FHDR_END>
//**********************************************************************
#include <HOB.h>
#include <Flash.h>
#include <AmiCspLib.h>
#include <AmiHobs.h>
#include <FlashUpd.h>
#include "ReFlash.h"


static EFI_GUID guidRecovery = RECOVERY_FORM_SET_GUID;
EFI_HANDLE ThisImageHandle;
FLASH_PROTOCOL *Flash;
EFI_HII_HANDLE ReflashHiiHandle = NULL;
UINT8 *RecoveryBuffer = NULL;
EFI_GUID BiosCapsuleFromAosGuid = { 0xCD193840, 0x2881, 0x9567, { 0x39, 0x28, 0x38, 0xc5, 0x97, 0x53, 0x49, 0x77 }};  //<EIP150193+>

EFI_HII_CONFIG_ACCESS_PROTOCOL CallBack = { NULL,NULL,FlashProgressEx };

CALLBACK_INFO SetupCallBack[] =
{
    // Last field in every structure will be filled by the Setup
    { &guidRecovery, &CallBack, RECOVERY_FORM_SET_CLASS, 0, 0},
};


//-------------------------------
//Before flash and After flash eLinks

typedef VOID (OEM_FLASH_UPDATE_CALLBACK) (VOID);
extern OEM_FLASH_UPDATE_CALLBACK OEM_BEFORE_FLASH_UPDATE_CALLBACK_LIST EndOfList;
extern OEM_FLASH_UPDATE_CALLBACK OEM_AFTER_FLASH_UPDATE_CALLBACK_LIST EndOfList;
OEM_FLASH_UPDATE_CALLBACK* OemBeforeFlashCallbackList[] = { OEM_BEFORE_FLASH_UPDATE_CALLBACK_LIST NULL };
OEM_FLASH_UPDATE_CALLBACK* OemAfterFlashCallbackList[] = { OEM_AFTER_FLASH_UPDATE_CALLBACK_LIST NULL };

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:    OemBeforeFlashCallback
// 
// Description:  This function executes OEM porting hooks before starting flash update
//               
//  Input:
// 	None
//
//  Output:
//  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID OemBeforeFlashCallback(
    VOID
)
{
    UINT32 i;
    for(i = 0; OemBeforeFlashCallbackList[i] != NULL; i++)
        OemBeforeFlashCallbackList[i]();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:    OemAfterFlashCallback
// 
// Description:  This function executes OEM porting hooks after finishing flash update
//               
//  Input:
// 	None
//
//  Output:
//  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID OemAfterFlashCallback(
    VOID
)
{
    UINT32 i;
    for(i = 0; OemAfterFlashCallbackList[i] != NULL; i++)
        OemAfterFlashCallbackList[i]();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetHiiString
//
// Description: This function reads a string from HII
//
// Input:       IN EFI_HII_HANDLE   HiiHandle - Efi Hii Handle
//              IN STRING_REF       Token     - String Token
//              IN OUT UINTN        *pDataSize - Length of the StringBuffer
//              OUT EFI_STRING      *ppData - The buffer to receive the characters in the string.
//
// Output:      EFI_STATUS - Depending on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS GetHiiString(
    IN EFI_HII_HANDLE HiiHandle,
    IN STRING_REF Token,
    IN OUT UINTN *pDataSize, 
    OUT EFI_STRING *ppData
)
{
    EFI_STATUS Status;
    
    if (!*ppData) *pDataSize=0;
    
    Status = HiiLibGetString(HiiHandle, Token, pDataSize, *ppData);
    if (!EFI_ERROR(Status)) return Status;
    //--- If size was too small free pool and try with right size, which was passed
    if (Status==EFI_BUFFER_TOO_SMALL)
    {
        if (*ppData) pBS->FreePool(*ppData);
        
        if (!(*ppData=Malloc(*pDataSize))) return EFI_OUT_OF_RESOURCES;
        
        Status = HiiLibGetString(HiiHandle, Token, pDataSize, *ppData);
    }
    
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ApplyUserSelection
//
// Description: This function updates flash parameteres based on user selection
// or Setup values
//
// Input:
//  IN BOOLEAN Interactive - if TRUE get selection from user input, otherwise
//                           use Setup values
//
// Output:
//  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ApplyUserSelection(
    IN BOOLEAN Interactive
)
{
    EFI_STATUS Status;
    AUTOFLASH FlashUpdateControl;
    UINTN Size = sizeof(FlashUpdateControl);
    UINT32 i;

    if(Interactive) {
    /* get values from Setup Browser */
        Status = HiiLibGetBrowserData(&Size, &FlashUpdateControl, &guidRecovery, L"Setup");
    } else {
    /* get values from NVRAM */
        Status = pRS->GetVariable(L"Setup", &guidRecovery, NULL, &Size, &FlashUpdateControl);
    }
    if(EFI_ERROR(Status)) {
    /* no user selection, use defaults */
        FlashUpdateControl.UpdateMain = REFLASH_UPDATE_MAIN_BLOCK;
        FlashUpdateControl.UpdateBb = REFLASH_UPDATE_BOOT_BLOCK;
        FlashUpdateControl.UpdateNv = REFLASH_UPDATE_NVRAM;
    }

    for(i = 0; BlocksToUpdate[i].Type != FvTypeMax; i++) {
        switch(BlocksToUpdate[i].Type) {
            case FvTypeMain:
                BlocksToUpdate[i].Update = FlashUpdateControl.UpdateMain;
#if FtRecovery_SUPPORT
                if(FlashUpdateControl.UpdateBb == 1)
                    BlocksToUpdate[i].Update = TRUE;    //with fault tolerant recovery FV_MAIN is used for backup - force update
#endif
                break;
            case FvTypeBootBlock:
                BlocksToUpdate[i].Update = FlashUpdateControl.UpdateBb;
#if FtRecovery_SUPPORT
                if(IsTopSwapOn())   //if we're here BB update failed we use backup copy - force BB update again
                    BlocksToUpdate[i].Update = TRUE;
#endif
                break;
            case FvTypeNvRam:
                BlocksToUpdate[i].Update = FlashUpdateControl.UpdateNv;
                break;
            default:
                break;
        }
    }
}

//----------------------------------------------------------------------------
// IsRecovery prototypes
//----------------------------------------------------------------------------
// Bit Mask of checks to perform on Aptio FW Image
// 1- Capsule integrity
// 2- Verify Signature
// 3- Verify FW Key
// 4- Verify FW Version compatibility. 
//    To prevent possible re-play attack:
//    update current FW with older version with lower security.
// 5- Compare MonotonicCounters/date. Replay attack
//----------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateSetupStrings
//
// Description: This function updates status strings in setup window, based
// on execution results
//
// Input:
//  IN EFI_HII_HANDLE Handle - handle of Reflash setup formset (page)
//  IN EFI_STATUS Error - execution error if any
//  IN UINT32 FailedStage - in case of authentication error failed stage description
//
// Output:
//  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID UpdateSetupStrings(
    IN EFI_HII_HANDLE Handle,
    IN EFI_STATUS Error,
    IN UINT32 FailedStage
)
{
    UINTN Size;
    EFI_STRING Template = NULL;
    EFI_STRING Template2 = NULL;
    CHAR16 ReportString[100];

    if(!EFI_ERROR(Error)) {
        GetHiiString(Handle, STRING_TOKEN(STR_SUBTITLE1_SUCCESS), &Size, &Template);
        GetHiiString(Handle, STRING_TOKEN(STR_SUBTITLE2_SUCCESS), &Size, &Template2);

        if(Template != NULL) {
            HiiLibSetString(Handle, STRING_TOKEN(STR_SUBTITLE1), Template);
            pBS->FreePool(Template);
        }

        if(Template2 != NULL) {
            HiiLibSetString(Handle, STRING_TOKEN(STR_SUBTITLE2), Template2);
            pBS->FreePool(Template2);
        }

        return;
    }

//Get Error string template
    GetHiiString(Handle, STRING_TOKEN(STR_SUBTITLE1_ERROR), &Size, &Template);
    if(Template != NULL) {
        HiiLibSetString(Handle, STRING_TOKEN(STR_SUBTITLE1), Template);
        pBS->FreePool(Template);
        Template = NULL;
    }

    GetHiiString(Handle, STRING_TOKEN(STR_SUBTITLE2_ERROR_TEMPLATE), &Size, &Template);
    switch(FailedStage) {
        case InvalidHeader:
            GetHiiString(Handle, STRING_TOKEN(STR_ERR), &Size, &Template2);
            break;
        case InvalidSignature:
            GetHiiString(Handle, STRING_TOKEN(STR_ERR1), &Size, &Template2);
            break;
        case IvalidPlatformKey:
            GetHiiString(Handle, STRING_TOKEN(STR_ERR2), &Size, &Template2);
            break;
        case InvalidFwVersion:
            GetHiiString(Handle, STRING_TOKEN(STR_ERR3), &Size, &Template2);
            break;
        default:
            GetHiiString(Handle, STRING_TOKEN(STR_ERR4), &Size, &Template2);
            break;
    }

    if((Template != NULL) && (Template2 != NULL)) {
        Swprintf(ReportString, Template, Error, Template2);
        HiiLibSetString(Handle, STRING_TOKEN(STR_SUBTITLE2), ReportString);
        pBS->FreePool(Template);
        pBS->FreePool(Template2);
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ReFlashEntry
//
// Description: This is the standard EFI driver entry point called for
//              Recovery flash module initlaization
// Input:       IN EFI_HANDLE ImageHandle - ImageHandle of the loaded driver
//              IN EFI_SYSTEM_TABLE SystemTable - Pointer to the System Table
//
// Output:      EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS EFIAPI ReFlashEntry (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
    static EFI_GUID guidHob = HOB_LIST_GUID;
    static EFI_GUID guidBootFlow = BOOT_FLOW_VARIABLE_GUID;
    UINT32 BootFlow = BOOT_FLOW_CONDITION_RECOVERY;
    EFI_HOB_HANDOFF_INFO_TABLE *pHit;
    UINTN Size;
    UINT32 Attributes;
    UINT32 FailedStage;
    EFI_STATUS RecoveryStatus = EFI_SUCCESS;
    EFI_STATUS Status;
    UINTN AosCapsule = 0;  //<EIP150193+>

    AUTOFLASH AutoFlash = {
        (UINT8)(EFI_SUCCESS), 
        REFLASH_UPDATE_NVRAM, 
        REFLASH_UPDATE_BOOT_BLOCK, 
        REFLASH_UPDATE_MAIN_BLOCK
    };

    ThisImageHandle = ImageHandle;
    InitAmiLib(ImageHandle,SystemTable);

    //Get Boot Mode
    pHit = GetEfiConfigurationTable(pST, &guidHob);
    
    //unload the module if we are not in recovery mode
   // TODO:need to distinguish between recovery and Flash Update
    if (!pHit || (pHit->BootMode != BOOT_IN_RECOVERY_MODE && pHit->BootMode != BOOT_ON_FLASH_UPDATE)) {
        InstallEsrtTable();
        return EFI_UNLOAD_IMAGE;
    }
	// If we are on the flash upadte boot path, apply AFU update settings
    if(pHit->BootMode == BOOT_ON_FLASH_UPDATE) {
		static EFI_GUID FlashUpdGuid = FLASH_UPDATE_GUID;
		AMI_FLASH_UPDATE_BLOCK  FlashUpdDesc;	
    	// Prep the FlashOp variable
        Size = sizeof(AMI_FLASH_UPDATE_BLOCK);
        if(!EFI_ERROR(pRS->GetVariable( FLASH_UPDATE_VAR,&FlashUpdGuid,NULL,&Size, &FlashUpdDesc)))
        {
            AutoFlash.UpdateNv = (FlashUpdDesc.ROMSection & (1<<FV_NV)) ? 1 : 0;
            AutoFlash.UpdateBb = (FlashUpdDesc.ROMSection & (1<<FV_BB)) ? 1 : 0;
            AutoFlash.UpdateMain=(FlashUpdDesc.ROMSection & (1<<FV_MAIN)) ? 1 : 0;
        }
    } // FlashUpdate
    VERIFY_EFI_ERROR(pBS->LocateProtocol(&gFlashProtocolGuid, NULL, &Flash));
//Get Recovery Image verification status
    if(!EFI_ERROR(FindNextHobByGuid(&gAmiRecoveryImageHobGuid, &pHit))) {
        if(((EFI_HOB_GENERIC_HEADER *)pHit)->HobLength < sizeof(RECOVERY_IMAGE_HOB)) {
            //we got update from older Core here
            FailedStage = 0;
            RecoveryStatus = EFI_SUCCESS;
        } else {
            FailedStage = ((RECOVERY_IMAGE_HOB*)pHit)->FailedStage;
            RecoveryStatus = (FailedStage == 0) ? ((RECOVERY_IMAGE_HOB*)pHit)->Status : EFI_SECURITY_VIOLATION;

            //Since RECOVERY_IMAGE_HOB Status field is byte long, we should set error bit by ourselves
            if(RecoveryStatus != 0)
                RecoveryStatus |= EFI_ERROR_BIT;
        }
        RecoveryBuffer = (UINT8 *)(UINTN)((RECOVERY_IMAGE_HOB*)pHit)->Address;
    } else {    //Recovery Hob not found - should not happen, we always create this hob to report errors
        FailedStage = 0;
        RecoveryStatus = EFI_ABORTED;
    }

    AutoFlash.FailedRecovery = (UINT8)RecoveryStatus;

//Update Reflash parameters
    Size = sizeof(AUTOFLASH);
    Attributes = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS;
        pRS->SetVariable(L"Setup", &guidRecovery, Attributes, Size, &AutoFlash);

//Verify if we're on OS firmware update path
    Status = IsWin8Update((EFI_ERROR(RecoveryStatus)) ? TRUE : FALSE);
    if(Status == EFI_SUCCESS || Status == EFI_UNLOAD_IMAGE)
        return Status;
//<EIP150193+> >>>
    Status = pRS->GetVariable(L"CapsuleFromAos", &BiosCapsuleFromAosGuid, NULL, &Size, (VOID *) &AosCapsule);
    if (Status == EFI_SUCCESS) {
      if ((AosCapsule == 1) || (AosCapsule == 2)) {
        return EFI_SUCCESS;
      }
    }
//<EIP150193+> <<<
//Load setup page and create error message if necessary
    LoadResources(ImageHandle, sizeof(SetupCallBack) / sizeof(CALLBACK_INFO), SetupCallBack, NULL);
    ReflashHiiHandle = SetupCallBack[0].HiiHandle;
    pRS->SetVariable(L"BootFlow", &guidBootFlow, EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof(BootFlow), &BootFlow);

    UpdateSetupStrings(ReflashHiiHandle, RecoveryStatus, FailedStage);

    return EFI_SUCCESS;
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
