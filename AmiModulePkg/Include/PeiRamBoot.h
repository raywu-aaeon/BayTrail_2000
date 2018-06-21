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
// $Header: /Alaska/SOURCE/Modules/PEI Ram Boot/PeiRamBoot.h 7     8/08/12 4:25a Calvinchen $
//
// $Revision: 7 $
//
// $Date: 8/08/12 4:25a $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name: PeiRamBoot.h
//
// Description: Definition file for PEI Ram Boot.
//
//<AMI_FHDR_END>
//**********************************************************************

#ifndef  PI_SPECIFICATION_VERSION
#define  PI_SPECIFICATION_VERSION   91
#endif

#define FLASH_DEVICE_BASE (0xFFFFFFFF - FLASH_SIZE + 1)

//-extern EFI_GUID gHobRomImageGuid;
//-extern EFI_GUID gRomImageAddressGuid;
//-extern EFI_GUID gRomImageHobGuid;
//-extern EFI_GUID gRomCacheEnablePpiGuid;

#define ROM_IMAGE_MEMORY_HOB_GUID \
{ 0xee2f45d2, 0x5ba4, 0x441e, 0x8a, 0x1d, 0xaa, 0x22, 0xdf, 0xa3, 0xb6, 0xc5 }

#define ROM_IMAGE_ADDRESS_GUID \
{ 0xDDE1BC72, 0xD45E, 0x4209, 0xAB, 0x85, 0x14, 0x46, 0x2D, 0x2F, 0x50, 0x74 }

#define SMBIOS_FLASH_DATA_FFS_GUID \
{ 0xFD44820B, 0xF1AB, 0x41C0, 0xAE, 0x4E, 0x0C, 0x55, 0x55, 0x6E, 0xB9, 0xBD }

#define ROM_CACHE_ENABLE_PPI_GUID \
{ 0x36E835BB, 0x661D, 0x4D37, 0x8D, 0xE5, 0x88, 0x53, 0x25, 0xDA, 0xE9, 0x10 }

#define FLASH_EMPTY_BYTE (UINT8)(-FLASH_ERASE_POLARITY)

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:        HOB_ROM_IMAGE
//
// Description: This structure contains information for ROM Image HOB.
//
// Referrals:
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
typedef struct {
    UINT32                FvAddress;
    UINT32                FvLength;
    UINT32                UsedBytes;
    UINT32                MemAddress;
    UINT16                NumOfPages;
    UINT8                 FvMemReady;
    UINT8                 IsBootFv;
} FV_INFO;

typedef struct {
#if PEI_RAM_BOOT_S3_SUPPORT == 2
    UINT32                PeimAddress;
    UINT32                PeimLength;
#else
    UINT32                FfsAddress;
    UINT8                 FfsLength[3];
    UINT8                 FvIndex;
#endif
} PEIM_FFS_INFO;

typedef struct {
    UINT32                FvAddress;
    UINT32                FvLength;
    UINT32                MemAddress;
} NESTED_FV_INFO;

typedef struct {
    EFI_HOB_GUID_TYPE     EfiHobGuidType;
    BOOLEAN               HobValid;
    UINT32                SmbiosFlashData;
    EFI_PHYSICAL_ADDRESS  StolenHobMemory;
    UINT8                 NumOfFv;
    FV_INFO               FvInfo[8];
    UINT8                 NumOfPeim;
#if PEI_RAM_BOOT_S3_SUPPORT == 2
    PEIM_FFS_INFO         PeimFfsInfo[16];
#else
    PEIM_FFS_INFO         RsvdFfsInfo[16];
#endif
    UINT8                 NestedFvValid;
    UINT8                 NumOfNestedFv;
    NESTED_FV_INFO        NestedFvInfo[4];
} HOB_ROM_IMAGE;

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
