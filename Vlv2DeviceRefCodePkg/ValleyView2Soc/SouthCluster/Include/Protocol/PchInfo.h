/**
  This file contains a 'Sample Driver' and is licensed as such  
  under the terms of your license agreement with Intel or your  
  vendor.  This file may be modified by the user, subject to    
  the additional terms of the license agreement                 
**/
/**

Copyright (c) 2012 - 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file 
  PchInfo.h
    
  @brief 
  This file defines the PCH Info Protocol.

**/
#ifndef _PCH_INFO_H_
#define _PCH_INFO_H_


#define EFI_PCH_INFO_PROTOCOL_GUID \
  { \
    0xd31f0400, 0x7d16, 0x4316, 0xbf, 0x88, 0x60, 0x65, 0x88, 0x3b, 0x40, 0x2b \
  }
extern EFI_GUID                       gEfiPchInfoProtocolGuid;

///
/// Forward reference for ANSI C compatibility
///
typedef struct _EFI_PCH_INFO_PROTOCOL EFI_PCH_INFO_PROTOCOL;

///
/// Protocol revision number
/// Any backwards compatible changes to this protocol will result in an update in the revision number
/// Major changes will require publication of a new protocol
///
/// Revision 1:  Original version
///
#define PCH_INFO_PROTOCOL_REVISION_1  1
#define PCH_INFO_PROTOCOL_REVISION_2  2

///
/// RCVersion[7:0] is the release number.
/// For example:
/// VlvFramework 0.6.0-01 should be 00 06 00 01 (0x00060001)
/// VlvFramework 0.6.2    should be 00 06 02 00 (0x00060200)
///
#define PCH_RC_VERSION                0x01000000

///
/// Protocol definition
///
struct _EFI_PCH_INFO_PROTOCOL {
  UINT8   Revision;
  UINT8   BusNumber;
  UINT32  RCVersion;
};

#endif
