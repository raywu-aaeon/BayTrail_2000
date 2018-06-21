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
  ScBiosWriteProtect.c

  @brief
  PCH BIOS Write Protect Driver.

**/
#include "ScBiosWriteProtect.h"
#include <Protocol/SmmIchnDispatch.h>
#include <Protocol/SmmSwDispatch.h>
#include <Library/IoLib.h>
#include <Library/DebugLib.h>
#include <Protocol/PchPlatformPolicy.h>
#include <Library/UefiBootServicesTableLib.h>
#include <PchCommonDefinitions.h>
//EIP167096 >>
#include <Library/SbCspLib.h>
#include <Library/S3BootScriptLib.h>
#include "Token.h"
//EIP167096 <<
//EIP180260 >>   
#include <Library/SmmServicesTableLib.h>
#include <Protocol/SmmReadyToLock.h>
//EIP180260 <<
///
/// Global variables
///
EFI_SMM_ICHN_DISPATCH_PROTOCOL  *mIchnDispatch;
EFI_SMM_SW_DISPATCH_PROTOCOL    *mSwDispatch;
UINTN                           SpiBase;

static
VOID
PchBiosWpCallback(
    IN  EFI_HANDLE                              DispatchHandle,
    IN  EFI_SMM_ICHN_DISPATCH_CONTEXT           *DispatchContext
)
/**

  @brief
  This hardware SMI handler will be run every time the BIOS Write Enable bit is set.

  @param[in] DispatchHandle       Not used
  @param[in] DispatchContext      Not used

  @retval None

**/
{
    ///
    /// Disable BIOSWE bit to protect BIOS
    ///
    MmioAnd8((UINTN)(SpiBase + R_PCH_SPI_BCR), (UINT8) ~B_PCH_SPI_BCR_BIOSWE);    
}

VOID
PchBiosLockSwSmiCallback(
    IN  EFI_HANDLE                    DispatchHandle,
    IN  EFI_SMM_SW_DISPATCH_CONTEXT   *DispatchContext
)
/**

  @brief
  Register an IchnBiosWp callback function to handle TCO BIOSWR SMI
  SMM_BWP and BLE bits will be set here

  @param[in] DispatchHandle       Not used
  @param[in] DispatchContext      Not used

  @retval None

**/
{
    EFI_STATUS                    Status;
    EFI_SMM_ICHN_DISPATCH_CONTEXT IchnContext;
    EFI_HANDLE                    IchnHandle;
    UINT32                        Index;  //EIP167096

    if(mIchnDispatch == NULL) {
        return;
    }

    IchnHandle = NULL;

    ///
    /// Set SMM_BWP bit before registering IchnBiosWp
    ///
    MmioOr8((UINTN)(SpiBase + R_PCH_SPI_BCR), (UINT8) (B_PCH_SPI_BCR_SMM_BWP | B_PCH_SPI_BCR_BLE)); //EIP130725
    ///
    /// Register an IchnBiosWp callback function to handle TCO BIOSWR SMI
    ///
    IchnContext.Type = IchnBiosWp;
    Status = mIchnDispatch->Register(
                 mIchnDispatch,
                 PchBiosWpCallback,
                 &IchnContext,
                 &IchnHandle
             );
    ASSERT_EFI_ERROR(Status);

    /* Has been done in Intel RC
    MmioOr8((UINTN)(SpiBase + R_PCH_SPI_SCS), (UINT8) B_PCH_SPI_SCS_SMIWPEN); //EIP130725 >>    
    */
    
    ///
    /// Unregister BIOS Lock SW SMI handler since we do not need it now
    ///
    Status = mSwDispatch->UnRegister(
                 mSwDispatch,
                 DispatchHandle
             );
    ASSERT_EFI_ERROR(Status);
    
//EIP167096 >>
    //
    // Restore Protected Range Registers for S3.
    //
    Status = SbFlashProtectedRange ();
    if (!EFI_ERROR(Status)) {
      //
      // Restore Protected Range Registers for S3
      // Note: PR4 register is RO
      //
      for (Index = 0; Index < 4; Index++) {
        S3BootScriptSaveMemWrite (
          EfiBootScriptWidthUint32,
          (UINTN) (SpiBase + R_PCH_SPI_PR0 + (Index * 4)),
          1,
          (VOID *) (UINTN) (SpiBase + R_PCH_SPI_PR0 + (Index * 4))
          );
      }
    }
//EIP167096 <<
}

