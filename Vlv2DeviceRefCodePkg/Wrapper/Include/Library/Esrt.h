/*++
Copyright (c)  2008 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

 Esrt.h

Abstract:


--*/
#ifndef _DFU_ESRT_H_
#define _DFU_ESRT_H_

typedef struct {
    EFI_GUID         FwClass;
    UINT32           FwType;
    UINT32           FwVersion;
    UINT32           FwLstCompatVersion;
    UINT32           CapsuleFlags;
    UINT32           LastAttemptVersion;
    UINT32           LastAttemptStatus;
} FW_RES_ENTRY;

typedef struct {
    UINT32           NumEntries;
    FW_RES_ENTRY     FwEntries[256];
} FW_RES_ENTRY_LIST;


typedef struct {
    UINT32          FwResourceCount;
    UINT32          FwResourceMax;
    UINT64          FwResourceVersion;
} EFI_SYSTEM_RESOURCE_TABLE;


typedef
EFI_STATUS
(EFIAPI *ESRT_POPULATE_TABLE)(
);

typedef
EFI_STATUS
(EFIAPI *ESRT_UPDATE_TABLE_ENTRY_BY_GUID)(
    IN EFI_GUID FwEntryGuid,
    IN FW_RES_ENTRY *FwEntry
);

typedef
EFI_STATUS
(EFIAPI *ESRT_GET_FW_ENTRY_BY_GUID)(
    IN EFI_GUID FwEntryGuid,
    OUT FW_RES_ENTRY *FwEntry
);


#pragma pack()

typedef struct _ESRT_OPERATION_PROTOCOL {
    ESRT_POPULATE_TABLE					EsrtPopulateTable;
    ESRT_UPDATE_TABLE_ENTRY_BY_GUID		EsrtUpdateTableEntryByGuid;
    ESRT_GET_FW_ENTRY_BY_GUID			EsrtGetFwEntryByGuid;
} ESRT_OPERATION_PROTOCOL;

extern EFI_GUID gEfiEsrtOperationProtocolGuid;

#endif
