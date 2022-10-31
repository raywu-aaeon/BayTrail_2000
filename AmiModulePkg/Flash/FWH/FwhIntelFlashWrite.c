//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2009, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//**********************************************************************
// $Header: /Alaska/SOURCE/Flash_Combined_2/Core/FWH/FwhIntelFlashWrite.c 16    12/23/09 6:12a Calvinchen $
//
// $Revision: 16 $
//
// $Date: 12/23/09 6:12a $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    FlashWrite.c
//
// Description: Flash update routines
//
//<AMI_FHDR_END>
//**********************************************************************

//----------------------------------------------------------------------
// Includes
#include <Efi.h>
#include <FlashPart.h>
#include <AmiDxeLib.h>
#include <Library/TimerLib.h>
#include "Token.h"


//----------------------------------------------------------------------
// define local MACROS

//Flash Part Specific Tokens
#define VENDOR_ID               0x89    // Intel Manufacturers ID
#define DEVICE_ID0              0xad    // 512K Firmware Hub
#define DEVICE_ID1              0xac    // 1 Meg Firmware Hub
#define Intel_82802AB           0xad89  // Intel 4Mb Firmware Hub
#define Intel_82802AC           0xac89  // Intel 8Mb Firmware Hub

#define VENDOR_ID2              0x1f    // Atmel Manufacturers ID
#define DEVICE_ID3              0xe1    // AT49LW080
#define ATMEL_49LW080           0xe11f  // ATMEL 8Mb Firmware Hub

#define VENDOR_ID_ST            0x20    // ST Manufacturers ID
#define DEVICE_ID_ST0           0x2d    // M50FW080
#define DEVICE_ID_ST1           0x2e    // M50FW016 - 2MB
#define STM_50FW080             0x2d20  // STM 8Mb Firmware Hub
#define STM_50FW016             0x2e20  // STM 16Mb Firmware Hub

#define SST_49LF016             0x5cbf  // SST 16Mb Firmware Hub

#define READ_ARRAY_CMD          0xff
#define RD_STATUS_CMD           0x70
#define CLR_STATUS_CMD          0x50
#define ERASE_SET_CMD           0x20
#define ERASE_CNF_CMD           0xd0
#define PRG_SETUP_CMD           0x40
#define SECTOR_ERASE_SET_CMD    0x30 // SST 49LF016 only

#define RD_ID_CODE              0x90

// Intel Status Register Bits
#define VPP_LOW                 0x08
#define PROGRAM_FAIL            0x10
#define ERASE_FAIL              0x20
#define WSM_BUSY                0x80

// Intel Lock Commands
#define UNLOCK                  0
#define WRITE_LOCK              1

#define SECTOR_SIZE_4KB         0x1000  // Common 4kBytes sector size
#define SECTOR_SIZE_64KB        0x10000  // Common 64kBytes sector size

//----------------------------------------------------------------------------
// Module level global data
extern FLASH_PART   *FlashAPI;
extern  UINT8       pFlashDeviceNumber[FLASH_PART_STRING_LENGTH];

//----------------------------------------------------------------------------
// Function Prototypes
VOID
IntelFlashEraseCommand  (
    volatile UINT8  *pBlockAddress
    );
VOID
IntelFlashReadCommand   (
    volatile UINT8  *pByteAddress,
    UINT8           *Data,
    UINT32          *Length
    );
VOID
IntelFlashProgramCommand    (
    volatile UINT8  *pByteAddress,
    UINT8           *Data,
    UINT32          *Length
    );
BOOLEAN
IntelFlashIsEraseCompleted  (
    volatile UINT8  *pBlockAddress,
    BOOLEAN         *pError,
    UINTN           *pStatus
    );
BOOLEAN
IntelFlashIsProgramCompleted    (
    volatile UINT8  *pByteAddress,
    UINT8           *Byte,
    UINT32          Length,
    BOOLEAN         *pError,
    UINTN           *pStatus
    );
VOID
IntelFlashBlockWriteEnable  (
    UINT8*          pBlockAddress
    );
VOID
IntelFlashBlockWriteDisable (
    UINT8*          pBlockAddress
    );
VOID
IntelFlashDeviceWriteEnable (
    VOID
    );
VOID
IntelFlashDeviceWriteDisable    (
    VOID
    );
VOID
IntelFlashVirtualFixup(EFI_RUNTIME_SERVICES *pRS);


