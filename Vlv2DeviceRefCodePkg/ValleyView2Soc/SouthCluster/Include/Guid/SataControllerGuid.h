//
// This file contains 'Framework Code' and is licensed as such 
// under the terms of your license agreement with Intel or your
// vendor.  This file may not be modified, except as allowed by
// additional terms of your license agreement.                 
//
/*++

Copyright (c)  2009 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  SataControllerGuid.h
    
Abstract:

  GUID for use in describing SataController

--*/
#ifndef _SERIAL_ATA_CONTROLLER_GUID_H_
#define _SERIAL_ATA_CONTROLLER_GUID_H_

#ifdef ECP_FLAG
#define PCH_SATA_CONTROLLER_DRIVER_GUID \
  { \
    0xbb929da9, 0x68f7, 0x4035, 0xb2, 0x2c, 0xa3, 0xbb, 0x3f, 0x23, 0xda, 0x55 \
  }
#else
#define PCH_SATA_CONTROLLER_DRIVER_GUID \
  {\
    0xbb929da9, 0x68f7, 0x4035, 0xb2, 0x2c, 0xa3, 0xbb, 0x3f, 0x23, 0xda, 0x55 \}

#endif

extern EFI_GUID gSataControllerDriverGuid;
#endif
