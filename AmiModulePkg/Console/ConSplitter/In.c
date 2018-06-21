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

//**********************************************************************
// $Header: /Alaska/SOURCE/Core/CORE_DXE/ConSplitter/In.c 28    9/22/11 6:29a Rameshr $
//
// $Revision: 28 $
//
// $Date: 9/22/11 6:29a $
//**********************************************************************
/** @file In.c
    File contains the Simple Text Output functionality for the
    Console Splitter Driver

**/
//**********************************************************************
#include "ConSplit.h"
#include "Token.h"
#include <Protocol/SimplePointer.h>

extern EFI_HII_KEYBOARD_LAYOUT *gKeyDescriptorList;
extern EFI_KEY_TOGGLE_STATE mCSToggleState;

EFI_SIMPLE_POINTER_MODE gSimplePointerMode = {
    0x10000,
    0x10000,
    0x10000,
    FALSE,
    FALSE
};

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


/**
    Function goes through all the devices containing the Simple Pointer Protocol
    and calls their reset functions. If the console control's LockStdIn has been
    called, this function will be blocked from executing

    @param  This Pointer to the ConsoleSplitter's Simple Pointer Protocol
    @param  ExtendedVerification Value to pass to the reset functino's Extended Verification

    @retval EFI_SUCCESS Devices were reset sucessfully
    @retval EFI_ERROR Some of the devices returned an error
**/
EFI_STATUS ConSplitterSimplePointerReset (
    IN  EFI_SIMPLE_POINTER_PROTOCOL *This,
    IN  BOOLEAN ExtendedVerification )
{
    EFI_STATUS Status = EFI_SUCCESS;
    EFI_STATUS TestStatus;
    CON_SPLIT_SIMPLE_POINTER *ConSimplePointer;

    if (CurrentStdInStatus)
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

/**
    Retrieves the current state of a pointer device.

    @param This Pointer to the ConsoleSplitter's Simple Pointer Protcool
    @param State Pointer to the buffer to return the collective state

    @retval EFI_SUCCESS The state information was returned
    @retval EFI_NOT_READY There were no devices being managed by the Console Splitter
    @retval EFI_DEVICE_ERROR A hardware error was encountered
**/
EFI_STATUS ConSplitterSimplePointerGetState(
    IN  EFI_SIMPLE_POINTER_PROTOCOL *This,
    IN OUT EFI_SIMPLE_POINTER_STATE *State )
{
    EFI_STATUS Status;
    EFI_SIMPLE_POINTER_STATE  CurrentState;
    CON_SPLIT_SIMPLE_POINTER *ConSimplePointer;
    BOOLEAN EfiSuccessDetected = FALSE;
    BOOLEAN EfiDeviceErrorDetected = FALSE;

    if (CurrentStdInStatus)
        return EFI_ACCESS_DENIED;

    State->RelativeMovementX  = 0;
    State->RelativeMovementY  = 0;
    State->RelativeMovementZ  = 0;
    State->LeftButton         = FALSE;
    State->RightButton        = FALSE;

    //if no device attached return success with no movement
    if (ConPointerList.pHead == NULL)
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


/**
    Callback function executed on the WaitForEvent timer experiation. If LockStdIn is not set,
    function will notify registered callbacks waiting for input that the event has expired.

    @param Event The Event assoicated with callback.
    @param Context Context registered when Event was created.
**/
VOID ConSplitterSimplePointerWaitForInput(
    IN  EFI_EVENT Event,
    IN  VOID *Context )
{
    EFI_STATUS TestStatus;
    CON_SPLIT_SIMPLE_POINTER *ConSimplePointer;

    if (CurrentStdInStatus)
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


/**
    Reset function for the Simple Text In virtual Device. This function goes
    through the list of Simple Text In devices being managed and calls the
    Reset function for each device.

    @param This Pointer to the Console Splitter's SimpleTextIn protocol
    @param EV Flag to determine if Extended Verification should be performed when resetting each device.

    @retval EFI_SUCCESS Managed devices were reset
    @retval EFI_ERROR One or more of the managed devices returned an error
    @retval EFI_ACCESS_DENIED The Console is locked via the console control protocol
**/
EFI_STATUS CSInReset(
	IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
	IN BOOLEAN                        EV
)
{
	EFI_STATUS	Status = EFI_SUCCESS;
	EFI_STATUS ManagedDeviceStatus;
	CON_SPLIT_IN *SimpleIn;

	if(CurrentStdInStatus)
	    return EFI_ACCESS_DENIED;

    SimpleIn = OUTTER(ConInList.pHead, Link, CON_SPLIT_IN);

	// we need to loop through all the registered simple text out devices
	//	and call each of their Reset function
	while( SimpleIn != NULL)
	{
		ManagedDeviceStatus = SimpleIn->SimpleIn->Reset(SimpleIn->SimpleIn, EV);

		if(EFI_ERROR(ManagedDeviceStatus))
			Status = ManagedDeviceStatus;

        SimpleIn = OUTTER(SimpleIn->Link.pNext, Link, CON_SPLIT_IN);
	}

	return Status;
}

/**
    Function to read the next keystroke from all the Simple Text Input devices
    that are managed by the Console Splitter.

    @param This Pointer to the Console Splitter's Simple Text In protocol
    @param Key Pointer to the buffer to return keypress information

    @retval EFI_SUCCESS Keystroke data successfully retrieved
    @retval EFI_NOT_READY There was no keystroke data available, or the console is locked
    @retval EFI_DEVICE_ERROR The keystroke information was not returned due to hardware error
**/
EFI_STATUS CSReadKeyStroke(
	IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
	OUT	EFI_INPUT_KEY                  *Key
)
{
	EFI_STATUS	Status = EFI_NOT_READY;
    AMI_EFI_KEY_DATA EfiKeyData;

    if(CurrentStdInStatus)
        return Status;

    Status = CSReadEfiKey( (AMI_EFIKEYCODE_PROTOCOL*) This, &EfiKeyData );
    if(!EFI_ERROR(Status)) {
        //
        // Check for the Partial Key. If found, SimpleTextIn ReadKeyStroke
        // Should not return that Key has bee found.
        //
        if(EfiKeyData.Key.ScanCode == 00 && EfiKeyData.Key.UnicodeChar == 0 &&
             (EfiKeyData.KeyState.KeyToggleState & KEY_STATE_EXPOSED )) {
            return EFI_NOT_READY;
        }
        *Key = EfiKeyData.Key;
    }

	return Status;
}

/**
    Timer callback functin called perioditically to check if a new keystroke has occured. When
    it determines that a keystroke has occured and that there is key data available, the
    function signals the passed event.  If the LockStdIn is set, new keystroke information will
    not be checked.

    @param Event Event to signal on new keystroke data being available
    @param Context Pointer to specific context
**/
VOID CSWaitForKey(IN EFI_EVENT Event, IN VOID *Context)
{
	EFI_STATUS	 TestStatus;
	CON_SPLIT_IN *SimpleIn;

	if(CurrentStdInStatus)
	    return;

	if(ConInList.pHead == NULL)
	    return;

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

/**
    Function to reset the input device hardware.

    This function resets the input device hardware. This routine is a part
    of SimpleTextInEx protocol implementation

    @param This pointer to protocol instance
    @param ExtendedVerification flag if Extended verification has to be performed

    @retval EFI_SUCCESS device was reset successfully
    @retval EFI_ERROR some of devices returned error

**/
EFI_STATUS CSInResetEx(
    IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
    IN BOOLEAN                           ExtendedVerification )
{
    if(CurrentStdInStatus)
        return EFI_SUCCESS;

    return CSInReset(0, ExtendedVerification);
}

/**
    This function reads the next keystroke from the input device. This
    routine is a part of SimpleTextInEx protocol implementation

    @param This Pointer to the simple text in ex protocol
    @param KeyData Buffer to return the keypress data in.

    @retval EFI_SUCCESS Keystroke data successfully retrieved
    @retval EFI_NOT_READY There was no keystroke data available
    @retval EFI_DEVICE_ERROR The keystroke information was not returned due to hardware error
**/
EFI_STATUS CSReadKeyStrokeEx (
    IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
    OUT EFI_KEY_DATA                      *KeyData
)
{
	EFI_STATUS	Status = EFI_INVALID_PARAMETER;
    AMI_EFI_KEY_DATA EfiKeyData;

    if(CurrentStdInStatus)
        return EFI_NOT_READY;

    if(KeyData != NULL)
    {
        Status = CSReadEfiKey( (AMI_EFIKEYCODE_PROTOCOL*) This, &EfiKeyData );
        if (Status == EFI_SUCCESS) {
            KeyData->Key = EfiKeyData.Key;
            KeyData->KeyState = EfiKeyData.KeyState;
            return EFI_SUCCESS;
        }
    }
	return Status;
}

/**
    This function reads the next keystroke from the input devices. This
    routine is a part of AmiKeyCode protocol implementation

    @param This pointer to protocol instance
    @param KeyData key pressed information

    @retval EFI_SUCCESS Keystroke data successfully retrieved
    @retval EFI_NOT_READY There was no keystroke data available
    @retval EFI_DEVICE_ERROR The keystroke information was not returned
        due to hardware error

**/
EFI_STATUS CSReadEfiKey (
    IN  AMI_EFIKEYCODE_PROTOCOL *This,
    OUT AMI_EFI_KEY_DATA        *KeyData
)
{
	AMI_EFI_KEY_DATA	TempKey;
	EFI_STATUS	Status = EFI_NOT_READY;
	CON_SPLIT_IN *SimpleIn;

	if(CurrentStdInStatus)
	    return EFI_NOT_READY;

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

/**
    Function to set keyboard states for all the keyboard devices.

    @param This Pointer to the ConsoleSplitter's Simple Text In Ex protocol
    @param KeyToggleState Pointer Key states to set into the keyboard devices

    @retval EFI_SUCCESS Keystroke data successfully retrieved
    @retval EFI_UNSUPPORTED Given state not supported
    @retval EFI_INVALID_PARAMETER KeyToggleState is NULL
    @retval EFI_DEVICE_ERROR input device not found
    @retval EFI_ACCESS_DENIED input device is busy
**/
EFI_STATUS CSInSetState (
    IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
    IN EFI_KEY_TOGGLE_STATE              *KeyToggleState
)
{
	EFI_STATUS	Status = EFI_SUCCESS;
	CON_SPLIT_IN *SimpleIn;

	if(CurrentStdInStatus)
	    return EFI_ACCESS_DENIED;

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

/**
    Function to register notificaiton functions into each of the Simple Text In Ex
    devices being managed by the ConsoleSplitter.

    @param This Pointer to the Console Splitter's Simple Text In Ex protocol
    @param KeyData The key data which should trigger the notification function
    @param KeyNotificationFunction The function to call when the keydata is input
    @param NotifyHandle A handle unique to the registered event

    @retval EFI_SUCCESS Notification function registered successfully
    @retval EFI_INVALID_PARAMETER KeyData/KeyNotificationFunction/NotifyHandle is NULL
    @retval EFI_DEVICE_ERROR input device not found
    @retval EFI_ACCESS_DENIED input device is busy
**/
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

    if (CurrentStdInStatus)
        return EFI_ACCESS_DENIED;

    if (ConInList.pHead == NULL)
        return EFI_DEVICE_ERROR;

    if(KeyData == NULL || KeyNotificationFunction == NULL || NotifyHandle == NULL )
        return EFI_INVALID_PARAMETER;

    Status = pBS->AllocatePool(EfiBootServicesData, sizeof(KEY_NOTIFY_LINK), (VOID**)&NotifyLink);
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

/**
    Function to unregister a previously registered Notification function.

    @param This Pointer to the Console Splitter's Simple Text In Ex protocol
    @param NotificationHandle handle to unregister

    @retval EFI_SUCCESS notification function unregistered successfully
    @retval EFI_INVALID_PARAMETER NotificationHandle is NULL
    @retval EFI_DEVICE_ERROR input device not found
    @retval EFI_ACCESS_DENIED input device is busy
**/
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

    if (CurrentStdInStatus)
        return EFI_ACCESS_DENIED;

    if (ConInList.pHead == NULL)
        return EFI_DEVICE_ERROR;

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
