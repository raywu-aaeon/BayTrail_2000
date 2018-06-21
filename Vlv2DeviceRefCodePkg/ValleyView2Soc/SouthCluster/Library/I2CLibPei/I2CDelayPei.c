/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
  ICH9 ACPI Timer implements one instance of Timer Library.

Copyright (c) 2007, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/
#ifdef ECP_FLAG
#include <EdkIIGluePeim.h>
#else
#include "PiPei.h"
#endif
#include "I2CAccess.h"
#include "I2CDelayPei.h"
#ifdef ECP_FLAG
#else
#include <Library/DebugLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Ppi/Stall.h>
#endif



/**
  Stalls the CPU for at least the given number of microseconds.

  Stalls the CPU for the number of microseconds specified by MicroSeconds.

  @param  MicroSeconds  The minimum number of microseconds to delay.

  @return MicroSeconds

**/
EFI_STATUS
EFIAPI
MicroSecondDelay (
  IN      UINTN                     MicroSeconds
  )
{

  EFI_PEI_STALL_PPI              *StallPpi;
  EFI_STATUS                     Status;
#ifdef ECP_FLAG
  EFI_PEI_SERVICES               **PeiServices;
#else
  CONST EFI_PEI_SERVICES         **PeiServices;
#endif
  PeiServices = GetPeiServicesTablePointer ();


  Status = (**PeiServices).LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, (VOID **) &StallPpi);
  ASSERT(!EFI_ERROR(Status));

  StallPpi->Stall (PeiServices, StallPpi, MicroSeconds);

  return EFI_SUCCESS;

}
