/**
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

  @file
  ScPolicyInitPei.c

  @brief
  This file is SampleCode for Intel PCH PEI Platform Policy initialzation.

**/
#include <Library/ScPolicyInitPei.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>

//EIP191291
#if defined(PCIE_ROOT_PORT_DETECT_NON_COMPLAINT) && (PCIE_ROOT_PORT_DETECT_NON_COMPLAINT != 0)
#include <Ppi/Stall.h>
#include <Library/SbPolicy.h>
#define RETRAIN_DELAY      50
// AMI_OVERRIDE >>> 
#define R_PCH_PCIE_BNUM                              	   0x18
#define R_PCH_PCIE_SLT                                     0x1B
#define PCI_PBUS                                           0x0018        // Primary Bus Number Register
#define PCIEBRS_DEV                                        0x1c          // South Bridge PCI Express Bridge 1 // Device Number
#define PCIEBRS_BUS             		      	           0             // South Bridge PCI Express Bridge 1  // Bus Number
#define PCI_VID             				               0x0000        // Vendor ID Register
#define R_PCH_PCIE_LCTL                                    0x50
#define B_PCH_PCIE_LCTL_LD                                 BIT4
#define B_PCH_PCIE_LCTL_RL                                 BIT5
#define R_PCH_PCIE_LCTL2                                   0x70

VOID
SbPcieDetectNonComplaintPcieDevice (
  IN CONST EFI_PEI_SERVICES       **PeiServices,
  IN EFI_PEI_STALL_PPI      *StallPpi,
  IN UINT8                  Index,
  IN PCH_PCIE_CONFIG        *PcieConfig,
  IN SB_SETUP_DATA          *SbSetupData
)
{
  if ((PcieConfig->PcieSpeed[Index] == PchPcieAuto)) {
     DEBUG ((DEBUG_INFO, "Enhance Detect Non-Compliance PCIE Device @B:0|D:1C|F:%x Start .\n", Index));

     // Assign temp bus
     //DEBUG ((DEBUG_INFO, "Assign temp bus ...\n"));
     WRITE_PCI16(PCIEBRS_BUS, PCIEBRS_DEV, Index, R_PCH_PCIE_BNUM+1, 0x0101);
     
     // Do a dummy Write
     WRITE_PCI32(1, 0, 0, PCI_VID, 0x12345678);

     if (READ_PCI16(1, 0, 0, PCI_VID) == 0xFFFF) {
       //DEBUG ((DEBUG_INFO, "Can't find Device... Retrain device first.\n"));
       WRITE_PCI8(PCIEBRS_BUS, PCIEBRS_DEV, Index, R_PCH_PCIE_LCTL, B_PCH_PCIE_LCTL_LD);

       StallPpi->Stall( PeiServices, StallPpi, (RETRAIN_DELAY * 10) ); //delay 500us

       WRITE_PCI8(PCIEBRS_BUS, PCIEBRS_DEV, Index, R_PCH_PCIE_LCTL, B_PCH_PCIE_LCTL_RL);

       StallPpi->Stall( PeiServices, StallPpi, (RETRAIN_DELAY * 8000) ); //delay 400ms       

       if (READ_PCI16(1, 0, 0, PCI_VID) == 0xFFFF) {
         //DEBUG ((DEBUG_INFO, "Still can't find Device in Gen2 Speed... Speed is setted in Gen1 and delay 200 ms.\n"));
         // Set Speed to Gen1
         RW_PCI8(PCIEBRS_BUS, PCIEBRS_DEV, Index, R_PCH_PCIE_LCTL2, 0x01, 0x03);

         StallPpi->Stall( PeiServices, StallPpi, (RETRAIN_DELAY * 4000) ); //delay 200ms           

         if (READ_PCI16(1, 0, 0, PCI_VID) == 0xFFFF) {
           //DEBUG ((DEBUG_INFO, "Still can't find Device in Gen1 Speed... Retrain device again !!!\n"));
           WRITE_PCI8(PCIEBRS_BUS, PCIEBRS_DEV, Index, R_PCH_PCIE_LCTL, B_PCH_PCIE_LCTL_LD);

           StallPpi->Stall( PeiServices, StallPpi, (RETRAIN_DELAY * 10) ); //delay 500us 

           WRITE_PCI8(PCIEBRS_BUS, PCIEBRS_DEV, Index, R_PCH_PCIE_LCTL, B_PCH_PCIE_LCTL_RL);

           StallPpi->Stall( PeiServices, StallPpi, (RETRAIN_DELAY * 8000) ); //delay 400ms 

           if (READ_PCI16(1, 0, 0, PCI_VID) != 0xFFFF) PcieConfig->PcieSpeed[Index] = PchPcieGen1;
         } else PcieConfig->PcieSpeed[Index] = PchPcieGen1;
       }
     }

     // Remove temp bus
     //DEBUG ((DEBUG_INFO, "Remove temp bus.\n"));
     WRITE_PCI32(PCIEBRS_BUS, PCIEBRS_DEV, Index, PCI_PBUS, 0xFF000000);

     DEBUG ((DEBUG_INFO, "Enhance Detect Non-Compliance PCIE Device end.\n"));
  }
}
#endif
// AMI_OVERRIDE <<< 
//EIP191291


