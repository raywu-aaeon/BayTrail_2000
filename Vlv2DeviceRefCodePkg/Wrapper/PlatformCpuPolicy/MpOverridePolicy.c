/*++

Copyright ?1996 - 2004, Intel Corporation.

This source code and any documentation accompanying it ("Material") is furnished
under license and may only be used or copied in accordance with the terms of that
license.  No license, express or implied, by estoppel or otherwise, to any
intellectual property rights is granted to you by disclosure or delivery of these
Materials.  The Materials are subject to change without notice and should not be
construed as a commitment by Intel Corporation to market, license, sell or support
any product or technology.  Unless otherwise provided for in the license under which
this Material is provided, the Material is provided AS IS, with no warranties of
any kind, express or implied, including without limitation the implied warranties
of fitness, merchantability, or non-infringement.  Except as expressly permitted by
the license for the Material, neither Intel Corporation nor its suppliers assumes
any responsibility for any errors or inaccuracies that may appear herein.  Except
as expressly permitted by the license for the Material, no part of the Material
may be reproduced, stored in a retrieval system, transmitted in any form, or
distributed by any means without the express written consent of Intel Corporation.

Module Name:
  MpOverridePolicy.c

Abstract:
  This file contains the protocol member for the policies
  for selecting the BSP, disabling unsupported processors etc.

--*/

#include "PlatformCpuPolicy.h"
#include <SetupDataDefinition.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
#include <PlatformCpuPolicyHooksLib.h>

SETUP_DATA  mSysCfg;

#pragma optimize("g", off)

#define IS_CPU_PRESENT(n)       (mInfoList[n].Status.SocketPopulated != 0)
#define IS_CPU_DISABLED(l, n)   (l[n] != EFI_CPU_CAUSE_NOT_DISABLED)
#define IS_CPU_AVAILABLE(l, n)  (IS_CPU_PRESENT (n) && !IS_CPU_DISABLED (l, n))

//
// Forward declarations of functions used only in this file.
//
VOID *
EFIAPI
CopyMem(
    OUT VOID       *DestinationBuffer,
    IN CONST VOID  *SourceBuffer,
    IN UINTN       Length
);

VOID *
EFIAPI
ZeroMem(
    OUT VOID  *Buffer,
    IN UINTN  Length
);

static
VOID
MpReport(
    VOID
);

static
EFI_STATUS
MpPolicy(
    VOID
);

static
VOID
MpFixDisableList(
    IN  OUT EFI_CPU_STATE_CHANGE_CAUSE     *CpuDisableList
);

static
UINTN
FindFirstInPackage(
    IN  EFI_CPU_STATE_CHANGE_CAUSE        *CpuDisableList,
    IN  UINTN                             PkgNo
);

static
UINTN
MpNoOfAvailable(
    IN      EFI_CPU_STATE_CHANGE_CAUSE     *CpuDisableList
);

static
VOID
MpDisablePerSetup(
    VOID
);

static PROCESSOR_SET_PERFORMANCE  mMpConfig;
static PROCESSOR_SET_PERFORMANCE  mMpConfigSaved;

static
VOID
SaveMpConfig(
    OUT     PROCESSOR_SET_PERFORMANCE         *Performance,
    IN      EFI_CPU_STATE_CHANGE_CAUSE        *CpuDisableList,
    IN      UINTN                             Bsp
);

static
VOID
DisableTheUnfit(
    IN  OUT EFI_CPU_STATE_CHANGE_CAUSE        *CpuDisableList,
    IN UINTN                                  Bsp
);

//
// Copies of the original values passed to the PlatformCpuPolicy.OverridePolicy()
// member.
// It saves space to share the parameters as globals rather than pass them
// around to the support functions.
// Reentrancy is lost, but we do not need this protocol member to be reentrant.
//
static UINT32                     mCount;
static EFI_DETAILED_CPU_INFO      *mInfoList;
static EFI_CPU_STATE_CHANGE_CAUSE *mDisableList;
static EFI_CPU_STATE_CHANGE_CAUSE *mDisableListOrg;
static UINT32                     *mBsp;

//
// Some scratch string space
//
STATIC CHAR16                     StrTmp[64];

