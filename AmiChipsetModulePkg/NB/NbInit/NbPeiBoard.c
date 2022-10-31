//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//*************************************************************************
// $Header:  $
//
// $Revision:  $
//
// $Date:  $
//*************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        NbPeiBoard.c
//
// Description: This file contains PEI stage board component code for
//              Template NB
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


// Module specific Includes
#include <Efi.h>
#include <Pei.h>
#include <token.h>
#include <AmiPeiLib.h>
#include <Library/NBCspLib.h>
#include <PPI/CspLibPpi.h>
#include <PPI/Cache.h>
#include <Protocol\MpService.h>
#include <Library/CpuConfigLib.h>
#include <CpuRegs.h>
#include <PPI/ReadOnlyVariable2.h>
#include <Guid/MemoryTypeInformation.h>
#include <Library/HobLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/NbPolicy.h>
#include <Library/NcPolicyInitPei.h>
#include <Nb.h>
// Produced PPIs

// GUID Definitions

// Portable Constants

// Function Prototypes

// PPI interface definition
AMI_PEI_PCI_INIT_TABLE_STRUCT stNBH2P_PCIInitTable [] = {
    { 0x00, 0xFF, 0x00 },
    { 0xFF, 0xFF, 0xFF }
//  { Register, AND Mask, OR Mask },
//  { Register, AND Mask, OR Mask}
};
UINT16      wNBH2P_PCIInitTableSize = sizeof(stNBH2P_PCIInitTable)/sizeof(AMI_PEI_PCI_INIT_TABLE_STRUCT);

EFI_MEMORY_TYPE_INFORMATION mDefaultMemoryTypeInformation[] = {
    { EfiACPIReclaimMemory,       0x40  },    // 0x40 pages = 256k for ASL
    { EfiACPIMemoryNVS,           0x100 },    // 0x100 pages = 1 MB for S3, SMM, HII, etc
    { EfiReservedMemoryType,      0x600 },    // 48k for BIOS Reserved
    { EfiMemoryMappedIO,          0     },
    { EfiMemoryMappedIOPortSpace, 0     },
    { EfiPalCode,                 0     },
    { EfiRuntimeServicesCode,     0x200 },  // EIP128872
    { EfiRuntimeServicesData,     0x100 },  // EIP128872
    { EfiLoaderCode,              0x100 },
    { EfiLoaderData,              0x100 },
    { EfiBootServicesCode,        0x800 },
    { EfiBootServicesData,        0x2500},  // EIP128872
    { EfiConventionalMemory,      0     },
    { EfiUnusableMemory,          0     },
    { EfiMaxMemoryType,           0     }
};

// Function Definition


STATIC
EFI_STATUS
GetMemorySize(
    IN  CONST EFI_PEI_SERVICES    **PeiServices,
    OUT UINT64              *LowMemoryLength,
    OUT UINT64              *HighMemoryLength
)
{
    EFI_STATUS              Status;
    EFI_PEI_HOB_POINTERS    Hob;

    *HighMemoryLength = 0;
    *LowMemoryLength = 0x100000;
    //
    // Get the HOB list for processing
    //
    Status = (*PeiServices)->GetHobList(PeiServices, &Hob.Raw);
    if(EFI_ERROR(Status)) {
        return Status;
    }

    //
    // Collect memory ranges
    //
    while(!END_OF_HOB_LIST(Hob)) {
        if(Hob.Header->HobType == EFI_HOB_TYPE_RESOURCE_DESCRIPTOR) {
            if(Hob.ResourceDescriptor->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY) {
                //
                // Need memory above 1MB to be collected here
                //
                if(Hob.ResourceDescriptor->PhysicalStart >= 0x100000 &&
                        Hob.ResourceDescriptor->PhysicalStart < (EFI_PHYSICAL_ADDRESS) 0x100000000) {
                    *LowMemoryLength += (UINT64)(Hob.ResourceDescriptor->ResourceLength);
                } else if(Hob.ResourceDescriptor->PhysicalStart >= (EFI_PHYSICAL_ADDRESS) 0x100000000) {
                    *HighMemoryLength += (UINT64)(Hob.ResourceDescriptor->ResourceLength);
                }
            }
        }
        Hob.Raw = GET_NEXT_HOB(Hob);
    }

    return EFI_SUCCESS;
}


