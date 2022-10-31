//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
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
// Name:    SMIFlash.c
//
// Description: SMIFlash Driver.
//
//<AMI_FHDR_END>
//**********************************************************************
#include <AmiDxeLib.h>
#include <Token.h>
#include <SMIFlashELinks.h>
#if AMIUSB_SUPPORT == 1
#include <Protocol\AmiUsbController.h>
#endif
//----------------------------------------------------------------------
// component MACROs

//----------------------------------------------------------------------
// Module defined global variables

//----------------------------------------------------------------------
// Module specific global variable
// oem flash write enable/disable list creation code must be in this order
typedef VOID (SMIFLASH_INIT) (VOID);
extern SMIFLASH_INIT SMIFLASH_NOT_IN_SMM_LIST EndOfNotInSmmList;
SMIFLASH_INIT* SMIFlashNotInSmm[] = {SMIFLASH_NOT_IN_SMM_LIST NULL};

//----------------------------------------------------------------------
// externally defined variables

//----------------------------------------------------------------------
// Function definitions
#if AMIUSB_SUPPORT == 1
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   GetUsbProtocol
//
// Description:
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
GetUsbProtocolPoint(
    IN EFI_EVENT   Event,
    IN VOID        *Context
)
{
    EFI_GUID            AmiGlobalVariableGuid = AMI_GLOBAL_VARIABLE_GUID;
    EFI_USB_PROTOCOL    *gSMIAmiUsb = NULL;
    EFI_STATUS          Status;

    Status = pBS->LocateProtocol(&gEfiUsbProtocolGuid, NULL, &gSMIAmiUsb);
    if (EFI_ERROR(Status)) return;
    pBS->CloseEvent (Event);
    pRS->SetVariable (  L"USB_POINT",
                        &AmiGlobalVariableGuid,
                        EFI_VARIABLE_RUNTIME_ACCESS |
                        EFI_VARIABLE_BOOTSERVICE_ACCESS,
                        sizeof(UINTN),
                        &(UINTN)gSMIAmiUsb );
    return;
}
#endif //AMIUSB_SUPPORT
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   SmiFlashDxeEntry
//
// Description:
//
// Input:
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS 
SmiFlashDxeEntry (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
{
    UINTN       i;
    EFI_EVENT   EvtProtocolEvt = NULL;
    VOID        *RgnUsbProtocol = NULL;

    InitAmiLib(ImageHandle, SystemTable);

#if AMIUSB_SUPPORT == 1
    // Register USB Protocol callback for saving USB Protocol address 
    // for runtime used.
    RegisterProtocolCallback (&gEfiUsbProtocolGuid, \
                              GetUsbProtocolPoint, \
                              NULL, \
                              &EvtProtocolEvt, \
                              &RgnUsbProtocol  );

    // Call Callback function for checking USB Protocol is installed or not.
    GetUsbProtocolPoint (EvtProtocolEvt, NULL);
#endif //AMIUSB_SUPPORT

    for (i = 0; SMIFlashNotInSmm[i] != NULL; SMIFlashNotInSmm[i++]());
    return EFI_SUCCESS;
}
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
