/*++
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
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

  fTPM.h

Abstract:

  Interface definition details for fTPM.

--*/
#ifndef _SEC_FTPM_PPI_H_
#define _SEC_FTPM_PPI_H_

#define SEC_FTPM_PPI_GUID \
  { \
    0x4fd1ba49, 0x8f90, 0x471a, 0xa2, 0xc9, 0x17, 0x3c, 0x7a, 0x73, 0x2f, 0xd0 \
  }

extern EFI_GUID  gSeCfTPMPolicyPpiGuid;

//
// PPI definition
//
typedef struct SEC_FTPM_POLICY_PPI {
  BOOLEAN                 fTPMEnable;
} SEC_FTPM_POLICY_PPI;

#endif
