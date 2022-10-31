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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/RT/usbmass.c 126   9/04/12 8:03a Wilsonlee $
//
// $Revision: 126 $
//
// $Date: 9/04/12 8:03a $
//****************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:           UsbMass.c
//
//  Description:    AMI USB Mass Storage support implementation
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include "AmiDef.h"
#include "UsbDef.h"
#include "AmiUsb.h"
#include "UsbMass.h"

extern  USB_GLOBAL_DATA *gUsbData;


VOID        USBMassInitialize(VOID);
UINT8       USBMassCheckForStorageDevice(DEV_INFO*, UINT8, UINT8, UINT8);
DEV_INFO*   USBMassConfigureStorageDevice(HC_STRUC*, DEV_INFO*,
                                        UINT8*, UINT16, UINT16);
UINT8       USBMassDisconnectStorageDevice(DEV_INFO*);
UINT16      USBMassSendCBICommand(DEV_INFO*);
UINT32      USBMassProcessBulkData(DEV_INFO*);
UINT8       USBMassConsumeBulkData(DEV_INFO*,UINT8,UINT16);
UINT32      USBMassIssueBOTTransaction(DEV_INFO*);
VOID        USBMassClearBulkEndpointStall(DEV_INFO*, UINT8);
VOID        USBMassBOTResetRecovery(DEV_INFO*);
UINT16      USBMassSendBOTCommand(DEV_INFO*);
UINT8       USBMassGetBOTStatus(DEV_INFO*);
UINT16      USBMassCBIGetStatus(DEV_INFO*);
UINT32      USBMassIssueCBITransaction(DEV_INFO*);
UINT8       USBMassReadCapacityCommand(DEV_INFO*);
UINT32      USBMassCheckDeviceReady(DEV_INFO*);
UINT32      USBMassRequestSense(DEV_INFO* fpDevInfo);
VOID        USBMassSenseKeyParsing(DEV_INFO* , UINT32);
MASS_INQUIRY        *USBMassInquiryCommand(DEV_INFO*);
UINT8       USBMassUpdateDeviceGeometry( DEV_INFO* fpDevInfo );
UINT16      USBMassBOTGetMaxLUN(DEV_INFO*);
VOID        USBMassIdentifyDeviceType(DEV_INFO*, UINT8*);
UINT32      USBMassIssueBulkTransfer(DEV_INFO*, UINT8, UINT8*, UINT32);
VOID        iPodShufflePatch(MASS_GET_DEV_INFO*);
VOID        USBMassUpdateCylinderInfo(DEV_INFO*, UINT32);
UINT8       USBMassSetDefaultGeometry(DEV_INFO*, UINT32);
UINT8       USBMassValidatePartitionTable(MASTER_BOOT_RECORD*, UINT32, MBR_PARTITION*);
UINT16      USBMassSetDefaultType(DEV_INFO*, UINT32);
VOID        USBMassGetPhysicalDeviceType(DEV_INFO*, UINT8*);
UINT8       USB_SetAddress(HC_STRUC*, DEV_INFO*, UINT8);
UINT32      dabc_to_abcd(UINT32);
DEV_INFO*   USBGetProperDeviceInfoStructure(DEV_INFO*, UINT8);
UINT32      USBMassTestUnitReady(DEV_INFO*);
VOID        StoreUsbMassDeviceName(DEV_INFO*, UINT8*);
extern      VOID AddPortNumbertoDeviceString(DEV_INFO*);

VOID MemCopy (UINT8*, UINT8*, UINT32);
VOID* USB_MemAlloc (UINT16);
DEV_INFO* USB_GetDeviceInfoStruc(UINT8, DEV_INFO*, UINT8, HC_STRUC*);
MASS_INQUIRY* USBMassGetDeviceParameters(DEV_INFO*);
UINT32      USBMassReadCapacityBlockSizePatch(UINT32, DEV_INFO*);

UINT8 USB_MemFree  (VOID*,  UINT16);
VOID FixedDelay(UINTN);    
VOID SpeakerBeep (UINT8, UINT16, HC_STRUC*); //(EIP64781+)

static char* IOMegaZIPString        = "IOMEGA  ZIP";
#define IOMegaZIPStringLength       11

static char* MSysDiskOnKeyString    = "M-Sys   DiskOnKey";
#define MSysDiskOnKeyStringLength   17

BOOLEAN CheckDeviceLimit(UINT8);

