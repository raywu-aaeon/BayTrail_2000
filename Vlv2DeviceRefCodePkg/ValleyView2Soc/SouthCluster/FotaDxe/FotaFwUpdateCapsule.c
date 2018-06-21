/**

  Copyright (c) 2009 - 2014, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Protocol/Spi.h>
#include <Library/PcdLib.h>
#include <Library/ShellLib.h>
#include <Library/DebugLib.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>

#include <Library/MemoryAllocationLib.h>
#include <Library/UefiLib.h>
#include <Library/FileHandleLib.h>

#include <Protocol/SimpleFileSystem.h>
#include <Protocol/PchPlatformPolicy.h>

#include <Guid/Gpt.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/GlobalVariable.h>
#include <Guid/Vlv2Variable.h>
#include <Guid/CapsuleProcessingResult.h>
#include <Library/DxeServicesTableLib.h>

#include <Library/DfuCapsuleLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/FmiLib.h>
#include <Library/Esrt.h>
#include <Library/PciLib.h>
#include <Library/PcdLib.h>
#include <SeCAccess.h>
#include <Protocol/PchExtendedReset.h>
#include <Protocol/FlashProtocol.h>  //AMI_OVERRIDE - EIP153486 Fault Tolerant Function

#include "FlashOperation.h"
#include "FotaFwUpdate.h"
#include "FotaUpdateCommon.h"

#define _SHOW_LOG_
#define CAP_BACKUP_BINARY L"Capsule.bin"
//EIP175650 >>
#if defined( EC_SUPPORT ) && ( EC_SUPPORT == 1 )
#include "Library/KscLib.h"
#define LOW_BATTERY 25 
#endif
//EIP175650 <<
EFI_SPI_PROTOCOL  *mSpiProtocol;
FLASH_PROTOCOL    *Flash = NULL;  //AMI_OVERRIDE - EIP153486 Fault Tolerant Function
UINT8             *mFileBuffer;
UINT8             *PartitionBuffer;
extern EFI_GUID gEfiNormalSetupGuid;
EFI_PCH_EXTENDED_RESET_PROTOCOL  *mPchExtendedResetProtocol;
EFI_GUID mSystemFirmwareGuid    = { 0x3bbdb30d, 0xaa9b, 0x4915, { 0x95, 0x03, 0xe4, 0xb8, 0x2f, 0xb6, 0xb6, 0xe9 }};
EFI_GUID BiosCapsuleFromAfuGuid = { 0xCD193840, 0x2881, 0x9567, { 0x39, 0x28, 0x38, 0xc5, 0x97, 0x53, 0x49, 0x77 }};

#define  MAX_UPD_SIZE  0x300000

#if defined(_SHOW_LOG_)
#define FWPrompt(...)       Print(__VA_ARGS__)
#define FWPrompt_LOW(...)   ((void) 0)
#else
#define FWPrompt(...)       ((void) 0)
#define FWPrompt_LOW(...)   ((void) 0)
#endif


BIOS_IMAGE_INFO   *CurrentImageInfo = NULL;
BIOS_IMAGE_INFO   *UpdateImageInfo = NULL;
UINT8             *IBBPtr = NULL;
UINT8             *UCodePtr = NULL;
UINT8             *Stage2Ptr = NULL;
UINT8             *RecoveryPtr = NULL;
UINTN             IBBSize = 0;
UINTN             UCodeSize = 0;
UINTN             Stage2Size = 0;
UINTN             RecoverySize = 0;
CHAR16            EfiMemoryConfigVariable[] = L"MemoryConfig";



/*++

Routine Description:

  The routine to get BIOS_MODULE_INFO from OEM data section of current firmware image.

Arguments:

  CurrentInfo  - The BIOS module info structure to be read from OEM data section

Returns:

  EFI_SUCCESS   if current image contains BIOS module info.
  Other               if failed to get BIOS module info, or the info corrupted.

 --*/

