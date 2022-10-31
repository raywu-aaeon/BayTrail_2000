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
// $Header: /AptioV/BIN/AMIDebugRx/DbgRxDebugSupportLib/DbgrHelp.c 1     11/02/12 10:14a Sudhirv $
//
// $Revision: 1 $
//
// $Date: 11/02/12 10:14a $
//*****************************************************************


//**********************************************************************
//<AMI_FHDR_START>
//
// Name:		DbgHelp.c
//
// Description:	File containing the globals for CAR base address & size.
//
//<AMI_FHDR_END>
//**********************************************************************

#ifndef EFIx64
#include <Library\AMIPeiDebug.h>
#else
#include <Library\AMIPeiDebugX64.h>
#endif
#include <AmiHobs.h>
#include <Library\AMIPeiGUIDS.h>
#include "token.h"

#ifndef USB_DEBUG_TRANSPORT
#define USB_DEBUG_TRANSPORT	0
#endif

//volatile UINTN USB_DEBUGGER_ENABLED = USB_DEBUG_TRANSPORT;

const UINTN	AMI_PEIDEBUGGER_DS_BASEADDRESS 	= 0;
const UINTN	AMI_PEIDEBUGGER_DS_SIZE			= 0x2048;

EFI_GUID  mPeiDebugDataGuidDbgSup = PEI_DBGSUPPORT_DATA_GUID;
EFI_GUID  mDxeDebugDataGuidDbgSup = DXE_DBG_DATA_GUID;
EFI_GUID  mPeiDbgBasePpiGuidDbgSup = EFI_PEI_DBG_BASEADDRESS_PPI_GUID;
EFI_GUID  mPeiDbgDbgrIfcGuidDbgSup = PEI_DBGR_REPORTSTATUSCODE_GUID;

#ifdef DBG_WRITE_IO_80_SUPPORT
volatile UINTN gDbgWriteIO80Support = DBG_WRITE_IO_80_SUPPORT;
#else
volatile UINTN gDbgWriteIO80Support = 0;
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


