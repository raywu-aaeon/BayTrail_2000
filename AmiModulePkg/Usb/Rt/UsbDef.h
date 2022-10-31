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

//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/rt/usbdef.h 105   9/04/12 8:04a Wilsonlee $
//
// $Revision: 105 $
//
// $Date: 9/04/12 8:04a $
//
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:           UsbDef.h
//
//  Description:    AMI USB driver definitions
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

// AVOID including multiple instance of this file
#ifndef     __USB_H
#define     __USB_H

#include    <Token.h>
#include    <AmiDxeLib.h>

#include    <Protocol/UsbHc.h>

#include    "Uhci.h"
#include    "Ohci.h"
#include    "Ehci.h"
#include    "Xhci.h"

#include	<Protocol/PciIo.h>
#include    <Protocol/DevicePath.h>
#include    <Protocol/UsbPolicy.h>

//
// USB Module version number
//
#define     USB_MAJOR_VER               USB_DRIVER_MAJOR_VER
#define     USB_MINOR_VER               USB_DRIVER_MINOR_VER
#define     USB_BUG_RELEASE_VER         USB_DRIVER_BUILD_VER
#define     USB_ACTIVE                  0xFC
#define     USB_LEGACY_ENABLE           0x01
#define     USB_6064_ENABLE             0x02

#define MAX_DEVICE_TYPES                0x07        // 7 different types of devices
#define MAX_HC_TYPES                    0x04        // 4 different types of host controllers
#define MAX_MASS_DEVICES                0x06
#define MAX_CCID_DEVICES                0x06

#define BIOS_DEV_TYPE_HID				0x01			
//#define BIOS_DEV_TYPE_KEYBOARD          0x01
//#define BIOS_DEV_TYPE_MOUSE             0x02
#define BIOS_DEV_TYPE_HUB               0x03
#define BIOS_DEV_TYPE_STORAGE           0x04
#define BIOS_DEV_TYPE_SECURITY          0x05
#define BIOS_DEV_TYPE_USBBUS            0x06        // Generic USB device driver
#define BIOS_DEV_TYPE_USBBUS_SHADOW     0x07        // Dummy device type for temp usage
#define BIOS_DEV_TYPE_CCID              0x08        // CCID device type

#define HID_DEV_TYPE_KEYBOARD          	BIT0        
#define HID_DEV_TYPE_MOUSE             	BIT1
#define HID_DEV_TYPE_POINT             	BIT2

#define MAX_DEVICES     (USB_DEV_HID_COUNT+USB_DEV_MASS_COUNT+USB_DEV_HUB_COUNT+USB_DEV_CCID_COUNT+USB_DEV_UNSUPPORTED)


// USB HC type
#define     USB_HC_UHCI                 0x10
#define     USB_HC_OHCI                 0x20
#define     USB_HC_EHCI                 0x30
#define     USB_HC_XHCI                 0x40
#define     GET_HCD_INDEX(bHCType)      (((bHCType) - USB_HC_UHCI) >> 4)
#define     USB_INDEX_UHCI              (GET_HCD_INDEX(USB_HC_UHCI))
#define     USB_INDEX_OHCI              (GET_HCD_INDEX(USB_HC_OHCI))
#define     USB_INDEX_EHCI              (GET_HCD_INDEX(USB_HC_EHCI))
#define     USB_INDEX_XHCI              (GET_HCD_INDEX(USB_HC_XHCI))

#define     USB_MEM_BLK_SIZE            32  // 32 bytes
#define     USB_MEM_BLK_SIZE_SHIFT      5   // log2 (USB_MEM_BLK_SIZE)

#define     USB_FORCE_64BIT_ALIGNMENT   1

#if USB_FORCE_64BIT_ALIGNMENT
#define USB_MEM_ALLOCATION_UNIT_SIZE 64
#else
#define USB_MEM_ALLOCATION_UNIT_SIZE 32
#endif

// The following macro returns number of memory blocks needed for the structure provided
#define     GET_MEM_BLK_COUNT_STRUC(Struc)      ((sizeof(Struc)+(sizeof(MEM_BLK)-1))/sizeof(MEM_BLK))

// The following macro returns number of memory blocks needed for the size of data provided
#define     GET_MEM_BLK_COUNT(Size)             (((Size) + (sizeof(MEM_BLK)-1))/sizeof(MEM_BLK))

//#define       TEMP_BUFFER_SIZE            0x80    // Size of temp buffer
//#define       CONTROL_DATA_SIZE           0x100
#define     MAX_CONTROL_DATA_SIZE       0x200
#define     MAX_TEMP_BUFFER_SIZE        0x80    // Size of temp buffer
#define     MAX_CONSUME_BUFFER_SIZE         0x1000  //(EIP59738+)
// USB state flag equates
#define     USB_FLAG_ENABLE_BEEP_MESSAGE    0x0001  // BIT 0
#define     USB_FLAG_RUNNING_UNDER_EFI      0x0002  // BIT 1
#define     USB_FLAG_DISABLE_LEGACY_SUPPORT 0x0004  // BIT 2
#define     USB_FLAG_6064EMULATION_ON       0x0008  // BIT 3
//#define     USB_FLAG_RUNNING_UNDER_OS       0x0010  // BIT 4
#define     USB_FLAG_DRIVER_CONSISTENT      0x0020  // BIT 5 //AMI Tracker 27603
#define     USB_FLAG_DRIVER_STARTED         0x0080  // BIT 7
#define     USB_FLAG_6064EMULATION_IRQ_SUPPORT  0x0100  // BIT 8
#define     USB_HOTPLUG_FDD_ENABLED         0x0200  // BIT 9
#define     USB_HOTPLUG_HDD_ENABLED         0x0400  // BIT 10
#define     USB_HOTPLUG_CDROM_ENABLED       0x0800  // BIT 11
#define     USB_FLAG_MASS_NATIVE_EMULATION  0x1000  // BIT 12
#define     USB_FLAG_MASS_MEDIA_CHECK       0x2000  // BIT 13
#define     USB_FLAG_MASS_SKIP_FDD_MEDIA_CHECK  0x4000  // BIT 14
#define     USB_FLAG_EFIMS_DIRECT_ACCESS    0x8000  // BIT15
#define     USB_FLAG_SKIP_CARD_READER_CONNECT_BEEP  0x10000 //BIT16 //(EIP64781+)
#define     USB_FLAG_MASS_SIZE_EMULATION    0x20000 //BIT17 //(EIP80382+)
#define     USB_FLAG_MASS_EMULATION_FOR_NO_MEDIA    0x40000 //BIT18	//(EIP86793+)

// PCI related equates
    // Invalid PCI register address bits
#define     PCI_REG_MAX_ADDRESS         0xFF00
#define     PCI_REG_ADDRESS_BYTE        PCI_REG_MAX_ADDRESS + 0x000
#define     PCI_REG_ADDRESS_WORD        PCI_REG_MAX_ADDRESS + 0x001
#define     PCI_REG_ADDRESS_DWORD       PCI_REG_MAX_ADDRESS + 0x003

// For systems with config mechanism 1
#define     CFG_SPACE_INDEX_REG         0xCF8
#define     CFG_SPACE_DATA_REG          0xCFC

// Standard PCI configuration register offsets and relevant values
//------------------------------------------------------------------------------
#define        PCI_REG_VENDID       0x00    //PCI vendor ID register
#define        PCI_REG_DEVID        0x02    //PCI device ID register
#define        PCI_REG_COMMAND      0x04    //PCI command register

//----------------------------------------------------------------------------
//          USB API equates
//----------------------------------------------------------------------------
#define     USB_NEW_API_START_FUNC          0x20

#define     USB_API_CHECK_PRESENCE          0x00
#define     USB_API_START                   0x20
#define     USB_API_STOP                    0x21
#define     USB_API_DISABLE_INTERRUPTS      0x22
#define     USB_API_ENABLE_INTERRUPTS       0x23
#define     USB_API_MOVE_DATA_AREA          0x24
#define     USB_API_GET_DEVICE_INFO         0x25
#define     USB_API_CHECK_DEVICE_PRESENCE   0x26
#define     USB_API_MASS_DEVICE_REQUEST     0x27
#define     USB_API_POWER_MANAGEMENT        0x28
#define     USB_API_PREPARE_FOR_OS          0x29
#define     USB_API_SECURITY_INTERFACE      0x2A
#define     USB_API_LIGHTEN_KEYBOARD_LEDS   0x2B
#define     USB_API_CHANGE_OWNER            0x2C
#define     USB_API_HC_PROC                 0x2D
#define     USB_API_CORE_PROC               0x2E
#define     USB_API_KBC_ACCESS_CONTROL      0x30    //(EIP29733+)
#define     USB_API_LEGACY_CONTROL          0x31    //
#define     USB_API_GET_DEV_ADDR            0x32
#define     USB_API_EXT_DRIVER_REQUEST      0x33
#define     USB_API_CCID_DEVICE_REQUEST     0x34
#define     USB_API_USB_STOP_CONTROLLER     0x35	//(EIP74876+)
#define		USB_API_HC_START_STOP			0x36

#define     USB_MASSAPI_GET_DEVICE_INFO     0x000
#define     USB_MASSAPI_GET_DEVICE_GEOMETRY 0x001
#define     USB_MASSAPI_RESET_DEVICE        0x002
#define     USB_MASSAPI_READ_DEVICE         0x003
#define     USB_MASSAPI_WRITE_DEVICE        0x004
#define     USB_MASSAPI_VERIFY_DEVICE       0x005
#define     USB_MASSAPI_FORMAT_DEVICE       0x006
#define     USB_MASSAPI_CMD_PASS_THRU       0x007
#define     USB_MASSAPI_ASSIGN_DRIVE_NUMBER 0x008
#define     USB_MASSAPI_CHECK_DEVICE        0x009
#define     USB_MASSAPI_GET_MEDIA_STATUS    0x00A
#define     USB_MASSAPI_GET_DEV_PARMS       0x00B

#define     USB_MASS_MEDIA_PRESENT      BIT0
#define     USB_MASS_MEDIA_CHANGED      BIT1
#define	    USB_MASS_GET_MEDIA_FORMAT	BIT2		//(EIP13457+)

#define     USB_SECURITY_API_READ_DEVICE    0x000
#define     USB_SECURITY_API_WRITE_DEVICE   0x001

#define     USB_PM_SUSPEND                  0x010
#define     USB_PM_RESUME                   0x020

// Error returned from API handler
#define     USB_SUCCESS             0x000
#define     USB_PARAMETER_ERROR     0x010
#define     USB_NOT_SUPPORTED       0x020
#define     USBAPI_INVALID_FUNCTION 0x0F0
#define     USB_ERROR               0x0FF

//
// Bit definitions for a generic pointer
//
#define        TERMINATE        0x00000001
#define        QUEUE_HEAD       0x00000002
#define        VERTICAL_FLAG    0x00000004
#define        LINK_POINTER     0xFFFFFFF0
// Mass storage data sync equates
#define USB_BULK_IN_DATA_SYNC           BIT0
#define USB_BULK_IN_DATA_SYNC_SHIFT     0
#define USB_BULK_OUT_DATA_SYNC          BIT1
#define USB_BULK_OUT_DATA_SYNC_SHIFT    1
#define USB_INT_DATA_SYNC               BIT2
#define USB_INT_DATA_SYNC_SHIFT         2

// Highest possible device address
#define MAX_DEVICE_ADDR                 MAX_DEVICES
// Addr that is guaranted not to be used
#define DUMMY_DEVICE_ADDR               (MAX_DEVICE_ADDR + 1)

#define DEFAULT_PACKET_LENGTH           8       // Max size of packet data

// USB BIOS related error codes
#define USB_ERROR_CODE_START            0x8100
#define ERRUSB_HC_NOT_FOUND             (USB_ERROR_CODE_START + 1)
#define ERRUSB_DEVICE_INIT              (USB_ERROR_CODE_START + 2)
#define ERRUSB_DEVICE_DISABLED          (USB_ERROR_CODE_START + 3)
#define ERRUSB_OHCI_EMUL_NOT_SUPPORTED  (USB_ERROR_CODE_START + 4)
#define ERRUSB_EHCI_64BIT_DATA_STRUC    (USB_ERROR_CODE_START + 5)

// USB internal error codes
#define USB_ERR_DEV_INIT_MEM_ALLOC              0x01
#define USB_ERR_DEV_INIT_GET_DESC_8             0x02
#define USB_ERR_DEV_INIT_SET_ADDR               0x03
#define USB_ERR_DEV_INIT_GET_DESC_100           0x04
#define USB_ERR_DEV_INIT_GET_DESC_200           0x05
#define USB_ERR_NO_DRIVER                       0x20
#define USB_ERR_NO_HCSTRUC                      0x21
#define USB_ERR_STARTHC_NO_MEMORY               0x22
#define USB_ERR_KBCONNECT_FAILED                0x23
#define USB_ERR_HC_RESET_FAILED                 0x24
#define USB_ERR_PORT_RESET_FAILED               0x25
#define USB_ERR_CONTROL_XFER_TIMEOUT            0x80
//----------------------------------------------------------------------------
//          Descriptor Type Values
//---------------------------------------------------------------------------
#define DESC_TYPE_DEVICE        1   // Device Descriptor (Type 1)
#define DESC_TYPE_CONFIG        2   // Configuration Descriptor (Type 2)
#define DESC_TYPE_STRING        3   // String Descriptor (Type 3)
#define DESC_TYPE_INTERFACE     4   // Interface Descriptor (Type 4)
#define DESC_TYPE_ENDPOINT      5   // Endpoint Descriptor (Type 5)
										//(EIP38434+)>
