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
// $Header: /Alaska/BIN/Core/Include/PPI/LoadedImagePpi.h 4     2/05/11 2:12p Artems $
//
// $Revision: 4 $
//
// $Date: 2/05/11 2:12p $
//**********************************************************************
//<AMI_FHDR_START>
//
// Name:	LoadedImagePpi.h
//
// Description:	EFI_PEI_LOADED_IMAGE_PPI PPI definition
//
//<AMI_FHDR_END>
//**********************************************************************
#ifndef __LoadedImagePpi__H__
#define __LoadedImagePpi__H__
#ifdef __cplusplus
extern "C" {
#endif

#include <Pei.h>
#include <Ffs.h>

#include <Ppi/LoadImage.h>

/*
// {A62A3FFF-97F0-4332-8CFD-1E343D3631F2}
#define EFI_PEI_LOADED_IMAGE_PPI_GUID \
{ 0xa62a3fff, 0x97f0, 0x4332, 0x8c, 0xfd, 0x1e, 0x34, 0x3d, 0x36, 0x31, 0xf2 }

GUID_VARIABLE_DECLARATION(gEfiPeiLoadedImagePpiGuid, EFI_PEI_LOADED_IMAGE_PPI_GUID);

typedef struct _EFI_PEI_LOADED_IMAGE_PPI {
    EFI_PHYSICAL_ADDRESS        ImageAddress;
    UINT64                      ImageSize;
    EFI_PEI_FILE_HANDLE         FileHandle;
} EFI_PEI_LOADED_IMAGE_PPI;
*/

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

