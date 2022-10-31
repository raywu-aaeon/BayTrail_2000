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
// $Header: /Alaska/SOURCE/Core/CORE_DXE/ConSplitter/ConSplit.c 49    12/15/11 12:15p Felixp $
//
// $Revision: 49 $
//
// $Date: 12/15/11 12:15p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:        ConSplit.c
//
// Description: Console Splitter driver that creates a cetralized input and 
//              output console so that the correct data is going to and coming
//              from the correct devices
//
//<AMI_FHDR_END>
//**********************************************************************

//----------------------------------------------------------------------------

#include "ConSplit.h"
#include <Protocol/DevicePath.h>
#include <Protocol/ConsoleControl.h>
#include <Protocol/HiiDatabase.h>
#include <Setup.h>
#include <Dxe.h>
#include <Hob.h>
#include <Token.h>

//----------------------------------------------------------------------------

EFI_HANDLE		ConSplitHandle = NULL;

DLIST 		ConInList;
DLIST		ConOutList;
DLIST		ConPointerList;
DLIST		KeyNotifyList;

EFI_KEY_TOGGLE_STATE mCSToggleState = TOGGLE_STATE_VALID;
BOOLEAN NumLockSet = FALSE;
static BOOLEAN InitModesTableCalled = FALSE;

extern SIMPLE_TEXT_OUTPUT_MODE	MasterMode;


EFI_STATUS InstallConOutDevice(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *SimpleOut, 
    IN EFI_HANDLE                      Handle
    );

EFI_STATUS InstallConInDevice(
    IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL    *SimpleIn, 
    IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *SimpleInEx, 
    IN AMI_EFIKEYCODE_PROTOCOL           *KeycodeIn,
    IN EFI_HANDLE                        Handle
    );

EFI_STATUS InstallSimplePointerDevice(
    IN EFI_SIMPLE_POINTER_PROTOCOL       *SimplePointer, 
    IN EFI_HANDLE                        Handle
    );

EFI_STATUS ConOutHandleCheck(
    IN EFI_HANDLE Handle
    );

EFI_STATUS ConInHandleCheck(
    IN EFI_HANDLE Handle
    );

VOID CSSetKbLayoutNotifyFn(
    IN EFI_EVENT Event, 
    IN VOID *Context
);

EFI_HII_DATABASE_PROTOCOL *HiiDatabase      = NULL;
EFI_HII_KEYBOARD_LAYOUT *gKeyDescriptorList = NULL;
UINT16 KeyDescriptorListSize                = 0;
static EFI_GUID SetKeyboardLayoutEventGuid = EFI_HII_SET_KEYBOARD_LAYOUT_EVENT_GUID;

EFI_STATUS ConSimplePointerHandleCheck( IN EFI_HANDLE Handle );

EFI_DRIVER_BINDING_PROTOCOL gConSplitterDriverBindingProtocol = {
	CSSupported,
	CSStart,
	CSStop,
	0x10,
	NULL,
	NULL
	};


AMI_MULTI_LANG_SUPPORT_PROTOCOL     gMultiLangSupportProtocol = {
    KeyboardLayoutMap
};

