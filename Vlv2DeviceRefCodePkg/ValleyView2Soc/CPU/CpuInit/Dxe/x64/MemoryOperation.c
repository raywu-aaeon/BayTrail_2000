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


--*/

#include "CpuDxe.h"
#include "PlatformCpuLib.h"
#include "MpCommon.h"
#include "VirtualMemory.h"
#include "MemoryAttribute.h"

VOID
InitializeExternalVectorTablePtr(
  EFI_CPU_INTERRUPT_HANDLER* VectorTable
);

extern EFI_CPU_INTERRUPT_HANDLER   mExternalVectorTable[];
extern EFI_PHYSICAL_ADDRESS        mBackupBuffer;
extern EFI_PLATFORM_CPU_INFO       mPlatformCpuInfo;

UINT8    *mPageStore = NULL;
UINTN    mPageStoreSize = 16;
UINTN    mPageStoreIndex = 0;

UINT64   mValidMtrrAddressMask;
UINT64   mValidMtrrBitsMask;

//
// BugBug: Non Portable
//
#define ALINE_16BYTE_BOUNDRY  __declspec(align(16))

#pragma pack (1)
typedef  struct {
  UINT16                     LimitLow;
  UINT16                     BaseLow;
  UINT8                      BaseMiddle;
  UINT8                      Attributes1;
  UINT8                      Attributes2;
  UINT8                      BaseHigh;

 // UINT32                     BaseAddressHighest32;
 // UINT32                     Reserved;
} SEGMENT_DESCRIPTOR_x64;

typedef struct {
  UINT16              Limit;
  UINTN               Base;
} PSEUDO_DESCRIPTOR_x64;

#pragma pack()


ALINE_16BYTE_BOUNDRY SEGMENT_DESCRIPTOR_x64  gGdt[] = {
  {
    // NULL Selector: selector[0]
    0,  // limit 15:0
    0,  // base  15:0
    0,  // base  23:16
    0,  //
    0,  // type & limit 19:16
    0,  // base  31:24
//   0,  // base  63:32
//   0   // reserved
  },
  {
    // Linear Selector: selector[8]
    0xffff,  // limit 15:0
    0,       // base  15:0
    0,       // base  23:16
    0x92,    //present, ring 0, data, expand-up writable
    0xcf,    // type & limit 19:16 
    0,       // base  31:24
//   0,      // base  63:32
//   0       // reserved
  },
  {
    // Linear code Selector: selector[10]
    0xffff,  // limit 15:0
    0,       // base  15:0
    0,       // base  23:16
    0x9a,    // present, ring 0, code, expand-up writable
    0xaf,    // type & limit 19:16   
    0,       // base  31:24
  //  0,     // base  63:32
  //  0      // reserved
  },
  {
    // Compatibility mode data Selector: selector[18]
    0xffff,  // limit 15:0
    0,       // base  15:0
    0,       // base  23:16
    0x92,    // type & limit 19:16
    0xcf,
    0,       // base  31:24
  // 0,      // base  63:32
  // 0       // reserved
  },
  {
    // Compatibility code Selector: selector[20]
    0xffff,  // limit 15:0
    0,       // base  15:0
    0,       // base  23:16
    0x9a,    // type & limit 19:16
    0xcf,
    0,       // base  31:24
  //  0,     // base  63:32
  //  0      // reserved
  },
  {
    // Spare3 Selector: selector[28]
    0,  // limit 15:0
    0,  // base  15:0
    0,  // base  23:16
    0,  // type & limit 19:16
    0,  // base  31:24
    0,
  //  0,  // base  63:32
  //  0   // reserved
  },
  {
    // 64-bit data Selector:selector[30]
    0xffff,  // limit 15:0
    0,       // base  15:0
    0,       // base  23:16
    0x92,    // type & limit 19:16
    0xcf,
    0,       // base  31:24
  //  0,     // base  63:32
  //  0      // reserved
  },
  {
    // 64-bit code Selector: selector[38]
    0xffff,  // limit 15:0
    0,       // base  15:0
    0,       // base  23:16
    0x9a,    // type & limit 19:16
    0xaf,
    0,       // base  31:24
  //  0,     // base  63:32
  //  0      // reserved
  },
  {
    // Spare3 Selector: selector[40]
    0,       // limit 15:0
    0,       // base  15:0
    0,       // base  23:16
    0,       // type & limit 19:16
    0,       // base  31:24
    0,
  //  0,     // base  63:32
  //  0      // reserved
  }
};

ALINE_16BYTE_BOUNDRY PSEUDO_DESCRIPTOR_x64 gGdtPseudoDescriptor = {
  sizeof (gGdt) - 1,
  (UINTN)gGdt
};