//EIP180260 >>   
/**
  Smm Ready To Lock event notification handler.

  The TCO_LOCK set for security.
  
  @param[in] Protocol   Points to the protocol's unique identifier.
  @param[in] Interface  Points to the interface instance.
  @param[in] Handle     The handle on which the interface was installed.

  @retval EFI_SUCCESS   Notification handler runs successfully.
 **/
EFI_STATUS
EFIAPI
PchBiosWriteProtectRegisterCallBack (
  IN CONST EFI_GUID  *Protocol,
  IN VOID            *Interface,
  IN EFI_HANDLE      EfiHandle
  )
{
  UINT16                           Data16And, Data16Or;
  ///
  /// Lock Down TCO
  ///
  Data16And = 0xFFFF;
  Data16Or = B_PCH_TCO_CNT_LOCK;
  IoOr16(PM_BASE_ADDRESS+ R_PCH_TCO_CNT, Data16Or);

  S3BootScriptSaveIoReadWrite (
    S3BootScriptWidthUint16,
    (UINTN) (PM_BASE_ADDRESS + R_PCH_TCO_CNT ),
    &Data16Or,  // Data to be ORed
    &Data16And  // Data to be ANDed
    );  

  return EFI_SUCCESS;
}
//EIP180260 <<   

EFI_STATUS
EFIAPI
InstallScBiosWriteProtect(
    IN EFI_HANDLE            ImageHandle,
    IN EFI_SYSTEM_TABLE      *SystemTable
)
/**

  @brief
  Entry point for Pch Bios Write Protect driver.

  @param[in] ImageHandle          Image handle of this driver.
  @param[in] SystemTable          Global system service table.

  @retval EFI_SUCCESS             Initialization complete.

**/
{
    EFI_STATUS                        Status;
    DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy;
    EFI_HANDLE                        SwHandle;
    EFI_SMM_SW_DISPATCH_CONTEXT       SwContext;
    UINTN                             mPciD31F0RegBase;
//EIP180260 >>       
    VOID        *NotifyReg;
//EIP180260 <<   
    ///
    /// Locate PCH Platform Policy protocol
    ///
    Status = gBS->LocateProtocol(&gDxePchPlatformPolicyProtocolGuid, NULL, &PchPlatformPolicy);
    ASSERT_EFI_ERROR(Status);
    if(EFI_ERROR(Status)) {
        DEBUG((EFI_D_ERROR | EFI_D_INFO, "Failed to locate PCH Policy protocol.\n"));
        return Status;
    }
    
    if(PchPlatformPolicy->LockDownConfig->BiosLock == PCH_DEVICE_ENABLE) {
        mPciD31F0RegBase = MmPciAddress(0,
                                        DEFAULT_PCI_BUS_NUMBER_PCH,
                                        PCI_DEVICE_NUMBER_PCH_LPC,
                                        PCI_FUNCTION_NUMBER_PCH_LPC,
                                        0
                                       );
        SpiBase          = MmioRead32(mPciD31F0RegBase + R_PCH_LPC_SPI_BASE) & B_PCH_LPC_SPI_BASE_BAR;
        ///
        /// Get the ICHn protocol
        ///
        mIchnDispatch = NULL;
        Status        = gBS->LocateProtocol(&gEfiSmmIchnDispatchProtocolGuid, NULL, &mIchnDispatch);
        ASSERT_EFI_ERROR(Status);
        ///
        /// Locate the ICH SMM SW dispatch protocol
        ///
        SwHandle  = NULL;
        Status    = gBS->LocateProtocol(&gEfiSmmSwDispatchProtocolGuid, NULL, &mSwDispatch);
        ASSERT_EFI_ERROR(Status);
        ///
        /// Register BIOS Lock SW SMI handler
        ///
        SwContext.SwSmiInputValue = PchPlatformPolicy->LockDownConfig->PchBiosLockSwSmiNumber;
        Status = mSwDispatch->Register(
                     mSwDispatch,
                     PchBiosLockSwSmiCallback,
                     &SwContext,
                     &SwHandle
                 );
        ASSERT_EFI_ERROR(Status);
    }
//EIP180260 >>    
    Status = gSmst->SmmRegisterProtocolNotify (
                     &gEfiSmmReadyToLockProtocolGuid,
                     PchBiosWriteProtectRegisterCallBack,
                     &NotifyReg
             );
    ASSERT_EFI_ERROR (Status);
//EIP180260 <<    
    return EFI_SUCCESS;
}
