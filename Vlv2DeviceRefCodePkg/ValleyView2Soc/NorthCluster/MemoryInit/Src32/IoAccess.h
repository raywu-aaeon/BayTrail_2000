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

#ifndef _IOACCESS_H_
#define _IOACCESS_H_

#include "Mrc.h"
#include "MchRegs.h"
//#include "MMRCLibraries.h"
#include <Library/IoLib.h>

//
//
// Memory Mapped IO
//
#define Mmio32Read( Register ) MmioRead32(Register)
#define Mmio32Write( Register, Value ) MmioWrite32(Register, Value)

#define Mmio16Read( Register )   MmioRead16(Register)
#define Mmio16Write( Register, Value ) MmioWrite16(Register, Value)

#define Mmio8Read( Register ) MmioRead8(Register)
#define Mmio8Write( Register, Value ) MmioWrite8(Register, Value)




#ifndef Mmio32Or
#define Mmio32Or( Register, OrData) Mmio32Write(Register, Mmio32Read(Register)|OrData)
#define Mmio16Or( Register, OrData) Mmio16Write(Register, Mmio16Read(Register)|OrData)
#define Mmio8Or( Register, OrData) Mmio8Write( Register, Mmio8Read(Register)|OrData)
#endif

#ifndef Mmio32And
#define Mmio32And( Register, AndData) Mmio32Write(Register, Mmio32Read(Register)& (AndData))
#define Mmio16And( Register, AndData) Mmio16Write(Register, Mmio16Read(Register)& (AndData))
#define Mmio8And( Register, AndData) Mmio8Write( Register, Mmio8Read(Register)& (AndData))
#endif

#ifndef Mmio32AndThenOr
#define Mmio32AndThenOr( Register, AndData, OrData) Mmio32Write(Register, (((Mmio32Read(Register)& (AndData)))|OrData))
#define Mmio16AndThenOr( Register, AndData, OrData) Mmio16Write(Register, (((Mmio16Read(Register)& (AndData)))|OrData))
#define Mmio8AndThenOr( Register, AndData, OrData) Mmio8Write(Register, (((Mmio8Read(Register)& (AndData)))|OrData))
#endif

/**************************
//
//Memory Mapped IO with implicit MCHBAR offset, for programmer convenience
//

#define McMmio32Read( Register ) Mmio32Read(CurrentMrcData->MchBar+Register)
#define McMmio16Read( Register ) Mmio16Read(CurrentMrcData->MchBar+Register)
#define McMmio8Read( Register ) Mmio8Read(CurrentMrcData->MchBar+Register)

#define McMmio32Write( Register, Value ) Mmio32Write(CurrentMrcData->MchBar+Register, Value)
#define McMmio16Write( Register, Value ) Mmio16Write(CurrentMrcData->MchBar+Register, Value)
#define McMmio8Write( Register, Value ) Mmio8Write(CurrentMrcData->MchBar+Register, Value)

#define McMmio32AndThenOr( Register, AndData, OrData) McMmio32Write(Register, (((McMmio32Read(Register) & (AndData)))|OrData))
#define McMmio16AndThenOr( Register, AndData, OrData) McMmio16Write(Register, (((McMmio16Read(Register) & (AndData)))|OrData))
#define McMmio8AndThenOr( Register, AndData, OrData) McMmio8Write(Register, (((McMmio8Read(Register) & (AndData)))|OrData))

#define McMmio32And( Register, AndData) McMmio32Write(Register, McMmio32Read(Register) & (AndData))
#define McMmio16And( Register, AndData) McMmio16Write(Register, McMmio16Read(Register) & (AndData))
#define McMmio8And( Register, AndData) McMmio8Write(Register, McMmio8Read(Register) & (AndData))

#define McMmio32Or( Register, OrData) McMmio32Write(Register, McMmio32Read(Register)|OrData)
#define McMmio16Or( Register, OrData) McMmio16Write(Register, McMmio16Read(Register)|OrData)
#define McMmio8Or( Register, OrData) McMmio8Write(Register, McMmio8Read(Register)|OrData)

***************************/
//
// Memory mapped PCI IO
//

#define PciCfgPtr(Bus, Device, Function, Register )\
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register)

#define PciCfg32Read_CF8CFC(B,D,F,R) \
  (UINT32)(IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoIn32(0xCFC))

#define PciCfg32Write_CF8CFC(B,D,F,R,Data) \
  (IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoOut32(0xCFC,Data))

#define PciCfg32AndThenOr_CF8CFC(B,D,F,R,A,O) \
  PciCfg32Write_CF8CFC(B,D,F,R, \
    (PciCfg32Read_CF8CFC(B,D,F,R) & (A)) | (O))

