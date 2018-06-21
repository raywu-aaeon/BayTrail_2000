/** @file
  TPM instance guid, used for PcdTpmInstanceGuid.

Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __TPM_INSTANCE_GUID_H__
#define __TPM_INSTANCE_GUID_H__

#define TPM_DEVICE_INTERFACE_NONE  \
  { 0x00000000, 0x0000, 0x0000, { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } }

#define TPM_DEVICE_INTERFACE_TPM12  \
  { 0x8b01e5b6, 0x4f19, 0x46e8, { 0xab, 0x93, 0x1c, 0x53, 0x67, 0x1b, 0x90, 0xcc } }

#define TPM_DEVICE_INTERFACE_TPM20_DTPM  \
  { 0x286bf25a, 0xc2c3, 0x408c, { 0xb3, 0xb4, 0x25, 0xe6, 0x75, 0x8b, 0x73, 0x17 } }

extern EFI_GUID  gEfiTpmDeviceInstanceNoneGuid;
extern EFI_GUID  gEfiTpmDeviceInstanceTpm12Guid;
extern EFI_GUID  gEfiTpmDeviceInstanceTpm20DtpmGuid;

#endif

