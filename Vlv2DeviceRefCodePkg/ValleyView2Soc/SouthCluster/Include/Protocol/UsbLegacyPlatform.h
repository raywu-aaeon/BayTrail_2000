/*++

Copyright (c)  2004 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  UsbLegacyPlatform.h

Abstract:

  Protocol used to get the platform policy on USB legacy operation.

--*/

#ifndef _USB_LEGACY_PLATFORM_H_
#define _USB_LEGACY_PLATFORM_H_

EFI_FORWARD_DECLARATION(EFI_USB_LEGACY_PLATFORM_PROTOCOL);


#define EFI_USB_LEGACY_PLATFORM_PROTOCOL_GUID \
  {0x13f40f6e, 0x50c1, 0x4b73, 0xb1, 0xe2, 0x6e, 0x72, 0xd2, 0x1c, 0xb0, 0x4a}

typedef struct {
  UINT16            ShiftKey     :1;
  UINT16            AltKey       :1;
  UINT16            CtrlKey      :1;
  UINT16            Reserved     :13;
} KEY_ATTRIBUTE;

typedef struct {
  UINT8            ScanCode;
  KEY_ATTRIBUTE    Keyattribute;
} KEY_ELEMENT;

// UsbLegacyEnable
//    0b = Disabled
//    1b = Enabled
// UsbBoot
//    00b = Not supported
//    01b = Disabled
//    10b = Enabled
//    11b = Reserved
// UsbZip
//    00b = Not supported
//    01b = Disabled
//    10b = Enabled
//    11b = Reserved
// Usb Zip Emulation
//    00b = Floppy
//    01b = HDD
//    10b = Reserved
//    11b = Reserved
// UsbFixedDiskWpBootSector
//    00b = Not supported
//    01b = Disabled
//    10b = Enabled
//    11b = Reserved
// Usb Ehci Enable
//     0b = Disable
//     1b = Enable
// Usb Mass Device Emulation
//    00b = Use default
//    01b = Floppy
//    10b = Hard Disk
//    11b = Size-based emulation
typedef struct {
  UINT32            UsbLegacyEnable          :1;
  UINT32            UsbBoot                  :2;
  UINT32            UsbZip                   :2;
  UINT32            UsbZipEmulation          :2;
  UINT32            UsbFixedDiskWpBootSector :2;
//UsbLegacyPlatform.165_001-- UINT32            Reserved                     :23;
  UINT32            UsbEhciEnable            :1;   //UsbLegacyPlatform.165_001++
  UINT32            UsbMassEmulation         :2;
  UINT32            Reserved                 :20;  //UsbLegacyPlatform.165_001++
  UINT32            UsbMassEmulationSizeLimit;
} USB_LEGACY_MODIFIERS;

// Here is the protocol
typedef
EFI_STATUS
(EFIAPI *EFI_GET_USB_PLATFORM_OPTIONS) (
  IN   EFI_USB_LEGACY_PLATFORM_PROTOCOL *This,
  OUT  USB_LEGACY_MODIFIERS           *UsbLegacyModifiers
  )
/*++

  Routine Description:
    Get SETUP/platform options for USB Legacy operation modification.

  Arguments:
    This                - Protocol instance pointer.
    UsbLegacyModifiers  - List of keys to monitor from. This includes both
                          PS2 and USB keyboards.

  Returns:
    EFI_SUCCESS   - Modifiers exist.
    EFI_NOT_FOUND - Modifiers not not exist.

--*/
;

typedef
EFI_STATUS
(EFIAPI *EFI_GET_PLATFORM_MONITOR_KEY_OPTIONS) (
  IN   EFI_USB_LEGACY_PLATFORM_PROTOCOL *This,
  OUT  KEY_ELEMENT                    **KeyList,
  OUT  UINTN                          *KeyListSize
  )
/*++

  Routine Description:
    Return a list of keys to monitor for.

  Arguments:
    This          - Protocol instance pointer.
    KeyList       - List of keys to monitor from. This includes both
                    USB & PS2 keyboard inputs.
    KeyListSize   - Size of KeyList in bytes

  Returns:
    EFI_SUCCESS   - Keys are to be monitored.
    EFI_NOT_FOUND - No keys are to be monitored.

--*/
;

typedef struct _EFI_USB_LEGACY_PLATFORM_PROTOCOL {
  EFI_GET_USB_PLATFORM_OPTIONS           GetUsbPlatformOptions;
  EFI_GET_PLATFORM_MONITOR_KEY_OPTIONS   GetPlatformMonitorKeyOptions;
} EFI_USB_LEGACY_PLATFORM_PROTOCOL;

extern EFI_GUID gEfiUsbLegacyPlatformProtocolGuid;

#endif
