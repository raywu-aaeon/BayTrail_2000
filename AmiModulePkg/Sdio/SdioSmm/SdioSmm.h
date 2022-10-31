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
// $Header: /Alaska/SOURCE/Modules/SdioDriver/SdioSmm.h 13    3/08/12 5:01a Rajeshms $
//
// $Revision: 13 $
//
// $Date: 3/08/12 5:01a $
//**********************************************************************

//<AMI_FHDR_START>
//--------------------------------------------------------------------------
//
// Name: SdioSmm.h
//
// Description:  Header file for the SdioSmm
//
//--------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _EFI_SDIO_SMM_DRIVER_H_
#define _EFI_SDIO_SMM_DRIVER_H_

#include <Efi.h>
#include <Token.h>
#include <Dxe.h>
#include <AmiDxeLib.h>
#if PI_SPECIFICATION_VERSION >= 0x1000A
#include <Protocol\SmmBase2.h>
#include <Protocol\SmmSwDispatch2.h>
#define RETURN(status) {return status;}
#else
#include <Protocol\SmmBase.h>
#include <Protocol\SmmSwDispatch.h>
#define RETURN(status) {return ;}
#endif
#include "SdioDef.h"


//-----------------------------------------------
//      ERROR CODE REPORTED TO CALLER
//-----------------------------------------------
#define SDIO_WRITE_PROTECT_ERR          0x003   // Write protect error
#define SDIO_TIME_OUT_ERR               0x080   // Command timed out error
#define SDIO_DRIVE_NOT_READY_ERR        0x0AA   // Drive not ready error
#define SDIO_DATA_CORRECTED_ERR         0x011   // Data corrected error
#define SDIO_PARAMETER_FAILED           0x007   // Bad parameter error
#define SDIO_MARK_NOT_FOUND_ERR         0x002   // Address mark not found error
#define SDIO_NO_MEDIA_ERR               0x031   // No media in drive
#define SDIO_READ_ERR                   0x004   // Read error
#define SDIO_UNCORRECTABLE_ERR          0x010   // Uncorrectable data error
#define SDIO_BAD_SECTOR_ERR             0x00A   // Bad sector error
#define SDIO_GENERAL_FAILURE            0x020   // Controller general failure

//
//SDIO Setup fields
//

typedef struct {
    UINT8   SdioMode;
    UINT8   SdioEmu1;
    UINT8   SdioEmu2;
    UINT8   SdioEmu3;
    UINT8   SdioEmu4;
    UINT8   SdioEmu5;
    UINT8   SdioEmu6;
    UINT8   SdioEmu7;
    UINT8   SdioEmu8;
	UINT8	SdioMassDevNum;
} SDIO_DEV_CONFIGURATION;






EFI_STATUS
NotInSmmFunction(
    EFI_HANDLE                  ImageHandle,
    EFI_SYSTEM_TABLE            *SystemTable
);

EFI_STATUS
SdioInSmmFunction(
    EFI_HANDLE                  ImageHandle,
    EFI_SYSTEM_TABLE            *SystemTable
);


#if PI_SPECIFICATION_VERSION >= 0x1000A
EFI_STATUS
SdioSWSMIHandler (
    EFI_HANDLE                  DispatchHandle,
    EFI_SMM_SW_REGISTER_CONTEXT *DispatchContext,
    IN OUT EFI_SMM_SW_CONTEXT   *SwContext,
    IN OUT UINTN                *CommBufferSize
);
#else
VOID
SdioSWSMIHandler (
    EFI_HANDLE                  DispatchHandle,
    EFI_SMM_SW_DISPATCH_CONTEXT *DispatchContext
);
#endif

#if !SDIO_SMM_SUPPORT
VOID
SdioAPIHandler (
    IN    SDIO_STRUC   *SdioURP
);
#endif

VOID
SdioPassControllerInformation (
    SDIO_STRUC                  *SdioURP
);

VOID
SdioAPIGetMassDeviceInformation (
    SDIO_STRUC                  *SdioURP
);

VOID
SdioAPIRead (
    SDIO_STRUC                  *SdioURP
);

VOID
SdioAPIWrite (
    SDIO_STRUC                  *SdioURP
);

VOID SdioAPICheckDevicePresence(
    SDIO_STRUC *SdioURP
);

VOID SdioApiConfigureDevice(
   SDIO_STRUC *SdioURP
);

VOID
ZeroMemorySmm (
    VOID                            *Buffer,
    UINTN                           Size
 );

UINT8 GetDevEntry (
    IN  UINT8                       DeviceAddress
);

VOID SioApiReset (
    SDIO_STRUC                      *SdioURP
);

VOID SdioApiDeviceGeometry (
    SDIO_STRUC                      *SdioURP
);

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
