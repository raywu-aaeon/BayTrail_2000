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
// $Header: /Alaska/SOURCE/Core/CORE_DXE/PS2CTL/efismplpp.c 7     11/03/11 5:56a Rajeshms $
//
// $Revision: 7 $
//
// $Date: 11/03/11 5:56a $
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name: efismplpp.c
//
// Description: PS/2 mouse implmentation of simple pointer protocol
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>

//----------------------------------------------------------------------

#include "ps2ctl.h"
#include <AmiLib.h>
#include "ps2ctl.h"
#include "kbc.h"
#include "ps2mouse.h"

//----------------------------------------------------------------------


extern MOUSE gMouse;
extern EFI_GUID gDevicePathProtocolGuid;
EFI_GUID gSimplePointerGuid = EFI_SIMPLE_POINTER_PROTOCOL_GUID;

//----------------------------------------------------------------------


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       StartMouse
//
// Description:     This routine is called from Driver Binding Start function.
//                  It starts the mouse support
//
// Parameters:      
//    EFI_DRIVER_BINDING_PROTOCOL *This -  Pointer to this instance of driver
//                                         binding protocol structure
//    EFI_HANDLE Controller - Handle for this driver
//
// Output:          EFI_STATUS - Status of the operation
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS StartMouse (
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE                  Controller )
{
    EFI_STATUS Status;
    EFI_DEVICE_PATH_PROTOCOL *pDummyDevPath;
    MOUSE* pmouse = 0;

    if (EFI_ERROR(gSysTable->BootServices->OpenProtocol(
            Controller,
            &gDevicePathProtocolGuid,
            &pDummyDevPath,
            This->DriverBindingHandle,
            Controller,
            EFI_OPEN_PROTOCOL_BY_DRIVER))) {
        return EFI_INVALID_PARAMETER;
    }

    DetectPS2KeyboardAndMouse();

    if ( EFI_ERROR(InitMOUSE( &pmouse ))) {
        gSysTable->BootServices->CloseProtocol(
            Controller,
            &gDevicePathProtocolGuid,
            This->DriverBindingHandle,
            Controller);

        return EFI_DEVICE_ERROR;
    }

    gSysTable->BootServices->CreateEvent(
        EVT_NOTIFY_WAIT,
        TPL_NOTIFY, 
        OnWaitingOnMouse,
        pmouse,
        &pmouse->iSmplPtr.WaitForInput);
    //
    // Install protocol interfaces for the pointer device.
    //
    Status = gSysTable->BootServices->InstallProtocolInterface (
        &Controller, &gSimplePointerGuid, EFI_NATIVE_INTERFACE,
        &pmouse->iSmplPtr);

    if (EFI_ERROR(Status)) {
        gSysTable->BootServices->CloseProtocol(
            Controller,
            &gDevicePathProtocolGuid,
            This->DriverBindingHandle,
            Controller);

        gSysTable->BootServices->CloseEvent(pmouse->iSmplPtr.WaitForInput);
    }

    return Status;  
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       StopMouse
//
// Description:     This routine is called from Driver Binding Start function.
//                  It stops the mouse support
//
// Parameters:      
//    EFI_DRIVER_BINDING_PROTOCOL *This -  Pointer to this instance of driver
//                                         binding protocol structure
//    EFI_HANDLE Controller - Handle for this driver
//
// Output:           EFI_STATUS - Status of the operation
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS StopMouse (
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE                  Controller )
{
    EFI_STATUS Status;

    //
    // Kill wait event
    //
    Status = gSysTable->BootServices->CloseEvent(gMouse.iSmplPtr.WaitForInput);
    if (EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Uninstall protocol interfaces from the Mouse device.
    //
    Status = gSysTable->BootServices->UninstallMultipleProtocolInterfaces (
        Controller,
        &gSimplePointerGuid, &gMouse.iSmplPtr,   
        NULL
    );

    if (EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Close protocols that is open during Start
    //
    Status = gSysTable->BootServices->CloseProtocol(
        Controller,
        &gEfiDevicePathProtocolGuid,
        This->DriverBindingHandle,
        Controller);

    if (EFI_ERROR(Status)) {
        return Status;
    }

    pBS->FreePool(gMouse.iSmplPtr.Mode);

    return Status;
}

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
