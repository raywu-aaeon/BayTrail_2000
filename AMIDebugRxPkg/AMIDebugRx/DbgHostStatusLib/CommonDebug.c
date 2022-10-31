//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2012, American Megatrends, Inc.        **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**   5555 Oakbrook Pkwy, Building 200,Norcross, Georgia 30093  **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
// $Header: /AptioV/BIN/AMIDebugRx/DbgHostStatusLib/CommonDebug.c 1     11/02/12 10:14a Sudhirv $
//
// $Revision: 1 $
//
// $Date: 11/02/12 10:14a $
//
//*********************************************************************
//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:          HostConStatus.C
//
// Description:   Checks whether the target is connected with host or not.
//
//----------------------------------------------------------------------------
//<AMI_FHDR_END>
#include "efi.h"
#include "Pei.h"

//#include "misc.h"
#ifndef	EFIx64
#include <Library\AMIPeiDebug.h>
#else
#include <Library\AMIPeiDebugX64.h>
#endif
#include <Library\AMIPeiGUIDs.h>

#include "token.h"

EFI_GUID  mPeiDbgBasePpiGuid = EFI_PEI_DBG_BASEADDRESS_PPI_GUID;

EFI_GUID  mDxeDbgDataGuid = DXE_DBG_DATA_GUID;
INT8 CompareGuid(EFI_GUID *G1, EFI_GUID *G2);

UINTN SMMDebugDataBaseAddress = 0;

BOOLEAN CheckForHostConnectedinPEI (EFI_PEI_SERVICES **PeiServices);
#define PEI_DBGSUPPORT_DATA_GUID  \
	{0x41cac730, 0xe64e, 0x463b, 0x89, 0x72, 0x25, 0x5e, 0xec, 0x55, 0x55, 0xc2}

//**********************************************************************
//<AMI_SHDR_START>
//
// Name:		PeiDbgDataSection of type PEI_DBG_DATA_SECTION
//
// Description:	The following global data structure is for relocation purpose
//				in order to support debugging after the debugger data section
//				is relocated. This second parameter of the data structure has
//				to be updated by the debugger service PEIM to point to relocated
//				data section into memory.The code section has this module
//				has to relocated before the update of second parameter.
//
//<AMI_SHDR_END>
//**********************************************************************
#ifndef EFIx64
PEI_DBG_DATA_SECTION PeiDbgDataSection = {
	"XPRT",
	(UINTN)NULL
};
#else
PEI_DBG_DATA_SECTIONx64 PeiDbgDataSection = {
	"XPRT",
	(UINTN)NULL
};
#endif

#ifndef EFIx64
PEI_DBG_DATA_SECTION SMMDbgDataSection = {
	"XPRT",
	(UINTN)NULL
};
#else
PEI_DBG_DATA_SECTIONx64 SMMDbgDataSection = {
	"XPRT",
	(UINTN)NULL
};
#endif

//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2012, American Megatrends, Inc.        **//
//**                                                             **//
//**                     All Rights Reserved.                    **//
//**                                                             **//
//**   5555 Oakbrook Pkwy, Building 200,Norcross, Georgia 30093  **//
//**                                                             **//
//**                     Phone (770)-246-8600                    **//
//**                                                             **//
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
