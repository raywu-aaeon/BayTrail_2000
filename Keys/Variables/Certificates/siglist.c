#include <AmiCertificate.h>
#include <Protocol\AmiDigitalSignature.h>
#include <Guid\ImageAuthentication.h>
#include "siglist.h"
#include "OwnerGuid.h"
#pragma pack(1)
typedef struct {
    EFI_SIGNATURE_LIST   SigList;
    EFI_GUID             SigOwner;
} EFI_SIGNATURE_LIST_1;

EFI_SIGNATURE_LIST_1 SigList1 = {
    {
#if CertSize == 32
        EFI_CERT_SHA256_GUID,    // SigList.Type
#endif
#if CertSize == 48 //sizeof(EFI_CERT_X509_SHA256)
        EFI_CERT_X509_SHA256_GUID,
#endif
#if CertSize == 64 //sizeof(EFI_CERT_X509_SHA384)
        EFI_CERT_X509_SHA384_GUID,
#endif
#if CertSize == 80 //sizeof(EFI_CERT_X509_SHA512)
        EFI_CERT_X509_SHA512_GUID,
#endif
#if CertSize == 256
        EFI_CERT_RSA2048_GUID,
#endif
#if CertSize > 256
        EFI_CERT_X509_GUID,
#endif
        (UINT32)(sizeof(EFI_GUID)+CertSize)+sizeof(EFI_SIGNATURE_LIST),
         0,
        (UINT32)(sizeof(EFI_GUID)+CertSize)
    },
        SIGOWNER_GUID,
};
