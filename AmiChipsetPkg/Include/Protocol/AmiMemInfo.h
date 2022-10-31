//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:        AmiMemInfo.h
//
// Description: The header file for AMI Memory Information
//
//<AMI_FHDR_END>
//*************************************************************************

#ifndef _AMI_MEMORY_INFO_
#define _AMI_MEMORY_INFO_

#ifdef __cplusplus
extern "C" {
#endif

#include <token.h>
#include <Library/PcdLib.h>

//#if !defined (_PCD_VALUE_PcdDimmSlotNum)
//#error "Please define gAmiChipsetPkgTokenSpaceGuid.PcdDimmSlotNum in the INF file while including AmiMemInfo.h ! "
//#endif

//#if !defined (_PCD_VALUE_PcdMemoryArrayNum)
//#error "Please define gAmiChipsetPkgTokenSpaceGuid.PcdMemoryArrayNum in the INF file while including AmiMemInfo.h ! "
//#endif

#define EFI_AMI_MEMORY_INFO_GUID \
  {0xa8f960c6, 0x4cc3, 0x4417, 0x8a, 0xd9, 0x2a, 0x3b, 0x3f, 0x80, 0x27, 0xea}

extern EFI_GUID gEfiAmiMemoryInfoGuid;

#define AMI_MEMORY_INFO_VARIABLE L"AmiMemoryInfo"

#pragma pack(push,1)

typedef enum {
    UnknownType,
    DDR1,
    DDR2,
    DDR3,
    DDR4,
    OtherType
} MEMORY_TYPE_INFO;

typedef struct {
    UINT32          Size;                  // MB
    BOOLEAN         DoubleSide;
    BOOLEAN         Ecc;
    UINT8           SpdAddr;
} MEMORY_SLOT_INFO;

typedef struct {
    MEMORY_TYPE_INFO        RamType;
    UINT32                  MaxCapacity;           // MB
    UINT32                  TotalMemory;           // MB
    UINT32                  Speed;                 // Mhz
//  MEMORY_SLOT_INFO        Slot[FixedPcdGet16 (PcdDimmSlotNum)];
    MEMORY_SLOT_INFO        Slot[DIMM_SLOT_NUM];
} AMI_MEMORY_ARRAY_INFO;

typedef struct {
//  AMI_MEMORY_ARRAY_INFO   MemoryArray[FixedPcdGet32 (PcdMemoryArrayNum)];
    AMI_MEMORY_ARRAY_INFO   MemoryArray[MEMORY_ARRAY_NUM];
} AMI_MEMORY_INFO;

#pragma pack(pop)

/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif

#endif

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