EFI_STATUS
PlatformCpuPolicyOverridePolicy(
    IN  EFI_PLATFORM_CPU_PROTOCOL           *This,
    IN  UINT32                              CpuCount,
    IN  EFI_DETAILED_CPU_INFO               *CpuInfoList,
    IN  OUT EFI_CPU_STATE_CHANGE_CAUSE      *CpuDisableList,
    IN  OUT UINT32                          *Bsp
)
/*++

Routine Description:
  This routine relies on info about all processors passed to it to
  determine which processor should be the BSP and which processors to disable.
  This routine sets the policies for the platform in this respect.
  The caller is responsible to provide correct info on all the processors and
  on return from this routine, to effect the settings for all the processors.
  This routine may also produce status reports for any unsupported
  processor configurations like incompatible families, cache sizes, stepping
  combinations etc.
  This routine may not: call MP or CPU drivers for any info, attempt to
  directly change status of any hardware (except for any effect that an
  emitted status report may cause, like port 80, log, etc.).

  Certain assumption are made:
  - All the CPUs on the Package are identical.
  - Thermal issue affecting on CPU is extended to all in the package.
  - The APIC IDs per package increment by order of Die/Core/Thread. Thread first.

Arguments:
  This           - Driver context.
  CpuCount       - Number of CPUs in the system, including disabled and
                   absent.
  CpuInfoList    - Array of detailed processor info for each processor,
                   including disabled and absent. The length is always
                   equal to CpuCount. The index into the array determines
                   the processor number.
  CpuDisableList - List of disabled processors. On input, the list of
                   processors that the caller driver determined to be
                   disabled. On output, the list of processors that this
                   protocol member function wants to disable.
                   The length is always equal to CpuCount. The index into
                   the array determines the processor number.
  Bsp            - On input, the current BSP. On output the BSP to be.

Returns:
  EFI_SUCCESS    - The computation of all OUT parameters
                   was successful and the parameters are valid.
  EFI_NOT_FOUND  - The driver could not find a processor fit for BSP.
                   Further caller action may be e.g., to continue to boot
                   using current BSP, or to display a message and halt.

--*/
{
    EFI_STATUS  Status;
    UINTN       Idx;
    //
    // Copy params to globals for use by other functions in this file.
    //
    mCount        = CpuCount;
    mInfoList     = CpuInfoList;
    mDisableList  = CpuDisableList;
    mBsp          = Bsp;

    DEBUG((EFI_D_ERROR, "MP Policy In:  BSP:%d,  Dsbl:", (UINTN) *mBsp));
    for(Idx = 0; Idx < mCount; Idx++) {
        DEBUG((EFI_D_ERROR, " %d=%d", (UINTN) Idx, (UINTN) mDisableList[Idx]));
    }

    DEBUG((EFI_D_ERROR, "\n"));

    //
    // Preserve the original Disable list. It will be used later.
    //
    mDisableListOrg = AllocatePool(mCount * sizeof(mDisableListOrg[0]));
    ASSERT(mDisableListOrg != NULL);
    CopyMem(mDisableListOrg, mDisableList, mCount * sizeof(mDisableListOrg[0]));

    //
    // Determine the BSP and list of processors to disable.
    //
    Status = MpPolicy();

    //
    // Produce error logs for unsupported configurations.
    //
    MpReport();

    DEBUG((EFI_D_ERROR, "MP Policy Out:  BSP:%d,  Dsbl:", (UINTN) *mBsp));
    for(Idx = 0; Idx < mCount; Idx++) {

        DEBUG((EFI_D_ERROR, " %d=%d", (UINTN) Idx, (UINTN) mDisableList[Idx]));

        if(IS_CPU_DISABLED(mDisableListOrg, Idx)) {
            ASSERT(IS_CPU_DISABLED(mDisableList, Idx));
            //
            // Sanity check: must not be reenabled by OverridePolicy()
            //
        }
    }

    DEBUG((EFI_D_ERROR, "\n"));

    gBS->FreePool(mDisableListOrg);

    return Status;
}

UINT64
Power10U64(
    IN UINT64   Operand,
    IN UINTN    Power
)
{
    UINT64 Result;

    Result = Operand;
    while(Power-- > 0) {
        Result = MultU64x32(Result, 10);
    }
    return Result;
}

EFI_STATUS
MpPolicy(
    VOID
)
/*++

Routine Description:
  This routine relies on info about all processors passed to it to
  determine which processor should be the BSP and which processors to disable.
  This routine sets the policies for the platform in this respect.

Returns:
  EFI_SUCCESS    - The computation of all OUT parameters
                   was successful and the parameters are valid.
--*/

