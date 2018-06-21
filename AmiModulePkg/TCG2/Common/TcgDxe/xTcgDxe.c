//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**             5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093 **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/TCG/xTcgDxe.c 101   5/09/12 3:52p Fredericko $
//
// $Revision: 101 $
//
// $Date: 5/09/12 3:52p $
//**********************************************************************
//--------------------------------------------------------------------------
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  xTcgDxe.c
//
// Description: 
//  Most Tcg DXE initialization and measurements are done here
//
//<AMI_FHDR_END>
//*************************************************************************
#include <EFI.h>
#include <AmiTcg\TcgCommon.h>
#include <token.h>

#if SMBIOS_SUPPORT == 1
  #include <Protocol\SmBios.h>
#endif

#include <AmiTcg\TcgMisc.h>
#include "AmiTcg\TcgPrivate.h"
#include<AmiTcg\sha1.h>
//#include <AmiDxeLib.h>
#include <AmiTcg\TcgPrivate.h>
#include <IndustryStandard/PeImage.h>
#include <Protocol/DiskIo.h>
#include <Protocol/BlockIo.h>
#include <protocol\TcgService.h>
#include <protocol\TpmDevice.h>
#include "Protocol/CpuIo.h"
#include "Protocol/FirmwareVolume.h"
#include "Protocol/DevicePath.h"
#include "Protocol/PciIo.h"
#include <Protocol\TcgPlatformSetupPolicy.h>
#include <AmiProtocol.h>
#include <Protocol\AmiTpmSupportTypeProtocol.h>
#include<Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiBootServicesTableLib.h>
//#include <Library/DevicePathLib.h>
#include <Library/DebugLib.h>
#include<Library/BaseLib.h>
#include<Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#if (defined(TCGMeasureSecureBootVariables) && (TCGMeasureSecureBootVariables != 0))
#include <ImageAuthentication.h>
#endif

//------------------------------------------------------------------------
//Internal Structures
//------------------------------------------------------------------------
typedef struct _TCG_DXE_PRIVATE_DATA
{
    EFI_TCG_PROTOCOL        TcgServiceProtocol;
    EFI_TPM_DEVICE_PROTOCOL *TpmDevice;
} TCG_DXE_PRIVATE_DATA;

EFI_STATUS
__stdcall TcgDxeCommonExtend(
    IN VOID         *CallbackContext,
    IN TPM_PCRINDEX PCRIndex,
    IN TCG_DIGEST   *Digest,
    OUT TCG_DIGEST  *NewPCRValue );

EFI_STATUS
__stdcall SHA1_init (
    IN VOID     *CallbackContext,
    IN SHA1_CTX *Sha1Ctx );

EFI_STATUS
__stdcall SHA1_update (
    IN VOID     *CallbackContext,
    IN SHA1_CTX *Sha1Ctx,
    IN VOID     *Data,
    IN UINTN    DataLen );

EFI_STATUS
__stdcall SHA1_final (
    IN VOID        *CallbackContext,
    IN SHA1_CTX    *Sha1Ctx,
    OUT TCG_DIGEST **Digest
    );


#define TCG_DXE_PRIVATE_DATA_FROM_THIS( This )  \
    _CR( This, TCG_DXE_PRIVATE_DATA, TcgServiceProtocol )
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

#define   GUID_VARIABLE_DECLARATION( Variable, Guid ) extern EFI_GUID Variable

//EFI_GUID                  gEfiTcgProtocolGuid = EFI_TCG_PROTOCOL_GUID;
EFI_GUID                  gDsdtGuidDxe        = DSDT_GUID;
EFI_GUID                  TcgEfiGlobalVariableGuid = TCG_EFI_GLOBAL_VARIABLE_GUID;
EFI_GUID                  AmiTcgPlatformProtocolGuid = AMI_TCG_PLATFORM_PROTOCOL_GUID;
EFI_GUID                  mAmiTpmSupportTypeProtocol =AMI_TPM_SUPPORT_TYPE_PROTOCOL_GUID;
static BOOLEAN            BootLaunchDone = FALSE;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#include <protocol/AcpiSupport.h>

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
EFI_STATUS EFIAPI TcgDxeHashLogExtendEvent (
    IN EFI_TCG_PROTOCOL     *This,
    IN EFI_PHYSICAL_ADDRESS HashData,
    IN UINT64               HashDataLen,
    IN TCG_ALGORITHM_ID     AlgorithmId,
    IN OUT TCG_PCR_EVENT    *TCGLogData,
    IN OUT UINT32           *evNum );

EFI_STATUS
EFIAPI
TcgMeasureGptTable (
  IN      EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  );


/////////////////////////////////////////////////
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



