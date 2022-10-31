//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/PEI Ram Boot/PeiRamBoot.c 12    8/08/12 4:25a Calvinchen $
//
// $Revision: 12 $
//
// $Date: 8/08/12 4:25a $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/PEI Ram Boot/PeiRamBoot.c $
//
// 12    8/08/12 4:25a Calvinchen
// 1. Changed for ReportFv2.c by Artem's suggestion.
// 2. Fixed System hangs if Memory Mapping Changed with warm boot.
//
// 11    7/18/12 3:38a Calvinchen
// [TAG]  		EIP94448
// [Category]  	Improvement
// [Description]  	Can not Save the DXE IPL Boot time,even add the PEI RAM
// Boot eModule.
//
// 10    2/23/12 9:06a Calvinchen
// Fixed building error.
//
// 9     2/23/12 6:35a Calvinchen
// [TAG]  		EIP82264
// [Category]  	Improvement
// [Description]  	Need to do cold boot to get the correct data in rom
// hole when changing data in rom hole.
// [Files]  		PeiRamBoot.sdl
// PeiRamBoot.mak
// PeiRamBoot.h
// PeiRamBoot.c
// PeiRamBootDxe.c
// PeiRamBoot.chm
// PeiRamBoot.cif
//
// 8     5/27/11 7:19a Calvinchen
// [TAG]  		EIP60320
// [Category]  	Bug Fix
// [Severity]  	Normal
// [Symptom]  	System hangs at CP 0x72 when SMM_THUNK_IN_CSM is 0 on
// Huronriver.
// [RootCause]  	InSmmFunction is NULL in InitSmmHandlerEx call.
// [Solution]  	Added dummy InSmmFunction.
// [Files]  		PeiRamBoot.c
//
// 7     4/22/11 1:27a Calvinchen
//
// 6     3/22/11 7:52a Calvinchen
// [TAG]  		EIP56322
// [Category]  	Bug Fix
// [Severity]  	Normal
// [Symptom]  	System hangs after changing TPM settings in SETUP.
// [RootCause]  	System Memory Mappings are changed with warm boot.
// [Solution]  	BIOS always go cold boot path if system memory mappings
// are changed
// [Files]  		PeiRamBoot.sdl
// PeiRamBoot.mak
// PeiRamBoot.dxs
// PeiRamBoot.h
// PeiRamBoot.c
// PeiRamBootHook.c
// PeiRamBootDxe.c
// PeiRamBootOfbd.c
// PeiRamBoot.chm
// PeiRamBoot.cif
//
// 5     2/11/11 3:16a Calvinchen
// Bug Fixed : System hangs after reflashed BIOS with warm reset if
// PEI_RAM_BOOT_S3_SUPPORT = 1 with fast warm boot support.
//
// 4     12/29/10 5:35a Calvinchen
// Bug Fixed : DMI structure not update in system memory if
// PEI_RAM_BOOT_S3_SUPPORT = 1 with fast warm boot support.
//
// 3     12/26/10 9:59p Calvinchen
// Bug Fixed:
// 1. Recovery not work if PEI_RAM_BOOT_S3_SUPPORT = 2.
// 2. S4 resume failed if PEI_RAM_BOOT_S3_SUPPORT = 1 with fast warm boot
// support.
//
// 2     12/14/10 2:25a Calvinchen
// Improvement :
// 1. Added an eLink "PeiRamBootList" for fast warm boot support
// (PEI_RAM_BOOT_S3_SUPPORT = 1). If system boots in warm boot state, BIOS
// directly boot to previous copied ROM image in RAM to save time of
// copying ROM.
// 2. Added "PEI_RAM_BOOT_S3_SUPPORT" = "2" for saving runtime memory, it
// only keep necessary PEIM FFS in runtime memory for S3 resume
// improvement.
//
// 1     10/27/10 2:48a Calvinchen
// Initial Check-in.
//
//
//**********************************************************************
//<AMI_FHDR_START>
//
// Name: PeiRamBoot.c
//
// Description: PEI RAM BOOT Pei driver.
//
//<AMI_FHDR_END>
//**********************************************************************
//----------------------------------------------------------------------------
// Includes
// Statements that include other files
#include <PEI.h>
#include <AmiPeiLib.h>
#include <ppi/ReadOnlyVariable2.h>
#include <PeiRamBootElinks.h>
#include <Library/AmiReportFVLib.h>
#include <ppi/RecoveryModule.h>
#include <RomLayout.h>
#include <Token.h>
#include <PeiRamBoot.h>
#include "PeiMain.h"
#if defined (SecureBoot_SUPPORT) && SecureBoot_SUPPORT == 1
#include <AmiCertificate.h>
#endif

//----------------------------------------------------------------------------
// Function Externs
extern
VOID
SwitchPeiServiceDataToRam (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN HOB_ROM_IMAGE                *HobRomImage
);
#if (PI_SPECIFICATION_VERSION >= 0x0001000A)
extern
BOOLEAN
IsPeimDispatched (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_FFS_FILE_HEADER          *FfsFile,
    IN UINTN                        Index
);
#else
extern
UINT64
GetDispatchedPeimBitMap (
    IN EFI_PEI_SERVICES             **PeiServices
);
#endif
EFI_STATUS GetRomLayout(
    IN  EFI_PEI_SERVICES **PeiServices,
    OUT ROM_AREA         **Layout
);

typedef BOOLEAN (PEI_RAM_BOOT_ELINK) (EFI_PEI_SERVICES **PeiServices);
extern PEI_RAM_BOOT_ELINK PEI_RAM_BOOT_LIST EndOfPeiRamBootList;
PEI_RAM_BOOT_ELINK* IsMrcColdBooteLink[] = {PEI_RAM_BOOT_LIST NULL};
//----------------------------------------------------------------------------
// Local prototypes
#define ROM_LAYOUT_FFS_GUID \
{ 0x0DCA793A, 0xEA96, 0x42d8, 0xBD, 0x7B, 0xDC, 0x7F, 0x68, 0x4E, 0x38, 0xC1 }
#define FID_FFS_FILE_NAME_GUID \
{ 0x3fd1d3a2, 0x99f7, 0x420b, 0xbc, 0x69, 0x8b, 0xb1, 0xd4, 0x92, 0xa3, 0x32 }
#define PKEY_FILE_GUID \
{ 0xCC0F8A3F, 0x3DEA,  0x4376, 0x96, 0x79, 0x54, 0x26, 0xba, 0x0a, 0x90, 0x7e }
#define KEK_FILE_GUID \
{ 0x9fe7de69, 0xaea, 0x470a, 0xb5, 0xa, 0x13, 0x98, 0x13, 0x64, 0x91, 0x89 }
#define DB_FILE_GUID \
{ 0xfbf95065, 0x427f, 0x47b3, 0x80, 0x77, 0xd1, 0x3c, 0x60, 0x71, 0x9, 0x98 }
#define DBX_FILE_GUID \
{ 0x9d7a05e9, 0xf740, 0x44c3, 0x85, 0x8b, 0x75, 0x58, 0x6a, 0x8f, 0x9c, 0x8e }

static EFI_GUID gHobRomImageGuid = ROM_IMAGE_MEMORY_HOB_GUID;
static EFI_GUID gEfiPeiEndOfPeiPhasePpiGuid = EFI_PEI_END_OF_PEI_PHASE_PPI_GUID;
static EFI_GUID gRomImageAddressGuid = ROM_IMAGE_ADDRESS_GUID;
static EFI_GUID gSmbiosFlashDataFfsGuid = SMBIOS_FLASH_DATA_FFS_GUID;
static EFI_GUID gRomCacheEnablePpiGuid = ROM_CACHE_ENABLE_PPI_GUID;
static EFI_GUID gLastFfsFileOverrideGuid = \
{0x45B9618F, 0xBAA1, 0x421B, { 0x94, 0xF0, 0xB9, 0xEB, 0xDD, 0x2B, 0xA1, 0x77 }};

