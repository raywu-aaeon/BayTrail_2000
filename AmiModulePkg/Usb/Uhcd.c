//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2009, American Megatrends, Inc.          **
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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/uhcd.c 121   9/12/12 9:45a Ryanchou $
//
// $Revision: 121 $
//
// $Date: 9/12/12 9:45a $
//
//****************************************************************************
//<AMI_FHDR_START>
//
//  Name:           UHCD.C
//
//  Description:    AMI USB Host Controller Driver
//
//<AMI_FHDR_END>
//****************************************************************************

#include <Token.h>
#include "AmiDef.h"
#include "UsbDef.h"
#include "Uhcd.h"
#include "UsbBus.h"
#include "EfiUsbKb.h"
#include "ComponentName.h"
#include "Protocol/Emul6064Trap.h"
#include <UsbDevDriverElinks.h>			//(EIP71750+)
#include <Protocol/PciIo.h>
#include <Pci.h>

extern UINT8 gFddHotplugSupport;
extern UINT8 gCdromHotplugSupport;
extern UINT8 gUsbMassNativeEmulation;
extern UINT8 UsbMassEmulationForNoMedia;	//(EIP86793+)	
extern UINT8 UsbMassSizeEmulation;		//(EIP80382+)
extern UINT8 gUsbMassMediaCheck;
extern UINT8 gUsbMassSkipFddMediaCheck;
extern UINT8 gUsbEfiMsDirectAccess;
extern UINT8 SkipCardReaderConnectBeep; //(EIP64781+)
extern VOID *gStartPointer;
extern VOID *gEndPointer;

extern  EFI_GUID gEfiSetupGuid;
extern  EFI_GUID gEfiUsbPolicyProtocolGuid;
extern  EFI_GUID gUsbTimingPolicyProtocolGuid;

const HCSPECIFICINFO aHCSpecificInfo[4] = {
{EFI_PCI_IO_ATTRIBUTE_IO | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER, 0x1000, 0x1000, DummyHcFunc, DummyHcFunc},      // UHCI
{EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER, 0x100,  0x100, DummyHcFunc, DummyHcFunc},   // OHCI
{EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER, 0x1000, 0x1000, DummyHcFunc, DummyHcFunc},  // EHCI
{EFI_PCI_IO_ATTRIBUTE_MEMORY | EFI_PCI_IO_ATTRIBUTE_BUS_MASTER, 0, 0, PreInitXhci, PostStopXhci},			// XHCI
};

UINT8	gSupportedInterfaceType[] = {
#if UHCI_SUPPORT
	PCI_CLASSC_PI_UHCI,
#endif
#if OHCI_SUPPORT
	PCI_CLASSC_PI_OHCI,
#endif
#if EHCI_SUPPORT
	PCI_CLASSC_PI_EHCI,
#endif
#if XHCI_SUPPORT
	PCI_CLASSC_PI_XHCI
#endif
};

USB_GLOBAL_DATA             *gUsbData;
EFI_USB_PROTOCOL            *gAmiUsbController;
EFI_EVENT                   gLegacyBootEvent;
EFI_EVENT                   gExitBootServicesEvent;
EFI_USB_POLICY_PROTOCOL EfiUsbPolicyProtocol;				//(EIP99882+)
                                        //(EIP60745+)>
EFI_DRIVER_BINDING_PROTOCOL gAmiUsbDriverBinding = {
    AmiUsbDriverBindingSupported,
    AmiUsbDriverBindingStart,
    AmiUsbDriverBindingStop,
    USB_DRIVER_VERSION,
    NULL,
    NULL
};
                                        //<(EIP60745+)
DEV_INFO*                   ICCQueueData[6];
EFI_EMUL6064TRAP_PROTOCOL   *gEmulationTrap = 0;

//extern USB_BADDEV_STRUC     gUsbBadDeviceTable[];			//(EIP60706-)

EFI_DRIVER_ENTRY_POINT (AmiUsbDriverEntryPoint)

VOID                        *gPciIoNotifyReg;
VOID                        *gProtocolNotifyRegistration;
BOOLEAN                     gLegacyUsbStatus=TRUE;
EFI_EVENT					gUsbIntTimerEvt = NULL;

#define EFI_PCIIRQ_PGM_PROTOCOL_GUID \
  { 0x9400d59b, 0xe9c, 0x4f6c, 0xb5, 0x9a, 0xfc, 0x20, 0x0, 0x9d, 0xb9, 0xec };

EFI_GUID gPciIrqProgramGuid = EFI_PCIIRQ_PGM_PROTOCOL_GUID;

typedef struct _PROGRAMMED_PCIIRQ_CTX {
    VOID    *PciIo;
    UINT8   Irq;
} PROGRAMMED_PCIIRQ_CTX;
										//(EIP71750+)>
typedef EFI_STATUS USB_DEV_EFI_DRIVER_CHECK (EFI_HANDLE, EFI_HANDLE);
extern USB_DEV_EFI_DRIVER_CHECK USB_DEV_EFI_DRIVER EndOfUsbDevEfiDriverList;
USB_DEV_EFI_DRIVER_CHECK* UsbDevEfiDrivers[]= {USB_DEV_EFI_DRIVER NULL};
										//<(EIP71750+)
                                        //(EIP60745+)>
#ifdef USB_CONTROLLERS_WITH_RMH
typedef struct {
    UINT16  Vid;
    UINT16  Did;
} CONTROLLER_WITH_RMH;

CONTROLLER_WITH_RMH gControllersWithRmh[] = {USB_CONTROLLERS_WITH_RMH};
#endif

                                        
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbDriverGetControllerName
//
// Description: This function is a part of binding protocol, it returns
//              the string "USB Host Controller".
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

CHAR16*
AmiUsbDriverGetControllerName(
    EFI_HANDLE  Controller,
    EFI_HANDLE  Child
)
{
    if (Child != NULL) {
        return NULL;
    }
    if (FindHcStruc(Controller)){
        return L"USB Host Controller";
    }
    return NULL;
}
                                        //<(EIP60745+)
