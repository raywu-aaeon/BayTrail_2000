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
  PchiPolicyInitPei.c

  @brief
  This file is SampleCode for Intel PCH PEI Platform Policy initialzation.

**/
#include "PchPolicyInitPei.h"
#ifdef ECP_FLAG
EFI_GUID gPchPlatformPolicyPpiGuid = PCH_PLATFORM_POLICY_PPI_GUID;
#else
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PeimEntryPoint.h>
#endif
//_ModuleEntryPoint
EFI_STATUS
EFIAPI
PchPolicyInitPeiEntryPoint (
#ifdef ECP_FLAG
  IN  EFI_FFS_FILE_HEADER    *FileHandle,
  IN  EFI_PEI_SERVICES       **PeiServices
#else
  IN       EFI_PEI_FILE_HANDLE       FileHandle,
  IN CONST EFI_PEI_SERVICES          **PeiServices
#endif
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

  PchPlatformPolicyPpiDesc = (EFI_PEI_PPI_DESCRIPTOR *) AllocateZeroPool (sizeof (EFI_PEI_PPI_DESCRIPTOR));
  ASSERT (PchPlatformPolicyPpiDesc != NULL);
  if (PchPlatformPolicyPpiDesc == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  PchPlatformPolicyPpi = (PCH_PLATFORM_POLICY_PPI *) AllocateZeroPool (sizeof (PCH_PLATFORM_POLICY_PPI));
  ASSERT (PchPlatformPolicyPpi != NULL);
  if (PchPlatformPolicyPpi == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  HpetConfig = (PCH_HPET_CONFIG *) AllocateZeroPool (sizeof (PCH_HPET_CONFIG));
  ASSERT (HpetConfig != NULL);
  if (HpetConfig == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  PcieConfig = (PCH_PCIE_CONFIG *) AllocateZeroPool (sizeof (PCH_PCIE_CONFIG));
  ASSERT (PcieConfig != NULL);
  if (PcieConfig == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  IoApicConfig = (PCH_IOAPIC_CONFIG *) AllocateZeroPool (sizeof (PCH_IOAPIC_CONFIG));
  ASSERT (IoApicConfig != NULL);
  if (IoApicConfig == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  PchPlatformPolicyPpi->Revision                = PCH_PLATFORM_POLICY_PPI_REVISION_1;
  PchPlatformPolicyPpi->BusNumber               = 0;
  PchPlatformPolicyPpi->SpiBase                 = 0xFED01000;
  PchPlatformPolicyPpi->PmcBase                 = 0xFED03000;
  PchPlatformPolicyPpi->IoBase                  = 0xFED0C000;
  PchPlatformPolicyPpi->IlbBase                 = 0xFED08000;
  PchPlatformPolicyPpi->Rcba                    = 0xFED1C000;
  PchPlatformPolicyPpi->MphyBase                = 0xFEA00000;
  PchPlatformPolicyPpi->AcpiBase                = 0x400;
  PchPlatformPolicyPpi->GpioBase                = 0x500;
  PchPlatformPolicyPpi->EnableGbe               = 1;
  PchPlatformPolicyPpi->SataMode                = 0;

  PchPlatformPolicyPpi->HpetConfig              = HpetConfig;
  PchPlatformPolicyPpi->PcieConfig              = PcieConfig;
  PchPlatformPolicyPpi->IoApicConfig            = IoApicConfig;
  HpetConfig->Enable = 1;
  HpetConfig->Base = 0xFED00000;

  PcieConfig->PcieSpeed[0] = PchPcieAuto;
  PcieConfig->PcieSpeed[1] = PchPcieAuto;
  PcieConfig->PcieSpeed[2] = PchPcieAuto;
  PcieConfig->PcieSpeed[3] = PchPcieAuto;
  IoApicConfig->IoApicId = 0x01;

  PchPlatformPolicyPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  PchPlatformPolicyPpiDesc->Guid = &gPchPlatformPolicyPpiGuid;
  PchPlatformPolicyPpiDesc->Ppi = PchPlatformPolicyPpi;

  //
  // Install PCH Platform Policy PPI
  //
  Status = (**PeiServices).InstallPpi (PeiServices, PchPlatformPolicyPpiDesc);
  ASSERT_EFI_ERROR (Status);

  return Status;
}