EFI_STATUS
GetCurrentBIOSImageInfo(
  IN OUT BIOS_IMAGE_INFO *CurrentInfo
  )
{

  EFI_STATUS         Status = EFI_NOT_FOUND;
  UINT8              *Data8 = NULL;
  UINTN              LoopIndex = 0;


//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
#ifdef INTEL_SPI_PROTOCOL
  if(mSpiProtocol == NULL) {
#else
  if(Flash == NULL){
#endif
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<
    Status = EFI_NOT_READY;
    goto info_exit;
  }

  Data8 = AllocateZeroPool(SECTOR_SIZE_4KB);
  if(Data8 == NULL) {
    FWPrompt (L"Failed to allocate buffer for image info.\r\n");
    Status = EFI_OUT_OF_RESOURCES;
    goto info_exit;
  }

//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
#ifdef INTEL_SPI_PROTOCOL
  Status = FlashRead (VLV_MANIFEST_ADDRESS, Data8, SECTOR_SIZE_4KB, EnumSpiRegionAll);
#else
  Status = Flash->DeviceWriteEnable();
  Status = Flash->Read ((VOID*)(FULL_FLASH_BASE_ADDRESS+VLV_MANIFEST_ADDRESS), SECTOR_SIZE_4KB, Data8);
  Status = Flash->DeviceWriteDisable();
#endif
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<
  if (!EFI_ERROR(Status)) {
    for(LoopIndex = VLV_OEM_DATA_OFFSET; LoopIndex <= VLV_OEM_DATA_SIZE; LoopIndex++) {
      CurrentInfo = (BIOS_IMAGE_INFO *)(Data8 + LoopIndex);
      if(CurrentInfo->Signature == SIGNATURE_32('$','B','N','F')) {
        Status = EFI_SUCCESS;
        break;
      }
    }
  }

info_exit:
  if(Data8 != NULL) {
    FreePool(Data8);
  }
  return Status;

}


//
//This API attempts to retrieve BIOS image info from the update image.
//The real IBB size and its pointer is provided. So this needs to be called after IBBHandler();
//

EFI_STATUS
GetUpdateBIOSImageInfo(
  IN UINT8                *pIBB,
  IN UINTN                IBBSize,
  IN OUT BIOS_IMAGE_INFO  **UpdateImageInfo  //AMI_OVERRIDE - EIP153486 Fault Tolerant Function
  )
{
  EFI_STATUS      Status = EFI_NOT_FOUND;
  UINTN           LoopIndex = 0;
  UINTN           OemDataOffset = 0;

  if(IBBSize == FOTA_IBB_SIZE_128K) {
    OemDataOffset =  VLV_OEM_DATA_OFFSET;
  } else {
    OemDataOffset =  VLV_KEY_MANIFEST_SIZE + VLV_OEM_DATA_OFFSET;
  }

  for(LoopIndex = 0; LoopIndex < VLV_OEM_DATA_SIZE; LoopIndex++) {
    *UpdateImageInfo = (BIOS_IMAGE_INFO*)(pIBB + OemDataOffset + LoopIndex);  //AMI_OVERRIDE - EIP153486 Fault Tolerant Function
    if((*UpdateImageInfo)->Signature == SIGNATURE_32('$','B','N','F')) {  //AMI_OVERRIDE - EIP153486 Fault Tolerant Function
      Status = EFI_SUCCESS;
      break;
    }
  }

  return Status;

}


//
//This retrieves information about whether the platform is with key manifest, and where the alternative BIOS limit is.
//
EFI_STATUS
GetSecPlatformInfo(
  IN OUT SEC_PLATFORM_INFO *PlatformInfo
  )
{
  SEC_BOOT_STS   SBStatus;

  if(PlatformInfo == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  SBStatus.dw = HeciPciRead32(REG_SEC_BOOT_STS);

  FWPrompt (L"TXE secure boot status reg:0x%x.\r\n", SBStatus.dw);
  FWPrompt (L"SBStatus.r.AltBiosLimit:0x%x.\r\n", SBStatus.r.AltBiosLimit);

  PlatformInfo->WithKeyManifest = (SBStatus.r.KeyManifestID == 0)?FALSE:TRUE;
  PlatformInfo->AlternativeBiosLimit = (SBStatus.r.AltBiosLimit == 0)?0:(SBStatus.r.AltBiosLimit << 12) | 0xFFF;

  return EFI_SUCCESS;
}


//
//Default Image Layout after LoadDFUImage() call:
//
//----------------------------------------------------------
//|      AUTH Header                                                                         |
//|---------------------------------------------------------
//|      SFIH Header                                                                          |
//|---------------------------------------------------------
//|      132KB IBB(key manfiest might be paddded with 0xFF)               |
//|---------------------------------------------------------
//|      Remaining part of BIOS image                                                 |
//|---------------------------------------------------------
//|      3MB TXE UPD Image                                                               |
//|---------------------------------------------------------
//|      Platform Specific data to be updated                                         |
//|---------------------------------------------------------
//
//
//


//
//This returns the IBB data buffer that the update process needs for FOTA update routine. Returns the pointer/size of the IBB block depending on
//whether KeyManifest is required on the platform. Could be 128KB or 132KB. The GenSysFwUpdImage tool will always pass 132KB binary in the IBB part,
//which may have 4KB data needs to be stripped off.
//

EFI_STATUS
IBBHandler(
  IN  UINT8           *DataIn,
  IN OUT UINT8        *DataOut,

  IN OUT UINTN        *pIBBSize
  )
{
  EFI_STATUS        Status;
  SEC_PLATFORM_INFO PlatformInfo;

  if(DataIn == NULL || pIBBSize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetSecPlatformInfo(&PlatformInfo);

  if(!EFI_ERROR(Status)) {
    if(PlatformInfo.WithKeyManifest == TRUE) {
      CopyMem(DataOut, DataIn, FOTA_IBB_SIZE_132K);
      *pIBBSize = FOTA_IBB_SIZE_132K;
    } else {
      CopyMem(DataOut, DataIn + VLV_KEY_MANIFEST_SIZE, FOTA_IBB_SIZE_128K);
      *pIBBSize = FOTA_IBB_SIZE_128K;
    }
  } else { //If we failed to get platform info, default to only flash
    CopyMem(DataOut, DataIn + VLV_KEY_MANIFEST_SIZE, FOTA_IBB_SIZE_128K);
    *pIBBSize = FOTA_IBB_SIZE_128K;
  }

  return EFI_SUCCESS;
}

//
//This returns the second stage of BIOS image.  If we failed to get the 2nd stage BIOS information from OEM data in the update image,
//we will fall back to assume default layout below.
//
//The caller should pass BIOS_IMAGE_INFO for the update image, and also indicate the buffer/length to be parsed by "DataIn" and "pStage2Size"
//
// -----------------------------------
//|         Variable   (256KB*2) (500000)      |
// -----------------------------------
//|         Padding                                       |
// -----------------------------------
//|          Ucode     (192KB)(600000)           |
// -----------------------------------
//|         Padding                                       |
// -----------------------------------
//|         BIOS Stage 2(up to 0x7c0000)      |
// -----------------------------------
//

EFI_STATUS
Stage2Handler(
  IN BIOS_IMAGE_INFO *UpdateImageInfo,
  IN UINT8           *DataIn,
  IN OUT UINT8       *DataOut,
  IN OUT UINTN       *pStage2Size
  )
{

  UINTN           LoopIndex = 0;
  BOOLEAN         Stage2Found = FALSE;
  UINT8 kFv_Compact_Guid[16] = {0x93,0xFD,0x21,0x9E,0x72,0x9C,0x15,0x4C,0x8C,0x4B,0xE7,0x7F,0x1D,0xB2,0xD7,0x92};

  if(DataIn == NULL || pStage2Size == NULL || (*pStage2Size == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if(UpdateImageInfo == NULL) { //Failing to get the update image info from OEM data section, will fall back to default settings.
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
#ifdef INTEL_SPI_PROTOCOL
    *pStage2Size = VLV_DEFAULT_FOTA_STGTWO_SIZE;
    CopyMem(DataOut, DataIn+VLV_DEFAULT_FOTA_STGTWO_OFFSET, VLV_DEFAULT_FOTA_STGTWO_SIZE);
#else
    *pStage2Size = FV_MAIN_SIZE;
#if defined( FWCAPSULE_FILE_FORMAT ) && ( FWCAPSULE_FILE_FORMAT == 1 )
    CopyMem( DataOut, DataIn + FV_MAIN_OFFSET + 0x1000, FV_MAIN_SIZE );
#else
    CopyMem( DataOut, DataIn + FV_MAIN_OFFSET, FV_MAIN_SIZE );
#endif
#endif
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<
    Stage2Found = TRUE;
  } else {
    //
    //Retrieve stage 2 BIOS information from the update image OEM data section.
    //
    *pStage2Size = UpdateImageInfo->StageTwoInfo.Size;
    //
    //Search the first occurence of GUID {9E21FD93-9C72-4c15-8C4B-E77F1DB2D792} for FVMAIN_COMPACT. OEM needs to customize this
    //if using different GUID or FV layout.
    //
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>	
#if defined( FWCAPSULE_FILE_FORMAT ) && ( FWCAPSULE_FILE_FORMAT == 1 )
    CopyMem( DataOut, DataIn + UpdateImageInfo->StageTwoInfo.SpiOffset + 0x1000, *pStage2Size);
#else
    CopyMem( DataOut, DataIn + UpdateImageInfo->StageTwoInfo.SpiOffset, *pStage2Size);
#endif
    Stage2Found = TRUE;
	
	return (Stage2Found == TRUE)?EFI_SUCCESS:EFI_NOT_FOUND;
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<
    for(LoopIndex = 0; LoopIndex < (*pStage2Size)-16; LoopIndex ++) {
      if(CompareMem(&kFv_Compact_Guid[0],DataIn+LoopIndex, 16)) {
        continue;
      } else {
        CopyMem(DataOut, DataIn + LoopIndex - 0x48, *pStage2Size);
        Stage2Found = TRUE;
        break;
      }
    }
  }

  return (Stage2Found == TRUE)?EFI_SUCCESS:EFI_NOT_FOUND;

}

//This returns the pointer to the uCode portion of the bios image in the update payload.
EFI_STATUS
UCodeHandler(
  IN BIOS_IMAGE_INFO *UpdateImageInfo,
  IN UINT8           *DataIn,
  IN OUT UINT8       *DataOut,
  IN OUT UINTN       *pUCodeSize

  )
{
  UINTN       LoopIndex = 0;
  BOOLEAN     UCodeFound = FALSE;
  UINT8 kFv_UCode_Guid[16] = {0x36,0xB2,0x7D,0x19,0x56,0xF8,0x24,0x49,0x90,0xF8,0xCD,0xF1,0x2F,0xB8,0x75,0xF3};

  if(DataIn == NULL || pUCodeSize == NULL || (*pUCodeSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if(UpdateImageInfo == NULL) { //Failing to get Ucode information from OEM data section, will fall back to default settings.
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
#ifdef INTEL_SPI_PROTOCOL
    *pUCodeSize = VLV_DEFAULT_FOTA_UCODE_SIZE;
    CopyMem(DataOut,  DataIn + VLV_DEFAULT_FOTA_UCODE_OFFSET, VLV_DEFAULT_FOTA_UCODE_OFFSET);
#else
    *pUCodeSize = FV_MICROCODE_SIZE;
#if defined( FWCAPSULE_FILE_FORMAT ) && ( FWCAPSULE_FILE_FORMAT == 1 )
    CopyMem( DataOut,  DataIn + MICROCODE_OFFSET + 0x1000, FV_MICROCODE_SIZE );
#else
    CopyMem( DataOut,  DataIn + MICROCODE_OFFSET , FV_MICROCODE_SIZE );
#endif
#endif
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<
    UCodeFound = TRUE;
  } else {
    //
    //Retrieve uCode information from the update image OEM data section
    //
    *pUCodeSize = UpdateImageInfo->UCodeInfo.Size;
    //
    //Search the first occurence of GUID {197DB236-F856-4924-90F8-CDF12FB875F3}
    //
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
#if defined( FWCAPSULE_FILE_FORMAT ) && ( FWCAPSULE_FILE_FORMAT == 1 )
    CopyMem( DataOut, DataIn + UpdateImageInfo->UCodeInfo.SpiOffset + 0x1000, *pUCodeSize);
#else
    CopyMem( DataOut, DataIn + UpdateImageInfo->UCodeInfo.SpiOffset, *pUCodeSize);
#endif
    UCodeFound = TRUE;
	
	return (UCodeFound == TRUE)?EFI_SUCCESS:EFI_NOT_FOUND;
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<
    for(LoopIndex = 0; LoopIndex <(*pUCodeSize)-16; LoopIndex ++) {
      if(CompareMem(&kFv_UCode_Guid[0], DataIn+LoopIndex, 16)) {
        continue;
      } else {
        CopyMem(DataOut,DataIn + LoopIndex - 0x48, *pUCodeSize);
        UCodeFound = TRUE;
        break;
      }
    }
  }

  return (UCodeFound == TRUE)?EFI_SUCCESS:EFI_NOT_FOUND;

}

//
//This handler extracts Variable section from the BIOS image. By default variable is not updated unless there's special case that needs settings change.
//(e.g. SetupDefault change, or secure boot keys change...)
//For reference BIOS, tool will pre-extract SetupDefault from the variable region and put it into "PDR" part of the SFIH image, so this handler is not needed.
//
EFI_STATUS
VariableHandler(
  IN UINT8           *DataIn,
  IN OUT UINT8       *DataOut,
  IN OUT UINTN       *pVariableSize

  )
{
  //
  //TODO: OEM add platform specific code to handle variable, if neccessary
  //
  return EFI_UNSUPPORTED;
}

//This returns the recovery module from the remaining part of BIOS excluding IBB. Normally there's no such recovery module so
EFI_STATUS
RecoveryModuleHandler(
  IN BIOS_IMAGE_INFO *UpdateImageInfo,
  IN UINT8           *DataIn,

  IN OUT UINT8       *DataOut,
  IN OUT UINTN       *pRecoverySize
  )
{
  if(DataIn == NULL || pRecoverySize == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if(UpdateImageInfo != NULL && UpdateImageInfo->RecoveryModuleInfo.Size == 0) { //By default there's no recovery module info.
    return EFI_NOT_FOUND;
  } else if(UpdateImageInfo!= NULL) {
    //
    //TODO: OEM needs to customize here to parse the "2nd stage" image (which includes ucode, variable, bios stage 2 and some padding) and extract specific
    //recovery module.
    //
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
    *pRecoverySize = UpdateImageInfo->RecoveryModuleInfo.Size;

#if defined( FWCAPSULE_FILE_FORMAT ) && ( FWCAPSULE_FILE_FORMAT == 1 )
    CopyMem( DataOut, DataIn + UpdateImageInfo->RecoveryModuleInfo.SpiOffset + 0x1000, *pRecoverySize);
#else
    CopyMem( DataOut, DataIn + UpdateImageInfo->RecoveryModuleInfo.SpiOffset, *pRecoverySize);
#endif
  } else if(UpdateImageInfo == NULL) {
    *pRecoverySize = FV_BB_SIZE;
#if defined( FWCAPSULE_FILE_FORMAT ) && ( FWCAPSULE_FILE_FORMAT == 1 )
    CopyMem( DataOut,  DataIn + FV_BB_OFFSET + 0x1000, FV_BB_SIZE );
#else
    CopyMem( DataOut,  DataIn + FV_BB_OFFSET, FV_BB_SIZE );
#endif
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<
  }

  return EFI_SUCCESS;

}


FOTA_BIOS_UPDATE_TABLE FotaBiosHandlerTable = {
  IBBHandler,
  Stage2Handler,
  UCodeHandler,
  VariableHandler,
  RecoveryModuleHandler,
  0x0000
};

EFI_STATUS
ExtractCodePartitions(
  IN UINT8* kFileBuffer,
  IN UINT32 SecUpdSize,
  OUT UINT32* mDestSize,
  OUT UINT8* ReUseSrcBuf,
  OUT VERSION *SecVersion
  )
{
  EFI_STATUS          Status;
  FPTHdrV2            *fpt_hdr;
  UINT32              NParts;
  UINT32              PartIdx;
  UINT8               *BufPtr;
  FPTEntryHdrV1       *fpt_entry;
  UINT32               FTPREnd;
  UINT32               FTPRStart;
  UINT32               NFTPEnd;
  UINT32               NFTPStart;
  UINT32               MDMVEnd;
  UINT32               MDMVStart;

  Status = EFI_SUCCESS;
  NParts = 0;
  PartIdx = 0;
  BufPtr = kFileBuffer;
  *mDestSize = 0;
  *ReUseSrcBuf = 1;
  FTPREnd = 0;
  NFTPEnd = 0;
  MDMVEnd = 0;
  FTPRStart = 0;
  NFTPStart = 0;
  MDMVStart = 0;

  //
  //TXE UPD size must not exceed 0x300000
  //
  if(SecUpdSize > MAX_UPD_SIZE) {
    FWPrompt (L"Invalid UPD size. Aborting.\r\n");
    return EFI_ABORTED;
  }

  fpt_hdr = (FPTHdrV2*)kFileBuffer;
  if(fpt_hdr->Signature != SIGNATURE_32('$','F','P','T')) {
    FWPrompt (L"Invalid FPT header. Aborting.\r\n");
    return EFI_ABORTED;
  }
  NParts = fpt_hdr->NumFptEntries;
  if(NParts <2) {
    FWPrompt (L"Not enough partitions for the update.\r\n");
    return EFI_ABORTED;
  }
  //
  //Parse the FPT entries, and find FTPR, NFTP and MDMV partitions, and check if they're continuous blocks
  //
  BufPtr = kFileBuffer + fpt_hdr->HdrLen;

  for(PartIdx = 0; PartIdx < NParts; PartIdx++) {
    fpt_entry = (FPTEntryHdrV1*)BufPtr;
    if(fpt_entry->PartitionName == SIGNATURE_32('F','T','P','R')) {
      FTPRStart = fpt_entry->Offset;
      if(FTPRStart > SecUpdSize || fpt_entry->Length > SecUpdSize) {
        return EFI_ABORTED;
      }
      PartitionBuffer += fpt_entry->Offset;
      *mDestSize += fpt_entry->Length;
      FTPREnd = fpt_entry->Offset + fpt_entry->Length -1;
      FWPrompt (L"FTPR Size:%x.\r\n", *mDestSize);
    } else if(fpt_entry->PartitionName == SIGNATURE_32('N','F','T','P')) {
      NFTPStart = fpt_entry->Offset;
      if(NFTPStart > SecUpdSize || fpt_entry->Length > SecUpdSize) {
        return EFI_ABORTED;
      }
      NFTPEnd = fpt_entry->Offset + fpt_entry->Length -1;
      *mDestSize += fpt_entry->Length;
      FWPrompt (L"NFTP Size: %x\n", fpt_entry->Length);
    } else if(fpt_entry->PartitionName == SIGNATURE_32('M','D','M','V')) {
      MDMVStart = fpt_entry->Offset;
      if(MDMVStart > SecUpdSize || fpt_entry->Length > SecUpdSize) {
        return EFI_ABORTED;
      }
      MDMVEnd = fpt_entry->Offset + fpt_entry->Length -1;
      *mDestSize += fpt_entry->Length;
      FWPrompt (L"MDMV Size: %x\n", fpt_entry->Length);
    }
    BufPtr += sizeof(FPTEntryHdrV1);
  }

  if ((FTPRStart < NFTPStart) &&
      (FTPREnd + 1 == NFTPStart) &&
      (((NFTPStart < MDMVStart) && (NFTPEnd + 1 == MDMVStart)) || (MDMVStart == 0))) {
    //
    //No need to copy buffer. Just point to the start of FTPR and provide the length.
    //
    FWPrompt (L"Reuse buffer allocated previously. Buffer size:%x, offset:%x\r\n", *mDestSize, FTPRStart);
    SecVersion->Major = *(UINT16*)(PartitionBuffer+ 0x24);
    SecVersion->Minor = *(UINT16*)(PartitionBuffer + 0x26);
    SecVersion->Hotfix= *(UINT16*)(PartitionBuffer + 0x28);
    SecVersion->Build = *(UINT16*)(PartitionBuffer + 0x2A);
    return EFI_SUCCESS;
  } else {
    //
    //We need to allocate buffer for this
    //
    if ((FTPRStart == 0) || (NFTPStart == 0)) {
      FWPrompt (L"Missing required partitions for a full update.\r\n");
      return EFI_ABORTED;
    } else {
      PartitionBuffer = AllocateZeroPool(*mDestSize);
      if(PartitionBuffer == NULL) {
        return EFI_ABORTED;
      }

      CopyMem(PartitionBuffer, mFileBuffer+FTPRStart, FTPREnd-FTPRStart +1);
      CopyMem(PartitionBuffer+(FTPREnd-FTPRStart+1), mFileBuffer+NFTPStart, NFTPEnd-NFTPStart+1);
      if (MDMVStart != 0) {
        CopyMem(PartitionBuffer+(FTPREnd-FTPRStart+1)+(NFTPEnd-NFTPStart+1), mFileBuffer+MDMVStart, MDMVEnd-MDMVStart+1);
      }
      *ReUseSrcBuf = 0;
      FWPrompt (L"Allocated new buffer to pass to TXE.\r\n");
      SecVersion->Major = *(UINT16*)(PartitionBuffer + 0x24);
      SecVersion->Minor = *(UINT16*)(PartitionBuffer+ 0x26);
      SecVersion->Hotfix= *(UINT16*)(PartitionBuffer + 0x28);
      SecVersion->Build = *(UINT16*)(PartitionBuffer + 0x2A);
      return EFI_SUCCESS;

    }

  }

}

//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
FILE_NAME_TO_STRING mFlashFileNames[] = {
  { L"BIOSUpdate.FV" },
};
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<

FILE_NAME_TO_STRING gStg2BackupFileNames[] = {
  { L"\\firmware.bin"},
};

PLATFORM_PCI_DEVICE_PATH gEmmcDevPath0 = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE (0x00, 0x10),
  gEndEntire
};

PLATFORM_PCI_DEVICE_PATH gEmmcDevPath1 = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE (0x00, 0x17),
  gEndEntire
};

//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
PLATFORM_PCI_DEVICE_PATH gStatDevPath0 = {
  gPciRootBridge,
  PCI_DEVICE_PATH_NODE (0x00, 0x13),
  gEndEntire
};
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<



//
//This is the entry of the upddate driver
//
EFI_STATUS
SysUpdateEntry(
  IN EFI_HANDLE                ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  )
{
  //
  //FOTA status variables
  //
  EFI_STATUS                            Status;
  UINT32                                UpdateStatus = UPDATE_SUCCESS;
  DFU_FAILURE_REASON                    FailureReason = NO_FAILURE;
  UINT8                                 PwrStatus = PRE_CHECK_PASS;

  EFI_CAPSULE_PROCESSING_RESULT         CapsuleProcessingResult;
  UINTN                                 Size;
  UINT8                                 RecoveryCapsule = 0;
//  EFI_HANDLE                            FvProtocolHandle;
//  UINT8                                 *ShellPtr = NULL;
//  UINTN                                 ShellSize;
#if defined( EC_SUPPORT ) && ( EC_SUPPORT == 1 ) //EIP175650 
  UINT8                                 BatteryStatus;
  UINT8                                 BatteryCapacity;
#endif //EIP175650 
  //
  //FOTA backup handle variables
  //
  EFI_HANDLE                            *HandleArray;
  UINTN                                 HandleArrayCount;
  UINTN                                 Index = 0;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *Fs;
  EFI_FILE                              *Root;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
  DXE_PCH_PLATFORM_POLICY_PROTOCOL      *DxePlatformPchPolicy;
  BOOLEAN                               IsEmmc45 = FALSE;
  EFI_FILE                              *FileHandle;
  VOID                                  *FileBuffer;
  UINTN                                 FileSize;
  UINT8                                 *FwBinaryBuf;
  UINT8                                 LoopCount = 0;
  PCH_EXTENDED_RESET_TYPES              PchResetType;
  VERSION                               NewSecVersion;
  VERSION                               OldSecVersion;
  //
  //FOTA image/region variables
  //
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
#ifdef INTEL_SPI_PROTOCOL
  SPI_REGION_TYPE                       SpiRegionType;
#endif
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<
  UINT64                                FwBinarySize;
  UINTN                                 RegionOffset;
  UINTN                                 IBBSpiOffset = VLV_IBB_OFFSET_DEFAULT_128K;

  //TXE firmware update varibles
  UINT8   ReUseSrcBuf = 0;
  UINT32  PartitionsSize = 0;

  UINT32   RuleData = 0;
  UINT32   MaxBufferSize = 0;
  OEM_UUID SecOemId;
  UINT32 ResetType =4;
  UINT8  SecAddress = 0;
  UINT8  InProgress = 0;

  //
  //ESRT varibles and UEFI variable variables
  //
  ESRT_OPERATION_PROTOCOL   *EsrtOp;
  FW_RES_ENTRY                FwResourceEntry;

  UINT32  VarSize = 0;
  UINT8   *VarBuf = NULL;

  FW_RES_ENTRY_LIST        FwEntryList;
  UINT32                   FwEntrySize = sizeof(UINT32) + sizeof(FW_RES_ENTRY)*256;
  UINT8                    *FwEntryBuffer = NULL;

  PSYS_FW_SFIH_HEADER SfihHdr = NULL;
  PIFWI_AUTH_HEADER IfwiAuthHdr = NULL;

  //
  //Platform Info variables.
  //
  SEC_PLATFORM_INFO   SecPlatformInfo;
  BOOLEAN             bFaultTolerant = TRUE;
  BOOLEAN             bHasKeyManifest = FALSE;

  UINTN FlashRetry=0;

  UINTN CompareResult = 0;  //AMI_OVERRIDE - EIP153486 Fault Tolerant Function


  //
  //BIOS handler variables
  //
  FOTA_BIOS_UPDATE_TABLE       *FotaHandlerPtr = &FotaBiosHandlerTable;


  //
  //TXE firmware update progress variables
  //
  UINT32 Percentage = 0;
  UINT32 CurrentStage = 0;
  UINT32 TotalStages = 0;
  UINT32 LastUpdateStatus = 0;
  UINT32 LastResetType = 0;


  RegionOffset = 0;
  mFileBuffer = NULL;
  PartitionBuffer = NULL;
  Root = NULL;

  FwBinaryBuf = (UINT8*)NULL;
  FwBinarySize = 0;

  FileHandle = (EFI_FILE*)NULL;
  FileSize = 0;
  FileBuffer = NULL;

  PchResetType.PowerCycle = 0;
  PchResetType.GlobalReset = 1;

  ZeroMem(&FwEntryList, FwEntrySize);

  Status = gBS->LocateProtocol (&gEfiEsrtOperationProtocolGuid, NULL, (VOID**)&EsrtOp);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Cannot locate Esrt Operation Protocol.\n"));
    return Status;
  }


  /**************************************************
   *  Phase 1: Update Check Phase. Verify the image,
   *       Check battery level, and get BIOS image info from
   *       both current image and the update image.
   *
   **************************************************/
  //
  //Step 1.1 -- Battery level check -- Power button disale is done by BIOS core so no need to
  //disable it again.
  //If our attempt to check power status failed due to EFI_ABORTED, this
  //will mean we have problem communicating with ULPMC... Handling this as if
  //we don't have any fuel guage/power source detection method. Pass the check
  //directly.
  //

#if defined( EC_SUPPORT ) && ( EC_SUPPORT == 1 ) //EIP175650 
  // Make sure Current Battery Capacity > 25%
  // Disallowed Case:
  // 1.AC + Battery < 25%
  // 2.Battery < 25%
  // Allowed Case:                                                    
  // 1.AC only
  // 2.AC + Battery >= 25%
  // 3.Battery >= 25%
  InitializeKscLib();
  Status = SendKscCommand(KSC_C_READ_MEM);  // Read the KSC memory
  if(!EFI_ERROR(Status)) {
    Status = SendKscData(0x32);		// Battery 1 Status
    Status = ReceiveKscData(&BatteryStatus);
    if (BatteryStatus & BIT3) { // BIT3: Battery Present Bit
      FWPrompt (L"Battery Present, Current Battery Capacity must be greater than 25% to proceed.\r\n");
      Status = SendKscCommand(KSC_C_READ_MEM);  // Read the KSC memory
      if(!EFI_ERROR(Status)) {    
        Status = SendKscData(0x34);		// Battery Current Capacity Offset
		    Status = ReceiveKscData(&BatteryCapacity);
        if (BatteryCapacity < LOW_BATTERY){
          FWPrompt (L"Current Battery Capacity %d is lower than 25%. Firmware update is not allowed.\r\n", BatteryCapacity);
          UpdateStatus = UPDATE_LOW_BATTERY;
          FailureReason = NOT_UPDATED;
          goto ErrorExit;  
        } // End if (BatteryCapacity < LOW_BATTERY)
      } //End if !EFI_ERROR(Status)
    } // End if (BatteryStatus && BIT3)
  }  
#endif //EIP175650 

  Status = PreUpdateCheck(&PwrStatus);
  if( Status == EFI_ABORTED) {
    FWPrompt (L"Unable to detect power status. Continue to dispatch the capsule.\r\n");
  } else if(Status == EFI_NOT_READY) {
    //
    //Battery lower than 25%
    //
    FWPrompt (L"Battery level too low. Firmware update not allowed.\r\n");
    if(PwrStatus == PRE_CHECK_LOW_BAT) {
      UpdateStatus = UPDATE_LOW_BATTERY;
      FailureReason = NOT_UPDATED;
      goto ErrorExit;
    }
  } else {
    FWPrompt (L"Pre update check succeed.\r\n");
  }

  //
  //Step 1.2 -- Load Update Image from the FOTA capsule
  //
  SfihHdr=AllocateZeroPool(sizeof(SYS_FW_SFIH_HEADER));
  if (SfihHdr == NULL) {
    FWPrompt (L"Sys FW update DXE fails to Allocate Memory.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }
  
  IfwiAuthHdr = AllocateZeroPool(sizeof(IFWI_AUTH_HEADER));

  Status = LoadDFUImage((VOID **) &FwBinaryBuf, &FwBinarySize);

  if(EFI_ERROR(Status)) {
    FWPrompt (L"Sys FW update DXE fails to load DFU image.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }

  //
  //Step 1.3 -- Capsule payload verification.
  //DXE is verified by secure boot trust chain so only verify payload.
  //

  Status = GetSysFwLayOutInfo(FwBinaryBuf, FwBinarySize, SfihHdr, IfwiAuthHdr);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Sys FW image verification failure.\n");
    UpdateStatus = UPDATE_AUTH_ERROR;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }

  FWPrompt (L"Continue to process the capsule driver.\r\n");

  VarSize = SfihHdr->PDRSize;
  VarBuf = (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->PDROffset);

//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
#ifdef INTEL_SPI_PROTOCOL
  Status = gBS->LocateProtocol(
                  &gEfiSpiProtocolGuid,
                  NULL,
                  (VOID **)&mSpiProtocol
                  );
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Sys FW capsule cannot locate SPI protocol.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }
#else	//AMI Flash Protocol
  Status = gBS->LocateProtocol(&gFlashProtocolGuid, NULL, &Flash);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Sys FW capsule cannot locate gFlashProtocol protocol.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }
#endif  //INTEL_SPI_PROTOCOL
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<

  //
  // Clear Memory Config parameter
  //
  Status = gRT->SetVariable (
                  EfiMemoryConfigVariable,
                  &gEfiVlv2VariableGuid,
                  (EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS),
                  0,
                  NULL
                  );
  
  PcdSetBool(PcdBootState, TRUE);

  //
  //Step 1.4 -- Check BIOS fault tolerant area existence, and update/current image layout.
  //
  Status = GetSecPlatformInfo(&SecPlatformInfo);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to get alternative bios limit and key manifest ID from TXE.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }

  if(SecPlatformInfo.AlternativeBiosLimit ==0) { //Update is not fault tolerant. Will not go thru any fault tolerant path.
    bFaultTolerant = FALSE;
  }

  if(SecPlatformInfo.AlternativeBiosLimit > 0x7FFFFF) {
    FWPrompt (L"Invalid alternative bios limit.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }

  if(SecPlatformInfo.WithKeyManifest == FALSE) {
    bHasKeyManifest = FALSE;
  }

  FWPrompt (L"Alternative BIOS limit is :%x.\r\n", SecPlatformInfo.AlternativeBiosLimit);

  CurrentImageInfo = AllocateZeroPool(sizeof(BIOS_IMAGE_INFO));
  UpdateImageInfo = AllocateZeroPool(sizeof(BIOS_IMAGE_INFO));

  if(CurrentImageInfo == NULL || UpdateImageInfo == NULL) {
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }

  Status = GetCurrentBIOSImageInfo(CurrentImageInfo);
  if(EFI_ERROR(Status)) { //Failing to get current bios image info, we will fall back to default layout
    FreePool(CurrentImageInfo);
    CurrentImageInfo = NULL;
  }

  FWPrompt (L"FMI authenticating the image.\r\n");

  //
  //Step #1.5 Verify IBB w/ FMI before proceeding. FMI interface will check existence of manifest by itself, not neccessary to explicitly
  //set IBB size according to secure boot status register value.
  //

  Status = FmiInit();

  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to init FMI interface.\r\n");
  } else {
    Status = FmiAuthIBB((UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->IBBOffset), SfihHdr->IBBSize);
    //
    //TODO: Production should abort update in case failure.
    //
    if(EFI_ERROR(Status)) {
      FWPrompt (L"IBB Verification failed :%r\r\n", Status);
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = NOT_UPDATED;
      goto ErrorExit;
    } else {
      //
      //TODO: Check the stage 2 image SHA256.
      //
      FWPrompt (L"IBB authentication done.\r\n");
    }
    FreeFmiResource();
  }

  FWPrompt (L"Handling all sections.\r\n");
  //
  //Step #1.6. Call all the handlers to retrieve all sections from the update image.
  //
  //
  IBBSize = SfihHdr->IBBSize;
  UCodeSize = RecoverySize = Stage2Size = SfihHdr->SecondStgSize;  //Initially set the size of Ucode/recovery/stage2 size to the whole Second stage size

  IBBPtr = AllocateZeroPool(IBBSize);
  UCodePtr = AllocateZeroPool(UCodeSize);
  RecoveryPtr = AllocateZeroPool(RecoverySize);
  Stage2Ptr = AllocateZeroPool(Stage2Size);

  Status = FotaHandlerPtr->ProcessIBB((UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->IBBOffset), IBBPtr, &IBBSize);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to process IBB.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }
  if(IBBSize == FOTA_IBB_SIZE_132K) {
    IBBSpiOffset = VLV_IBB_OFFSET_DEFAULT_132K;
  }

//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
  //Status = GetUpdateBIOSImageInfo((UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->IBBOffset),IBBSize, UpdateImageInfo);
  Status = GetUpdateBIOSImageInfo(IBBPtr,IBBSize, &UpdateImageInfo);
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<
  if(EFI_ERROR(Status)) { //Failing to get update bios image info, we will fall back to default layout
    FreePool(UpdateImageInfo);
    UpdateImageInfo = NULL;  //Reference BIOS use this to ensure its default flow is executed.
  }


  Status = FotaHandlerPtr->ProcessStageTwo(UpdateImageInfo, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE + SfihHdr->SecondStgOffset),
           Stage2Ptr, &Stage2Size);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to process stage 2.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }

  Status = FotaHandlerPtr->ProcessUcode(UpdateImageInfo, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE + SfihHdr->SecondStgOffset),
                                        UCodePtr, &UCodeSize);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to process Ucode.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }

  FotaHandlerPtr->ProcessRecovery(UpdateImageInfo, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE + SfihHdr->SecondStgOffset),
                                  RecoveryPtr, &RecoverySize);

  //
  //
  //Reference BIOS will handle variable in PDR region, so not calling variable handler here. Also reference BIOS does not use recovery module.
  //
  //
  //



  /**************************************************
   *  Phase 2: Update  Phase: Sequence:
   *    Backup stage 2, and then:
   *       TXE -> Recovery fault-T->UCode fault-T->IBB fault-T
   *     -> Destroy IBB -> Ucode->Recovery->IBB -> Stage 2
   *
   ***************************************************/

  //
  //Step #2.1 Back up 2nd stage BIOS onto eMMC for fail safe.
  //TODO: OEM needs to decide which portion of the BIOS image needs to be backed up depending on how 2nd stage is recovered by 1st stage or your recovery
  // module design.
  //   Default rereference design loads new update image including microcode + padding + 2nd stage when recovering 2ns stage, so we by default save microcode and 2nd stage
  //
  //
  {
    //if current BIOS does not contian BIOS_IMAGE_INFO, by default we backup image from Microcode to 2nd stage end.

    Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiPartTypeSystemPartGuid, NULL, &HandleArrayCount, &HandleArray);
    if (EFI_ERROR (Status)) {
      FWPrompt (L"Cannot locate FAT partitions.\r\n");
      goto UpdateTxe;
    }


    Status  = gBS->LocateProtocol (&gDxePchPlatformPolicyProtocolGuid, NULL, (VOID **) &DxePlatformPchPolicy);
    if (EFI_ERROR (Status)) {
      FWPrompt (L"Locate the gDxePchPlatformPolicyProtocolGuid Failed.\r\n");
    } else {
      if (DxePlatformPchPolicy->SccConfig->eMMCEnabled) {
        IsEmmc45 = FALSE;
      } else if (DxePlatformPchPolicy->SccConfig->eMMC45Enabled) {
        IsEmmc45 = TRUE;
      } else {
        IsEmmc45 = FALSE;
      }
    }
    //
    // For each system partition...
    //
    for (Index = 0; Index < HandleArrayCount; Index++) {
      Status = gBS->HandleProtocol (HandleArray[Index], &gEfiDevicePathProtocolGuid, (VOID **)&DevicePath);

      if (EFI_ERROR (Status)) {
        continue;
      }
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (-)>>
/*
      if(IsEmmc45) {
        if (CompareMem(DevicePath, &gEmmcDevPath1, sizeof(gEmmcDevPath1) - 4) != 0) {
          continue;
        }
      } else {
        if (CompareMem(DevicePath, &gEmmcDevPath0, sizeof(gEmmcDevPath0) - 4) != 0) {
          continue;
        }
      }
*/
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (-)<<

//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (+)>>
      if( IsEmmc45 )
      {
        CompareResult = (UINTN)CompareMem( DevicePath, &gEmmcDevPath1,
                                           sizeof(gEmmcDevPath1) - 4 );
      }
      else
      {
        CompareResult = (UINTN)CompareMem( DevicePath, &gEmmcDevPath0,
                                           sizeof(gEmmcDevPath0) - 4 );
      }

      if( CompareResult )
      {
        if( CompareMem(DevicePath, &gStatDevPath0, sizeof(gStatDevPath0) - 4) != 0 )
            continue;
      }
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (+)<<

      //
      // Get the SFS protocol from the handle
      //
      Status = gBS->HandleProtocol (HandleArray[Index], &gEfiSimpleFileSystemProtocolGuid, (VOID **)&Fs);
      if (EFI_ERROR (Status)) {
        continue;
      }
      //
      // Open the root directory, get EFI_FILE_PROTOCOL
      //
      Status = Fs->OpenVolume (Fs, &Root);
      if (EFI_ERROR (Status)) {
        continue;
      }

      Status = Root->Open (Root, &FileHandle, gStg2BackupFileNames[0].FileName, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, 0);

      if (EFI_ERROR (Status)) {
        continue;
      }

      if(FileHandle == NULL) {
        Status = EFI_UNSUPPORTED;
        FWPrompt (L"Failed to open root dir on eMMC for writing.\r\n");
        UpdateStatus = UPDATE_FAIL_GENERIC;
        FailureReason = NOT_UPDATED;
        goto ErrorExit;
      }


//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (-)>>
/*
      if(UpdateImageInfo == NULL) {
        FileSize = SfihHdr->SecondStgSize - 0x100000 - 0x1F000;           //TODO: OEM needs to customize here. For reference BIOS, remove the part before microcode, and the part if IBB fault tolerant
        //
        //TODO: OEM needs to customize here.
        //
        FileBuffer = AllocateZeroPool(SfihHdr->SecondStgSize - 0x11F000);
        if(FileBuffer == NULL) {
          UpdateStatus = UPDATE_FAIL_GENERIC;
          FailureReason = NOT_UPDATED;
          goto ErrorExit;
        }
        CopyMem(FileBuffer, FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->SecondStgOffset + 0x100000, FileSize);
      }
*/
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (-)<<

//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (+)>>
      //
      //For FOTA fault tolerant, we save whole BIOS image from capsule to eMMC.
      //
   
      FileSize = SfihHdr->IBBSize + SfihHdr->SecondStgSize;
      FileBuffer = AllocateZeroPool(FileSize);
      if(FileBuffer == NULL){
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = NOT_UPDATED;
      goto ErrorExit;
      }
   
      CopyMem(FileBuffer, FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->SecondStgOffset, SfihHdr->SecondStgSize);
      FileBuffer = (VOID*)((UINT8*)FileBuffer + SfihHdr->SecondStgSize);
      CopyMem(FileBuffer, FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->IBBOffset, SfihHdr->IBBSize);
      FileBuffer = (VOID*)((UINT8*)FileBuffer - SfihHdr->SecondStgSize);
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (+)<<
      
      Status =  FileHandleWrite(FileHandle,&FileSize,FileBuffer);
      if(EFI_ERROR (Status)) {
        UpdateStatus = UPDATE_FAIL_GENERIC;
        FailureReason = NOT_UPDATED;
        goto ErrorExit;
      }

      FWPrompt (L"Backed up second stage for seamless recovery. File size is: 0x%x\n", FileSize);

      Status =  FileHandleClose (FileHandle);

      if(EFI_ERROR (Status)) {
        UpdateStatus = UPDATE_FAIL_GENERIC;
        FailureReason = NOT_UPDATED;
        goto ErrorExit;
      }

    }// end of for

    FWPrompt (L"firmware.bin written. Continue to update Firmware.\n");
    gBS->Stall(100*1000);

  }