//_ModuleEntryPoint
EFI_STATUS
EFIAPI
ScPolicyInitPei(
    IN CONST EFI_PEI_SERVICES           **PeiServices,
    IN SB_SETUP_DATA                    *PchPolicyData
)
/**

  @brief
  This PEIM performs PCH PEI Platform Policy initialzation.

  @param[in] FfsHeader            Pointer to Firmware File System file header.
  @param[in] PeiServices          General purpose services available to every PEIM.

  @retval EFI_SUCCESS             The PPI is installed and initialized.
  @retval EFI ERRORS              The PPI is not successfully installed.
  @retval EFI_OUT_OF_RESOURCES    Do not have enough resources to initialize the driver

**/
{
    EFI_STATUS                    Status;
    EFI_PEI_PPI_DESCRIPTOR        *PchPlatformPolicyPpiDesc;
    PCH_PLATFORM_POLICY_PPI       *PchPlatformPolicyPpi;
    PCH_HPET_CONFIG               *HpetConfig;
    PCH_PCIE_CONFIG               *PcieConfig;
    PCH_IOAPIC_CONFIG             *IoApicConfig;
    UINT32                        SpiHsfsReg;
    UINT32                        SpiFdodReg;
    UINT8                         Index;
//EIP191291
#if defined(PCIE_ROOT_PORT_DETECT_NON_COMPLAINT) && (PCIE_ROOT_PORT_DETECT_NON_COMPLAINT != 0)
    EFI_PEI_STALL_PPI             *StallPpi;
    
    Status = (**PeiServices).LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, (VOID **) &StallPpi);
