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
// Name:        UsbbotPeim.C
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
   This file contains a 'Sample Driver' and is licensed as such
   under the terms of your license agreement with Intel or your
   vendor.  This file may be modified by the user, subject to
   the additional terms of the license agreement
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

   UsbBotPeim.c

   Abstract:

   Usb Bus PPI

   --*/

#include "UsbBotPeim.h"
#include "BotPeim.h"
#define PAGESIZE  4096
#include EFI_PPI_DEFINITION( Stall )
#include EFI_PPI_DEFINITION( LoadFile )

static EFI_GUID gPeiStallPpiGuid = EFI_PEI_STALL_PPI_GUID;
static EFI_GUID gPeiBlockIoPpiGuid = EFI_PEI_VIRTUAL_BLOCK_IO_PPI;
static EFI_GUID gPeiUsbIoPpiGuid = PEI_USB_IO_PPI_GUID;

//
// Global function
//
STATIC
EFI_PEI_NOTIFY_DESCRIPTOR mNotifyList = {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_DISPATCH |
    EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gPeiUsbIoPpiGuid,
    NotifyOnUsbIoPpi
};

STATIC
EFI_PEI_RECOVERY_BLOCK_IO_PPI mRecoveryBlkIoPpi = {
    BotGetNumberOfBlockDevices,
    BotGetMediaInfo,
    BotReadBlocks
};

STATIC
EFI_PEI_PPI_DESCRIPTOR mPpiList = {
    EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gPeiBlockIoPpiGuid,
    NULL
};

//
// Driver Entry Point
//
EFI_STATUS
PeimInitializeUsbBot (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices );

STATIC
EFI_STATUS
PeiBotDetectMedia (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDev );

EFI_STATUS PeimInitializeUsbBot (
    IN EFI_FFS_FILE_HEADER *FfsHeader,
    IN EFI_PEI_SERVICES    **PeiServices )
{
    EFI_STATUS     Status;
    UINTN          UsbIoPpiInstance;
    EFI_PEI_PPI_DESCRIPTOR *TempPpiDescriptor;
    PEI_USB_IO_PPI *UsbIoPpi;

  #if (PEI_EHCI_SUPPORT == 1)
    EhciPeiUsbEntryPoint( FfsHeader, PeiServices );
  #endif
  #if (PEI_UHCI_SUPPORT == 1)
    UhciPeiUsbEntryPoint( FfsHeader, PeiServices );
    UhcPeimEntry( FfsHeader, PeiServices );
  #endif
  #if (PEI_OHCI_SUPPORT == 1)
    OhciPeiUsbEntryPoint( FfsHeader, PeiServices ); // 0xff02
  #endif
  #if (PEI_XHCI_SUPPORT == 1)
    XhciPeiUsbEntryPoint( FfsHeader, PeiServices ); // 0xff02
  #endif
    PeimInitializeUsb( FfsHeader, PeiServices ); // 0xff05

    
    //
    // locate all usb io PPIs
    //
    for (UsbIoPpiInstance = 0;
         UsbIoPpiInstance < PEI_FAT_MAX_USB_IO_PPI;
         UsbIoPpiInstance++)
    {
        Status = (**PeiServices).LocatePpi( PeiServices,
            &gPeiUsbIoPpiGuid,
            UsbIoPpiInstance,
            &TempPpiDescriptor,
            &UsbIoPpi
                 );
        if ( EFI_ERROR( Status ) ) {
            break;
        }

        InitUsbBot( PeiServices, UsbIoPpi );

    }

    return EFI_SUCCESS;
}


EFI_STATUS NotifyOnUsbIoPpi (
    IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *InvokePpi )
{
    PEI_USB_IO_PPI *UsbIoPpi;

    UsbIoPpi = (PEI_USB_IO_PPI *) InvokePpi;

    InitUsbBot( PeiServices, UsbIoPpi );

    return EFI_SUCCESS;
}

