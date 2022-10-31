/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2006 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  HeciDrv.h

Abstract:

  Definitions for HECI driver

--*/
#ifndef _HECI_DRV_H
#define _HECI_DRV_H
#ifdef ECP_FLAG

#include "EdkIIGlueDxe.h"
#endif
#include "HeciHpet.h"
#include "Hecicore.h"
#include <SeCAccess.h>
#include <MkhiMsgs.h>
#include <HeciRegs.h>
#ifdef ECP_FLAG
#include "Pci22.h"
#include <Protocol/PciRootBridgeIo/PciRootBridgeIo.h>
#include <Protocol/SeCPlatformPolicy.h>
//
// Driver Produced Protocol Prototypes
//
#include <Protocol/Heci.h>
#include <Protocol/SeCRcInfo.h>
#include <Protocol/TdtOperation.h>
#else
#include <IndustryStandard/Pci22.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PciLib.h>
#include <Library/SeCLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/SeCPlatformPolicy.h>

//
// Driver Produced Protocol Prototypes
//
#include <Protocol/Heci.h>
#include <Protocol/SeCRcInfo.h>
#include <Protocol/TdtOperation.h>
#endif
#define HECI_PRIVATE_DATA_SIGNATURE         SIGNATURE_32 ('H', 'e', 'c', 'i')
#define HECI_ROUND_UP_BUFFER_LENGTH(Length) ((UINT32) ((((Length) + 3) / 4) * 4))

//
// HECI private data structure
//
typedef struct {
  UINTN                   Signature;
  EFI_HANDLE              Handle;
  UINT32                  HeciMBAR;
  UINT32                  HeciMBAR0;
  UINT16                  DeviceInfo;
  UINT32                  RevisionInfo;
  EFI_HECI_PROTOCOL       HeciCtlr;
  volatile UINT32         *HpetTimer;
  EFI_SEC_RC_INFO_PROTOCOL SeCRcInfo;
} HECI_INSTANCE;
#endif
