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
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioInt13.h 1     3/19/10 4:28a Rameshr $
//
// $Revision: 1 $
//
// $Date: 3/19/10 4:28a $
//**********************************************************************

//<AMI_FHDR_START>
//---------------------------------------------------------------------------
//
//  Name:           SdioInt13.H
//  Description:    Definitions and structures for SDIO INT13
//
//---------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef __SDIOI13_HEADER__
#define __SDIOI13_HEADER__

#include "efi.h"


#define BAID_TYPE_HDD           1
#define BAID_TYPE_RMD_HDD       2
#define BAID_TYPE_CDROM         3
#define BAID_TYPE_RMD_FDD       4
#define BAID_TYPE_FDD           5

#define CSM16_SDIO_RT_DID       4

#define SYSTYPE_ATA             0
#define DEVTYPE_SYS             1

#pragma pack(1)

EFI_STATUS  SdioInstallLegacyDevice (SDIO_MASS_DEV*);
EFI_STATUS InitInt13RuntimeImage();

#define     SDIODEVS_MAX_ENTRIES 8

typedef struct _SDIOMASS_INT13_DEV {
    UINT8   Handle;
    UINT8   BbsEntryNo;
    UINT8   DevBaidType;
    UINT8   NumHeads;
    UINT8   LBANumHeads;
    UINT16  NumCylinders;
    UINT16  LBANumCyls;
    UINT8   NumSectors;
    UINT8   LBANumSectors;
    UINT16  BytesPerSector;
    UINT8   MediaType;
    UINT32  LastLBA;
    UINT8   DeviceNameString[32];
} SDIOMASS_INT13_DEV;

//
// The following data structure is located in UI13.BIN
//
typedef struct _UINT13_DATA {
    SDIOMASS_INT13_DEV  SdioMassI13Dev[SDIODEVS_MAX_ENTRIES];
    UINT8               MfgGenericName[13];    // "SDIO Storage", 0
    UINT16              BcvOffset;
    UINT16              SdioSmmDataOffset;
} UINT13_DATA;

#pragma pack()

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
