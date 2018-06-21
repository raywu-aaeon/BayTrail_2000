/** @file
  SwitchableGraphics / HybridGraphics Pei driver.
  This Pei driver initialize GPIO programming for the platform.

@copyright
  Copyright (c) 2010 - 2013 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is licensed for 
  Intel CPUs and chipsets under the terms of your license agreement 
  with Intel or your vendor. This file may be modified by the user, 
  subject to additional terms of the license agreement.

**/

#include "SwitchableGraphicsInit.h"
#include <Guid/PlatformInfo.h>
#include <Library/HobLib.h>
#include <Library/DebugLib.h>    // AMI_OVERRIDE


#ifdef SG_SUPPORT

/**
  Initialize the SwitchableGraphics support (PEI).

  @param[in] PeiServices          - Pointer to the PEI services table
  @param[in] SaPlatformPolicyPpi  - SaPlatformPolicyPpi to access the GtConfig related information
**/
VOID
SwitchableGraphicsInit (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN VLV_POLICY_PPI             *VlvPolicyPpi
  )
{
  EFI_STATUS              Status;
  EFI_PEI_STALL_PPI      *StallPpi;
  EFI_PLATFORM_INFO_HOB   *PlatformInfo;
  EFI_PEI_HOB_POINTERS    Hob;
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#if defined (SgTpv_SUPPORT) && (SgTpv_SUPPORT == 1)
  UINT32                          HoldResetOffset;
  UINT32                          PowerEnableOffset;
#endif
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform <<
  //
  // Get HOB
  //
  Hob.Raw = GetFirstGuidHob (&gEfiPlatformInfoGuid);
  ASSERT (Hob.Raw != NULL);
  PlatformInfo = GET_GUID_HOB_DATA(Hob.Raw);

  if (PlatformInfo != NULL) {
    PlatformInfo->SgInfo.SgMode = VlvPolicyPpi->PlatformData.SgMode;
	DEBUG ((EFI_D_INFO, "Platform Info SG mode is %x\n", PlatformInfo->SgInfo.SgMode));	

    if (VlvPolicyPpi->PlatformData.SgMode == 2) {		// SG Mode enabled
       //
       // GPIO Assigned from policy settings, which are done from Platform package.
       //
       PlatformInfo->SgInfo.SgGpioSupport  = VlvPolicyPpi->SgGpioData->GpioSupport;

	  //
	  // VlvPolicy Settings are done in platform package as per Bayley Bay board configuration
	  // i.e., GPIO_S35 used to control PCIE Slot1 Reset signal (active Low) and
	  //       GPIO_S36 used to control PCIE Slot1 Power Enable signal (Active Low)
	  // Note: Assigning GPIOs for DGPU Card reset, Power enable, PowerOK & Prsnt are board specific and 
	  //       need to assign correctly
      PlatformInfo->SgInfo.SgDgpuHoldRst   = VlvPolicyPpi->SgGpioData->SgDgpuHoldRst->Value;
      PlatformInfo->SgInfo.SgDgpuPwrEnable = VlvPolicyPpi->SgGpioData->SgDgpuPwrEnable->Value;

	  //
	  //  dGPU PWROK & dGPU_PRSNT Signals are not controlled by GPIOs in Bayley Bay board, 
	  //  so cleared those values.
	  //
      PlatformInfo->SgInfo.SgDgpuPwrOK     = 0;
      PlatformInfo->SgInfo.SgDgpuPrsnt     = 0;

      //
      // Set Bit7 as indicator for GPIO Active Low/High
      // (Bit7 = 0 for Active Low; Bit7 = 1 for Active High signals)
	  //
      PlatformInfo->SgInfo.SgDgpuHoldRst   |= (VlvPolicyPpi->SgGpioData->SgDgpuHoldRst->Active   << 7);
      PlatformInfo->SgInfo.SgDgpuPwrEnable |= (VlvPolicyPpi->SgGpioData->SgDgpuPwrEnable->Active << 7);

      //
      // Locate PPI stall service
      //
      Status =(**PeiServices).LocatePpi (PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, &StallPpi);
      if (!EFI_ERROR (Status)) {

		//****************************************************************************
		// Note: Since in Bayley Bay board, PCIE_SLOT1_PRSNT is not controlled by GPIO
		//       and it is always enabled, So the dGPU card control is based on BIOS 
		//		 setup settings (i.e., when PrimaryVideoAdaptor is SG / PCI / Auto).
		//****************************************************************************

          //
		  //      If PCI Mode or SG Muxless, then Card is present (User to be selected only if Card is inserted)
          //              Power on MXM
          //              Configure GPIOs to drive MXM in PCI mode or SG Muxless
          //      else
          //              Do Nothing
          //
		  if ((VlvPolicyPpi->PlatformData.SgMode == SgModeMuxless)) { // ||
//              (VlvPolicyPpi->PlatformData.SgMode == SgModeDgpu)) {
			DEBUG ((EFI_D_INFO, "Configure GPIOs for driving the dGPU.\n"));

		//****************************************************************************
		// Note: Toggling GPIOs are based on selection of GPIOs. In Bayley Bay board,
		//       GPIO_S35 & GPIO_S36 are controlled through MMIO region, used MmioWrite32
		//       to access it. In case of using different GPIO which comes in IO access 
		//		 region then GpioRead/Write should be used.
		//****************************************************************************
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#if defined (SgTpv_SUPPORT) && (SgTpv_SUPPORT == 1)
			HoldResetOffset = Gpio_Platform_Offset[GPIO_dGPU_PWR_EN + SusStartOffset].offset * 16;
			PowerEnableOffset = Gpio_Platform_Offset[GPIO_dGPU_HOLD_RST + SusStartOffset].offset * 16;
			DEBUG ((EFI_D_INFO, "SG:: HoldResetOffset = %x.\n",HoldResetOffset));
			DEBUG ((EFI_D_INFO, "SG:: PowerEnableOffset = %x.\n",PowerEnableOffset));
#endif
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform <<
            //
            // Drive DGPU HOLD RST Enable to make sure we hold reset
			// GPIO_S35  (PCIE_SLOT1_RST_GPO)
            //
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#if defined (SgTpv_SUPPORT) && (SgTpv_SUPPORT == 1)
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+HoldResetOffset+0x8)& ~(ACTIVE_dGPU_HOLD_RST) );
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+HoldResetOffset+0x8)|BIT2);
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+HoldResetOffset+0x8)& (~BIT1));
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x370,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+HoldResetOffset)& 0XFFFFFFF8);
#else
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform <<
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378)& BIT0 );
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378)|BIT2);
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378)& (~BIT1));
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x370,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x370)& 0XFFFFFFF8);
#endif //AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform

            //
            // Drive DGPU PWR EN to Power On MXM
			// GPIO_S36 (PCIE_SLOT1_PWREN_GPO)
			//
