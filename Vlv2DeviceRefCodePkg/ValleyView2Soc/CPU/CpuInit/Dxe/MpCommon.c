/*++

Copyright (c) 1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  MpCommon.c

Abstract:

  Code which support multi-processor


--*/

#include "MpCommon.h"
#include "CpuDxe.h"
#include "MiscFuncs.h"
#include "Features.h"
#include <Token.h> //AMI_OVERRIDE - EIP166924 Use Isharedisk tools can't into OS with PXE boot, EIP164713 Legacy/UEFI OpROM with CSM.
//#include "SmmDef.h"
//#include "SmmLib.h"
//#include EFI_PROTOCOL_DEFINITION (SmmBase)
//#include EFI_PROTOCOL_DEFINITION (SmmAccess)
//#include EFI_PROTOCOL_DEFINITION (SmmControl)
//#include EFI_PROTOCOL_DEFINITION (ExitPmAuth)

#ifndef EFI_NO_MEMORY_TEST
EFI_GENERIC_MEMORY_TEST_PROTOCOL    *mGenMemoryTest;
#endif

extern MP_SYSTEM_DATA               *mMPSystemData;
extern EFI_PHYSICAL_ADDRESS         mOriginalBuffer;
extern EFI_PHYSICAL_ADDRESS         mBackupBuffer;
extern EFI_METRONOME_ARCH_PROTOCOL  *mMetronome;
extern EFI_PLATFORM_CPU_PROTOCOL    *mPlatformCpu;
extern EFI_PLATFORM_CPU_INFO        mPlatformCpuInfo;
volatile UINTN                      mSwitchToLegacyRegionCount = 0;

EFI_GUID  mSmramCpuNvsHeaderGuid = EFI_SMRAM_CPU_NVS_HEADER_GUID;
CHAR16    EfiPlatformCpuInfoVariable[] = L"PlatformCpuInfo";
BOOLEAN
IsXapicEnabled (
  VOID
  )
{
  UINT64 MsrValue;

  MsrValue = AsmReadMsr64 (EFI_MSR_IA32_APIC_BASE);
  if (MsrValue & B_EFI_MSR_IA32_APIC_BASE_APIC_GLOBAL_ENABLE) {
    if (MsrValue & B_EFI_MSR_IA32_APIC_BASE_M_XAPIC) {
      return TRUE;
    }
  }
  return FALSE;
}

UINT64
ReadApicMsrOrMemory (
  BOOLEAN   XapicEnabled,
  UINT32    MsrIndex,
  UINT64    MemoryMappedIo
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  UINT64  Value;

  if (XapicEnabled) {
    Value = AsmReadMsr64 (MsrIndex);
  } else {
    Value = (UINT64)*(volatile UINT32 *)(UINTN)MemoryMappedIo;
  }
  return Value;
}

VOID
WriteApicMsrOrMemory (
  BOOLEAN   XapicEnabled,
  UINT32    MsrIndex,
  UINT64    MemoryMappedIo,
  UINT64    Value
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  BSP - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  if (XapicEnabled) {
    AsmWriteMsr64 (MsrIndex, Value);
  } else {
    if (MsrIndex == EFI_MSR_EXT_XAPIC_ICR) {
      *(volatile UINT32 *)(UINTN)(MemoryMappedIo - APIC_REGISTER_ICR_LOW_OFFSET + APIC_REGISTER_ICR_HIGH_OFFSET) = (UINT32)(Value >> 32);
    }
    *(volatile UINT32 *)(UINTN)MemoryMappedIo = (UINT32)Value;
  }
}

EFI_STATUS
SendInterrupt (
  IN  UINT32                               BroadcastMode,
  IN  UINT32                               ApicID,
  IN  UINT32                               VectorNumber,
  IN  UINT32                               DeliveryMode,
  IN  UINT32                               TriggerMode,
  IN  BOOLEAN                              Assert
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  BroadcastMode - GC_TODO: add argument description
  ApicID        - GC_TODO: add argument description
  VectorNumber  - GC_TODO: add argument description
  DeliveryMode  - GC_TODO: add argument description
  TriggerMode   - GC_TODO: add argument description
  Assert        - GC_TODO: add argument description

Returns:

  EFI_INVALID_PARAMETER - GC_TODO: Add description for return value
  EFI_NOT_READY - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  UINT64                ApicBaseReg;
  EFI_PHYSICAL_ADDRESS  ApicBase;
  UINT32                ICRLow;
  UINT32                ICRHigh;
  BOOLEAN               XapicEnabled;

  if (SINGLE_THREAD_BOOT_FLAG != 0) {
    //
    // Only for Debug to use Single Thread Boot
    //
    return EFI_SUCCESS;
  }

  //
  // Initialze ICR high dword, since P6 family processor needs
  // the destination field to be 0x0F when it is a broadcast
  //
  ICRHigh = 0x0f000000;
  ICRLow  = VectorNumber | (DeliveryMode << 8);

  if (TriggerMode == TRIGGER_MODE_LEVEL) {
    ICRLow |= 0x8000;
  }

  if (Assert) {
    ICRLow |= 0x4000;
  }

  XapicEnabled = IsXapicEnabled ();

  switch (BroadcastMode) {
    case BROADCAST_MODE_SPECIFY_CPU:
      if (XapicEnabled) {
        ICRHigh = (UINT32)ApicID;
      } else {
        ICRHigh = ApicID << 24;
      }
      break;

    case BROADCAST_MODE_ALL_INCLUDING_SELF:
      ICRLow |= 0x80000;
      break;

    case BROADCAST_MODE_ALL_EXCLUDING_SELF:
      ICRLow |= 0xC0000;
      break;

    default:
      return EFI_INVALID_PARAMETER;
  }

  ApicBaseReg = AsmReadMsr64 (EFI_MSR_IA32_APIC_BASE);
  ApicBase    = ApicBaseReg & B_EFI_MSR_IA32_APIC_BASE_APIC_BASE_ADDRESS;

  //
  // According Nehalem BWG, if Extended XAPIC Mode is enabled,
  // legacy xAPIC is no longer working.
  // So, previous MMIO offset must be transferred to MSR offset R/W.
  // ----------------------------------------------------------------
  //     MMIO Offset     MSR Offset     Register Name
  // ----------------------------------------------------------------
  //      300h-310h        830h         Interrupt Command Register [63:0]
  //                       831h         [Reserved]
  // ----------------------------------------------------------------
  //
  WriteApicMsrOrMemory (XapicEnabled, EFI_MSR_EXT_XAPIC_ICR, ApicBase + APIC_REGISTER_ICR_LOW_OFFSET, (((UINT64)ICRHigh << 32) | (UINT64)ICRLow));

  gBS->Stall (10);

  ICRLow = (UINT32)ReadApicMsrOrMemory (XapicEnabled, EFI_MSR_EXT_XAPIC_ICR, ApicBase + APIC_REGISTER_ICR_LOW_OFFSET);

  if (ICRLow & 0x1000) {
    return EFI_NOT_READY;
  }

  gBS->Stall (100);

  return EFI_SUCCESS;
}

