//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
//
// $Header: $
//
// $Revision: $
//
// $Date: $
//
//*****************************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        Nb.h
//
// Description: This file contains NorthBridge specific structures and macro 
//              definitions
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>


#ifndef  _NB_H   //To Avoid this header get compiled twice
#define  _NB_H


// DO NOT REMOVE THE DEFINITION OF THIS STRUCTURE. THIS IS USED BY CSM ALSO

//<AMI_THDR_START>
//------------------------------------------------------------------
//
// Name:        ROOT_BRIDGE_MAPPING_ENTRY
//
// Fields:  Type    ParameterName   Description
//------------------------------------------------------------------
//  UINT32  rbUID Root bridge unique ID
//  UINT8   rbDevFunc Corresponding Root Bridge device/function number
//
// Description:	
//  This data structure contains the information that is used to 
//  map Root bridge UID to corresponding device/function number
//
//------------------------------------------------------------------
//<AMI_THDR_END>
typedef struct ROOT_BRIDGE_MAPPING_TABLE {
    UINT32  rbUID;
    UINT8   rbDevFunc;
} ROOT_BRIDGE_MAPPING_ENTRY;

#ifndef BIT0
#define BIT0                0x01
#define BIT1                0x02
#define BIT2                0x04
#define BIT3                0x08
#define BIT4                0x10
#define BIT5                0x20
#define BIT6                0x40
#define BIT7                0x80
#define BIT8                0x100
#define BIT9                0x200
#endif

#define NB_PCI_CFG_ADDRESS(bus,dev,func,reg)    \
    ((UINT64)((((UINTN)bus) << 24) + (((UINTN)dev) << 16) + \
    (((UINTN)func) << 8) + ((UINTN)reg))) & 0x00000000ffffffff

#define NB_EXTENDED_REGISTER(extreg)    (((UINT64)extreg) << 32)

// Define all the Northbridge specific equates and structures in this file

#define NB_REG(Reg)             NB_PCI_CFG_ADDRESS(0, 0, 0, Reg)
#define IGD_REG(Reg)            NB_PCI_CFG_ADDRESS(0, 2, 0, Reg)

#define NB_BUS_DEV_FUN          NB_REG(0)
//CSP20140329_22(-) #undef IGD_BUS_DEV_FUN
#define AMI_IGD_BUS_DEV_FUN     		IGD_REG(0) //CSP20140329_22


EFI_STATUS 
NbDxeBoardInit (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable 
);

#endif 

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