#if (defined(TCGMeasureSecureBootVariables) && (TCGMeasureSecureBootVariables != 0))
EFI_STATUS
MeasureCertificate(UINTN sizeOfCertificate, 
                   UINT8 *pterCertificate)
{
    EFI_STATUS   Status;
    TCG_PCR_EVENT             *TcgEvent = NULL;
    EFI_TCG_PROTOCOL          *tcgSvc;
    TCG_EFI_VARIABLE_DATA     *VarLog;
    EFI_PHYSICAL_ADDRESS      Last;
    UINT32                    evNum;
    BOOLEAN                   AlreadyMeasuredCert = FALSE;
    UINTN                     i=0;
    UINTN                     VarNameLength;
    static BOOLEAN            initialized = 0;
    static TPM_DIGEST         digestTrackingArray[5];
    static TPM_DIGEST         zeroDigest;
    UINT8                     *tempDigest = NULL;
    UINT64                    HashedDataLen = 20; 

    if(!initialized)
    {
        for(i=0;i<5; i++)
        {
            SetMem(digestTrackingArray[i].digest,20,0);
        }
        SetMem(zeroDigest.digest,20,0);
        initialized = TRUE;
    }

    Status = gBS->LocateProtocol(&gEfiTcgProtocolGuid,
                                     NULL, &tcgSvc );

    if(EFI_ERROR(Status))return Status;

    VarNameLength = StrLen(L"db");
    Status = gBS->AllocatePool( EfiBootServicesData,
               _TPM_STRUCT_PARTIAL_SIZE( TCG_PCR_EVENT,Event ) 
                + (UINT32)(sizeof(*VarLog) + VarNameLength 
                * sizeof(CHAR16) + sizeOfCertificate - 3),
                &TcgEvent);

    if(EFI_ERROR(Status))return Status;
        
    TcgEvent->PCRIndex  = 7;
    TcgEvent->EventType = 0x800000E0;

    TcgEvent->EventSize = (UINT32)( sizeof (*VarLog) + VarNameLength 
                              * sizeof (CHAR16) + sizeOfCertificate) - 3;

    gBS->AllocatePool( EfiBootServicesData, TcgEvent->EventSize, &VarLog );

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

    gBS->CopyMem( TcgEvent->Event,
                  VarLog,
                  TcgEvent->EventSize );

    //before extending verify if we have already measured it.
    tcgSvc->HashAll(tcgSvc,
        (UINT8 *)VarLog,
        TcgEvent->EventSize,
        4,
        &HashedDataLen,
        &tempDigest);

    for(i=0; i<5; i++)
    {
        //tempDigest
        if(!CompareMem(digestTrackingArray[i].digest, tempDigest, 20))
        return EFI_SUCCESS; //already measured

        if(!CompareMem(digestTrackingArray[i].digest, zeroDigest.digest, 20))
        break; //we need to measure
    }

    gBS->CopyMem(digestTrackingArray[i].digest, tempDigest, 20);

    Status = tcgSvc->HashLogExtendEvent(tcgSvc,
                            (EFI_PHYSICAL_ADDRESS)VarLog,
                            TcgEvent->EventSize,
                            4,
                            TcgEvent,
                            &evNum,
                            &Last );

    return Status;
}



VOID * TcgGetConfigurationTable(
    IN EFI_SYSTEM_TABLE *SystemTable,
    IN EFI_GUID         *Guid
    )
{
    EFI_CONFIGURATION_TABLE *Table = SystemTable->ConfigurationTable;
    UINTN i = SystemTable->NumberOfTableEntries;

    for (;i;--i,++Table)
    {
        if (CompareMem(&Table->VendorGuid,Guid, sizeof(EFI_GUID))==0) return Table->VendorTable;
    }
    return 0;
}



