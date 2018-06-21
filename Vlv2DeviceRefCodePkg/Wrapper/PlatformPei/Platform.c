/*++
  This file contains an 'Intel Pre-EFI Module' and is licensed
  for Intel CPUs and Chipsets under the terms of your license
  agreement with Intel or your vendor.  This file may be
  modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file
  This PEIM initialize platform for MRC, following action is performed,
    1. Initizluize GMCH
    2. Detect boot mode
    3. Detect video adapter to determine whether we need pre allocated memory
    4. Calls MRC to initialize memory and install a PPI notify to do post memory initialization.
  This file contains the main entrypoint of the PEIM.

Copyright (c) 2010, Intel Corporation.<BR>
All rights reserved.  This software and associated documentation
(if any) is furnished under a license and may only be used or
copied in accordance with the terms of the license.  Except as
permitted by such license, no part of this software or
documentation may be reproduced, stored in a retrieval system, or transmitted
in any form or by any means without the express written consent of Intel Corporation.

**/

#include <Library/BaseMemoryLib.h>
#include <Guid/PlatformInfo.h>
#include <Library/DebugLib.h>
#include <Library/PmicLib.h>
#include "PlatformBaseAddresses.h"
#include "PchRegs.h"
#include <Ppi/Stall.h>
#include <Library/IoLib.h>
#include <Platform.h>    //(CSP20130111F+)
#include <AmiLib.h>
#include <Token.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Setup.h>
//CSP20130725 >>
#include <AmiCspLib.h>
#include <PchCommonDefinitions.h>
//CSP20130725 <<
#include <AcpiOemElinks.h>  //EIP134732
#include <Library/KscLib.h>

static EFI_GUID gSetupGuid = SETUP_GUID; 

#define GTT_SIZE_1MB        1 //CSP20130725 

EFIAPI
SeCUmaEntry(
    IN       EFI_PEI_FILE_HANDLE       FileHandle,
    IN CONST EFI_PEI_SERVICES          **PeiServices
);
#ifdef FTPM_ENABLE
EFI_STATUS
PeimfTPMInit (
  IN EFI_PEI_FILE_HANDLE       *FfsHeader,
  IN CONST EFI_PEI_SERVICES    **PeiServices
  );
#endif

#ifdef NOCS_S3_SUPPORT
EFI_STATUS
UpdateBootMode(
    IN CONST EFI_PEI_SERVICES                       **PeiServices
);
#endif
  
EFI_STATUS
EFIAPI
Stall(
    IN CONST EFI_PEI_SERVICES   **PeiServices,
    IN CONST EFI_PEI_STALL_PPI      *This,
    IN UINTN              Microseconds
);

static EFI_PEI_STALL_PPI  mStallPpi = {
    1,
    Stall
};

static EFI_PEI_PPI_DESCRIPTOR mInstallStallPpi = {
    EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
    &gEfiPeiStallPpiGuid,
    &mStallPpi
};

