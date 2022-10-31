//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2011, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/CPU/Intel/Cedarview/PPM/AmiPpmPolicy/PpmPolicyInitDxe.c 2     1/06/11 12:52a Davidhsieh $
//
// $Revision: 2 $
//
// $Date: 1/06/11 12:52a $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/CPU/Intel/Cedarview/PPM/AmiPpmPolicy/PpmPolicyInitDxe.c $
// 
// 2     1/06/11 12:52a Davidhsieh
// [TAG]  		None
// [Category]  	New Feature
// [Description]  	Add more setup items for PPM policy.
// 
// 1     11/23/10 2:00a Davidhsieh
//
//
//**********************************************************************
/*++
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
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

  PpmPolicyInitDxe.c

Abstract:

  This file is a wrapper for Intel PPM Platform Policy driver.
  Get Setup Value to initilize Intel PPM DXE Platform Policy.

--*/

#include <Efi.h>
#include <token.h>
#include <AmiLib.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include <Protocol/MpService.h>
#include <Setup.h>

#include <Library/CpuCspLib.h>
#include <Protocol/PpmPlatformPolicy.h>
//#include "PpmPlatformPolicy\PpmPlatformPolicy.h"
#include "AmiPpmPolicy.h"

#ifndef PPM_ENABLED
#define PPM_ENABLED TRUE
#endif     

#ifndef PPM_DISABLED
#define PPM_DISABLED FALSE
#endif

PPM_PLATFORM_POLICY_PROTOCOL  mDxePlatformPpmPolicy;
EFI_GUID    gPpmPlatformPolicyProtocolGuid = PPM_PLATFORM_POLICY_PROTOCOL_GUID;
EFI_GUID	gSetupGuid = SETUP_GUID;
//
// Function implementations
//


//(EIP41465)>>>
OEM_CPU_DATA    gOemCpuData;


VOID
OemPpmPolicy (SETUP_DATA *SetupData)
{
}
//<<<(EIP41465)

#if 0
VOID *
AllocateZeroPool (
  IN  UINTN   AllocationSize
  )
/*++

Routine Description:

  Allocate BootServicesData pool and zero it.

Arguments:

  AllocationSize  - The size to allocate

Returns:

  Pointer of the buffer allocated.

--*/
{
  VOID  *Memory;

  Memory = NULL;

  pBS->AllocatePool (EfiBootServicesData, AllocationSize, &Memory);

  if (Memory != NULL) {
    pBS->SetMem (Memory, AllocationSize, 0);
  }

  return Memory;
}
#endif

