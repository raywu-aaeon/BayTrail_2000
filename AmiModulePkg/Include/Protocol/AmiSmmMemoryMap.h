//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
/** @file
AMI Memory Map SMM protocol definition.
 **/
#ifndef __AMI_SMM_MEMORY_MAP_PROTOCOL_H__
#define __AMI_SMM_MEMORY_MAP_PROTOCOL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define AMI_SMM_MEMORY_MAP_PROTOCOL_GUID \
    {0x299781bc, 0xc56e, 0x47be, {0xb2, 0xe0, 0xc3, 0x15, 0x4c, 0xd8, 0x9d, 0x4d}}

typedef struct _AMI_SMM_MEMORY_MAP_PROTOCOL AMI_SMM_MEMORY_MAP_PROTOCOL;

/**
Return the memory type for a passed memory address.

@param Address      Memory address
@param MemoryType   memory type of the Address.

@retval EFI_SUCCESS     MemoryType contains memory type of the Address.
@retval EFI_NOT_FOUND   Address was not found in the UEFI memory map.
@retval EFI_INVALID_PARAMETER   MemoryType is NULL.
**/
typedef EFI_STATUS (EFIAPI *AMI_SMM_MEMORY_MAP_GET_MEMORY_TYPE) (
    IN UINTN Address, OUT EFI_MEMORY_TYPE *MemoryType
);

/// Provide servuces to return information based on UEFI memory map
struct _AMI_SMM_MEMORY_MAP_PROTOCOL {
    AMI_SMM_MEMORY_MAP_GET_MEMORY_TYPE  GetMemoryType;
};

extern EFI_GUID gAmiSmmMemoryMapProtocolGuid;

#endif 
#ifdef __cplusplus
}
#endif
//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
