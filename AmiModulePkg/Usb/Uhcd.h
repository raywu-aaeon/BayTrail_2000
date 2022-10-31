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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/uhcd.h 41    8/29/12 8:40a Ryanchou $
//
// $Revision: 41 $
//
// $Date: 8/29/12 8:40a $
//
//****************************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:           Uhcd.h
//
//  Description:    AMI USB Host Controller Driver header file
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _AMIUSB_H
#define _AMIUSB_H

#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/PciIo.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/ComponentName.h>
#include <Protocol/AmiUsbController.h>
#include <Protocol/AmiUsbCcid.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/UsbIo.h>
#include "AmiDef.h"
#include "UsbDef.h"


/* PCI Configuration Registers for USB */
// Class Code Register offset
#define CLASSC                      0x09
// USB IO Space Base Address Register offset
#define USBBASE                     0x20

//
// USB Base Class Code,Sub-Class Code and Programming Interface.
//
#define PCI_CLASSC_BASE_CLASS_SERIAL        0x0c
#define PCI_CLASSC_SUBCLASS_SERIAL_USB      0x03
#define PCI_CLASSC_IFT_USB_DEVICE			0xFE    // Interface type for USB Device
#define PCI_CLASSC_PI_UHCI                  0x00
#define PCI_CLASSC_PI_OHCI                  0x10
#define PCI_CLASSC_PI_EHCI                  0x20
#define PCI_CLASSC_PI_XHCI                  0x30

#pragma pack(push, 1)

//
// USB Class Code structure
//
typedef struct
{
  UINT8    PI;
  UINT8    SubClassCode;
  UINT8    BaseCode;
} USB_CLASSC;

#pragma pack(pop)

UINT8 UsbSmiReConfigDevice(HC_STRUC* hc, DEV_INFO* dev);

#define DEBUG_LEVEL_USBBUS          DEBUG_LEVEL_3
#define DEBUG_USBHC_LEVEL           DEBUG_LEVEL_3
#define DEBUG_USBHC_LEVEL8          DEBUG_LEVEL_8

EFI_STATUS usbhc_init(EFI_HANDLE  ImageHandle,EFI_HANDLE  ServiceHandle);

#define ATOMIC(a) {\
        EFI_TPL savetpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);\
        {a;}\
        gBS->RestoreTPL(savetpl);   \
}\

#define CRITICAL_CODE(level, a) {\
        EFI_TPL savetpl;\
        EFI_TPL currenttpl = gBS->RaiseTPL (TPL_HIGH_LEVEL); \
        gBS->RestoreTPL(currenttpl);\
        if(currenttpl<=level)\
            currenttpl=level;\
        savetpl = gBS->RaiseTPL (currenttpl);\
        {a;}\
        gBS->RestoreTPL(savetpl);   \
}\

//#define CRITICAL_CODE(level, a) {\
//      EFI_TPL savetpl = gBS->RaiseTPL (level);\
//      {a;}\
//      gBS->RestoreTPL(savetpl);   \
//}\

extern EFI_GUID gEfiDriverBindingProtocolGuid;
extern EFI_GUID gEfiComponentName2ProtocolGuid; //(EIP59272)

EFI_STATUS  UsbMsInit(EFI_HANDLE  ImageHandle, EFI_HANDLE  ServiceHandle);
EFI_STATUS  UsbMassInit(EFI_HANDLE ImageHandle, EFI_HANDLE  ServiceHandle);
EFI_STATUS  UsbCCIDInit(EFI_HANDLE ImageHandle, EFI_HANDLE  ServiceHandle);
HC_STRUC*   FindHcStruc(EFI_HANDLE Controller);

EFI_STATUS
AmiUsbDriverEntryPoint(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable );

EFI_STATUS
AmiUsbDriverBindingSupported (
    IN EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN EFI_HANDLE                   Controller,
    IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath );

EFI_STATUS
AmiUsbDriverBindingStart (
    IN EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN EFI_HANDLE                   Controller,
    IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath );

EFI_STATUS
AmiUsbDriverBindingStop (
    IN EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN EFI_HANDLE                   Controller,
    IN UINTN                        NumberOfChildren,
    IN EFI_HANDLE                   *ChildHandleBuffer );

