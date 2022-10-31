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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/RT/usbhub.c 31    5/03/12 6:26a Roberthsu $
//
// $Revision: 31 $
//
// $Date: 5/03/12 6:26a $
//****************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:           UsbHub.c
//
//  Description:    AMI USB Hub support implementation
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"
#include "UsbDef.h"

extern  USB_GLOBAL_DATA *gUsbData;

VOID*   USB_MemAlloc(UINT16);
UINT8   USB_MemFree(void _FAR_*, UINT16);
DEV_INFO*   USB_GetDeviceInfoStruc(UINT8, DEV_INFO*, UINT8, HC_STRUC*);
UINT8   USB_StopDevice (HC_STRUC*,  UINT8, UINT8);
VOID    FixedDelay(UINTN);
UINT8   USB_ProcessPortChange (HC_STRUC*, UINT8, UINT8, UINT8);
UINT8   USB_InstallCallBackFunction (CALLBACK_FUNC  pfnCallBackFunction);
UINT8   USBCheckPortChange (HC_STRUC*, UINT8, UINT8);
UINT8	UsbControlTransfer(HC_STRUC*, DEV_INFO*, DEV_REQ, UINT16, VOID*);

UINT8   USBHUBDisconnectDevice (DEV_INFO*);
UINT8   USBHub_EnablePort(HC_STRUC*, UINT8, UINT8);
UINT8   USBHub_DisablePort(HC_STRUC*, UINT8, UINT8);
UINT8   USBHub_ResetPort(HC_STRUC*, UINT8, UINT8);

UINT8   USBHubCheckDeviceType (DEV_INFO*, UINT8, UINT8, UINT8);
UINT8   USBHub_ProcessHubData(HC_STRUC*, DEV_INFO*, UINT8*, UINT8*);
DEV_INFO*   USBHUBConfigureDevice (HC_STRUC*, DEV_INFO*, UINT8*, UINT16, UINT16);
UINT8	UsbHubResetPort(HC_STRUC*, DEV_INFO*, UINT8, BOOLEAN);

UINT8	UsbHubCearHubFeature(HC_STRUC*, DEV_INFO*, HUB_FEATURE);
UINT8	UsbHubClearPortFeature(HC_STRUC*, DEV_INFO*, UINT8, HUB_FEATURE);
UINT8	UsbHubGetHubDescriptor(HC_STRUC*, DEV_INFO*, VOID*, UINT16);
UINT8	UsbHubGetHubStatus(HC_STRUC*, DEV_INFO*, UINT32*);
UINT8	UsbHubGetPortStatus(HC_STRUC*, DEV_INFO*, UINT8, UINT32*);
UINT8	UsbHubGetErrorCount(HC_STRUC*, DEV_INFO*, UINT8, UINT16*);
UINT8	UsbHubSetHubDescriptor(HC_STRUC*, DEV_INFO*, VOID*, UINT16);
UINT8	UsbHubSetHubFeature(HC_STRUC*, DEV_INFO*, HUB_FEATURE);
UINT8	UsbHubSetHubDepth(HC_STRUC*, DEV_INFO*, UINT16);
UINT8	UsbHubSetPortFeature(HC_STRUC*, DEV_INFO*, UINT8, HUB_FEATURE);

PUBLIC
void
USBHubFillDriverEntries (DEV_DRIVER *fpDevDriver)
{
    fpDevDriver->bDevType               = BIOS_DEV_TYPE_HUB;
//  fpDevDriver->bBaseClass             = BASE_CLASS_HUB;
//  fpDevDriver->bSubClass              = SUB_CLASS_HUB;
    fpDevDriver->bProtocol              = 0;
    fpDevDriver->pfnDeviceInit          = 0;
    fpDevDriver->pfnCheckDeviceType     = USBHubCheckDeviceType;
    fpDevDriver->pfnConfigureDevice     = USBHUBConfigureDevice;
    fpDevDriver->pfnDisconnectDevice    = USBHUBDisconnectDevice;
}



