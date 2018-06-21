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
#include "UsbHostController.h"

#define PAGESIZE  4096
#include EFI_PPI_DEFINITION( Stall )
#include EFI_PPI_DEFINITION( LoadFile )

static EFI_GUID gPeiBlockIoPpiGuid = EFI_PEI_VIRTUAL_BLOCK_IO_PPI;
static EFI_GUID gPeiEndOfPeiPhaseGuid = EFI_PEI_END_OF_PEI_PHASE_PPI_GUID;

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

STATIC
EFI_PEI_NOTIFY_DESCRIPTOR gUsbPeiNotifyList = {
    EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gPeiEndOfPeiPhaseGuid,
    NotifyOnRecoveryCapsuleLoaded };


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

EFI_STATUS
BotCheckDeviceReady (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDev);

EFI_STATUS
UsbRecoveryEntryPoint(
    IN       EFI_PEI_FILE_HANDLE    FileHandle,
    IN CONST EFI_PEI_SERVICES   **PeiServices
)
{
    EFI_STATUS     Status;
    UINTN          UsbIoPpiInstance;
    EFI_PEI_PPI_DESCRIPTOR *TempPpiDescriptor;
    PEI_USB_IO_PPI *UsbIoPpi;
    UINTN           PpiInstance;

    for (PpiInstance = 0; PpiInstance < PEI_MAX_USB_RECOVERY_INIT_PPI; PpiInstance++)
    {
        PEI_USB_CHIP_INIT_PPI  *UsbChipsetRecoveryInitPpi;
        static EFI_GUID gPeiChipUsbRecoveryInitPpiGuid = PEI_USB_CHIP_INIT_PPI_GUID;

        Status = (**PeiServices).LocatePpi( PeiServices, &gPeiChipUsbRecoveryInitPpiGuid,
            PpiInstance, NULL, &UsbChipsetRecoveryInitPpi );
        if (EFI_ERROR( Status ) ) break;

        UsbChipsetRecoveryInitPpi->EnableChipUsbRecovery(PeiServices);
    }	

  #if (PEI_EHCI_SUPPORT == 1)
    EhciPeiUsbEntryPoint(FileHandle, PeiServices );
  #endif
  #if (PEI_UHCI_SUPPORT == 1)
    UhciPeiUsbEntryPoint(FileHandle, PeiServices );
    UhcPeimEntry(FileHandle, PeiServices );
  #endif
  #if (PEI_OHCI_SUPPORT == 1)
    OhciPeiUsbEntryPoint(FileHandle, PeiServices ); // 0xff02
  #endif
  #if (PEI_XHCI_SUPPORT == 1)
    XhciPeiUsbEntryPoint(FileHandle, PeiServices ); // 0xff02
  #endif
    PeimInitializeUsb(FileHandle, PeiServices ); // 0xff05

    
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

    (**PeiServices).NotifyPpi (PeiServices, &gUsbPeiNotifyList);

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
    EFI_USB_DEVICE_REQUEST      DevReq;
    EFI_STATUS                  EfiStatus;
    UINT32                      Timeout;
    UINT32                      UsbStatus;

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
        sizeof(UINT8),
        &UsbStatus
                );

    return EfiStatus;
}


