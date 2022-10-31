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
// $Header: /Alaska/SOURCE/Core/CORE_DXE/ConSplitter/In.c 28    9/22/11 6:29a Rameshr $
//
// $Revision: 28 $
//
// $Date: 9/22/11 6:29a $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:        Out.c
//
// Description: File contains the Simple Text Output functionality for the 
//		        Console Splitter Driver
//
//<AMI_FHDR_END>
//**********************************************************************

//----------------------------------------------------------------------------

#include "ConSplit.h"
#include "Token.h"
#include <Protocol/SimplePointer.h>

//----------------------------------------------------------------------------
extern EFI_HII_KEYBOARD_LAYOUT *gKeyDescriptorList;
extern EFI_KEY_TOGGLE_STATE mCSToggleState;
EFI_SIMPLE_POINTER_MODE gSimplePointerMode = {
    0x10000,
    0x10000,
    0x10000,
    FALSE,
    FALSE
};

VOID ConnectInputDevices(
    VOID
);

//----------------------------------------------------------------------------

EFI_SIMPLE_TEXT_INPUT_PROTOCOL mCSSimpleInProtocol = {
	CSInReset,
	CSReadKeyStroke,
	NULL
	} ;

EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL mCSSimpleInExProtocol = 	{
	CSInResetEx,
	CSReadKeyStrokeEx,
	NULL,
    CSInSetState,
    CSInRegisterKeyNotify,
    CSInUnRegisterKeyNotify
	} ;

AMI_EFIKEYCODE_PROTOCOL mCSKeycodeInProtocol = 	{
	CSInResetEx,
	CSReadEfiKey,
	NULL,
    CSInSetState,
    CSInRegisterKeyNotify,
    CSInUnRegisterKeyNotify
	} ;

EFI_SIMPLE_POINTER_PROTOCOL mCSSimplePointerProtocol = {
    ConSplitterSimplePointerReset,
    ConSplitterSimplePointerGetState,
    ConSplitterSimplePointerWaitForInput,
    &gSimplePointerMode
};

//----------------------------------------------------------------------------


// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: ConSplitterSimplePointerReset
//
// Description: Resets the pointer device hardware.
//
// Input:   
//  *This - pointer to protocol instance.
//  ExtendedVerification - Driver may perform diagnostics on reset.
//
// Output:
//          EFI_SUCCESS - device was reset successfully
//          EFI_ERROR - some of devices returned error
// 
//----------------------------------------------------------------------------
// <AMI_PHDR_END>         

EFI_STATUS ConSplitterSimplePointerReset (
    IN  EFI_SIMPLE_POINTER_PROTOCOL *This,
    IN  BOOLEAN ExtendedVerification )
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_STATUS TestStatus;
    CON_SPLIT_SIMPLE_POINTER *ConSimplePointer;

    if (StdInLocked) 
        return EFI_ACCESS_DENIED;

    if (ConPointerList.pHead == NULL)
        return EFI_SUCCESS;

    ConSimplePointer = OUTTER(ConPointerList.pHead, Link, CON_SPLIT_SIMPLE_POINTER);

    // we need to loop through all the registered simple pointer devices
    // and call each of their Reset function
    while (ConSimplePointer != NULL) {
        TestStatus = ConSimplePointer->SimplePointer->Reset(ConSimplePointer->SimplePointer, ExtendedVerification);
        ConSimplePointer = OUTTER( ConSimplePointer->Link.pNext, Link, CON_SPLIT_SIMPLE_POINTER );

        if (EFI_ERROR(TestStatus))
            Status = TestStatus;
    }

    return Status;
}


// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: ConSplitterSimplePointerGetState
//
// Description: Retrieves the current state of a pointer device.
//  Reads the next keystroke from the input device. The WaitForKey Event can
//  be used to test for existance of a keystroke via WaitForEvent () call.
//  If the ConIn is password locked make it look like no keystroke is availible
//
// Input:   
//    This   - Protocol instance pointer.
//    State  - A pointer to the state information on the pointer device.
//
// Output:
//    EFI_SUCCESS       - The keystroke information was returned.
//    EFI_NOT_READY     - There was no keystroke data availiable.
//    EFI_DEVICE_ERROR  - The keydtroke information was not returned due to
//                        hardware errors.
// 
//----------------------------------------------------------------------------
// <AMI_PHDR_END>         

