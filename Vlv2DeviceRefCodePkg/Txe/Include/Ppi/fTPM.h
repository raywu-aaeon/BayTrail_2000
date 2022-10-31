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
    0x10e26df1, 0x8775, 0x4ee1, 0xb5, 0x0a, 0x3a, 0xe8, 0x28, 0x93, 0x70, 0x3a \
  }

extern EFI_GUID  gSeCfTPMPpiGuid;

//
// Revision
//
#define SEC_FTPM_PPI_REVISION 1

//
// define the MRC recommended boot modes.
//
typedef enum {
  s3Boot_, // In current implementation, bmS3 == bmWarm
  warmBoot_,
  coldBoot_,
  fastBoot_,
} MRC_BOOT_MODE_;

typedef
UINT32
(EFIAPI *SEC_SEND_FTPM_SIZE) (
  IN EFI_PEI_SERVICES **PeiServices
  );

typedef
EFI_STATUS
(EFIAPI *SEC_CONFIG_FTPM) (
  IN EFI_PEI_SERVICES **PeiServices,
  UINT8                 MrcBootMode,
  UINT8                 InitStat,
  UINT32                SeCfTPMBase,
  UINT32                SeCfTPMSize
  );


//
// PPI definition
//
typedef struct SEC_FTPM_PPI {
  SEC_SEND_FTPM_SIZE      SeCSendfTPMSize;
  SEC_CONFIG_FTPM         SeCConfigfTPM;
} SEC_FTPM_PPI;

#endif
