/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
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

Module Name:

  VlvInitPeim.h

Abstract:

  Header file for the CV Init PEIM

--*/

#ifndef _VLV_INIT_PEIM_H_
#define _VLV_INIT_PEIM_H_

#include <Token.h>    // AMI_OVERRIDE
#ifdef ECP_FLAG
#include "EdkIIGluePeim.h"
#else
#include <PiPei.h>
#endif
#include <Ppi/VlvPolicy.h>


//
// Functions
//
EFI_STATUS
EFIAPI
VlvInitPeiEntryPoint (
#ifdef ECP_FLAG
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
#else
  IN EFI_PEI_FILE_HANDLE       FfsHeader,
  IN CONST EFI_PEI_SERVICES    **PeiServices
#endif
  )
/*++

Routine Description:

  CV PEI Initialization.

Arguments:

  FfsHeader    - Pointer to Firmware File System file header.
  PeiServices  - General purpose services available to every PEIM.

Returns:

  EFI_SUCCESS

--*/
;

VOID
ProgramEcBase (
  )
/*++

Routine Description:

  ProgramEcBase: Program the EC Base

Arguments: None


Returns:

  None

--*/
;

VOID
ISPConfig (
  IN CONST EFI_PEI_SERVICES                   **PeiServices,
  IN    VLV_POLICY_PPI                        *VlvPolicyPpi
  )
/*++

Routine Description:

  ISPConfig: Program the Iunit as B0D2F0 or B0D3F0

Arguments:
  PeiServices     - Pointer to the PEI services table
  VlvPolicyPpi  - VlvPolicyPpi to access the ISPPciDevConfig


Returns:

  None

--*/
;

VOID InitThermalRegisters()
/*++

Routine Description:

  InitThermalRegisters: Init Thermal Registers

Arguments: None


Returns:

  None

--*/

;
#ifdef SG_SUPPORT  
VOID
SwitchableGraphicsInit (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN VLV_POLICY_PPI             *VlvPolicyPpi
  )
/**
  SwitchableGraphicsInit: Initialize the Switchable Graphics if enabled

  @param[in] PeiServices          - Pointer to the PEI services table
  @param[in] SaPlatformPolicyPpi  - SaPlatformPolicyPpi to access the SgConfig related information
**/
;
#endif

VOID
GraphicsInit (
  IN CONST EFI_PEI_SERVICES                   **PeiServices,
  IN    VLV_POLICY_PPI                        *VlvPolicyPpi
  )
/*++

Routine Description:

  GraphicsInit: Initialize the IGD if no other external graphics is present

Arguments:

  PeiServices     - Pointer to the PEI services table
  VlvPolicyPpi  - VlvPolicyPpi to access the GtConfig related information

Returns:

  None

--*/
;
void CheckNoCpuCoreWarmReset(IN CONST EFI_PEI_SERVICES **PeiServices);

VOID
SSASafeConfiguration (
  )
/*++

Routine Description:

  SSASafeConfiguration: Program the Safe Configuration for SSA

Arguments: None


Returns:

  None

--*/
;

#endif
