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
  PchSmbus.h

  @brief
  PCH Smbus Protocol

**/
#ifndef _DXE_PCH_SMBUS_H
#define _DXE_PCH_SMBUS_H

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include <Protocol/Smbus/Smbus.h>
#else
//
// External include files do NOT need to be explicitly specified in real EDKII
// environment
//
#include <PiDxe.h>
#include <Protocol/SmbusHc.h>
#include <Protocol/Metronome.h>
#include <Protocol/PciRootBridgeIo.h>
#endif


#include "PchAccess.h"
#include "../Common/PchSmbusCommon.h"
#ifndef ECP_FLAG
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>

#include <IndustryStandard/SmBus.h>
#else
typedef
UINT8
(EFIAPI *SMBUS_IO_READ) (
  IN      UINT8                     Offset
  );

typedef
VOID
(EFIAPI *SMBUS_IO_WRITE) (
  IN      UINT8                     Offset,
  IN      UINT8                     Data
  );

typedef
BOOLEAN
(EFIAPI *SMBUS_IO_DONE) (
  IN      UINT8                     *StsReg
  );
#endif

//
// Definitions
//
#define MAX_SMBUS_DEVICES 107 // Max number of SMBus devices (7 bit
//   address yields 128 combinations but 21
//   of those are reserved)
//
#define MICROSECOND 10
#define MILLISECOND (1000 * MICROSECOND)
#define ONESECOND   (1000 * MILLISECOND)

///
/// Private Data Structures
///
typedef struct _SMBUS_NOTIFY_FUNCTION_LIST_NODE {
  UINT32                    Signature;
  LIST_ENTRY                Link;
  EFI_SMBUS_DEVICE_ADDRESS  SlaveAddress;
  UINTN                     Data;
  EFI_SMBUS_NOTIFY_FUNCTION NotifyFunction;
} SMBUS_NOTIFY_FUNCTION_LIST_NODE;

#define SMBUS_NOTIFY_FUNCTION_LIST_NODE_FROM_LINK(_node) \
  CR ( _node, SMBUS_NOTIFY_FUNCTION_LIST_NODE, Link, PCH_SMBUS_PRIVATE_DATA_SIGNATURE )

///
/// Declare a local instance structure for this driver
///
typedef struct _SMBUS_INSTANCE {
  UINTN                 Signature;
  EFI_HANDLE            Handle;

  UINT32                SmbusIoBase;
  SMBUS_IO_READ         SmbusIoRead;
  SMBUS_IO_WRITE        SmbusIoWrite;
  SMBUS_IO_DONE         IoDone;

  ///
  /// Published interface
  ///
  EFI_SMBUS_HC_PROTOCOL SmbusController;

  UINT8                 DeviceMapEntries;
  EFI_SMBUS_DEVICE_MAP  DeviceMap[MAX_SMBUS_DEVICES];

  UINT8                 PlatformNumRsvd;
  UINT8                 *PlatformRsvdAddr;

  LIST_ENTRY            NotifyFunctionList;
  EFI_EVENT             NotificationEvent;

} SMBUS_INSTANCE;

///
/// Driver global data
///
SMBUS_INSTANCE  *mSmbusContext;

///
/// Prototypes
///
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
;

EFI_STATUS
EFIAPI
InitializePchSmbus (
  IN EFI_HANDLE                         ImageHandle,
  IN EFI_SYSTEM_TABLE                   *SystemTable
  )
/**

  @brief
  Smbus driver entry point

  @param[in] ImageHandle          ImageHandle of this module
  @param[in] SystemTable          EFI System Table

  @retval EFI_SUCCESS             Driver initializes successfully
  @retval Other values            Some error occurred

**/
;

EFI_STATUS
SmbusArpDevice (
  IN CONST  EFI_SMBUS_HC_PROTOCOL         * This,
  IN        BOOLEAN                       ArpAll,
  IN        EFI_SMBUS_UDID                * SmbusUdid, OPTIONAL
  IN OUT  EFI_SMBUS_DEVICE_ADDRESS        * SlaveAddress OPTIONAL
  )
