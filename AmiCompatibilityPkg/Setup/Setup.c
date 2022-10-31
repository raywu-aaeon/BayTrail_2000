//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
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
// $Header: /Alaska/BIN/Board/Setup/Setup.c 63    7/01/11 3:15p Artems $
//
// $Revision: 63 $
//
// $Date: 7/01/11 3:15p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    Setup.c
//
// Description: 
//  This file contains supporting functions, data types and data that
//  correspond to the Setup driver.
//
//<AMI_FHDR_END>
//**********************************************************************

//=======================================================================
//  Includes
#include <DXE.h>
#include <AmiDxeLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/DevicePath.h>
#include <Protocol/Cpu.h>
#include <Protocol/SimpleTextOut.h>
#include <AmiHobs.h>
#include <TimeStamp.h>
#include <Protocol/HiiString.h>
#include <Protocol/HiiDatabase.h>
#include "SetupPrivate.h"
#include <SetupCallbackList.h>
// Supported Languages 
static CHAR8* Rfc4646LanguageList[] = {"en-US","fr-FR","es-ES","de-DE","ru-RU","zh-chs","zh-cht","ko-KR","ja-JP","it-IT","da-DK","fi-FI","nl-NL","nb-NO","pt-BR","sv-FI", NULL}; 

//=======================================================================
// MACROs
#define STR CONVERT_TO_WSTRING

#define LastLangCodes L"PlatformLastLangCodes"
//=======================================================================
// GUIDs
static EFI_GUID guidSetup = SETUP_GUID;
static EFI_GUID guidEfiVar = EFI_GLOBAL_VARIABLE;

//=======================================================================
// Module specific global variables
EFI_HANDLE ThisImageHandle = NULL;
static EFI_HII_STRING_PROTOCOL *HiiString=NULL;
static EFI_HII_DATABASE_PROTOCOL *HiiDatabase=NULL;
UINT8 Setup[sizeof(SETUP_DATA)];

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name:    SETUP_CALLBACK
//
// Fields: Type Name Description
//  EFI_FORM_CALLBACK_PROTOCOL Callback - Callback Protocol Instance for the 
//                                        Class and SubClass defined below
//  UINT16 Class - Value defined to identify a particular Hii form 
//  UINT16 SubClass - Secondary value used to uniquely define the an Hii form 
//
// Description:
//  These Data Structure define a structure used to match a specific 
//  Callback Protocol to an HII Form through the use of Class and SubClass 
//  values
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_THDR_END>
typedef struct {
    EFI_HII_CONFIG_ACCESS_PROTOCOL Callback;
    UINT16 Class, SubClass;
} SETUP_CALLBACK;

EFI_STATUS Callback(
    IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This,
    IN EFI_BROWSER_ACTION Action,
    IN EFI_QUESTION_ID KeyValue,
    IN UINT8 Type,
    IN EFI_IFR_TYPE_VALUE *Value,
    OUT EFI_BROWSER_ACTION_REQUEST *ActionRequest
);

//<AMI_GHDR_START>
//----------------------------------------------------------------------------
// Name:    Callback_Protocols
//
// Description:
//  These Variable definitions define the different formsets and what Callback 
//  protocol should be used for each one
//
//----------------------------------------------------------------------------
//<AMI_GHDR_END>
SETUP_CALLBACK SetupCallbackProtocol = {{NULL,NULL,Callback},SETUP_FORM_SET_CLASS,0};

EFI_GUID SetupFormSetGuid = SETUP_FORM_SET_GUID;


//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name:    SetupCallBack
//
// Fields: Type Name Description
//  EFI_GUID* pGuid - GUID used for future references
//  EFI_FORM_CALLBACK_PROTOCOL* pFormCallback - Structure that defines the Callback 
//                                              that occurs for this package
//  UINT16 Class - Formset Class of the Form Callback Protocol passed in
//  UINT16 SubClass - Formset Sub Class of the Form Callback Protocol passed in
//  EFI_HII_HANDLE HiiHandle - handle that identifies used Hii Package
//
// Description:
//  This array contains the different Hii packages that are used in the system
//
// Notes:
//  The HiiHandle is updated in the LoadResources function when the Hii Packages 
//  are loaded
//----------------------------------------------------------------------------
//<AMI_THDR_END>
CALLBACK_INFO SetupCallBack[] = {
    // Last field in every structure will be filled by the Setup
    { &SetupFormSetGuid, &SetupCallbackProtocol.Callback, SETUP_FORM_SET_CLASS, 0, 0}
};