#define PciCfg16Read_CF8CFC(B,D,F,R) \
  (UINT16)(IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoIn16(0xCFC))

#define PciCfg16Write_CF8CFC(B,D,F,R,Data) \
  (IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoOut16(0xCFC,Data))

#define PciCfg16AndThenOr_CF8CFC(B,D,F,R,A,O) \
  PciCfg16Write_CF8CFC(B,D,F,R, \
    (PciCfg16Read_CF8CFC(B,D,F,R) & (A)) | (O))

#define PciCfg8Read_CF8CFC(B,D,F,R) \
  (UINT8)(IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoIn8(0xCFC))

#define PciCfg8Write_CF8CFC(B,D,F,R,Data) \
  (IoOut32(0xCF8,(0x80000000|(B<<16)|(D<<11)|(F<<8)|(R))),IoOut8(0xCFC,Data))

#define PciCfg8AndThenOr_CF8CFC(B,D,F,R,A,O) \
  PciCfg8Write_CF8CFC(B,D,F,R, \
    (PciCfg8Read_CF8CFC(B,D,F,R) & (A)) | (O))


#define PciCfg32Read( PciExpressBase, Bus, Device, Function, Register ) \
  Mmio32Read(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register))

#define PciCfg32Write( PciExpressBase, Bus, Device, Function, Register, Value ) \
  Mmio32Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    Value)

#define PciCfg16Read( PciExpressBase, Bus, Device, Function, Register ) \
  Mmio16Read(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register))

#define PciCfg16Write( PciExpressBase, Bus, Device, Function, Register, Value ) \
  Mmio16Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    Value)

#define PciCfg8Read( PciExpressBase, Bus, Device, Function, Register ) \
  Mmio8Read(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register))

#define PciCfg8Write(PciExpressBase, Bus, Device, Function, Register, Value ) \
  Mmio8Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    Value)

#define PciCfg32Or( PciExpressBase, Bus, Device, Function, Register, OrValue ) \
  Mmio32Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    (PciCfg32Read(PciExpressBase, Bus, Device, Function, Register)|OrValue))

#define PciCfg32And( PciExpressBase, Bus, Device, Function, Register, AndValue ) \
  Mmio32Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    (PciCfg32Read(PciExpressBase, Bus, Device, Function, Register)& (AndValue)))

#define PciCfg16Or( PciExpressBase, Bus, Device, Function, Register, OrValue ) \
  Mmio16Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    (PciCfg16Read(PciExpressBase, Bus, Device, Function, Register)|OrValue))

#define PciCfg16And( PciExpressBase, Bus, Device, Function, Register, AndValue ) \
  Mmio16Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    (PciCfg16Read(PciExpressBase, Bus, Device, Function, Register)& (AndValue)))

#define PciCfg8Or( PciExpressBase, Bus, Device, Function, Register, OrValue ) \
  Mmio8Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    (PciCfg8Read(PciExpressBase, Bus, Device, Function, Register)|OrValue))

#define PciCfg8And( PciExpressBase, Bus, Device, Function, Register, AndValue ) \
  Mmio8Write(PciExpressBase + \
    (UINTN)(Bus << 20) + \
    (UINTN)(Device << 15) + \
    (UINTN)(Function << 12) + \
    (UINTN)(Register), \
    (PciCfg8Read(PciExpressBase, Bus, Device, Function, Register)& (AndValue)))


/* VLV Message Bus Register Definitions */
#define VLV_REG_DRP                 0x0100
#define VLV_REG_DTR0                0x0101
#define VLV_REG_DTR1                0x0102
#define VLV_REG_DCO                 0x0103


/* System Offsets */
#ifndef EC_BASE
#define EC_BASE                     0xE0000000      /**< Extended Configuration Base Address.  Port to value enabled by calling code,
                                                     *   if necessary.
                                                     *   If the EC space is not enabled by code calling the MRC, then enable the
                                                     *   EC space to this value via the instructions in the function header of 
                                                     *   LincroftMemInit (mrc.c) at the place marked '?? OEM PORTING REQUIRED ??'.
                                                     */
#endif

/** Read Message Bus Register */
//#define MsgBusWriteMcrx(offsethi)            Mmio32Write(EC_BASE + 0xD8, offsethi);

/** RUnit Read Message Bus Register */
//#define RUnitAdrLo32Read(offsetlo,data)      Mmio32Write(EC_BASE + 0xD0, ((0x06000000) | (VLV_UNIT_DDRIO <<16)| ((offsetlo) << 8) + 0xF0))   ; \
//										     (data) = Mmio32Read(EC_BASE + 0xD4);