UpdateTxe:
  Status = gRT->GetVariable(L"RecoveryCapsule",
                  &BiosCapsuleFromAfuGuid,
                  NULL,
                  &Size,
                  (VOID*)&RecoveryCapsule);

  if (EFI_ERROR(Status)) {
    RecoveryCapsule = 0;
  }

  if (RecoveryCapsule == 1) {
    Status = gRT->SetVariable(L"RecoveryCapsule",
                    &BiosCapsuleFromAfuGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_BOOTSERVICE_ACCESS,
                    0,
                    NULL);

    goto UpdateUCode;
  }
  //
  //Step 2.1 TXE firmware update
  //In production BIOS, it's required to have smaller UPD image pushed to Capsule, and
  //using HECI interaction with TXE to perform TXE FW update.
  //
  {

    PartitionBuffer = FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->SecUpdOffset;

    Status = ExtractCodePartitions(PartitionBuffer,SfihHdr->SecUpdSize,&PartitionsSize, &ReUseSrcBuf,&NewSecVersion);
    if(EFI_ERROR(Status)) {
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = NOT_UPDATED;
      goto ErrorExit;
    }

    FWPrompt (L"\n!!!Updating system firmware. DO NOT UNPLUG POWER or RESET SYSTEM!!!\r\n\n");

    Status = HeciGetLocalFwUpdate(&RuleData);
    if(EFI_ERROR(Status)||RuleData != 1) {
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }

    Status = HeciConnectFwuInterface(&SecAddress, &MaxBufferSize);
    if(EFI_ERROR(Status)) {
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }
   
   Status = HeciSendFwuGetVersionMsg(&OldSecVersion, SecAddress);
   if(EFI_ERROR(Status)){
      FWPrompt (L"Failed to get current sec firmware version.\r\n");
      goto ErrorExit;
   }

   FWPrompt (L"New version: %d.%d.%d.%d.\r\n", NewSecVersion.Major, NewSecVersion.Minor, NewSecVersion.Hotfix, NewSecVersion.Build);
   FWPrompt (L"Old version: %d.%d.%d.%d.\r\n", OldSecVersion.Major, OldSecVersion.Minor, OldSecVersion.Hotfix, OldSecVersion.Build);

    if( (NewSecVersion.Major == OldSecVersion.Major)&&
        (NewSecVersion.Minor == OldSecVersion.Minor)&&
        (NewSecVersion.Hotfix== OldSecVersion.Hotfix)&&
        (NewSecVersion.Build == OldSecVersion.Build)){
      FWPrompt (L"Sec firmware version is the same. Won't update it.\r\n");
      Status = EFI_SUCCESS;
      goto _no_sec_update;
    }

    Status = HeciSendFwuGetOemIdMsg(&SecOemId,SecAddress);

    if(EFI_ERROR(Status)) {
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }

    Status = HeciVerifyOemId(&SecOemId, SecAddress);
    if(EFI_ERROR(Status)) {
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }

    Status = HeciSendFwuStartMsg(PartitionsSize, &SecOemId,SecAddress, MaxBufferSize);
    if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to send FwuStart message to TXE.\n");
      Status = HeciCheckFwuInProgress(&InProgress);
      FWPrompt (L"FWU progress check after FWU start message. In progress: %d.\n", InProgress);
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }

    Status = HeciSendFwuDataMsg(PartitionBuffer, PartitionsSize, SecAddress, MaxBufferSize);
    if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to send Fwu Data messag to TXE.\n");
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }


    Status = HeciSendFwuEndMsg(&ResetType,SecAddress);
    if(EFI_ERROR(Status)) {
      FWPrompt (L"Timeout waiting for FWU_END reply. Continue to query the status for 90 seconds\n");
    }

    Status = HeciDisconnectFwuInterface(SecAddress,MaxBufferSize);
    if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to disconnect from TXE.\r\n");
    }

    //In theory, the update won't take longer than 90 seconds~
    do {
      Status = HeciFwuQueryUpdateStatus(&Percentage, &CurrentStage, &TotalStages, &LastUpdateStatus, &LastResetType, &InProgress);

      if(EFI_ERROR(Status)) {
        FWPrompt (L"Query update status failed.\r\n");
        UpdateStatus = UPDATE_FAIL_GENERIC;
        FailureReason = PARTIAL_UPDATED;
        goto ErrorExit;
      }

      FWPrompt (L"Fw update in progress- Percentage:%d%%, Stage:%d/%d, LastUpdateStatus:%d.\r", Percentage, CurrentStage, TotalStages, LastUpdateStatus);
      gBS->Stall(500*1000);
      LoopCount++;
    } while(Percentage<100 &&  LoopCount<180);

  }//End of TXE update

