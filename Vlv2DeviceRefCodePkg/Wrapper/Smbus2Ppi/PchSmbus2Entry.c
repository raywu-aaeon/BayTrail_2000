/**
  This file contains an 'Intel Peripheral Driver' and uniquely        
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your   
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the
  license agreement
**/
/**

Copyright (c) 1999 - 2011 Intel Corporation. All rights reserved
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
  PCH Smbus PEIM.

**/
#include "PchSmbus2.h"
#include <PchRegs.h>

///
/// Global variables
///
EFI_GUID  mPeiSmbusPolicyPpiGuid = PEI_SMBUS_POLICY_PPI_GUID;

///
/// Functions
///
EFI_STATUS
SmbusExecute (
  IN CONST EFI_PEI_SMBUS2_PPI         *This,
  IN       EFI_SMBUS_DEVICE_ADDRESS  SlaveAddress,
  IN       EFI_SMBUS_DEVICE_COMMAND  Command,
  IN       EFI_SMBUS_OPERATION       Operation,
  IN       BOOLEAN                   PecCheck,
  IN OUT   UINTN                     *Length,
  IN OUT   VOID                      *Buffer
  )
/**

  @brief 
  This function provides a standard way to execute an SMBUS command
  PPI as defined in the SMBus Specification. The data can either be of
  the length byte, word, or a block of data (1 to 32 bytes long). 
  The resulting transaction will be either the SMBus Slave Device accepts 
  this transaction or this function returns with an error

  @param[in] PeiServices          PEI services table pointer
  @param[in] This                 PEI_SMBUS_PPI instance
  @param[in] SlaveAddress         Smbus Slave device address
  @param[in] Command              Command to be sent
  @param[in] Operation            Which SMBus PPI will be used
  @param[in] PecCheck             Defines if Packet Error Code Checking is to be used
  @param[in] Length               How many bytes to read/write. Must be 1 <= Length <= 32 depending on the Operation
  @param[in] Buffer               Data buffer

  @retval EFI_SUCCESS             Operation success. 
                                  Length will contain the actual number of bytes read.
                                  Buffer will contain the data read.
  @retval Otherwise               Operation failed.

**/
{
  EFI_STATUS      Status;
  SMBUS_INSTANCE  *Private;

  DEBUG ((EFI_D_EVENT, "PEI Smbus2Execute() Start, SmbusDeviceAddress=%x, Command=%x, Operation=%x\n", (SlaveAddress.SmbusDeviceAddress << 1), Command, Operation));

  Private = SMBUS_PRIVATE_DATA_FROM_PPI_THIS (This);

  Status = SmbusExec (
            SlaveAddress,
            Command,
            Operation,
            PecCheck,
            Length,
            Buffer
            );
  ///
  /// Last step, check notification
  ///
  CheckNotification (Private);

  DEBUG ((EFI_D_EVENT, "PEI Smbus2Execute() End\n"));

  return Status;
}

EFI_STATUS
InitializePchSmbusPeim (
  IN      EFI_FFS_FILE_HEADER       *FfsHeader,
#ifdef ECP_FLAG 
  IN EFI_PEI_SERVICES         **PeiServices
#else
  IN      CONST EFI_PEI_SERVICES          **PeiServices
#endif
  )
