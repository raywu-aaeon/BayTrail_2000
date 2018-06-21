//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2015, American Megatrends, Inc.         **
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

#include <Token.h>
#include <AmiDxeLib.h>
#include <AmiCertificate.h>
#include <Protocol/AmiDigitalSignature.h>

#include <Setup.h>
#include <SecureBoot.h>
#include "TimeStamp.h"

#if !defined(NO_SETUP_COMPILE) && !defined(SMIFLASH_COMPILE)

#include <AutoId.h>

#include <Protocol/AMIPostMgr.h> 
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/HiiString.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LegacyBios.h>
#include <Guid/GlobalVariable.h>

#endif

extern EFI_RUNTIME_SERVICES *pRS;

static EFI_GUID PKeyFileGuid  = //CC0F8A3F-3DEA-4376-9679-5426BA0A907E
{ 0xCC0F8A3F, 0x3DEA,  0x4376, 0x96, 0x79, 0x54, 0x26, 0xba, 0x0a, 0x90, 0x7e };
static EFI_GUID KekFileGuid  =  // {9FE7DE69-0AEA-470a-B50A-139813649189}
{ 0x9fe7de69, 0xaea, 0x470a, 0xb5, 0xa, 0x13, 0x98, 0x13, 0x64, 0x91, 0x89 };
static EFI_GUID DbFileGuid  =  // {FBF95065-427F-47b3-8077-D13C60710998}
{ 0xfbf95065, 0x427f, 0x47b3, 0x80, 0x77, 0xd1, 0x3c, 0x60, 0x71, 0x9, 0x98 };
static EFI_GUID DbxFileGuid  =  // {9D7A05E9-F740-44c3-858B-75586A8F9C8E}
{ 0x9d7a05e9, 0xf740, 0x44c3, 0x85, 0x8b, 0x75, 0x58, 0x6a, 0x8f, 0x9c, 0x8e };
static EFI_GUID DbtFileGuid  =  // {C246FBBF-F75C-43F7-88A6-B5FD0CF1DB7F}
{ 0xC246FBBF, 0xF75C, 0x43F7, 0x88, 0xa6, 0xb5, 0xfd, 0x0c, 0xf1, 0xdb, 0x7f };

static EFI_GUID *SecureVariableFileGuid [] = {
    &DbxFileGuid,
    &DbtFileGuid,
    &DbFileGuid,
    &KekFileGuid,
    &PKeyFileGuid,
    NULL
};

static CHAR16* SecureVariableFileName[] = {
    EFI_IMAGE_SECURITY_DATABASE1,
    EFI_IMAGE_SECURITY_DATABASE2,
    EFI_IMAGE_SECURITY_DATABASE,
    EFI_KEY_EXCHANGE_KEY_NAME,
    EFI_PLATFORM_KEY_NAME,
    NULL
};

static CHAR16* SecureVariableFileNameDefault[] = {
    EFI_IMAGE_SECURITY_DATABASE1_DEFAULT,
    EFI_IMAGE_SECURITY_DATABASE2_DEFAULT,
    EFI_IMAGE_SECURITY_DATABASE_DEFAULT,
    EFI_KEY_EXCHANGE_KEY_NAME_DEFAULT,
    EFI_PLATFORM_KEY_NAME_DEFAULT,
    NULL
};

static SECURE_BOOT_SETUP_VAR SecureBootSetup = {
    DEFAULT_SECURE_BOOT_ENABLE, 
    DEFAULT_SECURE_BOOT_MODE, 
    DEFAULT_PROVISION_SECURE_VARS,
    0x0, // former LOAD_FROM_FV - now used to store BackDoorEvent Value
    LOAD_FROM_OROM,
    LOAD_FROM_REMOVABLE_MEDIA,
    LOAD_FROM_FIXED_MEDIA};

static EFI_GUID guidSecurity = SECURITY_FORM_SET_GUID;
static UINT8 bKey[5] = {0, 0, 0, 0, 0}; // 1 - custom, 2 - vendor, 3 - vendor+custom
static UINT8 mBackDoorVendorKeyChange = 0; // bit mask for each Secure Vendor Key, e.g PK bit4, KEK - bit3 etc.

typedef enum { RESET_NV_KEYS=1, SET_NV_DEFAULT_KEYS=2, SET_RT_DEFAULT_KEYS=4};

// 
//
// AMI_EFI_VARIABLE_AUTHENTICATION_2 descriptor
//
// A time-based authentication method descriptor template
//
#pragma pack(1)
#ifndef AMI_EFI_VARIABLE_AUTHENTICATION_2
typedef struct {
    EFI_TIME                            TimeStamp;
    WIN_CERTIFICATE_UEFI_GUID_1         AuthInfo;
} AMI_EFI_VARIABLE_AUTHENTICATION_2;
#endif
typedef struct {
    AMI_EFI_VARIABLE_AUTHENTICATION_2   AuthHdr;
    EFI_SIGNATURE_LIST                  SigList;
    EFI_SIGNATURE_DATA                  SigData;
} EFI_VARIABLE_SIG_HDR_2;
#pragma pack()

#define EFI_CERT_TYPE_RSA2048_SIZE        256
#define EFI_CERT_TYPE_SHA256_SIZE         32
#define EFI_CERT_TYPE_CERT_X509_SHA256_GUID_SIZE        48
#define EFI_CERT_TYPE_CERT_X509_SHA384_GUID_SIZE        64
#define EFI_CERT_TYPE_CERT_X509_SHA512_GUID_SIZE        80

// InstallVars
#define SET_SECURE_VARIABLE_DEL 1
#define SET_SECURE_VARIABLE_SET 2
#define SET_SECURE_VARIABLE_APPEND 4

#define StrMaxSize 200
static CHAR16 StrTitle[StrMaxSize], StrMessage[StrMaxSize];
static CHAR16 StrTemp[StrMaxSize];
//
#if !defined( TSE_FOR_APTIO_4_50) && !defined(NO_SETUP_COMPILE) && !defined(SMIFLASH_COMPILE)
//
static AMI_POST_MANAGER_PROTOCOL *mPostMgr = NULL;

typedef struct
{
    UINT64 Type;
    UINTN Size;
    CHAR16 *Name;
    STRING_REF Token;
} FILE_TYPE;

static EFI_HII_STRING_PROTOCOL *HiiString = NULL;
static EFI_HII_HANDLE gHiiHandle;

static STRING_REF SecureVariableStringTitle[] = {
    STRING_TOKEN(STR_DBX_TITLE),
    STRING_TOKEN(STR_DBT_TITLE),
    STRING_TOKEN(STR_DB_TITLE),
    STRING_TOKEN(STR_KEK_TITLE),
    STRING_TOKEN(STR_PK_TITLE),
    0
};
static STRING_REF SecureVariableStringRef[] = {
    STRING_TOKEN(STR_DBX_TITLE_TEXT),
    STRING_TOKEN(STR_DBT_TITLE_TEXT),
    STRING_TOKEN(STR_DB_TITLE_TEXT),
    STRING_TOKEN(STR_KEK_TITLE_TEXT),
    STRING_TOKEN(STR_PK_TITLE_TEXT),
    0
};
EFI_STATUS DevicePathToStr(EFI_DEVICE_PATH_PROTOCOL *Path,CHAR8    **Str);
EFI_STATUS FileBrowserLaunchFileSystem(BOOLEAN bSelectFile, OUT EFI_HANDLE **outFsHandle, OUT CHAR16 **outFilePath, OUT UINT8 **outFileBuf,OUT UINTN *size );
VOID GetHiiString(IN EFI_HII_HANDLE HiiHandle, IN STRING_REF Token, UINTN DataSize, CHAR16  * pData);
VOID SetAppendSecureBootDBCallback(EFI_HII_HANDLE HiiHandle, UINT8 VarSetMode, UINT8 index);
VOID DeleteSecureBootDBCallback(EFI_HII_HANDLE HiiHandle, UINT8 index);
#endif // no Setup link

#ifdef TSE_FOR_APTIO_4_50
extern BOOLEAN gBrowserCallbackEnabled;
#else
BOOLEAN gBrowserCallbackEnabled = FALSE;
#endif //#ifndef TSE_FOR_APTIO_4_50

//----------------------------------------------------------------------------
// Function forward declaration
//----------------------------------------------------------------------------
VOID UpdateSecureVariableBrowserInfo();
VOID UpdateSecureBootBrowserInfo ();
EFI_STATUS InstallSecureVariables (UINT16);
VOID UpdateSecureBootSetupVariable(UINT8);

EFI_STATUS ValidateSignatureList (
    IN VOID         *Data,
    IN UINTN        DataSize,
    IN OUT UINTN    *SigCount OPTIONAL
    );

