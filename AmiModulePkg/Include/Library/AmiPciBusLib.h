//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
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
// $Header:$
//
// $Revision:  $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:    AmiSioPeiLib.h
//
// Description: 
//  SIO PEI Init Library.
//
// Notes:
//
//<AMI_FHDR_END>
//*************************************************************************
#ifndef _AMI_PCI_BUS_COMMON_LIB_H_
#define _AMI_PCI_BUS_COMMON_LIB_H_
#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------
#include <Token.h>
#include <PciBus.h>
#include <Pci.h> 
#include <PciE.h>
#include <AmiDxeLib.h>
#include <Protocol/AmiBoardInfo2.h>

//-------------------------------------------------------------------------
// Constants, Macros and Type Definitions
//-------------------------------------------------------------------------
//PCI Bus Porting Constants definitions
const UINT16 NbDmiL0ExitLatency;
const UINT16 NbDmiL1ExitLatency;
const UINT16 SbDmiL0ExitLatency;
const UINT16 SbDmiL1ExitLatency;
const UINT16 NbDmiAspmSupport;
const UINT16 SbDmiAspmSupport;
const UINT16 SbInternalDelay;

//----------------------------------------------------------------------------------
//PCI Bus Configuration Constants definitions
//const BOOLEAN S3VideoRepost;
const BOOLEAN FixedBusAssign;
const BOOLEAN DecodeFullBusRanges;
const UINT8	PciRserveUncoreBuses;
const BOOLEAN HotPlugSupport;
const BOOLEAN ApplyPaddingAnyway;
//-------------------------------------------------------------------------
//Variable, Prototype, and External Declarations
//-------------------------------------------------------------------------

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   AddBusDbEntry()
//
// Description: Fills gPciBusDb Array in ascending  order.
//
// Input:
//  PCI_BRG_EXT     *Ext        Pointer to PCI Bridge Extension Private Data structure.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//  EFI_INVALID_PARAMETER   When some of the parameters are invalid.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS  AddBusDbEntry(AMI_SDL_PCI_DEV_INFO *SdlData, T_ITEM_LIST *BusDb);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   AddDevDbEntry()
//
// Description: Fills gPciBusDb Array in ascending  order.
//
// Input:
//  PCI_BRG_EXT     *Ext        Pointer to PCI Bridge Extension Private Data structure.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//  EFI_INVALID_PARAMETER   When some of the parameters are invalid.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS  AddDevDbEntry(AMI_SDL_PCI_DEV_INFO *SdlData, T_ITEM_LIST *BusDb);


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	PciCfgXX()
//
// Description: Will do PCI Configuration Space Access 8;16;32;64 bit width
//
// Input:
//  ROOT_BRIDGE_IO_PROTOCOL *RbIo   Pointer to PciRootBridgeIO Protocol.
//  PCI_CFG_ADDR            addr    PCI_CFG_ADDR filled by caller
//  BOOLEAN                 wr      TRUE = Write FALSE = Read
//  UINT8                   *buff   Pointer Data Buffer.
//
// Output:  EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_NOT_FOUND           When Device not present in the system.
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//
// Referals:PCI_CFG_ADDR
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS PciCfg8(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *RbIo, PCI_CFG_ADDR addr, BOOLEAN wr, UINT8 *buff);

EFI_STATUS PciCfg16(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *RbIo, PCI_CFG_ADDR addr, BOOLEAN wr, UINT16 *buff);

EFI_STATUS PciCfg32(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *RbIo, PCI_CFG_ADDR addr, BOOLEAN wr, UINT32 *buff);

