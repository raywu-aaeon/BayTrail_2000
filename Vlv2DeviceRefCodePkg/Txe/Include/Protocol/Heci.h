/*++
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
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

  Heci.h

Abstract:

  EFI HECI Protocol

--*/
#ifndef _EFI_HECI_H
#define _EFI_HECI_H

#include <SeCState.h>

#define HECI_PROTOCOL_GUID \
  { \
    0xcfb33810, 0x6e87, 0x4284, 0xb2, 0x3, 0xa6, 0x6a, 0xbe, 0x7, 0xf6, 0xe8 \
  }

#define EFI_HECI_PROTOCOL_GUID  HECI_PROTOCOL_GUID

typedef struct _EFI_HECI_PROTOCOL EFI_HECI_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_SENDWACK) (
  IN OUT  UINT32           *Message,
  IN OUT  UINT32           Length,
  IN OUT  UINT32           *RecLength,
  IN      UINT8            HostAddress,
  IN      UINT8            SECAddress
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_READ_MESSAGE) (
  IN      UINT32           Blocking,
  IN      UINT32           *MessageBody,
  IN OUT  UINT32           *Length
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_SEND_MESSAGE) (
  IN      UINT32           *Message,
  IN      UINT32           Length,
  IN      UINT8            HostAddress,
  IN      UINT8            SECAddress
  );
typedef
EFI_STATUS
(EFIAPI *EFI_HECI_RESET) (
  VOID
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_INIT) (
  VOID
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_REINIT) (
  VOID
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_RESET_WAIT) (
  IN        UINT32           Delay
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_GET_SEC_STATUS) (
  IN UINT32                       *Status
  );

typedef
EFI_STATUS
(EFIAPI *EFI_HECI_GET_SEC_MODE) (
  IN UINT32                       *Mode
  );



typedef struct _EFI_HECI_PROTOCOL {
  EFI_HECI_SENDWACK       SendwACK;
  EFI_HECI_READ_MESSAGE   ReadMsg;
  EFI_HECI_SEND_MESSAGE   SendMsg;
  EFI_HECI_RESET          ResetHeci;
  EFI_HECI_INIT           InitHeci;
  EFI_HECI_RESET_WAIT     SeCResetWait;
  EFI_HECI_REINIT         ReInitHeci;
  EFI_HECI_GET_SEC_STATUS  GetSeCStatus;
  EFI_HECI_GET_SEC_MODE    GetSeCMode;


} EFI_HECI_PROTOCOL;

extern EFI_GUID gEfiHeciProtocolGuid;

#endif // _EFI_HECI_H
