//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//*************************************************************************
// $Header: /Alaska/SOURCE/Modules/TcgNext/Common/Tpm20PlatformDxe/Tpm20PlatformDxe.c 2     10/09/13 6:32p Fredericko $
//
// $Revision: 2 $
//
// $Date: 10/09/13 6:32p $
//*************************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/TcgNext/Common/Tpm20PlatformDxe/Tpm20PlatformDxe.c $
// 
// 2     10/09/13 6:32p Fredericko
// 
// 1     10/08/13 12:06p Fredericko
// Initial Check-In for Tpm-Next module
// 
// 5     10/03/13 2:52p Fredericko
// 
// 4     9/16/13 1:37p Fredericko
// TPM 2.0 UEFI preboot fixes. 
// 
// 3     8/30/13 11:03p Fredericko
// 
// 2     7/11/13 6:16p Fredericko
// [TAG]  		EIP120969
// [Category]  	New Feature
// [Description]  	TCG (TPM20).
// 
// 1     7/10/13 5:57p Fredericko
// [TAG]  		EIP120969
// [Category]  	New Feature
// [Description]  	TCG (TPM20)
// 
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:	
//
// Description:	
//
//<AMI_FHDR_END>
//*************************************************************************
#include "Tpm20PlatformDxe.h"
#include <IndustryStandard\Smbios.h>
#include <IndustryStandard\PeImage.h>
#include <Uefi.h>
#include <ImageAuthentication.h>
#include <IndustryStandard\PeImage.h>
#include <AmiTcg\TrEEProtocol.h>
#include <AmiTcg\tcg.h>
#include <Protocol\DiskIo.h>
#include <Protocol\BlockIo.h>
#include "Protocol/CpuIo.h"
#include "Protocol/FirmwareVolume2.h"
#include "protocol/AMIPostMgr.h"
#include <Library/DebugLib.h>
#include <Tpm20PlatformDxeStrDefs.h>
#include "protocol\TcgPlatformSetupPolicy.h"
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/HiiPackageList.h>
#include <Protocol/HiiDatabase.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include "Protocol/DevicePath.h"
#include <Library/DevicePathLib.h>
#include <Library/HiiLib.h>
#include <Library/IoLib.h>
#include <..\TcgDxe\TrEEDxe.h>


EFI_GUID gEfiImageSecurityDatabaseguid =  EFI_IMAGE_SECURITY_DATABASE_GUID;
EFI_GUID AmitcgefiOsVariableGuid       = AMI_TCG_EFI_OS_VARIABLE_GUID;

#define AMI_VALID_BOOT_IMAGE_CERT_TBL_GUID \
    { 0x6683D10C, 0xCF6E, 0x4914, 0xB5, 0xB4, 0xAB, 0x8E, 0xD7, 0x37, 0x0E, 0xD7 }

#define BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID \
        {0xdbc9fd21, 0xfad8, 0x45b0, 0x9e, 0x78, 0x27, 0x15, 0x88, 0x67, 0xcc, 0x93}

EFI_GUID    gBdsAllDriversConnectedProtocolGuid = BDS_ALL_DRIVERS_CONNECTED_PROTOCOL_GUID;
EFI_GUID    gAmiPostManagerProtocolGuid = AMI_POST_MANAGER_PROTOCOL_GUID;



EFI_STATUS HandleTpm20Setup();

EFI_STATUS Tpm2GetRandom(
    UINTN                   AuthSize,
    UINT8*                  pOutBuf
);

EFI_STATUS
EFIAPI
Tpm2HierarchyChangeAuth (
  IN TPMI_RH_HIERARCHY_AUTH     AuthHandle,
  IN TPMS_AUTH_SESSION_COMMAND *AuthSession,
  IN TPM2B_AUTH                *NewAuth
  );

EFI_GUID ZeroGuid = {0,0,0,0,0,0,0,0,0,0,0};

EFI_GUID  gEfiSmbiosTableGuid = EFI_SMBIOS_TABLE_GUID;
EFI_GUID  FlagsStatusguid = AMI_TCG_CONFIRMATION_FLAGS_GUID;

UINTN      mMeasureGptCount = 0;
EFI_TREE_PROTOCOL   *TrEEProtocolInstance = NULL;
static UINT8              PpiRequest;

EFI_HII_HANDLE            gHiiHandle=0;
AMI_POST_MANAGER_PROTOCOL *pAmiPostMgr = NULL;
EFI_HANDLE PlatformProtocolHandle;
static PERSISTENT_BIOS_TPM_FLAGS  TpmNvflags;

#pragma pack (1)
typedef struct
{
    EFI_PHYSICAL_ADDRESS PostCodeAddress;
    #if x64_TCG
    UINT64               PostCodeLength;
    #else
    UINTN                PostCodeLength;
    #endif
} EFI_TCG_EV_POST_CODE;

typedef struct
{
    EFI_TCG_PCR_EVENT_HEADER Header;
    EFI_TCG_EV_POST_CODE     Event;
} PEI_EFI_POST_CODE;
#pragma pack()


//
//
// Data Table definition
//
typedef struct _AMI_VALID_CERT_IN_SIG_DB {
  UINT32          SigOffset;
  UINT32          SigLength;
} AMI_VALID_CERT_IN_SIG_DB;

EFI_STATUS
EFIAPI
GetRandomAuthPassword(
	IN     UINT16    RNGValueLength,
	IN OUT UINT8    *RNGValue
  )
{
  EFI_STATUS   Status = EFI_SUCCESS;
  return Status;
}


EFI_STATUS
EFIAPI
TpmRevokeTrust (
  )
{
   EFI_STATUS     Status = EFI_SUCCESS;
   return Status;
}


#define GET_HOB_TYPE( Hob )     ((Hob).Header->HobType)
#define GET_HOB_LENGTH( Hob )   ((Hob).Header->HobLength)
#define GET_NEXT_HOB( Hob )     ((Hob).Raw + GET_HOB_LENGTH( Hob ))
#define END_OF_HOB_LIST( Hob )  (GET_HOB_TYPE( Hob ) == \
                                 EFI_HOB_TYPE_END_OF_HOB_LIST)

//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   GetHob
//
// Description: Find instance of a HOB type in a HOB list
//
//
// Input:       IN UINT16  Type,
//              IN VOID    *HobStart
//
// Output:      VOID*
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
VOID* GetHob(
    IN UINT16 Type,
    IN VOID   *HobStart )
{
    EFI_PEI_HOB_POINTERS Hob;

    Hob.Raw = HobStart;

    //
    // Return input if not found
    //
    if ( HobStart == NULL )
    {
        return HobStart;
    }

    //
    // Parse the HOB list, stop if end of list or matching type found.
    //
    while ( !END_OF_HOB_LIST( Hob ))
    {
        if ( Hob.Header->HobType == Type )
        {
            break;
        }

        Hob.Raw = GET_NEXT_HOB( Hob );
    }

    //
    // Return input if not found
    //
    if ( END_OF_HOB_LIST( Hob ))
    {
        return HobStart;
    }

    return (VOID*)(Hob.Raw);
}



EFI_STATUS
MeasureSeparatorEvent (
  IN  UINT32  PCRIndex
)
{    
    UINT32           EventData;
    TrEE_EVENT       *Tpm20Event=NULL;
    UINT64           Flags = 0;
    EFI_STATUS       Status;

    if(TrEEProtocolInstance == NULL) return EFI_NOT_FOUND;
  
    gBS->AllocatePool(EfiBootServicesData, (sizeof(TrEE_EVENT_HEADER) + \
                      sizeof(UINT32) + sizeof(UINT32)), &Tpm20Event);

    if(Tpm20Event==NULL) return EFI_OUT_OF_RESOURCES;

    EventData = 0;
    Tpm20Event->Size  = sizeof(TrEE_EVENT_HEADER) + sizeof(UINT32) + sizeof(EventData);
    Tpm20Event->Header.HeaderSize = sizeof(TrEE_EVENT_HEADER);
    Tpm20Event->Header.HeaderVersion = 1;
    Tpm20Event->Header.PCRIndex    = PCRIndex;
    Tpm20Event->Header.EventType   = EV_SEPARATOR;

    gBS->CopyMem ((UINT32 *)((UINTN)&Tpm20Event->Event[0]),
                 &EventData,
                 sizeof(UINT32));

    Status = TrEEProtocolInstance->HashLogExtendEvent(TrEEProtocolInstance,
                                           Flags, (EFI_PHYSICAL_ADDRESS)&EventData, (UINT64)sizeof(EventData),
                                           Tpm20Event);

    gBS->FreePool(Tpm20Event);  
    
    return Status;
}



EFI_STATUS
MeasureCertificate(UINTN sizeOfCertificate, 
                   UINT8 *pterCertificate)
{
    EFI_STATUS                Status;
    TrEE_EVENT               *Tcg20Event;
    EFI_VARIABLE_DATA        *VarLog = NULL;
    BOOLEAN                   AlreadyMeasuredCert = FALSE;
    UINTN                     i=0;
    UINTN                     VarNameLength;
    static BOOLEAN            initialized = 0;
    static TPM_DIGEST         digestTrackingArray[5];
    static TPM_DIGEST         zeroDigest;
    UINT8                     *tempDigest = NULL;
    UINT64                    HashedDataLen = 20; 
    SHA1_CTX                  Sha1Ctx;
    TCG_DIGEST                Sha1Digest;
    UINT64                    Flags = 0;
    UINT32                    EventSize = 0;
    UINT8                     *EventDataPtr;

    if(TrEEProtocolInstance == NULL) return EFI_NOT_FOUND;

    VarNameLength = StrLen(L"db");

    EventSize = (UINT32)( sizeof (*VarLog) + VarNameLength 
                              * sizeof (CHAR16) + sizeOfCertificate) - 3;

    gBS->AllocatePool(EfiBootServicesData, (sizeof(TrEE_EVENT_HEADER) + \
                      sizeof(UINT32) + EventSize), &Tcg20Event);

    if(Tcg20Event==NULL) return EFI_OUT_OF_RESOURCES;

    if(!initialized)
    {
        for(i=0;i<5; i++)
        {
            gBS->SetMem(digestTrackingArray[i].digest,20, 0);
        }
        gBS->SetMem(zeroDigest.digest,20, 0);
        initialized = TRUE;
    }

    Tcg20Event->Size  = sizeof(TrEE_EVENT_HEADER) + sizeof(UINT32) + EventSize;
    Tcg20Event->Header.HeaderSize = sizeof(TrEE_EVENT_HEADER);
    Tcg20Event->Header.HeaderVersion = 1;
    Tcg20Event->Header.PCRIndex    = 7;
    Tcg20Event->Header.EventType   = 0x800000E0;
       
    Status = gBS->AllocatePool(EfiBootServicesData, EventSize, &VarLog);

    if ( VarLog == NULL ){
        return EFI_OUT_OF_RESOURCES;
    }
        
    VarLog->VariableName       = gEfiImageSecurityDatabaseGuid;
    VarLog->UnicodeNameLength  = VarNameLength;
    VarLog->VariableDataLength = sizeOfCertificate;

    gBS->CopyMem((CHAR16*)(VarLog->UnicodeName),
                L"db",
                VarNameLength * sizeof (CHAR16));
   
    gBS->CopyMem((CHAR16*)(VarLog->UnicodeName) + VarNameLength,
                 pterCertificate,
                 sizeOfCertificate);

    //before extending verify if we have already measured it.
    SHA1Init(&Sha1Ctx);
    
    SHA1Update(&Sha1Ctx,
                VarLog,
           (u32)EventSize);

    SHA1Final((unsigned char *)&Sha1Digest.digest, &Sha1Ctx);

    for(i=0; i<5; i++)
    {
        //tempDigest
        if(!CompareMem(digestTrackingArray[i].digest, &Sha1Digest, 20))
        return EFI_SUCCESS; //already measured

        if(!CompareMem(digestTrackingArray[i].digest, zeroDigest.digest, 20))
        break; //we need to measure
    }

    gBS->CopyMem(digestTrackingArray[i].digest, &Sha1Digest, 20);
    
    EventDataPtr = (UINT8 *)Tcg20Event;

    EventDataPtr += sizeof(TrEE_EVENT_HEADER) + sizeof(UINT32);

    gBS->CopyMem(EventDataPtr, VarLog, EventSize);
    
    Status = TrEEProtocolInstance->HashLogExtendEvent(TrEEProtocolInstance,
                                            Flags, (EFI_PHYSICAL_ADDRESS)(UINT8 *)(UINTN)VarLog, (UINT64)EventSize,
                                            Tcg20Event);
    return Status;
}