#endif
//EIP191291

    PchPlatformPolicyPpiDesc = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool(sizeof(EFI_PEI_PPI_DESCRIPTOR));
    ASSERT(PchPlatformPolicyPpiDesc != NULL);
    if(PchPlatformPolicyPpiDesc == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    PchPlatformPolicyPpi = (PCH_PLATFORM_POLICY_PPI *) AllocateZeroPool(sizeof(PCH_PLATFORM_POLICY_PPI));
    ASSERT(PchPlatformPolicyPpi != NULL);
    if(PchPlatformPolicyPpi == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    HpetConfig = (PCH_HPET_CONFIG *) AllocateZeroPool(sizeof(PCH_HPET_CONFIG));
    ASSERT(HpetConfig != NULL);
    if(HpetConfig == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    PcieConfig = (PCH_PCIE_CONFIG *) AllocateZeroPool (sizeof (PCH_PCIE_CONFIG));
    ASSERT (PcieConfig != NULL);
    if (PcieConfig == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    IoApicConfig = (PCH_IOAPIC_CONFIG *) AllocateZeroPool(sizeof(PCH_IOAPIC_CONFIG));
    ASSERT(IoApicConfig != NULL);
    if(IoApicConfig == NULL) {
        return EFI_OUT_OF_RESOURCES;
    }

    PchPlatformPolicyPpi->Revision                = PCH_PLATFORM_POLICY_PPI_REVISION_1;
    PchPlatformPolicyPpi->BusNumber               = DEFAULT_PCI_BUS_NUMBER_PCH;
    PchPlatformPolicyPpi->SpiBase                 = SPI_BASE_ADDRESS;
    PchPlatformPolicyPpi->PmcBase                 = PMC_BASE_ADDRESS;
    PchPlatformPolicyPpi->IoBase                  = IO_BASE_ADDRESS;
    PchPlatformPolicyPpi->IlbBase                 = ILB_BASE_ADDRESS;
    PchPlatformPolicyPpi->PUnitBase                 = PUNIT_BASE_ADDRESS;
    PchPlatformPolicyPpi->Rcba                    	= SB_RCBA;
    PchPlatformPolicyPpi->MphyBase                = MPHY_BASE_ADDRESS;
    PchPlatformPolicyPpi->AcpiBase                = PM_BASE_ADDRESS;
    PchPlatformPolicyPpi->GpioBase                = GPIO_BASE_ADDRESS;
    PchPlatformPolicyPpi->SataMode                = PchPolicyData->SataInterfaceMode;
    PchPlatformPolicyPpi->XhciWorkaroundSwSmiNumber = (PchPolicyData->OsSelect == 1) ? 0 : XHCI_WORKAROUND_SW_SMI; //EIP171355
    PchPlatformPolicyPpi->HpetConfig              = HpetConfig;
    PchPlatformPolicyPpi->PcieConfig              = PcieConfig;
    PchPlatformPolicyPpi->IoApicConfig            = IoApicConfig;
    PchPlatformPolicyPpi->EhciPllCfgEnable        = PchPolicyData->EhciPllCfgEnable;
    PchPlatformPolicyPpi->SataOddPort			  = PchPolicyData->SataOddPort; //EIP149024
    HpetConfig->Enable = PchPolicyData->HpetEnable;
    HpetConfig->Base = HPET_BASE_ADDRESS;
    
    IoApicConfig->IoApicId = 0x01;    //  PchPolicyData->IoApicId or by Token.

  for (Index = 0; Index < PCH_PCIE_MAX_ROOT_PORTS; Index++) {
    PcieConfig->PcieSpeed[Index] = PchPolicyData->PcieRootPortSpeed[Index];
//EIP191291
//Enhance Detect Non-Compliance PCIE Device
#if defined(PCIE_ROOT_PORT_DETECT_NON_COMPLAINT) && (PCIE_ROOT_PORT_DETECT_NON_COMPLAINT != 0)
       if(!EFI_ERROR(Status)){
          if ((PchPolicyData->PcieRPDetectNonComplaint[Index] == 1) && (PchPolicyData->PcieRootPortEn[0] != 0))
             SbPcieDetectNonComplaintPcieDevice(PeiServices, StallPpi, Index, PcieConfig, PchPolicyData);    
        }
#endif
//EIP191291
  }


  SpiHsfsReg = READ_MMIO32 (SPI_BASE_ADDRESS + R_PCH_SPI_HSFS);
  if ((SpiHsfsReg & B_PCH_SPI_HSFS_FDV) == B_PCH_SPI_HSFS_FDV) {
    WRITE_MMIO32 (SPI_BASE_ADDRESS + R_PCH_SPI_FDOC, V_PCH_SPI_FDOC_FDSS_FSDM);
    SpiFdodReg = READ_MMIO32 (SPI_BASE_ADDRESS + R_PCH_SPI_FDOD);
    if (SpiFdodReg == V_PCH_SPI_FDBAR_FLVALSIG) {
//      PchPlatformPolicyPpi->EnableGbe = PchPolicyData->Lan;	// policy "EnableGbe" removed in RC 0.6.0.
    }
  }

    PchPlatformPolicyPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
    PchPlatformPolicyPpiDesc->Guid = &gPchPlatformPolicyPpiGuid;
    PchPlatformPolicyPpiDesc->Ppi = PchPlatformPolicyPpi;

    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->Revision :%x \n",PchPlatformPolicyPpi->Revision));
    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->BusNumber :%x \n",PchPlatformPolicyPpi->BusNumber));
    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->SpiBase :%x \n",PchPlatformPolicyPpi->SpiBase));
    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->PmcBase :%x \n",PchPlatformPolicyPpi->PmcBase));
    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->IoBase :%x \n",PchPlatformPolicyPpi->IoBase));
    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->IlbBase :%x \n",PchPlatformPolicyPpi->IlbBase));
    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->PUnitBase :%x \n",PchPlatformPolicyPpi->PUnitBase));
    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->Rcba :%x \n",PchPlatformPolicyPpi->Rcba));
    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->MphyBase :%x \n",PchPlatformPolicyPpi->MphyBase));
    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->AcpiBase :%x \n",PchPlatformPolicyPpi->AcpiBase));
    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->GpioBase :%x \n",PchPlatformPolicyPpi->GpioBase));
    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->HpetConfig :%x \n",PchPlatformPolicyPpi->HpetConfig));
    DEBUG((EFI_D_ERROR, "PchPlatformPolicyPpi->IoApicConfig :%x \n",PchPlatformPolicyPpi->IoApicConfig));

    //
    // Install PCH Platform Policy PPI
    //
    Status = (**PeiServices).InstallPpi(PeiServices, PchPlatformPolicyPpiDesc);
    ASSERT_EFI_ERROR(Status);

    return Status;
}
