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
// Name:  MemoryDownLib.c
//
// Description: Library Class for Memory Down function.
//
//
//<AMI_FHDR_END>
//*************************************************************************
//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------
#include <Library/MemoryDownLib.h>
#include <Library/OemMemoryDownLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

#define DimmSpdTblSize  256

#if defined(NB_OEM_DIMM1_STATUS) && (NB_OEM_DIMM1_STATUS == 0x01)
static UINT8 mDimm1SpdTbl[] = NB_OEM_DIMM1_SPD_DATA;
#endif
#if defined(NB_OEM_DIMM2_STATUS) && (NB_OEM_DIMM2_STATUS == 0x01)
static UINT8 mDimm2SpdTbl[] = NB_OEM_DIMM2_SPD_DATA;
#endif
#if defined(NB_OEM_DIMM3_STATUS) && (NB_OEM_DIMM3_STATUS == 0x01)
static UINT8 mDimm3SpdTbl[] = NB_OEM_DIMM3_SPD_DATA;
#endif
#if defined(NB_OEM_DIMM4_STATUS) && (NB_OEM_DIMM4_STATUS == 0x01)
static UINT8 mDimm4SpdTbl[] = NB_OEM_DIMM4_SPD_DATA;
#endif

#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 2)
static const UINT8 OemRankEn[]        = OEM_RANK_EN;
static const UINT8 OemDimmDWidth[]    = OEM_DIMM_DWIDTH;
static const UINT8 OemDimmDensity[]   = OEM_DIMM_DENSITY;
static const UINT8 OemDimmBusWidth[]  = OEM_DIMM_BUSWIDTH;
static const UINT8 OemDimmSides[]     = OEM_DIMM_SIDES;
#endif


#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 1)
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   GetDimmSpdTbl
//
// Description: Get dummy Spd Table of each DIMM for memory down function
//
// Input:       
//              Channel               The Channel of the DIMM
//              CurrentDimmSocket     The Socket of the DIMM
// Output:      
//              UINT8 *               The dummy Spd Table pointer of the DIMM
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
UINT8 *
EFIAPI
GetDimmSpdTbl (
    IN UINT8                Channel,
    IN UINT8                CurrentDimmSocket
)
{
    UINT8       *DimmSpdTblBuffer;
    UINT8       *DimmSpdTbl;

    DimmSpdTbl  = NULL;

#if defined(NB_OEM_DIMM1_STATUS) && (NB_OEM_DIMM1_STATUS == 0x01)
    if (Channel == 0 && CurrentDimmSocket == 0) {
      DimmSpdTbl = mDimm1SpdTbl;
    }
#endif
#if defined(NB_OEM_DIMM2_STATUS) && (NB_OEM_DIMM2_STATUS == 0x01)
    if (Channel == 0 && CurrentDimmSocket == 1) {
      DimmSpdTbl = mDimm2SpdTbl;
    }
#endif
#if defined(NB_OEM_DIMM3_STATUS) && (NB_OEM_DIMM3_STATUS == 0x01)
    if (Channel == 1 && CurrentDimmSocket == 0) {
      DimmSpdTbl = mDimm3SpdTbl;
    }
#endif
#if defined(NB_OEM_DIMM4_STATUS) && (NB_OEM_DIMM4_STATUS == 0x01)
    if (Channel == 1 && CurrentDimmSocket == 1) {
      DimmSpdTbl = mDimm4SpdTbl;
    }
#endif

    if (DimmSpdTbl == NULL) {
      return NULL;
    } else {
      DimmSpdTblBuffer  = (UINT8 *) AllocateZeroPool(DimmSpdTblSize);
      CopyMem (DimmSpdTblBuffer, DimmSpdTbl, DimmSpdTblSize);
      OemUpdateDimmSpdTbl (Channel, CurrentDimmSocket, DimmSpdTblBuffer);
      return DimmSpdTblBuffer;
    }
}
#endif

#if defined(MRC_MEMORY_DOWN_SUPPORT) && (MRC_MEMORY_DOWN_SUPPORT == 2)
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   FillMemoryDownParam
//
// Description: Fill in Memory Down Parameters of MRC_DRAM_INPUT
//
// Input:       None
//
// Output:      The pointer of the Memory Down Parameters of MRC_DRAM_INPUT
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID
EFIAPI
FillMemoryDownParam (
    MRC_DRAM_INPUT        *DramInput
)
{
    DramInput->Rank_En[0][0]        = OemRankEn[0];         // Ranks Present
    DramInput->Rank_En[0][1]        = OemRankEn[1];         // Ranks Present
    DramInput->Rank_En[1][0]        = OemRankEn[2];         // Ranks Present
    DramInput->Rank_En[1][1]        = OemRankEn[3];         // Ranks Present
    DramInput->DIMM_DWidth[0][0]    = OemDimmDWidth[0];     // DRAM device data width
    DramInput->DIMM_DWidth[1][0]    = OemDimmDWidth[1];     // DRAM device data width
    DramInput->DIMM_Density[0][0]   = OemDimmDensity[0];    // DRAM device data density
    DramInput->DIMM_Density[1][0]   = OemDimmDensity[1];    // DRAM device data density
    DramInput->DRAM_Speed           = OEM_DRAM_SPEED;       // DRAM speed
    DramInput->DRAM_Type            = OEM_DRAM_TYPE;        // DRAM type
    DramInput->DIMM_MemDown         = OEM_DIMM_MEMDOWM;     // 0:DIMM, 1:Memory Down
    DramInput->DIMM_BusWidth[0][0]  = OemDimmBusWidth[0];   // DRAM Bus Width
    DramInput->DIMM_BusWidth[1][0]  = OemDimmBusWidth[1];   // DRAM Bus Width
    DramInput->DIMM_Sides[0][0]     = OemDimmSides[0];      // DRAM ranks per dimm
    DramInput->DIMM_Sides[1][0]     = OemDimmSides[1];      // DRAM ranks per dimm
    DramInput->tCL                  = OEM_DRAM_TCL;         // Actual CL
    DramInput->tRP_tRCD             = OEM_DRAM_TRP_TRCD;    // TRP and tRCD in dram clk
    DramInput->tWR                  = OEM_DRAM_TWR;         // Dram clk
    DramInput->tWTR                 = OEM_DRAM_TWTR;        // Dram clk 
    DramInput->tRRD                 = OEM_DRAM_TRRD;        // Dram clk 
    DramInput->tRTP                 = OEM_DRAM_TRTP;        // Dram clk
    DramInput->tFAW                 = OEM_DRAM_TFAW;        // Dram clk
    
    OemUpdateMemoryDownParam (DramInput);
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
