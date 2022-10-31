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
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        ElinkPei.c
//
// Description: This file contains the functions used for AMI Elink
//
//<AMI_FHDR_END>
//*************************************************************************

//-------------------------------------------------------------------------
// Include(s)
//-------------------------------------------------------------------------

#include <Library/BaseLib.h>
#include <Uefi/UefiBaseType.h>

//-------------------------------------------------------------------------
// Variable and External Declaration(s)
//-------------------------------------------------------------------------
// Variable Declaration(s)


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ElinkEntryPoint
//
// Description: This function is the entry point for this PEI.
//
// Input:       FfsHeader   - Pointer to the FFS file header
//              PeiServices - Pointer to the PEI services table
//
// Output:      EFI_SUCCESS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
EFIAPI
ElinkEntryPoint(
    IN        EFI_PEI_FILE_HANDLE   FileHandle,
    IN CONST  EFI_PEI_SERVICES      **PeiServices
)
{
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
