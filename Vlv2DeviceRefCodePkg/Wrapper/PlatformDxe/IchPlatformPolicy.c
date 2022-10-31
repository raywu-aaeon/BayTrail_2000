/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PchPlatformPolicy.c

Abstract:


--*/

#include "PlatformDxe.h"
#include <Protocol/PchPlatformPolicy.h>
#include <Protocol/VlvPlatformPolicy.h>
//#include "Protocol/GlobalNvsArea.h"
//#include "Protocol/DxePchPolicyUpdateProtocol.h"

EFI_STATUS
EFIAPI
ScPolicyInitDxe (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable,
    IN SB_SETUP_DATA            *PchPolicyData
  );

EFI_STATUS
EFIAPI
NcPolicyInitDxe (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable,
    IN NB_SETUP_DATA            *VlvPolicyData
  );

VOID
InitPchPlatformPolicy (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable,
    IN SB_SETUP_DATA            *PchPolicyData
  )
/*++

Routine Description:
  Updates the feature policies according to the setup variable.

Arguments:

Returns:
  VOID

--*/
{
  // PCH Policy Initialization based on Setup variable.
  ScPolicyInitDxe(ImageHandle, SystemTable, PchPolicyData);

}

VOID
InitVlvPlatformPolicy (
    IN EFI_HANDLE               ImageHandle,
    IN EFI_SYSTEM_TABLE         *SystemTable,
    IN NB_SETUP_DATA            *VlvPolicyData
  )
{
  // Vlv Policy Initialization based on Setup variable.
  NcPolicyInitDxe(ImageHandle, SystemTable, VlvPolicyData);

}
