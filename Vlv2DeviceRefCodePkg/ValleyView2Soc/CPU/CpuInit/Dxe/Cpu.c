/**@file

  Cpu driver, which initializes CPU and implements CPU Architecture
  Protocol as defined in Framework specification.

@copyright
 Copyright (c) 1999 - 2014 Intel Corporation. All rights reserved
 This software and associated documentation (if any) is furnished
 under a license and may only be used or copied in accordance
 with the terms of the license. Except as permitted by the
 license, no part of this software or documentation may be
 reproduced, stored in a retrieval system, or transmitted in any
 form or by any means without the express written consent of
 Intel Corporation.

This file contains an 'Intel Peripheral Driver' and is uniquely
 identified as "Intel Reference Module" and is licensed for Intel
 CPUs and chipsets under the terms of your license agreement with
 Intel or your vendor. This file may be modified by the user, subject
 to additional terms of the license agreement.
**/
/*++

Copyright (c) 1999 - 2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  Cpu.c

Abstract:

  Cpu driver, which initializes CPU and implements CPU Architecture
  Protocol as defined in Framework specification.

--*/

#include "CpuDxe.h"
#include "Processor.h"
#include "Cache.h"
#include "Exception.h"
#include "MiscFuncs.h"
#include "Thermal.h"
#include "MemoryAttribute.h"
#include <PchAccess.h>
#include <Library/PchPlatformLib.h>
#ifdef ECP_FLAG
#include "EdkIIGlueIoLib.h"
#else
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#endif

#ifdef ECP_FLAG
EFI_GUID gEfiMpServiceProtocolGuid        = { 0x3fdda605, 0xa76e, 0x4f46, {0xad, 0x29, 0x12, 0xf4, 0x53, 0x1b, 0x3d, 0x08}};
EFI_GUID gEfiPlatformCpuProtocolGuid      = EFI_PLATFORM_CPU_PROTOCOL_GUID;
EFI_GUID gEfiPlatformCpuInfoGuid          = EFI_PLATFORM_CPU_INFO_GUID;
EFI_GUID gEfiAcpiVariableCompatiblityGuid = EFI_ACPI_VARIABLE_COMPATIBILITY_GUID;
EFI_GUID gEfiHtBistHobGuid                = EFI_HT_BIST_HOB_GUID;
EFI_GUID gEfiPowerOnHobGuid               = EFI_POWER_ON_HOB_GUID;
EFI_HII_DATABASE_PROTOCOL   *gHiiDatabase = NULL;
EFI_GUID gEfiVlv2VariableGuid             = EFI_VLV2_VARIABLE;
#endif
/*
#include EFI_GUID_DEFINITION (Hob)
#include EFI_GUID_DEFINITION (SetupMode)
*/
#define   SAMPLE_TICK_COUNT        100

extern UINT64                      mValidMtrrAddressMask;
extern UINT64                      mValidMtrrBitsMask;
extern EFI_CPU_MICROCODE_HEADER  **mMicrocodePointerBuffer;

#ifndef ECP_FLAG
EFI_HII_PROTOCOL                  *mHii;
#endif
EFI_SMM_BASE_PROTOCOL             *mSmmBaseProtocol = NULL;
VOID                              *mSmmBaseRegistration;
EFI_METRONOME_ARCH_PROTOCOL       *mMetronome;
EFI_PLATFORM_CPU_PROTOCOL         *mPlatformCpu = NULL;
EFI_DATA_HUB_PROTOCOL             *mDataHub     = NULL;
EFI_HII_HANDLE                    mStringHandle;
BOOLEAN                           mIsFlushingGCD = TRUE;
EFI_PLATFORM_CPU_INFO             mPlatformCpuInfo;
UINT64                            mCpuFrequency = 0;

UINT16 miFSBFrequencyTable[4] = {
  83,           // 83.3MHz
  100,          // 100MHz
  133,          // 133MHz
  117           // 116.7MHz
};

