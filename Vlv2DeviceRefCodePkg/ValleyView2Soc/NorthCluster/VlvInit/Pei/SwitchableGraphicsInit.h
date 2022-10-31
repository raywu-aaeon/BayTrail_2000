/** @file
  Header file for the SwitchableGraphics Pei driver.

@copyright
  Copyright (c) 2010 - 2013 Intel Corporation. All rights reserved
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is licensed for 
  Intel CPUs and chipsets under the terms of your license agreement 
  with Intel or your vendor. This file may be modified by the user, 
  subject to additional terms of the license agreement.

**/

#ifndef _SWITCHABLE_GRAPHICS_PEI_H_
#define _SWITCHABLE_GRAPHICS_PEI_H_

#include "CpuRegs.h"
#include "VlvCommonDefinitions.h"
#include "PlatformBaseAddresses.h"

#include "VlvAccess.h"
#include <Ppi/Stall.h>
#include "VlvInitPeim.h"
#include <PchRegs/PchRegsPcu.h>
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#if defined (SgTpv_SUPPORT) && (SgTpv_SUPPORT == 1)
#include <PlatformGpio.h>        
#include "token.h"               
#endif
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform <<

#ifndef FALSE
#define FALSE 0
#endif

#ifndef HIGH
#define HIGH  1
#endif

#ifndef LOW
#define LOW 0
#endif

#define GP_ENABLE   1
#define GP_DISABLE  0

#ifndef STALL_ONE_MICRO_SECOND
#define STALL_ONE_MICRO_SECOND  1
#endif
#ifndef STALL_ONE_MILLI_SECOND
#define STALL_ONE_MILLI_SECOND  1000
#endif

//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#if defined (SgTpv_SUPPORT) && (SgTpv_SUPPORT == 1)
#define SG_DELAY_HOLD_RST    AMI_SG_DELAY_HOLD_RST * STALL_ONE_MILLI_SECOND
#define SG_DELAY_PWR_ENABLE  AMI_SG_DELAY_PWR_ENABLE * STALL_ONE_MILLI_SECOND
#else
//AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform >>
#define SG_DELAY_HOLD_RST    100 * STALL_ONE_MILLI_SECOND
#define SG_DELAY_PWR_ENABLE  300 * STALL_ONE_MILLI_SECOND
#endif //AMI_OVERRIDE - EIP149863 Switchable Graphic generic support for BYT-DM platform

#endif