INTERRUPT_GATE_DESCRIPTOR   gIdtTable[INTERRUPT_VECTOR_NUMBER] = { 0 };

ALINE_16BYTE_BOUNDRY PSEUDO_DESCRIPTOR_x64 gLidtPseudoDescriptor = {
  sizeof (gIdtTable) - 1,
  (UINTN)gIdtTable
};


VOID
InitializeSelectors (
  VOID
  )
{
  CpuLoadGlobalDescriptorTable (&gGdtPseudoDescriptor);
}

VOID
AsmIdtVector00 (
  VOID
  );

VOID
InitializeInterruptTables (
  VOID
  )
/*++

Routine Description:

  Slick around interrupt routines.

Arguments:

  None

Returns:

  None


--*/
{
  UINT16                         CodeSegment;
  INTERRUPT_GATE_DESCRIPTOR      *IdtEntry;
  UINT8                          *CurrentHandler;
  UINT32                         Index;

  CodeSegment = CpuCodeSegment ();

  IdtEntry = gIdtTable;
  CurrentHandler = (UINT8 *)(UINTN)AsmIdtVector00;
  for (Index = 0; Index < INTERRUPT_VECTOR_NUMBER; Index ++) {
    IdtEntry[Index].Offset15To0       = (UINT16)(UINTN)CurrentHandler;
    IdtEntry[Index].SegmentSelector   = CodeSegment;
    IdtEntry[Index].Attributes        = INTERRUPT_GATE_ATTRIBUTE; //8e00;
    IdtEntry[Index].Offset31To16      = (UINT16)((UINTN)CurrentHandler >> 16);
    IdtEntry[Index].Offset63To32      = (UINT32)((UINTN)CurrentHandler >> 32);

    CurrentHandler += 0x8;
    //
  }

  CpuLoadInterruptDescriptorTable (&gLidtPseudoDescriptor);

  return;
}

VOID
InitailizeMemoryAttributes (
  VOID
  )
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  Page;
  UINT32                MsrNum, MsrNumEnd;
  UINT64                TempQword;
  UINT64                ComplementBits;

#ifdef ECP_FLAG
  Status = (gBS->AllocatePages) (
#else
  Status = gBS->AllocatePages (
#endif
                 AllocateAnyPages, 
                 EfiBootServicesData, 
                 mPageStoreSize,
                 &Page
                 );
  ASSERT_EFI_ERROR (Status);

  mPageStore = (UINT8 *)(UINTN)Page;

  ZeroMem (mPageStore, 0x1000 * mPageStoreSize);

  mValidMtrrBitsMask    = (((UINT64) 1) << mPlatformCpuInfo.CpuAddress.PhysicalBits) - 1;
  mValidMtrrAddressMask = mValidMtrrBitsMask & 0xfffffffffffff000;

  MsrNumEnd = EFI_MSR_CACHE_VARIABLE_MTRR_BASE + (2 * (UINT32)(AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP) & B_EFI_MSR_IA32_MTRR_CAP_VARIABLE_SUPPORT)); //803 is put after PreMtrrChange()
  ComplementBits = mValidMtrrBitsMask & 0xfffffff000000000;
  if (ComplementBits != 0) {
    PreMtrrChange ();
    for (MsrNum = EFI_MSR_CACHE_VARIABLE_MTRR_BASE; MsrNum < MsrNumEnd; MsrNum += 2) {
      TempQword = AsmReadMsr64 (MsrNum + 1);
      if ((TempQword & B_EFI_MSR_CACHE_MTRR_VALID) != 0) {
        //
        // MTRR Physical Mask
        //
        TempQword = TempQword | ComplementBits;
        AsmWriteMsr64 (MsrNum + 1, TempQword);
      }
    }
    PostMtrrChange ();
  }
}

VOID  *
AllocateZeroedPage (
  VOID
  )
{
  if (mPageStoreIndex >= mPageStoreSize) {
    //
    // We are out of space
    //
    return NULL;
  }

  return (VOID *)(UINTN)&mPageStore[0x1000 * mPageStoreIndex++];
}