EFI_STATUS FindandMeasureSecureBootCertificate()
{
    EFI_STATUS      Status;
    UINTN           VarSize  = 0;
    UINTN           i=0;
    UINT8           *SecureDBBuffer = NULL;
    UINT8           *CertificateBuffer = NULL;
    UINTN           SizeofCerificate = 0;
    EFI_GUID        Certificateguid = AMI_VALID_BOOT_IMAGE_CERT_TBL_GUID;
    AMI_VALID_CERT_IN_SIG_DB    *CertInfo;
    UINT8           *CertOffsetPtr = NULL;
   
    VarSize = 0;

    Status   = gRT->GetVariable(L"db",
                    &gEfiImageSecurityDatabaseGuid,
                    NULL,
                    &VarSize,
                    NULL);

    if ( Status != EFI_BUFFER_TOO_SMALL )
    {
        return EFI_NOT_FOUND;
    }

    Status = gBS->AllocatePool(EfiBootServicesData, VarSize, &SecureDBBuffer);
    
    if ( SecureDBBuffer != NULL )
    {
        Status = gRT->GetVariable(L"db",
                        &gEfiImageSecurityDatabaseGuid,
                        NULL,
                        &VarSize,
                        SecureDBBuffer);

        if ( EFI_ERROR( Status ))
        {
            gBS->FreePool( SecureDBBuffer  );
            SecureDBBuffer = NULL;
            return EFI_NOT_FOUND;
        }
    }else{
        return EFI_OUT_OF_RESOURCES;
    }

    //we need to find the pointer in the EFI system table and work from 
    //there
    CertInfo = NULL;
    EfiGetSystemConfigurationTable(&Certificateguid, &CertInfo );
    if(CertInfo == NULL){
     return EFI_NOT_FOUND;
    }
    if(CertInfo->SigLength == 0){
     return EFI_NOT_READY;
    }

    CertOffsetPtr = NULL;
    CertOffsetPtr = (SecureDBBuffer + CertInfo->SigOffset);
    MeasureCertificate((UINTN)CertInfo->SigLength,CertOffsetPtr);
    
    if(SecureDBBuffer!=NULL){
        gBS->FreePool( SecureDBBuffer  );
    }
    
    return Status;
}


UINTN Tpm20AsciiStrLen (
    IN CHAR8 *String)
{
  UINTN Length;
  for (Length = 0; *String != '\0'; String++, Length++);
  return Length;
}



EFI_STATUS
EFIAPI
MeasureAction (
  IN  CHAR8 *String
)
{
  TCG_PCR_EVENT2_HDR                 TcgEvent;
  AMI_INTERNAL_HLXE_PROTOCOL        *InternalHLXE = NULL;
  EFI_GUID                          gEfiAmiHLXEGuid =  AMI_PROTOCOL_INTERNAL_HLXE_GUID;
  EFI_STATUS                        Status=EFI_SUCCESS;

  TcgEvent.PCRIndex     = 5;
  TcgEvent.EventType    = EV_EFI_ACTION;
  TcgEvent.EventSize    = (UINT32)Tpm20AsciiStrLen (String);

  Status = gBS->LocateProtocol(&gEfiAmiHLXEGuid, NULL, &InternalHLXE);
  if(EFI_ERROR(Status))return Status;

  Status = InternalHLXE->AmiHashLogExtend2(TrEEProtocolInstance, (UINT8*)String, 0, TcgEvent.EventSize, &TcgEvent, (UINT8*)String);
  return Status;
}





EFI_STATUS
EFIAPI
TreeMeasurePeImage (
  IN      BOOLEAN                   BootPolicy,
  IN      EFI_PHYSICAL_ADDRESS      ImageAddress,
  IN      UINTN                     ImageSize,
  IN      UINTN                     LinkTimeBase,
  IN      UINT16                    ImageType,
  IN      EFI_HANDLE                DeviceHandle,
  IN      EFI_DEVICE_PATH_PROTOCOL  *FilePath
  )
{

  EFI_STATUS                        Status;
  TCG_PCR_EVENT2_HDR                TcgEvent;
  UINT8                             *EventData = NULL;
  EFI_IMAGE_LOAD_EVENT              *ImageLoad;
  EFI_DEVICE_PATH_PROTOCOL          *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL          *FullPath;
  UINT32                            FullPathSize;
  SHA1_CTX                          Sha1Ctx;
  EFI_IMAGE_DOS_HEADER              *DosHdr;
  UINT32                            PeCoffHeaderOffset;
  EFI_IMAGE_NT_HEADERS64            *Hdr;
  EFI_IMAGE_SECTION_HEADER          *Section;
  UINT8                             *HashBase;
  UINTN                             HashSize;
  UINTN                             SumOfBytesHashed;
  EFI_IMAGE_SECTION_HEADER          *SectionHeader;
  UINTN                             Index, iPos;
  TCG_DIGEST                        Sha1Digest;
  AMI_INTERNAL_HLXE_PROTOCOL        *InternalHLXE = NULL;
  EFI_GUID                          gEfiAmiHLXEGuid =  AMI_PROTOCOL_INTERNAL_HLXE_GUID;
  TCG_PLATFORM_SETUP_PROTOCOL       *ProtocolInstance;
  EFI_GUID                          Policyguid = TCG_PLATFORM_SETUP_POLICY_GUID;
  SHA2_CTX                          Sha2Ctx;
  unsigned char                     Sha2DigestArray[32];  
  UINT32                            HashPolicy;


  
  Status = gBS->LocateProtocol (&Policyguid,  NULL, &ProtocolInstance);
  if (EFI_ERROR (Status)) {
      return 0;
  }

  HashPolicy = ProtocolInstance->ConfigFlags.HashPolicy;
    
      
  ImageLoad     = NULL;
  FullPath      = NULL;
  SectionHeader = NULL;
  FullPathSize  = 0;

  DEBUG ((-1, "TreeMeasurePeImage Entry\n"));

  if (DeviceHandle != NULL) {
    //
    // Skip images loaded from FVs
    //
    Status = gBS->OpenProtocol (
                    DeviceHandle,
                    &gEfiFirmwareVolume2ProtocolGuid,
                    NULL,
                    NULL,
                    NULL,
                    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                    );

    if (!EFI_ERROR (Status)) {
      goto Done;
    }
    ASSERT (Status == EFI_UNSUPPORTED);

    //
    // Get device path for the device handle
    //
    Status = gBS->HandleProtocol (
                    DeviceHandle,
                    &gEfiDevicePathProtocolGuid,
                    &DevicePath
                    );
    if (EFI_ERROR (Status)) {
      FullPathSize = (UINT32)GetDevicePathSize (FullPath); 
    }else{                 
      FullPath     = AppendDevicePath (DevicePath, FilePath);
      FullPathSize = (UINT32)GetDevicePathSize (FullPath); 
    }
  }

    //Allocate Event log memory
  Status = gBS ->AllocatePool(EfiBootServicesData, ((sizeof (*ImageLoad)
                                - sizeof (ImageLoad->DevicePath)) + FullPathSize), &EventData);

  if(EFI_ERROR(Status))return Status;
  //
  // Determine destination PCR by BootPolicy
  //
  TcgEvent.EventSize  = sizeof (*ImageLoad) - sizeof (ImageLoad->DevicePath);
  TcgEvent.EventSize += FullPathSize;

  switch (ImageType) {
    case EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION:
      TcgEvent.PCRIndex   = 4;
      TcgEvent.EventType = EV_EFI_BOOT_SERVICES_APPLICATION;
      break;
    case EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
      TcgEvent.PCRIndex   = 2;
      TcgEvent.EventType = EV_EFI_BOOT_SERVICES_DRIVER;
      goto Done; 
      break;
    case EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
      TcgEvent.PCRIndex   = 2;
      TcgEvent.EventType = EV_EFI_RUNTIME_SERVICES_DRIVER;
      goto Done; 
      break;
    default:
      TcgEvent.EventType = ImageType;
      Status = EFI_UNSUPPORTED;
      goto Done;
  }

  Status = gBS ->AllocatePool(EfiBootServicesData,TcgEvent.EventSize, &ImageLoad);

  if (ImageLoad == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }

  ImageLoad->ImageLocationInMemory = ImageAddress;
  ImageLoad->ImageLengthInMemory   = ImageSize;
  ImageLoad->ImageLinkTimeAddress  = LinkTimeBase;
  ImageLoad->LengthOfDevicePath    = FullPathSize;
  gBS->CopyMem( ImageLoad->DevicePath, FullPath,  FullPathSize );

  //
  // Check PE/COFF image
  //
  DosHdr = (EFI_IMAGE_DOS_HEADER *)(UINTN)ImageAddress;
  PeCoffHeaderOffset = 0;
  if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
    PeCoffHeaderOffset = DosHdr->e_lfanew;
  }
  if (((EFI_TE_IMAGE_HEADER *)((UINT8 *)(UINTN)ImageAddress + PeCoffHeaderOffset))->Signature 
       == EFI_TE_IMAGE_HEADER_SIGNATURE) {
    goto Done;
  }

  //
  // PE/COFF Image Measurement
  //
  //    NOTE: The following codes/steps are based upon the authenticode image hashing in 
  //      PE/COFF Specification 8.0 Appendix A.
  //      
  //

  // 1.	Load the image header into memory.
  
  // 2.	Initialize a SHA hash context.
  if(HashPolicy == TREE_BOOT_HASH_ALG_SHA1){ SHA1Init(&Sha1Ctx);}
  
  if(HashPolicy == TREE_BOOT_HASH_ALG_SHA256){sha256_init( &Sha2Ctx );};
  

  //
  // Measuring PE/COFF Image Header; 
  // But CheckSum field and SECURITY data directory (certificate) are excluded
  //
  Hdr   = (EFI_IMAGE_NT_HEADERS64 *)((UINT8 *)(UINTN)ImageAddress + PeCoffHeaderOffset);

  //
  // 3.	Calculate the distance from the base of the image header to the image checksum address.
  // 4.	Hash the image header from its base to beginning of the image checksum.
  //
  HashBase = (UINT8 *)(UINTN)ImageAddress;
  HashSize = (UINTN) ((UINT8 *)(&Hdr->OptionalHeader.CheckSum) - HashBase);
  if(HashPolicy == TREE_BOOT_HASH_ALG_SHA1){
     SHA1Update(&Sha1Ctx,
                HashBase,
                (u32)HashSize);
  }
  
  if(HashPolicy == TREE_BOOT_HASH_ALG_SHA256){
     sha256_process( &Sha2Ctx, HashBase, (u32)HashSize );
  }



  //
  // 5.	Skip over the image checksum (it occupies a single ULONG).
  // 6.	Get the address of the beginning of the Cert Directory.
  // 7.	Hash everything from the end of the checksum to the start of the Cert Directory.
  //
  HashBase = (UINT8 *) &Hdr->OptionalHeader.CheckSum + sizeof (UINT32);
  HashSize = (UINTN) ((UINT8 *)(&Hdr->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY]) - HashBase);

  if(HashPolicy == TREE_BOOT_HASH_ALG_SHA1){
     SHA1Update(&Sha1Ctx,
                HashBase,
                (u32)HashSize);
  }
  
  if(HashPolicy == TREE_BOOT_HASH_ALG_SHA256){
     sha256_process( &Sha2Ctx, HashBase, (u32)HashSize );
  }

  //
  // 8.	Skip over the Cert Directory. (It is sizeof(IMAGE_DATA_DIRECTORY) bytes.)
  // 9.	Hash everything from the end of the Cert Directory to the end of image header.
  //
  HashBase = (UINT8 *) &Hdr->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY + 1];
  HashSize = Hdr->OptionalHeader.SizeOfHeaders - 
             (UINTN) ((UINT8 *)(&Hdr->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY + 1]) - (UINT8 *)(UINTN)ImageAddress);
  
  if(HashPolicy == TREE_BOOT_HASH_ALG_SHA1){
     SHA1Update(&Sha1Ctx,
                HashBase,
                (u32)HashSize);
  }
  
  if(HashPolicy == TREE_BOOT_HASH_ALG_SHA256){
     sha256_process( &Sha2Ctx, HashBase, (u32)HashSize );
  }


  //
  // 10. Set the SUM_OF_BYTES_HASHED to the size of the header 
  //
  SumOfBytesHashed = Hdr->OptionalHeader.SizeOfHeaders;

  //
  // 11. Build a temporary table of pointers to all the IMAGE_SECTION_HEADER 
  //     structures in the image. The 'NumberOfSections' field of the image 
  //     header indicates how big the table should be. Do not include any 
  //     IMAGE_SECTION_HEADERs in the table whose 'SizeOfRawData' field is zero.   
  //
  gBS ->AllocatePool(EfiBootServicesData,sizeof (EFI_IMAGE_SECTION_HEADER) * Hdr->FileHeader.NumberOfSections, &SectionHeader);

  if(SectionHeader==NULL)return EFI_OUT_OF_RESOURCES;
  gBS->SetMem(SectionHeader, (sizeof (EFI_IMAGE_SECTION_HEADER) * Hdr->FileHeader.NumberOfSections), 0);

  //
  // 12.	Using the 'PointerToRawData' in the referenced section headers as 
  //      a key, arrange the elements in the table in ascending order. In other 
  //      words, sort the section headers according to the disk-file offset of 
  //      the section.
  //
  Section = (EFI_IMAGE_SECTION_HEADER *) (
               (UINT8 *)(UINTN)ImageAddress +
               PeCoffHeaderOffset +
               sizeof(UINT32) + 
               sizeof(EFI_IMAGE_FILE_HEADER) + 
               Hdr->FileHeader.SizeOfOptionalHeader
               );  
  for (Index = 0; Index < Hdr->FileHeader.NumberOfSections; Index++) {
    iPos = Index;
    while ((iPos > 0) && (Section->PointerToRawData < SectionHeader[iPos - 1].PointerToRawData)) {
      gBS->CopyMem (&SectionHeader[iPos], &SectionHeader[iPos - 1], sizeof(EFI_IMAGE_SECTION_HEADER));
      iPos--;
    }
      gBS->CopyMem( &SectionHeader[iPos], Section,
                    sizeof(EFI_IMAGE_SECTION_HEADER));
    Section += 1;    
  }
  
  //
  // 13.	Walk through the sorted table, bring the corresponding section 
  //      into memory, and hash the entire section (using the 'SizeOfRawData' 
  //      field in the section header to determine the amount of data to hash).
  // 14.	Add the section's 'SizeOfRawData' to SUM_OF_BYTES_HASHED .
  // 15.	Repeat steps 13 and 14 for all the sections in the sorted table.
  //
  for (Index = 0; Index < Hdr->FileHeader.NumberOfSections; Index++) {
    Section  = (EFI_IMAGE_SECTION_HEADER *) &SectionHeader[Index];
    if (Section->SizeOfRawData == 0) {
      continue;
    }
    HashBase = (UINT8 *)(UINTN)ImageAddress + Section->PointerToRawData;
    HashSize = (UINTN) Section->SizeOfRawData;

    if(HashPolicy == TREE_BOOT_HASH_ALG_SHA1){
     SHA1Update(&Sha1Ctx,
                HashBase,
                (u32)HashSize);
    }
    
    if(HashPolicy == TREE_BOOT_HASH_ALG_SHA256){
     sha256_process( &Sha2Ctx, HashBase, (u32)HashSize );
    }


    SumOfBytesHashed += HashSize;
  }    

  //
  // 16.	If the file size is greater than SUM_OF_BYTES_HASHED, there is extra
  //      data in the file that needs to be added to the hash. This data begins 
  //      at file offset SUM_OF_BYTES_HASHED and its length is:
  //             FileSize  -  (CertDirectory->Size)
  //
  if (ImageSize > SumOfBytesHashed) {
    HashBase = (UINT8 *)(UINTN)ImageAddress + SumOfBytesHashed;
    HashSize = (UINTN)(ImageSize -
               Hdr->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY].Size -
               SumOfBytesHashed);
    
    if(HashPolicy == TREE_BOOT_HASH_ALG_SHA1){
     SHA1Update(&Sha1Ctx,
                HashBase,
                (u32)HashSize);
    }
    
    if(HashPolicy == TREE_BOOT_HASH_ALG_SHA256){
     sha256_process( &Sha2Ctx, HashBase, (u32)HashSize );
    }
  }

  //
  // 17.	Finalize the SHA hash.
  //
  if(HashPolicy == TREE_BOOT_HASH_ALG_SHA1){
    SHA1Final(Sha1Digest.digest, &Sha1Ctx);
    gBS->CopyMem(&TcgEvent.Digests.digests.sha1, Sha1Digest.digest, SHA1_DIGEST_SIZE);
  }
  
  if(HashPolicy == TREE_BOOT_HASH_ALG_SHA256){
    sha256_done( &Sha2Ctx, Sha2DigestArray );
    gBS->CopyMem(&TcgEvent.Digests.digests.sha256, Sha2DigestArray, SHA256_DIGEST_SIZE);
  }

  //
  // HashLogExtendEvent 
  //
  gBS->CopyMem(EventData, ImageLoad, TcgEvent.EventSize);

  Status = gBS->LocateProtocol(&gEfiAmiHLXEGuid, NULL, &InternalHLXE);
  if(EFI_ERROR(Status))return Status;

  InternalHLXE->AmiHashLogExtend2(TrEEProtocolInstance, NULL, 0, 0, &TcgEvent, EventData);

  FindandMeasureSecureBootCertificate();

