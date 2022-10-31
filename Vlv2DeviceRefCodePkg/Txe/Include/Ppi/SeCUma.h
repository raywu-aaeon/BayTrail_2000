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

  PchMeUma.h

Abstract:

  Interface definition details for PCH Me UMA.

--*/
#ifndef _SEC_UMA_PPI_H_
#define _SEC_UMA_PPI_H_

#define SEC_UMA_PPI_GUID \
  { \
    0xcbd86677, 0x362f, 0x4c04, 0x94, 0x59, 0xa7, 0x41, 0x32, 0x6e, 0x05, 0xcf \
  }

extern EFI_GUID  gSeCUmaPpiGuid;

//
// Revision
//
#define SEC_UMA_PPI_REVISION 1

//
// define the MRC recommended boot modes.
//
typedef enum {
  s3Boot, // In current implementation, bmS3 == bmWarm
  warmBoot,
  coldBoot,
  fastBoot,
} MRC_BOOT_MODE_T;

typedef
EFI_STATUS
(EFIAPI *SEC_SEND_UMA_SIZE) (
  IN EFI_PEI_SERVICES **PeiServices
  );

typedef
EFI_STATUS
(EFIAPI *SEC_CONFIG_DID_REG) (
#ifdef ECP_FLAG
  IN       EFI_PEI_SERVICES **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES **PeiServices,
#endif
  MRC_BOOT_MODE_T           MrcBootMode,
  UINT8                     InitStat,
  UINT32                    SeCUmaBase,
  UINT32                    SeCUmaSize
  );

typedef
EFI_STATUS
(EFIAPI *HANDLE_SEC_BIOS_ACTION) (
#ifdef ECP_FLAG
  IN       EFI_PEI_SERVICES **PeiServices,
#else
  IN CONST EFI_PEI_SERVICES **PeiServices,
#endif
  UINT8                     BiosAction
  );

//
// PPI definition
//
typedef struct SEC_UMA_PPI {
  SEC_SEND_UMA_SIZE      SeCSendUmaSize;
  SEC_CONFIG_DID_REG     SeCConfigDidReg;
  HANDLE_SEC_BIOS_ACTION HandleSeCBiosAction;
} SEC_UMA_PPI;

#endif
