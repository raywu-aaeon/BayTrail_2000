/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c) 2004 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PchSpi.h

Abstract:

  Header file for the PCH SPI Runtime Driver.

--*/
#ifndef _PCH_SPI_H_
#define _PCH_SPI_H_

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#endif
#include "SpiCommon.h"
#include <Protocol/Spi.h>
#ifdef ECP_FLAG
#include "DxeRuntimePciLibPciExpress.h"
#include "EfiScriptLib.h"
#include <Protocol/BootScriptSave/BootScriptSave.h>
#else
#include <Protocol/BootScriptSave.h>
#endif

#define PCI_LIB_ADDRESS(Bus,Device,Function,Register)   \
  (((Register) & 0xfff) | (((Function) & 0x07) << 12) | (((Device) & 0x1f) << 15) | (((Bus) & 0xff) << 20))
//
// Function prototypes used by the SPI protocol.
//
VOID
PchSpiVirtualddressChangeEvent (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
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

VOID
EFIAPI
SpiPhaseInit (
  VOID
  )
/*++
Routine Description:

  This function is a hook for Spi Dxe phase specific initialization

Arguments:

  None

Returns:

  None

--*/
;
#endif