#define DESC_TYPE_REPORT        0x22 // Report Descriptor (Type 22h)
#define DESC_TYPE_HID           0x21 // HID Descriptor (Type 21h)
										//<(EIP38434+)
#define DESC_TYPE_HUB           0x29 // Hub Descriptor (Type 29h)
#define DESC_TYPE_SS_HUB		0x2A

#define DESC_TYPE_CLASS_HUB     0x2900   // Hub Class Descriptor (Type 0)

//----------------------------------------------------------------------------
//  USB protocol related routines
//----------------------------------------------------------------------------

#define MAX_USB_ERROR_RETRY     01

// USB Version structure
typedef struct {
    UINT8   bMajor;
    UINT8   bMinor;
    UINT8   bBugRel;
} USB_VERSION;

typedef struct {
    UINT8   aBuf[32];
} MEM_BLK;

#define		MEM_BLK_COUNT			(MEM_PAGE_COUNT * (4096 / USB_MEM_BLK_SIZE))
#define     MEM_BLK_STS_COUNT       (MEM_BLK_COUNT / 32)


typedef struct {
    UINT16  wPCIDev;
    UINT16  wHCType;
} HC_PCI_INFO;


typedef union {
    UHCI_DESC_PTRS  *fpUHCIDescPtrs;
    OHCI_DESC_PTRS  *fpOHCIDescPtrs;
    EHCI_DESC_PTRS  *fpEHCIDescPtrs;
} DESC_PTRS;

#pragma pack(push, 1)

typedef struct {
    UINT8       bDescLength;
    UINT8       bDescType;
    UINT16      wUsbSpecVersion;
    UINT8       bBaseClass;
    UINT8       bSubClass;
    UINT8       bProtocol;
    UINT8       bEndp0MaxPacket;
    UINT16      wVendorId;
    UINT16      wDeviceId;
    UINT16      wDeviceRev;
    UINT8       bMfgStr;
    UINT8       bProductStr;
    UINT8       bSerialStr;
    UINT8       bNumConfigs;
} DEV_DESC;

typedef struct {
    UINT8       bDescLength;
    UINT8       bDescType;
    UINT16      wTotalLength;
    UINT8       bNumInterfaces;
    UINT8       bConfigValue;
    UINT8       bConfigString;
    UINT8       bConfigFlags;
    UINT8       bConfigPower;
} CNFG_DESC;

typedef struct {
    UINT8          bDescLength;
    UINT8          bDescType;
    UINT8          bInterfaceNum;
    UINT8          bAltSettingNum;
    UINT8          bNumEndpoints;
    UINT8          bBaseClass;
    UINT8          bSubClass;
    UINT8          bProtocol;
    UINT8          bInterfaceString;
}INTRF_DESC;

typedef struct {
    UINT8           bDescLength;
    UINT8           bDescType;
    UINT8           bEndpointAddr;
    UINT8           bEndpointFlags;
    UINT16          wMaxPacketSize;
    UINT8           bPollInterval;
} ENDP_DESC;

										//(EIP38434+)>
typedef struct {
    UINT8          bDescLength;
    UINT8          bDescType;
    UINT16         bcdHID;
    UINT8          bCountryCode;
    UINT8          bNumEndpoints;
    UINT8          bDescriptorType;
    UINT16         bDescriptorLength;
}HID_DESC;

#pragma pack(pop)

//----------------------------------------------------------------------------
//	Report descriptor struct define
//----------------------------------------------------------------------------
#define HID_BFLAG_DATA_BIT							BIT0		//0:DATA 		1:CONSTANT
#define HID_BFLAG_ARRAY_BIT							BIT1		//0:ARRAY		1:VARIABLE
#define HID_BFLAG_RELATIVE_BIT						BIT2		//0:ABSOLUTE	1:RELATIVE
#define HID_BFLAG_SKIP								BIT3		//1:Skip this data
#define HID_BFLAG_INPUT 							BIT4 		//0:OUTPUT 	1:INPUT
#define HID_MAX_USAGE 								0x14						//(EIP96010)

typedef struct {
	UINT8			bFlag;			
	UINT8			bReportID;
	UINT8			bUsagePage;
	UINT8			bReportCount;
	UINT8			bReportSize;
	UINT16			wLogicalMin;
	UINT16			wLogicalMax;
    UINT16          PhysicalMax;        //(EIP127014)
    UINT16          PhysicalMin;        //(EIP127014)
    UINT8           UnitExponent;       //(EIP127014)
	UINT8 			bUsageCount;
	UINT8			bUsage[HID_MAX_USAGE];         //(EIP96010)
	UINT8			bUsageMaxCount;		//(EIP84455)
	UINT16			wUsageMax[5];		//(EIP84455)
	UINT16			wUsageMin[5];		//(EIP84455)
	UINT8 			bCollection_count;
}HID_STRUC,*HID_STRUC_PTR;		

#define HID_BTYPE_KEYBOARD							0x1
#define HID_BTYPE_MOUSE								0x2
#define HID_BTYPE_POINT		 						0X3 

#define HID_REPORT_BFLAG_REPORT_PROTOCOL			BIT0		//If use report protocol
#define HID_REPORT_BFLAG_REPORT_ID					BIT1 		//1:REPORT_ID EXIST
#define HID_REPORT_BFLAG_TOUCH_BUTTON_FLAG			BIT2 		
#define HID_REPORT_BFLAG_LED_FLAG			        BIT3        //1:LED  		 //EIP65344
#define HID_REPORT_BFLAG_RELATIVE_DATA			    BIT4
#define HID_REPORT_BFLAG_ABSOLUTE_DATA			    BIT5

typedef struct {
	UINT8			bTotalCount;
	UINT8 			bFlag;
	UINT16			wAbsMaxX; 			
	UINT16			wAbsMaxY;
    UINT16          wReportLength;      //(EIP80948)
	HID_STRUC		*pReport;           //(EIP80948)
}HIDReport_STRUC;

//----------------------------------------------------------------------------
//  Report descriptor's hid_item
//----------------------------------------------------------------------------
typedef struct {
	UINT8	bSize;
	UINT8   bType;
	UINT8   bTag;
	union {
	    UINT8   u8;
	    UINT16  u16;
	    UINT32  u32;
	} data;
}HID_Item,*HID_ITEM_PTR;


//----------------------------------------------------------------------------
// HID Report define start
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// HID report item format
//----------------------------------------------------------------------------
#define HID_ITEM_FORMAT_SHORT					0
#define HID_ITEM_FORMAT_LONG					1

//----------------------------------------------------------------------------
// HID report descriptor item type (prefix bit 2,3)
//----------------------------------------------------------------------------
#define HID_ITEM_TYPE_MAIN						0
#define HID_ITEM_TYPE_GLOBAL					1
#define HID_ITEM_TYPE_LOCAL						2
#define HID_ITEM_TYPE_RESERVED					3

//----------------------------------------------------------------------------
// HID report descriptor main item tags
//----------------------------------------------------------------------------
#define HID_MAIN_ITEM_TAG_INPUT					8
#define HID_MAIN_ITEM_TAG_OUTPUT				9
//#define HID_MAIN_ITEM_TAG_FEATURE				0xb
//#define HID_MAIN_ITEM_TAG_BEGIN_COLLECTION		0xa
//#define HID_MAIN_ITEM_TAG_END_COLLECTION		0xc

//----------------------------------------------------------------------------
// HID report descriptor main item contents
//----------------------------------------------------------------------------
#define HID_MAIN_ITEM_CONSTANT					0x001
#define HID_MAIN_ITEM_VARIABLE					0x002
#define HID_MAIN_ITEM_RELATIVE					0x004
#define HID_MAIN_ITEM_WRAP						0x008
#define HID_MAIN_ITEM_NONLINEAR					0x010
#define HID_MAIN_ITEM_NO_PREFERRED				0x020
#define HID_MAIN_ITEM_NULL_STATE				0x040
#define HID_MAIN_ITEM_VOLATILE					0x080
#define HID_MAIN_ITEM_BUFFERED_BYTE				0x100

//----------------------------------------------------------------------------
// HID report descriptor collection item types
//----------------------------------------------------------------------------
#define HID_COLLECTION_PHYSICAL					0
#define HID_COLLECTION_APPLICATION				1
#define HID_COLLECTION_LOGICAL					2

//----------------------------------------------------------------------------
// HID report descriptor global item tags
//----------------------------------------------------------------------------
#define HID_GLOBAL_ITEM_TAG_USAGE_PAGE			0
#define HID_GLOBAL_ITEM_TAG_LOGICAL_MINIMUM		1
#define HID_GLOBAL_ITEM_TAG_LOGICAL_MAXIMUM		2
#define HID_GLOBAL_ITEM_TAG_PHYSICAL_MINIMUM	3
#define HID_GLOBAL_ITEM_TAG_PHYSICAL_MAXIMUM	4
#define HID_GLOBAL_ITEM_TAG_UNIT_EXPONENT		5
#define HID_GLOBAL_ITEM_TAG_UNIT				6
#define HID_GLOBAL_ITEM_TAG_REPORT_SIZE			7
#define HID_GLOBAL_ITEM_TAG_REPORT_ID			8
#define HID_GLOBAL_ITEM_TAG_REPORT_COUNT		9
//#define HID_GLOBAL_ITEM_TAG_PUSH				0x0a
//#define HID_GLOBAL_ITEM_TAG_POP					0x0b

//----------------------------------------------------------------------------
// HID report descriptor local item tags
//----------------------------------------------------------------------------
#define HID_LOCAL_ITEM_TAG_USAGE				0
#define HID_LOCAL_ITEM_TAG_USAGE_MINIMUM		1
#define HID_LOCAL_ITEM_TAG_USAGE_MAXIMUM		2
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_INDEX		3
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_MINIMUM	4
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_MAXIMUM	5
#define HID_LOCAL_ITEM_TAG_STRING_INDEX			7
#define HID_LOCAL_ITEM_TAG_STRING_MINIMUM		8
#define HID_LOCAL_ITEM_TAG_STRING_MAXIMUM		9
//#define HID_LOCAL_ITEM_TAG_DELIMITER			0x0a

//----------------------------------------------------------------------------
// HID Report define end
//----------------------------------------------------------------------------
										//<(EIP38434+)
//----------------------------------------------------------------------------
//      Bit definitions for EndpointDescriptor.EndpointAddr
//----------------------------------------------------------------------------
#define EP_DESC_ADDR_EP_NUM     0x0F    //Bit 3-0: Endpoint number
#define EP_DESC_ADDR_DIR_BIT    0x80    //Bit 7: Direction of endpoint, 1/0 = In/Out
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//  Bit definitions for EndpointDescriptor.EndpointFlags
//----------------------------------------------------------------------------
#define EP_DESC_FLAG_TYPE_BITS  0x03    //Bit 1-0: Indicate type of transfer on endpoint
#define EP_DESC_FLAG_TYPE_CONT  0x00    //Bit 1-0: 00 = Endpoint does control transfers
#define EP_DESC_FLAG_TYPE_ISOC  0x01    //Bit 1-0: 01 = Endpoint does isochronous transfers
#define EP_DESC_FLAG_TYPE_BULK  0x02    //Bit 1-0: 10 = Endpoint does bulk transfers
#define EP_DESC_FLAG_TYPE_INT   0x03    //Bit 1-0: 11 = Endpoint does interrupt transfers
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
//      Values for InterfaceDescriptor.BaseClass
//---------------------------------------------------------------------------
#define BASE_CLASS_HID              0x03
#define BASE_CLASS_MASS_STORAGE     0x08
#define BASE_CLASS_HUB              0x09
//----------------------------------------------------------------------------


//---------------------------------------------------------------------------
//      Values for InterfaceDescriptor.SubClass
//---------------------------------------------------------------------------
#define SUB_CLASS_BOOT_DEVICE       0x01    // Boot device sub-class
#define SUB_CLASS_HUB               0x01    //Hub Device Sub Class?

// Mass storage related sub-class equates
#define SUB_CLASS_RBC                   0x01    // RBC T10 project,1240-D, e.g. Flash
#define SUB_CLASS_SFF8020I              0x02    // SFF8020I, e.g. ATAPI CD-ROM
#define SUB_CLASS_QIC157                0x03    // QIC-157, e.g. ATAPI Tape device
#define SUB_CLASS_UFI                   0x04    // UFI, e.g. Floppy
#define SUB_CLASS_SFF8070I              0x05    // SFF8070I, e.g. ATAPI Floppy
#define SUB_CLASS_SCSI                  0x06    // SCSI transparent command set