/** RUnit Write Message Bus Register */                                
#define RUnitAdrLo32Write(offsetlo,data)     Mmio32Write(EC_BASE + 0xD4, data)   ; \
                                             Mmio32Write(EC_BASE + 0xD0, ((0x07000000) | (VLV_UNIT_DDRIO <<16)| ((offsetlo) << 8) + 0xF0)); 


#define AunitMsgBus32Read(offset,data)       Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_READ_REG) | (VLV_UNIT_AUNIT <<16)| ((offset) << 8) + 0xF0))   ; \
                                             (data) = Mmio32Read(EC_BASE + 0xD4);

//#define AunitMsgBus32Read(offset,data)       *((volatile uint32_t*) (EC_BASE + 0xD0)) = ((VLV_CMD_READ_REG) | (VLV_UNIT_AUNIT <<16)| ((offset) << 8) + 0xF0); \
//                                             (data) = *((volatile UINT32*) (EC_BASE + 0xD4));
											


/** GUnit Read Message Bus Register */
#define GUnitMsgBus32Read(offsethi,offsetlo,data)        Mmio32Write(EC_BASE +(2<<15) +0xA8, offsethi); \
												         Mmio32Write(EC_BASE +(2<<15) +0xA0, ((VLV_CMD_READ_REG) | (VLV_UNIT_GUNIT <<16)| ((offsetlo) << 8) + 0xF0))   ; \
										                 (data) = Mmio32Read(EC_BASE +(2<<15) +0xA4);

/** GUnit Write Message Bus Register */                                
#define GUnitMsgBus32Write(offsethi,offsetlo,data)      Mmio32Write(EC_BASE +(2<<15) +0xA8, offsethi); \
                                                      Mmio32Write(EC_BASE +(2<<15) +0xA4, data)   ; \
                                                      Mmio32Write(EC_BASE +(2<<15) +0xA0, ((VLV_CMD_WRITE_REG) | (VLV_UNIT_GUNIT <<16)| ((offsetlo) << 8) + 0xF0)); 



/** DUNIT wake up command */
//#define DramWakeCommand()         Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_WAKE) | (VLV_UNIT_DUNIT <<16)| (0xF0)));

                                        
/** JEDEC Command */
#define JEDEC_CMD(data)             Mmio32Write(EC_BASE + 0xD8, 0); \
                                    Mmio32Write(EC_BASE + 0xD4, data)   ; \
                                    Mmio32Write(EC_BASE + 0xD0, ((VLV_CMD_DRAM_INIT) | ((VLV_UNIT_DUNIT << 16) + 0xF0)));                                        




//
// IO
//


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

#endif

VOID
RunitMsgBusOr(
  UINT32 RegisterAddress,  
  UINT32 Value
  );

VOID
RunitMsgBusAnd(
  UINT32 RegisterAddress, 
  UINT32 Mask
  );

VOID
RunitMsgBusAndThenOr(
  UINT32 RegisterAddress, 
  UINT32 Mask, 
  UINT32 Value
  );

UINT32  MsgBus32Read(UINT8 portid, UINT16 offset);
VOID    MsgBus32Write(UINT8 portid, UINT16 offset, UINT32 data);
VOID    MsgBus32AndThenOr(UINT8 portid, UINT16 offset, UINT32 mask, UINT32 data);

VOID 	MsgBus32WriteDualDunit(UINT8 portid, UINT16 offset, UINT32 data);
VOID    DramInitCommand(UINT8 portid, UINT32 data);
VOID 	DramInitCommandDualDunit(UINT8 portid, UINT32 data);
VOID 	DramWakeCommand();
VOID 	DramWakeCommandDualDunit();

UINT32  RUnitMsgBus32Read(UINT16 offsethi, UINT16 offsetlo);
VOID    RUnitMsgBus32Write(UINT16 offsethi, UINT16 offsetlo, UINT32 data);
VOID    RUnitMsgBus16Read(UINT16 offsethi, UINT16 offsetlo, UINT16 data);
VOID    RUnitMsgBus16Write(UINT16 offsethi, UINT16 offsetlo, UINT16 data);
UINT8   RUnitMsgBus8Read(UINT16 offsethi, UINT16 offsetlo);
VOID    RUnitMsgBus8Write(UINT16 offsethi, UINT16 offsetlo, UINT8 data);

UINT8 DunitPortID[MAX_CHANNELS_TOTAL];

//UINT32  MrcIosfReadField(UINT8, IOSF_sb_Reg_t );
//VOID    MrcIosfWriteField(UINT8, IOSF_sb_Reg_t, UINT32);

//UINT32 mmrc_iosfreadfield(UINT8 port_u8, mmrc_iosfreg_t reg_sr);
//VOID mmrc_iosfwritefield(UINT8 port_u8, mmrc_iosfreg_t reg_sr, UINT32 value_u32);

#endif