UINT32
GetApicID (
  OUT EFI_PHYSICAL_ADDRESS      * ApicBase OPTIONAL,
  OUT UINT32                    *ApicVersion OPTIONAL
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  ApicBase    - GC_TODO: add argument description
  ApicVersion - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  UINT64  ApicBaseReg;
  UINT32  ApicID;
  UINT32  LocalApicVersion;
  UINT64  LocalApicBase;
  UINTN   MsrValue;
  BOOLEAN XapicEnabled;

  XapicEnabled = IsXapicEnabled ();

  if (XapicEnabled) {
    //
    // According to Nehalem BWG, if Extended XAPIC Mode
    // is enabled, legacy xAPIC is no longer working.
    // So, previous MMIO offset must be transfered
    // to MSR offset R/W.
    // MMIO Offset     MSR Offset     Register Name
    //  020h             802h         EFI_EXT_XAPIC_LOGICAL_APIC_ID
    //  030h             803h         EFI_EXT_XAPIC_VERSION
    //
    MsrValue = (UINTN) AsmReadMsr64 (EFI_MSR_EXT_XAPIC_VERSION);
    *ApicVersion  = (UINT32) (MsrValue & B_EFI_MSR_EXT_XAPIC_VERSION_VERSION);
    *ApicBase = 0;

    MsrValue = (UINTN) AsmReadMsr64 (EFI_MSR_EXT_XAPIC_LOGICAL_APIC_ID);
    ApicID  = (UINT32) MsrValue;
    return (ApicID);
  }

  ApicBaseReg   = AsmReadMsr64 (EFI_MSR_IA32_APIC_BASE);
  LocalApicBase = ApicBaseReg & B_EFI_MSR_IA32_APIC_BASE_APIC_BASE_ADDRESS;
  if (ApicBase) {
    *ApicBase = LocalApicBase;
  }
  //
  // if Apic is not enabled yet, enable it here
  //
  if ((ApicBaseReg & B_EFI_MSR_IA32_APIC_BASE_APIC_GLOBAL_ENABLE) == 0) {
    ApicBaseReg |= B_EFI_MSR_IA32_APIC_BASE_APIC_GLOBAL_ENABLE;
    AsmWriteMsr64 (EFI_MSR_IA32_APIC_BASE, ApicBaseReg);
  }

  if (ApicVersion) {
    LocalApicVersion  = *(volatile UINT32 *) (UINTN) (LocalApicBase + APIC_REGISTER_APIC_VERSION_OFFSET);
    *ApicVersion      = LocalApicVersion & B_APIC_REGISTER_APIC_VERSION_OFFSET_VERSION_MASK;
  }

  ApicID = *(volatile UINT32 *) (UINTN) (LocalApicBase + APIC_REGISTER_LOCAL_ID_OFFSET);
  return ((ApicID & B_APIC_REGISTER_LOCAL_ID_OFFSET_XAPIC_ID_MASK) >> N_APIC_REGISTER_LOCAL_ID_OFFSET_XAPIC_ID_MASK);
}

VOID
ProgramVirtualWireMode (
  BOOLEAN                       BSP
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  BSP - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  UINT64                ApicBaseReg;
  EFI_PHYSICAL_ADDRESS  ApicBase;
  UINT64                EntryValue;
  BOOLEAN               XapicEnabled;
  UINT32                VirtualWire;

  VirtualWire = mPlatformCpu->VirtualWireMode;

  ApicBaseReg = AsmReadMsr64 (EFI_MSR_IA32_APIC_BASE);
  ApicBase    = ApicBaseReg & B_EFI_MSR_IA32_APIC_BASE_APIC_BASE_ADDRESS;

  XapicEnabled = IsXapicEnabled ();

  //
  // Program the Spurious Vector entry if XAPIC is enabled
  //
  EntryValue = ReadApicMsrOrMemory (XapicEnabled, EFI_MSR_EXT_XAPIC_SVR, ApicBase + APIC_REGISTER_SPURIOUS_VECTOR_OFFSET);
  EntryValue &= 0xFFFFFD0F;
  EntryValue |= 0x10F;
  WriteApicMsrOrMemory (XapicEnabled, EFI_MSR_EXT_XAPIC_SVR, ApicBase + APIC_REGISTER_SPURIOUS_VECTOR_OFFSET, EntryValue);

  //
  // Double check if it is BSP
  //
  if (!BSP) {
    CpuDisableInterrupt ();
  }

  //
  // Program the LINT0 vector entry as EntInt
  //
  EntryValue = ReadApicMsrOrMemory (XapicEnabled, EFI_MSR_EXT_XAPIC_LVT_LINT0, ApicBase + APIC_REGISTER_LINT0_VECTOR_OFFSET);
  if ((VirtualWire == VIRT_WIRE_A) && BSP) {
    EntryValue &= 0xFFFE00FF;
    EntryValue |= 0x700;
  } else {
    EntryValue |= 0x10000;      // set bit 16 as mask for LINT0
  }
  WriteApicMsrOrMemory (XapicEnabled, EFI_MSR_EXT_XAPIC_LVT_LINT0, ApicBase + APIC_REGISTER_LINT0_VECTOR_OFFSET, EntryValue);

// TODO: disable NMI temporary
//  //
//  // Program the LINT1 vector entry as NMI
//  //
//  EntryValue = ReadApicMsrOrMemory (XapicEnabled, EFI_MSR_EXT_XAPIC_LVT_LINT1, ApicBase + APIC_REGISTER_LINT1_VECTOR_OFFSET);
//  EntryValue &= 0xFFFE00FF;
//  if (BSP) {
//    EntryValue |= 0x400;
//  } else {
//    EntryValue |= 0x10400;
//  }
//  WriteApicMsrOrMemory (XapicEnabled, EFI_MSR_EXT_XAPIC_LVT_LINT1, ApicBase + APIC_REGISTER_LINT1_VECTOR_OFFSET, EntryValue);

  if ((VirtualWire != VIRT_WIRE_A) && BSP) {
    //
    // Initialize the I0XApic RT table
    //
    *(volatile UINT8   *)(UINTN)IO_APIC_INDEX_REGISTER = 0x10;
    *(volatile UINT32  *)(UINTN)IO_APIC_DATA_REGISTER  = 0x0700;
    *(volatile UINT8   *)(UINTN)IO_APIC_INDEX_REGISTER = 0x11;
    *(volatile UINT32  *)(UINTN)IO_APIC_DATA_REGISTER  = (GetApicID (NULL, NULL) << 24);
  }
}