/**

  @brief
  Set Slave address for an Smbus device with a known UDID or perform a general
  ARP of all devices.

  @param[in] This                 Pointer to the instance of the EFI_SMBUS_HC_PROTOCOL.
  @param[in] ArpAll               If TRUE, do a full ARP. Otherwise, just ARP the specified UDID.
  @param[in] SmbusUdid            When doing a directed ARP, ARP the device with this UDID.
  @@param[in, out] SlaveAddress   Buffer to store new Slave Address during directed ARP. On output,If
                                  ArpAlll == TRUE, this will contain the newly assigned Slave address.

  @retval EFI_INVALID_PARAMETER   ArpAll == FALSE but SmbusUdid or SlaveAddress are NULL.
                                  Return value from SmbusFullArp() or SmbusDirectedArp().

**/
;

EFI_STATUS
SmbusGetArpMap (
  IN CONST  EFI_SMBUS_HC_PROTOCOL         *This,
  IN OUT  UINTN                           *Length,
  IN OUT  EFI_SMBUS_DEVICE_MAP            **SmbusDeviceMap
  )
/**

  @brief
  Get a pointer to the assigned mappings of UDID's to Slave Addresses.

  @param[in] This                 Pointer to the instance of the EFI_SMBUS_HC_PROTOCOL.
  @param[in] Length               Buffer to contain the lenght of the Device Map, it will be updated to
                                  contain the number of pairs of UDID's mapped to Slave Addresses.
  @param[in] SmbusDeviceMap       Buffer to contian a pointer to the Device Map, it will be updated to
                                  point to the first pair in the Device Map

  @retval EFI_SUCCESS             Function completed successfully.

**/
;

