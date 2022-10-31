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

//*************************************************************************
// $Header: /Alaska/BIN/Modules/CRB Board/CRBAcpi.c 2     7/11/11 9:13a Abelwu $
//
// $Revision: 2 $
//
// $Date: 7/11/11 9:13a $
//*************************************************************************
// Revision History
// ----------------
// $Log: /Alaska/BIN/Modules/CRB Board/CRBAcpi.c $
// 
// 2     7/11/11 9:13a Abelwu
// [TAG]  		EIP63768
// [Category]  	Improvement
// [Description]  	Updated for Core 4.6.5.x uEFI 2.3.1 / PI 1.2 compliance
// [Files]  		CRBAcpi.c
// 
// 1     7/28/09 3:24a Abelwu
// 
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        CRBAcpi.c
//
// Description: This file contains 2 eLinks for CRB ACPI Enabled/Disabled 
//              event(s).
//
//<AMI_FHDR_END>
//*************************************************************************

//----------------------------------------------------------------------------
// Include(s)
//----------------------------------------------------------------------------

#include <Token.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>

#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
#include <Protocol\SmmSwDispatch2.h>
#define CRB_SMM_SW_DISPATCH_CONTEXT  EFI_SMM_SW_REGISTER_CONTEXT
#else
#include <Protocol\SmmSwDispatch.h>
#define CRB_SMM_SW_DISPATCH_CONTEXT  EFI_SMM_SW_DISPATCH_CONTEXT
#endif

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
// Procedure:   CRBAcpiEnabled
//
// Description: This routine will be called when ACPI enabled.
//
// Input:       DispatchHandle  - Handle to the Dispatcher
//              DispatchContext - SW SMM dispatcher context
//
// Output:      None
//
// Notes:       Porting if needed.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID CRBAcpiEnabled (
    IN EFI_HANDLE                   DispatchHandle,
    IN CRB_SMM_SW_DISPATCH_CONTEXT  *DispatchContext )
{
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CRBAcpiDisabled
//
// Description: This routine will be called when ACPI disabled.
//
// Input:       DispatchHandle  - Handle to the Dispatcher
//              DispatchContext - SW SMM dispatcher context
//
// Output:      None
//
// Notes:       Porting if needed.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID CRBAcpiDisabled (
    IN EFI_HANDLE                   DispatchHandle,
    IN CRB_SMM_SW_DISPATCH_CONTEXT  *DispatchContext )
{
}

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
