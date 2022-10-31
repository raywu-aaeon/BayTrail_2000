//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/VirtualSerialDevice/VirtualSerial.h 2     4/13/10 12:36a Rameshr $
//
// $Revision: 2 $
//
// $Date: 4/13/10 12:36a $
//**********************************************************************

//<AMI_FHDR_START>
//--------------------------------------------------------------------------
//
// Name: 	 VirtualSerial.h	
//
// Description: Header file for the Virtual Serial Device.
//
//--------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _EFI_VIRTUAL_SERIAL_H_
#define _EFI_VIRTUAL_SERIAL_H_

#include <AmiDxeLib.h>
#include <Token.h>
#include <Protocol\AmiSio.h>
#include <AcpiRes.h>

typedef struct {
    UINT16      BaseAddress;
    UINT8       Irq;
    UINT32      Uid;
} VIRTUAL_SERIAL_DETAILS;

EFI_STATUS 
VirtualSerialRegister(
    IN AMI_SIO_PROTOCOL *This,
    IN BOOLEAN          Write,
    IN BOOLEAN          ExitCfgMode,
    IN UINT8           	Register,
    IN OUT UINT8       	*Value
);

EFI_STATUS 
VirtualSerialCRS(
    IN AMI_SIO_PROTOCOL *This,
    IN BOOLEAN          Set,
    IN OUT T_ITEM_LIST  **Resources
);

EFI_STATUS 
VirtualSerialPRS(
    IN AMI_SIO_PROTOCOL     *This,
    IN BOOLEAN              Set,
    IN OUT T_ITEM_LIST      **Resources
);


#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