// Vendor specific mass storage related sub-class equates
#define SUB_CLASS_PL2307                0x80    // Prolific 2307 ,USB to IDE
#define SUB_CLASS_SL11R                 0x81    // ScanLogic SL11R-IDE
#define SUB_CLASS_THUMB_DRV             0x82    // ThumbDrive
#define SUB_CLASS_DFUSB01               0x83    // DataFab ATA Bridge
#define SUB_CLASS_DOK                   0x84    // Disk On Key
#define SUB_CLASS_VENDOR_SPECIFIC       0xFF    // Vendor Specific
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
//      Values for InterfaceDescriptor.Protocol
//---------------------------------------------------------------------------
#define PROTOCOL_KEYBOARD       0x01    // Keyboard device protocol
#define PROTOCOL_MOUSE          0x02    // Mouse device protocol?

// Mass storage related protocol equates
#define PROTOCOL_CBI            0x00    // Mass Storage Control/Bulk/Interrupt
                                        // with command completion interrupt
#define PROTOCOL_CBI_NO_INT     0x01    // MASS STORAGE Control/Bulk/Interrupt
                                        // with NO command completion interrupt
#define PROTOCOL_BOT            0x50    // Mass Storage Bulk-Only Transport
#define PROTOCOL_VENDOR         0xFF    // Vendor specific mass protocol
//---------------------------------------------------------------------------

// Definition of CCID class
#define BASE_CLASS_CCID_STORAGE     0x0B        // SMART device class
#define SUB_CLASS_CCID              0x00        // SubClass
#define PROTOCOL_CCID               0x00        // Interface Protocol
#define DESC_TYPE_SMART_CARD        0x21         // Smart Card Descriptor (Type 21h)

#pragma pack(push, 1)

typedef struct {
    UINT16      wRequestType;
    UINT16      wValue;
    UINT16      wIndex;
    UINT16      wDataLength;
} DEV_REQ;

#pragma pack(pop)

typedef struct _HC_STRUC HC_STRUC;
typedef struct _DEV_INFO DEV_INFO;
typedef struct _DEV_DRIVER DEV_DRIVER;
typedef struct _URP_STRUC URP_STRUC;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        HCD_HEADER
//
// Description: USB Host Controller Driver function list structure.
//
// Fields:   Name       Type    Description
//      ------------------------------------------------------------
//    bFlag UINT8 Driver Header Status
//    pfnHCDStart UINT8 Driver Start
//    pfnHCDStop UINT8 Driver Stop
//    pfnHCDEnumeratePorts UINT8 Enumerate Root Ports
//    pfnHCDDisableInterrupts UINT8 Disable Interrupts
//    pfnHCDEnableInterrupts UINT8 Enable Interrupts
//    pfnHCDProcessInterrupt UINT8 Process Interrupt
//    pfnHCDGetRootHubStatus UINT8 Get Root Hub Ports Status
//    pfnHCDDisableRootHub UINT8 Disable Root Hub 
//    pfnHCDEnableRootHub UINT8 Enable Root Hub
//    pfnHCDControlTransfer UINT16 Perform Control Transfer
//    pfnHCDBulkTransfer UINT32 Perform Bulk Transfer
//    pfnHCDInterruptTransfer UINT8 Perform Interrupt Transfer
//    pfnHCDDeactivatePolling UINT8  Deactivate Polling
//    pfnHCDActivatePolling UINT8 Activate Polling
//    pfnHCDDisableKeyRepeat UINT8 Disable Key Repead
//    pfnHCDEnableKeyRepeat UINT8 Enable Key Repeat
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
//!!!!
//!!!! If you change this structure, please, check UN_HCD_HEADER also.
//!!!!
typedef struct {
    UINT8       bFlag;
    UINT8       (*pfnHCDStart) (HC_STRUC*);
    UINT8       (*pfnHCDStop) (HC_STRUC*);
    UINT8       (*pfnHCDEnumeratePorts) (HC_STRUC*);
    UINT8       (*pfnHCDDisableInterrupts) (HC_STRUC*);
    UINT8       (*pfnHCDEnableInterrupts) (HC_STRUC*);
    UINT8       (*pfnHCDProcessInterrupt) (HC_STRUC*);
    UINT8       (*pfnHCDGetRootHubStatus) (HC_STRUC*,UINT8);
    UINT8       (*pfnHCDDisableRootHub) (HC_STRUC*,UINT8);
    UINT8       (*pfnHCDEnableRootHub) (HC_STRUC*,UINT8);
    UINT16      (*pfnHCDControlTransfer) (HC_STRUC*,DEV_INFO*,UINT16,UINT16,UINT16,UINT8*,UINT16);
    UINT32      (*pfnHCDBulkTransfer) (HC_STRUC*,DEV_INFO*,UINT8,UINT8*,UINT32);
    UINT16      (*pfnHCDInterruptTransfer) (HC_STRUC*,DEV_INFO*,UINT8*,UINT16);
    UINT8       (*pfnHCDDeactivatePolling) (HC_STRUC*,DEV_INFO*);
    UINT8       (*pfnHCDActivatePolling) (HC_STRUC*,DEV_INFO*);
    UINT8       (*pfnHCDDisableKeyRepeat) (HC_STRUC*);
    UINT8       (*pfnHCDEnableKeyRepeat) (HC_STRUC*);
    UINT8       (*pfnHCDEnableEndpoints) (HC_STRUC*,DEV_INFO*,UINT8*);
    UINT8       (*pfnHCDInitDeviceData) (HC_STRUC*,DEV_INFO*,UINT8,UINT8**);
    UINT8       (*pfnHCDDeinitDeviceData) (HC_STRUC*,DEV_INFO*);
	UINT8       (*pfnHCDResetRootHub) (HC_STRUC*,UINT8);
	UINT8		(*pfnHCDClearEndpointState) (HC_STRUC*,DEV_INFO*,UINT8);	//(EIP54283+)
	UINT8       (*pfnHCDGlobalSuspend) (HC_STRUC*);		//(EIP54018+)
} HCD_HEADER;

typedef union {
    HCD_HEADER hcd_header;
    struct {
        UINT8       bFlag;
        VOID*       proc[(sizeof(HCD_HEADER)-1)/sizeof(VOID*)];
    } asArray;
} UN_HCD_HEADER;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        HC_STRUC
//
// Description: USB Host Controller structure
//
// Fields:   Name       Type    Description
//      ------------------------------------------------------------
//      bHCNumber   UINT8   Host Controller number, 1-based
//      bHCType     UINT8   Host Controller Type, U/O/E HCI
//      fpFrameList UINT32* Host Controller Frame List Address
//      BaseAddress UINTN   Host Controller Base Address, memory (EHCI,OHCI) or IO (UHCI)
//      bNumPorts   UINT8   Number of root ports, 1-based
//      wBusDevFuncNum UINT16   PCI location, bus (Bits8..15), device (Bits3..7), function(bits0..2)
//      fpIRQInfo   IRQ_INFO IRQ information
//      stDescPtrs  DESC_PTRS   Commonly used descriptor pointers, see definition of DESC_PTRS
//      wAsyncListSize  UINT16  Async. list size
//      bOpRegOffset UINT8  Operation region offset
//      dMaxBulkDataSize    UINT32 Maximum Bulk Transfer data size
//      dHCFlag     UINT32  Host Controller flag
//      bExtCapPtr  UINT8   EHCI Extended Capabilities Pointer
//      bRegOfs     UINT8   EHCI Capabilities PCI register Offset
//      DebugPort   UINT8   Port number of EHCI debug port
//      usbbus_data VOID*   USB Bus data specific to this Host Controller
//      Controller  EFI_HANDLE  EFI Handle of this controller
//      pHCdp   EFI_DEVICE_PATH_PROTOCOL* Pointer to this controller's device path
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

struct _HC_STRUC {
    UINT8           bHCNumber;
    UINT8           bHCType;
    UINT32          *fpFrameList;
    UINTN           BaseAddress;
    UINT8           bNumPorts;
    UINT16          wBusDevFuncNum;
    UINT8           Irq;
    DESC_PTRS       stDescPtrs;
    UINT16          wAsyncListSize;
    UINT8           bOpRegOffset;
    UINT32          dMaxBulkDataSize;
	UINT32			dHCSParams;
	UINT32			dHCCParams;
    UINT32          dHCFlag;
    UINT8           bExtCapPtr; // EHCI Extended Capabilities Pointer
    UINT8           DebugPort;
    VOID*           usbbus_data;
    EFI_HANDLE      Controller;
    EFI_DEVICE_PATH_PROTOCOL    *pHCdp;
    UINT8           PwrCapPtr;	//(EIP54018+)
    EFI_PCI_IO_PROTOCOL	*PciIo;
    UINT16          Vid;
    UINT16          Did;
    EFI_HANDLE      HwSmiHandle;
#if !USB_RUNTIME_DRIVER_IN_SMM
	UINT32			MemPoolPages;
	UINT8			*MemPool;
	UINT32			MemBlkStsBytes;
	UINT32			*MemBlkSts;
#endif
};

// Equates related to host controller state
#define HC_STATE_RUNNING						BIT0
#define HC_STATE_SUSPEND						BIT1
#define HC_STATE_USED							BIT2
#define HC_STATE_INITIALIZED					BIT3
#define HC_STATE_EXTERNAL						BIT4
#define	HC_STATE_OWNERSHIP_CHANGE_IN_PROGRESS	BIT5
#define	HC_STATE_CONTROLLER_WITH_RMH        	BIT6

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        DEV_INFO
//
// Description: USB Device Information Structure
//
// Fields:   Name       Type    Description
//      ------------------------------------------------------------
//  bFlag       UINT8   Device Information flags
//  bDeviceType UINT8   Device Type
//  wVendorId   UINT16  Device VID
//  wDeviceId   UINT16  Device DID
//  bDeviceAddress  UINT8   Device USB Address
//  bHCNumber   UINT8   Host Controller Number this device is attached to
//  bHubDeviceNumber    UINT8   USB Hub Device Number this device is attached to
//  bHubPortNumber  UINT8   USB Hub Port Number this device is attached to
//  bEndpointNum    UINT8   Endpoint number
//  bEndpointSpeed  UINT8   Endpoint speed
//  bLUN        UINT8   Device Logical Unit number
//  wEndp0MaxPacket UINT16  Endpoint0 max packet size, in Bytes
//  bNumConfigs UINT8   Number of configurations
//  bConfigNum  UINT8   Active configuration number (0-based)
//  bInterfaceNum   UINT8   Active interface number
//  bAltSettingNum  UINT8   Alternate setting number (0-based)
//  bCallBackIndex  UINT8   Callback function index
//  fpPollTDPtr     UINT8*  Polling TD pointer
//  fpPollTEPtr     UINT8*  Polling ED pointer
//  bHubNumPorts    UINT8   Hub # of ports (USB hubs only)
//  bHubPowerOnDelay    UINT8 Hub power-on delay (USB hubs only)
//  fpLUN0DevInfoPtr    DEV_INFO* Pointer to Lun0 device (for multiple-LUN devices)
//  wDataIn/OutSync   UINT16   toggle tracking information
//  bStorageType    UINT8   USB_MASS_DEV_ARMD, USB_MASS_DEV_HDD, etc.
//  wIntMaxPkt  UINT16  Interrupt Max Packet size, in Bytes
//  bPresent    UINT8   Device presence indicator
//  bIntEndpoint    UINT8   Interrupt endpoint number
//  bBulkInEndpoint UINT8   Bulk-In endpoint number
//  bBulkOutEndpoint    UINT8   Bulk-Out endpoint number
//  bProtocol   UINT8   Protocol
//  wEmulationOption    UINT16  USB Mass Storage Drive Emulation Option, from Setup
//  bHiddenSectors  UINT8   Number of hidden sectors, for USB mass storage devices only
//  bSubClass   UINT8   Device sub-class
//  wBlockSize  UINT16  USB Mass Storage Device block size, in Bytes
//  dMaxLba     UINT32  USB Mass Storage Device Maximum LBA number
//  bHeads      UINT8   USB Mass Storage Device # of heads
//  bSectors    UINT8   USB Mass Storage Device # of sectors
//  wCylinders  UINT16  USB Mass Storage Device # of cylinders
//  bNonLBAHeads UINT8   USB Mass Storage Device # of heads reported in Non-LBA (CHS) functions
//  bNonLBASectors  UINT8   USB Mass Storage Device # of sectors reported in Non-LBA (CHS) functions
//  wNonLBACylinders    UINT16  USB Mass Storage Device # of cylinders reported in Non-LBA (CHS) functions
//  bEmuType    UINT8   USB Mass Storage Device emulation type
//  bPhyDevType UINT8   USB Mass Storage Device physical type
//  bMediaType  UINT8   USB Mass Storage Device media type
//  bDriveNumber    UINT8   USB Mass Storage Device INT13 drive number
//  wBulkInMaxPkt   UINT16  USB Mass Storage Device Bulk-In max packet size, in Bytes
//  wBulkOutMaxPkt  UINT16  USB Mass Storage Device Bulk-Out max packet size, in Bytes
//  wIncompatFlags  UINT16  USB Mass Storage Device Incompatibility flags
//  MassDev     VOID*   USB Mass Storage Device EFI handle
//  fpDeviceDriver  DEV_DRIVER*   Device driver pointer
//  bLastStatus UINT8   Last transaction status
//  pExtra      UINT8*  Pointer to extra device specific data
//  UINT32      UINT8   USB Mass Storage Device # of heads
//  Handle      UINT32[2]   USB Device Handle
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
struct _DEV_INFO {
    UINT8       bFlag;          //00
    UINT8       bDeviceType;    //01
    UINT16      wVendorId;      //02
    UINT16      wDeviceId;      //04
    UINT8       bDeviceAddress; //06

