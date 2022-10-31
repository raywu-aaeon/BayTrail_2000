//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2010, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//****************************************************************************
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/usbsb.c 30    5/22/12 10:02a Ryanchou $
//
// $Revision: 30 $
//
// $Date: 5/22/12 10:02a $
//
//****************************************************************************
//
//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:   USBSB.C
//
//  Description:    USB South Bridge Porting Hooks
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>

//****************************************************************************

#include <Token.h>
#include <Efi.h>
#include "AmiDef.h"
#include "UsbDef.h"
#include "AmiUsb.h"
#include "UsbKbd.h"
#if USB_ACPI_ENABLE_WORKAROUND
#include "AcpiModeEnable.h"
#endif

#include <Protocol/SmmPeriodicTimerDispatch2.h>
#include <Protocol/SmmGpiDispatch2.h>

EFI_SMM_PERIODIC_TIMER_DISPATCH2_PROTOCOL *gPeriodicTimerDispatch = NULL;

EFI_HANDLE  gPeriodicTimerHandle = NULL;
EFI_HANDLE  gUsbIntTimerHandle = NULL;
BOOLEAN InstallXhciHwSmiHandler = FALSE;
BOOLEAN InstallUsbIntTimerHandler = FALSE;

extern  USB_GLOBAL_DATA     *gUsbData;

UINT8   ByteReadIO (UINT16);
VOID    ByteWriteIO (UINT16, UINT8);
UINT32  ReadPCIConfig(UINT16, UINT8);
VOID    WordWritePCIConfig(UINT16, UINT8, UINT16);
VOID    DwordWritePCIConfig(UINT16, UINT8, UINT32);

