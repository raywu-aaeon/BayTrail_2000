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
#ifndef _EFI_PEI_SMM_CONTROL_DRIVER_H_
#define _EFI_PEI_SMM_CONTROL_DRIVER_H_

///
/// Driver private data
///
#ifdef ECP_FLAG
#include "EdkIIGluePeim.h"
#else
#include <PiPei.h>
#endif
#include "Ppi/SmmControl.h"
#ifndef ECP_FLAG
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#endif
#include <PchAccess.h>


///
/// Prototypes
///
EFI_STATUS
EFIAPI
SmmControlPeiDriverEntryInit (
#ifdef ECP_FLAG 
  IN EFI_FFS_FILE_HEADER                  *FileHandle,
  IN EFI_PEI_SERVICES                     **PeiServices
#else
  IN      EFI_PEI_FILE_HANDLE             FfsHeader,
  IN      CONST EFI_PEI_SERVICES          **PeiServices
#endif
  )
/**

  @brief
  This is the constructor for the SMM Control ppi

  @param[in] FfsHeader            FfsHeader.
  @param[in] PeiServices          General purpose services available to every PEIM.

  @retval EFI_STATUS              Results of the installation of the SMM Control Ppi

**/
;

EFI_STATUS
EFIAPI
PeiActivate (
  IN       EFI_PEI_SERVICES          **PeiServices,
  IN       PEI_SMM_CONTROL_PPI       *This,
  IN OUT  INT8                       *ArgumentBuffer OPTIONAL,
  IN OUT  UINTN                      *ArgumentBufferSize OPTIONAL,
  IN      BOOLEAN                    Periodic OPTIONAL,
  IN      UINTN                      ActivationInterval OPTIONAL
  )
/**

  @brief
  This routine generates an SMI

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] This                 The EFI SMM Control ppi instance
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
PeiDeactivate (
  IN EFI_PEI_SERVICES             **PeiServices,
  IN PEI_SMM_CONTROL_PPI          *This,
  IN  BOOLEAN                     Periodic OPTIONAL
  )
/**

  @brief
  This routine clears an SMI

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] This                 The EFI SMM Control ppi instance
  @param[in] Periodic             Periodic or not

  @retval EFI Status              Describing the result of the operation
  @retval EFI_INVALID_PARAMETER   Some parameter value passed is not supported

**/
;

#endif