    UINT8       bHCNumber;      //07
    UINT8       bHubDeviceNumber;   //08
    UINT8       bHubPortNumber; //09
//    UINT8       bEndpointNum;   //0A //(EIP70933-)
    UINT8       bEndpointSpeed; //0B
    UINT8       bLUN;           //0C
    UINT16      wEndp0MaxPacket;//0D
    UINT8       bNumConfigs;    //0F
    UINT8       bConfigNum;     //10
    UINT8       bInterfaceNum;  //11
    UINT8       bAltSettingNum; //12

    UINT8       bCallBackIndex; //13
    UINT8       *fpPollTDPtr;   //14
    UINT8       *fpPollEDPtr;   //18

    UINT8       bHubNumPorts;   //1C
    UINT8       bHubPowerOnDelay;//1D

    struct _DEV_INFO *fpLUN0DevInfoPtr; //1E
//    UINT8       bDataSync;      //22

    UINT16      wDataInSync;    // 22
    UINT16      wDataOutSync;   // 24
    UINT8       bStorageType;   // 26, USB_MASS_DEV_ARMD, USB_MASS_DEV_HDD, etc.
    UINT16      wIntMaxPkt;     //27
    UINT8       bIntEndpoint;   //2A
    UINT8       bBulkInEndpoint;    //2B
    UINT8       bBulkOutEndpoint;   //2C

    UINT8       bBaseClass;     // BASE_CLASS_HID, BASE_CLASS_MASS_STORAGE or BASE_CLASS_HUB
    UINT8       bSubClass;
    UINT8       bProtocol;          //
    UINT16      wEmulationOption;   //
    UINT8       bHiddenSectors;     //

    UINT16      wBlockSize;         //
    UINT32      dMaxLba;            //
    UINT8       bHeads;             //
    UINT8       bSectors;           //
    UINT16      wCylinders;         //
    UINT8       bNonLBAHeads;       //
    UINT8       bNonLBASectors;     //
    UINT16      wNonLBACylinders;   //
    UINT8       bEmuType;           //
    UINT8       bPhyDevType;        //
    UINT8       bMediaType;         //
    UINT8       bDriveNumber;       //
    UINT16      wBulkInMaxPkt;      //
    UINT16      wBulkOutMaxPkt;     //
    UINT16      wIncompatFlags;     //
    VOID        *MassDev;           //
    DEV_DRIVER  *fpDeviceDriver;    //
    UINT8       bLastStatus;        //
    UINT8       *pExtra;            //
    UINT32      Handle[2];
    UINT8       DevNameString[64];
    VOID        *DevMiscInfo;
    UINT8        HubDepth;
    UINT8         *fpPollDataBuffer;        //Polling Data Buffer    //(EIP54782+)
    VOID		*pCCIDDescriptor; // Ptr to CCID descriptor
    UINT32      *DataRates;           // List of DataRates supported by CCID  
    UINT32      *ClockFrequencies;    // List of Frequencies suported by CCID
    DLIST        ICCDeviceList;        // Linked list of ICC devices. :Linked to "ICCDeviceLink"
	HIDReport_STRUC	Hidreport;			//(EIP38434+)
	UINT8 		HidDevType;
	UINT8       bPollInterval;          //(EIP84455+)
	UINT16      HubPortConnectMap;
    UINT8       BpbMediaDesc;
	VOID		*KeyCodeBuffer;
	VOID		*UsbKeyBuffer;
};

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        DEV_DRIVER
//
// Description: USB Device Driver Structure
//
// Fields:   Name       Type    Description
//      ------------------------------------------------------------
//  bDevType    UINT8   Device Type
//  bBaseClass  UINT8   Device Base Type
//  bSubClass   UINT8   Device Subclass
//  bProtocol   UINT8   Device Protocol
//  pfnDeviceInit   VOID    Device Initialization Function
//  pfnCheckDeviceType  UINT8   Check Device Type Function
//  pfnConfigureDevice  DEV_INFO*   Configure Device Function
//  pfnDisconnectDevice UINT8   Disconnect Device Function
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

struct _DEV_DRIVER {
    UINT8           bDevType;
    UINT8           bBaseClass;
    UINT8           bSubClass;
    UINT8           bProtocol;
    VOID            (*pfnDeviceInit)(VOID);
    UINT8           (*pfnCheckDeviceType)(DEV_INFO*, UINT8, UINT8, UINT8);
    DEV_INFO*       (*pfnConfigureDevice)(HC_STRUC*, DEV_INFO*, UINT8*, UINT16, UINT16);
    UINT8           (*pfnDisconnectDevice)(DEV_INFO*);
    VOID            (*pfnDriverRequest)(DEV_INFO*, URP_STRUC*);
};

#pragma pack(push, 1)

typedef struct {
    UINT8              bDescLength;
    UINT8              bDescType;
    UINT8              bNumPorts;               // Number of downstream ports on hub
    UINT16             wHubFlags;               // See HUB_FLAG_xxx bit definitions below
    UINT8              bPowerOnDelay;           // Time to delay after turning on power to port (in 2ms units)
    UINT8              bHubControlAmps;         // Milliamps of current needed by hub controller
	UINT8              bHubHdrDecLat;
	UINT16             wHubDelay;
    UINT16             DeviceRemovable;         // Variable size array of bits (one for each port)
} HUB_DESC;

#pragma pack(pop)

#define DEV_INFO_VALID_STRUC            BIT0    // Structure validity
#define DEV_INFO_DEV_PRESENT            BIT1    // Device presence status
#define DEV_INFO_MASS_DEV_REGD          BIT2    // Mass device registered(have
                                                // drive number assigned)
#define DEV_INFO_MULTI_IF               BIT3    // Indicates that the device
                                                // is a part of multiple
                                                // interface device
#define DEV_INFO_HOTPLUG                BIT4    // Indicates that this device
                                                // is a hotplugged device
#define DEV_INFO_DEV_DUMMY              BIT5
#define DEV_INFO_DEV_BUS                BIT6    // Device info is locked by the bus
#define DEV_INFO_DEV_DISCONNECTING      BIT7	//(EIP60460+)


#define DEV_INFO_VALIDPRESENT  (DEV_INFO_VALID_STRUC | DEV_INFO_DEV_PRESENT)
// Call back routine type definition
typedef UINT8       (*CALLBACK_FUNC) (HC_STRUC*, DEV_INFO*, UINT8*, UINT8*);

#define MAX_CALLBACK_FUNCTION           50

#define MAX_USB_ERRORS_NUM              0x30    // 48 errors max

#pragma pack(push, 1)

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        CK_PRESENCE
//
// Description: This is a URP (USB Request Packet) structure for the BIOS
//      API call CheckPresence (API #0)
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT16      wBiosRev;       // USB BIOS Revision
    UINT8       bBiosActive;    // USB BIOS active/inactive
    UINT8       bNumBootDev;    // # of USB boot device
    UINT16      wUsbDataArea;   // USB Data area
    UINT8       bNumKeyboards;  // Number of USB keyboards present
    UINT8       bNumMice;       // Number of USB mice present
    UINT8       bNumPoint;      // Number of USB point present				//<(EIP38434+)
    UINT8       bNumHubs;       // Number of USB hubs present
    UINT8       bNumStorage;    // Number of USB storage devices present
///////// DO NOT ADD ANY FIELD HERE. IF IT IS NECESSARY PLEASE UPDATE THE CODE
///////// IN THE FUNCTION USBWrap_GetDeviceCount in the file USBWRAP.ASM
    UINT8       bNumHarddisk;   // Number of hard disk emulated USB devices
    UINT8       bNumCDROM;      // Number of CDROM emulated USB devices
    UINT8       bNumFloppy;     // Number of floppy emulated USB devices
} CK_PRESENCE;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        START_HC
//
// Description: This is a URP (USB Request Packet) structure for the BIOS
//      API call StartHC and MoveDataArea (API #20 & #24)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      wDataAreaFlag   UINT16  Indicates which data area to use
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT16  wDataAreaFlag;      // Data area to use
} START_HC;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        GET_DEV_INFO
//
// Description: This is a URP (USB Request Packet) structure for the BIOS
//      API call GetDeviceInfo (API #25)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevNumber  UINT8   Device # whose info is requested
//      bHCNumber   UINT8   HC # to which this device is connected (0 if no such device found)
//      bDevType    UINT8   Device type (0 if no such device found)
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8   bDevNumber;
    UINT8   bHCNumber;
    UINT8   bDevType;
} GET_DEV_INFO;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        CHK_DEV_PRSNC
//
// Description: This is a URP (USB Request Packet) structure for the BIOS
//      API call CheckDevicePresence (API #26)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevType    UINT8           Type of device to look for
//      fpHCStruc   FPHC_STRUC      Pointer to HC being checked for device connection
//      bNumber     UINT8           Number of devices connected
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevType;
    HC_STRUC    *fpHCStruc;
    UINT8       bNumber;
} CHK_DEV_PRSNC;