EFI_STATUS
PeiRamBootMemoryReady (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *NullPpi
);

EFI_STATUS
PeiRamBootEndOfPei (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *NullPpi
);

#if defined PRESERVE_NESTED_FV_IN_MEM && PRESERVE_NESTED_FV_IN_MEM == 1
EFI_STATUS
PeiRamBootPreserveNestedFv (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *NullPpi
);

static EFI_PEI_NOTIFY_DESCRIPTOR PeiRamBootNestedFvNotify[] =
{
    {
        EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | \
        EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gEfiPeiEndOfPeiPhasePpiGuid,
        PeiRamBootPreserveNestedFv
    }
};
#endif

static EFI_PEI_NOTIFY_DESCRIPTOR PeiRamBootMemoryReadyNotify[] =
{
    {
        EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | \
        EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gEfiPeiMemoryDiscoveredPpiGuid, // gCacheInstallGuid/gPeiPermanentMemInstalled
        PeiRamBootMemoryReady
    }
};
static EFI_PEI_NOTIFY_DESCRIPTOR PeiRamBootEndOfPeiNotify[] =
{
//<EIP153486+> >>>
    {
        EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK,
        &gEfiPeiMemoryDiscoveredPpiGuid,
        PeiRamBootEndOfPei
    },
//<EIP153486+> <<<
    // For attribute with ROM_AREA_FV_PEI and ROM_AREA_FV_PEI_MEM
#if COPY_TO_RAM_WHILE_DISPATCH == 1
    {
        EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK,
        &gRomCacheEnablePpiGuid,
        PeiRamBootEndOfPei
    },
#endif
    // For attribute with ROM_AREA_FV_DXE only
    {
        EFI_PEI_PPI_DESCRIPTOR_NOTIFY_CALLBACK | \
        EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
        &gEfiPeiEndOfPeiPhasePpiGuid,
        PeiRamBootEndOfPei
    }
};
//----------------------------------------------------------------------------
// Local Variables
UINT32 FvBootBlocksList[] = {
    PEI_RAM_BOOT_FV_BOOTBLOCK_LIST
    FV_BB_BASE,
    { -1 }
};

EFI_GUID PreservedFfsGuid[] = {
    ROM_LAYOUT_FFS_GUID,
    FID_FFS_FILE_NAME_GUID,
#if !defined(SMBIOS_DMIEDIT_DATA_LOC) || SMBIOS_DMIEDIT_DATA_LOC == 0
    SMBIOS_FLASH_DATA_FFS_GUID,
#endif
#if PEI_RAM_BOOT_S3_SUPPORT == 2
#if defined (SecureBoot_SUPPORT) && SecureBoot_SUPPORT == 1
    PKEY_FILE_GUID,
    KEK_FILE_GUID,
    DB_FILE_GUID,
    DBX_FILE_GUID,
    PR_KEY_FFS_FILE_RAW_GUID,
    PR_KEY_FFS_FILE_SHA256_GUID,
#endif
#endif
    PEI_RAM_BOOT_FFS_GUID_LIST
    { 0 }
};

//----------------------------------------------------------------------------
// Function Definitions
#if PEI_RAM_BOOT_S3_SUPPORT == 1
#if WARM_BOOT_VERIFY_CHECKSUM
//<AMI_PHDR_START>
//**********************************************************************
//
// Procedure:	VerifyFvHeaderChecksum
//
// Description:
//  This function verifies the checksum of the firmware volume header
//
// Input:
//    IN EFI_FIRMWARE_VOLUME_HEADER *FvHeader - pointer to the beginning of an FV header
//
// Output:
//  TRUE  -  Checksum verification passed
//  FALSE -  Checksum verification failed
//
// Notes:
//
//**********************************************************************
//<AMI_PHDR_END>
BOOLEAN VerifyFvHeaderChecksum (
    IN EFI_FIRMWARE_VOLUME_HEADER *FvHeader)
{

    UINT32  Index, HeaderLength;
    UINT16  Checksum, *ptr;

    HeaderLength = FvHeader->HeaderLength;
    ptr = (UINT16 *)FvHeader;
    Checksum = 0;

    for (Index = 0; Index < HeaderLength / sizeof (UINT16); Index++) {
        Checksum = (UINT16)(Checksum + ptr[Index]);
    }
    if (Checksum == 0) return TRUE;
    return FALSE;
}