EFI_STATUS ConSplitterSimplePointerGetState(
    IN  EFI_SIMPLE_POINTER_PROTOCOL *This,
    IN OUT EFI_SIMPLE_POINTER_STATE *State )
{
    EFI_STATUS Status; 
    EFI_SIMPLE_POINTER_STATE  CurrentState;
    CON_SPLIT_SIMPLE_POINTER *ConSimplePointer;
    BOOLEAN EfiSuccessDetected = FALSE;
    BOOLEAN EfiDeviceErrorDetected = FALSE;

    if (StdInLocked) 
        return EFI_ACCESS_DENIED;

    State->RelativeMovementX  = 0;
    State->RelativeMovementY  = 0;
    State->RelativeMovementZ  = 0;
    State->LeftButton         = FALSE;
    State->RightButton        = FALSE;

    if (ConPointerList.pHead == NULL)   //if no device attached return success with no movement
        return EFI_NOT_READY;

    ConSimplePointer = OUTTER( ConPointerList.pHead, Link, CON_SPLIT_SIMPLE_POINTER );
    
    // we need to loop through all the registered simple pointer devices
    while (ConSimplePointer != NULL) {

        Status = ConSimplePointer->SimplePointer->GetState(ConSimplePointer->SimplePointer, &CurrentState);

        if (!EFI_ERROR(Status)) {
        
            EfiSuccessDetected = TRUE;

            if (CurrentState.LeftButton)
                State->LeftButton = TRUE;
     
            if (CurrentState.RightButton)
                State->RightButton = TRUE;
     
            if ( CurrentState.RelativeMovementX != 0 && ConSimplePointer->SimplePointer->Mode->ResolutionX != 0 )
                State->RelativeMovementX += 
                    (CurrentState.RelativeMovementX * (INT32)gSimplePointerMode.ResolutionX) / 
                    (INT32)ConSimplePointer->SimplePointer->Mode->ResolutionX;

            if ( CurrentState.RelativeMovementY != 0 && ConSimplePointer->SimplePointer->Mode->ResolutionY != 0 )
                State->RelativeMovementY += 
                    (CurrentState.RelativeMovementY * (INT32)gSimplePointerMode.ResolutionY) / 
                    (INT32)ConSimplePointer->SimplePointer->Mode->ResolutionY;

            if ( CurrentState.RelativeMovementZ != 0 && ConSimplePointer->SimplePointer->Mode->ResolutionZ != 0 )
                State->RelativeMovementZ += 
                    (CurrentState.RelativeMovementZ * (INT32)gSimplePointerMode.ResolutionZ) / 
                    (INT32)ConSimplePointer->SimplePointer->Mode->ResolutionZ;

        } else if (Status == EFI_DEVICE_ERROR) {
            EfiDeviceErrorDetected = TRUE;
        } 

        ConSimplePointer = OUTTER( ConSimplePointer->Link.pNext, Link, CON_SPLIT_SIMPLE_POINTER );
    }

    return (EfiSuccessDetected) ? EFI_SUCCESS : (EfiDeviceErrorDetected) ? EFI_DEVICE_ERROR : EFI_NOT_READY;
}


// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: ConSplitterSimplePointerWaitForInput
//
// Description: This is callback function for WaitForInputEvent to use with 
//  WaitForEvent() to wait for input from the pointer device.
//
// Input:   
//  Event   - The Event assoicated with callback.
//  Context - Context registered when Event was created.
//
// Output: VOID
// 
//----------------------------------------------------------------------------
// <AMI_PHDR_END>         

VOID ConSplitterSimplePointerWaitForInput(
    IN  EFI_EVENT Event,
    IN  VOID *Context )
{
    EFI_STATUS TestStatus;
    CON_SPLIT_SIMPLE_POINTER *ConSimplePointer;

    if (StdInLocked) 
        return;
        
    if (ConPointerList.pHead == NULL) 
        return;

    ConSimplePointer = OUTTER( ConPointerList.pHead, Link, CON_SPLIT_SIMPLE_POINTER );

    // loop through simple pointer events and check their events 
    // if one event has been signaled, signal my event and exit.
    // we need to loop through all the registered simple pointer devices
    while (ConSimplePointer != NULL) {
        TestStatus = pBS->CheckEvent(ConSimplePointer->SimplePointer->WaitForInput);
        ConSimplePointer = OUTTER( ConSimplePointer->Link.pNext, Link, CON_SPLIT_SIMPLE_POINTER );

        if (!EFI_ERROR(TestStatus))
            pBS->SignalEvent(Event);
    }

    return;
}


// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSInReset
//
// Description:
//  This function resets the input device hardware. This routine is a part
//  of SimpleTextIn protocol implementation.
//
// Input:   
//  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This - pointer to protocol instance
//  IN BOOLEAN EV - flag if Extended verification has to be performed
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - device was reset successfully
//          EFI_ERROR - some of devices returned error
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//----------------------------------------------------------------------------
// <AMI_PHDR_END>         

EFI_STATUS CSInReset( 
	IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,	
	IN BOOLEAN                        EV 
)
{
	EFI_STATUS	Status = EFI_SUCCESS, TestStatus;
	CON_SPLIT_IN *SimpleIn;

	if (StdInLocked) return EFI_ACCESS_DENIED;

	if (ConInList.pHead == NULL)
		return EFI_SUCCESS;

    SimpleIn = OUTTER(ConInList.pHead, Link, CON_SPLIT_IN);

	// we need to loop through all the registered simple text out devices
	//	and call each of their Reset function
	while ( SimpleIn != NULL)
	{
		TestStatus = SimpleIn->SimpleIn->Reset(SimpleIn->SimpleIn, EV);
		SimpleIn = OUTTER(SimpleIn->Link.pNext, Link, CON_SPLIT_IN);

		if (EFI_ERROR(TestStatus))
			Status = TestStatus;
	}

	return Status;

}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSReadKeyStroke
//
// Description:
//  This function reads the next keystroke from the input device. This 
//  routine is a part of SimpleTextIn protocol implementation
//
// Input:   
//  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This - pointer to protocol instance
//  OUT EFI_INPUT_KEY *Key - key pressed information
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - Keystroke data successfully retrieved
//          EFI_NOT_READY - There was no keystroke data available
//          EFI_DEVICE_ERROR - The keystroke information was not returned
//                             due to hardware error
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>             

