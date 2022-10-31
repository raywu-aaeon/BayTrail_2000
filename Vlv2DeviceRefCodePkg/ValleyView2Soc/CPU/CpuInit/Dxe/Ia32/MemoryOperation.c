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

  MemoryOperation.c

Abstract:

  Memory Operation Functions for IA32 Architecture.

Revision History

--*/

//#include "Tiano.h"
//#include "EfiDriverLib.h"
//#include "CpuIA32.h"
#include "CpuDxe.h"
#include "MpCommon.h"
#include "PlatformMpService.h"

#ifdef ECP_FLAG
#include <Protocol/Legacy8259/Legacy8259.h>
#else
#include <Protocol/Legacy8259.h>
#endif

VOID
InitializeIdt (
  IN EFI_CPU_INTERRUPT_HANDLER      *TableStart,
  IN UINTN                          *IdtTablePtr,
  IN UINT16                         IdtTableLimit
  );

extern EFI_CPU_INTERRUPT_HANDLER   mExternalVectorTable[];
extern EFI_PHYSICAL_ADDRESS        mBackupBuffer;

UINT64   mValidMtrrAddressMask = EFI_CACHE_VALID_ADDRESS;
UINT64   mValidMtrrBitsMask    = EFI_MSR_VALID_MASK;

EFI_STATUS
InitializeSlick (
  VOID
  )
/*++

Routine Description:

  Slick around interrupt routines.

Arguments:

  None

Returns:

  EFI_SUCCESS - If interrupt settings are initialized successfully

--*/
{
  EFI_STATUS                      Status;
  INTERRUPT_GATE_DESCRIPTOR       *IdtTable;
  EFI_LEGACY_8259_PROTOCOL        *Legacy8259;

  INTERRUPT_HANDLER_TEMPLATE_MAP  TemplateMap;
  UINT16                          CodeSegment;
  INTERRUPT_GATE_DESCRIPTOR       *IdtEntry;
  UINT8                           *InterruptHandler;
  UINT8                           *CurrentHandler;
  UINTN                           Index;

  IdtTable  = AllocatePool (sizeof (INTERRUPT_GATE_DESCRIPTOR) * INTERRUPT_VECTOR_NUMBER);
  IdtEntry  = IdtTable;

  GetTemplateAddressMap (&TemplateMap);
  InterruptHandler  = AllocatePool (TemplateMap.Size * INTERRUPT_VECTOR_NUMBER);
  CurrentHandler    = InterruptHandler;

  CodeSegment       = GetCodeSegment ();

  for (Index = 0; Index < INTERRUPT_VECTOR_NUMBER; Index++) {
    CopyMem (CurrentHandler, TemplateMap.Start, TemplateMap.Size);
    *(UINT32 *) (CurrentHandler + TemplateMap.FixOffset)  = Index;

    IdtEntry[Index].OffsetLow = (UINT16) (UINTN) CurrentHandler;
    IdtEntry[Index].SegmentSelector = CodeSegment;
    IdtEntry[Index].Attributes = INTERRUPT_GATE_ATTRIBUTE;
    //
    // 8e00;
    //
    IdtEntry[Index].OffsetHigh = (UINT16) ((UINTN) CurrentHandler >> 16);

    CurrentHandler += TemplateMap.Size;
  }
  //
  // Find the Legacy8259 protocol.
  //
  Status = gBS->LocateProtocol (&gEfiLegacy8259ProtocolGuid, NULL, (VOID **) &Legacy8259);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Legacy8259->SetMode (Legacy8259, Efi8259ProtectedMode, NULL, NULL);

  InitializeIdt (
    &(mExternalVectorTable[0]),
    (UINTN *) IdtTable,
    sizeof (INTERRUPT_GATE_DESCRIPTOR) * INTERRUPT_VECTOR_NUMBER
    );

  return EFI_SUCCESS;
}

EFI_STATUS
PrepareMemory (
  VOID
  )
/*++

Routine Description:

  Prepare memory for essential system tables.

Arguments:

  None.

Returns:

  EFI_SUCCESS              - Memory successfully prepared.
  Other                    - Error occurred while initializating memory.

--*/
{
  EFI_STATUS  Status;

  ZeroMem (mExternalVectorTable, 0x100 * 4);

  //
  // Initialize the Interrupt Descriptor Table
  //
  Status = InitializeSlick ();

  return Status;
}

EFI_STATUS
PrepareMemoryForAPs (
  OUT EFI_PHYSICAL_ADDRESS       *WakeUpBuffer,
  OUT VOID                       **StackAddressStart,
  IN UINTN                       MaximumCPUsForThisSystem
  )
