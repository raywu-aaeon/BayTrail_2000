//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2009, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/BIN/Modules/ACPI/Template/S3Support/S3Save/BootScriptPrivate.h 1     2/03/11 4:08p Oleksiyy $
//
// $Revision: 1 $
//
// $Date: 2/03/11 4:08p $
//**********************************************************************

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
// Name:	BootScriptPrivate.h
//
// Description:	Boot script auxiliary data structures definitions
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef __BOOT_SCRIPT_PRIVATE__H__
#define __BOOT_SCRIPT_PRIVATE__H__

#include <efi.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TABLE_TYPE1(OpCode)				(OpCode)
#define TABLE_TYPE2(OpCode, Width)		((OpCode) + ((Width) << 8))

#define TABLE_END_OP_CODE				0xff

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_WRITE_IO
//
// Description: Boot script Write IO Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Port                 UINT16
// Count                UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
	UINT16	Type;
	UINT16	Port;
	UINT64	Count;
	//UINT8	Buffer[];
} BOOT_SCRIPT_WRITE_IO;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_READ_WRITE_IO
//
// Description: Boot script Read-Write IO Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Port                 UINT16
// Value                UINT32
// Mask                 UINT32
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
	UINT16	Type;
	UINT16	Port;
	UINT64	Value;
	UINT64	Mask;
} BOOT_SCRIPT_READ_WRITE_IO;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_POLL_IO
//
// Description: Boot script Poll IO Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Port                 UINT16
// Mask                 UINT32
// Result               UINT32
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
    UINT16 Type;
    UINT16 Port;
    UINT64 Mask;
    UINT64 Result;
} BOOT_SCRIPT_POLL_IO;


//////////////////////////////////////////////////////////////////////////
//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_WRITE_MEM
//
// Description: Boot script Write memory Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Address              UINT64
// Value                UINT8
// Count                UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
	UINT16	Type;
	UINT64	Address;
//	UINT8	Value;
	UINT64	Count;
	//UINT8	Buffer[];
} BOOT_SCRIPT_WRITE_MEM;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_READ_WRITE_MEM
//
// Description: Boot script Read-Write memory Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Address              UINT64
// Value                UINT64
// Mask                 UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
	UINT16	Type;
	UINT64	Address;
	UINT64	Value;
	UINT64	Mask;
} BOOT_SCRIPT_READ_WRITE_MEM;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_POLL_MEM
//
// Description: Boot script Poll memory Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Address              UINT64
// Mask                 UINT64
// Result               UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
    UINT16 Type;
    UINT64 Address;
    UINT64 Mask;
    UINT64 Result;
} BOOT_SCRIPT_POLL_MEM;

//////////////////////////////////////////////////////////////////////////
//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_WRITE_PCI
//
// Description: Boot script Write PCI Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Address              UINT64
// Count                UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
	UINT16	Type;
	UINT64	Address;
	UINT64	Count;
	//UINT8	Buffer[];
} BOOT_SCRIPT_WRITE_PCI;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_READ_WRITE_PCI
//
// Description: Boot script Read-Write PCI Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Address              UINT64
// Value                UINT64
// Mask                 UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>
typedef struct {
	UINT16	Type;
	UINT64	Address;
	UINT64	Value;
	UINT64	Mask;
} BOOT_SCRIPT_READ_WRITE_PCI;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_POLL_PCI
//
// Description: Boot script Poll PCI Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Address              UINT64
// Mask                 UINT64
// Result               UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
    UINT16 Type;
    UINT64 Address;
    UINT64 Mask;
    UINT64 Result;
} BOOT_SCRIPT_POLL_PCI;

