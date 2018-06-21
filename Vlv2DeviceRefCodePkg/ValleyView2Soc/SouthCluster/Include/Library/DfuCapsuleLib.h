/*++
Copyright (c) 2009 Intel Corporation. All rights reserved.
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.
--*/
#ifndef _DFU_CAPSULE_LIB_H
#define _DFU_CAPSULE_LIB_H
#include <Uefi.h>
#include <Uefi/UefiSpec.h>

#define UPDATE_SUCCESS   0
#define UPDATE_FAIL_GENERIC 1
#define UPDATE_NO_RESOURCE 2
#define UPDATE_VERSION_INCORRECT 3
#define UPDATE_INVALID_IMAGE 4
#define UPDATE_AUTH_ERROR 5
#define UPDATE_AC_NOT_CONNECTED 6
#define UPDATE_LOW_BATTERY 7

typedef enum {
  NO_FAILURE      =  0,
  NOT_UPDATED     =  1,
  PARTIAL_UPDATED =  2
} DFU_FAILURE_REASON;

extern EFI_GUID gEfiDFUResultGuid;
extern EFI_GUID gEfiDFUVerGuid;

typedef struct {
  EFI_GUID DFU_DEVICE_GUID;
  UINT32 UpdateStatus;
  DFU_FAILURE_REASON Failure;
} DFU_UPDATE_STATUS;

extern EFI_STATUS
EFIAPI
LoadDFUImage (
  void **pFileBuffer,
  UINT64 *pFileSize
  );

extern EFI_STATUS
EFIAPI
ReportUpdateStatus(
  UINT32 UpdateStatus,
  DFU_FAILURE_REASON failure
  );


#endif
