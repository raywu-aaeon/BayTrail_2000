//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioDriver.h 2     2/24/11 11:36p Rameshr $
//
// $Revision: 2 $
//
// $Date: 2/24/11 11:36p $
//**********************************************************************

//<AMI_FHDR_START>
//--------------------------------------------------------------------------
//
// Name: SdioDriver.h
//
// Description: Header file for the SIO
//
//--------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _EFI_SDIO_DRIVER_H_
#define _EFI_SDIO_DRIVER_H_

#include <Efi.h>
#include <Token.h>
#include <Dxe.h>
#include <PCI.h>
#include <AmiDxeLib.h>
#include <Protocol\PciIo.h>
#include <Protocol\DevicePath.h>
#include <Protocol\LegacyBios.h>
#include <protocol\DriverBinding.h>
#include <Protocol\ComponentName.h>
#include <protocol\BlockIo.h>
#include <Protocol\PDiskInfo.h>
#include <protocol\SdioBus.h>
#include "SdioDef.h"


#define     ZeroMemory(Buffer,Size) pBS->SetMem(Buffer,Size,0)

#define     DEVICE_DISABLED                     0
#define     DEVICE_IN_RESET_STATE               1
#define     DEVICE_DETECTION_FAILED             2
#define     DEVICE_DETECTED_SUCCESSFULLY        3
#define     DEVICE_CONFIGURED_SUCCESSFULLY      4
#define     DEVICE_REMOVED                      5
#define     BLKIO_REVISION                      1



// Driver Binding Protocol functions

EFI_STATUS
SdioDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
SdioDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  );

EFI_STATUS
SdioDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   Controller,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  );


EFI_STATUS
InstallSdioBusProtocol (
    IN EFI_HANDLE                       Controller,
    IN OUT SDIO_BUS_PROTOCOL            *SdioBusInterface,
    IN EFI_PCI_IO_PROTOCOL              *PciIO
 );

EFI_STATUS
SdioInitController (
    IN OUT SDIO_BUS_PROTOCOL            *SdioBusInterface
);


EFI_STATUS
CreateSdioDevicePath (
    IN EFI_DRIVER_BINDING_PROTOCOL      *This,
    IN EFI_HANDLE                       Controller,
    IN SDIO_DEVICE_INTERFACE            *SdioDevInterface,
    IN OUT EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
);

EFI_STATUS
InitSdioBlockIO (
    IN SDIO_DEVICE_INTERFACE            *SdioDevInterface
);

EFI_STATUS
InitSdioDiskInfo (
    IN SDIO_DEVICE_INTERFACE            *SdioDevInterface
);

SDIO_DEVICE_INTERFACE *
GetSdioDevInterface(
    IN SDIO_BUS_PROTOCOL                   *SdioBusInterface,
    IN UINT8                               Port
);

EFI_STATUS
DetectAndConfigureDevice (
    IN EFI_DRIVER_BINDING_PROTOCOL      *This,
    IN EFI_HANDLE                       Controller,
    IN EFI_DEVICE_PATH_PROTOCOL         *RemainingDevicePath,
    SDIO_BUS_PROTOCOL                   *SdioBusInterface,
    UINT8                               PortNo
);

EFI_STATUS
SdioDetectDevice (
    IN SDIO_BUS_PROTOCOL                   *SdioBusInterface,
    IN UINT8                               Port
);


EFI_STATUS
SdioBlkRead(
    IN EFI_BLOCK_IO_PROTOCOL        *This,
    IN UINT32                       MediaId,
    IN EFI_LBA                      LBA,
    IN UINTN                        BufferSize,
    OUT VOID                        *Buffer
);

EFI_STATUS
SdioBlkWrite(
    IN EFI_BLOCK_IO_PROTOCOL        *This,
    IN UINT32                       MediaId,
    IN EFI_LBA                      LBA,
    IN UINTN                        BufferSize,
    OUT VOID                        *Buffer
);

EFI_STATUS
SdioAtaBlkReadWrite (
    IN EFI_BLOCK_IO_PROTOCOL        *This,
    IN UINT32                       MediaId,
    IN EFI_LBA                      LBA,
    IN UINTN                        BufferSize,
    OUT VOID                        *Buffer,
    BOOLEAN                         READWRITE
);

EFI_STATUS
SdioReset (
    IN EFI_BLOCK_IO_PROTOCOL        *This,
    IN BOOLEAN                      ExtendedVerification
);

EFI_STATUS
SdioBlkFlush(
    IN EFI_BLOCK_IO_PROTOCOL        *This
);

EFI_STATUS
CheckDevicePresence (
    IN SDIO_DEVICE_INTERFACE                *SdioBusInterface,
    IN UINT8                                Port
);

EFI_STATUS
SDIOAPI_ReadCard (
    IN SDIO_DEVICE_INTERFACE               *SdioDevInterface,
    IN UINT8                                Port,
    IN EFI_LBA                              LBA,
    IN UINT32                               NumBlks,
    IN VOID                                 *BufferAddress
);


EFI_STATUS
SDIOAPI_WriteCard (
    IN SDIO_DEVICE_INTERFACE               *SdioDevInterface,
    IN UINT8                                Port,
    IN EFI_LBA                              LBA,
    IN UINT32                               NumBlks,
    IN VOID                                 *BufferAddress
);

VOID
SDIOGenerateSWSMI (
    UINT8               Data,
    SDIO_STRUC          *Parameters
);

VOID
SdioHandler (
	IN SDIO_STRUC          *sURP );

EFI_STATUS SdioCtlDriverName(
    IN EFI_COMPONENT_NAME2_PROTOCOL  *This,
    IN CHAR8                        *Language,
    OUT CHAR16                      **DriverName
);

EFI_STATUS
SdioCtlGetControllerName(
    IN EFI_COMPONENT_NAME2_PROTOCOL  *This,
    IN EFI_HANDLE                   ControllerHandle,
    IN EFI_HANDLE                   ChildHandle        OPTIONAL,
    IN CHAR8                        *Language,
    OUT CHAR16                      **ControllerName
);


#endif

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
