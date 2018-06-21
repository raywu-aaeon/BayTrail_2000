/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

  @file
  PchPcieSmm.h

  @brief
  PCH Pcie SMM Driver Header

**/
#ifndef _PCH_PCIE_SMM_H
#define _PCH_PCIE_SMM_H

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#endif
#include <Protocol/PchPlatformPolicy.h>
#include <Protocol/SmmIchnDispatchEx.h>
#ifdef ECP_FLAG
#include "EfiScriptLib.h"
#include <Protocol/BootScriptSave/BootScriptSave.h>
#else
#include <Library/S3BootScriptLib.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#endif

#include "PchCommonDefinitions.h"
#include "PchAccess.h"
#include "Library/PchPlatformLib.h"
#include "Library/PchPciExpressHelpersLib.h"

//
// Prototypes
//
EFI_STATUS
PchPcieSmi (
  IN  UINT8         PciePortNum
  )
/**

  @brief
  Program Common Clock and ASPM of Downstream Devices

  @param[in] PciePortNum          Pcie Root Port Number

  @retval EFI_SUCCESS             Function complete successfully

**/
;

VOID
PchPcieSmiHandlerFunction (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_EX_CONTEXT        *DispatchContext
  )
/**

  @brief
  PCIE Hotplug SMI call back function for each Root port

  @param[in] DispatchHandle       Handle of this dispatch function
  @param[in] DispatchContext      Pointer to the dispatch function's context.
                                  The DispatchContext fields are filled in by the dispatching driver
                                  prior to invoke this dispatch function

  @retval EFI_SUCCESS             Function complete successfully

**/
;

VOID
PchPcieLinkActiveStateChange (
  IN  EFI_HANDLE                              DispatchHandle,
  IN  EFI_SMM_ICHN_DISPATCH_EX_CONTEXT        *DispatchContext
  )
/**

  @brief
  PCIE Link Active State Change Hotplug SMI call back function for all Root ports

  @param[in] DispatchHandle       Handle of this dispatch function
  @param[in] DispatchContext      Pointer to the dispatch function's context.
                                  The DispatchContext fields are filled in by the dispatching driver
                                  prior to invoke this dispatch function

  @retval EFI_SUCCESS             Function complete successfully

**/
;

EFI_STATUS
EFIAPI
InitializePchPcieSmm (
  IN      EFI_HANDLE            ImageHandle,
  IN      EFI_SYSTEM_TABLE      *SystemTable
  )
/**

  @brief
  Register PCIE Hotplug SMI dispatch function to handle Hotplug enabling

  @param[in] ImageHandle          The image handle of this module
  @param[in] SystemTable          The EFI System Table

  @retval EFI_SUCCESS             The function completes successfully

**/
;
#endif
