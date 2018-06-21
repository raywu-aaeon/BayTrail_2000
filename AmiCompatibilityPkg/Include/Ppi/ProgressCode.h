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


//**********************************************************************
// $Header: /Alaska/BIN/Core/Include/PPI/ProgressCode.h 2     3/13/06 1:16a Felixp $
//
// $Revision: 2 $
//
// $Date: 3/13/06 1:16a $
//*****************************************************************************
//<AMI_FHDR_START>
//
// Name:
//
// Description:	This file contains
//
//<AMI_FHDR_END>
//*****************************************************************************
#ifndef _PEI_PROGRESS_CODE_PPI_H
#define _PEI_PROGRESS_CODE_PPI_H
#ifdef __cplusplus
extern "C" {
#endif
#include <Pei.h>

#include <Ppi/StatusCode.h>

/*
#define EFI_PEI_REPORT_PROGRESS_CODE_PPI_GUID  \
  {0x229832d3, 0x7a30, 0x4b36, 0xb8, 0x27, 0xf4, 0xc, 0xb7, 0xd4, 0x54, 0x36}

GUID_VARIABLE_DECLARATION(gPeiStatusCodePpiGuid,EFI_PEI_REPORT_PROGRESS_CODE_PPI_GUID);
  
typedef
EFI_STATUS (EFIAPI *PEI_REPORT_STATUS_CODE) (
  IN EFI_PEI_SERVICES        **PeiServices,
  IN EFI_STATUS_CODE_TYPE    CodeType,
  IN EFI_STATUS_CODE_VALUE   Value,
  IN UINT32                  Instance,
  IN EFI_GUID                *CallerId OPTIONAL,
  IN EFI_STATUS_CODE_DATA    *Data OPTIONAL
  );

typedef
struct _EFI_PEI_PROGRESS_CODE_PPI {
	PEI_REPORT_STATUS_CODE  ReportStatusCode;
} EFI_PEI_PROGRESS_CODE_PPI;
*/

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
