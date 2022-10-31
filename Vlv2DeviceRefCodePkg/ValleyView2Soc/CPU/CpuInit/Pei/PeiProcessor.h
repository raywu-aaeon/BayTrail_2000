/*++

Copyright (c)  1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PeiProcessor.h

Abstract:

  Definitions / etc that aren't (too) dependant on the processor in the system

--*/

#ifndef _PEI_PROCESSOR_H_
#define _PEI_PROCESSOR_H_

#ifdef ECP_FLAG
#include "EdkIIGluePeim.h"
#undef AllocatePool
#include "PchRegs.h"
#include <Ppi/SecPlatformInformation/SecPlatformInformation.h>
#else
#include <PiPei.h>

#include <Ppi/SecPlatformInformation.h>
#endif
#include <Ppi/Cache.h>
#ifndef ECP_FLAG
#include <Ppi/MasterBootMode.h>
#endif
#include <Guid/HtBistHob.h>
#ifndef ECP_FLAG
#include <Library/HobLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#endif

#include "CpuRegs.h"
#include "CpuType.h"

typedef struct {
  UINT32  RegEax;
  UINT32  RegEbx;
  UINT32  RegEcx;
  UINT32  RegEdx;
} EFI_CPUID_REGISTER;

#include <Guid/PlatformCpuInfo.h>

#define EFI_CU_HP_PC_PEI_INIT                     (EFI_SUBCLASS_SPECIFIC | 0x00000010)
#define EFI_CU_HP_PC_PEI_STEP1                    (EFI_SUBCLASS_SPECIFIC | 0x00000011)

#define PLATFORM_SUPPORTED_CPU_SOCKET_NUMBER 0x1 // 1

#ifndef PCI_EXPRESS_BASE_ADDRESS
#define PCI_EXPRESS_BASE_ADDRESS 0xE0000000
#endif

#define MmPciAddress( Segment, Bus, Device, Function, Register ) \
  ( (UINTN)PCI_EXPRESS_BASE_ADDRESS + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register) \
  )

#define MmPci16Ptr( Segment, Bus, Device, Function, Register ) \
  ( (volatile UINT16 *)MmPciAddress( Segment, Bus, Device, Function, Register ) )

#define MmPci16( Segment, Bus, Device, Function, Register ) \
  *MmPci16Ptr( Segment, Bus, Device, Function, Register )

#endif
