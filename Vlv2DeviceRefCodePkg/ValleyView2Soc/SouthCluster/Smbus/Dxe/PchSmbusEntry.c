/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 2004 - 2011 Intel Corporation. All rights reserved
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
  PCH Smbus Driver

**/
#include "PchSmbus.h"
#include <PchRegs.h>
#include <Protocol/PchPlatformPolicy.h>
#ifndef ECP_FLAG
#include <Guid/HobList.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#endif
#include <Guid/SmbusArpMap.h>
#ifdef ECP_FLAG
EFI_GUID gEfiSmbusProtocolGuid = EFI_SMBUS_HC_PROTOCOL_GUID;
EFI_GUID gEfiSmbusArpMapGuid = EFI_SMBUS_ARP_MAP_GUID;
EFI_GUID gDxePchPlatformPolicyProtocolGuid = DXE_PCH_PLATFORM_POLICY_PROTOCOL_GUID;
#else
#include <Library/BaseMemoryLib.h>
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

  DEBUG ((EFI_D_INFO, "SmbusExecute() Start, SmbusDeviceAddress=%x, Command=%x, Operation=%x\n", (SlaveAddress.SmbusDeviceAddress << 1), Command, Operation));

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
InitializePchSmbus (
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

**/
{
  DXE_PCH_PLATFORM_POLICY_PROTOCOL  *PchPlatformPolicy;
  EFI_STATUS                        Status;
  UINTN                             DataSize;
  VOID                              *Data;
  EFI_PEI_HOB_POINTERS              HobList;

  DEBUG ((EFI_D_INFO, "InitializePchSmbus() Start\n"));

  Status = gBS->LocateProtocol (
                  &gDxePchPlatformPolicyProtocolGuid,
                  NULL,
                  (VOID **) &PchPlatformPolicy
                  );
  ASSERT_EFI_ERROR (Status);

  mSmbusContext = AllocateZeroPool (sizeof (SMBUS_INSTANCE));
  if (mSmbusContext == NULL) {
    ASSERT (FALSE);
    return EFI_OUT_OF_RESOURCES;
  }

  mSmbusContext->Signature                  = PCH_SMBUS_PRIVATE_DATA_SIGNATURE;
  mSmbusContext->IoDone                     = IoDone;
  mSmbusContext->SmbusIoRead                = SmbusIoRead;
  mSmbusContext->SmbusIoWrite               = SmbusIoWrite;
  mSmbusContext->SmbusController.Execute    = SmbusExecute;
  mSmbusContext->SmbusController.ArpDevice  = SmbusArpDevice;
  mSmbusContext->SmbusController.GetArpMap  = SmbusGetArpMap;
  mSmbusContext->SmbusController.Notify     = SmbusNotify;
  mSmbusContext->PlatformNumRsvd            = PchPlatformPolicy->SmbusConfig->NumRsvdSmbusAddresses;
  mSmbusContext->PlatformRsvdAddr           = PchPlatformPolicy->SmbusConfig->RsvdSmbusAddressTable;

  ///
  /// See if PEI already ARPed any devices, and if so, update our device map.
  ///
  /// Get Hob list
  ///
  Status = EfiGetSystemConfigurationTable (&gEfiHobListGuid, (VOID **) &HobList.Raw);
  ASSERT_EFI_ERROR (Status);

  HobList.Raw = GetNextGuidHob (&gEfiSmbusArpMapGuid, HobList.Raw);
  //
  // If we found the right hob, store the information. Otherwise, continue.
  //
  if (HobList.Raw != NULL) {
    Data      = (VOID *) ((UINT8 *) (&HobList.Guid->Name) + sizeof (EFI_GUID));
    DataSize  = HobList.Header->HobLength - sizeof (EFI_HOB_GUID_TYPE);
    CopyMem (mSmbusContext->DeviceMap, Data, DataSize);
    mSmbusContext->DeviceMapEntries = (UINT8) (DataSize / sizeof (EFI_SMBUS_DEVICE_MAP));
  }
  ///
  /// Initialize the NotifyFunctionList
  ///
  InitializeListHead (&mSmbusContext->NotifyFunctionList);

  ///
  /// Install the SMBUS interface
  ///
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mSmbusContext->Handle,
#ifdef ECP_FLAG
                  &gEfiSmbusProtocolGuid,
#else
                  &gEfiSmbusHcProtocolGuid,
#endif
                  &mSmbusContext->SmbusController,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "InitializePchSmbus() End\n"));

  return EFI_SUCCESS;
}