EFI_STATUS FindandMeasureSecureBootCertificate()
{
    EFI_STATUS      Status;
    UINTN           VarSize  = 0;
    UINT8           *SecureDBBuffer = NULL;
    UINT8           *CertificateBuffer = NULL;
    UINTN           SizeofCerificate = 0;
    EFI_GUID     Certificateguid = AMI_VALID_BOOT_IMAGE_CERT_TBL_GUID;
    AMI_VALID_CERT_IN_SIG_DB    *CertInfo;
    UINT8           *CertOffsetPtr = NULL;
   
     Status   = gRT->GetVariable(L"db",
                    &gEfiImageSecurityDatabaseGuid,
                    NULL,
                    &VarSize,
                    NULL);

     if(EFI_ERROR(Status))
     {
	     if ( Status != EFI_BUFFER_TOO_SMALL )
	     {
		     return EFI_NOT_FOUND;
	     }
     }


    gBS->AllocatePool( EfiBootServicesData, VarSize, &SecureDBBuffer );
    
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
    CertInfo = TcgGetConfigurationTable(gST, &Certificateguid);
    if(CertInfo == NULL){
     DEBUG(( -1,"db variable found SecCertificate Information not found in EFI System Table \n"));
     return EFI_NOT_FOUND;
    }
    if(CertInfo->SigLength == 0){
     DEBUG(( -1,"SecCertificate Information found in EST but Information might be invalid \n"));
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


#endif




//--------------------------------------------------------------------------------------------
//Description:  Measure a PE/COFF image into PCR 2 or 4 depending on Boot policy of 0 or 1
//Arguments:
// 	BootPolicy  - Boolean value of 0 or 1 for PCR index 2 or 4.
//  ImageContext - Contains details about the image.
//  LinkTimeBase - Linking time Image Address
//  ImageType    - EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION, BOOT_SERVICE_DRIVER, EFI_RUNTIME_DRIVER
//  DeviceHandle - Device identification handle
//  FilePath     - Device File path
//Output:  EFI_SUCCESS - Image Measured successfully.
//---------------------------------------------------------------------------------------------

EFI_STATUS
EFIAPI
TcgMeasurePeImage (
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
	TCG_PCR_EVENT                     *TcgEvent;
	TCG_PCR_EVENT                     *TcgEventlog = NULL;
	EFI_IMAGE_LOAD_EVENT              *ImageLoad;
	UINT32                            FilePathSize;
	SHA1_CTX                          Sha1Ctx;
	EFI_IMAGE_DOS_HEADER              *DosHdr;
	UINT32                            PeCoffHeaderOffset;
	EFI_IMAGE_SECTION_HEADER          *Section;
	UINT8                             *HashBase;
	UINTN                             HashSize;
	UINTN                             SumOfBytesHashed;
	EFI_IMAGE_SECTION_HEADER          *SectionHeader;
	UINTN                             Index, Pos;
	UINT16                            Magic;
	UINT32                            EventSize;
	UINT32                            EventNumber;
    EFI_TCG_PROTOCOL			      *TcgProtocol;
	EFI_TPM_DEVICE_PROTOCOL	          *TpmProtocol;
	TCG_DIGEST                        Sha1Digest;
	EFI_IMAGE_OPTIONAL_HEADER_PTR_UNION  Hdr;

      
	Status        = EFI_SUCCESS;
	ImageLoad     = NULL;
	SectionHeader = NULL;
	FilePathSize  = (UINT32) GetDevicePathSize (FilePath);

	if(AutoSupportType()){
	    return EFI_SUCCESS;
    }
    
	Status = gBS->LocateProtocol (&gEfiTcgProtocolGuid,  NULL, &TcgProtocol);
	if (EFI_ERROR (Status)) {
      return Status;
	}

	Status = gBS->LocateProtocol (&gEfiTpmDeviceProtocolGuid,  NULL, &TpmProtocol );
	if (EFI_ERROR (Status)) {
      return Status;
	}

	TpmProtocol->Init(TpmProtocol);

	EventSize = sizeof (*ImageLoad) - sizeof (ImageLoad->DevicePath) + FilePathSize;
	TcgEvent = AllocateZeroPool (EventSize + sizeof (TCG_PCR_EVENT));
	if (TcgEvent == NULL) {
     return EFI_OUT_OF_RESOURCES;
	}

	TcgEvent->EventSize = EventSize;
	ImageLoad           = (EFI_IMAGE_LOAD_EVENT *) TcgEvent->Event;


    switch (ImageType) {
        case EFI_IMAGE_SUBSYSTEM_EFI_APPLICATION:
          TcgEvent->EventType = EV_EFI_BOOT_SERVICES_APPLICATION;
          TcgEvent->PCRIndex  = 4;
          break;
        case EFI_IMAGE_SUBSYSTEM_EFI_BOOT_SERVICE_DRIVER:
          TcgEvent->EventType = EV_EFI_BOOT_SERVICES_DRIVER;
          TcgEvent->PCRIndex  = 2;
          break;
        case EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
          TcgEvent->EventType = EV_EFI_RUNTIME_SERVICES_DRIVER;
          TcgEvent->PCRIndex  = 2;
          break;
        default:
          TcgEvent->EventType = ImageType;
          Status = EFI_UNSUPPORTED;
          goto Finish;
      }

      ImageLoad->ImageLocationInMemory = ImageAddress;
      ImageLoad->ImageLengthInMemory   = ImageSize;
      ImageLoad->ImageLinkTimeAddress  = LinkTimeBase;
      ImageLoad->LengthOfDevicePath    = FilePathSize;
      gBS->CopyMem (ImageLoad->DevicePath, FilePath, FilePathSize);

      //
      // Check PE/COFF image
      //
      DosHdr = (EFI_IMAGE_DOS_HEADER *) (UINTN) ImageAddress;
      PeCoffHeaderOffset = 0;
      if (DosHdr->e_magic == EFI_IMAGE_DOS_SIGNATURE) {
        PeCoffHeaderOffset = DosHdr->e_lfanew;
      }
      if (((EFI_TE_IMAGE_HEADER *)((UINT8 *) (UINTN) ImageAddress + PeCoffHeaderOffset))->Signature
           == EFI_TE_IMAGE_HEADER_SIGNATURE) {
        goto Finish;
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
  SHA1Init( &Sha1Ctx);

  //
  // Measuring PE/COFF Image Header; 
  // But CheckSum field and SECURITY data directory (certificate) are excluded
  //
  Hdr.Pe32 = (EFI_IMAGE_NT_HEADERS32 *)((UINT8 *) (UINTN) ImageAddress + PeCoffHeaderOffset);
    Magic    = Hdr.Pe32->OptionalHeader.Magic;

    //
    // 3.  Calculate the distance from the base of the image header to the image checksum address.
    // 4.  Hash the image header from its base to beginning of the image checksum.
    //
    HashBase = (UINT8 *) (UINTN) ImageAddress;
    if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
      //
      // Use PE32 offset
      //
      HashSize = (UINTN) ((UINT8 *)(&Hdr.Pe32->OptionalHeader.CheckSum) - HashBase);
    } else {
      //
      // Use PE32+ offset
      //
      HashSize = (UINTN) ((UINT8 *)(&Hdr.Pe32Plus->OptionalHeader.CheckSum) - HashBase);
    }

  SHA1Update( &Sha1Ctx,
               HashBase,
              (u32) HashSize
               );

  //
  // 5.	Skip over the image checksum (it occupies a single ULONG).
  // 6.	Get the address of the beginning of the Cert Directory.
  // 7.	Hash everything from the end of the checksum to the start of the Cert Directory.
  //
  if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
      //
      // Use PE32 offset
      //
      HashBase = (UINT8 *) &Hdr.Pe32->OptionalHeader.CheckSum + sizeof (UINT32);
      HashSize = (UINTN) ((UINT8 *)(&Hdr.Pe32->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY]) - HashBase);
    } else {
      //
      // Use PE32+ offset
      //
      HashBase = (UINT8 *) &Hdr.Pe32Plus->OptionalHeader.CheckSum + sizeof (UINT32);
      HashSize = (UINTN) ((UINT8 *)(&Hdr.Pe32Plus->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY]) - HashBase);
    }

   SHA1Update( &Sha1Ctx,
               HashBase,
               (u32)HashSize
               );  
  //
  // 8.	Skip over the Cert Directory. (It is sizeof(IMAGE_DATA_DIRECTORY) bytes.)
  // 9.	Hash everything from the end of the Cert Directory to the end of image header.
  //
  if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
     //
     // Use PE32 offset
     //
     HashBase = (UINT8 *) &Hdr.Pe32->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY + 1];
     HashSize = Hdr.Pe32->OptionalHeader.SizeOfHeaders -
              (UINTN) ((UINT8 *)(&Hdr.Pe32->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY + 1]) - (UINT8 *) (UINTN) ImageAddress);
   } else {
     //
     // Use PE32+ offset
     //
     HashBase = (UINT8 *) &Hdr.Pe32Plus->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY + 1];
     HashSize = Hdr.Pe32Plus->OptionalHeader.SizeOfHeaders -
              (UINTN) ((UINT8 *)(&Hdr.Pe32Plus->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY + 1]) - (UINT8 *) (UINTN) ImageAddress);
   }


 SHA1Update( &Sha1Ctx,
               HashBase,
               (u32)HashSize
               );

  //
  // 10. Set the SUM_OF_BYTES_HASHED to the size of the header 
  //
  if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
      //
      // Use PE32 offset
      //
      SumOfBytesHashed = Hdr.Pe32->OptionalHeader.SizeOfHeaders;
    } else {
      //
      // Use PE32+ offset
      //
      SumOfBytesHashed = Hdr.Pe32Plus->OptionalHeader.SizeOfHeaders;
    }


  //
  // 11. Build a temporary table of pointers to all the IMAGE_SECTION_HEADER 
  //     structures in the image. The 'NumberOfSections' field of the image 
  //     header indicates how big the table should be. Do not include any 
  //     IMAGE_SECTION_HEADERs in the table whose 'SizeOfRawData' field is zero.   
  //

  SectionHeader = (EFI_IMAGE_SECTION_HEADER *)AllocateZeroPool (sizeof (EFI_IMAGE_SECTION_HEADER) * Hdr.Pe32->FileHeader.NumberOfSections);
    if (SectionHeader == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Finish;
    }

  //
  // 12.	Using the 'PointerToRawData' in the referenced section headers as 
  //      a key, arrange the elements in the table in ascending order. In other 
  //      words, sort the section headers according to the disk-file offset of 
  //      the section.
  //
    Section = (EFI_IMAGE_SECTION_HEADER *) (
                   (UINT8 *) (UINTN) ImageAddress +
                   PeCoffHeaderOffset +
                   sizeof(UINT32) +
                   sizeof(EFI_IMAGE_FILE_HEADER) +
                   Hdr.Pe32->FileHeader.SizeOfOptionalHeader
                   );
      for (Index = 0; Index < Hdr.Pe32->FileHeader.NumberOfSections; Index++) {
        Pos = Index;
        while ((Pos > 0) && (Section->PointerToRawData < SectionHeader[Pos - 1].PointerToRawData)) {
          gBS->CopyMem (&SectionHeader[Pos], &SectionHeader[Pos - 1], sizeof(EFI_IMAGE_SECTION_HEADER));
          Pos--;
        }
        gBS->CopyMem (&SectionHeader[Pos], Section, sizeof(EFI_IMAGE_SECTION_HEADER));
        Section += 1;
      }
  
  //
  // 13.	Walk through the sorted table, bring the corresponding section 
  //      into memory, and hash the entire section (using the 'SizeOfRawData' 
  //      field in the section header to determine the amount of data to hash).
  // 14.	Add the section's 'SizeOfRawData' to SUM_OF_BYTES_HASHED .
  // 15.	Repeat steps 13 and 14 for all the sections in the sorted table.
  //
     for (Index = 0; Index < Hdr.Pe32->FileHeader.NumberOfSections; Index++) {
          Section  = (EFI_IMAGE_SECTION_HEADER *) &SectionHeader[Index];
          if (Section->SizeOfRawData == 0) {
            continue;
          }
          HashBase = (UINT8 *) (UINTN) ImageAddress + Section->PointerToRawData;
          HashSize = (UINTN) Section->SizeOfRawData;

          SHA1Update(&Sha1Ctx,
                        		HashBase,
                        		 (u32)HashSize
                        	   );

          SumOfBytesHashed += HashSize;
      }


  //
  // 16.	If the file size is greater than SUM_OF_BYTES_HASHED, there is extra
  //      data in the file that needs to be added to the hash. This data begins 
  //      at file offset SUM_OF_BYTES_HASHED and its length is:
  //             FileSize  -  (CertDirectory->Size)
  //
  if (ImageSize > SumOfBytesHashed) {
      HashBase = (UINT8 *) (UINTN) ImageAddress + SumOfBytesHashed;
      if (Magic == EFI_IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        //
        // Use PE32 offset
        //
        HashSize = (UINTN)(ImageSize -
                   Hdr.Pe32->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY].Size -
                   SumOfBytesHashed);
      } else {
        //
        // Use PE32+ offset
        //
        HashSize = (UINTN)(ImageSize -
                   Hdr.Pe32Plus->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_SECURITY].Size -
                   SumOfBytesHashed);
      }

       SHA1Update(&Sha1Ctx,
                    		HashBase,
                    		 (u32)HashSize
                    	   );
    }


  //
  // 17.	Finalize the SHA hash.
  //
  SHA1Final((unsigned char *)&Sha1Digest.digest,&Sha1Ctx );
  gBS->CopyMem (&TcgEvent->Digest.digest,Sha1Digest.digest, sizeof (TcgEvent->Digest.digest));

  //
  // HashLogExtendEvent 
  //
  
  //hash has been generated so extend it
  TpmProtocol->Init(TpmProtocol);

  Status = TcgDxeCommonExtend (
			 (void *)TcgProtocol,
			 TcgEvent->PCRIndex,
			 &TcgEvent->Digest,
             &Sha1Digest
			);

  TpmProtocol->Close(TpmProtocol);

  if (!EFI_ERROR (Status)) {  

 TcgEventlog  = AllocateZeroPool (EventSize + sizeof (TCG_PCR_EVENT));

 //Now log the event
    TcgEventlog->PCRIndex  = TcgEvent->PCRIndex;
    TcgEventlog->EventType = TcgEvent->EventType;
    TcgEventlog->EventSize = TcgEvent->EventSize;
    gBS->CopyMem(&TcgEventlog->Digest, &TcgEvent->Digest, sizeof(TCG_DIGEST));
    gBS->CopyMem(&TcgEventlog->Event, ImageLoad, TcgEvent->EventSize);
    Status = TcgProtocol->LogEvent(TcgProtocol, TcgEventlog, &EventNumber,0x01);
  }

