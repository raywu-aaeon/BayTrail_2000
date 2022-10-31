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
  PchPciExpressHelpersLib.h

  @brief
  Header file for PCH Pci Express helps library

**/
#ifndef _PCH_PCI_EXPRESS_HELPERS_LIB_H_
#define _PCH_PCI_EXPRESS_HELPERS_LIB_H_

///
/// function prototypes
///
UINT8
PcieFindCapId (
  IN UINT8   Bus,
  IN UINT8   Device,
  IN UINT8   Function,
  IN UINT8   CapId
  )
/**

  @brief
  Find the Offset to a given Capabilities ID
  CAPID list:
    0x01 = PCI Power Management Interface
    0x04 = Slot Identification
    0x05 = MSI Capability
    0x10 = PCI Express Capability

  @param[in] Bus                  Pci Bus Number
  @param[in] Device               Pci Device Number
  @param[in] Function             Pci Function Number
  @param[in] CapId                CAPID to search for

  @retval 0                       CAPID not found
  @retval Other                   CAPID found, Offset of desired CAPID

**/
;

UINT16
PcieFindExtendedCapId (
  IN UINT8   Bus,
  IN UINT8   Device,
  IN UINT8   Function,
  IN UINT16  CapId
  )
/**

  @brief
  Search and return the offset of desired Pci Express Capability ID
  CAPID list:
    0x0001 = Advanced Error Rreporting Capability
    0x0002 = Virtual Channel Capability
    0x0003 = Device Serial Number Capability
    0x0004 = Power Budgeting Capability

  @param[in] Bus                  Pci Bus Number
  @param[in] Device               Pci Device Number
  @param[in] Function             Pci Function Number
  @param[in] CapId                Extended CAPID to search for

  @retval 0                       CAPID not found
  @retval Other                   CAPID found, Offset of desired CAPID

**/
;

EFI_STATUS
PcieMapTcxVc0 (
  IN UINT8   Bus1,
  IN UINT8   Device1,
  IN UINT8   Function1,
  IN UINT8   Bus2,
  IN UINT8   Device2,
  IN UINT8   TCx
  )
/**

  @brief
  Map a TC to VC0 for port and endpoint

  @param[in] Bus1                 The bus number of the port
  @param[in] Device1              The device number of the port
  @param[in] Function1            The function number of the port
  @param[in] Bus2                 The bus number of the endpoint
  @param[in] Device2              The device number of the endpoint
  @param[in] TCx                  The TC number

  @exception EFI_UNSUPPORTED      Unsupported operation.
  @retval EFI_SUCCESS             Successfully completed.

**/
;

EFI_STATUS
PcieSetCommonClock (
  IN UINT8   Bus1,
  IN UINT8   Device1,
  IN UINT8   Function1,
  IN UINT8   Bus2,
  IN UINT8   Device2
  )
/**

  @brief
  Set Common clock to Root port and Endpoint PCI device

  @param[in] Bus1                 Root port Pci Bus Number
  @param[in] Device1              Root port Pci Device Number
  @param[in] Function1            Root port Pci Function Number
  @param[in] Bus2                 Endpoint Pci Bus Number
  @param[in] Device2              Endpoint Pci Device Number

  @exception EFI_UNSUPPORTED      Unsupported operation.
  @retval EFI_SUCCESS             VC mapping correctly initialized

**/
;

VOID
PcieSetClkreq (
  IN  UINT8   Bus,
  IN  UINT8   Device
  )
/**

  @brief
  This function enables the CLKREQ# PM on all the end point functions

  @param[in] Bus                  Pci Bus Number
  @param[in] Device               Pci Device Number

  @retval None

**/
;

EFI_STATUS
PcieMaxPayloadSize (
  IN      UINT8           EndPointBus,
  IN      UINT8           EndPointDevice,
  IN OUT  UINT16          *MaxPayload,
  IN      BOOLEAN         Operation
  )
