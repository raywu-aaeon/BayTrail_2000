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

#ifndef _FOTA_FW_UPDATE_H
#define _FOTA_FW_UPDATE_H

#include <Library/DevicePathLib.h>

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

typedef struct _BIOS_MODULE_INFO {
  UINT32 SpiOffset;
  UINT32 Size;
  UINT8  Hash[32];
} BIOS_MODULE_INFO;

typedef struct _UCODE_INFO {
  UINT32 SpiOffset;
  UINT32 Size;
} UCODE_INFO;

typedef struct _BIOS_IMAGE_INFO {
  UINT32              Signature;  //Fixed $BNF
  BIOS_MODULE_INFO    StageTwoInfo;
  BIOS_MODULE_INFO    RecoveryModuleInfo;
  UCODE_INFO          UCodeInfo;
  UCODE_INFO          UCodeRecoveryInfo;
} BIOS_IMAGE_INFO;

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


typedef struct _FPTHdrV2 {
  UINT32 ReservedQ[4];   //4 DWORD filled with all 0s
  UINT32 Signature;     //$FPT
  UINT32 NumFptEntries;  //Number of FPT entries
  UINT8  HdrVer;
  UINT8  EntryVer;
  UINT8  HdrLen;
  UINT8  CheckSum;
  UINT16 FlashLifeTime;
  UINT16 FlashCycleLmt;
  UINT32 DRAMSize;
  UINT32 FPTFlags;
  UINT32 ReservedS[2];   //2 DWORD reserved.
} FPTHdrV2;

typedef struct _FPTEntryHdrV1 {
  UINT32 PartitionName;       //Unique name of the partition
  UINT32 PartitionOwner;
  UINT32 Offset;              //Offset of the partition from beginning of FPT
  UINT32 Length;              //Length of the partition
  UINT32 TokensOnStart;
  UINT32 MaximumTokens;
  UINT32 ScratchSectors;
  UINT32 Attributes;
} FPTEntryHdrV1;

typedef union {
  UINT32 dw;
  struct {
    UINT32 SecureBoot:1;      //0: SB flow is executed
    UINT32 Recovery:1;        //1: Recovery flow was executed
    UINT32 DebugWasEnabled:1; //2: Exi and prob mode were enabled since last G3 resume/GRST
    UINT32 DebugEnabled:1;    //3: Exi and prob mode wereenabled before CPU reset de-assertion
    UINT32 EmulationMode:1;   //4: Current flow uses mirroring file instead of fuse
    UINT32 SVN:6;             //5:10: Secure Version number
    UINT32 KeyManifestID:4;   //11:14 Key Manifest ID. If 0, no key manfiest
    UINT32 KeyManifestSVN:4;  //15:18 SVN of key manifest
    UINT32 AltBiosLimit:13;   //Alternative BIOS limit
  } r;
} SEC_BOOT_STS;

typedef struct _VLV_SEC_PLATFORM_INFO {
  UINT32  AlternativeBiosLimit;
  BOOLEAN WithKeyManifest;
} SEC_PLATFORM_INFO;

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

typedef
EFI_STATUS
(EFIAPI *FOTA_API_HANDLE_IBB) (
  IN UINT8           *DataIn,
  IN OUT UINT8       *DataOut,
  IN OUT UINTN       *pIBBSize
  );

typedef
EFI_STATUS
(EFIAPI *FOTA_API_HANDLE_STAGETWO) (
  IN BIOS_IMAGE_INFO *ImageInfo,
  IN UINT8           *DataIn,
  IN OUT UINT8       *DataOut,
  IN OUT UINTN       *pStage2Size
  );

typedef
EFI_STATUS
(EFIAPI *FOTA_API_HANDLE_UCODE) (
  IN BIOS_IMAGE_INFO *ImageInfo,
  IN UINT8           *DataIn,
  IN OUT UINT8       *DataOut,
  IN OUT UINTN       *pUCodeSize
  );

typedef
EFI_STATUS
(EFIAPI *FOTA_API_HANDLE_VARIABLE) (
  IN UINT8           *DataIn,
  IN OUT UINT8       *DataOut,
  IN OUT UINTN       *VariableSize
  );

typedef
EFI_STATUS
(EFIAPI *FOTA_API_HANDLE_RECOVERY) (
  IN BIOS_IMAGE_INFO *ImageInfo,
  IN UINT8           *DataIn,
  IN OUT UINT8       *DataOut,
  IN OUT UINTN       *pRecoverySize
  );

typedef struct _FOTA_BIOS_UPDATE_TAB {
  FOTA_API_HANDLE_IBB      ProcessIBB;
  FOTA_API_HANDLE_STAGETWO ProcessStageTwo;
  FOTA_API_HANDLE_UCODE    ProcessUcode;
  FOTA_API_HANDLE_VARIABLE ProcessVariable;
  FOTA_API_HANDLE_RECOVERY ProcessRecovery;
  UINT16                   GetStdDescLength;
} FOTA_BIOS_UPDATE_TABLE;

#endif
