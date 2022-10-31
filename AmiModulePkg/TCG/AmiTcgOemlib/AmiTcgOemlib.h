//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/AmiTcgOemlib/AmiTcgOemlib.h 3     3/29/11 12:53p Fredericko $
//
// $Revision: 3 $
//
// $Date: 3/29/11 12:53p $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  AmiTcgOemlib.h
//
// Description: Header File
//      
//
//<AMI_FHDR_END>
//**********************************************************************
//Array of supported TCM devices
#include <efi.h>

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   iTPMVerifyMeStatus
//
// Description: OemHook to use to control functionality of TCG module with iTPM
//              good for when a platform allows disabling of ME while TCG support
//              is enabled.
//
//
// Input:
//
// Output:      BOOLEAN
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
BOOLEAN iTPMVerifyMeStatus( );



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   OemTPM_EnterConfig
//
// Description: OemHook to use to enter TPM config mode
//
//
// Input:
//
// Output:      BOOLEAN
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
BOOLEAN OemTPM_EnterConfig( );



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   OemTPM_ExitConfig
//
// Description: OemHook to use to exit TPM config mode
//
//
// Input:
//
// Output:      BOOLEAN
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
BOOLEAN OemTPM_ExitConfig( );




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   Read_Tpm_Chip
//
// Description: OemHook to read a specific offset in leg IOSpace
//
//
// Input:
//
// Output:      IN  UINT8
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
UINT8 Read_Tpm_Chip(
   IN  UINT8 Val );



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   Configure_Tpm_Chip
//
// Description: OemHook to configure the TPM for legacy IO mode
//
//
// Input:
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS Configure_Tpm_Chip( );

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**     5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
