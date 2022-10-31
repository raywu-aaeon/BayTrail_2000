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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/protocol/AmiUsbController.h 29    9/03/12 5:23a Roberthsu $
//
// $Revision: 29 $
//
// $Date: 9/03/12 5:23a $
//
//****************************************************************************

//<AMI_FHDR_START>
//-----------------------------------------------------------------------------
//
//  Name:           AmiUsbController.h
//
//  Description:    AMI USB Driver Protocol definition
//
//-----------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _USB_PROT_H
#define _USB_PROT_H

#include <Efi.h>

#define EFI_USB_PROTOCOL_GUID    \
  { 0x2ad8e2d2, 0x2e91, 0x4cd1, 0x95, 0xf5, 0xe7, 0x8f, 0xe5, 0xeb, 0xe3, 0x16 }

#define AMI_USB_SMM_PROTOCOL_GUID    \
  { 0x3ef7500e, 0xcf55, 0x474f, 0x8e, 0x7e, 0x0, 0x9e, 0xe, 0xac, 0xec, 0xd2 }

GUID_VARIABLE_DECLARATION(gEfiUsbProtocolGuid,EFI_USB_PROTOCOL_GUID);

GUID_VARIABLE_DECLARATION(gAmiUsbSmmProtocolGuid,AMI_USB_SMM_PROTOCOL_GUID);

#ifndef GUID_VARIABLE_DEFINITION
#include <Protocol/BlockIo.h>

typedef struct {
	UINT8	NumUsbKbds;
	UINT8	NumUsbMice;
	UINT8	NumUsbPoint; 				//(EIP38434+)
	UINT8	NumUsbMass;
	UINT8	NumUsbHubs;
	UINT8	NumUsbCCIDs;
} CONNECTED_USB_DEVICES_NUM;

typedef VOID (EFIAPI *EFI_USB_REPORT_DEVICES ) (
  CONNECTED_USB_DEVICES_NUM	*);

typedef struct _EFI_USB_HOTPLUG_DEVS {
	BOOLEAN cdrom;
	BOOLEAN	floppy;
} EFI_USB_HOTPLUG_DEVS;

typedef EFI_STATUS (EFIAPI *EFI_USB_GET_HOTPLUG_DEVS ) (
  EFI_USB_HOTPLUG_DEVS *);

typedef EFI_STATUS (EFIAPI *EFI_USB_GET_RUNTIME_REGION ) (
  EFI_PHYSICAL_ADDRESS *,
  EFI_PHYSICAL_ADDRESS *);

typedef UINT8 (EFIAPI *EFI_USB_GET_NEXT_MASS_DEVICE_NAME ) (
  UINT8*, UINT8, UINT8);

typedef struct
{
    EFI_BLOCK_IO_PROTOCOL   BlockIoProtocol;
    EFI_BLOCK_IO_MEDIA      *Media;
    VOID                    *DevInfo;
    UINT16                  LogicalAddress;
    EFI_HANDLE              Handle;
    UINT16                  PciBDF;
    UINT8                   *DevString;
    UINT8                   StorageType;
} USB_MASS_DEV;

//(EIP51653+)>
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        USB_SKIP_LIST
//
//
// Description:    If your roothub port 4 insert a hub.You want to skip hub's port 2.
//                 Set bRootPort = 4,  dRoutePath =2
//				   If your roothub port 4 insert a hub1.And hub1 port 2 insert a hub2. 
//				   You want to skip hub2's port 1. 
//                 Set bRootPort = 4,  dRoutePath =21 
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bSkipType   BYTE        Skip by which Type
//      bSkipAll   	BYTE        If this flag is 1 than skip all ports. 
//      wBDF        WORD        Bus Dev Function
//		bRootPort	BYTE		Root port path
//      dRoutePath 	DWORD       Hub route path. See description.
//      bBaseClass 	BYTE        Device Type
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
#define SKIP_FOR_ALLCONTROLLER              	0x1		//(EIP62695)
                                        //(EIP88776+)>
#define SKIP_FLAG_SKIP_PORT              	    0x0		 
#define SKIP_FLAG_KEEP_PORT              	    0x1		 
#define SKIP_FLAG_SKIP_LEVEL         	        0x2
                                        //<(EIP88776+)
typedef struct _USB_SKIP_LIST{
	UINT8 	bSkipType;
    UINT8  	bFlag; 					    //(EIP88776)
    UINT16  wBDF;
	UINT8	bRootPort;
    UINT32  dRoutePath;
    UINT8   bBaseClass;
} USB_SKIP_LIST;
//<(EIP51653+) 

