//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2008, American Megatrends, Inc.          **
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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/efiusbms.c 22    9/19/11 9:31a Lavanyap $
//
// $Revision: 22 $
//
// $Date: 9/19/11 9:31a $
//
//****************************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:           EFIUSBMS.C
//
//  Description:    EFI USB Mouse Driver
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"
#include "UsbDef.h"
#include "Uhcd.h"
#include "ComponentName.h"
#include "UsbBus.h"


#define USBMS_DRIVER_VERSION 2

#define USB_MOUSE_DEV_SIGNATURE   EFI_SIGNATURE_32('u','m','o','u')
#define CR(record, TYPE, field, signature) _CR(record, TYPE, field) 
#define USB_MOUSE_DEV_FROM_MOUSE_PROTOCOL(a,b) \
    CR(a, USB_MOUSE_DEV, b, USB_MOUSE_DEV_SIGNATURE)

typedef struct
{
    UINTN                           Signature;
    UINT8                           NumberOfButtons;
    INT32                           XLogicMax;
    INT32                           XLogicMin;
    INT32                           YLogicMax;
    INT32                           YLogicMin;
    EFI_SIMPLE_POINTER_PROTOCOL     SimplePointerProtocol;
    EFI_SIMPLE_POINTER_MODE         Mode;
    UINT8                           Endpoint;
    EFI_USB_IO_PROTOCOL             *UsbIo;
} USB_MOUSE_DEV;

static VOID
UsbMouseWaitForInput (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  );

static EFI_STATUS
UpdateUsbMouseData (
    EFI_SIMPLE_POINTER_PROTOCOL  *This, 
	EFI_SIMPLE_POINTER_STATE	*State
  );

//
// Mouse Protocol
//
static EFI_STATUS
GetMouseState(
  IN   EFI_SIMPLE_POINTER_PROTOCOL  *This,
  OUT  EFI_SIMPLE_POINTER_STATE     *MouseState
);

static EFI_STATUS
UsbMouseReset(
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN BOOLEAN                        ExtendedVerification
  );

extern USB_GLOBAL_DATA *gUsbData;

BOOLEAN                         StateChanged; 
UINT8                           ButtonsState; 
EFI_SIMPLE_POINTER_STATE        MsState;