EFI_STATUS
SmbusNotify (
#ifdef ECP_FLAG
  IN EFI_SMBUS_HC_PROTOCOL         *This,
  IN EFI_SMBUS_DEVICE_ADDRESS      SlaveAddress,
  IN UINTN                         Data,
  IN EFI_SMBUS_NOTIFY_FUNCTION     NotifyFunction
#else
  IN CONST  EFI_SMBUS_HC_PROTOCOL         *This,
  IN CONST  EFI_SMBUS_DEVICE_ADDRESS      SlaveAddress,
  IN CONST  UINTN                         Data,
  IN CONST  EFI_SMBUS_NOTIFY_FUNCTION     NotifyFunction
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

  @exception EFI_UNSUPPORTED      Unable to create the event needed for notifications.
  @retval EFI_INVALID_PARAMETER   NotifyFunction was NULL.
  @retval EFI_OUT_OF_RESOURCES    Unable to allocate space to register the notification.
  @retval EFI_SUCCESS             Function completed successfully

**/
;

EFI_STATUS
InitializePeriodicEvent (
  VOID
  )
/**

  @brief
  Set up a periodic event so that we can check if any Slave Device has sent a
  Notify ARP Master command.

  @param[in] None.

  @retval EFI_SUCCESS             Periodic event successfully set up.
  @retval Other Errors            Failed to set up Periodic event.
                                  Error value from CreateEvent().
                                  Error value from SetTimer().
**/
;

VOID
PollSmbusNotify (
  IN  EFI_EVENT                         Event,
  IN  VOID                              *Context
  )
/**

  @brief
  Function to be called every time periodic event happens. This will check if
  the SMBus Host Controller has received a Host Notify command. If so, it will
  see if a notification has been reqested on that event and make any callbacks
  that may be necessary.

  @param[in] Event                The periodic event that occured and got us into this callback.
  @param[in] Context              Event context. Will be NULL in this case, since we already have our
                                  private data in a module global variable.

  @retval None

**/
;

EFI_STATUS
SmbusPrepareToArp (
  IN      SMBUS_INSTANCE                *Private
  )
/**

  @brief
  Issue a prepare ARP command to informs all devices that the ARP Master is starting the ARP process

  @param[in] Private              Pointer to the SMBUS_INSTANCE

  @retval EFI_SUCCESS             The SMBUS operation is successful
  @retval Other Values            Something error occurred

**/
;

EFI_STATUS
SmbusGetUdidGeneral (
  IN      SMBUS_INSTANCE                *Private,
  IN OUT  EFI_SMBUS_DEVICE_MAP          *DeviceMap
  )
/**

  @brief
  Issue a Get UDID (general) command to requests ARP-capable and/or Discoverable devices to
  return their slave address along with their UDID.

  @param[in] Private              Pointer to the SMBUS_INSTANCE
  @param[in, out] DeviceMap       Pointer to SMBUS device map table that slave device return

  @retval EFI_SUCCESS             The SMBUS operation is successful
  @retval Other Values            Something error occurred

**/
;

EFI_STATUS
SmbusAssignAddress (
  IN      SMBUS_INSTANCE                *Private,
  IN OUT  EFI_SMBUS_DEVICE_MAP          *DeviceMap
  )
/**

  @brief
  Issue a Assign address command to assigns an address to a specific slave device.

  @param[in] Private              Pointer to the SMBUS_INSTANCE
  @param[in, out] DeviceMap       Pointer to SMBUS device map table that send to slave device

  @retval EFI_SUCCESS             The SMBUS operation is successful
  @retval Other Values            Something error occurred

**/
;

EFI_STATUS
SmbusFullArp (
  IN      SMBUS_INSTANCE                *Private
  )
/**

  @brief
  Do a fully (general) Arp procress to assign the slave address of all ARP-capable device.
  This function will issue issue the "Prepare to ARP", "Get UDID" and "Assign Address" commands.

  @param[in] Private              Pointer to the SMBUS_INSTANCE

  @retval EFI_OUT_OF_RESOURCES    No available address to assign
  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
SmbusDirectedArp (
  IN      SMBUS_INSTANCE                *Private,
  IN CONST  EFI_SMBUS_UDID              *SmbusUdid,
  IN OUT  EFI_SMBUS_DEVICE_ADDRESS      *SlaveAddress
  )
/**

  @brief
  Do a directed Arp procress to assign the slave address of a single ARP-capable device.

  @param[in] Private              Pointer to the SMBUS_INSTANCE
  @param[in] DeviceMap            Pointer to SMBUS device map table that send to slave device
  @param[in] SmbusUdid            When doing a directed ARP, ARP the device with this UDID.
  @param[in] SlaveAddress         Buffer to store new Slave Address during directed ARP.

  @retval EFI_OUT_OF_RESOURCES    DeviceMapEntries is more than Max number of SMBus devices
                                  Or there is no available address to assign

  @retval EFI_SUCCESS             The function completed successfully

**/
;

EFI_STATUS
GetNextAvailableAddress (
  IN      SMBUS_INSTANCE                *Private,
  IN OUT  EFI_SMBUS_DEVICE_ADDRESS      *SlaveAddress
  )
/**

  @brief
  Find an available address to assign

  @param[in] Private              Pointer to the SMBUS_INSTANCE
  @param[in] SlaveAddress         Buffer to store new Slave Address during directed ARP.

  @retval EFI_OUT_OF_RESOURCES    There is no available address to assign
  @retval EFI_SUCCESS             The function completed successfully

**/
;

BOOLEAN
IsAddressAvailable (
  IN      SMBUS_INSTANCE                *Private,
  IN      EFI_SMBUS_DEVICE_ADDRESS      SlaveAddress
  )
/**

  @brief
  Check whether the address is assignable.

  @param[in] Private              Pointer to the SMBUS_INSTANCE
  @param[in] SlaveAddress         The Slave Address for checking

  @retval TRUE                    The address is assignable
  @retval FALSE                   The address is not assignable

**/
;
#endif
