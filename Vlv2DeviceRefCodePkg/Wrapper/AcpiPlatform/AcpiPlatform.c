/*++

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  AcpiPlatform.c

Abstract:

  ACPI Platform Driver


--*/

#include <Protocol/TcgService.h>
#include <Protocol/FirmwareVolume.h>
#include "AcpiPlatform.h"
#include "AcpiPlatformHooks.h"
#include "AcpiPlatformHooksLib.h"
#include "Platform.h"
#include <Hpet.h>
#include <Mcfg.h>
#include "Osfr.h"
#include <Guid/Vlv2Variable.h>
#include <Guid/PlatformInfo.h>
#include <Protocol/CpuIo.h>
#include <Slic.h>
#include <Guid/BoardFeatures.h>
#include <Protocol/AcpiSupport.h>
#include <Protocol/AcpiS3Save.h>
#include <Protocol/Ps2Policy.h>
#include <Protocol/Cpu.h>
#include <SetupMode.h>
#include <Guid/AcpiTableStorage.h>
#include <Guid/EfiVpdData.h>
#include <PchAccess.h>
#include <Protocol/SeCOperation.h>
#include "CpuType.h"
#include "CpuRegs.h"
#include <Library/PchPlatformLib.h>

#undef EFI_EVENT_RUNTIME_CONTEXT

#if CMOS_MANAGER_SUPPORT == 1
#include <CmosAccess.h>
#include <SspTokens.h>
#endif //#if CMOS_MANAGER_SUPPORT == 1
#include <ACPI50.h>

typedef struct {
    UINT32  RegEax;
    UINT32  RegEbx;
    UINT32  RegEcx;
    UINT32  RegEdx;
} EFI_CPUID_REGISTER;

#include <Guid/PlatformCpuInfo.h>

extern EFI_GUID gEfiSetupVariableGuid;
extern EFI_GUID gAmiGlobalVariableGuid;	//(EIP112015)

CHAR16    EfiPlatformCpuInfoVariable[] = L"PlatformCpuInfo";
CHAR16    gACPIOSFRModelStringVariableName[] = ACPI_OSFR_MODEL_STRING_VARIABLE_NAME;
CHAR16    gACPIOSFRRefDataBlockVariableName[] = ACPI_OSFR_REF_DATA_BLOCK_VARIABLE_NAME;
CHAR16    gACPIOSFRMfgStringVariableName[] = ACPI_OSFR_MFG_STRING_VARIABLE_NAME;

EFI_CPU_IO_PROTOCOL                    *mCpuIo;

#pragma optimize("", off)

BOOLEAN                   mFirstNotify;
EFI_PLATFORM_INFO_HOB     *mPlatformInfo;
SETUP_DATA      	mSystemConfiguration;
UINT8                     mSLP20DataPresenceCheckDone = FALSE;
UINT8                     mSLP20DataPresentAndValid = FALSE;
UINT64                    mSLP20OemIdValue;
UINT64                    mSLP20OemTableIdValue;
UINT8                     *mSLP20PublicKeyBuffer = NULL, *mSLP20MarkerBuffer = NULL;
SETUP_DATA      	mSystemConfig;
UINTN       mUpdatedSsdtTableNumber= 0;

UINT8 mSmbusRsvdAddresses[] = PLATFORM_SMBUS_RSVD_ADDRESSES;
UINT8 mNumberSmbusAddress = sizeof(mSmbusRsvdAddresses) / sizeof(mSmbusRsvdAddresses[0]);

EFI_STATUS
MeasureAcpiTable(
    EFI_ACPI_COMMON_HEADER          *Table
);

UINT8
ReadCmosBank1Byte(
    IN  EFI_CPU_IO_PROTOCOL             *CpuIo,
    IN  UINT8                           Index
);

VOID
WriteCmosBank1Byte(
    IN  EFI_CPU_IO_PROTOCOL             *CpuIo,
    IN  UINT8                           Index,
    IN  UINT8                           Data
);

VOID CreateFpdtTables(VOID);

