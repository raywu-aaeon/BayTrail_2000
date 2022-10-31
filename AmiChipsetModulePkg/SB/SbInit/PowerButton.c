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
// Name:        PowerButton.c
//
// Description: This file contains code for all South Bridge SMI events
//
//<AMI_FHDR_END>
//*************************************************************************

//----------------------------------------------------------------------------
// Include(s)
//----------------------------------------------------------------------------

#include <Token.h>
#include <AmiDxeLib.h>
#include <Protocol/SmmPowerButtonDispatch2.h>
#include <PchRegs.h>
#include <Library/SbPolicy.h>

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

SB_SETUP_DATA                   PchPolicyData;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure: SbPwrBtnHandler
//
// Description: If the power button is pressed, then this function is called.
//
// Input: DispatchHandle  - Handle of dispatch function, for when interfacing
//                          with the parent SMM driver, will be the address of linked
//                          list link in the call back record.
//        DispatchContext - Pointer to the dispatch function's context.
//
// Output: VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SbPwrBtnHandler (
    IN EFI_HANDLE       DispatchHandle,
    IN CONST VOID       *DispatchContext OPTIONAL,
    IN OUT VOID         *CommBuffer OPTIONAL,
    IN OUT UINTN        *CommBufferSize OPTIONAL
)
{
    if (PchPolicyData.LastState == 2){
      //WRITE_MEM32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, READ_MEM32 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) | BIT00);
      MmioWrite32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, MmioRead32 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) | BIT00);
    }

	//
	// Enter S5 State
	//
	IoWrite16 (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_STS, B_PCH_ACPI_PM1_STS_PWRBTN);
	IoWrite16 (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT, V_PCH_ACPI_PM1_CNT_S5);
	IoWrite16 (PM_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT, V_PCH_ACPI_PM1_CNT_S5 + B_PCH_ACPI_PM1_CNT_SLP_EN);

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InSmmFunction
//
// Description: Installs South Bridge Power Button SMI Handler
//
// Input:       ImageHandle  - Image handle
//              *SystemTable - Pointer to the system table
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS InSmmFunction (
    IN EFI_HANDLE                 ImageHandle,
    IN EFI_SYSTEM_TABLE           *SystemTable
)
{
    EFI_STATUS                              Status;
    EFI_SMM_POWER_BUTTON_DISPATCH2_PROTOCOL *PowerButton;
    EFI_SMM_POWER_BUTTON_REGISTER_CONTEXT   DispatchContext = {EfiPowerButtonEntry}; //CSP20130801
    EFI_HANDLE                              Handle = NULL;


    Status  = pSmst->SmmLocateProtocol (
                      &gEfiSmmPowerButtonDispatch2ProtocolGuid,
                      NULL,
                      &PowerButton
                      );

    if (EFI_ERROR(Status)) return Status;

    Status = PowerButton->Register (
                            PowerButton,
                            SbPwrBtnHandler,
                            &DispatchContext,
                            &Handle
                            );

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InitializePowerButton
//
// Description: Initialize PowerButton SMM Driver.
//
// Input:       ImageHandle  - Image handle
//              *SystemTable - Pointer to the system table
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS InitializePowerButton (
    IN EFI_HANDLE                 ImageHandle,
    IN EFI_SYSTEM_TABLE           *SystemTable
)
{
    InitAmiLib(ImageHandle, SystemTable);

    GetSbSetupData (  (VOID*)pRS, &PchPolicyData, FALSE);

    return InitSmmHandler(ImageHandle, SystemTable, InSmmFunction, NULL);
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
