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
// $Header: /Alaska/SOURCE/Modules/PEI Ram Boot/PeiRamBootDxe.c 9     8/08/12 4:53a Calvinchen $
//
// $Revision: 9 $
//
// $Date: 8/08/12 4:53a $
//**********************************************************************
// Revision History
// ----------------
// $Log: /Alaska/SOURCE/Modules/PEI Ram Boot/PeiRamBootDxe.c $
//
// 9     8/08/12 4:53a Calvinchen
// 1. Changed for ReportFv2.c by Artem's suggestion.
// 2. Fixed System hangs if Memory Mapping Changed with warm boot.
//
// 8     5/04/12 5:03a Calvinchen
// [TAG]  		EIP88796
// [Category]  	Bug Fix
// [Severity]  	Minor
// [Symptom]  	(JP0005-Q208)When define more than two FV which has the
// same CUSTOM_SIZE, GetPhysicalAddress() returns invalid value.
// [RootCause]  	Coding mistake.
// [Solution]  	(JP0005-Q208)When define more than two FV which has the
// same CUSTOM_SIZE, GetPhysicalAddress() returns invalid value.
// [Files]  		PeiRamBootDxe.c
// PeiRamBoot.chm
// PeiRamBoot.cif
//
// 7     2/23/12 6:35a Calvinchen
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
// 6     5/27/11 7:22a Calvinchen
// Delete HobRomImage Variable if Recovery mode.
//
// 5     4/22/11 1:28a Calvinchen
//
// 4     3/22/11 7:52a Calvinchen
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
// 3     12/14/10 2:25a Calvinchen
// Improvement :
// 1. Added an eLink "PeiRamBootList" for fast warm boot support
// (PEI_RAM_BOOT_S3_SUPPORT = 1). If system boots in warm boot state, BIOS
// directly boot to previous copied ROM image in RAM to save time of
// copying ROM.
// 2. Added "PEI_RAM_BOOT_S3_SUPPORT" = "2" for saving runtime memory, it
// only keep necessary PEIM FFS in runtime memory for S3 resume
// improvement.
//
// 2     12/02/10 6:18a Calvinchen
// Bug Fixed : Fixed Update SMBIOS Structures failed with DMI Utility.
//
// 1     10/27/10 2:48a Calvinchen
// Initial Check-in.
//
//
//**********************************************************************
//<AMI_FHDR_START>
//
// Name: PeiRamBootDxe.c
//
// Description: PEI RAM BOOT DXE driver.
//
//<AMI_FHDR_END>
//**********************************************************************
//----------------------------------------------------------------------------
// Includes
// Statements that include other files
#include <AmiDxeLib.h>
#include <PeiRamBoot.h>
#include <Protocol/FirmwareVolumeBlock.h>
#include <Token.h>

//----------------------------------------------------------------------------
// Function Externs

//----------------------------------------------------------------------------
// Local prototypes
typedef struct {
    UINTN                       Base;
    UINTN                       Length;
} LBA_CACHE;

typedef struct {
    MEMMAP_DEVICE_PATH          MemMapDevPath;
    EFI_DEVICE_PATH_PROTOCOL    EndDevPath;
} FV_DEVICE_PATH;


typedef struct {
    UINTN                                 Signature;
    EFI_HANDLE                            Handle;
    FV_DEVICE_PATH                        DevicePath;
    EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL    FwVolBlockInstance;
    UINTN                                 NumBlocks;
    LBA_CACHE                             *LbaCache;
    UINT32                                FvbAttributes;
    EFI_PHYSICAL_ADDRESS                  BaseAddress;
} EFI_FW_VOL_BLOCK_DEVICE;

#define FVB_DEVICE_SIGNATURE       EFI_SIGNATURE_32('_','F','V','B')

#ifndef _CR
#define _CR(Record, TYPE, Field)  ((TYPE *) ((CHAR8 *) (Record) - (CHAR8 *) &(((TYPE *) 0)->Field)))
#endif
#ifndef CR
#define CR(Record, TYPE, Field, Signature)  _CR (Record, TYPE, Field)
#endif
#ifndef EFI_FVB_MEMORY_MAPPED
#define EFI_FVB_MEMORY_MAPPED EFI_FVB2_MEMORY_MAPPED
#endif

#define FVB_DEVICE_FROM_THIS(a) \
  CR(a, EFI_FW_VOL_BLOCK_DEVICE, FwVolBlockInstance, FVB_DEVICE_SIGNATURE)

