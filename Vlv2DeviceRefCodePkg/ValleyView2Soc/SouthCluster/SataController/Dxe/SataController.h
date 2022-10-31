/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++
Copyright (c) 1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    SataController.h

Abstract:

    Header file for chipset Serial ATA controller driver.

--*/
#ifndef _SERIAL_ATA_CONTROLLER_H_
#define _SERIAL_ATA_CONTROLLER_H_

#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include "pci22.h"
#else
#include <IndustryStandard/Pci22.h>
#endif
#include <PchAccess.h>
#ifdef ECP_FLAG
#include <Protocol/IdeControllerInit/IdeControllerInit.h>
#include <Protocol/PciIo/PciIo.h>
#else
#include <Protocol/IdeControllerInit.h>
#include <Protocol/PciIo.h>
#endif
#include <Library/PchPlatformLib.h>
#ifndef ECP_FLAG
#include <Protocol/DriverBinding.h>
#include <Protocol/ComponentName2.h>
#endif

//
// Global Variables definitions
//
extern EFI_DRIVER_BINDING_PROTOCOL  mSataControllerDriverBinding;
#ifndef ECP_FLAG
extern EFI_COMPONENT_NAME2_PROTOCOL  mSataControllerName;
#endif
extern UINTN                        mPchGpioBase;

#define PCH_SATA_ENUMER_ALL   FALSE

#define PCH_SATA_MASTER_DRIVE 0x00
#define PCH_SATA_SLAVE_DRIVE  0x01

//
// SATA controller driver private data structure
//
#define SATA_CONTROLLER_SIGNATURE SIGNATURE_32 ('S', 'A', 'T', 'A')

typedef struct _EFI_SATA_CONTROLLER_PRIVATE_DATA {
  //
  // Standard signature used to identify SATA controller private data
  //
  UINT32                            Signature;

  //
  // Protocol instance of IDE_CONTROLLER_INIT produced by this driver
  //
  EFI_IDE_CONTROLLER_INIT_PROTOCOL  IdeInit;

  //
  // copy of protocol pointers used by this driver
  //
  EFI_PCI_IO_PROTOCOL               *PciIo;

  //
  // The highest disqulified mode for each attached SATA device.
  // Per ATA/ATAPI spec, if a mode is not supported, the modes higher than
  // it should not be supported
  //
  EFI_ATA_COLLECTIVE_MODE           DisqulifiedModes[PCH_AHCI_MAX_PORTS][PCH_SATA_MAX_DEVICES];

  //
  // A copy of IDENTIFY data for each attached SATA device and its flag
  //
  EFI_IDENTIFY_DATA                 IdentifyData[PCH_AHCI_MAX_PORTS][PCH_SATA_MAX_DEVICES];
  BOOLEAN                           IdentifyValid[PCH_AHCI_MAX_PORTS][PCH_SATA_MAX_DEVICES];
} EFI_SATA_CONTROLLER_PRIVATE_DATA;

#define SATA_CONTROLLER_PRIVATE_DATA_FROM_THIS(a) \
  CR ( \
  a, \
  EFI_SATA_CONTROLLER_PRIVATE_DATA, \
  IdeInit, \
  SATA_CONTROLLER_SIGNATURE \
  )

//
// Driver binding functions declaration
//
EFI_STATUS
EFIAPI
SataControllerSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL       *This,
  IN EFI_HANDLE                        Controller,
  IN EFI_DEVICE_PATH_PROTOCOL          *RemainingDevicePath
  )
/*++

Routine Description:

  This function checks to see if the driver supports a device specified by
  "Controller handle" parameter. It is called by DXE Core StartImage() or
  ConnectController() routines. The driver uses 'device path' and/or
  'services' from the Bus I/O abstraction attached to the controller handle
  to determine if the driver support this controller handle.

  Note: In the BDS (Boot Device Selection) phase, the DXE core enumerate all
  devices (or, controller) and assigns GUIDs to them.

Arguments:

  This                    A pointer points to the Binding Protocol instance
  Controller              The handle of controller to be tested.
  RemainingDevicePath     A pointer to the device path. Ignored by device
                          driver but used by bus driver

Returns:
  EFI_SUCCESS             The device is supported
  EFI_UNSUPPORTED         The device is not supported

--*/
;

EFI_STATUS
EFIAPI
SataControllerStart (
  IN EFI_DRIVER_BINDING_PROTOCOL        *This,
  IN EFI_HANDLE                         Controller,
  IN EFI_DEVICE_PATH_PROTOCOL           *RemainingDevicePath
  )
/*++

Routine Description:

  This routine is called right after the .Supported() is called and returns
  EFI_SUCCESS. Notes: The supported protocols are checked but the Protocols
  are closed.

Arguments:

  This                    A pointer points to the Binding Protocol instance
  Controller              The handle of controller to be tested. Parameter
                          passed by the caller
  RemainingDevicePath     A pointer to the device path. Should be ignored by
                          device driver

Returns:
  EFI_SUCCESS             The device is started
  Other values            Something error happened

--*/
;

EFI_STATUS
EFIAPI
SataControllerStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL       *This,
  IN  EFI_HANDLE                        Controller,
  IN  UINTN                             NumberOfChildren,
  IN  EFI_HANDLE                        *ChildHandleBuffer
  )
/*++

Routine Description:

  Stop managing the target device

Arguments:

  This                    A pointer pointing to the Binding Protocol instance
  Controller              The handle of controller to be stopped
  NumberOfChildren        Number of child devices
  ChildHandleBuffer       Buffer holding child device handles

Returns:

  EFI_SUCCESS             The target device is stopped

--*/
;