// GC_TODO: function comment is missing 'Arguments:'
//
// GC_TODO: function comment is missing 'Arguments:'
//
// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO:    EFI_NOT_FOUND - add return value to function comment
//
{
    UINTN                 CpuIdx;
    EFI_DETAILED_CPU_INFO *ProcessorInfo;

    for(CpuIdx = 0; CpuIdx < mCount; CpuIdx++) {

        ProcessorInfo = &mInfoList[CpuIdx];
        if(!ProcessorInfo->Status.SocketPopulated) {
            DEBUG((EFI_D_ERROR, "MP: Proc:%d  ABSENT\n", (UINTN) CpuIdx));
            continue;
        }

        DEBUG(
            (EFI_D_ERROR,
             "Proc:%d Pkg:%d Apic:%x MuV/T:%x/%x",
             (UINTN) CpuIdx,
             (UINTN) ProcessorInfo->Context->PackageNumber,
             ProcessorInfo->Context->ApicID,
             (UINTN) ProcessorInfo->MuData.ProcessorMicrocodeRevisionNumber,
             (UINTN) ProcessorInfo->MuData.ProcessorMicrocodeType)
        );
        DEBUG(
            (EFI_D_ERROR,
             " CPUID:%x%x%x",
             (UINTN) ProcessorInfo->CpuId.Signature.ProcessorFamily,
             (UINTN) ProcessorInfo->CpuId.Signature.ProcessorModel,
             (UINTN) ProcessorInfo->CpuId.Signature.ProcessorSteppingId)
        );
        DEBUG(
            (EFI_D_ERROR,
             " %s/",
             StrHzToString(StrTmp, Power10U64(ProcessorInfo->BusFreq.Value, ProcessorInfo->BusFreq.Exponent)))
        );
        DEBUG(
            (EFI_D_ERROR,
             "%s\n",
             StrHzToString(StrTmp, Power10U64(ProcessorInfo->CoreFreq.Value, ProcessorInfo->CoreFreq.Exponent)))
        );
    }

    return EFI_SUCCESS;
}

VOID
MpFixDisableList(
    IN  OUT EFI_CPU_STATE_CHANGE_CAUSE     *CpuDisableList
)
/*++

Routine Description:
  This routine adjusts disabled processor list based on already disabled processors.
  For example, it is required that if an HT thread is bad, the entire core (not package or die) is disabled.
  If the reason is thermal, the entire package should be disabled.

Arguments:
  CpuDisableList - pointer to disable-table of processors to operate on.

--*/

// GC_TODO: function comment is missing 'Returns:'
//
// GC_TODO: function comment is missing 'Returns:'
//
// GC_TODO: function comment is missing 'Returns:'
//
{
    UINTN IdxProc;
    UINTN IdxProc2;

    //
    // Clear disabled by association to be re-evaluated further.
    //
    for(IdxProc = 0; IdxProc < mCount; IdxProc++) {
        if(CpuDisableList[IdxProc] == EFI_CPU_CAUSE_BY_ASSOCIATION) {
            CpuDisableList[IdxProc] = EFI_CPU_CAUSE_NOT_DISABLED;
        }
    }

    for(IdxProc = 0; IdxProc < mCount; IdxProc++) {

        switch(CpuDisableList[IdxProc]) {

        case EFI_CPU_CAUSE_NOT_DISABLED:
        case EFI_CPU_CAUSE_BY_ASSOCIATION:
            //
            // Good processors and those disabled by association are done.
            //
            continue;

        case EFI_CPU_CAUSE_UNSPECIFIED:
        case EFI_CPU_CAUSE_CONFIG_ERROR:
        case EFI_CPU_CAUSE_THERMAL_ERROR:
            //
            // Thermal problems, config errors (most likely resulting from mismatch)
            // and generic/unspecified errors must cause disabling of the entire package.
            //
            for(IdxProc2 = 0; IdxProc2 < mCount; IdxProc2++) {
                if((CpuDisableList[IdxProc2] == EFI_CPU_CAUSE_NOT_DISABLED) &&
                        (mInfoList[IdxProc2].Context->PackageNumber == mInfoList[IdxProc].Context->PackageNumber)
                  ) {

                    CpuDisableList[IdxProc2] = EFI_CPU_CAUSE_BY_ASSOCIATION;
                }
            }
            break;

        case EFI_CPU_CAUSE_USER_SELECTION:
            break;

        case EFI_CPU_CAUSE_INTERNAL_ERROR:
        case EFI_CPU_CAUSE_SELFTEST_FAILURE:
        case EFI_CPU_CAUSE_PREBOOT_TIMEOUT:
        case EFI_CPU_CAUSE_FAILED_TO_START:

        default:
            //
            // All other issues disable all threads in single bad core.
            //
            for(IdxProc2 = 0; IdxProc2 < mCount; IdxProc2++) {
                if((CpuDisableList[IdxProc2] == EFI_CPU_CAUSE_NOT_DISABLED) &&
                        (mInfoList[IdxProc2].Context->PackageNumber == mInfoList[IdxProc].Context->PackageNumber) &&
                        (CpuDisableList[IdxProc2 + 1] != EFI_CPU_CAUSE_USER_SELECTION)
                  ) {
                    //
                    // ESS : if User wants to disable only 1 thread, let hime do that. don't disable the other
                    // threads in the core!!
                    //
                    // Here should be more logical expressions involving Die/Core.
                    // But at this time the data in teh MP COntext struct does not contain enough info.
                    // Tracker #5869 was filed with the SSG to consider changes to the struct and the MP driver.
                    // Till then this expression will disable all units under the same Package.
                    //
                    CpuDisableList[IdxProc2] = EFI_CPU_CAUSE_BY_ASSOCIATION;
                }
            }
            break;
        }
    }
}

