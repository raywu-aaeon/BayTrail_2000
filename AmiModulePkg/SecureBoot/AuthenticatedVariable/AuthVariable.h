//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
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
//
// Name:  AuthVariable.h Implement authentication services for the authenticated variable
//                       service in UEFI2.2+
//
// Description:
//
//<AMI_FHDR_END>
//**********************************************************************

#ifndef _AUTHVARIABLE_H_
#define _AUTHVARIABLE_H_

#include <AmiDxeLib.h>
#include <AmiCspLib.h>
#include "NVRAM/NVRAM.h"
#include <Protocol/AmiDigitalSignature.h>

#include <Efi.h>
// All are EDKII defined headers
//#include "WinCertificate.h"
#include "Guid/ImageAuthentication.h"
#include <Protocol/Hash.h>

#define EFI_SECURE_BOOT_NAME              L"SecureBoot"
#define SECURE_BOOT                       1
#define NONSECURE_BOOT                    0

#define HASH_SHA256_LEN                   sizeof(EFI_SHA256_HASH)     // 32
#define HASH_SHA1_LEN                     sizeof(EFI_SHA1_HASH)
#define RSA2048_PUB_KEY_LEN               DEFAULT_RSA_KEY_MODULUS_LEN // 256
#define EFI_CERT_TYPE_RSA2048_SHA256_SIZE RSA2048_PUB_KEY_LEN
#define EFI_CERT_TYPE_RSA2048_SIZE        RSA2048_PUB_KEY_LEN

///
/// Size of AuthInfo prior to the data payload
///
#define AUTHINFO_SIZE(Cert) (((UINTN)(((EFI_VARIABLE_AUTHENTICATION *) Cert)->AuthInfo.Hdr.dwLength)) + sizeof(UINT64))
#define AUTHINFO_2_SIZE(Cert) (((UINTN)(((EFI_VARIABLE_AUTHENTICATION_2 *) Cert)->AuthInfo.Hdr.dwLength)) + sizeof(EFI_TIME))

#ifdef EFI_DEBUG
#define AVAR_TRACE(Arguments) { if (!AVarRuntime) TRACE(Arguments); }
#else
#define AVAR_TRACE(Arguments)
#endif


typedef enum {
    IsPkVarType = 0,
    IsKekVarType,
    IsDbVarType,
    IsPrivateVarType
} AUTHVAR_TYPE;

VOID AuthVariableServiceInitSMM (VOID );
VOID AuthVariableServiceInit ( VOID );

EFI_STATUS VerifyVariable (
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32   *Attributes,
    IN VOID    **Data,
    IN UINTN    *DataSize, 
    IN VOID     *OldData,
    IN UINTN     OldDataSize,
    IN OUT EXT_SEC_FLAGS *ExtFlags
    );

EFI_STATUS FindInSignatureDb (
    IN EFI_GUID *VendorGuid,
    IN UINT32    Attributes,
    IN VOID     *Data,
    IN UINTN    *DataSize,
    IN VOID     *SigDB,
    IN UINTN     SigDBSize
    );

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Variable Auth Hdr EFI_VARIABLE_AUTHENTICATION
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EFI_STATUS VerifyVariable1 (
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32    Attributes,
    IN VOID    **Data,
    IN UINTN    *DataSize, 
    IN VOID     *OldData,
    IN UINTN     OldDataSize,
    IN OUT EXT_SEC_FLAGS *ExtFlags
    );

EFI_STATUS VerifyDataPayload (
    IN VOID     *Data,
    IN UINTN    DataSize, 
    IN UINT8    *PubKey
    );

EFI_STATUS ProcessVarWithPk (
    IN  VOID        *Data,
    IN  UINTN        DataSize,
    IN  UINT32       Attributes,
    IN  BOOLEAN      IsPk
    );

EFI_STATUS ProcessVarWithKek (
    IN  VOID        *Data,
    IN  UINTN        DataSize,
    IN  UINT32       Attributes
    );

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Variable Auth Hdr EFI_VARIABLE_AUTHENTICATION_2
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EFI_STATUS VerifyVariable2 (
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32    Attributes,
    IN VOID    **Data,
    IN UINTN    *DataSize, 
    IN VOID     *OldData,
    IN UINTN     OldDataSize,
    IN OUT EXT_SEC_FLAGS *ExtFlags
    );

EFI_STATUS ValidateSelfSigned (
    IN UINT8     *Pkcs7Cert,
    IN UINTN      Pkcs7Cert_len,
    IN OUT UINT8 **pDigest,
    IN OUT UINTN  *Digest_len,
    IN UINT8       Operation
    );

EFI_STATUS ConstructDataParameter (
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid,
    IN UINT32    Attributes,
    IN VOID     *Data,
    IN UINTN     DataSize, 
    OUT UINT8   *pDigest,
    OUT UINTN   *Digest_len,
    IN  UINT8    Mutex    
    );

EFI_STATUS ProcessVarWithPk2 (
    IN  UINT8     *Pkcs7Cert,
    IN  UINTN      Pkcs7Cert_len,
    IN  UINT8     *pDigest,
    IN  UINTN      Digest_len
    );

EFI_STATUS ProcessVarWithKek2 (
    IN  UINT8     *Pkcs7Cert,
    IN  UINTN      Pkcs7Cert_len,
    IN  UINT8     *pDigest,
    IN  UINTN      Digest_len
    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Misc auxilary functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

INTN StrCmp16(CHAR16 *Dest, CHAR16 *Src);
UINT32 StrSize16(CHAR16 *String);

BOOLEAN IsPkVar(
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid
    );

BOOLEAN IsKekVar(
    IN CHAR16   *VariableName,
    IN EFI_GUID *VendorGuid
    );

BOOLEAN IsDbVar(
    IN EFI_GUID *VendorGuid
    );

EFI_STATUS GetPlatformMode (
    VOID
    );

EFI_STATUS GetmSecureBootSupport (
    UINT8
    );

VOID  UpdatePlatformMode (
    IN  UINT8 Mode
    );

EFI_STATUS ValidateSignatureList (
    IN VOID     *Data,
    IN UINTN     DataSize
);

UINT64 mkLongTime ( 
    EFI_TIME *TimeStamp 
);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// NVRAM module defined auxilary functions
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

EFI_STATUS FindVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN OUT UINTN *DataSize, OUT VOID **Data
    );

EFI_STATUS DxeSetVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    IN UINT32 Attributes, IN UINTN DataSize, IN VOID *Data
);
EFI_STATUS DxeGetVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes OPTIONAL,
    IN OUT UINTN *DataSize, OUT VOID *Data
);

#endif  // _AUTHVARIABLE_H_
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
