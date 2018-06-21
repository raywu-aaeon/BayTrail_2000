//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/SOURCE/Core/Modules/NVRAM/NVRAM.h 28    11/29/12 5:10p Felixp $
//
// $Revision: 28 $
//
// $Date: 11/29/12 5:10p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	NVRAM.h
//
// Description:	NVRAM Definitions
//
//<AMI_FHDR_END>
//**********************************************************************
#ifndef __NVRAM__H__
#define __NVRAM__H__
#ifdef __cplusplus
extern "C" {
#endif
#include <AmiLib.h>
#include <Flash.h>
#include <Hob.h>
#include <Ffs.h>
//AptioV
#include <NvramElink.h>

#define FLASH_EMPTY_BYTE ((UINT8)FlashEmpty)
#define FLASH_EMPTY_FLAG FLASH_EMPTY_BYTE
#define FLASH_EMPTY_NEXT FlashEmptyNext
#define FLASH_EMPTY_SIGNATURE ((UINT32)FlashEmpty)
#define FLASH_EMPTY_SIZE ((VAR_SIZE_TYPE)FlashEmpty)

#define NVRAM_FLAG_VALID 0x80
#define NVRAM_FLAG_RUNTIME 1
#define NVRAM_FLAG_ASCII_NAME 2
#define NVRAM_FLAG_GUID 4
#define NVRAM_FLAG_DATA_ONLY 8
#define NVRAM_FLAG_EXT_HEDER 0x10
#define NVRAM_FLAG_AUTH_WRITE 0x40
#define NVRAM_FLAG_HARDWARE_ERROR_RECORD 0x20
#define UEFI21_SPECIFIC_NVRAM_FLAGS (NVRAM_FLAG_HARDWARE_ERROR_RECORD | NVRAM_FLAG_AUTH_WRITE)
#define UEFI23_1_AUTHENTICATED_VARIABLE_ATTRIBUTES \
                (EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS | EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS)

#define NVRAM_EXT_FLAG_CHECKSUM 1

#define NVRAM_eFLAG_AUTH_WRITE              EFI_VARIABLE_AUTHENTICATED_WRITE_ACCESS //0x10
#define NVRAM_eFLAG_TIME_BASED_AUTH_WRITE   EFI_VARIABLE_TIME_BASED_AUTHENTICATED_WRITE_ACCESS //0x20

#define ALL_FLAGS (NVRAM_FLAG_VALID | NVRAM_FLAG_RUNTIME |\
				   NVRAM_FLAG_ASCII_NAME | NVRAM_FLAG_GUID|\
				   NVRAM_FLAG_DATA_ONLY | NVRAM_FLAG_EXT_HEDER|\
                   UEFI21_SPECIFIC_NVRAM_FLAGS\
                  )

#define NVAR_SIGNATURE ('N'+('V'<<8)+(('A'+('R'<<8))<<16))//NVAR

// {C0EC00FD-C2F8-4e47-90EF-9C8155285BEC}
#define NVRAM_HOB_GUID \
    { 0xc0ec00fd, 0xc2f8, 0x4e47, { 0x90, 0xef, 0x9c, 0x81, 0x55, 0x28, 0x5b, 0xec } }

#define NVRAM_MODE_MANUFACTORING 1
#define NVRAM_MODE_RESET_CONFIGURATION 2
#define NVRAM_MODE_DEFAULT_CONFIGURATION 4
#define NVRAM_MODE_SIMULATION 8

#define NVRAM_STORE_FLAG_NON_VALATILE 1
#define NVRAM_STORE_FLAG_READ_ONLY 2
#define NVRAM_STORE_FLAG_DO_NOT_ENUMERATE 4

extern const UINTN FlashEmpty;
extern const UINT32 FlashEmptyNext;
extern const EFI_GUID AmiDefaultsVariableGuid;
extern const EFI_GUID AmiNvramHobGuid;
extern const CHAR16 MfgDefaults[];
extern const CHAR16 StdDefaults[];
extern const UINT32 NvramHeaderLength;

typedef UINT16 VAR_SIZE_TYPE;
#define NEXT_OFFSET (EFI_FIELD_OFFSET(NVAR,size)+sizeof(VAR_SIZE_TYPE))
#define NEXT_SIZE 3
#define FLAG_SIZE 1
#define FLAG_OFFSET (NEXT_OFFSET + NEXT_SIZE)
#define MAX_NVRAM_VARIABLE_SIZE ((1<<(sizeof(VAR_SIZE_TYPE)<<3))-1)
#pragma pack(push)
#pragma pack(1)
typedef struct{
	UINT32 signature;
	VAR_SIZE_TYPE size;
	UINT32 next:24;
	UINT32 flags:8;
//  guid and name are there only if NVRAM_FLAG_DATA_ONLY is not set
//	UINT8 guid; 
//	CHAR8 or CHAR16 name[...];
//  UINT8 data[...];
//  if NVRAM_FLAG_EXT_HEDER is set
//  UINT8 extflags;
//  UINT8 extdata[...];
//  VAR_SIZE_TYPE extsize;
}NVAR;

typedef struct {
    EFI_HOB_GUID_TYPE Header;
    EFI_PHYSICAL_ADDRESS NvramAddress;
    EFI_PHYSICAL_ADDRESS BackupAddress;
    UINT32 NvramSize;
    UINT32 HeaderLength;
    UINT32 NvramMode;
} NVRAM_HOB;
#pragma pack(pop)

