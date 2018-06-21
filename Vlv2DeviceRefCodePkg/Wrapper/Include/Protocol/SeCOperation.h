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

#ifndef _EFI_SEC_OPERATION_H_
#define _EFI_SEC_OPERATION_H_


//
// Global ID for the SW SMI Protocol
//

#define EFI_SEC_OPERATION_PROTOCOL_GUID    \
  {0x704ebea2, 0x5ee6, 0x4898, 0x96, 0x59, 0x1, 0x8b, 0x74, 0xb4, 0x47, 0x89}

#define  SEC_OP_UNCONFIGURATION     0x01
#define  SEC_OP_CHECK_UNCONFIG      0x02
#define  SEC_OP_CHECK_HMRFPO        0x03

#pragma pack(1)

typedef struct {
    UINT32  CodeMinor;
    UINT32  CodeMajor;
    UINT32  CodeBuildNo;
    UINT32  CodeHotFix;
} SEC_VERSION_INFO;

typedef struct {
    UINT32       SeCEnable;
    UINT32       HmrfpoEnable;
    UINT32       FwUpdate;
    UINT32       SeCOpEnable;
    UINT32       SeCOpMode;
    UINT32       SeCFsIntegrityVal;    //(CSP20130313D+)
    BOOLEAN      SeCVerValid;
    BOOLEAN      SeCCapabilityValid;
    BOOLEAN      SeCFeatureValid;
    BOOLEAN      SeCOEMTagValid;
    SEC_VERSION_INFO  SeCVer;
    UINT32       SeCCapability;
    UINT32       SeCFeature;
    UINT32       SeCOEMTag;
    UINT32       SeCExist;
    UINT32       SeCEOPDone;
} SEC_INFOMATION;

typedef
EFI_STATUS
(EFIAPI *GET_PLATFORM_SEC_INFO)(
    OUT SEC_INFOMATION * SecInfo
);

typedef
EFI_STATUS
(EFIAPI *SET_PLATFORM_SEC_INFO)(
    IN  SEC_INFOMATION * SecInfo
);

typedef
EFI_STATUS
(EFIAPI *PERFORM_SEC_OPERATION)(
    IN  UINTN SeCOpId
);

#pragma pack()

typedef struct _SEC_OPERATION_PROTOCOL {
    GET_PLATFORM_SEC_INFO   GetPlatformSeCInfo;
    SET_PLATFORM_SEC_INFO   SetPlatformSeCInfo;
    PERFORM_SEC_OPERATION   PerformSeCOperation;
} SEC_OPERATION_PROTOCOL;

extern EFI_GUID gEfiSeCOperationProtocolGuid;

#endif
