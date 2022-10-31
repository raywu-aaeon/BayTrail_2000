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
// $Header: $
//
// $Revision: $
//
// $Date: $
//*************************************************************************
//<AMI_FHDR_START>
//
// Name:  AmiPciBusLib.c
//
// Description: Library Class for AMI PCI Bus And AMI Root Bridge common routines.
//
//
//<AMI_FHDR_END>
//*************************************************************************
//-------------------------------------------------------------------------
// Include Files
//-------------------------------------------------------------------------
#include <AmiLib.h>
#include <AcpiRes.h>
#include <PciBus.h>
#include <PciHostBridge.h>
#include <PciSetup.h>
#include <Token.h>
#include <Setup.h>
#include <Protocol/PciIo.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/AmiBoardInfo2.h>
#include <Library/AmiSdlLib.h>
//-------------------------------------------------------------------------
// Constants, Macros and Type Definitions
//-------------------------------------------------------------------------
EFI_GUID		gPciSetupGuid	= PCI_FORM_SET_GUID;

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: AmiPciExpressLibConstructor
//
// Description:
//
// Input:
//
// Output:
//
// Notes:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
/*
EFI_STATUS EFIAPI AmiPciBusLibConstructor(IN EFI_HANDLE  ImageHandle, IN EFI_SYSTEM_TABLE  *SystemTable)
{
	EFI_STATUS				Status=EFI_SUCCESS;
//-------------------------------------------------
	InitAmiLib(ImageHandle, SystemTable);

	return Status;
}
*/


//----------------------------------------------------------------------
//Define Variables == TOKENS to be able to use binary
// See token help for details
//----------------------------------------------------------------------

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    NbDmiL0ExitLatency;
//
// Description:	Variable to replace NB_DMI_L0_EXIT_LATENCY token.
//
// Notes: const UINT16
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
const UINT16 NbDmiL0ExitLatency =
#ifdef NB_DMI_L0_EXIT_LATENCY
    NB_DMI_L0_EXIT_LATENCY
#else
    0
#endif
    ;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    NbDmiL1ExitLatency;
//
// Description:	Variable to replace NB_DMI_L1_EXIT_LATENCY token.
//
// Notes: const UINT16
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
const UINT16 NbDmiL1ExitLatency =
#ifdef NB_DMI_L1_EXIT_LATENCY
    NB_DMI_L1_EXIT_LATENCY
#else
    0
#endif
    ;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    SbDmiL0ExitLatency;
//
// Description:	Variable to replace SB_DMI_L0_EXIT_LATENCY token.
//
// Notes: const UINT16
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
const UINT16 SbDmiL0ExitLatency =
#ifdef SB_DMI_L0_EXIT_LATENCY
    SB_DMI_L0_EXIT_LATENCY
#else
    0
#endif
    ;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    SbDmiL1ExitLatency;
//
// Description:	Variable to replace SB_DMI_L1_EXIT_LATENCY token.
//
// Notes: const UINT16
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
const UINT16 SbDmiL1ExitLatency =
#ifdef SB_DMI_L1_EXIT_LATENCY
    SB_DMI_L1_EXIT_LATENCY
#else
    0
#endif
    ;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    NbDmiAspmSupport;
//
// Description:	Variable to replace NB_DMI_ASPM_SUPPORT token.
//
// Notes: const UINT16
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
const UINT16 NbDmiAspmSupport =
#ifdef NB_DMI_ASPM_SUPPORT
    NB_DMI_ASPM_SUPPORT
#else
    0
#endif
    ;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    SbDmiAspmSupport;
//
// Description:	Variable to replace SB_DMI_ASPM_SUPPORT token.
//
// Notes: const UINT16
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
const UINT16 SbDmiAspmSupport =
#ifdef SB_DMI_ASPM_SUPPORT
    SB_DMI_ASPM_SUPPORT
#else
    0
#endif
    ;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    SbInternalDelay;
//
// Description:	Variable to replace SB_INTERNAL_DELAY token.
//
// Notes: const UINT16
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
const UINT16 SbInternalDelay =
#ifdef SB_INTERNAL_DELAY
    SB_INTERNAL_DELAY
#else
    0
#endif
    ;

const BOOLEAN FixedBusAssign=
#ifdef PCI_FIXED_BUS_ASSIGNMENT
	PCI_FIXED_BUS_ASSIGNMENT
#else
	0
#endif
;

const BOOLEAN DecodeFullBusRanges=
#ifdef PCI_DECODE_FULL_BUS_RANGES
	PCI_DECODE_FULL_BUS_RANGES
#else
	0
#endif
;

const UINT8	PciRserveUncoreBuses =
#ifdef PCI_UNCORE_RESERVED_BUSES_PER_ROOT
	PCI_UNCORE_RESERVED_BUSES_PER_ROOT
#else
	0
#endif
;

const BOOLEAN HotPlugSupport=
#ifdef HOTPLUG_SUPPORT
	HOTPLUG_SUPPORT
#else
	0
#endif
;

//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    LaunchInitRoutinePriority;
//
// Description:	Variable to replace PCI_PORT_LAUNCH_INIT_ROUTINE_PRIORITY token.
//
// Notes: const UINT16
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
const UINT32 LaunchInitRoutinePriority =
#ifdef PCI_PORT_LAUNCH_INIT_ROUTINE_PRIORITY
		PCI_PORT_LAUNCH_INIT_ROUTINE_PRIORITY
#else
    0
#endif
    ;
	
//<AMI_SHDR_START>
//----------------------------------------------------------------------------
// Name:    ApplyPaddingAnyway;
//
// Description:	Variable to replace HOTPLUG_APPLY_PADDING_ANYWAY token.
//
// Notes: const UINT16
//
//----------------------------------------------------------------------------
//<AMI_SHDR_END>
const BOOLEAN ApplyPaddingAnyway=
#ifdef HOTPLUG_APPLY_PADDING_ANYWAY
		HOTPLUG_APPLY_PADDING_ANYWAY
#else
	0
