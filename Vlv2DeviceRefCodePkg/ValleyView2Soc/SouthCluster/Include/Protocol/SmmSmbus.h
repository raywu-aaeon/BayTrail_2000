/*++
  This file contains a 'Sample Driver' and is licensed as such  
  under the terms of your license agreement with Intel or your  
  vendor.  This file may be modified by the user, subject to    
  the additional terms of the license agreement                 
--*/
/*++

Copyright (c)  2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  SmmSmbus.h

Abstract:

  SmmSmbus Protocol

--*/
#ifndef __EFI_SMM_SMBUS_PROTOCOL_H__
#define __EFI_SMM_SMBUS_PROTOCOL_H__

//
// GUID for the SmmSmbus Protocol
//
// EDK and EDKII have different GUID formats
//

#define EFI_SMM_SMBUS_PROTOCOL_GUID  \
  { \
    0x72e40094, 0x2ee1, 0x497a, 0x8f, 0x33, 0x4c, 0x93, 0x4a, 0x9e, 0x9c, 0xc \
  }

//
// Resuse the DXE definition, and use another GUID.
//
typedef EFI_SMBUS_HC_PROTOCOL  SMM_SMBUS_HC_PROTOCOL;

extern EFI_GUID  gEfiSmmSmbusProtocolGuid;

#endif