#if !USB_RUNTIME_DRIVER_IN_SMM
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbIntTimerCallback
//
// Description: 
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
UsbIntTimerCallback(
	EFI_EVENT	Event,
	VOID		*Context
)
{
	EFI_TPL OriginalTPL;
	HC_STRUC*	HcStruc;
	UINT8	i;

	OriginalTPL = pBS->RaiseTPL (TPL_NOTIFY);

	for (i = 0; i < gUsbData->HcTableCount; i++) {
		 HcStruc = gUsbData->HcTable[i];
         if (HcStruc == NULL) {
            continue;
         }
		 if(HcStruc->dHCFlag & HC_STATE_RUNNING) { // Process appropriate interrupt
			 (*gUsbData->aHCDriverTable
				 [GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDProcessInterrupt)(HcStruc);
		 }
	 }

	pBS->RestoreTPL (OriginalTPL);
}
#endif

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbDriverEntryPoint
//
// Description: Entry point for AMI USB EFI driver
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiUsbDriverEntryPoint(
    IN EFI_HANDLE     ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
{
    EFI_HANDLE          UsbHandle = NULL;
    EFI_STATUS          Status;
    EFI_STATUS          SetupStatus;
    UINTN               VariableSize;
    USB_SUPPORT_SETUP   gSetupData;
    EFI_EVENT           PciIoEvent;
    EFI_EVENT           Emul6064Event;
    UINT8               DevDriverIndex;	//(EIP71750+)

    EfiInitializeDriverLib (ImageHandle, SystemTable);

    VariableSize = sizeof(USB_SUPPORT_SETUP);
    SetupStatus = pRS->GetVariable( L"UsbSupport", &gEfiSetupGuid, NULL,
                        &VariableSize, &gSetupData );

    if (SetupStatus == EFI_SUCCESS && gSetupData.UsbMainSupport == 0) {
        return EFI_UNSUPPORTED;
    }

    PROGRESS_CODE(DXE_USB_BEGIN);

    //
    // Allocate and initialize USB memory
    //
    gUsbData = AllocAlignedMemory (sizeof(USB_GLOBAL_DATA), 0x10);
    ASSERT(gUsbData);
    if (gUsbData == NULL) return EFI_OUT_OF_RESOURCES;

    //
    // Initialize the data area
    // Set the signature
    //
    gUsbData->dUSBSignature = 0x41445524; // $UDA

    //
    // Set the USB version number
    //
    gUsbData->stUSBVersion.bMajor  = USB_MAJOR_VER;
    gUsbData->stUSBVersion.bMinor  = USB_MINOR_VER;
    gUsbData->stUSBVersion.bBugRel = USB_BUG_RELEASE_VER;

    //
    // Initialize the state flag
    //
    gUsbData->dUSBStateFlag = 0;

    gUsbData->DeviceAddressMap = (UINT64)(~BIT0);

    //
    // Enable beep message during device connect/disconnect
    //
    gUsbData->dUSBStateFlag |= USB_FLAG_ENABLE_BEEP_MESSAGE;
    gUsbData->dUSBStateFlag |= USB_FLAG_RUNNING_UNDER_EFI;
                                        //(EIP80382)>
    if (gUsbMassNativeEmulation) {
        gUsbData->dUSBStateFlag |= USB_FLAG_MASS_NATIVE_EMULATION;
    }else if (UsbMassSizeEmulation) {        
        gUsbData->dUSBStateFlag |= USB_FLAG_MASS_SIZE_EMULATION;
    }
                                        //<(EIP80382)
										//(EIP86793+)>
    if (UsbMassEmulationForNoMedia) {
        gUsbData->dUSBStateFlag |= USB_FLAG_MASS_EMULATION_FOR_NO_MEDIA;
    }
										//<(EIP86793+)
    if (gUsbMassMediaCheck) {
        gUsbData->dUSBStateFlag |= USB_FLAG_MASS_MEDIA_CHECK;
        if (gUsbMassSkipFddMediaCheck)
            gUsbData->dUSBStateFlag |= USB_FLAG_MASS_SKIP_FDD_MEDIA_CHECK;
    }
    if (gUsbEfiMsDirectAccess) {
        gUsbData->dUSBStateFlag |= USB_FLAG_EFIMS_DIRECT_ACCESS;
    }
                                        //(EIP64781+)>
    if (SkipCardReaderConnectBeep) {
        gUsbData->dUSBStateFlag |= USB_FLAG_SKIP_CARD_READER_CONNECT_BEEP;
    }
                                        //<(EIP64781+)

    gUsbData->bKbdDataReady  = FALSE;
    gUsbData->bProcessingPeriodicList  = FALSE;
    gUsbData->NumberOfFDDs = 0;
    gUsbData->NumberOfHDDs = 0;
    gUsbData->NumberOfCDROMs = 0;

    gUsbData->QueueCnnctDisc.data = gUsbData->QueueData1;
    gUsbData->QueueCnnctDisc.maxsize = COUNTOF(gUsbData->QueueData1);
    gUsbData->QueueCnnctDisc.head = 0;
    gUsbData->QueueCnnctDisc.tail = 0;
    gUsbData->ICCQueueCnnctDisc.data = ICCQueueData;
    gUsbData->ICCQueueCnnctDisc.maxsize = COUNTOF(ICCQueueData);
    gUsbData->ICCQueueCnnctDisc.head = 0;
    gUsbData->ICCQueueCnnctDisc.tail = 0;

    Status = InitUsbSetupVars(gUsbData, pBS, pRS);
    ASSERT_EFI_ERROR(Status);
										//(EIP99882)>
    EfiUsbPolicyProtocol.UsbDevPlcy = &(gUsbData->UsbSetupData);

    // Install USB policy protocol
    Status = pBS->InstallProtocolInterface(
                &ImageHandle,
                &gEfiUsbPolicyProtocolGuid,
                EFI_NATIVE_INTERFACE,
                &EfiUsbPolicyProtocol
    );
										//<(EIP99882)
    ASSERT_EFI_ERROR(Status);

    gUsbData->UsbTimingPolicy.UsbTimingPolicyRevision = 01;
    gUsbData->UsbTimingPolicy.UsbTimingPolicyLength = 14;
    gUsbData->UsbTimingPolicy.EhciPortPowerOnDelay = 100;
    gUsbData->UsbTimingPolicy.EhciPortConnect = 20;
    gUsbData->UsbTimingPolicy.EhciPortReset = 50;
    gUsbData->UsbTimingPolicy.OhciHcResetDelay= 10;
    gUsbData->UsbTimingPolicy.OhciPortEnable = 100;
    gUsbData->UsbTimingPolicy.OhciHcReset = 2;
    gUsbData->UsbTimingPolicy.UhciGlobalReset = 10;
    gUsbData->UsbTimingPolicy.UhciPortEnable = 100;
    gUsbData->UsbTimingPolicy.HubPortConnect = 50;
    gUsbData->UsbTimingPolicy.HubPortEnable = 50;
    gUsbData->UsbTimingPolicy.MassDeviceComeUp = 500;
    gUsbData->UsbTimingPolicy.RmhPowerOnDelay= 100;
    Status = pBS->InstallProtocolInterface(
                &ImageHandle,
                &gUsbTimingPolicyProtocolGuid, 
                EFI_NATIVE_INTERFACE,
                &(gUsbData->UsbTimingPolicy));
    //
    // Allocate memory for the pool and store it in global data
    //
    gUsbData->MemPages = MEM_PAGE_COUNT;
    gUsbData->fpMemBlockStart = AllocAlignedMemory(gUsbData->MemPages << 12, 0x1000);

    //
    // Initialize the memory block status array to free
    //
    gUsbData->MemBlkStsBytes = ((gUsbData->MemPages << 12) / sizeof(MEM_BLK)) / 8;

	Status = pBS->AllocatePool(EfiRuntimeServicesData, gUsbData->MemBlkStsBytes, 
				&gUsbData->aMemBlkSts);
	ASSERT_EFI_ERROR(Status);
	pBS->SetMem(gUsbData->aMemBlkSts, gUsbData->MemBlkStsBytes, (UINT8)(~0));

#if USB_RUNTIME_DRIVER_IN_SMM
    // TODO:: update this when KBC is initialized (callback)
    gUsbData->kbc_support = (IoRead8(0x64)==0xff)? 0 : 1;
#endif
    //
    // Install USB protocol
    //
    gAmiUsbController = (EFI_USB_PROTOCOL*)AllocAlignedMemory (sizeof(EFI_USB_PROTOCOL), 0x10);

    USB_DEBUG(DEBUG_LEVEL_0, "AMIUHCD USB Init: data located at........... %x\n", (UINTN)gUsbData);
    USB_DEBUG(DEBUG_LEVEL_3, "Log address: %x\n", &gUsbData->aErrorLogBuffer[0]);
    USB_DEBUG(DEBUG_LEVEL_3, "Memory map: %x\n", &gUsbData->aMemBlkSts[0]);
    USB_DEBUG(DEBUG_LEVEL_3, "Device address map: %x\n", &gUsbData->DeviceAddressMap);

	gAmiUsbController->Signature = 0x50425355;	//USBP		//(EIP55275+)
    gAmiUsbController->USBDataPtr = gUsbData;
    //gAmiUsbController->UsbBadDeviceTable = gUsbBadDeviceTable;	//(EIP60706-)

    gAmiUsbController->UsbReportDevices = ReportDevices;
    gAmiUsbController->UsbGetNextMassDeviceName = GetNextMassDeviceName;
    gAmiUsbController->UsbChangeEfiToLegacy = UsbChangeEfiToLegacy;
    gAmiUsbController->UsbGetRuntimeRegion = GetRuntimeRegion;
    gAmiUsbController->InstallUsbLegacyBootDevices = Dummy2;
    gAmiUsbController->UsbInstallLegacyDevice = Dummy1;
    gAmiUsbController->UsbUninstallLegacyDevice = Dummy1;
    gAmiUsbController->UsbGetAssignBootPort = OemGetAssignUsbBootPort;
    gAmiUsbController->UsbRtShutDownLegacy = UsbRtShutDownLegacy;		//<(EIP52339+)
	gAmiUsbController->UsbCopySkipTable = UsbGetSkipList;			//(EIP51653+)
	gAmiUsbController->UsbRtStopController= UsbRtStopController;	//(EIP74876+)
    Status = gBS->InstallProtocolInterface (
        &UsbHandle,
        &gEfiUsbProtocolGuid,
        EFI_NATIVE_INTERFACE,
        gAmiUsbController
    );

    {
                                        //(EIP59272)>
        static NAME_SERVICE_T Names;
                                        //(EIP60745)>
        //static EFI_DRIVER_BINDING_PROTOCOL Binding = {
        //    AmiUsbDriverBindingSupported,
        //    AmiUsbDriverBindingStart,
        //    AmiUsbDriverBindingStop,
        //    USB_DRIVER_VERSION,
        //    NULL,
        //    NULL
        //};
        gAmiUsbDriverBinding.DriverBindingHandle = ImageHandle;
        gAmiUsbDriverBinding.ImageHandle = ImageHandle;

        Status = gBS->InstallMultipleProtocolInterfaces(
            &gAmiUsbDriverBinding.DriverBindingHandle,
            &gEfiDriverBindingProtocolGuid, &gAmiUsbDriverBinding,
            &gEfiComponentName2ProtocolGuid, InitNamesProtocol(
                &Names, L"AMI USB Driver", AmiUsbDriverGetControllerName),
            NULL);
                                        //<(EIP60745)
                                        //<(EIP59272)
										//(EIP71750)>
        for(DevDriverIndex = 0; UsbDevEfiDrivers[DevDriverIndex]; DevDriverIndex++) {
            VERIFY_EFI_ERROR(
                Status = UsbDevEfiDrivers[DevDriverIndex](ImageHandle,0));
            if( EFI_ERROR(Status) ) return Status;
        }
/*
        USB_DEBUG(DEBUG_LEVEL_3,"Calling UsbBusInit...\n");
        VERIFY_EFI_ERROR(
            Status = UsbBusInit(ImageHandle,0));
        if( EFI_ERROR(Status) ) return Status;
										//(EIP38434+)>
        USB_DEBUG(DEBUG_LEVEL_3,"Calling UsbHidInit...\n");
        VERIFY_EFI_ERROR(
            Status = UsbHidInit(ImageHandle,0));
        if( EFI_ERROR(Status) ) return Status;
//	        USB_DEBUG(DEBUG_LEVEL_3,"Calling UsbKbdInit...\n");
//	        VERIFY_EFI_ERROR(
//	            Status = UsbKbdInit(ImageHandle,0));
//	        if( EFI_ERROR(Status) ) return Status;
//	
//	        USB_DEBUG(DEBUG_LEVEL_3,"Calling UsbMsInit...\n");
//	        VERIFY_EFI_ERROR(
//	            Status = UsbMsInit(ImageHandle,0));
//	        if( EFI_ERROR(Status) ) return Status;
										//<(EIP38434+)
        USB_DEBUG(DEBUG_LEVEL_3,"Calling UsbMassInit...\n");
        VERIFY_EFI_ERROR(
            Status = UsbMassInit(ImageHandle,0));
        if( EFI_ERROR(Status) ) return Status;
#if USB_DEV_CCID
        USB_DEBUG(DEBUG_LEVEL_3,"Calling UsbCCIDInit...\n");
        VERIFY_EFI_ERROR(
            Status = UsbCCIDInit(ImageHandle,0));
        if( EFI_ERROR(Status) ) return Status;
#endif
*/
										//<(EIP71750)
    }

    //
    // Create the notification and register callback function on the PciIo installation
    //
    Status = pBS->CreateEvent (EVT_NOTIFY_SIGNAL, TPL_CALLBACK,
        UhcdPciIoNotifyCallback, NULL, &PciIoEvent);
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) return Status;

    Status = pBS->RegisterProtocolNotify (
                &gEfiPciIoProtocolGuid, PciIoEvent, &gPciIoNotifyReg);
    ASSERT_EFI_ERROR(Status);

#if defined(CSM_SUPPORT) && CSM_SUPPORT		//(EIP69136)
    // Install HW interrupt handler
    {
        EFI_EVENT   HwIrqEvent;
        EFI_STATUS  Status;

        Status = RegisterProtocolCallback(&gPciIrqProgramGuid, UhcdPciIrqPgmNotifyCallback,
                    	NULL, &HwIrqEvent, &gProtocolNotifyRegistration);
        ASSERT_EFI_ERROR(Status);
    }
#endif

    Status = gBS->LocateProtocol (&gNonSmmEmul6064TrapProtocolGuid, NULL, &gEmulationTrap);
    if (EFI_ERROR(Status)) {
        Status = RegisterProtocolCallback(&gNonSmmEmul6064TrapProtocolGuid, Emul6064NotifyCallback,
                    	NULL, &Emul6064Event, &gProtocolNotifyRegistration);
    }

	VERIFY_EFI_ERROR(
		EfiCreateEventLegacyBoot (
			EFI_TPL_NOTIFY,
			OnLegacyBoot,
			NULL,
			&gLegacyBootEvent ));

	VERIFY_EFI_ERROR(
		gBS->CreateEvent (
			EVT_SIGNAL_EXIT_BOOT_SERVICES,
			EFI_TPL_NOTIFY,
			OnExitBootServices,
			NULL,
			&gExitBootServicesEvent
		));

#if !USB_RUNTIME_DRIVER_IN_SMM
	VERIFY_EFI_ERROR(
		gBS->CreateEvent ( EFI_EVENT_TIMER | EFI_EVENT_NOTIFY_SIGNAL,
		EFI_TPL_NOTIFY, UsbIntTimerCallback, 0, &gUsbIntTimerEvt));	
	VERIFY_EFI_ERROR(
		gBS->SetTimer ( gUsbIntTimerEvt, TimerPeriodic, USB_INTERRUPT_POLLING_PERIOD));
#endif
    return Status;
}


VOID
Emul6064NotifyCallback(
    EFI_EVENT Event,
    VOID      *Context
)
{
    UINTN       BufferSize = sizeof(EFI_HANDLE);
    EFI_HANDLE  Handle;
    EFI_STATUS  Status;

    Status = pBS->LocateHandle(ByRegisterNotify,
                NULL, gProtocolNotifyRegistration, &BufferSize, &Handle);
    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status)) return;

    //
    // Locate EFI_EMUL6064TRAP_PROTOCOL installed on Handle and assign to gEmulationTrap
    //
    Status = pBS->HandleProtocol(Handle, &gNonSmmEmul6064TrapProtocolGuid, &gEmulationTrap);
    ASSERT_EFI_ERROR(Status);
}


