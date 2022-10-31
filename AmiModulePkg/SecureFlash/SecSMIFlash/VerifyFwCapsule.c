//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:        VerifyFwCapsule.c
//
// Description:    Verify Aptio FW capsule integrity and performs other security checks
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#include <AmiDxeLib.h>
#include <Protocol/SmiFlash.h>
#include <Protocol/SecSmiFlash.h>
#include <Ppi/FwVersion.h>
#include <RomLayout.h>
#include <Ffs.h>
#include "AmiCertificate.h"

//----------------------------------------------------------------------------
// Function Externs
extern AMI_DIGITAL_SIGNATURE_PROTOCOL *gAmiSig;
extern UINTN     gRomFileSize; // add 4k for capsule's header
extern EFI_GUID  gFWCapsuleGuid;
extern EFI_GUID  gPRKeyGuid;
extern EFI_GUID  gFwCapFfsGuid;
extern EFI_SHA256_HASH  *gHashTbl;
extern UINT8     gHashDB[SHA256_DIGEST_SIZE];

BOOLEAN IsAddressInSmram (
    IN EFI_PHYSICAL_ADDRESS  Buffer,
    IN UINT64                Length
);
//----------------------------------------------------------------------------
// Local prototypes
EFI_STATUS CapsuleValidate (
    IN OUT UINT8     **pFwCapsule,
    IN OUT APTIO_FW_CAPSULE_HEADER     **pFWCapsuleHdr
);

typedef struct {
    EFI_FFS_FILE_HEADER FfsHdr;
// test
    EFI_COMMON_SECTION_HEADER SecHdr;
    EFI_GUID            SectionGuid;
// end test
    UINT8                FwCapHdr[0];
} AMI_FFS_COMMON_SECTION_HEADER;

//----------------------------------------------------------------------------
static EFI_GUID FwCapSectionGuid = AMI_FW_CAPSULE_SECTION_GUID;
#if IGNORE_RUNTIME_UPDATE_IMAGE_REVISION_CHECK == 0
//----------------------------------------------------------------------------
typedef struct _FID_SECTION {
    EFI_GUID   Guid;
    FW_VERSION FwVersion;
} FID_SECTION;

static EFI_GUID FidSectionGuid = \
    { 0x2EBE0275, 0x6458, 0x4AF9, 0x91, 0xed, 0xD3, 0xF4, 0xED, 0xB1, 0x00, 0xAA };

