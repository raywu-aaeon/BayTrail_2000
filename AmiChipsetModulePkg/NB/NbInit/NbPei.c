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
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************


//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        NbPei.c
//
// Description: This file contains code for Template Northbridge initialization
//              in the PEI stage
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


// Module specific Includes
#include <Efi.h>
#include <Pei.h>
#include <token.h>
#include <StatusCodes.h>
#include "MchRegs.h" //CSP20140329_22
#include <PCI.h>
#include <Nb.h>
#include <AmiLib.h>
#include <AmiCspLib.h> //EIP158981
#include <Library/NbPolicy.h>
#include <AmiPeiLib.h>
#include <PPI/CspLibPPI.h>
#include <Library/NbCspLib.h>
#include <Library/HobLib.h>
#include <AmiChipsetIoLib.h>
#include <Ppi/PciCfg.h>
#include <Ppi/CpuIo.h>
#include <Ppi/NbPpi.h>
#include <platformBaseAddresses.h>
#include <PchRegs.h>
#include "PchAccess.h"
#include <CpuRegs.h>

extern  AMI_PEI_PCI_INIT_TABLE_STRUCT   stNBH2P_PCIInitTable [];
extern  UINT16                          wNBH2P_PCIInitTableSize;
// Produced PPIs

// GUID Definitions

// Portable Constants

typedef struct {
    UINT32  RegEax;
    UINT32  RegEbx;
    UINT32  RegEcx;
    UINT32  RegEdx;
} EFI_CPUID_REGISTER;

// Function Prototypes
extern EFI_STATUS SetPeiCacheMode(IN  CONST EFI_PEI_SERVICES    **PeiServices);
extern EFI_STATUS PublishMemoryTypeInfo(IN  CONST EFI_PEI_SERVICES    **PeiServices);

// PPI interface definition

static AMI_PEI_NBINIT_POLICY_PPI mAmiPeiNbInitPolicyPpi = {
    NULL
};

// PPI that are installed

/** PORTING **
    Include any additional PPI needed for memory detection in this
    list and define the functions in this file
    **/
static EFI_PEI_PPI_DESCRIPTOR mPpiList[] = {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gAmiPeiNbInitPolicyGuid,
    &mAmiPeiNbInitPolicyPpi
};

#define NB_DEVICE_NUMBER        0
#define TempBusDevFuncNo        (PEI_PCI_CFG_ADDRESS(0, NB_DEVICE_NUMBER, 0, 0))

// Function Definition

EFI_STATUS
VlvPlatformInit(
    IN CONST EFI_PEI_SERVICES             **PeiServices,
    IN NB_SETUP_DATA                      *VlvPolicyData
);

EFI_STATUS NbPeiInitAfterMemInstalled(
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *InvokePpi
);

EFI_STATUS ProgramNbRegBeforeEndofPei(
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *InvokePpi
);

// PPI that are notified

static EFI_PEI_NOTIFY_DESCRIPTOR MemInstalledNotifyList[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | \
        EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST, \
        &gEfiPeiMemoryDiscoveredPpiGuid, NbPeiInitAfterMemInstalled
    },
};

