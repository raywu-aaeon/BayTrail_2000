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
// $Header: /Alaska/SOURCE/Modules/HddSecurity/IdeSecurity.h 12    10/11/11 1:54a Rameshr $
//
// $Revision: 12 $
//
// $Date: 10/11/11 1:54a $
//**********************************************************************
//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:    <IdeSecurity.h>
//
// Description: This file contains the Includes, Definitions, typedefs,
//              Variable and External Declarations, Structure and
//              function prototypes needed for the IdeSecurity driver
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _IdeSecurity_
#define _IdeSecurity_

#ifdef __cplusplus
extern "C" {
#endif

#include <Efi.h>
#include <Token.h>
#include <Dxe.h>
#include <AmiDxeLib.h>
#include "Protocol\PciIo.h"
#include "Protocol\DevicePath.h"
#include "protocol\DriverBinding.h"
#include "protocol\BlockIo.h"
#include "Protocol\PDiskInfo.h"
#include "Protocol\PIDEController.h"
#include "Protocol\PIDEBus.h"
#include "Protocol\PAhciBus.h"
#include <Protocol\ComponentName.h>
#include <Protocol\BootScriptSave.h>
#include <Protocol\SmmBase.h>
#if defined(PI_SPECIFICATION_VERSION)&&(PI_SPECIFICATION_VERSION>=0x0001000A)
#include <Protocol\SmmControl2.h>
#else
#include <Protocol\SmmControl.h>
#endif
#if ( defined(AHCI_SUPPORT) && (AHCI_SUPPORT != 0) )
#include <Protocol\AhciSmmProtocol.h>
#endif
#include <Protocol\SmmCommunication.h>

#define IDE_SECURITY_PWNV_GUID \
{ 0x69967a8c, 0x1159, 0x4522, 0xaa, 0x89, 0x74, 0xcd, 0xc6, 0xe5, 0x99, 0xa0}


#define EFI_SMM_SAVE_HDD_PASSWORD_GUID \
{ 0xeedcf975, 0x4dd3, 0x4d94, 0x96, 0xff, 0xaa, 0xca, 0x83, 0x53, 0xb8, 0x7b }

#define EFI_SMM_REMOVE_HDD_PASSWORD_GUID \
{ 0xc2b1e795, 0xf9c5, 0x4829, 0x8a, 0x42, 0xc0, 0xb3, 0xfe, 0x57, 0x15, 0x17 }

// Size of SMM communicate header, without including the Data.
#define SMM_COMMUNICATE_HEADER_SIZE  (OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data))

// Size of HDD Password DATA size
#define HDD_PASSWORD_SIZE  sizeof(HDD_PASSWORD)

EFI_GUID gSaveHddPasswordGuid=EFI_SMM_SAVE_HDD_PASSWORD_GUID;
EFI_GUID gRemoveHddPasswordGuid=EFI_SMM_REMOVE_HDD_PASSWORD_GUID;
EFI_GUID gEfiSmmCommunicationProtocolGuid=EFI_SMM_COMMUNICATION_PROTOCOL_GUID;


extern EFI_GUID                    gIdeSecurityInterfaceGuid;


#define HDD_PWD_ENCRYPTION_KEY      "H?p1mA*k920_84o3d^!z@L.x4$kY64"

#define EFI_SEGMENT( _Adr )     (UINT16) ((UINT16) (((UINTN) (_Adr))\
                                                    >> 4) & 0xf000)
#define EFI_OFFSET( _Adr )      (UINT16) (((UINT16) ((UINTN) (_Adr))) & 0xffff)


//###DEBUG  Uncomment the following for Required Debug Level.

//#define   TRACE_IDESMM TRACE

//###DEBUG END

#define     TRACE_IDESMM

#define     ZeroMemory( Buffer, Size ) pBS->SetMem( Buffer, Size, 0 )

// Forward reference for pure ANSI compatability
typedef struct _IDE_BUS_PROTOCOL IDE_BUS_PROTOCOL;

HDD_SECURITY_INIT_PROTOCOL *gHddSecurityInitProtocol;

#pragma pack(1)

typedef struct
{
    UINT8  Bus;
    UINT8  Device;
    UINT8  Function;
    UINT8  Controller;
    UINT32 Reserved;
} EDD_PCI;

typedef struct
{
    UINT16 Base;
    UINT16 Reserved;
    UINT32 Reserved2;
} EDD_LEGACY;

typedef union
{
    EDD_PCI    Pci;
    EDD_LEGACY Legacy;
} EDD_INTERFACE_PATH;

typedef struct
{
    UINT8 Master;
    UINT8 Reserved[15];
} EDD_ATA;

typedef struct
{
    UINT8 Master;
    UINT8 Lun;
    UINT8 Reserved[14];
} EDD_ATAPI;

typedef struct
{
    UINT16 TargetId;
    UINT64 Lun;
    UINT8  Reserved[6];
} EDD_SCSI;

typedef struct
{
    UINT64 SerialNumber;
    UINT64 Reserved;
} EDD_USB;

typedef struct
{
    UINT64 Guid;
    UINT64 Reserved;
} EDD_1394;

typedef struct
{
    UINT64 Wwn;
    UINT64 Lun;
} EDD_FIBRE;

typedef struct
{
    UINT8 bPortNum;
    UINT8 Reserved[15];
} EDD_SATA;

typedef union
{
    EDD_ATA   Ata;
    EDD_ATAPI Atapi;
    EDD_SCSI  Scsi;
    EDD_USB   Usb;
    EDD_1394  FireWire;
    EDD_FIBRE FibreChannel;
    EDD_SATA  Sata;
} EDD_DEVICE_PATH;