EFI_STATUS InitUsbBot (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_USB_IO_PPI   *UsbIoPpi )
{


    PEI_BOT_DEVICE *PeiBotDevice;
    EFI_STATUS     Status;
    EFI_USB_INTERFACE_DESCRIPTOR *InterfaceDesc;
    UINT8                        *AllocateAddress;
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
    
        Status = (*PeiServices)->AllocatePool( PeiServices,
            sizeof(PEI_BOT_DEVICE), &AllocateAddress );
        if ( EFI_ERROR( Status ) ) {
            return Status;
        }

        PeiBotDevice = (PEI_BOT_DEVICE *) ( (UINTN) AllocateAddress );
        (**PeiServices).SetMem(PeiBotDevice, sizeof(PEI_BOT_DEVICE), 0);

        PeiBotDevice->Signature = PEI_BOT_DEVICE_SIGNATURE;
        PeiBotDevice->UsbIoPpi = UsbIoPpi;
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


EFI_STATUS
BotGetMediaInfo (
    IN EFI_PEI_SERVICES                     **PeiServices,
    IN EFI_PEI_RECOVERY_BLOCK_IO_PPI        *This,
    IN UINTN                                DeviceIndex,
    OUT EFI_PEI_BLOCK_IO_MEDIA              *MediaInfo
)
{
    PEI_BOT_DEVICE      *PeiBotDev;
    EFI_STATUS          Status;

    PeiBotDev = PEI_BOT_DEVICE_FROM_THIS(This);

    Status = PeiBotDetectMedia(PeiServices, PeiBotDev);

    if (EFI_ERROR(Status)) {
        return Status;
    }

    *MediaInfo = PeiBotDev->Media;

    return EFI_SUCCESS;
}

EFI_STATUS
PeiBotDetectMedia (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDev
)
{

    EFI_STATUS              Status;
    UINT8                   *AllocateAddress;
    REQUEST_SENSE_DATA      *SensePtr;
    UINT8                   SenseKey;
    UINT8                   Asc;
    UINT8                   RetryCount;
    UINT8                   RetryReq = 0;
    PEI_STALL_PPI           *StallPpi;

    //PeiUsbReadCapacity fills PeiBotDev structure for
    //BlockSize, LastBlock, Media Present
    for (RetryCount = 0; RetryCount < 25; RetryCount++) {
        Status = PeiUsbReadCapacity(PeiServices, PeiBotDev);

        if (!EFI_ERROR(Status)) {
            break;
        }

        //If ReadCapcity fails, then find out type of error
        if (RetryCount == 0) {
            if (PeiBotDev->SensePtr == NULL) {
                //During the first retry allocate the memory
                Status = (**PeiServices).AllocatePool(
                                        PeiServices,
                                        sizeof(REQUEST_SENSE_DATA),
                                        &AllocateAddress
                                        );
                if (EFI_ERROR(Status)) {
                    return Status;
                }
                PeiBotDev->SensePtr = (REQUEST_SENSE_DATA *)AllocateAddress;
            }
            SensePtr = PeiBotDev->SensePtr;
            (**PeiServices).SetMem((VOID*)SensePtr, sizeof(REQUEST_SENSE_DATA), 0);
            Status = (**PeiServices).LocatePpi( PeiServices, &gEfiPeiStallPpiGuid, 
                                    0, NULL, &StallPpi );
        }

        Status = PeiUsbRequestSense(PeiServices, PeiBotDev, (UINT8 *)SensePtr);
        if (EFI_ERROR(Status)) {
            //If RequestSense also fails, then there is an serious error
            //Return to the caller with appropriate error code
            //              PeiBotDev->Media.MediaPresent = FALSE;
            //              PeiBotDev->Media.BlockSize = 0;
            //              Status = EFI_DEVICE_ERROR;
            //              return EFI_DEVICE_ERROR;
        }
        
        //Parse the sense buffer for the error
        
        SenseKey = SensePtr->sense_key;
        Asc = SensePtr->addnl_sense_code;
 
        if (SenseKey == 0x02) {
            //Logical Unit Problem
            switch (Asc) {
                case    0x3A:   // //Medium Not Present.
                    if (RetryCount >= 3) {
                        Status = BotCheckDeviceReady(PeiServices, PeiBotDev);
                        if (Status == EFI_NO_MEDIA) {
                            PeiBotDev->Media.MediaPresent = FALSE;
                            return Status;
                        }  
                    }
                case    0x04:   //Becoming Ready/Init Required/ Busy/ Format in Progress.
                case    0x06:   //No ref. found
                case    0x08:   //Comm failure
                    RetryReq = 1;   //Do retry
                    break;
                default:
                    break;
            }
        }

        PeiBotDev->Media.MediaPresent = FALSE;
        PeiBotDev->Media.BlockSize = 0;
        Status = EFI_DEVICE_ERROR;
        if (!RetryReq) {
            return Status;
        }
        //Wait for 100 msec
        StallPpi->Stall(PeiServices, StallPpi, 100 * 1000);
    }
    return Status;
}

EFI_STATUS
BotReadBlocks (
    IN EFI_PEI_SERVICES                     **PeiServices,
    IN EFI_PEI_RECOVERY_BLOCK_IO_PPI        *This,
    IN UINTN                                DeviceIndex,
    IN EFI_PEI_LBA                          StartLBA,
    IN UINTN                                BufferSize,
    OUT VOID                                *Buffer
)
{
    PEI_BOT_DEVICE          *PeiBotDev;
    EFI_STATUS              Status = EFI_SUCCESS;
    UINTN                   BlockSize;
    UINTN                   NumberOfBlocks;
    UINT8                   *AllocateAddress;
    REQUEST_SENSE_DATA      *SensePtr;
    UINT8                   SenseKey;
    UINT8                   Asc;
    UINT8                   RetryCount;
    PEI_STALL_PPI           *StallPpi;

    PeiBotDev = PEI_BOT_DEVICE_FROM_THIS(This);

    StartLBA += PeiBotDev->FdEmulOffset;

    //
    // Check parameters
    //
    if (Buffer == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    if (BufferSize == 0) {
        return EFI_SUCCESS;
    }

    BlockSize = PeiBotDev->Media.BlockSize;

    if (BufferSize % BlockSize != 0) {
        Status = EFI_BAD_BUFFER_SIZE;
    }

    if (!PeiBotDev->Media.MediaPresent) {
        return EFI_NO_MEDIA;
    }

    if (StartLBA > PeiBotDev->Media.LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    NumberOfBlocks = BufferSize / (PeiBotDev->Media.BlockSize);

    for (RetryCount = 0; RetryCount < 3; RetryCount++) {
        
        Status = PeiUsbRead10(
            PeiServices,
            PeiBotDev,
            Buffer,
            StartLBA,
            NumberOfBlocks
                 );

        if (!EFI_ERROR(Status)) {
            break;
        }

        if (RetryCount == 0) {
            if (PeiBotDev->SensePtr == NULL) {
                Status = (**PeiServices).AllocatePool(
                                        PeiServices,
                                        sizeof(REQUEST_SENSE_DATA),
                                        &AllocateAddress
                                        );
                if (EFI_ERROR(Status)) {
                    return Status;
                }
                PeiBotDev->SensePtr = (REQUEST_SENSE_DATA *)AllocateAddress;
            }
            SensePtr = PeiBotDev->SensePtr;
            (**PeiServices).SetMem((VOID*)SensePtr, sizeof(REQUEST_SENSE_DATA), 0);
            Status = (**PeiServices).LocatePpi(PeiServices, &gEfiPeiStallPpiGuid, 
                                        0, NULL, &StallPpi);
        }

        Status = PeiUsbRequestSense(PeiServices, PeiBotDev, (UINT8 *)SensePtr);
        if (EFI_ERROR(Status)) {
            //If RequestSense also fails, then there is an serious error
            //Return to the caller with appropriate error code
            return EFI_DEVICE_ERROR;
        }
        
        //Parse the sense buffer for the error

        SenseKey = SensePtr->sense_key;
        Asc = SensePtr->addnl_sense_code;

        if ((SenseKey == 0x02) && (Asc == 0x3A)) {
            Status = BotCheckDeviceReady(PeiServices, PeiBotDev);
            if (EFI_ERROR(Status)) {
                if (Status == EFI_NO_MEDIA) {
                    PeiBotDev->Media.MediaPresent = FALSE;
                }
                    return Status;
            }
        }
        StallPpi->Stall(PeiServices, StallPpi, 9000);
    }
    
    if (RetryCount == 3) {
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;

}

EFI_STATUS
BotCheckDeviceReady (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDev
)
{
    UINT8                   RetryCount;
    UINT8                   SenseKey;
    UINT8                   Asc;
    EFI_STATUS              Status;
    REQUEST_SENSE_DATA      *SensePtr;
    PEI_STALL_PPI           *StallPpi;
    UINT8                   *AllocateAddress;

    if (PeiBotDev->SensePtr == NULL) {
        Status = (**PeiServices).AllocatePool(
                                PeiServices,
                                sizeof(REQUEST_SENSE_DATA),
                                &AllocateAddress
                                );
        if (EFI_ERROR(Status)) {
            return Status;
        }
        PeiBotDev->SensePtr = (REQUEST_SENSE_DATA *)AllocateAddress;
    }
    SensePtr = PeiBotDev->SensePtr;
    (**PeiServices).SetMem((VOID*)SensePtr, sizeof(REQUEST_SENSE_DATA), 0);
    
    Status = (**PeiServices).LocatePpi(PeiServices,
                    &gEfiPeiStallPpiGuid, 0, NULL, &StallPpi);

    for (RetryCount = 0; RetryCount < 3; RetryCount++) {
        
        PeiUsbTestUnitReady(PeiServices, PeiBotDev);

        Status = PeiUsbRequestSense(PeiServices, PeiBotDev, (UINT8 *)SensePtr);
        if (EFI_ERROR(Status)) {
            //If RequestSense also fails, then there is an serious error
            //Return to the caller with appropriate error code
            return EFI_DEVICE_ERROR;
        }
        
        SenseKey = SensePtr->sense_key;
        Asc = SensePtr->addnl_sense_code;

        if (SenseKey == 0) {
            Status = EFI_SUCCESS;
            break;
        }
        if ((SenseKey == 0x28) && (Asc == 0x06))  {
            Status = EFI_MEDIA_CHANGED;
            StallPpi->Stall(PeiServices, StallPpi, 100 * 1000);
            continue;
        }
        if ((SenseKey == 0x02) && (Asc == 0x3A)) {
            Status = EFI_NO_MEDIA;
            StallPpi->Stall(PeiServices, StallPpi, 20 * 1000);
            continue;
        }
    }
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NotifyOnRecoveryCapsuleLoaded
//
// Description: This routine halts all available host controllers at end of PEI
//
// Input:       IN EFI_PEI_SERVICES          **PeiServices,
//	            IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
//	            IN VOID                      *InvokePpi 
//
// Output:
//              EFI_STATUS - this function returns EFI_SUCCESS if the
//                  host controller is reset successfully. Otherwise, returns
//                  any type of error it encountered during the reset operation.
//              
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
NotifyOnRecoveryCapsuleLoaded (
    IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *InvokePpi
)
{
    EFI_STATUS                      Status;
    UINT8                           ControllerIndex = 0;
    PEI_USB_HOST_CONTROLLER_PPI *UsbHcPpi;
    UINT8                           NumOfRootPort;
    UINT8                           PortNum;

    while (TRUE) {
        
        Status = (**PeiServices).LocatePpi(
                    PeiServices,
                    &gPeiUsbHostControllerPpiGuid,
                    ControllerIndex++,
                    NULL,
                    &UsbHcPpi
                    );

        if (EFI_ERROR (Status)) {
            break;
        }

        UsbHcPpi->GetRootHubPortNumber(PeiServices, UsbHcPpi, &NumOfRootPort);
        
        for (PortNum = 1; PortNum <= NumOfRootPort; PortNum++) {
            UsbHcPpi->ClearRootHubPortFeature(
                        PeiServices, UsbHcPpi, PortNum, EfiUsbPortEnable);
        }
     
        if (UsbHcPpi->Reset != NULL ) {
            Status = UsbHcPpi->Reset(PeiServices, UsbHcPpi, EFI_USB_HC_RESET_GLOBAL);
        }
    }    

    return EFI_SUCCESS;
}

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
