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
// $Header: /Alaska/SOURCE/Core/CORE_DXE/PS2CTL/ps2ctl.h 15    4/27/11 4:35a Lavanyap $
//
// $Revision: 15 $
//
// $Date: 4/27/11 4:35a $
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name: ps2ctl.h
//
// Description: PS/2 Controller DXE driver header
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>

//----------------------------------------------------------------------

#ifndef __PS2_MAIN_H__
#define __PS2_MAIN_H__

#include <Efi.h>
#include <AmiDxeLib.h>
#include <Protocol\DevicePath.h>
#include <Protocol\DriverBinding.h>
#include <Protocol\SimpleTextIn.h>
#include <Protocol\SimpleTextInEx.h>
#include <Protocol\AmiKeycode.h>
#include <Protocol\SimplePointer.h>
#include <token.h>
#define TRACEKBD 


#define PS2_DRIVER_VER 0x10
#define DEVICETYPE_MOUSE     1
#define DEVICETYPE_KEYBOARD  2
extern EFI_SYSTEM_TABLE     *gSysTable;


EFI_STATUS StartMouse(EFI_DRIVER_BINDING_PROTOCOL *This,
        EFI_HANDLE Controller);
EFI_STATUS StartKeyboard(EFI_DRIVER_BINDING_PROTOCOL *This,
        EFI_HANDLE Controller);
EFI_STATUS StopMouse(EFI_DRIVER_BINDING_PROTOCOL *This,
        EFI_HANDLE Controller);
EFI_STATUS StopKeyboard(EFI_DRIVER_BINDING_PROTOCOL *This,
        EFI_HANDLE Controller);

typedef EFI_STATUS (*STARTSTOPPROC)(EFI_DRIVER_BINDING_PROTOCOL *This, 
    EFI_HANDLE Controller);

typedef struct PS2DEV_TABLE_tag {
    UINT32 hid;
    UINT32 uid;
    UINT8  DeviceType;
    STARTSTOPPROC start;
    STARTSTOPPROC stop;
    CHAR16* name;
} PS2DEV_TABLE;

typedef void (*STATEMACHINEPROC)(void*);
void PS2DataDispatcher(void*);
UINT8 KBCGetData();

BOOLEAN LookupPs2Hid(PS2DEV_TABLE*, UINT32, UINT32, PS2DEV_TABLE**);
EFI_STATUS GetPS2_DP(EFI_DRIVER_BINDING_PROTOCOL*, EFI_HANDLE, ACPI_HID_DEVICE_PATH**, UINT32);

typedef VOID (*AUTODETECT_KBD_MOUSE_PORTS)();
typedef EFI_STATUS (*DETECT_KBC_DEVICE)();
extern AUTODETECT_KBD_MOUSE_PORTS AutodetectKbdMousePortsPtr;
extern DETECT_KBC_DEVICE DetectKeyboardPtr;
extern DETECT_KBC_DEVICE DetectMousePtr;

#endif  // __PS2_MAIN_H__

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