#endif
;


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   PciBusReadNextPciOpRom()
//
// Description: Reads Option Rom file. If it indicated  .
//
// Input:
//  PCI_DEV_INFO 	*Device Pointer to PCI Device Private Data structure.
//
// Output:	EFI_STATUS
//  EFI_SUCCESS             When everything is going on fine!
//  EFI_END_OF_MEDIA        When Search reaches last container.
//  EFI_INVALID_PARAMETER   When some of the parameters are invalid.
//
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS PciBusReadNextEmbeddedRom(	PCI_DEV_INFO *Device, UINTN OpRomNumber, //IN
									AMI_SDL_PCI_DEV_INFO **ThisRomSdlData, VOID **RomFile, UINTN *RomSize ) //OUT
{
	EFI_STATUS				Status;
	AMI_SDL_PCI_DEV_INFO	*SdlData;
//------------------------
	//If device does not have SDL data associated with it or search reache the and,
	if((Device->AmiSdlPciDevData==NULL) || (Device->SdlDevCount < OpRomNumber)) return EFI_INVALID_PARAMETER; 

	//Init Output data as if we did not found anything...
	*ThisRomSdlData=NULL;
	*RomFile=NULL;
	*RomSize=0;
	
	//Now read the ROM
	SdlData=NULL;
	Status=AmiSdlFindIndexRecord(Device->SdlIdxArray[OpRomNumber],&SdlData);
	//If following TRUE SdlData->RomFileGuid and SdlData->RomSectionGuid has to be filled on
	if(SdlData->PciDevFlags.Bits.EmbededRom){
		Status=AmiSdlReadFfsSdlData((UINT8**)RomFile, RomSize,&SdlData->RomFileGuid, &SdlData->RomSectionGuid);
		ASSERT_EFI_ERROR(Status);
		*ThisRomSdlData=SdlData;
		return Status;
	}
	return EFI_NOT_FOUND;
}


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
EFI_STATUS  AddBusDbEntry(AMI_SDL_PCI_DEV_INFO *SdlData, T_ITEM_LIST *BusDb)
{
    EFI_STATUS          	Status;
    UINTN               	i;
//--------------------------
    if(BusDb->ItemCount==0) {
         Status=AppendItemLst(BusDb, SdlData);
    } else {
        for(i=0; i<BusDb->ItemCount; i++){
			AMI_SDL_PCI_DEV_INFO	*sdldat=(AMI_SDL_PCI_DEV_INFO*)BusDb->Items[i];
		//------------------------------	
            if(sdldat->Bus > SdlData->Bus){
                return InsertItemLst(BusDb, SdlData, i);
            }
        }//for
        // if we here and didn't returned yet - BusHdr->BusBuild is the biggest one.
        Status = AppendItemLst(BusDb, SdlData);
    }

    return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	SortRootBusData
//
// Description:	This function will parse PCI SDL Database looking for HostBridge
//              entries  and populate gRootBusDb
//              Structure.
//
// Input:		Nothing
//
// Output:		EFI_SUCCESS is OK
//
//
// Notes:		CHIPSET AND/OR BOARD PORTING NEEDED
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
EFI_STATUS SortRbSdlData(AMI_SDL_PCI_DEV_INFO ***RbSdlData, UINTN *SdlDataCount)
{
	AMI_SDL_PCI_DEV_INFO	*tmp;
	UINTN					i;
	T_ITEM_LIST             rblst={0,0,NULL};
	AMI_SDL_PCI_DEV_INFO    **prbarray=*RbSdlData;
	EFI_STATUS				Status=EFI_SUCCESS;
//--------------------------------
	//Fill tmprbdb with Root Bridges SdlData checking PciDevFlags if it is a real RB
	for(i=0; i<*SdlDataCount; i++){
		tmp=prbarray[i];
		if(tmp->PciDevFlags.Bits.RootBridgeType){
			//AddBusDbEntry function will sort entries based on Bus number
			Status=AddBusDbEntry(tmp, &rblst);
			ASSERT_EFI_ERROR(Status);
			if(EFI_ERROR(Status)) return Status;
		}
	}

	//now rblst has Root bridges sorted in ascending order.
	//if among HB childs we found something else than RB update root count...
	if(rblst.ItemCount!=*SdlDataCount) *SdlDataCount=rblst.ItemCount;

	//Now copy sorted entries inside passed address
	//free memory used.
	pBS->FreePool(prbarray);

	prbarray=(AMI_SDL_PCI_DEV_INFO**)rblst.Items;
	*RbSdlData=prbarray;


    return Status;
}

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
EFI_STATUS  AddDevDbEntry(AMI_SDL_PCI_DEV_INFO *SdlData, T_ITEM_LIST *BusDb)
{
    EFI_STATUS          	Status;
//--------------------------
    if(BusDb->ItemCount==0) {
         Status=AppendItemLst(BusDb, SdlData);
    } else {
		UINTN   i;
		UINT32	thisdevf=(SdlData->Device<<8)|SdlData->Function;
	//-----------------	
        for(i=0; i<BusDb->ItemCount; i++){
		    UINT32					devf;
			AMI_SDL_PCI_DEV_INFO	*sdldat = (AMI_SDL_PCI_DEV_INFO*)BusDb->Items[i];
		//---------------------------------------
        	devf=(sdldat->Device<<8)|sdldat->Function;
        	if(devf > thisdevf){
                return InsertItemLst(BusDb, SdlData, i);
            }
        }//for
        // if we here and didn't returned yet - BusHdr->BusBuild is the biggest one.
        Status = AppendItemLst(BusDb, SdlData);
    }

    return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	PciCfg8()
//
// Description: Will do PCI Configuration Space Access 8 bit width
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
EFI_STATUS PciCfg8(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *RbIo, PCI_CFG_ADDR addr, BOOLEAN wr, UINT8 *buff){
	if(wr)return RbIo->Pci.Write(RbIo, EfiPciWidthUint8, addr.ADDR, 1, (VOID*)buff);
	else return RbIo->Pci.Read(RbIo, EfiPciWidthUint8, addr.ADDR, 1, (VOID*)buff);
}
//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	PciCfg16()
//
// Description: Will do PCI Configuration Space Access 16 bit width
//
// Input:
//  ROOT_BRIDGE_IO_PROTOCOL *RbIo   Pointer to PciRootBridgeIO Protocol.
//  PCI_CFG_ADDR            addr    PCI_CFG_ADDR filled by caller
//  BOOLEAN                 wr      TRUE = Write FALSE = Read
//  UINT16                  *buff   Pointer Data Buffer.
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
EFI_STATUS PciCfg16(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *RbIo, PCI_CFG_ADDR addr, BOOLEAN wr, UINT16 *buff){
	EFI_STATUS	Status;
//------------
	if(wr)Status=RbIo->Pci.Write(RbIo, EfiPciWidthUint16, addr.ADDR, 1, (VOID*)buff);
	else Status=RbIo->Pci.Read(RbIo, EfiPciWidthUint16, addr.ADDR, 1, (VOID*)buff);
	//it might be a Width issue on Pci Root bridge level
	if(Status==EFI_INVALID_PARAMETER) {
		UINT8	*b=(UINT8*)buff;
	//--------------------
		Status=PciCfg8(RbIo, addr, wr, b);
		if(EFI_ERROR(Status)) return Status;

        //Check if Extended register used then Addr.Register is ignored.
        if( addr.Addr.ExtendedRegister != 0) addr.Addr.ExtendedRegister += 1;
        else addr.Addr.Register += 1;

		b++;
		Status=PciCfg8(RbIo, addr, wr, b);
	}
	return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	PciCfg32()
//
// Description: Will do PCI Configuration Space Access 32 bit width
//
// Input:
//  ROOT_BRIDGE_IO_PROTOCOL *RbIo   Pointer to PciRootBridgeIO Protocol.
//  PCI_CFG_ADDR            addr    PCI_CFG_ADDR filled by caller
//  BOOLEAN                 wr      TRUE = Write FALSE = Read
//  UINT32                  *buff   Pointer Data Buffer.
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
EFI_STATUS PciCfg32(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *RbIo, PCI_CFG_ADDR addr, BOOLEAN wr, UINT32 *buff){
	EFI_STATUS	Status;
//------------
	if(wr)Status=RbIo->Pci.Write(RbIo, EfiPciWidthUint32, addr.ADDR, 1, (VOID*)buff);
	else Status=RbIo->Pci.Read(RbIo, EfiPciWidthUint32, addr.ADDR, 1, (VOID*)buff);
	//it might be a Width issue on Pci Root bridge level
	if(Status==EFI_INVALID_PARAMETER) {
		UINT16	*b=(UINT16*)buff;
	//--------------------
		Status=PciCfg16(RbIo, addr, wr, b);
		if(EFI_ERROR(Status)) return Status;

        //Check if Extended register used then Addr.Register is ignored.
        if( addr.Addr.ExtendedRegister != 0) addr.Addr.ExtendedRegister += 2;
        else addr.Addr.Register += 2;

		b++;
		Status=PciCfg16(RbIo, addr, wr, b);
	}
	return Status;
}


//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:	PciCfg64()
//
// Description: Will do PCI Configuration Space Access 64 bit width
//
// Input:
//  ROOT_BRIDGE_IO_PROTOCOL *RbIo   Pointer to PciRootBridgeIO Protocol.
//  PCI_CFG_ADDR            addr    PCI_CFG_ADDR filled by caller
//  BOOLEAN                 wr      TRUE = Write FALSE = Read
//  UINT64                  *buff   Pointer Data Buffer.
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
EFI_STATUS PciCfg64(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *RbIo, PCI_CFG_ADDR addr, BOOLEAN wr, UINT64 *buff){
	EFI_STATUS	Status;
//------------
	if(wr)Status=RbIo->Pci.Write(RbIo, EfiPciWidthUint64, addr.ADDR, 1, (VOID*)buff);
	else Status=RbIo->Pci.Read(RbIo, EfiPciWidthUint64, addr.ADDR, 1, (VOID*)buff);
	//it might be a Width issue on Pci Root bridge level
	if((Status==EFI_INVALID_PARAMETER) && (buff!=NULL)) {
		UINT32	*b=(UINT32*)buff;
	//--------------------
		Status=PciCfg32(RbIo, addr, wr, b);
		if(EFI_ERROR(Status)) return Status;

        //Check if Extended register used then Addr.Register is ignored.
        if( addr.Addr.ExtendedRegister != 0) addr.Addr.ExtendedRegister += 4;
        else addr.Addr.Register += 4;

		b++;
		Status=PciCfg32(RbIo, addr, wr, b);
	}
	return Status;
}

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
BOOLEAN IsFunc0OfMfDev(PCI_DEV_INFO *Device ){
    //If (Func0==NULL && FuncCount==0) function is a single function device, following fields are not used and reserved;
    //If (Func0!=NULL && FuncCount==0) function is one of the Func1..Func7 of multyfunc device Func0 points on DevFunc0;
    //If (Func0!=NULL && (FuncCount!=0 || FuncInitCnt!=0)) function is Func0 of multyfunc device DevFunc holds pointers at all other Func1..7 found
    //If (Func0==NULL && FuncCount!=0) Illehgal combination - reserved!
    if((Device->Func0!=NULL) && ((Device->FuncInitCnt!=0)||(Device->FuncCount!=0))) return TRUE;
    return FALSE;
}

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
BOOLEAN IsFunc0(PCI_DEV_INFO *Device ){
    //If (Func0==NULL && FuncCount==0) function is a single function device, following fields are not used and reserved;
    //If (Func0!=NULL && FuncCount==0) function is one of the Func1..Func7 of multyfunc device Func0 points on DevFunc0;
    //If (Func0!=NULL && (FuncCount!=0 || FuncInitCnt!=0)) function is Func0 of multyfunc device DevFunc holds pointers at all other Func1..7 found
    //If (Func0==NULL && FuncCount!=0) Illehgal combination - reserved!
    if(IsFunc0OfMfDev(Device)) return TRUE;
    if((Device->Func0==NULL) && (Device->FuncInitCnt==0) && (Device->FuncCount==0)) return TRUE;
    return FALSE;
}

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
EFI_STATUS CpyItemLst(T_ITEM_LIST *Lst, T_ITEM_LIST **NewLstPtr)
{
    T_ITEM_LIST     *NewLst;
//--------------------------
	if(*NewLstPtr == NULL) *NewLstPtr = MallocZ(sizeof(T_ITEM_LIST));

	if (*NewLstPtr==NULL) return EFI_OUT_OF_RESOURCES;

	NewLst = *NewLstPtr;
	NewLst->InitialCount = Lst->InitialCount;
	if (Lst->InitialCount == 0) return EFI_SUCCESS;

	NewLst->Items = MallocZ( Lst->InitialCount * sizeof(VOID*) );
	if (!NewLst->Items) return EFI_OUT_OF_RESOURCES;

	pBS->CopyMem((VOID*)NewLst->Items,(VOID*)Lst->Items,sizeof(VOID*)*Lst->ItemCount);

	NewLst->ItemCount = Lst->ItemCount;

    return EFI_SUCCESS;
}

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
EFI_STATUS SetSubBus(PCI_DEV_INFO *Brg, UINT8 SubBusNo)
{
	PCI_CFG_ADDR	addr;
//---------------------------------------
	addr.ADDR=Brg->Address.ADDR;
	addr.Addr.Register=PCI_BRIDGE_SUBORDINATE_BUS_REGISTER_OFFSET; //Sub Bus No reg

	return PciCfg8(Brg->RbIo, addr,TRUE,&SubBusNo);
}


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
EFI_STATUS MapBridgeBuses(PCI_DEV_INFO *Brg)
{
	EFI_STATUS		Status;
	PCI_CFG_ADDR	addr;
	UINT8			bus;
	PCI_BRG_EXT		*ext=(PCI_BRG_EXT*)(Brg+1);
//--------------------------------
	//Get Bridge Initial Address
	addr.ADDR=Brg->Address.ADDR;
	//Primary bus;
	addr.Addr.Register=PCI_BRIDGE_PRIMARY_BUS_REGISTER_OFFSET;    //Primary BusNo
	bus=Brg->Address.Addr.Bus;
	Status=PciCfg8(Brg->RbIo,addr,TRUE,&bus);
	if(EFI_ERROR(Status))return Status;

	//SecondaryBus Register
	addr.Addr.Register=PCI_BRIDGE_SECONDARY_BUS_REGISTER_OFFSET; //Secondary BusNo

	bus=(UINT8)ext->Res[rtBus].Base;
	Status=PciCfg8(Brg->RbIo,addr,TRUE,&bus);
	if(EFI_ERROR(Status))return Status;

	//Now Programm SubordinateBusNo reg
	bus=(UINT8)(ext->Res[rtBus].Base+ext->Res[rtBus].Length-1);
	return SetSubBus(Brg, bus);
}


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
EFI_STATUS RestoreBridgeBuses(PCI_DEV_INFO *DnPort){
    UINTN           i;
    PCI_BRG_EXT     *ext=(PCI_BRG_EXT*)(DnPort+1);
    PCI_DEV_INFO    *dev;
    EFI_STATUS      Status=EFI_SUCCESS;
//---------------------
    for(i=0; i< ext->ChildCount; i++){
        dev=ext->ChildList[i];
        if(dev->Type==tPci2PciBrg){
            Status=MapBridgeBuses(dev);
            if(EFI_ERROR(Status)) return Status;
            //call recoursively to cover all hierarchy
            Status=RestoreBridgeBuses(dev);
            if(EFI_ERROR(Status)) return Status;
        }
    }
    return Status;
}

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
BOOLEAN IsPowerOfTwo(UINT64 Value){
	UINTN 	i;
	UINT64 	t;
//--------------
	for(i=0; i<64; i++){
		t=Shl64(1,(UINT8)i);
		if(Value&t) {
			if(Value&(~t))return FALSE;
			else return TRUE;
		}
	}
	return FALSE;
}

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
VOID ClearBar(PCI_BAR *Bar)
{
	Bar->Type=tBarUnused;
	Bar->Gran=0;
	Bar->Length=0;
	Bar->Base=0;
}

UINT8 FindRootBus(PCI_DEV_INFO *Device){
	PCI_DEV_INFO *parent=Device->ParentBrg;
//-----------------------
	if (parent==NULL) return 0xFF;

	while(parent->Type!=tPciRootBrg){
		parent=parent->ParentBrg;
	}
	return parent->Address.Addr.Bus;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   BrdGetPciSetupData()
//
// Description: Porting function which collects all PCI Bus Driver specific
// SETUP configuration data and returns it to the PCI Bus Driver.
//
// Input:
//  PCI_SETUP_DATA*   PciSetupData  Pointer to the Pci Bus specific setup data buffer.
//
// Output:	Nothing
//----------------------------------------------------------------------------
//<AMI_PHDR_END>

//PCI_DEV_01

// <AMI_PHDR_START>
//-------------------------------------------------------------------------
//
// Procedure: GetSioLdVarName
//
// Description:
//This function generate Variable Name associated with each SIO Logical Device.
//"Volatile" or None "Volatile"
//
// Notes:
//-------------------------------------------------------------------------
// <AMI_PHDR_END>
EFI_STRING AmiPciGetPciVarName(AMI_SDL_PCI_DEV_INFO *SdlInfo, UINTN Idx, PCI_SETUP_TYPE VarType){
	CHAR16 s[40];
	EFI_STRING	ret=NULL;
//------------------	
	if(SdlInfo==NULL) return ret;
	
	switch (VarType){
		case dtDevice:
			Swprintf(s, PCI_DEV_DEV_VAR_NAME_FORMAT, Idx);
		break;	
		case dtPcie1:
			Swprintf(s, PCI_DEV_PE1_VAR_NAME_FORMAT, Idx);
		break;	
		case dtPcie2:
			Swprintf(s, PCI_DEV_PE2_VAR_NAME_FORMAT, Idx);
		break;	
		case dtHp:
			Swprintf(s, PCI_DEV_HP_VAR_NAME_FORMAT, Idx);
		break;
		default: return NULL;
	}
	
	ret=MallocZ(Wcslen(s)*sizeof(CHAR16)+sizeof(CHAR16));
	Wcscpy(ret, s);
	return ret;
}

EFI_STATUS AmiPciGetPciHpSetupData( PCI_HP_SETUP_DATA 		*PciHp,
									AMI_SDL_PCI_DEV_INFO    *DevSdlInfo,//OPTIONAL if == NULL get defaults...
									UINTN					DevIdx,		//OPTIONAL if == 0 
									BOOLEAN					Set)
{		
    EFI_STATUS          	Status=EFI_SUCCESS;
    UINTN               	vsz;
    EFI_STRING				vname=NULL;
//-------------------------------------------------------------
	if(PciHp != NULL){
		//Get PCI Express GEN 1 Device setup data variable
		vsz=sizeof(PCI_HP_SETUP_DATA);
		vname=AmiPciGetPciVarName(DevSdlInfo, DevIdx, dtHp);

		if(vname!=NULL){
			if(Set) {
				Status=pRS->SetVariable(vname, &gPciSetupGuid, 
								(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS), 
								vsz, PciHp);
			} else 	Status=pRS->GetVariable(vname,&gPciSetupGuid, NULL, &vsz, PciHp);
		}
		pBS->FreePool(vname);
	} else Status=EFI_INVALID_PARAMETER; //Pcie2	
	return Status;
}

EFI_STATUS AmiPciGetPcie2SetupData( PCIE2_SETUP_DATA 		*Pcie2,
									AMI_SDL_PCI_DEV_INFO    *DevSdlInfo,//OPTIONAL if == NULL get defaults...
									UINTN					DevIdx,		//OPTIONAL if == 0 
									BOOLEAN					Set)
{		
    EFI_STATUS          	Status=EFI_SUCCESS;
    UINTN               	vsz;
    EFI_STRING				vname=NULL;
//-------------------------------------------------------------

	if(Pcie2!=NULL){
		//Get PCI Express GEN 1 Device setup data variable
		vsz=sizeof(PCIE2_SETUP_DATA);
		vname=AmiPciGetPciVarName(DevSdlInfo, DevIdx, dtPcie2);

		if(vname!=NULL){
			if(Set) {
				Status=pRS->SetVariable(vname, &gPciSetupGuid, 
								(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS), 
								vsz, Pcie2);
				ASSERT_EFI_ERROR(Status);
				pBS->FreePool(vname);
				return Status;
			} else 	Status=pRS->GetVariable(vname,&gPciSetupGuid, NULL, &vsz, Pcie2);
		}
			
		//Fill Data buffer with Default values...
		if(vname==NULL || EFI_ERROR(Status) ){
			//Gen2 Link Settings
			//UINT8   LnkSpeed;               //[Auto]\ 5.0 GHz \ 2.5 GHz
			Pcie2->LnkSpeed=PCI_SETUP_AUTO_VALUE;
			//UINT8   DeEmphasis;             //[Disable]\ Enable
			//UINT8   ClockPm;                //[Disable]\ Enable
			//UINT8   ComplSos;               //[Disable]\ Enable

			//UINT8   HwAutoWidth;            //[Enable]\ Disable
			//PciSetupData->HwAutoWidth=0;
			//UINT8   HwAutoSpeed;            //[Enable]\ Disable
			//PciSetupData->HwAutoSpeed=0;
			if(Status==EFI_NOT_FOUND) {
				Status=pRS->SetVariable(vname, &gPciSetupGuid, 
								(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS), 
								vsz, Pcie2);
				ASSERT_EFI_ERROR(Status);
			} else {
				ASSERT_EFI_ERROR(Status);
			}
		}
		if(vname!=NULL)pBS->FreePool(vname);
	} else Status=EFI_INVALID_PARAMETER; //Pcie2	
	return Status;
}


EFI_STATUS AmiPciGetPcie1SetupData( PCIE1_SETUP_DATA 		*Pcie1,
									AMI_SDL_PCI_DEV_INFO    *DevSdlInfo,//OPTIONAL if == NULL get defaults...
									UINTN					DevIdx,		//OPTIONAL if == 0 
									BOOLEAN					Set)
{		
    EFI_STATUS          	Status=EFI_SUCCESS;
    UINTN               	vsz;
    EFI_STRING				vname=NULL;
//-------------------------------------------------------------

	if(Pcie1!=NULL){
		//Get PCI Express GEN 1 Device setup data variable
		vsz=sizeof(PCIE1_SETUP_DATA);
		vname=AmiPciGetPciVarName(DevSdlInfo, DevIdx, dtPcie1);
		if(vname!=NULL){
			if(Set) {
				Status=pRS->SetVariable(vname, &gPciSetupGuid, 
								(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS), 
								vsz, Pcie1);
				ASSERT_EFI_ERROR(Status);
				pBS->FreePool(vname);
				return Status;
			} else 	Status=pRS->GetVariable(vname,&gPciSetupGuid, NULL, &vsz, Pcie1);
		}
			
		//Fill Data buffer with Default values...
		if(vname==NULL || EFI_ERROR(Status) ){
			//PCI Express Device Settings: [] - default
			//UINT8	RelaxedOrdering; 		//[Disable]\ Enable
			//UINT8	ExtTagField; 			//[Disable]\ Enable
			//UINT8	NoSnoop;				// Disable \[Enable]
			Pcie1->NoSnoop=1;

			//UINT8	MaxPayload;				//[Auto]\ 128 \ 256 \ 512 \ 1024 \ 2048 \ 4096 (in bytes)
			Pcie1->MaxPayload=PCI_SETUP_AUTO_VALUE;
			//UINT8 MaxReadRequest;			//[Auto]\ 128 \ 256 \ 512 \ 1024 \ 2048 \ 4096 (in bytes)
			Pcie1->MaxReadRequest=PCI_SETUP_AUTO_VALUE;
			//PCI Express Link settings: [] - default
			//UINT8 AspmMode; 				//[Disable]\ Auto \ Forse L0
			//UINT8 ExtendedSynch;			//[Disable]\ Enable

			//Fill in Default Values... (buffer was initialized with ZEROs)
			//UINT8   LnkTrRetry;             //Disable\ 2 \ 3 \[5]
			Pcie1->LnkTrRetry=5;
			//UINT16  LnkTrTimout;            //<1.[2]..100> (Microseconds uS)
			Pcie1->LnkTrTimeout=0x10;

			//Gen2 Device Settings
			//UINT8   ComplTimeOut;           //[Disable]\ Default \ 50 - 100 us \ 1ms - 10ms \ 16...
			//UINT8   AriFwd;                 //[Disable]\ Enable
			//UINT8   AtomOpReq;              //[Disable]\ Enable
			//UINT8   AtomOpEgressBlk;        //[Disable]\ Enable
			//UINT8   IDOReq;                 //[Disable]\ Enable
			//UINT8   IDOCompl;               //[Disable]\ Enable
			//UINT8   LtrReport;              //[Disable]\ Enable
			//UINT8   E2ETlpPrBlk;            //[Disable]\ Enable
			if(Status==EFI_NOT_FOUND) {
				Status=pRS->SetVariable(vname, &gPciSetupGuid, 
								(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS), 
								vsz, Pcie1);
				ASSERT_EFI_ERROR(Status);
			} else {
				ASSERT_EFI_ERROR(Status);
			}
		}
		
		if(vname!=NULL)pBS->FreePool(vname);
	} else Status=EFI_INVALID_PARAMETER; //Pcie1	
	return Status;
}


EFI_STATUS AmiPciGetPciDevSetupData(PCI_DEVICE_SETUP_DATA 	*PciDev,
									AMI_SDL_PCI_DEV_INFO    *DevSdlInfo,//OPTIONAL if == NULL get defaults...
									UINTN					DevIdx,
									BOOLEAN 				Set)		

{		
    EFI_STATUS          	Status=EFI_SUCCESS;
    UINTN               	vsz;
    EFI_STRING				vname=NULL;
//-------------------------------------------------------------
	if(PciDev!=NULL){
		//Get Device Conventional PCI  setup data variable
		vsz=sizeof(PCI_DEVICE_SETUP_DATA);
		vname=AmiPciGetPciVarName(DevSdlInfo, DevIdx, dtDevice);

		if(vname!=NULL){
			if(Set) {
				Status=pRS->SetVariable(vname, &gPciSetupGuid, 
								(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS), 
								vsz, PciDev);
				ASSERT_EFI_ERROR(Status);
				pBS->FreePool(vname);
				return Status;
			} else 	Status=pRS->GetVariable(vname, &gPciSetupGuid, NULL, &vsz, PciDev);
		}
		
		//Fill Data buffer with Default values...
		if(vname==NULL || EFI_ERROR(Status)){
		
			//Zero out PCI_SETUP_DATA buffer
			pBS->SetMem(PciDev, sizeof(PCI_DEVICE_SETUP_DATA), 0);
		
			//General PCI Settings: [] - default
			//UINT8 PciLatency;				//[32]\ 64 \ 96 \ 128 \ 160 \ 192 \ 224 \ 248
			PciDev->PciLatency=32;
			//UINT8 OpRomPos                // Legacy \[EFI Compatible]
			//PciSetupData->OpRomPost=1;
			//UINT8 PciXLatency;			// 32 \[64]\ 96 \ 128 \ 160 \ 192 \ 224 \ 248
			PciDev->PciXLatency=64;
			//UINT8   VgaPallete;             //[Disable]\ Enable
			//UINT8   PerrEnable;             //[Disable]\ Enable
			//UINT8   SerrEnable;             //[Disable]\ Enable
			if(Status==EFI_NOT_FOUND) {
				Status=pRS->SetVariable(vname, &gPciSetupGuid, 
								(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS), 
								vsz, PciDev);
				ASSERT_EFI_ERROR(Status);
			} else {
				ASSERT_EFI_ERROR(Status);
			}
		}
		if(vname!=NULL)pBS->FreePool(vname);
	}else Status=EFI_INVALID_PARAMETER; //PciCommon	
	return Status;
}
    
EFI_STATUS AmiPciGetCommonSetupData(PCI_COMMON_SETUP_DATA 	*PciCommon)
{		
    EFI_STATUS          	Status=EFI_SUCCESS;
    UINTN               	vsz;
//-------------------------------------------------------------
    //Get Setup Data
	if(PciCommon!=NULL){
		//Get global setup variable
		vsz=sizeof(PCI_COMMON_SETUP_DATA);
		Status=pRS->GetVariable(PCI_COMMON_VAR_NAME, &gPciSetupGuid, NULL, &vsz, PciCommon);
		if(EFI_ERROR(Status)){
			//Get Common Settings first
			pBS->SetMem(PciCommon, sizeof(PCI_COMMON_SETUP_DATA), 0);
			//PciCommon->S3ResumeVideoRepost=0;	//[Disable]\ Enable
			//PciCommon->S3PciExpressScripts=0; //[Disable]\ Enable
			PciCommon->HotPlug=1;             // Disable \[Enable]
			//PciCommon->Above4gDecode=0;       //[Disable]\ Enable
			//PciCommon->SriovSupport=0;        //[Disable]\ Enable
			
			if(Status==EFI_NOT_FOUND) {
				Status=pRS->SetVariable(PCI_COMMON_VAR_NAME, &gPciSetupGuid, 
								(EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS), 
								vsz, PciCommon);
				ASSERT_EFI_ERROR(Status);
			} else {
				ASSERT_EFI_ERROR(Status);
			}
		}
	} else Status=EFI_INVALID_PARAMETER; //PciCommon	
	return Status;
}

//<AMI_PHDR_START>
//----------------------------------------------------------------------------
// Procedure:   BrdGetPciSetupData()
//
// Description: Porting function which collects all PCI Bus Driver specific
// SETUP configuration data and returns it to the PCI Bus Driver.
//
// Input:
//  PCI_SETUP_DATA*   PciSetupData  Pointer to the Pci Bus specific setup data buffer.
//
// Output:	Nothing
//----------------------------------------------------------------------------
//<AMI_PHDR_END>
#if (PCI_SETUP_USE_APTIO_4_STYLE == 1)
EFI_STATUS AmiPciGetSetupData(	PCI_SETUP_DATA 			*PciSetupData, 
								PCI_COMMON_SETUP_DATA 	*PciCommon,
								PCI_HOTPLUG_SETUP_DATA 	*HpSetup)
{
    EFI_GUID 			SetupGuid	= SETUP_GUID;
    EFI_STATUS          Status;
    UINTN               sz=sizeof(SETUP_DATA);
    SETUP_DATA          *SetupData=NULL;
//-------------------------------------

    if( PciSetupData==NULL && PciCommon==NULL && HpSetup==NULL) return EFI_INVALID_PARAMETER;
    //Get Setup Data
    SetupData=MallocZ(sizeof(SETUP_DATA));
    if (SetupData==NULL) return EFI_OUT_OF_RESOURCES;

    //Get global setup variable
    Status=GetEfiVariable(L"Setup",&SetupGuid, NULL, &sz, &SetupData);
	if(EFI_ERROR(Status)){
		
		if(PciCommon!=NULL){
			//Get Common Settings first
			pBS->SetMem(PciCommon, sizeof(PCI_COMMON_SETUP_DATA), 0);
			//PciCommon->S3ResumeVideoRepost=0;	//[Disable]\ Enable
			//PciCommon->S3PciExpressScripts=0; //[Disable]\ Enable
			PciCommon->HotPlug=1;             // Disable \[Enable]
			//PciCommon->Above4gDecode=0;       //[Disable]\ Enable
			//PciCommon->SriovSupport=0;        //[Disable]\ Enable
		}
		
		if(PciSetupData!=NULL){
		    //Zero out PCI_SETUP_DATA buffer
			pBS->SetMem(PciSetupData, sizeof(PCI_SETUP_DATA), 0);
			

			//General PCI Settings: [] - default
			//UINT8 PciLatency;				//[32]\ 64 \ 96 \ 128 \ 160 \ 192 \ 224 \ 248
			PciSetupData->PciDevSettings.PciLatency=32;
			//UINT8 OpRomPos                // Legacy \[EFI Compatible]
			//PciSetupData->OpRomPost=1;
	//#if PCI_X_SUPPORT
			//UINT8 PciXLatency;			// 32 \[64]\ 96 \ 128 \ 160 \ 192 \ 224 \ 248
			PciSetupData->PciDevSettings.PciXLatency=64;
	//#endif
			//UINT8   VgaPallete;             //[Disable]\ Enable
			//UINT8   PerrEnable;             //[Disable]\ Enable
			//UINT8   SerrEnable;             //[Disable]\ Enable

			//PCI Express Device Settings: [] - default
			//UINT8	RelaxedOrdering; 		//[Disable]\ Enable
			//UINT8	ExtTagField; 			//[Disable]\ Enable
			//UINT8	NoSnoop;				// Disable \[Enable]
			PciSetupData->Pcie1Settings.NoSnoop=1;

			//UINT8	MaxPayload;				//[Auto]\ 128 \ 256 \ 512 \ 1024 \ 2048 \ 4096 (in bytes)
			PciSetupData->Pcie1Settings.MaxPayload=55;
			//UINT8 MaxReadRequest;			//[Auto]\ 128 \ 256 \ 512 \ 1024 \ 2048 \ 4096 (in bytes)
			PciSetupData->Pcie1Settings.MaxReadRequest=55;
			//PCI Express Link settings: [] - default
			//UINT8 AspmMode; 				//[Disable]\ Auto \ Forse L0
			//UINT8 ExtendedSynch;			//[Disable]\ Enable

			//Fill in Default Values... (buffer was initialized with ZEROs)
			//UINT8   LnkTrRetry;             //Disable\ 2 \ 3 \[5]
			PciSetupData->Pcie1Settings.LnkTrRetry=5;
			//UINT16  LnkTrTimout;            //<1.[2]..100> (Microseconds uS)
			PciSetupData->Pcie1Settings.LnkTrTimeout=0x10;

			//Gen2 Device Settings
			//UINT8   ComplTimeOut;           //[Disable]\ Default \ 50 - 100 us \ 1ms - 10ms \ 16...
			//UINT8   AriFwd;                 //[Disable]\ Enable
			//UINT8   AtomOpReq;              //[Disable]\ Enable
			//UINT8   AtomOpEgressBlk;        //[Disable]\ Enable
			//UINT8   IDOReq;                 //[Disable]\ Enable
			//UINT8   IDOCompl;               //[Disable]\ Enable
			//UINT8   LtrReport;              //[Disable]\ Enable
			//UINT8   E2ETlpPrBlk;            //[Disable]\ Enable

			//Gen2 Link Settings
			//UINT8   LnkSpeed;               //[Auto]\ 5.0 GHz \ 2.5 GHz
			PciSetupData->Pcie2Settings.LnkSpeed=55;
			//UINT8   DeEmphasis;             //[Disable]\ Enable
			//UINT8   ClockPm;                //[Disable]\ Enable
			//UINT8   ComplSos;               //[Disable]\ Enable

			//UINT8   HwAutoWidth;            //[Enable]\ Disable
			//PciSetupData->HwAutoWidth=0;
			//UINT8   HwAutoSpeed;            //[Enable]\ Disable
			//PciSetupData->HwAutoSpeed=0;
		}

		if(HpSetup != NULL){
//#if AMI_HOTPLUG_INIT_SUPPORT
			//UINT8   BusPadd;                // Disable \[1]\ 2 \ 3 \ 4 \ 5
			HpSetup->ArrayField.BusPadd=1;         //    0       1    2     3    4
			//UINT8   IoPadd;                 // Disable \[ 4K]\ 8K \ 16K \ 32K
			HpSetup->ArrayField.IoPadd=0x1000;     //    0       1    2     3    4     5     6     7
			HpSetup->ArrayField.Io32Padd=0;		  //Not used in IAPC systems	
			//UINT8   Mmio32Padd;             // Disable \  1M \ 4M \  8M \[16M]\ 32M \ 64M \128M
			HpSetup->ArrayField.Mmio32Padd=0x1000000;
			//UINT8   Mmio32PfPadd;           // Disable \  1M \ 4M \  8M \[16M]\ 32M \ 64M \128M
			HpSetup->ArrayField.Mmio32PfPadd=0x1000000;
			HpSetup->ArrayField.Mmio64Padd=0;      //[Disable]\  1M \ 4M \  8M \ 16M \ 32M \ 64M \ 128M \ 256M \ 512M \ 1G
			HpSetup->ArrayField.Mmio64PfPadd=0;    //[Disable]\  1M \ 4M \  8M \ 16M \ 32M \ 64M \ 128M \ 256M \ 512M \ 1G

//#endif
		}

	} else {
		//General PCI Settings: [] - default
		if (PciCommon!=NULL){
			PciCommon->S3ResumeVideoRepost=SetupData->S3ResumeVideoRepost;
			PciCommon->S3PciExpressScripts=SetupData->S3PciExpressScripts;
			PciCommon->HotPlug=SetupData->HotPlugEnable;
			PciCommon->Above4gDecode=SetupData->Above4gDecode;
			PciCommon->SriovSupport = SetupData->SriovSupport;
		}

		if(PciSetupData!=NULL){
			PciSetupData->PciDevSettings.PciLatency = SetupData->PciLatency;
//#if PCI_X_SUPPORT
			PciSetupData->PciDevSettings.PciXLatency = SetupData->PciXLatency;
//#endif
			//General2 PCI Settings: [] - default
			PciSetupData->PciDevSettings.VgaPallete = SetupData->VgaPallete;
			PciSetupData->PciDevSettings.PerrEnable = SetupData->PerrEnable;
			PciSetupData->PciDevSettings.SerrEnable = SetupData->SerrEnable;

//#if PCI_EXPRESS_SUPPORT
			//PCI Express Device Settings: [] - default
			PciSetupData->Pcie1Settings.RelaxedOrdering = SetupData->RelaxedOrdering;
			PciSetupData->Pcie1Settings.ExtTagField = SetupData->ExtTagField;
			PciSetupData->Pcie1Settings.NoSnoop = SetupData->NoSnoop;
			PciSetupData->Pcie1Settings.MaxPayload = SetupData->MaxPayload;
			PciSetupData->Pcie1Settings.MaxReadRequest = SetupData->MaxReadRequest;
			//PCI Express Link settings: [] - default
			PciSetupData->Pcie1Settings.AspmMode = SetupData->AspmMode;
			PciSetupData->Pcie1Settings.ExtendedSynch = SetupData->ExtendedSynch;

			//Fill in Default Values... (buffer was initialized with ZEROs)
			//UINT8   LnkTrRetry;             //[Disable]\ 2 \ 3 \ 5
			PciSetupData->Pcie1Settings.LnkTrRetry=SetupData->LnkTrRetry;
			PciSetupData->Pcie1Settings.LnkTrTimeout=SetupData->LnkTrTimeout;
			PciSetupData->Pcie1Settings.LnkDisable=SetupData->LnkDisable; //[Keep ON == 0] / Disable ==1
			//UINT8   ClockPm;               //[Disable]\ Enable
			PciSetupData->Pcie1Settings.ClockPm=SetupData->ClockPm;
			//UINT8 S3PciExpressScripts       //[Disable]\ Enable
			//PciSetupData->S3PciExpressScripts = SetupData->S3PciExpressScripts;

	//#endif

	//#if PCI_EXPRESS_GEN2_SUPPORT
			//Gen2 Device Settings
			//UINT8   ComplTimeOut;           //[Disable]\ Default \ 50 - 100 us \ 1ms - 10ms \ 16...
			PciSetupData->Pcie2Settings.ComplTimeOut=SetupData->ComplTimeOut;
			//UINT8   AriFwd;                 //[Disable]\ Enable
			PciSetupData->Pcie2Settings.AriFwd=SetupData->AriFwd;
			//UINT8   AtomOpReq;              //[Disable]\ Enable
			PciSetupData->Pcie2Settings.AtomOpReq=SetupData->AtomOpReq;
			//UINT8   AtomOpEgressBlk;        //[Disable]\ Enable
			PciSetupData->Pcie2Settings.AtomOpEgressBlk=SetupData->AtomOpEgressBlk;
			//UINT8   IDOReq;                 //[Disable]\ Enable
			PciSetupData->Pcie2Settings.IDOReq=SetupData->IDOReq;
			//UINT8   IDOCompl;               //[Disable]\ Enable
			PciSetupData->Pcie2Settings.IDOCompl=SetupData->IDOCompl;
			//UINT8   LtrReport;              //[Disable]\ Enable
			PciSetupData->Pcie2Settings.LtrReport=SetupData->LtrReport;
			//UINT8   E2ETlpPrBlk;            //[Disable]\ Enable
			PciSetupData->Pcie2Settings.E2ETlpPrBlk=SetupData->E2ETlpPrBlk;

			//Gen2 Link Settings
			//UINT8   LnkSpeed;               //[Auto]\ 5.0 GHz \ 2.5 GHz
			PciSetupData->Pcie2Settings.LnkSpeed=SetupData->LnkSpeed;
			//UINT8   ComplSos;               //[Disable]\ Enable
			PciSetupData->Pcie2Settings.ComplSos=SetupData->ComplSos;
			//UINT8   HwAutoWidth;            //[Enable]\ Disable
			PciSetupData->Pcie2Settings.HwAutoWidth=SetupData->HwAutoWidth;
			//UINT8   HwAutoSpeed;            //[Enable]\ Disable
			PciSetupData->Pcie2Settings.HwAutoSpeed=SetupData->HwAutoSpeed;
//#endif
		}
		if(HpSetup != NULL){
//#if AMI_HOTPLUG_INIT_SUPPORT
			HpSetup->ArrayField.BusPadd=(UINT64)SetupData->BusPadd;
			HpSetup->ArrayField.IoPadd=(UINT64)SetupData->IoPadd*0x400; //Stored in units of KB
			HpSetup->ArrayField.Mmio32Padd=(UINT64)SetupData->Mmio32Padd*0x100000;
			HpSetup->ArrayField.Mmio32PfPadd=(UINT64)SetupData->Mmio32PfPadd*0x100000;
			HpSetup->ArrayField.Mmio64Padd=(UINT64)SetupData->Mmio64Padd*0x100000;
			HpSetup->ArrayField.Mmio64PfPadd=(UINT64)SetupData->Mmio64PfPadd*0x100000;
//#endif
		}
    }
    pBS->FreePool(SetupData);
    return EFI_SUCCESS;
}
#else

EFI_STATUS AmiPciGetSetupData(	PCI_COMMON_SETUP_DATA 	*PciCommon,
								PCI_DEVICE_SETUP_DATA	*PciDev, 
								PCIE1_SETUP_DATA		*Pcie1,
								PCIE2_SETUP_DATA		*Pcie2,
								AMI_SDL_PCI_DEV_INFO    *DevSdlInfo,  	//OPTIONAL if ==NULL get defaults...
								UINTN					DevIdx)			//OPTIONAL 
{
    EFI_STATUS          	Status=EFI_SUCCESS;
//-------------------------------------

    if( PciCommon==NULL && PciDev==NULL && Pcie1==NULL && Pcie2==NULL) return EFI_INVALID_PARAMETER;
    
    //Get Setup Data
	if(PciCommon!=NULL) Status=AmiPciGetCommonSetupData(PciCommon);
	ASSERT_EFI_ERROR(Status);
	if(EFI_ERROR(Status)) return Status;
	
	
	if(PciDev!=NULL)Status=AmiPciGetPciDevSetupData(PciDev,DevSdlInfo,DevIdx,FALSE);
	ASSERT_EFI_ERROR(Status);
	if(EFI_ERROR(Status)) return Status;
	
	if(Pcie1!=NULL)Status=AmiPciGetPcie1SetupData(Pcie1,DevSdlInfo,DevIdx,FALSE);
	ASSERT_EFI_ERROR(Status);
	if(EFI_ERROR(Status)) return Status;
	
	if(Pcie2!=NULL)Status=AmiPciGetPcie2SetupData(Pcie2,DevSdlInfo,DevIdx,FALSE);
	ASSERT_EFI_ERROR(Status);

	return Status;
}
#endif


EFI_STATUS LaunchPortingRoutine(PCI_DEV_INFO *Device, PCI_INIT_STEP InitStep, 
								VOID *Param1, VOID *Param2, VOID *Param3, VOID *Param4)
{
	EFI_STATUS	Status=EFI_UNSUPPORTED;
	EFI_STATUS	Status2=EFI_UNSUPPORTED;
//-----------------------
	if(Device->PciPortProtocol==NULL) return Status;

	switch (InitStep) {
		//(OEM_PCI_DEVICE_ATTRIBUTE_FUNCTION)(VOID *PciIoProtocol, UINT64 *Attr, UINT64 Capab, BOOLEAN Set);
		case isPciSetAttributes:
			Status=Device->PciPortProtocol->PciPortOemAttributes(
					Param1, 				//VOID *PciIoProtocol,		
					(UINT64*)Param2,		//UINT64 *Attr,
					*((UINT64*)Param3), 		//UINT64 Capab,
					*((BOOLEAN*)Param4)
					);
		break;
		
		//(OEM_PCI_DEVICE_GET_OPT_ROM_FUNCTION)(VOID *PciIoProtocol, VOID **OptRom, UINT64 *OptRomSize);
		case isPciGetOptionRom:
			Status=Device->PciPortProtocol->PciPortOemGetOptRom(
					Param1, 		//VOID *PciIoProtocol,		
					(VOID**)Param2,		//VOID **OptRom,
					(UINT64*)Param3 //UINT64 *OptRomSize
					);
		break;
		
		//(OEM_PCI_DEVICE_OUT_OF_RESOURCES_FUNCTION)(VOID *PciIoProtocol, UINTN Count, UINTN ResType);		
		case isPciOutOfResourcesCheck:
			Status=Device->PciPortProtocol->PciPortOutOfResourcesRemoveDevice(
					Param1, 			//VOID *PciIoProtocol,		
					*((UINTN*)Param2),	//UINTN Count,
					*((UINTN*)Param3) 	//UINTN ResType
					);
		break;
		
		//(OEM_PCI_PROGRAM_DEVICE_FUNCTION)(VOID *PciIoProtocol);
		case isPciSkipDevice:
			Status=Device->PciPortProtocol->PciPortSkipThisDevice(
					Param1 				//VOID *PciIoProtocol,		
					);
		break;
		
		//(OEM_PCI_PROGRAM_DEVICE_FUNCTION)(VOID *PciIoProtocol);
		case isPciProgramDevice:
			Status=Device->PciPortProtocol->PciPortOemProgDevice(
					Param1 				//VOID *PciIoProtocol,		
					);
		break;
		
		//(OEM_PCI_PROGRAM_DEVICE_FUNCTION)(VOID *PciIoProtocol);
		case isPciGetSetupConfig:
//			EFI_DEADLOOP();		
			Status=Device->PciPortProtocol->PciPortCheckBadPcieDevice(
					Param1 				//VOID *PciIoProtocol,		
					);
			//In aptio V we don't have PciPortCheckBadPcieDevice 
			//so we will map it to the setup settings disabling PCI express initialization.
			if (!EFI_ERROR(Status)) Device->DevSetup.Pcie1Disable=TRUE;
		break;

		//(OEM_PCI_DEVICE_SET_ASPM_FUNCTION_PTR)(VOID *PciIoProtocol, VOID *AspmMode);
		case isPcieSetAspm:
			//In Aptio V set ASPM routine passes both Upstream and Downstream Ports,
			//Aptio4 must call porting hook twice 1 for UP and 1 for Down Stream 
			
			//this one for DN Stream Port 
			Status=Device->PciPortProtocol->PciPortOemSetAspm(
					Param1, 				//VOID *PciIoProtocol,		
					Param3					//VOID *AspmMode
					);
			
			//this one for UP Stream Port 
			Status2=Device->PciPortProtocol->PciPortOemSetAspm(
					Param2, 				//VOID *PciIoProtocol,		
					Param3					//VOID *AspmMode
					);
		break;

		//(OEM_PCI_DEVICE_SET_LNK_SPEED_FUNCTION_PTR)(VOID *PciIoProtocol, UINT8 *LnkSpeed, UINT8 SuppSpeeds);
		case isPcieSetLinkSpeed:
			//In aptio V set Lnk Speed routine passes both Upstream and Downstream Ports,
			//Aptio4 must call porting hook twice 1 for UP and 1 for Down Stream 
			
			//this one for DN Stream Port 
			Status=Device->PciPortProtocol->PciPortOemSetLnkSpeed(
					Param1, 				//VOID *PciIoProtocol,		
					(UINT8*)Param3,			//UINT8 *LnkSpeed
					*((UINT8*)Param4)		//UINT8 SuppSpeeds (max link speed)
					);
			
			//this one for UP Stream Port 
			Status2=Device->PciPortProtocol->PciPortOemSetLnkSpeed(
					Param2, 				//VOID *PciIoProtocol,		
					(UINT8*)Param3,			//UINT8 *LnkSpeed
					*((UINT8*)Param4)		//UINT8 SuppSpeeds (max link speed)
					);
			
		break;
		
		default: 
			break;
	} //switch...
	
	//UNSUPPORTED is a NORMAL condition meaning NO PORTING NEEDED FOR THIS DEVICE!
	if(EFI_ERROR(Status)&& Status!=EFI_UNSUPPORTED){
		ASSERT_EFI_ERROR(Status);
		return Status;
	}
	
	if(EFI_ERROR(Status2)&& Status2!=EFI_UNSUPPORTED){
		ASSERT_EFI_ERROR(Status2);
		return Status2;
	}
	
	//In this case possible status combination would be 
	//SUCCESS & UNSUPPORTED = SUCCESS or 
	//UNSUPPORTED & UNSUPPORTED = UNSUPPORTED.
	return (Status & Status2);
}


EFI_STATUS LaunchInitRoutine(VOID *RoutineOwner, PCI_INIT_STEP InitStep, PCI_INIT_TYPE InitType,
		VOID *Param1, VOID *Param2, VOID *Param3, VOID *Param4)
{
	AMI_BOARD_INIT_PARAMETER_BLOCK 	Parameters;
	EFI_STATUS						Status;
	AMI_BOARD_INIT_PROTOCOL			*InitProtocol=NULL; //RoutineOwner->PciInitProtocol;
	UINTN							InitFunction;       //=RoutineOwner->AmiSdlPciDevData->InitRoutine;
	AMI_SDL_PCI_DEV_INFO			*AmiSdlPciDevData=NULL;
	UINTN							idx;
//------------------------
	
	switch(InitType){
		case itHost:
			InitProtocol=((PCI_HOST_BRG_DATA*)RoutineOwner)->PciInitProtocol;
			AmiSdlPciDevData=((PCI_HOST_BRG_DATA*)RoutineOwner)->HbSdlData;
			idx=((PCI_HOST_BRG_DATA*)RoutineOwner)->HbSdlIndex;
		break;
			
		case itRoot:
			InitProtocol=((PCI_ROOT_BRG_DATA*)RoutineOwner)->PciInitProtocol;
			AmiSdlPciDevData=((PCI_ROOT_BRG_DATA*)RoutineOwner)->RbSdlData;
			idx=((PCI_ROOT_BRG_DATA*)RoutineOwner)->RbSdlIndex;
		break;
		
		case itDevice:
			InitProtocol=((PCI_DEV_INFO*)RoutineOwner)->PciInitProtocol;
			AmiSdlPciDevData=((PCI_DEV_INFO*)RoutineOwner)->AmiSdlPciDevData;
			idx=((PCI_DEV_INFO*)RoutineOwner)->SdlDevIndex;
		break;
		
		default: return EFI_INVALID_PARAMETER;
	}
	
	if ((LaunchInitRoutinePriority==lpAptioV) && (InitType==itDevice)){
		PCI_TRACE((TRACE_PCI,"\nPciPort: PciCompatibility(Prior=%d; InitStep=%d; SdlIndex=%d(0x%X);",
				LaunchInitRoutinePriority,InitStep, idx, idx));
		Status=LaunchPortingRoutine((PCI_DEV_INFO*)RoutineOwner, InitStep, Param1,Param2,Param3,Param4);
		PCI_TRACE((TRACE_PCI,")=%r\n\n",Status));
		if(EFI_ERROR(Status)){
			if(Status!=EFI_UNSUPPORTED) 
				ASSERT_EFI_ERROR(Status);
		}
	}
	
	//First Check if device has SDL data and InitRoutine...
	if(AmiSdlPciDevData!= NULL && AmiSdlPciDevData->InitRoutine!=0){ 
	
		InitFunction=AmiSdlPciDevData->InitRoutine;
		if(InitProtocol->FunctionCount < InitFunction) {
			PCI_TRACE((TRACE_PCI,"\n\nPciInit: InitProtocol->FunctionCount[%d] < InitFunction (%d)!=EFI_INVALID_PARAMETER\n", InitProtocol->FunctionCount, InitFunction));
			return EFI_INVALID_PARAMETER;
		}
	
		Parameters.Signature=AMI_PCI_PARAM_SIG;
		Parameters.InitStep=InitStep;
		Parameters.Param1=Param1;
		Parameters.Param2=Param2;
		Parameters.Param3=Param3;
		Parameters.Param4=Param4;
	
		PCI_TRACE((TRACE_PCI,"\n\nPciInit: PciInitProtocol->Function[%d](InitStep=%d; SdlIndex=%d(0x%X);",InitFunction, InitStep, idx, idx));
		Status=InitProtocol->Functions[InitFunction](InitProtocol,&InitFunction, &Parameters);
		PCI_TRACE((TRACE_PCI,")=%r\n",Status));
	} else Status = EFI_UNSUPPORTED;	

	if ((LaunchInitRoutinePriority==lpAptio4x) && (InitType==itDevice)){
		PCI_TRACE((TRACE_PCI,"\nPciPort: PciCompatibility(Prior=%d; InitStep=%d; SdlIndex=%d(0x%X);",
				LaunchInitRoutinePriority, InitStep, idx, idx));
		Status=LaunchPortingRoutine((PCI_DEV_INFO*)RoutineOwner, InitStep, Param1,Param2,Param3,Param4);
		PCI_TRACE((TRACE_PCI,")=%r\n",Status));
		if(EFI_ERROR(Status)){
			if(Status!=EFI_UNSUPPORTED)
			ASSERT_EFI_ERROR(Status);
		}
	}
	
	if(idx){}; //EIP 131922 Building PCI module in GCC
	
	return Status;
}

BOOLEAN PciBusCheckIfPresent(PCI_DEV_INFO *Dev, AMI_SDL_PCI_DEV_INFO *DevFnData){
	PCI_CFG_ADDR 	addr;
	UINT32 			val=0;
//-------------------------------
	// Always report virtual devices as present
	if (DevFnData->PciDevFlags.Bits.Virtual) return TRUE;

	addr.ADDR=Dev->Address.ADDR;
	addr.Addr.Device=DevFnData->Device;
	addr.Addr.Function=DevFnData->Function;
	addr.Addr.ExtendedRegister=0;
	addr.Addr.Register=0; //will read vid did register

	// For root bridges, get bus from SDL
	if (DevFnData->PciDevFlags.Bits.RootBridgeType)
		addr.Addr.Bus=DevFnData->Bus;

	ASSERT_EFI_ERROR(PciCfg32(Dev->RbIo, addr, FALSE, &val));

    if(val==0xFFFFFFFF || val==0 ) return FALSE;
    else  return TRUE;
}

//PciBus BusShift variable format....
#define PCI_BUS_SHIFT_VAR_NAME_FORMAT 	L"H[%d]R[%d](%a)BSH"

EFI_STATUS AmiPciBusShift(			AMI_SDL_PCI_DEV_INFO 	*DevSdlInfo,
						  	  	  	INT16					*ShiftValue,	
						  	  	  	UINTN					HostIdx,
									UINTN					RootIdx,
						  	  	  	BOOLEAN					Set)
{
    EFI_STATUS          	Status=EFI_SUCCESS;
    UINTN               	vsz;
    CHAR16 					vname[40];
//------------------------------------------------------
	//Get Device Conventional PCI  setup data variable
	vsz=sizeof(INT16);
	pBS->SetMem(&vname[0], sizeof(vname), 0);
	Swprintf(&vname[0], PCI_BUS_SHIFT_VAR_NAME_FORMAT, HostIdx, RootIdx, DevSdlInfo->AslName);

	if(Set) {
		Status=pRS->SetVariable(vname, &gPciSetupGuid, EFI_VARIABLE_BOOTSERVICE_ACCESS, vsz, ShiftValue);
	} else {
		Status=pRS->GetVariable(vname, &gPciSetupGuid, NULL, &vsz, ShiftValue);
	}

	return Status;
}



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