VOID
USBMassFillDriverEntries (DEV_DRIVER    *fpDevDriver)
{
    fpDevDriver->bDevType               = BIOS_DEV_TYPE_STORAGE;
    fpDevDriver->bProtocol              = 0;
    fpDevDriver->pfnDeviceInit          = USBMassInitialize;
    fpDevDriver->pfnCheckDeviceType     = USBMassCheckForStorageDevice;
    fpDevDriver->pfnConfigureDevice     = USBMassConfigureStorageDevice;
    fpDevDriver->pfnDisconnectDevice    = USBMassDisconnectStorageDevice;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// FUNCTION:    USBMassInitialize
//
// DESCRIPTION: This function initializes mass storage device related data
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassInitialize ()
{
    //
    // Set default value for the delay. Selections are: 20,40,60,80 for 10,20,30,40 sec.
    //
    gUsbData->bUSBStorageDeviceDelayCount = (gUsbData->UsbMassResetDelay + 1)*10;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassCheckForStorageDevice
//
// DESCRIPTION: This routine checks for hub type device from the
//      interface data provided
//
// PARAMETERS:  bBaseClass  USB base class code
//              bSubClass   USB sub-class code
//              bProtocol   USB protocol code
//
// RETURN:  BIOS_DEV_TYPE_STORAGE type on success or 0FFH on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassCheckForStorageDevice (
    DEV_INFO*   fpDevInfo,
    UINT8       bBaseClass,
    UINT8       bSubClass,
    UINT8       bProtocol)
{
    if(bBaseClass != BASE_CLASS_MASS_STORAGE) return USB_ERROR;
										//(EIP99882+)>
    if (!gUsbData->UsbSetupData.UsbMassDriverSupport) {
        return USB_ERROR;
    }
										//<(EIP99882+)
//Skip USB mass storage devices enumeration when legacy is disabled
    if (gUsbData->dUSBStateFlag & USB_FLAG_DISABLE_LEGACY_SUPPORT)
        if(LEGACY_USB_DISABLE_FOR_USB_MASS) 									//(EIP93469)
           return USB_ERROR;
    //
    // Base class is okay. Check the protocol field for supported protocols.
    // Currently we support CBI, CB and BOT protocols.
    //
    if((bProtocol != PROTOCOL_CBI) &&
        (bProtocol != PROTOCOL_CBI_NO_INT) &&
        (bProtocol != PROTOCOL_BOT)) {
        return USB_ERROR;
    }

    return  BIOS_DEV_TYPE_STORAGE;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassGetFreeMassDeviceInfoStruc
//
// Description: This function finds a free mass device info structure and
//              returns the pointer to it
//
// Input:   None
//
// Output:  Pointer to the Mass Device Info (0 on failure)
//          The number mass storage DeviceInfo structure (0-based)
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
USBMassGetFreeMassDeviceInfoStruc(
	DEV_INFO	*DevInfo,
    UINT8		*indx
)
{
    DEV_INFO* dev = &gUsbData->aDevInfoTable[1];
    UINT8 count;
    UINT8 massdev_indx = 0;
    for (count = 0; count < (MAX_DEVICES-1); count++, dev++) {
		if (!(dev->bFlag & DEV_INFO_VALID_STRUC)) {
			continue;
		}
        if (dev->bDeviceType == BIOS_DEV_TYPE_STORAGE) {
            massdev_indx++;
        }
        if (dev == DevInfo) {
			break;
        }
    }
    if (count == (MAX_DEVICES-1)) {
		return NULL;
    }
    *indx = massdev_indx;
    return dev;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassFindFreeMassDeviceInfo
//
// Description: This function finds a free mass device info structure and
//      copies the current mass device info structure into it
//
// Input:   Current mass device info structure
//
// Output:  New mass device info
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
USBMassFindFreeMassDeviceInfo(DEV_INFO* Dev, UINT8 *EmulIndex)
{
    UINT8		Indx = 0;
    DEV_INFO	*NewDev;

    // Get the free mass device info structure pointer
    NewDev = USBMassGetFreeMassDeviceInfoStruc(Dev, &Indx);

    if (NewDev == NULL) return NULL;   // No free entry found.

    // Get the emulation type setup question associated with this device
    ASSERT(Indx>0 && Indx<17);
    if ((Indx == 0) || (Indx > 16)) {
	    return NULL;
    }

    Dev->wEmulationOption = gUsbData->USBMassEmulationOptionTable[Indx-1];
USB_DEBUG(DEBUG_LEVEL_3, "USBMassFindFreeMassDeviceInfo-------- indx %d, emu %d\n", Indx, Dev->wEmulationOption);

    // Set default device type and emulation type to 0
    Dev->bStorageType = 0;
    Dev->fpLUN0DevInfoPtr = 0;
    Dev->bFlag |= DEV_INFO_DEV_PRESENT;

    *EmulIndex = Indx-1;

    return Dev;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassCreateLogicalUnits
//
// Description: This function verifies the presence of logical units (LUN)
//              in the USB mass device and creates appropriate device info
//              structures for them
//
// Input:   fpDevInfo - Device information structure pointer
//          bMaxLun - Maximum number of logical units present (non-ZERO)
//
// Output:  USB_ERROR   On error
//          USB_SUCCESS On successfull completion
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassCreateLogicalUnits(
    DEV_INFO*   fpDevInfo,
    UINT8       bMaxLun,
    UINT8       EmulIndex)
{
    UINT8       bLUN;
    DEV_INFO*   fpNewDevInfo;
    MASS_INQUIRY    *inq;

    for (bLUN = 1; bLUN <= bMaxLun; bLUN++) {

        if (CheckDeviceLimit(BASE_CLASS_MASS_STORAGE) == TRUE) break;
        //
        // Get the proper device info structure
        //
        fpNewDevInfo = USBGetProperDeviceInfoStructure(fpDevInfo, bLUN);
        if (!fpNewDevInfo) return USB_ERROR;
        //
        // Check whether this device is reconnected by checking the
        // valid structure flag
        //
        if ((fpNewDevInfo->bFlag & DEV_INFO_MASS_DEV_REGD)) {
            //
            // Indicate device as connected
            //
            fpNewDevInfo->bFlag |= DEV_INFO_DEV_PRESENT;

			// Change the parent HC number and port number in the existing DEV_INFO
	        fpNewDevInfo->bHCNumber = fpDevInfo->bHCNumber;
	    	fpNewDevInfo->bHubDeviceNumber = fpDevInfo->bHubDeviceNumber;
			fpNewDevInfo->bHubPortNumber = fpDevInfo->bHubPortNumber;
			fpNewDevInfo->bEndpointSpeed = fpDevInfo->bEndpointSpeed;
			fpNewDevInfo->wEndp0MaxPacket = fpDevInfo->wEndp0MaxPacket;
	    	fpNewDevInfo->DevMiscInfo = fpDevInfo->DevMiscInfo;
	        fpNewDevInfo->bDeviceAddress = fpDevInfo->bDeviceAddress;
			fpNewDevInfo->bBulkInEndpoint = fpDevInfo->bBulkInEndpoint;
			fpNewDevInfo->wBulkInMaxPkt = fpDevInfo->wBulkInMaxPkt;
			fpNewDevInfo->bBulkOutEndpoint = fpDevInfo->bBulkOutEndpoint;
			fpNewDevInfo->wBulkOutMaxPkt = fpDevInfo->wBulkOutMaxPkt;
			fpNewDevInfo->bIntEndpoint = fpDevInfo->bIntEndpoint;
			fpNewDevInfo->wIntMaxPkt = fpDevInfo->wIntMaxPkt;
			fpNewDevInfo->bPollInterval = fpDevInfo->bPollInterval;

			fpNewDevInfo->fpLUN0DevInfoPtr	= fpDevInfo;
        } else {    // This is different device, it was not reconnected
            //
            // Copy the old device info structure into the new one
            //
            MemCopy((UINT8*)fpDevInfo,
                    (UINT8*)fpNewDevInfo,
                    sizeof (DEV_INFO));
            fpNewDevInfo->bLUN  = bLUN; // Change LUN number
            fpNewDevInfo->wEmulationOption = gUsbData->USBMassEmulationOptionTable[EmulIndex+bLUN];
            MemSet(fpNewDevInfo->DevNameString, 64, 0);
            //
            // Save the Lun0 device info pointer in the current LUN
            //
            fpNewDevInfo->fpLUN0DevInfoPtr  = fpDevInfo;

			//
			// The Lun0 device might have been already locked by the
			// bus (USBBUS.usbhc_on_timer), clear it for current LUN.
			//
			fpNewDevInfo->bFlag &= ~DEV_INFO_DEV_BUS;

			inq = USBMassGetDeviceParameters(fpNewDevInfo);
			ASSERT(inq);
			StoreUsbMassDeviceName(fpNewDevInfo, (UINT8*)inq+8);
        }

		if (gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI) {
			USB_SmiQueuePut(fpNewDevInfo);
		}
    }

    return  USB_SUCCESS;
}


VOID
StoreUsbMassDeviceName(
    DEV_INFO    *Device,
    UINT8       *Str
)
{
    UINT8   i;
    UINT8   j;

    for (i = 0; i < 64; i++) {
        if (Device->DevNameString[i] != 0) {
            return;
        }
    }

    for (i = 0, j = 0; i < 32; i++) {
        if (*Str == 0) {
            Str++; j++;     // supress leading zeroes
        }
    }

    for (i = 0; i < (32-j); i++, Str++) {
        // supress spaces if more than one
        if ((i>0) && (Device->DevNameString[i-1]==' ') && (*Str==' ')) {
            i--;
            continue;
        }
										//(EIP63706+)>
		// Filter out the character if it is invisible.
		if (((*Str != 0) && (*Str < 0x20)) || (*Str > 0x7E)) {
			i--;
			continue;
		}
										//<(EIP63706+)

        Device->DevNameString[i] = *Str;
    }

    //
    // Add Device number to the USB device string
    //
#if USB_DIFFERENTIATE_IDENTICAL_DEVICE_NAME
    AddPortNumbertoDeviceString(Device);
#endif
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassConfigureStorageDevice
//
// DESCRIPTION: This function checks an interface descriptor of a device
//              to see if it describes a USB mass device.  If the device
//              is a mass storage device,  then it is configured
//              and initialized.
//
// PARAMETERS:  pHCStruc    HCStruc pointer
//              pDevInfo    Device information structure pointer
//              pDesc       Pointer to the descriptor structure
//              wEnd        End offset of the device descriptor
//
// RETURN:      New device info structure, NULL on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

DEV_INFO*
USBMassConfigureStorageDevice (
        HC_STRUC*   fpHCStruc,
        DEV_INFO*   fpDevInfo,
        UINT8*      fpDesc,
        UINT16      wStart,
        UINT16      wEnd)
{
    UINT8           bTemp;
    UINT16          wRetValue;
    ENDP_DESC       *fpEndpDesc;
    INTRF_DESC      *fpIntrfDesc;
    UINT8           bMaxLUN;
    DEV_INFO*       newDev;
    MASS_INQUIRY    *inq;
    BOOLEAN         checkFDDhotplug, checkCDROMhotplug, checkHDDhotplug;
    UINT8           EmulIndex;
    UINT8           i;                  //(EIP64781+)      

    wRetValue       = 0;
    bMaxLUN         = 0;

//
// Set fpDevInfo->bDeviceType.  This serves as a flag
// that indicates a usable interface has been found in the current
// configuration. This is needed so we can check for other usable interfaces
// in the current configuration (composite device) without trying to search
// in other configurations.
//
    fpDevInfo->bDeviceType      = BIOS_DEV_TYPE_STORAGE;
    fpDevInfo->fpPollTDPtr      = 0;

USB_DEBUG (DEBUG_LEVEL_3, "USBMassConfigureDevice ....\n");

    bTemp = 0x03;       // bit 1 = Bulk In, bit 0 = Bulk Out

	fpDevInfo->bBulkOutEndpoint = 0;
	fpDevInfo->bBulkInEndpoint = 0;
	fpDevInfo->bIntEndpoint  = 0;

    fpIntrfDesc = (INTRF_DESC*)(fpDesc + wStart);
    fpDesc+=((CNFG_DESC*)fpDesc)->wTotalLength; // Calculate the end of descriptor block
    fpEndpDesc = (ENDP_DESC*)((char*)fpIntrfDesc + fpIntrfDesc->bDescLength);
    for( ;(fpEndpDesc->bDescType != DESC_TYPE_INTERFACE) && ((UINT8*)fpEndpDesc < fpDesc);
        fpEndpDesc = (ENDP_DESC*)((char*)fpEndpDesc + fpEndpDesc->bDescLength)){

        if(!(fpEndpDesc->bDescLength)) {
            break;  // Br if 0 length desc (should never happen, but...)
        }

        if( fpEndpDesc->bDescType != DESC_TYPE_ENDPOINT ) {
            continue;
        }

        if ((fpEndpDesc->bEndpointFlags & EP_DESC_FLAG_TYPE_BITS) ==
                EP_DESC_FLAG_TYPE_BULK) {   // Bit 1-0: 10 = Endpoint does bulk transfers
            if(!(fpEndpDesc->bEndpointAddr & EP_DESC_ADDR_DIR_BIT)) {
                //
                // Bit 7: Dir. of the endpoint: 1/0 = In/Out
                // If Bulk-Out endpoint already found then skip subsequent ones
                // on the interface.
                //
                if (bTemp & 1) {
                    fpDevInfo->bBulkOutEndpoint = (UINT8)(fpEndpDesc->bEndpointAddr
                                                        & EP_DESC_ADDR_EP_NUM);
                    fpDevInfo->wBulkOutMaxPkt = fpEndpDesc->wMaxPacketSize;
                    bTemp &= 0xFE;
					USB_DEBUG(3, "bulk out endpoint addr: %x, max packet size: %x\n", 
						fpDevInfo->bBulkOutEndpoint, fpDevInfo->wBulkOutMaxPkt);
                }
            } else {
                //
                // If Bulk-In endpoint already found then skip subsequent ones
                // on the interface
                //
                if (bTemp & 2) {
                    fpDevInfo->bBulkInEndpoint  = (UINT8)(fpEndpDesc->bEndpointAddr
                                                        & EP_DESC_ADDR_EP_NUM);
                    fpDevInfo->wBulkInMaxPkt    = fpEndpDesc->wMaxPacketSize;
                    bTemp   &= 0xFD;
					USB_DEBUG(3, "bulk in endpoint addr: %x, max packet size: %x\n", 
						fpDevInfo->bBulkInEndpoint, fpDevInfo->wBulkInMaxPkt);
                }
            }
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
			USB_DEBUG(3, "interrupt in endpoint addr: %x, max packet size: %x\n", 
				fpDevInfo->bIntEndpoint, fpDevInfo->wIntMaxPkt);
        }
    }

    //
    // Check the compatibility flag for LUN support
    //
    if (!(fpDevInfo->wIncompatFlags & USB_INCMPT_SINGLE_LUN_DEVICE)) {
        //
        // If it is a BOT device, get maximum LUN supported
        //
        if (fpDevInfo->bProtocol == PROTOCOL_BOT) {
            bMaxLUN = (UINT8)USBMassBOTGetMaxLUN(fpDevInfo);
        }
    }

    //
    // Check whether the device is already registered. If so, proceed with current
    // mass info structure
    //
    if (fpDevInfo->bFlag & DEV_INFO_MASS_DEV_REGD) {
        newDev = fpDevInfo;
        
        goto UMCM_MassDeviceOkay;
    }

    // Find a new mass device info structure and copy the old one into the new one
    // Note: this is called before GetDeviceParameters because it sets up dev->wEmulationOption
    newDev = USBMassFindFreeMassDeviceInfo(fpDevInfo, &EmulIndex);

    if (newDev == NULL) goto UMCM_Error;
    fpDevInfo = newDev;

    inq = USBMassGetDeviceParameters(fpDevInfo);
    if (inq == NULL) goto UMCM_Error;

    //
    // Do not enumerate device if it is not a CD-ROM and has the block size different from 512 Bytes
    // EIP#15595, iPod nano makes POST hang.
    //
                                        //(EIP59738-)>
    //if ( fpDevInfo->bPhyDevType!=USB_MASS_DEV_CDROM ) {
    //    if( fpDevInfo->wBlockSize!=0x200 && fpDevInfo->wBlockSize!=0xFFFF && fpDevInfo->wBlockSize!=0 )
    //        goto UMCM_Error;
    //}
                                        //<(EIP59738-)
    StoreUsbMassDeviceName(fpDevInfo, (UINT8*)inq+8);

    // Check for the hotplug devices current status, install the new one if needed
    if ( !(gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI) ) {
        // Find out if FDD/HDD/CDROM hotplugging is a valid option
        checkFDDhotplug = ((gUsbData->fdd_hotplug_support == SETUP_DATA_HOTPLUG_ENABLED) ||
            ((gUsbData->fdd_hotplug_support == SETUP_DATA_HOTPLUG_AUTO) &&
                (gUsbData->NumberOfFDDs == 0))) &&
            !(BOOLEAN)(gUsbData->dUSBStateFlag & USB_HOTPLUG_FDD_ENABLED);

        checkHDDhotplug = ((gUsbData->hdd_hotplug_support == SETUP_DATA_HOTPLUG_ENABLED) ||
            ((gUsbData->hdd_hotplug_support == SETUP_DATA_HOTPLUG_AUTO) &&
                (gUsbData->NumberOfHDDs == 0))) &&
            !(BOOLEAN)(gUsbData->dUSBStateFlag & USB_HOTPLUG_HDD_ENABLED);

        checkCDROMhotplug = ((gUsbData->cdrom_hotplug_support == SETUP_DATA_HOTPLUG_ENABLED) ||
            ((gUsbData->cdrom_hotplug_support == SETUP_DATA_HOTPLUG_AUTO) &&
                (gUsbData->NumberOfCDROMs == 0))) &&
            !(BOOLEAN)(gUsbData->dUSBStateFlag & USB_HOTPLUG_CDROM_ENABLED);

        if ( checkFDDhotplug || checkCDROMhotplug || checkHDDhotplug ) {
    USB_DEBUG(DEBUG_LEVEL_3, "connecting hotplug...");
//          inq = USBMassGetDeviceParameters(fpDevInfo);
//          if (inq == NULL) goto UMCM_Error;
    USB_DEBUG(DEBUG_LEVEL_3, "devtype phy %d, emu %d...", fpDevInfo->bPhyDevType, fpDevInfo->bEmuType);

            if ( checkFDDhotplug &&
                (fpDevInfo->bStorageType == USB_MASS_DEV_ARMD) ) {
                newDev = &gUsbData->FddHotplugDev;
                gUsbData->dUSBStateFlag |= USB_HOTPLUG_FDD_ENABLED;
            }

            if ( checkHDDhotplug && (fpDevInfo->bEmuType == USB_EMU_HDD_ONLY) ) {
                newDev = &gUsbData->HddHotplugDev;
                gUsbData->dUSBStateFlag |= USB_HOTPLUG_HDD_ENABLED;
            }

            if ( checkCDROMhotplug && (fpDevInfo->bPhyDevType == USB_MASS_DEV_CDROM) ) {
                newDev = &gUsbData->CdromHotplugDev;
                gUsbData->dUSBStateFlag |= USB_HOTPLUG_CDROM_ENABLED;
            }

            fpDevInfo->bFlag |= DEV_INFO_HOTPLUG;
            *newDev = *fpDevInfo;   // Copy device into DevInfo dedicated to hotplug
            fpDevInfo->bFlag &= ~DEV_INFO_VALIDPRESENT; // Release fpDevInfo
            fpDevInfo = newDev;
        }
    }

UMCM_MassDeviceOkay:
    if ( (newDev->bEmuType == USB_EMU_FLOPPY_ONLY) ||
        (newDev->bEmuType == USB_EMU_FORCED_FDD) ) {
        gUsbData->NumberOfFDDs++;
    }

    if ( newDev->bEmuType == USB_EMU_HDD_ONLY ) {
        gUsbData->NumberOfHDDs++;
    }

//    if ( newDev->bPhyDevType == USB_EMU_HDD_OR_FDD ) {
    if ( newDev->bPhyDevType == USB_MASS_DEV_CDROM ) {
        gUsbData->NumberOfCDROMs++;
    }

    if (bMaxLUN) {
        USBMassCreateLogicalUnits(newDev, bMaxLUN, EmulIndex);
    }

    										//(EIP64781+)>
    if (gUsbData->dUSBStateFlag & USB_FLAG_SKIP_CARD_READER_CONNECT_BEEP) {        
        if ((newDev->bLastStatus & USB_MASS_MEDIA_PRESENT) ||
            newDev->bPhyDevType == USB_MASS_DEV_CDROM ||
            newDev->bPhyDevType == USB_MASS_DEV_FDD) {
            SpeakerBeep(4, 0x1000, fpHCStruc);
        } else if (bMaxLUN) {
            for(i = 1; i < MAX_DEVICES; i++) {
                if (gUsbData->aDevInfoTable[i].fpLUN0DevInfoPtr == newDev) {
                    if (gUsbData->aDevInfoTable[i].bLastStatus & USB_MASS_MEDIA_PRESENT) {
                        SpeakerBeep(4, 0x1000, fpHCStruc);
                        break;
                    }
                }
            }
        }
    }
										//<(EIP64781+)

    return  newDev;

UMCM_Error:
    //USB_AbortConnectDev(fpDevInfo);   //(EIP59579-)
    return NULL;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassDisconnectStorageDevice
//
// DESCRIPTION: This function disconnects the storage device
//
// PARAMETERS:  pDevInfo    Device info structure pointer
//
// RETURN:  Nothing
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassDisconnectStorageDevice (DEV_INFO* fpDevInfo)
{
//  USB_DEBUG (DEBUG_LEVEL_5, "USBMassDisconnectDevice ....  \n");

	fpDevInfo->bBulkOutEndpoint = 0;
	fpDevInfo->bBulkInEndpoint = 0;
	fpDevInfo->bIntEndpoint  = 0;

    if ( (fpDevInfo->bEmuType == USB_EMU_FLOPPY_ONLY) ||
        (fpDevInfo->bEmuType == USB_EMU_FORCED_FDD) ) {
        gUsbData->NumberOfFDDs--;
    }

    if ( fpDevInfo->bEmuType == USB_EMU_HDD_ONLY ) {
        gUsbData->NumberOfHDDs--;
    }

//    if ( newDev->bPhyDevType == USB_EMU_HDD_OR_FDD ) {
    if ( fpDevInfo->bPhyDevType == USB_MASS_DEV_CDROM ) {
        gUsbData->NumberOfCDROMs--;
    }

    if ( fpDevInfo->bFlag & DEV_INFO_HOTPLUG ) {
        fpDevInfo->bFlag &= ~DEV_INFO_HOTPLUG;
		if (fpDevInfo == &gUsbData->FddHotplugDev) {
            gUsbData->dUSBStateFlag &= ~USB_HOTPLUG_FDD_ENABLED;
		} else if (fpDevInfo == &gUsbData->HddHotplugDev) {
			gUsbData->dUSBStateFlag &= ~USB_HOTPLUG_HDD_ENABLED;
		} else if (fpDevInfo == &gUsbData->CdromHotplugDev) {
			gUsbData->dUSBStateFlag &= ~USB_HOTPLUG_CDROM_ENABLED;
		}
    }

    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassClearMassXactStruc
//
// Description: This function clears the mass transaction structure
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
USBMassClearMassXactStruc()
{
    UINT8   i;
    UINT8* fpCleaner    = (UINT8*)&gUsbData->stMassXactStruc;
    for ( i = 0; i < sizeof (MASS_XACT_STRUC); i++ )
    {
        *fpCleaner++ = 0;
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassClearBulkEndpointStall
//
// Description: This function clears the bulk endpoint stall by sending
//      CLEAR_FEATURE command to bulk endpoints
//
// Input:   fpDevInfo   Pointer to DeviceInfo structure
//          bDirec      Endpoint direction
//
// Output:  Nothing
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassClearBulkEndpointStall(
    DEV_INFO* fpDevInfo,
    UINT8 bDirec)
{
										//(EIP54283)>
    UINT8       bShift;
    UINT16      wEndPoint;
	HC_STRUC	*HcStruc;
	HCD_HEADER	*HcdDriver;

	HcStruc = gUsbData->HcTable[fpDevInfo->bHCNumber - 1];
	HcdDriver = &gUsbData->aHCDriverTable[GET_HCD_INDEX(HcStruc->bHCType)];

    wEndPoint = (UINT16)((fpDevInfo->bBulkInEndpoint) | BIT7);

    if(!(bDirec & BIT7)) {
        wEndPoint = fpDevInfo->bBulkOutEndpoint;
    }
    //
    // Issue clear port feature command
    //
	HcdDriver->pfnHCDControlTransfer(HcStruc, fpDevInfo, (UINT16)ENDPOINT_CLEAR_PORT_FEATURE, 
		wEndPoint,(UINT16)ENDPOINT_HALT, 0, 0);

	if (HcdDriver->pfnHCDClearEndpointState) {
		HcdDriver->pfnHCDClearEndpointState(HcStruc, fpDevInfo, (UINT8)wEndPoint);
	} else {
	    //
	    // Reset the toggle bit
	    //
	    bShift = (wEndPoint & 0xF) - 1;

	    if(bDirec & BIT7)
			fpDevInfo->wDataInSync &= ~((UINT16)(1 << bShift));
	    else
			fpDevInfo->wDataOutSync &= ~((UINT16)(1 << bShift));

		if (fpDevInfo->fpLUN0DevInfoPtr == NULL) return;

		if(bDirec & BIT7)
			fpDevInfo->fpLUN0DevInfoPtr->wDataInSync &= ~((UINT16)(1 << bShift));
	    else
			fpDevInfo->fpLUN0DevInfoPtr->wDataOutSync &= ~((UINT16)(1 << bShift));
	}
										//<(EIP54283)
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassIssueMassTransaction
//
// Description: This function performs a mass storage transaction by
//      invoking proper transaction protocol.
//
// Input:   Pointer to DeviceInfo structure
//          stMassXactStruc
//              pCmdBuffer  Pointer to command buffer
//              bCmdSize    Size of command block
//              bXferDir    Transfer direction
//              fpBuffer    Data buffer far pointer
//              dwLength    Amount of data to be transferred
//              wPreSkip    Number of bytes to skip before data
//              wPostSkip   Number of bytes to skip after data
//
// Output:  Amount of data actually transferred
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT32
USBMassIssueMassTransaction (DEV_INFO* fpDevInfo)
{
    if ((fpDevInfo->bProtocol == PROTOCOL_CBI) ||
        (fpDevInfo->bProtocol == PROTOCOL_CBI_NO_INT))
    {
        return USBMassIssueCBITransaction( fpDevInfo );
    }

    if (fpDevInfo->bProtocol == PROTOCOL_BOT)
    {
        return USBMassIssueBOTTransaction( fpDevInfo );
    }

    return 0;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassGetDeviceParameters
//
// Description: This function gets the USB mass device parameters such as
//      max cylinder, head, sector, block size and
//
// Input:   Pointer to DeviceInfo structure
//
// Output:  Pointer to the temp buffer, NULL on error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

MASS_INQUIRY*
USBMassGetDeviceParameters (DEV_INFO* fpDevInfo)
{
    MASS_INQUIRY *inq;

    inq = USBMassInquiryCommand(fpDevInfo);
//USB_DEBUG(DEBUG_LEVEL_3, "fpMassInquiry = %x\n", fpMassInquiry);
    if (!inq) return NULL;

    fpDevInfo->wBlockSize = 0xFFFF; // Clear the cached block size

    //
    // Find the device type and update the device type structure accordingly
    //
    USBMassIdentifyDeviceType(fpDevInfo, (UINT8*)inq);

    return inq;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ValidateDeviceName
//
// Description: This procedure check whether device return valid device name
//      if no valid device name returned, assign default name for it
//
// Input:   Inquiry Data
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ValidateDeviceName (MASS_INQUIRY *inq_data)
{
    static UINT8 DefaultName[] = "USB     Storage Device";

    UINT8 *name = ((UINT8*)inq_data) + 8;
    UINT8 *def_name = DefaultName;
    UINT8 count;

    // check for a blank name
    if (*name) return;

//  for (count = 0; count < 28; count++) {
//      if (*(name+count)) return;  // Not blank
//  }

    // copy default name
    for (count = 0; count < sizeof(DefaultName); count++) {
        *(name+count) = *(def_name+count);
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassGetDeviceInfo
//
// Description: This function fills and returns the mass get device info
//      structure
//
// Input:   fpMassGetDevInfo    Pointer to the mass get info struc
//          bDevAddr    USB device address of the device
//
// Output:  USB_SUCCESS or USB_ERROR
//          fpMassGetDevInfo    Pointer to the mass get info struc
//              dSenseData  Sense data of the last command
//              bDevType    Device type byte (HDD, CD, Removable)
//              bEmuType    Emulation type used
//              fpDevId     Far pointer to the device ID
//
// Notes:   Initially the bDevAddr should be set to 0 as input. This
//          function returns the information regarding the first mass
//          storage device (if no device found it returns bDevAddr as
//          0FFh) and also updates bDevAddr to the device address of
//          the current mass storage device. If no other mass storage
//          device is found then the routine sets the bit7 to 1
//          indicating current information is valid but no more mass
//          device found in the system. The caller can get the next
//          device info if bDevAddr is not 0FFh and bit7 is not set
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassGetDeviceInfo (MASS_GET_DEV_INFO *fpMassGetDevInfo)
{
    DEV_INFO        *fpDevInfo;
    MASS_INQUIRY    *fpMassInq;
    UINT8 bDev = fpMassGetDevInfo->bDevAddr;

    //
    // Get the total number of Mass Storage Devices
    //
    fpMassGetDevInfo->bTotalMassDev = (UINT8)(UINTN)USB_GetDeviceInfoStruc(USB_SRCH_DEV_NUM,
                            0, BIOS_DEV_TYPE_STORAGE, 0);

    if (bDev == 0) {
        iPodShufflePatch(fpMassGetDevInfo);
    }

    if (bDev & BIT7) return USB_ERROR;  // Check for device address validity

    //
    // If bDev = 0 then get information about first mass storage device
    //
    if (!bDev) {
        fpDevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_TYPE, 0, BIOS_DEV_TYPE_STORAGE, 0);
//USB_DEBUG(DEBUG_LEVEL_3, "Get Mass0 info: %x\n", fpDevInfo);

        if (!fpDevInfo) {   // Set as no more device found
            fpMassGetDevInfo->bDevAddr  = 0xFF;
            return  USB_SUCCESS;
        }
    }
    else {  //  Not the first mass device
        //
        // Get the device info structure for the matching device index
        //
        fpDevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_INDX, 0, bDev, 0);
        ASSERT(fpDevInfo);
        if ( (!fpDevInfo) || (!(fpDevInfo->bFlag & DEV_INFO_DEV_PRESENT)) ) {   // Error
            return  USB_ERROR;
        }
        //
        // Get device info structure for next device
        //
        fpDevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_TYPE, fpDevInfo, BIOS_DEV_TYPE_STORAGE, 0);
        ASSERT(fpDevInfo);
        if (!fpDevInfo) {   // Error. Exit !
            return  USB_ERROR;
        }
    }
    fpMassInq = USBMassGetDeviceParameters(fpDevInfo);

    if (!fpMassInq) return  USB_ERROR;

    fpMassGetDevInfo->bDevType = fpDevInfo->bPhyDevType;
//  fpMassGetDevInfo->bPhyDevType = fpDevInfo->bPhyDevType;
    fpMassGetDevInfo->bEmuType = fpDevInfo->bEmuType;
    fpMassGetDevInfo->wPciInfo =
        gUsbData->HcTable[fpDevInfo->bHCNumber - 1]->wBusDevFuncNum;
    fpMassGetDevInfo->fpDevId = (UINT32)(UINTN)((UINT8*)fpMassInq+8);
//  fpMassGetDevInfo->fpDevId = USBMassAdjustIdString((UINT32)fpMassInq+8);

    bDev = (UINT8)(UINTN)USB_GetDeviceInfoStruc(USB_SRCH_DEV_INDX, fpDevInfo, 0, 0);
    ASSERT(bDev);

    bDev |= BIT7;   // Assume that no more mass device present

    //
    // Check whether more mass device is present
    //
    if (USB_GetDeviceInfoStruc(USB_SRCH_DEV_TYPE, fpDevInfo, BIOS_DEV_TYPE_STORAGE, 0)) {
        bDev &= ~BIT7;
    }

    fpDevInfo->bFlag |= DEV_INFO_MASS_DEV_REGD;
    fpMassGetDevInfo->bDevAddr = bDev;

    *(UINTN*)fpMassGetDevInfo->Handle = *(UINTN*)fpDevInfo->Handle;

    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   iPodShufflePatch
//
// Description: This check whether iPod shuffle attached to system and move
//      iPod shuffle to first initial device.
//
// Input:   Pointer to the mass get info struc
//
// Output:  None
//
// Notes:   Attaching iPod shuffle and iPod mini to system causes BIOS POST
//          stop. iPod shuffle must be initialized as early as possible.
//          iPod mini cosumes about 2 seconds to complete initialization,
//          init iPod shuffle first to fix problem.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
iPodShufflePatch(
    MASS_GET_DEV_INFO *fpMassGetDevInfo
)
{
    //TO BE IMPLEMENTED
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassGetDeviceStatus
//
// Description:  Get USB MassStorage device status. Include Media Informarion.
//               Refer to USB_MASS_MEDIA_XXX in USBDEF.H
//
// Input:       Pointer to DeviceInfo structure
//
// Output:      USB_ERROR or USB_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassGetDeviceStatus  (MASS_GET_DEV_STATUS *fpMassGetDevSts)
{
    DEV_INFO*   fpDevInfo;
    UINT8       bDevAddr = fpMassGetDevSts->bDevAddr;

    fpDevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_INDX, 0, bDevAddr, 0);
    ASSERT(fpDevInfo != NULL);
    if (fpDevInfo == NULL) return USB_ERROR;

//  USB_DEBUG (DEBUG_LEVEL_3, "USBMassGetDeviceStatus .... bDevAddr = %x\n",bDevAddr);

    USBMassCheckDeviceReady(fpDevInfo);

    fpMassGetDevSts->bDeviceStatus = fpDevInfo->bLastStatus;
    if(fpDevInfo->bLastStatus & USB_MASS_MEDIA_CHANGED) {

//  USB_DEBUG (DEBUG_LEVEL_3, "\t\t....MEDIA HAS CHANGED....\n");
        //
        // Clear Media Change Status.
        //
        fpDevInfo->bLastStatus &= (UINT8)(~USB_MASS_MEDIA_CHANGED);
//  USB_DEBUG (DEBUG_LEVEL_3, " bDevAddr = %x, bStatus = %x\n",bDevAddr,fpDevInfo->bLastStatus);
    }
    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassCmdPassThru
//
// Description: This function issues the command/data sequence provided
//      as input.  This function can be used to send raw data
//      to the USB mass storage device
//
// Input:       fpDevInfo   Pointer to Device Info structure
//              fpMassCmdPassThru   Pointer to the mass command pass
//                  through structure
//              bDevAddr        USB device address of the device
//              dSenseData      Sense data of the last command
//              fpCmdBuffer     Far pointer to the command buffer
//              wCmdLength      Command length
//              fpDataBuffer    Far pointer for data buffer
//              wDataLength     Data length
//              bXferDir        Data transfer direction
//
// Output:  USB_SUCCESS or USB_ERROR
//          dSenseData      Sense data of the last command
//          fpDataBuffer    Updated with returned data if the transfer
//                          is an IN transaction
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassCmdPassThru (
    MASS_CMD_PASS_THRU  *fpMassCmdPassThru)
{
    UINT8           *fpCmdBuffer;
    UINT8           *fpSrc;
    UINT8           *fpDst;
    DEV_INFO        *fpDevInfo;
//  UINT8           bCommandRetried = FALSE;
    UINT8           bCmdBlkSize;
    UINT8           count;
    UINT16          wData;
    UINT32          dData;

    UINT8   bDevAddr = fpMassCmdPassThru->bDevAddr;

    fpDevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_INDX, 0, bDevAddr, 0);
    if ( (!fpDevInfo) || (!(fpDevInfo->bFlag & DEV_INFO_DEV_PRESENT)) ) {   // Error
        return USB_ERROR;
    }

    bCmdBlkSize = (UINT8)((fpMassCmdPassThru->wCmdLength +
                USB_MEM_BLK_SIZE - 1) >> USB_MEM_BLK_SIZE_SHIFT);

    //
    // Check whether the drive is ready for read TOC command
    //
    USBMassCheckDeviceReady(fpDevInfo);

    //
    // Allocate memory for the command buffer
    //
    fpCmdBuffer = USB_MemAlloc((UINT8)GET_MEM_BLK_COUNT(bCmdBlkSize));
    if (!fpCmdBuffer) {
        return  USB_ERROR;
    }

    //
    // Copy the command into (just allocated) mass command buffer
    //
    fpSrc = (UINT8*)(UINTN)fpMassCmdPassThru->fpCmdBuffer;
    fpDst = fpCmdBuffer;
    for (count = 0; count < fpMassCmdPassThru->wCmdLength; count++) {
        *fpDst++ = *fpSrc++;
    }

    //
    // Clear the common bulk transaction structure
    //
    USBMassClearMassXactStruc();

    //
    // Fill the common bulk transaction structure
    //
    gUsbData->stMassXactStruc.fpCmdBuffer = fpCmdBuffer;
    gUsbData->stMassXactStruc.bCmdSize = (UINT8)fpMassCmdPassThru->wCmdLength;
    gUsbData->stMassXactStruc.bXferDir = fpMassCmdPassThru->bXferDir;
    gUsbData->stMassXactStruc.fpBuffer = (UINT8*)(UINTN)fpMassCmdPassThru->fpDataBuffer;
    gUsbData->stMassXactStruc.dLength = (UINT32)fpMassCmdPassThru->wDataLength;

    wData = (UINT16)USBMassIssueMassTransaction(fpDevInfo);

    //
    // Update the actual data length processed/returned
    //
    fpMassCmdPassThru->wDataLength = wData;

    dData = USBMassRequestSense (fpDevInfo);

    fpMassCmdPassThru->dSenseData = dData;

    //
    // Check and free command buffer
    //
    if (!fpCmdBuffer) {
        return  USB_ERROR;
    }

    USB_MemFree(fpCmdBuffer, (UINT16)GET_MEM_BLK_COUNT(bCmdBlkSize));

    return USB_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassReadCapacityCommand
//
// Description: This function issues read capacity of the mass storage
//
// Input:       Pointer to DeviceInfo structure
//
// Output:      USB_ERROR or USB_SUCCESS
//
// Notes:   This routine will update the MassDeviceInfo structure
//          with the block size & last LBA values obtained from the
//          device
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassReadCapacityCommand  (DEV_INFO* fpDevInfo)
{
    UINT32          dData;
    COMN_READ_CAPACITY_CMD  *fpCmdBuffer;

    if( !VALID_DEVINFO(fpDevInfo))
        return USB_ERROR;
    //
    // Allocate memory for the command buffer
    //
    fpCmdBuffer = USB_MemAlloc(GET_MEM_BLK_COUNT_STRUC(COMN_READ_CAPACITY_CMD));

    if(!fpCmdBuffer) {
        return USB_ERROR;
    }

    fpCmdBuffer->bOpCode    = COMMON_READ_CAPACITY_OPCODE;

    //
    // Clear the common bulk transaction structure
    //
    USBMassClearMassXactStruc();

    //
    // Change the bulk transfer delay to 10 seconds (For CDROM drives)
    //
    gUsbData->wBulkDataXferDelay = 10000;

    //
    // Fill the common bulk transaction structure
    //
    gUsbData->stMassXactStruc.fpCmdBuffer = (UINT8*)fpCmdBuffer;
                                        //(EIP51158+)>
    if (fpDevInfo->bSubClass == SUB_CLASS_SCSI) {
		gUsbData->stMassXactStruc.bCmdSize = 0x0A;	 //SBC-3_66
    } else {
        gUsbData->stMassXactStruc.bCmdSize = sizeof (COMN_READ_CAPACITY_CMD);
    }
										//<(EIP51158+)
    gUsbData->stMassXactStruc.bXferDir = BIT7;     // IN
    gUsbData->stMassXactStruc.fpBuffer = gUsbData->fpUSBTempBuffer;
    gUsbData->stMassXactStruc.dLength = 8;

USB_DEBUG (DEBUG_LEVEL_3, "rcc..");
    dData = USBMassIssueMassTransaction(fpDevInfo);

    //
    // Reset the delay back
    //
    gUsbData->wBulkDataXferDelay = 0;

    if(!dData) {
        USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_READ_CAPACITY_CMD));
USB_DEBUG (DEBUG_LEVEL_3, "err ");
        return  USB_ERROR;
    }

    dData = *((UINT32*)(gUsbData->fpUSBTempBuffer + 4));

    //
    // Change little endian format to big endian(INTEL) format
    //
    dData = dabc_to_abcd(dData);
										 	//(EIP37167+)>
	if(!dData) {
		USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_READ_CAPACITY_CMD));
		USB_DEBUG (DEBUG_LEVEL_3, "err ");
		return	USB_ERROR;
	}
											 //<(EIP37167+)
    dData = USBMassReadCapacityBlockSizePatch(dData, fpDevInfo);     //Patch the block size if needed

    fpDevInfo->wBlockSize = (UINT16)dData;
//USB_DEBUG(DEBUG_LEVEL_3, "succ: %x, %x\n", dData, fpDevInfo);
    //
    // Store the last LBA number in the mass info structure
    //
    dData =  *((UINT32*)(gUsbData->fpUSBTempBuffer));

    dData = dabc_to_abcd(dData);
											//(EIP37167+)>
	if(!dData) {
		USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_READ_CAPACITY_CMD));
        USB_DEBUG (DEBUG_LEVEL_3, "err ");
		return	USB_ERROR;
	}
											 //<(EIP37167+)

    fpDevInfo->dMaxLba = dData + 1; // 1-based value

USB_DEBUG (DEBUG_LEVEL_3, "%x ", fpDevInfo->dMaxLba);

    USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_READ_CAPACITY_CMD));

    return USB_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassReadCapacityBlockSizePatch
//
// Description: This function is used to correct the block size returned by ReadCapacity command.
//
// Input:       Pointer to DeviceInfo structure
//
// Output:      USB_ERROR or USB_SUCCESS
//
// Notes:   
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
USBMassReadCapacityBlockSizePatch(UINT32 dData, DEV_INFO* DevInfo)
{
  //So far TEAC CD-22E and ViPower IDE to USB2.0 bridge needs patch
  //TODO: Add the patch for above devices after getting VID/DID
  return dData;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassReadFormatCapacity
//
// Description: This function sends read format capacity command to the USB
//              mass storage device
//
// Input:       Pointer to DeviceInfo structure
//
// Output:      USB_ERROR or USB_SUCCESS
//
// Notes:       This routine will update the MassDeviceInfo structure
//              with the block size & last LBA values obtained from the
//              device
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassReadFormatCapacity (DEV_INFO* fpDevInfo)
{
    COMN_READ_FMT_CAPACITY  *fpCmdBuffer;
    UINT32  dData;
    UINT16  wData;
    UINT8*	DataBuffer;
	UINT16	DataBufferSize = 0xFC;

    //
    // Allocate memory for the command buffer
    //
    fpCmdBuffer = USB_MemAlloc(GET_MEM_BLK_COUNT_STRUC(COMN_READ_FMT_CAPACITY));

    if(!fpCmdBuffer) {
        return USB_ERROR;
    }

	DataBuffer = USB_MemAlloc(GET_MEM_BLK_COUNT(DataBufferSize));
	if (DataBuffer == NULL) {
		return USB_ERROR;
	}

    fpCmdBuffer->bOpCode = COMMON_READ_FORMAT_CAPACITY_OPCODE;
    fpCmdBuffer->wAllocLength = (UINT16)((DataBufferSize << 8) + (DataBufferSize >> 8));

    USBMassClearMassXactStruc();    // Clear the common bulk transaction structure

    //
    // Fill the common bulk transaction structure
    //
    gUsbData->stMassXactStruc.fpCmdBuffer = (UINT8*)fpCmdBuffer;
                                        //(EIP51158+)>
    if (fpDevInfo->bSubClass == SUB_CLASS_SCSI) {
		gUsbData->stMassXactStruc.bCmdSize = 0x0A;
    } else {
    	gUsbData->stMassXactStruc.bCmdSize = sizeof (COMN_READ_FMT_CAPACITY);
    }
										//<(EIP51158+)
    gUsbData->stMassXactStruc.bXferDir = BIT7;     // IN
    gUsbData->stMassXactStruc.fpBuffer = DataBuffer;
//  gUsbData->stMassXactStruc.dLength = MAX_TEMP_BUFFER_SIZE;
//
// Temp buffer 40h-64h was used as device name string buffer.
// Limit Transaction size to 40h to prevent name string display problem.
//
    gUsbData->stMassXactStruc.dLength = DataBufferSize;

USB_DEBUG (DEBUG_LEVEL_5, "Issue ReadFormatCapacityCommand .... \n");

    dData = USBMassIssueMassTransaction(fpDevInfo);

    //
    // The amount of data obtained should be atleast of read format capacity structure size
    //
    if (dData < sizeof (COMN_READ_FMT_CAPACITY)) {
		USB_MemFree(DataBuffer, GET_MEM_BLK_COUNT(DataBufferSize));
        USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_READ_FMT_CAPACITY));
        return USB_ERROR;
    }


    if ((DataBuffer[0] != 0) || (DataBuffer[1] != 0) || (DataBuffer[2] != 0) || (DataBuffer[3] < 0x08)) {
        USB_MemFree(DataBuffer, GET_MEM_BLK_COUNT(DataBufferSize));
        USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_READ_FMT_CAPACITY));
        return USB_ERROR;
    }

    wData = *((UINT16*)(DataBuffer + 10));   // Offset 10
    if (wData == 0) {
        USB_MemFree(DataBuffer, GET_MEM_BLK_COUNT(DataBufferSize));
        USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_READ_FMT_CAPACITY));
        return USB_ERROR;
    }
    fpDevInfo->wBlockSize = (UINT16)((wData << 8) + (wData >> 8));

    dData = *((UINT32*)(DataBuffer + 4));   // Offset 4
    if (dData == 0) {
        USB_MemFree(DataBuffer, GET_MEM_BLK_COUNT(DataBufferSize));
        USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_READ_FMT_CAPACITY));
        return USB_ERROR;
    }
    dData = dabc_to_abcd(dData);
    fpDevInfo->dMaxLba = dData;

    if (dData == USB_144MB_FDD_MAX_LBA) {
        //
        // Return parameters for 1.44MB floppy
        //
        fpDevInfo->bHeads           = USB_144MB_FDD_MAX_HEADS;
        fpDevInfo->bNonLBAHeads     = USB_144MB_FDD_MAX_HEADS;
        fpDevInfo->bSectors         = USB_144MB_FDD_MAX_SECTORS;
        fpDevInfo->bNonLBASectors   = USB_144MB_FDD_MAX_SECTORS;
        fpDevInfo->wCylinders       = USB_144MB_FDD_MAX_CYLINDERS;
        fpDevInfo->wNonLBACylinders = USB_144MB_FDD_MAX_CYLINDERS;
        fpDevInfo->bMediaType       = USB_144MB_FDD_MEDIA_TYPE;
    }

	USB_MemFree(DataBuffer, GET_MEM_BLK_COUNT(DataBufferSize));
    USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_READ_FMT_CAPACITY));

    return  USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassReadSector
