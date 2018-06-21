//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/**
 * @file
 * Source file for the DXE driver. This file contains the code to trigger the 
 * SWSMI that will save the boot scripts into SMM. Depending on the boot path, 
 * either a legacy boot event or an Exit Boot Services notification will notify 
 * the trigger function
 */
#include <AmiDxeLib.h>
#include <Token.h>
#include <Protocol/SmmControl2.h>

/**
 * Callback function called when either Exit Boot Services is called, or a legacy 
 * boot event is raised. This function will use the SmmControl protocol to trigger 
 * a SWSMI.
 * 
 * @param Event Event that caused this function to be called
 * @param Context  Context for the event that triggered this function
**/
VOID GenerateSaveBootScriptSwSmi (
    IN EFI_EVENT Event, IN VOID *Context
){
    static BOOLEAN BootScriptSaved = FALSE;
    EFI_STATUS Status;
    EFI_SMM_CONTROL2_PROTOCOL  *SmmControl;
    UINT8 SwSmiValue = BOOT_SCRIPT_SAVE_SW_SMI_VALUE;
    
    if (BootScriptSaved){
        pBS->CloseEvent(Event);
        return;
    }
    Status = pBS->LocateProtocol (&gEfiSmmControl2ProtocolGuid, NULL, (VOID **)&SmmControl);
    if (EFI_ERROR(Status)) return;
    SmmControl->Trigger (SmmControl, &SwSmiValue, NULL, FALSE, 0);
    BootScriptSaved = TRUE;
    pBS->CloseEvent(Event);
}

/**
 * Entry point for the DXE driver. Entry point will register a legacy boot event notification function, 
 * and a Exit Boot Services event handler.  The same function is called for the legacy boot event and 
 * the exit boot services notification function.
 * 
 * @param ImageHandle The handle that corresponds this this loaded DXE driver
 * @param SystemTable Pointer to the EFI System Table
 * 
 * @return EFI_STATUS
**/
EFI_STATUS EFIAPI BootScriptHideDxeEntryPoint(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable){
    EFI_EVENT Event;
    
    InitAmiLib(ImageHandle,SystemTable);
    // We're using TPL_NOTIFY here (as oppose to TPL_CALLBACK) to make sure our callback is called prior to NVRAM driver callback.
    // Otherwise we may be unable to read boot time variable in our SMI handler.
    CreateLegacyBootEvent(TPL_NOTIFY, &GenerateSaveBootScriptSwSmi, NULL, &Event);
    pBS->CreateEvent(
        EVT_SIGNAL_EXIT_BOOT_SERVICES,TPL_NOTIFY,
        &GenerateSaveBootScriptSwSmi, NULL, &Event
    );
    return EFI_SUCCESS;
}
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
