/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  SeCUma.h

Abstract:

  Framework PEIM to SeCUma

--*/
#ifndef _PCH_SEC_UMA_H_
#define _PCH_SEC_UMA_H_

#ifdef ECP_FLAG
#include "EdkIIGluePeim.h"
#include "Pci22.h"
#endif
#include <HeciRegs.h>
#include <SeCAccess.h>
#ifndef ECP_FLAG
#include <IndustryStandard/Pci22.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PciLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/PerformanceLib.h>
#endif
#include <Ppi/SeCUma.h>
#ifdef ECP_FLAG
#include <Ppi/Stall/Stall.h>
#else
#include <Ppi/Stall.h>
#endif
#include <Library/BaseLib.h>

#define R_MESEG_BASE  0x70  // Removed in VLV
#define B_EXCLUSION BIT8
#define HECI_BASE_ADDRESS                 0xFD000000
#define HECI2_BASE_ADDRESS                0xFD100000

//
// SEC FW communication timeout value definitions
//
#define DID_TIMEOUT_MULTIPLIER  0x1388


//
// SEC FW HOST ALIVENESS RESP timeout
//
#define HOST_ALIVENESS_RESP_TIMEOUT_MULTIPLIER  0x1388

//
// Function Prototype(s)
//
EFI_STATUS
SeCSendUmaSize (
  IN EFI_PEI_SERVICES **PeiServices
  )
/*++

Routine Description:

  This procedure will read and return the amount of SEC UMA requested
  by SEC ROM from the HECI device.

Arguments:

  PeiServices   - General purpose services available to every PEIM.

Returns:

  Return SEC UMA Size in KBs

--*/
;

EFI_STATUS
SeCConfigDidReg (
#ifdef ECP_FLAG
  IN       EFI_PEI_SERVICES **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES **PeiServices,
#endif
  MRC_BOOT_MODE_T           MrcBootMode,
  UINT8                     InitStat,
  UINT32                    SeCUmaBase,
  UINT32                    SeCUmaSize
  )
/*++

Routine Description:

  This procedure will configure the SEC Host General Status register,
  indicating that DRAM Initialization is complete and SEC FW may
  begin using the allocated SEC UMA space.

Arguments:

  PeiServices - General purpose services available to every PEIM.
  MrcBootMode - MRC BootMode
  InitStat    - H_GS[27:24] Status
  SeCUmaBase  - Memory Location ** must be with in 4GB range

Returns:
  EFI_SUCCESS

--*/
;

EFI_STATUS
HandleSeCBiosAction (
#ifdef ECP_FLAG
  IN       EFI_PEI_SERVICES **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES **PeiServices,
#endif
  UINT8                     BiosAction
  )
/*++

Routine Description:

  This procedure will enforce the BIOS Action that was requested by SEC FW
  as part of the DRAM Init Done message.

Arguments:

  PeiServices - General purpose services available to every PEIM.
  BiosAction  -  SeC requests BIOS to act

Returns:

  Return EFI_SUCCESS

--*/
;

EFI_STATUS
PerformReset (
#ifdef ECP_FLAG
  IN       EFI_PEI_SERVICES **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES **PeiServices,
#endif
  UINT8                     ResetType
  )
/*++

Routine Description:

  This procedure will issue a Non-Power Cycle, Power Cycle, or Global Rest.

Arguments:

  PeiServices - General purpose services available to every PEIM.
  ResetType  -  Type of reset to be issued.

Returns:

  Return EFI_SUCCESS

--*/
;

EFI_STATUS
PrepareMBP (
  IN EFI_PEI_SERVICES **PeiServices
  )
/*++

Routine Description:

  This procedure will handle the "SEC to BIOS Payload".

Arguments:

  PeiServices - General purpose services available to every PEIM.

Returns:

  Return EFI_SUCCESS

--*/
;

EFI_STATUS
ClearDISB (
  VOID
  )
/*++

Routine Description:

  This procedure will clear the DISB.

Arguments:

Returns:

  Return EFI_SUCCESS

--*/
;
EFI_STATUS
isSeCExpose(
  IN EFI_PEI_SERVICES **PeiServices
  )
/*++

Routine Description:

  This procedure will check the exposure of SeC device.

Arguments:

Returns:

  Return EFI_SUCCESS

--*/
;
#endif