_no_sec_update:

  //
  //Step 2.2 Update new recovery module to alt_bios_limit  if any. BIOS must be able to know it's booting from recovery flow via Secure_boot_Status register
  //   and 1st stage needs to load recovey module from the relative address.
  //
  //Orginal:
  //
  //------------------------Alt_Bios_limit
  //
  //-------------------R Module SpiOffset---
  //     R Module
  //-------------------------------------
  //    IBB+keyManifest
  //-------------------------------------
  //
  //
  //After update:
  //
  //-------------------
  //R Module(new)
  //-------------------
  //IBB+KeyManifest(new)
  //-------------------Alt_Bios_Limit
  //
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (-) >>
/*
  if((UpdateImageInfo != NULL) && bFaultTolerant && (RecoveryPtr != NULL) && (RecoverySize != 0)) {
    SpiRegionType = EnumSpiRegionAll;
    //
    //TODO: OEM please cutomize here accorindg to SPI size. Also, OEM needs to ensure this recovery region is reserved and never overlapp with BIOS 2nd stage.
    //
    RegionOffset = (SecPlatformInfo.AlternativeBiosLimit + 1) - IBBSize - RecoverySize - (IBBSpiOffset - UpdateImageInfo->RecoveryModuleInfo.SpiOffset);
    FWPrompt (L"Preparing to update recovery module fault tolerant region.\r\n");
    Status = BIOSFlashEx(RecoverySize, RecoveryPtr, SpiRegionType, RegionOffset);
    if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to update the recovery module.\r\n");
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }
  }
*/
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (-) <<

  //
  //Step #2.3. Update IBB fault tolerant area  with new IBB-> Corruption at this stage, BIOS is still booting from IBB main block, so old recovery module/ucode needs to be used.
  //
  if(bFaultTolerant) {
#ifdef INTEL_SPI_PROTOCOL //AMI_OVERRIDE - EIP153486 Fault Tolerant Function
    SpiRegionType = EnumSpiRegionAll;
    RegionOffset = SecPlatformInfo.AlternativeBiosLimit + 1 -IBBSize ;  //TODO: OEM please customize here according to SPI size.
    FWPrompt (L"Preparing to update IBB fault tolerant area. Region offset is :0x%x.\n", RegionOffset);

    Status = BIOSFlashEx(IBBSize, IBBPtr, SpiRegionType, RegionOffset);
    if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to flash IBB fault tolerant area:%r\n", Status);
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    } else {

      Status = BIOSVerifyEx(IBBSize, IBBPtr, SpiRegionType, RegionOffset);
      if(EFI_ERROR(Status)) {
        UpdateStatus = UPDATE_FAIL_GENERIC;
        FailureReason = PARTIAL_UPDATED;
        goto ErrorExit;
      }
    }

    if (VarSize < (256 * 1024)) {
      Status = gRT->SetVariable (
                      L"SetupDefault",
                      &gEfiNormalSetupGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      VarSize,
                      VarBuf);
      Status = gRT->SetVariable (
                      L"Setup",
                      &gEfiNormalSetupGuid,
                      EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                      VarSize,
                      VarBuf);
					  
    }
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
#else
    // AMI Flash protocol.
    RegionOffset = SecPlatformInfo.AlternativeBiosLimit - 0xF + FULL_FLASH_BASE_ADDRESS;
    if (*(UINT32*)RegionOffset == 0xFFFFFFFF) {
      FWPrompt (L"AlternativeBiosLimit:0x%x.\n", SecPlatformInfo.AlternativeBiosLimit);
      RegionOffset = (SecPlatformInfo.AlternativeBiosLimit + 1) + FULL_FLASH_BASE_ADDRESS - IBBSize;    //4G base.
      FWPrompt (L"Preparing to update IBB fault tolerant area. Region offset is :0x%x.\n", RegionOffset);
      Status = Flash->Erase((VOID*)RegionOffset, IBBSize);
      FWPrompt (L"Erase spi offset %xh size %xh and status is %r.\n", RegionOffset, IBBSize, Status);
      Status = Flash->Write((VOID*)RegionOffset, IBBSize, IBBPtr);
      FWPrompt (L"Write spi offset %xh size %xh and status is %r.\n", RegionOffset, IBBSize, Status);
      if(EFI_ERROR(Status)) {
        FWPrompt (L"Failed to flash IBB fault tolerant area:%r\n", Status);
        UpdateStatus = UPDATE_FAIL_GENERIC;
        FailureReason = PARTIAL_UPDATED;
        goto ErrorExit;
      }
    }