UINTN
MpNoOfAvailable(
    IN      EFI_CPU_STATE_CHANGE_CAUSE     *CpuDisableList
)
/*++

Routine Description:
  This routine counts enabled processors i.e., not on the disabled list.

--*/

// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
// GC_TODO:    CpuDisableList - add argument and description to function comment
//
// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
// GC_TODO:    CpuDisableList - add argument and description to function comment
//
// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
// GC_TODO:    CpuDisableList - add argument and description to function comment
//
{
    UINTN Idx;
    UINTN Count;

    Count = 0;
    for(Idx = 0; Idx < mCount; Idx++) {
        if(IS_CPU_AVAILABLE(CpuDisableList, Idx)) {
            Count++;
        }
    }

    return Count;
}

VOID
MpDisablePerSetup(
    VOID
)
/*++

Routine Description:
  This routine disables processors based of selections in Setup.
  Only some of the processors will be touched here, as only some relevant
  combinations of disabled processors are controlled of this Platform Driver.

--*/

// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
//
// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
//
// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
//
{
    UINTN Idx;

    for(Idx = 0; Idx < mCount; Idx++) {

        switch(mDisableList[Idx]) {

        case EFI_CPU_CAUSE_USER_SELECTION:
            //
            // Any processor disabled by user selection will be enabled here
            // to potentially be re-disabled by the part that effects Setup settings.
            //
            //
            // ESS_OVERRIDE_START
            //
            // Dual Core changes
            // mDisableList[Idx] = EFI_CPU_CAUSE_NOT_DISABLED;
            //
            // ESS_OVERRIDE_END
            //
            break;

        case EFI_CPU_CAUSE_BY_ASSOCIATION:
            break;
        }
    }
    //
    // This is an example of disabling a processor by a reason of user selection.
    //
    // mDisableList[] = EFI_CPU_CAUSE_USER_SELECTION;
    //
    MpFixDisableList(mDisableList);
}

static
UINTN
FindFirstInPackage(
    IN  EFI_CPU_STATE_CHANGE_CAUSE        *CpuDisableList,
    IN  UINTN                             PkgNo
)
/*++

Routine Description:
  This routine finds the first enabled processor in the package given processor number.
  The order

Arguments:
  CpuDisableList - List of disabled processors.
  CpuNo          - Processor number as received from MP Services.

Return:
  -1 - No CPU found that was enabled and in the same package

--*/

// GC_TODO: function comment is missing 'Returns:'
// GC_TODO:    PkgNo - add argument and description to function comment
//
// GC_TODO: function comment is missing 'Returns:'
// GC_TODO:    PkgNo - add argument and description to function comment
//
// GC_TODO: function comment is missing 'Returns:'
// GC_TODO:    PkgNo - add argument and description to function comment
//
{
    UINTN ApicId;
    UINTN CpuIdx;
    UINTN Lowest;

    //
    // Find the first non-disabled processor in the package. The unit with lowest APIC ID is considered the first.
    //
    ApicId  = (UINTN) -1;
    Lowest  = (UINTN) -1;
    for(CpuIdx = 0; CpuIdx < mCount; CpuIdx++) {

        if((mInfoList[CpuIdx].Context->PackageNumber == PkgNo) &&
                (mInfoList[CpuIdx].Context->ApicID < ApicId) &&
                (CpuDisableList[CpuIdx] == EFI_CPU_CAUSE_NOT_DISABLED)
          ) {

            ApicId  = mInfoList[CpuIdx].Context->ApicID;
            Lowest  = CpuIdx;
        }
    }

    return Lowest;
}

static
VOID
DisableTheUnfit(
    IN OUT  EFI_CPU_STATE_CHANGE_CAUSE        *CpuDisableList,
    IN  UINTN                                 Bsp
)
/*++

Routine Description:
  This routine computes processors that must be disabled given the BSP.

Arguments:
  CpuDisableList - List of disabled processors.
                   This list will possibly be added with more disabled processors
                   if they are with conflict with the BSP
  Bsp            - The performance is computed against this BSP.

--*/

