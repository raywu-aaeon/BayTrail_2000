/**
  This file contains a 'Sample Driver' and is licensed as such  
  under the terms of your license agreement with Intel or your  
  vendor.  This file may be modified by the user, subject to    
  the additional terms of the license agreement                 
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file 
  PchUsbPolicy.h

  @brief 
  PCH Usb policy PPI produced by a platform driver specifying 
  various expected PCH Usb settings. This PPI is consumed by the 
  PCH PEI drivers.

**/
#ifndef _PCH_USB_POLICY_H_
#define _PCH_USB_POLICY_H_

//
// PCH Usb policy provided by platform for PEI phase
//

#ifndef ECP_FLAG
#include <PiPei.h>
#endif

#include "PchRegs.h"
#include <Protocol/PchPlatformPolicy.h>

#define PCH_USB_POLICY_PPI_GUID \
  { \
    0xc02b0573, 0x2b4e, 0x4a31, 0xa3, 0x1a, 0x94, 0x56, 0x7b, 0x50, 0x44, 0x2c \
  }

extern EFI_GUID                     gPchUsbPolicyPpiGuid;

typedef struct _PCH_USB_POLICY_PPI  PCH_USB_POLICY_PPI;

///
/// PPI revision number
/// Any backwards compatible changes to this PPI will result in an update in the revision number
/// Major changes will require publication of a new PPI
///
/// Revision 1: Original version
///
#define PCH_USB_POLICY_PPI_REVISION_1 1

///
/// Generic definitions for device enabling/disabling used by PCH code.
///
#define PCH_DEVICE_ENABLE   1
#define PCH_DEVICE_DISABLE  0

#define EHCI_MODE           1

struct _PCH_USB_POLICY_PPI {
  UINT8           Revision;
  PCH_USB_CONFIG  *UsbConfig;
  UINT8           Mode;
  UINTN           EhciMemBaseAddr;
  UINT32          EhciMemLength;
  UINTN           XhciMemBaseAddr;
};

#endif