#endif  //INTEL_SPI_PROTOCOL
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<

    FWPrompt (L"IBB fault tolerant area updated.updated area size is: 0x%x.\n", IBBSize);
    gBS->Stall(100*1000);
  }

//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (+)>>
  if((UpdateImageInfo != NULL) && bFaultTolerant && (RecoveryPtr != NULL) && (RecoverySize != 0)) {
#ifdef INTEL_SPI_PROTOCOL
    SpiRegionType = EnumSpiRegionAll;
    //
    //TODO: OEM please cutomize here accorindg to SPI size. Also, OEM needs to ensure this recovery region is reserved and never overlapp with BIOS 2nd stage.
    //
    RegionOffset = (SecPlatformInfo.AlternativeBiosLimit + 1) - IBBSize - RecoverySize - (IBBSpiOffset - UpdateImageInfo->RecoveryModuleInfo.SpiOffset);  
    FWPrompt (L"Preparing to update recovery module fault tolerant region.\r\n");
    Status = BIOSFlashEx(RecoverySize, RecoveryPtr, SpiRegionType, RegionOffset);
    if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to update the recovery module.\r\n");
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }
#else
    RegionOffset = PcdGet32 (PcdFlashMicroCodeBackupAddress);
    if( ((EFI_FIRMWARE_VOLUME_HEADER*)RegionOffset)->Signature != EFI_FVH_SIGNATURE || 
        ((EFI_FIRMWARE_VOLUME_HEADER*)RegionOffset)->FvLength != UCodeSize )
    {
      RegionOffset = UpdateImageInfo->RecoveryModuleInfo.SpiOffset+UpdateImageInfo->RecoveryModuleInfo.Size+FLASH_BASE_ADDRESS;
      FWPrompt (L"Preparing to update recovery module fault tolerant area. Region offset is :0x%x.\n", RegionOffset);

      Status = Flash->Erase((VOID*)RegionOffset, RecoverySize);
      FWPrompt (L"Erase spi offset %xh size %xh and status is %r.\n", RegionOffset, RecoverySize, Status);

      Status = Flash->Write((VOID*)RegionOffset, RecoverySize, RecoveryPtr);
      FWPrompt (L"Write spi offset %xh size %xh and status is %r.\n", RegionOffset, RecoverySize, Status);

      if(EFI_ERROR(Status)) {
        FWPrompt (L"Failed to flash Recovery Fault-tolerant area:%r\n", Status);
        UpdateStatus = UPDATE_FAIL_GENERIC;
        FailureReason = PARTIAL_UPDATED;
        goto ErrorExit;
      }
    }
