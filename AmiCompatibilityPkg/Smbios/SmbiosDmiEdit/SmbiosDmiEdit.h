//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//

//**********************************************************************//
// $Header: /Alaska/SOURCE/Modules/SMBIOS/SmbiosDMIEditSupport/SmbiosDMIEdit.h 14    6/12/12 11:28a Davidd $
//
// $Revision: 14 $
//
// $Date: 6/12/12 11:28a $
//**********************************************************************//
//**********************************************************************//

#ifndef _SmbiosDMIEdit_DRIVER_H
#define _SmbiosDMIEdit_DRIVER_H

#include <Efi.h>
#include <Token.h>

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2
#include <Protocol/smiflash.h>
#endif

extern EFI_BOOT_SERVICES		*pBS;

UINT16 GetSmbiosInfo();
UINT16 GetSmbiosStructure();
UINT16 SetSmbiosStructure();

#pragma pack(1)

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SMBIOS_TABLE_ENTRY
//
// Description: SMBIOS Entry Point structure
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT32    AnchorString;
    UINT8     EntryPointStructChecksum;
    UINT8     EntryPointLength;
    UINT8     MajorVersion;
    UINT8     MinorVersion;
    UINT16    MaximumStructSize;
    UINT8     EntryPointRevision;
    UINT8     FormattedArea[5];
    UINT8     IntermediateAnchor[5];
    UINT8     IntermediateChecksum;
    UINT16    StructTableLength;
    UINT32    StructTableAddress;
    UINT16    NumStructs;
    UINT8     SmbiosBCDRevision;
} SMBIOS_TABLE_ENTRY;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  DMI_STRUC
//
// Description: Structure Header
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT8		Type;
    UINT8		Length;
    UINT16		Handle;
} DMI_STRUC;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SET_SMBIOS_STRUCTURE_DATA
//
// Description: Set SMBIOS Structure Data
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT8     Command;
    UINT8     FieldOffset;
    UINT32    ChangeMask;
    UINT32    ChangeValue;
    UINT16    DataLength;
    DMI_STRUC StructureHeader;
    UINT8     StructureData[1];
} SET_SMBIOS_STRUCTURE_DATA;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  GET_SMBIOS_INFO
//
// Description: Get SMBIOS Information
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT16		Function;
    UINT32		DmiBiosRevision32BitAddr;
    UINT32		NumStructures32BitAddr;
    UINT32		StructureSize32BitAddr;
    UINT32		DmiStorageBase32BitAddr;
    UINT32		DmiStorageSize32BitAddr;
    UINT16		BiosSelector; //Always 0.
/////////////////////////////////////
// The above pointers point below. //
/////////////////////////////////////
    UINT32		DmiBiosRevision;
    UINT32		NumStructures;
    UINT32		StructureSize;
    UINT32		pDmiStorageBase;
    UINT32		DmiStorageSize;
} GET_SMBIOS_INFO;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  GET_SMBIOS_STRUCTURE
//
// Description: Get SMBIOS Structure
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT16		Function;
    UINT32		Handle32BitAddr;
    UINT32		Buffer32BitAddr;
    UINT16		DmiSelector;  //Always 0
    UINT16		BiosSelector; //Always 0
/////////////////////////////////////
// The above pointers point below. //
/////////////////////////////////////
    UINT16		Handle;
    UINT8		Buffer[1];    //Variable Length;
} GET_SMBIOS_STRUCTURE;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  SET_SMBIOS_STRUCTURE
//
// Description: Set SMBIOS Structure
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT16		Function;
    UINT32		Buffer32BitAddr;
    UINT32		DmiWorkBuffer32BitAddr;
    UINT8		Control;    //?
    UINT16		DmiSelector;  //Always 0
    UINT16		BiosSelector; //Always 0
/////////////////////////////////////
// The above pointers point below. //
/////////////////////////////////////
    SET_SMBIOS_STRUCTURE_DATA StructureData;    //Variable Length;
} SET_SMBIOS_STRUCTURE;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  TABLE_INFO
//
// Description: DMI data record
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

#define DMIEDIT_WRITE_ONCE      0x01
#define DMIEDIT_DELETE_STRUC    0x02
#define DMIEDIT_ADD_STRUC       0x04
#define DMIEDIT_EXTENDED_HDR    0x80

typedef struct {
    UINT8   Type;
    UINT8   Offset;     // Structure field offset, or string number for Type 11 and 12
    UINT8   Reserved;   // Size of string including \0 or UUID (16)
    UINT8   Flags;      // Bit0 = Write Once
                        // Bit1 = Delete Structure
                        // Bit2 = Add structure
                        // Bit7 = Extended Header
    UINT8   HdrLength;
    UINT16  Size;
    UINT16  Handle;
} TABLE_INFO;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  WRITE_ONCE_TABLE
//
// Description: Write Once structure
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT8   Type;
    UINT8   Offset;
  	BOOLEAN WriteOnce;
} WRITE_ONCE_TABLE;

typedef struct {
    UINT8   Type;
    UINT8   Offset;
    UINT16  Handle;
} WRITE_ONCE_STATUS;

#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC != 2   // FV_BB or FV_MAIN

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:  FLASH_DATA_INFO
//
// Description: Flash Data Information
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
	UINT8		*Location;
	UINT16		Size;
	UINT8		*EndOfData;
} FLASH_DATA_INFO;

extern UINT8  					*gBlockSave;
extern VOID   					*gFlashData;
extern UINT32 					gFlashDataSize;
extern EFI_SMI_FLASH_PROTOCOL   *mSmiFlash;

#endif                                  // SMBIOS_DMIEDIT_DATA_LOC

#if (defined(NonSmiDmiEdit_Support) && (NonSmiDmiEdit_Support == 1))
#define AMI_DMIEDIT_SMBIOS_GUID \
    { 0x74211cd7, 0x3d8e, 0x496f, { 0xba, 0x2, 0x91, 0x9c, 0x2e, 0x1f, 0x6, 0xcb } }

typedef struct _EFI_SMBIOS_DMIEDIT_PROTOCOL EFI_SMBIOS_DMIEDIT_PROTOCOL;

typedef UINT32  (EFIAPI *DMIEDIT_NONSMI_HANDLER) (
    IN UINT8                    Data,
    IN UINT64                   pCommBuff
);

typedef struct _EFI_SMBIOS_DMIEDIT_PROTOCOL {
    DMIEDIT_NONSMI_HANDLER      DmiEditNonSmiHandler;
};

UINT32 DmiEditNonSmiHandler(
    IN UINT8            Data,
    IN UINT64           pCommBuff
);
#endif                                  // NonSmiDmiEdit_Support

#pragma pack()

#endif

//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//
