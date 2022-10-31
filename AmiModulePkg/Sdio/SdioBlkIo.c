//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioBlkIo.c 2     1/03/12 3:47a Deepthins $
//
// $Revision: 2 $
//
// $Date: 1/03/12 3:47a $
//**********************************************************************

//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    SdioBlkIo.c
//
// Description: BlockIo function implemented
//
//<AMI_FHDR_END>
//**********************************************************************

#include "SdioDriver.h"


//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SdioBlkRead
//
// Description: Read from the Sdio ATA Device
//
// Input:
//  IN EFI_BLOCK_IO_PROTOCOL        *This,
//  IN UINT32                       MediaId,
//  IN EFI_LBA                      LBA,
//  IN UINTN                        BufferSize,
//  OUT VOID                        *Buffer
//
// Output:
//      EFI_STATUS
//
// Modified:
//
// Referrals: SdioAtaBlkReadWrite
//
// Notes:
//
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SdioBlkRead(
    IN EFI_BLOCK_IO_PROTOCOL        *This,
    IN UINT32                       MediaId,
    IN EFI_LBA                      LBA,
    IN UINTN                        BufferSize,
    OUT VOID                        *Buffer
 )
{

    EFI_STATUS  Status;

    Status =  SdioAtaBlkReadWrite(This, MediaId, LBA, BufferSize, Buffer, 0);

    TRACE((-1,"Sdio Read: LBA : %lx ByteCount : %lx\n", LBA, BufferSize));

    return Status;
}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SdioBlkWrite
//
// Description: Write to Sdio ATA Device
//
// Input:
//  IN EFI_BLOCK_IO_PROTOCOL        *This,
//  IN UINT32                       MediaId,
//  IN EFI_LBA                      LBA,
//  IN UINTN                        BufferSize,
//  OUT VOID                        *Buffer
//
// Output:
//      EFI_STATUS
//
// Modified:
//
// Referrals: SdioAtaBlkReadWrite
//
// Notes:
//
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SdioBlkWrite(
    IN EFI_BLOCK_IO_PROTOCOL        *This,
    IN UINT32                       MediaId,
    IN EFI_LBA                      LBA,
    IN UINTN                        BufferSize,
    OUT VOID                        *Buffer
 )
{

    EFI_STATUS  Status;

    Status =  SdioAtaBlkReadWrite(This, MediaId, LBA, BufferSize, Buffer, 1);

    TRACE((-1,"Sdio Write: LBA : %lx ByteCount : %lx\n", LBA, BufferSize));

    return Status;

}

//<AMI_PHDR_START>
//---------------------------------------------------------------------------
//
// Procedure:   SdioAtaBlkReadWrite
//
// Description: Read/Write to/from Sdio ATA Device
//
// Input:
//  IN EFI_BLOCK_IO_PROTOCOL        *This,
//  IN UINT32                       MediaId,
//  IN EFI_LBA                      LBA,
//  IN UINTN                        BufferSize,
//  OUT VOID                        *Buffer,
//  BOOLEAN                         READWRITE
//
// Output:
//      EFI_STATUS
//
// Modified:
//
// Referrals: SdioReadWriteBusMaster, SdioReadWritePio
//
// Notes:
//  1. Check for validity of the input
//  2. Issue DMA or PIO Read/Write call.
//
//---------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SdioAtaBlkReadWrite (
    IN EFI_BLOCK_IO_PROTOCOL        *This,
    IN UINT32                       MediaId,
    IN EFI_LBA                      LBA,
    IN UINTN                        BufferSize,
    OUT VOID                        *Buffer,
    BOOLEAN                         READWRITE
)
{
    UINTN                       NumberOfBlocks;
    SDIO_DEVICE_INTERFACE       *SdioDevInterface = ((SDIO_BLOCK_IO *)This)->SdioDevInterface;
    EFI_BLOCK_IO_MEDIA          *BlkMedia = This->Media;
    EFI_STATUS                  Status=EFI_DEVICE_ERROR;
    UINT8                       Port=SdioDevInterface->PortNumber;
    UINTN                       BufferAddress;

    //
    // Check if Media ID matches
    //
    if (BlkMedia->MediaId != MediaId) return EFI_MEDIA_CHANGED;

    if (BufferSize == 0) return EFI_SUCCESS;

    //
    // If IoAlign values is 0 or 1, means that the buffer can be placed 
    // anywhere in memory or else IoAlign value should be power of 2. To be
    // properly aligned the buffer address should be divisible by IoAlign  
    // with no remainder. 
    // 
    (VOID *)BufferAddress = Buffer;
    if((BlkMedia->IoAlign > 1 ) && (BufferAddress % BlkMedia->IoAlign)) {
        return EFI_INVALID_PARAMETER;
    }
    
    // Check whether the block size is multiple of BlkMedia->BlockSize
    NumberOfBlocks = BufferSize % BlkMedia->BlockSize;
    if (NumberOfBlocks) {
        return EFI_BAD_BUFFER_SIZE;
    }

   // Check for Valid start LBA #
    if (LBA > BlkMedia->LastBlock) {
        return EFI_INVALID_PARAMETER;
    }

    NumberOfBlocks = BufferSize / BlkMedia->BlockSize;

    if (LBA + NumberOfBlocks > BlkMedia->LastBlock + 1) {
        return EFI_INVALID_PARAMETER;
    }

    if(!READWRITE) {
        Status=SDIOAPI_ReadCard(SdioDevInterface,Port,LBA, (UINT32)NumberOfBlocks, Buffer);
    } else {
        Status=SDIOAPI_WriteCard(SdioDevInterface,Port,LBA, (UINT32)NumberOfBlocks, Buffer);
    }

    return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SdioReset
//
// Description: Reset ATA device
//
// Input:
//  IN EFI_BLOCK_IO_PROTOCOL        *This,
//  IN BOOLEAN                      ExtendedVerification
//
// Output:
//  EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SdioReset (
    IN EFI_BLOCK_IO_PROTOCOL        *This,
    IN BOOLEAN                      ExtendedVerification
 )
{
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SdioBlkFlush
//
// Description: Flush the cache
// Input:
//  IN EFI_BLOCK_IO_PROTOCOL            *This,
//
// Output:
//  EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SdioBlkFlush(
    IN EFI_BLOCK_IO_PROTOCOL        *This
 )
{
    return EFI_SUCCESS;
}



//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
