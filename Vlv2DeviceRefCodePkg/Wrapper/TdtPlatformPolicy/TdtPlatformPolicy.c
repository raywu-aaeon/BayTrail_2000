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



#include "TdtPlatformPolicy.h"
#include <Setup.h>

extern EFI_GUID gEfiSetupVariableGuid;

DXE_TDT_POLICY_PROTOCOL mTDTPlatformPolicyInstance = { 0 };
#define BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID \
    {0xdbc9fd21, 0xfad8, 0x45b0, 0x9e, 0x78, 0x27, 0x15, 0x88, 0x67, 0xcc, 0x93}
EFI_GUID mBdsAllDriversConnectedProtocolGuid = BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID;

#define BDS_DISPATCHER_PROTOCOL_GUID \
    {0xcfc5b882, 0xebde, 0x4782, { 0xb1, 0x82, 0x2f, 0xec, 0x7e, 0x3f, 0x3e, 0x90 }}

EFI_GUID BdsDispatcherProtocolGuid = BDS_DISPATCHER_PROTOCOL_GUID;
//
// Function implementations
//
VOID
EFIAPI
TdtAllDriversConnected (
  IN      EFI_EVENT                 Event,
  IN      VOID                      *Context
  )
{
  TDT_OPERATION_PROTOCOL  *TdtOp;
  EFI_STATUS              Status;
  BOOLEAN                 SetTdtEnterSuspendState;
  UINT8                   TdtEnterSuspendState;
  UINTN                   VariableSize;
  SETUP_DATA              SystemConfiguration;
  UINT32                  Attributes = 0; //EIP168675

  Status = gBS->LocateProtocol (&gEfiTdtOperationProtocolGuid, NULL, (VOID **) &TdtOp);
  if (!EFI_ERROR (Status)) {
    Status = TdtOp->PerformTdtOperation(&SetTdtEnterSuspendState, &TdtEnterSuspendState);
    
    if ((!EFI_ERROR (Status)) && (SetTdtEnterSuspendState == TRUE)) {
      VariableSize  = sizeof (SETUP_DATA);
      Status = gRT->GetVariable (
        NORMAL_SETUP_NAME,
        &gEfiSetupVariableGuid,
        &Attributes, //EIP168675
        &VariableSize,
        &SystemConfiguration
      );
      if (!EFI_ERROR (Status)) {
        SystemConfiguration.Suspend = TdtEnterSuspendState;
        Status = gRT->SetVariable (
                                NORMAL_SETUP_NAME,
                                &gEfiSetupVariableGuid,
                                Attributes, //EIP168675
                                sizeof(SETUP_DATA),
                                &SystemConfiguration
        );
      }
      gST->ConOut->ClearScreen(gST->ConOut);
    }
  }
}
EFI_STATUS
EFIAPI
TdtPlatformPolicyEntryPoint (
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
  EFI_STATUS 					Status;
  UINTN                         SizeOfNvStore;
  SETUP_DATA	        SystemConfiguration;
  VOID                *TdtRegistration;
  EFI_EVENT           TdtEvent;

  TdtEvent = EfiCreateProtocolNotifyEvent (
                  &BdsDispatcherProtocolGuid,
                  TPL_CALLBACK,
                  TdtAllDriversConnected,
                  NULL,
                  &TdtRegistration
                  );

  //
  // Get TDT BIOS Setup 
  //	
  SizeOfNvStore = sizeof(SETUP_DATA);
  Status = gRT->GetVariable(
    NORMAL_SETUP_NAME,
    &gEfiSetupVariableGuid,
    NULL,
    &SizeOfNvStore,
    &SystemConfiguration
  );
  if (EFI_ERROR (Status)) {
    mTDTPlatformPolicyInstance.Tdt.TdtConfig	             = 1;
    mTDTPlatformPolicyInstance.Tdt.TdtEnterSuspendState    = 0;
    mTDTPlatformPolicyInstance.Tdt.TdtRecoveryAttempt      = 3;
    mTDTPlatformPolicyInstance.Tdt.TdtPBAEnable            = 1;
  } else {
    mTDTPlatformPolicyInstance.Tdt.TdtConfig 	           = SystemConfiguration.Tdt;
    mTDTPlatformPolicyInstance.Tdt.TdtEnterSuspendState    = SystemConfiguration.Suspend;
    mTDTPlatformPolicyInstance.Tdt.TdtRecoveryAttempt      = 3;
    mTDTPlatformPolicyInstance.Tdt.TdtPBAEnable            = SystemConfiguration.PBAEnable;
  }
	  
  //
  // Install the TDT Platform Policy PROTOCOL interface
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
               &ImageHandle,
               &gDxePlatformTdtPolicyGuid,	
               &mTDTPlatformPolicyInstance,
               NULL
             );



  return Status;
}