#endif
  }
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (+)<<
  //
  //Step #2.4. Update Ucode fault tolerant area -> Corruption at this stage, BIOS is still booting from IBB main area, and loading Microcode/recovery module from their main area
  //TODO: OEM BIOS layout should ensure this Ucode fault tolerant area does not overlap with any other BIOS ingredients.
  //
#ifdef INTEL_SPI_PROTOCOL //AMI_OVERRIDE - EIP153486 Fault Tolerant Function
  if((UpdateImageInfo != NULL) && bFaultTolerant && (UCodePtr != NULL)) {
    SpiRegionType = EnumSpiRegionAll;
    RegionOffset = UpdateImageInfo->UCodeInfo.SpiOffset;
    RegionOffset = UpdateImageInfo->UCodeInfo.SpiOffset - (VLV_SPI_REGION_SIZE - (SecPlatformInfo.AlternativeBiosLimit + 1));

    Status = BIOSFlashEx(UCodeSize, UCodePtr, SpiRegionType, RegionOffset);
    if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to flash Ucode fault tolerant area:%r\n", Status);
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }
  }
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
#else
  // AMI Flash protocol
  if((UpdateImageInfo != NULL) && bFaultTolerant && (UCodePtr != NULL)) {
    RegionOffset = PcdGet32 (PcdFlashMicroCodeBackupAddress);
    if( ((EFI_FIRMWARE_VOLUME_HEADER*)RegionOffset)->Signature != EFI_FVH_SIGNATURE || 
        ((EFI_FIRMWARE_VOLUME_HEADER*)RegionOffset)->FvLength != UCodeSize ) {
      FWPrompt (L"Preparing to update UCODE fault tolerant area. Region offset is :0x%x.\n", RegionOffset);
      
      Status = Flash->Erase((VOID*)RegionOffset, UCodeSize);
      FWPrompt (L"Erase spi offset %xh size %xh and status is %r.\n", RegionOffset, UCodeSize, Status);

      ((EFI_FIRMWARE_VOLUME_HEADER*)UCodePtr)->Signature = 0xFFFFFFFF;
      Status = Flash->Write((VOID*)RegionOffset, UCodeSize, UCodePtr);
      FWPrompt (L"Write spi offset %xh size %xh and status is %r.\n", RegionOffset, UCodeSize, Status);
      if(EFI_ERROR(Status)) {
        FWPrompt (L"Failed to flash UCODE fault tolerant area:%r\n", Status);
        UpdateStatus = UPDATE_FAIL_GENERIC;
        FailureReason = PARTIAL_UPDATED;
        goto ErrorExit;
      }

      ((EFI_FIRMWARE_VOLUME_HEADER*)UCodePtr)->Signature = EFI_FVH_SIGNATURE;
      Status = Flash->Write((VOID*)RegionOffset, 0x30, UCodePtr);
      if(EFI_ERROR(Status)) {
        FWPrompt (L"Failed to flash Ucode Fault-tolerant area FVH:%r\n", Status);
        UpdateStatus = UPDATE_FAIL_GENERIC;
        FailureReason = PARTIAL_UPDATED;
        goto ErrorExit;
      }
    }
  }
