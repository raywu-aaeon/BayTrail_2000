//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
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
// $Header: /Alaska/BIN/Core/Library/Ps2Tokens.c 22    5/24/12 3:22p Artems $
//
// $Revision: 22 $
//
// $Date: 5/24/12 3:22p $
//**********************************************************************
#include <Efi.h>
#include <token.h>
#include "PS2Ctl.h"
//Ps2Ctl driver global variables

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    KbdIrqSupport
//
// Description:	Variable to replace KB_IRQ_SUPPORT token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN KbdIrqSupport = 
#ifdef KB_IRQ_SUPPORT
    KB_IRQ_SUPPORT
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    MsIrqSupport
//
// Description:	Variable to replace MS_IRQ_SUPPORT token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN MsIrqSupport = 
#ifdef MS_IRQ_SUPPORT
    MS_IRQ_SUPPORT
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    Ps2MouseSupport
//
// Description:	Variable to replace PS2MOUSE_SUPPORT token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN Ps2MouseSupport = 
#ifdef PS2MOUSE_SUPPORT
    PS2MOUSE_SUPPORT
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    Ps2KbdSupport
//
// Description:	Variable to replace PS2KBD_SUPPORT token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN Ps2KbdSupport = 
#ifdef PS2KBD_SUPPORT
PS2KBD_SUPPORT
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    KbRdBeforeInstall
//
// Description:	Variable to replace KBD_READ_BEFORE_INSTALL token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN KbRdBeforeInstall = 
#ifdef KBD_READ_BEFORE_INSTALL
    KBD_READ_BEFORE_INSTALL
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    KbcAutoDetectPorts
//
// Description:	Variable to replace KBC_AUTODETECT_PORTS token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN KbcAutoDetectPorts = 
#ifdef KBC_AUTODETECT_PORTS
KBC_AUTODETECT_PORTS
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    BlockKbcPin2223Bit
//
// Description:	Variable to replace BLOCK_KBC_PIN_22_23_BIT token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN BlockKbcPin2223Bit = 
#ifdef BLOCK_KBC_PIN_22_23_BIT
BLOCK_KBC_PIN_22_23_BIT
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    LedsAtStartup
//
// Description:	Variable to replace LEDS_AT_STARTUP token.
//
// Notes: UINT8
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
UINT8 LedsAtStartup = 
#ifdef LEDS_AT_STARTUP
LEDS_AT_STARTUP
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    MaxHotKeys
//
// Description:	Variable to replace MAX_HOTKEYS token.
//
// Notes: UINT8
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
UINT8 MaxHotKeys = 
#ifdef MAX_HOTKEYS
MAX_HOTKEYS
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    InitDefaultHotKeys
//
// Description:	Variable to replace INIT_DEFAULT_HOTKEYS token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN InitDefaultHotKeys = 
#ifdef INIT_DEFAULT_HOTKEYS
INIT_DEFAULT_HOTKEYS
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    KbcBasicAssuranceTest
//
// Description:	Variable to replace KBC_BASIC_ASSURANCE_TEST token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN KbcBasicAssuranceTest = 
#ifdef KBC_BASIC_ASSURANCE_TEST
KBC_BASIC_ASSURANCE_TEST
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    DetectPs2KeyboardValue
//
// Description:	Variable to replace DETECT_PS2_KEYBOARD token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN DetectPs2KeyboardValue = 
#ifdef DETECT_PS2_KEYBOARD
DETECT_PS2_KEYBOARD
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    DetectPs2MouseValue
//
// Description:	Variable to replace DETECT_PS2_MOUSE token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN DetectPs2MouseValue = 
#ifdef DETECT_PS2_MOUSE
DETECT_PS2_MOUSE
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    InstallKeyboardMouseAlways
//
// Description:	Variable to replace INSTALL_KEYBOARD_MOUSE_ALWAYS token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN InstallKeyboardMouseAlways = 
#ifdef INSTALL_KEYBOARD_MOUSE_ALWAYS
INSTALL_KEYBOARD_MOUSE_ALWAYS
#else
    0
#endif
    ;
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    ReturnCurrentKeyState
//
// Description:	Variable to replace RETURN_CURRENT_KEY_STATE token.
//
// Notes: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
BOOLEAN ReturnCurrentKeyState = 
#ifdef RETURN_CURRENT_KEY_STATE
RETURN_CURRENT_KEY_STATE
#else
    0
#endif
    ;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    IbFreeTimeoutValue
//
// Description:	Variable to replace IBFREE_TIMEOUT token.
//
// Notes: UINT32
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
UINT32 IbFreeTimeoutValue = 
#ifdef IBFREE_TIMEOUT
IBFREE_TIMEOUT
#else
    0
#endif
    ;

AUTODETECT_KBD_MOUSE_PORTS AutodetectKbdMousePortsPtr =
#if KBC_AUTODETECT_PORTS_FUNCTION
    KBC_AUTODETECT_PORTS_FUNCTION
#else
    NULL
#endif
    ;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    IbMaxFreeTimeoutValue
//
// Description: Variable to replace MAXIMUM_TIMEOUT_FOR_IBFREE token.
//
// Notes: UINT32
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
UINT32 IbFreeMaxTimeoutValue = 
#ifdef MAXIMUM_TIMEOUT_FOR_IBFREE
MAXIMUM_TIMEOUT_FOR_IBFREE
#else
    0
#endif
    ;



//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