typedef struct {
    UINT8   ScrLock:    1;
    UINT8   NumLock:    1;
    UINT8   CapsLock:   1;
    UINT8   Resrvd:     5;
} LED_MAP;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        KB_LEDS_DATA
//
// Description: This is a URP (USB Request Packet) structure for the BIOS
//              API call LightenKeyboardLeds(API #2B)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      LedMapPtr    	UINT32		32-bit Pointer to LED_MAP structure
//		DevInfoPtr    	UINT32   	32-bit Pointer to DEV_INFO structure
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
	UINT32	LedMapPtr;
	UINT32  DevInfoPtr;
} KB_LEDS_DATA;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:         SECURITY_IF
//
// Description:  This is a URP (USB Request Packet) structure for the BIOS
//               API call SecurityInterface (API #2Ah)
//
// Fields:        Name           Type            Description
//               ------------------------------------------------------------
//               fpBuffer        FAR     Buffer pointer to read/write data
//               dLength         UINT32   Length of the buffer
//               dWaitTime       UINT32   Wait time for the transaction in msec
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
        UINT32  fpBuffer;
        UINT32  dLength;
        UINT32  dWaitTime;
} SECURITY_IF;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_GET_DEV_INFO
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassGetDeviceInfo (API #27h, SubFunc 00h)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//      bDevType    UINT8   Device type byte (HDD, CD, Removable)
//      bEmuType    UINT8   Emulation type used
//      fpDevId     UINT32  Far pointer to the device ID
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;       // (Return value)
    UINT32      dSenseData;     // USB Sense data
    UINT8       bDevType;       // Device type
    UINT8       bEmuType;       // Emulation type
//  UINT8       bPhyDevType;    // Physical device type
    UINT32      fpDevId;        // Far ptr to the device id
// DO NOT ADD OR DELETE ANY FIELD ABOVE - This should match the MASS_INQUIRY
// structure for proper working
    UINT8       bTotalMassDev;  // TotalNumber of devices
    UINT8       bReserved;
    UINT16      wPciInfo;       // PCI Bus/Dev/Func number of HC the device is connected to
    UINT32      Handle[2];      // Device handle
} MASS_GET_DEV_INFO;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_GET_DEV_STATUS
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MASS_GET_DEV_STATUS (API #27h, SubFunc XXh)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      bDeviceStatus   UINT8   Connection status of the Mass device
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;
    UINT8       bDeviceStatus;
} MASS_GET_DEV_STATUS;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_GET_DEV_GEO
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassGetDeviceGeometry (API #27h,
//      SubFunc 01h)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//      bNumHeads   UINT8   Number of heads
//      wNumCylinders   UINT16  Number of cylinders
//      bNumSectors UINT8   Number of sectors
//      bLBANumHeads    UINT8   Number of heads (for INT13h function 48h)
//      wLBANumCyls UINT16  Number of cylinders (for INT13h function 48h)
//      bLBANumSectors  UINT8   Number of sectors (for INT13h function 48h)
//      wUINT8sPerSector    UINT16  Number of bytes per sector
//      bMediaType  UINT8   Media type
//      dLastLBA    UINT32  Last LBA address
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8           bDevAddr;   // (Return value)
    UINT32          dSenseData; // USB Sense data
    UINT8           bNumHeads;
    UINT16          wNumCylinders;
    UINT8           bNumSectors;
    UINT8           bLBANumHeads;
    UINT16          wLBANumCyls;
    UINT8           bLBANumSectors;
    UINT16          wBytesPerSector;
    UINT8           bMediaType;
    UINT32          dLastLBA;
    UINT8	        bInt13FuncNum;	//(EIP13457+)
    UINT8           BpbMediaDesc;
} MASS_GET_DEV_GEO;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_RESET
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassResetDevice (API #27h, SubFunc 02h)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;       // USB Device Address
    UINT32      dSenseData;     // USB Sense data
} MASS_RESET;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_ASSIGN_DRIVE_NUM
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call USBMass_AssignDriveNumber
//      (API #27h, SubFunc 0Eh)
//
// Fields:   Name       Type    Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      bLogDevNum  UINT8   Logical Drive Number to assign to the device
//      bHeads      UINT8   Number of heads
//      bSectors    UINT8   Number of sectors/track
//      wCylinders  UINT16  Number of cylinders
//      wBlockSize  UINT16  Sector size in bytes
//      bLUN        UINT8   Maximum LUNs in the system
//      bSpeed      UINT8   <>0 if the device is hi-speed device
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;   // USB Device Address
    UINT8       bLogDevNum; // Logical Drive Number to assign to the device
    UINT8       bHeads;
    UINT8       bSectors;
    UINT16      wCylinders;
    UINT16      wBlockSize;
    UINT8       bLUN;
    UINT8       bSpeed;
} MASS_ASSIGN_DRIVE_NUM;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_READ
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassReadDevice (API #27h, SubFunc 03h)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//      dStartLBA   UINT32  Starting LBA address
//      wNumBlks    UINT16  Number of blocks to read
//      wPreSkipSize    UINT16  Number of bytes to skip before
//      wPostSkipSize   UINT16  Number of bytes to skip after
//      fpBufferPtr UINT32  Far buffer pointer
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;       // USB Device Address
    UINT32      dSenseData;     // USB Sense data
    UINT32      dStartLBA;      // Starting LBA address
    UINT16      wNumBlks;       // Number of blocks to read
    UINT16      wPreSkipSize;   // Number of bytes to skip before
    UINT16      wPostSkipSize;  // Number of bytes to skip after
    UINT32      fpBufferPtr;    // Far buffer pointer
} MASS_READ;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_WRITE
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassWriteDevice (API #27h, SubFunc 04h)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//      dStartLBA   UINT32  Starting LBA address
//      wNumBlks    UINT16  Number of blocks to write
//      wPreSkipSize    UINT16  Number of bytes to skip before
//      wPostSkipSize   UINT16  Number of bytes to skip after
//      fpBufferPtr UINT32  Far buffer pointer
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;       // USB Device Address
    UINT32      dSenseData;     // USB Sense data
    UINT32      dStartLBA;      // Starting LBA address
    UINT16      wNumBlks;       // Number of blocks to write
    UINT16      wPreSkipSize;   // Number of bytes to skip before
    UINT16      wPostSkipSize;  // Number of bytes to skip after
    UINT32      fpBufferPtr;    // Far buffer pointer
} MASS_WRITE;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_VERIFY
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassVerifyDevice (API #27h, SubFunc 05h)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//      dStartLBA   UINT32  Starting LBA address
//      wNumBlks    UINT16  Number of blocks to verify
//      wPreSkipSize    UINT16  Number of bytes to skip before
//      wPostSkipSize   UINT16  Number of bytes to skip after
//      fpBufferPtr UINT32  Far buffer pointer
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;       // USB Device Address
    UINT32      dSenseData;     // USB Sense data
    UINT32      dStartLBA;      // Starting LBA address
    UINT16      wNumBlks;       // Number of blocks to verify
    UINT16      wPreSkipSize;   // Number of bytes to skip before
    UINT16      wPostSkipSize;  // Number of bytes to skip after
    UINT32      fpBufferPtr;    // Far buffer pointer
} MASS_VERIFY;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_FORMAT
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassFormatDevice (API #27h, SubFunc 06h)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//      bHeadNumber UINT8   Head number to format
//      bTrackNumber    UINT8   Track number to format
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;       // USB Device Address
    UINT32      dSenseData;     // USB Sense data
    UINT8       bHeadNumber;    // Head number to format
    UINT8       bTrackNumber;   // Track number to format
} MASS_FORMAT;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_REQ_SENSE
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassRequestSense (API #27h, SubFunc 07h)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;       // USB Device Address
    UINT32      dSenseData;     // USB Sense data
} MASS_REQ_SENSE;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_TEST_UNIT_RDY
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassTestUnitReady (API #27h, SubFunc 08h)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;       // USB Device Address
    UINT32      dSenseData;     // USB Sense data
} MASS_TEST_UNIT_RDY;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_START_STOP_UNIT
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassStartStopUnit (API #27h, SubFunc 09h)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//      bCommand    UINT8   0 - Stop, 1 - Start
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;       // USB Device Address
    UINT32      dSenseData;     // USB Sense data
    UINT8       bCommand;       // 0 - Stop, 1 - Start
} MASS_START_STOP_UNIT;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_READ_CAPACITY
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassReadCapacity (API #27h, SubFunc 0Ah)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//      dMaxLBA     UINT32  Maximum LBA address
//      dBlockSize  UINT32  Block size
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;       // USB Device Address
    UINT32      dSenseData;     // USB Sense data
    UINT32      dMaxLBA;        // Max LBA address
    UINT32      dBlockSize;     // Block size
} MASS_READ_CAPACITY;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_MODE_SENSE
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassModeSense (API #27h, SubFunc 0Bh)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//      bNumHeads   UINT8   Number of heads
//      wNumCylinders   UINT16  Number of cylinders
//      bNumSectors UINT8   Number of sectors
//      wBytesPerSector UINT16  Number of bytes per sector
//      bMediaType  UINT8   Media type
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;       // USB Device Address
    UINT32      dSenseData;     // USB Sense data
    UINT8       bNumHeads;      // Number of heads
    UINT16      wNumCylinders;  // Number of cylinders
    UINT8       bNumSectors;    // Number of sectors
    UINT16      wBytesPerSector;// Number of bytes per sector
    UINT8       bMediaType;     // Media type
} MASS_MODE_SENSE;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_INQUIRY
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassInquiry (API #27h, SubFunc 0Ch)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    UINT8   USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//      bDevType    UINT8   Device type byte (HDD, CD, Removable)
//      bEmuType    BYTE    Emulation type used
//      fpDevId     UINT32  Far pointer to the device ID
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;       // USB Device Address
    UINT32      dSenseData;     // USB Sense data
    UINT8       bDevType;       // Device type
    UINT8       bEmuType;       // Emulation type
    UINT32      fpDevId;        // Far ptr to the device id
// DO NOT ADD OR DELETE ANY FIELD ABOVE - This should match the
// MASS_GET_DEV_INFO structure for proper working
} MASS_INQUIRY;

typedef struct {
    DEV_INFO        *fpDevInfo;
    MASS_INQUIRY    *fpInqData;
} MASS_GET_DEV_PARMS;

typedef struct {
    DEV_INFO*   fpDevInfo;
} MASS_CHK_DEV_READY;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_CMD_PASS_THRU
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call MassCmdPassThru command (API #27h,
//      SubFunc 0Dh)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bDevAddr    BYTE    USB device address of the device
//      dSenseData  UINT32  Sense data of the last command
//      fpCmdBuffer UINT32  Far pointer to the command buffer
//      wCmdLength  UINT16  Command length
//      fpDataBuffer    UINT32  Far pointer for data buffer
//      wDataLength UINT16  Data length
//      bXferDir    BYTE    Data transfer direction
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       bDevAddr;
    UINT32      dSenseData;
    UINT32      fpCmdBuffer;
    UINT16      wCmdLength;
    UINT32      fpDataBuffer;
    UINT16      wDataLength;
    UINT8       bXferDir;
} MASS_CMD_PASS_THRU;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        HCPROC_PARAM
//
// Description: N/A
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _HCPROC_PARAM{
    VOID*       paramBuffer; //parameters as they should apear in stack of
                            // of the corresponding function invocation
    unsigned    bHCType;
    unsigned    paramSize;
    UINTN       retVal;
} HCPROC_PARAM;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        COREPROC_PARAM
//
// Description: This is a Core Procedure URP  structure for
//      the BIOS API call core command (API #2eh )
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _COREPROC_PARAM{
    VOID*       paramBuffer; //parameters as they should apear in stack of
                            // of the corresponding function invocation
    unsigned    paramSize;
    UINTN       retVal;
} COREPROC_PARAM, * FPCOREPROC_PARAM;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_HOTPLUG
//
// Description: This is a Mass URP (Mass USB Request Packet) structure for
//      the BIOS API call USBMass_HotPlugDeviceSupport
//      (API #27h, SubFunc 09h)
//
// Fields:   Name       Type    Description
//      ------------------------------------------------------------
//      bDevAddr    BYTE    USB device address of the device
//      bNumUSBFDD  BYTE    Number of USB FDD's installed
//      bNumUSBCDROM    BYTE    Number of USB CDROM's installed
//      bDeviceFlag BYTE    Flag indicating what hot plug devices to be added
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _MASS_HOTPLUG {
    UINT8   bDevAddr;       // USB Device Address
    UINT8   bNumUSBFDD;     // Number of USB FDD's installed
    UINT8   bNumUSBCDROM;   // Number of USB CDROM's installed
    UINT8   bDeviceFlag;    // Flag indicating what hot plug devices to be added
} MASS_HOTPLUG;

#define USB_HOTPLUG_ENABLE_FDD      BIT0
#define USB_HOTPLUG_ENABLE_CDROM    BIT1
#define USB_HOTPLUG_HDD_ADDRESS     0x7D
#define USB_HOTPLUG_FDD_ADDRESS     0x7E
#define USB_HOTPLUG_CDROM_ADDRESS   0x7F

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        GET_DEV_ADDR
//
// Description: This is a URP structure for the BIOS API(API #32h)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT16	Vid;		// Vendor Id
	UINT16	Did;		// Device Id
    UINT8   DevAddr;	// USB Device Address
} GET_DEV_ADDR;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        HC_START_STOP
//
// Description: This is a URP structure for the BIOS API(API #36)
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
	BOOLEAN 	Start;
	HC_STRUC    *HcStruc;
} HC_START_STOP;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        CCID_GETSMARTCLASSDESCRIPTOR
//
// Description: This is a Core Procedure URP  structure for
//      the BIOS API call USB_API_CCID_DEVICE_REQUEST command (API #2Fh )
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _CCID_GETSMARTCLASSDESCRIPTOR{
    OUT UINTN            fpResponseBuffer;
    IN  UINT8            Slot;
    OUT UINTN            fpDevInfo;            
} CCID_GETSMARTCLASSDESCRIPTOR, * FPCCID_GETSMARTCLASSDESCRIPTOR;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        CCID_ATR
//
// Description: This is a Core Procedure URP  structure for
//      the BIOS API call USB_API_CCID_DEVICE_REQUEST command (API #2Fh )
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _CCID_ATR{
    IN UINT8            Slot;
    IN OUT UINTN        ATRData;
    OUT UINTN           fpDevInfo;
} CCID_ATR, * FPCCID_ATR;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        CCID_POWERUP_SLOT
//
// Description: This is a Core Procedure URP  structure for
//      the BIOS API call USB_API_CCID_DEVICE_REQUEST command (API #2Fh )
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _CCID_POWERUP_SLOT{
    IN UINT8            Slot;
    OUT UINT8            bStatus;
    OUT UINT8            bError;
    IN OUT UINTN        ATRData;
    OUT UINTN            fpDevInfo;                        
} CCID_POWERUP_SLOT, * FPCCID_POWERUP_SLOT;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        CCID_POWERDOWN_SLOT
//
// Description: This is a Core Procedure URP  structure for
//      the BIOS API call USB_API_CCID_DEVICE_REQUEST command (API #2Fh )
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _CCID_POWERDOWN_SLOT{
    IN  UINT8            Slot;
    OUT UINT8            bStatus;
    OUT UINT8            bError;
    OUT UINTN            fpDevInfo;            
} CCID_POWERDOWN_SLOT, * FPCCID_POWERDOWN_SLOT;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        CCID_GETSLOT_STATUS
//
// Description: This is a Core Procedure URP  structure for
//      the BIOS API call USB_API_CCID_DEVICE_REQUEST command (API #2Fh )
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _CCID_GETSLOT_STATUS{
    OUT UINT8            bStatus;
    OUT UINT8            bError;
    OUT UINT8            bClockStatus;    
    IN  UINT8            Slot;
    OUT UINTN            fpDevInfo;            
} CCID_GETSLOT_STATUS, * FPCCID_GETSLOT_STATUS;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        CCID_XFRBLOCK
//
// Description: This is a Core Procedure URP  structure for
//      the BIOS API call USB_API_CCID_DEVICE_REQUEST command (API #2Fh )
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _CCID_CCID_XFRBLOCK{
    IN UINTN            CmdLength;
    IN UINTN            fpCmdBuffer;
    IN UINT8            ISBlock;
    OUT UINT8            bStatus;
    OUT UINT8            bError;
    IN OUT UINTN        ResponseLength;
    OUT UINTN            fpResponseBuffer;
    IN  UINT8            Slot;
    OUT UINTN            fpDevInfo;
} CCID_XFRBLOCK, * FPCCID_XFRBLOCK;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        CCID_GET_PARAMS
//
// Description: This is a Core Procedure URP  structure for
//      the BIOS API call USB_API_CCID_DEVICE_REQUEST command (API #2Fh )
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct _CCID_GET_PARAMS{
    OUT UINT8            bStatus;
    OUT UINT8            bError;
    IN OUT UINTN        ResponseLength;
    OUT UINTN            fpResponseBuffer;
    IN  UINT8            Slot;
    OUT UINTN            fpDevInfo;
} CCID_GET_PARAMS, * FPCCID_GET_PARAMS;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        API_DATA
//
// Description: This is a union data type of all the API related data
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef union {
    CK_PRESENCE             CkPresence;
    START_HC                StartHc;
    GET_DEV_INFO            GetDevInfo;
    CHK_DEV_PRSNC           ChkDevPrsnc;
    SECURITY_IF             SecurityIf;
    MASS_GET_DEV_INFO       MassGetDevInfo;
    MASS_GET_DEV_STATUS     MassGetDevSts;
    MASS_GET_DEV_GEO        MassGetDevGeo;
    MASS_RESET              MassReset;
    MASS_READ               MassRead;
    MASS_WRITE              MassWrite;
    MASS_VERIFY             MassVerify;
    MASS_FORMAT             MassFormat;
    MASS_REQ_SENSE          MassReqSense;
    MASS_TEST_UNIT_RDY      MassTstUnitRdy;
    MASS_START_STOP_UNIT    MassStartStop;
    MASS_READ_CAPACITY      MassReadCap;
    MASS_MODE_SENSE         MassModeSense;
    MASS_INQUIRY            MassInquiry;
    MASS_CMD_PASS_THRU      MassCmdPassThru;
    MASS_ASSIGN_DRIVE_NUM   MassAssignNum;
    MASS_CHK_DEV_READY      MassChkDevReady;
    MASS_GET_DEV_PARMS      MassGetDevParms;
    KB_LEDS_DATA            KbLedsData;
    UINT8                   Owner;
    HCPROC_PARAM            HcProc;
    COREPROC_PARAM          CoreProc;
    UINT8                   KbcControlCode;            //(EIP29733+)
    GET_DEV_ADDR            GetDevAddr;
    UINT8                   DevAddr;
    //    CCID APIs
    CCID_GETSMARTCLASSDESCRIPTOR CCIDSmartClassDescriptor;
    CCID_ATR                    CCIDAtr;
    CCID_POWERUP_SLOT           CCIDPowerupSlot;
    CCID_POWERDOWN_SLOT         CCIDPowerdownSlot;
    CCID_GETSLOT_STATUS         CCIDGetSlotStatus;
    CCID_XFRBLOCK               CCIDXfrBlock;
    CCID_GET_PARAMS             CCIDGetParameters;
	UINT16					HcBusDevFuncNum;    //(EIP74876+)
	HC_START_STOP			HcStartStop;
} U_API_DATA;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        URP_STRUC
//
// Description: This structure is the URP structure
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bFuncNumber UINT8       Function number of the URP
//      bSubFunc    UINT8       Sub-func number of the URP
//      bRetValue   UINT8       Return value
//      ApiData     API_DATA    Refer structure definition
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

