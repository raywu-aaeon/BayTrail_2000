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
// $Header: /Alaska/SOURCE/Core/CORE_DXE/PS2CTL/ps2main.c 21    2/01/12 2:02a Deepthins $
//
// $Revision: 21 $
//
// $Date: 2/01/12 2:02a $
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------
//
// Name: ps2main.c
//
// Description: PS/2 Controller DXE driver
// This driver supports PS/2 keyboard and/or PS/2 mouse depending on the
// switches in SDL tokens.
//
//----------------------------------------------------------------------
//<AMI_FHDR_END>

//----------------------------------------------------------------------

#include "efi.h"
#include "ps2ctl.h"
#include "kbc.h"
#include <Protocol\ComponentName.h>

//----------------------------------------------------------------------

#define BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID \
        {0xdbc9fd21, 0xfad8, 0x45b0, 0x9e, 0x78, 0x27, 0x15, 0x88, 0x67, 0xcc, 0x93}

EFI_GUID    gBdsAllDriversConnectedProtocolGuid = BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID;

static EFI_GUID gDriverBindingProtocolGuid = EFI_DRIVER_BINDING_PROTOCOL_GUID;
#ifndef EFI_COMPONENT_NAME2_PROTOCOL_GUID //old Core
static EFI_GUID gComponentNameProtocolGuid = EFI_COMPONENT_NAME_PROTOCOL_GUID;
#else
static EFI_GUID gComponentNameProtocolGuid = EFI_COMPONENT_NAME2_PROTOCOL_GUID;
#endif
EFI_GUID gDevicePathProtocolGuid = EFI_DEVICE_PATH_PROTOCOL_GUID;
UINT8 gDriverStartCounter;

VOID        *gAllDriverConnectedNotifyRegistration;
EFI_EVENT   gAllDriverConnectedEvent;

extern BOOLEAN Ps2MouseSupport;
extern BOOLEAN Ps2KbdSupport;
extern BOOLEAN KbcBasicAssuranceTest;
extern EFI_COMPONENT_NAME_PROTOCOL  gPS2CtlDriverName;
extern EFI_LEGACY_8259_PROTOCOL     *mLegacy8259;
extern BOOLEAN                      KbRdBeforeInstall; 
extern BOOLEAN                      KbdIrqSupport;
EFI_SYSTEM_TABLE                    *gSysTable = NULL;
extern UINT8                        gKeyboardIrqInstall;
extern  UINT32                      IbFreeTimeoutValue;

  

//==================================================================================
// Function Prototypes for Driver Binding Protocol Interface
//==================================================================================
EFI_STATUS PS2CtlSupported(
        EFI_DRIVER_BINDING_PROTOCOL *This,
        EFI_HANDLE                  Controller,
        EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath);

EFI_STATUS PS2CtlStart(
        EFI_DRIVER_BINDING_PROTOCOL *This,
        EFI_HANDLE                  Controller,
        EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath);

EFI_STATUS PS2CtlStop(
        EFI_DRIVER_BINDING_PROTOCOL *This,
        EFI_HANDLE                  Controller,
        UINTN                       NumberOfChildren,
        EFI_HANDLE                  *ChildHandleBuffer);

//==================================================================================
// Driver binding protocol instance for PS2Ctl Driver
//==================================================================================
EFI_DRIVER_BINDING_PROTOCOL gPS2CtlDriverBinding = {
    PS2CtlSupported,
    PS2CtlStart,
    PS2CtlStop,
    PS2_DRIVER_VER,     // Driver version
    NULL,               // Image Handle
    NULL                // DriverBindingHandle
};

//==================================================================================
// Supported PS2 devices table
//==================================================================================
CHAR16 *gPS2ControllerName = L"PS/2 Controller";

