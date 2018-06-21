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
  ActiveBios.c

  @brief
  Source file for the ActiveBios ActiveBios protocol implementation

**/
#include "ActiveBios.h"

///
/// Prototypes for our ActiveBios protocol functions
///
static
EFI_STATUS
EFIAPI
SetState (
  IN EFI_ACTIVE_BIOS_PROTOCOL     *This,
  IN EFI_ACTIVE_BIOS_STATE        DesiredState,
  IN UINTN                        Key
  );

static
EFI_STATUS
EFIAPI
LockState (
  IN     EFI_ACTIVE_BIOS_PROTOCOL   *This,
  IN     BOOLEAN                    Lock,
  IN OUT UINTN                      *Key
  );

///
/// Function implementations
///
static
EFI_STATUS
EFIAPI
SetState (
  IN EFI_ACTIVE_BIOS_PROTOCOL     *This,
  IN EFI_ACTIVE_BIOS_STATE        DesiredState,
  IN UINTN                        Key
  )
/**

  @brief
  Change the current active BIOS settings to the requested state.
  The caller is responsible for requesting a supported state from
  the EFI_ACTIVE_BIOS_STATE selections.
  This will fail if someone has locked the interface and the correct key is
  not provided.

  @param[in] This                 Pointer to the EFI_ACTIVE_BIOS_PROTOCOL instance.
  @param[in] DesiredState         The requested state to configure the system for.
  @param[in] Key                  If the interface is locked, Key must be the Key
                                  returned from the LockState function call.

  @retval EFI_SUCCESS             The function completed successfully
  @retval EFI_ACCESS_DENIED       The interface is currently locked.

**/
{
  ///
  /// Verify requested state is allowed
  ///
  ASSERT (DesiredState < ActiveBiosStateMax);

  ///
  /// Check if the interface is locked by another
  ///
  if (mPrivateData.Locked && Key != mPrivateData.CurrentKey) {
    return EFI_ACCESS_DENIED;
  }

  if ((MmioRead32 (mPchRootComplexBar + R_PCH_RCRB_GCS) & B_PCH_RCRB_GCS_BILD) == B_PCH_RCRB_GCS_BILD) {
    return EFI_ACCESS_DENIED;
  }
  ///
  /// Set the requested state
  ///
  switch (DesiredState) {

    case ActiveBiosStateSpi:
      MmioAndThenOr16 (
        (UINTN) (mPchRootComplexBar + R_PCH_RCRB_GCS),
        (UINT16) (~B_PCH_RCRB_GCS_BBS),
        (UINT16) (V_PCH_RCRB_GCS_BBS_SPI)
        );
      break;

    case ActiveBiosStatePci:
      ///
      /// ActiveBiosStatePci has been obsolete by the protocol
      /// since Valleyview
      ///
      ASSERT (!EFI_UNSUPPORTED);
      break;

    case ActiveBiosStateLpc:
      MmioAndThenOr16 (
        (UINTN) (mPchRootComplexBar + R_PCH_RCRB_GCS),
        (UINT16) (~B_PCH_RCRB_GCS_BBS),
        (UINT16) (V_PCH_RCRB_GCS_BBS_LPC)
        );
      break;

    default:
      ///
      /// This is an invalid use of the protocol
      /// See definition, but caller must call with valid value
      ///
      ASSERT (!EFI_UNSUPPORTED);
      break;
  }
  ///
  /// Read state back
  /// This ensures the chipset MMIO was flushed and updates the protocol state
  ///
  MmioRead16 (mPchRootComplexBar + R_PCH_RCRB_GCS);

  ///
  /// Record current state
  ///
  mPrivateData.ActiveBiosProtocol.State = DesiredState;

  return EFI_SUCCESS;
}

static
EFI_STATUS
EFIAPI
LockState (
  IN     EFI_ACTIVE_BIOS_PROTOCOL   *This,
  IN     BOOLEAN                    Lock,
  IN OUT UINTN                      *Key
  )
/**

  @brief
  Lock or unlock the current active BIOS state.
  Key is a simple incrementing number.

  @param[in] This                 Pointer to the EFI_ACTIVE_BIOS_PROTOCOL instance.
  @param[in] Lock                 TRUE to lock the current state, FALSE to unlock.
  @param[in] Key                  If Lock is TRUE, then a key will be returned.  If
                                  Lock is FALSE, the key returned from the prior call
                                  to lock the protocol must be provided to unlock the
                                  protocol.  The value of Key is undefined except that
                                  it cannot be 0.

  @retval EFI_SUCCESS             Command succeed.
  @exception EFI_UNSUPPORTED      The function is not supported.
  @retval EFI_ACCESS_DENIED       The interface is currently locked.

**/
{
  ///
  /// Check if lock or unlock requesed
  ///
  if (Lock) {
    ///
    /// Check if already locked
    ///
    if (mPrivateData.Locked) {
      return EFI_ACCESS_DENIED;
    }
    ///
    /// Lock the interface
    ///
    mPrivateData.Locked = TRUE;

    ///
    /// Increment the key
    ///
    mPrivateData.CurrentKey++;

    ///
    /// Update the caller's copy
    ///
    *Key = mPrivateData.CurrentKey;
  } else {
    ///
    /// Verify caller "owns" the current lock
    ///
    if (*Key == mPrivateData.CurrentKey) {
      mPrivateData.Locked = FALSE;
    } else {
      return EFI_ACCESS_DENIED;
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
ActiveBiosProtocolConstructor (
  IN  EFI_ACTIVE_BIOS_PROTOCOL      *This
  )
/**

  @brief
  Initialization function for the ActiveBios protocol implementation.

  @param[in] This                 Pointer to the protocol

  @retval EFI_SUCCESS             The function completed successfully

**/
{
  EFI_STATUS  Status;
  EFI_HANDLE  Handle;
  EFI_GUID    EfiActiveBiosProtocolGuid = EFI_ACTIVE_BIOS_PROTOCOL_GUID;

  ///
  /// Read current state from the PCH
  ///
  switch (MmioRead16 (mPchRootComplexBar + R_PCH_RCRB_GCS) & B_PCH_RCRB_GCS_BBS) {

    case V_PCH_RCRB_GCS_BBS_SPI:
      mPrivateData.ActiveBiosProtocol.State = ActiveBiosStateSpi;
      break;

    case V_PCH_RCRB_GCS_BBS_LPC:
      mPrivateData.ActiveBiosProtocol.State = ActiveBiosStateLpc;
      break;

    default:
      ///
      /// ActiveBiosStatePci has been obsolete by the protocol
      /// since Valleyview
      ///
      /// This is an invalid use of the protocol
      /// See definition, but caller must call with valid value
      ///
      ASSERT (!EFI_UNSUPPORTED);
      break;
  }

  mPrivateData.ActiveBiosProtocol.SetState  = SetState;
  mPrivateData.ActiveBiosProtocol.LockState = LockState;
  mPrivateData.CurrentKey                   = 1;
  mPrivateData.Locked                       = FALSE;

  ///
  /// Install the protocol
  ///
  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &EfiActiveBiosProtocolGuid,
                  &mPrivateData.ActiveBiosProtocol,
                  NULL
                  );
  return Status;
}
