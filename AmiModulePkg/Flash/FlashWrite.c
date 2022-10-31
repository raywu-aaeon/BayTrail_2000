//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.            **
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
// $Header: $
//
// $Revision: $
//
// $Date: $
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
#include <Token.h>
#include <Flash.h>
#include <FlashPart.h>
//AptioV
#define _DXE_FLASH_LIB_
#include <FlashElink.h>
//AptioV end
#if defined _DXE_FLASH_LIB_
#include <AmiDxeLib.h>
#endif
//----------------------------------------------------------------------
// component MACROs


//----------------------------------------------------------------------
// Module defined global variables
FLASH_PART      *FlashAPI = NULL;

// flash part list creation code must be in this order
typedef BOOLEAN (IDENTIFY)(
    volatile UINT8*     pBlockAddress,
    FLASH_PART          **Struct
    );

extern IDENTIFY FLASH_LIST EndOfInitList;

IDENTIFY* FlashList[] = {FLASH_LIST NULL};

// oem flash write enable/disable list creation code must be in this order
typedef VOID (OEM_FLASH_WRITE) (VOID);
extern OEM_FLASH_WRITE OEM_FLASH_WRITE_ENABLE_LIST EndOfOemFlashList;
extern OEM_FLASH_WRITE OEM_FLASH_WRITE_DISABLE_LIST EndOfOemFlashList;
OEM_FLASH_WRITE* OemFlashWriteEnable[] = {OEM_FLASH_WRITE_ENABLE_LIST NULL};
OEM_FLASH_WRITE* OemFlashWriteDisable[] = {OEM_FLASH_WRITE_DISABLE_LIST NULL};
UINT16 OemFlashValidTable[] = {OEM_FLASH_VALID_TABLE_LIST 0xFFFF};

// END flash part list creation code


//----------------------------------------------------------------------
// Module specific global variable
UINT16  gFlashId = 0;
UINT8   gbDeviceWriteState = FALSE;
UINT8   pFlashDeviceNumber[FLASH_PART_STRING_LENGTH];

//----------------------------------------------------------------------
// externally defined variables
extern VOID ChipsetFlashDeviceWriteEnable();
extern VOID ChipsetFlashDeviceWriteDisable();

