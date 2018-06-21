/*++

This file contains a 'Sample Driver' and is licensed as such
under the terms of your license agreement with Intel or your
vendor.  This file may be modified by the user, subject to  
the additional terms of the license agreement               

--*/

/*++
Copyright (c)  2008-2013 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.
  
Module Name:

 SysFwupdate.h

Abstract:

*/

#ifndef _FOTA_FW_UPDATE_COMMON_H
#define _FOTA_FW_UPDATE_COMMON_H

#include "AutoGen.h"

#define VLV_VAR_REGION_SIZE       0x80000  //320K
#define VLV_IBB_REGION_SIZE       0x20000  //128K
#define VLV_SEC_REGION_SIZE       0x300000 //TXE region
#define VLV_BIOS_REGION_SIZE      0x300000
#define VLV_SPI_REGION_SIZE       0x800000
#define VLV_BIOS_STGTWO_OFFSET    0x600000

#define VLV_IBB_OFFSET_DEFAULT_132K       0x7DFC00
#define VLV_IBB_OFFSET_DEFAULT_128K       0x7E0000

#define VLV_DEFAULT_FOTA_STGTWO_OFFSET    0x170000//(_PCD_VALUE_PcdFlashFvMainBase - 0xFFD00000) //AMI_OVERRIDE
#define VLV_DEFAULT_FOTA_STGTWO_SIZE      0x150000//(_PCD_VALUE_PcdFlashFvMainSize + _PCD_VALUE_PcdFlashFvRecovery2Size) //AMI_OVERRIDE

#define VLV_DEFAULT_FOTA_UCODE_OFFSET     0x100000
#define VLV_DEFAULT_FOTA_UCODE_SIZE       0x30000

#define IFWI_AUTH_HEADER_SIZE     0x228
#define IFWI_SFIH_HEADER_SIZE     0x38

#define SHA256_DIGEST_SIZE        32
#define RSA1_KEY_SIZE             128

#define REG_SEC_BOOT_STS          0x50  //secure boot status register at TXE CFG offset 0x50

#define PRE_CHECK_PASS            0x0
#define PRE_CHECK_NO_AC           0x1
#define PRE_CHECK_LOW_BAT         0x2
#define PRE_CHECK_BAT_THRESHOLD   25    //According to UEFI update spec.

#define ULPMC_I2C_CONTROLLER_ID   0
#define ULPMC_I2C_SLAVE_ADDR      120
#define ULPMC_FG_FULLCAP_CMD_0    0x12
#define ULPMC_FG_FULLCAP_CMD_1    0x13
#define ULPMC_FG_REMAINCAP_CMD_0  0x10
#define ULPMC_FG_REMAINCAP_CMD_1  0x11

#define I2C_ACPI_ID_LEN           16   //I2C0X\\SFFFF\\400K
#define I2C_ADDRESS_MAX           0x3FE
#define I2C_TIMEOUT_DEFAULT       1000
#define I2C_READ_MAX              (1024*1024*10+1)   //Defined by the tool itself for maximum read length.

#define DID_ACPI_ID_PREFIX        "I2C0"
#define DID_ACPI_ID_SUFFIX        "\\SFFFF"
#define DID_ACPI_ID_SUFFIX_400K   "\\400K"

#define VLV_MANIFEST_ADDRESS      0x7E0000
#define VLV_OEM_DATA_OFFSET       0x58    //Offset 0x58 is the start offset of OEM data in 1K manifest 
#define VLV_MANIFEST_SIZE         0x400
#define VLV_KEY_MANIFEST_SIZE     0x1000
#define VLV_OEM_DATA_SIZE         400

#define FOTA_IBB_SIZE_128K        (128*1024)
#define FOTA_IBB_SIZE_132K        (132*1024)

#endif
