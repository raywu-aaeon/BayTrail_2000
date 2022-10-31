/*++
  This file contains a 'Sample Driver' and is licensed as such  
  under the terms of your license agreement with Intel or your  
  vendor.  This file may be modified by the user, subject to    
  the additional terms of the license agreement                 
--*/
/*++

Copyright (c) 2010 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  SeCRcInfo.h
    
Abstract:

  This file defines the SEC RC Info Protocol.

--*/
#ifndef _SEC_RC_INFO_H_
#define _SEC_RC_INFO_H_

//
// Define SEC RC INFO protocol GUID
//
#define EFI_SEC_RC_INFO_PROTOCOL_GUID \
  { \
    0x11fbfdfb, 0x10d2, 0x43e6, 0xb5, 0xb1, 0xb4, 0x38, 0x6e, 0xdc, 0xcb, 0x9a \
  }

//
// Extern the GUID for protocol users.
//
extern EFI_GUID                       gEfiSeCRcInfoProtocolGuid;


// Revision 1:  Original version
#define SEC_RC_INFO_PROTOCOL_REVISION_1  1
#define SEC_RC_VERSION                   0x00050000

typedef union _RC_VERSION {
  UINT32  Data;
  struct {
    UINT32  RcBuildNo : 8;
    UINT32  RcRevision : 8;
    UINT32  RcMinorVersion : 8;
    UINT32  RcMajorVersion : 8;
  } Fields;
} RC_VERSION;

//
// Protocol definition
//
typedef struct _EFI_SEC_RC_INFO_PROTOCOL {
  UINT8       Revision;
  RC_VERSION  RCVersion;
} EFI_SEC_RC_INFO_PROTOCOL;
#endif