EFI_STATUS PciCfg64(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *RbIo, PCI_CFG_ADDR addr, BOOLEAN wr, UINT64 *buff);


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsFunc0()
//
// Description: Checks if PCI_DEV_INFO is Function 0 of PCI device.
//
// Input:
//  PCI_DEV_INFO    *Device     Pointer to PCI Device Private Data structure.
//
// Output:	BOOLEAN
//  TRUE    Device is Function 0 of Mulifunctional device.
//  FALSE   Device is not Function 0 of Mulifunctional device.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsFunc0(PCI_DEV_INFO *Device );

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsFunc0OfMfDev()
//
// Description: Checks if PCI_DEV_INFO data passed belongs to Function 0 of
// Multy-Functional device.
//
// Input:
//  PCI_DEV_INFO    *Device     Pointer to PCI Device Private Data structure.
//
// Output:	BOOLEAN
//  TRUE    Device is Function 0 of Mulifunctional device.
//  FALSE   Device is not Function 0 of Mulifunctional device.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsFunc0OfMfDev(PCI_DEV_INFO *Device );

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   CpyItemLst()
//
// Description: Creates a copy of T_ITEM_LST structure.
//
// Input:
//  T_ITEM_LIST *Lst        Pointer to the structure to copy.
//  T_ITEM_LIST **NewLstPtr Double Pointer to the copied data (Memory allocation is done by this function).
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_OUT_OF_RESOURCES    When system runs out of resources.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS CpyItemLst(T_ITEM_LIST *Lst, T_ITEM_LIST **NewLstPtr);

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   SetSubBus()
//
// Description: Programm SubordinateBusNumber Register of PCI Bridge.
//
// Input:
//  PCI_DEV_INFO    *Brg        Pointer to PCI Bridge Private Data structure.
//  UINT8           SubBusNo    Number to programm.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//  EFI_INVALID_PARAMETER   When some of the parameters are invalid.
//
// Referals: PCI_BRG_INFO.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SetSubBus(PCI_DEV_INFO *Brg, UINT8 SubBusNo);


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   RestoreBridgeBuses()
//
// Description: This function will Reprogram Primary Secondary and Subordinate
// bus numbers for the downsteram bridges after SEC BUS reset signal assertion.
//
// Input:
//  PCI_DEV_INFO    *DnPort    Pointer to PCI Device Private Data structure.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS RestoreBridgeBuses(PCI_DEV_INFO *DnPort);


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   MapBridgeBuses()
//
// Description: Maps Bridge's Primary Secondary Subordinate Bus Numbers
// according information stored in PCI_DEV_INFO and PCI_BRG_EXT structures
// of the PCI Bridge.
//
// Input:
//  PCI_DEV_INFO    *Brg        Pointer to PCI Bridge Private Data structure.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_DEVICE_ERROR        When driver fails access PCI Bus.
//  EFI_INVALID_PARAMETER   When some of the parameters are invalid.
//
// Referals: PCI_DEV_INFO; PCI_BRG_INFO; PCI_BRG_EXT.
//
// Notes:
//  BaseBus         =   PCI_DEV_INFO->Address.Addr.Bus;
//  SecondaryBus    =   PCI_BRG_EXT->Res[rtBus].Base;
//  SubordinateBus  =   PCI_BRG_EXT->Res[rtBus].Base + PCI_BRG_EXT->Res[rtBus].Length-1;
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS MapBridgeBuses(PCI_DEV_INFO *Brg);


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   IsPowerOfTwo()
//
// Description:
//  Checks if value passed makes "POWER OF TWO"
//
// Input:
//  UINT64      Value       Value to check.
//
// Output: BOOLEAN
//  TRUE or FALSE based on value passed.
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
BOOLEAN IsPowerOfTwo(UINT64 Value);


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   ClearBar()
//
// Description: Clears/zeros contents of PCI_BAR structure.
//
// Input:
//  PCI_BAR         *Bar    Pointer to PCI_BAR structure to clear.
//
// Output:	Nothing
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
VOID ClearBar(PCI_BAR *Bar);


//<AMI_PHDR_START>
//-------------------------------------------------------------------------
// Procedure:  
//
// Description:
//
// Input:
//
// Output:
//  EFI_SUCCESS - Set successfully.
//  EFI_INVALID_PARAMETER - the Input parameter is invalid.
//
// Notes:
//
//-------------------------------------------------------------------------
//<AMI_PHDR_END>

EFI_STATUS SortRbSdlData(AMI_SDL_PCI_DEV_INFO ***RbSdlData, UINTN *SdlDataCount);


UINT8 FindRootBus(PCI_DEV_INFO *Device);