EFI_STATUS
AllocateWakeUpBuffer (
  OUT EFI_PHYSICAL_ADDRESS          *WakeUpBuffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  WakeUpBuffer  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  EFI_STATUS  Status;

  Status = EFI_SUCCESS;
//AMI_OVERRIDE - EIP166924 Use Isharedisk tools can't into OS with PXE boot, EIP164713 Legacy/UEFI OpROM with CSM. >>
#if CSM_SUPPORT == 1
  for (*WakeUpBuffer = (0xA0000 - PMM_EBDA_LOMEM_SIZE - 0x1000); *WakeUpBuffer >= 0x2000; *WakeUpBuffer -= 0x1000) {     
#else
  for (*WakeUpBuffer = 0x8F000; *WakeUpBuffer >= 0x2000; *WakeUpBuffer -= 0x1000) {
#endif
//AMI_OVERRIDE - EIP166924 Use Isharedisk tools can't into OS with PXE boot, EIP164713 Legacy/UEFI OpROM with CSM. <<
    //
    // Do memory range test if exists
    //
    Status = CompatibleMemoryRangeTestIfExist (
               mGenMemoryTest,
               *WakeUpBuffer,
               0x1000
               );
    if (EFI_ERROR (Status)) {
      continue;
    }

#ifdef ECP_FLAG
    Status = (gBS->AllocatePages) (
#else
    Status = gBS->AllocatePages (
#endif
                    AllocateAddress,
                    EfiACPIMemoryNVS,
                    1,
                    WakeUpBuffer
                    );

    if (!EFI_ERROR (Status)) {
      break;
    }
  }

  return Status;
}

EFI_STATUS
AllocateAlignedReservedMemory (
  IN  UINTN         Size,
  IN  UINTN         Alignment,
  OUT VOID          **Pointer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Size      - GC_TODO: add argument description
  Alignment - GC_TODO: add argument description
  Pointer   - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS  Status;
  UINTN       PointerValue;

  Status = AllocateReservedMemoryBelow4G (
             Size + Alignment - 1,
             Pointer
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  PointerValue  = (UINTN) *Pointer;
  PointerValue  = (PointerValue + Alignment - 1) / Alignment * Alignment;

  *Pointer      = (VOID *) PointerValue;
  return EFI_SUCCESS;
}


EFI_STATUS
FillInCpuLocation (
  IN EFI_CPU_PHYSICAL_LOCATION   *Location
  )
{
  UINT32                ApicId;
  UINT32                LevelType;
  UINT32                LevelBits;
  UINT32                RegEax;
  UINT32                RegEbx;
  UINT32                RegEcx;
  UINT8                 Shift;
  UINT8                 Bits;
  UINT32                Mask;
  BOOLEAN               HyperThreadingEnabled;

  AsmCpuid (EFI_CPUID_VERSION_INFO, NULL, &RegEbx, NULL, NULL);
  ApicId = (RegEbx >> 24);

  AsmCpuid (EFI_CPUID_SIGNATURE, &RegEax, NULL, NULL, NULL);
  if (RegEax >= EFI_CPUID_CORE_TOPOLOGY) {
    LevelBits = 0;
    LevelType = 0;
    do {
      AsmCpuidEx (EFI_CPUID_CORE_TOPOLOGY, LevelType, &RegEax, &RegEbx, &RegEcx, NULL);
      LevelType = ((RegEcx >> 8) & 0xFF);
      switch (LevelType) {
        case 1:  //Thread
          Location->Thread    = ApicId  & ((1 << (RegEax & 0x0F)) - 1);
          Location->Thread  >>= LevelBits;
          LevelBits           = RegEax & 0x0F;
          break;
        case 2:  //Core
          Location->Core      = ApicId  & ((1 << (RegEax & 0x0F)) - 1);
          Location->Core    >>= LevelBits;
          LevelBits           = RegEax & 0x0F;
          break;
        default: //End of Level
          Location->Package   = ApicId >> LevelBits;
          break;
      }
    } while (!(RegEax == 0 && RegEbx == 0));
  } else {

    AsmCpuid (EFI_CPUID_VERSION_INFO, NULL, &RegEbx, NULL, NULL);
    Bits  = 0;
    Shift = (UINT8)((RegEbx >> 16) & 0xFF);

    Mask  =  Shift - 1;
    while (Shift > 1) {
      Shift >>= 1;
      Bits++;
    }

    HyperThreadingEnabled = FALSE;
    AsmCpuidEx (EFI_CPUID_CACHE_PARAMS, 0, &RegEax, NULL, NULL, NULL);
    if  (Mask > (RegEax >> 26)) {
      HyperThreadingEnabled = TRUE;
    }

    Location->Package = (ApicId >> Bits);
    if (HyperThreadingEnabled) {
      Location->Core    = (ApicId & Mask) >> 1;
      Location->Thread  = (ApicId & Mask) & 1;
    } else {
      Location->Core    = (ApicId & Mask);
      Location->Thread  = 0;
    }
  }
  return EFI_SUCCESS;
}


EFI_STATUS
FillinDataforDataHub (
  IN   UINTN                            CpuNumber,
  OUT  CPU_DATA_FOR_DATAHUB             *CpuDataforDatahub
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  CpuNumber         - GC_TODO: add argument description
  CpuDataforDatahub - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_CPUID_REGISTER *CpuIdReg;

  ZeroMem (CpuDataforDatahub, sizeof (*CpuDataforDatahub));

  GetActualFrequency (mMetronome, &CpuDataforDatahub->IntendCoreFrequency);

  CpuDataforDatahub->IntendFsbFrequency = DetermineiFsbFromMsr();
  GetProcessorVersion (&CpuDataforDatahub->Version);
  CpuDataforDatahub->Manufacturer = GetProcessorManufacturer ();

  CpuIdReg = (EFI_CPUID_REGISTER *) &CpuDataforDatahub->CpuidData;
  AsmCpuid (EFI_CPUID_VERSION_INFO, &CpuIdReg->RegEax, &CpuIdReg->RegEbx, &CpuIdReg->RegEcx, &CpuIdReg->RegEdx);

  CpuDataforDatahub->Family   = GetProcessorFamily ();
  CpuDataforDatahub->Voltage  = GetProcessorVoltage ();
  CpuDataforDatahub->ApicID   = GetApicID (
                                  &CpuDataforDatahub->ApicBase,
                                  &CpuDataforDatahub->ApicVersion
                                  );

  CpuDataforDatahub->MicrocodeRevision = GetCpuUcodeRevision ();
  AsmCpuid (
    EFI_CPUID_CACHE_INFO,
    &CpuDataforDatahub->CacheInformation->RegEax,
    &CpuDataforDatahub->CacheInformation->RegEbx,
    &CpuDataforDatahub->CacheInformation->RegEcx,
    &CpuDataforDatahub->CacheInformation->RegEdx
  );

  //
  // Status field will be updated later, after calling PlatformCpu protocol to override
  //
  CpuDataforDatahub->Status = GetProcessorStatus (CpuNumber);;

  FillInCpuLocation (&CpuDataforDatahub->Location);

  CpuDataforDatahub->CoreCount = GetProcessorCoreCount ();
  CpuDataforDatahub->CoreEnabled = GetProcessorEnabledCoreCount ();
  CpuDataforDatahub->ThreadCount = GetProcessorThreadCount ();
  CpuDataforDatahub->ProcessorCharacteristics = GetProcessorCharacteristics ();

  //
  // Health field will be filled in else where
  //
  return EFI_SUCCESS;
}

EFI_STATUS
AllocateReservedMemoryBelow4G (
  IN   UINTN   Size,
  OUT  VOID    **Buffer
  )
/*++

Routine Description:

  Allocate EfiACPIMemoryNVS below 4G memory address.

Arguments:

  Size   - Size of memory to allocate.
  Buffer - Allocated address for output.

Returns:

  EFI_SUCCESS - Memory successfully allocated.
  Other       - Other errors occur.

--*/
{
  UINTN                 Pages;
  EFI_PHYSICAL_ADDRESS  Address;
  EFI_STATUS            Status;

  Pages   = EFI_SIZE_TO_PAGES (Size);
  Address = 0xffffffff;

#ifdef ECP_FLAG
    Status  = (gBS->AllocatePages) (
#else
    Status  = gBS->AllocatePages (
#endif
                   AllocateMaxAddress,
                   EfiACPIMemoryNVS,
                   Pages,
                   &Address
                   );

  *Buffer = (VOID *) (UINTN) Address;

  return Status;
}

#ifdef __GNUC__
#define _ReadWriteBarrier() do { __asm__ __volatile__ ("": : : "memory"); } while(0)

int _outp(
  unsigned short port,
  int databyte
  )
{
  __asm__ __volatile__ ("outb %b0,%w1" : : "a" (databyte), "d" ((UINT16)port));
  return databyte;
}
#else
#pragma intrinsic(_outp, _ReadWriteBarrier)
#endif

VOID
EFIAPI
CpuIoWrite8 (
  IN  UINT16  Port,
  IN  UINT32  Data
  )
{
  _ReadWriteBarrier();
  _outp (Port, (int)Data);
}

VOID
EFIAPI
InitSmramDataContent (
  IN EFI_EVENT     Event,
  IN VOID          *Context
  )
/*++

Routine Description:

  This function is invoked when SMM_BASE protocol is installed, then we
  allocate SMRAM and save all information there.

Arguments:

  Event   - The triggered event.
  Context - Context for this event.

Returns:

  None

--*/
{
#ifdef ECP_FLAG
  EFI_SMM_ACCESS_PROTOCOL    *SmmAccess;
#else
  EFI_SMM_ACCESS2_PROTOCOL   *SmmAccess;
#endif
  EFI_SMM_BASE_PROTOCOL      *SmmBase;
  EFI_SMRAM_DESCRIPTOR       *SmramRanges;
  UINTN                      Size;
  SMRAM_CPU_DATA             SmramCpuDataTemplate;
  UINTN                      LockBoxSize;
  UINT8                      *LockBoxData;
  IA32_DESCRIPTOR            *Idtr;
  IA32_DESCRIPTOR            *Gdtr;
  UINTN                      MicrocodeSize;
  EFI_CPU_MICROCODE_HEADER   **Microcode;
  UINT8                      *LockBoxMicrocode;
  UINTN                      Index;
  EFI_STATUS                 Status;
  EFI_SMM_CONTROL_PROTOCOL   *SmmControl;
  UINT8                      *SmramCpuData;
  UINTN                      VarSize;
  UINT64                     VarData[3];
  UINTN                      ArgBufferSize;
  UINT8                      ArgBuffer;
  EFI_SMM_CONTROL_REGISTER   SmiRegister;

  DEBUG ((EFI_D_ERROR, "InitSmramDataContent\n"));
#ifdef ECP_FLAG
  Status = gBS->LocateProtocol(&gEfiSmmAccessProtocolGuid, NULL, (VOID **) &SmmAccess);
#else
  Status = gBS->LocateProtocol(&gEfiSmmAccess2ProtocolGuid, NULL, (VOID **) &SmmAccess);
#endif
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol(&gEfiSmmBaseProtocolGuid, NULL, (VOID **) &SmmBase);
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol(&gEfiSmmControlProtocolGuid, NULL, (VOID **) &SmmControl);
  ASSERT_EFI_ERROR (Status);

  //
  // Get SMRAM information
  //
  Size = 0;
  Status = SmmAccess->GetCapabilities (SmmAccess, &Size, NULL);
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);

#ifdef ECP_FLAG
  Status = (gBS->AllocatePool) (
#else
  Status = gBS->AllocatePool (
#endif
                  EfiBootServicesData,
                  Size,
                  (VOID **) &SmramRanges
                  );
  ASSERT_EFI_ERROR (Status);

  Status = SmmAccess->GetCapabilities (
                        SmmAccess,
                        &Size,
                        SmramRanges
                        );
  ASSERT_EFI_ERROR (Status);

  //
  // Open all SMRAM ranges
  //
  Size /= sizeof (*SmramRanges);
#ifdef ECP_FLAG
  for (Index = 0; Index < Size; Index++) {
    Status = SmmAccess->Open (SmmAccess, Index);
    ASSERT_EFI_ERROR (Status);
  }
#else
  Status = SmmAccess->Open (SmmAccess);
  ASSERT_EFI_ERROR (Status);
#endif

  //
  // Init
  //
  CopyMem (&SmramCpuDataTemplate.HeaderGuid, &mSmramCpuNvsHeaderGuid, sizeof(EFI_GUID));
  SmramCpuDataTemplate.AcpiCpuPointer  = (EFI_PHYSICAL_ADDRESS)(UINTN)mAcpiCpuData;
  CopyMem (&SmramCpuDataTemplate.AcpiCpuData, mAcpiCpuData, sizeof(ACPI_CPU_DATA_COMPATIBILITY));

  //
  // Calculate size
  //
  SmramCpuDataTemplate.GdtrProfileSize            = sizeof (IA32_DESCRIPTOR);
  Gdtr = (IA32_DESCRIPTOR *)(UINTN)mAcpiCpuData->GdtrProfile;
  SmramCpuDataTemplate.GdtSize                    = Gdtr->Limit + 1;
  SmramCpuDataTemplate.IdtrProfileSize            = sizeof (IA32_DESCRIPTOR);
  Idtr = (IA32_DESCRIPTOR *)(UINTN)mAcpiCpuData->GdtrProfile;
  SmramCpuDataTemplate.IdtSize                    = Idtr->Limit + 1;
  SmramCpuDataTemplate.CpuPrivateDataSize         = sizeof(MP_CPU_S3_DATA_POINTER);
  SmramCpuDataTemplate.S3BootScriptTableSize      = sizeof(mMPSystemData->S3BootScriptTable);
  SmramCpuDataTemplate.S3BspMtrrTableSize         = sizeof(mMPSystemData->S3BspMtrrTable);
  //
  // Record best match for each CPU Microcode and NULL for end
  //
  SmramCpuDataTemplate.MicrocodePointerBufferSize = sizeof(UINT32) * (mAcpiCpuData->NumberOfCpus + 1);
  //
  // Calculate Microcode DataSize
  //
  SmramCpuDataTemplate.MicrocodeDataBufferSize    = 0;
  Microcode = (VOID *)(UINTN)mAcpiCpuData->MicrocodePointerBuffer;
  if (Microcode != NULL) {
    Index = 0;
    MicrocodeSize = 0;
    while (Microcode[Index] != NULL) {
      if (Microcode[Index]->DataSize == 0) {
        MicrocodeSize = 2048;
      } else {
        MicrocodeSize = Microcode[Index]->TotalSize;
      }
      SmramCpuDataTemplate.MicrocodeDataBufferSize += (UINT32)MicrocodeSize;
      Index ++;
    }
  }

  SmramCpuDataTemplate.GdtrProfileOffset            = sizeof(SMRAM_CPU_DATA);
  SmramCpuDataTemplate.GdtOffset                    = SmramCpuDataTemplate.GdtrProfileOffset +
                                                      SmramCpuDataTemplate.GdtrProfileSize;
  SmramCpuDataTemplate.IdtrProfileOffset            = SmramCpuDataTemplate.GdtOffset +
                                                      SmramCpuDataTemplate.GdtSize;
  SmramCpuDataTemplate.IdtOffset                    = SmramCpuDataTemplate.IdtrProfileOffset +
                                                      SmramCpuDataTemplate.IdtrProfileSize;
  SmramCpuDataTemplate.CpuPrivateDataOffset         = SmramCpuDataTemplate.IdtOffset +
                                                      SmramCpuDataTemplate.IdtSize;
  SmramCpuDataTemplate.S3BootScriptTableOffset      = SmramCpuDataTemplate.CpuPrivateDataOffset +
                                                      SmramCpuDataTemplate.CpuPrivateDataSize;
  SmramCpuDataTemplate.S3BspMtrrTableOffset         = SmramCpuDataTemplate.S3BootScriptTableOffset +
                                                      SmramCpuDataTemplate.S3BootScriptTableSize;
  SmramCpuDataTemplate.MicrocodePointerBufferOffset = SmramCpuDataTemplate.S3BspMtrrTableOffset +
                                                      SmramCpuDataTemplate.S3BspMtrrTableSize;
  SmramCpuDataTemplate.MicrocodeDataBufferOffset    = SmramCpuDataTemplate.MicrocodePointerBufferOffset +
                                                      SmramCpuDataTemplate.MicrocodePointerBufferSize;

  LockBoxSize = sizeof(SMRAM_CPU_DATA) +
                SmramCpuDataTemplate.GdtrProfileSize +
                SmramCpuDataTemplate.GdtSize +
                SmramCpuDataTemplate.IdtrProfileSize +
                SmramCpuDataTemplate.IdtSize +
                SmramCpuDataTemplate.CpuPrivateDataSize +
                SmramCpuDataTemplate.S3BootScriptTableSize +
                SmramCpuDataTemplate.S3BspMtrrTableSize +
                SmramCpuDataTemplate.MicrocodePointerBufferSize +
                SmramCpuDataTemplate.MicrocodeDataBufferSize;

  DEBUG ((EFI_D_ERROR, "LockBoxSize - %x\n", LockBoxSize));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.GdtrProfileSize            - %x\n", SmramCpuDataTemplate.GdtrProfileSize));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.GdtSize                    - %x\n", SmramCpuDataTemplate.GdtSize));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.IdtrProfileSize            - %x\n", SmramCpuDataTemplate.IdtrProfileSize));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.IdtSize                    - %x\n", SmramCpuDataTemplate.IdtSize));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.CpuPrivateDataSize         - %x\n", SmramCpuDataTemplate.CpuPrivateDataSize));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.S3BootScriptTableSize      - %x\n", SmramCpuDataTemplate.S3BootScriptTableSize));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.S3BspMtrrTableSize         - %x\n", SmramCpuDataTemplate.S3BspMtrrTableSize));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.MicrocodePointerBufferSize - %x\n", SmramCpuDataTemplate.MicrocodePointerBufferSize));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.MicrocodeDataBufferSize    - %x\n", SmramCpuDataTemplate.MicrocodeDataBufferSize));

  DEBUG ((EFI_D_ERROR, "SmramCpuData.GdtrProfileOffset            - %x\n", SmramCpuDataTemplate.GdtrProfileOffset));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.GdtOffset                    - %x\n", SmramCpuDataTemplate.GdtOffset));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.IdtrProfileOffset            - %x\n", SmramCpuDataTemplate.IdtrProfileOffset));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.IdtOffset                    - %x\n", SmramCpuDataTemplate.IdtOffset));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.CpuPrivateDataOffset         - %x\n", SmramCpuDataTemplate.CpuPrivateDataOffset));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.S3BootScriptTableOffset      - %x\n", SmramCpuDataTemplate.S3BootScriptTableOffset));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.S3BspMtrrTableOffset         - %x\n", SmramCpuDataTemplate.S3BspMtrrTableOffset));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.MicrocodePointerBufferOffset - %x\n", SmramCpuDataTemplate.MicrocodePointerBufferOffset));
  DEBUG ((EFI_D_ERROR, "SmramCpuData.MicrocodeDataBufferOffset    - %x\n", SmramCpuDataTemplate.MicrocodeDataBufferOffset));

  //
  // Allocate Normal Memory
  //
