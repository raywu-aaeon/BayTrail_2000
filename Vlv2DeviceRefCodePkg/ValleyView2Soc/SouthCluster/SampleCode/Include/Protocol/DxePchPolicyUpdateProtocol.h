/*++
  This file contains a 'Sample Driver' and is licensed as such  
  under the terms of your license agreement with Intel or your  
  vendor.  This file may be modified by the user, subject to    
  the additional terms of the license agreement                 
--*/
/*++

Copyright (c)  1999 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  DxePchPolicyUpdateProtocol.h

Abstract:

  PCH policy update protocol. This protocol is consumed by the PchDxePolicyInit driver

--*/
#ifndef _DXE_PCH_POLICY_UPDATE_PROTOCOL_H_
#define _DXE_PCH_POLICY_UPDATE_PROTOCOL_H_

#include "PchRegs.h"


#ifdef ECP_FLAG
#define DXE_PCH_POLICY_UPDATE_PROTOCOL_GUID \
  { \
    0x1a819e49, 0xd8ee, 0x48cb, 0x9a, 0x9c, 0xa, 0xa0, 0xd2, 0x81, 0xa, 0x38 \
  }
#else
#define DXE_PCH_POLICY_UPDATE_PROTOCOL_GUID \
  { \
    0x1a819e49, 0xd8ee, 0x48cb, \
    { \
        0x9a, 0x9c, 0xa, 0xa0, 0xd2, 0x81, 0xa, 0x38 \
    } \
  }
#endif

extern EFI_GUID                                   gDxePchPolicyUpdateProtocolGuid;
#define DXE_PCH_POLICY_UPDATE_PROTOCOL_REVISION_1 1

//
// ------------ General PCH policy Update protocol definition ------------
//
struct _DXE_PCH_POLICY_UPDATE_PROTOCOL {
  UINT8                   Revision;
};

typedef struct _DXE_PCH_POLICY_UPDATE_PROTOCOL  DXE_PCH_POLICY_UPDATE_PROTOCOL;

#endif