#pragma pack(push,1)
//////////////////////////////////////////////////////////////////////////
//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_SMBUS_EXECUTE
//
// Description: Boot script SMBUS command Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// SlaveAddress         EFI_SMBUS_DEVICE_ADDRESS
// Command              EFI_SMBUS_DEVICE_COMMAND
// Operation            EFI_SMBUS_OPERATION
// PecCheck             BOOLEAN
// Length               UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
	UINT16						Type;
	EFI_SMBUS_DEVICE_ADDRESS	SlaveAddress;
	EFI_SMBUS_DEVICE_COMMAND	Command;
	EFI_SMBUS_OPERATION			Operation;
	BOOLEAN						PecCheck;
	UINT64						Length;
	//UINT8						Buffer[];
} BOOT_SCRIPT_SMBUS_EXECUTE;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_SMBUS_EXECUTE_32
//
// Description: Boot script SMBUS command Data (32 Bit)
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// SlaveAddress         UINT32
// Command              UINT32
// Operation            EFI_SMBUS_OPERATION
// PecCheck             BOOLEAN
// Length               UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
	UINT16						Type;
	UINT32                      SlaveAddress;
	UINT32	                    Command;
	EFI_SMBUS_OPERATION			Operation;
	BOOLEAN						PecCheck;
	UINT64						Length;
	//UINT8						Buffer[];
} BOOT_SCRIPT_SMBUS_EXECUTE_32;
#pragma pack(pop)

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_STALL
//
// Description: Boot script stall Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Duration             UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
	UINT16	Type;
	UINT64	Duration;
} BOOT_SCRIPT_STALL;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_DISPATCH
//
// Description: Boot script dispatch Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// EntryPoint           EFI_PHYSICAL_ADDRESS
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
	UINT16					Type;
	EFI_PHYSICAL_ADDRESS	EntryPoint;
} BOOT_SCRIPT_DISPATCH;

#if PI_SPECIFICATION_VERSION>=0x0001000A
//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_INFORMATION
//
// Description: Boot script information Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Size                 UINT32
// MessagePtr           EFI_PHYSICAL_ADDRESS	
//----------------------------------------------------------------------------
//<AMI_THDR_END>
typedef struct {
	UINT16	Type;
	UINT32	Size;
	EFI_PHYSICAL_ADDRESS	MessagePtr;
} BOOT_SCRIPT_INFORMATION;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_DISPATCH2
//
// Description: Boot script Dispatch2 Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// EntryPoint           EFI_PHYSICAL_ADDRESS
// Context              EFI_PHYSICAL_ADDRESS
//----------------------------------------------------------------------------
//<AMI_THDR_END>
typedef struct {
	UINT16					Type;
	EFI_PHYSICAL_ADDRESS	EntryPoint;
    EFI_PHYSICAL_ADDRESS	Context;
} BOOT_SCRIPT_DISPATCH2;

//////////////////////////////////////////////////////////////////////////
//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_PCI_CFG2_WRITE
//
// Description: Boot script Write PCI Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Address              UINT64
// Count                UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
	UINT16	Type;
	UINT64	Address;
	UINT64	Count;
    UINT16  Segment;
	//UINT8	Buffer[];
} BOOT_SCRIPT_PCI_CFG2_WRITE;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_PCI_CFG2_READ_WRITE
//
// Description: Boot script Read-Write PCI Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Address              UINT64
// Value                UINT64
// Mask                 UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>
typedef struct {
	UINT16	Type;
	UINT64	Address;
	UINT64	Value;
	UINT64	Mask;
    UINT16  Segment;
} BOOT_SCRIPT_PCI_CFG2_READ_WRITE;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_IO_POLL
//
// Description: Boot script Poll IO Data (PI 1.1)
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Port                 UINT16
// Mask                 UINT32
// Result               UINT32
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
    UINT16 Type;
    UINT16 Port;
    UINT64 Mask;
    UINT64 Result;
    UINT64 Delay;
} BOOT_SCRIPT_IO_POLL;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_MEM_POLL
//
// Description: Boot script Poll memory Data (PI 1.1)
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Address              UINT64
// Mask                 UINT64
// Result               UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
    UINT16 Type;
    UINT64 Address;
    UINT64 Mask;
    UINT64 Result;
    UINT64 Delay;
    UINT64 LoopTimes;
} BOOT_SCRIPT_MEM_POLL;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_PCI_CFG_POLL
//
// Description: Boot script Poll PCI Data (PI 1.1)
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Address              UINT64
// Mask                 UINT64
// Result               UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
    UINT16 Type;
    UINT64 Address;
    UINT64 Mask;
    UINT64 Result;
    UINT64  Delay;
} BOOT_SCRIPT_PCI_CFG_POLL;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_PCI_CFG2_POLL
//
// Description: Boot script Poll PCI Data (PI 1.1)
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Address              UINT64
// Mask                 UINT64
// Result               UINT64
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
    UINT16 Type;
    UINT64 Address;
    UINT64 Mask;
    UINT64 Result;
    UINT64 Delay;
    UINT16 Segment;
} BOOT_SCRIPT_PCI_CFG2_POLL;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_LABEL
//
// Description: Boot script information Data
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// Size                 UINT32
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>
typedef struct {
	UINT16	Type;
	UINT32	Size;
	//UINT8	Buffer[];
} BOOT_SCRIPT_LABEL;

