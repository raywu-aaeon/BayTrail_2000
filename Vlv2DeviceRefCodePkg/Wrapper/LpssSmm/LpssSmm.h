//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************

#ifndef _LPSS_SMM_H_
#define _LPSS_SMM_H_

#include <Token.h>
#include <AmiDxeLib.h>
#include <AmiCspLib.h>

#include <Protocol/SmmBase2.h>
#include <Protocol/SmmSwDispatch2.h>
#include <PchAccess.h>

#define EC_BASE           0xE0000000
#define MC_MCR            0x000000D0 // Cunit Message Control Register
#define MC_MDR            0x000000D4 // Cunit Message Data Register
#define MC_MCRX           0x000000D8 // Cunit Message Control Register Extension

#define MSG_BUS_ENABLED   0x000000F0
#define MSGBUS_MASKHI     0xFFFFFF00
#define MSGBUS_MASKLO     0x000000FF
#define MESSAGE_DWORD_EN  BIT4 | BIT5 | BIT6 | BIT7

#define PCH_SCC_EP_PRIVATE_READ_OPCODE           0x06  // CUnit to SCC EP Private Space Read Opcode

#define R_PCH_SCC_EP_PCICFGCTR1                  0x500 // PCI Configuration Control 1 - eMMC
#define B_PCH_SCC_EP_PCICFGCTR1_ACPI_INT_EN1     BIT1  // ACPI Interrupt Enable
#define B_PCH_SCC_EP_PCICFGCTR1_PCI_CFG_DIS1     BIT0  // PCI Configuration Space Disable
#define R_PCH_SCC_EP_PCICFGCTR4                  0x50C // PCI Configuration Control 4 - EMMC45

#define MmioAddress( BaseAddr, Register ) \
  ( (UINTN)BaseAddr + \
    (UINTN)(Register) \
  )
#define Mmio32Ptr( BaseAddr, Register ) \
  ( (volatile UINT32 *)MmioAddress( BaseAddr, Register ) )
#define Mmio32( BaseAddr, Register ) \
  *Mmio32Ptr( BaseAddr, Register )

#define SccMsgBus32AndThenOr( Register, Dbuff, AndData, OrData ) \
  { \
    Mmio32( EC_BASE, MC_MCRX) = ( (Register & MSGBUS_MASKHI)); \
    Mmio32( EC_BASE, MC_MCR ) = (UINT32)( (PCH_SCC_EP_PRIVATE_READ_OPCODE << 24) | (PCH_SCC_EP_PORT_ID << 16) | ((Register & MSGBUS_MASKLO) << 8) | MESSAGE_DWORD_EN); \
    (UINT32)(Dbuff) = Mmio32( EC_BASE, MC_MDR ); \
    Mmio32( EC_BASE, MC_MCRX) = ( (Register & MSGBUS_MASKHI)); \
    Mmio32( EC_BASE, MC_MDR ) = ((Dbuff & AndData) | OrData); \
    Mmio32( EC_BASE, MC_MCR ) = (UINT32)( (PCH_SCC_EP_PRIVATE_WRITE_OPCODE << 24) | (PCH_SCC_EP_PORT_ID << 16) | ((Register & MSGBUS_MASKLO) << 8) | MESSAGE_DWORD_EN); \
  }

#define SccMsgBusRead32(Register, Dbuff) \
{ \
    Mmio32( EC_BASE, MC_MCRX) = ( (Register & MSGBUS_MASKHI)); \
    Mmio32( EC_BASE, MC_MCR ) = (UINT32)( (PCH_SCC_EP_PRIVATE_READ_OPCODE << 24) | (PCH_SCC_EP_PORT_ID << 16) | ((Register & MSGBUS_MASKLO) << 8) | MESSAGE_DWORD_EN); \
    (UINT32)(Dbuff) = Mmio32( EC_BASE, MC_MDR ); \
}

#define EMMC_441    1
#define EMMC_45     2
#define SD_CARD     3 //EIP143364 

#endif

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
