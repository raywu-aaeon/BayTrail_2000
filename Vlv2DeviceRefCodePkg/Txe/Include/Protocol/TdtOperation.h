/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    SeCOperation.h

Abstract:

    SeC Operation protocol



--*/

#ifndef _EFI_TDT_OPERATION_H_
#define _EFI_TDT_OPERATION_H_


//
// Global ID for the SW SMI Protocol
//

#define EFI_TDT_OPERATION_PROTOCOL_GUID    \
  {0xfd301ba4, 0x5e62, 0x4679, 0xa0, 0x6f, 0xe0, 0x9a, 0xab, 0xdd, 0x2a, 0x91}


#pragma pack()

typedef struct {
  UINT32       TdtEnabled;
  UINT32       PBACapable;
  UINT32       TdtEnrolled;
  UINT32       TdtState;
  UINT32       TdtWWAN;
} TDT_INFOMATION;

typedef enum _TDT_PERFORM_OPERATION_ID {
  TDT_PERFORM_NOTHING = 0,
  TDT_PERFORM_SUSPEND,
  TDT_PERFORM_ATAM_RECOVERY,
  TDT_PERFORM_NOTIFY,
  TDT_PERFORM_MAX
} TDT_PERFORM_OPERATION_ID;

typedef
EFI_STATUS
(EFIAPI *GET_PLATFORM_TDT_INFO) (
  OUT TDT_INFOMATION *TdtInfo
  );

typedef
EFI_STATUS
(EFIAPI *GET_PLATFORM_TDT_OPERATION) (
  OUT TDT_PERFORM_OPERATION_ID *TdtOperation
  );

typedef
EFI_STATUS
(EFIAPI *PERFORM_TDT_OPERATION) (
  );

#pragma pack()

typedef struct _TDT_OPERATION_PROTOCOL {
  GET_PLATFORM_TDT_INFO       GetPlatformTdtInfo;
  GET_PLATFORM_TDT_OPERATION  GetPlatformTdtOperation;
  PERFORM_TDT_OPERATION       PerformTdtOperation;
} TDT_OPERATION_PROTOCOL;

extern EFI_GUID gEfiTdtOperationProtocolGuid;

#endif
