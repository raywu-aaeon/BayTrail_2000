/*++
  This file contains an 'Intel Peripheral Driver' and uniquely
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/*++

Copyright (c)  2004 - 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  TDTPlatformPolicy.h

Abstract:

  Defines and TDTPlatformDriver driver.
  This driver implements the TDT Platform Policy protocol for Theft Deterrence Technology.

--*/
#ifndef _DXE_TDT_PLATFORM_POLICY_H_
#define _DXE_TDT_PLATFORM_POLICY_H_

//
// TDT policy provided by platform for DXE phase
//
#define DXE_PLATFORM_TDT_POLICY_GUID \
  { \
    0x20daf0fc, 0x5548, 0x44dc, 0xa4, 0x2a, 0x60, 0xea, 0xf0, 0xa2, 0x2e, 0x47 \
  }

//
// Protocol revision number
// Any backwards compatible changes to this protocol will result in an update in the revision number
// Major changes will require publication of a new protocol
//
#define DXE_PLATFORM_TDT_POLICY_PROTOCOL_REVISION 1

extern EFI_GUID gDxePlatformTdtPolicyGuid;
#pragma pack(1)
typedef struct {
  //
  // Byte 0, bit definition for functionality enable/disable
  //
  UINT8 TdtConfig;
  UINT8 TdtRecoveryAttempt;
  UINT8 TdtEnterSuspendState;
  UINT8 TdtPBAEnable;
  //
  // Byte 12-23 Reserved and make AMT_CONFIG as 32 bit alignment
  //
  UINT8 ByteReserved[28];
} TDT_CONFIG;
#pragma pack()
//
// AMT DXE Platform Policiy ====================================================
//
typedef struct _DXE_TDT_POLICY_PROTOCOL {
  UINT8       Revision;
  TDT_CONFIG  Tdt;
} DXE_TDT_POLICY_PROTOCOL;
#endif