HC_STRUC*
AddHC (
    EFI_HANDLE					Controller,
    UINTN						PciBus,
    UINTN						PciDev,
    UINTN						PciFunc,
    UINT8       				HcType,
	UINT8       				Irq,
	EFI_DEVICE_PATH_PROTOCOL    *DevicePatch,
	EFI_PCI_IO_PROTOCOL   		*PciIo
);

EFI_STATUS
LocateEhciController(
	IN EFI_DRIVER_BINDING_PROTOCOL	*This,
	IN EFI_HANDLE					Controller,
	IN EFI_DEVICE_PATH_PROTOCOL 	*CompanionDevicePath
);

VOID
OnLegacyBoot (
    EFI_EVENT   Event,
    VOID        *Context );

EFI_STATUS
GetRuntimeRegion (
    EFI_PHYSICAL_ADDRESS *Start,
    EFI_PHYSICAL_ADDRESS *End
);

EFI_STATUS  Dummy1(USB_MASS_DEV*   Device);
EFI_STATUS  Dummy2(VOID);

VOID        ReportDevices(CONNECTED_USB_DEVICES_NUM*);
UINT8       GetNextMassDeviceName(UINT8*, UINT8, UINT8);
VOID        UsbChangeEfiToLegacy (UINT8);
VOID*       AllocAlignedMemory(UINT32, UINT16);
VOID        USBGenerateSWSMI (UINT8);
VOID        UsbPrepareForLegacyOS();
EFI_STATUS  UpdateHcPciInfo();
EFI_STATUS  OemGetAssignUsbBootPort (UINT8*, UINT8*);
VOID  		UsbGetSkipList(USB_SKIP_LIST*, UINT8);	//(EIP51653+) 
VOID		FreeMemory(UINT32);
VOID		InvokeUsbApi(URP_STRUC*);


// USB HC binding protocol functions
EFI_STATUS
UsbHcSupported (
    EFI_DRIVER_BINDING_PROTOCOL *pThis,
    EFI_HANDLE  Controller,
    EFI_DEVICE_PATH_PROTOCOL *DevicePath
);

EFI_STATUS
InstallHcProtocols(
    IN EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN EFI_HANDLE                   Controller,
    IN EFI_PCI_IO_PROTOCOL   		*PciIo,
    IN HC_STRUC              		*HcData
);

EFI_STATUS
UsbHcStop (
   EFI_DRIVER_BINDING_PROTOCOL      *This,
   EFI_HANDLE                       Controller,
   UINTN                            NumberOfChildren,
   EFI_HANDLE                       *Children 
);


EFI_STATUS EFIAPI
AmiUsb2HcSyncInterruptTransfer(
  IN EFI_USB2_HC_PROTOCOL    *hc_protocol,
  IN     UINT8                      deviceaddress,
  IN     UINT8                      endpointaddress,
  IN     UINT8                      DeviceSpeed,
  IN     UINTN                      maximumpacketlength,
  IN OUT VOID                       *data,
  IN OUT UINTN                      *datalength,
  IN OUT UINT8                      *datatoggle,
  IN     UINTN                      timeout,
  IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
  OUT    UINT32                     *transferresult  );

EFI_STATUS EFIAPI AmiUsb2HcTransfer(
  IN EFI_USB2_HC_PROTOCOL           *hc_protocol,
  IN     UINT8                      deviceaddress,
  IN     UINT8                      DeviceSpeed,
  IN     UINTN                      maximumpacketlength,
  IN     EFI_USB_DEVICE_REQUEST     *request,
  IN     EFI_USB_DATA_DIRECTION     transferdirection,
  IN OUT VOID                       *data ,
  IN OUT UINTN                      *datalength,
  IN     UINTN                      timeout,
  IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
  OUT    UINT32                     *transferresult );

EFI_STATUS EFIAPI AmiUsb2HcBulkTransfer(
  IN EFI_USB2_HC_PROTOCOL   *hc_protocol,
  IN  UINT8                 deviceaddress,
  IN  UINT8                 endpointaddress,
  IN     UINT8              DeviceSpeed,
  IN  UINTN                 maximumpacketlength,
  IN     UINT8              DataBuffersNumber,
  IN OUT VOID               *Data[EFI_USB_MAX_BULK_BUFFER_NUM],
  IN OUT UINTN              *datalength,
  IN OUT UINT8              *datatoggle,
  IN  UINTN                 timeout,
  IN EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
  OUT UINT32                *transferresult );