#endif  //INTEL_SPI_PROTOCOL
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<

  //
  //Step #2.5. Destory the IBB main area, so if update stops after this point, IBB fault tolerant will be loaded. This is important because our next step is to update microcode
  //  and recovery module main area.
  //
#ifdef INTEL_SPI_PROTOCOL //AMI_OVERRIDE - EIP153486 Fault Tolerant Function
  SpiRegionType = EnumSpiRegionAll;
  RegionOffset = VLV_IBB_OFFSET_DEFAULT_128K; //we always destory the IBB manifest here.

  FWPrompt (L"Destroying IBB main area before updating UCode and Recovery Module.\r\n");
  Status = FlashErase(RegionOffset, SECTOR_SIZE_4KB, SpiRegionType);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to destroy IBB main area.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = PARTIAL_UPDATED;
    goto ErrorExit;
  }
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>  
#else
  // AMI Flash protocol.
  RegionOffset = 0xFFFFFFFF - IBBSize + 1 + 0x1000;    //4G base.
  FWPrompt (L"Preparing to destroy IBB area. Region offset is :0x%x.\n", RegionOffset);
  
  Status = Flash->Erase((VOID*)RegionOffset, IBBSize - 0x1000);
  FWPrompt (L"Erase spi offset %xh size %xh and status is %r.\n", RegionOffset, IBBSize, Status);
  if(EFI_ERROR(Status)){
    FWPrompt (L"Failed to destroy the IBB.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = PARTIAL_UPDATED;
    goto ErrorExit;
  }
#endif  //INTEL_SPI_PROTOCOL
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<

  //
  // *******At this point we're safe to update the Ucode and recovery module(if any)********
  //

UpdateUCode:
  //
  //Step #2.6. Update new UCode to Ucode target offset
  //
#ifdef INTEL_SPI_PROTOCOL //AMI_OVERRIDE - EIP153486 Fault Tolerant Function
  SpiRegionType = EnumSpiRegionAll;
  if(UpdateImageInfo == NULL) {
    RegionOffset = VLV_DEFAULT_FOTA_UCODE_OFFSET + 0x500000;
  } else {
    RegionOffset = UpdateImageInfo->UCodeInfo.SpiOffset;
  }

  Status = BIOSFlashEx(UCodeSize, UCodePtr, SpiRegionType, RegionOffset);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to update UCode.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = PARTIAL_UPDATED;
    goto ErrorExit;
  }
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>  
#else
  // AMI Flash protocol.
  RegionOffset = PcdGet32 (PcdFlashMicroCodeAddress);
  FWPrompt (L"Preparing to update Microcode area. Region offset is :0x%x.\n", RegionOffset);

  Status = Flash->Erase((VOID*)RegionOffset, UCodeSize);
  FWPrompt (L"Erase spi offset %xh size %xh and status is %r.\n", RegionOffset, UCodeSize, Status);

  ((EFI_FIRMWARE_VOLUME_HEADER*)UCodePtr)->Signature = 0xFFFFFFFF;
  Status = Flash->Write((VOID*)RegionOffset, UCodeSize, UCodePtr);
  FWPrompt (L"Write spi offset %xh size %xh and status is %r.\n", RegionOffset, UCodeSize, Status);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to update the UCode area.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = PARTIAL_UPDATED;
    goto ErrorExit;
  }

  ((EFI_FIRMWARE_VOLUME_HEADER*)UCodePtr)->Signature = EFI_FVH_SIGNATURE;
  Status = Flash->Write((VOID*)RegionOffset, 0x30, UCodePtr);
#endif  //INTEL_SPI_PROTOCOL
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<

  FWPrompt (L"Ucoded updated. continue to update recovery module and IBB.\r\n");
  //
  //Step #2.7 Update new recovery module(if any)
  //
#ifdef INTEL_SPI_PROTOCOL //AMI_OVERRIDE - EIP153486 Fault Tolerant Function
  if((UpdateImageInfo != NULL) && (RecoveryPtr != NULL) && (RecoverySize != 0)) {
    SpiRegionType = EnumSpiRegionAll;
    RegionOffset = UpdateImageInfo->RecoveryModuleInfo.SpiOffset;
    FWPrompt (L"Preparing to update recovery modules.\r\n");
    Status = BIOSFlashEx(RecoverySize, RecoveryPtr, SpiRegionType, RegionOffset);
    if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to update the recovery module.\r\n");
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }
  }
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>  
#else
  // AMI Flash protocol.
  if((UpdateImageInfo != NULL) && (RecoveryPtr != NULL) && (RecoverySize != 0)) {
    RegionOffset = UpdateImageInfo->RecoveryModuleInfo.SpiOffset+FLASH_BASE_ADDRESS;
  }
  //If VERIFY_BOOT_SUPPORT token is disable, UpdateImageInfo will be null.
  //But we must flash the FV_BB.
  else if( (UpdateImageInfo == NULL) && (RecoveryPtr != NULL) && (RecoverySize != 0) ) {
    RegionOffset = FV_BB_OFFSET + FLASH_BASE_ADDRESS;  //<2014/02/13*>
  }    
  FWPrompt (L"Preparing to update FV_BB area. Region offset is :0x%x.\n", RegionOffset);

  Status = Flash->Erase((VOID*)RegionOffset, RecoverySize);
  FWPrompt (L"Erase spi offset %xh size %xh and status is %r.\n", RegionOffset, RecoverySize, Status);

  Status = Flash->Write((VOID*)RegionOffset, RecoverySize, RecoveryPtr);
  FWPrompt (L"Write spi offset %xh size %xh and status is %r.\n", RegionOffset, RecoverySize, Status);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to update the recovery module.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = PARTIAL_UPDATED;
    goto ErrorExit;
  }

#endif  //INTEL_SPI_PROTOCOL
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<

  //
  //Step #2.8. Update IBB area -> At this stage, if corruption happens, the fault tolerant IBB must be aware that it's booting from alternative BIOS limit
  //   add load Microcode from the UCode fault tolerant area. The fault tolerant IBB must also load recovery module from the recovery fault tolerant area.
  //
  //Note: OEM BIOS must support above requirement by detecting secure boot status register bit 1(recovery flow bit) in both FlatIa32.asm and MPCpu driver
  //   to load correct Microcode.
  //
#ifdef INTEL_SPI_PROTOCOL //AMI_OVERRIDE - EIP153486 Fault Tolerant Function
  SpiRegionType = EnumSpiRegionAll;
  RegionOffset = VLV_SPI_REGION_SIZE- IBBSize;
  FWPrompt (L"Preparing to update IBB area. Region offset is :0x%x.\n", RegionOffset);

  FlashRetry=1;

_ibb_retry:

  Status = BIOSFlashEx(IBBSize, IBBPtr, SpiRegionType, RegionOffset);
  if(EFI_ERROR(Status)) {
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = PARTIAL_UPDATED;
    goto ErrorExit;
  } else {
    Status = BIOSVerifyEx(IBBSize, IBBPtr, SpiRegionType, RegionOffset);
    if(EFI_ERROR(Status)) {
      //
      //Give it another retry before we exit.
      if(FlashRetry ==1) {
        gBS->Stall(100*1000);
        goto _ibb_retry;
      }
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }
  }
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>  
#else
  // AMI Flash protocol
  RegionOffset = 0xFFFFFFFF - IBBSize + 1;    //4G base.
  FWPrompt (L"Preparing to update IBB area. Region offset is :0x%x.\n", RegionOffset);

  FlashRetry=1;

_ibb_retry_ami:

  Status = Flash->Erase((VOID*)RegionOffset, IBBSize);
  FWPrompt (L"Erase spi offset %xh size %xh and status is %r.\n", RegionOffset, IBBSize, Status);

  Status = Flash->Write((VOID*)RegionOffset, IBBSize, IBBPtr);
  FWPrompt (L"Write spi offset %xh size %xh and status is %r.\n", RegionOffset, IBBSize, Status);

  if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to flash IBB area:%r\n", Status);
      if(FlashRetry ==1) {
          FlashRetry=0;
          gBS->Stall(100*1000);
          goto _ibb_retry_ami;
      }
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
  }
#endif  //INTEL_SPI_PROTOCOL
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<
  FWPrompt (L"IBB area updated. size is :0x%x.\n", IBBSize);
  gBS->Stall(100*1000);


  //
  //When we reach this point, IBB is booting from the main area, loading the recovery module and Ucode from default location.
  // we're safe to update BIOS second stage now. if the second stage is corrupted during update, IBB will load 2nd stage from EMMC because
  // it detects a hash mis-match.
  //

  //
  //Step #2.9. Update 2nd stage area
  //
