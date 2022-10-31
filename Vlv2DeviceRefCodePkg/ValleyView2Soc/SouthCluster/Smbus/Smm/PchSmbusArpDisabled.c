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
  PchSmbusArpDisable.c

  @brief
  PCH Smbus Driver, ARP functions not supported

**/
#include "PchSmbus.h"

EFI_STATUS
SmbusArpDevice (
  IN CONST   EFI_SMBUS_HC_PROTOCOL         * This,
  IN CONST   BOOLEAN                       ArpAll,
  IN CONST   EFI_SMBUS_UDID                * SmbusUdid, OPTIONAL
  IN OUT  EFI_SMBUS_DEVICE_ADDRESS         * SlaveAddress OPTIONAL
  )
/**

  @brief
  Set Slave address for an Smbus device with a known UDID or perform a general
  ARP of all devices.

  @param[in] This                 Pointer to the instance of the EFI_SMBUS_HC_PROTOCOL.
  @param[in] ArpAll               If TRUE, do a full ARP. Otherwise, just ARP the specified UDID.
  @param[in] SmbusUdid            When doing a directed ARP, ARP the device with this UDID.
  @param[in] SlaveAddress         Buffer to store new Slave Address during directed ARP. On output,If
                                  ArpAlll == TRUE, this will contain the newly assigned Slave address.

  @exception EFI_UNSUPPORTED      This functionality is not supported

**/
{
  ///
  /// ARP should be done in DXE SMBUS driver.
  /// Not needed here.
  ///
  return EFI_UNSUPPORTED;
}

EFI_STATUS
SmbusGetArpMap (
  IN CONST   EFI_SMBUS_HC_PROTOCOL         *This,
  IN OUT  UINTN                         *Length,
  IN OUT  EFI_SMBUS_DEVICE_MAP          **SmbusDeviceMap
  )
/**

  @brief
  Get a pointer to the assigned mappings of UDID's to Slave Addresses.

  @param[in] This                 Pointer to the instance of the EFI_SMBUS_HC_PROTOCOL.
  @param[in] Length               Buffer to contain the lenght of the Device Map, it will be updated to
                                  contain the number of pairs of UDID's mapped to Slave Addresses.
  @param[in] SmbusDeviceMap       Buffer to contian a pointer to the Device Map, it will be updated to
                                  point to the first pair in the Device Map

  @exception EFI_UNSUPPORTED      This functionality is not supported

**/
{
  ///
  /// ARP should be done in DXE SMBUS driver.
  /// Not needed here.
  ///
  return EFI_UNSUPPORTED;
}

EFI_STATUS
SmbusNotify (
#ifdef ECP_FLAG
  IN EFI_SMBUS_HC_PROTOCOL         *This,
  IN EFI_SMBUS_DEVICE_ADDRESS      SlaveAddress,
  IN UINTN                         Data,
  IN EFI_SMBUS_NOTIFY_FUNCTION     NotifyFunction
#else
  IN CONST   EFI_SMBUS_HC_PROTOCOL         *This,
  IN CONST   EFI_SMBUS_DEVICE_ADDRESS      SlaveAddress,
  IN CONST   UINTN                         Data,
  IN CONST   EFI_SMBUS_NOTIFY_FUNCTION     NotifyFunction
#endif
  )
/**

  @brief
  Register a callback in the event of a Host Notify command being sent by a
  specified Slave Device.

  @param[in] This                 Pointer to the instance of the EFI_SMBUS_HC_PROTOCOL.
  @param[in] SlaveAddress         Address of the device whose Host Notify command we want to
                                  trap.
  @param[in] Data                 Data of the Host Notify command we want to trap.
  @param[in] NotifyFunction       Function to be called in the event the desired Host Notify
                                  command occurs.

  @exception EFI_UNSUPPORTED      This functionality is not supported

**/
{
  ///
  /// Not needed for SMM.
  ///
  return EFI_UNSUPPORTED;
}
