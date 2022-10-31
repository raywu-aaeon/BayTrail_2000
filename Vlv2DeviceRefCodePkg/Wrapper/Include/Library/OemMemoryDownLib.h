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
#ifndef _OEM_MEMORY_DOWN_LIBRARY_H_
#define _OEM_MEMORY_DOWN_LIBRARY_H_

//<AMI_FHDR_START>
//-------------------------------------------------------------------------
//
// Name:        OemMemoryDownLib.h
//
// Description: This header file contains OEM Memory Down Library related
//              structure and constant definitions.
//
//-------------------------------------------------------------------------
//<AMI_FHDR_END>

//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------
#include "token.h"
#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 2)
#include <MemoryInit/Src32/OemHooks.h>
#endif

#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 1)
VOID
OemUpdateDimmSpdTbl (
    IN UINT8                Channel,
    IN UINT8                CurrentDimmSocket,
    IN OUT UINT8            *DimmSpdTbl
);
#endif

#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 2)
VOID
OemUpdateMemoryDownParam (
    IN OUT MRC_DRAM_INPUT   *DramInput
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