#define R_PCH_EHCI_EHCSUSCFG                0x7C  // EHC Suspend Well Configuration
#define R_PCH_ACPI_PM1_EN                   0x02  // Power Management 1 Enables
#define R_PCH_ACPI_GPE0a_STS                0x20  // General Purpose Event 0a Status
#define R_PCH_ACPI_GPE0a_EN                 0x28  // General Purpose Event 0a Enables

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Function:    USBSB_PeriodicTimerCallBack
//
// Description:
//  This function is registers periodic timer callbacks.
//
// Input:
//  Pointer to the EFI System Table
//
// Output:
//  - EFI_SUCCESS if timers are initialized or function is not implemented
//  - timer initialization error
//
// Note:
//  If function is not implemented (timers are not needed for this chipset),
//  function must return EFI_SUCCESS
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
USBSB_PeriodicTimerCallBack (
	EFI_HANDLE  DispatchHandle,
	CONST VOID  *Context,
	VOID    	*CommBuffer,
	UINTN   	*CommBufferSize
) 
{
/*
    DEV_INFO*       fpDevInfo;
    int i;
    DEV_INFO* pDev = gUsbData->aDevInfoTable;

	for (i = 0; i < USB_DEV_HID_COUNT; i++) {
		fpDevInfo = gUsbData->aUSBKBDeviceTable[i];
		if (fpDevInfo != NULL) break;
	}
    if(fpDevInfo == NULL){
        for (i = 1; i < MAX_DEVICES;  ++i, ++pDev ){
            if ( (pDev->bFlag & DEV_INFO_VALID_STRUC) != 0 && 
            	pDev->bDeviceType == BIOS_DEV_TYPE_HID &&
                (pDev->HidDevType & HID_DEV_TYPE_MOUSE) ) {
                fpDevInfo= pDev;    
                break; 
            }
        }
    }

    if (fpDevInfo != NULL){
        USBKBDPeriodicInterruptHandler(gUsbData->HcTable[fpDevInfo->bHCNumber - 1]);
    }
*/
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBSB_InstallXhciHwSmiHandler
//
// Description:
//  This function registers XHCI hardware SMI callback function.
//
// Note:
//  Currently EHCI, UHCI and OHCI drivers install their SMI handlers in the
//  corresponding Start functions. In the future all code related to SMI
//  registration can be moved here.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
USBSB_InstallXhciHwSmiHandler()
{
	EFI_STATUS  Status = EFI_SUCCESS;
	EFI_HANDLE	Handle = NULL;

#if	XHCI_SUPPORT
#if XHCI_EVENT_SERVICE_MODE != 0
//GPI service
	EFI_SMM_GPI_DISPATCH2_PROTOCOL          *GpiDispatch = NULL;
	EFI_SMM_GPI_REGISTER_CONTEXT            Context;
	
	UINT8		HwSmiPinTable[] = {USB_XHCI_EXT_HW_SMI_PINS};
	UINT8		i;

    if (InstallXhciHwSmiHandler) {
        return Status;
    }

    InstallXhciHwSmiHandler = TRUE;

	Status = pSmst->SmmLocateProtocol(&gEfiSmmGpiDispatch2ProtocolGuid, NULL, &GpiDispatch);
	ASSERT_EFI_ERROR(Status);	// driver dependencies?

	if (!EFI_ERROR(Status)) {
		for (i = 0; i < sizeof(HwSmiPinTable)/sizeof(UINT8); i++) {
			if(HwSmiPinTable[i] == 0xFF) continue;
                                        //(EIP61556)>
#if defined(GPI_DISPATCH_BY_BITMAP) && (GPI_DISPATCH_BY_BITMAP == 0)
            Context.GpiNum = HwSmiPinTable[i];
#else
			Context.GpiNum = (UINTN)1 << HwSmiPinTable[i];
#endif
                                        //<(EIP61556)
			GpiDispatch->Register(GpiDispatch, XhciHwSmiHandler, &Context, &Handle);
		}
	}
#endif

#if XHCI_EVENT_SERVICE_MODE != 1
	Status = USBSB_InstallUsbIntTimerHandler();
#endif
#endif

	return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        UsbIntTimerCallBack
//
// Description:
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
UsbIntTimerCallBack (
	EFI_HANDLE  DispatchHandle,
	CONST VOID  *Context,
	VOID    	*CommBuffer,
	UINTN   	*CommBufferSize
)
{
	HC_STRUC*   HcStruc;
	UINT8		i;
	DEV_INFO    *fpDevInfo; //EIP128872
	DEV_INFO    *pDev = &gUsbData->aDevInfoTable[1]; //EIP128872

    for (i = 0; i < gUsbData->HcTableCount; i++) {
	    HcStruc = gUsbData->HcTable[i];
        if (HcStruc == NULL) {
            continue;
        }
	    if(HcStruc->dHCFlag & HC_STATE_RUNNING) { //EIP128872 
	        (*gUsbData->aHCDriverTable[
				GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDProcessInterrupt)(HcStruc);
	    }
	}
    
//EIP128872 >>	
    // Added for USB keyrepeat
    for (i = 0; i < USB_DEV_HID_COUNT; i++) {
        fpDevInfo = gUsbData->aUSBKBDeviceTable[i];
        if (fpDevInfo != NULL) break;
    }
    if(fpDevInfo == NULL){
    for (i = 1; i < MAX_DEVICES; ++i, ++pDev ){
        if ( (pDev->bFlag & DEV_INFO_VALID_STRUC) != 0 && 
            pDev->bDeviceType == BIOS_DEV_TYPE_HID &&
            (pDev->HidDevType & HID_DEV_TYPE_MOUSE) ) { 
                fpDevInfo= pDev; 
            break; 
         }
       }
    }
    
    if(fpDevInfo != NULL){
        USBKBDPeriodicInterruptHandler((HC_STRUC*)&gUsbData->aHCDriverTable[fpDevInfo->bHCNumber-1]);
    }
//EIP128872 <<

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBSB_InstallUsbIntTimerHandler
//
// Description:
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
USBSB_InstallUsbIntTimerHandler()
{
#if (XHCI_TIMER_EVENT_SMI == 0) && (USB_RUNTIME_DRIVER_IN_SMM == 1) //EIP128872
	EFI_STATUS  Status;
	EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT  TimerContext;

	if (InstallUsbIntTimerHandler) {
		return EFI_SUCCESS;
	}

	InstallUsbIntTimerHandler = TRUE;

	Status = pSmst->SmmLocateProtocol (
			&gEfiSmmPeriodicTimerDispatch2ProtocolGuid, 
			NULL, 
			&gPeriodicTimerDispatch);
	ASSERT_EFI_ERROR(Status);	// driver dependencies?

	if (!EFI_ERROR(Status)) {
		TimerContext.Period = 160000;	//16Ms 
		TimerContext.SmiTickInterval = 160000; 

		Status = gPeriodicTimerDispatch->Register (
						gPeriodicTimerDispatch, 
						UsbIntTimerCallBack, 
						&TimerContext,
						&gUsbIntTimerHandle);
		ASSERT_EFI_ERROR(Status);
	}

	return Status;
//EIP128872 >>
#else
	return EFI_SUCCESS;
#endif
//EIP128872 <<
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBSB_UninstallTimerHandlers
//
// Description: This function unregisters all the periodic timer handles.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
USBSB_UninstallTimerHandlers()
{
	EFI_STATUS  Status = EFI_SUCCESS;

	if (gPeriodicTimerDispatch == NULL) {
		return Status;
	}

	if (gUsbIntTimerHandle) {
		Status = gPeriodicTimerDispatch->UnRegister (
						gPeriodicTimerDispatch, 
						gUsbIntTimerHandle);
		ASSERT_EFI_ERROR(Status);

		gUsbIntTimerHandle = NULL;
	}

	if (gPeriodicTimerHandle) {
		Status = gPeriodicTimerDispatch->UnRegister (
						gPeriodicTimerDispatch, 
						gPeriodicTimerHandle);
		ASSERT_EFI_ERROR(Status);

		gPeriodicTimerHandle = NULL;
	}

	return Status;
}

#if USB_ACPI_ENABLE_WORKAROUND
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        AcpiEnableCallBack
//
// Description:
//  This is ACPI mode enable callback function. It is a workaround for non 
//	XHCI/EHCI aware OSes.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
AcpiEnableCallBack(
	IN EFI_HANDLE   DispatchHandle
)
{
	USB_StopUnsupportedHc();
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        RegisterAcpiEnableCallBack
//
// Description:
//  This function registers ACPI enable callback function.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
RegisterAcpiEnableCallBack(
    IN EFI_ACPI_DISPATCH_PROTOCOL   *This,
    IN EFI_ACPI_DISPATCH            Function,
    OUT EFI_HANDLE                  *Handle
)
{
	EFI_STATUS                      Status;
	EFI_HANDLE                      RegisterHandle;
	EFI_ACPI_DISPATCH_PROTOCOL      *AcpiEnDispatch;

	Status = pSmst->SmmLocateProtocol(&gEfiAcpiEnDispatchProtocolGuid, NULL, &AcpiEnDispatch);
	if (EFI_ERROR(Status)) {
        return Status;
    }

	Status = AcpiEnDispatch->Register(AcpiEnDispatch, AcpiEnableCallBack, &RegisterHandle);
	ASSERT_EFI_ERROR(Status);

	return Status;
}
#endif

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBSB_InstallSmiEventHandlers
//
// Description:
//  This function is called from USBRT entry point inside SMM. Any SMI handlers
//  registration related to USB driver can be done here.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
USBSB_InstallSmiEventHandlers()
{
    EFI_STATUS  Status = EFI_SUCCESS;

/*  The following block is needed to implement non-USB periodic SMI where
    UHCI or OHCI controllers are not present

#if USB_HID_KEYREPEAT_USE_SETIDLE == 0
	EFI_SMM_PERIODIC_TIMER_REGISTER_CONTEXT	 PeriodicTimerContext;

	Status = pSmst->SmmLocateProtocol (
			&gEfiSmmPeriodicTimerDispatch2ProtocolGuid, 
			NULL, 
			&gPeriodicTimerDispatch);

    ASSERT_EFI_ERROR(Status);   // driver dependencies?

    if (!EFI_ERROR(Status)) {
        PeriodicTimerContext.Period = 160000;   //16Ms 
    	PeriodicTimerContext.SmiTickInterval = 160000; 

		//PeriodicTimerContext.TimerEnabled = FALSE;

    	Status = gPeriodicTimerDispatch->Register (
        	            gPeriodicTimerDispatch, 
            	        USBSB_PeriodicTimerCallBack, 
        				&PeriodicTimerContext,
                  		&gPeriodicTimerHandle);
    	ASSERT_EFI_ERROR(Status);
    }
    if (EFI_ERROR(Status)) {
        gPeriodicTimerDispatch = NULL;
    }
#endif
*/
#if USB_ACPI_ENABLE_WORKAROUND
	{
		EFI_ACPI_DISPATCH_PROTOCOL	*AcpiEnDispatch;
		VOID		                    *Reg;
	
		Status = pSmst->SmmLocateProtocol(&gEfiAcpiEnDispatchProtocolGuid, NULL, &AcpiEnDispatch);
		if (!EFI_ERROR(Status)) {
			RegisterAcpiEnableCallBack(NULL, NULL, NULL);
		} else {
			Status = pSmst->SmmRegisterProtocolNotify(
							&gEfiAcpiEnDispatchProtocolGuid,
							RegisterAcpiEnableCallBack,
							&Reg);
			ASSERT_EFI_ERROR(Status);
		}
	}
#endif
    return Status;
}
                                        //(EIP54018+)>
#if USB_S5_WAKEUP_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        UsbSbEnablePme
//
// Description: 
//  The funciton enable usb PME
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
UsbSbEnablePme(VOID)
{
	UINT16	BusDevFuncNum;
	UINT32	EhcSusWellConfig;
	
	BusDevFuncNum = (UINT16)((0 << 8) | (0x1d << 3) | 0);
	
    // Disable USB Wake on Device Connect/Disconnect
	EhcSusWellConfig = ReadPCIConfig(BusDevFuncNum, R_PCH_EHCI_EHCSUSCFG);
	EhcSusWellConfig |= (BIT16 | BIT17);
	DwordWritePCIConfig(BusDevFuncNum, R_PCH_EHCI_EHCSUSCFG, EhcSusWellConfig);

    // Clear PM1_STS
    IoWrite16(PM_BASE_ADDRESS, IoRead16(PM_BASE_ADDRESS));
    // Clear GPE0_STS
    IoWrite32(PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_STS, 0xFFFFFFFF);
    // Set PME_B0_EN
    IoWrite16(PM_BASE_ADDRESS + R_PCH_ACPI_GPE0a_EN, BIT13);
    // Clear PCI Express Wake Disable    
    IoWrite16(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_EN,
              IoRead16(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_EN & ~BIT14));
}

#endif
                                        //<(EIP54018+)
//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2010, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
