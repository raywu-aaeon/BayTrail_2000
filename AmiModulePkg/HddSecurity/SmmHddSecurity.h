//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************

//**********************************************************************
// $Header: /Alaska/SOURCE/Modules/HddSecurity/IdeSmm/IDESMM.h 5     9/27/11 3:04a Rajeshms $ /Alaska/SOURCE/Core/Modules/IdeBus/IDESMM.h
//
// $Revision: 5 $
//
// $Date: 9/27/11 3:04a $
//
//*********************************************************************

//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:	<IDESMM.h>
//
// Description:	This file contains the Includes, Definitions, typedefs,
//		        Variable and External Declarations, Structure and
//              function prototypes needed for the IDESMM Component
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>

#ifndef _IDESMM_H_
#define _IDESMM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <AmiDxeLib.h>
#include <Protocol\SmmBase2.h>
#include <Protocol\SmmSwDispatch2.h>
#include <Token.h>
#include "HddSecurity.h"

extern EFI_BOOT_SERVICES     *pBS;

extern EFI_GUID              gIdeSecurityInterfaceGuid;
extern EFI_GUID              gEfiSmmSwDispatch2ProtocolGuid;
extern EFI_GUID              gEfiSmmBase2ProtocolGuid;


extern EFI_SMM_BASE2_PROTOCOL        *pSmmBase;
extern EFI_SMM_SYSTEM_TABLE2         *pSmst;
DLIST                        gPasswordList;

#ifndef DMA_ATAPI_COMMAND_COMPLETE_TIMEOUT
#define     DMA_ATAPI_COMMAND_COMPLETE_TIMEOUT  16000           // 16Sec
#endif

#ifndef S3_BUSY_CLEAR_TIMEOUT
#define     S3_BUSY_CLEAR_TIMEOUT               10000           // 10Sec
#endif

#define     BUSY_CLEAR_TIMEOUT                  1000            // 1Sec
#define     DRDY_TIMEOUT                        1000            // 1Sec
#define     DRQ_TIMEOUT                         10              // 10msec
#pragma pack(1)

typedef struct
{
    UINT8 bFeature;
    UINT8 bSectorCount;
    UINT8 bLbaLow;
    UINT8 bLbaMid;
    UINT8 bLbaHigh;
    UINT8 bDevice;
    UINT8 bCommand;
} COMMAND_BUFFER;


#pragma pack()
EFI_STATUS InSmmFunction (
	    IN EFI_HANDLE       ImageHandle,
	    IN EFI_SYSTEM_TABLE *SystemTable
 );


EFI_STATUS InstallSmiHandler (
    IN EFI_HANDLE       ImageHandle,
    IN EFI_SYSTEM_TABLE *SystemTable );

EFI_STATUS SMMSecurityUnlockCommand (
    HDD_PASSWORD *pHddPassword );

EFI_STATUS SMMIdeNonDataCommand (
    HDD_PASSWORD *pHddPassword,
    IN UINT8     Features,
    IN UINT8     SectorCount,
    IN UINT8     LBALow,
    IN UINT8     LBAMid,
    IN UINT8     LBAHigh,
    IN UINT8     Command );

void ZeroMemorySmm (
    void  *Buffer,
    UINTN Size );

static UINT64 DivU64x32Local(
    IN UINT64           Dividend,
    IN UINTN            Divisor,
    OUT UINTN*Remainder OPTIONAL );

VOID *
EFIAPI
CopyMem (
  OUT     VOID                      *DestinationBuffer,
  IN      CONST VOID                *SourceBuffer,
  IN      UINTN                     Length
  );
/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif

#endif  // _IDESMM_H_

//**********************************************************************
//**********************************************************************
//**                                                                  **
//**        (C)Copyright 1985-2010, American Megatrends, Inc.         **
//**                                                                  **
//**                       All Rights Reserved.                       **
//**                                                                  **
//**         5555 Oakbrook Pkwy, Suite 200, Norcross, GA 30093        **
//**                                                                  **
//**                       Phone: (770)-246-8600                      **
//**                                                                  **
//**********************************************************************
//**********************************************************************
