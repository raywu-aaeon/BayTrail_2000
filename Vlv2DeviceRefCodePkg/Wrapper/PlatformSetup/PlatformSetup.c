//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
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
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        PlatformSetup.c
//
// Description: South Bridge Setup related routines
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <Setup.h>
#include <SetupStrDefs.h>
#include <Protocol/PciIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/SeCOperation.h>
#include <Protocol/SeCPlatformPolicy.h>
#include <Protocol/TdtOperation.h>

extern EFI_GUID gEfiSeCOperationProtocolGuid;
EFI_GUID    gSetupGuid = SETUP_GUID;

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InitPlatformStrings
//
// Description: This function initializes the Platform related setup option values
//
// Input:       IN EFI_HII_HANDLE HiiHandle - Handle to HII database
//              IN UINT16 Class - Indicates the setup class
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID InitPlatformStrings(
    IN EFI_HII_HANDLE   HiiHandle,
    IN UINT16           Class
)
{
    EFI_STATUS              Status;
    SEC_OPERATION_PROTOCOL  *SeCOp;
    SEC_INFOMATION          SeCInfo;
    TDT_OPERATION_PROTOCOL  *TdtOp;
    TDT_INFOMATION          TdtInfo;
    UINTN                   VarSize;
    SETUP_DATA              mSystemConfiguration;
    UINT32                  Attributes = 0; //EIP168675
    
    VarSize = sizeof(SETUP_DATA);
    Status = pRS->GetVariable(
                 L"Setup",
                 &gSetupGuid,
                 &Attributes, //EIP168675
                 &VarSize,
                 &mSystemConfiguration
             );
    TRACE((-1, "GetVariable NORMAL_SETUP_NAME:%r  \n",Status));    
    
    Status = pBS->LocateProtocol (
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
    mSystemConfiguration.SecEnable = (UINT8)SeCInfo.SeCEnable;
    mSystemConfiguration.SeCOpEnable = (UINT8)mSystemConfiguration.SecEnable;
    mSystemConfiguration.SeCModeEnable = (UINT8)SeCInfo.SeCOpEnable;
    mSystemConfiguration.SecFirmwareUpdate = (UINT8)SeCInfo.FwUpdate;
    mSystemConfiguration.SecFlashUpdate = (UINT8)SeCInfo.HmrfpoEnable;

    Status = pBS->LocateProtocol (
                  &gEfiTdtOperationProtocolGuid,
                  NULL,
                  &TdtOp
                  );
    if (!EFI_ERROR(Status)){
      Status = TdtOp->GetPlatformTdtInfo(
                      &TdtInfo
                      );
      mSystemConfiguration.TdtEnrolled = (UINT8)TdtInfo.TdtEnrolled;
      mSystemConfiguration.TdtState = (UINT8)TdtInfo.TdtState;
    }
    Status = pRS->SetVariable (
              L"Setup",
              &gSetupGuid,
              Attributes, //EIP168675
              sizeof(SETUP_DATA),
              &mSystemConfiguration);
}
//****************************************************************************************
//<AMI_PHDR_START>
//
// Procedure: PlatformSetupCallback
//
// Description: Setup Item call backs
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
EFI_STATUS
PlatformSetupCallback
(
    EFI_HII_HANDLE HiiHandle, 
    UINT16 Class, 
    UINT16 SubClass, 
    UINT16 Key
)
{
    EFI_STATUS              Status;
    SEC_OPERATION_PROTOCOL  *SeCOp;
    CALLBACK_PARAMETERS     *Callback;

    Callback = GetCallbackParameters();
    if ((Callback->Action == EFI_BROWSER_ACTION_RETRIEVE) ||
        (Callback->Action == EFI_BROWSER_ACTION_FORM_OPEN) ||
        (Callback->Action == EFI_BROWSER_ACTION_FORM_CLOSE) ||
        (Callback->Action == EFI_BROWSER_ACTION_DEFAULT_STANDARD) ||
        (Callback->Action == EFI_BROWSER_ACTION_DEFAULT_MANUFACTURING))
        return EFI_SUCCESS;

    Status = pBS->LocateProtocol (
                  &gEfiSeCOperationProtocolGuid,
                  NULL,
                  &SeCOp
                  );
    Status = SeCOp->PerformSeCOperation(
                  SEC_OP_UNCONFIGURATION
                  );
    return EFI_SUCCESS;
}
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
