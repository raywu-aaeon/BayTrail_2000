#include "TxeFwDowngradeDxe.h"
#include <Setup.h>

EFI_STATUS  TxeFwDowngradeDxeEntryPoint(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    EFI_STATUS  Status;
    SETUP_DATA  SetupData;
    UINTN       VarSize;
    UINT32      Attributes = 0;
    UINT8       Result = 0;

    VarSize = sizeof(SETUP_DATA);
    Status = gRT->GetVariable(
                    L"Setup",
                    &gEfiSetupVariableGuid,
                    &Attributes,
                    &VarSize,
                    &SetupData );
    if( !EFI_ERROR(Status) && SetupData.TxeFwDowngrade == 1 )
    {
        SetupData.TxeFwDowngrade = 0;
        Status = gRT->SetVariable(
                    L"Setup",
                    &gEfiSetupVariableGuid,
                    Attributes,
                    VarSize,
                    &SetupData );
        if( !EFI_ERROR(Status) )
        {
            HeciHmrfpoEnable (0, &Result);
            HeciSendCbmResetRequest (0x02, 0x01);
        }
    }

    return Status;
}