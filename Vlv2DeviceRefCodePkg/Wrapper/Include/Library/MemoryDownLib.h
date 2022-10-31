//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//**********************************************************************
#ifndef _MEMORY_DOWN_LIBRARY_H_
#define _MEMORY_DOWN_LIBRARY_H_

//<AMI_FHDR_START>
//-------------------------------------------------------------------------
//
// Name:        MemoryDownLib.h
//
// Description: This header file contains Memory Down Library related
//              structure and constant definitions.
//
//-------------------------------------------------------------------------
//<AMI_FHDR_END>

//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------
#include "token.h"
#include <Library/MemoryAllocationLib.h>
#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 2)
#include <MemoryInit/Src32/OemHooks.h>
#endif

#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 1)
UINT8 *
EFIAPI
GetDimmSpdTbl (
    IN UINT8                Channel,
    IN UINT8                CurrentDimmSocket
);
#endif

#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 2)
VOID
EFIAPI
FillMemoryDownParam (
    MRC_DRAM_INPUT        *DramInput
);
#endif

#endif

//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2014, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************