EFI_STATUS GetMaxLun (
    IN  EFI_PEI_SERVICES    **PeiServices,
    IN  PEI_USB_IO_PPI      *UsbIoPpi,
    IN  UINT8               Port,
    OUT UINT8               *MaxLun )
{
    EFI_USB_DEVICE_REQUEST DevReq;
    EFI_STATUS EfiStatus;
    UINT32     Timeout;

    ZeroMem( &DevReq, sizeof(EFI_USB_DEVICE_REQUEST) );

    //
    // Fill Device request packet
    //
    DevReq.RequestType = 0xA1;
    DevReq.Request = 0x0FE;
    DevReq.Value = 0;
    DevReq.Index = Port;
    DevReq.Length = sizeof(UINT8);

    Timeout = 3000;

    EfiStatus = UsbIoPpi->UsbControlTransfer(
        PeiServices,
        UsbIoPpi,
        &DevReq,
        UsbDataIn,
        Timeout,
        MaxLun,
        sizeof(UINT8)
                );

    return EfiStatus;
}


EFI_STATUS InitUsbBot (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_USB_IO_PPI   *UsbIoPpi )
{
    STATIC UINTN   UsbIoPpiIndex = 0;

    PEI_BOT_DEVICE *PeiBotDevice;
    EFI_STATUS     Status;
    EFI_USB_INTERFACE_DESCRIPTOR *InterfaceDesc;
    UINTN MemPages;
    EFI_PHYSICAL_ADDRESS         AllocateAddress;
    EFI_USB_ENDPOINT_DESCRIPTOR  *EndpointDesc;
    UINT8 i;
    UINT8 MaxLun = 0;
    UINT8 CurrentLun;

    //
    // Check its interface
    //
    Status = UsbIoPpi->UsbGetInterfaceDescriptor( PeiServices,
        UsbIoPpi, &InterfaceDesc );
    if ( EFI_ERROR( Status ) ) {
        return Status;
    }

    //
    // Check if it is the BOT device we support
    //

    if ( (InterfaceDesc->InterfaceClass != BASE_CLASS_MASS_STORAGE)
        || (InterfaceDesc->InterfaceProtocol != PROTOCOL_BOT) ) {

        return EFI_NOT_FOUND;
    }

    Status = GetMaxLun(PeiServices,UsbIoPpi,InterfaceDesc->InterfaceNumber,&MaxLun);
    
    for(CurrentLun = 0; CurrentLun <= MaxLun; CurrentLun++) {
    
        MemPages = sizeof (PEI_BOT_DEVICE) / PAGESIZE + 1;
        Status = (*PeiServices)->AllocatePages( PeiServices,
        		EfiBootServicesData, MemPages, &AllocateAddress );
        if ( EFI_ERROR( Status ) ) {
            return Status;
        }

        PeiBotDevice = (PEI_BOT_DEVICE *) ( (UINTN) AllocateAddress );

        PeiBotDevice->Signature = PEI_BOT_DEVICE_SIGNATURE;
        PeiBotDevice->UsbIoPpi = UsbIoPpi;
        PeiBotDevice->AllocateAddress = (UINTN) AllocateAddress;
        PeiBotDevice->BotInterface = InterfaceDesc;
        PeiBotDevice->FdEmulOffset = 0; //bala
        //
        // Default value
        //
        PeiBotDevice->Media.DeviceType = UsbMassStorage;
        PeiBotDevice->Media.BlockSize = 0x200;
        PeiBotDevice->Lun = CurrentLun;
    
        //
        // Check its Bulk-in/Bulk-out endpoint
        //
        for (i = 0; i < 2; i++) {
            Status = UsbIoPpi->UsbGetEndpointDescriptor( PeiServices,
                UsbIoPpi, i, &EndpointDesc );
            if ( EFI_ERROR( Status ) ) {
                return Status;
            }

            if (EndpointDesc->Attributes != 2) {
                continue;
            }

            if ( (EndpointDesc->EndpointAddress & 0x80) != 0 ) {
                PeiBotDevice->BulkInEndpoint = EndpointDesc;
            }
            else {
                PeiBotDevice->BulkOutEndpoint = EndpointDesc;
            }

        }

        PeiBotDevice->BlkIoPpi = mRecoveryBlkIoPpi;
        PeiBotDevice->BlkIoPpiList = mPpiList;
        PeiBotDevice->BlkIoPpiList.Ppi = &PeiBotDevice->BlkIoPpi;

        Status = PeiUsbInquiry( PeiServices, PeiBotDevice );
        if ( EFI_ERROR( Status ) ) {
            return Status;
        }

        Status = (**PeiServices).AllocatePages( PeiServices,
        	EfiBootServicesData, 1, &AllocateAddress );
        if ( EFI_ERROR( Status ) ) {
            return Status;
        }

        PeiBotDevice->SensePtr = (REQUEST_SENSE_DATA *) ( (UINTN)
                                                        AllocateAddress );

        Status = (**PeiServices).InstallPpi( PeiServices,
            &PeiBotDevice->BlkIoPpiList );
        if ( EFI_ERROR( Status ) ) {
            return Status;
        }
    }

    return EFI_SUCCESS;
}


