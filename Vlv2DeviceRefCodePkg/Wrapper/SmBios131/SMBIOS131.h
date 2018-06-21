/*++

Copyright (c) 2004-2006 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  TDTPlatformPolicy.h
  
Abstract:

  TDTPlatformPolicy to check and set TDT Platform Policy.
  
--*/
/*++
 This file contains an 'Intel Peripheral Driver' and is        
 licensed for Intel CPUs and chipsets under the terms of your  
 license agreement with Intel or your vendor.  This file may   
 be modified by the user, subject to additional terms of the   
 license agreement                                             
--*/

#ifndef _SMBIOS_131_H_
#define _SMBIOS_131_H_


#include "Uefi.h"
#include <Protocol/TdtPlatformPolicy.h>
#include <Protocol/Smbios.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/SeCLib.h>


#pragma pack(1)
typedef struct {
  UINT8     Type;
  UINT8     Length;
  UINT16    Handle;
  UINT32    Reserved;
  UINT64    Reserved1;
  // offset 0x10
  UINT64    Reserved2;
  UINT32    TxeCapability[3];
  UINT32    TxeConfigState;
  UINT32    Reserved3[3];
  UINT32    BIOSSecurityCap;
  UINT32    StructIdentifier;
  UINT32    Reserved4;
} SMBIOS_131_STRUCT;
#pragma pack()

#endif

