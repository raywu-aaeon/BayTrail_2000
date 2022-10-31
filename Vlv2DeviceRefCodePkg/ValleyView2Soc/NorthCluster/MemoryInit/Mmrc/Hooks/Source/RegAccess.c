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

  This file handles register accesses.

--*/
#ifndef _REGACCESS_H
#define _REGACCESS_H

#include "../Include/RegAccess.h"

//
// Memory Mapped IO
//
UINT32
Mmio32Read (
  IN  UINT32 RegisterAddress
)
{
  VOLATILE UINT32 *addr = (UINT32 *) RegisterAddress;
  return *addr;
}

UINT16
Mmio16Read (
  IN    UINT32  RegisterAddress
)
{
  VOLATILE UINT16 *addr = (UINT16 *) RegisterAddress;
  return *addr;
}

UINT8
Mmio8Read (
  IN    UINT32  RegisterAddress
)
{
  VOLATILE UINT8 *addr = (UINT8 *) RegisterAddress;
  return *addr;
}

VOID
Mmio32Write (
  IN    UINT32  RegisterAddress,
  IN    UINT32  Value
)
{
  VOLATILE UINT32 *addr = (UINT32 *) RegisterAddress;
  *addr = Value;
}

VOID
Mmio16Write (
  IN    UINT32  RegisterAddress,
  IN    UINT16  Value
)
{
  VOLATILE UINT16 *addr = (UINT16 *) RegisterAddress;
  *addr = Value;
}

VOID
Mmio8Write (
  IN    UINT32  RegisterAddress,
  IN    UINT8   Value
)
{
  VOLATILE UINT8 *addr = (UINT8 *) RegisterAddress;
  *addr = Value;
}

const UINT8 DUNIT_PortID[MAX_CHANNELS]= {
		0x1,
		0x07
};
const UINT8 CPGC_PortID[MAX_CHANNELS]= {
		0xd,
		0x9
};
UINT32
GetRegisterAccessInfo (
  IN          UINT8       MrcDebugMsgLevel,
  IN          UINT8       BoxType,
  IN          UINT8       Instance,
  IN  OUT     UINT32      *Command,
  IN  OUT     UINT8       *PortId,
  IN  OUT     UINT32      *Offset,
  IN          UINT32      Mode
)
{
  UINT32 Error;
  Error = 0;
  //
  // Get only the low word of the offset since the registers currently have meta-information
  // stored in the upper 16 bits which we are not using at present.
  //
  *Offset &= 0x0000FFFF;
  //
  // These are the typical read/write command for CPU units.
  //
  if (Mode == MODE_READ) {
    *Command = 0x10000000;
  } else if (Mode == MODE_WRITE) {
    *Command = 0x11000000;
  } else {
    Error = INVALID_MODE;
  }
  switch (BoxType) {
  case DUNIT:
    //
    // Get only the low byte of the offset since the DUNIT registers currently have meta-information
    // stored in the upper 24 bits which we are not using at present.
    //
    *Offset &= 0x000000FF;
    *PortId = DUNIT_PortID[Instance];//0x01;
    break;
  case CPGC:
    //
    // Get only the low byte of the offset since the CPGC registers currently have meta-information
    // stored in the upper 24 bits which we are not using at present.
    //
    *Offset &= 0x000000FF;
    *PortId = CPGC_PortID[Instance];//0x0D;
    break;
  case DDRIO:
    //
    // Get only the low word of the offset since the DDRIO registers currently have meta-information
    // stored in the upper 16 bits which we are not using at present.
    //
    *Offset &= 0x0000FFFF;
    if (Mode == MODE_READ) {
      *Command = 0x06000000;
    } else if (Mode == MODE_WRITE) {
      *Command = 0x07000000;
    } else {
      Error = INVALID_MODE;
    }
    *PortId = 0x0C;
    break;
  case TUNIT:
    *PortId = 0x02;
    break;
  case PUNIT:
    *PortId = 0x04;
    break;
  case RTF:
    *PortId = 0xA6;
    break;
  case AUNIT:
    *PortId = 0x00;
    break;
  case BUNIT:
    *PortId = 0x03;
    break;
  case CUNIT:
    *PortId = 0x08;
    break;
  case DRNG:
    *PortId = 0x0f;
    break;
  case CCK:
    *PortId = 0x14;
    break;
  default:
    Error = INVALID_BOX_TYPE;
    break;
  }
  return Error;
}

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

  ModMrcData:   Host struture for all MRC global data.
  Boxtype:          Unit to select
  Instance:         Channel under test
  Offset:           Offset of register to read.

Returns:

  Value read

--*/
{
  UINT32 Command;
  UINT8  PortId;
  UINT32 Data;

  if (GetRegisterAccessInfo (MrcDebugMsgLevel, BoxType, Instance, &Command, &PortId, &Offset, MODE_READ) == NO_ERROR) {
    //
    // We have the ability to force register reads in simulation by bypassing the sideband and going
    // straight to the RTL signals. To enable, SIM_REGISTER_ACCESS_BYPASS_SIDEBAND must be enabled.
    // In addition, on the simulation run command line, the following argument must be specified:
    // -simv_args +DDR_MRC_FAST
    //
    Mmio32Write (EC_BASE + 0xD8, Offset & 0xFFFFFF00);
    Mmio32Write (EC_BASE + 0xD0, (Command | (PortId << 16) | ( (Offset & 0x000000FF) << 8) | 0xF0) );
    Data = Mmio32Read (EC_BASE + 0xD4);
    
    return Data;
  }
  return 0xFFFFFFFF;
}

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

  ModMrcData:   Host struture for all MRC global data.
  Boxtype:          Unit to select
  Instance:         Channel under test
  Offset:           Offset of register to write.
  Data:             Data to be written

Returns:

  None

--*/
{
  UINT32 Command;
  UINT8  PortId;

  if (GetRegisterAccessInfo (MrcDebugMsgLevel, BoxType, Instance, &Command, &PortId, &Offset, MODE_WRITE) == NO_ERROR) {
    //
    // We have the ability to force register writes in simulation by bypassing the sideband and going
    // straight to the RTL signals. To enable, SIM_REGISTER_ACCESS_BYPASS_SIDEBAND must be enabled.
    // In addition, on the simulation run command line, the following argument must be specified:
    // -simv_args +DDR_MRC_FAST
    //
    Mmio32Write (EC_BASE + 0xD8, Offset & 0xFFFFFF00);
    Mmio32Write (EC_BASE + 0xD4, Data);
    Mmio32Write (EC_BASE + 0xD0, (Command | (PortId << 16) | ( (Offset & 0x000000FF) << 8) | 0xF0) );
  }
}

#endif
