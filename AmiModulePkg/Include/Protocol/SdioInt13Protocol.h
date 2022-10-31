//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2012, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioInt13Protocol.h 2     4/19/11 6:59a Lavanyap $
//
// $Revision: 2 $
//
// $Date: 4/19/11 6:59a $
//**********************************************************************

//**********************************************************************
//<AMI_FHDR_START>
//
// Name:    SdioInt13Protocol.c
//
// Description: Sdio Int13 protocol header definiation.
//<AMI_FHDR_END>
//**********************************************************************

#ifndef _SDIO_INT13_PROT_H
#define _SDIO_INT13_PROT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <EFI.h>

//#define EFI_SDIO_PROTOCOL_GUID    \
//  { 0x9708adb2, 0x28b1, 0x46f7, 0x9a, 0x6c, 0xe7, 0x44, 0x97, 0xfa, 0x66, 0x79}


//GUID_VARIABLE_DECLARATION(gEfiSdioProtocolGuid,EFI_SDIO_PROTOCOL_GUID);
extern EFI_GUID gEfiSdioProtocolGuid;


// Values for Mass Storage Device type
//-------------------------------------
#define SDIO_MASS_DEV_HDD        1
#define SDIO_MASS_DEV_ARMD       2

typedef struct 
{
    UINT16      wBlockSize;         
    UINT32      dMaxLba;            
    UINT8       bHeads;             
    UINT8       bSectors;           
    UINT16      wCylinders;         
    UINT8       bNonLBAHeads;       
    UINT8       bNonLBASectors;     
    UINT16      wNonLBACylinders;   
} SDIO_DEV_INFO;

typedef struct
{
    VOID                    *DevInfo;
    UINT16                  LogicalAddress;
    EFI_HANDLE              Handle;
    UINT16                  PciBDF;
    UINT8                   *DevString;
    UINT8                   StorageType;
} SDIO_MASS_DEV;

typedef EFI_STATUS (EFIAPI *EFI_SDIO_INSTALL_LEGACY_DEVICE)(SDIO_MASS_DEV*);

typedef struct _EFI_SDIO_PROTOCOL {
    EFI_SDIO_INSTALL_LEGACY_DEVICE   SdioInstallLegacyDevice;
} EFI_SDIO_PROTOCOL;


/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif

#endif 

//****************************************************************************
//****************************************************************************
//**                                                                        **
//**             (C)Copyright 1985-2012, American Megatrends, Inc.          **
//**                                                                        **
//**                          All Rights Reserved.                          **
//**                                                                        **
//**                 5555 Oakbrook Pkwy, Norcross, GA 30093                 **
//**                                                                        **
//**                          Phone (770)-246-8600                          **
//**                                                                        **
//****************************************************************************
//****************************************************************************