// GC_TODO: function comment is missing 'Returns:'
//
// GC_TODO: function comment is missing 'Returns:'
//
// GC_TODO: function comment is missing 'Returns:'
//
{
    UINTN IdxCpu;
    //
    //  UINTN IdxCache;
    //
    for(IdxCpu = 0; IdxCpu < mCount; IdxCpu++) {
        //
        // Skip the unavailable.
        //
        if(!IS_CPU_AVAILABLE(CpuDisableList, IdxCpu)) {
            continue;
        }
        //
        // Disable a bad processor.
        //
        if(mInfoList[IdxCpu].Context->Health.Flags.Bits.Status != EFI_MP_HEALTH_FLAGS_STATUS_HEALTHY) {

            CpuDisableList[IdxCpu] = EFI_CPU_CAUSE_INTERNAL_ERROR;
            MpFixDisableList(CpuDisableList);
            continue;
        }
        //
        // Disable a processor with incompatible Family/Model/Setpping with the BSP.
        //
        if((mInfoList[IdxCpu].CpuId.Signature.ProcessorFamily != mInfoList[Bsp].CpuId.Signature.ProcessorFamily) ||
                (mInfoList[IdxCpu].CpuId.Signature.ProcessorModel != mInfoList[Bsp].CpuId.Signature.ProcessorModel)
          ) {
            CpuDisableList[IdxCpu] = EFI_CPU_CAUSE_CONFIG_ERROR;
            MpFixDisableList(CpuDisableList);
            continue;
        }

        /*
        //
        // Disable a processor with unmatching core frequency.
        //
        if ((mInfoList[IdxCpu].CoreFreq.Value != mInfoList[Bsp].CoreFreq.Value) ||
            (mInfoList[IdxCpu].CoreFreq.Exponent != mInfoList[Bsp].CoreFreq.Exponent)
            ) {

          CpuDisableList[IdxCpu] = EFI_CPU_CAUSE_CONFIG_ERROR;
          MpFixDisableList (CpuDisableList);
          goto lbl_continue;
        }
        //
        // Disable with cache sizes different from BSP.
        //
        for (IdxCache = 0; IdxCache < EFI_CACHE_LMAX; IdxCache++) {

          if ((mInfoList[Bsp].CacheSize[IdxCache].Value != mInfoList[IdxCpu].CacheSize[IdxCache].Value) ||
              (mInfoList[Bsp].CacheSize[IdxCache].Exponent != mInfoList[IdxCpu].CacheSize[IdxCache].Exponent)
              ) {

            CpuDisableList[IdxCpu] = EFI_CPU_CAUSE_CONFIG_ERROR;
            MpFixDisableList (CpuDisableList);
            goto lbl_continue;
          }
        }
        */
    }
}

VOID
SaveMpConfig(
    OUT     PROCESSOR_SET_PERFORMANCE         *Performance,
    IN      EFI_CPU_STATE_CHANGE_CAUSE        *CpuDisableList,
    IN      UINTN                             Bsp
)
/*++

Routine Description:
  This routine computes performance of the system given the enabled processors.

Arguments:
  Performance    - The performance is stred in this structure.
  CpuDisableList - List of disabled processors.
  Bsp            - BSP of the set of processors

Returns:
  Performance rating. The caller should not examine this value except for
  comparing it with another value returned from a previous call to this function.
  The comparison my only be performed with MpComparePerformance() function.

--*/
{
    //
    // Assuming that the BSP must be the least capable processor, its data is used for computing the overall performance.
    //
    Performance->MinStepping  = mInfoList[Bsp].CpuId.Signature.ProcessorSteppingId;
    Performance->MinCoreFreq  = mInfoList[Bsp].CoreFreq;
    Performance->MinBusFreq   = mInfoList[Bsp].BusFreq;
    CopyMem(Performance->MinCacheSize, mInfoList[Bsp].CacheSize, sizeof(Performance->MinCacheSize));
    Performance->NoOfProcessorsAvailable = MpNoOfAvailable(CpuDisableList);
}

VOID
MpReport(
    VOID
)
/*++

Routine Description:
  This routine analyzes types and states of all the processors and
  emits status reports for any unsupported processor configurations
  like different families, cache sizes, stepping combinations etc.

--*/

// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
//
// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
//
// GC_TODO: function comment is missing 'Arguments:'
// GC_TODO: function comment is missing 'Returns:'
//
{
    EFI_DETAILED_CPU_INFO                           *ProcessorInfo;
    EFI_DETAILED_CPU_INFO                           *ProcessorInfoBsp;
    UINT32                                          ProcessorNum;
    UINT32                                          CacheLev;

    EFI_COMPUTING_UNIT_CPU_DISABLED_ERROR_DATA      ErrDataDisabled;
    EFI_HOST_PROCESSOR_MISMATCH_ERROR_DATA          ErrDataMismatch;
    EFI_COMPUTING_UNIT_MICROCODE_UPDATE_ERROR_DATA  ErrDataMicrocode;

    UINT16                                          Attributes;
    BOOLEAN                                         NoMicrocode;

    ZeroMem(&ErrDataDisabled, sizeof(ErrDataDisabled));
    ZeroMem(&ErrDataMicrocode, sizeof(ErrDataMicrocode));
    ZeroMem(&ErrDataMismatch, sizeof(ErrDataMismatch));

    //
    // Populate common fields in the StatusCode structs.
    //
    ErrDataDisabled.DataHeader.Size   = (UINT16)(sizeof(ErrDataDisabled) - sizeof(EFI_STATUS_CODE_DATA));
    ErrDataMismatch.DataHeader.Size   = (UINT16)(sizeof(ErrDataMismatch) - sizeof(EFI_STATUS_CODE_DATA));
    ErrDataMicrocode.DataHeader.Size  = (UINT16)(sizeof(ErrDataMicrocode) - sizeof(EFI_STATUS_CODE_DATA));

    ErrDataDisabled.DataHeader.Type = ErrDataMismatch.DataHeader.Type = ErrDataMicrocode.DataHeader.Type = gEfiStatusCodeSpecificDataGuid;
    ErrDataDisabled.DataHeader.HeaderSize = ErrDataMismatch.DataHeader.HeaderSize = ErrDataMicrocode.DataHeader.HeaderSize = (UINT16) sizeof(EFI_STATUS_CODE_DATA);

    DEBUG((EFI_D_ERROR, "MpPolicy Report:\n"));
    DEBUG((EFI_D_ERROR, "++++ BSP+++:Proc:%d\n", (UINTN) *mBsp));

    //
    // Iterate and analyze data for all processors
    //
    ProcessorInfoBsp = &mInfoList[*mBsp];
    for(ProcessorNum = 0; ProcessorNum < mCount; ProcessorNum++) {
        //
        // Skip the BSP, it could never be at fault.
        //
        if(*mBsp == ProcessorNum) {
            continue;
        }
        //
        // Get the currect processor.
        //
        ProcessorInfo = &mInfoList[ProcessorNum];

        //
        // Skip absent.
        //
        if(!ProcessorInfo->Status.SocketPopulated) {
            continue;
        }

        Attributes = 0;

        //
        // Processor Family mismatch, MAJOR.
        //
        if(ProcessorInfo->CpuId.Signature.ProcessorFamily != ProcessorInfoBsp->CpuId.Signature.ProcessorFamily) {

            Attributes |= EFI_COMPUTING_UNIT_MISMATCH_FAMILY;
        }
        //
        // Processor Model mismatch, MAJOR.
        //
        if(ProcessorInfo->CpuId.Signature.ProcessorModel != ProcessorInfoBsp->CpuId.Signature.ProcessorModel) {

            Attributes |= EFI_COMPUTING_UNIT_MISMATCH_MODEL;
        }
        //
        // Processor Setpping mismatch, MINOR.
        // no error for equal or for N/N-1, BSP must be N-1, all others are erroneous.
        //
        if((
                    ProcessorInfo->CpuId.Signature.ProcessorSteppingId >
                    (ProcessorInfoBsp->CpuId.Signature.ProcessorSteppingId + 1)
                ) ||
                (ProcessorInfo->CpuId.Signature.ProcessorSteppingId < ProcessorInfoBsp->CpuId.Signature.ProcessorSteppingId)
          ) {

            Attributes |= EFI_COMPUTING_UNIT_MISMATCH_STEPPING;
        }
        //
        // Processor Maximum FSB Frequency mismatch, MAJOR.
        //
        if((ProcessorInfo->BusFreq.Value != ProcessorInfoBsp->BusFreq.Value) ||
                (ProcessorInfo->BusFreq.Exponent != ProcessorInfoBsp->BusFreq.Exponent)
          ) {

            Attributes |= EFI_COMPUTING_UNIT_MISMATCH_FSB_SPEED;
        }
        //
        // Processor Maximum Frequency mismatch, MAJOR.
        //
        if((ProcessorInfo->CoreFreq.Value != ProcessorInfoBsp->CoreFreq.Value) ||
                (ProcessorInfo->CoreFreq.Exponent != ProcessorInfoBsp->CoreFreq.Exponent)
          ) {

            Attributes |= EFI_COMPUTING_UNIT_MISMATCH_SPEED;
        }
        //
        // Processor Cache Size mismatch for all supported cache levels, MAJOR.
        //
        for(CacheLev = 0; CacheLev < EFI_CACHE_LMAX; CacheLev++) {

            if((ProcessorInfo->CacheSize[CacheLev].Value != ProcessorInfoBsp->CacheSize[CacheLev].Value) ||
                    (ProcessorInfo->CacheSize[CacheLev].Exponent != ProcessorInfoBsp->CacheSize[CacheLev].Exponent)
              ) {

                Attributes |= EFI_COMPUTING_UNIT_MISMATCH_CACHE_SIZE;
            }
        }

        NoMicrocode = (BOOLEAN)(ProcessorInfo->MuData.ProcessorMicrocodeRevisionNumber == 0);

        if((Attributes != 0) || NoMicrocode) {
            DEBUG((EFI_D_ERROR, "---- Bad AP:Proc:%d  Attribs:%x\n", (UINTN) ProcessorNum, (UINTN) Attributes));
        } else {
            DEBUG((EFI_D_ERROR, "+++ Good AP:Proc:%d\n", (UINTN) ProcessorNum));
        }
        //
        // Processor Microcode not loaded, log MAJOR.
        //
        // Commented out till the microcode version info issue is fixed in the CPU/MP driver.
        // if (NoMicrocode) {
        //  gRT->ReportStatusCode (
        //        EFI_ERROR_CODE | EFI_ERROR_MAJOR,
        //        EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_EC_MICROCODE_UPDATE,
        //        (UINT32)ProcessorNum,
        //        &gEfiPlatformCpuProtocolGuid,
        //        (EFI_STATUS_CODE_DATA*)&ErrDataMicrocode
        //        );
        // }
        //
        // Check and log MAJOR errors.
        //
        ErrDataMismatch.Instance = (UINT8) *mBsp;
        ErrDataMismatch.Attributes = (UINT16)
                                     (
                                         Attributes &
                                         (
                                             EFI_COMPUTING_UNIT_MISMATCH_FAMILY |
                                             EFI_COMPUTING_UNIT_MISMATCH_MODEL |
                                             EFI_COMPUTING_UNIT_MISMATCH_FSB_SPEED |
                                             EFI_COMPUTING_UNIT_MISMATCH_CACHE_SIZE |
                                             EFI_COMPUTING_UNIT_MISMATCH_SPEED
                                         )
                                     );

        if(ErrDataMismatch.Attributes != 0) {
            ReportStatusCodeEx(
                EFI_ERROR_CODE | EFI_ERROR_MAJOR,
                EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_EC_MISMATCH,
                (UINT32) ProcessorNum,
                &gEfiPlatformCpuProtocolGuid,
                NULL,
                (EFI_STATUS_CODE_DATA *) &ErrDataMismatch,
                sizeof(EFI_HOST_PROCESSOR_MISMATCH_ERROR_DATA)
            );
        }
        //
        // Check and log MINOR errors.
        //
        ErrDataMismatch.Attributes = (UINT16)(Attributes & EFI_COMPUTING_UNIT_MISMATCH_STEPPING);
        if(ErrDataMismatch.Attributes != 0) {
            ReportStatusCodeEx(
                EFI_ERROR_CODE | EFI_ERROR_MINOR,
                EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_HP_EC_MISMATCH,
                (UINT32) ProcessorNum,
                &gEfiPlatformCpuProtocolGuid,
                NULL,
                (EFI_STATUS_CODE_DATA *) &ErrDataMismatch,
                sizeof(EFI_HOST_PROCESSOR_MISMATCH_ERROR_DATA)
            );
        }
        //
        // Log disabled processors. A processor is considered disabled by software if the original disable differs from the disabel list returned from OverridePolicy().
        //
        if(IS_CPU_DISABLED(mDisableList, ProcessorNum)) {
            ErrDataDisabled.Cause             = mDisableList[ProcessorNum];
            ErrDataDisabled.SoftwareDisabled  = (BOOLEAN)(mDisableList[ProcessorNum] != mDisableListOrg[ProcessorNum]);
            ReportStatusCodeEx(
                EFI_ERROR_CODE | EFI_ERROR_MINOR,
                EFI_COMPUTING_UNIT_HOST_PROCESSOR | EFI_CU_EC_DISABLED,
                (UINT32) ProcessorNum,
                &gEfiPlatformCpuProtocolGuid,
                NULL,
                (EFI_STATUS_CODE_DATA *) &ErrDataDisabled,
                sizeof(EFI_COMPUTING_UNIT_CPU_DISABLED_ERROR_DATA)
            );
        }
    }
}

