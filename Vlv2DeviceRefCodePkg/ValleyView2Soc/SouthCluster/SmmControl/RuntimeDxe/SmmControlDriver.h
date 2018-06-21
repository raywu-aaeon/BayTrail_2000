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
  SmmControlDriver.h

  @brief
  Header file for SMM Control Driver.

**/
#ifndef _SMM_CONTROL_DRIVER_H_
#define _SMM_CONTROL_DRIVER_H_

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include <Protocol/SmmControl/SmmControl.h>
#include "Pci22.h"
#else
//
// External include files do NOT need to be explicitly specified in real EDKII
// environment
//
#include <PiDxe.h>
#include <Protocol/SmmControl.h>
#include <IndustryStandard/Pci22.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#endif
#include <PchAccess.h>


#define SMM_CONTROL_PRIVATE_DATA_SIGNATURE  SIGNATURE_32 ('i', '4', 's', 'c')

typedef struct {
  UINTN                     Signature;
  EFI_HANDLE                Handle;
  EFI_SMM_CONTROL_PROTOCOL  SmmControl;
} SMM_CONTROL_PRIVATE_DATA;

#define SMM_CONTROL_PRIVATE_DATA_FROM_THIS(a) CR (a, SMM_CONTROL_PRIVATE_DATA, SmmControl, SMM_CONTROL_DEV_SIGNATURE)

///
/// Prototypes
///
EFI_STATUS
EFIAPI
SmmControlDriverEntryInit (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/**

  @brief
  This is the constructor for the SMM Control protocol

  @param[in] ImageHandle          Handle for the image of this driver
  @param[in] SystemTable          Pointer to the EFI System Table

  @retval EFI_STATUS              Results of the installation of the SMM Control Protocol

**/
;

EFI_STATUS
SmmTrigger (
  UINT8   Data
  )
/**

  @brief
  Trigger the software SMI

  @param[in] Data                 The value to be set on the software SMI data port

  @retval EFI_SUCCESS             Function completes successfully

**/
;

EFI_STATUS
EFIAPI
SmmClear (
  VOID
  )
/**

  @brief
  Clear the SMI status

  @param[in] None

  @retval EFI_SUCCESS             The function completes successfully
  @retval EFI_DEVICE_ERROR        Something error occurred

**/
;

EFI_STATUS
EFIAPI
Activate (
  IN      EFI_SMM_CONTROL_PROTOCOL   * This,
  IN OUT  INT8                       *ArgumentBuffer OPTIONAL,
  IN OUT  UINTN                      *ArgumentBufferSize OPTIONAL,
  IN      BOOLEAN                    Periodic OPTIONAL,
  IN      UINTN                      ActivationInterval OPTIONAL
  )
/**

  @brief
  This routine generates an SMI

  @param[in] This                 The EFI SMM Control protocol instance
  @param[in] ArgumentBuffer       The buffer of argument
  @param[in] ArgumentBufferSize   The size of the argument buffer
  @param[in] Periodic             Periodic or not
  @param[in] ActivationInterval   Interval of periodic SMI

  @retval EFI Status              Describing the result of the operation
  @retval EFI_INVALID_PARAMETER   Some parameter value passed is not supported

**/
;

EFI_STATUS
EFIAPI
Deactivate (
  IN  EFI_SMM_CONTROL_PROTOCOL    *This,
  IN  BOOLEAN                     Periodic OPTIONAL
  )
/**

  @brief
  This routine clears an SMI

  @param[in] This                 The EFI SMM Control protocol instance
  @param[in] Periodic             Periodic or not

  @retval EFI Status              Describing the result of the operation
  @retval EFI_INVALID_PARAMETER   Some parameter value passed is not supported

**/
;

EFI_STATUS
EFIAPI
GetRegisterInfo (
  IN      EFI_SMM_CONTROL_PROTOCOL      *This,
  IN OUT  EFI_SMM_CONTROL_REGISTER      *SmiRegister
  )
/**

  @brief
  This routine gets SMM control register information

  @param[in] This                 The SMM Control protocol instance
  @param[in] SmiRegister          Output parameter: the SMI control register information is returned

  @retval EFI_INVALID_PARAMETER   Parameter SmiRegister is NULL
  @retval EFI_SUCCESS             Function completes successfully

**/
;

VOID
DisablePendingSmis (
  VOID
  )
/**

  @brief
  Disable all pending SMIs

  @param[in] None

  @retval None

**/
;

#endif
