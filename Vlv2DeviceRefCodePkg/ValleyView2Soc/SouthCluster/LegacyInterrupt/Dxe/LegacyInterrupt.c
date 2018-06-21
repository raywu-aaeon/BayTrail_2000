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
  LegacyInterrupt.c

  @brief
  This code supports a the private implementation
  of the Legacy Interrupt protocol

**/
#include "LegacyInterrupt.h"

///
/// Handle for the Legacy Interrupt Protocol instance produced by this driver
///
EFI_HANDLE                    mLegacyInterruptHandle = NULL;

///
/// The Legacy Interrupt Protocol instance produced by this driver
///
EFI_LEGACY_INTERRUPT_PROTOCOL mLegacyInterrupt = {
  GetNumberPirqs,
  GetLocation,
  ReadPirq,
  WritePirq
};

///
/// Module Global:
///  Since this driver will only ever produce one instance of the Private Data
///  protocol you are not required to dynamically allocate the PrivateData.
///
UINT8                         PirqReg[MAX_PIRQ_NUMBER] = { PIRQA, PIRQB, PIRQC, PIRQD, PIRQE, PIRQF, PIRQG, PIRQH };

///
///
///
EFI_STATUS
GetNumberPirqs (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  OUT UINT8                          *NumberPirqs
  )
/**

  @brief
  Return the number of PIRQs supported by this chipset.

  @param[in] This                 Pointer to LegacyInterrupt Protocol
  @param[in] NumberPirqs          The pointer which point to the max IRQ number supported by this PCH.

  @retval EFI_SUCCESS             Legacy BIOS protocol installed

**/
{
  *NumberPirqs = MAX_PIRQ_NUMBER;

  return EFI_SUCCESS;
}

EFI_STATUS
GetLocation (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  OUT UINT8                          *Bus,
  OUT UINT8                          *Device,
  OUT UINT8                          *Function
  )
/**

  @brief
  Return PCI location of this device. $PIR table requires this info.

  @param[in] This                 Protocol instance pointer.
  @param[in] Bus                  PCI Bus
  @param[in] Device               PCI Device
  @param[in] Function             PCI Function

  @retval EFI_SUCCESS             Bus/Device/Function returned

**/
{
  *Bus      = DEFAULT_PCI_BUS_NUMBER_PCH;
  *Device   = PCI_DEVICE_NUMBER_PCH_LPC;
  *Function = PCI_FUNCTION_NUMBER_PCH_LPC;

  return EFI_SUCCESS;
}

UINTN
GetAddress (
  UINT8  PirqNumber
  )
/**

  @brief
  Builds the PCIE configuration address for the register specified by PirqNumber

  @param[in] PirqNumber           The PIRQ number to build the PCIE configuration address for

  @retval UINTN                   The PCIE Configuration address for interrupt controller in PCH

**/
{
  return (PchLpcPciCfg32 (R_PCH_LPC_ILB_BASE) & B_PCH_LPC_ILB_BASE_BAR) + PirqReg[PirqNumber];
}

EFI_STATUS
ReadPirq (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  IN  UINT8                          PirqNumber,
  OUT UINT8                          *PirqData
  )
/**

  @brief
  Read the given PIRQ register

  @param[in] This                 Pointer to LegacyInterrupt Protocol
  @param[in] PirqNumber           The Pirq register 0 = A, 1 = B etc
  @param[in] PirqData             Value read

  @retval EFI_SUCCESS             Decoding change affected.
  @retval EFI_INVALID_PARAMETER   Invalid PIRQ number

**/
{
  if (PirqNumber >= MAX_PIRQ_NUMBER) {
    return EFI_INVALID_PARAMETER;
  }

  *PirqData = MmioRead8 (GetAddress (PirqNumber));
  *PirqData = (UINT8) (*PirqData & 0x7f);

  return EFI_SUCCESS;
}

EFI_STATUS
WritePirq (
  IN  EFI_LEGACY_INTERRUPT_PROTOCOL  *This,
  IN  UINT8                          PirqNumber,
  IN  UINT8                          PirqData
  )
/**

  @brief
  Read the given PIRQ register

  @param[in] This                 Pointer to LegacyInterrupt Protocol
  @param[in] PirqNumber           The Pirq register 0 = A, 1 = B etc
  @param[in] PirqData             Value read

  @retval EFI_SUCCESS             Decoding change affected.
  @retval EFI_INVALID_PARAMETER   Invalid PIRQ number

**/
{
  if (PirqNumber >= MAX_PIRQ_NUMBER) {
    return EFI_INVALID_PARAMETER;
  }

  MmioWrite8 (GetAddress (PirqNumber), PirqData);
  return EFI_SUCCESS;
}

EFI_STATUS
LegacyInterruptInstall (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/**

  @brief
  Install Driver to produce Legacy Interrupt protocol.

  @param[in] ImageHandle          Handle for this drivers loaded image protocol.
  @param[in] SystemTable          EFI system table.

  @retval EFI_SUCCESS             Legacy Interrupt protocol installed
  @retval Other                   No protocol installed, unload driver.

**/
{
  EFI_STATUS  Status;

  DEBUG ((EFI_D_INFO, "LegacyInterruptInstall() Start\n"));

  ///
  /// Make sure the Legacy Interrupt Protocol is not already installed in the system
  ///
  ASSERT_PROTOCOL_ALREADY_INSTALLED (NULL, &gEfiLegacyInterruptProtocolGuid);

  ///
  /// Make a new handle and install the protocol
  ///
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mLegacyInterruptHandle,
                  &gEfiLegacyInterruptProtocolGuid,
                  &mLegacyInterrupt,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  DEBUG ((EFI_D_INFO, "LegacyInterruptInstall() End\n"));
  return Status;
}
