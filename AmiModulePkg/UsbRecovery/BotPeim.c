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
// Name:        BotPeim.C
//
// Description: This file belongs to "Framework".
//              This file is modified by AMI to include copyright message,
//              appropriate header and integration code.
//              This file contains generic routines needed for USB recovery
//              Mass Storage BOT PEIM
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

   BotPeim.c

   Abstract:

   BOT Transportation implementation

   --*/

#include "UsbBotPeim.h"
#include "BotPeim.h"
#include "PeiUsbLib.h"
#include <Library/BaseMemoryLib.h>

extern VOID PeiCopyMem (
    IN VOID  *Destination,
    IN VOID  *Source,
    IN UINTN Length );


STATIC
EFI_STATUS BotRecoveryReset (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDev )
{
    EFI_USB_DEVICE_REQUEST DevReq;
    UINT32 Timeout;
    PEI_USB_IO_PPI         *UsbIoPpi;
    UINT8      EndpointAddr;
    EFI_STATUS Status;

    UsbIoPpi = PeiBotDev->UsbIoPpi;

    if (UsbIoPpi == NULL) {
        return EFI_INVALID_PARAMETER;
    }

    ZeroMem( &DevReq, sizeof(EFI_USB_DEVICE_REQUEST) );

    DevReq.RequestType = 0x21;
    DevReq.Request = 0xff;
    DevReq.Value = 0;
    DevReq.Index = 0;
    DevReq.Length = 0;

    Timeout = 3000;

    Status = UsbIoPpi->UsbControlTransfer(
        PeiServices,
        UsbIoPpi,
        &DevReq,
        EfiUsbNoData,
        Timeout,
        NULL,
        0
             );

    //
    // clear bulk in endpoint stall feature
    //
    EndpointAddr = (PeiBotDev->BulkInEndpoint)->EndpointAddress;
    PeiUsbClearEndpointHalt( PeiServices, UsbIoPpi, EndpointAddr );

    //
    // clear bulk out endpoint stall feature
    //
    EndpointAddr = (PeiBotDev->BulkOutEndpoint)->EndpointAddress;
    PeiUsbClearEndpointHalt( PeiServices, UsbIoPpi, EndpointAddr );

    return Status;
}