/*++

Routine Description:

  Prepare Wakeup Buffer and stack for APs.

Arguments:

  WakeUpBuffer             - Pointer to the address of wakeup buffer for output.
  StackAddressStart        - Pointer to the stack address of APs for output.
  MaximumCPUsForThisSystem - Maximum CPUs in this system.

Returns:

  EFI_SUCCESS              - Memory successfully prepared for APs.
  Other                    - Error occurred while allocating memory.

--*/
{
  EFI_STATUS                                  Status;
  MP_ASSEMBLY_ADDRESS_MAP                     AddressMap;

  //
  // Release All APs with a lock and wait for them to retire to rendezvous procedure.
  // We need a 64 aligned 4K aligned area for IA-32 to use broadcast APIs. But we need it only
  // on a temporary basis.
  //
  Status = AllocateWakeUpBuffer (WakeUpBuffer);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Claim memory for AP stack.
  //
#ifdef ECP_FLAG
  Status = (gBS->AllocatePool) (
#else
  Status = gBS->AllocatePool (
#endif
                  EfiACPIMemoryNVS,
                  MaximumCPUsForThisSystem * STACK_SIZE_PER_PROC,
                  StackAddressStart
                  );
  if (EFI_ERROR (Status)) {
#ifdef ECP_FLAG
    (gBS->FreePages) (*WakeUpBuffer, 1);
#else
    gBS->FreePages (*WakeUpBuffer, 1);
#endif
    return Status;
  }

  AsmGetAddressMap (&AddressMap);
  CopyMem ((VOID *) (UINTN) *WakeUpBuffer, AddressMap.RendezvousFunnelAddress, AddressMap.Size);
  *(UINT32 *) (UINTN) (*WakeUpBuffer + AddressMap.FlatJumpOffset + 3) = (UINT32) (*WakeUpBuffer + AddressMap.PModeEntryOffset);

  return EFI_SUCCESS;
}

EFI_STATUS
PrepareExchangeInfo (
  OUT MP_CPU_EXCHANGE_INFO           *ExchangeInfo,
  IN  VOID                           *StackAddressStart,
  IN  VOID                           *ApFunction,
  IN  EFI_PHYSICAL_ADDRESS           WakeUpBuffer
  )
/*++

Routine Description:

  Prepare exchange information for APs.

Arguments:

  ExchangeInfo      - Pointer to the exchange info buffer for output.
  StackAddressStart - Start address of APs' stacks.
  ApFunction        - Address of function assigned to AP.
  WakeUpBuffer      - Pointer to the address of wakeup buffer.

Returns:

  EFI_SUCCESS       - Exchange Info successfully prepared for APs.

--*/
{
#ifdef ECP_FLAG
  (gBS->SetMem) ((VOID *) ExchangeInfo, EFI_PAGE_SIZE - MP_CPU_EXCHANGE_INFO_OFFSET, 0);
#else
  gBS->SetMem ((VOID *) ExchangeInfo, EFI_PAGE_SIZE - MP_CPU_EXCHANGE_INFO_OFFSET, 0);
#endif

  ExchangeInfo->Lock        = VacantFlag;
  ExchangeInfo->StackStart  = StackAddressStart;
  ExchangeInfo->StackSize   = STACK_SIZE_PER_PROC;
  ExchangeInfo->ApFunction  = ApFunction;

  CopyMem (&ExchangeInfo->GdtrProfile, (VOID *) (UINTN) mAcpiCpuData->GdtrProfile, sizeof (IA32_DESCRIPTOR));
  CopyMem (&ExchangeInfo->IdtrProfile, (VOID *) (UINTN) mAcpiCpuData->IdtrProfile, sizeof (IA32_DESCRIPTOR));

  ExchangeInfo->BufferStart = (UINT32) WakeUpBuffer;
  ExchangeInfo->InitFlag    = 1;

  return EFI_SUCCESS;
}

EFI_STATUS
S3PrepareMemoryForAPs (
  OUT EFI_PHYSICAL_ADDRESS       *WakeUpBuffer,
  OUT VOID                       **StackAddressStart
  )
/*++

Routine Description:

  Prepare Wakeup Buffer and stack for APs during S3.

Arguments:

  WakeUpBuffer      - Pointer to the address of wakeup buffer for output.
  StackAddressStart - Pointer to the stack address of APs for output.

Returns:

  EFI_SUCCESS       - Memory successfully prepared for APs.

--*/
{
  MP_ASSEMBLY_ADDRESS_MAP        AddressMap;

  *WakeUpBuffer      = mAcpiCpuData->WakeUpBuffer;
  *StackAddressStart = (VOID *) (UINTN) mAcpiCpuData->StackAddress;

  AsmGetAddressMap (&AddressMap);
  CopyMem ((VOID *) (UINTN) *WakeUpBuffer, AddressMap.RendezvousFunnelAddress, AddressMap.Size);
  *(UINT32 *) (UINTN) (*WakeUpBuffer + AddressMap.FlatJumpOffset + 3) = (UINT32) (*WakeUpBuffer + AddressMap.PModeEntryOffset);

  return EFI_SUCCESS;
}

EFI_STATUS
S3PrepareExchangeInfo (
  OUT MP_CPU_EXCHANGE_INFO           *ExchangeInfo,
  IN  VOID                           *StackAddressStart,
  IN  VOID                           *ApFunction,
  IN  EFI_PHYSICAL_ADDRESS           WakeUpBuffer
  )
/*++

Routine Description:

  Prepare exchange information for APs during S3.

Arguments:

  ExchangeInfo      - Pointer to the exchange info for output.
  StackAddressStart - Start address of APs' stacks.
  ApFunction        - Address of function assigned to AP.
  WakeUpBuffer      - Pointer to the address of wakeup buffer.

Returns:

  EFI_SUCCESS       - Exchange Info successfully prepared for APs.

--*/
{
  ExchangeInfo->Lock            = VacantFlag;
  ExchangeInfo->StackStart      = (VOID *) (UINTN) StackAddressStart;
  ExchangeInfo->StackSize       = STACK_SIZE_PER_PROC;
  ExchangeInfo->ApFunction      = ApFunction;

  CopyMem (&ExchangeInfo->GdtrProfile, (VOID *) (UINTN) mAcpiCpuData->GdtrProfile, sizeof (IA32_DESCRIPTOR));
  CopyMem (&ExchangeInfo->IdtrProfile, (VOID *) (UINTN) mAcpiCpuData->IdtrProfile, sizeof (IA32_DESCRIPTOR));

  ExchangeInfo->BufferStart     = (UINT32) WakeUpBuffer;
  ExchangeInfo->InitFlag        = 2;

  //
  // There is no need to initialize CpuNumber and BistBuffer fields in ExchangeInfo here.
  //
  return EFI_SUCCESS;
}

VOID
RedirectFarJump (
  VOID
  )
/*++

Routine Description:

  Dynamically write the far jump destination in APs' wakeup buffer,
  in order to refresh APs' CS registers for mode switching.

Arguments:

  None.

Returns:

  None

--*/
{
  MP_ASSEMBLY_ADDRESS_MAP    AddressMap;

  AsmGetAddressMap (&AddressMap);
  *(UINT32 *) (UINTN) (mAcpiCpuData->WakeUpBuffer + AddressMap.FlatJumpOffset + 3) = (UINT32) (mAcpiCpuData->WakeUpBuffer + AddressMap.PModeEntryOffset);
  return;
}

UINTN
SetIdtEntry (
  IN  UINTN                       FunctionPointer,
  OUT INTERRUPT_GATE_DESCRIPTOR   *IdtEntry
  )
/*++

Routine Description:

  Set specified IDT entry with given function pointer.

Arguments:

  FunctionPointer - Function pointer for IDT entry.
  IdtEntry        - The IDT entry to update.

Returns:

  The original IDT entry value.

--*/
{
  UINTN  OriginalEntry;

  OriginalEntry = ((UINT32) IdtEntry->OffsetHigh << 16) + IdtEntry->OffsetLow;

  IdtEntry->OffsetLow  = (UINT16) FunctionPointer;
  IdtEntry->OffsetHigh = (UINT16) (FunctionPointer >> 16);

  return OriginalEntry;
}

EFI_STATUS
PrepareGdtIdtForAP (
  OUT IA32_DESCRIPTOR          *Gdtr,
  OUT IA32_DESCRIPTOR          *Idtr
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Gdtr  - GC_TODO: add argument description
  Idtr  - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  INTERRUPT_GATE_DESCRIPTOR *IdtForAP;
  SEGMENT_DESCRIPTOR        *GdtForAP;

  IA32_DESCRIPTOR         *IdtrForBSP;
  IA32_DESCRIPTOR         *GdtrForBSP;

  UINT8                     *MceHandler;
  EFI_STATUS                Status;

  AsmGetGdtrIdtr (&GdtrForBSP, &IdtrForBSP);

  //
  // Allocate reserved memory for IDT
  //
  Status = AllocateAlignedReservedMemory (
             IdtrForBSP->Limit + 1,
             8,
             (VOID **) &IdtForAP
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Allocate reserved memory for GDT
  //
  Status = AllocateAlignedReservedMemory (
             GdtrForBSP->Limit + 1,
             8,
             (VOID **) &GdtForAP
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
#ifdef ECP_FLAG
  Status = (gBS->AllocatePool) (
#else
  Status = gBS->AllocatePool (
#endif
                  EfiACPIMemoryNVS,
                  SIZE_OF_MCE_HANDLER,
                  (VOID **) &MceHandler
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // McheHandler content: iret (opcode = 0xcf)
  //
  *MceHandler = 0xCF;

  CopyMem (GdtForAP, (VOID *) GdtrForBSP->Base, GdtrForBSP->Limit + 1);
  CopyMem (IdtForAP, (VOID *) IdtrForBSP->Base, IdtrForBSP->Limit + 1);

  IdtForAP[INTERRUPT_HANDLER_MACHINE_CHECK].OffsetLow   = (UINT16) (UINTN) MceHandler;
  IdtForAP[INTERRUPT_HANDLER_MACHINE_CHECK].OffsetHigh  = (UINT16) ((UINTN) MceHandler >> 16);

  //
  // Create Gdtr, IDTR profile
  //
  Gdtr->Base  = (UINTN) GdtForAP;
  Gdtr->Limit = GdtrForBSP->Limit;

  Idtr->Base  = (UINTN) IdtForAP;
  Idtr->Limit = IdtrForBSP->Limit;

  return EFI_SUCCESS;
}