//
// Description: This function a sector at the LBA specified
//
// Input:   Pointer to DeviceInfo structure
//          LBA to read
//          DS:DI   Data buffer pointer
//
// Output:  USB_SUCCESS or USB_ERROR
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8
USBMassReadSector(
    DEV_INFO*   fpDevInfo,
    UINT32      dLba,
    UINT8*      fpBuffer)
{
    COMN_RWV_CMD    *fpCmdBuffer;
    UINT32          dData;
    UINT8           bCounter;
    UINT8           bRetValue = USB_ERROR;

    //
    // Allocate memory for the command buffer
    //
    fpCmdBuffer = USB_MemAlloc(GET_MEM_BLK_COUNT_STRUC(COMN_RWV_CMD));

    if(!fpCmdBuffer) {
        return USB_ERROR;
    }

    bCounter = 10;
    do {
		//
		// Set opcode to read command
		//
		fpCmdBuffer->bOpCode = COMMON_READ_10_OPCODE;
		fpCmdBuffer->wTransferLength = 0x100;	// Big endian to little endian: 0x0001 -> 0x0100
		dData = dLba;
		//
		// Change LBA from big endian to little endian
		//
		dData = dabc_to_abcd(dData);
		
		fpCmdBuffer->dLba = dData;

        //
        // Fill the common bulk transaction structure
        //
        gUsbData->stMassXactStruc.fpCmdBuffer = (UINT8*)fpCmdBuffer;
                                        //(EIP51158+)>
        if (fpDevInfo->bSubClass == SUB_CLASS_SCSI) {
	    	gUsbData->stMassXactStruc.bCmdSize = 0x0A;	//SBC-3_60
        } else {
            gUsbData->stMassXactStruc.bCmdSize = sizeof (COMN_RWV_CMD);
        }
										//<(EIP51158+)
        gUsbData->stMassXactStruc.bXferDir = BIT7;     // IN
        gUsbData->stMassXactStruc.fpBuffer = fpBuffer;
        gUsbData->stMassXactStruc.dLength = fpDevInfo->wBlockSize;  //(EIP59738)
        gUsbData->stMassXactStruc.wPreSkip = 0;
        gUsbData->stMassXactStruc.wPostSkip= 0;

USB_DEBUG (DEBUG_LEVEL_5, "Read Sector .... \n");

        dData = USBMassIssueMassTransaction(fpDevInfo);
        if(dData) {
			bRetValue	= USB_SUCCESS;
            break;  // Success
        }
        //
        // May be drive error. Try to correct from it !
        // Check whether the drive is ready for read/write/verify command
        //
        dData = USBMassCheckDeviceReady(fpDevInfo);
        if (dData) {    // Device is not ready.
            bRetValue   = USB_ERROR;
            break;
        }
        MemSet((UINT8*)fpCmdBuffer, sizeof(COMN_RWV_CMD), 0);
    } while (bCounter--);

    USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_RWV_CMD));

    return  bRetValue;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassUpdateCHSFromBootRecord
