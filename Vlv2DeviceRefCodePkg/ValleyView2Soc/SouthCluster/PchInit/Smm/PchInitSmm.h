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
  PchInitSmm.h

  @brief
  Header file for PCH SMM Initialization Driver.

**/


#ifndef _PCH_INIT_SMM_H_
#define _PCH_INIT_SMM_H_


#include <Protocol/SmmSxDispatch.h>
#include <Protocol/SmmSwDispatch.h>
#include <Protocol/SmmIchnDispatch.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <IndustryStandard/Pci30.h>
#include <PchAccess.h>
#include <Library/PchPlatformLib.h>
#include <Protocol/PchPlatformPolicy.h>
#include <Protocol/SmmIchnDispatchEx.h>
#include "Library\Uefilib.h"

extern  EFI_SMM_SX_DISPATCH_PROTOCOL              *mSxDispatch;
extern  EFI_SMM_SW_DISPATCH_PROTOCOL              *mSwDispatch;
extern  DXE_PCH_PLATFORM_POLICY_PROTOCOL       *PchPlatformPchPolicy;

#define    TIMER_1MS      1000

#pragma pack(push, 1)
typedef struct {
  UINT32 RegisterIndex;
  UINT32 RegisterData;
} XHCI_MMIO_REGISTER_INFO;
#pragma pack(pop)


/**
  Initializes the PCH SMM handler for PCH save and restore

  @param[in] ImageHandle - Handle for the image of this driver
  @param[in] SystemTable - Pointer to the EFI System Table

  @retval EFI_SUCCESS    - PCH SMM handler was installed
**/
EFI_STATUS
EFIAPI
PchInitLateSmm (
  IN      EFI_HANDLE            ImageHandle,
  IN      EFI_SYSTEM_TABLE      *SystemTable
  );

#endif
