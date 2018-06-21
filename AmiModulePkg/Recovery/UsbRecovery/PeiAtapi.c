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
// Name:        PeiAtapi.C
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

   PeiAtapi.c

   Abstract:

   Pei USB ATATPI command implementations

   --*/

#include "UsbBotPeim.h"
#include "BotPeim.h"
#include "Atapi.h"

#define MAXSENSEKEY  5

// Function prototype

VOID PeiUsbValidatePartitionTable (
    EFI_PEI_SERVICES **PeiServices,
    UINT8            *Buffer,
    UINTN            LastBlock,
    UINT32           *FdEmulOffset );



//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PeiUsbInquiry
//
// Description: This function issues the ATA Inquiry command to the USB
//              mass storage device
//
// Input:       **PeiServices       Pointer to the PEI services table
//              *PeiBotDevice       Pointer to the PEI_BOT_DEVICE structure
//
// Output:      Return Status based on the command
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS PeiUsbInquiry (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDevice )
{
    ATAPI_PACKET_COMMAND Packet;
    EFI_STATUS Status;
    USB_INQUIRY_DATA     Idata;
	UINT32 DataLen;

    // Fill command packet
    DataLen = sizeof(USB_INQUIRY_DATA);
	ZeroMem( &Packet, sizeof(ATAPI_PACKET_COMMAND) );
    ZeroMem( &Idata, sizeof(USB_INQUIRY_DATA) );

    Packet.Inquiry.opcode = INQUIRY;
    Packet.Inquiry.page_code = 0;
    Packet.Inquiry.allocation_length = sizeof(USB_INQUIRY_DATA);

    // Send command packet
    Status = PeiAtapiCommand( PeiServices, PeiBotDevice, &Packet,
        sizeof(ATAPI_PACKET_COMMAND), &Idata,
        &DataLen, EfiUsbDataIn, 5000 ); //(EIP69936) 
    if ( EFI_ERROR( Status ) ) {
        return EFI_DEVICE_ERROR;
    }

    if ( (Idata.peripheral_type & 0x1f) == 0x05 ) {
        PeiBotDevice->DeviceType = USBCDROM;
        PeiBotDevice->Media.BlockSize = 0x800;
    }
    else {
        PeiBotDevice->DeviceType = USBFLOPPY;
        PeiBotDevice->Media.BlockSize = 0x200;
    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PeiUsbTestUnitReady
//
// Description: This function issues the ATA Test Unit Ready command to the USB
//              mass storage device
//
// Input:       **PeiServices       Pointer to the PEI services table
//              *PeiBotDevice       Pointer to the PEI_BOT_DEVICE structure
//
// Output:      Return Status based on the command
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS PeiUsbTestUnitReady (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDevice )
{
    EFI_STATUS Status;
    ATAPI_PACKET_COMMAND Packet;

    // Fill command packet
    ZeroMem( &Packet, sizeof(ATAPI_PACKET_COMMAND) );
    Packet.TestUnitReady.opcode = TEST_UNIT_READY;

    // Send command packet
    Status = PeiAtapiCommand( PeiServices, PeiBotDevice, &Packet,
        sizeof(ATAPI_PACKET_COMMAND), NULL, NULL, EfiUsbNoData,
        5000 );
    if ( EFI_ERROR( Status ) ) {
        return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PeiUsbRequestSense
//
// Description: This function issues the ATA Request Sense command to the USB
//              mass storage device
//
// Input:       **PeiServices       Pointer to the PEI services table
//              *PeiBotDevice       Pointer to the PEI_BOT_DEVICE structure
//              *SenseCounts        Buffer to return sense key data
//              *SenseKeyBuffer     Buffer used for internal use
//
// Output:      Return Status based on the command
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS PeiUsbRequestSense (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDevice,
    IN UINT8            *SenseKeyBuffer )
{
    EFI_STATUS Status;
    ATAPI_PACKET_COMMAND Packet;
	UINT32 DataLen;

    // Fill command packet for Request Sense Packet Command
    ZeroMem( &Packet, sizeof(ATAPI_PACKET_COMMAND) );
    Packet.RequestSence.opcode = REQUEST_SENSE;
    Packet.RequestSence.allocation_length = sizeof(REQUEST_SENSE_DATA);

	DataLen = sizeof(REQUEST_SENSE_DATA);

    // Send out Request Sense Packet Command and get one Sense
    // data form device.
    Status = PeiAtapiCommand( PeiServices, PeiBotDevice, &Packet,
        sizeof(ATAPI_PACKET_COMMAND),
        (VOID *) SenseKeyBuffer,
        &DataLen,
        EfiUsbDataIn,
        5000 );

    // Failed to get Sense data
    if ( EFI_ERROR( Status ) ) {
		return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PeiUsbReadCapacity
//
// Description: This function issues the ATA Read Capacity command to the USB
//              mass storage device
//
// Input:       **PeiServices       Pointer to the PEI services table
//              *PeiBotDevice       Pointer to the PEI_BOT_DEVICE structure
//
// Output:      Return Status based on the command
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS PeiUsbReadCapacity (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDevice )
{
    EFI_STATUS           Status;
    ATAPI_PACKET_COMMAND Packet;
    READ_CAPACITY_DATA   Data;
    UINT8 Buffer[512];
	UINT32 DataLen;

    DataLen = sizeof(Data);
	ZeroMem( &Data, sizeof(READ_CAPACITY_DATA) );
    ZeroMem( &Packet, sizeof(ATAPI_PACKET_COMMAND) );

    Packet.Inquiry.opcode = READ_CAPACITY;

    // Send command packet
    Status = PeiAtapiCommand( PeiServices, PeiBotDevice, &Packet,
        sizeof(ATAPI_PACKET_COMMAND),
        (VOID *) &Data,
        &DataLen,
        EfiUsbDataIn,
        5000 );
    if ( EFI_ERROR( Status ) || (DataLen != sizeof(Data)))  {
        return EFI_DEVICE_ERROR;
    }

    PeiBotDevice->Media.LastBlock = (Data.LastLba3 << 24) |
                                    (Data.LastLba2 << 16) |
                                    (Data.LastLba1 << 8) |
                                    Data.LastLba0;

    // AMI Changes ->>
    //This is the right place to set Medium BlockSize.
    PeiBotDevice->Media.BlockSize = (Data.BlockSize3 << 24) |
                                    (Data.BlockSize2 << 16) |
                                    (Data.BlockSize1 << 8) |
                                    Data.BlockSize0;
    // <-- AMI Changes
	if (PeiBotDevice->Media.BlockSize == 0) {
        return EFI_DEVICE_ERROR;
    }

    PeiBotDevice->Media.MediaPresent = TRUE;

    // BIOS Forced FDD option to emulate USB Key Hard Drive as Floppy
    // Do Floppy emulation only for Harddrive/Direct Access Device
    if (PeiBotDevice->DeviceType == USBFLOPPY) {
        Status = PeiUsbRead10( PeiServices, PeiBotDevice, Buffer,
            0, /*StartLBA*/ 1 /*NumberOfBlocks*/ );
        if ( EFI_ERROR( Status ) ) {
            // Don't return error, as this shouldn't
            // messup with ReadCapacity
            return EFI_SUCCESS;
        }
        PeiUsbValidatePartitionTable( PeiServices, Buffer,
            (UINTN)PeiBotDevice->Media.LastBlock, &(PeiBotDevice->FdEmulOffset) );
    }
    return EFI_SUCCESS;
}


// AMI Changes ->>

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PeiUsbValidatePartitionTable
//
// Description: This function validates the existence of the partition table
//              from the LBA 0 data provided and return FdEmulOffset value
//              (hidden sector) from the partition table
//
// Input:       **PeiServices       Pointer to the PEI services table
//              *Buffer             Pointer to the buffer containing LBA 0
//                                  data
//              LastBlock           Last LBA address
//
//              *FdEmulOffset       Returned FD emulation hidden sector value
// Output:      Nothing
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID PeiUsbValidatePartitionTable (
    IN EFI_PEI_SERVICES **PeiServices,
    IN UINT8            *Buffer,
    IN UINTN            LastBlock,
    OUT UINT32          *FdEmulOffset )
{
    UINT8 i;
    UINT8 *pTable = Buffer + 0x1be;   // Start offset of partition
                                      // table
    UINT8 *active_partition_addr = NULL;
    UINT8 PartitionFound = 0;     // Total number of valid
                                  // partitions
    UINT8 *PartitionTableEntries[4];

    for (i = 0; i < 4; i++) {
        PartitionTableEntries[i] = 0;

        // Boot flag check added to ensure that boot sector will not be
        // treated as a valid partation table.
        if (*pTable & 0x7f) {
            return; // BootFlag should be 0x0 or 0x80

        }
        // Check whether beginning LBA is reasonable
        if (*(UINT32 *) (pTable + 8) > LastBlock) {
            return;
        }

        // Check whether the size is reasonable

        /** This check has to be refined
         #if HDD_PART_SIZE_CHECK
                if (*(UINT32*)(pTable + 0xc) > LastBlock)
                    return;
         #endif
         **/
        PartitionFound++;
        PartitionTableEntries[i] = pTable;
        // Update active entry offset
        if (*pTable & 0x80) {
            active_partition_addr = pTable;
        }

        pTable += 0x10; //Get next Partition
    }

    if (PartitionFound == 0) {
        return;
    }

    // If no active partition table entry found use first entry
    if (active_partition_addr == NULL) {
        for (i = 0; (i < 4) && !PartitionTableEntries[i]; i++) {
            ;
        }
        if (i == 4) return;
        active_partition_addr = PartitionTableEntries[i];
    }

    *FdEmulOffset = *( (UINT32 *) (active_partition_addr + 8) );
    PEI_TRACE( (EFI_D_ERROR, PeiServices,
                "USBPEIM: PeiUsbValidatePartitionTable() FdEmulOffset %x\n",
                *FdEmulOffset) );
}


// <-- AMI Changes


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PeiUsbReadFormattedCapacity
//
// Description: This function issues the ATA Read Formatted Capacity command
//              to the USB mass storage device
//
// Input:       **PeiServices       Pointer to the PEI services table
//              *PeiBotDevice       Pointer to the PEI_BOT_DEVICE structure
//
// Output:      Return Status based on the command
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS PeiUsbReadFormattedCapacity (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDevice )
{
    EFI_STATUS Status;
    ATAPI_PACKET_COMMAND      Packet;
    READ_FORMAT_CAPACITY_DATA FormatData;
	UINT32 DataLen;
	
	
	DataLen = sizeof(FormatData);
    ZeroMem( &FormatData, sizeof(READ_FORMAT_CAPACITY_DATA) );
	ZeroMem( &Packet, sizeof(ATAPI_PACKET_COMMAND) );
	
    Packet.ReadFormatCapacity.opcode = READ_FORMAT_CAPACITY;
    Packet.ReadFormatCapacity.allocation_length_lo = 12;

    // Send command packet
    Status = PeiAtapiCommand( PeiServices, PeiBotDevice, &Packet,
        sizeof(ATAPI_PACKET_COMMAND), (VOID *) &FormatData,
        &DataLen, EfiUsbDataIn, 5000 );
    if ( EFI_ERROR( Status ) ) {
        return EFI_DEVICE_ERROR;
    }

    if (FormatData.DesCode == 3) {
        // Media is not present
        PeiBotDevice->Media.MediaPresent = FALSE;
        PeiBotDevice->Media.LastBlock = 0;
    }
    else {
        PeiBotDevice->Media.LastBlock = (FormatData.LastLba3 << 24) |
                                        (FormatData.LastLba2 << 16) |
                                        (FormatData.LastLba1 << 8) |
                                        FormatData.LastLba0;

        PeiBotDevice->Media.LastBlock--;
        PeiBotDevice->Media.MediaPresent = TRUE;
    }

    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PeiUsbRead10
//
// Description: This function issues the ATA Read (10) command to the USB
//              mass storage device
//
// Input:       **PeiServices       Pointer to the PEI services table
//              *PeiBotDevice       Pointer to the PEI_BOT_DEVICE structure
//              *Buffer             Buffer to read the data into
//              Lba                 Start LBA Number
//              NumberOfBlocks      Number of blocks to read
//
// Output:      Return Status based on the command
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS PeiUsbRead10 (
    IN EFI_PEI_SERVICES **PeiServices,
    IN PEI_BOT_DEVICE   *PeiBotDevice,
    IN VOID             *Buffer,
    IN EFI_PEI_LBA      Lba,
    IN UINTN            NumberOfBlocks )
{
    VOID       *ptrBuffer;
    UINT16     MaxBlock;
    UINT16     BlocksRemaining;
    UINT16     SectorCount;
    UINT16     TimeOut;
    //  UINT32                  BufferSize;
    UINT32     Lba32;
    UINT32     BlockSize;
    UINT32     ByteCount;
    EFI_STATUS Status;
    READ10_CMD *Read10Packet;
    ATAPI_PACKET_COMMAND Packet;

    if (PeiBotDevice->Media.BlockSize == 0) {
        return EFI_DEVICE_ERROR;
    }

    // Prepare command packet for the Inquiry Packet Command.
    ZeroMem( &Packet, sizeof(ATAPI_PACKET_COMMAND) );
    Read10Packet = &Packet.Read10;
    Lba32 = (UINT32) Lba;
    ptrBuffer = Buffer;

    BlockSize = (UINT32) PeiBotDevice->Media.BlockSize;
    // AMI Changes -->
    MaxBlock = (UINT16) (65536 / BlockSize);
    //  BufferSize = NumberOfBlocks * BlockSize;
    //  MaxBlock = (UINT16)(BufferSize /BlockSize);
    // <-- AMI Changes
    BlocksRemaining = (UINT16) NumberOfBlocks;

    Status = EFI_SUCCESS;
    while (BlocksRemaining > 0) {
        if (BlocksRemaining <= MaxBlock) {
            SectorCount = BlocksRemaining;
        }
        else {
            SectorCount = MaxBlock;
        }

        // Fill the Packet data structure
        Read10Packet->opcode = READ_10;

        // Lba0 ~ Lba3 specify the start logical block
        // address of the data transfer.
        // Lba0 is MSB, Lba3 is LSB
        Read10Packet->Lba3 = (UINT8) (Lba32 & 0xff);
        Read10Packet->Lba2 = (UINT8) (Lba32 >> 8);
        Read10Packet->Lba1 = (UINT8) (Lba32 >> 16);
        Read10Packet->Lba0 = (UINT8) (Lba32 >> 24);

        // TranLen0 ~ TranLen1 specify the transfer length in block unit.
        // TranLen0 is MSB, TranLen is LSB
        Read10Packet->TranLen1 = (UINT8) (SectorCount & 0xff);
        Read10Packet->TranLen0 = (UINT8) (SectorCount >> 8);

        ByteCount = SectorCount * BlockSize;

        TimeOut = (UINT16) (SectorCount * 5000);

        // Send command packet
        Status = PeiAtapiCommand( PeiServices, PeiBotDevice, &Packet,
            sizeof(ATAPI_PACKET_COMMAND), (VOID *) ptrBuffer, &ByteCount,
            EfiUsbDataIn, TimeOut );
        if (Status != EFI_SUCCESS) {
            return Status;
        }

        Lba32 += SectorCount;
        ptrBuffer = (UINT8 *) ptrBuffer + SectorCount * BlockSize;
        BlocksRemaining = (UINT16) (BlocksRemaining - SectorCount);
    }

    return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsNoMedia
//
// Description: This function verifies whether Media is present in the drive
//              or not by checking the Sense Data obtained from the drive
//
// Input:       *SenseData          Sense data obtained from the drive
//              SenseCounts         Sense count value
//
// Output:      Returns TRUE if media is present
//                      FALSE otherwise
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN IsNoMedia (
    IN REQUEST_SENSE_DATA *SenseData,
    IN UINTN              SenseCounts )
{
    UINTN   i;
    BOOLEAN NoMedia;
    REQUEST_SENSE_DATA *SensePtr;

    NoMedia = FALSE;
    SensePtr = SenseData;

    for (i = 0; i < SenseCounts; i++) {
        switch (SensePtr->sense_key)
        {
        case SK_NOT_READY:
            switch (SensePtr->addnl_sense_code)
            {
                // If no media, fill IdeDev parameter with specific info.
            case ASC_NO_MEDIA:
                NoMedia = TRUE;
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }
        SensePtr++;
    }

    return NoMedia;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsMediaError
//
// Description: This function verifies whether Media access is having problem
//              or not by checking the Sense Data obtained from the drive
//
// Input:       *SenseData          Sense data obtained from the drive
//              SenseCounts         Sense count value
//
// Output:      Returns TRUE if media has error
//                      FALSE otherwise
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN IsMediaError (
    IN REQUEST_SENSE_DATA *SenseData,
    IN UINTN              SenseCounts )
{
    UINTN   i;
    BOOLEAN Error;
    REQUEST_SENSE_DATA *SensePtr;

    SensePtr = SenseData;
    Error = FALSE;

    for (i = 0; i < SenseCounts; i++) {
        switch (SensePtr->sense_key)
        {
            // Medium error case
        case SK_MEDIUM_ERROR:
            switch (SensePtr->addnl_sense_code)
            {
            case ASC_MEDIA_ERR1:                // fall through
            case ASC_MEDIA_ERR2:                // fall through
            case ASC_MEDIA_ERR3:                // fall through
            case ASC_MEDIA_ERR4:
                Error = TRUE;
                break;

            default:
                break;
            }
            break;

            // Medium upside-down case
        case SK_NOT_READY:
            switch (SensePtr->addnl_sense_code)
            {
            case ASC_MEDIA_UPSIDE_DOWN:
                Error = TRUE;
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }

        SensePtr++;
    }

    return Error;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsMediaChange
//
// Description: This function verifies whether Media change had happened
//              or not by checking the Sense Data obtained from the drive
//
// Input:       *SenseData          Sense data obtained from the drive
//              SenseCounts         Sense count value
//
// Output:      Returns TRUE if media has changed
//                      FALSE otherwise
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

BOOLEAN IsMediaChange (
    IN REQUEST_SENSE_DATA *SenseData,
    IN UINTN              SenseCounts )
{
    UINTN   i;
    BOOLEAN MediaChange;
    REQUEST_SENSE_DATA *SensePtr;

    MediaChange = FALSE;
    SensePtr = SenseData;
    for (i = 0; i < SenseCounts; i++) {
        // Catch media change sense key and addition sense data
        switch (SensePtr->sense_key)
        {
        case SK_UNIT_ATTENTION:
            switch (SensePtr->addnl_sense_code)
            {
            case ASC_MEDIA_CHANGE:
                MediaChange = TRUE;
                break;

            default:
                break;
            }
            break;

        default:
            break;
        }
        SensePtr++;
    }

    return MediaChange;
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