#ifdef INTEL_SPI_PROTOCOL //AMI_OVERRIDE - EIP153486 Fault Tolerant Function
  SpiRegionType = EnumSpiRegionAll;
  if(UpdateImageInfo == NULL) { //Reference BIOS does not incoude BIOS_IMAGE_INFO in OEM data area
    RegionOffset = VLV_DEFAULT_FOTA_STGTWO_OFFSET + 0x500000;
  } else {
    RegionOffset = UpdateImageInfo->StageTwoInfo.SpiOffset;
  }
  FWPrompt (L"Preparing to update 2nd stage area. Region offset is :0x%x.\n", RegionOffset);


  Status = BIOSFlashEx(Stage2Size, Stage2Ptr, SpiRegionType, RegionOffset);
  if(EFI_ERROR(Status)) {
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = PARTIAL_UPDATED;
    goto ErrorExit;
  } else {
    Status = BIOSVerifyEx(Stage2Size, Stage2Ptr, SpiRegionType, RegionOffset);
    if(EFI_ERROR(Status)) {
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }
  }
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function >>
#else
  // AMI Flash protocol.
  if(UpdateImageInfo == NULL) {
    RegionOffset = FV_MAIN_OFFSET + FLASH_BASE_ADDRESS;
  }else{
    RegionOffset = UpdateImageInfo->StageTwoInfo.SpiOffset + FLASH_BASE_ADDRESS;
  }
  FWPrompt (L"Preparing to update FV_MAIN area. Region offset is :0x%x.\n", RegionOffset);

  Status = Flash->Erase((VOID*)RegionOffset, Stage2Size);
  FWPrompt (L"Erase spi offset %xh size %xh and status is %r.\n", RegionOffset, Stage2Size, Status);

  Status = Flash->Write((VOID*)RegionOffset, Stage2Size, Stage2Ptr);
  FWPrompt (L"Write spi offset %xh size %xh and status is %r.\n", RegionOffset, Stage2Size, Status);

  if(EFI_ERROR(Status))
  {
      FWPrompt (L"Failed to flash FV_MAIN area:%r\n", Status);
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
  }
#endif  // INTEL_SPI_PROTOCOL
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function <<

  FWPrompt (L"2nd stage area updated. update size is :0x%x.\n", Stage2Size);

//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (+) >>
  //
  // After update BIOS, delete IBB bacup region
  //

  if(bFaultTolerant) {
    RegionOffset = (SecPlatformInfo.AlternativeBiosLimit + 1) + FULL_FLASH_BASE_ADDRESS - IBBSize;    //4G base.
    FWPrompt (L"Preparing to delete IBB fault tolerant area. Region offset is :0x%x.\n", RegionOffset);

    Status = Flash->Erase((VOID*)RegionOffset, IBBSize);
    FWPrompt (L"Erase spi offset %xh size %xh and status is %r.\n", RegionOffset, IBBSize, Status);

    if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to delete IBB fault tolerant area:%r\n", Status);
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }
  }
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (+) <<

/*
  //
  // Check shell if it needs to be updated.
  //
  Status = gDS->ProcessFirmwareVolume (
                  (VOID *)(UINTN)PcdGet32(PcdFlashFvShellBase),
                  PcdGet32(PcdFlashFvShellSize),
                  &FvProtocolHandle
                  );

  if (EFI_ERROR(Status)) {
    if (UpdateImageInfo == NULL) {
      SpiRegionType = EnumSpiRegionAll;
      RegionOffset = VLV_DEFAULT_FOTA_STGTWO_OFFSET + VLV_DEFAULT_FOTA_STGTWO_SIZE + 0x500000;
      FWPrompt (L"Preparing to update shell. Region offset is :0x%x.\n", RegionOffset);

      ShellPtr = FwBinaryBuf + IFWI_AUTH_HEADER_SIZE + SfihHdr->SecondStgOffset + VLV_DEFAULT_FOTA_STGTWO_OFFSET + VLV_DEFAULT_FOTA_STGTWO_SIZE;
      ShellSize = PcdGet32(PcdFlashFvShellSize);
      Status = BIOSFlashEx(ShellSize, ShellPtr, SpiRegionType, RegionOffset);
      if(EFI_ERROR(Status)) {
        UpdateStatus = UPDATE_FAIL_GENERIC;
        FailureReason = PARTIAL_UPDATED;
        goto ErrorExit;
      } else {
        Status = BIOSVerifyEx(ShellSize, ShellPtr, SpiRegionType, RegionOffset);
        if(EFI_ERROR(Status)) {
          UpdateStatus = UPDATE_FAIL_GENERIC;
          FailureReason = PARTIAL_UPDATED;
          goto ErrorExit;
        }
      }

      FWPrompt (L"Shell updated. Update size is :0x%x.\n", ShellSize);
    } else {
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
    }
  }
*/
ErrorExit:
  if (FwBinaryBuf != NULL) {
    FreePool(FwBinaryBuf);
  }

  if(SfihHdr != NULL) {
    FreePool(SfihHdr);
  }

  if(IfwiAuthHdr != NULL) {
    FreePool(IfwiAuthHdr);
  }

  if(CurrentImageInfo != NULL) {
    FreePool(CurrentImageInfo);
  }

  if(UpdateImageInfo != NULL) {
    FreePool(UpdateImageInfo);
  }

  if(IBBPtr != NULL) {
    FreePool(IBBPtr);
  }

  if(Stage2Ptr != NULL) {
    FreePool(Stage2Ptr);
  }

  if(UCodePtr != NULL) {
    FreePool(UCodePtr);
  }

  if(RecoveryPtr != NULL) {
    FreePool(RecoveryPtr);
  }

  if(FileBuffer != NULL) {
    FreePool(FileBuffer);
  }
  //According to the result, report status to Capsule service.
  ReportUpdateStatus(UpdateStatus, FailureReason);

  FWPrompt (L"Updating ESRT table.\r\n");
  EsrtOp->EsrtGetFwEntryByGuid(mSystemFirmwareGuid, &FwResourceEntry);
  if(UpdateStatus == UPDATE_SUCCESS) {
    FwResourceEntry.LastAttemptStatus = 0;
    FwResourceEntry.FwVersion = FwResourceEntry.LastAttemptVersion;
  } else {
    FwResourceEntry.LastAttemptStatus = UpdateStatus; //Keep version unchanged.
  }

  Status = EsrtOp->EsrtUpdateTableEntryByGuid(mSystemFirmwareGuid, &FwResourceEntry);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to update ESRT table after update.\r\n");
    return Status;
  }


  if((UpdateStatus == EFI_SUCCESS) && (Status == EFI_SUCCESS) && (VarSize > 0)) {
    FWPrompt (L"Continue to update the variable of size: %d\r\n", VarSize);
    //
    //If the PDR part of Sfih includes data, currently GenSysFwUpdImage tool extracts "SetupDefault" from the update image into PDR part;
    //Be noted for production SetupDefault/Setup variable should not be changed across BIOS updates.
    //
    if ((VarSize < (256 * 1024)) && (bFaultTolerant == FALSE)) { //This is a setup setting update.
      Status = gRT->SetVariable (
                 L"SetupDefault",
                 &gEfiNormalSetupGuid,
                 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 VarSize,
                 VarBuf);
      Status = gRT->SetVariable (
                 L"Setup",
                 &gEfiNormalSetupGuid,
                 EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                 VarSize,
                 VarBuf);

    }

    PcdSetBool(PcdBootState, TRUE);

    //
    // When we reach this point, the capsule process is finished successfully.
    // Create the Variable "CapsuleXXXX" as defined in UEFI spec 7.5.6, version 2.4.
    //
    Size = sizeof (EFI_CAPSULE_PROCESSING_RESULT);
    Status = gRT->GetVariable (
                    L"Capsule0000",
                    &gEfiCapsuleReportGuid,
                    NULL, 
                    &Size,
                    &CapsuleProcessingResult);
    if (Status == EFI_SUCCESS) {
      Status = gRT->SetVariable (
                      L"Capsule0000",
                      &gEfiCapsuleReportGuid,
                      0,
                      0,
                      NULL
                      );
    }
  
    CapsuleProcessingResult.VariableTotalSize = sizeof (CapsuleProcessingResult);
    CapsuleProcessingResult.CapsuleGuid = mSystemFirmwareGuid;
    gRT->GetTime ((EFI_TIME *)&CapsuleProcessingResult.CapsuleProcessed.Year, NULL);
    CapsuleProcessingResult.CapsuleStatus = EFI_SUCCESS;
    Status = gRT->SetVariable (
                    L"Capsule0000",
                    &gEfiCapsuleReportGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    sizeof (CapsuleProcessingResult),
                    &CapsuleProcessingResult
                    );
    if (Root != NULL) {
      Status = Root->Open (Root, &FileHandle, CAP_BACKUP_BINARY, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, 0);
      if (!EFI_ERROR (Status)) {
        Status = FileHandleDelete(FileHandle);
      }
    }
  }

  FreePool(FwEntryBuffer);

//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (+) >>
  for( Index = 0; Index < HandleArrayCount; Index++ )
  {
    Status = gBS->HandleProtocol( HandleArray[Index],
                                  &gEfiSimpleFileSystemProtocolGuid,
                                  (VOID**)&Fs );
    if( EFI_ERROR(Status) )
      continue;

    Status = Fs->OpenVolume( Fs, &Root );
    if( EFI_ERROR(Status) )
      continue;
      
    Status = Root->Open( Root, &FileHandle, mFlashFileNames[0].FileName, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, 0 );
    if ( !EFI_ERROR (Status) )
    {
      Status = FileHandleDelete(FileHandle);
    }
          
    Status = Root->Open( Root, &FileHandle, gStg2BackupFileNames[0].FileName, EFI_FILE_MODE_WRITE | EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE, 0 );
    if ( !EFI_ERROR (Status) )
    {
      Status = FileHandleDelete(FileHandle);
    }
  }
//AMI_OVERRIDE - EIP153486 Fault Tolerant Function (+) <<

  gRT->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, NULL);
  CpuDeadLoop();

  return Status;
}