/**

  @brief
  This function get or set the Max Payload Size on all the end point functions

  @param[in] EndPointBus          The Bus Number of the Endpoint
  @param[in] EndPointDevice       The Device Number of the Endpoint
  @param[in] MaxPayload           The Max Payolad Size of the root port
  @param[in] Operation            True:  Set the Max Payload Size on all the end point functions
                                  False: Get the Max Payload Size on all the end point functions

  @retval EFI_SUCCESS             Successfully completed.

**/
;

EFI_STATUS
PcieSetEoiFwdDisable (
  IN  UINT8   RootBus,
  IN  UINT8   RootDevice,
  IN  UINT8   RootFunction,
  IN  UINT8   EndPointBus,
  IN  UINT8   EndPointDevice
  )
/**

  @brief
  This function disable the forwarding of EOI messages unless it discovers
  an IOAPIC behind this root port.

  @param[in] RootBus              The Bus Number of the root port
  @param[in] RootDevice           The Device Number of the root port
  @param[in] RootFunction         The Function Number of the root port
  @param[in] EndPointBus          The Bus Number of the Endpoint
  @param[in] EndPointDevice       The Device Number of the Endpoint

  @exception EFI_UNSUPPORTED      Unsupported operation.
  @retval EFI_SUCCESS             Successfully completed.

**/
;

EFI_STATUS
PcieSetPm (
  IN  UINT8                         RootBus,
  IN  UINT8                         RootDevice,
  IN  UINT8                         RootFunction,
  IN  PCH_PCI_EXPRESS_ASPM_CONTROL  RootPortAspm,
  IN  UINT8                         NumOfDevAspmOverride,
  IN  PCH_PCIE_DEVICE_ASPM_OVERRIDE *DevAspmOverride,
  IN  UINT8                         TempBusNumberMin,
  IN  UINT8                         TempBusNumberMax,
  IN  UINT8                         NumOfDevltrOverride,
  IN  PCH_PCIE_DEVICE_LTR_OVERRIDE  *DevLtrOverride
  )
/**

  @brief
  This function perform the ASPM setting for root port and downstream device

  @param[in] RootBus              Pci Bus Number of the root port
  @param[in] RootDevice           Pci Device Number of the root port
  @param[in] RootFunction         Pci Function Number of the root port
  @param[in] RootPortAspm         Root port Aspm configuration
  @param[in] NumOfDevAspmOverride Number of Device specific ASPM policy override items
  @param[in] DevAspmOverride      Pointer to array of Device specific ASPM policy override items
  @param[in] TempBusNumberMin     Minimal temp bus number that can be assigned to the root port (as secondary
                                  bus number) and its down stream switches
  @param[in] TempBusNumberMax     Maximal temp bus number that can be assigned to the root port (as subordinate
                                  bus number) and its down stream switches

  @retval EFI_SUCCESS             The function completed successfully
  @exception EFI_UNSUPPORTED      The pointer to the Port PCI Express Capability Structure is not found

**/
;

EFI_STATUS
PchPcieInitRootPortDownstreamDevices (
  IN UINT8                          RootPortBus,
  IN UINT8                          RootPortDevice,
  IN UINT8                          RootPortFunc,
  IN UINT8                          TempBusNumberMin,
  IN UINT8                          TempBusNumberMax
  )
/**

  @brief
  Initializes the root port and its down stream devices

  @param[in] RootPortBus          Pci Bus Number of the root port
  @param[in] RootPortDevice       Pci Device Number of the root port
  @param[in] RootPortFunc         Pci Function Number of the root port
  @param[in] TempBusNumberMin     Minimal temp bus number that can be assigned to the root port (as secondary
                                  bus number) and its down stream switches
  @param[in] TempBusNumberMax     Maximal temp bus number that can be assigned to the root port (as subordinate
                                  bus number) and its down stream switches

  @retval EFI_SUCCESS             Successfully completed
  @retval EFI_NOT_FOUND           Can not find device.

**/
;
#endif
