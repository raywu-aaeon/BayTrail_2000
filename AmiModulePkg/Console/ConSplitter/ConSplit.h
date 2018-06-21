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
// $Header: /Alaska/SOURCE/Core/CORE_DXE/ConSplitter/ConSplit.h 14    8/12/11 12:17p Artems $
//
// $Revision: 14 $
//
// $Date: 8/12/11 12:17p $
//*****************************************************************************
/** @file ConSplit.h
    This file contains the structure and function prototypes needed
    for the Console Splitter driver

**/
//*****************************************************************************

#ifndef __CONSOLE_SPLITTER_H__
#define __CONSOLE_SPLITTER_H__

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------------------

#include <Efi.h>
#include <Library/UefiLib.h>
#include <Protocol/SimpleTextOut.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/AmiKeycode.h>
#include <Protocol/SimplePointer.h>
#include <AmiDxeLib.h>

//--------------------------------------------------------------------------------

#pragma pack (1)

/**
    This structure represents Splitter Simple Pointer devices linked list.
**/
typedef struct _CON_SPLIT_SIMPLE_POINTER {
    DLINK Link; ///< Linked List Pointer MUST BE THE FIRST FIELD
    EFI_SIMPLE_POINTER_PROTOCOL *SimplePointer; ///< Protocol pointer
    EFI_HANDLE Handle; ///< Device handle
} CON_SPLIT_SIMPLE_POINTER;

/**
    This structure represents Splitter ConOut devices linked list
**/
typedef struct _CON_SPLIT_OUT
{
    DLINK                           Link; ///< Linked list pointer MUST BE THE FIRST FIELD
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*SimpleOut; ///< Protocol pointer
    EFI_HANDLE	                    Handle; ///< Device handle
} CON_SPLIT_OUT;

/**
    This structure represents Splitter ConIn devices linked list
**/
typedef struct _CON_SPLIT_IN
{
    DLINK                             Link;         ///< Linked list pointer
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL	  *SimpleIn;    ///< Protocol pointer
    EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *SimpleInEx;  ///< Protocol pointer
    AMI_EFIKEYCODE_PROTOCOL	          *KeycodeInEx; ///< Protocol pointer
    EFI_HANDLE	                      Handle;       ///< Device handle
} CON_SPLIT_IN;

/**
    This structure represents Splitter devices structure
**/
typedef struct _CON_SPLIT
{
    CON_SPLIT_IN	Input;  ///< Input devices linked list
    CON_SPLIT_OUT	Output; ///< Output devices linked list
    CON_SPLIT_OUT	StdErr; ///< Error output devices linked list
} CON_SPLIT;

/**
    This structure represents Splitter supported resolutions database
**/
typedef struct __SUPPORT_RES
{
    INT32	Columns;    ///< Max number of text columns
    INT32	Rows;       ///< Max number of text rows
    BOOLEAN AllDevices; ///< Flag if all devices support given resolution
} SUPPORT_RES;

#define MAX_CON_IN_CHILDREN 10
#define UNUSED_NOTIFY_HANDLE 0xBADBAD

/**
    This structure represents information about individual registered keystroke callbacks
**/
typedef struct {
    CON_SPLIT_IN *Child;        ///< Child Device
    EFI_HANDLE NotifyHandle;    ///< Nofity handle
} KEY_NOTIFY_CHILD;

/**
    This structure represents linked list of registered keystroke callbacks
**/
typedef struct
{
    DLINK Link;                                     ///< Linked list pointer
    EFI_KEY_DATA KeyData;                           ///< Keystroke of interest
    EFI_KEY_NOTIFY_FUNCTION NotifyFunction;         ///< Callback to call
    KEY_NOTIFY_CHILD Children[MAX_CON_IN_CHILDREN]; ///< Array of child notify handles
} KEY_NOTIFY_LINK;

#pragma pack()

//-----------------------------------------------------------------------------------

extern SUPPORT_RES *SupportedModes;

//virtual splitter output buffer
extern CHAR16 *ScreenBuffer;
extern CHAR16 *EndOfTheScreen;
extern INT32  *AttributeBuffer;
extern INT32  Columns;

extern SIMPLE_TEXT_OUTPUT_MODE		     MasterMode;
extern EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	 mCSOutProtocol;
extern EFI_SIMPLE_TEXT_INPUT_PROTOCOL 	 mCSSimpleInProtocol;
extern EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL mCSSimpleInExProtocol;
extern AMI_EFIKEYCODE_PROTOCOL 	         mCSKeycodeInProtocol;
extern DLIST 		                     ConInList;
extern DLIST		                     ConOutList;
extern DLIST		                     ConPointerList;
extern DLIST		                     KeyNotifyList;
extern BOOLEAN                           CurrentStdInStatus;
extern EFI_SIMPLE_POINTER_PROTOCOL       mCSSimplePointerProtocol;

EFI_STATUS ConSplitterSimplePointerReset(
    IN  EFI_SIMPLE_POINTER_PROTOCOL *This,
    IN  BOOLEAN ExtendedVerification );