//<AMI_GHDR_START>
//----------------------------------------------------------------------------
// Name:    FormsetVisible
//
// Description:
//  This array contains information that indicates to the system whether or not 
//  a formset is visible when Setup is loaded.
//
// Reference: 
//  SetupCallBack
//
// Notes:
//  Formset index in this array should match with the formset index in the 
//  SetupCallBack array
//
//----------------------------------------------------------------------------
//<AMI_GHDR_END>
BOOLEAN FormsetVisible[] = {
    TRUE, //Setup formset is always displayed
};

#define NUMBER_OF_FORMSETS (sizeof(SetupCallBack)/sizeof(CALLBACK_INFO))

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name:    InitString
//
// Description:
//  This function updates a string defined by the StrRef Parameter in the HII 
//  database with the string and data passed in.
//
// Input:
//  EFI_HII_HANDLE HiiHandle - handle that identifies used Hii Package
//  STRING_REF StrRef - String Token defining string in the database to update
//  CHAR16 *sFormat - string with format descriptors in it
//  ... - extra paramaters that define data that correlate to the format 
//        descriptors in the String
//
// Output:
//  None
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_THDR_END>
VOID InitString(EFI_HII_HANDLE HiiHandle, STRING_REF StrRef, CHAR16 *sFormat, ...)
{
    CHAR16 s[1024];
    VA_LIST  ArgList;
    VA_START(ArgList,sFormat);
    Swprintf_s_va_list(s,sizeof(s),sFormat,ArgList);
    VA_END(ArgList);
    HiiLibSetString(HiiHandle, StrRef, s);
}

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name:    InitMain
//
// Description:
//  This function updates a few generic BIOS strings that are used on the 
//  setup pages.
//
// Input:
//  EFI_HII_HANDLE HiiHandle - handle that identifies used Hii Package
//
// Output:
//  None
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_THDR_END>
VOID InitMain(EFI_HII_HANDLE HiiHandle)
{
    DXE_SERVICES *DxeTable;

    ///////////////// BIOS /////////////////////////////////////
    CHAR16 *FirmwareVendor  =   (pST->FirmwareVendor) 
                              ? pST->FirmwareVendor 
                              : CONVERT_TO_WSTRING(CORE_VENDOR);
    UINT32 FirmwareRevision =  (pST->FirmwareRevision) 
                              ? pST->FirmwareRevision 
                              : CORE_COMBINED_VERSION;

    InitString(
        HiiHandle,STRING_TOKEN(STR_BIOS_VENDOR_VALUE),
        L"%s", FirmwareVendor
    );
    InitString(
        HiiHandle,STRING_TOKEN(STR_BIOS_CORE_VERSION_VALUE),
        L"%d.%03d",
        ((UINT16*)&FirmwareRevision)[1],
        *(UINT16*)&FirmwareRevision
    );
    InitString(
        HiiHandle,STRING_TOKEN(STR_BIOS_VERSION_VALUE),
#if PROJECT_BUILD_NUMBER_IN_TITLE_SUPPORTED && defined (PROJECT_BUILD)
#ifdef EFIx64
        L"%s %d.%02d.%03d x64",
#else
        L"%s %d.%02d.%03d",
#endif
        STR(PROJECT_TAG), PROJECT_MAJOR_VERSION, PROJECT_MINOR_VERSION, PROJECT_BUILD
#else //#if PROJECT_BUILD_NUMBER_IN_TITLE_SUPPORTED && defined (PROJECT_BUILD)
#ifdef EFIx64
        L"%s %d.%02d x64",
#else
        L"%s %d.%02d",
#endif
        STR(PROJECT_TAG), PROJECT_MAJOR_VERSION, PROJECT_MINOR_VERSION
#endif//#if PROJECT_BUILD_NUMBER_IN_TITLE_SUPPORTED && defined (PROJECT_BUILD)
    );
    InitString(
        HiiHandle,STRING_TOKEN(STR_BIOS_DATE_VALUE),
        L"%s %s", L_TODAY, L_NOW
    );
    VERIFY_EFI_ERROR(LibGetDxeSvcTbl(&DxeTable));
    InitString(
        HiiHandle,STRING_TOKEN(STR_BIOS_COMPLIANCY_VALUE),
        L"UEFI %d.%d; PI %d.%d",
        ((UINT16*)&pST->Hdr.Revision)[1],
        ((UINT16*)&pST->Hdr.Revision)[0] / 10,
        ((UINT16*)&DxeTable->Hdr.Revision)[1],
        ((UINT16*)&DxeTable->Hdr.Revision)[0] / 10
    );
}

