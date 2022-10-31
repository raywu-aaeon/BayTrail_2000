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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/RT/usbhid.c 15    8/27/12 5:07a Roberthsu $
//
// $Revision: 15 $
//
// $Date: 8/27/12 5:07a $
//****************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:           usbhid.c
//
//  Description:    USB HID class device driver
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"
#include "UsbDef.h"
#include "AmiUsb.h"
#include "UsbKbd.h"
extern USB_GLOBAL_DATA *gUsbData; 
VOID        USBHIDInitialize (VOID);
UINT8       USBHIDCheckForDevice (DEV_INFO*, UINT8, UINT8, UINT8);
DEV_INFO*   USBHIDConfigureDevice (HC_STRUC*, DEV_INFO*, UINT8*, UINT16, UINT16);
UINT8   	USBHIDProcessData ( HC_STRUC*, DEV_INFO*, UINT8*, UINT8*);
UINT8       USBHIDDisconnectDevice (DEV_INFO*);
VOID		USB_HIDReportDescriptor(HC_STRUC*,DEV_INFO*,UINT8*,UINT16,UINT16);  

VOID        USBMSInitialize (VOID);
DEV_INFO*   USBMSConfigureDevice (HC_STRUC*, DEV_INFO*, UINT8*, UINT16, UINT16); 
DEV_INFO*   USBKBDConfigureDevice (DEV_INFO*);  //(EIP84455)
DEV_INFO*   USBAbsConfigureDevice (HC_STRUC*, DEV_INFO*, UINT8*, UINT16, UINT16);  
VOID        CheckInputMode(HID_STRUC_PTR hid_struc,DEV_INFO *fpDevInfo);    //(EIP101990)
VOID
USBHIDFillDriverEntries (DEV_DRIVER *fpDevDriver)
{
    fpDevDriver->bDevType               = BIOS_DEV_TYPE_HID;
    fpDevDriver->bBaseClass             = BASE_CLASS_HID;
    fpDevDriver->bSubClass              = 0;
    fpDevDriver->bProtocol              = 0;
    fpDevDriver->pfnDeviceInit          = USBHIDInitialize;
    fpDevDriver->pfnCheckDeviceType     = USBHIDCheckForDevice;
    fpDevDriver->pfnConfigureDevice     = USBHIDConfigureDevice;
    fpDevDriver->pfnDisconnectDevice    = USBHIDDisconnectDevice;
    return;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBHIDInitialize (VOID)
//
// Description: This function returns fills the host controller driver
//              routine pointers in the structure provided
//
// Input:       Nothing
//
// Output:      Nothing
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBHIDInitialize (VOID)
{
	USBKBDInitialize();
	USBMSInitialize();

    return;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMSCheckForMouse
//
// Description: This routine checks for mouse type device from the
//              interface data provided
//
// Input:   bBaseClass  USB base class code
//          bSubClass   USB sub-class code
//          bProtocol   USB protocol code
//
// Output:  BIOS_DEV_TYPE_MOUSE type on success or 0FFH on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBHIDCheckForDevice (
    DEV_INFO* fpDevInfo,
    UINT8 bBaseClass,
    UINT8 bSubClass,
    UINT8 bProtocol
)
{
	//
	// Check the BaseClass, SubClass and Protocol for a HID/Boot/Mouse device.
	//
	if (bBaseClass != BASE_CLASS_HID) {
		return USB_ERROR;
	}

#if BOOT_PROTOCOL_SUPPORT
	if (bSubClass != SUB_CLASS_BOOT_DEVICE) {
		return USB_ERROR;
	}

	if (bProtocol != PROTOCOL_KEYBOARD &&
		bProtocol != PROTOCOL_MOUSE) {
		return USB_ERROR;
	}
#endif

	return	BIOS_DEV_TYPE_HID;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBHIDConfigureKeyboard
//
// Description: This routine checks an interface descriptor of the USB device
//      detected to see if it describes a HID/Boot/Keyboard device.
//      If the device matches the above criteria, then the device is
//      configured and initialized
//
// Input:       fpHCStruc   HCStruc pointer
//              fpDevInfo   Device information structure pointer
//              fpDesc      Pointer to the descriptor structure
//              wStart      Offset within interface descriptor
//                      supported by the device
//              wEnd        End offset of the device descriptor
//
// Output:      FPDEV_INFO  New device info structure, 0 on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
                                        //(EIP84455+)>
DEV_INFO*
USBHIDConfigureDevice (
    HC_STRUC*   fpHCStruc,
    DEV_INFO*   fpDevInfo,
    UINT8*      fpDesc,
    UINT16      wStart,
    UINT16      wEnd)
{
    ENDP_DESC       *fpEndpDesc;
    INTRF_DESC      *fpIntrfDesc;
    
	fpDevInfo->bDeviceType = (UINT8)BIOS_DEV_TYPE_HID;
	fpDevInfo->HidDevType = 0;
	fpDevInfo->Hidreport.bFlag = 0;

//Get report descriptor 
	USB_DEBUG(3, "USBHIDConfigureDevice...  \n"); 
	if (BOOT_PROTOCOL_SUPPORT == 0) {
		USB_HIDReportDescriptor(fpHCStruc,fpDevInfo,fpDesc,wStart,wEnd);
		fpDevInfo->Hidreport.bFlag |= HID_REPORT_BFLAG_REPORT_PROTOCOL;
	} else {
		switch (fpDevInfo->bProtocol) {
			case PROTOCOL_KEYBOARD:
				fpDevInfo->HidDevType = HID_DEV_TYPE_KEYBOARD;
				break;

			case PROTOCOL_MOUSE:
				fpDevInfo->HidDevType = HID_DEV_TYPE_MOUSE;
				break;

			default:
				break;
		}
	}


    fpDevInfo->bCallBackIndex = USB_InstallCallBackFunction(USBHIDProcessData);

    fpIntrfDesc = (INTRF_DESC*)(fpDesc + wStart);
    fpDesc+=((CNFG_DESC*)fpDesc)->wTotalLength; // Calculate the end of descriptor block
    fpEndpDesc = (ENDP_DESC*)((char*)fpIntrfDesc + fpIntrfDesc->bDescLength);

//Select correct endpoint
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

		if (fpEndpDesc->bEndpointAddr & EP_DESC_ADDR_DIR_BIT ) {
            fpDevInfo->bIntEndpoint = fpEndpDesc->bEndpointAddr;
            fpDevInfo->wIntMaxPkt = fpEndpDesc->wMaxPacketSize;
            fpDevInfo->bPollInterval = fpEndpDesc->bPollInterval;  
			break;
        }
    }

    if(fpDevInfo->bIntEndpoint == 0) {
        return 0;
    }

//Set protocol (Option)
    if (!(fpDevInfo->wIncompatFlags & USB_INCMPT_SET_BOOT_PROTOCOL_NOT_SUPPORTED) && 
		!(fpDevInfo->Hidreport.bFlag & HID_REPORT_BFLAG_REPORT_PROTOCOL)) {
        //
        // Send the set protocol command, wValue = 0 (Boot protocol)
        //
        (*gUsbData->aHCDriverTable[GET_HCD_INDEX(gUsbData->HcTable[fpDevInfo->bHCNumber - 1]->bHCType)].pfnHCDControlTransfer)(
                    gUsbData->HcTable[fpDevInfo->bHCNumber - 1],
                    fpDevInfo, (UINT16)HID_RQ_SET_PROTOCOL,
                    fpDevInfo->bInterfaceNum,                       
                    0, 0, 0);
    } 

//Send Set_Idle command 
    {
        UINT16          wTemp;             
        wTemp   = gUsbData->wTimeOutValue;     // Save original value         
        gUsbData->wTimeOutValue = 1000;    
        (*gUsbData->aHCDriverTable[GET_HCD_INDEX(gUsbData->HcTable[fpDevInfo->bHCNumber - 1]->bHCType)].pfnHCDControlTransfer)
                    (gUsbData->HcTable[fpDevInfo->bHCNumber - 1],
                    fpDevInfo,(UINT16)HID_RQ_SET_IDLE, fpDevInfo->bInterfaceNum, 0, 0, 0);
        gUsbData->wTimeOutValue = wTemp;   // Restore original timeout value
    }

    if (fpDevInfo->HidDevType & HID_DEV_TYPE_KEYBOARD) { 
        if(!(USBKBDConfigureDevice(fpDevInfo))) {
            return 0;
        }
    } 

//Active polling
    if (fpDevInfo->bPollInterval != 0) {
		if (!((fpDevInfo->HidDevType & HID_DEV_TYPE_MOUSE) && (gUsbData->dUSBStateFlag & USB_FLAG_EFIMS_DIRECT_ACCESS))) {
            (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDActivatePolling)(fpHCStruc,fpDevInfo);
        }
    }


    return fpDevInfo;
} 
                                        //<(EIP84455+)
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Name:        USBHIDDisconnectDevice
//
// Description: This routine disconnects the keyboard by freeing
//              the USB keyboard device table entry
//
// Input:       fpDevInfo   Pointer to DeviceInfo structure
//
// Output:      USB_SUCCESS/USB_ERROR
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBHIDDisconnectDevice (DEV_INFO* fpDevInfo)
{
	HC_STRUC* fpHCStruc = gUsbData->HcTable[fpDevInfo->bHCNumber - 1];

	// Stop polling the endpoint
	(*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDDeactivatePolling)(fpHCStruc,fpDevInfo);
	fpDevInfo->bIntEndpoint = 0;

    if (fpDevInfo->HidDevType & HID_DEV_TYPE_KEYBOARD) {
        USBKBDDisconnectDevice(fpDevInfo);
    }

    if (fpDevInfo->Hidreport.pReport != 0) {
        USB_MemFree(fpDevInfo->Hidreport.pReport, GET_MEM_BLK_COUNT(fpDevInfo->Hidreport.bTotalCount*sizeof(HID_STRUC)));    //(EIP84336)
        fpDevInfo->Hidreport.pReport = 0;
        fpDevInfo->Hidreport.wReportLength = 0;
    }

	return USB_SUCCESS; 	
} 

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       GetItemData
//
// Description: This funtion copy data of the item to buffer.
//
// intput:      
//
// Output:     
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
GetItemData (
	HID_ITEM_PTR	Item,
	VOID			*Buffer,
	UINT32			BufferSize
)
{
	UINT32	Size = Item->bSize > BufferSize ? BufferSize : Item->bSize;
	MemSet(Buffer, BufferSize, 0);
	MemCpy(Buffer, &Item->data, Size);
}

                                        //(EIP71068)>
//Remove function Check_UsagePage
                                        //<(EIP71068)
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       Add_Hid_Field
//
// Description: Add input or output item.
//
// intput:      
//
// Output:     
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
                                        //(EIP84455+)>
VOID Add_Hid_Field(HID_STRUC_PTR hid_struc,HIDReport_STRUC* hidreport)
{
    UINT8   i;
	USB_DEBUG(4, "============================================== \n");	
	USB_DEBUG(4, "Field  count  =%x \t",hidreport->bTotalCount); 	

	hidreport->pReport[hidreport->bTotalCount].bFlag	= hid_struc->bFlag;
	USB_DEBUG(4, "bFlag         =%x \n",hidreport->pReport[hidreport->bTotalCount].bFlag); 	

	hidreport->pReport[hidreport->bTotalCount].bUsagePage   = hid_struc->bUsagePage;
	USB_DEBUG(4, "UsagePage   =%x \t",hid_struc->bUsagePage); 	

	if(hid_struc->bReportID!=0)hidreport->bFlag|= HID_REPORT_BFLAG_REPORT_ID;
	hidreport->pReport[hidreport->bTotalCount].bReportID   = hid_struc->bReportID;
	USB_DEBUG(4, "ReportID    =%x \n",hid_struc->bReportID); 	

	hidreport->pReport[hidreport->bTotalCount].bReportCount= hid_struc->bReportCount;
	USB_DEBUG(4, "bReportCount =%x \t",hid_struc->bReportCount); 	
	
	hidreport->pReport[hidreport->bTotalCount].bReportSize = hid_struc->bReportSize;
	USB_DEBUG(4, "bReportSize  =%x \n",hid_struc->bReportSize); 	

	hidreport->pReport[hidreport->bTotalCount].wLogicalMax   = hid_struc->wLogicalMax;
	USB_DEBUG(4, "wLogicalMax  =%x \t",hid_struc->wLogicalMax); 	

	hidreport->pReport[hidreport->bTotalCount].wLogicalMin	 = hid_struc->wLogicalMin;
	USB_DEBUG(4, "wLogicalMin  =%x \n",hid_struc->wLogicalMin); 	
                                        //(EIP127014+)>
	hidreport->pReport[hidreport->bTotalCount].PhysicalMax   = hid_struc->PhysicalMax;
	USB_DEBUG(4, "PhysicalMax  =%x \t",hid_struc->PhysicalMax); 	

	hidreport->pReport[hidreport->bTotalCount].PhysicalMin	 = hid_struc->PhysicalMin;
	USB_DEBUG(4, "PhysicalMin  =%x \n",hid_struc->PhysicalMin); 	

	hidreport->pReport[hidreport->bTotalCount].UnitExponent	 = hid_struc->UnitExponent;
	USB_DEBUG(4, "UnitExponent  =%x \t",hid_struc->UnitExponent); 	
                                        //<(EIP127014+)
	hidreport->pReport[hidreport->bTotalCount].bUsageMaxCount = hid_struc->bUsageMaxCount;
	USB_DEBUG(4, "bUsageMaxCount  =%x \n",hid_struc->bUsageMaxCount); 	 

	for(i=0;i<(hid_struc->bUsageMaxCount);i++)
	{
		hidreport->pReport[hidreport->bTotalCount].wUsageMax[i]=hid_struc->wUsageMax[i];
		hidreport->pReport[hidreport->bTotalCount].wUsageMin[i]=hid_struc->wUsageMin[i]; 
    	USB_DEBUG(4, "wUsageMax_%x   =%x \t",i,hid_struc->wUsageMax[i]); 	 
        USB_DEBUG(4, "wUsageMin_%x   =%x \n",i,hid_struc->wUsageMin[i]); 	
    } 

	hidreport->pReport[hidreport->bTotalCount].bUsageCount = hid_struc->bUsageCount;
	USB_DEBUG(4, "bUsageCount  =%x \n",hid_struc->bUsageCount); 	

	for(i=0;i<(hid_struc->bUsageCount);i++)
	{
		hidreport->pReport[hidreport->bTotalCount].bUsage[i]=hid_struc->bUsage[i];

		if ((hid_struc->bUsagePage == 0x01) && (hid_struc->bUsage[i] == 0x30)) {
			if (hid_struc->bFlag & HID_BFLAG_RELATIVE_BIT) {
				hidreport->bFlag |= HID_REPORT_BFLAG_RELATIVE_DATA;
			} else {
				hidreport->bFlag |= HID_REPORT_BFLAG_ABSOLUTE_DATA;
			}
			hidreport->wAbsMaxX = hid_struc->wLogicalMax;
		}
		if(hid_struc->bUsage[i]==0x31)
			hidreport->wAbsMaxY= hid_struc->wLogicalMax; 
		USB_DEBUG(4, "num %x Usage %x \n",i,hid_struc->bUsage[i]); 			
	}
//		hidreport->bTotalCount++;       //(EIP84336-)

	return;
}
                                        //<(EIP84455+)

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       Hid_Parser_Main
//
// Description: 
//
// intput:      
//
// Output:     
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8 Hid_Parser_Main(HID_ITEM_PTR item,HID_STRUC_PTR hid_struc,HIDReport_STRUC* hidreport,DEV_INFO*   fpDevInfo)	//(EIP101990)
{
    UINT8   i;							//(EIP84455+)
    UINT8	Data = 0;
                                        //(EIP71068)>
	switch (item->bTag) {
		case HID_MAIN_ITEM_TAG_BEGIN_COLLECTION:
			GetItemData(item, &Data, sizeof(Data));
			// Check if it is application collection
			if (Data == 0x01) {
				if (hid_struc->bUsagePage == 0x01) { 	// Generic Desktop
					switch (hid_struc->bUsage[hid_struc->bUsageCount - 1]) {
						case 0x01:		// Pointer
						case 0x02:		// Mouse
							fpDevInfo->HidDevType |= HID_DEV_TYPE_MOUSE;
							break;
						case 0x06:		// Keyboard
						case 0x07:		// KeyPad
							fpDevInfo->HidDevType |= HID_DEV_TYPE_KEYBOARD;
							break;
						default:
							break;
					}
				} else if (hid_struc->bUsagePage == 0x0D) {	// Digitizer
					if (hid_struc->bUsage[hid_struc->bUsageCount - 1] == 0x04) {
						fpDevInfo->HidDevType |= HID_DEV_TYPE_POINT;
					}
				}
			}

			hid_struc->bCollection_count++;	
			break;
		case HID_MAIN_ITEM_TAG_END_COLLECTION:
			hid_struc->bCollection_count--;
			if(hid_struc->bCollection_count == 0)hid_struc->bFlag &= 0xF7;
			break;
		case HID_MAIN_ITEM_TAG_INPUT:

			if(hid_struc->bFlag & HID_BFLAG_SKIP) break;
			GetItemData(item, &hid_struc->bFlag, sizeof(hid_struc->bFlag));
			hid_struc->bFlag = (hid_struc->bFlag & 7) | HID_BFLAG_INPUT;
            if(hidreport->pReport != NULL)                                      //(EIP84336)
    			Add_Hid_Field(hid_struc,hidreport);
           	hidreport->bTotalCount++;                                           //(EIP84336)
			break;
		case HID_MAIN_ITEM_TAG_OUTPUT:
			if(hid_struc->bFlag & HID_BFLAG_SKIP) break;
			GetItemData(item, &hid_struc->bFlag, sizeof(hid_struc->bFlag));
			hid_struc->bFlag &= 7;
                                        //(EIP98251+)>
            if(hid_struc->bUsagePage == 0x8)
            {
                if(hidreport->pReport != NULL)                                      //(EIP84336) 
                {
            	    Add_Hid_Field(hid_struc,hidreport);
                }
                hidreport->bTotalCount++;                                           //(EIP84336)
            }
                                        //<(EIP98251+)
			break;
		case HID_MAIN_ITEM_TAG_FEATURE:
            CheckInputMode(hid_struc,fpDevInfo);  //(EIP101990)
			break;
		default:
			break;
	}
                                        //<(EIP71068)
                                        //(EIP84455+)>
//Clear Local Item
	hid_struc->bUsageCount =  0;
	hid_struc->bUsageMaxCount =  0;
    for(i=0;i<5;i++)
    {
    	hid_struc->wUsageMax[i] = 0;
    	hid_struc->wUsageMin[i] = 0;
        hid_struc->bUsage[i] = 0;
        hid_struc->bUsage[i+5] = 0;
    }
                                        //<(EIP84455+)
	return 0;
}  

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       Hid_Parser_Global
//
// Description:	Parsing Global item 
//
// intput:      
//
// Output:     
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8 Hid_Parser_Global(HID_ITEM_PTR item,HID_STRUC_PTR hid_struc,HIDReport_STRUC* hidreport,DEV_INFO*   fpDevInfo)		//(EIP101990)
{
	if(hid_struc->bFlag & BIT3)
		return 0;

	switch (item->bTag) {
		case HID_GLOBAL_ITEM_TAG_USAGE_PAGE:
			GetItemData(item, &hid_struc->bUsagePage, sizeof(hid_struc->bUsagePage));
										//(EIP65344+)>
            //Get Led usage page
			if(hid_struc->bUsagePage == 0x8) 
                hidreport->bFlag|= HID_REPORT_BFLAG_LED_FLAG;
										//<(EIP65344+) 
			return 0;

		case HID_GLOBAL_ITEM_TAG_LOGICAL_MINIMUM:
			GetItemData(item, &hid_struc->wLogicalMin, sizeof(hid_struc->wLogicalMin));
			return 0;
		
		case HID_GLOBAL_ITEM_TAG_LOGICAL_MAXIMUM:
			GetItemData(item, &hid_struc->wLogicalMax, sizeof(hid_struc->wLogicalMax));
			return 0; 
                                        //(EIP127014+)>
		case HID_GLOBAL_ITEM_TAG_PHYSICAL_MINIMUM:
			GetItemData(item, &hid_struc->PhysicalMin, sizeof(hid_struc->PhysicalMin));
			return 0;
		
		case HID_GLOBAL_ITEM_TAG_PHYSICAL_MAXIMUM:
			GetItemData(item, &hid_struc->PhysicalMax, sizeof(hid_struc->PhysicalMax));
			return 0;

		case HID_GLOBAL_ITEM_TAG_UNIT_EXPONENT:
			GetItemData(item, &hid_struc->UnitExponent, sizeof(hid_struc->UnitExponent));
			return 0; 
                                        //<(EIP127014+)
		case HID_GLOBAL_ITEM_TAG_REPORT_SIZE:
			GetItemData(item, &hid_struc->bReportSize, sizeof(hid_struc->bReportSize));
			return 0;
		
		case HID_GLOBAL_ITEM_TAG_REPORT_COUNT:
			GetItemData(item, &hid_struc->bReportCount, sizeof(hid_struc->bReportCount));
			return 0;
		
		case HID_GLOBAL_ITEM_TAG_REPORT_ID:
			GetItemData(item, &hid_struc->bReportID, sizeof(hid_struc->bReportID));
			return 0;
		
		default:
			return -1;
	} 
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       Hid_Parser_Local
//
// Description: Parsing Local item
//
// intput:      
//
// Output:     
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8 Hid_Parser_Local(HID_ITEM_PTR item,HID_STRUC_PTR hid_struc,HIDReport_STRUC* hidreport,DEV_INFO*   fpDevInfo)		//(EIP101990)
{
	if(hid_struc->bFlag & HID_BFLAG_SKIP)
		return 0;
	switch (item->bTag) {
                                        //(EIP84455+)>
		case HID_LOCAL_ITEM_TAG_USAGE:	
                                        //(EIP96010+)>
            if (hid_struc->bUsageCount < HID_MAX_USAGE) {
    			GetItemData(item, &hid_struc->bUsage[hid_struc->bUsageCount], 
    							sizeof(hid_struc->bUsage[hid_struc->bUsageCount]));
            }
            hid_struc->bUsageCount++;
                                        //<(EIP96010+)
			return 0;
		
		case HID_LOCAL_ITEM_TAG_USAGE_MINIMUM:
                                        //(EIP96010+)>
            if (hid_struc->bUsageMaxCount < 5) {
    			GetItemData(item, &hid_struc->wUsageMin[hid_struc->bUsageMaxCount], 
    							sizeof(hid_struc->wUsageMin[hid_struc->bUsageMaxCount]));
            }
			return 0;
		
		case HID_LOCAL_ITEM_TAG_USAGE_MAXIMUM:
            if (hid_struc->bUsageMaxCount < 5) {
				GetItemData(item, &hid_struc->wUsageMax[hid_struc->bUsageMaxCount], 
    							sizeof(hid_struc->wUsageMax[hid_struc->bUsageMaxCount]));
            }
            hid_struc->bUsageMaxCount++;
                                        //<(EIP96010+)
			return 0;
                                        //<(EIP84455+)
		
		default:	
			return 0;
}

}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       Hid_Parser_Reserved
//
// Description: Parsing Reserved item
//
// intput:      
//
// Output:     
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8 Hid_Parser_Reserved(HID_ITEM_PTR item,HID_STRUC_PTR hid_struc,HIDReport_STRUC* hidreport,DEV_INFO*   fpDevInfo)	//(EIP101990)
{
	return 0;
} 

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_ParseReportDescriptor
//
// Description: 
//              
// intput:      
//              
// Output:     
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

typedef UINT8 HID_PARSE_PROC(HID_ITEM_PTR,HID_STRUC_PTR,HIDReport_STRUC*,DEV_INFO*   fpDevInfo);	//(EIP101990)

VOID
USB_ParseReportDescriptor(
    DEV_INFO*   fpDevInfo,
    UINT8*  fpBuffer,
	UINT16 	wReportLength,
	UINT8 	bInterfaceProtocol
)
{
	HIDReport_STRUC 	*Hidreport;
	HID_STRUC			*temp;
	HID_Item  			item;
	UINT8 				bBufferData;
	UINT8				DataSize[] = {0, 1, 2, 4};
	HID_PARSE_PROC		*dispatch_type[] = {
			Hid_Parser_Main,
			Hid_Parser_Global,
			Hid_Parser_Local,
			Hid_Parser_Reserved
		};

    temp = USB_MemAlloc (GET_MEM_BLK_COUNT(sizeof(HID_STRUC)));

	Hidreport = &(fpDevInfo->Hidreport);	
	Hidreport->bTotalCount = 0;
	for(; wReportLength > 0; wReportLength--) {
//Get HID item		
		bBufferData = *fpBuffer++;

		item.bType = (bBufferData>> 2) & 3;

		item.bTag  = (bBufferData>> 4) & 15;

		item.bSize = DataSize[bBufferData & 3];

		MemCpy(&item.data.u32, fpBuffer, item.bSize);
		fpBuffer += item.bSize;
		wReportLength -= item.bSize;

//Parsing HID item
		dispatch_type[item.bType](&item,temp,Hidreport,fpDevInfo);
	}

    USB_MemFree(temp,GET_MEM_BLK_COUNT(sizeof(HID_STRUC)));

    return;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USB_HIDReportDescriptor
//
// Description: 
//
// intput:      
//              
// Output:     
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USB_HIDReportDescriptor(
    HC_STRUC*   fpHCStruc,
    DEV_INFO*   fpDevInfo,
    UINT8*      fpDesc,
    UINT16      wStart,
    UINT16      wEnd
)
{
	HID_DESC		*fpHidDesc;
	UINT16			wReportLength=0;
	UINT8			bInterfaceProtocol;
	UINT8			*fpreport;
	USB_DEBUG(3, "USB_HIDReportDescriptor...  \n");  

	bInterfaceProtocol=((INTRF_DESC*)(fpDesc + wStart))->bProtocol;		

	if (((HID_DESC*)(fpDesc + wStart + sizeof(HID_DESC)))->bDescType == DESC_TYPE_HID ) {
		fpHidDesc  = (HID_DESC*)(fpDesc + wStart + sizeof(INTRF_DESC));
		wReportLength = fpHidDesc->bDescriptorLength;
        fpreport= USB_MemAlloc (GET_MEM_BLK_COUNT(wReportLength));
        fpDevInfo->Hidreport.wReportLength = wReportLength ;

		//get report descriptor here
		if (wReportLength != 0) {
			//Get report descriptor
			{
			    UINT8           bGetDescIteration;
			    UINT16          wReg,
			                    wStatus;
				
			    bGetDescIteration = 0;      // Initialize the iteration counter
			    do
			    {
			        wReg = (UINT16)((DESC_TYPE_REPORT << 8) + 0);
			        wStatus = (*gUsbData->aHCDriverTable[GET_HCD_INDEX(fpHCStruc->bHCType)].pfnHCDControlTransfer)(
			                        fpHCStruc,
			                        fpDevInfo,
			                        (UINT16)HID_RQ_GET_DESCRIPTOR,
			                        (UINT16)fpDevInfo->bInterfaceNum,
			                        wReg,
			                        fpreport,
			                        wReportLength);
			        if (wStatus)
			        {
						break;
			        }
			        ++bGetDescIteration;
			    } while (bGetDescIteration < 4);
				
			}//get report descriptor end
			USB_ParseReportDescriptor(fpDevInfo,fpreport,wReportLength,bInterfaceProtocol);
                                        //(EIP89279+)>
            if(fpDevInfo->Hidreport.bTotalCount != 0)  
            {
                fpDevInfo->Hidreport.pReport = USB_MemAlloc (GET_MEM_BLK_COUNT(fpDevInfo->Hidreport.bTotalCount*sizeof(HID_STRUC)));
    			USB_ParseReportDescriptor(fpDevInfo,fpreport,wReportLength,bInterfaceProtocol); 
            }
                                        //<(EIP89279+)
        	USB_MemFree(fpreport, GET_MEM_BLK_COUNT(wReportLength));
		} 
	}
}
                                        //(EIP84455+)>

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       USBHIDProcessData
//
// Description: 
//
// intput:      
//              
// Output:     
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END> 

UINT8
USBHIDProcessData(
    HC_STRUC    *fpHcStruc,
    DEV_INFO    *fpDevInfo,
    UINT8       *fpTD,
    UINT8       *fpBuffer
)
{

    UINT8   data_type = 0,i,j;
	HIDReport_STRUC 	*Hidreport;
	Hidreport = &(fpDevInfo->Hidreport);
    data_type = fpDevInfo->bProtocol;

	if (Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_PROTOCOL) {	
		for(i=0;i<Hidreport->bTotalCount;i++)
		{
			//Check is input?
			if((Hidreport->pReport[i].bFlag & HID_BFLAG_INPUT ))
			{
				//if report id exist, check first byte
				if(Hidreport->bFlag & HID_REPORT_BFLAG_REPORT_ID) 
					if(Hidreport->pReport[i].bReportID != *(fpBuffer))continue;
                if(Hidreport->pReport[i].bUsagePage == 7)
                    data_type = HID_BTYPE_KEYBOARD;
                //Check X,Y
                if((Hidreport->pReport[i].bUsagePage == 1)&&(Hidreport->pReport[i].bUsageCount)!=0)
                {
                //serach 
                    for(j=0;j<Hidreport->pReport[i].bUsageCount;j++)
                    {
                    //find X
                        if(Hidreport->pReport[i].bUsage[j] == 0x30)
                        {
                            if(Hidreport->pReport[i].bFlag & 0x4)
                                data_type = HID_BTYPE_MOUSE;
                            else
                                data_type = HID_BTYPE_POINT;
                        }

                     }    
                }
            }
        }
     }
    switch(data_type)
    {
        case HID_BTYPE_KEYBOARD:
            USBKBDProcessKeyboardData (fpHcStruc,fpDevInfo,fpTD,fpBuffer);
            break;
        case HID_BTYPE_MOUSE:
            USBMSProcessMouseData(fpHcStruc,fpDevInfo,fpTD,fpBuffer);
            break;
        case HID_BTYPE_POINT:
            USBAbsProcessMouseData(fpHcStruc,fpDevInfo,fpTD,fpBuffer);
            break;          
        default:
            break;
    }
    return USB_SUCCESS; 
}
                                        //<(EIP84455+)

                                        //(EIP101990+)>
//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Input:       CheckInputMode
//
// Description: 
//
// intput:      
//              
// Output:     
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END> 
VOID
CheckInputMode(HID_STRUC_PTR hid_struc,DEV_INFO *fpDevInfo)
{
    UINT16          Value = 0x300,Length = 0;
    UINT8           *Fpset,i;

	Fpset= USB_MemAlloc (1); 
    Value |= hid_struc->bReportID;
    *(Fpset)=hid_struc->bReportID;
    *(Fpset+1)=2;
    *(Fpset+2)=0; 
    
    for (i = 0; i < HID_MAX_USAGE; i++) {
		if(hid_struc->bUsagePage == 0xd) {
        	if(hid_struc->bUsage[i] == 0x52 && hid_struc->bUsage[i+1] == 0x53) {
            	//Set Input Mode
            	(*gUsbData->aHCDriverTable[GET_HCD_INDEX(gUsbData->HcTable[fpDevInfo->bHCNumber - 1]->bHCType)].pfnHCDControlTransfer)(
                        	gUsbData->HcTable[fpDevInfo->bHCNumber -1],
                        	fpDevInfo,
                        	(UINT16)HID_RQ_SET_REPORT,
                        	(UINT16)fpDevInfo->bInterfaceNum,
                        	Value,
                        	Fpset,
                        	3);                         

            	break;
        	}
        }
    }    
    USB_MemFree(Fpset,1); 
}
                                        //<(EIP101990+)
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