//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBHUBConfigureDevice
//
// Description: This function checks an interface descriptor of a device
//              to see if it describes a USB hub.  If the device is a hub,
//              then it is configured and initialized.
//
// Input:   pHCStruc    HCStruc pointer
//          pDevInfo    Device information structure pointer
//          pDesc       Pointer to the descriptor structure
//                      supported by the device
//          wStart      Start offset of the device descriptor
//          wEnd        End offset of the device descriptor
//
// Output:  New device info structure, 0 on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
USBHUBConfigureDevice (
    HC_STRUC*   fpHCStruc,
    DEV_INFO*   fpDevInfo,
    UINT8*      fpDesc,
    UINT16  wStart,
    UINT16  wEnd)
{
    UINT8           bPortNum;
    UINTN          DelayValue;
    UINT8*          fpBuffer;
    HUB_DESC        *fpHubDesc;
    UINT8           Status;
	DEV_INFO*		ParentHub;
	BOOLEAN			SetPortPower = FALSE;
    ENDP_DESC       *fpEndpDesc;
    INTRF_DESC      *fpIntrfDesc;

	USB3_HUB_PORT_STATUS*	Usb3HubPortSts = (USB3_HUB_PORT_STATUS*)&gUsbData->dHubPortStatus;	


    //
    // Set the BiosDeviceType field in DeviceTableEntry[0].  This serves as a flag
    // that indicates a usable interface has been found in the current
    // configuration. This is needed so we can check for other usable interfaces
    // in the current configuration (i.e. composite device), but not try to search
    // in other configurations.
    //
    fpDevInfo->bDeviceType = BIOS_DEV_TYPE_HUB;
    fpDevInfo->bCallBackIndex = USB_InstallCallBackFunction(USBHub_ProcessHubData);

    fpIntrfDesc = (INTRF_DESC*)(fpDesc + wStart);
    fpDesc+=((CNFG_DESC*)fpDesc)->wTotalLength; // Calculate the end of descriptor block
    fpEndpDesc = (ENDP_DESC*)((char*)fpIntrfDesc + fpIntrfDesc->bDescLength);

    for( ;(fpEndpDesc->bDescType != DESC_TYPE_INTERFACE) && ((UINT8*)fpEndpDesc < fpDesc);
        fpEndpDesc = (ENDP_DESC*)((UINT8 *)fpEndpDesc + fpEndpDesc->bDescLength)){

        if(!(fpEndpDesc->bDescLength)) {
            break;  // Br if 0 length desc (should never happen, but...)
        }

        if( fpEndpDesc->bDescType != DESC_TYPE_ENDPOINT ) {
            continue;
        }

        //
        // Check for and configure Interrupt endpoint if present
        //
        if ((fpEndpDesc->bEndpointFlags & EP_DESC_FLAG_TYPE_BITS) !=
                EP_DESC_FLAG_TYPE_INT) {    // Bit 1-0: 10 = Endpoint does interrupt transfers
			continue;
        }

		if (fpEndpDesc->bEndpointAddr & EP_DESC_ADDR_DIR_BIT) {
			fpDevInfo->bIntEndpoint = fpEndpDesc->bEndpointAddr;
			fpDevInfo->wIntMaxPkt = fpEndpDesc->wMaxPacketSize;
			fpDevInfo->bPollInterval = fpEndpDesc->bPollInterval;
			break;
		}
    }

    if ((fpHCStruc->dHCFlag & HC_STATE_CONTROLLER_WITH_RMH) &&
        (fpDevInfo->bHubDeviceNumber & BIT7)) {
        fpDevInfo->wIncompatFlags |= USB_INCMPT_RMH_DEVICE;
    }

	fpDevInfo->HubDepth = 0;
	ParentHub = USB_GetDeviceInfoStruc(USB_SRCH_DEV_ADDR, 
						NULL, fpDevInfo->bHubDeviceNumber, NULL);
	if(ParentHub) {
		fpDevInfo->HubDepth = ParentHub->HubDepth + 1;
	}
	
	if(fpDevInfo->bEndpointSpeed == USB_DEV_SPEED_SUPER) {
		UsbHubSetHubDepth(fpHCStruc, fpDevInfo, fpDevInfo->HubDepth);
	}

    //
    // Allocate memory for getting hub descriptor
    //
    fpBuffer    = USB_MemAlloc(sizeof(MEM_BLK));
    if (!fpBuffer) {
		//USB_AbortConnectDev(fpDevInfo);   //(EIP59579-)
		return NULL;
    }

	Status = UsbHubGetHubDescriptor(fpHCStruc, fpDevInfo, fpBuffer, sizeof(MEM_BLK));
    if(Status != USB_SUCCESS) {    // Error
        USB_MemFree(fpBuffer, sizeof(MEM_BLK));
        //USB_AbortConnectDev(fpDevInfo);   //(EIP59579-)
		return NULL;
    }
    fpHubDesc                   = (HUB_DESC*)fpBuffer;
    fpDevInfo->bHubNumPorts     = fpHubDesc->bNumPorts;
    fpDevInfo->bHubPowerOnDelay = fpHubDesc->bPowerOnDelay; // Hub's ports have not been enumerated

    //
    // Turn on power to all of the hub's ports by setting its port power features.
    // This is needed because hubs cannot detect a device attach until port power
    // is turned on.
    //
    for (bPortNum = 1; bPortNum <= fpDevInfo->bHubNumPorts; bPortNum++)
    {
        if (fpDevInfo->wIncompatFlags & USB_INCMPT_RMH_DEVICE &&
            bPortNum == fpHCStruc->DebugPort)
        {
            continue;
        }

		if (fpDevInfo->bEndpointSpeed == USB_DEV_SPEED_SUPER) {
			UsbHubGetPortStatus(fpHCStruc, fpDevInfo, bPortNum, &gUsbData->dHubPortStatus);
			if (Usb3HubPortSts->PortStatus.Power == 1) {
				continue;
			}
		}

		UsbHubSetPortFeature(fpHCStruc, fpDevInfo, bPortNum, PortPower);
		SetPortPower = TRUE;
    }

    //
    // Delay the amount of time specified in the PowerOnDelay field of
    // the hub descriptor: in ms, add 30 ms to the normal time and multiply
    // by 64 (in 15us).
    //
    if(SetPortPower) {
        if (!(fpDevInfo->wIncompatFlags & USB_INCMPT_RMH_DEVICE)) {
	        if (gUsbData->PowerGoodDeviceDelay == 0) {
	            DelayValue = (UINTN)fpDevInfo->bHubPowerOnDelay * 2 * 1000;  // "Auto"
	        } else {
	            DelayValue = (UINTN)gUsbData->PowerGoodDeviceDelay * 1000* 1000;  // convert sec->15 mcs units
	        }
	        FixedDelay(DelayValue);
        }
    }

    fpDevInfo->fpPollTDPtr  = 0;
    fpDevInfo->fpPollEDPtr  = 0;

    //
    // Free the allocated buffer
    //
    USB_MemFree(fpBuffer, sizeof(MEM_BLK));

    fpDevInfo->HubPortConnectMap = 0;

    //
    // Check for new devices behind the hub
    //
    for (bPortNum = 1; bPortNum <= fpDevInfo->bHubNumPorts; bPortNum++) {
        USBCheckPortChange(fpHCStruc, fpDevInfo->bDeviceAddress, bPortNum);
    }

    // Start polling the new device's interrupt endpoint.
    (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDActivatePolling)
                (fpHCStruc, fpDevInfo);

    return fpDevInfo;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBHubDisconnect
//
// Description: This routine disconnects the hub by disconnecting all the
//      devices behind it
//
// Input:   pDevInfo    Device info structure pointer
//
// Output:  Nothing
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBHUBDisconnectDevice (DEV_INFO*   fpDevInfo)
{
    HC_STRUC* fpHCStruc = gUsbData->HcTable[fpDevInfo->bHCNumber - 1];
    UINT8 bPort;

	// Stop polling the endpoint
	(*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDDeactivatePolling)(fpHCStruc,fpDevInfo);
	fpDevInfo->bIntEndpoint = 0;

    //
    // A hub device is being disconnected.  For each of the hub's ports disconnect
    // any child device connected.
    //
    fpHCStruc = gUsbData->HcTable[fpDevInfo->bHCNumber - 1];

    for (bPort = 1; bPort <= (UINT8)fpDevInfo->bHubNumPorts; bPort++)
    {
        if (fpDevInfo->wIncompatFlags & USB_INCMPT_RMH_DEVICE &&
            bPort == fpHCStruc->DebugPort)
        {
            continue;
        }

        USB_StopDevice (fpHCStruc,  fpDevInfo->bDeviceAddress, bPort);
    }

    return USB_SUCCESS;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBHubCheckDeviceType
//
// Description: This routine checks for hub type device from the
//      interface data provided
//
// Input:   bBaseClass  USB base class code
//          bSubClass   USB sub-class code
//          bProtocol   USB protocol code
//
// Output:  BIOS_DEV_TYPE_HUB type on success or 0FFH on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBHubCheckDeviceType(
    DEV_INFO* fpDevInfo,
    UINT8 bBaseClass,
    UINT8 bSubClass,
    UINT8 bProtocol)
{
    if(bBaseClass == BASE_CLASS_HUB)
        return BIOS_DEV_TYPE_HUB;
    else
        return(0xFF);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBHub_GetPortStatus
//
// Description: This routine returns the hub port status
//
// Input:   fpDevInfo   USB device - the hub whose status has changed
//              bit 7   : 1 - Root hub, 0 for other hubs
//              bit 6-0 : Device address of the hub
//      bPortNum    Port number
//      pHCStruc    HCStruc of the host controller
//
// Output:  Port status flags (Refer USB_PORT_STAT_XX equates)
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBHub_GetPortStatus (
    HC_STRUC*   HcStruc,
    UINT8       HubAddr,
    UINT8       PortNum
)
{
    UINT8       PortSts = USB_PORT_STAT_DEV_OWNER;
	UINT8		Status;
    DEV_INFO    *DevInfo;
	HUB_FEATURE	Feature;
	UINT16		PortChange;
	UINT8		i = 0;

	HUB_PORT_STATUS*		HubPortSts = (HUB_PORT_STATUS*)&gUsbData->dHubPortStatus;
	USB3_HUB_PORT_STATUS*	Usb3HubPortSts = (USB3_HUB_PORT_STATUS*)&gUsbData->dHubPortStatus;	

    DevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_ADDR, NULL, HubAddr, HcStruc);
    ASSERT(DevInfo);
	if (DevInfo == NULL) {
        return 0;
    }

    if (DevInfo->wIncompatFlags & USB_INCMPT_RMH_DEVICE &&
		PortNum == HcStruc->DebugPort){
        return 0;
    }

    Status = UsbHubGetPortStatus(HcStruc, DevInfo, PortNum, &gUsbData->dHubPortStatus);
    if (Status == USB_ERROR) {
        return USB_ERROR;
    }

	USB_DEBUG(DEBUG_LEVEL_3, "Hub port[%d] status: %08x\n", PortNum, gUsbData->dHubPortStatus);

	if (DevInfo->bEndpointSpeed == USB_DEV_SPEED_SUPER) {
		for (i = 0; i < 20; i++) {
			if (Usb3HubPortSts->PortStatus.Reset == 0) {
                break;
            }
			FixedDelay(10 * 1000);	   // 10ms
			UsbHubGetPortStatus(HcStruc, DevInfo, PortNum, &gUsbData->dHubPortStatus);
		}

		switch (Usb3HubPortSts->PortStatus.LinkState) {
			case USB3_HUB_PORT_LINK_U0:
			case USB3_HUB_PORT_LINK_RXDETECT:
				break;
            case USB3_HUB_PORT_LINK_RECOVERY:
                for (i = 0; i < 20; i++) {
				    FixedDelay(10 * 1000);
                    UsbHubGetPortStatus(HcStruc, DevInfo, PortNum, &gUsbData->dHubPortStatus);
                    if (Usb3HubPortSts->PortStatus.LinkState != USB3_HUB_PORT_LINK_RECOVERY) {
                        break;
                    }
                }
			    break;
			case USB3_HUB_PORT_LINK_POLLING:
			    for (i = 0; i < 50; i++) {
                    FixedDelay(10 * 1000);
                    UsbHubGetPortStatus(HcStruc, DevInfo, PortNum, &gUsbData->dHubPortStatus);
                    if (Usb3HubPortSts->PortStatus.LinkState != USB3_HUB_PORT_LINK_POLLING) {
                        break;
                    }
                }
                if (Usb3HubPortSts->PortStatus.LinkState == USB3_HUB_PORT_LINK_U0 || 
                    Usb3HubPortSts->PortStatus.LinkState == USB3_HUB_PORT_LINK_RXDETECT) {
                    break;
			    }
			case USB3_HUB_PORT_LINK_COMPLIANCE_MODE:
				UsbHubResetPort(HcStruc, DevInfo, PortNum, TRUE);
				break;
			default:
				PortSts |= USB_PORT_STAT_DEV_CONNECTED;
				break;
		}
		if (Usb3HubPortSts->PortChange.ConnectChange) {
			PortSts |= USB_PORT_STAT_DEV_CONNECT_CHANGED;
            DevInfo->HubPortConnectMap &= (UINT16) (~(1 << PortNum));
			//UsbHubClearPortFeature(HcStruc, DevInfo, PortNum, PortConnectChange);
		}
		if (Usb3HubPortSts->PortStatus.Connected) {
            DevInfo->HubPortConnectMap |= (UINT16) (1 << PortNum);
			PortSts |= USB_PORT_STAT_DEV_CONNECTED | USB_PORT_STAT_DEV_SUPERSPEED;
	
			// USB 3.0 hub may not set Connect Status Change bit after reboot,
			// set the connect change flag if the BH Reset change is set
			if (Usb3HubPortSts->PortChange.BhResetChange) {
				PortSts |= USB_PORT_STAT_DEV_CONNECT_CHANGED;
			}
			if (Usb3HubPortSts->PortStatus.Enabled) {
				PortSts |= USB_PORT_STAT_DEV_ENABLED;
			}
		}
	} else {
		if (HubPortSts->PortChange.ConnectChange) {
			PortSts |= USB_PORT_STAT_DEV_CONNECT_CHANGED;
			//UsbHubClearPortFeature(HcStruc, DevInfo, PortNum, PortConnectChange);
            DevInfo->HubPortConnectMap &= (UINT16) (~(1 << PortNum));
			//if(HubPortSts->PortStatus.Connected) {
				// Delay for 100ms allowing power to settle.
				//FixedDelay(gUsbData->UsbTimingPolicy.HubPortConnect * 1000);	   // 50ms
			//}
		}
		if (HubPortSts->PortStatus.Connected) {
			PortSts |= USB_PORT_STAT_DEV_CONNECTED;
            DevInfo->HubPortConnectMap |= (UINT16) (1 << PortNum);
			if (HubPortSts->PortStatus.LowSpeed) {
				PortSts |= USB_PORT_STAT_DEV_LOWSPEED;
			} else if (HubPortSts->PortStatus.HighSpeed) {
				PortSts |= USB_PORT_STAT_DEV_HISPEED;
			} else {
				PortSts |= USB_PORT_STAT_DEV_FULLSPEED;
			}
			if (HubPortSts->PortStatus.Enabled) {
				PortSts |= USB_PORT_STAT_DEV_ENABLED;
			}
		}
	}
	
	// Clear all port status change
	//UsbHubGetPortStatus(HcStruc, DevInfo, PortNum, &gUsbData->dHubPortStatus);

	PortChange = (*((UINT16*)&HubPortSts->PortChange));
	for (Feature = PortConnectChange; Feature <= PortResetChange; Feature++) {
		if (PortChange & 1) {
			UsbHubClearPortFeature(HcStruc, DevInfo, PortNum, Feature);
		}
		PortChange >>= 1;
	}

	if (DevInfo->bEndpointSpeed == USB_DEV_SPEED_SUPER) {
		if (Usb3HubPortSts->PortChange.LinkStateChange) {
			UsbHubClearPortFeature(HcStruc, DevInfo, PortNum, PortLinkStateChange);
		}
		if (Usb3HubPortSts->PortChange.ConfigErrorChange) {
			UsbHubClearPortFeature(HcStruc, DevInfo, PortNum, PortConfigErrorChange);
		}
		if (Usb3HubPortSts->PortChange.BhResetChange) {
			UsbHubClearPortFeature(HcStruc, DevInfo, PortNum, BhPortResetChange);
		}
	}

    return PortSts;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBHub_DisablePort
//
// Description: This routine disables the hub port
//
// Input:   bHubAddr    USB device address of the hub whose status
//              has changed
//              bit 7   : 1 - Root hub, 0 for other hubs
//              bit 6-0 : Device address of the hub
//      bPortNum    Port number
//      pHCStruc    HCStruc of the host controller
//
// Output:  USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBHub_DisablePort(
    HC_STRUC*   fpHCStruc,
    UINT8       bHubAddr,
    UINT8       bPortNum)
{
    DEV_INFO*   fpDevInfo;

	HUB_PORT_STATUS*	HubPortSts = (HUB_PORT_STATUS*)&gUsbData->dHubPortStatus;
	
    //
    // Get DeviceInfo pointer
    //
    fpDevInfo   = USB_GetDeviceInfoStruc(USB_SRCH_DEV_ADDR,
                            (DEV_INFO*)0,
                            bHubAddr,
                            fpHCStruc);
//
// Disable the hub/port by clearing its Enable feature
//
    if (fpDevInfo->wIncompatFlags & USB_INCMPT_RMH_DEVICE &&
            bPortNum == fpHCStruc->DebugPort)
    {
        return USB_SUCCESS;
    }

	if(fpDevInfo->bEndpointSpeed == USB_DEV_SPEED_SUPER) return USB_SUCCESS;

	UsbHubGetPortStatus(fpHCStruc, fpDevInfo, bPortNum, &gUsbData->dHubPortStatus);

    // Perform control transfer with device request as HUB_RQ_CLEAR_PORT_FEATURE,
    // wIndex = Port number, wValue = HUB_FEATURE_PORT_ENABLE,
    // fpBuffer = 0 and wlength = 0
    //
    if(HubPortSts->PortStatus.Enabled) {
	    UsbHubClearPortFeature(fpHCStruc, fpDevInfo, bPortNum, PortEnable);
    }

    return USB_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBHub_EnablePort
//
// Description: This routine enables the hub port
//
// Input:   bHubAddr    USB device address of the hub whose status
//              has changed
//              bit 7   : 1 - Root hub, 0 for other hubs
//              bit 6-0 : Device address of the hub
//      bPortNum    Port number
//      pHCStruc    HCStruc of the host controller
//
// Output:  USB_SUCCESS if the hub port is enabled. USB_ERROR otherwise
//
// Modified:    Nothing
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBHub_EnablePort(
    HC_STRUC*   fpHCStruc,
    UINT8       bHubAddr,
    UINT8       bPortNum)
{
    return USB_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBHub_ResetPort
//
// Description: This routine resets the hub port
//
// Input:   HCStruc    HCStruc of the host controller
//			HubAddr    USB device address of the hub whose status
//              		has changed
//              bit 7   : 1 - Root hub, 0 for other hubs
//              bit 6-0 : Device address of the hub
//      	PortNum    Port number
//
// Output:  USB_SUCCESS if the hub port is enabled. USB_ERROR otherwise
//
// Modified:    Nothing
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBHub_ResetPort(
    HC_STRUC*   HcStruc,
    UINT8       HubAddr,
    UINT8       PortNum)
{
	UINT8		Status;
    DEV_INFO*   DevInfo;

    //
    // Get DeviceInfo pointer
    //
    DevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_ADDR, 
    						(DEV_INFO*)0, HubAddr, HcStruc);
    if (DevInfo == NULL) return USB_ERROR;

    if ((DevInfo->wIncompatFlags & USB_INCMPT_RMH_DEVICE) && 
		(PortNum == HcStruc->DebugPort)) {
        return USB_SUCCESS;
    }
	Status = UsbHubResetPort(HcStruc, DevInfo, PortNum, FALSE);

    return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBHub_ProcessHubData
//
// Description: This routine is called with USB hub status change
//      report data
//
// Input:   pHCStruc    Pointer to HCStruc
//          pDevInfo    Pointer to device information structure
//          pTD         Pointer to the polling TD
//          pBuffer     Pointer to the data buffer
//
//
// Notes:   The status change data is an array of bit flags:
//          Bit     Description
//      ----------------------------------------------------------
//          0   Indicate connect change status for all ports
//          1   Indicate connect change status for port 1
//          2   Indicate connect change status for port 2
//          ...     ..............
//          n   Indicate connect change status for port n
//      -----------------------------------------------------------
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBHub_ProcessHubData (
    HC_STRUC* fpHCStruc,
    DEV_INFO* fpDevInfo,
    UINT8* fpTD,
    UINT8* fpBuffer)
{
    UINT8       PortNum;
    UINT16      PortMap;
    BOOLEAN     ConnectDelay = FALSE;

    USB_DEBUG(DEBUG_LEVEL_3, "USBHub_ProcessHubData, gUsbData->bEnumFlag = %d\n", gUsbData->bEnumFlag);
    //
    // Check for enum flag and avoid hub port enumeration if needed
    //
    if (gUsbData->bEnumFlag == TRUE) return USB_SUCCESS;
    
    for (PortNum = 1; PortNum <= fpDevInfo->bHubNumPorts; PortNum++) {
        PortMap = (UINT16)(1 << PortNum);
        if (*(UINT16*)fpBuffer & PortMap) {
            if (!ConnectDelay && ((~fpDevInfo->HubPortConnectMap) & PortMap)) {
                //Delay for 50 ms allowing port to settle.
                FixedDelay(50 * 1000);
                ConnectDelay = TRUE;
            }
            //
            // Set enumeration flag so that another device will not get enabled
            //
            gUsbData->bEnumFlag = TRUE;

            USBCheckPortChange(fpHCStruc, fpDevInfo->bDeviceAddress, PortNum);

            //
            // Reset enumeration flag so that other devices can be enumerated
            //
            gUsbData->bEnumFlag = FALSE;
        }
    }
	return USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbHubResetPort
//
// Description: 
//
// Input:   
//
// Output: USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHubResetPort(
	HC_STRUC*   HcStruc,
    DEV_INFO*   DevInfo,
    UINT8		Port,
    BOOLEAN		WarmReset)
{
	UINT8		Status;
	UINT8		i;
	BOOLEAN		IsResetChange;

	HUB_PORT_STATUS*		HubPortSts = (HUB_PORT_STATUS*)&gUsbData->dHubPortStatus;
	USB3_HUB_PORT_STATUS*	Usb3HubPortSts = (USB3_HUB_PORT_STATUS*)&gUsbData->dHubPortStatus;	

	if (WarmReset && DevInfo->bEndpointSpeed == USB_DEV_SPEED_SUPER) {
		Status = UsbHubSetPortFeature(HcStruc, DevInfo, Port, BhPortReset);
		if(Status != USB_SUCCESS) return USB_ERROR;
		
		for(i = 0; i < 10; i++) {
			FixedDelay(10 * 1000);
			Status = UsbHubGetPortStatus(HcStruc, DevInfo, Port, &gUsbData->dHubPortStatus);
			if(Status != USB_SUCCESS) return USB_ERROR;
		
			if(Usb3HubPortSts->PortChange.BhResetChange) break;
		}
		if (!Usb3HubPortSts->PortChange.BhResetChange) {
			return USB_ERROR;
		}
		
		Status = UsbHubClearPortFeature(HcStruc, DevInfo, Port, BhPortResetChange);
		if(Status != USB_SUCCESS) return USB_ERROR;
		
		Status = UsbHubClearPortFeature(HcStruc, DevInfo, Port, PortResetChange);
		if(Status != USB_SUCCESS) return USB_ERROR;

	} else {
		Status = UsbHubSetPortFeature(HcStruc, DevInfo, Port, PortReset);
		if(Status != USB_SUCCESS) return USB_ERROR;

        // The duration of the Resetting state is nominally 10 ms to 20 ms
        FixedDelay(20 * 1000);      // 20 ms delay

		for(i = 0; i < 10; i++) {
			Status = UsbHubGetPortStatus(HcStruc, DevInfo, Port, &gUsbData->dHubPortStatus);
			if(Status != USB_SUCCESS) return USB_ERROR;

			if(DevInfo->bEndpointSpeed == USB_DEV_SPEED_SUPER) {
				IsResetChange = Usb3HubPortSts->PortChange.ResetChange ? TRUE : FALSE;
			} else {
				IsResetChange = HubPortSts->PortChange.ResetChange ? TRUE : FALSE;
			}

			if(IsResetChange) break;

            FixedDelay(5 * 1000);      // 5 ms delay
		}
		if (!IsResetChange) {
			return USB_ERROR;
		}

		Status = UsbHubClearPortFeature(HcStruc, DevInfo, Port, PortResetChange);
		if(Status != USB_SUCCESS) return USB_ERROR;

		// 1 ms delay for Low-Speed device
		if (DevInfo->bEndpointSpeed != USB_DEV_SPEED_SUPER &&
			HubPortSts->PortStatus.LowSpeed == 1) {
			FixedDelay(1 * 1000);
		}
	}
	return USB_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbHubCearHubFeature
//
// Description: 
//
// Input:   
//
// Output: USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHubCearHubFeature(
	HC_STRUC*   HcStruc,
    DEV_INFO*   DevInfo,
    HUB_FEATURE	HubFeature)
{
	return USB_ERROR;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbHubClearPortFeature
//
// Description: 
//
// Input:   
//
// Output: USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHubClearPortFeature(
	HC_STRUC*   HcStruc,
    DEV_INFO*   DevInfo,
    UINT8		Port,
    HUB_FEATURE	PortFeature)
{
	DEV_REQ	DevReq;

	DevReq.wRequestType = HUB_RQ_CLEAR_PORT_FEATURE;
	DevReq.wValue = PortFeature;
	DevReq.wIndex = Port;
	DevReq.wDataLength = 0;

	return UsbControlTransfer(HcStruc, DevInfo, DevReq, 50, NULL);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbHubGetHubDescriptor
//
// Description: 
//
// Input:   
//
// Output: USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHubGetHubDescriptor(
	HC_STRUC*   HcStruc,
    DEV_INFO*   DevInfo,
    VOID*		Buffer,
    UINT16		Length)
{
	DEV_REQ	DevReq;

	DevReq.wRequestType = USB_RQ_GET_CLASS_DESCRIPTOR;
	DevReq.wValue = DevInfo->bEndpointSpeed == 
						USB_DEV_SPEED_SUPER ? DESC_TYPE_SS_HUB << 8 : DESC_TYPE_HUB << 8;
	DevReq.wIndex = 0;
	DevReq.wDataLength = Length;

	return UsbControlTransfer(HcStruc, DevInfo, DevReq, 150, Buffer);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbHubGetHubStatus
//
// Description: 
//
// Input:   
//
// Output: USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHubGetHubStatus(
	HC_STRUC*   HcStruc,
    DEV_INFO*   DevInfo,
    UINT32*		HubStatus)
{
	return USB_ERROR;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbHubGetPortStatus
//
// Description: 
//
// Input:   
//
// Output: USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHubGetPortStatus(
	HC_STRUC*   HcStruc,
    DEV_INFO*   DevInfo,
	UINT8		Port,
    UINT32*		PortStatus)
{
	DEV_REQ	DevReq;

	DevReq.wRequestType = HUB_RQ_GET_PORT_STATUS;
	DevReq.wValue = 0;
	DevReq.wIndex = Port;
	DevReq.wDataLength = 4;

	return UsbControlTransfer(HcStruc, DevInfo, DevReq, 150, PortStatus);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbHubGetErrorCount
//
// Description: 
//
// Input:   
//
// Output: USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHubGetErrorCount(
	HC_STRUC*   HcStruc,
    DEV_INFO*   DevInfo,
    UINT8		Port,
    UINT16*		ErrorCount)
{
	return USB_ERROR;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbHubSetHubDescriptor
//
// Description: 
//
// Input:   
//
// Output: USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHubSetHubDescriptor(
	HC_STRUC*   HcStruc,
    DEV_INFO*   DevInfo,
	VOID*		Buffer,
    UINT16		Length)
{
	return USB_ERROR;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbHubSetHubFeature
//
// Description: 
//
// Input:   
//
// Output: USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHubSetHubFeature(
	HC_STRUC*   HcStruc,
    DEV_INFO*   DevInfo,
    HUB_FEATURE	HubFeature)
{
	return USB_ERROR;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbHubSetHubDepth
//
// Description: 
//
// Input:   
//
// Output: USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHubSetHubDepth(
	HC_STRUC*	HcStruc,
	DEV_INFO*	DevInfo,
	UINT16		HubDepth)
{
	DEV_REQ	DevReq;

	DevReq.wRequestType = HUB_RQ_SET_HUB_DEPTH;
	DevReq.wValue = HubDepth;
	DevReq.wIndex = 0;
	DevReq.wDataLength = 0;

	return UsbControlTransfer(HcStruc, DevInfo, DevReq, 50, NULL);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UsbHubSetPortFeature
//
// Description: 
//
// Input:   
//
// Output: USB_ERROR on error, USB_SUCCESS on success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
UsbHubSetPortFeature(
	HC_STRUC*   HcStruc,
    DEV_INFO*   DevInfo,
    UINT8		Port,
    HUB_FEATURE	PortFeature)
{
	DEV_REQ	DevReq;

	DevReq.wRequestType = HUB_RQ_SET_PORT_FEATURE;
	DevReq.wValue = PortFeature;
	DevReq.wIndex = Port;
	DevReq.wDataLength = 0;

	return UsbControlTransfer(HcStruc, DevInfo, DevReq, 100, NULL);		//(EIP77526)		 
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