Done:
  if (ImageLoad != NULL) {
    gBS->FreePool (ImageLoad);
  }

  if (FullPathSize > 0) {
   gBS->FreePool (FullPath);
  }

  if (SectionHeader != NULL) {
    gBS->FreePool (SectionHeader);
  }
  return Status;
}



//*******************************************************************************
//<AMI_PHDR_START>
//
// Procedure:   FindAndMeasureDxeFWVol
//
// Description: 
//
// Input:      
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//******************************************************************************
EFI_STATUS FindAndMeasureDxeFWVol()
{
    EFI_STATUS              Status;
    EFI_GUID                      NameGuid =\
                            {0x7739f24c, 0x93d7, 0x11d4,\
                             0x9a, 0x3a, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d};
    UINTN                         Size;
    void                          *Buffer = NULL;
    VOID                          *HobStart;
    UINTN                          TableEntries;
    EFI_PEI_HOB_POINTERS           FirmwareVolumeHob;
    BOOLEAN                        Found = FALSE;
    TrEE_EVENT                     *Tcg20Event = NULL;
    EFI_TCG_EV_POST_CODE           EventData; 


   if(TrEEProtocolInstance == NULL) return EFI_NOT_FOUND;  

   Status = gBS->AllocatePool(EfiBootServicesData, (sizeof(TrEE_EVENT_HEADER) + \
                      sizeof(UINT32) + sizeof(EventData)), &Tcg20Event);
   
   if(EFI_ERROR(Status) || (Tcg20Event == NULL))return Status;
    
   
    TableEntries = gST->NumberOfTableEntries;

    while ( TableEntries > 0 )
    {
        TableEntries--;

        if ((!CompareMem(
                 &gST->ConfigurationTable[TableEntries].VendorGuid,
                 &NameGuid, sizeof(EFI_GUID))))
        {            
            HobStart = gST->ConfigurationTable[TableEntries].VendorTable;
            FirmwareVolumeHob.Raw = GetHob (EFI_HOB_TYPE_FV, HobStart);
            if (FirmwareVolumeHob.Header->HobType != EFI_HOB_TYPE_FV) {
                 continue;
            }
            break;   
        }
    }        
   
    for (Status = EFI_NOT_FOUND; EFI_ERROR (Status);) {
        if (END_OF_HOB_LIST (FirmwareVolumeHob)) {
          return EFI_NOT_FOUND;
        }

        if (GET_HOB_TYPE (FirmwareVolumeHob) == EFI_HOB_TYPE_FV) {
        if ((((UINT64)FirmwareVolumeHob.FirmwareVolume->BaseAddress)\
                < (UINT64)NVRAM_ADDRESS ) || 
                ((UINT64)FirmwareVolumeHob.FirmwareVolume->BaseAddress) == FV_MAIN_BASE)
            {
                Found = TRUE;
                break;
            }
        }
        
        FirmwareVolumeHob.Raw = GET_NEXT_HOB (FirmwareVolumeHob);
    }

    if(Found == FALSE)return EFI_NOT_FOUND;

    gBS->AllocatePool(EfiBootServicesData, (UINTN)FirmwareVolumeHob.FirmwareVolume->Length, &Buffer);

    if(Buffer == NULL) return EFI_OUT_OF_RESOURCES;
  
    if(FirmwareVolumeHob.FirmwareVolume->BaseAddress == FV_MAIN_BASE)
      {
         if(FirmwareVolumeHob.FirmwareVolume->Length > TCG_SIZE){
              Size = TCG_SIZE;
         }else{
             Size = (UINTN)FirmwareVolumeHob.FirmwareVolume->Length;
         }

         gBS->CopyMem(Buffer, (UINT8 *)(EFI_PHYSICAL_ADDRESS)FirmwareVolumeHob.FirmwareVolume->BaseAddress,\
                    Size);

      }else{

        Buffer = (UINT8 *)(EFI_PHYSICAL_ADDRESS)FirmwareVolumeHob.FirmwareVolume->BaseAddress;
        Size = (UINTN)FirmwareVolumeHob.FirmwareVolume->Length;
      }
      
      EventData.PostCodeAddress = \
                    (EFI_PHYSICAL_ADDRESS)FirmwareVolumeHob.FirmwareVolume->BaseAddress;

  #if defined x64_TCG &&  x64_TCG == 1
      EventData.PostCodeLength = Size;
  #else
      EventData.PostCodeLength = Size;
  #endif


      Tcg20Event->Size      = sizeof(TrEE_EVENT_HEADER) + sizeof(UINT32) + sizeof(EventData);
      Tcg20Event->Header.HeaderSize     = sizeof(TrEE_EVENT_HEADER);
      Tcg20Event->Header.HeaderVersion  = 1;
      Tcg20Event->Header.PCRIndex       = 0;
      Tcg20Event->Header.EventType      = EV_POST_CODE;

      gBS->CopyMem(Tcg20Event->Event, &EventData,sizeof(EventData));


      Status = TrEEProtocolInstance->HashLogExtendEvent(TrEEProtocolInstance,
                                           0, EventData.PostCodeAddress, Size,
                                           Tcg20Event);

      return Status;
}


EFI_STATUS
EFIAPI
MeasureHandoffTables (
  VOID
  )
{
    EFI_STATUS                        Status;
    SMBIOS_TABLE_ENTRY_POINT          *SmbiosTable;
    TrEE_EVENT                        *Tpm20Event;

    if(TrEEProtocolInstance == NULL) return EFI_NOT_FOUND;

    Status = gBS->AllocatePool(EfiBootServicesData, (sizeof(TrEE_EVENT_HEADER) + \
                      sizeof(UINT32) + sizeof(EFI_HANDOFF_TABLE_POINTERS)), &Tpm20Event);

    if(EFI_ERROR(Status) || (Tpm20Event == NULL))return Status;

    Status = EfiGetSystemConfigurationTable (&gEfiSmbiosTableGuid,
                            (VOID **) &SmbiosTable);

    if (!EFI_ERROR (Status)) {
    ASSERT (SmbiosTable != NULL);
    }
  
    Tpm20Event->Size  = sizeof(TrEE_EVENT_HEADER) + \
                      sizeof(UINT32) + sizeof(EFI_HANDOFF_TABLE_POINTERS);

    Tpm20Event->Header.HeaderSize = sizeof(TrEE_EVENT_HEADER);
    Tpm20Event->Header.HeaderVersion = 1;
    Tpm20Event->Header.PCRIndex    = 1;
    Tpm20Event->Header.EventType   = EV_EFI_HANDOFF_TABLES;

    ((EFI_HANDOFF_TABLE_POINTERS *)((UINTN)&Tpm20Event->Event[0]))->NumberOfTables = 1;
    ((EFI_HANDOFF_TABLE_POINTERS *)((UINTN)&Tpm20Event->Event[0]))->TableEntry[0].VendorGuid = gEfiSmbiosTableGuid;
    ((EFI_HANDOFF_TABLE_POINTERS *)((UINTN)&Tpm20Event->Event[0]))->TableEntry[0].VendorTable = SmbiosTable;

    Status = TrEEProtocolInstance->HashLogExtendEvent(TrEEProtocolInstance,
                                           0, (EFI_PHYSICAL_ADDRESS)(UINT8*)(UINTN)SmbiosTable->TableAddress, 
                                           SmbiosTable->TableLength,
                                           Tpm20Event);
    
    gBS->FreePool(Tpm20Event);

    return Status;
}