//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        InitUSBMouse
//
// Description: Initialize USB mouse device and all private data structures.
//
// Input:       None
//
// Output:      EFI_SUCCESS or EFI_ERROR
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
InitUSBMouse()
{
    EfiZeroMem (&MsState, sizeof(EFI_SIMPLE_POINTER_STATE));
    ButtonsState = 0;
    StateChanged = FALSE; 
    return EFI_SUCCESS;
}  

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        InstallUSBMouse
//
// Description: Installs SimplePointerProtocol interface on a given controller.
//
// Input:       Controller - controller handle to install interface on.
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
InstallUSBMouse(
    EFI_HANDLE Controller,
    EFI_USB_IO_PROTOCOL *UsbIo,
    DEV_INFO *DevInfo
)
{
    USB_MOUSE_DEV       *UsbMouse; 
    EFI_STATUS Status;

    VERIFY_EFI_ERROR(
        Status = gBS->AllocatePool(
        EfiBootServicesData,
        sizeof(USB_MOUSE_DEV),
        &UsbMouse));

    EfiZeroMem(UsbMouse, sizeof(USB_MOUSE_DEV));

    //
    // Initialize UsbMouseDevice
    //
    UsbMouse->Signature = USB_MOUSE_DEV_SIGNATURE;
    UsbMouse->SimplePointerProtocol.GetState = GetMouseState;
    UsbMouse->SimplePointerProtocol.Reset = UsbMouseReset;
    UsbMouse->SimplePointerProtocol.Mode = &UsbMouse->Mode;

    UsbMouse->NumberOfButtons = 2;
    UsbMouse->XLogicMax = UsbMouse->YLogicMax = 127;
    UsbMouse->XLogicMin = UsbMouse->YLogicMin = -127;

    UsbMouse->Mode.LeftButton = TRUE;
    UsbMouse->Mode.RightButton = TRUE;
    UsbMouse->Mode.ResolutionX = 8;
    UsbMouse->Mode.ResolutionY = 8;
    UsbMouse->Mode.ResolutionZ = 8; 

    UsbMouse->UsbIo = UsbIo;
    UsbMouse->Endpoint = DevInfo->bIntEndpoint;

    UsbMouseReset(NULL, FALSE);
 
    VERIFY_EFI_ERROR(
        Status = gBS->CreateEvent (
        EFI_EVENT_NOTIFY_WAIT,
        EFI_TPL_NOTIFY,
        UsbMouseWaitForInput,
        UsbMouse,
        &((UsbMouse->SimplePointerProtocol).WaitForInput)
        ));

    USB_DEBUG(DEBUG_LEVEL_4, "Mouse event is created, status = %r\n", Status);

    //
    // Install protocol interfaces for the USB mouse device
    //
    VERIFY_EFI_ERROR(
        Status = gBS->InstallProtocolInterface(
        &Controller,
        &gEfiSimplePointerProtocolGuid,
        EFI_NATIVE_INTERFACE,
        &UsbMouse->SimplePointerProtocol));

    USB_DEBUG(DEBUG_LEVEL_4, "Mouse protocol is installed, status = %r\n", Status);
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UninstallUSBMouse
//
// Description: Stops USB mouse device
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UninstallUSBMouse (
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE Controller,
    UINTN NumberOfChildren,
    EFI_HANDLE *Children
)
{
    EFI_STATUS Status;
    EFI_SIMPLE_POINTER_PROTOCOL     *SimplePoint; 
    USB_MOUSE_DEV       *UsbMouse = 0; 
 

    Status = pBS->OpenProtocol( Controller,
                                &gEfiSimplePointerProtocolGuid,
                                (VOID **)&SimplePoint,
                                This->DriverBindingHandle,
                                Controller,
                                EFI_OPEN_PROTOCOL_GET_PROTOCOL); 

    UsbMouse = USB_MOUSE_DEV_FROM_MOUSE_PROTOCOL(SimplePoint,SimplePointerProtocol);
    VERIFY_EFI_ERROR(
        Status = gBS->UninstallProtocolInterface(
            Controller,
            &gEfiSimplePointerProtocolGuid,
            &UsbMouse->SimplePointerProtocol));
    if(EFI_ERROR(Status))
        return Status;

        VERIFY_EFI_ERROR(
            gBS->CloseEvent (
            (UsbMouse->SimplePointerProtocol).WaitForInput));


        gBS->FreePool(UsbMouse);
        UsbMouse = 0;

    return Status;
} 


/************ SimplePointer Protocol implementation routines*************/

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        UsbMouseReset
//
// Description: This routine is a part of SimplePointerProtocol implementation;
//              it resets USB mouse.
//
// Input:       This - A pointer to the EFI_SIMPLE_POINTER_PROTOCOL instance.
//              ExtendedVerification - Indicates that the driver may perform
//              a more exhaustive verification operation of the device during
//              reset.
//
// Output:      EFI_SUCCESS or EFI_DEVICE_ERROR
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static EFI_STATUS
UsbMouseReset(
    IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
    IN BOOLEAN                        ExtendedVerification
  )
{
    EfiZeroMem (
        &MsState,
        sizeof(EFI_SIMPLE_POINTER_STATE)
        );
    StateChanged = FALSE;
    
    EfiZeroMem (&gUsbData->MouseData, sizeof(MOUSE_DATA));

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        GetMouseState
//
// Description: This routine is a part of SimplePointerProtocol implementation;
//              it retrieves the current state of a pointer device.
//
// Input:       This - A pointer to the EFI_SIMPLE_POINTER_PROTOCOL instance.
//              MouseState - A pointer to the state information on the pointer
//              device. Type EFI_SIMPLE_POINTER_STATE is defined as follows:
//                typedef struct {
//                    INT32 RelativeMovementX;
//                    INT32 RelativeMovementY;
//                    INT32 RelativeMovementZ;
//                    BOOLEAN LeftButton;
//                    BOOLEAN RightButton;
//                } EFI_SIMPLE_POINTER_STATE;
//
// Output:      EFI_SUCCESS      - The state of the pointer device was returned
//                                 in MouseState.
//              EFI_NOT_READY    - The state of the pointer device has not changed
//                                 since the last call to GetMouseState().
//              EFI_DEVICE_ERROR - A device error occurred while attempting to
//                                 retrieve the pointer device�s current state.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static EFI_STATUS
GetMouseState(
    EFI_SIMPLE_POINTER_PROTOCOL  *This,
    EFI_SIMPLE_POINTER_STATE     *MouseState
)
{
    if (MouseState == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    return UpdateUsbMouseData(This,MouseState);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        UpdateUsbMouseData
//
// Description: This routine updates current mouse data.
//
// Input:       Data* - pointer to the data area to be updated.
//
// Output:      EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static EFI_STATUS
UpdateUsbMouseData (
    EFI_SIMPLE_POINTER_PROTOCOL  *This,
	EFI_SIMPLE_POINTER_STATE	*State
)
{
    BOOLEAN LeftButton, RightButton;
    INT32   rX, rY, rZ;
    UINT8   bData;
    EFI_STATUS  Status;
    UINT8   MouseData[4];
    UINTN   DataLength;
    UINT32  UsbStatus;
    INT32   Coordinates;
    USB_MOUSE_DEV       *UsbMouse = 0; 
 
    if ((gUsbData->dUSBStateFlag & USB_FLAG_EFIMS_DIRECT_ACCESS) && (This != NULL) ){
        UsbMouse = USB_MOUSE_DEV_FROM_MOUSE_PROTOCOL(This,SimplePointerProtocol); 
        // Get the data from mouse
        DataLength = 4;
    
        Status = UsbMouse->UsbIo->UsbSyncInterruptTransfer(
            UsbMouse->UsbIo,
            UsbMouse->Endpoint | 0x80,    // IN
            MouseData,
            &DataLength,
            0,  // Timeout
            &UsbStatus
        );
    
        gUsbData->MouseData.ButtonStatus = MouseData[0];
    
        Coordinates = (INT8)MouseData[1];
        gUsbData->MouseData.MouseX += Coordinates;
        Coordinates = (INT8)MouseData[2];
        gUsbData->MouseData.MouseY += Coordinates;
     }

    bData = gUsbData->MouseData.ButtonStatus & 7;

    //
    // Check mouse Data
    //
	LeftButton=(BOOLEAN)(bData & 0x01)?TRUE:FALSE;
	RightButton=(BOOLEAN)(bData & 0x02)?TRUE:FALSE;

    rX = gUsbData->MouseData.MouseX;
    rY = gUsbData->MouseData.MouseY;
    rZ = - (gUsbData->MouseData.MouseZ);

	if (StateChanged == FALSE) {
		if (rX == 0 && rY == 0 && rZ == 0 && 
			bData == ButtonsState) {
			return EFI_NOT_READY;
		}
        StateChanged = TRUE;
	}

    gUsbData->MouseData.MouseX=0;
    gUsbData->MouseData.MouseY=0;
    gUsbData->MouseData.MouseZ=0;

    ButtonsState = bData;
    MsState.LeftButton = LeftButton;
    MsState.RightButton = RightButton;
    MsState.RelativeMovementX += rX;
    MsState.RelativeMovementY += rY;
    MsState.RelativeMovementZ += rZ; 


	if (State != NULL) {
		EfiCopyMem(State, &MsState, sizeof(EFI_SIMPLE_POINTER_STATE));
	    //
	    // Clear previous move state
	    //
        MsState.RelativeMovementX = 0;
        MsState.RelativeMovementY = 0;
        MsState.RelativeMovementZ = 0;  
        StateChanged = FALSE;
	}

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        UsbMouseWaitForInput
//
// Description: Event notification function for SIMPLE_POINTER.WaitForInput
//              event. Signal the event if there is input from mouse.
//
// Input:       Event - event to signal in case of mouse activity
//              Context - data to pass along with the event.
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static VOID
EFIAPI
UsbMouseWaitForInput (
    EFI_EVENT   Event,
    VOID        *Context
    )
{
	EFI_STATUS Status;
    USB_MOUSE_DEV       *UsbMouse = (USB_MOUSE_DEV*)Context; 

    Status = UpdateUsbMouseData (&UsbMouse->SimplePointerProtocol,NULL);
	if (EFI_ERROR(Status)) {
		return;
	}

    //
    // Someone is waiting on the mouse event, if there's
    // input from mouse, signal the event
    //
    gBS->SignalEvent(Event);

}



//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbMsInit
//
// Description: Initialize USB Mouse driver
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

CHAR16*
UsbMsGetControllerName(
    EFI_HANDLE Controller,
    EFI_HANDLE Child
)
{
    return 0;
}

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2008, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
