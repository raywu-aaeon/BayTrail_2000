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

  LegacyRegion.c

Abstract:

  This code provides a private implementation of the Legacy Region protocol.

--*/

#include "LegacyRegion.h"
#include "Valleyview.h"
#include "VlvAccess.h"

//
// Module Global:
//  Since this driver will only ever produce one instance of the Private Data
//  protocol you are not required to dynamically allocate the PrivateData.
//
LEGACY_REGION_INSTANCE mPrivateData;


EFI_STATUS
LegacyRegionManipulation (
  IN  EFI_LEGACY_REGION_PROTOCOL  *This,
  IN  UINT32                      Start,
  IN  UINT32                      Length,
  IN  UINT32                      Mode,
  OUT UINT32                      *Granularity  OPTIONAL
  )
/*++

Routine Description:
  Modify PAM registers for region specified to MODE state.

Arguments:

  This    - Pointer to EFI_LEGACY_REGION_PROTOCOL instance.

  Start   - Starting address of a memory region covered by HUNIT HMISC2 registers (C0000h - FFFFFh).

  Length  - Memory region length.

  Mode    - Action to perform on a PAM region: UNLOCK, LOCK or BOOTLOCK.

  Granularity - Granularity of region in bytes.

Returns:

  EFI_SUCCESS - PAM region(s) state modified as requested.

--*/
{
  // UNLOCK = normal operation (read/write to DRAM)
  // LOCK = reads to DRAM, writes to DMI
  // BOOTLOCK = LOCK = reads to DRAM, writes to DMI

  // Since there is no concept of write routing for legacy
  // regions in CDV, we will always assume that the caller
  // is requesting reads to be directed to DRAM.  The Mode parameter
  // is therefore ignored.

  UINT32  Data = 0xFFFFFFFF;

  if ((Start < 0xF0000) && ((Start + Length - 1) >= 0xE0000)) {
    //F-segment are routed to DRAM
    MsgBus32Or(VLV_BUNIT, BUNIT_BMISC, Data, B_BMISC_RFSDRAM );
  }

  if ((Start < 0x100000) && ((Start + Length - 1) >= 0xF0000)) {
    //E-segment are routed to DRAM
    MsgBus32Or(VLV_BUNIT, BUNIT_BMISC, Data, B_BMISC_RESDRAM );
  }

  if (Granularity) {
    *Granularity = 64 * 1024;   // All regions are 64K.
  }

  return EFI_SUCCESS;
}


EFI_STATUS
LegacyRegionDecode (
  IN  EFI_LEGACY_REGION_PROTOCOL  *This,
  IN  UINT32                      Start,
  IN  UINT32                      Length,
  IN  BOOLEAN                     *On
  )
/*++
Routine Description:
  Enable/Disable decoding of the given region

Arguments:
  Start  - Starting address of region.
  Length - Length of region in bytes.
  On     - 0 = Disable decoding, 1 = Enable decoding.

Returns:

  EFI_SUCCESS - Decoding change affected.

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
LegacyRegionBootLock (
  IN  EFI_LEGACY_REGION_PROTOCOL   *This,
  IN  UINT32                       Start,
  IN  UINT32                       Length,
  OUT UINT32                       *Granularity  OPTIONAL
  )
/*++

Routine Description:
  Make the indicated region read from RAM / write to ROM.

Arguments:
  Start       - Starting address of region.
  Length      - Length of region in bytes.
  Granularity - Granularity of region in bytes.

Returns:

  EFI_SUCCESS - Region locked or made R/O.

--*/
{
  return LegacyRegionManipulation(This, Start, Length, BOOTLOCK, Granularity);
}

EFI_STATUS
LegacyRegionLock (
  IN  EFI_LEGACY_REGION_PROTOCOL   *This,
  IN  UINT32                       Start,
  IN  UINT32                       Length,
  OUT UINT32                       *Granularity  OPTIONAL
  )
/*++

Routine Description:
  Make the indicated region read from RAM / write to ROM.

Arguments:
  Start       - Starting address of region.
  Length      - Length of region in bytes.
  Granularity - Granularity of region in bytes.

Returns:

  EFI_SUCCESS - Region locked or made R/O.

--*/
{
  return LegacyRegionManipulation(This, Start, Length, LOCK, Granularity);
}

EFI_STATUS
LegacyRegionUnlock (
  IN  EFI_LEGACY_REGION_PROTOCOL   *This,
  IN  UINT32                       Start,
  IN  UINT32                       Length,
  OUT UINT32                       *Granularity  OPTIONAL
  )
/*++

Routine Description:
  Make the indicated region read from RAM / write to RAM.

Arguments:
  Start       - Starting address of region.
  Length      - Length of region in bytes.
  Granularity - Granularity of region in bytes.

Returns:

  EFI_SUCCESS - Region unlocked or made R/W.

--*/
{
  return LegacyRegionManipulation(This, Start, Length, UNLOCK, Granularity);
}

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
{
  LEGACY_REGION_INSTANCE        *LegacyRegion;

  LegacyRegion = &mPrivateData;

  //
  // Grab a copy of all the protocols we depend on. Any error would
  // be a dispatcher bug!.
  //

  LegacyRegion->Signature                      = LEGACY_REGION_INSTANCE_SIGNATURE;
  LegacyRegion->LegacyRegion.Decode            = LegacyRegionDecode;
  LegacyRegion->LegacyRegion.Lock              = LegacyRegionLock;
  LegacyRegion->LegacyRegion.BootLock          = LegacyRegionBootLock;
  LegacyRegion->LegacyRegion.UnLock            = LegacyRegionUnlock;
  LegacyRegion->ImageHandle                    = ImageHandle;

  //
  // Make a new handle and install the protocol
  //
  LegacyRegion->Handle = NULL;
  return gBS->InstallProtocolInterface (
                &LegacyRegion->Handle,
                &gEfiLegacyRegionProtocolGuid,
                EFI_NATIVE_INTERFACE,
                &LegacyRegion->LegacyRegion
                );
}