// Declare list of string initialization functions
typedef VOID (STRING_INIT_FUNC)(
    EFI_HII_HANDLE HiiHandle, UINT16 Class
);
extern STRING_INIT_FUNC SETUP_STRING_INIT_LIST EndOfFunctionList;
STRING_INIT_FUNC *StringInitFunc[] = { SETUP_STRING_INIT_LIST NULL };

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name:    InitStrings
//
// Description:
//  This function is called for each Formset and initializes strings based on 
//  the porting provided and then updates the HII database
//
// Input:
//  EFI_HII_HANDLE HiiHandle - handle that that identifies used Hii Package
//  CALLBACK_INFO *pCallBackFound - pointer to an instance of CALLBACK_INFO 
//                                  that works with HiiHandle
//
// Output:
//  None
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_THDR_END>
VOID InitStrings(
    EFI_HII_HANDLE HiiHandle,
    CALLBACK_INFO *pCallBackFound
)
{
    UINT16  i;
    
    if (!pCallBackFound || !pCallBackFound->HiiHandle) return;

    for (i = 0; StringInitFunc[i]!=NULL; i++) {
        StringInitFunc[i](HiiHandle, pCallBackFound->Class);
    }
    
    switch(pCallBackFound->Class)
    {
        case SETUP_FORM_SET_CLASS:
            InitMain(HiiHandle);
            InitString(
                HiiHandle,STRING_TOKEN(STR_MIN_PASSWORD_LENGTH__VALUE),
                L"%d", PASSWORD_MIN_SIZE
            );
            InitString(
                HiiHandle,STRING_TOKEN(STR_MAX_PASSWORD_LENGTH__VALUE),
                L"%d", PASSWORD_MAX_SIZE
            );
            break;
    }
// AMITODO: Defaults initialization

}