//<AMI_PHDR_START>
//**********************************************************************
//
// Procedure:	FileChecksum
//
// Description:
//  This function checks if it's a valid FFS file.
//
// Input:
//    IN EFI_FFS_FILE_HEADER* FfsHeader - pointer to the header of an FFS File
//
// Output:
//  TRUE - the checksum of the FFS file passed
//  FALSE - the checksum of the FFS file failed
//
// Notes:
//
//**********************************************************************
//<AMI_PHDR_END>
BOOLEAN
FileChecksum(
    IN EFI_FFS_FILE_HEADER* FfsHeader
)
{
    UINT32            Index, Length;
    UINT8             *ptr, FileChecksum;

    Length =  *(UINT32 *)FfsHeader->Size & 0x00FFFFFF;

    ptr = (UINT8 *)FfsHeader;
    FileChecksum = 0;
    for (Index = 0; Index < Length; Index++) {
        FileChecksum = (UINT8)(FileChecksum + ptr[Index]);
    }

    FileChecksum = FileChecksum - FfsHeader->State;
    if (FileChecksum == 0) return TRUE;
    return FALSE;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   VerifyFirmwareVolumeChecksum
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
VerifyFirmwareVolumeChecksum (
    IN EFI_PEI_SERVICES     **PeiServices,
    IN UINT8                *pFvBB,
    IN UINTN                FvLength
)
{
    EFI_FFS_FILE_HEADER    *pFfsFile = NULL;
    EFI_STATUS              Status;

    if (VerifyFvHeaderChecksum((EFI_FIRMWARE_VOLUME_HEADER*)pFvBB) != TRUE)
        return FALSE;
    while ( TRUE ) {
        Status = (*PeiServices)->FfsFindNextFile (  PeiServices, \
                 EFI_FV_FILETYPE_ALL, \
                 (EFI_FIRMWARE_VOLUME_HEADER*)pFvBB, \
                 &pFfsFile );
        if ( Status == EFI_NOT_FOUND ) break;
        if (pFfsFile->Attributes & FFS_ATTRIB_CHECKSUM) {
            if (FileChecksum(pFfsFile) != TRUE) return FALSE;
        }
    }
    return TRUE;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IsFvFfsChecksumGood
//
// Description: This procedure checks whether the PEIM is relating to recovery
//              module.
//
// Input:       EFI_PEI_SERVICES**  - PeiServices
//              EFI_FFS_FILE_HEADER* - FfsHdr
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
IsFvFfsChecksumGood (
    IN EFI_PEI_SERVICES     **PeiServices,
    IN HOB_ROM_IMAGE        *HobRomImage
)
{
    UINT8       i;
    for (i = 0; i < HobRomImage->NumOfFv; i++) {
        if (!VerifyFirmwareVolumeChecksum ( \
                                            PeiServices,\
                                            (UINT8*)(HobRomImage->FvInfo[i].MemAddress), \
                                            (UINTN)(HobRomImage->FvInfo[i].FvLength))) {
            return FALSE;
        }
    }
    return TRUE;
}
#endif
#endif
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IsBootFirmwareVolume
//
// Description:
//
// Input:       EFI_PEI_SERVICES**      - PeiServices
//              EFI_PHYSICAL_ADDRESS    - Address
//
// Output:      None.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
IsBootBlockFirmwareVolumes (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PHYSICAL_ADDRESS         Address
)
{
    UINT8           i;
#if (PI_SPECIFICATION_VERSION >= 0x0001000A)
    PEI_CORE_INSTANCE           *Private = NULL;
    Private = PEI_CORE_INSTANCE_FROM_PS_THIS(PeiServices);
    if (Private->Fv[0].FvHeader == (EFI_FIRMWARE_VOLUME_HEADER*)Address) 
        return TRUE;
#endif
    for (i = 0; FvBootBlocksList[i] != -1; i++)
        if (Address == (EFI_PHYSICAL_ADDRESS)FvBootBlocksList[i]) return TRUE;
    return FALSE;    
}
#if PEI_RAM_BOOT_S3_SUPPORT == 2
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IsRecoveryFfsFile
//
// Description: This procedure checks whether the PEIM is relating to recovery
//              module.
//
// Input:       EFI_PEI_SERVICES**  - PeiServices
//              EFI_FFS_FILE_HEADER* - FfsHdr
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN
IsRecoveryFfsFile (
    IN EFI_PEI_SERVICES         **PeiServices,
    IN EFI_FFS_FILE_HEADER      *FfsHdr
)
{
    EFI_COMMON_SECTION_HEADER   *Section = NULL;
    UINT32                      SectionSize, i;

    Section = (EFI_COMMON_SECTION_HEADER *)(FfsHdr + 1);
    if (Section->Type == EFI_SECTION_PEI_DEPEX) {
        SectionSize = (*(UINT32*)Section->Size & 0x00FFFFFF);
        if ((SectionSize - sizeof(EFI_COMMON_SECTION_HEADER)) < \
                sizeof(EFI_GUID)) return FALSE;
        for (i = sizeof(EFI_COMMON_SECTION_HEADER);
                i < (SectionSize - sizeof(EFI_GUID)); i++) {
            if (!guidcmp((UINT8*)Section + i, &gEfiPeiBootInRecoveryModePpiGuid)) //gEfiPeiRecoveryModePpiGuid
                return TRUE;
            if (!guidcmp((UINT8*)Section + i, &gEfiPeiRecoveryModulePpiGuid))
                return TRUE;
        }
    }
    return FALSE;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   InvalidatePeiModules
//
// Description: This procedure copy in used ROM data to memroy.
//
// Input:       EFI_PEI_SERVICES**  - PeiServices
//              EFI_PHYSICAL_ADDRESS - Buffer
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
InvalidatePeiModules (
    IN EFI_PEI_SERVICES         **PeiServices,
    IN EFI_PHYSICAL_ADDRESS     Buffer,
    IN HOB_ROM_IMAGE            *HobRomImage,
    IN UINT8                    Index
)
{
    UINT8           i, j, HeaderSum, SavedFileSum, SavedFileState;
    UINT32          PeimAddress;
    for (i = 0; i < HobRomImage->NumOfPeim; i++) {
        PeimAddress = HobRomImage->PeimFfsInfo[i].PeimAddress;
        if ((PeimAddress > HobRomImage->FvInfo[Index].FvAddress) && \
                (PeimAddress < HobRomImage->FvInfo[Index].FvAddress + \
                 HobRomImage->FvInfo[Index].FvLength)) {
            PeimAddress -= HobRomImage->FvInfo[Index].FvAddress;
            // Change Type of FFS File from PEIM to RAW.
//-            ((EFI_FFS_FILE_HEADER*)(Buffer + PeimAddress))->Type = EFI_FV_FILETYPE_RAW;
            ((EFI_FFS_FILE_HEADER*)(Buffer + PeimAddress))->Type = EFI_FV_FILETYPE_DRIVER;
            // Following update the Header Checksum of FFS File.
            SavedFileSum = \
                           ((EFI_FFS_FILE_HEADER*)(Buffer + PeimAddress))->IntegrityCheck.Checksum.File;
            SavedFileState = \
                             ((EFI_FFS_FILE_HEADER*)(Buffer + PeimAddress))->State;
#if PI_SPECIFICATION_VERSION < 0x00010000
            ((EFI_FFS_FILE_HEADER*)(Buffer + PeimAddress))->IntegrityCheck.TailReference = 0;
#else
            ((EFI_FFS_FILE_HEADER*)(Buffer + PeimAddress))->IntegrityCheck.Checksum16 = 0;
#endif
            ((EFI_FFS_FILE_HEADER*)(Buffer + PeimAddress))->State = 0;
            // Calculate
            for (HeaderSum = 0, j = 0; j < sizeof(EFI_FFS_FILE_HEADER); j++)
                HeaderSum += *(UINT8*)(Buffer + PeimAddress + j);
            ((EFI_FFS_FILE_HEADER*)(Buffer + PeimAddress))->IntegrityCheck.Checksum.Header = (UINT8)(~HeaderSum + 1);
            ((EFI_FFS_FILE_HEADER*)(Buffer + PeimAddress))->IntegrityCheck.Checksum.File = SavedFileSum;
            ((EFI_FFS_FILE_HEADER*)(Buffer + PeimAddress))->State = SavedFileState;
        }
    }
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CopyMininumRomImage2Buffer
//
// Description: This procedure copy in used ROM data to memroy.
//
// Input:       EFI_PEI_SERVICES**  - PeiServices
//              EFI_PHYSICAL_ADDRESS - Buffer
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
CopyMininumRomImage2Buffer (
    IN EFI_PEI_SERVICES         **PeiServices,
    IN HOB_ROM_IMAGE            *HobRomImage
)
{
    UINT8                   i, j;
    UINTN                   NumOfPages;
    EFI_PHYSICAL_ADDRESS    Buffer = 0, PtrAddress;
    EFI_STATUS              Status = EFI_SUCCESS;
    EFI_PHYSICAL_ADDRESS    ExtFvHeader;
    UINT32                  ExtFvHeaderOffset;
    UINT32                  ExtFvHeaderLength;
    UINT32                  CopyLength;

    for (i = 0; i < HobRomImage->NumOfFv; i++) {
        // Only copy not dispatched yet PEIM to RAM.
        if (HobRomImage->FvInfo[i].FvAddress == FV_BB_BASE) {
            for (j = 0, NumOfPages = 0; j < HobRomImage->NumOfPeim; j++) {
                NumOfPages += HobRomImage->PeimFfsInfo[j].PeimLength;
            }
            // 0x1400 = 1)4k aligned, 2)NULL bytes of FV, 3)HOB_ROM_IMAGE struct
            NumOfPages = (NumOfPages + 0x1400) & 0xfffff000;
            Status = (*PeiServices)->AllocatePages ( PeiServices, \
                     EfiRuntimeServicesData, \
                     NumOfPages >> 12, \
                     &Buffer);
            if (EFI_ERROR(Status)) return EFI_UNSUPPORTED;
            HobRomImage->FvInfo[i].NumOfPages = NumOfPages >> 12;
            HobRomImage->FvInfo[i].IsBootFv = TRUE;
            HobRomImage->StolenHobMemory = (Buffer + NumOfPages - 0x200);
            // Copy Firmware Volume header.
            PtrAddress = HobRomImage->FvInfo[i].FvAddress;
            //Check EFI_FIRMWARE_VOLUME_EXT_HEADER exist or not.
            //If exist, copy the whole firmware volume header.
            CopyLength = ((EFI_FIRMWARE_VOLUME_HEADER*)PtrAddress)->HeaderLength;
            if( ((EFI_FIRMWARE_VOLUME_HEADER*)PtrAddress)->ExtHeaderOffset )
            {
                ExtFvHeaderOffset = \
                    ((EFI_FIRMWARE_VOLUME_HEADER*)PtrAddress)->ExtHeaderOffset;
                ExtFvHeader = PtrAddress + ExtFvHeaderOffset;
                ExtFvHeaderLength = \
                    ((EFI_FIRMWARE_VOLUME_EXT_HEADER*)ExtFvHeader)->ExtHeaderSize;
                CopyLength = ExtFvHeaderOffset + ExtFvHeaderLength;
            }
            (*PeiServices)->CopyMem ( \
                                      (UINT8*)Buffer, \
                                      (UINT8*)PtrAddress, \
                                      CopyLength);
            PtrAddress = (UINT32)Buffer + CopyLength;
            //Align on 8 bytes
            (*PeiServices)->SetMem ((UINT8*)PtrAddress, 0x8, 0xff);
            PtrAddress = ((UINT32)PtrAddress + 7) & 0xfffffff8;
            // Copy not dispatched PEIM to ram.
            for (j = 0; j < HobRomImage->NumOfPeim; j++) {
                (*PeiServices)->CopyMem ( \
                                          (UINT8*)PtrAddress, \
                                          (UINT8*)(HobRomImage->PeimFfsInfo[j].PeimAddress), \
                                          HobRomImage->PeimFfsInfo[j].PeimLength );
                PtrAddress += HobRomImage->PeimFfsInfo[j].PeimLength;
                (*PeiServices)->SetMem ((UINT8*)PtrAddress, 128, 0xff);
                PtrAddress = ((UINT32)PtrAddress + 7) & 0xfffffff8;
            }
        } else {
            if (HobRomImage->FvInfo[i].FvLength != \
                    HobRomImage->FvInfo[i].UsedBytes) {
                NumOfPages = \
                             ((HobRomImage->FvInfo[i].UsedBytes + 0x1000) & 0xffffff000);
            } else NumOfPages = HobRomImage->FvInfo[i].FvLength;
            HobRomImage->FvInfo[i].NumOfPages = NumOfPages >> 12;
            HobRomImage->FvInfo[i].IsBootFv = FALSE;
            Status = (*PeiServices)->AllocatePages ( PeiServices, \
                     EfiBootServicesData, \
                     NumOfPages >> 12, \
                     &Buffer );
            if (EFI_ERROR(Status)) return EFI_UNSUPPORTED;
            (*PeiServices)->CopyMem ( \
                                      (UINT8*)Buffer, \
                                      (UINT8*)HobRomImage->FvInfo[i].FvAddress, \
                                      HobRomImage->FvInfo[i].UsedBytes);
            InvalidatePeiModules(PeiServices, Buffer, HobRomImage, i);
        }
        HobRomImage->FvInfo[i].MemAddress = (UINT32)Buffer;
        HobRomImage->FvInfo[i].FvMemReady = TRUE;
    }
    // Save HOB Rom image to stolen memroy buffer.
    (*PeiServices)->CopyMem ( (UINT8*)HobRomImage->StolenHobMemory, \
                              (UINT8*)HobRomImage, \
                              sizeof(HOB_ROM_IMAGE));
    return EFI_SUCCESS;
}
#else
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   PrepareForCopyRomToRam
//
// Description: This procedure allocate memroy buffer for copying rom to ram.
//
// Input:       EFI_PEI_SERVICES**  - PeiServices
//              HOB_ROM_IMAGE       - *HobRomImage
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
PrepareForCopyRomToRam (
    IN EFI_PEI_SERVICES         **PeiServices,
    IN HOB_ROM_IMAGE            *HobRomImage
)
{
    UINT8                   i;
    EFI_STATUS              Status = EFI_SUCCESS;
    UINTN                   NumOfPages;
    EFI_PHYSICAL_ADDRESS    Buffer = 0;

    for (i = 0; i < HobRomImage->NumOfFv; i++) {
        NumOfPages = HobRomImage->FvInfo[i].UsedBytes;
        if (HobRomImage->FvInfo[i].FvLength !=
                HobRomImage->FvInfo[i].UsedBytes) {
#if SAVE_ENTIRE_FV_IN_MEM == 0
            // 0x1200 = 1)4k aligned, 2)NULL bytes of FV
            NumOfPages = HobRomImage->FvInfo[i].UsedBytes + 0x1200;
#else
            NumOfPages = HobRomImage->FvInfo[i].FvLength;
#endif
        }
        Status = (*PeiServices)->AllocatePages ( PeiServices,
#if PEI_RAM_BOOT_S3_SUPPORT == 1
                 EfiRuntimeServicesData,
#else
                 EfiBootServicesData,
#endif
                 NumOfPages >> 12,
                 &Buffer);
        if (EFI_ERROR(Status)) return EFI_UNSUPPORTED;
        // If HobValid is "TRUE", it means FV memories aren't destroyed, just
        // reserve the necessary memory space for FV then exit.
        if (HobRomImage->HobValid == TRUE) continue;
        HobRomImage->FvInfo[i].NumOfPages = NumOfPages >> 12;
        HobRomImage->FvInfo[i].MemAddress = (UINT32)Buffer;
        HobRomImage->FvInfo[i].FvMemReady = FALSE;
    }
    return EFI_SUCCESS;
}
#if (PI_SPECIFICATION_VERSION >= 0x0001000A) && (OPTIMIZE_BOOT_FV_COPY == 1)
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   MinimumizeBootFv
//
// Description: This routine copy only undispatched PEIM and Preserved FFS Guid
//              to memory and FFS Header only if dispatched PEIM.
//
// Input:       EFI_PEI_SERVICES**  - PeiServices
//              HOB_ROM_IMAGE*      - HobRomImage,
//              UINT8               - Index
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
MinimumizeBootFv (
    IN EFI_PEI_SERVICES         **PeiServices,
    IN HOB_ROM_IMAGE            *HobRomImage,
    IN UINT8                    Index
)
{
    UINTN       k = 0, n = 0, FfsLength;
    UINT8       *p = (UINT8*)HobRomImage->FvInfo[Index].MemAddress;
    UINT8       *q = (UINT8*)HobRomImage->FvInfo[Index].FvAddress;
    EFI_FFS_FILE_HEADER     *FfsFile = NULL;
    EFI_FFS_FILE_HEADER     *PadFfsHeader = NULL;
    EFI_GUID                *pGuid = NULL;
    EFI_STATUS              Status = EFI_SUCCESS;
    EFI_PHYSICAL_ADDRESS    ExtFvHeader;
    UINT32                  ExtFvHeaderOffset;
    UINT32                  ExtFvHeaderLength;
    UINT32                  CopyLength;

    // 1. Copy Firmware Volume Header to Memory buffer.
    CopyLength = ((EFI_FIRMWARE_VOLUME_HEADER*)q)->HeaderLength;
    //If ExtHeader exist
    if( ((EFI_FIRMWARE_VOLUME_HEADER*)q)->ExtHeaderOffset )
    {
        ExtFvHeaderOffset = \
                    ((EFI_FIRMWARE_VOLUME_HEADER*)q)->ExtHeaderOffset;
        ExtFvHeader = (EFI_PHYSICAL_ADDRESS)(q + ExtFvHeaderOffset);
        ExtFvHeaderLength = \
                    ((EFI_FIRMWARE_VOLUME_EXT_HEADER*)ExtFvHeader)->ExtHeaderSize;
        CopyLength = ExtFvHeaderOffset + ExtFvHeaderLength;
    }
    (*PeiServices)->CopyMem ( p, q, CopyLength);
    p += CopyLength;
    //Align on 8 bytes
    (*PeiServices)->SetMem ((UINT8*)p, 0x8, 0xff);
    p = (UINT8*)(((UINT32)p + 7) & 0xfffffff8);
    do {
        Status = (*PeiServices)->FfsFindNextFile (PeiServices, \
              EFI_FV_FILETYPE_ALL, (EFI_FIRMWARE_VOLUME_HEADER*)q, &FfsFile );
        if (EFI_ERROR(Status)) break;
        // calculate file alignment (Align on 8 bytes).
        FfsLength = *(UINT32*)FfsFile->Size & 0xffffff;
        FfsLength = (FfsLength + 7) & 0xfffffff8;
        // Copy undispatched PEIMs and FFS File Header only if dispatched PEIM 
        if ((FfsFile->Type == EFI_FV_FILETYPE_PEI_CORE) || \
            ((FfsFile->Type == EFI_FV_FILETYPE_PEIM) && \
            (!IsPeimDispatched(PeiServices, FfsFile, k)))) n = FfsLength;
        else {
            n = sizeof(EFI_FFS_FILE_HEADER);
            // Copy RAW and FREEFORM FFS file..
            if ((FfsFile->Type == EFI_FV_FILETYPE_FREEFORM) || \
                (FfsFile->Type == EFI_FV_FILETYPE_RAW)) n = FfsLength;
        }
        // Increase PEIM index meeting PEI Core Private Data of PI 1.2  
        if (FfsFile->Type == EFI_FV_FILETYPE_PEIM) k++;    
        (*PeiServices)->CopyMem (p, (UINT8*)FfsFile, n);
//<EIP141743+> >>>
        if( (FfsFile->Type == EFI_FV_FILETYPE_SECURITY_CORE) ||
            ((FfsFile->Type == EFI_FV_FILETYPE_PEIM) &&
		    IsPeimDispatched(PeiServices, FfsFile, k)) )
        {
            if( ((EFI_FFS_FILE_HEADER*)p)->Attributes & FFS_ATTRIB_CHECKSUM )
            {
                ((EFI_FFS_FILE_HEADER*)p)->Attributes &= (~FFS_ATTRIB_CHECKSUM);
                ((EFI_FFS_FILE_HEADER*)p)->IntegrityCheck.Checksum.Header += FFS_ATTRIB_CHECKSUM;
            }
            ((EFI_FFS_FILE_HEADER*)p)->IntegrityCheck.Checksum.File = 0xAA;
        }
//<EIP141743+> <<<
        p = p + FfsLength;
        if(FfsFile->Type == EFI_FV_FILETYPE_SECURITY_CORE) continue;
        // Save PAD_FFS_FILE Header for FvCheck of NotifyFwVolBlock procedure.
        PadFfsHeader = (EFI_FFS_FILE_HEADER*)((UINT8*)FfsFile + FfsLength);
        if( PadFfsHeader->Type == EFI_FV_FILETYPE_FFS_PAD )
        {
            FfsLength = *(UINT32*)(PadFfsHeader->Size) & 0xffffff;
            FfsLength = (FfsLength + 7) & 0xfffffff8;
            n = sizeof(EFI_FFS_FILE_HEADER);
            (*PeiServices)->CopyMem (p, (UINT8*)PadFfsHeader, n);
            p += FfsLength;
        }
    } while(!EFI_ERROR(Status));
}
#endif  // #if (PI_SPECIFICATION_VERSION >= 0x0001000A) && (OPTIMIZE_BOOT_FV_COPY == 1)
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CopyFirmwareVolumesToRam
//
// Description: This procedure copy in used Firmware Volume to memroy.
//
// Input:       EFI_PEI_SERVICES**  - PeiServices
//              HOB_ROM_IMAGE*      - HobRomImage,
//              BOOLEAN             - IsEndOfPei
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
CopyFirmwareVolumesToRam (
    IN EFI_PEI_SERVICES         **PeiServices,
    IN HOB_ROM_IMAGE            *HobRomImage,
    IN BOOLEAN                  IsEndOfPei
)
{
    UINT8                   i;
    EFI_STATUS              Status = EFI_SUCCESS;

    for (i = 0; i < HobRomImage->NumOfFv; i++) {
        if ((!IsEndOfPei) && (!HobRomImage->FvInfo[i].IsBootFv)) continue;
        else if (IsEndOfPei && HobRomImage->FvInfo[i].IsBootFv) continue;
        if (HobRomImage->FvInfo[i].FvMemReady == TRUE) continue;
        HobRomImage->FvInfo[i].FvMemReady = TRUE;
        if (HobRomImage->HobValid) continue;
#if (PI_SPECIFICATION_VERSION >= 0x0001000A) && (OPTIMIZE_BOOT_FV_COPY == 1)
        if (HobRomImage->FvInfo[i].IsBootFv == TRUE) {
            MinimumizeBootFv (PeiServices, HobRomImage, i);
        }
        else
#endif  // #if (OPTIMIZE_BOOT_FV_COPY == 1)
        {    
            (*PeiServices)->CopyMem ((UINT8*)HobRomImage->FvInfo[i].MemAddress, \
                                     (UINT8*)HobRomImage->FvInfo[i].FvAddress, \
                                     HobRomImage->FvInfo[i].UsedBytes);
        }
        if (HobRomImage->FvInfo[i].UsedBytes != HobRomImage->FvInfo[i].FvLength) 
        {
            UINT32          NumofNullBytes;
#if SAVE_ENTIRE_FV_IN_MEM == 1
            // Fill Null bytes after Used bytes
            NumofNullBytes = HobRomImage->FvInfo[i].FvLength - \
                                    HobRomImage->FvInfo[i].UsedBytes;
#else
            // Fill Max 512 Null bytes after Used bytes
            NumofNullBytes = (HobRomImage->FvInfo[i].NumOfPages << 12) - \
                                    HobRomImage->FvInfo[i].UsedBytes;
            if (NumofNullBytes > 512) NumofNullBytes = 512;
#endif  // #if SAVE_ENTIRE_FV_IN_MEM == 1
            (*PeiServices)->SetMem (
                (UINT8*)(HobRomImage->FvInfo[i].MemAddress + \
                                    HobRomImage->FvInfo[i].UsedBytes), \
                NumofNullBytes, FLASH_EMPTY_BYTE );
        }                         
    }
#if PEI_RAM_BOOT_S3_SUPPORT == 1
    if ((IsEndOfPei) && (!HobRomImage->HobValid)) {
        (*PeiServices)->CopyMem ( (UINT8*)HobRomImage->StolenHobMemory, \
                                  (UINT8*)HobRomImage, \
                                  sizeof(HOB_ROM_IMAGE));
    }
#endif
    return EFI_SUCCESS;
}
#endif
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   RestoreHobInfoFromRam
//
// Description: This procedure restores the Hob Informations from stolen
//              memory.
//
// Input:       EFI_PEI_SERVICES**  - PeiServices
//              HOB_ROM_IMAGE*      - HobRomImage,
//              HOB_ROM_IMAGE*      - HobInRam,
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
RestoreHobInfoFromRam (
    IN EFI_PEI_SERVICES         **PeiServices,
    IN HOB_ROM_IMAGE            *HobRomImage,
    IN HOB_ROM_IMAGE            *HobInRam
)
{
    UINT8           i;

    HobRomImage->NumOfFv = HobInRam->NumOfFv;
    HobRomImage->StolenHobMemory = HobInRam->StolenHobMemory;
    HobRomImage->SmbiosFlashData = HobInRam->SmbiosFlashData;
    for (i = 0; i < HobInRam->NumOfFv; i++) {
        HobRomImage->FvInfo[i].FvAddress = HobInRam->FvInfo[i].FvAddress;
        HobRomImage->FvInfo[i].FvLength = HobInRam->FvInfo[i].FvLength;
        HobRomImage->FvInfo[i].UsedBytes = HobInRam->FvInfo[i].UsedBytes;
        HobRomImage->FvInfo[i].MemAddress = HobInRam->FvInfo[i].MemAddress;
        HobRomImage->FvInfo[i].NumOfPages = HobInRam->FvInfo[i].NumOfPages;
        HobRomImage->FvInfo[i].IsBootFv = HobInRam->FvInfo[i].IsBootFv;
        HobRomImage->FvInfo[i].FvMemReady = FALSE;
    }
#if defined PRESERVE_NESTED_FV_IN_MEM && PRESERVE_NESTED_FV_IN_MEM == 1
    HobRomImage->NumOfNestedFv = HobInRam->NumOfNestedFv;
    HobRomImage->NestedFvValid = HobInRam->NestedFvValid;
    for (i = 0; i < HobInRam->NumOfNestedFv; i++) {
        HobRomImage->NestedFvInfo[i].FvAddress = HobInRam->NestedFvInfo[i].FvAddress;
        HobRomImage->NestedFvInfo[i].FvLength = HobInRam->NestedFvInfo[i].FvLength;
        HobRomImage->NestedFvInfo[i].MemAddress = HobInRam->NestedFvInfo[i].MemAddress;
    }
#endif
#if PEI_RAM_BOOT_S3_SUPPORT == 1
    HobRomImage->NumOfPeim = HobInRam->NumOfPeim;
    for (i = 0; i < HobInRam->NumOfPeim; i++) {
        UINT32      j, offset;
        HobRomImage->RsvdFfsInfo[i].FfsAddress = \
                HobInRam->RsvdFfsInfo[i].FfsAddress;
        *(UINT32*)HobRomImage->RsvdFfsInfo[i].FfsLength = \
                *(UINT32*)HobInRam->RsvdFfsInfo[i].FfsLength;
        HobRomImage->RsvdFfsInfo[i].FvIndex = HobInRam->RsvdFfsInfo[i].FvIndex;
        j = HobRomImage->RsvdFfsInfo[i].FvIndex;
        offset = HobRomImage->RsvdFfsInfo[i].FfsAddress - \
                 (UINT32)HobRomImage->FvInfo[j].FvAddress;
        (*PeiServices)->CopyMem ( \
                    (UINT8*)(HobRomImage->FvInfo[j].MemAddress + offset), \
                    (UINT8*)(HobRomImage->RsvdFfsInfo[i].FfsAddress), \
                    *(UINT32*)HobRomImage->RsvdFfsInfo[i].FfsLength & 0xffffff);
    }
#endif
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   CollectRomImageInfo
//
// Description:
//
// Input:       EFI_PEI_SERVICES**  - PeiServices
//              EFI_PHYSICAL_ADDRESS - Buffer
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
CollectRomImageInfo (
    IN EFI_PEI_SERVICES         **PeiServices,
    IN HOB_ROM_IMAGE            *HobRomImage
)
{
    ROM_AREA                *Area;
    UINT32                  FileSize;
    UINT8                   i = 0, j = 0, n = 0;
    UINTN                   k = 0;
    EFI_STATUS              Status = EFI_SUCCESS;
    EFI_FFS_FILE_HEADER     *FfsFile = NULL, *LastFfsFile = NULL;
    BOOLEAN                 IsDispatched = FALSE;
    UINT64                  DispatchedPeimBitMap = 0;
    EFI_GUID                *pGuid = NULL, *pLastFileOvrdeGuid = NULL;
    ROM_AREA                *RomLayout = NULL;
    
    Status = GetRomLayout(PeiServices, &RomLayout);
    if ( EFI_ERROR(Status) || (RomLayout == NULL) ) return EFI_UNSUPPORTED;
#if (PI_SPECIFICATION_VERSION < 0x0001000A)
    DispatchedPeimBitMap = GetDispatchedPeimBitMap(PeiServices);
#endif
    // Get Last File Override Guid for skiping unnecessary files copied
    // for reducing POST time. 
    Status = (*PeiServices)->LocatePpi ( PeiServices, \
                                         &gLastFfsFileOverrideGuid, \
                                         0, \
                                         NULL, \
                                         &pLastFileOvrdeGuid);
    if (EFI_ERROR(Status)) pLastFileOvrdeGuid = NULL;

    // find last ffs file for calculating used rom space for each Firmware Volume.
    for (Area = RomLayout; Area->Size != 0; Area++, FfsFile = NULL, FileSize = 0) {
        if (Area->Type != RomAreaTypeFv) continue;
        if (!(Area->Attributes & (ROM_AREA_FV_PEI_ACCESS + ROM_AREA_FV_DXE)))
            continue;
//<EIP153486+> >>>
        if ( ((EFI_FIRMWARE_VOLUME_HEADER*)Area->Address)->Signature != FV_SIGNATURE )
            return EFI_VOLUME_CORRUPTED;
//<EIP153486+> >>>
        // find last ffs file for calculating used rom space.
        do {
            IsDispatched = TRUE;
            Status = (*PeiServices)->FfsFindNextFile (
                         PeiServices, \
                         EFI_FV_FILETYPE_ALL, \
                         (EFI_FIRMWARE_VOLUME_HEADER*)(Area->Address), \
                         &FfsFile );
#if PEI_RAM_BOOT_S3_SUPPORT == 1
            if (!EFI_ERROR(Status)) {
                if  ((FfsFile->Type == EFI_FV_FILETYPE_FREEFORM) || \
                     (FfsFile->Type == EFI_FV_FILETYPE_RAW)) {
                    for (pGuid = PreservedFfsGuid; pGuid->Data1 != 0; pGuid++) {
                        if (!guidcmp(&FfsFile->Name, pGuid)) {
                            HobRomImage->RsvdFfsInfo[n].FfsAddress = (UINT32)FfsFile;
                            *(UINT32*)HobRomImage->RsvdFfsInfo[n].FfsLength = \
                                *(UINT32*)FfsFile->Size & 0xffffff;
                            HobRomImage->RsvdFfsInfo[n].FvIndex = i;
                            HobRomImage->NumOfPeim = ++n;
                            break;
                        }
                    }
				}
				// Terminate searching Last File if Last File Override Guid 
                // is defined and is identified.
                if ((pLastFileOvrdeGuid != NULL) && \
                    (!guidcmp(&FfsFile->Name, pLastFileOvrdeGuid))) {
                    FfsFile = LastFfsFile;
                    Status = EFI_NOT_FOUND;
                } else LastFfsFile = FfsFile;
            }
#endif
#if PEI_RAM_BOOT_S3_SUPPORT == 2
            if (!EFI_ERROR(Status)) {
                // 1. Save not dispatched PEIM ffs.
                if (FfsFile->Type == EFI_FV_FILETYPE_PEIM) {
#if (PI_SPECIFICATION_VERSION >= 0x0001000A)
                    if ((!IsPeimDispatched(PeiServices, FfsFile, k)) &&
#else
                    if ((!(DispatchedPeimBitMap & Shl64(1, k))) &&
#endif
                            (!IsRecoveryFfsFile (PeiServices, FfsFile))) {
                        IsDispatched = FALSE;
                    }
                    k++;
                } else {
                    if (FfsFile->Type == EFI_FV_FILETYPE_FREEFORM) {
                        // 2. Save Smbios Flash Data FFS if it's stored in FV_BB.
                        for (pGuid = PreservedFfsGuid; pGuid->Data1 != 0; pGuid++)
                        {
                            if (!guidcmp(&FfsFile->Name, pGuid)) {
                                IsDispatched = FALSE;
                                break;
                            }
                        }
                    }
                }
                if (!IsDispatched) {
                    HobRomImage->PeimFfsInfo[j].PeimAddress = (UINT32)FfsFile;
                    HobRomImage->PeimFfsInfo[j].PeimLength = \
                            *(UINT32*)FfsFile->Size & 0xffffff;
                    HobRomImage->NumOfPeim = ++j;
                }
            }
#endif
        } while (!EFI_ERROR(Status));
        FileSize = *(UINT32*)LastFfsFile->Size & 0xffffff;
        FileSize += ((UINT32)LastFfsFile - (UINT32)Area->Address);
        HobRomImage->FvInfo[i].FvAddress = (UINT32)Area->Address;
        HobRomImage->FvInfo[i].FvLength = Area->Size;
        HobRomImage->FvInfo[i].UsedBytes = FileSize;
        HobRomImage->FvInfo[i].MemAddress = 0;
        if (IsBootBlockFirmwareVolumes(PeiServices, Area->Address))
            HobRomImage->FvInfo[i].IsBootFv = TRUE;
        else HobRomImage->FvInfo[i].IsBootFv = FALSE;
        HobRomImage->NumOfFv = ++i;
    }
    if (HobRomImage->NumOfFv == 0) return EFI_UNSUPPORTED;
    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   FvHobSwitchToRam
//
// Description: This procedure redirect the FV Base Address of FV HOB to RAM.
//
// Input:       EFI_PEI_SERVICES**  - PeiServices
//              EFI_PHYSICAL_ADDRESS - RomImageBuffer
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
FvHobSwitchToRam (
    IN EFI_PEI_SERVICES         **PeiServices,
    IN HOB_ROM_IMAGE            *HobRomImage
)
{
    VOID                    *p;
    EFI_HOB_FIRMWARE_VOLUME	*FvHob;
    UINT8                   i, j = 0;
    EFI_BOOT_MODE           BootMode;
    EFI_STATUS              Status = EFI_SUCCESS;
    EFI_PEI_FIRMWARE_VOLUME_INFO_PPI *FirmwareVolumeInfo = NULL;
#if (PI_SPECIFICATION_VERSION >= 0x0001000A)
    PEI_CORE_INSTANCE       *PrivateData;
    PrivateData = PEI_CORE_INSTANCE_FROM_PS_THIS (PeiServices);
#endif

    // Get current Boot Mode.
    Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
    if (EFI_ERROR(Status)) return EFI_UNSUPPORTED;

    // Update FV HOB (BaseAddress)
    for ((*PeiServices)->GetHobList(PeiServices,&p);
            !(FindNextHobByType(EFI_HOB_TYPE_FV,&p)); ) {
        FvHob = (EFI_HOB_FIRMWARE_VOLUME*)p;
        for (i = 0; i < HobRomImage->NumOfFv; i++) {
            if ((FvHob->BaseAddress == HobRomImage->FvInfo[i].FvAddress) && \
                    (FvHob->Length == HobRomImage->FvInfo[i].FvLength) && \
                    (HobRomImage->FvInfo[i].MemAddress != 0) && \
                    (HobRomImage->FvInfo[i].FvMemReady)) {
#if (PI_SPECIFICATION_VERSION >= 0x0001000A)
                for (j = 0; j < PrivateData->FvCount; ++j) {
                    if ((UINT32)FvHob->BaseAddress == (UINT32)PrivateData->Fv[j].FvHandle) {
                        PrivateData->Fv[j].FvHandle = (EFI_PEI_FV_HANDLE)HobRomImage->FvInfo[i].MemAddress;
                    }
                }
#endif
                FvHob->BaseAddress = HobRomImage->FvInfo[i].MemAddress;
#if PEI_RAM_BOOT_S3_SUPPORT == 2
                if ((BootMode == BOOT_ON_S3_RESUME) && \
                        (!HobRomImage->FvInfo[i].IsBootFv)) {
                    FvHob->Header.HobType = EFI_HOB_TYPE_UNUSED;
                }
#endif
                break;
            }
        }
    }

    return EFI_SUCCESS;
}
#if defined PRESERVE_NESTED_FV_IN_MEM && PRESERVE_NESTED_FV_IN_MEM == 1
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   PeiRamBootPreserveNestedFv
//
// Description: This procedure redirect the FV Base Address of FV HOB to RAM in
//              End of PEI Phase PPI.
//
// Input:       EFI_PEI_SERVICES**          - PeiServices
//              EFI_PEI_NOTIFY_DESCRIPTOR*  - NotifyDescriptor
//              VOID*                       - NullPpi
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
PeiRamBootPreserveNestedFv (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *NullPpi
)
{
    VOID                        *p, *p1, *p2;
    UINT8                       NumNestedFv = 0;
    INTN                        Result;
    EFI_STATUS                  Status;
    EFI_BOOT_MODE               BootMode;
    EFI_HOB_MEMORY_ALLOCATION   *Hob;
    EFI_HOB_FIRMWARE_VOLUME2    *FvHob2;

    Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
    if (EFI_ERROR(Status) || \
            (BootMode == BOOT_IN_RECOVERY_MODE) || \
            (BootMode == BOOT_IN_RECOVERY_MODE)) return EFI_SUCCESS;

    for ((*PeiServices)->GetHobList(PeiServices,&p); \
            !(FindNextHobByType(EFI_HOB_TYPE_GUID_EXTENSION, &p));	) {
        Result = guidcmp(&((EFI_HOB_GUID_TYPE*)p)->Name, &gHobRomImageGuid);
        if (!Result) break;
    }
    if (Result) return EFI_UNSUPPORTED;

    if ((((HOB_ROM_IMAGE*)p)->HobValid == 1) && \
            (((HOB_ROM_IMAGE*)p)->NestedFvValid == 1)) return EFI_SUCCESS;
    p = (VOID*)((HOB_ROM_IMAGE*)p)->StolenHobMemory;
    for ((*PeiServices)->GetHobList(PeiServices,&p1);
            !(FindNextHobByType(EFI_HOB_TYPE_FV2,&p1)); ) {
        FvHob2 = (EFI_HOB_FIRMWARE_VOLUME2*)p1;
        for ((*PeiServices)->GetHobList(PeiServices,&p2); \
                !(FindNextHobByType(EFI_HOB_TYPE_MEMORY_ALLOCATION, &p2));	) {
            Hob = (EFI_HOB_MEMORY_ALLOCATION*)p2;
            if (Hob->AllocDescriptor.MemoryBaseAddress != \
                    (FvHob2->BaseAddress & (EFI_PHYSICAL_ADDRESS)~0xfff)) continue;
            Hob->AllocDescriptor.MemoryType = EfiRuntimeServicesData;
            ((HOB_ROM_IMAGE*)p)->NestedFvInfo[NumNestedFv].MemAddress = \
                    (UINT32)FvHob2->BaseAddress;
            ((HOB_ROM_IMAGE*)p)->NestedFvInfo[NumNestedFv].FvLength = \
                    (UINT32)FvHob2->Length;
            ((HOB_ROM_IMAGE*)p)->NumOfNestedFv = ++NumNestedFv;
        }
    }
    return EFI_SUCCESS;
}
#endif

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   PeiRamBootEndOfPei
//
// Description: This procedure redirect the FV Base Address of FV HOB to RAM in
//              End of PEI Phase PPI.
//
// Input:       EFI_PEI_SERVICES**          - PeiServices
//              EFI_PEI_NOTIFY_DESCRIPTOR*  - NotifyDescriptor
//              VOID*                       - NullPpi
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
PeiRamBootEndOfPei (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *NullPpi
)
{
    VOID                    *p;
    INTN                    Result;
    EFI_STATUS              Status = EFI_SUCCESS;
    EFI_BOOT_MODE           BootMode;
    // Update BaseAddress of FV HOB again if new FV is reported.
    for ((*PeiServices)->GetHobList(PeiServices,&p); \
            !(FindNextHobByType(EFI_HOB_TYPE_GUID_EXTENSION, &p));	) {
        Result = guidcmp(&((EFI_HOB_GUID_TYPE*)p)->Name, &gHobRomImageGuid);
        if (!Result) break;
    }
    if (Result) return EFI_UNSUPPORTED;

    Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
    if (EFI_ERROR(Status)) return EFI_UNSUPPORTED;
//<EIP153486+> >>>
    if ((BootMode == BOOT_IN_RECOVERY_MODE) || \
            (BootMode == BOOT_ON_FLASH_UPDATE)) return EFI_SUCCESS;
//<EIP153486+> <<<

#if PEI_RAM_BOOT_S3_SUPPORT != 2
    if (BootMode != BOOT_ON_S3_RESUME) {
        Status = CopyFirmwareVolumesToRam(PeiServices, (HOB_ROM_IMAGE*)p, TRUE);
    }
#endif
    SwitchPeiServiceDataToRam (PeiServices, (HOB_ROM_IMAGE*)p);
    FvHobSwitchToRam (PeiServices, (HOB_ROM_IMAGE*)p);
    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   PeiRamBootMemoryReady
//
// Description: This procedure allocate a Memory buffer and redirect the FV from
//              ROM to RAM in PERMANENT MEMORY INSTALLED PPI;
//
// Input:       EFI_PEI_SERVICES**          - PeiServices
//              EFI_PEI_NOTIFY_DESCRIPTOR*  - NotifyDescriptor
//              VOID*                       - NullPpi
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
PeiRamBootMemoryReady (
    IN EFI_PEI_SERVICES             **PeiServices,
    IN EFI_PEI_NOTIFY_DESCRIPTOR    *NotifyDescriptor,
    IN VOID                         *NullPpi
)
{
    EFI_STATUS                  Status;
    EFI_PHYSICAL_ADDRESS        Buffer = 0, HobBuffer = 0;
    HOB_ROM_IMAGE               *HobRomImage = NULL;
    EFI_BOOT_MODE               BootMode;
    EFI_PEI_READ_ONLY_VARIABLE2_PPI  *ReadOnlyVar2 = NULL;
    //EFI_GUID                    RomImageAddressGuid = ROM_IMAGE_ADDRESS_GUID;
    UINTN                       VarSize = 0;
    UINT8                       i = 0;
    BOOLEAN                     HobValid = FALSE;

    // Get current Boot Mode.
    Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
    if (EFI_ERROR(Status)) return EFI_UNSUPPORTED;
    // Check Boot mode again, do nothing if recovery mode.
    if ((BootMode == BOOT_IN_RECOVERY_MODE) || \
            (BootMode == BOOT_ON_FLASH_UPDATE)) return EFI_SUCCESS;

    // Locate Read Only Variable PPI.
    Status = (*PeiServices)->LocatePpi ( PeiServices, \
                                         &gEfiPeiReadOnlyVariable2PpiGuid, \
                                         0, \
                                         NULL, \
                                         &ReadOnlyVar2   );
    if (!EFI_ERROR(Status)) {
        // Get HobRomImage Variable.
        VarSize = sizeof(EFI_PHYSICAL_ADDRESS);
        Status = ReadOnlyVar2->GetVariable ( ReadOnlyVar2, \
                                             L"HobRomImage", \
                                             &gRomImageAddressGuid, \
                                             NULL, \
                                             &VarSize, \
                                             &Buffer  );
        // If HobRomImage Found, validate the HOB  and FV memories, if both
        // valid, skip to copy rom to ram for fast boot.
        if (!EFI_ERROR(Status)) {
            HobRomImage = (HOB_ROM_IMAGE*)Buffer;
#if PEI_RAM_BOOT_S3_SUPPORT == 1
            for (i = 0, HobValid = FALSE; IsMrcColdBooteLink[i] != NULL; i++)
                HobValid = !IsMrcColdBooteLink[i](PeiServices);
            if (HobValid) {
#if WARM_BOOT_VERIFY_CHECKSUM
                HobValid = IsFvFfsChecksumGood(PeiServices, HobRomImage);
#else
                if (guidcmp(&HobRomImage->EfiHobGuidType.Name, \
                            &gHobRomImageGuid)) HobValid = FALSE;
#endif
            }
#endif
        }
    }

    if ( BootMode != BOOT_ON_S3_RESUME ) {
        Status = (*PeiServices)->CreateHob ( PeiServices, \
                                             EFI_HOB_TYPE_GUID_EXTENSION, \
                                             sizeof(HOB_ROM_IMAGE), \
                                             &HobRomImage   );
        if (EFI_ERROR(Status)) return EFI_UNSUPPORTED;
        HobRomImage->EfiHobGuidType.Name = gHobRomImageGuid;
        HobRomImage->HobValid = HobValid;
        HobRomImage->StolenHobMemory = 0;
#if PEI_RAM_BOOT_S3_SUPPORT == 1
        // Here checks whether memory mappings are changed.
        Status = (*PeiServices)->AllocatePages ( PeiServices, \
                 EfiRuntimeServicesData, \
                 1, \
                 &HobBuffer );
        if (EFI_ERROR(Status)) return EFI_UNSUPPORTED;
        HobRomImage->StolenHobMemory = (UINT32)HobBuffer;
        if ((!HobValid) || (!Buffer) || (HobBuffer != Buffer)) {
            HobRomImage->HobValid = HobValid = FALSE;
        }
#endif
        if (!HobValid) {
            HobRomImage->NumOfFv = 0;
            HobRomImage->NumOfPeim = 0;
            HobRomImage->NumOfNestedFv = 0;
            HobRomImage->SmbiosFlashData = 0;
            Status = CollectRomImageInfo (PeiServices, HobRomImage);
//<EIP153486*> >>>
            if (EFI_ERROR(Status)) {
                HobRomImage->EfiHobGuidType.Header.HobType = EFI_HOB_TYPE_UNUSED;
                return EFI_UNSUPPORTED;
            }
//<EIP153486*> <<<
        } else {
            RestoreHobInfoFromRam ( PeiServices, \
                                    HobRomImage, \
                                    (HOB_ROM_IMAGE*)Buffer);
        }
#if PEI_RAM_BOOT_S3_SUPPORT == 2
        Status = CopyMininumRomImage2Buffer(PeiServices, HobRomImage);
#else
        Status = PrepareForCopyRomToRam (PeiServices, HobRomImage);
        if (EFI_ERROR(Status)) return EFI_UNSUPPORTED;
        Status = CopyFirmwareVolumesToRam (PeiServices, HobRomImage, FALSE);
        if (EFI_ERROR(Status)) return EFI_UNSUPPORTED;
#endif
    }
    SwitchPeiServiceDataToRam (PeiServices, HobRomImage);
    FvHobSwitchToRam (PeiServices, HobRomImage);
    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   PeiRamBootEntry
//
// Description: PEI Entry Point for PeiRamBoot Driver.
//
// Input:       EFI_FFS_FILE_HEADER*    - FfsHeader
//              EFI_PEI_SERVICES**      - PeiServices
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
PeiRamBootEntry (
    IN EFI_FFS_FILE_HEADER  *FfsHeader,
    IN EFI_PEI_SERVICES     **PeiServices
)
{
    EFI_STATUS          Status = EFI_SUCCESS;
    EFI_BOOT_MODE       BootMode;

    Status = (*PeiServices)->GetBootMode (PeiServices, &BootMode);
    if ((BootMode == BOOT_IN_RECOVERY_MODE) || \
            (BootMode == BOOT_ON_FLASH_UPDATE)) return EFI_SUCCESS;
#if PEI_RAM_BOOT_S3_SUPPORT == 0
    if (BootMode == BOOT_ON_S3_RESUME) return EFI_SUCCESS;
#endif
    Status = (*PeiServices)->NotifyPpi ( PeiServices, \
                                         PeiRamBootMemoryReadyNotify );
    Status = (*PeiServices)->NotifyPpi ( PeiServices, \
                                         PeiRamBootEndOfPeiNotify );
#if defined PRESERVE_NESTED_FV_IN_MEM && PRESERVE_NESTED_FV_IN_MEM == 1
    Status = (*PeiServices)->NotifyPpi ( PeiServices, \
                                         PeiRamBootNestedFvNotify );
#endif

    return EFI_SUCCESS;
}
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
