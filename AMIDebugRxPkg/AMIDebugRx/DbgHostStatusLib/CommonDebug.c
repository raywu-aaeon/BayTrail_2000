//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2014, American Megatrends, Inc.        **//
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
// Revision History
// ----------------
// $Log: /AptioV/SRC/AMIDebugRx/DbgHostStatusLib/CommonDebug.c $
// 
//
//*********************************************************************
//<AMI_FHDR_START>
//----------------------------------------------------------------------------
//
// Name:          CommonDebug.C
//
// Description:   Common Debug definitions
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

#ifdef REDIRECTION_ONLY_MODE
#if REDIRECTION_ONLY_MODE
volatile UINTN gRedirectionOnlyEnabled = 1;
#else
volatile UINTN gRedirectionOnlyEnabled = 0;
#endif
#endif

#ifdef DBG_PERFORMANCE_RECORDS
UINTN gDbgPerformanceRecords = DBG_PERFORMANCE_RECORDS;
#else
UINTN gDbgPerformanceRecords = 0;
#endif

#ifdef DBG_WRITE_IO_80_SUPPORT
volatile UINTN gDbgWriteIO80Support = DBG_WRITE_IO_80_SUPPORT;
#else
volatile UINTN gDbgWriteIO80Support = 0;
#endif

#ifndef GENERIC_USB_CABLE_SUPPORT
#define GENERIC_USB_CABLE_SUPPORT 0
#endif

// Load Fv Support
UINTN gFvMainBase = FV_MAIN_BASE;
UINTN gFvMainBlocks = FV_MAIN_BLOCKS;
UINTN gFvBBBlocks = FV_BB_BLOCKS;
UINTN gBlockSize = FLASH_BLOCK_SIZE;

volatile UINTN gGenericUsbSupportEnabled = GENERIC_USB_CABLE_SUPPORT;

EFI_GUID  mPeiDbgBasePpiGuid = EFI_PEI_DBG_BASEADDRESS_PPI_GUID;

EFI_GUID  mDxeDbgDataGuid = DXE_DBG_DATA_GUID;
INT8 CompareGuid(EFI_GUID *G1, EFI_GUID *G2);

UINTN DebugDataBaseAddress = 0;
UINTN SMMDebugDataBaseAddress = 0;
UINTN DxeDataBaseAddress = 0;

BOOLEAN CheckForHostConnectedinPEI (EFI_PEI_SERVICES **PeiServices);

//<AMI_PHDR_START>
//--------------------------------------------------------------------
// Procedure:	GetPCIBaseAddr
//
// Description:	Returns the PCIBase Address
//
// Input:		VOID
//
// Output:		UINT32 PCIExBaseAddress
//
//--------------------------------------------------------------------
//<AMI_PHDR_END>

UINTN  GetPciExBaseAddr()
{
	return (UINTN) PcdGet64 (PcdPciExpressBaseAddress);
}
//*****************************************************************//
//*****************************************************************//
//*****************************************************************//
//**                                                             **//
//**         (C)Copyright 2014, American Megatrends, Inc.        **//
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