//
// IDE controller init functions declaration
//
EFI_STATUS
IdeInitGetChannelInfo (
  IN   EFI_IDE_CONTROLLER_INIT_PROTOCOL    *This,
  IN   UINT8                               Channel,
  OUT  BOOLEAN                             *Enabled,
  OUT  UINT8                               *MaxDevices
  )
/*++

Routine Description:

  This function can be used to obtain information about a specified channel.
  It's usually used by IDE Bus driver during enumeration process.

Arguments:

  This                    the EFI_IDE_CONTROLLER_INIT_PROTOCOL instance.
  Channel                 Channel number (0 based, either 0 or 1)
  Enabled                 TRUE if the channel is enabled. If the channel is disabled,
                          then it will no be enumerated.
  MaxDevices              The Max number of IDE devices that the bus driver can expect
                          on this channel. For ATA/ATAPI, this number is either 1 or 2.

Returns:
  EFI_SUCCESS             Function completes successfully
  Other Values            Something error happened
  EFI_INVALID_PARAMETER   The Channel parameter is invalid

--*/
;

EFI_STATUS
IdeInitNotifyPhase (
  IN  EFI_IDE_CONTROLLER_INIT_PROTOCOL  *This,
  IN  EFI_IDE_CONTROLLER_ENUM_PHASE     Phase,
  OUT UINT8                             Channel
  )
/*++

Routine Description:

  This function is called by IdeBus driver before executing certain actions.
  This allows IDE Controller Init to prepare for each action.

Arguments:

  This                    the EFI_IDE_CONTROLLER_INIT_PROTOCOL instance.
  Phase                   phase indicator defined by IDE_CONTROLLER_INIT protocol
  Channel                 Channel number (0 based, either 0 or 1)

Returns:
  EFI_SUCCESS             Function completes successfully
  EFI_INVALID_PARAMETER   Channel parameter is out of range
  EFI_UNSUPPORTED         Phase is not supported

--*/
;

EFI_STATUS
IdeInitSubmitData (
  IN  EFI_IDE_CONTROLLER_INIT_PROTOCOL  *This,
  IN  UINT8                             Channel,
  IN  UINT8                             Device,
  IN  EFI_IDENTIFY_DATA                 *IdentifyData
  )
/*++

Routine Description:

  This function is called by IdeBus driver to submit EFI_IDENTIFY_DATA data structure
  obtained from IDE deivce. This structure is used to set IDE timing

Arguments:

  This                    the EFI_IDE_CONTROLLER_INIT_PROTOCOL instance.
  Channel                 IDE channel number (0 based, either 0 or 1)
  Device                  IDE device number
  IdentifyData            A pointer to EFI_IDENTIFY_DATA data structure

Returns:

  EFI_SUCCESS             Function completes successfully
  EFI_INVALID_PARAMETER   Channel or Device parameter is out of range

--*/
;

EFI_STATUS
IdeInitDisqualifyMode (
  IN  EFI_IDE_CONTROLLER_INIT_PROTOCOL  *This,
  IN  UINT8                             Channel,
  IN  UINT8                             Device,
  IN  EFI_ATA_COLLECTIVE_MODE           *BadModes
  )
/*++

Routine Description:

  This function is called by IdeBus driver to disqualify unsupported operation
  mode on specfic IDE device

Arguments:

  This                    The EFI_IDE_CONTROLLER_INIT_PROTOCOL instance.
  Channel                 IDE channel number (0 based, either 0 or 1)
  Device                  IDE device number
  BadModes                Operation mode indicator

Returns:

  EFI_SUCCESS             Function completes successfully
  EFI_INVALID_PARAMETER   Channel parameter or Devicde parameter is out of range;
                          or BadModes is NULL

--*/
;

EFI_STATUS
IdeInitCalculateMode (
  IN  EFI_IDE_CONTROLLER_INIT_PROTOCOL  *This,
  IN  UINT8                             Channel,
  IN  UINT8                             Device,
  IN  EFI_ATA_COLLECTIVE_MODE           **SupportedModes
  )
/*++

Routine Description:

  This function is called by IdeBus driver to calculate the best operation mode
  supported by specific IDE device

Arguments:

  This                    The EFI_IDE_CONTROLLER_INIT_PROTOCOL instance.
  Channel                 IDE channel number (0 based, either 0 or 1)
  Device                  IDE device number
  SupportedModes          Modes collection supported by IDE device

Returns:

  EFI_SUCCESS             Function completes successfully
  EFI_INVALID_PARAMETER   Channel parameter or Device parameter is out of range;
                          Or SupportedModes is NULL
  EFI_NOT_READY           Identify data is not valid
  EFI_OUT_OF_RESOURCES    SupportedModes is out of range

--*/
;

EFI_STATUS
IdeInitSetTiming (
  IN  EFI_IDE_CONTROLLER_INIT_PROTOCOL  *This,
  IN  UINT8                             Channel,
  IN  UINT8                             Device,
  IN  EFI_ATA_COLLECTIVE_MODE           *Modes
  )
/*++

Routine Description:

  This function is called by IdeBus driver to set appropriate timing on IDE
  controller according supported operation mode

Arguments:

  This                    The EFI_IDE_CONTROLLER_INIT_PROTOCOL instance.
  Channel                 IDE channel number (0 based, either 0 or 1)
  Device                  IDE device number
  Modes                   Operation modes

Returns:
  EFI_SUCCESS             This function always returns EFI_SUCCESS

--*/
;

#endif
