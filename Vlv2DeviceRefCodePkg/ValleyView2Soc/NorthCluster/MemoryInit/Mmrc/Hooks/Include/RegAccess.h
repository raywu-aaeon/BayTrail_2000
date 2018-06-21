/*++

Copyright (c) 2005-2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  RegAccess.h

Abstract:

  This file include register access.

--*/

#ifndef _REGACCESS_H_
#define _REGACCESS_H_

#include "../../IpBlocks/VLVA0/Include/MMRC.h"

//
// Add more units here if they need to be read/written by MRC.
//
typedef enum {
  DUNIT,
  CPGC,
  DDRIO,
  BUNIT,
  TUNIT,
  PUNIT,
  AUNIT,
  CUNIT,
  RTF,
  DRNG,
  CCK
} CpuBoxType;

typedef enum {
  NO_ERROR,
  INVALID_INSTANCE,
  INVALID_BOX_TYPE,
  INVALID_MODE,
  BOX_TYPE_NOT_IN_SIMULATION,
  UNKNOWN_ERROR
} ACCESS_ERROR;

typedef enum {
  MODE_READ,
  MODE_WRITE
} ACCESS_MODE;

#ifndef _VLVACCESS_H_INCLUDED_
//
// Memory Mapped IO
//
UINT32
Mmio32Read (
  IN    UINT32  RegisterAddress
)
;

VOID
Mmio32Write (
  IN    UINT32  RegisterAddress,
  IN    UINT32  Value
)
;

UINT16
Mmio16Read (
  IN    UINT32  RegisterAddress
)
;

VOID
Mmio16Write (
  IN    UINT32  RegisterAddress,
  IN    UINT16  Value
)
;

UINT8
Mmio8Read (
  IN    UINT32  RegisterAddress
)
;

VOID
Mmio8Write (
  IN    UINT32  RegisterAddress,
  IN    UINT8   Value
)
;

#ifndef Mmio32Or
#define Mmio32Or(Register, OrData) Mmio32Write(Register, Mmio32Read(Register)|OrData)
#define Mmio16Or(Register, OrData) Mmio16Write(Register, Mmio16Read(Register)|OrData)
#define Mmio8Or(Register, OrData) Mmio8Write(Register, Mmio8Read(Register)|OrData)

#define Mmio32And(Register, AndData) Mmio32Write(Register, Mmio32Read(Register)& (AndData))
#define Mmio16And(Register, AndData) Mmio16Write(Register, Mmio16Read(Register)& (AndData))
#define Mmio8And(Register, AndData) Mmio8Write(Register, Mmio8Read(Register)& (AndData))

#define Mmio32AndThenOr(Register, AndData, OrData) Mmio32Write(Register, (((Mmio32Read(Register)& (AndData)))|OrData))
#define Mmio16AndThenOr(Register, AndData, OrData) Mmio16Write(Register, (((Mmio16Read(Register)& (AndData)))|OrData))
#define Mmio8AndThenOr(Register, AndData, OrData) Mmio8Write(Register, (((Mmio8Read(Register)& (AndData)))|OrData))
#endif

//
// Memory mapped PCI IO
//
#define PCI_CFG_PTR(Bus, Device, Function, Register )\
    ((UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register))

#define PCI_CFG_32B_READ_CF8CFC(B,D,F,R)\
  (UINT32)(IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoIn32(0xCFC))

#define PCI_CFG_32B_WRITE_CF8CFC(B,D,F,R,Data) \
  (IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoOut32(0xCFC,Data))

#define PCI_CFG_32B_AND_THEN_OR_CF8CFC(B,D,F,R,A,O) \
  PCI_CFG_32B_WRITE_CF8CFC (B,D,F,R, \
    (PCI_CFG_32B_READ_CF8CFC (B,D,F,R) & (A)) | (O))

#define PCI_CFG_16B_READ_CF8CFC(B,D,F,R) \
  (UINT16)(IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoIn16(0xCFC))

#define PCI_CFG_16B_WRITE_CF8CFC(B,D,F,R,Data) \
  (IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoOut16(0xCFC,Data))

#define PCI_CFG_16B_AND_THEN_OR_CF8CFC(B,D,F,R,A,O) \
  PCI_CFG_16B_WRITE_CF8CFC (B,D,F,R, \
    (PCI_CFG_16B_READ_CF8CFC (B,D,F,R) & (A)) | (O))

#define PCI_CFG_8B_READ_CF8CFC(B,D,F,R) \
  (UINT8)(IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoIn8(0xCFC))

#define PCI_CFG_8B_WRITE_CF8CFC(B,D,F,R,Data) \
  (IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoOut8(0xCFC,Data))

#define PCI_CFG_8B_AND_THEN_OR_CF8CFC(B,D,F,R,A,O) \
  PCI_CFG_8B_WRITE_CF8CFC (B,D,F,R, \
    (PCI_CFG_8B_READ_CF8CFC (B,D,F,R) & (A)) | (O))


#define PCI_CFG_32B_READ(PciExpressBase, Bus, Device, Function, Register) \
  Mmio32Read(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register))

#define PCI_CFG_32B_WRITE(PciExpressBase, Bus, Device, Function, Register, Value) \
  Mmio32Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    Value)

#define PCI_CFG_16B_READ(PciExpressBase, Bus, Device, Function, Register) \
  Mmio16Read(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register))

#define PCI_CFG_16B_WRITE(PciExpressBase, Bus, Device, Function, Register, Value) \
  Mmio16Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    Value)

#define PCI_CFG_8B_READ(PciExpressBase, Bus, Device, Function, Register) \
  Mmio8Read(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register))

