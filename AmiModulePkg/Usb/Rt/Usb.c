#pragma warning(disable: 4001)
#pragma warning(disable: 4127)
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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/RT/usb.c 130   9/03/12 4:55a Roberthsu $
//
// $Revision: 130 $
//
// $Date: 9/03/12 4:55a $
//****************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:           Usb.c
//
//  Description:    AMI USB main wrapper
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"
#include "UsbDef.h"

#include <Pci.h>
#include <Library/AmiChipsetIoLib.h> //EIP148801 

BOOLEAN     KeyRepeatStatus=FALSE;

BOOLEAN	OEMSkipList(UINT8,UINT8,UINT16,UINT8,UINT8);		//(EIP74609+)

#if USB_DEV_HUB
UINT8   USBHub_EnablePort(HC_STRUC*, UINT8, UINT8);
UINT8   USBHub_DisablePort(HC_STRUC*, UINT8, UINT8);
UINT8   USBHub_ResetPort(HC_STRUC*, UINT8, UINT8);
//VOID    USBHubFillDriverEntries (DEV_DRIVER*);    //(EIP71750-)
UINT8   USBHub_GetPortStatus (HC_STRUC*, UINT8, UINT8);
#endif

VOID    SpeakerBeep (UINT8, UINT16, HC_STRUC*);
VOID    FixedDelay(UINTN);
//VOID    BusFillDriverEntries(DEV_DRIVER*);    //(EIP71750-)
										//(EIP38434+)>
//#if USB_DEV_KBD
//VOID    USBKBDFillDriverEntries (DEV_DRIVER*);
//#endif
//#if USB_DEV_MOUSE
//VOID    USBMSFillDriverEntries (DEV_DRIVER*);
//#endif
                                        //(EIP71750-)>                                       
//VOID    USBHIDFillDriverEntries (DEV_DRIVER*);
										//<(EIP38434+)
//#if USB_DEV_MASS
//VOID    USBMassFillDriverEntries (DEV_DRIVER*);
//#endif
//#if USB_DEV_CCID
//VOID    USBCCIDFillDriverEntries (DEV_DRIVER*);
//#endif
                                        //<(EIP71750-)
VOID    MemCopy (UINT8*, UINT8*, UINT32);

VOID USBAPI_CheckDevicePresence (URP_STRUC*);

extern  USB_BADDEV_STRUC gUsbBadDeviceTable[];				//(EIP60706)

DEV_INFO*    USB_GetDeviceInfoStruc(UINT8, DEV_INFO*, UINT8, HC_STRUC*);    //(EIP98145)
VOID*       USB_MemAlloc(UINT16);
UINT8       USB_MemFree (VOID*, UINT16);
UINT8*      USB_GetDescriptor (HC_STRUC*, DEV_INFO*, UINT8*, UINT16, UINT8 , UINT8);
UINT8       USB_SetAddress(HC_STRUC*,DEV_INFO*, UINT8);
DEV_INFO*   USBIdentifyAndConfigureDevice (HC_STRUC* , DEV_INFO* , UINT8* , UINT16 ,UINT16 );
UINT8       USB_DisconnectDevice (HC_STRUC*, UINT8, UINT8 );
VOID        USB_InitFrameList (HC_STRUC*, UINT32);

UINT8   USB_MemInit (VOID);
UINT8   USBInitHostController(UINT16 *, UINT8);
UINT8   USB_EnumerateRootHubPorts(UINT8);
UINT8   USBLogError(UINT16);
BOOLEAN CheckDeviceLimit(UINT8);
VOID	USB_SmiQueuePutMsg(QUEUE_T*, VOID*, int);

										//(EIP54018+)>
UINT32  ReadPCIConfig(UINT16, UINT8);
#if USB_S5_WAKEUP_SUPPORT
VOID    DwordWritePCIConfig(UINT16, UINT8, UINT32);
VOID    WordWritePCIConfig(UINT16, UINT8, UINT16);
VOID    ByteWritePCIConfig(UINT16, UINT8, UINT8);
VOID    UsbSbEnablePme(VOID);
#endif
										//<(EIP54018+)

