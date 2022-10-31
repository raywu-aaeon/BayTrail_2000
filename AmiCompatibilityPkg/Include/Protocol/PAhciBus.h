//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
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
// $Header: /Alaska/BIN/Core/Include/Protocol/PAhciBus.h 8     11/28/12 7:22a Deepthins $
//
// $Revision: 8 $
//
// $Date: 11/28/12 7:22a $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	AhciBus.h
//
// Description:	
//
//<AMI_FHDR_END>
//**********************************************************************

#ifndef _PAchiBus_
#define _PAchiBus_

#ifdef __cplusplus
extern "C" {
#endif



#define AHCI_BUS_INIT_PROTOCOL_GUID \
        {0xB2FA4764, 0x3B6E, 0x43D3, 0x91, 0xDF, 0x87, 0xD1, 0x5A, 0x3E, 0x56, 0x68}


GUID_VARIABLE_DECLARATION(gAhciBusInitProtocolGuid,AHCI_BUS_INIT_PROTOCOL_GUID);
extern EFI_GUID gAmiAhciBusProtocolGuid;
#define AHCI_PLATFORM_POLICY_PROTOCOL_GUID \
        {0x17706d27, 0x83fe, 0x4770,0x87, 0x5f, 0x4c, 0xef, 0x4c, 0xb8, 0xf6, 0x3d}

GUID_VARIABLE_DECLARATION(gAhciPlatformPolicyProtocolGuid,AHCI_PLATFORM_POLICY_PROTOCOL_GUID);

#ifndef GUID_VARIABLE_DEFINITION
#include <AmiDxeLib.h>

// Forward reference for pure ANSI compatability
typedef	struct _SATA_DEVICE_INTERFACE SATA_DEVICE_INTERFACE;
typedef	struct _AHCI_BUS_PROTOCOL AHCI_BUS_PROTOCOL;

#define		COMMAND_COMPLETE_TIMEOUT	5000					// 5Sec

#pragma pack(1)

typedef struct {
  UINT32   Lowdword;
  UINT32   Upperdword;
} STRUCT_U64_U32;

//typedef enum {	
//  ATA = 0,
//  ATAPI = 1,
//  PMPORT = 2
//} DEVICE_TYPE;

typedef enum {	
  NON_DATA_CMD = 0,
  PIO_DATA_IN_CMD = 1,
  PIO_DATA_OUT_CMD = 2,
  DMA_DATA_IN_CMD = 3,
  DMA_DATA_OUT_CMD = 4,
  PACKET_PIO_DATA_IN_CMD = 5,
  PACKET_PIO_DATA_OUT_CMD = 6,
  PACKET_DMA_DATA_IN_CMD = 7,
  PACKET_DMA_DATA_OUT_CMD = 8,
} COMMAND_TYPE;


typedef struct {
  DLIST   AhciControllerList;
  DLINK   AhciControllerLink;
} AHCI_CONTOLLER_LINKED_LIST;

typedef struct _AHCI_ATAPI_COMMAND{
  	UINT8    		Ahci_Atapi_Command[0x10];
} AHCI_ATAPI_COMMAND;

typedef struct{
	VOID 						*Buffer;
	UINT32						ByteCount;
    UINT8						Features;
    UINT8						FeaturesExp;
    UINT16						SectorCount;
    UINT8						LBALow;
    UINT8						LBALowExp;
    UINT8						LBAMid;
    UINT8						LBAMidExp;
    UINT8						LBAHigh;
    UINT8						LBAHighExp;
    UINT8						Device;
    UINT8						Command;
	UINT8						Control;
    AHCI_ATAPI_COMMAND          AtapiCmd;
}COMMAND_STRUCTURE;

typedef EFI_STATUS (*EFI_SATA_DEV_RAED_WRITE_PIO) (
	IN SATA_DEVICE_INTERFACE                *SataDevInterface,
	IN OUT VOID						        *Buffer,
	IN UINTN						        ByteCount,
	IN UINT64						        LBA,
	IN UINT8						        ReadWriteCommand,
	IN BOOLEAN						        READWRITE
);

typedef EFI_STATUS (*EFI_SATA_DEV_PIO_DATA_IN) (
    IN SATA_DEVICE_INTERFACE                *SataDevInterface, 
    IN OUT COMMAND_STRUCTURE                *CommandStructure,
    IN BOOLEAN                              READWRITE

);

typedef EFI_STATUS (*EFI_SATA_DEV_PIO_DATA_OUT) (
    IN SATA_DEVICE_INTERFACE        *SataDevInterface,
    IN OUT VOID                     *Buffer,
    IN UINTN                        ByteCount,
    IN UINT8                        Features,
    IN UINT8                        LBALow,
    IN UINT8                        LBALowExp,
    IN UINT8                        LBAMid,
    IN UINT8                        LBAMidExp,
    IN UINT8                        LBAHigh,
    IN UINT8                        LBAHighExp,
    IN UINT8                        Command,
    IN BOOLEAN                      READWRITE
);
typedef EFI_STATUS (*EFI_SATA_DEV_NON_DATA_CMD) (
   IN SATA_DEVICE_INTERFACE                 *SataDevInterface, 
    IN COMMAND_STRUCTURE                    CommandStructure
);

typedef EFI_STATUS (*EFI_SATA_DEV_WAIT_FOR_CMD_COMPLETE) (
    IN SATA_DEVICE_INTERFACE                *SataDevInterface,
    IN COMMAND_TYPE                         CommandType,
    IN UINTN                                TimeOut    

);

typedef EFI_STATUS (*EFI_SATA_GENERATE_PORT_RESET) (
    AHCI_BUS_PROTOCOL                   *AhciBusInterface, 
    SATA_DEVICE_INTERFACE               *SataDevInterface, 
    UINT8                               Port,
    UINT8                               PMPort,
    UINT8                               Speed,
    UINT8                               PowerManagement   
);

typedef EFI_STATUS (*EFI_EXECUTE_PACKET_COMMAND) (
    IN SATA_DEVICE_INTERFACE               *SataDevInterface, 
    IN COMMAND_STRUCTURE                   *CommandStructure,
    IN BOOLEAN                             READWRITE
);

typedef struct _AHCI_BUS_PROTOCOL{
	EFI_HANDLE                          ControllerHandle;
    UINT32                              AhciBaseAddress;
    UINT32                              AhciVersion;
    UINT32                              HBACapability;
    UINT32                              HBAPortImplemented;        // Bit Map
    UINT32                              PortCommandListBaseAddr;
    UINT32                              PortCommandListLength;
    UINT32                              PortCommandTableBaseAddr;
    UINT32                              PortCommandTableLength;
    UINT32                              PortFISBaseAddr;
    UINT32                              PortFISBaseAddrEnd;
    DLIST                               SataDeviceList;
	EFI_DEVICE_PATH_PROTOCOL		    *DevicePathProtocol;
    EFI_IDE_CONTROLLER_INIT_PROTOCOL    *IdeControllerInterface;
    EFI_PCI_IO_PROTOCOL					*PciIO;
    UINT32                              Address1;                   // Unmodified PortFISBaseAddr
    UINT32                              Address2;                   // Unmodified PortCommandListBaseAddr
    UINT8                               NumberofPortsImplemented;   // 1 based Count
	BOOLEAN								AHCIRAIDMODE;				// Set to TRUE in AHCI mode, FALSE in RAID mode

    EFI_SATA_DEV_RAED_WRITE_PIO         SataReadWritePio;
    EFI_SATA_DEV_PIO_DATA_IN            ExecutePioDataCommand;
    EFI_SATA_DEV_PIO_DATA_OUT           SataPioDataOut;
    EFI_SATA_DEV_NON_DATA_CMD           ExecuteNonDataCommand;
    EFI_SATA_DEV_WAIT_FOR_CMD_COMPLETE  WaitforCommandComplete;
    EFI_SATA_GENERATE_PORT_RESET        GeneratePortReset;
    EFI_EXECUTE_PACKET_COMMAND          ExecutePacketCommand;

	BOOLEAN								Acoustic_Enable;            // Acoustic Support
	UINT8								Acoustic_Management_Level;  // Acoustic Level
    UINT8                               DiPM;
    UINT16                              PrevPortNum;
    UINT16                              PrevPortMultiplierPortNum;

}AHCI_BUS_PROTOCOL;

typedef struct _SATA_DISK_INFO{ 
	EFI_DISK_INFO_PROTOCOL				DiskInfo;				// should be the first Entry
	SATA_DEVICE_INTERFACE			    *SataDevInterface;
}SATA_DISK_INFO;

typedef struct _SATA_BLOCK_IO{ 
	EFI_BLOCK_IO_PROTOCOL				BlkIo;					// should be the first Entry
	SATA_DEVICE_INTERFACE				*SataDevInterface;
}SATA_BLOCK_IO;

typedef struct _SATA_DEVICE_INTERFACE{
	EFI_HANDLE				            IdeDeviceHandle;
    UINT8                               PortNumber;
    UINT8                               PMPortNumber; 
	UINT8								NumPMPorts;			// Number of Ports in PM, Valid for PMPORT only
    UINT8                               DeviceState;
    UINT32                              Signature;
	UINT32								SControl;
	DEVICE_TYPE				            DeviceType;

	UINT8					            PIOMode;
	UINT8					            SWDma;
	UINT8					            MWDma;
	UINT8					            UDma;
	UINT8					            IORdy;
	UINT8					            ReadCommand;
	UINT8					            WriteCommand;
    IDENTIFY_DATA			            IdentifyData;
	EFI_UNICODE_STRING_TABLE            *UDeviceName;
	ATAPI_DEVICE			            *AtapiDevice;
    UINT8                               AtapiSenseData[256];
    UINT8                               AtapiSenseDataLength;

    UINT32                              PortCommandListBaseAddr;
    UINT32                              PortFISBaseAddr;

    AHCI_BUS_PROTOCOL                   *AhciBusInterface;
	EFI_DEVICE_PATH_PROTOCOL            *DevicePathProtocol; 
    SATA_BLOCK_IO                       *SataBlkIo;
    SATA_DISK_INFO                      *SataDiskInfo;
	SECURITY_PROTOCOL					*IdeSecurityInterface;
	SMART_INTERFACE						*SMARTInterface;
	POWER_MGMT_INTERFACE				*PowerMgmtInterface;

    DLINK                               SataDeviceLink; 
    DLIST                               PMSataDeviceList;         // for devices behind Port Multiplier
    DLINK                               PMSataDeviceLink; 
}SATA_DEVICE_INTERFACE;

typedef struct{
	BOOLEAN         RaidDriverMode;				// Set to TRUE For UEFI Raid driver and FALSE for Legacy Raid option rom 
    BOOLEAN         AhciBusAtapiSupport;        // Set to FALSE For UEFI Raid driver and TRUE for Legacy Raid option rom 
    BOOLEAN         DriverLedOnAtapiEnable;     // Set to TRUE to enable the Drive LED on ATAPI Enable (DLAE) bit
    BOOLEAN         PowerUpInStandbySupport;
    BOOLEAN         PowerUpInStandbyMode;
	BOOLEAN         DipmSupport;                // Set to TRUE to enable the Device initiated power management.
} AHCI_PLATFORM_POLICY_PROTOCOL;

#pragma pack()

#endif // #ifndef GUID_VARIABLE_DEFINITION
/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif

#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
