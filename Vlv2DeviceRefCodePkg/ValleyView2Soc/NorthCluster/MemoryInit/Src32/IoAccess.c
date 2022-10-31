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

  IoAccess.h

Abstract:

  This file include external IO Access.

--*/
#include "IoAccess.h"
#if MAX_CHANNELS == 1
UINT8 DunitPortID[MAX_CHANNELS] = {
  VLV_UNIT_DUNIT0
};
#endif

#if MAX_CHANNELS == 2
UINT8 DunitPortID[MAX_CHANNELS] = {
  VLV_UNIT_DUNIT0,
  VLV_UNIT_DUNIT1
  };
#endif

UINT32 MsgBus32Read(UINT8 portid, UINT16 offset) {       
    UINT32 data;
    Mmio32Write(EC_BASE + 0xD8, 0);
	Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_READ_REG) | (portid <<16)| ((offset) << 8) + 0xF0));
	data = Mmio32Read(EC_BASE + 0xD4);
	return data;
}


VOID MsgBus32Write(UINT8 portid, UINT16 offset, UINT32 data) {
    Mmio32Write(EC_BASE + 0xD8, 0); 
    Mmio32Write(EC_BASE + 0xD4, data); 
    Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_WRITE_REG) | (portid <<16)| ((offset) << 8) + 0xF0)); 
}

VOID MsgBus32AndThenOr(UINT8 portid, UINT16 offset, UINT32 mask, UINT32 data) {
    UINT32 data32;
    data32 = MsgBus32Read(portid, offset);
    data32 &= mask;
    data32 |= data;
    MsgBus32Write (portid, offset, data32);
    return;
}

VOID MsgBus32WriteDualDunit(UINT8 portid, UINT16 offset, UINT32 data) {
    Mmio32Write(EC_BASE + 0xD8, 0);
    Mmio32Write(EC_BASE + 0xD4, data);
    Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_WRITE_REG) | (VLV_UNIT_DUNIT0 <<16)| ((offset) << 8) + 0xF0));
    Mmio32Write(EC_BASE + 0xD8, 0);
    Mmio32Write(EC_BASE + 0xD4, data);
    Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_WRITE_REG) | (VLV_UNIT_DUNIT1 <<16)| ((offset) << 8) + 0xF0));
}
/** Message Bus Command */
VOID DramInitCommand(UINT8 portid, UINT32 data) {
    Mmio32Write(EC_BASE + 0xD8, 0); 
    Mmio32Write(EC_BASE + 0xD4, data)   ; 
    Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_DRAM_INIT) | (portid <<16)| (0xF0)));
}

/** Message Bus Command */
VOID DramInitCommandDualDunit(UINT8 portid, UINT32 data){
	Mmio32Write(EC_BASE + 0xD8, 0);
	Mmio32Write(EC_BASE + 0xD4, data); 
	Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_DRAM_INIT) | (VLV_UNIT_DUNIT0 <<16)| (0xF0)));
	Mmio32Write(EC_BASE + 0xD8, 0);
	Mmio32Write(EC_BASE + 0xD4, data); 
	Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_DRAM_INIT) | (VLV_UNIT_DUNIT1 <<16)| (0xF0)));
}


/** DUNIT wake up command */
VOID DramWakeCommand(){
	Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_WAKE) | (VLV_UNIT_DUNIT0 <<16)| (0xF0)));
}

/** DUNIT wake up command */
VOID DramWakeCommandDualDunit(){
	Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_WAKE) | (VLV_UNIT_DUNIT0 <<16)| (0xF0)));
	Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_WAKE) | (VLV_UNIT_DUNIT1 <<16)| (0xF0)));
}

/** RUnit Read Message Bus Register */
UINT32 RUnitMsgBus32Read(UINT16 offsethi, UINT16 offsetlo) {
    UINT32 data;
    Mmio32Write(EC_BASE + 0xD8, offsethi); 
    Mmio32Write(EC_BASE + 0xD0, ((0x06000000) | (VLV_UNIT_DDRIO <<16)| ((offsetlo) << 8) + 0xF0))   ; 
    data = Mmio32Read(EC_BASE + 0xD4);
    return data;
}

