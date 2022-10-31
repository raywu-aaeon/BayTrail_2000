//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//

//**********************************************************************//
// $Header: /Alaska/BIN/Modules/SMBIOS/SmbiosProtocol/SmbiosGetFlashDataProtocol.h 7     5/18/10 5:15p Davidd $
//
// $Revision: 7 $
//
// $Date: 5/18/10 5:15p $
//**********************************************************************//
//**********************************************************************//

#ifndef _EFI_SMBOS_GET_FLASH_DATA_PROTOCOL_H_
#define _EFI_SMBOS_GET_FLASH_DATA_PROTOCOL_H_

#include <token.h>

extern EFI_GUID gAmiSmbiosFlashDataProtocolGuid;

typedef struct _EFI_SMBIOS_FLASH_DATA_PROTOCOL EFI_SMBIOS_FLASH_DATA_PROTOCOL;

typedef EFI_STATUS (*GET_FLASH_TABLE_INFO)(
    IN  EFI_SMBIOS_FLASH_DATA_PROTOCOL  *This,
    OUT VOID                            **Location,
    OUT UINT32                          *Size
);

typedef EFI_STATUS (*GET_FIELD) (
    IN  EFI_SMBIOS_FLASH_DATA_PROTOCOL  *This,
    IN  UINT8                           Table,
    IN  UINT8                           Offset,
    OUT VOID                            **String
);

struct _EFI_SMBIOS_FLASH_DATA_PROTOCOL {
    GET_FLASH_TABLE_INFO                GetFlashTableInfo;
    GET_FIELD                           GetField;
};

#endif

//**********************************************************************//
//**********************************************************************//
//**                                                                  **//
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **//
//**                                                                  **//
//**                       All Rights Reserved.                       **//
//**                                                                  **//
//**        5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093         **//
//**                                                                  **//
//**                       Phone: (770)-246-8600                      **//
//**                                                                  **//
//**********************************************************************//
//**********************************************************************//
