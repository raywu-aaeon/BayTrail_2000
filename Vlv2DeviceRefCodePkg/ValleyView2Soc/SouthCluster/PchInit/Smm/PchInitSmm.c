/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2014 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  PchInitSmm.c

  @brief
  This is the driver that initializes the Intel PCH for save/restore during S3.

--*/

#include "PchInitSmm.h"

EFI_SMM_SX_DISPATCH_PROTOCOL               *mSxDispatch;
EFI_SMM_SW_DISPATCH_PROTOCOL              *mSwDispatch;
DXE_PCH_PLATFORM_POLICY_PROTOCOL       *PchPlatformPchPolicy;


/**
  Initializes the PCH SMM handler for  PCH register S3 Save/Restore

  @param[in] ImageHandle - Handle for the image of this driver
  @param[in] SystemTable - Pointer to the EFI System Table

  @retval EFI_SUCCESS    - PCH SMM handler was installed
**/
EFI_STATUS
EFIAPI
PchInitSmmEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                                Status;

  DEBUG ((DEBUG_INFO, "PchInitSmmEntryPoint()\n"));
  //
  // Locate the PCH Trap dispatch protocol
  //
  Status = gBS->LocateProtocol (&gEfiSmmSxDispatchProtocolGuid, NULL, (VOID **) &mSxDispatch);
  ASSERT_EFI_ERROR (Status);
  Status = gBS->LocateProtocol (&gEfiSmmSwDispatchProtocolGuid, NULL, (VOID **) &mSwDispatch);
  ASSERT_EFI_ERROR (Status);
  Status  = gBS->LocateProtocol (&gDxePchPlatformPolicyProtocolGuid, NULL, (VOID **) &PchPlatformPchPolicy);
  ASSERT_EFI_ERROR (Status);

  PchInitLateSmm (ImageHandle, SystemTable);

  return EFI_SUCCESS;
}