#if defined(CSM_SUPPORT) && CSM_SUPPORT		//(EIP69136)
VOID
UhcdPciIrqPgmNotifyCallback(
    EFI_EVENT Event,
    VOID      *Context
)
{
    PROGRAMMED_PCIIRQ_CTX   *PciIrqCtx;
    EFI_PCI_IO_PROTOCOL     *PciIo;
    UINTN Seg, Bus, Dev, Func;
    EFI_STATUS  Status;

    Status = pBS->LocateProtocol(&gPciIrqProgramGuid, NULL, &PciIrqCtx);
    if (EFI_ERROR(Status)) {
        TRACE((-1, "USBHC:: can not locate PCI IRQ program interface.\n"));
        return;
    }

    PciIo = (EFI_PCI_IO_PROTOCOL*)PciIrqCtx->PciIo;
    if (PciIo == NULL) return;

    Status = PciIo->GetLocation(PciIo, &Seg, &Bus, &Dev, &Func);
    ASSERT_EFI_ERROR(Status);

    TRACE((-1, "Ready to install interrupt handler for IRQ%d for PCI B%d/D%d/F%d\n", PciIrqCtx->Irq, Bus, Dev, Func));
}
#endif

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UhcdPciIoNotifyCallback
//
// Description:
//  PciIo notification callback. It calls UpdateHcPciInfo porting function
//  to update the PCI information in the HC device table.
//
// Input:       Event - event signaled by the DXE Core upon PciIo installation
//              Context - event context
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
UhcdPciIoNotifyCallback (
    EFI_EVENT Event,
    VOID      *Context
)
{
    // Porting hook that updates a list of PCI devices that will be used during enumeration
    UpdateHcPciInfo();
    pBS->CloseEvent(Event); // this is one time callback
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  IsSupportedInterfaceType
//
// Description: This function check whether the Interface Type is supported.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN
IsSupportedInterfaceType (
	UINT8	InterfaceType
)
{
	UINT8	i;

	for (i = 0; i < sizeof(gSupportedInterfaceType); i++) {
		if (gSupportedInterfaceType[i] == InterfaceType) {
			return TRUE;
		}
	}
	return FALSE;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  IsExternalController
//
// Description: This function check whether the controller is behind bridge
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN
IsExternalController (
	EFI_DEVICE_PATH_PROTOCOL	*DevicePath
)
{
	EFI_STATUS					Status;
	EFI_DEVICE_PATH_PROTOCOL    *Dp = DevicePath;
    EFI_DEVICE_PATH_PROTOCOL    *DpPciNode = NULL;
	EFI_DEVICE_PATH_PROTOCOL	*DpBridge = NULL;
	EFI_HANDLE					Bridge = NULL;
	EFI_PCI_IO_PROTOCOL 		*PciIo = NULL;
	UINT8						PciClass[3];

	while(!EfiIsDevicePathEnd(Dp)) {
		if ((Dp->Type == HARDWARE_DEVICE_PATH) && 
			(Dp->SubType == HW_PCI_DP)) {
			DpPciNode = Dp;
			break;
		}
		Dp = EfiNextDevicePathNode(Dp);
	}
	ASSERT(DpPciNode);

	DpBridge = EfiDuplicateDevicePath(DevicePath);
	ASSERT(DpBridge);

	Dp = (EFI_DEVICE_PATH_PROTOCOL*)((UINTN)DpBridge + 
			((UINTN)EfiNextDevicePathNode(DpPciNode) - (UINTN)DevicePath));

	Dp->Type = EFI_END_ENTIRE_DEVICE_PATH;
	Dp->SubType = EFI_END_ENTIRE_DEVICE_PATH_SUBTYPE;
	SetDevicePathNodeLength(Dp, sizeof(EFI_DEVICE_PATH_PROTOCOL));

	Dp = DpBridge;
	Status = gBS->LocateDevicePath(
					  &gEfiPciIoProtocolGuid,
					  &Dp,
					  &Bridge);
	gBS->FreePool(DpBridge);
	if (EFI_ERROR(Status)) {
		return FALSE;
	}

	Status = pBS->HandleProtocol(Bridge, &gEfiPciIoProtocolGuid, &PciIo);
	if (EFI_ERROR(Status)) {
		return FALSE;
	}

	Status = PciIo->Pci.Read (
				 PciIo,
				 EfiPciIoWidthUint8,
				 PCI_PI_OFFSET,
				 sizeof(PciClass),
				 PciClass);
	if (EFI_ERROR(Status)) {
		return FALSE;
	}

	if (PciClass[2] != PCI_CL_BRIDGE || 
		PciClass[1] != PCI_CL_BRIDGE_SCL_P2P) {
		return FALSE;
	}

	return TRUE;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbDriverBindingSupported
//
// Description: Test to see if this driver supports ControllerHandle.
//
// Input:       This                - Protocol instance pointer
//              ControllerHandle    - Handle of device to test
//
// Output:      EFI_SUCCESS         - This driver supports this device.
//              EFI_UNSUPPORTED     - This driver does not support this device.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiUsbDriverBindingSupported (
    EFI_DRIVER_BINDING_PROTOCOL    *This,
    EFI_HANDLE                     Controller,
    EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
    )

{
    EFI_STATUS            		Status;
	EFI_PCI_IO_PROTOCOL   		*PciIo;
    USB_CLASSC            		UsbClassCReg;
	EFI_DEVICE_PATH_PROTOCOL	*DevicePath;

    Status = gBS->OpenProtocol (
			        Controller,
			        &gEfiPciIoProtocolGuid,
			        &PciIo,
			        This->DriverBindingHandle,
			        Controller,
			        EFI_OPEN_PROTOCOL_BY_DRIVER );
	if (EFI_ERROR(Status)) {
		return Status;
	}

    Status = PciIo->Pci.Read (
		        PciIo,
		        EfiPciIoWidthUint8,
		        CLASSC,
		        sizeof(USB_CLASSC) / sizeof(UINT8),
		        &UsbClassCReg);

    gBS->CloseProtocol (
	        Controller,
	        &gEfiPciIoProtocolGuid,
	        This->DriverBindingHandle,
	        Controller);

	if (EFI_ERROR(Status)) {
		return EFI_UNSUPPORTED;
	}

	if ((UsbClassCReg.BaseCode != PCI_CLASSC_BASE_CLASS_SERIAL) || 
		(UsbClassCReg.SubClassCode != PCI_CLASSC_SUBCLASS_SERIAL_USB) ||
		!IsSupportedInterfaceType(UsbClassCReg.PI)) {
		return EFI_UNSUPPORTED;
	}

	Status = gBS->OpenProtocol (
					Controller,
					&gEfiDevicePathProtocolGuid,
					&DevicePath,
					This->DriverBindingHandle,
					Controller,
					EFI_OPEN_PROTOCOL_BY_DRIVER );
	if (EFI_ERROR(Status)) {
		return Status;
	}

    gBS->CloseProtocol (
        Controller,
        &gEfiDevicePathProtocolGuid,
        This->DriverBindingHandle,
        Controller);

#if !EXTERNAL_USB_CONTROLLER_SUPPORT
	if (IsExternalController(DevicePath)) {
		return EFI_UNSUPPORTED;
	}
#endif

    return EFI_SUCCESS;

}  // end of AmiUsbDriverBindingSupported


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbDriverBindingStart
//
// Description: Binding protocol function to start the AMI USB driver
//
// Input:       This                - Protocol instance pointer.
//              ControllerHandle    - Handle of device to test
//
// Output:    EFI_SUCCESS         - This driver supports this device.
//            EFI_UNSUPPORTED     - This driver does not support this device.
//            EFI_DEVICE_ERROR    - This driver cannot be started due to device
//                                  Error
//            EFI_OUT_OF_RESOURCES
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiUsbDriverBindingStart (
    IN EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN EFI_HANDLE                   Controller,
    IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath )
{
    EFI_STATUS					Status;
	EFI_PCI_IO_PROTOCOL   		*PciIo;
	EFI_DEVICE_PATH_PROTOCOL	*DevicePath;
    UINTN			SegNum;
    UINTN       	BusNum;
    UINTN       	DevNum;
    UINTN       	FuncNum;
    UINT8       	PciCfg[0x40];
	HC_STRUC		*HcStruc;
	UINT64			Capabilities;
	URP_STRUC   	Parameters;

	USB_DEBUG(DEBUG_LEVEL_3, "AmiUsbDriverBindingStart for %x\n", Controller);
	
    Status = gBS->OpenProtocol (
			        Controller,
			        &gEfiPciIoProtocolGuid,
			        &PciIo,
			        This->DriverBindingHandle,
			        Controller,
			        EFI_OPEN_PROTOCOL_BY_DRIVER );
	if (EFI_ERROR(Status)) {
		return Status;
	}

	Status = PciIo->GetLocation(PciIo, &SegNum, &BusNum, &DevNum, &FuncNum);
	if (EFI_ERROR(Status)) {
		gBS->CloseProtocol (
				Controller,
				&gEfiPciIoProtocolGuid,
				This->DriverBindingHandle,
				Controller);
		return EFI_DEVICE_ERROR;
	}

	Status = PciIo->Pci.Read (
				 PciIo,
				 EfiPciIoWidthUint8,
				 0,
				 sizeof(PciCfg),
				 PciCfg);
	if (EFI_ERROR(Status)) {
		gBS->CloseProtocol (
				Controller,
				&gEfiPciIoProtocolGuid,
				This->DriverBindingHandle,
				Controller);
		return EFI_DEVICE_ERROR;
	}

    // Get the device path
	Status = gBS->OpenProtocol (
					Controller,
					&gEfiDevicePathProtocolGuid,
					&DevicePath,
					This->DriverBindingHandle,
					Controller,
					EFI_OPEN_PROTOCOL_BY_DRIVER );
	if (EFI_ERROR(Status)) {
		gBS->CloseProtocol (
				Controller,
				&gEfiPciIoProtocolGuid,
				This->DriverBindingHandle,
				Controller);
		return Status;
	}

	USB_DEBUG(3, "USB HC Bus# %x Dev# %x Func# %x, PI %x\n", BusNum, DevNum, FuncNum ,PciCfg[0x9]);
	// Try to locate EHCI controller
	if (PciCfg[0x9] == PCI_CLASSC_PI_UHCI || 
		PciCfg[0x9] == PCI_CLASSC_PI_OHCI ) {
		LocateEhciController(This, Controller, DevicePath);
	}

	// Enable the device
	Status = PciIo->Attributes (PciIo,
			EfiPciIoAttributeOperationSupported, 0,
			&Capabilities); 	// Get device capabilities
	ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status)) {
		gBS->CloseProtocol (
				Controller,
				&gEfiPciIoProtocolGuid,
				This->DriverBindingHandle,
				Controller);
		gBS->CloseProtocol (
				Controller,
				&gEfiDevicePathProtocolGuid,
				This->DriverBindingHandle,
				Controller);
		return EFI_DEVICE_ERROR;
	}

	Status = PciIo->Attributes (PciIo,
			EfiPciIoAttributeOperationEnable,
			Capabilities & EFI_PCI_DEVICE_ENABLE,
			NULL);				// Enable device
	ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status)) {
		gBS->CloseProtocol (
				Controller,
				&gEfiPciIoProtocolGuid,
				This->DriverBindingHandle,
				Controller);
		gBS->CloseProtocol (
				Controller,
				&gEfiDevicePathProtocolGuid,
				This->DriverBindingHandle,
				Controller);
		return EFI_DEVICE_ERROR;
	}

	HcStruc = AddHC(Controller, BusNum, DevNum, FuncNum, PciCfg[0x9], 
					PciCfg[0x3C], DevicePath, PciIo);
	if (HcStruc == NULL) {
		gBS->CloseProtocol (
				Controller,
				&gEfiPciIoProtocolGuid,
				This->DriverBindingHandle,
				Controller);
		gBS->CloseProtocol (
				Controller,
				&gEfiDevicePathProtocolGuid,
				This->DriverBindingHandle,
				Controller);
		return EFI_OUT_OF_RESOURCES;
	}

	// Initialize host controller
    Parameters.bFuncNumber = USB_API_HC_START_STOP;
    Parameters.ApiData.HcStartStop.Start = TRUE;
	Parameters.ApiData.HcStartStop.HcStruc = HcStruc;
	InvokeUsbApi(&Parameters);

	USB_DEBUG(3, "HC start completed, exit code %d.\n", Parameters.bRetValue);

    Status = InstallHcProtocols(This, Controller, PciIo, HcStruc);
	if (EFI_ERROR(Status)) {
		gBS->CloseProtocol (
				Controller,
				&gEfiPciIoProtocolGuid,
				This->DriverBindingHandle,
				Controller);
		gBS->CloseProtocol (
				Controller,
				&gEfiDevicePathProtocolGuid,
				This->DriverBindingHandle,
				Controller);
		return Status;
	}
	PROGRESS_CODE(DXE_USB_ENABLE);

    return EFI_SUCCESS;

} // end of AmiUsbDriverBindingStart


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AmiUsbDriverBindingStop
//
// Description: Stop this driver on ControllerHandle. Support stoping any
//              child handles created by this driver.
//
// Input:       This        - Protocol instance pointer.
//              DeviceHandle      - Handle of device to stop driver on
//              NumberOfChildren  - Number of Children in the ChildHandleBuffer
//              ChildHandleBuffer - List of handles for the children we
//                                  need to stop.
// Output:      EFI_SUCCESS on success, EFI_ERROR on error
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
AmiUsbDriverBindingStop (
    IN EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN EFI_HANDLE Controller,
    IN UINTN      NumberOfChildren,
    IN EFI_HANDLE *ChildHandleBuffer
)
{
    EFI_STATUS Status;
    URP_STRUC  Parameters;
    EFI_USB_HC_PROTOCOL     *HcProtocol;
    EFI_USB2_HC_PROTOCOL    *HcProtocol2;
    HC_DXE_RECORD           *DxeRecord;
    UINT32                  Index;
	UINT32					MemSize = 0;
	HC_STRUC				*HcStruc;
	UINT64					Capabilities;

    USB_DEBUG(DEBUG_LEVEL_3, "AmiUsbDriverBindingStop for %x\n", Controller);

	Status = gBS->OpenProtocol (
			Controller,
			&gEfiUsbHcProtocolGuid,
			&HcProtocol,
			This->DriverBindingHandle,
			Controller,
			EFI_OPEN_PROTOCOL_GET_PROTOCOL
			);
	if (EFI_ERROR(Status)) {
		return EFI_UNSUPPORTED;
	}

	Status = gBS->OpenProtocol (
			Controller,
			&gEfiUsb2HcProtocolGuid,
			&HcProtocol2,
			This->DriverBindingHandle,
			Controller,
			EFI_OPEN_PROTOCOL_GET_PROTOCOL
			);
	if (EFI_ERROR(Status)) {
		return EFI_UNSUPPORTED;
	}

	DxeRecord = (HC_DXE_RECORD*)(UINTN)HcProtocol;
	HcStruc = DxeRecord->hc_data;

	// Stop host controller
    Parameters.bFuncNumber = USB_API_HC_START_STOP;
    Parameters.ApiData.HcStartStop.Start = FALSE;
	Parameters.ApiData.HcStartStop.HcStruc = HcStruc;
	InvokeUsbApi(&Parameters);

	USB_DEBUG(3, "HC stop completed, exit code %d.\n", Parameters.bRetValue);

    Status = gBS->UninstallMultipleProtocolInterfaces ( Controller,
        &gEfiUsbHcProtocolGuid, HcProtocol,
        &gEfiUsb2HcProtocolGuid, HcProtocol2,
        NULL);
    ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status)) {
		return EFI_DEVICE_ERROR;
	}

	// Free HC memory
	Index = (UINT8)((HcStruc->bHCType - USB_HC_UHCI) >> 4);

	aHCSpecificInfo[Index].HcPostStop(Controller, HcStruc);
	if (HcStruc->fpFrameList) {
		FreeHcMemory(HcStruc->PciIo, 
			EFI_SIZE_TO_PAGES(aHCSpecificInfo[Index].FrameListSize),
			HcStruc->fpFrameList);
		HcStruc->fpFrameList = NULL;
	}

