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
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
// Revision History
// ----------------
// $Log: $
// 
// 
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	MeudMsg.c
//
// Description:
//
//<AMI_FHDR_END>
//**********************************************************************
#include "Efi.h"
#include "token.h"
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include "../OFBD.h"
#include "MEUD.h"
#include <Protocol/SimpleTextOut.h>

extern EFI_BOOT_SERVICES *gBS;
extern EFI_SYSTEM_TABLE *gST;
extern EFI_RUNTIME_SERVICES *gRT;
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	ShowMEUDErrors
//
// Description:	Show Message on Screen If Ignition FW runs on Factory Default
//
// Input:
//      VOID
//
// Output:      BOOLEAN. Should return TRUE if the screen was used to
//              ask password; FALSE if the screen was not used to ask
//              password.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN ShowMEUDErrors(VOID)
{
    EFI_GUID       gMEUDErrorGuid = MEUD_ERROR_GUID;
    UINTN          DataSize;
    UINT8          Data;
    EFI_STATUS     Status;
    DataSize = sizeof(UINT8);

    Status = gRT->GetVariable( L"ShowMEUDFailMSG", &gMEUDErrorGuid,
        NULL,
        &DataSize, &Data);
    if(EFI_ERROR(Status))
        return FALSE;


    gST->ConOut->OutputString(gST->ConOut, 
           L"\n\rMe Ignition FW Data Paritition Usage :" );
    if(Data & 0x4)
        gST->ConOut->OutputString(gST->ConOut, \
               L"[Runtime Image]\n\r" );
    else
        gST->ConOut->OutputString(gST->ConOut, \
               L"[Factory Default]\n\r" );


    gST->ConOut->OutputString(gST->ConOut, 
           L"Me Ignition FW Code Paritition Usage :" );
    if(Data & 0x8)
        gST->ConOut->OutputString(gST->ConOut, \
               L"[Runtime Image]\n\r" );
    else
        gST->ConOut->OutputString(gST->ConOut, \
               L"[Factory Default]\n\r" );

    return FALSE;

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