//----------------------------------------------------------------------
// Function definitions
VOID OemFlashDeviceWriteEnable ( VOID );
VOID OemFlashDeviceWriteDisable ( VOID );

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   OemIsFlashValid
//
// Description: This function identifies the supported SPI flash parts and
//              returns TRUE. If flash part is not supported by this
//              module it will return FALSE.
//
// Input:       dFlashId    Present Flash Part ID
//
// Output:      None
//
// Return:      TRUE            Flash part is Valid.
//              FALSE           Flash part is not Valid.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN 
OemIsFlashValid(VOID)
{
    UINTN           i = 0;
    if (OemFlashValidTable[0] == 0xFFFF) return TRUE;
    for (i = 0; OemFlashValidTable[i] != 0xFFFF; i++) {
        if (gFlashId == OemFlashValidTable[i]) return TRUE;
    }
    return FALSE;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FlashInit
//
// Description: This function goes through the elinked list of identify
//      functions giving control until one part is identified properly
//
// Input:   *pBlockAddress - Address to access the flash part
//
// Output: None
//
// Returns: EFI_SUCCESS
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
FlashInit   (
    IN volatile UINT8*      pBlockAddress
)
{
    UINTN i;
    BOOLEAN found = FALSE;

    for(i=0; !found && FlashList[i]; i++)
    {
        found=FlashList[i](pBlockAddress, &FlashAPI);
    }
    if (found && !OemIsFlashValid()) FlashAPI = NULL;
    return EFI_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FlashEraseCommand
//
// Description: This function Chooses the correct flash part to call and then
//      initiates flash block erase command
//
// Input:   *pBlockAddress - Address to access flash part
//
// Output:  None
//
// Returns: None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
FlashEraseCommand   (
    IN volatile UINT8*      pBlockAddress
)
{
    if ( FlashAPI == NULL)
        FlashInit(pBlockAddress);

    // if FlashAPI is still NULL return
    if ( FlashAPI == NULL)
        return;

    FlashAPI->FlashEraseCommand(pBlockAddress);

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FlashIsEraseCompleted
//
// Description: This function chooses the correct flash part to call and
//      then checks current status of the last erase operation
//
// Input:   *pBlockAddress - Address to access flash part
//          *pError - Boolean that tells if fatal error occured
//          *pStatus - Status of the erase command
//
// Output:  *pError - Boolean that tells if fatal error occured
//          *pStatus - Status of the erase command
//
// Returns: TRUE - erase complete
//          FALSE - erase not completed
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
FlashIsEraseCompleted   (
    IN volatile UINT8     *pBlockAddress,
    OUT         BOOLEAN   *pError,
    OUT         UINTN     *pStatus)
{
    BOOLEAN Temp;

    if ( FlashAPI == NULL)
        FlashInit(pBlockAddress);

    Temp = FlashAPI->FlashIsEraseCompleted(pBlockAddress, pError, pStatus);

    return Temp;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FlashProgramCommand
//
// Description: This function chooses the correct flash part to call and
//      then initiates the program command
//
// Input:   *pBlockAddress - Address to access flash part
//          *Byte - pointer to data to write to the flash part
//          Length - The total amount of data that Byte points to
//
// Output:  None
//
// Return:  None
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
FlashProgramCommand (
    IN volatile UINT8*      pByteAddress,
    IN UINT8                *Byte,
    IN UINT32               Length
)
{
    volatile UINT8 *a=(volatile UINT8 *) pByteAddress;
    UINT32  TempLength = Length;
    UINT8   *DataPtr = Byte;

    if ( FlashAPI == NULL)
        FlashInit((volatile UINT8*)BLOCK(pByteAddress));

    // if FlashAPI is still NULL return
    if ( FlashAPI == NULL)
        return;

    do {
        FlashAPI->FlashProgramCommand(a, DataPtr, &TempLength);
        a += (Length - TempLength);
        DataPtr += (Length - TempLength);
        Length = TempLength;
    } while (TempLength != 0);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FlashReadCommand
//
// Description: This function chooses the correct flash part to call and
//      then initiates the program command
//
// Input:   *pBlockAddress - Address to access flash part
//          *Byte - pointer to data to write to the flash part
//          Length - The total amount of data that Byte points to
//
// Output:  None
//
// Return:  None
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
FlashReadCommand    (
    IN  volatile UINT8*     pByteAddress,
    OUT UINT8               *Byte,
    IN  UINT32              Length
)
{
    volatile UINT8 *a=(volatile UINT8 *) pByteAddress;
    UINT32  TempLength = Length;
    UINT8   *DataPtr = Byte;

    if ( FlashAPI == NULL)
        FlashInit((volatile UINT8*)BLOCK(pByteAddress));

    // if FlashAPI is still NULL return
    if ( FlashAPI == NULL)
        return;

    do {
        FlashAPI->FlashReadCommand(a, DataPtr, &TempLength);
        a += (Length - TempLength);
        DataPtr += (Length - TempLength);
        Length = TempLength;
    } while (TempLength != 0);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FlashIsProgramCompleted
//
// Description: This function chooses the correct flash part to call
//      and then checks current status of the last program operation
//
// Input:   *pBlockAddress - Address to access flash part
//          *Byte - values previously written to the Flash Device
//          Length - The amount of data that needs to be checked
//          *pError - Boolean that tells if fatal error occured
//          *pStatus - Status of the erase command
//
// Output: *pError - Boolean that tells if fatal error occured
//         *pStatus - Status of the erase command
//
// Return:  TRUE - Program completed, check pError for fatal error
//          FALSE - programming in progress
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
FlashIsProgramCompleted (
    IN  volatile UINT8*     pByteAddress,
    IN  UINT8               *Byte,
    IN  UINT32              Length,
    OUT BOOLEAN             *pError,
    OUT UINTN               *pStatus
)
{
//FlashProgramCommand implementation already checks the status
//So this function is no longer needed
//Just return success
    if (pError) *pError = FALSE;
    return TRUE;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FlashBlockWriteEnable
//
// Description: This function chooses the correct flash part to call and
//      then enables write operations(erase/programming) for a specific block
//
// Input:   *pBlockAddress - Address to access flash part
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
FlashBlockWriteEnable   (
    IN UINT8*               pBlockAddress
)
{
    if ( FlashAPI == NULL)
        FlashInit(pBlockAddress);

    // if FlashAPI is still NULL return
    if ( FlashAPI == NULL)
        return;

    FlashAPI->FlashBlockWriteEnable(pBlockAddress);
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FlashBlockWriteDisable
//
// Description: This function chooses the correct flash part to call and
//      then disables write operations(erase/programming) for a specific
//      block
//
// Input:   *pBlockAddress - Address to access flash part
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
FlashBlockWriteDisable  (
    IN UINT8*               pBlockAddress
)
{
    if ( FlashAPI == NULL)
        FlashInit(pBlockAddress);

    // if FlashAPI is still NULL return
    if ( FlashAPI == NULL)
        return;

    FlashAPI->FlashBlockWriteDisable(pBlockAddress);

}


//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FlashDeviceWriteEnable
//
// Description: This function chooses the correct flash part to call and
//      then enables write operation for a flash device
//
// Input:   None
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
FlashDeviceWriteEnable  (VOID)
{
    OemFlashDeviceWriteEnable ();

    ChipsetFlashDeviceWriteEnable();

    if ( FlashAPI == NULL)
        FlashInit((UINT8 *)FlashDeviceBase);

    // if FlashAPI is still NULL return
    if ( FlashAPI == NULL)
        return;

    FlashAPI->FlashDeviceWriteEnable();

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FlashDeviceWriteDisable
//
// Description: This function chooses the correct flash part to call and
//      then disables write operation for a flash device
//
// Input:   None
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
FlashDeviceWriteDisable (VOID)
{
    ChipsetFlashDeviceWriteDisable();

    if ( FlashAPI == NULL)
        FlashInit((UINT8 *)FlashDeviceBase);

    // if FlashAPI is still NULL return
    if ( FlashAPI == NULL)
        return;

    FlashAPI->FlashDeviceWriteDisable();

    OemFlashDeviceWriteDisable ();
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   OemFlashDeviceWriteEnable
//
// Description: This function goes through the elinked list of oem flash
//     write enable functions giving control.
//
// Input:   None
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
OemFlashDeviceWriteEnable (VOID)
{
    UINT8   i;

    if ( gbDeviceWriteState != TRUE ) {
        gbDeviceWriteState = TRUE;
        for(i = 0; OemFlashWriteEnable[i] != NULL; i++) 
            OemFlashWriteEnable[i]();
    }
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   OemFlashDeviceWriteDisable
//
// Description: This function goes through the elinked list of oem flash
//     write disable functions giving control.
//
// Input:   None
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
OemFlashDeviceWriteDisable (VOID)
{
    UINT8   i;

    if ( gbDeviceWriteState != FALSE ) {
        gbDeviceWriteState = FALSE;
        for(i = 0; OemFlashWriteDisable[i] != NULL; i++) 
            OemFlashWriteDisable[i]();
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FlashVirtualFixup
//
// Description: Fixup global data for for a virtual address space.
//      This routine must be called by the library consumer in the
//      EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE event handler
//
// Input:   EFI_RUNTIME_SERVICES *pRS - pointer to the Runtime Services Table
//
// Output:  None
//
// Return:  None
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
FlashVirtualFixup   (
    IN EFI_RUNTIME_SERVICES *pRS
)
{
    static BOOLEAN  bFlashVirtual = FALSE;
    VOID            **p;
    UINT8           i;

    //if FlashAPI is NULL, nothing to fix up
    //TODO: Current implementation of Identify routines
    //will not work in virtual mode since absolute addresses are used
    //That's Ok for now since current flash library consumers
    //call flash routines at boot time, so we should have FlashAPI
    //initialized at this point
    if (!FlashAPI) return;
    
    // This function gets called from Nvramdxe.c and flash.c, do anything when 
    // the function is called second time.
    if (bFlashVirtual == TRUE) return;
    else bFlashVirtual = TRUE;

    //Do device specific fixups
    FlashAPI->FlashVirtualFixup(pRS);

    //Fixup FlashAPI member functions
    for(p = (VOID**)FlashAPI; p < (VOID**)(FlashAPI + 1); p++)
        pRS->ConvertPointer(0, p);

    //Fixup FlashAPI pointer
    pRS->ConvertPointer(0, (VOID**)&FlashAPI);

    //convert FeatureSpaceBase address
    pRS->ConvertPointer(0, (VOID**)&FwhFeatureSpaceBase);

    //convert FlashDeviceBase address
    pRS->ConvertPointer(0, (VOID**)&FlashDeviceBase);

    //convert FlashBlockSize address
    pRS->ConvertPointer(0, (VOID**)&FlashBlockSize);

    //Fixup OemFlashWriteEnable list
    for(i = 0; OemFlashWriteEnable[i] != NULL; i++)
        pRS->ConvertPointer(0, (VOID**)&OemFlashWriteEnable[i]);

    //Fixup OemFlashWriteDisable list
    for(i = 0; OemFlashWriteDisable[i] != NULL; i++)
        pRS->ConvertPointer(0, (VOID**)&OemFlashWriteDisable[i]);
}

#if defined _DXE_FLASH_LIB_
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   GetFlashPartInfomation
//
// Description: 
//
// Input:   *pBlockAddress - Address to access the flash part
//          *Buffer - Buffer to write the flash part number string.
//
// Output: None
//
// Returns: EFI_SUCCESS
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
GetFlashPartInfomation  (
    IN  UINT8*          pBlockAddress,
    IN  UINT8*          Buffer
)
{
    UINT8           bFlashStringSign[4] = {'$','F','P','S'};
    
    if ( FlashAPI == NULL) {
        MemSet( &pFlashDeviceNumber, FLASH_PART_STRING_LENGTH, 0 );
        FlashInit(pBlockAddress);
    }        
    if ((FlashAPI != NULL) && ( FlashAPI->FlashPartNumber != NULL )) {
        MemCpy ( Buffer, bFlashStringSign, 4 );
        Buffer += sizeof (UINT32);
        *(UINT32*)Buffer = FLASH_PART_STRING_LENGTH;
        Buffer += sizeof (UINT32);
        MemCpy ( Buffer, FlashAPI->FlashPartNumber, FLASH_PART_STRING_LENGTH );
    }
}    
#endif  // #ifdef _DXE_FLASHLIB_
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