#define PCI_CFG_8B_WRITE(PciExpressBase, Bus, Device, Function, Register, Value) \
  Mmio8Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    Value)

#define PCI_CFG_32B_OR(PciExpressBase, Bus, Device, Function, Register, OrValue) \
  Mmio32Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    (PCI_CFG_32B_READ (PciExpressBase, Bus, Device, Function, Register)|OrValue))

#define PCI_CFG_32B_AND(PciExpressBase, Bus, Device, Function, Register, AndValue) \
  Mmio32Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    (PCI_CFG_32B_READ (PciExpressBase, Bus, Device, Function, Register)& (AndValue)))

#define PCI_CFG_16B_OR(PciExpressBase, Bus, Device, Function, Register, OrValue) \
  Mmio16Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    (PCI_CFG_16B_READ(PciExpressBase, Bus, Device, Function, Register)|OrValue))

#define PCI_CFG_16B_AND(PciExpressBase, Bus, Device, Function, Register, AndValue) \
  Mmio16Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    (PCI_CFG_16B_READ (PciExpressBase, Bus, Device, Function, Register)& (AndValue)))

#define PCI_CFG_8B_OR(PciExpressBase, Bus, Device, Function, Register, OrValue) \
  Mmio8Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    (PCI_CFG_8B_READ (PciExpressBase, Bus, Device, Function, Register)|OrValue))

#define PCI_CFG_8B_AND(PciExpressBase, Bus, Device, Function, Register, AndValue) \
  Mmio8Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    (PCI_CFG_8B_READ (PciExpressBase, Bus, Device, Function, Register)& (AndValue)))


//
// Extended Configuration Base Address.  Port to value enabled by calling code, if necessary.
// If the EC space is not enabled by code calling the MRC, then enable the
// EC space to this value
//
#ifndef EC_BASE
#define EC_BASE                 PCIEX_BASE_ADDRESS
#endif
#endif
//
// Read Message Register
//
#define MSG_BUS_32B_READ(portid,offset,data)\
{\
  Mmio32Write (EC_BASE + 0xD8, 0); \
  Mmio32Write (EC_BASE + 0xD0, ((0x06000000) | (portid <<16)| ((offset) << 8) + 0xF0))   ; \
  (data) = Mmio32Read (EC_BASE + 0xD4);\
}

//
// Write Message Register
//
#define MSG_BUS_32B_WRITE(portid,offset,data)\
{\
  Mmio32Write (EC_BASE + 0xD8, 0); \
  Mmio32Write (EC_BASE + 0xD4, data)   ; \
  Mmio32Write (EC_BASE + 0xD0, ((0x07000000) | (portid <<16)| ((offset) << 8) + 0xF0)); \
}

//
// Read Message Register with Offset Hi
//
#define PSF_MSG_BUS_32B_READ(portid,offsethi,offsetlo,data)\
{\
  Mmio32Write (EC_BASE + 0xD8, offsethi); \
  Mmio32Write (EC_BASE + 0xD0, ((0x06000000) | (portid <<16)| ((offsetlo) << 8) + 0xF0))   ; \
  (data) = Mmio32Read (EC_BASE + 0xD4); \
}

//
// Write Message Register with Offset Hi
//
#define PSF_MSG_BUS_32B_WRITE(portid,offsethi,offsetlo,data)\
{\
  Mmio32Write (EC_BASE + 0xD8, offsethi); \
  Mmio32Write (EC_BASE + 0xD4, data)   ; \
  Mmio32Write (EC_BASE + 0xD0, ((0x07000000) | (portid <<16)| ((offsetlo) << 8) + 0xF0)); \
}

UINT32
GetRegisterAccessInfo (
  IN          UINT8       MrcDebugMsgLevel,
  IN          UINT8       BoxType,
  IN          UINT8       Instance,
  IN  OUT     UINT32      *Command,
  IN  OUT     UINT8       *PortId,
  IN  OUT     UINT32      *Offset,
  IN          UINT32      Mode
);

UINT32
MemRegRead (
  IN          UINT8       MrcDebugMsgLevel,
  IN          UINT8       BoxType,
  IN          UINT8       Instance,
  IN          UINT32      Offset
)
/*++

Routine Description:

  Reads registers from an specified Unit

Arguments:

  CurrentMrcData:   Host struture for all MRC global data.
  Boxtype:          Unit to select
  Instance:         Channel under test
  Offset:           Offset of register to read.

Returns:

  Value read

--*/
;

VOID
MemRegWrite (
  IN          UINT8       MrcDebugMsgLevel,
  IN          UINT8       BoxType,
  IN          UINT8       Instance,
  IN          UINT32      Offset,
  IN          UINT32      Data
)
/*++

Routine Description:

  Reads registers from an specified Unit

Arguments:

  CurrentMrcData:   Host struture for all MRC global data.
  Boxtype:          Unit to select
  Instance:         Channel under test
  Offset:           Offset of register to write.
  Data:             Data to be written

Returns:

  None

--*/
;

#ifndef _VLVACCESS_H_INCLUDED_

#ifndef IoIn8
#define IoIn8(Port) \
  IoRead8(Port)

#define IoIn16(Port) \
  IoRead16(Port)

#define IoIn32(Port) \
  IoRead32(Port)

#define IoOut8(Port, Data) \
  IoWrite8(Port, Data) 

#define IoOut16(Port, Data) \
  IoWrite16(Port, Data)

#define IoOut32(Port, Data) \
  IoWrite32(Port, Data)
#endif // IoIn8

#endif //VLVACCESS
#endif //IOACCESS



