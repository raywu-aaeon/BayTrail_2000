//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************

//**********************************************************************
//<AMI_FHDR_START>
//
// Name: TerminalName.c
//
// Description: EFI_COMPONENT_NAME_PROTOCOL: GetDriverName(), GetControllerName().
//
//<AMI_FHDR_END>
//**********************************************************************

#include "Terminal.h"

BOOLEAN LanguageCodesEqual(
    IN CONST CHAR8* LangCode1, 
    IN CONST CHAR8* LangCode2
);

CHAR16 gName[] = L"AMI Terminal Driver";

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure: TerminalGetDriverName
//
// Description: Gets the driver name.
//
// Input:
//  IN EFI_COMPONENT_NAME_PROTOCOL  *This
//  IN CHAR8                        *Language
//  OUT CHAR16                      **DriverName
//
// Output:
//  EFI_STATUS
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS TerminalGetDriverName (
    IN EFI_COMPONENT_NAME_PROTOCOL  *This,
    IN CHAR8                        *Language,
    OUT CHAR16                      **DriverName
)
{
    if (!Language || !DriverName) return EFI_INVALID_PARAMETER;

    if ( !LanguageCodesEqual( Language, This->SupportedLanguages) ) {
        return EFI_UNSUPPORTED;
    }

    *DriverName = gName;
    return EFI_SUCCESS;
}

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure: TerminalGetControllerName
//
// Description: returns EFI_UNSUPPORTED. 
//
// Input:
//  IN EFI_COMPONENT_NAME_PROTOCOL  *This
//  IN EFI_HANDLE                   ControllerHandle
//  IN EFI_HANDLE                   ChildHandle OPTIONAL
//  IN CHAR8                        *Language
//  OUT CHAR16                      **ControllerName
//
// Output:
//  EFI_UNSUPPORTED
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS TerminalGetControllerName (
    IN EFI_COMPONENT_NAME_PROTOCOL  *This,
    IN EFI_HANDLE                   ControllerHandle,
    IN EFI_HANDLE                   ChildHandle OPTIONAL,
    IN CHAR8                        *Language,
    OUT CHAR16                      **ControllerName
)
{
    return EFI_UNSUPPORTED;
}

EFI_COMPONENT_NAME_PROTOCOL gComponentName = {
    TerminalGetDriverName,
    TerminalGetControllerName,
    0
};

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2013, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**             5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093          **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
