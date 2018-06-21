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

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name:        SleepSmi.c
//
// Description: Provide functions to register and handle Sleep SMI
//              functionality.  
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <Token.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSxDispatch2.h>
#include <Library/SbPolicy.h>
#include <PchAccess.h>

#define AMI_SMM_SX_DISPATCH_PROTOCOL EFI_SMM_SX_DISPATCH2_PROTOCOL
#define AMI_SMM_SX_DISPATCH_CONTEXT  EFI_SMM_SX_REGISTER_CONTEXT
#define SMM_CHILD_DISPATCH_SUCCESS   EFI_SUCCESS


SB_SETUP_DATA                   PchPolicyData;
    
// Function declarations

VOID ChipsetSleepWorkaround(
    VOID
);

// Function Definitions

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   S1SleepSmiOccurred
//
// Description: This function will be called by EfiSmmSxDispatch when a Sleep
//              SMI occurs and the sleep state is S1.
//
// Input:       PI 0.91, 1.0
//                  DispatchHandle   - SMI dispatcher handle
//                  *DispatchContext - Pointer to the dispatch context
//              PI 1.1, 1.2
//                  DispatchHandle  - SMI dispatcher handle
//                  *DispatchContext- Points to an optional Sx SMI context
//                  CommBuffer      - Points to the optional communication
//                                    buffer
//                  CommBufferSize  - Points to the size of the optional
//
// Output:      EFI_STATUS if the new SMM PI is applied.
//
// Notes:       This function does not need to put the system to sleep.  This is
//              handled by PutToSleep.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS S1SleepSmiOccurred (
	IN EFI_HANDLE       DispatchHandle,
	IN CONST VOID       *DispatchContext OPTIONAL,
	IN OUT VOID         *CommBuffer OPTIONAL,
	IN OUT UINTN        *CommBufferSize OPTIONAL )
{
    return SMM_CHILD_DISPATCH_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   S3SleepSmiOccurred
//
// Description: This function will be called by EfiSmmSxDispatch when a Sleep
//              SMI occurs and the sleep state is S3.
//
// Input:       PI 0.91, 1.0
//                  DispatchHandle   - SMI dispatcher handle
//                  *DispatchContext - Pointer to the dispatch context
//              PI 1.1, 1.2
//                  DispatchHandle  - SMI dispatcher handle
//                  *DispatchContext- Points to an optional Sx SMI context
//                  CommBuffer      - Points to the optional communication
//                                    buffer
//                  CommBufferSize  - Points to the size of the optional
//
// Output:      EFI_STATUS if the new SMM PI is applied.
//
// Notes:       This function does not need to put the system to sleep.  This is
//              handled by PutToSleep.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS S3SleepSmiOccurred (
	IN EFI_HANDLE       DispatchHandle,
	IN CONST VOID       *DispatchContext OPTIONAL,
	IN OUT VOID         *CommBuffer OPTIONAL,
	IN OUT UINTN        *CommBufferSize OPTIONAL )
{
    ChipsetSleepWorkaround();

#if LastStateForS3
    if (PchPolicyData.LastState == 2){
      WRITE_MEM32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, READ_MEM32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) | BIT00);
    }
#endif
    
    return SMM_CHILD_DISPATCH_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   S4SleepSmiOccurred
//
// Description: This function will be called by EfiSmmSxDispatch when a Sleep
//              SMI occurs and the sleep state is S4.
//
// Input:       PI 0.91, 1.0
//                  DispatchHandle   - SMI dispatcher handle
//                  *DispatchContext - Pointer to the dispatch context
//              PI 1.1, 1.2
//                  DispatchHandle  - SMI dispatcher handle
//                  *DispatchContext- Points to an optional Sx SMI context
//                  CommBuffer      - Points to the optional communication
//                                    buffer
//                  CommBufferSize  - Points to the size of the optional
//                                    communication buffer
//
// Output:      EFI_STATUS if the new SMM PI is applied.
//
// Notes:       This function does not need to put the system to sleep.  This is
//              handled by PutToSleep.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS S4SleepSmiOccurred (
	IN EFI_HANDLE       DispatchHandle,
	IN CONST VOID       *DispatchContext OPTIONAL,
	IN OUT VOID         *CommBuffer OPTIONAL,
	IN OUT UINTN        *CommBufferSize OPTIONAL )
{
    ChipsetSleepWorkaround();

#if LastStateForS4
    if (PchPolicyData.LastState == 2){
      WRITE_MEM32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, READ_MEM32 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) | BIT00);
    }
#endif
    
    return SMM_CHILD_DISPATCH_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   S5SleepSmiOccurred
