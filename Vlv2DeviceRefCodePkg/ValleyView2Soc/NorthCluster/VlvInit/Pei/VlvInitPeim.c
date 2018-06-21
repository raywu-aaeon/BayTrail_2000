/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/

/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  VlvInitPeim.c

Abstract:

  The PEIM implements the SA PEI Initialization.

--*/

#include "VlvInitPeim.h"
#include "token.h"
#if PCIE_VGA_WORKAROUND
#include <Ppi\Stall.h>
#include "GraphicsInit.h"
#endif
#include <Ppi/VlvPeiInit.h>
#ifndef ECP_FLAG
#include <Library/DebugLib.h>
#endif
#include "VlvAccess.h"
//#include <Guid/SetupVariable.h>		//AMI_OVERRIDE
#ifdef ECP_FLAG
#include EFI_PPI_DEFINITION(Variable)
#else
#include <Ppi/ReadOnlyVariable2.h>
#endif
#include <Setup.h>    // AMI_OVERRIDE

#ifdef ECP_FLAG
EFI_GUID gVlvPolicyPpiGuid = VLV_POLICY_PPI_GUID;
EFI_GUID gVlvPeiInitPpiGuid = VLV_PEI_INIT_PPI_GUID;
EFI_GUID gPeiReadOnlyVariablePpiGuid = PEI_READ_ONLY_VARIABLE_ACCESS_PPI_GUID;
EFI_GUID gEfiSetupVariableGuid = SYSTEM_CONFIGURATION_GUID;
#endif

//
// Global variables
//


static EFI_PEI_PPI_DESCRIPTOR mVlvPeiInitPpi[] = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gVlvPeiInitPpiGuid,
  NULL
};

extern EFI_GUID gEfiPeiReadOnlyVariable2PpiGuid;
extern EFI_GUID gEfiSetupVariableGuid;