//========================================================================
// Local Variable definitions
UINT8   gAtmel = 0;
UINT8   gbEraseSetCmd = ERASE_SET_CMD;
// Flash Part Data structure fo the intel 82802ACC
FLASH_PART mIntelFirmwareHub =
    {
        IntelFlashReadCommand,
        IntelFlashEraseCommand,
        IntelFlashProgramCommand,
        IntelFlashIsEraseCompleted,
        IntelFlashIsProgramCompleted,
        IntelFlashBlockWriteEnable,
        IntelFlashBlockWriteDisable,
        IntelFlashDeviceWriteEnable,
        IntelFlashDeviceWriteDisable,
        IntelFlashVirtualFixup,
        1,                  // Number of bytes to program to the
                            // Flash part in each program command
        SECTOR_SIZE_64KB,   // Dummy value to hold place - only used in SPI
        NULL                // Flash Part Number Pointer
    };



//========================================================================
// Function Definitions
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFwhCmdDelay
//
// Description: This function resets the Sst flash part
//
// Input:   None
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
IntelFwhCmdDelay (VOID)
{
//-    IoWrite8 ( 0xeb, 0x55 );
//-    IoWrite8 ( 0xeb, 0xaa );
    MicroSecondDelay(10);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFwhResetFlash
//
// Description: This function resets the Sst flash part
//
// Input:   None
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
VOID
IntelFwhResetFlash  (
    IN volatile UINT8*      pAddress
)
{
    *pAddress = READ_ARRAY_CMD;// Return to read mode
    IntelFwhCmdDelay ();
    *pAddress = CLR_STATUS_CMD;// clear status
    IntelFwhCmdDelay ();
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFwhOperationCompleted
//
// Description:
//      This function verifies whether the command sent to the FWH part
//      has completed and returns the status of the command
//
// Input:
//  IN volatile UINT8*  pAddress    Location to check the device status
//
// Output:
//      EFI_SUCCESS -
//      EFI_TIMEOUT -
//      EFI_DEVICE_ERROR -
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
EFI_STATUS
IntelFwhOperationCompleted  (
    IN volatile UINT8*      pAddress
)
{
    UINT8                   bFwhStatus;
    UINT32                  dTimeout = FLASH_RETRIES * 0x10000;

    do {
        *pAddress = RD_STATUS_CMD;          // read status.
        IntelFwhCmdDelay ();
        bFwhStatus = *pAddress;
        if ( bFwhStatus & WSM_BUSY ) {
            if ( bFwhStatus & ( VPP_LOW + PROGRAM_FAIL + ERASE_FAIL ) )
                return EFI_DEVICE_ERROR;
            else return EFI_SUCCESS;
        }
        dTimeout--;
    } while ( dTimeout != 0 );
    return EFI_TIMEOUT;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFlashEraseCommand
//
// Description: This API function erases a block in the flash. Flash model
//              specific code will branch out from this routine
//
// Input:       pBlockAddress   Block that need to be erased
//
// Output:      Nothing
//
// Returns:     None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
VOID
IntelFlashEraseCommand  (
    IN volatile UINT8*      pBlockAddress
)
{
    EFI_STATUS          Status;
    UINT8               bFlashRetry;
    UINTN               nSectorAddr;
    UINT16              wNumSectors, wSectorCount;

    IntelFwhResetFlash( pBlockAddress );
    wNumSectors = ( FlashBlockSize / FlashAPI->FlashSectorSize );
    for ( wSectorCount = 0; wSectorCount < wNumSectors; wSectorCount++ ) {
        nSectorAddr = (UINTN)( wSectorCount * FlashAPI->FlashSectorSize );
        for ( bFlashRetry = 0; bFlashRetry < FLASH_RETRIES; bFlashRetry++ ) {
            *(UINT8*)( (UINTN)pBlockAddress + nSectorAddr ) = gbEraseSetCmd;
            IntelFwhCmdDelay ();
            *(UINT8*)( (UINTN)pBlockAddress + nSectorAddr ) = ERASE_CNF_CMD;
            IntelFwhCmdDelay ();
            Status = IntelFwhOperationCompleted ( \
                                (UINT8*)((UINTN)pBlockAddress + nSectorAddr) );
            IntelFwhResetFlash( (UINT8*)((UINTN)pBlockAddress + nSectorAddr) );
            if ( Status != EFI_SUCCESS ) continue;
            if (*(volatile UINT8*)((UINTN)pBlockAddress + nSectorAddr) != 0xFF)
                Status = EFI_DEVICE_ERROR;
            else {
                Status = EFI_SUCCESS;
                break;
            }
        }
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFlashIsEraseCompleted
//
// Description: This function verifies whether the block erase
//              command is completed and returns the status of the command
//
// Input:       *pBlockAddress  Location of the block erase
//
// Output:      *pError     True on error and false on success
//              *Status     Error status code (0 - Success)
//
// Return:      TRUE        If operation completed successfully
//                          *pError = FALSE, *pStatus = 0
//              FALSE       If operation failed
//                          *pError = TRUE, *pStatus = error status
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
BOOLEAN
IntelFlashIsEraseCompleted(
    IN  volatile UINT8*     pBlockAddress,
    OUT BOOLEAN             *pError,
    OUT UINTN               *pStatus)
{
    UINT32                      dNumBytes;

    for ( dNumBytes = 0; dNumBytes < FlashBlockSize; dNumBytes++ ) {
        if ( *(volatile UINT8*)( pBlockAddress + dNumBytes ) != 0xFF ) {
            if ( pError )  *pError = TRUE;
            if ( pStatus ) *pStatus = EFI_DEVICE_ERROR;
            return FALSE;
        }
    }
    if ( pError ) *pError = FALSE;
    if ( pStatus ) *pStatus = EFI_SUCCESS;
    return TRUE;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFlashProgramCommand
//
// Description: This function programs a byte data to the specified location
//
// Input:   *pByteAddress   Location where the data to be written
//          *Byte - Byte to be written
//          *Length - Number of bytes to write
//
// Output:  *Length - Number of bytes still left to write
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
VOID
IntelFlashProgramCommand    (
    IN      volatile UINT8* pByteAddress,
    IN      UINT8           *Byte,
    IN OUT  UINT32          *Length
)
{
    UINT8               bFlashRetry;
    EFI_STATUS          Status;

    if (*pByteAddress != *Byte) {
        IntelFwhResetFlash( pByteAddress );
        for ( bFlashRetry = 0; bFlashRetry < FLASH_RETRIES; bFlashRetry++ ) {
            *pByteAddress = PRG_SETUP_CMD; // Issue program command
            IntelFwhCmdDelay ();
            *pByteAddress = *Byte; // Program a byte
            IntelFwhCmdDelay ();
            // Check for completion of the program operation
            Status = IntelFwhOperationCompleted( pByteAddress );
            if ( Status != EFI_SUCCESS ) continue;
            *pByteAddress = READ_ARRAY_CMD; // read mode
            if ( *pByteAddress != *Byte ) Status = EFI_DEVICE_ERROR;
            else {
                Status = EFI_SUCCESS;
                break;
            }
        }
    } else Status = EFI_SUCCESS;
    *Length = *Length - 1;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFlashIsProgramCompleted
//
// Description: This function verifies whether the program (page or byte)
//              command is completed and returns the status of the command
//
// Input:       *pByteAddress   Location of the program command
//              Byte            Last data byte written
//
// Output:      *pError         True on error and false on success
//              *Status         Error status code (0 - Success)
//
// Return:      TRUE        If operation completed successfully
//                          *pError = FALSE, *pStatus = 0
//              FALSE       If operation failed
//                          *pError = TRUE, *pStatus = error status
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
BOOLEAN
IntelFlashIsProgramCompleted    (
    IN  volatile UINT8*     pByteAddress,
    IN  UINT8               *Byte,
    IN  UINT32              Length,
    OUT BOOLEAN             *pError,
    OUT UINTN               *pStatus)
{
    UINT32                      dNumBytes;
    UINT8                       bByte;

    for ( dNumBytes = 0; dNumBytes < Length; dNumBytes++ ) {
        bByte = * ( Byte + dNumBytes );
        if ( bByte != *(volatile UINT8*)( pByteAddress + dNumBytes ) ) {
            if ( pError )  *pError = TRUE;
            if ( pStatus ) *pStatus = EFI_DEVICE_ERROR;
            return FALSE;
        }
    }
    if ( pError ) *pError = FALSE;
    if ( pStatus ) *pStatus = EFI_SUCCESS;
    return TRUE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFlashReadCommand
//
// Description: This function programs a byte data to the specified location
//
// Input:   *pByteAddress   Location where the data to be written
//          Bytes - data to be written.
//          Length - number of bytes to write
//
// Output:  Length - number of bytes that were not written
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
VOID
IntelFlashReadCommand   (
    IN      volatile UINT8* pByteAddress,
    IN OUT  UINT8           *Byte,
    IN OUT  UINT32          *Length
)
{
    UINT32              dNumBytes = 0;
    
    // Changes for SMIFlash module label "4.6.3.6_SMIFLASH_12" or later.
    for ( dNumBytes = 0; dNumBytes < *Length ; dNumBytes++ )
        *( Byte + dNumBytes ) = *(UINT8*)((UINT32)pByteAddress + dNumBytes );
    *Length = 0;
    return ;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetBlockLock
//
// Description: This function programs a page of data at a time
//
// Input:   *pBlockAddress - This is location where the data
//                  is to be written
//          LockState - Value to use to set the Lock register for the
//                  block defined by pBlockAddress
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
void
SetBlockLock    (
    IN volatile UINT8*  pBlockAddress,
    IN UINT8            LockState
)
{
    // Update the block lock register
    ((UINT8 *)((UINTN)pBlockAddress - FlashDeviceBase + \
                            FwhFeatureSpaceBase))[2] = LockState;
    ((UINT8 *)((UINTN)pBlockAddress - FlashDeviceBase + \
                            FwhFeatureSpaceBase + 0x8000))[2] = LockState;
    ((UINT8 *)((UINTN)pBlockAddress - FlashDeviceBase + \
                            FwhFeatureSpaceBase + 0xA000))[2] = LockState;
    ((UINT8 *)((UINTN)pBlockAddress - FlashDeviceBase + \
                            FwhFeatureSpaceBase + 0xC000))[2] = LockState;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFlashBlockWriteEnable
//
// Description: This function contains any flash specific code need to
//      enable a particular flash block write
//
// Input:   None
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
VOID
IntelFlashBlockWriteEnable  (
    IN UINT8*           pBlockAddress
)
{
    SetBlockLock(pBlockAddress, UNLOCK);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFlashBlockWriteDisable
//
// Description: This function contains any flash specific code need to
//              disable a particular flash block write
//
// Input:       None
//
// Output:      None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
VOID
IntelFlashBlockWriteDisable (
    IN UINT8*               pBlockAddress
)
{
    SetBlockLock(pBlockAddress, WRITE_LOCK);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFlashDeviceWriteEnable
//
// Description: This function contains any flash specific code need to
//              enable flash write
//
// Input:       None
//
// Output:      None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
VOID
IntelFlashDeviceWriteEnable (VOID)
{
    //We don't have to do anything here because
    //Flash Device is write enabled by the South Bridge driver
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFlashDeviceWriteDisable
//
// Description: This function contains any flash specific code need to
//              disable flash write
//
// Input:       None
//
// Output:      None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
static
VOID
IntelFlashDeviceWriteDisable (VOID)
{
    //We don't have to do anything here because
    //we always keep flash device in the write enabled state
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IntelFlashVirtualFixup
//
// Description: This function will be invoked by the core to convert
//              runtime pointers to virtual address
//
// Input:       *pRS    Pointer to runtime services
//
// Output:      None
//
// Return:  None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

static
VOID
IntelFlashVirtualFixup  (
    IN EFI_RUNTIME_SERVICES *pRS
)
{
//  // Following is an example code for virtual address conversion
//  pRS->ConvertPointer(0, (VOID**)&FlashDeviceBase);

    return;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   mIntelIdentify
//
// Description: This function identifies the supported LPC flash parts and
//              returns appropriate flash device API pointer. If flash part is
//              not supported by this module it will return FALSE.
//
//
// Input:   pBlockAddress   Block address of the flash part. Can be used to
//                              send ID command
//
// Output:  **FlashApi      Pointer to hold the returned flash API
//
// Return:  TRUE            If flash part is supported, FlashApi contains
//                              routines to handle the flash requests
//          FALSE           Flash part is not supported
//
// Note:    This routine is part of the global flash init list. Make sure
//          it is properly linked to the init list "FlashList" (in SDL file)
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
mIntelIdentify  (
    IN  volatile UINT8*     pBlockAddress,
    OUT FLASH_PART          **Struct
)
{
    UINT8 VID, DID;

    SetBlockLock(pBlockAddress, UNLOCK);

    *pBlockAddress = READ_ARRAY_CMD;// Return to read mode
    IntelFwhCmdDelay ();

    *pBlockAddress = RD_ID_CODE;// Set to read ID code mode
    IntelFwhCmdDelay ();
    VID = *pBlockAddress;
    DID = *(pBlockAddress + 1);

    *pBlockAddress = READ_ARRAY_CMD;// Return to read mode

    switch ( (DID << 8) + VID ) {

        case ATMEL_49LW080 :
            MemCpy ( pFlashDeviceNumber, "ATMEL 49LW080", 13 );
            gAtmel = 1;
            break;
        case Intel_82802AB :
        case Intel_82802AC :
            MemCpy ( pFlashDeviceNumber, "Intel N82802AB/C", 16 );
            break;
        case STM_50FW080 :
        case STM_50FW016 :
            MemCpy ( pFlashDeviceNumber, "ST M25FW080/016", 15 );
            break;
        case SST_49LF016 :
            gbEraseSetCmd = SECTOR_ERASE_SET_CMD;
            mIntelFirmwareHub.FlashSectorSize = SECTOR_SIZE_4KB;
            MemCpy ( pFlashDeviceNumber, "SST 49LF016C", 12 );
            break;
        default:
            return FALSE;
    }
    mIntelFirmwareHub.FlashPartNumber = pFlashDeviceNumber;
    *Struct = &mIntelFirmwareHub;
    return TRUE;
}
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2009, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