STATIC
EFI_STATUS BotCommandPhase (
    IN EFI_PEI_SERVICES       **PeiServices,
    IN PEI_BOT_DEVICE         *PeiBotDev,
    IN VOID                   *Command,
    IN UINT8                  CommandSize,
    IN UINT32                 DataTransferLength,
    IN EFI_USB_DATA_DIRECTION Direction,
    IN UINT16                 Timeout )
{
    CBW            cbw;
    EFI_STATUS     Status;
    PEI_USB_IO_PPI *UsbIoPpi;
    UINTN          DataSize;

    UsbIoPpi = PeiBotDev->UsbIoPpi;

    ZeroMem( &cbw, sizeof(CBW) );

    //
    // Fill the command block, detailed see BOT spec
    //
    cbw.dCBWSignature = CBWSIG;
    cbw.dCBWTag = 0x01;
    cbw.dCBWDataTransferLength = DataTransferLength;
    cbw.bmCBWFlags = (UINT8) (Direction << 7);
    cbw.bCBWLUN = PeiBotDev->Lun;
    cbw.bCBWCBLength = CommandSize;

    PeiCopyMem( cbw.CBWCB, Command, CommandSize );

    DataSize = sizeof(CBW);

    Status = UsbIoPpi->UsbBulkTransfer(
        PeiServices,
        UsbIoPpi,
        (PeiBotDev->BulkOutEndpoint)->EndpointAddress,
        (UINT8 *) &cbw,
        &DataSize,
        Timeout
             );
    if ( EFI_ERROR( Status ) ) {
        //
        // Command phase fail, we need to recovery reset this device
        //
        BotRecoveryReset( PeiServices, PeiBotDev );
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
}


STATIC
EFI_STATUS BotDataPhase (
    IN EFI_PEI_SERVICES       **PeiServices,
    IN PEI_BOT_DEVICE         *PeiBotDev,
    IN UINT32                 *DataSize,
    IN OUT VOID               *DataBuffer,
    IN EFI_USB_DATA_DIRECTION Direction,
    IN UINT16                 Timeout )
{
    EFI_STATUS     Status;
    PEI_USB_IO_PPI *UsbIoPpi;
    UINT8  EndpointAddr;
    UINTN  Remain;
    UINTN  Increment;
    UINT32 MaxPacketLen;
    UINT8  *BufferPtr;
    UINTN  TransferredSize;
	UINTN  TransferSize;

    UsbIoPpi = PeiBotDev->UsbIoPpi;

    Remain = *DataSize;
    BufferPtr = (UINT8 *) DataBuffer;
    TransferredSize = 0;

    //
    // retrieve the the max packet length of the given endpoint
    //
    if (Direction == UsbDataIn) {
        MaxPacketLen = (PeiBotDev->BulkInEndpoint)->MaxPacketSize;
        EndpointAddr = (PeiBotDev->BulkInEndpoint)->EndpointAddress;
    }
    else {
        MaxPacketLen = (PeiBotDev->BulkOutEndpoint)->MaxPacketSize;
        EndpointAddr = (PeiBotDev->BulkOutEndpoint)->EndpointAddress;
    }

    while (Remain > 0) {
        //
        // Using 15 packets to avoid Bitstuff error
        //
        if (Remain > 16 * MaxPacketLen) {
            TransferSize = 16 * MaxPacketLen;
        }
        else {
            TransferSize = Remain;
        }

		Increment = TransferSize;
	
        Status = UsbIoPpi->UsbBulkTransfer(
            PeiServices,
            UsbIoPpi,
            EndpointAddr,
            BufferPtr,
            &Increment,
            Timeout
                 );

        TransferredSize += Increment;

        if ( EFI_ERROR( Status ) ) {
            PeiUsbClearEndpointHalt( PeiServices, UsbIoPpi, EndpointAddr );
            return Status;
        }
	
		if (Increment < TransferSize) {
			break;
		}
		
        BufferPtr += Increment;
        Remain -= Increment;
    }

    *DataSize = (UINT32) TransferredSize;

    return EFI_SUCCESS;
}


STATIC
EFI_STATUS BotStatusPhase (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDev,
    OUT UINT8           *TransferStatus,
    IN UINT16           Timeout )
{
    CSW            csw;
    EFI_STATUS     Status;
    PEI_USB_IO_PPI *UsbIoPpi;
    UINT8          EndpointAddr;
    UINTN          DataSize;

    //UINT32 Temp;

    UsbIoPpi = PeiBotDev->UsbIoPpi;

    ZeroMem( &csw, sizeof(CSW) );

    EndpointAddr = (PeiBotDev->BulkInEndpoint)->EndpointAddress;

    //  DataSize = sizeof(CSW);
    DataSize = 0x0d; //bala changed

    //
    // Get the status field from bulk transfer
    //
    Status = UsbIoPpi->UsbBulkTransfer(
        PeiServices,
        UsbIoPpi,
        EndpointAddr,
        &csw,
        &DataSize,
        Timeout
             );

    if ( EFI_ERROR( Status ) ) {
        return Status;
    }

    if (csw.dCSWSignature == CSWSIG)
    {
        *TransferStatus = csw.bCSWStatus;
    }
    else {
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
}


EFI_STATUS PeiAtapiCommand (
    IN EFI_PEI_SERVICES       **PeiServices,
    IN PEI_BOT_DEVICE         *PeiBotDev,
    IN VOID                   *Command,
    IN UINT8                  CommandSize,
    IN VOID                   *DataBuffer,
    IN UINT32                 BufferLength,
    IN EFI_USB_DATA_DIRECTION Direction,
    IN UINT16                 TimeOutInMilliSeconds )

/*++

   Routine Description:
    Send ATAPI command using BOT protocol.

   Arguments:
    This                  - Protocol instance pointer.
    Command               - Command buffer
    CommandSize           - Size of Command Buffer
    DataBuffer            - Data buffer
    BufferLength          - Length of Data buffer
    Direction             - Data direction of this command
    TimeoutInMilliseconds - Timeout value in ms

   Returns:
    EFI_SUCCES          - Commond succeeded.
    EFI_DEVICE_ERROR    - Command failed.

   --*/
{
    EFI_STATUS Status;
    EFI_STATUS BotDataStatus = EFI_SUCCESS;
    UINT8      TransferStatus;
    UINT32     BufferSize;
    UINT8      *Tmp;
    UINT32     Temp;

    Tmp = (UINT8 *) Command;
    Temp = Tmp[0];
    PEI_TRACE( (EFI_D_ERROR, PeiServices, "Executing ScsiCmd(%x)\n", Temp) );
    //
    // First send ATAPI command through Bot
    //
    Status = BotCommandPhase(
        PeiServices,
        PeiBotDev,
        Command,
        CommandSize,
        BufferLength,
        Direction,
        TimeOutInMilliSeconds
             );

    if ( EFI_ERROR( Status ) ) {
        return EFI_DEVICE_ERROR;
    }

    //
    // Send/Get Data if there is a Data Stage
    //
    switch (Direction)
    {
    case UsbDataIn:
    case UsbDataOut:
        BufferSize = BufferLength;

        BotDataStatus = BotDataPhase(
            PeiServices,
            PeiBotDev,
            &BufferSize,
            DataBuffer,
            Direction,
            TimeOutInMilliSeconds
                        );

        break;

    case EfiUsbNoData:
        break;
    }

    //
    // Status Phase
    //

    Status = BotStatusPhase(
        PeiServices,
        PeiBotDev,
        &TransferStatus,
        TimeOutInMilliSeconds
             );
    if ( EFI_ERROR( Status ) ) {
        BotRecoveryReset( PeiServices, PeiBotDev );
        return EFI_DEVICE_ERROR;
    }

    if (TransferStatus == 0x01) {
        return EFI_DEVICE_ERROR;
    }

    return BotDataStatus;
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