UINTN
MpComparePerformance(
    IN PROCESSOR_SET_PERFORMANCE  *MpConfig1,
    IN PROCESSOR_SET_PERFORMANCE  *MpConfig2
)
/*++

Routine Description:
  This routine compares performance of two systems given two sets of
  enabled processors.
  It is platform specific because technology changes from platform
  to platform and criteria determining better performance will change.

Arguments:
  MpConfig1,
  MpConfig2 - Two performances to compare.

Returns:
  1 - MpConfig1 is better than, or identical to MpConfig2.
  2 - MpConfig2 is better than MpConfig1.

--*/
{
    INTN  Idx;

    //
    // Compare the various parameters of two systems and determine the best of the two.
    //
    //
    // The number of processors is most important. If there is inequality, the performance with most processors wins.
    //
    if(MpConfig2->NoOfProcessorsAvailable > MpConfig1->NoOfProcessorsAvailable) {
        return 2;
    }
    //
    // Next, the core frequency rules.
    //
    if(Power10U64(MpConfig2->MinCoreFreq.Value, MpConfig2->MinCoreFreq.Exponent) > Power10U64(
                MpConfig1->MinCoreFreq.Value,
                MpConfig1->MinCoreFreq.Exponent
            )) {
        return 2;
    }
    //
    // Cache sizes are important. Start comparing from the highest cache level.
    //
    for(Idx = EFI_CACHE_LMAX - 1; Idx >= 0; Idx--) {
        if(LShiftU64(MpConfig2->MinCacheSize[Idx].Value, MpConfig2->MinCacheSize[Idx].Exponent) > LShiftU64(
                    MpConfig1->MinCacheSize[Idx].Value,
                    MpConfig1->MinCacheSize[Idx].Exponent
                )) {
            return 2;
        }
    }
    //
    // Next, the FSB frequency.
    //
    if(Power10U64(MpConfig2->MinBusFreq.Value, MpConfig2->MinBusFreq.Exponent) > Power10U64(
                MpConfig1->MinBusFreq.Value,
                MpConfig1->MinBusFreq.Exponent
            )) {
        return 2;
    }
    //
    // Lastly, the stepping could matter for performance too.
    //
    else if(MpConfig2->MinStepping > MpConfig1->MinStepping) {
        return 2;
    }
    //
    // Everything being equal, by default prefer the first one.
    //
    return 1;
}

