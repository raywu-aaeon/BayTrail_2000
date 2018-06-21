/*++
  This file contains an 'Intel Peripheral Driver' and uniquely  
  identified as "Intel Reference Module" and is                 
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/

/*++

Copyright (c)  1999 - 2012 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  PnPDxe.h

Abstract:

  Power and Performance header file

--*/

#ifndef _PNP_DXE_H_
#define _PNP_DXE_H_

#include <IndustryStandard/Pci22.h>
#include <PchRegs.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PchPlatformLib.h>
#include "VlvAccess.h"
#include <Setup.h>    // AMI_OVERRIDE
#include <Library/UefiRuntimeServicesTableLib.h>
#include <SetupMode.h>    // AMI_OVERRIDE
//#if defined (BYTI_PF_ENABLE) && (BYTI_PF_ENABLE == 1)
#include <Guid/PlatformInfo.h>
//#endif

typedef struct {
  UINT8   MsgPort;
  UINT32  MsgRegAddr;
  UINT8   MSB;
  UINT8   LSB;
  UINT32  Value;
} PNP_SETTING;

typedef struct {
  UINT32   MsgPort;
  UINT32  MsgReadOpcode;
  UINT32  MsgWriteOpcode;
  UINT32  BarIndex;
  UINT32  Device;
  UINT32  Function;
  UINT32  MsgRegAddr;
  UINT8   MSB;
  UINT8   LSB;
  UINT32  Value;
} PNP_SETTING_EX;

#define POWER_PERFORMANCE_SETTING_ENABLED_AUTO_DETECT         3
#define POWER_PERFORMANCE_SETTING_ENABLED_AX_STEPPING         4
#define POWER_PERFORMANCE_SETTING_ENABLED_BX_STEPPING         5
#endif