EFI_STATUS
LocateSupportProtocol(
    IN   EFI_GUID       *Protocol,
    OUT  VOID           **Instance,
    IN   UINT32         Type
)
/*++

Routine Description:

  Locate the first instance of a protocol.  If the protocol requested is an
  FV protocol, then it will return the first FV that contains the ACPI table
  storage file.

Arguments:

  Protocol  -  The protocol to find.
  Instance  -  Return pointer to the first instance of the protocol.
  Type      -  The type of protocol to locate.

Returns:

  EFI_SUCCESS           -  The function completed successfully.
  EFI_NOT_FOUND         -  The protocol could not be located.
  EFI_OUT_OF_RESOURCES  -  There are not enough resources to find the protocol.

--*/
{
    EFI_STATUS              Status;
    EFI_HANDLE              *HandleBuffer;
    UINTN                   NumberOfHandles;
    EFI_FV_FILETYPE         FileType;
    UINT32                  FvStatus;
    EFI_FV_FILE_ATTRIBUTES  Attributes;
    UINTN                   Size;
    UINTN                   Index;

    FvStatus = 0;
    //
    // Locate protocol.
    //
    Status = gBS->LocateHandleBuffer(
                 ByProtocol,
                 Protocol,
                 NULL,
                 &NumberOfHandles,
                 &HandleBuffer
             );
    if(EFI_ERROR(Status)) {
        //
        // Defined errors at this time are not found and out of resources.
        //
        return Status;
    }
    //
    // Looking for FV with ACPI storage file
    //
    for(Index = 0; Index < NumberOfHandles; Index++) {
        //
        // Get the protocol on this handle
        // This should not fail because of LocateHandleBuffer
        //
        Status = gBS->HandleProtocol(
                     HandleBuffer[Index],
                     Protocol,
                     Instance
                 );
        ASSERT(!EFI_ERROR(Status));

        if(!Type) {
            //
            // Not looking for the FV protocol, so find the first instance of the
            // protocol.  There should not be any errors because our handle buffer
            // should always contain at least one or LocateHandleBuffer would have
            // returned not found.
            //
            break;
        }
        //
        // See if it has the ACPI storage file
        //
        Status = ((EFI_FIRMWARE_VOLUME_PROTOCOL *)(*Instance))->ReadFile(
                     *Instance,
                     &gEfiAcpiTableStorageGuid,
                     NULL,
                     &Size,
                     &FileType,
                     &Attributes,
                     &FvStatus
                 );

        //
        // If we found it, then we are done
        //
        if(!EFI_ERROR(Status)) {
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
    gBS->FreePool(HandleBuffer);

    return Status;
}

EFI_STATUS
PlatformUpdateTables(
    IN OUT EFI_ACPI_COMMON_HEADER  *Table
)
/*++

Routine Description:

  This function will update any runtime platform specific information.
  This currently includes:
    Setting OEM table values, ID, table ID, creator ID and creator revision.
    Enabling the proper processor entries in the APIC tables.

Arguments:

  Table  -  The table to update

Returns:

  EFI_SUCCESS  -  The function completed successfully.

--*/
{
    EFI_ACPI_DESCRIPTION_HEADER                                 *TableHeader;
    UINT8                                                       *CurrPtr;
    UINT8                                                       *EndPtr;
    ACPI_APIC_STRUCTURE_PTR                                     *ApicPtr;
    UINT8                                                       CurrProcessor;
    EFI_STATUS                                                  Status;
    EFI_MP_SERVICES_PROTOCOL                                    *MpService;
    UINTN                                                       MaximumNumberOfCPUs;
    UINTN                                                       NumberOfEnabledCPUs;
    UINTN                                                       BufferSize;
//    ACPI_APIC_STRUCTURE_PTR                                     *ProcessorLocalApicEntry;
    UINTN                                                       BspIndex;
    EFI_ACPI_1_0_ASF_DESCRIPTION_TABLE                          *AsfEntry;
    EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER            *HpetTbl;
    UINT64                                                      OemIdValue;
    UINT8                                                       Index;
    EFI_CPU_IO_PROTOCOL                                         *CpuIo;
    UINT8                                                       Data;
    EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE                   *pFACP;
    EFI_PLATFORM_CPU_PROTOCOL                                   *PlatformCpuPolicy;
    EFI_ACPI_SLIC_TABLE                                         *SlicTable;
    UINT8                                                       *SLP20PKSignBuffer = NULL;
    UINT32                                                      SLP20Magic;
    EFI_GUID                                                    SLP20MagicGuid = {0x41282EF2L, 0x9B5A, 0x4EB7, 0x95, 0xD8, 0xD9, 0xCD, 0x7B, 0xDC, 0xE3, 0x67};
    EFI_DPSD_RSA1024_AND_SHA256_SIGNATURE_VERIFICATION_PROTOCOL *DpsdRSA1024AndSHA256SignatureVerification;
    EFI_ACPI_OSFR_TABLE                                         *pOsfrTable;
    EFI_ACPI_OSFR_OCUR_OBJECT                                   *pOcurObject;
    EFI_ACPI_OSFR_OCUR_OBJECT                                   OcurObject = {{0xB46F133D, 0x235F, 0x4634, 0x9F, 0x03, 0xB1, 0xC0, 0x1C, 0x54, 0x78, 0x5B}, 0, 0, 0, 0, 0};
    CHAR16                                                      *OcurMfgStringBuffer = NULL, *OcurModelStringBuffer = NULL;
    UINT8                                                       *OcurRefDataBlockBuffer = NULL;
    UINTN                                                       OcurMfgStringBufferSize, OcurModelStringBufferSize, OcurRefDataBlockBufferSize;
//#if defined (IDCC2_SUPPORTED) && IDCC2_SUPPORTED
//    EFI_ACPI_ASPT_TABLE                                         *pSpttTable;
//#endif
    UINT16                                                      NumberOfHpets;
    UINT16                                                      HpetCapIdValue;
    UINT32                                                      HpetBlockID;
//    UINTN                                                       LocalApicCounter;
    EFI_PROCESSOR_INFORMATION                                   ProcessorInfoBuffer;


    CurrPtr                 = NULL;
    EndPtr                  = NULL;
    ApicPtr                 = NULL;
//    LocalApicCounter        = 0;
    CurrProcessor           = 0;
//    ProcessorLocalApicEntry = NULL;

    // Check for presence of SLP 2.0 data in proper variables
    if(mSLP20DataPresenceCheckDone == FALSE) {
        mSLP20DataPresenceCheckDone = TRUE;
        // Prep handshaking variable for programming SLP 2.0 data
        BufferSize = sizeof(UINT32);
        Status = gRT->GetVariable(L"SLP20Magic", &SLP20MagicGuid, NULL, &BufferSize, &SLP20Magic);
        if(EFI_ERROR(Status)) {
            // SLP 2.0 magic variable not present - set it to current value
            SLP20Magic = SLP20_MAGIC_NUMBER;
            Status = gRT->SetVariable(L"SLP20Magic", &SLP20MagicGuid, EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS
                                      | EFI_VARIABLE_NON_VOLATILE, BufferSize, &SLP20Magic);
        }
        BufferSize = sizeof(EFI_ACPI_SLIC_OEM_PUBLIC_KEY);
        mSLP20PublicKeyBuffer = AllocatePool(BufferSize);
        Status = gRT->GetVariable(L"SLP20OEMPublicKey", &gSLP20OEMPublicKeyVariableGuid, NULL, &BufferSize, mSLP20PublicKeyBuffer);
        if(!EFI_ERROR(Status)) {
            BufferSize = sizeof(EFI_ACPI_SLIC_SLP_MARKER);
            mSLP20MarkerBuffer = AllocatePool(BufferSize);
            Status = gRT->GetVariable(L"SLP20Marker", &gSLP20MarkerVariableGuid, NULL, &BufferSize, mSLP20MarkerBuffer);
            if(!EFI_ERROR(Status)) {
                BufferSize = sizeof(EFI_ACPI_SLIC_SIGNED_OEM_PUBLIC_KEY);
                SLP20PKSignBuffer = AllocatePool(BufferSize);
                Status = gRT->GetVariable(L"SLP20EncryptedOEMPublicKey", &gSLP20EncryptedOEMPublicKeyVariableGuid, NULL, &BufferSize, SLP20PKSignBuffer);
                if(!EFI_ERROR(Status)) {
                    // All SLP 2.0 data is here.  Let's see if the signature has already been verified
                    if((*(UINT32 *)SLP20PKSignBuffer != SLP20_VERIFIED_INDICATOR)) {
                        Status = gBS->LocateProtocol(&gEfiDpsdRSA1024AndSHA256SignatureVerificationProtocolGuid, NULL, &DpsdRSA1024AndSHA256SignatureVerification);
                        if(!EFI_ERROR(Status)) {
                            Status = DpsdRSA1024AndSHA256SignatureVerification->VerifySignature(mSLP20PublicKeyBuffer, sizeof(EFI_ACPI_SLIC_OEM_PUBLIC_KEY), SLP20PKSignBuffer);
                            if(!EFI_ERROR(Status)) {
                                // Signature has been verified
                                mSLP20DataPresentAndValid = TRUE;
                                CopyMem(&mSLP20OemIdValue, ((EFI_ACPI_SLIC_SLP_MARKER *) mSLP20MarkerBuffer)->sOEMID, 6);
                                mSLP20OemTableIdValue = ((EFI_ACPI_SLIC_SLP_MARKER *) mSLP20MarkerBuffer)->sOEMTABLEID;
                                BufferSize = sizeof(UINT32);
                                SLP20Magic = SLP20_VERIFIED_INDICATOR;
                                // Update variable to indicate that signature verification has already succeeded
                                gRT->SetVariable(L"SLP20EncryptedOEMPublicKey#!rtUY9o", &gSLP20EncryptedOEMPublicKeyVariableGuid, EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS
                                                 | EFI_VARIABLE_NON_VOLATILE, BufferSize, &SLP20Magic);
                            }
                        }
                    }
                    // Signature has been previously verified
                    else {
                        mSLP20DataPresentAndValid = TRUE;
                        CopyMem(&mSLP20OemIdValue, ((EFI_ACPI_SLIC_SLP_MARKER *) mSLP20MarkerBuffer)->sOEMID, 6);
                        mSLP20OemTableIdValue = ((EFI_ACPI_SLIC_SLP_MARKER *) mSLP20MarkerBuffer)->sOEMTABLEID;
                        BufferSize = sizeof(UINT32);
                        SLP20Magic = SLP20_VERIFIED_INDICATOR;
                        // Update variable to indicate that signature verification has already succeeded
                        gRT->SetVariable(L"SLP20EncryptedOEMPublicKey#!rtUY9o", &gSLP20EncryptedOEMPublicKeyVariableGuid, EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS
                                         | EFI_VARIABLE_NON_VOLATILE, BufferSize, &SLP20Magic);
                    }
                }
            }
        }

        // Free all allocated memory
        if(SLP20PKSignBuffer != NULL) {
            gBS->FreePool(SLP20PKSignBuffer);
        }
    }

    if(Table->Signature != EFI_ACPI_1_0_FIRMWARE_ACPI_CONTROL_STRUCTURE_SIGNATURE) {
        TableHeader = (EFI_ACPI_DESCRIPTION_HEADER *) Table;
        //
        // Update the OEMID
        //
        if(mSLP20DataPresentAndValid) {
            // Override ACPI OEMID from valid SLP 2.0 data
            OemIdValue = mSLP20OemIdValue;
            *(UINT32 *)(TableHeader->OemId)     = (UINT32)OemIdValue;
            *(UINT16 *)(TableHeader->OemId + 4) = *(UINT16*)(((UINT8 *)&OemIdValue) + 4);
        } else {
            for(Index = 0; Index < 6; Index ++) {
                TableHeader->OemId[Index] = mPlatformInfo->AcpiOemId[Index];
            }
        }

        if((Table->Signature != EFI_ACPI_2_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) ||
                ((Table->Signature == EFI_ACPI_2_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) &&
                 (mUpdatedSsdtTableNumber == 0))) {
            //
            // According to ACPI Spec, each SSDT table should have unique OemTableId,
            // we only update the first SSDT table OemTableId
            //
            if(Table->Signature == EFI_ACPI_2_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
                mUpdatedSsdtTableNumber ++;
            }
            //
            // Update the OEM Table ID
            //
            for(Index = 0; Index < 8; Index ++) {
                *(UINT8 *)(UINT32)(TableHeader->OemTableId + Index) = mPlatformInfo->AcpiOemTableId[Index];
            }
//      BoardIdVarSize = sizeof (EFI_BOARD_FEATURES);
//      Status = gRT->GetVariable (BOARD_FEATURES_NAME,
//                                 &gEfiBoardFeaturesGuid,
//                                 NULL,
//                                 &BoardIdVarSize,
//                                 &BoardIdVar);
//
//      if (!EFI_ERROR (Status)) {
//        BoardIdVar &= B_BOARD_FEATURES_VERB_TABLE_MASK;
//        if (BoardIdVar == B_BOARD_FEATURES_VERB_TABLE2){
//          TableHeader->OemTableId = EFI_ACPI_OEM_TABLE_ID_KT;
//        }
//      }
        }
        // Override ACPI OEMTABLEID from valid SLP 2.0 data
        if(mSLP20DataPresentAndValid && Table->Signature != EFI_ACPI_2_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE) {
            TableHeader->OemTableId = mSLP20OemTableIdValue;
        }

        //
        // Update the OEM Table ID
        //
//    TableHeader->OemRevision = PRODUCT_BUILD_NUMBER_AS_INT;
        TableHeader->OemRevision = EFI_ACPI_OEM_REVISION;
        //
        // Update the creator ID
        //
        TableHeader->CreatorId = EFI_ACPI_CREATOR_ID;

        //
        // Update the creator revision
        //
        TableHeader->CreatorRevision = EFI_ACPI_CREATOR_REVISION;
    }
    //
    // Complete this function
    //
    //
    // Locate the MP services protocol
    //
    //
    // Find the MP Protocol. This is an MP platform, so MP protocol must be
    // there.
    //
    Status = gBS->LocateProtocol(
                 &gEfiMpServiceProtocolGuid,
                 NULL,
                 &MpService
             );
    if(EFI_ERROR(Status)) {
        return Status;
    }
    //
    // Determine the number of processors
    //
    MpService->GetNumberOfProcessors(
        MpService,
        &MaximumNumberOfCPUs,
        &NumberOfEnabledCPUs
    );

    ASSERT(MaximumNumberOfCPUs <= MAX_CPU_NUM && NumberOfEnabledCPUs >= 1);

    //
    // Assign a invalid intial value for update
    //
    //
    // Update the processors in the APIC table
    //
    switch(Table->Signature) {
    case EFI_ACPI_1_0_ASF_DESCRIPTION_TABLE_SIGNATURE:
        //
        // Update the table if ASF is enabled. Otherwise, return error so caller will not install
        //
        AsfEntry = (EFI_ACPI_1_0_ASF_DESCRIPTION_TABLE *) Table;
        for(Index = 0; (Index < mNumberSmbusAddress) && (Index < ASF_ADDR_DEVICE_ARRAY_LENGTH); Index++) {
            AsfEntry->AsfAddr.FixedSmbusAddresses[Index] = mSmbusRsvdAddresses[Index];
        }
        break;

    case EFI_ACPI_3_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE:

        Status = MpService->WhoAmI(
                     MpService,
                     &BspIndex
                 );

        CurrPtr = (UINT8 *) &((EFI_ACPI_DESCRIPTION_HEADER *) Table)[1];
        CurrPtr = CurrPtr + 8;
        //
        // Size of Local APIC Address & Flag
        //
        EndPtr  = (UINT8 *) Table;
        EndPtr  = EndPtr + Table->Length;
        while(CurrPtr < EndPtr) {
            ApicPtr = (ACPI_APIC_STRUCTURE_PTR *) CurrPtr;
            switch(ApicPtr->AcpiApicCommon.Type) {
            case EFI_ACPI_3_0_PROCESSOR_LOCAL_APIC:
                //
                // ESS override
                // Fix for Ordering of MADT to be maintained as it is in MADT table.
                //
                // Update processor enabled or disabled and keep the local APIC
                // order in MADT intact
                //
                // Sanity check to make sure proc-id is not arbitrary
                //
                DEBUG((EFI_D_ERROR, "ApicPtr->AcpiLocalApic.AcpiProcessorId = %x, MaximumNumberOfCPUs = %x\n", ApicPtr->AcpiLocalApic.AcpiProcessorId, MaximumNumberOfCPUs));
                //ASSERT (ApicPtr->AcpiLocalApic.AcpiProcessorId <= MaximumNumberOfCPUs);
                if(ApicPtr->AcpiLocalApic.AcpiProcessorId > MaximumNumberOfCPUs) {
                    ApicPtr->AcpiLocalApic.AcpiProcessorId = (UINT8)MaximumNumberOfCPUs;
                }

                BufferSize                    = 0;
                ApicPtr->AcpiLocalApic.Flags  = 0;

                for(CurrProcessor = 0; CurrProcessor < MaximumNumberOfCPUs; CurrProcessor++) {
                    Status = MpService->GetProcessorInfo(
                                 MpService,
                                 CurrProcessor,
                                 &ProcessorInfoBuffer
                             );

                    if(Status == EFI_SUCCESS && ProcessorInfoBuffer.ProcessorId == ApicPtr->AcpiLocalApic.ApicId) {
                        //
                        // Check to see whether or not a processor (or thread) is enabled
                        //
                        if(BspIndex == CurrProcessor || ((ProcessorInfoBuffer.StatusFlag & PROCESSOR_ENABLED_BIT) != 0)) {
                            //
                            // Go on and check if Hyper Threading is enabled. If HT not enabled
                            // hide this thread from OS by not setting the flag to 1.  This is the
                            // software way to disable Hyper Threading.  Basically we just hide it
                            // from the OS.
                            //
                            ApicPtr->AcpiLocalApic.Flags = EFI_ACPI_1_0_LOCAL_APIC_ENABLED;

                            if(!mSystemConfiguration.HTD) {
                                if(ProcessorInfoBuffer.Location.Thread != 0) {
                                    ApicPtr->AcpiLocalApic.Flags = 0;
                                }
                            }

//                  if(mSystemConfiguration.ActiveProcessorCores) {
                            if(mSystemConfiguration.ActiveCoreCount) {
//                    if(ProcessorInfoBuffer.Location.Core >= mSystemConfiguration.ActiveProcessorCores) {
                                if(ProcessorInfoBuffer.Location.Core >= mSystemConfiguration.ActiveCoreCount) {
                                    ApicPtr->AcpiLocalApic.Flags = 0;
                                }
                            }
//(CSP20130125D+)<<

                            AppendCpuMapTableEntry(&(ApicPtr->AcpiLocalApic));
                        }
                        break;
                    }
                }

                //
                // If no APIC-ID match, the cpu may not be populated
                //
                break;

            case EFI_ACPI_3_0_IO_APIC:
                //
                // IO APIC entries can be patched here
                //
                break;
            }

            CurrPtr = CurrPtr + ApicPtr->AcpiApicCommon.Length;
        }
        break;

    case EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE:

        Status = gBS->LocateProtocol(&gEfiPlatformCpuProtocolGuid,
                                     NULL,
                                     (VOID **)&PlatformCpuPolicy);

        if(!EFI_ERROR(Status)) {
            if(PlatformCpuPolicy->CcxEnable==0) {
                pFACP = (EFI_ACPI_3_0_FIXED_ACPI_DESCRIPTION_TABLE *) Table;
                pFACP->Flags &= (UINT32)(~(3<<2));
            }
        }

        break;

    case EFI_ACPI_3_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
        //
        // Patch the memory resource
        //
        PatchDsdtTable((EFI_ACPI_DESCRIPTION_HEADER *) Table);
        break;

    case EFI_ACPI_3_0_SECONDARY_SYSTEM_DESCRIPTION_TABLE_SIGNATURE:
        //
        // Gv3 support
        //
        // TBD: Need re-design based on the ValleyTrail platform.
        //

        break;

    case EFI_ACPI_3_0_HIGH_PRECISION_EVENT_TIMER_TABLE_SIGNATURE:
        //
        // Adjust HPET Table to correct the Base Address
        //
        // Enable HPET always as Hpet.asi always indicates that Hpet is enabled.
        MmioOr8(R_PCH_PCH_HPET + R_PCH_PCH_HPET_GCFG, B_PCH_PCH_HPET_GCFG_EN);

        //
        // Update CMOS to reflect HPET enable/disable for ACPI usage
        //
        Status = gBS->LocateProtocol(&gEfiCpuIoProtocolGuid, NULL, &CpuIo);
        ASSERT_EFI_ERROR(Status);

        Data = CmosReadByte(ACPI_EFI_CMOS_TABLE_FLAG_ADDRESS);
        Data &= ~B_CMOS_HPET_ENABLED;

        Data |= B_CMOS_HPET_ENABLED;

        CmosWriteByte(ACPI_EFI_CMOS_TABLE_FLAG_ADDRESS, Data);



        HpetTbl = (EFI_ACPI_HIGH_PRECISION_EVENT_TIMER_TABLE_HEADER *) Table;
        HpetTbl->BaseAddressLower32Bit.Address = HPET_BASE_ADDRESS;
        HpetTbl->EventTimerBlockId = *((UINT32*)(UINTN)HPET_BASE_ADDRESS);

        HpetCapIdValue = *(UINT16 *)(UINTN)(HPET_BASE_ADDRESS);
        NumberOfHpets = HpetCapIdValue & B_PCH_PCH_HPET_GCID_NT;  // Bits [8:12] contains the number of Hpets
        HpetBlockID = EFI_ACPI_EVENT_TIMER_BLOCK_ID;

        if((NumberOfHpets) && (NumberOfHpets & B_PCH_PCH_HPET_GCID_NT)) {
            HpetBlockID |= (NumberOfHpets);
        }
        HpetTbl->EventTimerBlockId = HpetBlockID;

        break;

    case EFI_ACPI_3_0_PCI_EXPRESS_MEMORY_MAPPED_CONFIGURATION_SPACE_BASE_ADDRESS_DESCRIPTION_TABLE_SIGNATURE:
        //
        // Update MCFG base and end bus number
        //
        ((EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE *) Table)->Segment[0].BaseAddress
            = mPlatformInfo->PciData.PciExpressBase;
        ((EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_BASE_ADDRESS_TABLE *) Table)->Segment[0].EndBusNumber
            = (UINT8)RShiftU64(mPlatformInfo->PciData.PciExpressSize, 20) - 1;
        break;

    case EFI_ACPI_SLIC_TABLE_SIGNATURE:
        if(!mSLP20DataPresentAndValid) {
            return EFI_UNSUPPORTED;
        }
        SlicTable = (EFI_ACPI_SLIC_TABLE *) Table;
        CopyMem(&(SlicTable->OemPublicKey), mSLP20PublicKeyBuffer, sizeof(EFI_ACPI_SLIC_OEM_PUBLIC_KEY));
        CopyMem(&(SlicTable->SlpMarker), mSLP20MarkerBuffer, sizeof(EFI_ACPI_SLIC_SLP_MARKER));
        gBS->FreePool(mSLP20PublicKeyBuffer);
        gBS->FreePool(mSLP20MarkerBuffer);
        break;

    case EFI_ACPI_OSFR_TABLE_SIGNATURE:
        // Get size of OSFR variable
        OcurMfgStringBufferSize = 0;
        Status = gRT->GetVariable(gACPIOSFRMfgStringVariableName, &gACPIOSFRMfgStringVariableGuid, NULL, &OcurMfgStringBufferSize, NULL);
        if(Status != EFI_BUFFER_TOO_SMALL) {
            // Variable must not be present on the system
            return EFI_UNSUPPORTED;
        }
        // Allocate memory for variable data
        OcurMfgStringBuffer = AllocatePool(OcurMfgStringBufferSize);
        Status = gRT->GetVariable(gACPIOSFRMfgStringVariableName, &gACPIOSFRMfgStringVariableGuid, NULL, &OcurMfgStringBufferSize, OcurMfgStringBuffer);
        if(!EFI_ERROR(Status)) {
            OcurModelStringBufferSize = 0;
            Status = gRT->GetVariable(gACPIOSFRModelStringVariableName, &gACPIOSFRModelStringVariableGuid, NULL, &OcurModelStringBufferSize, NULL);
            if(Status != EFI_BUFFER_TOO_SMALL) {
                // Variable must not be present on the system
                return EFI_UNSUPPORTED;
            }
            // Allocate memory for variable data
            OcurModelStringBuffer = AllocatePool(OcurModelStringBufferSize);
            Status = gRT->GetVariable(gACPIOSFRModelStringVariableName, &gACPIOSFRModelStringVariableGuid, NULL, &OcurModelStringBufferSize, OcurModelStringBuffer);
            if(!EFI_ERROR(Status)) {
                OcurRefDataBlockBufferSize = 0;
                Status = gRT->GetVariable(gACPIOSFRRefDataBlockVariableName, &gACPIOSFRRefDataBlockVariableGuid, NULL, &OcurRefDataBlockBufferSize, NULL);
                if(Status == EFI_BUFFER_TOO_SMALL) {
                    // Allocate memory for variable data
                    OcurRefDataBlockBuffer = AllocatePool(OcurRefDataBlockBufferSize);
                    gRT->GetVariable(gACPIOSFRRefDataBlockVariableName, &gACPIOSFRRefDataBlockVariableGuid, NULL, &OcurRefDataBlockBufferSize, OcurRefDataBlockBuffer);
                }
                pOsfrTable = (EFI_ACPI_OSFR_TABLE *) Table;
                // Currently only one object is defined: OCUR_OSFR_TABLE
                pOsfrTable->ObjectCount = 1;
                // Initialize table length to fixed portion of the ACPI OSFR table
                pOsfrTable->Header.Length = sizeof(EFI_ACPI_OSFR_TABLE_FIXED_PORTION);
                *(UINT32 *)((UINTN) pOsfrTable + sizeof(EFI_ACPI_OSFR_TABLE_FIXED_PORTION)) = \
                        (UINT32)(sizeof(EFI_ACPI_OSFR_TABLE_FIXED_PORTION) + sizeof(UINT32));
                pOcurObject = (EFI_ACPI_OSFR_OCUR_OBJECT *)((UINTN) pOsfrTable + sizeof(EFI_ACPI_OSFR_TABLE_FIXED_PORTION) + \
                              sizeof(UINT32));
                CopyMem(pOcurObject, &OcurObject, sizeof(EFI_ACPI_OSFR_OCUR_OBJECT));
                pOcurObject->ManufacturerNameStringOffset = (UINT32)((UINTN) pOcurObject - (UINTN) pOsfrTable + sizeof(EFI_ACPI_OSFR_OCUR_OBJECT));
                pOcurObject->ModelNameStringOffset = (UINT32)((UINTN) pOcurObject - (UINTN) pOsfrTable + sizeof(EFI_ACPI_OSFR_OCUR_OBJECT) + OcurMfgStringBufferSize);
                if(OcurRefDataBlockBufferSize > 0) {
                    pOcurObject->MicrosoftReferenceOffset = (UINT32)((UINTN) pOcurObject - (UINTN) pOsfrTable + sizeof(EFI_ACPI_OSFR_OCUR_OBJECT) + OcurMfgStringBufferSize + OcurModelStringBufferSize);
                }
                CopyMem((UINTN *)((UINTN) pOcurObject + sizeof(EFI_ACPI_OSFR_OCUR_OBJECT)), OcurMfgStringBuffer, OcurMfgStringBufferSize);
                CopyMem((UINTN *)((UINTN) pOcurObject + sizeof(EFI_ACPI_OSFR_OCUR_OBJECT) + OcurMfgStringBufferSize), \
                        OcurModelStringBuffer, OcurModelStringBufferSize);
                if(OcurRefDataBlockBufferSize > 0) {
                    CopyMem((UINTN *)((UINTN) pOcurObject + sizeof(EFI_ACPI_OSFR_OCUR_OBJECT) + OcurMfgStringBufferSize + OcurModelStringBufferSize), \
                            OcurRefDataBlockBuffer, OcurRefDataBlockBufferSize);
                }
                pOsfrTable->Header.Length += (UINT32)(OcurMfgStringBufferSize + OcurModelStringBufferSize + OcurRefDataBlockBufferSize);
                pOsfrTable->Header.Length += sizeof(EFI_ACPI_OSFR_OCUR_OBJECT) + sizeof(UINT32);
            }
        }
        gBS->FreePool(OcurMfgStringBuffer);
        gBS->FreePool(OcurModelStringBuffer);
        gBS->FreePool(OcurRefDataBlockBuffer);
        break;

    default:
        break;
    }
    //
    //
    // Update the hardware signature in the FACS structure
    //
    //
    // Locate the SPCR table and update based on current settings.
    // The user may change CR settings via setup or other methods.
    // The SPCR table must match.
    //
    return EFI_SUCCESS;
}

STATIC
VOID
EFIAPI
OnReadyToBoot(
    IN      EFI_EVENT                 Event,
    IN      VOID                      *Context
)
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Event   - GC_TODO: add argument description
  Context - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
    EFI_STATUS                  Status;
    EFI_ACPI_TABLE_VERSION      TableVersion;
    EFI_ACPI_SUPPORT_PROTOCOL   *AcpiSupport;
    EFI_ACPI_S3_SAVE_PROTOCOL   *AcpiS3Save;
    SETUP_DATA        SetupVarBuffer;
    UINTN                       VariableSize;
    EFI_PLATFORM_CPU_INFO       *PlatformCpuInfoPtr = NULL;
    EFI_PLATFORM_CPU_INFO       PlatformCpuInfo;
//  UINT8                       *TPM12BaseAddress = (UINT8 *)(UINTN) 0xFED40000;
    EFI_PEI_HOB_POINTERS          GuidHob;

    if(mFirstNotify) {
        return;
    }

    mFirstNotify = TRUE;

    //
    // To avoid compiler warning of "C4701: potentially uninitialized local variable 'PlatformCpuInfo' used"
    //
    PlatformCpuInfo.CpuVersion.FullCpuId = 0;
    //
    // Get Platform CPU Info HOB
    //
    PlatformCpuInfoPtr = NULL;
    ZeroMem(&PlatformCpuInfo, sizeof(EFI_PLATFORM_CPU_INFO));
    VariableSize = sizeof(EFI_PLATFORM_CPU_INFO);
    Status = gRT->GetVariable(
                 EfiPlatformCpuInfoVariable,
                 &gEfiVlv2VariableGuid,
                 NULL,
                 &VariableSize,
                 PlatformCpuInfoPtr
             );
    if(EFI_ERROR(Status)) {
        GuidHob.Raw = GetHobList();
        if(GuidHob.Raw != NULL) {
            if((GuidHob.Raw = GetNextGuidHob(&gEfiPlatformCpuInfoGuid, GuidHob.Raw)) != NULL) {
                PlatformCpuInfoPtr = GET_GUID_HOB_DATA(GuidHob.Guid);
            }
        }
    }

    if((PlatformCpuInfoPtr != NULL)) {
        CopyMem(&PlatformCpuInfo, PlatformCpuInfoPtr, sizeof(EFI_PLATFORM_CPU_INFO));
    }

    //
    // Update the ACPI parameter blocks finally
    //
    VariableSize = sizeof(SETUP_DATA);
    Status = gRT->GetVariable(
                 L"Setup",
                 &gEfiSetupVariableGuid,
                 NULL,
                 &VariableSize,
                 &SetupVarBuffer
             );
    ASSERT_EFI_ERROR(Status);

    //
    // Find the AcpiSupport protocol
    //
    Status = LocateSupportProtocol(&gEfiAcpiSupportProtocolGuid, &AcpiSupport, 0);
    ASSERT_EFI_ERROR(Status);

    TableVersion = EFI_ACPI_TABLE_VERSION_2_0;

    //
    // Publish ACPI 1.0 or 2.0 Tables
    //
    Status = AcpiSupport->PublishTables(
                 AcpiSupport,
                 TableVersion
             );
    ASSERT_EFI_ERROR(Status);

    //
    // S3 script save
    //
    Status = gBS->LocateProtocol(&gEfiAcpiS3SaveProtocolGuid, NULL, &AcpiS3Save);
    if(!EFI_ERROR(Status)) {
        AcpiS3Save->S3Save(AcpiS3Save, NULL);
    }

}


EFI_STATUS
EFIAPI
AcpiPlatformEntryPoint(
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
/*++

Routine Description:

  Entry point for Acpi platform driver.

Arguments:

  ImageHandle  -  A handle for the image that is initializing this driver.
  SystemTable  -  A pointer to the EFI system table.

Returns:

  EFI_SUCCESS           -  Driver initialized successfully.
  EFI_LOAD_ERROR        -  Failed to Initialize or has been loaded.
  EFI_OUT_OF_RESOURCES  -  Could not allocate needed resources.

--*/
{
    EFI_STATUS                    Status;
    EFI_STATUS                    AcpiStatus;
    EFI_ACPI_SUPPORT_PROTOCOL     *AcpiSupport;
    EFI_FIRMWARE_VOLUME_PROTOCOL  *FwVol;
    INTN                          Instance;
    EFI_ACPI_COMMON_HEADER        *CurrentTable;
    UINTN                         TableHandle;
    UINT32                        FvStatus;
    UINT32                        Size;
    EFI_EVENT                     Event;
    EFI_ACPI_TABLE_VERSION        TableVersion;
    UINTN                         VarSize;
    UINTN                         SysCfgSize;
    EFI_HANDLE                    Handle;
    EFI_PS2_POLICY_PROTOCOL       *Ps2Policy;
    EFI_PEI_HOB_POINTERS          GuidHob;
    UINT8                         PortData;
    SB_SETUP_DATA                     PchPolicyData;
    NB_SETUP_DATA                     VlvPolicyData;
    EFI_MP_SERVICES_PROTOCOL      *MpService;
    UINTN                         MaximumNumberOfCPUs;
    UINTN                         NumberOfEnabledCPUs;
    UINT8 			RevId;
    UINT32			Data32;
    
    mFirstNotify      = FALSE;

    TableVersion      = EFI_ACPI_TABLE_VERSION_2_0;
    Instance          = 0;
    CurrentTable      = NULL;
    TableHandle       = 0;

    //
    // Update HOB variable for PCI resource information
    // Get the HOB list.  If it is not present, then ASSERT.
    //
    GuidHob.Raw = GetHobList();
    if(GuidHob.Raw != NULL) {
        if((GuidHob.Raw = GetNextGuidHob(&gEfiPlatformInfoGuid, GuidHob.Raw)) != NULL) {
            mPlatformInfo = GET_GUID_HOB_DATA(GuidHob.Guid);
        }
    }

    //
    // Search for the Memory Configuration GUID HOB.  If it is not present, then
    // there's nothing we can do. It may not exist on the update path.
    //

    VarSize = sizeof(SETUP_DATA);
    Status = gRT->GetVariable(
                 L"Setup",
                 &gEfiSetupVariableGuid,
                 NULL,
                 &VarSize,
                 &mSystemConfiguration
             );
    ASSERT_EFI_ERROR(Status);

    // Find the AcpiSupport protocol
    //
    Status = LocateSupportProtocol(&gEfiAcpiSupportProtocolGuid, &AcpiSupport, 0);
    ASSERT_EFI_ERROR(Status);

    //
    // Locate the firmware volume protocol
    //
    Status = LocateSupportProtocol(&gEfiFirmwareVolumeProtocolGuid, &FwVol, 1);
    ASSERT_EFI_ERROR(Status);

    //
    // Read the current system configuration variable store.
    //
    SysCfgSize = sizeof(SETUP_DATA);
    gRT->GetVariable(L"Setup",
                     &gEfiSetupVariableGuid,
                     NULL,
                     &SysCfgSize,
                     &mSystemConfig
                    );

    Instance  = 0;

    GetSbSetupData((VOID*)gRT, &PchPolicyData, FALSE);
    GetNbSetupData((VOID*)gRT, &VlvPolicyData, FALSE);
    //
    // TBD: Need re-design based on the ValleyTrail platform.
    //
    Status = gBS->LocateProtocol(
                 &gEfiMpServiceProtocolGuid,
                 NULL,
                 &MpService
             );
    if(EFI_ERROR(Status)) {
        return Status;
    }
    //
    // Determine the number of processors
    //
    MpService->GetNumberOfProcessors(
        MpService,
        &MaximumNumberOfCPUs,
        &NumberOfEnabledCPUs
    );

    //
    // Allocate and initialize the NVS area for SMM and ASL communication.

    Status = gBS->AllocatePool(EfiACPIMemoryNVS, sizeof(EFI_GLOBAL_NVS_AREA), &mGlobalNvsArea.Area);
    ASSERT_EFI_ERROR(Status);
    gBS->SetMem(mGlobalNvsArea.Area, sizeof(EFI_GLOBAL_NVS_AREA), 0);

    //
    // Update global NVS area for ASL and SMM init code to use
    //
    mGlobalNvsArea.Area->ApicEnable                  = 1;
    mGlobalNvsArea.Area->EmaEnable                  = 0;

    mGlobalNvsArea.Area->NumberOfBatteries         = 1;
    mGlobalNvsArea.Area->BatteryCapacity0            = 100;
    mGlobalNvsArea.Area->BatteryStatus0             		= 84;
    mGlobalNvsArea.Area->OnboardCom                 	= 1;
    mGlobalNvsArea.Area->IdeMode                    			= 0;
    mGlobalNvsArea.Area->PowerState                 			= 1;

    mGlobalNvsArea.Area->LogicalProcessorCount    = (UINT8)NumberOfEnabledCPUs;
    mGlobalNvsArea.Area->PassiveThermalTripPoint  = VlvPolicyData.PassiveThermalTripPoint;
    mGlobalNvsArea.Area->PassiveTc1Value          = VlvPolicyData.PassiveTc1Value;
    mGlobalNvsArea.Area->PassiveTc2Value          = VlvPolicyData.PassiveTc2Value;
    mGlobalNvsArea.Area->PassiveTspValue          = VlvPolicyData.PassiveTspValue;
    mGlobalNvsArea.Area->CriticalThermalTripPoint = VlvPolicyData.CriticalThermalTripPoint;

    mGlobalNvsArea.Area->IgdPanelType             = VlvPolicyData.IgdFlatPanel;
    mGlobalNvsArea.Area->IgdPanelScaling         	= VlvPolicyData.PanelScaling;
    mGlobalNvsArea.Area->IgdSciSmiMode         	  = VlvPolicyData.IgdSciSmiMode;
    mGlobalNvsArea.Area->IgdTvFormat              	= VlvPolicyData.IgdTvFormat;
    mGlobalNvsArea.Area->IgdTvMinor               	= VlvPolicyData.IgdTvMinor;
    mGlobalNvsArea.Area->IgdSscConfig             	= VlvPolicyData.IgdSscConfig;
    mGlobalNvsArea.Area->IgdBiaConfig             	= VlvPolicyData.IgdBiaConfig;
    mGlobalNvsArea.Area->IgdBlcConfig             	= VlvPolicyData.IgdBlcConfig;
    mGlobalNvsArea.Area->IgdDvmtMemSize      	    = VlvPolicyData.IgdDvmtMemSize;
    mGlobalNvsArea.Area->AlsEnable                  = VlvPolicyData.AlsEnable;

    mGlobalNvsArea.Area->BacklightControlSupport  = VlvPolicyData.BacklightControlSupport;
    mGlobalNvsArea.Area->BrightnessPercentage     = VlvPolicyData.BrightnessPercentage;
    mGlobalNvsArea.Area->IgdState = VlvPolicyData.IgdState;
    mGlobalNvsArea.Area->LidState = VlvPolicyData.LidStatus;

    mGlobalNvsArea.Area->DeviceId1 = VlvPolicyData.DeviceId1;
    mGlobalNvsArea.Area->DeviceId2 = VlvPolicyData.DeviceId2;
    mGlobalNvsArea.Area->DeviceId3 = VlvPolicyData.DeviceId3;
    mGlobalNvsArea.Area->DeviceId4 = VlvPolicyData.DeviceId4;
    mGlobalNvsArea.Area->DeviceId5 = VlvPolicyData.DeviceId5;
    mGlobalNvsArea.Area->NumberOfValidDeviceId = VlvPolicyData.NumberOfValidDeviceId;
    mGlobalNvsArea.Area->CurrentDeviceList = VlvPolicyData.CurrentDeviceList;
    mGlobalNvsArea.Area->PreviousDeviceList = VlvPolicyData.PreviousDeviceList;

    //
    // DPTF related
    mGlobalNvsArea.Area->DptfEnable                               = VlvPolicyData.EnableDptf;
    mGlobalNvsArea.Area->DptfSysThermal0                          = VlvPolicyData.DptfSysThermal0;
    mGlobalNvsArea.Area->DptfSysThermal1                          = VlvPolicyData.DptfSysThermal1;
    mGlobalNvsArea.Area->DptfSysThermal2                          = VlvPolicyData.DptfSysThermal2;
    mGlobalNvsArea.Area->DptfSysThermal3                          = VlvPolicyData.DptfSysThermal3;
    mGlobalNvsArea.Area->DptfSysThermal4                          = VlvPolicyData.DptfSysThermal4;
    mGlobalNvsArea.Area->DptfCharger                              = VlvPolicyData.DptfChargerDevice;
    mGlobalNvsArea.Area->DptfDisplayDevice                        = VlvPolicyData.DptfDisplayDevice;
    mGlobalNvsArea.Area->DptfSocDevice                            = VlvPolicyData.DptfSocDevice;
    mGlobalNvsArea.Area->DptfProcessor                            = VlvPolicyData.DptfProcessor;
    mGlobalNvsArea.Area->DptfProcCriticalTemperature        = VlvPolicyData.CriticalThermalTripPoint;
    mGlobalNvsArea.Area->DptfProcPassiveTemperature         = VlvPolicyData.PassiveThermalTripPoint;
    mGlobalNvsArea.Area->DptfGenericCriticalTemperature0    = VlvPolicyData.GenericCriticalTemp0;
    mGlobalNvsArea.Area->DptfGenericPassiveTemperature0     = VlvPolicyData.GenericPassiveTemp0;
    mGlobalNvsArea.Area->DptfGenericCriticalTemperature1    = VlvPolicyData.GenericCriticalTemp1;
    mGlobalNvsArea.Area->DptfGenericPassiveTemperature1     = VlvPolicyData.GenericPassiveTemp1;
    mGlobalNvsArea.Area->DptfGenericCriticalTemperature2    = VlvPolicyData.GenericCriticalTemp2;
    mGlobalNvsArea.Area->DptfGenericPassiveTemperature2     = VlvPolicyData.GenericPassiveTemp2;
    mGlobalNvsArea.Area->DptfGenericCriticalTemperature3    = VlvPolicyData.GenericCriticalTemp3;
    mGlobalNvsArea.Area->DptfGenericPassiveTemperature3     = VlvPolicyData.GenericPassiveTemp3;
    mGlobalNvsArea.Area->DptfGenericCriticalTemperature4    = VlvPolicyData.GenericCriticalTemp4;
    mGlobalNvsArea.Area->DptfGenericPassiveTemperature4     = VlvPolicyData.GenericPassiveTemp4;
    mGlobalNvsArea.Area->CLpmSetting                              	= VlvPolicyData.Clpm;
    mGlobalNvsArea.Area->DptfSuperDbg                            	= VlvPolicyData.SuperDebug;
    mGlobalNvsArea.Area->LPOEnable                                	= VlvPolicyData.LPOEnable;
    mGlobalNvsArea.Area->LPOStartPState                           	= VlvPolicyData.LPOStartPState;
    mGlobalNvsArea.Area->LPOStepSize                              	= VlvPolicyData.LPOStepSize;
    mGlobalNvsArea.Area->LPOPowerControlSetting            = VlvPolicyData.LPOPowerControlSetting;
    mGlobalNvsArea.Area->LPOPerformanceControlSetting  = VlvPolicyData.LPOPerformanceControlSetting;
    mGlobalNvsArea.Area->DppmEnabled                             = VlvPolicyData.EnableDppm;

    //
    //
    // Platform Flavor
    //
    mGlobalNvsArea.Area->PlatformFlavor = PLATFORM_FLAVOR_SELECT;

    // Update the Platform id
    // fRC 070 remove the mGlobalNvsArea.Area->BoardId
//    mGlobalNvsArea.Area->BoardId = mPlatformInfo->BoardId;

    // Update the Platform id
    // fRC 070 remove the mGlobalNvsArea.Area->BoardRev
//    mGlobalNvsArea.Area->BoardRev =  mPlatformInfo->BoardRev;

//(EIP120879+)>>
    // Update SOC Stepping
    mGlobalNvsArea.Area->SocStepping = (UINT8)(PchStepping());
    mGlobalNvsArea.Area->WittEnable = PchPolicyData.WittEnable;
//(EIP120879+)<<
    mGlobalNvsArea.Area->XhciMode                 = PchPolicyData.PchUsb30Mode;
    //
    // Override invalid Pre-Boot Driver and XhciMode combination
    //
    if((PchPolicyData.UsbXhciSupport == 0) && (PchPolicyData.PchUsb30Mode == 3)) {
        mGlobalNvsArea.Area->XhciMode               = 2;
    }
    if((PchPolicyData.UsbXhciSupport == 1) && (PchPolicyData.PchUsb30Mode == 2)) {
        mGlobalNvsArea.Area->XhciMode               = 3;
    }
  //PMIC is enabled by default. When it is disabled, we will not expose it in DSDT.
    mGlobalNvsArea.Area->PmicEnable          						= PchPolicyData.PmicEnable;
    mGlobalNvsArea.Area->ISPDevSel           						= VlvPolicyData.ISPDevSel;
    mGlobalNvsArea.Area->LpeEnable          						= PchPolicyData.Lpe;
    mGlobalNvsArea.Area->UartSelection 									= PchPolicyData.UartDebugEnable; //EIP133060
    mGlobalNvsArea.Area->PcuUart1Enable 							 	= PchPolicyData.PcuUart1;
    mGlobalNvsArea.Area->PcuUart2Enable 							 	= PchPolicyData.PcuUart2;

    mGlobalNvsArea.Area->I2CTouchAddress                = 0;
    mGlobalNvsArea.Area->S0ix               			 = PchPolicyData.S0ixSupport;   //(EIP114446)
    
	// Update FRC version 0.70 (EIP120879+)>>
  if (PchPolicyData.eMMCEnabled== 1) {// Auto detect mode
   DEBUG ((EFI_D_ERROR, "Auto detect mode------------start\n"));
   switch (PchStepping()) {
     case PchA0:
     case PchA1:
       DEBUG ((EFI_D_ERROR, "eMMC 4.41 Configuration\n"));
       mGlobalNvsArea.Area->emmcVersion              =  0;
       break;
     case PchB0:
       DEBUG ((EFI_D_ERROR, "eMMC 4.5 Configuration\n"));
       mGlobalNvsArea.Area->emmcVersion              =  1;
       break;
     default:
       DEBUG ((EFI_D_ERROR, "Unknown Steppting, eMMC 4.41 Configuration\n"));
       mGlobalNvsArea.Area->emmcVersion              =  0;
       break;
   }
  }else if (PchPolicyData.eMMCEnabled == 2) { // eMMC 4.41 
      DEBUG ((EFI_D_ERROR, "eMMC 4.41 Configuration\n"));
      mGlobalNvsArea.Area->emmcVersion              =  0;
  } else if (PchPolicyData.eMMCEnabled == 3) { // eMMC 4.5
      DEBUG ((EFI_D_ERROR, "eMMC 4.5 Configuration\n"));
      mGlobalNvsArea.Area->emmcVersion              =  1;
  } else { // Disable eMMC controllers
      DEBUG ((EFI_D_ERROR, "Disable eMMC controllers\n"));
      mGlobalNvsArea.Area->emmcVersion              =  0;
  }
    MsgBus32Read (VLV_BUNIT, BUNIT_BMBOUND, Data32);
    mGlobalNvsArea.Area->BmBound 			= Data32;
    mGlobalNvsArea.Area->FsaStatus               	 = FSA_SUPPORT;// 0 - Fsa is off, 1- Fsa is on
    mGlobalNvsArea.Area->BoardID               	 = mPlatformInfo->BoardId;
    mGlobalNvsArea.Area->FabID               		 = mPlatformInfo->BoardRev;
    mGlobalNvsArea.Area->OtgMode               	 = PchPolicyData.PchUsbOtg;// 0- OTG disable 1- OTG PCI mode  
		    
    RevId = MmioRead8 (
	            MmPciAddress (0,
	              DEFAULT_PCI_BUS_NUMBER_PCH,
	              PCI_DEVICE_NUMBER_PCH_LPC,
	              PCI_FUNCTION_NUMBER_PCH_LPC,
	              R_PCH_LPC_RID_CC)
	            );
    mGlobalNvsArea.Area->Stepping               	 = RevId;// Stepping							    
    //(EIP120879+)<<
	
    //
    // SIO related option
    //
    Status = gBS->LocateProtocol(&gEfiCpuIoProtocolGuid, NULL, &mCpuIo);
    ASSERT_EFI_ERROR(Status);

    mGlobalNvsArea.Area->WPCN381U = GLOBAL_NVS_DEVICE_DISABLE;

    mGlobalNvsArea.Area->DockedSioPresent = GLOBAL_NVS_DEVICE_DISABLE;

    if(mGlobalNvsArea.Area->DockedSioPresent != GLOBAL_NVS_DEVICE_ENABLE) {
        //
        // Check ID for SIO WPCN381U
        //
        Status = mCpuIo->Io.Read(
                     mCpuIo,
                     EfiCpuIoWidthUint8,
                     WPCN381U_CONFIG_INDEX,
                     1,
                     &PortData
                 );
        ASSERT_EFI_ERROR(Status);
        if(PortData != 0xFF) {
            PortData = 0x20;
            Status = mCpuIo->Io.Write(
                         mCpuIo,
                         EfiCpuIoWidthUint8,
                         WPCN381U_CONFIG_INDEX,
                         1,
                         &PortData
                     );
            ASSERT_EFI_ERROR(Status);
            Status = mCpuIo->Io.Read(
                         mCpuIo,
                         EfiCpuIoWidthUint8,
                         WPCN381U_CONFIG_DATA,
                         1,
                         &PortData
                     );
            ASSERT_EFI_ERROR(Status);
            if((PortData == WPCN381U_CHIP_ID) || (PortData == WDCP376_CHIP_ID)) {
                mGlobalNvsArea.Area->WPCN381U = GLOBAL_NVS_DEVICE_ENABLE;
                mGlobalNvsArea.Area->OnboardCom = GLOBAL_NVS_DEVICE_ENABLE;
                mGlobalNvsArea.Area->OnboardComCir = GLOBAL_NVS_DEVICE_DISABLE;
            }
        }
    }

    DEBUG((EFI_D_INFO, "Dumping DPTF settings in global nvs init...\n"));
    DEBUG((EFI_D_INFO, "DPTFEnabled = %d\n", mGlobalNvsArea.Area->DptfEnable));
    DEBUG((EFI_D_INFO, "CpuParticipantCriticalTemperature = %d\n", mGlobalNvsArea.Area->DptfProcCriticalTemperature));
    DEBUG((EFI_D_INFO, "CpuParticipantPassiveTemperature = %d\n", mGlobalNvsArea.Area->DptfProcPassiveTemperature));
    DEBUG((EFI_D_INFO, "GenParticipant0CriticalTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericCriticalTemperature0));
    DEBUG((EFI_D_INFO, "GenParticipant0PassiveTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericPassiveTemperature0));
    DEBUG((EFI_D_INFO, "GenParticipant1CriticalTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericCriticalTemperature1));
    DEBUG((EFI_D_INFO, "GenParticipant1PassiveTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericPassiveTemperature1));
    DEBUG((EFI_D_INFO, "GenParticipant2CriticalTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericCriticalTemperature2));
    DEBUG((EFI_D_INFO, "GenParticipant2PassiveTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericPassiveTemperature2));
    DEBUG((EFI_D_INFO, "GenParticipant3CriticalTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericCriticalTemperature3));
    DEBUG((EFI_D_INFO, "GenParticipant3PassiveTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericPassiveTemperature3));
    DEBUG((EFI_D_INFO, "GenParticipant4CriticalTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericCriticalTemperature4));
    DEBUG((EFI_D_INFO, "GenParticipant4PassiveTemperature = %d\n", mGlobalNvsArea.Area->DptfGenericPassiveTemperature4));

    DEBUG((EFI_D_INFO, "ClpmSetting = %d\n", mGlobalNvsArea.Area->CLpmSetting));
    DEBUG((EFI_D_INFO, "SuperDebug = %d\n", mGlobalNvsArea.Area->DptfSuperDbg));
    DEBUG((EFI_D_INFO, "LPOEnable = %d\n", mGlobalNvsArea.Area->LPOEnable));
    DEBUG((EFI_D_INFO, "LPOStartPState = %d\n", mGlobalNvsArea.Area->LPOStartPState));
    DEBUG((EFI_D_INFO, "LPOStepSize = %d\n", mGlobalNvsArea.Area->LPOStepSize));
    DEBUG((EFI_D_INFO, "LPOPowerControlSetting = %d\n", mGlobalNvsArea.Area->LPOPowerControlSetting));
    DEBUG((EFI_D_INFO, "LPOPerformanceControlSetting = %d\n", mGlobalNvsArea.Area->LPOPerformanceControlSetting));
    DEBUG((EFI_D_INFO, "bDppmEnabled = %d\n", mGlobalNvsArea.Area->DppmEnabled));

    DEBUG((EFI_D_INFO, "DptfEnable = %d\n", mGlobalNvsArea.Area->DptfEnable));
    DEBUG((EFI_D_INFO, "DptfSysThermal0 = %d\n", mGlobalNvsArea.Area->DptfSysThermal0));
    DEBUG((EFI_D_INFO, "DptfSysThermal1 = %d\n", mGlobalNvsArea.Area->DptfSysThermal1));
    DEBUG((EFI_D_INFO, "DptfSysThermal2 = %d\n", mGlobalNvsArea.Area->DptfSysThermal2));
    DEBUG((EFI_D_INFO, "DptfSysThermal3 = %d\n", mGlobalNvsArea.Area->DptfSysThermal3));
    DEBUG((EFI_D_INFO, "DptfCharger = %d\n", mGlobalNvsArea.Area->DptfCharger));
    DEBUG((EFI_D_INFO, "DptfDisplayDevice = %d\n", mGlobalNvsArea.Area->DptfDisplayDevice));
    DEBUG((EFI_D_INFO, "DptfSocDevice = %d\n", mGlobalNvsArea.Area->DptfSocDevice));
    DEBUG((EFI_D_INFO, "DptfProcessor = %d\n", mGlobalNvsArea.Area->DptfProcessor));

    DEBUG((EFI_D_INFO, "PlatformFlavor = %d\n", mGlobalNvsArea.Area->PlatformFlavor));
    DEBUG((EFI_D_INFO, "BoardID = %d\n", mGlobalNvsArea.Area->BoardID));
    DEBUG((EFI_D_INFO, "FabID = %d\n", mGlobalNvsArea.Area->FabID));
    DEBUG((EFI_D_INFO, "XhciMode = %d\n", mGlobalNvsArea.Area->XhciMode));
    DEBUG((EFI_D_INFO, "PmicEnable = %d\n", mGlobalNvsArea.Area->PmicEnable));
    DEBUG((EFI_D_INFO, "BatteryChargingSolution = %d\n", mGlobalNvsArea.Area->BatteryChargingSolution));
    DEBUG((EFI_D_INFO, "ISPDevSel = %d\n", mGlobalNvsArea.Area->ISPDevSel));
    DEBUG((EFI_D_INFO, "LpeEnable = %d\n", mGlobalNvsArea.Area->LpeEnable));
    DEBUG((EFI_D_INFO, "I2CTouchAddress = %d\n", mGlobalNvsArea.Area->I2CTouchAddress));

    DEBUG((EFI_D_INFO, "emmcVersion = %d\n", mGlobalNvsArea.Area->emmcVersion));
    DEBUG((EFI_D_INFO, "BmBound = %d\n", mGlobalNvsArea.Area->BmBound));
    DEBUG((EFI_D_INFO, "FsaStatus = %d\n", mGlobalNvsArea.Area->FsaStatus));
    DEBUG((EFI_D_INFO, "OtgMode = %d\n", mGlobalNvsArea.Area->OtgMode));
    //
    // Get Ps2 policy to set. Will be use if present.
    //
    Status =  gBS->LocateProtocol(
                  &gEfiPs2PolicyProtocolGuid,
                  NULL,
                  (VOID **)&Ps2Policy
              );
    if(!EFI_ERROR(Status)) {
        Status = Ps2Policy->Ps2InitHardware(ImageHandle);
    }

    mGlobalNvsArea.Area->SDIOMode = SettingSDIOMODE;
    Handle = NULL;
    Status = gBS->InstallMultipleProtocolInterfaces(
                 &Handle,
                 &gEfiGlobalNvsAreaProtocolGuid,
                 &mGlobalNvsArea,
                 NULL
             );

    //
    // Read tables from the storage file.
    //
    while(!EFI_ERROR(Status)) {
        CurrentTable = NULL;

        Status = FwVol->ReadSection(
                     FwVol,
                     &gEfiAcpiTableStorageGuid,
                     EFI_SECTION_RAW,
                     Instance,
                     &CurrentTable,
                     (UINTN *) &Size,
                     &FvStatus
                 );

        if(!EFI_ERROR(Status)) {
            //
            // Allow platform specific code to reject the table or update it
            //
            AcpiStatus = AcpiPlatformHooksIsActiveTable(CurrentTable);

            if(!EFI_ERROR(AcpiStatus)) {
                //
                // Perform any table specific updates.
                //
                AcpiStatus = PlatformUpdateTables(CurrentTable);
                if(!EFI_ERROR(AcpiStatus)) {
                    MeasureAcpiTable(CurrentTable);

                    //
                    // Add the table
                    //
                    TableHandle = 0;
                    AcpiStatus = AcpiSupport->SetAcpiTable(
                                     AcpiSupport,
                                     CurrentTable,
                                     TRUE,
                                     TableVersion,
                                     &TableHandle
                                 );
                    ASSERT_EFI_ERROR(AcpiStatus);
                }
            }
            //
            // Increment the instance
            //
            Instance++;
        }
    }

    CreateFpdtTables();

    EfiCreateEventReadyToBootEx(
        TPL_NOTIFY,
        OnReadyToBoot,
        NULL,
        &Event
    );
    //
    // Finished
    //
    return EFI_SUCCESS;
}

EFI_STATUS
MeasureAcpiTable(
    EFI_ACPI_COMMON_HEADER          *Table
)
/*++

Routine Description:

  This will measure the ACPI table

Arguments:

Returns:

  EFI_SUCCESS           - ACPI table measured

--*/
{
    /*
     EFI_STATUS                      Status;
     EFI_TCG_PROTOCOL               *TcgService;
     UINT32                          EventNumber;
     TCG_DIGEST                      TpmDigest;
     TCG_PCR_EVENT                   TssEvent;

     Status = gBS->LocateProtocol (&gEfiTcgProtocolGuid, NULL, &TcgService);
     if (EFI_ERROR (Status)) {
       //
       // TPM not supported, return SUCCESS
       //
       return EFI_SUCCESS;
     }

     //
     // Microcode update should go into PCR1.
     // If the OPROM loads a microcode update, it should go into PCR2
     //
     DEBUG ((EFI_D_ERROR, "TCG hash extend ACPI table to PCR 0\n"));
     TssEvent.EventType  = 1;
     TssEvent.PCRIndex   = 0;
     TssEvent.EventSize  = 0;
     Status  = TcgService->HashLogExtendEvent (
                             TcgService,
                             (EFI_PHYSICAL_ADDRESS)(UINT8 *) Table,
                             Table->Length,
                             TssEvent.PCRIndex,
                             &TssEvent,
                             (UINT32)((UINTN) (&(TssEvent.Event)) - (UINTN) (&TssEvent)),
                             &EventNumber,
                             &TpmDigest
                             );
     if (EFI_ERROR (Status)) {
       DEBUG ((EFI_D_ERROR, "Error: Failed to hash ACPI table to PCR 0\n"));
     }
    */
    return  EFI_SUCCESS;
}

UINT8
ReadCmosBank1Byte(
    IN  EFI_CPU_IO_PROTOCOL             *CpuIo,
    IN  UINT8                           Index
)
{
    UINT8                               Data;

    CpuIo->Io.Write(CpuIo, EfiCpuIoWidthUint8, 0x72, 1, &Index);
    CpuIo->Io.Read(CpuIo, EfiCpuIoWidthUint8, 0x73, 1, &Data);
    return Data;
}

VOID
WriteCmosBank1Byte(
    IN  EFI_CPU_IO_PROTOCOL             *CpuIo,
    IN  UINT8                           Index,
    IN  UINT8                           Data
)
{
    CpuIo->Io.Write(CpuIo, EfiCpuIoWidthUint8, 0x72, 1, &Index);
    CpuIo->Io.Write(CpuIo, EfiCpuIoWidthUint8, 0x73, 1, &Data);
}

VOID
CreateFpdtTables(VOID)
{
    EFI_STATUS Status;
    EFI_ACPI_COMMON_HEADER      *CurrentTable;
    UINTN                       TableHandle;
    FPDT_50                     *FPDT;
    UINT8                       *TempPtr;
    PERF_TAB_HEADER             *S3PerfRecHdr, *BasBootPerfRecHdr;
    BASIC_S3_RESUME_PERF_REC    *S3PerfRec;
    BASIC_BOOT_PERF_REC         *BasBootPerfRec;
    UINTN                       RecordsSize;
    EFI_FPDT_STRUCTURE          *FpdtVar, *OldFpdtVarAddress;
    EFI_CPU_ARCH_PROTOCOL       *Cpu;
    UINTN                       VarSize = sizeof(UINT32);
//      UINT32                      ACPI_OEM_REV = 0x1072009;
//      UINT32                      CREATOR_ID_AMI = 0x20494D41; //"AMI"
//      UINT32                      CREATOR_REV_MS = 0x00010013;
    UINT8                       OemId[6] = "A M I";
    UINT8                       OemTblId[8] = "ALASKA";
    EFI_ACPI_SUPPORT_PROTOCOL   *AcpiSupport;


    TableHandle  = 0;
    RecordsSize = (sizeof(EFI_FPDT_STRUCTURE) + sizeof(PERF_TAB_HEADER)*2 +
                   sizeof(BASIC_S3_RESUME_PERF_REC)+
                   sizeof(BASIC_BOOT_PERF_REC));
    Status = gBS->AllocatePool(EfiRuntimeServicesData, RecordsSize, &TempPtr);
    DEBUG((EFI_D_INFO,"Check AllocatePool EfiRuntimeServicesData: %r.\n" , Status));

    gBS->SetMem(TempPtr, RecordsSize, 0);

    FpdtVar = (EFI_FPDT_STRUCTURE*) TempPtr;
    TempPtr += sizeof(EFI_FPDT_STRUCTURE);
    S3PerfRecHdr = (PERF_TAB_HEADER*) TempPtr;
    TempPtr += sizeof(PERF_TAB_HEADER);
    S3PerfRec = (BASIC_S3_RESUME_PERF_REC*)TempPtr;
    TempPtr += sizeof(BASIC_S3_RESUME_PERF_REC);
    BasBootPerfRecHdr = (PERF_TAB_HEADER*) TempPtr;
    TempPtr += sizeof(PERF_TAB_HEADER);
    BasBootPerfRec = (BASIC_BOOT_PERF_REC*)TempPtr;
    DEBUG((EFI_D_INFO,"Check AllocatePool FpdtVar: %x.\n" , FpdtVar));
    DEBUG((EFI_D_INFO,"Check AllocatePool S3PerfRecHdr: %x.\n" , S3PerfRecHdr));
    DEBUG((EFI_D_INFO,"Check AllocatePool S3PerfRec: %x.\n" , S3PerfRec));
    DEBUG((EFI_D_INFO,"Check AllocatePool BasBootPerfRecHdr: %x.\n" , BasBootPerfRecHdr));

    S3PerfRecHdr->Signature = 0x54503353;//   `S3PT'
    S3PerfRecHdr->Length = sizeof(PERF_TAB_HEADER) + sizeof(BASIC_S3_RESUME_PERF_REC);

    S3PerfRec->Header.PerfRecType = 0;
    S3PerfRec->Header.RecLength = sizeof(BASIC_S3_RESUME_PERF_REC);
    S3PerfRec->Header.Revision = 1;

    BasBootPerfRecHdr->Signature = 0x54504246;//  `FBPT'
    BasBootPerfRecHdr->Length = sizeof(PERF_TAB_HEADER) + sizeof(BASIC_BOOT_PERF_REC);

    BasBootPerfRec->Header.PerfRecType = 2;
    BasBootPerfRec->Header.RecLength = sizeof(BASIC_BOOT_PERF_REC);
    BasBootPerfRec->Header.Revision = 2;


    CurrentTable = NULL;
    CurrentTable = AllocateZeroPool(sizeof(EFI_ACPI_COMMON_HEADER));


    //Set the FPDT content
    FPDT = AllocateZeroPool(sizeof(FPDT_50));
    gBS->SetMem(FPDT,sizeof(FPDT_50),0);
    FPDT->Header.Signature = FPDT_SIG;
    FPDT->Header.Length = sizeof(FPDT_50);
    FPDT->Header.Revision = 1;
    FPDT->Header.Checksum = 0;
    CopyMem(&(FPDT->Header.OemId[0]), OemId, 6);
    CopyMem(&(FPDT->Header.OemTblId[0]), OemTblId, 8);

    FPDT->Header.OemRev = ACPI_OEM_REV;
    FPDT->Header.CreatorId = CREATOR_ID_AMI;
    FPDT->Header.CreatorRev = CREATOR_REV_MS;

    //fill BasS3Rec Fields
    FPDT->BasS3Rec.PerfRecType = 1;
    FPDT->BasS3Rec.Length = sizeof(FPDT_PERF_RECORD);
    FPDT->BasS3Rec.Revision = 1;
    FPDT->BasS3Rec.Pointer = (EFI_PHYSICAL_ADDRESS)((UINTN)S3PerfRecHdr);

    //fill BasBootRec Fields
    FPDT->BasBootRec.PerfRecType = 0;
    FPDT->BasBootRec.Length = sizeof(FPDT_PERF_RECORD);
    FPDT->BasBootRec.Revision = 1;
    FPDT->BasBootRec.Pointer = (EFI_PHYSICAL_ADDRESS)((UINTN)BasBootPerfRecHdr);
    FPDT->Header.Checksum = CalculateCheckSum8((UINT8*)FPDT, FPDT->Header.Length);

    // Get Cpu Frequency
    Status = gBS->LocateProtocol(
                 &gEfiCpuArchProtocolGuid,
                 NULL,
                 &Cpu
             );
    DEBUG((EFI_D_INFO,"Check Locate CpuArchProtocol: %r.\n" , Status));
    if(!EFI_ERROR(Status)) {
        UINT64   CurrentTicker, TimerPeriod;

        Status = Cpu->GetTimerValue(Cpu, 0, &CurrentTicker, &TimerPeriod);
        DEBUG((EFI_D_INFO,"Check GetTimerValue: %r.\n" , Status));
        if(!EFI_ERROR(Status))
//(EIP88889)>>
            FpdtVar->NanoFreq = TimerPeriod;
        DEBUG((EFI_D_INFO,"FpdtVar->NanoFreq:%x \n" ,FpdtVar->NanoFreq));
//         FpdtVar->NanoFreq = DivU64x32Remainder(1000000, (UINTN) TimerPeriod, NULL);
//(EIP88889)<<
    }

    FpdtVar->S3Pointer = FPDT->BasS3Rec.Pointer;
    FpdtVar->BasBootPointer = FPDT->BasBootRec.Pointer;
    Status = gRT->GetVariable(
                 EFI_FPDT_VARIABLE, &gAmiGlobalVariableGuid,
                 NULL, &VarSize, &OldFpdtVarAddress
             );
    DEBUG((EFI_D_INFO,"Check GetVariable FPDT: %r.\n" , Status));
    if(EFI_ERROR(Status) || (FpdtVar != OldFpdtVarAddress)) {
        Status = gRT->SetVariable(
                     EFI_FPDT_VARIABLE,
                     &gAmiGlobalVariableGuid,
                     EFI_VARIABLE_NON_VOLATILE |
                     EFI_VARIABLE_BOOTSERVICE_ACCESS |
                     EFI_VARIABLE_RUNTIME_ACCESS,
                     sizeof(UINT32),
                     &FpdtVar
                 );
        ASSERT_EFI_ERROR(Status);
    }

    CurrentTable = (EFI_ACPI_COMMON_HEADER*) FPDT;  //pointer to FPDT table

    Status = gBS->LocateProtocol(&gEfiAcpiSupportProtocolGuid, NULL, &AcpiSupport);
    DEBUG((EFI_D_INFO,"Check LocateProtocol: %r.\n" , Status));
    Status = AcpiSupport->SetAcpiTable(
                 AcpiSupport,
                 CurrentTable,
                 TRUE,
                 EFI_ACPI_TABLE_VERSION_1_0B | EFI_ACPI_TABLE_VERSION_2_0,
                 &TableHandle
             );
    DEBUG((EFI_D_INFO,"Check SetAcpiTable FPDT: %r.\n" , Status));

}
