/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  Bist.h

Abstract:

  Describes the functions visible to the rest of the CpuPeim.
--*/

#ifndef _CPU_BIST_H_
#define _CPU_BIST_H_

EFI_STATUS
EFIAPI
BuildBistHob (
  IN EFI_PEI_SERVICES           **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR  *NotifyDescriptor,
  IN VOID                       *Ppi
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  PeiServices       - GC_TODO: add argument description
  NotifyDescriptor  - GC_TODO: add argument description
  Ppi               - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

#endif