struct _URP_STRUC {
    UINT8       bFuncNumber;
    UINT8       bSubFunc;
    UINT8       bRetValue;
    U_API_DATA  ApiData;
};

#pragma pack(pop)

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        MASS_XACT_STRUC
//
// Description: This structure holds the information needed for the mass
//      transaction (for CBI or BULK)
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      pCmdBuffer  UINT16  Pointer to the mass transaction command buffer
//      bCmdSize    UINT8   Size of the command buffer
//      bXferDir    UINT8   Transfer direction (BIT7)
//      fpBuffer    UINT32  Far pointer of the data buffer (IN/OUT)
//      dwLength    UINT32  Length of the data buffer
//      wPreSkip    UINT16  Number of bytes to skip before getting actual data
//      wPostSkip   UINT16  Number of bytes to skip after getting actual data
//      wMiscFlag   UINT16  Flag for special cases refer USBM_XACT_FLAG_XXX
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       *fpCmdBuffer;
    UINT8       bCmdSize;
    UINT8       bXferDir;
    UINT8       *fpBuffer;
    UINT32      dLength;
    UINT16      wPreSkip;
    UINT16      wPostSkip;
    UINT16      wMiscFlag;
} MASS_XACT_STRUC;

#define USBM_XACT_FLAG_32BIT_DATA_BUFFER        BIT0

typedef struct _QUEUE_T{
    VOID* volatile* data;
    int maxsize;
    volatile int head;
    volatile int tail;
} QUEUE_T;
										//(EIP38434+)>
typedef struct _ABS_MOUSE{
    UINT8   ReportID;
    UINT8   ButtonStauts;
    UINT16  Xcoordinate;
    UINT16  Ycoordinate;
    UINT16  Pressure;
	UINT16	Max_X;
	UINT16  Max_Y;
} ABS_MOUSE;
										//<(EIP38434+)
typedef struct MOUSE_DATA{
    UINT8   ButtonStatus;
    INT32   MouseX;
    INT32   MouseY;
    INT32   MouseZ;
} MOUSE_DATA;


#define MAX_NOTIFICATIONS_COUNT 100

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        USB_GLOBAL_DATA
//
// Description: USB Global Data Area structure
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT32          dUSBSignature;
    USB_VERSION     stUSBVersion;
    UINT32          dUSBStateFlag;
    UINT16          aErrorLogBuffer[MAX_USB_ERRORS_NUM];
    UINT8           bErrorLogIndex;
    HCD_HEADER      aHCDriverTable[MAX_HC_TYPES];           // For 4 type of HC
    DEV_DRIVER      aDevDriverTable[MAX_DEVICE_TYPES];      // For 5 types of devices
    DEV_DRIVER      aDelayedDrivers[MAX_DEVICE_TYPES];      // For 5 types of devices
    DEV_INFO        aDevInfoTable[MAX_DEVICES];
    HC_STRUC        **HcTable;
    UINT8           HcTableCount;
    UINT8           NumOfHc;
    DEV_INFO        FddHotplugDev;
    DEV_INFO        HddHotplugDev;
    DEV_INFO        CdromHotplugDev;
    UINT8           bCallBackFunctionIndex;
    CALLBACK_FUNC   aCallBackFunctionTable[MAX_CALLBACK_FUNCTION];
    UINT64          DeviceAddressMap;
    UINT8           bEnumFlag;
	UINT32			MemPages;
    UINT8           *fpMemBlockStart;
	UINT32			MemBlkStsBytes;
    UINT32          *aMemBlkSts;
//Hub related fields
    UINT32          dHubPortStatus;
    //UINT16          wHubPortStatus;
//KBD related fields
// Buffer to store keyboard shift key status bytes. This is correlated with
// scan code buffer to generate proper scan code sequence
	UINT8			aKBCShiftKeyStatusBufferStart[16];
	UINT8			aKBCDeviceIDBufferStart[16];	// Buffer to store keyboard device ID
// Buffer to store keyboard shift key status bytes. This is correlated with
// scan code buffer to generate proper scan code sequence
	UINT8			aKBCScanCodeBufferStart[16];

    UINT8           aKBCCharacterBufferStart[128];	//(EIP29345)
    UINT8           bCurrentUSBKeyCode;
    UINT8           bUSBKBShiftKeyStatus;
    UINT8           bNonUSBKBShiftKeyStatus;
    UINT8           bUSBKBC_ExtStatusFlag;
    UINT8           bUSBDeviceList;
    UINT8           bSet2ScanCode;          // Temporary storage for the scan code set 2 scan code
    UINT8           bLastUSBKeyCode;        // Last USB key code processed
    UINT8           bBreakCodeDeviceID;     // Device IDs for the keyboards generating break code
    UINT8           bCurrentDeviceID;       // Current USB keyboard device ID
    UINT16          wUSBKBC_StatusFlag;
    UINT16          wRepeatCounter;         // Typematic repeat counter
    UINT16          wRepeatRate;            // Typematic repeat rate
    UINT8           *fpKBCCharacterBufferHead;  // Keyboard character buffer head and tail pointers
    UINT8           *fpKBCCharacterBufferTail;
    UINT8           *fpKBCScanCodeBufferPtr;        // Keyboard scan code buffer pointer
    UINT8           bUSBKBC_MassStorage;
    UINT8           bKbdDataReady;
    UINT8           aKBInputBuffer[16];     // Keyboard expanded input buffer pointer (null-terminated)
    UINT8           bCCB;
    VOID            *EfiKeyboardBuffer;
    UINT8           RepeatKey;
    HC_STRUC        *fpSavedHCStruc;      // Temporary location to store the HCStruc pointer
    HC_STRUC        *fpKeyRepeatHCStruc;
    DEV_INFO        *fpKeyRepeatDevInfo;
    DEV_INFO        *aUSBKBDeviceTable[USB_DEV_HID_COUNT];
// Added by mouse driver
    MOUSE_DATA      MouseData;				
    UINT8           aMouseInputBuffer[15];
    ABS_MOUSE       AbsMouseData[10];			//(EIP38434+)
// Mouse input buffer head and tail pointers
    UINT8           *fpMouseInputBufferHeadPtr;
    UINT8           *fpMouseInputBufferTailPtr;
    UINT8           bMouseStatusFlag;
                    // Bit 7   : Mouse enabled bit (1/0)
                    // Bit 6   : Mouse data ready (1/0)
                    // BIT 5   : Mouse data from USB (1/0)
                    // BIT 4   : 4-byte mouse data (1/0)
                    // Bit 3-0 : Reserved
    UINT8           *fpUSBTempBuffer;
    UINT8           *fpUSBMassConsumeBuffer;
    UINT32          wInterruptStatus;
    URP_STRUC       *fpURP;     // Request Packet pointer
// BOTCommandTag used to maintain BOT command block number
    UINT32          dBOTCommandTag;
    //UINT16          wMassTempData;
    UINT8           bUSBStorageDeviceDelayCount;
    UINT16          wBulkDataXferDelay;
    MASS_XACT_STRUC stMassXactStruc;
// Flag that allows mass storage device to handle special conditions. The
// bit pattern is defined by the USBMASS_MISC_FLAG_XXX equates in USB.EQU
    UINT16          wMassStorageMiscFlag;
    UINT8           bGeometryCommandStatus;
    UINT8           bModeSenseSectors;
    UINT8           bModeSenseHeads;
    UINT16          wModeSenseCylinders;
    UINT16          wModeSenseBlockSize;
    UINT32          dModeSenseMaxLBA;
    UINT8           bReadCapSectors;
    UINT8           bReadCapHeads;
    UINT16          wReadCapCylinders;
    UINT16          wReadCapBlockSize;
    UINT32          dReadCapMaxLBA;
    UINT8           bDiskMediaType;
    UINT16          wTimeOutValue;
    UINT8           bLastCommandStatus;
    UINT32          dLastCommandStatusExtended;
// Added by EHCI driver
    //UINT8           aControlSetupData[8];
    EHCI_QH         *fpQHAsyncXfer;
    UINT8           bIgnoreConnectStsChng;
    UINT8           bProcessingPeriodicList;
    UINT8           bHandOverInProgress;
    HC_STRUC		*RootHubHcStruc;

// Tokens representation for the module binary
    UINT8           kbc_support;
    UINT8           fdd_hotplug_support;
    UINT8           hdd_hotplug_support;
    UINT8           cdrom_hotplug_support;
    UINT8           UsbMassResetDelay;  // 0/1/2/3 for 10/20/30/40 seconds
    UINT8           PowerGoodDeviceDelay;  // 0/1/5/6/../10 seconds
    UINT8			UsbXhciSupport;		// 0/1 for Disabled/Enabled
    UINT8           UsbXhciHandoff;     // 0/1 for Disabled/Enabled
    UINT8           UsbEhciHandoff;     // 0/1 for Disabled/Enabled
    UINT8           UsbOhciHandoff;     // 0/1 for Disabled/Enabled
    UINT8           UsbEmul6064;        // 0/1 for Disabled/Enabled
    UINT8           NumberOfFDDs;
    UINT8           NumberOfHDDs;
    UINT8           NumberOfCDROMs;
    UINT8           USBMassEmulationOptionTable[16];
    QUEUE_T         QueueCnnctDisc;
    QUEUE_T         ICCQueueCnnctDisc;    //QueueCnnctDisc will work of USB devices. Smart Card reader will work in that queue. But Smart Card(ICC) isn't a USB device. So create a new queue to handle it.
    DEV_INFO        *QueueData1[MAX_NOTIFICATIONS_COUNT];
    UINT8   		*gUsbSkipListTable;			//(EIP51653+)	
    UINT8			UsbHiSpeedSupport;
    USB_TIMING_POLICY   UsbTimingPolicy;
    USB_SUPPORT_SETUP   UsbSetupData;	//(EIP99882+)
} USB_GLOBAL_DATA;

