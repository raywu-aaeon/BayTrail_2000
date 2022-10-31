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
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioDef.h 5     3/07/12 4:07a Rajeshms $
//
// $Revision: 5 $
//
// $Date: 3/07/12 4:07a $
//**********************************************************************

//<AMI_FHDR_START>
//--------------------------------------------------------------------------
//
// Name:SdioDef.h 		
//
// Description: Header file for Smm and Non Smm interface
//
//--------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _EFI_SDIO_DEF_H_
#define _EFI_SDIO_DEF_H_

#define SDIO_DATA_EBDA_OFFSET 0x104

#define SDIO_API_DEVICE_DETECT  0
#define SDIO_API_DEVICE_CONFIGURE  1
#define SDIO_API_GET_MASS_DEVICE_DETAILS 2
#define SDIO_API_READ           3
#define SDIO_API_WRITE          4
#define SDIO_API_GET_DEVICE_GEOMETRY	5
#define SDIO_API_RESET_DEVICE 	6

//----------------------------------------------------------------------------
//      SDIO Mass Storage Related Data Structures and Equates
//----------------------------------------------------------------------------
#define SDIO_EMU_NONE            0
#define SDIO_EMU_FLOPPY_ONLY     1
#define SDIO_EMU_HDD_ONLY        2
#define SDIO_EMU_FORCED_FDD      3

// Error returned from API handler
#define     SDIO_SUCCESS                0x000
#define     SDIO_PARAMETER_ERROR        0x010
#define     SDIO_NOT_SUPPORTED          0x020
#define     SDIO_INVALID_FUNCTION       0x0F0
#define     SDIO_ERROR                  0x0FF

#define     SDIO_MANUFACTUREID_LENGTH   30

#pragma pack(1)
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        CONTROLLER_INFO
//
// Description: This is a URP (SDIO Request Packet) structure for the BIOS
//      API call Controller Info
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT32      TransferBufferAddress;
    UINT64      SdioBaseAddress;
    UINT8       PciBus;
    UINT8       PciDevice;
    UINT8       PciFunc;
    UINT8       Port;
    BOOLEAN     DeviceDetected;
    UINT8       DeviceAddress;
    UINT8       NumHeads;
    UINT8       LBANumHeads;
    UINT16      NumCylinders;
    UINT16      LBANumCyls;
    UINT8       NumSectors;
    UINT8       LBANumSectors;
    UINT32      MaxLBA;
    UINT16      BlockSize;
    UINT8       StorageType;
    UINT8       PNM[27];
    BOOLEAN     SdIODevice;
    UINT8       SdIOManufactureId[SDIO_MANUFACTUREID_LENGTH];
} CONTROLLER_INFO;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        RESET_SDIO
//
// Description: This is a URP (SDIO Request Packet) structure for the BIOS
//      API call Reset SDIO
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       DeviceAddress;
} RESET_SDIO;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        READ_DATA
//
// Description: This is a URP (SDIO Request Packet) structure for the BIOS
//      API call Read 
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8       DeviceAddress;
    UINT8       Port;
    UINT32      LBA;            // Starting LBA address
    UINT16      NumBlks;        // Number of blocks to read
    UINT32      *BufferAddress;  // Far buffer pointer
} READ_DATA;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        DEVICE_GEO
//
// Description: This is a URP (SDIO Request Packet) structure for the BIOS
//      API call Device Geometry 
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT8       DeviceAddress;
    UINT8       NumHeads;
    UINT16      NumCylinders;
    UINT8       NumSectors;
    UINT8       LBANumHeads;
    UINT16      LBANumCyls;
    UINT8       LBANumSectors;
    UINT16      BlockSize;
    UINT32      MaxLBA;
    UINT8       Int13FunctionNo;
} DEVICE_GEO;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        API_DATA
//
// Description: This is a union data type of all the API related data
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef union {
    RESET_SDIO              Reset;
    CONTROLLER_INFO         ControllerInfo;
    READ_DATA               Read;    
    DEVICE_GEO              DeviceGeo;
} SDIO_API_DATA;


//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        URP_STRUC
//
// Description: This structure is the URP structure
//
// Fields:   Name       Type        Description
//      ------------------------------------------------------------
//      bFuncNumber UINT8       Function number of the URP
//      bSubFunc    UINT8       Sub-func number of the URP
//      bRetValue   UINT8       Return value
//      ApiData     API_DATA    Refer structure definition
//----------------------------------------------------------------------------
//<AMI_SHDR_END>

typedef struct {
    UINT8           bFuncNumber;
    UINT8           bSubFunc;
    UINT8           bRetValue;
    SDIO_API_DATA   ApiData;
} SDIO_STRUC;

#pragma pack()

// Defining a GUID for SDIO_SMM_NON_SMM_ADDRESS_PROTOCOL
#define SDIO_SMM_NON_SMM_ADDRESS_PROTOCOL_GUID \
{0x17d6d323, 0x43ce, 0x438a, 0xbc, 0x95, 0x78, 0xa2, 0xde, 0x99, 0x19, 0xd7}

//
// Interface stucture for the SDIO SMM NON-SMM ADDRESS Protocol
//
typedef struct _SDIO_SMM_NON_SMM_ADDRESS_PROTOCOL{
    UINTN     Address;
}SDIO_SMM_NON_SMM_ADDRESS_PROTOCOL;

#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             6145-F Northbelt Pkwy, Norcross, GA 30071            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
