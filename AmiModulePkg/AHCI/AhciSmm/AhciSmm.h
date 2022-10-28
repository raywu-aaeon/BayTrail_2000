//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

/** @file AhciSmm.h
    Ahci Smm function definition

**/
#ifndef _AMI_AHCI_SMM_H_
#define _AMI_AHCI_SMM_H_

#ifndef ATAPI_BUSY_CLEAR_TIMEOUT
#define     ATAPI_BUSY_CLEAR_TIMEOUT            16000           // 16sec
#endif

#ifndef DMA_ATA_COMMAND_COMPLETE_TIMEOUT
#define     DMA_ATA_COMMAND_COMPLETE_TIMEOUT    5000            // 5Sec
#endif

#ifndef DMA_ATAPI_COMMAND_COMPLETE_TIMEOUT
#define     DMA_ATAPI_COMMAND_COMPLETE_TIMEOUT  16000           // 16Sec
#endif

#define     COMMAND_LIST_SIZE_PORT              0x800
#define     TIMEOUT_1SEC                        1000            // 1sec Serial ATA 1.0 Sec 5.2

#define PCI_CFG_ADDR(bus,dev,func,reg) \
    ((VOID*)(UINTN) (PciExpressBaseAddress + ((bus) << 20) + ((dev) << 15) + ((func) << 12) + reg))

EFI_STATUS
AhciSmmExecuteNonDataCommand (
    AMI_AHCI_BUS_SMM_PROTOCOL    *SataDevInterface, 
    COMMAND_STRUCTURE            CommandStructure,
    UINT8                        PortNumber,
    UINT8                        PMPortNumber, 
    DEVICE_TYPE                  DeviceType
);

EFI_STATUS
AhciSmmExecutePioDataCommand (
    AMI_AHCI_BUS_SMM_PROTOCOL    *SataDevInterface, 
    COMMAND_STRUCTURE            *CommandStructure,
    UINT8                        PortNumber,
    UINT8                        PMPortNumber, 
    DEVICE_TYPE                  DeviceType,
    BOOLEAN                      READWRITE 
);

EFI_STATUS
AhciSmmInitPortOnS3Resume(
    AMI_AHCI_BUS_SMM_PROTOCOL    *SataDevInterface, 
    UINT8                        Port
);

EFI_STATUS
StartController (
    IN AMI_AHCI_BUS_SMM_PROTOCOL    *SataDevInterface,
    IN UINT32                       CIBitMask
);

EFI_STATUS
ReadWritePMPort (
    IN AMI_AHCI_BUS_SMM_PROTOCOL    *SataDevInterface,
    IN UINT8                        Port,
    IN UINT8                        RegNum,
    IN OUT UINT32                   *Data,
    IN BOOLEAN                      READWRITE
);

EFI_STATUS 
AhciSmmExecutePacketCommand (
    IN AMI_AHCI_BUS_SMM_PROTOCOL    *SataDevInterface, 
    IN COMMAND_STRUCTURE            *CommandStructure,
    IN BOOLEAN                      READWRITE,
    UINT8                           PortNumber,
    UINT8                           PMPortNumber, 
    DEVICE_TYPE                     DeviceType
 );

UINT64 Shr64( IN UINT64 Value, IN UINT8 Shift );
UINT64 Shl64( IN UINT64 Value, IN UINT8 Shift );

#if INDEX_DATA_PORT_ACCESS
UINT32
ReadDataDword (
    IN  UINTN   BaseAddr,
    IN  UINTN   Index
);

UINT16
ReadDataWord (
    IN  UINTN   BaseAddr,
    IN  UINTN   Index
);

UINT8
ReadDataByte (
    IN  UINTN   BaseAddr,
    IN  UINTN   Index
);

VOID
WriteDataDword (
    IN  UINTN   BaseAddr,
    IN  UINTN   Index, 
    IN  UINTN   Data
);

VOID
WriteDataWord (
    IN  UINTN   BaseAddr,
    IN  UINTN   Index, 
    IN  UINTN   Data
);

VOID
WriteDataByte (
    IN  UINTN   BaseAddr,
    IN  UINTN   Index,
    IN  UINTN   Data
);
#endif

#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************