//
// Description: This function will be called by EfiSmmSxDispatch when a Sleep
//              SMI occurs and the sleep state is S1.
//
// Input:       PI 0.91, 1.0
//                  DispatchHandle   - SMI dispatcher handle
//                  *DispatchContext - Pointer to the dispatch context
//              PI 1.1, 1.2
//                  DispatchHandle  - SMI dispatcher handle
//                  *DispatchContext- Points to an optional Sx SMI context
//                  CommBuffer      - Points to the optional communication
//                                    buffer
//                  CommBufferSize  - Points to the size of the optional
//                                    communication buffer
//
// Output:      EFI_STATUS if the new SMM PI is applied.
//
// Notes:       This function does not need to put the system to sleep.  This is
//              handled by PutToSleep.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS S5SleepSmiOccurred (
	IN EFI_HANDLE       DispatchHandle,
	IN CONST VOID       *DispatchContext OPTIONAL,
	IN OUT VOID         *CommBuffer OPTIONAL,
	IN OUT UINTN        *CommBufferSize OPTIONAL )
{
//EIP148801 >>
#if USB_S5_WAKEUP_SUPPORT
    UINT32    Data32;
#endif
//EIP148801 <<    
    UINT32    DisEhci,DisXhci; //EIP166368
    
  ChipsetSleepWorkaround();
    
#if LastStateForS5
    if (PchPolicyData.LastState == 2){
      WRITE_MEM32(PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1, READ_MEM32 (PMC_BASE_ADDRESS + R_PCH_PMC_GEN_PMCON_1) | BIT00);
    }
#endif

    //EIP166368 >>
    DisEhci = MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS) & B_PCH_PMC_FUNC_DIS_USB;
    DisXhci = MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS2) & B_PCH_PMC_FUNC_DIS2_USH_SS_PHY; 
    
      //USB Smart Auto
    if ((PchPolicyData.PchUsb30Mode == 3) && (DisEhci == 0) && (DisXhci == 0))    
    {
      MmioOr32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS, B_PCH_PMC_FUNC_DIS_USB);
      MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS);  // Read back Posted Writes Register
    }
    //EIP166368 <<

//EIP148801 >>
#if USB_S5_WAKEUP_SUPPORT    
    // Clear PM1_STS
    IoWrite32(PM_BASE_ADDRESS, IoRead32(PM_BASE_ADDRESS));
    // Clear GPE0_STS
    IoWrite32(PM_BASE_ADDRESS + 0x20, IoRead32(PM_BASE_ADDRESS + 0x20));
    // Set PME_B0_EN
    IoWrite32(PM_BASE_ADDRESS + 0x28, (IoRead32(PM_BASE_ADDRESS + 0x28) | BIT13));
    // Clear PCI Express Wake Disable
    Data32 = IoRead32(PM_BASE_ADDRESS) & (UINT32) ~BIT30;
    IoWrite32(PM_BASE_ADDRESS, Data32);
#endif
//EIP148801 <<    
    return SMM_CHILD_DISPATCH_SUCCESS;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ChipsetSleepWorkaround
//
// Description: This function executes chipset workaround that is needed.  It is 
//              necessary for the system to go to S3 - S5.
//
// Input:       None
//
// Output:      None
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ChipsetSleepWorkaround( 
    VOID 
)   
{
/** CHIPSET PORTING IF NEEDED.
    UINT16 Value;
    UINT32 PciRegister = 0x800000D4;
    
    IoWrite32(0xCF8,PciRegister);
    Value = IoRead16(0xCFC);
    Value |= BIT14;
    IoWrite16(0xCFC,Value);
 **/
}

//----------------------------------------------------------------------------
//
// Procedure:   InSmmFunction
//
// Description: Install Sleep SMI Handlers for south bridge.
//
// Input:       ImageHandle  - Image handle
//              *SystemTable - Pointer to the system table
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS InSmmFunction (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    EFI_STATUS                      Status;
    EFI_HANDLE                      hS1Smi;
    EFI_HANDLE                      hS3Smi;
    EFI_HANDLE                      hS4Smi;
    EFI_HANDLE                      hS5Smi;
    AMI_SMM_SX_DISPATCH_PROTOCOL    *SxDispatch;
    AMI_SMM_SX_DISPATCH_CONTEXT     S1DispatchContext = {SxS1, SxEntry};
    AMI_SMM_SX_DISPATCH_CONTEXT     S3DispatchContext = {SxS3, SxEntry};
    AMI_SMM_SX_DISPATCH_CONTEXT     S4DispatchContext = {SxS4, SxEntry};
    AMI_SMM_SX_DISPATCH_CONTEXT     S5DispatchContext = {SxS5, SxEntry};

    Status = InitAmiSmmLib( ImageHandle, SystemTable );
    if (EFI_ERROR(Status)) return Status;
    Status = pSmst->SmmLocateProtocol( &gEfiSmmSxDispatch2ProtocolGuid , \
                                       NULL, \
                                       &SxDispatch );

    if (EFI_ERROR(Status)) return Status;

    // Register Sleep SMI Handlers
    Status = SxDispatch->Register( SxDispatch, \
                                   S1SleepSmiOccurred, \
                                   &S1DispatchContext, \
                                   &hS1Smi );
    if (EFI_ERROR(Status)) return Status;

    Status = SxDispatch->Register( SxDispatch, \
                                   S3SleepSmiOccurred, \
                                   &S3DispatchContext, \
                                   &hS3Smi );
    if (EFI_ERROR(Status)) return Status;

    Status = SxDispatch->Register( SxDispatch, \
                                   S4SleepSmiOccurred, \
                                   &S4DispatchContext, \
                                   &hS4Smi );
    if (EFI_ERROR(Status)) return Status;

    Status = SxDispatch->Register( SxDispatch, \
                                   S5SleepSmiOccurred, \
                                   &S5DispatchContext, \
                                   &hS5Smi );
    return Status;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InitSleepSmi
//
// Description: This function Registers Sleep SMI functionality.
//
// Input:       IN EFI_HANDLE ImageHandle     Handle for this FFS image
//              IN EFI_SYSTEM_TABLE *SystemTable    Pointer to the system table
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS InitSleepSmi(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    InitAmiLib( ImageHandle, SystemTable );

    // Get the value of the SB Setup data.
    GetSbSetupData (  (VOID*)pRS, &PchPolicyData, FALSE);

    return InitSmmHandler( ImageHandle, SystemTable, InSmmFunction, NULL );
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