EFI_STATUS
EFIAPI
SetPeiCacheMode(
    IN  CONST EFI_PEI_SERVICES    **PeiServices
)
{
    EFI_STATUS              Status;
    PEI_CACHE_PPI           *CachePpi;
    UINT64                  LowMemoryLength;
    UINT64                  MaxLowMemoryLength;
    UINT64                  HighMemoryLength;
    UINT64                  MaxHighMemoryLength;
    UINT64                  MemoryLengthUc;
    UINT32                  MemOverflow;
    UINT32      			TsegTop = 0, TsegBase = NBGetTsegBase(); //CSP20140329_22

    EFI_BOOT_MODE           BootMode;

    Status = (*PeiServices)->GetBootMode(PeiServices, &BootMode);

    //
    // Variable initialization
    //
    MaxLowMemoryLength = 0;
    MaxHighMemoryLength = 0;
    MemoryLengthUc = 0;

    //
    // Determine memory usage
    //
    GetMemorySize(
        PeiServices,
        &LowMemoryLength,
        &HighMemoryLength
    );

    //
    // Set start of UC area below 4GB
    //
    MaxLowMemoryLength = LowMemoryLength;

    //
    // Round up to nearest 256MB
    //
    MemOverflow = (UINT32)LowMemoryLength & 0x0fffffff;
    if(MemOverflow) {
        MaxLowMemoryLength = LowMemoryLength + (0x10000000 - MemOverflow);
    }

    //
    // Compute memory to cache above 4GB if available and set the proper UC range
    //
    if(HighMemoryLength) {
        MaxHighMemoryLength = HighMemoryLength;
        if(HighMemoryLength != GetPowerOfTwo64(HighMemoryLength)) {
            MaxHighMemoryLength = GetPowerOfTwo64(LShiftU64(HighMemoryLength, 1));
        }
        //
        // WORKAROUND: Due to limited MTRR available, we need to sacrify some cache range here with 16MB alignment for UC range
        //
        MemoryLengthUc = (MaxLowMemoryLength - LowMemoryLength);
    } else {
        MemoryLengthUc = (0x10000000 - MemOverflow);
    }

    //
    // Load Cache PPI
    //
    Status = (**PeiServices).LocatePpi(
                 PeiServices,
                 &gPeiCachePpiGuid,    // GUID
                 0,                    // Instance
                 NULL,                 // EFI_PEI_PPI_DESCRIPTOR
                 &CachePpi             // PPI
             );
    if(!EFI_ERROR(Status)) {
        //
        // Clear the CAR Settings (Default Cache Type => UC)
        //
        AsmDisableCache (); //EIP132575
        CachePpi->ResetCache(
            (EFI_PEI_SERVICES**)PeiServices,
            CachePpi
        );

        if(!(BootMode == BOOT_ON_S3_RESUME)) {
            //
            // Set fixed MTRR values
            //
            CachePpi->SetCache(
                (EFI_PEI_SERVICES**)PeiServices,
                CachePpi,
                0x00000,
                0xA0000,
                EFI_CACHE_WRITEBACK
            );

            CachePpi->SetCache(
                (EFI_PEI_SERVICES**)PeiServices,
                CachePpi,
                0xA0000,
                0x20000,
                EFI_CACHE_UNCACHEABLE
            );

            //
            // Cache low memory below 4GB
            //

//EIP164574 >>
            CachePpi->SetCache(
                (EFI_PEI_SERVICES**)PeiServices,
                CachePpi,
                0,
                MaxLowMemoryLength,
                EFI_CACHE_WRITEBACK
            );

            //
            // Uncache Graphics and TSEG here
            //
			if ( (UINT32)LowMemoryLength == TsegBase) { // If Tseg is aligned with LowMemoryLength
			  CachePpi->SetCache(					  // then ignore the Tseg, Tseg would be covered by SMRR
				  (EFI_PEI_SERVICES**)PeiServices,	  // Uncache Graphics here
				  CachePpi,
				  LowMemoryLength + TSEG_SIZE,
				  MemoryLengthUc - TSEG_SIZE,
				  EFI_CACHE_UNCACHEABLE
			  );
			} else {
			  CachePpi->SetCache(	 // Uncache Graphics and TSEG here
				  (EFI_PEI_SERVICES**)PeiServices,
				  CachePpi,
				  LowMemoryLength,
				  MemoryLengthUc,
				  EFI_CACHE_UNCACHEABLE
			  );
			}


            //
            // WORKAROUND: Due to we don't have enough MTRR, in order to let recovery run fast with 4GB memory
            //             BIOS has to let go the caching for memory above 4GB so that we can free up 1 MTRR
            //             for Boot Block 2 caching. During recovery BIOS is not utilizing memory above 4GB anyway.
            //
            if(!(BootMode == BOOT_IN_RECOVERY_MODE)) {
                //
                // Cache memory above 4GB
                //
                if(HighMemoryLength) {
                    CachePpi->SetCache(
                        (EFI_PEI_SERVICES**)PeiServices,
                        CachePpi,
                        0x100000000,        // Start caching from 4GB and up.
                        HighMemoryLength,
                        EFI_CACHE_WRITEBACK
                    );
                }
            }	
//EIP164574 <<
        }

        //
        // Cache flash firmware bootblocks
        // Note: The caching of bootblock 2 may not work for 4GB system. This will cause S3 resume time longer.
        //
    }

    return EFI_SUCCESS;
}