#if !USB_RUNTIME_DRIVER_IN_SMM
	if (HcStruc->MemPool) {
		FreeHcMemory(HcStruc->PciIo, HcStruc->MemPoolPages, HcStruc->MemPool);
		HcStruc->MemPool = NULL;
		gBS->FreePool(HcStruc->MemBlkSts);
		HcStruc->MemBlkSts = NULL;
	}
#endif

	gUsbData->HcTable[HcStruc->bHCNumber - 1] = NULL;
    gBS->FreePool(HcStruc);
    gUsbData->NumOfHc--;

	// Disable the device
	Status = DxeRecord->pciIo->Attributes (
			DxeRecord->pciIo,
			EfiPciIoAttributeOperationSupported, 0,
			&Capabilities); 	// Get device capabilities
	ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status)) {
		return EFI_DEVICE_ERROR;
	}

	Status = DxeRecord->pciIo->Attributes (
			DxeRecord->pciIo,
			EfiPciIoAttributeOperationDisable,
			Capabilities & EFI_PCI_DEVICE_ENABLE,
			NULL);				// Disable device
	ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status)) {
		return EFI_DEVICE_ERROR;
	}

    Status = gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
    );
    ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status)) {
		return Status;
	}

    Status = gBS->CloseProtocol (
        Controller,
        &gEfiDevicePathProtocolGuid,
        This->DriverBindingHandle,
        Controller);
    ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status)) {
		return Status;
	}

	// Free memory allocated in Start function
    gBS->FreePool(DxeRecord);

    return EFI_SUCCESS;

} // End of UHCIDriverBindingStop


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        HotplugIsEnabled
//
// Description: This function returns TRUE if there is a need for extra USB
//              devices that might be inserted/enumerated after legacy boot
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN HotplugIsEnabled()
{
    BOOLEAN FddHotplug;
    BOOLEAN HddHotplug;
    BOOLEAN CdromHotplug;

    FddHotplug = ((gUsbData->fdd_hotplug_support == SETUP_DATA_HOTPLUG_ENABLED)
        || ((gUsbData->fdd_hotplug_support == SETUP_DATA_HOTPLUG_AUTO)
            && (gUsbData->NumberOfFDDs == 0)));

    HddHotplug = ((gUsbData->hdd_hotplug_support == SETUP_DATA_HOTPLUG_ENABLED)
        || ((gUsbData->hdd_hotplug_support == SETUP_DATA_HOTPLUG_AUTO)
            && (gUsbData->NumberOfHDDs == 0)));

    CdromHotplug = ((gUsbData->cdrom_hotplug_support == SETUP_DATA_HOTPLUG_ENABLED)
        || ((gUsbData->cdrom_hotplug_support == SETUP_DATA_HOTPLUG_AUTO)
            && (gUsbData->NumberOfCDROMs == 0)));

    return FddHotplug || HddHotplug || CdromHotplug;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        OnExitBootServices
//
// Description: EXIT_BOOT_SERVICES notification callback function.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
OnExitBootServices(
    IN EFI_EVENT    Event,
    IN VOID       *Context
)
{
    URP_STRUC   Parameters;
    
#if USB_RUNTIME_DRIVER_IN_SMM
    HC_STRUC    *HcStruc;
    UINT8       Index;   
    for (Index = 0; Index < gUsbData->HcTableCount; Index++) {
        HcStruc = gUsbData->HcTable[Index];
        if (HcStruc == NULL) {
            continue;
        }
        if (HcStruc->HwSmiHandle != NULL) {
            continue;
        }
        // Stop host controller
        Parameters.bFuncNumber = USB_API_HC_START_STOP;
        Parameters.ApiData.HcStartStop.Start = FALSE;
	    Parameters.ApiData.HcStartStop.HcStruc = HcStruc;
	    InvokeUsbApi(&Parameters);
    }
#else

    gBS->SetTimer(gUsbIntTimerEvt, TimerCancel, 0);
    gBS->CloseEvent(gUsbIntTimerEvt);

    Parameters.bFuncNumber = USB_API_STOP;
    Parameters.bSubFunc = 0;
    InvokeUsbApi(&Parameters);
#endif

}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        OnLegacyBoot
//
// Description: This function is invoked when on Legacy Boot
//
// Input:       Event       - Efi event occurred upon legacyboot
//              Context     - Not used
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
OnLegacyBoot (
    IN EFI_EVENT  Event,
    IN VOID       *Context
)
{
    CONNECTED_USB_DEVICES_NUM   Devs;
//    URP_STRUC                   Parameters;			//<(EIP52339-)
    UINT8                       UsbLegacySupport;
	UINT32						EbdaAddr = (UINT32)(*((UINT16*)0x40E)) << 4;

	*(UINT32*)(EbdaAddr + USB_PROTOCOL_EBDA_OFFSET) = (UINT32)gAmiUsbController;

    UsbLegacySupport = UsbSetupGetLegacySupport();

    USB_DEBUG(DEBUG_LEVEL_3, "AMIUHCD::OnLegacyBoot::%d\n", UsbLegacySupport);

    //
    //Enable the KBC Emulation SMI's
    //
    if(gEmulationTrap != NULL && gLegacyUsbStatus) {
        gEmulationTrap->TrapEnable(gEmulationTrap);
    }

    gUsbData->dUSBStateFlag  &= ~(USB_FLAG_RUNNING_UNDER_EFI);

    switch (UsbLegacySupport) {
        case 0: break;  // Enable
        case 2: // Auto - check for devices, stop USB if none are present.
            if (HotplugIsEnabled()) break;  // Do not stop as Hotplug devices will be inserted
            ReportDevices(&Devs);
            if (Devs.NumUsbKbds+Devs.NumUsbMice+Devs.NumUsbPoint+Devs.NumUsbMass+Devs.NumUsbHubs) {		//(EIP38434)
                break;
            }
        case 1:         // Disable - stop USB controllers
					//(EIP52339)>
            UsbRtShutDownLegacy();
//	            gLegacyUsbStatus=FALSE;
//	            Parameters.bFuncNumber = USB_API_STOP;
//	            Parameters.bSubFunc = 0;
//	            gUsbData->fpURP = &Parameters;
//	            USB_DEBUG(DEBUG_LEVEL_3, "Stop USB controllers.\n");
//	            USBGenerateSWSMI (USB_SWSMI);
					//<(EIP52339)
            return;
    }

    UsbPrepareForLegacyOS();
}

#ifdef USB_CONTROLLERS_WITH_RMH
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        IsControllerWithRmh
//
// Description: This function checks if the controller has integrated 
//              USB 2.0 Rate Matching Hubs (RMH).
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN
IsControllerWithRmh (
    EFI_PCI_IO_PROTOCOL *PciIo
)
{
    UINT16  Vid = 0;
    UINT16  Did = 0;
    UINT8   Index = 0;

    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCI_VID, 1, &Vid);
    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCI_DID, 1, &Did);

    for (Index = 0; Index < COUNTOF(gControllersWithRmh); Index++) {
        if ((gControllersWithRmh[Index].Vid == Vid) &&
            (gControllersWithRmh[Index].Did == Did)) {
            return TRUE;
        }
    }
    return FALSE;
}
#endif

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AddHC
//
// Description: This function checks the type of controller and its PCI info
//              against gHcPciInfo data table; if HC is found appropriate, then
//              it allocates the frame list for this HC and adds the new HCStruc
//              entry.
//
// Input:       This              - Binding Protocol instance pointer
//              PciBus/Dev/Func   - PCI location of the HC
//              Controller        - Host Controller handle
//              Irq               - HW Interrupt number
//
// Output:      TRUE  - the new controller has been added
//              FALSE - controller was not added (see notes)
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