EFI_STATUS BotGetNumberOfBlockDevices (
    IN EFI_PEI_SERVICES              **PeiServices,
    IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *This,
    OUT UINTN                        *NumberBlockDevices )
{
    //
    // For Usb devices, this value should be always 1
    //

    *NumberBlockDevices = 1;
    return EFI_SUCCESS;
}


EFI_STATUS BotGetMediaInfo (
    IN EFI_PEI_SERVICES              **PeiServices,
    IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *This,
    IN UINTN                         DeviceIndex,
    OUT EFI_PEI_BLOCK_IO_MEDIA       *MediaInfo )
{
    PEI_BOT_DEVICE *PeiBotDev;
    EFI_STATUS     Status = EFI_SUCCESS;

    PeiBotDev = PEI_BOT_DEVICE_FROM_THIS( This );

    Status = PeiBotDetectMedia(
        PeiServices,
        PeiBotDev
             );

    if ( EFI_ERROR( Status ) ) {
        return Status;
    }

    *MediaInfo = PeiBotDev->Media;

    return EFI_SUCCESS;
}


EFI_STATUS PeiBotDetectMedia (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDev )
{

    EFI_STATUS Status = EFI_SUCCESS;
    UINTN      SenseCounts;
    EFI_PHYSICAL_ADDRESS AllocateAddress;
    REQUEST_SENSE_DATA   *SensePtr;
    UINT8         SenseKey, ASC, ASCQ;
    UINT8         RetryCount, RetryReq = 0;
    PEI_STALL_PPI *StallPpi;
    UINT32        Temp;
    UINT32        Temp1, Temp2, Temp3;

    //PeiUsbReadCapacity fills PeiBotDev structure for
    //BlockSize, LastBlock, Media Present
    for (RetryCount = 0; RetryCount < 25; RetryCount++)
    {
        Status = PeiUsbReadCapacity(
            PeiServices,
            PeiBotDev
                 );

        if ( EFI_ERROR( Status ) )
        {
            Temp = RetryCount;

            //If ReadCapcity fails, then find out type of error
            if (RetryCount == 0)
            {
                //During the first retry allocate the memory
                Status = (**PeiServices).AllocatePages(
                    PeiServices,
                    EfiBootServicesData,
                    1,
                    &AllocateAddress
                         );
                if ( EFI_ERROR( Status ) )
                {
                    return Status;
                }

                SensePtr = PeiBotDev->SensePtr;
                ZeroMem( SensePtr, PAGESIZE );
                Status = (**PeiServices).LocatePpi( PeiServices,
                    &gPeiStallPpiGuid, 0, NULL, &StallPpi );
            }

            Status = PeiUsbRequestSense(
                PeiServices,
                PeiBotDev,
                &SenseCounts,
                (UINT8 *) SensePtr
                     );
            if ( EFI_ERROR( Status ) )
            {
                //If RequestSense also fails, then there is an serious error
                //Return to the caller with appropriate error code
                //              PeiBotDev->Media.MediaPresent = FALSE;
                //              PeiBotDev->Media.BlockSize = 0;
                //              Status = EFI_DEVICE_ERROR;
                //              return EFI_DEVICE_ERROR;
            }
            //TODO:Parse the sense buffer for the error
            //If media getting ready, then wait for few mSec, then
            //retry ReadCapacity
            //For all other errors, return with error condition

            SenseKey = SensePtr->sense_key;
            ASC = SensePtr->addnl_sense_code;
            ASCQ = SensePtr->addnl_sense_code_qualifier;
            Temp1 = SenseKey;
            Temp2 = ASC;
            Temp3 = ASCQ;

            if ( (SenseKey == 0x02) && (ASC == 0x3a) && (ASCQ == 00) )
            {
                //medium Not Present.
                //Don't retry.
                return EFI_DEVICE_ERROR;

            }
            // The following retry logic is broken, assigning RetryReq 1 does
            // not make sense and leads to a dead code later: "if (!RetryReq)"
            // Remove this assignment. EIP34507
            //For all error retry ReadCapacity 25 times
            //RetryReq = 1;   //Do retry
            if (SenseKey == 0x02)
            {
                //Logical Unit Problem
                if (ASC == 0x04)
                {
                    //Becoming Ready/Init Required/ Busy/ Format in Progress.
                    RetryReq = 1;   //Do retry

                }
                if ( (ASC == 0x06) || (ASC == 0x08) )
                {
                    //No ref. found/ Comm failure
                    RetryReq = 1;   //Do retry
                }
            }


            PeiBotDev->Media.MediaPresent = FALSE;
            PeiBotDev->Media.BlockSize = 0;
            Status = EFI_DEVICE_ERROR;
            if (!RetryReq) {
                return Status;
            }
        }
        else {
            Status = EFI_SUCCESS;
            return Status; //Command Passed so return to caller
        }

        //Wait for 100 msec
        StallPpi->Stall( PeiServices, StallPpi, 100 * 1000 );
    }
    return Status;
}