EFI_STATUS CSReadKeyStroke(
	IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
	OUT	EFI_INPUT_KEY                  *Key 
)
{
	EFI_STATUS	Status;
    AMI_EFI_KEY_DATA EfiKeyData;

    Status = CSReadEfiKey ( (AMI_EFIKEYCODE_PROTOCOL*) This, &EfiKeyData );
    if (Status == EFI_SUCCESS) {
        //
        // Check for the Partial Key. If found, SimpleTextIn ReadKeyStroke
        // Should not return that Key has bee found.
        //
        if(EfiKeyData.Key.ScanCode == 00 && EfiKeyData.Key.UnicodeChar == 0 &&
             (EfiKeyData.KeyState.KeyToggleState & KEY_STATE_EXPOSED )) {
            return EFI_NOT_READY;
        }
        *Key = EfiKeyData.Key;
        return EFI_SUCCESS;
    }

	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSWaitForKey
//
// Description:
//  This function is a callback for the EFI_SIMPLE_TEXT_INPUT_PROTOCOL.WaitForKey event
//  Checks whether the new key is available and if so - signals the event
//
// Input:   
//  IN EFI_EVENT Event - event to signal
//  IN VOID *Context - pointer to event specific context
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

VOID CSWaitForKey(
    IN EFI_EVENT Event, 
    IN VOID *Context
)
{
	EFI_STATUS	 TestStatus;
	CON_SPLIT_IN *SimpleIn;

	if (StdInLocked) return ;
	if (ConInList.pHead == NULL) return;

// loop through simple in events and check their events 
//	if one event has been signaled, signal my event and exit

    SimpleIn = OUTTER(ConInList.pHead, Link, CON_SPLIT_IN);

	// we need to loop through all the registered simple text out devices
	//	and call each of their Reset function
	while ( SimpleIn != NULL)
	{
		TestStatus = pBS->CheckEvent(SimpleIn->SimpleIn->WaitForKey);
		SimpleIn = OUTTER(SimpleIn->Link.pNext, Link, CON_SPLIT_IN);

		if (!EFI_ERROR(TestStatus))
     		pBS->SignalEvent (Event);
	}

	return;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSInResetEx
//
// Description:
//  This function resets the input device hardware. This routine is a part
//  of SimpleTextInEx protocol implementation
//
// Input:   
//  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This - pointer to protocol instance
//  IN BOOLEAN ExtendedVerification - flag if Extended verification has to be performed
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - device was reset successfully
//          EFI_ERROR - some of devices returned error
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSInResetEx( 
    IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,  
    IN BOOLEAN                           ExtendedVerification )
{
    return CSInReset(0, ExtendedVerification);
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSReadKeyStrokeEx
//
// Description:
//  This function reads the next keystroke from the input device. This 
//  routine is a part of SimpleTextInEx protocol implementation
//
// Input:   
//  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This - pointer to protocol instance
//  OUT EFI_KEY_DATA *KeyData - key pressed information
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - Keystroke data successfully retrieved
//          EFI_NOT_READY - There was no keystroke data available
//          EFI_DEVICE_ERROR - The keystroke information was not returned
//                             due to hardware error
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSReadKeyStrokeEx (
    IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
    OUT EFI_KEY_DATA                      *KeyData
)
{
	EFI_STATUS	Status;
    AMI_EFI_KEY_DATA EfiKeyData;

    if(KeyData == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    Status = CSReadEfiKey ( (AMI_EFIKEYCODE_PROTOCOL*) This, &EfiKeyData );
    if (Status == EFI_SUCCESS) {
        KeyData->Key = EfiKeyData.Key;
        KeyData->KeyState = EfiKeyData.KeyState;
        return EFI_SUCCESS;
    }

	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSReadEfiKey
//
// Description:
//  This function reads the next keystroke from the input device. This 
//  routine is a part of AmiKeyCode protocol implementation
//
// Input:   
//  IN AMI_EFIKEYCODE_PROTOCOL *This - pointer to protocol instance
//  OUT AMI_EFI_KEY_DATA *KeyData - key pressed information
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - Keystroke data successfully retrieved
//          EFI_NOT_READY - There was no keystroke data available
//          EFI_DEVICE_ERROR - The keystroke information was not returned
//                             due to hardware error
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSReadEfiKey (
    IN  AMI_EFIKEYCODE_PROTOCOL *This,
    OUT AMI_EFI_KEY_DATA        *KeyData
)
{
	AMI_EFI_KEY_DATA	TempKey;
	EFI_STATUS	Status = EFI_NOT_READY;
	CON_SPLIT_IN *SimpleIn;

	if (StdInLocked) return EFI_ACCESS_DENIED;

    ConnectInputDevices();

	if (ConInList.pHead == NULL) return EFI_NOT_READY;

    SimpleIn = OUTTER(ConInList.pHead, Link, CON_SPLIT_IN);

    pBS->SetMem(KeyData, sizeof(AMI_EFI_KEY_DATA), 0);

	// we need to loop through all the registered EfiKey, SimpleInEx and
    // SimpleIn devices and call each of their ReadKeyStroke function
	while (SimpleIn != NULL)
	{
        if (SimpleIn->KeycodeInEx) {
    		Status = SimpleIn->KeycodeInEx->ReadEfikey(SimpleIn->KeycodeInEx, &TempKey);
        } else if(SimpleIn->SimpleInEx != NULL) {
    		Status = SimpleIn->SimpleInEx->ReadKeyStrokeEx(
                        SimpleIn->SimpleInEx, (EFI_KEY_DATA*)&TempKey);
        } else if(SimpleIn->SimpleIn != NULL) {
            Status = SimpleIn->SimpleIn->ReadKeyStroke(
                        SimpleIn->SimpleIn, (EFI_INPUT_KEY*)&TempKey);
        }

        // Check for the Toggle State change
        if (!EFI_ERROR(Status) && (TempKey.KeyState.KeyToggleState & TOGGLE_STATE_VALID)) {
            if ((TempKey.KeyState.KeyToggleState & ~KEY_STATE_EXPOSED ) != mCSToggleState) {
                mCSToggleState = (TempKey.KeyState.KeyToggleState & ~KEY_STATE_EXPOSED);
                CSInSetState ( (EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL*) This,
                                &mCSToggleState );
            }
        }

		if (!EFI_ERROR(Status)) {
			*KeyData = TempKey;
            break;
        }

		SimpleIn = OUTTER(SimpleIn->Link.pNext, Link, CON_SPLIT_IN);
	}
#if PAUSEKEY_SUPPORT
    if (!EFI_ERROR(Status) && TempKey.EfiKey == EfiKeyPause) {
        while(TRUE) {
            Status = CSReadEfiKey ( This, &TempKey );
            if ((!EFI_ERROR(Status)) && (TempKey.EfiKey != EfiKeyPause) && (TempKey.EfiKey != 0x55)) {
                break;
            }
        }
        *KeyData = TempKey;
    }
#endif
	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSInSetState
//
// Description:
//  This function sets certain state for input device
//
// Input:   
//  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This - pointer to protocol instance
//  IN EFI_KEY_TOGGLE_STATE *KeyToggleState - pointer to state to set
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - Keystroke data successfully retrieved
//          EFI_UNSUPPORTED - Given state not supported
//          EFI_INVALID_PARAMETER - KeyToggleState is NULL
//          EFI_DEVICE_ERROR - input device not found
//          EFI_ACCESS_DENIED - input device is busy
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSInSetState (
    IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
    IN EFI_KEY_TOGGLE_STATE              *KeyToggleState
)
{
	EFI_STATUS	Status = EFI_SUCCESS;
	CON_SPLIT_IN *SimpleIn;

	if (StdInLocked) return EFI_ACCESS_DENIED;

	if (ConInList.pHead == NULL)
		return EFI_UNSUPPORTED;

    if(KeyToggleState == NULL ) {
        return EFI_INVALID_PARAMETER;
    }

    if (!(*KeyToggleState & TOGGLE_STATE_VALID) || 
        ((*KeyToggleState & (~(TOGGLE_STATE_VALID | KEY_STATE_EXPOSED | 
                            SCROLL_LOCK_ACTIVE | NUM_LOCK_ACTIVE | CAPS_LOCK_ACTIVE)))) ) {
        return EFI_UNSUPPORTED;
    }

    mCSToggleState = *KeyToggleState;  // Update global toggle state

    SimpleIn = OUTTER(ConInList.pHead, Link, CON_SPLIT_IN);

	// we need to loop through all the registered KeycodeInEx devices
	//	and call each of their SetState function
	while ( SimpleIn != NULL )
	{
        if (SimpleIn->SimpleInEx) {
    		SimpleIn->SimpleInEx->SetState(SimpleIn->SimpleInEx, KeyToggleState);
        }
		SimpleIn = OUTTER(SimpleIn->Link.pNext, Link, CON_SPLIT_IN);
	}
	
	return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSInRegisterKeyNotify
//
// Description:
//  This function registers a notification function for a particular
//  keystroke of the input device
//
// Input:   
//  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This - pointer to protocol instance
//  IN EFI_KEY_DATA *KeyData - key value
//  IN EFI_KEY_NOTIFY_FUNCTION KeyNotificationFunction - notification function
//  OUT EFI_HANDLE *NotifyHandle - returned registered handle
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - notification function registered successfully
//          EFI_INVALID_PARAMETER - KeyData/KeyNotificationFunction/NotifyHandle is NULL
//          EFI_DEVICE_ERROR - input device not found
//          EFI_ACCESS_DENIED - input device is busy
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSInRegisterKeyNotify(
    IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
    IN  EFI_KEY_DATA                      *KeyData,
    IN  EFI_KEY_NOTIFY_FUNCTION           KeyNotificationFunction,
    OUT EFI_HANDLE                        *NotifyHandle
)
{
    EFI_STATUS      Status = EFI_NOT_READY;
    CON_SPLIT_IN    *SimpleIn;
    KEY_NOTIFY_LINK *NotifyLink;
    UINT32          ChildIndex;

    if (StdInLocked) return EFI_ACCESS_DENIED;

    if (ConInList.pHead == NULL) return EFI_DEVICE_ERROR;

    if(KeyData == NULL || KeyNotificationFunction == NULL || NotifyHandle == NULL ) {
        return EFI_INVALID_PARAMETER;
    }

    Status = pBS->AllocatePool(EfiBootServicesData, sizeof(KEY_NOTIFY_LINK), &NotifyLink);
    if(EFI_ERROR(Status))
        return Status;

    NotifyLink->KeyData = *KeyData;
    NotifyLink->NotifyFunction = KeyNotificationFunction;

    SimpleIn = OUTTER(ConInList.pHead, Link, CON_SPLIT_IN);
    ChildIndex = 0;

    // we need to loop through all the registered SimpleInEx 
    // and call each of their ReadKeyStroke function
    while (SimpleIn != NULL)
    {
        NotifyLink->Children[ChildIndex].Child = SimpleIn;
        if(SimpleIn->SimpleInEx != NULL) {
            Status = SimpleIn->SimpleInEx->RegisterKeyNotify(
                        SimpleIn->SimpleInEx, KeyData, KeyNotificationFunction, NotifyHandle);
            NotifyLink->Children[ChildIndex].NotifyHandle = (EFI_ERROR(Status)) ? (EFI_HANDLE) UNUSED_NOTIFY_HANDLE : *NotifyHandle;
        } else {
            NotifyLink->Children[ChildIndex].NotifyHandle = (EFI_HANDLE) UNUSED_NOTIFY_HANDLE;
        }
        ChildIndex++;
        SimpleIn = OUTTER(SimpleIn->Link.pNext, Link, CON_SPLIT_IN);
    }

    DListAdd(&KeyNotifyList, (DLINK *)NotifyLink);
    *NotifyHandle = (EFI_HANDLE) NotifyLink;

    return Status;
}

// <AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name: CSInUnRegisterKeyNotify
//
// Description:
//  This function unregisters a notification function with given handle
//
// Input:   
//  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This - pointer to protocol instance
//  IN EFI_HANDLE NotificationHandle - handle to unregister
//
// Output:
//      EFI_STATUS
//          EFI_SUCCESS - notification function unregistered successfully
//          EFI_INVALID_PARAMETER - NotificationHandle is NULL
//          EFI_DEVICE_ERROR - input device not found
//          EFI_ACCESS_DENIED - input device is busy
// 
// Modified:
//
// Referrals:
//
// Notes:
//
//-------------------------------------------------------------------------- 
// <AMI_PHDR_END>

EFI_STATUS CSInUnRegisterKeyNotify(
    IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
    IN EFI_HANDLE                        NotificationHandle
)
{
    EFI_STATUS      Status = EFI_NOT_READY;
    CON_SPLIT_IN    *SimpleIn;
    KEY_NOTIFY_LINK *NotifyLink;
    UINT32          ChildIndex;
    EFI_HANDLE      Handle;

    if (StdInLocked) return EFI_ACCESS_DENIED;

    if (ConInList.pHead == NULL) return EFI_DEVICE_ERROR;

    if(NotificationHandle == NULL ) {
        return EFI_INVALID_PARAMETER;
    }

    NotifyLink = (KEY_NOTIFY_LINK *)KeyNotifyList.pHead;
    while(NotifyLink != NULL) {
        if((EFI_HANDLE)NotifyLink == NotificationHandle)
            break;
        NotifyLink = (KEY_NOTIFY_LINK *)NotifyLink->Link.pNext;
    }

    if(NotifyLink == NULL)
        return EFI_INVALID_PARAMETER;

    for(ChildIndex = 0; ChildIndex < ConInList.Size; ChildIndex++) {
        SimpleIn = NotifyLink->Children[ChildIndex].Child;
        Handle = NotifyLink->Children[ChildIndex].NotifyHandle;
        if(SimpleIn->SimpleInEx != NULL && Handle != (EFI_HANDLE) UNUSED_NOTIFY_HANDLE) {
            Status = SimpleIn->SimpleInEx->UnregisterKeyNotify(SimpleIn->SimpleInEx, Handle);
            if(EFI_ERROR(Status))
                return Status;
        }
    }
            
    DListDelete(&KeyNotifyList, (DLINK *)NotifyLink);
    pBS->FreePool(NotifyLink);

    return EFI_SUCCESS;
}


/****** AMI TODO ******************************************/
//This is temporary; PTM module that we are using publishes FAST_BOOT_SUPPORT SDL token but produces a different (incompatible) fast boot protocol.
//Fix this!
#if 0
//#if FAST_BOOT_SUPPORT
#include <Protocol/FastBootProtocol.h>
static AMI_FAST_BOOT_PROTOCOL *AmiFbProtocol = NULL;

VOID ConnectInputDevices(
    VOID
)
{
    EFI_STATUS Status;
    static Executed = FALSE;

    if(Executed)
        return;

    if(AmiFbProtocol == NULL) {
        Status = pBS->LocateProtocol(&AmiFastBootProtocolGuid, NULL, &AmiFbProtocol);
        if(EFI_ERROR(Status)) {
            AmiFbProtocol = NULL;
            return;
        }
    }

    if(AmiFbProtocol->IsRuntime()) {
        AmiFbProtocol->ConnectInputDevices();
        Executed = TRUE;
    }
}

#else //#if FAST_BOOT_SUPPORT
VOID ConnectInputDevices(
    VOID
){return;}
#endif

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