HC_STRUC*
AddHC (
    EFI_HANDLE					Controller,
    UINTN						PciBus,
    UINTN						PciDev,
    UINTN						PciFunc,
    UINT8       				HcType,
	UINT8       				Irq,
	EFI_DEVICE_PATH_PROTOCOL    *DevicePath,
	EFI_PCI_IO_PROTOCOL   		*PciIo
)
{
    HC_STRUC        **NewHcTable = NULL;
    HC_STRUC        *HcStruc = NULL;
    UINT8           Index;
	UINT16			PciAddr;
    EFI_STATUS      Status;

    if (gUsbData->NumOfHc >= gUsbData->HcTableCount) {
        NewHcTable = ReallocateMemory(
                    gUsbData->HcTableCount * sizeof(HC_STRUC*),
                    (gUsbData->HcTableCount + 1) * sizeof(HC_STRUC*),
                    (VOID*)gUsbData->HcTable);
        if (NewHcTable == NULL) {
            return NULL;
        }

        gUsbData->HcTable = NewHcTable;
        gUsbData->HcTableCount++;
    }

    for (Index = 0; Index < gUsbData->HcTableCount; Index++) {
        if(gUsbData->HcTable[Index] == NULL) {
            break;
        }
    }

    if (Index == gUsbData->HcTableCount) {
        return NULL;
    }

    Status = gBS->AllocatePool (EfiRuntimeServicesData, sizeof(HC_STRUC), &HcStruc);
    if (EFI_ERROR(Status)) {
        return NULL;
    }
    gBS->SetMem(HcStruc, sizeof(HC_STRUC), 0);

	PciAddr = (UINT16)((PciBus << 8) | (PciDev << 3) | PciFunc);
    USB_DEBUG(DEBUG_LEVEL_3, "AddHC for device %x\n", PciAddr);

    //
    // Initialize the HC_STRUC with available values
    //
    HcStruc->dHCFlag = HC_STATE_USED;
    HcStruc->bHCNumber = Index + 1;
    HcStruc->bHCType = (UINT8)(HcType + USB_HC_UHCI);
    HcStruc->wBusDevFuncNum = PciAddr;
    HcStruc->Controller = Controller;
	HcStruc->Irq = Irq;
	HcStruc->pHCdp = DevicePath;
	HcStruc->PciIo = PciIo;

    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCI_VID, 1, &HcStruc->Vid);
    PciIo->Pci.Read(PciIo, EfiPciIoWidthUint16, PCI_DID, 1, &HcStruc->Did);
    
