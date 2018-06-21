//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
//  Name:           SecureBoot.h
//
//  Description:    Common Secure Boot definitions
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>
#ifndef _SECURE_BOOT_MODE_H_
#define _SECURE_BOOT_MODE_H_

#define AMI_SECURE_BOOT_SETUP_VAR  L"SecureBootSetup"
#define AMI_SECURE_VAR_PRESENT_VAR  L"SecureVarPresent"

#pragma pack(1)
typedef struct{
    UINT8 SecureBootSupport;
    UINT8 SecureBootMode;
    UINT8 DefaultKeyProvision;
    UINT8 BackDoorVendorKeyChange;
    UINT8 Load_from_OROM;
    UINT8 Load_from_REMOVABLE_MEDIA;
    UINT8 Load_from_FIXED_MEDIA;
} SECURE_BOOT_SETUP_VAR;

typedef struct{
    UINT8 Value;
} SETUP_MODE_VAR;

typedef struct{
    UINT8 Value;
} SECURE_BOOT_VAR;

typedef struct{
    UINT8 DBX;
    UINT8 DBT;
    UINT8 DB;
    UINT8 KEK;
    UINT8 PK;
} SECURE_VAR_INSTALL_VAR;

typedef struct{
    UINT8 Value;
} SECURE_BOOT_VENDOR_KEY_VAR;

#pragma pack()

// EFI_IMAGE_SECURITY_DATABASE_DEFAULT must be defined in ImageAuthentication.h
// UEFI ECR874: Install Factory defaults as Read-only volatile variables for key distribution.
#ifndef EFI_IMAGE_SECURITY_DATABASE_DEFAULT
#define EFI_IMAGE_SECURITY_DATABASE_DEFAULT  L"dbDefault"
#define EFI_IMAGE_SECURITY_DATABASE1_DEFAULT L"dbxDefault"
#define EFI_PLATFORM_KEY_NAME_DEFAULT        L"PKDefault"
#define EFI_KEY_EXCHANGE_KEY_NAME_DEFAULT    L"KEKDefault"
#define EFI_IMAGE_SECURITY_DATABASE2_DEFAULT L"dbtDefault"
#endif
#ifndef EFI_IMAGE_SECURITY_DATABASE2
#define EFI_IMAGE_SECURITY_DATABASE2 L"dbt"
#define EFI_IMAGE_SECURITY_DATABASE2_DEFAULT L"dbtDefault"
#endif
#ifndef EFI_VENDOR_KEYS_NAME
#define EFI_VENDOR_KEYS_NAME L"VendorKeys"
#endif
#define SIGSUPPORT_NUM 8 
#define SIGSUPPORT_LIST EFI_CERT_X509_SHA256_GUID, EFI_CERT_X509_SHA384_GUID, EFI_CERT_X509_SHA512_GUID, EFI_CERT_SHA256_GUID, EFI_CERT_X509_GUID, EFI_CERT_RSA2048_GUID, EFI_CERT_RSA2048_SHA256_GUID, EFI_CERT_RSA2048_SHA1_GUID

#endif //_SECURE_BOOT_MODE_H_
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
