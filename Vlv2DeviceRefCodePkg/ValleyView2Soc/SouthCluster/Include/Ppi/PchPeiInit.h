/*++
  This file contains an 'Intel Peripheral Driver' and uniquely  
  identified as "Intel Reference Module" and is                 
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/

/*++

Copyright (c)  2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  PchPeiInit.h

Abstract:


--*/

#ifndef _PCH_PEI_INIT_H_
#define _PCH_PEI_INIT_H_

//
// Define the PCH PEI Init PPI GUID
//
#define PCH_PEI_INIT_PPI_GUID \
  { \
    0xACB93B08, 0x5CDC, 0x4A8F, 0x93, 0xD4, 0x6, 0xE3, 0x42, 0xDF, 0x18, 0x2E \
  }

//
// Extern the GUID for PPI users.
//
extern EFI_GUID     gPchPeiInitPpiGuid;

#endif
