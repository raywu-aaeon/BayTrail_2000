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
  ActiveBios.h

  @brief
  Defines and prototypes for the ActiveBios driver.
  This driver implements the ActiveBios protocol for the PCH.
  It provides a simple implementation that allows for basic control
  of the PCH flash mapping state.

**/
#ifndef _ACTIVE_BIOS_H_
#define _ACTIVE_BIOS_H_

#include "PchAccess.h"
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#else
#include "Base.h"
#endif

#ifndef ECP_FLAG
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#endif
#include <Library/PchPlatformLib.h>
#ifndef ECP_FLAG
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#endif
#include <Protocol/ActiveBiosProtocol.h>

//
// Active BIOS private data
//
#define ACTIVE_BIOS_SIGNATURE SIGNATURE_32 ('D', 'P', 'B', 'A')

typedef struct {
  UINT32                    Signature;
  EFI_HANDLE                Handle;
  EFI_ACTIVE_BIOS_PROTOCOL  ActiveBiosProtocol;
  UINTN                     CurrentKey;
  BOOLEAN                   Locked;
} ACTIVE_BIOS_INSTANCE;

#define ACTIVE_BIOS_INSTANCE_FROM_ACTIVE_BIOS_THIS(a) \
  CR ( \
  a, \
  ACTIVE_BIOS_INSTANCE, \
  ActiveBiosProtocol, \
  ACTIVE_BIOS_SIGNATURE \
  )

///
/// Driver global data
///
extern ACTIVE_BIOS_INSTANCE mPrivateData;
extern UINT32               mPchRootComplexBar;

///
/// Protocol constructor
///
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
;

///
/// Driver entry point
///
EFI_STATUS
InstallActiveBios (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/**

  @brief
  ActiveBios driver entry point function.

  @param[in] ImageHandle          Image handle for this driver image
  @param[in] SystemTable          Pointer to the EFI System Table

  @retval EFI_SUCCESS             Application completed successfully
  @exception EFI_UNSUPPORTED      Unsupported chipset detected

**/
;

#endif