//EFI_GUID gEfiFirmwareVolumeBlockProtocolGuid = FW_VOLUME_BLOCK_PROTOCOL_GUID;
static EFI_GUID gHobRomImageGuid = ROM_IMAGE_MEMORY_HOB_GUID;
static EFI_GUID gRomImageAddressGuid = ROM_IMAGE_ADDRESS_GUID;

//----------------------------------------------------------------------------
// Local Variables
//----------------------------------------------------------------------------
// Function Definitions
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   IsRecoveryMode
//
// Description: Delete HobRomImage Variable if Recovery Mode.
//
// Input:       None.
//
// Output:      TRUE    - Recovery Mode.
//              FALSE   - not Recovery Mode.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsRecoveryMode (VOID)
{
    static EFI_GUID             guidHob = HOB_LIST_GUID;
    EFI_HOB_HANDOFF_INFO_TABLE  *pHit;
    EFI_PHYSICAL_ADDRESS        PhysicalAddress = 0;
    EFI_STATUS                  Status;
    //EFI_GUID                    RomImageAddressGuid = ROM_IMAGE_ADDRESS_GUID;

    pHit = GetEfiConfigurationTable(pST, &guidHob);
    //if we are not in recovery mode ==> unload the module
    if (!pHit || ((pHit->BootMode != BOOT_IN_RECOVERY_MODE) && \
                  (pHit->BootMode != BOOT_ON_FLASH_UPDATE))) return FALSE;
    Status = pRS->SetVariable ( L"HobRomImage", \
                                &gRomImageAddressGuid, \
                                EFI_VARIABLE_NON_VOLATILE | \
                                EFI_VARIABLE_BOOTSERVICE_ACCESS | \
                                EFI_VARIABLE_RUNTIME_ACCESS, \
                                0, \
                                &PhysicalAddress );
    return TRUE;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   PeiRamBootDxeInit
//
// Description: DXE Entry Point for PeiRamBoot Driver.
//
// Input:       EFI_HANDLE          - ImageHandle
//              EFI_SYSTEM_TABLE*   - SystemTable
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
PeiRamBootDxeInit (
    IN EFI_HANDLE             ImageHandle,
    IN EFI_SYSTEM_TABLE       *SystemTable
)
{
    EFI_GUID                GuidHob = HOB_LIST_GUID;
    EFI_STATUS              Status;
    //EFI_GUID                RomImageHobGuid = ROM_IMAGE_MEMORY_HOB_GUID;
    //EFI_GUID                RomImageAddressGuid = ROM_IMAGE_ADDRESS_GUID;
    UINTN                   VarSize = 0, NumHandles;
    UINT8                   i, j;
    EFI_PHYSICAL_ADDRESS    HobRomImageAddress = 0;
    VOID                    *p =  NULL;
    EFI_HOB_FIRMWARE_VOLUME	*FvHob =  NULL;
    HOB_ROM_IMAGE           *HobRomImage;
    EFI_HANDLE				*HandleBuffer;
    EFI_FW_VOL_BLOCK_DEVICE *FvbDevice;
    EFI_FIRMWARE_VOLUME_BLOCK_PROTOCOL  *FvBlock;

    InitAmiLib(ImageHandle, SystemTable);
    if (IsRecoveryMode()) return EFI_SUCCESS;
    
    // Get RamBoot Informatoin from HOB.    
    HobRomImage = (HOB_ROM_IMAGE*)GetEfiConfigurationTable (pST, &GuidHob);
    Status = FindNextHobByGuid (&gHobRomImageGuid, (VOID**)&HobRomImage);
    if (EFI_ERROR(Status)) return EFI_UNSUPPORTED;
    if (!HobRomImage->StolenHobMemory) return EFI_SUCCESS;
#if PEI_RAM_BOOT_S3_SUPPORT
    // PEI_RAM_BOOT_S3_SUPPORT = 1 or 2, need to save RamBoot Info to NVRAM.
    if (!HobRomImage->StolenHobMemory) return EFI_SUCCESS;

    // Save Current RAMBoot information for sub-sequencing boot or resume. 
    VarSize = sizeof(EFI_PHYSICAL_ADDRESS);
    Status = pRS->GetVariable ( L"HobRomImage", \
                                &gRomImageAddressGuid, \
                                NULL, \
                                &VarSize, \
                                &HobRomImageAddress );
    if ((EFI_ERROR(Status)) || \
            (HobRomImage->StolenHobMemory != HobRomImageAddress)) {
			// Update RamBoot info to NVRAM if not exist or content changed.
        HobRomImageAddress = HobRomImage->StolenHobMemory;
        VarSize = sizeof(EFI_PHYSICAL_ADDRESS);
        Status = pRS->SetVariable ( L"HobRomImage", \
                                    &gRomImageAddressGuid, \
                                    EFI_VARIABLE_NON_VOLATILE | \
                                    EFI_VARIABLE_BOOTSERVICE_ACCESS | \
                                    EFI_VARIABLE_RUNTIME_ACCESS, \
                                    VarSize, \
                                    &HobRomImageAddress );
    }
#endif
    // Redirect the FV address to RAM and Free memory.
    for (p = GetEfiConfigurationTable (pST, &GuidHob);
            !(FindNextHobByType(EFI_HOB_TYPE_FV,&p)); ) {
        FvHob = (EFI_HOB_FIRMWARE_VOLUME*)p;
        for (i = 0; i < HobRomImage->NumOfFv; i++) {
            if ((FvHob->BaseAddress == HobRomImage->FvInfo[i].MemAddress) && \
                    (FvHob->Length == HobRomImage->FvInfo[i].FvLength)) {
                FvHob->BaseAddress = HobRomImage->FvInfo[i].FvAddress;
#if PEI_RAM_BOOT_S3_SUPPORT != 1
#if PEI_RAM_BOOT_S3_SUPPORT == 2
                if (!HobRomImage->FvInfo[i].IsBootFv)
#endif
                {
                    pBS->FreePages (HobRomImage->FvInfo[i].MemAddress, \
                    HobRomImage->FvInfo[i].NumOfPages);
                }
#endif
                break;
            }
        }
    }

    // Restore the Base Address of Firmware Volume Block Protocol from RAM
    // to ROM.
    Status = pBS->LocateHandleBuffer ( ByProtocol, \
                                       &gEfiFirmwareVolumeBlockProtocolGuid, \
                                       NULL, \
                                       &NumHandles, \
                                       &HandleBuffer    );
    if (EFI_ERROR(Status)) return Status;

    for (i = 0; i < NumHandles; ++i) {
        Status = pBS->HandleProtocol ( HandleBuffer[i], \
                                       &gEfiFirmwareVolumeBlockProtocolGuid, \
                                       &FvBlock );
        if (EFI_ERROR(Status)) continue;
        FvbDevice = FVB_DEVICE_FROM_THIS (FvBlock);
        if (!(FvbDevice->FvbAttributes & EFI_FVB_MEMORY_MAPPED)) continue;
//-TRACE((-1, "FvBaseAddress[%02X] = %08X\n", i, FvbDevice->BaseAddress));
        for (j = 0; j < HobRomImage->NumOfFv; j++) {
            if ((FvbDevice->BaseAddress == HobRomImage->FvInfo[j].MemAddress) && \
                    (FvbDevice->NumBlocks == \
                     (HobRomImage->FvInfo[j].FvLength / FLASH_BLOCK_SIZE))) {
                FvbDevice->BaseAddress = HobRomImage->FvInfo[j].FvAddress;
            }
        }
    }

    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   PeiRamBootSmmInit
//
// Description: PeiRamBoot InSmm Function.
//
// Input:       EFI_HANDLE          - ImageHandle
//              EFI_SYSTEM_TABLE*   - SystemTable
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
PeiRamBootDxeSmmInit (
    IN EFI_HANDLE             ImageHandle,
    IN EFI_SYSTEM_TABLE       *SystemTable
)
{
    return EFI_SUCCESS;
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
//
// Procedure:   PeiRamBootDxeEntry
//
// Description: DXE Entry Point for PeiRamBoot Driver.
//
// Input:       EFI_HANDLE          - ImageHandle
//              EFI_SYSTEM_TABLE*   - SystemTable
//
// Output:      EFI_STATUS
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS
PeiRamBootDxeEntry (
    IN EFI_HANDLE             ImageHandle,
    IN EFI_SYSTEM_TABLE       *SystemTable
)
{
    InitAmiLib(ImageHandle,SystemTable);
    return InitSmmHandlerEx(ImageHandle, \
                            SystemTable, PeiRamBootDxeSmmInit, PeiRamBootDxeInit);
}
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2009, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