VOID
Convert2MBPageTo4KPages (
  IN      EFI_PHYSICAL_ADDRESS        PageAddress,
  IN OUT  x64_PAGE_TABLE_ENTRY        **PageDirectoryToConvert
  )
{
  UINTN                                       Index;
  EFI_PHYSICAL_ADDRESS                        WorkingAddress;
  x64_PAGE_TABLE_ENTRY_4K                     *PageTableEntry;
  x64_PAGE_TABLE_ENTRY                        Attributes;

  
  //
  // Save the attributes of the 2MB table
  //
  Attributes.Page2Mb.Uint64 = (*PageDirectoryToConvert)->Page2Mb.Uint64;

  //
  // Convert PageDirectoryEntry2MB into a 4K Page Directory
  //
  PageTableEntry = AllocateZeroedPage ();
  if (PageTableEntry == NULL) {
  	return;
  }
  (*PageDirectoryToConvert)->Page2Mb.Uint64 = (UINT64)PageTableEntry;
  (*PageDirectoryToConvert)->Page2Mb.Bits.ReadWrite = 1;
  (*PageDirectoryToConvert)->Page2Mb.Bits.Present = 1;

  WorkingAddress = PageAddress;
  for (Index = 0; Index < 512; Index++, PageTableEntry++, WorkingAddress += 0x1000) {
    PageTableEntry->Uint64 = (UINT64)WorkingAddress;
    PageTableEntry->Bits.Present = 1;

    //
    // Update the new page to have the same attributes as the 2MB page
    //
    PageTableEntry->Bits.ReadWrite = Attributes.Common.ReadWrite;
    PageTableEntry->Bits.CacheDisabled = Attributes.Common.CacheDisabled;
    PageTableEntry->Bits.WriteThrough  = Attributes.Common.WriteThrough;

    if (WorkingAddress == PageAddress) {
      //
      // Return back the 4K page that matches the Working addresss
      //
      *PageDirectoryToConvert = (x64_PAGE_TABLE_ENTRY *)PageTableEntry;
    }
  }
}

