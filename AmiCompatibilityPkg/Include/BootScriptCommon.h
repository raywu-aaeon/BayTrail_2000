//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/BIN/Core/Include/BootScriptCommon.h 5     6/24/11 1:47p Felixp $
//
// $Revision: 5 $
//
// $Date: 6/24/11 1:47p $
//**********************************************************************


//<AMI_FHDR_START>
//---------------------------------------------------------------------------
// Name:	BootScriptCommon.h
//
// Description: Boot Script Common definitions header.
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef __BOOT_SCRIPT_COMMON__H__
#define __BOOT_SCRIPT_COMMON__H__

// Include files from EDKII
// MdePkg:
#include <Pi/PiS3BootScript.h>

#ifdef __cplusplus
extern "C" {
#endif

// Pi/PiS3BootScript.h
/*
//*******************************************
// EFI Save State Script Opcode definitions (Common)
//*******************************************
#define EFI_BOOT_SCRIPT_IO_WRITE_OPCODE                 0x00
#define EFI_BOOT_SCRIPT_IO_READ_WRITE_OPCODE            0x01
#define EFI_BOOT_SCRIPT_MEM_WRITE_OPCODE                0x02
#define EFI_BOOT_SCRIPT_MEM_READ_WRITE_OPCODE           0x03
#define EFI_BOOT_SCRIPT_PCI_CONFIG_WRITE_OPCODE         0x04
#define EFI_BOOT_SCRIPT_PCI_CONFIG_READ_WRITE_OPCODE    0x05
#define EFI_BOOT_SCRIPT_SMBUS_EXECUTE_OPCODE            0x06
#define EFI_BOOT_SCRIPT_STALL_OPCODE                    0x07
#define EFI_BOOT_SCRIPT_DISPATCH_OPCODE                 0x08

#if PI_SPECIFICATION_VERSION>=0x0001000A
//*******************************************
// EFI Save State Script Opcode definitions (PI)
//*******************************************
#define EFI_BOOT_SCRIPT_DISPATCH_2_OPCODE               0x09
#define EFI_BOOT_SCRIPT_INFORMATION_OPCODE              0x0A
#define EFI_BOOT_SCRIPT_PCI_CONFIG2_WRITE_OPCODE        0x0B
#define EFI_BOOT_SCRIPT_PCI_CONFIG2_READ_WRITE_OPCODE   0x0C
#define EFI_BOOT_SCRIPT_IO_POLL_OPCODE                  0x0D
#define EFI_BOOT_SCRIPT_MEM_POLL_OPCODE                 0x0E
#define EFI_BOOT_SCRIPT_PCI_CONFIG_POLL_OPCODE          0x0F
#define EFI_BOOT_SCRIPT_PCI_CONFIG2_POLL_OPCODE         0x10
*/
#define EFI_BOOT_SCRIPT_LABEL_OPCODE_OEM                0x83
//#endif

#define EFI_BOOT_SCRIPT_TABLE_OPCODE                  0xAA
#define EFI_BOOT_SCRIPT_TERMINATE_OPCODE              0xFF

// Pi/PiS3BootScript.h
/*
//*******************************************
// EFI_BOOT_SCRIPT_WIDTH
//*******************************************
typedef enum {
	EfiBootScriptWidthUint8,
	EfiBootScriptWidthUint16,
	EfiBootScriptWidthUint32,
	EfiBootScriptWidthUint64,
	EfiBootScriptWidthFifoUint8,
	EfiBootScriptWidthFifoUint16,
	EfiBootScriptWidthFifoUint32,
	EfiBootScriptWidthFifoUint64,
	EfiBootScriptWidthFillUint8,
	EfiBootScriptWidthFillUint16,
	EfiBootScriptWidthFillUint32,
	EfiBootScriptWidthFillUint64,
	EfiBootScriptWidthMaximum
} EFI_BOOT_SCRIPT_WIDTH;
*/

//////////////////////////////////////////////////////////////////////////////////

/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