static EFI_PEI_NOTIFY_DESCRIPTOR mNotifyList[] = {
    {
        EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | \
        EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST, \
        &gEfiEndOfPeiSignalPpiGuid, ProgramNbRegBeforeEndofPei
    },
};

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   NbPeiInit
//
// Description: This function is the entry point for this PEI. This function
//              initializes the chipset NB
//
// Input:       FileHandle      Handle of the file being invoked
//              **PeiServices   Pointer to the PEI services table
//
// Output:      Return Status based on errors that occurred while waiting for
//              time to expire.
//
// Notes:       This function should initialize North Bridge for memory detection.
//              Install AMI_PEI_NBINIT_POLICY_PPI to indicate that NB Init PEIM
//              is installed
//              Following things can be done at this point:
//                  - Enabling top of 4GB decode for full flash ROM
//                  - Programming North Bridge ports to enable access to South
//                    Bridge and other I/O bridge access
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS
EFIAPI
NbPeiInit(
    IN       EFI_PEI_FILE_HANDLE  FileHandle,
    IN CONST EFI_PEI_SERVICES     **PeiServices
)
{

    EFI_STATUS                  Status;
    EFI_HOB_CPU                 *pCpu = NULL;
    NB_SETUP_DATA               VlvPolicyData;
//    EFI_PEI_PCI_CFG2_PPI        *PciCfg;

    // Report Progress code
    PEI_PROGRESS_CODE(PeiServices, PEI_CAR_NB_INIT);

    // Install the NB Init Policy PPI
    Status = (*PeiServices)->InstallPpi(PeiServices, &mPpiList[0]);
    ASSERT_PEI_ERROR(PeiServices, Status);

    // Get pointer to the PCI config PPI
//    PciCfg = (*PeiServices)->PciCfg;

    // Get the value of the NB Setup data.
    GetNbSetupData((VOID *)PeiServices, &VlvPolicyData, TRUE);

    //EIP158981 >>
    if(ReadCmos(CMOS_PS2_VCC_VNN_CONTROL) != VlvPolicyData.EnablePS2ForVccVnn) {
    	WriteCmos(CMOS_PS2_VCC_VNN_CONTROL, VlvPolicyData.EnablePS2ForVccVnn);
    }
    //EIP158981 <<	
	
    // Create CPU HOB with appropriate memory space size and IO space size
    Status = (*PeiServices)->CreateHob(PeiServices, EFI_HOB_TYPE_CPU, sizeof(EFI_HOB_CPU), &pCpu);
    if(Status == EFI_SUCCESS) {
        pCpu->SizeOfMemorySpace = 36;       // Maximum address space supported by the CPU/Chipset
        pCpu->SizeOfIoSpace     = 16;
        pCpu->Reserved[0] = pCpu->Reserved[1] = pCpu->Reserved[2] = pCpu->Reserved[3] =
                pCpu->Reserved[4] = pCpu->Reserved[5] = 0;
    }

    // Do basic Vlv init
    Status = VlvPlatformInit(PeiServices, &VlvPolicyData);
    ASSERT_PEI_ERROR(PeiServices, Status);

    // Setup a NBPEI entry after PEI permantent memory be installed
    Status = (*PeiServices)->NotifyPpi(PeiServices, MemInstalledNotifyList);
    ASSERT_PEI_ERROR(PeiServices, Status);

    return EFI_SUCCESS;

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure: ProgramNbSubId
//
// Description: This function programs NB PCI devices sub-vendor ID and
//              sub-system ID.
//
// Input:
//  EFI_PEI_SERVICES **PeiServices - Pointer to the PEI services table
//  EFI_PEI_PCI_CFG2_PPI *PciCfg - Pointer to the PCI Configuration PPI
//
// Output:      VOID
//
// Notes:       1. This routine only programs the PCI device in NB, hence, we
//                 have to check the bus/device/function numbers whether they
//                 are a NB PCI device or not.
//              2. This routine is invoked by PEI phase.(After PEI permantent
//                 memory be installed)
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

VOID ProgramNbSubId(
    IN EFI_PEI_SERVICES         **PeiServices,
    IN EFI_PEI_PCI_CFG2_PPI     *PciCfg)
{
    EFI_STATUS                    Status;
    AMI_PEI_NB_CUSTOM_PPI         *NbPeiOemPpi;
    UINTN                         i = 0;
    UINT32                        PciSid = 0xffffffff;
    AMI_NB_PCI_SSID_TABLE_STRUCT  DefaultSidTbl[] = {NB_PCI_DEVICES_SSID_TABLE};
    AMI_NB_PCI_SSID_TABLE_STRUCT  *SsidTblPtr = DefaultSidTbl;

    Status = (*PeiServices)->LocatePpi(PeiServices, \
                                       &gAmiPeiNbCustomPpiGuid, \
                                       0, \
                                       NULL, \
                                       &NbPeiOemPpi);

    if(Status == EFI_SUCCESS) {
        if((NbPeiOemPpi->SsidTable != NULL) && (NbPeiOemPpi->SsidTable[0].PciAddr != 0xffffffffffffffff))
            SsidTblPtr = NbPeiOemPpi->SsidTable;
    }

    // Porting Start
    while(SsidTblPtr[i].PciAddr != 0xffffffffffffffff) {
        if(SsidTblPtr[i].PciAddr == (NB_BUS_DEV_FUN) ||
                SsidTblPtr[i].PciAddr == (AMI_IGD_BUS_DEV_FUN)) { //CSP20140329_22
            if(SsidTblPtr[i].Sid == 0xffffffff) {
                PciCfg->Read(PeiServices,
                                      PciCfg,
                                      EfiPeiPciCfgWidthUint32,
                                      SsidTblPtr[i].PciAddr,
                                      &PciSid);
            } else {
                PciSid = SsidTblPtr[i].Sid;
            }

            PciCfg->Write(PeiServices,
                                   PciCfg,
                                   EfiPeiPciCfgWidthUint32,
                                   SsidTblPtr[i].PciAddr | PCI_SVID,
                                   &PciSid);

        }

        i++;
    }
    // Porting End

}

VOID
CheckPowerOffNow(
    VOID
)
/*++

Routine Description:

  Turn off system if needed.

Arguments:

  PeiServices Pointer to PEI Services
  CpuIo       Pointer to CPU I/O Protocol

Returns:

  None.

--*/
{
    UINT16  Pm1Sts;

    //
    // Read and check the ACPI registers
    //
    Pm1Sts = IoRead16(ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_STS);
    if((Pm1Sts & B_PCH_ACPI_PM1_STS_PWRBTN) == B_PCH_ACPI_PM1_STS_PWRBTN) {
        IoWrite16(ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_STS, B_PCH_ACPI_PM1_STS_PWRBTN);
        IoWrite16(ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT, V_PCH_ACPI_PM1_CNT_S5);
        IoWrite16(ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT, V_PCH_ACPI_PM1_CNT_S5 + B_PCH_ACPI_PM1_CNT_SLP_EN);
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   NbPeiInitAfterMemInstalled
//
// Description: This function can be used to program any NB regisater after
//              PEI permantent memory be installed.
//
// Input:       PeiServices      - Pointer to the PEI services table
//              NotifyDescriptor - Pointer to the descriptor for the
//                                 notification event.
//              InvokePpi        - Pointer to the PPI that was installed
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS NbPeiInitAfterMemInstalled(
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *InvokePpi)
{
    EFI_STATUS                  Status;
//    EFI_PEI_CPU_IO_PPI          *CpuIo;
    EFI_PEI_PCI_CFG2_PPI        *PciCfg;
    EFI_BOOT_MODE               BootMode;
    UINT16                                 Pm1Cnt;
    UINT32                      RootComplexBar;
    UINT32                      PmcBase;
    UINT32                      IoBase;
    UINT32                      IlbBase;
    UINT32                      SpiBase;
    UINT32                      MphyBase;
    EFI_CPUID_REGISTER          FeatureInfo;
    UINT8                       CpuAddressWidth;

    //CpuIo = (*PeiServices)->CpuIo;
    PciCfg = (*PeiServices)->PciCfg;

    PEI_PROGRESS_CODE(PeiServices, PEI_MEM_NB_INIT);
    PEI_TRACE((-1, PeiServices,  "NbPeiInitAfterMemInstalled\n"));
    // Program NB Devices' Subsystem Vendor ID & Subsystem ID
    ProgramNbSubId(PeiServices, PciCfg);

    Status = (*PeiServices)->GetBootMode(PeiServices, &BootMode);

    //
    // Check if user wants to turn off in PEI phase
    //
    if((BootMode != BOOT_ON_S3_RESUME) && (BootMode != BOOT_ON_FLASH_UPDATE)) {
        CheckPowerOffNow();
    } else {
        Pm1Cnt  = IoRead16(ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT);
        Pm1Cnt &= ~B_PCH_ACPI_PM1_CNT_SLP_TYP;
        IoWrite16(ACPI_BASE_ADDRESS + R_PCH_ACPI_PM1_CNT, Pm1Cnt);
    }

    //
    // Set PEI cache mode here
    //
// (EIP127918-)>>
//  SetPeiCacheMode(PeiServices);   //Cache is set in MemoryCallback so no need to set here.
// (EIP127918-)<<

    //  Publish memory type info
    PublishMemoryTypeInfo (PeiServices); // EIP128872

    // Set up necessary PPI notifications after PEI permantent memory be installed
    Status = (*PeiServices)->NotifyPpi(PeiServices, &mNotifyList[0]);
    ASSERT_PEI_ERROR(PeiServices, Status);

    //
    // Work done if on a S3 resume
    //
    if(BootMode == BOOT_ON_S3_RESUME) {
        //
        //Program the side band packet register to send a sideband message to Punit
        //To indicate that DRAM has been initialized and PUNIT FW base address in memory.
        //
        //MsgBusCmdWrite(CDV_UNIT_PUNIT, dPUnitBase);
        return EFI_SUCCESS;
    }

    RootComplexBar = MmioRead32(MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, 0, R_PCH_LPC_RCBA)) & B_PCH_LPC_RCBA_BAR;

    BuildResourceDescriptorHob(
        EFI_RESOURCE_MEMORY_MAPPED_IO,
        (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
        RootComplexBar,
        0x1000
    );
    PEI_TRACE((-1, PeiServices,  "RootComplexBar     : 0x%x\n", RootComplexBar));

    PmcBase =  MmioRead32(MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, 0, R_PCH_LPC_PMC_BASE)) & B_PCH_LPC_PMC_BASE_BAR;
    BuildResourceDescriptorHob(
        EFI_RESOURCE_MEMORY_MAPPED_IO,
        (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
        PmcBase,
        0x1000
    );
    PEI_TRACE((-1, PeiServices,  "PmcBase            : 0x%x\n", PmcBase));

    IoBase =  MmioRead32(MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, 0, R_PCH_LPC_IO_BASE)) & B_PCH_LPC_IO_BASE_BAR;
    BuildResourceDescriptorHob(
        EFI_RESOURCE_MEMORY_MAPPED_IO,
        (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
        IoBase,
        0x4000
    );
    PEI_TRACE((-1, PeiServices,  "IoBase             : 0x%x\n", IoBase));

    IlbBase =  MmioRead32(MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, 0, R_PCH_LPC_ILB_BASE)) & B_PCH_LPC_ILB_BASE_BAR;
    BuildResourceDescriptorHob(
        EFI_RESOURCE_MEMORY_MAPPED_IO,
        (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
        IlbBase,
        0x1000
    );
    PEI_TRACE((-1, PeiServices,  "IlbBase            : 0x%x\n", IlbBase));

    SpiBase =  MmioRead32(MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, 0, R_PCH_LPC_SPI_BASE)) & B_PCH_LPC_SPI_BASE_BAR;
    BuildResourceDescriptorHob(
        EFI_RESOURCE_MEMORY_MAPPED_IO,
        (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
        SpiBase,
        0x1000
    );
    PEI_TRACE((-1, PeiServices,  "SpiBase            : 0x%x\n", SpiBase));

    MphyBase = MmioRead32(MmPciAddress(0, DEFAULT_PCI_BUS_NUMBER_PCH, PCI_DEVICE_NUMBER_PCH_LPC, 0, R_PCH_LPC_MPHY_BASE)) & B_PCH_LPC_MPHY_BASE_BAR;
    BuildResourceDescriptorHob(
        EFI_RESOURCE_MEMORY_MAPPED_IO,
        (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
        MphyBase,
        0x100000
    );
    PEI_TRACE((-1, PeiServices,  "MphyBase           : 0x%x\n", MphyBase));

    // Local APIC
    BuildResourceDescriptorHob(
        EFI_RESOURCE_MEMORY_MAPPED_IO,
        (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
        LOCAL_APIC_ADDRESS,
        0x1000
    );
    PEI_TRACE((-1, PeiServices,  "LOCAL_APIC_ADDRESS : 0x%x\n", LOCAL_APIC_ADDRESS));

    // IO APIC
    BuildResourceDescriptorHob(
        EFI_RESOURCE_MEMORY_MAPPED_IO,
        (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
        IO_APIC_ADDRESS,
        0x1000
    );
    PEI_TRACE((-1, PeiServices,  "IO_APIC_ADDRESS    : 0x%x\n", IO_APIC_ADDRESS));

    // Adding the PCIE Express area to the E820 memory table as type 2 memory.
    BuildResourceDescriptorHob(
        EFI_RESOURCE_MEMORY_MAPPED_IO,
        (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
        PCIEX_BASE_ADDRESS,
        PCIEX_LENGTH
    );
    PEI_TRACE((-1, PeiServices,  "PciExpressBase     : 0x%x\n", PCIEX_BASE_ADDRESS));

    // Adding the Flashpart to the E820 memory table as type 2 memory.
    BuildResourceDescriptorHob(
        EFI_RESOURCE_FIRMWARE_DEVICE,
        (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
        FLASH_BASE_ADDRESS,
        FLASH_SIZE
    );
    PEI_TRACE((-1, PeiServices,  "FLASH_BASE_ADDRESS : 0x%x\n", FLASH_BASE_ADDRESS));

    //
    // Create a CPU hand-off information
    //
    CpuAddressWidth = 32;
    AsmCpuid(EFI_CPUID_EXTENDED_FUNCTION, &FeatureInfo.RegEax, &FeatureInfo.RegEbx, &FeatureInfo.RegEcx, &FeatureInfo.RegEdx);
    if(FeatureInfo.RegEax >= EFI_CPUID_VIRT_PHYS_ADDRESS_SIZE) {
        AsmCpuid(EFI_CPUID_VIRT_PHYS_ADDRESS_SIZE, &FeatureInfo.RegEax, &FeatureInfo.RegEbx, &FeatureInfo.RegEcx, &FeatureInfo.RegEdx);
        CpuAddressWidth = (UINT8)(FeatureInfo.RegEax & 0xFF);
    }
    BuildCpuHob(CpuAddressWidth, 16);

    return  EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   ProgramNbRegBeforeEndofPei
//
// Description: This function can be used to program any NB regisater before
//              end of PEI phase.
//
// Input:       PeiServices      - Pointer to the PEI services table
//              NotifyDescriptor - Pointer to the descriptor for the
//                                 notification event.
//              InvokePpi        - Pointer to the PPI that was installed
//
// Output:      EFI_STATUS
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS ProgramNbRegBeforeEndofPei(
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *InvokePpi)
{
// (EIP127918+)>>
    EFI_STATUS          Status = EFI_SUCCESS;
    EFI_BOOT_MODE       BootMode;

    Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);

    if (BootMode == BOOT_ON_S3_RESUME) {
      // Porting if needed.
    } else {
      // Porting if needed.
      PEI_TRACE((-1, PeiServices,  "SetPeiCacheMode Start\n"));
      SetPeiCacheMode(PeiServices);
      PEI_TRACE((-1, PeiServices,  "SetPeiCacheMode End\n"));
    }
// (EIP127918+)<<
    return  EFI_SUCCESS;
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