extern  USB_GLOBAL_DATA  *gUsbData;  // Defined in AMIUHCD

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_StartHostControllers
//
// Description: This function initializes the USB host controllers and
//              enumerate the root hub ports for possible USB devices.
//
// Output:      fpGlobalDataArea    Far pointer to the global data area
//
// Output:     Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_StartHostControllers(UINT8* fpGlobalDataArea)
{
    return USB_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       StopControllerType
//
// Description: This function stops all USB host controllers of a given type
//
// Output:      HC type
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
StopControllerType(
    UINT8 hc_type
)
{
    UINT8 i;
    HC_STRUC*   fpHCStruc;

    USB_DEBUG(DEBUG_LEVEL_3, "stopping all HC type %x:", hc_type);
    for (i = 0; i < gUsbData->HcTableCount; i++) {
        fpHCStruc = gUsbData->HcTable[i];
        if (fpHCStruc == NULL) {
            continue;
        }
        if ((fpHCStruc->bHCType == hc_type) &&
            (fpHCStruc->dHCFlag & HC_STATE_RUNNING)) {
            (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDStop)(fpHCStruc);
            USB_DEBUG(DEBUG_LEVEL_3, ".");
        }
    }
    USB_DEBUG(DEBUG_LEVEL_3, "\n");
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       StartControllerType
//
// Description: This function start all USB host controllers of a given type
//
// Output:      HC type
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
StartControllerType(
    UINT8 hc_type
)
{
    UINT8 i;
    HC_STRUC*   fpHCStruc;

    USB_DEBUG(DEBUG_LEVEL_3, "starting all HC type %x:", hc_type);
    for (i=0; i<gUsbData->HcTableCount; i++) {
        fpHCStruc = gUsbData->HcTable[i];
        if (fpHCStruc == NULL) {
            continue;
        }
        if ((fpHCStruc->bHCType == hc_type) ) {
            (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDStart)(fpHCStruc);
            USB_DEBUG(DEBUG_LEVEL_3, ".");
        }
    }
    USB_DEBUG(DEBUG_LEVEL_3, "\n");
}

										//(EIP74876+)>
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:       StopControllerBdf
//
// Description: This function stops the USB host controllers of a given Bus Dev Function
//
// Input:       BusDevFuncNum
//
// Output:      None
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
StopControllerBdf(
    UINT16 BusDevFuncNum
)
{
    UINT8 i;
    HC_STRUC*   HcStruc;

    for (i = 0; i < gUsbData->HcTableCount; i++) {
        HcStruc = gUsbData->HcTable[i];
        if (HcStruc == NULL) {
            continue;
        }
        if ((HcStruc->wBusDevFuncNum == BusDevFuncNum) &&
            (HcStruc->dHCFlag & HC_STATE_RUNNING)) {
            (*gUsbData->aHCDriverTable[GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDStop)(HcStruc);
            break;
        }
    }
}
										//<(EIP74876+)

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_StopHostControllers
//
// Description: This function stops the USB host controllers and
//              frees the data structures associated with the host controllers
//              In case of USB2.0 first stop USB1.1 controllers, then USB2.0.
//
// Output:      None
//
// Output:     Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_StopHostControllers (VOID)
{
    // Suppress disconnect beeps as they might be confusing
    gUsbData->dUSBStateFlag  &= (~USB_FLAG_ENABLE_BEEP_MESSAGE);

    StopControllerType(USB_HC_XHCI);    				//(EIP52339+)
    StopControllerType(USB_HC_UHCI);
    StopControllerType(USB_HC_OHCI);
    StopControllerType(USB_HC_EHCI);

    return USB_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       UsbHcStart
//
// Description: This function initializes the USB host controller and
//              enumerate the root hub ports for possible USB devices.
//
// Input:		HcStruc		HC struc pointer
//
// Output:		Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHcStart(HC_STRUC* HcStruc)
{
	USB_DEBUG(DEBUG_LEVEL_3, "Starting HC %X, HCNum %d, type %x\n",
                    HcStruc->wBusDevFuncNum, HcStruc->bHCNumber, HcStruc->bHCType);
	
	// Execute start routine of the host controller driver
	(*gUsbData->aHCDriverTable[GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDStart)(HcStruc);

	// Check if the HC is running
	if ((HcStruc->dHCFlag & HC_STATE_RUNNING) == 0) {
		return USB_ERROR;
	}
	HcStruc->dHCFlag |= HC_STATE_INITIALIZED;

	USB_DEBUG(DEBUG_LEVEL_3, "Enumerating HC#%d, type 0x%x\n", HcStruc->bHCNumber, HcStruc->bHCType);
	// Issue enumerate call for this HC
	(*gUsbData->aHCDriverTable[GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDEnumeratePorts)(HcStruc);

	return USB_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       UsbHcStop
//
// Description: This function stops the USB host controller.
//
// Input:		HcStruc		HC struc pointer
//
// Output:		Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHcStop(HC_STRUC* HcStruc)
{
	if ((HcStruc->dHCFlag & HC_STATE_RUNNING) == 0) {
		return USB_ERROR;
	}

	USB_DEBUG(DEBUG_LEVEL_3, "Stopping HC %X, HCNum %d, type %x\n", 
		 		HcStruc->wBusDevFuncNum, HcStruc->bHCNumber, HcStruc->bHCType);
	(*gUsbData->aHCDriverTable[GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDStop)(HcStruc);

	return USB_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_EnmumerateRootHubPorts
//
// Description: This function enumerates the root hub ports of the all
//              selected type HCs
//
//
// Output:      bType   - HC type
//
// Output:     Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_EnumerateRootHubPorts (UINT8 bType)
{
    UINT8		Index;
    HC_STRUC*   fpHCStruc;

    USB_DEBUG(DEBUG_LEVEL_4, "Enumerating HC Ports.\n");
    for (Index = 0; Index < gUsbData->HcTableCount; Index++) {
        //
        // Get the HCStruc pointer associated with this controller
        //
        fpHCStruc = gUsbData->HcTable[Index];
        if (fpHCStruc == NULL) {
              continue;
        }

        if((fpHCStruc->bHCType) == bType && (fpHCStruc->dHCFlag & HC_STATE_RUNNING)) {
            USB_DEBUG(DEBUG_LEVEL_3, "Enumerating HC#%d, type 0x%x\n", fpHCStruc->bHCNumber, bType);
            //
            // Issue enumerate call for this HC
            //
            (*gUsbData->aHCDriverTable[GET_HCD_INDEX(bType)].pfnHCDEnumeratePorts)(fpHCStruc);
        }
    }
    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_InitHostControllers
//
// Description: This function initializes the specified type of the HC
//              from the provided list of host controller PCI addresses
//
// Output:      pHCPCIAddrList  Pointer to table of HC PCI addresses in the system
//              bHCType         Type of HC to be initialized (EHCI, OHCI etc)
//
// Output:     Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBInitHostController(
    UINT16  * pHCPCIAddrList,
    UINT8   bHCType)
{
    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_GetHubPortStatus
//
// Description: This function returns the hub port status
//
// Input:       HcStruc   HC struc pointer
//              HubAddr   USB device address of the hub or HC number
//                        BIT7 = 1/0  Roothub/Hub
//              PortNum   Port number
//
// Output:      Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_GetHubPortStatus (
    HC_STRUC*	HcStruc,
    UINT8       HubAddr,
    UINT8		PortNum)
{
    //
    // Check whether the request is for root hub or generic hub
    //
    if (HubAddr & BIT7) {
		// Root hub
        return (*gUsbData->aHCDriverTable[GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDGetRootHubStatus)(
                                HcStruc, PortNum);
    } else {
    #if USB_DEV_HUB
        return USBHub_GetPortStatus (HcStruc, HubAddr, PortNum);
    #else
        return 0;
    #endif
    }
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_DisableHubPort
//
// Description: This function disables the hub port
//
// Output:      fpHCStruc   HC struc pointer
//              bHubAddr    USB device address of the hub or HC number
//                          BIT7 = 1/0  Roothub/Hub
//              bPortNum    Port number
//
// Output:     Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_DisableHubPort(
    HC_STRUC*   fpHCStruc,
    UINT8       bHubAddr,
    UINT8       bPortNum)
{
    //
    // Check whether the request is for root hub or generic hub
    //
    if (bHubAddr & BIT7) {
        //
        // Issue the disable root hub call to disable the hub port
        //
        (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDDisableRootHub)(
                        fpHCStruc,
                        bPortNum);
#if USB_DEV_HUB
    }
    else {
        USBHub_DisablePort (fpHCStruc, bHubAddr, bPortNum);
#endif
    }
    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_EnableHubPort
//
// Description: This function enables the hub port
//
// Output:      fpHCStruc   HC struc pointer
//              bHubAddr    USB device address of the hub or HC number
//                          BIT7 = 1/0  Roothub/Hub
//              bPortNum    Port number
//
// Output:     Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_EnableHubPort (
    HC_STRUC*   fpHCStruc,
    UINT8       bHubAddr,
    UINT8       bPortNum)
{
    //
    // Check whether the request is for root hub or generic hub
    //
    if (bHubAddr & BIT7) {
        //
        // Root hub
        // Issue the disable root hub call to disable the hub port
        //
        return (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDEnableRootHub)(fpHCStruc, bPortNum);
    } else {
    #if USB_DEV_HUB
        return USBHub_EnablePort (fpHCStruc, bHubAddr, bPortNum);
    #else
        return USB_ERROR;   // Only root hub could be successfully enabled
    #endif
    }
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_ResetHubPort
//
// Description: This function resets the hub port
//
// Input:       HcStruc   HC struc pointer
//              HubAddr   USB device address of the hub or HC number
//                        BIT7 = 1/0  Roothub/Hub
//              PortNum   Port number
//
// Output:      Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_ResetHubPort (
    HC_STRUC*	HcStruc,
    UINT8       HubAddr,
    UINT8		PortNum)
{
    //
    // Check whether the request is for root hub or generic hub
    //
    if (HubAddr & BIT7) {
        //
        // Root hub
        // Issue the reset root hub call to reset the hub port
        //
        return (*gUsbData->aHCDriverTable[GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDResetRootHub)(HcStruc, PortNum);
    } else {
    #if USB_DEV_HUB
        return USBHub_ResetPort (HcStruc, HubAddr, PortNum);
    #else
        return USB_ERROR;   // Only root hub could be successfully reset
    #endif
    }
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_DetectNewDevice
//
// Description: This function checks the port status provided and depending
//              on the status it invokes device connect/disconnect routine
//
// Output:      fpHCStruc   Pointer to HCStruc
//              bHubAddr    For root port this is the host controller index
//                          in gUsbData->aHCStrucTable combined with BIT7;
//                          For devices connected to a hub this is parent
//                          hub USB address
//              bHubPort    Parent hub port number
//              bPortStatus Port status read
//
// Output:     Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
USB_DetectNewDevice(
    HC_STRUC*   fpHCStruc,
    UINT8   bHubAddr,
    UINT8   bHubPort,
    UINT8   bPortStatus
)
{
    UINT8           bErrorFlag  = 0;
    UINT16          wDescLength;
    UINT8           bDevConfigured;
    UINTN           ConfigLevel = 0;
    UINT16          wTotalLength;
    UINT8           *fpBuffer = NULL;
    DEV_INFO        *fpDevInfo,
                    *fPointer;
    DEV_DESC        *fpDevDesc;
    CNFG_DESC       *fpCnfgDesc;
    INTRF_DESC      *fpIntrfDesc;
    UINT8           *DevMiscInfo;
	UINT8			Status;
    BOOLEAN         SkipConnectBeep = FALSE;   //(EIP64781+)
    UINT16			OrgTimeOutValue;			//(EIP75441+)
    UINT8           DeviceAddress;

    //
    // Get the temporary device info structure pointer (index 0)
    //
    fpDevInfo = gUsbData->aDevInfoTable;

    //
    // Fill the necessary entries in the device info
    //
    fpDevInfo->bFlag            = DEV_INFO_VALID_STRUC;
    fpDevInfo->bDeviceAddress   = 0;
    //fpDevInfo->wEndp0MaxPacket  = 0x40;   //(EIP98230-)
    fpDevInfo->bDeviceType      = 0;
    fpDevInfo->wIncompatFlags   = 0;
	fpDevInfo->DevMiscInfo		= NULL;

    //
    // Fill the hub/host controller information
    //
    fpDevInfo->bHubDeviceNumber = bHubAddr;
    fpDevInfo->bHubPortNumber   = bHubPort;

    //
    // Fill the device speed
    //
USB_DEBUG(DEBUG_LEVEL_3, "USB_DetectNewDevice: wPS = %x\n", bPortStatus);
    fpDevInfo->bEndpointSpeed = (bPortStatus & USB_PORT_STAT_DEV_SPEED_MASK) >>
                        USB_PORT_STAT_DEV_SPEED_MASK_SHIFT;

                                        //(EIP98145+)>
#if SHOW_SKIP_PORT_INFORMATION
{
    UINT8   i;
    DEV_INFO   						*tmpDevInfo; 
    tmpDevInfo = fpDevInfo; 	
    USB_DEBUG(3, "==== SHOW_SKIP_PORT_INFORMATION ==== \n"); 				  
    USB_DEBUG(DEBUG_LEVEL_3, "BDF %x \nRoutePath = ",fpHCStruc->wBusDevFuncNum);  
    for(i=0;i<5;i++)
    {
        if(tmpDevInfo->bHubDeviceNumber & BIT7)
        {
            USB_DEBUG(3, "\nRootPort %x \n",tmpDevInfo->bHubPortNumber); 					  
            break;
        }
        USB_DEBUG(3, "%x ",tmpDevInfo->bHubPortNumber); 				  
        tmpDevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_ADDR, 0, tmpDevInfo->bHubDeviceNumber, 0);     
        if(tmpDevInfo == NULL) break;         
    }
    USB_DEBUG(3, "==== SHOW_SKIP_PORT_INFORMATION ==== \n"); 				  
}
#endif
                                        //<(EIP98145+)
                                        //(EIP98230+)>    
    switch (fpDevInfo->bEndpointSpeed) {
        case USB_DEV_SPEED_SUPER:
            fpDevInfo->wEndp0MaxPacket = 0x200;
            break;
        case USB_DEV_SPEED_HIGH:
            fpDevInfo->wEndp0MaxPacket = 0x40;
            break;
        case USB_DEV_SPEED_FULL:
        case USB_DEV_SPEED_LOW:
            fpDevInfo->wEndp0MaxPacket = 0x08;
            break;
    }
                                        //<(EIP98230+)
    
    //
    // Fill the HC struc index value
    //
    fpDevInfo->bHCNumber = fpHCStruc->bHCNumber;

    bErrorFlag = TRUE;      // Assume as error
    bDevConfigured = FALSE;     // No device configured

    //
    // Allocate memory for device requests
    //
    ConfigLevel = USB_ERR_DEV_INIT_MEM_ALLOC;   // For proper error handling
    fpBuffer = USB_MemAlloc (GET_MEM_BLK_COUNT(MAX_CONTROL_DATA_SIZE));
    if (fpBuffer == NULL) {
		SpeakerBeep(8, 0x2000, fpHCStruc);
		return (DEV_INFO*)ConfigLevel;
    }
    ConfigLevel = USB_ERR_DEV_INIT_GET_DESC_8;

    // Initialize HC specific data before device configuration
    Status = (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDInitDeviceData)(
                        fpHCStruc, fpDevInfo, bPortStatus, &DevMiscInfo);
	if(Status != USB_SUCCESS) {
		USB_MemFree(fpBuffer, (UINT8)(MAX_CONTROL_DATA_SIZE / sizeof(MEM_BLK)));
		SpeakerBeep(8, 0x2000, fpHCStruc);
		return (DEV_INFO*)ConfigLevel;
	}
    fpDevInfo->DevMiscInfo = (VOID*)DevMiscInfo;

//
// Next send a GetDescriptor command to the device to get its Device
// Descriptor. Assume a MaxPacket size of 64 bytes (the device will use 8,
// 16, 32, or 64). Regardless of the packet size used by te device we can
// always get the real MaxPacket size that the device is using, because
// this piece of information is at offset 7 in the device descriptor.
//
	OrgTimeOutValue = gUsbData->wTimeOutValue;	//(EIP75441+)
	gUsbData->wTimeOutValue = 1000;				//(EIP75441+)

    fpDevDesc = (DEV_DESC*)USB_GetDescriptor(
                            fpHCStruc,
                            fpDevInfo,
                            fpBuffer,
                            8,
                            DESC_TYPE_DEVICE,
                            0);

	gUsbData->wTimeOutValue = OrgTimeOutValue;	//(EIP75441+)
    if(fpDevDesc == NULL) {
        goto detection_complete;
    }

    //
    // Get and store the endpoint 0 max packet size
    //
    ConfigLevel = USB_ERR_DEV_INIT_SET_ADDR;
    //
    // Endpoint 0 max packet size check.
    // CyQ've USB modem(Model:MQ4UFM560) return invalid device descriptor after 
    // warm reset.
    //
                                        //(EIP81612)>
    if (fpDevDesc->wUsbSpecVersion == 0x0300){
        fpDevInfo->wEndp0MaxPacket = (UINT16)1 << fpDevDesc->bEndp0MaxPacket;
    } else {
        fpDevInfo->wEndp0MaxPacket = (fpDevDesc->bEndp0MaxPacket)?
                (UINT16)fpDevDesc->bEndp0MaxPacket : 0x40;
    }
                                        //<(EIP81612)
                                        //(EIP73803)>
    if((fpDevInfo->bEndpointSpeed == USB_DEV_SPEED_LOW) ||
        (fpDevInfo->bEndpointSpeed == USB_DEV_SPEED_FULL)){
        FixedDelay(10 * 1000);     // 10msec delay
    }
                                        //<(EIP73803)

    //To assign an address to a USB device, the USB device transitions the state 
    //from the Default to the Address state.
    for (DeviceAddress = 1; DeviceAddress < 64; DeviceAddress++) {
        if (gUsbData->DeviceAddressMap & Shl64(1, DeviceAddress)) {
            break;
        }
    }

    if (DeviceAddress == 64) {
        goto detection_complete;
    }
    Status = USB_SetAddress(fpHCStruc, fpDevInfo, DeviceAddress);
    if (Status == USB_ERROR) {
		goto detection_complete;
    }
    gUsbData->DeviceAddressMap &= ~(Shl64(1, DeviceAddress));
    fpDevInfo->bDeviceAddress = DeviceAddress;
    FixedDelay(2 * 1000);
    
    //
    // Now send a GetDescriptor command to the device to get its device descriptor.
    //
    fpDevDesc = (DEV_DESC*)USB_GetDescriptor(
                            fpHCStruc,
                            fpDevInfo,
                            fpBuffer,
                            18,
                            DESC_TYPE_DEVICE,
                            0);

    //ASSERT(fpDevDesc != NULL);
    if (fpDevDesc == NULL) {
		goto detection_complete;
    }
    if(fpDevDesc->bDescLength != 18) {
        goto detection_complete;
    }
    if (fpDevDesc->bNumConfigs==0) fpDevDesc->bNumConfigs = 1;

    ConfigLevel = USB_ERR_DEV_INIT_GET_DESC_200;
    //
    // Get the relevant information from the descriptor and store it in
    // device information struture
    //
    fpDevInfo->wVendorId    = fpDevDesc->wVendorId;
    fpDevInfo->wDeviceId    = fpDevDesc->wDeviceId;
//
// Look at each of the device's ConfigDescriptors and InterfaceDescriptors
// until an InterfaceDescriptor is found with BaseClass, SubClass, and
// Protocol fields indicating boot keyboard, mouse, hub or storage support.
//
    fpDevInfo->bNumConfigs  = fpDevDesc->bNumConfigs;
    fpDevInfo->bConfigNum   = 0;

    do {    // For processing multiple configurations
								//(EIP70933+)>
        fpCnfgDesc = (CNFG_DESC*)USB_GetDescriptor(
                        fpHCStruc,
                        fpDevInfo,
                        fpBuffer,
                        0xFF,
                        DESC_TYPE_CONFIG,
                        fpDevInfo->bConfigNum);
		if(fpCnfgDesc == NULL) {
			break;
		}
		wTotalLength = fpCnfgDesc->wTotalLength;
        if (wTotalLength > 0xFF) {
            if(wTotalLength > (MAX_CONTROL_DATA_SIZE - 1)) {
                wTotalLength = MAX_CONTROL_DATA_SIZE - 1;
            }
            fpCnfgDesc = (CNFG_DESC*)USB_GetDescriptor(
                        fpHCStruc,
                        fpDevInfo,
                        fpBuffer,
                        wTotalLength,
                        DESC_TYPE_CONFIG,
                        fpDevInfo->bConfigNum);
								//<(EIP70933+)
            if(fpCnfgDesc == NULL) {
                break;
            }
        }
        if (fpDevInfo->bEndpointSpeed == USB_DEV_SPEED_FULL) {
            FixedDelay(100);
        }
//
// fpCnfgDesc should now point to a ConfigDescriptor.  Verify this and
// then get some fields out of it.  Then point to the next descriptor.
//
        if(fpCnfgDesc->bDescType == DESC_TYPE_CONFIG) {
            (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDEnableEndpoints)(
                        fpHCStruc, fpDevInfo, (UINT8*)fpCnfgDesc);

            //wTotalLength = fpCnfgDesc->wTotalLength; //(EIP70933-)
            wDescLength = (UINT8)fpCnfgDesc->bDescLength;
            fpDevInfo->bConfigNum = fpCnfgDesc->bConfigValue;

										//(EIP70933-)>
/*
            if(wTotalLength > (MAX_CONTROL_DATA_SIZE - 1)) {
                wTotalLength = MAX_CONTROL_DATA_SIZE - 1;
            }
*/
										//<(EIP70933-)

            for (;wDescLength < wTotalLength;) {
                //
                // fpIntrfDesc should now point to an InterfaceDescriptor.  Verify this
                // and then check its BaseClass, SubClass, and Protocol fields for
                // usable devices.
                //
                fpIntrfDesc = (INTRF_DESC*)((UINT8*)fpCnfgDesc + wDescLength);
										//(EIP59601+)>
				if ((fpIntrfDesc->bDescLength == 0) || 
					((fpIntrfDesc->bDescLength + wDescLength) > wTotalLength)) {
					break;
				}
										//<(EIP59601+)
                if ((fpIntrfDesc->bDescType == DESC_TYPE_INTERFACE) && (fpIntrfDesc->bAltSettingNum == 0)) {
                    fpDevInfo->bInterfaceNum    = fpIntrfDesc->bInterfaceNum;
                    fpDevInfo->bAltSettingNum   = 0;
                    USB_DEBUG(DEBUG_LEVEL_6, "USBIdentifyAndConfigureDevice::fpIntrfDesc %lx\n",fpIntrfDesc);

                    fPointer = USBIdentifyAndConfigureDevice(
                                    fpHCStruc,
                                    fpDevInfo,
                                    (UINT8*)fpCnfgDesc,
                                    wDescLength,
                                    wTotalLength);
                    if(fPointer != NULL) {
                        fpDevInfo = fPointer;
                        bDevConfigured = TRUE;  // At-least one device is configured
                                        //(EIP64781+)>
                        if(gUsbData->dUSBStateFlag & USB_FLAG_SKIP_CARD_READER_CONNECT_BEEP) {
                            if(fpDevInfo->bBaseClass == BASE_CLASS_MASS_STORAGE) {
                                SkipConnectBeep = TRUE;
                            }
                        }
                                        //<(EIP64781+)
                    }
															//(EIP22046+)>
					//
                    // There is one more config. Set device info structure entry 0 for it
                    //
                    if ((fpCnfgDesc->bNumInterfaces > 1) && bDevConfigured) {
						gUsbData->aDevInfoTable[0].bFlag   |= DEV_INFO_MULTI_IF;
                    }
															//<(EIP22046+)
                }
                if(fpIntrfDesc->bDescLength) {
                    wDescLength += (UINT16)fpIntrfDesc->bDescLength;
                    if(wDescLength < wTotalLength) {
															//(EIP22046-)>
                        //
                        // There is one more config. Set device info structure entry 0 for it
                        //
                        /*
                        if (fpDevInfo->bInterfaceNum > 0) {
                            fpDevInfo->bFlag |= DEV_INFO_MULTI_IF;
                        }
                        gUsbData->aDevInfoTable[0].bFlag   |= DEV_INFO_MULTI_IF;
                        */
															//<(EIP22046-)
                        fpDevInfo = gUsbData->aDevInfoTable;
                    }
                } else break;   // fpIntrfDesc->bDescLength == 0
            }   // while ()
        }   // if
        //
        // Check if we have at least one usable device
        //
        if (bDevConfigured) {
            bErrorFlag = FALSE; // Device successfully configured
            ConfigLevel = (UINTN)gUsbData->aDevInfoTable;
            goto detection_complete;
        }
        else {
            fpDevInfo->bConfigNum++;
        }
    } while (fpDevInfo->bConfigNum < fpDevInfo->bNumConfigs );  // while

detection_complete:
    //
    // At this point, if bErrorFlag is FALSE then we successfully configured
    // atleast a device.
    // If bErrorFlag is TRUE then there is error in configuring the device
    //
    if (bErrorFlag) {
        USBLogError((UINT16)ConfigLevel); // Log configuration level

        SpeakerBeep(8, 0x2000, fpHCStruc);

		(*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDDeinitDeviceData)
 						(fpHCStruc, fpDevInfo);
        if(ConfigLevel != USB_ERR_DEV_INIT_GET_DESC_8) {
            //
            // Disable the hub port
            //
            USB_DisableHubPort(
                fpHCStruc,
                fpDevInfo->bHubDeviceNumber,
                fpDevInfo->bHubPortNumber);
            ConfigLevel = 0;
        }
        if (fpDevInfo->bDeviceAddress) {
            gUsbData->DeviceAddressMap |= Shl64(1, fpDevInfo->bDeviceAddress);
        }
    }
    else {
                                        //(EIP64781+)>
        if(!SkipConnectBeep) {
            SpeakerBeep(4, 0x1000, fpHCStruc);
        }
                                        //<(EIP64781+)
    }
    USB_MemFree(fpBuffer, (UINT8)(MAX_CONTROL_DATA_SIZE / sizeof(MEM_BLK)));

    return (DEV_INFO*)ConfigLevel;

}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_StopDevice
//
// Description: This function stops the device:
//              - calls its disconnect function if available
//              - stops polling the device's interrupt endpoint
//              - updates device address memory map
//
//
// Output:      fpHCStruc   Pointer to HCStruc
//              bHCNubAddr  Hub address
//              bHCPort     Port number
//
// Output:     Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_StopDevice(
    HC_STRUC*   fpHCStruc,
    UINT8       bHubAddr,
    UINT8       bHCPort)
{
    UINT8           bCount;
    DEV_DRIVER* fpDevDriver;
    DEV_INFO*       fpDevice;
    UINT8           Status;

    Status = USB_ERROR;
    //
    // Find the device entry that would match the input.
    //
    for (bCount = 1; bCount < MAX_DEVICES; bCount++)
    {
        fpDevice = &gUsbData->aDevInfoTable[bCount];
        if((fpDevice->bFlag & (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_DUMMY)) ==
			DEV_INFO_VALID_STRUC) {
            if((fpDevice->bHubDeviceNumber == bHubAddr) &&
                                    (fpDevice->bHubPortNumber == bHCPort))
            {
                //
                // Device found - issue disconnect call for the device
                //
                if (gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI) {
                    USB_SmiQueuePut(fpDevice);
                }

                fpDevDriver = (DEV_DRIVER*)fpDevice->fpDeviceDriver;
                //
                // Check disconnect function is valid, if yes - execute it
                //
                if (fpDevDriver && fpDevDriver->pfnDisconnectDevice)
                {
                    fpDevDriver->pfnDisconnectDevice(fpDevice);
					fpDevice->fpDeviceDriver = NULL;
                } else {
					//
					// Stop polling the device's interrupt endpoint
					//
					if (fpDevice->bIntEndpoint) {
						Status = (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDDeactivatePolling)
										(fpHCStruc, fpDevice);
						fpDevice->bIntEndpoint = 0;
					}
                }

                // HC device removal call
                Status = (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDDeinitDeviceData)
                                (fpHCStruc, fpDevice);

				// Reset the disconnecting flag
				fpDevice->bFlag &= ~DEV_INFO_DEV_DISCONNECTING;

                //
                // Update Device Address Map, preserving the address for registered devices
                //
                gUsbData->DeviceAddressMap |= Shl64(1, fpDevice->bDeviceAddress);
                fpDevice->bFlag &= ~DEV_INFO_DEV_PRESENT;
                if (!(fpDevice->bFlag & (DEV_INFO_DEV_BUS | DEV_INFO_MASS_DEV_REGD))) {
					// Reset the device info structure validity ~flag
					fpDevice->bFlag &= ~DEV_INFO_VALID_STRUC;
                }
                USB_DEBUG(3, "Release Dev[%d]: %x, flag %x\n", bCount, fpDevice, fpDevice->bFlag);
            }
        }
    }
    return Status;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_DisconnectDevice
//
// Description: This function is called when a device disconnect is
//              detected. This routine disables the hub port and stops the
//              device and its children by calling another routine.
//
// Output:      fpHCStruc   Far pointer to HCStruc of the host controller
//              bHubAddr    USB device address of the hub whose status
//                          has changed
//                  bit 7   : 1 - Root hub, 0 for other hubs
//                  bit 6-0 : Device address of the hub
//              bPortNum    Port number
//
// Output:     Status: USB_SUCCESS = Success
//                      USB_ERROR = Failure
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_DisconnectDevice(
    HC_STRUC*   fpHCStruc,
    UINT8       bHubAddr,
    UINT8       bHCPort)
{
    //
    // A device has been disconnected from the USB.  First disable the hub port
    // that the device was plugged into.  Then free up the device's entry in the
    // DeviceTable.  If there an error occurs while disabling the port, assume
    // that the device is still present an leave its DeviceTable entry in place.
    //
    USB_DisableHubPort(fpHCStruc, bHubAddr, bHCPort);

    USB_StopDevice(fpHCStruc, bHubAddr, bHCPort);

    return USB_SUCCESS;

}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   USBCheckPortChange
//
// Description: This routine processes the port status change (like connect,
//              disconnect, etc.) for the root hub and external hubs.
//
// Output:      HcStruc   Pointer to Host Controller structure
//              HubAddr     Device address of the hub whose status
//                          has changed:
//                                bit 7	: 1 - Root hub, 0 for other hubs
//                                bit 6-0	: Device address of the hub
//              PortNum     Hub port number
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBCheckPortChange (
    HC_STRUC    *HcStruc,
    UINT8       HubAddr,
    UINT8       PortNum
)
{
    UINT8       PortStatus;
    DEV_INFO    *Dev;
    UINT8       Count;

    for (Count = 0; Count < 5; Count++) {
		PortStatus = USB_GetHubPortStatus(HcStruc, HubAddr, PortNum);

        //
        // Check the obtained port status
        //
        if (PortStatus == USB_ERROR) {
			return USB_ERROR;
        }
        if (!(PortStatus & USB_PORT_STAT_DEV_OWNER)) {
            return USB_SUCCESS;
        }
        if (OEMSkipList(HubAddr,PortNum,HcStruc->wBusDevFuncNum,0,0)) {
            USB_DEBUG(3, "Match the skip table ; skipping this device.\n");   //(EIP98145)
            return USB_SUCCESS;
        }
        if (!Count && !(PortStatus & USB_PORT_STAT_DEV_CONNECT_CHANGED)) {
			return USB_SUCCESS;
        }
    
        if (PortStatus & USB_PORT_STAT_DEV_CONNECTED) {
            if (gUsbData->bHandOverInProgress) {
                USB_DisableHubPort(HcStruc, HubAddr, PortNum);
                return USB_SUCCESS;
            }
			if ((Count != 0) || !(PortStatus & USB_PORT_STAT_DEV_ENABLED)) {
				// Reset and enable the port
				USB_ResetHubPort(HcStruc, HubAddr, PortNum);
				USB_EnableHubPort(HcStruc, HubAddr, PortNum);
				PortStatus = USB_GetHubPortStatus(HcStruc, HubAddr, PortNum);

                if (PortStatus == USB_ERROR) {
			        return USB_ERROR;
                }
                if (!(PortStatus & USB_PORT_STAT_DEV_OWNER)) {
                    return USB_SUCCESS;
                }
				if (!(PortStatus & USB_PORT_STAT_DEV_CONNECTED)) {
					// Some device will be disconnected after 
					// port reset, and reconnected for a while.
					FixedDelay(100 * 1000);
					continue;
				}
				// Check whether port is enabled
				if (!(PortStatus & USB_PORT_STAT_DEV_ENABLED)) {
					FixedDelay(100 * 1000);	 // 100msec delay
					continue;
				}
			}
            Dev = USB_DetectNewDevice(HcStruc, HubAddr, PortNum, PortStatus);
            if ((UINTN)Dev == USB_ERR_DEV_INIT_GET_DESC_8) {
                FixedDelay(100 * 1000);     // 100msec delay
                continue;
            }
			if ((UINTN)Dev == 0) {
				return USB_ERROR;
			}
            if ((UINTN)Dev > USB_ERR_DEV_INIT_GET_DESC_200) {
                return USB_SUCCESS;
            }
            SpeakerBeep(16, 0x4000, HcStruc);  // Issue error beep
            return USB_ERROR;
        } else {  // Disconnect
            USB_DisconnectDevice(HcStruc, HubAddr, PortNum);
            SpeakerBeep(8, 0x1000, HcStruc);
			return USB_SUCCESS;
        }
    }
    if (Count == 5) {
        USB_DisableHubPort(HcStruc, HubAddr, PortNum);
        return USB_ERROR;
    }

    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_MemAlloc
//
// Description: This routine allocates blocks of memory from the global
//              memory pool
//
// Output:      bNumBlocks  Number of 32 byte blocks needed
//
// Output:     Start offset to the allocated block (NULL on error)
//
// NOTES:       This routine allocates continuous 32 byte memory blocks.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID _FAR_ *
USB_MemAlloc(UINT16  wNumBlk)
{
    UINT8       bMemIsFound     = FALSE,
                bBitCount       = 0,
                bStart          = 0;
    UINT16      wCount;
    UINT16      Count           = 0;    // Contiguous blocks counter

    UINT16      BlkOffset       = 0,
                wMapDwordPtr;
										//(EIP89641)>
	UINT16		PageCount = 0;
	UINT16		MapDwordCount = 0;
	UINT32		BlksStsDwordsPerPage = 0;

    UINT32      dMask,
                dTemp;
#if USB_FORCE_64BIT_ALIGNMENT
    if (wNumBlk % 2) wNumBlk++;
#endif
	wCount = wNumBlk;

	BlksStsDwordsPerPage = (gUsbData->MemBlkStsBytes >> 2) / gUsbData->MemPages;

    //
    // Locate wNumBlk contiguous blocks from each memory page
    //
    for(PageCount = 0; (PageCount < gUsbData->MemPages) && !bMemIsFound; PageCount++) {

        // Do not reset the counter if the allocated blocks greater than a page.
        if (wNumBlk <= (0x1000 / sizeof(MEM_BLK))) {
		    Count = 0;  // Reset contiguous blocks counter
        }

	    for (MapDwordCount = 0; MapDwordCount < BlksStsDwordsPerPage; MapDwordCount++) {
	        //
	        // Read the next DWORD memory map data
	        //
	        wMapDwordPtr = (PageCount * BlksStsDwordsPerPage) + MapDwordCount;
        	dTemp = gUsbData->aMemBlkSts[wMapDwordPtr];

	        for (bBitCount = 0; bBitCount < 32; bBitCount++)  {
	            BlkOffset++;
	            if (dTemp & (UINT32)(1 << bBitCount))  {
	                Count++;    // Found another free block
	                if(Count == wCount) {
	                    BlkOffset = (UINT16)(BlkOffset-Count);
	                    bMemIsFound = TRUE;
	                    break;  // Found the requested number of free blocks
	                }
	            }
	            else
	            {
	                Count = 0;  // Reset contiguous blocks counter
	            }
	        }
	        if (bMemIsFound) break;
	    }
    }
										//<(EIP89641)
    if (!bMemIsFound) {
        ASSERT(FALSE);
        return NULL;
    }

//
// Change the appropriate bits in the memory map to indicate that some memory
// is being allocated
//
// At this point,
//  bBitCount points to the end of the block within DWORD
//  wMapDwordPtr points to the status dword in question

// We have to reset bCount number of bits starting from
// wMapDwordPtr[bBitCount] to wStsX[BitPosY]
// where wStsX is the status double word of the starting block,
// BitPosY is the bit position of the starting block.
//
    USB_DEBUG(DEBUG_LEVEL_4, "wMapDwordPtr = %d\n", wMapDwordPtr);
//
// Let us have a do loop to do the trick
//
    do {
        //
        // Find out how many bits we can reset in current (pointed by wMapDwordPtr)
        // double word
        //
        Count = (UINT16)((bBitCount >= (wCount-1)) ? wCount : bBitCount+1);
        //
        // Find out the starting bit offset
        //
        bStart = (UINT8)(bBitCount + 1 - Count);
        //
        // Form the 32bit mask for the AND operation
        // First prepare the bits left on the left
        //
        // Note: FFFFFFFF << 32 treated differently by different compilers; it
        // results as 0 for 16 bit compiler and FFFFFFFF for 32 bit. That's why
        // we use caution while preparing the AND mask for the memory map update.
        //
        dMask = ((Count + bStart) < 32) ? (0xFFFFFFFF << (Count + bStart)) : 0;

        //
        // Second, prepare the bits on the right
        //
        if (bStart)
        {
            dMask = dMask | ~(0xFFFFFFFF << bStart);
        }

        //
        // Reset the specified number of bits
        //
        gUsbData->aMemBlkSts[wMapDwordPtr] &= dMask;

        //
        // Update the bCount, StsWordCount & BitCount
        //
        bBitCount = 31;     // End of previous double word where we have to start
        wMapDwordPtr--;     // Previous double word
        wCount = wCount - Count;
    } while ( wCount );

    USB_DEBUG(DEBUG_LEVEL_4, "MemAlloc: %d block(s) at %x %x %x\n",
            wNumBlk,
            gUsbData->fpMemBlockStart + BlkOffset * sizeof(MEM_BLK),
            gUsbData->aMemBlkSts[0],
            gUsbData->aMemBlkSts[1]);

    return  ((VOID _FAR_ *)
        (gUsbData->fpMemBlockStart + (UINT32)BlkOffset * sizeof(MEM_BLK)));
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_MemFree
//
// Description: This routine frees the chunk of memory allocated using
//              the USBMem_Alloc call
//
// Output:      fpPtr       Pointer to the memory block to be freed
//              bNumBlocks  Number of 32 byte blocks to be freed
//
// Output:     Start offset to the allocated block (NULL on error)
//
// NOTES:       This routine frees continuous memory blocks starting
//              from the fpPtr.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_MemFree (
    VOID _FAR_ * fpPtr,
    UINT16    wNumBlk)
{
    UINT8   bOffset, bCount;
    UINT16  wBlkCount, wBlkOffset, wStsWord;

#if USB_FORCE_64BIT_ALIGNMENT
    if (wNumBlk % 2) wNumBlk++;
#endif
    wBlkCount = wNumBlk;
    wBlkOffset = 0;

    //
    // Check for pointer validity
    //
    if (fpPtr == NULL) return USB_ERROR;

    if ((fpPtr < (VOID *)gUsbData->fpMemBlockStart) ||
            (fpPtr > (VOID *)(gUsbData->fpMemBlockStart +
                            (MEM_BLK_COUNT+1)*sizeof(MEM_BLK)))) {
        return  USB_ERROR;
    }

    wBlkOffset = (UINT16)((UINTN)fpPtr - (UINTN)gUsbData->fpMemBlockStart) / sizeof (MEM_BLK);

    if (wBlkOffset >= MEM_BLK_COUNT) {
        return USB_ERROR;
    }

    wStsWord = (UINT16)(wBlkOffset >> 5);   // Divide by 32
    bOffset = (UINT8)(wBlkOffset & 0x1F);   // Mod 32
    bCount = 0;

    do {
        gUsbData->aMemBlkSts[wStsWord] |= ((UINT32)1 << (bCount + bOffset));
        wBlkCount--;
        bCount++;

        if ((bCount + bOffset) && (!((bCount + bOffset) & 0x1F))) {
            wStsWord ++;
            bCount = bOffset = 0;
        }
    } while (wBlkCount);

    USB_DEBUG(DEBUG_LEVEL_4, "MemFree: %d block(s) at %x %x %x\n",
                wNumBlk, fpPtr,
                gUsbData->aMemBlkSts[0], gUsbData->aMemBlkSts[1]);
    //
    // Pointer is valid. Fill the memory with 0's
    //
    MemSet(fpPtr, (UINT32)(wNumBlk * sizeof (MEM_BLK)), 0);

    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_InstallCallBackFunction
//
// Description: This function adds a new callback function to the globall
//              callback function list and returns the index of it.
//
// Output:      pfnCallBackFunction     Callback function address
//
// Output:     Callback function index
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_InstallCallBackFunction (CALLBACK_FUNC  pfnCallBackFunction)
{
    UINT8       bIndex;
    //
    // Check whether this function is already installed or none found
    //
    for (bIndex=0; bIndex < MAX_CALLBACK_FUNCTION; bIndex++)
    {
        //
        // Check for null entry
        //
        if (gUsbData->aCallBackFunctionTable[bIndex] == 0)
        {
            break;  // No entry found
        }

        if (gUsbData->aCallBackFunctionTable[bIndex] == pfnCallBackFunction)
        {
            return (UINT8)(bIndex+1);
        }
    }

    ASSERT(bIndex != MAX_CALLBACK_FUNCTION);
    if (bIndex == MAX_CALLBACK_FUNCTION) {
        EFI_DEADLOOP(); // Exceeding max # of callback function is illegal
    }

    //
    // Store the call back function
    //
    gUsbData->aCallBackFunctionTable[bIndex] = pfnCallBackFunction;

    return (UINT8)(bIndex+1);
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_GetDescriptor
//
// Description: This function executes a get descriptor command to the
//              given USB device and endpoint
//
// Output:      fpHCStruc   HCStruc pointer
//              fpDevInfo   Device info pointer
//              fpBuffer    Buffer to be used for the transfer
//              wLength     Size of the requested descriptor
//              bDescType   Requested descriptor type
//              bDescIndex  Descriptor index
//
// Output:     Pointer to memory buffer containing the descriptor
//              NULL on error
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8*
USB_GetDescriptor(
    HC_STRUC*   fpHCStruc,
    DEV_INFO*   fpDevInfo,
    UINT8*  fpBuffer,
    UINT16  wLength,
    UINT8   bDescType,
    UINT8   bDescIndex)
{
    UINT8           bGetDescIteration;
    UINT16          wReg,
                    wStatus;
                                        //(EIP60640)>
    for (bGetDescIteration = 0; bGetDescIteration < 5; bGetDescIteration++) {
        wReg = (UINT16)((bDescType << 8) + bDescIndex);
        wStatus = (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDControlTransfer)(
                        fpHCStruc,
                        fpDevInfo,
                        (UINT16)USB_RQ_GET_DESCRIPTOR,
                        (UINT16)0,
                        wReg,
                        fpBuffer,
                        wLength);
        if (wStatus) {
            return fpBuffer;
        }
        if (gUsbData->bLastCommandStatus & USB_TRNSFR_TIMEOUT) {
            break;
        }
        FixedDelay(10 * 1000);
    }
                                        //<(EIP60640)
    return NULL;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_SetAddress
//
// Description: This function sets the USB device address of device 0 to
//              the given value. After this call the USB device will respond
//              at its new address.
//
// Output:      fpHCStruc   Pointer to HCStruc structure
//              fpDevInfo   Pointer to device info structure
//              bNewDevAddr New device address to set
//
// Output:      USB_SUCCESS or USB_ERROR
//
// Notes:       Skip SET_ADDRESS request for XHCI controllers
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_SetAddress(
    HC_STRUC*   fpHCStruc,
    DEV_INFO*   fpDevInfo,
    UINT8       bNewDevAddr)
{
                                        //(EIP60640)>
    UINT8           SetAddressIteration;

    for (SetAddressIteration = 0; SetAddressIteration < 5; SetAddressIteration++) {
        (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDControlTransfer)(
                            fpHCStruc,
                            fpDevInfo,
                            (UINT16)USB_RQ_SET_ADDRESS,
                            0,
                            (UINT16)bNewDevAddr,
                            0,
                            0);
        if (!(gUsbData->bLastCommandStatus & USB_CONTROL_STALLED )) {
            USB_DEBUG(DEBUG_LEVEL_5, "USB_SetAddress#%d\n",bNewDevAddr);
            return USB_SUCCESS;
        }
    }
    return USB_ERROR;
                                        //<(EIP60640)
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_SetConfig
//
// Description: This function sets the device configuration.
//
// Input:       HcStruc   Pointer to HCStruc structure
//              DevInfo   Pointer to device info structure
//              ConfigNum Configuration Value
//
// Output:      USB_SUCCESS or USB_ERROR
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_SetConfig(
    HC_STRUC*   HcStruc,
    DEV_INFO*   DevInfo,
    UINT8       ConfigNum)
{
     (*gUsbData->aHCDriverTable[GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDControlTransfer)(
                    HcStruc,
                    DevInfo,
                    USB_RQ_SET_CONFIGURATION,
                    0,
                    (UINT16)ConfigNum,
                    0,
                    0);

    return USB_SUCCESS;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USBLogError
//
// Description: This routine logs the USB error in the data area. This
//              logged errors will be displayed during the POST.
//
// Output:      wErrorCode  Error code to log
//
// Output:     USB_SUCCESS or USB_ERROR
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBLogError(UINT16  wErrorCode)
{
    //
    // First check for end of the buffer
    //
    if(gUsbData->bErrorLogIndex < MAX_USB_ERRORS_NUM)
    {
        //
        // Still have space to log errors
        //
        gUsbData->aErrorLogBuffer[gUsbData->bErrorLogIndex] = wErrorCode;
        gUsbData->bErrorLogIndex++;
    }
    return USB_ERROR;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_GetDeviceInfoStruc
//
// Description: This function is used to retrieve the device info structure
//              for the particular device address & HCStruc
//
// Output:     bSearchFlag  Flag indicating search type
//                  = USB_SRCH_DEV_ADDR to search by device address and
//                      HCStruc pointer
//                  = USB_SRCH_DEV_TYPE to search by device type
//                  = USB_SRCH_HC_STRUC to search by HC struc pointer
//                  = USB_SRCH_DEV_NUM to count the number of devices connected:
//                      if fpHCStruc is not NULL - count only devices connected to
//                      certain controller, otherwise - all devices of requested
//                      type.
//                  = USB_SERCH_DEV_INDX to search by device location in the DEV_INFO:
//                     a) if fpDevInfo <> 0 return index or the fpDevInfo
//                     b) if bDevAddr <> 0 return the corresponding fpDevInfo
//                     c) if both bDevAddr <> 0 and fpDevInfo <> 0, consider a)
//
//              fpDevInfoPtr    Pointer to the device info structure from where the
//                              search begins (if 0 start from first entry)
//              bDev    Device address/drive number/device type
//              pHCStruc    Pointer to the HCStruc structure
//
// Output:     Depending on bSearchFlag this function returns:
//              - pointer to DEV_INFO structure
//              - table index
//              - number of devices
//              Function returns NULL if device is not found.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
USB_GetDeviceInfoStruc(
    UINT8       bSearchFlag,
    DEV_INFO*   fpDev_Info,
    UINT8       bDev,
    HC_STRUC*   fpHcStruc
)
{
    UINT8       Index;
    BOOLEAN     TerminateSearch = FALSE;
    UINT32      dDeviceCounter  = 0;

    if (bSearchFlag == USB_SRCH_DEV_INDX) {
        if (fpDev_Info) {
            for (Index=1; Index < MAX_DEVICES; Index++) {
                if (&gUsbData->aDevInfoTable[Index] == fpDev_Info) {
                    return (DEV_INFO*)(UINTN)Index;
                }
            }
            return NULL;    // Device address not found in the table
        }
        if (bDev == USB_HOTPLUG_FDD_ADDRESS) return &gUsbData->FddHotplugDev;
        if (bDev == USB_HOTPLUG_HDD_ADDRESS) return &gUsbData->HddHotplugDev;
        if (bDev == USB_HOTPLUG_CDROM_ADDRESS) return &gUsbData->CdromHotplugDev;

        if (bDev) return &gUsbData->aDevInfoTable[bDev];
        return NULL;        // Invalid input - both bDev and fpDevInfo are zeroes.
    }

    for (Index = 1; Index < MAX_DEVICES; Index ++) {
        //
        // if fpDev_Info is not null then position the search at the correct
        // index that matches the fpDev_Info
        //
        if (fpDev_Info) {
            if (&gUsbData->aDevInfoTable[Index] != fpDev_Info)
                continue;
            else {
                fpDev_Info = 0;
                continue;
            }
        }
        //
        // For USB_SRCH_DEVBASECLASS_NUM devices are counted regardless of their
        // DEV_INFO_VALID_STRUC flag
        //
        if (bSearchFlag == USB_SRCH_DEVBASECLASS_NUM)
        {
            if(gUsbData->aDevInfoTable[Index].bBaseClass == bDev) {
                if (fpHcStruc) {
                    //
                    // Check if device belongs to the specified HC
                    //
                    if (gUsbData->aDevInfoTable[Index].bHCNumber != fpHcStruc->bHCNumber)
                    {
                        continue;
                    }
                }
                if (gUsbData->aDevInfoTable[Index].bFlag & DEV_INFO_DEV_PRESENT)
                {
                    dDeviceCounter++;
                }
            }
            continue;
        }

        if ((gUsbData->aDevInfoTable[Index].bFlag & DEV_INFO_VALIDPRESENT) ==
            DEV_INFO_VALIDPRESENT){
            switch(bSearchFlag) {
                case  USB_SRCH_HC_STRUC:
                    if (fpHcStruc == NULL) return NULL;
                    if (gUsbData->HcTable
                        [gUsbData->aDevInfoTable[Index].bHCNumber-1] == fpHcStruc) {
                        TerminateSearch = TRUE;
                    }
                    break;

                case  USB_SRCH_DEV_TYPE:
                    if (gUsbData->aDevInfoTable[Index].bDeviceType == bDev) {
                        TerminateSearch = TRUE;
                    }
                    break;
                case  USB_SRCH_DEV_NUM:
                    if (gUsbData->aDevInfoTable[Index].bDeviceType == bDev) {
                        if (fpHcStruc) {
                            //
                            // Check if device belongs to the specified HC
                            //
                            if (gUsbData->aDevInfoTable[Index].bHCNumber != fpHcStruc->bHCNumber)
                            {
                                break;
                            }
                        }
                        dDeviceCounter++;
                    }
                    break;  // Do not change TerminateSearch so loop continues
                case  USB_SRCH_DEV_ADDR:
                    if (gUsbData->aDevInfoTable[Index].bDeviceAddress == bDev) {
                        if ((fpHcStruc == NULL) ||
                            (gUsbData->HcTable
                                [gUsbData->aDevInfoTable[Index].bHCNumber-1] == fpHcStruc)) {
                            TerminateSearch = TRUE;
                        }
                    }
                    break;

                default:
                    return NULL;
            }
        }
        if (TerminateSearch) return ((DEV_INFO*)&gUsbData->aDevInfoTable[Index]);
    }
    if ( (bSearchFlag == USB_SRCH_DEV_NUM) || (bSearchFlag == USB_SRCH_DEVBASECLASS_NUM) )
         return (DEV_INFO*)(UINTN)dDeviceCounter;

    return NULL;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// PROCEDURE:   UsbAllocDevInfo
//
// Description: Finds a non-used DEV_INFO record in aDevInfoTable and marks it
//              reserved. To free the user need to clear DEV_INFO_VALID_STRUC
//              bit in bFlag of DEV_INFO
//
// Output:     Pointer to new device info. struc. 0 on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
DEV_INFO* UsbAllocDevInfo()
{
    UINT8       bIndex;
    DEV_INFO    *fpNewDevInfo;

    for (bIndex = 1; bIndex < MAX_DEVICES; bIndex ++){
        fpNewDevInfo = gUsbData->aDevInfoTable +bIndex;
        if ((fpNewDevInfo->bFlag &
        ( DEV_INFO_VALID_STRUC | DEV_INFO_DEV_BUS)) == 0 ){
            //
            // Free device info structure. Save it if not.
            //
            fpNewDevInfo->bFlag |= DEV_INFO_VALID_STRUC |  DEV_INFO_DEV_PRESENT;
            return  fpNewDevInfo;
        }
    }
    return NULL;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBGetProperDeviceInfoStructure
//
// Description: This routine searches for a device info structure that
//              matches the vendor and device id, and LUN of the device
//              found. If such a device info structure not found, then it
//              will return a free device info structure
//
// Input:       Vendor, Device ID, Current LUN
//
// Output:     Pointer to new device info. struc. NULL on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
USBGetProperDeviceInfoStructure(
    DEV_INFO*	Dev,
    UINT8   	Lun)
{
    UINT8       bCount;
    DEV_INFO    *fpDevInfo, *fpFreeDevInfo;

    fpFreeDevInfo = NULL;

//
// Scan through the device info table for a free entry. Also if the device
// connected is a mass storage device look for a device info structure whose
// device is disconnected and its vendor & device id matches the one of
// current device. If such a structure found that means this device may be
// reconnected - use the same structure
//
    for (bCount = 1; bCount < MAX_DEVICES; bCount++)
    {
        fpDevInfo   = (DEV_INFO*) &gUsbData->aDevInfoTable[bCount];

        if (fpDevInfo->bFlag & DEV_INFO_DEV_DUMMY) {
            continue;
        }

        // Check whether the structure is valid
        if (!(fpDevInfo->bFlag & DEV_INFO_VALID_STRUC)) {    
            if (fpFreeDevInfo == NULL) {
                fpFreeDevInfo = fpDevInfo;    // Store the value of the free device info
            }
        } else {
            //
            // Yes, structure is valid. Check for device presence
            //
			if (fpDevInfo->bFlag & DEV_INFO_DEV_PRESENT) {
				if ((fpDevInfo->bHubDeviceNumber != Dev->bHubDeviceNumber) ||
					(fpDevInfo->bHubPortNumber != Dev->bHubPortNumber)) {
					continue;
				}
			}
            //
            // Device is not present. Match the vendor, device id  and LUN with
            // current device info
            //
            if ((fpDevInfo->wVendorId == Dev->wVendorId) &&
                (fpDevInfo->wDeviceId == Dev->wDeviceId) &&
                (fpDevInfo->bInterfaceNum == Dev->bInterfaceNum) &&
                (fpDevInfo->bEndpointSpeed == Dev->bEndpointSpeed) &&
                (fpDevInfo->bLUN == Lun)) {
                return fpDevInfo;   // "Abandoned" device entry found
            }
        }
    }
    return fpFreeDevInfo;
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// PROCEDURE:   USB_ConfigureDevice
//
// Description: This routine completes the USB device configuration for
//              the devices supported by USB BIOS. This routine
//              handles the generic configuration for the devices.
//
// Output:      pHCStruc    HCStruc pointer
//              pDevInfo    Device information structure pointer
//              pDesc       Pointer to the descriptor structure
//              wStart      Offset within interface descriptor
//                          supported by the device
//              wEnd        End offset of the device descriptor
//
// Output:     Pointer to new device info. struc. 0 on error
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
USB_ConfigureDevice (
    HC_STRUC*   fpHCStruc,
    DEV_INFO*   fpDevInfo,
    UINT8*      fpDesc,
    UINT16      wStart,
    UINT16      wEnd)
{
    DEV_INFO    *fpNewDevInfo;
    ENDP_DESC   *fpEndpDesc = NULL;

    fpNewDevInfo = USBGetProperDeviceInfoStructure(fpDevInfo, 0);

    if (fpNewDevInfo == NULL) {
        return NULL;
    }
    //
    // Check whether this device is reconnected by checking the valid
    // structure flag
    //
    if (fpNewDevInfo->bFlag & DEV_INFO_VALID_STRUC) {
        USB_DEBUG(DEBUG_LEVEL_3, "USB_ConfigureDevice: Existing device.\n");
        //
        // This device is reconnected. Reuse the old device address so that
        // INT13h can identify this drive properly
        //
        fpDevInfo->bFlag |= fpNewDevInfo->bFlag;
		fpNewDevInfo->wDataInSync = 0;
		fpNewDevInfo->wDataOutSync = 0;
    }
    else {
        //
        // Check whether we reached the limit of devices of this type
        //
        if (CheckDeviceLimit(fpDevInfo->bBaseClass) == TRUE) {
            return NULL;
        }
    }

    //
    // For registered devices skip updating bFlag
    //
    if (!(fpNewDevInfo->bFlag & DEV_INFO_MASS_DEV_REGD)) {
        //
        // Since DeviceInfo[0] already has many fields filled in, the new entry
        // should be initialized with a copy of DeviceInfo[0].  But, the new
        // DeviceInfo should not be  marked as "present" until the device
        // is successfully initialized.
        //
        // Copy old DeviceInfo struc to new DeviceInfo struc and zero device[0]
        //
        MemCopy ((UINT8*)fpDevInfo, (UINT8*)fpNewDevInfo, sizeof (DEV_INFO));
		fpNewDevInfo->bFlag &= DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT |
								DEV_INFO_MASS_DEV_REGD | DEV_INFO_DEV_BUS;
    } else {
        // Change the parent HC number and port number in the existing DEV_INFO
        fpNewDevInfo->bHCNumber = fpDevInfo->bHCNumber;
    	fpNewDevInfo->bHubDeviceNumber = fpDevInfo->bHubDeviceNumber;
		fpNewDevInfo->bHubPortNumber = fpDevInfo->bHubPortNumber;
		fpNewDevInfo->bEndpointSpeed = fpDevInfo->bEndpointSpeed;
		fpNewDevInfo->wEndp0MaxPacket = fpDevInfo->wEndp0MaxPacket;
    	fpNewDevInfo->DevMiscInfo = fpDevInfo->DevMiscInfo;
        fpNewDevInfo->bDeviceAddress = fpDevInfo->bDeviceAddress;
    }

    //
    // Do a SetConfiguration command to the device to set it to its
    // HID/Boot configuration.
    //
    fpNewDevInfo->bFlag |= DEV_INFO_VALIDPRESENT;
    if (!(fpDevInfo->bFlag & DEV_INFO_MULTI_IF)) {
	    USB_SetConfig(fpHCStruc, fpNewDevInfo, fpNewDevInfo->bConfigNum);
    }

    USB_DEBUG(3, "new dev: %x, flag: %x, addr %d\n",
                fpNewDevInfo, fpNewDevInfo->bFlag, fpNewDevInfo->bDeviceAddress);

    return fpNewDevInfo;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBCheckNonCompliantDevice
//
// Description: This function checks for non-compliant USB devices by
//              by comparing the device's vendor and device id with
//              the non-compliant device table list and updates the
//              data structures appropriately to support the device.
//
// Input:       fpHCStruc - HCStruc pointer
//              fpDevInfo - Device information structure pointer
//              fpDesc    - Pointer to the descriptor structure
//              wDescLength - End offset of the device descriptor
//
// Output:     Updated fpDevInfo->wIncompatFlags field
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
USBCheckNonCompliantDevice(
    HC_STRUC*   fpHCStruc,
    DEV_INFO*   fpDevInfo,
    UINT8*      fpDesc,
    UINT16      wLength,
    UINT16      wDescLength
)
{
    USB_BADDEV_STRUC    *fpBadDevice;
    INTRF_DESC          *fpIntrfDesc;

    fpIntrfDesc = (INTRF_DESC*)((UINT8*)fpDesc + wLength);

    //
    // Search the bad device table to get the structure for this device
    //
    for (fpBadDevice = gUsbBadDeviceTable;
         fpBadDevice->wDID | fpBadDevice->wVID; fpBadDevice++) {

        if ((fpBadDevice->wDID != fpDevInfo->wDeviceId) ||
                (fpBadDevice->wVID != fpDevInfo->wVendorId)) {
            continue;
        }
USB_DEBUG(DEBUG_LEVEL_3, "Found non-compatible device: DID=%x, VID=%x\n", fpBadDevice->wDID, fpBadDevice->wVID);
        //
        // Save the incompatibility flag into device info structure
        //
        fpDevInfo->wIncompatFlags = fpBadDevice->wFlags;

        //
        // Check which fields to update in the interface descriptor
        //
        // Check for base class field
        //
        if (fpBadDevice->bBaseClass) {
            //
            // Update base class field in the interface descriptor
            //
            fpIntrfDesc->bBaseClass = fpBadDevice->bBaseClass;
        }
        //
        // Check for base sub class field
        //
        if (fpBadDevice->bSubClass) {
            //
            // Update sub class field in the interface descriptor
            //
            fpIntrfDesc->bSubClass = fpBadDevice->bSubClass;
        }
        //
        // Check for protocol field
        //
        if (fpBadDevice->bProtocol) {
            //
            // Update protocol field in the interface descriptor
            //
            fpIntrfDesc->bProtocol = fpBadDevice->bProtocol;
        }
        break;
    }
}


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// PROCEDURE:   USBIdentifyAndConfigureDevice
//
// Description: This routine invokes the device drivers 'check device type'
//              routine and identifies the device type.
//
// Output:      pHCStruc    HCStruc pointer
//              pDevInfo    Device information structure pointer
//              pDesc       Pointer to the descriptor structure
//              wStart      Offset within interface descriptor
//                          supported by the device
//              wEnd        End offset of the device descriptor
//
// Output:     Pointer to new device info. struc, NULL on error
//
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
USBIdentifyAndConfigureDevice (
    HC_STRUC*   fpHCStruc,
    DEV_INFO*   fpDevInfo,
    UINT8*      fpDesc,
    UINT16      wLength,
    UINT16      wDescLength)
{
    UINT8           bBaseClass, bSubClass, bProtocol,
                    bIndex, bRetValue;
    DEV_INFO*       fpDevInfoLocal;
    DEV_DRIVER*     fpDevDriver;
    INTRF_DESC*     fpIntrfDesc;

    //
    // Check for non-compliant device. If non-compliant device found then
    // the descriptor values will get updated depending on the need.
    //
    USBCheckNonCompliantDevice (
            fpHCStruc,
            fpDevInfo,
            fpDesc,
            wLength,
            wDescLength);

    USB_DEBUG(DEBUG_LEVEL_3, "USBIdentifyAndConfigureDevice...");

    //
    // Check whether device needs to be disable
    //
    if (fpDevInfo->wIncompatFlags & USB_INCMPT_DISABLE_DEVICE) {
        USB_DEBUG(DEBUG_LEVEL_3, "not compatible device.\n");
        return NULL;
    }

    fpIntrfDesc = (INTRF_DESC*)(fpDesc + wLength);

//(EIP74609+)>
	if(OEMSkipList(fpDevInfo->bHubDeviceNumber,fpDevInfo->bHubPortNumber,fpHCStruc->wBusDevFuncNum,fpIntrfDesc->bBaseClass,1)) {
        USB_DEBUG(3, "Match the skip table ; skipping this device.\n");   //(EIP98145) 
		return NULL;
	}	
//<(EIP74609+) 
    //
    // Get the base, sub class & protocol values
    //
    bBaseClass  = fpIntrfDesc->bBaseClass;
    bSubClass   = fpIntrfDesc->bSubClass;
    bProtocol   = fpIntrfDesc->bProtocol;

    //
    // Check for matching device driver
    //
    fpDevInfoLocal = NULL;
    bRetValue = USB_ERROR;
                                        
                                        //(EIP96616+)>
    for (bIndex = 0; bIndex < MAX_DEVICE_TYPES; bIndex ++) {
        fpDevDriver = &gUsbData->aDevDriverTable[bIndex];
        //
        // Check structure validity
        //
        if (!fpDevDriver->bDevType) {
            continue;       // Driver table not valid
        }
        //
        // Verify presence of Check Device routine
        //
        if (fpDevDriver->pfnCheckDeviceType) {
            //
            // Check device type is implemented. Execute it!
            //
            bRetValue = (*fpDevDriver->pfnCheckDeviceType)(
                           fpDevInfo,bBaseClass,
                           bSubClass,bProtocol);
                if (bRetValue != USB_ERROR)
                    break;
            }  else {
            //
            // Check device type is not implemented. Compare the class codes
            //
            if((fpDevDriver->bBaseClass == bBaseClass) ||
                (fpDevDriver->bSubClass == bSubClass) ||
                (fpDevDriver->bProtocol == bProtocol)) {
                //
                // If the class codes match set bRetValue with the bDevType from the Device Driver
                //
                bRetValue = fpDevDriver->bDevType;
                break;
            }
        }
    }
    if(bRetValue != USB_ERROR){
        //
        // Check whether we reached the limit of devices of this type
        //
        //if (CheckDeviceLimit(bBaseClass) == TRUE) continue;   //(EIP81761-)

        //
        // Set the device type in the Device Info structure
        //
        fpDevInfo->bDeviceType  = bRetValue;

        //
        // Set Base Class, Subclass and Protocol information
        //
        fpDevInfo->bBaseClass = bBaseClass;
        fpDevInfo->bProtocol = bProtocol;
        fpDevInfo->bSubClass = bSubClass;

        //
        // Device identified. Issue common configure call
        // Call a common routine to handle the remaining initialization that is done
        // for all devices.
        //
        fpDevInfoLocal = USB_ConfigureDevice(
                            fpHCStruc,
                            fpDevInfo,
                            fpDesc,
                            wLength,
                            wDescLength);

        if (fpDevInfoLocal == NULL) {
            USB_DEBUG(DEBUG_LEVEL_3, "USB: Common configure failed.\n");
            return fpDevInfoLocal;
        }

        if (gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI) {
            USB_SmiQueuePut(fpDevInfoLocal);
        }

        fpDevInfoLocal->fpDeviceDriver = fpDevDriver;
        fpDevInfoLocal = (*fpDevDriver->pfnConfigureDevice)(
                                fpHCStruc,
                                fpDevInfoLocal,
                                fpDesc,
                                wLength,
                                wDescLength);
        if (!fpDevInfoLocal ||
        !(fpDevInfoLocal->bFlag & DEV_INFO_VALID_STRUC) )
        {
            fpDevInfoLocal = 0;
            USB_DEBUG(DEBUG_LEVEL_3, "USB: Device specific configure failed.\n");
            return fpDevInfoLocal;
        }

                                        //<(EIP96616+)
    }

    USB_DEBUG(DEBUG_LEVEL_3, "%x\n", fpDevInfoLocal);

    return fpDevInfoLocal;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_InitFrameList
//
// Description: This routine initializes the frame list pointed by fpPtr
//              with the dValue provided
//
// Output:      fpHCStruc   Pointer to the Host Controller structure
//              dValue  Value to be initialized with
//
// Output:     Nothing
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>


VOID
USB_InitFrameList(
    HC_STRUC*   fpHCStruc,
    UINT32      dValue)
{
    UINT16  wIndex;
    UINT32  *fpPtr = (UINT32*)fpHCStruc->fpFrameList;

    for (wIndex = 0; wIndex < fpHCStruc->wAsyncListSize; wIndex ++) {
        fpPtr[wIndex] = dValue;
    }
    return;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBKeyRepeat
//
// Description: This function handles different key repeat related functions
//              depending on the input
//
// Input:   fpHCStruc - pointer for the HC that implements the key repeat function
//          bAction   - sub-function index:
//              0  Install key repeat HCStruc
//              1  Disable key repeat
//              2  Enable key repeat
//				3  Uninstall key repeat HCStruc
//
// Output:  None
//
// Note:    fpHCStruc is only relevant for sub-function 0.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBKeyRepeat(
    HC_STRUC*   fpHCStruc,
    UINT8       bAction
)
{

//USB_DEBUG(DEBUG_LEVEL_3, "KR%d\n", bAction);
    EFI_STATUS  Status = EFI_SUCCESS;
	UINT8		i = 0;

    switch (bAction) {
        case 0:     // Sub-function 0: Save the HCStruc value for later use
        	if (gUsbData->fpKeyRepeatHCStruc == NULL) {
            	gUsbData->fpKeyRepeatHCStruc = fpHCStruc;
        	}
            break;
        case 1:     // Sub-function 0: Disable key repeat
			if (KeyRepeatStatus) {
#if USB_HID_KEYREPEAT_USE_SETIDLE == 1 
	            if(gUsbData->fpKeyRepeatDevInfo != NULL) {
	            //
	            // Set the HID SET_IDLE request to 0
	            //
	            (*gUsbData->aHCDriverTable[GET_HCD_INDEX(gUsbData->HcTable[gUsbData->fpKeyRepeatDevInfo->bHCNumber - 1].bHCType)].pfnHCDControlTransfer)
	                    (gUsbData->HcTable[gUsbData->fpKeyRepeatDevInfo->bHCNumber - 1],
	                    gUsbData->fpKeyRepeatDevInfo,(UINT16)HID_RQ_SET_IDLE, gUsbData->fpKeyRepeatDevInfo->bInterfaceNum, 0, 0, 0);	//(EIP54782)
	            }
#else
	            if (gUsbData->fpKeyRepeatHCStruc) {
	                (*gUsbData->aHCDriverTable[GET_HCD_INDEX(
	                    gUsbData->fpKeyRepeatHCStruc->bHCType)].pfnHCDDisableKeyRepeat)(
	                        gUsbData->fpKeyRepeatHCStruc);
	            }
#endif
				KeyRepeatStatus=FALSE;
			}
            break;
        case 2:     // Sub-function 0: Enable key repeat
        	if (!KeyRepeatStatus) {
#if USB_HID_KEYREPEAT_USE_SETIDLE == 1 
	            if(gUsbData->fpKeyRepeatDevInfo != NULL) {
	                //
	                // Set the HID SET_IDLE request to 0x200 (8ms)
	                //
	                (*gUsbData->aHCDriverTable[GET_HCD_INDEX(gUsbData->HcTable[gUsbData->fpKeyRepeatDevInfo->bHCNumber - 1].bHCType)].pfnHCDControlTransfer)
	                        (gUsbData->HcTable[gUsbData->fpKeyRepeatDevInfo->bHCNumber - 1],
	                        gUsbData->fpKeyRepeatDevInfo,(UINT16)HID_RQ_SET_IDLE, gUsbData->fpKeyRepeatDevInfo->bInterfaceNum, 0x400, 0, 0);	//(EIP54782)
	            }
#else 
	            if (gUsbData->fpKeyRepeatHCStruc) {
	                (*gUsbData->aHCDriverTable[GET_HCD_INDEX(
	                    gUsbData->fpKeyRepeatHCStruc->bHCType)].pfnHCDEnableKeyRepeat)(
	                        gUsbData->fpKeyRepeatHCStruc);
	            }
#endif
				KeyRepeatStatus=TRUE;
        	}
            break;
		case 3:
        	if (gUsbData->fpKeyRepeatHCStruc == fpHCStruc) {
            	gUsbData->fpKeyRepeatHCStruc = NULL;
				for (i = 0; i < gUsbData->HcTableCount; i++) {
                    if (gUsbData->HcTable[i] == NULL) continue;
					if ((gUsbData->HcTable[i]->bHCType == fpHCStruc->bHCType) && 
						(gUsbData->HcTable[i]->dHCFlag & HC_STATE_RUNNING) &&
						(fpHCStruc != gUsbData->HcTable[i])) {
						gUsbData->fpKeyRepeatHCStruc = gUsbData->HcTable[i];
                        if (KeyRepeatStatus) {
                            KeyRepeatStatus = FALSE;
                            USBKeyRepeat(NULL, 2);
                        }
                        break;
					}
				}
        	}
			break;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   BusFillDriverEntries
//
// Description: Install drivers that redirects ...????
//
// Input:   fpDevDriver - record that the routine can use to install the drive
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_bus_interrupt_handler (HC_STRUC* hc, DEV_INFO* dev, UINT8* td, UINT8* buf )
{
	USBHC_INTERRUPT_DEVNINFO_T *Idi = (USBHC_INTERRUPT_DEVNINFO_T *)dev->pExtra;
	ASSERT(Idi);
	if (Idi == NULL) {
		return USB_SUCCESS;
	}
	USB_SmiQueuePutMsg( &Idi->QCompleted, buf, (int)Idi->DataLength );
    return USB_SUCCESS;
}

DEV_INFO*
USB_on_configDev(
    HC_STRUC    *fpHCStruc,
    DEV_INFO    *fpDevInfo,
    UINT8       *fpDesc,
    UINT16      wStart,
    UINT16      wEnd
)
{
    fpDevInfo->bDeviceType      = (UINT8)BIOS_DEV_TYPE_USBBUS;
    fpDevInfo->bCallBackIndex   = USB_InstallCallBackFunction(USB_bus_interrupt_handler);
    return(fpDevInfo);
}

UINT8
USB_on_identifyDev(
    DEV_INFO*   fpDevInfo,
    UINT8       bBaseClass,
    UINT8       bSubClass,
    UINT8       bProtocol
)
{
                                        //(EIP96616+)>
    if (gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI)
        return  BIOS_DEV_TYPE_USBBUS;
    else
        return  USB_ERROR;
                                        //<(EIP96616+)
}

UINT8
USB_on_disconnectDev(
    DEV_INFO* fpDevInfo
)
{
    return  USB_SUCCESS;
}

VOID
BusFillDriverEntries(
    DEV_DRIVER  *fpDevDriver
)
{
    fpDevDriver->bDevType               = BIOS_DEV_TYPE_USBBUS;
    fpDevDriver->bBaseClass             = 0;
    fpDevDriver->bSubClass              = 0;
    fpDevDriver->bProtocol              = 0;
    fpDevDriver->pfnDeviceInit          = 0;
    fpDevDriver->pfnCheckDeviceType     = USB_on_identifyDev;
    fpDevDriver->pfnConfigureDevice     = USB_on_configDev;
    fpDevDriver->pfnDisconnectDevice    = USB_on_disconnectDev;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USB_ReConfigDevice, USB_ReConfigDevice2
//
// Description: EFI code will call this function to give a chance for
//              SMI dev driver to complete the configuration of device
//
//              Before call, USB device is connected, address is assigned
//              and configuration is set. DEV_INFO structure is initalized
//              from information parsed from descriptors and linked
//              to USBBUS dev driver. Device driver specific to the type
//              of USB device wasn't called on this device
//
//              After the call returns, a specific device driver
//              initialization was  performed by calling pfnCheckDeviceType
//              and pfnConfigureDevice functions of device driver. Parameters
//              to those functions are taken from descriptors downloaded from
//              the device. Device preserve old address and active configuration
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
int USB_ReConfigDevice2( HC_STRUC* fpHCStruc, DEV_INFO* fpDevInfo,
                        CNFG_DESC   *fpCnfgDesc,
                        INTRF_DESC * fpIntrfDesc );

int USB_ReConfigDevice( HC_STRUC* fpHCStruc, DEV_INFO* fpDevInfo )
{
    INTRF_DESC      *fpIntrfDesc=NULL;

    UINT8           iConfig;
    int             status = USB_SUCCESS;   //(EIP90124)
    UINT8*          fpBuffer;
    CNFG_DESC       *fpCnfgDesc=NULL;

	if (fpDevInfo->bDeviceType != 0 && 
		fpDevInfo->bDeviceType != BIOS_DEV_TYPE_USBBUS) {
		return USB_SUCCESS;
	}

    fpBuffer = USB_MemAlloc (GET_MEM_BLK_COUNT(MAX_CONTROL_DATA_SIZE));
    if (fpBuffer == NULL)
        return USB_ERROR;
    //
    // Find configuration desc
    //
    for(iConfig=0;iConfig<fpDevInfo->bNumConfigs;++iConfig){
        fpCnfgDesc = (CNFG_DESC*)USB_GetDescriptor(
            fpHCStruc,
            fpDevInfo,
            fpBuffer,
            (MAX_CONTROL_DATA_SIZE - 1),
            DESC_TYPE_CONFIG,
            iConfig);
        if (fpDevInfo->bEndpointSpeed == USB_DEV_SPEED_FULL) {
            FixedDelay(1000);           
        }
        if(fpCnfgDesc != NULL && fpCnfgDesc->bDescType == DESC_TYPE_CONFIG &&
            fpDevInfo->bConfigNum == fpCnfgDesc->bConfigValue ){
            break;
        }

        fpCnfgDesc = NULL;
    }

    if( fpCnfgDesc ){
        UINT16 offset;
        UINT16 wDescLength;
        INTRF_DESC      *pIntrf;

        if(fpCnfgDesc->wTotalLength > MAX_CONTROL_DATA_SIZE - 1)
            fpCnfgDesc->wTotalLength = MAX_CONTROL_DATA_SIZE - 1;
        wDescLength = fpCnfgDesc->wTotalLength;
        for(offset=(UINT16)fpCnfgDesc->bDescLength;offset <wDescLength ;offset = offset + (UINT16)pIntrf->bDescLength){
            pIntrf  = (INTRF_DESC*)((UINT8*)fpCnfgDesc + offset);
            if(pIntrf->bDescLength == 0) {
                break;
            }
            if (pIntrf->bDescType == DESC_TYPE_INTERFACE &&
                fpDevInfo->bInterfaceNum == pIntrf->bInterfaceNum &&
                fpDevInfo->bAltSettingNum == pIntrf->bAltSettingNum ) {
                fpIntrfDesc =pIntrf;
                break;
            }
        }
    }

    USB_DEBUG(DEBUG_LEVEL_3,
        "USB_reConfigDev:: CfgDsc=%x; IntrfDsc=%x\n",
        fpCnfgDesc, fpIntrfDesc);

    if (fpIntrfDesc && fpCnfgDesc) {
        status = USB_ReConfigDevice2(fpHCStruc, fpDevInfo,
                                    fpCnfgDesc, fpIntrfDesc);
    } else {
        status = USB_ERROR;
    }

    USB_MemFree(fpBuffer, (UINT8)(MAX_CONTROL_DATA_SIZE / sizeof(MEM_BLK)));

    return status;
}

//----------------------------------------------------------------------------
//  USB_ReConfigDevice2
//----------------------------------------------------------------------------
int
USB_ReConfigDevice2(
    HC_STRUC    *fpHCStruc,
    DEV_INFO    *fpDevInfo,
    CNFG_DESC   *fpCnfgDesc,
    INTRF_DESC  *fpIntrfDesc
)
{
//  int             abort=0;
    int             bIndex;
    UINT8               bRetValue = USB_ERROR;
    DEV_DRIVER          *fpDevDriver = NULL;
    DEV_INFO            *fpDevInfoLocal;
    UINT8               bBaseClass, bSubClass, bProtocol;

    USB_DEBUG(DEBUG_LEVEL_3, "USB_ReConfigDevice2.\n");

    //
    // Check for non-compliant device. If non-compliant device found then
    // the descriptor values will get updated depending on the need.
    //
    USBCheckNonCompliantDevice (
        fpHCStruc,
        fpDevInfo,
        (UINT8*)fpCnfgDesc,
        fpCnfgDesc->bDescLength,
        fpCnfgDesc->wTotalLength);

    //
    // Check whether device needs to be disable
    //
    if (fpDevInfo->wIncompatFlags & USB_INCMPT_DISABLE_DEVICE)
    {
        return USB_ERROR;
    }

    //
    // Get the base, sub class & protocol values
    //
    bBaseClass  = fpIntrfDesc->bBaseClass;
    bSubClass   = fpIntrfDesc->bSubClass;
    bProtocol   = fpIntrfDesc->bProtocol;

    //
    // Check for matching device driver
    //
    fpDevInfoLocal = NULL;
    for (bIndex = 0, bRetValue = USB_ERROR;
        bIndex < MAX_DEVICE_TYPES && bRetValue == USB_ERROR; bIndex ++) {
        fpDevDriver = &gUsbData->aDelayedDrivers[bIndex];
        if (!fpDevDriver->bDevType)
            continue;
        if (fpDevDriver->pfnCheckDeviceType){
            bRetValue = (*fpDevDriver->pfnCheckDeviceType)(
                fpDevInfo,bBaseClass,bSubClass,bProtocol);
        }else if((fpDevDriver->bBaseClass == bBaseClass) &&
                (fpDevDriver->bSubClass == bSubClass) &&
                (fpDevDriver->bProtocol == bProtocol)){
            bRetValue = fpDevDriver->bDevType;
        }
    }
    if(bRetValue == USB_ERROR)
        return bRetValue;

    //driver was found

    fpDevInfo->bDeviceType = bRetValue;
    fpDevInfo->fpDeviceDriver = fpDevDriver;
    fpDevInfoLocal = (*fpDevDriver->pfnConfigureDevice)(
        fpHCStruc,fpDevInfo,(UINT8*)fpCnfgDesc,
        (UINT16)(UINTN)((char*)fpIntrfDesc - (char*)fpCnfgDesc),fpCnfgDesc->wTotalLength);
    if (!fpDevInfoLocal)
    {
        USB_DEBUG(DEBUG_LEVEL_0, "USB_ReConfigDevice2: Device specific configure failed.\n");
        return USB_ERROR;
    }
    return USB_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   hcnum2hcstruc
//
// Description: Search for the HC_STRUC with specified bHCNumber
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

HC_STRUC*
hcnum2hcstruc(
    UINT8 bHCNumber
)
{
    return gUsbData->HcTable[bHCNumber - 1];
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   prepareForLegacyOS
//
// Description: Changes global state of USBSMI module to function properly
//              in non-EFI OS - without support from EFI drivers
//
//              Before call USB BUS is a driver that handles all devices (
//              except hub) and rest of the drivers are delayed. Number of
//              devices are supported by SUBBUS driver and custom EFI driver
//
//              After call returns, USBBUS driver is removed and all drivers
//              that where
//              delayed became active. All USBBUS devices are reconfigured.
//              Devices that are not supported by now active drivers are decon-
//              figured.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
prepareForLegacyOS()
{
                                        //(EIP96616)>
    DEV_INFO* di;
    HC_STRUC* fpHCStruc;
    DEV_INFO* e = gUsbData->aDevInfoTable + COUNTOF(gUsbData->aDevInfoTable);
    int status;


    //
    //First Reconfigure all USBBUS device (while drivers are in delayed array)
    //
    for( di = &gUsbData->aDevInfoTable[1]; di != e; ++di ){		//(EIP34448)
        if((di->bFlag & DEV_INFO_VALIDPRESENT) == DEV_INFO_VALIDPRESENT &&
            di->bDeviceType == BIOS_DEV_TYPE_USBBUS )
        {
            fpHCStruc = hcnum2hcstruc(di->bHCNumber);
            status = USB_ReConfigDevice(fpHCStruc, di );
            if(status == USB_ERROR){
                //
                // Disable the hub port; release DEV_INFO
                //
                // TODO: Either check if there is another device at the same address
                // or don't disable port
                /*USB_DisableHubPort(
                    fpHCStruc,
                    di->bHubDeviceNumber,
                    di->bHubPortNumber); */
                di->bFlag &= ~DEV_INFO_VALIDPRESENT;
            }
        }
		//di->bFlag &= ~DEV_INFO_DEV_BUS;
    }
                                        //<(EIP96616)


    USBKeyRepeat(NULL, 1);  // Disable key repeat
    //gUsbData->dUSBStateFlag |= USB_FLAG_RUNNING_UNDER_OS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USB_ResetAndReconfigDev
//
// Description: This routine resets and reconfigures the device.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
USB_ResetAndReconfigDev(
	HC_STRUC    *HcStruc,
	DEV_INFO    *DevInfo
)
{
	DEV_INFO	*Dev;
	UINT32		Status;
	UINT8		DevAddr;
	UINT8		*Buffer;
    DEV_DESC	*DevDesc;
    CNFG_DESC	*CnfgDesc;
    INTRF_DESC	*IntrfDesc;
	UINT8		ConfigIndx;
	UINT8		IntrfIndx;
	DEV_DRIVER	*DevDriver;
	UINT8		i;
    UINT8       PortStatus;
    UINT8       *DevMiscInfo;
    UINT16      TotalLength;

	for (i = 1; i < MAX_DEVICES; i++) {
		Dev = &gUsbData->aDevInfoTable[i];
		if ((Dev->bFlag & (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT | 
			DEV_INFO_DEV_DUMMY)) != (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT)) {
			continue;
		}
		if ((Dev->bHubDeviceNumber == DevInfo->bHubDeviceNumber) && 
			(Dev->bHubPortNumber == DevInfo->bHubPortNumber) &&
			(Dev->bDeviceType != BIOS_DEV_TYPE_USBBUS)) {
			(*gUsbData->aHCDriverTable[GET_HCD_INDEX(
					HcStruc->bHCType)].pfnHCDDeactivatePolling)(HcStruc, Dev);
		}
	}

    Status = (*gUsbData->aHCDriverTable[GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDDeinitDeviceData)
                                (HcStruc, DevInfo);
	if (Status != USB_SUCCESS) {
		return Status;
	}
	
	Status = USB_ResetHubPort(HcStruc, DevInfo->bHubDeviceNumber, DevInfo->bHubPortNumber);
	if (Status != USB_SUCCESS) {
		return Status;
	}

	Status = USB_EnableHubPort(HcStruc, DevInfo->bHubDeviceNumber, DevInfo->bHubPortNumber);
	if (Status != USB_SUCCESS) {
		return Status;
	}
    
    PortStatus = USB_GetHubPortStatus(HcStruc, DevInfo->bHubDeviceNumber, DevInfo->bHubPortNumber);

    if (PortStatus == USB_ERROR) {
        return USB_ERROR;
    }

    if (!(PortStatus & USB_PORT_STAT_DEV_ENABLED)) {
        return USB_ERROR;
    }

    // Initialize HC specific data before device configuration
    Status = (*gUsbData->aHCDriverTable[GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDInitDeviceData)(
                        HcStruc, DevInfo, PortStatus, &DevMiscInfo);
    if (Status != USB_SUCCESS) {
        return Status;
    }

    DevInfo->DevMiscInfo = (VOID*)DevMiscInfo;

    Buffer = USB_MemAlloc(GET_MEM_BLK_COUNT(sizeof(DEV_DESC)));
    if (Buffer == NULL) {
        return USB_ERROR;
    }

	DevAddr = DevInfo->bDeviceAddress;
	DevInfo->bDeviceAddress = 0;

	DevDesc = (DEV_DESC*)USB_GetDescriptor(HcStruc, DevInfo, Buffer, sizeof(DEV_DESC), 
				DESC_TYPE_DEVICE, 0);
	if(DevDesc == NULL) {
		USB_MemFree(Buffer, GET_MEM_BLK_COUNT(sizeof(DEV_DESC)));
		return USB_ERROR;
	}

	Status = USB_SetAddress(HcStruc, DevInfo, DevAddr);	
	if (Status != USB_SUCCESS) {
		USB_MemFree(DevDesc, GET_MEM_BLK_COUNT(sizeof(DEV_DESC)));
		return Status;
	}	

	DevInfo->bDeviceAddress = DevAddr;

    Buffer = USB_MemAlloc(GET_MEM_BLK_COUNT(MAX_CONTROL_DATA_SIZE));
    if (Buffer == NULL) {
		USB_MemFree(DevDesc, GET_MEM_BLK_COUNT(sizeof(DEV_DESC)));
        return USB_ERROR;
    }

    for (ConfigIndx = 0; ConfigIndx < DevDesc->bNumConfigs; ConfigIndx++) {
        CnfgDesc = (CNFG_DESC*)USB_GetDescriptor(HcStruc, DevInfo, Buffer, 
						0xFF, DESC_TYPE_CONFIG, ConfigIndx);
		if (CnfgDesc == NULL) {
			continue;
		}
		TotalLength = CnfgDesc->wTotalLength;
        if (TotalLength > 0xFF) {
            if (TotalLength > (MAX_CONTROL_DATA_SIZE - 1)) {
                TotalLength = MAX_CONTROL_DATA_SIZE - 1;
            }
            CnfgDesc = (CNFG_DESC*)USB_GetDescriptor(HcStruc, DevInfo, Buffer, 
						TotalLength, DESC_TYPE_CONFIG, ConfigIndx);
            if (CnfgDesc == NULL) {
                continue;
            }
        }

        if (CnfgDesc->bDescType == DESC_TYPE_CONFIG) {
            (*gUsbData->aHCDriverTable[GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDEnableEndpoints)(
                        HcStruc, DevInfo, (UINT8*)CnfgDesc);
        }

		USB_SetConfig(HcStruc, DevInfo, CnfgDesc->bConfigValue);
	
		IntrfDesc = (INTRF_DESC*)CnfgDesc;
		for (IntrfIndx = 0; IntrfIndx < CnfgDesc->bNumInterfaces; IntrfIndx++) {
			do {
				IntrfDesc = (INTRF_DESC*)((UINTN)IntrfDesc + IntrfDesc->bDescLength);
				if ((UINTN)IntrfDesc > ((UINTN)CnfgDesc + CnfgDesc->wTotalLength) ||
					(UINTN)IntrfDesc > ((UINTN)CnfgDesc + MAX_CONTROL_DATA_SIZE)) {
					break;
				}
			} while (IntrfDesc->bDescType != DESC_TYPE_INTERFACE);

			if (IntrfDesc->bDescType != DESC_TYPE_INTERFACE) {
				break;
			}

			for (i = 1; i < MAX_DEVICES; i++) {
				Dev = &gUsbData->aDevInfoTable[i];
				if ((Dev->bFlag & (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT | 
					DEV_INFO_DEV_DUMMY)) != (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT)) {
					continue;
				}
				if ((Dev->bHubDeviceNumber == DevInfo->bHubDeviceNumber) && 
					(Dev->bHubPortNumber == DevInfo->bHubPortNumber) &&
					(Dev->bConfigNum == CnfgDesc->bConfigValue) &&
					(Dev->bInterfaceNum == IntrfDesc->bInterfaceNum) &&
					(Dev->bAltSettingNum == IntrfDesc->bAltSettingNum)) {
					break;
				}
			}
			if (i == MAX_DEVICES) {
				continue;
			}

			Dev->wVendorId = DevDesc->wVendorId;
			Dev->wDeviceId = DevDesc->wDeviceId;

			if (Dev->bDeviceType != BIOS_DEV_TYPE_USBBUS) {
				DevDriver = Dev->fpDeviceDriver;
				(*DevDriver->pfnConfigureDevice)(HcStruc, Dev, (UINT8*)CnfgDesc,
	        		(UINT16)((UINTN)IntrfDesc - (UINTN)CnfgDesc), CnfgDesc->wTotalLength);
			}
		}
	}

	USB_MemFree(DevDesc, GET_MEM_BLK_COUNT(sizeof(DEV_DESC)));
	USB_MemFree(Buffer, GET_MEM_BLK_COUNT(MAX_CONTROL_DATA_SIZE));

	return USB_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USB_DevDriverDisconnect
//
// Description:  
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
USB_DevDriverDisconnect(
	HC_STRUC    *HcStruc,
	DEV_INFO    *DevInfo
)
{
	DEV_DRIVER* DevDriver;
	UINT8		Index;

	DevDriver = (DEV_DRIVER*)DevInfo->fpDeviceDriver;

	if (DevDriver && DevDriver->pfnDisconnectDevice) {
		DevDriver->pfnDisconnectDevice(DevInfo);

		DevInfo->bDeviceType = 0;
		DevInfo->fpDeviceDriver = NULL;

		for (Index = 0; Index < MAX_DEVICE_TYPES; Index++) {
			DevDriver = &gUsbData->aDevDriverTable[Index];

			if (DevDriver->bDevType == BIOS_DEV_TYPE_USBBUS) {
				DevInfo->bDeviceType = DevDriver->bDevType;
				DevDriver->pfnConfigureDevice(HcStruc, DevInfo, NULL, 0, 0);
				break;
			}
		}
	} else {
		if (DevInfo->bIntEndpoint) {
			// Stop polling the device's interrupt endpoint
			(*gUsbData->aHCDriverTable[GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDDeactivatePolling)
							(HcStruc, DevInfo);
			DevInfo->bIntEndpoint = 0;
		}
	}

	return USB_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VALID_DEVINFO
//
// Description:  Checks if DEV_INFO is a valid connected device info
//               Due to hot-plug a DEV_INFO can become invalid in the
//               midle of configuration
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
int VALID_DEVINFO(DEV_INFO* pDevInfo)
{
    return (pDevInfo->bFlag & DEV_INFO_VALIDPRESENT)!=0;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   USB_AbortConnectHubChildren
//
// Description: Mark DEV_INFO not valid for all the devices connected to a
//              given hub.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USB_AbortConnectHubChildren(
    UINT8 HubAddr
)
{
    UINT8       i;
    DEV_INFO    *Dev = &gUsbData->aDevInfoTable[1];

    for (i=1;  i<MAX_DEVICES; i++, Dev++) {
        if ((Dev->bHubDeviceNumber == HubAddr) && (Dev->bFlag & DEV_INFO_VALIDPRESENT)) {
			Dev->bFlag &= ~DEV_INFO_DEV_PRESENT;
			if (!(Dev->bFlag & DEV_INFO_MASS_DEV_REGD)) {
            	Dev->bFlag &= ~DEV_INFO_VALID_STRUC;
			}

            USB_DEBUG(DEBUG_LEVEL_3, "USB: abort device [%x] connected to hub[%x]\n",
                Dev->bDeviceAddress, HubAddr);

            if (Dev->bDeviceType == BIOS_DEV_TYPE_HUB) {
                USB_AbortConnectHubChildren(Dev->bDeviceAddress);
            }
        }
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   USB_FreeDeviceAddress
//
// Description: This routine releases the given device's address by
//              updating gUsbData->dDeviceAddressMap.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USB_FreeDeviceAddress(
    DEV_INFO    *DevInfo
)
{
    UINT8   i;
    UINT8   Found = 0;

    if (DevInfo->bDeviceAddress)
    {
        for (i=1;  i<MAX_DEVICES; i++) {
            if (gUsbData->aDevInfoTable+i != DevInfo &&
                gUsbData->aDevInfoTable[i].bDeviceAddress == DevInfo->bDeviceAddress)
            {
                Found++;
            }
        }
        if (Found == 0){
            //The DevInfo was the only function with allocated address -
            // return the address to the pool
            gUsbData->DeviceAddressMap |= Shl64(1, DevInfo->bDeviceAddress);
        }
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USB_AbortConnectDev
//
// Description: Mark DEV_INFO not valid and release its device address
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USB_AbortConnectDev(
    DEV_INFO* pDevInfo
)
{
    USB_DEBUG(DEBUG_LEVEL_3, "USB: abort connect [%x].flag = %x\n",
          pDevInfo, pDevInfo->bFlag );

	pDevInfo->bFlag &= ~DEV_INFO_DEV_PRESENT;

	if (!(pDevInfo->bFlag & DEV_INFO_MASS_DEV_REGD)) {
	    pDevInfo->bFlag &= ~DEV_INFO_VALID_STRUC;
	    if (pDevInfo->bDeviceAddress == 0) return;

	    USB_FreeDeviceAddress(pDevInfo);
	}

    // Remove children (if any) from aborted parent hub device.
    // Assume the child device has not been connected since
    // the hub has to be connected first. TODO:: Verify this assumption
    if (pDevInfo->bDeviceType == BIOS_DEV_TYPE_HUB) {
        USB_AbortConnectHubChildren(pDevInfo->bDeviceAddress);
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   USB_SmiQueuePut
//
// Description: Puts the pointer pointer into the queue for processing,
//              updates queue head and tail.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USB_SmiQueuePut(VOID * d)
{
    QUEUE_T* q = &gUsbData->QueueCnnctDisc;

    q->data[q->head++] = d;
    if(q->head==q->maxsize) q->head -= q->maxsize;
    if(q->head==q->tail){
        //Drop data from queue
        q->tail++;
        if( q->tail == q->maxsize ) q->tail -= q->maxsize;
    }
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:        QueuePutMsg
//
// Description: Add a variable size item to the queue
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USB_SmiQueuePutMsg( QUEUE_T* q, VOID * d, int sz )
{
    if(q->head + sz > q->maxsize )
        q->head = 0;
    //EfiCopyMem( (char*)q->data + q->head, d, sz );
    MemCopy((UINT8*)d, (UINT8*)((UINTN)q->data + q->head), sz);
    q->head += sz;
    if(q->head==q->maxsize) q->head = 0;
    if(q->head==q->tail){
        //Drop data from queue
        q->tail+=sz;
        if( q->tail >= q->maxsize ) q->tail = 0;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CheckDeviceLimit
//
// Description: Verifies whether the number of initialized devices of a given
//              class has reached the limit.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN
CheckDeviceLimit(
    UINT8   BaseClass
)
{
    URP_STRUC Urp;
    UINT8 DevNumber;

    Urp.bFuncNumber = USB_API_CHECK_DEVICE_PRESENCE;
    Urp.bSubFunc = 1;
    Urp.ApiData.ChkDevPrsnc.fpHCStruc = NULL;
    Urp.ApiData.ChkDevPrsnc.bDevType = BaseClass;

    USBAPI_CheckDevicePresence(&Urp);

    if (Urp.bRetValue == USB_SUCCESS)
    {
        DevNumber = Urp.ApiData.ChkDevPrsnc.bNumber;
        if ((BaseClass == BASE_CLASS_HID)
              && ((USB_DEV_HID_COUNT == 0) || (DevNumber == USB_DEV_HID_COUNT)))
        {
            USB_DEBUG(3, "Reached the limit of supported HIDs (%d); skipping this device.\n", USB_DEV_HID_COUNT);
            return TRUE;
        }

        if ((BaseClass == BASE_CLASS_HUB)
              && ((USB_DEV_HUB_COUNT == 0) || (DevNumber == USB_DEV_HUB_COUNT)))
        {
            USB_DEBUG(3, "Reached the limit of supported HUBs (%d); skipping this device.\n", USB_DEV_HUB_COUNT);
            return TRUE;
        }

        if ((BaseClass == BASE_CLASS_MASS_STORAGE)
              && ((USB_DEV_MASS_COUNT == 0) || (DevNumber == USB_DEV_MASS_COUNT)))
        {
            USB_DEBUG(3, "Reached the limit of supported Mass Storage Devices (%d); skipping this device.\n", USB_DEV_MASS_COUNT);
            return TRUE;
        }
        if ((BaseClass == BASE_CLASS_CCID_STORAGE)
              && ((USB_DEV_CCID_COUNT == 0) || (DevNumber == USB_DEV_CCID_COUNT+1)))
        {
            USB_DEBUG(3, "Reached the limit of supported CCID Devices (%d); skipping this device.\n", USB_DEV_CCID_COUNT);
            return TRUE;
        }
    }
    return FALSE;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbControlTransfer
//
// Description: 
//
// Input:  
//
// Output:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbControlTransfer(
	HC_STRUC*   HcStruc,
    DEV_INFO*	DevInfo,
	DEV_REQ		DevReq,
	UINT16		Timeout,
	VOID*		Buffer)
{
	UINT16	Status;
	UINT16  SavedTimeout;

	SavedTimeout = gUsbData->wTimeOutValue;
	gUsbData->wTimeOutValue = Timeout;

    Status = (*gUsbData->aHCDriverTable[
				GET_HCD_INDEX(HcStruc->bHCType)].pfnHCDControlTransfer)(
                HcStruc,
                DevInfo,
                DevReq.wRequestType,
                DevReq.wIndex,
                DevReq.wValue,
                Buffer,
                DevReq.wDataLength);

	gUsbData->wTimeOutValue = SavedTimeout;

	return DevReq.wDataLength && (Status == 0)? USB_ERROR : USB_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   USB_EnableEndpointsDummy
//
// Description: Dummy HC API function used by the HC drivers that do not need
//              to implement enable endpoint function.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USB_EnableEndpointsDummy (
    HC_STRUC    *HcStruc,
    DEV_INFO    *DevInfo,
    UINT8       *Desc
)
{
    return USB_SUCCESS;
}

UINT8
USB_InitDeviceDataDummy (
    HC_STRUC    *HcStruc,
    DEV_INFO    *DevInfo,
    UINT8       PortStatus,
    UINT8       **DeviceData
)
{
    *DeviceData = NULL;
    return USB_SUCCESS;
}

UINT8
USB_DeinitDeviceDataDummy (
    HC_STRUC    *HcStruc,
    DEV_INFO    *DevInfo
)
{
    return USB_SUCCESS;
}

										//(EIP54018+)>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        LocatePwrCapOffset
//
// Description: 
//  This function locate power management capability offset
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
LocatePwrCapOffset (
    IN UINT16 BusDevFunc
)
{
#if USB_RUNTIME_DRIVER_IN_SMM
    UINT16  StatusReg;
    UINT8   CapOffset;
    EFI_PCI_CAPABILITY_HDR CapHeader;

    // Check if device supports extended capabilities
    StatusReg = (UINT16)ReadPCIConfig(BusDevFunc, PCI_STATUS_REGISTER_OFFSET);
    if((StatusReg & PCI_STS_CAPABILITY) == 0) {
        return 0;
    }    
    // Get offset of first capability structure
    CapOffset = (UINT8)ReadPCIConfig(BusDevFunc, EFI_PCI_CAPABILITY_PTR);
    // Check capabilities until PMI is found or no more capabilities
    while (CapOffset) {
        CapHeader.CAP_HDR = (UINT16)ReadPCIConfig(BusDevFunc, CapOffset);
        // If PMI block, return offset 
        if(CapHeader.CapabilityID == PCI_CAP_ID_PMI) {
            return CapOffset;
        }        
        // If not, check for next offset
        CapOffset = CapHeader.NextItemPtr;
    }
#endif
    return 0;
}

#if USB_S5_WAKEUP_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        ResumePciBridge
//
// Description: 
//  This function resumed PciBridge
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
ResumePciBridge(
    UINT16      BusDevFun
)
{
    UINT32      PmStaCtlReg;
    UINT32      PmCapReg;
    UINT8       PwrCapPtr;
    UINT8       PciCommand;

    PwrCapPtr = LocatePwrCapOffset(BusDevFun);
    
    if (PwrCapPtr) {
        PmCapReg = ReadPCIConfig(BusDevFun, PwrCapPtr);
        PmStaCtlReg = ReadPCIConfig(BusDevFun, PwrCapPtr + 0x04);
        if (PmCapReg & (BIT31 | BIT30)) {
            PmStaCtlReg |= BIT8;
        }
        if (PmStaCtlReg & (BIT0 | BIT1)) {
            PmStaCtlReg &= ~(BIT0 | BIT1);
        }
        DwordWritePCIConfig(BusDevFun, PwrCapPtr + 0x04, PmStaCtlReg);
    }
    
    PciCommand = (UINT8)ReadPCIConfig(BusDevFun, PCI_COMMAND_REGISTER_OFFSET);
    PciCommand |= (PCI_CMD_MEMORY_SPACE + PCI_CMD_BUS_MASTER);
    ByteWritePCIConfig(BusDevFun, PCI_COMMAND_REGISTER_OFFSET, PciCommand);
    
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        StopPciBridge
//
// Description: 
//  This function stopped PciBridge
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
StopPciBridge(
    UINT16      BusDevFun
)
{
    UINT32      PmStaCtlReg;
    UINT8       PwrCapPtr;
    UINT8       PciCommand;
    
    PwrCapPtr = LocatePwrCapOffset(BusDevFun);
    
    if (PwrCapPtr) {
        PmStaCtlReg = ReadPCIConfig(BusDevFun, PwrCapPtr + 0x04);
        PmStaCtlReg |= (BIT0 + BIT1);
        DwordWritePCIConfig(BusDevFun, PwrCapPtr + 0x04, PmStaCtlReg);
    }
    
    PciCommand = (UINT8)ReadPCIConfig(BusDevFun, PCI_COMMAND_REGISTER_OFFSET);
    PciCommand &= (~PCI_CMD_BUS_MASTER);
    ByteWritePCIConfig(BusDevFun, PCI_COMMAND_REGISTER_OFFSET, PciCommand);
    
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        EnablePciBridge
//
// Description: 
//  This function scaned PciBridge
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
ScanPciBridge(
    UINT16      *BridgePciAddr
)
{
    HC_STRUC    *HcStruc;
    UINT8       HcBus[256];
    UINT16      HcBusIndex = 0;
    UINT16      MaxHcBus = 0;
    UINT16      i;
	UINT16	    PciAddr;
    UINT16      PciBus;
    UINT16      PciDev;
    UINT16      PciFun;
    UINT16      BridgeIndex = 255;
    UINT16      DownstreamBus;
    
    for (i = 0; i < gUsbData->HcTableCount; i++) {
        HcStruc = gUsbData->HcTable[i];
        if (HcStruc == NULL) {
            continue;
        }

        if (HcStruc->dHCFlag & HC_STATE_EXTERNAL) {
            HcBus[HcBusIndex] = (UINT8)(HcStruc->wBusDevFuncNum >> 8);
            if(MaxHcBus < HcBus[HcBusIndex]) {
                MaxHcBus = HcBus[HcBusIndex];
            }
            HcBusIndex++;
        }
    }
    
    for (PciBus = 0; PciBus  < MaxHcBus; PciBus++) {
        for (PciDev = 0; PciDev < 0x20 ; PciDev++) {
            PciAddr = (UINT16)((PciBus << 8) | (PciDev << 3) | 0);  
            if (ReadPCIConfig(PciAddr, PCI_VID) != 0xffffffff) {
                PciFun = ((UINT8)ReadPCIConfig(PciAddr, PCI_HDR) & 0x80) ? 8 : 1;
                do {
                    PciFun--;
                    PciAddr = (UINT16)((PciBus << 8) | (PciDev << 3) | PciFun);
                    if (PciFun != 0) {
                        if (ReadPCIConfig(PciAddr, PCI_VID) == 0xffffffff) {
                            continue;
                        }
                    }
                    if (ReadPCIConfig(PciAddr, PCI_SCC) == 0x0604) {
                        DownstreamBus = (UINT16)ReadPCIConfig(PciAddr, PCI_SBUS);
                        for (i = 0; i < HcBusIndex; i++) {
                            if ((HcBus[i] >= (UINT8)(DownstreamBus)) &&
                                (HcBus[i] <= (UINT8)(DownstreamBus >> 8))) {
                                ResumePciBridge(PciAddr);
                                BridgePciAddr[BridgeIndex] = PciAddr;
                                BridgeIndex--;
                                break;
                            }
                        }
                    }
                } while (PciFun);
            }
        }
    }
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Name:        UsbSuspend
//
// Description: 
//  This function suspend USB
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
UsbSuspend(
    VOID
)
{
    HC_STRUC    *HcStruc;
    DEV_INFO    *DevInfo;
    CNFG_DESC   *CnfgDesc;
    UINT8       *Buffer;
    UINT16      i;
    UINT32      PmStaCtlReg;
    UINT32      PmCapReg;
    UINT16      BridgePciAddr[256];
    UINT32      HcLowBaseAddress;
    UINT32      HcHighBaseAddress;
    UINT16      HcIoAddress;
    UINTN       EhciPciMmBase; //EIP148801 

    EhciPciMmBase = CSP_PCIE_CFG_ADDRESS (0, 29, 0, 0); //EIP148801

    for (i = 0; i < 256; i++) {
        BridgePciAddr[i] = 0;
    }

    ScanPciBridge(BridgePciAddr);

    for (i = 0; i < gUsbData->HcTableCount; i++) {
        HcStruc = gUsbData->HcTable[i];
        if (HcStruc == NULL) {
            continue;
        }

        HcStruc->PwrCapPtr = LocatePwrCapOffset(HcStruc->wBusDevFuncNum);	
        if (HcStruc->PwrCapPtr) {
            PmCapReg = ReadPCIConfig(HcStruc->wBusDevFuncNum,
                                HcStruc->PwrCapPtr);
            PmStaCtlReg = ReadPCIConfig(HcStruc->wBusDevFuncNum,
                                        HcStruc->PwrCapPtr + 0x04);
            if (PmCapReg & (BIT31 | BIT30)) {
                PmStaCtlReg |= BIT8;
            }
            if (PmStaCtlReg & (BIT0 | BIT1)) {
                PmStaCtlReg &= ~(BIT0 | BIT1);
            }
            DwordWritePCIConfig(HcStruc->wBusDevFuncNum,
                            HcStruc->PwrCapPtr + 0x04, PmStaCtlReg);
		}
        if (HcStruc->bHCType == USB_HC_UHCI) {
            HcIoAddress = (UINT16)ReadPCIConfig(HcStruc->wBusDevFuncNum, USB_IO_BASE_ADDRESS);
            if (HcStruc->BaseAddress != HcIoAddress) {
                WordWritePCIConfig(HcStruc->wBusDevFuncNum,
                        USB_IO_BASE_ADDRESS, (UINT32)HcStruc->BaseAddress);
            }
            ByteWritePCIConfig(HcStruc->wBusDevFuncNum,
	                PCI_CMD, PCI_CMD_IO_SPACE | PCI_CMD_BUS_MASTER);
        } else {       
            HcLowBaseAddress = ReadPCIConfig(HcStruc->wBusDevFuncNum, PCI_BAR0);
            if ((((UINT8)HcLowBaseAddress & (BIT1 |BIT2)) == BIT2) && ((sizeof(VOID*) / sizeof(UINT32)) == 2)) {
                HcHighBaseAddress = ReadPCIConfig(HcStruc->wBusDevFuncNum, PCI_BAR1);
                if(HcStruc->BaseAddress != ((UINTN)(HcLowBaseAddress & 0xFFFFFFF0)) +
                    (Shl64((UINTN)HcHighBaseAddress, 32))) {
                    DwordWritePCIConfig(HcStruc->wBusDevFuncNum,
                        PCI_BAR0, (UINT32)HcStruc->BaseAddress);
                    DwordWritePCIConfig(HcStruc->wBusDevFuncNum,
                        PCI_BAR1, (UINT32)(Shr64(HcStruc->BaseAddress, 32)));
                }
            } else {
                if (HcStruc->BaseAddress != (HcLowBaseAddress & 0xFFFFFFF0)) {
                    DwordWritePCIConfig(HcStruc->wBusDevFuncNum,
                        PCI_BAR0, (UINT32)HcStruc->BaseAddress);
                }
            }
		    ByteWritePCIConfig(HcStruc->wBusDevFuncNum,
	                PCI_CMD, PCI_CMD_MEMORY_SPACE | PCI_CMD_BUS_MASTER);
        }
    }

    gUsbData->bHandOverInProgress = FALSE;

    //Start XHCI
    StartControllerType(USB_HC_XHCI);
    //Start EHCI
	//EIP148801 >>
    if (MmioRead16 (EhciPciMmBase) != 0xFFFF) { 
      StartControllerType(USB_HC_EHCI);
    }
	//EIP148801 <<
    //Start UHCI
    StartControllerType(USB_HC_UHCI);
    //Start OHCI
    StartControllerType(USB_HC_OHCI);

    //Wait for the usb devices connect.
    FixedDelay(50 * 1000);
    
    USB_EnumerateRootHubPorts(USB_HC_XHCI);
	//EIP148801 >>
    if (MmioRead16 (EhciPciMmBase) != 0xFFFF) {
      USB_EnumerateRootHubPorts(USB_HC_EHCI);
    }
	//EIP148801 <<
    USB_EnumerateRootHubPorts(USB_HC_UHCI);
    USB_EnumerateRootHubPorts(USB_HC_OHCI);
 
    Buffer = USB_MemAlloc(sizeof(CNFG_DESC));
    for (i = 1; i < MAX_DEVICES; i++) {

        DevInfo = &gUsbData->aDevInfoTable[i];
        HcStruc = gUsbData->HcTable[DevInfo->bHCNumber - 1];
		
        if ((DevInfo ->bFlag & DEV_INFO_VALIDPRESENT) != DEV_INFO_VALIDPRESENT) {
			continue;
		}

        CnfgDesc = (CNFG_DESC*)USB_GetDescriptor(
                            HcStruc,
                            DevInfo,
                            Buffer,
                            sizeof(CNFG_DESC),
                            DESC_TYPE_CONFIG,
                            0);

        //Check the device have the capable of remote wakeup
        if(CnfgDesc ->bConfigFlags & BIT5) {
            //Send device wakeup command to the device
            (*gUsbData->aHCDriverTable[GET_HCD_INDEX(
                    HcStruc->bHCType)].pfnHCDControlTransfer)(
                    HcStruc,
                    DevInfo,
                    (UINT16)USB_RQ_SET_FEATURE,
                    0,
                    (UINT16)USB_FSEL_DEV_REMOTE_WAKEUP,
                    0, 0); 
        
        }

    }

    USB_MemFree(Buffer, sizeof(CNFG_DESC));

    for (i = 0; i < gUsbData->HcTableCount; i++) {
        HcStruc = gUsbData->HcTable[i];
        if (HcStruc == NULL) {
            continue;
        }

        //Global suspend host
        if (HcStruc->dHCFlag & HC_STATE_RUNNING) {
            (*gUsbData->aHCDriverTable[GET_HCD_INDEX(
                HcStruc->bHCType)].pfnHCDGlobalSuspend)(HcStruc);
            if (HcStruc->PwrCapPtr) {
                PmStaCtlReg = ReadPCIConfig(HcStruc->wBusDevFuncNum,
                                        HcStruc->PwrCapPtr + 0x04);
                PmStaCtlReg |= (BIT0 + BIT1);
                DwordWritePCIConfig(HcStruc->wBusDevFuncNum,
                            HcStruc->PwrCapPtr + 0x04, PmStaCtlReg);
            }
        }    
    }

    for (i = 0; i < 256; i++) {
        if (BridgePciAddr[i] != 0) {
            StopPciBridge(BridgePciAddr[i]);
        }
    }

    UsbSbEnablePme();

}

#endif
										//<(EIP54018+)
UINT8
UsbGetDataToggle(
	DEV_INFO	*DevInfo,
	UINT8		EndpointAddr
)
{
	DEV_INFO *DevInfoToToggle = (DevInfo->fpLUN0DevInfoPtr)? 
									DevInfo->fpLUN0DevInfoPtr : DevInfo;
	UINT8	ToggleBit = (EndpointAddr & 0xF) - 1;
	UINT16	*DataSync = EndpointAddr & BIT7 ? &DevInfoToToggle->wDataInSync : 
							&DevInfoToToggle->wDataOutSync;;
	return (UINT8)((*DataSync) >> ToggleBit) & 0x1;
}

VOID
UsbUpdateDataToggle(
	DEV_INFO	*DevInfo,
	UINT8		EndpointAddr,
	UINT8		DataToggle
)
{
	DEV_INFO *DevInfoToToggle = (DevInfo->fpLUN0DevInfoPtr)? 
									DevInfo->fpLUN0DevInfoPtr : DevInfo;
	UINT8	ToggleBit = (EndpointAddr & 0xF) - 1;
	UINT16	*DataSync = EndpointAddr & BIT7 ? &DevInfoToToggle->wDataInSync : 
							&DevInfoToToggle->wDataOutSync;;
	*DataSync &= (UINT16)~(1 << ToggleBit);
	*DataSync |= (UINT16)(DataToggle << ToggleBit);
	return;
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