void CheckNoCpuCoreWarmReset (
  IN CONST EFI_PEI_SERVICES **PeiServices
  )
{
  UINT8                                       ActiveProcCores;
  UINT32                                      EnableProcCores;
  UINT32                                      data32;
  UINT32                                      VariableSize;
  EFI_STATUS                                  Status;
  SETUP_DATA                                  SystemConfig;    //AMI_OVERRIDE
#ifdef ECP_FLAG
  PEI_READ_ONLY_VARIABLE_PPI *CpuCoreVariable;
#else
  EFI_PEI_READ_ONLY_VARIABLE2_PPI *CpuCoreVariable;
#endif

  //Get variable of ActiveProcessorCores from NVRAM
#ifdef ECP_FLAG
  Status = (*PeiServices)->LocatePpi(
                             (EFI_PEI_SERVICES **) PeiServices,
                             &gPeiReadOnlyVariablePpiGuid,
#else
  Status = (*PeiServices)->LocatePpi(
                             PeiServices,
                             &gEfiPeiReadOnlyVariable2PpiGuid,
#endif
                             0,
                             NULL,
                             (VOID **) &CpuCoreVariable
                             );
  ASSERT_EFI_ERROR (Status);

	VariableSize = sizeof(SETUP_DATA);    //AMI_OVERRIDE
#ifdef ECP_FLAG
  Status = CpuCoreVariable->PeiGetVariable(
                              (EFI_PEI_SERVICES **) PeiServices,
#else
  Status = CpuCoreVariable->GetVariable(
                              CpuCoreVariable,
#endif
                              L"Setup",
                              &gEfiSetupVariableGuid,
                              NULL,
                              &VariableSize,
                              &SystemConfig
                              );
  ASSERT_EFI_ERROR (Status);
  // AMI_OVERRIDE - EIP145090 Boot With NVRAM Destroyed Test-fail >>
  //ASSERT_EFI_ERROR (Status);
  if (EFI_ERROR(Status)) {
    ActiveProcCores = 0;
  } else {
    ActiveProcCores = SystemConfig.ActiveCoreCount;
  }
  // AMI_OVERRIDE - EIP145090 Boot With NVRAM Destroyed Test-fail <<
  ActiveProcCores &= 0x1;
  //Get No. of Enabled Cpu cores,Actually
  MsgBus32Read(VLV_PUNIT,PUNIT_CPU_SOFT_STRAPS,EnableProcCores);
  EnableProcCores &= 0x700;

  //Set the value from NVRAM to PUnit register
  if(((ActiveProcCores == 0) && (EnableProcCores != 0)) || ((ActiveProcCores == 1) && (EnableProcCores != 0x100))) {
    if((UINT32)ActiveProcCores != EnableProcCores) {
      if(ActiveProcCores == 0x01) {
        MsgBus32AndThenOr(VLV_PUNIT, PUNIT_CPU_SOFT_STRAPS, data32, 0xfffff8ff, 0x00000100);
      } else {
        MsgBus32AndThenOr(VLV_PUNIT, PUNIT_CPU_SOFT_STRAPS, data32, 0xfffff8ff, 0x00000000);
      }
      //Trigger Warm Reset
      IoWrite8 (0xCF9, 0x02);
      IoWrite8 (0xCF9, 0x06);
    }

  }
}

//
// Functions
//
EFI_STATUS
EFIAPI
VlvInitPeiEntryPoint (
#ifdef ECP_FLAG
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
#else
  IN EFI_PEI_FILE_HANDLE       FfsHeader,
  IN CONST EFI_PEI_SERVICES    **PeiServices
#endif
  )
/*++

Routine Description:

  SA PEI Initialization.

Arguments:

  FfsHeader    - Pointer to Firmware File System file header.
  PeiServices  - General purpose services available to every PEIM.

Returns:

  EFI_SUCCESS

--*/
{
  EFI_STATUS            Status;
  VLV_POLICY_PPI        *VlvPolicyPpi;
  UINT32                Dbuff=0xFFFFFFFF;
#if PCIE_VGA_WORKAROUND
  EFI_PEI_STALL_PPI *StallPpi;
#endif
  //
  // Get platform policy settings through the VlvPolicy PPI
  //
  Status = (**PeiServices).LocatePpi (
                            PeiServices,
                            &gVlvPolicyPpiGuid,
                            0,
                            NULL,
                            (VOID **) &VlvPolicyPpi
                            );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_ERROR, "VlvInitPeiEntryPoint start....\n"));

  DEBUG ((EFI_D_ERROR, "\n------------------------ VLV SSA Platform Policy dump Begin ---------------\n"));
  DEBUG ((EFI_D_ERROR, "Graphics Configuration:\n"));
  DEBUG ((EFI_D_ERROR, " GttSize : %x MB\n",VlvPolicyPpi->GtConfig.GttSize));
  DEBUG ((EFI_D_ERROR, " IgdDvmt50PreAlloc : %x\n",VlvPolicyPpi->GtConfig.IgdDvmt50PreAlloc));
  DEBUG ((EFI_D_ERROR, " PrimaryDisplay : %x\n",VlvPolicyPpi->GtConfig.PrimaryDisplay));
  DEBUG ((EFI_D_ERROR, " ApertureSize : %x\n",VlvPolicyPpi->GtConfig.ApertureSize));
  DEBUG ((EFI_D_ERROR, " Turbo Enable : %x\n",VlvPolicyPpi->GtConfig.IgdTurboEn));

  DEBUG ((EFI_D_ERROR, "\n------------------------ VLV SSA Platform Policy dump END -----------------\n"));

#if PCIE_VGA_WORKAROUND
  Status = (**PeiServices).LocatePpi(PeiServices, &gEfiPeiStallPpiGuid, 0, NULL, &StallPpi);
  ASSERT_EFI_ERROR (Status);

  if (VlvPolicyPpi->GtConfig.PrimaryDisplay != IGD) {
      // This delay is needed for PCIe stabilization
      StallPpi->Stall(PeiServices, StallPpi, 1000000);
  }
#endif

  //
  // Program EC Base
  //
  ProgramEcBase ();
  DEBUG ((EFI_D_ERROR, "ProgramEcBase Done....\n"));
  //
  // Program BuitConfig
  //
  SSASafeConfiguration();
  CheckNoCpuCoreWarmReset(PeiServices);
  DEBUG ((EFI_D_ERROR, "SSASafeConfiguration Done....\n"));
  //
  // Program ISPConfig
  //
  DEBUG ((EFI_D_ERROR, "Clear Dbuff  to all zero before read 0x%x\n",Dbuff));
  MsgBus32Read(VLV_PUNIT,PUNIT_ISPSSPM0,Dbuff);
  DEBUG ((EFI_D_ERROR, "PUNIT_ISPSSPM0 value is 0x%X\n",Dbuff));
  if((Dbuff & B_ISPSSPM0_FUSDIS)==0) {  //compare to bit26, 0 is Device fuse enbled; 1 is Device fuse disabled
    DEBUG ((EFI_D_ERROR, "ISP Device is enabled by fuse\n"));
    if(VlvPolicyPpi->ISPEn == 0x01) {
      DEBUG ((EFI_D_ERROR, "Iunit Configuration - Start\n"));
      ISPConfig(PeiServices, VlvPolicyPpi);
    } else {
      DEBUG ((EFI_D_ERROR, "Skip ISPConfig\n"));
    }
  } else {
    DEBUG ((EFI_D_ERROR, "ISP Device is disabed by fuse\n"));
  }
  //
  // Thermal Registers Init
  //
  InitThermalRegisters();
  DEBUG ((EFI_D_INFO, "InitThermalRegisters Done....\n"));

#ifdef SG_SUPPORT
  //
  // Initialize SwitchableGraphics
  //
  DEBUG ((EFI_D_INFO, "Initializing SwitchableGraphics\n"));
  SwitchableGraphicsInit (PeiServices, VlvPolicyPpi);
#endif

  //
  // Initialize Graphics (IGD/External)
  //
  GraphicsInit (PeiServices, VlvPolicyPpi);
  DEBUG ((EFI_D_ERROR, "GraphicsInit Done....\n"));
  //
  // Program PUNIT_BIOS_CONFIG for S0ix
  //
  if(VlvPolicyPpi->S0ixEn == 0x01) {
   DEBUG ((EFI_D_INFO,"Set PUNIT_BIOS_CONFIG BIT18 for S0ix...\n"));
   MsgBus32AndThenOr(VLV_PUNIT, PUNIT_BIOS_CONFIG, Dbuff,0xFFFBFFFF, (BIT18) );
  }
  //
  // Install Ppi with VlvInitPeim complete
  //
  Status = (**PeiServices).InstallPpi (PeiServices, mVlvPeiInitPpi);
  DEBUG ((EFI_D_ERROR, "Install mVlvPeiInitPpi Done....\n"));
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

VOID ProgramEcBase()
{
  //
  //Setting up EC base
  //
  PciCfg32Write_CF8CFC(MC_BUS, 0x00, 0x00, 0xD4, (EC_BASE|BIT0));
  PciCfg32Write_CF8CFC(MC_BUS, 0x00, 0x00, 0xD0, ((VLV_MBR_WRITE_CMD) | (VLV_BUNIT << 16)) + (BUNIT_BECREG  << 8) + 0xF0);
}

VOID ISPConfig(
  IN CONST EFI_PEI_SERVICES    **PeiServices,
  IN VLV_POLICY_PPI    *VlvPolicyPpi
  )
{
  UINT32 Dbuff=0xFFFFFFFF;
  UINT32 Dbgcounter=0x0;
  UINT8 ISPOnRetryCounter=10;
  DEBUG ((EFI_D_ERROR, "Default Iunit configure as B0D2F0\n"));
  if(VlvPolicyPpi->ISPPciDevConfig == 0x01) {
    //Config IUNIT as B0D2F0
    DEBUG ((EFI_D_ERROR, "Re-Configuring Iunit as B0D2F0\n"));
    //BIOS Spec REV 0.5 section 21.
    //Program 1b to the IUNIT_DEVICE_TYPE field (bit 0) of the BIOSCFG_IPDEVTYPE register in PUNIT (offset 1Dh, port 04h).
    MsgBus32AndThenOr(VLV_PUNIT, PUNIT_BIOSCFG_IPDEVTYPE, Dbuff,0xFFFFFFFF, BIT0);
  } else if(VlvPolicyPpi->ISPPciDevConfig == 0x02) {
    //Config IUNIT as B0D3F0
    DEBUG ((EFI_D_ERROR, "Configuring Iunit as B0D3F0\n"));
    //BIOS Spec REV 0.5 section 21
    //Writing 00b to Punit offset 0x39 bit 1:0 (ISPSCC). Default is 11b
    MsgBus32And(VLV_PUNIT, PUNIT_ISPSSPM0, Dbuff, 0xFFFFFFFC);
    //Bios Spec x.2
    //System BIOS or IA Software should wait until the ISPSSS field (bits 25:24) of the ISPSSPM0 register in PUNIT is 00b indicating that IUNIT is powered up.

    Dbuff = 0;
    DEBUG ((EFI_D_ERROR, "Clear Dbuff  to all zero before read 0x%x\n",Dbuff));
    MsgBus32Read(VLV_PUNIT,PUNIT_ISPSSPM0,Dbuff);
    DEBUG ((EFI_D_ERROR, "PUNIT_ISPSSPM0 value is 0x%x\n",Dbuff));


    while((Dbuff & BIT24) != 0 && (Dbuff & BIT25) != 0) {
      Dbgcounter++;
      if(Dbgcounter != ISPOnRetryCounter) { //Max limit for ReTry
        DEBUG ((EFI_D_ERROR, "Waiting Iunit being power up now. Retry counter %d:\n",Dbgcounter));
        Dbuff = 0;
        DEBUG ((EFI_D_ERROR, "Clear Dbuff  to all zero before read 0x%x\n",Dbuff));
        MsgBus32Read(VLV_PUNIT,PUNIT_ISPSSPM0,Dbuff);
        DEBUG ((EFI_D_ERROR, "PUNIT_ISPSSPM0 value is 0x%x\n",Dbuff));
      } else {
        Dbgcounter = 0;
        DEBUG ((EFI_D_ERROR, "Fail to power up ISP after 10 time retry\n"));
        //break;
        goto Exist_ISPConfig;
      }
    }
    DEBUG ((EFI_D_ERROR, "Iunit being powered up now\n"));
  }
  Exist_ISPConfig:
  DEBUG ((EFI_D_ERROR, "Iunit Configuration - End\n"));
}



VOID SSASafeConfiguration()
{
  UINT32 data32;
  // Workaround for sighting #4376986, still under discussion to fix in A0
  // SSA-BUnit Line 14
  MsgBus32AndThenOr(VLV_BUNIT,BUNIT_BALIMIT0,data32,0xc0d0d0d0, 0x1f2f2f2f);

  MsgBus32AndThenOr(VLV_AUNIT, AUNIT_AVCCTL, data32, 0x7ffffeff, 0x80000100);
  //Valleyview Sighting HSD#4682673
  MsgBus32AndThenOr(VLV_AUNIT, AUNIT_ACFCACV, data32, 0x7fffffff, 0x00000000);
  MsgBus32AndThenOr(VLV_CUNIT, CUNIT_ACCESS_CTRL_VIOL, data32, 0x7fffffff, 0x00000000);
  //Valleyview Sighting HSD#4682852
  MsgBus32Write(VLV_CUNIT, CUNIT_SSA_REGIONAL_TRUNKGATE_CTL,0x00070008); // PnP settings
  MsgBus32AndThenOr(VLV_TUNIT, TUNIT_CTL, data32, 0xffeefbcf, 0x00110430);
  MsgBus32AndThenOr(VLV_TUNIT, TUNIT_MISC_CTL, data32, 0xfffbffef, 0x00040010);

}


VOID InitThermalRegisters()
{
/*bteo1


bteo1*/
}

