//
// This file contains an 'Intel Pre-EFI Module' and is licensed
// for Intel CPUs and Chipsets under the terms of your license
// agreement with Intel or your vendor.  This file may be
// modified by the user, subject to additional terms of the
// license agreement
//
/** @file
  The header file of Platform PEIM.

Copyright (c) 2010 Intel Corporation.<BR>
All rights reserved.  This software and associated documentation
(if any) is furnished under a license and may only be used or
copied in accordance with the terms of the license.  Except as
permitted by such license, no part of this software or
documentation may be reproduced, stored in a retrieval system, or transmitted
in any form or by any means without the express written consent of Intel Corporation.

**/


#ifndef __PEI_PLATFORM_H__
#define __PEI_PLATFORM_H__

#include <Ppi/MemoryDiscovered.h>
#include <Library/HobLib.h>
#include <Library/MtrrLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PcdLib.h>
#include <Library/DebugLib.h>

/**
  This function will be called when MRC is done.

  @param  PeiServices General purpose services available to every PEIM.

  @param  NotifyDescriptor Information about the notify event..

  @param  Ppi The notify context.

  @retval EFI_SUCCESS If the function completed successfully.
**/
EFI_STATUS
EFIAPI
MemoryDiscoveredPpiNotifyCallback(
    IN EFI_PEI_SERVICES                     **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR            *NotifyDescriptor,
    IN VOID                                 *Ppi
);

#endif
