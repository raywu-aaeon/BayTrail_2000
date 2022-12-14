//
// This file contains 'Framework Code' and is licensed as such
// under the terms of your license agreement with Intel or your
// vendor.  This file may not be modified, except as allowed by
// additional terms of your license agreement.
//

/*++

Copyright (c)  1999 - 2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  McAccess.h

Abstract:

  Macros to simplify and abstract the interface to PCI configuration.

--*/

#ifndef _MCACCESS_H_INCLUDED_
#define _MCACCESS_H_INCLUDED_

#include "Valleyview.h"
#include "VlvCommonDefinitions.h"

//
// Memory Controller PCI access macros
//

#define MCH_BASE_ADDRESS    0xfed14000

// Device 0, Function 0

#define McD0PciCfg32(Register)              MmPci32 (0, MC_BUS, 0, 0, Register)

#define McD0PciCfg32Or(Register, OrData)    MmPci32Or (0, MC_BUS, 0, 0, Register, OrData)

#define McD0PciCfg32And(Register, AndData)  MmPci32And (0, MC_BUS, 0, 0, Register, AndData)

#define McD0PciCfg32AndThenOr(Register, AndData, OrData) \
  MmPci32AndThenOr (0, MC_BUS, 0, 0, Register, AndData, OrData)


#define McD0PciCfg16(Register)              MmPci16 (0, MC_BUS, 0, 0, Register)

#define McD0PciCfg16Or(Register, OrData)    MmPci16Or (0, MC_BUS, 0, 0, Register, OrData)

#define McD0PciCfg16And(Register, AndData)  MmPci16And (0, MC_BUS, 0, 0, Register, AndData)

#define McD0PciCfg16AndThenOr(Register, AndData, OrData) \
  MmPci16AndThenOr (0, MC_BUS, 0, 0, Register, AndData, OrData)


#define McD0PciCfg8(Register)               MmPci8 (0, MC_BUS, 0, 0, Register)

#define McD0PciCfg8Or(Register, OrData)     MmPci8Or (0, MC_BUS, 0, 0, Register, OrData)

#define McD0PciCfg8And(Register, AndData)   MmPci8And (0, MC_BUS, 0, 0, Register, AndData)

#define McD0PciCfg8AndThenOr( Register, AndData, OrData ) \
  MmPci8AndThenOr (0, MC_BUS, 0, 0, Register, AndData, OrData)


// Device 1, Function 0

#define McD1PciCfg32(Register)              MmPci32 (0, MC_BUS, 1, 0, Register)

#define McD1PciCfg32Or(Register, OrData)    MmPci32Or (0, MC_BUS, 1, 0, Register, OrData)

#define McD1PciCfg32And(Register, AndData)  MmPci32And (0, MC_BUS, 1, 0, Register, AndData)

#define McD1PciCfg32AndThenOr(Register, AndData, OrData) \
  MmPci32AndThenOr (0, MC_BUS, 1, 0, Register, AndData, OrData)


#define McD1PciCfg16(Register)              MmPci16 (0, MC_BUS, 1, 0, Register)

#define McD1PciCfg16Or(Register, OrData)    MmPci16Or (0, MC_BUS, 1, 0, Register, OrData)

#define McD1PciCfg16And(Register, AndData)  MmPci16And (0, MC_BUS, 1, 0, Register, AndData)

#define McD1PciCfg16AndThenOr(Register, AndData, OrData) \
  MmPci16AndThenOr (0, MC_BUS, 1, 0, Register, AndData, OrData)


#define McD1PciCfg8(Register)               MmPci8 (0, MC_BUS, 1, 0, Register)

#define McD1PciCfg8Or(Register, OrData)     MmPci8Or (0, MC_BUS, 1, 0, Register, OrData)

#define McD1PciCfg8And(Register, AndData)   MmPci8And (0, MC_BUS, 1, 0, Register, AndData)

#define McD1PciCfg8AndThenOr(Register, AndData, OrData) \
  MmPci8AndThenOr (0, MC_BUS, 1, 0, Register, AndData, OrData)


// Device 2, Function 0

#define McD2PciCfg32(Register)              MmPci32 (0, MC_BUS, 2, 0, Register)

#define McD2PciCfg32Or(Register, OrData)    MmPci32Or (0, MC_BUS, 2, 0, Register, OrData)

#define McD2PciCfg32And(Register, AndData)  MmPci32And (0, MC_BUS, 2, 0, Register, AndData)

#define McD2PciCfg32AndThenOr(Register, AndData, OrData) \
  MmPci32AndThenOr (0, MC_BUS, 2, 0, Register, AndData, OrData)


#define McD2PciCfg16(Register)              MmPci16 (0, MC_BUS, 2, 0, Register)

#define McD2PciCfg16Or(Register, OrData)    MmPci16Or (0, MC_BUS, 2, 0, Register, OrData)

#define McD2PciCfg16And(Register, AndData)  MmPci16And (0, MC_BUS, 2, 0, Register, AndData)

#define McD2PciCfg16AndThenOr(Register, AndData, OrData) \
  MmPci16AndThenOr (0, MC_BUS, 2, 0, Register, AndData, OrData)


#define McD2PciCfg8(Register)               MmPci8 (0, MC_BUS, 2, 0, Register)

#define McD2PciCfg8Or(Register, OrData)     MmPci8Or (0, MC_BUS, 2, 0, Register, OrData)

#define McD2PciCfg8And(Register, AndData)   MmPci8And (0, MC_BUS, 2, 0, Register, AndData)

#define McD2PciCfg8AndThenOr(Register, AndData, OrData) \
  MmPci8AndThenOr (0, MC_BUS, 2, 0, Register, AndData, OrData)


//
// Memory Controller Hub Memory Mapped IO register accesse
//

#define McMmioAddress(Register)             ((UINTN)MCH_BASE_ADDRESS + (UINTN)(Register))


#define McMmio32Ptr(Register)               ((volatile UINT32 *)McMmioAddress (Register))

#define McMmio32(Register)                  *McMmio32Ptr( Register )

#define McMmio32Or(Register, OrData)        (McMmio32 (Register) |= (UINT32)(OrData))

#define McMmio32And(Register, AndData)      (McMmio32 (Register) &= (UINT32)(AndData))

#define McMmio32AndThenOr(Register, AndData, OrData) \
  (McMmio32 ( Register ) = (McMmio32( Register ) & (UINT32)(AndData)) | (UINT32)(OrData))


#define McMmio16Ptr(Register)               ((volatile UINT16 *)McMmioAddress (Register))

#define McMmio16(Register)                  *McMmio16Ptr (Register)

#define McMmio16Or(Register, OrData)        (McMmio16 (Register) |= (UINT16)(OrData))

#define McMmio16And( Register, AndData )    (McMmio16 (Register) &= (UINT16)(AndData))

#define McMmio16AndThenOr(Register, AndData, OrData) \
  (McMmio16 (Register) = (McMmio16 (Register) & (UINT16)(AndData)) | (UINT16)(OrData))


#define McMmio8Ptr(Register)                ((volatile UINT8 *)McMmioAddress (Register))

#define McMmio8(Register)                   *McMmio8Ptr (Register)

#define McMmio8Or(Register, OrData)         (McMmio8 (Register) |= (UINT8)(OrData))

#define McMmio8And(Register, AndData)       (McMmio8 (Register) &= (UINT8)(AndData))

#define McMmio8AndThenOr(Register, AndData, OrData) \
  (McMmio8 (Register) = (McMmio8 (Register) & (UINT8)(AndData)) | (UINT8)(OrData))

#endif

