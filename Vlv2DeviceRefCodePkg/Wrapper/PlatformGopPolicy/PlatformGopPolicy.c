/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

--*/

/** @file
**/

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/PlatformGOPPolicy.h>

#include <SetupMode.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Library/NbPolicy.h>

EFI_BOOT_SERVICES   *gBS;


PLATFORM_GOP_POLICY_PROTOCOL  mPlatformGOPPolicy;

//
// Function implementations
//

/*++

Routine Description:

  The function will excute with as the platform policy, and gives
  the Platform Lid Status. IBV/OEM can customize this code for their specific
  policy action.

Arguments:

  CurrentLidStatus - Gives the current LID Status

Returns:

  EFI_SUCCESS.

--*/
EFI_STATUS
GetPlatformLidStatus(
    OUT LID_STATUS *CurrentLidStatus
)
{
    NB_SETUP_DATA       SystemConfiguration;

    //
    //At present return LidOpen status by default
    //
    if(CurrentLidStatus==NULL) {
        return EFI_INVALID_PARAMETER;
    }

    GetNbSetupData((VOID*)gRT, &SystemConfiguration, FALSE);

    if(SystemConfiguration.LidStatus == 1) {
        *CurrentLidStatus = LidOpen;
    } else {
        *CurrentLidStatus = LidClosed;
    }

    return EFI_SUCCESS;
}

/*++

Routine Description:

  The function will excute and gives the Video Bios Table Size and Address.

Arguments:

  VbtAddress - Gives the Physical Address of Video BIOS Table

  VbtSize - Gives the Size of Video BIOS Table

Returns:

  EFI_STATUS.

--*/

EFI_STATUS
GetVbtData(
    OUT EFI_PHYSICAL_ADDRESS *VbtAddress,
    OUT UINT32 *VbtSize
)
{
    EFI_STATUS                    Status;
    UINTN                         FvProtocolCount;
    EFI_HANDLE                    *FvHandles;
    EFI_FIRMWARE_VOLUME2_PROTOCOL  *Fv;
    UINTN                         Index;
    UINT32                        AuthenticationStatus;

    UINT8                         *Buffer;
    UINTN                         VbtBufferSize;

    Buffer = 0;
    FvHandles       = NULL;

    if(VbtAddress == NULL || VbtSize == NULL) {
        return EFI_INVALID_PARAMETER;
    }
    Status = gBS->LocateHandleBuffer(
                 ByProtocol,
                 &gEfiFirmwareVolume2ProtocolGuid,
                 NULL,
                 &FvProtocolCount,
                 &FvHandles
             );

    if(!EFI_ERROR(Status)) {
        for(Index = 0; Index < FvProtocolCount; Index++) {
            gBS->HandleProtocol(
                FvHandles[Index],
                &gEfiFirmwareVolume2ProtocolGuid,
                (VOID **) &Fv
            );
//(CSP20130111E+)>>
            VbtBufferSize = 0;
            Status = Fv->ReadSection(
                         Fv,
                         &gBmpImageGuid,
                         EFI_SECTION_RAW,
                         0,
                         &Buffer,
                         &VbtBufferSize,
                         &AuthenticationStatus
                     );
//(CSP20130111E+)<<

            if(!EFI_ERROR(Status)) {
                *VbtAddress = (EFI_PHYSICAL_ADDRESS)Buffer;
                *VbtSize = (UINT32)VbtBufferSize;
                Status = EFI_SUCCESS;
                break;
            }
        }
    } else {
        Status = EFI_NOT_FOUND;
    }

    if(FvHandles != NULL) {
        gBS->FreePool(FvHandles);
        FvHandles = NULL;
    }

    return Status;
}

/*++

Routine Description:

  Entry point for the Platform GOP Policy Driver.

Arguments:

  ImageHandle       Image handle of this driver.
  SystemTable       Global system service table.

Returns:

  EFI_SUCCESS           Initialization complete.
  EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.

--*/

EFI_STATUS
EFIAPI
PlatformGOPPolicyEntryPoint(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)

{
    EFI_STATUS  Status;
    //-SETUP_DATA          SystemConfiguration;
    //-UINTN       VarSize;


    gBS = SystemTable->BootServices;

    gBS->SetMem(&mPlatformGOPPolicy, sizeof(PLATFORM_GOP_POLICY_PROTOCOL), 0);

    mPlatformGOPPolicy.Revision                = PLATFORM_GOP_POLICY_PROTOCOL_REVISION_01;
    mPlatformGOPPolicy.GetPlatformLidStatus    = GetPlatformLidStatus;
    mPlatformGOPPolicy.GetVbtData              = GetVbtData;

    //
    // Install protocol to allow access to this Policy.
    //

    /*
      VarSize = sizeof(SETUP_DATA);
      Status = gRT->GetVariable(L"Setup",
                                &gEfiNormalSetupGuid,
                                NULL,
                                &VarSize,
                                &SystemConfiguration);
      ASSERT_EFI_ERROR(Status);
      if(SystemConfiguration.GOPEnable == 0)
      {
    */
    Status = gBS->InstallMultipleProtocolInterfaces(
                 &ImageHandle,
                 &gPlatformGOPPolicyGuid,
                 &mPlatformGOPPolicy,
                 NULL
             );
    /*
      }
    */

    return Status;
}
