/**
  This file contains an 'Intel Peripheral Driver' and uniquely        
  identified as "Intel Reference Module" and is
  licensed for Intel CPUs and chipsets under the terms of your   
  license agreement with Intel or your vendor.  This file may   
  be modified by the user, subject to additional terms of the
  license agreement
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
  PchInitCommon.h

  @brief 
  Header file for PCH common Initialization Driver.

**/
#ifndef _PCH_INIT_COMMON_DRIVER_H_
#define _PCH_INIT_COMMON_DRIVER_H_

#include <Protocol/PchPlatformPolicy.h>
#ifdef ECP_FLAG
#include "EdkIIGlueDxe.h"
#include "EfiScriptLib.h"
#else
#include <Library/S3BootScriptLib.h>
#endif

#define PCH_INIT_COMMON_SCRIPT_IO_WRITE(TableName, Width, Address, Count, Buffer) \
          S3BootScriptSaveIoWrite (Width, Address, Count, Buffer)

#define PCH_INIT_COMMON_SCRIPT_IO_READ_WRITE(TableName, Width, Address, Data, DataMask) \
          S3BootScriptSaveIoReadWrite (Width, Address, Data, DataMask)

#define PCH_INIT_COMMON_SCRIPT_MEM_WRITE(TableName, Width, Address, Count, Buffer) \
          S3BootScriptSaveMemWrite (Width, Address, Count, Buffer)

#define PCH_INIT_COMMON_SCRIPT_MEM_READ_WRITE(TableName, Width, Address, Data, DataMask) \
          S3BootScriptSaveMemReadWrite (Width, Address, Data, DataMask)

#define PCH_INIT_COMMON_SCRIPT_PCI_CFG_WRITE(TableName, Width, Address, Count, Buffer) \
          S3BootScriptSavePciCfgWrite (Width, Address, Count, Buffer)

#define PCH_INIT_COMMON_SCRIPT_PCI_CFG_READ_WRITE(TableName, Width, Address, Data, DataMask) \
          S3BootScriptSavePciCfgReadWrite (Width, Address, Data, DataMask)

#define PCH_INIT_COMMON_SCRIPT_STALL(TableName, Duration) S3BootScriptSaveStall (Duration)

#endif
