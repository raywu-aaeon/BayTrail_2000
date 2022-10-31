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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/RT/amiusb.c 94    8/29/12 8:10a Ryanchou $
//
// $Revision: 94 $
//
// $Date: 8/29/12 8:10a $
//
//****************************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:   AMIUSB.C
//
//  Description:    AMI USB API implementation. The following code will be
//                  copied to SMM; only RT functions can be used. gUsbData
//                  is obtained from AMIUHCD in the entry point and can be
//                  used afterwards.
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <AmiDxeLib.h>
#include "AmiUsb.h"
#include "AmiDef.h"
#include <UsbDevDriverElinks.h>			//(EIP71750+)

#if USB_DEV_KBD
#include "UsbKbd.h"
#endif
#include "UsbMass.h"

										//(EIP54018+)>
#if USB_S5_WAKEUP_SUPPORT
#include <Protocol/SmmSxDispatch.h>
#include <Protocol/SmmPowerButtonDispatch.h>
#endif
										//<(EIP54018+)
//#pragma warning (disable: 4152)

extern UINT8 IsKbcAccessBlocked;            //(EIP29733+)
EFI_EMUL6064MSINPUT_PROTOCOL* gMsInput = 0;
EFI_EMUL6064KBDINPUT_PROTOCOL* gKbdInput = 0;
EFI_EMUL6064TRAP_PROTOCOL* gEmulationTrap = 0;

USB_GLOBAL_DATA     *gUsbData;
//USB_BADDEV_STRUC    *gUsbBadDeviceTable;			//(EIP60706-)

AMI_USB_SMM_PROTOCOL	gUsbSmmProtocol = {0};

VOID    StopControllerType(UINT8);
VOID    StartControllerType(UINT8);
UINT8   USB_StopDevice (HC_STRUC*,  UINT8, UINT8);
UINT8   USB_EnumerateRootHubPorts(UINT8);   //(EIP57521+)              
VOID    StopControllerBdf(UINT16);			//(EIP74876+)

VOID	FillHcdEntries();
										//(EIP71750+)>
typedef VOID USB_DEV_DELAYED_DRIVER_CHECK (DEV_DRIVER*);
extern USB_DEV_DELAYED_DRIVER_CHECK USB_DEV_DELAYED_DRIVER EndOfUsbDevDelayedDriverList;
USB_DEV_DELAYED_DRIVER_CHECK* UsbDevDelayedDrivers[]= {USB_DEV_DELAYED_DRIVER NULL};

typedef VOID USB_DEV_DRIVER_CHECK (DEV_DRIVER*);
extern USB_DEV_DRIVER_CHECK USB_DEV_DRIVER EndOfUsbDevDriverList;
USB_DEV_DRIVER_CHECK* UsbDevDrivers[]= {USB_DEV_DRIVER NULL};
										//<(EIP71750+)
										//(EIP54018+)>
#if USB_S5_WAKEUP_SUPPORT
VOID    UsbSuspend(VOID);
#endif

extern	UINT8	UHCI_FillHCDEntries(HCD_HEADER*);
extern	UINT8	OHCI_FillHCDEntries(HCD_HEADER*);
extern	UINT8	EHCI_FillHCDEntries(HCD_HEADER*);
extern	UINT8	XHCI_FillHCDEntries(HCD_HEADER*);

void FillHcdEntries()
{
#if UHCI_SUPPORT
	UHCI_FillHCDEntries (&gUsbData->aHCDriverTable[USB_INDEX_UHCI]);
#endif
#if OHCI_SUPPORT
	OHCI_FillHCDEntries (&gUsbData->aHCDriverTable[USB_INDEX_OHCI]);
#endif
#if EHCI_SUPPORT
	EHCI_FillHCDEntries (&gUsbData->aHCDriverTable[USB_INDEX_EHCI]);
#endif
#if XHCI_SUPPORT
	XHCI_FillHCDEntries (&gUsbData->aHCDriverTable[USB_INDEX_XHCI]);
#endif
}