EFI_STATUS EFIAPI AmiUsb2HcAsyncInterruptTransfer(
    IN EFI_USB2_HC_PROTOCOL                             *hc_protocol,
    IN UINT8                            deviceaddress,
    IN UINT8                            endpointaddress,
    IN UINT8                             DeviceSpeed,
    IN UINTN                            maxpacket,
    IN BOOLEAN                          isnewtransfer,
    IN OUT UINT8                        *datatoggle,
    IN UINTN                            pollinginterval  ,
    IN UINTN                            datalength,
    IN     EFI_USB2_HC_TRANSACTION_TRANSLATOR *Translator,
    IN EFI_ASYNC_USB_TRANSFER_CALLBACK  callbackfunction ,
    IN VOID                             *context);

EFI_STATUS
AmiUsbBlkIoReset (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  BOOLEAN                ExtendedVerification
);

EFI_STATUS
AmiUsbBlkIoFlushBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This
);

EFI_STATUS
AmiUsbBlkIoReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                LBA,
  IN  UINTN                  BufferSize,
  OUT VOID                   *Buffer
);

EFI_STATUS
AmiUsbBlkIoWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                LBA,
  IN UINTN                  BufferSize,
  IN VOID                   *Buffer
);

VOID
ReportDevices(
    IN OUT CONNECTED_USB_DEVICES_NUM    *devs
);

EFI_STATUS UpdateMassDevicesForSetup();

UINT32 CalculateMemorySize(VOID);

VOID OnExitBootServices(EFI_EVENT, VOID*);
EFI_STATUS InitUsbSetupVars (USB_GLOBAL_DATA*, EFI_BOOT_SERVICES*, EFI_RUNTIME_SERVICES*);
UINT8 UsbSetupGetLegacySupport();

typedef EFI_STATUS (*USB_HC_PREINIT_FUNC) (
  IN EFI_HANDLE Handle,
    HC_STRUC    *HcStruc
);

typedef EFI_STATUS (*USB_HC_POSTSTOP_FUNC) (
  IN EFI_HANDLE Handle,
    HC_STRUC    *HcStruc
);

typedef struct {
    UINT64  PCICommand;
    UINT16  FrameListSize;
    UINT16  FrameListAlignment;
    USB_HC_PREINIT_FUNC HcPreInit;
	USB_HC_POSTSTOP_FUNC HcPostStop;
} HCSPECIFICINFO;

enum {
    opHC_Start,
    opHC_Stop,
    opHC_EnumeratePorts,
    opHC_DisableInterrupts,
    opHC_EnableInterrupts,
    opHC_ProcessInterrupt,
    opHC_GetRootHubStatus,
    opHC_DisableRootHub,
    opHC_EnableRootHub,
    opHC_ControlTransfer,
    opHC_BulkTransfer,
    opHC_InterruptTransfer,
    opHC_DeactivatePolling,
    opHC_ActivatePolling,
    opHC_DisableKeyRepeat,
    opHC_EnableKeyRepeat,
};

EFI_STATUS  DummyHcFunc(EFI_HANDLE, HC_STRUC*);
EFI_STATUS  PreInitXhci(EFI_HANDLE, HC_STRUC*);
EFI_STATUS  PostStopXhci(EFI_HANDLE, HC_STRUC*);
EFI_STATUS  Usb3OemGetMaxDeviceSlots(UINT8*);
VOID        *AllocateHcMemory (IN EFI_PCI_IO_PROTOCOL*, UINTN, UINTN);
VOID		FreeHcMemory(IN EFI_PCI_IO_PROTOCOL*, IN UINTN, IN VOID*);
VOID        *ReallocateMemory(UINTN, UINTN, VOID*);
VOID        UsbSmiPeriodicEvent(VOID);
VOID        UhcdPciIoNotifyCallback (EFI_EVENT, VOID*);
VOID        UhcdPciIrqPgmNotifyCallback (EFI_EVENT, VOID*);
UINTN       UsbSmiHc(UINT8,UINT8, ...);
EFI_STATUS  USBPort_InstallEventHandler(HC_STRUC*);
VOID        Emul6064NotifyCallback(EFI_EVENT, VOID*);
EFI_STATUS  USBPort_XhciCapabilityOverride(    USB3_HOST_CONTROLLER *Usb3Hc);
VOID    UsbRtShutDownLegacy(VOID); 					//<(EIP52339+)
VOID        UsbRtStopController(UINT16);	//(EIP74876+)
#endif

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