// Note: If additional space is needed in USB data segment,
// MAX_BULK_DATA_SIZE can be changed to 200h without significant
// decrease in mass storage data transfer performance
// .. moved to SDL
//#define MAX_BULK_DATA_SIZE          0x0400          // Maximum amount of data to transfer

#define USB_HC_CLASS_CODE           0x0C03

#define MAX_NUM_HC_MODULES          0x03

// Equates to disable/enable USB port interrupt generation and
// 060/064h trapping
#define USB_DISABLE_INTERRUPT       0x000
#define USB_SAFE_DISABLE_INTERRUPT  0x001
#define USB_ENABLE_INTERRUPT        0x00F

#define TRAP_REQUEST_CLEAR          0x000
#define TRAP_REQUEST_DISABLE        0x0FF

//---------------------------------------------------------------------------
// Equates for Generic USB specific registers in the PCI config space
//---------------------------------------------------------------------------
#define USB_REG_COMMAND         0x004
#define USB_MEM_BASE_ADDRESS    0x010   // Offset 10-13h
#define USB_IO_BASE_ADDRESS     0x020
#define USB_IRQ_LEVEL           0x03C
#define USB_RELEASE_NUM         0x060
#define USB_HC_CLASS_CODE       0x0C03

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//      Equates common to all host controllers
//---------------------------------------------------------------------------
#define USB_PORT_STAT_DEV_CONNECTED                 BIT0
#define USB_PORT_STAT_DEV_LOWSPEED                  BIT1
#define USB_PORT_STAT_DEV_FULLSPEED                 BIT2
#define USB_PORT_STAT_DEV_HISPEED                   0//(BIT1 + BIT2)
#define USB_PORT_STAT_DEV_SUPERSPEED                BIT3
#define USB_PORT_STAT_DEV_SPEED_MASK                (BIT1 + BIT2 + BIT3)
#define USB_PORT_STAT_DEV_SPEED_MASK_SHIFT          0x1
#define USB_PORT_STAT_DEV_CONNECT_CHANGED           BIT4
#define USB_PORT_STAT_DEV_ENABLED					BIT5
#define USB_PORT_STAT_DEV_OWNER                     BIT6

#define USB_DEV_SPEED_LOW	(USB_PORT_STAT_DEV_LOWSPEED >> USB_PORT_STAT_DEV_SPEED_MASK_SHIFT)
#define USB_DEV_SPEED_FULL	(USB_PORT_STAT_DEV_FULLSPEED >> USB_PORT_STAT_DEV_SPEED_MASK_SHIFT)
#define USB_DEV_SPEED_HIGH	(USB_PORT_STAT_DEV_HISPEED >> USB_PORT_STAT_DEV_SPEED_MASK_SHIFT)
#define USB_DEV_SPEED_SUPER	(USB_PORT_STAT_DEV_SUPERSPEED >> USB_PORT_STAT_DEV_SPEED_MASK_SHIFT)

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//      Equates related to USB equipment list
//---------------------------------------------------------------------------
                                        //(EIP84455+)>
#define USB_TYPE_KEYBOARD       BIT0
#define USB_TYPE_MOUSE          BIT1
#define USB_TYPE_POINT          BIT2    
#define USB_TYPE_HUB            BIT3
#define USB_TYPE_MASS_STORAGE   BIT4
                                        //<(EIP84455+)

// Bulk transfer error status (bLastCommandStatus)
#define USB_BULK_STALLED        BIT0
#define USB_BULK_TIMEDOUT       BIT1
#define USB_CONTROL_STALLED     BIT2

#define USB_TRNSFR_ERRBIT_SHIFT 0
#define USB_TRNSFR_BITSTUFF     BIT0
#define USB_TRNSFR_CRCERROR     BIT1
#define USB_TRNSFR_NAK          BIT2
#define USB_TRNSFR_BABBLE       BIT3
#define USB_TRSFR_BUFFER_ERROR  BIT4
#define USB_TRSFR_STALLED       BIT5
#define USB_TRNSF_ERRORS_MASK   (USB_TRNSFR_ERRBIT_SHIFT | \
        USB_TRNSFR_BITSTUFF | USB_TRNSFR_CRCERROR |\
        USB_TRNSFR_NAK|USB_TRNSFR_BABBLE|USB_TRSFR_BUFFER_ERROR )
#define USB_TRNSFR_TIMEOUT      BIT6

//----------------------------------------------------------------------------
// Equates regarding USB device info structure search parameter
//----------------------------------------------------------------------------
#define USB_SRCH_DEV_ADDR       0x10        //
#define USB_SRCH_DEV_TYPE       0x20        // Next device of a given type
#define USB_SRCH_DEV_NUM        0x30        // Number of devices of certain type
#define USB_SRCH_DEVBASECLASS_NUM 0x31      // Number of devices of certain base class
#define USB_SRCH_HC_STRUC       0x40        // Next device of a given HC
#define USB_SRCH_DEV_INDX       0x80        // Device of a given index
/*
// USB Initialization Flags - passed in when USB is initialized
//----------------------------------------------------------------------------
#define INIT_FLAG_MANUAL            0x07        //Bit 2-0: 000 = Auto enum
                    //         001 = KB on port 1
                    //         ...   ...
                    //         111 = KB on port 7
#define INIT_FLAG_ENUM_DISABLE      0x08        //    3: If set, do not enum the USB
#define INIT_FLAG_BEEP_DISABLE      0x10        //  4: If set, do not beep on new devices
#define INIT_FLAG_USB_STOP_EHCI_IN_OHCI_HANDOVER 0x20
*/
//----------------------------------------------------------------------------
//      Bit definitions for DeviceRequest.RequestType
//----------------------------------------------------------------------------
//               Bit 7:   Data direction
#define USB_REQ_TYPE_OUTPUT         0x00    //  0 = Host sending data to device
#define USB_REQ_TYPE_INPUT          0x80    //  1 = Device sending data to host

//               Bit 6-5: Type
#define USB_REQ_TYPE_STANDARD       0x00    //  00 = Standard USB request
#define USB_REQ_TYPE_CLASS          0x20    //  01 = Class specific
#define USB_REQ_TYPE_VENDOR         0x40    //  10 = Vendor specific

//               Bit 4-0: Recipient
#define USB_REQ_TYPE_DEVICE         0x00    //  00000 = Device
#define USB_REQ_TYPE_INTERFACE      0x01    //  00001 = Interface
#define USB_REQ_TYPE_ENDPOINT       0x02    //  00010 = Endpoint
#define USB_REQ_TYPE_OTHER          0x03    //  00011 = Other

//----------------------------------------------------------------------------
// Values for DeviceRequest.RequestType and DeviceRequest.RequestCode fields
// combined as a word value.
//---------------------------------------------------------------------------
#define USB_RQ_SET_ADDRESS          ((0x05 << 8) | USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE)
#define USB_RQ_GET_DESCRIPTOR       ((0x06 << 8) | USB_REQ_TYPE_INPUT  | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE)
#define USB_RQ_GET_CONFIGURATION    ((0x08 << 8) | USB_REQ_TYPE_INPUT  | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE)
#define USB_RQ_SET_CONFIGURATION    ((0x09 << 8) | USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE)
#define USB_RQ_SET_INTERFACE        ((0x0B << 8) | USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE)
#define USB_RQ_SET_FEATURE          ((0x03 << 8) | USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_DEVICE)

#define USB_FSEL_DEV_REMOTE_WAKEUP  01

#define USB_RQ_GET_CLASS_DESCRIPTOR ((0x06 << 8) | USB_REQ_TYPE_INPUT  | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_DEVICE)
#define HID_RQ_GET_DESCRIPTOR	    ((0x06 << 8) | USB_REQ_TYPE_INPUT  | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE)	//(EIP38434+)
#define HID_RQ_SET_PROTOCOL         ((0x0B << 8) | USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE)
#define HID_RQ_SET_REPORT           ((0x09 << 8) | USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE)
#define HID_RQ_SET_IDLE             ((0x0A << 8) | USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE)
#define HUB_RQ_GET_PORT_STATUS      ((0x00 << 8) | USB_REQ_TYPE_INPUT  | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_OTHER)
#define HUB_RQ_SET_PORT_FEATURE     ((0x03 << 8) | USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_OTHER)
#define HUB_RQ_CLEAR_PORT_FEATURE   ((0x01 << 8) | USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_OTHER)
#define HUB_RQ_SET_HUB_DEPTH	    ((0x0C << 8) | USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_DEVICE)

//----------------------------------------------------------------------------
//      Bit definitions for HubDescriptor.HubFlags
//----------------------------------------------------------------------------
#define HUB_FLAG_PWR_MODE_BITS      0x03    //Bit 1-0: Power switching mode used by hub
#define HUB_FLAG_PWR_MODE_GANG      0x00    //    =00: All ports power on/off together
#define HUB_FLAG_PWR_MODE_INDIV     0x01    //    =01: Ports power on/off individually
#define HUB_FLAG_PWR_MODE_FIXED     0x02    //    =1x: Ports power is always on
#define HUB_FLAG_COMPOUND_DEV       0x04    //Bit 2: If set, hub is part of a compound device
#define HUB_FLAG_OVR_CUR_BITS       0x18    //Bit 4-3: Over-current protection mode used by hub
#define HUB_FLAG_OVR_CUR_GLOBAL     0x00    //    =00: Hub reports only global over-current status
#define HUB_FLAG_OVR_CUR_INDIV      0x08    //    =01: Hub reports individual over-current status
#define HUB_FLAG_OVR_CUR_NONE       0x10    //    =1x: Hub has no over-current protection
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//          Hub Class Feature Selectors
//----------------------------------------------------------------------------
#define	HUB_FEATURE_C_HUB_LOCAL_POWER		0
#define	HUB_FEATURE_C_HUB_OVER_CURRENT		1
#define	HUB_FEATURE_PORT_CONNECTION			0
#define HUB_FEATURE_PORT_ENABLE				1	//Hub port enable feature
#define	HUB_FEATURE_PORT_SUSPEND			2
#define HUB_FEATURE_PORT_OVER_CURRENT		3
#define HUB_FEATURE_PORT_RESET				4	//Hub port reset feature
#define HUB_FEATURE_PORT_LINK_STATE			5
#define HUB_FEATURE_PORT_POWER				8	//Hub port power feature
#define HUB_FEATURE_PORT_LOW_SPEED      	9	//Hub port low speed feature
#define HUB_FEATURE_C_PORT_CONNECTION		16	//Hub port connect change feature
#define HUB_FEATURE_C_PORT_ENABLE  			17	//Hub port enable change feature
#define HUB_FEATURE_C_PORT_SUSPEND			18
#define HUB_FEATURE_C_PORT_OVER_CURRENT 	19
#define HUB_FEATURE_C_PORT_RESET			20	//Hub port reset change feature
#define HUB_FEATURE_PORT_U1_TIMEOUT			23
#define HUB_FEATURE_PORT_U2_TIMEOUT			24
#define HUB_FEATURE_C_PORT_LINK_STATE		25
#define HUB_FEATURE_C_PORT_CONFIG_ERROR		26
#define HUB_FEATURE_PORT_REMOTE_WAKE_MASK	27
#define	HUB_FEATURE_BH_PORT_RESET			28
#define	HUB_FEATURE_C_BH_PORT_RESET			29
#define	HUB_FEATURE_FORCE_LINKPM_ACCEPT		30

typedef enum {
	HubLocalPower = 0,
	HubOverCurrent,
	PortConnection = 0,
	PortEnable,
	PortSuspend,
	PortOverCurrent,
	PortReset,
	PortLinkState,
	PortPower = 8,
	PortLowSpeed,
	PortConnectChange = 16,
	PortEnableChange,
	PortSuspendChange,
	PortOverCurrentChange,
	PortResetChange,
	PortTest,
	PortIndicator,
	PortU1Timeout,
	PortU2Timeout,
	PortLinkStateChange,
	PortConfigErrorChange,
	PortRemoteWakeMask,
	BhPortReset,
	BhPortResetChange,
	ForceLinkPmAccept
} HUB_FEATURE;

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//          Hub Port Status Bit Definitions
//----------------------------------------------------------------------------
#define HUB_PORT_STATUS_PORT_CONNECTION		BIT0	//Bit 0: Set if device present
#define HUB_PORT_STATUS_PORT_ENABLED		BIT1 	//Bit 1: Set if port is enabled
#define HUB_PORT_STATUS_PORT_SUSPEND		BIT2 	//Bit 2: Set if device on port is suspended
#define HUB_PORT_STATUS_PORT_OVER_CURRENT	BIT3	//Bit 3: Set if port has been powered down due to over-current
#define HUB_PORT_STATUS_PORT_RESET			BIT4	//Bit 4: Set if reset sigaling is active
#define HUB_PORT_STATUS_PORT_POWER			BIT8 	//Bit 8: Set if port is enabled
#define HUB_PORT_STATUS_PORT_LOW_SPEED		BIT9 	//Bit 9: Set if a low speed device is attached
#define HUB_PORT_STATUS_PORT_HIGH_SPEED     BIT10 	//Bit 10: Set if a high speed device is attached
#define	HUB_PORT_STATUS_PORT_TEST			BIT11
#define	HUB_PORT_STATUS_PORT_INDICATOR		BIT12
#define HUB_PORT_STATUS_C_PORT_CONNECTION	(BIT0 << 16)	//Bit 0: Set if device has been attached/removed
#define HUB_PORT_STATUS_C_PORT_ENABLE		(BIT1 << 16)	//Bit 1: Set if port has been enabled/disabled by hardware in hub
#define HUB_PORT_STATUS_C_PORT_SUSPEND		(BIT2 << 16)	//Bit 2: Set if device has entered/left suspend state
#define HUB_PORT_STATUS_C_PORT_OVER_CURRENT	(BIT3 << 16)	//Bit 3: Set if over current indicator has changed
#define HUB_PORT_STATUS_C_PORT_RESET		(BIT4 << 16)	//Bit 4: Set when port reset sequence is complete