//----------------------------------------------------------------------------
//      USB Mass Storage Related Data Structures and Equates
//----------------------------------------------------------------------------
#define USB_EMU_NONE            0
#define USB_EMU_FLOPPY_ONLY     1
#define USB_EMU_HDD_ONLY        2
#define USB_EMU_HDD_OR_FDD      3
#define USB_EMU_FORCED_FDD      4

#define BAID_TYPE_HDD           1
#define BAID_TYPE_RMD_HDD       2
#define BAID_TYPE_CDROM	        3
#define BAID_TYPE_RMD_FDD       4
#define BAID_TYPE_FDD           5

// Values for Mass Storage Device type
//-------------------------------------
#define USB_MASS_DEV_UNKNOWN    0
#define USB_MASS_DEV_HDD        1
#define USB_MASS_DEV_CDROM      2
#define USB_MASS_DEV_ARMD       3
#define USB_MASS_DEV_FDD        4
#define USB_MASS_DEV_MO         5


#define STOP_USB_CONTROLLER     0				//(EIP43475+)
#define START_USB_CONTROLLER    1				//(EIP43475+)

typedef VOID (EFIAPI *EFI_USB_CHANGE_EFI_TO_LEGACY) (UINT8);
//typedef EFI_STATUS (EFIAPI *EFI_USB_BBS_REMOVE_MASSSTORAGE) ();

typedef EFI_STATUS (EFIAPI *EFI_INSTALL_USB_LEGACY_BOOT_DEVICES)(VOID);
typedef EFI_STATUS (EFIAPI *EFI_USB_INSTALL_LEGACY_DEVICE)(USB_MASS_DEV*);
typedef EFI_STATUS (EFIAPI *EFI_USB_UNINSTALL_LEGACY_DEVICE)(USB_MASS_DEV*);
typedef EFI_STATUS (EFIAPI *EFI_GET_ASSIGN_USB_BOOT_PORT)(UINT8*, UINT8*);
typedef VOID (EFIAPI *EFI_KBC_ACCESS_CONTROL)(UINT8);
typedef EFI_STATUS (EFIAPI *EFI_USB_RT_LEGACY_CONTROL)(VOID *);
typedef VOID (EFIAPI *EFI_USB_STOP_UNSUPPORTED_HC)();
typedef VOID (EFIAPI *EFI_USB_SHUTDOWN_LEGACY)(); 			//<(EIP52339+)
typedef VOID (EFIAPI *EFI_USB_COPY_SKIP_TABLE)(USB_SKIP_LIST*, UINT8);			//(EIP51653+)	
typedef VOID (EFIAPI *EFI_USB_RT_STOP_CONTROLLER)(UINT16);		    //(EIP74876+)
typedef VOID (EFIAPI *EFI_USB_INVOKE_API)(VOID*);
typedef struct _EFI_USB_PROTOCOL {
	UINT32							Signature;				//(EIP55275+)
	VOID        					*USBDataPtr;
//    VOID                            *UsbBadDeviceTable;		//(EIP60706-)
	EFI_USB_REPORT_DEVICES			UsbReportDevices;
	EFI_USB_GET_NEXT_MASS_DEVICE_NAME	UsbGetNextMassDeviceName;
    EFI_USB_CHANGE_EFI_TO_LEGACY    UsbChangeEfiToLegacy;
//    EFI_USB_BBS_REMOVE_MASSSTORAGE  UsbBbsRemoveMassStorage;
    EFI_USB_GET_RUNTIME_REGION      UsbGetRuntimeRegion;
    EFI_INSTALL_USB_LEGACY_BOOT_DEVICES InstallUsbLegacyBootDevices;
    EFI_USB_INSTALL_LEGACY_DEVICE   UsbInstallLegacyDevice;
    EFI_USB_UNINSTALL_LEGACY_DEVICE UsbUninstallLegacyDevice;
    EFI_GET_ASSIGN_USB_BOOT_PORT    UsbGetAssignBootPort;
    EFI_KBC_ACCESS_CONTROL          UsbRtKbcAccessControl;
    EFI_USB_RT_LEGACY_CONTROL       UsbLegacyControl;
	EFI_USB_STOP_UNSUPPORTED_HC		UsbStopUnsupportedHc;
    EFI_USB_SHUTDOWN_LEGACY       UsbRtShutDownLegacy;      //EIP52339+
    EFI_USB_COPY_SKIP_TABLE       	UsbCopySkipTable;			//(EIP51653+)	
    EFI_USB_RT_STOP_CONTROLLER      UsbRtStopController;	        //(EIP74876+)
    EFI_USB_INVOKE_API				UsbInvokeApi;
} EFI_USB_PROTOCOL;

typedef struct {
	EFI_USB_STOP_UNSUPPORTED_HC		UsbStopUnsupportedHc;
} AMI_USB_SMM_PROTOCOL;

#endif
#endif // _USB_PROT_H

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