//
// Function declarations
//
EFI_STATUS
EFIAPI
InitializeCpu (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
PrepareMemory (
  VOID
  );

EFI_STATUS
EFIAPI
FlushCpuDataCache (
  IN EFI_CPU_ARCH_PROTOCOL     *This,
  IN EFI_PHYSICAL_ADDRESS      Start,
  IN UINT64                    Length,
  IN EFI_CPU_FLUSH_TYPE        FlushType
  );

EFI_STATUS
EFIAPI
EnableInterrupt (
  IN EFI_CPU_ARCH_PROTOCOL     *This
  );

EFI_STATUS
EFIAPI
DisableInterrupt (
  IN EFI_CPU_ARCH_PROTOCOL     *This
  );

EFI_STATUS
EFIAPI
CpuGetInterruptState (
  IN  EFI_CPU_ARCH_PROTOCOL     *This,
  OUT BOOLEAN                   *State
  );

EFI_STATUS
EFIAPI
Init (
  IN EFI_CPU_ARCH_PROTOCOL     *This,
  IN EFI_CPU_INIT_TYPE         InitType
  );

EFI_STATUS
EFIAPI
RegisterInterruptHandler (
  IN EFI_CPU_ARCH_PROTOCOL         *This,
  IN EFI_EXCEPTION_TYPE            InterruptType,
  IN EFI_CPU_INTERRUPT_HANDLER     InterruptHandler
  );

EFI_STATUS
EFIAPI
GetTimerValue (
  IN  EFI_CPU_ARCH_PROTOCOL       *This,
  IN  UINT32                      TimerIndex,
  OUT UINT64                      *TimerValue,
  OUT UINT64                      *TimerPeriod OPTIONAL
  );

EFI_STATUS
EFIAPI
SetMemoryAttributes (
  IN EFI_CPU_ARCH_PROTOCOL      *This,
  IN EFI_PHYSICAL_ADDRESS       BaseAddress,
  IN UINT64                     Length,
  IN UINT64                     Attributes
  );

//AMI_OVERRIDE - EIP137713 Hang at CP 0x68 when limitcpuid is enabled. >>
VOID
ProgrameCpuidLimit (
  IN  MP_SYSTEM_DATA                       *MPSystemData
  );
//AMI_OVERRIDE - EIP137713 Hang at CP 0x68 when limitcpuid is enabled. <<
//
// Global Variables
//
EFI_CPU_INTERRUPT_HANDLER   mExternalVectorTable[0x100];

BOOLEAN                     mInterruptState = FALSE;

//
// The Cpu Architectural Protocol that this Driver produces
//
EFI_CPU_ARCH_PROTOCOL       gCpu = {
  FlushCpuDataCache,
  EnableInterrupt,
  DisableInterrupt,
  CpuGetInterruptState,
  Init,
  RegisterInterruptHandler,
  GetTimerValue,
  SetMemoryAttributes,
  1,  // NumberOfTimers
  4,  // DmaBufferAlignment
};

#ifdef ECP_FLAG
/**
  Registers a list of packages in the HII Database and returns the HII Handle
  associated with that registration.  If an HII Handle has already been registered
  with the same PackageListGuid and DeviceHandle, then NULL is returned.  If there
  are not enough resources to perform the registration, then NULL is returned.
  If an empty list of packages is passed in, then NULL is returned.  If the size of
  the list of package is 0, then NULL is returned.

  The variable arguments are pointers which point to package header that defined
  by UEFI VFR compiler and StringGather tool.

  #pragma pack (push, 1)
  typedef struct {
    UINT32                  BinaryLength;
    EFI_HII_PACKAGE_HEADER  PackageHeader;
  } EDKII_AUTOGEN_PACKAGES_HEADER;
  #pragma pack (pop)

  @param[in]  PackageListGuid  The GUID of the package list.
  @param[in]  DeviceHandle     If not NULL, the Device Handle on which
                               an instance of DEVICE_PATH_PROTOCOL is installed.
                               This Device Handle uniquely defines the device that
                               the added packages are associated with.
  @param[in]  ...              The variable argument list that contains pointers
                               to packages terminated by a NULL.

  @retval NULL   A HII Handle has already been registered in the HII Database with
                 the same PackageListGuid and DeviceHandle.
  @retval NULL   The HII Handle could not be created.
  @retval NULL   An empty list of packages was passed in.
  @retval NULL   All packages are empty.
  @retval Other  The HII Handle associated with the newly registered package list.

**/
//
// Template used to mark the end of a list of packages
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST EFI_HII_PACKAGE_HEADER  mEndOfPakageList = {
  sizeof (EFI_HII_PACKAGE_HEADER),
  EFI_HII_PACKAGE_END
};

EFI_HII_HANDLE
EFIAPI
HiiAddPackages (
  IN CONST EFI_GUID    *PackageListGuid,
  IN       EFI_HANDLE  DeviceHandle  OPTIONAL,
  ...
  )
{
  EFI_STATUS                   Status;
  VA_LIST                      Args;
  UINT32                       *Package;
  EFI_HII_PACKAGE_LIST_HEADER  *PackageListHeader;
  EFI_HII_HANDLE               HiiHandle;
  UINT32                       Length;
  UINT8                        *Data;

  ASSERT (PackageListGuid != NULL);

  //
  // Calculate the length of all the packages in the variable argument list
  //
  for (Length = 0, VA_START (Args, DeviceHandle); (Package = VA_ARG (Args, UINT32 *)) != NULL; ) {
    Length += (ReadUnaligned32 (Package) - sizeof (UINT32));
  }
  VA_END (Args);

  //
  // If there are no packages in the variable argument list or all the packages
  // are empty, then return a NULL HII Handle
  //
  if (Length == 0) {
    return NULL;
  }

  //
  // Add the length of the Package List Header and the terminating Package Header
  //
  Length += sizeof (EFI_HII_PACKAGE_LIST_HEADER) + sizeof (EFI_HII_PACKAGE_HEADER);

  //
  // Allocate the storage for the entire Package List
  //
  PackageListHeader = AllocateZeroPool (Length);

  //
  // If the Package List can not be allocated, then return a NULL HII Handle
  //
  if (PackageListHeader == NULL) {
    return NULL;
  }

  //
  // Fill in the GUID and Length of the Package List Header
  //
  CopyGuid (&PackageListHeader->PackageListGuid, PackageListGuid);
  PackageListHeader->PackageLength = Length;

  //
  // Initialize a pointer to the beginning if the Package List data
  //
  Data = (UINT8 *)(PackageListHeader + 1);

  //
  // Copy the data from each package in the variable argument list
  //
  for (VA_START (Args, DeviceHandle); (Package = VA_ARG (Args, UINT32 *)) != NULL; ) {
    Length = ReadUnaligned32 (Package) - sizeof (UINT32);
    CopyMem (Data, Package + 1, Length);
    Data += Length;
  }
  VA_END (Args);

  //
  // Append a package of type EFI_HII_PACKAGE_END to mark the end of the package list
  //
  CopyMem (Data, &mEndOfPakageList, sizeof (mEndOfPakageList));

  //
  // Register the package list with the HII Database
  //
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, " gHiiDatabase->NewPackageList Start\n"));

  Status = (gHiiDatabase->NewPackageList) (
                           gHiiDatabase,
                           PackageListHeader,
                           DeviceHandle,
                           &HiiHandle
                           );
  DEBUG ((EFI_D_ERROR | EFI_D_INFO, " gHiiDatabase->NewPackageList End\n"));
  if (EFI_ERROR (Status)) {
    HiiHandle = NULL;
  }

  //
  // Free the allocated package list
  //
  FreePool (PackageListHeader);

  //
  // Return the new HII Handle
  //
  return HiiHandle;
}
#endif
BOOLEAN
ExecutionInSmm (
  VOID
  )
/*++

Routine Description:

  Decide if the CPU is executing in SMM mode

Arguments:

  None

Returns:

  TRUE  - The CPU is executing in SMM mode
  FALSE - The CPU is not executing in SMM mode

--*/
{
  EFI_STATUS  Status;
  BOOLEAN     InSmm;

  if (mSmmBaseProtocol == NULL) {
    return FALSE;
  }

  Status = mSmmBaseProtocol->InSmm (mSmmBaseProtocol, &InSmm);
  ASSERT_EFI_ERROR (Status);
  return InSmm;
}

EFI_STATUS
EFIAPI
FlushCpuDataCache (
  IN EFI_CPU_ARCH_PROTOCOL     *This,
  IN EFI_PHYSICAL_ADDRESS      Start,
  IN UINT64                    Length,
  IN EFI_CPU_FLUSH_TYPE        FlushType
  )
