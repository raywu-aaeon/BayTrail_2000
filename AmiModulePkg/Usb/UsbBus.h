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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/usbbus.h 14    9/04/12 6:15a Ryanchou $
//
// $Revision: 14 $
//
// $Date: 9/04/12 6:15a $
//
//****************************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:           UsbBus.h
//
//  Description:    AMI USB bus driver header file
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _USBBUS_INC_
#define _USBBUS_INC_

#include "Tree.h"
#include "UsbDef.h"
#include <Protocol/UsbHc.h>
#include <Protocol/UsbIo.h>
#include <Protocol/DevicePath.h>


#define USB_MAXLANID          16
#define USB_MAXCHILDREN       8
#define USB_MAXCONTROLLERS    4

#pragma pack(push, 1)

typedef struct {
  UINT8  Length;
  UINT8  DescriptorType;
} USB_DESCRIPTOR_T;

typedef struct {
  UINT8  len;
  UINT8  desctype;
  UINT16 langID[1];
} lang_table_t;

typedef struct {
    UINT8   address;
    EFI_USB_ENDPOINT_DESCRIPTOR* desc;
} endpoint_t;

#pragma pack(pop)

enum node_type_enum { NodeHC, NodeDevice, NodeGroup };


typedef struct {
    int                                 type;
    EFI_HANDLE                          handle; // handle of the controller
    EFI_DEVICE_PATH_PROTOCOL            *dp;
    EFI_USB2_HC_PROTOCOL                *hc;     // USB_HC_ installed on controller
} usbbus_data_t;

typedef struct _USBBUS_HC_T {
    int                                 type;
    EFI_HANDLE                          handle; // handle of the controller
    EFI_DEVICE_PATH_PROTOCOL            *dp;
    EFI_USB2_HC_PROTOCOL                *hc;     // USB_HC_ installed on controller
    HC_STRUC                            *hc_data;
    TREENODE_T                          node;
} USBBUS_HC_T;

#define COMPRESS_EP_ADR(a)              ( a & 0xF )

typedef struct _DEVGROUP_T {
    int                                 type;
    EFI_HANDLE                          handle; // handle of the controller
    EFI_DEVICE_PATH_PROTOCOL            *dp;
    EFI_USB2_HC_PROTOCOL                *hc;    //  USB_HC_ that the controller is attached to
    DEV_INFO                            *dev_info;
    HC_STRUC                            *hc_info;
    lang_table_t                        *lang_table;
    EFI_USB_DEVICE_DESCRIPTOR           dev_desc;
    EFI_USB_CONFIG_DESCRIPTOR           **configs;
    endpoint_t                          endpoints[0x20];
    EFI_USB_ENDPOINT_DESCRIPTOR*        a2endpoint[0x20];
    int                                 endpoint_count;

    int                                 active_config; // index in configs
    int                                 config_count;
    int                                 f_DevDesc;
    TREENODE_T                          node;
} DEVGROUP_T;

typedef struct _USBDEV_T {
    int                                 type;
    EFI_HANDLE                          handle; // handle of the controller
    EFI_DEVICE_PATH_PROTOCOL            *dp;
    EFI_USB2_HC_PROTOCOL                *hc; //USB_HC_ that the controller is attached to
    DEV_INFO                            *dev_info;
    HC_STRUC                            *hc_info;
    //UINT8                             toggle; //toggle param for bulk transfer
    CHAR16*                             name;
    int                                 f_connected; //was ConnectControllers successful?
    int                                 first_endpoint;
    int                                 end_endpoint;
    EFI_USB_INTERFACE_DESCRIPTOR*       descIF;
    UINT8                               speed;
    EFI_USB_IO_PROTOCOL                 io;
    TREENODE_T                          node;
	int									async_endpoint;
} USBDEV_T;



EFI_STATUS UsbBusSupported (
  EFI_DRIVER_BINDING_PROTOCOL     *pThis,
  EFI_HANDLE                      controller,
  EFI_DEVICE_PATH_PROTOCOL        * );

EFI_STATUS UsbBusStart (
  EFI_DRIVER_BINDING_PROTOCOL     *pThis,
  EFI_HANDLE                      controller,
  EFI_DEVICE_PATH_PROTOCOL        * );

EFI_STATUS UsbBusStop (
  EFI_DRIVER_BINDING_PROTOCOL     *pThis,
  EFI_HANDLE                      controller,
  UINTN                           NumberOfChildren,
  EFI_HANDLE                      *ChildHandleBuffer );

EFI_STATUS UsbBusInit(EFI_HANDLE  ImageHandle,EFI_HANDLE  ServiceHandle);

USBDEV_T* UsbIo2Dev(EFI_USB_IO_PROTOCOL* p);

UINT8*
UsbSmiGetDescriptor(
    HC_STRUC* Hc,
    DEV_INFO* Dev,
    UINT8*    Buf,
    UINT16    Len,
    UINT8     DescType,
    UINT8     DescIndex
);

UINT16
UsbSmiControlTransfer (
    HC_STRUC*   HCStruc,
    DEV_INFO*   DevInfo,
    UINT16      Request,
    UINT16      Index,
    UINT16      Value,
    UINT8       *Buffer,
    UINT16      Length
);

UINT8
UsbResetAndReconfigDev(
    HC_STRUC*   HostController,
    DEV_INFO*   Device
);

UINT8
UsbDevDriverDisconnect(
    HC_STRUC*   HostController,
    DEV_INFO*   Device
);

#define GETBIT(bitarray,value,bit) \
    ((value) =  (UINT8)(((bitarray) & (1 << (bit)))>>(bit)))\

#define SETBIT(bitarray,value,bit) \
    (bitarray) =  (((bitarray) & ~(1 << (bit))) | (((value)&1) << (bit)) )\

#define IsSlow(dev) dev->speed
#define GetSpeed(dev) dev->speed

VOID InstallDevice(DEV_INFO* DevInfo);
int eUninstallDevice(VOID* Node, VOID* Context);
EFI_STATUS RemoveDevInfo(DEV_INFO* pDevInfo);


#endif //_USBBUS_INC_

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