CHAR16 *
StrHzToString(
    OUT CHAR16                    *String,
    IN  UINT64                    Val
)
/*++

Routine Description:
  Converts frequency in Hz to Unicode string.
  Three significant digits are delivered.
  Used for things like processor info display.

Arguments:
  String - string that will contain the frequency.
  Val    - value to convert, minimum is  100000 i.e., 0.1 MHz.

--*/

// GC_TODO: function comment is missing 'Returns:'
//
// GC_TODO: function comment is missing 'Returns:'
//
// GC_TODO: function comment is missing 'Returns:'
//
{
    CHAR16        HlpStr[8];
    UINT32        i;
    UINT32        IdxPoint;
    UINT32        IdxUnits;
    static CHAR16 *FreqUnits[] = { L" Hz", L" kHz", L" MHz", L" GHz", L" THz", L" PHz" };

    //
    // Normalize to 9999 or less.
    //
    i = 0;
    while(Val >= 10000) {
        Val = DivU64x32(Val, 10);
        i++;
    }
    //
    // Make it rounded to the nearest, but only by
    // a .3. This assures that .6 is not rounded.
    //
    if(Val >= 1000) {
        Val += 3;
        Val = DivU64x32(Val, 10);
        i++;
    }

    UnicodeValueToString(String, (UINTN)Val, 0, 0);

    //
    // Get rid of that cursed number!
    //
    if(!StrCmp(&String[1], L"66")) {
        String[2] = L'7';
    }
    //
    // Compute index to the units substrings.
    //
    IdxUnits = (i + 2) / 3;

    if(IdxUnits >= (sizeof(FreqUnits) / sizeof(FreqUnits)[0])) {
        //
        // Frequency is too high.
        //
        StrCpy(String, L"OVERFLOW");
        return String;
    }
    //
    // Compute the position of the decimal point.
    //
    IdxPoint = i % 3;

    //
    // Test if decimal point needs to be inserted.
    //
    if(IdxPoint != 0) {
        //
        // Save the part after decimal point.
        //
        StrCpy(HlpStr, &String[IdxPoint]);

        //
        // Insert the point.
        //
        String[IdxPoint] = L'.';

        //
        // Reattach the saved part.
        //
        StrCpy(&String[IdxPoint + 1], HlpStr);

        //
        // Clear the insignificant zero.
        //
        if(String[3] == L'0') {
            String[4 - IdxPoint] = L'\0';
        }
    }
    //
    // Attach units.
    //
    StrCat(String, FreqUnits[IdxUnits]);

    return String;
}

