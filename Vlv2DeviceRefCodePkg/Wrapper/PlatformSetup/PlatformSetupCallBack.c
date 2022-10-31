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
// $Header:  $
//
// $Revision:  $
//
// $Date:  $
//**********************************************************************
//*************************************************************************
//<AMI_FHDR_START>
//
// Name: PlatformSetupHook.c
//
// Description:
// Contains functions that handle PlatformSetupHook  authentication
//
//<AMI_FHDR_END>
//*************************************************************************

#include "token.h"
#include <EFI.h>
#include <Protocol/SimpleTextIn.h>
#include <Setup.h>
#include <Library/HiiLib.h>
#include <Protocol/SeCOperation.h>
#include <AmiDxeLib.h>
#include "AmiTsePkg\Core\EM\AMITSE\Inc\Variable.h"

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

EFI_GUID    gSetupGuid = SETUP_GUID;
//****************************************************************************************
//<AMI_PHDR_START>
//
// Procedure: TcgUpdateDefaultsHook
//
// Description: Updates TCG status on F3
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
VOID PlatformSaveHook(VOID )
{
    EFI_STATUS              Status;
    SEC_OPERATION_PROTOCOL  *SeCOp;
    SEC_INFOMATION          SeCInfo;
    UINTN                                    VarSize;
    SETUP_DATA  mSystemConfiguration;
    
    VarSize = sizeof(SETUP_DATA);
    Status = gRT->GetVariable(
                 L"Setup",
                 &gSetupGuid,
                 NULL,
                 &VarSize,
                 &mSystemConfiguration
             );
    TRACE((-1,"[KL Debug] GetVariable Status = %r \n",Status));
  Status = gBS->LocateProtocol (
                &gEfiSeCOperationProtocolGuid,
                NULL,
                &SeCOp
                );
  if (EFI_ERROR(Status)) {
    return;
  }

  Status = SeCOp->GetPlatformSeCInfo(
                    &SeCInfo
                  );
  TRACE((-1,"[KL Debug] mSystemConfiguration.SecEnable = %x \n",mSystemConfiguration.SecEnable));
  TRACE((-1,"[KL Debug] mSystemConfiguration.SecFirmwareUpdate = %x \n",mSystemConfiguration.SecFirmwareUpdate));
  TRACE((-1,"[KL Debug] mSystemConfiguration.SecFlashUpdate = %x \n",mSystemConfiguration.SecFlashUpdate));
  SeCInfo.SeCEnable = mSystemConfiguration.SecEnable;
  SeCInfo.FwUpdate = mSystemConfiguration.SecFirmwareUpdate;
  SeCInfo.HmrfpoEnable = mSystemConfiguration.SecFlashUpdate;
  
  Status = SeCOp->SetPlatformSeCInfo(
                    &SeCInfo
                  );

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
