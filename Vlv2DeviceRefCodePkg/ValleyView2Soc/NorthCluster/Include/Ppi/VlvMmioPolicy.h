/*++
  This file contains an 'Intel Peripheral Driver' and uniquely  
  identified as "Intel Reference Module" and is                 
  licensed for Intel CPUs and chipsets under the terms of your  
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the   
  license agreement                                             
--*/

/*++

Copyright (c)  2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  VlvMmioPolicy.h

Abstract:

  Interface definition details between ValleyView platform drivers during PEI phase.

--*/

#ifndef _VLV_MMIO_POLICY_PPI_H_
#define _VLV_MMIO_POLICY_PPI_H_

#define VLV_MMIO_POLICY_PPI_GUID \
  { \
    0xE767BF7F, 0x4DB6, 0x5B34, 0x10, 0x11, 0x4F, 0xBE, 0x4C, 0xA7, 0xAF, 0xD2 \
  }

extern EFI_GUID gVlvMmioPolicyPpiGuid;


//
// MRC Platform Policiy PPI
//
typedef struct _VLV_MMIO_POLICY_PPI {
  UINT16                 MmioSize;
} VLV_MMIO_POLICY_PPI;

#pragma pack()

#endif // _VLV_MMIO_POLICY_PPI_H_
