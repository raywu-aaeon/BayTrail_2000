/**

  Copyright (c) 2009 - 2013, Intel Corporation. All rights reserved.<BR>
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

#include <Library/DfuCapsuleLib.h>
#include <Library/HeciMsgLib.h>
#include <Library/FmiLib.h>
#include <Library/Esrt.h>
#include <Protocol/PchExtendedReset.h>

#include "FlashOperation.h"
#include "SysFwUpdate.h"

#define _SHOW_LOG_

EFI_SPI_PROTOCOL  *mSpiProtocol;
UINT8             *mFileBuffer;
UINT8             *PartitionBuffer;
extern EFI_GUID gEfiNormalSetupGuid;
EFI_PCH_EXTENDED_RESET_PROTOCOL  *mPchExtendedResetProtocol;
EFI_GUID mSystemFirmwareGuid = { 0x3bbdb30d, 0xaa9b, 0x4915, { 0x95, 0x03, 0xe4, 0xb8, 0x2f, 0xb6, 0xb6, 0xe9 }};
EFI_GUID mAtml1664FirmwareGuid = { 0x2959D05F, 0xBFD3, 0x48AD, { 0x97, 0x86, 0x84, 0x2F, 0x8A, 0x7D, 0xDF, 0xDE }};



#define  MAX_UPD_SIZE  0x300000

#if defined(_SHOW_LOG_)
#define FWPrompt(...)       Print(__VA_ARGS__)
#define FWPrompt_LOW(...)   ((void) 0)
#else
#define FWPrompt(...)       ((void) 0)
#define FWPrompt_LOW(...)   ((void) 0)
#endif

#pragma pack(1)
typedef struct _FPTHdrV2 {
  UINT32 ReservedQ[4];   //4 DWORD filled with all 0s
  UINT32 Signature;     //$FPT
  UINT32 NumFptEntries;  //Number of FPT entries
  UINT8  HdrVer;
  UINT8  EntryVer;
  UINT8  HdrLen;
  UINT8  CheckSum;
  UINT16 FlashLifeTime;
  UINT16 FlashCycleLmt;
  UINT32 DRAMSize;
  UINT32 FPTFlags;
  UINT32 ReservedS[2];   //2 DWORD reserved.
} FPTHdrV2;

typedef struct _FPTEntryHdrV1 {
  UINT32 PartitionName;       //Unique name of the partition
  UINT32 PartitionOwner;
  UINT32 Offset;              //Offset of the partition from beginning of FPT
  UINT32 Length;              //Length of the partition
  UINT32 TokensOnStart;
  UINT32 MaximumTokens;
  UINT32 ScratchSectors;
  UINT32 Attributes;
} FPTEntryHdrV1;

#pragma pack()

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
  BOOLEAN              WithDesc = FALSE;

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
    WithDesc = TRUE;
  }
  if(WithDesc){
    fpt_hdr = (FPTHdrV2*)(kFileBuffer + 0x1000);
  }else{
    fpt_hdr = (FPTHdrV2*)kFileBuffer;
  }
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
  if(WithDesc){
    BufPtr = kFileBuffer + 0x1000 + fpt_hdr->HdrLen;
    PartitionBuffer += 0x1000;
  }else{
    BufPtr = kFileBuffer + fpt_hdr->HdrLen;
  }

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

  if(FTPRStart<NFTPStart && NFTPStart <MDMVStart && (FTPREnd+1 == NFTPStart) && (NFTPEnd+1 == MDMVStart)) {
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
    if( (FTPRStart == 0) || (NFTPStart == 0) || (MDMVStart == 0)) {
      FWPrompt (L"Missing required partitions for a full update.\r\n");
      return EFI_ABORTED;
    } else {
      PartitionBuffer = AllocateZeroPool(*mDestSize);
      if(PartitionBuffer == NULL) {
        return EFI_ABORTED;
      }

      CopyMem(PartitionBuffer, mFileBuffer+FTPRStart, FTPREnd-FTPRStart +1);
      CopyMem(PartitionBuffer+(FTPREnd-FTPRStart+1), mFileBuffer+NFTPStart, NFTPEnd-NFTPStart+1);
      CopyMem(PartitionBuffer+(FTPREnd-FTPRStart+1)+(NFTPEnd-NFTPStart+1), mFileBuffer+MDMVStart, MDMVEnd-MDMVStart+1);
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

//
//This is the entry of the upddate driver
//
EFI_STATUS
SysUpdateEntry(
  IN EFI_HANDLE                ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
)
{
  EFI_STATUS                            Status;
  UINT32                                UpdateStatus = UPDATE_SUCCESS;
  DFU_FAILURE_REASON                    FailureReason = NO_FAILURE;

  EFI_HANDLE                            *HandleArray;
  UINTN                                 HandleArrayCount;
  UINTN                                 Index = 0;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *Fs;
  EFI_FILE                              *Root;
  EFI_FILE                              *FileHandle;
  VOID                                  *FileBuffer;
  UINTN                                 FileSize;

  UINT8                                 *FwBinaryBuf;

  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;

  SPI_REGION_TYPE                       SpiRegionType;
  UINT64                                FwBinarySize;

  UINTN                                 RegionOffset;
#ifndef _NO_POWER_CHECK_
  UINT8                                 PwrStatus = PRE_CHECK_PASS;
#endif

  UINT8                                 LoopCount = 0;

  UINT32 Percentage = 0;
  UINT32 CurrentStage = 0;
  UINT32 TotalStages = 0;
  UINT32 LastUpdateStatus = 0;
  UINT32 LastResetType = 0;

  UINT8   ReUseSrcBuf = 0;
  UINT32  PartitionsSize = 0;

  UINT32   RuleData = 0;
  UINT32   MaxBufferSize = 0;
  OEM_UUID SecOemId;
  UINT32 ResetType =4;
  UINT8  SecAddress = 0;
  UINT8  InProgress = 0;

  DXE_PCH_PLATFORM_POLICY_PROTOCOL *DxePlatformPchPolicy;
  BOOLEAN                          IsEmmc45 = FALSE;

  ESRT_OPERATION_PROTOCOL   *EsrtOp;
  FW_RES_ENTRY                FwResourceEntry;
  FW_RES_ENTRY                TouchEntry;
  UINT32  VarSize = 0;
  UINT8   *VarBuf = NULL;
  UINT32  ESRT_SIGNATURE = SIGNATURE_32('E','S','R','T');
  FW_RES_ENTRY_LIST        FwEntryList;
  UINTN                   FwEntrySize = sizeof(UINT32) + sizeof(FW_RES_ENTRY)*256;
  UINT8                    *FwEntryBuffer = NULL;
  UINT32                   *FwEntrySignature = NULL;
  PCH_EXTENDED_RESET_TYPES              PchResetType;
  VERSION                               NewSecVersion;
  VERSION                               OldSecVersion;

  PSYS_FW_SFIH_HEADER SfihHdr = NULL;
  PIFWI_AUTH_HEADER IfwiAuthHdr = NULL;


  UINTN FlashRetry=0;


  RegionOffset = 0;
  mFileBuffer = NULL;
  PartitionBuffer = NULL;


  FwBinaryBuf = (UINT8*)NULL;
  FwBinarySize = 0;

  FileHandle = (EFI_FILE*)NULL;
  FileSize = 0;
  FileBuffer = NULL;

  PchResetType.PowerCycle = 0;
  PchResetType.GlobalReset = 1;

  ZeroMem(&FwEntryList, FwEntrySize);

  Status = gBS->LocateProtocol (&gEfiEsrtOperationProtocolGuid, NULL, (VOID **) &EsrtOp);
  if(EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Cannot locate Esrt Operation Protocol.\n"));
    return Status;
  }

#ifndef _NO_POWER_CHECK_
  //
  //If our attempt to check power status failed due to EFI_ABORTED, this
  //will mean we have problem communicating with ULPMC... Handling this as if
  //we don't have any fuel guage/power source detection method. Pass the check
  //directly.
  //
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
  } else { //EFI_SUCCESS
    FWPrompt (L"Pre update check succeed.\r\n");
  }

#endif

  SfihHdr=AllocateZeroPool(sizeof(SYS_FW_SFIH_HEADER));
  if(SfihHdr == NULL) {
	UpdateStatus = UPDATE_NO_RESOURCE;
	FailureReason = NOT_UPDATED;
	goto ErrorExit;
  }
  IfwiAuthHdr = AllocateZeroPool(sizeof(IFWI_AUTH_HEADER));



  //
  //For the system firmware update capsule, we use capsule lib in DFU package for the image.
  //
  Status = LoadDFUImage((VOID **) &FwBinaryBuf, &FwBinarySize);

  if(EFI_ERROR(Status)) {
    FWPrompt (L"Sys FW update DXE fails to load DFU image.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = NOT_UPDATED;

    goto ErrorExit;
  }

  //
  //This will verify the image loaded by current DXE.
  //

  Status = GetSysFwLayOutInfo(FwBinaryBuf, FwBinarySize, SfihHdr, IfwiAuthHdr);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Sys FW image verification failure.\n");
    UpdateStatus = UPDATE_AUTH_ERROR;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }


  //
  //More check on image size, offset, etc...
  //

  if(SfihHdr->IBBSize!=VLV_IBB_REGION_SIZE || (SfihHdr->SecUpdSize >VLV_SEC_REGION_SIZE +0x1000)) {

    FWPrompt (L"Image size checking failure.\r\n");
    Status = EFI_LOAD_ERROR;
    UpdateStatus = UPDATE_INVALID_IMAGE;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
  }

  VarSize = SfihHdr->PDRSize;
  VarBuf = (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->PDROffset);

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
  //
  //Step #0. Store 2nd stage onto eMMC ESP. Stop update process in case any failure.
  //
  Status = gBS->LocateHandleBuffer (ByProtocol, &gEfiPartTypeSystemPartGuid, NULL, &HandleArrayCount, &HandleArray);
  if (EFI_ERROR (Status)) {
    FWPrompt (L"Cannot locate FAT partitions.\r\n");
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = NOT_UPDATED;
    goto ErrorExit;
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

    if(IsEmmc45) {
      if (CompareMem(DevicePath, &gEmmcDevPath1, sizeof(gEmmcDevPath1) - 4) != 0) {
        continue;
      }
    } else {
      if (CompareMem(DevicePath, &gEmmcDevPath0, sizeof(gEmmcDevPath0) - 4) != 0) {
        continue;
      }
    }
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

    FileSize = SfihHdr->SecondStgSize;
    FileBuffer = FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->SecondStgOffset;


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

  //
  //Step #2.0 Verify IBB w/ FMI before proceeding.
  //
  Status = FmiInit();

  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to init FMI interface.\r\n");
  } else {
    Status = FmiAuthIBB((UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->IBBOffset), SfihHdr->IBBSize);

    if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to verify IBB: %r\n", Status);
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }

    FreeFmiResource();
  }
    //
    //This is an real UPD image passed in. use Sec local firmware interface to update it.
    //
    PartitionBuffer = FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->SecUpdOffset;

    Status = ExtractCodePartitions(PartitionBuffer,SfihHdr->SecUpdSize,&PartitionsSize, &ReUseSrcBuf, &NewSecVersion);
    if(EFI_ERROR(Status)) {
      goto ErrorExit;
    }

    Status = HeciGetLocalFwUpdate(&RuleData);
    if(EFI_ERROR(Status)||RuleData != 1) {
      goto ErrorExit;
    }

    Status = HeciConnectFwuInterface(&SecAddress, &MaxBufferSize);
    if(EFI_ERROR(Status)) {
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
  //
  //Step #x. Update TXE region. For 3MB TXE image, TXE is updated directly since IA is granted access in engineering BIOS
  //In production BIOS, it's required to have smaller UPD image pushed to Capsule, and
  //using HECI interaction with TXE to perform TXE FW update.
  //
  if(SfihHdr->SecUpdSize == VLV_SEC_REGION_SIZE) { //Full image provided. Update the whole SEC region then.
    //
    //Send TXE disable HECI to stop TXE.
    //
    Status = HeciHaltTxe();
    gBS->Stall(2000*1000);

    SpiRegionType = EnumSpiRegionAll;
    RegionOffset = 0x1000;
    Status = BIOSFlashEx(VLV_SEC_REGION_SIZE, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->SecUpdOffset), SpiRegionType, RegionOffset);

    if(EFI_ERROR(Status)) {
      FWPrompt (L"Flashing TXE region failed.\n");
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    } else {
      Status = BIOSVerifyEx(VLV_SEC_REGION_SIZE, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->SecUpdOffset), SpiRegionType, RegionOffset);
      if(EFI_ERROR(Status)) {
        FWPrompt (L"Verifying TXE region failed.\n");
        UpdateStatus = UPDATE_FAIL_GENERIC;
        FailureReason = PARTIAL_UPDATED;
        goto ErrorExit;
      }
    }
  } else if(SfihHdr->SecUpdSize == VLV_SEC_REGION_SIZE + 0x1000) { //Full image + Descritpor provided.
    //
    //Send TXE disable HECI to stop TXE.
    //
    Status = HeciHaltTxe();
    gBS->Stall(2000*1000);

    SpiRegionType = EnumSpiRegionAll;
    RegionOffset = 0;
    Status = BIOSFlashEx(VLV_SEC_REGION_SIZE+0x1000, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->SecUpdOffset), SpiRegionType, RegionOffset);

    if(EFI_ERROR(Status)) {
      FWPrompt (L"Flashing TXE region failed.\r\n");
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    } else {
      Status = BIOSVerifyEx(VLV_SEC_REGION_SIZE+0x1000, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->SecUpdOffset), SpiRegionType, RegionOffset);
      if(EFI_ERROR(Status)) {
        FWPrompt (L"Verifying TXE region failed.\r\n");
        UpdateStatus = UPDATE_FAIL_GENERIC;
        FailureReason = PARTIAL_UPDATED;
        goto ErrorExit;
      }
    }
  } else {


    Status = HeciSendFwuGetOemIdMsg(&SecOemId,SecAddress);

    if(EFI_ERROR(Status)) {
      goto ErrorExit;
    }

    Status = HeciVerifyOemId(&SecOemId, SecAddress);
    if(EFI_ERROR(Status)) {
      goto ErrorExit;
    }

    Status = HeciSendFwuStartMsg(PartitionsSize, &SecOemId,SecAddress, MaxBufferSize);
    if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to send FwuStart message to TXE.\n");
      Status = HeciCheckFwuInProgress(&InProgress);
      FWPrompt (L"FWU progress check after FWU start message. In progress: %d.\n", InProgress);
      goto ErrorExit;
    }

    Status = HeciSendFwuDataMsg(PartitionBuffer, PartitionsSize, SecAddress, MaxBufferSize);
    if(EFI_ERROR(Status)) {
      FWPrompt (L"Failed to send Fwu Data messag to  TXE.\n");
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
        goto ErrorExit;
      }

      FWPrompt (L"Fw update in progress- Percentage:%d%%, Stage:%d/%d, LastUpdateStatus:%d.\n", Percentage, CurrentStage, TotalStages, LastUpdateStatus);
      gBS->Stall(500*1000);
      LoopCount++;
    } while(Percentage<100 &&  LoopCount<180);

  }//End of TXE update

_no_sec_update:
  //
  //Step #2. Update IBB fault tolerant area.
  //
  SpiRegionType = EnumSpiRegionAll;
  RegionOffset = VLV_SPI_REGION_SIZE-2*VLV_IBB_REGION_SIZE;
  FWPrompt (L"Preparing to update IBB fault tolerant area. Region offset is :0x%x.\n", RegionOffset);

  Status = BIOSFlashEx(SfihHdr->IBBSize, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->IBBOffset), SpiRegionType, RegionOffset);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to flash IBB fault tolerant area:%r\n", Status);
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = PARTIAL_UPDATED;
    goto ErrorExit;
  } else {

    Status = BIOSVerifyEx(SfihHdr->IBBSize, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->IBBOffset), SpiRegionType, RegionOffset);
    if(EFI_ERROR(Status)) {
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }
  }

  FWPrompt (L"IBB fault tolerant area updated.updated area size is: 0x%x.\n", SfihHdr->IBBSize);
  gBS->Stall(100*1000);

  //
  //Step #3. Update IBB area.
  //

  RegionOffset = VLV_SPI_REGION_SIZE-VLV_IBB_REGION_SIZE;
  FWPrompt (L"Preparing to update IBB area. Region offset is :0x%x.\n", RegionOffset);

  FlashRetry=1;

_ibb_retry:

  Status = BIOSFlashEx(SfihHdr->IBBSize, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->IBBOffset), SpiRegionType, RegionOffset);
  if(EFI_ERROR(Status)) {
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = PARTIAL_UPDATED;
    goto ErrorExit;
  } else {
    Status = BIOSVerifyEx(SfihHdr->IBBSize, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->IBBOffset), SpiRegionType, RegionOffset);
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

  FWPrompt (L"IBB area updated. size is :0x%x.\n", SfihHdr->IBBSize);
  gBS->Stall(100*1000);

  //
  //Step #4. Update 2nd stage area(micro-code + FVMian + FVRecovery2)
  //

  RegionOffset = VLV_SPI_REGION_SIZE - 2*VLV_IBB_REGION_SIZE- SfihHdr->SecondStgSize;
  FWPrompt (L"Preparing to update 2nd stage area. Region offset is :0x%x.\n", RegionOffset);


  Status = BIOSFlashEx(SfihHdr->SecondStgSize, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->SecondStgOffset), SpiRegionType, RegionOffset);
  if(EFI_ERROR(Status)) {
    UpdateStatus = UPDATE_FAIL_GENERIC;
    FailureReason = PARTIAL_UPDATED;
    goto ErrorExit;
  } else {
    Status = BIOSVerifyEx(SfihHdr->SecondStgSize, (UINT8*)(FwBinaryBuf+IFWI_AUTH_HEADER_SIZE+SfihHdr->SecondStgOffset), SpiRegionType, RegionOffset);
    if(EFI_ERROR(Status)) {
      UpdateStatus = UPDATE_FAIL_GENERIC;
      FailureReason = PARTIAL_UPDATED;
      goto ErrorExit;
    }
  }

  FWPrompt (L"2nd stage area updated. update size is :0x%x.\n", SfihHdr->SecondStgSize);


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

  //
  //Since touch firmware update support is not intially in feature list, handle it specially in case current platform does not include this entry.
  //
  //
  Status = EsrtOp->EsrtGetFwEntryByGuid(mAtml1664FirmwareGuid, &TouchEntry);
  if(EFI_ERROR(Status)) { //It does not exist.
    FWPrompt (L"Failed to get Touch firmware entry. Adding it.\r\n");
  }

  TouchEntry.FwClass = mAtml1664FirmwareGuid;
  TouchEntry.FwType = 0x02;
  TouchEntry.FwVersion = 0x01;
  TouchEntry.FwLstCompatVersion = 0;
  TouchEntry.CapsuleFlags = 0;
  TouchEntry.LastAttemptVersion = 0x01;
  TouchEntry.LastAttemptStatus = 0;

#if 1
  Status = EsrtOp->EsrtUpdateTableEntryByGuid(mAtml1664FirmwareGuid, &TouchEntry);
  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to add Touch fimrware update entry.\r\n");
  }
#endif
  //
  //Before we flash the variable block, backup the FwEntry variable to SPI address 0x580000. ESRT driver will restore it from SPI.
  //

  Status = gRT->GetVariable(L"FwEntry",
                    &gEfiVlv2VariableGuid,
                    NULL, 
                    &FwEntrySize,
                    &FwEntryList);

  if(EFI_ERROR(Status)) {
    FWPrompt (L"Failed to get FwEntry from BIOS.\r\n");
  } else {
    FwEntryBuffer = AllocateZeroPool(FwEntrySize + 4);

    if(FwEntryBuffer != NULL) {
      FwEntrySignature = (UINT32*)FwEntryBuffer;
      *FwEntrySignature = ESRT_SIGNATURE;
      CopyMem(FwEntryBuffer+4, &FwEntryList, FwEntrySize);

      SpiRegionType = EnumSpiRegionAll;
      Status = BIOSFlashEx(FwEntrySize+4, FwEntryBuffer, SpiRegionType, 0x580000);

      if((Status == EFI_SUCCESS) && (VarSize > 0)) {
        FWPrompt (L"Backing up EsrtEntry succeed. Continue to update the variable of size: %d\r\n", VarSize);
        //
        //Allow both Setup update or the whole VAR block update.
        //
        if(VarSize < 256*1024) { //This is a setup setting update.
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

        } else {
          Status = BIOSFlashEx(VarSize, VarBuf, SpiRegionType, 0x500000);
        }
        FWPrompt (L"Upate variable block result:%r\r\n", Status);
      }

      FreePool(FwEntryBuffer);
    }
  }
  Status = gBS->LocateProtocol(
                  &gEfiPchExtendedResetProtocolGuid,
                  NULL,
                  (VOID **)&mPchExtendedResetProtocol
                  );
  if(!EFI_ERROR(Status)){
    mPchExtendedResetProtocol->Reset(mPchExtendedResetProtocol, PchResetType);
  }

  CpuDeadLoop();

  return Status;
}

