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

#include <Setup.h>
//
// Global defines and variables
// 
EFI_STATUS DxeGetVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN OUT UINTN *DataSize, OUT VOID *Data
);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//  Procedure:   PhysicalUserPresent
//
//  Description: Default implementation for Physical User Presence detection.
//               Detects if Admin User sign in by checking "SystemAccess" Variable
//
//  Input:  NONE
//
//  Output: BOOLEAN
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN PhysicalUserPresent ( VOID  ) 
{
    EFI_GUID  SystemAccessGuid  = SYSTEM_ACCESS_GUID;
    SYSTEM_ACCESS SystemAccess = {SYSTEM_PASSWORD_USER};
    UINTN         Size         = sizeof(SYSTEM_ACCESS);
    UINT32        Attributes   = 0;
    EFI_STATUS    Status;
   // TBD. Replace Admin User mode with true Physical user presence detection
    Status = DxeGetVariable(L"SystemAccess", &SystemAccessGuid, &Attributes, &Size, &SystemAccess);
    if (!EFI_ERROR(Status) 
        && !(Attributes & EFI_VARIABLE_NON_VOLATILE)
        && SystemAccess.Access==SYSTEM_PASSWORD_ADMIN)
    { 
        return TRUE;
    }

    return FALSE;
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
