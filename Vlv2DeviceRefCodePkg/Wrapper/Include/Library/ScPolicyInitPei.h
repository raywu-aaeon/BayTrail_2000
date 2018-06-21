/**
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
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
  ScPolicyInitPei.h

  @brief
  Header file for the PchPeiPolicy PEIM.

**/
#ifndef _PCH_POLICY_INIT_PEI_H_
#define _PCH_POLICY_INIT_PEI_H_

#include "PchAccess.h"
#include <Ppi/PchPlatformPolicy.h>
#include <Setup.h>
#include <Library/SbPolicy.h>
#include <Token.h>
#include <Library/AmiChipsetIoLib.h>

///
/// Functions
///
EFI_STATUS
EFIAPI
ScPolicyInitPei(
    IN CONST EFI_PEI_SERVICES           **PeiServices,
    IN SB_SETUP_DATA                    *PchPolicyData
)
/**

  @brief
  This PEIM performs PCH PEI Platform Policy initialzation.

  @param[in] FfsHeader            Pointer to Firmware File System file header.
  @param[in] PeiServices          General purpose services available to every PEIM.

  @retval EFI_SUCCESS             The PPI is installed and initialized.
  @retval EFI ERRORS              The PPI is not successfully installed.
  @retval EFI_OUT_OF_RESOURCES    Do not have enough resources to initialize the driver

**/
;
#endif
