
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
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//*****************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        SmmControl2
//
// Description: This file contains code for SmmControl2 protocol - the
//              protocol defined by Framework
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


// Module specific Includes
#include <Library/UefiBootServicesTableLib.h>
#include <AmiChipsetIoLib.h>
#include <Protocol/SmmAccess2.h>
#include <Protocol/SmmControl2.h>
#include <Library/SbCspLib.h>
#include <Sb.h>
#include <token.h>

// Function Prototypes
EFI_STATUS SbSmmClearSmi2(
    IN CONST EFI_SMM_CONTROL2_PROTOCOL  *This,
    IN BOOLEAN                          Periodic OPTIONAL
);

EFI_STATUS SbSmmTriggerSmi2(
    IN CONST EFI_SMM_CONTROL2_PROTOCOL  *This,
    IN OUT UINT8                        *CommandPort       OPTIONAL,
    IN OUT UINT8                        *DataPort          OPTIONAL,
    IN BOOLEAN                          Periodic           OPTIONAL,
    IN UINTN                            ActivationInterval OPTIONAL
);

// GUID Definitions



// Global variable declarations
EFI_SMM_ACCESS2_PROTOCOL            *gSmmAccess2;
EFI_SMM_CONTROL2_PROTOCOL           gEfiSmmControl2Protocol = {
    SbSmmTriggerSmi2,
    SbSmmClearSmi2,
    0
};

AMI_S3_SAVE_PROTOCOL                *gBootScript;;

// Portable Constants

// Function Definition

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SbSmmEnableSwSmi
//
// Description: This function programs the SB chipset registers to enable
//              S/W SMI generation
//
// Input:       None
//
// Output:      EFI_SUCCESS Always
//
// Notes:       CHIPSET AND/OR BOARD PORTING NEEDED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SbSmmEnableSwSmi(
    VOID
)
{

    SbSmmClearSmi2(&gEfiSmmControl2Protocol, FALSE);

    SET_IO32_PM(R_SMI_EN, 0x21); // Enable S/W SMI
    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   GenerateSwSmi
//
// Description: This function generates a software SMI by writing the
//              provided 2 byte values Command & Data into the software SMI
//              generation registers.
//
// Input:       Command - The value written to the command port.
//              Data - The value written to the data port. (Optional)
//
// Output:      None
//
// Notes:       CHIPSET AND/OR BOARD PORTING NEEDED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID GenerateSwSmi(
    IN UINT8            Command,
    IN UINT8            Data)
{

    SET_IO32_PM(R_SMI_EN, 0x21); // Enable S/W SMI
    WRITE_IO8(SW_SMI_IO_ADDRESS + 1, Data); // Write SMI data port
    WRITE_IO8(IO_DELAY_PORT, Data);   // I/O delay
    WRITE_IO8(SW_SMI_IO_ADDRESS, Command); // This triggers SMI

    Data = READ_IO8(SW_SMI_IO_ADDRESS + 1);   // I/O delay is necessary
    WRITE_IO8(IO_DELAY_PORT, Data);   // I/O delay is necessary
    Data = READ_IO8(SW_SMI_IO_ADDRESS + 1);   // I/O delay is necessary
    WRITE_IO8(IO_DELAY_PORT, Data);   // I/O delay is necessary

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbSmmTriggerSmi2
//
// Description: This function generates a software SMI by writing the provided
//              byte value into the software SMI generation register
//
// Input:       *This - Pointer to the SMM control II protocol
//              *CommandPort - The value written to the command port.
//              *DataPort - The value written to the data port.
//              Periodic - Boolean indicating the nature of generation
//                  TRUE - means periodic generation depending on
//                         timing value provided in the next variable
//                         CURRENTLY NOT SUPPORTED. EXPECTS FALSE
//              ActivationInterval - Optional. NOT SUPPORTED
//
// Output:      EFI_STATUS
//                  EFI_SUCCESS - S/W SMI triggered successfully
//                  EFI_INVALID_PARAMETER - If Periodic is TRUE or when
//                                          (ArgumentBuffer is not NULL and
//                                          ArgumentBufferSize is not 1)
//
// Notes:       No Porting Required
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SbSmmTriggerSmi2(
    IN CONST EFI_SMM_CONTROL2_PROTOCOL *This,
    IN OUT UINT8                       *CommandPort OPTIONAL,
    IN OUT UINT8                       *DataPort OPTIONAL,
    IN BOOLEAN                         Periodic OPTIONAL,
    IN UINTN                           ActivationInterval OPTIONAL)
{
    UINT8     Data;
    
    if(Periodic) return EFI_INVALID_PARAMETER;

    if (CommandPort == NULL) {
      Data = 0xFF;
    } else {
      Data = *CommandPort;
    }

//<EIP131876 >>
    if ( DataPort == NULL ) {
        GenerateSwSmi(Data , 0xFF);
    } else {
        GenerateSwSmi(Data , *DataPort);
    }
//<EIP131876 <<

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   SbSmmClearSmi2
//
// Description: This function clears software SMI status registers and
//              generates End-of-SMI (EOS).
//
// Input:       *This    - Pointer to the SMM control II protocol
//              Periodic - Boolean indicating the nature of clearing,
//                         TRUE means periodic SMI clearing.
//                         CURRENTLY NOT SUPPORTED. EXPECTS FALSE
//
// Output:      EFI_STATUS
//                  EFI_SUCCESS - SMI status successfully cleared
//                  EFI_INVALID_PARAMETER - If Periodic is TRUE
//
// Notes:       No Porting Required
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SbSmmClearSmi2(
    IN CONST EFI_SMM_CONTROL2_PROTOCOL  *This,
    IN BOOLEAN                          Periodic OPTIONAL)
{
    if(Periodic) return EFI_INVALID_PARAMETER;

    /// Clear the Power Button Override Status Bit, it gates EOS from being set.
    WRITE_IO16_PM(R_PM1_STS, 0x0800);

    // Porting Required.  Include code to clear software SMI status only
    WRITE_IO32_PM(R_SMI_STS, 0x20); // 0x34

    // Set EOS
    SET_IO8_PM(R_SMI_EN, 0x02); // 0x30

    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SmmControl2EntryPoint
//
// Description: This function installs SMM Control 2 protocol
//
// Input:       IN EFI_HANDLE ImageHandle - Image handle
//              IN EFI_SYSTEM_TABLE *SystemTable - Pointer to the system table
//
// Output:      Return Status based on errors that install the protocol
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SmmControl2EntryPoint(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    UINT8       Value;
    EFI_STATUS  Status;

    Status = gBS->LocateProtocol(&gEfiSmmAccess2ProtocolGuid, \
                                 NULL, \
                                 &gSmmAccess2);
    if(EFI_ERROR(Status)) return Status;

    Status = gBS->LocateProtocol(AMI_S3_SAVE_PROTOCOL_GUID,
                                 NULL,
                                 &gBootScript);
    if(EFI_ERROR(Status)) return Status;


    // Enable S/W SMI Generation
    SbSmmEnableSwSmi();

    // Save in boot script for S3 wake up
    Value = READ_IO8_PM(R_SMI_EN);
    WRITE_IO8_S3(gBootScript, PM_BASE_ADDRESS + R_SMI_EN, Value);

    return gBS->InstallMultipleProtocolInterfaces(
               &ImageHandle,
               &gEfiSmmControl2ProtocolGuid,
               &gEfiSmmControl2Protocol,
               NULL,
               NULL);

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