#ifdef USB_CONTROLLERS_WITH_RMH
    if (IsControllerWithRmh(PciIo)) {
        HcStruc->dHCFlag |= HC_STATE_CONTROLLER_WITH_RMH;
    }
#endif

    //
    // Allocate memory for UHC
    // HC Type Index: 0/1/2/3 for U/O/E/XHCI
    //
    Index = (UINT8)(HcType >> 4);

    Status = aHCSpecificInfo[Index].HcPreInit(Controller, HcStruc);
    if (EFI_ERROR(Status)) {
		EfiZeroMem(HcStruc, sizeof(HC_STRUC));
		return NULL;
    }

	if (aHCSpecificInfo[Index].FrameListSize) {
        HcStruc->fpFrameList = (UINT32*)AllocateHcMemory ( PciIo,
					EFI_SIZE_TO_PAGES(aHCSpecificInfo[Index].FrameListSize), 
					aHCSpecificInfo[Index].FrameListAlignment);
		if (HcStruc->fpFrameList == NULL) {
			EfiZeroMem(HcStruc, sizeof(HC_STRUC));
			return NULL;
		}

        EfiZeroMem(HcStruc->fpFrameList, aHCSpecificInfo[Index].FrameListSize);
        USB_DEBUG(DEBUG_LEVEL_3, "Frame List is allocated at %x.\n", HcStruc->fpFrameList);
	}

#if !USB_RUNTIME_DRIVER_IN_SMM
	HcStruc->MemPoolPages = 2;
	HcStruc->MemPool = (UINT8*)AllocateHcMemory(PciIo, HcStruc->MemPoolPages, 0x1000);
	ASSERT(HcStruc->MemPool);
	gBS->SetMem(HcStruc->MemPool, HcStruc->MemPoolPages << 12, 0);

	HcStruc->MemBlkStsBytes = (HcStruc->MemPoolPages << 12) / sizeof(MEM_BLK) / 8;
	Status = gBS->AllocatePool(EfiRuntimeServicesData, HcStruc->MemBlkStsBytes , 
				&HcStruc->MemBlkSts);
	ASSERT_EFI_ERROR(Status);
	gBS->SetMem(HcStruc->MemBlkSts, HcStruc->MemBlkStsBytes, (UINT8)(~0));
#endif

	if (IsExternalController(DevicePath)) {
		HcStruc->dHCFlag |= HC_STATE_EXTERNAL;
	}

    gUsbData->HcTable[gUsbData->NumOfHc] = HcStruc;
    gUsbData->NumOfHc++;

    USB_DEBUG(DEBUG_LEVEL_3, " controller #%x added to HCStrucTable\n", HcStruc->bHCNumber);

    return HcStruc;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:    CheckDeviceReady
//
// Description: This function invokes USB Mass Storage API handler to
//              check whether device is ready. If called for the first time,
//              this function retrieves the mass storage device geometry
//              and fills the corresponding fpDevInfo fields.
//
// Input:       Pointer to device which needs to be checked
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
CheckDeviceReady(DEV_INFO* DevInfo)
{
#if USB_DEV_MASS
    URP_STRUC       Parameters;
    
    USB_DEBUG(DEBUG_LEVEL_3, "UHCD CheckDeviceReady-->");
    
    //
    // Prepare URP_STRUC with USB_MassRead attributes
    //
    Parameters.bFuncNumber  = USB_API_MASS_DEVICE_REQUEST;
    Parameters.bSubFunc   = USB_MASSAPI_CHECK_DEVICE;
    Parameters.ApiData.MassChkDevReady.fpDevInfo = DevInfo;
    
	InvokeUsbApi(&Parameters);
    USB_DEBUG(DEBUG_LEVEL_3, "-->done.\n");
#endif

}

