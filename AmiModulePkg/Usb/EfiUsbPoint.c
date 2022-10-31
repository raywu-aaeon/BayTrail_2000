//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2011, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/efiusbpoint.c 3     11/23/11 4:46a Roberthsu $
//
// $Revision: 3 $
//
// $Date: 11/23/11 4:46a $
//
//****************************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:           EfiusbAbs.C
//
//  Description:    EFI USB Absolute pointer Driver
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"
#include "UsbDef.h"
#include "Uhcd.h"
#include "ComponentName.h"
#include "UsbBus.h"

#if USB_DEV_POINT                       //(EIP66231) 

#include <Protocol/AbsPointerProtocol.h>
#define USB_ABSOLUTE_MOUSE_DRIVER_VERSION 1

#define USB_ABSOLUTE_MOUSE_DEV_SIGNATURE   EFI_SIGNATURE_32('u','a','b','s')
#define USB_ABSOLUTE_MOUSE_DEV_FROM_ABSOLUTE_PROTOCOL(a) \
    CR(a, USB_ABSOLUTE_MOUSE_DEV, AbsolutePointerProtocol, USB_ABSOLUTE_MOUSE_DEV_SIGNATURE)

EFI_GUID    gEfiAbsolutePointerProtocolGuid=EFI_ABSOLUTE_POINTER_PROTOCOL_GUID;

typedef struct
{
    UINTN                           Signature;
    EFI_ABSOLUTE_POINTER_PROTOCOL   AbsolutePointerProtocol;
    EFI_ABSOLUTE_POINTER_STATE      State;
    EFI_ABSOLUTE_POINTER_MODE       Mode;
    BOOLEAN                         StateChanged;
} USB_ABSOLUTE_MOUSE_DEV;

static VOID
UsbAbsMouseWaitForInput (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  );

static EFI_STATUS
UpdateUsbAbsMouseData (
  IN  ABS_MOUSE              *Data
  );

//
// ABS Protocol
//
static EFI_STATUS
GetAbsMouseState(
  IN   EFI_ABSOLUTE_POINTER_PROTOCOL  *This,
  OUT  EFI_ABSOLUTE_POINTER_STATE     *AbsState
);

static EFI_STATUS
UsbAbsMouseReset(
  IN EFI_ABSOLUTE_POINTER_PROTOCOL      *This,
  IN BOOLEAN                            ExtendedVerification
  );

extern USB_GLOBAL_DATA *gUsbData;