/** RUnit Write Message Bus Register */                                
VOID RUnitMsgBus32Write(UINT16 offsethi, UINT16 offsetlo, UINT32 data) {
    Mmio32Write(EC_BASE + 0xD8, offsethi); 
    Mmio32Write(EC_BASE + 0xD4, data)   ; 
    Mmio32Write(EC_BASE + 0xD0, ((0x07000000) | (VLV_UNIT_DDRIO <<16)| ((offsetlo) << 8) + 0xF0)); 
}

/** RUnit Read Message Bus Register */
VOID RUnitMsgBus16Read(UINT16 offsethi, UINT16 offsetlo, UINT16 data) {
    Mmio32Write(EC_BASE + 0xD8, offsethi); 
    Mmio32Write(EC_BASE + 0xD0, ((0x06000000) | (VLV_UNIT_DDRIO <<16)| ((offsetlo) << 8) + 0xF0))   ; 
    (data) = Mmio16Read(EC_BASE + 0xD4);
}

/** RUnit Read Message Bus Register */
VOID RUnitMsgBus16Write(UINT16 offsethi, UINT16 offsetlo, UINT16 data) {
     Mmio32Write(EC_BASE + 0xD8, offsethi); 
     Mmio16Write(EC_BASE + 0xD4, data)   ; 
     Mmio32Write(EC_BASE + 0xD0, ((0x07000000) | (VLV_UNIT_DDRIO <<16)| ((offsetlo) << 8) + 0x30));
}
/** RUnit Read Message Bus Register */
UINT8 RUnitMsgBus8Read(UINT16 offsethi, UINT16 offsetlo) {
    UINT8 data;
    Mmio32Write(EC_BASE + 0xD8, offsethi); 
    Mmio32Write(EC_BASE + 0xD0, ((0x06000000) | (VLV_UNIT_DDRIO <<16)| ((offsetlo) << 8) + 0xF0))   ; 
    data = Mmio8Read(EC_BASE + 0xD4);
    return data;
}

/** RUnit Read Message Bus Register */
VOID RUnitMsgBus8Write(UINT16 offsethi, UINT16 offsetlo, UINT8 data)  {
    Mmio32Write(EC_BASE + 0xD8, offsethi); 
    Mmio8Write(EC_BASE + 0xD4, data)   ; 
    Mmio32Write(EC_BASE + 0xD0, ((0x07000000) | (VLV_UNIT_DDRIO <<16)| ((offsetlo) << 8) + 0x10));
}

VOID
RunitMsgBusOr(
  UINT32 RegisterAddress,  
  UINT32 Value
  )
{
    //operation 0 = Or, 1 = And, 2 = AndThenOr
    UINT32 buffer32 = 0;
	
    buffer32 = RUnitMsgBus32Read((UINT16)(RegisterAddress&MSGBUS_MASKHI),(UINT16)(RegisterAddress&MSGBUS_MASKLO));
  	buffer32 |= Value;
    RUnitMsgBus32Write((UINT16)(RegisterAddress&MSGBUS_MASKHI),(UINT16)(RegisterAddress&MSGBUS_MASKLO),buffer32);
	
}

VOID
RunitMsgBusAnd(
  UINT32 RegisterAddress, 
  UINT32 Mask
  )
{
    //operation 0 = Or, 1 = And, 2 = AndThenOr
    UINT32 buffer32 = 0;
	
    buffer32 = RUnitMsgBus32Read((UINT16)(RegisterAddress&MSGBUS_MASKHI),(UINT16)(RegisterAddress&MSGBUS_MASKLO));
    buffer32 &= Mask;
    RUnitMsgBus32Write((UINT16)(RegisterAddress&MSGBUS_MASKHI),(UINT16)(RegisterAddress&MSGBUS_MASKLO),buffer32);
	
}

VOID
RunitMsgBusAndThenOr(
  UINT32 RegisterAddress, 
  UINT32 Mask, 
  UINT32 Value
  )
{
    //operation 0 = Or, 1 = And, 2 = AndThenOr
    UINT32 buffer32 = 0;
	
    buffer32 = RUnitMsgBus32Read((UINT16)(RegisterAddress&MSGBUS_MASKHI),(UINT16)(RegisterAddress&MSGBUS_MASKLO));
    buffer32 &= Mask;
	buffer32 |= Value;
    RUnitMsgBus32Write((UINT16)(RegisterAddress&MSGBUS_MASKHI),(UINT16)(RegisterAddress&MSGBUS_MASKLO),buffer32);
	
}

