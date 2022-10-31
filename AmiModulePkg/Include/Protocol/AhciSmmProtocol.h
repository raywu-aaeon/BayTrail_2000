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
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/AHCI/AhciSmm/AhciSmmProtocol.h 6     5/05/11 7:39a Rameshr $
//
// $Revision: 6 $
//
// $Date: 5/05/11 7:39a $
//**********************************************************************

//<AMI_FHDR_START>
//--------------------------------------------------------------------------
//
// Name: AHCISmmProtocol.h
//
// Description: Protocol definition
//
//--------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _EFI_AHCI_SMM_PROTOCOLS_H_
#define _EFI_AHCI_SMM_PROTOCOLS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define AHCI_SMM_PROTOCOL_GUID \
        {0xB2FA5764, 0x3B6E, 0x43D3, 0x91, 0xDF, 0x87, 0xD1, 0x5A, 0x3E, 0x56, 0x68}

GUID_VARIABLE_DECLARATION(gAhciSmmProtocolGuid,AHCI_SMM_PROTOCOL_GUID);

#ifndef GUID_VARIABLE_DEFINITION

#include <Protocol\PAhciBus.h>

typedef struct _AHCI_BUS_SMM_PROTOCOL AHCI_BUS_SMM_PROTOCOL;

typedef EFI_STATUS (*EFI_AHCI_SMM_SATA_DEV_PIO_DATA_IN) (
    IN AHCI_BUS_SMM_PROTOCOL                    *SataDevInterface, 
    IN OUT COMMAND_STRUCTURE                    *CommandStructure,
    UINT8                                       PortNumber,
    UINT8                                       PMPortNumber, 
    DEVICE_TYPE                                 DeviceType,
    IN BOOLEAN                                  READWRITE

);

typedef EFI_STATUS (*EFI_AHCI_SMM_SATA_DEV_NON_DATA_CMD) (
    IN AHCI_BUS_SMM_PROTOCOL                    *SataDevInterface, 
    IN COMMAND_STRUCTURE                        CommandStructure,
    UINT8                                       PortNumber,
    UINT8                                       PMPortNumber, 
   	DEVICE_TYPE                                 DeviceType
);

typedef EFI_STATUS (*EFI_AHCI_SMM_SATA_DEV_PACKET_CMD) (
    IN AHCI_BUS_SMM_PROTOCOL                    *SataDevInterface, 
    IN COMMAND_STRUCTURE                        *CommandStructure,
    IN BOOLEAN                                  READWRITE,
    UINT8                                       PortNumber,
    UINT8                                       PMPortNumber, 
   	DEVICE_TYPE                                 DeviceType
);

typedef EFI_STATUS (*EFI_AHCI_SMM_INIT_ON_S3) (
    IN AHCI_BUS_SMM_PROTOCOL                    *SataDevInterface, 
    IN UINT8                                    Port
);

typedef struct _AHCI_BUS_SMM_PROTOCOL{
    UINT64                                      AhciBaseAddress;
    UINT32                                      PortCommandTableBaseAddr;
    UINT32                                      PortCommandListBaseAddr;
    UINT32                                      PortFISBaseAddr;
    UINT8                                       PortNumber;
    UINT8                                       PMPortNumber; 
   	DEVICE_TYPE                                 DeviceType;
	ATAPI_DEVICE			                    AtapiDevice;
    EFI_AHCI_SMM_INIT_ON_S3                     AhciSmmInitPortOnS3Resume;
    EFI_AHCI_SMM_SATA_DEV_PIO_DATA_IN           AhciSmmExecutePioDataCommand;
    EFI_AHCI_SMM_SATA_DEV_NON_DATA_CMD          AhciSmmExecuteNonDataCommand;
    EFI_AHCI_SMM_SATA_DEV_PACKET_CMD            AhciSmmExecutePacketCommand;
}AHCI_BUS_SMM_PROTOCOL;

#endif // #ifndef GUID_VARIABLE_DEFINITION
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
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
