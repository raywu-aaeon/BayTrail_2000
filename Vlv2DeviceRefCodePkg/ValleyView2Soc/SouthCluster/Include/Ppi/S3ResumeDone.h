/**
  This file contains a 'Sample Driver' and is licensed as such
  under the terms of your license agreement with Intel or your
  vendor.  This file may be modified by the user, subject to
  the additional terms of the license agreement
**/
/**

Copyright (c) 2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


  @file
  S3ResumeDone.h

  @brief
  This file defines S3 Resume Done PPI

**/
#ifndef _S3_RESUME_DONE_H_
#define _S3_RESUME_DONE_H_

//
// Define the S3 Resume Done PPI GUID
//

#define S3_RESUME_DONE_PPI_GUID \
  { \
    0xb710ba, 0x8cd6, 0x4bf3, 0xab, 0x7a, 0x9a, 0x24, 0xf5, 0x4c, 0xc3, 0x34 \
  }
extern EFI_GUID               gS3ResumeDonePpiGuid;

#endif
