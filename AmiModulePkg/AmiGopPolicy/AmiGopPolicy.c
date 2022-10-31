//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/AmiGopPolicy/AmiGopPolicy.c 2     7/26/12 7:30a Josephlin $
//
// $Revision: 2 $
//
// $Date: 7/26/12 7:30a $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:        AmiGopPolicy.c
//
// Description:	AmiGopPolicy output initialization in the DXE stage.
//
//<AMI_FHDR_END>
//**********************************************************************

//----------------------------------------------------------------------
// Include(s)
//----------------------------------------------------------------------

#include <Efi.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include <Setup.h>
#include <token.h>

#include <Protocol\DevicePath.h>

//----------------------------------------------------------------------
// Constant, Macro and Type Definition(s)
//----------------------------------------------------------------------
// Constant Definition(s)

// Macro Definition(s)

// Type Definition(s)

// Function Prototype(s)

//----------------------------------------------------------------------
// Variable and External Declaration(s)
//----------------------------------------------------------------------
// Variable Declaration(s)

// GUID Definition(s)

// Protocol Definition(s)

// External Declaration(s)

// Function Definition(s)

VOID ConnectDevicePath(IN EFI_DEVICE_PATH_PROTOCOL *pPath);

//----------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Name:        ConnectAmiGopPolicyConOut
//
// Description: None.
//
// Input:       None.
//
// Output:      None.
//
// Notes:       None.
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ConnectAmiGopPolicyConOut (VOID)
{
	EFI_STATUS                  Status;
    EFI_GUID                    gEfiGlobalVariableGuid = EFI_GLOBAL_VARIABLE;
    EFI_DEVICE_PATH_PROTOCOL    *GopDevicePath = NULL;
    UINTN                       VariableSize = 0;

    Status = GetEfiVariable (
             L"AmiGopOutputDp",
             &gEfiGlobalVariableGuid,
             NULL,
             &VariableSize,
             &GopDevicePath);
    if ((EFI_ERROR(Status)) || (GopDevicePath == NULL)) return ;

    ConnectDevicePath(GopDevicePath);

    return;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
