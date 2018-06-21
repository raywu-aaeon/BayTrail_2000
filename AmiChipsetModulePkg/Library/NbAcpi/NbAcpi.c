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

//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        NbAcpi.c
//
// Description: This file contains 2 eLinks for all North Bridge ACPI
//              Enabled/Disabled events.
//
//<AMI_FHDR_END>
//*************************************************************************

//----------------------------------------------------------------------------
// Include(s)
//----------------------------------------------------------------------------

#include <Token.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include <Protocol/SmmSwDispatch2.h>
#include <Library/ElinkLib.h>

//----------------------------------------------------------------------------
// Constant, Macro and Type Definition(s)
//----------------------------------------------------------------------------
// Constant Definition(s)

// Macro Definition(s)

// Type Definition(s)

// Function Prototype(s)

//----------------------------------------------------------------------------
// Variable and External Declaration(s)
//----------------------------------------------------------------------------
// Variable Declaration(s)

// GUID Definition(s)

// Protocol Definition(s)

// External Declaration(s)

// Function Definition(s)

//----------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbAcpiEnabled
//
// Description: This routine will be called when ACPI enabled.
//
// Input:       DispatchHandle  - Handle to the Dispatcher
//              DispatchContext - Pointer to SW SMM dispatcher context
//
// Output:      None
//
// Notes:       Porting if needed.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID NbAcpiEnabled (
    IN EFI_HANDLE                   DispatchHandle,
    IN AMI_SMM_SW_DISPATCH_CONTEXT  *DispatchContext )
{
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbAcpiDisabled
//
// Description: This routine will be called when ACPI disabled.
//
// Input:       DispatchHandle  - Handle to the Dispatcher
//              DispatchContext - Pointer to SW SMM dispatcher context
//
// Output:      None
//
// Notes:       Porting if needed.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID NbAcpiDisabled (
    IN EFI_HANDLE                   DispatchHandle,
    IN AMI_SMM_SW_DISPATCH_CONTEXT  *DispatchContext )
{
}

EFI_STATUS
EFIAPI
NbAcpiConstruct (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  ElinkRegister (PcdEnableAcpiModeElink, NbAcpiEnabled);
  return ElinkRegister (PcdDisableAcpiModeElink, NbAcpiDisabled);
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