CHAR8* NormalizeLanguageList(CHAR8 *LanguageList, UINTN *LanguageListSize){
	CHAR8 *NewLanguageList, *LangStart, *Lang, *NewLang;
	if (LanguageList==NULL || LanguageListSize==NULL) return NULL;
	NewLanguageList = Malloc((*LanguageListSize/2)*5+1);
	Lang = LanguageList;
	NewLang = NewLanguageList;
	do{
		BOOLEAN HasDash = FALSE;
		UINTN LangSize;
		for(LangStart = Lang; *Lang!=0 && *Lang!=';'; Lang++) if (*Lang=='-') HasDash = TRUE;
		LangSize = Lang-LangStart;
		if (!HasDash){
			UINTN i;
			for(i = 0; Rfc4646LanguageList[i] != NULL; i++){
				if (MemCmp(Rfc4646LanguageList[i],LangStart,LangSize) == 0){
					LangStart = Rfc4646LanguageList[i];
					LangSize = Strlen(Rfc4646LanguageList[i]);
				}
			}
		}
		//copy
		MemCpy(NewLang,LangStart,LangSize);
		NewLang += LangSize;
		*NewLang++ = ';';
		if	(*Lang==';') Lang++;
	} while(*Lang!=0);
	*--NewLang = 0;
	*LanguageListSize = NewLang - NewLanguageList + 1;
	return NewLanguageList;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:    InitLanguages
//
// Description:
//  Determine the current language that will be used based on language
//  related EFI Variables.
//
// Input:
//  EFI_HII_HANDLE HiiHandle - handle that that identifies used Hii Package
//
// Output:
//  None
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID InitLanguages(EFI_HII_HANDLE HiiHandle)
{
    UINTN Size = 0;
    EFI_STATUS Status;
    CHAR8* LangCodes = NULL;
	CHAR8* NormalizedLangCodes;

    if (HiiString == NULL) {
        return;
    }

    Status = HiiString->GetLanguages(HiiString, HiiHandle, LangCodes, &Size);
    if (Status==EFI_BUFFER_TOO_SMALL){
        LangCodes = Malloc(Size);
        Status = HiiString->GetLanguages(HiiString, HiiHandle, LangCodes, &Size);
        if (EFI_ERROR(Status)) pBS->FreePool(LangCodes);
    }
    if (EFI_ERROR(Status)){
        Size = sizeof(CONVERT_TO_STRING(DEFAULT_LANGUAGE_CODE));
        LangCodes = Malloc(Size);
        pBS->CopyMem(
            LangCodes, CONVERT_TO_STRING(DEFAULT_LANGUAGE_CODE), Size
        );
    }
	NormalizedLangCodes = NormalizeLanguageList(LangCodes,&Size);
	if (NormalizedLangCodes==NULL) return;
    //LangCodes is a volatile variable and needs to be initialized during every boot
    //However, this routine is not invoked during every boot,
    //It is always invoked during the first boot.
    //During subsequent boots it is only invoked when user is trying to enter Setup
    //In order to initialize LanCodes when this routine is not invoked, let's create
    //a non-volatile variable LastLangCodes with the copy of LanCodes.
    //This copy is used to initialize LanCodes when this routine is not invoked.
    //(This code is part of SetupEntry routine)
    pRS->SetVariable( LastLangCodes,
                      &guidSetup,
                      EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                      Size,
                      NormalizedLangCodes );

    pBS->FreePool(LangCodes);
	pBS->FreePool(NormalizedLangCodes);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:    SetupCallback
//
// Description:
//  This function publishes all HII resources and initializes the HII databases
//  There is a token ALWAYS_PUBLISH_HII_RESOURCES that would call this function
//  on every boot not just when the user tries to enter Setup
//
// Input:
//  IN EFI_EVENT Event - Event that was triggered
//  IN VOID *Context - data pointer to context information
//
// Output:
//  None
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SetupCallback(IN EFI_EVENT Event, IN VOID *Context)
{
    static BOOLEAN ResourcesLoaded = FALSE;
    UINT32 i;

    if (   !HiiString 
        && EFI_ERROR(pBS->LocateProtocol(
                        &gEfiHiiStringProtocolGuid, NULL, (VOID**)&HiiString
           ))
    ) return ;
    if (   !HiiDatabase 
        && EFI_ERROR(pBS->LocateProtocol(
                        &gEfiHiiDatabaseProtocolGuid, NULL, (VOID**)&HiiDatabase
           ))
    ) return ;
    if (Event) pBS->CloseEvent(Event);
    if (ResourcesLoaded) return;
    ResourcesLoaded = TRUE;
    LoadResources(ThisImageHandle, NUMBER_OF_FORMSETS, SetupCallBack, InitStrings);
    //Get list of available languages and initialize Lang and LangCodes variables
    //All setup packages share the same string pack
    //that's why it is enough to only process single pack in the SetupCallBack array.
    InitLanguages(SetupCallBack[0].HiiHandle);
    //Hide (remove) formsets if necessary
    for(i = 0; i < NUMBER_OF_FORMSETS; i++){
        if (!FormsetVisible[i]) 
            HiiDatabase->RemovePackageList(
                HiiDatabase, SetupCallBack[i].HiiHandle
            );
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    GetLangVariable
//
// Description:
//  This function returns language variable stored in NVRAM
//
// Input:
//  IN CHAR16 *VariableName - Human-readable name of language variable
//  EFI_GUID *VariableGuid - pointer to variable GUID
//  UINTN *VariableSize - pointer to store output buffer size
//
// Output:
//  CHAR8 * - A string that contains list of languages supported by platform
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
CHAR8* GetLangVariable(
    CHAR16 *VariableName, EFI_GUID *VariableGuid, UINTN *VariableSize
){
    UINTN Size = 0;
    CHAR8 *Buffer = NULL;
    EFI_STATUS Status;

    Status = GetEfiVariable(VariableName, VariableGuid, NULL, &Size, (VOID**)&Buffer);
    if (EFI_ERROR(Status)) Buffer=NULL;
    else if (VariableSize!=NULL) *VariableSize=Size;
    return Buffer;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    SetLangCodes
//
// Description:
//  This function stores "PlatformLangCodes" variable
//
// Input:
//  IN CHAR16 *VariableName - Human-readable name of language variable
//  CHAR8* LangBuffer - pointer to list of language codes to store
//  UINTN LangBufferSize - size of the passed buffer
//
// Output:
//  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SetLangCodes(CHAR16 *VariableName, CHAR8* LangBuffer, UINTN LangBufferSize){
    if (LangBuffer==NULL) return;
    pRS->SetVariable(
        VariableName, &guidEfiVar,
        EFI_VARIABLE_BOOTSERVICE_ACCESS |
        EFI_VARIABLE_RUNTIME_ACCESS,
        LangBufferSize, LangBuffer
    );
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    SetLang
//
// Description:
//  This function stores "PlatformLang" variable
//
// Input:
//  IN CHAR16 *VariableName - Human-readable name of language variable
//  CHAR8* LangBuffer - pointer to alanguage code to store
//  UINTN LangBufferSize - size of the passed buffer
//
// Output:
//  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SetLang(CHAR16 *VariableName, CHAR8* LangBuffer, UINTN LangBufferSize){
    if (LangBuffer==NULL) return;
    pRS->SetVariable(
        VariableName, &guidEfiVar,
        EFI_VARIABLE_NON_VOLATILE |
        EFI_VARIABLE_BOOTSERVICE_ACCESS |
        EFI_VARIABLE_RUNTIME_ACCESS,
        LangBufferSize, LangBuffer
    );
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    IsVarisbleExist
//
// Description:
//  Helper routine that checks if a given variable exists
//
// Input:
//  IN CHAR16 *VariableName - Name of the variable to check on
//  IN EFI_GUID *VendorGuid - GUID of the variable to check on
//
// Output:
//  TRUE - if variable exsits
//  FALSE - if variable does not exist
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsVariableExist(IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid){
    UINTN Size=0;
    return 
           pRS->GetVariable(VariableName, VendorGuid, NULL, &Size, NULL)
        == EFI_BUFFER_TOO_SMALL;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    SetupEntry
//
// Description:
//  This function is the entry point for the Setup driver. It registers the 
//  the Setup callback functions and then it checks if the
//  "Setup" and "Lang" variables are defined. If not it is a first boot
//  (first flash or first boot after BIOS upgrade) and these variables will 
//  need to be defined.
//  If "Setup" and "Lang" variables are defined, then make sure the language 
//  variables all agree and then return
//
// Input:
//  IN EFI_HANDLE ImageHandle - Image handle
//  IN EFI_SYSTEM_TABLE *SystemTable - pointer to the UEFI System Table
//
// Output:
//  EFI_SUCCESS
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SetupEntry(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE   *SystemTable
)
{
#if !ALWAYS_PUBLISH_HII_RESOURCES
    static EFI_EVENT SetupEnterEvent;
    static EFI_GUID guidSetupEnter = AMITSE_SETUP_ENTER_GUID;
#endif	
#if FORCE_USER_TO_SETUP_ON_FIRST_BOOT
    static UINT32 BootFlow = BOOT_FLOW_CONDITION_FIRST_BOOT;
    static EFI_GUID guidBootFlow = BOOT_FLOW_VARIABLE_GUID;
#endif

	CHAR8 *LangCodesBuffer;
	UINTN LangCodesSize;

    ThisImageHandle = ImageHandle;
    InitAmiLib(ImageHandle,SystemTable);

    pBS->SetMem(Setup, sizeof(Setup), 0);

    if (!IsVariableExist(LastLangCodes, &guidSetup))
    {   //If LastLangCodes Variable is not found,
        //this is first boot after FW upgrade.
        //We have to submit resources to HII to get Setup defaults
        //and list of supported languages.
        //After that we have to:
        //  1. If Setup variable is missing,
        //     initialize it with Defaults
        //  2. If Lang is missing, initialize
        //     Lang variable.
        //  3. Force user to go to Setup
        //    ( if FORCE_USER_TO_SETUP_ON_FIRST_BOOT SDL token is on).
        SetupCallback(NULL, NULL); //submit resources to HII and get defaults
        //Setup global variable is initialized during SetupCallback
// AMITODO: Defaults initialization
#if FORCE_USER_TO_SETUP_ON_FIRST_BOOT
        if (!IsVariableExist(L"BootFlow", &guidBootFlow)){
            return pRS->SetVariable(
                L"BootFlow", &guidBootFlow,
                EFI_VARIABLE_BOOTSERVICE_ACCESS,
                sizeof(BootFlow), &BootFlow);
        }
#endif
    }
    else
    {   //otherwise
        //  Register setup callbacks to submit resources to HII
        //     only if/when setup is launched
#if ALWAYS_PUBLISH_HII_RESOURCES
        SetupCallback(NULL, NULL);
#else
        VOID *pSetupRegistration;
        RegisterProtocolCallback(
            &guidSetupEnter, SetupCallback,
            NULL,&SetupEnterEvent, &pSetupRegistration
        );
#endif
    }
    //Set PlatformLanCodes variable
    LangCodesBuffer = GetLangVariable(LastLangCodes, &guidSetup, &LangCodesSize);
    SetLangCodes(L"PlatformLangCodes", LangCodesBuffer, LangCodesSize);
    pBS->FreePool(LangCodesBuffer);

    return EFI_SUCCESS;
}

#ifndef SETUP_ITEM_CALLBACK_DEFINED
typedef struct{
    UINT16 Class, SubClass, Key;
    SETUP_ITEM_CALLBACK_HANDLER *UpdateItem;
} SETUP_ITEM_CALLBACK;
#endif

// Brings the definitions of the SDL token defined list of callbacks into this
//  file as a list of functions that can be called
#define ITEM_CALLBACK(Class,Subclass,Key,Callback) Callback
extern SETUP_ITEM_CALLBACK_HANDLER SETUP_ITEM_CALLBACK_LIST EndOfList;
#undef ITEM_CALLBACK

// This creates an array of callbacks to be used 
#define ITEM_CALLBACK(Class,Subclass,Key,Callback) {Class,Subclass,Key,&Callback}
SETUP_ITEM_CALLBACK SetupItemCallback[] = { SETUP_ITEM_CALLBACK_LIST {0,0,0,NULL} };

CALLBACK_PARAMETERS *CallbackParametersPtr = NULL;
CALLBACK_PARAMETERS* GetCallbackParameters(){
    return CallbackParametersPtr;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    Callback
//
// Description:
//  This function is used to identify the function to call when an interactive 
//  item has been triggered in the setup browser based on the information in
//  the Callback protocol and the SetupCallBack Array
//
// Input:
//  IN EFI_FORM_CALLBACK_PROTOCOL *This - Pointer to the instance of the callback 
//                                        protocol
//  IN UINT16 KeyValue - Unique value that defines the type of data to expect in 
//                       the Data parameter
//  IN EFI_IFR_DATA_ARRAY *Data - Data defined by KeyValue Parameter
//  OUT EFI_HII_CALLBACK_PACKET **Packet - Data passed from the Callback back to 
//                                         the setup Browser
//
// Output:
//  EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS Callback(
    IN CONST EFI_HII_CONFIG_ACCESS_PROTOCOL *This,
    IN EFI_BROWSER_ACTION Action,
    IN EFI_QUESTION_ID KeyValue,
    IN UINT8 Type,
    IN EFI_IFR_TYPE_VALUE *Value,
    OUT EFI_BROWSER_ACTION_REQUEST *ActionRequest
)
{
    UINTN i;
    SETUP_CALLBACK *pCallback = (SETUP_CALLBACK*)This;
    CALLBACK_PARAMETERS CallbackParameters;
    EFI_STATUS Status;

    CallbackParameters.This = (VOID*)This;
    CallbackParameters.Action = Action;
    CallbackParameters.KeyValue = KeyValue;
    CallbackParameters.Type = Type;
    CallbackParameters.Value = Value;
    CallbackParameters.ActionRequest = ActionRequest;
    if (ActionRequest) *ActionRequest=EFI_BROWSER_ACTION_REQUEST_NONE;
    Status = EFI_UNSUPPORTED;
    CallbackParametersPtr = &CallbackParameters;
    for(i=0; i<NUMBER_OF_FORMSETS; i++)
        if (SetupCallBack[i].Class == pCallback->Class && SetupCallBack[i].SubClass == pCallback->SubClass)
        {
            SETUP_ITEM_CALLBACK *pItemCallback = SetupItemCallback;
            while(pItemCallback->UpdateItem)
            {
                if (    pItemCallback->Class == pCallback->Class
                    &&  pItemCallback->SubClass == pCallback->SubClass
                    &&  pItemCallback->Key == KeyValue
                ){
                    Status = pItemCallback->UpdateItem(
                        SetupCallBack[i].HiiHandle,
                        pItemCallback->Class, pItemCallback->SubClass,
                        KeyValue
                    );
                    if (Status != EFI_UNSUPPORTED) break;
                }
                pItemCallback++;
            }
        }
    CallbackParametersPtr = NULL;
    return Status;
}
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
