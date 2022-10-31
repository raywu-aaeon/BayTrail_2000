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
// $Header: /Alaska/SOURCE/Modules/USB/ALASKA/Int13/UsbInt13.h 9     1/06/12 1:01a Rajeshms $
//
// $Revision: 9 $
//
// $Date: 1/06/12 1:01a $
//
//****************************************************************************

#ifndef __UI13_HEADER__
#define __UI13_HEADER__

#include <Efi.h>

#pragma pack(1)

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
//  Name:           UINT13.H
//  Description:    Definitions and structures for USB INT13
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

VOID        AmiUsbNotifyCallback (EFI_EVENT, VOID*);
VOID        ReadyToBootNotify(EFI_EVENT, VOID*);
EFI_STATUS  InstallUsbLegacyBootDevices (VOID);
EFI_STATUS  UsbInstallLegacyDevice (USB_MASS_DEV*);
EFI_STATUS  UsbUninstallLegacyDevice (USB_MASS_DEV*);

#define     USBDEVS_MAX_ENTRIES 16

typedef struct _USBMASS_INT13_DEV {
    UINT8   Handle;
    UINT8   BbsEntryNo;
    UINT8   DevBaidType;
    UINT8   NumHeads;
    UINT8   LBANumHeads;
    UINT16  NumCylinders;
    UINT16  LBANumCyls;
    UINT8   NumSectors;
    UINT8   LBANumSectors;
    UINT16  BytesPerSector;
    UINT8   MediaType;
    UINT32  LastLBA;
    UINT8   BpbMediaDesc;
    UINT8   DeviceNameString[64];
} USBMASS_INT13_DEV;

//
// The following data structure is located in UI13.BIN
//
typedef struct _UINT13_DATA {
    USBMASS_INT13_DEV   UsbMassI13Dev[USBDEVS_MAX_ENTRIES];
    UINT8               MfgGenericName[12];    // "USB Storage", 0
    UINT16              BcvOffset;
    UINT16              CheckForUsbCDROMOffset;
    UINT16              UsbSmmDataOffset;
    UINT16              UsbPciLocationTableOffset;
} UINT13_DATA;

typedef struct _USB_PCI_LOCATION {
    UINT8   Handle;         // USB device address
    UINT16  PciLocation;    // Bus[15..8] Device[7..3] Function[2..0]
} USB_PCI_LOCATION;

#pragma pack()

typedef enum {
    Floppy,
    HardDrive,
    CDROM
} HOTPLUG_DEVICE;

EFI_STATUS InitializeHotplugDevices();

#endif

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