#ifndef EFI_COMPONENT_NAME2_PROTOCOL_GUID //old Core
#ifndef LANGUAGE_CODE_ENGLISH
#define LANGUAGE_CODE_ENGLISH "eng"
#endif
static BOOLEAN LanguageCodesEqual(
    CONST CHAR8* LangCode1, CONST CHAR8* LangCode2
){
    return    LangCode1[0]==LangCode2[0] 
           && LangCode1[1]==LangCode2[1]
           && LangCode1[2]==LangCode2[2];
}
static EFI_GUID gEfiComponentName2ProtocolGuid = EFI_COMPONENT_NAME_PROTOCOL_GUID;
#endif
static CHAR16 *gDriverName=L"AMI Console Splitter Driver";
static CHAR16 *gControllerName=L"AMI Console Splitter";

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Name:  ComponentNameGetControllerName
//
// Description: 
//  EFI_COMPONENT_NAME_PROTOCOL GetControllerName function
//
// Input:       
//  IN EFI_COMPONENT_NAME_PROTOCOL* This - pointer to protocol instance
//  IN EFI_HANDLE Controller - controller handle
//  IN EFI_HANDLE ChildHandle - child handle
//  IN CHAR8* Language - pointer to language description
//  OUT CHAR16** ControllerName - pointer to store pointer to controller name
//
// Output:      
//      EFI_STATUS
//          EFI_SUCCESS - controller name returned
//          EFI_INVALID_PARAMETER - language undefined
//          EFI_UNSUPPORTED - given language not supported
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static EFI_STATUS ComponentNameGetControllerName (
	IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
	IN  EFI_HANDLE                   ControllerHandle,
 	IN  EFI_HANDLE                   ChildHandle        OPTIONAL,
  	IN  CHAR8                        *Language,
  	OUT CHAR16                       **ControllerName 
)
{
	//Supports only English
	if(!Language || !ControllerName || !ControllerHandle)
        return EFI_INVALID_PARAMETER;

	if (   ChildHandle!=ConSplitHandle 
        || !LanguageCodesEqual( Language, LANGUAGE_CODE_ENGLISH)
    ) return EFI_UNSUPPORTED;

	*ControllerName=gControllerName;
	return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Name:   ComponentNameGetDriverName
//
// Description: 
//  EFI_COMPONENT_NAME_PROTOCOL GetDriverName function
//
// Input:       
//  IN EFI_COMPONENT_NAME_PROTOCOL* This - pointer to protocol instance
//  IN CHAR8* Language - pointer to language description
//  OUT CHAR16** DriverName - pointer to store pointer to driver name
//
// Output:      
//  EFI_STATUS
//      EFI_SUCCESS - driver name returned
//      EFI_INVALID_PARAMETER - language undefined
//      EFI_UNSUPPORTED - given language not supported
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static EFI_STATUS ComponentNameGetDriverName(
    IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
	IN  CHAR8                        *Language,
	OUT CHAR16                       **DriverName
)
{
	//Supports only English 
	if(!Language || !DriverName) 
        return EFI_INVALID_PARAMETER;

	if (!LanguageCodesEqual( Language, LANGUAGE_CODE_ENGLISH)) 
        return EFI_UNSUPPORTED;
	else 
        *DriverName=gDriverName;
	
	return EFI_SUCCESS;
}

//Component Name Protocol
static EFI_COMPONENT_NAME2_PROTOCOL gComponentName = {
  ComponentNameGetDriverName,
  ComponentNameGetControllerName,
  LANGUAGE_CODE_ENGLISH
};

EFI_CONSOLE_CONTROL_SCREEN_MODE ScreenMode = EfiConsoleControlScreenText;
BOOLEAN CursorVisible = TRUE;
BOOLEAN StdInLocked   = FALSE;

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: GetMode
//
// Description:
//  This function returns current console mode
//
// Input:   
//  IN EFI_CONSOLE_CONTROL_PROTOCOL *This - pointer to console protocol
//  OUT EFI_CONSOLE_CONTROL_SCREEN_MODE *Mode - placeholder for mode to return
//  OUT OPTIONAL BOOLEAN *UgaExists - if not NULL on return will have current UGA present state
//  OUT OPTIONAL BOOLEAN *StdInLocked - if not NULL on return will have value of STD_IN_LOCKED state
//                                                              
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - function returns correct values
// 
// Modified:
//
// Referrals:
//      ScreenMode
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS GetMode(
	IN  EFI_CONSOLE_CONTROL_PROTOCOL    *This,
	OUT EFI_CONSOLE_CONTROL_SCREEN_MODE *Mode,
	OUT BOOLEAN                         *UgaExists OPTIONAL, 
	OUT BOOLEAN                         *StdInLocked OPTIONAL
)
{
	if (Mode) *Mode = ScreenMode;
	if (UgaExists) *UgaExists = TRUE;
	if (StdInLocked) *StdInLocked = FALSE;
	return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: SetMode
//
// Description:
//  This function sets current console mode
//
// Input:   
//  IN  EFI_CONSOLE_CONTROL_PROTOCOL *This - pointer to console protocol
//  IN  EFI_CONSOLE_CONTROL_SCREEN_MODE Mode - mode to set
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - mode set successfully
//          EFI_INVALID_PARAMETER - incorrect mode given
// 
// Modified:
//      ScreenMode
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS SetMode(
	IN EFI_CONSOLE_CONTROL_PROTOCOL    *This,
	IN EFI_CONSOLE_CONTROL_SCREEN_MODE Mode
)
{
	if (ScreenMode != Mode)
	{
		ScreenMode = Mode;
		if (ScreenMode == EfiConsoleControlScreenText)
		{
			//Restore UGA mode when switching from graphics to text
			DLINK			*ListPtr = ConOutList.pHead;
			CON_SPLIT_OUT 	*SimpleOut;
			while ( ListPtr != NULL)
			{
				SimpleOut = OUTTER(ListPtr, Link, CON_SPLIT_OUT);
				RestoreUgaMode(SimpleOut->Handle);
				ListPtr = ListPtr->pNext;
			}

			if (CursorVisible)
				mCSOutProtocol.EnableCursor(&mCSOutProtocol,TRUE);
		}
		else if (ScreenMode == EfiConsoleControlScreenGraphics)
		{
			DLINK			*ListPtr = ConOutList.pHead;
			CON_SPLIT_OUT 	*SimpleOut;
			CursorVisible = MasterMode.CursorVisible;
			if (CursorVisible)
				mCSOutProtocol.EnableCursor(&mCSOutProtocol,FALSE);
			//Save UGA mode when switching from text to graphics
			while ( ListPtr != NULL)
			{
				SimpleOut = OUTTER(ListPtr, Link, CON_SPLIT_OUT);
				SaveUgaMode(SimpleOut->Handle);
				ListPtr = ListPtr->pNext;
			}

		}
		else return EFI_INVALID_PARAMETER;
	}
	return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: LockStdIn
//
// Description:
//  This function toggles STD_IN_LOCKED state
//
// Input:   
//  IN  EFI_CONSOLE_CONTROL_PROTOCOL *This - pointer to console protocol
//  IN  CHAR16 *Password - pointer to password string
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - function executed successfully
// 
// Modified:
//      StdInLocked
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS LockStdIn(
	IN EFI_CONSOLE_CONTROL_PROTOCOL *This, 
    IN CHAR16 *Password
)
{
	//TODO: add support for the password
	StdInLocked = !StdInLocked;
	return EFI_SUCCESS;
};

EFI_GUID gConsoleControlProtocolGuid = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;

EFI_CONSOLE_CONTROL_PROTOCOL gConsoleControlProtocol = { 
	GetMode, 
    SetMode, 
    LockStdIn
};

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: UpdateSystemTableConOut
//
// Description:
//  This function updates system table ConOut pointer
//
// Input:   
//  IN EFI_EVENT Event - signalled event
//  IN VOID *Context - pointer to event context
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID UpdateSystemTableConOut()
{
	UINT32		CRC32 = 0;

    if( ConOutList.Size==0 ){
        //Initialize all the global variables used by 
        //splitter implementation of TxtOut.
        //When physical ConOut devices are available, the initialization is performed
        //within InitModesTable routine.
        VERIFY_EFI_ERROR(ResizeSplitterBuffer(0));
        VERIFY_EFI_ERROR(pBS->AllocatePool(EfiBootServicesData, sizeof(SUPPORT_RES), (VOID**)&SupportedModes));
        SupportedModes[0].Rows =  25;
	    SupportedModes[0].Columns = 80;
	    SupportedModes[0].AllDevices = TRUE;
    }

	pST->ConOut = &mCSOutProtocol;
	pST->ConsoleOutHandle = ConSplitHandle;
    // We want to initialize ConOut-related fields of the systems table early
    // to workaround bugs in some of the UEFI OpROM drivers
    // that are using pST->ConOut without NULL checking.
    // We are not installing the instance of SimpleTextOut on ConSplitHandle though,
    // because it confuses the logic of TSE notification callbacks.
    // The protocol is installed once all ConOut devices are connected
    // in ReportNoConOutError.
    if (pST->StdErr==NULL){
        pST->StdErr = pST->ConOut;
        pST->StandardErrorHandle  = pST->ConsoleOutHandle;
    }

	// Now calculate the CRC32 value
	pST->Hdr.CRC32 = 0;
	pST->BootServices->CalculateCrc32(pST, sizeof(EFI_SYSTEM_TABLE), &CRC32);
	pST->Hdr.CRC32 = CRC32;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: ReportNoConOutError
//
// Description:
//  This function checks if physical ConOut devices are available.
//  If not, DXE_NO_CON_OUT error is reported.
//
// Input:   
//  IN EFI_EVENT Event - signalled event
//  IN VOID *Context - pointer to event context
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID ReportNoConOutError(
    IN EFI_EVENT Event, 
    IN VOID *Context
)
{
#if REPORT_NO_CON_OUT_ERROR
    DLINK       *Link;
    EFI_STATUS Status;

    //Report error if no ConOut devices available or
    // all console devices are fake devices (without device path).
    for(Link = ConOutList.pHead; Link!=NULL; Link=Link->pNext){
        CON_SPLIT_OUT *SimpleOut = OUTTER(Link, Link, CON_SPLIT_OUT);
        VOID *Dp;

        Status = pBS->HandleProtocol(
            SimpleOut->Handle, &gEfiDevicePathProtocolGuid, &Dp
        );
        if (!EFI_ERROR(Status)) break; // Got one device path
    }
    //Report error if no ConOut devices with device path exists
    if( ConOutList.Size==0 || EFI_ERROR(Status) )
        ERROR_CODE(DXE_NO_CON_OUT, EFI_ERROR_MAJOR);
#endif
	pBS->InstallMultipleProtocolInterfaces (
		&ConSplitHandle, &gEfiSimpleTextOutProtocolGuid, &mCSOutProtocol,
		&gConsoleControlProtocolGuid, &gConsoleControlProtocol,
		NULL
	);
    pBS->CloseEvent(Event);
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: UpdateSystemTableConIn
//
// Description:
//  This function updates system table ConIn pointer
//
// Input:   
//  IN EFI_EVENT Event - signalled event
//  IN VOID *Context - pointer to event context
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID UpdateSystemTableConIn(
    IN EFI_EVENT Event, 
    IN VOID *Context
)
{
	UINT32		CRC32 = 0;

#if REPORT_NO_CON_IN_ERROR
    DLINK       *Link;
    EFI_STATUS Status;

    EFI_HOB_HANDOFF_INFO_TABLE *pHit;
    static EFI_GUID guidHob = HOB_LIST_GUID;

    //Report error if no ConIn devices available or
    // all console devices are fake devices (without device path).
    for(Link = ConInList.pHead; Link!=NULL; Link=Link->pNext){
        CON_SPLIT_IN *SimpleIn = OUTTER(Link, Link, CON_SPLIT_IN);
        VOID *Dp;

        Status = pBS->HandleProtocol(
            SimpleIn->Handle, &gEfiDevicePathProtocolGuid, &Dp
        );
        if (!EFI_ERROR(Status)) break; // Got one device path
    }

	pHit = GetEfiConfigurationTable(pST, &guidHob);
    //Report error if no ConIn devices with device path exists
    if( (ConInList.Size == 0 || EFI_ERROR(Status)) && (pHit->BootMode == BOOT_WITH_FULL_CONFIGURATION))
        ERROR_CODE(DXE_NO_CON_IN, EFI_ERROR_MAJOR);
#endif
	pST->ConIn = &mCSSimpleInProtocol;

	pBS->InstallMultipleProtocolInterfaces (
		&ConSplitHandle,       
        &gEfiSimpleTextInputExProtocolGuid, &mCSSimpleInExProtocol,
        &gAmiEfiKeycodeProtocolGuid, &mCSKeycodeInProtocol,
        &gEfiSimplePointerProtocolGuid, &mCSSimplePointerProtocol, 
        &gEfiSimpleTextInProtocolGuid, &mCSSimpleInProtocol,
		NULL
	);
	
	pST->ConsoleInHandle = ConSplitHandle;

	// Now calculate the CRC32 value
	pST->Hdr.CRC32 = 0;
	pST->BootServices->CalculateCrc32(pST, sizeof(EFI_SYSTEM_TABLE), &CRC32);
	pST->Hdr.CRC32 = CRC32;

	pBS->CloseEvent(Event);
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSEntryPoint
//
// Description:
//  This function is Console splitter driver entry point
//
// Input:   
//  IN EFI_HANDLE ImageHandle - image handle of Console splitter driver
//  IN EFI_SYSTEM_TABLE *SystemTable - pointer to system table
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - driver installed successfully
//          EFI_ERROR - error occured during execution
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS EFIAPI CSEntryPoint(
	IN EFI_HANDLE       ImageHandle,
	IN EFI_SYSTEM_TABLE *SystemTable
)
{
	static EFI_GUID guidConInStarted = CONSOLE_IN_DEVICES_STARTED_PROTOCOL_GUID;
	static EFI_GUID guidConOutStarted = CONSOLE_OUT_DEVICES_STARTED_PROTOCOL_GUID;

	EFI_STATUS	Status;
	EFI_EVENT 	Event;
	VOID *pRegistration;

	// initialize AMI library
	InitAmiLib(ImageHandle, SystemTable);

	// initiaize the ImageHandle and DriverBindingHandle
	gConSplitterDriverBindingProtocol.DriverBindingHandle = NULL;
	gConSplitterDriverBindingProtocol.ImageHandle = ImageHandle;

	// Install driver binding protocol here
	Status = EfiLibInstallDriverBindingComponentName2 (
        ImageHandle,
        SystemTable,
        &gConSplitterDriverBindingProtocol,
        gConSplitterDriverBindingProtocol.DriverBindingHandle,
        NULL,
        &gComponentName
    );
	ASSERT_EFI_ERROR (Status);

	// Create and event for the Simple In Interface
	Status = pBS->CreateEvent (EFI_EVENT_NOTIFY_WAIT, TPL_NOTIFY,
				CSWaitForKey, &mCSSimpleInProtocol,
				&mCSSimpleInProtocol.WaitForKey
				);
	ASSERT_EFI_ERROR (Status);

	// Create and event for the SimpleInEx Interface
	Status = pBS->CreateEvent (EFI_EVENT_NOTIFY_WAIT, TPL_NOTIFY,
				CSWaitForKey, &mCSSimpleInExProtocol,
				&mCSSimpleInExProtocol.WaitForKeyEx
				);
	ASSERT_EFI_ERROR (Status);


	// Create and event for the KeycodeIn Interface
	Status = pBS->CreateEvent (EFI_EVENT_NOTIFY_WAIT, TPL_NOTIFY,
				CSWaitForKey, &mCSKeycodeInProtocol,
				&mCSKeycodeInProtocol.WaitForKeyEx
				);
	ASSERT_EFI_ERROR (Status);

    // Create an event for the SimplePointer Interface
    Status = pBS->CreateEvent(
        EFI_EVENT_NOTIFY_WAIT, 
        TPL_NOTIFY,
        ConSplitterSimplePointerWaitForInput, 
        &mCSSimplePointerProtocol,
        &mCSSimplePointerProtocol.WaitForInput
    );
    ASSERT_EFI_ERROR(Status);

	// Initialize the global lists here 
	DListInit(&ConInList);
	DListInit(&ConOutList);
	DListInit(&ConPointerList);
	DListInit(&KeyNotifyList);

	// Register Protocol Notification to expose 
	// Console Splitter interface only after all consoles initialized
	RegisterProtocolCallback(
		&guidConOutStarted, ReportNoConOutError,
		NULL, &Event,&pRegistration
	);
	RegisterProtocolCallback(
		&guidConInStarted, UpdateSystemTableConIn,
		NULL, &Event,&pRegistration
	);

    //We need a valid handle
    //The only way to create it is to install a protocol
    //Let's install a dummy protocol
	pBS->InstallMultipleProtocolInterfaces (
		&ConSplitHandle,
		&gAmiMultiLangSupportProtocolGuid, &gMultiLangSupportProtocol,
		NULL
	);
	
    //install pST->ConOut
    UpdateSystemTableConOut();

//multi keyboard layout support
	Status = pBS->CreateEventEx(
					EVT_NOTIFY_SIGNAL,
					TPL_CALLBACK,
					CSSetKbLayoutNotifyFn,
					NULL,
					&SetKeyboardLayoutEventGuid,
					&Event);
	CSSetKbLayoutNotifyFn(NULL, NULL);
	
	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSSupported
//
// Description:
//  This function is Console splitter driver Supported function for driver
//  binding protocol
//
// Input:   
//  IN EFI_DRIVER_BINDING_PROTOCOL *This - pointer to driver binding protocol
//  IN EFI_HANDLE ControllerHandle - controller handle to install driver on
//  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath - pointer to device path
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - driver supports given controller
//          EFI_UNSUPPORTED - driver doesn't support given controller
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSSupported (
	IN EFI_DRIVER_BINDING_PROTOCOL *This,
	IN EFI_HANDLE                  ControllerHandle,
	IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
)
{
	EFI_SIMPLE_TEXT_INPUT_PROTOCOL			*SimpleIn = NULL;
	EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL		*SimpleInEx = NULL;
	AMI_EFIKEYCODE_PROTOCOL                 *KeycodeIn = NULL;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*SimpleOut = NULL;
    EFI_SIMPLE_POINTER_PROTOCOL *SimplePointer = NULL;
    EFI_STATUS  SimplePointerStatus;
	EFI_STATUS	OutStatus;
	EFI_STATUS	InStatus;
	EFI_STATUS	InExStatus;
	EFI_STATUS	KeycodeInStatus;
    INT32       Dummy;
	
    // 1. Don't connect to our own handle (ConSplitHandle).
    // 2. Don't connect to pST->ConsoleOutHandle.
    // In a typical scenario pST->ConsoleOutHandle is ConSplitHandle (see UpdateSystemTableConOut above).
    // If  ConSplitHandle has been replaced with another handle, the chances are that the TxtOut associated
    // with this handle is special (f.i. alternative splitter) and installing our splitter on top of it
    // can cause undesired side effects.
    // For example, Shell installs an instance of TxtOut that internally calls our splitter. 
    // An attempt to install our splitter on top of this TxtOut would lead to an endless dispatching loop
    // within the TxtOut member functions implemented in Out.c.
    if (ControllerHandle == ConSplitHandle || ControllerHandle == pST->ConsoleOutHandle)
		return EFI_UNSUPPORTED;

	// check to see if this device has a simple text out protocol installed on it
	OutStatus = pBS->OpenProtocol ( ControllerHandle, &gEfiSimpleTextOutProtocolGuid,
							(VOID**)&SimpleOut, This->DriverBindingHandle,
							ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER );
    if(!EFI_ERROR(OutStatus)) {
        OutStatus = IsModeSupported(SimpleOut, MasterMode.Mode, &Dummy);
        if(EFI_ERROR(OutStatus)) {
            pBS->CloseProtocol(ControllerHandle, &gEfiSimpleTextOutProtocolGuid,
						       This->DriverBindingHandle, ControllerHandle);
        }
    }


	// check to see if this device has a simple input protocol installed on it
	InStatus = pBS->OpenProtocol ( ControllerHandle, &gEfiSimpleTextInProtocolGuid,
							(VOID**)&SimpleIn, This->DriverBindingHandle,
							ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER );
    InExStatus = pBS->OpenProtocol ( ControllerHandle, &gEfiSimpleTextInputExProtocolGuid,
							(VOID**)&SimpleInEx, This->DriverBindingHandle,
							ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER );
    KeycodeInStatus = pBS->OpenProtocol ( ControllerHandle, &gAmiEfiKeycodeProtocolGuid,
							(VOID**)&KeycodeIn, This->DriverBindingHandle,
							ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER );

    // check if device has simple pointer protocol installed on it
    SimplePointerStatus = pBS->OpenProtocol(
        ControllerHandle, 
        &gEfiSimplePointerProtocolGuid,
        (VOID**)&SimplePointer, 
        This->DriverBindingHandle,
        ControllerHandle, 
        EFI_OPEN_PROTOCOL_BY_DRIVER 
    );
    if (!EFI_ERROR(SimplePointerStatus))
        pBS->CloseProtocol(
            ControllerHandle,
            &gEfiSimplePointerProtocolGuid,
            This->DriverBindingHandle,
            ControllerHandle
        );

	if (!EFI_ERROR(OutStatus))
		pBS->CloseProtocol(ControllerHandle, &gEfiSimpleTextOutProtocolGuid,
						   This->DriverBindingHandle, ControllerHandle);

	if (!EFI_ERROR(InStatus))
		pBS->CloseProtocol(ControllerHandle, &gEfiSimpleTextInProtocolGuid,
						   This->DriverBindingHandle, ControllerHandle);

	if (!EFI_ERROR(InExStatus))
		pBS->CloseProtocol(ControllerHandle, &gAmiEfiKeycodeProtocolGuid,
						   This->DriverBindingHandle, ControllerHandle);

	if (!EFI_ERROR(KeycodeInStatus))
		pBS->CloseProtocol(ControllerHandle, &gEfiSimpleTextInputExProtocolGuid,
						   This->DriverBindingHandle, ControllerHandle);

	if ( EFI_ERROR(SimplePointerStatus) && 
         EFI_ERROR(OutStatus) && 
         EFI_ERROR(InStatus) && 
         EFI_ERROR(InExStatus) && 
         EFI_ERROR(KeycodeInStatus) )
		return EFI_UNSUPPORTED;

	return EFI_SUCCESS;
}


// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSStart
//
// Description:
//  This function is Console splitter driver Start function for driver
//  binding protocol
//
// Input:   
//  IN EFI_DRIVER_BINDING_PROTOCOL *This - pointer to driver binding protocol
//  IN EFI_HANDLE ControllerHandle - controller handle to install driver on
//  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath - pointer to device path
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - driver started successfully
//          EFI_UNSUPPORTED - driver didn't start
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSStart(
	IN EFI_DRIVER_BINDING_PROTOCOL *This,
	IN EFI_HANDLE                  ControllerHandle,
	IN EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath
)
{
	EFI_SIMPLE_TEXT_INPUT_PROTOCOL			*SimpleIn = NULL;
	EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL		*SimpleInEx = NULL;
	AMI_EFIKEYCODE_PROTOCOL                 *KeycodeIn = NULL;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*SimpleOut = NULL;
    EFI_SIMPLE_POINTER_PROTOCOL *SimplePointer = NULL;
    EFI_STATUS  Status;
	EFI_STATUS	OutStatus;
	EFI_STATUS	InStatus;
	EFI_STATUS	InExStatus;
	EFI_STATUS	KeycodeInStatus;
    SETUP_DATA *SetupData = NULL;
    UINTN VariableSize = 0;	
    EFI_GUID SetupGuid = SETUP_GUID;

	// grab the pointers for the ConIn, ConOut, and StdErr from the System Table
	// install the current handles for these devices.
	
	// if Simple In, add the Con In device to the list and 
	InStatus = pBS->OpenProtocol(ControllerHandle, &gEfiSimpleTextInProtocolGuid,
								(VOID**)&SimpleIn, This->DriverBindingHandle,
								ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER );
	InExStatus = pBS->OpenProtocol(ControllerHandle, &gEfiSimpleTextInputExProtocolGuid,
								(VOID**)&SimpleInEx, This->DriverBindingHandle,
								ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER );
    KeycodeInStatus = pBS->OpenProtocol(ControllerHandle, &gAmiEfiKeycodeProtocolGuid,
								(VOID**)&KeycodeIn, This->DriverBindingHandle,
								ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER );

	if (!EFI_ERROR(InStatus) || !EFI_ERROR(InExStatus) || !EFI_ERROR(KeycodeInStatus))
	{
		InStatus = InstallConInDevice(SimpleIn, SimpleInEx, KeycodeIn, ControllerHandle);
        if(EFI_ERROR(InStatus)) {
            pBS->CloseProtocol(ControllerHandle, &gEfiSimpleTextInProtocolGuid,
                               This->DriverBindingHandle, ControllerHandle);
            pBS->CloseProtocol(ControllerHandle, &gEfiSimpleTextInputExProtocolGuid,
                               This->DriverBindingHandle, ControllerHandle);
            pBS->CloseProtocol(ControllerHandle, &gAmiEfiKeycodeProtocolGuid,
                               This->DriverBindingHandle, ControllerHandle);
		    if (InStatus == EFI_OUT_OF_RESOURCES)
			    return InStatus;
        } else {
			pBS->OpenProtocol(ControllerHandle, &gEfiSimpleTextInProtocolGuid,
						      (VOID**)&SimpleIn, This->DriverBindingHandle,
						      ConSplitHandle, EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER );
        }
	}			

    Status = pBS->OpenProtocol(
        ControllerHandle, 
        &gEfiSimplePointerProtocolGuid,
        (VOID**)&SimplePointer, 
        This->DriverBindingHandle,
        ControllerHandle, 
        EFI_OPEN_PROTOCOL_BY_DRIVER 
    );
    if (!EFI_ERROR(Status)) {
        Status = InstallSimplePointerDevice( SimplePointer, ControllerHandle );
        if (!EFI_ERROR(Status)) {
            Status = pBS->OpenProtocol(
                ControllerHandle, 
                &gEfiSimplePointerProtocolGuid,
                (VOID**)&SimplePointer, 
                This->DriverBindingHandle,
                ConSplitHandle, 
                EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 
            );
        } else {
            pBS->CloseProtocol(ControllerHandle, &gEfiSimplePointerProtocolGuid,
                               This->DriverBindingHandle, ControllerHandle);
            if (Status == EFI_OUT_OF_RESOURCES)
                return Status;
        }
    }

	// if it has a simple text out add the Con Out device to the list and 
	OutStatus = pBS->OpenProtocol(ControllerHandle, &gEfiSimpleTextOutProtocolGuid,
								(VOID**)&SimpleOut, This->DriverBindingHandle,
								ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER );
	if (!EFI_ERROR(OutStatus) )
	{
		OutStatus = InstallConOutDevice(SimpleOut, ControllerHandle);
		if (EFI_ERROR(OutStatus)) {
            pBS->CloseProtocol(ControllerHandle, &gEfiSimpleTextOutProtocolGuid,
                        This->DriverBindingHandle, ControllerHandle);
			return OutStatus;
        } else {
			RestoreTheScreen(ControllerHandle,SimpleOut);
			pBS->OpenProtocol(ControllerHandle, &gEfiSimpleTextOutProtocolGuid,
						  	  (VOID**)&SimpleOut, This->DriverBindingHandle,
						      ConSplitHandle, EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER );
		}
	}			
	
	// If no devices were installed, then Houston we have a problem
	if ( EFI_ERROR(OutStatus) && EFI_ERROR(InStatus) && EFI_ERROR(Status) )
		return EFI_UNSUPPORTED;

    // Lighten up the keyboard(s) lights
    if(NumLockSet == FALSE) {
    Status = GetEfiVariable(L"Setup", &SetupGuid, NULL, &VariableSize, (VOID**)&SetupData);	
    if (!EFI_ERROR(Status)) {
        if (SetupData->Numlock) {
            mCSToggleState |= NUM_LOCK_ACTIVE;
            }
        }
        NumLockSet=TRUE;
    }
    pBS->FreePool(SetupData);

    CSInSetState ( NULL, &mCSToggleState );

	return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSStop
//
// Description:
//  This function is Console splitter driver Stop function for driver
//  binding protocol
//
// Input:   
//  IN EFI_DRIVER_BINDING_PROTOCOL *This - pointer to driver binding protocol
//  IN EFI_HANDLE ControllerHandle - controller handle to install driver on
//  IN UINTN NumberOfChildren - number of childs on this handle
//  IN OPTIONAL EFI_HANDLE *ChildHandleBuffer - pointer to child handles array
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - driver stopped successfully
//          EFI_INVALID_PARAMETER - invalid values passed for NumberOfChildren or
//                                  ChildHandleBuffer
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSStop (
    IN EFI_DRIVER_BINDING_PROTOCOL *This,
    IN EFI_HANDLE                  ControllerHandle,
    IN UINTN                       NumberOfChildren,
    IN EFI_HANDLE                  *ChildHandleBuffer OPTIONAL
)
{
    EFI_STATUS Status = EFI_UNSUPPORTED;
    DLINK *ListPtr;
    CON_SPLIT_IN *SimpleIn;
    CON_SPLIT_OUT *SimpleOut;
    CON_SPLIT_SIMPLE_POINTER *SimplePointer = NULL;
    BOOLEAN Stopped = FALSE;
    KEY_NOTIFY_LINK *Link;
    VOID *Free;
    
    if (NumberOfChildren == 0) 
        return EFI_SUCCESS;

    if ( NumberOfChildren != 1 || 
         ChildHandleBuffer == NULL ||
         *ChildHandleBuffer!= ConSplitHandle ) 
        return EFI_INVALID_PARAMETER;

    // remove simple text out, simple in, simple pointer
    ListPtr = ConOutList.pHead;
    while ( ListPtr != NULL)
    {
        SimpleOut = OUTTER(ListPtr, Link, CON_SPLIT_OUT);
        if ( SimpleOut->Handle == ControllerHandle)
        {
            pBS->CloseProtocol(ControllerHandle, &gEfiSimpleTextOutProtocolGuid,
                        This->DriverBindingHandle, ControllerHandle);

            pBS->CloseProtocol(ControllerHandle, &gEfiSimpleTextOutProtocolGuid,
                        This->DriverBindingHandle, ConSplitHandle);
            DListDelete(&ConOutList, ListPtr);
            Stopped = TRUE;
            SaveTheScreen(ControllerHandle, SimpleOut->SimpleOut);
            Status = pBS->FreePool(SimpleOut);
            break;
        }
        
        ListPtr = ListPtr->pNext;
    }

    //If we already populated pST->ConOut preserve
    //screen buffer and list of supported modes
    //to keep using it when ConOut devices are connected
    if(ConOutList.Size == 0 && !pST->ConOut) //all devices stops
    {
        if(SupportedModes != NULL) 
        { 
            pBS->FreePool(SupportedModes); 
            SupportedModes = NULL; 
        }

        if(ScreenBuffer != NULL) 
        { 
            pBS->FreePool(ScreenBuffer); 
            ScreenBuffer = NULL; 
        }

        if(AttributeBuffer != NULL) 
        { 
            pBS->FreePool(AttributeBuffer); 
            AttributeBuffer = NULL; 
        }

        MasterMode.Mode=0;
    } else {
        if(Stopped && ConOutList.Size > 0) //re-initialize supported modes buffer if at least one child was stopped
            AdjustSupportedModes();
    }

    ListPtr = ConInList.pHead;
    while ( ListPtr != NULL)
    {
        SimpleIn = OUTTER(ListPtr, Link, CON_SPLIT_IN);
        if ( SimpleIn->Handle == ControllerHandle)
        {

            KeyNotifyRemoveChild(SimpleIn);

            DListDelete(&ConInList, ListPtr);

            if (ConInList.Size == 0) {    //no children left - cleanup KeyNotifyList
                Link = (KEY_NOTIFY_LINK *)KeyNotifyList.pHead;
                while(Link != NULL) {
                    Free = (VOID *)Link;
                    Link = (KEY_NOTIFY_LINK *)Link->Link.pNext;
                    pBS->FreePool(Free);
                }
                DListInit(&KeyNotifyList);  //reset linked list
            }

            pBS->CloseProtocol(ControllerHandle, &gEfiSimpleTextInProtocolGuid,
                        This->DriverBindingHandle, ControllerHandle);

            pBS->CloseProtocol(ControllerHandle, &gEfiSimpleTextInputExProtocolGuid,
                        This->DriverBindingHandle, ControllerHandle);

            pBS->CloseProtocol(ControllerHandle, &gAmiEfiKeycodeProtocolGuid,
                        This->DriverBindingHandle, ControllerHandle);

            pBS->CloseProtocol(ControllerHandle, &gEfiSimpleTextInProtocolGuid,
                        This->DriverBindingHandle, ConSplitHandle);

            Status = pBS->FreePool(SimpleIn);
            break;
        }
        
        ListPtr = ListPtr->pNext;
    }

    ListPtr = ConPointerList.pHead;
    while (ListPtr != NULL) {
        SimplePointer = OUTTER(ListPtr, Link, CON_SPLIT_SIMPLE_POINTER);
        if ( SimplePointer->Handle == ControllerHandle ) {
            DListDelete(&ConPointerList, ListPtr);

            pBS->CloseProtocol(
                ControllerHandle, 
                &gEfiSimplePointerProtocolGuid,
                This->DriverBindingHandle, 
                ControllerHandle
            );
            
            pBS->CloseProtocol(
                ControllerHandle, 
                &gEfiSimplePointerProtocolGuid,
                This->DriverBindingHandle, 
                ConSplitHandle
            );
            Status = pBS->FreePool(SimplePointer);
            break;
        }
        ListPtr = ListPtr->pNext;
    }

    return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: UpdateConVar
//
// Description:
//  This function stores device path of given controller in EFI variable with
//  name sVarName
//
// Input:   
//  IN EFI_HANDLE Controller - controller, which device path to store
//  IN CHAR16 *sVarName - name of EFI variable to store device path under
//
// Output:
//      VOID
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID UpdateConVar(
    IN EFI_HANDLE Controller, 
    IN CHAR16     *sVarName
)
{
	static EFI_GUID guidDevicePath = EFI_DEVICE_PATH_PROTOCOL_GUID;
	static EFI_GUID guidEfiVar = EFI_GLOBAL_VARIABLE;
	EFI_DEVICE_PATH_PROTOCOL *pDevicePath, *pConDev = NULL;
	EFI_STATUS Status;
	UINTN DataSize=0;
	UINT32 Attributes;

	Status = pBS->HandleProtocol(Controller,&guidDevicePath, (VOID**)&pDevicePath);
	if (EFI_ERROR(Status)) 
        return;
    
	Status = GetEfiVariable(sVarName, &guidEfiVar, &Attributes, &DataSize, (VOID**)&pConDev);
	if (EFI_ERROR(Status))
	{
		if (Status!=EFI_NOT_FOUND) 
            return;

		DataSize = DPLength(pDevicePath);
		pRS->SetVariable(sVarName, &guidEfiVar, 
                        EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS, 
                        DataSize, pDevicePath);
		return;
	}

	if (!DPIsOneOf(pConDev, pDevicePath, FALSE))
	{
		EFI_DEVICE_PATH_PROTOCOL *pNewConDev = DPAddInstance(pConDev, pDevicePath);
		DataSize = DPLength(pNewConDev);
		pRS->SetVariable(sVarName, &guidEfiVar, Attributes, DataSize, pNewConDev);
		pBS->FreePool(pNewConDev);
	}

	pBS->FreePool(pConDev);
}


// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: InstallSimplePointerDevice
//
// Description:
//  This function adds another ConIn device to splitter
//
// Input:   
//  *SimpleIn - pointer to new protocol
//  *SimpleInEx - pointer to new extended protocol
//  *KeycodeIn - pointer to AMI key code protocol
//  Handle - handle of new device
//
// Output:
//          EFI_SUCCESS - device added successfully
//          EFI_UNSUPPORTED - device not supported
//          EFI_OUT_OF_RESOURCES - not enough resources to perform operation
// 
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS InstallSimplePointerDevice(
    IN EFI_SIMPLE_POINTER_PROTOCOL *SimplePointer, 
    IN EFI_HANDLE Handle ) 
{
    EFI_STATUS Status;
    CON_SPLIT_SIMPLE_POINTER *ConSimplePointer = NULL;

    if (EFI_ERROR(ConSimplePointerHandleCheck(Handle)))
        return EFI_UNSUPPORTED;

    Status = pBS->AllocatePool(
        EfiBootServicesData, 
        sizeof(CON_SPLIT_SIMPLE_POINTER), 
        (VOID**)&ConSimplePointer
    );
    if (EFI_ERROR(Status))
        return EFI_OUT_OF_RESOURCES;

    ConSimplePointer->SimplePointer = SimplePointer;
    ConSimplePointer->Handle = Handle;
    mCSSimplePointerProtocol.Mode->LeftButton |= SimplePointer->Mode->LeftButton;
    mCSSimplePointerProtocol.Mode->RightButton |= SimplePointer->Mode->RightButton;
	DListAdd(&ConPointerList, &ConSimplePointer->Link);	
    return EFI_SUCCESS;
}


// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: InstallConOutDevice
//
// Description:
//  This function adds another ConOut device to splitter
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *SimpleOut - pointer to new protocol
//  IN EFI_HANDLE Handle - handle of new device
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - device added successfully
//          EFI_UNSUPPORTED - device not supported
//          EFI_OUT_OF_RESOURCES - not enough resources to perform operation
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS InstallConOutDevice(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *SimpleOut, 
    IN EFI_HANDLE                      Handle
)
{
	EFI_STATUS		Status;
	CON_SPLIT_OUT	*ConOut;
	INT32	DeviceModeNumber = 0;
    BOOLEAN FirstChild = FALSE;

	if (EFI_ERROR(ConOutHandleCheck(Handle)))
		return EFI_UNSUPPORTED;

	if(MasterMode.Mode != 0) //already in extended mode
		if (EFI_ERROR(IsModeSupported(SimpleOut, MasterMode.Mode, &DeviceModeNumber)))
			return EFI_UNSUPPORTED;	//device doesn't support current mode - do not include into list


    if (ConOutList.Size==0 && !InitModesTableCalled)     //this is first call
	{
        FirstChild = TRUE;
        /// check to see if platform engineer has overridden the default value of the token
        /// If it has been set to FALSE, then change the value of the MasterMode.CursorVisible
        /// field.  This will only be done for the First Child.  All the rest should follow 
        /// from this device
        if (PcdGetBool(PcdDefaultCursorState) == FALSE)
               MasterMode.CursorVisible = 0;
            
		Status = InitModesTable(SimpleOut, Handle);
		if(EFI_ERROR(Status)) //first device becomes master
			return Status;
	}
	else 
        UpdateModesTable(SimpleOut, Handle); //all next devices

		
	Status = pBS->AllocatePool(EfiBootServicesData, sizeof(CON_SPLIT_OUT), (VOID**)&ConOut);
	if (EFI_ERROR(Status))
		return EFI_OUT_OF_RESOURCES;

	ConOut->SimpleOut = SimpleOut;

	ConOut->Handle = Handle;

	DListAdd(&ConOutList, &(ConOut->Link));	

    if(!FirstChild) {
	// set child display to a current master mode
	    SimpleOut->SetMode(SimpleOut, DeviceModeNumber);
        SimpleOut->EnableCursor(SimpleOut, MasterMode.CursorVisible);
    }
	UpdateConVar(Handle, L"ConOutDev");
	return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: InstallConInDevice
//
// Description:
//  This function adds another ConIn device to splitter
//
// Input:   
//  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *SimpleIn - pointer to new protocol
//  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *SimpleInEx - pointer to new extended
//                                                     protocol
//  IN AMI_EFIKEYCODE_PROTOCOL *KeycodeIn - pointer to AMI key code protocol
//  IN EFI_HANDLE Handle - handle of new device
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - device added successfully
//          EFI_UNSUPPORTED - device not supported
//          EFI_OUT_OF_RESOURCES - not enough resources to perform operation
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS InstallConInDevice(
    IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL    *SimpleIn, 
    IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *SimpleInEx, 
    IN AMI_EFIKEYCODE_PROTOCOL           *KeycodeIn, 
    IN EFI_HANDLE                        Handle
)
{
	EFI_STATUS	Status;
	CON_SPLIT_IN	*ConIn;
	
	if (EFI_ERROR(ConInHandleCheck(Handle)))
		return EFI_UNSUPPORTED;
	
	Status = pBS->AllocatePool(EfiBootServicesData, sizeof(CON_SPLIT_IN), (VOID**)&ConIn);
	if (EFI_ERROR(Status))
		return EFI_OUT_OF_RESOURCES;
		
	ConIn->SimpleIn = SimpleIn;
	ConIn->SimpleInEx = SimpleInEx;
	ConIn->KeycodeInEx = KeycodeIn;

	ConIn->Handle = Handle;

    KeyNotifyAddChild(ConIn);
	
	DListAdd(&ConInList, &(ConIn->Link));	

	UpdateConVar(Handle, L"ConInDev");	

	return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: ConOutHandleCheck
//
// Description:
//  This function checks if ConOut device already present in Splitter
//
// Input:   
//  IN EFI_HANDLE Handle - handle of device to check
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - device not present
//          EFI_UNSUPPORTED - device already present
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS ConOutHandleCheck(
    IN EFI_HANDLE Handle
)
{
	CON_SPLIT_OUT *SimpleOut = OUTTER(ConOutList.pHead, Link, CON_SPLIT_OUT);
	
	// if the list is empty return the status that was passed in 
	if (SimpleOut == NULL)
		return EFI_SUCCESS;

	// check for a handle that was already identified
	while ( SimpleOut != NULL)
	{
		// check the handle
		if (SimpleOut->Handle == Handle)
			return EFI_UNSUPPORTED;
		// go to the next element in the list
		SimpleOut = OUTTER(SimpleOut->Link.pNext, Link, CON_SPLIT_OUT);
	}

	// if it is a new handle return the status pass in
	return EFI_SUCCESS;		
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: ConInHandleCheck
//
// Description:
//  This function checks if ConIn device already present in Splitter
//
// Input:   
//  IN EFI_HANDLE Handle - handle of device to check
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - device not present
//          EFI_UNSUPPORTED - device already present
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS ConInHandleCheck(
    IN EFI_HANDLE Handle
)
{
	CON_SPLIT_IN *SimpleIn = OUTTER(ConInList.pHead, Link, CON_SPLIT_IN);
	
	// if the list is empty return the status that was passed in 
	if (SimpleIn == NULL)
		return EFI_SUCCESS;

	// check for a handle that was already identified
	while ( SimpleIn != NULL)
	{
		// check the handle
		if (SimpleIn->Handle == Handle)
			return EFI_UNSUPPORTED;
		// go to the next element in the list
		SimpleIn = OUTTER(SimpleIn->Link.pNext, Link, CON_SPLIT_IN);
	}

	// if it is a new handle return the status pass in
	return EFI_SUCCESS;		
}


// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: ConSimplePointerHandleCheck
//
// Description:
//  This function checks if ConIn device already present in Splitter.
//
// Input:   Handle - handle of device to check
//
// Output:
//          EFI_SUCCESS - device not present
//          EFI_UNSUPPORTED - device already present
// 
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS ConSimplePointerHandleCheck( IN EFI_HANDLE Handle )
{
    CON_SPLIT_SIMPLE_POINTER *SimplePointer;

    SimplePointer = OUTTER(ConPointerList.pHead, Link, CON_SPLIT_SIMPLE_POINTER);

    // if the list is empty return the status that was passed in 
    if (SimplePointer == NULL)
        return EFI_SUCCESS;

    // check for a handle that was already identified
    while ( SimplePointer != NULL) {

        // check the handle
        if (SimplePointer->Handle == Handle)
            return EFI_UNSUPPORTED;

        // go to the next element in the list
        SimplePointer = OUTTER(SimplePointer->Link.pNext, Link, CON_SPLIT_SIMPLE_POINTER);
    }

    // if it is a new handle return the status pass in
    return EFI_SUCCESS;
}


// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: InitModesTable
//
// Description:
//  This function fills the SupportedModes table with modes supported by first 
//  simple text out device
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol
//  IN EFI_HANDLE Handle - handle of first ConOut device
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - table filled successfully
//          EFI_OUT_OF_RESOURCES - not enough resources to perform operation
// 
// Modified:    SupportedModes, MasterMode
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS InitModesTable(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, 
    IN EFI_HANDLE                      Handle
)
{
	INT32 MaxMode;
	INT32 CurrentMode;
	UINTN Rows, Columns;
	INT32 i;
	EFI_STATUS Status;

	if(Handle == ConSplitHandle) 
        return EFI_SUCCESS;

    InitModesTableCalled = TRUE;

	MaxMode = This->Mode->MaxMode;
	CurrentMode = This->Mode->Mode;

    //The SupportedModes structure
    //may have already been initialized in UpdateSystemTableConOut.
    //If this is the case, free the memory before reinitialization.
    if (SupportedModes!=NULL){
        //If SupportedModes is not NULL, 
        //ResizeSplitterBuffer(0) has already been called
        pBS->FreePool(SupportedModes);
        SupportedModes = NULL;
    }

	Status = pBS->AllocatePool(EfiBootServicesData, sizeof(SUPPORT_RES)* MaxMode,
				                (VOID**)&SupportedModes);
	if(EFI_ERROR(Status))
	{
		MasterMode.MaxMode = 1;
		return EFI_SUCCESS;
	}

	MasterMode.MaxMode = MaxMode; //modify default value

	for(i = 0; i < MaxMode; i++)
	{
		Status = This->QueryMode(This, i, &Columns, &Rows);
		SupportedModes[i].Rows = (INT32)Rows;
		SupportedModes[i].Columns = (INT32)Columns;
		SupportedModes[i].AllDevices = EFI_ERROR(Status) ? FALSE : TRUE;
	}

//Make sure MasterMode.Mode <> CurrentMode, otherwise ResizeSplitterBuffer won't do anything
    MasterMode.Mode = CurrentMode + 1;
    Status = ResizeSplitterBuffer(CurrentMode);
    MasterMode.Mode = CurrentMode;
	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: IsModeSupported
//
// Description:
//  This function determines if mode, specified by ModeNum supported by 
//  simple text out device
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol to check
//  IN UINTN ModeNum - mode to check
//  OUT INT32 *DeviceModeNumber - device mode number correspondent to ModeNum
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - mode supported by device
//          EFI_UNSUPPORTED - mode not supported by device
// 
// Modified:
//
// Referrals:   SupportedModes
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS IsModeSupported(
    IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, 
    IN  UINTN                           ModeNum, 
	OUT INT32                           *DeviceModeNumber)
{
	INT32 MaxMode;
	UINTN Rows, Columns;
	INT32 i;
	EFI_STATUS Status;

//first check if ModeNum on device is the same as in Splitter
    Status = This->QueryMode(This, ModeNum , &Columns, &Rows);
    if(!EFI_ERROR(Status) && 
       SupportedModes[ModeNum].Rows == (INT32)Rows &&
       SupportedModes[ModeNum].Columns == (INT32)Columns) {
			*DeviceModeNumber = (INT32)ModeNum;
			return EFI_SUCCESS;
    }

//now check if mode is still supported but with different mode number

	MaxMode = This->Mode->MaxMode;	
	for(i = 0; i < MaxMode; i++)
	{
        if(i == ModeNum)
            continue;

		Status = This->QueryMode(This, i , &Columns, &Rows);
		if (!EFI_ERROR(Status) && \
		    SupportedModes[ModeNum].Rows == (INT32)Rows && \
		    SupportedModes[ModeNum].Columns == (INT32)Columns)
		{
			*DeviceModeNumber = i;
			return EFI_SUCCESS;
		}

	}
	return EFI_UNSUPPORTED;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: UpdateModesTable
//
// Description:
//  This function updates supported modes table when new devices started don't 
//  support some of this modes
//
// Input:   
//  IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This - pointer to protocol to check
//  IN EFI_HANDLE Handle - handle of device
//
// Output:
//      VOID
// 
// Modified:    SupportedModes
//
// Referrals:   
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID UpdateModesTable(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, 
    IN EFI_HANDLE                      Handle
)
{
	INT32 i, ModeNumber;
	EFI_STATUS Status;

	if(Handle == ConSplitHandle) 
        return;

	
	for(i = 0; i < MasterMode.MaxMode; i++)	{
		if(SupportedModes[i].AllDevices == FALSE) continue;
		Status = IsModeSupported(This, i, &ModeNumber);
		SupportedModes[i].AllDevices = EFI_ERROR(Status) ? FALSE : TRUE;
	}

//update MasterMode.MaxMode value based on modes supported by different devices
//lookup for the first mode above 1 not supported by all devices - this will be
//new MaxMode value
    for(i = 2; i < MasterMode.MaxMode; i++) {
        if(SupportedModes[i].AllDevices == FALSE) {
            MasterMode.MaxMode = i;
            break;
        }
    }
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: ResizeSplitterBuffer
//
// Description:
//  This function allocates new buffers when mode changed 
//
// Input:   
//  IN INT32 ModeNum - new mode
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - new buffers allocated
//          EFI_OUT_OF_RESOURCES - not enough resources to perform operation
// 
// Modified:    ScreenBuffer, EndOfTheScreen,AttributeBuffer
//
// Referrals:   SupportedModes, MasterMode
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS ResizeSplitterBuffer(
    IN INT32 ModeNum
)
{
	INT32 Row, Col;
	CHAR16 *NewCharBuffer;
	INT32 *NewAttributeBuffer;
	EFI_STATUS Status;

	if(ModeNum != MasterMode.Mode || SupportedModes == NULL) //check if it is first init
	{

		if(SupportedModes == NULL)
		{
			Row = 25;
			Col = 80;
		}
		else
		{
			Row = SupportedModes[ModeNum].Rows;
			Col = SupportedModes[ModeNum].Columns;
		}

		Status = pBS->AllocatePool(EfiBootServicesData, 
                                    sizeof(CHAR16) * Row * Col, 
                                    (VOID**)&NewCharBuffer);
		if(EFI_ERROR(Status)) 
            return Status;

		Status = pBS->AllocatePool(EfiBootServicesData, 
                                    sizeof(INT32) * Row * Col, 
                                    (VOID**)&NewAttributeBuffer);
		if(EFI_ERROR(Status)) 
		{
			pBS->FreePool(NewCharBuffer);
			return Status;
		}

		if(ScreenBuffer != NULL) 
            pBS->FreePool(ScreenBuffer);
		ScreenBuffer = NewCharBuffer;
		EndOfTheScreen = ScreenBuffer + (Row * Col);

		if(AttributeBuffer != NULL) 
            pBS->FreePool(AttributeBuffer);
		AttributeBuffer = NewAttributeBuffer;
		Columns = Col;
	}
	MemClearScreen();
	return EFI_SUCCESS;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSSetKbLayoutNotifyFn
//
// Description:
//  This function stores a pointer to the current keyboard layout
//
// Input:   
//  IN EFI_EVENT Event - event that caused this function to be called
//  IN VOID *Context - context of the event
//
// Output:
//      VOID
// 
// Modified:
//  gKeyDescriptorList - changed to point to the current EFI_HII_KEYBOARD_LAYOUT
// 						  or, NULL if no layout currently set
//  LayoutLength - set to the size of the current keyboard layout
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

VOID CSSetKbLayoutNotifyFn(
    IN EFI_EVENT Event, 
    IN VOID *Context)
{
    EFI_STATUS Status;

    if(HiiDatabase == NULL) {
	    Status = pBS->LocateProtocol(&gEfiHiiDatabaseProtocolGuid, NULL, (VOID**)&HiiDatabase);
        if(EFI_ERROR(Status))
            return;
    }

    Status = HiiDatabase->GetKeyboardLayout(HiiDatabase, NULL, &KeyDescriptorListSize, gKeyDescriptorList);
    if (Status == EFI_BUFFER_TOO_SMALL) {
        if(gKeyDescriptorList != NULL)
            pBS->FreePool(gKeyDescriptorList);

        Status = pBS->AllocatePool(EfiBootServicesData, KeyDescriptorListSize, (VOID**)&gKeyDescriptorList);
        if(EFI_ERROR(Status)) {
            KeyDescriptorListSize = 0;
            gKeyDescriptorList = NULL;
        } else {
            HiiDatabase->GetKeyboardLayout(HiiDatabase, NULL, &KeyDescriptorListSize, gKeyDescriptorList);
        }
    } else if(Status == EFI_NOT_FOUND) {
        if(gKeyDescriptorList != NULL) {
            pBS->FreePool(gKeyDescriptorList);
            KeyDescriptorListSize = 0;
            gKeyDescriptorList = NULL;
        }
    }
}

VOID AdjustSupportedModes(
    VOID
)
{
    DLINK *ListPtr;
    CON_SPLIT_OUT *SimpleOut;
    EFI_STATUS Status;
    INT32 i;
    UINTN Columns;
    UINTN Rows;

    ListPtr = ConOutList.pHead;
    SimpleOut = OUTTER(ListPtr, Link, CON_SPLIT_OUT);

//re-initialize supported modes buffer
    if(MasterMode.MaxMode < SimpleOut->SimpleOut->Mode->MaxMode) {
        if (SupportedModes != NULL)
            pBS->FreePool(SupportedModes);
        Status = pBS->AllocatePool(EfiBootServicesData, SimpleOut->SimpleOut->Mode->MaxMode * sizeof(SUPPORT_RES), (VOID**)&SupportedModes);
        if(EFI_ERROR(Status))
            return;
    }
    MasterMode.MaxMode = SimpleOut->SimpleOut->Mode->MaxMode;
    for(i = 0; i < MasterMode.MaxMode; i++) {
        Status = SimpleOut->SimpleOut->QueryMode(SimpleOut->SimpleOut, i, &Columns, &Rows);
		SupportedModes[i].Rows = (INT32)Rows;
		SupportedModes[i].Columns = (INT32)Columns;
		SupportedModes[i].AllDevices = EFI_ERROR(Status) ? FALSE : TRUE;
	}

//update supported modes buffer
    ListPtr = ListPtr->pNext;
    while(ListPtr != NULL) {
        SimpleOut = OUTTER(ListPtr, Link, CON_SPLIT_OUT);
        UpdateModesTable(SimpleOut->SimpleOut, SimpleOut->Handle);
        ListPtr = ListPtr->pNext;
    }
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: KeyboardLayoutMap
//
// Description:
//  This function maps an EFI_KEY to a Unicode character, based on the current
//  keyboard layout
//
// Input:   
//  IN AMI_EFI_KEY_DATA *KeyData - pointer to the key data returned by a device
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - key was mapped successfully
//          EFI_NOT_FOUND - the key was not found in the keyboard layout
//			EFI_INVALID_PARAMETER - KeyData is NULL
// 
// Modified:
//  AMI_EFI_KEY_DATA *KeyData - KeyData->Key.UnicodeChar is changed to match
//      the character found in the keyboard layout
//
// Referrals:
//  EFI_HII_KEYBOARD_LAYOUT* gKeyDescriptorList - Pointer to the current
//      keyboard layout
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS KeyboardLayoutMap(
    IN      AMI_MULTI_LANG_SUPPORT_PROTOCOL *This,
    IN OUT  AMI_EFI_KEY_DATA                *KeyData)
{
    EFI_STATUS Status;
    EFI_KEY_DESCRIPTOR *KeyDescriptor; 

    BOOLEAN AltState = FALSE;
    BOOLEAN ShiftKeyState = FALSE;
    BOOLEAN ShiftState = ShiftKeyState;
    
    static UINT16 ModifierIndex = 0xFFFF;
    static CHAR16 ModifierUnicodeChar = 0x0000;

    UINT16 i = 0;

    if(gKeyDescriptorList== NULL) {
        return EFI_NOT_FOUND;
    }

    KeyDescriptor = (EFI_KEY_DESCRIPTOR *)(gKeyDescriptorList + 1);
    
    // check alt status (left alt or right alt)
    if( ((KeyData->KeyState.KeyShiftState)&(RIGHT_ALT_PRESSED|LEFT_ALT_PRESSED)) != 0 )
        AltState = TRUE;

    if( ((KeyData->KeyState.KeyShiftState)&(RIGHT_SHIFT_PRESSED|LEFT_SHIFT_PRESSED)) != 0 )
        ShiftKeyState = TRUE;
        
    Status = EFI_NOT_FOUND;
    if ( (ModifierIndex != 0xFFFF) && (KeyDescriptor[ModifierIndex].Modifier == EFI_NS_KEY_MODIFIER) ) {
        // Previous key had a modifier, we need to find out what to do
        // for now, only handle EFI_NS_KEY_MODIFIER
        for (i = ModifierIndex+1; KeyDescriptor[i].Modifier == EFI_NS_KEY_DEPENDENCY_MODIFIER && i < gKeyDescriptorList->DescriptorCount; i++) {
            if(KeyDescriptor[i].Key == KeyData->EfiKey) {
                if ((KeyDescriptor[i].AffectedAttribute&EFI_AFFECTED_BY_STANDARD_SHIFT) != 0)
                    ShiftState = ShiftKeyState;
                else
                    ShiftState = FALSE;
                // account for cAPS lOCK, only applicable if the affected attribute is set
                if (!AltState && ((KeyDescriptor[i].AffectedAttribute&EFI_AFFECTED_BY_CAPS_LOCK) != 0) && ((KeyData->KeyState.KeyToggleState&CAPS_LOCK_ACTIVE) != 0))
                    ShiftState = !ShiftState;
                    
                if (AltState && ShiftState && (KeyDescriptor[i].ShiftedAltGrUnicode != 0x0000)) {
                    KeyData->Key.UnicodeChar = KeyDescriptor[i].ShiftedAltGrUnicode;
                    Status = EFI_SUCCESS;
                }
                else if (AltState && !ShiftState && (KeyDescriptor[i].AltGrUnicode != 0x0000)) {
                    KeyData->Key.UnicodeChar = KeyDescriptor[i].AltGrUnicode;
                    Status = EFI_SUCCESS;
                }
                else if (!AltState && ShiftState && (KeyDescriptor[i].ShiftedUnicode != 0x0000)) {
                    KeyData->Key.UnicodeChar = KeyDescriptor[i].ShiftedUnicode;
                    Status = EFI_SUCCESS;
                }
                else if (!AltState && !ShiftState && (KeyDescriptor[i].Unicode != 0x0000)) {
                    KeyData->Key.UnicodeChar = KeyDescriptor[i].Unicode;
                    Status = EFI_SUCCESS;
                }
                break;
            }
        }

        if (EFI_ERROR(Status))
            // No match found, just return the deadkey's character
            KeyData->Key.UnicodeChar = ModifierUnicodeChar;
        ModifierIndex = 0xFFFF;
        ModifierUnicodeChar = 0x0000;
        return EFI_SUCCESS;
    }

    // Search the KeyDescriptorList for a matching key
    for(i = 0; i < gKeyDescriptorList->DescriptorCount; i++)
    {
        if(KeyDescriptor[i].Key == KeyData->EfiKey || (KeyDescriptor[i].Key == 0xA5A5 && KeyData->PS2ScanCode == 0x73)) {
            if ((KeyDescriptor[i].AffectedAttribute&EFI_AFFECTED_BY_STANDARD_SHIFT) != 0)
                ShiftState = ShiftKeyState;
            else
                ShiftState = FALSE;
            // account for cAPS lOCK, only applicable if the affected attribute is set
            if (!AltState && ((KeyDescriptor[i].AffectedAttribute&EFI_AFFECTED_BY_CAPS_LOCK) != 0) && ((KeyData->KeyState.KeyToggleState&CAPS_LOCK_ACTIVE) != 0))
                ShiftState = !ShiftState;
            
            switch (KeyDescriptor[i].Modifier) {
            case EFI_NULL_MODIFIER:
                Status = EFI_SUCCESS;
                if (AltState && ShiftState) {
                    KeyData->Key.UnicodeChar = KeyDescriptor[i].ShiftedAltGrUnicode;
                }
                else if (AltState && !ShiftState) {
                    KeyData->Key.UnicodeChar = KeyDescriptor[i].AltGrUnicode;
                }
                else if (!AltState && ShiftState) {
                    KeyData->Key.UnicodeChar = KeyDescriptor[i].ShiftedUnicode;
                }
                else if (!AltState && !ShiftState) {
                    KeyData->Key.UnicodeChar = KeyDescriptor[i].Unicode;
                }
                break;
            case EFI_NS_KEY_MODIFIER:
                Status = EFI_SUCCESS;
                if (AltState && ShiftState && (KeyDescriptor[i].ShiftedAltGrUnicode != 0x0000)) {
                    ModifierIndex = i;
                    ModifierUnicodeChar = KeyDescriptor[i].ShiftedAltGrUnicode;
                    KeyData->Key.UnicodeChar = 0x0000;		// don't return a character yet, the next keypress will determine the correct character
                }
                else if (AltState && !ShiftState && (KeyDescriptor[i].AltGrUnicode != 0x0000)) {
                    ModifierIndex = i;
                    ModifierUnicodeChar = KeyDescriptor[i].AltGrUnicode;
                    KeyData->Key.UnicodeChar = 0x0000;		// don't return a character yet, the next keypress will determine the correct character
                }
                else if (!AltState && ShiftState && (KeyDescriptor[i].ShiftedUnicode != 0x0000)) {
                    ModifierIndex = i;
                    ModifierUnicodeChar = KeyDescriptor[i].ShiftedUnicode;
                    KeyData->Key.UnicodeChar = 0x0000;		// don't return a character yet, the next keypress will determine the correct character
                }
                else if (!AltState && !ShiftState && (KeyDescriptor[i].Unicode != 0x0000)) {
                    ModifierIndex = i;
                    ModifierUnicodeChar = KeyDescriptor[i].Unicode;
                    KeyData->Key.UnicodeChar = 0x0000;		// don't return a character yet, the next keypress will determine the correct character
                }
            default:
            case EFI_NS_KEY_DEPENDENCY_MODIFIER:
                // skip deadkey-dependent modifiers and unknown modifiers
                break;
            }			// switch (KeyDescriptor[i].Modifier)
            
            if (!EFI_ERROR(Status) && (KeyData->Key.UnicodeChar != 0x0000 || ModifierUnicodeChar != 0x0000))
                break;	// successfully mapped a key, break for(...) loop
        }
    }
    return Status;
}

EFI_STATUS KeyNotifyAddChild(
    IN CON_SPLIT_IN *Child
)
{
    KEY_NOTIFY_LINK *NotifyLink;
    UINT32 ChildIndex;
    EFI_STATUS Status;
    EFI_HANDLE NotifyHandle;

    if(ConInList.Size >= MAX_CON_IN_CHILDREN)
        return EFI_OUT_OF_RESOURCES;

    if(KeyNotifyList.Size == 0)     //no callbacks registered
        return EFI_SUCCESS;

    NotifyLink = (KEY_NOTIFY_LINK *)KeyNotifyList.pHead;
    ChildIndex = (UINT32) ConInList.Size;
    while(NotifyLink != NULL) {
        NotifyLink->Children[ChildIndex].Child = Child;
        if(Child->SimpleInEx != NULL) {
            Status = Child->SimpleInEx->RegisterKeyNotify(
                        Child->SimpleInEx, 
                        &(NotifyLink->KeyData), 
                        NotifyLink->NotifyFunction, 
                        &NotifyHandle);
            NotifyLink->Children[ChildIndex].NotifyHandle = (EFI_ERROR(Status)) ? (EFI_HANDLE) UNUSED_NOTIFY_HANDLE : NotifyHandle;
        } else {
            NotifyLink->Children[ChildIndex].NotifyHandle = (EFI_HANDLE) UNUSED_NOTIFY_HANDLE;
        }
        NotifyLink = (KEY_NOTIFY_LINK *)NotifyLink->Link.pNext;
    }
    return Status;   
}

EFI_STATUS KeyNotifyRemoveChild(
    IN CON_SPLIT_IN *Child
)
{
    KEY_NOTIFY_LINK *NotifyLink;
    UINT32 ChildIndex;
    EFI_STATUS Status;

    if(KeyNotifyList.Size == 0)     //no callbacks registered
        return EFI_SUCCESS;

    NotifyLink = (KEY_NOTIFY_LINK *)KeyNotifyList.pHead;
    while(NotifyLink != NULL) {
        for(ChildIndex = 0; ChildIndex < ConInList.Size; ChildIndex++) {
            if(NotifyLink->Children[ChildIndex].Child == Child && 
               Child->SimpleInEx != NULL &&
               NotifyLink->Children[ChildIndex].NotifyHandle != (EFI_HANDLE) UNUSED_NOTIFY_HANDLE) {
                Status = Child->SimpleInEx->UnregisterKeyNotify(Child->SimpleInEx, NotifyLink->Children[ChildIndex].NotifyHandle);
                NotifyLink->Children[ChildIndex].Child = NULL;
                NotifyLink->Children[ChildIndex].NotifyHandle = (EFI_HANDLE) UNUSED_NOTIFY_HANDLE;
            }
        }
        NotifyLink = (KEY_NOTIFY_LINK *)NotifyLink->Link.pNext;
    }
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