typedef struct
{
    UINT16             StructureSize;
    UINT16             Flags;
    UINT32             MaxCylinders;
    UINT32             MaxHeads;
    UINT32             SectorsPerTrack;
    UINT64             PhysicalSectors;
    UINT16             BytesPerSector;
    UINT32             FDPT;
    UINT16             Key;
    UINT8              DevicePathLength;
    UINT8              Reserved1;
    UINT16             Reserved2;
    CHAR8              HostBusType[4];
    CHAR8              InterfaceType[8];
    EDD_INTERFACE_PATH InterfacePath;
    EDD_DEVICE_PATH    DevicePath;
    UINT8              Reserved3;
    UINT8              Checksum;
} EDD_DRIVE_PARAMETERS;

typedef struct _HDDSECDATA
{
    UINT16 UserMaster;
    UINT32 PasswordLength;
    UINT8  HddUserPassword[IDE_PASSWORD_LENGTH];
    UINT8  HddMasterPassword[IDE_PASSWORD_LENGTH];
} HDDSECDATA;

#pragma pack()



EFI_STATUS InstallSecurityInterface (
    IN VOID    *BusInterface,
    IN BOOLEAN ModeFlag );

EFI_STATUS StopSecurityModeSupport (
    IN VOID    *BusInterface,
    IN BOOLEAN ModeFlag );

EFI_STATUS ReturnSecurityStatus (
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   *SecurityStatus );

EFI_STATUS SecuritySetPassword (
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   Control,
    UINT8                    *Buffer,
    UINT16                   RevisionCode );

EFI_STATUS SecurityUnlockPassword (
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   Control,
    UINT8                    *Buffer );

EFI_STATUS SecurityDisablePassword (
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   Control,
    UINT8                    *Buffer );

EFI_STATUS SetDefaultMasterPassword (
    IN IDE_SECURITY_PROTOCOL *This );

EFI_STATUS SecuritySetDefaultMasterPassword (
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   Control,
    UINT8                    *Buffer,
    UINT16                   RevisionCode );

EFI_STATUS SecurityFreezeLock (
    IN IDE_SECURITY_PROTOCOL *This );

EFI_STATUS SecurityEraseUnit (
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   Control,
    UINT8                    *Buffer );

EFI_STATUS ReturnIdePasswordFlags (
    IN IDE_SECURITY_PROTOCOL *This,
    UINT32                   *IdePasswordFlags );

EFI_STATUS SecurityCommonRoutine (
    IN VOID *IdeBusInterface,
    UINT16  Control,
    UINT8   *Buffer,
    UINT8   SecurityCommand,
    UINT16  RevisionCode,
    BOOLEAN ModeFlag );

EFI_STATUS ConnectController (
    IN EFI_HANDLE                                   ControllerHandle,
    IN EFI_HANDLE*DriverImageHandle                 OPTIONAL,
    IN EFI_DEVICE_PATH_PROTOCOL*RemainingDevicePath OPTIONAL,
    IN BOOLEAN                                      Recursive );

EFI_STATUS GatherIdeInfo (
    IN VOID    *BusInterface,
    IN BOOLEAN ModeFlag,
    OUT UINT32 *HddDataId );

EFI_STATUS UpdateIdentifyData (
    IN VOID    *BusInterface,
    IN BOOLEAN ModeFlag );

EFI_STATUS SetHddSecDataInNvram (
    IN VOID *BusInterface,
    UINT32  *HddDataId,
    UINT16  Control,
    UINT8   *Buffer );

EFI_STATUS TransferPwDataFromNvramToSmram (
    IN IDE_SECURITY_PROTOCOL *This );

VOID ConvertHddDataIdToString (
    IN UINT32  DataId,
    OUT CHAR16 *String );

VOID EncodeDecodePassword (
    IN UINT8  *InputString,
    OUT UINT8 *OutputString,
    IN UINT32 StringLength );

EFI_STATUS OEMSetMasterPassword (
    IN IDE_SECURITY_PROTOCOL *This );

EFI_STATUS SmmHDDPasswordInterface (
    IN IDE_SECURITY_PROTOCOL *This,
    UINT16                   Control,
    UINT8                    *Buffer,
    UINT8                    Action );

VOID IdeBusCallbackBootScript (
    IN EFI_EVENT Event,
    IN VOID      *Context );

VOID IdeBusMiscSmmFeatureCallback (
    IN EFI_EVENT Event,
    IN VOID      *Context );

BOOLEAN CheckAhciMode (
    IN IDE_BUS_PROTOCOL *IdeBusInterface );

UINTN EfiValueToString (
    IN OUT CHAR16 *Buffer,
    IN INT64      Value,
    IN UINTN      Flags,
    IN UINTN      Width );

EFI_STATUS CommonNonDataHook (
    IN VOID              *BusInterface,
    IN COMMAND_STRUCTURE CommandStructure,
    IN BOOLEAN           ModeFlag );

EFI_STATUS CommonPioDataHook (
    IN VOID              *BusInterface,
    IN COMMAND_STRUCTURE CommandStructure,
    IN BOOLEAN           ModeFlag );

EFI_STATUS CommonReadWritePioHook (
    IN VOID     *BusInterface,
    IN OUT VOID *Buffer,
    IN UINTN    ByteCount,
    IN UINT64   LBA,
    IN UINT8    ReadWriteCommand,
    IN BOOLEAN  ReadWrite,
    IN BOOLEAN  ModeFlag );

EFI_STATUS CommonWfccHook (
    IN VOID    *BusInterface,
    IN BOOLEAN ModeFlag );

VOID LocateAhciSmmServiceEvent (
    EFI_EVENT Event,
    VOID      *Context );



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
