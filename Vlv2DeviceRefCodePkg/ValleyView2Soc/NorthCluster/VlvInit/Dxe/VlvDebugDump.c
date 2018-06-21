/**
  This file contains an 'Intel Peripheral Driver' and uniquely        
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your   
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the
  license agreement
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
  PchDebugDump.c
  
  @brief 
  Dump whole DXE_PCH_PLATFORM_POLICY_PROTOCOL and serial out. 

**/
#include "VlvInit.h"

VOID
VlvDumpPlatformProtocol (
  IN  DXE_VLV_PLATFORM_POLICY_PROTOCOL    *PlatformSaPolicy
  )
/**

  @brief 
  Dump whole DXE_PCH_PLATFORM_POLICY_PROTOCOL and serial out.

  @param[in] PchPlatformPolicy    The PCH Platform Policy protocol instance

  @retval None

**/
{
  DEBUG ((EFI_D_INFO, "\n"));
  DEBUG ((EFI_D_INFO, "------------------------ VLV Dump platform protocol Start -----------------\n"));
  DEBUG ((EFI_D_INFO, " Revision= %x\n", PlatformSaPolicy->Revision));
  
  DEBUG ((EFI_D_INFO, "\n"));
  DEBUG ((EFI_D_INFO, "------------------------ IGD_PANEL_FEATURES -----------------\n"));
  DEBUG ((EFI_D_INFO, " PFITStatus= %x\n", PlatformSaPolicy->IgdPanelFeatures.PFITStatus));
  DEBUG ((EFI_D_INFO, " IgdTheramlSupport= %x\n", PlatformSaPolicy->IgdPanelFeatures.IgdTheramlSupport));
  DEBUG ((EFI_D_INFO, " ALSEnabled= %x\n", PlatformSaPolicy->IgdPanelFeatures.ALSEnabled));
  DEBUG ((EFI_D_INFO, " LidStatus= %x\n", PlatformSaPolicy->IgdPanelFeatures.LidStatus));
  
  DEBUG ((EFI_D_INFO, "\n"));
  DEBUG ((EFI_D_INFO, "------------------------ DPTF_SETTINGS -----------------\n"));
  DEBUG ((EFI_D_INFO, " SdpProfile= %x\n", PlatformSaPolicy->DptfSettings.SdpProfile));
  DEBUG ((EFI_D_INFO, " EnableDptf= %x\n", PlatformSaPolicy->DptfSettings.EnableDptf));
  DEBUG ((EFI_D_INFO, " ProcCriticalTemp= %x\n", PlatformSaPolicy->DptfSettings.ProcCriticalTemp));
  DEBUG ((EFI_D_INFO, " ProcPassiveTemp= %x\n", PlatformSaPolicy->DptfSettings.ProcPassiveTemp));
  DEBUG ((EFI_D_INFO, " GenericCriticalTemp0= %x\n", PlatformSaPolicy->DptfSettings.GenericCriticalTemp0));
  DEBUG ((EFI_D_INFO, " GenericPassiveTemp0= %x\n", PlatformSaPolicy->DptfSettings.GenericPassiveTemp0));
  DEBUG ((EFI_D_INFO, " GenericCriticalTemp1= %x\n", PlatformSaPolicy->DptfSettings.GenericCriticalTemp1));
  DEBUG ((EFI_D_INFO, " GenericPassiveTemp1= %x\n", PlatformSaPolicy->DptfSettings.GenericPassiveTemp1));
  DEBUG ((EFI_D_INFO, " GenericCriticalTemp2= %x\n", PlatformSaPolicy->DptfSettings.GenericCriticalTemp2));
  DEBUG ((EFI_D_INFO, " GenericPassiveTemp2= %x\n", PlatformSaPolicy->DptfSettings.GenericPassiveTemp2));
  DEBUG ((EFI_D_INFO, " GenericCriticalTemp3= %x\n", PlatformSaPolicy->DptfSettings.GenericCriticalTemp3));
  DEBUG ((EFI_D_INFO, " GenericPassiveTemp3= %x\n", PlatformSaPolicy->DptfSettings.GenericPassiveTemp3)); 
  DEBUG ((EFI_D_INFO, " GenericCriticalTemp4= %x\n", PlatformSaPolicy->DptfSettings.GenericCriticalTemp4));
  DEBUG ((EFI_D_INFO, " GenericPassiveTemp4= %x\n", PlatformSaPolicy->DptfSettings.GenericPassiveTemp4));   
  DEBUG ((EFI_D_INFO, " Clpm= %x\n", PlatformSaPolicy->DptfSettings.Clpm));
  DEBUG ((EFI_D_INFO, " SuperDebug= %x\n", PlatformSaPolicy->DptfSettings.SuperDebug));
  DEBUG ((EFI_D_INFO, " LPOEnable= %x\n", PlatformSaPolicy->DptfSettings.LPOEnable));
  DEBUG ((EFI_D_INFO, " LPOStartPState= %x\n", PlatformSaPolicy->DptfSettings.LPOStartPState));
  DEBUG ((EFI_D_INFO, " LPOStepSize= %x\n", PlatformSaPolicy->DptfSettings.LPOStepSize));
  DEBUG ((EFI_D_INFO, " LPOPowerControlSetting= %x\n", PlatformSaPolicy->DptfSettings.LPOPowerControlSetting));
  DEBUG ((EFI_D_INFO, " LPOPerformanceControlSetting= %x\n", PlatformSaPolicy->DptfSettings.LPOPerformanceControlSetting));
  DEBUG ((EFI_D_INFO, " EnableDppm= %x\n", PlatformSaPolicy->DptfSettings.EnableDppm));

  DEBUG ((EFI_D_INFO, "\n"));
  DEBUG ((EFI_D_INFO, " EnableRenderStandby= %x\n", PlatformSaPolicy->EnableRenderStandby));
  DEBUG ((EFI_D_INFO, " GraphicsPerfAnalyzers= %x\n", PlatformSaPolicy->GraphicsPerfAnalyzers));
  DEBUG ((EFI_D_INFO, " MaxInverterPWM= %x\n", PlatformSaPolicy->MaxInverterPWM));
  DEBUG ((EFI_D_INFO, " MinInverterPWM= %x\n", PlatformSaPolicy->MinInverterPWM));
  DEBUG ((EFI_D_INFO, " PmSupport= %x\n", PlatformSaPolicy->PmSupport));
  DEBUG ((EFI_D_INFO, " ForceWake= %x\n", PlatformSaPolicy->ForceWake));
  DEBUG ((EFI_D_INFO, " GfxPause= %x\n", PlatformSaPolicy->GfxPause));
  DEBUG ((EFI_D_INFO, " GraphicsFreqReq= %x\n", PlatformSaPolicy->GraphicsFreqReq));
  DEBUG ((EFI_D_INFO, " EuControl= %x\n", PlatformSaPolicy->EuControl));
  DEBUG ((EFI_D_INFO, " PmWeights= %x\n", PlatformSaPolicy->PmWeights));
  DEBUG ((EFI_D_INFO, " PmLock= %x\n", PlatformSaPolicy->PmLock));
  DEBUG ((EFI_D_INFO, " PavpMode= %x\n", PlatformSaPolicy->PavpMode));
  DEBUG ((EFI_D_INFO, " DopClockGating= %x\n", PlatformSaPolicy->DopClockGating));
  DEBUG ((EFI_D_INFO, " UlClockGating= %x\n", PlatformSaPolicy->UlClockGating));
  DEBUG ((EFI_D_INFO, " S0ixSupported= %x\n", PlatformSaPolicy->S0ixSupported));

  DEBUG ((EFI_D_INFO, "\n"));
  DEBUG ((EFI_D_INFO, "------------------------ PCH Dump platform protocol End -----------------\n"));
  DEBUG ((EFI_D_INFO, "\n"));
}