typedef struct {
    UINT32 UniqueIndex;
    UINT32 Length;
} BOOT_SCRIPT_INFO_STRUCTURE;
#endif

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_TABLE_END
//
// Description: Boot script end table marker
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// Type                 UINT16
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef struct {
	UINT16 Type;
} BOOT_SCRIPT_TABLE_END;

//<AMI_THDR_START>
//----------------------------------------------------------------------------
// Name: BOOT_SCRIPT_DATA
//
// Description: Boot script data union
//
// Fields: Name             Type                    Description
//----------------------------------------------------------------------------
// WrIo                 BOOT_SCRIPT_WRITE_IO
// RwIo                 BOOT_SCRIPT_READ_WRITE_IO
// PollIo               BOOT_SCRIPT_POLL_IO
// WrMem                BOOT_SCRIPT_WRITE_MEM
// RwMem                BOOT_SCRIPT_READ_WRITE_MEM
// PollMem              BOOT_SCRIPT_POLL_MEM
// WrPci                BOOT_SCRIPT_WRITE_PCI
// RwPci                BOOT_SCRIPT_READ_WRITE_PCI
// PollPci              BOOT_SCRIPT_POLL_PCI
// ExecSmbus            BOOT_SCRIPT_SMBUS_EXECUTE
// Stall                BOOT_SCRIPT_STALL
// Dispatch             BOOT_SCRIPT_DISPATCH
// End                  BOOT_SCRIPT_TABLE_END
// 
//----------------------------------------------------------------------------
//<AMI_THDR_END>

typedef union {
	BOOT_SCRIPT_WRITE_IO		WrIo;
	BOOT_SCRIPT_READ_WRITE_IO	RwIo;
    BOOT_SCRIPT_POLL_IO         PollIo;
	BOOT_SCRIPT_WRITE_MEM		WrMem;
	BOOT_SCRIPT_READ_WRITE_MEM	RwMem;
    BOOT_SCRIPT_POLL_MEM        PollMem;
	BOOT_SCRIPT_WRITE_PCI		WrPci;
	BOOT_SCRIPT_READ_WRITE_PCI	RwPci;
    BOOT_SCRIPT_POLL_PCI        PollPci;
	BOOT_SCRIPT_SMBUS_EXECUTE	ExecSmbus;
	BOOT_SCRIPT_STALL			Stall;
	BOOT_SCRIPT_DISPATCH		Dispatch;
#if PI_SPECIFICATION_VERSION>=0x0001000A
    BOOT_SCRIPT_INFORMATION         Info;
    BOOT_SCRIPT_DISPATCH2		    Dispatch2;
    BOOT_SCRIPT_PCI_CFG2_WRITE      WrPci2;
    BOOT_SCRIPT_PCI_CFG2_READ_WRITE RwPci2;
    BOOT_SCRIPT_IO_POLL             IoPoll;
    BOOT_SCRIPT_MEM_POLL            MemPoll;
    BOOT_SCRIPT_PCI_CFG_POLL        PciPoll;
    BOOT_SCRIPT_PCI_CFG2_POLL       PciPoll2;
#endif
	BOOT_SCRIPT_TABLE_END		End;	
} BOOT_SCRIPT_DATA;

VOID CallbackReadyToBoot(
	IN EFI_EVENT	Event,
	IN VOID			*Context
);

/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2009, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Pkwy, Norcross, GA 30093               **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
