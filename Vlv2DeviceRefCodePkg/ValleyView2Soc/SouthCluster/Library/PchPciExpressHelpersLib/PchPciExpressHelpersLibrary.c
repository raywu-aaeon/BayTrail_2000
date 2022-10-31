/**
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
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
  PchPciExpressHelpersLibrary.c

  @brief
  This file contains routines that support PCI Express initialization

**/
#include "PchPciExpressHelpersLibrary.h"

#include "Library/PchPlatformLib.h"

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
{
  UINT8 CapHeaderOffset;
  UINT8 CapHeaderId;
  UINTN DeviceBase;

  DeviceBase = MmPciAddress (0, Bus, Device, Function, 0);

  ///
  /// Check the header layout to determine the Offset of Capabilities Pointer Register
  ///
  if ((MmioRead8 (DeviceBase + PCI_HEADER_TYPE_OFFSET) & HEADER_LAYOUT_CODE) == (HEADER_TYPE_CARDBUS_BRIDGE)) {
    ///
    /// If CardBus bridge, start at Offset 0x14
    ///
    CapHeaderOffset = 0x14;
  } else {
    ///
    /// Otherwise, start at Offset 0x34
    ///
    CapHeaderOffset = 0x34;
  }
  ///
  /// Get Capability Header, A pointer value of 00h is used to indicate the last capability in the list.
  ///
  CapHeaderId     = 0;
  CapHeaderOffset = MmioRead8 (DeviceBase + CapHeaderOffset) & ((UINT8) ~(BIT0 | BIT1));
  while (CapHeaderOffset != 0 && CapHeaderId != 0xFF) {
    CapHeaderId = MmioRead8 (DeviceBase + CapHeaderOffset);
    if (CapHeaderId == CapId) {
      return CapHeaderOffset;
    }
    ///
    /// Each capability must be DWORD aligned.
    /// The bottom two bits of all pointers (including the initial pointer at 34h) are reserved
    /// and must be implemented as 00b although software must mask them to allow for future uses of these bits.
    ///
    CapHeaderOffset = MmioRead8 (DeviceBase + CapHeaderOffset + 1) & ((UINT8) ~(BIT0 | BIT1));
  }

  return 0;
}

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
{
  UINT16  CapHeaderOffset;
  UINT16  CapHeaderId;
  UINTN   DeviceBase;

  DeviceBase = MmPciAddress (0, Bus, Device, Function, 0);

  ///
  /// Start to search at Offset 0x100
  /// Get Capability Header, A pointer value of 00h is used to indicate the last capability in the list.
  ///
  CapHeaderId     = 0;
  CapHeaderOffset = 0x100;
  while (CapHeaderOffset != 0 && CapHeaderId != 0xFFFF) {
    CapHeaderId = MmioRead16 (DeviceBase + CapHeaderOffset);
    if (CapHeaderId == CapId) {
      return CapHeaderOffset;
    }
    ///
    /// Each capability must be DWORD aligned.
    /// The bottom two bits of all pointers are reserved and must be implemented as 00b
    /// although software must mask them to allow for future uses of these bits.
    ///
    CapHeaderOffset = (MmioRead16 (DeviceBase + CapHeaderOffset + 2) >> 4) & ((UINT16) ~(BIT0 | BIT1));
  }

  return 0;
}

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
{
  UINT16  Offset;
  UINTN   DeviceBase1;
  UINTN   DeviceBase2;
  UINT8   DeviceIndex;
  UINT8   FunctionIndex;
  UINT8   Function2;

  DeviceBase1 = MmPciAddress (0, Bus1, Device1, Function1, 0);

  ///
  /// Set TCx-VC0 value on the port
  ///
  Offset = PcieFindExtendedCapId (Bus1, Device1, Function1, 2);
  if (Offset == 0) {
    return EFI_UNSUPPORTED;
  }

  MmioAndThenOr8 (DeviceBase1 + Offset + 0x014, (UINT8) (~0xF), 1);
  MmioWrite8 (DeviceBase1 + Offset + 0x014, (UINT8) (1 << TCx));

  ///
  /// Set TCx-VC0 value on the Endpoint
  ///
  for (DeviceIndex = 0; DeviceIndex <= Device2; DeviceIndex++) {
    DeviceBase2 = MmPciAddress (0, Bus2, DeviceIndex, 0, 0);
    if (MmioRead16 (DeviceBase2 + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
      continue;
    }
    ///
    /// Check if EndPoint device is Multi-Function Device
    ///
    if (MmioRead8 (DeviceBase2 + PCI_HEADER_TYPE_OFFSET) & HEADER_TYPE_MULTI_FUNCTION) {
      ///
      /// If multi-function Device, check function 0-7
      ///
      Function2 = PCI_MAX_FUNC;
    } else {
      ///
      /// Otherwise, check function 0 only
      ///
      Function2 = 0;
    }

    for (FunctionIndex = 0; FunctionIndex <= Function2; FunctionIndex++) {
      DeviceBase2 = MmPciAddress (0, Bus2, DeviceIndex, FunctionIndex, 0);

      Offset      = PcieFindExtendedCapId (Bus2, DeviceIndex, FunctionIndex, 2);
      if (Offset == 0) {
        continue;
      }

      MmioAndThenOr8 (DeviceBase2 + Offset + 0x014, (UINT8) (~0xF), 1);
      MmioWrite8 (DeviceBase2 + Offset + 0x014, (UINT8) (1 << TCx));
    }
  }

  return EFI_SUCCESS;
}

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
{
  UINT8       CapOffset1;
  UINT8       CapOffset2;
  BOOLEAN     CommonClockSupport;
  EFI_STATUS  Status;
  UINTN       DeviceBase1;
  UINTN       DeviceBase2;
  UINT16      RegData16;
  UINT8       DeviceIndex;
  UINT8       FunctionIndex;
  UINT8       Function2;

  DeviceBase1 = MmPciAddress (0, Bus1, Device1, Function1, 0);

  ///
  /// Get the pointer to the Port PCI Express Capability Structure.
  ///
  CommonClockSupport  = FALSE;
  CapOffset1          = PcieFindCapId (Bus1, Device1, Function1, EFI_PCI_CAPABILITY_ID_PCIEXP);
  if (CapOffset1 == 0) {
    return EFI_UNSUPPORTED;
  }
  ///
  /// Check the Port Slot Clock Configuration Bit.
  ///
  if ((MmioRead16 (DeviceBase1 + CapOffset1 + 0x012) & BIT12) == 0) {
    return EFI_UNSUPPORTED;
  }

  for (DeviceIndex = 0; DeviceIndex <= Device2; DeviceIndex++) {
    DeviceBase2 = MmPciAddress (0, Bus2, DeviceIndex, 0, 0);
    if (MmioRead16 (DeviceBase2 + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
      continue;
    }
    ///
    /// Check if EndPoint device is Multi-Function Device
    ///
    if (MmioRead8 (DeviceBase2 + PCI_HEADER_TYPE_OFFSET) & HEADER_TYPE_MULTI_FUNCTION) {
      ///
      /// If multi-function Device, check function 0-7
      ///
      Function2 = PCI_MAX_FUNC;
    } else {
      ///
      /// Otherwise, check function 0 only
      ///
      Function2 = 0;
    }

    for (FunctionIndex = 0; FunctionIndex <= Function2; FunctionIndex++) {
      DeviceBase2 = MmPciAddress (0, Bus2, DeviceIndex, FunctionIndex, 0);
      ///
      /// Check the Endpoint Slot Clock Configuration Bit.
      ///
      CapOffset2 = PcieFindCapId (Bus2, DeviceIndex, FunctionIndex, EFI_PCI_CAPABILITY_ID_PCIEXP);
      if ((CapOffset2 != 0) && ((MmioRead16 (DeviceBase2 + CapOffset2 + 0x012) & BIT12) != 0)) {
        ///
        /// Common clock is supported, set common clock bit on root port
        /// and the endpoint
        ///
        if (CommonClockSupport == FALSE) {
          MmioOr8 (DeviceBase1 + CapOffset1 + 0x010, BIT6);
          CommonClockSupport = TRUE;
        }

        MmioOr8 (DeviceBase2 + CapOffset2 + 0x010, BIT6);
      }
    }
  }
  ///
  /// If common clock supported on root port and endpoint, retrain,
  /// otherwise, return failure.
  ///
  if (CommonClockSupport == FALSE) {
    Status = EFI_UNSUPPORTED;
  } else {
    ///
    /// Retrain the Link per PCI Express Specification.
    ///
    MmioOr8 (DeviceBase1 + CapOffset1 + 0x010, BIT5);

    ///
    /// Wait until Re-Training has completed.
    ///
    do {
      RegData16 = MmioRead16 (DeviceBase1 + CapOffset1 + 0x012) & BIT11;
    } while (RegData16 != 0);

    Status = EFI_SUCCESS;
  }

  return Status;
}

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
{
  UINT8 CapOffset;
  UINTN DeviceBase;
  UINT8 DeviceIndex;
  UINT8 FunctionIndex;
  UINT8 Function;

  for (DeviceIndex = 0; DeviceIndex <= Device; DeviceIndex++) {
    DeviceBase = MmPciAddress (0, Bus, DeviceIndex, 0, 0);
    if (MmioRead16 (DeviceBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
      continue;
    }
    ///
    /// Check if EndPoint device is Multi-Function Device
    ///
    if (MmioRead8 (DeviceBase + PCI_HEADER_TYPE_OFFSET) & HEADER_TYPE_MULTI_FUNCTION) {
      ///
      /// If multi-function Device, check function 0-7
      ///
      Function = PCI_MAX_FUNC;
    } else {
      ///
      /// Otherwise, check function 0 only
      ///
      Function = 0;
    }
    ///
    /// Parse thro all the functions of the endpoint and find the PCIe Cap ID (offset 10h) and if
    /// exists then enable the CLKREQ# bit (BIT8) on that function
    ///
    for (FunctionIndex = 0; FunctionIndex <= Function; FunctionIndex++) {
      ///
      /// Find the PCIe Cap Id (offset 10h)
      ///
      CapOffset = PcieFindCapId (Bus, DeviceIndex, FunctionIndex, EFI_PCI_CAPABILITY_ID_PCIEXP);
      if (CapOffset == 0) {
        continue;
      }

      DeviceBase = MmPciAddress (0, Bus, DeviceIndex, FunctionIndex, 0);
      ///
      /// Check if CLKREQ# is supported by the endpoints
      ///
      if ((MmioRead32 (DeviceBase + CapOffset + 0x0C) & BIT18) == 0) {
        ///
        /// CLKREQ# is not supported so dont do anything
        ///
        return ;
      }
    }
    ///
    /// Now enable the CLKREQ#
    ///
    for (FunctionIndex = 0; FunctionIndex <= Function; FunctionIndex++) {
      ///
      /// Find the PCIe Cap Id (offset 10h)
      ///
      CapOffset = PcieFindCapId (Bus, DeviceIndex, FunctionIndex, EFI_PCI_CAPABILITY_ID_PCIEXP);
      if (CapOffset == 0) {
        continue;
      }

      DeviceBase = MmPciAddress (0, Bus, DeviceIndex, FunctionIndex, 0);
      MmioOr16 (DeviceBase + CapOffset + 0x010, BIT8);
    }
  }
}

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
{
  UINTN   DeviceBase;
  UINT8   PcieCapOffset;
  UINT16  EndPointMaxPayload;
  UINT8   DeviceIndex;
  UINT8   FunctionIndex;
  UINT8   EndPointFunction;

  ///
  /// Obtain the Max Payload Size for all the end point functions
  ///
  for (DeviceIndex = 0; DeviceIndex <= EndPointDevice; DeviceIndex++) {
    DeviceBase = MmPciAddress (0, EndPointBus, DeviceIndex, 0, 0);
    if (MmioRead16 (DeviceBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
      continue;
    }
    ///
    /// Check if EndPoint device is Multi-Function Device
    ///
    if (MmioRead8 (DeviceBase + PCI_HEADER_TYPE_OFFSET) & HEADER_TYPE_MULTI_FUNCTION) {
      ///
      /// If multi-function Device, check function 0-7
      ///
      EndPointFunction = PCI_MAX_FUNC;
    } else {
      ///
      /// Otherwise, check function 0 only
      ///
      EndPointFunction = 0;
    }

    for (FunctionIndex = 0; FunctionIndex <= EndPointFunction; FunctionIndex++) {
      DeviceBase = MmPciAddress (0, EndPointBus, DeviceIndex, FunctionIndex, 0);
      if (MmioRead16 (DeviceBase + 0x0) != 0xFFFF) {
        ///
        /// Get the pointer to the Endpoint PCI Express Capability Structure.
        ///
        PcieCapOffset = PcieFindCapId (EndPointBus, DeviceIndex, FunctionIndex, EFI_PCI_CAPABILITY_ID_PCIEXP);
        if (PcieCapOffset == 0) {
          continue;
        }

        if (Operation == TRUE) {
          ///
          /// Set the Max Payload Size of the end point function
          ///
          MmioAndThenOr16 (
            DeviceBase + PcieCapOffset + 0x08,
            (UINT16)~(BIT7 | BIT6 | BIT5),
            *MaxPayload << 5
            );
        } else {
          ///
          /// Get the end point function Max Payload Size support
          ///
          EndPointMaxPayload = MmioRead16 (DeviceBase + PcieCapOffset + 0x04) & (BIT2 | BIT1 | BIT0);
          ///
          /// Obtain the minimum Max Payload Size between the PCIE root Port and the end point functions
          ///
          if (*MaxPayload > EndPointMaxPayload) {
            *MaxPayload = EndPointMaxPayload;
          }
        }
      }
    }
  }

  return EFI_SUCCESS;
}

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
{
  BOOLEAN IoApicBehind;
  UINTN   RootDeviceBase;
  UINTN   DeviceBase;
  UINT8   ProgInterface;
  UINT8   SubClassCode;
  UINT8   BaseClassCode;
  UINT8   DeviceIndex;
  UINT8   FunctionIndex;
  UINT8   EndPointFunction;

  IoApicBehind    = FALSE;
  RootDeviceBase  = MmPciAddress (0, RootBus, RootDevice, RootFunction, 0);

  ///
  /// Check if an IOAPIC behind the root port
  ///
  for (DeviceIndex = 0; DeviceIndex <= EndPointDevice; DeviceIndex++) {
    DeviceBase = MmPciAddress (0, EndPointBus, DeviceIndex, 0, 0);
    if (MmioRead16 (DeviceBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
      continue;
    }
    ///
    /// Check if EndPoint device is Multi-Function Device
    ///
    if (MmioRead8 (DeviceBase + PCI_HEADER_TYPE_OFFSET) & HEADER_TYPE_MULTI_FUNCTION) {
      ///
      /// If multi-function Device, check function 0-7
      ///
      EndPointFunction = PCI_MAX_FUNC;
    } else {
      ///
      /// Otherwise, check function 0 only
      ///
      EndPointFunction = 0;
    }

    for (FunctionIndex = 0; FunctionIndex <= EndPointFunction; FunctionIndex++) {
      DeviceBase    = MmPciAddress (0, EndPointBus, DeviceIndex, FunctionIndex, 0);
      BaseClassCode = MmioRead8 (DeviceBase + PCI_CLASSCODE_OFFSET + 2);
      SubClassCode  = MmioRead8 (DeviceBase + PCI_CLASSCODE_OFFSET + 1);
      ProgInterface = MmioRead8 (DeviceBase + PCI_CLASSCODE_OFFSET);

      if ((BaseClassCode == PCI_CLASS_SYSTEM_PERIPHERAL) &&
          (SubClassCode == PCI_SUBCLASS_PIC) &&
          ((ProgInterface == PCI_IF_APIC_CONTROLLER) ||
           (ProgInterface == PCI_IF_APIC_CONTROLLER2))) {
        IoApicBehind = TRUE;
      }
    }
  }
  ///
  /// PCH BIOS Spec Rev 0.5.0, Section 8.14 Additional PCI Express* Programming Steps
  /// Step 20
  /// If there is no IOAPIC behind the root port, set EOI Forwarding Disable bit (B0:D28:F0-F7:D4h[1]) to 1b.
  ///
  //AMI_OVERRIDE - Workaround EIP144446 PCIe devices have yellow mark under Windows 8 when plugged on a PCIe to PCIe bridge >>
  /*  
  if (IoApicBehind == FALSE) {
    MmioOr8 (RootDeviceBase + R_PCH_PCIE_MPC2, (UINT8) (B_PCH_PCIE_MPC2_EOIFD));
  }
  */
  //AMI_OVERRIDE - Workaround EIP144446 PCIe devices have yellow mark under Windows 8 when plugged on a PCIe to PCIe bridge <<

  return EFI_SUCCESS;
}

typedef enum {
  CalculateAspm,
  ManualAspm,
  SetAspm
} OPERATION;

VOID
UpdateLtrOverrideValue (
  IN OUT  UINT16   *LatencyValue1,
  IN      UINT16   *LatencyValue2
  )
/**

@brief

  This function compares the actual latency in LatencyValue1
  with actual latency in LatencyValue2 and stores the minimum
  back to LatencyValue1, in the required format.

  @param[in] LatencyValue1  - Current latency value
  @param[in] LatencyValue2  - Latency value from the Table

**/
{
  UINT8      Scale1;
  UINT8      Scale2;
  UINT64     ActualLatency1 = 0;
  UINT64     ActualLatency2 = 0;
  UINT32     Multiplier[6] = {
    1,
    32,
    1024,
    32768,
    1048576,
    33554432
  };

  Scale1 = LTR_MULTIPLIER_INDEX(*LatencyValue1);
  Scale2 = LTR_MULTIPLIER_INDEX(*LatencyValue2);
  if ((Scale1 <= 5) && (Scale2 <= 5)) {
    ActualLatency1 = LTR_LATENCY_VALUE(*LatencyValue1) * Multiplier[Scale1];
    ActualLatency2 = LTR_LATENCY_VALUE(*LatencyValue2) * Multiplier[Scale2];
  }

  ///
  /// Store the lower latency value and corresponding scale bits back to LatencyValue1
  ///
  if (ActualLatency1 > ActualLatency2) {
    *LatencyValue1 = *LatencyValue2;
  }
}

EFI_STATUS
PcieEndPointPm (
  IN      UINTN                             RootDeviceBase,
  IN      UINT32                            RootPcieCapOffset,
  IN      UINT8                             EndPointBus,
  IN      UINT8                             NumOfDevAspmOverride,
  IN      PCH_PCIE_DEVICE_ASPM_OVERRIDE     *DevAspmOverride,
  IN OUT  UINT16                            *LinkAspmVal,
  IN      OPERATION                         Operation,
  IN      UINT8                             NumOfDevLtrOverride,
  IN      PCH_PCIE_DEVICE_LTR_OVERRIDE      *DevLtrOverride,
  IN OUT  UINT32                            *LtrOverrideVal
  )
/**

  @brief
  Calculate/Set EndPoint device Power management settings

  @param[in] RootDeviceBase       The Root Port PCI Express address
  @param[in] RootPcieCapOffset    The pointer to the Root Port PCI Express Capability Structure
  @param[in] EndPointBus          The Bus Number of the Endpoint
  @param[in] NumOfDevAspmOverride Number of Device specific ASPM policy override items
  @param[in] DevAspmOverride      Pointer to array of Device specific ASPM policy override items
  @param[in] LinkAspmVal          Resulting Link ASPM value programmed
  @param[in] Operation            Operation Types

  @retval EFI_SUCCESS             Successfully completed
  @retval EFI_NOT_FOUND           Can not find device

  @retval EFI_OUT_OF_RESOURCES    The endpoint device is a bridge, but the Subordinate Bus Number of
                                  the root port is not greater than its Secondary Bus Number. You may
                                  get this error if PCI emulation is not done before this function gets
                                  called and the platform policy settings of "TempRootPortBusNumMax" and
                                  "TempRootPortBusNumMin" do not provide enough resource for temp bus
                                  number usage.

**/
{
  EFI_STATUS  Status;
  UINTN       EndPointBase;
  UINT8       EndPointFunction;
  UINT8       EndPointPcieCapOffset;
  UINT8       PcieDeviceIndex;
  UINT16      EndPointAspm;
  UINT16      EndPointVendorId;
  UINT16      EndPointDeviceId;
  UINT8       EndPointRevId;
  UINT8       EndPointBaseClassCode;
  UINT8       EndPointSubClassCode;
  UINT32      PortLxLat;
  UINT32      EndPointLxLat;
  UINT32      LxLat;
  UINT8       DownStreamBusMin;
  UINT8       ClassCode;
  UINT8       RootDevSubBusNum;
  BOOLEAN     BusAssign;
  UINT8       DeviceIndex;
  UINT8       FunctionIndex;
  UINT16      LtrExtendedCapOffset;
  UINT32      DeviceCapabilities2;
  UINT16      Data16;
  UINT32      Data32;
  BOOLEAN     OverrideEntry;

  for (DeviceIndex = 0; DeviceIndex <= PCI_MAX_DEVICE; DeviceIndex++) {
    EndPointBase = MmPciAddress (0, EndPointBus, DeviceIndex, 0, 0);
    if (MmioRead16 (EndPointBase + PCI_VENDOR_ID_OFFSET) == 0xFFFF) {
      continue;
    }
    ///
    /// Check if EndPoint device is Multi-Function Device
    ///
    if (MmioRead8 (EndPointBase + PCI_HEADER_TYPE_OFFSET) & HEADER_TYPE_MULTI_FUNCTION) {
      ///
      /// If multi-function Device, check function 0-7
      ///
      EndPointFunction = PCI_MAX_FUNC;
    } else {
      ///
      /// Otherwise, check function 0 only
      ///
      EndPointFunction = 0;
    }

    for (FunctionIndex = 0; FunctionIndex <= EndPointFunction; FunctionIndex++) {
      ///
      /// Get the pointer to the Endpoint PCI Express Capability Structure.
      ///
      EndPointPcieCapOffset = PcieFindCapId (EndPointBus, DeviceIndex, FunctionIndex, EFI_PCI_CAPABILITY_ID_PCIEXP);

      if (EndPointPcieCapOffset == 0) {
        if (FunctionIndex < EndPointFunction) {
          continue;
        } else {
          return EFI_NOT_FOUND;
        }
      }
      ///
      /// Configure downstream device if present.
      ///
      EndPointBase = MmPciAddress (0, EndPointBus, DeviceIndex, FunctionIndex, 0);
      EndPointVendorId      = (UINT16) (MmioRead32 (EndPointBase + R_PCH_PCIE_ID) & B_PCH_PCIE_ID_VID);
      EndPointDeviceId      = (UINT16) ((MmioRead32 (EndPointBase + R_PCH_PCIE_ID) & B_PCH_PCIE_ID_DID) >> 16);
      EndPointRevId         = MmioRead8  (EndPointBase + R_PCH_PCIE_RID_CC);

      if (Operation == CalculateAspm || Operation == ManualAspm) {
        EndPointAspm = (MmioRead16 (EndPointBase + EndPointPcieCapOffset + 0x00C) >> 10) & 3;
        DEBUG ((EFI_D_INFO, "Endpoint Device %0x Capability ASPM: %0x\n", DeviceIndex, EndPointAspm));
        if (Operation == CalculateAspm) {
          ///
          /// Mask APMC with values from lookup table.
          /// RevID of 0xFF applies to all steppings.
          ///
          EndPointBaseClassCode = (UINT8) ((MmioRead32 (EndPointBase + R_PCH_PCIE_RID_CC) & B_PCH_PCIE_RID_CC_BCC) >> 24);
          EndPointSubClassCode  = (UINT8) ((MmioRead32 (EndPointBase + R_PCH_PCIE_RID_CC) & B_PCH_PCIE_RID_CC_SCC) >> 16);

          for (PcieDeviceIndex = 0; PcieDeviceIndex < NumOfDevAspmOverride; PcieDeviceIndex++) {
            if (((DevAspmOverride[PcieDeviceIndex].VendorId       == EndPointVendorId)      ||
                 (DevAspmOverride[PcieDeviceIndex].VendorId       == 0xFFFF)) &&
                ((DevAspmOverride[PcieDeviceIndex].DeviceId       == EndPointDeviceId)      ||
                 (DevAspmOverride[PcieDeviceIndex].DeviceId       == 0xFFFF)) &&
                ((DevAspmOverride[PcieDeviceIndex].RevId          == EndPointRevId)         ||
                 (DevAspmOverride[PcieDeviceIndex].RevId          == 0xFF))   &&
                ((DevAspmOverride[PcieDeviceIndex].BaseClassCode  == EndPointBaseClassCode) ||
                 (DevAspmOverride[PcieDeviceIndex].BaseClassCode  == 0xFF))   &&
                ((DevAspmOverride[PcieDeviceIndex].SubClassCode   == EndPointSubClassCode)  ||
                 (DevAspmOverride[PcieDeviceIndex].SubClassCode   == 0xFF))) {
              ///
              /// Override value of 0xFF applies to all.
              ///
              EndPointAspm = DevAspmOverride[PcieDeviceIndex].EndPointAspm;
              break;
            }
          }
          ///
          /// Check if L1 should be enabled based on port and endpoint L1 exit latency.
          ///
          if (EndPointAspm & BIT1) {
            PortLxLat     = MmioRead32 (RootDeviceBase + RootPcieCapOffset + 0x00C) & (BIT17 + BIT16 + BIT15);
            EndPointLxLat = MmioRead32 (EndPointBase + EndPointPcieCapOffset + 0x00C) & (BIT17 + BIT16 + BIT15);

            LxLat         = PortLxLat;
            if (PortLxLat < EndPointLxLat) {
              LxLat = EndPointLxLat;
            }
            ///
            /// check if the value is bigger than endpoint L1 acceptable exit latency, if it is
            /// larger than accepted value, then we should disable L1
            ///
            LxLat >>= 6;
            if (LxLat > (MmioRead32 (EndPointBase + EndPointPcieCapOffset + 0x004) & (BIT11 + BIT10 + BIT9))) {
              EndPointAspm &= ~BIT1;
            }
          }
          ///
          /// Check if L0s should be enabled based on port and endpoint L0s exit latency.
          ///
          if (EndPointAspm & BIT0) {
            PortLxLat     = MmioRead32 (RootDeviceBase + RootPcieCapOffset + 0x00C) & (BIT14 + BIT13 + BIT12);
            EndPointLxLat = MmioRead32 (EndPointBase + EndPointPcieCapOffset + 0x00C) & (BIT14 + BIT13 + BIT12);

            LxLat         = PortLxLat;
            if (PortLxLat < EndPointLxLat) {
              LxLat = EndPointLxLat;
            }
            ///
            /// check if the value is bigger than endpoint L0s acceptable exit latency, if it is
            /// larger than accepted value, then we should disable L0s
            ///
            LxLat >>= 6;
            if (LxLat > (MmioRead32 (EndPointBase + EndPointPcieCapOffset + 0x004) & (BIT8 + BIT7 + BIT6))) {
              EndPointAspm &= ~BIT0;
            }
          }
        }

        *LinkAspmVal &= EndPointAspm;
        DEBUG ((EFI_D_INFO, "Calculate Endpoint Device %0x Aspm Value: %0x\n", DeviceIndex, EndPointAspm));
      } else if (Operation == SetAspm) {
        ///
        /// Write it to the Link Control register
        ///
        DEBUG ((EFI_D_INFO, "Program Endpoint Device %0x Aspm Value: %0x\n", DeviceIndex, *LinkAspmVal));
        MmioAndThenOr16 (EndPointBase + EndPointPcieCapOffset + 0x10, 0xFFFC, *LinkAspmVal);
        ///
        /// For each device detected, scan the LTR override table and check if need to update
        /// B0:D28:F0-F7:400h and 404h
        /// The devices in the table can be directly attached to the root port or connected
        /// behind a bridge.
        /// LtrOverrideVal is passed into this function with default override values. At return, if device(s)
        /// is found in the table this will hold the updated latency values that will be programmed in the override
        /// register of this root port
        /// If this is the only endpoint connected to this root port, Data32 will have the latency values,
        /// specified in the table, corresponsding to this endpoint.
        /// If there are more than one endpoints attached to this root port (via a bridge) that are also
        /// found in the table, Data32 will maintain the lowest latency values.
        ///
        Data32 = *LtrOverrideVal;
        OverrideEntry = FALSE;
        if (DevLtrOverride != NULL) {
          for (PcieDeviceIndex = 0; PcieDeviceIndex < NumOfDevLtrOverride; PcieDeviceIndex++) {
            if ((DevLtrOverride[PcieDeviceIndex].VendorId  == EndPointVendorId)    &&
                ((DevLtrOverride[PcieDeviceIndex].DeviceId == EndPointDeviceId) ||
                 (DevLtrOverride[PcieDeviceIndex].DeviceId == 0xFFFF))             &&
                ((DevLtrOverride[PcieDeviceIndex].RevId     == EndPointRevId)             ||
                 (DevLtrOverride[PcieDeviceIndex].RevId     == 0xFF))) {
              OverrideEntry = TRUE;

              ///
              /// Get the Non-Snoop latency value from the table, compare and store the minimum
              ///
              if (DevLtrOverride[PcieDeviceIndex].NonSnoopLatency & BIT15) {
                Data16 = (UINT16)((Data32 & 0xFFFF0000) >> 16);
                UpdateLtrOverrideValue(&Data16, &DevLtrOverride[PcieDeviceIndex].NonSnoopLatency);
                Data32 = (Data32 & 0xFFFF) | ((UINT32)(Data16 << 16));
              }

              ///
              /// Get the Snoop latency value from the table, compare and store the minimum
              ///
              if (DevLtrOverride[PcieDeviceIndex].SnoopLatency & BIT15) {
                Data16 = (UINT16)(Data32 & 0xFFFF);
                UpdateLtrOverrideValue(&Data16, &DevLtrOverride[PcieDeviceIndex].SnoopLatency);
                Data32 = (Data32 & 0xFFFF0000) | (UINT32)Data16;
              }
              *LtrOverrideVal = Data32;
              break;
            }
          }
        }
        ///
        /// PCH BIOS Spec Rev 0.5.0, Section 8.14.1 Power Optimizer Configuration
        /// Step 3
        /// For PCIe Endpoint,
        /// If Endpoint device supported LTR, Device Capabilities 2 Register Offset 24h [11],
        ///
        DeviceCapabilities2 = MmioRead32 (EndPointBase + EndPointPcieCapOffset + 0x24);
        if (DeviceCapabilities2 & BIT11) {
          ///
          /// Enable the LTR capability of this device only if it is not found in the table
          ///
          ///
          if (OverrideEntry == FALSE) {
            ///
            /// Step 3.1
            /// Program Endpoint LTR Mechanism Enable, Device Control 2 Register Offset 28h [10] = 1b
            ///
            MmioOr16 (EndPointBase + EndPointPcieCapOffset + 0x28, BIT10);
          }
          ///
          /// Get the pointer to the Endpoint PCI Express Extended Capability Structure
          /// and configure the Max Snoop and Max No-Snoop Latency for the endpoint
          ///
          LtrExtendedCapOffset = PcieFindExtendedCapId (EndPointBus, DeviceIndex, FunctionIndex, 0x18);
          if (LtrExtendedCapOffset != 0) {
            ///
            /// Step 3.2
            /// Program Endpoint Max Snoop Latency Register, Latency Tolerance Reporting (LTR)
            /// Capability Offset 04h [15:0] = 0846h
            ///
            Data16 = 0x0846;
            MmioAndThenOr16 (
              EndPointBase + LtrExtendedCapOffset + 4,
              (UINT16) (~0x1FFF),
              Data16
              );
            ///
            /// Step 3.3
            /// Program Endpoint Max No-Snoop Latency Register, Latency Tolerance Reporting (LTR)
            /// Capability Offset 06h [15:0] = 0846h
            ///
            Data16 = 0x0846;
            MmioAndThenOr16 (
              EndPointBase + LtrExtendedCapOffset + 6,
              (UINT16) (~0x1FFF),
              Data16
              );
          }
        }
        ///
        /// Step 4
        /// For PCIe Endpoint,
        /// If Endpoint device supported OBFF, Device Capabilities 2 Register Offset 24h [19:18] = 2h,
        /// Root port not support OBFF
        //if (DeviceCapabilities2 & BIT19) {
        //  ///
        //  /// Step 4.1
        //  /// Program Endpoint OBFF Mechanism Enable, Device Control 2 Register Offset 28h [14:13] = 3h
        //  ///
        //  MmioOr16 (EndPointBase + EndPointPcieCapOffset + 0x28, (BIT14 + BIT13));
        //}
      }
      ///
      /// Check if this device is a bridge
      ///
      ClassCode = (UINT8) (MmioRead32 (EndPointBase + R_PCH_PCIE_RID_CC) >> 24);

      if (ClassCode == PCI_CLASS_BRIDGE) {
        ///
        /// Get the downstream Bus number
        ///
        DownStreamBusMin = (UINT8) (MmioRead32 (EndPointBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET) >> 8);
        ///
        /// If the Secondary Bus Number of endpoint device is not assigned
        ///
        if (DownStreamBusMin == 0) {
          RootDevSubBusNum = (UINT8) (MmioRead32 (RootDeviceBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET) >> 16);
          ///
          /// If the endpoint device is a bridge, the Subordinate Bus Number of the root port will need to be greater
          /// than the Secondary Bus Number of the root port (the Bus Number of endpoint device).
          ///
          if (RootDevSubBusNum > EndPointBus) {
            ///
            /// Assign the Primary, Secondary and Subordinate Bus Number to endpoint device
            ///
            MmioAndThenOr32 (
              EndPointBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
              0xFF000000,
              EndPointBus | (((UINT32) (EndPointBus + 1) << 8)) | ((UINT32) (RootDevSubBusNum << 16))
              );
            DownStreamBusMin = EndPointBus + 1;
          } else {
            return EFI_OUT_OF_RESOURCES;
          }

          BusAssign = FALSE;
        } else {
          BusAssign = TRUE;
        }

        if (DownStreamBusMin > EndPointBus) {
          Status = PcieEndPointPm (
                     RootDeviceBase,
                     RootPcieCapOffset,
                     DownStreamBusMin,
                     NumOfDevAspmOverride,
                     DevAspmOverride,
                     LinkAspmVal,
                     Operation,
                     NumOfDevLtrOverride,
                     DevLtrOverride,
                     LtrOverrideVal
                     );
          if (Status == EFI_NOT_FOUND) {
            DEBUG ((EFI_D_INFO, "Check DownStreamBus:%d and no device found!\n", DownStreamBusMin));
          }

          if (BusAssign == FALSE) {
            ///
            /// Clear Bus Numbers.
            ///
            MmioAnd32 (EndPointBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET, 0xFF000000);
          }
        }
      }
    }
  }

  return EFI_SUCCESS;
}

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
  IN  UINT8                         NumOfDevLtrOverride,
  IN  PCH_PCIE_DEVICE_LTR_OVERRIDE  *DevLtrOverride
  )
/**

  @brief
  This function performs the Power Management settings for root port and downstream device

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
{
  ///
  /// PCH BIOS Spec Rev 0.5.0, Section 8.3.1 ASPM on DMI and the PCI Express* Root Ports
  ///
  /// When enabling L0s / L1 support, BIOS should enable upstream device before downstream
  /// device. When disabling ASPM, BIOS should make sure downstream device is disabled
  /// before upstream device.
  /// The System BIOS must perform the following steps to enable
  /// L0s/L1 on the root ports:
  ///
  /// 1. Determine whether the endpoint supports L1 by checking the Active State Link PM
  ///    Support field of the endpoint Link Capability Register.  If the endpoint does not
  ///    support L1, the System BIOS can skip the L1 calculations below.  Likewise, System
  ///    BIOS should not enable L1 on the root port or the endpoint if the endpoint does not
  ///    support L1.
  /// 2. Calculate the total L0s and L1 exit latency. A description of this calculation
  ///    is provided in Section 8.3.1.1.
  /// 3. Compare the calculated total exit latency with Endpoint L0s/L1 Acceptable Latency
  ///    read from the Device Capabilities Register of the Endpoint to determine if L0s or
  ///    L1 can be enabled for all or some of the links on the entire path to satisfy the
  ///    Acceptable Latency reported by the Endpoint.  The Exit Latency fields reported by
  ///    the registers are given as a range.  It is recommended that System BIOS uses the
  ///    high end of the range for the latency calculation and comparison.  For example, if
  ///    the latency field reports "2 us to less than 4 us", then 4 us should be used for
  ///    the calculation.
  /// 4. If the comparison in step 3 indicates L0s and L1 can be enabled on a root port and
  ///    the endpoints attached to the root port, then set the root port register
  ///    D28:F0~F7:Reg E8h[1], then set the APMC field, D28:F0~F7:Reg 50h[1:0] to 11b and write
  ///    the same value to the APMC field of the endpoint Link Control register.  If the
  ///    comparison in step 1 indicates only L0s can be enabled on a root port and the
  ///    endpoints attached to the root port, then set the APMC field, D28:F0~F7:Reg 50h[1:0]
  ///    to 01b and write the same value to the APMC field of the endpoint Link Control
  ///    register.
  ///
  /// NOTE: current implementation does not support full length exit latency calculation
  ///
  UINT16      AspmVal;
  EFI_STATUS  Status;
  UINTN       RootDeviceBase;
  UINT32      RootPcieCapOffset;
  UINT8       EndPointBus;
  OPERATION   Operation;
  UINT16      SlotStatus;
  BOOLEAN     BusAssign;
  UINT32      DeviceCapabilities2;
  UINT32      LtrOvrVal;
  UINT32      Data32Or;

  Status          = EFI_SUCCESS;
  RootDeviceBase  = MmPciAddress (0, RootBus, RootDevice, RootFunction, 0);
  ///
  /// Get the pointer to the Port PCI Express Capability Structure.
  ///
  RootPcieCapOffset = PcieFindCapId (RootBus, RootDevice, RootFunction, EFI_PCI_CAPABILITY_ID_PCIEXP);
  if (RootPcieCapOffset == 0) {
    return EFI_UNSUPPORTED;
  }
  DeviceCapabilities2 = MmioRead32 (RootDeviceBase + RootPcieCapOffset + 0x24);

  ///
  /// Enable LTR mechanism for this root port if it is capable
  ///
  if (DeviceCapabilities2 & BIT11) {
    MmioOr16 (RootDeviceBase + RootPcieCapOffset + 0x28, BIT10);
  }

  ///
  /// Enable OBFF using WAKE# signaling for this root port if it is capable
  /// Root port not support OBFF
  //if (DeviceCapabilities2 & BIT19) {
  //  MmioOr16 (RootDeviceBase + RootPcieCapOffset + 0x28, (BIT14 + BIT13));
  //}
  AspmVal = (MmioRead16 (RootDeviceBase + RootPcieCapOffset + 0x00C) >> 10) & 3;
  if (RootPortAspm == PchPcieAspmAutoConfig) {
    Operation = CalculateAspm;
  } else {
    Operation = ManualAspm;
    AspmVal &= RootPortAspm;
  }
  ///
  /// Get the downstream Bus number
  ///
  EndPointBus = (UINT8) (MmioRead32 (RootDeviceBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET) >> 8);
  ///
  /// If the Secondary Bus Number of the root port is not assigned
  /// Note:
  /// It will be better that PCI emulation has been done before PcieSetPm(). Or, you will need to assign
  /// a larger number to TempRootPortBusNumMax to support the specific card which has many bridges behind.
  /// If it is not, the platform policy settings of "TempRootPortBusNumMax" and "TempRootPortBusNumMin"
  /// will be assigned to the Subordinate and Secondary Bus Number of the root ports.
  /// The assigned bus number will be cleared in the end of PcieSetPm().
  ///
  if (EndPointBus == 0) {
    MmioAndThenOr32 (
      RootDeviceBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
      0xFF0000FF,
      ((UINT32) (TempBusNumberMin << 8)) | ((UINT32) (TempBusNumberMax << 16))
      );
    EndPointBus = TempBusNumberMin;
    BusAssign   = FALSE;
  } else {
    BusAssign   = TRUE;
  }
  ///
  /// Check whether the slot has a device connected
  ///
  SlotStatus = MmioRead16 (RootDeviceBase + RootPcieCapOffset + 0x1A);
  LtrOvrVal = 0;

  ///
  /// Obtain initial ASPM settings from respective port capability registers.
  ///
  if (EndPointBus != 0 && (SlotStatus & BIT6) != 0) {
    Status = PcieEndPointPm (
               RootDeviceBase,
               RootPcieCapOffset,
               EndPointBus,
               NumOfDevAspmOverride,
               DevAspmOverride,
               &AspmVal,
               Operation,
               NumOfDevLtrOverride,
               DevLtrOverride,
               &LtrOvrVal
               );
  }
  ///
  /// Set Root Port Aspm and enable LTR capability of the device
  ///
  MmioAndThenOr16 (RootDeviceBase + RootPcieCapOffset + 0x010, 0xFFFC, AspmVal);
  if (EndPointBus != 0 && (SlotStatus & BIT6) != 0) {
    ///
    /// Set Endpoint Aspm
    ///
    Status = PcieEndPointPm (
               RootDeviceBase,
               RootPcieCapOffset,
               EndPointBus,
               NumOfDevAspmOverride,
               DevAspmOverride,
               &AspmVal,
               SetAspm,
               NumOfDevLtrOverride,
               DevLtrOverride,
               &LtrOvrVal
               );

    Data32Or   = 0;

    ///
    /// Program R_PCH_PCIE_LTROVR with LtrOvrVal
    ///
    MmioWrite32 (RootDeviceBase + R_PCH_PCIE_LTROVR, LtrOvrVal);

    ///
    /// Enable the override bits
    ///
    Data32Or = (B_PCH_PCIE_LTROVR2_LTRNSOVREN | B_PCH_PCIE_LTROVR2_LTRSOVREN);
    MmioWrite32 (RootDeviceBase + R_PCH_PCIE_LTROVR2, Data32Or);
  }

  if (BusAssign == FALSE) {
    ///
    /// Clear Bus Numbers.
    ///
    MmioAnd32 (RootDeviceBase + R_PCH_PCIE_BNUM_SLT, 0xFF0000FF);
  }

  if (!EFI_ERROR (Status)) {
    ///
    /// If L0s and L1 can be enabled on a root port and the endpoints attached to the root port,
    /// then set the root port register D28:Fx:Reg E8h[1]
    ///
    if (AspmVal == V_PCH_PCIE_LCTL_LSTS_ASPM_L0S_L1) {
      MmioOr8 (RootDeviceBase + R_PCH_PCIE_PWRCTL, B_PCH_PCIE_PWRCTL_RPL1SQPOL);
    }
  }

  return Status;
}

EFI_STATUS
PchPcieInitDownstreamDevices (
  IN      UINT8           RootPortBus,
  IN      UINT8           RootPortDevice,
  IN      UINT8           RootPortFunc,
  IN      UINT8           TempBusNumberMin,
  IN      UINT8           TempBusNumberMax,
  IN OUT  UINT16          *MaxPayload
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
  @param[in] MaxPayload           The Max Payolad Size of the root port

  @retval EFI_SUCCESS             Successfully completed
  @retval EFI_NOT_FOUND           Can not find device.

**/
{
  EFI_STATUS  Status;
  UINT32      Index;
  UINTN       RPBase;
  UINTN       EndPointBase;

  RPBase = MmPciAddress (0, RootPortBus, RootPortDevice, RootPortFunc, 0);
  ///
  /// Temporarily Hardcode the Root Port Bridge Number to TempBusNumberMin
  ///
  MmioAndThenOr32 (
    RPBase + PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET,
    0xFF000000,
    RootPortBus | ((UINT32) (TempBusNumberMin << 8)) | ((UINT32) (TempBusNumberMax << 16))
  );
  ///
  /// This Endpoint check should immediately pass.  Howerver, a 1.0s delay
  /// has been added to match the timing requirements of the PCI Express Base
  /// Specification, Revision 1.0A, Section 6.6 ("...software must allow 1.0s
  /// after a reset of a device, before it may determine that a device which
  /// fails to return a Successful Completion status for a valid Configuration
  /// Request is a broken device").
  ///
  EndPointBase = MmPciAddress (0, TempBusNumberMin, 0, 0, 0);
  ///
  /// A config write is required in order for the device to re-capture the Bus number,
  /// according to PCI Express Base Specification, 2.2.6.2 ("Note that the Bus Number
  /// and Device Number may be changed at run time, and so it is necessary to re-capture
  /// this information with each and every Configuration Write Request")
  ///
  MmioWrite8 (EndPointBase + 0x0, 0);
  for (Index = 0; Index < 100000; Index++) {
    if (MmioRead16 (EndPointBase + PCI_VENDOR_ID_OFFSET) != 0xFFFF) {
      break;
    }

    PchPmTimerStall (10);
  }

  if (Index >= 100000) {
    ///
    /// Clear Bus Numbers.
    ///
    MmioAnd32 (RPBase + R_PCH_PCIE_BNUM_SLT, (UINT32)~(B_PCH_PCIE_BNUM_SLT_SBBN | B_PCH_PCIE_BNUM_SLT_SCBN));
    return EFI_NOT_FOUND;
  }
  ///
  /// Get the Max Payload Size on all the end point functions
  ///
  PcieMaxPayloadSize (TempBusNumberMin, PCI_MAX_DEVICE, MaxPayload, FALSE);
  ///
  /// Check if this device is a bridge
  ///
  if ((UINT8) (MmioRead32 (EndPointBase + R_PCH_PCIE_RID_CC) >> 24) == PCI_CLASS_BRIDGE) {
    ///
    /// Initialize downstream devices
    ///
    if (TempBusNumberMax > TempBusNumberMin) {
      Status = PchPcieInitDownstreamDevices (
                 TempBusNumberMin,
                 0,
                 0,
                 TempBusNumberMin + 1,
                 TempBusNumberMax,
                 MaxPayload
                 );
    }
  }
  ///
  /// Complete Common Port and Endpoint Configuration.
  ///
  ///
  /// Map TC0-VC0
  ///
  PcieMapTcxVc0 (RootPortBus, RootPortDevice, (UINT8) RootPortFunc, TempBusNumberMin, PCI_MAX_DEVICE, 0x0);

  ///
  /// Set Common Clock for inserted cards
  ///
  ///
  /// PCH BIOS Spec Rev 0.5.0, Section 8.3.1 ASPM on DMI and the PCI Express* Root Ports
  /// Before determining whether ASPM can be enabled or not,
  /// the System BIOS must perform the following steps:
  ///
  /// 3. For each root port, read the Slot Clock Configuration bit, D28:F0~F7:Reg 52h[12],
  /// of the root port and the endpoint device connected to the port (i.e., D0:F0 on the
  /// secondary bus behind the root port). If both components have this bit set, then the
  /// System BIOS should set the Common Clock Configuration (CCC) bit, D28:F0~F7:Reg 50h[6],
  /// for both components at both sides of the link to indicate that components at both ends
  /// of the link use a common clock source.
  ///
  /// 4. If the CCC bit was changed by the System BIOS in step 3, System BIOS should initiate
  /// a link training by setting the Retrain Link (RL) bit, D28:F0~F7:Reg 50h[5], and then poll the Link
  /// Training (LT) bit, D28:F0~F7:Reg 52h[11], until it is clear.
  /// Note that System BIOS should save and restore CCC bit on S3.
  ///
  PcieSetCommonClock (RootPortBus, RootPortDevice, (UINT8) RootPortFunc, TempBusNumberMin, PCI_MAX_DEVICE);

  ///
  /// Enable the PCIe CLKREQ#
  ///
  PcieSetClkreq (TempBusNumberMin, PCI_MAX_DEVICE);

  ///
  /// Set the Max Payload Size on all the end point functions
  ///
  PcieMaxPayloadSize (TempBusNumberMin, PCI_MAX_DEVICE, MaxPayload, TRUE);

  ///
  /// Disable the forwarding of EOI messages unless it discovers an IOAPIC behind this root port
  ///
  PcieSetEoiFwdDisable (RootPortBus, RootPortDevice, RootPortFunc, TempBusNumberMin, PCI_MAX_DEVICE);
  ///
  /// Clear Bus Numbers
  ///
  MmioAnd32 (RPBase + R_PCH_PCIE_BNUM_SLT, (UINT32)~(B_PCH_PCIE_BNUM_SLT_SBBN | B_PCH_PCIE_BNUM_SLT_SCBN));

  return EFI_SUCCESS;
}

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
{
  UINT32      SlotStatus;
  UINTN       RPBase;
  UINT16      RootPortMaxPayload;
  UINT8       PcieCapOffset;
  EFI_STATUS  Status;

  RPBase = MmPciAddress (0, RootPortBus, RootPortDevice, RootPortFunc, 0);
  ///
  /// Check for a Presence Detect Change.
  ///
  SlotStatus = MmioRead32 (RPBase + R_PCH_PCIE_SLCTL_SLSTS);

  ///
  /// Check whether the slot has a device connected
  ///
  if ((SlotStatus & B_PCH_PCIE_SLCTL_SLSTS_PDS) == 0) {
    return EFI_NOT_FOUND;
  }
  ///
  /// Get the pointer to the Endpoint PCI Express Capability Structure.
  ///
  PcieCapOffset = PcieFindCapId (
                    RootPortBus,
                    RootPortDevice,
                    RootPortFunc,
                    EFI_PCI_CAPABILITY_ID_PCIEXP
                    );

  ///
  /// Get the root port Max Payload Size support
  ///
  RootPortMaxPayload = MmioRead16 (RPBase + PcieCapOffset + 0x04) & (BIT2 | BIT1 | BIT0);

  ///
  /// Initialize downstream devices
  ///
  Status = PchPcieInitDownstreamDevices (
             RootPortBus,
             RootPortDevice,
             RootPortFunc,
             TempBusNumberMin,
             TempBusNumberMax,
             &RootPortMaxPayload
             );

  ///
  /// Set the PCIE root Port Max Payload Size
  ///
  MmioAndThenOr16 (RPBase + PcieCapOffset + 0x08, (UINT16)~(BIT7 | BIT6 | BIT5), RootPortMaxPayload << 5);

  return Status;
}
