/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PpmInitialization.c

Abstract:

  Platform power management initialization code.  This code determines current
  user configuration and modifies and loads ASL as well as initializing chipset
  and processor features to enable the proper power management.

  Acronyms:
    PPM   Platform Power Management
    GV    Geyserville
    TM    Thermal Monitor
    IST   Intel(R) Speedstep technology
    HT    Hyper-Threading Technology

--*/

//
// Statements that include other files
//
#include <Ppm.h>
#include <PowerManagement.h>
#ifdef ECP_FLAG
#include <EfiScriptLib.h>
EFI_GUID gPpmPlatformPolicyProtocolGuid = PPM_PLATFORM_POLICY_PROTOCOL_GUID;
EFI_GUID gPowerManagementAcpiTableStorageGuid = POWER_MANAGEMENT_ACPI_TABLE_STORAGE_GUID;
#else
#include <IchPpmLib.h>
#include <PchAccess.h>
#include <MchPpmLib.h>
#include <CpuRegs.h>
#include <IndustryStandard/Acpi30.h>
#include <Library/AslUpdateLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <CpuPpmLib.h>
#include <IstAppletLib.h>
#include <Library/SmmServicesTableLib.h>
#include <Protocol/AcpiSupport.h>
#include <Protocol/GlobalNvsArea.h>
#include <Protocol/BootScriptSave.h>
#include <Protocol/PpmPlatformPolicy.h>
#include <Guid/PowerManagementAcpiTableStorage.h>
#endif
//
// This is not a standard protocol, as it is never published.
// It is more of a dynamic internal library.
//
#include <Protocol/PpmProcessorSupport2.h>

//
// Global variables
//

//
// PPM policy configurations
//
extern PPM_PLATFORM_POLICY_PROTOCOL mPpmPlatformPolicyProtocol;

//
// FVID Table Information
//
extern UINT16 mNumberOfStates;
extern FVID_TABLE *mFvidPointer;

//
// Power management ACPI base address
//
UINT16 AcpiBase;

//
// Global NVS area (communication buffer between SMM and ASL code)
// This area is special because it is in ACPI NVS memory and should
// not be relocated by the OS.  It is accessed in BS, SMM, and by ASL.
//
extern EFI_GLOBAL_NVS_AREA *mGlobalNvsAreaPtr;

//
// CDV iFSB Frequency
//
UINT16 miFSBFrequency;

//
// Last requested GV state
//
UINT16 mRequestedState;

//
// Default FVID table
// One header field plus states
//
FVID_TABLE mEmptyFvidTable[FVID_MAX_STATES + 1];
FVID_TABLE *mFvidPointer = &mEmptyFvidTable[0];

//
// Timer global data
//
UINT8                          mPpmCstTmrFlags;
UINTN                          mPpmTscCorrFactor;
UINTN                          mPpmTscCorrFactorRem;
UINTN                          mPpmCstTscCorrRem;
UINT64                         mPpmCstTscTicks;

//
// Globals to support updating ACPI Tables
//

EFI_ACPI_SUPPORT_PROTOCOL       *mAcpiSupport = NULL;

EFI_ACPI_DESCRIPTION_HEADER     *mCpu0IstTable  = NULL;
EFI_ACPI_DESCRIPTION_HEADER     *mApIstTable    = NULL;
EFI_ACPI_DESCRIPTION_HEADER     *mCpu0CstTable  = NULL;
EFI_ACPI_DESCRIPTION_HEADER     *mApCstTable    = NULL;
EFI_ACPI_DESCRIPTION_HEADER     *mCpuPmTable    = NULL;
EFI_ACPI_DESCRIPTION_HEADER     *mCpu0TstTable  = NULL;
EFI_ACPI_DESCRIPTION_HEADER     *mApTstTable  = NULL;

//
// S3 resume scripting protocol
//
extern  EFI_BOOT_SCRIPT_SAVE_PROTOCOL *mBootScriptSave;

//
// Function prototypes
//
STATIC
UINT32
FindStateFrequency (
  IN  UINT16 RatioSetting
  );

