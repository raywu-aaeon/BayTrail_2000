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
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/TcgSetup/TPMPwd.c 6     11/22/11 6:45p Fredericko $
//
// $Revision: 6 $
//
// $Date: 11/22/11 6:45p $
//**********************************************************************
//*************************************************************************
//<AMI_FHDR_START>
//
// Name: TPMPwd.c
//
// Description:
// Contains functions that handle TPM authentication
//
//<AMI_FHDR_END>
//*************************************************************************

#include "token.h"
#include <EFI.h>
#include <Protocol/SimpleTextIn.h>
#include <Protocol/EfiOEMBadging.h>
#include <Setup.h>
//include "AMITSEStrTokens.h"
#include "AMITSEStrDefs.h"
#include "commonoem.h"
#include "Protocol\AMIPostMgr.h"
#include "LogoLib.h"
#include "Mem.h"
#include "HiiLib.h"
#include "PwdLib.h"
#include "KeyMon.h"
#include "bootflow.h"
#include "commonoem.h"
#include "Core\EM\AMITSE\Inc\Variable.h"
#include "TcgPlatformSetupPolicy.h"

#if EFI_SPECIFICATION_VERSION>0x20000 && !defined(GUID_VARIABLE_DEFINITION)
    #include "Include\UefiHii.h"
    #include "Protocol/HiiDatabase.h"
    #include "Protocol/HiiString.h"
#else
  #include "Protocol/HII.h"
#endif

extern EFI_BOOT_SERVICES    *gBS;
extern EFI_SYSTEM_TABLE     *gST;
extern EFI_RUNTIME_SERVICES *gRT;
extern EFI_GUID             gEfiGlobalVariableGuid;


typedef struct
{   UINT16   VID;
    UINT16   DID;
} TCM_ID_STRUC;


TCM_ID_STRUC  TCMSupportedArray[NUMBER_OF_SUPPORTED_TCM_DEVICES]={
    {SUPPORTED_TCM_DEVICE_1_VID,SUPPORTED_TCM_DEVICE_1_DID},  //ZTEIC
    {SUPPORTED_TCM_DEVICE_2_VID,SUPPORTED_TCM_DEVICE_2_DID}  //ZTEIC2
};



//****************************************************************************************
//<AMI_PHDR_START>
//
// Procedure: TCGProcessConInAvailability
//
// Description: This is a replacement for the ProcessConInAvailability
//              hook in TSE, to provide password verification in the
//              TCG eModule.
//              This function is a hook called when TSE determines
//              that console is available. This function is available
//              as ELINK. In the generic implementation boot password
//              is prompted in this function.
//
//
// Input:       VOID
//
// Output:      BOOLEAN
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//****************************************************************************************
BOOLEAN TCGProcessConInAvailability(
    VOID )
{
    CHAR16        *text = NULL;
    UINTN         NoOfRetries;
    UINT32        PasswordInstalled = AMI_PASSWORD_NONE;
    UINTN         Index;
    EFI_INPUT_KEY Key;
    BOOLEAN       bScreenUsed  = FALSE;
    UINTN         VariableSize = sizeof(UINT32);
    UINT32        VariableData;
    BOOLEAN       PasswordRequest = FALSE;
    EFI_STATUS    Status;

    #if SETUP_PRINT_EVAL_MSG
    //Print evaluation message here
    text = HiiGetString( gHiiHandle,STRING_TOKEN( STR_EVAL_MSG ));

    if ( text != NULL )
    {
        PostManagerDisplayPostMessage( text );
    }
    MemFreePointer((VOID**)&text );
    #endif

    Status = gRT->GetVariable(
        L"AskPassword",
        &gEfiGlobalVariableGuid,
        NULL,
        &VariableSize,
        &VariableData
        );

    if ( VariableData == 0x58494d41 )   // "AMIX"
    {
        PasswordRequest = TRUE;
    }

    //Print setup utility prompt unless we have external password request
    if ( !PasswordRequest )
    {
        text = HiiGetString( gHiiHandle, STRING_TOKEN( STR_DEL_ENTER_SETUP ));

        if ( text != NULL )
        {
            PostManagerDisplayPostMessage( text );
        }
        MemFreePointer((VOID**)&text );
    }

    PasswordInstalled = PasswordCheckInstalled( );
    NoOfRetries       = 3;

    #if SETUP_USER_PASSWORD_POLICY

    if ((PasswordInstalled & AMI_PASSWORD_USER) || (PasswordRequest))
    {
    #else

    if ((PasswordInstalled & AMI_PASSWORD_ANY) || (PasswordRequest))
    {
        #endif

        bScreenUsed = TRUE;

        if ( AMI_PASSWORD_NONE ==
             CheckSystemPassword( AMI_PASSWORD_NONE, &NoOfRetries, NULL ))
        {
            while ( 1 )
            {
                //Patch
                //Ctl-Alt-Del is not recognized by core unless a
                //ReadKeyStroke is issued
                gBS->WaitForEvent( 1, gST->ConIn->WaitForKey, &Index );
                gST->ConIn->ReadKeyStroke( gST->ConIn, &Key );
            }
        }
    }

    return bScreenUsed;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   AutoSupportType
//
// Description: verifies support for a TCM module on a platform
//
// Input:       NONE
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
extern
BOOLEAN
__stdcall AutoSupportType ()
{
#if TCG_LEGACY == 0
  UINTN i=0;

   for(i=0;i<(sizeof(TCMSupportedArray)/sizeof(TCM_ID_STRUC));i++){
     if((TCMSupportedArray[i].VID == *(UINT16 *)(UINTN)(PORT_TPM_IOMEMBASE + 0xF00)) &&
        (TCMSupportedArray[i].DID == *(UINT16 *)(UINTN)(PORT_TPM_IOMEMBASE + 0xF02))){
         return TRUE;
      }
    }
#endif
    return FALSE;
}


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
//**********************************************************************