typedef struct {
    UINT8 *NvramAddress;
    UINTN NvramSize;
    EFI_GUID* NvramGuidsAddress;
    UINT8 *pEndOfVars, *pFirstVar;
    INT16 NextGuid;
    VAR_SIZE_TYPE LastVarSize;
    NVAR *pLastReturned;
    UINT8 Flags;
} NVRAM_STORE_INFO;

//Low level access routines
EFI_GUID* NvGetGuid(NVAR* pVar, NVRAM_STORE_INFO *pInfo);
VOID* NvGetName(NVAR* pVar);
NVAR* NvGetDataNvar(NVAR *pVar, NVRAM_STORE_INFO *pInfo);
VOID* NvGetData(NVAR* pVar, UINTN NameLength, UINTN* pDataSize, NVRAM_STORE_INFO *pInfo);

//Validation routines
BOOLEAN NvIsVariable(NVAR *pVar, NVRAM_STORE_INFO *pInfo);
BOOLEAN NvIsValid(NVAR* pVar);

//Iteration routines
NVAR* NvGetNextNvar(NVAR* pVar, NVRAM_STORE_INFO *pInfo);
NVAR* NvGetNextValid(NVAR* pVar, NVRAM_STORE_INFO *pInfo);

//Comparison routines
BOOLEAN NvAttribEq(NVAR* pNvar, UINT32 Attributes, NVRAM_STORE_INFO *pInfo);
BOOLEAN NvVarEq(NVAR* pNvar, CHAR16* sName, EFI_GUID* pGuid, UINTN* pNameSize, NVRAM_STORE_INFO *pInfo);
BOOLEAN NvarEqNvar(NVAR *Nvar1, NVRAM_STORE_INFO *Info1, NVAR *Nvar2, NVRAM_STORE_INFO *Info2);

//High level routines that work with a single NV store
VOID* NvFindVariable(CHAR16* sName, EFI_GUID* pGuid, UINTN* pNameSize, NVRAM_STORE_INFO *pInfo);
EFI_STATUS NvGetVariable(
	IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
	OUT UINT32 *Attributes OPTIONAL,
	IN OUT UINTN *DataSize, OUT VOID *Data, 
    NVRAM_STORE_INFO *pInfo,  OUT NVAR **Var OPTIONAL
);
EFI_STATUS NvGetNextVariableName(
	IN OUT UINTN *VariableNameSize,
	IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid,
    NVRAM_STORE_INFO *pInfo, BOOLEAN Runtime
);

//High level routines that work with a multiple NV stores
EFI_STATUS NvGetVariable2(
	IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
	OUT UINT32 *Attributes OPTIONAL,
	IN OUT UINTN *DataSize, OUT VOID *Data,
    UINT32 InfoCount, NVRAM_STORE_INFO *pInfo
);
EFI_STATUS NvGetNextVariableName2(
	IN OUT UINTN *VariableNameSize,
	IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid,
    UINT32 InfoCount, NVRAM_STORE_INFO *pInfo, UINT32 *LastInfoIndex,
    BOOLEAN Runtime
);

//Service routines
VOID NvInitInfoBuffer(IN NVRAM_STORE_INFO *pInfo, UINTN HeaderSize, UINT8 Flags);
NVRAM_STORE_INFO* NvGetDefaultsInfo(
    IN const CHAR16* DefaultsVar, 
    IN NVRAM_STORE_INFO *pInInfo, OUT NVRAM_STORE_INFO *pOutInfo
);
EFI_STATUS NvGetAttributesFromNvar(
    IN NVAR *pNvar, IN NVRAM_STORE_INFO *pInfo,
    OUT UINT32 *Attributes
);
EFI_STATUS NvGetVariableFromNvar(
    NVAR *pNvar, UINTN NameSize, OUT UINT32 *Attributes OPTIONAL,
	IN OUT UINTN *DataSize, OUT VOID *Data,
    IN NVRAM_STORE_INFO *pInfo, OUT UINT8 *Flags OPTIONAL
);
EFI_FFS_FILE_STATE* GetNvramFfsFileStatePtr(NVRAM_STORE_INFO *Info);
EFI_FFS_FILE_STATE GetNvramFfsFileState(NVRAM_STORE_INFO *Info);
BOOLEAN IsMainNvramStoreValid(
    NVRAM_STORE_INFO *MainInfo, VOID *BackUpAddress,
    BOOLEAN *BackupStoreValid
);

UINT8 NvCalculateNvarChecksum(NVAR* pVar);
UINT8* NvGetExtFlags (NVAR* pVar);

// Shared with AuthVariable service
typedef struct {
    UINT8  AuthFlags; // AuthWriteAccess = 0x10 and TimeWriteAccess = 0x20
    UINT64 Mc;
    UINT8  KeyHash[32]; // sha256
}EXT_SEC_FLAGS; 

VOID
GetVarAuthExtFlags(
    IN NVAR *Var, 
    IN NVRAM_STORE_INFO *pInfo, 
    OUT EXT_SEC_FLAGS *ExtFlags
);

EFI_STATUS FindVariable(
    IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid,
    OUT UINT32 *Attributes ,
    IN OUT UINTN *DataSize, OUT VOID **Data
);

VOID CheckStore(
    IN BOOLEAN Recover
);

BOOLEAN IsNvramRuntime();

/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