VOID
InitializePpm (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Initialize the processor power management based on hardware capabilities
  and user configuration settings.

Arguments:

  This                Pointer to the protocol instance

Returns:

  None

--*/
{
  UINT32 PpmFlagsMask;       // User configuration mask

  //
  // Set PPM initialization global variables
  //
  PpmPreInit (This);

  //
  // Initialize flags for the current processor
  //
  This->SetPpmFlags (This);

  //
  // Save the hardware features.
  //
  This->PpmFeatureFlags = This->PpmFlags;

  //
  // Determine current user configuration
  //
  SetUserConfigurationPpmFlagsMask (&PpmFlagsMask);

  //
  // Modify PpmFlags based on user configuration
  //
  This->PpmFlags &= PpmFlagsMask;

  //
  // Initialize P states
  //
  if ((This->PpmFlags & PPM_GV3) && (This->InitGv3 != NULL) ) {
    This->InitGv3 (This, mFvidPointer, &mPpmPlatformPolicyProtocol);
    mNumberOfStates = mFvidPointer[0].FvidHeader.Gv3States;
    if (This->PpmFlags & PPM_DYNAMIC_FSB) {
      EnableMchDynamicFsbFrequencySwitching ();
    }
  }

  //
  // Test for EMTTM supported and requested and initialize if true.
  //
  if ((This->PpmFlags & (PPM_EMTTM | PPM_GV3)) == (PPM_EMTTM | PPM_GV3)) {
    if (This->EnableEmttm != NULL) {
      This->EnableEmttm (This, mFvidPointer);
    }
  }

  //
  // Save the number of states for easy access by SPSS and NPSS patching.
  //

  //
  // Initialize C states, some is general, some is processor specific
  // Dynamic loading of CST SSDT tables occurs at PpmPostInit.
  //

  //  @NOTE: This feature is no longer available in Silvermont Core.
  //         Therefore, is disabled by default. Write to this bit will
  //         trigger GP1 fault.
  // see description of MSR IA32_MISC_ENABLES bit[10]
  //
  //EnableFerr ();

  //
  // Configure C States setting and Enable C States if C-State setup option is enabled
  //
  //This->EnableCStates (This, (AcpiBase + PM_CST_LVL2), (mIoTrapBaseAddress + CST_REQ_OFFSET));
  This->EnableCStates (This, 0, 0); // VLV PMC does not support legacy I/O control for ACPI.

///
// Remove the function because VLV is not support switch C-State by FADT IO method
//
//  if (This->PpmFlags & PPM_C_STATES) {
//    ConfigureFadtCStates (This);
//  }

  //
  // Initialize the TSC update
  //
  if ((This->PpmFlags & PPM_TSC) && (This->EnableTsc != NULL)) {
    This->EnableTsc (This, mFvidPointer, miFSBFrequency, &mPpmCstTmrFlags, &mPpmTscCorrFactor, &mPpmTscCorrFactorRem);
    mPpmCstTscCorrRem = 0;
  }

  //
  // Initialize thermal features
  //
  This->InitThermal (This, &mPpmPlatformPolicyProtocol);

  //
  // Initialize TM1 before TM2 because some processors (Dothan)
  // only support one at a time, so enabling TM2 later results in TM2
  // instead of TM1.
  //
  if (This->PpmFlags & PPM_TM) {
    This->EnableTm (This);
  }
  //if (This->PpmFlags & PPM_TM2) {
  //  This->EnableTm2 (This, mFvidPointer);
  //}
  if (This->PpmFlags & PPM_PROC_HOT) {
    This->EnableProcHot (This);
  }

  //
  // Initialize PPM ASL code
  //
  mGlobalNvsAreaPtr->PpmFlags = This->PpmFlags;
  InitializePpmAcpiTable (This);

  //
  // Complete initialization
  //
  PpmPostInit (This);
}

VOID
PpmPreInit (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Initialize global variables used during PPM init
    miFSBFrequency - iFSB frequency.
    mFvidPointer - Set FVID states
    mAcpiSupport - Set ACPI support protocol instance

Arguments:

  This                Pointer to the protocol instance

Returns:

  None

--*/
{
  EFI_STATUS                    Status;

  //
  // Get the ACPI Base Address
  //
  AcpiBase = GetAcpiBase ();

  //
  // Determine the processor core iFSB frequency
  //

  miFSBFrequency = DetermineiFsbFromMsr();

  //
  // If specified, create a custom the FVID table.
  // (The settings populating the FVID table may not be correct for the
  // specific processor, and it is up to the user to specify settings
  // applicable to the processor being used.)
  //
  SetMem (mFvidPointer, sizeof (mEmptyFvidTable), 0);
  if (mPpmPlatformPolicyProtocol.CustomVidTable.VidNumber >= 2) {
    CreateCustomFvidTable (This, mFvidPointer);
  }

  //
  // Locate ACPI support protocol
  //
  Status = gBS->LocateProtocol (&gEfiAcpiSupportProtocolGuid, NULL, (VOID **) &mAcpiSupport);
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize runtime PPM code
  //
  InitializePpmRuntime (This);
}

VOID
SetUserConfigurationPpmFlagsMask (
  IN OUT UINT32 *PpmFlagsMask
  )
/*++

Routine Description:

  Set the PPM flags based on current user configuration

Arguments:

  PpmFlagsMask  Mask of feature options to be enabled as specified by the policy

Returns:

  None

--*/
{
  UINT32 Ecx = 0;
  EFI_CPUID_REGISTER mCpuid0B = { 0, 0, 0, 0 };
  //
  // Initialize flags
  //
  *PpmFlagsMask = 0;
  *PpmFlagsMask |= PPM_TSC;
  *PpmFlagsMask |= PPM_MWAIT_EXT;

  // Retrieve Processor Topology.
  Ecx = 1;
  AsmCpuidEx (EFI_CPUID_CORE_TOPOLOGY, Ecx, &mCpuid0B.RegEax, &mCpuid0B.RegEbx, &mCpuid0B.RegEcx, &mCpuid0B.RegEdx);
  //
  // Configure based on setup values
  //
  if (mPpmPlatformPolicyProtocol.FunctionEnables.EnableGv) {
    *PpmFlagsMask |= PPM_GV3;
  }
  //if (mPpmPlatformPolicyProtocol.FunctionEnables.EnableTm2) {
  //  *PpmFlagsMask |= PPM_TM2;
  //}

  if (mPpmPlatformPolicyProtocol.FunctionEnables.EnableCx) {
    *PpmFlagsMask |= PPM_C1;

    if (mPpmPlatformPolicyProtocol.FunctionEnables.EnableCxe) {
      *PpmFlagsMask |= PPM_C1E;
    }

    if (mPpmPlatformPolicyProtocol.FunctionEnables.EnableC4) {
      *PpmFlagsMask |= PPM_C4;
    }

    if (mPpmPlatformPolicyProtocol.FunctionEnables.EnableC6) {
      *PpmFlagsMask |= (PPM_C6|PPM_C4);
    }

    if (mPpmPlatformPolicyProtocol.FunctionEnables.EnableC7) {
      *PpmFlagsMask |= (PPM_C7|PPM_C6|PPM_C4);
    }
  }
  //
  // end if EnableCx
  //
  if (mPpmPlatformPolicyProtocol.FunctionEnables.EnableTm) {
    *PpmFlagsMask |= PPM_TM;
  }

  if (mPpmPlatformPolicyProtocol.FunctionEnables.EnableProcHot) {
    *PpmFlagsMask |= PPM_PROC_HOT;
  }

  if (mPpmPlatformPolicyProtocol.FunctionEnables.EnableCMP) {
    //if dual core present
    if ((mCpuid0B.RegEbx  & 0xFF) > 1) {
      *PpmFlagsMask |= (PPM_CMP);
    }
    //if Quad core present
    if ((mCpuid0B.RegEbx  & 0xFF) > 2) {
      *PpmFlagsMask |= (PPM_QUAD);
    }
  }

  if (mPpmPlatformPolicyProtocol.FunctionEnables.EnableTurboMode) {
    *PpmFlagsMask |= PPM_TURBO;
  }

  if (mPpmPlatformPolicyProtocol.FunctionEnables.EnableEmttm) {
    *PpmFlagsMask |= PPM_EMTTM;
  }

  //
  // Check if MCH supports dynamic FSB frequency switching
  // We assume that the chipset will support it.
  //
  if ((mPpmPlatformPolicyProtocol.FunctionEnables.EnableDynamicFsb) && MchSupportDynamicFsbFrequencySwitching ()) {
    *PpmFlagsMask |= PPM_DYNAMIC_FSB;
  }

  if (mPpmPlatformPolicyProtocol.FunctionEnables.TStatesEnable) {
    *PpmFlagsMask |= PPM_TSTATES;
  }

  if (mPpmPlatformPolicyProtocol.BootInLfm == PPM_DISABLE) {
    *PpmFlagsMask |= PPM_BOOT_P_ST_HFM;
  }

  if (mPpmPlatformPolicyProtocol.FunctionEnables.S0ixSupport) {
    *PpmFlagsMask |= PPM_S0ix;
  }
}

VOID
AcpiPatchSpssNpss (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Patch the SMI _PSS (SPSS) package with performance values and the native _PSS (NPSS) package with the GV3 values

  Uses ratio/VID values from the FVID table to fix up the control values in the NPSS and SPSS.

  (1) Find NPSS or SPSS package:
    (1.1) Find the _PR_CPU0 scope.
    (1.2) Save a pointer to the package length.
    (1.3) Find the NPSS or SPSS AML name object.
  (2) Resize the NPSS and SPSS package.
  (3) Fix up the NPSS and SPSS package entries
    (3.1) Check Turbo mode support.
    (3.2) Check Dynamic FSB support.
  (4) Fix up the Processor block and \_PR_CPU0 Scope length.
  (5) Update SSDT Header with new length.
  (6) Renames PSS packages for ACPI utility evaluation

Arguments:

  This                Pointer to the protocol instance

Returns:

  None

--*/
{
  UINT8                           *CurrPtr;
  UINT8                           *EndOfTable;
  UINT8                           i;
  UINT16                          NewPackageLength;
  UINT16                          MaxPackageLength;
  UINT16                          Temp;
  UINT16                          *PackageLength;
  UINT16                          *ScopePackageLengthPtr = NULL;
  UINT32                          *Signature;
  PSS_PACKAGE_LAYOUT              *PssPackage = NULL;

  //
  // Calculate new package length
  //
  NewPackageLength = Temp = (UINT16) (mNumberOfStates * sizeof (PSS_PACKAGE_LAYOUT) + 3);
  MaxPackageLength = (UINT16) (FVID_MAX_STATES * sizeof (PSS_PACKAGE_LAYOUT) + 3);

  //
  // Locate the SSDT package in the IST table
  //
  CurrPtr = (UINT8*) mCpu0IstTable;
  EndOfTable =(UINT8*) (CurrPtr + mCpu0IstTable->Length);
  for (; CurrPtr <= EndOfTable; CurrPtr++) {
    Signature = (UINT32*) (CurrPtr + 1);

    //
    // If we found the the _PR_CPU0 scope, save a pointer to the package length
    //
    if ((*CurrPtr == AML_SCOPE_OP) && (*(Signature + 1) == SIGNATURE_32 ('_', 'P', 'R', '_')) && (*(Signature + 2) == SIGNATURE_32 ('C', 'P', 'U', '0'))) {
      ScopePackageLengthPtr = (UINT16*) (CurrPtr + 1);
    }

    //
    // Patch the SMI _PSS (SPSS) package with performance values and
    // the native _PSS (NPSS) package with the GV3 values
    //
    if ((*CurrPtr == AML_NAME_OP) && ((*Signature == SIGNATURE_32 ('S', 'P', 'S', 'S')) || (*Signature == SIGNATURE_32 ('N', 'P', 'S', 'S')))) {
      //
      // Check table dimensions
      // SPSS and NPSS packages reserve space for FVID_MAX_STATES number of P-states so check if the
      // current number of P- states is more than FVID_MAX_STATES. Also need to update the SSDT contents
      // if the current number of P-states is less than FVID_MAX_STATES.
      //
      ASSERT (mNumberOfStates <= FVID_MAX_STATES);
      if (mNumberOfStates < FVID_MAX_STATES) {
        *(CurrPtr + 8) = (UINT8) mNumberOfStates;
        PackageLength = (UINT16*) (CurrPtr + 6);

        //
        // Update the Package length in AML package length format
        //
        *PackageLength = ((NewPackageLength & 0x0F) | 0x40) | ((Temp << 4) & 0x0FF00);

        //
        // Move SSDT contents
        //
        CopyMem ((CurrPtr + NewPackageLength), (CurrPtr + MaxPackageLength), EndOfTable - (CurrPtr + MaxPackageLength));

        //
        // Save the new end of the SSDT
        //
        EndOfTable = EndOfTable - (MaxPackageLength - NewPackageLength);
      }

      PssPackage = (PSS_PACKAGE_LAYOUT*) (CurrPtr + 9);

      for (i = 1; i <= mNumberOfStates; i++) {
        //
        // Update the NPSS or SPSS table
        //

        //
        // If Turbo mode is supported, add one to the HFM frequency
        //
        if ((This->PpmFlags & PPM_TURBO) && (i == 1)) {
          PssPackage->CoreFrequency  = FindStateFrequency (mFvidPointer[i+1].FvidState.BusRatio);
          PssPackage->CoreFrequency++;
        } else {
          PssPackage->CoreFrequency  = FindStateFrequency (mFvidPointer[i].FvidState.BusRatio);
        }

        PssPackage->Power = (UINT32) mFvidPointer[i].FvidState.Power;

        //
        // If it's SPSS table, Status is simply the state number.
        // Use the state number w/ OS command value so that the
        // legacy interface may be used.  Latency for SMM is 100 + BM latency.
        //
        if (*Signature == SIGNATURE_32 ('S', 'P', 'S', 'S')) {
          PssPackage->TransLatency  = SMI_PSTATE_LATENCY;
          PssPackage->Control       = (UINT32) (OS_REQUEST | ((i - 1) << 8));
          PssPackage->Status        = i - 1;
        } else {
          //
          // If it's NPSS table, Control is the PERF_CTL value.
          // Status entry is the same as control entry.
          // TransLatency uses 10
          //
          PssPackage->TransLatency  = NATIVE_PSTATE_LATENCY;
          PssPackage->Control       = (UINT32) ((mFvidPointer[i].FvidState.BusRatio << 8) | (mFvidPointer[i].FvidState.Vid));
          PssPackage->Status        = (UINT32) ((mFvidPointer[i].FvidState.BusRatio << 8) | (mFvidPointer[i].FvidState.Vid));
        }

        PssPackage->BMLatency = PSTATE_BM_LATENCY;
        PssPackage++;
      }
    }
  }
  //
  // Update the Package length in AML package length format
  //
  ASSERT (ScopePackageLengthPtr != NULL);

  CurrPtr                 = (UINT8*) ScopePackageLengthPtr;
  NewPackageLength        = Temp = (UINT16) (EndOfTable - CurrPtr);
  if (ScopePackageLengthPtr != NULL) {
    *ScopePackageLengthPtr  = ((NewPackageLength & 0x0F) | 0x40) | ((Temp << 4) & 0x0FF00);
  }
  mCpu0IstTable->Length   = (UINT32) (EndOfTable - (UINT8*) mCpu0IstTable);

  //
  // If CMP is enabled, we only support native P state control,
  // So _PSS is renamed to XPSS and SPSS is renamed to _PSS
  // And tables will be dynamically loaded
  // This helps debug because AML parsers know how to parse _PSS package.
  //
  if (This->PpmFlags & PPM_CMP) {
    //
    // Search the IST SSDT
    //
    CurrPtr = (UINT8*) mCpu0IstTable;
    for (; CurrPtr <= EndOfTable; CurrPtr++)  {
      Signature = (UINT32*) (CurrPtr + 1);

      //
      // If NPSS
      //
      if ((*CurrPtr == AML_NAME_OP) && (*Signature == SIGNATURE_32 ('N', 'P', 'S', 'S'))) {
        //
        // Rename to _PSS
        //
        *Signature = SIGNATURE_32 ('_', 'P', 'S', 'S');
      }

      //
      // If _PSS
      //
      if ((*(CurrPtr-1) == AML_METHOD_OP) && (*Signature == SIGNATURE_32 ('_', 'P', 'S', 'S'))) {
        //
        // Rename to XPSS (unrecognized method so OS will ignore)
        //
        *Signature = SIGNATURE_32 ('X', 'P', 'S', 'S');
      }
    }
  }
}


VOID
EnableFerr (
  VOID
  )
/*++

Routine Description:

  Enable FERR# Interrupt Reporting Enable of IA32_MISC_ENABLE bit10
  When this flag is set and the processor is in the stop-clock state
  (STPCLK# is asserted), asserting the FERR# pin signals to the processor
  that an interrupt (such as, INIT#, BINIT#, INTR, NMI, SMI#, or RESET#)
  is pending and that the processor should return to normal operation to handle the interrupt.

Arguments:

  None

Returns:

  None

--*/
{
  MSR_REGISTER                Ia32MiscEnable;

  //
  // Enable FERR# in the CPU MSR EFI_MSR_IA32_MISC_ENABLE
  //
  Ia32MiscEnable.Qword = AsmReadMsr64 (EFI_MSR_IA32_MISC_ENABLE);
  Ia32MiscEnable.Qword |= FERR_MUX_ENABLE;
  AsmWriteMsr64 (EFI_MSR_IA32_MISC_ENABLE, Ia32MiscEnable.Qword);
}

VOID
PpmPostInit (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Completes processor power management initialization
    (1) Initializes the TSC update variables.
    (2) Initializes the GV state for processors.
    (3) Saves MSR state for S3
    (4) Adds a callback (SMI) in S3 resume script to restore the MSR

Arguments:

  This                Pointer to the protocol instance

Returns:

  None

--*/
{
//  EFI_STATUS                      Status;
//  UINTN                           Index;
//  UINTN                           PState;
//  UINT8                           Data8;

  //
  // Set Boot P-state based on Policy.
  //
  This->SetBootPState(This);
/*
  //
  // Save boot Script by DXE driver.
  // 
  //
  // Generate an SMI to restore the MSRs when resuming from S3
  //
  Data8 = mPpmPlatformPolicyProtocol.S3RestoreMsrSwSmiNumber;
  Status = S3BootScriptSaveIoWrite(
             EfiBootScriptWidthUint8,
             (UINT64) R_PCH_APM_CNT,
             (UINTN) 1,
             &Data8
             );
  ASSERT_EFI_ERROR (Status);
*/
  //
  // Configure Turbo Power Limits
  //
  if (This->PpmFlags & PPM_TURBO) {
    This->ConfigureTurboPowerLimit (This, &mPpmPlatformPolicyProtocol);
  }
  //
  // Save the MSRs so that they can be restored while S3 resume
  //
  This->S3SaveMsr (This);
}

EFI_STATUS
ConfigureFadtCStates (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Configure the FACP for C state support

Arguments:

  This                Pointer to the protocol instance

Returns:

  None

--*/
{
  EFI_STATUS                                  Status;
  EFI_ACPI_DESCRIPTION_HEADER                 *Table;
  EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE   *FadtPointer;
  INTN                                        Index;
  UINTN                                       Handle;
  EFI_ACPI_TABLE_VERSION                      Version;

  //
  // Locate table with matching ID
  //
  Index = 0;
  do {
    Status = mAcpiSupport->GetAcpiTable (mAcpiSupport, Index, (VOID **) &Table, &Version, &Handle);
    if (Status == EFI_NOT_FOUND) {
      break;
    }
    ASSERT_EFI_ERROR (Status);
    Index++;
  } while (Table->Signature != EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE);

  //
  // Can't have ACPI without FADT, so safe to assert
  //
  ASSERT (Table->Signature == EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE);
  FadtPointer = (EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE*) Table;

  //
  // Verify expected state.  Should be initialized to off during build.
  //
  ASSERT (FadtPointer->PLvl2Lat >= FADT_C2_LATENCY_DISABLED);
  ASSERT (FadtPointer->PLvl3Lat >= FADT_C3_LATENCY_DISABLED);

  //
  // Configure C states
  //
  if (This->PpmFlags & PPM_C4) {
    //
    // Enable C2 in FADT.
    //
    FadtPointer->PLvl2Lat = FADT_C2_LATENCY;
  }

  if (This->PpmFlags & (PPM_C6)) {
    //
    // Enable C3 in FADT.
    //
    FadtPointer->PLvl3Lat = FADT_C3_LATENCY;
  }

  //
  // Update the table
  //
  Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, Table, TRUE, Version, &Handle);
  ASSERT_EFI_ERROR (Status);
#ifdef ECP_FLAG
  (gBS->FreePool) (Table);
#else
  gBS->FreePool (Table);
#endif

  return EFI_SUCCESS;
}

VOID
CreateCustomFvidTable (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This,
  IN OUT FVID_TABLE                         *FvidPointer
  )
/*++

Routine Description:

  Create a custom FVID table based on setup options.
  Caller is responsible for providing a large enough table.

Arguments:

  This          Pointer to the protocol instance
  FvidPointer   Table to update, must be initialized.

Returns:

  None

--*/
{
  UINT16 Index;

  //
  // Fill in the table header
  //
  FvidPointer[0].FvidHeader.Stepping     = mPpmPlatformPolicyProtocol.CustomVidTable.VidCpuid;
  FvidPointer[0].FvidHeader.MaxVid       = mPpmPlatformPolicyProtocol.CustomVidTable.VidMaxVid;
  FvidPointer[0].FvidHeader.MaxBusRatio  = mPpmPlatformPolicyProtocol.CustomVidTable.VidMaxRatio;
  FvidPointer[0].FvidHeader.Gv3States    = mPpmPlatformPolicyProtocol.CustomVidTable.VidNumber;


  //
  // Fill in the state data
  //
  for (Index = 0; Index < mPpmPlatformPolicyProtocol.CustomVidTable.VidNumber; Index++) {
    FvidPointer[Index + 1].FvidState.State     = Index;
    FvidPointer[Index + 1].FvidState.Vid       = mPpmPlatformPolicyProtocol.CustomVidTable.StateVid[Index];
    FvidPointer[Index + 1].FvidState.BusRatio  = mPpmPlatformPolicyProtocol.CustomVidTable.StateRatio[Index];
  }
}

EFI_STATUS
PatchCpuPmTable (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Update the SSDT table pointers and config DWORD CFGD with the PpmFlags current configuration value

Arguments:

  This                Pointer to the protocol instance

Returns:

  EFI_SUCCESS         The function completed successfully

--*/
{
  UINT8         *CurrPtr;
  UINT32        *Signature;
  SSDT_LAYOUT   *SsdtPackage = NULL;
  CFGD_LAYOUT   *CfgdPackage = NULL;

  //
  // Locate the SSDT package
  //
  CurrPtr = (UINT8*) mCpuPmTable;
  for (; CurrPtr <= ((UINT8*) mCpuPmTable + mCpuPmTable->Length); CurrPtr++) {
    Signature = (UINT32*) (CurrPtr + 1);
    if ((*CurrPtr == AML_NAME_OP) && *Signature == SIGNATURE_32 ('S', 'S', 'D', 'T')) {
      //
      // Update the SSDT table pointers for dynamically loaded tables
      //
      SsdtPackage = (SSDT_LAYOUT*) CurrPtr;

      //
      // Set the P-State SSDT table information
      //
      SsdtPackage->Cpu0IstAddr  = (UINT32) (UINTN) mCpu0IstTable;
      SsdtPackage->Cpu0IstLen   = mCpu0IstTable->Length;
      SsdtPackage->ApIstAddr    = (UINT32) (UINTN) mApIstTable;
      SsdtPackage->ApIstLen     = mApIstTable->Length;

      //
      // Set the C-State SSDT table information
      //
      SsdtPackage->Cpu0CstAddr  = (UINT32) (UINTN) mCpu0CstTable;
      SsdtPackage->Cpu0CstLen   = mCpu0CstTable->Length;
      SsdtPackage->ApCstAddr    = (UINT32) (UINTN) mApCstTable;
      SsdtPackage->ApCstLen     = mApCstTable->Length;
    }
    //
    // Update the config DWORD CFGD with the PpmFlags current configuration value
    //
    if ((*CurrPtr == AML_NAME_OP) && *Signature == SIGNATURE_32 ('C', 'F', 'G', 'D')) {
      CfgdPackage = (CFGD_LAYOUT*) CurrPtr;
      CfgdPackage->Value = This->PpmFlags;
      break;
    }
  }

  //
  // Assert if we didn't update the PM table
  //
  ASSERT (SsdtPackage != NULL);
  ASSERT (CfgdPackage != NULL);

  return EFI_SUCCESS;
}


EFI_STATUS
PatchCpu0IstTable (
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Update the SPSS table with the control register address
Arguments:

  This                Pointer to the protocol instance

Returns:

  EFI_SUCCESS         The function completed successfully

--*/
{
  UINT8         *CurrPtr;
  UINT8         *EndPtr;
  UINT32        PctLength;
  UINT32        *Signature;
  PCTP_LAYOUT   *PctpPackage = NULL;


  //
  // Locate the SSDT package
  //
  CurrPtr = (UINT8*) mCpu0IstTable;
  EndPtr = CurrPtr + mCpu0IstTable->Length;

  for (; CurrPtr <= EndPtr; CurrPtr++) {
    Signature = (UINT32*) (CurrPtr + 3);
    //
    // Update the address with the SMM P-State control port
    //
    if ((*CurrPtr == AML_METHOD_OP) && *Signature == SIGNATURE_32 ('_', 'P', 'C', 'T')) {
      //
      // This code assumes a two-byte package length encoding, ASSERT if not.
      //
      ASSERT ((*(CurrPtr + 1) & 0xC0) == 0x40);
      PctLength = (((UINT32)(*(CurrPtr + 2))) << 4) + (((UINT32)(*(CurrPtr + 1))) & 0x0F);
      EndPtr = CurrPtr + PctLength;
      //
      // There are four different resource packages in the provided _PCT method, so
      // we search for the only one with a width of 16 and a default address of 0x8000.
      //
      for (; CurrPtr <= EndPtr; CurrPtr++) {
        if((*CurrPtr == 0x82) && (*(CurrPtr + 4) == 0x10) && (*((UINT32*)(CurrPtr + 7)) == 0x8000)) {
          PctpPackage = (PCTP_LAYOUT*) CurrPtr;
          //PctpPackage->RegAddress = mIoTrapBaseAddress + IST_REQ_OFFSET;
          PctpPackage->RegAddress = 0x8000;
          break;
        }
      }
      break;
    }
  }

  //
  // Assert if we didn't update the _PCT port address
  //
  ASSERT (PctpPackage != NULL);

  return EFI_SUCCESS;
}


VOID
InitializePpmAcpiTable(
  IN OUT PPM_PROCESSOR_SUPPORT_PROTOCOL_2   *This
  )
/*++

Routine Description:

  Locate the PPM ACPI tables data file and read ACPI SSDT tables.
  Publish the appropriate SSDT based on current configuration and capabilities.

Arguments:

  This                Pointer to the protocol instance

Returns:

  None

--*/
{
  EFI_STATUS                    Status;
  EFI_HANDLE                    *HandleBuffer;
  UINTN                         NumberOfHandles;
  EFI_FV_FILETYPE               FileType;
  UINT32                        FvStatus;
  EFI_FV_FILE_ATTRIBUTES        Attributes;
  UINTN                         Size;
  UINTN                         i;
#ifdef ECP_FLAG
  EFI_FIRMWARE_VOLUME_PROTOCOL *FwVol = NULL;
#else
  EFI_FIRMWARE_VOLUME2_PROTOCOL *FwVol = NULL;
#endif
  INTN                          Instance;
  EFI_ACPI_TABLE_VERSION        Version;
  EFI_ACPI_COMMON_HEADER        *CurrentTable;
  EFI_ACPI_DESCRIPTION_HEADER   *TempTable;
  UINTN                         AcpiTableHandle;

  //
  // Locate protocol.
  // There is little chance we can't find an FV protocol
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
#ifdef ECP_FLAG
                  &gEfiFirmwareVolumeProtocolGuid,
#else
                  &gEfiFirmwareVolume2ProtocolGuid,
#endif
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Looking for FV with ACPI storage file
  //
  for (i = 0; i < NumberOfHandles; i++) {

    //
    // Get the protocol on this handle
    // This should not fail because of LocateHandleBuffer
    //
    Status = gBS->HandleProtocol (
                    HandleBuffer[i],
#ifdef ECP_FLAG
                    &gEfiFirmwareVolumeProtocolGuid,
#else
                    &gEfiFirmwareVolume2ProtocolGuid,
#endif
                    (VOID **)&FwVol
                    );
    ASSERT_EFI_ERROR (Status);

    //
    // See if it has the ACPI storage file
    //
    Size      = 0;
    FvStatus  = 0;
    Status = FwVol->ReadFile (
                      FwVol,
                      &gPowerManagementAcpiTableStorageGuid,
                      NULL,
                      &Size,
                      &FileType,
                      &Attributes,
                      &FvStatus
                      );

    //
    // If we found it, then we are done
    //
    if (Status == EFI_SUCCESS) {
      break;
    }
  }

  //
  // Our exit status is determined by the success of the previous operations
  // If the protocol was found, Instance already points to it.
  //
  //
  // Free any allocated buffers
  //
#ifdef ECP_FLAG
  (gBS->FreePool) (HandleBuffer);
#else
  gBS->FreePool (HandleBuffer);
#endif

  //
  // Sanity check that we found our data file
  //
  ASSERT (FwVol != NULL);
  if( FwVol == NULL ) {
    return;
  }
  //
  // By default, a table belongs in all ACPI table versions published.
  //
  Version = EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0 | EFI_ACPI_TABLE_VERSION_3_0;

  //
  // Read tables from the storage file.
  //
  Instance = 0;
  CurrentTable = NULL;
  while (Status == EFI_SUCCESS) {
    Status = FwVol->ReadSection (
                      FwVol,
                      &gPowerManagementAcpiTableStorageGuid,
                      EFI_SECTION_RAW,
                      Instance,
                      (VOID **)&CurrentTable,
                      &Size,
                      &FvStatus
                      );

    if (!EFI_ERROR (Status)) {
      //
      // Check the table ID to modify the table
      //
      switch (((EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable)->OemTableId) {

        case (SIGNATURE_64 ('C', 'p', 'u', '0', 'I', 's', 't', 0)):
          mCpu0IstTable = (EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable;
          if ((This->PpmFlags & PPM_GV3 )||(This->PpmFlags & PPM_BOOT_P_ST_HFM)) {
            //
            // Patch the SMI _PSS (SPSS) package with performance values and
            // the native _PSS (NPSS) package with the GV3 values
            //
            AcpiPatchSpssNpss (This);
            //
            // Update the Cpu0Ist SSDT table in the ACPI tables.
            //
            // @NOTE: This patching routine of SSDT is targetting IO interface for P-state control.
            //        VLV SoC does not support IO Trap.
            //PatchCpu0IstTable (This);
          }
          break;

        case (SIGNATURE_64 ('C', 'p', 'u', '0', 'C', 's', 't', 0)):
          mCpu0CstTable = (EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable;
          break;

        case (SIGNATURE_64 ('C', 'p', 'u', '0', 'T', 's', 't', 0)):
          mCpu0TstTable = (EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable;
          break;

        case (SIGNATURE_64 ('A', 'p', 'I', 's', 't', 0, 0, 0)):
          mApIstTable = (EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable;
          break;

        case (SIGNATURE_64 ('A', 'p', 'C', 's', 't', 0, 0, 0)):
          mApCstTable = (EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable;
          break;

        case (SIGNATURE_64 ('A', 'p', 'T', 's', 't', 0, 0, 0)):
          mApTstTable = (EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable;
          break;

        case (SIGNATURE_64 ('C', 'p', 'u', 'P', 'm', 0, 0, 0)):
          mCpuPmTable = (EFI_ACPI_DESCRIPTION_HEADER*) CurrentTable;
          break;

        default:
          break;
      }

      //
      // Increment the instance
      //
      Instance++;
      CurrentTable = NULL;
    }
  }

////Fixed the BSOD "ACPI_BIOS_USE_OS_MEMORY" when Disable CMP function
#ifdef ECP_FLAG
  Status = (gBS->AllocatePool) (EfiReservedMemoryType, mApIstTable->Length, (VOID **) &TempTable);
#else
  Status = gBS->AllocatePool (EfiReservedMemoryType, mApIstTable->Length, (VOID **) &TempTable);
#endif
  ASSERT_EFI_ERROR (Status);
  CopyMem (TempTable, mApIstTable, mApIstTable->Length);
#ifdef ECP_FLAG
  (gBS->FreePool) (mApIstTable);
#else
  gBS->FreePool (mApIstTable);
#endif
  mApIstTable = TempTable;
  AcpiChecksum (mApIstTable, mApIstTable->Length, OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum));

#ifdef ECP_FLAG
  Status = (gBS->AllocatePool) (EfiReservedMemoryType, mApCstTable->Length, (VOID **) &TempTable);
#else
  Status = gBS->AllocatePool (EfiReservedMemoryType, mApCstTable->Length, (VOID **) &TempTable);
#endif
  ASSERT_EFI_ERROR (Status);
  CopyMem (TempTable, mApCstTable, mApCstTable->Length);
#ifdef ECP_FLAG
  (gBS->FreePool) (mApCstTable);
#else
  gBS->FreePool (mApCstTable);
#endif
  mApCstTable = TempTable;
  AcpiChecksum (mApCstTable, mApCstTable->Length, OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum));

  //
  // If we are CMP, then the PPM tables are dynamically loaded:
  //   We need to publish the CpuPm table to the ACPI tables, and move the IST and CST
  //   tables that are dynamically loaded to a separate location so that we can fix the
  //   addresses in the CpuPm table.
  // Otherwise (non-CMP)
  //   We need to publish CPU 0 tables only, and IST and CST tables only if IST and CST are enabled
  //
  if (This->PpmFlags & PPM_CMP) {
    //
    // Copy tables to our own location and checksum them
    //

#ifdef ECP_FLAG
    Status = (gBS->AllocatePool) (EfiReservedMemoryType, mCpu0IstTable->Length, (VOID **) &TempTable);
#else
    Status = gBS->AllocatePool (EfiReservedMemoryType, mCpu0IstTable->Length, (VOID **) &TempTable);
#endif
    ASSERT_EFI_ERROR (Status);
    CopyMem (TempTable, mCpu0IstTable, mCpu0IstTable->Length);
#ifdef ECP_FLAG
    (gBS->FreePool) (mCpu0IstTable);
#else
    gBS->FreePool (mCpu0IstTable);
#endif
    mCpu0IstTable = TempTable;
    AcpiChecksum (mCpu0IstTable, mCpu0IstTable->Length, OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum));

//    Status = gBS->AllocatePool (EfiReservedMemoryType, mApIstTable->Length, (VOID **) &TempTable);
//    ASSERT_EFI_ERROR (Status);
//    CopyMem (TempTable, mApIstTable, mApIstTable->Length);
//    gBS->FreePool (mApIstTable);
//    mApIstTable = TempTable;
//    AcpiChecksum (mApIstTable, mApIstTable->Length, OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum));

#ifdef ECP_FLAG
    Status = (gBS->AllocatePool) (EfiReservedMemoryType, mCpu0CstTable->Length, (VOID **) &TempTable);
#else
    Status = gBS->AllocatePool (EfiReservedMemoryType, mCpu0CstTable->Length, (VOID **) &TempTable);
#endif
    ASSERT_EFI_ERROR (Status);
    CopyMem (TempTable, mCpu0CstTable, mCpu0CstTable->Length);
#ifdef ECP_FLAG
    (gBS->FreePool) (mCpu0CstTable);
#else
    gBS->FreePool (mCpu0CstTable);
#endif
    mCpu0CstTable = TempTable;
    AcpiChecksum (mCpu0CstTable, mCpu0CstTable->Length, OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum));

//    Status = gBS->AllocatePool (EfiReservedMemoryType, mApCstTable->Length, (VOID **) &TempTable);
//    ASSERT_EFI_ERROR (Status);
//    CopyMem (TempTable, mApCstTable, mApCstTable->Length);
//    gBS->FreePool (mApCstTable);
//    mApCstTable = TempTable;
//    AcpiChecksum (mApCstTable, mApCstTable->Length, OFFSET_OF (EFI_ACPI_DESCRIPTION_HEADER, Checksum));

  } else {
    //
    // CMP disabled, so statically load the tables
    //

    //
    // Add IST SSDT if GV3 enabled
    //
    if (This->PpmFlags & PPM_GV3) {
      AcpiTableHandle = 0;
      Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, mCpu0IstTable, TRUE, Version, &AcpiTableHandle);
      ASSERT_EFI_ERROR (Status);
    }

    //
    // Add CST SSDT if C states are enabled
    //
    if (This->PpmFlags & PPM_C_STATES) {
      AcpiTableHandle = 0;
      Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, mCpu0CstTable, TRUE, Version, &AcpiTableHandle);
      ASSERT_EFI_ERROR (Status);
    }

    //
    // Since we are UP, there is no need for the CPU 1 tables
    //

    //
    // At this time, we can free all tables, since they have been copied into ACPI tables by ACPI support protocol
    //
#ifdef ECP_FLAG
    (gBS->FreePool) (mCpu0IstTable);
    (gBS->FreePool) (mCpu0CstTable);
#else
    gBS->FreePool (mCpu0IstTable);
    gBS->FreePool (mCpu0CstTable);
#endif

////Fixed the BSOD "ACPI_BIOS_USE_OS_MEMORY" when Disable CMP function
//    gBS->FreePool (mApIstTable);
//    gBS->FreePool (mApCstTable);
  }

  //
  // Update the CpuPm SSDT table in the ACPI tables.
  //
  PatchCpuPmTable (This);

  AcpiTableHandle = 0;
  Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, mCpuPmTable, TRUE, Version, &AcpiTableHandle);
  ASSERT_EFI_ERROR (Status);
#ifdef ECP_FLAG
  (gBS->FreePool) (mCpuPmTable);
#else
  gBS->FreePool (mCpuPmTable);
#endif

  if (This->PpmFlags & PPM_TSTATES) {
    //
    // Load the Cpu0Tst SSDT table in the ACPI tables
    //
    AcpiTableHandle = 0;
    Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, mCpu0TstTable, TRUE, Version, &AcpiTableHandle);
    ASSERT_EFI_ERROR (Status);
#ifdef ECP_FLAG
    (gBS->FreePool) (mCpu0TstTable);
#else
    gBS->FreePool (mCpu0TstTable);
#endif

    //
    // If the CMP is enabled then load the ApTst SSDT table in the ACPI tables
    //
    if (This->PpmFlags & PPM_CMP) {
      AcpiTableHandle = 0;
      Status = mAcpiSupport->SetAcpiTable (mAcpiSupport, mApTstTable, TRUE, Version, &AcpiTableHandle);
      ASSERT_EFI_ERROR (Status);
    }
  }
#ifdef ECP_FLAG
  (gBS->FreePool) (mApTstTable);
#else
  gBS->FreePool (mApTstTable);
#endif

  //
  // Publish all ACPI Tables
  //
  Status = mAcpiSupport->PublishTables (mAcpiSupport, Version);
  ASSERT_EFI_ERROR (Status);
}

STATIC
UINT32
FindStateFrequency (
  IN  UINT16 RatioSetting
  )
/*++

Routine Description:

  Returns the frequency (MHz) of a given state based on the ratio

Arguments:

  RatioSetting  The ratio settings for the state

Returns:

  None

--*/
{
  UINT32  Frequency;

  Frequency = (UINT32)(RatioSetting * miFSBFrequency);

  return Frequency;
}
