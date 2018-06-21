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
  PchInitPeim.h

  @brief
  Header file for the PCH Init PEIM

**/
#ifndef _PCH_INIT_PEIM_H_
#define _PCH_INIT_PEIM_H_

#ifdef ECP_FLAG
#include "EdkIIGluePeim.h"
#endif
#include "PchAccess.h"

#include <Ppi/PchInit.h>
#ifdef ECP_FLAG
#include <Ppi/MemoryDiscovered/MemoryDiscovered.h>
#else
#include <Ppi/MemoryDiscovered.h>
#endif

#include <Ppi/PchUsbPolicy.h>
#include <Ppi/PchPlatformPolicy.h>
#include <Library/PchPlatformLib.h>
#include "../Common/PchUsbCommon.h"

#ifndef ECP_FLAG
#include <PiPei.h>
#include <Library/PcdLib.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/PeimEntryPoint.h>
#endif

#ifdef ECP_FLAG
#include <Ppi/Stall/Stall.h>
#else
#include <Ppi/Stall.h>
#endif

#include "PlatformBaseAddresses.h"

extern EFI_GUID               gS3ResumeDonePpiGuid;
#include <Library/BaseMemoryLib.h>
#include <Setup.h>
#include <Ppi/ReadOnlyVariable2.h>
EFI_STATUS
EFIAPI
PchSataInit (
  IN  EFI_PEI_SERVICES            **PeiServices
  )
/**

  @brief
  Internal function performing SATA init needed in PEI phase

  @param[in] PeiServices          General purpose services available to every PEIM.

  @retval None

**/
;
EFI_STATUS
EFIAPI
PchInitAfterMemInstall (
  IN  EFI_PEI_SERVICES            **PeiServices,
  IN  EFI_PEI_NOTIFY_DESCRIPTOR   *NotifyDescriptor,
  IN  VOID                        *Ppi
  )
/*++

Routine Description:

  Internal function performing PCH init needed in PEI phase right after memory installed

Arguments:

  PeiServices             General purpose services available to every PEIM.
  NotifyDescriptor        The notification structure this PEIM registered on install.
  Ppi                     The memory discovered PPI.  Not used.

Returns:

  EFI_SUCCESS             The function completed successfully.

--*/
;

EFI_STATUS
PchIoApicInit (
  IN  PCH_PLATFORM_POLICY_PPI     *PchPlatformPolicyPpi
  )
/**

  @brief
  Initialize IOAPIC according to IoApicConfig policy of the PCH
  Platform Policy PPI

  @param[in] PchPlatformPolicyPpi The PCH Platform Policy PPI instance

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_DEVICE_ERROR        Device error, aborts abnormally.

**/
;

EFI_STATUS
EFIAPI
PchInitialize (
  IN EFI_PEI_SERVICES             **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
  IN VOID                         *Ppi
  )
/**

  @brief
  This function performs basic initialization for PCH in PEI phase.
  If any of the base address arguments is zero, this function will disable the corresponding
  decoding, otherwise this function will enable the decoding.
  This function locks down the PMBase.

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] NotifyDescriptor     The notification structure this PEIM registered on install.
  @param[in] Ppi                  The memory discovered PPI.  Not used.

  @retval EFI_SUCCESS             Succeeds.
  @retval EFI_DEVICE_ERROR        Device error, aborts abnormally.

**/
;

EFI_STATUS
EFIAPI
PchUsbInit (
  IN EFI_PEI_SERVICES             **PeiServices
  )
/**

  @brief
  The function performing USB init in PEI phase. This could be used by USB recovery
  or debug features that need USB initialization during PEI phase.
  Note: Before executing this function, please be sure that PCH_INIT_PPI.Initialize
  has been done and PchUsbPolicyPpi has been installed.

  @param[in] PeiServices    General purpose services available to every PEIM

  @retval EFI_SUCCESS       The function completed successfully
  @retval Others            All other error conditions encountered result in an ASSERT.

**/
;


EFI_STATUS
EFIAPI
PchS3ResumeDonePpiNotifyCallback (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  );

#endif
