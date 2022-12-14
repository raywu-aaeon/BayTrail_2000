/** @file
  Define the variable data structures used for TrEE physical presence.
  The TPM2 request from firmware or OS is saved to variable. And it is
  cleared after it is processed in the next boot cycle. The TPM2 response 
  is saved to variable.

Copyright (c) 2012, Intel Corporation. All rights reserved. <BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __TREE_PHYSICAL_PRESENCE_DATA_GUID_H__
#define __TREE_PHYSICAL_PRESENCE_DATA_GUID_H__

#define EFI_TREE_PHYSICAL_PRESENCE_DATA_GUID \
  { \
    0xf24643c2, 0xc622, 0x494e, { 0x8a, 0xd, 0x46, 0x32, 0x57, 0x9c, 0x2d, 0x5b }\
  }

#define TREE_PHYSICAL_PRESENCE_VARIABLE  L"TrEEPhysicalPresence"

typedef struct {
  UINT8   PPRequest;      ///< Physical Presence request command.
  UINT8   LastPPRequest;
  UINT32  PPResponse;
  UINT8   Flags;
} EFI_TREE_PHYSICAL_PRESENCE;

//
// The definition bit of the flags
//
#define TREE_FLAG_NO_PPI_CLEAR                        BIT1
#define TREE_FLAG_RESET_TRACK                         BIT3

//
// The definition of physical presence operation actions
//
#define TREE_PHYSICAL_PRESENCE_NO_ACTION                               0
#define TREE_PHYSICAL_PRESENCE_CLEAR_CONTROL_CLEAR                     5
#define TREE_PHYSICAL_PRESENCE_CLEAR_CONTROL_CLEAR_2                   14
#define TREE_PHYSICAL_PRESENCE_SET_NO_PPI_CLEAR_FALSE                  17
#define TREE_PHYSICAL_PRESENCE_SET_NO_PPI_CLEAR_TRUE                   18
#define TREE_PHYSICAL_PRESENCE_CLEAR_CONTROL_CLEAR_3                   21
#define TREE_PHYSICAL_PRESENCE_CLEAR_CONTROL_CLEAR_4                   22

#define TREE_PHYSICAL_PRESENCE_NO_ACTION_MAX                           20

extern EFI_GUID  gEfiTrEEPhysicalPresenceGuid;

#endif