#if USB_S5_WAKEUP_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        UsbS5SmiCallback
//
// Description: 
//  This function enter usb s5 callback.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
UsbS5SmiCallback(
    IN EFI_HANDLE DispatchHandle,
    IN EFI_SMM_SX_DISPATCH_CONTEXT *DispatchContext
)
{
    UsbSuspend();
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        UsbPowerButtonSmiCallback
//
// Description: 
//  This function enter s5 callback if press power button.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
UsbPowerButtonSmiCallback(
    IN EFI_HANDLE DispatchHandle,
	IN EFI_SMM_POWER_BUTTON_DISPATCH_CONTEXT *DispatchContext
)
{
    UsbSuspend();
}
#endif
										//<(EIP54018+)

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name:        aUsbApiTable - USB API Function Dispatch Table
//
// Type:        Function Dispatch Table
//
// Description: This is the table of functions used by USB API
//
// Notes:       This functions are invoked via software SMI
//
//----------------------------------------------------------------------------
//<AMI_THDR_END>

API_FUNC aUsbApiTable[] = {
    USBAPI_CheckPresence,
    USBAPI_Start,
    USBAPI_Stop,
    USBAPI_DisableInterrupts,
    USBAPI_EnableInterrupts,
    USBAPI_MoveDataArea,
    USBAPI_GetDeviceInfo,
    USBAPI_CheckDevicePresence,
    USBAPI_MassDeviceRequest,
    USBAPI_PowerManageUSB,
    USBAPI_PrepareForOS,
    USBAPI_SecureInterface,
    USBAPI_LightenKeyboardLEDs,
    USBAPI_ChangeOwner,
    USBAPI_HC_Proc,
    USBAPI_Core_Proc,
    USBAPI_LightenKeyboardLEDs_Compatible,
    USBAPI_KbcAccessControl,
    USBAPI_LegacyControl,
    USBAPI_GetDeviceAddress,
    USBAPI_ExtDriverRequest,
    USBAPI_CCIDRequest,
    USBAPI_UsbStopController,				//(EIP74876+)
    USBAPI_HcStartStop
};

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name:        USBMassAPITable - USB Mass Storage API Function Dispatch Table
//
// Type:        Function Dispatch Table
//
// Description: This is the table of functions used by USB Mass Storage API
//
//----------------------------------------------------------------------------
//<AMI_THDR_END>

API_FUNC aUsbMassApiTable[] = {
    USBMassAPIGetDeviceInformation, // USB Mass API Sub-Func 00h
    USBMassAPIGetDeviceGeometry,    // USB Mass API Sub-Func 01h
    USBMassAPIResetDevice,          // USB Mass API Sub-Func 02h
    USBMassAPIReadDevice,           // USB Mass API Sub-Func 03h
    USBMassAPIWriteDevice,          // USB Mass API Sub-Func 04h
    USBMassAPIVerifyDevice,         // USB Mass API Sub-Func 05h
    USBMassAPIFormatDevice,         // USB Mass API Sub-Func 06h
    USBMassAPICommandPassThru,      // USB Mass API Sub-Func 07h
    USBMassAPIAssignDriveNumber,    // USB BIOS API function 08h
    USBMassAPICheckDevStatus,       // USB BIOS API function 09h
    USBMassAPIGetDevStatus,         // USB BIOS API function 0Ah
    USBMassAPIGetDeviceParameters   // USB BIOS API function 0Bh
};

EFI_DRIVER_ENTRY_POINT(USBDriverEntryPoint)

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   USBDriverEntryPoint
//
// Description: USB Driver entry point
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
USBDriverEntryPoint(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    EFI_STATUS Status;

    InitAmiLib(ImageHandle, SystemTable);

#if USB_RUNTIME_DRIVER_IN_SMM
    Status = InitSmmHandler(ImageHandle, SystemTable, InSmmFunction, NULL);
#else
	Status = InstallUsbProtocols(); 
	InitializeUsbGlobalData();
#endif
    ASSERT_EFI_ERROR(Status);

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:       InitializeUsbGlobalData
//
// Description: This function initializes the USB global data.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
InitializeUsbGlobalData(
	VOID
)
{
	EFI_STATUS	Status;
    UINT8   	bDriverIndex;
    UINT8   	bDelayedIndex;
    EFI_PHYSICAL_ADDRESS    MemAddress;

    //
    // Initialize host controller drivers
    //
    FillHcdEntries();   // This routine is implemented in $(BUILD_DIR)\dummyusbrt.c

    //
    // Initialize the device driver pointers
    //
    bDriverIndex = 0;
    bDelayedIndex = 0;
										//(EIP71750)>
    while(UsbDevDelayedDrivers[bDelayedIndex]) {
        UsbDevDelayedDrivers[bDelayedIndex](&gUsbData->aDelayedDrivers[bDelayedIndex]);
		if (gUsbData->aDelayedDrivers[bDelayedIndex].pfnDeviceInit) {
            (*gUsbData->aDelayedDrivers[bDelayedIndex].pfnDeviceInit)();
        }
        if (gUsbData->aDelayedDrivers[bDelayedIndex].bDevType) {
            bDelayedIndex++;
        }
    }

    while(UsbDevDrivers[bDriverIndex]) {
        UsbDevDrivers[bDriverIndex](&gUsbData->aDevDriverTable[bDriverIndex]);
		if (gUsbData->aDevDriverTable[bDriverIndex].pfnDeviceInit) {
            (*gUsbData->aDevDriverTable[bDriverIndex].pfnDeviceInit)();
        }
        if (gUsbData->aDevDriverTable[bDriverIndex].bDevType) {
            bDriverIndex++;
        }
    }

	//
	// Allocate a block of memory to be used as a temporary
	// buffer for  USB mass transfer
	//
	MemAddress = 0xFFFFFFFF;
	Status = gBS->AllocatePages(AllocateMaxAddress, EfiRuntimeServicesData,
					EFI_SIZE_TO_PAGES(MAX_CONSUME_BUFFER_SIZE), 
					&MemAddress);

	ASSERT_EFI_ERROR(Status);
    gUsbData->fpUSBMassConsumeBuffer = (VOID*)(UINTN)MemAddress;
	pBS->SetMem(gUsbData->fpUSBMassConsumeBuffer, MAX_CONSUME_BUFFER_SIZE, 0);

	//
	// Allocate a block of memory for the temporary buffer
	//
    MemAddress = 0xFFFFFFFF;
	Status = gBS->AllocatePages(AllocateMaxAddress, EfiRuntimeServicesData,
					EFI_SIZE_TO_PAGES(MAX_TEMP_BUFFER_SIZE), 
					&MemAddress);

	ASSERT_EFI_ERROR(Status);
    gUsbData->fpUSBTempBuffer = (VOID*)(UINTN)MemAddress;
	pBS->SetMem(gUsbData->fpUSBTempBuffer, MAX_TEMP_BUFFER_SIZE, 0);

    //
    // Allow to enumerate ports
    //
    gUsbData->bEnumFlag = FALSE;

	return USB_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:       UsbApiHandler
//
// Description: 
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
UsbApiHandler(VOID* Param)
{
    URP_STRUC   *fpURP = (URP_STRUC*)Param;
    UINT8       bFuncIndex;
    UINT8       bNumberOfFunctions;

	if (fpURP == NULL) {
		return;
	}

    bFuncIndex = fpURP->bFuncNumber;
    bNumberOfFunctions = sizeof aUsbApiTable / sizeof (API_FUNC *);

    //
    // Make sure function number is valid; if function number is not zero
    // check for valid extended USB API function
    //
    if (bFuncIndex && ((bFuncIndex < USB_NEW_API_START_FUNC ) ||
            bFuncIndex > (bNumberOfFunctions + USB_NEW_API_START_FUNC))) {
        fpURP->bRetValue = USBAPI_INVALID_FUNCTION;
        return;
    }

    if (bFuncIndex) {
        bFuncIndex = (UINT8)(bFuncIndex - USB_NEW_API_START_FUNC + 1);
    }

    aUsbApiTable[bFuncIndex](fpURP);    // Call the appropriate function

}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:       InstallUsbProtocols
//
// Description: This function initializes the USB global data.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
InstallUsbProtocols(
	VOID
)
{
	EFI_STATUS	Status;
	EFI_USB_PROTOCOL	*UsbProtocol;

    Status = pBS->LocateProtocol(&gEfiUsbProtocolGuid, NULL, &UsbProtocol);
	if (EFI_ERROR(Status)) {
		return Status;
	}
    gUsbData = UsbProtocol->USBDataPtr;

	UsbProtocol->UsbRtKbcAccessControl = UsbKbcAccessControl;

	//Hook USB legacy control function for shutdown/init USB legacy support
	UsbProtocol->UsbLegacyControl = USBRT_LegacyControl;
	UsbProtocol->UsbStopUnsupportedHc = USB_StopUnsupportedHc;
	UsbProtocol->UsbInvokeApi = UsbApiHandler;

	return Status;
}

#if USB_RUNTIME_DRIVER_IN_SMM
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InSmmFunction
//
// Description: SMM entry point of AMIUSB driver
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
InSmmFunction(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE    *SystemTable
)
{
    EFI_STATUS                      Status;
	EFI_HANDLE                      SwSmiHandle = NULL;
    EFI_SMM_SW_REGISTER_CONTEXT     SwSmiContext;
	EFI_SMM_SW_DISPATCH2_PROTOCOL    *SwSmiDispatch;
    UINT32                          KbcEmulFeature = 0;
    VOID	                        *ProtocolNotifyRegistration;
    EFI_EVENT                       Emul6064Event = NULL;

									//(EIP54018+)>
#if USB_S5_WAKEUP_SUPPORT
    EFI_SMM_SX_DISPATCH_CONTEXT     S5DispatchContext;
    EFI_SMM_SX_DISPATCH_PROTOCOL    *SxDispatchProtocol;
    EFI_SMM_POWER_BUTTON_DISPATCH_CONTEXT   PbDispatchContext;
    EFI_SMM_POWER_BUTTON_DISPATCH_PROTOCOL  *PbDispatchProtocol;
    EFI_HANDLE  S5DispatchHandle;
    EFI_HANDLE  PbDispatchHandle;
#endif
									//<(EIP54018+)
	EFI_HANDLE	UsbSmmProtocolHandle = NULL;

	Status = InitAmiSmmLib( ImageHandle, SystemTable );
	if (EFI_ERROR(Status)) return Status;

	InstallUsbProtocols();
	InitializeUsbGlobalData();

    Status = pSmst->SmmLocateProtocol(
                    &gEmul6064TrapProtocolGuid,
                    NULL,
                    &gEmulationTrap);

    if (EFI_ERROR(Status)) {
        Status = pSmst->SmmRegisterProtocolNotify (
                    &gEmul6064TrapProtocolGuid,
                    Emul6064TrapCallback,
                    &ProtocolNotifyRegistration
                    );
    }
    if (!gUsbData->kbc_support) {
        Status = pSmst->SmmLocateProtocol(
                        &gEmul6064MsInputProtocolGuid,
                        NULL,
        			    &gMsInput);

        Status = pSmst->SmmLocateProtocol(
                        &gEmul6064KbdInputProtocolGuid,
                        NULL,
                        &gKbdInput);

        if (Status == EFI_SUCCESS) {
            gUsbData->dUSBStateFlag |= USB_FLAG_6064EMULATION_ON;
            if (gEmulationTrap) {
                KbcEmulFeature = gEmulationTrap->FeatureSupported(gEmulationTrap);
            }
            if (KbcEmulFeature & IRQ_SUPPORTED) {
                gUsbData->dUSBStateFlag |= USB_FLAG_6064EMULATION_IRQ_SUPPORT;
            }
        } else {
           InitSysKbc( &gKbdInput, &gMsInput );
        }
    } else {
        //
        //Init Fake Emulation interface
        //
        InitSysKbc( &gKbdInput, &gMsInput );
    }

    Status = USBSB_InstallSmiEventHandlers();

    USB_DEBUG(DEBUG_LEVEL_3,"AMIUSB global data at 0x%x\n", gUsbData);

    //
    // Register the USB SW SMI handler
    //
   Status = pSmst->SmmLocateProtocol (&gEfiSmmSwDispatch2ProtocolGuid, NULL, &SwSmiDispatch);

    if (EFI_ERROR (Status)) {
        USB_DEBUG(DEBUG_LEVEL_0, "SmmSwDispatch protocol: %r\n", Status);
        return Status;
    }

    SwSmiContext.SwSmiInputValue = USB_SWSMI;
    Status = SwSmiDispatch->Register (SwSmiDispatch, USBSWSMIHandler, &SwSmiContext, &SwSmiHandle);

    USB_DEBUG(DEBUG_LEVEL_3, "AMIUSB SW SMI registration:: %r\n", Status);

	gUsbSmmProtocol.UsbStopUnsupportedHc = USB_StopUnsupportedHc;
    Status = pSmst->SmmInstallProtocolInterface(
                    &UsbSmmProtocolHandle,
                    &gAmiUsbSmmProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &gUsbSmmProtocol
                    );
	
	USB_DEBUG(DEBUG_LEVEL_3, "AMIUSB SMM protocol: %r\n", Status);
										//(EIP54018+)>
#if USB_S5_WAKEUP_SUPPORT
    Status = pBS->LocateProtocol(
        &gEfiSmmSxDispatchProtocolGuid,
        NULL,
        &SxDispatchProtocol
    );

    if (EFI_ERROR(Status)) {
        return Status;
    }

    S5DispatchContext.Type  = SxS5;
    S5DispatchContext.Phase = SxEntry;
    Status = SxDispatchProtocol->Register(
        SxDispatchProtocol,
        UsbS5SmiCallback,
        &S5DispatchContext,
        &S5DispatchHandle
    );

    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Locate Power Button Dispatch Protocol
    Status = pBS->LocateProtocol(
        &gEfiSmmPowerButtonDispatchProtocolGuid,
        NULL,
        &PbDispatchProtocol
    );

    if (EFI_ERROR(Status)) {
        return Status;
    }

    // Register the handler for power button presses
    PbDispatchContext.Phase = PowerButtonEntry;
    Status = PbDispatchProtocol->Register(
        PbDispatchProtocol,
        UsbPowerButtonSmiCallback,
        &PbDispatchContext,
        &PbDispatchHandle
    );
#endif
										//<(EIP54018+)

    return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBSWSMIHandler
//
// Description: Invoked on reads from SW SMI port with value USB_SWSMI. This
//              function dispatches the USB Request Packets (URP) to the
//              appropriate functions.
//
// Input:       EBDA:USB_DATA_EBDA_OFFSET - Pointer to the URP (USB Request
//              Packet structure)
//              DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT
//
// Output:      bRetValue   Zero on successfull completion
//              Non-zero on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
USBSWSMIHandler (
    EFI_HANDLE	DispatchHandle,
    CONST VOID	*Context OPTIONAL,
    VOID		*CommBuffer OPTIONAL,
    UINTN	    *CommBufferSize OPTIONAL
)
{
    URP_STRUC   *fpURP;
    UINT16      EbdaSeg;

	if (gUsbData->fpURP) {			// Call from AMIUSB C area
		fpURP = gUsbData->fpURP;
		gUsbData->fpURP = 0;   		// Clear the switch
	} else {
        //
        // Get the fpURP pointer from EBDA
        //
        EbdaSeg = *((UINT16*)0x40E);
        fpURP = *(URP_STRUC**)(UINTN)(((UINT32)EbdaSeg << 4) + USB_DATA_EBDA_OFFSET);
        fpURP = (URP_STRUC*)((UINTN)fpURP & 0xFFFFFFFF);
	}

	if (fpURP == NULL) {
        return EFI_OUT_OF_RESOURCES;
	}

	UsbApiHandler(fpURP);

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbHwSmiHandler
//
// Description: USB Hardware SMI handler.
//
// Input:       Host controller type.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
UsbHwSmiHandler (UINT8 HcType)
{
    UINT8       Index;
    HC_STRUC    *HcStruc;

    for (Index = 0; Index < gUsbData->HcTableCount; Index++) {
        HcStruc = gUsbData->HcTable[Index];
        if (HcStruc == NULL) {
            continue;
        }
        if(HcStruc->bHCType == HcType) { // Process appropriate interrupt
            (*gUsbData->aHCDriverTable
				[GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDProcessInterrupt)(HcStruc);
        }
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   xhciHWSMIHandler
//
// Description: USB Hardware SMI handler.
//
// Input:       DispatchHandle  - EFI Handle
//              DispatchContext - Pointer to the EFI_SMM_SW_DISPATCH_CONTEXT
//
// Output:      Nothing
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
UhciHWSMIHandler (
	EFI_HANDLE	DispatchHandle,
	CONST VOID	*Context,
	VOID		*CommBuffer,
	UINTN		*CommBufferSize
)
{
    UsbHwSmiHandler(USB_HC_UHCI);
    return EFI_SUCCESS;
}
EFI_STATUS
OhciHWSMIHandler (
    EFI_HANDLE	DispatchHandle,
    CONST VOID	*Context,
    VOID		*CommBuffer,
    UINTN		*CommBufferSize
)
{
    UsbHwSmiHandler(USB_HC_OHCI);
	return EFI_SUCCESS;
}
EFI_STATUS
EhciHWSMIHandler (
    EFI_HANDLE	DispatchHandle,
    CONST VOID	*Context,
    VOID		*CommBuffer,
    UINTN		*CommBufferSize
)
{
    UsbHwSmiHandler(USB_HC_EHCI);
    return EFI_SUCCESS;
}
EFI_STATUS
XhciHwSmiHandler (
    EFI_HANDLE	DispatchHandle,
    CONST VOID	*Context,
    VOID		*CommBuffer,
    UINTN		*CommBufferSize
)
{
    UsbHwSmiHandler(USB_HC_XHCI);
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBInstallHwSmiHandler
//
// Description:
//  This function registers USB hardware SMI callback function.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
UsbInstallHwSmiHandler(
	HC_STRUC    *HcStruc
)
{
	EFI_STATUS                      Status;
	EFI_SMM_USB_REGISTER_CONTEXT    UsbContext;
    EFI_SMM_USB_DISPATCH2_PROTOCOL  *UsbDispatch;
    EFI_SMM_HANDLER_ENTRY_POINT2	UsbCallback;
    EFI_HANDLE                      Handle = NULL;

    if (HcStruc->HwSmiHandle != NULL) {
        return EFI_SUCCESS;
    }
	

    Status = pSmst->SmmLocateProtocol(
            &gEfiSmmUsbDispatch2ProtocolGuid,
            NULL,
            &UsbDispatch);

    ASSERT_EFI_ERROR(Status);
    
	if (EFI_ERROR(Status)) {
		return Status;
	}

	switch (HcStruc->bHCType) {
		case USB_HC_UHCI:
			UsbCallback = UhciHWSMIHandler;
			break;

		case USB_HC_OHCI:
			UsbCallback = OhciHWSMIHandler;
			break;

		case USB_HC_EHCI:
			UsbCallback = EhciHWSMIHandler;
			break;

		case USB_HC_XHCI:
			UsbCallback = XhciHwSmiHandler;
			break;

		default:
			return EFI_UNSUPPORTED;
	}

    UsbContext.Type = UsbLegacy;
    UsbContext.Device = HcStruc->pHCdp;

    Status = UsbDispatch->Register(
                UsbDispatch,
                UsbCallback,
                &UsbContext,
                &Handle);
    HcStruc->HwSmiHandle = Handle;
    USB_DEBUG(DEBUG_LEVEL_3, "AMIUSB HC type %x HW SMI registation status:: %r\n", HcStruc->bHCType, Status);
	return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        Emul6064TrapCallback
//
// Description:
//  Update the KbcEmul feature when the Emul6064Trap Protocol becomes available.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
Emul6064TrapCallback (
    CONST EFI_GUID  *Protocol,
    VOID            *Interface,
    EFI_HANDLE      Handle
)
{
    EFI_STATUS                      Status;
    UINT32                          KbcEmulFeature = 0;
    
    Status = pSmst->SmmLocateProtocol(
                    &gEmul6064TrapProtocolGuid,
                    NULL,
                    &gEmulationTrap);
    if (!gUsbData->kbc_support) {
        Status = pSmst->SmmLocateProtocol(
                        &gEmul6064MsInputProtocolGuid,
                        NULL,
                        &gMsInput);
        	
        Status = pSmst->SmmLocateProtocol(
                        &gEmul6064KbdInputProtocolGuid,
                        NULL,
                        &gKbdInput);

        if (Status == EFI_SUCCESS) {
            gUsbData->dUSBStateFlag |= USB_FLAG_6064EMULATION_ON;
            if (gEmulationTrap) {
                KbcEmulFeature = gEmulationTrap->FeatureSupported(gEmulationTrap);
            }
            if (KbcEmulFeature & IRQ_SUPPORTED) {
                gUsbData->dUSBStateFlag |= USB_FLAG_6064EMULATION_IRQ_SUPPORT;
            }
        } else {
           InitSysKbc( &gKbdInput, &gMsInput );
        }
    } else {
        //
        //Init Fake Emulation interface
        //
        InitSysKbc( &gKbdInput, &gMsInput );
    }
    
    return EFI_SUCCESS;
}
#endif

//////////////////////////////////////////////////////////////////////////////
//
// USB API Functions
//
//////////////////////////////////////////////////////////////////////////////

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_MassDeviceRequest
//
// Description: This routine services the USB API function number 27h.  It
//      handles all the mass storage related calls from the higher
//      layer. Different sub-functions are invoked depending on
//      the sub-function number
//
// Input:   fpURPPointer    Pointer to the URP structure
//      fpURPPointer.bSubFunc   Subfunction number
//          00  Get Device Information
//          01  Get Device Parameter
//          02  Reset Device
//          03  Read Device
//          04  Write Device
//          05  Verify Device
//          06  Format Device
//          07  Request Sense
//          08  Test Unit Ready
//          09  Start Stop Unit
//          0A  Read Capacity
//          0B  Mode Sense
//          0C  Device Inquiry
//          0D  Send Command
//          0E  Assign drive number
//
// Output:  URP structure is updated with the relevant information
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBAPI_MassDeviceRequest (URP_STRUC *fpURP)
{
    UINT8 bMassFuncIndex = fpURP->bSubFunc;
    UINT8 bNumberOfMassFunctions = sizeof aUsbMassApiTable / sizeof (API_FUNC *);

    //
    // Make sure function number is valid
    //
    if (bMassFuncIndex >= bNumberOfMassFunctions) {
        fpURP->bRetValue = USBAPI_INVALID_FUNCTION;
        return;
    }
    gUsbData->bUSBKBC_MassStorage = 01;
    //
    // Function number is valid - call it
    //
    aUsbMassApiTable[bMassFuncIndex](fpURP);
    gUsbData->bUSBKBC_MassStorage = 00;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_CheckPresence
//
// Description: This routine services the USB API function number 0.  It
//              reports the USB BIOS presence, its version number and
//              its current status information
//
// Input: fpURPPointer - Pointer to the URP structure
//
// Output: URP structure is updated with the following information
//            CkPresence.wBiosRev       USB BIOS revision (0210h means r2.10)
//            CkPresence.bBiosActive    0 - if USB BIOS is not running
//            CkPresence.bNumBootDev    Number of USB boot devices found
//            CkPresence.bNumHC         Number of host controller present
//            CkPresence.bNumPorts      Number of root hub ports
//            CkPresence.dUsbDataArea   Current USB data area
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBAPI_CheckPresence (URP_STRUC *fpURP)
{
    fpURP->bRetValue                        = USB_SUCCESS;
    fpURP->ApiData.CkPresence.bBiosActive   = 0;

    fpURP->ApiData.CkPresence.bNumBootDev   = 0;    // Number of USB boot devices found
    fpURP->ApiData.CkPresence.bNumKeyboards = 0;    // Number of USB keyboards present
    fpURP->ApiData.CkPresence.bNumMice      = 0;    // Number of USB mice present
    fpURP->ApiData.CkPresence.bNumPoint		= 0;    // Number of USB Point present    //(EIP38434+)
    fpURP->ApiData.CkPresence.bNumHubs      = 0;    // Number of USB hubs present
    fpURP->ApiData.CkPresence.bNumStorage   = 0;    // Number of USB storage devices present

    fpURP->ApiData.CkPresence.wBiosRev = (USB_DRIVER_MAJOR_VER << 4) + USB_DRIVER_MINOR_VER;
    fpURP->ApiData.CkPresence.bBiosActive = USB_ACTIVE; // Set USB BIOS as active
    if (!(gUsbData->dUSBStateFlag & USB_FLAG_DISABLE_LEGACY_SUPPORT)) {
        fpURP->ApiData.CkPresence.bBiosActive |= USB_LEGACY_ENABLE;
    }
    if (gUsbData->dUSBStateFlag & USB_FLAG_6064EMULATION_ON) {
        fpURP->ApiData.CkPresence.bBiosActive |= USB_6064_ENABLE;
    }
    USBWrap_GetDeviceCount(fpURP);  // Get active USB devices
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_Start
//
// Description: This API routine configures the USB host controllers and
//      enumerate the devices
//
// Input: fpURPPointer  URP structure with input parameters
//        StartHc.wDataAreaFlag Indicates which data area to use
//
// Output: StartHc.wDataAreaFlag Returns current data area pointer
//         bRetValue - USB_SUCCESS on success, USB_ERROR on error.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID USBAPI_Start (URP_STRUC *fpURP)
{
    USB_DEBUG(DEBUG_LEVEL_3, "USBSMI: Start\n");
    USB_DEBUG(DEBUG_LEVEL_3, "\tUSBAPI_HC_Proc:%x\n", &USBAPI_HC_Proc);
    USB_DEBUG(DEBUG_LEVEL_3, "\tUSBAPI_Core_Proc:%x\n", &USBAPI_Core_Proc);
    USB_DEBUG(DEBUG_LEVEL_3, "\tUSB_ReConfigDevice:%x\n", &USB_ReConfigDevice);
    fpURP->bRetValue = USB_StartHostControllers (gUsbData);
    USB_DEBUG(DEBUG_LEVEL_3, "USB_StartHostControllers returns %d\n", fpURP->bRetValue);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_Stop
//
// Description: This routine stops the USB host controllers
//
// Input:   fpURPPointer    Pointer to the URP structure
//
// Output:  bRetValue   USB_SUCCESS on success
//              USB_ERROR on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID USBAPI_Stop (URP_STRUC *fpURP)
{
    fpURP->bRetValue = USB_StopHostControllers (gUsbData);
    USB_DEBUG(DEBUG_LEVEL_3, "USB_StopHostControllers returns %d\n", fpURP->bRetValue);
    gUsbData->dUSBStateFlag &= ~(USB_FLAG_DRIVER_STARTED);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_PowerManageUSB
//
// Description: This routine suspends the USB host controllers
//
// Input:   fpURPPointer    Pointer to the URP structure
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID USBAPI_PowerManageUSB (URP_STRUC *fpURP)
{
    fpURP->bRetValue = USB_NOT_SUPPORTED;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_PrepareForOS
//
// Description: This routine updates data structures to reflect that
//      POST is completed
//
// Input:   fpURPPointer    Pointer to the URP structure
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID USBAPI_PrepareForOS (URP_STRUC *fpURP)
{
    fpURP->bRetValue = USB_NOT_SUPPORTED;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_SecureInterface
//
// Description: This routine handles the calls related to security device
//
// Input:   fpURPPointer    Pointer to the URP structure
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID USBAPI_SecureInterface (URP_STRUC *fpURP)
{
    fpURP->bRetValue = USB_NOT_SUPPORTED;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_DisableInterrupts
//
// Description: This routine stops the USB host controllers interrupts
//
// Input:   fpURPPointer    Pointer to the URP structure
//
// Output:  bRetValue   USB_SUCCESS on success
//              USB_ERROR on error (Like data area not found)
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID USBAPI_DisableInterrupts (URP_STRUC *fpURP)
{
    fpURP->bRetValue = USB_NOT_SUPPORTED;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_EnableInterrupts
//
// Description: This routine re-enable the USB host controller interrupts
//
// Input:   fpURPPointer    Pointer to the URP structure
//
// Output:  bRetValue   USB_SUCCESS on success
//              USB_ERROR on error (Like data area not found)
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID USBAPI_EnableInterrupts (URP_STRUC *fpURP)
{
    fpURP->bRetValue = USB_NOT_SUPPORTED;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_MoveDataArea
//
// Description: This routine stops the USB host controllers and moves
//      the data area used by host controllers to a new area.
//      The host controller is started from the new place.
//
// Input:   fpURPPointer    URP structure with input parameters
//      StartHc.wDataAreaFlag   Indicates which data area to use
//
// Output:  bRetValue   USB_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID USBAPI_MoveDataArea(URP_STRUC *fpURP)
{
    fpURP->bRetValue = USB_NOT_SUPPORTED;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_GetDeviceInfo
//
// Description: This routine returns the information regarding
//      a USB device like keyboard, mouse, floppy drive etc
//
// Input:   fpURPPointer            URP structure with input parameters
//          GetDevInfo.bDevNumber   Device number (1-based) whose
//                  information is requested
//
// Output:  URP structure is updated with the following information
//      GetDevInfo.bHCNumber - HC number in which the device is found
//      GetDevInfo.bDevType  - Type of the device
//      bRetValue will be one of the following value
//      USB_SUCCESS         on successfull completion
//      USB_PARAMETER_ERROR if bDevNumber is invalid
//      USB_ERROR           on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID USBAPI_GetDeviceInfo (URP_STRUC *fpURP)
{
    DEV_INFO* fpDevInfo;

    //
    // Initialize the return values
    //
    fpURP->ApiData.GetDevInfo.bHCNumber = 0;
    fpURP->ApiData.GetDevInfo.bDevType  = 0;
    fpURP->bRetValue                    = USB_ERROR;

    //
    // Check for parameter validity
    //
    if ( !fpURP->ApiData.GetDevInfo.bDevNumber ) return;

    fpURP->bRetValue = USB_PARAMETER_ERROR;

    //
    // Get the device information structure for the 'n'th device
    //
    fpDevInfo = USBWrap_GetnthDeviceInfoStructure(fpURP->ApiData.GetDevInfo.bDevNumber);
//  if (!wRetCode) return;  // USB_PARAMETER_ERROR

    //
    // Return value
    //
    fpURP->ApiData.GetDevInfo.bDevType  = fpDevInfo->bDeviceType;
    fpURP->ApiData.GetDevInfo.bHCNumber = fpDevInfo->bHCNumber;
    fpURP->bRetValue                    = USB_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_CheckDevicePresence
//
// Description: This routine checks whether a particular type of USB device
//      is installed in the system or not.
//
// Input:   fpURPPointer            URP structure with input parameters
//          ChkDevPrsnc.bDevType    Device type to find
//          ChkDevPrsnc.fpHCStruc   Pointer to HC being checked for device
//            connection; if NULL then the total number of devices
//            connected to ANY controller is returned.
//          ChkDevPrsnc.bNumber     Number of devices connected
//
// Output:  bRetValue will be one of the following value
//          USB_SUCCESS     if device type present, ChkDevPrsnc.bNumber <> 0
//          USB_ERROR       if device type absent, ChkDevPrsnc.bNumber returns 0
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBAPI_CheckDevicePresence (URP_STRUC *fpURP)
{
    UINT8 bSearchFlag;
    UINTN dData;

    bSearchFlag = USB_SRCH_DEV_NUM;
    if (fpURP->bSubFunc == 1)
    {
        bSearchFlag = USB_SRCH_DEVBASECLASS_NUM;
    }
    //
    // The total number of devices connected to ANY controller has been requested
    //
    dData = (UINTN) USB_GetDeviceInfoStruc( bSearchFlag,
            0, fpURP->ApiData.ChkDevPrsnc.bDevType, fpURP->ApiData.ChkDevPrsnc.fpHCStruc);

    fpURP->ApiData.ChkDevPrsnc.bNumber = (UINT8)dData;
    fpURP->bRetValue = (UINT8)((fpURP->ApiData.ChkDevPrsnc.bNumber)?
                                            USB_SUCCESS : USB_ERROR);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassAPIGetDeviceInformation
//
// Description: This function is part of the USB BIOS MASS API. This function
//      returns the device information of the mass storage device
//
// Input:   fpURPPointer    Pointer to the URP structure
//          bDevAddr    USB device address of the device
//
// Output:  bRetValue   Return value
//      fpURPPointer    Pointer to the URP structure
//          dSenseData  Sense data of the last command
//          bDevType    Device type byte (HDD, CD, Removable)
//          bEmuType    Emulation type used
//          fpDevId     Far pointer to the device ID
//          dInt13Entry INT 13h entry point
//
// Notes:   Initially the bDevAddr should be set to 0 as input. This
//      function returns the information regarding the first mass
//      storage device (if no device found it returns bDevAddr as
//      0FFh) and also updates bDevAddr to the device address of
//      the current mass storage device. If no other mass storage
//      device is found then the routine sets the bit7 to 1
//      indicating current information is valid but no more mass
//      device found in the system. The caller can get the next
//      device info if bDevAddr is not 0FFh and bit7 is not set
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassAPIGetDeviceInformation (URP_STRUC *fpURP)
{
    fpURP->bRetValue = USBMassGetDeviceInfo (&fpURP->ApiData.MassGetDevInfo);
}

VOID
USBMassAPIGetDeviceParameters (URP_STRUC *fpURP)
{
    fpURP->ApiData.MassGetDevParms.fpInqData =
        USBMassGetDeviceParameters (fpURP->ApiData.MassGetDevParms.fpDevInfo);
    fpURP->bRetValue = (fpURP->ApiData.MassGetDevParms.fpInqData == NULL)? USB_ERROR : USB_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USBMassAPIGetDevStatus
//
// Description: This function returns the drive status and media presence
//      status
//
// Input:       fpURPPointer    Pointer to the URP structure
//              fpURP->ApiData.fpDevInfo - pointer to USB device that is
//              requested to be checked
//
// Output:  Return code USB_ERROR - Failure, USB_SUCCESS - Success
//
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>

VOID
USBMassAPIGetDevStatus(URP_STRUC *fpURP)
{
#if USB_DEV_MASS
    fpURP->bRetValue    = USBMassGetDeviceStatus (&fpURP->ApiData.MassGetDevSts);
//    USB_DEBUG(DEBUG_LEVEL_3, "USBMassAPIGetDevStatus ... check function call correct?\n");
#endif
}



//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassAPIGetDeviceGeometry
//
// Description: This function is part of the USB BIOS MASS API.
//
// Input:   fpURPPointer    Pointer to the URP structure
//          bDevAddr    USB device address of the device
//
// Output:  bRetValue   Return value
//      fpURPPointer    Pointer to the URP structure
//          dSenseData  Sense data of the last command
//          bNumHeads   Number of heads
//          wNumCylinders   Number of cylinders
//          bNumSectors Number of sectors
//          wBytesPerSector Number of bytes per sector
//          bMediaType  Media type
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassAPIGetDeviceGeometry(URP_STRUC *fpURP)
{
    fpURP->bRetValue = USBMassGetDeviceGeometry (&fpURP->ApiData.MassGetDevGeo);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassAPIResetDevice
//
// Description: This function is part of the USB BIOS MASS API.
//
// Input:   fpURPPointer    Pointer to the URP structure
//
// Output:  bRetValue   Return value
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassAPIResetDevice (URP_STRUC *fpURP)
{
    UINT8       bDevAddr;
    DEV_INFO    *fpDevInfo;
    UINT16      wResult;

    bDevAddr = fpURP->ApiData.MassReset.bDevAddr;

    //
    // Get the device info structure for the matching device address
    //
    fpDevInfo   = USB_GetDeviceInfoStruc(USB_SRCH_DEV_INDX, 0, bDevAddr, 0);
    if((fpDevInfo == NULL)|| (!(fpDevInfo->bFlag & DEV_INFO_DEV_PRESENT))) {
		fpURP->bRetValue = USB_ATA_TIME_OUT_ERR;
        return;
    }
    //
    // Send Start/Stop Unit command to UFI class device only
    //
    fpURP->bRetValue    = USB_SUCCESS;
    if(fpDevInfo->bSubClass ==  SUB_CLASS_UFI) {
        wResult = USBMassStartUnitCommand (fpDevInfo);
        if (wResult) {
            fpURP->bRetValue  = USBWrapGetATAErrorCode(fpURP->ApiData.MassReset.dSenseData);
        }
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassAPIReadDevice
//
// Description: This function is part of the USB BIOS MASS API.
//
// Input:   fpURPPointer    Pointer to the URP structure, it contains the following:
//                  bDevAddr      USB device address of the device
//                  dStartLBA     Starting LBA address
//                  wNumBlks      Number of blocks to read
//                  wPreSkipSize  Number of bytes to skip before
//                  wPostSkipSize Number of bytes to skip after
//                  fpBufferPtr   Far buffer pointer
//
// Output: fpURPPointer Pointer to the URP structure
//                  bRetValue    Return value
//                  dSenseData   Sense data of the last command
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassAPIReadDevice (URP_STRUC *fpURP)
{
    DEV_INFO    *fpDevInfo;
    UINT8       bDevAddr;
    UINT16      wResult;
											//(EIP15037+)>
    UINT32		dData = 0;
											//<(EIP15037+)
    bDevAddr = fpURP->ApiData.MassRead.bDevAddr;

    if (((bDevAddr == USB_HOTPLUG_FDD_ADDRESS) &&
            ((gUsbData->dUSBStateFlag & USB_HOTPLUG_FDD_ENABLED) == FALSE)) ||
        ((bDevAddr == USB_HOTPLUG_HDD_ADDRESS) &&
            ((gUsbData->dUSBStateFlag & USB_HOTPLUG_HDD_ENABLED) == FALSE)) ||
        ((bDevAddr == USB_HOTPLUG_CDROM_ADDRESS) &&
            ((gUsbData->dUSBStateFlag & USB_HOTPLUG_CDROM_ENABLED) == FALSE)))
    {
        fpURP->bRetValue = USB_ATA_DRIVE_NOT_READY_ERR;
        return;
    }

    //
    // Get the device info structure for the matching device address
    //
    fpDevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_INDX, 0, bDevAddr, 0);
    if((fpDevInfo == NULL)|| (!(fpDevInfo->bFlag & DEV_INFO_DEV_PRESENT))) {
		fpURP->bRetValue = USB_ATA_TIME_OUT_ERR;
        return;
    }

											//(EIP15037+)>
#if EXTRA_CHECK_DEVICE_READY
    //
    // Check device ready
    //
    dData = USBMassCheckDeviceReady(fpDevInfo);
	if (dData) {
        fpURP->ApiData.MassRead.dSenseData = dData;
        fpURP->bRetValue = USBWrapGetATAErrorCode(fpURP->ApiData.MassRead.dSenseData);	//(EIP31535)
		return;
	}
#endif
											//<(EIP15037+)

    //
    // Issue read command
    //
    wResult = USBMassRWVCommand(fpDevInfo, COMMON_READ_10_OPCODE, &fpURP->ApiData.MassRead);
//USB_DEBUG(DEBUG_LEVEL_3, " wr(%x):%x %x", bDevAddr, wResult, fpURP->ApiData.MassRead.dSenseData);
    if (wResult) {
        fpURP->bRetValue    = USB_SUCCESS;
        return;
    }
    fpURP->bRetValue = USBWrapGetATAErrorCode(fpURP->ApiData.MassRead.dSenseData);
//USB_DEBUG(DEBUG_LEVEL_3, " er(%x):%x", bDevAddr, fpURP->bRetValue);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassAPIWriteDevice
//
// Description: This function is part of the USB BIOS MASS API.
//
// Input:   fpURPPointer    Pointer to the URP structure
//          bDevAddr    USB device address of the device
//          dStartLBA   Starting LBA address
//          wNumBlks    Number of blocks to write
//          wPreSkipSize    Number of bytes to skip before
//          wPostSkipSize   Number of bytes to skip after
//          fpBufferPtr Far buffer pointer
//
// Output:  fpURPPointer    Pointer to the URP structure
//          bRetValue   Return value
//          dSenseData  Sense data of the last command
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassAPIWriteDevice(URP_STRUC *fpURP)
{
    DEV_INFO    *fpDevInfo;
    UINT8       bDevAddr;
    UINT16      wResult;
											//(EIP15037+)>
    UINT32		dData = 0;
											//<(EIP15037+)

    bDevAddr = fpURP->ApiData.MassWrite.bDevAddr;

    if (((bDevAddr == USB_HOTPLUG_FDD_ADDRESS) &&
            ((gUsbData->dUSBStateFlag & USB_HOTPLUG_FDD_ENABLED) == FALSE)) ||
        ((bDevAddr == USB_HOTPLUG_HDD_ADDRESS) &&
            ((gUsbData->dUSBStateFlag & USB_HOTPLUG_HDD_ENABLED) == FALSE)) ||
        ((bDevAddr == USB_HOTPLUG_CDROM_ADDRESS) &&
            ((gUsbData->dUSBStateFlag & USB_HOTPLUG_CDROM_ENABLED) == FALSE)))
    {
        fpURP->bRetValue = USB_ATA_DRIVE_NOT_READY_ERR;
        return;
    }

    //
    // Get the device info structure for the matching device address
    //
    fpDevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_INDX, 0, bDevAddr, 0);
    if((fpDevInfo == NULL)|| (!(fpDevInfo->bFlag & DEV_INFO_DEV_PRESENT))) {
		fpURP->bRetValue = USB_ATA_TIME_OUT_ERR;
        return;
    }
/*
    if (!(fpDevInfo->bLastStatus & USB_MASS_MEDIA_PRESENT)) {
        fpURP->bRetValue = USB_ATA_NO_MEDIA_ERR;
        return;
    }
*/
											//(EIP15037+)>
#if EXTRA_CHECK_DEVICE_READY
    //
    // Check device ready
    //
    dData = USBMassCheckDeviceReady(fpDevInfo);
	if (dData) {
        fpURP->ApiData.MassRead.dSenseData = dData;
        fpURP->bRetValue = USBWrapGetATAErrorCode(fpURP->ApiData.MassWrite.dSenseData);	//(EIP31535)
		return;
	}
#endif
											//<(EIP15037+)

    //
    // Issue write command
    //
    wResult = USBMassRWVCommand(fpDevInfo, COMMON_WRITE_10_OPCODE, &fpURP->ApiData.MassWrite);
    if (wResult) {
        fpURP->bRetValue = USB_SUCCESS;
        return;
    }

    fpURP->bRetValue = USBWrapGetATAErrorCode(fpURP->ApiData.MassWrite.dSenseData);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassAPIVerifyDevice
//
// Description: This function is part of the USB BIOS MASS API.
//
// Input:   fpURPPointer    Pointer to the URP structure
//          bDevAddr    USB device address of the device
//          dStartLBA   Starting LBA address
//          wNumBlks    Number of blocks to write
//          wPreSkipSize    Number of bytes to skip before
//          wPostSkipSize   Number of bytes to skip after
//          fpBufferPtr Far buffer pointer
//
// Output:  fpURPPointer    Pointer to the URP structure
//          bRetValue   Return value
//          dSenseData  Sense data of the last command
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassAPIVerifyDevice(URP_STRUC *fpURP)
{
    DEV_INFO    *fpDevInfo;
    UINT8       bDevAddr;
    UINT16      wResult;
											//(EIP15037+)>
    UINT32		dData = 0;
											//<(EIP15037+)

    bDevAddr = fpURP->ApiData.MassVerify.bDevAddr;

    if (((bDevAddr == USB_HOTPLUG_FDD_ADDRESS) &&
            ((gUsbData->dUSBStateFlag & USB_HOTPLUG_FDD_ENABLED) == FALSE)) ||
        ((bDevAddr == USB_HOTPLUG_HDD_ADDRESS) &&
            ((gUsbData->dUSBStateFlag & USB_HOTPLUG_HDD_ENABLED) == FALSE)) ||
        ((bDevAddr == USB_HOTPLUG_CDROM_ADDRESS) &&
            ((gUsbData->dUSBStateFlag & USB_HOTPLUG_CDROM_ENABLED) == FALSE)))
    {
        fpURP->bRetValue = USB_ATA_DRIVE_NOT_READY_ERR;
        return;
    }

    //
    // Get the device info structure for the matching device address
    //
    fpDevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_INDX, 0, bDevAddr, 0);
    if((fpDevInfo == NULL)|| (!(fpDevInfo->bFlag & DEV_INFO_DEV_PRESENT))) {
		fpURP->bRetValue = USB_ATA_TIME_OUT_ERR;
        return;
    }
											//(EIP15037+)>
#if EXTRA_CHECK_DEVICE_READY
    //
    // Check device ready
    //
    dData = USBMassCheckDeviceReady(fpDevInfo);
	if (dData) {
        fpURP->ApiData.MassRead.dSenseData = dData;
        fpURP->bRetValue = USBWrapGetATAErrorCode(fpURP->ApiData.MassVerify.dSenseData);	//(EIP31535)
		return;
	}
#endif
											//<(EIP15037+)

    //
    // Issue write command
    //
    wResult = USBMassRWVCommand( fpDevInfo, COMMON_VERIFY_OPCODE, &fpURP->ApiData.MassVerify );
    if (wResult) {
        fpURP->bRetValue    = USB_SUCCESS;
        return;
    }

    fpURP->bRetValue  = USBWrapGetATAErrorCode(fpURP->ApiData.MassVerify.dSenseData);

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassAPIFormatDevice
//
// Description: This function is part of the USB BIOS MASS API.
//
// Input:   fpURPPointer    Pointer to the URP structure
//
// Output:  bRetValue   Return value
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassAPIFormatDevice(URP_STRUC *fpURP)
{
    fpURP->bRetValue = USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassAPICommandPassThru
//
// Description: This function is part of the USB BIOS MASS API. This
//      function can be used to pass raw command/data sequence to
//      the USB mass storage device
//
// Input:   fpURPPointer    Pointer to the URP structure
//
// Output:  bRetValue   Return value
//
// Modified:    Nothing
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassAPICommandPassThru (URP_STRUC *fpURP)
{
    UINT8 bResult = USBMassCmdPassThru(&fpURP->ApiData.MassCmdPassThru);
    fpURP->bRetValue = bResult;
}


//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USBMassAPIAssignDriveNumber
//
// Description: This function is part of the USB BIOS MASS API. This function
//      assigns the logical drive device according to the information of the
//      mass storage device
//
// Input:   fpURPPointer    Pointer to the URP structure
//      bDevAddr    USB device address of the device
//      bLogDevNum  Logical Drive Number to assign to the device
//
// Output:  Return code USB_ERROR - Failure, USB_SUCCESS - Success
//
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>

VOID
USBMassAPIAssignDriveNumber (URP_STRUC *fpURP)
{
    fpURP->bRetValue = USB_SUCCESS; // No errors expected after this point
}


//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USBMassAPICheckDevStatus
//
// Description: This function is part of the USB BIOS MASS API. This function
//              invokes USB Mass Storage API handler to check whether device
//              is ready. If called for the first time, this function retrieves
//              the mass storage device geometry and fills the corresponding
//              fpDevInfo fields.
//
// Input:       fpURPPointer    Pointer to the URP structure
//              fpURP->ApiData.fpDevInfo - pointer to USB device that is
//              requested to be checked
//
// Output:  Return code USB_ERROR - Failure, USB_SUCCESS - Success
//
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>

VOID
USBMassAPICheckDevStatus(URP_STRUC *fpURP)
{
#if USB_DEV_MASS
    UINT32  dResult;
    dResult = USBMassCheckDeviceReady (fpURP->ApiData.MassChkDevReady.fpDevInfo);
    fpURP->bRetValue = (UINT8)dResult;
#endif
}


//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USBAPI_LightenKeyboardLEDs
//
// Description: This function is part of the USB BIOS API. This function
//              controls LED state on the connected USB keyboards
//
// Input:       fpURP   Pointer to the URP structure
//
// Output:  Return code USB_ERROR - Failure, USB_SUCCESS - Success
//
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>

VOID
USBAPI_LightenKeyboardLEDs(URP_STRUC *fpURP)
{
#if USB_DEV_KBD
	if (fpURP->ApiData.KbLedsData.LedMapPtr) {
		gUsbData->bUSBKBShiftKeyStatus = 0;
		if(((LED_MAP*)fpURP->ApiData.KbLedsData.LedMapPtr)->NumLock) {
			gUsbData->bUSBKBShiftKeyStatus |= KB_NUM_LOCK_BIT_MASK;
		}
		if(((LED_MAP*)fpURP->ApiData.KbLedsData.LedMapPtr)->CapsLock) {
			gUsbData->bUSBKBShiftKeyStatus |= KB_CAPS_LOCK_BIT_MASK;
		}
		if(((LED_MAP*)fpURP->ApiData.KbLedsData.LedMapPtr)->ScrLock) {
			gUsbData->bUSBKBShiftKeyStatus |= KB_SCROLL_LOCK_BIT_MASK;
		}
	}

	if (fpURP->ApiData.KbLedsData.DevInfoPtr) {
		UsbKbdSetLed((DEV_INFO*)fpURP->ApiData.KbLedsData.DevInfoPtr, 
						((gUsbData->bUSBKBShiftKeyStatus) >> 4) & 0x07);
	}

	fpURP->bRetValue	= USB_SUCCESS;
	return;
#else
	fpURP->bRetValue	= USB_NOT_SUPPORTED;
#endif
}

VOID
USBAPI_LightenKeyboardLEDs_Compatible(URP_STRUC *fpURP)
{
#if USB_DEV_KBD
	if (fpURP->ApiData.KbLedsData.LedMapPtr) {
		gUsbData->bUSBKBShiftKeyStatus = 0;
		if(((LED_MAP*)fpURP->ApiData.KbLedsData.LedMapPtr)->NumLock) {
			gUsbData->bUSBKBShiftKeyStatus |= KB_NUM_LOCK_BIT_MASK;
		}
		if(((LED_MAP*)fpURP->ApiData.KbLedsData.LedMapPtr)->CapsLock) {
			gUsbData->bUSBKBShiftKeyStatus |= KB_CAPS_LOCK_BIT_MASK;
		}
		if(((LED_MAP*)fpURP->ApiData.KbLedsData.LedMapPtr)->ScrLock) {
			gUsbData->bUSBKBShiftKeyStatus |= KB_SCROLL_LOCK_BIT_MASK;
		}
	}

	//USB_DEBUG(DEBUG_LEVEL_3," LEDs: %d\n", gUsbData->bUSBKBShiftKeyStatus);
	USBKB_LEDOn();

	fpURP->bRetValue	= USB_SUCCESS;
	return;
#else
	fpURP->bRetValue	= USB_NOT_SUPPORTED;
#endif
}

					                        //(EIP29733+)>
//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USBAPI_KbcAccessControl
//
// Description: This function is part of the USB BIOS API. This function
//              is used to control whether KBC access in USB module 
//              should be blocked or not.
//
// Input:       fpURP   Pointer to the URP structure
//
// Output:      Return code USB_ERROR - Failure, USB_SUCCESS - Success
//
// Notes:
//
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>


VOID
UsbKbcAccessControl(UINT8 ControlSwitch)
{
    IsKbcAccessBlocked = (ControlSwitch != 0)? TRUE : FALSE;

    //
    // Check if the USB access in Legacy mode. If it's legacy mode enable/disable
    // the Kbcemulation based on the ControlSwitch  
    //
    if(!(gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI)) {

        if(IsKbcAccessBlocked) {
            if(gEmulationTrap) { 
                //
                // Keyboard access blocked. Disable the Emulation
                //
                gEmulationTrap->TrapDisable(gEmulationTrap);
            }
        } else {
            if(gEmulationTrap) { 
                //
                // Keyboard access enabled. Enable the KbcEmulation
                //
                gEmulationTrap->TrapEnable(gEmulationTrap);
            }
        }
    }
					//(EIP48323+)>
    //Reflush USB data buffer if intend to disable usb keyboard data throughput.
    if(IsKbcAccessBlocked) {
        USBKeyRepeat(NULL, 1);  // Disable Key repeat
        gUsbData->RepeatKey = 0;

		// Clear Legacy USB keyboard buffer
		MemSet(gUsbData->aKBCCharacterBufferStart, sizeof(gUsbData->aKBCCharacterBufferStart), 0);
		gUsbData->fpKBCCharacterBufferHead = gUsbData->aKBCCharacterBufferStart;
		gUsbData->fpKBCCharacterBufferTail = gUsbData->aKBCCharacterBufferStart;
		
		MemSet(gUsbData->aKBCScanCodeBufferStart, sizeof(gUsbData->aKBCScanCodeBufferStart), 0);
		gUsbData->fpKBCScanCodeBufferPtr = gUsbData->aKBCScanCodeBufferStart;
											
		MemSet(gUsbData->aKBCDeviceIDBufferStart, sizeof(gUsbData->aKBCDeviceIDBufferStart), 0);
		MemSet(gUsbData->aKBCShiftKeyStatusBufferStart, sizeof(gUsbData->aKBCShiftKeyStatusBufferStart), 0);
		
		MemSet(gUsbData->aKBInputBuffer, sizeof(gUsbData->aKBInputBuffer), 0);
    }
					//<(EIP48323+)
}


VOID
USBAPI_KbcAccessControl(URP_STRUC *fpURP)
{
    UsbKbcAccessControl(fpURP->ApiData.KbcControlCode);
}
					                        //<(EIP29733+)
//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USB_StopLegacy
//
// Description: This function is part of the USB BIOS API. This function init USB 
//             legacy support.
//
// Input:       fpURP   Pointer to the URP structure
//
// Output:      None
//
// Notes:
//
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>
VOID
USB_StopLegacy(URP_STRUC *fpURP)
{
    //shutdown device first
    UINT8       bIndex;
    DEV_INFO    *fpDevInfo;
    HC_STRUC    *fpHCStruc;
    
    for (bIndex = 1; bIndex < MAX_DEVICES; bIndex ++){
        fpDevInfo = gUsbData->aDevInfoTable +bIndex;
        if ((fpDevInfo->bFlag & 
            (DEV_INFO_VALID_STRUC |DEV_INFO_DEV_PRESENT)    ) ==   
            (DEV_INFO_VALID_STRUC |DEV_INFO_DEV_PRESENT)    ){
            //
            fpHCStruc = gUsbData->HcTable[fpDevInfo->bHCNumber - 1];
            //
            USB_StopDevice (fpHCStruc, fpDevInfo->bHubDeviceNumber, fpDevInfo->bHubPortNumber);
        }
    }

    StopControllerType(USB_HC_XHCI);    //(EIP57521+)
    StopControllerType(USB_HC_EHCI);
    StopControllerType(USB_HC_UHCI);
    StopControllerType(USB_HC_OHCI);
    
    //return as success
    fpURP->bRetValue    = USB_SUCCESS;
    
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USB_StartLegacy
//
// Description: This function is part of the USB BIOS API. This function init USB 
//             legacy support.
//
// Input:       fpURP   Pointer to the URP structure
//
// Output:      None
//
// Notes:
//
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>
VOID
USB_StartLegacy(URP_STRUC *fpURP)
{
                                        //(EIP57521)>
    gUsbData->bHandOverInProgress = FALSE;
    //Start XHCI
    StartControllerType(USB_HC_XHCI);
    USB_EnumerateRootHubPorts(USB_HC_XHCI);
    //Start EHCI
    StartControllerType(USB_HC_EHCI);
    USB_EnumerateRootHubPorts(USB_HC_EHCI);
    //Start UHCI
    StartControllerType(USB_HC_UHCI);
    USB_EnumerateRootHubPorts(USB_HC_UHCI);
    //Start OHCI
    StartControllerType(USB_HC_OHCI);
    USB_EnumerateRootHubPorts(USB_HC_OHCI);
                                        //<(EIP57521)
    //return as success
  fpURP->bRetValue    = USB_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USBAPI_LegacyControl
//
// Description: This function is part of the USB BIOS API. This function
//              is used to shutdown/init USB legacy support.
//
// Input:       fpURP   Pointer to the URP structure
//
// Output:      None
//
// Notes:
//
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>
VOID USBAPI_LegacyControl (URP_STRUC *fpURP)
{
    UINT8 bSubLegacyFunc = fpURP->bSubFunc,i;       //(EIP102150+)
    UINT8 Count = (UINT8)(gUsbData->fpKBCScanCodeBufferPtr - 
                  (UINT8*)gUsbData->aKBCScanCodeBufferStart);   //(EIP102150+) 

    USB_DEBUG(DEBUG_LEVEL_3, "USBAPI_LegacyControl %d\n", fpURP->bSubFunc);
    if(bSubLegacyFunc==STOP_USB_CONTROLLER){ 		//(EIP43475+)>  	
        USB_StopLegacy (fpURP);
                                        //(EIP102150+)>
    for(i = Count; i > 0; i--)
        USBKB_DiscardCharacter(&gUsbData->aKBCShiftKeyStatusBufferStart[i-1]); 
                                        //<(EIP102150+)
        if(gEmulationTrap) 
            gEmulationTrap->TrapDisable(gEmulationTrap);
    }

    if(bSubLegacyFunc==START_USB_CONTROLLER){
        USB_StartLegacy (fpURP);
        if(gEmulationTrap)
            gEmulationTrap->TrapEnable(gEmulationTrap);
    }												//<(EIP43475+)
    USB_DEBUG(DEBUG_LEVEL_3, "Result %d\n", fpURP->bRetValue);
}
										//(EIP74876+)>
//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USBAPI_UsbStopController
//
// Description: This function is part of the USB BIOS API. This function stops 
//              the USB host controller.
//
// Input:       fpURP   Pointer to the URP structure
//
// Output:      None
//
// Notes:
//
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>
VOID USBAPI_UsbStopController (URP_STRUC *fpURP)
{
	StopControllerBdf(fpURP->ApiData.HcBusDevFuncNum);
}
										//<(EIP74876+)
//-----------------------------------------------------
//
//-----------------------------------------------------
EFI_STATUS USBRT_LegacyControl (VOID *fpURP)
{
  //
  USBAPI_LegacyControl ((URP_STRUC *)fpURP);
  //
  return((EFI_STATUS)(((URP_STRUC *)fpURP)->bRetValue));
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_GetDeviceAddress
//
// Description: 
//
// Input:  
//
// Output: 
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBAPI_GetDeviceAddress(
    URP_STRUC *Urp
)
{
	UINT8		i;
	DEV_INFO	*DevInfo = NULL;

	for (i = 1; i < MAX_DEVICES; i++) {
		if (!(gUsbData->aDevInfoTable[i].bFlag & DEV_INFO_VALID_STRUC)) {
			continue;
		}
		if ((gUsbData->aDevInfoTable[i].wVendorId == Urp->ApiData.GetDevAddr.Vid) && 
				(gUsbData->aDevInfoTable[i].wDeviceId == Urp->ApiData.GetDevAddr.Did)) {
			DevInfo = &gUsbData->aDevInfoTable[i];
		}
	}
	if (DevInfo == NULL) {
		Urp->bRetValue = USB_ERROR;
		return;
	}

	Urp->ApiData.GetDevAddr.DevAddr = DevInfo->bDeviceAddress;
	Urp->bRetValue = USB_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBAPI_ExtDriverRequest
//
// Description: 
//
// Input:  
//
// Output: 
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBAPI_ExtDriverRequest (
    URP_STRUC *Urp
)
{
    DEV_INFO	*DevInfo = NULL;

	DevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_ADDR, 0, Urp->ApiData.DevAddr, 0);
	if (DevInfo == NULL) {
		Urp->bRetValue = USB_ERROR;
		return;
	}

    DevInfo->fpDeviceDriver->pfnDriverRequest(DevInfo, Urp);
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USB_StopUnsupportedHc
//
// Description: This routine is called, from host controllers that supports
//				OS handover functionality, when OS wants the BIOS to hand-over 
//				the host controllers to the OS.  This routine will stop HC that 
//				does not support this functionality.
//
// Input:       None
//
// Output:      None
//
// Notes:
//
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>
VOID
USB_StopUnsupportedHc()
{
#if USB_RUNTIME_DRIVER_IN_SMM
	USBSB_UninstallTimerHandlers();
#endif

	if(gUsbData->UsbXhciHandoff) {
		StopControllerType(USB_HC_XHCI);
	}
	if(gUsbData->UsbEhciHandoff) {
		gUsbData->bHandOverInProgress = TRUE;
		StopControllerType(USB_HC_EHCI);
	}
	if(gUsbData->UsbOhciHandoff) {
		StopControllerType(USB_HC_OHCI);
	}
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USBAPI_ChangeOwner
//
// Description: This function is part of the USB BIOS API. This function
//              updates the global variables according to the new owner
//
// Input:       fpURP   Pointer to the URP structure
//
// Output:      Return code USB_ERROR - Failure, USB_SUCCESS - Success
//
// Notes:       It is a caller responsibility to release the keyboard only if it
//              was previously acquired.
//
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>

VOID
USBAPI_ChangeOwner(URP_STRUC *fpURP)
{
//USB_DEBUG(DEBUG_LEVEL_3, "USBAPI_ChangeOwner..");

    if(fpURP->ApiData.Owner) {  // Changing to Efi driver
//USB_DEBUG(DEBUG_LEVEL_3, "fpURP->ApiData.Owner=%d\n", fpURP->ApiData.Owner);
        if(gEmulationTrap) {
            gEmulationTrap->TrapDisable(gEmulationTrap);
        }
        gUsbData->dUSBStateFlag |= USB_FLAG_RUNNING_UNDER_EFI;
    } else {    // Acquiring - check the current condition first
//USB_DEBUG(DEBUG_LEVEL_3, "fpURP->ApiData.Owner=%d...", fpURP->ApiData.Owner);
        if(gEmulationTrap) {
            gEmulationTrap->TrapEnable(gEmulationTrap);
        }

        if (gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI) {
//USB_DEBUG(DEBUG_LEVEL_3, "USB_FLAG_RUNNING_UNDER_EFI\n");
            gUsbData->dUSBStateFlag &= ~USB_FLAG_RUNNING_UNDER_EFI;
        } else {
//USB_DEBUG(DEBUG_LEVEL_3, "not USB_FLAG_RUNNING_UNDER_EFI\n");
        }
    }
    fpURP->bRetValue = USB_SUCCESS;
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USBAPI_HcStartStop
//
// Description: This function is part of the USB BIOS API. This function 
//              starts/stops the USB host controller.
//
// Input:       fpURP   Pointer to the URP structure
//
// Output:      None
//
// Notes:
//
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>

VOID
USBAPI_HcStartStop(URP_STRUC *Urp)
{
	 if(Urp->ApiData.HcStartStop.Start) {
	 	Urp->bRetValue = UsbHcStart(Urp->ApiData.HcStartStop.HcStruc);
	 } else {
	 	Urp->bRetValue = UsbHcStop(Urp->ApiData.HcStartStop.HcStruc);
	 }
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USBAPI_invoke_in_frame
//
// Description: Invokes procedure passing parameters supplied in the buffer
//      It replicates the stack frame so that target procedure can
//      see the parameters passed to the stub.
//
// Output:  Returns result of invoked proc
//
//------------------------------------------------------------------------------
//<AMI_PHDR_END>

//
// The following typedef corresponds to the min width type that can be passed
// into function call as a parameter without padding
//
typedef UINTN STACKWORD;

UINTN 
USBAPI_invoke_in_frame(
    VOID* pProc,
    VOID* buffer,
    UINT32 paramSize )
{
    STACKWORD* params = (STACKWORD*)buffer;

    switch(paramSize/sizeof(STACKWORD)){
    case 0: return ((STACKWORD (*)())pProc)();
    case 1: return ((STACKWORD (*)(STACKWORD))pProc)(params[0]);
    case 2: return ((STACKWORD (*)(STACKWORD,STACKWORD))pProc)(params[0],
                params[1]);
    case 3: return ((STACKWORD (*)(STACKWORD,STACKWORD,STACKWORD))pProc)(
                params[0],params[1],params[2]);
    case 4: return ((STACKWORD (*)(STACKWORD,STACKWORD,STACKWORD,
                STACKWORD))pProc)(
                params[0],params[1],params[2],params[3]);
    case 5: return ((STACKWORD (*)(STACKWORD,STACKWORD,STACKWORD,STACKWORD,
                STACKWORD))pProc)(
                params[0],params[1],params[2],params[3],params[4]);
    case 6: return ((STACKWORD (*)(STACKWORD,STACKWORD,STACKWORD,STACKWORD,
                STACKWORD,STACKWORD))pProc)(
                params[0],params[1],params[2],params[3],params[4],params[5]);
    case 7: return ((STACKWORD (*)(STACKWORD,STACKWORD,STACKWORD,STACKWORD,
                STACKWORD,STACKWORD,STACKWORD))pProc)(
                params[0],params[1],params[2],params[3],params[4],params[5],
                params[6]);
    default:
        ASSERT(paramSize/sizeof(STACKWORD) < 4);
        return 0;
    }
/*  kept for reference
    __asm {
        push    ecx
        push    esi
        pushf
                        //Copy stak frame
        std
        mov     esi, buffer
        mov     ecx, paramSize
        add     esi, ecx
        sub     esi, 4
        shr     ecx, 2
loop1:
        lodsd   //DWORD PTR ds:edi
        push    eax
        loop    loop1
                        //Call proc
        mov     eax, pProc
        cld
        call    eax
                        //Read return value
        mov     retVal, eax

                        //Restore stack and registers
        add     esp, paramSize
        popf
        pop     esi
        pop     ecx
    }
    return retVal;*/
}

//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USBAPI_HC_Proc
//
// Description: Bridge to a number of procedures supplied by HC driver
//
//
// Input:       fpURP   Pointer to the URP structure
//
// Output:  Return code USB_ERROR - Failure, USB_SUCCESS - Success)
//
// Notes:
//      Assumes that buffer has a correct image of the stack that
//      corresponding function reads argument from
//      Size of the buffer can be biger than actually used.
//
//      Following code copies the buffer (some stack frame) into new
//      stack frame such that invoked dirver proc can read parametes
//      supplied by buffer
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>

VOID USBAPI_HC_Proc(URP_STRUC *fpURP)
{
    VOID* buffer = fpURP->ApiData.HcProc.paramBuffer;
    UINT32 paramSize = // align size on DWORD
        (fpURP->ApiData.HcProc.paramSize + 3) & ~0x3;
    UN_HCD_HEADER* pHdr;

    ASSERT( GET_HCD_INDEX(fpURP->ApiData.HcProc.bHCType) <
        sizeof(gUsbData->aHCDriverTable)/sizeof(HCD_HEADER));
    ASSERT( fpURP->bSubFunc < sizeof(pHdr->asArray.proc)/sizeof(VOID*));

    pHdr = (UN_HCD_HEADER*)(gUsbData->aHCDriverTable +
                GET_HCD_INDEX(fpURP->ApiData.HcProc.bHCType));
    fpURP->ApiData.HcProc.retVal = USBAPI_invoke_in_frame(
        pHdr->asArray.proc[fpURP->bSubFunc],buffer,paramSize);
    fpURP->bRetValue = USB_SUCCESS;
}


//<AMI_PHDR_START>
//-----------------------------------------------------------------------------
// Procedure:   USBAPI_Core_Proc
//
// Description: Bridge to a number of procedures supplied by Core proc table
//
// Input:       fpURP   Pointer to the URP structure
//
// Output:  Return code USB_ERROR - Failure, USB_SUCCESS - Success
//
// Notes:
//      Assumes that buffer has a correct image of the stack that
//      corresponding function reads argument from
//      Size of the buffer can be biger than actually used.
//
//      Following code copies the buffer (some stack frame) into new
//      stack frame such that invoked proc can read parametes
//      supplied by buffer
//------------------------------------------------------------------------------;
//<AMI_PHDR_END>


VOID* core_proc_table[] = {
        USB_GetDescriptor,
        USB_ReConfigDevice,
        USB_ReConfigDevice2,
        UsbAllocDevInfo,
        prepareForLegacyOS,
        USB_ResetAndReconfigDev,
        USB_DevDriverDisconnect,
//        USB_GetDataPtr,
//        MemCopy,
    };

VOID USBAPI_Core_Proc(URP_STRUC *fpURP)
{
    VOID* buffer = fpURP->ApiData.CoreProc.paramBuffer;
    UINT32 paramSize = // align size on DWORD
        (fpURP->ApiData.CoreProc.paramSize + 3) & ~0x3;

    ASSERT( fpURP->bSubFunc < COUNTOF(core_proc_table));

    fpURP->ApiData.CoreProc.retVal = USBAPI_invoke_in_frame(
        core_proc_table[fpURP->bSubFunc],buffer,paramSize);

    fpURP->bRetValue = USB_SUCCESS;
}


//----------------------------------------------------------------------------
//          USB API Procedures Ends
//----------------------------------------------------------------------------


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBWrapGetATAError
//
// Description: This routine converts the sense data information into
//      ATAPI error code
//
// Input:   dSenseData  Sense data obtained from the device
//
// Output:  BYTE - ATAPI error code
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 USBWrapGetATAErrorCode (UINT32 dSenseData)
{
    UINT8   sc = (UINT8)dSenseData;             // Sense code
    UINT8   asc = (UINT8)(dSenseData >> 16);    // Additional Sense Code (ASC)
    UINT8   ascq = (UINT8)(dSenseData >> 8);    // Additional Sense Code Qualifier (ASCQ)

    if (ascq == 0x28) return USB_ATA_DRIVE_NOT_READY_ERR;
    if (sc == 7) return USB_ATA_WRITE_PROTECT_ERR;
    if ((asc == 0x80) && (ascq == 0x80)) return USB_ATA_TIME_OUT_ERR;
    if (ascq == 0x18) return USB_ATA_DATA_CORRECTED_ERR;
    if ((ascq==6) && (asc == 0)) return USB_ATA_MARK_NOT_FOUND_ERR;
    if ((ascq==0x3a) && (asc == 0)) return USB_ATA_NO_MEDIA_ERR;
    if ((ascq==0x11) && (asc == 0)) return USB_ATA_READ_ERR;
    if ((ascq==0x11) && (asc == 6)) return USB_ATA_UNCORRECTABLE_ERR;
    if (ascq==0x30) return USB_ATA_BAD_SECTOR_ERR;
    if ((ascq<0x20) || (ascq>0x26)) return USB_ATA_GENERAL_FAILURE;

    return USB_ATA_PARAMETER_FAILED;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBWrap_GetnthDeviceInfoStructure
//
// Description: This routine finds the 'n'th device's DeviceInfo entry.
//
// Input:   bDevNumber  Device number (1-based)
//
// Output:  DeviceInfo structure
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
USBWrap_GetnthDeviceInfoStructure(UINT8 bDevNumber)
{
    return &gUsbData->aDevInfoTable[bDevNumber];
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBWrap_GetDeviceCount
//
// Description: This routine searches through the device entry table
//      and returns number of active USB devices configured
//      by the BIOS.
//
// Input:   fpURPPointer    Pointer to the URP
//
// Output:  Following fields in the URP are modified
//      CkPresence.bNumBootDev      Number of USB boot devices found
//      CkPresence.bNumKeyboards    Number of USB keyboards present
//      CkPresence.bNumMice         Number of USB mice present
//      CkPresence.bNumHubs         Number of USB hubs present
//      CkPresence.bNumStorage      Number of USB storage devices present
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBWrap_GetDeviceCount(URP_STRUC *fpURP)
{
    DEV_INFO    *fpDevInfo;
    UINT8       i;

    for (i=1; i<MAX_DEVICES; i++) {
        fpDevInfo   = &gUsbData->aDevInfoTable[i];

        if ( (fpDevInfo->bFlag & DEV_INFO_VALID_STRUC) &&
            (fpDevInfo->bFlag & DEV_INFO_DEV_PRESENT)) {
            fpURP->ApiData.CkPresence.bNumBootDev++;

            switch (fpDevInfo->bDeviceType) {
				case BIOS_DEV_TYPE_HID:
                    if (fpDevInfo->HidDevType & HID_DEV_TYPE_KEYBOARD) {
                        fpURP->ApiData.CkPresence.bNumKeyboards++;
                    }
                    if (fpDevInfo->HidDevType & HID_DEV_TYPE_MOUSE) {
                        fpURP->ApiData.CkPresence.bNumMice++;
                    }
                    if (fpDevInfo->HidDevType & HID_DEV_TYPE_POINT) {
                        fpURP->ApiData.CkPresence.bNumPoint++;
                    }
					break;
										//<(EIP84455+)
                case  BIOS_DEV_TYPE_HUB:
                            fpURP->ApiData.CkPresence.bNumHubs++;
                            break;
                case  BIOS_DEV_TYPE_STORAGE:
                            fpURP->ApiData.CkPresence.bNumStorage++;
                            break;
                case  BIOS_DEV_TYPE_SECURITY:
                            break;
            }
        }
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   MemCopy
//
// Description: This routine copies data from source to destination.
//
// Input:
//    fpSrc    - Pointer to the source.
//    fpDest   - Pointer to the destination.
//    wSize    - Number of bytes to copy.
//
// Output: None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
MemCopy (
    UINT8*  fpSrc,
    UINT8*  fpDest,
    UINT32  dSize)
{
    UINT32  dCount;

    //
    // Check for pointer validity
    //
    if ((fpSrc) && (fpDest)) {
        for(dCount = 0; dCount < dSize; dCount++) {
            fpDest[dCount] = fpSrc[dCount];
        }
    }
}


UINTN DevicePathSize (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
    EFI_DEVICE_PATH_PROTOCOL     *Start;

    if (DevicePath == NULL) {
        return 0;
    }

    //
    // Search for the end of the device path structure
    //
    Start = DevicePath;
    while (!EfiIsDevicePathEnd (DevicePath)) {
        DevicePath = EfiNextDevicePathNode (DevicePath);
    }

    //
    // Compute the size and add back in the size of the end device path structure
    //
    return ((UINTN)DevicePath - (UINTN)Start) + sizeof(EFI_DEVICE_PATH_PROTOCOL);
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