/**

  @brief 
  Initialize the Smbus PPI and program the Smbus BAR

  @param[in] FfsHeader            Not used.
  @param[in] PeiServices          General purpose services available to every PEIM.

  @retval EFI_SUCCESS             The function completes successfully
  @retval EFI_OUT_OF_RESOURCES    Insufficient resources to create database

**/
{
  EFI_STATUS      Status;
  SMBUS_INSTANCE  *Private;
  UINTN           SmbusRegBase;

  DEBUG ((EFI_D_INFO, "InitializePchSmbus2Peim() Start\n"));

  Private = (SMBUS_INSTANCE *) AllocatePool (sizeof (SMBUS_INSTANCE));
  if (Private == NULL) {
    DEBUG ((EFI_D_ERROR, "Failed to allocate memory for Private! \n"));
    return EFI_OUT_OF_RESOURCES;
  }

  InitializePeiPrivate (PeiServices, Private);

  SmbusRegBase = MmPciAddress (0,
                   DEFAULT_PCI_BUS_NUMBER_PCH,
                   PCI_DEVICE_NUMBER_PCH_SMBUS,
                   PCI_FUNCTION_NUMBER_PCH_SMBUS,
                   0
                 );
  ///
  /// Since PEI has no PCI enumerator, set the BAR & I/O space enable ourselves
  ///
  MmioAndThenOr32 (SmbusRegBase + R_PCH_SMBUS_BASE, B_PCH_SMBUS_BASE_BAR, Private->SmbusIoBase);

  MmioOr8 (SmbusRegBase + R_PCH_SMBUS_PCICMD, B_PCH_SMBUS_PCICMD_IOSE);

  ///
  /// Reset the SMBus host controller
  ///
  MmioOr8 (SmbusRegBase + R_PCH_SMBUS_HOSTC, B_PCH_SMBUS_HOSTC_SSRESET);

  ///
  /// Enable the SMBus host controller
  ///
  MmioAndThenOr8 (
    SmbusRegBase + R_PCH_SMBUS_HOSTC,
    (UINT8) (~(B_PCH_SMBUS_HOSTC_SMI_EN | B_PCH_SMBUS_HOSTC_I2C_EN)),
    B_PCH_SMBUS_HOSTC_HST_EN
    );

  ///
  /// Clear Status Register before anyone uses the interfaces
  ///
  SmbusIoWrite (R_PCH_SMBUS_HSTS, B_PCH_SMBUS_HSTS_ALL);

  Status = PeiServicesInstallPpi (&Private->PpiDescriptor);
  ASSERT_EFI_ERROR (Status);

  ///
  /// Install a call-back for the permanent-memory so that we can fix up internal pointers
  ///
  Status = (**PeiServices).NotifyPpi (PeiServices, &Private->NotifyDescriptor);
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "InitializePchSmbus2Peim() End\n"));

  return EFI_SUCCESS;
}

VOID
InitializePeiPrivate (
#ifdef ECP_FLAG 
  IN EFI_PEI_SERVICES         **PeiServices,
#else
  IN  CONST EFI_PEI_SERVICES  **PeiServices,
#endif
  IN  SMBUS_INSTANCE    *Private
  )
/**

  @brief 
  This function initializes the SmBus driver in PEI.

  @param[in] PeiServices          Standard PEI services
  @param[in] Private              SMBUS private data structure

  @retval None.

**/
{
  EFI_STATUS  Status;

  Private->Signature    = PCH_SMBUS_PRIVATE_DATA_SIGNATURE;
  Private->PeiServices  = PeiServices;

  Status = (**PeiServices).LocatePpi (
                            PeiServices,
                            &mPeiSmbusPolicyPpiGuid,
                            0,
                            NULL,
                            &(Private->SmbusPolicy)
                            );
  ASSERT_EFI_ERROR (Status);

  Private->SmbusIoBase              = Private->SmbusPolicy->BaseAddress;

  Private->PpiDescriptor.Flags      = EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST;
  Private->PpiDescriptor.Guid       = &gEfiPeiSmbus2PpiGuid;

  Private->PpiDescriptor.Ppi        = &Private->SmbusPpi;

  Private->SmbusPpi.Execute         = SmbusExecute;
  Private->SmbusPpi.ArpDevice       = SmbusArpDevice;
  Private->SmbusPpi.GetArpMap       = SmbusGetArpMap;
  Private->SmbusPpi.Notify          = SmbusNotify;

  Private->NotifyDescriptor.Flags   = (EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST);
  Private->NotifyDescriptor.Guid    = &gEfiPeiMemoryDiscoveredPpiGuid;
  Private->NotifyDescriptor.Notify  = MemoryDiscoveredPpiNotifyCallback;

  Private->DeviceMapEntries         = 0;
  Private->PlatformNumRsvd          = Private->SmbusPolicy->NumRsvdAddress;
  Private->PlatformRsvdAddr         = Private->SmbusPolicy->RsvdAddress;

  Private->NotifyFunctionNum        = 0;

  return;
}

EFI_STATUS
EFIAPI
MemoryDiscoveredPpiNotifyCallback (
  IN EFI_PEI_SERVICES             **PeiServices,
  IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
  IN VOID                         *Ppi
  )
/**

  @brief 
  Fix up pointers since they are located in real memory now.

  @param[in] PeiServices          General purpose services available to every PEIM.
  @param[in] NotifyDescriptor     The notification structure this PEIM registered on install.
  @param[in] Ppi                  The memory discovered PPI.  Not used.

  @retval EFI_SUCCESS             The function completed successfully.

**/
{
  SMBUS_INSTANCE  *Private;

  Private = SMBUS_PRIVATE_DATA_FROM_NOTIFY_THIS (NotifyDescriptor);

  InitializePeiPrivate (PeiServices, Private);

  return EFI_SUCCESS;
}