//  PERF_END(0,L"MeasurePeImg",NULL,0);

#if (defined(TCGMeasureSecureBootVariables) && (TCGMeasureSecureBootVariables != 0))
  FindandMeasureSecureBootCertificate();
#endif

  TpmProtocol->Close(TpmProtocol);

  Finish:
  if (TcgEventlog != NULL )
  {
    gBS->FreePool( TcgEventlog );
  }

  if (TcgEvent!= NULL )
  {
    gBS->FreePool( TcgEvent );
  }

  if(SectionHeader!=NULL)
 {
     gBS->FreePool( SectionHeader );
 }

  return Status;
}


EFI_STATUS
EFIAPI
TcgMeasureGptTable (
  IN      EFI_DEVICE_PATH_PROTOCOL  *DevicePath
  )
{
 
  EFI_STATUS                        Status;
  EFI_BLOCK_IO_PROTOCOL             *BlockIo;
  EFI_DISK_IO_PROTOCOL              *DiskIo;
  EFI_PARTITION_TABLE_HEADER    *PrimaryHeader;
  EFI_PARTITION_ENTRY           *PartitionEntry;
  UINT8                             *EntryPtr;
  UINTN                             NumberOfPartition;
  UINT32                            Index;
  TCG_PCR_EVENT                     *TcgEvent;
  EFI_GPT_DATA                  *GptData;
  EFI_GUID                          NullGuid = EFI_NULL_GUID;
  EFI_HANDLE                        Handle;
  EFI_TCG_PROTOCOL			        *TcgProtocol;
  EFI_TPM_DEVICE_PROTOCOL			*TpmProtocol;
  UINT32                            evNum;
  EFI_PHYSICAL_ADDRESS              Last;
  UINTN                             GptIndex;

//AMI_OVERRIDE_TXE0000_START : Support FTPM
  return EFI_SUCCESS; //EIP226550
//AMI_OVERRIDE_TXE0000_END
  if(AutoSupportType()){
    return EFI_SUCCESS;
  }

  Status = gBS->LocateDevicePath (
                          &gEfiDiskIoProtocolGuid,
                          &DevicePath,
                          &Handle
                          );

  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }
  Status = gBS->HandleProtocol (Handle, &gEfiBlockIoProtocolGuid, &BlockIo);
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }
  Status = gBS->HandleProtocol (Handle, &gEfiDiskIoProtocolGuid, &DiskIo);


  Status = gBS->LocateProtocol (&gEfiTcgProtocolGuid,  NULL, &TcgProtocol);
  if (EFI_ERROR (Status)) {
      return Status;
   }

  Status = gBS->LocateProtocol (&gEfiTpmDeviceProtocolGuid,  NULL, &TpmProtocol );
  if (EFI_ERROR (Status)) {
      return Status;
   }

  TpmProtocol ->Init( TpmProtocol );

  //
  // Read the EFI Partition Table Header
  //  

  Status = gBS->AllocatePool( EfiBootServicesData,
                              BlockIo->Media->BlockSize,
                             &PrimaryHeader );

  if (PrimaryHeader == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }  

  Status = DiskIo->ReadDisk (
                     DiskIo,
                     BlockIo->Media->MediaId,
                     1 * BlockIo->Media->BlockSize,
                     BlockIo->Media->BlockSize,
                     (UINT8 *)PrimaryHeader
                     );

  if (EFI_ERROR (Status)) {
    DEBUG ((-1, "Failed to Read Partition Table Header!\n"));
    gBS->FreePool (PrimaryHeader);
    return EFI_DEVICE_ERROR;
  }  
  //
  // Read the partition entry.
  //
  Status = gBS->AllocatePool( EfiBootServicesData,
                              PrimaryHeader->NumberOfPartitionEntries * PrimaryHeader->SizeOfPartitionEntry,
                              &EntryPtr );

  if (EntryPtr == NULL) {
    gBS->FreePool (PrimaryHeader);
    return EFI_OUT_OF_RESOURCES;
  }

  Status = DiskIo->ReadDisk (
                     DiskIo,
                     BlockIo->Media->MediaId,
                     PrimaryHeader->PartitionEntryLBA * BlockIo->Media->BlockSize,
                     PrimaryHeader->NumberOfPartitionEntries * PrimaryHeader->SizeOfPartitionEntry,
                     EntryPtr);

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
    if (CompareMem(&PartitionEntry->PartitionTypeGUID, &NullGuid, sizeof(EFI_GUID))) {
      NumberOfPartition++;  
    }
    PartitionEntry++;
  }   
  //
  // Parepare Data for Measurement
  //  

  //allocate memory for TCG event
  Status = gBS->AllocatePool( EfiBootServicesData,
                              sizeof(TCG_PCR_EVENT_HDR) + \
                             (UINT32)(sizeof (EFI_PARTITION_TABLE_HEADER) + sizeof(UINTN)\
                             + (NumberOfPartition * PrimaryHeader->SizeOfPartitionEntry)),
                             &TcgEvent );

  TcgEvent->PCRIndex   = 5;
  TcgEvent->EventType  = EV_EFI_GPT_EVENT;
  TcgEvent->EventSize  =  (UINT32)(sizeof (EFI_PARTITION_TABLE_HEADER) + sizeof(UINTN)\
                             + (NumberOfPartition * PrimaryHeader->SizeOfPartitionEntry));


  Status = gBS->AllocatePool( EfiBootServicesData,
                              TcgEvent->EventSize,
                              &GptData );
  if (GptData == NULL) {
    gBS->FreePool (PrimaryHeader);
    gBS->FreePool (EntryPtr);
    return EFI_OUT_OF_RESOURCES;
  }  

  SetMem(GptData, TcgEvent->EventSize, 0);
  //
  // Copy the EFI_PARTITION_TABLE_HEADER and NumberOfPartition
  //  
  gBS->CopyMem ((UINT8 *)GptData, (UINT8*)PrimaryHeader, sizeof (EFI_PARTITION_TABLE_HEADER));
  GptData->NumberOfPartitions = NumberOfPartition;
  //
  // Copy the valid partition entry
  //

  PartitionEntry = (EFI_PARTITION_ENTRY*)EntryPtr;
  GptIndex = 0;
  for (Index = 0; Index < PrimaryHeader->NumberOfPartitionEntries; Index++) {
    if (CompareMem (&PartitionEntry->PartitionTypeGUID, &NullGuid, sizeof(EFI_GUID))) {
      gBS->CopyMem (
        (UINT8 *)&GptData->Partitions + (GptIndex * sizeof (EFI_PARTITION_ENTRY)),
        (UINT8 *)PartitionEntry,
        sizeof (EFI_PARTITION_ENTRY)
        );
        GptIndex+=1;
    }
    PartitionEntry++;
  }
  //
  // Measure the GPT data
  // 

  gBS->CopyMem (TcgEvent->Event,
        		GptData,
        		TcgEvent->EventSize);

  TpmProtocol ->Init( TpmProtocol );

  Status = TcgProtocol->HashLogExtendEvent(
        TcgProtocol,
        (EFI_PHYSICAL_ADDRESS)GptData,
        TcgEvent->EventSize,
        TCG_ALG_SHA,
        TcgEvent,
        &evNum,
        &Last);


  TpmProtocol ->Close( TpmProtocol );

  gBS->FreePool (PrimaryHeader);
  gBS->FreePool (EntryPtr);
  gBS->FreePool (TcgEvent);
  gBS->FreePool (GptData);

  DEBUG(( -1,"GPT_EXIT"));
  return Status;
}



