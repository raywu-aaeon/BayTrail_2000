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
//
//*************************************************************************
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  OemMemoryDownLib.c
//
// Description: Library Class for OEM Memory Down function.
//
//
//<AMI_FHDR_END>
//*************************************************************************
//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------
#include <Library/MemoryDownLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 1)
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   OemUpdateDimmSpdTbl
//
// Description: Update dummy Spd Table of the DIMM for memory down function
//              dynamically per your board design if needed.
//
// Input:       
//              Channel               The Channel of the DIMM
//              CurrentDimmSocket     The Socket of the DIMM
//              *DimmSpdTbl           The dummy Spd Table of the DIMM
// Output:      
//              None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
OemUpdateDimmSpdTbl (
    IN UINT8                Channel,
    IN UINT8                CurrentDimmSocket,
    IN OUT UINT8            *DimmSpdTbl
)
{
    //
    // If you want to report dummy SPD data dynamically per your
    // Board ID, you will need to port this function per your design.
    //
    /* 
    if (Channel == 0 && CurrentDimmSocket == 0) {
      DimmSpdTbl[5] = 0x11;
    }
    */
}
#endif

#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 2)
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   OemUpdateMemoryDownParam
//
// Description: Update Memory Down Parameters of MRC_DRAM_INPUT dynamically
//              per your board design if needed.
//
// Input:       *DramInput    The pointer of the Memory Down Parameters of
//                            MRC_DRAM_INPUT
//
// Output:      None
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
OemUpdateMemoryDownParam (
    IN OUT MRC_DRAM_INPUT   *DramInput
)
{
    //
    // If you want to report Memory Down Parameters dynamically per your
    // Board ID, you will need to port this function per your design.
    //
    /*
    DramInput->DRAM_Speed = 1;
    */
}
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