/*
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:          GetNextMassDevice
//
// Description: This is the interface function that executes
//              USBMassAPIGetDeviceInformation function and returns the
//              information about mass storage device.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 GetNextMassDevice(UINT8 *Data, UINT8 DataSize, UINT8 DevAddr)
{
    URP_STRUC   Parameters;

    Parameters.bFuncNumber = USB_API_MASS_DEVICE_REQUEST;
    Parameters.bSubFunc = USB_MASSAPI_GET_DEVICE_INFO;
    Parameters.ApiData.MassGetDevInfo.bDevAddr = DevAddr;

    gUsbData->fpURP = &Parameters;
    USBGenerateSWSMI (USB_SWSMI);

    pBS->CopyMem(Data,
        (UINT8*)(UINTN)Parameters.ApiData.MassGetDevInfo.fpDevId, DataSize);

    return Parameters.ApiData.MassGetDevInfo.bDevAddr;
}
*/

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        GetNextMassDeviceName
//
// Description: This function returns a name string of connected mass storage
//              device.
//
// Input:       Data - Pointer to a string buffer to be filled
//              DataSize - Size of the data to copy to a buffer
//              DevIndex - Device index
// Output:      The updated device index, see below.
//
// Notes:       Initially DevIndex should be set to 0. This function returns
//              the name of the first mass storage device (if no device found
//              it returns DevIndex as 0FFh) and also updates DevIndex to the
//              device address of the current mass storage device. If no other
//              mass storage device is found then the routine sets the bit7 to 1
//              indicating current information is valid but no more mass device
//              found in the system. The caller can get the next device info if
//              DevIndex is not 0FFh and bit7 is not set.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
GetNextMassDeviceName(
    UINT8   *Data,
    UINT8   DataSize,
    UINT8   DevIndex
)
{
    UINT8   i;
    UINT8   CurrentDevIndex;

    for (i=DevIndex; i<MAX_DEVICES; i++) {
        if (gUsbData->aDevInfoTable[i].bDeviceType == BIOS_DEV_TYPE_STORAGE) {
            break;
        }
    }
    if (i == MAX_DEVICES) return USB_ERROR;  // No mass storage devices present

    if (!(gUsbData->aDevInfoTable[i].bFlag & DEV_INFO_DEV_PRESENT)) {
        return USB_ERROR;    // Invalid device
    }

    //
    // Copy device name
    //
    pBS->CopyMem(Data, &gUsbData->aDevInfoTable[i].DevNameString, DataSize);

    CurrentDevIndex = i;

    //
    // Look for the other devices for any subsequent calls
    //
    for (i++; i<MAX_DEVICES; i++) {
        if (gUsbData->aDevInfoTable[i].bDeviceType == BIOS_DEV_TYPE_STORAGE) {
            break;
        }
    }
    if (i == MAX_DEVICES) {
        CurrentDevIndex |= 0x80;    // No other devices, return current device w/ bit7
    } else {
        CurrentDevIndex = i;        // Return next device index
    }

    return CurrentDevIndex;

}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:          ClearLegacyUsbKbdBuffer
//
// Description: 
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
ClearLegacyUsbKbdBuffer(
	VOID
)
{
//		gUsbData->bLastUSBKeyCode = 0;	//(EIP102150-)

	EfiZeroMem(gUsbData->aKBCCharacterBufferStart, sizeof(gUsbData->aKBCCharacterBufferStart));
	gUsbData->fpKBCCharacterBufferHead = gUsbData->aKBCCharacterBufferStart;
	gUsbData->fpKBCCharacterBufferTail = gUsbData->aKBCCharacterBufferStart;
	
	EfiZeroMem(gUsbData->aKBCScanCodeBufferStart, sizeof(gUsbData->aKBCScanCodeBufferStart));
	gUsbData->fpKBCScanCodeBufferPtr = gUsbData->aKBCScanCodeBufferStart;
                                        
	EfiZeroMem(gUsbData->aKBCDeviceIDBufferStart, sizeof(gUsbData->aKBCDeviceIDBufferStart));               //(EIP102150+)
	EfiZeroMem(gUsbData->aKBCShiftKeyStatusBufferStart, sizeof(gUsbData->aKBCShiftKeyStatusBufferStart));   //(EIP102150+)    

	EfiZeroMem(gUsbData->aKBInputBuffer, sizeof(gUsbData->aKBInputBuffer));
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbChangeEfiToLegacy
//
// Description: This is the interface function that reports switches between EFI and
//              Legacy USB operation.
//
// Input:       Switch that indicates where the switch should be turned:
//                  1 - from EFI to Legacy
//                  0 - from Legacy to EFI
//
// Output:      None
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID UsbChangeEfiToLegacy (UINT8 EfiToLegacy)
{
                                        //(EIP96616+)>
    DEV_INFO        *DevInfo =  NULL; 
    DEV_INFO        *DevInfoEnd = gUsbData->aDevInfoTable + COUNTOF(gUsbData->aDevInfoTable); 

    if(EfiToLegacy) {  // Changing to Legacy
        if(gEmulationTrap != NULL && gLegacyUsbStatus) {
            gEmulationTrap->TrapEnable(gEmulationTrap);
        }
		ClearLegacyUsbKbdBuffer();

        gUsbData->dUSBStateFlag &= ~USB_FLAG_RUNNING_UNDER_EFI;

    } else {           // Changing to EFI
        if(gEmulationTrap != NULL && gLegacyUsbStatus) {
            gEmulationTrap->TrapDisable(gEmulationTrap);
        }
		ClearLegacyUsbKbdBuffer();
        
        for (DevInfo = &gUsbData->aDevInfoTable[1]; DevInfo != DevInfoEnd; ++DevInfo ){	
            if ((DevInfo->bFlag & DEV_INFO_VALIDPRESENT) == DEV_INFO_VALID_STRUC) {
                if (DevInfo->bFlag & DEV_INFO_DEV_BUS) {
                    QueuePut(&gUsbData->QueueCnnctDisc, DevInfo);
                }
            }
        }

        for (DevInfo = &gUsbData->aDevInfoTable[1]; DevInfo != DevInfoEnd; ++DevInfo ){	
            if ((DevInfo->bFlag & DEV_INFO_VALIDPRESENT) == DEV_INFO_VALIDPRESENT) {
                if (!(DevInfo->bFlag & DEV_INFO_DEV_BUS)) {
                    QueuePut(&gUsbData->QueueCnnctDisc, DevInfo);
                }
            }
        }

        gUsbData->dUSBStateFlag |= USB_FLAG_RUNNING_UNDER_EFI;
    }

                                        //<(EIP96616+)
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        ReportDevices
//
// Description: This is the interface function that reports the number of devices
//              currently controlled by the driver.
//
// Input:       Pointer to a structure that indicates the number of connected devices.
//
// Output:      Input structure is updated.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
ReportDevices(
    IN OUT CONNECTED_USB_DEVICES_NUM    *Devs
)
{
											//(EIP38434)> 
	UINT8	i,Kbd = 0, Hub = 0, Mouse = 0, Mass = 0, Point = 0, Ccid = 0;
	
	for (i = 1; i<MAX_DEVICES; i++) {

		if ((gUsbData->aDevInfoTable[i].bFlag & DEV_INFO_VALIDPRESENT) 
			!= DEV_INFO_VALIDPRESENT) {
			continue;
		}
		switch(gUsbData->aDevInfoTable[i].bDeviceType)
		{
			case BIOS_DEV_TYPE_HID:
                if (gUsbData->aDevInfoTable[i].HidDevType & HID_DEV_TYPE_KEYBOARD) {
					Kbd++;
                }
                if (gUsbData->aDevInfoTable[i].HidDevType & HID_DEV_TYPE_MOUSE) {
                    Mouse++;
                }
                if (gUsbData->aDevInfoTable[i].HidDevType & HID_DEV_TYPE_POINT) {
                    Point++;
                }
				break;

			case BIOS_DEV_TYPE_HUB:
				Hub++;
				break;

			case BIOS_DEV_TYPE_STORAGE:
				Mass++;
				break;

			case BIOS_DEV_TYPE_CCID:
				Ccid++;
				break;				
		}
	} 

USB_DEBUG(DEBUG_LEVEL_3, "Devices total: %d KBDs, %d HUBs, %d MICE, %d POINT %d MASS %d CCID\n", Kbd, Hub, Mouse, Point, Mass, Ccid);
    (*Devs).NumUsbKbds = Kbd;
    (*Devs).NumUsbMice = Mouse;
	(*Devs).NumUsbPoint = Point;
    (*Devs).NumUsbMass = Mass;
    (*Devs).NumUsbHubs = Hub;
    (*Devs).NumUsbCCIDs = Ccid;
										//	<(EIP38434+)
}

					//(EIP52339+)>
VOID    UsbRtShutDownLegacy(VOID)
{
//	    URP_STRUC   Params;
//	    
//	    Params.bFuncNumber = USB_API_LEGACY_CONTROL;
//	    Params.bFuncNumber = USB_SHUTDOWN_LEGACY;
//	
//	    gUsbData->fpURP = &Params;  // Need to update gUsbData->fpURP every time
//	    USBGenerateSWSMI (USB_SWSMI);
    URP_STRUC                   Parameters;

    if(gLegacyUsbStatus) {
        gLegacyUsbStatus=FALSE;
        Parameters.bFuncNumber = USB_API_STOP;
        Parameters.bSubFunc = 0;
        USB_DEBUG(DEBUG_LEVEL_3, "Stop USB controllers.\n");
		InvokeUsbApi(&Parameters);
    }
}    
					//<(EIP52339+)
										//(EIP74876+)>
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbRtStopController
//
// Description: This function stops the USB host controllers of a given 
//				Bus Dev Function
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
UsbRtStopController(
    UINT16 HcBusDevFuncNum
)
{
    URP_STRUC                   Parameters;

    Parameters.bFuncNumber = USB_API_USB_STOP_CONTROLLER;
    Parameters.ApiData.HcBusDevFuncNum = HcBusDevFuncNum;
	InvokeUsbApi(&Parameters);

}
										//<(EIP74876+)

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:		LocateEhciController
//
// Description: Visit all companions as different PCI functions of the same
//				PCI device as Controller (enumerate HCPCIInfo ). For each
//				companion function locate corresponding PCI_IO handle, execute
//				ConnectController if necessary; add them to aHCStrucTable.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
LocateEhciController(
	IN EFI_DRIVER_BINDING_PROTOCOL	*This,
	IN EFI_HANDLE					Controller,
	IN EFI_DEVICE_PATH_PROTOCOL		*CompanionDevicePath
)
{
	EFI_STATUS					Status;
    EFI_DEVICE_PATH_PROTOCOL    *Dp = CompanionDevicePath;
    EFI_DEVICE_PATH_PROTOCOL    *DpLastPciNode = NULL;
	EFI_DEVICE_PATH_PROTOCOL	*DpBridge;
	EFI_DEVICE_PATH_PROTOCOL	*DpRemaining;
	EFI_HANDLE                  Bridge = NULL;
	UINT8						EhciFunc;
	EFI_DEVICE_PATH_PROTOCOL	*DpEhci;
	PCI_DEVICE_PATH				*DpEhciPciNode;
	EFI_HANDLE                  Ehci = NULL;
	EFI_PCI_IO_PROTOCOL         *PciIo;
	UINTN						SegNum;
	UINTN						BusNum;
	UINTN						DevNum;
	UINTN						FuncNum;
	USB_CLASSC            		UsbClassCReg;
	EFI_USB2_HC_PROTOCOL		*UsbHc2Protocol;
	UINT8						ConnectAttempt = 0;

	USB_DEBUG(3, "LocateEhciController..\n");

	while(!EfiIsDevicePathEnd(Dp)) {
		Dp = EfiNextDevicePathNode(Dp);
		if ((Dp->Type == HARDWARE_DEVICE_PATH) && 
			(Dp->SubType == HW_PCI_DP)) {
			DpLastPciNode = Dp;
		}
	}

	DpBridge = EfiDuplicateDevicePath(CompanionDevicePath);
	if (DpBridge == NULL) {
		return EFI_OUT_OF_RESOURCES;
	}

	Dp = (EFI_DEVICE_PATH_PROTOCOL*)((UINT8*)DpBridge +
			((UINT8*)DpLastPciNode - (UINT8*)CompanionDevicePath ));

	Dp->Type = EFI_END_ENTIRE_DEVICE_PATH;
	Dp->SubType = EFI_END_ENTIRE_DEVICE_PATH_SUBTYPE;
	SET_NODE_LENGTH(Dp, sizeof(EFI_DEVICE_PATH_PROTOCOL));

	DpRemaining = DpBridge;
	Status = gBS->LocateDevicePath(
					  &gEfiDevicePathProtocolGuid,
					  &DpRemaining,
					  &Bridge);
	gBS->FreePool(DpBridge);
	if (EFI_ERROR (Status)) {
		return Status;
	}

	DpEhci = EfiDuplicateDevicePath(CompanionDevicePath);
	if (DpEhci == NULL) {
		return EFI_OUT_OF_RESOURCES;
	}

	DpEhciPciNode = (PCI_DEVICE_PATH*)((UINT8*)DpEhci +
            ((UINT8*)DpLastPciNode - (UINT8*)CompanionDevicePath )); // Locate last PCI node
	EhciFunc = ((PCI_DEVICE_PATH*)DpLastPciNode)->Function;

	for (EhciFunc++; EhciFunc <= 7; EhciFunc++) {
		DpEhciPciNode->Function = EhciFunc;

		ConnectAttempt = 0;
		do {
			USB_DEBUG(DEBUG_LEVEL_3, "\ttry Dev# %x Func# %x...",
					DpEhciPciNode->Device, DpEhciPciNode->Function);

			DpRemaining = DpEhci;
			// Locate EHCI handle using device path
			Status = gBS->LocateDevicePath(
							&gEfiPciIoProtocolGuid,
							&DpRemaining,
							&Ehci);
			if (!EFI_ERROR(Status)) {
				Status = gBS->OpenProtocol (
							Ehci,
							&gEfiPciIoProtocolGuid,
							&PciIo,
							This->DriverBindingHandle,
							Ehci,
							EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
				ASSERT_EFI_ERROR(Status);
				if (EFI_ERROR(Status)) {
					break;
				}
				Status = PciIo->GetLocation(
					PciIo, 
					&SegNum, 
					&BusNum, 
					&DevNum, 
					&FuncNum);
				ASSERT_EFI_ERROR(Status);
				if (EFI_ERROR(Status)) {
					break;
				}
				if ((DevNum == DpEhciPciNode->Device) &&
					(FuncNum == DpEhciPciNode->Function)) {
					Status = PciIo->Pci.Read (
							PciIo,
							EfiPciIoWidthUint8,
							CLASSC,
							sizeof(USB_CLASSC),
							&UsbClassCReg
						);
					ASSERT_EFI_ERROR(Status);
					if (EFI_ERROR(Status) ||
						(UsbClassCReg.BaseCode != PCI_CLASSC_BASE_CLASS_SERIAL) ||
						(UsbClassCReg.SubClassCode != PCI_CLASSC_SUBCLASS_SERIAL_USB) ||
						(UsbClassCReg.PI != PCI_CLASSC_PI_EHCI)) {
						USB_DEBUG(DEBUG_LEVEL_3, "BaseCode %x, SubClassCode %x, PI %x...", 
							UsbClassCReg.BaseCode, UsbClassCReg.SubClassCode, UsbClassCReg.PI);
						Status = EFI_NOT_FOUND;
						USB_DEBUG(DEBUG_LEVEL_3, "%r\n", Status);
						break;
					}
					Status = gBS->OpenProtocol (
							Ehci,
							&gEfiUsbHcProtocolGuid,
							&UsbHc2Protocol,
							This->DriverBindingHandle,
							Ehci,
							EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
							);
					if (!EFI_ERROR(Status)) {
						Status = EFI_ALREADY_STARTED;
						USB_DEBUG(DEBUG_LEVEL_3, "%r\n", Status);
						break;
					}
					Status = gBS->ConnectController(Ehci, NULL, NULL, FALSE);
					USB_DEBUG(DEBUG_LEVEL_3, "%r\n", Status);
					break;
				}
			}

	        // ConnectController to produce EHCI handle.
	        // Do not assert on EFI_ERROR because controller
	        // might not be functional (hidden for example) due
	        // to the board implementation or project policy and
	        // unsuccessfull connection is okay.
	        Status = gBS->ConnectController(Bridge, 0,
	            		(EFI_DEVICE_PATH_PROTOCOL*)DpEhciPciNode, FALSE);
		} while (!EFI_ERROR(Status) && (++ConnectAttempt < 2));
	}

	gBS->FreePool(DpEhci);

	return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        CalculateMemorySize
//
// Description: This function returns the total amount of memory used by
//              all supported USB controllers.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 CalculateMemorySize(VOID)
{
    UINT32 Result = 0;

    Result += (UINT32)(sizeof(USB_GLOBAL_DATA));
    Result += 0x10;     // Alignment for USB_GLOBAL_DATA allocation

	Result += (UINT32)(sizeof(EFI_USB_PROTOCOL));
	Result += 0x10; 	// Alignment for EFI_USB_PROTOCOL allocation

USB_DEBUG(DEBUG_LEVEL_3,"USB RT memory allocation:\n 0x%x (gUsbData), ", Result);

    Result += (UINT32) (MEM_BLK_COUNT * sizeof(MEM_BLK));
    Result += 0x1000; // Alignment for local memory pool

USB_DEBUG(DEBUG_LEVEL_3," 0x%x (gUsbData+Pool), ", Result);
    //
    // The following alignment adjustment are made with the assumption of
    // the sequentual AllocAlignedMemory calls for frame lists allocation;
    // if frame list allocations procedure will be change, the alignments
    // might be revised.
    //
USB_DEBUG(DEBUG_LEVEL_3,"totally 0x%x Bytes allocated\n", Result);

    return Result;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  GetRuntimeRegion
//
// Description: This function returns the beginning and the end of USB 
//              runtime memory region.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
GetRuntimeRegion (
    EFI_PHYSICAL_ADDRESS *Start,
    EFI_PHYSICAL_ADDRESS *End
)
{
    *Start = (EFI_PHYSICAL_ADDRESS)gStartPointer;
    *End = (EFI_PHYSICAL_ADDRESS)gEndPointer;
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  Dummy1
//
// Description: This function is a legacy mass storage support API stub,
//              replaced by the API producer.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
Dummy1(
    USB_MASS_DEV*   Device
)
{
    return EFI_UNSUPPORTED;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  Dummy2
//
// Description: This function is a legacy mass storage support API stub,
//              replaced by the API producer.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
Dummy2(
    VOID
)
{
    return EFI_UNSUPPORTED;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:  DummyHcFunc
//
// Description: This function is dummy HC memory allocation routine.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
DummyHcFunc(
    EFI_HANDLE  Handle,
    HC_STRUC    *HcStruc
)
{
    return EFI_SUCCESS;
}

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2009, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
