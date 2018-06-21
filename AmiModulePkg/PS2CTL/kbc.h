//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/SOURCE/Core/CORE_DXE/PS2CTL/kbc.h 22    5/02/12 2:28a Deepthins $
//
// $Revision: 22 $
//
// $Date: 5/02/12 2:28a $
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name: kbc.h
//
// Description: Keyboard Controller functions header
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef __PS2_KBC_SUPPORT_H__
#define __PS2_KBC_SUPPORT_H__

#include <Protocol\Cpu.h>
#include <Protocol\Legacy8259.h>
//
// KBC command/status/data IO ports
//
#define KBC_CMDSTS_PORT 0x64
#define KBC_DATA_PORT   0x60

//
// KBC status bits definition
//
#define KBC_OBF         0x01
#define KBC_IBF         0x02 
#define KBC_SYSFLAG     0x04 
#define KBC_CMD_DATA    0x08
#define KBC_INHIBIT_SW  0x10
#define KBC_AUX_OBF     0x20
#define KBC_TIMEOUT_ERR 0x40
#define KBC_PARITY_ERR  0x80

//
//    COMMANDS from KEYBOARD to SYSTEM
//
#define KB_ACK_COM          0xFA    // ACKNOWLEDGE command
#define KB_RSND_COM         0xFE    // RESEND command
#define KB_OVRN_COM         0xFF    // OVERRUN command
#define KB_DIAG_FAIL_COM    0xFD    // DIAGNOSTIC FAILURE command

#define KBD_ENABLE_SCANNING 0xF4    
#define KBD_DISABLE_SCANNING 0xF5   
#define KBD_RESET           0xFF    

#define rKeyboardID         0xF2
#define rMouseID            0xF2

//
// Keyboard scanner states
//
#define KBST_READY      0
#define KBST_E0         1
#define KBST_E1         2

#define BUFFER_SIZE  16
#define SYSTEM_KEYBOARD_IRQ 0x01
#define SYSTEM_MOUSE_IRQ    0x0C
#define SLAVE_IRQ           0X02
// LED inter command state
#define ED_COMMAND_ISSUED       0x01
#define ED_DATA_ISSUED          0x02

typedef struct {
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL iSimpleIn;
    EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL iSimpleInEx;
    AMI_EFIKEYCODE_PROTOCOL iKeycodeInEx;
    AMI_EFI_KEY_DATA KeyData;
    BOOLEAN KeyIsReady;
    UINT8  ScannerState;
    AMI_EFI_KEY_DATA* pBufHead;
    AMI_EFI_KEY_DATA* pBufTail;
    AMI_EFI_KEY_DATA* pBufStart;
    AMI_EFI_KEY_DATA* pBufEnd;
    UINT8  Count;
//    UINT16 Modifiers;
    UINT8  Indicators;
    UINT8  LEDCommandState;
    UINT8  CommandResponded;           
} KEYBOARD;

typedef struct {
    UINT8   KbdBuffer[BUFFER_SIZE];
    UINT8   KbdIndex;
} KEYBOARD_IRQ_STORE;

EFI_STATUS KBCBatTest();
UINT8 Read8042(UINT8 bCmd);
UINT8 ObFullRead();
void WriteKeyboardCommand(UINT8 bCmd);
void WriteKeyboardData(UINT8 bCmd);
void Write8042CommandByte(UINT8 bCCB);
void IbFree();
EFI_STATUS IbFreeTimeout(UINT32 TimeoutValue);
void IoDelay();
void AutodetectKbdMousePorts();
EFI_STATUS ReadDevice(UINT8 bCmd, UINT8 *Data, UINT8 Response);
BOOLEAN ObFullReadTimeout(UINT8* data,  UINT32 msec, BOOLEAN ONLYOBF);
UINT8 IssueCommand(UINT8 bCmd);
UINT8 AuxDeviceCommand(UINT8 bCmd);
EFI_STATUS OutToKb(KEYBOARD* kbd, UINT8 bCmd);
EFI_STATUS InsertKeyToBuffer (KEYBOARD* kbd, AMI_EFI_KEY_DATA *key);
EFI_STATUS GetKeyFromBuffer (KEYBOARD* kbd, VOID* key, UINT8 size);
BOOLEAN CheckKeyinBuffer (KEYBOARD* kbd);
void ReadAndProcessKey(void *Context);
void DisableKeyboard();
EFI_STATUS EnableKeyboard();
void DisableAuxDevice();
void EnableAuxDevice();
void LEDsOnOff(KEYBOARD* kbd);
void CheckIssueLEDCmd (KEYBOARD *kbd);
void ProcessKBDResponse (KEYBOARD *kbd, UINT8 bData); 
EFI_STATUS DetectPS2Keyboard();
EFI_STATUS DetectPS2KeyboardAndMouse();
void HandleKBDData(void *Context, UINT8 data);
void ProcessKBDData (KEYBOARD *kbd, UINT8 data);
BOOLEAN ObFullReadMTimeout(UINT8* data, UINT32 msec);
EFI_STATUS ReadDeviceM(UINT8 bCmd, UINT8 *Data, UINT8 Response);
EFI_STATUS InitHotKeys(EFI_HANDLE Controller);
EFI_STATUS ProcessHotKey(UINT8 code, UINT16 modifiers);
EFI_STATUS DetectPs2Mouse();
EFI_STATUS GetMouseData();
VOID UpdateSioVariableForKeyboardMouse(
    EFI_EVENT   Event,
    VOID        *Context
);
EFI_STATUS
CheckPartialKey (
    KEYBOARD    *Kbd,
    EFI_KEY_DATA        *Key
);
VOID InitKeyboardIrq(VOID);     
void ProcessLEDCommandData(KEYBOARD* kbd);
EFI_STATUS ProcessMultiLanguage(
    IN OUT  AMI_EFI_KEY_DATA                *KeyData
	);
#endif  // __PS2_KBC_SUPPORT_H__

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
