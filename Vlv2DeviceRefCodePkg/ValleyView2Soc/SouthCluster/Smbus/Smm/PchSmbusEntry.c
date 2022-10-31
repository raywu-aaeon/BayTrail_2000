/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2009 - 2011 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

  @file
  PchSmbusEntry.c

  @brief
  PCH Smbus Driver Entry

**/
#include "PchSmbus.h"
#include <PchRegs.h>
#include <Protocol/PchPlatformPolicy.h>
#include <protocol/smmsmbus.h>
#ifdef ECP_FLAG
EFI_GUID  gEfiSmmSmbusProtocolGuid = EFI_SMM_SMBUS_PROTOCOL_GUID;
#else
#include <Library/SmmServicesTableLib.h>
#endif


EFI_STATUS
SmbusExecute (
  IN CONST  EFI_SMBUS_HC_PROTOCOL         *This,
  IN CONST  EFI_SMBUS_DEVICE_ADDRESS      SlaveAddress,
  IN CONST  EFI_SMBUS_DEVICE_COMMAND      Command,
  IN CONST  EFI_SMBUS_OPERATION           Operation,
  IN CONST  BOOLEAN                       PecCheck,
  IN OUT  UINTN                           *Length,
  IN OUT  VOID                            *Buffer
  )
/**

  @brief
  Execute an SMBUS operation

  @param[in] This                 The protocol instance
  @param[in] SlaveAddress         The address of the SMBUS slave device
  @param[in] Command              The SMBUS command
  @param[in] Operation            Which SMBus protocol will be issued
  @param[in] PecCheck             If Packet Error Code Checking is to be used
  @param[in] Length               Length of data
  @param[in] Buffer               Data buffer

  @retval EFI_SUCCESS             The SMBUS operation is successful
  @retval Other Values            Something error occurred

**/
{
  InitializeSmbusRegisters ();

  return SmbusExec (
           SlaveAddress,
           Command,
           Operation,
           PecCheck,
           Length,
           Buffer
           );
}

EFI_STATUS
EFIAPI
InitializePchSmbusSmm (
  IN      EFI_HANDLE            ImageHandle,
  IN      EFI_SYSTEM_TABLE      *SystemTable
  )
/**

  @brief
  Smbus driver entry point

  @param[in] ImageHandle          ImageHandle of this module
  @param[in] SystemTable          EFI System Table

  @retval EFI_SUCCESS             Driver initializes successfully
  @retval Other values            Some error occurred

--*/
{
  EFI_STATUS  Status;

  mSmbusContext = AllocateZeroPool (sizeof (SMBUS_INSTANCE));	// AMI_OVERRIDE - Fix X64 mode will hang. - EIP111040+

// AMI_OVERRIDE - Fix X64 mode will hang. - EIP111040+ >>
  if (mSmbusContext == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
// AMI_OVERRIDE - Fix X64 mode will hang. - EIP111040+ <<
  } else {
    mSmbusContext->Signature                  = PCH_SMBUS_PRIVATE_DATA_SIGNATURE;
    mSmbusContext->IoDone                     = IoDone;
    mSmbusContext->SmbusIoRead                = SmbusIoRead;
    mSmbusContext->SmbusIoWrite               = SmbusIoWrite;
    mSmbusContext->SmbusController.Execute    = SmbusExecute;
    mSmbusContext->SmbusController.ArpDevice  = SmbusArpDevice;
    mSmbusContext->SmbusController.GetArpMap  = SmbusGetArpMap;
    mSmbusContext->SmbusController.Notify     = SmbusNotify;

    ///
    /// Install the SMBUS interface
    ///
// AMI_OVERRIDE - Fix X64 mode will hang. - EIP111040+ >>
    Status = gSmst->SmmInstallProtocolInterface (
                      &mSmbusContext->Handle,
                      &gEfiSmmSmbusProtocolGuid,
                      EFI_NATIVE_INTERFACE,
                      &mSmbusContext->SmbusController
                      );
// AMI_OVERRIDE - Fix X64 mode will hang. - EIP111040+ <<
    ASSERT_EFI_ERROR (Status);
  }

  return EFI_SUCCESS;
}
