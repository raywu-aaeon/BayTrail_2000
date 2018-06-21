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

//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/PEI Ram Boot/PeiRamBootOfbd.c 1     2/11/11 3:15a Calvinchen $
//
// $Revision: 1 $
//
// $Date: 2/11/11 3:15a $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/PEI Ram Boot/PeiRamBootOfbd.c $
// 
// 1     2/11/11 3:15a Calvinchen
// Bug Fixed : System hangs after reflashed BIOS with warm reset if
// PEI_RAM_BOOT_S3_SUPPORT = 1 with fast warm boot support.
// 
// 
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	PeiRamBootOfbd.c
//
// Description: If system updated BIOS with warm reset, PeiRamBoot module
//              will not copy rom to ram in sequence boot, and, it could 
//              cause system crashed. For avoiding this situation, BIOS 
//              delete the PEI RAM Boot related variables if system executed
//              AFU.
//
//<AMI_FHDR_END>
//**********************************************************************
//----------------------------------------------------------------------------
// Includes
// Statements that include other files
//#include <AmiLib.h>
#include <AmiDxeLib.h>
//#include <Setup.h>
//#include <HOB.h>
#include "PeiRamBoot.h"
//#include "OFBD.h"
//----------------------------------------------------------------------------
// Function Externs

//----------------------------------------------------------------------------
// Local prototypes
typedef EFI_STATUS (*SHOW_BOOT_TIME_VARIABLES)(BOOLEAN Show);
typedef struct{
    SHOW_BOOT_TIME_VARIABLES ShowBootTimeVariables;
} AMI_NVRAM_CONTROL_PROTOCOL;

//----------------------------------------------------------------------------
// Local Variables

//----------------------------------------------------------------------------
// Function Definitions
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	PeiRamBootOfbdEntry
//
// Description:	
//
// Input:
//      IN VOID             *Buffer
//      IN OUT UINT8        *pOFBDDataHandled
// Output:
//      VOID
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID PeiRamBootOfbdEntry (
    IN VOID             *Buffer,
    IN OUT UINT8        *pOFBDDataHandled )
{
    EFI_GUID                RomImageAddressGuid = ROM_IMAGE_ADDRESS_GUID;
    EFI_STATUS              Status;
    EFI_PHYSICAL_ADDRESS    HobRomImageAddress;
    UINTN                   VariableSize;
    static BOOLEAN          blIsProcessed = FALSE;
    static EFI_GUID         gAmiNvramControlProtocolGuid = \
    { 0xf7ca7568, 0x5a09, 0x4d2c, { 0x8a, 0x9b, 0x75, 0x84, 0x68, 0x59, 0x2a, 0xe2 } };
    AMI_NVRAM_CONTROL_PROTOCOL  *NvramControl = NULL;
    
    if (blIsProcessed == FALSE) blIsProcessed = TRUE;
    else return;

    NvramControl = GetSmstConfigurationTable(&gAmiNvramControlProtocolGuid);
    // Temporarily enable Nvram Variable Boot Time Access permission. 
    if (NvramControl) NvramControl->ShowBootTimeVariables(TRUE);

    VariableSize = sizeof(EFI_PHYSICAL_ADDRESS);
    Status = pRS->GetVariable ( L"HobRomImage", \
                                &RomImageAddressGuid, \
                                NULL, \
                                &VariableSize, \
                                &HobRomImageAddress );
    if (!EFI_ERROR(Status)) {
        // Delete HobRomImage Variable if system is executing AFU for forcing BIOS
        // copy rom to image in sequence boot.    
        VariableSize = 0;
        Status = pRS->SetVariable ( L"HobRomImage", \
                                    &RomImageAddressGuid, \
                                    EFI_VARIABLE_NON_VOLATILE | \
                                    EFI_VARIABLE_BOOTSERVICE_ACCESS,
                                    VariableSize, \
                                    &HobRomImageAddress );
    }
    // Disable Nvram Variable Boot Time Access permission. 
    if (NvramControl) NvramControl->ShowBootTimeVariables(FALSE);
    return;
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