//(CSP20130111F+)>>
EFI_PEI_NOTIFY_DESCRIPTOR mMemoryDiscoveredNotifyList[1] = {
    {
        (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
        &gEfiPeiMemoryDiscoveredPpiGuid,
        MemoryDiscoveredPpiNotifyCallback
    }
};
//(CSP20130111F+)<<

//
// The global indicator, the FvFileLoader callback will modify it to TRUE after loading PEIM into memory
//

EFI_STATUS
ReadPlatformIds(
    IN CONST EFI_PEI_SERVICES             **PeiServices,
    IN OUT EFI_PLATFORM_INFO_HOB          *PlatformInfoHob
);

//CSP20130725 >>
void
SetupIGDStolenMemory (
  IN CONST EFI_PEI_SERVICES                **PeiServices
  )
{
    UINTN                             VariableSize;
    EFI_STATUS                        Status;
    EFI_PEI_READ_ONLY_VARIABLE2_PPI   *Variable = NULL;
    SETUP_DATA                        SystemConfiguration;
    UINT32                            GGC = 0;
    
    DEBUG((EFI_D_INFO, "SetupIGDStolenMemory Start.\n"));
    VariableSize = sizeof (SETUP_DATA);
    ZeroMem (&SystemConfiguration, VariableSize);
    Status = (*PeiServices)->LocatePpi (PeiServices,
                                        &gEfiPeiReadOnlyVariable2PpiGuid,
                                        0,
                                        NULL,
                                        &Variable
                                        );
    ASSERT_EFI_ERROR (Status);
  
    Status = Variable->GetVariable (Variable,
                                     L"Setup",
                                     &gSetupGuid,
                                     NULL,
                                     &VariableSize,
                                     &SystemConfiguration);
    ASSERT_EFI_ERROR(Status);
    if(SystemConfiguration.InternalGraphics == 1) {
      GGC = (SystemConfiguration.IgdDvmt50PreAlloc << 3) |
            (SystemConfiguration.GttSize == GTT_SIZE_1MB ? 0x100: 0x200);
      MmioWrite16(CSP_PCIE_CFG_ADDRESS(0, 2, 0, 0x50), GGC);
      GGC = MmioRead16(CSP_PCIE_CFG_ADDRESS(0, 2, 0, 0x50));
      DEBUG((EFI_D_INFO , "GGC: 0x%08x GMSsize:0x%08x\n", GGC, (GGC & (BIT7|BIT6|BIT5|BIT4|BIT3))>>3));
    }
    DEBUG((EFI_D_INFO, "SetupIGDStolenMemory End.\n"));
}
//CSP20130725 <<

#ifdef FTPM_ENABLE
EFI_STATUS
EFIAPI
fTPMInit (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  
    EFI_STATUS                                  Status;
    SETUP_DATA                      	    SetupData;
    UINTN                                             VariableSize = sizeof(SETUP_DATA);
    EFI_PEI_READ_ONLY_VARIABLE2_PPI *ReadOnlyVariable = NULL;

    Status = (*PeiServices)->LocatePpi(PeiServices, \
                                       &gEfiPeiReadOnlyVariable2PpiGuid, \
                                       0, \
                                       NULL, \
                                       &ReadOnlyVariable);

    if(!EFI_ERROR(Status)) {
        Status = ReadOnlyVariable->GetVariable(ReadOnlyVariable, \
                                               L"Setup", \
                                               &gSetupGuid, \
                                               NULL, \
                                               &VariableSize, \
                                               &SetupData);
    }

    if(SetupData.fTPM == 1){
    	    PeimfTPMInit(FileHandle, PeiServices);
    }
    
  return EFI_SUCCESS;
}
#endif
/**
  This is the entrypoint of PEIM

  @param  FileHandle  Handle of the file being invoked.
  @param  PeiServices Describes the list of possible PEI Services.

  @retval EFI_SUCCESS if it completed successfully.
**/
EFI_STATUS
EFIAPI
PeiInitPlatform(
    IN       EFI_PEI_FILE_HANDLE  FileHandle,
    IN CONST EFI_PEI_SERVICES     **PeiServices
)
{

    EFI_PLATFORM_INFO_HOB       PlatformInfo;
    EFI_STATUS                  Status;
    UINT8                       OemId[6] = ACPI_OEM_ID_MAK;  //EIP134732
    UINT8                       OemTblId[8] = ACPI_OEM_TBL_ID_MAK;  //EIP134732
    UINTN                       i;

    ZeroMem(&PlatformInfo, sizeof(EFI_PLATFORM_INFO_HOB)); //P20120624_2 

    //
    // Initialize Stall PPIs
    //
    Status = (*PeiServices)->InstallPpi(PeiServices, &mInstallStallPpi);
    ASSERT_EFI_ERROR(Status);

//(CSP20130111F+)>>
#if defined (EnableCacheflashByPlatform) && (EnableCacheflashByPlatform == 1)
    Status = (*PeiServices)->NotifyPpi(PeiServices, &mMemoryDiscoveredNotifyList[0]);
    ASSERT_EFI_ERROR(Status);
#endif
//(CSP20130111F+)<<

//CSP20130725 >>
    //Setup IGD stolen memory size according to BIOS setup
    //MRC will allocate stolen memory to IGD according to this.
    //Refer to GraphicsInit in GraphicsInit.c  and ProgMemoryMappingRegisters in ConfigMem.c
    PchMmPci32( 0, 0, 2, 0, 0x50) = 0x210;
    SetupIGDStolenMemory(PeiServices);
//CSP20130725 <<    
    // Initialize PlatformInfo HOB
    //
    Status = ReadPlatformIds(PeiServices, &PlatformInfo);
    ASSERT_EFI_ERROR(Status);

    for(i=0; i<6; i++) PlatformInfo.AcpiOemId[i]=OemId[i];

    for(i=0; i<8; i++) PlatformInfo.AcpiOemTableId[i]=OemTblId[i];

//P20120624_2 >>
#if defined(PLATFORM_FLAVOR_SELECT) && PLATFORM_FLAVOR_SELECT == 1
    PlatformInfo.PlatformFlavor = FlavorMobile;
#elif defined(PLATFORM_FLAVOR_SELECT) && PLATFORM_FLAVOR_SELECT == 2
    PlatformInfo.PlatformFlavor = FlavorDesktop;
#else
    PlatformInfo.PlatformFlavor = FlavorMobile;
#endif
//P20120624_2 <<
    
    //
    // Build HOB for PlatformInfo
    //
    BuildGuidDataHob(
        &gEfiPlatformInfoGuid,
        &PlatformInfo,
        sizeof(EFI_PLATFORM_INFO_HOB)
    );

    SeCUmaEntry(FileHandle, PeiServices);
#ifdef FTPM_ENABLE
  fTPMInit(FileHandle, PeiServices);
#endif

  //
  // Set the new boot mode for MRC
  // 
#ifdef NOCS_S3_SUPPORT
  Status = UpdateBootMode (PeiServices); 
  ASSERT_EFI_ERROR (Status);
#endif
    return Status;
}

EFI_STATUS
ReadPlatformIds(
    IN CONST EFI_PEI_SERVICES             **PeiServices,
    IN OUT EFI_PLATFORM_INFO_HOB          *PlatformInfoHob
)
{
    EFI_STATUS  Status;
    UINTN       Count = 0;
    UINT8       KscStatus = 0;
    UINT8       BoardId = 0;
    UINT8       MemCfgID = 0;
    UINT8       FabId = 0;
    UINT8       BoardRev = 0;

    if(ENABLE_OVERRIDE_SUPPORT == 0) {
        DEBUG ((EFI_D_ERROR,  "ReadPlatformIds()\n"));

        Status = SendKscCommand (KSC_C_FAB_ID);
        if(!EFI_ERROR(Status)) {
            // Read 1st Byte from EC (MSB)
            Status = ReceiveKscData (&FabId);
            if(!EFI_ERROR(Status)) {
                FabId = (FabId >> 1) & 0x7;
                BoardRev = FabId + 1;
                
                // Read 2nd Byte from EC (LSB)
                Status = ReceiveKscData (&BoardId);
                if(!EFI_ERROR(Status)) {
                    BoardId &= 0x3f;
                }
            }
        }

        DEBUG((EFI_D_ERROR,  "BoardId   = %2xh\n", BoardId));
        DEBUG((EFI_D_ERROR,  "MemCfgID  = %2xh\n", MemCfgID));
        DEBUG((EFI_D_ERROR,  "BoardRev  = %2xh\n", BoardRev));

        if (BoardId == 1) {
            PlatformInfoHob->BoardId = BOARD_ID_BB_RVP;
            DEBUG ((EFI_D_ERROR,  "Bayley Bay CRB detected\n"));
        }
        else if (BoardId == 3) {
            PlatformInfoHob->BoardId = BOARD_ID_BS_RVP;
            DEBUG ((EFI_D_ERROR,  "Bakersport CRB detected\n"));
        }
        else {
            PlatformInfoHob->BoardId = OVERRIDE_BOARD_ID;
            MemCfgID = OVERRIDE_MEMCFG_ID;
            BoardRev = OVERRIDE_BOARD_REV;
            DEBUG ((EFI_D_ERROR,  "Unknown board!! Set BoardId to OVERRIDE_BOARD_x tokens:\n"));
            DEBUG((EFI_D_ERROR,  "BoardId   = %2xh\n", PlatformInfoHob->BoardId));
            DEBUG((EFI_D_ERROR,  "MemCfgID  = %2xh\n", MemCfgID));
            DEBUG((EFI_D_ERROR,  "BoardRev  = %2xh\n", BoardRev));
        }

        PlatformInfoHob->MemCfgID   = MemCfgID;
        PlatformInfoHob->BoardRev   = BoardRev;
        
    } else {
        PlatformInfoHob->BoardId    = OVERRIDE_BOARD_ID;
        PlatformInfoHob->MemCfgID   = OVERRIDE_MEMCFG_ID;
        PlatformInfoHob->BoardRev   = OVERRIDE_BOARD_REV;
        DEBUG((EFI_D_ERROR,  "Enable the override\n"));
        DEBUG((EFI_D_ERROR,  "BoardId   = %2xh\n", PlatformInfoHob->BoardId));
        DEBUG((EFI_D_ERROR,  "MemCfgID  = %2xh\n", PlatformInfoHob->MemCfgID));
        DEBUG((EFI_D_ERROR,  "BoardRev  = %2xh\n", PlatformInfoHob->BoardRev));
    }

    if(DISABLE_PCEI_FUNCTION) {
        ///
        /// Disable PCIE function and read back to take effect
        ///
        DEBUG((EFI_D_ERROR,  "Disable PCIE function and read back to take effect!!\n"));
        MmioOr32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS, B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC0);
        MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS);  // Read back Posted Writes Register

        MmioOr32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS, B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC1);
        MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS);  // Read back Posted Writes Register

        MmioOr32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS, B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC2);
        MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS);  // Read back Posted Writes Register

        MmioOr32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS, B_PCH_PMC_FUNC_DIS_PCI_EX_FUNC3);
        MmioRead32(PMC_BASE_ADDRESS + R_PCH_PMC_FUNC_DIS);  // Read back Posted Writes Register
    }


    return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PlatformMicroSecondDelay