EFI_STATUS ConSplitterSimplePointerGetState(
    IN  EFI_SIMPLE_POINTER_PROTOCOL *This,
    IN OUT EFI_SIMPLE_POINTER_STATE *State );

VOID ConSplitterSimplePointerWaitForInput(
    IN  EFI_EVENT Event,
    IN  VOID *Context );

EFI_STATUS	CSSupported (
	IN EFI_DRIVER_BINDING_PROTOCOL    *This,
	IN EFI_HANDLE                     ControllerHandle,
	IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath);

EFI_STATUS	CSStart (
	IN EFI_DRIVER_BINDING_PROTOCOL    *This,
	IN EFI_HANDLE                     ControllerHandle,
	IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
	);

EFI_STATUS	CSStop (
	IN	EFI_DRIVER_BINDING_PROTOCOL	  *This,
	IN	EFI_HANDLE                    ControllerHandle,
	IN  UINTN						  NumberOfChildren,
	IN  EFI_HANDLE					  *ChildHandleBuffer
	);

EFI_STATUS CSReset(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN BOOLEAN                         ExtendedVerification
	);

EFI_STATUS CSOutputString(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN CHAR16                          *String
    );

EFI_STATUS CSTestString(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN CHAR16                          *String
    );

EFI_STATUS CSQueryMode(
    IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN  UINTN                           ModeNum,
    OUT UINTN                           *Col,
    OUT UINTN                           *Row
    );

EFI_STATUS CSSetMode(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN UINTN                           ModeNum
    );

EFI_STATUS CSSetAttribute(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN UINTN                           Attribute
    );

EFI_STATUS CSClearScreen(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
    );

EFI_STATUS CSSetCursorPosition(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
    IN UINTN                            Column,
    IN UINTN                            Row
    );

EFI_STATUS CSEnableCursor(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL	*This,
    IN BOOLEAN                          Visible
    );

EFI_STATUS CSInReset(
    IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
    IN BOOLEAN                        EV
    );

EFI_STATUS CSReadKeyStroke (
    IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
    OUT EFI_INPUT_KEY                  *Key
    );

VOID CSWaitForKey(
    IN EFI_EVENT Event,
    IN VOID      *Context
    );

EFI_STATUS CSInResetEx(
    IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
    IN BOOLEAN                           EV
    );

EFI_STATUS CSReadKeyStrokeEx (
    IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
    OUT EFI_KEY_DATA                      *KeyData
    );

EFI_STATUS
CSReadEfiKey (
    IN  AMI_EFIKEYCODE_PROTOCOL *This,
    OUT AMI_EFI_KEY_DATA        *KeyData
    );

EFI_STATUS CSInSetState (
    IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
    IN EFI_KEY_TOGGLE_STATE              *KeyToggleState
);

EFI_STATUS CSInRegisterKeyNotify(
    IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
    IN  EFI_KEY_DATA                      *KeyData,
    IN  EFI_KEY_NOTIFY_FUNCTION           KeyNotificationFunction,
    OUT EFI_HANDLE                        *NotifyHandle
);

EFI_STATUS CSInUnRegisterKeyNotify(
    IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
    IN EFI_HANDLE                        NotificationHandle
);

VOID RestoreTheScreen(
    IN EFI_HANDLE                       ControllerHandle,
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *SimpleOut
    );

VOID SaveTheScreen(
    IN EFI_HANDLE                       ControllerHandle,
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *SimpleOut
    );

EFI_STATUS MemClearScreen(
    VOID
    );

VOID SaveUgaMode(
    IN EFI_HANDLE ControllerHandle
    );

VOID RestoreUgaMode(
    IN EFI_HANDLE ControllerHandle
    );

EFI_STATUS InitModesTable(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN EFI_HANDLE                      Handle
    );

VOID UpdateModesTable(
    IN EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN EFI_HANDLE                      Handle
    );

EFI_STATUS IsModeSupported(
    IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    IN  UINTN                           ModeNum,
	OUT INT32                           *DeviceModeNumber
    );

EFI_STATUS ResizeSplitterBuffer(
    IN INT32 ModeNum
    );

VOID UpdateMasterMode(
    IN SIMPLE_TEXT_OUTPUT_MODE *Mode
    );

EFI_STATUS CSInKeyboardLayoutMap(
    IN OUT AMI_EFI_KEY_DATA *KeyData
    );

VOID AdjustSupportedModes(
    VOID
    );

EFI_STATUS KeyboardLayoutMap(
    IN      AMI_MULTI_LANG_SUPPORT_PROTOCOL *This,
    IN OUT  AMI_EFI_KEY_DATA                *KeyData
	);

EFI_STATUS KeyNotifyAddChild(
    IN CON_SPLIT_IN *Child
    );

EFI_STATUS KeyNotifyRemoveChild(
    IN CON_SPLIT_IN *Child
    );
#ifdef __cplusplus
}
#endif
#endif

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