EFI_STATUS
EFIAPI
PpmDxePolicyInitEntryPoint (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
/*++

Routine Description:

  Initilize Intel PPM DXE Platform Policy

Arguments:

  ImageHandle       Image handle of this driver.
  SystemTable       Global system service table.

Returns:

  EFI_SUCCESS           Initialization complete.
  EFI_UNSUPPORTED       The processor is unsupported by this driver.
  EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.
  EFI_DEVICE_ERROR      Device error, driver exits abnormally.

--*/
{
    EFI_GUID                  MpServiceGuid = EFI_MP_SERVICES_PROTOCOL_GUID;
    EFI_MP_SERVICES_PROTOCOL  *MpService;
    EFI_STATUS                Status;
    UINTN                     CpuCount, NoEnableCpus;
    SETUP_DATA	            *SetupData = NULL;
    UINTN		VariableSize;// = NULL;

    InitAmiLib(ImageHandle, SystemTable);

    CpuCount = 0;

    Status = pBS->LocateProtocol (
                  &MpServiceGuid,
                  NULL,
                  &MpService
                  );
    ASSERT_EFI_ERROR (Status);

    //
    // Get processor count from MP service.
    //
    
//    Status = MpService->GetGeneralMPInfo (MpService, &CpuCount, NULL, NULL, NULL, NULL);
    Status = MpService->GetNumberOfProcessors(MpService, &CpuCount, &NoEnableCpus);
    ASSERT_EFI_ERROR (Status);
    
    pBS->SetMem (&mDxePlatformPpmPolicy, sizeof (mDxePlatformPpmPolicy), 0);    
   
    mDxePlatformPpmPolicy.Revision = PPM_PLATFORM_POLICY_PROTOCOL_REVISION_4;
    mDxePlatformPpmPolicy.FunctionEnables.EnableGv = PPM_DISABLED;
    mDxePlatformPpmPolicy.FunctionEnables.EnableCx = PPM_DISABLED;
    mDxePlatformPpmPolicy.FunctionEnables.EnableCxe = PPM_DISABLED;
    mDxePlatformPpmPolicy.FunctionEnables.EnableC4 = PPM_DISABLED;
//    mDxePlatformPpmPolicy.FunctionEnables.EnableHardC4E = PPM_DISABLED;
    mDxePlatformPpmPolicy.FunctionEnables.EnableC6 = PPM_DISABLED;
    mDxePlatformPpmPolicy.FunctionEnables.EnableC7 = PPM_DISABLED;    
    
    mDxePlatformPpmPolicy.FunctionEnables.EnableTm = PPM_ENABLED;
//    mDxePlatformPpmPolicy.FunctionEnables.EnableTm2 = PPM_ENABLED;
    mDxePlatformPpmPolicy.FunctionEnables.EnableEmttm = PPM_ENABLED;
    mDxePlatformPpmPolicy.FunctionEnables.EnableDynamicFsb = PPM_DISABLED;
    mDxePlatformPpmPolicy.FunctionEnables.EnableTurboMode = PPM_DISABLED;
    mDxePlatformPpmPolicy.FunctionEnables.PowerLimit2 = PPM_DISABLED;
    mDxePlatformPpmPolicy.FunctionEnables.EnableProcHot = PPM_ENABLED;
    
    mDxePlatformPpmPolicy.FunctionEnables.HTD = PPM_DISABLED;
    mDxePlatformPpmPolicy.FunctionEnables.EnableCMP = PPM_ENABLED;
    mDxePlatformPpmPolicy.FunctionEnables.TStatesEnable = PPM_ENABLED;

    mDxePlatformPpmPolicy.FunctionEnables.S0ixSupport = PPM_DISABLED;
		    
    mDxePlatformPpmPolicy.CustomVidTable.VidNumber = 0;

    mDxePlatformPpmPolicy.BootInLfm = PPM_DISABLED;

    Status = GetEfiVariable(
		L"Setup",
		&gSetupGuid,
		NULL,
		&VariableSize,
		&SetupData
	);

    if(!EFI_ERROR(Status)) {
        TRACE ((-1, "Installing PPM Policy Protocol\n"));
		
		mDxePlatformPpmPolicy.FunctionEnables.EnableCMP = SetupData->ActiveCoreCount ? 0 : 1;
			
        if (SetupData->EIST)
            mDxePlatformPpmPolicy.FunctionEnables.EnableGv = PPM_ENABLED;	
        else
            mDxePlatformPpmPolicy.FunctionEnables.EnableGv = PPM_DISABLED;
        
        if (SetupData->PpmS0ix)
            mDxePlatformPpmPolicy.FunctionEnables.S0ixSupport = PPM_ENABLED;
        else
            mDxePlatformPpmPolicy.FunctionEnables.S0ixSupport = PPM_DISABLED;

        if (SetupData->TurboMode)
            mDxePlatformPpmPolicy.FunctionEnables.EnableTurboMode = PPM_ENABLED;
        else
            mDxePlatformPpmPolicy.FunctionEnables.EnableTurboMode = PPM_DISABLED;
//      if (SetupData->PpmTStates)
//          mDxePlatformPpmPolicy.FunctionEnables.TStatesEnable = PPM_ENABLED;
//      else
//          mDxePlatformPpmPolicy.FunctionEnables.TStatesEnable = PPM_DISABLED;

        if (SetupData->PpmCxEnable) {	 
            mDxePlatformPpmPolicy.FunctionEnables.EnableCx = PPM_ENABLED;

            if (SetupData->PpmEnhCxEnable)
                mDxePlatformPpmPolicy.FunctionEnables.EnableCxe = PPM_ENABLED;
            else
                mDxePlatformPpmPolicy.FunctionEnables.EnableCxe = PPM_DISABLED;

            if ((SetupData->PpmMaxCx)== 6){
                mDxePlatformPpmPolicy.FunctionEnables.EnableC6 = PPM_ENABLED;
//              mDxePlatformPpmPolicy.FunctionEnables.EnableHardC4E = PPM_ENABLED;			
            }

            if ((SetupData->PpmMaxCx)== 7){
#if ValleyView_Client == 1
            	UINT8 CpuSigNoVer;              
                CpuSigNoVer = MmioRead8(CSP_PCIE_CFG_ADDRESS(0,0x1f,0,0x08));
                if (CpuSigNoVer < 0x07){
                    mDxePlatformPpmPolicy.FunctionEnables.EnableC6 = PPM_ENABLED;
                    mDxePlatformPpmPolicy.FunctionEnables.EnableC7 = PPM_DISABLED;
                }else {
                    mDxePlatformPpmPolicy.FunctionEnables.EnableC6 = PPM_ENABLED;
                    mDxePlatformPpmPolicy.FunctionEnables.EnableC7 = PPM_ENABLED;
                }
#else				
                mDxePlatformPpmPolicy.FunctionEnables.EnableC6 = PPM_ENABLED;
                mDxePlatformPpmPolicy.FunctionEnables.EnableC7 = PPM_ENABLED;
#endif				
            }
        } else {
            mDxePlatformPpmPolicy.FunctionEnables.EnableCx = PPM_DISABLED;	  
            mDxePlatformPpmPolicy.FunctionEnables.EnableCxe = PPM_DISABLED;    
//          mDxePlatformPpmPolicy.FunctionEnables.EnableHardC4E = PPM_DISABLED;	 
            mDxePlatformPpmPolicy.FunctionEnables.EnableC6 = PPM_DISABLED;
            mDxePlatformPpmPolicy.FunctionEnables.EnableC7 = PPM_DISABLED;
        }	 
    }	

    TRACE((TRACE_ALWAYS, "Going to install PPM Policy Protocol\n"));

    Status = pBS->InstallProtocolInterface (
                  &ImageHandle,
                  &gPpmPlatformPolicyProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mDxePlatformPpmPolicy
                  );
    ASSERT_EFI_ERROR(Status);
  
    OemPpmPolicy(SetupData);    //<<<(EIP41465)
    
    return EFI_SUCCESS;
}
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