//
// Description: This function delays for the number of micro seconds passed in
//
// Input:       MicroSeconds Number of microseconds(us) to delay
//
// Output:      Value passed in for microseconds delay
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

UINTN
EFIAPI
PlatformMicroSecondDelay(
    IN      UINTN                     Microseconds
)
{
    UINTN   Ticks;
    UINTN   Counts;
    UINT32  CurrentTick;
    UINT32  OriginalTick;
    UINT32  RemainingTick;

    if(Microseconds == 0) {
        return Microseconds;
    }

    OriginalTick = IoRead32(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_TMR);
    OriginalTick &= (V_PCH_ACPI_PM1_TMR_MAX_VAL - 1);
    CurrentTick = OriginalTick;

    //
    // The timer frequency is 3.579545MHz, so 1 ms corresponds to 3.58 clocks
    //
    Ticks = Microseconds * 358 / 100 + OriginalTick + 1;

    //
    // The loops needed for timer overflow
    //
    Counts = (UINTN)RShiftU64((UINT64)Ticks, 24);

    //
    // Remaining clocks within one loop
    //
    RemainingTick = Ticks & 0xFFFFFF;

    //
    // Do not intend to use TMROF_STS bit of register PM1_STS, because this add extra
    // one I/O operation, and may generate SMI
    //
    while(Counts != 0) {
        CurrentTick = IoRead32(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_TMR) & B_PCH_ACPI_PM1_TMR_VAL;
        if(CurrentTick <= OriginalTick) {
            Counts--;
        }
        OriginalTick = CurrentTick;
    }

    while((RemainingTick > CurrentTick) && (OriginalTick <= CurrentTick)) {
        OriginalTick  = CurrentTick;
        CurrentTick   = IoRead32(PM_BASE_ADDRESS + R_PCH_ACPI_PM1_TMR) & B_PCH_ACPI_PM1_TMR_VAL;
    }
    return Microseconds;
}

EFI_STATUS
EFIAPI
Stall(
    IN CONST EFI_PEI_SERVICES   **PeiServices,
    IN CONST EFI_PEI_STALL_PPI      *This,
    IN UINTN              Microseconds
)
/*++

Routine Description:

  Waits for at least the given number of microseconds.

Arguments:

  PeiServices     General purpose services available to every PEIM.
  This            PPI instance structure.
  Microseconds    Desired length of time to wait.

Returns:

  EFI_SUCCESS     If the desired amount of time was passed.

--*/
{
    if(Microseconds == 0) {
        return EFI_SUCCESS;
    }
    PlatformMicroSecondDelay(Microseconds);
    return EFI_SUCCESS;
}