PS2DEV_TABLE    supportedDevices[] = {
    {EISA_PNP_ID(0xF03), 0, DEVICETYPE_MOUSE, StartMouse, StopMouse, L"Microsoft PS/2 Mouse"},
    {EISA_PNP_ID(0xF12), 0, DEVICETYPE_MOUSE, StartMouse, StopMouse, L"Logitech PS/2 Mouse"},
    {EISA_PNP_ID(0xF13), 0, DEVICETYPE_MOUSE, StartMouse, StopMouse, L"Generic PS/2 Mouse"},
    {EISA_PNP_ID(0x303), 1, DEVICETYPE_MOUSE, StartMouse, StopMouse, L"IBM Keyboard, PS/2 Mouse Support"},
    {EISA_PNP_ID(0x303), 0, DEVICETYPE_KEYBOARD, StartKeyboard, StopKeyboard, L"Generic PS/2 Keyboard"},
    {0} // End of table
};


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       PS2CtlEntryPoint
//
// Description:     PS/2 Controller Driver Entry Point
//                  This function is a part of DriverBinfing protocol
//
// Parameters:      EFI_HANDLE ImageHandle - Image handle for this driver
//                                           image
//                  EFI_SYSTEM_TABLE *SystemTable - pointer to the EFI
//                                                  system table
//
// Output:          EFI_STATUS - Status of the operation
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS PS2CtlEntryPoint(
    EFI_HANDLE          ImageHandle,
    EFI_SYSTEM_TABLE    *SystemTable )
{
    EFI_STATUS         Status;

    InitAmiLib(ImageHandle,SystemTable);

    gSysTable = SystemTable;
    gPS2CtlDriverBinding.DriverBindingHandle = NULL;
    gPS2CtlDriverBinding.ImageHandle = ImageHandle;

    Status = gSysTable->BootServices->InstallMultipleProtocolInterfaces(
                &gPS2CtlDriverBinding.DriverBindingHandle,
                &gDriverBindingProtocolGuid, &gPS2CtlDriverBinding,
                &gComponentNameProtocolGuid, &gPS2CtlDriverName,
                NULL);

    gDriverStartCounter = 0;

    //
    // Update the SIO variable in the ACPI name space depend on the 
    // Ps2keyboard and Mouse Present state.
    //
    Status = RegisterProtocolCallback(
                    &gBdsAllDriversConnectedProtocolGuid,
                    UpdateSioVariableForKeyboardMouse,
                    NULL,   
                    &gAllDriverConnectedEvent,
                    &gAllDriverConnectedNotifyRegistration
    );
    ASSERT_EFI_ERROR(Status);

    //
    // Initialized Keyboard irq if keys are to be read before starting driver
    // IRQ handler will save the data in temp buffer and once the Keyboard
    // Driver started , the temp buffer data is pushed into Keyboard driver.
    // So that the key's are pressed in post also taken by Ps2 driver
    //
    if(KbRdBeforeInstall){
        if(IoRead8(KBC_CMDSTS_PORT) != 0xFF) { 
            //
            // Enable the Keyboard and Keyboard Interrupt  
            //
            Write8042CommandByte(0x65);
            InitKeyboardIrq();
        }
    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       PS2CtlSupported
//
// Description:     PS/2 Controller Driver Supported function
//                  This function is a part of DriverBinfing protocol
//
// Paremeters:      EFI_DRIVER_BINDING_PROTOCOL *This - Pointer to this
//                      instance of the driver binding protocol
//                  EFI_HANDLE Controller - Handle for this controller
//                  EFI_DEVICE_PATH_PROTOCOL - *RemainingDevicePath -
//                      Pointer to last node in device path
//
// Output:          EFI_SUCCESS - Ps2 Controller supported
//                  EFI_UNSUPPORTED -- Ps2 Controller not supported
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS PS2CtlSupported(
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE                  Controller,
    EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath)
{
    ACPI_HID_DEVICE_PATH* acpiDP;

    //
    // Checking if KBC is available at all
    //
    if (IoRead8 (KBC_CMDSTS_PORT) == 0xFF){
        return EFI_DEVICE_ERROR;
	}
    //
    // Find the last device node in the device path and return "Supported" 
    // for mouse and/or keyboard depending on the SDL switches.
    //
    if( !EFI_ERROR(GetPS2_DP(This, Controller, &acpiDP, EFI_OPEN_PROTOCOL_BY_DRIVER)) &&
            LookupPs2Hid(supportedDevices, acpiDP->HID, acpiDP->UID, 0)){
        return EFI_SUCCESS;
    } 

    return EFI_UNSUPPORTED;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       PS2CtlStart
//
// Description:     PS/2 Controller Driver Start function
//                  This function is a part of DriverBinfing protocol
//
// Paremeters:      EFI_DRIVER_BINDING_PROTOCOL *This - Pointer to this
//                      instance of the driver binding protocol
//                  EFI_HANDLE Controller - Handle for this controller
//                  EFI_DEVICE_PATH_PROTOCOL - *RemainingDevicePath -
//                      Pointer to last node in device path
//
// Output:          EFI_STATUS - Status of the operation
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS PS2CtlStart(
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE                  Controller,
    EFI_DEVICE_PATH_PROTOCOL    *RemainingDevicePath)
{
    ACPI_HID_DEVICE_PATH    *acpiDP;
    PS2DEV_TABLE            *ps2dev = 0;
    EFI_STATUS              Status;
    EFI_TPL                 OldTpl;

    //
    // The following code performs the basic KBC initialization
    // It has to be executed only once, we use global variable
    // gDriverStartCounter to control this. Also, this code is
    // executed on a higher TPL to prevent re-entrance.
    //
    OldTpl = gSysTable->BootServices->RaiseTPL(TPL_NOTIFY);
    if (OldTpl > TPL_NOTIFY) return EFI_DEVICE_ERROR;
    
    while (!gDriverStartCounter) {
        //
        // Disable the Keyboard IRQ if it's enabled before 
        // Finish all the Keyboard Initilization and Re-enable the IRQ again 
        // 
        if(KbRdBeforeInstall){
            mLegacy8259->DisableIrq( mLegacy8259, SYSTEM_KEYBOARD_IRQ );
            gKeyboardIrqInstall=FALSE;
        }
        // Initialize KBC hardware
        //
        if ( KbcBasicAssuranceTest ) {
            Status = KBCBatTest();      // Perform KBC Basic Assurance Test
            if (EFI_ERROR(Status)) {
                //
                // Report the Error code if the BAT test failed
                //
                ERROR_CODE (DXE_KEYBOARD_STUCK_KEY_ERROR, EFI_ERROR_MAJOR);
                break;
            }

            //
            // Check for stuck keys
            //
            Status = IbFreeTimeout(IbFreeTimeoutValue);
            if (EFI_ERROR(Status)) {
                //
                // Report the Error Code.
                //
                ERROR_CODE (DXE_KEYBOARD_STUCK_KEY_ERROR, EFI_ERROR_MAJOR);
                break;
            }
        }
        // Swap ports if needed
        if (AutodetectKbdMousePortsPtr!=NULL) AutodetectKbdMousePortsPtr();
    
        gDriverStartCounter++;
    }
    
    
    if(!EFI_ERROR(GetPS2_DP(This, Controller, &acpiDP, EFI_OPEN_PROTOCOL_BY_DRIVER)) &&
            LookupPs2Hid(supportedDevices, acpiDP->HID, acpiDP->UID, &ps2dev) ){
             Status = ps2dev->start(This, Controller);

            //
            // End of critical section - restore TPL
            //
            gSysTable->BootServices->RestoreTPL(OldTpl);

            if(EFI_ERROR(Status)) {
                return EFI_DEVICE_ERROR;
            }

            return EFI_SUCCESS;
    } 
        
    //
    // End of critical section - restore TPL
    //
    gSysTable->BootServices->RestoreTPL(OldTpl);

    //
    // If control is here then something totally wrong happend:
    // if device is not supported then Start shouldn't be called.
    //
    return EFI_DEVICE_ERROR;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       PS2CtlStop
//
// Description:     PS/2 Controller Driver Stop function
//                  This function is a part of DriverBinfing protocol
//
// Paremeters:      EFI_DRIVER_BINDING_PROTOCOL *This - Pointer to this
//                      instance of the driver binding protocol
//                  EFI_HANDLE Controller - Handle for this controller
//                  UINTN NumberOfChildren - Number of children of this
//                      controller
//                  EFI_HANDLE *ChildHandleBuffer - Pointer to a buffer
//                      for child handles
//
// Output:          EFI_STATUS - Status of the operation
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS PS2CtlStop(
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE                  Controller,
    UINTN                       NumberOfChildren,
    EFI_HANDLE                  *ChildHandleBuffer)
{
    ACPI_HID_DEVICE_PATH        *acpiDP;
    PS2DEV_TABLE                *ps2dev = 0;
    EFI_STATUS                  Status;

    if(!EFI_ERROR(GetPS2_DP(This, Controller, &acpiDP, EFI_OPEN_PROTOCOL_GET_PROTOCOL)) &&
            LookupPs2Hid(supportedDevices, acpiDP->HID, acpiDP->UID, &ps2dev) ){
        Status = ps2dev->stop(This, Controller);
        if(EFI_ERROR(Status)) {
            return EFI_DEVICE_ERROR;
        }
        return EFI_SUCCESS;
    } 

    //
    // If control is here then device path was not found in the lookup table
    //
    return EFI_DEVICE_ERROR;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       LookupPs2Hid
//
// Description:     This fuction searches the PS2 device in table that 
//                  matches given HID and UID
//
// Paremeters:
//  PS2DEV_TABLE *  devTable - Lookup table pointer
//  UINT32          hid - HID to look for
//  UINT32          uid - UID to look for
//  PS2DEV_TABLE**  dev - address of the matched table entry
//
// Output:
//          BOOLEAN - TRUE if match is found, FALSE otherwise
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN LookupPs2Hid(
    PS2DEV_TABLE *  devTable,
    UINT32          hid,
    UINT32          uid,
    PS2DEV_TABLE**  dev)
{
    for( ;devTable->hid;++devTable){
        if( devTable->hid == hid && devTable->uid==uid){
            if ( (devTable->DeviceType == DEVICETYPE_MOUSE && Ps2MouseSupport) ||
                 (devTable->DeviceType == DEVICETYPE_KEYBOARD && Ps2KbdSupport) ) {
                if(dev) *dev = devTable;
                return TRUE;
            }
        } 
    }
    return FALSE;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
//
// Procedure:       GetPS2_DP
//
// Description:     This fuction returns the last node in the device 
//                  path for the given controller.
//
// Paremeters:      EFI_DRIVER_BINDING_PROTOCOL *This - Pointer to this
//                      instance of the driver binding protocol
//                  EFI_HANDLE Controller - Handle for this controller
//                  ACPI_HID_DEVICE_PATH** ps2dp - Pointer to ACPI HID
//                      device path
//                  UINT32 Attributes - Attributes passed to driver binding
//                      protocol
//
// Output:
//          EFI_SUCCESS or EFI_UNSUPPORTED
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS GetPS2_DP(
    EFI_DRIVER_BINDING_PROTOCOL *This,
    EFI_HANDLE                  Controller,
    ACPI_HID_DEVICE_PATH**      ps2dp,
    UINT32                      Attributes)
{   
    EFI_STATUS Status;
    ACPI_HID_DEVICE_PATH        *acpiDP;
    EFI_DEVICE_PATH_PROTOCOL    * ps2DevPath, *pDP;

    //
    // Get device path from Controller handle.
    //
    Status = gSysTable->BootServices->OpenProtocol (
                Controller,
                &gDevicePathProtocolGuid,
                (VOID**)&pDP,
                This->DriverBindingHandle,
                Controller,   
                Attributes
                );
  
    if (EFI_ERROR(Status)) {
        if( Status != (EFI_ALREADY_STARTED || EFI_ACCESS_DENIED) ) {
            return EFI_UNSUPPORTED;
        } else { 
            return Status;
        }
    }

    //
    // Process ps2DevPath - get the node which is before the EndNode
    //
    ps2DevPath=DPGetLastNode(pDP);

    //
    // ps2DevPath is now the last node
    //
    acpiDP = *ps2dp = (ACPI_HID_DEVICE_PATH*)ps2DevPath;

    Status = (acpiDP->Header.Type == ACPI_DEVICE_PATH && 
        acpiDP->Header.SubType == ACPI_DP)? EFI_SUCCESS : EFI_UNSUPPORTED;

    if (Attributes!=EFI_OPEN_PROTOCOL_GET_PROTOCOL)
        gSysTable->BootServices->CloseProtocol (
            Controller,
            &gDevicePathProtocolGuid,
            This->DriverBindingHandle,
            Controller
            );

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