EFI_STATUS
GetCurrentMapping (
  IN  EFI_PHYSICAL_ADDRESS    BaseAddress,
  OUT x64_PAGE_TABLE_ENTRY    **PageTable,
  OUT BOOLEAN                 *Page2MBytes
  )
{
  UINT64                                        Cr3;
  x64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K   *PageMapLevel4Entry;
  x64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K   *PageDirectoryPointerEntry;
  x64_PAGE_TABLE_ENTRY_2M                     *PageTableEntry2Mb;
  x64_PAGE_DIRECTORY_ENTRY_4K                 *PageDirectoryEntry4k;
  x64_PAGE_TABLE_ENTRY_4K                     *PageTableEntry4k;
  UINTN                                       Pml4Index;
  UINTN                                       PdpIndex;
  UINTN                                       Pde2MbIndex;
  UINTN                                       PteIndex;

  Cr3 = CpuReadCr3 ();
  
  PageMapLevel4Entry = (x64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K *)(Cr3 & 0x000ffffffffff000);

  Pml4Index = (UINTN)RShiftU64 (BaseAddress, 39) & 0x1ff;
  if (PageMapLevel4Entry[Pml4Index].Bits.Present == 0) {
    return EFI_NOT_FOUND;
  }
  PageDirectoryPointerEntry = (x64_PAGE_MAP_AND_DIRECTORY_POINTER_2MB_4K *)(PageMapLevel4Entry[Pml4Index].Uint64 & 0x000ffffffffff000);
  PdpIndex = (UINTN)RShiftU64 (BaseAddress, 30) & 0x1ff;
  if (PageDirectoryPointerEntry[PdpIndex].Bits.Present == 0) {
    return EFI_NOT_FOUND;
  }

  PageTableEntry2Mb = (x64_PAGE_TABLE_ENTRY_2M *)(PageDirectoryPointerEntry[PdpIndex].Uint64 & 0x000ffffffffff000);
  Pde2MbIndex = (UINTN)RShiftU64 (BaseAddress, 21) & 0x1ff;
  if (PageTableEntry2Mb[Pde2MbIndex].Bits.Present == 0) {
    return EFI_NOT_FOUND;
  }

  if (PageTableEntry2Mb[Pde2MbIndex].Bits.MustBe1 == 1) {
    //
    // We found a 2MByte page so lets return it
    //
    *Page2MBytes = TRUE;
    *PageTable = (x64_PAGE_TABLE_ENTRY *)&PageTableEntry2Mb[Pde2MbIndex].Uint64;
    return EFI_SUCCESS;
  }

  //
  // 4K page so keep walking
  //
  PageDirectoryEntry4k = (x64_PAGE_DIRECTORY_ENTRY_4K *)&PageTableEntry2Mb[Pde2MbIndex].Uint64;

  PageTableEntry4k = (x64_PAGE_TABLE_ENTRY_4K *)(PageDirectoryEntry4k[Pde2MbIndex].Uint64 & 0x000ffffffffff000);
  PteIndex = (UINTN)RShiftU64 (BaseAddress, 12) & 0x1ff;
  if (PageTableEntry4k[PteIndex].Bits.Present == 0) {
    return EFI_NOT_FOUND;
  }

  *Page2MBytes = FALSE;
  *PageTable = (x64_PAGE_TABLE_ENTRY *)&PageTableEntry4k[PteIndex];
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

--*/
{
  //
  // Allocate space to convert 2MB page tables to 4K tables.
  //  This can not be done a call time as the TPL level will
  //  not be correct.
  //
  InitailizeMemoryAttributes ();

  InitializeExternalVectorTablePtr (mExternalVectorTable);
  //
  // Initialize the Interrupt Descriptor Table
  //
  InitializeInterruptTables ();

  return EFI_SUCCESS;
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
  Status = AllocateReservedMemoryBelow4G (
             (MaximumCPUsForThisSystem + 1) * STACK_SIZE_PER_PROC,
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
  *(UINT32 *) (UINTN) (*WakeUpBuffer + AddressMap.LongJumpOffset + 2) = (UINT32) (*WakeUpBuffer + AddressMap.LModeEntryOffset);

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

  ExchangeInfo->Lock              = VacantFlag;
  ExchangeInfo->StackStart        = StackAddressStart;
  ExchangeInfo->StackSize         = STACK_SIZE_PER_PROC;
  ExchangeInfo->ApFunction        = ApFunction;

  CopyMem ((VOID *) (UINTN) &ExchangeInfo->GdtrProfile, (VOID *) (UINTN) mAcpiCpuData->GdtrProfile, sizeof (IA32_DESCRIPTOR));
  CopyMem ((VOID *) (UINTN) &ExchangeInfo->IdtrProfile, (VOID *) (UINTN) mAcpiCpuData->IdtrProfile, sizeof (IA32_DESCRIPTOR));

  ExchangeInfo->BufferStart       = (UINT32) WakeUpBuffer;
  ExchangeInfo->Cr3               = (UINT32) (AsmGetCr3 ());
  ExchangeInfo->InitFlag   = 1;

  return EFI_SUCCESS;
}

EFI_STATUS
S3PrepareMemoryForAPs (
  OUT EFI_PHYSICAL_ADDRESS       *WakeUpBuffer,
  OUT VOID                       **StackAddressStart
  )
/*++

Routine Description:

  Prepare Wakeup Buffer and stack for APs.

Arguments:

  WakeUpBuffer      - Pointer to the address of wakeup buffer for output.
  StackAddressStart - Pointer to the stack address of APs for output.

Returns:

  EFI_SUCCESS       - Memory successfully prepared for APs.

--*/
{
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

  Prepare exchange information for APs.

Arguments:

  ExchangeInfo      - Pointer to the exchange info for output.
  StackAddressStart - Start address of APs' stacks.
  ApFunction        - Address of function assigned to AP.
  WakeUpBuffer      - Pointer to the address of wakeup buffer.

Returns:

  EFI_SUCCESS       - Exchange Info successfully prepared for APs.

--*/
{
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
  *(UINT32 *) (UINTN) (mAcpiCpuData->WakeUpBuffer + AddressMap.LongJumpOffset + 2) = (UINT32) (mAcpiCpuData->WakeUpBuffer + AddressMap.LModeEntryOffset);

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

  OriginalEntry = ((UINT64) IdtEntry->Offset63To32 << 32) + ((UINT32) IdtEntry->Offset31To16 << 16) + IdtEntry->Offset15To0;

  IdtEntry->Offset15To0  = (UINT16) FunctionPointer;
  IdtEntry->Offset31To16 = (UINT16) (FunctionPointer >> 16);
  IdtEntry->Offset63To32 = (UINT32) (FunctionPointer >> 32);

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

  IA32_DESCRIPTOR           *IdtrForBSP;
  IA32_DESCRIPTOR           *GdtrForBSP;

  UINT16                    *MceHandler;
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
  *MceHandler = 0xCF48;

  CopyMem (GdtForAP, (VOID *) GdtrForBSP->Base, GdtrForBSP->Limit + 1);
  CopyMem (IdtForAP, (VOID *) IdtrForBSP->Base, IdtrForBSP->Limit + 1);

  IdtForAP[INTERRUPT_HANDLER_MACHINE_CHECK].Offset15To0  = (UINT16) (UINTN) MceHandler;
  IdtForAP[INTERRUPT_HANDLER_MACHINE_CHECK].Offset31To16 = (UINT16) ((UINTN) MceHandler >> 16);
  IdtForAP[INTERRUPT_HANDLER_MACHINE_CHECK].Offset63To32 = (UINT32) ((UINTN) MceHandler >> 32);

  //
  // Create Gdtr, IDTR profile
  //
  Gdtr->Base  = (UINTN) GdtForAP;
  Gdtr->Limit = GdtrForBSP->Limit;

  Idtr->Base  = (UINTN) IdtForAP;
  Idtr->Limit = IdtrForBSP->Limit;

  return EFI_SUCCESS;
}
