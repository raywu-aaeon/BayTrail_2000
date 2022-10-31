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