USB_ABSOLUTE_MOUSE_DEV         *UsbAbsMouseDevice=0;
static int                      gAbsMouseRefCount=0;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        InstallUSBAbsMouse
//
// Description: Installs ABSOLUTEPointerProtocol interface on a given controller.
//
// Input:       Controller - controller handle to install interface on.
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
InstallUSBAbsMouse(
    EFI_HANDLE Controller,
    DEV_INFO   *pDevInfo
)
{
    EFI_STATUS Status;
    int RefCount;

    ATOMIC( RefCount = gAbsMouseRefCount++ );

    if (RefCount == 0){
        VERIFY_EFI_ERROR(
            Status = gBS->AllocatePool(
            EfiBootServicesData,
            sizeof(USB_ABSOLUTE_MOUSE_DEV),
            &UsbAbsMouseDevice));

        EfiZeroMem(UsbAbsMouseDevice, sizeof(USB_ABSOLUTE_MOUSE_DEV));

        //
        // Initialize UsbABSDevice
        //
        UsbAbsMouseDevice->Signature = USB_ABSOLUTE_MOUSE_DEV_SIGNATURE;

        UsbAbsMouseDevice->AbsolutePointerProtocol.GetState = GetAbsMouseState;
        UsbAbsMouseDevice->AbsolutePointerProtocol.Reset = UsbAbsMouseReset;
        UsbAbsMouseDevice->AbsolutePointerProtocol.Mode = &UsbAbsMouseDevice->Mode;

        UsbAbsMouseDevice->Mode.Attributes = EFI_ABSP_SupportsPressureAsZ;

        UsbAbsMouseDevice->Mode.AbsoluteMinX = 0;
        UsbAbsMouseDevice->Mode.AbsoluteMinY = 0;
        UsbAbsMouseDevice->Mode.AbsoluteMinZ = 0;
        UsbAbsMouseDevice->Mode.AbsoluteMaxX = pDevInfo->Hidreport.wAbsMaxX;
        UsbAbsMouseDevice->Mode.AbsoluteMaxY = pDevInfo->Hidreport.wAbsMaxY;
        UsbAbsMouseDevice->Mode.AbsoluteMaxZ = 0;

        EfiZeroMem (&UsbAbsMouseDevice->State, sizeof(EFI_ABSOLUTE_POINTER_STATE));
        UsbAbsMouseDevice->StateChanged = FALSE;

        UsbAbsMouseReset(NULL, FALSE);
        
        VERIFY_EFI_ERROR(
            Status = gBS->CreateEvent (
            EFI_EVENT_NOTIFY_WAIT,
            EFI_TPL_NOTIFY,
            UsbAbsMouseWaitForInput,
            UsbAbsMouseDevice,
            &((UsbAbsMouseDevice->AbsolutePointerProtocol).WaitForInput)
            ));

        USB_DEBUG(DEBUG_LEVEL_4, "ABS event is created, status = %r\n", Status);
    }
    //
    // Install protocol interfaces for the USB ABS device
    //
    VERIFY_EFI_ERROR(
        Status = gBS->InstallProtocolInterface(
        &Controller,
        &gEfiAbsolutePointerProtocolGuid,
        EFI_NATIVE_INTERFACE,
        &UsbAbsMouseDevice->AbsolutePointerProtocol));

    USB_DEBUG(DEBUG_LEVEL_4, "ABS protocol is installed, status = %r\n", Status);
	
	return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        UninstallUSBAbsMouse
//
// Description: Uninstalls ABSOLUTEPointerProtocol interface.
//
// Input:       Controller - controller handle.
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UninstallUSBAbsMouse(
    IN EFI_HANDLE Controller
)
{
    EFI_STATUS Status;
    int RefCount;

    VERIFY_EFI_ERROR(
        Status = gBS->UninstallProtocolInterface(
            Controller,
            &gEfiAbsolutePointerProtocolGuid,
            &UsbAbsMouseDevice->AbsolutePointerProtocol));
    if(EFI_ERROR(Status))
        return Status;

    ATOMIC( RefCount = --gAbsMouseRefCount );
    if (RefCount == 0) {
        VERIFY_EFI_ERROR(
            gBS->CloseEvent (
            (UsbAbsMouseDevice->AbsolutePointerProtocol).WaitForInput));

        gBS->FreePool(UsbAbsMouseDevice);
        UsbAbsMouseDevice = 0;
    }
    return Status;
}




//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        UsbAbsMouseReset
//
// Description: This routine is a part of ABSOLUTEPointerProtocol implementation;
//              it resets USB ABS.
//
// Input:       This - A pointer to the EFI_ABSOLUTE_POINTER_PROTOCOL instance.
//              ExtendedVerification - Indicates that the driver may perform
//              a more exhaustive verification operation of the device during
//              reset.
//
// Output:      EFI_SUCCESS or EFI_DEVICE_ERROR
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static EFI_STATUS
UsbAbsMouseReset(
    IN EFI_ABSOLUTE_POINTER_PROTOCOL    *This,
    IN BOOLEAN                          ExtendedVerification
  )
{

    EfiZeroMem (
            &UsbAbsMouseDevice->State,
            sizeof(EFI_ABSOLUTE_POINTER_STATE)
            );
    UsbAbsMouseDevice->StateChanged = FALSE;
    
    EfiZeroMem (&gUsbData->AbsMouseData, 10 * sizeof(ABS_MOUSE));

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        GetAbsMouseState
//
// Description: This routine is a part of ABSOLUTEPointerProtocol implementation;
//              it retrieves the current state of a pointer device.
//
// Input:       This - A pointer to the EFI_ABSOLUTE_POINTER_PROTOCOL instance.
//              ABSState - A pointer to the state information on the pointer
//              device. Type EFI_ABSOLUTE_POINTER_STATE is defined as follows:
//                typedef struct {
//                    INT32 RelativeMovementX;
//                    INT32 RelativeMovementY;
//                    INT32 RelativeMovementZ;
//                    BOOLEAN LeftButton;
//                    BOOLEAN RightButton;
//                } EFI_ABSOLUTE_POINTER_STATE;
//
// Output:      EFI_SUCCESS      - The state of the pointer device was returned
//                                 in ABSState.
//              EFI_NOT_READY    - The state of the pointer device has not changed
//                                 since the last call to GetABSState().
//              EFI_DEVICE_ERROR - A device error occurred while attempting to
//                                 retrieve the pointer device’s current state.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static EFI_STATUS
GetAbsMouseState(
    EFI_ABSOLUTE_POINTER_PROTOCOL  *This,
    EFI_ABSOLUTE_POINTER_STATE     *AbsMouseState
)
{
    if (AbsMouseState == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    UpdateUsbAbsMouseData (&gUsbData->AbsMouseData[0]);

    if (UsbAbsMouseDevice->StateChanged == FALSE) {
        return EFI_NOT_READY;
    }

    EfiCopyMem(
        AbsMouseState,
        &UsbAbsMouseDevice->State,
        sizeof(EFI_ABSOLUTE_POINTER_STATE)
    );
    UsbAbsMouseDevice->StateChanged = FALSE;

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        UpdateUsbAbsMouseData
//
// Description: This routine updates current AbsMouse data.
//
// Input:       Data* - pointer to the data area to be updated.
//
// Output:      EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static EFI_STATUS
UpdateUsbAbsMouseData (
    ABS_MOUSE *AbsData
)
{
    if(UsbAbsMouseDevice->State.CurrentX != (UINT64)AbsData->Xcoordinate ||
        UsbAbsMouseDevice->State.CurrentY != (UINT64)AbsData->Ycoordinate ||
        UsbAbsMouseDevice->State.CurrentZ != (UINT64)AbsData->Pressure ||
        UsbAbsMouseDevice->State.ActiveButtons != (UINT32)AbsData->ButtonStauts) {
            UsbAbsMouseDevice->StateChanged=TRUE;
    } else {
            UsbAbsMouseDevice->StateChanged=FALSE;
    }

    if(UsbAbsMouseDevice->StateChanged) {
        UsbAbsMouseDevice->State.ActiveButtons = (UINT32)AbsData->ButtonStauts;
        UsbAbsMouseDevice->State.CurrentX = (UINT64)AbsData->Xcoordinate;
        UsbAbsMouseDevice->State.CurrentY = (UINT64)AbsData->Ycoordinate;
        UsbAbsMouseDevice->State.CurrentZ = (UINT64)AbsData->Pressure; 
        UsbAbsMouseDevice->Mode.AbsoluteMaxX= (UINT64)AbsData->Max_X; 
        UsbAbsMouseDevice->Mode.AbsoluteMaxY= (UINT64)AbsData->Max_Y;  
    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        UsbAbsMouseWaitForInput
//
// Description: Event notification function for AbsMouseOLUTE_POINTER.WaitForInput
//              event. Signal the event if there is input from AbsMouse.
//
// Input:       Event - event to signal in case of AbsMouse activity
//              Context - data to pass along with the event.
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static VOID
EFIAPI
UsbAbsMouseWaitForInput (
    EFI_EVENT   Event,
    VOID        *Context
    )
{

    //
    // Someone is waiting on the AbsMouse event, if there's
    // input from AbsMouse, signal the event
    //
    if(UsbAbsMouseDevice->State.CurrentX != (UINT64)gUsbData->AbsMouseData[0].Xcoordinate ||
       UsbAbsMouseDevice->State.CurrentY != (UINT64)gUsbData->AbsMouseData[0].Ycoordinate ||
       UsbAbsMouseDevice->State.CurrentZ != (UINT64)gUsbData->AbsMouseData[0].Pressure ||
       UsbAbsMouseDevice->State.ActiveButtons != (UINT32)gUsbData->AbsMouseData[0].ButtonStauts) {
        gBS->SignalEvent(Event);
    }

    return;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbAbsMsStop
//
// Description: Stops USB ABS device
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbAbsMsStop (
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE Controller,
    UINTN NumberOfChildren,
    EFI_HANDLE *Children
)
{
    EFI_STATUS Status;

    Status = UninstallUSBAbsMouse(Controller);
    VERIFY_EFI_ERROR(
        gBS->CloseProtocol (
        Controller, &gEfiUsbIoProtocolGuid,
        This->DriverBindingHandle, Controller));
    return Status;
}

#endif

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2011, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