//----------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateSecureBootBrowserInfo
//
// Description: Update Secure Boot flags status
//
// Input:       none
//
// Output:      none
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID UpdateSecureBootBrowserInfo ()
{
    UINT8       Byte;
    UINTN       Size;
    BOOLEAN     tmpBrowserCallbackEnabled = gBrowserCallbackEnabled;

    gBrowserCallbackEnabled = TRUE;
    Size = sizeof(bKey);
    pRS->GetVariable(AMI_SECURE_VAR_PRESENT_VAR, &guidSecurity, NULL, &Size, bKey);
    HiiLibSetBrowserData( sizeof(bKey), &bKey, &guidSecurity, AMI_SECURE_VAR_PRESENT_VAR);
    Size = 1;
    pRS->GetVariable(EFI_SECURE_BOOT_MODE_NAME, &gEfiGlobalVariableGuid, NULL, &Size, &Byte);
    TRACE((-1,"%S=%x\n", EFI_SECURE_BOOT_MODE_NAME, Byte));
    HiiLibSetBrowserData(Size, &Byte, &gEfiGlobalVariableGuid, EFI_SECURE_BOOT_MODE_NAME);
    Size = 1;
    pRS->GetVariable(EFI_SETUP_MODE_NAME, &gEfiGlobalVariableGuid, NULL, &Size, &Byte);
    TRACE((-1,"%S =%x\n",EFI_SETUP_MODE_NAME,  Byte));
    HiiLibSetBrowserData( Size, &Byte, &gEfiGlobalVariableGuid, EFI_SETUP_MODE_NAME);
    Size = 1;
    pRS->GetVariable(EFI_VENDOR_KEYS_NAME, &gEfiGlobalVariableGuid, NULL, &Size, &Byte);
    TRACE((-1,"%S=%x\n",EFI_VENDOR_KEYS_NAME,  Byte));
    HiiLibSetBrowserData(Size, &Byte, &gEfiGlobalVariableGuid, EFI_VENDOR_KEYS_NAME);
    gBrowserCallbackEnabled = tmpBrowserCallbackEnabled;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateSecureVariableBrowserInfo
//
// Description: Detect 5 EFI Variables: PK, KEK, db, dbt & dbx
//
// Input:       none
//
// Output:      none
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID UpdateSecureVariableBrowserInfo()
{
    EFI_STATUS  Status;
    UINT8       Index;
    EFI_GUID    *EfiVarGuid;
    UINTN       SigCount;
    UINTN       Size, Size2;
    UINT8       *Buf1, *Buf2;

    Index = 0;
    while(SecureVariableFileName[Index] != NULL)
    {
        if(Index < 3)
            EfiVarGuid = &gEfiImageSecurityDatabaseGuid;
        else
            EfiVarGuid = &gEfiGlobalVariableGuid;

        Size = 0;
        Size2 = 0;
        SigCount = 0;
        bKey[Index] = 0;
        Status = pRS->GetVariable( SecureVariableFileName[Index], EfiVarGuid, NULL, &Size, NULL);
        TRACE((-1,"NV Var %S(%d),  status=%r\n",  SecureVariableFileName[Index], Size, Status));
        if(Status == EFI_BUFFER_TOO_SMALL) {
            bKey[Index] = 1; //custom
//
// enhanced logic for vendor/custom key detection
// 
            Buf1 = NULL;
//get var size
            Status = pBS->AllocatePool(EfiBootServicesData, Size, &Buf1);
            ASSERT_EFI_ERROR (Status);
//read Key var to a buffer
            Status = pRS->GetVariable( SecureVariableFileName[Index], EfiVarGuid, NULL, &Size, Buf1);
            if(!EFI_ERROR(Status)) {
#if !defined( TSE_FOR_APTIO_4_50) && !defined(NO_SETUP_COMPILE) && !defined(SMIFLASH_COMPILE)
                ValidateSignatureList (Buf1, Size, &SigCount);
#endif                
 //get Default Key from Volatile key var (use Key size)
                Buf2 = NULL;
                Status = pBS->AllocatePool(EfiBootServicesData, Size, &Buf2);
                ASSERT_EFI_ERROR (Status);
                EfiVarGuid = &gEfiGlobalVariableGuid;
                Size2 = Size;
                Status = pRS->GetVariable( SecureVariableFileNameDefault[Index], EfiVarGuid, NULL, &Size2, Buf2);
                TRACE((-1,"RT Var %S(%d),  status=%r\n",  SecureVariableFileName[Index], Size2, Status));
                if( !EFI_ERROR(Status) && 
//do bin compare using size from a Default var (Key var sz must be >= Def key var)
                    !MemCmp(Buf1, Buf2, Size2) 
                ){
                    bKey[Index] = 2;      //vendor
                    if(Size > Size2)
                       bKey[Index] = 3;   // vendor+extra keys
                }
            }
//Release all Data, Var....
            if(Buf1) pBS->FreePool(Buf1);
            if(Buf2) pBS->FreePool(Buf2);
        } 
#if !defined( TSE_FOR_APTIO_4_50) && !defined(NO_SETUP_COMPILE) && !defined(SMIFLASH_COMPILE)
        switch(bKey[Index]) 
        {
            case 1: GetHiiString(gHiiHandle, STRING_TOKEN(STR_CUSTOM_KEY), sizeof(StrMessage), StrMessage);
                break;
            case 2: GetHiiString(gHiiHandle, STRING_TOKEN(STR_DEFAULT_KEY), sizeof(StrMessage), StrMessage);
                break;
            case 3: GetHiiString(gHiiHandle, STRING_TOKEN(STR_MIXED_KEY), sizeof(StrMessage), StrMessage);
                break;
            case 0:
            default: GetHiiString(gHiiHandle, STRING_TOKEN(STR_ABSENT_KEY), sizeof(StrMessage), StrMessage);
                break;
        }
        GetHiiString(gHiiHandle, SecureVariableStringRef[Index], sizeof(StrTitle), StrTitle);
        Swprintf_s(StrTitle, sizeof(StrTitle), L"%s| %5d| %4d| %s", StrTitle,  Size, SigCount, StrMessage);
        if(mBackDoorVendorKeyChange & (1 << Index)) {
            Swprintf_s(StrTitle, sizeof(StrTitle), L"%s *", StrTitle);
        }
//        TRACE((-1,"StrTitle '%S'\n", StrTitle));
        HiiLibSetString(gHiiHandle, SecureVariableStringTitle[Index], StrTitle);
#endif
        Index++;
    }
    pRS->SetVariable(AMI_SECURE_VAR_PRESENT_VAR, &guidSecurity, EFI_VARIABLE_BOOTSERVICE_ACCESS, sizeof(bKey), &bKey);

    UpdateSecureBootBrowserInfo();
}

#ifndef TSE_FOR_APTIO_4_50
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   UpdateSecureBootSetupVariable
//
// Description: Update Browser Info with Vendor Key status change
//
// Input:       UINT8 updated BackDoorVendorKeyChange
//
// Output:      none 
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID UpdateSecureBootSetupVariable( 
  UINT8 BackDoorVendorKeyChange
  )
{
    EFI_STATUS  Status;
    UINTN       Size;
    UINT32      Attributes=0;
    UINT8       Value=0;
    BOOLEAN     tmpBrowserCallbackEnabled = gBrowserCallbackEnabled;
    SECURE_BOOT_SETUP_VAR SecureBootSetup1;

    TRACE((-1,"VendorKey bitmap Change - %s (org %02X, new %02X)\n", (mBackDoorVendorKeyChange != BackDoorVendorKeyChange)?"Yes":"No",
          mBackDoorVendorKeyChange, BackDoorVendorKeyChange));
    if(mBackDoorVendorKeyChange != BackDoorVendorKeyChange )
    {
        mBackDoorVendorKeyChange = BackDoorVendorKeyChange;

        Size = sizeof(SECURE_BOOT_SETUP_VAR);
        pRS->GetVariable (AMI_SECURE_BOOT_SETUP_VAR ,&guidSecurity, &Attributes, &Size, &SecureBootSetup1);
        SecureBootSetup1.BackDoorVendorKeyChange = mBackDoorVendorKeyChange;
        Status = pRS->SetVariable (AMI_SECURE_BOOT_SETUP_VAR, &guidSecurity, Attributes, Size, &SecureBootSetup1);
        TRACE((-1,"Update %S %r\n", AMI_SECURE_BOOT_SETUP_VAR, Status));

        gBrowserCallbackEnabled = TRUE;
        Size = sizeof(SECURE_BOOT_SETUP_VAR);
        HiiLibGetBrowserData( &Size, &SecureBootSetup, &guidSecurity, AMI_SECURE_BOOT_SETUP_VAR);
        SecureBootSetup.BackDoorVendorKeyChange = mBackDoorVendorKeyChange;
        HiiLibSetBrowserData( Size, &SecureBootSetup, &guidSecurity, AMI_SECURE_BOOT_SETUP_VAR);
        gBrowserCallbackEnabled = tmpBrowserCallbackEnabled;

        Size = 1;
        // EFI_VENDOR_KEYS_NAME is a RO. The value will be set according to internal logic
        Status = pRS->SetVariable (EFI_VENDOR_KEYS_NAME, &gEfiGlobalVariableGuid, 
                 EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS, Size, &Value);
        TRACE((-1,"Update %S %r\n",EFI_VENDOR_KEYS_NAME,  Status));
    }
}
//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  CryptoGetRawImage
//
// Description:    Loads binary from RAW section of X firmware volume
//
//  Input:
//               NameGuid  - The guid of binary file
//               Buffer    - Returns a pointer to allocated memory. Caller must free it when done.
//               Size      - Returns the size of the binary loaded into the buffer.
//
// Output:         Buffer - returns a pointer to allocated memory. Caller
//                        must free it when done.
//               Size  - returns the size of the binary loaded into the
//                        buffer.
//               EFI_NOT_FOUND  - Can't find the binary.
//               EFI_LOAD_ERROR - Load fail.
//               EFI_SUCCESS    - Load success.
//
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
CryptoGetRawImage (
  IN      EFI_GUID       *NameGuid,
  IN OUT  VOID           **Buffer,
  IN OUT  UINTN          *Size
  )
{
  EFI_STATUS                    Status;
  UINTN                         HandleCount;
  UINTN                         Index;
  EFI_FIRMWARE_VOLUME_PROTOCOL  *Fv;
  EFI_HANDLE                    *HandleBuff;
  UINT32                        AuthenticationStatus;

 *Buffer=0;
 *Size=0;
  Status = pBS->LocateHandleBuffer (ByProtocol,&gEfiFirmwareVolume2ProtocolGuid,NULL,&HandleCount,&HandleBuff);
  if (EFI_ERROR (Status) || HandleCount == 0) {
    return EFI_NOT_FOUND;
  }
  //
  // Find desired image in all Fvs
  //
  for (Index = 0; Index < HandleCount; Index++) {
    Status = pBS->HandleProtocol (HandleBuff[Index],&gEfiFirmwareVolume2ProtocolGuid,&Fv);

    if (EFI_ERROR (Status)) {
       continue;//return EFI_LOAD_ERROR;
    }
    //
    // Try a raw file
    //
    Status = Fv->ReadSection (
                  Fv,
                  NameGuid,
                  EFI_SECTION_RAW,
                  0,    //Instance
                  Buffer,
                  Size,
                  &AuthenticationStatus
                  );

    if (Status == EFI_SUCCESS) break;
  }

  pBS->FreePool(HandleBuff);

  if (Index >= HandleCount) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   InstallSecureVariables
//
// Description: Install 5 EFI Variables: PK, KEK, db, dbt & dbx
//
// Input:       BOOLEAN InstallVars
//                  TRUE  - attempt to install secure variables
//                  FALSE - erase secure variables
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
InstallSecureVariables (
    UINT16    InstallVars
){
    EFI_STATUS  Status = EFI_SUCCESS;
    UINT8      *pVarBuffer = NULL;
    UINTN       Size, FileSize, Offset;
    UINT8       Index;
    EFI_GUID    *EfiVarGuid;
    UINT32      Attributes;
    AMI_EFI_VARIABLE_AUTHENTICATION_2 *AuthHdr2;
    UINT8       temp;

    temp = mBackDoorVendorKeyChange;
///////////////////////////////////////////////////////////////////////////////
// Initial provisioning of Authenticated non-volatile EFI Variables 
///////////////////////////////////////////////////////////////////////////////
    Attributes = EFI_VARIABLE_RUNTIME_ACCESS |
                 EFI_VARIABLE_BOOTSERVICE_ACCESS; 
    Index = 0;
    while(/*!EFI_ERROR(Status) &&*/ SecureVariableFileName[Index] != NULL)
    {
        if(Index < 3) 
            EfiVarGuid = &gEfiImageSecurityDatabaseGuid;
        else
            EfiVarGuid = &gEfiGlobalVariableGuid;
// if SET_NV_DEFAULT_KEYS set
/*
1. check if File is present CryptoGetRawImage
2. if not - skip to next var
3. if present -> move to Erase... 
*/        
        if(InstallVars & (SET_NV_DEFAULT_KEYS | SET_RT_DEFAULT_KEYS)) {
            pVarBuffer = NULL;
            FileSize = 0 ; 
            Status = CryptoGetRawImage( SecureVariableFileGuid[Index], &pVarBuffer, (UINTN*)&FileSize);
//            TRACE((-1,"Get Raw image %S, Status %r, sz = %x\n",SecureVariableFileName[Index], Status, FileSize));
            if(EFI_ERROR(Status)) {
                Index++;
                continue;
            }
        }
        if((InstallVars & RESET_NV_KEYS)== RESET_NV_KEYS) {
        // try to erase. should succeed if system in pre-boot and Admin mode
            Status = pRS->SetVariable(SecureVariableFileName[Index],EfiVarGuid,0,0,NULL);
            TRACE((-1,"Clear NV Var %S, Status %r\n",SecureVariableFileName[Index], Status));
            if(!EFI_ERROR(Status))
                temp |= (1<<Index);
            else 
                if(Status == EFI_NOT_FOUND) 
                    Status = EFI_SUCCESS;
        }
        if((InstallVars & (SET_NV_DEFAULT_KEYS | SET_RT_DEFAULT_KEYS)) &&
            pVarBuffer && FileSize ) {
            if(InstallVars & (SET_RT_DEFAULT_KEYS)) {
                AuthHdr2 = (AMI_EFI_VARIABLE_AUTHENTICATION_2*)pVarBuffer;
                Offset = sizeof(EFI_TIME) + AuthHdr2->AuthInfo.Hdr.dwLength;
                Status = pRS->SetVariable(SecureVariableFileNameDefault[Index],
                       &gEfiGlobalVariableGuid,
                        Attributes,
                        FileSize - Offset,
                        (UINT8*)pVarBuffer + Offset
                );
                TRACE((-1,"Set RT Var %S, Status %r\n",SecureVariableFileNameDefault[Index], Status));
            }
            if(InstallVars & (SET_NV_DEFAULT_KEYS)) {
                Size = 0;
                Status = pRS->GetVariable( SecureVariableFileName[Index], EfiVarGuid, NULL, &Size, NULL);
                if(EFI_ERROR(Status) && Status == EFI_NOT_FOUND ) {
                    Status = pRS->SetVariable(SecureVariableFileName[Index],
                            EfiVarGuid,
                            (UINT32)(Attributes | EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS),
                            FileSize,
                            pVarBuffer
                    );
                    TRACE((-1,"Set NV Var %S, Status %r\n",SecureVariableFileName[Index], Status));
                    if(!EFI_ERROR(Status))
                        temp &= ~(1<<Index);
                }
            }
            pBS->FreePool(pVarBuffer);
        }
        Index++;
    }
    if(temp != mBackDoorVendorKeyChange) {
        // set new defaults if change was successful
        UpdateSecureBootSetupVariable(temp);
    }
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FillAuthHdr
//
// Description: 
//
// Input:       NONE
//
// Output:      NONE
//
// Returns:     NONE
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
static VOID FillAuthHdr(
    UINT8*  pVar,
    UINT8   VarSetMode
)
{
    AMI_EFI_VARIABLE_AUTHENTICATION_2 *AuthHdr2;
    static EFI_TIME EfiTime = {
    FOUR_DIGIT_YEAR_INT,
    TWO_DIGIT_MONTH_INT,
    TWO_DIGIT_DAY_INT,
    TWO_DIGIT_HOUR_INT,
    TWO_DIGIT_MINUTE_INT,
    TWO_DIGIT_SECOND_INT,0,0,0,0,0};

    AuthHdr2 = (AMI_EFI_VARIABLE_AUTHENTICATION_2*)pVar;
    MemCpy (&AuthHdr2->TimeStamp, &EfiTime, sizeof (EFI_TIME));
    if((VarSetMode & SET_SECURE_VARIABLE_APPEND)== SET_SECURE_VARIABLE_APPEND)
        AuthHdr2->TimeStamp.Year = 2000; // append should not change the original date the var was created
    AuthHdr2->AuthInfo.Hdr.dwLength = sizeof(WIN_CERTIFICATE_UEFI_GUID_1);
    AuthHdr2->AuthInfo.Hdr.wRevision = 0x200;
    AuthHdr2->AuthInfo.Hdr.wCertificateType = WIN_CERT_TYPE_EFI_GUID;
    AuthHdr2->AuthInfo.CertType = gEfiCertPkcs7Guid;

    return;
}

// only linked with the Setup
#if /* not defined( TSE_FOR_APTIO_4_50) &&*/ !defined(NO_SETUP_COMPILE) && !defined(SMIFLASH_COMPILE)

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetHiiString
//
// Description: This function Reads a String from HII
//
// Input:       IN EFI_HII_HANDLE   HiiHandle - Efi Hii Handle
//              IN STRING_REF       Token     - String Token
//              IN OUT UINTN        *pDataSize - Length of the StringBuffer
//              OUT EFI_STRING      *ppData - The buffer to receive the characters in the string.
//
// Output:      EFI_STATUS - Depending on result
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID GetHiiString(
    IN EFI_HII_HANDLE HiiHandle,
    IN STRING_REF Token,
    IN  UINTN DataSize,
    IN OUT CHAR16  *ppData
    )
{
    EFI_STATUS Status;

    if (!ppData) return;
    
    Status = HiiLibGetString(HiiHandle, Token, &DataSize, (EFI_STRING)ppData);
    ASSERT_EFI_ERROR (Status);
    if (EFI_ERROR (Status)) ppData=L"??? ";

//TRACE((-1,"%r, StrRef '%S', Size %d, Token=%d\n",Status, ppData, DataSize, Token));
}

//#ifndef NO_SETUP_COMPILE
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InitSecureBootStrings
//
// Description: Initializes Secure Boot page Setup String
//
// Input:       HiiHandle - Handle to HII database
//              Class - Indicates the setup class
//
// Output:      None
//
// Notes:       PORTING REQUIRED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID InitSecureBootStrings (
    IN EFI_HII_HANDLE   HiiHandle,
    IN UINT16           Class )
{
    EFI_STATUS          Status;
    UINTN               VarSize;

// 1. Init VendorKey backdoor change
    VarSize = sizeof(SECURE_BOOT_SETUP_VAR);
    Status = pRS->GetVariable (AMI_SECURE_BOOT_SETUP_VAR, &guidSecurity,NULL,&VarSize,&SecureBootSetup);
    mBackDoorVendorKeyChange = SecureBootSetup.BackDoorVendorKeyChange;
    
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FillAuthVarHdr
//
// Description: 
//
// Input:       NONE
//
// Output:      NONE
//
// Returns:     NONE
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
static VOID FillAuthVarHdr(
    UINT8 *pVar,
    UINT8 *pCert,
    UINTN CertSize,
    UINT8 VarSetMode
)
{
    EFI_VARIABLE_SIG_HDR_2 *AuthHdr2;    
    static EFI_GUID    AmiSigOwner = AMI_APTIO_SIG_OWNER_GUID;
    EFI_GUID          *pGuid;

    AuthHdr2 = (EFI_VARIABLE_SIG_HDR_2*)pVar;

    FillAuthHdr(pVar, VarSetMode); // Append AuthHdr to Var data.

    if(CertSize == EFI_CERT_TYPE_SHA256_SIZE)
        pGuid = &gEfiCertSha256Guid;
    if(CertSize == EFI_CERT_TYPE_CERT_X509_SHA256_GUID_SIZE)
        pGuid = &gEfiCertX509Sha256Guid;
    if(CertSize == EFI_CERT_TYPE_CERT_X509_SHA384_GUID_SIZE)
        pGuid = &gEfiCertX509Sha384Guid;
    if(CertSize == EFI_CERT_TYPE_CERT_X509_SHA512_GUID_SIZE)
        pGuid = &gEfiCertX509Sha512Guid;
    if(CertSize == EFI_CERT_TYPE_RSA2048_SIZE)
        pGuid = &gEfiCertRsa2048Guid;
    if(CertSize > EFI_CERT_TYPE_RSA2048_SIZE)
        pGuid = &gEfiCertX509Guid;
    
//      CopyMem (&AuthHdr2->SigList.SignatureType, *pGuid, sizeof (EFI_GUID));
    AuthHdr2->SigList.SignatureType = *pGuid;
    AuthHdr2->SigList.SignatureSize = (UINT32)(sizeof(EFI_GUID)+CertSize);
    AuthHdr2->SigList.SignatureListSize = AuthHdr2->SigList.SignatureSize+sizeof(EFI_SIGNATURE_LIST);
    AuthHdr2->SigList.SignatureHeaderSize = 0;
    AuthHdr2->SigData.SignatureOwner = AmiSigOwner;

TRACE((TRACE_ALWAYS,"SigList Type=%g\nSigSize=%x\nListSize=%x\n", &(AuthHdr2->SigList.SignatureType), AuthHdr2->SigList.SignatureSize, AuthHdr2->SigList.SignatureListSize));

//    MemCpy(AuthHdr2->SigData.SignatureData, pCert, CertSize);

    return;
}   

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ValidateSignatureList
//
// Description: 
//              Validate the data payload begins with valid Signature List header
//              and based on the results returns Status.
//
// Input:
//              IN VOID *Data - pointer to the Var data
//              IN UINTN DataSize - size of Var data
//              OUT     *SigCount - Number of Sig Lists encountered inside Data
//
// Output:      EFI_STATUS
//              UINTN RealDataSize - only the size of the combined length of Signature Lists
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS ValidateSignatureList (
    IN VOID         *Data,
    IN UINTN        DataSize,
    IN OUT UINTN    *SigCount OPTIONAL
)
{
    EFI_STATUS          Status;
    EFI_SIGNATURE_LIST *SigList;
    UINTN               Index, count, SigSize;
    static EFI_GUID mSignatureSupport[SIGSUPPORT_NUM] = {SIGSUPPORT_LIST};

    Status = EFI_SECURITY_VIOLATION;

    if(DataSize == 0 || Data == NULL)
        return Status; // Sig not found

    SigList  = (EFI_SIGNATURE_LIST *)Data;

// loop till end of DataSize for all available SigLists
// Verify signature is one from SigDatabase list mSignatureSupport / sizeof(EFI_GUID)
// SigData begins with SigOwner GUID
// SignatureHdrSize = 0 for known Sig Types
    
    count = 0;
    while ((DataSize > 0) && (DataSize >= SigList->SignatureListSize)) {

        for (Index = 0; Index < SIGSUPPORT_NUM; Index++) {
            if (!guidcmp ((EFI_GUID*) &(SigList->SignatureType), &mSignatureSupport[Index]))
                break;
        }
TRACE((TRACE_ALWAYS,"SigList.Type(%g) -", &(SigList->SignatureType)));
        if(Index >= SIGSUPPORT_NUM)
            return EFI_SECURITY_VIOLATION; // Sig not found

TRACE((TRACE_ALWAYS,"OK\nSigList.Size(4C < %X < %X)-",SigList->SignatureListSize, NVRAM_SIZE));
        if(SigList->SignatureListSize < 0x4C || // Min size for SHA2 Hash Certificate sig list
           SigList->SignatureListSize > NVRAM_SIZE)
            return EFI_SECURITY_VIOLATION; 

TRACE((TRACE_ALWAYS,"OK\nSigList.HdrSize(%X)-",SigList->SignatureHeaderSize));
        if(SigList->SignatureHeaderSize != 0)
            return EFI_SECURITY_VIOLATION; // Sig not found

TRACE((TRACE_ALWAYS,"OK\nSigList.SigSize(%X)\n",SigList->SignatureSize));
        for(SigSize=sizeof(EFI_SIGNATURE_LIST); SigSize < SigList->SignatureListSize; SigSize+=SigList->SignatureSize)
            count++;
            
TRACE((TRACE_ALWAYS,"Sig.Count(%d)\n", count));
        DataSize -= SigList->SignatureListSize;
        SigList = (EFI_SIGNATURE_LIST *) ((UINT8 *) SigList + SigList->SignatureListSize);
        TRACE((TRACE_ALWAYS,"DataSize left(%X)\n", DataSize));

        Status = EFI_SUCCESS;
    }
    if(SigCount)
        *SigCount = count;
    
    return Status;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:    IsCsmEnabled
//
// Description:  This function checks if CSM is enabled
//
//  Input:
//     None
//
//  Output:
//  0 - CSM is disabled
//  1 - CSM is enabled
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
#if defined(CSM_SUPPORT) && CSM_SUPPORT
BOOLEAN
IsCsmEnabled(VOID)
{
    EFI_STATUS Status;
    UINTN Size = sizeof(EFI_HANDLE);
    EFI_HANDLE Handle;

    Status = pBS->LocateHandle(ByProtocol, &gEfiLegacyBiosProtocolGuid, NULL, &Size, &Handle);
    return (EFI_ERROR(Status)) ? 0 : 1;
}
#endif
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ForceSetupModeCallback
//
// Description: 
//
// Input:       none
//
// Output:      VOID
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
ForceSetupModeCallback(EFI_HII_HANDLE HiiHandle, UINT16 Class, UINT16 SubClass, UINT16 Key)
{
    EFI_STATUS Status;
    UINT8       Sel = 0;
    UINTN       Size;

    CALLBACK_PARAMETERS *Callback;
    EFI_BROWSER_ACTION_REQUEST *rq;

    Callback = GetCallbackParameters();
    if(!Callback) {
        return EFI_SUCCESS;
    }
    TRACE((-1,"\n====ForceSetupModeCallback==== Key=%0X, Action=%0X\n",  Key, Callback->Action));
    if( Callback->Action != EFI_BROWSER_ACTION_CHANGING) {
//        if(Callback->Action==EFI_BROWSER_ACTION_RETRIEVE)
//            UpdateSecureVariableBrowserInfo();
        return EFI_UNSUPPORTED;
    }
    if(mPostMgr == NULL)
    {
        Status = pBS->LocateProtocol(&gAmiPostManagerProtocolGuid, NULL, &mPostMgr);
        if(EFI_ERROR(Status) || !mPostMgr) {
            return EFI_SUCCESS;
        }
    }
    Status = EFI_SUCCESS;
    gHiiHandle = HiiHandle;
    switch(Key) {
        case  SECURE_BOOT_MENU_REFRESH:
            UpdateSecureBootBrowserInfo();
            break;
        case  KEY_MANAGEMENT_MENU_REFRESH:
            UpdateSecureVariableBrowserInfo();
            break;
        case SECURE_BOOT_SUPPORT_CHANGE_KEY:
            if( Callback->Value->u8 == 1) // trying to switch Secure Boot from Disable to Enable
            {
                rq = Callback->ActionRequest;
                *rq = EFI_BROWSER_ACTION_REQUEST_NONE;
                Size = 1;
                Status=pRS->GetVariable(EFI_SETUP_MODE_NAME, &gEfiGlobalVariableGuid, NULL, &Size, &Sel);
                if(Sel) {
                    GetHiiString(HiiHandle, STRING_TOKEN(STR_ENABLE_ERROR_MODE_TITLE), sizeof(StrTitle), StrTitle);
                    GetHiiString(HiiHandle, STRING_TOKEN(STR_ENABLE_ERROR_MODE), sizeof(StrMessage),StrMessage);
                    mPostMgr->DisplayMsgBox( StrTitle,  StrMessage, MSGBOX_TYPE_OK,NULL);
        #if DEFAULT_SECURE_BOOT_ENABLE == 0
                    Status = EFI_UNSUPPORTED;
        #endif
                }  
#if defined(CSM_SUPPORT) && CSM_SUPPORT
                else if (IsCsmEnabled()) {
                        GetHiiString(HiiHandle, STRING_TOKEN(STR_CSM_LOAD_TITLE), sizeof(StrTitle),StrTitle);
                        GetHiiString(HiiHandle, STRING_TOKEN(STR_CSM_LOAD), sizeof(StrMessage),StrMessage);
                        mPostMgr->DisplayMsgBox( StrTitle,  StrMessage, MSGBOX_TYPE_OK,NULL);
/* this should be enough, but TSE has a bug that doesn't support FORM_DISCARD action try a workaround instead 
                       {
                           SETUP_DATA Setup;
                           UINTN Size = sizeof(Setup);
                           static EFI_GUID SetupGuid = SETUP_GUID;
                           Status = HiiLibGetBrowserData(&Size, &Setup, &SetupGuid, L"Setup");
                            TRACE((-1, "Setup.CsmSupport=: %x, Size = %x\n", Setup.CsmSupport, Size));
                           if(!EFI_ERROR(Status) && Setup.CsmSupport == 1) {
                               GetHiiString(HiiHandle, STRING_TOKEN(STR_CSM_LOAD_TITLE), sizeof(StrTitle),StrTitle);
                               GetHiiString(HiiHandle, STRING_TOKEN(STR_CSM_LOAD), sizeof(StrMessage),StrMessage);
                               mPostMgr->DisplayMsgBox( StrTitle,  StrMessage, (UINT8)MSGBOX_TYPE_YESNO, &Sel);
                               if (Sel == 1) {
                                   Setup.CsmSupport = 0;
                                   Status = HiiLibSetBrowserData(Size, &Setup, &SetupGuid, L"Setup");
//                                    pRS->SetVariable ( L"Setup", &SetupGuid, Attributes, Size, &Setup); 
                               }
                           }
                       }
end of workaround */
                        
        #if DEFAULT_SECURE_BOOT_ENABLE == 0
                       Status = EFI_UNSUPPORTED;
        #endif
                    }
#endif // CSM                
            }
            break;
        case SECURE_BOOT_MODE_CHANGE_KEY:
            if( Callback->Value->u8 == 1) // trying to switch from Custom to Standard
                break;
        case KEY_PROVISION_CHANGE_KEY:
            if(Key == KEY_PROVISION_CHANGE_KEY && 
                Callback->Value->u8 == 0) // trying to switch from Disable to Enable
                break;
        case FORCE_SETUP_KEY:
        case FORCE_DEFAULT_KEY:
            if (Key == FORCE_SETUP_KEY)
            {
                GetHiiString(HiiHandle, STRING_TOKEN(STR_FORCE_SETUP_MODE), sizeof(StrTitle),StrTitle);
                GetHiiString(HiiHandle, STRING_TOKEN(STR_RESET_TO_SETUP), sizeof(StrMessage),StrMessage);
                mPostMgr->DisplayMsgBox( StrTitle,  StrMessage, (UINT8)MSGBOX_TYPE_YESNO, &Sel);
                if (Sel == 0)
                   Status = InstallSecureVariables(RESET_NV_KEYS);    // erase
            }
            else
            if (Key == FORCE_DEFAULT_KEY || Key == KEY_PROVISION_CHANGE_KEY || Key == SECURE_BOOT_MODE_CHANGE_KEY)
            {
                GetHiiString(HiiHandle, STRING_TOKEN(STR_LOAD_DEFAULT_VARS_TITLE), sizeof(StrTitle),StrTitle);
                GetHiiString(HiiHandle, STRING_TOKEN(STR_LOAD_DEFAULT_VARS), sizeof(StrMessage),StrMessage);
                mPostMgr->DisplayMsgBox( StrTitle,  StrMessage, (UINT8)MSGBOX_TYPE_YESNO, &Sel);
                if (Sel == 0)
                    Status = InstallSecureVariables(RESET_NV_KEYS | SET_NV_DEFAULT_KEYS);    // erase+set
            }
            UpdateSecureVariableBrowserInfo();
            if (EFI_ERROR(Status))
            {
                GetHiiString(HiiHandle, STRING_TOKEN(STR_VAR_UPDATE_LOCKED_TITLE), sizeof(StrTitle),StrTitle);
                GetHiiString(HiiHandle, STRING_TOKEN(STR_VAR_UPDATE_LOCKED), sizeof(StrMessage),StrMessage);
                mPostMgr->DisplayMsgBox( StrTitle,  StrMessage, MSGBOX_TYPE_OK,NULL);
                Status = EFI_SUCCESS;
            }
            break;

        default:
            break;
    }
    return Status;//EFI_SUCCESS;
}

EFI_STATUS SetSecureVariable(UINT8 Index, UINT16 InstallVars, UINT8 *pVarBuffer, UINTN VarSize )
{
    EFI_STATUS  Status = EFI_SUCCESS;
    EFI_GUID    *EfiVarGuid;
    UINT32      Attributes;

///////////////////////////////////////////////////////////////////////////////
// Initial provisioning of Authenticated non-volatile EFI Variables 
///////////////////////////////////////////////////////////////////////////////
    if(SecureVariableFileName[Index] != NULL)
    {
        if(Index < 3) 
            EfiVarGuid = &gEfiImageSecurityDatabaseGuid;
        else
            EfiVarGuid = &gEfiGlobalVariableGuid;

        if((InstallVars & SET_SECURE_VARIABLE_DEL) == SET_SECURE_VARIABLE_DEL) {
        // try to erase. should succeed if system in pre-boot and Admin mode
            Status = pRS->SetVariable(SecureVariableFileName[Index],EfiVarGuid,0,0,NULL);
            TRACE((-1,"Del Var %S, Status %r\n",SecureVariableFileName[Index], Status));
        }
        if((InstallVars & SET_SECURE_VARIABLE_SET)==SET_SECURE_VARIABLE_SET &&
            pVarBuffer && VarSize) {

            Attributes = EFI_VARIABLE_RUNTIME_ACCESS |
                         EFI_VARIABLE_NON_VOLATILE | 
                         EFI_VARIABLE_BOOTSERVICE_ACCESS |
                         EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS; 

            if((InstallVars & SET_SECURE_VARIABLE_APPEND)== SET_SECURE_VARIABLE_APPEND)
                Attributes |= EFI_VARIABLE_APPEND_WRITE;

            Status = pRS->SetVariable(SecureVariableFileName[Index], EfiVarGuid, Attributes, VarSize, pVarBuffer);
            TRACE((-1,"Set Var %S, Status %r\n",SecureVariableFileName[Index], Status));
        }
    }

    return Status;
}

EFI_STATUS
ManageSecureBootKeysCallback(EFI_HII_HANDLE HiiHandle, UINT16 Class, UINT16 SubClass, UINT16 Key)
{
    EFI_STATUS Status;
    UINT8 index;
    UINT16 CertSel = 0;
    UINT8 MenuSize, DelItem, SetItem, AppendItem;
    POSTMENU_TEMPLATE MenuList[3];
    CALLBACK_PARAMETERS *Callback;

    Callback = GetCallbackParameters();
    if(!Callback || Callback->Action != EFI_BROWSER_ACTION_CHANGING)
        return EFI_UNSUPPORTED;

    switch(Key)
    {
    case MANAGE_PK_KEY:
                    index = 4;
                    break;
    case MANAGE_KEK_KEY:
                    index = 3;
                    break;
    case MANAGE_DB_KEY:
                    index = 2;
                    break;
    case MANAGE_DBT_KEY:
                    index = 1;
                    break;
    case MANAGE_DBX_KEY:
                    index = 0;
                    break;
    default:
        return EFI_SUCCESS;
    }

    if(mPostMgr == NULL)
        return EFI_SUCCESS;

    gHiiHandle = HiiHandle;

    SetItem = 0;
    AppendItem = 1;
    DelItem = 2;
    MenuSize = 3; 
    if(!bKey[index]) { // no Key - no Del
        MenuSize--;
    }
    if(index == 4) { // PK - no Append
        MenuSize--;
        DelItem--; // move Del up
        AppendItem = 2;
    }

    // Clear the memory allocated 
    MemSet(MenuList, sizeof(MenuList), 0);
    MenuList[SetItem].ItemToken = STRING_TOKEN(STR_SECURE_SET);
    MenuList[AppendItem].ItemToken = STRING_TOKEN(STR_SECURE_APPEND);
    MenuList[DelItem].ItemToken = STRING_TOKEN(STR_SECURE_DELETE);

    // Call post manager to display the menu
    Status = mPostMgr->DisplayPostMenu(HiiHandle,
                        STRING_TOKEN(STR_KEY_MANAGEMENT_TITLE),
                        0,
                        MenuList,
                        MenuSize,
                        &CertSel);

    if(!EFI_ERROR(Status))
    {
        if(CertSel == DelItem) {
            if(bKey[index])
                DeleteSecureBootDBCallback( HiiHandle,  index);
        } else
                if(CertSel == SetItem) {
                    SetAppendSecureBootDBCallback( HiiHandle,  SET_SECURE_VARIABLE_SET | SET_SECURE_VARIABLE_DEL, index);
                } else 
                        if(CertSel == AppendItem) {
                            if(index!=4)
                                SetAppendSecureBootDBCallback( HiiHandle,  SET_SECURE_VARIABLE_SET | SET_SECURE_VARIABLE_APPEND,  index);
                        }
    }
    return EFI_SUCCESS;
}

VOID
SetAppendSecureBootDBCallback(EFI_HII_HANDLE HiiHandle, UINT8 VarSetMode, UINT8 index)
{
    EFI_STATUS Status;
    EFI_HANDLE *FsHandle;
    UINT8 *FileBuf=NULL;
    UINT8 *Data=NULL;
    UINTN size, VarSize; //DataSize;
    CHAR16 *FilePath=NULL;
    UINT8 Sel;
    UINT16 CertSel, AddSize;
    UINT8 MenuSize;
    POSTMENU_TEMPLATE MenuList[3];
    UINT8       temp;

    MemSet(StrTemp, sizeof(StrTemp), 0);
    Sel = 1; // No
    CertSel = 0;
    temp = mBackDoorVendorKeyChange;
    
//    DataSize = sizeof(StrTitle);
    if(VarSetMode & SET_SECURE_VARIABLE_APPEND)
//        HiiLibGetString(HiiHandle, STRING_TOKEN(STR_SECURE_APPEND), &DataSize, (EFI_STRING)StrTitle);
        GetHiiString(HiiHandle, STRING_TOKEN(STR_SECURE_APPEND), sizeof(StrTitle), StrTitle);
    else
        GetHiiString(HiiHandle, STRING_TOKEN(STR_SECURE_SET), sizeof(StrTitle), StrTitle);

    GetHiiString(HiiHandle, STRING_TOKEN(STR_UPDATE_FROM_DEFAULTS),sizeof(StrMessage), StrMessage);
    Swprintf_s(StrTemp, sizeof(StrTemp), StrMessage ,  SecureVariableFileName[index]);
    
    mPostMgr->DisplayMsgBox( StrTitle, StrTemp, (UINT8)MSGBOX_TYPE_YESNO, &Sel);
    if(Sel == 0)
    {
        size = 0 ; 
        Status = CryptoGetRawImage( SecureVariableFileGuid[index], &FileBuf, (UINTN*)&size);
        temp &= ~(1 << index); // Clearing corresponding key backdoor
    } else
    {
        size = 0;
        AddSize = 0;
//        MenuSize = (index == 3 || index == 4)?2:3;
        MenuSize = 2;
        // updating keys from external media - consider non-Vendor keys
        temp |= (1 << index); // setting corresponding key backdoor
        Status = FileBrowserLaunchFileSystem(TRUE, &FsHandle, &FilePath, &FileBuf, &size);
        if(!EFI_ERROR(Status) && FileBuf)
        {
            // Clear the memory allocated 
            MemSet(MenuList, sizeof(MenuList), 0);
            MenuList[0].ItemToken = STRING_TOKEN(STR_SECURE_CER);
            MenuList[1].ItemToken = STRING_TOKEN(STR_SECURE_VAR);
//            MenuList[2].ItemToken = STRING_TOKEN(STR_SECURE_EFI);
            
            // Call post manager to display the menu
            Status = mPostMgr->DisplayPostMenu(gHiiHandle,
                                                STRING_TOKEN(STR_SECURE_FILE_TITLE), // Change this
                                                0,
                                                MenuList,
                                                MenuSize,
                                                &CertSel);

            if(!EFI_ERROR(Status))
            {
                GetHiiString(HiiHandle, STRING_TOKEN(STR_UPDATE_FROM_FILE), sizeof(StrMessage), StrMessage);
                Swprintf_s(StrTemp, sizeof(StrTemp),StrMessage, SecureVariableFileName[index],FilePath);


//                if(CertSel==2) // Efi executable
//                    TRACE((-1,"Efi executable %S, Status %r\n",StrTemp, Status));
/*
Install New protocol ImageVerificationExt
GetSignerInfo(pSignerCert, CertSize, Instance)
Instance - Output - number of signers, Input - select SignerCert instance 
Draw the drop Down box with names of Signers Subjects
 */                
                if(CertSel==0) {
                    AddSize = sizeof(EFI_VARIABLE_SIG_HDR_2)-1; // decrement by 1 byte as SIG_DATA adds 1 dummy byte
                }

                // Validate Signature List integrity 
                if(!EFI_ERROR(ValidateSignatureList (FileBuf, size, NULL))) {
                    CertSel=MenuSize;
                    AddSize = sizeof(AMI_EFI_VARIABLE_AUTHENTICATION_2);
                }
                //
                // form an AuthVar Hdr on top of Var
                //
                //Allocate new Size
                VarSize = size+AddSize;
                Status = pBS->AllocatePool(EfiBootServicesData, VarSize, &Data);
                ASSERT_EFI_ERROR (Status);
                // Append AuthHdr to Var data.
                if(CertSel==0)
                    FillAuthVarHdr(Data,FileBuf,size, VarSetMode);
                else 
                if(CertSel==MenuSize) // unsupported - append from SigList
                    FillAuthHdr(Data, VarSetMode);

                MemCpy ((VOID*)((UINTN)Data+AddSize), FileBuf, size);
                if(FileBuf)
                    pBS->FreePool(FileBuf);
                FileBuf = Data;
                size = VarSize;
            }
        }
    }
    if(FileBuf) {

        if(Sel == 1)
            mPostMgr->DisplayMsgBox( StrTitle, StrTemp, (UINT8)MSGBOX_TYPE_YESNO, &Sel);

        if(Sel == 0)
        {
            Status = SetSecureVariable(index, VarSetMode, FileBuf, size);
            if(!EFI_ERROR(Status)){
                GetHiiString(HiiHandle, STRING_TOKEN(STR_SUCCESS), sizeof(StrMessage),StrMessage);
                // set new defaults if change was successful
                UpdateSecureBootSetupVariable(temp);
            }
            else
            {
                GetHiiString(HiiHandle, STRING_TOKEN(STR_FAILED), sizeof(StrMessage),StrMessage);
            }
            mPostMgr->DisplayMsgBox( StrTitle, StrMessage, MSGBOX_TYPE_OK, NULL );
            UpdateSecureVariableBrowserInfo();
        }
    }
    if(FileBuf)
        pBS->FreePool(FileBuf);

    if(FilePath)
        pBS->FreePool(FilePath);
}

VOID
DeleteSecureBootDBCallback(EFI_HII_HANDLE HiiHandle, UINT8 index)
{
    EFI_STATUS Status;
    UINT8 Sel = 0;    

    MemSet(StrTemp, sizeof(StrTemp), 0);

    GetHiiString(HiiHandle, STRING_TOKEN(STR_DELETE_SEC_KEY_TITLE), sizeof(StrTitle),StrTitle);
    if(index == 4) { // del PK
        GetHiiString(HiiHandle, STRING_TOKEN(STR_PK_DEL_WARNING), sizeof(StrTemp),StrTemp);
        mPostMgr->DisplayMsgBox( StrTitle,  StrTemp, (UINT8)MSGBOX_TYPE_OK, NULL);
    }
    GetHiiString(HiiHandle, STRING_TOKEN(STR_DELETE_SEC_KEY), sizeof(StrMessage), StrMessage);
    MemSet(StrTemp, sizeof(StrTemp), 0);
    Swprintf_s(StrTemp, sizeof(StrTemp), StrMessage, SecureVariableFileName[index]);
    mPostMgr->DisplayMsgBox( StrTitle,  StrTemp, (UINT8)MSGBOX_TYPE_YESNO, &Sel);
    if(Sel == 0)
    {
        Status = SetSecureVariable(index, SET_SECURE_VARIABLE_DEL, NULL, 0);
        if(!EFI_ERROR(Status)){
            GetHiiString(HiiHandle, STRING_TOKEN(STR_SUCCESS), sizeof(StrMessage),StrMessage);
            // setting a corresponding key back-door
            UpdateSecureBootSetupVariable(mBackDoorVendorKeyChange | (1 << index));
        }
        else
        {
            GetHiiString(HiiHandle, STRING_TOKEN(STR_FAILED), sizeof(StrMessage),StrMessage);
        }            
        mPostMgr->DisplayMsgBox( StrTitle, StrMessage, MSGBOX_TYPE_OK, NULL );
        UpdateSecureVariableBrowserInfo();
    }
}

EFI_STATUS
GetSecureBootDBCallback(EFI_HII_HANDLE HiiHandle, UINT16 Class, UINT16 SubClass, UINT16 Key)
{
    EFI_STATUS Status;
    EFI_HANDLE *FsHandle;
    UINT8 *FileBuf=NULL;
    UINTN size;
    CHAR16 *FilePath=NULL;
    UINT8 Index, nVars;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *pSFP;
    EFI_FILE_PROTOCOL *pRoot,*FileHandle;
    EFI_GUID    *EfiVarGuid;
    UINT8       *Data=NULL;
    BOOLEAN     bFound = TRUE;
    
    CALLBACK_PARAMETERS *Callback;

    Callback = GetCallbackParameters();
    if(!Callback || Callback->Action != EFI_BROWSER_ACTION_CHANGING)
       return EFI_UNSUPPORTED;

    gHiiHandle = HiiHandle;
    size = 0;
    Status = FileBrowserLaunchFileSystem(FALSE, &FsHandle, &FilePath, &FileBuf, &size);
    if(EFI_ERROR(Status))
       goto Done;

    Index = 0;
    nVars = 0;
    MemSet(StrTemp, sizeof(StrTemp), 0);
    while(bFound && SecureVariableFileName[Index] != NULL)
    {
        if(Index < 3) 
            EfiVarGuid = &gEfiImageSecurityDatabaseGuid;
        else
            EfiVarGuid = &gEfiGlobalVariableGuid;

        size = 0;  
        Status = pRS->GetVariable( SecureVariableFileName[Index], EfiVarGuid, NULL, &size, NULL);
        if(Status == EFI_BUFFER_TOO_SMALL)
        {
            // Append AuthHdr to Var data.
            //Allocate Size
            Status = pBS->AllocatePool(EfiBootServicesData, size, &Data);
            ASSERT_EFI_ERROR (Status);

            // Read the Variable
            Status = pRS->GetVariable( SecureVariableFileName[Index], EfiVarGuid, NULL, &size, Data);
            if (!EFI_ERROR(Status)) 
            {
                Status = pBS->HandleProtocol( FsHandle, &gEfiSimpleFileSystemProtocolGuid, &pSFP );
                if (!EFI_ERROR(Status)) 
                {
                    Status = pSFP->OpenVolume(pSFP,&pRoot);
                    if (!EFI_ERROR(Status)) 
                    {
                        Status = pRoot->Open(pRoot,&FileHandle,SecureVariableFileName[Index],
                            EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE,0);
                        if(!EFI_ERROR(Status))
                        {
                            // Write
                            FileHandle->Write(FileHandle,&size, Data);
                            FileHandle->Close(FileHandle);
                            nVars++;
                        }
                    }
                }

               if (EFI_ERROR(Status)) {
                    bFound = FALSE;
                    Swprintf_s(StrTemp, sizeof(StrTemp), L" %s ", SecureVariableFileName[Index]);
                    GetHiiString(HiiHandle, STRING_TOKEN(STR_WRITE_ERROR_TITLE), sizeof(StrTitle),StrTitle);
                    mPostMgr->DisplayMsgBox( StrTitle, StrTemp, MSGBOX_TYPE_OK, NULL );
                }
            }
            if(Data!=NULL)
                pBS->FreePool(Data);
        }

        Index++;
    }
    if (Index==5 && bFound) {
        GetHiiString(HiiHandle, STRING_TOKEN(STR_SAVE_SEC_KEY),sizeof(StrMessage), StrMessage);
        Swprintf_s(StrTemp, sizeof(StrTemp), StrMessage, nVars);
        GetHiiString(HiiHandle, STRING_TOKEN(STR_SAVE_SEC_KEY_TITLE),sizeof(StrTitle), StrTitle);
        mPostMgr->DisplayMsgBox(StrTitle , StrTemp, MSGBOX_TYPE_OK, NULL );
    }

Done:
 
    if(FileBuf)
        pBS->FreePool(FileBuf);

    if(FilePath)
        pBS->FreePool(FilePath);

    return EFI_SUCCESS;
}

static VOID EfiStrCat (
    IN CHAR16   *Destination,
    IN CHAR16   *Source
    )
{   
    Wcscpy (Destination + Wcslen (Destination), Source);
}

static CHAR16 *StrDup8to16( CHAR8 *string )
{
    CHAR16  *text;
    UINTN   i;

    if ( string == NULL )
        return NULL;

    pBS->AllocatePool(EfiBootServicesData, (1 + Strlen( string )) * sizeof(CHAR16), &text);

    if ( text != NULL )
    {
        i=0;
        while(text[i] = (CHAR16)string[i])
            i++;
    }

    return text;
}
 
EFI_STRING_ID HiiAddString(IN EFI_HII_HANDLE HiiHandle,IN CHAR16 *String)
{
    EFI_STATUS Status;
    CHAR8* Languages = NULL;
    UINTN LangSize = 0;
    CHAR8* CurrentLanguage;
    BOOLEAN LastLanguage = FALSE;
    EFI_STRING_ID  StringId = 0;
    CHAR8          *SupportedLanguages=NULL;

    if(HiiString == NULL) {
        Status = pBS->LocateProtocol(&gEfiHiiStringProtocolGuid, NULL, (VOID **) &HiiString);
        if(EFI_ERROR(Status))
            return 0;
    }

    if(SupportedLanguages == NULL) {
        Status = HiiString->GetLanguages(HiiString, HiiHandle, Languages, &LangSize);
        if(Status == EFI_BUFFER_TOO_SMALL) {
            Status = pBS->AllocatePool(EfiBootServicesData, LangSize, &Languages);
            if(EFI_ERROR(Status))
                return 0;        //not enough resources to allocate string
            Status = HiiString->GetLanguages(HiiString, HiiHandle, Languages, &LangSize);
        }
        if(EFI_ERROR(Status))
            return 0;
    } else {
        Languages = SupportedLanguages;
     }

    while(!LastLanguage) {
        CurrentLanguage = Languages;        //point CurrentLanguage to start of new language
        while(*Languages != ';' && *Languages != 0)
            Languages++;

        if(*Languages == 0) {       //last language in language list
            LastLanguage = TRUE;
            if(StringId == 0)
                Status = HiiString->NewString(HiiString, HiiHandle, &StringId, CurrentLanguage, NULL, String, NULL);
            else
                Status = HiiString->SetString(HiiString, HiiHandle, StringId, CurrentLanguage, String, NULL);
            if(EFI_ERROR(Status))
                return 0;
        } else {
            *Languages = 0;         //put null-terminator
            if(StringId == 0)
                Status = HiiString->NewString(HiiString, HiiHandle, &StringId, CurrentLanguage, NULL, String, NULL);
            else
                Status = HiiString->SetString(HiiString, HiiHandle, StringId, CurrentLanguage, String, NULL);
            *Languages = ';';       //restore original character
            Languages++;
            if(EFI_ERROR(Status))
                return 0;
        }
    }
    return StringId;        
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:    EfiLibAllocateCopyPool
//
// Description:    Allocate BootServicesData pool and use a buffer provided by 
//                    caller to fill it.
//
// Input:    AllocationSize  - The size to allocate
//                    Buffer          - Buffer that will be filled into the buffer allocated
//
// Output:    Pointer of the buffer allocated.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID *
EfiLibAllocateCopyPool (
  IN  UINTN   AllocationSize,
  IN  VOID    *Buffer
  )
{
    VOID  *Memory;
    
    Memory = NULL;
    pBS->AllocatePool (EfiBootServicesData, AllocationSize, &Memory);
    if (Memory != NULL) {
        pBS->CopyMem (Memory, Buffer, AllocationSize);
    }
    
    return Memory;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   CleanFileTypes
//
// Description: Frees all allocated memory associated with the FILE_TYPE structure
//      and resets the InternalString current strings next available string token
//      to be the first dynamically added string
//
// Input:   FILE_TYPE **FileList - The array of FILE_TYPE structures found in 
//              a directory
//          UINTN *FileCount - pointer to the number of entries in the FileList
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID CleanFileTypes(FILE_TYPE **FileList, UINTN *FileCount)
{
    UINTN i;
    for(i = 0; i<*FileCount; i++) pBS->FreePool((*FileList)[i].Name);
    if(FileList!=NULL && (*FileList!=NULL) && (*FileCount>0)) pBS->FreePool(*FileList);
    if(FileList!=NULL) *FileList = NULL;
    *FileCount = 0;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   CheckDirectoryType
//
// Description: Checks if the EFI_FILE_INFO is a directory (and not the current directory)
//
// Input:   EFI_FILE_INFO *File
//
// Output:  
//
// Returns: BOOLEAN - TRUE - item is a directory, and not the current directory
//                    FALSE - item is not a directory, or is the current directory
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN CheckDirectoryType(EFI_FILE_INFO *File)
{
    BOOLEAN Status = FALSE;

    if((File->Attribute & EFI_FILE_DIRECTORY) && (Wcscmp(File->FileName, L".") != 0)) {

        Status = TRUE;
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   CheckExtension
//
// Description: Check is the EFI_FILE_INFO has the same extension as the 
//      extension passed in the second parameter
//
// Input:   EFI_FILE_INFO *File - The file entry whose extension should be checked
//          CHAR16 *ExtensionEfi - the extension
//
// Output:
//
// Returns: BOOLEAN - TRUE - The extension matches
//                    FALSE - the extension does not match
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN CheckExtension(EFI_FILE_INFO *File, CHAR16 *ExtensionEfi)
{
    BOOLEAN Status = FALSE;
    UINTN Length = Wcslen(File->FileName);

    if((File->Attribute & EFI_FILE_DIRECTORY) != EFI_FILE_DIRECTORY && Length > 3)
        if((((File->FileName[Length-1])&0xdf) == ((ExtensionEfi[2])&0xdf)) &&
           (((File->FileName[Length-2])&0xdf) == ((ExtensionEfi[1])&0xdf)) &&
           (((File->FileName[Length-3])&0xdf) == ((ExtensionEfi[0])&0xdf)))
            Status = TRUE;
    return Status;    
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FindInsertionIndex
//
// Description: Finds the inded where directories items turn into file items
//
// Input:   FILE_TYPE *List - the current array of File Type structures
//          UINTN FileCount - the count of File Type structures in the array
//
// Output:
//
// Returns: the index to insert a new item
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
UINTN FindInsertionIndex(FILE_TYPE *List, UINTN FileCount)
{
    UINTN i = 0;
    
    if(FileCount <= 1) return 0;

    for(i = 1; i < (FileCount-1); i++)
    {
        if(List[i-1].Type != List[i].Type)
        break;
    }

    return i;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   AddFileTypeEntry
//
// Description: Creates a new entry in the FILE_TYPE array and adds the current File into
//      the array.
//
// Input:   FILE_TYPE **List - Array of FILE_TYPE structures alread found
//          UINTN *FileCount - number of entries in the FILE_TYPE array
//          EFI_FILE_INFO *FileInfo - file info of the file that should be added
//
// Output:
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
static VOID AddFileTypeEntry(FILE_TYPE **List, UINTN *FileCount, EFI_FILE_INFO *FileInfo)
{
    FILE_TYPE *NewList=NULL;
    UINTN Length;
    UINTN Index = 0;

    Length = (Wcslen(FileInfo->FileName)+3)*sizeof(CHAR16);

    // Allocate space for a new list entry plus all the previous list items
    NewList = EfiLibAllocateCopyPool(sizeof(FILE_TYPE)*(++(*FileCount)), NewList);
    if (NewList != NULL) 
    {
        // Clear the memory of the entire list
        MemSet(NewList, sizeof(FILE_TYPE)*(*FileCount), 0);
    
        // Copy the old entries (if there are any old entries to copy)
        if(*List != NULL) 
        {
            Index = FindInsertionIndex(*List, *FileCount);

            pBS->CopyMem(NewList, *List, sizeof(FILE_TYPE)*(Index));
            pBS->CopyMem(&(NewList[Index+1]), &((*List)[Index]), sizeof(FILE_TYPE)*((*FileCount)-Index-1));

            pBS->FreePool(*List);
        }

        // Store the type of this FILE_TYPE entry (non-zero is directory)
        NewList[Index].Type = ((FileInfo->Attribute) & EFI_FILE_DIRECTORY);

        // Store the size of the file
        NewList[Index].Size = (UINTN)FileInfo->FileSize;

        // Allocate space for the string
        NewList[Index].Name = EfiLibAllocateCopyPool(Length, NewList[Index].Name);
        if((NewList[Index].Name) != NULL )
        {
            // Clear the allocated memory
            MemSet(NewList[Index].Name, Length, 0);

            // Create either a Dir string or a File string for addition to the HiiDataBase
            if(NewList[Index].Type == EFI_FILE_DIRECTORY)
                 Swprintf_s(NewList[Index].Name, Length, L"<%s>", FileInfo->FileName);
            else
                 Swprintf_s(NewList[Index].Name, Length, L"%s", FileInfo->FileName);

            // Add the string to the HiiDataBase
            ///NewList[Index].Token = AddStringToHii(FileInfo->FileName, &gInternalStrings);    ///Just by trying using the following line
            NewList[Index].Token =     HiiAddString(gHiiHandle, NewList[Index].Name );

            // Clear the memory and create the string for the File Structure
            MemSet(NewList[Index].Name, Length, 0);
             Swprintf_s(NewList[Index].Name, Length, L"%s", FileInfo->FileName);            
        }
        *List = NewList;
    }
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   CreateFileList
//
// Description: Parse all the files in the current directory and add valid files to the
//      FILE_TYPE list and update the filecount
//
// Input:   EFI_FILE_PROTOCOL *FileProtocol - the current directory to parse
//
// Output:  FILE_TYPE **FileList - pointer in which to return the array of FileType items
//          UINTN *FileCount - the count of filetype items discovered
//
// Returns: EFI_STATUS
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
static EFI_STATUS CreateFileList(EFI_FILE_PROTOCOL *FileProtocol, FILE_TYPE **FileList, UINTN *FileCount)
{
    EFI_STATUS Status = EFI_SUCCESS;

    UINTN   BufferSize = 1;
    EFI_FILE_INFO *File = NULL;

//    CHAR16 ExtensionEfi[] = L"EFI";

    // Continue parsing the directory until we no longer read valid files
    while(BufferSize != 0 && !EFI_ERROR(Status))
    {
        BufferSize = 0;
        Status = FileProtocol->Read(FileProtocol, &BufferSize, NULL);

        if(!EFI_ERROR(Status)) break;

        if(Status == EFI_BUFFER_TOO_SMALL)
        {
            File = EfiLibAllocateCopyPool(BufferSize, File);
            if(File != NULL) {
                    MemSet(File, BufferSize, 0);
                }
        }

        Status = FileProtocol->Read(FileProtocol, &BufferSize, File);

        // Check if a valid file was read
        if(!EFI_ERROR(Status) && BufferSize != 0)
        {
            // check if the file read was a directory or a ".efi" extension
//            if(CheckDirectoryType(File) ||  CheckExtension(File, ExtensionEfi))
//            {
                // the file was valid, add it to the file list
                AddFileTypeEntry(FileList, FileCount, File);
//            }
        }

        // free the space allocated for readin the file info structure
        pBS->FreePool(File);

        // set the pointer to null to prevent the chance of memory corruption
        File = NULL;
    }

    if(*FileCount == 0) Status = EFI_NOT_FOUND;

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   DisplayFileListMenu
//
// Description: Display a menu of the FILE_TYPE items and return the selected item
//              in the Selection
//
// Input:   FILE_TYPE *FileList - List of FILE_TYPE items to display in the menu
//          UINTN FileCount - the number of FILE_TYPE items in the list
//
// Output:  UINT16 *Selection - The index of the selected FILE_TYPE item
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
static EFI_STATUS DisplayFileListMenu(FILE_TYPE *FileList, UINTN FileCount, UINT16 *Selection)
{
    EFI_STATUS Status = EFI_SUCCESS;

    UINT16 i = 0;

    POSTMENU_TEMPLATE *List = NULL;

    // Check there are any files to display
    if(FileCount != 0 && FileList != NULL)
    {
        // allocate space for the POSTMENU_TEMPLATE items
        List = EfiLibAllocateCopyPool(sizeof(POSTMENU_TEMPLATE)*FileCount, List);
        if(List != NULL)
        {
            // Clear the memory allocated 
            MemSet(List, sizeof(POSTMENU_TEMPLATE)*FileCount, 0);

            // Add the STRING_REF tokens to the POSTMENU_TEMPLATE structures
            for(i = 0; i < FileCount; i++)
                List[i].ItemToken = FileList[i].Token;
        }

        // Call post manager to display the menu
        Status = mPostMgr->DisplayPostMenu(gHiiHandle,
                                            STRING_TOKEN(STR_FILE_SELECT), // Change this
                                            0,
                                            List,
                                            (UINT16)FileCount,
                                            Selection);
    }
    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   UpdateFilePathString
//
// Description: To create the File Path string based on the file selected.
//
// Input:   CHAR16 *FilePath  - Buffer to fill with the file path
//          CHAR16 * CurFile  - current file selected
//          UINT16 idx        - Index of the file in the current directory
//
// Output:  CHAR16 *FilePath - Updated File Path
//
// Returns:
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID UpdateFilePathString(CHAR16 *FilePath, CHAR16 * CurFile, UINT16 idx)
{
    UINTN Length=0;

    if(Wcslen(FilePath))
    {
        if( idx==0 ) {
            if(Wcscmp(CurFile,L".."))  {
                EfiStrCat(FilePath,L"\\");
                EfiStrCat(FilePath,CurFile);
            }
            else {
                
                for ( Length = Wcslen(FilePath); ( Length!= 0 ) && (FilePath[Length-1] != L'\\') ; Length -- ); 
                    if ( Length )
                        FilePath[Length-1] = L'\0';
                    else
                        FilePath[Length] = L'\0'; 
            }
        }
        else {
            EfiStrCat(FilePath,L"\\");
            EfiStrCat(FilePath,CurFile);
        }
    }
    else {
        Wcscpy(FilePath,CurFile);
    }
}

EFI_STATUS FileBrowserLaunchFilePath(IN EFI_HANDLE *FileHandle, IN BOOLEAN ValidOption, OUT CHAR16 **outFilePath, OUT UINT8 **outFileBuf,OUT UINTN *size );

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FileBrowserLaunchFileSystem
//
// Description: To select the File System for the new boot option with the help of file browser.
//
// Input:   BOOLEAN bSelectFile - TRUE  - Select FSHandle and File path
//                                FALSE - Select only FSHandle
//
// Output:  Selected File System Index
//
// Returns: EFI_STATUS
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS FileBrowserLaunchFileSystem(BOOLEAN bSelectFile, OUT EFI_HANDLE **outFsHandle,OUT CHAR16 **outFilePath, OUT UINT8 **outFileBuf,OUT UINTN *size )
{
    EFI_STATUS Status;
    UINTN Count = 0;
    UINT16 i = 0;
    EFI_HANDLE *gSmplFileSysHndlBuf = NULL;    
    UINT16 gSelIdx=0;
    BOOLEAN ValidOption = FALSE;
    
    EFI_DEVICE_PATH_PROTOCOL *Dp = NULL;

    POSTMENU_TEMPLATE *PossibleFileSystems = NULL;

    if(mPostMgr == NULL)
        return EFI_UNSUPPORTED;
    
    // To launch the files from the selected file system
    if(!size)
        return EFI_INVALID_PARAMETER;

    // Locate all the simple file system devices in the system
    Status = pBS->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &Count, &gSmplFileSysHndlBuf);
    if(!EFI_ERROR(Status))
    {
        // allocate space to display all the simple file system devices
        PossibleFileSystems = EfiLibAllocateCopyPool(sizeof(POSTMENU_TEMPLATE)*Count,PossibleFileSystems);
        if(PossibleFileSystems != NULL)
        {
            // clear the allocated space
            MemSet(PossibleFileSystems, sizeof(POSTMENU_TEMPLATE)*Count, 0);
            for(i = 0; i < Count; i++)
            {
                // get the device path for each handle with a simple file system
                Status = pBS->HandleProtocol(gSmplFileSysHndlBuf[i], &gEfiDevicePathProtocolGuid, &Dp);
                if(!EFI_ERROR(Status))
                {
                    CHAR16 *Name;
                    CHAR8  *Name8;
                    // Get the name of the driver installed on the handle
                    // GetControllerName(gHandleBuffer[i],&Name);

                    Name8 = NULL;
                    Status  = DevicePathToStr(Dp, &Name8 );
                    Name = StrDup8to16(Name8);

                    // Add the name to the Hii Database
                    //PossibleFileSystems[i].ItemToken = AddStringToHii(Name);
                    PossibleFileSystems[i].ItemToken = HiiAddString(gHiiHandle, Name ); 

                    PossibleFileSystems[i].Attribute = AMI_POSTMENU_ATTRIB_FOCUS;
                    pBS->FreePool(Name);
                    pBS->FreePool(Name8);
                }
                else
                {
                    PossibleFileSystems[i].ItemToken = 0;
                    PossibleFileSystems[i].Attribute = AMI_POSTMENU_ATTRIB_HIDDEN;
                }
            }
            // Reset the item selected to be the first item
            gSelIdx = 0;

            // Display the post menu and wait for user input
            Status = mPostMgr->DisplayPostMenu(gHiiHandle,
                                                STRING_TOKEN(STR_FILE_SYSTEM),
                                                0,
                                                PossibleFileSystems,
                                                (UINT16)Count,
                                                &gSelIdx);

            // A valid item was selected by the user
            if(!EFI_ERROR(Status))
            {
                ValidOption = TRUE;
            }
        }
    }
    
    else {
        GetHiiString(gHiiHandle, STRING_TOKEN(STR_NO_VALID_FS_TITLE),  sizeof(StrTitle) ,StrTitle);
        GetHiiString(gHiiHandle, STRING_TOKEN(STR_NO_VALID_FS), sizeof(StrMessage),StrMessage);
        mPostMgr->DisplayMsgBox( StrTitle, StrMessage, MSGBOX_TYPE_OK, NULL );//EIP:41615  To display Warning message when there is no file system connected.
    }
    
    // Free the allocated menu list space
    if(PossibleFileSystems != NULL) 
        pBS->FreePool(PossibleFileSystems);

     *outFsHandle = gSmplFileSysHndlBuf[gSelIdx];

     *size = 0; 
     if(bSelectFile)
         Status = FileBrowserLaunchFilePath(*outFsHandle, ValidOption, outFilePath, outFileBuf,size );//EIP:41615 Returning the status of Filebrowselaunchfilepath

     if(gSmplFileSysHndlBuf != NULL) 
        pBS->FreePool(gSmplFileSysHndlBuf);

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   FileBrowserLaunchFilePath
//
// Description: To select the Boot file for the new boot option with the help of file browser.
//
// Input:   VOID
//
// Output:  File Path string
//
// Returns: EFI_STATUS
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS FileBrowserLaunchFilePath(IN EFI_HANDLE *FileHandle,IN BOOLEAN ValidOption, OUT CHAR16 **outFilePath, OUT UINT8 **outFileBuf,OUT UINTN *size )
{
    EFI_STATUS Status;

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFs = NULL;
    EFI_FILE_PROTOCOL *NewFs = NULL;
    FILE_TYPE *FileList = NULL;
    UINTN FileCount = 0;
    UINT16 i = 0;
    CHAR16 FilePath[120];
    EFI_FILE_PROTOCOL *gFs = NULL;

    // Attempt to locate the post manager protocol
    if(mPostMgr == NULL)
        return EFI_UNSUPPORTED;

    if( ValidOption == TRUE ) 
    {

        // Get the simple file system protocol 
        Status = pBS->HandleProtocol(FileHandle, &gEfiSimpleFileSystemProtocolGuid, &SimpleFs);
        if(!EFI_ERROR(Status))
        {
            // And open it and return the efi file protocol
            Status = SimpleFs->OpenVolume(SimpleFs, &gFs);
        }
    }
    else {
            return EFI_UNSUPPORTED;
    }

    // clean up the file list and strings used in getting the file system
    CleanFileTypes(&FileList, &FileCount);

    while(!EFI_ERROR(Status) && gFs != NULL)
    {
        i = 0;
        MemSet(FilePath, sizeof(FilePath), 0);

        // Create a list of the files in the current directory
        Status = CreateFileList(gFs, &FileList, &FileCount);
        if(!EFI_ERROR(Status))
        {
            // Display the list in a menu and allow the user to select
            Status = DisplayFileListMenu(FileList, FileCount, &i);
            if(!EFI_ERROR(Status))
            {
                // The user selected something, attempt to open it
                Status = gFs->Open(  gFs,
                                    &NewFs,
                                    FileList[i].Name,
                                    EFI_FILE_MODE_READ,
                                    0);

                // close the old file system protocol and set it to null
                gFs->Close(gFs);
                gFs = NULL;

                // Create the File Path based on the file selected
                UpdateFilePathString(FilePath, FileList[i].Name, i);

                // the newly selected item was opened correctly
                if(!EFI_ERROR(Status))
                {
                    // check what type was opened
                    if(FileList[i].Type != EFI_FILE_DIRECTORY)
                    {

                        
                        Status = pBS->AllocatePool(EfiBootServicesData,FileList[i].Size, (VOID**)outFileBuf);
                        if(!EFI_ERROR(Status))
                        {
                            *size = FileList[i].Size;
                            // The user selected something, attempt to open it
                            Status = NewFs->Read( NewFs, size, *outFileBuf); }

                        // the file was read, close the file system protocol and set it to null
                        NewFs->Close(NewFs);
                        NewFs = NULL;
                        //Swprintf_s (FileName, 50, L"%s", FileList[i].Name);
                        //ShowPostMsgBox( L"Selected Boot File Name", FileName, MSGBOX_TYPE_OK, &SelOpt );
                    }
                    gFs = NewFs;
                }
            }
        }

        if(FileCount <= 0) {
            GetHiiString(gHiiHandle, STRING_TOKEN(STR_NO_VALID_FILE_TITLE),sizeof(StrTitle), StrTitle);
            GetHiiString(gHiiHandle, STRING_TOKEN(STR_NO_VALID_FILE),sizeof(StrMessage),StrMessage);
            mPostMgr->DisplayMsgBox( StrTitle, StrMessage, MSGBOX_TYPE_OK, NULL );//EIP:41615 Warning message to show unavailability of the selected file
        }

        // clean the strings that were used and free allocated space
        CleanFileTypes(&FileList, &FileCount);

        if(Status == EFI_ABORTED) {
            return Status;//EIP:41615 Returning the status if its aborted.
        }
    }
    // Set the File path for the new boot option added.
    Status = pBS->AllocatePool(EfiBootServicesData, ((Wcslen(FilePath)+1)*sizeof(CHAR16)), (void**)outFilePath);
    Wcscpy (*outFilePath, FilePath);

    return Status;
}
#endif // #ifdef NO_SETUP_COMPILE

#if defined(SMIFLASH_COMPILE) && defined(PRESERVE_SECURE_VARIABLES) && PRESERVE_SECURE_VARIABLES==1

static EFI_GUID AmiNvramControlProtocolGuid = { 0xf7ca7568, 0x5a09, 0x4d2c, { 0x8a, 0x9b, 0x75, 0x84, 0x68, 0x59, 0x2a, 0xe2 } };
typedef EFI_STATUS (*SHOW_BOOT_TIME_VARIABLES)(BOOLEAN Show);

typedef struct{
    SHOW_BOOT_TIME_VARIABLES ShowBootTimeVariables;
} AMI_NVRAM_CONTROL_PROTOCOL;

static AMI_NVRAM_CONTROL_PROTOCOL *NvramControl = NULL;

// Array of pointers to Secure Variables in TSEG.
static UINT8* SecureFlashVar[6];
static UINTN SecureFlashVarSize[6];
static UINT32 SecureFlashVarAttr[6];
static UINT32 SecBootVar_Attr=0;
//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   PreserveSecureVariables
//
// Description: Save the PK-KEK-db-dbx & dbt
//
// Input:       NONE
//
// Output:      NONE
//
// Returns:     NONE
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID PreserveSecureVariables(VOID)
{
    EFI_STATUS  Status;
    UINT8       Index;
    EFI_GUID    *EfiVarGuid;
    UINTN       VarSize;

TRACE((TRACE_ALWAYS,"PreserveVar:\n"));
// 1. Preserve Secure Boot variables.
    Index = 0;
    while(SecureVariableFileName[Index] != NULL)
    {
        if(Index < 3) 
            EfiVarGuid = &gEfiImageSecurityDatabaseGuid;
        else
            EfiVarGuid = &gEfiGlobalVariableGuid;
        
        SecureFlashVar[Index] = NULL;
        SecureFlashVarSize[Index] = 0;
        SecureFlashVarAttr[Index] = 0;
        Status = pRS->GetVariable( SecureVariableFileName[Index], EfiVarGuid, &SecureFlashVarAttr[Index], &SecureFlashVarSize[Index], SecureFlashVar[Index]);
        if(Status == EFI_BUFFER_TOO_SMALL) {
            VarSize = SecureFlashVarSize[Index]+sizeof(AMI_EFI_VARIABLE_AUTHENTICATION_2);
            Status = pSmst->SmmAllocatePool(EfiRuntimeServicesData, VarSize, (void**)&SecureFlashVar[Index]);
        }
        if(!EFI_ERROR(Status)) {
            Status = pRS->GetVariable(SecureVariableFileName[Index], EfiVarGuid, &SecureFlashVarAttr[Index],&SecureFlashVarSize[Index], SecureFlashVar[Index]+sizeof(AMI_EFI_VARIABLE_AUTHENTICATION_2));

            SecureFlashVarSize[Index] = VarSize;
            FillAuthHdr(SecureFlashVar[Index], SET_SECURE_VARIABLE_SET);
      
TRACE((TRACE_ALWAYS,"%S (%r) Size=%x\n", SecureVariableFileName[Index], Status, SecureFlashVarSize[Index]));
        }
        Index++;
    }
// 2. Preserve Secure Boot Setup variables
    if (NvramControl == NULL)   // first time?
        NvramControl = GetSmstConfigurationTable(&AmiNvramControlProtocolGuid);
    if (NvramControl == NULL) 
        return;
    // Set "Show BootTime Variables" flag
    NvramControl->ShowBootTimeVariables(TRUE);

    VarSize = sizeof(SECURE_BOOT_SETUP_VAR);
    Status = pRS->GetVariable (AMI_SECURE_BOOT_SETUP_VAR,&guidSecurity,&SecBootVar_Attr,&VarSize,&SecureBootSetup);
TRACE((TRACE_ALWAYS,"SecureBootSetup (%r) Size=%x, SecBoot-%d, SecMode-%d,DefaultProvision-%d\n", Status, VarSize, SecureBootSetup.SecureBootSupport, SecureBootSetup.SecureBootMode,SecureBootSetup.DefaultKeyProvision));

    // Clear "Show Boot Time Variables"
    NvramControl->ShowBootTimeVariables(FALSE);

}

//<AMI_PHDR_START>
//----------------------------------------------------------------------
// Procedure:   RestoreSecureVariables
//
// Description: Restore previous PK-KEK-db-dbx &dbt
//
// Input:       NONE
//
// Output:      NONE
//
// Returns:     NONE
//
//----------------------------------------------------------------------
//<AMI_PHDR_END>
VOID RestoreSecureVariables (VOID)
{
    EFI_STATUS  Status;
    UINT8       Index;
    EFI_GUID    *EfiVarGuid;

TRACE((TRACE_ALWAYS,"RestoreVar:\n"));
// 1. Restore Secure Boot variables.
    Index = 0;
    while(SecureVariableFileName[Index] != NULL)
    {
        if(SecureFlashVar[Index] && SecureFlashVarSize[Index])
        {
            if(Index < 3) 
                EfiVarGuid = &gEfiImageSecurityDatabaseGuid;
            else
                EfiVarGuid = &gEfiGlobalVariableGuid;

            Status = pRS->SetVariable(SecureVariableFileName[Index], EfiVarGuid, SecureFlashVarAttr[Index], SecureFlashVarSize[Index], SecureFlashVar[Index] );
TRACE((TRACE_ALWAYS,"%S (%r) Size=%x Attr=%x\n", SecureVariableFileName[Index], Status, SecureFlashVarSize[Index], SecureFlashVarAttr[Index]));
            pSmst->SmmFreePool(SecureFlashVar[Index]);
            SecureFlashVar[Index] = NULL;
        }
        Index++;
    }
// 2. Restore Secure Boot Setup variables
    if (NvramControl == NULL) 
        return;
    // Set "Show BootTime Variables" flag
    NvramControl->ShowBootTimeVariables(TRUE);
    
    Status = pRS->SetVariable (AMI_SECURE_BOOT_SETUP_VAR, &guidSecurity,SecBootVar_Attr, sizeof(SECURE_BOOT_SETUP_VAR),&SecureBootSetup);
TRACE((TRACE_ALWAYS,"SecureBootSetup (%r)\n", Status));

    // Clear "Show Boot Time Variables"
    NvramControl->ShowBootTimeVariables(FALSE);

}
#endif //#if SMIFLASH_COMPILE defined(PRESERVE_SECURE_VARIABLES) && PRESERVE_SECURE_VARIABLES==1

#ifdef NO_SETUP_COMPILE
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SecureMod_Init
//
// Description: Entry point of Secure Module DXE driver
//
// Input:       EFI_HANDLE           ImageHandle,
//              EFI_SYSTEM_TABLE     *SystemTable
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
SecureBootMod_Init (
    IN EFI_HANDLE         ImageHandle,
    IN EFI_SYSTEM_TABLE   *SystemTable
)
{
    EFI_STATUS Status;
    UINTN      DataSize;
    UINT8      Byte;
    UINT32     Attributes;
    VOID      *pHobList;
    static EFI_GUID gHobListGuid  = HOB_LIST_GUID;

    InitAmiLib(ImageHandle, SystemTable);
    //
    // Look up for Secure Boot policy in "SecureBootSetup" variable. If not defined - create one with SDL defaults
    //
    DataSize = sizeof(SECURE_BOOT_SETUP_VAR);
    Status = pRS->GetVariable (AMI_SECURE_BOOT_SETUP_VAR, &guidSecurity,&Attributes,&DataSize,&SecureBootSetup);
TRACE((TRACE_ALWAYS,"SecureBootSetup (%r) Attrib=%x,\nSecBoot-%d, SecMode-%d, DefaultProvision-%d, BackDoorVendorKeyChange-%d\n", 
    Status, Attributes, SecureBootSetup.SecureBootSupport, SecureBootSetup.SecureBootMode, SecureBootSetup.DefaultKeyProvision, SecureBootSetup.BackDoorVendorKeyChange));
    // Default variable is created with RT attribute which violates 
    // Intel's Secure Boot technical advisory #527669 and MS Windows Secure Boot requirements
    if((!EFI_ERROR(Status) && 
        (Attributes & EFI_VARIABLE_RUNTIME_ACCESS)==EFI_VARIABLE_RUNTIME_ACCESS)
        || Status == EFI_NOT_FOUND) // Var is not yet initialized
    {
        // Clear RT attributes if set for "SecureBootSetup" 
        pRS->SetVariable (AMI_SECURE_BOOT_SETUP_VAR, &guidSecurity, Attributes, 0, NULL);
        pRS->SetVariable (AMI_SECURE_BOOT_SETUP_VAR, &guidSecurity,(Attributes & ~EFI_VARIABLE_RUNTIME_ACCESS), DataSize, &SecureBootSetup);
    }
    
    mBackDoorVendorKeyChange = SecureBootSetup.BackDoorVendorKeyChange;
    
    // Provisioning of secure boot defaults
    if(SecureBootSetup.DefaultKeyProvision == 1) 
    {
        DataSize = 1;
        Status = pRS->GetVariable(EFI_SETUP_MODE_NAME, &gEfiGlobalVariableGuid, NULL, &DataSize, &Byte);
        if(!EFI_ERROR(Status) && Byte == SETUP_MODE) 
        {
            Status = InstallSecureVariables(SET_NV_DEFAULT_KEYS);
            // Status of last key to be installed - PK
            if(!EFI_ERROR(Status) && SecureBootSetup.SecureBootSupport) 
            {
                // Issue reset only if in User Mode and SecureBootSupport flag is enabled
                Status = pRS->GetVariable(EFI_SETUP_MODE_NAME, &gEfiGlobalVariableGuid, NULL, &DataSize, &Byte);
                if(!EFI_ERROR(Status) && Byte == USER_MODE) 
                    //Get Boot Mode
                    pHobList = GetEfiConfigurationTable(SystemTable, &gHobListGuid);
                    if (pHobList)
                    {
                        if (((EFI_HOB_HANDOFF_INFO_TABLE*)pHobList)->BootMode!=BOOT_IN_RECOVERY_MODE && 
                            ((EFI_HOB_HANDOFF_INFO_TABLE*)pHobList)->BootMode!=BOOT_ON_FLASH_UPDATE
                        ) 
                            pRS->ResetSystem(EfiResetCold, Status, 0, NULL);
                    }
            }
        }
    }
    InstallSecureVariables(SET_RT_DEFAULT_KEYS);

    return EFI_SUCCESS;
}
#endif // #ifdef NO_SETUP_COMPILE
#endif // #ifndef TSE_FOR_APTIO_4_50

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2015, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
