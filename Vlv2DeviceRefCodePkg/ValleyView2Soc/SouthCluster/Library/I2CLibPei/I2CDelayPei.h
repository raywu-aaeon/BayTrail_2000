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

#ifndef __I2C_DELAY_PEI__

#define __I2C_DELAY_PEI__
#ifdef ECP_FLAG
#include <EdkIIGlueBase.h>
#else
#include "PiPei.h"
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
  );

#endif
