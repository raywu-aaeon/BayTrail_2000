//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
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
// Name:        UsbIoPeim.C
//
// Description: This file belongs to "Framework".
//              This file is modified by AMI to include copyright message,
//              appropriate header and integration code.
//              This file contains generic routines needed for USB recovery
//              PEIM
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

/*++
   This file contains 'Framework Code' and is licensed as such
   under the terms of your license agreement with Intel or your
   vendor.  This file may not be modified, except as allowed by
   additional terms of your license agreement.
   --*/

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

   UsbIoPeim.c

   Abstract:

   Usb Io PPI

   --*/

#include "UsbPeim.h"
#include "PeiUsbLib.h"

#define PAGESIZE  4096

VOID
ResetRootPort (
    IN EFI_PEI_SERVICES            **PeiServices,
    IN PEI_USB_HOST_CONTROLLER_PPI *UsbHcPpi,
    UINT8                          PortNum );

EFI_STATUS PeiUsbControlTransfer (
    IN EFI_PEI_SERVICES       **PeiServices,
    IN PEI_USB_IO_PPI         *This,
    IN EFI_USB_DEVICE_REQUEST *Request,
    IN EFI_USB_DATA_DIRECTION Direction,
    IN UINT32                 Timeout,
    IN OUT VOID *Data         OPTIONAL,
    IN UINTN DataLength       OPTIONAL )
{
    EFI_STATUS     Status;
    PEI_USB_DEVICE *PeiUsbDev;
    PEI_USB_HOST_CONTROLLER_PPI *UsbHcPpi;
    UINT32         TransferResult;

    PeiUsbDev = PEI_USB_DEVICE_FROM_THIS( This );
    UsbHcPpi = PeiUsbDev->UsbHcPpi;

    Status = UsbHcPpi->ControlTransfer(
        PeiServices,
        UsbHcPpi,
        PeiUsbDev->DeviceAddress,
        PeiUsbDev->DeviceSpeed,
        PeiUsbDev->MaxPacketSize0,
        PeiUsbDev->TransactionTranslator,
        Request,
        Direction,
        Data,
        &DataLength,
        Timeout,
        &TransferResult
             );

    return Status;
}


EFI_STATUS PeiUsbBulkTransfer (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_USB_IO_PPI   *This,
    IN UINT8            DeviceEndpoint,
    IN OUT VOID         *Data,
    IN OUT UINTN        *DataLength,
    IN UINTN            Timeout )
{
    EFI_STATUS     Status;
    PEI_USB_DEVICE *PeiUsbDev;
    PEI_USB_HOST_CONTROLLER_PPI *UsbHcPpi;
    UINT32         TransferResult;
    UINT16         MaxPacketLength;
    UINT8 DataToggle;
    UINT8 OldToggle;
    EFI_USB_ENDPOINT_DESCRIPTOR *EndpointDescriptor;
    UINT8 EndpointIndex;

    PeiUsbDev = PEI_USB_DEVICE_FROM_THIS( This );
    UsbHcPpi = PeiUsbDev->UsbHcPpi;

    EndpointIndex = 0;

    while (EndpointIndex < MAX_ENDPOINT) {
        Status = PeiUsbGetEndpointDescriptor( PeiServices,
            This, EndpointIndex, &EndpointDescriptor );
        if ( EFI_ERROR( Status ) ) {
            return EFI_INVALID_PARAMETER;
        }

        if (EndpointDescriptor->EndpointAddress == DeviceEndpoint) {
            break;
        }

        EndpointIndex++;
    }

    if (EndpointIndex == MAX_ENDPOINT) {
        return EFI_INVALID_PARAMETER;
    }

    MaxPacketLength = (PeiUsbDev->EndpointDesc[EndpointIndex]->MaxPacketSize);
    if ( ( PeiUsbDev->DataToggle & (1 << EndpointIndex) ) != 0 ) {
        DataToggle = 1;
    }
    else {
        DataToggle = 0;
    }

    OldToggle = DataToggle;

    Status = UsbHcPpi->BulkTransfer(
        PeiServices,
        UsbHcPpi,
        PeiUsbDev->DeviceAddress,
        DeviceEndpoint,
        PeiUsbDev->DeviceSpeed,
        MaxPacketLength,
        PeiUsbDev->TransactionTranslator,
        Data,
        DataLength,
        &DataToggle,
        Timeout,
        &TransferResult
             );

    if (OldToggle != DataToggle) {
        PeiUsbDev->DataToggle =
            (UINT8) ( PeiUsbDev->DataToggle ^ (1 << EndpointIndex) );
    }

    return Status;
}


EFI_STATUS PeiUsbGetInterfaceDescriptor (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN PEI_USB_IO_PPI               *This,
    IN EFI_USB_INTERFACE_DESCRIPTOR **InterfaceDescriptor )
{
    PEI_USB_DEVICE *PeiUsbDev;

    PeiUsbDev = PEI_USB_DEVICE_FROM_THIS( This );

    *InterfaceDescriptor = PeiUsbDev->InterfaceDesc;

    return EFI_SUCCESS;
}


EFI_STATUS PeiUsbGetEndpointDescriptor (
    IN EFI_PEI_SERVICES            **PeiServices,
    IN PEI_USB_IO_PPI              *This,
    IN UINT8                       EndpointIndex,
    IN EFI_USB_ENDPOINT_DESCRIPTOR **EndpointDescriptor )
{
    PEI_USB_DEVICE *PeiUsbDev;

    PeiUsbDev = PEI_USB_DEVICE_FROM_THIS( This );

    if (EndpointIndex >= PeiUsbDev->InterfaceDesc->NumEndpoints) {
        return EFI_INVALID_PARAMETER;
    }

    *EndpointDescriptor = PeiUsbDev->EndpointDesc[EndpointIndex];

    return EFI_SUCCESS;
}


EFI_STATUS PeiUsbPortReset (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_USB_IO_PPI   *This )
{
    PEI_USB_DEVICE *PeiUsbDev;
    EFI_STATUS     Status;
    UINT8 Address;

    PeiUsbDev = PEI_USB_DEVICE_FROM_THIS( This );

    ResetRootPort(
        PeiServices,
        PeiUsbDev->UsbHcPpi,
        PeiUsbDev->DeviceAddress
    );

    //
    // Set address
    //
    Address = PeiUsbDev->DeviceAddress;
    PeiUsbDev->DeviceAddress = 0;

    Status = PeiUsbSetDeviceAddress(
        PeiServices,
        This,
        Address
             );

    if ( EFI_ERROR( Status ) ) {
        return Status;
    }

    PeiUsbDev->DeviceAddress = Address;

    //
    // Set default configuration
    //
    Status = PeiUsbSetConfiguration(
        PeiServices,
        This
             );

    return Status;
}


//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
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