VOID *
EFIAPI
ReadVariable (
  IN    CHAR16      *VarName,
  IN    EFI_GUID    *VendorGuid,
  OUT   UINTN       *VarSize
  )
{
  EFI_STATUS                        Status;
  VOID                              *VarData;

  *VarSize = 0;
  Status = gRT->GetVariable (
                  VarName,
                  VendorGuid,
                  NULL,
                  VarSize,
                  NULL
                  );
  if (Status != EFI_BUFFER_TOO_SMALL) {
    return NULL;
  }

  gBS->AllocatePool (EfiBootServicesData, *VarSize, &VarData);
  if (VarData != NULL) {
    Status = gRT->GetVariable (
                    VarName,
                    VendorGuid,
                    NULL,
                    VarSize,
                    VarData
                    );
    if (EFI_ERROR (Status)) {
      gBS->FreePool (VarData);
      VarData = NULL;
      *VarSize = 0;
    }
  }
  return VarData;
}



EFI_STATUS
EFIAPI
MeasureVariable (
  IN      UINT32                    PCRIndex,
  IN      UINT32                    EventType,
  IN      CHAR16                    *VarName,
  IN      EFI_GUID                  *VendorGuid,
  IN      VOID                      *VarData,
  IN      UINTN                     VarSize
  )
{
    EFI_STATUS            Status;
    TrEE_EVENT            *Tpm20Event;
    UINTN                 EventSize;
    UINTN                 VarNameLength;
    EFI_VARIABLE_DATA    *VarLog;

    VarNameLength = StrLen (VarName);

    if(TrEEProtocolInstance == NULL) return EFI_NOT_FOUND;

    EventSize = (UINT32)(sizeof (*VarLog) + VarNameLength * sizeof (*VarName) + VarSize
                        - sizeof (VarLog->UnicodeName) - sizeof (VarLog->VariableData));

    gBS->AllocatePool(EfiBootServicesData, (sizeof(TrEE_EVENT_HEADER) + \
                      sizeof(UINT32) + EventSize), &Tpm20Event);

    if(Tpm20Event==NULL)return EFI_OUT_OF_RESOURCES;
    
    Tpm20Event->Size  = sizeof(TrEE_EVENT_HEADER) + \
                      sizeof(UINT32) + (UINT32)EventSize;

    Tpm20Event->Header.HeaderSize = sizeof(TrEE_EVENT_HEADER);
    Tpm20Event->Header.HeaderVersion = 1;
    Tpm20Event->Header.PCRIndex    = PCRIndex;
    Tpm20Event->Header.EventType   = EventType;


    ((EFI_VARIABLE_DATA *)((UINTN)&Tpm20Event->Event[0]))->VariableName       = *VendorGuid;
    ((EFI_VARIABLE_DATA *)((UINTN)&Tpm20Event->Event[0]))->UnicodeNameLength  = VarNameLength;
    ((EFI_VARIABLE_DATA *)((UINTN)&Tpm20Event->Event[0]))->VariableDataLength = VarSize;

    gBS->CopyMem (((EFI_VARIABLE_DATA *)((UINTN)&Tpm20Event->Event[0]))->UnicodeName,
                 VarName,
                 VarNameLength * sizeof (*VarName));

    gBS->CopyMem ((CHAR16 *)((EFI_VARIABLE_DATA *)((UINTN)&Tpm20Event->Event[0]))->UnicodeName + VarNameLength,
                 VarData,
                 VarSize);
    
    Status = TrEEProtocolInstance->HashLogExtendEvent(TrEEProtocolInstance,
                                           0, (EFI_PHYSICAL_ADDRESS)(UINT8 *)(&Tpm20Event->Event[0]), EventSize,
                                           Tpm20Event);

    gBS->FreePool(Tpm20Event);

    return Status;
}


EFI_STATUS
EFIAPI
TcgMeasureGptTable (
  IN  EFI_HANDLE         GptHandle
  )
{
  EFI_STATUS                        Status;
  EFI_BLOCK_IO_PROTOCOL             *BlockIo;
  EFI_DISK_IO_PROTOCOL              *DiskIo;
  EFI_PARTITION_TABLE_HEADER        *PrimaryHeader;                     
  EFI_PARTITION_ENTRY               *PartitionEntry;
  UINT8                             *EntryPtr;
  UINTN                             NumberOfPartition;
  UINT32                            Index;
  UINT64							Flags;
  EFI_GPT_DATA                      *GptData;
  UINT32                            EventSize;
  MASTER_BOOT_RECORD                *Mbr;
  UINT8                             Count;
  UINT32                            LBAofGptHeader = 0;
  TCG_PCR_EVENT2_HDR                 TcgEvent;
  AMI_INTERNAL_HLXE_PROTOCOL        *InternalHLXE = NULL;
  EFI_GUID                          gEfiAmiHLXEGuid =  AMI_PROTOCOL_INTERNAL_HLXE_GUID;

  if (mMeasureGptCount > 0) {
    return EFI_SUCCESS;
  }

  Status = gBS->HandleProtocol (GptHandle, &gEfiBlockIoProtocolGuid, (VOID**)&BlockIo);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }
  Status = gBS->HandleProtocol (GptHandle, &gEfiDiskIoProtocolGuid, (VOID**)&DiskIo);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //Read the protective MBR 
  gBS->AllocatePool (EfiBootServicesData, BlockIo->Media->BlockSize, &Mbr);
  if (Mbr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }  
 
  Status = DiskIo->ReadDisk (
                     DiskIo,
                     BlockIo->Media->MediaId,
                     0 * BlockIo->Media->BlockSize,
                     BlockIo->Media->BlockSize,
                     (UINT8 *)Mbr
                     );

  for(Count=0; Count<MAX_MBR_PARTITIONS;Count++){
    if(Mbr->Partition[Count].OSIndicator == 0xEE){//(i.e., GPT Protective)
		  LBAofGptHeader = *(Mbr->Partition[Count].StartingLBA);
		  break;
	  }
  }

  if(LBAofGptHeader == 0x00)//Did not find the correct GPTHeader so return EFI_NOT_FOUND
    return EFI_NOT_FOUND;

  //
  // Read the EFI Partition Table Header
  //  
  gBS->AllocatePool (EfiBootServicesData, BlockIo->Media->BlockSize, &PrimaryHeader);
  if (PrimaryHeader == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }  
 
  Status = DiskIo->ReadDisk (
                     DiskIo,
                     BlockIo->Media->MediaId,
                     LBAofGptHeader * BlockIo->Media->BlockSize,
                     BlockIo->Media->BlockSize,
                     (UINT8 *)PrimaryHeader);

//  if(PrimaryHeader->Header.Signature != EFI_GPT_HEADER_ID)//Check for "EFI PART" signature
  if (CompareMem(EFI_GPT_HEADER_ID, &PrimaryHeader->Header.Signature, sizeof(UINT64))) return EFI_NOT_FOUND;

  if (EFI_ERROR (Status)) {
    DEBUG ((-1, "Failed to Read Partition Table Header!\n"));
    gBS->FreePool (PrimaryHeader);                                                    
    return EFI_DEVICE_ERROR;
  }  

  //
  // Read the partition entry.
  //
  gBS->AllocatePool (EfiBootServicesData, PrimaryHeader->NumberOfPartitionEntries * PrimaryHeader->SizeOfPartitionEntry, &EntryPtr);
  if (EntryPtr == NULL) {
    gBS->FreePool (PrimaryHeader);
    return EFI_OUT_OF_RESOURCES;
  }
  Status = DiskIo->ReadDisk (
                     DiskIo,
                     BlockIo->Media->MediaId,
                     PrimaryHeader->PartitionEntryLBA * BlockIo->Media->BlockSize,
                     PrimaryHeader->NumberOfPartitionEntries * PrimaryHeader->SizeOfPartitionEntry,
                     EntryPtr
                     );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (PrimaryHeader);
    gBS->FreePool (EntryPtr);
    return EFI_DEVICE_ERROR;
  }
  
  //
  // Count the valid partition
  //
  PartitionEntry    = (EFI_PARTITION_ENTRY *)EntryPtr;
  NumberOfPartition = 0;
  for (Index = 0; Index < PrimaryHeader->NumberOfPartitionEntries; Index++) {
    if (CompareMem (&PartitionEntry->PartitionTypeGUID, &ZeroGuid, sizeof(EFI_GUID))) {
      NumberOfPartition++;  
    }
    PartitionEntry++;
  }

  //
  // Parepare Data for Measurement
  // 
  EventSize = (UINT32)(sizeof (EFI_GPT_DATA) - sizeof (GptData->Partitions) 
                        + NumberOfPartition * PrimaryHeader->SizeOfPartitionEntry);
  
  gBS->AllocatePool (EfiBootServicesData, EventSize, &GptData);
  if (GptData == NULL) {
    gBS->FreePool (PrimaryHeader);
    gBS->FreePool (EntryPtr);
    return EFI_OUT_OF_RESOURCES;
  }

  gBS->SetMem(GptData, EventSize, 0);

  TcgEvent.PCRIndex   = 5;
  TcgEvent.EventType  = EV_EFI_GPT_EVENT;
  TcgEvent.EventSize  = EventSize;

  Flags = 0;

  //
  // Copy the EFI_PARTITION_TABLE_HEADER and NumberOfPartition
  //  
  gBS->CopyMem ((UINT8 *)GptData, (UINT8*)PrimaryHeader, sizeof (EFI_PARTITION_TABLE_HEADER));
  GptData->NumberOfPartitions = NumberOfPartition;
  //
  // Copy the valid partition entry
  //
  PartitionEntry    = (EFI_PARTITION_ENTRY*)EntryPtr;
  NumberOfPartition = 0;
  for (Index = 0; Index < PrimaryHeader->NumberOfPartitionEntries; Index++) {
    if (CompareMem (&PartitionEntry->PartitionTypeGUID, &ZeroGuid, sizeof(EFI_GUID))) {
      gBS->CopyMem (
        (UINT8 *)&GptData->Partitions + NumberOfPartition * sizeof (EFI_PARTITION_ENTRY),
        (UINT8 *)PartitionEntry,
        sizeof (EFI_PARTITION_ENTRY)
        );
      NumberOfPartition++;
    }
    PartitionEntry++;
  }

  //
  // Measure the GPT data
  //
  if(NumberOfPartition > 0)
  {

    Status = gBS->LocateProtocol(&gEfiAmiHLXEGuid, NULL, &InternalHLXE);
    if(EFI_ERROR(Status))return Status;

    InternalHLXE->AmiHashLogExtend2(TrEEProtocolInstance, (UINT8 *)GptData, 0, EventSize, &TcgEvent, (UINT8 *)GptData);

    if (!EFI_ERROR (Status)) {
      mMeasureGptCount++;
	  DEBUG ((-1, "\n GPT measurement successfull !!!\n"));
    }
  }

  gBS->FreePool (PrimaryHeader);
  gBS->FreePool (EntryPtr);
  gBS->FreePool (GptData);
  return Status;
}





EFI_STATUS
EFIAPI
MeasureGptTable ()
{
  EFI_STATUS                  Status;
  EFI_HANDLE                  Handle;
  EFI_HANDLE                  *HandleArray;
  UINTN                       HandleArrayCount;
  UINTN                       Index;  
  EFI_DEVICE_PATH_PROTOCOL   *BlockIoDevicePath;
  EFI_DEVICE_PATH_PROTOCOL   *DevicePath;
  static  BOOLEAN             mMeasureGptTableFlag = FALSE;

  DEBUG ((-1, "MeasureGptTable\n"));


  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiBlockIoProtocolGuid, NULL, &HandleArrayCount, &HandleArray);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  for (Index=0; Index < HandleArrayCount; Index++) {
    Status = gBS->HandleProtocol (HandleArray[Index], &gEfiDevicePathProtocolGuid, (VOID *) &BlockIoDevicePath);
    if (EFI_ERROR (Status) || BlockIoDevicePath == NULL) {
      continue;
    }
    for (DevicePath = BlockIoDevicePath; !IsDevicePathEnd (DevicePath); DevicePath = NextDevicePathNode (DevicePath)) {
      if ((DevicePathType (DevicePath) == ACPI_DEVICE_PATH) && (DevicePathSubType (DevicePath) == ACPI_DP)) {
        Status = gBS->LocateDevicePath (&gEfiBlockIoProtocolGuid, &DevicePath, &Handle);
        if (!EFI_ERROR (Status)) {
          Status = TcgMeasureGptTable (Handle);
          if (!EFI_ERROR (Status)) {
            //
            // GPT partition check done.
            //
            mMeasureGptTableFlag = TRUE;
          }
			  }
        break;
      }
	  }
  }
  
  return Status;
}


