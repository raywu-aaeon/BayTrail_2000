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
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioBus.h 5     3/07/12 4:21a Rajeshms $
//
// $Revision: 5 $
//
// $Date: 3/07/12 4:21a $
//**********************************************************************

//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    SdioDriver.c
//
// Description: 
//<AMI_FHDR_END>
//**********************************************************************

#ifndef _SDIO_BUS_H
#define _SDIO_BUS_H

#ifdef __cplusplus
extern "C" {
#endif


//#define SDIO_BUS_INIT_PROTOCOL_GUID \
//        {0x94c69847, 0xa0cf, 0x4635, 0xaa, 0x23, 0xd2, 0x66, 0x7b, 0xd7, 0xf7, 0x91}

//GUID_VARIABLE_DECLARATION(gSdioBusInitProtocolGuid,SDIO_BUS_INIT_PROTOCOL_GUID);
extern EFI_GUID gSdioBusInitProtocolGuid;

#ifndef GUID_VARIABLE_DEFINITION
#include <AmiDxeLib.h>

#define PCI_CL_SYSTEM_PERIPHERALS		    0x08
#define PCI_CL_SYSTEM_PERIPHERALS_SCL_SD	0x05

#define     SDIO_MANUFACTUREID_LENGTH   30

typedef	struct _SDIO_DEVICE_INTERFACE SDIO_DEVICE_INTERFACE;

typedef VOID (EFIAPI *EFI_SDIO_INVOKE_API) (
		VOID*          fURP    );

typedef struct{
	EFI_HANDLE                          ControllerHandle;
    UINT64                              SdioBaseAddress;
    UINT32                              TransferBufferAddress;
    DLIST                               SdioDeviceList;
	EFI_DEVICE_PATH_PROTOCOL		    *DevicePathProtocol;
    EFI_PCI_IO_PROTOCOL					*PciIO;
    EFI_SDIO_INVOKE_API	      			SdioInvokeApi;
}SDIO_BUS_PROTOCOL;

typedef struct _Sdio_DISK_INFO{ 
	EFI_DISK_INFO_PROTOCOL				DiskInfo;				// should be the first Entry
	SDIO_DEVICE_INTERFACE			    *SdioDevInterface;
}SDIO_DISK_INFO;

typedef struct _Sdio_BLOCK_IO{ 
	EFI_BLOCK_IO_PROTOCOL				BlkIo;					// should be the first Entry
	SDIO_DEVICE_INTERFACE				*SdioDevInterface;
}SDIO_BLOCK_IO;

typedef struct _SDIO_DEVICE_INTERFACE{
	EFI_HANDLE				            SdioDeviceHandle;
    UINT8                               DeviceAddress;
    BOOLEAN                             MassStorageDevice;
    UINT8                               PortNumber;
    UINT8                               DeviceState;
    UINT8                               bMode;
    UINT8                               bState;
    UINT8                               bActive;
    UINT32                              dOCR;
    UINT32                              d4CID[4];
    UINT16                              wRCA;
    UINT32                              d4CSD[4];
    BOOLEAN                             bWrite_Bl_Partial;
    UINT8                               bWrite_Bl_Len;
    BOOLEAN                             bRead_Bl_Partial;
    UINT8                               bRead_Bl_Len;
    UINT8                               d2SCR[8];

    UINT8                               NumHeads;
    UINT8                               LBANumHeads;
    UINT16                              NumCylinders;
    UINT16                              LBANumCyls;
    UINT8                               NumSectors;
    UINT8                               LBANumSectors;
    UINT32                              dMaxLBA;
    UINT16                              wBlockSize;
    UINT8                               StorageType;
    UINT8                               PNM[27];
    CHAR16                              UnicodePNMString[27];
    UINT8                               SdIOManufactureId[SDIO_MANUFACTUREID_LENGTH];
    
    SDIO_BUS_PROTOCOL                   *SdioBusInterface;
    EFI_DEVICE_PATH_PROTOCOL            *DevicePathProtocol; 
    SDIO_BLOCK_IO                       *SdioBlkIo;
    SDIO_DISK_INFO                      *SdioDiskInfo;
    DLINK                               SdioDeviceLink; 

}SDIO_DEVICE_INTERFACE;

#endif // #ifndef GUID_VARIABLE_DEFINITION

/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif

#endif 

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2012, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
