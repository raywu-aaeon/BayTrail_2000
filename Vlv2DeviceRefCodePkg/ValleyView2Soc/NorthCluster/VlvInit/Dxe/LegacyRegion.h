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

  LegacyRegion.h

Abstract:

  This code supports a the private implementation of the Legacy Region protocol.

--*/

#ifndef _LEGACY_REGION_H_
#define _LEGACY_REGION_H_

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#else
#include <Protocol/LegacyRegion.h>
#include <Library/UefiBootServicesTableLib.h>
#endif


#define LEGACY_REGION_INSTANCE_SIGNATURE   SIGNATURE_32('R','E','G','N')

#define UNLOCK            0x0000
#define LOCK              0x0001
#define BOOTLOCK          0x0002

typedef struct {
  UINT32                  Signature;
  EFI_HANDLE              Handle;
  EFI_LEGACY_REGION_PROTOCOL      LegacyRegion;
  EFI_HANDLE              ImageHandle;
} LEGACY_REGION_INSTANCE;

#define LEGACY_REGION_INSTANCE_FROM_THIS (this) \
  CR (this, LEGACY_REGION_INSTANCE, LegacyRegion,LEGACY_REGION_INSTANCE_SIGNATURE)

EFI_STATUS
LegacyRegionInstall (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:
  Install Driver to produce Legacy Region protocol.

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns:

  EFI_SUCCESS - Legacy Region protocol installed

  Other       - No protocol installed, unload driver.

--*/
;
#endif
