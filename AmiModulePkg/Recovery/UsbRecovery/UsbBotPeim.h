//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
//
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        UsbBotPeim.h
//
// Description: This file belongs to "Framework".
//              This file is modified by AMI to include copyright message,
//              appropriate header and integration code.
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


//
// This file contains a 'Sample Driver' and is licensed as such
// under the terms of your license agreement with Intel or your
// vendor.  This file may be modified by the user, subject to
// the additional terms of the license agreement
//

/*++

   Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
   This software and associated documentation (if any) is furnished
   under a license and may only be used or copied in accordance
   with the terms of the license. Except as permitted by such
   license, no part of this software or documentation may be
   reproduced, stored in a retrieval system, or transmitted in any
   form or by any means without the express written consent of
   Intel Corporation.


   Module Name:

   UsbBotPeim.h

   Abstract:

   Usb BOT Peim definition

   --*/

#ifndef _PEI_USB_BOT_PEIM_H
#define _PEI_USB_BOT_PEIM_H

#include "Efi.h"
#include "Pei.h"
// #include "PeiLib.h"
#include "usb.h"
#include "atapi.h"
#include "AmiMapping.h"
#include <Library/BaseMemoryLib.h>

//
// Driver consumed PPI Prototypes
//
#include "UsbIo.h"

//
// Driver produces PPI Prototypes
//
#include "Include/Ppi/DeviceRecoveryBlockIo.h"
#include <Token.h>

#define PEI_FAT_MAX_USB_IO_PPI  127

//---------------------------------------------------------------------------
//      Values for InterfaceDescriptor.BaseClass
//---------------------------------------------------------------------------
#define BASE_CLASS_HID           0x03
#define BASE_CLASS_MASS_STORAGE  0x08
#define BASE_CLASS_HUB           0x09
//----------------------------------------------------------------------------

//---------------------------------------------------------------------------
//      Values for InterfaceDescriptor.Protocol
//---------------------------------------------------------------------------
#define PROTOCOL_KEYBOARD  0x01         // Keyboard device protocol
#define PROTOCOL_MOUSE     0x02         // Mouse device protocol?

// Mass storage related protocol equates
#define PROTOCOL_CBI         0x00       // Mass Storage Control/Bulk/Interrupt
                                        // with command completion interrupt
#define PROTOCOL_CBI_NO_INT  0x01       // MASS STORAGE Control/Bulk/Interrupt
                                        // with NO command completion interrupt
#define PROTOCOL_BOT         0x50       // Mass Storage Bulk-Only Transport
#define PROTOCOL_VENDOR      0xff       // Vendor specific mass protocol
//---------------------------------------------------------------------------

//
// BlockIo PPI prototype
//
EFI_STATUS
BotGetNumberOfBlockDevices (
    IN EFI_PEI_SERVICES              **PeiServices,
    IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *This,
    OUT UINTN                        *NumberBlockDevices );

EFI_STATUS
BotGetMediaInfo (
    IN EFI_PEI_SERVICES              **PeiServices,
    IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *This,
    IN UINTN                         DeviceIndex,
    OUT EFI_PEI_BLOCK_IO_MEDIA       *MediaInfo );

EFI_STATUS
BotReadBlocks (
    IN EFI_PEI_SERVICES              **PeiServices,
    IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *This,
    IN UINTN                         DeviceIndex,
    IN EFI_PEI_LBA                   StartLBA,
    IN UINTN                         BufferSize,
    OUT VOID                         *Buffer );

//
// UsbIo PPI Notification
//
EFI_STATUS
NotifyOnUsbIoPpi (
    IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *InvokePpi );

EFI_STATUS
NotifyOnRecoveryCapsuleLoaded (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDesc,
    IN VOID                         *InvokePpi);

EFI_STATUS
InitUsbBot (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_USB_IO_PPI   *UsbIoPpi );

#define USBFLOPPY   1
#define USBFLOPPY2  2   // for those that use ReadCapacity(0x25)
                        // command to retrieve media capacity
#define USBCDROM    3
//
// Bot device structure
//
#define PEI_BOT_DEVICE_SIGNATURE  EFI_SIGNATURE_32( 'U', 'B', 'O', 'T' )
typedef struct
{
    UINTN Signature;
    EFI_PEI_RECOVERY_BLOCK_IO_PPI BlkIoPpi;
    EFI_PEI_PPI_DESCRIPTOR BlkIoPpiList;
    EFI_PEI_BLOCK_IO_MEDIA Media;
    PEI_USB_IO_PPI *UsbIoPpi;
    EFI_USB_INTERFACE_DESCRIPTOR  *BotInterface;
    EFI_USB_ENDPOINT_DESCRIPTOR   *BulkInEndpoint;
    EFI_USB_ENDPOINT_DESCRIPTOR   *BulkOutEndpoint;
    UINTN  DeviceType;
    REQUEST_SENSE_DATA *SensePtr;
    UINT32 FdEmulOffset;
    UINT8  Lun;
} PEI_BOT_DEVICE;

#define PEI_BOT_DEVICE_FROM_THIS( a ) \
    PEI_CR( a, PEI_BOT_DEVICE, BlkIoPpi, PEI_BOT_DEVICE_SIGNATURE )

//
// USB ATATPI command
//
EFI_STATUS
PeiAtapiCommand (
    IN EFI_PEI_SERVICES       **PeiServices,
    IN PEI_BOT_DEVICE         *PeiBotDev,
    IN VOID                   *Command,
    IN UINT8                  CommandSize,
    IN VOID                   *DataBuffer,
    IN OUT UINT32             *BufferLength,
    IN EFI_USB_DATA_DIRECTION Direction,
    IN UINT16                 TimeOutInMilliSeconds );


//extern VOID ZeroMem (
//    IN VOID  *Buffer,
//    IN UINTN Size );

#define ONE_SECOND_DELAY  1000000 // 1 second = 1000000 microseconds

// Controller-specific externs
#if (PEI_UHCI_SUPPORT == 1)
EFI_STATUS UhciPeiUsbEntryPoint (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices );

extern EFI_STATUS UhcPeimEntry (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices );
#endif

#if (PEI_OHCI_SUPPORT == 1)
EFI_STATUS OhciPeiUsbEntryPoint (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices );
#endif

#if (PEI_EHCI_SUPPORT == 1)
EFI_STATUS EhciPeiUsbEntryPoint (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices );
#endif

#if (PEI_XHCI_SUPPORT == 1)
EFI_STATUS XhciPeiUsbEntryPoint (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices );
#endif

extern EFI_STATUS PeimInitializeUsb (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices );

#define PEI_MAX_USB_RECOVERY_INIT_PPI 16

// The following section should eventually go to SBPPI.H.
#ifndef PEI_USB_CHIP_INIT_PPI_GUID
#define PEI_USB_CHIP_INIT_PPI_GUID \
    { 0xdb75358d, 0xfef0, 0x4471, 0xa8, 0xd, 0x2e, 0xeb, 0x13, 0x8, 0x2d, 0x2d }

typedef EFI_STATUS (EFIAPI *PEI_ENABLE_CHIP_USB_RECOVERY) (
  IN EFI_PEI_SERVICES               **PeiServices
  );

typedef struct _PEI_USB_CHIP_INIT_PPI       PEI_USB_CHIP_INIT_PPI;

typedef struct _PEI_USB_CHIP_INIT_PPI {
    PEI_ENABLE_CHIP_USB_RECOVERY  EnableChipUsbRecovery;           
} PEI_USB_CHIP_INIT_PPI;
#endif

#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