/*++

Routine Description:

  Publish Memory Type Information.

Arguments:

  NULL

Returns:

  EFI_SUCCESS  -  Success.
  Others       -  Errors have occurred.
--*/

EFI_STATUS
EFIAPI
PublishMemoryTypeInfo(
    IN  CONST EFI_PEI_SERVICES    **PeiServices
)
{
    EFI_STATUS                      Status;
    EFI_PEI_READ_ONLY_VARIABLE2_PPI *Variable;
    UINTN                           DataSize;
    EFI_MEMORY_TYPE_INFORMATION     MemoryData[EfiMaxMemoryType + 1];

    Status = PeiServicesLocatePpi(
                 &gEfiPeiReadOnlyVariable2PpiGuid,
                 0,
                 NULL,
                 &Variable
             );
    if(EFI_ERROR(Status)) {
        PEI_TRACE((-1,PeiServices,  "WARNING: Locating Pei variable failed 0x%x \n", Status));
        PEI_TRACE((-1,PeiServices,  "Build Hob from default\n"));
        //
        // Build the default GUID'd HOB for DXE
        //
        BuildGuidDataHob(&gEfiMemoryTypeInformationGuid, mDefaultMemoryTypeInformation, sizeof(mDefaultMemoryTypeInformation));

        return Status;
    }


    DataSize = sizeof(MemoryData);
    // This variable is saved in BDS stage. Now read it back
    Status = Variable->GetVariable(
                 Variable,
                 EFI_MEMORY_TYPE_INFORMATION_VARIABLE_NAME,
                 &gEfiMemoryTypeInformationGuid,
                 NULL,
                 &DataSize,
                 &MemoryData
             );
    if(EFI_ERROR(Status)) {
        //build default
        PEI_TRACE((-1,PeiServices,  "Build Hob from default\n"));
        BuildGuidDataHob(&gEfiMemoryTypeInformationGuid, mDefaultMemoryTypeInformation, sizeof(mDefaultMemoryTypeInformation));

    } else {
        // Build the GUID'd HOB for DXE from variable
        PEI_TRACE((-1,PeiServices,  "Build Hob from variable \n"));
        BuildGuidDataHob(&gEfiMemoryTypeInformationGuid, MemoryData, DataSize);
    }

    return Status;
}

EFI_STATUS
PlatformSsaInit(
    IN CONST EFI_PEI_SERVICES      **PeiServices,
    IN NB_SETUP_DATA               *VlvPolicyData
)
/*++

Routine Description:

  Perform SSA related platform initialization.

--*/
{

    PEI_TRACE((-1,PeiServices, "PlatformSsaInit() - Start\n"));
    PEI_TRACE((-1,PeiServices, "PlatformSsaInit() - VlvPolicyData->ISPDevSel 0x%x\n",VlvPolicyData->ISPDevSel));
    if(VlvPolicyData->ISPDevSel == 0x02) {
        //
        // Device 3 Interrupt Route
        //
        MmioWrite16(
            (ILB_BASE_ADDRESS + R_PCH_ILB_D3IR),
            V_PCH_ILB_DXXIR_IAR_PIRQH   // For IUNIT
        );
        MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D3IR); // Read Posted Writes Register
        PEI_TRACE((-1,PeiServices, "PlatformSsaInit() - Device 3 Interrupt Route Done\n"));
    }

    //
    // Device 2 Interrupt Route
    //
    MmioWrite16(
        (ILB_BASE_ADDRESS + R_PCH_ILB_D2IR),
        V_PCH_ILB_DXXIR_IAR_PIRQA   // For IGD
    );
    MmioRead16(ILB_BASE_ADDRESS + R_PCH_ILB_D2IR); // Read Posted Writes Register
    PEI_TRACE((-1,PeiServices, "PlatformSsaInit() - Device 2 Interrupt Route Done\n"));

    return EFI_SUCCESS;
}

EFI_STATUS
VlvPolicyInit(
    IN CONST EFI_PEI_SERVICES             **PeiServices,
    IN OUT NB_SETUP_DATA                  *VlvPolicyData
)
{
    EFI_STATUS                      Status;

    Status = NcPolicyInitPei(PeiServices, VlvPolicyData);

    return Status;
}

EFI_STATUS
VlvPlatformInit(
    IN CONST EFI_PEI_SERVICES             **PeiServices,
    IN NB_SETUP_DATA                      *VlvPolicyData
)
{
    EFI_STATUS                      Status;

    // Do basic Vlv init
    Status = VlvPolicyInit(PeiServices, VlvPolicyData);
    ASSERT_PEI_ERROR(PeiServices, Status);

    // Perform SSA related platform initialization.
    Status = PlatformSsaInit(PeiServices, VlvPolicyData);
    ASSERT_PEI_ERROR(PeiServices, Status);

    // Set LVDS_BKLT_CTRL to 50%.
    MmPci8(0, 0, 2, 0, 0xF4) = 128;

    return Status;
}
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