EFI_STATUS BotReadBlocks (
    IN EFI_PEI_SERVICES              **PeiServices,
    IN EFI_PEI_RECOVERY_BLOCK_IO_PPI *This,
    IN UINTN                         DeviceIndex,
    IN EFI_PEI_LBA                   StartLBA,
    IN UINTN                         BufferSize,
    OUT VOID                         *Buffer )
{
    PEI_BOT_DEVICE *PeiBotDev;
    EFI_STATUS     Status = EFI_SUCCESS;
    UINTN BlockSize;
    UINTN NumberOfBlocks;
    EFI_PHYSICAL_ADDRESS AllocateAddress;
    REQUEST_SENSE_DATA   *SensePtr;
    UINT8         SenseKey, ASC, ASCQ;
    UINTN         SenseCounts;
    UINT8         RetryCount;
    PEI_STALL_PPI *StallPpi;
    UINT32        Temp1, Temp2, Temp3;

    PeiBotDev = PEI_BOT_DEVICE_FROM_THIS( This );

    Temp1 = (UINT32) StartLBA;

    StartLBA += PeiBotDev->FdEmulOffset;

    //
    // Check parameters
    //
    if (Buffer == NULL)
    {
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize == 0)
    {
        return EFI_SUCCESS;
    }

    BlockSize = PeiBotDev->Media.BlockSize;

    if (BufferSize % BlockSize != 0)
    {
        Status = EFI_BAD_BUFFER_SIZE;
    }

    if (!PeiBotDev->Media.MediaPresent)
    {
        return EFI_NO_MEDIA;
    }

    if (StartLBA > PeiBotDev->Media.LastBlock)
    {
        Status = EFI_INVALID_PARAMETER;
    }

    NumberOfBlocks = BufferSize / (PeiBotDev->Media.BlockSize);

    for (RetryCount = 0; RetryCount < 3; RetryCount++)
    {
        Status = PeiUsbRead10(
            PeiServices,
            PeiBotDev,
            Buffer,
            StartLBA,
            NumberOfBlocks
                 );
        if ( EFI_ERROR( Status ) )
        {

            if (RetryCount == 0)
            {
                Status = (**PeiServices).AllocatePages(
                    PeiServices,
                    EfiBootServicesData,
                    1,
                    &AllocateAddress
                         );
                if ( EFI_ERROR( Status ) )
                {
                    return Status;
                }
                SensePtr = PeiBotDev->SensePtr;
                ZeroMem( SensePtr, PAGESIZE );
                Status = (**PeiServices).LocatePpi( PeiServices,
                    &gPeiStallPpiGuid, 0, NULL, &StallPpi );
            }

            Status = PeiUsbRequestSense(
                PeiServices,
                PeiBotDev,
                &SenseCounts,
                (UINT8 *) SensePtr
                     );
            if ( EFI_ERROR( Status ) )
            {
                //If RequestSense also fails, then there is an serious error
                //Return to the caller with appropriate error code
                return EFI_DEVICE_ERROR;
            }
            //TODO:Parse the sense buffer for the error
            //If media getting ready, then wait for few mSec, then
            //retry ReadCapacity
            //For all other errors, return with error condition

            SenseKey = SensePtr->sense_key;
            ASC = SensePtr->addnl_sense_code;
            ASCQ = SensePtr->addnl_sense_code_qualifier;
            Temp1 = SenseKey;
            Temp2 = ASC;
            Temp3 = ASCQ;
            StallPpi->Stall( PeiServices, StallPpi, 9000 );

        }
        break; //break the for loop
    }
    return EFI_SUCCESS;

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