#if (PCI_SETUP_USE_APTIO_4_STYLE == 1)
EFI_STATUS AmiPciGetSetupData(	PCI_SETUP_DATA 			*PciSetupData, 
								PCI_COMMON_SETUP_DATA 	*PciCommon,
								PCI_HOTPLUG_SETUP_DATA 	*HpSetup);
#else
EFI_STATUS AmiPciGetSetupData(	PCI_COMMON_SETUP_DATA 	*PciCommon,
								PCI_DEVICE_SETUP_DATA	*PciDev, 
								PCIE1_SETUP_DATA		*Pcie1,
								PCIE2_SETUP_DATA		*Pcie2,
								AMI_SDL_PCI_DEV_INFO    *DevSdlInfo,  	//OPTIONAL if ==NULL get defaults...
								UINTN					DevIdx);		//OPTIONAL 
#endif

EFI_STATUS LaunchInitRoutine(VOID *RoutineOwner, PCI_INIT_STEP InitStep, PCI_INIT_TYPE InitType,
		VOID *Param1, VOID *Param2, VOID *Param3, VOID *Param4);

BOOLEAN PciBusCheckIfPresent(PCI_DEV_INFO *Dev, AMI_SDL_PCI_DEV_INFO *DevFnData);

EFI_STATUS PciBusReadNextEmbeddedRom(	PCI_DEV_INFO *Device, UINTN OpRomNumber, //IN
									    AMI_SDL_PCI_DEV_INFO **ThisRomSdlData, VOID **RomFile, UINTN *RomSize); //OUT


EFI_STATUS AmiPciGetCommonSetupData(PCI_COMMON_SETUP_DATA 	*PciCommon);

EFI_STATUS AmiPciGetPciDevSetupData(PCI_DEVICE_SETUP_DATA 	*PciDev,
									AMI_SDL_PCI_DEV_INFO    *DevSdlInfo,//OPTIONAL if == NULL get defaults...
									UINTN					DevIdx,
									BOOLEAN 				Set);

EFI_STATUS AmiPciGetPcie1SetupData( PCIE1_SETUP_DATA 		*Pcie1,
									AMI_SDL_PCI_DEV_INFO    *DevSdlInfo,//OPTIONAL if == NULL get defaults...
									UINTN					DevIdx,		//OPTIONAL if == 0 
									BOOLEAN					Set);


EFI_STATUS AmiPciGetPcie2SetupData( PCIE2_SETUP_DATA 		*Pcie2,
									AMI_SDL_PCI_DEV_INFO    *DevSdlInfo,//OPTIONAL if == NULL get defaults...
									UINTN					DevIdx,		//OPTIONAL if == 0 
									BOOLEAN					Set);

EFI_STATUS AmiPciGetPciHpSetupData( PCI_HP_SETUP_DATA 		*PciHp,
									AMI_SDL_PCI_DEV_INFO    *DevSdlInfo,//OPTIONAL if == NULL get defaults...
									UINTN					DevIdx,		//OPTIONAL if == 0 
									BOOLEAN					Set);

EFI_STATUS AmiPciBusShift(			AMI_SDL_PCI_DEV_INFO 	*DevSdlInfo,
						  	  	  	INT16					*ShiftValue,	
						  	  	  	UINTN					HostIdx,
									UINTN					RootIdx,
						  	  	  	BOOLEAN					Set);

//Compatibility routine from AmiCompatibilityPkg
EFI_STATUS LaunchPortingRoutine(PCI_DEV_INFO *Device, PCI_INIT_STEP InitStep, 
								VOID *Param1, VOID *Param2, VOID *Param3, VOID *Param4);
/****** DO NOT WRITE BELOW THIS LINE *******/
#ifdef __cplusplus
}
#endif
#endif //_AMI_PCI_BUS_COMMON_LIB_H_
//*************************************************************************
//*************************************************************************
//**                                                                     **
//**        (C)Copyright 1985-2013, American Megatrends, Inc.            **
//**                                                                     **
//**                       All Rights Reserved.                          **
//**                                                                     **
//**      5555 Oakbrook Parkway, Suite 200, Norcross, GA 30093           **
//**                                                                     **
//**                       Phone: (770)-246-8600                         **
//**                                                                     **
//*************************************************************************
//*************************************************************************