EFI_STATUS
MeasureSecureBootState(
  VOID
  )
{
  EFI_STATUS          Status;
  UINT32              Attribute;
  UINTN               DataSize;
  UINT8               *Variable;
  UINT64              MaxStorSize;
  UINT64              RemStorSize;
  UINT64              MaxVarSize;
  TCG_PCR_EVENT_HDR   TcgEvent;
  CHAR16              *VarName;
  EFI_GUID            VendorGuid;

  Attribute = EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS;
	
  TcgEvent.PCRIndex      = 7;
  TcgEvent.EventType     =  EV_EFI_VARIABLE_DRIVER_CONFIG;

  // Query maximum size of the variable and allocate memory

  Status = gRT->QueryVariableInfo(Attribute, &MaxStorSize, &RemStorSize, &MaxVarSize);
  if (EFI_ERROR(Status)) {
    return (Status);
  }
  
  DataSize = (UINTN)MaxStorSize;
  gBS->AllocatePool(EfiBootServicesData, DataSize, &Variable);
  if (Variable == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  gBS->SetMem(Variable, DataSize, 0);  // Clear the buffer

  // 1.Measure Secure Boot Variable Value

  Status = gRT->GetVariable (
                  EFI_SECURE_BOOT_NAME,
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &DataSize, 
                  Variable
                  ); 

  VarName = EFI_SECURE_BOOT_NAME;
  VendorGuid = gEfiGlobalVariableGuid;

  if(EFI_ERROR(Status) || *Variable == 0){
		DataSize = 0;
        *Variable = 0;
  }


  Status = MeasureVariable (
             7,
             EV_EFI_VARIABLE_DRIVER_CONFIG,
             VarName,
             &VendorGuid,
             Variable,
             DataSize
             );

	// 2.Measure PK Variable Value

	DataSize = (UINTN)MaxStorSize; // DataSize gets updated by GetVariable. So initialize everytime before the call
	gBS->SetMem(Variable, DataSize, 0);  // Clear the buffer

    Status = gRT->GetVariable (
                  EFI_PLATFORM_KEY_NAME,
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &DataSize, 
                  Variable
                  ); 

	VarName = EFI_PLATFORM_KEY_NAME;
    VendorGuid = gEfiGlobalVariableGuid;

    if(EFI_ERROR(Status)){
      DataSize = 0;
      *Variable = 0;
    }

    Status = MeasureVariable (
             7,
             EV_EFI_VARIABLE_DRIVER_CONFIG,
             VarName,
             &VendorGuid,
             Variable,
             DataSize
             );

  // 3.Measure KEK Variable Value

  DataSize = (UINTN)MaxStorSize; // DataSize gets updated by GetVariable. So initialize everytime before the call
  gBS->SetMem(Variable, DataSize, 0);  // Clear the buffer

  Status = gRT->GetVariable (
                          EFI_KEY_EXCHANGE_KEY_NAME,
                          &gEfiGlobalVariableGuid,
                          NULL,
                          &DataSize, 
                          Variable
                          ); 

	VarName = EFI_KEY_EXCHANGE_KEY_NAME;
    VendorGuid = gEfiGlobalVariableGuid;

    if(EFI_ERROR(Status)){
      DataSize = 0;
      *Variable = 0;
    }

    Status = MeasureVariable (
             7,
             EV_EFI_VARIABLE_DRIVER_CONFIG,
             VarName,
             &VendorGuid,
             Variable,
             DataSize
             );

	if(EFI_ERROR(Status)){
		goto Exit;
	}

	// 4.Measure EFI_IMAGE_SECURITY_DATABASE Variable Value

	DataSize = (UINTN)MaxStorSize; // DataSize gets updated by GetVariable. So initialize everytime before the call
	gBS->SetMem(Variable, DataSize, 0);  // Clear the buffer

  Status = gRT->GetVariable (
                          EFI_IMAGE_SECURITY_DATABASE,
                          &gEfiImageSecurityDatabaseGuid,
                          NULL,
                          &DataSize, 
                          Variable
                          ); 

	VarName = EFI_IMAGE_SECURITY_DATABASE;
    VendorGuid = gEfiImageSecurityDatabaseGuid;

    if(EFI_ERROR(Status)){
      DataSize = 0;
      *Variable = 0;
    }

    Status = MeasureVariable (
             7,
             EV_EFI_VARIABLE_DRIVER_CONFIG,
             VarName,
             &VendorGuid,
             Variable,
             DataSize
             );

	if(EFI_ERROR(Status)){
		goto Exit;
	}

	// 5.Measure EFI_IMAGE_SECURITY_DATABASE1 Variable Value

	DataSize = (UINTN)MaxStorSize; // DataSize gets updated by GetVariable. So initialize everytime before the call
	gBS->SetMem(Variable, DataSize, 0);  // Clear the buffer

  Status = gRT->GetVariable (
                          EFI_IMAGE_SECURITY_DATABASE1,
                          &gEfiImageSecurityDatabaseGuid,
                          NULL,
                          &DataSize, 
                          Variable
                          ); 

	VarName = EFI_IMAGE_SECURITY_DATABASE1;
    VendorGuid = gEfiImageSecurityDatabaseGuid;

    if(EFI_ERROR(Status)){
      DataSize = 0;
      *Variable = 0;
    }

    Status = MeasureVariable (
             7,
             EV_EFI_VARIABLE_DRIVER_CONFIG,
             VarName,
             &VendorGuid,
             Variable,
             DataSize
             );

	if(EFI_ERROR(Status)){
		goto Exit;
	}

Exit:
	gBS->FreePool(Variable);
	return EFI_SUCCESS;
}


EFI_STATUS ResetMorVariable()
{
    EFI_STATUS      Status;
    EFI_GUID MorGuid = MEMORY_ONLY_RESET_CONTROL_GUID;
    UINT32 Attribs   = EFI_VARIABLE_NON_VOLATILE
                       | EFI_VARIABLE_BOOTSERVICE_ACCESS
                       | EFI_VARIABLE_RUNTIME_ACCESS;

    UINT8 Temp       = 0;
    UINTN TempSize = sizeof (UINT8);



    Status = gRT->GetVariable(
        L"MemoryOverwriteRequestControl",
        &MorGuid,
        &Attribs,
        &TempSize,
        &Temp );

    if ( EFI_ERROR( Status ) || Temp != 0 )
    {
        Temp = 0;
        Status = gRT->SetVariable(
            L"MemoryOverwriteRequestControl",
            &MorGuid,
            EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS
            | EFI_VARIABLE_RUNTIME_ACCESS,
            sizeof (UINT8),
            &Temp );
    }

    return Status;
}

EFI_STATUS
InternalMeasureAction (
  IN      CHAR8                     *ActionString
  )
{
   return EFI_SUCCESS; //not supported
}


EFI_STATUS
InternalMeasureGpt (
  IN      EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{

    return EFI_SUCCESS; //not supported;
}



EFI_TCG_PLATFORM_PROTOCOL  mTcgPlatformProtocol = {
  TreeMeasurePeImage,
  InternalMeasureAction,
  InternalMeasureGpt
};


EFI_STATUS InstallTcgPlatformProtocol(
    VOID    
)
{

 EFI_GUID  gEfiTcgPrivateInterfaceGuid = EFI_TCG_PLATFORM_PROTOCOL_GUID;

 return gBS->InstallProtocolInterface (
                &PlatformProtocolHandle,
                &gEfiTcgPrivateInterfaceGuid,
                EFI_NATIVE_INTERFACE,
                &mTcgPlatformProtocol
                );

}


VOID
EFIAPI
Tpm20OnReadyToBoot (
  IN      EFI_EVENT                 Event,
  IN      VOID                      *Context
  )
{
  EFI_STATUS                Status;
  UINT32                    PcrIndex;
  static  BOOLEAN           mBootAttempts=0;
  TPM2B_AUTH                        NewAuth;
 
  if (mBootAttempts == 0) {

    ResetMorVariable();
    //
    // Measure handoff tables
    //
    /*Status = MeasureHandoffTables ();
    if (EFI_ERROR (Status)) {
      DEBUG ((-1, "HandoffTables not measured.\n"));
    }
    else {
      DEBUG((-1, "HandoffTables measured.\n"));
    }*/

    //
	// Measure the fact that Secure Boot is disabled
    //
    Status = MeasureSecureBootState();
    if (EFI_ERROR (Status)) {
      DEBUG ((-1, "Measuring secure boot state failed.\n"));
    }
    else {
      DEBUG((-1, "Secure boot state measured.\n"));
    }

	
    //
    // This is the first boot attempt
    //
    Status = MeasureAction (
               "EFI_CALLING_EFI_APPLICATION"
             );
    if (EFI_ERROR (Status)) {
      DEBUG ((-1, "First boot attempt not Measured.\n"));
    }
    else {
      DEBUG((-1, "First boot attempt measured.\n"));
    }

    //
    // Draw a line between pre-boot env and entering post-boot env
    //
    for (PcrIndex = 0; PcrIndex < 8; PcrIndex++) {
      Status = MeasureSeparatorEvent (PcrIndex);
      if (EFI_ERROR (Status)) {
        DEBUG ((-1, "Measuring separtator event failed.\n"));
      }
      else {
        DEBUG((-1, "Separator event measured.\n"));
      }

    }

    //
    // Measure GPT
    //
    Status = MeasureGptTable ();
    if (EFI_ERROR (Status)) {
      DEBUG ((-1, "Measuring GPT failed.\n"));
    }
    else {
      DEBUG((-1, "GPT measured.\n"));
    }
    
    NewAuth.size = SHA1_DIGEST_SIZE;
        
     Tpm2GetRandom(
         NewAuth.size,
         &NewAuth.buffer[0]
     );
     
     Status = Tpm2HierarchyChangeAuth (TPM_RH_PLATFORM, NULL, &NewAuth);
    
  } 
  else {
    //
    // Not first attempt, meaning a return from last attempt
    //
/*
    Status = MeasureAction (
               "EFI_RETURNING_FROM_EFI_APPLICATOIN"
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((-1, "Measuring additional boot attempt failed.\n"));
    }
*/
  }
  //
  // Increase boot attempt counter.
  //
  mBootAttempts++;

}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:   GetStringFromToken
//
// Description: Gets a UNI string by Token
//
// Input:       IN      STRING_REF                Token,
//              OUT     CHAR16                    **String
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS GetStringFromToken(
    IN STRING_REF Token,
    OUT CHAR16    **String )
{
    EFI_STATUS Status;
    UINTN      StringBufferLength;
    UINT16     *Temp;
    UINTN      Size = 0;


    //
    // Find the string based on the current language
    //
    StringBufferLength = 0x500;
    Status             = gBS->AllocatePool( EfiBootServicesData,
                                            sizeof (CHAR16) * 0x500,
                                            String );
    Temp               = *String;
    while ( Temp < *String + StringBufferLength )
    {
        *Temp = 0x0;
        Temp++;
    }

#if EFI_SPECIFICATION_VERSION>0x20000 

    *String = HiiGetString (
        gHiiHandle,
        Token,
        NULL
    );
    if (EFI_ERROR(Status)) {
        return Status;
    }

#else
    if ( Hii == NULL )
    {
        return EFI_NOT_FOUND;
    }

    Status = Hii->GetString(
        Hii,
        gHiiHandle,
        Token,
        TRUE,
        NULL,
        &StringBufferLength,
        *String
        );
#endif


    if ( EFI_ERROR( Status ))
    {
        gBS->FreePool( *String );
        return EFI_NOT_FOUND;
    }

    return EFI_SUCCESS;
}


//****************************************************************************************
//<AMI_PHDR_START>
//
// Procedure: write_PPI_result
//
// Description: Updates TCG PPI variable in NVRAM
//
//
// Input:       IN  UINT8 last_op,
//              IN  UINT16 status
//
// Output:      VOID
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//****************************************************************************************
void WritePpiResult(
    IN UINT8  last_op,
    IN UINT16 status )
{
    UINTN          Size = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR Temp;
    EFI_STATUS     Status;
    UINT8          Manip = 0;

    Status = gRT->GetVariable( L"AMITCGPPIVAR", \
                               &AmitcgefiOsVariableGuid, \
                               NULL, \
                               &Size, \
                               &Temp );

    //now set variable to data
    Temp.RQST  = Manip;
    Manip      = (UINT8)( status & 0xFFFF );
    Temp.ERROR = Manip;

    if(status>0xFF && status<0xFFFF)
    {
        Temp.AmiMisc = (UINT8)(status >> 8);
    }else{
        Temp.AmiMisc = 0;
    }

    if ( EFI_ERROR( Status ))
    {
        DEBUG((-1, "Error Setting Return value\n"));
        return;
    }

    Status = gRT->SetVariable( L"AMITCGPPIVAR", \
                               &AmitcgefiOsVariableGuid, \
                               EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               Size, \
                               &Temp );
}



/**
  Send ClearControl command to TPM2.

  @param Disable           if we need disable owner clear flag.

  @retval EFI_SUCCESS      Operation completed successfully.
  @retval EFI_DEVICE_ERROR Unexpected device behavior.
**/
EFI_STATUS
EFIAPI
Tpm2ClearControl (
  IN TPMI_YES_NO          Disable
  )
{
  EFI_STATUS                        Status;
  TPM2_CLEAR_CONTROL_COMMAND        Cmd;
  TPM2_CLEAR_CONTROL_RESPONSE       Res;
  UINT32                            ResultBufSize;
  UINT32                            CmdSize;
  UINT32                            RespSize;
  UINT8                             *Buffer;
  UINT8                             *AuthSizeOffset;

  Cmd.Header.tag         = (TPMI_ST_COMMAND_TAG)TPM_H2NS(TPM_ST_SESSIONS);
  Cmd.Header.commandCode = TPM_H2NL(TPM_CC_ClearControl);
  Cmd.Auth               = TPM_H2NL(TPM_RH_PLATFORM);

  Buffer = (UINT8 *)&Cmd.AuthorizationSize;

  //
  // Add in Auth session
  //
  AuthSizeOffset = Buffer;
  *(UINT32 *)Buffer = 0;
  Buffer += sizeof(UINT32);

  // authHandle
  *(UINT32 *)Buffer = TPM_H2NL(TPM_RS_PW);
  Buffer += sizeof(UINT32);

  // nonce = nullNonce
  *(UINT16 *)Buffer = 0;
  Buffer += sizeof(UINT16);

  // sessionAttributes = 0
  *(UINT8 *)Buffer = 0;
  Buffer += sizeof(UINT8);

  // auth = nullAuth
  *(UINT16 *)Buffer = 0;
  Buffer += sizeof(UINT16);

  // authorizationSize
  *(UINT32 *)AuthSizeOffset = TPM_H2NL((UINT32)(Buffer - AuthSizeOffset - sizeof(UINT32)));

  // disable
  *(UINT8 *)Buffer = Disable;
  Buffer += sizeof(UINT8);

  CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
  Cmd.Header.paramSize   = TPM_H2NL(CmdSize);

  ResultBufSize = sizeof(Res);
  Status = TrEEProtocolInstance->SubmitCommand(TrEEProtocolInstance,CmdSize,(UINT8 *)&Cmd , ResultBufSize , (UINT8 *)&Res);

  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (ResultBufSize > sizeof(Res)) {
    DEBUG((-1, "ClearControl: Failed ExecuteCommand: Buffer Too Small\r\n"));
    Status = EFI_BUFFER_TOO_SMALL;
    goto ClearControlEND;
  }

  //
  // Validate response headers
  //
  RespSize = TPM_H2NL(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG((-1, "ClearControl: Response size too large! %d\r\n", RespSize));
    Status = EFI_BUFFER_TOO_SMALL;
    goto ClearControlEND;
  }

  //
  // Fail if command failed
  //
  if (TPM_H2NL(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG((-1, "ClearControl: Response Code error! 0x%08x\r\n", TPM_H2NL(Res.Header.responseCode)));
    Status = EFI_DEVICE_ERROR;
    goto ClearControlEND;
  }

  //
  // Unmarshal the response
  //

  // None

  Status = EFI_SUCCESS;
 
ClearControlEND:
  return Status;
}



EFI_STATUS
EFIAPI
Tpm2Clear (
  VOID
  )
{
  EFI_STATUS                        Status;
  TPM2_CLEAR_COMMAND                Cmd;
  TPM2_CLEAR_RESPONSE               Res;
  UINT32                            ResultBufSize;
  UINT32                            CmdSize;
  UINT32                            RespSize;
  UINT8                             *Buffer;
  UINT8                             *AuthSizeOffset;

  Cmd.Header.tag         = (TPMI_ST_COMMAND_TAG)TPM_H2NS(TPM_ST_SESSIONS);
  Cmd.Header.commandCode = TPM_H2NL(TPM_CC_Clear);
  Cmd.Auth               = TPM_H2NL(TPM_RH_PLATFORM);

  Buffer = (UINT8 *)&Cmd.AuthorizationSize;

  //
  // Add in Auth session
  //
  AuthSizeOffset = Buffer;
  *(UINT32 *)Buffer = 0;
  Buffer += sizeof(UINT32);

  // authHandle
  *(UINT32 *)Buffer = TPM_H2NL(TPM_RS_PW);
  Buffer += sizeof(UINT32);

  // nonce = nullNonce
  *(UINT16 *)Buffer = 0;
  Buffer += sizeof(UINT16);

  // sessionAttributes = 0
  *(UINT8 *)Buffer = 0;
  Buffer += sizeof(UINT8);

  // auth = nullAuth
  *(UINT16 *)Buffer = 0;
  Buffer += sizeof(UINT16);

  // authorizationSize
  *(UINT32 *)AuthSizeOffset = TPM_H2NL((UINT32)(Buffer - AuthSizeOffset - sizeof(UINT32)));

  CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
  Cmd.Header.paramSize   = TPM_H2NL(CmdSize);

  ResultBufSize = sizeof(Res);
  
  Status = TrEEProtocolInstance->SubmitCommand(TrEEProtocolInstance,CmdSize,(UINT8 *)&Cmd , ResultBufSize , (UINT8 *)&Res);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  if (ResultBufSize > sizeof(Res)) {
    DEBUG((-1, "Clear: Failed ExecuteCommand: Buffer Too Small\r\n"));
    Status = EFI_BUFFER_TOO_SMALL;
    goto ClearEND;
  }

  //
  // Validate response headers
  //
  RespSize = TPM_H2NL(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG((-1, "Clear: Response size too large! %d\r\n", RespSize));
    Status = EFI_BUFFER_TOO_SMALL;
    goto ClearEND;
  }

  //
  // Fail if command failed
  //
  if (TPM_H2NL(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG((-1, "Clear: Response Code error! 0x%08x\r\n", TPM_H2NL(Res.Header.responseCode)));
    Status = EFI_DEVICE_ERROR;
    goto ClearEND;
  }

  //
  // Unmarshal the response
  //

  // None

  Status = EFI_SUCCESS;
 
ClearEND:
  return Status;
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  CopyAuthSessionCommand
//
// Description:
//
//
// Input:       UINT8  AuthSessionIn
//
// Output:      TPMS_AUTH_RESPONSE* AuthSessionOut
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
UINT32
EFIAPI
CopyAuthSessionCommand (
  IN      TPMS_AUTH_SESSION_COMMAND         *AuthSessionIn, OPTIONAL
  OUT     UINT8                             *AuthSessionOut
  )
{
  UINT8  *Buffer;

  Buffer = (UINT8 *)AuthSessionOut;
  
  //
  // Add in Auth session
  //
  if (AuthSessionIn != NULL) {
    //  sessionHandle
    WriteUnaligned32 ((UINT32 *)Buffer, SwapBytes32(AuthSessionIn->sessionHandle));
    Buffer += sizeof(UINT32);

    // nonce
    WriteUnaligned16 ((UINT16 *)Buffer, SwapBytes16 (AuthSessionIn->nonce.size));
    Buffer += sizeof(UINT16);

    CopyMem (Buffer, AuthSessionIn->nonce.buffer, AuthSessionIn->nonce.size);
    Buffer += AuthSessionIn->nonce.size;

    // sessionAttributes
    *(UINT8 *)Buffer = *(UINT8 *)&AuthSessionIn->sessionAttributes;
    Buffer += sizeof(UINT8);

    // hmac
    WriteUnaligned16 ((UINT16 *)Buffer, SwapBytes16 (AuthSessionIn->auth.size));
    Buffer += sizeof(UINT16);

    CopyMem (Buffer, AuthSessionIn->auth.buffer, AuthSessionIn->auth.size);
    Buffer += AuthSessionIn->auth.size;
  } else {
    //  sessionHandle
    WriteUnaligned32 ((UINT32 *)Buffer, SwapBytes32(TPM_RS_PW));
    Buffer += sizeof(UINT32);

    // nonce = nullNonce
    WriteUnaligned16 ((UINT16 *)Buffer, SwapBytes16(0));
    Buffer += sizeof(UINT16);

    // sessionAttributes = 0
    *(UINT8 *)Buffer = 0x00;
    Buffer += sizeof(UINT8);

    // hmac = nullAuth
    WriteUnaligned16 ((UINT16 *)Buffer, SwapBytes16(0));
    Buffer += sizeof(UINT16);
  }

  return (UINT32)(UINTN)(Buffer - (UINT8 *)AuthSessionOut);
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  CopyAuthSessionResponse
//
// Description: 
//
//
// Input:       UINT8  AuthSessionIn
//
// Output:      TPMS_AUTH_RESPONSE* AuthSessionOut
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
UINT32
EFIAPI
CopyAuthSessionResponse (
  IN      UINT8                              *AuthSessionIn,
  OUT     TPMS_AUTH_SESSION_RESPONSE         *AuthSessionOut OPTIONAL
  )
{
  UINT8                             *Buffer;
  TPMS_AUTH_SESSION_RESPONSE         LocalAuthSessionOut;

  if (AuthSessionOut == NULL) {
    AuthSessionOut = &LocalAuthSessionOut;
  }

  Buffer = (UINT8 *)AuthSessionIn;

  // nonce
  AuthSessionOut->nonce.size = SwapBytes16 (ReadUnaligned16 ((UINT16 *)Buffer));
  Buffer += sizeof(UINT16);

  CopyMem (AuthSessionOut->nonce.buffer, Buffer, AuthSessionOut->nonce.size);
  Buffer += AuthSessionOut->nonce.size;

  // sessionAttributes
  *(UINT8 *)&AuthSessionOut->sessionAttributes = *(UINT8 *)Buffer;
  Buffer += sizeof(UINT8);

  // hmac
  AuthSessionOut->auth.size = SwapBytes16 (ReadUnaligned16 ((UINT16 *)Buffer));
  Buffer += sizeof(UINT16);

  CopyMem (AuthSessionOut->auth.buffer, Buffer, AuthSessionOut->auth.size);
  Buffer += AuthSessionOut->auth.size;

  return (UINT32)(UINTN)(Buffer - (UINT8 *)AuthSessionIn);
}



//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  Tpm2GetRandom
//
// Description: gets random bytes from the TPM 
//
//
// Input:       AuthSize
//
// Output:      UINT8* pOutBuf
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS Tpm2GetRandom(
    UINTN                   AuthSize,
    UINT8*                  pOutBuf
)
{
    EFI_STATUS              Status = EFI_SUCCESS;

    TPM2_GetRandom_COMMAND  GetRandom_cmd;
    TPM2_GetRandom_RESPONSE GetRandom_ret;
    UINT32                  RetSize;

    if(TrEEProtocolInstance == NULL) return EFI_NOT_FOUND; 
    
    // Check the Request Size
    if( AuthSize > sizeof(TPM2B_DIGEST) - sizeof(UINT16) )
    {
        DEBUG(( -1, "Tpm2GetRandom Error. Request too large\n"));
        return EFI_BUFFER_TOO_SMALL;
    }

    SetMem( &GetRandom_cmd, sizeof(GetRandom_cmd), 0 );
    SetMem( &GetRandom_ret, sizeof(GetRandom_ret), 0 );

    GetRandom_cmd.Header.tag =  (UINT16)TPM_H2NS(TPM_ST_NO_SESSIONS);
    GetRandom_cmd.Header.commandCode = TPM_H2NL(TPM_CC_GetRandom);
    GetRandom_cmd.Header.paramSize = TPM_H2NL( sizeof(GetRandom_cmd) );
    GetRandom_cmd.bytesRequested = TPM_H2NS(AuthSize );

    RetSize = sizeof(GetRandom_ret);
    
    Status = TrEEProtocolInstance->SubmitCommand(TrEEProtocolInstance, sizeof(GetRandom_cmd), (UINT8*)&GetRandom_cmd,  RetSize, (UINT8*)&GetRandom_ret );
    if (EFI_ERROR (Status)) {
        DEBUG(( -1, "Tpm2GetRandom TrEEProtocolInstance->SubmitCommand = %r \n", Status));
        return Status;
    }

    if (TPM_H2NL(GetRandom_ret.Header.responseCode) != TPM_RC_SUCCESS) {
        DEBUG(( -1, "Tpm2GetRandom TrEEProtocolInstance->SubmitCommand Response Code error! = %x \n", TPM_H2NL(GetRandom_ret.Header.responseCode)));
        return EFI_DEVICE_ERROR;
    }

    CopyMem( pOutBuf, &GetRandom_ret.randomBytes.buffer[0], AuthSize );

    return Status;
}







//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  Tpm2HierarchyChangeAuth
//
// Description: allows the authorization secret for a hierarchy or lockout to be changed using the current
//              authorization value
//
//
// Input:       AuthHandle, AuthSession, NewAuth
//
// Output:      EFI_STATUS
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EFIAPI
Tpm2HierarchyChangeAuth (
  IN TPMI_RH_HIERARCHY_AUTH     AuthHandle,
  IN TPMS_AUTH_SESSION_COMMAND *AuthSession,
  IN TPM2B_AUTH                *NewAuth
  )
{
  EFI_STATUS                           Status;
  TPM2_HIERARCHY_CHANGE_AUTH_COMMAND   Cmd;
  TPM2_HIERARCHY_CHANGE_AUTH_RESPONSE  Res;
  UINT32                               CmdSize;
  UINT32                               RespSize;
  UINT8                                *Buffer;
  UINT32                               SessionInfoSize;
  UINT8                                *ResultBuf;
  UINT32                               ResultBufSize;

  //
  // Construct command
  //
  Cmd.Header.tag          = (UINT16)TPM_H2NS(TPM_ST_SESSIONS);
  Cmd.Header.paramSize    = TPM_H2NL(sizeof(Cmd));
  Cmd.Header.commandCode  = TPM_H2NL(TPM_CC_HierarchyChangeAuth);
  Cmd.AuthHandle          = TPM_H2NL(AuthHandle);

  //
  // Add in Auth session
  //
  Buffer = (UINT8 *)&Cmd.AuthSession;

  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (AuthSession, Buffer);
  Buffer += SessionInfoSize;
  Cmd.AuthorizationSize = TPM_H2NL(SessionInfoSize);

  // New Authorization size
  WriteUnaligned16 ((UINT16 *)Buffer, SwapBytes16(NewAuth->size));
  Buffer += sizeof(UINT16);

  // New Authorizeation
  CopyMem(Buffer, NewAuth->buffer, NewAuth->size);
  Buffer += NewAuth->size;

  CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
  Cmd.Header.paramSize = TPM_H2NL(CmdSize);

  ResultBuf     = (UINT8 *) &Res;
  ResultBufSize = sizeof(Res);

  Status = TrEEProtocolInstance->SubmitCommand(TrEEProtocolInstance, CmdSize, (UINT8*)&Cmd,  ResultBufSize, ResultBuf );

  if (ResultBufSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "HierarchyChangeAuth: Failed ExecuteCommand: Buffer Too Small\r\n"));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Validate response headers
  //
  RespSize = TPM_H2NL(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "HierarchyChangeAuth: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (TPM_H2NL(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG((EFI_D_ERROR,"HierarchyChangeAuth: Response Code error! 0x%08x\r\n", TPM_H2NL(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}




//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  Tpm2HierarchyControl
//
// Description: enables and disables use of a hierarchy
//
//
// Input:       AuthSize
//
// Output:      UINT8* pOutBuf
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EFIAPI
Tpm2HierarchyControl (
  IN TPMI_RH_HIERARCHY              AuthHandle,
  IN TPMS_AUTH_SESSION_COMMAND      *AuthSession,
  IN TPMI_RH_HIERARCHY              Hierarchy,
  IN TPMI_YES_NO                    State
  )
{
  EFI_STATUS                       Status;
  TPM2_HIERARCHY_CONTROL_COMMAND   Cmd;
  TPM2_HIERARCHY_CONTROL_RESPONSE  Res;
  UINT32                           CmdSize;
  UINT32                           RespSize;
  UINT8                            *Buffer;
  UINT32                           SessionInfoSize;
  UINT8                            *ResultBuf;
  UINT32                           ResultBufSize;

  if(TrEEProtocolInstance == NULL) return EFI_NOT_FOUND; 
  //
  // Construct command
  //
  Cmd.Header.tag          = (UINT16)TPM_H2NS(TPM_ST_SESSIONS);
  Cmd.Header.paramSize    = TPM_H2NL(sizeof(Cmd));
  Cmd.Header.commandCode  = TPM_H2NL(TPM_CC_HierarchyControl);
  Cmd.AuthHandle          = TPM_H2NL(AuthHandle);

  //
  // Add in Auth session
  //
  Buffer = (UINT8 *)&Cmd.AuthSession;

  // sessionInfoSize
  SessionInfoSize = CopyAuthSessionCommand (AuthSession, Buffer);
  Buffer += SessionInfoSize;
  Cmd.AuthorizationSize = TPM_H2NL(SessionInfoSize);

  WriteUnaligned32 ((UINT32 *)Buffer, TPM_H2NL(Hierarchy));
  Buffer += sizeof(UINT32);

  *(UINT8 *)Buffer = State;
  Buffer += sizeof(UINT8);

  CmdSize = (UINT32)(Buffer - (UINT8 *)&Cmd);
  Cmd.Header.paramSize = TPM_H2NL(CmdSize);

  ResultBuf     = (UINT8 *) &Res;
  ResultBufSize = sizeof(Res);

  Status = TrEEProtocolInstance->SubmitCommand(TrEEProtocolInstance, CmdSize, (UINT8 *)&Cmd,  ResultBufSize, ResultBuf );
  if (EFI_ERROR (Status)) {
       DEBUG(( EFI_D_ERROR, "Tpm2GetRandom TrEEProtocolInstance->SubmitCommand = %r \n", Status));
       return Status;
   }

   if (ResultBufSize > sizeof(Res)) {
       DEBUG ((EFI_D_ERROR, "HierarchyControl: Failed ExecuteCommand: Buffer Too Small\r\n"));
       return EFI_BUFFER_TOO_SMALL;
   }

  //
  // Validate response headers
  //
  RespSize = TPM_H2NL(Res.Header.paramSize);
  if (RespSize > sizeof(Res)) {
    DEBUG ((EFI_D_ERROR, "HierarchyControl: Response size too large! %d\r\n", RespSize));
    return EFI_BUFFER_TOO_SMALL;
  }

  //
  // Fail if command failed
  //
  if (TPM_H2NL(Res.Header.responseCode) != TPM_RC_SUCCESS) {
    DEBUG((EFI_D_ERROR,"HierarchyControl: Response Code error! 0x%08x\r\n", TPM_H2NL(Res.Header.responseCode)));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}





//**********************************************************************
//<AMI_PHDR_START>
//
// Procedure:  read_PPI_request
//
// Description: Reads and returns TCG PPI requests Value
//
//
// Input:
//
// Output:      UINT8
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
UINT8 ReadPpiRequest( )
{
    UINTN          Size = sizeof(AMI_PPI_NV_VAR);
    AMI_PPI_NV_VAR Temp;
    EFI_STATUS     Status;

    Status = gRT->GetVariable( L"AMITCGPPIVAR", \
                               &AmitcgefiOsVariableGuid, \
                               NULL, \
                               &Size, \
                               &Temp );

    if(Status == EFI_NOT_FOUND)
    {
        Temp.RQST    = 0;
        Temp.RCNT    = 0;
        Temp.ERROR   = 0;
        Temp.Flag    = 0;
        Temp.AmiMisc = 0;

         Status = gRT->SetVariable( L"AMITCGPPIVAR", \
                             &AmitcgefiOsVariableGuid, \
                             EFI_VARIABLE_NON_VOLATILE   \
                               | EFI_VARIABLE_BOOTSERVICE_ACCESS, \
                               Size, \
                               &Temp );

    }

    return Temp.RQST;
}


EFI_STATUS HandleTpm20Setup()
{
    EFI_STATUS                        Status = EFI_SUCCESS;
    TCG_PLATFORM_SETUP_PROTOCOL       *ProtocolInstance;
    EFI_GUID                          Policyguid = TCG_PLATFORM_SETUP_POLICY_GUID;

    Status = gBS->LocateProtocol (&Policyguid,  NULL, &ProtocolInstance);
    if (EFI_ERROR (Status)) {
        return 0;
    }
    
    if(!ProtocolInstance->ConfigFlags.EndorsementHierarchy){
        Tpm2HierarchyControl(TPM_RH_PLATFORM,  NULL, TPM_RH_ENDORSEMENT,  0);
    }
    
    if(!ProtocolInstance->ConfigFlags.PlatformHierarchy){
        Tpm2HierarchyControl(TPM_RH_PLATFORM,  NULL, TPM_RH_PLATFORM,  0);
    }
    
    if(!ProtocolInstance->ConfigFlags.StorageHierarchy){
        Tpm2HierarchyControl(TPM_RH_PLATFORM,  NULL, TPM_RH_OWNER,  0);
    }
    
    
    if(ProtocolInstance->ConfigFlags.TpmOperation == 1){
        Status = Tpm2ClearControl(0);
        if(!EFI_ERROR(Status)){  
           Status = Tpm2Clear();
        }
        ProtocolInstance->ConfigFlags.TpmOperation = 0;
        ProtocolInstance->UpdateStatusFlags(&ProtocolInstance->ConfigFlags, TRUE);  
    }
            
    return Status;
}



void HandleTpm20Ppi(IN EFI_EVENT ev,
                    IN VOID *ctx)
{
    BOOLEAN             UserAction;
    UINT8               StringType = 0;
    UINTN               CurX, CurY;
    CHAR16              *StrBuffer = NULL;
    CHAR16              *String;
#if (AUTO_ACCEPT_PPI) == 0   
    EFI_INPUT_KEY       key;
#endif
    TSE_POST_STATUS     TsePostStatus;
    EFI_STATUS          Status;
    

    DEBUG((-1, "HandleTpm20Ppi Entry \n"));
    if (pAmiPostMgr == NULL) {
        Status = gBS->LocateProtocol( &gAmiPostManagerProtocolGuid,
                                      NULL,
                                      &pAmiPostMgr );
        if (EFI_ERROR(Status)) {
            return;
        }
    }

    //
    // Calling GetPostStatus() to check current TSE_POST_STATUS
    //
    TsePostStatus = pAmiPostMgr->GetPostStatus();

    switch(PpiRequest){
        case TPM20_PP_NO_ACTION_MAX:
                return;
        
        case TPM20_PP_CLEAR_CONTROL_CLEAR:          
        case TPM20_PP_CLEAR_CONTROL_CLEAR_2:
        case TPM20_PP_CLEAR_CONTROL_CLEAR_3:
        case TPM20_PP_CLEAR_CONTROL_CLEAR_4:
            if(TpmNvflags.NoPpiClear != 1)
            {
                UserAction = TRUE;
                StringType = 1;
                break;
            }
        case TPM20_PP_SET_NO_PPI_CLEAR_FALSE:
            UserAction = FALSE;
            StringType = 0;
            break;

        case TPM20_PP_SET_NO_PPI_CLEAR_TRUE:
            if(TpmNvflags.NoPpiClear != 1)
            {
                UserAction = TRUE;
                StringType = 2;
                break;
            }
        
        default:
            if (PpiRequest <= TPM20_PP_NO_ACTION_MAX){
                WritePpiResult( PpiRequest, (UINT16)(0));
            }else{
                WritePpiResult( PpiRequest, (UINT16)(TCPA_PPI_BIOSFAIL));
            }
            return;
    }

    if(UserAction)
    {
         pAmiPostMgr->SwitchToPostScreen( );

         Status = gBS->AllocatePool(EfiBootServicesData,
                        sizeof (CHAR16) * 0x100,
                        (VOID*) &StrBuffer);

         if ( EFI_ERROR( Status ) || StrBuffer == NULL )
         {
            return;
         }

         gBS->SetMem( StrBuffer, sizeof (CHAR16) * 0x100, 0 );

         pAmiPostMgr->DisplayPostMessage( StrBuffer );

         pAmiPostMgr->GetCurPos(&CurX, &CurY);  

         CurX =  0;
         CurY -= PPI_DISPLAY_OFFSET;  
    
         if(StringType == 1){
             GetStringFromToken( STRING_TOKEN(TPM_CLEAR_STR), &String );
             pAmiPostMgr->DisplayPostMessage( String );
             gBS->FreePool(String);
             pAmiPostMgr->GetCurPos(&CurX, &CurY); 
             CurX =0;
             CurY-=2;
             pAmiPostMgr->SetCurPos(CurX, CurY);
             GetStringFromToken( STRING_TOKEN( TPM_WARNING_CLEAR ), &String );
             pAmiPostMgr->DisplayPostMessage( String );
             gBS->FreePool(String);
             GetStringFromToken( STRING_TOKEN( TPM_CAUTION_KEY ), &String );
             pAmiPostMgr->DisplayPostMessage( String );
             pAmiPostMgr->GetCurPos(&CurX, &CurY); 
             CurX += StrLen(String)+1;
             CurY-=1;
             pAmiPostMgr->SetCurPos(CurX, CurY);
             gBS->FreePool(String);

         }else if(StringType == 2){
             
             GetStringFromToken( STRING_TOKEN(TPM_PPI_HEAD_STR), &String );
             pAmiPostMgr->DisplayPostMessage( String );
             gBS->FreePool(String);
             pAmiPostMgr->GetCurPos(&CurX, &CurY); 
             CurX =0;
             CurY+=1;
             pAmiPostMgr->SetCurPos(CurX, CurY);
             GetStringFromToken( STRING_TOKEN( TPM_NOTE_CLEAR ), &String );
             pAmiPostMgr->DisplayPostMessage( String );
             gBS->FreePool(String);
             GetStringFromToken( STRING_TOKEN( TPM_ACCEPT_KEY ), &String );
             pAmiPostMgr->DisplayPostMessage( String );
             pAmiPostMgr->GetCurPos(&CurX, &CurY); 
             CurX += StrLen(String)+1;
             pAmiPostMgr->SetCurPos(CurX, CurY);
             gBS->FreePool(String);
         }
         
        GetStringFromToken( STRING_TOKEN( TPM_REJECT_KEY ), &String );
        pAmiPostMgr->DisplayPostMessage( String );
        gBS->FreePool(String);

        if ( gST->ConIn )
        {
#if (AUTO_ACCEPT_PPI) == 0
            while ( TRUE )
            {
                Status = gST->ConIn->ReadKeyStroke( gST->ConIn, &key );
                if ( Status == EFI_SUCCESS )
                {
                    if ( PpiRequest == TPM20_PP_CLEAR_CONTROL_CLEAR ||  
                         PpiRequest == TPM20_PP_CLEAR_CONTROL_CLEAR_2 ||
                         PpiRequest == TPM20_PP_CLEAR_CONTROL_CLEAR_3 ||
                         PpiRequest == TPM20_PP_CLEAR_CONTROL_CLEAR_4 )
                    {
                        if ( key.ScanCode == TCG_CLEAR_REQUEST_KEY )
                        {
                            break;
                        } else if ( key.ScanCode == TCG_CONFIGURATION_IGNORE_KEY )
                        {
                            return;
                        }
                     }
                    else if(PpiRequest == TPM20_PP_SET_NO_PPI_CLEAR_FALSE ||
                             PpiRequest == TPM20_PP_SET_NO_PPI_CLEAR_TRUE)
                    {                                
    
                        if ( key.ScanCode == TCG_CONFIGURATION_ACCEPT_KEY )
                        {
                            break;
                        }
                        else if ( key.ScanCode == TCG_CONFIGURATION_IGNORE_KEY )
                        {
                           return;
                        }
                    }
                    else if ( key.ScanCode == TCG_CONFIGURATION_IGNORE_KEY )
                    {
                        return;
                    }
                }
            }
#endif 
        }       

    }

    if ( PpiRequest == TPM20_PP_CLEAR_CONTROL_CLEAR ||
         PpiRequest == TPM20_PP_CLEAR_CONTROL_CLEAR_2 ||
         PpiRequest == TPM20_PP_CLEAR_CONTROL_CLEAR_3 ||
         PpiRequest == TPM20_PP_CLEAR_CONTROL_CLEAR_4 )
    {
         Status = Tpm2ClearControl(0);
         if(!EFI_ERROR(Status)){
            
            Status = Tpm2Clear();
            if(EFI_ERROR(Status)){
                DEBUG((-1, "Error Clearing TPM20 device Status = %r \n", Status));
                WritePpiResult( PpiRequest, (UINT16)(TCPA_PPI_BIOSFAIL));
            }else{
                WritePpiResult( PpiRequest, (UINT16)(0));
            }
         }else{
            DEBUG((-1, "Tpm2ClearControl failure Status = %r \n", Status));
            WritePpiResult( PpiRequest, (UINT16)(TCPA_PPI_BIOSFAIL));
        }
    }
    else if(PpiRequest == TPM20_PP_SET_NO_PPI_CLEAR_FALSE ||
            PpiRequest == TPM20_PP_SET_NO_PPI_CLEAR_TRUE)
    {
        if(PpiRequest == TPM20_PP_SET_NO_PPI_CLEAR_FALSE)
        {
            TpmNvflags.NoPpiClear = 0;

        }else{
            TpmNvflags.NoPpiClear = 1;
        }
         
        Status = gRT->SetVariable(L"TPMPERBIOSFLAGS",
                          &FlagsStatusguid,
                          EFI_VARIABLE_NON_VOLATILE
                          | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                          sizeof (PERSISTENT_BIOS_TPM_FLAGS),
                          &TpmNvflags );

        if(EFI_ERROR(Status)){
                DEBUG((-1, "Error Clearing TPM20 device\n"));
                WritePpiResult( PpiRequest, (UINT16)(TCPA_PPI_BIOSFAIL));
         }
         else{
            WritePpiResult( PpiRequest, (UINT16)(0));    
        }
    }else{
        WritePpiResult( PpiRequest, (UINT16)(0));
    }

    DEBUG((-1, "TPM20 changes made reseting system\n"));
    gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
}

VOID
EFIAPI
Tpm20OnExitBootServices (
  IN      EFI_EVENT                 Event,
  IN      VOID                      *Context
  )
{
  EFI_STATUS    Status;

  //
  // Measure invocation of ExitBootServices,
  //
  Status = MeasureAction (
             "Exit Boot Services Invocation");

  //
  // Measure success of ExitBootServices
  //
  Status = MeasureAction (
             "Exit Boot Services Returned with Success");
}


//*************************************************************************
//<AMI_PHDR_START>
//
// Name: Tpm20LoadStrings
//
// Description:
//  EFI_STATUS LoadStrings(EFI_HANDLE ImageHandle,
// EFI_HII_HANDLE *pHiiHandle) - loads HII string packages associated with 
// the specified image and publishes them to the HII database
//
// Input:
//  EFI_HANDLE ImageHandle - Image Handle
//  EFI_HII_HANDLE *pHiiHandle - HII package list handle
//
// Output:
//   EFI_STATUS
//
//<AMI_PHDR_END>
//*************************************************************************
EFI_STATUS Tpm20LoadStrings(
    EFI_HANDLE ImageHandle, EFI_HII_HANDLE *pHiiHandle
)
{
    EFI_STATUS                      Status;
    EFI_HII_PACKAGE_LIST_HEADER     *PackageList;
    EFI_HII_DATABASE_PROTOCOL       *HiiDatabase;
    
    //
    // Retrieve HII package list from ImageHandle
    //
    Status = gBS->OpenProtocol (
        ImageHandle,
        &gEfiHiiPackageListProtocolGuid,
        (VOID **) &PackageList,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );
    if (EFI_ERROR (Status)) {
        DEBUG((-1,"gEfiHiiPackageListProtocolGuid protocol is not found\n"));
        return Status;
    }
        
    Status = gBS->LocateProtocol (
           &gEfiHiiDatabaseProtocolGuid,
           NULL,
           &HiiDatabase
       );
       if (EFI_ERROR (Status)) {
           DEBUG((-1,"gEfiHiiDatabaseProtocolGuid protocol is not found\n"));
           return Status;
       }
     
    //
    // Publish HII package list to HII Database.
    //
    
    Status = HiiDatabase->NewPackageList (
        HiiDatabase,
        PackageList,
        NULL,
        pHiiHandle
    );
    DEBUG((-1,"NewPackageList status: %r\n",Status));
    return Status;
}


EFI_STATUS
Tpm20PlatformEntry(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
    EFI_STATUS          Status;
    EFI_GUID            gEfiTrEEProtocolGuid = EFI_TREE_PROTOCOL_GUID;
    EFI_EVENT           ReadyToBootEvent;
    EFI_EVENT           ev;
    EFI_EVENT           ExitBSEvent;
    static VOID         *reg;
    UINTN               Size = sizeof(PERSISTENT_BIOS_TPM_FLAGS);
    
    DEBUG((-1, "Tpm20PlatformEntry\n"));
    
    Status = gBS->LocateProtocol(&gEfiTrEEProtocolGuid, NULL, &TrEEProtocolInstance);
    if(EFI_ERROR(Status))return Status;

    Status = InstallTcgPlatformProtocol();
    if(EFI_ERROR(Status))return Status;

    //Update to NIST measuremetns
    Status = FindAndMeasureDxeFWVol();
    ASSERT_EFI_ERROR(Status);
    
    //we found TrEE protocol do Tpm20 
    //Initializations set ready to boot callback
    //install platform protocol

    Status = EfiCreateEventReadyToBootEx(TPL_CALLBACK,
                                    Tpm20OnReadyToBoot, 
                                    NULL, 
                                    &ReadyToBootEvent);
    if(EFI_ERROR(Status))return Status;

    Status = gBS->CreateEvent (
                      EVT_SIGNAL_EXIT_BOOT_SERVICES,
                      EFI_TPL_NOTIFY,
                      Tpm20OnExitBootServices,
                      NULL,
                      &ExitBSEvent
                      );
    if(EFI_ERROR(Status))return Status;
   
    Tpm20LoadStrings( ImageHandle, &gHiiHandle );
    
    Status = gRT->GetVariable( L"TPMPERBIOSFLAGS", \
                               &FlagsStatusguid, \
                               NULL, \
                               &Size, \
                               &TpmNvflags );

    if(EFI_ERROR(Status))
    {
        TpmNvflags.NoPpiProvision = 1;  
        TpmNvflags.NoPpiClear = 0;
        TpmNvflags.NoPpiMaintenance = 0;

        Status = gRT->SetVariable(L"TPMPERBIOSFLAGS",
                         &FlagsStatusguid,
                      	 EFI_VARIABLE_NON_VOLATILE
                         | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                         sizeof (PERSISTENT_BIOS_TPM_FLAGS),
                         &TpmNvflags );

        if(EFI_ERROR(Status))return Status;
    }
       
    HandleTpm20Setup();
    PpiRequest = ReadPpiRequest();
    
    PpiRequest &= 0xFF;

    if(PpiRequest > 0  &&  PpiRequest <= TCPA_PPIOP_ENABLE_ACTV_CLEAR_ENABLE_ACTV){
        
        Status = gBS->CreateEvent( EFI_EVENT_NOTIFY_SIGNAL,
                                   EFI_TPL_CALLBACK,
                                   HandleTpm20Ppi,
                                   0,
                                   &ev );

        if(EFI_ERROR(Status)){
            return Status;
        }

        Status = gBS->RegisterProtocolNotify( 
                        &gBdsAllDriversConnectedProtocolGuid,
                        ev,
                        &reg );
        if(EFI_ERROR(Status)) {
            return Status;
        }
    }

    return Status;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**     5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093            **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
