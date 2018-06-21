/*++
  This file contains an 'Intel Peripheral Driver' and uniquely  
  identified as "Intel Reference Module" and is                 
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
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

  VlvPeiInit.h

Abstract:

  Interface definition between ValleyView MRC and VlvInitPeim driver..

--*/

#ifndef _VLV_PEI_INIT_H_
#define _VLV_PEI_INIT_H_

//
// Define the VLV PEI Init PPI GUID
//
#define VLV_PEI_INIT_PPI_GUID \
  { \
    0x9ea8911, 0xbe0d, 0x4230, 0xa0, 0x3, 0xed, 0xc6, 0x93, 0xb4, 0x8e, 0x11 \
  }

//
// Extern the GUID for PPI users.
//
extern EFI_GUID     gVlvPeiInitPpiGuid;

#endif
