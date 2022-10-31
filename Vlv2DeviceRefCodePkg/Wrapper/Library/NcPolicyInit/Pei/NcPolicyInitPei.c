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
  NcPolicyInitPei.c

  @brief
  This file is SampleCode for Intel PCH PEI Platform Policy initialzation.

**/
#include <Library/NcPolicyInitPei.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
//_ModuleEntryPoint
EFI_STATUS
EFIAPI
NcPolicyInitPei(
    IN CONST EFI_PEI_SERVICES           **PeiServices,
    IN NB_SETUP_DATA                    *VlvPolicyData
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
    EFI_STATUS                      Status;
    EFI_PEI_PPI_DESCRIPTOR          *mVlvPolicyPpiDesc;
    VLV_POLICY_PPI                  *mVlvPolicyPpi;

    Status = (*PeiServices)->AllocatePool(PeiServices, sizeof(EFI_PEI_PPI_DESCRIPTOR), &mVlvPolicyPpiDesc);
    ASSERT_EFI_ERROR(Status);

    Status = (*PeiServices)->AllocatePool(PeiServices, sizeof(VLV_POLICY_PPI), &mVlvPolicyPpi);
    ASSERT_EFI_ERROR(Status);

    //
    // Initialize PPI
    //
    (*PeiServices)->SetMem((VOID *)mVlvPolicyPpi, sizeof(VLV_POLICY_PPI), 0);
    mVlvPolicyPpiDesc->Flags = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
    mVlvPolicyPpiDesc->Guid = &gVlvPolicyPpiGuid;
    mVlvPolicyPpiDesc->Ppi = mVlvPolicyPpi;

    /// SwitchableGraphics Policy update from Setup Configuration
    /// GPIO support exists for both Mobile & Desktop platforms
    mVlvPolicyPpi->SgGpioData->GpioSupport = 1;

    /// For SG mode dGPU PWR Enable is Active Low (When Low - Power given to card, when High - No power)
    mVlvPolicyPpi->SgGpioData->SgDgpuPwrEnable->Active = 0;

    /// In Switchable Gfx mode PEG needs to be always enabled
    /// and IGFX must be set as Primary Display.
    if (VlvPolicyData->PrimaryDisplay == 4) {
      mVlvPolicyPpi->PlatformData.SgMode        = 2; //SgModeMuxless
      mVlvPolicyPpi->PlatformData.SgSubSystemId = 0x2112;
      mVlvPolicyPpi->GtConfig.PrimaryDisplay    = 0;    // This already updated, can be removed later
      DEBUG ((EFI_D_ERROR, "SG Mode selected, Primary display is IGD"));
    } else if ((VlvPolicyData->PrimaryDisplay == 1) || (VlvPolicyData->PrimaryDisplay == 2) || (VlvPolicyData->PrimaryDisplay == 3)) {
      /// In PCI or Auto mode set Switchable Gfx mode as dGPU
      mVlvPolicyPpi->PlatformData.SgMode        = 3; //SgModeDgpu
      mVlvPolicyPpi->PlatformData.SgSubSystemId = 0x2212;
      mVlvPolicyPpi->GtConfig.PrimaryDisplay = VlvPolicyData->PrimaryDisplay;
    } else if (VlvPolicyData->PrimaryDisplay == 0) {
      /// In IGFX only mode mode set Switchable Gfx mode as Disabled
      mVlvPolicyPpi->PlatformData.SgMode        = 0; //SgModeDisabled
      mVlvPolicyPpi->PlatformData.SgSubSystemId = 0x2212;
      mVlvPolicyPpi->GtConfig.PrimaryDisplay = VlvPolicyData->PrimaryDisplay;
    }

    mVlvPolicyPpi->GtConfig.PAVPMode = VlvPolicyData->PavpMode;
    mVlvPolicyPpi->GtConfig.IgdDvmt50PreAlloc = VlvPolicyData->IgdDvmt50PreAlloc; //1-32M , 2-64M , 4-128M
    mVlvPolicyPpi->GtConfig.ApertureSize = VlvPolicyData->ApertureSize; //1-128M , 2-256M , 3-512M
    mVlvPolicyPpi->GtConfig.GttSize = VlvPolicyData->GttSize; //1-1M , other-2M
    mVlvPolicyPpi->GtConfig.InternalGraphics = VlvPolicyData->InternalGraphics; //1-IGD_ENABLE , 0-IGD_DISABLE
    mVlvPolicyPpi->GtConfig.IgdTurboEn = VlvPolicyData->IgdTurboEn; //1-IGDTurboEn_ENABLE , 0-IGDTurboEn_DISABLE

    mVlvPolicyPpi->PlatformData.FastBoot = VlvPolicyData->FastBoot; //Not to be used now.
    mVlvPolicyPpi->PlatformData.DynSR = VlvPolicyData->DynSR;       //Not to be used now.

    mVlvPolicyPpi->ISPEn                      = VlvPolicyData->ISPEn;
    mVlvPolicyPpi->ISPPciDevConfig           = VlvPolicyData->ISPDevSel;
    mVlvPolicyPpi->S0ixEn           = VlvPolicyData->S0ixSupported;
    
    DEBUG((EFI_D_ERROR, "mVlvPolicyPpi->GtConfig.GttSize: 0x%x\n", mVlvPolicyPpi->GtConfig.GttSize));
    DEBUG((EFI_D_ERROR, "mVlvPolicyPpi->GtConfig.IgdDvmt50PreAlloc: 0x%x\n", mVlvPolicyPpi->GtConfig.IgdDvmt50PreAlloc));
    DEBUG((EFI_D_ERROR, "mVlvPolicyPpi->GtConfig.PrimaryDisplay: 0x%x\n", mVlvPolicyPpi->GtConfig.PrimaryDisplay));
    DEBUG((EFI_D_ERROR, "mVlvPolicyPpi->GtConfig.PAVPMode: 0x%x\n", mVlvPolicyPpi->GtConfig.PAVPMode));
    DEBUG((EFI_D_ERROR, "mVlvPolicyPpi->GtConfig.ApertureSize: 0x%x\n", mVlvPolicyPpi->GtConfig.ApertureSize));
    DEBUG((EFI_D_ERROR, "mVlvPolicyPpi->GtConfig.InternalGraphics: 0x%x\n", mVlvPolicyPpi->GtConfig.InternalGraphics));
    DEBUG((EFI_D_ERROR, "mVlvPolicyPpi->GtConfig.IgdTurboEn: 0x%x\n", mVlvPolicyPpi->GtConfig.IgdTurboEn));

    DEBUG((EFI_D_ERROR, "mVlvPolicyPpi->PlatformData.FastBoot: 0x%x\n", mVlvPolicyPpi->PlatformData.FastBoot));
    DEBUG((EFI_D_ERROR, "mVlvPolicyPpi->PlatformData.DynSR: 0x%x\n", mVlvPolicyPpi->PlatformData.DynSR));
    DEBUG((EFI_D_ERROR, "mVlvPolicyPpi->ISPEn: 0x%x\n", mVlvPolicyPpi->ISPEn));
    DEBUG((EFI_D_ERROR, "mVlvPolicyPpi->ISPPciDevConfig: 0x%x\n",mVlvPolicyPpi->ISPPciDevConfig));

    DEBUG((EFI_D_ERROR, "mVlvPolicyPpi->S0ixEn: 0x%x\n",mVlvPolicyPpi->S0ixEn));

    Status = (*PeiServices)->InstallPpi(PeiServices, mVlvPolicyPpiDesc);
    ASSERT_EFI_ERROR(Status);

    return Status;
}
