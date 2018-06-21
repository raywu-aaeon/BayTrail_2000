/*++
  This file contains an 'Intel Peripheral Driver' and is        
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/
/** @file
  Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#include <PiPei.h>
#include <Ppi/S3ResumeDone.h>
#include <Library/DebugLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/PeiServicesLib.h>
#include "VlvAccess.h"
#include <Library/SbPolicy.h>        // AMI_OVERRIDE

/**
  Send Shadow Done Message. 

  @param PeiServices       An indirect pointer to the EFI_PEI_SERVICES table published by the PEI Foundation
  @param NotifyDescriptor  Address of the notification descriptor data structure.
  @param Ppi               Address of the PPI that was installed.

  @retval EFI_SUCCESS      Successfully update the Boot records.
  @retval EFI_NOT_FOUND    PEI boot records are not found or Basic boot performance table is not found. 

**/
EFI_STATUS
EFIAPI
S3ResumeDonePpiNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
{
  UINT32                          Data32;
  
  DEBUG ((EFI_D_INFO, "Send Shadow Done Message Start\n"));
  Data32 = MmPci32(0, 0, 26, 0, 0x64);
  DEBUG ((EFI_D_INFO, "Send Shadow Done Message read Offset 0x64 is %x \n", Data32));
  Data32 |= 1;
  DEBUG ((EFI_D_INFO, "Send Shadow Done Message write Offset 0x64 is %x try to send Shadow Done Message \n", Data32));
  MmPci32(0, 0, 26, 0, 0x64) = Data32;
  DEBUG ((EFI_D_INFO, "Sent Shadow Done Message End\n"));
    
  return EFI_SUCCESS;
}

EFI_PEI_NOTIFY_DESCRIPTOR mS3ResumeDonePpiNotifyList[] = {
  {
    (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gS3ResumeDonePpiGuid,
    S3ResumeDonePpiNotifyCallback
  }
};

/**
  Main entry for Shadow Down PEIM.
  It installs S3ResumeDonePpi Notify function on S3 boot path. 

  @param[in]  FileHandle              Handle of the file being invoked.
  @param[in]  PeiServices             Pointer to PEI Services table.

  @retval EFI_SUCCESS Install Notify function successfully. 

**/
EFI_STATUS
EFIAPI
ShadowDownPeiEntryPoint (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                      Status;
  EFI_BOOT_MODE                   BootMode;

  Status = PeiServicesGetBootMode(&BootMode);
  ASSERT_EFI_ERROR (Status);

  if (BootMode == BOOT_ON_S3_RESUME) {
    //
    // Register for a callback once S3 resume has been done.
    //
    Status = PeiServicesNotifyPpi (&mS3ResumeDonePpiNotifyList[0]);
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}