/*++

Routine Description:

  Flush CPU data cache. If the instruction cache is fully coherent
  with all DMA operations then function can just return EFI_SUCCESS.

Arguments:

  This                - Protocol instance structure
  Start               - Physical address to start flushing from.
  Length              - Number of bytes to flush. Round up to chipset granularity.
  FlushType           - Specifies the type of flush operation to perform.

Returns:

  EFI_SUCCESS           - If cache was flushed
  EFI_UNSUPPORTED       - If flush type is not supported.
  EFI_DEVICE_ERROR      - If requested range could not be flushed.

--*/
{
  if (FlushType == EfiCpuFlushTypeWriteBackInvalidate) {
    AsmWbinvd ();
    return EFI_SUCCESS;
  } else if (FlushType == EfiCpuFlushTypeInvalidate) {
    AsmInvd ();
    return EFI_SUCCESS;
  } else {
    return EFI_UNSUPPORTED;
  }
}

EFI_STATUS
EFIAPI
EnableInterrupt (
  IN EFI_CPU_ARCH_PROTOCOL          *This
  )
/*++

Routine Description:

  Enables CPU interrupts.

Arguments:

  This                - Protocol instance structure

Returns:

  EFI_SUCCESS           - If interrupts were enabled in the CPU
  EFI_DEVICE_ERROR      - If interrupts could not be enabled on the CPU.

--*/
{
  if (!ExecutionInSmm ()) {
    CpuEnableInterrupt ();
  }

  mInterruptState = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
DisableInterrupt (
  IN EFI_CPU_ARCH_PROTOCOL     *This
  )
/*++

Routine Description:

  Disables CPU interrupts.

Arguments:

  This                - Protocol instance structure

Returns:

  EFI_SUCCESS           - If interrupts were disabled in the CPU.
  EFI_DEVICE_ERROR      - If interrupts could not be disabled on the CPU.

--*/
{
  CpuDisableInterrupt ();

  mInterruptState = FALSE;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CpuGetInterruptState (
  IN  EFI_CPU_ARCH_PROTOCOL     *This,
  OUT BOOLEAN                   *State
  )
/*++

Routine Description:

  Return the state of interrupts.

Arguments:

  This                - Protocol instance structure
  State               - Pointer to the CPU's current interrupt state

Returns:

  EFI_SUCCESS           - If interrupts were disabled in the CPU.
  EFI_INVALID_PARAMETER - State is NULL.

--*/
{
  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  *State = mInterruptState;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Init (
  IN EFI_CPU_ARCH_PROTOCOL      *This,
  IN EFI_CPU_INIT_TYPE          InitType
  )
/*++

Routine Description:

  Generates an INIT to the CPU

Arguments:

  This                - Protocol instance structure
  InitType            - Type of CPU INIT to perform

Returns:

  EFI_SUCCESS           - If CPU INIT occurred. This value should never be seen.
  EFI_DEVICE_ERROR      - If CPU INIT failed.
  EFI_UNSUPPORTED       - Requested type of CPU INIT not supported.

--*/
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
RegisterInterruptHandler (
  IN EFI_CPU_ARCH_PROTOCOL         *This,
  IN EFI_EXCEPTION_TYPE            InterruptType,
  IN EFI_CPU_INTERRUPT_HANDLER     InterruptHandler
  )
/*++

Routine Description:

  Registers a function to be called from the CPU interrupt handler.

Arguments:
  This                - Protocol instance structure
  InterruptType       - Defines which interrupt to hook. IA-32 valid range
                         is 0x00 through 0xFF
  InterruptHandler    - A pointer to a function of type
      EFI_CPU_INTERRUPT_HANDLER that is called when a
      processor interrupt occurs. A null pointer
      is an error condition.

Returns:

  EFI_SUCCESS           - If handler installed or uninstalled.
  EFI_ALREADY_STARTED   - InterruptHandler is not NULL, and a handler for
        InterruptType was previously installed
  EFI_INVALID_PARAMETER - InterruptHandler is NULL, and a handler for
        InterruptType was not previously installed.
  EFI_UNSUPPORTED       - The interrupt specified by InterruptType is not
        supported.

--*/
{
  if (InterruptType < 0 || InterruptType > 0xff) {
    return EFI_UNSUPPORTED;
  }

  if (InterruptHandler == NULL && mExternalVectorTable[InterruptType] == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (InterruptHandler != NULL && mExternalVectorTable[InterruptType] != NULL) {
    return EFI_ALREADY_STARTED;
  }

  mExternalVectorTable[InterruptType] = InterruptHandler;
  return EFI_SUCCESS;
}

EFI_STATUS
GetCpuBusRatio (
  OUT UINT32        *Ratio
  )
/*++

Routine Description:

  Returns the CPU core to processor bus frequency ratio.

Arguments:

  Ratio  - Pointer to the CPU core to processor bus frequency ratio.

Returns:

  EFI_SUCCESS           - If the ratio is returned successfully
  EFI_UNSUPPORTED       - If the ratio cannot be measured
  EFI_INVALID_PARAMETER - If the input parameter is not valid

--*/
{
  UINT64              TempQword;

  if (Ratio == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TempQword = AsmReadMsr64 (EFI_MSR_IA32_PERF_STS);
  *Ratio    = (UINT32) (RShiftU64 (TempQword, 8) & 0x1F);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GetTimerValue (
  IN  EFI_CPU_ARCH_PROTOCOL     *This,
  IN  UINT32                    TimerIndex,
  OUT UINT64                    *TimerValue,
  OUT UINT64                    *TimerPeriod OPTIONAL
  )
/*++

Routine Description:

  Returns a timer value from one of the CPU's internal timers. There is no
  inherent time interval between ticks but is a function of the CPU frequency.

Arguments:

  This                - Protocol instance structure.
  TimerIndex          - Specifies which CPU timer is requested.
  TimerValue          - Pointer to the returned timer value.
  TimerPeriod         - A pointer to the amount of time that passes in femtoseconds (10-15) for each
                        increment of TimerValue. If TimerValue does not increment at a predictable
                        rate, then 0 is returned. The amount of time that has passed between two calls to
                        GetTimerValue() can be calculated with the formula
                        (TimerValue2 - TimerValue1) * TimerPeriod. This parameter is optional and may be NULL.

Returns:

  EFI_SUCCESS           - If the CPU timer count was returned.
  EFI_UNSUPPORTED       - If the CPU does not have any readable timers.
  EFI_DEVICE_ERROR      - If an error occurred while reading the timer.
  EFI_INVALID_PARAMETER - TimerIndex is not valid or TimerValue is NULL.

--*/
{
  UINT64          Actual;

  if (TimerValue == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (TimerIndex != 0) {
    return EFI_INVALID_PARAMETER;
  }

  *TimerValue = AsmReadTsc ();

  if (TimerPeriod != NULL) {
    GetActualFrequency (mMetronome, &Actual);
    *TimerPeriod = DivU64x32 (1000000000, (UINT32) Actual);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SetMemoryAttributes (
  IN EFI_CPU_ARCH_PROTOCOL     *This,
  IN EFI_PHYSICAL_ADDRESS      BaseAddress,
  IN UINT64                    Length,
  IN UINT64                    Attributes
  )
/*++

Routine Description:

  Set memory cacheability attributes for given range of memeory

Arguments:

  This                - Protocol instance structure
  BaseAddress         - Specifies the start address of the memory range
  Length              - Specifies the length of the memory range
  Attributes          - The memory cacheability for the memory range

Returns:

  EFI_SUCCESS           - If the cacheability of that memory range is set successfully
  EFI_UNSUPPORTED       - If the desired operation cannot be done
  EFI_INVALID_PARAMETER - The input parameter is not correct, such as Length = 0

--*/
{
  EFI_STATUS                Status;
  UINT64                    TempQword;
  UINT32                    MsrNum, MsrNumEnd;
  UINTN                     MtrrNumber;
  BOOLEAN                   Positive;
  BOOLEAN                   OverLap;
  UINTN                     Remainder;
  EFI_MP_SERVICES_PROTOCOL  *MpService;
  EFI_STATUS                Status1;

  if (mIsFlushingGCD) {
    return EFI_SUCCESS;
  }

  TempQword = 0;

  //
  // Check for invalid parameter
  //
  if (Length == 0) {
    return EFI_INVALID_PARAMETER;
  }

  if ((BaseAddress &~mValidMtrrAddressMask) != 0 || (Length &~mValidMtrrAddressMask) != 0) {
    return EFI_UNSUPPORTED;
  }

  switch (Attributes) {
    case EFI_MEMORY_UC:
      Attributes = EFI_CACHE_UNCACHEABLE;
      break;

    case EFI_MEMORY_WC:
      Attributes = EFI_CACHE_WRITECOMBINING;
      break;

    case EFI_MEMORY_WT:
      Attributes = EFI_CACHE_WRITETHROUGH;
      break;

    case EFI_MEMORY_WP:
      Attributes = EFI_CACHE_WRITEPROTECTED;
      break;

    case EFI_MEMORY_WB:
      Attributes = EFI_CACHE_WRITEBACK;
      break;

    default:
      return EFI_UNSUPPORTED;
  }
  //
  // Check if Fixed MTRR
  //
  Status = EFI_SUCCESS;
  while ((BaseAddress < (1 << 20)) && (Length > 0) && Status == EFI_SUCCESS) {
    PreMtrrChange ();
    Status = ProgramFixedMtrr (Attributes, &BaseAddress, &Length);
    PostMtrrChange ();
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  if (Length == 0) {
    //
    // Just Fixed MTRR. NO need to go through Variable MTRR
    //
    goto Done;
  }
  //
  // since mem below 1m will be override by fixed mtrr, we can set it to 0 to save mtrr.
  //
  if (BaseAddress == 0x100000) {
    BaseAddress = 0;
    Length += 0x100000;
  }
  //
  // Check memory base address alignment
  //
  Remainder = ModU64x32(BaseAddress, (UINT32) Power2MaxMemory (LShiftU64 (Length, 1)));
  if (Remainder != 0) {
    Remainder = ModU64x32 (BaseAddress, (UINT32) Power2MaxMemory (Length));
    if (Remainder != 0) {
      Status = EFI_UNSUPPORTED;
      goto Done;
    }
  }
  //
  // Check overlap
  //
  GetMemoryAttribute ();
  OverLap = CheckMemoryAttributeOverlap (BaseAddress, BaseAddress + Length - 1);
  if (OverLap) {
    Status = CombineMemoryAttribute (Attributes, &BaseAddress, &Length);
    if (EFI_ERROR (Status)) {
      goto Done;
    }

    if (Length == 0) {
      //
      // combine successfully
      //
      Status = EFI_SUCCESS;
      goto Done;
    }
  } else {
    if (Attributes == EFI_CACHE_UNCACHEABLE) {
      Status = EFI_SUCCESS;
      goto Done;
    }
  }
  //
  // Program Variable MTRRs
  //
  if (mUsedMtrr >= (8-EFI_CACHE_NUM_VAR_MTRR_PAIRS_FOR_OS)) { //AMI_OVERRIDE - CSP20140329_22 fix MTRR may cause BSOD with build x32 
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  //
  // Find first unused MTRR
  //
  MsrNumEnd = EFI_MSR_CACHE_VARIABLE_MTRR_BASE + (2 * (UINT32)(AsmReadMsr64(EFI_MSR_IA32_MTRR_CAP) & B_EFI_MSR_IA32_MTRR_CAP_VARIABLE_SUPPORT));
  for (MsrNum = EFI_MSR_CACHE_VARIABLE_MTRR_BASE; MsrNum < MsrNumEnd; MsrNum += 2) {
    if ((AsmReadMsr64 (MsrNum + 1) & B_EFI_MSR_CACHE_MTRR_VALID) == 0) {
      break;
    }
  }

  TempQword = Length;

  if (TempQword == Power2MaxMemory (TempQword)) {
    ProgramVariableMtrr (
      MsrNum,
      BaseAddress,
      Length,
      Attributes
      );
  } else {

    GetDirection (TempQword, &MtrrNumber, &Positive);

    if ((mUsedMtrr + MtrrNumber) > 6) {
      goto Done;
    }

    if (!Positive) {
      Length = Power2MaxMemory (LShiftU64 (TempQword, 1));
      ProgramVariableMtrr (
        MsrNum,
        BaseAddress,
        Length,
        Attributes
        );
      BaseAddress += TempQword;
      TempQword   = Length - TempQword;
      Attributes  = EFI_CACHE_UNCACHEABLE;
    }

    do {
      //
      // Find unused MTRR
      //
      for (; MsrNum < MsrNumEnd; MsrNum += 2) {
        if ((AsmReadMsr64 (MsrNum + 1) & B_EFI_MSR_CACHE_MTRR_VALID) == 0) {
          break;
        }
      }

      Length = Power2MaxMemory (TempQword);
      ProgramVariableMtrr (
        MsrNum,
        BaseAddress,
        Length,
        Attributes
        );
      BaseAddress += Length;
      TempQword -= Length;

    } while (TempQword);

  }

Done:
  Status1 = gBS->LocateProtocol (
                  &gEfiMpServiceProtocolGuid,
                  NULL,
                  &MpService
                  );
  if (!EFI_ERROR (Status1)) {
    ReadMtrrRegisters ();
    Status1 = MpService->StartupAllAPs (
                          MpService,
                          MpMtrrSynchUp,
                          TRUE,
                          NULL,
                          0,
                          NULL,
                          NULL
                          );
  }

  return Status;
}

VOID
EFIAPI
InitializeSmmBasePtr (
  IN EFI_EVENT     Event,
  IN VOID          *Context
  )
/*++

Routine Description:

  Initialize the SmmBase pointer when SmmBase protocol get installed

Arguments:

  Event   - Event whose notification function is being invoked.
  Context - Pointer to the notification functions context, which is implementation dependent.

Returns:

  None

--*/
{
  EFI_STATUS  Status;

  Status = gBS->LocateProtocol (&gEfiSmmBaseProtocolGuid, NULL, &mSmmBaseProtocol);
  if (EFI_ERROR (Status)) {
    mSmmBaseProtocol = NULL;
  }
}

typedef struct {
  UINT8   Bus;
  UINT8   Device;
  UINT8   Function;
} PCI_CONTROLLER_BITMAP;

PCI_CONTROLLER_BITMAP  mHostBus[] = {
  {0, 0, 0},
  {0, 27, 0},
  {0, 30, 0},
  {0, 30, 1},
  {0, 30, 2},
  {0, 30, 3},
  {0, 30, 4},
  {0, 30, 5},
  {0, 30, 6},
  {0, 30, 7},
  {0, 31, 3},
  {0, 16, 0},
  {0, 17, 0},
  {0, 18, 0},
  {0, 26, 0},
  {0, 26, 1},
  {0, 31, 0},
  {0, 21, 0} ,
  {0, 19, 0} ,
  {0, 2, 0},
  {0, 29, 0},
  {0, 20, 0} ,
  {0, 22, 0},
  {0, 3, 0},
  {0, 24, 0},
  {0, 24, 1} ,
  {0, 24, 2},
  {0, 24, 3} ,
  {0, 24, 4},
  {0, 24, 5},
  {0, 24, 6} ,
  {0, 24, 7} ,
  {0, 28, 0},
  {0, 28, 1} ,
  {0, 28, 2},
  {0, 28, 3} ,
  {0, 23, 0},
  {0, 25, 0} ,
};

VOID
CheckPCIExisting (
  OUT UINT64 MSR[4]
  )
{
  UINTN           PciMmBase;
  UINTN           i;
  UINT16          PCI_Data;
  UINT32          Temp;
  UINT64          Bitmap;
  for(i=0; i< sizeof(mHostBus)/sizeof(PCI_CONTROLLER_BITMAP); i++) {
    PciMmBase   = MmPciAddress (
                    0,
                    mHostBus[i].Bus,
                    mHostBus[i].Device,
                    mHostBus[i].Function,
                    0
                    );
    PCI_Data = MmioRead16 (PciMmBase);
    if(PCI_Data !=0xFFFF) {
      if ((0<=mHostBus[i].Device)&&(mHostBus[i].Device<=7)) {
        Temp = ((mHostBus[i].Device)<<3)+ mHostBus[i].Function;
        Bitmap = BIT0;
        MSR[0]|=LShiftU64(Bitmap,Temp);
      } else if((8<=mHostBus[i].Device)&&(mHostBus[i].Device<=15)) {
        Temp = ((mHostBus[i].Device-8)<<3)+ mHostBus[i].Function;
        Bitmap = BIT0;
        MSR[1]|=LShiftU64(Bitmap,Temp);
      } else if((16<=mHostBus[i].Device)&&(mHostBus[i].Device<=23)) {
        Temp = ((mHostBus[i].Device-16)<<3)+ mHostBus[i].Function;
        Bitmap = BIT0;
        MSR[2]|=LShiftU64(Bitmap,Temp) ;
      } else {
        Temp = ((mHostBus[i].Device-24)<<3)+ mHostBus[i].Function;
        Bitmap = BIT0;
        MSR[3]|=LShiftU64(Bitmap,Temp);
      }
    }
  }
  return;
}

VOID
ApPCIConfigWABeforeBoot (UINT64 MSR_Array[4])
{
  // AMI_OVERRIDE - Fix system hang issue when using B0 Stepping. >>
  UINT64            UcodeVer = AsmReadMsr64 (0x8B);
  UcodeVer = *((UINT32*) &UcodeVer+1);
  // AMI_OVERRIDE - Fix system hang issue when using B0 Stepping. <<
  DEBUG ((EFI_D_INFO, "ApPCIConfigWABeforeBoot Begin\n"));

// AMI_OVERRIDE - Fix system hang issue when using B0 Stepping. >>
  if (UcodeVer > 0x20B){
    AsmWriteMsr64 (0x12C, (MSR_Array[0]|BIT63));
    EfiWriteToScript (0x12C,(MSR_Array[0]|BIT63));
    AsmWriteMsr64 (0x12D, (MSR_Array[1]|BIT63));
    EfiWriteToScript (0x12D, (MSR_Array[1]|BIT63));
    AsmWriteMsr64 (0x12E, (MSR_Array[2]|BIT63));
    EfiWriteToScript (0x12E, (MSR_Array[2]|BIT63));
    AsmWriteMsr64 (0x12F, (MSR_Array[3]|BIT63));
    EfiWriteToScript (0x12F, (MSR_Array[3]|BIT63));
  }
// AMI_OVERRIDE - Fix system hang issue when using B0 Stepping. <<
    
  return ;
}

VOID
PCIConfigWA  (
  EFI_EVENT  Event,
  VOID       *Context
  )
/**

@brief

  BIOS has to inform the CPU uCODE to enable the WA for PCI config space access filtering for disabled/non existing PCI devices.
  PUNIT FW or uCODE will emulate a scratch PAD reg for this purpose and BIOS will have to set a bit in this reg.
  On seeing this bit, uCODE will enable the filtering logic. Need to enable this before OS boot loader hand-off.

  @param[in] Event     - A pointer to the Event that triggered the callback.
  @param[in] Context   - A pointer to private data registered with the callback function.


  **/
{
#if !(SLE_FLAG) && !(_SIMIC_)
  EFI_STATUS                 Status;
  EFI_MP_SERVICES_PROTOCOL  *MpService;
  UINT64 MSR_Array[4] = {0};

// Silicon Steppings
// This w/a is only applicable for B0 and B1
  if((PchStepping() != PchB0) && (PchStepping() != PchB1)) {
    return ;
  }

  CheckPCIExisting (MSR_Array);
  Status = gBS->LocateProtocol (
                  &gEfiMpServiceProtocolGuid,
                  NULL,
                  (VOID **) &MpService
                  );
  if (!EFI_ERROR (Status)) {
    // Configure for BSP first
    ApPCIConfigWABeforeBoot(MSR_Array);
    // Confiture the rest APs
    Status = MpService->StartupAllAPs (
                          MpService,
                          (EFI_AP_PROCEDURE) ApPCIConfigWABeforeBoot,
                          TRUE,
                          NULL,
                          0,
                          (VOID *) &MSR_Array,
                          NULL
                          );
  }
#endif //SLE_FLAG && _SIMIC_
  //DEBUG ((EFI_D_INFO, "CPU DXE<CpuInit\\Dxe\\Cpu.c>: CpuInitBeforeBoot() End\n"));
  return ;
}

//AMI_OVERRIDE - EIP137713 Hang at CP 0x68 when limitcpuid is enabled. >>
VOID
CpuidLimitReadyToBoot (
  EFI_EVENT  Event,
  VOID       *Context
  )
{

  EFI_STATUS            Status;
  EFI_MP_SERVICES_PROTOCOL  *MpService;
  DEBUG ((EFI_D_INFO, " \n CreateEvent ProgrameCpuidLimit\n"));

  Status = gBS->LocateProtocol (
				&gEfiMpServiceProtocolGuid,
				NULL,
				&MpService
				);
  if (!EFI_ERROR (Status)) {
  ProgrameCpuidLimit(mMPSystemData);
  Status = MpService->StartupAllAPs (
                          MpService,
                          (EFI_AP_PROCEDURE) ProgrameCpuidLimit,
                          TRUE,
                          NULL,
                          0,
                          (VOID *) &mMPSystemData,
                          NULL
                          );
  }
}
//AMI_OVERRIDE - EIP137713 Hang at CP 0x68 when limitcpuid is enabled. <<

VOID
ReadyToBootFunction (
  EFI_EVENT  Event,
  VOID       *Context
  )
/**

@brief

  Init before boot

  @param[in] Event     - A pointer to the Event that triggered the callback.
  @param[in] Context   - A pointer to private data registered with the callback function.


  **/
{
  EFI_STATUS            Status;
  EFI_EVENT             ExitBootServicesEvent;
  DEBUG ((EFI_D_INFO, "CreateEvent ApPCIConfigWABeforeBoot\n"));
  Status = gBS->CreateEvent (
                  EVT_SIGNAL_EXIT_BOOT_SERVICES,
                  TPL_CALLBACK,
                  PCIConfigWA,
                  NULL,
                  &ExitBootServicesEvent
                  );
  ASSERT_EFI_ERROR (Status);

  return ;
}

#ifdef ECP_FLAG
EFI_STATUS
HobLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );
#endif
EFI_STATUS
EFIAPI
InitializeCpu (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
/*++

Routine Description:

  Initialize the state information for the CPU Architectural Protocol

Arguments:

  ImageHandle - Image handle of the loaded driver
  SystemTable - Pointer to the System Table

Returns:

  EFI_SUCCESS           - thread can be successfully created
  EFI_OUT_OF_RESOURCES  - cannot allocate protocol data structure
  EFI_DEVICE_ERROR      - cannot create the thread

--*/
{
  EFI_STATUS               Status;
  EFI_HANDLE               NewHandle;
//UINT32                   FailedRevision;
  EFI_PLATFORM_CPU_INFO    *PlatformCpuInfoPtr;
  UINTN                    VarSize;
  EFI_PEI_HOB_POINTERS     GuidHob;
  EFI_EVENT                LegacyBootEvent;
  EFI_EVENT    PmAuthEvent = NULL;
  VOID        *RegistrationExitPmAuth = NULL;
  EFI_EVENT                ReadyToBootEvent; //AMI_OVERRIDE - EIP137713 Hang at CP 0x68 when limitcpuid is enabled.
//  EfiInitializeCommonDriverLib (ImageHandle, SystemTable);

  if (ImageHandle != NULL) {
//    DxeInitializeDriverLib (ImageHandle, SystemTable);
#ifdef ECP_FLAG
    HobLibConstructor(ImageHandle, SystemTable);
#endif
    //
    // DXE CPU Post Codes are defined in PostCode.c, variable mPort80Table[]
    //
    Status = REPORT_STATUS_CODE_EX (
               EFI_PROGRESS_CODE,
               EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_PC_DXE_INIT,
               0,
               &gEfiCallerIdGuid,
               NULL,
               NULL,
               0
               );

    //
    // Get Platform CPU Info HOB
    //
    PlatformCpuInfoPtr = NULL;
    ZeroMem (&mPlatformCpuInfo, sizeof(EFI_PLATFORM_CPU_INFO));
    VarSize = sizeof(EFI_PLATFORM_CPU_INFO);
    Status = gRT->GetVariable(
                    EfiPlatformCpuInfoVariable,
                    &gEfiVlv2VariableGuid,
                    NULL,
                    &VarSize,
                    PlatformCpuInfoPtr
                    );
    if (EFI_ERROR(Status)) {
      GuidHob.Raw = GetHobList ();
      if (GuidHob.Raw != NULL) {
        GuidHob.Raw = GetNextGuidHob (&gEfiPlatformCpuInfoGuid, GuidHob.Raw);
        if (GuidHob.Raw != NULL) {
          PlatformCpuInfoPtr = GET_GUID_HOB_DATA (GuidHob.Guid);
          Status = EFI_SUCCESS;
        } else {
          Status = EFI_NOT_FOUND;
        }
      } else {
        Status = EFI_NOT_FOUND;
      }
    }
    if (!EFI_ERROR(Status) && (PlatformCpuInfoPtr != NULL)) {
      CopyMem(&mPlatformCpuInfo, PlatformCpuInfoPtr, sizeof(EFI_PLATFORM_CPU_INFO));
    }

    //
    // Initialize the Global Descriptor Table
    //
    InitializeSelectors ();

    Status = PrepareMemory ();
    ASSERT_EFI_ERROR (Status);

    //
    // Initialize Exception Handlers
    //
    Status = InitializeException (&gCpu);

    CpuInitFloatPointUnit (); //AMI_OVERRIDE - EIP164769 SCT Execution Test-fail

/*  //AMI_OVERRIDE - Fix: X64 hang issue. >>
    //
    // Install CPU Architectural Protocol
    //
    NewHandle = NULL;
    Status = gBS->InstallProtocolInterface (
                    &NewHandle,
                    &gEfiCpuArchProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &gCpu
                    );

    ASSERT_EFI_ERROR (Status);

    Status = REPORT_STATUS_CODE_EX (
               EFI_PROGRESS_CODE,
               EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_PC_DXE_STEP1,
               0,
               &gEfiCallerIdGuid,
               NULL,
               NULL,
               0
               );
*/  //AMI_OVERRIDE - Fix: X64 hang issue. <<

    //
    // Refresh memory space attributes according to MTRRs
    //
    Status = RefreshGcdMemoryAttributes ();
    mIsFlushingGCD = FALSE;
    if (EFI_ERROR(Status)) {
      return Status;
    }

    //
    // Initialize the mPlatformCpu to contain the PlatformCpu protocol,
    // thus no further need to located it later
    //
    Status = InitializePlatformCpuPtr (&mPlatformCpu);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Initialize the mDataHub to contain the DataHub protocol,
    // thus no further need to located it later
    //
    Status = InitializeDataHubPtr (&mDataHub);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = REPORT_STATUS_CODE_EX (
               EFI_PROGRESS_CODE,
               EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_PC_DXE_STEP2,
               0,
               &gEfiCallerIdGuid,
               NULL,
               NULL,
               0
               );


    //
    // Load the microcode if needed
    //
    Status = LoadAllMicrocodeUpdates ();

    //
    // Update microcode for BSP
    //
    //Status  = InitializeMicrocode (mMicrocodePointerBuffer, &FailedRevision, TRUE);
    //Status  = CheckMicrocodeUpdate (0, Status, FailedRevision);


    Status = gBS->LocateProtocol (&gEfiMetronomeArchProtocolGuid, NULL, (VOID **) &mMetronome);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Status = REPORT_STATUS_CODE_EX (
               EFI_PROGRESS_CODE,
               EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_PC_DXE_STEP3,
               0,
               &gEfiCallerIdGuid,
               NULL,
               NULL,
               0
               );

    //AMI_OVERRIDE - Fix: X64 hang issue. >>
    //
    // Install CPU Architectural Protocol
    //
    NewHandle = NULL;
    Status = gBS->InstallProtocolInterface (
                    &NewHandle,
                    &gEfiCpuArchProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    &gCpu
                    );

    ASSERT_EFI_ERROR (Status);

    Status = REPORT_STATUS_CODE_EX (
               EFI_PROGRESS_CODE,
               EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_PC_DXE_STEP1,
               0,
               &gEfiCallerIdGuid,
               NULL,
               NULL,
               0
               );
	//AMI_OVERRIDE - Fix: X64 hang issue. <<

    //
    // Initialize strings to HII database
    //
#ifdef ECP_FLAG
    Status = (gBS->LocateProtocol) (&gEfiHiiDatabaseProtocolGuid,
                                    NULL,
                                    (VOID **) &gHiiDatabase);
    ASSERT_EFI_ERROR (Status);
#endif
    mStringHandle = HiiAddPackages(&gProcessorProducerGuid, NULL, STRING_ARRAY_NAME, NULL);
    ASSERT (mStringHandle != NULL);

    EfiCreateProtocolNotifyEvent (
      &gEfiSmmBaseProtocolGuid,
      TPL_CALLBACK,
      InitializeSmmBasePtr,
      NULL,
      &mSmmBaseRegistration
      );

  }

  Status = REPORT_STATUS_CODE_EX (
             EFI_PROGRESS_CODE,
             EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_PC_DXE_STEP4,
             0,
             &gEfiCallerIdGuid,
             NULL,
             NULL,
             0
             );

  //
  // Initialize MP Support if necessary
  //
  Status = InitializeMpSupport (ImageHandle, SystemTable);

  if (ImageHandle != NULL) {
    Status = REPORT_STATUS_CODE_EX (
               EFI_PROGRESS_CODE,
               EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_PC_DXE_END,
               0,
               &gEfiCallerIdGuid,
               NULL,
               NULL,
               0
               );
  }

//Fix EFI SHELL MSR(120) bit6=0.
    gBS->CreateEvent (
           EVT_NOTIFY_SIGNAL,
           TPL_CALLBACK,
           CpuInitBeforeBoot,
           NULL,
           &PmAuthEvent
           );

    gBS->RegisterProtocolNotify (
           &gExitPmAuthProtocolGuid,
           PmAuthEvent,
           &RegistrationExitPmAuth
           );

  Status = EfiCreateEventLegacyBootEx (
            TPL_CALLBACK,
            CpuInitBeforeBoot,
            NULL,
            &LegacyBootEvent
            );
  ASSERT_EFI_ERROR (Status);

//AMI_OVERRIDE - EIP137713 Hang at CP 0x68 when limitcpuid is enabled. >>
  // Create a ReadyToBoot Event
  Status = EfiCreateEventReadyToBootEx (
									TPL_CALLBACK,
									CpuidLimitReadyToBoot,
									NULL,
									&ReadyToBootEvent
									);
  ASSERT_EFI_ERROR (Status);
//AMI_OVERRIDE - EIP137713 Hang at CP 0x68 when limitcpuid is enabled. <<
  return EFI_SUCCESS;
}

UINT16
DetermineiFsbFromMsr (
  VOID
  )
/*++

Routine Description:

  Determine the processor core frequency

Arguments:

  None

Returns:

  Processor core frequency multiplied by 3


--*/
{

  // Determine the processor core frequency
  //
  UINT64    Temp;
  Temp = (AsmReadMsr64 (BSEL_CR_OVERCLOCK_CONTROL)) & FUSE_BSEL_MASK;
  return miFSBFrequencyTable[(UINT32)(Temp)];

}
EFI_STATUS
GetActualFrequency (
  IN  EFI_METRONOME_ARCH_PROTOCOL   *Metronome,
  OUT UINT64                        *Frequency
  )
/*++

Routine Description:

  Returns the actual CPU core frequency in MHz.

Arguments:

  Metronome       - Metronome protocol
  Frequency       - Pointer to the CPU core frequency

Returns:

  EFI_SUCCESS     - If the frequency is returned successfully

  EFI_INVALID_PARAMETER - If the input parameter is wrong

--*/
{
  UINT64     BeginValue;
  UINT64     EndValue;
  UINTN      TickCount;
  BOOLEAN    mInterruptState;
  EFI_STATUS Status;
  UINT64     TotalValue;

  if (Metronome == NULL || Frequency == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  if (mCpuFrequency == 0) {
    *Frequency = 0;

    //
    // In order to calculate the actual CPU frequency, we keep track of the CPU Tsc value (which
    // increases by 1 for every cycle) for a know period of time. The Metronome is not accurate
    // for the 1st tick, so I choose to wait for 100 ticks, thus the error can be control to be
    // lower than 1%.
    //
    CpuGetInterruptState (&gCpu, &mInterruptState);
    if (mInterruptState) {
      DisableInterrupt (&gCpu);
    }
    //
    // Wait for 3000us = 3ms for the calculation
    // It needs a precise timer to calculate the ticks
    //
    TickCount   = SAMPLE_TICK_COUNT *4;
    while(TRUE) {
      BeginValue  = AsmReadTsc ();
      //  mPlatformCpu->Stall (mPlatformCpu, TickCount);  //in some platform the ACPI timer cannot work correctly.
      Status      = Metronome->WaitForTick (Metronome, (UINT32)TickCount);
      EndValue    = AsmReadTsc ();
      if (!EFI_ERROR (Status)) {
        TotalValue = EndValue - BeginValue;
        break;
      }
    }
    if (mInterruptState) {
      EnableInterrupt (&gCpu);
    }

    mCpuFrequency = MultU64x32 (TotalValue, 10);
    mCpuFrequency = DivU64x32 (mCpuFrequency, Metronome->TickPeriod * (UINT32)TickCount);

  }
  *Frequency = mCpuFrequency;
  return EFI_SUCCESS;
}

EFI_STATUS
InitializePlatformCpuPtr (
  OUT EFI_PLATFORM_CPU_PROTOCOL     **PlatformCpu
  )
/*++

Routine Description:

  Initialize the global PlatformCpu pointer

Arguments:

  PlatformCpu - Pointer to the PlatformCpu protocol as output

Returns:

  EFI_SUCCESS - If the PlatformCpu pointer is initialized successfully

--*/
{
  EFI_STATUS  Status;

  Status = gBS->LocateProtocol (&gEfiPlatformCpuProtocolGuid, NULL, (VOID **) PlatformCpu);
  return Status;
}

EFI_STATUS
InitializeDataHubPtr (
  OUT EFI_DATA_HUB_PROTOCOL        **DataHub
  )
/*++

Routine Description:

  Initialize the global DataHub pointer

Arguments:

  DataHub - Pointer to the DataHub protocol as output

Returns:

  EFI_SUCCESS - If the DataHub pointer is initialized successfully

--*/
{
  EFI_STATUS  Status;

  Status = gBS->LocateProtocol (&gEfiDataHubProtocolGuid, NULL, (VOID **) DataHub);
  return Status;
}

VOID
CpuInitBeforeBoot (
  IN EFI_EVENT    Event,
  IN VOID         *Context
  )
/**

@brief

  Create Perform Final Init before boot to OS

  @param[in] Event     - A pointer to the Event that triggered the callback.
  @param[in] Context   - A pointer to private data registered with the callback function.


  **/
{
#if !(SLE_FLAG) && !(_SIMIC_)
  EFI_STATUS                Status;

  EFI_MP_SERVICES_PROTOCOL  *MpService;

  //DEBUG ((EFI_D_INFO, "CPU DXE<CpuInit\\Dxe\\Cpu.c>: CpuInitBeforeBoot() Start\n"));


  Status = gBS->LocateProtocol (
                  &gEfiMpServiceProtocolGuid,
                  NULL,
                  (VOID **) &MpService
                  );
  if (!EFI_ERROR (Status)) {
    // Configure for BSP first
    ApCpuInitBeforeBoot();
    // Confiture the rest APs
    Status = MpService->StartupAllAPs (
                          MpService,
                          (EFI_AP_PROCEDURE) ApCpuInitBeforeBoot,
                          TRUE,
                          NULL,
                          0,
                          NULL,
                          NULL
                          );
  }
#endif //SLE_FLAG && _SIMIC_
  //DEBUG ((EFI_D_INFO, "CPU DXE<CpuInit\\Dxe\\Cpu.c>: CpuInitBeforeBoot() End\n"));
  return ;
}


VOID
ApCpuInitBeforeBoot ()
/**

@brief

  Create Perform Final Init before boot to OS

  @param[in] Event     - A pointer to the Event that triggered the callback.
  @param[in] Context   - A pointer to private data registered with the callback function.

S
  **/
{
  UINT64            PowerMisc;
  // Lastly IA Core shall be switched to IA_UNTRUSTED Mode by setting ENABLE_IA_UNTRUSTED_MODE to 1.
  PowerMisc = AsmReadMsr64 (EFI_MSR_POWER_MISC);
  //DEBUG ((EFI_D_INFO, "CPU DXE<CpuInit\\Dxe\\Cpu.c>: EFI_MSR_POWER_MISC(0x120)=%X\n",PowerMisc));
  PowerMisc |= B_EFI_MSR_POWER_MISC_ENABLE_IA_UNTRUSTED_MODE;
  //
  // Debug code: TO disable IA untrusted mode
  //
  AsmWriteMsr64 (EFI_MSR_POWER_MISC, PowerMisc);
  //DEBUG ((EFI_D_INFO, "CPU DXE<CpuInit\\Dxe\\Cpu.c>: Switched IA Core to IA_UNTRUSTED Mode. MSR_POWER_MISC(0x120)=%X\n",PowerMisc));
}