EFI_STATUS
TcgMeasureAction(
  IN      CHAR8				*String
  )
{

  TCG_PCR_EVENT		                *TcgEvent = NULL;
  EFI_PHYSICAL_ADDRESS				Last;
  EFI_TCG_PROTOCOL					*tcgSvc;
  UINT32							evNum;
  UINT32							Len;
  EFI_STATUS						Status;


  Status = gBS->LocateProtocol (
				&gEfiTcgProtocolGuid,
				NULL,
				&tcgSvc);

  ASSERT(!EFI_ERROR(Status));

  Len = (UINT32)AsciiStrLen(String);
  Status = gBS->AllocatePool (EfiBootServicesData,
        							_TPM_STRUCT_PARTIAL_SIZE (TCG_PCR_EVENT, Event) + 
        							Len,
        							&TcgEvent);

  ASSERT(!EFI_ERROR(Status));
  if(TcgEvent == NULL) return EFI_OUT_OF_RESOURCES;

  TcgEvent->PCRIndex 	= 5;
  TcgEvent->EventType 	= EV_EFI_ACTION;
  TcgEvent->EventSize 	= Len;

  gBS->CopyMem (TcgEvent->Event,
        		String,
        		Len);

   Status = tcgSvc->HashLogExtendEvent (
				tcgSvc,
				(EFI_PHYSICAL_ADDRESS)String,
				TcgEvent->EventSize,
				TCG_ALG_SHA,
				TcgEvent,
				&evNum,
				&Last);

  if(TcgEvent!=NULL)
  {
		gBS->FreePool (TcgEvent);
  }

  return Status;
}