//AMI_OVERRIDE  - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#if defined (SgTpv_SUPPORT) && (SgTpv_SUPPORT == 1)
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+PowerEnableOffset+0x8)& ACTIVE_dGPU_PWR_EN );
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+PowerEnableOffset+0x8)|BIT2);
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+PowerEnableOffset+0x8)& (~BIT1));
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x370,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+PowerEnableOffset)& 0XFFFFFFF8);
#else
//AMI_OVERRIDE  - EIP149863 Switchable Graphic generic support for BYT-DM platform <<
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x308,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x308)& (~BIT0) );
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x308,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x308)|BIT2);
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x308,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x308)& (~BIT1));
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x300,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x300)& 0XFFFFFFF8);
#endif //AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform
			//
            // wait 20ms
            //
            StallPpi->Stall (PeiServices, StallPpi, 20);

            //
            // Drive DGPU HOLD RST Disabled to remove reset
			// GPIO_S35 (PCIE_SLOT1_RST_GPO)
			//
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#if defined (SgTpv_SUPPORT) && (SgTpv_SUPPORT == 1)
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+HoldResetOffset+0x8)& ACTIVE_dGPU_HOLD_RST );
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+HoldResetOffset+0x8)|BIT2);
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+HoldResetOffset+0x8)& (~BIT1));
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x370,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+HoldResetOffset)& 0XFFFFFFF8);
#else
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform <<
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378)| (~BIT0) );
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378)|BIT2);
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x378)& (~BIT1));
			MmioWrite32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x370,MmioRead32(IO_BASE_ADDRESS+GPIO_SSUS_OFFSET+0x370)& 0XFFFFFFF8);
#endif //AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform
            //
            // wait 10
            //
			StallPpi->Stall (PeiServices, StallPpi, 10);
          }
      }
    }
  }
  //
  // Program SubsystemID for IGFX
  //
  DEBUG ((EFI_D_INFO, "Program SDID [Subsystem ID] for IGFX: 0x%x\n", VlvPolicyPpi->PlatformData.SgSubSystemId));
  McD2PciCfg16Or (PCI_SID, VlvPolicyPpi->PlatformData.SgSubSystemId);

}

#endif //SG_SUPPORT