//
// Description: This function parses the boot record and extract the CHS
//      information of the formatted media from the boot record.
//      This routine checks for DOS & NTFS formats only
//
// Input:   Pointer to DeviceInfo structure
//          Maximum LBA in the device
//          Boot record of the device
//
// Output:  USB_ERROR   If the boot record is un-recognizable and CHS info
//          is not extracted
//          USB_SUCCESS If the boot record is recognizable and CHS info
//          is extracted. CHS information is updated in the
//          mass device info structure
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassUpdateCHSFromBootRecord(
    DEV_INFO		*fpDevInfo,
    UINT32			dMaxLba,
    BOOT_SECTOR		*BootSetor)
{
	UINT32	OemName = 0;
	UINT32	Fat16SysType = 0;
	UINT32	Fat32SysType = 0;

    if (BootSetor->Signature != 0xAA55) {
		return USB_ERROR;
    }

    //
    // Check for valid MSDOS/MSWIN/NTFS boot record
    //
	MemCopy((UINT8*)BootSetor->OEMName, (UINT8*)&OemName, sizeof(OemName));
    if ((OemName != 0x4F44534D) &&    // "ODSM", MSDO...
        (OemName != 0x4957534D) &&    // "IWSM", MSWI...
        (OemName != 0x5346544E)) {    // "SFTN", NTFS
        //
        // Check for valid FAT,FAT16 or FAT32 boot records
        //
        BootSetor->Fat.Fat16.FilSysType[3] = 0x20;
		MemCopy((UINT8*)BootSetor->Fat.Fat16.FilSysType, (UINT8*)&Fat16SysType, 
			sizeof(Fat16SysType));
		MemCopy((UINT8*)BootSetor->Fat.Fat32.FilSysType, (UINT8*)&Fat32SysType, 
			sizeof(Fat32SysType));
        if ((Fat16SysType != 0x20544146) &&	// " TAF", FAT
            (Fat32SysType != 0x33544146)) {	// "3TAF", FAT3

            //
            // None of the conditions met - boot record is invalid. Return with error
            //
            return  USB_ERROR;
        }
    }

    // zero check added to prevent invalid sector/head information in BPB
    if (BootSetor->SecPerTrk == 0) {
		return USB_ERROR;
    }

    fpDevInfo->bSectors = (UINT8)BootSetor->SecPerTrk;
	fpDevInfo->bNonLBASectors = (UINT8)BootSetor->SecPerTrk;

    // Wrong BPB in MSI MegaStick 128; this is preformat usility issue, wrong BPB
    // information built in device.
    if (BootSetor->NumHeads == 0) {
		return USB_ERROR;
    }

    fpDevInfo->bHeads = (UINT8)BootSetor->NumHeads;
	fpDevInfo->bNonLBAHeads = (UINT8)BootSetor->NumHeads;
    fpDevInfo->BpbMediaDesc = BootSetor->Media;

    USBMassUpdateCylinderInfo(fpDevInfo, dMaxLba);

USB_DEBUG (DEBUG_LEVEL_4, "CHS: %x %x %x\n",
                fpDevInfo->bSectors,
                fpDevInfo->bHeads,
                fpDevInfo->wCylinders);

    return  USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassUpdateCylinderInfo
//
// Description: This procedure update cylinder parameter for device geometry.
//      head and sector paramaters are required before invoke this
//      function.
//
// Input:   Pointer to DeviceInfo structure
//          Maximum LBA in the device
//              dev->bHeads
//              dev->bSectors
//
// Output:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassUpdateCylinderInfo(DEV_INFO* dev, UINT32 lba)
{
    UINT32 data = lba /(dev->bSectors * dev->bHeads);
    if (data <= 1) data++;
    if (data > 0xFFFF) data = 0xFFFF;   // DOS workaround

    dev->wCylinders = (UINT16)data;
    dev->wNonLBACylinders = (UINT16)data;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassGetFormatType
//
// Description: This function reads the first sector from the mass storage
//      device and identifies the formatted type.
//
// Input:   Pointer to DeviceInfo structure
//          Maximum LBA of the device
//
// Output:  USB_ERROR   If could not identify the formatted type
//          USB_SUCCESS If formatted type is identified
//              MSB of emu - Emulation type
//              LSB of emu - Device type (Floppy, Harddisk or CDROM)
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassGetFormatType(
    DEV_INFO*   fpDevInfo,
    UINT32      dMaxLba,
    UINT16      *emu)
{
    UINT16  emu_ = 0;
	MBR_PARTITION Partition = {0};

    //
    // Read the first sector of the device
    //
    if (USBMassReadSector(fpDevInfo, 0, gUsbData->fpUSBMassConsumeBuffer) == USB_ERROR) {
        return  USB_ERROR;
    }

    fpDevInfo->bHiddenSectors = 0;

    //
    // Check for validity of the partition table/boot record
    //
    if (*((UINT16*)(gUsbData->fpUSBMassConsumeBuffer + 0x1FE)) != 0xAA55) {
        USBMassSetDefaultGeometry(fpDevInfo, dMaxLba);
        return  USB_ERROR;
    }

    if (USBMassValidatePartitionTable((MASTER_BOOT_RECORD*)gUsbData->fpUSBMassConsumeBuffer, 
			dMaxLba, &Partition) == USB_SUCCESS) {
        //
        // Only one partition present, check the device size, if the device size
        // is less than 530 MB assume FDD or else assume the emulation as HDD
        //
										//(EIP86793)>
        //if (((dMaxLba >> 11) < MAX_SIZE_FOR_USB_FLOPPY_EMULATION) &&	//(EIP80382)
        //    !(gUsbData->dUSBStateFlag & USB_FLAG_MASS_NATIVE_EMULATION)) {
        //    emu_ = (UINT16)(USB_EMU_FORCED_FDD << 8) + USB_MASS_DEV_ARMD;
        //}else {
        //    emu_ = (UINT16)(USB_EMU_HDD_ONLY << 8) + USB_MASS_DEV_HDD;
        //}
        //
        // Read boot sector, set the LBA number to boot record LBA number
        //
        fpDevInfo->bHiddenSectors = Partition.StartingLba;

        if (USBMassReadSector(fpDevInfo, Partition.StartingLba,
                gUsbData->fpUSBMassConsumeBuffer) == USB_ERROR) {
            return  USB_ERROR;
        }

        if (USBMassUpdateCHSFromBootRecord(fpDevInfo, dMaxLba, 
				(BOOT_SECTOR*)gUsbData->fpUSBMassConsumeBuffer) == USB_SUCCESS) {
            if (((dMaxLba>> 11) < MAX_SIZE_FOR_USB_FLOPPY_EMULATION) &&
                !(gUsbData->dUSBStateFlag & USB_FLAG_MASS_NATIVE_EMULATION)) {
                if(fpDevInfo->bSubClass != SUB_CLASS_UFI) {
                    *emu = (UINT16)(USB_EMU_FORCED_FDD << 8) + USB_MASS_DEV_ARMD;
                }
            }
            return USB_SUCCESS;
        }
        else {  // Reset hidden sector value and return HDD emulation
            USBMassSetDefaultGeometry(fpDevInfo, dMaxLba);
            fpDevInfo->bHiddenSectors = 0;
										//(EIP43711)>
            //don't emulate as HDD for UFI class even media has valid partition like HDD
            if(gUsbData->dUSBStateFlag & USB_FLAG_MASS_SIZE_EMULATION) {
	            if(fpDevInfo->bSubClass != SUB_CLASS_UFI) {
		            if ((dMaxLba >> 11) < MAX_SIZE_FOR_USB_FLOPPY_EMULATION) {
                        *emu = (UINT16)(USB_EMU_FORCED_FDD << 8) + USB_MASS_DEV_ARMD;
                    }
                }
            }
										//<(EIP43711)
            return USB_SUCCESS;
        }
    }
    
    *emu = (UINT16)(USB_EMU_FLOPPY_ONLY << 8) + USB_MASS_DEV_ARMD;
    
    if (USBMassUpdateCHSFromBootRecord(fpDevInfo, dMaxLba, 
			(BOOT_SECTOR*)gUsbData->fpUSBMassConsumeBuffer) == USB_SUCCESS) {
        //*emu = USBMassSetDefaultType(fpDevInfo, dMaxLba);
         if(gUsbData->dUSBStateFlag & USB_FLAG_MASS_SIZE_EMULATION) {
            if(fpDevInfo->bSubClass != SUB_CLASS_UFI) {
                if ((dMaxLba >> 11) >= MAX_SIZE_FOR_USB_FLOPPY_EMULATION) {
                    *emu = (UINT16)(USB_EMU_HDD_ONLY << 8) + USB_MASS_DEV_HDD;
                }
            }
        }
        return USB_SUCCESS;
    }
    USBMassSetDefaultGeometry(fpDevInfo, dMaxLba);
										//(EIP80382)>
    //*emu = USBMassSetDefaultType(fpDevInfo, dMaxLba);

    if (((dMaxLba>> 11) >= MAX_SIZE_FOR_USB_FLOPPY_EMULATION) &&
        !(gUsbData->dUSBStateFlag & USB_FLAG_MASS_NATIVE_EMULATION)) {
        if(fpDevInfo->bSubClass != SUB_CLASS_UFI) {
            *emu = (UINT16)(USB_EMU_HDD_ONLY << 8) + USB_MASS_DEV_HDD;
        }
        fpDevInfo->bHiddenSectors = 0;
    }
    //*emu = emu_;
										//<(EIP80382)
										//<(EIP86793)
    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassSetDefaultType
//
// Description: This procedure set device type depend on device class.
//
// Input:   Pointer to DeviceInfo structure
//          Maximum LBA in the device (DWORD)
//
// Output:  Device Type (WORD)
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
USBMassSetDefaultType(DEV_INFO* dev, UINT32 lba)
{
    UINT16 devtype = (UINT16)(USB_EMU_FLOPPY_ONLY << 8) + USB_MASS_DEV_ARMD;

    if (dev->bSubClass != SUB_CLASS_UFI) {  // Check whether UFI class device
        // Assume force FDD emulation for non-UFI class device
        devtype = (UINT16)(USB_EMU_FORCED_FDD << 8) + USB_MASS_DEV_ARMD;
    }
    return devtype;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassValidatePartitionTable
//
// Description: This procedure check whether partition table valid.
//
// Input:   Partition table content
//          Maximum LBA in the device
//
// Output:  USB_SUCCESS - partion table is valid:
//              Possible valid entry count(1-based)
//              Table entry counts(0-based, 4 means all entries scaned)
//              Activate entry offset(Absolute offset)
//          USB_ERROR - Invalid partition table
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassValidatePartitionTable(
    IN MASTER_BOOT_RECORD	*Mbr,
    IN UINT32 				lba,
    OUT MBR_PARTITION		*Partition)
{
	UINT8	Index = 0;
	UINT8	ActivateIndex = 0;

    // The partition table area could be all 0's, and it would pass the below tests,
    // So test for that here (test sector count for all partitions).
	if ((Mbr->PartRec[0].SizeInLba == 0) && 
		(Mbr->PartRec[1].SizeInLba == 0) &&
		(Mbr->PartRec[2].SizeInLba == 0) &&
		(Mbr->PartRec[3].SizeInLba == 0)) {
		return USB_ERROR;
	}

    for (Index = 0; Index < 4; Index++) {
        // Boot flag check added to ensure that boot sector will not be treated as
        // a valid partation table.
        if (Mbr->PartRec[Index].BootIndicator & 0x7F) {
			return USB_ERROR;   // BootFlag should be 0x0 or 0x80
        }

        // Check whether beginning LBA is reasonable
        if (Mbr->PartRec[Index].StartingLba > lba) {
			return USB_ERROR;
        }

        // Check whether the size is reasonable
#if HDD_PART_SIZE_CHECK
        if (Mbr->PartRec[Index].SizeInLba > lba) {
			return USB_ERROR;
        }
#endif
        // Update activate entry offset
        if (!(Mbr->PartRec[Index].BootIndicator & 0x80)) {
			continue;
        }

		ActivateIndex = Index;
    }

    // If no activate partition table entry found use first entry
    MemCopy((UINT8*)&Mbr->PartRec[ActivateIndex], (UINT8*)Partition, 
    	sizeof(MBR_PARTITION));

    return USB_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassSetDefaultGeometry
//
// Description: This procedure set default geometry for mass storage devices.
//
// Input:   Pointer to DeviceInfo structure
//          Maximum LBA in the device
//
// Output:  USB_ERROR   If could not identify the formatted type
//          USB_SUCCESS If formatted type is identified
//              Emulation type - bits 8..15
//              Device type (Floppy, Harddisk or CDROM) - bits 0..7
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8 USBMassSetDefaultGeometry(DEV_INFO* dev, UINT32 lba)
{
    if (dev->bSubClass == SUB_CLASS_UFI) {
        dev->bHeads = 0x02;
        dev->bNonLBAHeads = 0x02;
        dev->bSectors = 0x12;
        dev->bNonLBASectors = 0x12;
    }
    else {
    	dev->bSectors = 0x3F;
    	dev->bNonLBASectors = 0x3F;
// Use default heads that results in 1023 (3FF) cylinders or less for CHS
    	if (lba <= 0x1F7820) {
            dev->bHeads = 0x20;
            dev->bNonLBAHeads = 0x20;
        }
        else if ( (lba > 0x1F7820) && (lba <= 0x3EF040) ) {
            dev->bHeads = 0x40;
            dev->bNonLBAHeads = 0x40;
        }
        else if ( (lba > 0x3EF040) && (lba <= 0x7DE080) ) {
            dev->bHeads = 0x80;
            dev->bNonLBAHeads = 0x80;
        }
        else if (lba > 0x7DE080) {
            dev->bHeads = 0xFF;
            dev->bNonLBAHeads = 0xFF;
        }
    }

    USBMassUpdateCylinderInfo(dev, lba);
    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassIdentifyDeviceType
//
// Description: This function identifies the type of the USB mass storage
//              device attached from the INQUIRY data obtained from the drive
//
// Input:   Pointer to DeviceInfo structure
//          Pointer to the inquiry data (read from device)
//
// Output:  Nothing
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassIdentifyDeviceType(
    DEV_INFO*   dev,
    UINT8*      inq_data)
{
    UINT16  wEmulationType;
    UINT16  wForceEmulationType = 0;
    UINT32  dData = 0;
    static  UINT16 USBMassEmulationTypeTable[5] = {
        0,  // Auto
        (USB_EMU_FLOPPY_ONLY << 8) + USB_MASS_DEV_ARMD,     // Floppy
        (USB_EMU_FORCED_FDD << 8) + USB_MASS_DEV_ARMD,      // Forced floppy
        (USB_EMU_HDD_ONLY << 8) + USB_MASS_DEV_HDD,         // HDD
        (USB_EMU_HDD_OR_FDD << 8) + USB_MASS_DEV_CDROM };   // CDROM

    USBMassGetPhysicalDeviceType(dev, inq_data);

    // Note: at this point we assume that dev->wEmulationOption is filled in
    // according to the setup question selection.
    if (!(dev->bFlag & DEV_INFO_HOTPLUG) || dev->wEmulationOption) {    // not auto
        wEmulationType = USBMassEmulationTypeTable[dev->wEmulationOption];
        wForceEmulationType = USBMassEmulationTypeTable[dev->wEmulationOption];
    }
//USB_DEBUG(DEBUG_LEVEL_3, ">>-- IdentifyDeviceType:: Device #%d, Emul#: %d, Emul: %x\n", dev->bDeviceAddress, dev->wEmulationOption, wEmulationType);

#if USB_STORAGE_DEVICE_RMB_CHECK
    if (*(inq_data+1) & 0x80) { // Check RMB status
        wForceEmulationType = (UINT16)(USB_EMU_HDD_ONLY << 8) + USB_MASS_DEV_HDD;
    }
#endif
    dev->bLastStatus |= USB_MASS_MEDIA_PRESENT; // Assume Media Present

    if (*inq_data == 5) {   // CDROM
        // Set the type as CDROM and emulation as HDD or FDD
        wEmulationType = (UINT16)(USB_EMU_HDD_OR_FDD << 8) + USB_MASS_DEV_CDROM;
        goto UMIDT_DeviceTypeOver;
    }
//					;(EIP25229+)>
#if USB_START_UNIT_BEFORE_MSD_ENUMERATION
//  Start unit command before access it
	USBMassStartUnitCommand (dev);
#endif
//					;<(EIP25229+)
                                        //(EIP80382)>
    if (dev->bSubClass == SUB_CLASS_UFI) { 
        wEmulationType = (UINT16)(USB_EMU_FLOPPY_ONLY << 8) + USB_MASS_DEV_ARMD;
    } else {
        wEmulationType = (UINT16)(USB_EMU_HDD_ONLY << 8) + USB_MASS_DEV_HDD;
    }
                                        //<(EIP80382)
                                        
    FixedDelay(gUsbData->UsbTimingPolicy.MassDeviceComeUp * 1000);    // Device is coming up give 500ms delay
    //
    // Some USB mass storage devces are not fast enough to accept mass storage
    // commands for parsing geometry, issue read capacity command to make sure device
    // is ready for further access. (USB0089+)>
    //
    if(dev->bSubClass != SUB_CLASS_UFI)
    {
        UINT8 count;
        for (count = 0; count < 30 && VALID_DEVINFO(dev) ; count++) {

            if ( USBMassReadCapacityCommand(dev) == USB_SUCCESS ) {
                break;
            }
            if ( (UINT16)USBMassRequestSense(dev) == 0x3A02 ) {	//(EIP86793)
                break;  // No media
            }
        }
    }
    //
    // Get the block size & last LBA number
    //
    dData = USBMassCheckDeviceReady(dev);
										//(EIP86793)>
    if ((UINT16)dData == 0x3A02) {  // Check for media presence status
        //
        // Media not present. Try to get disk geometry from Format
        // capacity command
        //
        if (!(dev->wIncompatFlags & USB_INCMPT_FORMAT_CAPACITY_NOT_SUPPORTED)) {
            if (dev->bSubClass == SUB_CLASS_UFI) {
                USBMassReadFormatCapacity(dev);
                if ((dev->dMaxLba != 0) && (dev->dMaxLba <= USB_144MB_FDD_MAX_LBA)) {
                    wEmulationType = (UINT16)(USB_EMU_FLOPPY_ONLY << 8) + USB_MASS_DEV_ARMD;
                }else {
                    if (!(gUsbData->dUSBStateFlag & USB_FLAG_MASS_EMULATION_FOR_NO_MEDIA)) {
                    wEmulationType = (UINT16)(USB_EMU_FORCED_FDD << 8) + USB_MASS_DEV_ARMD;
                    }
                }
                goto UMIDT_DeviceTypeOver;
            }
        }
    }

    //
    // Proceed with normal checking
    //
    if (!dData) {
                                        //(EIP59738-)>
        //
        // Get the max LBA & block size; if block size is other than
        // 512 bytes assume emulation as CDROM
        //
        //if ( dev->wBlockSize > 0x200 ) {
        //    wEmulationType = (UINT16)(USB_EMU_HDD_OR_FDD << 8) + USB_MASS_DEV_CDROM;
        //    goto UMIDT_DeviceTypeOver;
        //}
                                        //(<EIP59738-)
                                        //(EIP80382)>
        if (USBMassGetFormatType(dev, dev->dMaxLba, &wEmulationType) == USB_ERROR) {
            //
            // Find the device type by size
            //
            if (((dev->dMaxLba >> 11) < MAX_SIZE_FOR_USB_FLOPPY_EMULATION) || 
                    (gUsbData->dUSBStateFlag & USB_FLAG_MASS_NATIVE_EMULATION)) {
				if (dev->bSubClass != SUB_CLASS_UFI) {
					wEmulationType = (USB_EMU_FORCED_FDD << 8) + USB_MASS_DEV_ARMD;
                }
            }
        }
    }
										//<(EIP80382)
										//<(EIP86793)

UMIDT_DeviceTypeOver:

    if (wForceEmulationType) wEmulationType = wForceEmulationType;
    dev->bStorageType = (UINT8)wEmulationType;
    dev->bEmuType = (UINT8)(wEmulationType >> 8);

USB_DEBUG(DEBUG_LEVEL_3, "<<-- IdentifyDeviceType:: Emul: %x\n", wEmulationType);

    return;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassGetPhysicalDeviceType
//
// Description: This procedure classify USB mass storage devices according to
//              inquiry command return data.
//
// Input:   Pointer to DeviceInfo structure
//          Pointer to the inquiry data (read from device)
//
// Output:  Nothing
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassGetPhysicalDeviceType(
    DEV_INFO*   dev,
    UINT8       *buf
)
{
    switch (*buf) {
        case 0x0:
            if (dev->bSubClass == SUB_CLASS_UFI) {
                dev->bPhyDevType = USB_MASS_DEV_FDD;
                break;
            }
            dev->bPhyDevType = (*(buf+1) & 0x80) ? 
                USB_MASS_DEV_ARMD : USB_MASS_DEV_HDD;
            break;
        case 0x5:
            dev->bPhyDevType = USB_MASS_DEV_CDROM;
            break;
        case 0x7:
            dev->bPhyDevType = USB_MASS_DEV_MO;
            break;
        case 0xE:
            dev->bPhyDevType = USB_MASS_DEV_ARMD;
            break;
        default:
            dev->bPhyDevType = USB_MASS_DEV_UNKNOWN;
            break;
    }
}

/*                                      //(EIP59738-)>
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassConsumeBulkData
//
// DESCRIPTION: This function reads unwanted amount of data specified in
//              the size
//
// PARAMETERS:  fpDevInfo   Pointer to DeviceInfo structure
//              bXferDir    Transfer direction
//              wLength     Size of data to consume
//
// RETURN:      USB_ERROR or USB_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassConsumeBulkData(
    DEV_INFO*   fpDevInfo,
    UINT8       bXferDir,
    UINT16      wLength)
{
    UINT16  wBytesToTransfer, wBytesRemaining;
    UINT32  dData;

//
// Need to process only maximum amount of data that pUSBMassConsumeBuffer can
// handle, i.e. MAX_CONTROL_DATA_SIZE
//
    wBytesRemaining = wLength;
    do {
        wBytesToTransfer = (UINT16)((wBytesRemaining < MAX_CONTROL_DATA_SIZE)?
                    wBytesRemaining : MAX_CONTROL_DATA_SIZE);

        dData = USBMassIssueBulkTransfer(fpDevInfo, bXferDir,
                    gUsbData->fpUSBMassConsumeBuffer, (UINT32)wBytesToTransfer);

        if ((UINT16)dData != wBytesToTransfer) {    // Comparing word should be sufficient
            return  USB_ERROR;
        }
        wBytesRemaining = (UINT16)(wBytesRemaining - dData);

    } while (wBytesRemaining);

    return  USB_SUCCESS;
}
*/                                      //<(EIP59738-)


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassProcessBulkData
//
// DESCRIPTION: This function reads/writes the data to the mass storage
//              device using bulk transfer. It also takes care of pre and
//              post skip bytes.
//
// PARAMETERS:  fpDevInfo   Pointer to DeviceInfo structure
//              stMassXactStruc (given for reference)
//                  bXferDir    Transfer direction
//                  fpBuffer    Data buffer far pointer
//                  dLength Amount of data to be transferred
//                  wPreSkip    Number of bytes to skip before data
//                  wPostSkip   Number of bytes to skip after data
//
// RETURN:      Amount of data actually transferred
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
										//(EIP70814)>
UINT32
USBMassProcessBulkData(DEV_INFO* fpDevInfo)
{
    UINT32      dData;
    UINT16      wTemp;
	UINT8		*Buffer;
	UINT8		*SrcBuffer;
	UINT8		*DestBuffer;
	UINT16		PreSkip;
	UINT32		XferSize;
	UINT32		XferedSize;
	UINT32		RemainingSize;

//USB_DEBUG (DEBUG_LEVEL_3, "Pre,%x;Post,%x\n", gUsbData->stMassXactStruc.wPreSkip,
//                  gUsbData->stMassXactStruc.wPostSkip);
    //
    // Check whether something we have to transfer
    //
    if (!gUsbData->stMassXactStruc.dLength) {
        return 0;
    }


    wTemp   = gUsbData->wTimeOutValue;     // Save original value
    if (gUsbData->wBulkDataXferDelay) {    // Check the bulk data delay specified
        gUsbData->wTimeOutValue    = gUsbData->wBulkDataXferDelay;
    }

	if ((gUsbData->stMassXactStruc.wPreSkip == 0) && 
		(gUsbData->stMassXactStruc.wPostSkip == 0)) {

	    dData = USBMassIssueBulkTransfer(
	                    fpDevInfo,
	                    gUsbData->stMassXactStruc.bXferDir,
	                    gUsbData->stMassXactStruc.fpBuffer,
	                    gUsbData->stMassXactStruc.dLength);
	} else {
		// Allocate a data buffer
		Buffer = USB_MemAlloc((UINT16)GET_MEM_BLK_COUNT(fpDevInfo->wBlockSize));
		PreSkip = gUsbData->stMassXactStruc.wPreSkip;
		RemainingSize = gUsbData->stMassXactStruc.dLength - 
						(PreSkip + gUsbData->stMassXactStruc.wPostSkip);
		DestBuffer = gUsbData->stMassXactStruc.fpBuffer;

		for (XferedSize = 0; XferedSize < gUsbData->stMassXactStruc.dLength;) {
			XferSize = gUsbData->stMassXactStruc.dLength >= fpDevInfo->wBlockSize ?
						fpDevInfo->wBlockSize : gUsbData->stMassXactStruc.dLength;

		    dData = USBMassIssueBulkTransfer(
		                fpDevInfo,
		                gUsbData->stMassXactStruc.bXferDir,
		                Buffer,
		                XferSize);
		    if (dData == 0) {
                                        //(EIP83295)>
                //return 0;
                XferedSize = 0;
                break;
                                        //<(EIP83295)
		    }

			XferedSize += XferSize;
			if (RemainingSize == 0) {
				continue;
			}

			SrcBuffer = Buffer;
	
			if (PreSkip != 0) {
				if (PreSkip >= XferSize) {
					PreSkip -= XferSize;
					continue;
				}
	
				SrcBuffer += PreSkip;
				XferSize -= (UINT32)PreSkip;
				PreSkip = 0;
			}

			XferSize = RemainingSize < XferSize ? RemainingSize : XferSize;
			MemCopy(SrcBuffer, DestBuffer, XferSize);

			// Update the destination buffer pointer
			DestBuffer += XferSize;
			RemainingSize -= XferSize;
		}
		
		USB_MemFree(Buffer, (UINT16)GET_MEM_BLK_COUNT(fpDevInfo->wBlockSize));

		dData = XferedSize;
	}

    gUsbData->wTimeOutValue = wTemp;   // Restore original timeout value
    gUsbData->wBulkDataXferDelay = 0;

	return dData;
}
										//<(EIP70814)

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassInquiryCommand
//
// DESCRIPTION: This function sends inquiry command to the USB mass storage
//              device
//
// PARAMETERS:  fpDevInfo   Pointer to DeviceInfo structure
//
// RETURN:      Pointer to the inquiry data
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

MASS_INQUIRY*
USBMassInquiryCommand (DEV_INFO* fpDevInfo)
{
    COMMON_INQ_CMD  *fpCmdBuffer;
    UINT32          dData;

    //
    // Allocate memory for the command buffer
    //
    fpCmdBuffer = USB_MemAlloc(GET_MEM_BLK_COUNT_STRUC(COMMON_INQ_CMD));
    if(!fpCmdBuffer) {
        return NULL;
    }

    fpCmdBuffer->bOpCode = COMMON_INQUIRY_OPCODE;
    fpCmdBuffer->bAllocLength = 0x24;

    //
    // Clear the common bulk transaction structure
    //
    USBMassClearMassXactStruc();

    //
    // Fill the common bulk transaction structure
    //
    gUsbData->stMassXactStruc.fpCmdBuffer = (UINT8*)fpCmdBuffer;
                                        //(EIP51158+)>
    if (fpDevInfo->bSubClass == SUB_CLASS_SCSI) {
		gUsbData->stMassXactStruc.bCmdSize = 0x06;	//SPC-4_246	
    } else {
    	gUsbData->stMassXactStruc.bCmdSize = sizeof (COMMON_INQ_CMD);
    }
										//<(EIP51158+)
    gUsbData->stMassXactStruc.bXferDir = BIT7;     // IN
    gUsbData->stMassXactStruc.fpBuffer = gUsbData->fpUSBTempBuffer + 0x40;
    gUsbData->stMassXactStruc.dLength = 0x24;

USB_DEBUG (DEBUG_LEVEL_5, "Issue Inquiry Command .... \n");

    dData = USBMassIssueMassTransaction(fpDevInfo);

    USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMMON_INQ_CMD));


    if (dData) {
        return (MASS_INQUIRY*)(gUsbData->fpUSBTempBuffer + 0x40);
    }
    else {
        return NULL;
    }
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassRWVCommand
//
// Description: This function reads/writes/verifies blocks of data from the
//              USB mass device specified by its device address
//
// Input:   fpDevInfo   Pointer to DeviceInfo structure
//          bOpCode     Read/Write/Verify
//          fpReadData  Pointer to the read command structure
//              bDevAddr        USB device address of the device
//              dStartLBA       Starting LBA address
//              wNumBlks        Number of blocks to process
//              wPreSkipSize    Number of bytes to skip before
//              wPostSkipSize   Number of bytes to skip after
//              fpBufferPtr     Far buffer pointer
//
// Output:  Return code (0 - Failure, <>0 - Size read)
//              fpReadData  Pointer to the mass read command structure
//                  dSenseData  Sense data of the last command
//                  fpBufferPtr Far buffer pointer
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
USBMassRWVCommand(
    DEV_INFO    *fpDevInfo,
    UINT8       bOpCode,
    VOID        *DataStruc
)
{
	MASS_READ		*fpDataStruc = (MASS_READ*)DataStruc;
    COMN_RWV_CMD	*fpCmdBuffer = NULL;
    UINT32  dStartLba;
    UINT32  dBytesToRW;
    UINT32  dData, dSenseData;
    UINT8   bDir;       // BIT7 0/1 - R/W
    UINT8   bRetryNum;
    UINT16  wRetCode = 0;

    //
    // Set the sense code as 0
    //
    fpDataStruc->dSenseData = 0;

    //
    // Allocate memory for the command buffer
    //
    fpCmdBuffer = (COMN_RWV_CMD*)USB_MemAlloc(GET_MEM_BLK_COUNT_STRUC(COMN_RWV_CMD));
    if (!fpCmdBuffer) {
        return  0;
    }

	for (bRetryNum = 0; bRetryNum < 2; bRetryNum++) {
	    //
	    // Load command into (just allocated) mass command buffer
	    //
	    fpCmdBuffer->bOpCode = bOpCode;
	    dStartLba = fpDataStruc->dStartLBA;
                                        //(EIP60588+)>
        if (dStartLba > (fpDevInfo->dMaxLba - fpDataStruc->wNumBlks)) {
            dStartLba = fpDevInfo->dMaxLba - fpDataStruc->wNumBlks;
        }
                                        //<(EIP60588+)
	//
	// If the "Forced FDD" option is selected that means the device has
	// to be emulated as a floppy drive even though it has a HDD emulated
	// image.  This is accomplished by hiding the first cylinder totally.
	// The partition table is in the first cylinder.  LBA value for all
	// the requests to the device will be offset with the number of sectors
	// in the cylinder.
	//

	    //
	    // Check for forced floppy emulated device and change LBA accordingly
	    //
	    if (fpDevInfo->bEmuType == USB_EMU_FORCED_FDD) {
            if  (!(gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI)) //(EIP113379+)
    	        //
    	        // Skip first track in case of floppy emulation
    	        //
    	        dStartLba += fpDevInfo->bHiddenSectors;
	    }

	    //
	    // Change big endian format (INTEL) to little endian format
	    //
	    dStartLba = dabc_to_abcd(dStartLba);

	    fpCmdBuffer->dLba = dStartLba;

        //
        // Check the validity of the block size
        //
        if (fpDevInfo->wBlockSize != 0xFFFF) {
            //
            // Change big endian format (INTEL) to little endian format
            //
            fpCmdBuffer->wTransferLength =
                (UINT16)((fpDataStruc->wNumBlks << 8) + (fpDataStruc->wNumBlks >> 8));
            //
            // Verify command does not need delay
            //
            gUsbData->wBulkDataXferDelay = 0;

            //
            // Calculate number of bytes to transfer (for verify command nothing
            // to read/write.
            //
            dBytesToRW = 0;
            if (bOpCode != COMMON_VERIFY_OPCODE) {
                //
                // Read/write command may need long time delay
                //
                gUsbData->wBulkDataXferDelay = 20000;
                dBytesToRW = (UINT32)fpDataStruc->wNumBlks * (UINT32)fpDevInfo->wBlockSize;
            }

            //
            // Set the direction properly
            //
            bDir = (UINT8)((bOpCode == COMMON_WRITE_10_OPCODE)? 0 : BIT7);

            //
            // Fill the common bulk transaction structure
            // Fill Command buffer address & size
            //
            gUsbData->stMassXactStruc.fpCmdBuffer = (UINT8*)fpCmdBuffer;
                                        //(EIP51158+)>
            if (fpDevInfo->bSubClass == SUB_CLASS_SCSI) {
	        	gUsbData->stMassXactStruc.bCmdSize = 0x0A;	//SBC-3_60
            } else {
                gUsbData->stMassXactStruc.bCmdSize = sizeof (COMN_RWV_CMD);
            }
										//<(EIP51158+)
            gUsbData->stMassXactStruc.bXferDir = bDir;
            gUsbData->stMassXactStruc.fpBuffer = (UINT8*)(UINTN)fpDataStruc->fpBufferPtr;
            gUsbData->stMassXactStruc.wPreSkip = fpDataStruc->wPreSkipSize;
            gUsbData->stMassXactStruc.wPostSkip = fpDataStruc->wPostSkipSize;
            gUsbData->stMassXactStruc.dLength = dBytesToRW;

            dData = USBMassIssueMassTransaction(fpDevInfo);

            if (dData) {    // Some data processed. Set return value
                //
                // Apacer USB flash drive (Model:Handy Steno HT202) workaround.
                // Apacer USB flash drive's contents will lost if write data to device and
                // reboot system immediately.
                // Flush device ouput FIFO could avoid this problem.
                // This issue is found with N-Vidia(CK8,CK8) and SiS963 chipsets, and might
                // happen with other chipsets.
                //
                if((bOpCode == COMMON_WRITE_10_OPCODE) &&
                        (fpDevInfo->wVendorId == 0x0B1131005)) {
                    USBMassReadSector(
                        fpDevInfo,
                        0,
                        gUsbData->fpUSBMassConsumeBuffer);
                }

                // TODO:: ApacerHT202Workaround

                //
                // Bug fix for installing Linux from USB CD-ROM.
                // Linux64Bit Boot
                // If data read is 64K or higher return 0FFFFh
                //
                if(dData >= 0x010000) {
                    dData = 0xFFFF;
                }

                wRetCode = (UINT16)dData;
                //
                // Check for forced floppy emulated device
                //
                if ((fpDevInfo->bEmuType == USB_EMU_FORCED_FDD) &&
                     (bOpCode == COMMON_READ_10_OPCODE) &&
                     (fpDataStruc->dStartLBA == 0) &&       //(EIP113379)
                    !(gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI) ) {     //(EIP113379+)
                    //
                    // This is a floppy emulated ZIP drive, with read to
                    // first sector. Update the boot record so that floppy
                    // emulation is okay.
                    //
                    // Force #of hidden sectors to 0
                    //
                    *(UINT32*)((UINTN)fpDataStruc->fpBufferPtr + 0xB + 0x11) = 0;

                    //
                    // FreeDOS workaround
                    //
                    if ((*(UINT32*)((UINTN)fpDataStruc->fpBufferPtr+3)==0x65657246) &&  // 'eerF'
                        (*(UINT32*)((UINTN)fpDataStruc->fpBufferPtr+7)==0x20534F44) &&  // ' SOD'
                        (*(UINT32*)((UINTN)fpDataStruc->fpBufferPtr+0x3A)!=0x20202032)) {				//(EIP61388)
                        *(UINT16*)((UINTN)fpDataStruc->fpBufferPtr+0x42) =
                            *(UINT16*)((UINTN)fpDataStruc->fpBufferPtr+0x42)-(UINT16)fpDevInfo->bHiddenSectors;
                        *(UINT16*)((UINTN)fpDataStruc->fpBufferPtr+0x46) =
                            *(UINT16*)((UINTN)fpDataStruc->fpBufferPtr+0x46)-(UINT16)fpDevInfo->bHiddenSectors;
                        *(UINT16*)((UINTN)fpDataStruc->fpBufferPtr+0x4A) =
                            *(UINT16*)((UINTN)fpDataStruc->fpBufferPtr+0x4A)-(UINT16)fpDevInfo->bHiddenSectors;
                    }
                    //
                    // Force physical drive# to 0
                    // For FAT32, physical drive number is present in offset 40h
                    //
                    if ((*(UINT32*)((UINTN)fpDataStruc->fpBufferPtr + 0x52)) ==
                                        0x33544146) {       // "3TAF", FAT3
                        *(UINT8*)((UINTN)fpDataStruc->fpBufferPtr + 0x40) = 0;
                    }
                    else {
                        *(UINT8*)((UINTN)fpDataStruc->fpBufferPtr + 0x24) = 0;
                    }
                }
                break;  // dData ready

            }
            else {  // Error condition: dData = 0, wRetCode = 0
                //
                // Check for error
                //
                dSenseData = USBMassRequestSense(fpDevInfo);
                fpDataStruc->dSenseData = dSenseData;
                dData = dSenseData;

                //
                // Check for write protect error code
                //
                if ((UINT8)dSenseData == 7) break;

                if ((bOpCode == COMMON_VERIFY_OPCODE) && (!dSenseData)) {
                    //
                    // This is verify command so no data to send or read and
                    // also sense data is 0. So set return value to success.
                    //
                    wRetCode = 0xFFFF;
                    break;
                }
            }
        }   // fpDevInfo->wBlockSize != 0xFFFF

        //
        // UPRCC_ProceedIfRW
        // May be drive error, try to correct it
        // Check whether the drive is ready for read/write/verify command
        //
        dData = USBMassCheckDeviceReady(fpDevInfo);
        fpDataStruc->dSenseData = dData;

        if (dData) {
            break;  // Return error
        }

        MemSet((UINT8*)fpCmdBuffer, sizeof(COMN_RWV_CMD), 0);
    }   // Fof loop

    USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_RWV_CMD));

    return wRetCode;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassStartUnitCommand
//
// DESCRIPTION: This function sends the start unit command to the mass device
//
// PARAMETERS:  fpDevInfo   Pointer to DeviceInfo structure
//
// RETURN:      Sense data: 0 - Success, <>0 - Error
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
USBMassStartUnitCommand (DEV_INFO* fpDevInfo)
{
    COMMON_START_STOP_UNIT_CMD  *fpCmdBuffer;
//  MASS_START_STOP_UNIT        *fpStartData;

    USB_DEBUG (DEBUG_LEVEL_5, "USBMProStartUnitCommand ....  \n");

    //
    // Check the compatibility flag for start unit command not supported
    //
    if (fpDevInfo->wIncompatFlags & USB_INCMPT_START_UNIT_NOT_SUPPORTED) {
        return  USB_SUCCESS;
    }

    //
    // Allocate memory for the command buffer
    //
    fpCmdBuffer = USB_MemAlloc(GET_MEM_BLK_COUNT_STRUC(COMMON_START_STOP_UNIT_CMD));
    if (!fpCmdBuffer) {
        return  USB_ERROR;
    }

    //
    // Load command into (just allocated) mass command buffer
    //
    fpCmdBuffer->bOpCode = COMMON_START_STOP_UNIT_OPCODE;
    fpCmdBuffer->bStart = 1;

    //
    // Clear the common bulk transaction structure
    //
    USBMassClearMassXactStruc();
    gUsbData->wBulkDataXferDelay = 10000;  // Start unit command may need long time delay
    //
    // Fill the common bulk transaction structure
    //
    gUsbData->stMassXactStruc.fpCmdBuffer = (UINT8*)fpCmdBuffer;
                                        //(EIP51158+)>
    if (fpDevInfo->bSubClass == SUB_CLASS_SCSI) {
		gUsbData->stMassXactStruc.bCmdSize = 0x06;	//SBC-3_77
    } else {
    	gUsbData->stMassXactStruc.bCmdSize = sizeof (COMMON_START_STOP_UNIT_CMD);
    }
										//<(EIP51158+)
    USBMassIssueMassTransaction(fpDevInfo);

    //
    // No data to read/write. So do not process return code.
    // Check and free command buffer
    //
    USB_MemFree(fpCmdBuffer,GET_MEM_BLK_COUNT_STRUC(COMMON_START_STOP_UNIT_CMD));

    return USBMassRequestSense(fpDevInfo);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassModeSense
//
// DESCRIPTION: This function requests the mode sense data page number 5 from
//              the USB mass storage device
//
// PARAMETERS:  fpDevInfo   Pointer to DeviceInfo structure
//
// RETURN:      USB_SUCCESS/USB_ERROR on Success/Failure
//              fpModeSenseData     Pointer to the mode sense data
//                  dSenseData  Sense data
//                  bNumHeads   Number of heads
//                  wNumCylinders   Number of cylinders
//                  bNumSectors Number of sectors
//                  wBytesPerSector Number of bytes per sector
//                  bMediaType  Media type
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassModeSense(
    DEV_INFO        *fpDevInfo,
    MASS_MODE_SENSE *fpModeSenseData)
{
    UINT32                  dData;
    UINT8                   bRetCode;
    COMN_MODE_SENSE_10CMD   *fpCmdBuffer;
    MODE_SENSE_10_HEADER    *fpModeSense10_Header;
    PAGE_CODE_5             *fpPageCode5;

    dData = 0;
    bRetCode = USB_ERROR;

    fpCmdBuffer = USB_MemAlloc(GET_MEM_BLK_COUNT_STRUC(COMN_MODE_SENSE_10CMD));
    if (!fpCmdBuffer) {
        return  USB_ERROR;
    }

    //
    // Load command into (just allocated) mass command buffer
    //
    fpCmdBuffer->bOpCode = COMMON_MODE_SENSE_10_OPCODE;
    fpCmdBuffer->wAllocLength = 0x2800; // Allocation Length = 40 bytes (0x28)
    fpCmdBuffer->bPageCode = 5; // Page code

    //
    // Clear the common bulk transaction structure
    //
    USBMassClearMassXactStruc();

    //
    // Fill the common bulk transaction structure
    //
    gUsbData->stMassXactStruc.fpCmdBuffer = (UINT8*)fpCmdBuffer;
                                        //(EIP51158+)>
    if (fpDevInfo->bSubClass == SUB_CLASS_SCSI) {
		gUsbData->stMassXactStruc.bCmdSize = 0x0A;	//SPC-4_280
    } else {
    	gUsbData->stMassXactStruc.bCmdSize = sizeof (COMN_MODE_SENSE_10CMD);
    }
										//<(EIP51158+)
    gUsbData->stMassXactStruc.bXferDir = BIT7;     // IN
    gUsbData->stMassXactStruc.fpBuffer = gUsbData->fpUSBTempBuffer;
    gUsbData->stMassXactStruc.dLength = 0x28;

    //
    // Bulk in, with temp buffer & 40 bytes of data to read
    //
    dData = USBMassIssueMassTransaction(fpDevInfo);

    if (!dData) {
        USBMassRequestSense( fpDevInfo );
        USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_MODE_SENSE_10CMD));
        return  USB_ERROR;
    }

    //
    // Fill in the output data
    //
    fpModeSense10_Header = (MODE_SENSE_10_HEADER*)gUsbData->fpUSBTempBuffer;

    //
    // Process media type
    //
    fpModeSenseData->bMediaType = fpModeSense10_Header->bMediaType;

    //
    // Position to the correct page code starting location
    //
    fpPageCode5 = (PAGE_CODE_5*)((UINT8*)fpModeSense10_Header +
                                        fpModeSense10_Header->wBlkDescSize +
                                        sizeof (MODE_SENSE_10_HEADER));
//  USB_DEBUG (DEBUG_LEVEL_3, "USBMassModeSense ....  fpPageCode5->bPageCode %x\n",fpPageCode5->bPageCode);

    bRetCode = USB_ERROR;
    if(fpPageCode5->bPageCode == 5) {
        //
        // Process number of bytes per sector (the block size)
        //
        fpModeSenseData->wBytesPerSector = (UINT16)((fpPageCode5->wBlockSize << 8)
                                            + (fpPageCode5->wBlockSize >>8));
        //
        // Process number of heads and number of sectors/track
        //
        fpModeSenseData->bNumHeads = fpPageCode5->bHeads;
        fpModeSenseData->bNumSectors = fpPageCode5->bSectors;

        //
        // Process number of cylinders
        //
        fpModeSenseData->wNumCylinders  = (UINT16)((fpPageCode5->wCylinders << 8)
                                        + (fpPageCode5->wCylinders >> 8));
        bRetCode = USB_SUCCESS;
    }

    fpModeSenseData->dSenseData = USBMassRequestSense( fpDevInfo );

    USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_MODE_SENSE_10CMD));
// USB_DEBUG (DEBUG_LEVEL_5, "USBMProModeSense ....  wRetCode %x\n",wRetCode);

    return bRetCode;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassRequestSense
//
// DESCRIPTION: This function sends request sense command and returns
//      the sense key information
//
// PARAMETERS:  fpDevInfo   Pointer to DeviceInfo structure
//
// RETURN:      Sense data
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
USBMassRequestSense(DEV_INFO* fpDevInfo)
{
    UINT32                  dData;
    UINT8                   *fpDataBuffer;
    COMMON_REQ_SENSE_CMD    *fpCmdBuffer;

    //
    // Allocate memory for the command buffer
    //
    fpCmdBuffer = (COMMON_REQ_SENSE_CMD*)USB_MemAlloc(GET_MEM_BLK_COUNT_STRUC(COMMON_REQ_SENSE_CMD));
    if(!fpCmdBuffer) {
        return USB_ERROR;   // Error - return no sense data <>0
    }

    fpDataBuffer = USB_MemAlloc(GET_MEM_BLK_COUNT(1));
    if(!fpDataBuffer) {
        return USB_ERROR;   // Error - return no sense data <>0
    }

    //
    // Load command into (just allocated) mass command buffer
    //
    fpCmdBuffer->bOpCode = COMMON_REQUEST_SENSE_OPCODE;
    fpCmdBuffer->bAllocLength = 0x12;   // Length of transfer

    USBMassClearMassXactStruc();    // Clear the common bulk transaction structure

    //
    // Fill the common bulk transaction structure
    //
    gUsbData->stMassXactStruc.fpCmdBuffer  = (UINT8*)fpCmdBuffer;
                                        //(EIP51158+)>
    if (fpDevInfo->bSubClass == SUB_CLASS_SCSI) {
		gUsbData->stMassXactStruc.bCmdSize = 0x06;	//SPC-4_350
    } else {
    	gUsbData->stMassXactStruc.bCmdSize = sizeof (COMMON_REQ_SENSE_CMD);
    }
										//<(EIP51158+)
    gUsbData->stMassXactStruc.bXferDir = BIT7;     // IN
    gUsbData->stMassXactStruc.fpBuffer = fpDataBuffer;
    gUsbData->stMassXactStruc.dLength  = 0x12;

    //
    // Bulk in, with locally allocated temp buffer & 12h bytes of data to read
    //
    dData = USBMassIssueMassTransaction(fpDevInfo);

    if(dData) {
        //
        // Form the return value:
        //      Bit 0..7    - Sense key (offset 002d)
        //      Bit 8..15   - ASC code (offset 012d)
        //      Bit 16..23  - ASCQ code (offset 013d)
        //
        dData = (UINT32)(fpDataBuffer[2] +
                    (fpDataBuffer[12] << 8) +
                    (fpDataBuffer[13] << 16));
        USBMassSenseKeyParsing(fpDevInfo, dData);
    }
							//(EIP20863+)>
	else
		dData = USB_ERROR;
							//<(EIP20863+)

    USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMMON_REQ_SENSE_CMD));
    USB_MemFree(fpDataBuffer, GET_MEM_BLK_COUNT(1));

    return dData;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassSenseKeyParsing
//
// DESCRIPTION: Translate USB sense key to USB MassStorage status.
//
// PARAMETERS:  fpDevInfo   Pointer to DeviceInfo structure
//                 dCode[23..16]   ASCQ
//                 dCode[15..08]   ASC
//                 dCode[07..00]   Sense Code

//
// RETURN:      Sense data
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassSenseKeyParsing(DEV_INFO* fpDevInfo, UINT32 dCode)
{
    if ((UINT16)dCode == 0x3A02) {		//(EIP86793)
        fpDevInfo->bLastStatus &= ~USB_MASS_MEDIA_PRESENT;
    }
    if((UINT16)dCode == 0x2806) {
        fpDevInfo->bLastStatus |= (USB_MASS_MEDIA_PRESENT | USB_MASS_MEDIA_CHANGED);
    }
										//(EIP86125+)>
    if(dCode == 0) {
        fpDevInfo->bLastStatus |= USB_MASS_MEDIA_PRESENT;
    }
										//<(EIP86125+)
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassTestUnitReady
//
// DESCRIPTION: This function sends test unit ready command
//
// PARAMETERS:  fpDevInfo   Pointer to DeviceInfo structure
//
// RETURN:      Sense data
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
USBMassTestUnitReady(DEV_INFO* fpDevInfo)
{
    COMN_TEST_UNIT_READY_CMD    *fpCmdBuffer;

    fpCmdBuffer = (COMN_TEST_UNIT_READY_CMD*)USB_MemAlloc(
                    GET_MEM_BLK_COUNT_STRUC(COMN_TEST_UNIT_READY_CMD));
    if(!fpCmdBuffer) {
        return USB_ERROR;       // Error - return no sense data
    }

    fpCmdBuffer->bOpCode = COMMON_TEST_UNIT_READY_OPCODE;
    USB_DEBUG (DEBUG_LEVEL_5, "USBMassTestUnitReady ....  \n");

    USBMassClearMassXactStruc();    // Clear the common bulk transaction structure

    //
    // Fill the common bulk transaction structure
    //
    gUsbData->stMassXactStruc.fpCmdBuffer = (UINT8*)fpCmdBuffer;
                                        //(EIP51158+)>
    if (fpDevInfo->bSubClass == SUB_CLASS_SCSI) {
		gUsbData->stMassXactStruc.bCmdSize = 0x06;	//SPC-4_368
    } else {
    	gUsbData->stMassXactStruc.bCmdSize = sizeof (COMN_TEST_UNIT_READY_CMD);
    }
										//<(EIP51158+)
    USBMassIssueMassTransaction(fpDevInfo);

    USB_MemFree(fpCmdBuffer, GET_MEM_BLK_COUNT_STRUC(COMN_TEST_UNIT_READY_CMD));

    return USBMassRequestSense(fpDevInfo);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassCheckDeviceReady
//
// Description: This function makes sure the device is ready for next
//      command
//
// Input:   fpDevInfo   Pointer to DeviceInfo structure
//
// Output:  Sense code
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
USBMassCheckDeviceReady (DEV_INFO* fpDevInfo)
{
    UINT8   count, nomedia_count;
    UINT8   NotReadyCount; 				//(EIP101623+)
    UINT32  dData = 0;

    count = gUsbData->bUSBStorageDeviceDelayCount;
    nomedia_count = 3;
    NotReadyCount = 3;  				//(EIP101623+)
    while (count) {
        if (fpDevInfo->wIncompatFlags & USB_INCMPT_TEST_UNIT_READY_FAILED) {
            break;  // consider device is ready
        }

        //
        // Issue test unit ready command and check the return value
        //
        dData = USBMassTestUnitReady( fpDevInfo );
//USB_DEBUG(DEBUG_LEVEL_3, "(%d)tur..%x ", fpDevInfo->bDeviceAddress, dData);
        if ((UINT8)dData == 0) { // Device ready
            break;
        }
        //
        // Device is not ready.
        // Check for getting ready/reset command occurence in dData:
        //      Bit 0..7    - Sense Code
        //      Bit 8..15   - Additional Sense Code (ASC)
        //      Bit 16..23  - Additional Sense Code Qualifier (ASCQ)
        //
        if ((UINT16)dData == 0x2806) {
            //
            // Send Start/Stop Unit command to UFI class device only
            //
            if (fpDevInfo->bSubClass == SUB_CLASS_UFI) {
                USBMassStartUnitCommand (fpDevInfo);
            }
            FixedDelay(100 * 1000);        // 100 msec delay
            count--;
            continue;
        }
        if ((UINT16)dData == 0x3A02) {			// Media is not present
            nomedia_count--;
            if (nomedia_count == 0) return  dData;  // No media
            FixedDelay(20 * 1000);        // 20 msec delay
            count--;
            continue;
        }

		if (dData == 0x020402)
		{
			USBMassStartUnitCommand (fpDevInfo);
			FixedDelay(100 * 1000);
			count--;
			continue;
		}

		if ((UINT16)dData == 0x1103) {
			FixedDelay(100 * 1000);
			count--;
			continue;
		}

        //
        // Check whether we can recover from this error condition
        // Currently only recoverable error condition are
        // 1. Device is getting ready (010402)
        // 2. Device reset occurred (002906)
        //
        if (dData != 0x010402) {
            //
            // Check for write protected command
            //
            if ( (UINT8)dData == 7 ) {
                break;
            }
            if (((UINT8)dData != 0x06) && ((UINT8)dData != 0x02)) {
                return  dData;
            }
        }

                                        //(EIP101623+)>
        if (dData == 0x02) {			
            NotReadyCount--;
            if (NotReadyCount == 0) return  dData;  
            FixedDelay(20 * 1000); // 20 msec delay
            count--;
            continue;
        }
                                        //<(EIP101623+)
            
        //
        // Prepare for the next itaration
        // Delay for the device to get ready
        //
        FixedDelay(1000 * 1000);       // 1 sec delay
        count--;
    }   // while
                                        //(EIP53416+)>
    if (count == 0) {
        return dData;
    }
                                        //<(EIP53416+)
    return USBMassUpdateDeviceGeometry(fpDevInfo);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassUpdateParamUsingModeSense
//
// Description: This function obtains the device geometry from the device
//              using mode sense command and updates the global variables
//
// Input:   Pointer to DeviceInfo structure
//
// Output:  USB_ERROR   On error
//          USB_SUCCESS On success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassUpdateParamUsingModeSense(DEV_INFO* fpDevInfo)
{
    MASS_MODE_SENSE ModeSenseData;

    gUsbData->wModeSenseCylinders = gUsbData->bModeSenseHeads =
                                gUsbData->bModeSenseSectors = 0;
    //
    // Check the compatibility flag for mode sense support
    //
    if (fpDevInfo->wIncompatFlags & USB_INCMPT_MODE_SENSE_NOT_SUPPORTED) {
        return  USB_SUCCESS;
    }

    //
    // CDROM devices never support mode sense page code 5 (Flexible disk page)
    // so skip it
    //
    if (fpDevInfo->bStorageType == USB_MASS_DEV_CDROM) {
        return  USB_ERROR;
    }

    //
    // Issue mode sense command
    //
    if (USBMassModeSense(fpDevInfo, &ModeSenseData)) {
USB_DEBUG(DEBUG_LEVEL_3, "ms..err ");
        return  USB_ERROR;
    }

    //
    // Mode sense is supported. Update the local structure.
    //
    gUsbData->wModeSenseCylinders  = ModeSenseData.wNumCylinders;  // Number of cylinders
    gUsbData->bModeSenseHeads      = ModeSenseData.bNumHeads;      // Number of heads
    gUsbData->bModeSenseSectors    = ModeSenseData.bNumSectors;    // Number of sectors
    gUsbData->wModeSenseBlockSize  = ModeSenseData.wBytesPerSector;// Number of bytes per sector
    gUsbData->bDiskMediaType       = ModeSenseData.bMediaType;     // Media type


USB_DEBUG(DEBUG_LEVEL_4, "ms..%x %x %x %x %x ",
    gUsbData->wModeSenseCylinders,
    gUsbData->bModeSenseHeads,
    gUsbData->bModeSenseSectors,
    gUsbData->wModeSenseBlockSize,
    gUsbData->bDiskMediaType
);

    if (fpDevInfo->bStorageType == USB_MASS_DEV_HDD) {
        gUsbData->bDiskMediaType = USB_UNKNOWN_MEDIA_TYPE;
    }

    //
    // Calculate and update Max LBA
    //
    gUsbData->dModeSenseMaxLBA =
            (UINT32)(ModeSenseData.wNumCylinders *
            ModeSenseData.bNumHeads * ModeSenseData.bNumSectors);
    //
    // Set the flag indicating mode sense is executed
    //
    gUsbData->bGeometryCommandStatus |= MODE_SENSE_COMMAND_EXECUTED;

    return  USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassUpdateParamUsingReadCapacity
//
// Description: This function obtains the device geometry from the device
//              using read capacity command and updates the global variables
//
// Input:   Pointer to DeviceInfo structure
//
// Output:  USB_ERROR   On error
//          USB_SUCCESS On success
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassUpdateParamUsingReadCapacity(DEV_INFO* fpDevInfo)
{
    UINT8   bSectors, bHeads;

    //
    // Either mode sense not supported or failed. Try read capacity
    // Issue read capacity command
    //
    if (USBMassReadCapacityCommand(fpDevInfo)) {
        return  USB_ERROR;
    }

    //
    // Set the flag indicating read capacity is executed
    //
    gUsbData->bGeometryCommandStatus |= READ_CAPACITY_COMMAND_EXECUTED;

    //
    // Max LBA & block size are updated in MassDeviceInfo structure
    //
    if ( fpDevInfo->dMaxLba < 0x4000 ) {    //  last LBA < 16MB
        switch ( fpDevInfo->dMaxLba ) {
            case USB_144MB_FDD_MAX_LBA:
                gUsbData->bReadCapHeads    = USB_144MB_FDD_MAX_HEADS;
                gUsbData->bReadCapSectors  = USB_144MB_FDD_MAX_SECTORS;
                gUsbData->wReadCapCylinders= USB_144MB_FDD_MAX_CYLINDERS;
                gUsbData->bDiskMediaType   = USB_144MB_FDD_MEDIA_TYPE;
                return  USB_SUCCESS;

            case USB_720KB_FDD_MAX_LBA:
                gUsbData->bReadCapHeads    = USB_720KB_FDD_MAX_HEADS;
                gUsbData->bReadCapSectors  = USB_720KB_FDD_MAX_SECTORS;
                gUsbData->wReadCapCylinders= USB_720KB_FDD_MAX_CYLINDERS;
                gUsbData->bDiskMediaType   = USB_720KB_FDD_MEDIA_TYPE;
                return  USB_SUCCESS;
        }
    }

    //
    // Convert to CHS
    //
    gUsbData->wReadCapBlockSize = fpDevInfo->wBlockSize;

    //
    // Do CHS conversion
    // Use fixed sectors/track & heads for CHS conversion
    //
    if (fpDevInfo->dMaxLba < 0x400) {   // < 512 KB
        bSectors    = 1;
        bHeads      = 1;
    }
    else {
        if ( fpDevInfo->dMaxLba < 0x200000 ) {  // < 1GB
            bSectors    = USB_FIXED_LBA_SPT_BELOW_1GB;
            bHeads      = USB_FIXED_LBA_HPT_BELOW_1GB;
        }
        else {                                  // > 1GB
            bSectors    = USB_FIXED_LBA_SPT_ABOVE_1GB;
            bHeads      = USB_FIXED_LBA_HPT_ABOVE_1GB;
        }
    }

    gUsbData->bReadCapSectors  = bSectors;
    gUsbData->bReadCapHeads    = bHeads;

    //
    // Calculate number of cylinders Cyl = LBA/(Head*Sec)
    //
    gUsbData->wReadCapCylinders = (UINT16)(fpDevInfo->dMaxLba / (bSectors * bHeads));

    return  USB_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassUpdateDeviceGeometry
//
// Description: This function updates the device geometry information
//
// Input:       Pointer to device info structure
//
// Output:      USB_SUCCESS or USB_ERROR
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassUpdateDeviceGeometry (DEV_INFO* fpDevInfo)
{
    UINT32  dMaxLba;
    UINT8   bHeads, bSectors;
    UINT16  wCylinders;
    UINT8   bStatus;

    //
    // Try to update geometry if it is not valid
    // "Valid" block size is 1...FFFE
    // Additional check added to ensure the head, sector, and cylinder values are non-zero.
    //
                            //(EIP13457+)>
    if ((fpDevInfo->bHeads!=0)
        && (fpDevInfo->bSectors!=0)
        && (fpDevInfo->wCylinders!=0)&&
        (!((fpDevInfo->bLastStatus & USB_MASS_GET_MEDIA_FORMAT)&&
        (fpDevInfo->bSubClass == SUB_CLASS_UFI)))) {

        fpDevInfo->bLastStatus &= ~USB_MASS_GET_MEDIA_FORMAT;

        if (fpDevInfo->wBlockSize && (fpDevInfo->wBlockSize != 0xFFFF)) {
            return USB_SUCCESS;
        }
    }

    fpDevInfo->bLastStatus &= ~USB_MASS_GET_MEDIA_FORMAT;

    //
    // Set default values for the global variables
    //
    gUsbData->bDiskMediaType = USB_UNKNOWN_MEDIA_TYPE;
    gUsbData->bGeometryCommandStatus &= ~(MODE_SENSE_COMMAND_EXECUTED |
                    READ_CAPACITY_COMMAND_EXECUTED);

    //
    // Get disk geometry using Mode Sense
    //
    if(fpDevInfo->bSubClass == SUB_CLASS_UFI)   //(EIP94060)
	    USBMassUpdateParamUsingModeSense(fpDevInfo);

    //
    // Get disk geometry using Read Capacity
    //
    bStatus = USBMassUpdateParamUsingReadCapacity(fpDevInfo);

    //
    // Parameters are obtained and stored in respective global variables;
    // check whether any of the commands executed.
    //
    if (!(gUsbData->bGeometryCommandStatus & (READ_CAPACITY_COMMAND_EXECUTED |
                        MODE_SENSE_COMMAND_EXECUTED)))  {
USB_DEBUG(DEBUG_LEVEL_3, "-error\n");
        return USB_ERROR;
    }

    //
    // Check whether read capacity is executed. If so, then max LBA & block size
    // are already updated in the MassDeviceInfo structure. If not update it using
    // mode sense parameters
    //
    if (!(gUsbData->bGeometryCommandStatus & READ_CAPACITY_COMMAND_EXECUTED)) {
        //
        // At this point we made sure atleast one of the command (Mode sense or Read
        // Capacity) was executed. So if one command is not executed then other
        // command is surely executed.
        //

        //
        // Update the max LBA & block size using mode sense parameters
        //
        fpDevInfo->wBlockSize = gUsbData->wModeSenseBlockSize;
        fpDevInfo->dMaxLba = gUsbData->dModeSenseMaxLBA;
USB_DEBUG(DEBUG_LEVEL_4, "size %x lba %x\n", fpDevInfo->wBlockSize, fpDevInfo->dMaxLba);
    }

    //
    // Update the media type byte
    //
    fpDevInfo->bMediaType = gUsbData->bDiskMediaType;

    //
    // Check whether mode sense is executed. If so, then update CHS from mode
    // sense value or else update from read capacity values.
    //

    //
    // Update the CHS values using mode sense parameters
    //
    bHeads      = gUsbData->bModeSenseHeads;
    bSectors    = gUsbData->bModeSenseSectors;
    wCylinders  = gUsbData->wModeSenseCylinders;

//  if ((gUsbData->bGeometryCommandStatus & MODE_SENSE_COMMAND_EXECUTED) &&
    if ((bHeads * bSectors * wCylinders) == 0) {
        //
        // Update the CHS values using read capacity parameters
        //
        bHeads      = gUsbData->bReadCapHeads;
        bSectors    = gUsbData->bReadCapSectors;
        wCylinders  = gUsbData->wReadCapCylinders;
    }

USB_DEBUG (DEBUG_LEVEL_4, "Cyl-%x, Hds-%x, Sec-%x", wCylinders, bHeads, bSectors);

    fpDevInfo->bHeads       = bHeads;
    fpDevInfo->bSectors     = bSectors;
    fpDevInfo->wCylinders   = wCylinders;

    //
    // Update Efi BlockIo device
    //
    if (gUsbData->dUSBStateFlag & USB_FLAG_RUNNING_UNDER_EFI) {
        if (fpDevInfo->MassDev) {
//          (*(EFI_BLOCK_IO_PROTOCOL*)fpDevInfo->MassDev).Media->MediaId++;
            (*(EFI_BLOCK_IO_PROTOCOL*)fpDevInfo->MassDev).Media->BlockSize = fpDevInfo->wBlockSize;
            (*(EFI_BLOCK_IO_PROTOCOL*)fpDevInfo->MassDev).Media->LastBlock = fpDevInfo->dMaxLba;
        }
    }

    //
    // Calculate non-LBA CHS values from max LBA
    //
    dMaxLba = fpDevInfo->dMaxLba;

    //
    // Do not translate sectors for non HDD devices
    //
    if ((!fpDevInfo->bStorageType) || (fpDevInfo->bStorageType == USB_MASS_DEV_HDD)) {
        //
        // If Total number of sectors < 1032192(0FC000h) CHS translation is not
        // needed
        //
        if ( dMaxLba >= 0xFC000 ) {
            bSectors    = 63;
            bHeads      = 32;
            //
            // If Total number of sectors < 2064384(01F8000h) then use
            // 63 Sec/track and 32 head for translation
            //
            if ( dMaxLba >= 0x01F8000 ) {
                bHeads = 64;
                //
                // If Total number of sectors < 4128768(03F0000h) then use
                // 63 Sec/track and 64 head for translation
                //
                if (dMaxLba >= 0x03F0000) {
                    bHeads = 128;
                    //
                    // If Total number of sectors < 8257536(07E0000h) then use
                    // 63 Sec/track and 128 head for translation else use 255 heads
                    //
                    if (dMaxLba >= 0x7E0000) {
                        bHeads      = 255;
                        dMaxLba = fpDevInfo->dMaxLba;
                    }
                }
            }
        }

        //
        // In any case, check the parameters for maximum values allowed by BIOS and
        // ATA specs (that is, 1024 cylinders, 16 heads and 63 sectors per track)
        //
        for (;;) {
            //
            // Calculate translated number of cylinders
            //
            wCylinders = (UINT16)(dMaxLba/(bHeads * bSectors));

            //
            // Check whether number of cylinders is less than or equal to 1024
            //
            if (wCylinders <= 1024) break;

            //
            // Cylinders are getting larger than usually supported try increasing
            // head count keeping cylinders within safe limit
            //
            wCylinders = 1024;
            if (bHeads == 0xFF) break;  // Heads limit reached
            //
            // Double number of heads
            //
            bHeads <<= 1;
            if (!bHeads)
                bHeads = 0xFF;
        }
    }

    //
    // Save the parameters
    //
    fpDevInfo->bNonLBAHeads     = bHeads;
    fpDevInfo->bNonLBASectors   = bSectors;
    fpDevInfo->wNonLBACylinders = wCylinders;

USB_DEBUG(DEBUG_LEVEL_5, "BPS %d H %d S %d C %d MT %d\n",
        fpDevInfo->wBlockSize,
        fpDevInfo->bHeads,
        fpDevInfo->bSectors,
        fpDevInfo->wCylinders,
        fpDevInfo->bMediaType);

    return  USB_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassIssueBOTTransaction
//
// Description: This function performs a mass storage transaction using bulk
//      only transport (BOT) protocol.
//
// Input:   Pointer to DeviceInfo structure
//          stMassXactStruc
//              pCmdBuffer  Pointer to command buffer
//              bCmdSize    Size of command block
//              bXferDir    Transfer direction
//              fpBuffer    Data buffer far pointer
//              dwLength    Amount of data to be transferred
//              wPreSkip    Number of bytes to skip before data
//              wPostSkip   Number of bytes to skip after data
//
// Output:  Amount of data actually transferred
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
USBMassIssueBOTTransaction(DEV_INFO* fpDevInfo)
{
    UINT32  dData;

    dData = USBMassSendBOTCommand(fpDevInfo);   // Send the command control transfer

    if (!dData) {
        //
        // Check for stall/timedout condition
        //
        if (gUsbData->bLastCommandStatus & (USB_BULK_STALLED + USB_BULK_TIMEDOUT)) {
            //
            // Perform USB BOT reset recovery
            //
            USBMassBOTResetRecovery(fpDevInfo);
            return 0;
        }
        else {
            return  0;  // Unknown error exit
        }
    }

    if (!gUsbData->stMassXactStruc.dLength) {  // No data
        if(gUsbData->wBulkDataXferDelay) {
            //
            // Issue some delay
            //
            FixedDelay(100 * 1000);
            gUsbData->wBulkDataXferDelay   = 0;
        }
        //
        // Get the status for the last transfer
        //
        USBMassGetBOTStatus(fpDevInfo);
        return  0;
    }

    //
    // Tranfer the bulk data
    //
    dData = USBMassProcessBulkData(fpDevInfo);  // Actual data size

    //
    // Check for stall/timeout condition
    //
    if (!(gUsbData->bLastCommandStatus & (USB_BULK_STALLED + USB_BULK_TIMEDOUT))) {
        //
        // Get the status for the last transfer
        //
        USBMassGetBOTStatus(fpDevInfo);
		if (gUsbData->bLastCommandStatus & USB_BULK_TIMEDOUT) {
			return 0;
		} else {
        	return dData;
		}
    }

    //
    // Check for time out condition
    //
    if (gUsbData->bLastCommandStatus & USB_BULK_TIMEDOUT) {
        //
        // Perform USB BOT reset recovery
        //
        USBMassBOTResetRecovery(fpDevInfo);
        return 0;
    }

    //
    // Clear endpoint stall
    //
    USBMassClearBulkEndpointStall(fpDevInfo, gUsbData->stMassXactStruc.bXferDir);

    //
    // Get the status for the last transfer
    //
    USBMassGetBOTStatus(fpDevInfo);

    return  dData;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBBOTSendCommand
//
// DESCRIPTION: This function performs a mass storage transaction using bulk
//              only transport (BOT) protocol.
//
// PARAMETERS:  fpDevInfo   Pointer to DeviceInfo structure
//                  bXferDir    Transfer direction
//                  dwDataSize  Amount of data to be transferred
//                  fpCmdBuffer Pointer to the command buffer
//                  bCmdSize    Size of command block

// RETURN:      Amount of data actually transferred
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
USBMassSendBOTCommand(DEV_INFO* fpDevInfo)
{
    UINT8           bCount;
    UINT8           *fpSrc,
                    *fpDest;
    BOT_CMD_BLK     *fpBOTCmdBlk;
    UINT8           bCmdSize;

    fpBOTCmdBlk = (BOT_CMD_BLK*)gUsbData->stMassXactStruc.fpCmdBuffer;

    bCmdSize = gUsbData->stMassXactStruc.bCmdSize;

//    if( !VALID_DEVINFO2( fpDevInfo) )
//        return 0;
    //
    // Make enough space for BOT command block wrapper
    // Move backwards
    //
    fpSrc = gUsbData->stMassXactStruc.fpCmdBuffer + bCmdSize - 1;

    //
    // BOT_COMMAND_BLOCK + end of command
    //
    fpDest = fpSrc + ((UINT8*)fpBOTCmdBlk->aCBWCB - (UINT8*)fpBOTCmdBlk);

    for (bCount = 0; bCount < bCmdSize; bCount++) {
        *fpDest = *fpSrc;
        --fpDest;
        --fpSrc;
    }

    fpDest = gUsbData->stMassXactStruc.fpCmdBuffer;

    //
    // Clear the BOT command block
    //
    for (bCount = 0; bCount < bCmdSize; bCount++) {
        *fpDest = 0x00;
        ++fpDest;
    }

    fpBOTCmdBlk->dCbwSignature      = BOT_CBW_SIGNATURE;
    fpBOTCmdBlk->dCbwTag            = ++(gUsbData->dBOTCommandTag);
    fpBOTCmdBlk->dCbwDataLength     = gUsbData->stMassXactStruc.dLength;
    fpBOTCmdBlk->bmCbwFlags         = gUsbData->stMassXactStruc.bXferDir;
    fpBOTCmdBlk->bCbwLun            = fpDevInfo->bLUN;
    fpBOTCmdBlk->bCbwLength         = bCmdSize;

    return (UINT16)USBMassIssueBulkTransfer(
                    fpDevInfo,
                    0,
                    (UINT8*)fpBOTCmdBlk,
                    sizeof (BOT_CMD_BLK));
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBBOTGetStatus
//
// Description: This function gets the BOT status sequence using
//              bulk IN transfer
//
// Input:   fpDevInfo   Pointer to DeviceInfo structure
//
// Output:  Nothing
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassGetBOTStatus(DEV_INFO* fpDevInfo)
{
										//(EIP90503)>
    UINT8*          fpCmdBuffer;
    UINT16          wData;

    fpCmdBuffer = gUsbData->stMassXactStruc.fpCmdBuffer;

    wData = (UINT16)USBMassIssueBulkTransfer(fpDevInfo, BIT7,
                    fpCmdBuffer, sizeof (BOT_STATUS_BLOCK));
    if ((wData != sizeof (BOT_STATUS_BLOCK))) {
        if (gUsbData->bLastCommandStatus & USB_BULK_STALLED) {
            USBMassClearBulkEndpointStall(fpDevInfo, BIT7);
        }
        wData = (UINT16)USBMassIssueBulkTransfer(fpDevInfo, BIT7,
                    fpCmdBuffer, sizeof (BOT_STATUS_BLOCK));
        if (gUsbData->bLastCommandStatus & USB_BULK_STALLED) {
            USBMassBOTResetRecovery(fpDevInfo);
            return USB_ERROR;
        }
    }

    //
    // Check for valid CSW
    //
    if ((wData != sizeof (BOT_STATUS_BLOCK)) ||
        (((BOT_STATUS_BLOCK*)fpCmdBuffer)->dCswSignature != BOT_CSW_SIGNATURE) ||
        (((BOT_STATUS_BLOCK*)fpCmdBuffer)->dCswTag != gUsbData->dBOTCommandTag)) {
        //USBMassClearBulkEndpointStall(fpDevInfo, BIT7);	//(EIP63308-)
        //USBMassClearBulkEndpointStall(fpDevInfo, BIT0);	//(EIP63308-)
        return USB_ERROR;
    }
										//<(EIP90503)
    //
    // Check for meaningful CSW
    //
    if (((BOT_STATUS_BLOCK*)fpCmdBuffer)->bmCswStatus) {
   		if (((BOT_STATUS_BLOCK*)fpCmdBuffer)->bmCswStatus > 1) {
	        //
	        // Perform reset recovery if BOT status is phase error
	        //
        	USBMassBOTResetRecovery(fpDevInfo);
    	}
		return USB_ERROR;
	}

    return USB_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassBOTResetRecovery
//
// DESCRIPTION: This function performs the BOT reset recovery
//
// PARAMETERS:  fpDevInfo   Pointer to DeviceInfo structure
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID
USBMassBOTResetRecovery(DEV_INFO* fpDevInfo)
{
    (*gUsbData->aHCDriverTable[GET_HCD_INDEX(gUsbData->HcTable
                [fpDevInfo->bHCNumber - 1]->bHCType)].pfnHCDControlTransfer)
                (gUsbData->HcTable[fpDevInfo->bHCNumber - 1],
      																			//(EIP20863)>
                //fpDevInfo, ADSC_OUT_REQUEST_TYPE,
                //(UINT16)fpDevInfo->bInterfaceNum,BOT_RESET_REQUEST_CODE, 0, 0);
				fpDevInfo, ADSC_OUT_REQUEST_TYPE + (BOT_RESET_REQUEST_CODE << 8),
                (UINT16)fpDevInfo->bInterfaceNum, 0, 0, 0);
																				//<(EIP20863)
    USBMassClearBulkEndpointStall(fpDevInfo, BIT7);
    USBMassClearBulkEndpointStall(fpDevInfo, BIT0);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassBOTGetMaxLUN
//
// Description: This function gets the maximum logical unit number(LUN)
//      supported by the device.  It is zero based value.
//
// Input:   Pointer to DeviceInfo structure
//
// Output:  Max LUN supported
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
USBMassBOTGetMaxLUN(DEV_INFO* fpDevInfo)
{
	UINT8	*Buffer = NULL;
	UINT8	MaxLun = 0;

    if (fpDevInfo->wIncompatFlags & USB_INCMPT_GETMAXLUN_NOT_SUPPORTED) {
        return 0;
    }

	Buffer = USB_MemAlloc(1);
	ASSERT(Buffer);
	if (Buffer == NULL) {
		return 0;
	}

    (*gUsbData->aHCDriverTable[GET_HCD_INDEX(gUsbData->HcTable
        [fpDevInfo->bHCNumber - 1]->bHCType)].pfnHCDControlTransfer)
        (gUsbData->HcTable[fpDevInfo->bHCNumber - 1],
        fpDevInfo, ADSC_IN_REQUEST_TYPE + (BOT_GET_MAX_LUN_REQUEST_CODE << 8),
        fpDevInfo->bInterfaceNum, 0, Buffer, 1);

	MaxLun = *Buffer;
	USB_MemFree(Buffer, 1);

    return MaxLun;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassIssueCBITransaction
//
// DESCRIPTION: This function performs a mass storage transaction using CBI
//              or CB protocol.
//
// PARAMETERS:  fpDevInfo   Pointer to DeviceInfo structure
//              fpCmdBuffer Pointer to command buffer
//                  bCmdSize    Size of command block
//                  bXferDir    Transfer direction
//                  fpBuffer    Data buffer far pointer
//                  dwLength    Amount of data to be transferred
//                  wPreSkip    Number of bytes to skip before data
//                  wPostSkip   Number of bytes to skip after data
//
// RETURN:      Amount of data actually transferred
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
USBMassIssueCBITransaction(DEV_INFO* fpDevInfo)
{
    UINT32  dData = 0;

    if (!(USBMassSendCBICommand(fpDevInfo))) {  // Returns 0 on error
        return  0;
    }

    if (gUsbData->stMassXactStruc.dLength) {
        dData   = USBMassProcessBulkData(fpDevInfo);
        if (!dData) {
            if(gUsbData->bLastCommandStatus & USB_BULK_STALLED) {
                USBMassClearBulkEndpointStall(fpDevInfo,
                            gUsbData->stMassXactStruc.bXferDir);
                return  dData;
            }
        }
    }

    if(fpDevInfo->bProtocol != PROTOCOL_CBI_NO_INT && fpDevInfo->bIntEndpoint != 0) {
        //
        // Bypass interrupt transaction if it is CB protocol
        //
        USBMassCBIGetStatus(fpDevInfo);
    }

    return dData;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// PROCEDURE:   USBMassSendCBICommand
//
// DESCRIPTION: This function performs a mass storage transaction using CBI
//              or CB protocol.
//
// PARAMETERS:  fpDevInfo   Pointer to DeviceInfo structure
//              fpCmdBuffer Pointer to the command buffer
//              bCmdSize    Size of command block
//
// RETURN:      0xFFFF  SUCCESS
//              0x00    ERROR
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
USBMassSendCBICommand(DEV_INFO* fpDevInfo)
{
    UINT16  wRetValue;

    wRetValue   = (*gUsbData->aHCDriverTable[GET_HCD_INDEX(gUsbData->HcTable
                [fpDevInfo->bHCNumber - 1]->bHCType)].pfnHCDControlTransfer)
                (gUsbData->HcTable[fpDevInfo->bHCNumber - 1],
                fpDevInfo, ADSC_OUT_REQUEST_TYPE,
                (UINT16)fpDevInfo->bInterfaceNum, 0,
                gUsbData->stMassXactStruc.fpCmdBuffer,
                (UINT16)gUsbData->stMassXactStruc.bCmdSize);

    return wRetValue;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassCBIGetStatus
//
// Description: This function gets the status of the mass transaction
//      through an interrupt transfer
//
// Input:   pDevInfo    Pointer to DeviceInfo structure
//
// Output:  Return value from the interrupt transfer
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT16
USBMassCBIGetStatus(DEV_INFO*   fpDevInfo)
{
    (*gUsbData->aHCDriverTable[GET_HCD_INDEX(gUsbData->HcTable
        [fpDevInfo->bHCNumber - 1]->bHCType)].pfnHCDInterruptTransfer)
        (gUsbData->HcTable[fpDevInfo->bHCNumber - 1],
         fpDevInfo, (UINT8*)&gUsbData->wInterruptStatus, 2);

    return ((UINT16)gUsbData->wInterruptStatus);

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMiscIssueBulkTransfer
//
// Description: This function executes a bulk transaction on the USB. The
//      transfer may be either DATA_IN or DATA_OUT packets containing
//      data sent from the host to the device or vice-versa. This
//      function wil not return until the request either completes
//      successfully or completes with error (due to time out, etc.)
//      Size of data can be upto 64K
//
// Input:   - DeviceInfo structure (if available else 0)
//          - Transfer direction
//              Bit 7   : Data direction
//                          0 Host sending data to device
//                          1 Device sending data to host
//              Bit 6-0 : Reserved
//          - Buffer containing data to be sent to the device or
//            buffer to be used to receive data. Value in
//          - Length request parameter, number of bytes of data
//            to be transferred in or out of the host controller
//
// Output:  Amount of data transferred
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32
USBMassIssueBulkTransfer(DEV_INFO* fpDevInfo, UINT8 bXferDir,
                    UINT8* fpCmdBuffer, UINT32 dSize)
{
    return (*gUsbData->aHCDriverTable[GET_HCD_INDEX(gUsbData->HcTable
                [fpDevInfo->bHCNumber - 1]->bHCType)].pfnHCDBulkTransfer)
                (gUsbData->HcTable[fpDevInfo->bHCNumber -1],
                fpDevInfo, bXferDir,
                fpCmdBuffer, dSize);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassGetDeviceGeometry
//
// Description: This function fills and returns the mass get device geometry
//      structure
//
// Input:   fpMassGetDevGeo     Pointer to mass get geometry struc
//
// Output:  Return value
//      fpMassGetDevGeo     Pointer to mass get geometry struc
//          dSenseData  Sense data of the last command
//          bNumHeads   Number of heads
//          wNumCylinders   Number of cylinders
//          bNumSectors Number of sectors
//          wBytesPerSector Number of bytes per sector
//          bMediaType  Media type
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassGetDeviceGeometry (MASS_GET_DEV_GEO *fpGetDevGeometry)
{
    DEV_INFO    *fpDevInfo;
    UINT8       bDevAddr    = fpGetDevGeometry->bDevAddr;
    BOOLEAN     ValidGeo;
    MASS_GET_DEV_STATUS MassGetDevSts;

    fpDevInfo = USB_GetDeviceInfoStruc(USB_SRCH_DEV_INDX, 0, bDevAddr, 0);

    if ( (!fpDevInfo) || (!(fpDevInfo->bFlag & DEV_INFO_DEV_PRESENT)) ) {   // Error
        return USB_ERROR;
    }

    MassGetDevSts.bDevAddr = bDevAddr;
                        //(EIP13457+)>
    if (fpGetDevGeometry->bInt13FuncNum==0x20){
        fpDevInfo->bLastStatus |= USB_MASS_GET_MEDIA_FORMAT;
    }
    if ((!fpDevInfo->wBlockSize) || (fpDevInfo->wBlockSize == 0xFFFF) ||
            (!(fpDevInfo->bLastStatus & USB_MASS_MEDIA_PRESENT) ||
            (fpGetDevGeometry->bInt13FuncNum==0x20)) ) {
//      USBMassCheckDeviceReady(fpDevInfo);
        USBMassGetDeviceStatus(&MassGetDevSts);
    }                   //<(EIP13457+)
    ValidGeo = (BOOLEAN)((fpDevInfo->wBlockSize != 0xFFFF) && (fpDevInfo->wBlockSize != 0));
    ValidGeo &= (fpDevInfo->bLastStatus & USB_MASS_MEDIA_PRESENT);
                                        //(EIP107198+)>
    fpGetDevGeometry->wBytesPerSector   = ValidGeo? fpDevInfo->wBlockSize : 0;
    fpGetDevGeometry->bLBANumHeads      = ValidGeo? fpDevInfo->bHeads : 0;
    fpGetDevGeometry->bLBANumSectors    = ValidGeo? fpDevInfo->bSectors : 0;
    fpGetDevGeometry->wLBANumCyls       = ValidGeo? fpDevInfo->wCylinders : 0;
    fpGetDevGeometry->bNumHeads         = ValidGeo? fpDevInfo->bNonLBAHeads : 0;
    fpGetDevGeometry->bNumSectors       = ValidGeo? fpDevInfo->bNonLBASectors : 0;
    fpGetDevGeometry->wNumCylinders     = ValidGeo? fpDevInfo->wNonLBACylinders : 0;
    fpGetDevGeometry->bMediaType        = fpDevInfo->bMediaType;
    fpGetDevGeometry->dLastLBA          = ValidGeo? fpDevInfo->dMaxLba : 0;
    fpGetDevGeometry->BpbMediaDesc      = ValidGeo? fpDevInfo->BpbMediaDesc : 0;

    if(!(fpDevInfo->bLastStatus & USB_MASS_MEDIA_PRESENT))
        return USB_ATA_NO_MEDIA_ERR;
                                        //<(EIP107198+)

USB_DEBUG(DEBUG_LEVEL_4, "BPS %d H %d S %d C %d MT %d\n",
        fpDevInfo->wBlockSize,
        fpDevInfo->bHeads,
        fpDevInfo->bSectors,
        fpDevInfo->wCylinders,
        fpDevInfo->bMediaType);

    return  USB_SUCCESS;

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   USBMassReadCapacity
//
// Description: This function issues read capacity command to the mass
//      device and returns the value obtained
//
// Input:   fpReadCapacity  Pointer to the read capacity structure
//          bDevAddr    USB device address of the device
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT8
USBMassReadCapacity (MASS_READ_CAPACITY *fpReadCapacity)
{
    DEV_INFO    *fpDevInfo;
    UINT8       bDevAddr = fpReadCapacity->bDevAddr;

    fpDevInfo   = USB_GetDeviceInfoStruc(USB_SRCH_DEV_INDX, 0, bDevAddr, 0);

    if ( (!fpDevInfo) || (!(fpDevInfo->bFlag & DEV_INFO_DEV_PRESENT)) ) {   // Error
        return USB_ERROR;
    }

    return USBMassReadCapacityCommand(fpDevInfo);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   dabc_to_abcd
//
// Description: This function swaps the bytes in dword: 0-3,1-2,2-1,3-0. Can be
//              used for example in little endian->big endian conversions.
//
// Input:   DWORD to swap
//
// Output:  Input value with the swapped bytes in it.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINT32 dabc_to_abcd(UINT32 dData)
{
    return (((dData & 0x000000FF) << 24)
            | ((dData & 0x0000FF00) << 8)
            | ((dData & 0x00FF0000) >> 8)
            | ((dData & 0xFF000000) >> 24));
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
