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
  LegacyInterrupt.h

  @brief
  This code supports a the private implementation
  of the Legacy Interrupt protocol

**/
#ifndef LEGACY_INTERRUPT_H_
#define LEGACY_INTERRUPT_H_

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include <Protocol/LegacyInterrupt/LegacyInterrupt.h>
#else
#include <PiDxe.h>

#include <Protocol/LegacyInterrupt.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#endif
#include "PchAccess.h"

#define PIRQN           0x00  /// PIRQ Null
#define PIRQA           R_PCH_ILB_PIRQA_ROUT
#define PIRQB           R_PCH_ILB_PIRQB_ROUT
#define PIRQC           R_PCH_ILB_PIRQC_ROUT
#define PIRQD           R_PCH_ILB_PIRQD_ROUT
#define PIRQE           R_PCH_ILB_PIRQE_ROUT
#define PIRQF           R_PCH_ILB_PIRQF_ROUT
#define PIRQG           R_PCH_ILB_PIRQG_ROUT
#define PIRQH           R_PCH_ILB_PIRQH_ROUT

#define MAX_PIRQ_NUMBER 8

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
;

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
;

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
;

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
;

#endif
