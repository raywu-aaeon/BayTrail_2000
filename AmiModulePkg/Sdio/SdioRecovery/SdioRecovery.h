//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioRecovery.h 1     7/18/12 4:49a Rajeshms $
//
// $Revision: 1 $
//
// $Date: 7/18/12 4:49a $
//
//*************************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/SdioDriver/SdioRecovery.h $
// 
// 1     7/18/12 4:49a Rajeshms
// [TAG]  		EIP93345
// [Category]  	New Feature
// [Description]  	Create a PEI driver for Boot Block recovery from SD/MMC
// devices
// [Files]  		SdioRecovery.cif
// SdioRecovery.sdl
// SdioRecovery.mak
// SdioRecovery.c
// SdioFindRecoveryDevice.c
// SdioRecovery.h
// 
// 1     7/18/12 4:30a Rajeshms
// [TAG]  		EIP93345 
// [Category]  	New Feature
// [Description]  	Create a PEI driver for Boot Block recovery from SD/MMC
// devices
// [Files]  		Board\EM\SdioRecovery\SdioRecovery.cif
// Board\EM\SdioRecovery\SdioRecovery.sdl
// Board\EM\SdioRecovery\SdioRecovery.mak
// Board\EM\SdioRecovery\SdioRecovery.c
// Board\EM\SdioRecovery\SdioFindRecoveryDevice.c
// Board\EM\SdioRecovery\SdioRecovery.h
// 
//*************************************************************************
//<AMI_FHDR_START>
//
//  Name:           SdioRecovery.h
//
//  Description:    Private Strunctures and Function Declarations, inclusion 
//                  of some header files  are Present in this file.
//
//<AMI_FHDR_END>
//*************************************************************************

#include <PEI.h>
#include <AmiPeiLib.h> 
#include <AmiLib.h> 
#include <Ppi/DeviceRecoveryBlockIo.h>
#include <Protocol\BlockIo.h>
#include <Pci.h>
#include <Ppi/Stall.h>
#include "SdioController.h"
#include "token.h"

#define CMD_ENABLE_IO   0x05
#define CMD_ENABLE_MEM  0x06
#define MAX_SUBORDINATE_NUMBER 10
#define BAR_ADDRESS_MASK 0xFFFFFFFF
#define INVALID_VENDOR_ID 0xFFFF
#define MASK_IO_DECODE_RANGE  0xFFFFFFFE
#define MASK_MEM_DECODE_RANGE 0xFFFFFFF0
#define FIRST_SECONDARY_BUS_NUMBER 1
#define MASK_MEM_BUS_MASTER  0xF9
#define MASK_IO_BUS_MASTER  0xFA

#define SD_BLOCK_SIZE  512
#define SIZE_FOR_DMA_TRANFER 8192   // 8kb


EFI_STATUS
EnumerateBus(
    IN UINT8 Bus
);

EFI_STATUS
CheckforProgrammedBridgeorDevice (
    IN UINT8 Bus,
    IN UINT8 Device,
    IN UINT8 Function,
    IN UINT8 SecondaryBusNo
);

EFI_STATUS
ProgramPciBridge (
    IN UINT8 Bus,
    IN UINT8 Device,
    IN UINT8 Function,
    IN UINT16 Address,
    IN BOOLEAN IsMmio
);

VOID
ProgramSubordinateForBridgeAbove(
    IN UINT8   PrimaryBusNo,
    IN UINT8   SubordinateBusNo
);

EFI_STATUS
EFIAPI NotifyOnRecoveryCapsuleLoaded (
    IN EFI_PEI_SERVICES          **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR *NotifyDesc,
    IN VOID                      *InvokePpi
);

#pragma pack(1)
typedef struct {
    SDIO_DEVICE_INFO SdioDeviceInfo;
    EFI_PEI_BLOCK_IO_MEDIA MediaInfo;
    BOOLEAN     LookedForMedia;
} SDIO_RECOVERY_DEVICE_INFO;

typedef struct {
    EFI_PEI_RECOVERY_BLOCK_IO_PPI RecoveryBlkIo;
    BOOLEAN                  HaveEnumeratedDevices;
    UINTN                    DeviceCount;
    SDIO_RECOVERY_DEVICE_INFO *DeviceInfo[MAX_SDIO_RECOVERY_DEVICE];
} SDIO_RECOVERY_BLOCK_IO_DEV;

typedef struct {
    UINT8   BusNumber;
    UINT8   Device;
    UINT8   Function;
    BOOLEAN IsMmioDevice;
    UINT32  BaseAddress;
} PCI_DEVICE_INFO;


typedef struct {
    UINT8   PrimaryBusNumber;
    UINT8   Device;
    UINT8   Function;
    UINT8   IsMMIO;
    UINT16  MemIOBaseLimit;
} PCI_PROGRAMMED_BRIDGE_INFO;

typedef struct {
    UINT8   PrimaryBusNumber;
    UINT8   SecBusNumber;
    UINT8   SubBusNumber;
    UINT8   Device;
    UINT8   Function;
} PCI_BRIDGE_INFO;

typedef struct {
    UINT8   Bus;
    UINT8   Dev;
    UINT8   Func;
} ROOT_BRIDGE;

#pragma pack()

EFI_STATUS
EnumerateSdDevices( 
    SDIO_RECOVERY_BLOCK_IO_DEV *Sd_BlkIoDev
);


//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
