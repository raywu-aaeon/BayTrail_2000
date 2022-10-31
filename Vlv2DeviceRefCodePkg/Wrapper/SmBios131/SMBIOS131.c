/*++

Copyright (c) 2004-2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  TDTPlatformPolicy.c
  
Abstract:

  TDTPlatformPolicy to check and set TDT Platform Policy.
  
--*/
/*++
 This file contains an 'Intel Peripheral Driver' and is        
 licensed for Intel CPUs and chipsets under the terms of your  
 license agreement with Intel or your vendor.  This file may   
 be modified by the user, subject to additional terms of the   
 license agreement                                             
--*/


//
// Statements that include other files
//
//#include "Efi.h"



#include "SMBIOS131.h"
#include <Setup.h>

extern EFI_GUID gEfiSetupVariableGuid;

//
// Function implementations
//
EFI_STATUS
EFIAPI
SMBIOS131EntryPoint (
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable
)
/*++
 
Routine Description:
 
  Entry point for the TDTPlatformPolicy Driver.
  
Arguments:
 
  ImageHandle       Image handle of this driver.
  SystemTable       Global system service table.
 
Returns:
 
  EFI_SUCCESS           Initialization complete.
  EFI_UNSUPPORTED       The chipset is unsupported by this driver.
  EFI_OUT_OF_RESOURCES  Do not have enough resources to initialize the driver.
  EFI_DEVICE_ERROR      Device error, driver exits abnormally.
 
--*/
{
  EFI_STATUS                Status;
  EFI_SMBIOS_PROTOCOL       *EfiSmbiosProtocol;
  EFI_GUID                  gEfiSmbiosTableGuid = EFI_SMBIOS_PROTOCOL_GUID;
  SMBIOS_131_STRUCT         *mSmbios131;
  GEN_GET_FW_CAPSKU         MsgGenGetFwCapsSku;
  GEN_GET_FW_CAPS_SKU_ACK   MsgGenGetFwCapsSkuAck;
  DXE_TDT_POLICY_PROTOCOL   *mTDTPlatformPolicy;
  GEN_GET_FW_VER_ACK        MsgGenGetFwVersionAck;
  AT_STATE_STRUCT           AtStateInfo;
  EFI_GUID                  gDxePlatformTdtPolicyGuid = DXE_PLATFORM_TDT_POLICY_GUID;
  
  Status = HeciGetFwCapsSkuMsg (&MsgGenGetFwCapsSku, &MsgGenGetFwCapsSkuAck);
  DEBUG((EFI_D_ERROR, "[SMBIOS 131] HeciGetFwCapsSkuMsg = %r, MsgGenGetFwCapsSkuAck.Data.FWCapSku.Fields.IntelAT = %x \n" , Status, MsgGenGetFwCapsSkuAck.Data.FWCapSku.Fields.IntelAT));  

  Status = gBS->AllocatePool(EfiBootServicesData,  sizeof(SMBIOS_131_STRUCT)+2, &mSmbios131);
  DEBUG((EFI_D_ERROR, "[SMBIOS 131] AllocatePool mSmbios131 Status = %r\n" , Status));
  gBS->SetMem(mSmbios131, sizeof(SMBIOS_131_STRUCT)+2, 0);
  Status = gBS->LocateProtocol (
           &gEfiSmbiosTableGuid, 
           NULL,
           (VOID **)&EfiSmbiosProtocol);
  
  DEBUG((EFI_D_ERROR, "[SMBIOS 131] Locate SMBIOS Protocol Status = %r -- \n" , Status));
  
  if(EFI_ERROR(Status))
    return EFI_SUCCESS;

  mSmbios131->Type = 0x83;
  mSmbios131->Length = sizeof(SMBIOS_131_STRUCT);
  mSmbios131->Handle = 0xFFFE;
  Status = gBS->LocateProtocol (
           &gDxePlatformTdtPolicyGuid, 
           NULL,
           (VOID **)&mTDTPlatformPolicy);
  DEBUG((EFI_D_ERROR, "[SMBIOS 131] Locate TDT Protocol Status = %r, mTDTPlatformPolicy->Tdt.TdtConfig = %x \n" , Status,mTDTPlatformPolicy->Tdt.TdtConfig));  
  mSmbios131->TxeCapability[0] |= BIT0;
  if(!EFI_ERROR(Status) && mTDTPlatformPolicy->Tdt.TdtConfig && MsgGenGetFwCapsSkuAck.Data.FWCapSku.Fields.IntelAT)
  {
    mSmbios131->TxeCapability[0] |= BIT13;
  }
  if(EFI_ERROR(Status))
    mTDTPlatformPolicy = NULL;
  // =================================
  // Intel TXE Platform Capabilities
  // =================================

  Status = HeciGetFwVersionMsg (&MsgGenGetFwVersionAck);
  DEBUG((EFI_D_ERROR, "[SMBIOS 131] HeciGetFwVersionMsg = %r\n" , Status));  
  DEBUG((EFI_D_ERROR, "[SMBIOS 131] MsgGenGetFwVersionAck.Data.CodeMinor = %x \n" ,MsgGenGetFwVersionAck.Data.CodeMinor));  
  DEBUG((EFI_D_ERROR, "[SMBIOS 131] MsgGenGetFwVersionAck.Data.CodeMajor = %x \n" ,MsgGenGetFwVersionAck.Data.CodeMajor));  
  DEBUG((EFI_D_ERROR, "[SMBIOS 131] MsgGenGetFwVersionAck.Data.CodeBuildNo = %x \n" ,MsgGenGetFwVersionAck.Data.CodeBuildNo));  
  DEBUG((EFI_D_ERROR, "[SMBIOS 131] MsgGenGetFwVersionAck.Data.CodeHotFix = %x \n" ,MsgGenGetFwVersionAck.Data.CodeHotFix));  
  if(!EFI_ERROR(Status))
  {
    mSmbios131->TxeCapability[1] = MsgGenGetFwVersionAck.Data.CodeMinor + MsgGenGetFwVersionAck.Data.CodeMajor*0x10000;
    mSmbios131->TxeCapability[2] = MsgGenGetFwVersionAck.Data.CodeBuildNo + MsgGenGetFwVersionAck.Data.CodeHotFix*0x10000;
  }
  // =================================
  // TXE Platform Configuration State
  // =================================
  Status = HeciGetAtFwStateInfoMsg(&AtStateInfo);
  if(!EFI_ERROR(Status) && (AtStateInfo.AtState != TDT_STATE_INACTIVE))
    mSmbios131->TxeConfigState |= BIT5;
  
  // =================================
  //    BIOS Security Capabilities
  // =================================

  // BIOS support TXE Setup Item
  mSmbios131->BIOSSecurityCap |= BIT4;
  if(mTDTPlatformPolicy && mTDTPlatformPolicy->Tdt.TdtPBAEnable)
  {
    mSmbios131->BIOSSecurityCap |= BIT6;
  }
  mSmbios131->StructIdentifier = 0x6F725076;
  
  Status = EfiSmbiosProtocol->Add(EfiSmbiosProtocol, NULL, &mSmbios131->Handle, (EFI_SMBIOS_TABLE_HEADER*)mSmbios131);
  DEBUG((EFI_D_ERROR, "[SMBIOS 131] SMBIOS add Struct Status = %r mSmbios131.Handle = %x \n" , Status, mSmbios131->Handle));
  gBS->FreePool(mSmbios131);
  
  return EFI_SUCCESS;
}