#define USB3_HUB_PORT_STATUS_PORT_CONNECTION		BIT0
#define USB3_HUB_PORT_STATUS_PORT_ENABLED			BIT1
#define USB3_HUB_PORT_STATUS_PORT_OVER_CURRENT		BIT3
#define USB3_HUB_PORT_STATUS_PORT_RESET				BIT4
#define USB3_HUB_PORT_STATUS_PORT_LINK_STATE		(BIT5 | BIT6 | BIT7 | BIT8)
#define USB3_HUB_PORT_STATUS_PORT_POWER				BIT9
#define USB3_HUB_PORT_STATUS_PORT_SPEED				(BIT10 | BIT11 | BIT12)
#define	USB3_HUB_PORT_STATUS_C_PORT_CONNECTION		(BIT0 << 16)
#define	USB3_HUB_PORT_STATUS_C_PORT_OVER_CURRENT	(BIT3 << 16)
#define	USB3_HUB_PORT_STATUS_C_PORT_RESET			(BIT4 << 16)
#define	USB3_HUB_PORT_STATUS_C_BH_PORT_RESET		(BIT5 << 16)
#define	USB3_HUB_PORT_STATUS_C_PORT_LINK_STATE		(BIT6 << 16)
#define	USB3_HUB_PORT_STATUS_C_PORT_CONFIG_ERROR	(BIT7 << 16)

#define	USB3_HUB_PORT_LINK_U0				0x00
#define	USB3_HUB_PORT_LINK_U1				0x01
#define	USB3_HUB_PORT_LINK_U2				0x02
#define	USB3_HUB_PORT_LINK_U3				0x03
#define	USB3_HUB_PORT_LINK_DISABLED			0x04
#define	USB3_HUB_PORT_LINK_RXDETECT			0x05
#define	USB3_HUB_PORT_LINK_INACTIVE			0x06
#define	USB3_HUB_PORT_LINK_POLLING			0x07
#define	USB3_HUB_PORT_LINK_RECOVERY			0x08
#define	USB3_HUB_PORT_LINK_HOT_RESET		0x09
#define	USB3_HUB_PORT_LINK_COMPLIANCE_MODE	0x0A
#define	USB3_HUB_PORT_LINK_LOOPBACK			0x0B

#pragma pack(push, 1)

typedef struct {
	struct {
		UINT16	Connected	:1;
		UINT16	Enabled		:1;
		UINT16	Suspend		:1;
		UINT16	OverCurrent	:1;
		UINT16	Reset		:1;
		UINT16	Reserved	:3;
		UINT16	Power		:1;
		UINT16	LowSpeed	:1;
		UINT16	HighSpeed	:1;
		UINT16	TestMode	:1;
		UINT16	Indicator	:1;
		UINT16	Reserved1	:3;
	} PortStatus;
	struct {
		UINT16	ConnectChange		:1;
		UINT16	EnableChange		:1;
		UINT16	SuspendChange		:1;
		UINT16	OverCurrentChange	:1;
		UINT16	ResetChange			:1;
		UINT16	Reserved			:11;
	} PortChange;
} HUB_PORT_STATUS;

typedef struct {
	struct {
		UINT16	Connected	:1;
		UINT16	Enabled		:1;
		UINT16	Reserved	:1;
		UINT16	OverCurrent	:1;
		UINT16	Reset		:1;
		UINT16	LinkState	:4;
		UINT16	Power		:1;
		UINT16	Speed		:3;
		UINT16	Reserved1	:3;
	} PortStatus;
	struct {
		UINT16	ConnectChange		:1;
		UINT16	Reserved			:2;
		UINT16	OverCurrentChange	:1;
		UINT16	ResetChange			:1;
		UINT16	BhResetChange		:1;
		UINT16	LinkStateChange		:1;
		UINT16	ConfigErrorChange	:1;
		UINT16	Reserved1			:8;
	} PortChange;
} USB3_HUB_PORT_STATUS;

#pragma pack(pop)

#define ENDPOINT_CLEAR_PORT_FEATURE (0x01 << 8) | USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_ENDPOINT

#define ADSC_OUT_REQUEST_TYPE       (0x00 << 8) | USB_REQ_TYPE_OUTPUT | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE
#define ADSC_IN_REQUEST_TYPE        (0x00 << 8) | USB_REQ_TYPE_INPUT  | USB_REQ_TYPE_CLASS | USB_REQ_TYPE_INTERFACE

#define ENDPOINT_HALT               00

// Standard PCI configuration register offsets and relevant values
//------------------------------------------------------------------------------
#define PCI_REG_VENDID          0x00    //PCI vendor ID register
#define PCI_REG_DEVID           0x02    //PCI device ID register
#define PCI_REG_COMMAND         0x04    //PCI command register
#define CMD_DEV_DISABLE         0x00    //Disables device when written to cmd reg
#define CMD_IO_SPACE            0x01    //IO space enable bit
#define CMD_MEM_SPACE           0x02    //Memory space enable bit
#define CMD_BUS_MASTER          0x4     //Bus master enable bit
#define CMD_SPECIAL_CYCLE       0x08    //Special cycle enable bit
#define CMD_MEM_INVALIDATE      0x10    //Memory write & invalidate enable bit
#define CMD_PAL_SNOOP           0x20    //VGA palette snoop enable bit
#define CMD_PARITY              0x40    //Parity error enable bit
#define CMD_WAIT_CYCLE          0x80    //Wait cycle control bit
#define CMD_SERR                0x100   //SERR# enable bit
#define CMD_FAST_BTOB           0x200   //Fast back-to-back enable bit
#define PCI_REG_STATUS          0x06    //PCI status register
#define STAT_RESET_ALL          0x0FFFF //Resets all status bits
#define PCI_REG_REVID           0x08    //PCI revision ID register
#define PCI_REG_IF_TYPE         0x09    //PCI interface type register
#define PCI_REG_SUB_TYPE        0x0A    //PCI sub type register
#define PCI_REG_BASE_TYPE       0x0B    //PCI base type register
#define PCI_REG_LINE_SIZE       0x0C    //PCI cache line size register
#define PCI_REG_LATENCY         0x0D    //PCI latency timer register
#define PCI_REG_LATENCY         0x0D    //PCI latency timer register
#define PCI_REG_HEADER_TYPE     0x0E    //PCI header type register
#define MULTI_FUNC_BIT          0x80    //If set, device is multi function
#define PCI_CFG_HEADER_STD      0x00    //Standard PCI config space
#define PCI_CFG_HEADER_PPB      0x01    //PCI-PCI bridge config space
#define PCI_REG_BIST            0x0F    //PCI built in self test register
#define PCI_REG_FIRST_BASE_ADD  0x10    //PCI first base address register
#define PCI_REG_LAST_BASE_ADD   0x24    //PCI last base address register
#define PCI_BASE_ADD_PORT_BIT   0x01    //If set, base add reg is for I/O port
#define PCI_BASE_ADD_MEMTYPE    0x06    //Bits in lower word that are mem type
#define PCI_BASE_ADD_MT_32      0x00    //Memory must be located at 32 bit add
#define PCI_BASE_ADD_MT_20      0x02    //Memory must be located at 20 bit add
#define PCI_BASE_ADD_MT_64      0x04    //Memory must be located at 64 bit add
#define PCI_BASE_ADD_PREFETCH   0x08    //If set, memory is prefetchable
#define PCI_BASE_ADD_MEMMASK    0x0FFF0 //Bits in lower word that are memory mask
#define PCI_BASE_ADD_PORTMASK   0x0FFFC //Bits in lower word that are port mask
#define PCI_REG_ROM_BASE_ADD    0x30    //PCI expansion ROM base address register
#define PCI_BASE_ADD_ROMMASK    0x0FC00 //Bits in lower word that are ROM mask
#define PCI_REG_INT_LINE        0x3C    //PCI interrupt line register
#define PCI_REG_INT_PIN         0x3D    //PCI interrupt pin register
#define PCI_REG_MAX_GNT         0x3E    //PCI max grant register
#define PCI_REG_MAX_LAT         0x3F    //PCI max latency register

#define INTR_CNTRLR_MASTER_PORT     0x020
#define INTR_CNTRLR_SLAVE_PORT      0x0A0
#define READ_IN_SERVICE_REGISTER    0x00B

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        USB_BADDEV_STRUC
//
// Description: This structure is used to define a non-compliant USB device
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      wVID        WORD        Vendor ID of the device
//      wDID        WORD        Device ID of the device
//      bBaseClass  BYTE        Base class of the device
//      bSubClass   BYTE        Sub class of the device
//      bProtocol   BYTE        Protocol used by the device
//      wFlags      INCMPT_FLAGS    Incompatibility flags
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT16  wVID;
    UINT16  wDID;
    UINT8   bBaseClass;
    UINT8   bSubClass;
    UINT8   bProtocol;
    UINT16  wFlags;
} USB_BADDEV_STRUC;


//<AMI_GHDR_START>
//----------------------------------------------------------------------------
// Name:        INCMPT_FLAGS (Incompatibility attributes)
//
// Type:        Equates
//
// Description: This equates are used to describe the incompatible USB mass
//      storage devices. The bits defined are:
//      BIT     Description
//      =============================================================
//      0       1, indicates this device does not support mode sense command
//      1       1, indicates that this is a single LUN device (even though it is reporting as multiple LUN)
//      2       1, indicates that this device should be disabled
//      3       1, indicates this device does not support test unit ready command
//      4       1, indicates this device responds with wrong BOT status value
//      5       1, indicates that this device does not support start unit command
//      6       1, indicates that this device does not support read format capacity command
//      7       1, indicates that this hispeed device has to be in full speed always
//      8       1, indicates that this hispeed device has to be in hispeed always
//
// Notes:   The device is identified by the vendor id and device id
//      associated with the flags above
//
// Referrals:   USB_BADDEV_STRUC
//
//----------------------------------------------------------------------------
//<AMI_GHDR_END>

#define USB_INCMPT_MODE_SENSE_NOT_SUPPORTED         BIT0
#define USB_INCMPT_SINGLE_LUN_DEVICE                BIT1
#define USB_INCMPT_DISABLE_DEVICE                   BIT2
#define USB_INCMPT_TEST_UNIT_READY_FAILED           BIT3
#define USB_INCMPT_BOT_STATUS_FAILED                BIT4
#define USB_INCMPT_START_UNIT_NOT_SUPPORTED         BIT5
#define USB_INCMPT_FORMAT_CAPACITY_NOT_SUPPORTED    BIT6
#define USB_INCMPT_KEEP_FULLSPEED                   BIT7
#define USB_INCMPT_KEEP_HISPEED                     BIT8
#define USB_INCMPT_SET_BOOT_PROTOCOL_NOT_SUPPORTED  BIT9
#define USB_INCMPT_GETMAXLUN_NOT_SUPPORTED          BIT10
#define USB_INCMPT_RMH_DEVICE                       BIT11
#define USB_INCMPT_HID_DATA_OVERFLOW                BIT12
#define USB_INCMPT_BOOT_PROTOCOL_IGNORED            BIT13
#define USB_INCMPT_REPORT_PROTOCOL_ONLY             BIT14	//(EIP38434+)


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        HcDxeRecord
//
// Description: state information for USB_HC_PROTOCOL implementation
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct _HC_DXE_RECORD {
    EFI_USB_HC_PROTOCOL hcprotocol;
    EFI_USB2_HC_PROTOCOL hcprotocol2;
    HC_STRUC            *hc_data;
    EFI_PCI_IO_PROTOCOL *pciIo;
    DLIST               AsyncTransfers;
} HC_DXE_RECORD;

typedef struct {
    QUEUE_T         QCompleted;
    UINTN           DataLength; //size of each transfer
    EFI_ASYNC_USB_TRANSFER_CALLBACK  CallbackFunction;
    VOID*           Context;
    EFI_EVENT       Event;
    DLINK           Link;
    UINT8           Lock;
    UINT8           EndpointAddress;
    UINT8           Data[1];
} USBHC_INTERRUPT_DEVNINFO_T;

int VALID_DEVINFO(DEV_INFO* pDevInfo);
VOID    USB_AbortConnectDev(DEV_INFO* );

#endif      // __USB_H

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
