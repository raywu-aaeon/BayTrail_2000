/**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
//
// This file contains an 'Intel Peripheral Driver' and is      
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may 
// be modified by the user, subject to additional terms of the 
// license agreement                                           
//
/** @file

  The internal header file includes the common header files, defines
  internal structure and functions used by ImageVerificationLib.

Copyright (c) 2009 - 2010, Intel Corporation. All rights reserved. <BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/

#ifndef __IMAGEVERIFICATIONLIB_H__
#define __IMAGEVERIFICATIONLIB_H__

#include <Protocol/DevicePath.h>
#include <Library/SecurityManagementLib.h>

#include <Guid/ImageAuthentication.h>
#include <Protocol/Hash.h>

#define EFI_SECURE_BOOT_NAME              L"SecureBoot"
#define SECURE_BOOT                       1
#define NONSECURE_BOOT                    0

#define EFI_CERT_TYPE_RSA2048_SHA256_SIZE 256
#define EFI_CERT_TYPE_RSA2048_SIZE        256
#define EFI_CERT_TYPE_SHA256_SIZE         32
#define MAX_NOTIFY_STRING_LEN             64

//
// Image type definitions
//
#define IMAGE_UNKNOWN                         0x00000000
#define IMAGE_FROM_FV                         0x00000001
#define IMAGE_FROM_OPTION_ROM                 0x00000002
#define IMAGE_FROM_REMOVABLE_MEDIA            0x00000003
#define IMAGE_FROM_FIXED_MEDIA                0x00000004

//
// Authorization policy bit definition
//
#define ALWAYS_EXECUTE                         0x00000000
#define NEVER_EXECUTE                          0x00000001
#define ALLOW_EXECUTE_ON_SECURITY_VIOLATION    0x00000002
#define DEFER_EXECUTE_ON_SECURITY_VIOLATION    0x00000003
#define DENY_EXECUTE_ON_SECURITY_VIOLATION     0x00000004
#define QUERY_USER_ON_SECURITY_VIOLATION       0x00000005

//
// Support hash types
//
#define HASHALG_SHA1                           0x00000001
#define HASHALG_SHA256                         0x00000002

#define MAX_ELEM_NUM        28 // ~28 max number of PE hdr elements to be hashed in one pass
///
/// SHA-1 digest size in bytes.
///
#define SHA1_DIGEST_SIZE    20
///
/// SHA-256 digest size in bytes
///
#define SHA256_DIGEST_SIZE  32

//
// Set max digest size as SHA256 Output (32 bytes) by far
//
#define MAX_DIGEST_SIZE    SHA256_DIGEST_SIZE      

#define ALIGNMENT_SIZE                    8
#define ALIGN_SIZE(a) (((a) % ALIGNMENT_SIZE) ? ALIGNMENT_SIZE - ((a) % ALIGNMENT_SIZE) : 0)

#ifndef OFFSET_OF
#define OFFSET_OF(TYPE, Field) ((UINTN) &(((TYPE *)0)->Field))
#endif

//defined for every CPU architecture
// Maximum legal IA-32 address
#ifndef EFI_MAX_ADDRESS
#define EFI_MAX_ADDRESS   0xFFFFFFFF
#endif

//
//
// PKCS7 Certificate definition
//
typedef struct _WIN_CERTIFICATE_EFI_PKCS {
  WIN_CERTIFICATE Hdr;
  UINT8           CertData[1];
} WIN_CERTIFICATE_EFI_PKCS;

//
// Definitions for Efi System table entry to store the location within DB variable 
// of the Trusted Certificate that was used to verify Efi OS BootLoader image 
//
//6683D10C-CF6E-4914-B5B4-AB8ED7370ED7
#define AMI_VALID_BOOT_IMAGE_CERT_TBL_GUID \
    { 0x6683D10C, 0xCF6E, 0x4914, 0xB5, 0xB4, 0xAB, 0x8E, 0xD7, 0x37, 0x0E, 0xD7 }
//
//
// Data Table definition
//
typedef struct _AMI_VALID_CERT_IN_SIG_DB {
  UINT32          SigOffset;
  UINT32          SigLength;
} AMI_VALID_CERT_IN_SIG_DB;


/**
  Get the image type.

  @param[in]  File         This is a pointer to the device path of the file that is
                           being dispatched. This will optionally be used for logging.
  @param[in]  FileBuffer   A pointer to the buffer with the UEFI file image
  @param[in]  FileSize     The size of File buffer.
  @param[in]  BootPolicy   A boot policy that was used to call LoadImage() UEFI service.

  @return UINT32           Image Type

**/
UINT32
AmiGetImageType (
  IN  CONST EFI_DEVICE_PATH_PROTOCOL   *File,
  IN  VOID                             *FileBuffer,
  IN  UINTN                            FileSize,
  IN BOOLEAN                           BootPolicy
  );

#endif
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
