//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//
// $Header:  $
//
// $Revision: $
//
// $Date: $
//
//**********************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:  SbDxeBoard.c
//
// Description: This file contains DXE stage board component code for
//              Template SB
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


// Module specific Includes
#include <Efi.h>
#include <DXE.h>
#include <token.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <SbDxeInitElink.h>


// Produced Protocols

// GUID Definitions

// Portable Constants

// Function Prototypes

// PPI interface definition


// Protocols that are installed

// Function Definition

typedef VOID (SAVE_RESTORE_CALLBACK)( BOOLEAN Save );

extern SAVE_RESTORE_CALLBACK SAVE_RESTORE_CALLBACK_LIST EndOfList;
SAVE_RESTORE_CALLBACK* SaveRestoreCallbackList[] = { SAVE_RESTORE_CALLBACK_LIST NULL };

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SaveRestoreRegisters
//
// Description: This function calls registered callbacks to save/restore registers
//              value in timer interrupt routine
//
// Input:       BOOLEAN Save - if TRUE - save registers, FALSE - restore back
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID SaveRestoreRegisters(
    IN BOOLEAN Save
)
{
    UINTN i;
    for(i = 0; SaveRestoreCallbackList[i] != NULL; i++) 
        SaveRestoreCallbackList[i](Save);
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbDxeBoardInit
//
// Description: This function initializes the board specific component in
//              in the chipset South bridge
//
// Input:       ImageHandle - Image handle
//              SystemTable - Pointer to the system table
//
// Output:      Return Status based on errors that occurred while waiting for
//              time to expire.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SbDxeBoardInit (
    IN EFI_HANDLE                 ImageHandle,
    IN EFI_SYSTEM_TABLE           *SystemTable
)
{
    InitAmiLib( ImageHandle, SystemTable );

    return EFI_SUCCESS;
}


//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
