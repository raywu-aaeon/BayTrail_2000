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

#ifndef _SYS_FW_UPDATE_H
#define _SYS_FW_UPDATE_H

#include <Library/DevicePathLib.h>
#include <token.h>

//#define VLV_VAR_REGION_SIZE       0x80000  //320K
#define VLV_VAR_REGION_SIZE       NVRAM_SIZE*2
#define VLV_IBB_REGION_SIZE       0x20000  //128K

//#define VLV_SEC_REGION_SIZE       0x300000 //SEC region
#define VLV_SEC_REGION_SIZE       FLASH_SIZE //SEC region

//#define VLV_BIOS_REGION_SIZE      0x300000
#define VLV_BIOS_REGION_SIZE      FLASH_SIZE

#define VLV_SPI_REGION_SIZE       0x800000 //Not use
#define VLV_BIOS_STGTWO_OFFSET    0x600000 //Not use

// Macro to Enable/Disable the Validation of the FW
// using the the Key read from SPI flash
// Use the following line to Enable Validating the FW using the key from SPI Flash
//#define SPI_KEY_VALIDATION

#define IFWI_AUTH_HEADER_SIZE  0x228
#define IFWI_SFIH_HEADER_SIZE  0x38
#define SHA256_DIGEST_SIZE     32

#ifndef SPI_KEY_VALIDATION
#define RSA1_KEY_SIZE          128
#else
#define RSA1_KEY_SIZE               256 /*As per Security Requirement, key must be 2048-bit or larger*/
#define RSA_KEY_EXPONENT_SIZE       4
#define PKCS_VERSION_1_5            0x00000015
#define VLV_MANIFEST_ADDRESS        0x7E0000
#define VLV_MANIFEST_OEM_DATA_SIZE  0x188
#define VLV_MANIFET_SOCID_SIZE      16

#pragma pack(1)
typedef struct _SECURE_BOOT_MANIFEST_SIGNED_FIELDS {
  UINT32 ManifestIdentifier;
  UINT32 ManifestVersion;
  UINT32 Size;
  UINT32 SecureVersionNumber;
  UINT32 Reserved1;
  UINT8  IBBHash[SHA256_DIGEST_SIZE];
  UINT32 Reserved2;
  UINT8  Reserved3[32];
  UINT8  OEMData[VLV_MANIFEST_OEM_DATA_SIZE];
  UINT32 DebugActivation;
  UINT8  SOCID[VLV_MANIFET_SOCID_SIZE];
  UINT32 ModulusSize;
  UINT32 ExponentSize;
} SECURE_BOOT_MANIFEST_SIGNED_FIELDS;

typedef struct _SECURE_BOOT_MANIFEST_MODULUS_EXPONENT_FIELDS {
  UINT8 Modulus[RSA1_KEY_SIZE];
  UINT32 Exponent;
} SECURE_BOOT_MANIFEST_MODULUS_EXPONENT_FIELDS;

#pragma pack()
#endif

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

#pragma pack(1)
//
//System firmware image header(SFIH). This is the part of
//
typedef struct _SYS_FW_SFIH {
  UINT32     Signature;       //SFIH header signature. Always "SFIH"
  UINT32     IFWILength;      //IFWI image section length in byte, including SFIH header.
  UINT32     IFWIVersion;     //4-byte IFWI version
  UINT32     BIOSVersion;     //4-byte BIOS version
  UINT32     MCUVersion;      //4-byte MCU version
  UINT32     SECVersion;      //4-byte SEC version
  UINT32     IBBOffset;       //offset of IBB from start of SFIH
  UINT32     IBBSize;         //Constant: 128*1024 bytes
  UINT32     SecondStgOffset; //Second stage binary offset from start of SFIH
  UINT32     SecondStgSize;   //Size of 2nd stage;
  UINT32     PDROffset;       //Offset of PDR. if 0, PDR does not exist
  UINT32     PDRSize;         //Size of PDR. 0 if PDR does not exist
  UINT32     SecUpdOffset;    //SEC UPD image offset from start of SFIH
  UINT32     SecUpdSize;      //Size of SEC UPD image.
} SYS_FW_SFIH_HEADER, *PSYS_FW_SFIH_HEADER;

//IFWI Authentication header
typedef struct _IFWI_AUTH_HEADER {
  UINT32    AuthHeaderSize;      //IFWI authentication header size including the padding(padding is always FF);
  UINT8     IFWIHash[32];        //32-byte IFWI SHA256 hash including SFIH and binary followed. Auth header not included.
  UINT32    Reserved;            //4 bytes reserved;
  UINT8     IFWISignature[512];  //256-byte RSA signature for the header
} IFWI_AUTH_HEADER, *PIFWI_AUTH_HEADER;

#define PCI_DEVICE_PATH_NODE(Func, Dev) \
  { \
    { \
      HARDWARE_DEVICE_PATH, \
      HW_PCI_DP, \
      { \
        (UINT8) (sizeof (PCI_DEVICE_PATH)), \
        (UINT8) ((sizeof (PCI_DEVICE_PATH)) >> 8) \
      } \
    }, \
    (Func), \
    (Dev) \
  }

#define PNPID_DEVICE_PATH_NODE(PnpId) \
  { \
    { \
      ACPI_DEVICE_PATH, \
      ACPI_DP, \
      { \
        (UINT8) (sizeof (ACPI_HID_DEVICE_PATH)), \
        (UINT8) ((sizeof (ACPI_HID_DEVICE_PATH)) >> 8) \
      }, \
    }, \
    EISA_PNP_ID((PnpId)), \
    0 \
  }

#define gPciRootBridge \
  PNPID_DEVICE_PATH_NODE(0x0A03)

#define gEndEntire \
  { \
    END_DEVICE_PATH_TYPE, \
    END_ENTIRE_DEVICE_PATH_SUBTYPE, \
    { \
      END_DEVICE_PATH_LENGTH, \
      0 \
    } \
  }


typedef struct {
  ACPI_HID_DEVICE_PATH      PciRootBridge;
  PCI_DEVICE_PATH           PciDevice;
  EFI_DEVICE_PATH_PROTOCOL  End;
} PLATFORM_PCI_DEVICE_PATH;


typedef struct {
  CHAR16    *FileName;
  CHAR16    *String;
} FILE_NAME_TO_STRING;


#pragma pack()

//
//This API accepts the buffer passed to it, and analyze out the SFIH and AUTH header. On any error, it will return EFI_UNSUPPORTED.
//On success, SFIH and AUTH header will be
//
EFI_STATUS
GetSysFwLayOutInfo(UINT8 *pSysFwBuffer, UINT64 FwBufferSize, PSYS_FW_SFIH_HEADER pSfihHdr, PIFWI_AUTH_HEADER pIfwiAuthHdr);

//
//This API checks the power status before update is performed.
//
EFI_STATUS
PreUpdateCheck(UINT8 *PwrStatus);

#endif