/*++

Copyright (c) 2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  ProcessorDef.h

Abstract:

  Definition for IA32 processor

--*/

#ifndef _PROCESSOR_DEF_H
#define _PROCESSOR_DEF_H

//#include EFI_GUID_DEFINITION (AcpiVariable)

#pragma pack(1)

typedef struct {
  UINT16  OffsetLow;
  UINT16  SegmentSelector;
  UINT16  Attributes;
  UINT16  OffsetHigh;
} INTERRUPT_GATE_DESCRIPTOR;

#pragma pack()

typedef struct {
  UINT8   *RendezvousFunnelAddress;
  UINTN   PModeEntryOffset;
  UINTN   FlatJumpOffset;
  UINTN   Size;
} MP_ASSEMBLY_ADDRESS_MAP;

VOID
AsmGetAddressMap (
  OUT MP_ASSEMBLY_ADDRESS_MAP    *AddressMap
  )
/*++

Routine Description:

  Get address map of RendezvousFunnelProc.

Arguments:

  AddressMap  - Output buffer for address map information

Returns:

  None

--*/
;

#endif