#ifdef ECP_FLAG
  Status = (gBS->AllocatePool) (
#else
  Status = gBS->AllocatePool (
#endif
                  EfiBootServicesData,
                  LockBoxSize,
                  (VOID **) &SmramCpuData
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Allocate SMRAM
  //
  Status = SmmBase->SmmAllocatePool (
                      SmmBase,
                      EfiRuntimeServicesData,
                      LockBoxSize + EFI_PAGE_SIZE,
                      (VOID **) &LockBoxData
                      );
  ASSERT_EFI_ERROR (Status);

  //
  // Let it page aligned
  //
  LockBoxData = (UINT8 *)(((UINTN)LockBoxData + EFI_PAGE_SIZE - 1) & ~(EFI_PAGE_SIZE - 1));
  DEBUG ((EFI_D_ERROR, "CPU SMRAM NVS Data - %x\n", LockBoxData));

  //
  // Copy data buffer
  //
  CopyMem (SmramCpuData, &SmramCpuDataTemplate, sizeof(SmramCpuDataTemplate));

  CopyMem (
    SmramCpuData + SmramCpuDataTemplate.GdtrProfileOffset,
    (VOID *)(UINTN)mAcpiCpuData->GdtrProfile,
    SmramCpuDataTemplate.GdtrProfileSize
    );
  CopyMem (
    SmramCpuData + SmramCpuDataTemplate.GdtOffset,
    (VOID *)(UINTN)Gdtr->Base,
    SmramCpuDataTemplate.GdtSize
    );
  CopyMem (
    SmramCpuData + SmramCpuDataTemplate.IdtrProfileOffset,
    (VOID *)(UINTN)mAcpiCpuData->IdtrProfile,
    SmramCpuDataTemplate.IdtrProfileSize
    );
  CopyMem (
    SmramCpuData + SmramCpuDataTemplate.IdtOffset,
    (VOID *)(UINTN)Idtr->Base,
    SmramCpuDataTemplate.IdtSize
    );
  CopyMem (
    SmramCpuData + SmramCpuDataTemplate.CpuPrivateDataOffset,
    (VOID *)(UINTN)mAcpiCpuData->CpuPrivateData,
    SmramCpuDataTemplate.CpuPrivateDataSize
    );
  CopyMem (
    SmramCpuData + SmramCpuDataTemplate.S3BootScriptTableOffset,
    (VOID *)(UINTN)mMPSystemData->S3DataPointer.S3BootScriptTable,
    SmramCpuDataTemplate.S3BootScriptTableSize
    );
  CopyMem (
    SmramCpuData + SmramCpuDataTemplate.S3BspMtrrTableOffset,
    (VOID *)(UINTN)mMPSystemData->S3DataPointer.S3BspMtrrTable,
    SmramCpuDataTemplate.S3BspMtrrTableSize
    );
  CopyMem (
    SmramCpuData + SmramCpuDataTemplate.MicrocodePointerBufferOffset,
    (VOID *)(UINTN)mAcpiCpuData->MicrocodePointerBuffer,
    SmramCpuDataTemplate.MicrocodePointerBufferSize
    );
  //
  // Copy Microcode
  //
  LockBoxMicrocode = SmramCpuData + SmramCpuDataTemplate.MicrocodeDataBufferOffset;
  Microcode = (VOID *)(UINTN)mAcpiCpuData->MicrocodePointerBuffer;
  if (Microcode != NULL) {
    Index = 0;
    MicrocodeSize = 0;
    while (Microcode[Index] != NULL) {
      if (Microcode[Index]->DataSize == 0) {
        MicrocodeSize = 2048;
      } else {
        MicrocodeSize = Microcode[Index]->TotalSize;
      }
      CopyMem (LockBoxMicrocode, Microcode[Index], MicrocodeSize);
      LockBoxMicrocode += MicrocodeSize;
      Index ++;
    }
  }

  //
  // Copy to SMRAM
  //
  //
  // We have to use SMI to copy SMRAM, because we can not access SMRAM after SMRR enabled.
  // SMM_ACCESS.Open () takes no effect.
  //
  VarSize = sizeof(VarData);
  VarData[0] = (UINT64)(UINTN)LockBoxData;
  VarData[1] = (UINT64)(UINTN)SmramCpuData;
  VarData[2] = (UINT64)LockBoxSize;
  Status = gRT->SetVariable (
                  L"SmramCpuNvs",
                  &mSmramCpuNvsHeaderGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS,
                  VarSize,
                  VarData
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Fill SMI data port
  //
  Status = SmmControl->GetRegisterInfo (SmmControl, &SmiRegister);
  ASSERT_EFI_ERROR (Status);
  CpuIoWrite8 (SmiRegister.SmiDataRegister, SMM_FROM_CPU_DRIVER_SAVE_INFO);

  //
  // Trigger SMI
  //
  ArgBufferSize = sizeof (ArgBuffer);
  ArgBuffer = SMM_FROM_SMBASE_DRIVER;
  Status = SmmControl->Trigger (SmmControl, (INT8 *) &ArgBuffer, &ArgBufferSize, FALSE, 0);
  Status = SmmControl->Clear (SmmControl, 0);

  //
  // Close all SMRAM ranges
  //
#ifdef ECP_FLAG
  for (Index = 0; Index < Size; Index++) {
    Status = SmmAccess->Close (SmmAccess, Index);
    ASSERT_EFI_ERROR (Status);
  }
#else
  Status = SmmAccess->Close (SmmAccess);
  ASSERT_EFI_ERROR (Status);
#endif

  //
  // Free resource
  //
#ifdef ECP_FLAG
  (gBS->FreePool) (SmramRanges);
#else
  gBS->FreePool (SmramRanges);
#endif

  //
  // Done
  //
  return ;
}

VOID
EFIAPI
ReAllocateMemoryForAP (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
/*++

Routine Description:

  This function is invoked when LegacyBios protocol is installed, we must
  allocate reserved memory under 1M for AP.

Arguments:

  Event   - The triggered event.
  Context - Context for this event.

Returns:

  None

--*/
{
  EFI_LEGACY_BIOS_PROTOCOL   *LegacyBios;
  EFI_PHYSICAL_ADDRESS        LegacyRegion;
  EFI_STATUS                  Status;
  MP_CPU_EXCHANGE_INFO       *ExchangeInfo;
  BOOLEAN                     HasCsm;
//AMI_OVERRIDE - EIP166924 Use Isharedisk tools can't into OS with PXE boot, EIP164713 Legacy/UEFI OpROM with CSM. >>
  EFI_PHYSICAL_ADDRESS        EbdaOld; 
  EFI_PHYSICAL_ADDRESS        EbdaNew; 
  UINTN                       EbdaSize;
//AMI_OVERRIDE - EIP166924 Use Isharedisk tools can't into OS with PXE boot, EIP164713 Legacy/UEFI OpROM with CSM. <<
  STATIC BOOLEAN              InitDone = FALSE;
  if (InitDone) {
    return ;
  }
  InitDone = TRUE;
  //
  // WA for B0/B1
  //
  PCIConfigWA(Event, Context );

  Status = gBS->LocateProtocol (&gEfiLegacyBiosProtocolGuid, NULL, (VOID **) &LegacyBios);
  if (EFI_ERROR (Status)) {
    HasCsm   = FALSE;
  } else {
    HasCsm = TRUE;
  }

  while (ApRunning()) {
    CpuPause ();
  }

  if (HasCsm) {
    //
    // PLEASE NOTE:
    // For legacy implementation, we have reserved 0x9F000 to 0x9FFFF for S3 usage in CSM,
    // Please don't need to allocate it again
    // This range will be used for MpS3 driver and S3Resume driver on S3 boot path
    // The base needs to be aligned to 4K to satisfy the AP vector requirement
    // The original implementation requires 8K from legacy memory form E/F segment,
    // which needs lock/unlock and makes lots of code chipset dependent on S3 boot path
    // Here we just use normal low memory to eliminate the dependency
    // In this case, EBDA will start from 0x9F000 - sizeof (EBDA) in CSM definition
    // CSM EBDA base and memory size in BDA area needs to be consistent with this
    //

    //
    // Get EDBA address/length and turn it into the S3 reserved address
    // The length of this range is limited so we need to keep the real mode code small
    //
//AMI_OVERRIDE - EIP166924 Use Isharedisk tools can't into OS with PXE boot, EIP164713 Legacy/UEFI OpROM with CSM. >>	
    EbdaOld = (EFI_PHYSICAL_ADDRESS)(*(UINT16 *)(UINTN)0x40E) << 4;
    EbdaSize = (UINTN)(*((UINT8 *)(UINTN)EbdaOld));
    LegacyRegion = EbdaOld + (EbdaSize << 10);
    LegacyRegion = (LegacyRegion - 0x1000) & 0xFFFFF000;
    EbdaNew = LegacyRegion - (EbdaSize << 10);
    (*(UINT16 *)(UINTN)0x40E) = (UINT16)(EbdaNew >> 4);
    CopyMem ((VOID*)(UINTN)EbdaNew, (VOID*)(UINTN)EbdaOld, EbdaSize << 10);
    
    //
    // Update 40:13 with the new size of available base memory
    //
    *(UINT16*)(UINTN)0x413 = (*(UINT16*)(UINTN)0x413) - (UINT16)(((EbdaOld - EbdaNew) >> 10));
//AMI_OVERRIDE - EIP166924 Use Isharedisk tools can't into OS with PXE boot, EIP164713 Legacy/UEFI OpROM with CSM. <<	
  } else {
    //
    // The BackBuffer is 4k.
    // Allocate 0x2000 bytes from below 640K memory to make sure I can
    // get a 4k aligned spaces of 0x1000 bytes, since Alignment argument does not work.
    //
    LegacyRegion = 0x9FFFF;
    Status = (gBS->AllocatePages) (
                     AllocateMaxAddress,
                     EfiReservedMemoryType,
                     EFI_SIZE_TO_PAGES(0x2000),
                     &LegacyRegion
                     );
    ASSERT_EFI_ERROR (Status);
    DEBUG ((EFI_D_ERROR, "LegacyRegion NonCSM - %x\n", LegacyRegion));
    if (EFI_ERROR (Status)) {
      return ;
    }
  }

  DEBUG((EFI_D_INFO, "LegacyRegion: 0x%08x\r\n", (UINT32)(UINTN)LegacyRegion));
  //
  // This address should be less than A seg.
  // And it should be aligned to 4K
  //
  ASSERT (!((UINTN)LegacyRegion & 0x0FFF) && ((UINTN)LegacyRegion < 0xA0000));

  mAcpiCpuData->WakeUpBuffer = (EFI_PHYSICAL_ADDRESS) LegacyRegion;
  mAcpiCpuData->WakeUpBuffer = (mAcpiCpuData->WakeUpBuffer + 0x0fff) & 0x0fffff000;

  ExchangeInfo               = (MP_CPU_EXCHANGE_INFO *) (UINTN) (mBackupBuffer + MP_CPU_EXCHANGE_INFO_OFFSET);
  ExchangeInfo->BufferStart  = (UINT32) mAcpiCpuData->WakeUpBuffer;
  ExchangeInfo->ApFunction   = (VOID *) (UINTN) LegacyRegionAPCount;

#ifdef ECP_FLAG
  (gBS->CopyMem) (
#else
  gBS->CopyMem (
#endif
         (VOID *) (UINTN) mAcpiCpuData->WakeUpBuffer,
         (VOID *) (UINTN) mBackupBuffer,
         EFI_PAGE_SIZE
         );
  RedirectFarJump();
  if (HasCsm) {
    Status = LegacyBios->CopyLegacyRegion (
                          LegacyBios,
                          sizeof (MP_CPU_EXCHANGE_INFO),
                          (VOID *) (UINTN) (mAcpiCpuData->WakeUpBuffer + MP_CPU_EXCHANGE_INFO_OFFSET),
                          (VOID *) (UINTN) (mBackupBuffer + MP_CPU_EXCHANGE_INFO_OFFSET)
                          );
  }

  //
  // Do final intialization for APs
  // Is it neccessary?
  //
  SendInterrupt (BROADCAST_MODE_ALL_EXCLUDING_SELF, 0, 0, DELIVERY_MODE_INIT, TRIGGER_MODE_EDGE, TRUE);
  SendInterrupt (
    BROADCAST_MODE_ALL_EXCLUDING_SELF,
    0,
    (UINT32) RShiftU64 (mAcpiCpuData->WakeUpBuffer, 12),
    DELIVERY_MODE_SIPI,
    TRIGGER_MODE_EDGE,
    TRUE
    );
  //
  // Wait until all APs finish
  //
  while (mSwitchToLegacyRegionCount < mAcpiCpuData->NumberOfCpus - 1) {
    CpuPause ();
  }

  ExchangeInfo->ApFunction   = (VOID *) (UINTN) ApProcWrapper;
#ifdef ECP_FLAG
  (gBS->CopyMem) (
#else
  gBS->CopyMem (
#endif
         (VOID *) (UINTN) mAcpiCpuData->WakeUpBuffer,
         (VOID *) (UINTN) mBackupBuffer,
         EFI_PAGE_SIZE
         );
  RedirectFarJump();
  if (HasCsm) {
    Status = LegacyBios->CopyLegacyRegion (
                          LegacyBios,
                          sizeof (MP_CPU_EXCHANGE_INFO),
                          (VOID *) (UINTN) (mAcpiCpuData->WakeUpBuffer + MP_CPU_EXCHANGE_INFO_OFFSET),
                          (VOID *) (UINTN) (mBackupBuffer + MP_CPU_EXCHANGE_INFO_OFFSET)
                          );
  }

  //
  // Invoke the InitSmram directly, since it is in ExitPmAuth event.
  //
  //TODO: need fix SMM issue
  Status = gBS->LocateProtocol (&gExitPmAuthProtocolGuid , NULL, (VOID **) &LegacyBios);
  if(!EFI_ERROR(Status)) InitSmramDataContent (NULL, NULL);
}

VOID
ResetAPs (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
/*++

Routine Description:

  This function is invoked by EFI_EVENT_SIGNAL_LEGACY_BOOT.
  Before booting to legacy OS, reset AP's wakeup buffer address,
  preparing for S3 usage.

Arguments:

  Event   - The triggered event.
  Context - Context for this event.

Returns:

  None

--*/
{
}