const UINT8 *FidSignature = "$FID";
//----------------------------------------------------------------------------
// Function Definitions

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   VerifyProjectId
//
// Description: 
//
// Input:       
//
// Output:      
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN 
VerifyProjectId (
    IN FW_VERSION      *FwVersionData
)
{
    char  *strProjectId = CONVERT_TO_STRING(PROJECT_TAG);
    UINTN  Size = sizeof(CONVERT_TO_STRING(PROJECT_TAG));
/*
CHAR8       BiosTag[9];     //BIOS Tag
EFI_GUID    FirmwareGuid;       //Firmware GUID
CHAR8       CoreMajorVersion[3];    
CHAR8       CoreMinorVersion[3];
CHAR8       ProjectMajorVersion[3];
CHAR8       ProjectMinorVersion[3];
*/
// Project ID, Major, Minor rev    
    if (Size==0 || MemCmp (FwVersionData->BiosTag, strProjectId, Size-1)) return FALSE;
#if IGNORE_IMAGE_ROLLBACK == 0
    if(Atoi(FwVersionData->ProjectMajorVersion) < PROJECT_MAJOR_VERSION) return FALSE;
    if(Atoi(FwVersionData->ProjectMinorVersion) < PROJECT_MINOR_VERSION) return FALSE;
#endif

    return TRUE;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetFidData
//
// Description: Function to read FFS FID data structure from the given data buffer
//
// Input:       OUT FW_VERSION **Fid - pointer to output buffer
//              IN  VOID *pFV - pointer to data buffer to read from

//
// Output:      EFI_SUCCESS if FID data is retrieved
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN GetFidData(
    IN  VOID           *pFV,
    IN  UINT32          Size,
    OUT FW_VERSION    **FwVersionData
)
{
//    UINT32 Signature;
    UINT8 *SearchPointer;
    FID_SECTION *Section;

// Simplified search by $FID signature only.
//    SearchPointer = (UINT32 *)((UINT8 *)pFV + sizeof(EFI_GUID));
//    Signature = FidSectionGuid.Data1;
    SearchPointer = (UINT8 *)pFV;

    do {
//        if(*SearchPointer == Signature) {
            Section = (FID_SECTION *)SearchPointer;
            if(!guidcmp(&FidSectionGuid, &(Section->Guid)) && 
               (*((UINT32*)(&Section->FwVersion.FirmwareID[0])) == *(UINT32*)FidSignature)){ 
                *FwVersionData = &Section->FwVersion;
                return TRUE;
            }
//        }
    } while( SearchPointer++ < (UINT8*)((UINT32)pFV+Size));

    return FALSE;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   VerifyFwRevision
//
// Description: Verify Fw revision compatibility
//              NewVer > OldVer, newProjectTAGid = oldProjectTAGid
//
// Input:
//      IN OUT UINT8 *pCapsule
// Output:
//      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
VerifyFwRevision (
    IN APTIO_FW_CAPSULE_HEADER  *FWCapsuleHdr,
    IN UINT8 *RomData    
)    
{
    ROM_AREA                *Area;
    EFI_PHYSICAL_ADDRESS    FvAddress;
    FW_VERSION             *FwVersionData;

    Area = (ROM_AREA *)(UINTN)((UINT32)FWCapsuleHdr+FWCapsuleHdr->RomLayoutOffset);

    for (Area; Area->Size != 0; Area++) {
        if (!(Area->Attributes & ROM_AREA_FV_SIGNED)) 
            continue;
        //  $FID can be in FV with either PEI or DXE
        if (!(Area->Attributes & (ROM_AREA_FV_PEI+ROM_AREA_FV_DXE))) 
            continue;

        FvAddress = (EFI_PHYSICAL_ADDRESS)RomData + (Area->Offset);
        if (GetFidData((UINT8*)FvAddress, Area->Size, &FwVersionData)) {
            if(VerifyProjectId(FwVersionData))
                return EFI_SUCCESS;
             break;
        }
    } 
// At least one FW block must be signed OR no $FID structure found in the new FW image
    return EFI_SECURITY_VIOLATION;
}

#endif // #if IGNORE_RUNTIME_UPDATE_IMAGE_REVISION_CHECK == 0

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FindCapHdrFFS
//
// Description: Function to read FW Cap Sig data from Ffs
//
// Input:       OUT UINT8 **pFwCapHdr - pointer to output buffer
//              IN  VOID *pCapsule - pointer to data buffer to read from
//
// Output:      EFI_SUCCESS if Capsule Hdr with Signature is retrieved
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS FindCapHdrFFS(
    IN  VOID    *pCapsule,
    OUT UINT8 **pFfsData
)
{
    UINT32 Signature;
    UINT32 *SearchPointer;
    AMI_FFS_COMMON_SECTION_HEADER *FileSection;
    APTIO_FW_CAPSULE_HEADER *pFwCapHdr;

    SearchPointer = (UINT32 *)((UINT8 *)pCapsule - sizeof(AMI_FFS_COMMON_SECTION_HEADER) + FLASH_SIZE);
    Signature = gFwCapFfsGuid.Data1;
    do {
        if(*SearchPointer == Signature) {
            FileSection = (AMI_FFS_COMMON_SECTION_HEADER *)SearchPointer;
            if(!guidcmp(&gFwCapFfsGuid, &(FileSection->FfsHdr.Name))
               && !guidcmp(&FwCapSectionGuid, &(FileSection->SectionGuid))
            ){
                pFwCapHdr = (APTIO_FW_CAPSULE_HEADER*)(FileSection->FwCapHdr);
                // just a sanity check - Cap Size must match the Section size
                if(((*(UINT32 *)FileSection->FfsHdr.Size) & 0xffffff) >=
                        pFwCapHdr->CapHdr.HeaderSize + sizeof(AMI_FFS_COMMON_SECTION_HEADER) &&
                    !guidcmp((EFI_GUID*)&pFwCapHdr->CapHdr.CapsuleGuid, &gFWCapsuleGuid)
                ){
                    *pFfsData = (UINT8*)pFwCapHdr;
                        return EFI_SUCCESS;
                }    
            }
        }
    } while(SearchPointer-- != pCapsule);

    return EFI_NOT_FOUND;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    HashFwRomMapImage
//
// Description:    The Rom image hash is calculated based on info from the Rom Area map
//
// Input:        
//          Payload - pointer to a FW Image
//          FwCapsuleHdr - pointer to a FW Capsule Hdr
//          RomSize - Size of Rom Image
//
// Output:      EFI_SUCCESS - capsule processed successfully
//              EFI_DEVICE_ERROR - capsule processing failed
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS HashFwRomMapImage (
    IN APTIO_FW_CAPSULE_HEADER  *FWCapsuleHdr,
    IN UINT8                    *Payload,
    IN UINTN                     RomSize,
    OUT UINT8                   *gHashDB
){
    EFI_STATUS              Status = EFI_SUCCESS;
    ROM_AREA                *RomAreaTbl;
    UINTN                   *Addr;
    UINTN                   *Len;
    UINTN    i, RomMap_size, max_num_elem, num_elem, max_hash_elem;

    RomAreaTbl = (ROM_AREA *)(UINTN)((UINT32)FWCapsuleHdr+FWCapsuleHdr->RomLayoutOffset);

    RomMap_size = FWCapsuleHdr->RomImageOffset-FWCapsuleHdr->RomLayoutOffset;
    max_num_elem = RomMap_size/sizeof(ROM_AREA);
// assume max size of RomMap array = RomMap_size/sizeof(ROM_AREA);
// or better yet ...calculate exact number
    num_elem = 0;
    for (i=0; i < max_num_elem, RomAreaTbl[i].Size != 0; i++ )
    {
        if (RomAreaTbl[i].Attributes & ROM_AREA_FV_SIGNED)
            num_elem++;
    }
    max_num_elem  = i;
    max_hash_elem = num_elem+2; // add 2 extra entries
    Addr = (UINTN*)gHashTbl;
    Len = (UINTN*)((UINT8*)gHashTbl + max_hash_elem*sizeof(UINTN));

    num_elem = 0;
    for(i=0; i < max_num_elem, num_elem < max_hash_elem, RomAreaTbl[i].Size != 0; i++)
    {
        if (!(RomAreaTbl[i].Attributes & ROM_AREA_FV_SIGNED)) 
            continue;
        if(RomAreaTbl[i].Offset > RomSize ||
           (UINT64)RomAreaTbl[i].Offset + RomAreaTbl[i].Size > RomSize)
            return EFI_SECURITY_VIOLATION;
    // RomArea only holds offsets within a payload
        Addr[num_elem] = (UINTN)((UINTN)Payload + RomAreaTbl[i].Offset);
        Len[num_elem] = RomAreaTbl[i].Size;

        num_elem++;

    }
    if(num_elem >= max_hash_elem) return EFI_SECURITY_VIOLATION;
//
//  Hash of Capsule Hdr + FW Certificate Hdr
//
    if(FWCapsuleHdr->CapHdr.Flags & CAPSULE_FLAGS_CAPHDR_IN_SIGNCERT) {
        Addr[num_elem] = (UINTN) FWCapsuleHdr;
        Len[num_elem] = (UINTN)&FWCapsuleHdr->FWCert.SignCert.CertData - (UINTN)FWCapsuleHdr;
        num_elem++;
        if(num_elem >= max_hash_elem) return EFI_SECURITY_VIOLATION;
    }
//
//  Hash of the ROM_MAP table
//
    Addr[num_elem] = (UINTN)RomAreaTbl;
    Len[num_elem] = (i+1)*sizeof(ROM_AREA);
    num_elem++;

    Status = gAmiSig->Hash(gAmiSig, &gEfiHashAlgorithmSha256Guid, 
                num_elem, (const UINT8**)Addr,  (const UINTN*)Len, gHashDB );

//    TRACE((-1, "Hash the FW Image %r\n", Status));

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    VerifyFwCertPkcs7
//
// Description:    This code verifies FW Capsule is genuine, 
//                 and performs following checks on the image:
//                  1. Signing certificate is signed with trusted Root Platform key
//                  2. Integrity check. Image Signature verification
//
// Input:        
//          Payload - pointer to a FW Image
//          FwCapsuleHdr - pointer to a FW Capsule Hdr
//          RomSize - Size of Rom Image
//
// Output:      EFI_SUCCESS - capsule processed successfully
//              EFI_DEVICE_ERROR - capsule processing failed
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS VerifyFwCertPkcs7 (
    IN APTIO_FW_CAPSULE_HEADER  *FWCapsuleHdr,
    IN UINT8                    *Payload,
    IN UINTN                     RomSize
){
    EFI_STATUS              Status;
    CRYPT_HANDLE            PubKeyHndl;
    UINT8                  *Pkcs7Cert, *pDigest;
    UINTN                   Pkcs7Cert_len, DigestLen;
    UINT32                  Flags=0;

//
// 1. Get ptr to a Fw Platform Root Key
//
    PubKeyHndl.AlgGuid = gEfiCertX509Guid;
    PubKeyHndl.BlobSize =  0;
    PubKeyHndl.Blob = NULL;

    Status = gAmiSig->GetKey(gAmiSig, &PubKeyHndl, &gPRKeyGuid, 0, Flags);
//    TRACE((-1, "Get x509 Root Cert Key == FW Key : %r\n", Status));
    if(EFI_ERROR(Status) || PubKeyHndl.Blob==NULL)
        return EFI_SECURITY_VIOLATION; 

    if(guidcmp(&PubKeyHndl.AlgGuid, &gEfiCertX509Guid))
        return EFI_UNSUPPORTED;

// 2. Verify Signing Cert Signature
//
// 2.1 The Rom image hash is calculated based on info from the Rom Area map
//
    Status = HashFwRomMapImage(FWCapsuleHdr, Payload, RomSize, gHashDB);
    if (EFI_ERROR(Status)) return Status;

// 2.2 Verify Fw Certificate
    pDigest = &gHashDB[0];
    DigestLen = SHA256_DIGEST_SIZE;
    Pkcs7Cert = (UINT8*)&FWCapsuleHdr->FWCert.SignCert.CertData; 
    Pkcs7Cert_len = FWCapsuleHdr->FWCert.SignCert.Hdr.Hdr.dwLength-sizeof(WIN_CERTIFICATE_UEFI_GUID_1);
    Status = gAmiSig->Pkcs7Verify( gAmiSig,
                     Pkcs7Cert, Pkcs7Cert_len,           // Pkcs7Cert
                     PubKeyHndl.Blob, PubKeyHndl.BlobSize, // TrustCert
                     &pDigest, &DigestLen,               // In/OutData
                     Pkcs7CertValidate,
                     RELEASE                             // Flags, mutex
                     );
//    TRACE((-1, "Verify Fw Pkcs7 Cert %r\n", Status));

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    VerifyFwCertRsa2048Sha256
//
// Description:    This code verifies FW Capsule is genuine, 
//                 and performs following checks on the image:
//                  1. Signing certificate is signed with trusted Root Platform key
//                  2. Integrity check. Image Signature verification
//
// Input:        
//          Payload - pointer to a FW Image
//          FwCapsuleHdr - pointer to a FW Capsule Hdr
//          RomSize - Size of Rom Image
//
// Output:      EFI_SUCCESS - capsule processed successfully
//              EFI_DEVICE_ERROR - capsule processing failed
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS VerifyFwCertRsa2048Sha256 (
    IN APTIO_FW_CAPSULE_HEADER  *FWCapsuleHdr,
    IN UINT8                    *Payload,
    IN UINTN                     RomSize
){
    EFI_STATUS              Status;
    CRYPT_HANDLE            HashHndl;
    CRYPT_HANDLE            PubKeyHndl;
    UINT8                   *pSig;
    UINT32                  Flags;
    UINT8                   *Addr;
    UINTN                   Size;
    EFI_CERT_BLOCK_RSA_2048_SHA256* pRootCert;

    Flags = (FWCapsuleHdr->CapHdr.Flags & CAPSULE_FLAGS_RSA_PSS_PADDING_SCHEME)?EFI_CRYPT_RSASSA_PSS:EFI_CRYPT_RSASSA_PKCS1V15;
    
    HashHndl.AlgGuid = gEfiHashAlgorithmSha256Guid;
    HashHndl.BlobSize =  SHA256_DIGEST_SIZE;
    HashHndl.Blob = gHashDB;

    PubKeyHndl.AlgGuid = gEfiCertRsa2048Guid;
    PubKeyHndl.BlobSize =  DEFAULT_RSA_KEY_MODULUS_LEN;
//
// 1. Compare Capsule's Sign Cert key with Platform Root Key
//
    PubKeyHndl.Blob = (UINT8*)FWCapsuleHdr->FWCert.SignCert.CertData.PublicKey;
    Status = gAmiSig->VerifyKey(gAmiSig, &gPRKeyGuid, &PubKeyHndl);
    //  Skip the RootCert key checking if SignCert Key and PR Key are a Match
    TRACE((-1, "Compare SignCert Key == FW Key : %r\n", Status));
    if(EFI_ERROR(Status)) {
//
// 1.1 Compare Platform Root with Capsule's Key from a Root Key store
//
        for (pRootCert = &FWCapsuleHdr->FWCert.RootCert; 
            (UINT8*)pRootCert < 
                (UINT8*)&FWCapsuleHdr->FWCert+FWCapsuleHdr->FWCert.SignCert.Hdr.Hdr.dwLength, 
                pRootCert->PublicKey[0]!=0;
            pRootCert++) 
        {
            PubKeyHndl.Blob = pRootCert->PublicKey;
            Status = gAmiSig->VerifyKey(gAmiSig, &gPRKeyGuid, &PubKeyHndl);
//TRACE((-1, "Compare RootCert Key == FW Key(%X) : %r\n", (UINT32)*PubKeyHndl.Blob, Status));
            if (EFI_ERROR(Status)) continue;
//
// 2. Verify RootCert.Signature
//
// 2.1  Compute FWCert.SignCert.PublicKey Hash
            if(FWCapsuleHdr->CapHdr.Flags & CAPSULE_FLAGS_SIGNKEY_IN_ROOTCERT)
            {
                Addr = (UINT8*)&FWCapsuleHdr->FWCert.SignCert.CertData.PublicKey;
                Size = DEFAULT_RSA_KEY_MODULUS_LEN;
            } else
// 2.2  Compute FWCert.SignCert Hash
            {
                Addr = (UINT8*)&FWCapsuleHdr->FWCert.SignCert;
                Size = sizeof(AMI_CERTIFICATE_RSA2048_SHA256);
            }

            Status = gAmiSig->Hash(gAmiSig, &gEfiHashAlgorithmSha256Guid, 1,&Addr,(const UINTN*)&Size, gHashDB); 
            if (EFI_ERROR(Status)) break;
        
            pSig = (void*)pRootCert->Signature; 
            Status = gAmiSig->Pkcs1Verify(gAmiSig, &PubKeyHndl, &HashHndl, pSig, DEFAULT_RSA_SIG_LEN, Flags); 
//TRACE((-1, "Verify Root Cert : %r\n", Status));
            break;
        }
    }
    if (EFI_ERROR(Status)) return EFI_SECURITY_VIOLATION;
// 3. Verify Signing Cert Signature
//
// 3.1 The Rom image hash is calculated based on info from the Rom Area map
//
    Status = HashFwRomMapImage(FWCapsuleHdr, Payload, RomSize, gHashDB);
    if (EFI_ERROR(Status)) return Status;

    pSig = (void*)FWCapsuleHdr->FWCert.SignCert.CertData.Signature; 
    PubKeyHndl.Blob = (UINT8*)FWCapsuleHdr->FWCert.SignCert.CertData.PublicKey;
    Status = gAmiSig->Pkcs1Verify(gAmiSig, &PubKeyHndl, &HashHndl, pSig, DEFAULT_RSA_SIG_LEN, Flags); 
//    TRACE((-1, "Verify Sign Cert %r\n", Status));

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    CapsuleValidate
//
// Description:    This code verifies FW Capsule is genuine, 
//                 and performs following checks on the image:
//                  1. Play-back protection. Verifies that new FW image verson is newer then the current one 
//                  2. Signing certificate is signed with trusted Root Platform key
//                  3. Integrity check. Image Signature verification
//
// Input:        
//  IN VOID *pFwCapsule - pointer to a FW Image
//  OUT UINT8 *pFwCapsuleHdr - pointer to a FW Capsule Hdr
//
// Output:      EFI_SUCCESS - capsule processed successfully
//              EFI_DEVICE_ERROR - capsule processing failed
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CapsuleValidate (
    IN OUT UINT8     **pFwCapsule,
    IN OUT APTIO_FW_CAPSULE_HEADER     **pFWCapsuleHdr
){
    EFI_STATUS              Status = EFI_DEVICE_ERROR;
    APTIO_FW_CAPSULE_HEADER     *FWCapsuleHdr;
    UINTN                   RomSize;
    UINT8                   *Payload;

    FWCapsuleHdr = (APTIO_FW_CAPSULE_HEADER*)*pFwCapsule;
    Payload = (UINT8*)*pFwCapsule;
    RomSize = FLASH_SIZE;

#if NEW_BIOS_MEM_ALLOC != 0
    if(IsAddressInSmram((EFI_PHYSICAL_ADDRESS)FWCapsuleHdr, sizeof(EFI_PHYSICAL_ADDRESS)))
        return EFI_DEVICE_ERROR;
#endif

/*
 - CapsuleValidate 
     - Look up Capsule GUID
         - Found - 
            update pFwCapsule ptr to beginning of BIOS ROM data
            continue with Image Verify
         - Not found at offs 0 - assume Cap Hdr in FFS
            Call GetSigFFS
            locate FFS by Hole GUID, Sec GUID
             if found, update FWCapsuleHdr,
            continue with Image Verify
*/
// proper FW Capsule presence check

// verify Capsule Mailbox points to FW_CAPSULE hdr
    if(!guidcmp((EFI_GUID*)&FWCapsuleHdr->CapHdr.CapsuleGuid, &gFWCapsuleGuid))
    {
// Update pFwCapsule to point to beginning of Bios ROM
        Payload = (UINT8*)((UINT32)FWCapsuleHdr + FWCapsuleHdr->RomImageOffset);
        RomSize = (FWCapsuleHdr->CapHdr.CapsuleImageSize - FWCapsuleHdr->RomImageOffset);
        *pFwCapsule = Payload;
    }
    else
    {
//        TRACE((-1, "Looking for embedded Signature...\n"));
        if(EFI_ERROR(FindCapHdrFFS(Payload, (UINT8**)&FWCapsuleHdr)))
            return EFI_SECURITY_VIOLATION;
    }

    *pFWCapsuleHdr = FWCapsuleHdr;
//    TRACE((-1, "Fw Capsule GUID %g\n\r", &(FWCapsuleHdr->CapHdr.CapsuleGuid)));

// Aptio FW Capsule only supporting WIN_CERT_TYPE_EFI_GUID 
    if(FWCapsuleHdr->FWCert.SignCert.Hdr.Hdr.wCertificateType != WIN_CERT_TYPE_EFI_GUID)
        return EFI_SECURITY_VIOLATION;

// sanity check for buffer overruns
    if((FWCapsuleHdr->CapHdr.CapsuleImageSize > gRomFileSize) ||
       (FWCapsuleHdr->RomImageOffset > 0x8000/*FWCAPSULE_MAX_HDR_SIZE*/) || // 32k is a MAX possible FwCap Hdr size
       (FWCapsuleHdr->CapHdr.HeaderSize > FWCapsuleHdr->RomImageOffset) ||
       (FWCapsuleHdr->RomLayoutOffset > FWCapsuleHdr->RomImageOffset) ||
       (FWCapsuleHdr->FWCert.SignCert.Hdr.Hdr.dwLength + offsetof(APTIO_FW_CAPSULE_HEADER, FWCert) >
        FWCapsuleHdr->RomLayoutOffset )
    )
        return EFI_SECURITY_VIOLATION;

// Begin Authentication
    if(!guidcmp((EFI_GUID*)&FWCapsuleHdr->FWCert.SignCert.Hdr.CertType, &gEfiCertPkcs7Guid))
        Status = VerifyFwCertPkcs7(FWCapsuleHdr, Payload, RomSize);
    else
        Status = VerifyFwCertRsa2048Sha256(FWCapsuleHdr, Payload, RomSize);

    if (EFI_ERROR(Status)) 
        return Status;
// 
// Local PEI $FID is linked with CspLib. extern FW_VERSION FwVersionData;
// Find $FID in new Fw FVs. Any found should do for us. Use RomMap from Capsule's Hdr
// compare local BB and Main $Fid BIOS Major/Minor revs with New one.
#if IGNORE_RUNTIME_UPDATE_IMAGE_REVISION_CHECK == 0
    Status = VerifyFwRevision(FWCapsuleHdr, Payload);
//    TRACE((-1, "FW Revision test %r\n", Status));
#endif
    
//    TRACE((-1, "Capsule Verify %r\n", Status));

    return Status;
}

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