UINT8 GetPlatformSupportType()
{
   return (AutoSupportType());
}


static EFI_TCG_PLATFORM_PROTOCOL  mTcgPlatformProtocol = {
  TcgMeasurePeImage,
  TcgMeasureAction,
  TcgMeasureGptTable
};


static AMI_TPM_SUPPORT_TYPE_PROTOCOL  mAmiTcgPlatformProtocol = {
  GetPlatformSupportType
};


EFI_STATUS EFIAPI TcmDxeEntry (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable);

TpmDxeEntry(
    IN EFI_HANDLE ImageHandle,
    IN EFI_SYSTEM_TABLE          * SystemTable);



EFI_STATUS
EFIAPI TcgDxeEntry (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable );


//**********************************************************************
//<AMI_PHDR_START>
// Procedure:   CommonTcgDxEntryPoint
//
// Description: Common entry point for Tcgdxe
//
// Input:       IN EFI_HANDLE        ImageHandle
//              IN EFI_SYSTEM_TABLE *SystemTable
//
// Output:
//
// Modified:
//
// Referrals:
//
// Notes:
//<AMI_PHDR_END>
//**********************************************************************
EFI_STATUS
EFIAPI CommonTcgDxEntryPoint(
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable )
{
    EFI_STATUS                      Status;
    TCG_PLATFORM_SETUP_PROTOCOL     *ProtocolInstance;
    EFI_GUID                        Policyguid = TCG_PLATFORM_SETUP_POLICY_GUID;
    BOOLEAN                         TpmInitError = FALSE;
#if TCG_LEGACY == 1
    BOOLEAN			                TpmLegBin = TRUE;
#else
    BOOLEAN			                TpmLegBin = FALSE;
#endif
    TCG_CONFIGURATION               Config;
    EFI_TCG_PROTOCOL			    *TcgProtocol;
    EFI_GUID                        TcgFirstbootGuid = AMI_TCG_RESETVAR_HOB_GUID;
    void                            ** DummyPtr;
    BOOLEAN                         *ResetAllTcgVar = NULL;
    UINT8                           *TcgDeviceTypeHob = NULL;
    EFI_GUID                        Tpm20Hobguid = TPM20_HOB_GUID;   
        
    Status = gBS->LocateProtocol (&Policyguid,  NULL, &ProtocolInstance);
    if (EFI_ERROR (Status)) {
      return Status;
    }
        
    CopyMem(&Config, &ProtocolInstance->ConfigFlags, sizeof(TCG_CONFIGURATION));  
    
    if(Config.DeviceType == 1){
        return EFI_SUCCESS;
    }
    
    Config.TcgSupportEnabled = 0;
    
    if((AutoSupportType()== TRUE) || (TpmLegBin == TRUE))
    {
        if( Config.TpmSupport != 0x00)
        {
#if TCG_LEGACY == 0
            Config.TcmSupport = TRUE;
            Status = TcmDxeEntry( ImageHandle, SystemTable );
#else
            Config.TcmSupport = FALSE; 
            Status = TpmDxeEntry( ImageHandle, SystemTable);
#endif 
           if(Status){
                Config.TpmHardware = TRUE; 
                TpmInitError = TRUE;
           }              
        }
    }else{
         Config.TcmSupport = FALSE;  
         Status = TpmDxeEntry( ImageHandle, SystemTable );
         if(Status){
            Config.TpmHardware = TRUE; 
            TpmInitError = TRUE;
         }else{
            Config.TpmHardware = FALSE; //negative logic False means present
        }              
    }

    if(TpmInitError){
         ProtocolInstance->UpdateStatusFlags(&Config, TRUE); 
         return Status;
    }
    
    Status = TcgDxeEntry( ImageHandle, SystemTable );
    if(EFI_ERROR(Status)){
             
        //if Support was enabled don't change TPM state
        if((ProtocolInstance->ConfigFlags.TcgSupportEnabled!=0 &&
            ProtocolInstance->ConfigFlags.TpmSupport == 0) || 
           (ProtocolInstance->ConfigFlags.TcgSupportEnabled!=0 &&
            ProtocolInstance->ConfigFlags.TcmSupport == 0))
        {  
            Config.TcgSupportEnabled = FALSE;
        }
        else{
            Config.TpmEnable        = 0;
            Config.TpmOperation     = 0;
            Config.TpmEnaDisable    = TRUE;
            Config.TpmActDeact      = TRUE;
            Config.TpmOwnedUnowned  = FALSE;
        }
 
        Config.PpiSetupSyncFlag = TRUE;
        ProtocolInstance->UpdateStatusFlags(&Config, TRUE); 
        return Status;
    }else{
        
        Status = gBS->LocateProtocol (&gEfiTcgProtocolGuid,  NULL, &TcgProtocol);
        if (EFI_ERROR (Status)) {
            Config.TcgSupportEnabled = FALSE;            
            ResetAllTcgVar = (UINT8*)LocateATcgHob(
                                gST->NumberOfTableEntries,
                                gST->ConfigurationTable,
                                &TcgFirstbootGuid);

            DummyPtr = &ResetAllTcgVar;
            if ( *DummyPtr != NULL )
            {
                if ( *ResetAllTcgVar == TRUE )
                {
                    Config.PpiSetupSyncFlag = TRUE;
                }
            }
            ProtocolInstance->UpdateStatusFlags(&Config, TRUE); 
            return Status;
        }
        
        Config.Tpm20Device = 0;        
        Config.TcgSupportEnabled = TRUE;        
        ProtocolInstance->UpdateStatusFlags(&Config, TRUE); 
    }

    Status = gBS->InstallProtocolInterface(
                &ImageHandle,
                &gEfiTcgPlatformProtocolGuid,
                EFI_NATIVE_INTERFACE,
                &mTcgPlatformProtocol);

    Status = gBS->InstallProtocolInterface(
                &ImageHandle,
                &gAmiTpmSupportTypeProtocolguid,
                EFI_NATIVE_INTERFACE,
                &mAmiTpmSupportTypeProtocol);

    return Status;
}

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093     **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
