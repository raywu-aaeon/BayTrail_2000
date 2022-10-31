/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    PchReset.h

Abstract:

  Some definitions for PCHx reset

--*/
#ifndef _PCH_RESET_H
#define _PCH_RESET_H

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include <Guid/EventGroup/EventGroup.h>
#include <ArchProtocol/Reset/Reset.h>
#else
#include <PiDxe.h>
#include <Guid/CapsuleVendor.h>
#include <Guid/EventGroup.h>
#include <Protocol/Reset.h>
#endif
#include <Protocol/PchExtendedReset.h>
#ifndef ECP_FLAG
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#endif
#include "PchAccess.h"
#include <Library/BaseLib.h>
//
// Driver private data
//
#define PCH_RESET_SIGNATURE SIGNATURE_32 ('I', 'E', 'R', 'S')

typedef struct {
  UINT32                          Signature;
  EFI_HANDLE                      Handle;
  EFI_PCH_EXTENDED_RESET_PROTOCOL PchExtendedResetProtocol;
#if ((TIANO_RELEASE_VERSION != 0) && (EFI_SPECIFICATION_VERSION < 0x00020000))
  CHAR16                          *CapsuleVariableName;
#endif
  UINTN                           PmcBase;
  UINTN                           AcpiBar;
} PCH_RESET_INSTANCE;

#define PCH_RESET_INSTANCE_FROM_THIS(a) \
  CR ( \
  a, \
  PCH_RESET_INSTANCE, \
  PchExtendedResetProtocol, \
  PCH_RESET_SIGNATURE \
  )

EFI_STATUS
EFIAPI
InitializePchReset (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Initialize the state information for the Timer Architectural Protocol

Arguments:

  ImageHandle             Image handle of the loaded driver
  SystemTable             Pointer to the System Table

Returns:

  EFI_SUCCESS             Thread can be successfully created
  EFI_OUT_OF_RESOURCES    Cannot allocate protocol data structure
  EFI_DEVICE_ERROR        Cannot create the timer service

--*/
;

VOID
EFIAPI
IntelPchResetSystem (
  IN EFI_RESET_TYPE   ResetType,
  IN EFI_STATUS       ResetStatus,
  IN UINTN            DataSize,
  IN VOID             *ResetData OPTIONAL
  )
/*++

Routine Description:

  Reset the system.

Arguments:

  ResetType             Warm or cold
  ResetStatus           Possible cause of reset
  DataSize              Size of ResetData in bytes
  ResetData             Optional Unicode string

Returns:

  Does not return if the reset takes place.
  EFI_INVALID_PARAMETER   If ResetType is invalid.

--*/
;

#if ((TIANO_RELEASE_VERSION != 0) && (EFI_SPECIFICATION_VERSION < 0x00020000))
VOID
CapsuleReset (
  IN UINTN   CapsuleDataPtr
  )
/*++

Routine Description:

  If need be, do any special reset required for capsules. For this
  implementation where we're called from the ResetSystem() api,
  just set our capsule variable and return to let the caller
  do a soft reset.

Arguments:

  CapsuleDataPtr          pointer to the capsule block descriptors

Returns:

  None

--*/
;
#endif

EFI_STATUS
EFIAPI
PchExtendedReset (
  IN     EFI_PCH_EXTENDED_RESET_PROTOCOL   *This,
  IN     PCH_EXTENDED_RESET_TYPES          PchExtendedResetTypes
  )
/*++

Routine Description:

  Execute Pch Extended Reset from the host controller.

Arguments:

  This                    Pointer to the EFI_PCH_EXTENDED_RESET_PROTOCOL instance.
  PchExtendedResetTypes   Pch Extended Reset Types which includes PowerCycle, Globalreset.

Returns:

  EFI_SUCCESS             Successfully completed.
  EFI_INVALID_PARAMETER   If ResetType is invalid.

--*/
;

VOID
PchResetVirtualddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  )
/*++

Routine Description:

  Fixup internal data pointers so that the services can be called in virtual mode.

Arguments:

  Event     The event registered.
  Context   Event context. Not used in this event handler.

Returns:

  None.

--*/
;
#endif
